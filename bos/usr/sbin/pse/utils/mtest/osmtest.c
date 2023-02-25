static char sccsid[] = "@(#)56	1.3  src/bos/usr/sbin/pse/utils/mtest/osmtest.c, cmdpse, bos411, 9428A410j 6/19/91 13:48:42";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** osmtest.c 1.1
 **/

/* AIX dependent routines for MTEST */

#include <stdio.h>
#include <pse/common.h>
#include "mtest.h"
#include <sys/wait.h>

static	boolean	main_thread = true;

void
etime (cnt, len, t1, msg)
	int	cnt;
	int	len;
	TIMER	* t1;
	char	* msg;
{
	unsigned long	ms1;
	float		secs;
	struct tms	tbuf;
	long		t2;

	/* Get elapsed clock ticks */
	t2 = times(&tbuf) - t1->timer_ret;
	/* Now convert to ms */
	ms1 = t2 * 10;
	if (ms1 == 0)
		ms1 = 1;
	secs = (float)ms1 / (float)1000;
	sprintf(msg, "time: %.2f, P/s: %.2f, ms/p: %.3f, Mb/s: %.3f"
	, secs, (float)cnt / secs, (float)ms1 / (float)cnt, ((float)((float)cnt * (float)len * 8.) / (1000. * 1000.)) / secs);
}

char *
os_get_cmd () {
static	char	buf[128];

	do {
		if ( fgets(buf, sizeof(buf), stdin)  == NULL )
			exit(0);
	} while ( buf[0] == '\n' );
	return buf;
}

int
CreateThread (pfi, thread_id_ptr, stptr, stlen, arg)
	pfi_t	pfi;
	u32	* thread_id_ptr;
	char	* stptr;
	u32	stlen;
	u32	arg;
{
	int	i1;
	int	pid;

	/* We don't completely eliminate zombies, but we do make sure that
	** there won't be a growing number of them.
	*/
	while ( wait3(0, WNOHANG, 0) > 0 )
		;
	signal(SIGCHLD, SIG_IGN);
	switch (pid = fork()) {
	case -1:
		warn("couldn't fork, %s\n", errmsg(0));
		exit(0);
	case 0:
		if ( main_thread ) {
			main_thread = false;
			break;
		}
		if ( thread_id_ptr )
			*thread_id_ptr = 1;
		return 0;
	default:
		if ( !main_thread )
			break;
		if ( thread_id_ptr )
			*thread_id_ptr = pid;
		return 0;
	}
	(*pfi)(arg);
	exit(0);
}
