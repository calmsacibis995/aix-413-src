static char sccsid[] = "@(#)04  1.6  src/rspc/kernext/pm/pmmi/pmevent.c, pwrsysx, rspc41J, 9520A_all 5/11/95 09:04:28";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: 
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
#include <sys/intr.h>
#include <sys/lock_def.h>
#include <procinfo.h>

#include <sys/pm.h>
#include "pmmi.h"


/*------------------------*/
/* External data symbols  */
/*------------------------*/
extern 	pm_control_data_t pm_control_data;
extern  pmmd_IF_t	pmmd_IF;
extern  event_query_control_t event_query_control;
extern 	transit_lock_t	transit_lock;
extern	pm_parameters_t	pm_parameters;

/*-------------------------*/
/* function proto type for debug */
/*-------------------------*/
void receive_ext_event(int event);
int check_event_occurrence(ext_event_q_t *arg);
void insert_new_event_to_Q(int);
int strategic_process(int);	
void remove_ineffective_event
		(ext_event_q_t *current_ptr,ext_event_q_t *former_ptr);
ext_event_q_t * get_empty_cell();
void put_empty_cell(ext_event_q_t *ptr);
void init_event_queue();
int convertPhase1event();
void ShutDownPreparation();


/*
 * NAME:  receive_ext_event
 *
 * FUNCTION:  Inform the external event detected asynchronously
 *
 * NOTES:
 *      This routine is called whenever external event asynchronously 
 *      occurs. The external events are:
 *          - Main power switch depress 
 *	    - LID close 
 *	    - Low battery notice
 * 	    - keyboard/mouse activity on standby mode 
 * 	    - AC/DC power source changing
 *	    - System idle detection 
 *	    - Extra input pin activity, etc.
 *
 *      Note that resume events are all processed in transit.c because
 *	they are detected in the pseudo seamless control flow though
 *      powering CPU itself off is inserted implicitly on the way of the
 *      flow. In other words, those events are returned from the low 
 *      level routine controlling the power-off.
 *
 * DATA STRUCTURES:
 *	N/A
 *
 * RETURN VALUE DESCRIPTION:
 *      none 
 *     
 */
