static char sccsid[] = "@(#)36        1.7  src/tcpip/usr/bin/talk/get_names.c, tcptalk, tcpip411, GOLD410 3/19/91 14:35:22";
/* 
 * COMPONENT_NAME: TCPIP get_names.c
 * 
 * FUNCTIONS: MSGSTR, any, get_names 
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
 * Copyright (c) 1983 Regents of the University of California.
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
static char sccsid[] = "get_names.c	5.5 (Berkeley) 6/29/88";
#endif  not lint */

#include "talk_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

#include "talk.h"
#include <sys/param.h>
#include <protocols/talkd.h>
#include <netinet/in.h>
#include <pwd.h>

char	*getlogin();
char	*ttyname();
char	*rindex();
extern	CTL_MSG msg;

/*
 * Determine the local and remote user, tty, and machines
 */
get_names(argc, argv)
	int argc;
	char *argv[];
{
	char hostname[MAXHOSTNAMELEN];
	char *his_name, *my_name;
	char *my_machine_name, *his_machine_name;
	char *my_tty, *his_tty;
	register char *cp;
	char * dots(char *,int );
	int count;

	if (argc < 2 ) {
		printf(MSGSTR(USAGE, "Usage: talk user [ttyname]\n")); /*MSG*/
		exit(-1);
	}
	if (!isatty(0)) {
		printf(MSGSTR(IS_TTY, "Standard input must be a tty, not a pipe or a file\n")); /*MSG*/
		exit(-1);
	}
	if ((my_name = getlogin()) == NULL) {
		struct passwd *pw;

		if ((pw = getpwuid(getuid())) == NULL) {
		printf(MSGSTR(GO_AWAY, "You don't exist. Go away.\n")); /*MSG*/
			exit(-1);
		}
		my_name = pw->pw_name;
	}
	gethostname(hostname, sizeof (hostname));
	my_machine_name = hostname;
	/* check for, and strip out, the machine name of the target */
	/* check for the number dots in the arguments */
	count = count_dots(argv[1]);
	/* if argument contains any of "@!:" */
	if (anyone(argv[1],"@!:"))
	{
	for (cp = argv[1]; *cp && !any(*cp, "@:!"); cp++)
		;
		if (*cp++ == '@') {
			/* user@host */
			his_name = argv[1];
			his_machine_name = cp;
		} else {
			/*  host!user or host:user */
			his_name = cp;
                        his_machine_name = argv[1];
		}
	}
	/* if host is IP address or name contains dots */
	else if((cp=(dots(argv[1],count))) != '\0')  {
                his_name = cp;
                his_machine_name = argv[1]; 
	}
        else { 
		/* this is a local to local talk */
		his_name = argv[1];
                his_machine_name = my_machine_name; 
	}
	if (*cp != '\0')
                *--cp = '\0';

	if (argc > 2)
		his_tty = argv[2];	/* tty name is arg 2 */
	else
		his_tty = "";
	get_addrs(my_machine_name, his_machine_name);
	/*
	 * Initialize the message template.
	 */
	msg.vers = TALK_VERSION;
	msg.addr.sa_family = htons(AF_INET);
	msg.ctl_addr.sa_family = htons(AF_INET);
	msg.id_num = htonl(0);
	strncpy(msg.l_name, my_name, NAME_SIZE);
	msg.l_name[NAME_SIZE - 1] = '\0';
	strncpy(msg.r_name, his_name, NAME_SIZE);
	msg.r_name[NAME_SIZE - 1] = '\0';
	strncpy(msg.r_tty, his_tty, TTY_SIZE);
	msg.r_tty[TTY_SIZE - 1] = '\0';
}
	
static
any(c, cp)
	register char c, *cp;
{

	while (*cp)
		if (c == *cp++)
			return (1);
	return (0);
}

char * dots (arg,cnt)
        char *arg ;
        int cnt;
{
        int count = 0;
        int done = 0;
        char *c = arg;

        while (!done && *c) {
                if (*c == '.')
                   {
                        count++;
                        if (count == cnt)
                           done = 1;
                        }
                c++;
	}
	if (done) 
                return c;
        else
                return 0;
}

static anyone (arg,cp)
        char *arg, *cp;
{
        char *c = arg;

        while (*c)
                if (any (*c++,cp))
                        return (1);
        c++;

        return (0);
}

int count_dots (arg)
        char *arg;
{
        char *c = arg;
        int cnt = 0;

        while (*c)
                if (*c++ == '.')
                        cnt++;
        c++;
        return cnt;
}

