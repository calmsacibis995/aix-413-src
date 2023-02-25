static char sccsid[] = "@(#)07	1.8  src/bos/usr/sbin/crash/dump.c, cmdcrash, bos411, 9428A410j 1/14/93 15:31:44";

/*
 * COMPONENT_NAME: CMDCRASH
 *
 * FUNCTIONS: 
 *	showdump(batch,all,special_routines)
 *	match_rtn_names(index)
 *	disp_cdts()
 *	get_cdtname(index)
 *	endofdata(areas,start)
 * 	call_fmt_rts(index)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _KERNEL
#undef _KERNEL
#endif
/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include <sys/dir.h>
#include "crash.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/dump.h>
#include <sys/param.h>

extern char **environ;

#define DEFAULT_FMT_RTN	"_default_dmp_fmt"

char *get_cdtname(),*match_rtn_names();
unsigned long endofdata();
void showdump();

/*
 *  Function:	dump routine formatters
 *  Returns:	nothing
 */
void
showdump(batch,all,special_routines)
long	batch;	/* batch mode flag; batch implies "all" flag to dmproutines */
int	all;	/* dump all areas flag */
int	special_routines;	/* some routines called thru this interface
				   even though they are "traditional" */
{
	int cdt_index;

	/* if special routine do it and return */
	switch(special_routines) {
		case TTY:
			/* call something */
			return;
		case 0:
			/* not a special routine */
			break;
		default:
			printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_1, "what?\n") );
			return;
	}

	/* 
	 * display components of dump file 
	 * and return a cdt the user wishes dumped
	 */
	if(!(cdt_index=disp_cdts()))
		return ; 

	/* fork & exec the requested choice */
	call_fmt_rts(cdt_index);

}

/*  Function: 	Use index to try and match a cdt with a dump routine.
 *		If no match use default dump routine.
 *  Returns:	1 = found some dump routines ; 0 = no dump routines;
 */
char *
match_rtn_names(index)
{
	DIR *dirp;
	struct dirent *dp;
	struct stat statbuf;
	static char	path[MAXPATHLEN]; 	/* full path name */
	char *cdtname;		/* name of cdt we want to format */

	/* translate index into cdt name that matches */
	if((cdtname=get_cdtname(index)) ==0 ) {
		printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_2, "Component dump table entry %d not found.\n") ,index);
		return 0;
	}

	/*
	 * find all the dump formatting routines
	 * that are available to format cdts
	 */
	/* open the directory */
	if((dirp = opendir(DMPRTNS_DIR)) == NULL) {
		printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_3, "0452-050: No dump routines available in directory %s.\n") ,DMPRTNS_DIR);
		return 0;
	}

	/* loop, reading the files in the directory */
	while((dp=readdir(dirp)) != NULL) { /* read a filename */
		/* construct the full pathname */
		strcpy(path,DMPRTNS_DIR);
		strcat(path,"/");
		strcat(path,dp->d_name);

		/* is it a file or dir ? */
		if(stat(path,&statbuf) != 0) {
			printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_4, "0452-051: Cannot stat %s.\n") ,path);
			continue;
		}
/*DEBUG 	printf("path:%s,ISREG:0x%8.8x, ",path,S_ISREG(statbuf.st_mode));*/
/*DEBUG		printf("statbuf.st_mode:0x%08x\n",statbuf.st_mode);
*/
		if(!S_ISREG(statbuf.st_mode))	/* if not a regular file? */
			continue;

		/* compare this file to our cdt name */
/*DEBUG	  	printf("  comparing cdt:%s with file:\n",cdtname,dp->d_name);
*/
		if(!strcmp(dp->d_name,cdtname)) 
			return path; /* this is it */
			
	}

	/* 
	 * no match found so construct the full 
	 * pathname to default routine 
	 */
	strcpy(path,DMPRTNS_DIR);
	strcat(path,"/");
	strcat(path, DEFAULT_FMT_RTN);

	return path;

} /* match_rtn_names */