void 
receive_ext_event(int event)
{
	int	cvt_event;
	int	opri;

	cvt_event = convertPhase1event(event);

#ifdef PM_DEBUG
	switch (event) {
	case PM_EVENT_LID_OPEN:
	DBG(Receiving PM event PM_EVENT_LID_OPEN)
	break;
	case PM_EVENT_LID_CLOSE:
	DBG(Receiving PM event PM_EVENT_LID_CLOSE)
	break;
	case PM_EVENT_MAIN_POWER:
	DBG(Receiving PM event PM_EVENT_MAIN_POWER)
	break;
	case PM_EVENT_PD_CLICK:
	DBG(Receiving PM event PM_EVENT_PD_CLICK)
	break;
	case PM_EVENT_KEYBOARD:          
	DBG(Receiving PM event PM_EVENT_KEYBOARD)
	break;
	case PM_EVENT_LOW_BATTERY:
	DBG(Receiving PM event PM_EVENT_LOW_BATTERY)
	break;
	case PM_EVENT_CRITICAL_LOW_BATTERY:
	DBG(Receiving PM event PM_EVENT_CRITICAL_LOW_BATTERY)
	break;
	case PM_EVENT_RING:
	DBG(Receiving PM event PM_EVENT_RING)
	break;
	case PM_EVENT_RESUME_TIMER:
	DBG(Receiving PM event PM_EVENT_RESUME_TIMER)
	break;
	case PM_EVENT_SYSTEM_IDLE_TIMER:
	DBG(Receiving PM event PM_EVENT_IDLE_TIMER)
	break;
	case PM_EVENT_SOFTWARE_REQUEST:
	DBG(Receiving PM event PM_EVENT_SOFTWARE_REQUEST)
	break;
	case PM_EVENT_SUSPPEND_TIMER:
	DBG(Receiving PM event PM_EVENT_SUSPEND_TIMER)
	break;
	case PM_EVENT_AC:
	DBG(Receiving PM event PM_EVENT_AC)
	break;
	case PM_EVENT_DC:                
	DBG(Receiving PM event PM_EVENT_DC)
	break;
	case PM_EVENT_RTC:
	DBG(Receiving PM event PM_EVENT_RTC)
	break;
	case PM_EVENT_MOUSE:
	DBG(Receiving PM event PM_EVENT_MOUSE)
	break;
	case PM_EVENT_EXTRA_INPUTDD: 
	DBG(Receiving PM event PM_EVENT_EXTRA_INPUTDD)
	break;
	case PM_EVENT_EXTRA_BUTTON:
	DBG(Receiving PM event PM_EVENT_EXTRA_BUTTON)
	break;
	case PM_EVENT_SPECIFIED_TIME:
	DBG(Receiving PM event PM_EVENT_SPECIFIED_TIME)
	break;
	case PM_EVENT_GRAPHICAL_INPUT:
	DBG(Receiving PM event PM_EVENT_GRAPHICAL_INPUT)
	break;
	case PM_EVENT_TERMINATE:
	DBG(Receiving PM event PM_EVENT_TERMINATE)
	break;
	case PM_EVENT_POWER_SWITCH_ON:
	DBG(Receiving PM event PM_EVENT_SWITCH_ON)
	break;
	case PM_EVENT_POWER_SWITCH_OFF:
	DBG(Receiving PM event PM_EVENT_SWITCH_OFF)
	break;
	case PM_EVENT_BATTERY_STATUS_RDY:
	DBG(Receiving PM event PM_EVENT_BATTERY_STATUS_RDY)
	break;
	} /* endswitch */
#endif

	if ((pm_control_data.system_state == PM_SYSTEM_FULL_ON) &&
	    (cvt_event != PM_EVENT_POWER_SWITCH_OFF) &&
	    (cvt_event != PM_EVENT_LOW_BATTERY)) { 
		return;
	} /* endif */


	/* Serialize asynchronous calling of this funtion:"receive_ext_event" 
  	 * because this function may be called asynchronously by pmmd
	 * external interrupt through PMMD kproc.
	 */
	DBG(Enter strategic_process())
	opri = disable_lock(INTMAX, &(event_query_control.lock));

	if (strategic_process(cvt_event)) { 

		(void)insert_new_event_to_Q(cvt_event);

		if (event_query_control.now_queried &&
	    			!(transit_lock.on_going)) {
			event_query_control.now_queried = FALSE;
			e_wakeup(&event_query_control.event_wait);	
		} /* endif */
	} /* endif */

	unlock_enable(opri, &(event_query_control.lock));


	return;

} /* receive_ext_event */


/*
 * NAME:  convertPhase1event
 *
 * FUNCTION:  conver the event if it's one of phase_1
 *
 * RETURN VALUE DESCRIPTION:
 *      converted new event ID 
 *     
 */
int
convertPhase1event(int event)
{
	int	cvt_event;

	cvt_event = event;
	if ((event == PM_EVENT_MAIN_POWER) ||
	    (event == PM_EVENT_POWER_SWITCH_OFF)) {
		if ((pm_control_data.system_state == PM_SYSTEM_STANDBY) ||
		    (pm_control_data.system_state == PM_SYSTEM_SUSPEND) ||
		    (pm_control_data.system_state == PM_SYSTEM_HIBERNATION)) {
			cvt_event = PM_EVENT_POWER_SWITCH_ON;  
		} else {	
			cvt_event = PM_EVENT_POWER_SWITCH_OFF;  
		} /* endif */
	} /* endif */

	return cvt_event;

} /* convertPhase1event */



/*
 * NAME:  insert_new_event_to_queue
 *
 * FUNCTION:  A new event is put into event queue
 *
 * RETURN VALUE DESCRIPTION:
 *      none 
 *     
 */
