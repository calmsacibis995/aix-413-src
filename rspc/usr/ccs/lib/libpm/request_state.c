static char sccsid[] = "@(#)21  1.1  src/rspc/usr/ccs/lib/libpm/request_state.c, pwrcmd, rspc41J, 9510A 3/8/95 07:42:20";
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: pmlib_request_state, send_state, receive_state
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
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pmlib.h>
#include "pmlib_internal.h"
#include "pm_sock.h"
#include "pmlibdebug.h"


/* request_state.c */
static int send_state(int, pmlib_state_t *);
static int receive_state(int, pmlib_state_t *);
static int receive_error(int, pmlib_state_t *);


extern int errno;

/*
 * NAME:  pmlib_request_state
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is pmlib_request_state library.
 *
 * DATA STRUCTURES:
 *      pmlib_state_t - structure used for request state change or
 *                      query current state or latest state change error.
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Invalid ctrl argument or Unsuccessfully processed
 */
int
pmlib_request_state(int ctrl, pmlib_state_t *pms)
{
	switch(ctrl) {
	case PMLIB_QUERY_STATE:
	case PMLIB_QUERY_ERROR:
		return receive_state(ctrl, pms);

	case PMLIB_REQUEST_STATE:
	case PMLIB_CONFIRM:
		return send_state(ctrl, pms);

	default:
		DEBUG_1("Invalid ctrl %d\n", ctrl);
		errno = EINVAL;
		return PMLIB_ERROR;
	}
}



/*
 * NAME:  send_state
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_state library.
 *      Request/confirm to change state.
 *
 * DATA STRUCTURES:
 *      pmlib_state_t - structure used for request state change or
 *                      query current state or latest state change error.
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Unsuccessfully processed
 */
static int
send_state(int ctrl, pmlib_state_t *pms)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_STATE;
	clidata.ctrl = ctrl;
	clidata.pcu.pms.state = pms->state;

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}


/*
 * NAME:  receive_state
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_state library.
 *      Query current PM system state or the latest error of PM system
 *      state change.
 *
 * DATA STRUCTURES:
 *      pmlib_state_t - structure used for request state change or
 *                      query current state or latest state change error.
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Unsuccessfully processed
 */
static int
receive_state(int ctrl, pmlib_state_t *pms)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	memset((char *)pms, 0, sizeof(pmlib_state_t));
	memset((char *)&clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_STATE;
	clidata.ctrl = ctrl;

	if(pmlib_recv_reply(&clidata, &servdata, sizeof(servdata))
							!= PMLIB_SUCCESS){
		return PMLIB_ERROR;
	}

	pms->state = servdata.psu.pms.state;
	pms->error = servdata.psu.pms.error;
	pms->pid = servdata.psu.pms.pid;
	strcpy(pms->name, servdata.psu.pms.name);

	return PMLIB_SUCCESS;
}
