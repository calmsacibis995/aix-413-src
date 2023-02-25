/* @(#)97	1.1  src/tcpip/usr/sbin/snmpd/ddmibs.h, snmp, tcpip411, GOLD410 1/21/94 17:06:46 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/ddmibs.h
 */

#ifndef	_H_DDMIBS
#define	_H_DDMIBS

#include <sys/ndd.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/tokenring_mibs.h>

typedef union {
    ethernet_all_mib_t ethernet_all;
    token_ring_all_mib_t token_ring_all;
} combo_mibs_t;

#endif	/* _H_DDMIBS */
