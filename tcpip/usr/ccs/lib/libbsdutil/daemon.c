static char sccsid[] = "@(#)72	1.1  src/tcpip/usr/ccs/lib/libbsdutil/daemon.c, tcpip, tcpip411, 9433B411a 3/6/93 13:00:09";
/*
 * COMPONENT_NAME: tcpip
 *
 * FUNCTIONS: daemon
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
#include <paths.h>

daemon(nochdir, noclose)
	int nochdir, noclose;
{
	int cpid;

	if ((cpid = fork()) == -1)
		return (-1);
	if (cpid)
		exit(0);
	(void) setsid();
	if (!nochdir)
		(void) chdir("/");
	if (!noclose) {
		int devnull = open(_PATH_DEVNULL, O_RDWR, 0);

		if (devnull != -1) {
			(void) dup2(devnull, 0);
			(void) dup2(devnull, 1);
			(void) dup2(devnull, 2);
			if (devnull > 2)
				(void) close(devnull);
		}
	}
	return (0);
}
