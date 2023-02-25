static char sccsid[] = "@(#)03	1.6  src/tenplus/interface/editor.c, tenplus, tenplus411, GOLD410 3/23/93 12:02:34";

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
 
/****************************************************************************/
/* File: editor.c - used to generate the eimports.c file                    */
/****************************************************************************/

#if BLTHELP || PIPEHELP
#include    <stdio.h>
#include    "pipe.h"

/* pick up the list of functions defined elsewhere in the editor        */

#include    "edltab1.c"

/* pick up definitions of functions which may be executed locally       */

LOCALSTRUCT g_locals [] = {
{0, 0, 0},          /* these slots correspond to RETURN and RETURN-ERROR*/
{0, 0, 0},

#include    "edltab2.c"
};


/* pick up definitions of functions which may be executed remotely      */

REMOTESTRUCT g_remotes [] = {
{0, 0},             /* these slots correspond to RETURN and RETURN-ERROR*/
{0, 0},

#include    "edrtab2.c"
};

#endif
/* pick up stubs to call remote functions                               */

#ifndef LOADHELPERS
#include    "edrtab1.c"
#endif