void 
insert_new_event_to_Q(event)
{
	int	t=0;
	ext_event_q_t	*p;
	ext_event_q_t	*q;

	if(!(p = get_empty_cell())) {
		if ((event == PM_EVENT_POWER_SWITCH_OFF) ||
		    (event == PM_EVENT_POWER_SWITCH_ON)) {
			CALL_PMMD_WO_LOCK(TurnPowerOff, NULL)		
			while (!t) {;}	/* wait here forever */
		} 
		DBG(A new PM event is ignored due to no empty cell)
		return;	/* Unless the event is main power off, it's ignored. */
	}
	p->event = event;
	p->next = NULL;		/* set a delimitter as the last event */

	if (event_query_control.event_queue_begin) {
 
		for (q = event_query_control.event_queue_begin; 
	     	     q->next != NULL;
	     	     q = q->next) {;
#ifdef PM_DEBUG
			printf("Queue DUMP: Queue address is %08X\n",q);
			printf("Queue DUMP: Event in Queue is %D\n",q->event);
#endif 
		} /* endfor */
		q->next = p;

	} else {

		event_query_control.event_queue_begin = p;

	} /* endif */

#ifdef PM_DEBUG
	printf("New Queue address is %08X\n",p);
	printf("New Event in Queue is %D\n",p->event);
#endif

	return;

} /* insert_new_event_to_Q */



/*
 * NAME:  get_empty_cell
 *
 * FUNCTION:  Get an empty cell of event queue (max. 16 cells)
 *
 * DATA STRUCTURES:
 *	ext_event_q_t
 *	event_query_control_t
 *
 * RETURN VALUE DESCRIPTION:
 *      If success ----- ext_event_q_t  pointer - pointing a new empty cell
 *	
 *      If error   ----- NULL   
 */
ext_event_q_t *
get_empty_cell()
{
	ext_event_q_t	*p;

	if (event_query_control.event_empty_queue_begin == NULL) {
		return NULL;
	}
	p = event_query_control.event_empty_queue_begin;
	event_query_control.event_empty_queue_begin = p->next;
	return p;
}


/*
 * NAME:  put_empty_cell
 *
 * FUNCTION:  Put an empty cell into event queue 
 *
 * DATA STRUCTURES:
 *	ext_event_q_t
 *	event_query_control_t
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 */
void
put_empty_cell(ext_event_q_t *ptr)
{
    ptr->next = event_query_control.event_empty_queue_begin;
    event_query_control.event_empty_queue_begin = ptr; 

    return;

} /* put_empty_cell */


/*
 * NAME:  init_event_queue
 *
 * FUNCTION:  Arrange the cells of event queue before the actual use
 *
 * DATA STRUCTURES:
 *	ext_event_q_t
 *	event_query_control_t
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 */
void
init_event_queue()
{
    int	t;
    ext_event_q_t *p;

    simple_lock(&(event_query_control.lock));

    event_query_control.event_queue_begin = NULL;

    event_query_control.event_empty_queue_begin = event_query_control.queue;

    for (p=event_query_control.event_empty_queue_begin, t=1 ; t <= 15; t++) {

    	p->next = &(event_query_control.queue[t]);
	p = p->next;

    } /* endfor */

    p->next = NULL;	/* Set a delimitter as the last one */

    simple_unlock(&(event_query_control.lock));

#ifdef PM_DEBUG
    printf ("Event queue = %08X\n",event_query_control.event_empty_queue_begin);
#endif

    return;

} /* init_event_queue */



