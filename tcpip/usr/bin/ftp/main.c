static char sccsid[] = "@(#)14      1.20.1.1  src/tcpip/usr/bin/ftp/main.c, tcpfilexfr, tcpip411, GOLD410 2/14/94 17:04:21";
/* 
 * COMPONENT_NAME: TCPIP main.c
 * 
 * FUNCTIONS: CTRL, MSGSTR, Mmain, call, cmdscanner, getcmd, help, 
 *            intr, lostpeer, makeargv, slurpstring 
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
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint
*/

/*
 * FTP User Program -- Command Interface.
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

#include "ftp_var.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#ifdef aiws
#define CTRL(c) ('c'&037)
#endif

#include <arpa/ftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/syslog.h>
#include <stdlib.h>

#include <nl_types.h>
#include "ftp_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTP,n,s) 

#ifdef _CSECURITY
#include <sys/id.h>
#include "tcpip_audit.h"
char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
uid_t	saved_uid, effective_uid;
char	username[64];
#endif _CSECURITY

uid_t	getuid();
int	intr();
int	lostpeer();
extern	char *home;
char	*getlogin();

extern struct cmd cmdtab[];

#include <locale.h>

main(argc, argv)
	char *argv[];
{
	register char *cp;
	int top;
	struct passwd *pw = NULL;
	char homedir[MAXPATHLEN];
	struct servent *ftamsp ;

	setlocale(LC_ALL,"");
	catd = catopen (MF_FTP,NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;

#ifdef _CSECURITY
	/* open log for syslog messages*/
        openlog("ftp", LOG_PID|LOG_NDELAY, LOG_USER);
	if ((auditproc((char *)0,AUDIT_STATUS,AUDIT_SUSPEND, (char *)0)) < 0) {
		syslog(LOG_ALERT, "ftp: auditproc: %m");
		exit(1);
	}
	effective_uid = getuid();
	saved_uid = getuidx(ID_SAVED);
#endif _CSECURITY

	seteuid(getuid());

	ftamsp = getservbyname("ftp-ftam", "tcp");
	if(ftamsp == 0) ftam_port = -1 ;
	else ftam_port= ftamsp->s_port ;

	sp = getservbyname("ftp", "tcp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKNOWN_SERVICE, "ftp: ftp/tcp: unknown service\n")); /*MSG*/
		exit(1);
	}

	doglob = 1;
	interactive = 1;
	autologin = 1;
	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		for (cp = *argv + 1; *cp; cp++)
			switch (*cp) {

			case 'd':
				options |= SO_DEBUG;
				debug++;
				break;
			
			case 'v':
				verbose++;
				break;

			case 't':
				trace++;
				break;

			case 'i':
				interactive = 0;
				break;

			case 'n':
				autologin = 0;
				break;

			case 'g':
				doglob = 0;
				break;

			default:
				fprintf(stdout, MSGSTR(UNKNOWN_OPTION, "ftp: %c: unknown option\n"), *cp); /*MSG*/
				exit(1);
			}
		argc--, argv++;
	}
	fromatty = isatty(fileno(stdin));
	/*
	 * Set up defaults for FTP.
	 */
	siginterrupt(SIGCONT, 0);		/* set restartable system
						   call instead of EINTR */
	(void) strcpy(typename, "ascii"), type = TYPE_A;
	(void) strcpy(typecode, "A");
	(void) strcpy(formname, "non-print"), form = FORM_N;
	(void) strcpy(formcode, "N");
	(void) strcpy(modename, "stream"), mode = MODE_S;
	(void) strcpy(structname, "file"), stru = STRU_F;
	(void) strcpy(bytename, "8"), bytesize = 8;
        line_state = START_OF_LINE;
        car_ctl_char = ' ';
        next_bits = bytesize;
        out_bits = 0;
        out_char = '\0';
        rest_count = 0x20000;	/* restart markers at 128 K */
	rest_flag = 0;		/* restart not pending */
	rest_name[0] = '\0';
        rest_mark = 0;		/* restart not pending */
	exp_cmds = 0;		/* experimental commands off */
	if (fromatty)
		verbose++;
	cpend = 0;           /* no pending replies */
	proxy = 0;	/* proxy not active */
	crflag = 1;    /* strip c.r. on ascii gets */
	/*
	 * Set up the home directory in case we're globbing.
	 */
	cp = getlogin();
	if (cp != NULL) {
		pw = getpwnam(cp);
	}
	if (pw == NULL)
		pw = getpwuid(getuid());
	if (pw != NULL) {
		home = &homedir[0];
		(void) strcpy(home, pw->pw_dir);
#ifdef _CSECURITY
		/* added for syslog messages */
                (void) strcpy(username, pw->pw_name);
	} else {
                (void) strcpy(username, "UNKNOWN");
#endif _CSECURITY
	}
	if (argc > 0) {
		if (setjmp(toplevel))
			exit(0);
		(void) signal(SIGINT, intr);
		(void) signal(SIGPIPE, lostpeer);
		setpeer(argc + 1, argv - 1);
	}
	top = setjmp(toplevel) == 0;
	if (top) {
		(void) signal(SIGINT, intr);
		(void) signal(SIGPIPE, lostpeer);
	}
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

intr()
{

	longjmp(toplevel, 1);
}

