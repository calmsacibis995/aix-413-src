static char sccsid[] = "@(#)21	1.9  src/bos/usr/sbin/sliplogin/sliplogin.c, cmdnet, bos411, 9438B411a 9/21/94 17:24:36";
/* 
 * COMPONENT_NAME: (CMDNET) Network commands
 * 
 * FUNCTIONS: sliploginM, findid, sigstr, hup_handler, sliplogin_exit,
 *	      if_datalock, getunit
 *
 * ORIGINS: 26 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * #ifndef lint
 * "@(#) Copyright (c) 1990 The Regents of the University of California.\n\
 * All rights reserved.\n";
 * #endif /% not lint %/
 * 
 * #ifndef lint
 * static char sccsid[] = "@(#)sliplogin.c	5.6 (Berkeley) 3/2/91";
 * #endif /% not lint %/
 */

/*
 * sliplogin.c
 * [MUST BE RUN SUID, SLOPEN DOES A SUSER()!]
 *
 * This program initializes its own tty port to be an async TCP/IP interface.
 * It sets the line discipline to slip, invokes a shell script to initialize
 * the network interface, then pauses forever waiting for hangup.
 *
 * It is a remote descendant of several similar programs with incestuous ties:
 * - Kirk Smith's slipconf, modified by Richard Johnsson @ DEC WRL.
 * - slattach, probably by Rick Adams but touched by countless hordes.
 * - the original sliplogin for 4.2bsd, Doug Kingston the mover behind it.
 *
 * There are two forms of usage:
 *
 * "sliplogin"
 * Invoked simply as "sliplogin", the program looks up the username
 * in the file /etc/slip.hosts.
 * If an entry is found, the line on fd0 is configured for SLIP operation
 * as specified in the file.
 *
 * "sliplogin IPhostlogin </dev/ttyb"
 * Invoked by root with a username, the name is looked up in the
 * /etc/slip.hosts file and if found fd0 is configured as in case 1.
 */

#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <nlist.h>
#include <paths.h>
#include <sys/syslog.h>

#define POSIX

#ifdef POSIX
#include <sys/termio.h>
#include <sys/ioctl.h>
#include <ttyent.h>
#else
#include <sgtty.h>
#endif
#include <netinet/in.h>
#include <net/if.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "pathnames.h"
#include "sliplogin_msg.h"

#define	BUF_SIZ	4000
#define IFLOCKNAME "sl_if"
#define MSGSTR(N,S)     catgets(catd, MS_SLIPLOGIN, N, S)
nl_catd catd;

int	unit;
int	slip_mode;
int	speed;
int	uid;
int	devlocked;
char	loginargs[BUFSIZ];
char	loginfile[MAXPATHLEN];
char	loginname[BUFSIZ];
char	interface_list[50][16];	/* 50 i.f.'s.  Better safe than sorry */

char	*kmemf = _PATH_KMEM;
int	kmem;


struct slip_modes {
	char	*sm_name;
	int	sm_value;
}	 modes[] = {
	"normal",	0,              
/*	"compress",	SC_COMPRESS,	*/
/*	"noicmp",	SC_NOICMP,	*/
/*	"autocomp",	SC_AUTOCOMP	*/
};

sliplogin_exit(ecode) 
	int ecode;
{
	ttyunlock(IFLOCKNAME);  
	if (devlocked) {
		ttyunlock(ttyname(0));
	}
	exit(ecode);
}

if_datalock() {
	ttywait(IFLOCKNAME);
	if ((ttylock(IFLOCKNAME)) != 0) {
		fprintf(stderr, MSGSTR(IF_LOCK_FAIL,
				"Can't lock interface data. quitting\n"));
		sliplogin_exit(ENODEV);
	}
}

/* Get slip interface unit corresponding to the user's tty */
int getunit(tty)
	char *tty;
{
	int result = -1;

	while (isalpha(*tty))
		tty++;

	if (isdigit(*tty))
		result = atoi(tty);
	return(result);
}

