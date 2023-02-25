static char sccsid[] = "@(#)53	1.2  src/tcpip/usr/lib/libsnmp/global.c, snmp, tcpip411, GOLD410 3/2/93 18:23:06";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/global.c
 */


int SNMP_requests;		/* # of SNMP requests, last ID sent */
int last_SNMP;			/* ID of last SNMP packet received */
int SNMP_port;			/* fd of SNMP port */
int max_SNMP_retries = 3;	/* number of times to retry request */
int SNMP_timeout = 5;		/* timeout period for a read. Default 5 secs */

