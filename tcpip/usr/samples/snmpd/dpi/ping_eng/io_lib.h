/* @(#)26	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/io_lib.h, snmp, tcpip411, GOLD410 9/13/93 15:08:45 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/io_lib.h
 */

/*
 * INTERNATIONAL BUSINESS MACHINES CORP. PROVIDES THIS SOURCE CODE
 * EXAMPLE, ASSOCIATED LIBRARIES AND FILES "AS IS," WITHOUT WARRANTY 
 * OF ANY KIND EITHER EXPRESSED OR IMPLIED INCLUDING BUT NOT LIMITED 
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  The entire risk as to the quality and performance 
 * of this example is with you.  Should any part of this source code 
 * example prove defective you (and not IBM or an IBM authorized 
 * dealer) assume the entire cost of all necessary servicing, repair, 
 * or correction.
 */

#ifndef _H_PING_ENG_IO_LIB_
#define _H_PING_ENG_IO_LIB_

extern int read_packet();
extern int connect_to_agent();
extern unsigned long lookup_host();
extern void del_from_mask();
extern void add_fd();
extern int await_queries();

/* user-exit */
extern int data_available();

extern unsigned long select_mask;

#define MAX_FDS	20

#endif /* _H_PING_ENG_IO_LIB_ */
