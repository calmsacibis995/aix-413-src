static char sccsid[] = "@(#)54	1.5  src/tcpip/usr/sbin/gated/inet.c, tcprouting, tcpip411, GOLD410 12/6/93 17:49:57";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  inet.c,v 1.34.2.5 1993/08/27 22:28:11 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#include "inet.h"

int inet_ipforwarding = TRUE;		/* IP forwarding engine enabled */
int inet_udpcksum = TRUE;		/* UDP checksums enabled */

static task *inet_task;

#ifdef	IP_MULTICAST
#define	INET_IFPS_ALLROUTERS	IFPS_KEEP1	/* We joined the all-routers group on this interface */

static sockaddr_un *inet_addr_allrouters;	/* All routers multicast address */

#define	INET_ADDR_ALLROUTERS	0xe0000002	/* 224.0.0.2 */

static const bits inet_if_bits[] = {
    { INET_IFPS_ALLROUTERS, "AllRouters" },
    { 0 }
};
#endif	/* IP_MULTICAST */

sockaddr_un *inet_masks[34] = { 0 };

sockaddr_un *inet_addr_default = 0;
sockaddr_un *inet_addr_loopback = 0;
sockaddr_un *inet_addr_any = 0;
sockaddr_un *inet_addr_reject = 0;		/* Where reject routes need to point */
sockaddr_un *inet_addr_blackhole = 0;		/* Where unreachable routes need to point */

/* Table for quick classification of networks */
struct inet_class inet_classes[256] = {

    /* Class A */

    { INET_CLASSC_A,	INET_CLASSF_NETWORK|INET_CLASSF_DEFAULT,	INET_MASK_DEFAULT },	/* 0 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 1 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 2 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 3 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 4 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 5 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 6 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 7 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 8 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 9 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 10 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 11 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 12 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 13 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 14 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 15 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 16 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 17 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 18 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 19 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 20 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 21 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 22 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 23 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 24 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 25 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 26 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 27 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 28 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 29 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 30 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 31 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 32 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 33 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 34 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 35 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 36 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 37 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 38 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 39 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 40 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 41 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 42 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 43 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 44 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 45 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 46 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 47 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 48 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 49 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 50 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 51 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 52 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 53 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 54 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 55 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 56 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 57 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 58 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 59 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 60 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 61 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 62 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 63 */
#ifdef	INET_CLASS_A_SHARP

    /* Class A# */

    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 64 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 65 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 66 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 67 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 68 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 69 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 70 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 71 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 72 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 73 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 74 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 75 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 76 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 77 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 78 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 79 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 80 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 81 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 82 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 83 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 84 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 85 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 86 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 87 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 88 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 89 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 90 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 91 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 92 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 93 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 94 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 95 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 96 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 97 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 98 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 99 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 100 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 101 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 102 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 103 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 104 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 105 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 106 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 107 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 108 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 109 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 110 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 111 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 112 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 113 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 114 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 115 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 116 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 117 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 118 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 119 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 120 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 121 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 122 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 123 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 124 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 125 */
    { INET_CLASSC_A_SHARP,	INET_CLASSF_EXPERIMENTAL,	INET_MASK_INVALID },	/* 126 */

#else	/* INET_CLASS_A_SHARP */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 64 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 65 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 66 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 67 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 68 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 69 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 70 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 71 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 72 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 73 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 74 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 75 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 76 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 77 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 78 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 79 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 80 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 81 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 82 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 83 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 84 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 85 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 86 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 87 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 88 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 89 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 90 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 91 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 92 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 93 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 94 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 95 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 96 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 97 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 98 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 99 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 100 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 101 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 102 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 103 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 104 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 105 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 106 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 107 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 108 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 109 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 110 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 111 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 112 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 113 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 114 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 115 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 116 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 117 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 118 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 119 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 120 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 121 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 122 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 123 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 124 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 125 */
    { INET_CLASSC_A,	INET_CLASSF_NETWORK,	INET_MASK_CLASSA },	/* 126 */
#endif	/* INET_CLASS_A_SHARP */
    { INET_CLASSC_A,	INET_CLASSF_LOOPBACK,	INET_MASK_CLASSA },	/* 127 */

