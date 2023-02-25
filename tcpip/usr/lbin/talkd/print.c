static char sccsid[] = "@(#)49	1.7  src/tcpip/usr/lbin/talkd/print.c, tcptalk, tcpip411, GOLD410 3/18/91 11:59:10";
/* 
 * COMPONENT_NAME: TCPIP print.c
 * 
 * FUNCTIONS: MSGSTR, print_request, print_response 
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
static char sccsid[] = "print.c	5.6 (Berkeley) 6/18/88";
#endif  not lint */

/* debug print routines */

#include <stdio.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <netinet/in.h>

#include <protocols/talkd.h>

#include <nl_types.h>
#include "talkd_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALKD,n,s) 

static	char *types[] = { 
	"leave_invite", 		/* LEAVE_INVITE */
	"look_up", 			/* LOOK_UP */
	"delete", 			/* DELETE */
	"announce" 			/* ANNOUNCE */
};
#define	NTYPES	(sizeof (types) / sizeof (types[0]))
static	char *answers[] = { 
	"success", 			/* SUCCESS */
	"not_here", 			/* NOT_HERE */
	"failed", 			/* FAILED */
	"machine_unknown", 		/* MACHINE_UNKNOWN */
	"permission_denied",		/* PERMISSION_DENIED */
      	"unknown_request", 		/* UNKNOWN_REQUEST */
	"badversion", 			/* BADVERSION */
	"badaddr", 			/* BADADDR */
	"badctladdr" 			/* BADCTLADDR */
};
#define	NANSWERS	(sizeof (answers) / sizeof (answers[0]))

print_request(cp, mp)
	char *cp;
	register CTL_MSG *mp;
{
	char tbuf[80], *tp;
	
	if (mp->type > NTYPES) {
		sprintf(tbuf, MSGSTR(MSG_TYPE, "type %d"), mp->type); /*MSG*/
		tp = tbuf;
	} else
		tp = MSGSTR(MSG_LV_INV + mp->type, types[mp->type]);
	syslog(LOG_DEBUG, MSGSTR(ID_INFO_SYS, "%s: %s: id %d, l_user %s, r_user %s, r_tty %s"), /*MSG*/
	    cp, tp, mp->id_num, mp->l_name, mp->r_name, mp->r_tty);
}

print_response(cp, rp)
	char *cp;
	register CTL_RESPONSE *rp;
{
	char tbuf[80], *tp, abuf[80], *ap;
	
	if (rp->type > NTYPES) {
		sprintf(tbuf, MSGSTR(MSG_TYPE, "type %d"), rp->type); /*MSG*/
		tp = tbuf;
	} else
		tp = MSGSTR(MSG_LV_INV + rp->type, types[rp->type]);
	if (rp->answer > NANSWERS) {
		sprintf(abuf, MSGSTR(ANS_SYS, "answer %d"), rp->answer); /*MSG*/
		ap = abuf;
	} else
		ap = MSGSTR(MSG_SUCCESS + rp->answer, answers[rp->answer]);
	syslog(LOG_DEBUG, MSGSTR(ID_SYSLOG, "%s: %s: %s, id %d"), cp, tp, ap, ntohl(rp->id_num)); /*MSG*/
}
