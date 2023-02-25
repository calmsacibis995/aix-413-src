static char sccsid[] = "@(#)43	1.7  src/tcpip/usr/bin/talk/talk.c, tcptalk, tcpip411, GOLD410 2/14/94 13:39:42";
/* 
 * COMPONENT_NAME: TCPIP talk.c
 * 
 * FUNCTIONS: MSGSTR, Mtalk 
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
char copyright[] =
" Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "talk.c	5.4 (Berkeley) 6/29/88";
#endif  not lint */

#include "talk.h"

#include <nl_types.h>
#include "talk_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

/*
 * talk:	A visual form of write. Using sockets, a two way 
 *		connection is set up between the two people talking. 
 *		With the aid of curses, the screen is split into two 
 *		windows, and each users text is added to the window,
 *		one character at a time...
 *
 *		Written by Kipp Hickman
 *		
 *		Modified to run under 4.1a by Clem Cole and Peter Moore
 *		Modified to run between hosts by Peter Moore, 8/19/82
 *		Modified to run under 4.1c by Peter Moore 3/17/83
 */

#include <locale.h>

main(argc, argv)
	int argc;
	char *argv[];
{

	setlocale(LC_ALL,"");
	catd = catopen(MF_TALK,NL_CAT_LOCALE);
	get_names(argc, argv);
	init_display();
	open_ctl();
	open_sockt();
	start_msgs();
	if (!check_local() )
		invite_remote();
	end_msgs();
	set_edit_chars();
	talk();
}
