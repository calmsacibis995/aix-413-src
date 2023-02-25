static char sccsid[] = "@(#)08  1.12  src/rspc/kernext/pm/pmmi/pmsyscall.c, pwrsysx, rspc41J, 9524E_all 6/14/95 11:54:04";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: 
 *         PM_SYSTEM_CALL: 
 *		pm_control_state
 *              pm_control_parameter
 *		pm_system_event_query
 *              pm_event_query
 *              pm_battery_control
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

#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/time.h>           /* for the timeval structure            */


#include <sys/pm.h>
#include <sys/pmdev.h>
#include "pmmi.h"


/*------------------------*/
/* External data symbols  */
/*------------------------*/
extern struct _pm_kernel_data   pm_kernel_data;
extern pm_parameters_t pm_parameters;
extern pm_control_data_t pm_control_data;
extern pmmd_IF_t pmmd_IF;
extern struct _pm_core_data pm_core_data;
extern event_query_control_t event_query_control;
extern pm_hibernation_t pm_hibernation;
extern pm_sector_t	pm_sector;
extern transit_lock_t	transit_lock;
extern pm_machine_profile_t pm_machine_profile;
extern int pm_kflag; 
extern pm_battery_data_t  battery_status;
extern battery_status_ready_t battery_status_ready;


/*-------------------------*/
/*external procedure symbol*/
/*-------------------------*/
extern int pm_control_state1(int, caddr_t); 
extern int pm_event_query1(int *event, int *action); 
extern void state_transition(pm_system_state_t *state_control_block);
extern int dump_mem(int *);
extern void pm_core_second_init();
extern int check_event_occurrence(ext_event_q_t *arg);
extern int pm_control_parameter1(int ctrl, caddr_t arg);
extern void update_BEEP_setting();
extern int ScanPMawareDD(dev_t *arg);
extern int  get_NumberOfAwareDD(); 
extern void establish_PMMD_Interface();

/*-------------------------*/
/* function proto type for debug */
/*-------------------------*/
void Establish_PMMD_Interface();
void transfer_DDnames(caddr_t buffer_addr);
void cvtlong2asc(ulong d,char *p);
void maintain_phase1_core_data();
int  battery_discharge();
int  battery_query(pm_battery_data_t *battery);


/*-------------------------------------------------------------------------*/
/*  	(((((  Initialization between PM core and PM daemon )))))          */ 
/*-------------------------------------------------------------------------*/
/*
 *   There is a certain assumption between PM core and PM daemon at         
 *  the initialization sequence which occurs when PM daemon is loaded.      
 *									    
 *    The following steps are the fixed order of calling system call at     
 *   the init.								    
 * 									   
 *	1). Call "pm_control_parameter"  				   
 *			with ctrl = PM_CTRL_SET_PARAMETERS		  
 * 									
 *	2). Call "pm_control_parameter"	
 *			with ctrl = PM_CTRL_SET_HIBERNATION_VOLUME
 *
 *      3). Call "pm_system_event_query" 
 *
 *	4). Call "pm_control_state" 
 *  			with ctrl = PM_CTRL_START_SYSTEM_STATE for PM_ENABLE 
 *			  	      (if the default PM state is PM_ENABLE)
 *
 *     # Note that all of PM aware DDs can first be enabled at the above (4).
 *     # Also note that PM core doesn't complete its init until the above (3).
 *       In other words, although PM core is already loaded at its config
 *	 methods, it doesn't run at all until receiving the first pm_system_
 *       event_query from PM daemon.
 *---------------------------------------------------------------------------
 */

