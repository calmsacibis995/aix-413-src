char    sccs_id[] = " @(#)20 1.6  src/bos/usr/sbin/nimclient/set_date.c, cmdnim, bos41J, 9515B_all  4/12/95  11:49:17";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/nimclient/set_date.c
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------------------- set_date
 *
 * NAME: set_date
 *
 * FUNCTION:
 *		Send a request to the NIM Master for its current 
 *	date, time and timezone (even if we're in a different time
 *	zone...) and when we get the reply update this client to 
 *	match.
 *
 *
 * DATA STRUCTURES:
 *		parameters:
 *
 * RETURNS:
 *
 *-----------------------------------------------------------------------------*/

#include "cmdnim.h"

/* #define DATE_CMD 	"/usr/bin/date %s && /usr/bin/chtz %s" */
#define DATE_CMD 	"TZ=%s && /usr/bin/chtz %s && /usr/bin/date %s" 
#define DATE_ITEMS 	2

set_date(int argc, char *argv[], int *rc)
{

	int	pipeExit;
	int	itemCount;

	FILE	*pipeFP; 
	FILE	*rmt_out=NULL; 

	char	cmd_str[120];
	char	date_str[30]; 
	char	tz_str[80];

	/* 
	 * Make sure that argv[0] points to the remote thing 
	 * to execute. 
	 */
	argv[0] = M_GETDATE;

	/* 
	 * Send the request to the NIM Master, we would like a reply so 
	 * we specify the pointer rmt_out. 
	 */
	if (To_Master(ATTR_MASTER_T, rc, &rmt_out, argc, argv) != SUCCESS )
		return(FAILURE);

	if (*rc != 0)
		return(FAILURE);

	itemCount=fscanf(rmt_out, "%s %s", date_str, tz_str);


	if (itemCount != DATE_ITEMS)
		nim_error(ERR_ERRNO_LOG,"set_date","fscanf",NULL);

	/* 
	 * Test if stream had an error OR we did not get the items we 
	 * expected.
	 */
	if (!ferror(rmt_out) && (itemCount == DATE_ITEMS) ) {
		/* 
	 	 * Reset the date and time. 
	 	 */
		/* sprintf(cmd_str, DATE_CMD, date_str, tz_str); */
		sprintf(cmd_str, DATE_CMD, tz_str, tz_str, date_str);

		if ( (pipeFP=popen(cmd_str, "r")) != NULL) {
			pipeExit=pclose(pipeFP);
			if ( (pipeExit == 127) || (pipeExit == -1) ) {
				nim_error(ERR_ERRNO_LOG,"do_date","popen",NULL);
			}
			else
				*rc=pipeExit;
		}
		else
			*rc=ERR_ERRNO;
	}
	fclose(rmt_out);
	return(0);
} 