void
findid(name,unit)
	char *name;
	int unit;
{
	FILE *fp;
	static char slopt[5][16];
	static char laddr[16];
	static char raddr[16];
	static char mask[16];
	char user[16];
	int i, j, n, s, len;
	struct ifconf ifc;
	struct	ifreq	ifreq, *ifr;
	struct sockaddr_in *sin;
	int addr_in_use, iflist_built, p_to_p;
	char buf[BUF_SIZ], *cp, *cplim;

	(void)strcpy(loginname, name);
	if ((fp = fopen(_PATH_ACCESS, "r")) == NULL) {
		(void)fprintf(stderr, MSGSTR(HOSTFILE_ACCESS,
					"sliplogin: %s: %s\n"),
		    		_PATH_ACCESS, strerror(errno));
		syslog(LOG_ERR, "%s: %m\n", _PATH_ACCESS);
		sliplogin_exit(errno);
	}
	iflist_built = FALSE;
	while (fgets(loginargs, sizeof(loginargs) - 1, fp)) {
		if (ferror(fp))
			break;
		bzero(user, sizeof(user));
		bzero(laddr, sizeof(laddr));
		bzero(raddr, sizeof(raddr));
		bzero(mask, sizeof(mask));
		bzero(slopt, sizeof(slopt));
		n = sscanf(loginargs, "%15s%*[ \t]%15s%*[ \t]%15s%*[ \t]%15s%*[ \t]%15s%*[ \t]%15s%*[ \t]%15s%*[ \t]%15s%*[ \t]%15s\n",
                        user, laddr, raddr, mask, slopt[0], slopt[1], 
			slopt[2], slopt[3], slopt[4]);
		if (user[0] == '#' || isspace(user[0]))
			continue;
		if (strcmp(user, name) != 0)
			continue;

		/*
		 * Found an IP address pair for this user.  Check if it's
		 * available for use by checking each point-to-point 
		 * interface for having a pre-existing entry with the 
		 * same remote address.  If the remote address is in use 
		 * on another point-to-point interface, it cannot be used. 
		 * If the local address matches that of another interface
		 * which is not point-to-point, It cannot be used.  
		 * If either the local or remote address cannot be used, then
		 * continue and hopefully get an alternative address pair for 
		 * this user.
		 */
		if (!iflist_built) {
			if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				fprintf(stderr, 
					MSGSTR(BADSOCK, 
					"sliplogin: socket call failed\n"));
				sliplogin_exit(errno);
			}
		
			ifc.ifc_len = sizeof (buf);
			ifc.ifc_buf = buf;
			if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
				fprintf(stderr, 
				    MSGSTR(IFCONF_FAIL, 
				    "sliplogin: interface listing failed. Exiting.\n"));
				sliplogin_exit(errno);
			}
		
			ifr = ifc.ifc_req;
	
			/* skip over if's with big ifr_addr's */
			cplim = buf + ifc.ifc_len; 
			iflist_built = TRUE;
		}
