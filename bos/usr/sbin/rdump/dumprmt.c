static char sccsid[] = "@(#)91	1.17  src/bos/usr/sbin/rdump/dumprmt.c, cmdarch, bos411, 9428A410j 2/18/93 17:58:25";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
static char sccsid[] = "(#)dumprmt.c	5.4 (Berkeley) 12/11/85";
#endif not lint
*/

#include <nl_types.h>
#include "dumpi_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_DUMP,N,S)
nl_catd catd;

#include <sys/param.h>
#include <sys/types.h>
#include <sys/tape.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <jfs/inode.h>
#include <jfs/ino.h>
#define  _KERNEL
#include <sys/dir.h>
#undef   _KERNEL
#include <sys/signal.h>

#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <netdb.h>
#include <dumprestor.h>


#define	TS_CLOSED	0
#define	TS_OPEN		1

static	int rmtstate = TS_CLOSED;
static	size;
int	rmtape;
int	rmtconnaborted(void);
char	*rmtpeer;

extern int ntrec;		/* blocking factor on tape */
extern char *user;
#ifndef RRESTORE
extern void (*dump_signal()) ();
#endif

rmthost(host)
	char *host;
{

	rmtpeer = host;
#ifndef RRESTORE
	dump_signal(SIGPIPE, (void (*)(int))rmtconnaborted);
#else
	signal(SIGPIPE, (void (*)(int))rmtconnaborted);
#endif
	rmtgetconn();
	if (rmtape < 0)
		return (0);
	return (1);
}

rmtconnaborted(void)
{

	fprintf(stderr, MSGSTR(LOSTCON, "rdump: Lost connection to remote host.\n"));
	exit(1);
}

rmtgetconn()
{
	static struct servent *sp = 0;
	struct passwd *pw;
	char *name = "root";

	if (sp == 0) {
		sp = getservbyname("shell", "tcp");
		if (sp == 0) {
			fprintf(stderr, MSGSTR(UNKSERV, "rdump: shell/tcp: unknown service\n"));
			exit(1);
		}
	}
	pw = getpwuid(getuid());
	if (pw && pw->pw_name)
		name = pw->pw_name;
	if (user == 0 || *user == 0) user = name;
	rmtape = rcmd(&rmtpeer, sp->s_port, name, user, "/usr/sbin/rmt", 0);
	/* 
	 * restrict socket size to maximum of 32k.  Any thing
	 * greater causes unpredictable problems.
	 */
	size = HIGHDENSITYTREC * TP_BSIZE;
	while (size > TP_BSIZE &&
	    setsockopt(rmtape, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size)) < 0)
		size -= TP_BSIZE;
}

rmtopen(tape, mode)
	char *tape;
	int mode;
{
	char buf[256];

	sprintf(buf, "O%s\n%d\n", tape, mode);
	rmtstate = TS_OPEN;
	return (rmtcall(tape, buf));
}

rmtclose()
{
	int rc;

	if (rmtstate != TS_OPEN)
		return (0);
	rc = rmtcall("close", "C\n");
	rmtstate = TS_CLOSED;
	return (rc);
}

rmtread(buf, count)
	char *buf;
	int count;
{
	char line[30];
	int n, i, cc;
	extern errno;

	sprintf(line, "R%d\n", count);
	n = rmtcall("read", line);
	if (n < 0) {
		errno = n;
		return (-1);
	}
	for (i = 0; i < n; i += cc) {
		cc = read(rmtape, buf+i, (unsigned)(n-i));
		if (cc <= 0) {
			rmtconnaborted();
		}
	}
	return (n);
}

/*
 * Break up the write into little pieces, no greater than
 * 32k.
 */
rmtwrite(buf, count)
	char *buf;
	int count;
{
	char line[30];
	int buf_off;	
	int rc;

	rc = 0;
	if (count <= size) {
		sprintf(line, "W%d\n", count);
		write(rmtape, line, (unsigned)strlen(line));
		write(rmtape, buf, (unsigned)count);
		rc += rmtreply("write");
	}
	else {
		buf_off = 0;
		while (count > size) {
			sprintf(line, "W%d\n", size);
			write(rmtape, line, (unsigned)strlen(line));
			write(rmtape, buf+buf_off, (unsigned)size);
			rc += rmtreply("write");
			buf_off += size;
			count -= size;
		}
		sprintf(line, "W%d\n", count);
		write(rmtape, line, (unsigned)strlen(line));
		write(rmtape, buf+buf_off, (unsigned)count);
		rc += rmtreply("write");
	}
	return (rc);
}

rmtwrite0(count)
	int count;
{
	char line[30];

	sprintf(line, "W%d\n", count);
	write(rmtape, line, (unsigned)strlen(line));
}

rmtwrite1(buf, count)
	char *buf;
	int count;
{

	write(rmtape, buf, (unsigned)count);
}

rmtwrite2()
{
	int i;

	return (rmtreply("write"));
}

rmtseek(offset, pos)
	int offset, pos;
{
	char line[80];

	sprintf(line, "L%d\n%d\n", offset, pos);
	return (rmtcall("seek", line));
}

/*
struct	mtget mts;

struct mtget *
rmtstatus()
{
	register int i;
	register char *cp;

	if (rmtstate != TS_OPEN)
		return (0);
	rmtcall("status", "S\n");
	for (i = 0, cp = (char *)&mts; i < sizeof(mts); i++)
		*cp++ = rmtgetb();
	return (&mts);
}
*/

rmtioctl(cmd, count)
	int cmd, count;
{
	char buf[256];

	if (count < 0)
		return (-1);
	sprintf(buf, "I%d\n%d\n", cmd, count);
	return (rmtcall("ioctl", buf));
}

rmtcall(cmd, buf)
	char *cmd, *buf;
{

	if (write(rmtape, buf, (unsigned)strlen(buf)) != strlen(buf))
		rmtconnaborted();
	return (rmtreply(cmd));
}

rmtreply(cmd)
	char *cmd;
{
	register int c;
	char code[30], emsg[BUFSIZ];

	rmtgets(code, sizeof (code));
	if (*code == 'E' || *code == 'F') {
		rmtgets(emsg, sizeof (emsg));
		msg("%s: %s\n", cmd, emsg, code + 1);
		if (*code == 'F') {
			rmtstate = TS_CLOSED;
			return (-1);
		}
		return (-1);
	}
	if (*code != 'A') {
		msg(MSGSTR(PROTO, "Protocol to remote tape server botched (code %s?).\n"),
		    code);
		rmtconnaborted();
	}
	return (atoi(code + 1));
}

rmtgetb()
{
	char c;

	if (read(rmtape, &c, (unsigned)1) != 1)
		rmtconnaborted();
	return (c);
}

rmtgets(cp, len)
	char *cp;
	int len;
{

	while (len > 1) {
		*cp = rmtgetb();
		if (*cp == '\n') {
			cp[1] = 0;
			return;
		}
		cp++;
		len--;
	}
	msg(MSGSTR(PROTO1, "Protocol to remote tape server botched (in rmtgets).\n"));
	rmtconnaborted();
}