    /* Class B */

    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 128-128.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 129-129.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 130-130.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 131-131.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 132-132.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 133-133.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 134-134.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 135-135.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 136-136.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 137-137.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 138-138.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 139-139.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 140-140.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 141-141.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 142-142.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 143-143.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 144-144.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 145-145.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 146-146.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 147-147.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 148-148.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 149-149.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 150-150.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 151-151.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 152-152.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 153-153.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 154-154.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 155-155.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 156-156.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 157-157.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 158-158.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 159-159.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 160-160.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 161-161.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 162-162.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 163-163.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 164-164.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 165-165.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 166-166.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 167-167.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 168-168.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 169-169.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 170-170.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 171-171.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 172-172.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 173-173.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 174-174.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 175-175.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 176-176.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 177-177.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 178-178.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 179-179.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 180-180.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 181-181.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 182-182.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 183-183.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 184-184.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 185-185.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 186-186.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 187-187.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 188-188.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 189-189.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 190-190.255 */
    { INET_CLASSC_B,	INET_CLASSF_NETWORK,	INET_MASK_CLASSB },	/* 191-191.255 */

    /* Class C */
    
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 192.0.0-192.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 193.0.0-193.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 194.0.0-194.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 195.0.0-195.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 196.0.0-196.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 197.0.0-197.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 198.0.0-198.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 199.0.0-199.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 200.0.0-200.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 201.0.0-201.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 202.0.0-202.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 203.0.0-203.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 204.0.0-204.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 205.0.0-205.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 206.0.0-206.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 207.0.0-207.255.255 */
#ifdef	INET_CLASS_C_SHARP

    /* Class C Sharp */

    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 208.0.0-208.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 209.0.0-209.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 210.0.0-210.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 211.0.0-211.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 212.0.0-212.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 213.0.0-213.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 214.0.0-214.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 215.0.0-215.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 216.0.0-216.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 217.0.0-217.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 218.0.0-218.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 219.0.0-219.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 220.0.0-220.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 221.0.0-221.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 222.0.0-222.255.255 */
    { INET_CLASSC_C_SHARP,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC_SHARP },	/* 223.0.0-223.255.255 */
#else	/* INET_CLASS_C_SHARP */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 208.0.0-208.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 209.0.0-209.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 210.0.0-210.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 211.0.0-211.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 212.0.0-212.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 213.0.0-213.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 214.0.0-214.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 215.0.0-215.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 216.0.0-216.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 217.0.0-217.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 218.0.0-218.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 219.0.0-219.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 220.0.0-220.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 221.0.0-221.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 222.0.0-222.255.255 */
    { INET_CLASSC_C,	INET_CLASSF_NETWORK,	INET_MASK_CLASSC },	/* 223.0.0-223.255.255 */
#endif	/* INET_CLASS_C_SHARP */

    /* Class D (Multicast) */

    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 224.0.0.0-224.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 225.0.0.0-225.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 226.0.0.0-226.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 227.0.0.0-227.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 228.0.0.0-228.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 229.0.0.0-229.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 230.0.0.0-230.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 231.0.0.0-231.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 232.0.0.0-232.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 233.0.0.0-233.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 234.0.0.0-234.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 235.0.0.0-235.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 236.0.0.0-236.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 237.0.0.0-237.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 238.0.0.0-238.255.255.255 */
    { INET_CLASSC_D,	INET_CLASSF_MULTICAST,	INET_MASK_CLASSD },	/* 239.0.0.0-239.255.255.255 */

#ifdef	INET_CLASS_E

    /* Class E */

    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 240.0.0-240.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 241.0.0-241.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 242.0.0-242.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 243.0.0-243.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 244.0.0-244.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 245.0.0-245.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 246.0.0-246.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 247.0.0-247.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 248.0.0-248.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 249.0.0-249.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 250.0.0-250.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 251.0.0-251.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 252.0.0-252.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 253.0.0-253.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 254.0.0-254.255.255.240 */
    { INET_CLASSC_E,	INET_CLASSF_NETWORK,	INET_MASK_CLASSE },	/* 255.0.0-255.255.255.240 */
#else	/* INET_CLASS_E */