#define size(p)	MAX((p).sa_len, sizeof(p))
		addr_in_use = FALSE;
		/* 
		 * Check each inet interface for a possible confict.
		 * It's ok if the same local IP address is assigned
		 * to multiple interfaces if they're all point-to-
		 * point connections.  Check for conflicting
		 * remote addresses, though.
		 */
		for (cp = buf; (cp < cplim) && (addr_in_use == FALSE) ;
		     cp += sizeof(ifr->ifr_name) + size(ifr->ifr_addr)) {
			ifr = (struct ifreq *)cp;
			if (ifr->ifr_addr.sa_family == AF_INET) {
				/* 
				 * If destination addrs match for a point-to
				 * point interface, keep looking 
				 */
				(void) memcpy(&ifreq, ifr, sizeof(ifreq));
				if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0)
				{
					fprintf(stderr, 
						MSGSTR(FLAGCHECK_FAIL, 
						"Check of flags for interface %s failed. errno = %d\n"), 
						ifr->ifr_name, errno);
					syslog(LOG_ERR, 
				  	MSGSTR(FLAGCHK_FAIL, 
						"Check of flags for interface %s failed. error is %m\n"), 
						ifr->ifr_name);
					sliplogin_exit(errno);
				}

				p_to_p = (ifreq.ifr_flags & IFF_POINTOPOINT);

				if (p_to_p) {
					if (ioctl(s, SIOCGIFDSTADDR, 
						(char *)&ifreq) < 0) {
						fprintf(stderr, 
							MSGSTR(RADDRCHECK, 
							"Check of destination addr for interface %s failed. errno = %d\n"), 
							ifr->ifr_name, errno);
						syslog(LOG_ERR, MSGSTR(RADDRCHK,
							"Check of remote addr for interface %s failed. error is %m\n"), 
							ifr->ifr_name);
						sliplogin_exit(errno);
					}
					sin = (struct sockaddr_in *) 
							&ifreq.ifr_dstaddr;
					if (!strcmp(raddr,
					    inet_ntoa(sin->sin_addr.s_addr))) {
						char *temp = ifr->ifr_name;
						while (!isdigit(*temp))
							temp++;	
						/* 
						 * If not re-using same addr
						 * on same i.f., consider addr
						 * in use.
						 */
						if (atoi(temp) != unit) {
							addr_in_use = TRUE;
							break;
						}
					}
				}
				else { /* not point-to-point */ 
					if (ioctl(s, SIOCGIFADDR, 
						(char *)&ifreq) < 0) {
						fprintf(stderr, 
							MSGSTR(LADDRCHECK, 
							"Check of local addr for interface %s failed. errno = %d\n"), 
							ifr->ifr_name, errno);
						syslog(LOG_ERR, MSGSTR(LADDRCHK,
							"Check of local addr for interface %s failed. error is %m\n"), 
							ifr->ifr_name);
						sliplogin_exit(errno);
					}
					/*
				 	* If local addrs match and it's not a 
				 	* point-to-point interface, keep looking
				 	*/
					sin = (struct sockaddr_in *) 
							&ifr->ifr_addr;
					if (!strcmp(laddr, inet_ntoa(
							sin->sin_addr.s_addr)))
					{
						addr_in_use = TRUE;
						break;
					}
				}
			}
		} /* End for each inet interface */

		/*
		 * If the address pair found in _PATH_ACCESS conflicts with
		 * those already in in use, try to find another one in 
		 * _PATH_ACCESS.  Otherwise, use the address pair just found 
		 * in _PATH_ACCESS.
		 */
		if (addr_in_use)
			continue;

		/*
		 * At this point, the local and remote addresses
		 * have been deemed 'usable' 
		 */
		slip_mode = 0;
		for (i = 0; i < n - 4; i++) {
			for (j = 0; j < sizeof(modes)/sizeof(struct slip_modes);
				j++) {
				if (strcmp(modes[j].sm_name, slopt[i]) == 0) {
					slip_mode |= modes[j].sm_value;
					break;
				}
			}
		}

		/*
		 * we've found the guy we're looking for -- see if
		 * there's a login file we can use.  First check for
		 * one specific to this host.  If none found, try for
		 * a generic one.
		 */
		(void)sprintf(loginfile, "%s.%s", _PATH_LOGIN, name);
		if (accessx(loginfile, R_OK|X_OK, ACC_SELF) != 0) {
			(void)strcpy(loginfile, _PATH_LOGIN);
			if (accessx(loginfile, R_OK|X_OK, ACC_SELF)) {
				fprintf(stderr, MSGSTR(NO_LOGIN_FILE,
					"%s access denied - no %s file\n"),
					name, _PATH_LOGIN);
				syslog(LOG_ERR, MSGSTR(NO_LOGIN_FILE,
				       "access denied for %s - no %s file\n"),
				       name, _PATH_LOGIN);
				sliplogin_exit(EACCES);
			}
		}

		(void) fclose(fp);
		return;
	}
	(void)fprintf(stderr, MSGSTR(ACCESS_DENIED, 
		"SLIP access denied for %s -- no %s entries available.\n"),
		name, _PATH_ACCESS);
	syslog(LOG_ERR, MSGSTR(ACCESS_DENIED,
		"SLIP access denied for %s -- no %s entries available.\n"),
		name, _PATH_ACCESS);
	sliplogin_exit(EACCES);
	/* NOTREACHED */
}

