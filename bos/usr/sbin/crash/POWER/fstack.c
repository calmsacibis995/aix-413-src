static char sccsid[] = "@(#)97	1.10  src/bos/usr/sbin/crash/POWER/fstack.c, cmdcrash, bos411, 9428A410j 6/4/94 18:24:36";
/*
*/
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: fstack,xdump
 *
 * ORIGINS: 27,3,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
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

#include "crash.h"
#include <fcntl.h>
#include <sys/debug.h>
#include <sys/pseg.h>
#include <sys/seg.h>

#define MAXPROCSIZE 10000
#define MAXSAVEREG  15
#define LINKREG     15
#define REGARG      13
#define READSIZ	    256

#define R1	1			/* R1 is the frame pointer */
#define FP	x_thread.ut_save.gpr[R1]
#define IAR	x_thread.ut_save.iar
#define LR	x_thread.ut_save.lr

#define RET_OFFSET	(sizeof(ulong))

#define KSTACK_TOP	x_thread.ut_kstack

extern int errno;
extern int dumpflag;
extern	struct	uthread	x_thread;
extern	char symnamebuf[];

/* 
 * NAME: fstack
 *
 * FUNCTION:
 * 	prints the stack and trace of a thread
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

fstack(threadslot)
int	threadslot;

{
	register  int	ret;
	struct	syment	*sp, *search();
	ulong	u_segval;	/* segval of segment 2 */
	ulong frame_ptr, old_frame_ptr;
	ulong ret_addr;
	int fs;
	ulong segment;		/* segval of of either segment 1 or 2 depending where the thread stack is */
	struct thread thread;
	struct proc proc;

	if (threadslot == CURTHREAD) {
		threadslot = get_slot_curthread();
	}
	
	if(threadslot > (int)v.ve_thread) {
		printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_20, 
		"0452-600: Thread slot %d above maximum limit.Specify a value less than %d.\n"),
		threadslot,v.ve_thread);
		return(-1);
	}
	if((ret=getuarea_thread(threadslot,&u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_21, 
		"0452-601: Cannot read uarea for this thread.\n") );
		return(-1);
	}
	else if(ret == SWAPPED) {
		printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_22, 
		"0452-602: Thread is currently swapped out.\n"));
		return(-1);
    }

	if ((FP >> SEGSHIFT) == PRIVSEG) {
		/* the stack is in segment 2 */
		segment = u_segval;
	}
	else {
		/* the stack is in segment 1 */
		if ((ret = read_thread(threadslot, &thread)) != sizeof(struct thread)) {
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_23, 
							"0452-603: Cannot read thread structure.\n"));
			return(-1);
		}
		if ((ret = read_proc(thread.t_procp, &proc)) != sizeof(struct proc)){
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_24, 
							"0452-604: Cannot find associated process.\n") );
			return(-1);
		}
		segment = proc.p_kstackseg;
	}

	printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_4, "STACK TRACE:\n") );

	sp = search(IAR,symnamebuf,!MATCHNAME);
	if(sp == 0)
		printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_5, 
		"\n\t**** no match ****\n") );
	else if((int)sp == -1) /* error */
		return(-1);
	if (strcmp(sp->n_name,"endcomm") == 0)
		printf("\n\t**** %8.8x ****\n", IAR);
	else if(sp->n_zeroes) {
		printf("\n\t**** %s ****\n", sp->n_name);
	}
	else {		/* flexname */
		printf("\n\t**** %s ****\n", symnamebuf);
	}
	old_frame_ptr = frame_ptr = FP;
	if (frame_ptr >= KSTACK_TOP || 
	    frame_ptr < (KSTACK_TOP - KSTACKSIZE)) {
		printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_6, 
		"0452-074: Frame pointer 0x%8x not valid.\n") ,frame_ptr);
		return(-1);
	}
	if (dumpflag) {
		if (readmem_thread(&frame_ptr, frame_ptr, sizeof frame_ptr, threadslot)
		    != sizeof frame_ptr){
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_7, 
			"0452-075: Cannot read frame pointer at address 0x%8x.\n") ,old_frame_ptr);
			return (-1);
		}
		if (frame_ptr == 0xbad)
			return(-1);
	}
	else {
		lseek(mem, frame_ptr, 0);
		if (readx(mem, &frame_ptr, sizeof frame_ptr, segment)
		    != sizeof frame_ptr){
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_8, 
			"0452-076: Cannot read frame pointer at address 0x%8x.\n") ,old_frame_ptr);
			return (-1);
		}
	}
	while (1) {
		if (frame_ptr >= KSTACK_TOP || 
		    frame_ptr < (KSTACK_TOP - KSTACKSIZE)) {
                        if(frame_ptr != 0)
			   printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_9, 
                           "0452-077: Frame pointer 0x%8x not valid.\n") ,frame_ptr);
			return(-1);
		}
		fs = (int)frame_ptr - (int)old_frame_ptr;
		xdump(old_frame_ptr, fs, segment, threadslot);
		old_frame_ptr = frame_ptr;
		if (dumpflag) {
			if (readmem_thread(&frame_ptr, frame_ptr, sizeof frame_ptr,
			     threadslot) != sizeof frame_ptr){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_10, 
				"0452-078: Cannot read frame pointer at address 0x%8x.\n") ,old_frame_ptr);
				return (-1);
			}
			if (frame_ptr == 0xbad)
				return(-1);
		}
		else {
			lseek(mem, frame_ptr, 0);
			if (readx(mem, &frame_ptr, sizeof frame_ptr, segment)
			    != sizeof frame_ptr){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_11, 
				"0452-079: Cannot read frame pointer at address 0x%8x.\n") ,old_frame_ptr);
				return (-1);
			}
		}
		if (frame_ptr >= KSTACK_TOP)
			break;
		if (dumpflag) {
			if (readmem_thread(&ret_addr, old_frame_ptr+2*RET_OFFSET, 
			     sizeof ret_addr, threadslot) != sizeof ret_addr){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_12, 
				"0452-080: Cannot read return address at address 0x%8x.\n") ,old_frame_ptr+2*RET_OFFSET);
				return (-1);
			}
			if (ret_addr == 0xbad)
				return(-1);
		}
		else {
			lseek(mem, RET_OFFSET, 1);
			if (readx(mem, &ret_addr, sizeof ret_addr, segment)
			    != sizeof ret_addr){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_13, 
				"0452-081: Cannot read return address at address 0x%8x.\n") ,old_frame_ptr+2*RET_OFFSET);
				return (-1);
			}
		}
		if ((ret_addr >> SEGSHIFT) != KERNELSEG) {
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_14, "IAR not in kernel segment.\n") );
			return(-1);
		}
		sp = search(ret_addr,symnamebuf,!MATCHNAME);
		if(sp == 0)
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_15, "\n\t**** no match ****\n") );
		else if((int)sp == -1) /* error */
			return(-1);
		if (strcmp(sp->n_name,"endcomm") == 0)
			printf("\n\t**** %8.8x ****\n", ret_addr);
		else if(sp->n_zeroes) {
			printf("\n\t**** %s ****\n", sp->n_name);
		}
		else {		/* flexname */
			printf("\n\t**** %s ****\n", symnamebuf);
		}
	}
	printf("\n");
	printf("\n");
	return(0);
}

