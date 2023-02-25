/* @(#)17       1.1  src/rspc/usr/ccs/lib/libpm/pmlibdebug.h, pwrcmd, rspc41J, 9510A 3/8/95 07:41:34 */
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: Power Management Library
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PMLIBDEBUG
#define _H_PMLIBDEBUG

#ifdef PM_DEBUG
#define DEBUG_0(A)			fprintf(stderr,A);
#define DEBUG_1(A,B)			fprintf(stderr,A,B);
#define DEBUG_2(A,B,C)			fprintf(stderr,A,B,C);
#define DEBUG_3(A,B,C,D)		fprintf(stderr,A,B,C,D);
#define DEBUG_4(A,B,C,D,E)		fprintf(stderr,A,B,C,D,E);
#define DEBUG_5(A,B,C,D,E,F)		fprintf(stderr,A,B,C,D,E,F);
#define DEBUG_6(A,B,C,D,E,F,G)		fprintf(stderr,A,B,C,D,E,F,G);
#else	/* PM_DEBUG */
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#endif /* PM_DEBUG */

#endif /* _H_PMLIBDEBUG */