    /* Reserved */
    
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 240.0.0-240.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 241.0.0-241.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 242.0.0-242.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 243.0.0-243.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 244.0.0-244.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 245.0.0-245.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 246.0.0-246.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 247.0.0-247.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 248.0.0-248.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 249.0.0-249.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 250.0.0-250.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 251.0.0-251.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 252.0.0-252.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 253.0.0-253.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 254.0.0-254.255.255.240 */
    { INET_CLASSC_RESERVED,	INET_CLASSF_RESERVED,	INET_MASK_INVALID },	/* 255.0.0-255.255.255.240 */
#endif	/* INET_CLASS_E */
} ;


struct martians {
    const char *dest;
    const char *mask;
    flag_t flags;
};

static struct martians inet_martians[] = {
    {	"0.0.0.0",		"255.255.255.255",	0 },		/* Default */
    {	"0.0.0.0",		"255.0.0.0",		ADVF_NO },	/* Reserved */
    {	"127.0.0.0",		"255.0.0.0",		ADVF_NO },	/* Reserved */
    {	"128.0.0.0",		"255.255.0.0",		ADVF_NO },	/* Reserved */
    {	"191.255.0.0",		"255.255.0.0",		ADVF_NO },	/* Reserved */
    {	"192.0.0.0",		"255.255.255.0",	ADVF_NO },	/* Reserved */
    {	"223.255.255.0",	"255.255.255.0",	ADVF_NO },	/* Reserved */
#ifdef	INET_CLASS_E
    {	"255.255.240.0",	"255.255.240.0",	ADVF_NO },	/* Reserved */
#else	/* INET_CLASS_E */
    {	"240.0.0.0",		"240.0.0.0",		ADVF_NO },	/* Reserved */
#endif /* INET_CLASS_E */
    {	NULL,			NULL,			0 },
};


/* Optimized routine to locate an inet mask */
sockaddr_un *
inet_mask_locate __PF1(mask, u_int32)
{
    register sockaddr_un *mp;
	
    if (mask) {
	/* ffs returns the first bit set */
	register int i = ffs((int) ntohl(mask));
		
	if (i > 32 || mask != sock2ip(mp = inet_masks[33 - i])) {
	    /* Bogus mask or non contiguous */
	    mp = (sockaddr_un *) 0;
	}
    } else {
	/* The easy case */
	
	mp = inet_mask_default;
    }
	
    return mp;
}



/*
 * Return the mask for the given address, assumes fixed subnet mask from
 * the interface given.  Used by RIP and HELLO.
 */
sockaddr_un *
inet_mask_withif __PF3(addr, sockaddr_un *,
		       ifap, if_addr *,
		       state, flag_t *)
{
    int subnet = FALSE;
    register sockaddr_un *mask;
    
    /* Get the appropriate subnet mask for this destination */
    if (inet_net_natural(addr) == sock2ip(ifap->ifa_net)
#ifndef	VARIABLE_MASKS
	/* BSD 4.3 and earlier assume the same subnet mask for all interfaces */
	|| (ifap == if_withnet(addr))
#endif	/* VARIABLE_MASKS */
	) {
	/* Same net, assume same subnet mask */

	if (BIT_TEST(ifap->ifa_state, IFS_SUBNET)
	    || (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)
		&& ifap->ifa_subnetmask != inet_mask_host)) {
	    mask = ifap->ifa_subnetmask;
	    if (state) {
		/* Indicate subnet mask was derived */
		BIT_SET(*state, RTS_IFSUBNETMASK);
	    }
	} else {
	    mask = ifap->ifa_netmask;
	    subnet = state != 0;
	}
    } else {
	/* default to the natural mask */

	mask = inet_mask_natural(addr);
    }

    if (sock2ip(addr) & ~sock2ip(mask)) {
	/* Host route */

	mask = inet_mask_host;
	if (subnet) {
	    BIT_SET(*state, RTS_IFSUBNETMASK);
	}
    }

    return mask;
}