char *
sigstr(s)
	int s;
{
	static char buf[32];

	switch (s) {
	case SIGHUP:	return("HUP");
	case SIGINT:	return("INT");
	case SIGQUIT:	return("QUIT");
	case SIGILL:	return("ILL");
	case SIGTRAP:	return("TRAP");
	case SIGIOT:	return("IOT");
	case SIGEMT:	return("EMT");
	case SIGFPE:	return("FPE");
	case SIGKILL:	return("KILL");
	case SIGBUS:	return("BUS");
	case SIGSEGV:	return("SEGV");
	case SIGSYS:	return("SYS");
	case SIGPIPE:	return("PIPE");
	case SIGALRM:	return("ALRM");
	case SIGTERM:	return("TERM");
	case SIGURG:	return("URG");
	case SIGSTOP:	return("STOP");
	case SIGTSTP:	return("TSTP");
	case SIGCONT:	return("CONT");
	case SIGCHLD:	return("CHLD");
	case SIGTTIN:	return("TTIN");
	case SIGTTOU:	return("TTOU");
	case SIGIO:	return("IO");
	case SIGXCPU:	return("XCPU");
	case SIGXFSZ:	return("XFSZ");
	case SIGVTALRM:	return("VTALRM");
	case SIGPROF:	return("PROF");
	case SIGWINCH:	return("WINCH");
	case SIGUSR1:	return("USR1");
	case SIGUSR2:	return("USR2");
	}
	(void)sprintf(buf, "sig %d", s);
	return(buf);
}

