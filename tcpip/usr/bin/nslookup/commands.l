%{
/* @(#)59	1.2  src/tcpip/usr/bin/nslookup/commands.l, tcpras, tcpip411, GOLD410 6/3/93 11:27:32 */
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: yylex
 *
 *   ORIGINS: 26 27 65
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1 snapshot 4
 *
 * Copyright (c) 1985 Regents of the University of California.
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
 *
 * commands.l	5.12 (Berkeley) 5/27/90
 */

/*
 *******************************************************************************
 *
 *  commands.l
 *
 *	Andrew Cherenson	CS298-26  Fall 1985
 *
 *	Lex input file for the nslookup program command interpreter.
 *	When a sequence is recognized, the associated action
 *	routine is called. The action routine may need to
 *	parse the string for additional information.
 *
 *  Recognized commands: (identifiers are shown in uppercase)
 *
 *	server NAME	- set default server to NAME, using default server
 *	lserver NAME	- set default server to NAME, using initial server
 *	finger [NAME]	- finger the optional NAME
 *	exit		- exit the program
 *	root		- set default server to the root
 *	ls NAME		- list the domain NAME
 *	view FILE	- sorts and view the file with more
 *	set OPTION	- set an option
 *	help		- print help information
 *	?		- print help information
 *	NAME		- print info about the host/domain NAME
 *			  using default server.
 *	NAME1 NAME2	- as above, but use NAME2 as server
 *
 *
 *   yylex Results:
 *	0		upon end-of-file.
 *	1		after each command.
 *
 *******************************************************************************
 */

#include "res.h"
extern char rootServerName[];

%}
WS	[ \t]
FLET	[A-Za-z0-9.*\\]
LET	[A-Za-z0-9.*]
NAME	[A-Za-z0-9.*=_/-]
%%
^{WS}*server{WS}+{LET}{NAME}*{WS}*$	{
					    /*
					     * 0 == use current server to find
					     *	    the new one.
					     * 1 == use original server to find
					     *	    the new one.
					     */
					    SetDefaultServer(yytext, 0);
					    return(1);
					}
^{WS}*lserver{WS}+{LET}{NAME}*{WS}*$	{
					    SetDefaultServer(yytext, 1);
					    return(1);
					}
^{WS}*exit{WS}*$			{
					    return(0);
					}
^{WS}*root{WS}*$			{
					    SetDefaultServer(rootServerName, 1);
					    return(1);
					}
^{WS}*finger({WS}+{LET}{NAME}*)?{WS}+>>?{WS}*{NAME}+{WS}*$	{
					    /*
					     * 2nd arg.
					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    Finger(yytext, 1);
					    return(1);
					}
^{WS}*finger({WS}+{LET}{NAME}*)?{WS}*$	{
					    Finger(yytext, 0);
					    return(1);
					}
^{WS}*view{WS}+{NAME}+{WS}*$	{
					    ViewList(yytext);
					    return(1);
					}
^{WS}*ls{WS}+(("-a"|"-d"|"-h"|"-m"|"-s"){WS}+)?{LET}{NAME}*{WS}+>>?{WS}+{NAME}+{WS}*$	{
					    /*
					     * 2nd arg.
					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    ListHosts(yytext, 1);
					    return(1);
					}
^{WS}*ls{WS}+(("-a"|"-d"|"-h"|"-m"|"-s"){WS}+)?{LET}{NAME}*{WS}*$	{
					    ListHosts(yytext, 0);
					    return(1);
					}
^{WS}*ls{WS}+-t{WS}+({LET}{NAME}*{WS}+)?{LET}{NAME}*{WS}+>>?{WS}+{NAME}+{WS}*$ {
					    /*
					     * 2nd arg.
					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    ListHostsByType(yytext, 1);
					    return(1);
					}
^{WS}*ls{WS}+-t{WS}+({LET}{NAME}*{WS}+)?{LET}{NAME}*{WS}*$ {
					    ListHostsByType(yytext, 0);
					    return(1);
					}
^{WS}*set{WS}+{NAME}+{WS}*$		{
					    SetOption(yytext);
					    return(1);
					}
^{WS}*help{WS}*$			{
					    extern void PrintHelp();

					    PrintHelp();
					    return(1);
					}
^{WS}*"?"{WS}*$				{
					    PrintHelp();
					    return(1);
					}
^{WS}*{FLET}{NAME}*{WS}+>>?{WS}*{NAME}+{WS}*$	{
					    /*
					     * 0 == output to stdout
					     * 1 == output to file
					     */
					    LookupHost(yytext, 1);
					    return(1);
					}
^{WS}*{FLET}{NAME}*{WS}*$		{
					    LookupHost(yytext, 0);
					    return(1);
					}
^{WS}*{FLET}{NAME}*{WS}+{LET}{NAME}*{WS}+>>?{WS}*{NAME}+{WS}*$	{
					    /*
					     * 0 == output to stdout
					     * 1 == output to file
					     */
					    LookupHostWithServer(yytext, 1);
					    return(1);
					}
^{WS}*{FLET}{NAME}*{WS}+{LET}{NAME}*{WS}*$	{
					    LookupHostWithServer(yytext, 0);
					    return(1);
					}
^{WS}*\n				{
					    return(1);
					}
^.*\n					{
					    printf("Unrecognized command: %s",
							yytext);
					    return(1);
					}
\n					{ ; }
%%
