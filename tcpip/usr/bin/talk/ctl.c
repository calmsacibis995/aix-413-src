static char sccsid[] = "@(#)32        1.7  src/tcpip/usr/bin/talk/ctl.c, tcptalk, tcpip411, GOLD410 10/8/93 10:30:20";
/* 
 * COMPONENT_NAME: TCPIP ctl.c
 * 
 * FUNCTIONS: MSGSTR, open_ctl, open_sockt, print_addr 
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
static char sccsid[] = "ctl.c	5.4 (Berkeley) 6/29/88";
#endif  not lint */

/*
 * This file handles haggling with the various talk daemons to
 * get a socket to talk to. sockt is opened and connected in
 * the progress
 */

#include "talk_ctl.h"

#include <nl_types.h>
#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

struct	sockaddr_in daemon_addr = {sizeof(daemon_addr), AF_INET};
struct	sockaddr_in ctl_addr  = {sizeof(ctl_addr), AF_INET};
struct	sockaddr_in my_addr  = {sizeof(my_addr), AF_INET};

	/* inet addresses of the two machines */
struct	in_addr my_machine_addr;
struct	in_addr his_machine_addr;

u_short daemon_port;	/* port number of the talk daemon */

int	ctl_sockt;
int	sockt;
int	invitation_waiting = 0;

CTL_MSG msg;

open_sockt()
{
	int length;
	
	my_addr.sin_addr = my_machine_addr;
	my_addr.sin_port = 0;
	sockt = socket(AF_INET, SOCK_STREAM, 0);
	if (sockt <= 0)
		p_error(MSGSTR(BAD_SOCK_ERR, "Bad socket")); /*MSG*/
	if (bind(sockt, &my_addr, sizeof(my_addr)) != 0)
		p_error(MSGSTR(BIND_SOCK_ERR, "Binding local socket")); /*MSG*/
	length = sizeof(my_addr);
	if (getsockname(sockt, &my_addr, &length) == -1)
		p_error(MSGSTR(BADSCKADR_ERR, "Bad address for socket")); /*MSG*/
}

/* open the ctl socket */
open_ctl() 
{
	int length;

	ctl_addr.sin_port = 0;
	ctl_addr.sin_addr = my_machine_addr;
	ctl_sockt = socket(AF_INET, SOCK_DGRAM, 0);
	if (ctl_sockt <= 0)
		p_error(MSGSTR(BAD_SOCK_ERR, "Bad socket")); /*MSG*/
	if (bind(ctl_sockt, &ctl_addr, sizeof(ctl_addr)) != 0)
		p_error(MSGSTR(NOBND_CSCK_ERR, "Couldn't bind to control socket")); /*MSG*/
	length = sizeof(ctl_addr);
	if (getsockname(ctl_sockt, &ctl_addr, &length) == -1)
		p_error(MSGSTR(BADADR_CSCK_ERR, "Bad address for ctl socket")); /*MSG*/
}

/* print_addr is a debug print routine */
print_addr(addr)
	struct sockaddr_in addr;
{
	int i;

	printf(MSGSTR(ADR_PORT_FAM_Z, "addr = %x, port = %o, family = %o zero = "), /*MSG*/
		addr.sin_addr, addr.sin_port, addr.sin_family);
	for (i = 0; i<8;i++)
	printf("%o ", (int)addr.sin_zero[i]);
	putchar('\n');
}
