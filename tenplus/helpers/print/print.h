/*NOTUSED*/
/* @(#)46	1.3  src/tenplus/helpers/print/print.h, tenplus, tenplus411, GOLD410 7/18/91 12:42:52 */

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

/********************************************************************
* file:  print.h                                                    *
*                                                                   *
* Prototypes of all function defined in helpers/print               *
*                                                                   *
*********************************************************************/
 
char *PTfixcommand (char *);
char *PTskipws(register char *);
char *PTbasename (register char *);
char *PTctu(register char *);
int PTsetfds(register int *, register char *, register char *);
int Hinit (register char *);
int PTresetfds(register int *);
int PTrderr(register char *, register char *);
void PTstdprint (register FILE *);
void PTprtit (register ATTR *, register FILE *);
int PTrmtrlblks(register ATTR *);
char *PTgetdir(register char *);
void PTinvtext(register ATTR *, register int );
void PTgut (register ATTR *);
void PTstuff (register ATTR *, register WSTRUCT *, int );
int PTsystem(register char *);
void Hstop (void);
char *getstring (POINTER *, char *);
int parse_termname (char *, char *);
