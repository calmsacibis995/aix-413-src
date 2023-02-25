static char sccsid[] = "@(#)50  1.9  src/bos/usr/sbin/install/inulib/getbasename.c, cmdinstl, bos411, 9428A410j 6/17/93 16:13:22";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: getbasename (inulib)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <string.h>

/*
 * NAME: getbasename
 *
 * FUNCTION:
 *	Given entire file specification, return a pointer to
 *	the beginning of the individual filename.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * ON ENTRY:
 *	path	pointer to a string containing a path name
 *
 * RETURNS:
 *	Return value is a pointer to the char within the string
 *	that is the beginning of the individual filename.
 */
char *getbasename(
char	*path)		/* pointer to string containing filename	*/
{
char	*p;		/* pointer into name				*/

	if((p=strrchr(path,'/'))==NULL)
		return(path);
	else
		return(++p);
}
