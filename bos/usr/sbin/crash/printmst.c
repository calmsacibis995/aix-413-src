static char sccsid[] = "@(#)98	1.5  src/bos/usr/sbin/crash/printmst.c, cmdcrash, bos41B, 412_41B_sync 11/28/94 16:31:37";

/*
 * COMPONENT_NAME: (CMDCRASH) Crash
 *
 * FUNCTIONS:  prmst
 *
 * ORIGINS: 3, 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
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

#include        "crash.h"
#include 	<sys/proc.h>
#include 	<sys/user.h>
#include 	<sys/buf.h>
#include 	<sys/pseg.h> /* needed for pinned/paged constants */
#include    <stddef.h>
#include    <sys/ppda.h>
#define PPDSIZE sizeof(struct ppda)

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern int dumpflag;

#undef u_sfirst
#undef u_slast
#undef u_srw
#undef u_segbase
#undef u_segoffs
#undef u_ofirst
#undef u_olast
extern int errno;
extern cpu_t selected_cpu;

/*
 *  Function:	print mstsave areas 
 *
 *  Returns:	nothing.
*/
prmst(addr)
ulong addr;
{
	register int i;
	char *p;
	struct mstsave mst, *cur_mst, *x;

	if(addr)
		cur_mst = (struct mstsave *)addr;
	else {
		/*
		 * unwind mst stack starting at csa 
		 */
		if(!(cur_mst = (struct mstsave *)(SYM_VALUE(Ppd) + selected_cpu*PPDSIZE + offsetof(struct ppda,_csa)))) {
			printf(catgets(scmc_catd,MS_crsh, MST_MSG_8,"0452-850: Cannot read csa at 0x%08x \n"),
						(SYM_VALUE(Ppd)+selected_cpu*PPDSIZE+offsetof(struct ppda,_csa)));
			return;
		}
		/*
		 *  need a dereference
		 */
		if((readmem(&cur_mst,(long)(cur_mst), sizeof(cur_mst), 
	  	   CURPROC)) != sizeof(cur_mst)) {
	  	   printf(catgets(scmc_catd,MS_crsh,MST_MSG_1,"Cannot read csa at 0x%08x \n"),
			   cur_mst);
		   return;
		}
	}


    for( ; cur_mst; cur_mst=x->prev) {

	/* Read in the mst structure */
	if((readmem(&mst,(long)(cur_mst), sizeof(mst), CURPROC)) 
	    != sizeof(mst)){
		printf(catgets(scmc_catd,MS_crsh,MST_MSG_2,"Mst at 0x%08x paged out \n"),cur_mst);
		return;
	}
	if(mst.prev == 0xbad){
		printf(catgets(scmc_catd,MS_crsh,MST_MSG_3,"Cannot read mst at 0x%08x \n"),cur_mst);
		return;
	}

	x = &mst;
	printf(catgets(scmc_catd, MS_crsh, MST_MSG_9,
				   "                MSTSAVE AREA AT ADDRESS 0x%08x\n\n"), cur_mst);

	printf("    curid:0x%08x  m/q:0x%08x  iar:0x%08x  cr:0x%08x \n",
		x->curid,x->mq,x->iar, x->cr);
	printf("    msr:0x%08x    lr:0x%08x   xer:0x%08x  kjmpbuf:0x%08x\n",
		x->msr,x->lr,x->xer,x->kjmpbuf);
   	printf(
     "    backtrack:0x%02x    tid:0x%08x  fpeu:0x%02x       excp_type:0x%08x\n",
		x->backt, x->tid, x->fpeu, x->excp_type);
	printf("    ctr:0x%08x    \n", x->ctr);
   	printf("    *prevmst:0x%08x  *stackfix:0x%08x  intpri:0x%02x \n",
		 x->prev,x->stackfix, x->intpri);
   	printf("    o_iar:0x%08x  o_toc:0x%08x  o_arg1:0x%08x  excbranch:0x%08x\n",
		 x->o_iar,x->o_toc, x->o_arg1,x->excbranch);

	printf("    o_vaddr:0x%08x\n", x->o_vaddr);

	printf( catgets(scmc_catd, MS_crsh, MST_MSG_4, 
		"    Exception Struct\n") );
	printf("      0x%08x  0x%08x  0x%08x  0x%08x  0x%08x\n",
		x->except[0], x->except[1],
		x->except[2], x->except[3],x->except[4]);

	printf( catgets(scmc_catd, MS_crsh, MST_MSG_5, 
		"    Segment Regs\n    ") );
	for(i=0;i<NSRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->as.srval[i]);
		else
			printf("%d:0x%08x  ",i,x->as.srval[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	printf( catgets(scmc_catd, MS_crsh, MST_MSG_6, 
		"General Purpose Regs\n    ") );
	for(i=0;i<NGPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->gpr[i]);
		else
			printf("%d:0x%08x  ",i,x->gpr[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	printf("\n");

	printf( catgets(scmc_catd, MS_crsh, MST_MSG_7, 
		"    Floating Point Regs\n    ") );

	printf("    Fpscr: 0x%08x \n    ",x->fpscr);
	for(i=0;i<NFPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x 0x%08x ",i,x->fpr[i]);
		else
			printf("%d:0x%08x 0x%08x ",i,x->fpr[i]);
		if(!((i+1)%3))
			printf("\n    ");
	}
	if(i%4)		/* we didn't print \n and left margin in loop */
		printf("\n    ");

	printf("\n\n");
    } /* while (cur_mst) */
}


