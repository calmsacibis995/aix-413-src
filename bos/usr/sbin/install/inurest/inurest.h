/* @(#)04       1.10  src/bos/usr/sbin/install/inurest/inurest.h, cmdinstl, bos411, 9428A410j 4/26/91 14:36:26 */
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inurest.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _H_INUREST
#define _H_INUREST

#include <stdio.h>
#include <inuerr.h>

#ifndef _NO_PROTO	/* if compiler supports prototyping */
int location(char *, char *, FILE *);
int is_diskette(char *);
#else			/* if compiler does not support prototyping */
int location();
int is_diskette();
#endif	/* _NO_PROTO */

#endif	/* _H_INUREST */
