#if !defined(lint)
static char sccsid[] = "@(#)70	1.6  src/tenplus/hlib/help/Urestart.c, tenplus, tenplus411, GOLD410 3/23/93 12:01:00";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hrestart
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

#include <stdio.h>
#include <database.h>
#include <edexterns.h>
#include <libhelp.h>
#include <libobj.h>

/****************************************************************************/
/* Hrestart:  called when helper environment is being restarted             */
/****************************************************************************/

/*ARGSUSED*/
int Hrestart (char *stateobj)

    {
    /* for compatibility with old helpers that do not distinguish between
       Hrestart and Hinit, the appropriate Hrestart stub is to determine
       the filename and call Hinit with it (ignoring the stateobj argument). */

    char *filename;
    int   retval;

    filename = Efilename ();
    retval = Hinit (filename);
    s_free (filename);
    return (retval);
    }

