/* @(#)27	1.6  src/tenplus/e2/include/termcap.h, tenplus, tenplus411, GOLD410 3/23/93 11:55:12 */

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

#include <curses.h>
/* the following are low-level curses routines,                     */
/* but they do not have prototypes in the header file               */
extern	int	fixterm(void);
extern	int	putp(char *);
extern	int	resetterm(void);
extern	int	setupterm(char *, int, int *);
extern	char *	tparm(char *, int, ...);
extern	int	tputs(char *, int, int(*)());
#include <term.h>

/* the following table represents the bit position, esc sequence to enter,
   and esc sequence to exit each of the defined attributes.  It is set in
   St_getcap() and is used in St_outchar().                             */

struct modestab
{
    short bitnum;
    char *getin;
    char *getout;
};
