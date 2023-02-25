static char sccsid[] = "@(#)81  1.1  src/bos/usr/sbin/snap/read_dump.c, cmdsnap, bos411, 9432A411a 8/4/94 13:38:11";

/*
 * COMPONENT_NAME: CMDSNAP
 *
 * FUNCTIONS: readmem,unpack,fill,pad,readmem_thread
 *
 * ORIGINS: 27
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
#include	"check.h"
#include	<errno.h>
#include	<sys/dump.h>
#include    <sys/thread.h>


#define PAD 0x0BAD
#define QUERY	1		/* ask what is already unpacked */
#define STORE	2		/* record what has just been unpacked */

char *get_cdtname();
ulong *unpack(), pad();
unsigned long endofdata();
int	bos_cdt;
int	proc_cdt;
int	thrd_cdt;

extern int get_slot_curthread();
extern int read_thread();
extern int  associated_proc();
extern 	struct nlist *Proc;
extern 	struct nlist *Thread;
extern 	struct nlist *Ppd, *Kernel_heap;
extern 	struct nlist *Ublock, *g_kxsrval;

/*
 * Name: readmem
 *
 * Function: 	Here is where we have to determine if we are
 * 	 	dealing with a dump.  If so unpack it and seek to
 *		proper position in memory.
 *
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
			    

	if (!bos_cdt) 
		{
		/* this is a dump with no bos cdt */
		return 0;
		}
	switch(vad >>SEGSHIFT) 
		{
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
			return 0 ;

		} /* switch */
}

