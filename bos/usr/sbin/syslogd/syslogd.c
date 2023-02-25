static char sccsid[] = "@(#)33	1.30.2.4  src/bos/usr/sbin/syslogd/syslogd.c, cmdnet, bos41J, 9519A_all 5/1/95 18:06:04";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: FDMASK
 *		MSGSTR
 *		cfline
 *		cvthname
 *		decode
 *		dosrcpacket
 *		flushmsg
 *		init
 *		logerror
 *		logmsg
 *		main
 *		printline
 *		printsys
 *		sigmask
 *		untty
 *		usage
 *		wallmsg
 *		werrlog
 *
 *   ORIGINS: 26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1989
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximimum line length that can be handled.
 * NLOGS   -- the maximum number of simultaneous log files.
 * DEFUPRI -- the default priority for user messages
 * DEFSPRI -- the default priority for kernel messages
 *
 * Author: Eric Allman
 * extensive changes by Ralph Campbell
 */

#define	NLOGS		20		/* max number of log files */
#define	MAXLINE		1024		/* maximum line length */
#define DEFUPRI		(LOG_USER|LOG_NOTICE)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)
#define MARKCOUNT	10		/* ratio of minor to major marks */
/* This is a prickly pear at 4 oclock in the morning story. */
#define sigmask(m)	(1 << ((m)-1))
#define DELIMS		"\t "	/* Allowable delimiters in syslog.conf */

#include <errno.h>
#include <stdio.h>
/*** #include <utmp.h> *** need to move this until after types.h */
#include <ctype.h>
#include <sys/signal.h>
#include <sysexits.h>
#include <string.h>

#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/msgbuf.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/time.h>
/*** #include <sys/resource.h> ***/
#include <utmp.h> 

#include <netinet/in.h>
#include <netdb.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/access.h>
#include <locale.h>

#include <nl_types.h>
#include "syslogd_msg.h"
nl_catd catd;
#define	MSGSTR(n,s)	catgets(catd, MS_SYSLOGD, n, s)
/*
 * The following includes and declerations are for support of the System
 * Resource Controller (SRC) .
 */
#include <spc.h>
#define SRCSTATNDX 	6
#define SRC_FD 		13
#define SRCMSG     	(sizeof(srcpacket))
static 	struct srcreq srcpacket;
struct	statrep *srcstatus;
short	srcstatsize;
int 	srcreplen;
int 	cont;
struct 	srchdr *srchdr;
char	progname[128];

char	*LogName = "/dev/log";
char	*ConfFile = "/etc/syslog.conf";
char	*PidFile = "/etc/syslog.pid";
char	ctty[] = "/dev/console";
char 	errl[] = "errlog";


#define FDMASK(fd)	(1 << (fd))

#define	dprintf		if (Debug) printf

#define UNAMESZ		8	/* length of a login name */
#define MAXUNAMES	20	/* maximum number of user names */
#define MAXFNAME	200	/* max file pathname length */

#define NOPRI		0x10	/* the "no priority" priority */
#define	LOG_MARK	(LOG_NFACILITIES << 3)	/* mark "facility" */

/*
 * Flags to logmsg().
 */

#define IGN_CONS	0x001	/* don't print on console */
#define SYNC_FILE	0x002	/* do fsync on file after printing */
#define NOCOPY		0x004	/* don't suppress duplicate messages */
#define ADDDATE		0x008	/* add a date to the message */
#define MARK		0x010	/* this message is a mark */

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
	short	f_type;			/* entry type, see below */
	short	f_file;			/* file descriptor */
	time_t	f_time;			/* time this was last written */
	u_char	f_pmask[LOG_NFACILITIES+1];	/* priority mask */
	union {
		char	f_uname[MAXUNAMES][UNAMESZ+1];
		struct {
			char	f_hname[MAXHOSTNAMELEN+1];
			struct sockaddr_in	f_addr;
		} f_forw;		/* forwarding address */
		char	f_fname[MAXFNAME];
	} f_un;
};

/* values for f_type */
#define F_UNUSED	0		/* unused entry */
#define F_FILE		1		/* regular file */
#define F_TTY		2		/* terminal */
#define F_CONSOLE	3		/* console terminal */
#define F_FORW		4		/* remote machine */
#define F_USERS		5		/* list of users */
#define F_WALL		6		/* everyone logged on */
#define F_ERRLOG	7		/* errlog	*/

char	*TypeNames[8] = {
	"UNUSED",	"FILE",		"TTY",		"CONSOLE",
	"FORW",		"USERS",	"WALL",		"ERRLOG"
};

struct filed	Files[NLOGS];

