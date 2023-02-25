static char sccsid[] = "@(#)21	1.18  src/bos/usr/sbin/crash/POWER/sysvad.c, cmdcrash, bos411, 9428A410j 6/8/94 14:43:17";

/*
 * COMPONENT_NAME: CMDCRASH
 *
 * FUNCTIONS: readmem,unpack,fill,pad,read_dump, readmem_thread
 *
 * ORIGINS: 3,27,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */
/*
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	"crash.h"
#include	<errno.h>
#include	<sys/dump.h>
#include    <sys/thread.h>


#define PAD 0x0BAD
#define QUERY	1		/* ask what is already unpacked */
#define STORE	2		/* record what has just been unpacked */
extern int get_slot_curthread();
extern int read_thread();
extern int  associated_proc();
char 	*get_cdtname();
ulong 	*unpack(), pad(), read_dump();
extern int dumpflag;
extern int initialized;
int	bos_cdt;
int	proc_cdt;
int	thrd_cdt;

/* #define Debug */

/*
 * Function: 	here is where we have to determine if we are
 * 	 	dealing with a dump.  If so unpack it and seek to
 *		proper position in memory.
 * Returns:	number of bytes read or 0 on error.
 */
readmem(buf, vad, len, procslot)
char *buf;
ulong vad;
ulong len;
int   procslot;
{
	char	data_area[10];
	int	i=0;
	int	threadslot;
			    

	if (!dumpflag) {
		if (lseek(mem, vad, 0) == -1) {
			return(0);
		}
		if ((i=read(mem, buf, len)) != len) {
			return(0);
		}
		return(len);
	}
	else { /* is a dump */
		if (!bos_cdt) {
			/* this is a dump with no bos cdt */
			printf( catgets(scmc_catd, MS_crsh, SYSVAD_MSG_1, "Kernel not included in this dump.\n") );
			return 0;
		}
		if (initialized && (procslot == CURPROC) && (vad >= 0x10000000)) {
			threadslot = get_slot_curthread();
			if ((procslot = associated_proc(threadslot)) == BADSLOT)
				return 0;
		}
		switch(vad >>SEGSHIFT) {
			case 0x0: 
				/* 
				 * If address is in segment 0,
	 	 		 * we want standard bos/kernel cdt
	 	 		 */
			return((int)unpack(bos_cdt,"kernel",buf,vad,len));

			case 0x2:
				sprintf(data_area,"%dU",procslot);
				return((int)unpack(proc_cdt,data_area,buf,vad,len));

			case 0xE:
				sprintf(data_area,"%dp",procslot);
				return((int)unpack(proc_cdt,data_area,buf,vad,len));

			default:
				printf( catgets(scmc_catd, MS_crsh, SYSVAD_MSG_2, "Not a valid dump data area:0x%08x\n") ,vad);
				return 0 ;

		} /* switch */
	} /* else - is a dump */
}

/*
 * Function: like readmem but the slot parameter is a threadslot.	
 *           besides, enable to read the thread structures in 	
 *           the dump.
 *		
 * Returns:	number of bytes read or 0 on error.
 */
readmem_thread(buf, vad, len, threadslot)
char *buf;
ulong vad;
ulong len;
int   threadslot;
{
	char	data_area[10];
	int	i=0;
	int	procslot;
	struct thread wk_thread;

	if (!dumpflag) 
		return(readmem(buf, vad, len, CURPROC));
	else { /* is a dump */
		
		if (!initialized)
			return(0);

		if (threadslot == CURTHREAD)
				threadslot = get_slot_curthread();

		switch(vad >>SEGSHIFT) {
			case 0x0: 
				/* 
				 * If address is in segment 0,
	 	 		 * we want standard bos/kernel cdt
	 	 		 */
				return(readmem(buf, vad, len, CURPROC));

			case 0x1:
				sprintf(data_area,"%ds",threadslot);
				return((int)unpack(thrd_cdt,data_area,buf,vad,len));

			case 0x2:
				if ((read_thread(threadslot, &wk_thread)) != sizeof (struct thread))
					return 0;


				if(vad < (int) wk_thread.t_uaddress.userp - sizeof(struct uthread)) {
					/* we are in the stack */
					sprintf(data_area,"%ds",threadslot);
					return((int)unpack(thrd_cdt,data_area,buf,vad,len));
				}
				else { 
				if ((vad < (int) wk_thread.t_uaddress.userp) ||
					(vad >= (int)wk_thread.t_uaddress.userp + sizeof(struct user))) {
						/* we are in the uthread structure */
						sprintf(data_area,"%du",threadslot);
						return((int)unpack(thrd_cdt,data_area,buf,vad,len));
					}
				else  /* we are in userp  */
					if ((procslot = associated_proc(threadslot)) != BADSLOT)
						return (readmem(buf, vad, len, procslot));
					else
						return(0);
			}

			case 0xE:
				if((vad >= SYM_VALUE(Thread)) &&
				   (vad < SYM_VALUE(Thread)+(long)v.ve_thread*sizeof(struct thread))) {
					   /* we are in the thread table */
					   sprintf(data_area,"%dt",threadslot);
					   return((int)unpack(thrd_cdt,data_area,buf,vad,len));
				   }
				else {
					if ((procslot = associated_proc(threadslot)) != BADSLOT)
						return (readmem(buf, vad, len, procslot));
					else
						return(0);
				}

			} /* switch */
	} /* else - is a dump */
}

