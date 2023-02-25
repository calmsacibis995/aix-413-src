static char sccsid[] = "@(#)20  1.2  src/rspc/usr/ccs/lib/libpm/request_parameter.c, pwrcmd, rspc41J, 9516A_all 4/18/95 13:48:53";
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: pmlib_request_pamameter, query_param, set_time,
 *            query_device_info, set_device_info, set_action,
 *            set_on_off, query_device_number, query_device_names
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pmlib.h>
#include "pmlib_internal.h"
#include "pm_sock.h"
#include "pmlibdebug.h"


/* request_parameter.c */
static int query_param(int, int *);
static int set_time(int, int *);
static int query_device_info(int, pmlib_device_info_t *);
static int set_device_info(int, pmlib_device_info_t *);
static int set_action(int, int *, int);
static int set_on_off(int, int *);
static int query_device_names(int, pmlib_device_names_t *);


extern int errno;

/*
 * NAME:  pmlib_request_parameter
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is pmlib_request_parameter library.
 *
 * DATA STRUCTURES:
 *      pmlib_device_info_t - timer structure for PM aware DDs
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Invalid ctrl argument or Unsuccessfully processed
 */
int
pmlib_request_parameter(int ctrl, caddr_t arg)
{
	switch( ctrl ) {
	case PMLIB_QUERY_SYSTEM_IDLE_TIME:
	case PMLIB_QUERY_LID_CLOSE_ACTION:
	case PMLIB_QUERY_SYSTEM_IDLE_ACTION:
	case PMLIB_QUERY_MAIN_SWITCH_ACTION:
	case PMLIB_QUERY_LOW_BATTERY_ACTION:
	case PMLIB_QUERY_BEEP:
	case PMLIB_QUERY_DEVICE_NUMBER:
	case PMLIB_QUERY_PERMISSION:
	case PMLIB_QUERY_RINGING_RESUME:
	case PMLIB_QUERY_RESUME_TIME:
	case PMLIB_QUERY_DURATION_TO_HIBERNATION:
	case PMLIB_QUERY_SPECIFIED_TIME:
	case PMLIB_QUERY_SPECIFIED_TIME_ACTION:
	case PMLIB_QUERY_SUPPORTED_STATES:
	case PMLIB_QUERY_KILL_LFT_SESSION:
	case PMLIB_QUERY_KILL_TTY_SESSION:
	case PMLIB_QUERY_PASSWD_ON_RESUME:
	case PMLIB_QUERY_KILL_SYNCD:
		return query_param(ctrl, (int *)arg);

	case PMLIB_SET_SYSTEM_IDLE_TIME:
	case PMLIB_SET_RESUME_TIME:
	case PMLIB_SET_DURATION_TO_HIBERNATION:
	case PMLIB_SET_SPECIFIED_TIME:
		return set_time(ctrl, (int *)arg);

	case PMLIB_QUERY_DEVICE_INFO:
		return query_device_info(ctrl, (pmlib_device_info_t *)arg);

	case PMLIB_SET_DEVICE_INFO:
		return set_device_info(ctrl, (pmlib_device_info_t *)arg);

	case PMLIB_SET_LID_CLOSE_ACTION:
	case PMLIB_SET_SYSTEM_IDLE_ACTION:
	case PMLIB_SET_PERMISSION:
	case PMLIB_SET_MAIN_SWITCH_ACTION:
	case PMLIB_SET_LOW_BATTERY_ACTION:
		return set_action(ctrl, (int *)arg,
			PMLIB_SYSTEM_STANDBY | PMLIB_SYSTEM_SUSPEND |
			PMLIB_SYSTEM_HIBERNATION | PMLIB_SYSTEM_SHUTDOWN);

                /* for these actions above, PMLIB_NONE can be set */

	case PMLIB_SET_SPECIFIED_TIME_ACTION:
		return set_action(ctrl, (int *)arg,
			PMLIB_SYSTEM_SUSPEND |
			PMLIB_SYSTEM_HIBERNATION | PMLIB_SYSTEM_SHUTDOWN);
                /* for this action above, PMLIB_NONE can NOT be set */
                /* see inside set_action.                           */
	case PMLIB_SET_BEEP:
	case PMLIB_SET_RINGING_RESUME:
	case PMLIB_SET_KILL_LFT_SESSION:
	case PMLIB_SET_KILL_TTY_SESSION:
	case PMLIB_SET_PASSWD_ON_RESUME:
	case PMLIB_SET_KILL_SYNCD:
		return set_on_off(ctrl, (int *)arg);

	case PMLIB_QUERY_DEVICE_NAMES:
		return query_device_names(ctrl, (pmlib_device_names_t *)arg);

	default:
		errno = EINVAL;
		return PMLIB_ERROR;
	}
}


/*
 * NAME:  query_param
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_parameter library.
 *      Used for querying just one parameter.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Unsuccessfully processed
 */
static int
query_param(int ctrl, int *value)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;

	if(pmlib_recv_reply(&clidata, &servdata, sizeof(servdata))
							!= PMLIB_SUCCESS){
		return PMLIB_ERROR;
	}

	*value = servdata.psu.pmp.value;
	return PMLIB_SUCCESS;
}


/*
 * NAME:  set_time
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_parameter library.
 *      Set time value.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_ERROR - Caller doesn't have privilege or Unsuccessfully
 *                    processed.
 *      PMLIB_SUCCESS - Successfully processed
 */
