static char sccsid[] = "@(#)84  1.7  src/rspc/kernext/pm/pmmi/coreinit.c, pwrsysx, rspc41J, 9520A_all 5/10/95 11:51:04";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: pm_core_init
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
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <procinfo.h>

#include <sys/pm.h>
#include "pmmi.h"

/*--------------------*/
/*   PM core data     */
/*--------------------*/
extern struct _pm_core_data	pm_core_data;
extern  pm_parameters_t		pm_parameters;
extern	pm_control_data_t	pm_control_data;
extern	pmmd_IF_t		pmmd_IF;
extern	core_data_t		core_data;
extern  int			pm_kflag;
extern	transit_lock_t		transit_lock;
extern  event_query_control_t	event_query_control;
extern 	int 			pm_term; 

/*--------------------*/
/* external functions */
/*--------------------*/
extern int	pm_kproc();
extern int	state_transition();
extern int 	pm_core_first_init();	
extern int	unconfig_pmmi_core();
extern void 	insert_new_event_to_Q(int);



/*
 * NAME:  pm_core_init
 *
 * FUNCTION:  initialize PM core data and creates PM kproc.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	This routine is entry point to configure PM machine independent
 *	part.
 *
 * DATA STRUCTURES:
 *      _pm_core_data - PM Machine independet data structure
 *      uio - user i/o area structure
 *
 * RETURN VALUE DESCRIPTION:	
 *	0  : PM initialization success.
 *	-1 : error.
 */
int
pm_core_init(int cmd, struct uio *uiop)
{
    struct _pm_core_data pmcore_pas;
    pid_t pmkprocpid;
    pm_system_state_t state_data;
    int  pmd_already_killed;
    int  i;

    DBG(PMMI init start)
    switch( cmd ) {
      case PM_CONFIG:
	if( pincode(state_transition) != 0 ) {
		return -1;
	}

	pm_control_data.pmmi_init_comp = FALSE;

	(void)pm_core_first_init();	

	/* The following lock is for phase-1 backward compatibility */
	lock_alloc(&(pm_core_data.lock), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(pm_core_data.lock));

	DBG(PMMI init ends)
	return(PM_SUCCESS);


      case PM_UNCONFIG:
	/* 
	 * Unconfiguration was not formally supported in PM phase_1(AIX4.1.2)
	 */
	DBG(PM core(PMMI) Unconfiguration entry)

	/* 
	 * Freeze any other state transition or external event handling
	 */
	simple_lock(&(transit_lock.lock));
	transit_lock.on_going = TRUE;      /* Set during state transition */
        pm_kflag = FALSE;                  /* Make pm_kproc terminate */


	/* Flush event queue */
	(void)init_event_queue();


	/* 
	 * wait pm daemon to call "pm_system_event_query" syscall. 
	 */
        pmd_already_killed = FALSE;
        for (i=0; !(event_query_control.now_queried); i++) {
                if (i > 6) {
                        pmd_already_killed = TRUE;
                        break;
                } /* endif */
                delay(HZ/2);
        } /* endfor */

	DBG(Complete to confirm PMD has queried with pm_system_event_query.)


	/* 
	 * Now, we are going to transit the system full_on state 
	 * All the PM aware DDs are set to full_on state.
	 */
	state_data.state = PM_SYSTEM_FULL_ON;	
	(void)state_transition(&state_data);  


        if (pmd_already_killed != TRUE) {
        /*
         * Inform PM daemon of PM core unconfiguration
         */
	simple_lock(&(event_query_control.lock));
        (void)insert_new_event_to_Q(PM_EVENT_TERMINATE);
	simple_unlock(&(event_query_control.lock));

        /*
         * Already confirmed if PM daemon already calls
         * PM core with pm_system_event_query syscal.
         */
        e_wakeup(&event_query_control.event_wait);

	/*
	 *  wait pm daemon to get PM_EVENT_TERMINATE. 
	 */ 
        while (event_query_control.in_sleep) { 
		delay(HZ/2);	
	} /* endwhile */
	DBG(Confirmed that PMD has just got the EVENT_TERMINATE)

        } /* endif of pmd_already_killed */


	/* Unfreeze event occurrence. But the flag of "transit_lock.on_going"
	 * is still on in this case. So any of event/syscall are not 
	 * handled any more.
	 */
	simple_unlock(&(transit_lock.lock));

	/* 
	 * Try to terminate PM kproc loop   
	 */
	if (pm_control_data.pmmi_init_comp) {
		DBG(Try to terminate pm-kproc)
		/* 
		 * At exit of the loop, pmkproc sets this flag again 
		 * to signal that it has been out of the loop. 
		 */
		while(!pm_term) {	
	   		delay(HZ/2);
		} /* endif */
		DBG(Completion of pm kproc exit)

	} /* endif */	

	lock_free(&(pm_core_data.lock));
	unconfig_pmmi_core();
	DBG(Completion of unconfig_pmmi_core)
	unpincode(state_transition);
	DBG(PMMI init ends)
	return 0;
    }

    return -1;
}

