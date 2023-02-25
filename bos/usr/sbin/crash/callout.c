static char sccsid[] = "@(#)41  1.17  src/bos/usr/sbin/crash/callout.c, cmdcrash, bos411, 9437B411a 9/14/94 20:10:22";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prcallout
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
 *
 */

/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

/* (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */


/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include	"crash.h"
#include 	<sys/timer.h>	/* trb maintenance declarations	*/
#include    <sys/ppda.h>

#define PPDSIZE sizeof(struct ppda)

extern cpu_t ncpus;

prcallout()
{

	struct ppda ppda;
	struct	trb	*tmptrb, trb;
	int current;
	int i, ret;

	for (i=0;i<ncpus;i++) {

		printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_1, "\nTRB's On The Active List Of Processor %d.\n\n"), i);

		if ((ret = readmem(&ppda, SYM_VALUE(Ppd)+ i * PPDSIZE, PPDSIZE, CURPROC)) != PPDSIZE) {
			printf(catgets(scmc_catd, MS_crsh, CALLOUT_MSG_2, 
			"0452-901: Cannot read ppda structure at address 0x%08x\n"), (SYM_VALUE(Ppd)+i*PPDSIZE));
			return;
		}
	
		if (ppda._csa == NULL) {
			printf(catgets(scmc_catd, MS_crsh, CALLOUT_MSG_3, "0452-553: This processor is disabled\n"));
			return;
		}

		if((tmptrb = ppda.ppda_timer.t_active) == NULL) {
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_4, 
			"0452-554: There are no TRB's on the active list of this processor.\n") );
			return;
		}

		current=1;
		while (tmptrb != NULL)  {
			if(readmem(&trb, tmptrb, sizeof(struct trb), CURPROC) 
			!= sizeof(struct trb)){
				   printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_5, 
				   "0452-555: Cannot read trb structure at address 0x%8x\n") ,tmptrb);
				   return;
			   }
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_6,
					"TRB #%d on Active List\n\n"), current);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_7,
				   " Timer address (trb)................0x%x\n"), tmptrb);
			printf(" trb.knext..........................0x%x\n", trb.knext);
			printf(" trb.kprev..........................0x%x\n", trb.kprev);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_8,
				   " Thread id (-1 for dev drv).........0x%x\n"), trb.id);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_9, 
				   " Timer flags........................0x%x\n"), trb.flags);
			printf(" trb.timerid........................0x%x\n", 
				   trb.timerid);
			printf(" trb.eventlist......................0x%x\n", 
				   trb.eventlist);
			printf(" trb.timeout.it_interval.tv_sec.....0x%x\n", 
				   trb.timeout.it_interval.tv_sec);
			printf(" trb.timeout.it_interval.tv_nsec....0x%x\n", 
				   trb.timeout.it_interval.tv_nsec);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_10,
				   " Next scheduled timeout (secs)......0x%x\n") , 
				   trb.timeout.it_value.tv_sec);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_11,
				   " Next scheduled timeout (nanosecs)..0x%x\n"), 
				   trb.timeout.it_value.tv_nsec);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_12,
				   " Timeout function...................0x%x\n"), trb.func);
			printf( catgets(scmc_catd, MS_crsh, CALLOUT_MSG_13,
				   " Timeout function data..............0x%x\n"), trb.func_data);
			printf("\n");
			current++;
			tmptrb = trb.knext;
		}
	}
}
