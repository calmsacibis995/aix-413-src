/* @(#)68	1.5  src/tenplus/tools/fc/fc.h, tenplus, tenplus411, GOLD410 7/18/91 13:56:19 */

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
 */
 
/*****************************************************************************
* fc.h: defines and variable declarations for the sources in this dir.    *
*****************************************************************************/


extern char    *g_filename;

extern void     bx_init(void);
extern POINTER *findboxes(SFILE *);
extern int      compile(void);
extern void     error(char *, ...);
extern void     Efatal(char *, ...);
extern void     Eerror(char *, ...);

