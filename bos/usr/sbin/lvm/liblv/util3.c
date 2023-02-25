static char sccsid[] = "@(#)42	1.5  src/bos/usr/sbin/lvm/liblv/util3.c, cmdlvm, bos411, 9428A410j 5/30/91 15:48:45";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: parsepair getregions 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 *
 * FILENAME: util3.c
 *
 * FILE DESCRIPTION: Source file for some common functions used by the Logical Volume high-level commands.
 *
 * FUNCTION_NAMES: parsepair, getregions, getkmid
 *
 */


#include <stdio.h>
#include <string.h>
#include "lvm.h"
#include "cmdlvm.h"





/*
 * NAME: parsepair
 *
 * FUNCTION: Parse and convert a pair of coupled ASCII data values - either "pvid:ppnum" or "vgid.minor_number".
 *
 * RETURN CODE:          0 == success
 *                negitive == failure
 */

int parsepair (string, delimiter, min_size, max_size, id_value, int_value)
char
    *string,    /* data pair to be parsed (INPUT) */
    *delimiter; /* delimiter seperating the data values in pair (INPUT) */
int
    min_size,   /* minimum size of the data pair (characters) (INPUT) */
    max_size,   /* maximum size of the data pair (INPUT) */
    *int_value; /* value for number element (right-most) of input pair (OUTPUT) */
struct unique_id
    *id_value;  /* value for id of input pair (OUTPUT) */
{
	int return_code = 0;
	char *ptr, delim, copystr[STRSIZE];

	strcpy(copystr, string);                /* make copy so original string is not modified */
	if ((strlen(copystr) < min_size) ||     /* validate string size is within bounds */
	    (strlen(copystr) > max_size) ||
	    !(ptr=strpbrk(copystr, delimiter))) /* confirm delimiter exist - find its location */
		return_code = -1;
	else {
		sscanf (ptr+1, "%d", int_value);  /* read/convert the number portion - right of delimiter location */
		if ((ptr-copystr)>8) {  /* check size of the id - upto 16 ASCII hexidecimal digits */
			sscanf (ptr-8, "%8x%c", &id_value->word2, &delim);  /* read/convert 8 least significant digits */
			if (delim != *delimiter)  /* the delimiter must follow the 8 digits */
				return_code = -2;
			*(ptr-8) = (char) 0;
			sscanf (copystr, "%x", &id_value->word1);  /* read/convert 8 most significant digits of id */
		}
		else if ((ptr-copystr)>0) {  /* if id portion exist - it must contains 8 or less digits */
			id_value->word1 = 0;   /* since small number - set high/unused bits to zero */
			delim = *(ptr) = '\0';
			sscanf (copystr, "%x%c", &id_value->word2, &delim); /* read/convert the small id */
			if (delim != '\0')  /* if any non-hexidecimal digits were encountered */
				return_code = -2;
		}
		else
			return_code = -3;  /* error - no id portion of string */
	}
	return (return_code);
}




/*
 * NAME: getregions
 *
 * FUNCTION:  Calculates the five region boundaries for a physical volume.
 *
 * RETURN VALUE: pointer to struct containing starting and ending pp for each region
 *
 */

struct REGION *getregions (numpps)
int numpps;  /* number of partitions in pv (INPUT) */
{
	static struct REGION regions[SREGION];  /* the pp bounds for each region (OUTPUT) */

	int i, low = 1;                  /* index and counter */
	static int count[SREGION];       /* holds number of pps in each region */
	static int extra_index[SREGION] = { 0, 4, 1, 3, 2   };  /* defines assignment order for extra pps */

	/* Find number of partitions in each region. */
	/* first, find the number that evenly fits all the regions */
	for (i=0; i<SREGION; i++)
		count[i] = numpps / SREGION;
	/* second, add any remaining partitions using defined order for adding extras */
	for (i=0; i<(numpps % SREGION); count[extra_index[i++]]++);


	/* Using the number of pps in each region, find the lowest pp number and
	   the highest for each region. */
	for (i=0; i<SREGION; i++) {
		regions[i].low = low;
		regions[i].high = (low += count[i]) - 1;
	}
	return (regions);
}

/*************************************************************************
*                                                                        *
* NAME: getkmid                                                         *
*                                                                        *
* FUNCTION: Get the kernel module identification number.                 *
*                                                                        *
* INPUT:   NONE                                                          *
*                                                                        *
* OUTPUT:                                                                *
*           kmid_ptr     A POINTER to a structure used to store          *
*                        the kernel module id.                           *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION                                               *
*                          0 Function successfully completed             *
*                          1 Unsuccessfully completed                    *
*                                                                        *
*                                                                        *
**************************************************************************/

int getkmid (kmid_ptr)
mid_t *kmid_ptr;
{
  FILE *kmid_file_ptr;

 *kmid_ptr = (mid_t)loadext("hd_pin",FALSE,TRUE);
 if (kmid_ptr == NULL)
   {
     return (NOKMID_FOUND);
   }
 else
   {
     return (NO_ERROR);
   } /* endif */

}

