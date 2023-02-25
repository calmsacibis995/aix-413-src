#if !defined(lint)
static char sccsid[] = "@(#)41	1.7  src/tenplus/lib/prf/Pinit.c, tenplus, tenplus411, GOLD410 3/23/93 12:07:40";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Pinit
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
 
/****************************************************************************/
/* Pinit:  initializes the profile library for the current helper           */
/*                                                                          */
/* This routine initializes the profile library on a given profile file.    */
/* It takes the name of the helper and will search for the profile          */
/* associated with that helper in the directories that normally contain     */
/* helper profile information.  The file name is the helper name with "prf" */
/* appended to it.  Pinit returns RET_OK if the file was                    */
/* found, else ERROR.                                                       */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include <limits.h>
#include <editorprf.h>
#include <libobj.h>
#include <libprf.h>
#include <edexterns.h>

/* define paths */

#define SPRF     "%s/%sprf"

int Pinit (register char *helper)
    /*helper:    helper name    */
    {
    register char *directory; /* directory name */
    char absname [PATH_MAX];       /* full file name */

#ifdef DEBUG
    debug ("Pinit:  helper name = '%s'", helper ? helper : "NULL");
#endif

    directory = (char *)Efixvars (USER_PROFILE_DIR);
    (void) sprintf (absname, SPRF, directory, helper);
    s_free (directory);

    if (fexists (absname))
	return (Pfopen (absname, "r"));

    /* If user doesn't have his own copy, then use the system copy. */
    /* This implicitly uses the "prime" one if that is appropriate. */
    directory = (char *)Efixvars (SYS_PROFILE_DIR);
    (void) sprintf (absname, SPRF, directory, helper);
    s_free (directory);
    return (Pfopen (absname, "r"));
    }

