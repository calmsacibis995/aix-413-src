static char sccsid[] = "@(#)73	1.1  src/tcpip/usr/ccs/lib/libbsdutil/login.c, tcpip, tcpip411, 9433B411a 3/6/93 13:00:19";
/*
 * COMPONENT_NAME: tcpip
 *
 * FUNCTIONS: login
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
#include <utmp.h>
#include <stdio.h>

extern struct utmp *getutent(void);
extern struct utmp *pututline(struct utmp *);

void
login(ut)
	struct utmp *ut;
{
	register int fd;
	int tty;
	off_t lseek();
	struct utmp *xut;

	/*
	 * The bsd programs which call this routine will fill in the
	 * bsd elements of the utmp structure: name, line, time and host
	 */
	setutent();
	ut->ut_type = USER_PROCESS;
	ut->ut_pid = getpid();
	ut->ut_exit.e_exit = 0;
	ut->ut_exit.e_termination = 0;

	/*
	 * look for a slot in the utmp file with our pid 
	 * If we find one, when use that id (which is probably from inittab
	 */
	while ((xut = getutent()) != NULL) 
		if (xut->ut_pid == ut->ut_pid) {
		    strncpy(ut->ut_id, xut->ut_id, sizeof(ut->ut_id));
		    break;
		}
	/*
	 * If we couldn't find our pid, then use the last two characters
	 * of the tty name (probably p0, p1, etc)
	 */
	if (xut == NULL)
		strncpy(ut->ut_id , ut->ut_line + strlen(ut->ut_line)-2, 2);

	(void)pututline(ut);
	endutent();

	if ((fd = open(WTMP_FILE, O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
}
