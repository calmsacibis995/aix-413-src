#if !defined(lint)
static char sccsid[] = "@(#)52	1.5  src/tenplus/hlib/DT/DTlookup.c, tenplus, tenplus411, GOLD410 7/18/91 12:43:55";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	DTfind_entry
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
 
#include <string.h>
#include <database.h>

/* DTfind_entry: lookup string in list
   Arguments:
	char *name: item to look up
	char *list[][2]: two dimensional list of strings
        int   nbrrows: the number of rows (first dimension). (The
                       number of columns (second dimension) is 
                       always 2.)

   DTfind_entry returns the row of the list the item was found in, or ERROR
   if it was not found. The structure of the list is and array of pointers 
   to arrays of pointers to character strings. For example a list might look
   like:

       ptr->   [0] -> [0] January
	              [1] Jan

               [1] -> [0] February
	              [1] Feb

               ...

   This routine is not intended for use outside of the DT
   library. */

int DTfind_entry (char *name, char *list[][2], int nbrrows)
{
  int row, column;

  for (column = 0; column < 2; column++)
    for (row = 0; row < nbrrows; row++)
      if (strcmp(list[row][column], name) == 0)
        return(row);     

  return (ERROR);
}
