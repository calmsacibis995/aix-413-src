static char sccsid[] = "@(#)35        1.7  src/tcpip/usr/bin/talk/get_addrs.c, tcptalk, tcpip411, GOLD410 3/19/91 14:33:39";
/* 
 * COMPONENT_NAME: TCPIP get_addrs.c
 * 
 * FUNCTIONS: MSGSTR, get_addrs 
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
static char sccsid[] = "get_addrs.c	5.5 (Berkeley) 10/11/88";
#endif  not lint */

#include "talk_ctl.h"
#include <nl_types.h>
#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 
#include <netdb.h>

get_addrs(my_machine_name, his_machine_name)
	char *my_machine_name, *his_machine_name;
{
	struct hostent *hp;
	struct servent *sp;
	long	address;

	msg.pid = htonl(getpid());
	/* look up the address of the local host */
	/* see if it's a name or an address */
        if (!isinet_addr(my_machine_name)) {
              if ((hp = gethostbyname(my_machine_name))==(struct hostent *) 0) {
                     fprintf(stderr,MSGSTR(NME_NT_FND,
                                 "host: name %s NOT FOUND\n"), my_machine_name);
                     exit(1);
                }
        }
        else {
		if ((address = inet_addr(my_machine_name)) == -1) {
                        fprintf(stderr, MSGSTR(BADADDR_MSG,
                                "%s: address misformed\n"), my_machine_name);
                        exit(2);
                        }
                if ((hp = gethostbyaddr(&address, sizeof(address),
                                        AF_INET)) == (struct hostent *)0) {
			fprintf(stderr, MSGSTR(HOST,
                               "talk: %s: Can't figure out network address.\n"),
                                my_machine_name);
                        exit(2);
                        }
        }

	bcopy(hp->h_addr, (char *)&my_machine_addr, hp->h_length);
	/*
	 * If the callee is on-machine, just copy the
	 * network address, otherwise do a lookup...
	 */
	if (strcmp(his_machine_name, my_machine_name)) {
	    /* see if it's a name or an address */
            if (!isinet_addr(his_machine_name)) {
              if ((hp =gethostbyname(his_machine_name))==(struct hostent *) 0) {
                     fprintf(stderr,MSGSTR(NME_NT_FND,
                                 "host: name %s NOT FOUND\n"),his_machine_name);
                     exit(1);
                }
            }
            else {
                if ((address = inet_addr(his_machine_name)) == -1) {
                        fprintf(stderr, MSGSTR(BADADDR_MSG,
                                "%s: address misformed\n"), his_machine_name);
                        exit(2);
                        }
                if ((hp = gethostbyaddr(&address, sizeof(address),
                                        AF_INET)) == (struct hostent *)0) {
                        fprintf(stderr, MSGSTR(HOST,
                               "talk: %s: Can't figure out network address.\n"),
                                his_machine_name);
                        exit(2);
                        }
             }
	     bcopy(hp->h_addr, (char *) &his_machine_addr, hp->h_length);
	} else
		his_machine_addr = my_machine_addr;
	/* find the server's port */
	sp = getservbyname("ntalk", "udp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(ERR_NO_SERVICE, "talk: %s/%s: service is not registered.\n"), /*MSG*/
		     "ntalk", "udp");
		exit(-1);
	}
	daemon_port = sp->s_port;
}
