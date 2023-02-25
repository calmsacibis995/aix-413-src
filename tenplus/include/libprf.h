/*NOTUSED*/
/* @(#)52	1.3  src/tenplus/include/libprf.h, tenplus, tenplus411, GOLD410 7/18/91 12:45:45 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10, 27
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

/*file: libprf.h*/

extern int Pcheckbox(char *);
extern int Pinit(char *);
extern int Pfopen(char *, char *);
extern void Pcleanup(void);
extern POINTER *Pgetmulti(char *);
extern char *Pgetsingle(char *);
extern long Plong(char *);
extern short Pshort(char *); 
