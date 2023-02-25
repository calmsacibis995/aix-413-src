static char sccsid[] = "@(#)65      1.4.1.1  src/tcpip/usr/ccs/lib/libtcp_audit/tcpip_audit.c, tcpip, tcpip411, GOLD410 12/7/92 18:01:43";
/* 
 * COMPONENT_NAME: TCPIP tcpip_audit.c
 * 
 * FUNCTIONS: make_in_addr, tcpip_auditlog 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Front end routine for TCP/IP LPP socket programs using the
 * auditwrite() library call.
 *
*/

#ifdef _CSECURITY
#include <stdio.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/syslog.h>

#include "tcpip_audit.h"

tcpip_auditlog(audit_type, audit_tail, audit_result)
int audit_type;
char *audit_tail[];
int audit_result;
{
	int signal_mask;
	static int first_trip = 1;
	static int audit_status;

	if (first_trip) {
		audit_tail[0] = PROTOCOL;
		signal_mask = sigblock(0xffff);
		audit_status = auditproc((char *)0, AUDIT_QSTATUS,(char *)0, 0);
		(void) sigsetmask(signal_mask);
		first_trip--;
	}

	if (audit_status) {
		signal_mask = sigblock(0xffff);
		switch (audit_type) {
			case CHANGE_HOST_ID: {
				auditwrite(EVENT_CHANGE_HOST_ID, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					(char *) 0);
				break;
			}
			case CHANGE_CONFIGURATION: {
				auditwrite(EVENT_CHANGE_CONFIGURATION, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case CHANGE_MANUALLY_SET_ROUTE: {
				auditwrite(EVENT_CHANGE_MANUALLY_SET_ROUTE, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case CONNECTION: {
				auditwrite(EVENT_CONNECTION, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case IMPORT_DATA: {
				auditwrite(EVENT_IMPORT_DATA, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case EXPORT_DATA: {
				auditwrite(EVENT_EXPORT_DATA, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case NET_ACCESS: {
				auditwrite(EVENT_NET_ACCESS, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case SET_NETWORK_TIME: {
				auditwrite(EVENT_SET_NETWORK_TIME, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					(char *) 0);
				break;
			}
			case MAIL_CONFIGURATION: {
				auditwrite(EVENT_MAIL_CONFIGURATION, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					(char *) 0);
				break;
			}
			case MAIL_TO_A_FILE: {
				auditwrite(EVENT_MAIL_TO_A_FILE, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					(char *) 0);
				break;
			}
			default: {
				syslog(LOG_ALERT, "Unknown audit event!!!");
				exit(1);
			}
			
		}
		(void) sigsetmask(signal_mask);
	}
}

unsigned long
make_in_addr(p)
char *p;
{
	unsigned long inetaddr;
	bcopy(p, &inetaddr, sizeof(inetaddr));
	return(inetaddr);
}

#endif _CSECURITY
