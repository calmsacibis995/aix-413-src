/* @(#)55	1.4  src/tenplus/sf_progs/tree.h, tenplus, tenplus411, GOLD410 3/23/93 12:11:32 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/**************************************************************
* file  tree.h                                                *
**************************************************************/
 
void tree(POINTER *, int, int, int );

/* General message catalogue access for readfile and tree */
#include "readfile_msg.h"

#define MSGSTR(num,str) catgets(g_catd, MS_READFILE, num, str)
