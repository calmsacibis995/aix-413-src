static char sccsid[] = "@(#)37	1.11  src/bos/usr/ucb/vacation/vacation.c, cmdmailx, bos411, 9428A410j 11/15/93 14:28:32";
/* 
 * COMPONENT_NAME: CMDMAILX vacation.c
 * 
 * FUNCTIONS: Mvacation, getfrom, initialize, junkmail, knows, newstr, 
 *            sameword, sendmessage, setknows, syserr, usrerr 
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
 *  Vacation
 *  Copyright (c) 1983  Eric P. Allman
 *  Berkeley, California
 *
 *  Copyright (c) 1983 Regents of the University of California.
 *  All rights reserved.  The Berkeley software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
 *  @(#)vacation.c	5.3 (Berkeley) 7/1/85
 *
 */


# include <sys/types.h>
# include <pwd.h>
# include <stdio.h>
# include <sysexits.h>
# include <ctype.h>
#include <locale.h>
#include "vac_msg.h" 
#include <nl_types.h>

/*
**  VACATION -- return a message to the sender when on vacation.
**
**	This program could be invoked as a message receiver
**	when someone is on vacation.  It returns a message
**	specified by the user to whoever sent the mail, taking
**	care not to return a message too often to prevent
**	"I am on vacation" loops.
**
**	Positional Parameters:
**		the user to collect the vacation message from.
**
**	Flag Parameters:
**		-I	initialize the database.
**		-d	turn on debugging.
**
**	Side Effects:
**		A message is sent back to the sender.
**
**	Author:
**		Eric Allman
**		UCB/INGRES
*/

typedef int	bool;

# define TRUE		1
# define FALSE		0

# define MAXLINE	256	/* max size of a line */
# define MAXNAME	128	/* max size of one name */

# define ONEWEEK	(60L*60L*24L*7L)

long	Timeout = ONEWEEK;	/* timeout between notices per user */

struct dbrec
{
	long	sentdate;
};

typedef struct
{
	char	*dptr;
	int	dsize;
} DATUM;

extern DATUM fetch();



bool	Debug = FALSE;

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

main(argc, argv)
	char **argv;
{
	char *from;
	register int p;
	struct passwd *pw;
	char *homedir;
	char *myname;
	char buf[MAXLINE];
	extern char *newstr();
	extern char *getfrom();
	extern bool knows();
	extern bool junkmail();
	extern char *optarg;
	int interval_set = FALSE;

        setlocale(LC_ALL,"");
	scmc_catd = catopen(MF_VAC,NL_CAT_LOCALE);

	/* process arguments */
	while ((p = getopt(argc, argv, "Idf:")) != EOF)
		switch (p)
		{
		  case 'I':	/* initialize */
			initialize();
			exit(EX_OK);

		  case 'd':	/* debug */
			Debug = TRUE;
			break;

		  case 'f':	/* frequency interval */
			Timeout = convtime(optarg);
			if (!Timeout) {
				usrerr(catgets(scmc_catd, MS_vacation, M_USAGE, "Usage: vacation [-f interval] username (or) vacation -I"));
				exit(EX_USAGE);
			}
			interval_set = TRUE;
			break;
	
		  default:
			usrerr( catgets(scmc_catd, MS_vacation, M_USAGE, "Usage: vacation [-f interval] username (or) vacation -I") );
			exit(EX_USAGE);
		}

	/* verify recipient argument */
	if ( (!interval_set && argc != 2) ||
		 (interval_set && (argc != 3 && argc != 4)) )
	{
		usrerr( catgets(scmc_catd, MS_vacation, M_USAGE, "Usage: vacation [-f interval] username (or) vacation -I") );
		exit(EX_USAGE);
	}
	
	myname = argv[argc - 1];

	/* find user's home directory */
	pw = getpwnam(myname);
	if (pw == NULL)
	{
		usrerr( catgets(scmc_catd, MS_vacation, M_NOUSER, "Unknown user %s") , myname);
		exit(EX_NOUSER);
	}
	homedir = newstr(pw->pw_dir);
	(void) strcpy(buf, homedir);
	(void) strcat(buf, "/.vacation");
	dbminit(buf);

	/* read message from standard input (just from line) */
	from = getfrom();

	/* check if junk mail or this person is already informed */
	if (!junkmail(from) && !knows(from))
	{
		/* mark this person as knowing */
		setknows(from);

		/* send the message back */
		(void) strcpy(buf, homedir);
		(void) strcat(buf, "/.vacation.msg");
		if (Debug)
			printf( catgets(scmc_catd, MS_vacation, M_SENDING, "Sending %s to %s\n") , buf, from);
		else
		{
			sendmessage(buf, from, myname);
			/*NOTREACHED*/
		}
	}
	exit (EX_OK);
}
/*
**  GETFROM -- read message from standard input and return sender
**
**	Parameters:
**		none.
**
**	Returns:
**		pointer to the sender address.
**
**	Side Effects:
**		Reads first line from standard input.
*/