lostpeer()
{
	extern FILE *cout;
	extern int data;

	if (connected) {
		if (cout != NULL) {
			(void) shutdown(fileno(cout), 1+1);
			(void) fclose(cout);
			cout = NULL;
		}
		if (data >= 0) {
			(void) shutdown(data, 1+1);
			(void) close(data);
			data = -1;
		}
		connected = 0;
	}
	pswitch(1);
	if (connected) {
		if (cout != NULL) {
			(void) shutdown(fileno(cout), 1+1);
			(void) fclose(cout);
			cout = NULL;
		}
		connected = 0;
	}
	proxflag = 0;
	pswitch(0);
}

/*char *
tail(filename)
	char *filename;
{
	register char *s;
	
	while (*filename) {
		s = rindex(filename, '/');
		if (s == NULL)
			break;
		if (s[1])
			return (s + 1);
		*s = '\0';
	}
	return (filename);
}
*/
/*
 * Command parser.
 */
cmdscanner(top)
	int top;
{
	register struct cmd *c;
	struct cmd *getcmd();
	int help();

	if (!top)
		(void) putchar('\n');
	for (;;) {
		if (fromatty) {
			printf(MSGSTR(MAIN_FTP, "ftp> ")); /*MSG*/
			(void) fflush(stdout);
		}
		if (gets(line) == 0) {
			if (feof(stdin) || ferror(stdin))
				quit();
			break;
		}
		if (line[0] == 0)
			break;
		makeargv();
		if (margc == 0) {
			continue;
		}
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n")); /*MSG*/
			continue;
		}
		if (c == 0) {
			printf(MSGSTR(INVALID, "?Invalid command\n")); /*MSG*/
			continue;
		}
		if (c->c_conn && !connected) {
			printf (MSGSTR(NOT_CONN, "Not connected.\n")); /*MSG*/
			continue;
		}
		(*c->c_handler)(margc, margv);
		if (bell && c->c_bell)
/* Temporary change to get around compiler problem. mvs. */
/*			(void) putchar(CTRL(g));  */
			(void) putchar('g'&037); 
		if (c->c_handler != help)
			break;
	}
	(void) signal(SIGINT, intr);
	(void) signal(SIGPIPE, lostpeer);
}

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->c_name; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */

int slrflag;

makeargv()
{
	char **argp;
	char *slurpstring();

	margc = 0;
	argp = margv;
	stringbase = line;		/* scan from first of buffer */
	argbase = argbuf;		/* store from first of buffer */
	slrflag = 0;
	while (*argp++ = slurpstring())
		margc++;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
char *
slurpstring()
{
	int got_one = 0;
	register char *sb = stringbase;
	register char *ap = argbase;
	char *tmp = argbase;		/* will return this if token found */

	if (*sb == '!' || *sb == '$') {	/* recognize ! as a token for shell */
		switch (slrflag) {	/* and $ as token for macro invoke */
			case 0:
				slrflag++;
				stringbase++;
				return ((*sb == '!') ? "!" : "$");
			case 1:
				slrflag++;
				altarg = stringbase;
				break;
			default:
				break;
		}
	}

S0:
	switch (*sb) {

	case '\0':
		goto OUT;

	case ' ':
	case '\t':
		sb++; goto S0;

	default:
		switch (slrflag) {
			case 0:
				slrflag++;
				break;
			case 1:
				slrflag++;
				altarg = sb;
				break;
			default:
				break;
		}
		goto S1;
	}

S1:
	switch (*sb) {

	case ' ':
	case '\t':
	case '\0':
		goto OUT;	/* end of token */

	case '\\':
		sb++; goto S2;	/* slurp next character */

	case '"':
		sb++; goto S3;	/* slurp quoted string */

	default:
		*ap++ = *sb++;	/* add character to token */
		got_one = 1;
		goto S1;
	}

S2:
	switch (*sb) {

	case '\0':
		goto OUT;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S1;
	}

S3:
	switch (*sb) {

	case '\0':
		goto OUT;

	case '"':
		sb++; goto S1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S3;
	}

OUT:
	if (got_one)
		*ap++ = '\0';
	argbase = ap;			/* update storage pointer */
	stringbase = sb;		/* update scan pointer */
	if (got_one) {
		return(tmp);
	}
	switch (slrflag) {
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			altarg = (char *) 0;
			break;
		default:
			break;
	}
	return((char *)0);
}

#define HELPINDENT (sizeof ("directory"))

/*
 * Help command.
 * Call each command handler with argc == 0 and argv[0] == name.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;

	if (argc == 1) {
		register int i, j, w, k;
		int columns, width = 0, lines;
		extern int NCMDS;

		printf(MSGSTR(HELP_CMD, "Commands may be abbreviated.  Commands are:\n\n")); /*MSG*/
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if (c->c_name && (!proxy || c->c_proxy)) {
					printf("%s", c->c_name);
				}
				else if (c->c_name) {
					for (k=0; k < strlen(c->c_name); k++) {
						(void) putchar(' ');
					}
				}
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					(void) putchar('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf(MSGSTR(AMB_HELP, "?Ambiguous help command %s\n"), arg); /*MSG*/
		else if (c == (struct cmd *)0)
			printf(MSGSTR(HELP_INVALID, "?Invalid help command %s\n"), arg); /*MSG*/
		else
			printf(MSGSTR(c->help_id, "%-*s\t%s\n"), HELPINDENT,
				c->c_name, c->c_help); /*MSG*/
	}
}

/*
 * Call routine with argc, argv set from args (terminated by 0).
 */
/*VARARGS1*/
call(routine, args)
	int (*routine)();
	int args;
{
	register int *argp;
	register int argc;

	for (argc = 0, argp = &args; *argp++ != 0; argc++)
		;
	(*routine)(argc, &args);
}
