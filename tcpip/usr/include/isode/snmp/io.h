/* @(#)86	1.4  src/tcpip/usr/include/isode/snmp/io.h, snmp, tcpip411, GOLD410 1/21/94 17:51:48 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: remfd
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
 * FILE: src/tcpip/usr/include/isode/snmp/io.h
 */

#ifndef _IO_
#define _IO_

/* bring in common include files. */

/* system */
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* isode library */
#include <isode/psap.h> /* includes manifest.h, and general.h */

/* when all else fails... some default defines */
#define SNMP_PORT	161
#define SNMP_TRAP	162
#define SMUX_PORT	199

/* keep track of internal fd's */
struct fdinfo {
    struct sockaddr_in *sockaddr_in;
    int	fi_fd;			/* the fd in question */
    PS	fi_ps;			/* PStream associated with this fd */
    int fi_size;		/* amount of data we will read/write */
    int fi_flags;
#define FI_NOTINUSE	0
#define FI_UDP		1
#define FI_SMUX		2
#define FI_SMUX_CLIENT	4
#define FI_SRC		8
#define FI_DEVICE	16

    struct fdinfo *fi_next;	/* linked list */
    caddr_t fi_info;		/* any additional needed info */
};

#define	remfd(fd)	remfi(fd2fi(fd))

extern	struct fdinfo *addfd (), *fd2fi (), *type2fi ();
extern	void remfi ();

extern	IFP	o_dumpbits;
extern	IFP	o_advise;
extern	IFP     o_adios;

#endif /* _IO_ */
