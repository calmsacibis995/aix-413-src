static char sccsid[] = "@(#)60	1.10  src/tcpip/usr/bin/setclock/setclock.c, tcptimer, tcpip411, GOLD410 2/27/94 21:17:00";
/* 
 * COMPONENT_NAME: TCPIP setclock.c
 * 
 * FUNCTIONS: MSGSTR, Msetclock, gethostinfo, readtime, readwait 
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
 */

/* setclock.c */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <netdb.h>

#ifdef CSECURITY
#include <sys/syslog.h>
#include "tcpip_audit.h"
#endif CSECURITY

extern	int errno;

/* DDN Protocol Handbook, RFC 868 */
#define ARPA_TIMESERVER_PORT		37

/* host-related flags, limits */
#define	HOST_KNOWN    0
#define	HOST_UNKNOWN  -1
#define	HOSTNAMELEN   100

/* timeout for reading on socket */
#define  TIMEOUT_SECS                    5
#define  TIMEOUT_MICROSECS               0

static struct timeval
   timeout = { TIMEOUT_SECS, TIMEOUT_MICROSECS };

#define  UNIX_INTERNET_TIME_DIFF   2208988800;

#ifdef CSECURITY
char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
extern char *sys_errlist[];
ulong remote_addr;
#endif CSECURITY

#include <nl_types.h>
#include "setclock_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SETCLOCK,n,s) 

#define TIMEBUF 50	/* buffer to hold time string */

#include <locale.h>

