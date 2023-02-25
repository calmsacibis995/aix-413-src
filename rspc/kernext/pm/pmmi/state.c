static char sccsid[] = "@(#)53  1.2  src/rspc/kernext/pm/pmmi/state.c, pwrsysx, rspc411, 9437B411a 9/15/94 00:11:44";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: pm_control_state1, query_state, request_state
 *              start_state, query_request
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

#include <sys/pm.h>

/* PM core data */
extern struct _pm_core_data pm_core_data;

/* ids/events for pm_control_state1() */
extern struct pm_state	pmstate[];
extern int		pm_event_id;

#define		NO_ACTION	0xffffffff	/* not processed yet */
#define		ID2INDEX(id)	((id) % (PM_MAX_REQUESTED_EVENT_NUMBER))
#define		SET_REQUEST(new_state)					\
			pmstate[ID2INDEX(pm_event_id)].state = NO_ACTION;\
			pm_core_data.system_next_state = (new_state);	\
			pm_set_event(PM_EVENT_SOFTWARE_REQUEST);
#define		UNLOCK_AND_EINVAL_ERROR_RETURN()		\
			simple_unlock(&(pm_core_data.lock));	\
			setuerror(EINVAL);			\
			return PM_ERROR;
#define		UNLOCK_AND_ERROR_RETURN()			\
			simple_unlock(&(pm_core_data.lock));	\
			return PM_ERROR;


/*
 * NAME:  pm_control_state1  (add suffix 1 for phase-1 only)
 *
 * FUNCTION:  PM system call of phase1 pm (AIX4.1.1)
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine provides the phase-1 feature of system call, 
 *	"pm_control_state."
 *
 * DATA STRUCTURES:
 *	ctrl - Specifies a pm_control_state command. 
 *	pms - Points a pm_state structure to transfer data.
 *	pmstate[] - Stores id/event information.
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Caller doesn't have priveledge or invalid argument.
 */
int
pm_control_state1(int ctrl, struct pm_state *pms) /* add suffix 1 for phase1 */
{
    char ep;

    if( suser( &ep ) == 0 ) {		/* make sure super user */
	setuerror(ep);
	return PM_ERROR;
    }

    switch( ctrl ) {
      case PM_CTRL_QUERY_STATE:
	return query_state(pms);

      case PM_CTRL_REQUEST_STATE:
	if( pmstate[ID2INDEX(pm_event_id)].state == NO_ACTION ) {
	    setuerror(EBUSY);
	    return PM_ERROR;
	} else {
	    return request_state(pms);
        }

      case PM_CTRL_START_STATE:
	return start_state(pms);

      case PM_CTRL_QUERY_REQUEST:
	return query_request(pms);
    }

    setuerror(EINVAL);
    return PM_ERROR;
}


/*
 * NAME:  query_state
 *
 * FUNCTION:  Returns PM system state.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core to refer.
 *      pm_state - Returns data in state field.
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Error in copyout.
 */
