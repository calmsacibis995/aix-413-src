static char sccsid[] = "@(#)43	1.6  src/tcpip/usr/lbin/ftpd/logwtmp.c, tcpfilexfr, tcpip411, GOLD410 6/2/93 09:29:12";
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
/*
#ifndef lint
static char sccsid[] = "logwtmp.c	5.2 (Berkeley) 9/22/88";
#endif  not lint */

/* The ftpd_append_wtmp routine was stolen from libc.a.  We needed
 * to keep the file open for doing chroot on anonymous ftp.
 * The libc.a version opened and closed the file each time the
 * file is called, and we need to keep it open.
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <utmp.h>

static struct flock write_lock =  {
    F_WRLCK,
    SEEK_SET,
    0,
    sizeof(struct utmp),
};

/* Argument for unlocks */
static struct flock unlock = {
  F_UNLCK,
  SEEK_SET,
  0,
  sizeof(struct utmp),
};

#define GET_WRITELOCK(fd)	(lockfx(fd, F_SETLKW, &write_lock))
#define REL_WRITELOCK(fd) (lockfx(fd, F_SETLKW, &unlock))

#define LOGGING_IN	0
#define LOGGING_OUT	1

static int	fd = -1;

logwtmp(line, user, host, status)
char 	*line, *user, *host;
int	status;

{
	struct utmp utmp;	/* utmp structure being created */

	bzero(&utmp, sizeof(utmp));
	if (!strncmp(line, "/dev/", 5))
	    line += 5;
	strncpy(utmp.ut_user, user, sizeof(utmp.ut_user));
	strncpy(utmp.ut_id, line, sizeof(utmp.ut_id));
	strncpy(utmp.ut_line, line, sizeof(utmp.ut_line));
	if (status == LOGGING_IN) {
	    utmp.ut_type = USER_PROCESS;
	} else {
	    utmp.ut_type = DEAD_PROCESS;
	}
	utmp.ut_pid = getpid();
	utmp.ut_time = time((time_t *)0);
	if (host)
		strncpy(utmp.ut_host, host, sizeof(utmp.ut_host));
	ftpd_append_wtmp(WTMP_FILE, &utmp);
}


ftpd_append_wtmp(char *s, struct utmp *u)
{
    struct utmp utmp;	/* utmp structure being created */
    long pos;
    int err = 0;
    char	foo[255];

    if (fd < 0 && (fd = open(s, O_WRONLY|O_APPEND, 0)) < 0)
	return;
    
    if (GET_WRITELOCK(fd) < 0 ||		/* get write lock */
	(pos = lseek(fd, 0L, SEEK_END)) < 0 ||	/* seed to end */
	((pos %= sizeof(*u)) &&			/* seek to rec boundry */
	 lseek(fd, -pos, SEEK_END) < 0) ||	/* backup to rec boundry */
	write(fd, u, sizeof(*u)) != sizeof(*u))	/* write out record */
	err = -1;

    REL_WRITELOCK(fd);
    return err;
}

