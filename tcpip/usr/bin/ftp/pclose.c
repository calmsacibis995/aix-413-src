static char sccsid[] = "@(#)79	1.3  src/tcpip/usr/bin/ftp/pclose.c, tcpfilexfr, tcpip411, GOLD410 10/8/89 16:15:57";
/* 
 * COMPONENT_NAME: TCPIP pclose.c
 * 
 * FUNCTIONS: mypclose, mypopen, pabort, sigmask, tst 
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#ifndef aiws
#include <sys/wait.h>
#else
#include <fcntl.h>		/* for F_DUPFD */
#ifndef sigmask
#define sigmask(s)	(1 << ((s)-1))
#endif
#endif

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern	char *malloc();

static	int *popen_pid;
static	int nfiles;

FILE *
mypopen(cmd,mode)
	char *cmd;
	char *mode;
{
	int p[2];
	int myside, hisside, pid;

	if (nfiles <= 0)
		nfiles = getdtablesize();
	if (popen_pid == NULL) {
		popen_pid = (int *)malloc((unsigned) nfiles * sizeof *popen_pid);
		if (popen_pid == NULL)
			return (NULL);
		for (pid = 0; pid < nfiles; pid++)
			popen_pid[pid] = -1;
	}
	if (pipe(p) < 0)
		return (NULL);
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if ((pid = fork()) == 0) {
		/* myside and hisside reverse roles in child */
		(void) close(myside);
		if (hisside != tst(0, 1)) {
#ifndef aiws
			(void) dup2(hisside, tst(0, 1));
#else
			(void)close( tst(0,1) );
			(void)fcntl( hisside, F_DUPFD, tst(0,1) );
#endif
			(void) close(hisside);
		}
		execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
		_exit(127);
	}
	if (pid == -1) {
		(void) close(myside);
		(void) close(hisside);
		return (NULL);
	}
	popen_pid[myside] = pid;
	(void) close(hisside);
	return (fdopen(myside, mode));
}

pabort()
{
	extern int mflag;

	mflag = 0;
}

mypclose(ptr)
	FILE *ptr;
{
	int child, pid, omask, pabort(), (*istat)();
#ifndef aiws
	union wait status;
#else
	int	status;
#endif

	child = popen_pid[fileno(ptr)];
	popen_pid[fileno(ptr)] = -1;
	(void) fclose(ptr);
	if (child == -1)
		return (-1);
	istat = signal(SIGINT, pabort);
	omask = sigblock(sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = wait(&status)) != child && pid != -1)
		;
	(void) sigsetmask(omask);
	(void) signal(SIGINT, istat);
	return (pid == -1 ? -1 : 0);
}
