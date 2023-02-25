static char sccsid[] = "@(#)46  1.4  src/tcpip/usr/sbin/mkhosts/mkhosts.c, tcpip, tcpip411, GOLD410 3/8/94 18:02:00";
/* 
 * COMPONENT_NAME: TCPIP rsh.c
 * 
 * FUNCTIONS: MSGSTR, Mrsh, mask, sendsig 
 *
 * ORIGINS: 26  27 
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
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)mkhosts.c	5.1 (Berkeley) 5/28/85";
#endif not lint
*/

#include <sys/file.h>
#include <stdio.h>
#include <netdb.h>
#include <ndbm.h>

#include <nl_types.h>
#include "mkhosts_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_MKHOSTS, n, s) 

#include <locale.h>
char	buf[BUFSIZ];

main(argc, argv)
	char *argv[];
{
	DBM *dp;
	register struct hostent *hp;
	datum key, content;
	register char *cp, *tp, **sp;
	register int *nap;
	int naliases;
	int verbose = 0, entries = 0, maxlen = 0, error = 0;
	char tempname[BUFSIZ], newname[BUFSIZ];

	setlocale(LC_ALL,"");
	catd = catopen(MF_MKHOSTS,NL_CAT_LOCALE);

	if (argc > 1 && strcmp(argv[1], "-v") == 0) {
		verbose++;
		argv++, argc--;
	}
	if (argc != 2) {
		fprintf(stderr, MSGSTR(USAGE, "usage: mkhosts [ -v ] file\n"));
		exit(1);
	}
	if (access(argv[1], R_OK) < 0) {
		perror(argv[1]);
		exit(1);
	}
	umask(0);

	sprintf(tempname, "%s.new", argv[1]);
	dp = dbm_open(tempname, O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (dp == NULL) {
		fprintf(stderr, MSGSTR(DBM_OPEN_FAIL, "dbm_open failed: "));
		perror(argv[1]);
		exit(1);
	}
	sethostfile(argv[1]);
	sethostent(1);
	while (hp = gethostent()) {
		cp = buf;
		tp = hp->h_name;
		while (*cp++ = *tp++)
			;
		nap = (int *)cp;
		cp += sizeof (int);
		naliases = 0;
		for (sp = hp->h_aliases; *sp; sp++) {
			tp = *sp;
			while (*cp++ = *tp++)
				;
			naliases++;
		}
		bcopy((char *)&naliases, (char *)nap, sizeof(int));
		bcopy((char *)&hp->h_addrtype, cp, sizeof (int));
		cp += sizeof (int);
		bcopy((char *)&hp->h_length, cp, sizeof (int));
		cp += sizeof (int);
		bcopy(hp->h_addr, cp, hp->h_length);
		cp += hp->h_length;
		content.dptr = buf;
		content.dsize = cp - buf;
		if (verbose)
			printf(MSGSTR(ALIASES, "store %s, %d aliases\n"),
			                                 hp->h_name, naliases);
		key.dptr = hp->h_name;
		key.dsize = strlen(hp->h_name);
		if (dbm_store(dp, key, content, DBM_INSERT) < 0) {
			perror(hp->h_name);
			goto err;
		}
		for (sp = hp->h_aliases; *sp; sp++) {
			key.dptr = *sp;
			key.dsize = strlen(*sp);
			if (dbm_store(dp, key, content, DBM_INSERT) < 0) {
				perror(*sp);
				goto err;
			}
		}
		key.dptr = hp->h_addr;
		key.dsize = hp->h_length;
		if (dbm_store(dp, key, content, DBM_INSERT) < 0) {
			perror(MSGSTR(DBM_STORE_ERR,"dbm_store host address"));
			goto err;
		}
		entries++;
		if (cp - buf > maxlen)
			maxlen = cp - buf;
	}
	endhostent();
	dbm_close(dp);

	sprintf(tempname, "%s.new.pag", argv[1]);
	sprintf(newname, "%s.pag", argv[1]);
	if (rename(tempname, newname) < 0) {
		perror("rename .pag");
		exit(1);
	}
	sprintf(tempname, "%s.new.dir", argv[1]);
	sprintf(newname, "%s.dir", argv[1]);
	if (rename(tempname, newname) < 0) {
		perror("rename .dir");
		exit(1);
	}
	printf(MSGSTR(HOST_ENTRIES,"%d host entries, maximum length %d\n"), 
                                                              entries, maxlen);
	exit(0);
err:
	sprintf(tempname, "%s.new.pag", argv[1]);
	unlink(tempname);
	sprintf(tempname, "%s.new.dir", argv[1]);
	unlink(tempname);
	exit(1);
}
