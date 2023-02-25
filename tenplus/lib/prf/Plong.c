#if !defined(lint)
static char sccsid[] = "@(#)43	1.5  src/tenplus/lib/prf/Plong.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:47";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Plong
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
/* Plong:  returns the value of a 32 bit integer field                      */
/*                                                                          */
/* This routine is similar to Pshort but is used for 32 bit integers.  It   */
/* takes a data path in the profile and returns a long integer.  If the     */
/* field does not exist, ERROR is returned and g_errno is set to E_NOFIELD. */
/* If the field exists but does not contain a valid integer specification,  */
/* ERROR is returned and g_errno is set to E_BADFIELD.  Plong sets g_errno  */
/* to E_NOERR when it returns a valid long integer.                         */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include <ctype.h>

#include <libobj.h>
#include <libprf.h>

long Plong (char *datapath)
    /*datapath:    data path from the top of the profile file */
    {
    register char *cp;    /* datapath character pointer */
    register long number; /* return value               */
    register char *field; /* text of the field          */
    register sign;        /* -1 if "-" seen             */
    register digits;      /* number of digits           */

#ifdef DEBUG
    debug ("Plong:  datapath = '%s'", datapath);
#endif

    field = Pgetsingle (datapath);

    if (field == (char *) ERROR)
	{
	g_errno = E_NOFIELD;
	return (ERROR);
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

	if ((++digits > 10) || (number < 0L))
	    {
	    s_free (field);
	    g_errno = E_BADFIELD;
	    return ((long) ERROR);
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
    return (number);
    }


