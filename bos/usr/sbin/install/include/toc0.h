/* @(#)02       1.4  src/bos/usr/sbin/install/include/toc0.h, cmdinstl, bos411, 9428A410j 6/17/93 16:08:04 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef __H_TOC0
#define __H_TOC0

extern char     ClassStr[];
extern char     PlatformStr[];
extern char     YesNoStr[];
extern char     ContentStr[];

/*-------------------------------------------------------------------*
 * Defines
 *-------------------------------------------------------------------*/

#define PERCENT_FOUND   2
#define R_BRACE_FOUND   3

/*-------------------------------------------------------------------*
 * TOC header formats
 *-------------------------------------------------------------------*/

#define PRELOAD_HEADER_FMT      "%d %c\n"
#define ITEMS_IN_PRELOAD_HDR    2

#define TAPE_HEADER_FMT         "%d %d %s\n"
#define ITEMS_IN_TAPE_HDR       3

/*-------------------------------------------------------------------*
 * BFF header formats
 *-------------------------------------------------------------------*/

#define SINGLE_BFF_HEADER_FMT   "%c %c %c"
#define ITEMS_IN_SINGLE_BFF     3

#define PRELOAD_BFF_HEADER_FMT  "%s %c %c %c"
#define ITEMS_IN_PRELOAD_BFF    4

/*-------------------------------------------------------------------*
 * Lpp-option line entry formats
 *-------------------------------------------------------------------*/

#define SINGLE_BFF_ENTRY_FMT    "%s %s %s %c %c %s %[^}\n]\n"
#define ITEMS_IN_SINGLE_BFF_ENTRY       7

#define PRELOAD_ENTRY_FMT       "%s %s %c %c %s %[^}\n]\n"
#define ITEMS_IN_PRELOAD_ENTRY  6

#define _3_1_TAPE_ENTRY_FMT     "%s %s %c %c %s %[^}\n]\n"
#define ITEMS_IN_3_1_TAPE_ENTRY 6

/*-------------------------------------------------------------------*
 * Number of tape marks to get to the TOC file
 *-------------------------------------------------------------------*/

#define TAPE_TOC_POSITION       3

#endif  /* ifndef __H_TOC0       */
