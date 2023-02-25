static char sccsid[] = "@(#)19  1.46  src/tcpip/usr/bin/rcp/rcp.c, tcprcmds, tcpip41J, 9516B_all 4/21/95 13:55:45";
/* 
 * COMPONENT_NAME: TCPIP rcp.c
 * 
 * FUNCTIONS: MSGSTR, Mrcp, SYSERR, allocbuf, allostr, colon, error, 
 *            expandpath, ga, localhostp, lostconn, okname, parsearg, 
 *            rcp_local, rcp_remote, recv_remote, response, rsource, 
 *            send_remote, sink, source, susystem, syserr, usage, 
 *            verifydir 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 The Regents of the University of California.
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
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "rcp.c	5.11 (Berkeley) 9/22/88";
#endif  not lint */

/*
 * rcp
 */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <netinet/in.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>

#include <string.h>	
#include <sys/dir.h>
extern char	*sys_errlist[];

#include <nl_types.h>
#include "rcp_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_RCP, n, s) 

/* These 2 defines for sys_errlist[] messages */
#define MF_LIBC     "libc.cat"
#define MS_LIBC     1
char *allostr();	/* local routine to malloc a string */
char *syserr(err)
	int err;
{
	char *tmp, *tmp1;
	nl_catd catd2;

	catd2 = catopen(MF_LIBC,NL_CAT_LOCALE);
	tmp1 = catgets(catd2,MS_LIBC,err,sys_errlist[err]);
	tmp = allostr(strlen(tmp1)+1),
	strcpy(tmp, tmp1);
	catclose(catd2);
	return(tmp);
}
#define SYSERR(x)	syserr(x)


#define RSH   "/usr/bin/rsh"

int	rem;
char	*expandpath(), *colon();
int	errs;
int	lostconn();
extern int	errno;
int	iamremote, targetshouldbedirectory;
int	iamrecursive;
int	pflag;
struct	passwd *getpwuid();
int	userid;
int	port;

struct buffer {
	int	cnt;
	char	*buf;
} *allocbuf();

/*VARARGS*/
int	error();

#define	ga()	 	(void) write(rem, "", 1)
#include <locale.h>
#include <stdlib.h>

main(argc, argv)
	int argc;
	char **argv;
{
	char *username, *suser, *tuser, *thost, *tpath, *shost, *spath;
	char *arg1, *arg2;
	int i, opt;
	char buf[BUFSIZ], cmd[16];
	struct servent *sp;
	struct passwd *pwd;
	extern int optind;

	setlocale(LC_ALL,"");
	catd = catopen(MF_RCP,NL_CAT_LOCALE);

	/* start logging to syslog */
        openlog("rcp", LOG_PID, LOG_USER);
	sp = getservbyname("shell", "tcp");
	if (sp == NULL) {
		fprintf(stderr, MSGSTR(UNKSERV, "rcp: shell/tcp: unknown service\n")); /*MSG*/
		exit(1);
	}
	port = sp->s_port;
	pwd = getpwuid(userid = getuid());
	if (pwd == 0) {
		fprintf(stderr, MSGSTR(IDERR, "who are you?\n")); /*MSG*/
		exit(1);
	}
	username = pwd->pw_name;

	while ((opt = getopt (argc, argv, "rpdft")) != EOF) {
		switch (opt) {

		    case 'r':
			iamrecursive++;
			break;

		    case 'p':		/* preserve mtimes and atimes */
			pflag++;
			break;

		    /* The rest of these are not for users. */
		    case 'd':
			targetshouldbedirectory = 1;
			break;

		    case 'f':		/* "from" */
			iamremote = 1;
			(void) response();
			(void) setuid(userid);
			source(argc - optind, argv + optind);
			exit(errs);

		    case 't':		/* "to" */
			iamremote = 1;
			(void) setuid(userid);
			sink(argc - optind, argv + optind);
			exit(errs);

		    default:
			usage();
		}
	}

        /* Adjust & Check arglist */
        argc -= optind;
        argv += optind;
        if (argc < 2)
        {
                usage();
                exit(1);
        }

        /* Initialize for remote transfers */
        rem = -1;
        if (argc > 2) targetshouldbedirectory = 1;
        sprintf(cmd, "rcp%s%s%s",
            iamrecursive ? " -r" : "", pflag ? " -p" : "",
            targetshouldbedirectory ? " -d" : "");
        (void) signal(SIGPIPE, lostconn);

        /* Determine identity of target */
        parsearg(argv[argc - 1],&tuser,&thost,&tpath);
        if (!tpath) tpath = ".";
        if (tuser && !okname(tuser)) exit(1);

        /* Dispatch each source file: */
        for (i=0; (i < argc -1) ;i++)
        {       /* Indentify source file */
                parsearg(argv[i],&suser,&shost,&spath);
                if (!spath) spath = ".";
                if (suser && !okname(suser)) continue;

                /* Dispatch one of four ways:
                 *   remote to remote
                 *   local  to remote
                 *   remote to local
                 *   local  to local
                 */
                if (!localhostp(thost))
                   if (!localhostp(shost))
                        rcp_remote(buf,cmd,
                                suser,shost,spath,tuser,thost,tpath);
                   else
                        send_remote(buf,cmd,username,port,
                                suser,spath,tuser,thost,tpath);
                else
                {   if (targetshouldbedirectory) verifydir(tpath);
                    if (!localhostp(shost)) {
                        recv_remote(buf,cmd,username,port,
                                suser,shost,spath,tuser,tpath);
			}
                    else
                        rcp_local(buf,suser,spath,tuser,tpath);
                }
        }
        return(errs);
}

