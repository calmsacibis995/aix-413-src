static char sccsid[] = "@(#)18      1.2.1.2  src/bos/usr/sbin/hostid/hostid.c, cmdnet, bos41J, 9511A_all 3/13/95 10:52:20";
/* 
 * COMPONENT_NAME: CMDNET hostid.c
 * 
 * FUNCTIONS: MSGSTR, Mhostid 
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
 * Copyright (c) 1983, 1988 Regents of the University of California.
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
" Copyright (c) 1983, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "hostid.c	5.6 (Berkeley) 6/18/88";
#endif  not lint */

#include <sys/types.h>
#include <stdio.h>
#include <netdb.h>

#include <nl_types.h>
#include "hostid_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_HOSTID,n,s) 

#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
	register char *id, *ptr;
	struct hostent *hp;
	u_long addr, inet_addr();
	long hostid, gethostid();
	char *index();
	int count;

	setlocale(LC_ALL,"");
	catd = catopen(MF_HOSTID,NL_CAT_LOCALE);

	if (argc < 2) {
		printf("%#x\n", gethostid());
		exit(0);
	}

	id = argv[1];
	if (hp = gethostbyname(id)) {
		bcopy(hp->h_addr, &addr, sizeof(addr));
		hostid = addr;
	} else if (index(id, '.')) {
		if ((hostid = inet_addr(id)) == -1)
			goto usage;
	} else {
		if (id[0] == '0' && (id[1] == 'x' || id[1] == 'X'))
			id += 2;
		/* Let's see if this is a valid HEX number for hostid
		 * or not before we sscanf and set it as hostid.
		 */
		ptr = id;
		for (count = 0; *ptr && (isdigit(*ptr) || isxdigit(*ptr)); count++, ptr++);
		if (count < 7 || count > 8)
			goto usage;
		if (sscanf(id, "%x", &hostid) != 1) {
usage:			fprintf(stderr, MSGSTR(USAGE, "usage: %s [hexnum or internet address]\n"), /*MSG*/ argv[0]);
			exit(1);
		}
	}

	if (sethostid(hostid) < 0) {
		perror(MSGSTR(SETHOST,"sethostid"));
		exit(1);
	} else {
		exit(0);
	}
}
