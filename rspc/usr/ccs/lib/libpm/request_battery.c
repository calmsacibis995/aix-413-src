static char sccsid[] = "@(#)19  1.1  src/rspc/usr/ccs/lib/libpm/request_battery.c, pwrcmd, rspc41J, 9510A 3/8/95 07:41:53";
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: pmlib_battery_request, discharge_battery, query_battery
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


static int discharge_battery(int);
static int query_battery(int, pmlib_battery_t *);

extern int errno;


/*
 * NAME:  pmlib_request_battery
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is pmlib_request_battery library.
 *
 * DATA STRUCTURES:
 *      pmlib_battery - structure used for battery information query
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Invalid cmd argument or Unsuccessfully processed
 */
int
pmlib_request_battery(int cmd, pmlib_battery_t *pmb)
{
	switch(cmd){
	case PMLIB_DISCHARGE_BATTERY:
		return discharge_battery(cmd);
	case PMLIB_QUERY_BATTERY:
		return query_battery(cmd, pmb);
	default:
		DEBUG_1("Invalid cmd %d\n", cmd);
		errno = EINVAL;
		return PMLIB_ERROR;
	}
}


/*
 * NAME:  discharge_battery
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_battery library.
 *      Discharge battery.
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
discharge_battery(int cmd)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	memset((char *) &clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REQUEST_BATTERY;
	clidata.ctrl = cmd;

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}


/*
 * NAME:  query_battery
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_request_battery library.
 *      Query battery information.
 *
 * DATA STRUCTURES:
 *      pmlib_battery - structure used for battery information query
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Unsuccessfully processed
 */
static int
query_battery(int cmd, pmlib_battery_t *pmb)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	memset((char *) &clidata, 0, sizeof(struct pmlib_client));

	clidata.ftype = PMLIB_INTERNAL_REQUEST_BATTERY;
	clidata.ctrl = cmd;

	if(pmlib_recv_reply(&clidata, &servdata, sizeof(servdata))
							!= PMLIB_SUCCESS){
		DEBUG_0("query_battery failed\n");
		return PMLIB_ERROR;
	}

	memcpy(pmb, &(servdata.psu.pmb), sizeof(pmlib_battery_t));
	return PMLIB_SUCCESS;
}