/*
 * NAME:        rcp_remote
 *
 * FUNCTION:    Send from remote host to remote host:
 *              Use the submit function (susystem()) to relocate this
 *              rcp onto the source's host and convert to send_remote()
 *              at the remote source.
 */

rcp_remote(buf,cmd,suser,shost,spath,tuser,thost,tpath)
        char    *buf,                   /* command buffer */
                *cmd,                   /* command prefix */
                *suser, *shost, *spath, /* source user,host,path */
                *tuser, *thost, *tpath; /* target user,host,path */

{       if (suser)
                /* User name present for source */
                sprintf(buf, "%s %s -l %s -n %s %s '%s%s%s:%s'",
                    RSH, shost, suser, cmd, spath,
                    tuser, tuser ? "@" : "", thost, tpath);
        else
                /* No user name present for source */
                sprintf(buf, "%s %s -n %s %s '%s%s%s:%s'",
                    RSH, shost, cmd, spath,
                    tuser, tuser ? "@" : "", thost, tpath);

	if (susystem(buf))
                errs++;
        return;
}

/*
 * NAME:        send_remote
 *
 * FUNCTION:    Send to remote host.
 */

send_remote(buf,cmd,uname,port,suser,spath,tuser,thost,tpath)
        char    *buf,                   /* command buffer */
                *cmd,                   /* command prefix */
                *uname,                 /* user login name */
                *suser, *spath,         /* local user,path */
                *tuser, *thost, *tpath; /* target user,host,path */
        int     port;                   /* port id for rsh */

{
	char hostnamebuf[MAXHOSTNAMELEN];
        char    *host = hostnamebuf,                  /* rcmd interfacing temporary */
                *sepath;                /* expanded local pathname */

        /* Since we are sending to one remote target,
         * the destination socket could still be open to it from a
         * previous send_remote() during the main loop.
         */

        /* Make sure the remote receiver is ready */
        if (rem == -1)
        {       sprintf(buf, "%s -t %s", cmd, tpath);
		(void) strncpy(host, thost, sizeof(hostnamebuf));
{
        long address;
        struct hostent *ha = 0;

	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
                if ((ha = gethostbyname (host)) == (struct hostent *) 0) {
                        fprintf(stderr,MSGSTR(NME_NT_FND,
                                            "host: name %s NOT FOUND\n"), host);
                        exit(1);
                }
        }
        else { 
        	if ((address = inet_addr(host)) == -1) {
                       	fprintf(stderr, MSGSTR(BADADDR, 
			 	        "%s: address misformed\n"), host);
                	exit(2);
                	}
		if ((ha = gethostbyaddr(&address, sizeof(address),
                                        AF_INET)) == (struct hostent *)0) {
			fprintf(stderr, MSGSTR(BADHOST,
					"%s: host not found\n"),host);
			exit(2);
			}
		(void) strncpy(host, ha->h_name, sizeof(hostnamebuf));
        }
}
	syslog(LOG_INFO,
                  "EXPORT file to host %s cmd is %s, local user %s.",
                                   host, buf, tuser ? tuser : uname);

                rem = rcmd(&host, port,
                        uname, tuser ? tuser : uname,
                        buf, 0);
                if (rem < 0) exit(1);
                if (response() < 0) exit(1);
                (void) setuid(userid);
        }

        /* Send a local file */
        sepath = expandpath(suser,spath);
        source(1, &sepath);
        free(sepath);
        return;
}

