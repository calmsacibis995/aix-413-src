static char sccsid[] = "@(#)52  1.1  src/rspc/kernext/pm/pmmi/parameter.c, pwrsysx, rspc411, 9435A411a 8/29/94 18:34:16";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: pm_control_pamameter1,query_system_idle_timer,
 *              set_system_idle_timer,query_device_idle_timer,
 *              set_device_idle_timer,query_lid_close_action,
 *              set_lid_close_action,query_system_idle_action,
 *              set_system_idle_action,query_main_switch_action,
 *              set_main_switch_action,query_low_battery_action,
 *              set_low_battery_action,query_beep,set_beep,
 *              query_pm_dd_number,query_pm_dd_list,query_lid_state
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/user.h>
#include <sys/errno.h>
#include <sys/malloc.h>

#include <sys/pm.h>


/* various handle for PM from PMkernel and PMcore */
extern struct _pm_kernel_data   pm_kernel_data;

extern struct _pm_core_data   pm_core_data;

/*
 * NAME:  pm_control_parameter1
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_control_parameter system call.
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Succesfully processed
 *      PM_ERROR - Invalid ctrl argument or Unsuccessfully processed
 */
int
pm_control_parameter1(int ctrl, caddr_t arg)
{

    char ep;

    if( suser( &ep ) == 0 ) {		/* make sure super user */
	setuerror(ep);
	return PM_ERROR;
    }

    switch( ctrl ) {

      case PM_CTRL_QUERY_SYSTEM_IDLE_TIMER:
	return query_system_idle_timer(arg);
	break;

      case PM_CTRL_SET_SYSTEM_IDLE_TIMER:
	return set_system_idle_timer(arg);
	break;

      case PM_CTRL_QUERY_DEVICE_IDLE_TIMER:
	return query_device_idle_timer(arg);
	break;

      case PM_CTRL_SET_DEVICE_IDLE_TIMER:
	return set_device_idle_timer(arg);
	break;

      case PM_CTRL_QUERY_LID_CLOSE_ACTION:
	return query_lid_close_action(arg);
	break;

      case PM_CTRL_SET_LID_CLOSE_ACTION:
	return set_lid_close_action(arg);
	break;

      case PM_CTRL_QUERY_SYSTEM_IDLE_ACTION:
	return query_system_idle_action(arg);
	break;

      case PM_CTRL_SET_SYSTEM_IDLE_ACTION:
	return set_system_idle_action(arg);
	break;

      case PM_CTRL_QUERY_MAIN_SWITCH_ACTION:
	return query_main_switch_action(arg);
	break;

      case PM_CTRL_SET_MAIN_SWITCH_ACTION:
	return set_main_switch_action(arg);
	break;

      case PM_CTRL_QUERY_LOW_BATTERY_ACTION:
	return query_low_battery_action(arg);
	break;

      case PM_CTRL_SET_LOW_BATTERY_ACTION:
	return set_low_battery_action(arg);
	break;

      case PM_CTRL_QUERY_BEEP:
	return query_beep(arg);
	break;

      case PM_CTRL_SET_BEEP:
	return set_beep(arg);
	break;

      case PM_CTRL_QUERY_PM_DD_NUMBER:
	return query_pm_dd_number((int *)arg);
	break;

      case PM_CTRL_QUERY_PM_DD_LIST:
	return query_pm_dd_list(arg);
	break;

      case PM_CTRL_QUERY_LID_STATE:
	return query_lid_state(arg);
	break;

      default:
	return PM_ERROR;
	break;

    }
}


/*
 * NAME:  query_system_idle_timer
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_system_idle_timer(int *timer)
{
    *timer = pm_core_data.system_idle_time;
    return PM_SUCCESS;
}


/*
 * NAME:  set_system_idle_timer
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Caller doesn't have priveledge
 *      PM_SUCCESS - Successfully processed
 */
static int
set_system_idle_timer(int *timer)
{
    if( *timer < 0 ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }

    pm_core_data.system_idle_time = *timer;
    return PM_SUCCESS;
}


/*
 * NAME:  query_device_idle_timer
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_device_timer_struct - timer struct for PM aware DDs
 *      pm_handle - PM handle structure
 *      pm_kernel_data - PM data structure in kernel
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Caller doesn't have priveledge,or invalid argument
 *      PM_SUCCESS - Successfully processed
 */
