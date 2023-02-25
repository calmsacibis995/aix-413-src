/*NOTUSED*/
/* @(#)88	1.6  src/tenplus/include/edexterns.h, tenplus, tenplus411, GOLD410 3/23/93 12:01:47 */
/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
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
 
/*****************************************************************************
* File: edexterns: functional interface to the editor.                       *
*                                                                            *
* Useful in helper routines.  Also describes functional interface to the     *
* editor library routines.                                                   *
*****************************************************************************/


/***** Routines that a helper can call in the editor *****/
extern char    *Efixvars (char *);
extern int      Eline (void);
extern int      Ecolumn (void);
extern char    *Egettext (char *, int);
extern int      Eputtext (char *, int , char *);
extern char     *s_nzalloc(int, int, char *);
extern void     Ettyunfix (void);
extern char    *Efilename (void);
extern char    *Egetmessage (int, char *, int);
extern int      Ecopyprofile (char *, char *);
extern int      Ecountlines (char *, int *);
extern int      Esizelines (char *, int[]);
extern int      Emenu (int, char *, int, char *);
extern int      Ecustommenu (int, char *, POINTER *, int);
extern int      Enumlines (char *);
extern void     Esaveascii (char *);
extern void     Ettyfix (void);
extern char    *Eask (int, char *, ...);
extern int      Eputcursor (char *, int, int);
extern void     Esethook (char *, char *);
extern int      Egetttline (int);
extern int      Egettbline (int);
extern int      Edelline (char *, int, int);
extern char    *Eformname (void);
extern char    *Egetpath (void);
extern void     Ereopen (void);
extern int      Euseform (char *);
extern void     Ermfile (char *);
extern void     Edispname (char *);
extern int      Egetflags (char *);
extern int      Enumsiblings (void);
extern int 	Ewritefield (char *, char *);
extern void	Esetpath(char *, char *);
extern int      Einsline (char *, int, int);
extern int      Ereadonly (void);
extern char    *Esync (void);
extern char    *Efieldname (void);
extern int      Eusefile (char *);
extern int      Esavefile (void);
extern void     Ekillhelper (void);
extern void     Eflushform (char *);
extern POINTER *Egetlist (char *, int, int);
extern int      Ekeyvalue (void);
extern int 	Econfirm(int, char *, ...);

