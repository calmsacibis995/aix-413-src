static char sccsid[] = "@(#)74      1.25.2.2  src/tcpip/usr/bin/ftp/cmds.c, tcpfilexfr, tcpip411, GOLD410 3/25/94 18:18:08";
/* 
 * COMPONENT_NAME: TCPIP cmds.c
 * 
 * FUNCTIONS: MSGSTR, account, cd, cdup, confirm, 
 *            delete, disconnect, domap, doproxy, dotrans, fatal, 
 *            get, globulize, lcd, ls, mabort, macdef, makedir, 
 *            mdelete, mget, mls, modtime, mput, onoff, proxabort, 
 *            put, pwd, quit, quote, remglob, removedir, renamefile, 
 *            rmthelp, rmtstatus, setascii, setbell, setbinary, 
 *            setcase, setcr, setdebug, setebcdic, setform, setglob, 
 *            sethash, setmode, setnmap, setntrans, setpeer, setport, 
 *            setprompt, setrunique, setstruct, setsunique, settenex, 
 *            settrace, settype, setverbose, shell, sizecmd, status, 
 *            syst, user, gethdir, copyloc
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
 * FTP User Program -- Command Routines.
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
 *	site args		SITE args (up to 4 args)
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
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/ftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <ctype.h>
#include <pwd.h>
#include <glob.h>
#include <sys/wait.h>

#ifdef _CSECURITY
#include "tcpip_audit.h"
#endif _CSECURITY

#include <stdlib.h>
#include <nl_types.h>
#include "ftp_msg.h";
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTP,n,s) 

char *home;
extern	short gflag;
extern	char *remglob();
extern	char *getenv();
extern	char *index();
extern	char *rindex();
extern  char reply_string[];

char *mname;
jmp_buf jabort;
char *dotrans(), *domap();

char    tempstr[MAXPATHLEN];
char    tempuser[MAXPATHLEN];

#ifdef _CSECURITY
struct hostent audit_host;
struct hostent *audit_hostent = &audit_host;
extern char *audit_tail[];
extern uid_t saved_uid, effective_uid;
#endif _CSECURITY


/*
 * Connect to peer server and
 * auto-login, if possible.
 */
