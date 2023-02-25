/* @(#)45	1.9  src/bos/usr/sbin/nimesis/nimesis.h, cmdnim, bos411, 9428A410j  6/14/94  16:31:33 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/nimesis/nimesis.h
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

#ifndef _H_NIMESIS
#define _H_NIMESIS
#include "cmdnim_ip.h"

typedef enum {
	NIM_PKT_NAME,
	NIM_PKT_CPUID,
	NIM_PKT_REPLY,
	NIM_PKT_NARGS,
	NIM_PKT_ARGS
} NIM_FIELDS;

#define MAX_RETRY	10

#define RC(rc, a, b, c, d) \
 	{	nene(a, b, c, d); \
		buglog(" ");	\
		return(rc);  \
	}
#endif /* _H_NIMESIS */
