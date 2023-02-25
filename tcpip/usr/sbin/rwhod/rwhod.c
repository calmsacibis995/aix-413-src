static char sccsid[] = "@(#)24	1.29  src/tcpip/usr/sbin/rwhod/rwhod.c, tcpadmin, tcpip411, GOLD410 6/27/94 13:52:05";
/* 
 * COMPONENT_NAME: TCPIP rwhod.c
 * 
 * FUNCTIONS: MSGSTR, Mrwhod, configure, die, dosrcpacket, getkmem, 
 *            interval, onalrm, sendto, verify 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the Univeristy of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
#ifndef lint
static char sccsid[] = "rwhod.c	5.9 (Berkeley) 3/5/86";
#endif not lint
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fullstat.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/termio.h>

#include <net/if.h>
#include <netinet/in.h>

#include <nl_types.h>
nl_catd catd;
#include "rwhod_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_RWHOD, n, s) 

#include <nlist.h>
#undef n_name	/* this atrocity is called for because someone redefined
		   this structure field in nlist.h.  AGGGHHHH!!! */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <utmp.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <protocols/rwhod.h>
#ifdef _AIX
#define TTYLEN	sizeof(we->we_utmp.out_line)
#endif /* _AIX */
void die();

/* 
 * The following includes and declerations are for support of the System 
 * Resource Controller (SRC) - .
 */
#include <spc.h>
#include "libsrc.h"

static 	struct srcreq srcpacket;
char	progname[128];
int 	cont;
struct 	srchdr *srchdr;

/*
 * Alarm interval. Don't forget to change the down time check in ruptime
 * if this is changed.
 */
#define AL_INTERVAL (3 * 60)

struct	sockaddr_in sin = { AF_INET };

extern	errno;

char	myname[32];

#define NELEM_NLIST 2

struct  nlist nl[] = {
#define NL_AVENRUN      0
        { "avenrun" },
#define NL_BOOTTIME     1
#ifdef _AIX
	{ "lbolt" },
#else  /* _AIX */
        { "boottime" }, 
#endif /* _AIX */
        0
};
#ifdef _AIX
#define FSHIFT     16          /* SBITS in sched.c */
#define FSCALE     (1<<FSHIFT)
#endif /* _AIX */


/*
 * We communicate with each neighbor in
 * a list constructed at the time we're
 * started up.  Neighbors are currently
 * directly connected via a hardware interface.
 */
struct	neighbor {
	struct	neighbor *n_next;
	char	*n_name;		/* interface name */
	char	*n_addr;		/* who to send to */
	int	n_addrlen;		/* size of address */
	int	n_flags;		/* should forward?, interface flags */
	char	*myaddr;		/* my addr on this interface */
};

struct	neighbor *neighbors;
struct	whod mywd;
struct	servent *sp;
int	s, utmpf, kmemf = -1;

#define	WHDRSIZE	(sizeof (mywd) - sizeof (mywd.wd_we))
#define	RWHODIR		"/usr/spool/rwho"

int	onalrm();
char	*strcpy(),  *malloc();
long	lseek();
int	getkmem();
struct	in_addr inet_makeaddr();

#include <locale.h>

main(argc,argv)
	int argc;
	char *argv[];
{
	struct sockaddr_in from;
	struct fullstat st;
	char path[64];
	int on = 1;
	char *cp;
	extern char *index();

/* 
 * The declerations and code are for SRC support - .
 */

	int rc, i, addrsz;
	struct sockaddr srcaddr;
	int src_exists = TRUE;

	setlocale(LC_ALL,"");
	catd = catopen(MF_RWHOD,NL_CAT_LOCALE);
	strcpy(progname,argv[0]);

	/* make sure file descriptor 0 is a socket */
	addrsz = sizeof(srcaddr);
	if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
		src_exists = FALSE;
	}
	if (src_exists)
		dup2(0,SRC_FD);


	if (getuid()) {
		fprintf(stderr, MSGSTR(SUPERUSER, "rwhod: not super user\n")); /*MSG*/
		exit(1);
	}
	sp = getservbyname("who", "udp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKSERV, "rwhod: udp/who: unknown service\n")); /*MSG*/
		exit(1);
	}
