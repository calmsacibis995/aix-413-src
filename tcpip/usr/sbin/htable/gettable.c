static char sccsid[] = "@(#)42	1.7  src/tcpip/usr/sbin/htable/gettable.c, tcpadmin, tcpip411, GOLD410 3/8/94 17:35:54";
/* 
 * COMPONENT_NAME: TCPIP gettable.c
 * 
 * FUNCTIONS: MSGSTR, Mgettable, equaln 
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
 * Copyright (c) 1983 The Regents of the University of California.
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
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "gettable.c	5.4 (Berkeley) 10/11/88";
#endif  not lint */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <netdb.h>

#include <locale.h>

#include <nl_types.h>
#include "gettable_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GETTABLE,n,s) 

#define	OUTFILE		"hosts.txt"	/* default output file */
#define	VERFILE		"hosts.ver"	/* default version file */
#define	QUERY		"ALL\r\n"	/* query to hostname server */
#define	VERSION		"VERSION\r\n"	/* get version number */

#define	equaln(s1, s2, n)	(!strncmp(s1, s2, n))

struct	sockaddr_in sin;
char	buf[BUFSIZ];
char	*outfile = OUTFILE;

main(argc, argv)
	int argc;
	char *argv[];
{
	int s;
	register len;
	register FILE *sfi, *sfo, *hf;
	char *host;
	char *vers, *query;
	register struct hostent *hp;
	struct servent *sp;
	int version = 0;
	int beginseen = 0;
	int endseen = 0;

	setlocale(LC_ALL,"");
	catd = catopen (MF_GETTABLE, NL_CAT_LOCALE);

	vers = MSGSTR(VERS, "VERSION\r\n");
	query = MSGSTR(MS_QUERY, "ALL\r\n");
	argv++, argc--;
	if (**argv == '-') {
		if (argv[0][1] != 'v')
			fprintf(stderr, MSGSTR(UNKOPT, "unknown option %s ignored\n"), *argv);
		else
			version++, outfile = VERFILE;
		argv++, argc--;
	}
	if (argc < 1 || argc > 2) {
		fprintf(stderr, MSGSTR(USAGE, "usage: gettable [-v] host [ file ]\n"));
		exit(1);
	}
	sp = getservbyname("hostnames", "tcp");
	if (sp == NULL) {
		fprintf(stderr, MSGSTR(UNKSERV, "gettable: hostnames/tcp: unknown service\n"));
		exit(3);
	}
	host = *argv;
	argv++, argc--;
	hp = gethostbyname(host);
	if (hp == NULL) {
		fprintf(stderr, MSGSTR(UNKHOST, "gettable: %s: "), host);
		herror((char *)NULL);
		exit(2);
	}
	host = hp->h_name;
	if (argc > 0)
		outfile = *argv;
	sin.sin_family = hp->h_addrtype;
	s = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if (s < 0) {
		perror(MSGSTR(SOCKET, "gettable: socket"));
		exit(4);
	}
	if (bind(s, &sin, sizeof (sin)) < 0) {
		perror(MSGSTR(BIND,"gettable: bind"));
		exit(5);
	}
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = sp->s_port;
	if (connect(s, &sin, sizeof (sin)) < 0) {
		perror(MSGSTR(CONNECT,"gettable: connect"));
		exit(6);
	}
	fprintf(stderr, MSGSTR(CONNOK,"Connection to %s opened.\n"), host);
	sfi = fdopen(s, "r");
	sfo = fdopen(s, "w");
	if (sfi == NULL || sfo == NULL) {
		perror(MSGSTR(FDOPEN,"gettable: fdopen"));
		close(s);
		exit(1);
	}
	hf = fopen(outfile, "w");
	if (hf == NULL) {
		fprintf(stderr, MSGSTR(GETTABLE,"gettable: ")); perror(outfile);
		close(s);
		exit(1);
	}
	fprintf(sfo, version ? vers : query);
	fflush(sfo);
	while (fgets(buf, sizeof(buf), sfi) != NULL) {
		len = strlen(buf);
		buf[len-2] = '\0';
		if (!version && equaln(buf, "BEGIN", 5)) {
			if (beginseen || endseen) {
				fprintf(stderr,MSGSTR(BEGINERR, 
				    "gettable: BEGIN sequence error\n"));
				exit(90);
			}
			beginseen++;
			continue;
		}
		if (!version && equaln(buf, "END", 3)) {
			if (!beginseen || endseen) {
				fprintf(stderr, MSGSTR(ENDERR,
				    "gettable: END sequence error\n"));
				exit(91);
			}
			endseen++;
			continue;
		}
		if (equaln(buf, "ERR", 3)) {
			fprintf(stderr,MSGSTR(SERVERR,
			    "gettable: hostname service error: %s"), buf);
			exit(92);
		}
		fprintf(hf, "%s\n", buf);
	}
	fclose(hf);
	if (!version) {
		if (!beginseen) {
			fprintf(stderr, MSGSTR(NOBEGIN,"gettable: no BEGIN seen\n"));
			exit(93);
		}
		if (!endseen) {
			fprintf(stderr, MSGSTR(NOEND,"gettable: no END seen\n"));
			exit(94);
		}
		fprintf(stderr, MSGSTR(GETHOST, "Host table received.\n"));
	} else
		fprintf(stderr, MSGSTR(VERSRECV, "Version number received.\n"));
	close(s);
	fprintf(stderr, MSGSTR(CONNCLOSE, "Connection to %s closed\n"), host);
	exit(0);
}