/*
 * NAME:  strategic_process
 *
 * FUNCTION:  All of the unresolved events are arranged in event queue.
 *
 * NOTES:
 *      This routine is called whenever a new external event is put into
 *      the event queue. And if the event opposite to the new event is
 *	still unresolved, that event is removed with the new event. For
 *	instance, if LID close event is unresolved when LID open event 
 * 	occurs, those two events are removed as if they wouldn't occurred.
 *	Another example is "Low battery vs. AC power source connection".
 *      In addition, if there would be more than 5 main power switch off
 *	events in event queue, turning system off is unconditionally 
 *      performed for fail safe in "insert_new_event_in_Q()".
 *
 * DATA STRUCTURES:
 *	N/A
 *
 * RETURN VALUE DESCRIPTION:
 *	TRUE: the current event has put into the event queue.
 *	        (A former event may be removed in a certain case.)
 *	FALSE: the current event was removed with the opposite event.
 *	 	(So far, a new event is not removed.)
 *     
 */
int	
strategic_process(int event)
{
    int	i=0;
    ext_event_q_t	*p;
    ext_event_q_t	*q;

    /* Check if main power switch off fails to be a trouble */
    if ((event == PM_EVENT_POWER_SWITCH_OFF) 
			|| 
	(event == PM_EVENT_POWER_SWITCH_ON)) {

		if (check_main_power_event_in_Q()) 

    		{  /* If true, fail safe turn off.*/

			CALL_PMMD_WO_LOCK(TurnPowerOff, NULL)		
			while (!i) {;}	/* wait here forever */

    		} /* endif */
    } /* endif */
    		
    /* Check if both of LID open/close results in putting in queue. */
    if (event_query_control.event_queue_begin) {

    	for (p = event_query_control.event_queue_begin, q = NULL; 
     	     p != NULL;
     	     p = p->next, q = p) {

		switch (event) {
		case PM_EVENT_LID_OPEN:
			if (p->event == PM_EVENT_LID_CLOSE) {
				(void)remove_ineffective_event(p,q);	
		DBG(LID CLOSE event has been removed and LID OPEN event remains)
			}
			break;

		case PM_EVENT_LID_CLOSE:
			if (p->event == PM_EVENT_LID_OPEN) {
				(void)remove_ineffective_event(p,q);	
		DBG(LID OPEN event has been removed and LID CLOSE event remains)
			}
			break;

		case PM_EVENT_AC:
			if (p->event == PM_EVENT_LOW_BATTERY) { 
				(void)remove_ineffective_event(p,q);	
		DBG(LOW BATTERY event has just been removed)
			}
			if (p->event == PM_EVENT_DC) {
				(void)remove_ineffective_event(p,q);
		DBG(DC event has just been removed)
			}
			break; /* Here, PM_EVENT_AC is not removed. */

		case PM_EVENT_DC:
			if (p->event == PM_EVENT_AC) {
				(void)remove_ineffective_event(p,q);
		DBG(AC event has just been removed)
			}
			break; /* Here, PM_EVENT_DC is not removed. */

		} /* endswitch */
    	} /* endfor */
    } /* endif */

    return TRUE;

} /* strategic_process */



/*
 * NAME:  remove_ineffective_event
 *
 * FUNCTION: Remove a specified event from queue due to being ineffective.    
 *
 * RETURN VALUE DESCRIPTION:
 *	none	
 */
void
remove_ineffective_event(ext_event_q_t *current_ptr, ext_event_q_t *former_ptr)
{ 
   if (former_ptr == NULL) {
	event_query_control.event_queue_begin = NULL;
   } else {
   	former_ptr->next = current_ptr->next;
   }
   put_empty_cell(current_ptr);

   return;

} /* remove_ineffective_event */



/*
 * NAME:  check_main_power_event_in_Q
 *
 * FUNCTION: Check if a lot of the events of "main power switch off" exist 
 *
 * RETURN VALUE DESCRIPTION:
 *      TRUE - more than 5 PM_EVENT_MAIN_POWER are still unresolved. Need
 *	       to issue a fail safe power-off.
 *      FALSE - no critical situation for main power off event 
 */