/*
 * NAME:        recv_remote
 *
 * FUNCTION:    Receive from remote host.
 */

recv_remote(buf,cmd,uname,port,suser,shost,spath,tuser,tpath)
        char    *buf,                   /* command buffer */
                *cmd,                   /* command prefix */
                *uname,                 /* user login name */
                *tuser, *tpath,         /* local user & path */
                *suser, *shost, *spath; /* source user,host,path */
        int     port;                   /* port for rsh */

{
	char hostnamebuf[MAXHOSTNAMELEN];
        char *host = hostnamebuf,             /* rcmd() interface argument */
             *tepath;                   /* expanded pathname for target */

	/* rcmd() interface argument */
	(void) strncpy(host, shost, sizeof(hostnamebuf));

        /* Set up remote sender side */
        sprintf(buf, "%s -f %s", cmd, spath);
{
        long address;
        struct hostent *ha = 0;

	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
                if ((ha = gethostbyname (host)) == (struct hostent *) 0) {
                        fprintf(stderr,MSGSTR(NME_NT_FND,
                                            "host: name %s NOT FOUND\n"), host);
                        exit(1);
                }
        }
        else {
        	if ((address = inet_addr(host)) == -1) {
                       	fprintf(stderr, MSGSTR(BADADDR, 
					"%s: address misformed\n"), host);
                	exit(2);
                	}
		if ((ha = gethostbyaddr(&address, sizeof(address),
                                        AF_INET)) == (struct hostent *)0) {
			fprintf(stderr, MSGSTR(BADHOST,
                                        "%s: host not found\n"),host);
                        exit(2);
                        }
		(void) strncpy(host, ha->h_name, sizeof(hostnamebuf));
	   }	
        
}
	syslog(LOG_INFO, "IMPORT file from host %s cmd is %s, local user %s.",
                                        host, buf, suser ? suser : uname);
        rem = rcmd(&host, port,
                uname, suser ? suser : uname,
                buf, 0);
        if (rem < 0){
		errs++;
		return;
	}

        /* Receive data locally */
        tepath = expandpath(tuser,tpath);
        (void) setreuid(0, userid);
        sink(1, &tepath);
        (void) setreuid(userid, 0);
        (void) close(rem);
        rem = -1;
        free(tepath);
        return;
}

/*
 * NAME:        rcp_local
 *
 * FUNCTION:    Convert from rcp to cp for a local copy.
 *
 */

rcp_local(buf,suser,spath,tuser,tpath)
        char    *buf,                   /* command buffer */
                *suser, *spath,         /* source user,host,path */
                *tuser, *tpath;         /* target user,host,path */

{       char    *sepath,                /* expanded source path */
                *tepath;                /* expanded target path */

        /* use expandpath() to possibly insert the home directory names */
        sepath = expandpath(suser,spath);
        tepath = expandpath(tuser,tpath);

        /* build a cp command and submit it */
        sprintf(buf, "cp%s%s %s %s",
                iamrecursive ? " -r":"",
                pflag ? " -p" : "",
                sepath, tepath);
	syslog(LOG_INFO, "LOCAL COPY cmd is %s.", buf);
	if (susystem(buf))
	    errs++;

        free(sepath);
        free(tepath);
        return;
}

verifydir(cp)
	char *cp;
{
	struct stat stb;

	if (stat(cp, &stb) >= 0) {
		if ((stb.st_mode & S_IFMT) == S_IFDIR)
			return;
		errno = ENOTDIR;
	}
	error(MSGSTR(RCPERR, "rcp: %s: %s.\n"), cp, SYSERR(errno)); /*MSG*/
	exit(1);
}

char *
colon(cp)
	char *cp;
{

	while (*cp) {
		if (*cp == ':' )
			return (cp);
		if (*cp == '/')
			return (0);
		cp++;
	}
	return (0);
}

