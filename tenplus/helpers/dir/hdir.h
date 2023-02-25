/*NOTUSED*/
/* @(#)44	1.3  src/tenplus/helpers/dir/hdir.h, tenplus, tenplus411, GOLD410 7/18/91 12:39:54 */

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
 
extern void I_hins(ASTRING, int, int);
extern int I_hbefore(int, int, ASTRING, int);

extern void Hafter(int);
extern int Hbefore(int, int, char *, int);
extern char *Hsavestate (void);
extern int Hrestart(char *);
extern void Hstop(void);
extern int Hinit(ASTRING);
extern void Hins(char *, int, int);
extern int Hmod( char *, int, char *, char *);

extern int bad_file_name( char *);
extern int I_hinit(ASTRING);
extern char *I_hsavestate(void);
extern int I_hrestart(char *);
extern void I_hstop(void);
extern int I_hmod(ASTRING, int, ASTRING, ASTRING);
extern struct tm * parsedate(char *);
extern int copyfile( char *, char *);
extern int uprepare( int, ASTRING, int, int);
extern void urestore(int);
extern int usave(int, ASTRING, int, int);
extern int check_bits(char *, int);
extern void I_hafter(int);
extern POINTER *putfield(POINTER *, char *, char *);
