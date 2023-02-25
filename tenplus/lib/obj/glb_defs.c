#if !defined(lint)
static char sccsid[] = "@(#)63	1.5  src/tenplus/lib/obj/glb_defs.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:09";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *                      Copyrighted as an unpublished work.
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, 1988
 *	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
 * 
 *                      Copyrighted as an unpublished work.
 *                         INTERACTIVE TEN/PLUS System
 *                        TEN/PLUS Programmer's Took Kit
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, 1988
 *                             All rights reserved.
 *
 *   RESTRICTED RIGHTS
 *   These programs are supplied under a license.  They may be used and/or
 *   copied only as permitted under such license agreement.  Any copy must
 *   contain the above notice and this restricted rights notice.  Use,
 *   copying, and/or disclosure of the programs is strictly prohibited
 *   unless otherwise provided in the license agreement.
 *                                                                          
 *   TEN/PLUS is a registered trademark of INTERACTIVE Systems Corporation
 */

#if !defined(lint)
static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
#endif

/****************************************************************************/
/* glb_defs:  global definitions which do not pertain directly to the       */
/* global type descriptor table.                                            */
/****************************************************************************/

#include    <stdio.h>

FILE *g_debugfp = stderr;   /* file pointer for tracing and print routines */

    /* E_NOERR == 0, so we are taking advantage of g_errno being  */
    /* initialized to zero to avoid having to include database.h. */
    /* This is desirable since database.h contains many externs   */
    /* that we really don't want in this file.                    */
int   g_errno;
int   g_flags = 0;          /* global flag bits                            */
void  (*g_allocfcn)() = NULL;/* allocation failure function                 */
void  (*g_typefcn)() = NULL; /* type check failure function                 */
int  (*g_diskfcn)() = NULL; /* full disk error handler                     */

