/*NOTUSED*/
/* @(#)50	1.4  src/tenplus/include/libhelp.h, tenplus, tenplus411, GOLD410 3/23/93 12:01:57 */

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

/*
Prototypes of all function defined in hlib/help
*/
 
extern void Hafter (int );
extern void Halarm (void);
extern void errx(void);
extern void dosetup (FILE *, FILE *);
extern char *doremote (int , ...);
extern int Hbefore (int , int , char *, int );
extern char *Eask(int, char *, ...);
extern int Econfirm(int, char *, ...);
extern void Eerror(int, char *, ...);
extern void Efatal(int, char *, ...);
extern void Emessage(int, char *, ...);
extern void fatal(char *, ...);
extern POINTER	*makelist(char **);
extern int Ediskfull(char *);
extern void Hdel (char *, int , int );
extern int Hinit (char *);
extern void Hins (char *, int , int );
extern int Hmod (char *, int , char *, char *);
extern int Hrestart (char *);
extern void Hstop (void);
extern char *Iask (int, char *);
extern int Iconfirm (int, char *);
extern int Ierror (int , char *);
extern int Ifatal (int , char *);
extern int Imessage (int , char *);
extern int dolocal (void);