/* Find the subnetmask for the specified address and return it in host byte */
/* order. If this system does not support variable subnet masks, the assumption */
/* is made that the subnet mask is the same throughout the network.  If variable */
/* subnet masks are supported, we will only match the specified interface */
sockaddr_un *
inet_ifsubnetmask __PF1(addr, struct in_addr)
{
    if_addr *ifap;

    IF_ADDR(ifap) {
	if (socktype(ifap->ifa_addr) == AF_INET &&
	    BIT_MATCH(ifap->ifa_state, IFS_UP|IFS_SUBNET)) {
	    switch (socktype(ifap->ifa_addr)) {
	    case AF_INET:
		if (
#ifndef	VARIABLE_MASKS
		    (sock2ip(ifap->ifa_netmask) & addr.s_addr) == sock2ip(ifap->ifa_net)
#else	/* VARIABLE_MASKS */
		    (sock2ip(ifap->ifa_subnetmask) & addr.s_addr) == sock2ip(ifap->ifa_subnet)
#endif	/* VARIABLE_MASKS */
		    ) {
		    goto Return;
		}
		break;

	    default:
		continue;
	    }
	}
    } IF_ADDR_END(ifap) ;

 Return:
    return ifap ? ifap->ifa_subnetmask : (sockaddr_un *) 0;
}


#ifdef	IP_MULTICAST
/**/

/* Drop the all routers multicast group when we are not functioning as a router */
void
inet_allrouters_drop __PF1(ifap, if_addr *)
{
    if (BIT_TEST(ifap->ifa_ps[RTPROTO_INET].ips_state, INET_IFPS_ALLROUTERS)) {

	if (!BIT_TEST(task_state, TASKS_TEST)) {
	    /* Not in test mode, drop group membership */

	    (void) task_set_option(inet_task,
				   TASKOPTION_GROUP_DROP,
				   ifap,
				   inet_addr_allrouters);
	}

	/* Indicate now dropped */
	BIT_RESET(ifap->ifa_ps[RTPROTO_INET].ips_state, INET_IFPS_ALLROUTERS);
    }
}


/* Join the all routers multicast group when we are functioning as a router */
void
inet_allrouters_join __PF1(ifap, if_addr *)
{
    if (socktype(ifap->ifa_addr) == AF_INET &&
	BIT_TEST(ifap->ifa_state, IFS_MULTICAST)) {

	if (ifap->ifa_rtactive) {
	    if (!BIT_TEST(ifap->ifa_ps[RTPROTO_INET].ips_state, INET_IFPS_ALLROUTERS)) {
		/* Try to add to this multicast interface */

		if (BIT_TEST(task_state, TASKS_TEST)
		    || (!task_set_option(inet_task,
					 TASKOPTION_GROUP_ADD,
					 ifap,
					 inet_addr_allrouters)
			|| (errno == EADDRNOTAVAIL)
			|| (errno == EADDRINUSE))) {
		    /* Indicate we enabled it on this interface */

		    BIT_SET(ifap->ifa_ps[RTPROTO_INET].ips_state, INET_IFPS_ALLROUTERS);
		}
	    }
	} else {
	    /* Make sure we are not added on this interface */

	    inet_allrouters_drop(ifap);
	}
    }
}
#endif	/* IP_MULTICAST */

#ifdef	AUTONOMOUS_SYSTEM
/**/

as_t	inet_autonomous_system = 0;	/* Our autonomous system */

/* Maintain a list of all local AS numbers referenced */

as_entry inet_as_root = { &inet_as_root, &inet_as_root };
static block_t inet_as_block_index;

as_entry *
inet_as_link __PF1(as, as_t)
{
    register as_entry *asp;

    AS_LIST(asp) {
	if (asp->as_as > as) {
	    break;
	} else if (asp->as_as == as) {
	    return asp;
	}
    } AS_LIST_END(asp) ;

    if (!asp) {
	asp = inet_as_root.as_back;
    }
    
    insque((struct qelem *) task_block_alloc(inet_as_block_index),
	   (struct qelem *) asp);
    asp = asp->as_forw;
    asp->as_as = as;
    asp->as_refcount++;

    return asp;
}

