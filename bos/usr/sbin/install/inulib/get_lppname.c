static char sccsid[] = "@(#)94	1.6  src/bos/usr/sbin/install/inulib/get_lppname.c, cmdinstl, bos411, 9428A410j 6/17/93 16:13:09";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: get_lppname
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: get_lppname
 *
 * FUNCTION:
 *	Get next LPP name listed in file pointed to by fp.
 *	Leave pointer at next line.  Put name in str.
 *	Skip any line beginning with a #.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * ON ENTRY:
 *	fp	file pointer to open file containing list of LPP
 *		and LPP option names
 *	str	string buffer in which to store name
 *
 * RETURNS:
 *	SUCCESS		if a new name is obtained
 *	FAILURE		if no more names exist
 */
#include <stdio.h>
#include <inudef.h>

int get_lppname(
FILE	*fp,			/* pointer to file with LPP names	*/
char	*str)			/* buffer in which to return name	*/
{
	char	*p;		/* generic char pointer		*/
	char	buf[BUFSIZ];	/* line from list of LPPs	*/
	static char pound='#';

	p = &pound;
	while ((p != NULL) && (*p == '#')) {
		p = fgets(buf, (int)sizeof(buf), fp);
	}

	if (p == NULL) return(FAILURE);		/* if the fgets failed*/

	sscanf(buf, "%s", str);

	return(SUCCESS);

}
