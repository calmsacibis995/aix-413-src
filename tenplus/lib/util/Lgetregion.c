/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)67	1.6  src/tenplus/lib/util/Lgetregion.c, tenplus, tenplus411, GOLD410 3/23/93 12:08:25";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Lgetregion
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
/* File:  Lgetregion.c                                                      */
/****************************************************************************/

#include <stdlib.h>

#include <chardefs.h>
#include <database.h>

/****************************************************************************/
/* Lgetregion:  parse region coordinates from string into set of ints.      */
/*     The string should consist of four integers separated by              */
/*     one or more spaces.                                                  */
/****************************************************************************/

void Lgetregion (char *string, int *ullptr, int *ulcptr, int *lrlptr, 
                 int *lrcptr)
    /* string:         string argument from Ubefore call                 */
    /* ullptr:         address of where to store upper left hand line    */
    /* ulcptr:         address of where to store upper left hand column  */
    /* lrlptr:         address of where to store lower right hand line   */
    /* lrcptr:         address of where to store lower right hand column */
    {
    int   nread;     /* how many numbers read                             */
    int   nvals[4];  /* to store values while reading                     */
    char  *nxtp;     /* points to character after last read integer */

    for (nread = 0; nread < 4; nread++)
	{
	/* if no more string left, we are in trouble */
	if ((string == NULL) || (*string == '\0'))
	    break;

	nvals [nread] = (int) strtoul(string, &nxtp, 10);

        if (string >= nxtp)
              /* an integer could not be formed */
              break;

        string = nxtp;

	}

    /* If we didn't get all four numbers, do an error return.
       Otherwise copy the array entries into the results */

    if (nread == 4)
	{
	*ullptr = nvals [0];
	*ulcptr = nvals [1];
	*lrlptr = nvals [2];
	*lrcptr = nvals [3];
	}
    else
	{
	*ullptr = ERROR;
	*ulcptr = ERROR;
	*lrlptr = ERROR;
	*lrcptr = ERROR;
	}
    }
