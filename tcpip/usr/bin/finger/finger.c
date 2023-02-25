static char sccsid[] = "@(#)11	1.35  src/tcpip/usr/bin/finger/finger.c, tcpadmin, tcpip41J, 9520B_all 5/17/95 14:19:17";
/* 
 * COMPONENT_NAME: TCPIP finger.c
 * 
 * FUNCTIONS: MSGSTR, Mfinger, decode, doall, donames, findidle, 
 *            findwhen, fwclose, fwopen, ltimeprint, matchcmp, 
 *            namecmp, netfinger, personprint, print, pwdcopy, 
 *            quickprint, shortprint, stimeprint, show_text, vputc 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 The Regents of the University of California.
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
"Copyright (c) 1980 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "finger.c	5.11 (Berkeley) 9/20/88";
#endif  not lint */

/*
 * This is a finger program.  It prints out useful information about users
 * by digging it up from various system files.  It is not very portable
 * because the most useful parts of the information (the full user name,
 * office, and phone numbers) are all stored in the VAX-unused gecos field
 * of /etc/passwd, which, unfortunately, other UNIXes use for other things.
 *
 * There are three output formats, all of which give login name, teletype
 * line number, and login time.  The short output format is reminiscent
 * of finger on ITS, and gives one line of information per user containing
 * in addition to the minimum basic requirements (MBR), the full name of
 * the user, his idle time and office location and phone number.  The
 * quick style output is UNIX who-like, giving only name, teletype and
 * login time.  Finally, the long style output give the same information
 * as the short (in more legible format), the home directory and shell
 * of the user, and, if it exits, a copy of the file .plan in the users
 * home directory.  Finger may be called with or without a list of people
 * to finger -- if no list is given, all the people currently logged in
 * are fingered.
 *
 * The program is validly called by one of the following:
 *
 *	finger			{short form list of users}
 *	finger -l		{long form list of users}
 *	finger -b		{briefer long form list of users}
 *	finger -q		{quick list of users}
 *	finger -i		{quick list of users with idle times}
 *	finger -w 		{narrow, short format list of users}
 *	finger namelist		{long format list of specified users}
 *	finger -s namelist	{short format list of specified users}
 *	finger -w namelist	{long format list of specified users}
 *
 * where 'namelist' is a list of user login names.
 *
 * The other options can all be given after one '-', or each can have its
 * own '-'.
 *	finger -f		{disables the printing of headers for}
 *				{   short and quick ouputs.}
 *	finger -h		{disables printing .project info in long}
 *				{   output}
 *	finger -p		{disables printing .plan info in long output}
 *	finger -m		{only search for the name as a USERID}
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <sys/signal.h>
#include <pwd.h>
#include <stdio.h>
#ifdef LAST_LOGIN
#include <lastlog.h>
#endif
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/access.h>
#include <nl_types.h>
#include <locale.h>

#include "finger_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_FINGER, Num, Str)

#define ASTERISK	'*'		/* ignore this in real name */
#define COMMA		','		/* separator in pw_gecos field */
#define COMMAND		'-'		/* command line flag char */
#define SAMENAME	'&'		/* repeat login name in real name */
#define SLASH		'/'		/* delimeter for ulimit in name */

struct utmp user;
struct tm *tmptr;
#define NMAX sizeof(user.ut_user)
#define LMAX sizeof(user.ut_line)
#define HMAX sizeof(user.ut_host)

struct person {			/* one for each person fingered */
	char *name;			/* name */
	char *realname;			/* pointer to full name */
	char *site_info;		/* pointer to gecos after real name */
	struct passwd *pwd;		/* structure of /etc/passwd stuff */
	char loggedin;			/* person is logged in */
	struct where *whead, *wtail;	/* list of where he/she has been */
	struct person *link;		/* link to next person */
};

struct where {
        struct where *next;             /* next place he/she has been */
	char writable;			/* tty is writable */
	long loginat;			/* time of (last) login */
	long idletime;			/* how long idle (if logged in) */
	char tty[LMAX+1];		/* null terminated tty line */
	char host[HMAX+1];		/* null terminated remote host name */
};

#ifdef LAST_LOGIN
char LASTLOG[] = "/usr/adm/lastlog";	/* last login info */
#endif
char PLAN[] = "/.plan";			/* what plan file is */
char PROJ[] = "/.project";		/* what project file */
	
