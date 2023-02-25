static char sccsid[] = "@(#)74	1.1  src/tcpip/usr/ccs/lib/libbsdutil/login_tty.c, tcpip, tcpip411, 9433B411a 3/6/93 13:00:28";
/*
 * COMPONENT_NAME: tcpip
 *
 * FUNCTIONS: login_tty
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

#include <sys/param.h>
#include <sys/ioctl.h>

login_tty(fd)
	int fd;
{
	(void) setsid();
#ifdef TIOCSCTTY
	if (ioctl(fd, TIOCSCTTY, (char *)NULL) == -1)
		return (-1);
#endif
	(void) dup2(fd, 0);
	(void) dup2(fd, 1);
	(void) dup2(fd, 2);
	if (fd > 2)
		(void) close(fd);
	return (0);
}
