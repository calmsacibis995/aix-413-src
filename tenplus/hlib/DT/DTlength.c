#if !defined(lint)
static char sccsid[] = "@(#)50	1.5  src/tenplus/hlib/DT/DTlength.c, tenplus, tenplus411, GOLD410 7/18/91 12:43:52";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	DTylength, DTmlength
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
 
#include <database.h>
#include <DT.h>

int	Dtmlength(int, int);


/* DTylength: find length of year 
   Arguments:
	int year: year of interest

   DTylength returns the number of days in the year.

   This routine is not intended for use outside of the DT library */

int DTylength (int year)

{
    year += 1900;

    if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
        return (366);
    return (365);
}

/* DTmlength: find length of month
   Argument:
	int month: month of interest
	int year: year of interest

   DTmlength returns the number of days in the month.

   This routine is not intended for use outside of the DT library */

int DTmlength (int month, int year)

{
    switch (month) {
	case APRIL:
	case JUNE:
	case SEPTEMBER:
	case NOVEMBER:
	    return (30);
	case FEBRUARY:
            return ((DTylength (year) == 366) ? 29 : 28);
	default:
	    return (31);
    }
}
