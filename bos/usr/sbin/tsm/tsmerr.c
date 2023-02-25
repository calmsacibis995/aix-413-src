static char sccsid[] = "@(#)05	1.12.2.2  src/bos/usr/sbin/tsm/tsmerr.c, cmdsauth, bos411, 9428A410j 2/10/94 15:51:28";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS: tsm_err, pmsg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include "tsm.h"

#define SLEEPTIME	15

/*
 * NAME: tsm_err
 *
 * FUNCTION: Handle non-transient error condition messages
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	tsm_err handles error messages caused by non-transient error
 *	conditions.  This routine sleeps for a period of time to prevent
 *	init from needlessly re-forking the failing process.
 *
 * RETURNS:
 *	Nothing.  Exits if exit_code != TSM_NOEXIT.
 */

void
tsm_err (char *reason , int exit_code, int naptime)
{
	char msg[BUFSIZ];

	/*
	 * Write the error message to standard out.
	 */

	sprintf (msg, "%s: %s", portname, reason);
	pmsg (msg);

	/*
	 * Syslog the error as well.
	 */

	syslog (LOG_ERR, msg);

	/*
	 * sleep for some period to avoid having
	 * init respawn tsm with the same error condition.
	 */

	if (naptime == TRUE) {
		sleep (SLEEPTIME);
	}
	else if (naptime != FALSE) 	{
		sleep (naptime);
	}

	/*
	 * If we want to exit then do so
	 */

	if (exit_code != TSM_NOEXIT )  {
		exit(exit_code);
	}
}

/*
 * NAME: pmsg
 *
 * FUNCTION: Output an error message onto the user's terminal
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	outputs the supplied message onto the user's terminal.  These
 *	aren't fatal errors necessarily, so the routines doesn't exit.
 *
 * RETURNS:
 *	Nothing.
 */

void
pmsg (args)
char	*args;
{
	char	buf[BUFSIZ];

	/*
	 * Just use standard output.  If tsmgetty failed opening
	 * the port there is no reason to assume we will be able to.
	 */

	sprintf (buf, "%s\r\n", args);
	write (1, buf, strlen (buf));
}

#ifdef	DEBUGX

#	include <fcntl.h>
#	include	<sys/termio.h>

int
printargs (av)
char	**av;
{
	int	i;

	if (av[0] == NULL)
	{
		DPRINTF (("no args"));
		return;
	}
	DPRINTF (("calling %s with args:", av[0]));
	for (i = 1; av[i]; i++)
		DPRINTF (("    %s", av[i]));
}

dprintf (args)
char	*args;
{
	int	fd;
	char	**argv;
	char	buf[BUFSIZ];
	static	char	prefix[] = ": ";
	static	char	suffix[] = "\n";

	fd = open ("/TERM-DEBUG", O_WRONLY | O_APPEND | O_CREAT);
	if (fd <= -1)
		return;
	write (fd, Progname, strlen (Progname));
	write (fd, prefix, sizeof (prefix));
	argv = &args;
	vsprintf (buf, argv[0], &argv[1]);
	write (fd, buf, strlen (buf));
	write (fd, suffix, sizeof (suffix) - 1);
	close (fd);
}

Debugmodes(s, tty)
char *s;
struct termios *tty;
{
	DPRINTF(("%s cflag=%o iflag=%o lflag=%o oflag=%o", s, 
	    tty->c_cflag, 
	    tty->c_iflag, 
	    tty->c_lflag, 
	    tty->c_oflag));
}

#endif
