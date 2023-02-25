static char sccsid[] = "@(#)75  1.8.1.1  src/tcpip/usr/bin/ftp/cmdtab.c, tcpfilexfr, tcpip411, GOLD410 3/23/94 19:27:36";
/* 
 * COMPONENT_NAME: TCPIP cmdtab.c
 * 
 * FUNCTIONS: MSGSTR, sizeof 
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
 * Copyright (c) 1985, 1989 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
static char sccsid[] = "cmdtab.c	5.8.1.2 (Berkeley) 3/1/89";
#endif  not lint */

#include <nl_types.h>
#include "ftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTP,n,s) 

#include "ftp_var.h"

/*
 * User FTP -- Command Tables.
 */

/*
 * The following commands have been implemented or enabled according
 * to RFC 1123:
 *	ascii			TYPE A f (f is current format)
 *	binary			TYPE I
 *	block			MODE B
 *	carriage-control	TYPE t C (t is current type)
 *	debug 2			Debug 1 plus keeps restart control file
 *	ebcdic			TYPE E f (f is current format)
 *	exp_cmd			use experimental commands
 *	file			STRU F
 *	form [non-print |
 *	      telnet |
 *	      carriage-control]	TYPE t f (t is current type)
 *	image			TYPE I
 *	local m			TYPE L m
 *	mode [stream | block]	MODE m
 *	mount s			SMNT s
 *	non-print		TYPE t N (t is current type)
 *	record			STRU R
 * 	reinitialize		REIN
 *	restart get | put |
 *		append file	REST m (m is restart control file info)
 *				RETR | STOR | APPE file
 *	site args		SITE args (up to 3 args)
 *	status			shows new variables
 *	stream			MODE S
 *	struct [file | record]	STRU s
 *	telnet			TYPE t T (t is current type)
 *	tenex			TYPE L 8
 *	type [ascii | ebcdic |
 *	      binary | image |
 *	      tenex | local m]	TYPE t f (f is current format)
 *
 * Note that the size command is not compatible with the restart command.
 *
 * Note that the proxy command may be used for restart.
 */

int	setascii(), setbell(), setbinary(), setblock(), setcar_ctl();
int	setdebug(), setebcdic(), setfile(), setform();
int	setglob(), sethash(), setlocal(), setmode(), setexp_cmd();
int	setnon_print(), setpeer(), setport();
int	setprompt(), setrecord(), setstream(), setstruct();
int	settelnet(), settenex(), settrace(), settype(), setverbose();
int	disconnect(), syst(), copyloc();
int	cd(), lcd(), delete(), mdelete(), mount(), user();
int	ls(), mls(), get(), mget(), help(), append(), put(), mput();
int	quit(), reinit(), renamefile(), restart(), status();
int	quote(), rmthelp(), shell();
int	pwd(), makedir(), removedir(), setcr();
int	account(), doproxy(), reset(), setcase(), setntrans(), setnmap();
int	setsunique(), setrunique(), cdup(), macdef(), domacro();
int	site(), sizecmd(), modtime(), rmtstatus();

char	accounthelp[] =	"send account command to remote server";
char	appendhelp[] =	"append to a file";
char	asciihelp[] =	"set ascii transfer type";
char	beephelp[] =	"beep when command completed";
char	binaryhelp[] =	"set binary transfer type";
char	blockhelp[] =	"set block transfer mode";
char	car_ctlhelp[] =	"set carriage-control transfer form";
char	casehelp[] =	"toggle mget upper/lower case id mapping";
char	cdhelp[] =	"change remote working directory";
char	cduphelp[] = 	"change remote working directory to parent directory";
char	connecthelp[] =	"connect to remote ftp";
char	copylochelp[] = "toggle for local copy";
char	crhelp[] =	"toggle carriage return stripping on ascii gets";
char	deletehelp[] =	"delete remote file";
char	debughelp[] =	"toggle/set debugging mode";
char	dirhelp[] =	"list contents of remote directory";
char	disconhelp[] =	"terminate ftp session";
char	domachelp[] = 	"execute macro";
char	ebcdichelp[] = 	"set ebcdic transfer type";
char	exp_cmdhelp[] = "toggle experimental commands mode";
char	filehelp[] = 	"set file transfer structure";
char	formhelp[] =	"set transfer format";
char	globhelp[] =	"toggle metacharacter expansion of local file names";
char	hashhelp[] =	"toggle printing `#' for each buffer transferred";
char	helphelp[] =	"print local help information";
char	lcdhelp[] =	"change local working directory";
char	localhelp[] =	"set local transfer type";
char	lshelp[] =	"list contents of remote directory";
char	macdefhelp[] =  "define a macro";
char	mdeletehelp[] =	"delete multiple files";
char	mdirhelp[] =	"list contents of multiple remote directories";
char	mgethelp[] =	"get multiple files";
char	mkdirhelp[] =	"make directory on the remote machine";
char	mlshelp[] =	"nlist contents of multiple remote directories";
char	modtimehelp[] = "show last modification time of remote file";
char	modehelp[] =	"set transfer mode";
char	mounthelp[] =	"structure mount";
char	mputhelp[] =	"send multiple files";
char	nlisthelp[] =	"nlist contents of remote directory";
char	nmaphelp[] =	"set templates for default file name mapping";
char	non_printhelp[]
		 =	"set non-print transfer form";
