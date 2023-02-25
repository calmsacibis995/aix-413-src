#if !defined(lint)
static char sccsid[] = "@(#)79	1.5  src/tenplus/helpers/history/stdalone.c, tenplus, tenplus411, GOLD410 7/18/91 12:42:31";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hbefore, Hinit, Hsavestate, Hrestart
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
 
/****************************************************************************/
/* file: standalone.c - routines to interface to forms via the pipe         */
/* interface.                                                               */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

int Hbefore (int , int , char *, int );
int Hinit (char *);
char *Hsavestate (void);

/****************************************************************************/
/* Hbefore: calls H_ubefore                                                 */
/****************************************************************************/

int Hbefore (int key, int type, char *string, int number)
    {
    return (H_ubefore (key, type, string, number));
    }

/****************************************************************************/
/* Hinit: calls H_uinit                                                     */
/****************************************************************************/

int Hinit (char *filename)
    {
    return (H_uinit (filename));
    }

/****************************************************************************/
/* Hsavestate: calls H_usavestate                                           */
/****************************************************************************/

char *Hsavestate (void)
    {
    extern char *H_usavestate (void);

    return (H_usavestate ());
    }

/****************************************************************************/
/* Hrestart: calls H_urestart                                               */
/****************************************************************************/

int Hrestart (char *state)
    {
    return (H_urestart ((POINTER *)state));
    }
