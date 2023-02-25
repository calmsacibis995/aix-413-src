/* @(#)37	1.2  src/bos/usr/sbin/pse/utils/mtest/mtest.h, cmdpse, bos411, 9428A410j 5/18/91 20:27:43 */
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1989  Mentat Inc.
 ** mtest.h 1.9, last change 10/20/90
 **/

#ifndef	_MTEST_
#define	_MTEST_

#include "osmtest.h"

/* from Mentat propriatary ip.h */
/** IP Address Structure. */
typedef	struct ipaddr_s	{
	u16	ip_family;	/* Presumably AF_INET. */
	u_char	ip_port[2];	/* Port	in net-byte order. */
	u_char	ip_addr[4];	/* Internet addr. in net-byte order. */
	u_char	ip_pad[8];
} ipa_t;

#define	STACK_SIZE	4096

#define	CLIENT_SEND	1
#define	CLIENT_RECV	2
#define	CLIENT_SNDU	3
#define	CLIENT_WRITE	4
#define	CLIENT_READ	5
#define	CLIENT_NPWRITE	6
#define	CLIENT_NPREAD	7
#define	CLIENT_NPCALL	8
#define	CLIENT_NPTRANSC	9
#define	CLIENT_NPTRANSS	10
#define	CLIENT_NPPEEK	11
#define	CLIENT_NPGET	12
#define	CLIENT_DISC	13
#define	CLIENT_IMSNDU	14
#define	CLIENT_ECHO	15
#define	CLIENT_NSND	16
#ifdef	NETWARE
#define	CLIENT_NWPUT	17
#endif

/* Note that there is a symmetry relationship between the SERVER and CLIENT definitions that is crucial! */
#define	SERVER_RECV	1
#define	SERVER_SEND	2
#define	SERVER_READ	4
#define	SERVER_WRITE	5
#define	SERVER_NPREAD	6
#define	SERVER_NPWRITE	7
#define	SERVER_NPTRANSS	9
#define	SERVER_NPTRANSC	10
#define	SERVER_NPPEEK	11
#define	SERVER_NPGET	12

#define	XSTST_SOCKET	51
#define	XDGTST_SOCKET	52

#define	XDGTST_IDLE	0
#define	XDGTST_START	1
#define	XDGTST_STOP	2

#define	TCPTST_PORT		2012
#define	UDPTST_PORT		2012
#define	UDP_DISCARD_PORT	9
#define	UDP_ECHO_PORT		7

#define	UNIT_DATA_TEST_MAX_SIZE	4096

typedef	struct cmd_s {
	char	* cmd_cmd;
	char	* cmd_help;
} CMD, * CMDP;

typedef	struct tests_s {
	pfi_t	tests_server;
	char	* tests_name;
	CMDP	* tests_cmds;
	pfi_t	tests_client;
} TESTS, * TESTSP;

typedef	struct	pfm_s {
	char	* pfm_name;
	TESTSP	pfm_tests;
} PFM, * PFMP, ** PFMPP;

/* We may have lots of duplicate routine names */
#ifdef	staticf
#undef	staticf
#endif
#define	staticf	static

#endif	/*_MTEST_*/
