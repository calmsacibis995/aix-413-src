static char sccsid[] = "@(#)80        1.17.1.1  src/tcpip/usr/bin/ftp/ruserpass.c, tcpfilexfr, tcpip411, 9439B411a 9/28/94 17:26:51";
/* 
 * COMPONENT_NAME: TCPIP ruserpass.c
 * 
 * FUNCTIONS: MSGSTR, rnetrc, token
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


struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

extern int macnum, proxy;			/* number of defined macros */
extern struct macel macros[16], *macpt;
extern char macbuf[4096];

#include <stdio.h>
#include <netdb.h>
#ifdef aiws
#include <sys/types.h>
#endif
#include <utmp.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/nameser.h>
#include <errno.h>
#ifdef _AIX
#include <sys/access.h>
#else
#endif _AIX

#include <nl_types.h>
#include "ftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTP,n,s) 

char	*renvlook(), *malloc(), *index(), *getenv(), *getpass(), *getlogin();
char	*strcpy();
struct	utmp *getutmp();
static	FILE *cfile;

/*
ruserpass(host, aname, apass, aacct)
	char *host, **aname, **apass, **aacct;
{

	renv(host, aname, apass, aacct);		THESE TWO LINES WERE
	if (*aname == 0 || *apass == 0)			ALREADY COMMENTED.

		return(rnetrc(host, aname, apass, aacct));
}
*/

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	ACCOUNT 4
#define MACDEF  5
#define	ID	10
#define	MYMACHINE	11

static char tokval[MAXDNAME];

static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	"default",	DEFAULT,
	"login",	LOGIN,
	"password",	PASSWD,
	"account",	ACCOUNT,
	"machine",	MYMACHINE,
	"macdef",	MACDEF,
	0,		0
};

rnetrc(host, aname, apass, aacct)
	char *host, **aname, **apass, **aacct;
{
	char *hdir, buf[BUFSIZ], *tmp;
	int t, i, c;
	struct stat stb;
	char ldomain[MAXDNAME], *domainp = NULL, lhost[MAXDNAME];
	long addr;
	struct hostent *hp;
	extern int errno;

	hdir = getenv("HOME");
	if (hdir == NULL)
		hdir = ".";
	(void) sprintf(buf, "%s/.netrc", hdir);
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (errno != ENOENT)
			perror(buf);
		return(0);
	}

	/*
	 * get local domain name
	 */
	if (!gethostname(ldomain, sizeof(ldomain))) {
		ldomain[sizeof(ldomain)-1] = '\0';
		domainp = index(ldomain, '.');
	}

	/*
	 * check for ip address and do a lookup to convert to hostname
	 */
	if (isinet_addr(host) && (addr = inet_addr(host)) != -1 &&
	    (hp = gethostbyaddr(&addr, sizeof(addr), AF_INET))) {
		strcpy(lhost, hp->h_name);
		host = lhost;
	}

	/*
	 * add local domain to domainless hostname
	 */
	if (domainp && !index(host, '.')) {
		strcpy(lhost, host);
		strcat(lhost, domainp);
		host = lhost;
	}