int unbrief = 1;			/* -b option default */
int header = 1;				/* -f option default */
int hack = 1;				/* -h option default */
int idle = 0;				/* -i option default */
int large = 0;				/* -l option default */
int match = 1;				/* -m option default */
int plan = 1;				/* -p option default */
int unquick = 1;			/* -q option default */
int small = 0;				/* -s option default */
int wide = 1;				/* -w option default */

int unshort;
#ifdef LAST_LOGIN
int lf;					/* LASTLOG file descriptor */
#endif
struct person *person1, *lastperson;	/* head and tail of list of people */
long tloc;				/* current time */

struct passwd *pwdcopy();
struct where *wherecopy();
struct person *enter_person();
char *strcpy();
char *malloc();
char *ctime();
struct tm *localtime();
char *asctime();
char buf[BUFSIZ];			/* to hold "date and time" string */


main(argc, argv)
	int argc;
	register char **argv;
{
	FILE *fp;
	register char *s;
	extern int optind;
	int ch;

	setlocale(LC_ALL,"");
	catd = catopen (MF_FINGER, NL_CAT_LOCALE);
	/* parse command line for (optional) arguments */
	while ((ch = getopt(argc, argv, "bfhilmpqsw")) != EOF)
			switch(ch) {
			case 'b':
				unbrief = 0;
				break;
			case 'f':
				header = 0;
				break;
			case 'h':
				hack = 0;
				break;
			case 'i':
				idle = 1;
				unquick = 0;
				break;
			case 'l':
				large = 1;
				break;
			case 'm':
				match = 0;
				break;
			case 'p':
				plan = 0;
				break;
			case 'q':
				unquick = 0;
				break;
			case 's':
				small = 1;
				break;
			case 'w':
				wide = 0;
				break;
			default:
				fprintf(stderr, MSGSTR(USAGE,"Usage: finger [-bfhilmpqsw] [login1 [login2 ...] ]\n"));
				exit(1);
			}
	if (unquick || idle)
		time(&tloc);

	argc -= optind;
	argv += optind;

	/*
	 * *argv == 0 means no names given
	 */
	if (*argv == 0)
		doall();
	else
		donames(argc, argv);
	if (person1)
		print();
	exit(0);
}

doall()
{
	register struct person *p, *ptr;
	register struct passwd *pw;
	register struct utmp *ut;
	char name[NMAX + 1];
	int  y;

	unshort = large;
	setutent();
	if (unquick) {
		setpwent();
#ifdef LAST_LOGIN
		fwopen();
#endif
	}
	while (ut = getutent()) {
		/* verify that the utmp entry is active */
		if (ut->ut_type != USER_PROCESS)
			continue;
		/* get null-terminated name to see if it is in /etc/passwd */
		for (y = 0; y < NMAX; y++)
                   if (ut->ut_user[y] != 0)
                      name[y] = ut->ut_user[y];
                   else
                      break;
		name[y] = 0;
		if ((ut->ut_user[0] == 0) || (getpwnam (name) == 0))
			continue;
		if (person1 == 0) {
			p = person1 = (struct person *) malloc(sizeof *p);
			p->whead = p->wtail = 0;
			p->link = 0;
			p->pwd = 0;
			if (unquick && (pw = getpwnam(name))) {
				p->pwd = pwdcopy(pw);
				decode(p);
				p->name = p->pwd->pw_name;
			} 
			else
		        	p->name = strcpy(malloc(strlen(name) + 1), name);
			ptr = p;
                }
		else {
        		/* search for name in current list */
                        for (ptr = person1; ptr != 0; ptr = ptr->link) 
				if (strcmp(ptr->name, ut->ut_user) == 0)
 					break;
			/* if name not found in list then create new entry */
			if (ptr == 0) {
				p->link = (struct person *) malloc(sizeof *p);
				p = p->link;
				p->whead = p->wtail = 0;
				p->link = 0;
				p->pwd = 0;
			   	if (unquick && (pw = getpwnam(name))) {
					p->pwd = pwdcopy(pw);
					decode(p);
					p->name = p->pwd->pw_name;
				} else
				        p->name = strcpy(malloc(strlen(name) + 1), name);
				ptr = p;
			}
		}		
		ptr->loggedin = 1;
          	if (ptr->whead == 0)
		        ptr->whead = ptr->wtail = wherecopy(ut,ptr);
	 	else {		
			ptr->wtail->next = wherecopy(ut,ptr);
			ptr->wtail = ptr->wtail->next;
   		}
		ptr->wtail->next = 0;
	}
	if (unquick) {
#ifdef LAST_LOGIN
		fwclose();
#endif
		endpwent();
	}
	endutent();
	if (person1 == 0) {
		printf(MSGSTR(NOLOG, "No one logged on\n"));
		return;
	}
}

