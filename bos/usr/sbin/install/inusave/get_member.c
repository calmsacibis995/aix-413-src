static char sccsid[] = "@(#)06  1.7  src/bos/usr/sbin/install/inusave/get_member.c, cmdinstl, bos411, 9428A410j 6/18/93 12:30:09";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: get_member (inusave)
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
#include <inusave.h>

/*
 * NAME: get_member
 *
 * FUNCTION: extracts the member name of an archive control file from its
 *           full path
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS:  a pointer to the member name
 */
char *get_member(
char path[])		      /* full path of a	restored member	file  */
{
    int	last,		/* points to null char in member name   */
    i;			/* counter				*/

/* Begin get_member	*/

    last = -1;
    for (i=0; path[i] != NULL; i++)
	if (path[i] == '/')
	    last = i;
    return(&path[last+1]);
}