char	ntranshelp[] =	"set translation table for default file name mapping";
char	porthelp[] =	"toggle use of PORT cmd for each data connection";
char	prompthelp[] =	"force interactive prompting on multiple commands";
char	proxyhelp[] =	"issue command on alternate connection";
char	pwdhelp[] =	"print working directory on remote machine";
char	quithelp[] =	"terminate ftp session and exit";
char	quotehelp[] =	"send arbitrary ftp command";
char	receivehelp[] =	"receive file";
char	recordhelp[] =	"set record transfer structure";
char	reinithelp[] =	"reinitialize";
char	remotehelp[] =	"get help from remote server";
char	renamehelp[] =	"rename file";
char	resethelp[] =	"clear queued command replies";
char	restarthelp[] =	"restart failed file transfer";
char	rmdirhelp[] =	"remove directory on the remote machine";
char	rmtstatushelp[]="show status of remote machine";
char	runiquehelp[] = "toggle store unique for local files";
char	sendhelp[] =	"send one file";
char	shellhelp[] =	"escape to the shell";
char	sitehelp[] = 	"site specific commands";
char	sizecmdhelp[] = "show size of remote file";
char	statushelp[] =	"show current status";
char	streamhelp[] =	"set stream transfer mode";
char	structhelp[] =	"set transfer structure";
char	suniquehelp[] = "toggle store unique on remote machine";
char	systemhelp[] =  "show remote system type";
char	telnethelp[] =	"set telnet transfer form";
char	tenexhelp[] =	"set tenex transfer type";
char	tracehelp[] =	"toggle packet tracing";
char	typehelp[] =	"set transfer type";
char	userhelp[] =	"send new user information";
char	verbosehelp[] =	"toggle verbose mode";


