static char sccsid[] = "@(#)92  1.8  src/bos/usr/sbin/grpck/comfix.c, cmdsadm, bos411, 9428A410j 3/25/94 14:46:14";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: init_query, ck_query
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <nl_types.h>
#include <langinfo.h>

/*
 * Global variables
 */

extern	int	fixit;
extern	int	query;
static	char	*yes_string;
static	char	*no_string;

/*
 * NAME:	init_query
 *
 * FUNCTION:	Initialize strings for default response.
 *
 * RETURN VALUE:
 *	NONE
 */

static void
init_query (void)
{
	char	*cp;

	/*
	 * Get the default YES answer and save it in a static variable.
	 */

	cp = nl_langinfo(YESSTR);
	if (*cp == '\0')
		cp = "yes";

	if (! (yes_string = malloc (strlen (cp) + 1))) {
		perror ("malloc");
		exit (ENOMEM);
	}
	strcpy(yes_string, cp);

	/*
	 * Get the default NO answer and save it in a static variable.
	 */
	cp = nl_langinfo(NOSTR);
	if (*cp == '\0')
		cp = "no";

	if (! (no_string = malloc (strlen (cp) + 1))) {
		perror ("malloc");
		exit (ENOMEM);
	}
	strcpy(no_string, cp);
}

/*
 * NAME:	ck_query
 *
 * FUNCTION:	Query user for an action to perform
 *
 * SPECIAL NOTES:
 *	ck_query will always return fixit if standard input is
 *	not a TTY.
 *
 * RETURN VALUE:
 *	1 for an affirmative response. 0 for a negative response.
 */

int
ck_query (char *msg, char *arg)
{
	char	buf[BUFSIZ];
	char	*cp;

	/*
	 * Don't have to bother with all this if the interactive
	 * mode isn't turned on.
	 */

	if (! query)
		return fixit;

	/*
	 * Get the values for the default responses.
	 */

	if (! yes_string )
		init_query ();

	/*
	 * Output the prompt (with an optional argument).
	 */

	fprintf (stderr, msg, arg);

	fprintf (stderr, "(%s, %s) ", yes_string, no_string);
	fflush (stderr);

	/*
	 * Read the user's reply.
	 */

	fgets (buf, BUFSIZ, stdin);
	buf[BUFSIZ-1] = '\0';

	if (cp = strrchr (buf, (int) '\n'))
		*cp = 0;

	switch(rpmatch(buf)) {
	      case 1:			/* Yes */
		return 1;
	      case 0:			/* No */
		return 0;
	}

	/*
	 * Wasn't YES or NO - Assume NO, output a NO-like reponse, and
	 * return 0.
	 */

	fprintf (stderr, "%s\n", no_string);
	fflush (stderr);
	return 0;
}
