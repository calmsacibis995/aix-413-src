static char sccsid[] = "@(#)77	1.1  src/tcpip/usr/ccs/lib/libbsdutil/pty.c, tcpip, tcpip411, 9433B411a 3/6/93 13:00:53";
/*
 * COMPONENT_NAME: tcpip
 *
 * FUNCTIONS: openpty, forkpty
 *
 * ORIGINS: 27, 26, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 */

#include <sys/file.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <grp.h>
#include <errno.h>

openpty(amaster, aslave, name, termp, winp)
	int *amaster, *aslave;
	char *name;
	struct termios *termp;
	struct winsize *winp;
{
	char *line;
	register master, slave, ruid, ttygid;
	struct group *gr;
	extern errno;

	strcpy (line, "/dev/ptyXX");
	if ((gr = getgrnam("tty")) != NULL)
		ttygid = gr->gr_gid;
	else
		ttygid = -1;
	ruid = getuid();

        if ((master = open("/dev/ptc", O_RDWR, 0)) >= 0) {
                line = ttyname(master);
		(void) chown(line, ruid, ttygid);
		(void) chmod(line, 0620);
		(void) revoke(line);
		if ((slave = open(line, O_RDWR, 0)) >= 0) {
			*amaster = master;
			*aslave = slave;
			if (name)
				strcpy(name, line);
			if (termp)
				(void) tcsetattr(slave, TCSAFLUSH, termp);
			if (winp)
				(void) ioctl(slave, TIOCSWINSZ, (char *)winp);
#ifdef DEBUG
			printf ("Openpty, found pty %s\n", line);
#endif
			return (0);
		}
		(void) close(master);
		return (-1);	/* out of slave ptys */
        }
#ifdef DEBUG
	printf ("Openpty, line %s, errno = %d, %s\n",
						line, errno, strerror (errno));
#endif
	return (-1);	/* out of ptys */
}

forkpty(amaster, name, termp, winp)
	int *amaster;
	char *name;
	struct termios *termp;
	struct winsize *winp;
{
	int master, slave, pid;

	if (openpty(&master, &slave, name, termp, winp) == -1)
		return (-1);
	switch (pid = fork()) {
	case -1:
		return (-1);
	case 0:
		/* 
		 * child
		 */
		(void) close(master);
		(void) login_tty(slave);
		return (0);
	}
	/*
	 * parent
	 */
	*amaster = master;
	(void) close(slave);
	return (pid);
}