#define PRINTABLE(x) ((((x) >= 0x20) && ((x) <= 0x7e))? (x) : '.')
#define LINESIZE     36
#define ASCIISIZE    16

/* 
 * NAME: xdump
 *
 * FUNCTION:
 * 	dumps a structure (not formatted)
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

static 
xdump (osaddr, count, segment, thread_slot)
char *osaddr;
int count;
ulong segment;
int thread_slot;
{

    register int i, j;
    register int c;
    char null_line[] = "                                    ";
    char null_ascii[] = "                ";
    char *lineptr, linebuf[LINESIZE+1];
    char *asciiptr, asciibuf[ASCIISIZE+1];
    char prevbuf[LINESIZE+1];
    char readbuf[READSIZ];
    char *saddr;
    int asterisk = 0;
    int curcnt = 0;
    int readcnt;


    prevbuf[0] = '\0';
    osaddr = (char *)((int)osaddr & 0xfffffffc);
    for (i = 0; i < count; i += 4, lineptr += 9) {
	if (i == curcnt) {
    		saddr = readbuf;
		readcnt = (curcnt + READSIZ <= count)? READSIZ : count-curcnt;
		if (dumpflag) {
			if (readmem_thread(readbuf, osaddr, readcnt, thread_slot) 
			     != readcnt){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_18, 
				"0452-084: Cannot read frame pointer at address 0x%8x.\n") ,osaddr);
				return (-1);
			}
			if (*(ulong *)readbuf == 0xbad)
				return(-1);
		}
		else {
			lseek(mem, osaddr, 0);
			if (readx(mem, readbuf, readcnt, segment) != readcnt){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_19, 
				"0452-085: Cannot read frame pointer at address 0x%8x.\n") ,osaddr);
				return (-1);
			}
		}
		curcnt += readcnt;
	}
    	if (!(i % 16)) {
	    strcpy(linebuf,null_line);
	    strcpy(asciibuf,null_ascii);
    	    lineptr = linebuf;
  	    asciiptr = asciibuf;
    	}
	sprintf(lineptr," %8.8X",*(ulong *)(saddr));
	for (j=0; j<4; j++) {
	    c = *(saddr++) & 0xFF;
	    *(asciiptr++) = PRINTABLE(c);
	}
	if ((i % 16) == 12) {
	    if (strcmp (linebuf, prevbuf) == 0) {
	        if (!asterisk) {
		    asterisk++;
		    (void) printf ("*\n");
	        }
	    }
	    else {
	    	(void) printf ("%8.8x %s    %s\n",osaddr, linebuf, asciibuf);
	        asterisk = 0;
	        strcpy (prevbuf, linebuf);
	    }
	    osaddr += 16;

        }
    }
    if (i % 16) {
	if (strcmp (linebuf, prevbuf) == 0) {
	    if (!asterisk)
		(void) printf ("*\n");
	}
	else
	    *lineptr = ' ';
	    (void) printf ("%8.8x %s    %s\n",osaddr, linebuf, asciibuf);
    }
}
