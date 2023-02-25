/*NOTUSED*/
/* @(#)45	1.3  src/tenplus/helpers/history/history.h, tenplus, tenplus411, GOLD410 7/18/91 12:42:24 */

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
 
extern int Hbefore (int, int, char *, int);
extern int H_ubefore (int, int, char *);
extern int Hinit (char *);
extern char *Hsavestate (void);
extern int Hrestart (char *);


extern void H_u9 (void);
extern int H_addfield (void);
extern int H_cookie (FILE *, FILE *);
extern int H_com_init (void);
extern void H_exit (void);
extern char *H_formprefix (char *);
extern int H_mkhistfile (void);
extern int H_mktmpnames (char *);
extern int H_record (FILE *, FILE *);
extern void H_showframe (int);
extern void H_sigcatch (int);
extern void H_u1 (void);
extern void H_u2 (void);
extern void H_u3 (void);
extern void H_u4 (void);
extern void H_u5 (int, char *);
extern int H_uinit (char *);
extern int H_urestart (POINTER *);
extern char *H_usavestate (void);
extern int H_zoomin (void);
extern int H_zoomout (void);
