#if !defined(lint)
static char sccsid[] = "@(#)40	1.5  src/tenplus/lib/prf/Pcheckbox.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:38";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Pcheckbox
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
/* Pcheckbox:  returns the value for a check-off box field                  */
/*                                                                          */
/* This routine takes a data path in the profile to a check-off box field.  */
/* If the field exists and its first line does not start with a space       */
/* character (as defined by isspace), then Pboolean will return TRUE, else   */
/* FALSE.  The data path may be simple or indexed (with an imbedded asterisk). */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include <ctype.h>

#include <libobj.h>
#include <libprf.h>

int Pcheckbox (char *datapath)
    /*datapath:    data path from the top of the profile file */
    {
    register char *field; /* field text   */
    register retval;      /* return value */

#ifdef DEBUG
    debug ("Pcheckbox:  datapath = '%s'", datapath);
#endif

    field = Pgetsingle (datapath);

    if (field == (char *) ERROR)
	return (FALSE);

    if ((field == NULL) || (*field == '\0') || (isspace (*field)))
	retval = FALSE;
    else
	retval = TRUE;

    s_free (field);
    return (retval);
    }


