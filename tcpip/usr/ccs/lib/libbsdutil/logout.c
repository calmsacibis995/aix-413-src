static char sccsid[] = "@(#)75	1.1  src/tcpip/usr/ccs/lib/libbsdutil/logout.c, tcpip, tcpip411, 9433B411a 3/6/93 13:00:37";
/*
 * COMPONENT_NAME: tcpip
 *
 * FUNCTIONS: logout
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
#include <utmp.h>
#include <stdio.h>

extern struct utmp *getutline(struct utmp *);
extern struct utmp *pututline(struct utmp *);

/* 0 on failure, 1 on success */

logout(line)
	register char *line;
{
	register FILE *fp;
	struct utmp ut;
	struct utmp *utptr;
	int rval;
	time_t time();

	rval = 0;
	setutent();
	strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	if ((utptr = getutline(&ut)) != NULL) {
		utptr->ut_type = DEAD_PROCESS;
		bzero(utptr->ut_user, sizeof(utptr->ut_user));
                bzero(utptr->ut_host, sizeof(utptr->ut_host));
		(void) time(&utptr->ut_time);
		if (pututline(utptr) != NULL) rval = 1;
	}
	endutent();
	return(rval);
}