void
hup_handler(s)
	int s;
{
	char logoutfile[MAXPATHLEN];
	int i;

	(void)sprintf(logoutfile, "%s.%s", _PATH_LOGOUT, loginname);
	if (accessx(logoutfile, R_OK|X_OK, ACC_SELF) != 0)
		(void)strcpy(logoutfile, _PATH_LOGOUT);
	if (accessx(logoutfile, R_OK|X_OK, ACC_SELF) == 0) {
		char logoutcmd[2*MAXPATHLEN+32];

		(void) sprintf(logoutcmd, "%s %d %d %s", logoutfile, unit, 
					speed, loginargs);
		if (s = system(logoutcmd)) {
			syslog(LOG_ERR, MSGSTR(LOGOUT_FAIL,
				"logout script failed: exit status %d from %s"),
		       		s, logoutfile);
		}
	}
	else {
		syslog(LOG_INFO, MSGSTR(NO_LO_FILE,
			"%s file not found\n"), logoutfile);
	}
	
        i=ioctl(0, I_POP, "slip");
        if  (i < 0) {
		syslog(LOG_ERR, MSGSTR(SLIP_RMV_FAIL,
			"SLIP discipline removal from tty failed. Errno is %d"),
			errno);
	}
	(void) close(0);
	syslog(LOG_INFO, MSGSTR(CLOSED_SLUNIT,
			"closed %s slip unit %d (%s)\n"), 
			loginname, unit, sigstr(s));
	sliplogin_exit(s);
	/* NOTREACHED */
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int fd, 
	s, 			/* Interface socket descriptor */
	ldisc, odisc, 
	i;			/* Temp var */

	char *name, *devname, *temp, argcopy[sizeof(loginargs)],
		*localaddr, *remoteaddr, *netmask;

#ifdef POSIX
	struct termios tios, otios;
#else
	struct sgttyb tty, otty;
#endif
	char logincmd[2*BUFSIZ+32];

	struct	ifreq	ifr;

	extern uid_t getuid();

	/* close all file descriptors except stdin, stdout,sterr */
	s = getdtablesize();
	for (fd = 3 ; fd < s ; fd++)
		(void) close(fd);

	/* enable use of message catalogues */
	setlocale(LC_ALL,"");
        catd = catopen (MF_SLIPLOGIN,NL_CAT_LOCALE);

	fprintf(stderr, 
		MSGSTR(INVOKED, "sliplogin invoked.  Configuring...\n"));
	if ((name = strrchr(argv[0], '/')) == NULL)
		name = argv[0];
	openlog(name, LOG_PID, LOG_DAEMON);
	devlocked = FALSE;
	uid = getuid();
	if (argc > 1) {

		/*
		 * Disassociate from current controlling terminal, if any,
		 * and ensure that the slip line is our controlling terminal.
		 */
#ifdef POSIX
		if (fork() > 0)
			exit(0);

		name = argv[1];
		if (setsid() != 0)
			perror("setsid");
    		if (ttylocked(ttyname(0))) { 
			fprintf(stderr, MSGSTR(DEVLOCKED, 
				"sliplogin: device already in use (locked)\n"),
				ttyname(0));
			sliplogin_exit(EBUSY);
    		}
		else if (ttylock(ttyname(0))) {
			fprintf(stderr, MSGSTR(LOCKFAIL,
				"Device lock attempt on %s failed.\n"),
				ttyname(0));
			sliplogin_exit(EBUSY);
		}
		devlocked = TRUE;
#else
		name = argv[1];
    		if (ttylocked(ttyname(0))) { 
			fprintf(stderr, MSGSTR(DEVLOCKED,
				"sliplogin: device already in use (locked) \n"),
				ttyname(0));
			sliplogin_exit(EBUSY);
    		}
		else if (ttylock(ttyname(0))) {
			fprintf(stderr, MSGSTR(LOCKFAIL,
				"Device lock attempt on %s failed.\n"),
				ttyname(0));
			sliplogin_exit(EBUSY);
		}
		devlocked = TRUE;
		if ((fd = open("/dev/tty", O_RDONLY, 0)) >= 0) {
			extern char *ttyname();

			(void) ioctl(fd, TIOCNOTTY, (caddr_t)0);
			(void) close(fd);
			/* open slip tty again to acquire as controlling tty? */
			fd = open(ttyname(0), O_RDWR, 0);
			if (fd >= 0)
				(void) close(fd);
		}
		(void) setpgrp(0, getpid());
#endif
		if (argc > 2) {
			if ((fd = open(argv[2], O_RDWR)) == -1) {
				perror(argv[2]);
				sliplogin_exit(errno);
			}
			(void) dup2(fd, 0);
			if (fd > 2)
				close(fd);
		}
#ifdef TIOCSCTTY
		if (ioctl(0, TIOCSCTTY, (caddr_t)0) != 0)
			perror("ioctl (TIOCSCTTY)");
#endif
	} else {
		extern char *getlogin();

		if ((name = getlogin()) == NULL) {
			(void) fprintf(stderr, MSGSTR(NO_USERNAME,
					"access denied - no username\n"));
			syslog(LOG_ERR, MSGSTR(GETLOGIN_FAIL,
				"access denied - getlogin returned 0\n"));
			sliplogin_exit(EPERM);
		}
	}

	/* Determine if fd0 really corresponds to a tty device */
	devname = basename(ttyname(0));
	if (((strncmp(devname, "tty", 3)) != 0) ||
	    (*(devname+3) < '0' || *(devname+3) > '9')){
			fprintf(stderr, MSGSTR(NOTATTY,
				"not on a tty device.  Exiting.\n"));
			sliplogin_exit(ENOTTY);
	}

	/* find out what unit number we were assigned */
	unit = getunit(devname);
	if (unit < 0) {
		fprintf(stderr, MSGSTR(NOSLIP_IF,
		"No slip interface for %s. errno is %m\n"), devname);
		syslog(LOG_ERR, MSGSTR(NOSLIP_IF,
		"No slip interface for %s. errno is %m\n"), devname);
		sliplogin_exit(ENODEV);
	}

	if_datalock();
	findid(name, unit);

	(void) fchmod(0, 0600);
	(void) fprintf(stderr, MSGSTR(SL_START, 
			"starting slip login for %s\n"),
			 loginname);

	(void) strncpy(argcopy, loginargs, sizeof(argcopy));
	/* Skip first arg (username) */
	localaddr = strtok(argcopy, " \t");  
	localaddr = strtok((char *) 0, " \t");
	remoteaddr = strtok((char *) 0, " \t");
	netmask = strtok((char *) 0, " \t");
	
	fprintf(stderr, MSGSTR(LOCAL_ADDR, 
		"Called system's internet address is %s\n"), 
			localaddr);
	fprintf(stderr, MSGSTR(REMOTE_ADDR,
		"Calling system's internet address is %s\n"),
			remoteaddr);
	fprintf(stderr, MSGSTR(NETMASK, "netmask = %s\n"), 
			netmask);
	
	fprintf(stderr, MSGSTR(GOING_SLIP,
			"activating slip line discipline.\n>"));

#ifdef POSIX

	/* set up the line parameters */
	/* gotta flush the term so that I_PUSH "slip" will succeed */
	/* we also gotta turn off all output and input processing  */
	/* in the adapter */
	if (tcgetattr(0, &tios) < 0) {
		syslog(LOG_ERR, "tcgetattr: %m");
		sliplogin_exit(errno);
	}
	otios = tios;
	tios.c_iflag = 0;                 /* turn off all input processing */
	tios.c_oflag = 0;                 /* turn off all output processing */
	tios.c_lflag = 0;                 /* turn off local options */
	tios.c_cflag |= CREAD;            /* make sure we receive characters */
	for (i = 0; i < NCCS; i++)
		tios.c_cc[i] = 0xFF;            /* disable control characters */
	if (tcsetattr(0, TCSAFLUSH, &tios) < 0) {
		fprintf(stderr, MSGSTR(TTYSETUP, "terminal setup failed.\n"));
		syslog(LOG_ERR, "tcsetattr: %m");
		sliplogin_exit(errno);
	}
	speed = cfgetispeed(&tios);
#else
	/* set up the line parameters */
	if (ioctl(0, TIOCGETP, (caddr_t)&tty) < 0) {
		fprintf(stderr, MSGSTR(TTYSETUP, "terminal setup failed.\n"));
		syslog(LOG_ERR, "ioctl (TIOCGETP): %m");
		sliplogin_exit(errno);
	}
	otty = tty;
	speed = tty.sg_ispeed;
	tty.sg_flags = RAW | ANYP;
	if (ioctl(0, TIOCSETP, (caddr_t)&tty) < 0) {
		fprintf(stderr, MSGSTR(TTYSETUP, "terminal setup failed.\n"));
		syslog(LOG_ERR, "ioctl (TIOCSETP): %m");
		sliplogin_exit(errno);
	}
#endif
	/* find out what ldisc we started with */
	if (ioctl(0, TIOCGETD, (caddr_t)&odisc) < 0) {
		fprintf(stderr, MSGSTR(TTYSETUP, "terminal setup failed.\n"));
		syslog(LOG_ERR, "ioctl(TIOCGETD) (1): %m");
		sliplogin_exit(errno);
	}

	/*
	 * Run login and logout scripts as root (real and effective);
	 * current route(8) is setuid root, and checks the real uid
	 * to see whether changes are allowed (or just "route get").
	 */
	syslog(LOG_INFO, MSGSTR(SLOGIN_ATTACH,
			"attaching slip unit %d for %s on %s\n"), 
			unit, loginname, devname);
	(void)sprintf(logincmd, "%s %d %d %s", loginfile, unit, speed,
		      loginargs);
	if (s = system(logincmd)) {
		syslog(LOG_ERR, MSGSTR(LOGIN_FAIL,
			"%s login failed: exit status %d from %s"),
		       loginname, s, loginfile);
		sliplogin_exit(s);
	}

	/* Clean up tty before pushing slip */
	while (ioctl(0, I_POP, 0) == 0)
		continue;

	/* set line discipline to slip */
    	if ((ioctl(0, I_PUSH, "slip")) < 0) {
     		fprintf(stderr, MSGSTR(SLIPDISC_FAIL,
				  "sliplogin: cannot add slip disc. to %s\n"),
			devname);
     		syslog(LOG_ERR, MSGSTR(SLIPDISC_FAIL,
				  "sliplogin: cannot add slip disc. to %s\n"),
			devname);
		perror("ioctl(I_PUSH)");
		sliplogin_exit(errno);
    	}

	i = ioctl(0, SLIOCSATTACH, &unit);
	if (i < 0) {
		fprintf(stderr, MSGSTR(NOATTACH, 
				"ioctl(SLIOCSATTACH) failed. errno is %d\n"), 
			errno);
    		ioctl(fd, I_POP, "slip");
		exit(1);
	}

	ttyunlock(IFLOCKNAME);  /* Done with manipulations.  Let others in. */

	(void) signal(SIGHUP, hup_handler);
	(void) signal(SIGTERM, hup_handler);

	/* twiddle thumbs until we get a signal */
	while (1)
		sigpause(0);

	/* NOTREACHED */
}