donames(argc, argv)
	register argc;
	register char **argv;
{
	register struct person *p, *ptr;
	register struct passwd *pw;
	register struct utmp *ut;
	int *used;
	register i;

	/*
	 * get names from command line and check to see if they're
	 * logged in
	 */
	unshort = !small;
	setpwent();

    	if (!(used = (int *)calloc((unsigned int)argc, (unsigned int)sizeof(int)))) {
		printf(MSGSTR(NOSPACE,"finger: out of space.\n"));
		exit(1);
	}
	/* Do all network requests */
	for (i = 0; i < argc; i++)
		if (netfinger(argv[i]))
			used[i] = -1;
	if (!match) {
		for (i = 0; i < argc; i++) 
			if (used[i] >= 0 && (pw = getpwnam(argv[i]))) {
				enter_person(pw);
				used[i] = 1;
			}
	}
	else { while ((pw = _getpwent()) != 0) {
	 	for (i = 0; i < argc; i++)
			if (used[i] >= 0 &&
			   ((strcmp(argv[i], pw->pw_name) == 0) ||
			    matchcmp(pw->pw_gecos, pw->pw_name, argv[i]))) {
				enter_person(pw);
				used[i] = 1;
			}
    		}
	}
	/* print all errors */
	for (i = 0; i < argc; i++)
		if (!used[i])
			printf(MSGSTR(NOUSER, "finger: %s: no such user.\n"), argv[i]);

	endpwent();

	/* Now get login information */
	setutent();
	while (ut = getutent()) {
		if ((*ut->ut_user == 0) || (ut->ut_type != USER_PROCESS))
			continue;
		for (p = person1; p != 0; p = p->link) {
			if (strncmp(p->pwd ? p->pwd->pw_name : p->name,
				    ut->ut_user, NMAX) != 0)
				continue;
			if (p->loggedin == 0) {
				p->loggedin = 1;
				p->whead = p->wtail = wherecopy(ut,p);
			} 
			else {
				p->wtail->next = wherecopy(ut,p);
				p->wtail = p->wtail->next;
			}
			p->wtail->next = 0;
		}
	}
	endutent();
	if (unquick) {
#ifdef LAST_LOGIN
		fwopen();
#endif
		for (p = person1; p != 0; p = p->link)
			decode(p);
#ifdef LAST_LOGIN
		fwclose();
#endif
	}
} /* end of donames */


struct person *
enter_person(pw)
	register struct passwd *pw;
{
 	register struct person *ptr;

#define savestr(s) strcpy(malloc(strlen(s) + 1), s)
	if (person1 == 0) {
		ptr = person1 = (struct person *) malloc(sizeof *ptr);
		ptr->whead = ptr->wtail = 0;
		ptr->pwd = pwdcopy(pw);
		ptr->name = savestr(pw->pw_name);
		ptr->loggedin = 0;
		ptr->link = 0;
		lastperson=person1;
	}

	/* Otherwise, search through the list of names to see if */
  	/* the name already exists in the list                   */
	else
		for (ptr = person1; 
			ptr != 0 && (strcmp(ptr->name, pw->pw_name) !=0);
			ptr = ptr->link)
			;

	/* Create an entry for the name if an entry does */
	/* not already exist.                            */
	if (ptr == 0) {
		lastperson->link = (struct person *) malloc(sizeof *lastperson);
		lastperson = lastperson->link;
		lastperson->whead = lastperson->wtail = 0;
		lastperson->pwd = pwdcopy(pw);
		lastperson->name = savestr(pw->pw_name);
		lastperson->loggedin = 0;
		lastperson->link = 0;
	}
#undef savestr
	return(ptr);
}

