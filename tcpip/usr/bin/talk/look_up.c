static char sccsid[] = "@(#)41        1.6  src/tcpip/usr/bin/talk/look_up.c, tcptalk, tcpip411, GOLD410 4/5/91 10:56:42";
/* 
 * COMPONENT_NAME: TCPIP look_up.c
 * 
 * FUNCTIONS: MSGSTR, check_local, look_for_invite 
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
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "talk_ctl.h"

#include <nl_types.h>
#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

/*
 * See if the local daemon has an invitation for us.
 */
check_local()
{
	CTL_RESPONSE response;
	register CTL_RESPONSE *rp = &response;

	/* the rest of msg was set up in get_names */
#ifdef MSG_EOR
	/* copy new style sockaddr to old, swap family (short in old) */
	msg.ctl_addr = *(struct osockaddr *)&ctl_addr;
	msg.ctl_addr.sa_family = htons(ctl_addr.sin_family);
#else
	msg.ctl_addr = *(struct sockaddr *)&ctl_addr; 
#endif
	/* must be initiating a talk */
	if (!look_for_invite(rp))
		return (0);
	/*
	 * There was an invitation waiting for us, 
	 * so connect with the other (hopefully waiting) party 
	 */
	current_state = MSGSTR(WAIT_TO_CONN, "Waiting to connect with caller");
	do {
		if (rp->addr.sa_family != AF_INET)
			p_error(MSGSTR(ERR_INV_RESP, "Response uses invalid network address")); /*MSG*/
		errno = 0;
		if (connect(sockt, &rp->addr, sizeof (rp->addr)) != -1)
			return (1);
	} while (errno == EINTR);
	if (errno == ECONNREFUSED) {
		/*
		 * The caller gave up, but his invitation somehow
		 * was not cleared. Clear it and initiate an 
		 * invitation. (We know there are no newer invitations,
		 * the talkd works LIFO.)
		 */
		ctl_transact(his_machine_addr, msg, DELETE, rp);
		close(sockt);
		open_sockt();
		return (0);
	}
	p_error(MSGSTR(ERR_CONNECT, "Unable to connect with initiator")); /*MSG*/
	/*NOTREACHED*/
}

/*
 * Look for an invitation on 'machine'
 */
look_for_invite(rp)
	CTL_RESPONSE *rp;
{
	struct in_addr machine_addr;

	current_state = MSGSTR(CHK_INVITE, "Checking for invitation on caller's machine");
	ctl_transact(his_machine_addr, msg, LOOK_UP, rp);
	/* the switch is for later options, such as multiple invitations */
	switch (rp->answer) {

	case SUCCESS:
		msg.id_num = htonl(rp->id_num);
		return (1);

	default:
		/* there wasn't an invitation waiting for us */
		return (0);
	}
}
