static char sccsid[] = "@(#)13	1.6  src/bos/usr/sbin/install/inulib/in_al.c, cmdinstl, bos411, 9428A410j 6/17/93 16:08:50";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: in_al (inucp)
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

#include <sys/limits.h>
#include <stdio.h>
#include <inudef.h>

/*
 * NAME: in_al
 *
 * FUNCTION: determines if the file specified by "member" is in the apply list
 *           pointed to by "al"
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS:
 *	TRUE if the file is in the apply list
 *	FALSE otherwise
 */
int in_al(
char *member,		/* pathname of file		*/
FILE *al)		/* file pointer of apply list	*/
{
	char al_member[PATH_MAX+1];	/* pathname of file in apply list */

/* Begin in_al */

	rewind(al);		/* reset file pointer to beginning */

	/* While not the end of the apply list do.... */
	while (fscanf(al, "%s", al_member) != EOF) {

		/* compare the given file to the file in the apply list */
		if(strcmp(al_member, member) == 0)
			return(TRUE);

	} /* end while(!=EOF) */

	return(FALSE);
}
