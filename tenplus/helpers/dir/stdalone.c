#if !defined(lint)
static char sccsid[] = "@(#)29	1.5  src/tenplus/helpers/dir/stdalone.c, tenplus, tenplus411, GOLD410 7/18/91 12:40:49";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hafter, Hbefore, Hsavestate, Hrestart, Hstop, Hinit,
 *		Hins, Hmod
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
/* File: standalone.c - routines to interface to forms via the pipe         */
/* interface.  NOTE: this file cannot include variables.h.                  */
/****************************************************************************/

#include <database.h>
#include <sys/types.h>
#include "index.h" 
#include "hdir.h"

/****************************************************************************/
/* Hafter: calls I_hafter.                                                  */
/****************************************************************************/


void Hafter( int key )
{
    I_hafter( key );
}


/****************************************************************************/
/* Hbefore: calls I_hbefore.                                                */
/****************************************************************************/

int Hbefore( int key, int i, char *s, int n )
{
    return( I_hbefore( key, i, s, n ) );
}

/****************************************************************************/
/* Hsavestate: calls I_hsavestate                                           */
/****************************************************************************/

char *Hsavestate (void)
{
    return( I_hsavestate() );
}


/****************************************************************************/
/* Hrestart: calls I_hrestart                                               */
/****************************************************************************/

int Hrestart(char *state)
{
    return( I_hrestart (state) );
}


/****************************************************************************/
/* Hstop: calls I_hstop                                                     */
/****************************************************************************/

void Hstop(void)
{
     I_hstop ();
}



/****************************************************************************/
/* Hinit: calls I_hinit.                                                    */
/****************************************************************************/

int Hinit( ASTRING s )
{
#ifdef	DEBUG
    debug ("Hinit(\"%s\")", s);
#endif
    return (I_hinit( s ));
}


/****************************************************************************/
/* Hins: calls I_hins.                                                      */
/****************************************************************************/

void Hins( char *fieldname, int line, int count )
{
    I_hins( fieldname, line, count );
}


/****************************************************************************/
/* Hmod: calls I_hmod.                                                      */
/****************************************************************************/

int Hmod( char *fieldname, int line, char *oldtext, char *newtext )
{
    return( I_hmod( fieldname, line, oldtext, newtext ) );
}