setpeer(argc, argv)
	int argc;
	char *argv[];
{
	struct hostent *hp = 0;
	char *host, *hookup();
	int port, interact_proxy = 0; /* Flag for interactive proxy command */
	static unsigned long inet_host;  /* to store inet addr */
	static char *cp;  /* points to inet_host */

	if (connected) {
    	    printf(MSGSTR(ALREADY_CONN, "Already connected to %s, use close first.\n"), /*MSG*/
			hostname);
		code = -1;
		return;
	}
	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(TO_PROMPT, "(to) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
		if (proxy) 	/* So we have interactive proxy command */
			interact_proxy = 1;
		
	}
	if (argc > 3) {
    	 printf(MSGSTR(USAGE, "usage: %s host-name [port]\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	port = sp->s_port;
	if (!interact_proxy) {
		if (argc > 2) {
		   port = atoi(argv[2]);
		   if (port <= 0) {
      			printf(MSGSTR(BAD_PORT, "%s: bad port number-- %s\n"), argv[1], argv[2]); /*MSG*/
    	 		printf(MSGSTR(USAGE, "usage: %s host-name [port]\n"), argv[0]); /*MSG*/
			code = -1;
			return;
		   }
		   port = htons(port);
		}
		host = hookup(argv[1], port);
	} else { 	     /* we have interactive proxy so proxy is argv[0] */
		if (argc > 3) { /* all other args are shifted right one place */
		   port = atoi(argv[3]);
		   if (port <= 0) {
      			printf(MSGSTR(BAD_PORT, "%s: bad port number-- %s\n"), argv[1], argv[3]); /*MSG*/
    	 		printf(MSGSTR(USAGE, "usage: %s host-name [port]\n"), argv[1]); /*MSG*/
			code = -1;
			return;
		   }
		   port = htons(port);
		}
		host = hookup(argv[2], port);
	}
	if (host) {
#ifdef _CSECURITY
	  if (!isinet_addr(host)) {
	  	hp = gethostbyname(host);
		if (hp == 0) {
     			fprintf(stderr, MSGSTR(GETHOSTBYNAME,
			    "gethostbyname(%s) failed\n"), host);
			perror("");
			exit(1);
		}
		bcopy ((char *) hp, (char *)audit_hostent,
			sizeof (struct hostent));
	  } else {  /* host is in inet addr format */
		/* NOTE that we only set the h_addr member of audit_hostent */
	   	if ((inet_host = inet_addr(host)) != -1) {
			cp = (char *) &inet_host;
			audit_hostent->h_addr_list = &cp;
		}
	  }
	  (void) setuid(saved_uid);
	  CONNECTION_WRITE(make_in_addr(audit_hostent->h_addr),"ftp/tcp",
		"open", "", 0);
	  (void) setuid(effective_uid);
#endif _CSECURITY
		connected = 1;
		if (autologin)
			(void) login(argv[1]);
	}
}

/*
 * Send the SITE command to the server with up to 3 args.
 */
site(argc, argv)
	char *argv[];
{
	switch(argc) {
	case 1:
		(void) command("SITE");
		break;
	case 2:
		(void) command("SITE %s", argv[1]);
		break;
	case 3:
		(void) command("SITE %s %s", argv[1], argv[2]);
		break;
	case 4:
		(void) command("SITE %s %s %s", argv[1], argv[2], argv[3]);
		break;
	}
}

/*
 * Send the SMNT command to the server.
 */
mount(argc, argv)
	char *argv[];
{
	(void) command("SMNT %s", argv[1]);
}

/*
 * Send the REIN command to the server.
 */
reinit(argc, argv)
	char *argv[];
{
	(void) command("REIN");
}

restart(argc, argv)
	char *argv[];
{
	if (mode != MODE_B) {
    		printf(MSGSTR(RESTART_STATE,
		"usage: restart valid only in block mode\n")); /*MSG*/
		code = -1;
		return;
	}

	if (strcmp(argv[1], "get") == 0) {

	    rest_flag = 1;
	    get(argc-1, argv+1);
	    rest_flag = 0;
	}
	else if ((strcmp(argv[1], "append") == 0) ||
	    (strcmp(argv[1], "put") == 0)) {

	    rest_flag = 1;
	    put(argc-1, argv+1);
	    rest_flag = 0;
	}
	else {
    	    printf(MSGSTR(RESTART_USAGE,
	    "usage: restart [get | put | append] local-file remote-file\n")); /*MSG*/
	    code = -1;
	    return;
	}
}

struct	types {
	char	*t_name;
	char	*t_mode;
	int	t_type;
	char	*t_arg;
} types[] = {
	{ "ascii",	"A",	TYPE_A,	0 },
	{ "binary",	"I",	TYPE_I,	0 },
	{ "image",	"I",	TYPE_I,	0 },
	{ "ebcdic",	"E",	TYPE_E,	0 },
	{ "local",	"L",	TYPE_L,	bytename },
	{ "tenex",	"L",	TYPE_L,	bytename },
	0
};

/*
 * Set transfer type.
 */
settype(argc, argv)
	char *argv[];
{
	register struct types *p;
	int comret;

	if (argc > 3) {
		char *sep;

    		printf(MSGSTR(SETTYPE_USAGE, "usage: %s ["), argv[0]); /*MSG*/
		sep = " ";
		for (p = types; p->t_name; p++) {
			printf("%s%s", sep, p->t_name);
			if (*sep == ' ')
				sep = " | ";
		}
		printf(" ]\n");
		code = -1;
		return;
	}
	if (argc < 2) {
		if ((type == TYPE_L) && (strcmp(typename, "tenex")))
    			printf(MSGSTR(TYPE_LOCAL, "Using %s %s type to transfer files.\n"), typename, bytename); /*MSG*/
		else
    			printf(MSGSTR(TYPE, "Using %s type to transfer files.\n"), typename); /*MSG*/
		code = 0;
		return;
	}
	for (p = types; p->t_name; p++)
		if (strcmp(argv[1], p->t_name) == 0)
			break;
	if (p->t_name == 0) {
    		printf(MSGSTR(UNKNOWN_TYPE, "%s: unknown type\n"), argv[1]); /*MSG*/
		code = -1;
		return;
	}
	if (strcmp(argv[1], "tenex") == 0)
		comret = command ("TYPE L 8");
	else if ((p->t_arg != NULL) && (*(p->t_arg) != '\0'))
		comret = command ("TYPE %s %s", p->t_mode, argv[2]);
	else if ((p->t_type == TYPE_A) || (p->t_type == TYPE_E))
		comret = command ("TYPE %s %s", p->t_mode, formcode);
	else
		comret = command("TYPE %s", p->t_mode);
	if (comret == COMPLETE) {
		(void) strcpy(typecode, p->t_mode);
		(void) strcpy(typename, p->t_name);
		type = p->t_type;
		if (strcmp(argv[1], "tenex") == 0)
			(void) strcpy(p->t_arg, "8");
		else if ((p->t_arg != NULL) && (*(p->t_arg) != '\0'))
			(void) strcpy(p->t_arg, argv[2]);
	}
}

/*
 * Set binary transfer type.
 */
/*VARARGS*/
setbinary()
{

	call(settype, "type", "binary", 0);
}

/*
 * Set ascii transfer type.
 */
/*VARARGS*/
setascii()
{

	call(settype, "type", "ascii", 0);
}

/*
 * Set local transfer type.
 */
/*VARARGS*/
setlocal(argc, argv)
	char *argv[];
{

	if (argc != 2) {
    		printf(MSGSTR(LOCAL_USAGE, "usage: local m\n")); /*MSG*/
		code = -1;
		return;
	}
	call(settype, "type", "local", argv[1], 0);
}

/*
 * Set tenex transfer type.
 */
/*VARARGS*/
settenex()
{

	call(settype, "type", "tenex", 0);
}

/*
 * Set ebcdic transfer type.
 */
/*VARARGS*/
setebcdic()
{

	call(settype, "type", "ebcdic", 0);
}

struct	modes {
	char	*m_name;
	char	*m_mode;
	int	m_type;
	char	*m_arg;
} modes[] = {
	{ "stream",	"S",	MODE_S,	0 },
	{ "block",	"B",	MODE_B,	0 },
	0
};

/*
 * Set file transfer mode.
 */
/*ARGSUSED*/
setmode(argc, argv)
	char *argv[];
{
	register struct modes *p;
	int comret;

	if (argc > 2) {
		char *sep;

    		printf(MSGSTR(SETMODE_USAGE, "usage: %s ["), argv[0]); /*MSG*/
		sep = " ";
		for (p = modes; p->m_name; p++) {
			printf("%s%s", sep, p->m_name);
			if (*sep == ' ')
				sep = " | ";
		}
		printf(" ]\n");
		code = -1;
		return;
	}
	if (argc < 2) {
    		printf(MSGSTR(MODE, "Using %s mode to transfer files.\n"), modename); /*MSG*/
		code = 0;
		return;
	}
	for (p = modes; p->m_name; p++)
		if (strcmp(argv[1], p->m_name) == 0)
			break;
	if (p->m_name == 0) {
    		printf(MSGSTR(UNKNOWN_MODE, "%s: unknown mode\n"), argv[1]); /*MSG*/
		code = -1;
		return;
	}
        if ((p->m_arg != NULL) && (*(p->m_arg) != '\0'))
                comret = command ("MODE %s %s", p->m_mode, p->m_arg);
	else
		comret = command("MODE %s", p->m_mode);
	if (comret == COMPLETE) {
		(void) strcpy(modename, p->m_name);
		mode = p->m_type;
	}

}

/*
 * Set stream transfer mode.
 */
/*VARARGS*/
setstream()
{

	call(setmode, "mode", "stream", 0);
}

/*
 * Set block transfer mode.
 */
/*VARARGS*/
setblock()
{

	call(setmode, "mode", "block", 0);
}

struct	forms {
	char	*f_name;
	char	*f_mode;
	int	f_type;
	char	*f_arg;
} forms[] = {
	{ "non-print",		"N",	FORM_N,	0 },
	{ "telnet",		"T",	FORM_T,	0 },
	{ "carriage-control",	"C",	FORM_C,	0 },
	0
};

/*
 * Set file transfer format.
 */
/*ARGSUSED*/
setform(argc, argv)
	char *argv[];
{
	register struct forms *p;
	int comret;

	if (argc > 2) {
		char *sep;

    		printf(MSGSTR(SETFORM_USAGE, "usage: %s ["), argv[0]); /*MSG*/
		sep = " ";
		for (p = forms; p->f_name; p++) {
			printf("%s%s", sep, p->f_name);
			if (*sep == ' ')
				sep = " | ";
		}
		printf(" ]\n");
		code = -1;
		return;
	}
	if (argc < 2) {
    		printf(MSGSTR(FORM, "Using %s form to transfer files.\n"), formname); /*MSG*/
		code = 0;
		return;
	}
	for (p = forms; p->f_name; p++)
		if (strcmp(argv[1], p->f_name) == 0)
			break;
	if (p->f_name == 0) {
    		printf(MSGSTR(UNKNOWN_FORM, "%s: unknown form\n"), argv[1]); /*MSG*/
		code = -1;
		return;
	}
	if ((type == TYPE_A) || (type == TYPE_E)) {
		comret = command ("TYPE %s %s", typecode, p->f_mode);
	}
	else {
		comret = COMPLETE;
	}
	if (comret == COMPLETE) {
		(void) strcpy(formcode, p->f_mode);
		(void) strcpy(formname, p->f_name);
		form = p->f_type;
	}

}

/*
 * Set non-print transfer form.
 */
/*VARARGS*/
setnon_print()
{

	call(setform, "form", "non-print", 0);
}

/*
 * Set telnet transfer form.
 */
/*VARARGS*/
settelnet()
{

	call(setform, "form", "telnet", 0);
}

/*
 * Set carriage-control transfer form.
 */
/*VARARGS*/
setcar_ctl()
{

	call(setform, "form", "carriage-control", 0);
}

struct	strus {
	char	*stru_name;
	char	*stru_mode;
	int	stru_type;
	char	*stru_arg;
} strus[] = {
	{ "file",	"F",	STRU_F,	0 },
	{ "record",	"R",	STRU_R,	0 },
	0
};

/*
 * Set file transfer structure.
 */
/*ARGSUSED*/
setstruct(argc, argv)
	char *argv[];
{
	register struct strus *p;
	int comret;

	if (argc > 2) {
		char *sep;

    		printf(MSGSTR(SETSTRUCT_USAGE, "usage: %s ["), argv[0]); /*MSG*/
		sep = " ";
		for (p = strus; p->stru_name; p++) {
			printf("%s%s", sep, p->stru_name);
			if (*sep == ' ')
				sep = " | ";
		}
		printf(" ]\n");
		code = -1;
		return;
	}
	if (argc < 2) {
    		printf(MSGSTR(STRUCT, "Using %s structure to transfer files.\n"), structname); /*MSG*/
		code = 0;
		return;
	}
	for (p = strus; p->stru_name; p++)
		if (strcmp(argv[1], p->stru_name) == 0)
			break;
	if (p->stru_name == 0) {
    		printf(MSGSTR(UNKNOWN_STRUCT, "%s: unknown structure\n"), argv[1]); /*MSG*/
		code = -1;
		return;
	}
	if ((p->stru_arg != NULL) && (*(p->stru_arg) != '\0'))
		comret = command ("STRU %s %s", p->stru_mode, p->stru_arg);
	else
		comret = command("STRU %s", p->stru_mode);
	if (comret == COMPLETE) {
		(void) strcpy(structname, p->stru_name);
		stru = p->stru_type;
	}

}

/*
 * Set record transfer structure.
 */
/*VARARGS*/
setrecord()
{

	call(setstruct, "struct", "record", 0);
}

/*
 * Set file transfer structure.
 */
/*VARARGS*/
setfile()
{

	call(setstruct, "struct", "file", 0);
}

/*
 * Send a single file.
 */
put(argc, argv)
	int argc;
	char *argv[];
{
	char *cmd;
	int loc = 0;
	char *oldargv1;

	if (argc == 2) {
		argc++;
		argv[2] = argv[1];
		loc++;
	}
	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(LOCAL_FILE, "(local-file) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
usage:
    		printf(MSGSTR(PUT_USAGE, "usage:%s local-file remote-file\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (argc < 3) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOTE_FILE, "(remote-file) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}

	if (argc < 3) 
		goto usage;
	oldargv1 = argv[1];
	if (!globulize(&argv[1])) {
		code = -1;
		return;
	}
	/*
	 * If "globulize" modifies argv[1], and argv[2] is a copy of
	 * the old argv[1], make it a copy of the new argv[1].
	 */
	if (argv[1] != oldargv1 && argv[2] == oldargv1) {
		argv[2] = argv[1];
	}
	cmd = (argv[0][0] == 'a') ? "APPE" : ((sunique) ? "STOU" : "STOR");
	if (loc && ntflag) {
		argv[2] = dotrans(argv[2]);
	}
	if (loc && mapflag) {
		argv[2] = domap(argv[2]);
	}

	sendrequest(cmd, argv[1], argv[2]);
}

/*
 * Send multiple files.
 */
mput(argc, argv)
	char *argv[];
{
	register int i, j;
	int ointer, (*oldintr)(), mabort();
	extern jmp_buf jabort;
	char *tp;

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(LOCAL_FILES, "(local-files) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(MPUT_MESSAGE, "usage:%s local-files\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	mname = argv[0];
	mflag = 1;
	oldintr = signal(SIGINT, mabort);
	(void) setjmp(jabort);
	if (proxy) {
		char *cp, *tp2, tmpbuf[MAXPATHLEN];

		while ((cp = remglob(argv,0)) != NULL) {
			if (*cp == 0) {
				mflag = 0;
				continue;
			}
			if (mflag && confirm(argv[0], cp)) {
				tp = cp;
				if (mcase) {
				   if (mb_cur_max > 1) {   /* multibyte code */
					int mbcount, count, NEXT = FALSE;
					wchar_t wc;
					
					while (*tp) {
					  mbcount = mbtowc(&wc, tp, mb_cur_max);
					  if (mbcount > 0) { 
					     if (!iswlower(wc))
						tp += mbcount;
					  }
					  else {
					     fprintf(stderr, MSGSTR(MBTOWC, "error converting to widechar (%s)\n"), tp);
					    NEXT = TRUE;
					    break;
					  }
					} /* end of while */
					  
		/* if NEXT is TRUE that means we have encountered an error */
		/* in converting to widechar, so do not process this file  */
		/* and continue to next file in "mput"			   */
			    	        if (NEXT)
						continue;

					if (!*tp) {
					    tp = cp;
					    tp2 = tmpbuf;
					    do {
						mbcount = mbtowc(&wc, tp, mb_cur_max);
						if (mbcount < 0) {
						    fprintf(stderr, MSGSTR(MBTOWC, "error converting to widechar (%s)\n"), tp);
						    NEXT = TRUE;
						    break;
						}
						if (mbcount > 0) 
						    wc = towlower(wc);
						count = wctomb(tp2, wc);
						if (count < 0) {
						   fprintf(stderr, MSGSTR(WCTOMB, "error converting widechar to multibyte (%s)\n"), tp);
						   NEXT = TRUE;
						   break;
						}
						tp += mbcount;
						tp2 += count;
					    } while (mbcount > 0);
		/* if NEXT is TRUE that means we have encountered an error */
		/* in converting to widechar, so do not process this file  */
		/* and continue to next file in "mput"			   */
					   if (NEXT)
						continue;
   					}
				   }			
				   else {
					while (*tp && !islower(*tp))
					    tp++;
					  
					if (!*tp) {
					   tp = cp;
					   tp2 = tmpbuf;
					   while( (*tp2 = *tp) != NULL) {
						if (isupper(*tp2))
						    *tp2 = tolower(*tp2);
						tp++;
						tp2++;
					   }
					}
				   }
				   tp = tmpbuf;
				}
				if (ntflag) {
					tp = dotrans(tp);
				}
				if (mapflag) {
					tp = domap(tp);
				}
				sendrequest((sunique) ? "STOU" : "STOR", cp,
									tp);
				if (!mflag && fromatty) {
					ointer = interactive;
					interactive = 1;
         				if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"),"mput")) { /*MSG*/
						mflag++;
					}
					interactive = ointer;
				}
			}
		}
		(void) signal(SIGINT, oldintr);
		mflag = 0;
		return;
	}
	for (i = 1; i < argc; i++) {
		register char *cp, **cpp, **gargs;
		glob_t globbuf;

		if (!doglob) {
			if (mflag && confirm(argv[0], argv[i])) {
				tp = (ntflag) ? dotrans(argv[i]) : argv[i];
				tp = (mapflag) ? domap(tp) : tp;
				sendrequest((sunique) ? "STOU" : "STOR",
				            argv[i], tp);
				if (!mflag && fromatty) {
					ointer = interactive;
					interactive = 1;
         				if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"),"mput")) { /*MSG*/
						mflag++;
					}
					interactive = ointer;
				}
			}
			continue;
		}
		/*
		 * First we need to do tilde expansion since glob()
		 * in libc.a is to dumb to do it for us.
		 */
		cp = argv[i];
		if (*cp == '~') { 	/* we found a tilde */
			cp++;
			if (*cp == '\0' || *cp == '/') {
				/*
				 * we found a null,
				 * or / after the tilde.
				 */
				strcpy(tempstr, home);
				strcat(tempstr, cp);
			} else {
				/* we might of found a user after the tilde */
				for (i = 0; i < sizeof(tempuser) &&
						 *cp != '/' && *cp != '\0'; i++)
					tempuser[i] = *cp++;
				tempuser[i] = '\0';
				if (gethdir(tempuser)) {
						printf(MSGSTR(UNKNOWN_USER,
						  "Unknown user name after ~\n"));
						code = -1;
						return;
				} else {
					strcpy(tempstr, tempuser);/* user dir */
					strcat(tempstr, cp); /* rest of path */
				}
			}
		} else
			strcpy(tempstr, cp);
	if (strpbrk(tempstr, "*?[{")) { /* we have glob characters in tempstr */
		if (glob(tempstr, GLOB_QUOTE|GLOB_NOCHECK, NULL,&globbuf) != 0){
                        /* globbing failed */
			printf(MSGSTR(BAD_DIR, "Bad directory components"));
			code = -1;
			return;
                }
                if (globbuf.gl_pathc == 0) {
                        /* globbing failed */
			printf(MSGSTR(BAD_DIR, "Bad directory components"));
			code = -1;
			return;
                }
		gargs = (char **) malloc((unsigned) sizeof(gargs)
							* globbuf.gl_pathc);
                for (j = 0; j < globbuf.gl_pathc && globbuf.gl_pathv[j] != NULL;
                                                                        j++) {
			if ((gargs[j] = (char *) malloc((unsigned)
				       strlen(globbuf.gl_pathv[j])+1)) != NULL)
				strcpy(gargs[j], globbuf.gl_pathv[j]);
                }
		gargs[j] = NULL;
	} else {
		gargs = (char **) malloc((unsigned) sizeof(gargs) * 2);
		if ((gargs[0] = (char *) malloc((unsigned)
				       strlen(tempstr)+1)) != NULL)
			strcpy(gargs[0], tempstr);
		gargs[1] = NULL;
	}
		for (cpp = gargs; cpp && *cpp != NULL; cpp++) {
			if (mflag && confirm(argv[0], *cpp)) {
                                tp = (ntflag) ? dotrans(*cpp) : *cpp;
				tp = (mapflag) ? domap(tp) : tp;
				sendrequest((sunique) ? "STOU" : "STOR",
					   *cpp, tp);
				if (!mflag && fromatty) {
					ointer = interactive;
					interactive = 1;
         				if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"),"mput")) { /*MSG*/
						mflag++;
					}
					interactive = ointer;
				}
			}
		}
	}
	(void) signal(SIGINT, oldintr);
	mflag = 0;
}

/*
 * Receive one file.
 */
get(argc, argv)
	char *argv[];
{
	int loc = 0;

	if (argc == 2) {
		argc++;
		argv[2] = argv[1];
		loc++;
	}
	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOTE_FILE, "(remote-file) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
usage:
    		printf(MSGSTR(GET_USAGE, "usage: %s remote-file [ local-file ]\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (argc < 3) {
		(void) strcat(line, " ");
    		printf(MSGSTR(LOCAL_FILE, "(local-file) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 3) 
		goto usage;
	if (!globulize(&argv[2])) {
		code = -1;
		return;
	}
	if (loc && mcase) {
		char *tp = argv[1], *tp2, tmpbuf[MAXPATHLEN];

		if (mb_cur_max > 1) {		/* multibyte code */
		   int mbcount, count;
		   wchar_t wc;
		
		   while (*tp) {
			mbcount = mbtowc(&wc, tp, mb_cur_max);
			if (mbcount > 0) {
			    if (!iswlower((wint_t) wc))
				tp += mbcount;
			    }
			else {
			    fprintf(stderr, MSGSTR(MBTOWC, "error in converting to widechar (%s)\n"), tp);
			    code = -1;
			    return;
			}
		   }
		   if (!*tp) {
			tp = argv[2];
			tp2 = tmpbuf;
			do {
			    mbcount = mbtowc(&wc, tp, mb_cur_max);
			    if (mbcount < 0) {
				fprintf(stderr, MSGSTR(MBTOWC, "error converting to widechar (%s)\n"), tp);
				code = -1;
				return;
			    }
			    if (mbcount > 0)
				wc = towlower((wint_t) wc);
				count = wctomb(tp2, wc);
				if (count < 0) {
				   fprintf(stderr, MSGSTR(WCTOMB, "error converting widechar to multibyte (%s)\n"), tp);
				   code = -1;
				   return;
				}
				tp += mbcount;
				tp2 += count;
			} while (mbcount > 0);
			argv[2] = tmpbuf;
		   }
		}
		else {			/* single byte code */
		   while (*tp && !islower(*tp))		
			tp++;

		   if (!*tp) {
			tp = argv[2];
			tp2 = tmpbuf;
			while ((*tp2 = *tp) != NULL) {
			    if (isupper(*tp2))
				*tp2 = tolower(*tp2);
			    tp++;
			    tp2++;
			}
			argv[2] = tmpbuf;
		   }
		}
	}
	if (loc && ntflag) {
		argv[2] = dotrans(argv[2]);
	}
	if (loc && mapflag) {
		argv[2] = domap(argv[2]);
	}
	recvrequest("RETR", argv[2], argv[1], "w", 0);
}

mabort()
{
	int ointer;
	extern jmp_buf jabort;

	printf("\n");
	(void) fflush(stdout);
	if (mflag && fromatty) {
		ointer = interactive;
		interactive = 1;
    		if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"), mname)) { /*MSG*/
			interactive = ointer;
			longjmp(jabort,0);
		}
		interactive = ointer;
	}
	mflag = 0;
	longjmp(jabort,0);
}

/*
 * Get multiple files.
 */
mget(argc, argv)
	char *argv[];
{
	char *cp, *tp, *newtp, *tp2, tmpbuf[MAXPATHLEN];
	int ointer, (*oldintr)(), mabort();
	int dont_convert;
	extern jmp_buf jabort;

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOTE_FILES, "(remote-files) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(MGET_USAGE, "usage:%s remote-files\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	mname = argv[0];
	mflag = 1;
	oldintr = signal(SIGINT,mabort);
	(void) setjmp(jabort);
	while ((cp = remglob(argv,proxy)) != NULL) {
		if (*cp == '\0') {
			mflag = 0;
			continue;
		}
		if (mflag && confirm(argv[0], cp)) {
			(void) strcpy(tmpbuf, cp);
			tp = tmpbuf;
			/*
			 * Get the basename from the entire path. K.M. (9/21/88)
			 * And, watch out for a / at the end of the filename.
			 */
			if ((newtp = rindex(tp, '/')) != NULL) {
				++newtp;
				if (*newtp)
					tp = newtp;
			}
			dont_convert = FALSE;
			if (mcase) {
				/* 
				 * Convert it to lower case. K.M. (9/21/88)
				 */
				tp2 = tp;
				
				if (mb_cur_max > 1) {	/* multibyte code */
				   int count = 0, mbcount, NEXT = FALSE;
				   wchar_t wc;
				   char *tp3, tp3buf[MAXPATHLEN];
				   
				   while (*tp2) {
				        mbcount = mbtowc(&wc, tp2, mb_cur_max);
				 	if (mbcount > 0) {
					    if (iswlower((wint_t) wc))
						dont_convert = TRUE;
					    tp2 += mbcount;
					}
					else {
					     fprintf(stderr, MSGSTR(MBTOWC, "error converting  to widechar (%s)\n"), tp);
					     NEXT = TRUE;
					     break;
					}
				   }
			/* if NEXT = TRUE then there was an error converting */
			/* to widechar, do not process this file, but        */
			/* continue to next file   */
				   if (NEXT)
					continue;

				   tp2 = tp;
				   tp3 = tp3buf;
				   while (!dont_convert && *tp2) {
					mbcount = mbtowc(&wc, tp2, mb_cur_max);
				        if (mbcount < 0) {
					   fprintf(stderr, MSGSTR(MBTOWC, "error converting to widechar (%s)\n"), tp);
					   NEXT = TRUE;
					   break;
				        }
				        wc = towlower((wint_t) wc);
				        count = wctomb(tp3, wc);
				        if (count < 0) {
					   fprintf(stderr, MSGSTR(WCTOMB, "error converting widechar to multibyte (%s)\n"), tp);
					   NEXT = TRUE;
					   break;
				        }	
				        tp2 += mbcount;
				        tp3 += count;
				   }
			/* if NEXT = TRUE then there was an error converting */
			/* to widechar, do not process this file, but        */
			/* continue to next file   */
			       	   if (NEXT)
					continue;

				   (*tp3) = '\0';
				   tp = tp3buf;
				}
				else {		/* single byte code */
				   while (*tp2) {
					if (islower(*tp2)) 
					   dont_convert = TRUE;
					++tp2;
				   }
				   tp2 = tp;
				   while (!dont_convert && *tp2) {
				        if (isupper(*tp2))
					   *tp2 = tolower(*tp2);
				        ++tp2;
				   }
				}		
			}
			if (ntflag) {
				tp = dotrans(tp);
			}
			if (mapflag) {
				tp = domap(tp);
			}
			recvrequest("RETR", tp, cp, "w", 0);
			if (!mflag && fromatty) {
				ointer = interactive;
				interactive = 1;
        			if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"),"mget")) { /*MSG*/
					mflag++;
				}
				interactive = ointer;
			}
		}
	}
	(void) signal(SIGINT, oldintr);
	mflag = 0;
}

char *
remglob(argv,doswitch)
	char *argv[];
	int doswitch;
{
	char temp[16];
	static char buf[MAXPATHLEN];
	static FILE *ftemp = NULL;
	static char **args;
	int oldverbose, oldhash, unique_changed = 0;
	char *cp, *mode;

	if (!mflag) {
		if (!doglob) {
			args = NULL;
		}
		else {
			if (ftemp) {
				(void) fclose(ftemp);
				ftemp = NULL;
			}
		}
		return(NULL);
	}
	if (!doglob) {
		if (args == NULL)
			args = argv;
		if ((cp = *++args) == NULL)
			args = NULL;
		return (cp);
	}
	if (ftemp == NULL) {
		oldverbose = verbose, verbose = 0;
		oldhash = hash, hash = 0;
		if (doswitch) {
			pswitch(!proxy);
		}
		if (runique) {
			runique = !runique;
			unique_changed = !unique_changed;
		}
		(void) strcpy(temp, "/tmp/ftpXXXXXX");
		(void) mktemp(temp);
		for (mode = "w"; *++argv != NULL; mode = "a")
			recvrequest ("NLST", temp, *argv, mode, 1);
		if (unique_changed) {
			runique = !runique;
			unique_changed = !unique_changed;
		}
		if (doswitch) {
			pswitch(!proxy);
		}
		verbose = oldverbose; hash = oldhash;
		ftemp = fopen(temp, "r");
		(void) unlink(temp);
		if (ftemp == NULL) {
      		  printf(MSGSTR(LIST, "can't find list of remote files, oops\n")); /*MSG*/
			return (NULL);
		}
	}
	if (fgets(buf, sizeof (buf), ftemp) == NULL) {
		(void) fclose(ftemp), ftemp = NULL;
		return (NULL);
	}
	if ((cp = index(buf, '\n')) != NULL)
		*cp = '\0';
	return (buf);
}

char *
onoff(bool)
	int bool;
{

  	return (bool ? MSGSTR(ONOFF_ON, "on") : MSGSTR(ONOFF_OFF, "off")); /*MSG*/ /*MSG*/
}

/*
 * Show status.
 */
/*ARGSUSED*/
status(argc, argv)
	char *argv[];
{
	int i;

	if (connected)
    	    printf(MSGSTR(CONNECTED, "Connected to %s.\n"), hostname); /*MSG*/
	else
    	    printf(MSGSTR(NOT_CONN, "Not connected.\n")); /*MSG*/
	if (!proxy) {
		pswitch(1);
		if (connected) {
      		  printf(MSGSTR(PROXY_CONN, "Connected for proxy commands to %s.\n"), hostname); /*MSG*/
		}
		else {
      			printf(MSGSTR(PROXY_NOT_CONN, "No proxy connection.\n")); /*MSG*/
		}
		pswitch(0);
	}
		if (type == TYPE_L) {
  			printf(MSGSTR(MO_TY_L_FO_ST, "Mode: %s; Type: %s %s; Form: %s; Structure: %s\n"), 
			modename, typename, bytename, formname, structname);
		} else {
  			printf(MSGSTR(MO_TY_FO_ST, "Mode: %s; Type: %s; Form: %s; Structure: %s\n"), 
			modename, typename, formname, structname);
		}
  		printf(MSGSTR(VE_BE_PR_GL, "Verbose: %s; Bell: %s; Prompting: %s; Globbing: %s\n"),  /*MSG*/
		onoff(verbose), onoff(bell), onoff(interactive),
		onoff(doglob));
  		printf(MSGSTR(UNIQUES, "Store unique: %s; Receive unique: %s\n"),
		onoff(sunique), onoff(runique));
  		printf(MSGSTR(CASE, "Case: %s; CR stripping: %s\n"), 
		onoff(mcase), onoff(crflag));
	if (ntflag) {
    		printf(MSGSTR(NTRANS, "Ntrans: (in) %s (out) %s\n"), ntin,ntout); /*MSG*/
	}
	else {
    		printf(MSGSTR(NTRANS1, "Ntrans: off\n")); /*MSG*/
	}
	if (mapflag) {
    		printf(MSGSTR(NMAP, "Nmap: (in) %s (out) %s\n"), mapin, mapout); /*MSG*/
	}
	else {
    		printf(MSGSTR(NMAP_OFF, "Nmap: off\n")); /*MSG*/
	}
  	printf(MSGSTR(HASH_PORT, "Hash mark printing: %s; Use of PORT cmds: %s\n"), 
		onoff(hash), onoff(sendport));
	printf(MSGSTR(EXP_CMD, "Experimental commands: %s\n"), onoff(exp_cmds));
	if (macnum > 0) {
    		printf(MSGSTR(MACROS, "Macros:\n")); /*MSG*/
		for (i=0; i<macnum; i++) {
			printf("\t%s\n",macros[i].mac_name);
		}
	}
	code = 0;
}

/*
 * Set beep on cmd completed mode.
 */
/*VARARGS*/
setbell()
{

	bell = !bell;
	if (bell)
    	  printf(MSGSTR(BELL_ON, "Bell mode on.\n")); /*MSG*/
	else
    	  printf(MSGSTR(BELL_OFF, "Bell mode off.\n")); /*MSG*/
	code = bell;
}

/*
 * Turn on packet tracing.
 */
/*VARARGS*/
settrace()
{

	trace = !trace;
	if (trace)
    	  printf(MSGSTR(PR_TR_ON, "Packet tracing on.\n"));  /*MSG*/
	else
    	  printf(MSGSTR(PR_TR_OFF, "Packet tracing off.\n"));  /*MSG*/
	code = trace;
}

/*
 * Toggle hash mark printing during transfers.
 */
/*VARARGS*/
sethash()
{

	hash = !hash;
	code = hash;
	if (hash)
            printf(MSGSTR(HASH_ON, "Hash mark printing on (%d bytes/hash mark).\n"), HASHBYTES); /*MSG*/
	else
    	   printf(MSGSTR(HASH_OFF, "Hash mark printing off.\n")); /*MSG*/
}

/*
 * Toggle using experimental commands.
 */
/*VARARGS*/
setexp_cmd()
{

	exp_cmds = !exp_cmds;
	code = exp_cmds;
	if (exp_cmds)
            printf(MSGSTR(EXP_CMD_ON, "Experimental commands on.\n")); /*MSG*/
	else
    	   printf(MSGSTR(EXP_CMD_OFF, "Experimental commands off.\n")); /*MSG*/
}

/*
 * Turn on printing of server echo's.
 */
/*VARARGS*/
setverbose()
{

	verbose = !verbose;
	if (verbose)
    	    printf(MSGSTR(VERBOSE_ON, "Verbose mode on.\n")); /*MSG*/
	else
    	    printf(MSGSTR(VEBOSE_OFF, "Verbose mode off.\n")); /*MSG*/
	code = verbose;
}

/*
 * Toggle PORT cmd use before each data connection.
 */
/*VARARGS*/
setport()
{

	sendport = !sendport;
	if (sendport)
    	   printf(MSGSTR(PORT_ON, "Use of PORT cmds on.\n")); /*MSG*/
	else
    	   printf(MSGSTR(PORT_OFF, "Use of PORT cmds off.\n")); /*MSG*/
	code = sendport;
}

/*
 * Turn on interactive prompting
 * during mget, mput, and mdelete.
 */
/*VARARGS*/
setprompt()
{

	interactive = !interactive;
  if (interactive)
    printf(MSGSTR(INTRACTIVE_ON, "Interactive mode on.\n")); /*MSG*/
  else
    printf(MSGSTR(INTERACTIVE_OFF, "Interactive mode off.\n")); /*MSG*/
	code = interactive;
}

/*
 * Toggle metacharacter interpretation
 * on local file names.
 */
/*VARARGS*/
setglob()
{
	
	doglob = !doglob;
	code = doglob;
  if (doglob)
    printf(MSGSTR(GLOB_ON, "Globbing on.\n")); /*MSG*/
  else
    printf(MSGSTR(GLOB_OFF, "Globbing off.\n")); /*MSG*/
}

/*
 * Set debugging mode on/off and/or
 * set level of debugging.
 */
/*VARARGS*/
setdebug(argc, argv)
	char *argv[];
{
	int val;

	if (argc > 1) {
		val = atoi(argv[1]);
		if (val < 0) {
      			printf(MSGSTR(DEBUG_BAD, "%s: bad debugging value.\n"), argv[1]); /*MSG*/
			code = -1;
			return;
		}
	} else
		val = !debug;
	debug = val;
	if (debug)
		options |= SO_DEBUG;
	else
		options &= ~SO_DEBUG;
	if (debug)
    	    printf(MSGSTR(DEBUG_ON, "Debugging on (debug=%d).\n"), debug); /*MSG*/
	else
    	    printf(MSGSTR(DEBUG_OFF, "Debugging off (debug=%d).\n"), debug); /*MSG*/
	code = debug > 0;
}

/*
 * Set current working directory
 * on remote machine.
 */
cd(argc, argv)
	char *argv[];
{

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOT_DIR, "(remote-directory) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(CD_USAGE, "usage:%s remote-directory\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (exp_cmds)
		(void) command("XCWD %s", argv[1]);
	else
		(void) command("CWD %s", argv[1]);
}

/*
 * Set current working directory
 * on local machine.
 */
lcd(argc, argv)
	char *argv[];
{
	char buf[MAXPATHLEN];

	if (argc < 2)
		argc++, argv[1] = home;
	if (argc != 2) {
    		printf(MSGSTR(LCD_USGAE, "usage:%s local-directory\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (!globulize(&argv[1])) {
		code = -1;
		return;
	}
	if (chdir(argv[1]) < 0) {
		perror(argv[1]);
		code = -1;
		return;
	}
  	printf(MSGSTR(LOCAL_DIR, "Local directory now %s\n"), getwd(buf)); /*MSG*/
	code = 0;
}

/*
 * Delete a single file.
 */
delete(argc, argv)
	char *argv[];
{

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOTE_FILE, "(remote-file) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(DELETE_USAGE, "usage:%s remote-file\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	(void) command("DELE %s", argv[1]);
}

/*
 * Delete multiple files.
 */
mdelete(argc, argv)
	char *argv[];
{
	char *cp;
	int ointer, (*oldintr)(), mabort();
	extern jmp_buf jabort;

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOTE_FILES, "(remote-files) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(MGET_USAGE, "usage:%s remote-files\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	mname = argv[0];
	mflag = 1;
	oldintr = signal(SIGINT, mabort);
	(void) setjmp(jabort);
	while ((cp = remglob(argv,0)) != NULL) {
		if (*cp == '\0') {
			mflag = 0;
			continue;
		}
		if (mflag && confirm(argv[0], cp)) {
			(void) command("DELE %s", cp);
			if (!mflag && fromatty) {
				ointer = interactive;
				interactive = 1;
        			if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"), "mdelete")) { /*MSG*/
					mflag++;
				}
				interactive = ointer;
			}
		}
	}
	(void) signal(SIGINT, oldintr);
	mflag = 0;
}

/*
 * Rename a remote file.
 */
renamefile(argc, argv)
	char *argv[];
{

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(FROM_NAME, "(from-name) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
usage:
    		printf(MSGSTR(FROM_TO_NAME, "%s from-name to-name\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (argc < 3) {
		(void) strcat(line, " ");
    		printf(MSGSTR(TO_NAME, "(to-name) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 3) 
		goto usage;
	if (command("RNFR %s", argv[1]) == CONTINUE)
		(void) command("RNTO %s", argv[2]);
}

/*
 * Get a directory listing
 * of remote files.
 */
ls(argc, argv)
	char *argv[];
{
	char *cmd;
  
	if (argc < 2)
		argc++, argv[1] = NULL;
	if (argc < 3)
		argc++, argv[2] = "-";
	if (argc > 3) {
    		printf(MSGSTR(LS_USAGE, "usage: %s remote-directory local-file\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	cmd = argv[0][0] == 'l' ? "NLST" : "LIST";
	if (strcmp(argv[2], "-") && !globulize(&argv[2])) {
		code = -1;
		return;
	}
	if (strcmp(argv[2], "-") && *argv[2] != '|')
    		if (!globulize(&argv[2]) || !confirm(MSGSTR(CONFIRM_OUTPUT, "output to local-file:"), argv[2])) { /*MSG*/
			code = -1;
			return;
	}
	recvrequest(cmd, argv[2], argv[1], "w", 0);
}

/*
 * Get a directory listing
 * of multiple remote files.
 */
mls(argc, argv)
	char *argv[];
{
	char *cmd, mode[1], *dest;
	int ointer, i, (*oldintr)(), mabort();
	extern jmp_buf jabort;

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(REMOTE_FILES, "(remote-files) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 3) {
		(void) strcat(line, " ");
    		printf(MSGSTR(LOCAL_FILE, "(local-file) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 3) {
    		printf(MSGSTR(MLS_USAGE, "usage:%s remote-files local-file\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	dest = argv[argc - 1];
	argv[argc - 1] = NULL;
	if (strcmp(dest, "-") && *dest != '|')
    	  if (!globulize(&dest) || !confirm(MSGSTR(CONFIRM_OUTPUT, "output to local-file:"), dest)) { /*MSG*/
			code = -1;
			return;
	}
	cmd = argv[0][1] == 'l' ? "NLST" : "LIST";
	mname = argv[0];
	mflag = 1;
	oldintr = signal(SIGINT, mabort);
	(void) setjmp(jabort);
	for (i = 1; mflag && i < argc-1; ++i) {
		*mode = (i == 1) ? 'w' : 'a';
		recvrequest(cmd, dest, argv[i], mode, 1);
		if (!mflag && fromatty) {
			ointer = interactive;
			interactive = 1;
      			if (confirm(MSGSTR(CONTINUE_WITH, "Continue with"), argv[0])) { /*MSG*/
				mflag ++;
			}
			interactive = ointer;
		}
	}
	(void) signal(SIGINT, oldintr);
	mflag = 0;
}

/*
 * Do a shell escape
 */
/*ARGSUSED*/
shell(argc, argv)
	char *argv[];
{
	int pid, (*old1)(), (*old2)();
	char shellnam[40], *shell, *namep; 
	union wait status;

	old1 = signal (SIGINT, SIG_IGN);
	old2 = signal (SIGQUIT, SIG_IGN);
	if ((pid = fork()) == 0) {
		for (pid = 3; pid < 20; pid++)
			(void) close(pid);
		(void) signal(SIGINT, SIG_DFL);
		(void) signal(SIGQUIT, SIG_DFL);
		shell = getenv("SHELL");
		if (shell == NULL)
			shell = "/bin/sh";
		namep = rindex(shell,'/');
		if (namep == NULL)
			namep = shell;
		(void) strcpy(shellnam, "-");  /* SH */
		(void) strcat(shellnam, ++namep);
		if (strcmp(namep, "sh") != 0)
			shellnam[0] = '+';
		if (debug) {
			printf ("%s\n", shell);
			(void) fflush (stdout);
		}
		if (argc > 1) {
			execl(shell,shellnam,"-c",altarg,(char *)0);
		}
		else {
			execl(shell,shellnam,(char *)0);
		}
		perror(shell);
		code = -1;
		exit(1);
		}
	if (pid > 0)
		while (wait(&status) != pid)
			;
	(void) signal(SIGINT, old1);
	(void) signal(SIGQUIT, old2);
	if (pid == -1) {
    		perror(MSGSTR(TRYAGAIN, "Try again later"));  /*MSG*/
		code = -1;
	}
	else {
		code = 0;
	}
	return (0);
}

/*
 * Send new user information (re-login)
 */
user(argc, argv)
	int argc;
	char **argv;
{
	char acct[80], *getpass();
	int n, aflag = 0;

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(USERNAME, "(username) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc > 4) {
    		printf(MSGSTR(USER_USAGE, "usage: %s username [password] [account]\n"), argv[0]); /*MSG*/
		code = -1;
		return (0);
	}
	n = command("USER %s", argv[1]);
	if (n == CONTINUE) {
		if (argc < 3 )
      			argv[2] = getpass(MSGSTR(PASSWORD, "Password: ")), argc++; /*MSG*/
		(void) signal(SIGINT,SIG_IGN);
		n = command("PASS %s", argv[2]);
	}
	if (n == CONTINUE) {
		if (argc < 4) {
      			printf(MSGSTR(ACCNT, "Account: ")); 
			(void) fflush(stdout); /*MSG*/
			(void) fgets(acct, sizeof(acct) - 1, stdin);
			acct[strlen(acct) - 1] = '\0';
			argv[3] = acct; argc++;
		}
		n = command("ACCT %s", argv[3]);
		aflag++;
	}
	if (n != COMPLETE) {
		fprintf(stdout, MSGSTR(LOGIN_FAILED, "Login failed.\n"));/*MSG*/
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		NET_ACCESS_WRITE( make_in_addr(audit_hostent->h_addr),
		                  "ftp/tcp",
		                  argv[1],
		                  MSGSTR(LOGIN_FAILED, "Login failed.\n"),
		                  -1);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return (0);
	}

#ifdef _CSECURITY
	(void) setuid(saved_uid);
	NET_ACCESS_WRITE( make_in_addr(audit_hostent->h_addr),
	                  "ftp/tcp",
	                  argv[1],
	                  "Login accepted",
	                  0);
	(void) setuid(effective_uid);
#endif _CSECURITY

	if (!aflag && argc == 4) {
		(void) command("ACCT %s", argv[3]);
	}
	return (1);
}

/*
 * Print working directory.
 */
/*VARARGS*/
pwd()
{
	int oldverbose = verbose;

	verbose = 1;
	if (exp_cmds)
		(void) command("XPWD");
	else
		(void) command("PWD");
	verbose = oldverbose;
}

/*
 * Make a directory.
 */
makedir(argc, argv)
	char *argv[];
{

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(DIR_NAME, "(directory-name) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(MAKEDIR_USAGE, "usage: %s directory-name\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (exp_cmds)
		(void) command("XMKD %s", argv[1]);
	else
		(void) command("MKD %s", argv[1]);
}

/*
 * Remove a directory.
 */
removedir(argc, argv)
	char *argv[];
{

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(DIR_NAME, "(directory-name) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(MAKEDIR_USAGE, "usage: %s directory-name\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (exp_cmds)
		(void) command("XRMD %s", argv[1]);
	else
		(void) command("RMD %s", argv[1]);
}

/*
 * Send a line, verbatim, to the remote machine.
 */
quote(argc, argv)
	char *argv[];
{
	int i;
	char buf[BUFSIZ];

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(COMMAND_LINE, "(command line to send) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(QUOTE_USAGE, "usage: %s line-to-send\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	(void) strcpy(buf, argv[1]);
	for (i = 2; i < argc; i++) {
		(void) strcat(buf, " ");
		(void) strcat(buf, argv[i]);
	}
	if (command(buf) == PRELIM) {
		while (getreply(0) == PRELIM);
	}
}

/*
 * Ask the other side for help.
 */
rmthelp(argc, argv)
	char *argv[];
{
	int oldverbose = verbose;

	verbose = 1;
	(void) command(argc == 1 ? "HELP" : "HELP %s", argv[1]);
	verbose = oldverbose;
}

/*
 * Terminate session and exit.
 */
/*VARARGS*/
quit()
{

	if (connected)
		disconnect();
	pswitch(1);
	if (connected) {
		disconnect();
	}
	exit(0);
}

/*
 * Terminate session, but don't exit.
 */
disconnect()
{
	extern FILE *cout;
	extern int data;

	if (!connected)
		return;
	(void) command("QUIT");
	if (cout) {
		(void) fclose(cout);
	}
	cout = NULL;
	connected = 0;
	data = -1;
	if (!proxy) {
		macnum = 0;
	}
#ifdef _CSECURITY
	(void) setuid(saved_uid);
	CONNECTION_WRITE(make_in_addr(audit_hostent->h_addr),"ftp/tcp",
			"close/disconnect", "", 0);
	(void) setuid(effective_uid);
#endif _CSECURITY
}

confirm(cmd, file)
	char *cmd, *file;
{
	char line[BUFSIZ];
	int i;

	if (!interactive)
		return (1);

		printf("%s %s? ", cmd, file);
		(void) fflush(stdout);
		(void) gets(line);
		i = rpmatch(line);
	return (i);
}

fatal(msg)
	char *msg;
{

	fprintf(stderr, MSGSTR(FTP, "ftp: %s\n"), msg);
	exit(1);
}

/*
 * Glob a local file name specification with
 * the expectation of a single return value.
 * Can't control multiple values being expanded
 * from the expression, we return only the first.
 */
globulize(cpp)
	char **cpp;
{
        glob_t globbuf;
	char *cp;
	int i;

	if (!doglob)
		return (1);
	/*
	 * First we need to do tilde expansion since glob()
	 * in libc.a is to dumb to do it for us.
	 */
	cp = *cpp;
	if (*cp == '~') { 	/* we found a tilde */
		cp++;
		if (*cp == (char) NULL) {
			/*
			 * we found a null after the tilde.
			 */
			strcpy(tempstr, home);
			*cpp = tempstr;
			return(1);
		} else if (*cp == '/') {
			/*
			 * we found a / after the tilde.
			 */
			strcpy(tempstr, home);
			strcat(tempstr, cp);
			cp++;
			if (*cp == (char) NULL) {
				*cpp = tempstr;
				return(1);
			}
		} else {
			/* we might of found a user after the tilde */
			for (i = 0; i < sizeof(tempuser) &&
					 *cp != '/' && *cp != '\0'; i++)
				tempuser[i] = *cp++;
			tempuser[i] = '\0';
			if (gethdir(tempuser)) {
					printf(MSGSTR(UNKNOWN_USER,
					  "Unknown user name after ~\n"));
					code = -1;
					return (0);
			} else {
				strcpy(tempstr, tempuser);/* user dir */
				strcat(tempstr, cp); /* rest of path */
			}
		}
	} else
		strcpy(tempstr, cp);
	if (strpbrk(tempstr, "*?[{")) { /* we have glob characters in tempstr */
		if (glob(tempstr,GLOB_QUOTE|GLOB_NOCHECK, NULL,&globbuf) != 0){
			/* globbing failed */
			return (0);
		}
		if (globbuf.gl_pathc == 0) {
			/* globbing failed */
			return (0);
		}
		if ((*cpp = (char *) malloc((unsigned)
				      strlen(globbuf.gl_pathv[0])+1)) != NULL)
				strcpy(*cpp, globbuf.gl_pathv[0]);
	} else
		*cpp = tempstr;
	return (1);
}

account(argc,argv)

	int argc;
	char **argv;
{
	char acct[50], *getpass(), *ap;

	if (argc > 1) {
		++argv;
		--argc;
		(void) strncpy(acct,*argv,49);
		acct[49] = '\0';
		while (argc > 1) {
			--argc;
			++argv;
			(void) strncat(acct,*argv, 49-strlen(acct));
		}
		ap = acct;
	}
	else {
    		ap = getpass(MSGSTR(ACCNT, "Account: ")); /*MSG*/
	}
	(void) command("ACCT %s", ap);
}

jmp_buf abortprox;

proxabort()
{
	extern int proxy;

	if (!proxy) {
		pswitch(1);
	}
	if (connected) {
		proxflag = 1;
	}
	else {
		proxflag = 0;
	}
	pswitch(0);
	longjmp(abortprox,1);
}

doproxy(argc,argv)
	int argc;
	char *argv[];
{
	int (*oldintr)(), proxabort();
	register struct cmd *c;
	struct cmd *getcmd();
	extern struct cmd cmdtab[];
	extern jmp_buf abortprox;

	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(COMMAND, "(command) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
    		printf(MSGSTR(DOPROXY_USAGE, "usage:%s command\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	c = getcmd(argv[1]);
	if (c == (struct cmd *) -1) {
    		printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n")); /*MSG*/
		(void) fflush(stdout);
		code = -1;
		return;
	}
	if (c == 0) {
    		printf(MSGSTR(INVALID, "?Invalid command\n")); /*MSG*/
		(void) fflush(stdout);
		code = -1;
		return;
	}
	if (!c->c_proxy) {
    		printf(MSGSTR(INVLD_PROXY, "?Invalid proxy command\n")); /*MSG*/
		(void) fflush(stdout);
		code = -1;
		return;
	}
	if (setjmp(abortprox)) {
		code = -1;
		return;
	}
	oldintr = signal(SIGINT, proxabort);
	pswitch(1);
	if (c->c_conn && !connected) {
    		printf(MSGSTR(NOT_CONNECTED, "Not connected\n")); /*MSG*/
		(void) fflush(stdout);
		pswitch(0);
		(void) signal(SIGINT, oldintr);
		code = -1;
		return;
	}
	(*c->c_handler)(argc-1, argv+1);
	if (connected) {
		proxflag = 1;
	}
	else {
		proxflag = 0;
	}
	pswitch(0);
	(void) signal(SIGINT, oldintr);
}

setcase()
{
	mcase = !mcase;
  if(mcase)
    printf(MSGSTR(CASE_ON, "Case mapping on.\n")); /*MSG*/
  else
    printf(MSGSTR(CASE_OFF, "Case mapping off.\n")); /*MSG*/
	code = mcase;
}

setcr()
{
	crflag = !crflag;
  if(crflag)
    printf(MSGSTR(CR_STRIP_ON, "Carriage Return stripping on.\n")); /*MSG*/
  else
    printf(MSGSTR(CR_STRIP_OFF, "Carriage Return stripping off.\n")); /*MSG*/
	code = crflag;
}

setntrans(argc,argv)
	int argc;
	char *argv[];
{
	if (argc == 1) {
		ntflag = 0;
    		printf(MSGSTR(NTRANS_OFF, "Ntrans off.\n")); /*MSG*/
		code = ntflag;
		return;
	}
	ntflag++;
	code = ntflag;
	(void) strncpy(ntin, argv[1], 16);
	ntin[16] = '\0';
	if (argc == 2) {
		ntout[0] = '\0';
		return;
	}
	(void) strncpy(ntout, argv[2], 16);
	ntout[16] = '\0';
}

char *
dotrans(name)
  char *name;
{
  static char new[MAXPATHLEN];
  char *cp1, *cp2 = new;
  register int i, ostop, found;

  if (mb_cur_max > 1) {		/* multibyte code */
    wchar_t *wcname, *tmpwc;
    wchar_t wc_ntin[16], wc_ntout[16];
    register int  n, istop, mbcount, count;
	
    /* convert ntin and ntout to wide characters */
    istop = mbstowcs(wc_ntin, ntin, 16);
    ostop = mbstowcs(wc_ntout, ntout, 16);
    if (ostop < 0 || istop < 0) {
	fprintf(stderr, MSGSTR(DT_MBTOWC, "dotrans: error converting to widechar.\n"));
	return (name);
    }
     
    /* convert "name" to widechar */
    cp1 = name;
    n = (strlen(cp1)+1)*sizeof(wchar_t);
    wcname = (wchar_t *)malloc(n);
    mbcount = mbstowcs(wcname, cp1, n);
    if (mbcount < 0) {
	fprintf(stderr, MSGSTR(DT_MBTOWC2, "dotrans: error converting to widechar (%s) \n"), name);
	free(wcname);
	return(name);
    }

   /* find an occurence of *name in ntin, and map to ntout */
   for (count=0; count <= mbcount; count++) {
	for (i = 0; i < 16; i++) {
	    if (wcname[count] == wc_ntin[i]) {
		if (i < ostop)
		    wcname[count] = wc_ntout[i];
		break;
	    }
	}
    }
    tmpwc = wcname;
    /* after mapping is done, convert widechar back to multibyte */
    n = mbcount*mb_cur_max + 1;
    cp2 = (char *)malloc(n);
    mbcount = wcstombs(cp2, tmpwc, n);
    free(wcname);
    if (mbcount < 0) {
	fprintf(stderr, MSGSTR(DT_WCTOMB, "dotrans: error in converting from widechar to multibyte (%s) \n"), name);
	return(name);
    }
    return(cp2);
  }
   else {		/* single byte code */
	for (ostop = 0; *(ntout + ostop) && ostop < 16; ostop++);
	for (cp1 = name; *cp1; cp1++) {
	    found = 0;
	    for(i = 0 ;*(ntin + i) && i < 16; i++){
		if (*cp1 == *(ntin + i)) {
		    found++;
        	    if (i < ostop ) 
	    	       *cp2++ = *(ntout + i);
		    break;
		}
	    }
	    if (!found) 
	       *cp2++ = *cp1;
	}
	*cp2 = '\0';
   return(new);
   }	
}

setnmap(argc, argv)
	int argc;
	char *argv[];
{
	char *cp;

	if (argc == 1) {
		mapflag = 0;
    		printf(MSGSTR(NMAP_OF, "Nmap off.\n")); /*MSG*/
		code = mapflag;
		return;
	}
	if (argc < 3) {
		(void) strcat(line, " ");
    		printf(MSGSTR(MAPOUT, "(mapout) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 3) {
    		printf(MSGSTR(SETNMAP_USAGE, "Usage: %s [mapin mapout]\n"),argv[0]); /*MSG*/
		code = -1;
		return;
	}
	mapflag = 1;
	code = 1;
	cp = index(altarg, ' ');
	if (proxy) {
		while(*++cp == ' ');
		altarg = cp;
		cp = index(altarg, ' ');
	}
	*cp = '\0';
	(void) strncpy(mapin, altarg, MAXPATHLEN - 1);
	while (*++cp == ' ');
	(void) strncpy(mapout, cp, MAXPATHLEN - 1);
}

char *
domap(name)
	char *name;
{
	static char new[MAXPATHLEN];
	register char *cp1 = name, *cp2 = mapin;
	char *tp[9], *te[9];
	int i, toks[9], toknum = 0, match = 1;

	for (i=0; i < 9; ++i) {
		toks[i] = 0;
	}
	while (match && *cp1 && *cp2) {
		switch (*cp2) {
			case '\\':
				if (*++cp2 != *cp1) {
					match = 0;
				}
				break;
			case '$':
				if (*(cp2+1) >= '1' && (*cp2+1) <= '9') {
					if (*cp1 != *(++cp2+1)) {
						toks[toknum = *cp2 - '1']++;
						tp[toknum] = cp1;
						while (*++cp1 && *(cp2+1)
							!= *cp1);
						te[toknum] = cp1;
					}
					cp2++;
					break;
				}
				/* intentional drop through */
			default:
				if (*cp2 != *cp1) {
					match = 0;
				}
				break;
		}
		if (*cp1) {
			cp1++;
		}
		if (*cp2) {
			cp2++;
		}
	}
	if (!match && *cp1) /* last token mismatch */
	{
		toks[toknum] = 0;
	}
	cp1 = new;
	*cp1 = '\0';
	cp2 = mapout;
	while (*cp2) {
		match = 0;
		switch (*cp2) {
			case '\\':
				if (*(cp2 + 1)) {
					*cp1++ = *++cp2;
				}
				break;
			case '[':
LOOP:
				if (*++cp2 == '$' && isdigit(*(cp2+1))) { 
					if (*++cp2 == '0') {
						char *cp3 = name;

						while (*cp3) {
							*cp1++ = *cp3++;
						}
						match = 1;
					}
					else if (toks[toknum = *cp2 - '1']) {
						char *cp3 = tp[toknum];

						while (cp3 != te[toknum]) {
							*cp1++ = *cp3++;
						}
						match = 1;
					}
				}
				else {
					while (*cp2 && *cp2 != ',' && 
					    *cp2 != ']') {
						if (*cp2 == '\\') {
							cp2++;
						}
						else if (*cp2 == '$' &&
   						        isdigit(*(cp2+1))) {
							if (*++cp2 == '0') {
							   char *cp3 = name;

							   while (*cp3) {
								*cp1++ = *cp3++;
							   }
							}
							else if (toks[toknum =
							    *cp2 - '1']) {
							   char *cp3=tp[toknum];

							   while (cp3 !=
								  te[toknum]) {
								*cp1++ = *cp3++;
							   }
							}
						}
						else if (*cp2) {
							*cp1++ = *cp2++;
						}
					}
					if (!*cp2) {
            				  printf(MSGSTR(NMAP_UNBAL, "nmap: unbalanced brackets\n")); /*MSG*/
						return(name);
					}
					match = 1;
					cp2--;
				}
				if (match) {
					while (*++cp2 && *cp2 != ']') {
					      if (*cp2 == '\\' && *(cp2 + 1)) {
							cp2++;
					      }
					}
					if (!*cp2) {
            				        printf(MSGSTR(NMAP_UNBAL, "nmap: unbalanced brackets\n")); /*MSG*/
						return(name);
					}
					break;
				}
				switch (*++cp2) {
					case ',':
						goto LOOP;
					case ']':
						break;
					default:
						cp2--;
						goto LOOP;
				}
				break;
			case '$':
				if (isdigit(*(cp2 + 1))) {
					if (*++cp2 == '0') {
						char *cp3 = name;

						while (*cp3) {
							*cp1++ = *cp3++;
						}
					}
					else if (toks[toknum = *cp2 - '1']) {
						char *cp3 = tp[toknum];

						while (cp3 != te[toknum]) {
							*cp1++ = *cp3++;
						}
					}
					break;
				}
				/* intentional drop through */
			default:
				*cp1++ = *cp2;
				break;
		}
		cp2++;
	}
	*cp1 = '\0';
	if (!*new) {
		return(name);
	}
	return(new);
}

setsunique()
{
	sunique = !sunique;
    if (sunique)
        printf(MSGSTR(STORE_U_ON, "Store unique on.\n")); /*MSG*/
    else
        printf(MSGSTR(STORE_U_OFF, "Store unique off.\n")); /*MSG*/
	code = sunique;
}

setrunique()
{
	runique = !runique;
  if (runique)
    printf(MSGSTR(RECEIVE_U_ON, "Receive unique on.\n")); /*MSG*/
  else
    printf(MSGSTR(RECEIVE_U_OFF, "Receive unique off.\n")); /*MSG*/
	code = runique;
}

/* change directory to perent directory */
cdup()
{
	if (exp_cmds)
		(void) command("XCUP");
	else
		(void) command("CDUP");
}

/* show remote system type */
syst()
{
	(void) command("SYST");
}

macdef(argc, argv)
	int argc;
	char *argv[];
{
	char *tmp;
	int c;

	if (macnum == 16) {
    		printf(MSGSTR(MACRO_LIMIT, "Limit of 16 macros have already been defined\n")); /*MSG*/
		code = -1;
		return;
	}
	if (argc < 2) {
		(void) strcat(line, " ");
    		printf(MSGSTR(MACRO_NAME, "(macro name) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
    		printf(MSGSTR(MACRO_USAGE, "Usage: %s macro_name\n"),argv[0]); /*MSG*/
		code = -1;
		return;
	}
	if (interactive) {
    		printf(MSGSTR(MACRO_LINE, "Enter macro line by line, terminating it with a null line\n")); /*MSG*/
	}
	(void) strncpy(macros[macnum].mac_name, argv[1], 8);
	if (macnum == 0) {
		macros[macnum].mac_start = macbuf;
	}
	else {
		macros[macnum].mac_start = macros[macnum - 1].mac_end + 1;
	}
	tmp = macros[macnum].mac_start;
	while (tmp != macbuf+4096) {
		if ((c = getchar()) == EOF) {
      			printf(MSGSTR(MACRO_EOF, "macdef:end of file encountered\n")); /*MSG*/
			code = -1;
			return;
		}
		if ((*tmp = c) == '\n') {
			if (tmp == macros[macnum].mac_start) {
				macros[macnum++].mac_end = tmp;
				code = 0;
				return;
			}
			if (*(tmp-1) == '\0') {
				macros[macnum++].mac_end = tmp - 1;
				code = 0;
				return;
			}
			*tmp = '\0';
		}
		tmp++;
	}
	while (1) {
		while ((c = getchar()) != '\n' && c != EOF);
		if (c == EOF || getchar() == '\n') {
      			printf(MSGSTR(MACRO_UNDEFINED, "Macro not defined - 4k buffer exceeded\n")); /*MSG*/
			code = -1;
			return;
		}
	}
}

/*
 * get size of file on remote machine
 */
sizecmd(argc, argv)
	char *argv[];
{

	if (argc < 2) {
		(void) strcat(line, " ");
		printf(MSGSTR(FILENAME, "(filename) "));
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		printf(MSGSTR(USAGEFNAME,"usage:%s filename\n"), argv[0]);
		code = -1;
		return;
	}
	(void) command("SIZE %s", argv[1]);
}

/*
 * get last modification time of file on remote machine
 */
modtime(argc, argv)
	char *argv[];
{
	int overbose;

	if (argc < 2) {
		(void) strcat(line, " ");
		printf(MSGSTR(FILENAME, "(filename) "));
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		printf(MSGSTR(USAGEFNAME,"usage:%s filename\n"), argv[0]);
		code = -1;
		return;
	}
	overbose = verbose;
	if (debug == 0)
		verbose = -1;
	if (command("MDTM %s", argv[1]) == COMPLETE) {
		int yy, mo, day, hour, min, sec;
		sscanf(reply_string, "%*s %04d%02d%02d%02d%02d%02d", &yy, &mo,
			&day, &hour, &min, &sec);
		/* might want to print this in local time */
		printf("%s\t%02d/%02d/%04d %02d:%02d:%02d GMT\n", argv[1],
			mo, day, yy, hour, min, sec);
	} else {
		printf (reply_string, argv[1]);
		printf ("\n");
	}
	verbose = overbose;
}

/*
 * show status on remote machine
 */
rmtstatus(argc, argv)
	char *argv[];
{
	(void) command(argc > 1 ? "STAT %s" : "STAT" , argv[1]);
}

/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 */
int
gethdir(char *home)
{
        register struct passwd *pp = getpwnam(home);

        if (pp == 0)
                return (1);
        (void) strcpy(home, pp->pw_dir);
        return (0);
}

copyloc()
{
	copyflag = !copyflag;
	if (copyflag)
	 printf(MSGSTR(COPYLOCAL_ON, "Copylocal mode on.\n")); /*MSG*/
	else
	 printf(MSGSTR(COPYLOCAL_OFF, "Copylocal mode off.\n")); /*MSG*/
}

