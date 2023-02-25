static char sccsid[] = "@(#)10	1.3  src/tcpip/usr/sbin/timed/logwtmp.c, tcptimer, tcpip411, GOLD410 3/12/91 21:18:37";
/* 
 * COMPONENT_NAME: TCPIP logwtmp.c
 * 
 * FUNCTIONS: logwtmp 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <utmp.h>

logwtmp(line, user, host)
	char *line, *user, *host;
{
	struct utmp utmp;

	bzero(&utmp, sizeof(utmp));
	if (!strncmp(line, "/dev/", 5))
	    line += 5;
	strncpy(utmp.ut_user, user, sizeof(utmp.ut_user));
	strncpy(utmp.ut_id, line, sizeof(utmp.ut_id));
	strncpy(utmp.ut_line, line, sizeof(utmp.ut_line));
	utmp.ut_type = USER_PROCESS;
	utmp.ut_pid = getpid();
	utmp.ut_time = time((time_t *)0);
	if (host)
		strncpy(utmp.ut_host, host, sizeof(utmp.ut_host));
	append_wtmp(WTMP_FILE, &utmp);
}