print()
{
	register FILE *fp;
	register struct person *p;
	register char *s;
	register c;

	/*
	 * print out what we got
	 */
	if (header) {
		if (unquick) {
			if (!unshort)
				if (wide)
					printf(MSGSTR(HEADER1,"Login       Name              TTY   Idle    When    Site Info\n"));
				else
					printf(MSGSTR(HEADER2,"Login    TTY   Idle    When    Site Info\n"));
		} else {
			printf(MSGSTR(HEADER3,"Login      TTY            When"));
			if (idle)
				printf(MSGSTR(HEADER4,"             Idle"));
			putchar('\n');
		}
	}
	for (p = person1; p != 0; p = p->link) {
		if (!unquick) {
			quickprint(p);
			continue;
		}
		if (!unshort) {
			shortprint(p);
			continue;
		}
		personprint(p);
		if (p->pwd != 0) {
			if (hack) {
				s = malloc(strlen(p->pwd->pw_dir) + 1+
					sizeof PROJ);
				strcpy(s, p->pwd->pw_dir);
				strcat(s, PROJ);
                                (void) show_text(s,MSGSTR(PROJECT,"Project: "));
                                free(s);
			}
			if (plan) {
				s = malloc(strlen(p->pwd->pw_dir) + 1+
					sizeof PLAN);
				strcpy(s, p->pwd->pw_dir);
				strcat(s, PLAN);
                                if (!show_text(s,MSGSTR(PALN, "\nPlan:")))
                                      (void)printf(MSGSTR(NOPLAN,"No Plan.\n"));
                                free(s);
			}
		}
		if (p->link != 0)
			putchar('\n');
	}
}

/*
 * Duplicate a pwd entry.
 * Note: Only the useful things (what the program currently uses) are copied.
 */
struct passwd *
pwdcopy(pfrom)
	register struct passwd *pfrom;
{
	register struct passwd *pto;

	pto = (struct passwd *) malloc(sizeof *pto);
#define savestr(s) strcpy(malloc(strlen(s) + 1), s)
	pto->pw_name = savestr(pfrom->pw_name);
	pto->pw_uid = pfrom->pw_uid;
	pto->pw_gecos = savestr(pfrom->pw_gecos);
	pto->pw_dir = savestr(pfrom->pw_dir);
	pto->pw_shell = savestr(pfrom->pw_shell);
#undef savestr
	return pto;
}

/*
 * Get the login information  
 */
struct where *
wherecopy(utinfo, pers)
	register struct utmp *utinfo;
	register struct person *pers;
{
	register struct where *w;

	w = (struct where *) malloc(sizeof *w);
	bcopy(utinfo->ut_line, w->tty, LMAX);
	w->tty[LMAX] = 0;
	bcopy(utinfo->ut_host, w->host, HMAX);
	w->host[HMAX] = 0;
	w->loginat = utinfo->ut_time;
	if (pers->loggedin)
		findidle(w);
	else
#ifdef LAST_LOGIN
		findwhen(pers, w);
#else
		w->loginat = 0L;
#endif
	return w;
}

/*
 * print out information on quick format giving just name, tty, login time
 * and idle time if idle is set.
 */
quickprint(pers)
	register struct person *pers;
{
	register struct where *w;

	printf(MSGSTR(PNAME,"%-*.*s  "), NMAX, NMAX, pers->name);
	if (pers->loggedin) {
		w = pers->whead;
		for (w = pers->whead; w != 0; w = w->next) { 
			if (w != pers->whead)
				printf(MSGSTR(PNAME,"%-*.*s  "), NMAX, NMAX, pers->name);
			if (idle) {
				tmptr = localtime(&w->loginat);
				if (strftime(buf, BUFSIZ, "%a %sD %H:%M", tmptr))
					printf(MSGSTR(QUICKPRINT1,"%c%-*s %s"), w->writable ? ' ' : '*',
					LMAX, w->tty, buf );
				else
					printf(MSGSTR(QUICKPRINT1,"%c%-*s %-16.16s"), w->writable ? ' ' : '*',
					LMAX, w->tty, asctime(tmptr));
				ltimeprint("   ", &w->idletime, "");
			} 
			else {
				tmptr = localtime(&w->loginat);
				if (strftime(buf, BUFSIZ, "%a %sD %H:%M", tmptr))
					printf(MSGSTR(QUICKPRINT2," %-*s %s"), LMAX,
					w->tty, buf);
				else 
					printf(MSGSTR(QUICKPRINT2," %-*s %-16.16s"), LMAX, w->tty, ctime(&w->loginat));
			}			
			putchar('\n');
		} 
	} else
		printf(MSGSTR(NOTLOG,"          Not Logged In\n"));
}

/*
 * print out information in short format, giving login name, full name,
 * tty, idle time, login time, office location and phone.
 */
