static char sccsid[] = "@(#)07  1.5  src/rspc/kernext/pm/pmmi/pmmiinit.c, pwrsysx, rspc41J 6/7/95 23:36:59";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: int pm_core_first_init();
 *		int pm_core_second_init();
 *		void unconfig_pmmi_core();
 *
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

#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/intr.h>

#include <sys/pm.h>
#include "pmmi.h"

/*------------------------*/
/*  PM core data 	  */
/*------------------------*/
extern	pm_control_data_t pm_control_data;
extern	pm_parameters_t	pm_parameters;
extern	pmmd_IF_t	pmmd_IF;
extern 	event_query_control_t	event_query_control;
extern  transit_lock_t	transit_lock;
extern  pm_machine_profile_t 	pm_machine_profile;


/*------------------------*/
/* external functions     */ 
/*------------------------*/
extern  void init_event_queue();
extern 	void receive_ext_event();
extern  int  start_hibernation();
extern  int  pm_kproc();
extern void update_BEEP_setting();
extern void GetandAnalizeResidualData(pm_machine_profile_t *arg);

/*------------------------*/
/* Proto type declaration */ 
/*------------------------*/
int pm_core_first_init();
void pm_core_second_init();
void establish_PMMD_Interface();
void unconfig_pmmi_core();

/*---------------------------------*/
/* For pmjob_on_INTLEVEL           */
/*---------------------------------*/
extern struct _pm_off_level_data {
	struct intr		pm_off_level;
	int			off_level_done;
	Simple_lock		lock_for_off_level;
	int			pm_off_level_event;
	int			state;
} pm_off_level_data;



/*
 * NAME: pm_core_first_init
 *  	
 * FUNCTION: PMMI init routine for PM phase2 which is called 
 *           in PM config. 
 *
 * NOTES:  
 *
 * DATA STRUCTURES:
 */
int
pm_core_first_init()
{
	pm_control_data.pmd_type = NO_PMD;
	pm_control_data.former_xmalloc_ptr = NULL;
	pm_control_data.bd_xmalloc_ptr = NULL;
	pm_control_data.xpm_sector = NULL;

	lock_alloc(&(pm_parameters.lock), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(pm_parameters.lock));

	lock_alloc(&(pmmd_IF.lock), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(pmmd_IF.lock));

	lock_alloc(&(event_query_control.lock), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(event_query_control.lock));

	lock_alloc(&(transit_lock.lock), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(transit_lock.lock));

	lock_alloc(&(pm_off_level_data.lock_for_off_level), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(pm_off_level_data.lock_for_off_level));

	(void)init_event_queue();

	/* save info of PCI devieces which ROS doesn't disable */
	(void)save_pcidev();

	return (PM_SUCCESS);
}


/*
 * NAME: pm_core_second_init
 *  	
 * FUNCTION:
 *	  Setup the interface from PMMD to PMMI. 
 *	  (The path from PMMI to PMMD has already been established 
 *        when PMMD is configured.)
 *
 * NOTES: 
 * 	  PMMI is not enabled until PM Daemon first calls             
 *	  "pm_system_event_query" function. PM Daemon is expected to
 *        calls the system call after user's programmable various PM 
 *	  parameter are obtained from ODM and informed PMMD of by 
 *        itself. 
 *	  When this routine is called by PMD, the following are done.
 *
 *   	  	- Callback address are sent to PMMD.   
 * 	  	- PM Kernel process are created.
 *
 *	  This routine must not be called twice.
 *
 * DATA STRUCTURES:
 */
void
pm_core_second_init()
{
    	pid_t pmkprocpid;

	(void)establish_PMMD_Interface();	
	/*
	    Before here, 
	       	Already connected:  PMMI ----->> PMMD  
	       	Not yet:	    PMMI <<----- PMMD

	    So all of external events were ignored if happened. 
	*/

        /*
         *  Initialize PM kernel process
         */
        pmkprocpid = creatp();
        initp(pmkprocpid, pm_kproc, 0, 0, "PM");
	delay(HZ/100);

	update_BEEP_setting();
	GetandAnalizeResidualData(&pm_machine_profile);

	return;
}


/*
 * NAME: establish_PMMD_Interface
 *  	
 * FUNCTION:  Setup the interface from PMMD to PMMI. 
 *	       (The path from PMMI to PMMD has already been established 
 *       	when PMMD is configured.)
 *
 * NOTES: Some callback address of PMMI are informed to PMMD so that
 *	  PMMD can notify PMMI of various external events. 
 *
 * DATA STRUCTURES:
 *      *event - points to the data which contains a new external event. 
 */
void
establish_PMMD_Interface()
{
        callback_entry_block_t   p;		

	p.pmmi_event_receive_entry = (int (*)())receive_ext_event;
	p.hibernation_entry = (int (*)())start_hibernation;
#ifdef PM_DEBUG
	printf ("hibernation entry address = %08X\n", 
				(int (*)())start_hibernation);
#endif
	CALL_PMMD(set_pmmi_callback, &p);
	return;
}



/*
 * NAME: Unconfig_pmmi_core 
 *  	
 * FUNCTION: Unconfigure PM core for PM phase2 
 *
 * NOTES:
 *      This routine is called when PM code is unconfigured. 
 *
 * DATA STRUCTURES:
 */
void 
unconfig_pmmi_core()
{
	lock_free(&(pm_parameters.lock));
	lock_free(&(pmmd_IF.lock));
	lock_free(&(event_query_control.lock));
	lock_free(&(transit_lock.lock));
	lock_free(&(pm_off_level_data.lock_for_off_level));

	if (pm_control_data.former_xmalloc_ptr != NULL) {
	  	(void)xmfree(pm_control_data.former_xmalloc_ptr, kernel_heap);
		pm_control_data.former_xmalloc_ptr = NULL;
   	} /* endif */

   	if (pm_control_data.bd_xmalloc_ptr != NULL) {
   		(void)xmfree(pm_control_data.bd_xmalloc_ptr, pinned_heap);
   		pm_control_data.bd_xmalloc_ptr = NULL;
   	} /* endif */

	if (pm_control_data.xpm_sector != NULL) {
		(void)xmfree(pm_control_data.xpm_sector,pinned_heap);
		pm_control_data.xpm_sector = NULL;
   	} /* endif */

	/*
 	 *  calling for fail safe
	 */
	term_hibernation();	
	return;
}