void
inet_as_unlink __PF1(as, as_t)
{
    register as_entry *asp;

    AS_LIST(asp) {
	if (asp->as_as > as) {
	    break;
	} else if (asp->as_as == as) {
	    remque((struct qelem *) asp);
	    task_block_free(inet_as_block_index, (void_t) asp);
	    break;
	}
    } AS_LIST_END(asp) ;

    return;
}

#endif	/* AUTONOMOUS_SYSTEM */

#ifdef	ROUTER_ID
/**/


if_addr_entry *inet_routerid_entry = 0;		/* Router ID */
static if_addr_entry *inet_routerid_config;	/* Requested Router ID */

/*
 *	Set the configured router_id
 */
int
inet_parse_routerid __PF2(addr, sockaddr_un *,
			  parse_error, char *)
{
    if (socktype(addr) != AF_INET) {
	(void) sprintf(parse_error, "routeid not an IP address");
	return TRUE;
    }
    if (inet_routerid_config) {
	(void) sprintf(parse_error, "routerid specified twice");
	return TRUE;
    }

    if (is_martian(addr)) {
	(void) sprintf(parse_error, "address invalid for routerid");
	return TRUE;
    }

    inet_routerid_config = ifae_locate(addr, &if_local_list);

    return FALSE;
}


/*
 *	Cleanup
 */
static void
inet_cleanup __PF1(tp, task *)
{
    if (inet_routerid_config) {
	ifae_free(inet_routerid_config);
	inet_routerid_config = (if_addr_entry *) 0;
    }
}


/*
 *	Select an initial routerID if possible
 */
static void
inet_routerid_select __PF0(void)
{
    if_addr *ifap = (if_addr *) 0;
    if_addr *ifap2;
	
    IF_ADDR(ifap2) {
    	if (!BIT_TEST(ifap2->ifa_state, IFS_UP)
	    || BIT_TEST(inet_class_flags(ifap2->ifa_addr_local), INET_CLASSF_LOOPBACK|INET_CLASSF_RESERVED|INET_CLASSF_DEFAULT)
	    || is_martian(ifap2->ifa_addr_local)) {
	    /* Ignore down interfaces, loopback addresses and martians */
	    continue;
	}

	if (!ifap) {
	    /* First valid one */

	    ifap = ifap2;
	} else {
	    /* We already have one, is this one better? */

	    if (BIT_TEST(ifap2->ifa_state, IFS_LOOPBACK)
		&& !BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)) {
		/* An address on the loopback interface is primo */

		ifap = ifap2;
	    }
	    if (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)
		&& !BIT_TEST(ifap2->ifa_state, IFS_POINTOPOINT)) {
		/* A non-p2p address is prefered */

		ifap = ifap2;
	    }
	}
    } IF_ADDR_END(ifap2) ;

    if (ifap) {
	inet_routerid_entry = ifae_alloc(ifap->ifa_addrent_local);
    }
}


/*
 *	Notify what the router id is
 */
static inline void
inet_routerid_notify __PF0(void)
{
    if (inet_routerid_entry) {
	trace(TR_ALL, 0, "inet_routerid_notify: Router ID: %A",
	      inet_routerid_entry->ifae_addr);
    } else {
	trace(TR_ALL, 0, "inet_routerid_notify: No Router ID assigned");
    }

    trace(TR_ALL, 0, NULL);
}

static inline void
inet_routerid_reinit __PF0(void)
{
    if (inet_routerid_config
	&& inet_routerid_entry != inet_routerid_config) {
	/* Our current selection is no longer valid */

	if (inet_routerid_entry) {
	    ifae_free(inet_routerid_entry);
	}

	inet_routerid_entry = ifae_alloc(inet_routerid_config);
    }

    if (!inet_routerid_entry) {
	/* Try to make an initial selection now to keep the protocols happy */

	inet_routerid_select();
    }

    inet_routerid_notify();
}
#endif	/* ROUTER_ID */


#if	defined(ROUTER_ID) || defined(ROUTE_AGGREGATION)
/*
 *	Reinit
 */