#ifndef TCP_DEBUG
/*
 * fork and setsid invoked only if SRC not present.
 */
	if (!src_exists) {
  		if (fork())
  			exit(0);
  		(void) setsid();
	}
	{ int s;
	  for (s = 0; s < 10; s++)
		(void) close(s);
	  (void) open("/", 0);
	  (void) dup2(0, 1);
	  (void) dup2(0, 2);
	  s = open("/dev/tty", 2);
	  if (s >= 0) {
		(void) ioctl(s, TIOCNOTTY, (char *) 0);
		(void) close(s);
	  }
	}
#endif
	(void) signal(SIGTERM, die);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGHUP, getkmem);

	openlog("rwhod", LOG_PID, LOG_DAEMON);

	if (!src_exists)
		syslog(LOG_NOTICE, MSGSTR(SRC1,
		"%s: ERROR: SRC not found, continuing without SRC support\n"),
		progname);

	if (chdir(RWHODIR) < 0) {
		syslog(LOG_ERR, MSGSTR(NODIR2, "chdir(%s): %m"), RWHODIR); /*MSG*/
		perror(RWHODIR);
		exit(1);
	}

	/*
	 * Establish host name as returned by system.
	 */
	if (gethostname(myname, sizeof (myname) - 1) < 0) {
		syslog(LOG_ERR, MSGSTR(HOSTNM, "gethostname: %m")); /*MSG*/
		exit(1);
	}
	if ((cp = index(myname, '.')) != NULL)
		*cp = '\0';
	strncpy(mywd.wd_hostname, myname, sizeof (myname) - 1);
	utmpf = open(UTMP_FILE, O_RDONLY);
	if (utmpf < 0) {
		(void) close(creat(UTMP_FILE, 0644));
		utmpf = open(UTMP_FILE, O_RDONLY);
	}
	if (utmpf < 0) {
		syslog(LOG_ERR, MSGSTR(UTMP, "/etc/utmp: %m")); /*MSG*/
		exit(1);
	}
	getkmem();
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, MSGSTR(SOCKET, "socket: %m")); /*MSG*/
		exit(1);
	}
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		syslog(LOG_ERR, MSGSTR(SETSOCK, "setsockopt SO_BROADCAST: %m")); /*MSG*/
		exit(1);
	}
	sin.sin_port = sp->s_port;
	if (bind(s, &sin, sizeof (sin)) < 0) {
		syslog(LOG_ERR, MSGSTR(BIND, "bind: %m")); /*MSG*/
		exit(1);
	}
	if (!configure(s)) 
		exit(1);
	signal(SIGALRM, onalrm);
	onalrm();

	for (;;) {
		struct whod wd;
		int cc, whod, len = sizeof (from);
        	fd_set ibits;
		register int n;
	    	int onoff;

		FD_ZERO(&ibits);
		cont=END;
		addrsz=sizeof(srcaddr);
		do {
		        FD_SET(s, &ibits);
			if (src_exists)
		        	FD_SET(SRC_FD, &ibits);

                	n = select(20, &ibits, 0, 0, (struct timeval *)NULL);
			if (n == 0) 
				continue;
			if (n < 0) {
				if (errno != EINTR)
					syslog(LOG_WARNING, "recv: %m");
				continue;
			}

			if FD_ISSET(s, &ibits) {
			    cc = recvfrom(s,(char *)&wd,sizeof(struct whod),
			    	0, &from, &len);
			    if (cc <= 0) {
				if (cc < 0 && errno != EINTR)
					syslog(LOG_WARNING, MSGSTR(RECVWRN, "recv: %m")); /*MSG*/
				continue;
			    }
			    if (from.sin_port != sp->s_port) {
				syslog(LOG_WARNING, MSGSTR(BADADDR, "%d: bad from port"), /*MSG*/
					ntohs(from.sin_port));
				continue;
			    }
#ifdef notdef
			    if (gethostbyname(wd.wd_hostname) == 0) {
				syslog(LOG_WARNING, MSGSTR(UNKHOST, "%s: unknown host"), /*MSG*/
					wd.wd_hostname);
				continue;
			    }
#endif
			    if (wd.wd_vers != WHODVERSION)
				continue;
			    if (wd.wd_type != WHODTYPE_STATUS)
				continue;
			    if (!verify(wd.wd_hostname)) {
				syslog(LOG_WARNING, MSGSTR(HOSTNMERR, "malformed host name from %x"), /*MSG*/
					from.sin_addr);
				continue;
			    }
			    (void) sprintf(path, "whod.%s", wd.wd_hostname);
			/*
		 	* Rather than truncating and growing the file each time,
		 	* use ftruncate if size is less than previous size.
		 	*/
			    whod = open(path, O_WRONLY | O_CREAT, 0644);
			    if (whod < 0) {
				syslog(LOG_WARNING, MSGSTR(WHOPEN, "%s: %m"), path); /*MSG*/
				continue;
			    }
#if vax || pdp11
		            {
				int i, n = (cc - WHDRSIZE)/sizeof(struct whoent);
				struct whoent *we;
	
				/* undo header byte swapping before writing to file */
				wd.wd_sendtime = ntohl(wd.wd_sendtime);
				for (i = 0; i < 3; i++)
					wd.wd_loadav[i] = ntohl(wd.wd_loadav[i]);
				wd.wd_boottime = ntohl(wd.wd_boottime);
				we = wd.wd_we;
				for (i = 0; i < n; i++) {
					we->we_idle = ntohl(we->we_idle);
					we->we_utmp.out_time =
				    	ntohl(we->we_utmp.out_time);
					we++;
				}
			}
#endif
			    (void) time(&wd.wd_recvtime);
			    (void) write(whod, (char *)&wd, cc);
			    if (fstat(whod, &st) < 0 || st.st_size > cc)
				    ftruncate(whod, cc);
			    (void) close(whod);
			}

			rc = -1;
			errno = EINTR;
			if (src_exists)
			    if FD_ISSET(SRC_FD, &ibits)
			        rc = recvfrom(SRC_FD,&srcpacket,SRCMSG,0,&srcaddr,&addrsz);
		} while (rc == -1 && errno == EINTR);
		if (rc < 0) {
			syslog(LOG_ERR,MSGSTR(SRC2,"%s: ERROR: '%d' recvfrom\n"),progname,errno);
			die(1);
		}

		if (src_exists) {
		    switch(srcpacket.subreq.action) {
			case START:
			case REFRESH:
			case STATUS:
				dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: rwhod does not support this option.\n"),sizeof(struct srcrep));
				break;
			case TRACE:
				if (srcpacket.subreq.object == SUBSYSTEM) { 
					onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
					if (setsockopt(s, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0) {
						syslog(LOG_ERR, MSGSTR(SRC4,"setsockopt SO_DEBUG: %m"));
						close(s);
						die(1);
					}
					dosrcpacket(SRC_OK,progname,NULL,sizeof(struct srcrep)); 
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: rwhod does not support this option.\n"),sizeof(struct srcrep));
				break;
			case STOP:
				if (srcpacket.subreq.object == SUBSYSTEM) {
					dosrcpacket(SRC_OK,argv[0],NULL,sizeof(struct srcrep));
					if (srcpacket.subreq.parm1 == NORMAL)
						die(0);
					exit(0);
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: rwhod does not support this option.\n"),sizeof(struct srcrep));
				break;
			default:
				dosrcpacket(SRC_SUBICMD,progname,NULL,sizeof(struct srcrep));
				break;

		    }
		}
	}
}

/*
 * SRC packet processing - .
 */
int dosrcpacket(msgno, subsys, txt, len)
	int msgno;
	char *subsys;
	char *txt;
	int len;
{
	struct srcrep reply;

	reply.svrreply.rtncode = msgno;
	strcpy(reply.svrreply.objname, "rwhod");
	strcpy(reply.svrreply.rtnmsg, txt);
	srchdr = srcrrqs(&srcpacket);
	srcsrpy(srchdr, &reply, len, cont);
}


/*
 * Check out host name for unprintables
 * and other funnies before allowing a file
 * to be created.  Sorry, but blanks aren't allowed.
 */
verify(name)
	register char *name;
{
	register int size = 0;

	while (*name) {
		if (!isascii(*name) || !(isalnum(*name) || ispunct(*name)))
			return (0);
		name++, size++;
	}
	return (size > 0);
}

int	utmptime;
int	utmpent;
int	utmpsize = 0;
struct	utmp *utmp;
int	alarmcount;

onalrm()
{
	register int i;
	struct fullstat stb;
	register struct whoent *we = mywd.wd_we, *wlast;
	int cc;
#ifdef _AIX
	unsigned long avenrun[3];
#else
       double avenrun[3]; 
#endif
	time_t now = time(0);
	register struct neighbor *np;
	if (alarmcount % 10 == 0)
		getkmem();
	alarmcount++;
	(void) fstat(utmpf, &stb);
	if ((stb.st_mtime != utmptime) || (stb.st_size > utmpsize)) {
		utmptime = stb.st_mtime;
		if (stb.st_size > utmpsize) {
			utmpsize = stb.st_size + 10 * sizeof(struct utmp);
			if (utmp)
				utmp = (struct utmp *)realloc(utmp, utmpsize);
			else
				utmp = (struct utmp *)malloc(utmpsize);
			if (! utmp) {
				fprintf(stderr, MSGSTR(MALLOCERR, "rwhod: malloc failed\n")); /*MSG*/
				utmpsize = 0;
				goto done;
			}
		}
		(void) lseek(utmpf, (long)0, L_SET);
		cc = read(utmpf, (char *)utmp, stb.st_size);
		if (cc < 0) {
			perror(MSGSTR(UTMPRD, UTMP_FILE)); /*MSG*/
			goto done;
		}
		wlast = &mywd.wd_we[1024 / sizeof (struct whoent) - 1];
		utmpent = cc / sizeof (struct utmp);
		for (i = 0; i < utmpent; i++) {
			if( utmp[i].ut_type != USER_PROCESS )
				continue;
			if (utmp[i].ut_name[0]) {
				bcopy(utmp[i].ut_line, we->we_utmp.out_line,
				   (sizeof (utmp[i].ut_line) < TTYLEN) ? 
				      sizeof (utmp[i].ut_line) : TTYLEN);
				we->we_utmp.out_line[TTYLEN] = '\0';
				bcopy(utmp[i].ut_name, we->we_utmp.out_name,
				   sizeof (utmp[i].ut_name));
				we->we_utmp.out_time = htonl(utmp[i].ut_time);
				if (we >= wlast)
					break;
				we++;
			}
		}
		utmpent = we - mywd.wd_we;
	}

	/*
	 * The test on utmpent looks silly---after all, if no one is
	 * logged on, why worry about efficiency?---but is useful on
	 * (e.g.) compute servers.
	 */
	if (utmpent && chdir("/dev")) {
		syslog(LOG_ERR, MSGSTR(NODIR, "chdir(/dev): %m")); /*MSG*/
		exit(1);
	}
	we = mywd.wd_we;
	for (i = 0; i < utmpent; i++) {
		if (stat(we->we_utmp.out_line, &stb) >= 0)
			we->we_idle = htonl(now - stb.st_atime);
		we++;
	}
	(void) lseek(kmemf, (long)nl[NL_AVENRUN].n_value, L_SET);
	(void) read(kmemf, (char *)avenrun, sizeof (avenrun));
	for (i = 0; i < 3; i++)
#ifdef _AIX
		mywd.wd_loadav[i] = htonl((u_long)(100 * avenrun[i] /FSCALE));
#else /* _AIX */
		mywd.wd_loadav[i] = htonl((u_long)(avenrun[i] * 100));
#endif /* _AIX */
	cc = (char *)we - (char *)&mywd;
	mywd.wd_sendtime = htonl(time(0));
	mywd.wd_vers = WHODVERSION;
	mywd.wd_type = WHODTYPE_STATUS;
	for (np = neighbors; np != NULL; np = np->n_next) {
		(void) sendto(s, (char *)&mywd, cc, 0,
			np->n_addr, np->n_addrlen);
	}

	if (utmpent && chdir(RWHODIR)) {
		syslog(LOG_ERR, MSGSTR(NODIR2, "chdir(%s): %m"), RWHODIR); /*MSG*/
		exit(1);
	}
done:
	
	(void) alarm(AL_INTERVAL);
}

getkmem()
{
	static long vmunixino;
	static time_t vmunixctime;
	struct fullstat sb;

#ifndef _AIX
	if (stat("/vmunix", &sb) < 0) {
		if (vmunixctime)
			return;
	} else {
		if (sb.st_ctime == vmunixctime && sb.st_ino == vmunixino)
			return;
		vmunixctime = sb.st_ctime;
		vmunixino= sb.st_ino;
	}
#endif
	if (kmemf >= 0)
		(void) close(kmemf);
loop:
#ifdef _AIX
	if (knlist(nl, NELEM_NLIST, sizeof (struct nlist))) {
		syslog(LOG_WARNING, MSGSTR(UNIXNLST, "namelist botch")); /*MSG*/
#else
	if (nlist("/vmunix", nl)) {
		syslog(LOG_WARNING, MSGSTR(VMUNIXNLST, "/vmunix namelist botch")); /*MSG*/
#endif
		sleep(300);
		goto loop;
	}
	kmemf = open("/dev/kmem", O_RDONLY);
	if (kmemf < 0) {
		syslog(LOG_ERR, MSGSTR(KMEMOPEN, "/dev/kmem: %m")); /*MSG*/
		exit(1);
	}
	(void) lseek(kmemf, (long)nl[NL_BOOTTIME].n_value, L_SET);
	(void) read(kmemf, (char *)&mywd.wd_boottime,
	    sizeof (mywd.wd_boottime));
#ifdef _AIX

/* AIX BOOTTIME is # of ticks since boot.  # of secs since boot is BOOTIME/HZ.
 * rwho protocol bootime is boottime since EPOCH so we subtract our bootime
 * from time(0) */

        mywd.wd_boottime = htonl(time(0) - (mywd.wd_boottime / HZ));
#else
        mywd.wd_boottime = htonl(mywd.wd_boottime);
#endif /* _AIX */
}

/*
 * Figure out device configuration and select
 * networks which deserve status information.
 */
configure(s)
	int s;
{
	char buf[BUFSIZ], *cp, *cplim;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	struct sockaddr_in *sin;
	register struct neighbor *np;
	int n;

	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
		syslog(LOG_ERR, MSGSTR(IOCTLERR, "ioctl (get interface configuration)")); /*MSG*/
		return (0);
	}
	ifr = ifc.ifc_req;
#define max(a, b) (a > b ? a : b)
#ifdef AF_LINK
#define size(p) max((p).sa_len, sizeof(p))
#else
#define size(p) (sizeof(p))
#endif
	cplim = buf + ifc.ifc_len; /* skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim; cp += max(sizeof(struct ifreq), (sizeof(ifr->ifr_name) + size(ifr->ifr_addr))) ) {
		ifr = (struct ifreq *) cp;
		ifreq = *ifr;
		for (np = neighbors; np != NULL; np = np->n_next)
			if (np->n_name &&
			    strcmp(ifr->ifr_name, np->n_name) == 0)
				break;
		/* Skip over non AF_INET (ie. internet) interfaces */
		if (ifreq.ifr_addr.sa_family != AF_INET)
			continue;
		if (np != NULL)
			continue;
		np = (struct neighbor *)malloc(sizeof (*np));
		if (np == NULL)
			continue;
		np->n_name = malloc(strlen(ifr->ifr_name) + 1);
		if (np->n_name == NULL) {
			free((char *)np);
			continue;
		}
		strcpy(np->n_name, ifr->ifr_name);
		np->n_addrlen = sizeof (ifr->ifr_addr);
		np->n_addr = malloc(np->n_addrlen);
		if (np->n_addr == NULL) {
			free(np->n_name);
			free((char *)np);
			continue;
		}
		bcopy((char *)&ifr->ifr_addr, np->n_addr, np->n_addrlen);
		if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, MSGSTR(IOCTLERR2, "ioctl (get interface flags)")); /*MSG*/
			free((char *)np);
			continue;
		}
		if ((ifreq.ifr_flags & IFF_UP) == 0 )  {
			free((char *)np);
			syslog(LOG_INFO, MSGSTR(IFMSG, "Interface %s is down - will ignore") /*MSG*/
					, ifr->ifr_name );
			continue;
		}
		if ((ifreq.ifr_flags & (IFF_BROADCAST|IFF_POINTOPOINT)) == 0) {
			free((char *)np);
			syslog(LOG_INFO, MSGSTR(BRDCSTERR, "Broadcast unsupported on interface %s - ignoring") /* MSG */
					, ifr->ifr_name );
			continue;
		}
		np->n_flags = ifreq.ifr_flags;
		if (np->n_flags & IFF_POINTOPOINT) {
			if (ioctl(s, SIOCGIFDSTADDR, (char *)&ifreq) < 0) {
				syslog(LOG_ERR, MSGSTR(IOCTLERR3, "ioctl (get dstaddr)")); /*MSG*/
				free((char *)np);
				continue;
			}
			/* we assume addresses are all the same size */
			bcopy((char *)&ifreq.ifr_dstaddr,
			  np->n_addr, np->n_addrlen);
		}
		if (np->n_flags & IFF_BROADCAST) {
			if (ioctl(s, SIOCGIFBRDADDR, (char *)&ifreq) < 0) {
				syslog(LOG_ERR, MSGSTR(IOCTLERR4, "ioctl (get broadaddr)")); /*MSG*/
syslog(LOG_ERR, "ioctl (get broadaddr) errno = %d %m", errno); /*MSG*/
				free((char *)np);
				continue;
			}
			/* we assume addresses are all the same size */
			bcopy((char *)&ifreq.ifr_broadaddr,
			  np->n_addr, np->n_addrlen);
		}
/*
**              Get my address on this interface.
*/
		if (ioctl(s, SIOCGIFADDR, (char *)&ifreq) < 0) {
		    syslog(LOG_ERR, MSGSTR(IOCTLERR5, "ioctl (get my addr)")); /*MSG*/
		    np->myaddr = NULL;
		    continue;
		}
		np->myaddr = malloc(np->n_addrlen);
		bcopy((char *)&ifreq.ifr_addr, np->myaddr, np->n_addrlen);
		sin = (struct sockaddr_in *) np->myaddr;
		sin->sin_port = sp->s_port;

		/* gag, wish we could get rid of Internet dependencies */
		sin = (struct sockaddr_in *)np->n_addr;
		sin->sin_port = sp->s_port;
		np->n_next = neighbors;
		neighbors = np;
	}
	return (1);
}