okname(cp0)
	char *cp0;
{
	register char *cp = cp0;
	register int c;

	if (MB_CUR_MAX > 1) {
		wchar_t *wc;
		int n, mbcount, i = 0;

		n = (strlen(cp)+1)*sizeof(wchar_t);
		wc = (wchar_t *)malloc(n);
		mbcount = mbstowcs(wc, cp, n);
		if (mbcount < 0) {
			fprintf(stderr, MSGSTR(OK_MBTOWC, "okname: error converting to widechar (%s)\n"), cp);
			return(0);
		}
		do {
			if (!iswprint(wc[i]))
				goto bad;
			i++;
		} while(wc[i]);
		return(1);
	}
	else {
		do {
			c = *cp;
	
/*			if (c & 0200) */	/* 7 bit ascii only */
/*				goto bad; */  	/* from original code */ 
			if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad;
			cp++;
		} while (*cp);
		return (1);
	}
bad:
	fprintf(stderr, MSGSTR(USERNMERR, "rcp: invalid user name %s\n"), cp0); /*MSG*/
	return (0);
}

susystem(s)
	char *s;
{
	int status, pid, w;
	struct sigvec istat, qstat, ign_vec;
	ign_vec.sv_handler=SIG_IGN;
	ign_vec.sv_mask=0;
	ign_vec.sv_onstack=0;

	if ((pid = vfork()) == 0) {
		(void) setuid(userid);
		execl("/bin/sh", "sh", "-c", s, (char *)0);
		_exit(127);
	}
	(void) sigvec(SIGINT, &ign_vec, &istat);
	(void) sigvec(SIGQUIT, &ign_vec, &qstat);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	(void) sigvec(SIGINT, &istat, (struct sigvec *)0);
	(void) sigvec(SIGQUIT, &qstat, (struct sigvec *)0);
	return (status);
}

source(argc, argv)
	int argc;
	char **argv;
{
	char *last, *name;
	struct stat stb;
	static struct buffer buffer;
	struct buffer *bp;
	int x, readerr, f, amt;
	off_t i;
	char buf[BUFSIZ];

	for (x = 0; x < argc; x++) {
		name = argv[x];
		if ((f = open(name, 0)) < 0) {
			error(MSGSTR(FNMERR, "rcp: %s: %s\n"), name, SYSERR(errno)); /*MSG*/
			continue;
		}
		if (fstat(f, &stb) < 0)
			goto notreg;

		switch (stb.st_mode&S_IFMT) {

		case S_IFREG:
			break;

		case S_IFDIR:
			if (iamrecursive) {
				(void) close(f);
				rsource(name, &stb);
				continue;
			}
			/* fall into ... */
		default:
notreg:
			(void) close(f);
			error(MSGSTR(FILERR, "rcp: %s: not a plain file\n"), name); /*MSG*/
			continue;
		}
		last = rindex(name, '/');
		if (last == 0)
			last = name;
		else
			last++;
		if (pflag) {
			/*
			 * Make it compatible with possible future
			 * versions expecting microseconds.
			 */
			(void) sprintf(buf, "T%ld 0 %ld 0\n",
			    stb.st_mtime, stb.st_atime);
			if (write(rem, buf, strlen(buf)) < 0) {
				perror(buf);
				exit(1);
			}
			if (response() < 0) {
				(void) close(f);
				continue;
			}
		}
		(void) sprintf(buf, "C%04o %ld %s\n",
		    stb.st_mode&07777, stb.st_size, last);
		if (write(rem, buf, strlen(buf)) < 0) {
			perror(buf);
			exit(1);
		}
		if (response() < 0) {
			(void) close(f);
			continue;
		}
		if ((bp = allocbuf(&buffer, f, BUFSIZ)) == 0) {
			(void) close(f);
			continue;
		}
		readerr = 0;
		for (i = 0; i < stb.st_size; i += bp->cnt) {
			amt = bp->cnt;
			if (i + amt > stb.st_size)
				amt = stb.st_size - i;
			if (readerr == 0 && read(f, bp->buf, amt) != amt)
				readerr = errno;
			if (write(rem, bp->buf, amt) < 0) {
				perror("write");
				exit(1);
			}
		}
		(void) close(f);
		if (readerr == 0)
			ga();
		else
			error(MSGSTR(FILSZ, "rcp: %s: %s\n"), name, SYSERR(errno));
		(void) response();
	}
}