shortprint(pers)
	register struct person *pers;
{
	char *p;
	char dialup;
	register struct where *w;

	w = pers->whead;
	do {
		if (pers->pwd == 0) {
			printf("%-15s       ???\n", pers->name);
			return;
		}
		printf("%-*s", NMAX, pers->pwd->pw_name);
		dialup = 0;
		if (wide) {
			if (pers->realname)
				printf(" %-20.20s", pers->realname);
			else
				printf("        ???          ");
		}
		putchar(' ');
		if (pers->loggedin && !w->writable)
			putchar('*');
		else
			putchar(' ');
		if (*w->tty) {
			if (w->tty[0] == 't' && w->tty[1] == 't' &&
		    	w->tty[2] == 'y') {
				if (w->tty[3] == 'd' && pers->loggedin)
					dialup = 1;
				printf("%-1.1s%-2.2s ", w->tty , w->tty + 3);
			}
		        else if (w->tty[0] == 'l' && w->tty[1] == 'f' &&
			           w->tty[2] == 't') 
				printf("%-1.1s%-2.4s ", w->tty , w->tty + 3);
			else 
				printf("%-1.1s%-2.4s ", w->tty , w->tty + 4);
		} else
			printf("   ");
		if (w != 0 ) {
			/* print the idle time */
			stimeprint(&w->idletime); 
			if (w->loginat != 0)
				tmptr = localtime(&w->loginat);
		}
		else
			printf("      ");
		if (pers->loggedin) {
			if (strftime(buf, BUFSIZ, "%a %H:%M", tmptr))
				printf(" %s ", buf);
			else 
			{
				p = ctime(&w->loginat);
				printf(" %3.3s %-5.5s ", p, p + 11);
			}
		} else if (w == 0 || w->loginat == 0) 
			printf(" < .  .  .  . >");
		else if (tloc - w->loginat >= 180 * 24 * 60 * 60) {
			if (strftime(buf, BUFSIZ, "%sD, %Y", tmptr))
				printf( " <%s>", buf);
			else
			{
				p = ctime(&w->loginat);
				printf(" <%-6.6s, %-4.4s>", p + 4, p + 20);
			}
		} else {
			if (strftime(buf, BUFSIZ, "%sD %H:%M", tmptr))
				printf(" <%s>", buf);
			else 
			{	 
				p = ctime(&w->loginat); 
				printf(" <%-12.12s>", p + 4);
			}
		}
		if (strlen (pers->site_info) > 0)
			printf (" %s", pers->site_info);
		putchar('\n');
		if (pers->loggedin)
			w = w->next;
	} while (w != 0);
}

/*
 * print out a person in long format giving all possible information.
 * directory and shell are inhibited if unbrief is clear.
 */
personprint(pers)
	register struct person *pers;
{
	register struct where *w;

	if (pers->pwd == 0) {
		printf(MSGSTR(LOGIN1,"Login name: %-10s\t\t\tIn real life: ???\n"), pers->name);
		return;
	}
	printf(MSGSTR(LOGIN2,"Login name: %-10s"), pers->pwd->pw_name);
	if (pers->realname) {
		printf("\t\t      "); 
		printf(MSGSTR(REALNAME, "In real life: %s"), pers->realname);
	}
	if (pers->site_info)
		printf(MSGSTR(SITEINFO, "\nSite Info: %s"), pers->site_info);
	if (unbrief) {
		printf(MSGSTR(DIR,"\nDirectory: %-25s"), pers->pwd->pw_dir);
		if (*pers->pwd->pw_shell)
			printf(MSGSTR(SHELL,"\tShell: %-s"), pers->pwd->pw_shell);
	}
	if (pers->loggedin) {
		for (w = pers->whead; w != 0 ; w = w->next) {
			tmptr = localtime(&w->loginat);
			if (strftime(buf, BUFSIZ, "%sD %H:%M:%S", tmptr))
				printf(MSGSTR(SINCE1,"\nOn since %s on %s"), buf, w->tty);
			else
			{
				register char *ep = asctime(tmptr);
				printf(MSGSTR(SINCE1,"\nOn since %15.15s on %s"),
				&ep[4], w->tty);
			}
			ltimeprint(", ", &w->idletime, MSGSTR(IDLE," Idle Time"));
			if (*w->host)
				printf(MSGSTR(FROM,"\n    from %s"), w->host);

			/* Since we print out the idle time in a different */
			/* format from bsd due to ILS, this message no     */
			/* longer fits on the first line of the output.    */
			/* Therefore, we chose to indent it on a seperate  */
			/* line.					   */
			if (pers->loggedin && !w->writable)
				printf(MSGSTR(MSGOFF,"\n    (messages off)"));
		}
	} 
#ifdef LAST_LOGIN
	else if (tloc - w->loginat > 180 * 24 * 60 * 60) {
		tmptr = localtime(&w->loginat);
		if (strftime(buf, BUFSIZ, "%a %sD, %Y", tmptr))		
			printf(MSGSTR(LASTLOGIN1,"Last login %s on %s"),
			buf, w->tty);
		else 
		{
			register char *ep = asctime(tmptr);
			printf(MSGSTR(LASTLOGIN1,"Last login %10.10s, %4.4s on %s"), ep, ep+20, w->tty);
		}
		if (*w->host)
			printf(MSGSTR(FROM,"\n    from %s"), w->host);
	} else {
		tmptr = localtime(&w->loginat);
		if (strftime(buf, BUFSIZ, "%a %sD %H:%M", tmptr))		
			printf(MSGSTR(LASTLOGIN2,"Last login %s on %s"), ep, w->tty);
		else
		{
			register char *ep = asctime(tmptr);
			printf(MSGSTR(LASTLOGIN2,"Last login %16.16s on %s"), ep, w->tty);
		}
		if (*w->host)
			printf(MSGSTR(FROM,"\n    from %s"), w->host);
	}
#endif
	putchar('\n');
}