/*
 * Function:	unpack cdt's data from a dump into callers buffer,
 * 		filling in holes with 0BADD, and return bytes read
 * Returns:	NULL on error or number of bytes read
 *		
 */
ulong *
unpack(index,data_name,ubuf,vad,len)
int index;				/* index of cdt */
char *data_name;			/* data area to unpack (read from) */
char *ubuf;				/* where to put data read */
ulong vad;				/* address from which to start reading*/
ulong len;				/* lenght in bytes to read */
{
	struct cdt0 head;		/* 1 head per component dump table */
	struct cdt_entry cdt_e;	 	/* multiple cdt entries */
	ulong offset, next_cdt_e; 	/* pointers into files */
	bitmap_t bitmap[8192];		/* allows up to 64M per data area */
	unsigned long npages;		/* num of pages represented in bitmap */
	int i,j;
	char *buf;
	ulong addr,count,actual_cnt,unpackit=0,toread,tmp_vad;
	/* buf = pt to users buffer,next byte to store into
	 * tmp_vad = current pseudo addr to start reading.
	 *	(address user is intersted in)
	 * toread = # bytes left to read for caller
	 * addr = 1st addr that can be read on this page.
	 * actual_cnt = # bytes that can be read on this page
	 * count = # bytes to read in this cdt data area
	 * on_this_page = # bytes to use from this page.
	 */
	ulong on_this_page;

	buf = ubuf;
	/* 
	 * position "mem" file descriptor 
	 * to cdt of interest 
	 */
	if((get_cdtname(index-1))==0)
		return NULL;	/* something wrong with dump file */

	/* read header */
	if(read(mem,&head,sizeof(head)) != sizeof(head)) {
		return NULL;	/* something wrong with dump file */
	}
#ifdef Debug
	printf("unpack:head name:%s,len:%d,0x%08x\n",
		head.cdt_name,head.cdt_len,head.cdt_len);
	printf("unpack:num entries:%d\n",NUM_ENTRIES(&head));
	printf("unpack:looking for vad:0x%08x, len:%d\n",vad,len);
#endif
	/* 
	 * position offset at start of 
	 * data areas (at the bitmap)
	 */
	offset = lseek(mem,0,1)+((NUM_ENTRIES(&head)*sizeof(struct cdt_entry)));

	/* 
	 * unpack data area
	 */
	 /* for each data area in this cdt */
	for(j=1;j<=NUM_ENTRIES(&head); j++) {
		/* 
		 * read in the cdt_entry
		 */
		read(mem,&cdt_e,sizeof(cdt_e)); 
		/* 
		 * save start of next cdt_entry 
		 */
		next_cdt_e = lseek(mem,0,1); 
#ifdef Debug
	      printf("Entry %d:name:%s,len:0x%.8x,ptr:0x%.8x,nextcdte:0x%.8x\n",
		j,cdt_e.d_name,cdt_e.d_len,cdt_e.d_ptr,next_cdt_e);
#endif
		npages = (ulong) NPAGES(cdt_e.d_ptr,cdt_e.d_len);
		/* 
		 * is this the cdt_entry we are interested in ?
		 */
		if(!strcmp(cdt_e.d_name,data_name)) {
			unpackit = 1;
			tmp_vad = vad;
			toread = len;
		}
		/* 
		 * read bitmap for this data area and
		 * move offset to where we are now in the file
		 */
		lseek(mem,offset,0);
		offset += read(mem,bitmap, BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len));

		addr  = (unsigned long) cdt_e.d_ptr;
		count = (unsigned long) cdt_e.d_len;
#ifdef Debug
	    /* 	printf("unpack:bitmapsize:%d, bm[0]:0x%02x  offset:0x%08x\n",
			BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len),bitmap[0],offset);*/
		printf("unpack:npages:0x%x  count:0x%.8x  addr:0x%.8x\n",
			npages,count,addr);
#endif
		/* for each page */
		for(i = 0; i < npages; i++) {
		        actual_cnt = min(count,PAGESIZE-(addr%PAGESIZE));
			/* 
			 * if the page is in the dump 
			 * and this is the data entry we want
			 */
		        if(ISBITMAP(bitmap,i)) {
				if(unpackit) {
				    /* 
			  	     * if our data is on this page get it
			 	     */
  /*** chk for boundry prob! */	    if(tmp_vad>=addr&&tmp_vad<actual_cnt+addr){
					/* Get bytes to use from this page. */
					on_this_page = min(toread,(addr+actual_cnt-tmp_vad));
					if(!fill(tmp_vad,addr,toread,
						actual_cnt,buf))
						/* trouble reading dump file */
						return NULL;
/** watch this! */			buf = (char *)((ulong)buf+on_this_page);
		        		tmp_vad += on_this_page;
		        		toread -= on_this_page;
				    }
				    else	/* seek ahead a page */
					lseek(mem,actual_cnt,1);
				}
				/* increment user's data pointer
				 * and psuedo address we are reading 
				 * from for next time
				 */
		                offset += actual_cnt;	/* inc file pt */
			    }
		        else {
				/* 
			 	* if page is NOT in dump
			 	* fill memory with some dummy value
			 	*/
				if(unpackit) {
				    /* 
			  	     * if our data is on this page pad it
			 	     */
  /*** chk for boundry prob! */	    if(tmp_vad>=addr&&tmp_vad<actual_cnt+addr){
					/* Get bytes to use from this page. */
					on_this_page = min(toread,(addr+actual_cnt-tmp_vad));
					/* lseek(mem,offset,0); */
		                 	/* inc in mem pt */
					pad(buf,on_this_page);
					/*
					 * We do not want this printf to appear
					 * if we are looking for parts of the
					 * uarea outside the first page.  The 
					 * first page of the uarea is the only
					 * one that is pinned and therefore the
					 * only one saved in the dump but it
					 * contains all the important info for
					 * crash.
					 */
					if ((tmp_vad < (SYM_VALUE(Ublock)+PAGESIZE) -
						 (SYM_VALUE(Ublock) % PAGESIZE)) ||
					    (tmp_vad > SYM_VALUE(Ublock)+
						sizeof(struct ublock))) 
		                		printf( catgets(scmc_catd, MS_crsh, SYSVAD_MSG_3, 
						"Address 0x%08x: page not in dump. Padding with 0x%08x.\n") ,
				     		tmp_vad,PAD);
/** watch this! */			buf = (char *)((ulong)buf+on_this_page);
		        		tmp_vad += on_this_page;
		        		toread -= on_this_page;
				    }
				}
			}
		    count -= actual_cnt;
		    addr  += actual_cnt;
		    if(unpackit && (!toread))	/* finished unpacking */
				break;
		}
 
		if(unpackit) {	
			/* we have finished */
			if(!toread)
				return((ulong *)len);
			/* check to see if we get all the data that
			 * was requested - if not tell user.
			 * There is a basic assumption here that 
			 * if any part of the data requested is here
	 		 * the beginning will be here, though not 
			 * necessarily the end.
			 */
			printf( catgets(scmc_catd, MS_crsh, SYSVAD_MSG_4, "Dump data incomplete.Only %d bytes found out of %d.\n") ,
				len - toread,len);
			return((ulong *)(len-toread));
		}
		/* 
	 	 * set pointer to read nex cdt_e
		 */
		lseek(mem,next_cdt_e,0);
	}
	return NULL;	/* didn't find the cdt entry we wanted */
}


