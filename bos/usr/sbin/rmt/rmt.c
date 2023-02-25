static char sccsid[] = "@(#)08	1.14  src/bos/usr/sbin/rmt/rmt.c, cmdarch, bos411, 9428A410j 11/9/93 10:45:31";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: rmt
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*#ifndef lint
char copyright[] =
"(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint
*/
/*
#ifndef lint
static char sccsid[] = "(#)rmt.c	5.2 (Berkeley) 1/7/86";
#endif not lint
*/

/*
 * rmt
 */
#include <stdio.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/tape.h>
#include <errno.h>
#include <locale.h>
#include "rmt_msg.h"

int	tape = -1;

char	*record;
int	maxrecsize = -1;
char	*checkbuf();

#ifdef SSIZE
#undef SSIZE
#endif
#define	SSIZE	64
char	device[SSIZE];
char	count[SSIZE], mode[SSIZE], pos[SSIZE], op[SSIZE];

extern	errno;
char	resp[BUFSIZ];

long	lseek();

nl_catd catd;

FILE	*debug;

#define MSGSTR(N,S) catgets(catd,MS_RMT,N,S)
#define	DEBUG0(f)	if (debug) fprintf(debug, f)
#define	DEBUG1(f,a)	if (debug) fprintf(debug, f, a)
#define	DEBUG2(f,a1,a2)	if (debug) fprintf(debug, f, a1, a2)

main(argc, argv)
	int argc;
	char **argv;
{
	int rval;
	char c;
	int n, i, cc;

	(void) setlocale(LC_ALL,"");
	catd = catopen("rmt.cat", NL_CAT_LOCALE);
	argc--, argv++;
	if (argc > 0) {
		debug = fopen(*argv, "w");
		if (debug == 0)
			exit(1);
		(void) setbuf(debug, (char *)0);
	}
top:
	errno = 0;
	rval = 0;
	if (read(0, &c, 1) != 1)
		exit(0);
	switch (c) {

	case 'O':
		if (tape >= 0)
			(void) close(tape);
		getstring(device); getstring(mode);
		DEBUG2("rmtd: O %s %s\n", device, mode);
		tape = open(device, atoi(mode));
		if (tape < 0)
			goto ioerror;
		goto respond;

	case 'C':
		DEBUG0("rmtd: C\n");
		getstring(device);		/* discard */
		if (close(tape) < 0)
			goto ioerror;
		tape = -1;
		goto respond;

/*
	case 'L':
		getstring(count); getstring(pos);
		DEBUG2("rmtd: L %s %s\n", count, pos);
		rval = lseek(tape, (long) atoi(count), atoi(pos));
		if (rval < 0)
			goto ioerror;
		goto respond;
*/

	case 'W':
		getstring(count);
		n = atoi(count);
		DEBUG1("rmtd: W %s\n", count);
		record = checkbuf(record, n);
		for (i = 0; i < n; i += cc) {
			cc = read(0, &record[i], n - i);
			if (cc <= 0) {
				DEBUG0("rmtd: premature eof\n");
				exit(2);
			}
		}
		rval = write(tape, record, n);
		if (rval < 0)
			goto ioerror;
		goto respond;

	case 'R':
		getstring(count);
		DEBUG1("rmtd: R %s\n", count);
		n = atoi(count);
		record = checkbuf(record, n);
		rval = read(tape, record, n);
		if (rval < 0)
			goto ioerror;
		(void) sprintf(resp, "A%d\n", rval);
		(void) write(1, resp, strlen(resp));
		(void) write(1, record, rval);
		goto top;

	case 'I':
		getstring(op); getstring(count);
		DEBUG2("rmtd: I %s %s\n", op, count);
		{ struct stop mtop;
		  mtop.st_op = atoi(op);
		  mtop.st_count = atoi(count);
		  if (ioctl(tape, STIOCTOP, (char *)&mtop) < 0)
			goto ioerror;
		  rval = mtop.st_count;
		}
		goto respond;

/*
	case 'S':
		DEBUG0("rmtd: S\n");
		{ struct mtget mtget;
		  if (ioctl(tape, MTIOCGET, (char *)&mtget) < 0)
			goto ioerror;
		  rval = sizeof (mtget);
		  (void) sprintf(resp, "A%d\n", rval);
		  (void) write(1, resp, strlen(resp));
		  (void) write(1, (char *)&mtget, sizeof (mtget));
		  goto top;
		}
*/

	default:
		DEBUG1("rmtd: garbage command %c\n", c);
		exit(3);
	}
respond:
	DEBUG1("rmtd: A %d\n", rval);
	(void) sprintf(resp, "A%d\n", rval);
	(void) write(1, resp, strlen(resp));
	goto top;
ioerror:
	error(errno);
	goto top;
	catclose(catd);
}

getstring(bp)
	char *bp;
{
	int i;
	char *cp = bp;

	for (i = 0; i < SSIZE; i++) {
		if (read(0, cp+i, 1) != 1)
			exit(0);
		if (cp[i] == '\n')
			break;
	}
	cp[i] = '\0';
}

char *
checkbuf(record, size)
	char *record;
	int size;
{
	extern char *malloc();

	if (size <= maxrecsize)
		return (record);
	if (record != 0)
		free(record);
	record = malloc(size);
	if (record == 0) {
		DEBUG0("rmtd: cannot allocate buffer space\n");
		exit(4);
	}
	maxrecsize = size;
	while (size > 1024 &&
	       setsockopt(0, SOL_SOCKET, SO_RCVBUF, &size, sizeof (size)) < 0)
		size -= 1024;
	return (record);
}

error(num)
	int num;
{

	DEBUG2("rmtd: E %d (%s)\n", num, strerror(num));
	(void) sprintf(resp, "E%d\n%s\n", num, strerror(num));
	(void) write(1, resp, strlen (resp));
}