static int
query_device_idle_timer(struct pm_device_timer_struct *devtimer)
{
    struct pm_handle *p = pm_kernel_data.handle_head;
    struct pm_device_timer_struct dt;

    if( p == NULL ) { 
	return PM_ERROR;
    }
    if( devtimer == NULL ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }

    if( copyin(devtimer, &dt, PM_DTSIZE) != 0 ) {
        return PM_ERROR;
    }

    while( p != NULL ) {
	if( p->devno == dt.devno ) {
	    break;
	}
	p = p->next1;
    }

    if( p == NULL ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }

    dt.device_idle_time = p->device_idle_time;
    dt.device_standby_time = p->device_standby_time;

    if( copyout(&dt, devtimer, PM_DTSIZE) != 0 ) {
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  set_device_idle_timer
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_device_timer_struct - timer struct for PM aware DDs
 *      pm_handle - PM handle structure
 *      pm_kernel_data - PM data structure in kernel
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Caller doesn't have priveledge,or invalid argument
 *      PM_SUCCESS - Successfully processed
 */
static int
set_device_idle_timer(struct pm_device_timer_struct *devtimer)
{
    struct pm_device_timer_struct dt;
    struct pm_handle *p = pm_kernel_data.handle_head;

    if( p == NULL ) {
	return PM_ERROR;
    }

    if( copyin(devtimer, &dt, PM_DTSIZE) != 0 ) {
        return PM_ERROR;
    }

    while( p != NULL ) {
	if( p->devno == dt.devno ) {
	    break;
	}
	p = p->next1;
    }

    if( p == NULL ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }

    if( dt.device_idle_time < -1 || dt.device_standby_time < -1 ) {
	setuerror(EINVAL);
        return PM_ERROR;
    }

    if( dt.device_idle_time >= 0  ) {
	p->device_idle_time = dt.device_idle_time;
    }
    if( dt.device_standby_time >= 0 ) {
	p->device_standby_time = dt.device_standby_time;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  query_lid_close_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_lid_close_action(int *action)
{
    *action = pm_core_data.lid_close_action;
    return PM_SUCCESS;
}


/*
 * NAME:  set_lid_close_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 *      PM_ERROR - Invalid argument
 */
static int
set_lid_close_action(int *action)
{
    if( *action != PM_NONE &&
       *action != PM_SYSTEM_STANDBY &&
       *action != PM_SYSTEM_SUSPEND &&
       *action != PM_SYSTEM_HIBERNATION &&
       *action != PM_SYSTEM_SHUTDOWN ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }
    pm_core_data.lid_close_action = *action;
    return PM_SUCCESS;
}


/*
 * NAME:  query_system_idle_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_system_idle_action(int *action)
{
    *action = pm_core_data.system_idle_action;
    return PM_SUCCESS;
}


/*
 * NAME:  set_system_idle_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 *      PM_ERROR - Invalid argument
 */
static int
set_system_idle_action(int *action)
{
    if( *action != PM_NONE &&
       *action != PM_SYSTEM_STANDBY &&
       *action != PM_SYSTEM_SUSPEND &&
       *action != PM_SYSTEM_HIBERNATION &&
       *action != PM_SYSTEM_SHUTDOWN ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }
    pm_core_data.system_idle_action = *action;
    return PM_SUCCESS;
}


/*
 * NAME:  query_main_switch_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_main_switch_action(int *action)
{
    *action = pm_core_data.main_switch_action;
    return PM_SUCCESS;
}


/*
 * NAME:  set_main_switch_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 *      PM_ERROR - Invalid argument
 */
static int
set_main_switch_action(int *action)
{
    if( *action != PM_NONE &&
       *action != PM_SYSTEM_SUSPEND &&
       *action != PM_SYSTEM_HIBERNATION &&
       *action != PM_SYSTEM_SHUTDOWN  ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }
    pm_core_data.main_switch_action = *action;
    return PM_SUCCESS;
}


/*
 * NAME:  query_low_battery_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_low_battery_action(int *action)
{
    *action = pm_core_data.low_battery_action;
    return PM_SUCCESS;
}


/*
 * NAME:  set_low_battery_action
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Invalid argument
 *      PM_SUCCESS - Successfully processed
 */
static int
set_low_battery_action(int *action)
{
    if( *action != PM_NONE &&
       *action != PM_SYSTEM_SUSPEND &&
       *action != PM_SYSTEM_HIBERNATION &&
       *action != PM_SYSTEM_SHUTDOWN ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }
    pm_core_data.low_battery_action = *action;
    return PM_SUCCESS;
}


/*
 * NAME:  query_beep
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_beep(int *on)
{
    *on = pm_core_data.beep;
    return PM_SUCCESS;
}


/*
 * NAME:  set_beep
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Invalid argument
 *      PM_SUCCESS - Successfully processed
 */
static int
set_beep(int *on)
{

    if( *on != PM_BEEP_ON && *on != PM_BEEP_OFF ) {
	setuerror(EINVAL);
	return PM_ERROR;
    }
    pm_core_data.beep = *on;
    return PM_SUCCESS;
}


/*
 * NAME:  query_pm_dd_number
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_handle - PM handle structure
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 *
 */
static int
query_pm_dd_number(int *number)
{
    int pm_dd_number = 0;
    struct pm_handle *p = pm_kernel_data.handle_head;

    while( p != NULL ) {
	pm_dd_number++;
	p = p->next1;
    }

    *number = pm_dd_number;
    return PM_SUCCESS;
}


/*
 * NAME:  query_pm_dd_list
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_kernel_data - PM data structure in kernel
 *      pm_handle - PM handle structure
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - failed to call kernel service
 *      PM_SUCCESS - Successfully processed
 */
static int
query_pm_dd_list(dev_t *devno)
{
    struct pm_handle *p;
    int number;
    int count;
    dev_t *devno_list,*first;

    if( query_pm_dd_number(&number) != PM_SUCCESS )
	return PM_ERROR;

    if( number == 0) {
	devno = NULL;
        return PM_SUCCESS;
    }

    count = number * sizeof(dev_t *);

    first = devno_list = (dev_t *)xmalloc( count, PGSHIFT, kernel_heap);

    if( first == NULL ) {
	setuerror(ENOMEM);
	return PM_ERROR;
    }

    p = pm_kernel_data.handle_head;

    while( p != NULL ) {
	*devno_list = p->devno;
	p = p->next1;
	devno_list++;
    }

    if( copyout(first, devno, count) != 0 ) {
	xmfree(first, kernel_heap);
	return PM_ERROR;
    }

    xmfree(first, kernel_heap);
    return PM_SUCCESS;
}


/*
 * NAME:  query_lid_state
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 */
static int
query_lid_state(int *state)
{
    *state = pm_core_data.lid_state;
    return PM_SUCCESS;
}