/*
 * decode the information in the gecos field of /etc/passwd
 */
decode(pers)
	register struct person *pers;
{
	char buffer[256];
	register char *bp, *gp, *lp;
	int len;

	pers->realname = 0;
	if (pers->pwd == 0)
		return;
	gp = pers->pwd->pw_gecos;
	bp = buffer;
	if (*gp == ASTERISK)
		gp++;
	while (*gp && *gp != COMMA && *gp != SLASH)		/* name */
		if (*gp == SAMENAME) {
			lp = pers->pwd->pw_name;
			if (islower(*lp))
				*bp++ = toupper(*lp++);
			while (*bp++ = *lp++)
				;
			bp--;
			gp++;
		} else
			*bp++ = *gp++;
	*bp++ = 0;
	if ((len = bp - buffer) > 1)
		pers->realname = strcpy(malloc(len+1), buffer);
	if (*gp)
		pers->site_info = strcpy (malloc (strlen (gp+1)+1), gp+1);
	else
		pers->site_info = (char *) NULL;
}

/*
 * find the last log in of a user by checking the LASTLOG file.
 * the entry is indexed by the uid, so this can only be done if
 * the uid is known (which it isn't in quick mode)
 */

#ifdef LAST_LOGIN
fwopen()
{
	if ((lf = open(LASTLOG, 0)) < 0)
		fprintf(stderr, "finger: %s open error\n", LASTLOG);
}
#endif

#ifdef LAST_LOGIN
findwhen(pers, w)
	register struct person *pers;
	register struct where *w;
{
	struct lastlog ll;
	int i;

	if (lf >= 0) {
		lseek(lf, (long)pers->pwd->pw_uid * sizeof ll, 0);
		if ((i = read(lf, (char *)&ll, sizeof ll)) == sizeof ll) {
			bcopy(ll.ll_line, w->tty, LMAX);
			w->tty[LMAX] = 0;
			bcopy(ll.ll_host, w->host, HMAX);
			w->host[HMAX] = 0;
			w->loginat = ll.ll_time;
		} else {
			if (i != 0)
				fprintf(stderr, "finger: %s read error\n",
					LASTLOG);
			w->tty[0] = 0;
			w->host[0] = 0;
			w->loginat = 0L;
		}
	} else {
		w->tty[0] = 0;
		w->host[0] = 0;
		w->loginat = 0L;
	}
}
#endif

#ifdef LAST_LOGIN
fwclose()
{
	if (lf >= 0)
		close(lf);
}
#endif

/*
 * find the idle time of a user by doing a stat on /dev/tty??,
 * where tty?? has been gotten from utmp, supposedly.
 */
findidle(w)
	register struct where *w;
{
	struct stat ttystatus;
	static char buffer[20] = "/dev/";
	long t;
#define TTYLEN 5

	strcpy(buffer + TTYLEN, w->tty);
	buffer[TTYLEN+LMAX] = 0;
	if (stat(buffer, &ttystatus) < 0) {
		fprintf(stderr, MSGSTR(STATTTY,"finger: Can't stat %s\n"), buffer);
		exit(4);
	}
	time(&t);
	if (t < ttystatus.st_atime)
		w->idletime = 0L;
	else 
		w->idletime = t - ttystatus.st_atime;
	/*
	 * Check to see if the tty device is writable for
	 * the person being fingered. 
	 */
        if (accessx(buffer, W_ACC, ACC_ALL) < 0)
                w->writable = (char) 0;
        else
                w->writable = (char) 1;
}