static void
inet_reinit __PF1(tp, task *)
{
#ifdef	ROUTER_ID
    inet_routerid_reinit();
#endif	/* ROUTER_ID */
}
#endif	/* defined(ROUTER_ID) || defined(ROUTE_AGGREGATION) */


#if	defined(ROUTER_ID)
/*
 *  Watch for interface changes to keep the router-ID up to date
 */
static void
inet_ifachange __PF2(tp, task *,
		     ifap, if_addr *)
{
#ifdef	ROUTER_ID
    int change = 0;
#endif	/* ROUTER_ID */
    
    if (socktype(ifap->ifa_addr) != AF_INET
	|| is_martian(ifap->ifa_addr_local)) {
	/* Not valid for Router ID */

	return;
    }
    
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
	/* Already chosen at reinit time */
	break;
	
    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	Up:
#ifdef	ROUTER_ID
	    Router_Up:
	    if (!inet_routerid_entry) {
		/* No Router ID, this must be the first valid interface */

		inet_routerid_entry = ifae_alloc(ifap->ifa_addrent_local);
		change++;
	    }
#endif	/* ROUTER_ID */
	}	
	break;

    case IFC_DELETE:
	/* Already down */
	break;
	
    case IFC_DELETE|IFC_UPDOWN:
    Down:
#ifdef	ROUTER_ID
    Router_Down:
	if (!inet_routerid_config
	    && ifap->ifa_addrent_local == inet_routerid_entry
	    && !inet_routerid_entry->ifae_n_if) {
	    /* Free this reference */

	    ifae_free(inet_routerid_entry);
	    inet_routerid_entry = (if_addr_entry *) 0;

	    /* Try to select a new one */
	    inet_routerid_select();

	    change++;
	}
#endif	/* ROUTER_ID */
	break;

    default:
	/* Something has changed */

	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		goto Up;
	    } else {
		goto Down;
	    }
	}

	if (BIT_TEST(ifap->ifa_change, IFC_ADDR)) {
	    /* Local address change, see if it effects the routerID */

#ifdef	ROUTER_ID
	    if (!inet_routerid_entry) {
		/* No routerid, try to select this one */
		goto Router_Up;
	    }

	    /* See if our routerid went away */
	    goto Router_Down;
#endif	/* ROUTER_ID */
	}
	/* METRIC - We don't care */
	/* NETMASK - We don't care */
	/* BROADCAST - We don't care */
	/* MTU - We don't care */
	break;
    }

#ifdef	ROUTER_ID
    if (change) {
	inet_routerid_notify();
    }
#endif	/* ROUTER_ID */
}
#endif	/* defined(ROUTER_ID) */


static void
inet_dump __PF2(tp, task *,
		fp, FILE *)
{
    (void) fprintf(fp, "\tIP forwarding: %d\tUDP checksums: %d\n\n",
		   inet_ipforwarding,
		   inet_udpcksum);

    if (inet_addr_reject || inet_addr_blackhole) {
	if (inet_addr_reject) {
	    (void) fprintf(fp, "\tReject address: %A",
			   inet_addr_reject);
	}
	if (inet_addr_blackhole) {
	    (void) fprintf(fp, "\tBlackhole address: %A",
			   inet_addr_blackhole);
	}
	(void) fprintf(fp, "\n\n");
    }
#ifdef	AUTONOMOUS_SYSTEM
    /* Print our local ASes */
    if (inet_autonomous_system) {
	as_entry *asp;

	(void) fprintf(fp, "\tAutonomous system: %u",
		       inet_autonomous_system);

	(void) fprintf(fp, "\tLocal autonomous systems\n");
	AS_LIST(asp) {
	    (void) fprintf(fp, "\t\tAS: %u\trefcount: %u\n",
			   asp->as_as,
			   asp->as_refcount);
	} AS_LIST_END(asp) ;    

	(void) fprintf(fp, "\n\n");
    }
#endif	/* AUTONOMOUS_SYSTEM */
    
#ifdef	ROUTER_ID
    if (inet_routerid_entry) {
	(void) fprintf(fp, "\tRouter ID: %A",
		       inet_routerid);

    (void) fprintf(fp, "\n\n");
    }
#endif	/* ROUTER_ID */
}


