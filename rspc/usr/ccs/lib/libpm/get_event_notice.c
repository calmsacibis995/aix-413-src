static char sccsid[] = "@(#)13  1.1  src/rspc/usr/ccs/lib/libpm/get_event_notice.c, pwrcmd, rspc41J, 9510A 3/8/95 07:41:02";
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: pmlib_get_event_notice
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


extern int errno;


/*
 * NAME:  pmlib_get_event_notice
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is pmlib_get_event_notice library.
 *      Called from SIGPM handler to get latest event from
 *      PM daemon.
 *
 * DATA STRUCTURES:
 *      pmlib_client - data structure used for IPC, client->pm daemon
 *      pmlib_server - data structure used for IPC, pm daemon->client
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Invalid ctrl argument or Unsuccessfully processed
 */
int
pmlib_get_event_notice(int *event)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	DEBUG_0("pmlib_get_event_notice\n");

	memset((char *) &clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_GET_EVENT_NOTICE;
	clidata.ctrl = 0;

	if(pmlib_recv_reply(&clidata, &servdata, sizeof(servdata))
							!= PMLIB_SUCCESS){
		return PMLIB_ERROR;
	}

	*event = servdata.psu.pme.event;
	return PMLIB_SUCCESS;
}