/* Name:	fill - fill users buffer with as much data as they
 *		need from this page 
 * Returns	num of bytes read; or 0 on error;
 */
fill(wanted,pos,get,cnt,buf)
ulong wanted;			/* starting addr to read from */
ulong pos;			/* next byte read = this address */
ulong get;			/* num of bytes to xfer to buf */
ulong cnt;			/* num of bytes to advance fd pointer */
				/* i.e. num of bytes on this page of dump */
char *buf;			/* put bytes here */
{
	ulong count;
	ulong can_get = min(get,(pos+cnt-wanted));

#ifdef Debug
	printf("fill:wntd:0x%08x,pos:0x%08x,get:%d,cnt:%d\n",wanted,pos,
		get,cnt);
#endif
	if(wanted < pos) {
		printf( catgets(scmc_catd, MS_crsh, SYSVAD_MSG_5, "0452-374: Parameters to fill not valid.\n") );
		return 0;
	}
	if(wanted > pos+cnt) {
		printf( catgets(scmc_catd, MS_crsh, SYSVAD_MSG_6, "0452-375: Parameters to fill not valid.\n") );
		return 0;
	}
	/* lseek to where to begin reading */
	if(wanted > pos){
#ifdef Debug
		printf("fill:lseek:0x%08x\n",wanted-pos);
#endif
		if(lseek(mem, wanted-pos, 1) == -1) 
			return 0;
	}
	/* 
	 * read which ever is less: 
	 * end of page OR bytes to get from this page.
	 */
	count = min(cnt,can_get);
	read(mem,buf,count );
	/* 
	 * if(get <= cnt)  we are through getting data for the caller
	 * if(get>cnt) we have more to get but we have read to the end
	 *	of this page in the dump 
	 */
#ifdef Debug
	printf("fill: returning %d\n",count);
#endif
	return count;
}

