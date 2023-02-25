static char sccsid[] = "@(#)50	1.3.1.1  src/tcpip/usr/sbin/timed/candidate.c, tcptimer, tcpip411, GOLD410 3/25/94 19:07:32";
/* 
 * COMPONENT_NAME: TCPIP candidate.c
 * 
 * FUNCTIONS: MSGSTR, election 
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
static char sccsid[] = "candidate.c	2.5 (Berkeley) 6/18/88";
#endif  not lint */

#include "globals.h"
#include <protocols/timed.h>

#include <nl_types.h>
#include "timed_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s)  catgets(catd, MS_TIMED, n, s)

#define ELECTIONWAIT	3	/* seconds */

/*
 * `election' candidates a host as master: it is called by a slave 
 * which runs with the -M option set when its election timeout expires. 
 * Note the conservative approach: if a new timed comes up, or another
 * candidate sends an election request, the candidature is withdrawn.
 */

election(net)
struct netinfo *net;
{
	int ret;
	struct tsp *resp, msg, *readmsg();
	struct timeval wait;
	struct tsp *answer, *acksend();
	long casual();
	struct sockaddr_in server;

	syslog(LOG_INFO, MSGSTR(CANDIDATE, "THIS MACHINE IS A CANDIDATE"));
	if (trace) {
		fprintf(fd, MSGSTR(CANDIDATE, "THIS MACHINE IS A CANDIDATE\n"));
	}

	ret = MASTER;
	slvcount = 1;

	msg.tsp_type = TSP_ELECTION;
	msg.tsp_vers = TSPVERSION;
	(void)strcpy(msg.tsp_name, hostname);
	bytenetorder(&msg);
	if (sendto(sock, (char *)&msg, sizeof(struct tsp), 0, &net->dest_addr,
	    sizeof(struct sockaddr_in)) < 0) {
		syslog(LOG_ERR, MSGSTR(SENDTO2, "sendto: %m"));
		return(SLAVE);
	}

	do {
		wait.tv_sec = ELECTIONWAIT;
		wait.tv_usec = 0;
		resp = readmsg(TSP_ANY, (char *)ANYADDR, &wait, net);
		if (resp != NULL) {
			switch (resp->tsp_type) {

			case TSP_ACCEPT:
				(void) addmach(resp->tsp_name, &from);
				break;

			case TSP_MASTERUP:
			case TSP_MASTERREQ:
				/*
				 * If a timedaemon is coming up at the same time,
				 * give up the candidature: it will be the master.
				 */
				ret = SLAVE;
				break;

			case TSP_QUIT:
			case TSP_REFUSE:
				/*
				 * Collision: change value of election timer 
				 * using exponential backoff.
				 * The value of timer will be recomputed (in slave.c)
				 * using the original interval when election will 
				 * be successfully completed.
				 */
				backoff *= 2;
				delay2 = casual((long)MINTOUT, 
							(long)(MAXTOUT * backoff));
				ret = SLAVE;
				break;

			case TSP_ELECTION:
				/* no master for another round */
				msg.tsp_type = TSP_REFUSE;
				(void)strcpy(msg.tsp_name, hostname);
				server = from;
				answer = acksend(&msg, &server, resp->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (answer == NULL) {
					syslog(LOG_ERR, MSGSTR(ELECTION, "error in election"));
				} else {
					(void) addmach(resp->tsp_name, &from);
				}
				break;

			case TSP_SLAVEUP:
				(void) addmach(resp->tsp_name, &from);
				break;

			case TSP_SETDATE:
			case TSP_SETDATEREQ:
				break;

			default:
				if (trace) {
					fprintf(fd, MSGSTR(CAND, "candidate: "));
					print(resp, &from);
				}
				break;
			}
		} else {
			break;
		}
	} while (ret == MASTER);
	return(ret);
}