rsource(name, statp)
	char *name;
	struct stat *statp;
{
	DIR *d = opendir(name);
	char *last;
	struct direct *dp;
	char buf[BUFSIZ];
	char *bufv[1];

	if (d == 0) {
		error(MSGSTR(RCPDIR, "rcp: %s: %s\n"), name, SYSERR(errno)); /*MSG*/
		return;
	}
	last = rindex(name, '/');
	if (last == 0)
		last = name;
	else
		last++;
	if (pflag) {
		(void) sprintf(buf, "T%ld 0 %ld 0\n",
		    statp->st_mtime, statp->st_atime);
		if (write(rem, buf, strlen(buf)) < 0) {
			perror(buf);
			exit(1);
		}
		if (response() < 0) {
			closedir(d);
			return;
		}
	}
	(void) sprintf(buf, "D%04o %d %s\n", statp->st_mode&07777, 0, last);
	if (write(rem, buf, strlen(buf)) < 0) {
		perror(buf);
		exit(1);
	}
	if (response() < 0) {
		closedir(d);
		return;
	}
	while (dp = readdir(d)) {
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(name) + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error(MSGSTR(NMTOOBIG, "%s/%s: Name too long.\n"), name, dp->d_name); /*MSG*/
			continue;
		}
		(void) sprintf(buf, "%s/%s", name, dp->d_name);
		bufv[0] = buf;
		source(1, bufv);
	}
	closedir(d);
	if (write(rem, "E\n", 2) < 0) {
		perror("write E");
		exit(1);
	}
	(void) response();
}

response()
{
	char resp, c, rbuf[BUFSIZ], *cp = rbuf;

	if (read(rem, &resp, 1) != 1)
		lostconn();
	switch (resp) {

	case 0:				/* ok */
		return (0);

	default:
		*cp++ = resp;
		/* fall into... */
	case 1:				/* error, followed by err msg */
	case 2:				/* fatal error, "" */
		do {
			if (read(rem, &c, 1) != 1)
				lostconn();
			*cp++ = c;
		} while (cp < &rbuf[BUFSIZ] && c != '\n');
                if (iamremote == 0) {
                        if (write(2, rbuf, cp - rbuf) < 0) {
                                perror("write");
                                exit(1);
                        }
                }
		errs++;
		if (resp == 1)
			return (-1);
		exit(1);
	}
	/*NOTREACHED*/
}

lostconn()
{

	if (iamremote == 0)
		fprintf(stderr, MSGSTR(LOSTCONN, "rcp: lost connection\n")); /*MSG*/
	exit(1);
}

