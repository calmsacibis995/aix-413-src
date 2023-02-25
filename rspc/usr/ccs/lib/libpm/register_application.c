static char sccsid[] = "@(#)18  1.1  src/rspc/usr/ccs/lib/libpm/register_application.c, pwrcmd, rspc41J, 9510A 3/8/95 07:41:45";
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: pmlib_register_application, register_appl
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

static int register_appl(int);

extern int errno;


/*
 * NAME:  pmlib_register_application
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is pmlib_request_parameter library.
 *      Register/unregister the caller process as a PM Aware Application.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *      PMLIB_SUCCESS - Succesfully processed
 *      PMLIB_ERROR - Invalid cmd argument or Unsuccessfully processed
 */
int
pmlib_register_application(int cmd)
{
	DEBUG_0("pmlib_register_application\n");

	switch(cmd){
	case PMLIB_REGISTER:
	case PMLIB_UNREGISTER:
		return register_appl(cmd);
	default:
		errno = EINVAL;
		return PMLIB_ERROR;
	}
}


/*
 * NAME:  register_appl
 *
 * FUNCTION:  PM library
 *
 * NOTES:
 *      This routine is used in pmlib_register_parameter library.
 *      Registers the caller process as a PM Aware Appication.
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
register_appl(int cmd)
{
	struct pmlib_client	clidata;
	struct pmlib_server	servdata;

	memset((char *) &clidata, 0, sizeof(struct pmlib_client));
	clidata.ftype = PMLIB_INTERNAL_REGISTER_APPLICATION;
	clidata.ctrl = cmd;

	return pmlib_recv_reply(&clidata, &servdata, sizeof(servdata));
}
