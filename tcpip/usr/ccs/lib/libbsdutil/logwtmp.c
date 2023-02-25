static char sccsid[] = "@(#)76	1.1  src/tcpip/usr/ccs/lib/libbsdutil/logwtmp.c, tcpip, tcpip411, 9433B411a 3/6/93 13:00:45";
/*
 * COMPONENT_NAME: tcpip
 *
 * FUNCTIONS: logwtmp
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
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <utmp.h>

logwtmp(line, name, host)
	char *line, *name, *host;
{
	struct utmp ut = {0};
	struct stat buf;
	int fd;
	time_t time();
	char *strncpy();

	if ((fd = open(WTMP_FILE, O_WRONLY|O_APPEND, 0)) < 0)
		return;
	if (!fstat(fd, &buf)) {
		(void)strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		(void)strncpy(ut.ut_user, name, sizeof(ut.ut_user));
		(void)strncpy(ut.ut_host, host, sizeof(ut.ut_host));
		(void)time(&ut.ut_time);
		(void)strncpy(ut.ut_id, line + strlen(line)-2, 2);
		ut.ut_type = DEAD_PROCESS;
		ut.ut_pid = getpid();
		if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
		    sizeof(struct utmp))
			(void)ftruncate(fd, buf.st_size);
	}
	(void)close(fd);
}