sink(argc, argv)
	int argc;
	char **argv;
{
	off_t i, j;
	char *targ, *whopp, *cp;
	int of, mode, wrerr, exists, first, count, amt, size;
	struct buffer *bp;
	static struct buffer buffer;
	struct stat stb;
	int targisdir = 0;
	int mask = umask(0);
	char *myargv[1];
	char cmdbuf[BUFSIZ], nambuf[BUFSIZ];
	int setimes = 0;
	struct timeval tv[2];
#define atime	tv[0]
#define mtime	tv[1]
#define	SCREWUP(str)	{ whopp = str; goto screwup; }

	if (!pflag)
		(void) umask(mask);
	if (argc != 1) {
		error(MSGSTR(AMBTGT, "rcp: ambiguous target\n")); /*MSG*/
		exit(1);
	}
	targ = *argv;
	if (targetshouldbedirectory)
		verifydir(targ);
	ga();
	if (stat(targ, &stb) == 0 && (stb.st_mode & S_IFMT) == S_IFDIR)
		targisdir = 1;
	for (first = 1; ; first = 0) {
		cp = cmdbuf;
		if (read(rem, cp, 1) <= 0)
			return;
		if (*cp++ == '\n')
			SCREWUP(MSGSTR(UNEXP, "unexpected '\\n'")); /*MSG*/
		do {
			if (read(rem, cp, 1) != 1)
				SCREWUP(MSGSTR(LOSTCON2, "lost connection")); /*MSG*/
		} while (*cp++ != '\n');
		*cp = 0;
		if (cmdbuf[0] == '\01' || cmdbuf[0] == '\02') {
			if (iamremote == 0) {
				if (write(2, cmdbuf+1, strlen(cmdbuf+1)) < 0) {
					perror("write cmdbuf");
					exit(1);
				}
			}
			if (cmdbuf[0] == '\02')
				exit(1);
			errs++;
			continue;
		}
		*--cp = 0;
		cp = cmdbuf;
		if (*cp == 'E') {
			ga();
			return;
		}

#define getnum(t) {	int isneg; \
			(t) = 0; \
		        if (*cp == '-') { isneg = 1; cp++; } else isneg = 0; \
			while (isdigit(*cp)) (t) = (t) * 10 + (*cp++ - '0'); \
			if (isneg)  t = -t; \
		  }

		if (*cp == 'T') {
			setimes++;
			cp++;
			getnum(mtime.tv_sec);
			if (*cp++ != ' ')
				SCREWUP(MSGSTR(SECDELIM, "mtime.sec not delimited")); /*MSG*/
			getnum(mtime.tv_usec);
			if (*cp++ != ' ')
				SCREWUP(MSGSTR(MSECDELIM, "mtime.usec not delimited")); /*MSG*/
			getnum(atime.tv_sec);
			if (*cp++ != ' ')
				SCREWUP(MSGSTR(ATIMDELIM, "atime.sec not delimited")); /*MSG*/
			getnum(atime.tv_usec);
			if (*cp++ != '\0')
				SCREWUP(MSGSTR(ATIMMSDELIM, "atime.usec not delimited")); /*MSG*/
			ga();
			continue;
		}
		if (*cp != 'C' && *cp != 'D') {
			/*
			 * Check for the case "rcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the rcp command on the remote is
			 * executed so the ^Aerror_message convention isn't
			 * followed.
			 */
			if (first) {
				error("%s\n", cp);
				exit(1);
			}
			SCREWUP(MSGSTR(CTRLREC, "expected control record")); /*MSG*/
		}
		cp++;
		mode = 0;
		for (; cp < cmdbuf+5; cp++) {
			if (*cp < '0' || *cp > '7')
				SCREWUP(MSGSTR(BADMODE, "bad mode")); /*MSG*/
			mode = (mode << 3) | (*cp - '0');
		}
		if (*cp++ != ' ')
			SCREWUP(MSGSTR(MODELIM, "mode not delimited")); /*MSG*/
		size = 0;
		while (isdigit(*cp))
			size = size * 10 + (*cp++ - '0');
		if (*cp++ != ' ')
			SCREWUP(MSGSTR(SZDELIM, "size not delimited")); /*MSG*/
		if (targisdir)
			(void) sprintf(nambuf, "%s%s%s", targ,
			    *targ ? "/" : "", cp);
		else
			(void) strcpy(nambuf, targ);
		exists = stat(nambuf, &stb) == 0;
		if (cmdbuf[0] == 'D') {
			if (exists) {
				if ((stb.st_mode&S_IFMT) != S_IFDIR) {
					errno = ENOTDIR;
					goto bad;
				}
				if (pflag)
					(void) chmod(nambuf, mode);
			} else if (mkdir(nambuf, mode) < 0)
				goto bad;
			myargv[0] = nambuf;
			sink(1, myargv);
			if (setimes) {
				setimes = 0;
				if (utimes(nambuf, tv) < 0)
					error(MSGSTR(TIMSET,"rcp: can't set times on %s: %s\n"),
					    nambuf, SYSERR(errno));
			}
			continue;
		}
		if ((of = open(nambuf, O_WRONLY|O_CREAT, mode)) < 0) {
	bad:
			error(MSGSTR(CREATERR,"rcp: %s: %s\n"), nambuf, SYSERR(errno));
			continue;
		}
		if (exists && pflag)
			(void) fchmod(of, mode);
		ga();
		if ((bp = allocbuf(&buffer, of, BUFSIZ)) == 0) {
			(void) close(of);
			continue;
		}
		cp = bp->buf;
		count = 0;
		wrerr = 0;
		for (i = 0; i < size; i += BUFSIZ) {
			amt = BUFSIZ;
			if (i + amt > size)
				amt = size - i;
			count += amt;
			do {
				j = read(rem, cp, amt);
				if (j <= 0) {
					if (j == 0)
					    error(MSGSTR(LOSTCON3, "rcp: dropped connection")); /*MSG*/
					else
					    error(MSGSTR(RDERR,"rcp: %s\n"), 
						SYSERR(errno));
					exit(1);
				}
				amt -= j;
				cp += j;
			} while (amt > 0);
			if (count == bp->cnt) {
				if (wrerr == 0 &&
				    write(of, bp->buf, count) != count)
					wrerr++;
				count = 0;
				cp = bp->buf;
			}
		}
		if (count != 0 && wrerr == 0 &&
		    write(of, bp->buf, count) != count)
			wrerr++;
		/* aix ftruncate only does reg files; 4.3BSD does all  */
                if (!fstat(of, &stb) && (stb.st_mode & S_IFMT) == S_IFREG) {
			/*
			 * if the file is nfs-mounted then it isn't still
			 * open, so if the file mode prevents writing then
			 * the ftruncate may fail.  so we chmod the file
			 * to writable before the call and switch it back
			 * to the intended mode afterward.
			 */
			(void) fchmod(of, 0777);
			if (ftruncate(of, size))
				error("rcp: can't truncate %s: %s\n",
			    	nambuf, sys_errlist[errno]);
			(void) fchmod(of, stb.st_mode);
		}
		(void) close(of);
		(void) response();
		if (setimes) {
			setimes = 0;
			if (utimes(nambuf, tv) < 0)
				error(MSGSTR(TIMSET, "rcp: can't set times on %s: %s\n"),
				    nambuf, SYSERR(errno)); /*MSG*/
		}				   
		if (wrerr)
			error(MSGSTR(WRTERR,"rcp: error in writing to %s : check for ulimit\n"),nambuf); /*MSG*/
		else
			ga();
	}