struct cmd cmdtab[] = {
	{ "!",
	SHELLHELP,	shellhelp,	0,	0,	0,	shell },
	{ "$",
	DOMACHELP,	domachelp,	1,	0,	0,	domacro },
	{ "account",
	ACCOUNTHELP,	accounthelp,	0,	1,	1,	account},
	{ "append ",
	APPENDHELP,	appendhelp,	1,	1,	1,	put },
	{ "ascii",	
	ASCIIHELP,	asciihelp,	0,	1,	1,	setascii },
	{ "bell",	
	BEEPHELP,	beephelp,	0,	0,	0,	setbell },
	{ "binary",	
	BINARYHELP,	binaryhelp,	0,	1,	1,	setbinary },
	{ "block",	
	BLOCKHELP,	blockhelp,	0,	1,	1,	setblock },
	{ "bye",	
	QUITHELP,	quithelp,	0,	0,	0,	quit },
	{ "carriage-control",	
	CARCTLHELP,	car_ctlhelp,	0,	1,	1,	setcar_ctl },
	{ "case",	
	CASEHELP,	casehelp,	0,	0,	1,	setcase },
	{ "cd",		
	CDHELP,		cdhelp,		0,	1,	1,	cd },
	{ "cdup",	
	CDUPHELP,	cduphelp,	0,	1,	1,	cdup },
	{ "close",	
	DISCONHELP,	disconhelp,	0,	1,	1,	disconnect },
	{ "copylocal",
	COPYLOCHELP,    copylochelp,    0,      0,      0,      copyloc },
	{ "cr",		
	CRHELP,		crhelp,		0,	0,	0,	setcr },
	{ "delete",	
	DELETEHELP,	deletehelp,	0,	1,	1,	delete },
	{ "debug",	
	DEBUGHELP,	debughelp,	0,	0,	0,	setdebug },
	{ "dir",	
	DIRHELP,	dirhelp,	1,	1,	1,	ls },
	{ "disconnect",	
	DISCONHELP,	disconhelp,	0,	1,	1,	disconnect },
	{ "ebcdic",	
	EBCDICHELP,	ebcdichelp,	0,	1,	1,	setebcdic },
	{ "exp_cmd",	
	EXP_CMDHELP,	exp_cmdhelp,	0,	0,	1,	setexp_cmd },
	{ "file",	
	FILEHELP,	filehelp,	0,	1,	1,	setfile },
	{ "form",	
	FORMHELP,	formhelp,	0,	1,	1,	setform },
	{ "get",	
	RECEIVEHELP,	receivehelp,	1,	1,	1,	get },
	{ "glob",	
	GLOBHELP,	globhelp,	0,	0,	0,	setglob },
	{ "hash",	
	HASHHELP,	hashhelp,	0,	0,	0,	sethash },
	{ "help",	
	HELPHELP,	helphelp,	0,	0,	1,	help },
	{ "image",	
	BINARYHELP,	binaryhelp,	0,	1,	1,	setbinary },
	{ "lcd",	
	LCDHELP,	lcdhelp,	0,	0,	0,	lcd },
	{ "local",	
	LOCALHELP,	localhelp,	0,	1,	1,	setlocal },
	{ "ls",		
	LSHELP,		lshelp,		1,	1,	1,	ls },
	{ "macdef",	
	MACDEFHELP,	macdefhelp,	0,	0,	0,	macdef },
	{ "mdelete",	
	MDELETEHELP,	mdeletehelp,	1,	1,	1,	mdelete },
	{ "mdir",	
	MDIRHELP,	mdirhelp,	1,	1,	1,	mls },
	{ "mget",	
	MGETHELP,	mgethelp,	1,	1,	1,	mget },
	{ "mkdir",	
	MKDIRHELP,	mkdirhelp,	0,	1,	1,	makedir },
	{ "mls",	
	MLSHELP,	mlshelp,	1,	1,	1,	mls },
	{ "mode",	
	MODEHELP,	modehelp,	0,	1,	1,	setmode },
	{ "modtime",	
	MODTIMEHELP,	modtimehelp,	0,	1,	1,	modtime },
	{ "mount",	
	MOUNTHELP,	mounthelp,	1,	1,	1,	mount },
	{ "mput",	
	MPUTHELP,	mputhelp,	1,	1,	1,	mput },
	{ "nmap",	
	NMAPHELP,	nmaphelp,	0,	0,	1,	setnmap },
	{ "nlist",	
	NLISTHELP,	nlisthelp,	1,	1,	1,	ls },
	{ "non-print",	
	NONPRINTHELP,	non_printhelp,	0,	1,	1,	setnon_print },
	{ "ntrans",	
	NTRANSHELP,	ntranshelp,	0,	0,	1,	setntrans },
	{ "open",	
	CONNECTHELP,	connecthelp,	0,	0,	1,	setpeer },
	{ "prompt",	
	PROMPTHELP,	prompthelp,	0,	0,	0,	setprompt },
	{ "proxy",	
	PROXYHELP,	proxyhelp,	0,	0,	1,	doproxy },
	{ "put",	
	SENDHELP,	sendhelp,	1,	1,	1,	put },
	{ "pwd",	
	PWDHELP,	pwdhelp,	0,	1,	1,	pwd },
	{ "quit",	
	QUITHELP,	quithelp,	0,	0,	0,	quit },
	{ "quote",	
	QUOTEHELP,	quotehelp,	1,	1,	1,	quote },
	{ "record",	
	RECORDHELP,	recordhelp,	1,	1,	1,	setrecord },
	{ "recv",	
	RECEIVEHELP,	receivehelp,	1,	1,	1,	get },
	{ "reinitialize",	
	REINITHELP,	reinithelp,	1,	1,	1,	reinit },
	{ "remotehelp",	
	REMOTEHELP,	remotehelp,	0,	1,	1,	rmthelp },
	{ "rename",	
	RENAMEHELP,	renamehelp,	0,	1,	1,	renamefile },
	{ "reset",	
	RESETHELP,	resethelp,	0,	1,	1,	reset },
	{ "restart",	
	RESTARTHELP,	restarthelp,	0,	1,	1,	restart },
	{ "rhelp",	
	REMOTEHELP,	remotehelp,	0,	1,	1,	rmthelp },
	{ "rmdir",	
	RMDIRHELP,	rmdirhelp,	0,	1,	1,	removedir },
	{ "rstatus",	
	RMTSTATUSHELP,	rmtstatushelp,	0,	1,	1,	rmtstatus },
	{ "runique",	
	RUNIQUEHELP,	runiquehelp,	0,	0,	1,	setrunique },
	{ "send",	
	SENDHELP,	sendhelp,	1,	1,	1,	put },
	{ "sendport",	
	PORTHELP,	porthelp,	0,	0,	0,	setport },
	{ "site",	
	SITEHELP,	sitehelp,	1,	1,	1,	site },
	{ "size",	
	SIZECMDHELP,	sizecmdhelp,	1,	1,	1,	sizecmd },
	{ "status",	
	STATUSHELP,	statushelp,	0,	0,	1,	status },
	{ "stream",	
	STREAMHELP,	streamhelp,	0,	0,	1,	setstream },
	{ "struct",	
	STRUCTHELP,	structhelp,	0,	1,	1,	setstruct },
	{ "sunique",	
	SUNIQUEHELP,	suniquehelp,	0,	0,	1,	setsunique },
	{ "system",	
	SYSTEMHELP,	systemhelp,	0,	1,	1,	syst },
	{ "telnet",	
	TELNETHELP,	telnethelp,	0,	1,	1,	settelnet },
	{ "tenex",	
	TENEXHELP,	tenexhelp,	0,	1,	1,	settenex },
	{ "trace",	
	TRACEHELP,	tracehelp,	0,	0,	0,	settrace },
	{ "type",	
	TYPEHELP,	typehelp,	0,	1,	1,	settype },
	{ "user",	
	USERHELP,	userhelp,	0,	1,	1,	user },
	{ "verbose",	
	VERBOSEHELP,	verbosehelp,	0,	0,	0,	setverbose },
	{ "?",		
	HELPHELP,	helphelp,	0,	0,	1,	help },
	{ 0 },
};

int	NCMDS = (sizeof (cmdtab) / sizeof (cmdtab[0])) - 1;