#ifdef TCP_DEBUG
sendto(s, buf, cc, flags, to, tolen)
	int s;
	char *buf;
	int cc, flags;
	char *to;
	int tolen;
{
	register struct whod *w = (struct whod *)buf;
	register struct whoent *we;
	struct sockaddr_in *sin = (struct sockaddr_in *)to;
	char *interval();

#ifdef MSG
	char up[16];

	(void) strcpy(up,MSGSTR(UP,"  up"));
#else
	char *up = "  up";
#endif

	printf(MSGSTR(SENDMSG, "sendto %x.%d\n"), ntohl(sin->sin_addr),
		ntohs(sin->sin_port)); /*MSG*/
	printf(MSGSTR(HOSTNAME, "hostname %s %s\n"), w->wd_hostname, 
		interval(ntohl(w->wd_sendtime) - ntohl(w->wd_boottime),up) );/*MSG*/ 
	printf(MSGSTR(LOAD, "load %4.2f, %4.2f, %4.2f\n"), /*MSG*/
	    ntohl(w->wd_loadav[0]) / 100.0, ntohl(w->wd_loadav[1]) / 100.0,
	    ntohl(w->wd_loadav[2]) / 100.0);
	cc -= WHDRSIZE;
	for (we = w->wd_we, cc /= sizeof (struct whoent); cc > 0; cc--, we++) {
		time_t t = ntohl(we->we_utmp.out_time);
		printf("%-8.8s %s:%s %.12s",
			we->we_utmp.out_name,
			w->wd_hostname, we->we_utmp.out_line,
			ctime(&t)+4);
		we->we_idle = ntohl(we->we_idle) / 60;
		if (we->we_idle) {
			if (we->we_idle >= 100*60)
				we->we_idle = 100*60 - 1;
			if (we->we_idle >= 60)
				printf(" %2d", we->we_idle / 60);
			else
				printf("   ");
			printf(":%02d", we->we_idle % 60);
		}
		printf("\n");
	}
}

char *
interval(time, updown)
	int time;
	char *updown;
{
	static char resbuf[32];
	int days, hours, minutes;

	if (time < 0 || time > 3*30*24*60*60) {
		(void) sprintf(resbuf, MSGSTR(UPTIMERR, "   %s ??:??"), updown); /*MSG*/
		return (resbuf);
	}
	minutes = (time + 59) / 60;		/* round to minutes */
	hours = minutes / 60; minutes %= 60;
	days = hours / 24; hours %= 24;
	if (days)
		(void) sprintf(resbuf, MSGSTR(DAYHRMIN, "%s %2d+%02d:%02d"), /*MSG*/
		    updown, days, hours, minutes);
	else
		(void) sprintf(resbuf, MSGSTR(HRMIN, "%s    %2d:%02d"), /*MSG*/
		    updown, hours, minutes);
	return (resbuf);
}
#endif
void die()
{
	exit(0);
}