static int
set_time(int ctrl, int *timer)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	/* check the validity of *timer */
	if(*timer < 0){
		errno = EINVAL;
		return PMLIB_ERROR;
	}

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;
	clidata.pcu.pmp.value = *timer;

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}


/*
 * NAME:  query_device_info
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used pmlib_request_parameter library.
 *      Query the information of a PM Aware DD
 *
 * DATA STRUCTURES:
 *      pmlib_device_info_t - timer structure for PM aware DDs
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_ERROR - Caller doesn't have priveledge,or invalid argument
 *      PMLIB_SUCCESS - Successfully processed
 */
static int
query_device_info(int ctrl, pmlib_device_info_t *devinfo)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	/* check the validity of devinfo */
	if(strlen(devinfo->name) == 0){
		DEBUG_1("strlen(devinfo->name) is %d\n", strlen(devinfo->name));
		errno = EINVAL;
		return PMLIB_ERROR;
	}

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;
	strcpy(clidata.pcu.pmdi.name, devinfo->name);

	if(pmlib_recv_reply(&clidata, &servdata, sizeof(servdata))
							!= PMLIB_SUCCESS){
		return PMLIB_ERROR;
	}

	devinfo->mode = servdata.psu.pmdi.mode;
	devinfo->attribute = servdata.psu.pmdi.attribute;
	devinfo->idle_time = servdata.psu.pmdi.idle_time;
	devinfo->standby_time = servdata.psu.pmdi.standby_time;
	devinfo->idle_time1 = servdata.psu.pmdi.idle_time1;
	devinfo->idle_time2 = servdata.psu.pmdi.idle_time2;
	return PMLIB_SUCCESS;
}


/*
 * NAME:  set_device_info
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_parameter library.
 *      Set the information of a PM Aware DD
 *
 * DATA STRUCTURES:
 *      pmlib_device_info_t - timer struct for PM aware DDs
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_ERROR - Caller doesn't have priveledge, or invalid argument
 *      PMLIB_SUCCESS - Successfully processed
 */
static int
set_device_info(int ctrl, pmlib_device_info_t *devinfo)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;
	int			dit, dst, dit1, dit2;

	dit = devinfo->idle_time; 
	dst = devinfo->standby_time; 
	dit1 = devinfo->idle_time1; 
	dit2 = devinfo->idle_time2; 

	/* check the validity of devinfo */
	if(strlen(devinfo->name) == 0){
		DEBUG_1("strlen(devinfo->name) is %d\n", strlen(devinfo->name));
		errno = EINVAL;
		return PMLIB_ERROR;
	}

	if((dit < -1) || (dst < -1) || (dit1 < -1) || (dit2 < -1)){
		errno = EINVAL;
		return PMLIB_ERROR;
	}

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;
	memcpy(&(clidata.pcu.pmdi), devinfo, sizeof(pmlib_device_info_t));

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}



/*
 * NAME:  set_action
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_parameter library.
 *      Set default action for lid close.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Successfully processed
 *      PMLIB_ERROR - Invalid argument
 */
static int
set_action(int ctrl, int *action, int range)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	/* check the validity of *action , within the range or PMLIB_NONE */
	if ( ((*action & range) == 0)&&(*action != PMLIB_NONE) ||
	     (ctrl == PMLIB_SET_SPECIFIED_TIME_ACTION)&&(*action == PMLIB_NONE)
	   ){
           /* for the SPECIFIED_ACTION, PMLIB_NONE can NOT be set */

	    errno = EINVAL;
	    return PMLIB_ERROR;
	}

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;
	clidata.pcu.pmp.value = *action;

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}


/*
 * NAME:  set_on_off
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_parameter library.
 *      Set whether flag is on or off.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_ERROR - Invalid argument
 *      PMLIB_SUCCESS - Successfully processed
 */
static int
set_on_off(int ctrl, int *flag)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	DEBUG_2("set_on_off(%d, %d)\n", ctrl, *flag);

	/* check the validity of *flag */
	if((*flag != PMLIB_ON) && (*flag != PMLIB_OFF)){
		errno = EINVAL;
		return PMLIB_ERROR;
	}

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;
	clidata.pcu.pmp.value = *flag;

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}


/*
 * NAME:  query_device_names
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_parameter library.
 *      Query all the names of registered PM Aware DDs.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_ERROR - failed to call kernel service
 *      PMLIB_SUCCESS - Successfully processed
 */
static int
query_device_names(int ctrl, pmlib_device_names_t *pmdn)
{
	int			len;
	struct pmlib_client	clidata;
	struct pmlib_server	*servdata;

	if(pmdn->names == NULL){
		DEBUG_0("query_device_names: pmdn->names is NULL\n");
		errno = EINVAL;
		return PMLIB_ERROR;
	}

	len = sizeof(struct pmlib_server) +
			(pmdn->number - 1) * PMLIB_DEVICE_NAME_LEN;
	servdata = malloc(len);
	if(servdata == NULL){
		errno = ENOMEM;
		return PMLIB_ERROR;
	}

	memset((char *)servdata, 0, len);
	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_PARAMETER;
	clidata.ctrl = ctrl;
	clidata.pcu.pmdn.number = pmdn->number;

	if(pmlib_recv_reply(&clidata, servdata, len) != PMLIB_SUCCESS){
		DEBUG_0("query_device_names: pmlib_recv_reply error\n");
		return PMLIB_ERROR;
	}

	pmdn->number = servdata->psu.pmdn.number;
	memcpy(pmdn->names, servdata->psu.pmdn.names,
				pmdn->number * PMLIB_DEVICE_NAME_LEN);

	return PMLIB_SUCCESS;
}