static int
query_state(struct pm_state *pms)
{
    int	state;

    state = pm_core_data.system_state;

    if( copyout( &state, &(pms->state), sizeof(int)) != 0 ) {
	return PM_ERROR;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  request_state
 *
 * FUNCTION:  Requests PM system state change.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_state - Returns data in id field.
 *      pm_core_data - Data structure in PM core.
 *	pmstate[] - Stores id/event information.
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed.
 *      PM_ERROR - Failed to call kernel service or invalid argument.
 */
static int
request_state(struct pm_state *pms)
{
    struct pm_state p;

    if( copyin(pms, &p, sizeof(struct pm_state)) != 0 ) {
	return PM_ERROR;
    }

    simple_lock(&(pm_core_data.lock));

    switch( p.state ) {
      case PM_SYSTEM_FULL_ON:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_ENABLE:
	    SET_REQUEST(PM_SYSTEM_FULL_ON);
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_ENABLE:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_FULL_ON:
	    SET_REQUEST(PM_SYSTEM_ENABLE);
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_STANDBY:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_ENABLE:
	    SET_REQUEST(PM_SYSTEM_STANDBY);
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_SUSPEND:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_ENABLE:
	  case PM_SYSTEM_STANDBY:
	    SET_REQUEST(PM_SYSTEM_SUSPEND);
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_HIBERNATION:
      case PM_SYSTEM_SHUTDOWN:
	SET_REQUEST(PM_SYSTEM_SHUTDOWN);
	break;

      case PM_NONE:
	SET_REQUEST(PM_NONE);
	break;

      default:
	UNLOCK_AND_EINVAL_ERROR_RETURN();
    }

    p.id = pm_event_id;

    simple_unlock(&(pm_core_data.lock));

    if( copyout( &p, pms, sizeof(struct pm_state)) != 0 ) {
	return PM_ERROR;
    }
    return PM_SUCCESS;
}


/*
 * NAME:  start_state
 *
 * FUNCTION:  Initiates PM system state change.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_state - Returns data in event field. (and devno field at failure)
 *      pm_core_data - Data structure in PM core
 *	pmstate[] - Stores id/event information.
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS - Successfully processed
 *      PM_ERROR - Failed to call kernel service or invalid argument
 */
static int
start_state(struct pm_state *pms)
{
    struct pm_state p;

    if( copyin(pms, &p, sizeof(struct pm_state)) != 0 ) {
	return PM_ERROR;
    }

    simple_lock(&(pm_core_data.lock));

    p.id = pm_event_id++;
    if( pm_event_id < 0 ) {
	pm_event_id = 0;
    }
    pmstate[ID2INDEX(pm_event_id)].state = 0;
    pmstate[ID2INDEX(p.id)].state = p.state;
    pmstate[ID2INDEX(p.id)].id = p.id;

    switch( p.state ) {
      case PM_SYSTEM_FULL_ON:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_ENABLE:
	    if( pm_start_system_full_on(&p) == PM_ERROR ) {
		UNLOCK_AND_ERROR_RETURN();
	    }
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_ENABLE:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_FULL_ON:
	  case PM_SYSTEM_STANDBY:
	  case PM_SYSTEM_SUSPEND:
	    p.event = PM_EVENT_SOFTWARE_REQUEST;
	    if( pm_start_system_enable(&p) == PM_ERROR ) {
		UNLOCK_AND_ERROR_RETURN();
	    }
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_STANDBY:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_ENABLE:
	    if( pm_start_system_standby(&p) == PM_ERROR ) {
		UNLOCK_AND_ERROR_RETURN();
	    }
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_SUSPEND:
	switch( pm_core_data.system_state ) {
	  case PM_SYSTEM_ENABLE:
	  case PM_SYSTEM_STANDBY:
	    if( pm_start_system_suspend(&p) == PM_ERROR ) {
		UNLOCK_AND_ERROR_RETURN();
	    }
	    break;

	  default:
	    UNLOCK_AND_EINVAL_ERROR_RETURN();
	}
	break;

      case PM_SYSTEM_HIBERNATION: /* just return for pmd */
      case PM_SYSTEM_SHUTDOWN:
	p.event = PM_EVENT_NONE;
	break;

      case PM_NONE:
	p.event = PM_EVENT_NONE;
	break;

      default:
	UNLOCK_AND_EINVAL_ERROR_RETURN();
    }

    pmstate[ID2INDEX(p.id)].event = p.event;
    pmstate[ID2INDEX(p.id)].devno = p.devno;

    simple_unlock(&(pm_core_data.lock));

    if( copyout( &p, pms, sizeof(struct pm_state)) != 0 ) {
	return PM_ERROR;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  query_request
 *
 * FUNCTION:  Queries the result of an action request.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	<<<< This routine is only called for PM phase 1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_state - Returns data in event field. (and devno field at failure)
 *	pmstate[] - Stores id/event information.
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_ERROR - Failed to call kernel service
 *      PM_SUCCESS - Successfully processed
 */
static int
query_request(struct pm_state *pms)
{
    struct pm_state p;
    int i;

    if( copyin(pms, &p, sizeof(struct pm_state)) != 0 ) {
	return PM_ERROR;
    }

    for( i = 0; i < PM_MAX_REQUESTED_EVENT_NUMBER; i++ ) {
	if( p.id == pmstate[ID2INDEX(i)].id ) {
	    p.event = pmstate[ID2INDEX(i)].event;
	    p.devno = pmstate[ID2INDEX(i)].devno;
	    break;
	}
    }
    if ( i >= PM_MAX_REQUESTED_EVENT_NUMBER ){
	setuerror(EINVAL);
	return PM_ERROR;
    }

    if( copyout( &p, pms, sizeof(struct pm_state)) != 0 ) {
	return PM_ERROR;
    }

    return PM_SUCCESS;
}