char *
getfrom()
{
	static char line[MAXLINE];
	register char *p;
	extern char *index();

	/* read the from line */
	if (fgets(line, sizeof line, stdin) == NULL ||
	    strncmp(line, "From ", 5) != (int)NULL)
	{
		usrerr( catgets(scmc_catd, MS_vacation, M_NOFROM, "No initial From line") );
		exit(EX_USAGE);
	}

	/* find the end of the sender address and terminate it */
	p = index(&line[5], ' ');
	if (p == NULL)
	{
		usrerr( catgets(scmc_catd, MS_vacation, M_BADFROM, "Funny From line '%s'") , line);
		exit(EX_USAGE);
	}
	*p = '\0';

	/* return the sender address */
	return (&line[5]);
}
/*
**  JUNKMAIL -- read the header and tell us if this is junk/bulk mail.
**
**	Parameters:
**		from -- the Return-Path of the sender.  We assume that
**			anything from "*-REQUEST@*" is bulk mail.
**
**	Returns:
**		TRUE -- if this is junk or bulk mail (that is, if the
**			sender shouldn't receive a response).
**		FALSE -- if the sender deserves a response.
**
**	Side Effects:
**		May read the header from standard input.  When this
**		returns the position on stdin is undefined.
*/

bool
junkmail(from)
	char *from;
{
	register char *p;
	char buf[MAXLINE+1];
	extern char *index();
	extern char *rindex();
	extern bool sameword();

	/* test for inhuman sender */
	p = rindex(from, '@');
	if (p != NULL)
	{
		*p = '\0';
		if (sameword(&p[-8],  "-REQUEST") ||
		    sameword(&p[-10], "Postmaster") ||
		    sameword(&p[-13], "MAILER-DAEMON"))
		{
			*p = '@';
			return (TRUE);
		}
		*p = '@';
	}

	/* read the header looking for a "Precedence:" line */
	while (fgets(buf, MAXLINE, stdin) != NULL && buf[0] != '\n')
	{
		/* should ignore case, but this is reasonably safe */
		if (strncmp(buf, "Precedence", 10) != 0 ||
		    !(buf[10] == ':' || buf[10] == ' ' || buf[10] == '\t'))
		{
			continue;
		}

		/* find the value of this field */
		p = index(buf, ':');
		if (p == NULL)
			continue;
		while (*++p != '\0' && isspace(*p))
			continue;
		if (*p == '\0')
			continue;

		/* see if it is "junk" or "bulk" */
		p[4] = '\0';
		if (sameword(p, "junk") || sameword(p, "bulk"))
			return (TRUE);
	}
	return (FALSE);
}
/*
**  KNOWS -- predicate telling if user has already been informed.
**
**	Parameters:
**		user -- the user who sent this message.
**
**	Returns:
**		TRUE if 'user' has already been informed that the
**			recipient is on vacation.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

bool
knows(user)
	char *user;
{
	DATUM k, d;
	long now;
	auto long then;

	time(&now);
	k.dptr = user;
	k.dsize = strlen(user) + 1;
	d = fetch(k);
	if (d.dptr == NULL)
		return (FALSE);
	
	/* be careful on 68k's and others with alignment restrictions */
	bcopy((char *) &((struct dbrec *) d.dptr)->sentdate, (char *) &then, sizeof then);
	if (then + Timeout < now)
		return (FALSE);
	return (TRUE);
}
/*
**  SETKNOWS -- set that this user knows about the vacation.
**
**	Parameters:
**		user -- the user who should be marked.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The dbm file is updated as appropriate.
*/

setknows(user)
	char *user;
{
	DATUM k, d;
	struct dbrec xrec;

	k.dptr = user;
	k.dsize = strlen(user) + 1;
	time(&xrec.sentdate);
	d.dptr = (char *) &xrec;
	d.dsize = sizeof xrec;
	store(k, d);
}
/*
**  SENDMESSAGE -- send a message to a particular user.
**
**	Parameters:
**		msgf -- filename containing the message.
**		user -- user who should receive it.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends mail to 'user' using /usr/sbin/sendmail.
*/

