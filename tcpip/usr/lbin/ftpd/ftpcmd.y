/* @(#)82      1.16.3.1  src/tcpip/usr/lbin/ftpd/ftpcmd.y, tcpfilexfr, tcpip41J, 9510A 2/10/95 15:48:33 */
/* 
 * COMPONENT_NAME: TCPIP ftpcmd.y
 * 
 * FUNCTIONS: copy, getline, help, lookup, sizecmd, toolong, upper, 
 *            yylex 
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
 * Copyright (c) 1985, 1988 Regents of the University of California.
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
 *
 *	ftpcmd.y	5.20.1.1 (Berkeley) 3/2/89
 */

/*
 * Grammar for FTP commands.
 * See RFC 959.
 */

/*
 * The following commands have been implemented or enabled according
 * to RFC 1123 (some original options are also shown):
 * 	REIN		(Reinitialize - USER command expected next)
 *	TYPE 	A N	(Ascii type, Non-printing format)
 *		A T	(Ascii type, Telnet - same as TYPE N)
 *		A C	(Ascii type, Carriage-control format)
 *	 	E N	(Ebcdic type, Non-printing format)
 *		E T	(Ebcdic type, Telnet - same as TYPE N)
 *		E C	(Ebcdic type, Carriage-control format)
 *		I	(Image type)
 *		L m	(Logical byte size m)
 *	STRU	F	(File structure)
 *		R	(Record structure)
 *	MODE	B	(Block mode)
 *		S	(Stream mode)
 *	SITE	HELP	(Help for the site command)
 *		UMASK	(Display or set file creation umask value)
 *		CHMOD	(chmod on a file)
 *		IDLE	(Display or set the idle timeout value)
 *	REST	m	(Restart the subsequent transfer with state m)
 *	XMKD		(Same as MKD)
 *	XRMD		(Same as RMD)
 *	XPWD		(Same as PWD)
 *	XCUP		(Same as CDUP)
 *	XCWD		(Same as CWD)
 *
 * Note that the SIZE command is not compatible with the implementation
 * of the REST command.
 */

%{
/*
#ifndef lint
static char sccsid[] = "ftpcmd.y	5.20.1.1 (Berkeley) 3/2/89";
#endif  not lint */

#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/ftp.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <setjmp.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <time.h>
#include <glob.h>

#ifdef _CSECURITY
#include "tcpip_audit.h"
/* Security New Stuff */
#include <sys/id.h>
#include <sys/priv.h>
extern 	ulong	remote_addr;
extern 	uid_t saved_uid, effective_uid;
extern	priv_t priv;

/* Restore privs and set the invoker back to uid saved_uid */
#define GET_PRIV(a)            \
        getpriv(PRIV_MAXIMUM, &priv,sizeof(priv_t)); \
        setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t)); \
        setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, (a));

/* Drop privs and set the invoker to uid a */
#define DROP_PRIV(a)           \
        priv.pv_priv[0] = 0;    \
        priv.pv_priv[1] = 0;    \
        setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t)); \
        setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, (a));

#endif _CSECURITY

#include <nl_types.h>
#include "ftpd_msg.h" 
extern  nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTPD,n,s) 

extern	struct sockaddr_in data_dest;
extern	int logged_in;
extern	struct passwd *pw;
extern	int guest;
extern	int logging;
extern	int type;
extern	int bytesz;
extern	int form;
extern	int stru;
extern	int mode;
extern	off_t rest_mark;
extern	int line_state;
extern	int car_ctl_char;
extern	int next_bits;
extern	int out_bits;
extern	int out_char;
extern	int debug;
extern	int timeout;
extern	int maxtimeout;
extern  int pdata;
extern	char hostname[], remotehost[];
extern	char proctitle[];
extern	int usedefault;
extern  int transflag;
extern  char tmpline[];

extern	char    tempstr[];
extern	char    tempuser[];
extern	int	gethdir();
extern	char 	*home;		/* pointer to home directory for glob */

static	int cmd_type;
static	int cmd_form;
static	int cmd_bytesz;
static	int cmd_struct;
static	int cmd_mode;
char	cbuf[512];
char	*fromname;