/*
 * print idle time in short format; this program always prints 4 characters;
 * if the idle time is zero, it prints 4 blanks.
 */
stimeprint(dt)
	long *dt;
{
	register struct tm *delta;

	delta = gmtime(dt);
	if (delta->tm_yday == 0)
		if (delta->tm_hour == 0)
			if (delta->tm_min == 0)
				printf("    ");
			else
				printf("  %2d", delta->tm_min);
		else
			if (delta->tm_hour >= 10)
				printf("%3d:", delta->tm_hour);
			else
				printf("%1d:%02d",
					delta->tm_hour, delta->tm_min);
	else
		printf("%3dd", delta->tm_yday);
}

/*
 * print idle time in long format with care being taken not to pluralize
 * 1 minutes or 1 hours or 1 days.
 * print "prefix" first.
 */
ltimeprint(before, dt, after)
	long *dt;
	char *before, *after;
{
	register struct tm *delta;

	delta = gmtime(dt);
	if (delta->tm_yday == 0 && delta->tm_hour == 0 && delta->tm_min == 0 &&
	    delta->tm_sec <= 10)
		return (0);
	printf("%s", before);
	if (delta->tm_yday >= 10)
		printf(MSGSTR(DAY,"%d days"), delta->tm_yday);
	else if (delta->tm_yday > 0)
		if (delta->tm_yday == 1)
		    if (delta->tm_hour > 1)
		        printf(MSGSTR(DAYHR1,"%d day %d hours"),delta->tm_yday,delta->tm_hour);
		    else
		        printf(MSGSTR(DAYHR2,"%d day %d hour"),delta->tm_yday,delta->tm_hour);
		else
		    if (delta->tm_hour > 1)
		        printf(MSGSTR(DAYHR3,"%d days %d hours"),delta->tm_yday,delta->tm_hour);
		    else
		        printf(MSGSTR(DAYHR4,"%d days %d hour"),delta->tm_yday,delta->tm_hour);
	
	else
		if (delta->tm_hour >= 10)
			printf(MSGSTR(HOUR,"%d hours"), delta->tm_hour);
		else if (delta->tm_hour > 0)
		   	if (delta->tm_hour == 1) 
			   if (delta->tm_min > 1) 
			      printf(MSGSTR(HRMIN1,"%d hour %d minutes"), delta->tm_hour, delta->tm_min);
			   else
			      printf(MSGSTR(HRMIN2,"%d hour %d minute"), delta->tm_hour, delta->tm_min);
			else
			   if (delta->tm_min > 1)
			      printf(MSGSTR(HRMIN3,"%d hours %d minutes"), delta->tm_hour, delta->tm_min);
			   else
			      printf(MSGSTR(HRMIN4,"%d hours %d minute"), delta->tm_hour, delta->tm_min);
		else
			if (delta->tm_min >= 10)
				printf(MSGSTR(MINUTE,"%2d minutes"), delta->tm_min);
			else if (delta->tm_min == 0)
				printf(MSGSTR(SECOND,"%2d seconds"), delta->tm_sec);
			else
			     if (delta->tm_min == 1)
				if (delta->tm_sec > 1)
				  printf(MSGSTR(MINSEC1,"%d minute %d seconds"), delta->tm_min, delta->tm_sec);
				else
				  printf(MSGSTR(MINSEC2,"%d minute %d second"), delta->tm_min, delta->tm_sec);
			     else
				if (delta->tm_sec > 1)
				  printf(MSGSTR(MINSEC3,"%d minutes %d seconds"), delta->tm_min, delta->tm_sec);
				else
				  printf(MSGSTR(MINSEC4,"%d minutes %d second\n"), delta->tm_min, delta->tm_sec);
		
	printf("%s", after);
}