/* main - setclock routine */
main(argc, argv)
int argc;
char *argv[];
{

	struct servent *sp;
	struct in_addr hostaddr;
	struct sockaddr_in sin;
	int fd;
	int socktype;
	char timehost[HOSTNAMELEN];
	time_t timevalue;
	char buf[TIMEBUF];
	struct tm *tmptr;
	struct tm *localtime();

	setlocale(LC_ALL,"");
	catd = catopen(MF_SETCLOCK,NL_CAT_LOCALE);

#ifdef CSECURITY
        if ((auditproc((char *)0,AUDIT_STATUS,AUDIT_SUSPEND, (char *)0)) < 0) {
                syslog(LOG_ALERT, MSGSTR(ALERT_SYS, "setclock: auditproc: %m")); /*MSG*/
                exit(1);
        }
#endif CSECURITY

	/* parse args */
	if (argc == 2)
		/* single arg is the desired timeserver */
		strcpy(timehost, argv[1]);
	else if (argc == 1)
		/* no arg, so use default host, "timeserver" */
		strcpy(timehost, "timeserver");
	else {
		fprintf(stderr, MSGSTR(USAGE, "usage: %s <timeserver>\n"), argv[0]); /*MSG*/
		exit(1);
	}
		
	/* what do we know about timehost? */
	if (gethostinfo(&hostaddr, timehost) == HOST_UNKNOWN) {
		fprintf(stderr, MSGSTR(UNKN_HST, "%s: unknown host\n"), timehost); /*MSG*/
		exit(1);
	}
	
	/* create a socket for the conversation */
	if (( fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror(MSGSTR(SOCK,"socket"));
		exit(1);
	}

	/* specify who we're conversing with */
	sin.sin_family = AF_INET;
	sin.sin_port = ARPA_TIMESERVER_PORT;
	sin.sin_addr = hostaddr;

	/* connect does bind for us */
	if ((connect(fd, &sin, sizeof(sin))) != 0) {
#ifdef CSECURITY
		CONNECTION_WRITE(0, "time/tcp", "open",
			sys_errlist[errno], errno);
#endif CSECURITY 
		perror(MSGSTR(CONN, "connect"));
		exit(1);
	}
#ifdef CSECURITY
	remote_addr = sin.sin_addr.s_addr;
	CONNECTION_WRITE(remote_addr, "time/tcp", "open", "", 0);
#endif CSECURITY 

	/* talk to timehost on the network */
	timevalue = readtime(fd, timehost);

#ifdef TCP_DEBUG
	printf("Converted Unix time is %d\n", timevalue );
	printf("Which translates to %s\n", asctime(localtime(&timevalue)));
#endif

	/* print, and set system time if you can */
	tmptr = localtime(&timevalue);
	if (strftime(buf, TIMEBUF, "%a %h %d %H:%M:%S %Y", tmptr))
		printf("%s\n", buf);
	else
		printf("%s", asctime(tmptr));

	/* return to who we really are */
	if (setuid(getuid())) {
		perror(MSGSTR(SETUID, "setclock : setuid"));
		exit(1);
	}

	if ((stime(&timevalue) == -1) & (getuid() == 0)) {
		perror(MSGSTR(STIME_FAIL, "stime failed")); /*MSG*/
#ifdef CSECURITY
		CONNECTION_WRITE(remote_addr, "time/tcp", "close", "", 0);
#endif CSECURITY
		exit(1);
	} else {
#ifdef TCP_DEBUG
		printf("system time successfully set.\n");
#endif DEBUG
	}
#ifdef CSECURITY
	CONNECTION_WRITE(remote_addr, "time/tcp", "close", "", 0);
#endif CSECURITY 
	exit(0);

}


/* readtime - get the time from the timehost */
time_t
readtime(s, timehost)
int s;
char *timehost;
{
	long	rawtime;
	int	rawsize = sizeof(rawtime);
	int	readmask;

	/* protocol: request time by sending an empty datagram */
	if (write(s, &rawtime, rawsize) <= 0) {
		perror(MSGSTR(WR_SOCK_FAIL, "write on socket failed")); /*MSG*/
#ifdef CSECURITY
		CONNECTION_WRITE(remote_addr, "time/tcp", "close",
			sys_errlist[errno], errno);
#endif CSECURITY 
		exit(1);
	}

#ifdef TCP_DEBUG
	printf("wrote on socket; will now read reply\n");
#endif

#ifdef TCP_DEBUG
	printf("right before readwait, s = %d, readmask = %x\n", s, readmask);
#endif

	readmask = 1 << s;
	switch (readwait(s, &readmask)) {
	case -1:
		perror(MSGSTR(SELECT,"select"));
#ifdef CSECURITY
		CONNECTION_WRITE(remote_addr, "time/tcp", "close",
			sys_errlist[errno], errno);
#endif CSECURITY 
		exit(1);
	case 0:
		printf(MSGSTR(NO_RESPONSE, "Time server %s does not respond.\n"), timehost); /*MSG*/
#ifdef CSECURITY
		CONNECTION_WRITE(remote_addr, "time/tcp", "close",
			"Time server does not respond", -1);
#endif CSECURITY 
		exit(0);
	default:
		/* process time data that should be incoming */
		if (read(s, &rawtime, rawsize) <= 0) {
			perror(MSGSTR(RD_SOCK_FAIL, "read on socket failed")); /*MSG*/
#ifdef CSECURITY
			CONNECTION_WRITE(remote_addr, "time/tcp", "close",
				sys_errlist[errno], errno);
#endif CSECURITY 
			exit(1);
         	}
	}

#ifdef TCP_DEBUG
	printf( "raw time is %ld (0x%x)\n", rawtime, rawtime );
#endif

	/* convert from internet (1900-based) to unix (1970-based) time */
	rawtime -= UNIX_INTERNET_TIME_DIFF;

	return((time_t) rawtime);
}


/* gethostinfo - lookup hostname or dot address in argument hostname */
gethostinfo(hostaddrp, hostname)
struct in_addr		*hostaddrp;
char					*hostname;
{
	struct hostent		*hp, *gethostbyname(), *gethostbyaddr();
	char					*addrp;
	int					addrlen;

	if ((hostaddrp->s_addr = inet_addr(hostname)) != -1) {

		/* hostname is an internet "." address; it's network byte */
		/* order representation is in hostaddrp->s_addr */

		addrp = (char *) &(hostaddrp->s_addr);
		addrlen = sizeof(hostaddrp->s_addr);
		if ((hp = gethostbyaddr(addrp, addrlen, AF_INET)) == NULL)
			return(HOST_UNKNOWN);

		/* get official name */
		strncpy(hostname, hp->h_name, HOSTNAMELEN);

	} else {

		/* not "." notation, so try it as a hostname */
		if ((hp = gethostbyname(hostname)) == NULL)
			return(HOST_UNKNOWN);

		/* save its official name and internet address */
		strncpy(hostname, hp->h_name, HOSTNAMELEN);
		bcopy(hp->h_addr, (char *) &(hostaddrp->s_addr), hp->h_length);

	}
	return(HOST_KNOWN);
}


/* readwait - wait for a socket to have something to read with timeout */
readwait(fd, readmask)
int fd, *readmask;
{
   int w = 8 * sizeof(w);

   return(select(w, readmask, (char *) 0, (char *) 0, &timeout));
}
