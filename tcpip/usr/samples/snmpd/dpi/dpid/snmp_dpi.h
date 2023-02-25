/* @(#)17	1.1  src/tcpip/usr/samples/snmpd/dpi/dpid/snmp_dpi.h, snmp, tcpip411, GOLD410 9/13/93 15:05:52 */
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
 * FILE:        /usr/samples/snmpd/dpi/dpid/snmp_dpi.h
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

/*
 *  This file contains the header information for the basic DPI routines.
 *  This include file is described and defined in RFC 1228.
 *
 *  The only change to this file from rfc1228 is the registered tree list
 *  structure.
 */

/* SNMP distributed program interface */

#ifndef _H_SNMP_DPI_
#define _H_SNMP_DPI_

#include <netinet/in.h>
#include <isode/snmp/objects.h>

#define SNMP_DPI_GET 		1
#define SNMP_DPI_GET_NEXT	2
#define SNMP_DPI_SET		3
#define SNMP_DPI_TRAP		4
#define SNMP_DPI_RESPONSE	5
#define SNMP_DPI_REGISTER	6

#define SNMP_DPI_PROTOCOL	2
#define SNMP_DPI_VERSION	1
#define SNMP_DPI_RELEASE	1

/* SNMP error codes from RFC 1157 */
#define SNMP_NO_ERROR		0
#define SNMP_TOO_BIG		1
#define SNMP_NO_SUCH_NAME	2
#define SNMP_BAD_VALUE		3
#define SNMP_READ_ONLY		4
#define SNMP_GEN_ERR		5

/* variable types */
#define SNMP_TYPE_TEXT            0       /* textual representation */
#define SNMP_TYPE_NUMBER          (128|1) /* number */
#define SNMP_TYPE_STRING          2       /* text string */
#define SNMP_TYPE_OBJECT          3       /* object identifier */
#define SNMP_TYPE_EMPTY           4       /* no value */
#define SNMP_TYPE_INTERNET        (128|5) /* internet address */
#define SNMP_TYPE_COUNTER         (128|6) /* counter */
#define SNMP_TYPE_GAUGE           (128|7) /* gauge */
#define SNMP_TYPE_TICKS           (128|8) /* time ticks (1/100ths second) */
#define SNMP_TYPE_MASK            0x7f    /* mask for type */


struct dpi_get_packet {
	char	*object_id;
};

struct dpi_next_packet {
	char	*object_id;
	char	*group_id;
};

struct dpi_set_packet {
	char		*object_id;
	unsigned char	type;
	unsigned short	value_len;
	char		*value;
	struct dpi_set_packet	*next;
};

struct dpi_resp_packet {
	unsigned char	ret_code;
	struct dpi_set_packet 	*ret_data;
};

struct dpi_trap_packet {
	unsigned char	generic;
	unsigned char	specific;
	char 			*enterprise;
	struct dpi_set_packet	*info;
};

struct snmp_dpi_hdr {
	unsigned char	proto_major;
	unsigned char	proto_minor;
	unsigned char	proto_release;

	unsigned char	packet_type;
	union {
		struct dpi_get_packet	*dpi_get;
		struct dpi_next_packet	*dpi_next;
		struct dpi_set_packet	*dpi_set;
		struct dpi_resp_packet	*dpi_response;
		struct dpi_trap_packet	*dpi_trap;
	} packet_body;
};

/* This is added to handle the list of registered trees. */
struct snmp_dpi_registered_objects {
	OID    object;
	int    owning_fd;
	struct snmp_dpi_registered_objects	*next;
};


extern struct snmp_dpi_hdr *pDPIpacket();
extern void fDPIparse();
extern unsigned char *mkMIBquery();
extern struct dpi_set_packet *mkDPIlist();
extern unsigned char *mkDPIregister();
extern unsigned char *mkDPIresponse();
extern unsigned char *mkDPItrap();
extern unsigned char *mkDPItrape();
extern struct dpi_set_packet *mkDPIset();
extern long int_val();

#endif /* _H_SNMP_DPI_ */