screwup:
	error(MSGSTR(PROTOERR, "rcp: protocol is at fault %s\n"), whopp); /*MSG*/
	exit(1);
}

struct buffer *
allocbuf(bp, fd, blksize)
	struct buffer *bp;
	int fd, blksize;
{
	struct stat stb;
	int size;

	if (fstat(fd, &stb) < 0) {
		error(MSGSTR(STATERR, "rcp: fstat: %s\n"), SYSERR(errno)); /*MSG*/
		return ((struct buffer *)0);
	}
	size = roundup(stb.st_blksize, blksize);
	if (size == 0)
		size = blksize;
	if (bp->cnt < size) {
		if (bp->buf != 0)
			free(bp->buf);
		bp->buf = (char *)malloc((unsigned) size);
		if (bp->buf == 0) {
			error(MSGSTR(MEMERR, "rcp: malloc: out of memory\n")); /*MSG*/
			return ((struct buffer *)0);
		}
	}
	bp->cnt = size;
	return (bp);
}

/*VARARGS1*/
error(fmt, a1, a2, a3, a4, a5)
	char *fmt;
	int a1, a2, a3, a4, a5;
{
	char buf[BUFSIZ], *cp = buf;

	errs++;
	*cp++ = 1;
	(void) sprintf(cp, fmt, a1, a2, a3, a4, a5);

	/* make sure buffer terminated with only 1 newline */
	if (buf[strlen(buf)-1] == '\n')
		if (buf[strlen(buf)-2] == '\n')
			buf[strlen(buf)-1] = 0;

	(void) write(rem, buf, strlen(buf));
	if (iamremote == 0) {
		if (write(2, buf+1, strlen(buf+1)) < 0) {
			perror("write");
			exit(1);
		}
	}
}

/*
 * NAME:        localhostp
 *
 * FUNCTION:    Check to see if a host name is the local host.
 *
 */