matchcmp(gname, login, given)
	register char *gname;
	char *login;
	char *given;
{
	char buffer[100];
	register char *bp, *lp;
	register c;

	if (*gname == ASTERISK)
		gname++;
	lp = 0;
	bp = buffer;
	for (;;)
		switch (c = *gname++) {
		case SAMENAME:
			for (lp = login; bp < buffer + sizeof buffer
					 && (*bp++ = *lp++);)
				;
			bp--;
			break;
		case ' ':
		case COMMA:
		case '\0':
			*bp = 0;
			if (namecmp(buffer, given))
				return (1);
			if (c == COMMA || c == 0)
				return (0);
			bp = buffer;
			break;
		default:
			if (bp < buffer + sizeof buffer)
				*bp++ = c;
		}
	/*NOTREACHED*/
}

namecmp(name1, name2)
	register char *name1, *name2;
{
	register c1, c2;

	for (;;) {
		c1 = *name1++;
		if (islower(c1))
			c1 = toupper(c1);
		c2 = *name2++;
		if (islower(c2))
			c2 = toupper(c2);
		if (c1 != c2)
			break;
		if (c1 == 0)
			return (1);
	}
	if (!c1) {
		for (name2--; isdigit(*name2); name2++)
			;
		if (*name2 == 0)
			return (0);
	} else if (!c2) {
		for (name1--; isdigit(*name1); name1++)
			;
		if (*name2 == 0)
			return (0);
	}
	return (0);
}

netfinger(name)
	char *name;
{
	char *host;
	char fname[100];
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int s;
	char *rindex();
	register FILE *f;
	register int c;
	register int lastc;

	if (name == NULL)
		return (0);
	host = rindex(name, '@');
	if (host == NULL)
		return (0);
	*host++ = 0;

	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
                if ((hp = gethostbyname (host)) == (struct hostent *) 0) {
                        printf(MSGSTR(NME_NT_FND,
                                            "finger: Unknown host %s\n"), host);
                        return(1);
                }
        }
        else {
		static struct hostent def;
		static struct in_addr defaddr;
		static char *alist[1];
		static char namebuf[128];
		int inet_addr();

		defaddr.s_addr = inet_addr(host);
		if (defaddr.s_addr == -1) {
			printf(MSGSTR(UNKHOST,
				"finger: Address %s misformed\n"), host);
			return (1);
			}
		strcpy(namebuf, host);
		def.h_name = namebuf;
		def.h_addr_list = alist, def.h_addr = (char *)&defaddr;
		def.h_length = sizeof (struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	sp = getservbyname("finger", "tcp");
	if (sp == 0) {
		printf(MSGSTR(UNKSERVICE,"tcp/finger: unknown service\n"));
		return (1);
	}
	sin.sin_family = hp->h_addrtype;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = sp->s_port;
	s = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if (s < 0) {
		perror(MSGSTR(SOCKET,"socket"));
		return (1);
	}
	printf("[%s]\n", hp->h_name);
	fflush(stdout);
	if (connect(s, (char *)&sin, sizeof (sin)) < 0) {
		perror(MSGSTR(CONN,"connect"));
		close(s);
		return (1);
	}
	if (large) write(s, "/W ", 3);
	write(s, name, strlen(name));
	write(s, "\r\n", 2);
	f = fdopen(s, "r");
	while ((c = getc(f)) != EOF) {
		lastc = c;
		putchar(c);
	}
	if (lastc != '\n')
		putchar('\n');
	(void)fclose(f);
	return (1);
}

/*
 * Open filename, print header and then spit out the files contents.
 */
show_text(file_name, header)
        char *file_name, *header;
{
        register int ch, lastc;
        register FILE *fp;

        if ((fp = fopen(file_name, "r")) == NULL)
                return(0);
        (void)printf("%s\n", header);
        while ((ch = getc(fp)) != EOF)
                vputc(lastc = ch);
        if (lastc != '\n')
                (void)putchar('\n');
        (void)fclose(fp);
        return(1);
}

/*
 * Print only "nice characters" to the screen.
 * Must use NLS-friendly functions here.  If non-printable and non-space,
 * then we test to see if it's a ctrl char.  If not, use the M- form
 * from BSD.
 */

vputc(ch)
        register int ch;
{

        if (isprint(ch) || isspace(ch)) {
                (void)putchar(ch);
        } else {
                if (iscntrl(ch)) {
                        printf("^%c",ch == 0177 ? '?' : ch | 0100);
                } else {

                        /* Treat the rest as BSD does */

                        ch = toascii(ch);
                        if (iscntrl(ch))
                                printf("M-^%c", ch == 0177 ? '?' : ch | 0100);
                        else

                                printf("M-%c", ch);
                }
        }
}