char	*index();
#ifdef _CSECURITY
extern char *audit_tail[];
#endif _CSECURITY
%}

%token
	A	B	C	D	E	F
	G	H	I	J	K	L
	M	N	O	P	Q	R
	S	T	U	V	W	X
	Y	Z

	SP	CRLF	COMMA	STRING	NUMBER

	USER	PASS	ACCT	REIN	QUIT	PORT
	PASV	TYPE	STRU	MODE	RETR	STOR
	APPE	MLFL	MAIL	MSND	MSOM	MSAM
	MRSQ	MRCP	ALLO	REST	RNFR	RNTO
	ABOR	DELE	CWD	LIST	NLST	SITE
	STAT	HELP	NOOP	MKD	RMD	PWD
	CDUP	STOU	SMNT	SYST	SIZE	MDTM

	UMASK	IDLE	CHMOD

	LEXERR

%start	cmd_list

%%

cmd_list:	/* empty */
	|	cmd_list cmd
		= {
			fromname = (char *) 0;
		}
	|	cmd_list rcmd
	;

cmd:		USER SP username CRLF
		= {
			user((char *) $3);
			free((char *) $3);
		}
	|	PASS SP password CRLF
		= {
			pass((char *) $3);
			free((char *) $3);
		}
	|	REIN CRLF
		= {
			reply(220, MSGSTR(REINIT_OK, "Service ready for new user."));
			end_login();
		}
	|	PORT SP host_port CRLF
		= {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, MSGSTR(PORTCMD_OK, "PORT command successful."));
		}
	|	PASV CRLF
		= {
			passive();
		}
	|	TYPE SP type_code CRLF
		= {
			switch (cmd_type) {

			case TYPE_A:
				switch (cmd_form) {
				case FORM_C:
					reply(200, MSGSTR(TYPESET_A_C,
					  "Type set to A; form set to C."));
					type = cmd_type;
					form = cmd_form;
					break;
				case FORM_N:
					reply(200, MSGSTR(TYPESET_A_N,
					  "Type set to A; form set to N."));
					type = cmd_type;
					form = cmd_form;
					break;
				case FORM_T:
					reply(200, MSGSTR(TYPESET_A_T,
					  "Type set to A; form set to T."));
					type = cmd_type;
					form = cmd_form;
					break;
				default:
					reply(501, MSGSTR(BAD_FORM, 
					  "Form must be C, N, or T."));
					break;
				}
				break;

			case TYPE_E:
				switch (cmd_form) {
				case FORM_C:
					reply(200, MSGSTR(TYPESET_E_C,
					  "Type set to E; form set to C."));
					type = cmd_type;
					form = cmd_form;
					break;
				case FORM_N:
					reply(200, MSGSTR(TYPESET_E_N,
					  "Type set to E; form set to N."));
					type = cmd_type;
					form = cmd_form;
					break;
				case FORM_T:
					reply(200, MSGSTR(TYPESET_E_T,
					  "Type set to E; form set to T."));
					type = cmd_type;
					form = cmd_form;
					break;
				default:
					reply(501, MSGSTR(BAD_FORM, 
					  "Form must be C, N, or T."));
					break;
				}
				break;

			case TYPE_I:
				reply(200, MSGSTR(TYPESET_I, "Type set to I."));
				type = cmd_type;
				break;

			case TYPE_L:
#if NBBY == 8
				reply(200,
				    MSGSTR(TYPESET_L,
				    "Type set to L (byte size %d)."),
				     cmd_bytesz);
				type = cmd_type;
				bytesz = cmd_bytesz;
#else /* NBBY == 8 */
				UNIMPLEMENTED for NBBY != 8
#endif /* NBBY == 8 */
				break;

			default:
				reply(501, MSGSTR(BAD_TYPE, 
				  "Type must be A, E, I, or L."));
				break;
			}
		}
	|	STRU SP struct_code CRLF
		= {
			switch (cmd_struct) {

			case STRU_F:
				reply(200, MSGSTR(STRUCTSET_F,
				    "Structure set to F."));
				stru = cmd_struct;
				break;

			case STRU_R:
				reply(200, MSGSTR(STRUCTSET_R,
				    "Structure set to R."));
				stru = cmd_struct;
				break;

			case STRU_P:
				reply(504, MSGSTR(UNIMP_STRUCT,
				    "Unimplemented structure type."));
				stru = cmd_struct;
				break;

			default:
				reply(501, MSGSTR(BAD_STRUCT, 
				  "Structure must be F or R."));
				break;
			}
		}
	|	MODE SP mode_code CRLF
		= {
			switch (cmd_mode) {

			case MODE_B:
				reply(200, MSGSTR(MODESET_B, "MODE set to B."));
				mode = cmd_mode;
				break;

			case MODE_S:
				reply(200, MSGSTR(MODESET_S, "MODE set to S."));
				mode = cmd_mode;
				break;

			case MODE_C:
				reply(504, MSGSTR(UNIMP_MODE,
				    "Unimplemented mode type."));
				break;

			default:
				reply(501, MSGSTR(BAD_MODE, 
				  "Mode must be B or S."));
				break;
			}
		}
	|	ALLO SP NUMBER CRLF
		= {
			reply(202, MSGSTR(ALLO_IGN, "ALLO command ignored."));
		}
	|	ALLO SP NUMBER SP R SP NUMBER CRLF
		= {
			reply(202, MSGSTR(ALLO_IGN, "ALLO command ignored."));
		}
	|	RETR check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				retrieve((char *) 0, (char *) $4);
			if ($4 != NULL)
				free((char *) $4);
			rest_mark = 0;
		}
	|	STOR check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
			    if (rest_mark == 0)
				store((char *) $4, "w", 0);
			    else
				store((char *) $4, "r+", 0);
			if ($4 != NULL)
				free((char *) $4);
			rest_mark = 0;
		}
	|	APPE check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
			    if (rest_mark == 0)
				store((char *) $4, "a", 0);
			    else
				store((char *) $4, "r+", 0);
			if ($4 != NULL)
				free((char *) $4);
			rest_mark = 0;
		}
	|	NLST check_login CRLF
		= {
			if ($2)
				send_file_list(".");
		}
	|	NLST check_login SP STRING CRLF
		= {
			if ($2 && $4 != NULL) 
				send_file_list((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	LIST check_login CRLF
		= {
			if ($2)
				retrieve("/bin/ls -lA", "");
		}
	|	LIST check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				retrieve("/bin/ls -lA %s", (char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	STAT check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				statfilecmd((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	STAT CRLF
		= {
			statcmd();
		}
	|	DELE check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				delete((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	RNTO SP pathname CRLF
		= {
			if (fromname) {
				renamecmd(fromname, (char *) $3);
				free(fromname);
				fromname = (char *) 0;
			} else {
				reply(503, MSGSTR(BADSEQ_CMD, "Bad sequence of commands."));
			}
			free((char *) $3);
		}
	|	ABOR CRLF
		= {
			reply(225, MSGSTR(ABOR_OK, "ABOR command successful."));
		}
	|	CWD check_login CRLF
		= {
			if ($2)
				cwd(pw->pw_dir);
		}
	|	CWD check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				cwd((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	HELP CRLF
		= {
			help(cmdtab, (char *) 0);
		}
	|	HELP SP STRING CRLF
		= {
			register char *cp = (char *)$3;

			if (strncasecmp(cp, "SITE", 4) == 0) {
				cp = (char *)$3 + 4;
				if (*cp == ' ')
					cp++;
				if (*cp)
					help(sitetab, cp);
				else
					help(sitetab, (char *) 0);
			} else
				help(cmdtab, (char *) $3);
		}
	|	NOOP CRLF
		= {
			reply(200, MSGSTR(NOOP_OK, "NOOP command successful."));
		}
	|	MKD check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				makedir((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	RMD check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				removedir((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	PWD check_login CRLF
		= {
			if ($2)
				pwd();
		}
	|	CDUP check_login CRLF
		= {
			if ($2)
				cwd("..");
		}
	|	SITE SP HELP CRLF
		= {
			help(sitetab, (char *) 0);
		}
	|	SITE SP HELP SP STRING CRLF
		= {
			help(sitetab, (char *) $5);
		}
	|	SITE SP UMASK check_login CRLF
		= {
			int oldmask;

			if ($4) {
				oldmask = umask(0);
				(void) umask(oldmask);
				reply(200, MSGSTR(UMASKIS, "Current UMASK is %03o"), oldmask);
			}
		}
	|	SITE SP UMASK check_login SP octal_number CRLF
		= {
			int oldmask;

			if ($4) {
				if (($6 == -1) || ($6 > 0777)) {
					reply(501, MSGSTR(BAD_UMASK, "Bad UMASK value"));
				} else {
					oldmask = umask($6);
					reply(200,
					    MSGSTR(SET_UMASK, "UMASK set to %03o (was %03o)"),
					    $6, oldmask);
				}
			}
		}
	|	SITE SP CHMOD check_login SP octal_number SP pathname CRLF
		= {
			if ($4 && ($8 != NULL)) {
				if ($6 > 0777)
					reply(501,MSGSTR(CHMOD_ERR, 
			       "CHMOD: Mode value must be between 0 and 0777"));
				else if (chmod((char *) $8, $6) < 0)
					perror_reply(550, (char *) $8);
				else
					reply(200, MSGSTR(CHMOD_OK, "CHMOD command successful."));
			}
			if ($8 != NULL)
				free((char *) $8);
		}
	|	SITE SP IDLE CRLF
		= {
			reply(200,MSGSTR(IDLELIMIT,
			    "Current IDLE time limit is %d seconds; max %d"),
				timeout, maxtimeout);
		}
	|	SITE SP IDLE SP NUMBER CRLF
		= {
			if ($5 < 30 || $5 > maxtimeout) {
				reply(501,MSGSTR(MAX_IDLE, 
			"Maximum IDLE time must be between 30 and %d seconds"),
				    maxtimeout);
			} else {
				timeout = $5;
				(void) alarm((unsigned) timeout);
				reply(200,MSGSTR(MAX_IDLE_SET,
				    "Maximum IDLE time set to %d seconds"),
				    timeout);
			}
		}
	|	STOU check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				store((char *) $4, "w", 1);
			if ($4 != NULL)
				free((char *) $4);
		}
	|	SYST CRLF
		= {
#ifdef unix
#ifdef _BSD
			reply(215, MSGSTR(UNIX_T_V, "UNIX Type: L%d Version: BSD-%d"),
				NBBY, _BSD);
#else /* _BSD */
			reply(215, MSGSTR(UNIXTYPE, "UNIX Type: L%d"), NBBY);
#endif /* _BSD */
#else /* unix */
			reply(215, MSGSTR(UNKTYPE, "UNKNOWN Type: L%d"), NBBY);
#endif /* unix */
		}

		/*
		 * SIZE is not in RFC959, but Postel has blessed it and
		 * it will be in the updated RFC.
		 *
		 * Return size of file in a format suitable for
		 * using with RESTART (we just count bytes).
		 */
	|	SIZE check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				sizecmd((char *) $4);
			if ($4 != NULL)
				free((char *) $4);
		}

		/*
		 * MDTM is not in RFC959, but Postel has blessed it and
		 * it will be in the updated RFC.
		 *
		 * Return modification time of file as an ISO 3307
		 * style time. E.g. YYYYMMDDHHMMSS or YYYYMMDDHHMMSS.xxx
		 * where xxx is the fractional second (of any precision,
		 * not necessarily 3 digits)
		 */
	|	MDTM check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL) {
				struct stat stbuf;
				if (stat((char *) $4, &stbuf) < 0)
					perror_reply(550, "%s", (char *) $4);
				else if ((stbuf.st_mode&S_IFMT) != S_IFREG) {
					reply(550, MSGSTR(NOTPLAINF, "%s: not a plain file."),
						(char *) $4);
				} else {
					register struct tm *t;
					struct tm *gmtime();
					t = gmtime(&stbuf.st_mtime);
					reply(213,
					    "19%02d%02d%02d%02d%02d%02d",
					    t->tm_year, t->tm_mon+1, t->tm_mday,
					    t->tm_hour, t->tm_min, t->tm_sec);
				}
			}
			if ($4 != NULL)
				free((char *) $4);
		}
	|	REST SP marker CRLF
		= {
			rest_mark = 0;
                        sscanf((char *)$3, "%08d%02d%03d%03d%02d%03d",
                                &rest_mark, &line_state, &car_ctl_char,
                                &next_bits, &out_bits, &out_char);
			if (rest_mark > 0) {
				reply(350, 
				    MSGSTR(REST_OK, "Restart begun at %d."), 
				    rest_mark);
			}
			else {
				reply(554, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid REST parameter"));
			}
			if ($3 != NULL)
				free((char *) $3);
		}
	|	QUIT CRLF
		= {
			reply(221, MSGSTR(GOODBYE1, "Goodbye."));
#ifdef _CSECURITY
			GET_PRIV(saved_uid);
			CONNECTION_WRITE(remote_addr,"ftp/tcp","close",
						MSGSTR(GOODBYE1, "Goodbye."),0);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
			dologout(0);
		}
	|	error CRLF
		= {
			yyerrok;
		}
	;
rcmd:		RNFR check_login SP pathname CRLF
		= {
			char *renamefrom();

			if ($2 && $4) {
				fromname = renamefrom((char *) $4);
				if (fromname == (char *) 0 && $4) {
					free((char *) $4);
				}
			}
		}
	;
		
username:	STRING
	;

password:	/* empty */
		= {
			*(char **)&($$) = "";
		}
	|	STRING
	;

byte_size:	NUMBER
	;

host_port:	NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA 
		NUMBER COMMA NUMBER
		= {
			register char *a, *p;

			a = (char *)&data_dest.sin_addr;
			a[0] = $1; a[1] = $3; a[2] = $5; a[3] = $7;
			p = (char *)&data_dest.sin_port;
			p[0] = $9; p[1] = $11;
			data_dest.sin_family = AF_INET;
		}
	;

form_code:	N
	= {
		$$ = FORM_N;
	}
	|	T
	= {
		$$ = FORM_T;
	}
	|	C
	= {
		$$ = FORM_C;
	}
	;

type_code:	A
	= {
		cmd_type = TYPE_A;
		cmd_form = FORM_N;
	}
	|	A SP form_code
	= {
		cmd_type = TYPE_A;
		cmd_form = $3;
	}
	|	E
	= {
		cmd_type = TYPE_E;
		cmd_form = FORM_N;
	}
	|	E SP form_code
	= {
		cmd_type = TYPE_E;
		cmd_form = $3;
	}
	|	I
	= {
		cmd_type = TYPE_I;
	}
	|	L SP byte_size
	= {
		cmd_type = TYPE_L;
		cmd_bytesz = $3;
	}
	/* this is for a bug in the BBN ftp */
	|	L byte_size
	= {
		cmd_type = TYPE_L;
		cmd_bytesz = $2;
	}
	;

struct_code:	F
	= {
		cmd_struct= STRU_F;
	}
	|	R
	= {
		cmd_struct= STRU_R;
	}
	|	P
	= {
		cmd_struct= STRU_P;
	}
	;

mode_code:	S
	= {
		cmd_mode= MODE_S;
	}
	|	B
	= {
		cmd_mode= MODE_B;
	}
	|	C
	= {
		cmd_mode= MODE_C;
	}
	;

pathname:	pathstring
	= {
		/*
		 * Problem: this production is used for all pathname
		 * processing, but only gives a 550 error reply.
		 * This is a valid reply in some cases but not in others.
		 */
		if (logged_in && $1 && strncmp((char *) $1, "~", 1) == 0) {
			glob_t globbuf;
			char *cp;
			int i;
			/*
			 * First we need to do tilde expansion since glob()
			 * in libc.a is to dumb to do it for us.
			 */
			cp = (char *) $1;
			cp++;
			if (*cp == (char) NULL) {
				/*
				 * we found a null after the tilde.
				 */
				strcpy(tempstr, home);
				$$ = tempstr;
			} else if (*cp == '/') {
				/*
				 * we found a / after the tilde.
				 */
				strcpy(tempstr, home);
				strcat(tempstr, cp);
				cp++;
				if (*cp == (char) NULL)
					$$ = tempstr;
			} else {
				/* we might of found a user after the tilde */
				for (i = 0; i < MAXPATHLEN &&
						 *cp != '/' && *cp != '\0'; i++)
					tempuser[i] = *cp++;
				tempuser[i] = '\0';
				if (gethdir(tempuser)) {
					reply(550, MSGSTR(UNKNOWN_USER,
						"Unknown user name after ~\n"));
					$$ = NULL;
				} else {
					strcpy(tempstr, tempuser);/* user dir */
					strcat(tempstr, cp); /* rest of path */
				}
			}
			if (strpbrk(tempstr, "*?[{")) {
				/* we have glob characters in tempstr */
				if (glob(tempstr,GLOB_QUOTE|GLOB_NOCHECK,
							NULL,&globbuf) != 0) {
					/* globbing failed */
					reply(550, MSGSTR(BAD_DIR,
						"Bad directory components"));
					$$ = NULL;
				}
				if (globbuf.gl_pathc == 0) {
					/* globbing failed */
					reply(550, MSGSTR(BAD_DIR,
						"Bad directory components"));
					$$ = NULL;
				}
				if (($$ = (char *) malloc((unsigned)
				        strlen(globbuf.gl_pathv[0])+1)) != NULL)
						strcpy($$, globbuf.gl_pathv[0]);
			} else
				$$ = tempstr;
		} else
			$$ = $1;
	}
	;

pathstring:	STRING
	;

marker:		STRING
	;

octal_number:	NUMBER
	= {
		register int ret, dec, multby, digit;

		/*
		 * Convert a number that was read as decimal number
		 * to what it would be if it had been read as octal.
		 */
		dec = $1;
		multby = 1;
		ret = 0;
		while (dec) {
			digit = dec%10;
			if (digit > 7) {
				ret = -1;
				break;
			}
			ret += digit * multby;
			multby *= 8;
			dec /= 10;
		}
		$$ = ret;
	}
	;

check_login:	/* empty */
	= {
		if (logged_in)
			$$ = 1;
		else {
			reply(530, MSGSTR(LOGUSERPASS, "Please login with USER and PASS."));
			$$ = 0;
		}
	}
	;

%%

extern jmp_buf errcatch;

#define	CMD	0	/* beginning of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional SP then STRING */
#define	ZSTR1	5	/* SP then optional STRING */
#define	ZSTR2	6	/* optional STRING after SP */
#define	SITECMD	7	/* SITE command */
#define	NSTR	8	/* Number followed by a string */

struct tab {
	char	*name;
	short	token;
	short	state;
	short	implemented;	/* 1 if command is implemented */
	char	*help;
};

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, ZSTR1, 1,	"<sp> password" },
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "SMNT", SMNT, ARGS, 0,	"(structure mount)" },
	{ "REIN", REIN, ARGS, 1,	"(reinitialize server state)" },
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "PASV", PASV, ARGS, 1,	"(set server in passive mode)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
	{ "REST", REST, STR1, 1,	"(restart command)" },
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
	{ "ABOR", ABOR, ARGS, 1,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name ]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, SITECMD, 1,	"site-cmd [ <sp> arguments ]" },
	{ "SYST", SYST, ARGS, 1,	"(get type of operating system)" },
	{ "STAT", STAT, OSTR, 1,	"[ <sp> path-name ]" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
	{ "MKD",  MKD,  STR1, 1,	"<sp> path-name" },
	{ "XMKD", MKD,  STR1, 1,	"<sp> path-name" },
	{ "RMD",  RMD,  STR1, 1,	"<sp> path-name" },
	{ "XRMD", RMD,  STR1, 1,	"<sp> path-name" },
	{ "PWD",  PWD,  ARGS, 1,	"(return current directory)" },
	{ "XPWD", PWD,  ARGS, 1,	"(return current directory)" },
	{ "CDUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "XCUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "STOU", STOU, STR1, 1,	"<sp> file-name" },
	{ "SIZE", SIZE, OSTR, 1,	"<sp> path-name" },
	{ "MDTM", MDTM, OSTR, 1,	"<sp> path-name" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab sitetab[] = {
	{ "UMASK", UMASK, ARGS, 1,	"[ <sp> umask ]" },
	{ "IDLE", IDLE, ARGS, 1,	"[ <sp> maximum-idle-time ]" },
	{ "CHMOD", CHMOD, NSTR, 1,	"<sp> mode <sp> file-name" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab *
lookup(p, cmd)
	register struct tab *p;
	char *cmd;
{

	for (; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include <arpa/telnet.h>

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
getline(s, n, iop)
	char *s;
	register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
/* tmpline may contain saved command from urgent mode interruption */
	for (c = 0; tmpline[c] != '\0' && --n > 0; ++c) {
		*cs++ = tmpline[c];
		if (tmpline[c] == '\n') {
			*cs++ = '\0';
			if (debug)
			    syslog(LOG_DEBUG, MSGSTR(CMDMSG, "command: %s"), s);
			tmpline[0] = '\0';
			return(s);
		}
		if (c == 0)
			tmpline[0] = '\0';
	}
	while ((c = getc(iop)) != EOF) {
		c &= 0377;
		if (c == IAC) {
		    if ((c = getc(iop)) != EOF) {
			c &= 0377;
			switch (c) {
			case WILL:
			case WONT:
				c = getc(iop);
				printf("%c%c%c", IAC, DONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case DO:
			case DONT:
				c = getc(iop);
				printf("%c%c%c", IAC, WONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case IAC:
				break;
			default:
				continue;	/* ignore command */
			}
		    }
		}
		*cs++ = c;
		if (--n <= 0 || c == '\n')
			break;
	}
	if (c == EOF && cs == s)
		return (NULL);
	*cs++ = '\0';
	if (debug)
	   syslog(LOG_DEBUG, MSGSTR(CMDMSG, "command: %s"), s);
	return (s);
}

static int
toolong()
{
	time_t now;
	extern char *ctime();
	extern time_t time();

	reply(421,MSGSTR(TIMEOUT, "Timeout (%d seconds): closing control connection."), timeout);
	(void) time(&now);
	if (logging) {
		syslog(LOG_INFO,MSGSTR(USER_TIMEOUT, "User %s timed out after %d seconds at %s"),
			(pw ? pw -> pw_name : "unknown"), timeout, ctime(&now));
	}
#ifdef _CSECURITY
			GET_PRIV(saved_uid);
                        CONNECTION_WRITE(remote_addr, "ftp/tcp", "close",
                                "Control connection timeout", -1);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
	dologout(1);
}

yylex()
{
	static int cpos, state;
	register char *cp, *cp2;
	register struct tab *p;
	int n;
	char c, *strpbrk();
	char *copy();

	for (;;) {
		switch (state) {

		case CMD:
			(void) signal(SIGALRM, toolong);
			(void) alarm((unsigned) timeout);
			if (getline(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, MSGSTR(GOODBYE, "You could at least say goodbye."));
#ifdef _CSECURITY
			GET_PRIV(saved_uid);
                        CONNECTION_WRITE(remote_addr, "ftp/tcp", "close",
			MSGSTR(GOODBYE, "You could at least say goodbye."), -1);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
				dologout(0);
			}
			(void) alarm(0);
#ifdef SETPROCTITLE
			if (strncasecmp(cbuf, "PASS", 4) != NULL)
				setproctitle("%s: %s", proctitle, cbuf);
#endif /* SETPROCTITLE */
			if ((cp = index(cbuf, '\r'))) {
				*cp++ = '\n';
				*cp = '\0';
			}
			if ((cp = strpbrk(cbuf, " \n")))
				cpos = cp - cbuf;
			if (cpos == 0)
				cpos = 4;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cmdtab, cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				*(char **)&yylval = p->name;
				return (p->token);
			}
			break;

		case SITECMD:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			cp = &cbuf[cpos];
			if ((cp2 = strpbrk(cp, " \n")))
				cpos = cp2 - cbuf;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cp);
			p = lookup(sitetab, cp);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					state = CMD;
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				*(char **)&yylval = p->name;
				return (p->token);
			}
			state = CMD;
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR1:
		case ZSTR1:
		dostr1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = state == OSTR ? STR2 : ++state;
				return (SP);
			}
			break;

		case ZSTR2:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				*(char **)&yylval = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case NSTR:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				state = STR1;
				return (NUMBER);
			}
			state = STR1;
			goto dostr1;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'D':
			case 'd':
				return (D);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'G':
			case 'g':
				return (G);

			case 'H':
			case 'h':
				return (H);

			case 'I':
			case 'i':
				return (I);

			case 'J':
			case 'j':
				return (J);

			case 'K':
			case 'k':
				return (K);

			case 'L':
			case 'l':
				return (L);

			case 'M':
			case 'm':
				return (M);

			case 'N':
			case 'n':
				return (N);

			case 'O':
			case 'o':
				return (O);

			case 'P':
			case 'p':
				return (P);

			case 'Q':
			case 'q':
				return (Q);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			case 'U':
			case 'u':
				return (U);

			case 'V':
			case 'v':
				return (V);

			case 'W':
			case 'w':
				return (W);

			case 'X':
			case 'x':
				return (X);

			case 'Y':
			case 'y':
				return (Y);

			case 'Z':
			case 'z':
				return (Z);

			}
			break;

		default:
			fatal(MSGSTR(UNK_STATE, "Unknown state in scanner."));
		}
		yyerror((char *) 0);
		state = CMD;
		longjmp(errcatch,0);
	}
}

upper(s)
	register char *s;
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

char *
copy(s)
	char *s;
{
	char *p;
	extern char *malloc(), *strcpy();

	p = malloc((unsigned) strlen(s) + 1);
	if (p == NULL)
		fatal(MSGSTR(NOMEMORY, "Ran out of memory."));
	(void) strcpy(p, s);
	return (p);
}

help(ctab, s)
	struct tab *ctab;
	char *s;
{
	register struct tab *c;
	register int width, NCMDS;
	char *type;

	if (ctab == sitetab)
		type = "SITE ";
	else
		type = "";
	width = 0, NCMDS = 0;
	for (c = ctab; c->name != NULL; c++) {
		int len = strlen(c->name);

		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		register int i, j, w;
		int columns, lines;

		lreply(214, MSGSTR(CMDS_RECOGNIZED, "The following %scommands are recognized %s."),
		    type, MSGSTR(UNIMP_CMD, "(* =>'s unimplemented)"));
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			printf("   ");
			for (j = 0; j < columns; j++) {
				c = ctab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &ctab[NCMDS])
					break;
				w = strlen(c->name) + 1;
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
			printf("\r\n");
		}
		(void) fflush(stdout);
		reply(214, MSGSTR(DIR_COMMENT, "Direct comments to ftp-bugs@%s."), hostname);
		return;
	}
	upper(s);
	c = lookup(ctab, s);
	if (c == (struct tab *)0) {
		reply(502, MSGSTR(UNK_CMD, "Unknown command %s."), s);
		return;
	}
	if (c->implemented)
		reply(214, MSGSTR(SYNTEX, "Syntax: %s%s %s"), type, c->name, c->help);
	else
		reply(214, MSGSTR(UNIMP1, "%s%-*s\t%s; unimplemented."), type, width,
		    c->name, c->help);
}

sizecmd(filename)
char *filename;
{
	switch (type) {
	case TYPE_L:
	case TYPE_I: {
		struct stat stbuf;
		if (stat(filename, &stbuf) < 0 ||
		    (stbuf.st_mode&S_IFMT) != S_IFREG)
			reply(550, MSGSTR(NOTPLAINF, "%s: not a plain file."), filename);
		else
			reply(213, "%lu", stbuf.st_size);
		break;}
	case TYPE_A: {
		FILE *fin;
		register int c, count;
		struct stat stbuf;
		fin = fopen(filename, "r");
		if (fin == NULL) {
			perror_reply(550, filename);
			return;
		}
		if (fstat(fileno(fin), &stbuf) < 0 ||
		    (stbuf.st_mode&S_IFMT) != S_IFREG) {
			reply(550, MSGSTR(NOTPLAINF, "%s: not a plain file."), filename);
			(void) fclose(fin);
			return;
		}

		count = 0;
		while((c=getc(fin)) != EOF) {
			if (c == '\n')	/* will get expanded to \r\n */
				count++;
			count++;
		}
		(void) fclose(fin);

		reply(213, "%ld", count);
		break;}
	default:
		reply(504, MSGSTR(SIZENOTIMP, "SIZE not implemented for Type %c."), "?AEIL"[type]);
	}
}