localhostp(name)
  char *name;
{
	char localname[MAXHOSTNAMELEN];

	if (!name || !*name) return(TRUE);      /* no name => local */

	/* If "name" and "*name" are not NULL, we should return TRUE
       ONLY if the "name" is the local hostname.
	   If gethostname() fails, we treat the "name" as remote.
	   Both "name" and local hostname are possibly short format
	   hostname or fully qualified domain hostname, Ex: hostx or
       hostx.austin.ibm.com, we except either one.
	 */
        
	if (gethostname(localname, sizeof localname) >= 0) {
		if (((strchr(localname,'.') == NULL)
				&& (strchr(name,'.') == NULL))
			|| ((strchr(localname,'.') != NULL)
				&& (strchr(name,'.') != NULL))) {
			/* Either both localname and name ARE fully
			 * qualified or both are NOT fully qualified.
			 * In these cases we can simply do a strcmp() call.
			 */
			if (strcmp(localname, name) == 0)
				return(TRUE);
			else
				return(FALSE);
		} else {
			if ((strchr(localname,'.') != NULL)
					&& (strchr(name,'.') == NULL)) {
			/* localname is fully qualified and name is not
			 * So call strncmp() using the length of localname.
			 */
				if (strncmp(localname, name,
						strlen(localname)) == 0)
					return(TRUE);
				else
					return(FALSE);
			} else {
				if ((strchr(localname,'.') == NULL)
					&& (strchr(name,'.') != NULL)) {
				/* localname is NOT fully qualified and name IS
				 * So call strncmp() using the length of name.
				 */
					if (strncmp(localname, name,
							strlen(name)) == 0)
						return(TRUE);
					else
						return(FALSE);
				}
			}
		}
	}
	return(0);
}

/*
 * NAME:        expandpath
 *
 * FUNCTION:    Prefix a pathname with the user's home directory.
 *
 *              If only a user is specified then get that person's default
 *              path form /etc/passwd.  If only the file name is specified
 *              then return the default path of the person executing this
 *              command.  Otherwise return the path specified in the command
 *              line.
 *
 * NOTES:       The return string has been malloc'd.
 *
 */

char *expandpath(user, path)
  char  *user,          /* user's name */
        *path;          /* pathname relative to user's home directory */

{       char    *uprefix,       /* pointer to user's directory prefix */
                *epath;         /* pointer to new extended path */
        struct passwd *pwd;     /* pointer to /etc/passwd pointers */

        /* Find the requested user's login pathname prefix. */
        if (user)
        {       /* get password entry for the specified user */
                pwd = getpwnam(user);
                if (!pwd)
                {
		   error(MSGSTR(NOACCT, 
                   "rcp: %s does not have an account on this machine\n"), user);
                   exit(1);
                }
                uprefix = pwd->pw_dir;
        }
        else uprefix = "";

        /* no [path] is equivalent to dot */
        if (!path || !*path) path = ".";

        /* special case -- path leads off with root : dump user reference */
        if (*path == '/')  uprefix = "";

        epath = malloc( strlen(uprefix) + strlen(path) + 2 );
        *epath = NULL;                          /* init string */
        if (strlen(uprefix))
        {       strcat(epath, uprefix);
                strcat(epath, "/");
        }
        strcat(epath,path);
        return(epath);
}

/*
 * NAME:        parsearg
 *
 * FUNCTION:    Break a <user>@<host>:<pathname> string into its parts.
 */

parsearg(arg,user,host,path)
        char *arg,              /* raw argument string */
             **user,            /* pointer to user string */
             **host,            /* pointer to host string */
             **path;            /* pointer to pathname string */

{       char    *u = NULL,
                *h = NULL,
                *p = NULL;              /* local pointer values */

        p = colon(arg);                 /* special : search */
        if (p)
        {       *p++ = 0;               /* cut arg in half at the : */
		h = arg;

		/* find occurrence of @ and ensure it is an ascii @ */
		do {
			int n;
			n = mblen(h, MB_CUR_MAX);
			if (n == 1)
				if (*h == '@')
					break;
			 h += n;
		} while (*h);      
		if (*h == '@') {
			*h++ = 0;   	/* split user@host at the @ mark */
			u = arg;	/* user takes front fragment */
		}
		else	h = arg;	/* no user, front goes to host */
        }
        else    p = arg;                /* path only, trivial case */

        if (p && !*p) p = NULL;
        if (h && !*h) h = NULL;
        if (u && !*u) u = NULL;

        *path = p;                      /* update caller's pointers */
        *host = h;
        *user = u;
        return;
}

usage()
{
	fputs(MSGSTR(USAGE, "Usage: rcp [-p] f1 f2; or: rcp [-rp] f1 ... fn d2\n"), stderr); /* MSG */
	exit(1);
}

#ifdef MSG
char *allostr(n)
register unsigned n;
{
	register char *s;

	if(s = malloc(n)) return s;
	error(MSGSTR(MEMERR, "rcp: malloc: out of memory\n")); /*MSG*/
	exit(1);
}
#endif /* MSG */