/*
 * Name: pad
 * 
 * Function:	write cnt bytes of PAD into memory
 *		at loc
 *
 * Returns:	number of bytes xfered
 */
ulong
pad(loc,count )
char *loc;
ulong count;
{
	ulong i,*wordloc,cnt;

	cnt = count;
	/* 
	 * loc is not on a word boundry 
	 * so pad it with zero until it is
	 */
	if(i=(ulong)loc%4)
		while(i--) {
			*loc++ = 0;
			cnt-- ;
		}
			
	/* 
	 * pad each word with PAD
	 */
	wordloc = (ulong *)loc;
	while(cnt/4) {
		*wordloc++ = PAD;
		cnt -= 4;
	}
	loc = (char *)wordloc;
	
	/* 
	 * if we have less than a word's worth of
	 * bytes left, pad them with zero
	 */
	if(i=cnt%4)
		while(i--) 
			*loc++ = 0;

	return count;
}


#if 0
/*
 * Name: read_dump
 * 
 * Function:	read len bytes into buf
 *		Uses global pointers cdtfp_ZERO (points to 
 *		start of dump data area expanded in memory),
 *		cdtfp_OFFSET (where dump data area came 
 *		from in kernel/user space), and cdtfp_END
 *		(end of expanded dump data area)
 *
 * Returns:	number of bytes xfered to buf
 */
ulong
read_dump(buf,len,addr)
char *buf;			/* buffer we are "reading" into */
int len;			/* length to "read" */
ulong addr;			/* addr at which to  "read" */
{
	ulong ad;		/* data area requested to read */
	ulong data_start;	/* start of dump data area */
	ulong mem_len;		/* length of malloc'ed area */
	char *mem_pt;		/* pt to malloc'ed area we want*/
	ulong i;

	ad = addr & 0x0FFFFFFF;	/* strip of seg reg */
	data_start = cdtfp_OFFSET & 0x0FFFFFF;
	/* 
	 * if the first byte reguested to read 
	 * starts before the first address dumped
	 * return an error
	 */
	if(ad < data_start){
#ifdef Debug
		printf("read_dump: data 0x%08x not in dump (start:0x%08x).\n",
			addr,data_start);
#endif
		return 0;
	}
	mem_len = (cdtfp_END & 0x0FFFFFFF) - (cdtfp_ZERO & 0x0FFFFFFF);
	/* 
	 * if the last byte requested to read 
	 * would take us past the amount of memory malloc'ed
	 * return an error
	 */
	if((ad-data_start+len) > mem_len){
#ifdef Debug
		printf("read_dump: data 0x%08x not in dump (end:0x%08x).\n",
			addr,data_start+mem_len);
#endif
		return 0;
	}
	/* 
	 * compute offset into our memory area
	 * where we should start reading
	 */
	mem_pt = ad-data_start+cdtfp_ZERO;

	/* read it */
	for(i=1; i<=len ; i++)
		*buf++ = *mem_pt++;

	return len;

}
#endif

#undef Debug
