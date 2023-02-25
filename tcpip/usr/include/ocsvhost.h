/* @(#)65	1.2.1.1  src/tcpip/usr/include/ocsvhost.h, tcpip, tcpip411, GOLD410 3/18/94 13:10:38 */
/*
 * COMPONENT_NAME: TCPIP ocsvhost.h
 *
 * FUNCTIONS: 
 *            
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Virtual Host Database
 * Structures for ODM Interface for OCSvhost Object Class
 * Output .h file from odmcreate()
 */

#include <odmi.h>
#include <sys/cfgodm.h>
#include "lm.h"

#define INETADDRSIZE    256
#define PROTONAMESIZE    10

#define OCSvhost_PATH   "/etc/objrepos"

struct OCSvhost {
	long _id;
	long _reserved;
	long _scratch;
	char vh_inetaddr[INETADDRSIZE];
	char protoname[PROTONAMESIZE];
	char hostname[HOSTNMSIZE];
	char alt_hostname_one[HOSTNMSIZE];
	char alt_hostname_two[HOSTNMSIZE];
	};
#define OCSvhost_Descs 5

extern struct Class OCSvhost_CLASS[];
#define get_OCSvhost_list(a,b,c,d,e) (struct OCSvhost * )odm_get_list(a,b,c,d,e)

/*
 * Structure used by OCS to allow multiple network interfaces.
 * For each connection, we must bind to the address of the
 * OCS node name specified in the OCS stanza on AIX/ESA, which
 * is the network interface used.  And then connect to the host
 * name that has been determined to be connected.
 */
struct OCSconnection {
    char ocsnodename[HOSTNMSIZE];      /* OCS node to bind to */
    char hostname[HOSTNMSIZE];         /* Host node to connect to */
    char protoname[PROTONAMESIZE];       /* protocol name */
    };