int	
check_main_power_event_in_Q()
{
    int			i=0;
    ext_event_q_t	*p;

    if (event_query_control.event_queue_begin) { 
    	for (p = event_query_control.event_queue_begin; 
     	     p != NULL;
     	     p = p->next) {

		if ((p->event == PM_EVENT_POWER_SWITCH_OFF) ||
		    (p->event == PM_EVENT_POWER_SWITCH_ON)) { 
			i++;
		} /* endif */

    	} /* endfor */
    } /* endif */ 

    /* One is already counted as the last event. */
    if (i >= 4) {
	return TRUE;
    } /* endif */ 
    return FALSE;

} /* check_main_power_event_in_Q */



/*
 * NAME:  check_event_occurrence
 *
 * FUNCTION:  To check if a new event has occurred.
 *
 * NOTES: If PM event has already occurred, return value is "TRUE" and 
 *        the oldest event is picked up and put into the area pointed to 
 *	  by input argument. 
 *
 * RETURN VALUE DESCRIPTION:
 *      TRUE - event has occurred
 *	FALSE - no event has occurred 
 *     
 */
int 
check_event_occurrence(ext_event_q_t *arg)
{
	ext_event_q_t	*p;

	DBG(Entry of Check_event_occurrence())
	simple_lock(&(event_query_control.lock));

	if (p = event_query_control.event_queue_begin) {

		event_query_control.event_queue_begin = p->next;
		memcpy(arg, p, sizeof(ext_event_q_t));
		put_empty_cell(p);

		switch (arg->event) {
		case PM_EVENT_POWER_SWITCH_OFF:

		    if ((pm_parameters.daemon_data.main_switch_action
						== PM_SYSTEM_SHUTDOWN) ||
		        (pm_control_data.system_state == PM_SYSTEM_FULL_ON)){

			ShutDownPreparation();	

		    } /* endif */
		    break;

		case PM_EVENT_LOW_BATTERY:
	
	            if ((pm_parameters.daemon_data.low_battery_action
						== PM_SYSTEM_SHUTDOWN) ||
			(pm_control_data.system_state == PM_SYSTEM_FULL_ON)){

			ShutDownPreparation();	

		    } /* endif */
		    break;

		case PM_EVENT_SYSTEM_IDLE_TIMER:
	
	            if (pm_parameters.daemon_data.system_idle_action
						== PM_SYSTEM_SHUTDOWN) {
			ShutDownPreparation();	

		    } /* endif */
		    break;

		case PM_EVENT_LID_CLOSE:
	
	            if (pm_parameters.daemon_data.lid_close_action
						== PM_SYSTEM_SHUTDOWN) { 
			ShutDownPreparation();	

		    } /* endif */
		    break;

		case PM_EVENT_SPECIFIED_TIME:
	
	            if (pm_parameters.daemon_data.specified_time_action
						== PM_SYSTEM_SHUTDOWN) { 
			ShutDownPreparation();	

		    } /* endif */
		    break;

		} /* endswitch */

		simple_unlock(&(event_query_control.lock));
		return PM_SUCCESS;

	} /* endif */

	simple_unlock(&(event_query_control.lock));
	return (PM_ERROR);

} /* check_event_occurrence */



/*
 * NAME:  ShutDownPreparation
 *
 * FUNCTION: When the event corresponding to shutdown occurs, 
 *	     Power LED starts to blink and the screen is forcedly
 *	     turned on. 
 *
 * RETURN VALUE DESCRIPTION:
 *	     none
 */
void
ShutDownPreparation()
{
   	LED_block_t	LED_block;
	ScreenOn_req_blk_t ScreenOn_req_blk;

        LED_block.LED_type=power_on_LED;
        LED_block.control=LED_blink;
        CALL_PMMD_WO_LOCK(LED_control,&LED_block)

	ScreenOn_req_blk.LCD_on = TRUE;
	ScreenOn_req_blk.CRT_on = TRUE;
	CALL_PMMD_WO_LOCK(EmergencyScreenOn, &ScreenOn_req_blk)
		
	pm_control_data.shutdown_on_going = TRUE;
	return;
}

