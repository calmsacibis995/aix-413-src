static char sccsid[] = "@(#)46	1.4  src/tcpip/usr/sbin/timed/cmdtab.c, tcptimer, tcpip411, GOLD410 3/18/91 11:38:06";
/* 
 * COMPONENT_NAME: TCPIP cmdtab.c
 * 
 * FUNCTIONS: MSGSTR, sizeof, tab_load 
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
static char sccsid[] = "cmdtab.c	2.5 (Berkeley) 6/18/88";
#endif  not lint */

#include "timedc.h"

int	clockdiff(), help(), msite(), quit(), testing(), tracing();

#include <nl_types.h>
#include "timed_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_TIMEDC, n, s)

struct table {
    char   cmd[20];
    char   hlp[90];
} cmd_hlp[7];

struct cmd cmdtab[] = {
	{ cmd_hlp[0].cmd, cmd_hlp[0].hlp, 	clockdiff,	0 },
	{ cmd_hlp[1].cmd, cmd_hlp[1].hlp,	help,		0 },
	{ cmd_hlp[2].cmd, cmd_hlp[2].hlp,	msite,		0 },
	{ cmd_hlp[3].cmd, cmd_hlp[3].hlp,	quit,		0 },
	{ cmd_hlp[4].cmd, cmd_hlp[4].hlp,	tracing,	1 },
	{ cmd_hlp[5].cmd, cmd_hlp[5].hlp,	help,		0 },
#ifdef TESTING
	{ cmd_hlp[6].cmd, cmd_hlp[6].hlp,	testing,	1 },
#endif
};
int	NCMDS = sizeof (cmdtab) / sizeof (cmdtab[0]);
tab_load ()
{
    int i;

    
    for (i =0 ; i < 7; i++)
    {
	cmdtab[i].c_name = cmd_hlp[i].cmd;
	cmdtab[i].c_help = cmd_hlp[i].hlp;
    }
    strcpy(cmd_hlp[0].cmd, MSGSTR(C_CLOCKDIFF, "clockdiff"));
    strcpy(cmd_hlp[1].cmd, MSGSTR(C_HELP, "help"));
    strcpy(cmd_hlp[2].cmd, MSGSTR(C_MSITE, "msite"));
    strcpy(cmd_hlp[3].cmd, MSGSTR(C_QUIT, "quit"));
    strcpy(cmd_hlp[4].cmd, MSGSTR(C_TRACE, "trace"));
    strcpy(cmd_hlp[5].cmd, MSGSTR(C_QUES, "?"));
    strcpy(cmd_hlp[6].cmd, MSGSTR(C_ELECTION, "election"));
    strcpy(cmd_hlp[0].hlp, MSGSTR(H_CLOCKDIFF, "measures clock differences between machines"));
    strcpy(cmd_hlp[1].hlp, MSGSTR(H_HELP, "gets help on commands")); 
    strcpy(cmd_hlp[2].hlp, MSGSTR(H_MSITE, "finds location of master"));
    strcpy(cmd_hlp[3].hlp, MSGSTR(H_QUIT, "exits timedc"));
    strcpy(cmd_hlp[4].hlp, MSGSTR(H_TRACE, "turns tracing on or off"));
    strcpy(cmd_hlp[5].hlp, MSGSTR(H_HELP, "gets help on commands"));
    strcpy(cmd_hlp[6].hlp, MSGSTR(H_ELECTION, "causes election timers to expire"));

}