/* Function: display cdts found in dump
 *		and return users choice
 * Returns:  number user selects representing cdt
 *		to be displayed ; or 0 for quit;
 */
disp_cdts()
{
	char *p;
	int i;
	char	*token();	/* routine that gets users response */
	char	*str;		/* where users response is "stored" */

    	while(1) {
		for(i=1;i<MAXCDTS;i++) {
			if((p = get_cdtname(i))==0)
				break;
			if(i==1)
				printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_5, "Component Names:\n") );
			printf("%2d)  %s \n",i,p);
		}
		if(i==1) { 
			printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_6, "0452-053:  Component dump table not found.\n") );
			return 0;
		}
	
		while(token() != NULL); /* flush input buffer */
		/* change to get args if exist, use isalnum()*/

		printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_7, "Enter component number (or q to quit)\n") );
		str = token();
/*DEBUG 	printf("       dbg:token received:|%s|,0x%0x\n",str,*str); */
		/*** if atoi() of response not > 0 and < i  
		print(q to quit) and loop? 
***/
		if(!strcmp("q",str) || !strcmp("quit",str))
			return 0;

		if(i=atoi(str)) {
			if(i != -1)
				return i;
		}
    }
}



/*
 * Name: get_cdtname
 * Function:	get the Component Dump Table names from a dump
 *
 * Input:	index = request the i'th cdt name to be returned
 *
 * Output:	pt to a static buffer with the name in it or
 *		NULL if no i'th cdt entry in the dump.
 *		Also, a side effect is that mem will be postioned
 *		to index+1 cdt. 
 */
char *
get_cdtname(index)
{
	static struct cdt0 head;	/* 1 head per component dump table */
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
		printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_8, "0452-055: Cannot read dump file.\n") );
		return NULL;
	}


	lseek(mem,0,0);
	/*
	 *	loop reading a cdt_head
	 *	then for each cdt_entry seek past its bitmap
	 *	and data area.
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
#ifdef Debug 
		printf("head %d,name:%s,len:%d,0x%.8x\n",i,
			head.cdt_name,head.cdt_len,head.cdt_len);
#endif
		/* 
		 * seek past data area 
		 * note that mem is postioned at first cdt_entry 
		 * thus lseek returns current position
		 */
		offset=endofdata(NUM_ENTRIES(&head),lseek(mem,0L,1/*SEEK_CUR*/));

		if(!offset) {
			printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_9, "0452-056: Premature end of data in dump file.\n") );
			return NULL;
		}
		lseek(mem,offset,0);
		
	}
	return head.cdt_name;
		

}

/* 
 * Name:	endofdata
 * Function:	find the end of this component dump table's data
 * Returns:	offset of the end of this cdt's data areas 
 *		(offset from beggining of file since "start" is 
 *		position of start of cdt_entrys).
 *		0 on error
 * Inputs:	start - where to start reading cdt_entry structs
 *		areas - number of cdt_entrys for this cdt
 *
 * Note:	this function will affect the position of 
 *		global file descripter "mem" (dumpfile).
 *		"mem" is positioned at start, which is
 *		the first cdt_entry after a cdt_head,
 *		then left no where special
 */
unsigned long
endofdata(areas,start)
int areas;
unsigned long start;
{
	struct cdt_entry cdt_e; 
	unsigned long next_cdt_e;
	unsigned long offset;		/* return this + start */
	int i,j,nb;
	unsigned long npages;		/* num of pages represented in bitmap */
	bitmap_t bitmap[8192];		/* allows up to 256M per data area */
	unsigned long addr,count,actual_cnt;

	if (areas < 0) {
		return 0;
	}
	if (lseek(mem,start,0) == -1) {
		return 0;
	}
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
#ifdef Debug
	      printf("Entry %d:name:%s,len:0x%.8x,ptr:0x%.8x,nextcdte:0x%.8x\n",
		j,cdt_e.d_name,cdt_e.d_len,cdt_e.d_ptr,next_cdt_e);
#endif
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
#ifdef Debug
	      	printf("bitmapsize:%d, bm[0]:0x%.2x  offset:0x%.8x\n",
			BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len),bitmap[0],offset);
