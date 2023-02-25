static char sccsid[] = "@(#)05	1.6  src/bos/usr/sbin/install/inulib/decapitate.c, cmdinstl, bos411, 9428A410j 6/17/93 16:12:44";
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS: decapitate
 *
 * ORIGIN: 27
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

#include <inulib.h>

/*
 * NAME: decapitate
 *
 * FUNCTION: removes the first part of a qualified option name and copies it
 *	     into first_name.  It returns a pointer to the rest of the pathname
 *	     after the "."
 *
 * RETURN VALUE DESCRIPTION: pointer to the "decapitated" string or NULL if the
 *			     string had no '.'
 */
char *decapitate(
char option_name[],	/* LPP option name which may contain periods (.)    */
char first_name[])	/* string which will contain the first component of
			   option_name					    */
{
	int i;		/* index into the character strings		    */

/* Begin decapitate	*/

	/* copy option_name into first_name until the first '.' or NULL	*/

	for(i = 0; option_name[i] != NUL && option_name[i] != '.'; i++)
		first_name[i] = option_name[i];

	/* NULL-terminate the first name		*/

	first_name[i] = NUL;

	/* Return value should point past the '.'	*/

	if (option_name[i] == '.')
		i++;

	return(&option_name[i]);
} /* End decapitate */