/*
 * Name:	unpack
 *	
 * Function:	Unpack cdt's data from a dump into callers buffer,
 * 		filling in holes with 0BADD, and return bytes read.
 *
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
	if(read(mem,&head,sizeof(head)) != sizeof(head)) 
		return NULL;	/* something wrong with dump file */

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
		npages = (ulong) NPAGES(cdt_e.d_ptr,cdt_e.d_len);
		/* 
		 * is this the cdt_entry we are interested in ?
		 */
		if(!strcmp(cdt_e.d_name,data_name)) 
			{
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

		/* for each page */
		for(i = 0; i < npages; i++) {
		    actual_cnt = min(count,PAGESIZE-(addr%PAGESIZE));
			/* 
			 * if the page is in the dump 
			 * and this is the data entry we want
			 */
		    if(ISBITMAP(bitmap,i)) 
				{
				if(unpackit) 
					{
				    /* 
			  	     * if our data is on this page get it
			 	     */
  				    if(tmp_vad>=addr&&tmp_vad<actual_cnt+addr)
						{
						/* Get bytes to use from this page. */
						on_this_page = min(toread,(addr+actual_cnt-tmp_vad));
						if(!fill(tmp_vad,addr,toread,actual_cnt,buf))
							/* trouble reading dump file */
							return NULL;
						buf = (char *)((ulong)buf+on_this_page);
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
		        else 
				{
				/* 
			 	* if page is NOT in dump
			 	* fill memory with some dummy value
			 	*/
				if(unpackit) 
					{
				    /* 
			  	     * if our data is on this page pad it
			 	     */
  					if(tmp_vad>=addr&&tmp_vad<actual_cnt+addr)
						{
                    	on_this_page = min(toread,(addr+actual_cnt-tmp_vad));
                    	pad(buf,on_this_page);
						buf = (char *)((ulong)buf+on_this_page);
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
 
		if(unpackit) 
			{	
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

	if(wanted < pos) 
		return 0;

	if(wanted > pos+cnt)
		return 0;

	/* lseek to where to begin reading */
	if(wanted > pos)
		{
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

#undef Debug
/*
 * Name: get_cdtname
 * Function:    get the Component Dump Table names from a dump
 *
 * Input:       index = request the i'th cdt name to be returned
 *
 * Output:      pt to a static buffer with the name in it or
 *              NULL if no i'th cdt entry in the dump.
 *              Also, a side effect is that mem will be postioned
 *              to index+1 cdt.
 */
char *
get_cdtname(index)
{
        static struct cdt0 head;        /* 1 head per component dump table */
        unsigned long t,offset;
        int i;

        /* we are just postioning our (dump) pointer to read
         * the next cdt
         */
        if(!index) {
                lseek(mem,0,0);
                /* read past head */
                if(read(mem,&head,sizeof(head)) != sizeof(head)) {
                        /* end of table */
                        return NULL;
                }
                if(head.cdt_magic != DMP_MAGIC) {
                         /*
                          * seek past unused lvm data and
                         * try and read another header
                         */
                         lseek(mem,512,0);
                         if (read(mem,&head,sizeof(head)) != sizeof(head))
                                 return NULL;
                        if (head.cdt_magic !=DMP_MAGIC)
                                return NULL;
                        else
                                lseek(mem,512,0);
                }
                else
                        lseek(mem,0,0);
                return  "unused";
        }
        lseek(mem,0,0);
        if(read(mem,&t,sizeof(t)) != sizeof(t)) {
                return NULL;
        }


        lseek(mem,0,0);
        /*
         *      loop reading a cdt_head
         *      then for each cdt_entry seek past its bitmap
         *      and data area.
         */
        for(i=1;i<=index;i++) {

                /* read past head */
                if(read(mem,&head,sizeof(head)) != sizeof(head)) {
                        /* end of table */
                        return NULL;
                }
                if(head.cdt_magic != DMP_MAGIC) {
                         /*
                          * seek past unused lvm data and
                         * try and read another header
                         */
                        if (i==1) {
                                 lseek(mem,512,0);
                                 if (read(mem,&head,sizeof(head))
                                        != sizeof(head))
                                         return NULL;
                                if (head.cdt_magic !=DMP_MAGIC)
                                        return NULL;
                        }
                        else
                                return NULL;
                }
                /*
                 * seek past data area
                 * note that mem is postioned at first cdt_entry
                 * thus lseek returns current position
                 */
                offset=endofdata(NUM_ENTRIES(&head),lseek(mem,0L,1/*SEEK_CUR*/));

                if(!offset) {
                        return NULL;
                }
                lseek(mem,offset,0);

        }
        return head.cdt_name;
}

/*
 * Name:        endofdata
 * Function:    find the end of this component dump table's data
 * Returns:     offset of the end of this cdt's data areas
 *              (offset from beggining of file since "start" is
 *              position of start of cdt_entrys).
 *              0 on error
 * Inputs:      start - where to start reading cdt_entry structs
 *              areas - number of cdt_entrys for this cdt
 *
 * Note:        this function will affect the position of
 *              global file descripter "mem" (dumpfile).
 *              "mem" is positioned at start, which is
 *              the first cdt_entry after a cdt_head,
 *              then left no where special
 */
unsigned long
endofdata(areas,start)
int areas;
unsigned long start;
{
        struct cdt_entry cdt_e;
        unsigned long next_cdt_e;
        unsigned long offset;           /* return this + start */
        int i,j,nb;
        unsigned long npages;           /* num of pages represented in bitmap */
        bitmap_t bitmap[8192];          /* allows up to 256M per data area */
        unsigned long addr,count,actual_cnt;

        if (areas < 0) 
                return 0;

        if (lseek(mem,start,0) == -1)
                return 0;
       
        /*
         * position offset at start of
         * data areas
         */
        offset = start + (areas * sizeof(struct cdt_entry));

        for(j=1;j<=areas;j++){
                /* read in the cdt_entry */
                if (read(mem,&cdt_e,sizeof(cdt_e)) != sizeof(cdt_e)) {
                        return 0;
                }
                next_cdt_e = lseek(mem,0,1);
                /* read bitmap for this data area */
                if (lseek(mem,offset,0) == -1) {
                        return 0;
                }
                nb = BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len);
                if (nb > sizeof(bitmap) || read(mem,bitmap, nb) != nb) {
                        return 0;
                }

                /* move offset to where we are now */
                offset += nb;

                /* here is the big loop */
                addr  = (unsigned long) cdt_e.d_ptr;
                count = (unsigned long) cdt_e.d_len;
                npages = (unsigned long) NPAGES(cdt_e.d_ptr,cdt_e.d_len);
                for(i = 0; i < npages; i++) {
                        actual_cnt = min(count,PAGESIZE-(addr%PAGESIZE));
                        if(ISBITMAP(bitmap,i)) 
                                offset += actual_cnt;
                        count -= actual_cnt;
                        addr  += actual_cnt;
                }

                /* set pointer to read nex cdt_e */
                lseek(mem,next_cdt_e,0);
        }
        lseek(mem,offset,0);
        return offset;
}