#endif

		/* here is the big loop */
		addr  = (unsigned long) cdt_e.d_ptr;
		count = (unsigned long) cdt_e.d_len;
#ifdef Debug
		printf("endofdata:npages:0x%x PAGESIZE:0x%x\n",npages,PAGESIZE);
		printf("endofdata: count:0x%.8x  addr:0x%.8x\n",count,addr);
#endif
		npages = (unsigned long) NPAGES(cdt_e.d_ptr,cdt_e.d_len);
		for(i = 0; i < npages; i++) {
		        actual_cnt = min(count,PAGESIZE-(addr%PAGESIZE));
#if 0
		        printf("actual_cnt=%x addr=%x\n",actual_cnt,addr);
#endif
		        if(ISBITMAP(bitmap,i)) {
		                offset += actual_cnt;
			}
		        else {
#if 0
		                printf(
				     "page %d (address 0x%.8x) not in memory\n",
				     i,addr);
#endif
			}
		        count -= actual_cnt;
		        addr  += actual_cnt;
		}
 
		/* set pointer to read nex cdt_e */
		lseek(mem,next_cdt_e,0);
	}
	lseek(mem,offset,0);
	return offset;
}

/* Name: 	call_fmt_rts
 * Function:	fork and exec the dump formatting routines
 *
 */
call_fmt_rts(index)
{
	char *fmt_rtn;		/* full pathname of routine to exec */
	char *args[5];		/* args for exec */
	char  fdbuf[5];		/* file desc in ascii */
	int fpid,wpid;	

#ifdef Debug
	printf("call_fmt: index %d, mem:0x%.8, tell:0x%.8x\n",index,mem,
		lseek(mem,0,1));
#endif
	/* match users choice with a formatting routine */
	if(!(fmt_rtn=match_rtn_names(index)))
		return ;

	/* call below will position file descriptor
	 * mem to the component dump table we are
	 * interested in
	 */
	if((get_cdtname(index-1))==0) {
		printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_10, "Specified component dump table index is not valid.\n") );
		return ;
	}
#ifdef Debug
	printf("call_fmt:new mem:0x%.8,tell:0x%.8x\n",index,mem,lseek(mem,0,1));
#endif

	/* set up args */
	args[0] = fmt_rtn;	/* pathname */
	args[1] = "-f";		/* file desc flag */
	sprintf(fdbuf,"%d",mem);
	args[2] = fdbuf;	/* file desc value */
	args[3] = "-l";		/* interactive mode flag */
	args[4] = '\0';		/* null terminate */

	errno = 0;
#ifdef Debug
	printf("about to fork %s; 0:%s,1:%s,2:%s,3:%s,4:0x%8x\n",
		fmt_rtn,args[0],args[1],args[2],args[3],args[4]);
#endif
	if((fpid=fork())==0) {		/* child */
		if(execve(fmt_rtn,args,environ)) {
			printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_13, "0452-058: Cannot execute %s, errno = %d.\n") ,fmt_rtn,errno);
			return ;
		}
	}
	else if(fpid == -1) {		/* error */
		printf( catgets(scmc_catd, MS_crsh, DUMP_MSG_14, "Cannot fork %s, errno = %d.\n") ,fmt_rtn,errno);
		return ;
	}

	/* parent here */
	if((wpid=wait()) == -1) {
#ifdef Wait_Always_Returns_Minus1
		printf("Could not wait? %d\n",errno);
#endif
		return ;
	}
	if(fpid != wpid)
		printf("pids: f:0x%.8x!=w:0x%.8x \n",fpid,wpid);
}
