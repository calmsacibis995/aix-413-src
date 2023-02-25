static char sccsid[] = "@(#)85  1.1  src/rspc/kernext/pm/pmmi/event.c, pwrsysx, rspc411, 9435A411a 8/29/94 18:36:32";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: pm_event_query1   (add suffix_1 for phase-1 only)
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sleep.h>
#include <sys/lock_def.h>

#include <sys/pm.h>

/* PM core data */
extern struct _pm_core_data pm_core_data;

extern int pm_set_event(int);

/* event_words for PM */
extern int pm_queried_event;

/*
 * NAME:  pm_event_query1   (add suffix 1 for phase-1 only)
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine provides the phase-1 feature of system call, 
 *	"pm_event_query".
 * 	<<<<<< This routine is only called for PM phase1. >>>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Caller doesn't have priviledge
 *      PM_SUCCESS - Successfully processed
 */
int
pm_event_query1(int *event, int *action)  /* Add suffix 1 for phase1 */
{
    char ep;

    if( suser( &ep ) == 0 ) {
	setuerror(ep);
	return PM_ERROR;
    }

    if( pm_core_data.system_event == PM_EVENT_NONE ) {
        e_sleep( &pm_queried_event, EVENT_SHORT );
    }

    simple_lock(&(pm_core_data.lock));
    *event = pm_core_data.system_event;
    *action = pm_core_data.system_next_state;
    pm_core_data.system_event = PM_EVENT_NONE;
    simple_unlock(&(pm_core_data.lock));

    return PM_SUCCESS;
}
