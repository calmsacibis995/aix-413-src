static char sccsid[] = "@(#)50        1.10  src/tcpip/usr/lbin/talkd/process.c, tcptalk, tcpip411, GOLD410 3/22/91 01:09:21";
/* 
 * COMPONENT_NAME: TCPIP process.c
 * 
 * FUNCTIONS: MSGSTR, do_announce, find_user, process_request 
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
static char sccsid[] = "process.c	5.6 (Berkeley) 6/18/88";
#endif  not lint */

/*
 * process.c handles the requests, which can be of three types:
 *	ANNOUNCE - announce to a user that a talk is wanted
 *	LEAVE_INVITE - insert the request into the table
 *	LOOK_UP - look up to see if a request is waiting in
 *		  in the table for the local user
 *	DELETE - delete invitation
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <netinet/in.h>

#include <protocols/talkd.h>

#ifdef _AIX
#include <sys/access.h>
#endif _AIX

#include <nl_types.h>
#include "talkd_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALKD,n,s) 

char	*strcpy();
CTL_MSG *find_request();
CTL_MSG *find_match();

process_request(mp, rp)
	register CTL_MSG *mp;
	register CTL_RESPONSE *rp;
{
	register CTL_MSG *ptr;
	extern int debug;

	rp->vers = TALK_VERSION;
	rp->type = mp->type;
	rp->id_num = htonl(0);
	if (mp->vers != TALK_VERSION) {
		syslog(LOG_WARNING, MSGSTR(BAD_PROTO_VSN, "Bad protocol version %d"), mp->vers); /*MSG*/
		rp->answer = BADVERSION;
		return;
	}
	mp->id_num = ntohl(mp->id_num);
	mp->addr.sa_family = ntohs(mp->addr.sa_family);
	if (mp->addr.sa_family != AF_INET) {
		syslog(LOG_WARNING, MSGSTR(BAD_ADDR_FAM, "Bad address, family %d"), /*MSG*/
		    mp->addr.sa_family);
		rp->answer = BADADDR;
		return;
	}
	mp->ctl_addr.sa_family = ntohs(mp->ctl_addr.sa_family);
	if (mp->ctl_addr.sa_family != AF_INET) {
		syslog(LOG_WARNING, MSGSTR(BAD_CTL_ADDR, "Bad control address, family %d"), /*MSG*/
		    mp->ctl_addr.sa_family);
		rp->answer = BADCTLADDR;
		return;
	}
	mp->pid = ntohl(mp->pid);
	if (debug)
		print_request(MSGSTR(PROCESS_REQ, "process_request"), mp); /*MSG*/
	switch (mp->type) {

	case ANNOUNCE:
		do_announce(mp, rp);
		break;

	case LEAVE_INVITE:
		ptr = find_request(mp);
		if (ptr != (CTL_MSG *)0) {
			rp->id_num = htonl(ptr->id_num);
			rp->answer = SUCCESS;
		} else
			insert_table(mp, rp);
		break;

	case LOOK_UP:
		ptr = find_match(mp);
		if (ptr != (CTL_MSG *)0) {
			rp->id_num = htonl(ptr->id_num);
			rp->addr = ptr->addr;
			rp->addr.sa_family = htons(ptr->addr.sa_family);
			rp->answer = SUCCESS;
		} else
			rp->answer = NOT_HERE;
		break;

	case DELETE:
		rp->answer = delete_invite(mp->id_num);
		break;

	default:
		rp->answer = UNKNOWN_REQUEST;
		break;
	}
	if (debug)
		print_response(MSGSTR(PROCESS_REQ, "process_request"), rp); /*MSG*/
}

do_announce(mp, rp)
	register CTL_MSG *mp;
	CTL_RESPONSE *rp;
{
	struct hostent *hp;
	CTL_MSG *ptr;
	int result;

	/* see if the user is logged */
	result = find_user(mp->r_name, mp->r_tty);
	if (result != SUCCESS) {
		rp->answer = result;
		return;
	}
#define	satosin(sa)	((struct sockaddr_in *)(sa))
	hp = gethostbyaddr(&satosin(&mp->ctl_addr)->sin_addr,
		sizeof (struct in_addr), AF_INET);
	if (hp == (struct hostent *)0) {
		rp->answer = MACHINE_UNKNOWN;
		return;
	}
	ptr = find_request(mp);
	if (ptr == (CTL_MSG *) 0) {
		insert_table(mp, rp);
		rp->answer = announce(mp, hp->h_name);
		return;
	}
	if (mp->id_num > ptr->id_num) {
		/*
		 * This is an explicit re-announce, so update the id_num
		 * field to avoid duplicates and re-announce the talk.
		 */
		ptr->id_num = new_id();
		rp->id_num = htonl(ptr->id_num);
		rp->answer = announce(mp, hp->h_name);
	} else {
		/* a duplicated request, so ignore it */
		rp->id_num = htonl(ptr->id_num);
		rp->answer = SUCCESS;
	}
}

#include <utmp.h>

/*
 * Search utmp for the local user
 */
find_user(name, tty)
	char *name, *tty;
{
	struct utmp *ubuf, *getutent();
	int status;
	FILE *fd;
	struct stat statb;
	char ftty[20];

	setutent();
#define SCMPN(a, b)	strncmp(a, b, sizeof (a))
	status = NOT_HERE;
	(void) strcpy(ftty, "/dev/");
	while (ubuf = getutent())
		if (SCMPN(ubuf->ut_name, name) == 0 && ubuf->ut_type == USER_PROCESS) {
			if (*tty == '\0') {
				status = PERMISSION_DENIED;
				/* no particular tty was requested */
				(void) strcpy(ftty+5, ubuf->ut_line);
#ifdef _AIX
				if (accessx(ftty, W_ACC, ACC_ALL) == 0) {
#else
				if (stat(ftty,&statb) == 0) {
					if (!(statb.st_mode & 020))
						continue;
#endif _AIX
					(void) strcpy(tty, ubuf->ut_line);
					status = SUCCESS;
					break;
				}
			}
			if (strcmp(ubuf->ut_line, tty) == 0) {
				status = SUCCESS;
				break;
			}
		}
	endutent();
	return (status);
}