/*
 * NAME:  pm_control_parameter
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_control_parameter system call.
 *
 * DATA STRUCTURES:
 *      ctrl - Specifies a pm_control_parameter command.
 *		PM_CTRL_SET_PARAMETERS
 *		PM_CTRL_QUERY_DEVICE_NUMBER
 *		PM_CTRL_QUERY_DEVICE_LIST
 *	   	PM_CTRL_QUERY_DEVICE_INFO
 *		PM_CTRL_SET_DEVICE_INFO
 *		PM_CTRL_SET_HIBERNATION_VOLUME
 *
 *	arg  -  In case of PM_CTRL_SET_PARAMETERS
 *		Pointer to data structure whose data type is pm_parameters_t.
 *
 *		In case of PM_CTRL_QUERY_DEVICE_NUMBER
 *		Pointer to an integer where the nmb of PM aware DDs is returned
 *
 *		In case of PM_CTRL_QUERY_DEVICE_LIST
 *		Pointer to an array of all of device logical names
 *
 *	   	In case of PM_CTRL_QUERY_DEVICE_INFO & PM_CTRL_SET_DEVICE_INFO
 *		Pointer to data structure whose data type is pm_device_info_t
 *
 *		In case of PM_CTRL_SET_HIBERNATION_VOLUME
 *		Pointer to data structure whose data type is pm_hibernation_t 
 *				 
 * RETURN VALUE DESCRIPTION:
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Indicating an error condition. 
 *			Error code: EINVAL
 */
