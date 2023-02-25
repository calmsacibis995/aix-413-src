 static char sccsid[] = "@(#)52  1.8  src/bos/usr/sbin/crash/POWER/chngmap.c, cmdcrash, bos411, 9428A410j 10/15/93 10:13:29";
/*
*/
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: chngmap,verify
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
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */


#include	"crash.h"
#include	<sys/proc.h>
#include	<sys/seg.h>
#include	<sys/thread.h>

extern struct uthread x_thread;
extern int errno;
extern int dumpflag;

#define SREG	x_thread.ut_save.as.srval

/* Change current map for od 
 */
struct cm *
chngmap(cm)
struct cm *cm;
{
	char	ch, *cp;
	int	i, ret;
	int	c, seg;
	ulong	u_segval;
	struct thread *t, thread;

	c = cm->cm_thread;
	seg = cm->cm_reg;

	if(c == -1)
		return(NULL);
	if(c > (int)v.ve_thread) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_1, 
		"0452-500: Thread slot %d above maximum limit.Specify a value less than %d.\n") ,c,v.ve_thread);
		return(NULL);
	}

	if(seg >= 16) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_4, 
		"0452-503: Segment %d above maximum limit.\n\tSpecify a value that is greater than 0 and less than 16\n"),
		seg);
		return(NULL);
	}

	/* read thread structure */
	if (read_thread(c,&thread)!= sizeof(struct thread))
		return NULL;
	t = &thread;
	if(t->t_state == TSNONE || t->t_state == TSZOMB) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_2, 
		"0452-501: Thread Table Slot %d not in use or zombie.\n"), c);
		return(NULL);
	}
	if((ret=getuarea_thread(c,&u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_3, 
		"0452-502: Cannot read uarea for this thread."));
		return(NULL);
	}
		
	if((cm->cm_segid = SREG[seg]) == INVLSID) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_5, 
		"0452-504: Segment %d not in use by thread %d\n") , seg, c);
		return(NULL);
	}
	cm->cm_tid = t->t_tid;
	return(cm);
}

verify(cmp)
struct cm *cmp;
{
	/* Verify that the tid in the current map is still using
	 * the segid, best you can do
	 */
	int c, seg, ret;
	ulong	u_segval;
	struct thread thread, *t;

	c = cmp->cm_thread;
	seg = cmp->cm_reg;

	if (read_thread(c, &thread) != sizeof(struct thread)) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_6, 
		"0452-505: Cannot read thread table entry %3d.\n") , c);
		return(0);
	}

	if((ret=getuarea_thread(c,&u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, CHNGMAP_MSG_7,
		"0452-506: Cannot read uarea for thread table entry %3d.\n") , c);
		return(0);
	}
	t = &thread;

	return((t->t_tid != cmp->cm_tid || 
		t->t_state == TSNONE || 
		t->t_state == TSZOMB || 
		cmp->cm_segid != SREG[seg]) ? 0 : 1);
}
