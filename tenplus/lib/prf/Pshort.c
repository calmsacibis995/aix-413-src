#if !defined(lint)
static char sccsid[] = "@(#)44	1.5  src/tenplus/lib/prf/Pshort.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:50";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Pshort
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
/* Pshort:  returns the value of an 16 bit integer field                    */
/*                                                                          */
/* This routine takes a data path in the profile file for a single line     */
/* integer field and returns the value of that field.  If the field does    */
/* not exist, ERROR is returned and the g_errno global is set to E_NOFIELD. */
/* If the field exists but does not contain a valid integer specification,  */
/* or if the number will not fit into a short integer, ERROR is returned    */
/* and g_errno is set to E_BADFIELD.  E_NOFIELD and E_BADFIELD are new      */
/* defines in the database.h include file.  An integer specification is an  */
/* optional minus sign followed by an optional space followed by a string   */
/* of numeric digits  (ala atoi), so the return value may be either         */
/* positive or negative.  The data path may be simple or indexed (with an   */
/* imbedded asterisk).  If the field value is a string that evaluates to    */
/* minus one, g_errno is set to E_NOERR to distinguish this case from the   */
/* error returns described above.                                           */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include <ctype.h>

#include <libobj.h>
#include <libprf.h>

short Pshort (char *datapath)
    /*datapath:     data path from the top of the profile file */
    {
    register char *cp;    /* datapath character pointer */
    register long number; /* return value               */
    register char *field; /* text of the field          */
    register sign;        /* -1 if "-" seen             */
    register digits;      /* number of digits           */

#ifdef DEBUG
    debug ("Pshort:  datapath = '%s'", datapath);
#endif

    field = Pgetsingle (datapath);

    if (field == (char *) ERROR)
	{
	g_errno = E_NOFIELD;
	return ((short) ERROR);
	}
    for (cp = field; *cp; cp++) /* skip leading white space */
	if (! isspace (*cp))
	    break;

    sign = 1;
    number = 0L;
    digits = 0;

    if (*cp == '-')
	{
	sign = -1;
	cp++;
	}
    else if (*cp == '+')
	cp++;

    while (isdigit (*cp))
	{
	number = (number * 10L) + *cp++ - '0';

	if ((++digits > 5) || (number > 32767L))
	    {
	    s_free (field);
	    g_errno = E_BADFIELD;
	    return ((short) ERROR);
	    }
	}
    while (isspace (*cp))
	cp++;

    if ((*cp != '\0') || (digits == 0))
	{
	g_errno = E_BADFIELD;
	number = ERROR;
	}
    else
	{
	g_errno = E_NOERR;
	number *= sign;
	}
    s_free (field);
    return ((short) number);
    }