next:
	while ((t = token())) switch(t) {

	case DEFAULT:
		(void) token();
		continue;

	case MYMACHINE:
		if (token() != ID)
			continue;

		/*
		 * NB: if we change the hostname-comparison logic here,
		 * do the same in lib/c/net/ruserpass.c, so that ftp and
		 * rexec process the .netrc file the same way.
		 */

		/*
		 * check for ip address and do a lookup to convert to hostname
		 */
		if (isinet_addr(tokval) && (addr = inet_addr(tokval)) != -1
		    && (hp = gethostbyaddr(&addr, sizeof(addr), AF_INET)))
			strcpy(tokval, hp->h_name);

		/*
		 * add local domain to domainless hostname
		 */
		if (domainp && !index(tokval, '.'))
			strcat(tokval, domainp);

		if (strcasecmp(host, tokval))
			continue;

		while ((t = token()) && t != MYMACHINE) switch(t) {

		case LOGIN:
			if (token())
				if (*aname == 0) { 
					*aname = malloc((unsigned) strlen(tokval) + 1);
					(void) strcpy(*aname, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			break;
		case PASSWD:
#ifdef _AIX
                        if ((faccessx(fileno(cfile), R_ACC, ACC_OTHERS) == 0) ||
                            (faccessx(fileno(cfile), W_ACC, ACC_OTHERS) == 0) ||
                            (faccessx(fileno(cfile), X_ACC, ACC_OTHERS) == 0))
#else
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0)
#endif _AIX
							 {
	fprintf(stderr, MSGSTR(NETRC_ERR, "Error - .netrc file not correct mode.\n")); /*MSG*/
	fprintf(stderr, MSGSTR(NETRC_ERR_PASS, "Remove password or correct mode.\n")); /*MSG*/
				return(-1);
			}
			if (token() && *apass == 0) {
				*apass = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*apass, tokval);
			}
			break;
		case ACCOUNT:
#ifdef _AIX
                        if ((faccessx(fileno(cfile), R_ACC, ACC_OTHERS) == 0) ||
                            (faccessx(fileno(cfile), W_ACC, ACC_OTHERS) == 0) ||
                            (faccessx(fileno(cfile), X_ACC, ACC_OTHERS) == 0))
#else
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0)
#endif _AIX
							 {
	fprintf(stderr, MSGSTR(NETRC_ERR, "Error - .netrc file not correct mode.\n")); /*MSG*/
	fprintf(stderr, MSGSTR(NETRC_ERR_ACCT, "Remove account or correct mode.\n")); /*MSG*/
				return(-1);
			}
			if (token() && *aacct == 0) {
				*aacct = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*aacct, tokval);
			}
			break;
		case MACDEF:
			if (proxy) {
				return(0);
			}
			while ((c=getc(cfile)) != EOF && c == ' ' || c == '\t');
			if (c == EOF || c == '\n') {
				printf(MSGSTR(MDEF, "Missing macdef name argument.\n")); /*MSG*/
				return(-1);
			}
			if (macnum == 16) {
				printf(MSGSTR(MACRO_LIMIT, "Limit of 16 macros have already been defined\n")); /*MSG*/
				return(-1);
			}
			tmp = macros[macnum].mac_name;
			*tmp++ = c;
			for (i=0; i < 8 && (c=getc(cfile)) != EOF &&
			    !isspace(c); ++i) {
				*tmp++ = c;
			}
			if (c == EOF) {
				printf(MSGSTR(MACRO_DEF, "Macro definition missing null line terminator.\n")); /*MSG*/
				return(-1);
			}
			*tmp = '\0';
			if (c != '\n') {
				while ((c=getc(cfile)) != EOF && c != '\n');
			}
			if (c == EOF) {
				printf(MSGSTR(MACRO_DEF, "Macro definition missing null line terminator.\n")); /*MSG*/
				return(-1);
			}
			if (macnum == 0) {
				macros[macnum].mac_start = macbuf;
			}
			else {
				macros[macnum].mac_start = macros[macnum-1].mac_end + 1;
			}
			tmp = macros[macnum].mac_start;
			while (tmp != macbuf + 4096) {
				if ((c=getc(cfile)) == EOF) {
				printf(MSGSTR(MACRO_DEF, "Macro definition missing null line terminator.\n")); /*MSG*/
					return(-1);
				}
				*tmp = c;
				if (*tmp == '\n') {
					if (*(tmp-1) == '\0') {
					   macros[macnum++].mac_end = tmp - 1;
					   break;
					}
					*tmp = '\0';
				}
				tmp++;
			}
			if (tmp == macbuf + 4096) {
				printf(MSGSTR(MBUF_EXCEEDED, "4K macro buffer exceeded\n")); /*MSG*/
				return(-1);
			}
			break;
		default:
	fprintf(stderr, MSGSTR(NETRC_KEYWORD, "Unknown .netrc keyword %s\n"), tokval); /*MSG*/
			break;
		}
		goto done;
	}
done:
	(void) fclose(cfile);
	return(0);
}

static
token()
{
	char *cp;
	int c;
	struct toktab *t;

	if (feof(cfile))
		return (0);
	while ((c = getc(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' '))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (t = toktab; t->tokstr; t++)
		if (!strcmp(t->tokstr, tokval))
			return (t->tval);
	return (ID);
}