int	Debug;			/* debug flag */
char	LocalHostName[MAXHOSTNAMELEN+1];	/* our hostname */
char	*LocalDomain;		/* our local domain name */
int	InetInuse = 0;		/* non-zero if INET sockets are being used */
int	LogPort;		/* port number for INET connections */
char	PrevLine[MAXLINE + 1];	/* copy of last line to supress repeats */
char	PrevHost[MAXHOSTNAMELEN+1];		/* previous host */
int	PrevFlags;
int	PrevPri;
int	PrevCount = 0;		/* number of times seen */
int	Initialized = 0;	/* set when we have initialized ourselves */
int	MarkInterval = 20;	/* interval between marks in minutes */
int	MarkSeq = 0;		/* mark sequence number */

extern	int errno, sys_nerr;
extern void die(int), domark(int), reapchild(int);
void   init(int);

main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	register char *p;
	int funix, finet, inetm, fklog, klogm, len;
	struct sockaddr_un sun, fromunix;
	struct sockaddr_in sin, frominet;
	FILE *fp;
	char line[MSG_BSIZE + 1];
	struct sigvec sigin;

/* 
 * SRC declerations and code startup.  fd 0 (stdin) must be the SRC socket
 * startup.  fd 0 is duped to a new descriptor so that stdin can be used
 * internally by syslogd - .
 */
	int rc, addrsz;
	struct sockaddr srcaddr;
	int src_exists = TRUE;
	int ch;
	extern int optind;
	extern char *optarg;


	setlocale(LC_ALL, "");
	catd = catopen(MF_SYSLOGD, NL_CAT_LOCALE);
	strcpy(progname,argv[0]);
	addrsz = sizeof(srcaddr);
	if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
		fprintf(stderr, MSGSTR(NE_NOSRC, "%s: continuing without SRC support\n"),progname);
		src_exists = FALSE;
	}
	if (src_exists)
		dup2(0, SRC_FD);

	while ((ch = getopt(argc, argv, "df:m:p:")) != EOF)
		switch((char)ch) {
		case 'd':		/* debug */
			Debug++;
			break;
		case 'f':		/* configuration file */
			ConfFile = optarg;
			break;
		case 'm':		/* mark interval */
			MarkInterval = atoi(optarg);
			break;
		case 'p':		/* path */
			LogName = optarg;
			break;
		case '?':
		default:
			usage();
		}
	if (argc -= optind)
		usage();

	if (!Debug) {
/* 
 * fork only if SRC is not in effect.
 */
		if (!src_exists)
 			if (fork())
 				exit(0);

		for (i = 0; i < 10; i++)
			(void) close(i);
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		untty();
	} else
		setlinebuf(stdout);

	(void) gethostname(LocalHostName, sizeof LocalHostName);
	if (p = index(LocalHostName, '.')) {
		*p++ = '\0';
		LocalDomain = p;
	}
	else
		LocalDomain = "";
	sigin.sv_handler = die;
	if ((sigvec(SIGTERM,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGTERM, "sigvec failure on SIGTERM errno = %d\n "),errno);
	sigin.sv_handler = Debug ? die : SIG_IGN;
	if ((sigvec(SIGINT,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGINT, "sigvec failure on SIGINT errno = %d\n "),errno);
	if ((sigvec(SIGQUIT,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGQUIT, "sigvec failure on SIGQUIT errno = %d\n "),errno);
	sigin.sv_handler = reapchild;
	if ((sigvec(SIGCLD,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGCHLD, "sigvec failure on SIGCHLD errno = %d\n "),errno);
	sigin.sv_handler = domark;
	if ((sigvec(SIGALRM,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGALRM, "sigvec failure on SIGALRM errno = %d\n "),errno);
	sigin.sv_handler = SIG_IGN;
	if ((sigvec(SIGPIPE,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGPIPE, "sigvec failure on SIGPIPE errno = %d\n "),errno);
	(void) alarm(MarkInterval * 60 / MARKCOUNT);
	(void) unlink(LogName);

	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	(void) strncpy(sun.sun_path, LogName, sizeof sun.sun_path);
	funix = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (funix < 0 ||
	    bind(funix, (struct sockaddr *) &sun, SUN_LEN(&sun)) < 0 ||
	    acl_set(LogName, R_ACC | W_ACC, R_ACC | W_ACC, R_ACC | W_ACC) < 0) {
		(void) sprintf(line, "cannot create %s", LogName);
		logerror(line);
		dprintf(MSGSTR(ND_CREATE, "cannot create %s (%d)\n"), LogName, errno);
		die(0);
	}
	sin.sin_family = AF_INET;
	finet = socket(AF_INET, SOCK_DGRAM, 0);
	if (finet >= 0) {
		struct servent *sp;

		sp = getservbyname("syslog", "udp");
		if (sp == NULL) {
			errno = 0;
			logerror("syslog/udp: unknown service");
			die(0);
		}
		sin.sin_port = sp->s_port;
		sin.sin_addr.s_addr = INADDR_ANY;
		LogPort = sp->s_port;
		if (bind(finet, &sin, sizeof(sin)) < 0) {
			logerror("bind");
			if (!Debug)
				die(0);
		} else {
			inetm = FDMASK(finet);
			InetInuse = 1;
		}
	}
	if ((fklog = open("/dev/klog", O_RDONLY)) >= 0)
		klogm = FDMASK(fklog);
	else {
		dprintf(MSGSTR(ND_KLOG, "can't open /dev/klog (%d)\n"), errno);
		klogm = 0;
	}
	/* tuck my process id away */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}

	dprintf(MSGSTR(ND_RUNNING, "off & running....\n"));

	init(0);
	sigin.sv_handler = init;
	if ((sigvec(SIGHUP,&sigin,(struct sigvec *) 0)) == EOF)
		dprintf(MSGSTR(ND_SIGHUP, "sigvec failure on SIGHUP errno = %d\n "),errno);

	for (;;) {
		register FILE *cf;
	    	int onoff, confentries, x;
		int nfds, readfds;

		cont=END;
		addrsz=sizeof(srcaddr);
		errno = 0;
		if (src_exists)
		    readfds = FDMASK(funix) | inetm | klogm | FDMASK(SRC_FD);
		else
		    readfds = FDMASK(funix) | inetm | klogm;
		dprintf(MSGSTR(ND_RFDS, "readfds = %#x\n"), readfds, funix, finet, fklog);
		nfds = select(20, (fd_set *) &readfds, (fd_set *) NULL,
			      (fd_set *) NULL, (struct timeval *) NULL);
		dprintf(MSGSTR(ND_GOTMSG, "got a message (%d, %#x)\n"), nfds, readfds);
		if (nfds == 0)
		    continue;
		if (nfds < 0) {
		    if (errno != EINTR)
			    logerror("select");
		    continue;
		}
		if (readfds & klogm) {
		    i = read(fklog, line, sizeof(line) - 1);
		    if (i > 0) {
			    line[i] = '\0';
			    printsys(line);
		    } else if (i < 0 && errno != EINTR) {
			    logerror("klog");
			    fklog = -1;
			    klogm = 0;
		    }
		}
		if (readfds & FDMASK(funix)) {
		    len = sizeof fromunix;
		    i = recvfrom(funix, line, MAXLINE, 0,
				 (struct sockaddr *) &fromunix, &len);
		    if (i > 0) {
			    line[i] = '\0';
			    printline(LocalHostName, line);
		    } else if (i < 0 && errno != EINTR)
			    logerror("recvfrom unix");
		}
		if (readfds & inetm) {
		    len = sizeof frominet;
		    i = recvfrom(finet, line, MAXLINE, 0, &frominet, &len);
		    if (i > 0) {
			    extern char *cvthname();

			    line[i] = '\0';
			    printline(cvthname(&frominet), line);
		    } else if (i < 0 && errno != EINTR)
			    logerror("recvfrom inet");
		} 
		if (src_exists && (readfds & FDMASK(SRC_FD))) {
		    rc = recvfrom(SRC_FD,&srcpacket,SRCMSG,0,&srcaddr,&addrsz);
		    if (rc < 0)
			if (errno != EINTR) {
			    sprintf(line,"%s: ERROR: '%d' recvfrom\n",
				progname,errno); 
			    logerror(line);
			    die(1);
			} else  /* punt on short read */
			    continue;

		    /* we should check for a valid src packet here instead
		       of simply assuming that it is */

		    switch(srcpacket.subreq.action) {
			case START:
				dosrcpacket(SRC_SUBMSG,"ERROR: syslogd does not support this option.\n",sizeof(struct srcrep));
				break;
			case STOP:
				if (srcpacket.subreq.object == SUBSYSTEM) {
					dosrcpacket(SRC_OK,NULL,sizeof(struct srcrep));
					die(0);
				} else
					dosrcpacket(SRC_SUBMSG,"ERROR: syslogd does not support this option.\n",sizeof(struct srcrep));
				break;
			case REFRESH:
				init(0);
				dosrcpacket(SRC_OK,NULL,sizeof(struct srcrep));
				break;
			case STATUS:
			    	srcreplen = srcstatsize = sizeof(struct srchdr) + (sizeof(struct statcode) * SRCSTATNDX);
			    	srcstatus = (struct statrep *)malloc(srcstatsize);
			    	memset(srcstatus,'\0',srcreplen);
				cont = NEWREQUEST;
			    	srcstat("","",getpid(),&srcstatsize,srcstatus, &cont);
			    	srchdr = srcrrqs(&srcpacket);
				x = 3;
				confentries = TRUE;
				if ((cf = fopen(ConfFile, "r")) == NULL) {
					dprintf(MSGSTR(ND_BADOPEN, "cannot open %s\n"), ConfFile);
					cfline("*.ERR\t/dev/console", &Files[0]);
					cfline("*.PANIC\t*", &Files[1]);
					confentries = FALSE;
				}
				if (confentries) {
				    while(fgets(line,sizeof(line),cf) != NULL) {
					if (line[0] == '\n' || line[0] == '#')
						continue;
					if (p = index(line, '\n'))
						*p = '\0';
					if (x < SRCSTATNDX) {
						sprintf(srcstatus->statcode[x].objname,"%-16s","Syslogd Config");
						sprintf(srcstatus->statcode[x].objtext,"%s",line);
						x++;
					} else {
			    			srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
						x = 0;
			    			memset(srcstatus,'\0',srcreplen);
					}
				    }
				    (void) fclose(cf);
				}
				if (x > 0)
			    		srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
			    	srcsrpy(srchdr,srcstatus,sizeof(struct srchdr),END);
			    	free(srcstatus);
	    		    	break;
			case TRACE:
				if (srcpacket.subreq.object == SUBSYSTEM) { 
					onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
					if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0) {
						sprintf(line, "setsockopt SO_DEBUG: %m");
						logerror(line);
						close(finet);
						die(1);
					}
					if (setsockopt(funix, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0) {
						sprintf(line, "setsockopt SO_DEBUG: %m");
						logerror(line);
						close(funix);
						die(1);
					}
					dosrcpacket(SRC_OK,NULL,sizeof(struct srcrep)); 
				} else
					dosrcpacket(SRC_SUBMSG,"ERROR: syslogd does not support this option.\n",sizeof(struct srcrep)); 
	    		    	break;
			default:
				dosrcpacket(SRC_SUBICMD,NULL,sizeof(struct srcrep));
				break;

		     }
		}
	}
}
		
/* 
 * SRC packet processing - .
 */
int dosrcpacket(msgno, txt, len)
	int msgno;
	char *txt;
	int len;
{
	struct srcrep reply;

	reply.svrreply.rtncode = msgno;
	strcpy(reply.svrreply.objname, "syslogd");
	strcpy(reply.svrreply.rtnmsg, txt);
	srchdr = srcrrqs(&srcpacket);
	srcsrpy(srchdr, &reply, len, cont);
}

usage()
{
	fprintf(stderr, MSGSTR(N_USAGE, "usage: syslogd [-d] [-mmarkinterval] [-ppath] [-fconffile]\n"));
	exit(1);
}
#ifndef TIOCNOTTY
#define TIOCNOTTY ((('t')<<8)|113)
#endif

untty()
{
	int i;

	if (!Debug) {
		i = open("/dev/tty", O_RDWR);
		if (i >= 0) {
			(void) ioctl(i, (int) TIOCNOTTY, (char *)0);
			(void) close(i);
		}
	}
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

printline(hname, msg)
	char *hname;
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri;

	/* test for special codes */
	pri = DEFUPRI;
	p = msg;
	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
			pri = 10 * pri + (*p - '0');
		if (*p == '>')
			++p;
		if (pri <= 0 || pri >= (LOG_NFACILITIES << 3))
			pri = DEFUPRI;
	}

	q = line;

	while ((c = *p++) != '\0' && q < &line[sizeof(line) - 1]) {
		if (c == '\n')  /* convert to blank */
			*q++ = ' ';
		else if (iscntrl(c)) {
			*q++ = '^';
			*q++ = c ^ 0100;
		} else
			*q++ = c;
	}
	if (*(q-1) == ' ')  /* truncate trailing blank */
	    --q;
	*q = '\0';

	logmsg(pri, line, hname, 0);
}

/*
 * Take a raw input line from /dev/klog, split and format similar to syslog().
 */

printsys(msg)
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri, flags;
	char *lp;
	time_t now;

	(void) time(&now);
	(void) sprintf(line, "%.15s vmunix: ", ctime(&now) + 4);
	lp = line + strlen(line);
	for (p = msg; *p != '\0'; ) {
		flags = SYNC_FILE;	/* fsync file after write */
		pri = DEFSPRI;
		if (*p == '<') {
			pri = 0;
			while (isdigit(*++p))
				pri = 10 * pri + (*p - '0');
			if (*p == '>')
				++p;
			if (pri <= 0 || pri >= (LOG_NFACILITIES << 3))
				pri = DEFSPRI;
		} else {
			/* kernel printf's come out on console */
			flags |= IGN_CONS;
		}
		q = lp;
		while (*p != '\0' && (c = *p++) != '\n' &&
		    q < &line[MAXLINE])
			*q++ = c;
		*q = '\0';
		logmsg(pri, line, LocalHostName, flags);
	}
}

/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */

logmsg(pri, msg, from, flags)
	int pri;
	char *msg, *from;
	int flags;
{
	register struct filed *f;
	register int l;
	int fac, prilev;
	time_t now;
	int omask;
	struct iovec iov[6];
	register struct iovec *v = iov;
	char line[MAXLINE + 1];

	dprintf(MSGSTR(ND_LOGMSG, "logmsg: pri %o, flags %x, from %s, msg %s\n"), pri, flags, from, msg);

	omask = sigblock(sigmask(SIGHUP)|sigmask(SIGALRM));

	/*
	 * Check to see if msg has a date field at front.
	 */
	if (strlen(msg) < 16 || msg[3] != ' ' || msg[6] != ' ' ||
	    msg[9] != ':' || msg[12] != ':' || msg[15] != ' ')
		flags |= ADDDATE;

	if (!(flags & NOCOPY)) {
		if (flags & (ADDDATE|MARK))
			flushmsg();
		else if ( (!strcmp(msg + 16, PrevLine + 16)) &&
			   PrevPri == pri)
		{
			/* we found a match, update the time */
			(void) strncpy(PrevLine, msg, 15);
			PrevCount++;
			(void) sigsetmask(omask);
			return;
		} else {
			/* new line, save it */
			flushmsg();
			(void) strcpy(PrevLine, msg);
			(void) strcpy(PrevHost, from);
			PrevFlags = flags;
			PrevPri = pri;
		}
	}

	(void) time(&now);
	if (flags & ADDDATE)
		v->iov_base = (caddr_t) (ctime(&now) + 4);
	else
		v->iov_base = msg;
	v->iov_len = 15;
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	v->iov_base = from;
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	if (flags & ADDDATE)
		v->iov_base = msg;
	else
		v->iov_base = msg + 16;
	v->iov_len = strlen(v->iov_base);
	v++;

	/* extract facility and priority level */
	fac = (pri & LOG_FACMASK) >> 3;
	if (flags & MARK)
		fac = LOG_NFACILITIES;
	prilev = pri & LOG_PRIMASK;

	/* log the message to the particular outputs */
	if (!Initialized) {
		int cfd = open(ctty, O_WRONLY);

		if (cfd >= 0) {
			v->iov_base = "\r\n";
			v->iov_len = 2;
			(void) writev(cfd, iov, 6);
			(void) close(cfd);
		}
		untty();
		(void) sigsetmask(omask);
		return;
	}
	for (f = Files; f < &Files[NLOGS]; f++) {
		/* skip messages that are incorrect priority */
		if (f->f_pmask[fac] < prilev || f->f_pmask[fac] == NOPRI)
			continue;

		/* don't output marks to recently written files */
		if ((flags & MARK) && (now - f->f_time) < (MarkInterval * 60 / 2))
			continue;

		dprintf(MSGSTR(ND_LOGGING, "Logging to %s"), TypeNames[f->f_type]);
		f->f_time = now;
		switch (f->f_type) {
		case F_UNUSED:
			dprintf("\n");
			break;

		case F_FORW:
			dprintf(" %s\n", f->f_un.f_forw.f_hname);
			(void) sprintf(line, "<%d>%.15s %s", pri,
				iov[0].iov_base, iov[4].iov_base);
			l = strlen(line);
			if (l > MAXLINE)
				l = MAXLINE;
			if (sendto(f->f_file, line, l, 0,
			    &f->f_un.f_forw.f_addr,
			    sizeof f->f_un.f_forw.f_addr) != l) {
				int e = errno;
				(void) close(f->f_file);
				f->f_type = F_UNUSED;
				errno = e;
				logerror("sendto");
			}
			break;

		case F_CONSOLE:
			if (flags & IGN_CONS) {
				dprintf(MSGSTR(N_IGN, " (ignored)\n"));
				break;
			}

		case F_TTY:
		case F_FILE:
			dprintf(" %s\n", f->f_un.f_fname);
			if (f->f_type != F_FILE) {
				v->iov_base = "\r\n";
				v->iov_len = 2;
			} else {
				v->iov_base = "\n";
				v->iov_len = 1;
			}
			if (writev(f->f_file, iov, 6) < 0) {
				int e = errno;
				(void) close(f->f_file);
				/*
				 * Check for EBADF on TTY's due to vhangup() XXX
				 */
				if (e == EBADF && f->f_type != F_FILE) {
					f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND);
					if (f->f_file < 0) {
						f->f_type = F_UNUSED;
						logerror(f->f_un.f_fname);
					}
				} else {
					f->f_type = F_UNUSED;
					errno = e;
					logerror(f->f_un.f_fname);
				}
			} else if (flags & SYNC_FILE)
				(void) fsync(f->f_file);
			break;

		case F_USERS:
		case F_WALL:
			dprintf("\n");
			v->iov_base = "\r\n";
			v->iov_len = 2;
			wallmsg(f, iov, omask);
			break;

		case F_ERRLOG:
			dprintf ("\n");
			(void) sprintf (line, "<%d>%.15s %s", pri,
				iov[0].iov_base, iov[4].iov_base);
			l = strlen(line);
			if (l > MAXLINE)
				l = MAXLINE;
			werrlog (line, l);
			break;
		}
	}

	(void) sigsetmask(omask);
}


/*
 *  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 */

wallmsg(f, iov, s_mask)
	register struct filed *f;
	struct iovec *iov;
	int s_mask;
{
	register char *p;
	register int i;
	int ttyf, len;
	FILE *uf;
	static int reenter = 0;
	struct utmp ut;
	struct sigvec sigin;
	time_t now;
	char greetings[200];

	if (reenter++)
		return;

	/* open the user login file */
	if ((uf = fopen("/etc/utmp", "r")) == NULL) {
		logerror("/etc/utmp");
		reenter = 0;
		return;
	}

	(void) time(&now);
	(void) sprintf(greetings,
	    "\r\n\7Message from syslogd@%s at %s ...\r\n",
		iov[2].iov_base, ctime(&now));
	len = strlen(greetings);

	/* scan the user login file */
	while (fread((char *) &ut, sizeof ut, 1, uf) == 1) {
		/* is this slot used? */
		if (ut.ut_name[0] == '\0')
			continue;

		/* should we send the message to this user? */
		if (f->f_type == F_USERS) {
			for (i = 0; i < MAXUNAMES; i++) {
				if (!f->f_un.f_uname[i][0]) {
					i = MAXUNAMES;
					break;
				}
				if (strncmp(f->f_un.f_uname[i], ut.ut_name,
				    UNAMESZ) == 0)
					break;
			}
			if (i >= MAXUNAMES)
				continue;
		}

		/* compute the device name */
		p = "/dev/12345678";
		/*** OBSELETE strcpyn(&p[5], ut.ut_line, UNAMESZ); ***/
		strncpy(&p[5], ut.ut_line, UNAMESZ);

		/*
		 * Might as well fork instead of using nonblocking I/O
		 * and doing notty().
		 */
		if (fork() == 0) {
			if (f->f_type == F_WALL) {
				iov[0].iov_base = greetings;
				iov[0].iov_len = len;
				iov[1].iov_len = 0;
			}
			sigin.sv_handler = SIG_DFL;
			if ((sigvec(SIGALRM,&sigin,(struct sigvec *) 0)) == EOF)
				dprintf(MSGSTR(ND_SIGALRM, "sigvec failure on SIGALRM errno = %d\n "),errno);
			(void) sigsetmask(s_mask);
			(void) alarm(30);
			/* open the terminal */
			ttyf = open(p, O_WRONLY);
			if (ttyf >= 0)
				(void) writev(ttyf, iov, 6);
			exit(0);
		}
	}
	/* close the user login file */
	(void) fclose(uf);
	reenter = 0;
}

/* werrlog() was added to support errlog as one of the	*/
/* destinations for syslog messages.			*/

werrlog(buf, len)
char *buf;
int len;
{
	struct {
		struct err_rec0 err_rec;
		char detail_data[ERR_REC_MAX];
	} e;

	e.err_rec.error_id = ERRID_SYSLOG;
	strcpy (e.err_rec.resource_name, "syslog");
	strncpy (e.detail_data, buf, len);

	errlog(&e, sizeof(e.err_rec) + len);
}

void reapchild(int s)
{
	union wait status;

	while (wait3(&status, WNOHANG, (struct rusage *) NULL) > 0)
		;
}

/*
 * Return a printable representation of a host address.
 */
char *
cvthname(f)
	struct sockaddr_in *f;
{
	struct hostent *hp;
	register char *p;
	extern char *inet_ntoa();

	dprintf(MSGSTR(ND_CVTHNAME, "cvthname(%s)\n"), inet_ntoa(f->sin_addr));

	if (f->sin_family != AF_INET) {
		dprintf(MSGSTR(ND_MALFORM, "Malformed from address\n"));
		return ("???");
	}
	hp = gethostbyaddr(&f->sin_addr, sizeof(struct in_addr), f->sin_family);
	if (hp == 0) {
		dprintf(MSGSTR(ND_HOST, "Host name for your address (%s) unknown\n"),
			inet_ntoa(f->sin_addr));
		return (inet_ntoa(f->sin_addr));
	}
	if ((p = index(hp->h_name, '.')) && strcmp(p + 1, LocalDomain) == 0)
		*p = '\0';
	return (hp->h_name);
}

void domark(int s)
{
	int pri;

	if ((++MarkSeq % MARKCOUNT) == 0)
		logmsg(LOG_INFO, "-- MARK --", LocalHostName, ADDDATE|MARK);
	else
		flushmsg();
	alarm(MarkInterval * 60 / MARKCOUNT);
}

flushmsg()
{
	if (PrevCount == 0)
		return;
	if (PrevCount > 1)
		(void) sprintf(PrevLine+16, "last message repeated %d times",
			PrevCount);
	PrevCount = 0;
	logmsg(PrevPri, PrevLine, PrevHost, PrevFlags|NOCOPY);
	PrevLine[0] = '\0';
}

/*
 * Print syslogd errors some place.
 */
logerror(type)
	char *type;
{
	char buf[100];

	if (errno == 0)
		(void) sprintf(buf, "syslogd: %s", type);
	else if ((unsigned) errno > sys_nerr)
		(void) sprintf(buf, "syslogd: %s: error %d", type, errno);
	else
		(void) sprintf(buf, "syslogd: %s: errno = %d", type, errno);
	errno = 0;
	dprintf("%s\n", buf);
	logmsg(LOG_SYSLOG|LOG_ERR, buf, LocalHostName, ADDDATE);
}

void die(int sig)
{
	char buf[100];

	if (sig) {
		dprintf(MSGSTR(ND_DOWN, "syslogd: going down on signal %d\n"), sig);
		flushmsg();
		(void) sprintf(buf, "going down on signal %d", sig);
		logerror(buf);
	}
	(void) unlink(LogName);
	exit(0);
}

/*
 *  INIT -- Initialize syslogd from configuration table
 */

void init(int s)
{
	register int i;
	register FILE *cf;
	register struct filed *f;
	register char *p;
	char cline[BUFSIZ];

	dprintf(MSGSTR(ND_INIT, "init\n"));

	/* flush any pending output */
	flushmsg();

	/*
	 *  Close all open log files.
	 */
	for (f = Files; f < &Files[NLOGS]; f++) {
		if (f->f_type == F_FILE || f->f_type == F_TTY)
			(void) close(f->f_file);
		f->f_type = F_UNUSED;
	}

	/* open the configuration file */
	if ((cf = fopen(ConfFile, "r")) == NULL) {
		dprintf(MSGSTR(ND_BADOPEN, "cannot open %s\n"), ConfFile);
		cfline("*.ERR\t/dev/console", &Files[0]);
		cfline("*.PANIC\t*", &Files[1]);
		return;
	}

	/*
	 *  Foreach line in the conf table, open that file.
	 */
	f = Files;
	while (fgets(cline, sizeof cline, cf) != NULL && f < &Files[NLOGS]) {
		/* check for end-of-section */
		if (cline[0] == '\n' || cline[0] == '#')
			continue;

		/* strip off newline character */
		p = index(cline, '\n');
		if (p)
			*p = '\0';

		cfline(cline, f++);
	}

	/* close the configuration file */
	(void) fclose(cf);

	Initialized = 1;

	if (Debug) {
		for (f = Files; f < &Files[NLOGS]; f++) {
			for (i = 0; i <= LOG_NFACILITIES; i++)
				if (f->f_pmask[i] == NOPRI)
					printf("X ");
				else
					printf("%d ", f->f_pmask[i]);
			printf("%s: ", TypeNames[f->f_type]);
			switch (f->f_type) {
			case F_ERRLOG:
				printf ("%s", errl);
				break;

			case F_FILE:
			case F_TTY:
			case F_CONSOLE:
				printf("%s", f->f_un.f_fname);
				break;

			case F_FORW:
				printf("%s", f->f_un.f_forw.f_hname);
				break;

			case F_USERS:
				for (i = 0; i < MAXUNAMES && *f->f_un.f_uname[i]; i++)
					printf("%s, ", f->f_un.f_uname[i]);
				break;
			}
			printf("\n");
		}
	}

	logmsg(LOG_SYSLOG|LOG_INFO, "syslogd: restart", LocalHostName, ADDDATE);
	dprintf(MSGSTR(ND_RESTART, "syslogd: restarted\n"));
}

/*
 * Crack a configuration file line
 */

struct code {
	char	*c_name;
	int	c_val;
};

struct code	PriNames[] = {
	"panic",	LOG_EMERG,
	"emerg",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"err",		LOG_ERR,
	"error",	LOG_ERR,
	"warn",		LOG_WARNING,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	"none",		NOPRI,
	NULL,		-1
};

struct code	FacNames[] = {
	"kern",		LOG_KERN,
	"user",		LOG_USER,
	"mail",		LOG_MAIL,
	"daemon",	LOG_DAEMON,
	"auth",		LOG_AUTH,
	"security",	LOG_AUTH,
	"mark",		LOG_MARK,
	"news",		LOG_NEWS,
	"syslog",	LOG_SYSLOG,
	"uucp",		LOG_UUCP,
	"lpr",		LOG_LPR,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1
};

cfline(line, f)
	char *line;
	register struct filed *f;
{
	register char *p;
	register char *q;
	register int i;
	char *bp;
	int pri;
	struct hostent *hp;
	char buf[MAXLINE];

	dprintf(MSGSTR(ND_CFLINE, "cfline(%s)\n"), line);

	/* clear out file entry */
	bzero((char *) f, sizeof *f);
	for (i = 0; i <= LOG_NFACILITIES; i++)
		f->f_pmask[i] = NOPRI;

	/* skip space or tab in the begining of the line */
	while(*line == ' ' || *line == '\t') line++ ;

	/* scan through the list of selectors */
	for (p = line; *p && !index(DELIMS, *p);) {

		/* find the end of this facility name list */
		for (q = p; *q && !index(DELIMS, *q) && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !index("\t,; ", *q); )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		while (index(",;", *q))
			q++;

		/* decode priority name */
		pri = decode(buf, PriNames);
		if (pri < 0) {
			char xbuf[200];

			(void) sprintf(xbuf, "unknown priority name \"%s\"", buf);
			logerror(xbuf);
			return;
		}

		/* scan facilities */
		while (*p && !index("\t.; ", *p)) {
			int i;

			for (bp = buf; *p && !index("\t,;. ", *p); )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*')
				for (i = 0; i < LOG_NFACILITIES; i++)
					f->f_pmask[i] = pri;
			else {
				i = decode(buf, FacNames);
				if (i < 0) {
					char xbuf[200];

					(void) sprintf(xbuf, "unknown facility name \"%s\"", buf);
					logerror(xbuf);
					return;
				}
				f->f_pmask[i >> 3] = pri;
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	/* skip to action part */
	while (index(DELIMS, *p))
		p++;

	switch (*p)
	{
	case '@':
		if (!InetInuse)
			break;
		(void) strcpy(f->f_un.f_forw.f_hname, ++p);
		/* see if it's a name or an address */
                if (!isinet_addr(p)) {
                        if ((hp = gethostbyname(p))==(struct hostent *) 0) {
                                fprintf(stderr,MSGSTR(NME_NT_FND,
                                       "syslogd: host name %s NOT FOUND\n"), p);
                        break;
                        }
                }
                else {
                    if ((f->f_un.f_forw.f_addr.sin_addr.s_addr = inet_addr(p))
                                == -1) {
                                fprintf(stderr, MSGSTR(BADADDR,
                                       "syslogd: %s: address misformed\n"), p);
                                break;
                                }
                }
		bzero((char *) &f->f_un.f_forw.f_addr,
			 sizeof f->f_un.f_forw.f_addr);
		f->f_un.f_forw.f_addr.sin_family = AF_INET;
		f->f_un.f_forw.f_addr.sin_port = LogPort;
		bcopy(hp->h_addr, (char *) &f->f_un.f_forw.f_addr.sin_addr, hp->h_length);
		f->f_file = socket(AF_INET, SOCK_DGRAM, 0);
		if (f->f_file < 0) {
			logerror("socket");
			break;
		}
		f->f_type = F_FORW;
		break;

	case '/':
		(void) strcpy(f->f_un.f_fname, p);
		if ((f->f_file = open(p, O_WRONLY|O_APPEND)) < 0) {
			logerror(p);
			break;
		}
		if (isatty(f->f_file)) {
			f->f_type = F_TTY;
			untty();
		}
		else
			f->f_type = F_FILE;
		if (strcmp(p, ctty) == 0)
			f->f_type = F_CONSOLE;
		break;

	case '*':
		f->f_type = F_WALL;
		break;

	case 'e':
		if (strcmp(p, errl) == 0) {
			f->f_type = F_ERRLOG;
			break;
		}

	default:
		for (i = 0; i < MAXUNAMES && *p; i++) {
			for (q = p; *q && *q != ','; )
				q++;
			(void) strncpy(f->f_un.f_uname[i], p, UNAMESZ);
			if ((q - p) > UNAMESZ)
				f->f_un.f_uname[i][UNAMESZ] = '\0';
			else
				f->f_un.f_uname[i][q - p] = '\0';
			while (*q == ',' || *q == ' ')
				q++;
			p = q;
		}
		f->f_type = F_USERS;
		break;
	}
}


/*
 *  Decode a symbolic name to a numeric value
 */

decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[40];

	if (isdigit(*name))
		return (atoi(name));

	(void) strcpy(buf, name);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);

	return (-1);
}