#ifdef	IP_MULTICAST
static void
inet_terminate __PF1(tp, task *)    
{
    if_addr *ifap;
	
    /* Leave the all routers multicast group */
    IF_ADDR(ifap) {
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    inet_allrouters_drop(ifap);
	}
    } IF_ADDR_END(ifap) ;

    task_delete(tp);
}
#endif	/* IP_MULTICAST */


void
inet_init __PF0(void)
{
    if (!inet_task) {
	inet_task = task_alloc("INET", TASKPRI_FAMILY);
#ifdef	ROUTER_ID
	inet_task->task_cleanup = inet_cleanup;
	inet_task->task_ifachange = inet_ifachange;
#endif	/* ROUTER_ID */
#if	defined(ROUTER_ID) || defined(ROUTE_AGGREGATION)
	inet_task->task_reinit = inet_reinit;
#endif	/* defined(ROUTER_ID) || defined(ROUTE_AGGREGATION) */
#ifdef	IP_MULTICAST
	inet_task->task_terminate = inet_terminate;
	inet_task->task_rtproto = RTPROTO_INET;
	inet_task->task_socket = task_get_socket(PF_INET, SOCK_DGRAM, 0);
#endif	/* IP_MULTICAST */
	inet_task->task_dump = inet_dump;
	if (!task_create(inet_task)) {
	    task_quit(EINVAL);
	}
    }

    /* Init martian list */
    {
	struct martians *ptr = inet_martians;

	/* Build list of martian networks */
	do {
	    struct in_addr mask;
	    struct in_addr dest;

	    if (!inet_aton(ptr->dest, &dest) ||
		!inet_aton(ptr->mask, &mask)) {
		trace(TR_ALL, LOG_WARNING, "inet_init: Invalid martian entry at %s/%s",
		      ptr->dest,
		      ptr->mask);
		continue;
	    }
	    martian_add(sockbuild_in(0, dest.s_addr),
			mask_locate(sockbuild_in(0, mask.s_addr)),
			ptr->flags);
	} while ((++ptr)->dest) ;
    }

#ifdef	ROUTER_ID
    /* Try to choose a Router ID now */
    inet_routerid_reinit();
#endif	/* ROUTER_ID */

#ifdef	ROUTE_AGGREGATION
    rt_aggregate_list_add(RTPROTO_DIRECT,
			  inet_addr_loopback,
			  inet_mask_host,
			  inet_mask_classa);
#endif	/* ROUTE_AGGREGATION */
}


/*
 *	Init all kinds of inet structures
 */
void
inet_family_init __PF0(void)
{
    int i = 32;
    u_int32 mask = 0;
    sockaddr_un **mp = inet_masks;
    sockaddr_un *addr;

#ifdef	AUTONOMOUS_SYSTEM
    inet_as_block_index = task_block_init(sizeof (as_entry));
#endif	/* AUTONOMOUS_SYSTEM */
    sock_block_index[AF_INET] = task_block_init(sizeof (inet_addr_default->in));

    /* Build all possible contiguous masks */
    *mp++ = mask_locate(sockbuild_in(0, htonl(mask)));
    while (i--) {
	mask |= 1 << i;
	*mp++ = mask_locate(sockbuild_in(0, htonl(mask)));
    }

    /* Build useful addresses */
    inet_addr_default = sockdup(sockbuild_in(0, htonl(INADDR_DEFAULT)));
    inet_addr_any = sockdup(sockbuild_in(0, htonl(INADDR_ANY)));
    inet_addr_loopback = sockdup(sockbuild_in(0, htonl(INADDR_LOOPBACK)));

    rt_table_init_family(AF_INET,
			 (int) (sizeof (addr->in) - sizeof (addr->in.sin_addr)) * NBBY,
			 inet_addr_default,
			 inet_mask_host);

#ifdef	IP_MULTICAST
    inet_addr_allrouters = sockdup(sockbuild_in(0, htonl(INET_ADDR_ALLROUTERS)));

    /* Set up interface bits to be printed */
    int_ps_bits[RTPROTO_INET] = inet_if_bits;
#endif	/* IP_MULTICAST */
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