int
pm_control_parameter(int ctrl, caddr_t arg)
{
   int	rc;
   int	device_count;
   caddr_t  mybuffer;
   struct pm_handle *p;
   pm_device_info_t devinfo;
   pm_device_names_t pm_device_names, *ppm_device_names=&pm_device_names; 
   int	pmhibsect_size;

   DBG(Receiving PM_CTRL_SET_PARAMETERS)
   if (pm_kflag == FALSE) { /* Check if still loaded but already unconfigured */
	setuerror (EPERM);  /* pm_kflag is set TRUE at default(global data).  */
	return (PM_ERROR);
   } /* endif */
  
   if (suser (&rc) == 0) {	/* Make sure of super user */
	setuerror (rc);
	return (PM_ERROR);
   } /* endif */

   simple_lock(&(transit_lock.lock));

   if (!(pm_control_data.pmmi_init_comp)) {
   DBG(Second init start)
	(void)pm_core_second_init();	
	pm_control_data.pmmi_init_comp = TRUE;
   } /* only effective at first call. */ 

   switch (ctrl) {
   case PM_CTRL_SET_PARAMETERS:


	DBG(PM_CTRL_SET_PARAMETERS)
#ifdef PM_DEBUG 
	printf("pm_parameters structure address: %08X\n", &pm_parameters);
	printf("Current time(in second) = %08X\n", time);
	printf("reserved2 = %X\n", (pm_parameters.core_data.reserve));
#endif


	if (rc=copyin(arg, &pm_parameters,sizeof(pm_parameters_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR; 
	}

	maintain_phase1_core_data();

	if (pm_control_data.pmmi_init_comp) {
		/* Because PMMD interface may not be established yet. */
		update_BEEP_setting();

	} /* endif */

	break;


   case PM_CTRL_QUERY_DEVICE_NUMBER:
	DBG(PM_CTRL_QUERY_DEVICE_NUMBER)
	simple_lock(&(pm_kernel_data.lock));
	device_count = get_NumberOfAwareDD();
	simple_unlock(&(pm_kernel_data.lock));

#ifdef PM_DEBUG 
	printf("Number of PM aware DDs = %03X\n", device_count);
#endif

	if (rc=copyout(&device_count, arg, sizeof(&device_count)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	}
	break;


   case PM_CTRL_QUERY_DEVICE_LIST:
	DBG(PM_CTRL_QUERY_DEVICE_LIST)
	if (rc=copyin(arg, &pm_device_names, sizeof(pm_device_names_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR; 
	}
	simple_lock(&(pm_kernel_data.lock));
	if (!(device_count = get_NumberOfAwareDD())) { 
		/* If no DD, this syscall mustn't be called.*/
		simple_unlock(&(pm_kernel_data.lock));
   		simple_unlock(&(transit_lock.lock));
		setuerror(EINVAL);
		return PM_ERROR;	
	}
	if ((pm_device_names.names == NULL)  ||
	    (pm_device_names.number == NULL)) {
		DBG(PMD Error: Either names_pointer or names_number is NULL)
		simple_unlock(&(pm_kernel_data.lock));
   		simple_unlock(&(transit_lock.lock));
		setuerror(EINVAL);
		return PM_ERROR;
	}
	if (device_count > pm_device_names.number) {
		device_count = pm_device_names.number;
	} else {
		pm_device_names.number = device_count;
	}	

	if (pm_control_data.former_xmalloc_ptr != NULL) {
	  	(void)xmfree(pm_control_data.former_xmalloc_ptr, kernel_heap);
		pm_control_data.former_xmalloc_ptr = NULL;
	}

	/* 1 device name consists of 16 bytes */
	if(!(mybuffer = (caddr_t)xmalloc(device_count<<4, 2, kernel_heap))) {
		simple_unlock(&(pm_kernel_data.lock));
   		simple_unlock(&(transit_lock.lock));
		setuerror(ENOMEM);
		return PM_ERROR;   /* Not enough memory for device names */
	}

	pm_control_data.former_xmalloc_ptr = mybuffer;
	memset((void *)mybuffer, NULL, device_count<<4);
	(void)transfer_DDnames(mybuffer);

	simple_unlock(&(pm_kernel_data.lock));

	if ((rc=copyout(mybuffer, pm_device_names.names,device_count<<4) != 0)
					||
	    (rc=copyout(&pm_device_names,arg,sizeof(pm_device_names_t)) != 0)){
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	} /* endif */
	break;


   case PM_CTRL_QUERY_DEVICE_INFO:
	DBG(PM_CTRL_QUERY_DEVICE_INFO)
	/* First, get the device name of the specified PM aware DD */
	if (rc=copyin(arg, &devinfo,sizeof(pm_device_info_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	}

	simple_lock(&(pm_kernel_data.lock));
	for(p = pm_kernel_data.handle_head; p != NULL; p = p->next1) {
		if (strncmp(devinfo.name, p->device_logical_name, 0x10) == 0){
			devinfo.mode = p->mode;
			devinfo.attribute = p->attribute;
			devinfo.idle_time = p->device_idle_time;
			devinfo.standby_time = p->device_standby_time;
			devinfo.idle_time1 = p->device_idle_time1;
			devinfo.idle_time2 = p->device_idle_time2;
			break;
		} /* endif */ 
	} /* endfor */
	simple_unlock(&(pm_kernel_data.lock));

	if (p == NULL) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (EINVAL);
		return PM_ERROR;
	}
	if(rc=copyout(&devinfo,arg,sizeof(pm_device_info_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	}
#ifdef PM_DEBUG
 	printf("Device information structure address: %08X\n", &devinfo);
#endif
	break;


   case PM_CTRL_SET_DEVICE_INFO:
	if (rc=copyin(arg, &devinfo,sizeof(pm_device_info_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	}

	simple_lock(&(pm_kernel_data.lock));
	for(p = pm_kernel_data.handle_head; p != NULL; p = p->next1) {
		if (strncmp(devinfo.name, p->device_logical_name, 0x10) == 0){
			p->device_idle_time = devinfo.idle_time;
			p->device_standby_time = devinfo.standby_time;
			p->device_idle_time1 = devinfo.idle_time1;
			p->device_idle_time2 = devinfo.idle_time2;
			p->idle_counter = 0;	/* reinit again */
			break;
		} /* endif */ 
	} /* endfor */
	simple_unlock(&(pm_kernel_data.lock));

	if (p == NULL) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (EINVAL);
		return PM_ERROR;
	}
	if(rc=copyout(&devinfo,arg,sizeof(pm_device_info_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	}
	break;


   case PM_CTRL_SET_HIBERNATION_VOLUME:
       	if (!(arg)) {
		memset(&pm_hibernation,NULL,sizeof(pm_hibernation_t));
		pm_hibernation.slist = NULL; 
		break;
	} /* endif */
	if (rc=copyin(arg, &pm_hibernation,sizeof(pm_hibernation_t)) != 0) {
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;
	} /* endif */
	pmhibsect_size = ((pm_hibernation.snum)*(sizeof(pm_sector_t)));

	if (pm_control_data.xpm_sector != NULL) {
		(void)xmfree(pm_control_data.xpm_sector,pinned_heap);
		pm_control_data.xpm_sector = NULL;
	} /* endif */
	if (!(pm_control_data.xpm_sector 
		= (pm_sector_t *)xmalloc(pmhibsect_size, 2, pinned_heap))){
   		simple_unlock(&(transit_lock.lock));
		setuerror(ENOMEM);
		return PM_ERROR;

	} /* endif */
	if (rc=copyin((caddr_t *)pm_hibernation.slist,
		   (caddr_t *)pm_control_data.xpm_sector,
		   pmhibsect_size) != 0) {

		(void)xmfree(pm_control_data.xpm_sector,pinned_heap);
		pm_control_data.xpm_sector = NULL;
   		simple_unlock(&(transit_lock.lock));
		setuerror (rc);
		return PM_ERROR;

	} /* endif */ 

	pm_hibernation.slist = pm_control_data.xpm_sector;

#ifdef PM_DEBUG 
	printf("--Dump data of pm_hibernation volume information---\n");
	printf("pm_hibernation.devno:	%08X\n",pm_hibernation.devno);
	printf("pm_hibernation.ppnum: 	%08X\n",pm_hibernation.ppnum);
	printf("pm_hibernation.ppsize: 	%08X\n",pm_hibernation.ppsize);
	printf("pm_hibernation.snum: 	%08X\n",pm_hibernation.snum);
	printf("pm_hibernation.slist: 	%08X\n",pm_hibernation.slist);
	printf("------- Dump data of the first sector list --------\n");
	printf("(sector address, sector length): (%08X,  %08X)\n",
						 (pm_hibernation.slist)->start,
						 (pm_hibernation.slist)->size);
#endif
	break;


   default:
   	simple_unlock(&(transit_lock.lock));
	return (pm_control_parameter1(ctrl, arg));	
	/* For backward compatibility with 4.1.1 PM */

   } /* endswitch */

   DBG(Exit of pm_control_parameter) 
   simple_unlock(&(transit_lock.lock));
   return PM_SUCCESS;
}


/*
 * NAME:  transfer_DDnames
 *
 * FUNCTION:  Get all of the logical name of all of the PM aware DDs.
 *
 * INPUT:
 *	address to the buffer for storing device logical names of PM aware DDs
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 */
void
transfer_DDnames(caddr_t buffer_addr)
{
   caddr_t b;
   struct pm_handle *p;
   char phase1dd[] ="Phase1=";
   char ascdigits[]="        ";

   for (b = buffer_addr, p = pm_kernel_data.handle_head; 
	p != NULL; 
	p = p->next1) {

		if (p->pm_version == PM_PHASE1_SUPPORT_ONLY) {

			memcpy (b,phase1dd,0x07);
			cvtlong2asc((ulong)p->devno,ascdigits);
			memcpy (b+0x07,ascdigits,0x08);

		} else {			
			memcpy (b,p->device_logical_name,0x10);
		}
		b += 0x10;

   } /* endfor */
   return;
}



/*
 *  NAME: cvtlong2asc
 *	
 *  FUNCTION: Convert long to ascii 8bytes
 *    (A better library may exist. If so, it should be used.)
 *
 *  INPUT: ulong value to be converted
 *	   char pointer pointing to 8 char string area.
 *
 *  RETURN:
 *	none
 */
void
cvtlong2asc(ulong d,char *p) 
{
	int	t;
	ulong	input_d;
	ulong	working_d;
	uchar	working_c;

	input_d = d;
	for (p += 8, t=0; t < 8; t++) {

		working_d = input_d;
		working_d &=0x0000000f;
		working_c = (uchar)working_d;
 		if (working_c < 0x0a) {
			working_c += 0x30; 
		} else { 
			working_c = ((working_c - 0x0a) + 0x41);
		} 
		(*(--p)) = (char)working_c;
		input_d = input_d>>4;

	} /* endfor */

	return;

} /* cvtlong2asc */



/*
 * NAME:  pm_control_state
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_control_state system call.
 *
 * DATA STRUCTURES:
 *      ctrl - Specifies a pm_control_state command.
 *      arg - Points a pm_system_state structure to transfer data.
 *
 * RETURN VALUE DESCRIPTION:
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Caller doesn't have priveledge or invalid argument.
 */
int
pm_control_state(int ctrl, caddr_t arg)
{
   char	rc;
   dev_t phase1_devno;
   pm_system_state_t state_data;
   struct pm_state *pm_state_phase1 = (struct pm_state *)&(state_data.state);
   int     opri;


   DBG(Receiving PM_CONTROL_STATE system call)
   if (pm_kflag == FALSE) { /* Check if still loaded but already unconfigured */
	setuerror (EPERM);  /* pm_kflag is set TRUE at default(global data).  */
	return (PM_ERROR);
   } /* endif */
  
   if (suser (&rc) == 0) {	/* Make sure of super user */
	setuerror (rc);
	return (PM_ERROR);
   } /* endif */

   if (rc=copyin(arg, &state_data, sizeof(pm_system_state_t)) != 0) {
	setuerror (rc);
	return (PM_ERROR);
   } /* endif */

#ifdef PM_DEBUG 
   if (pm_control_data.phase_1_only == TRUE) {
     printf("Received 'pm_state' block: state = %X\n", pm_state_phase1->state);
     printf("                           id    = %X\n", pm_state_phase1->id); 
     printf("                           event = %D\n", pm_state_phase1->event);
     printf("                           devno = %X\n", pm_state_phase1->devno);
   } else {
     printf("Received 'pm_system_state': state = %X\n", state_data.state);
     printf("                            event = %D\n", state_data.event); 
     printf("                            dev_name = %s\n", state_data.name);
   } /* endif */
#endif 


   switch (ctrl) {
   case	PM_CTRL_QUERY_SYSTEM_STATE: 
        DBG(pmsyscall.c: PM_CTRL_QUERY_SYSTEM_STATE)
	pm_control_data.phase_1_only = FALSE;
	state_data.state = pm_control_data.system_state;
	break;
		

   case	PM_CTRL_START_SYSTEM_STATE:
     	DBG(pmsyscall.c: PM_CTRL_STATE_SYSTEM_STATE)
	pm_control_data.phase_1_only = FALSE;

	if ((state_data.state == PM_SYSTEM_SUSPEND) || 
	    (state_data.state == PM_SYSTEM_HIBERNATION)) {

		if (ScanPMawareDD(&phase1_devno) == PM_ERROR) {
			state_data.event = PM_EVENT_PHASE1_DD;
                	pm_control_data.last_resume_event = PM_EVENT_PHASE1_DD;
			break;
		} /* endif */
	
		if (!(
		     ((state_data.state == PM_SYSTEM_SUSPEND) &&  
		      (pm_machine_profile.suspend_support == SUPPORTED))
					 ||
		     ((state_data.state == PM_SYSTEM_HIBERNATION) &&
		      (pm_machine_profile.hibernation_support == SUPPORTED))
		     )) {
			state_data.event = PM_EVENT_NOT_SUPPORTED;
                       pm_control_data.last_resume_event=PM_EVENT_NOT_SUPPORTED;
			break;

		} /* endif */
	} /* endif */

	/* 
	 * Reject the further reentrant call 
	 */
	simple_lock(&(transit_lock.lock));
	if (transit_lock.on_going == TRUE) {
	    	/* If this flag is still on even after the above lock,	
		 * it means that the system is about to be unconfigured.
		 */
		simple_unlock(&(transit_lock.lock));
		setuerror (EPERM);  
		return (PM_ERROR);
	}
	/* Set during state transition */
	transit_lock.on_going = TRUE;       

	/* 
	* Now, we are going to transit the system state.
	 */
	simple_lock(&(pm_kernel_data.lock));
	(void)state_transition(&state_data);  
	simple_unlock(&(pm_kernel_data.lock));


        opri = disable_lock(INTMAX, &(event_query_control.lock));
        if ((event_query_control.now_queried) &&
	    (event_query_control.event_queue_begin)) {
		/* 
		 * If any other event would occur during on_going, 
		 * it is picked up from event queue here. 
		 */
                event_query_control.now_queried = FALSE;
                e_wakeup(&(event_query_control.event_wait));

        } /* endif */
        unlock_enable(opri, &(event_query_control.lock));

	/* Reset during state transition */
	transit_lock.on_going = FALSE;      
	simple_unlock(&(transit_lock.lock));

	break;


   case	PM_CTRL_START_STATE:
   case	PM_CTRL_REQUEST_STATE:
   case	PM_CTRL_QUERY_STATE:
   case	PM_CTRL_QUERY_REQUEST:
	DBG(Phase1 pm_control_state request is called)
	pm_control_data.phase_1_only = TRUE;
	/* 
	 * This flag, phase_1_only also contributes to decide the difference 
	 * of the size of the structure(pm_state/pm_system_state) of 
	 * pm_control_state system call. 
	 */

	switch (pm_control_data.pmd_type) {
	case NO_PMD:
		setuerror (EPERM);
		return PM_ERROR;

	case PHASE1_PMD:
		return(pm_control_state1(ctrl, arg));

	case PHASE2_PMD:
		switch (ctrl) {
   		case	PM_CTRL_START_STATE:
   		case	PM_CTRL_REQUEST_STATE:
			if ((pm_state_phase1->state == PM_SYSTEM_SUSPEND) || 
		            (pm_state_phase1->state == PM_SYSTEM_STANDBY)) {
				receive_ext_event(PM_EVENT_P1SWRQ_STANDBY); 

			} else if (pm_state_phase1->state 
						== PM_SYSTEM_HIBERNATION){
				receive_ext_event(PM_EVENT_P1SWRQ_SHUTDOWN); 

			} else if((pm_state_phase1->state == PM_SYSTEM_ENABLE)||
				(pm_state_phase1->state == PM_SYSTEM_FULL_ON)){

				transit_lock.on_going = TRUE;  
				(void)state_transition(&state_data);  
				transit_lock.on_going = FALSE;  

			} /* endif */
			break;

   		case	PM_CTRL_QUERY_STATE:
			pm_state_phase1->state = pm_control_data.system_state;
			pm_state_phase1->id    = 1;	/* dummy ID number */
			pm_state_phase1->event = PM_EVENT_NONE;
			pm_state_phase1->devno = 0;
			break;

   		case	PM_CTRL_QUERY_REQUEST:
			/* do nothing */
			break;
		} /* endswitch */ 
		break;
	} /* endswitch */ 
	break;

   default:
	setuerror (EINVAL);
	return (PM_ERROR);
		
   } /* endswitch */

   if (pm_control_data.phase_1_only == TRUE) {
   	if (rc = copyout(&state_data, arg, sizeof(struct pm_state)) != 0) {
		setuerror (rc);
		return (PM_ERROR);
   	} /* endif */
   } else {  
   	if (rc = copyout(&state_data, arg, sizeof(pm_system_state_t)) != 0) {
		setuerror (rc);
		return (PM_ERROR);
	} /* endif */
   } /* endif */

#ifdef PM_DEBUG
   if (pm_control_data.phase_1_only == TRUE) {
   printf("Returned 'pm_state' block: state = %X\n", pm_state_phase1->state);
   printf("                           id    = %X\n", pm_state_phase1->id); 
   printf("                           event = %D\n", pm_state_phase1->event);
   printf("                           devno = %X\n", pm_state_phase1->devno);
   } else {
   printf("Returned 'pm_system_state' : state = %X\n", state_data.state);
   printf("                             event = %D\n", state_data.event); 
   printf("                             dev_name = %s\n", state_data.name);
   } /* endif */
#endif 

   return (PM_SUCCESS);

} /* pm_control_state */



/*
 * NAME:  pm_system_event_query
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_system_event_query system call.
 *
 * DATA STRUCTURES:
 *      *event - points to the data which contains external event. 
 *
 * RETURN VALUE DESCRIPTION:
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Caller doesn't have priveledge or invalid argument.
 */
int
pm_system_event_query(int *event)
{
    int	   rc;
    ext_event_q_t event_data, *pevent_data=&event_data;

    DBG("pm_system_event_query" is called)
   if (pm_kflag == FALSE) { /* Check if still loaded but already unconfigured */
	setuerror (EPERM);  /* pm_kflag is set TRUE at default(global data).  */
	return (PM_ERROR);
   } /* endif */
  
    if (suser (&rc) == 0) {
	setuerror(rc); 		/* Only root user can use this syscall */
	return (PM_ERROR);

    } else {
    	pm_control_data.phase_1_only = FALSE; 
    	pm_control_data.pmd_type = PHASE2_PMD; 
        /* Because this sys-call is only supported by phase-2. */ 

	simple_lock(&(transit_lock.lock));
	if (transit_lock.on_going == TRUE) {
	    	/* If this flag is still on even after the above lock,	
		 * it means that the system is about to be unconfigured.
		 */
		simple_unlock(&(transit_lock.lock));
		setuerror (EPERM);  
		return (PM_ERROR);
	}
	simple_unlock(&(transit_lock.lock));

	/* Failsafe after loading pmd again after killing pmd forcedly*/
	if (event_query_control.in_sleep == TRUE) {

        	/* Flush event queue */
        	(void)init_event_queue();

		e_wakeup(&event_query_control.event_wait);

	} /* endif */

	event_query_control.now_queried = FALSE;
	DBG(Check existing event)
	if (check_event_occurrence(pevent_data) == PM_ERROR) { 

		DBG(Sleep due to no event now)

		event_query_control.now_queried = TRUE;/* small windows exist.*/

		event_query_control.in_sleep = TRUE;
		e_sleep (&event_query_control.event_wait, EVENT_SIGRET);
		event_query_control.in_sleep = FALSE;

		DBG(Wakeup due to a new event)
		/* 
		 * ".now_queried" flag is already set to FALSE 
	 	 * when e_wakeup is called in "receive_ext_event()".
		 */

		if (check_event_occurrence(pevent_data) == PM_ERROR) {
			pevent_data->event = PM_EVENT_NONE;		
		}
#ifdef PM_DEBUG
		printf("The new event is %D\n", pevent_data->event);
#endif

	} /* endif */

	if (rc=copyout(pevent_data, event, sizeof(int)) != 0) {
		setuerror (rc);
		return (PM_ERROR);
	} /* endif */

    } /* endif */

    return (PM_SUCCESS);

} /* pm_system_event_query */



/*
 * NAME:  pm_event_query
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_evebt_query system call.
 *	This system call is only for phase-1 feature.
 *
 * DATA STRUCTURES:
 *      *event - points to the data which contains a new external event. 
 *	*action -recommended action which already programmed through
 *               pmctrl and ODM(deafult).
 * ERROR CODE:
 *      EINVAL	Invalid argument
 *	EBUSY   Already another process is blocked for query
 *
 *
 * RETURN VALUE DESCRIPTION:
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Caller doesn't have priveledge or invalid argument.
 */
int 
pm_event_query(int *event, int *action)
{
    pm_system_state_t state_data;

    DBG("pm_event_query" system call is hooked correctly)

    pm_control_data.phase_1_only = TRUE; 
    pm_control_data.pmd_type = PHASE1_PMD; 

    	if (!(pm_control_data.pmmi_init_comp)) {
    		state_data.state = PM_SYSTEM_ENABLE;
    		(void)state_transition(&state_data);  
 		pm_control_data.pmmi_init_comp = TRUE;
		pm_control_data.system_state = PM_SYSTEM_ENABLE;
    	} /* only effective at first call. */ 

    	(void)establish_PMMD_Interface();  
    	/* 
     	* This is a tentative call because this should be done 
     	* in the second init which is only for phase2. However,
     	* for debug, this feature is desired to be tested with
     	* phase1 environment. 
     	*/

    return(pm_event_query1(event, action)); 

} /* pm_event_query */

	


/*
 * NAME:  maintain_phase1_data
 *
 * FUNCTION: To maintain the consistency between phase_2 control data and
 *	     the core data which is used for phase_1 operation only. 
 *
 * NOTES:
 *	This routine maintains the core data consistency for phase_1 PM.
 *	Olde type(phase_1) of pmctrl command queries the current control 
 *	parameters to pm core. Because in phase-1, such control parameters
 *      were actually maintained in pm core, but not in PM daemon differently
 * 	from phase_2 implementation. This routine is prepared for that query. 
 *
 * DATA STRUCTURES:
 *	pm_core_data
 *	pm_parameters
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 */
void 
maintain_phase1_core_data()
{
	simple_lock(&(pm_core_data.lock));	
	pm_core_data.system_idle_time =
		pm_parameters.core_data.system_idle_time;
	pm_core_data.system_idle_action =
		pm_parameters.daemon_data.system_idle_action;
	pm_core_data.lid_close_action =
		pm_parameters.daemon_data.lid_close_action;
	pm_core_data.lid_state = PM_LID_OPEN;

	pm_core_data.main_switch_action =
		pm_parameters.daemon_data.main_switch_action;
	pm_core_data.low_battery_action =
		pm_parameters.daemon_data.low_battery_action;
	pm_core_data.beep =
		pm_parameters.core_data.pm_beep;
	simple_unlock(&(pm_core_data.lock));	

	return;
}
	


/*
 * NAME:  pm_battery_control
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_control_parameter system call.
 * 	This function is not limited to root user differently from other pm syscall.
 *
 * DATA STRUCTURES:
 *        pm_battery - PM battery information structure
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Invalid cmd argument or falied to process
 *                   battery functions.
 */
int
pm_battery_control(int cmd, struct pm_battery *battery)
{
    char ep;
    int	rc;
    struct pm_battery tentative_status = {0,0,0,0,0,0};

    if (pm_kflag == FALSE) {/* Check if still loaded but already unconfigured */
	setuerror (EPERM);  /* pm_kflag is set TRUE at default(global data).  */
	return (PM_ERROR);
    } /* endif */
  	
    if (pm_machine_profile.battery_support == 0) {
	DBG(Battery feature is not supported)
	if (rc=copyout(&tentative_status, battery,
					sizeof(struct pm_battery)) != 0) {
		setuerror(rc);
		return (PM_ERROR);
	} else {
		return (PM_SUCCESS);
	} /* endif */
    } /* endif */

    if (battery_status_ready.available == FALSE) {
	if (e_sleep(&battery_status_ready.event_wait, EVENT_SIGRET) 
							== EVENT_SIG) {
		setuerror (EPERM);  
		return (PM_ERROR);
	}
    }

    simple_lock(&(transit_lock.lock));
    if (transit_lock.on_going == TRUE) {
    	/* 
	 * If this flag is still on even after the above lock,	
  	 * it means that the system is about to be unconfigured.
	 */
	simple_unlock(&(transit_lock.lock));
	setuerror (EPERM);  
	return (PM_ERROR);
    }
    simple_unlock(&(transit_lock.lock));

    DBG(Battery syscall is received)
    switch( cmd ) {
      case PM_BATTERY_DISCHARGE:
	return (battery_discharge());

      case PM_BATTERY_QUERY:
	/* 
	 * Getting from global data which is maintained asynchronously. 
	 */
	if (rc=copyout(&battery_status, battery, 	
			sizeof(struct pm_battery)) != 0) {
		setuerror(rc);
		return (PM_ERROR);
	} /* endif */

#ifdef PM_DEBUG
       printf ("Battery block data starting address = %08X\n", &battery_status);
#endif
	return (PM_SUCCESS);

      default:
	setuerror(EINVAL);
	return (PM_ERROR);

    } /* endswitch */
}/* pm_battery_control */



/*
 * NAME:  battery_discharge
 *
 * FUNCTION:  Request battery to be discharged for refreshment
 *
 * NOTE: This request immediately returns before the completion
 *	 of discharging battery. 
 *
 * DATA STRUCTURES:
 *	  none
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Falied to call battery status function.
 */
int
battery_discharge()
{
    battery_control_t 	battery_control;

    battery_control.control = discharge_battery;
    battery_control.battery_status = NULL;
    /* With Lock for being exclusive */
    CALL_PMMD(battery_job,&battery_control)   

    return PM_SUCCESS;    

} /* battery_discharge */