sendmessage(msgf, user, myname)
	char *msgf;
	char *user;
	char *myname;
{
	FILE *f;

	/* find the message to send */
	f = freopen(msgf, "r", stdin);
	if (f == NULL)
	{
		f = freopen("/usr/share/lib/vacation.def", "r", stdin);
		if (f == NULL)
			syserr( catgets(scmc_catd, MS_vacation, M_NOMSG, "No message to send") );
	}

	execl("/usr/sbin/sendmail", "sendmail", "-f", myname, user, NULL);
	syserr( catgets(scmc_catd, MS_vacation, M_BADEXEC, "Cannot exec /usr/sbin/sendmail") );
}
/*
**  INITIALIZE -- initialize the database before leaving for vacation
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Initializes the files .vacation.{pag,dir} in the
**		caller's home directory.
*/

initialize()
{
	char *homedir;
	char buf[MAXLINE];
	extern char *getenv();

	setgid(getgid());
	setuid(getuid());
	homedir = getenv("HOME");
	if (homedir == NULL)
		syserr( catgets(scmc_catd, MS_vacation, M_NOHOME, "No home!") );
	(void) strcpy(buf, homedir);
	(void) strcat(buf, "/.vacation.dir");
	if (close(creat(buf, 0644)) < 0)
		syserr( catgets(scmc_catd, MS_vacation, M_ECREATE, "Cannot create %s") , buf);
	(void) strcpy(buf, homedir);
	(void) strcat(buf, "/.vacation.pag");
	if (close(creat(buf, 0644)) < 0)
		syserr( catgets(scmc_catd, MS_vacation, M_ECREATE, "Cannot create %s") , buf);
}
/*
**  USRERR -- print user error
**
**	Parameters:
**		f -- format.
**		p -- first parameter.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

usrerr(f, p)
	char *f;
	char *p;
{
	fprintf(stderr,  catgets(scmc_catd, MS_vacation, M_VAC, "vacation: ") );
	_doprnt(f, &p, stderr);
	fprintf(stderr, "\n");
}
/*
**  SYSERR -- print system error
**
**	Parameters:
**		f -- format.
**		p -- first parameter.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

syserr(f, p)
	char *f;
	char *p;
{
	fprintf(stderr,  catgets(scmc_catd, MS_vacation, M_VAC, "vacation: ") );
	_doprnt(f, &p, stderr);
	fprintf(stderr, "\n");
	exit(EX_USAGE);
}
/*
**  NEWSTR -- copy a string
**
**	Parameters:
**		s -- the string to copy.
**
**	Returns:
**		A copy of the string.
**
**	Side Effects:
**		none.
*/

char *
newstr(s)
	char *s;
{
	char *p;
	extern char *malloc();

	p = malloc(strlen(s) + 1);
	if (p == NULL)
	{
		syserr( catgets(scmc_catd, MS_vacation, M_EMALLOC, "newstr: cannot alloc memory") );
		exit(EX_OSERR);
	}
	strcpy(p, s);
	return (p);
}
/*
**  SAMEWORD -- return TRUE if the words are the same
**
**	Ignores case.
**
**	Parameters:
**		a, b -- the words to compare.
**
**	Returns:
**		TRUE if a & b match exactly (modulo case)
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

bool
sameword(a, b)
	register char *a, *b;
{
	char ca, cb;

	do
	{
		ca = *a++;
		cb = *b++;
		if (isascii(ca) && isupper(ca))
			ca = ca - 'A' + 'a';
		if (isascii(cb) && isupper(cb))
			cb = cb - 'A' + 'a';
	} while (ca != '\0' && ca == cb);
	return (ca == cb);
}

long
convtime(the_time)
	char *the_time;
{
	register long t, r;
	register char c;

	r = 0;
	while (*the_time != '\0')
	{
		t = 0;
		while (isdigit(c = *the_time++))
			t = t * 10 + (c - '0');
		if (c == '\0')
			the_time--;
		switch (c)
		{	
		  case 'w':		/* weeks */
			t *= 7;
		  
		  case 'd':		/* days */
		  case '\0': 
			t *= 24;
		
		  case 'h':		/* hours */
			t *= 60;
		
		  case 'm':		/* minutes */
			t *= 60;

		  case 's':		/* seconds */
			break;

		  default:
			t = 0;
		}
		r += t;
	}
	return (r);
}	
