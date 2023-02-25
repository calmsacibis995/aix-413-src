static char sccsid[] = "@(#)56  1.31.1.1  src/tcpip/usr/lbin/tftpd/tftpd.c, tcpfilexfr, tcpip41B, 9504A 1/13/95 11:10:36";
/* 
 * COMPONENT_NAME: TCPIP tftpd.c
 * 
 * FUNCTIONS: MSGSTR, Mtftpd, err_load, getopt, if,
 *            justquit, nak, recvfile, sendfile, switch, tftp, timer, 
 *            trace_handler, validate_access 
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
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "tftpd.c	5.6 (Berkeley) 5/13/86";
#endif not lint
*/

#include <nl_types.h>
#include "tftpd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTPD,n,s) 

/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

/*
 * The following changes have been made according to RFC 1123:
 *
 * Packets that have a block number for a previous transmission
 * are assumed to have been delayed and are silently ignored.
 * This solves the Sorcerer's Apprentice Syndrome which resulted
 * from transmitting a duplicate packet when a duplicate ACK
 * was received.
 *
 * Adaptive timeout has been implemented with a technique similar
 * to that used by TCP.  When a packet that was not retransmitted
 * is acknowledged, the round trip time is computed and used to 
 * update a smoothed round trip time (srtt = rtt/8 + srtt * 7/8) 
 * and a smoothed round trip time difference (srttd = srttd * 3/4 + 
 * |delta|/4, where delta = rtt - srtt).  When a packet that was
 * retransmitted is acknowledged, the smoothed times are not
 * updated because it is not known which packet is being 
 * acknowledged and thus the round trip time is uncertain.
 * When a packet is transmitted for the first time, the timeout
 * is calculated from the smoothed round trip time and the smoothed
 * round trip time difference (timeout = srtt + 2 * srttd).  When a
 * packet is retransmitted, the timeout is doubled (limited by the
 * maximum number of retries).  When a retransmitted packet is acknowledged,
 * the round trip time is used as the best guess for the timeout
 * for the next packet.
 *
 * Access control has been implemented.  /etc/tftpaccess.ctl is
 * searched for lines that start with "allow:" or "deny:".  Other
 * lines are ignored.  If the file doesn't exist, then access is
 * allowed.  The directories and files indicated as allowed minus those
 * indicated as denied may be accessed.   The longest allowed pathname from
 * the filename is found; then the denied pathnames are searched starting
 * with that pathname.  The syntax of the control file is allow:name or
 * deny:name, where there is one entry per line, no leading or embedded
 * blanks and the name is an absolute pathname of a directory or file.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syslog.h>

#ifdef _AIX
#include <sys/access.h>
#endif _AIX

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/limits.h>

#define TFTPD_MIN_TIMEOUT 1
#define TFTPD_MAX_TIMEOUT 64
#define TFTPD_MAX_RXT 6
#ifdef TFTPD_DEBUG
#define TFTPD_MAX_RXT 6
#endif /* TFTPD_DEBUG */
#define NAMELEN PATH_MAX+1

extern	int errno;
struct	sockaddr_in sin = { AF_INET };
int	peer;
int	rexmtval;
int	newstyle = 0;	/* write create as well as write replace flag */
int	tracing = 0;

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[9];

char err_msg[10][1024];
#define	PKTSIZE	SEGSIZE+4
char	buf[PKTSIZE];
char	ackbuf[PKTSIZE];
struct	sockaddr_in from;
int	fromlen;
int	fd;
char filename[NAMELEN];
/* Following for holding host name or addr */
char sbia[100];
struct hostent *fromhostent;
long fromhostaddr;
int finform = 0;    
int frpthst = 0;    
int fresolv = 0;
int destdir = 0;
char *def_dirname;
char sbsyslog[500];
#include <locale.h>
char *minimize(char *);

/*
 * Packet control block
 */
static struct packet_cb {
        short   pk_rtt;			/* round trip time */
        short   pk_srtt;		/* smoothed round-trip time */
					/* stored fixed pt. 13.3 */
        short   pk_srttd;		/* difference in round-trip time */
					/* stored fixed pt. 14.2 */
        struct timeval pk_xtstart;	/* transmit start time */
        struct timeval pk_xtend;	/* transmit end time */
        short   pk_rxtcnt;		/* retransmit count */
        short   pk_rxtcur;		/* current retransmit timeout value */
        short   pk_rxtnxt;		/* next retransmit timeout value */
} pkcb;
static struct timezone zone;
/*
 * Force a time value to be in a certain range.
 */
#define RANGESET(tv, value, tvmin, tvmax) { \
        (tv) = (value); \
        if ((tv) < (tvmin)) \
                (tv) = (tvmin); \
        else if ((tv) > (tvmax)) \
                (tv) = (tvmax); \
}

#ifdef TFTPD_DEBUG
FILE *debug_file;
#endif /* TFTPD_DEBUG */

main(argc,argv)
int argc;
char **argv;
{
	register struct tftphdr *tp;
	register int n;
	int on = 1;
	char	trash[PKTSIZE];
	struct	sockaddr_in trfrom;
	int	c, trlen, mrc;
	char    *sbiap;
	struct  sigvec sv;
	int	trace_handler();
	extern  char *optarg;

	setlocale(LC_ALL,"");
	catd = catopen(MF_TFTPD,NL_CAT_LOCALE);
	err_load();

/* I put getopt here in case anyone else wanted to add flags to this daemon. */
	while ((c = getopt(argc, argv, "nivrsSd:")) != EOF)
		switch(c) {
			case 'n' : newstyle++;
				   /* Disallow execute privs */
				   umask(S_IXUSR|S_IXGRP|S_IXOTH);
				   break;
			case 'r' : fresolv++; /* Resolve IP addr in syslog */
			           break;
			case 'v' : finform++; /* Log informational messages */
			case 'i' : frpthst++; /* Report host on errors */
			           break;
			case 's' : tracing++; /* Turn on Socket level debug */
			           break;
			case 'd' : destdir++; /* Default Dest dir for puts */
                                   def_dirname = optarg;
                                   if (*def_dirname != '/') {
                                        syslog(LOG_ERR,"inetd.conf : invalid tftpd destination directory \n");
					exit (1);
                                   }
                                   break;
			case '?' : syslog(LOG_ERR,"inetd.conf : invalid tftpd option.\n");
				   exit(1);
		}

	openlog("tftpd", LOG_PID, LOG_DAEMON);
	if (ioctl(0, FIONBIO, &on) < 0) {
		syslog(LOG_ERR, "ioctl(FIONBIO): %m\n");
		exit(1);
	}

	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);

	fromlen = sizeof (from);
	n = recvfrom(0, buf, sizeof (buf), 0,
	    (caddr_t)&from, &fromlen);
	if (n < 0) {
		if (errno != EAGAIN)
			syslog(LOG_ERR, "recvfrom: %m\n");
		exit(1);
	}
	/*
	 * Now that we have read the message out of the UDP
	 * socket, we fork and exit.  Thus, inetd will go back
	 * to listening to the tftp port, and the next request
	 * to come in will start up a new instance of tftpd.
	 *
	 * We do this so that inetd can run tftpd in "wait" mode.
	 * The problem with tftpd running in "nowait" mode is that
	 * inetd may get one or more successful "selects" on the
	 * tftp port before we do our receive, so more than one
	 * instance of tftpd may be started up.  Worse, if tftpd
	 * break before doing the above "recvfrom", inetd would
	 * spawn endless instances, clogging the system.
	 */
	/* Well Frankie Lee and Judas (wrong story) ...
	 * trash any additional requests pending on socket.
	 * This may drop some request, but those
	 * will be resent by the clients when
	 * they timeout.  The positive effect of
	 * this flush is to (try to) prevent more
	 * than one tftpd being started up to service
	 * a single request from a single client.
	 */
	trlen = sizeof (trfrom);
	while ((recvfrom(0, trash, sizeof (trash), 0,
	    (caddr_t)&trfrom, &trlen)) >= 0)
		;
	{
		int pid;
		int i, j, k;

		for (i = 1; i < 20; i++) {
		    pid = fork();
		    if (pid < 0) {
				sleep(i);
				/*
				 * flush out to most recently sent request.
				 *
				 * This may drop some request, but those
				 * will be resent by the clients when
				 * they timeout.  The positive effect of
				 * this flush is to (try to) prevent more
				 * than one tftpd being started up to service
				 * a single request from a single client.
				 */
				j = sizeof from;
				k = recvfrom(0, buf, sizeof (buf), 0,
				    (caddr_t)&from, &j);
				if (k > 0) {
					n = k;
					fromlen = j;
				}
		    } else {
				break;
		    }
		}
		if (pid < 0) {
			syslog(LOG_ERR, "fork: %m\n");
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}
	}
	from.sin_family = AF_INET;
	alarm(0);
	close(0);
	close(1);
	peer = socket(AF_INET, SOCK_DGRAM, 0);
	if (peer < 0) {
		syslog(LOG_ERR, "socket: %m\n");
		exit(1);
	}
	if (bind(peer, (caddr_t)&sin, sizeof (sin)) < 0) {
		syslog(LOG_ERR, "bind: %m\n");
		exit(1);
	}
#ifdef TFTPD_DEBUG
	debug_file = fopen("/tmp/tftp_debug", "w+");
	fprintf ( debug_file, "Opening file \n");
	fflush (debug_file);
#endif /* TFTPD_DEBUG */
	if (connect(peer, (caddr_t)&from, sizeof(from)) < 0) {
		syslog(LOG_ERR, "connect: %m\n");
		exit(1);
	}
/*      
 *      Look up host name (or address) for use in syslog messages.
 *      Host name (or address if name unknown or !fresolv) will be put 
 *      into (char *) sbia.
 */
        if (frpthst) {
	    sbiap = inet_ntoa(from.sin_addr);
	    if (fresolv && (fromhostaddr = inet_addr(sbiap)) != -1 &&
		(fromhostent = gethostbyaddr(&fromhostaddr, 
					     sizeof(fromhostaddr), 
					     AF_INET)) != (struct hostent *)0)
		sbiap = fromhostent->h_name;
	    if (sbiap) 
		sprintf(sbia, "%s\n", sbiap);
	    else 
		strcpy(sbia, "unknown host\n");
	}

        tp = (struct tftphdr *)buf;
	tp->th_opcode = ntohs(tp->th_opcode);
	if (tp->th_opcode == RRQ || tp->th_opcode == WRQ)
		tftp(tp, n);
	exit(1);
}

/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
trace_handler(sig)
	int	sig;
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/
}


int	validate_access();
int	sendfile(), recvfile();

struct formats {
	char	*f_mode;
	int	(*f_validate)();
	int	(*f_send)();
	int	(*f_recv)();
	int	f_convert;
} formats[] = {
	{ "netascii",	validate_access,	sendfile,	recvfile, 1 },
	{ "octet",	validate_access,	sendfile,	recvfile, 0 },
#ifdef notdef
	{ "mail",	validate_user,		sendmail,	recvmail, 1 },
#endif
	{ 0 }
};

/*
 * Handle initial connection protocol.
 */
tftp(tp, size)
	struct tftphdr *tp;
	int size;
{
	register char *cp;
	int first = 1, ecode;
	register struct formats *pf;
	char *mode;


	cp = tp->th_stuff;
	strcpy(filename,cp);
again:
	while (cp < buf + size) {
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
		nak(EBADOP);
		exit(1);
	}
	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}
	for (cp = mode; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	for (pf = formats; pf->f_mode; pf++)
		if (strcmp(pf->f_mode, mode) == 0)
			break;
	if (pf->f_mode == 0) {
		nak(EBADOP);
		exit(1);
	}

	ecode = (*pf->f_validate)(filename, tp->th_opcode);
	if (ecode) {
		nak(ecode);
		exit(1);
	}

	/* Initialize packet control block */
	pkcb.pk_rtt = 0;
	pkcb.pk_srtt = TFTPD_MIN_TIMEOUT << 3;
	pkcb.pk_srttd = 2 << 2;
	pkcb.pk_rxtcnt = 0;
	pkcb.pk_rxtcur = 0;
	pkcb.pk_rxtnxt = 0;

	if (tp->th_opcode == WRQ)
		(*pf->f_recv)(pf);
	else
		(*pf->f_send)(pf);
	exit(0);
}


FILE *file;

/*
 * Validate file existence and access.
 * If old style, then file must exist
 * and be writable by other.
 * If new style, then create file if it
 * does not exist and directory is writable
 * by "nobody". nobody is the uid this daemon
 * runs as, and a more fitting monicker
 * could not have been applied, barring vulgarity.
 * Likewise, if new style and the file does
 * exist and is writable by other, then
 * proceed as with old style.
 * Note also, full path name must be
 * given as we have no login directory.
 */
validate_access(filename, mode)
	char *filename;
	int mode;
{
		
    struct stat stbuf;
    char *cp;
    char bootfile[NAMELEN];
    
    if (*filename != '/') {
	if (mode != RRQ) {
            if (!destdir)
                return (EACCESS);
            else {
                strcpy (bootfile,def_dirname);
                strcat(bootfile,"/");
                strcat(bootfile,filename);
                strcpy(filename,bootfile);
		syslog(LOG_DEBUG,"Using default destination directory: filename=%s\n",filename);
	    }
	}
	else {
	    if ((strstr(filename,"..")==NULL) && 
	      (strstr(filename,"/")==NULL)  ) { /* may be sun diskless */
		strcpy (bootfile,"/tftpboot/");  /* default location */
		strcat(bootfile,filename);
		strcpy(filename,bootfile);
		syslog(LOG_DEBUG,"Sun diskless filename=%s\n",filename);
	    }
	}
    }	
    
    if (*filename != '/') 
	return (EACCESS);
    
    /* 
     * do a stat before resolving the path, to see if it is
     * there at all.  For instance, /tmp/dlfj/../foo" is going
     * to be resolved to /tmp/foo, but if the /tmp/dlfj directory 
     * doesn't exist, then this is an invalid filename.  If we don't 
     * do the stat before resolving the actual filename they want, we 
     * won't catch this.
     */
    if (stat(filename, &stbuf) < 0) {
	if ( errno == ENOENT || ENOTDIR ) {
	    if ( (mode == RRQ) || (!newstyle) )
		return(ENOTFOUND);
	}
	else 
	    return(EACCESS);
    }

    if (!newstyle) {
	if ( mode == WRQ ) {
#ifdef _AIX
	    if (accessx(filename, W_ACC, ACC_SELF) != 0)
#else
		if ((stbuf.st_mode&(S_IWRITE >> 6)) == 0)
#endif _AIX
		    return (EACCESS);
	}
	else {
#ifdef _AIX
	    if (accessx(filename, R_ACC, ACC_SELF) != 0)
#else
		if ((stbuf.st_mode&(S_IREAD >> 6)) == 0)
#endif _AIX
		    return (EACCESS);
	}
    }
    if (acc_ctl(filename) == 0)
	return (EACCESS);
    
    fd = open(filename, mode == RRQ ? O_RDONLY : O_WRONLY|O_CREAT,
	      S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH); 
    if (fd < 0)
	return (errno + 100);
    if (mode == RRQ) {
	file = fdopen(fd, "r");
	if (file == NULL) {
	    return errno+100;
	}
    }
    return (0);
}

/*
 * Determine whether the filename is included in the access control file
 * (/etc/tftpaccess.ctl).  If the file doesn't exist, then access is
 * allowed.  The directories and files indicated as allowed minus those 
 * indicated as denied may be accessed.   The longest allowed pathname from 
 * the filename is found; then the denied pathnames are searched starting
 * with that pathname.  The syntax of the control file is allow:name or
 * deny:name, where there is one entry per line, no leading or embedded 
 * blanks and the name is an absolute pathname of a directory or file.
 */

int
acc_ctl(filename)
char *filename;
{
	char buf[MAXPATHLEN + 10];
	char search_name[MAXPATHLEN + 10];
	char search[MAXPATHLEN + 10];
	FILE *a_d_file;
	char *ptr,*ptr1;
	char *comp;
	int end_pos;
	int return_value, rc = 0;

	return_value = 1;
	end_pos = 0;

	a_d_file = fopen("/etc/tftpaccess.ctl", "r");
	if (a_d_file == (FILE *)NULL) {
		return(return_value);
	}
	else {
		return_value = 0;
		while (fgets(buf, MAXPATHLEN + 9, a_d_file) == buf) {
			if ((ptr = strchr(buf, '\n')) != (char *)0)
				*ptr = '\0';
			if (strncmp(buf, "allow:", 6) != 0)
				continue;
			ptr1=buf + 6;
			if (*ptr1 != '/')
				return(0);
		 	strcpy(ptr, minimize(ptr1));
			if (strcmp(ptr, "/") == 0) {
				return_value = 1;
				end_pos = 1;
				break;
			}
		}
		strcpy(search,filename);
		strcpy(search_name, minimize(search));
		search_name[MAXPATHLEN + 9] = '\0';
		comp = search_name + 1;
		while (comp = strchr(comp, '/')) {
			*comp = '\0';
			fseek(a_d_file, 0, 0);
			while (fgets(buf, MAXPATHLEN + 9, a_d_file) == buf) {
				if ((ptr = strchr(buf, '\n')) != (char *)0)
					*ptr = '\0';
				if (strncmp(buf, "allow:", 6) != 0)
					continue;
				ptr1=buf + 6;
				strcpy(ptr, minimize(ptr1));
				if (strcmp(ptr, search_name)==0) {
					return_value = 1;
					end_pos = comp - search_name;
					break;
				}
			}
			*comp = '/';
			++comp;
		}
		fseek(a_d_file, 0, 0);
		while (fgets(buf, MAXPATHLEN + 9, a_d_file) == buf) {
			if ((ptr = strchr(buf, '\n')) != (char *)0)
				*ptr = '\0';
			if (strncmp(buf, "allow:", 6) != 0)
				continue;
			ptr1=buf + 6;
			strcpy(ptr, minimize(ptr1));
			if (strcmp(ptr, search_name)==0) {
				return_value = 1;
				end_pos = strlen(search_name);
				break;
			}
		}
	}

	if (return_value != 0) {
		if (end_pos <= 1) {
			fseek(a_d_file, 0, 0);
			while (fgets(buf, MAXPATHLEN + 9, a_d_file) == buf) {
				if ((ptr = strchr(buf, '\n')) != (char *)0)
					*ptr = '\0';
				if (strncmp(buf, "deny:", 5) != 0)
					continue;
				ptr1=buf + 5;
				if (*ptr1 != '/')
					return(0);
				strcpy(ptr, minimize(ptr1));
				if (strcmp(ptr, "/") == 0) {
					fclose(a_d_file);
					return(0);
				}
			}
		}
		strcpy(search,filename);
		strcpy(search_name, minimize(search));
		search_name[MAXPATHLEN + 9] = '\0';
		comp = &search_name[end_pos];
		while (comp = strchr(comp, '/')) {
			*comp = '\0';
			fseek(a_d_file, 0, 0);
			while (fgets(buf, MAXPATHLEN + 9, a_d_file) == buf) {
				if ((ptr = strchr(buf, '\n')) != (char *)0)
					*ptr = '\0';
				if (strncmp(buf, "deny:", 5) != 0)
					continue;
				ptr1=buf + 5;
				strcpy(ptr, minimize(ptr1));
				if (strcmp(ptr, search_name)==0) {
					fclose(a_d_file);
					return(0);
				}
			}
			*comp = '/';
			++comp;
		}
		fseek(a_d_file, 0, 0);
		while (fgets(buf, MAXPATHLEN + 9, a_d_file) == buf) {
			if ((ptr = strchr(buf, '\n')) != (char *)0)
				*ptr = '\0';
			if (strncmp(buf, "deny:", 5) != 0)
				continue;
			ptr1=buf + 5;
			strcpy(ptr, minimize(ptr1));
			if (strcmp(ptr, search_name)==0) {
				fclose(a_d_file);
				return(0);
			}
		}
	}

	if (a_d_file != (FILE *)NULL)
		fclose(a_d_file);
	return(return_value);
}



char *
minimize(args)
char *args;
{
    
    char*	remove_dots(char*);
    char 	*saved=NULL;
    char 	buffer[MAXPATHLEN + 10];
    char 	*new = NULL, *new1 ; /* new modified string */
    char        *tmp = NULL ;
    struct 	stat statb;
    int 	found_link;
    char	*cp;
    
    new = (char*) malloc(MAXPATHLEN + 11);
    tmp = (char*) malloc(MAXPATHLEN + 11);
    *new = '\0';
    while (*args != '\0') {
        while (*args == '/') {  /* get rid of all / in the begining */
                args++;
                /* get rid of single dots followed by a slash */
                if (*args == '.' && *(args+1) == '/')
                        args++;
        }
        *--args='/';
        strcpy(buffer, args); /* save string without inital / and ./ */
        buffer[MAXPATHLEN + 9] = '\0';
        saved = buffer+1;
        if ((saved = strchr(saved, '/')) != NULL) {
                strcpy(args, saved); /* save each part */
                *saved='\0';
                if (strcmp(args,"/") == 0)
                        *args = '\0';
        } else {
                strcpy(args,saved); /* for the last part */
        }
        strcpy(tmp,new); /* in case link to a file */
        strcat(new,buffer);  /* concat all together */
        new1 = remove_dots(new);
	strcpy (new,new1);
        free (new1);
        /*
         * Use the stat() and lstat() functions to determine
         * if new is a link or not.  If new is a link follow the link
         * down to the real file.
         */
        found_link = 1;
        while (found_link) {
                if (lstat(new, &statb) < 0) {
                        found_link = 0;
                } else {
                        if ((statb.st_mode & S_IFMT) == S_IFLNK) {
                                int     cc;

                                cc = readlink(new, buffer, MAXPATHLEN);
                                if (cc >= 0)
                                        buffer[cc] = 0;
                                strcpy(new, buffer);
                                if ( *new != '/') {   /* link to a file */
                                        strcat (tmp, "/");
                                        strcat (tmp, new);
                                        new = tmp;
                                }
                        } else
                                found_link = 0;
                }
        }
    }
    /*
     * if any portion of this "new" path is a symbolic link then
     * we need to call minimize() recursively until we find a path
     * that has no portion of the path with symbolic links
     */
    cp = new;           /* cp jumps from '/' to next '/' */
    while (1) {         /* we will break when we are done */
        *cp = '/';      /* fix the cp  back to '/' */
        cp++;
        cp = strchr(cp, '/');
        if (cp != NULL)
                *cp = '\0';

        if (((lstat(new, &statb) == 0) &&
            (statb.st_mode & S_IFMT) == S_IFLNK))  {
                new = minimize(new);
                break;
        }

        if (cp == NULL)
                break;
    }

    return(new);
}


/* 
 * This routine is used for to simplified the path name, i.e. if the
 * pathname is like /.///usr/././/../usr/bin/./help then this makes like
 * /usr/bin/help
 */ 

char* remove_dots(char* s) {
    char	buf2[MAXPATHLEN]; 
    char*	p; 
    char*	buf = NULL;
    char*	tmp = NULL; 
    char*	ptr = NULL;

    if(strlen(s) <= 0) {
	return NULL;
    }

    /* if the directory is just "/", strtok won't find it,
     * so let's check for it.
     */
    if (strcmp(s, "/") == 0) {
	return(s);
    }

    buf = (char*) malloc(strlen(s) + 1);
    strcpy(buf, s);
    strcpy(buf2, "");
    p = buf2;

    for (tmp = strtok(buf, "/");
	 tmp;
	 tmp = strtok(NULL, "/")) 
    {
	if (strcmp(tmp, "..") == 0) {
	    /* 
	     * If there isn't anything in buf2, then just 
	     * continue, otherwise remove the last entry
	     */
	    if (strcmp(p, "/") == 0) {
		continue;
	    }
	    else {
		/* Remove the last directory entry because of /.. */
		ptr = strrchr(p, '/');
		*ptr = '\0';
	    }
	}
	else {
	    if (strcmp(tmp, ".") == 0) {
		/* 
		 * If there isn't anything in buf2, then just 
		 * continue, otherwise remove the last entry
		 */
		if (strcmp(p, "") == 0) {
		    continue;
		}
		else {
		    /* Remove the last directory entry because of /.. */
		    ptr = strrchr(p, '/');
		    *ptr = '\0';
		}
	    }
	    else {
		if ((strlen(p) + strlen(tmp) + 1) > MAXPATHLEN) {
		    return NULL;
		}
		sprintf(p, "%s/%s", p, tmp);
	    }
	}
    }
    free(buf);
    ptr = (char*)malloc(strlen(p) + 1);
    strcpy(ptr, p);
    return(ptr);
}	    


int	timeout;
jmp_buf	timeoutbuf;

timer()
{
	signal(SIGALRM, timer);
	pkcb.pk_rxtcnt++;
	if (pkcb.pk_rxtcnt > TFTPD_MAX_RXT) {
		syslog(LOG_ERR, MSGSTR(MAXTIME, "timer: maxtimeout reached.\n")); /*MSG*/
		if (frpthst && sbia) syslog(LOG_ERR, sbia);
#ifdef TFTPD_DEBUG
	if (debug_file != (FILE *)0)
		fclose(debug_file);
#endif /* TFTPD_DEBUG */
		exit(1);
	}
	longjmp(timeoutbuf, 1);
}

/*
 * Set packet start time.
 */
start_pk_time()
{
	/*
	 * If this packet has timed out and is being retransmitted,
	 * then backoff to a longer retransmit timeout value.
	 */
	if (pkcb.pk_rxtcnt > 0) {
		RANGESET(pkcb.pk_rxtcur, pkcb.pk_rxtcur * 2,
			TFTPD_MIN_TIMEOUT, TFTPD_MAX_TIMEOUT);
		rexmtval = pkcb.pk_rxtcur;
#ifdef TFTPD_DEBUG
	if (debug_file != (FILE *)0) {
		fprintf(debug_file, 
			"   backoff rexmtval to:  %d\n", rexmtval);
		fflush(debug_file);
	}
#endif /* TFTPD_DEBUG */
		return;
	}
	/*
	 * Otherwise, this packet is being transmitted for the first time.
	 */

	/*
	 * Set the start time for this packet.
	 */
	if (gettimeofday(&pkcb.pk_xtstart, &zone) < 0) {
		syslog(LOG_ERR, "gettimeofday: %m\n");
		exit(1);
	}

	/*
	 * If the previous packet was retransmitted, its final retransmit
	 * timeout value is more appropriate to use as a starting point
	 * for this packet.
	 */
	if (pkcb.pk_rxtnxt > 0) {
		rexmtval = pkcb.pk_rxtcur = pkcb.pk_rxtnxt;
		pkcb.pk_rxtnxt = 0;
#ifdef TFTPD_DEBUG
	if (debug_file != (FILE *)0) {
		fprintf(debug_file, 
			"   using prev rexmtval:  %d\n", rexmtval);
		fflush(debug_file);
	}
#endif /* TFTPD_DEBUG */
		return;
	}

	/*
	 * Otherwise, determine the retransmission timeout value =
	 * srtt + 2 * srttd.
	 */
	RANGESET(pkcb.pk_rxtcur, ((pkcb.pk_srtt >> 2) + pkcb.pk_srttd) >> 1,
		TFTPD_MIN_TIMEOUT, TFTPD_MAX_TIMEOUT);
	rexmtval = pkcb.pk_rxtcur;
#ifdef TFTPD_DEBUG
	if (debug_file != (FILE *)0) {
		fprintf(debug_file, 
			"    calculate rexmtval:  %d\n", rexmtval);
		fflush(debug_file);
	}
#endif /* TFTPD_DEBUG */
}

/*
 * Set packet end time.
 */
end_pk_time()
{
	register short delta;

	/*
	 * If the first transmission of the packet has been acknowledged, 
	 * then determine the smoothed round-trip time and the 
	 * smoothed round-trip time difference.
	 *	srtt = rtt/8 + srtt * 7/8
	 *	srttd = srttd * 3/4 + |delta|/4
	 *		where delta = rtt - srtt
	 */
	if (pkcb.pk_rxtcnt == 0) {
		/*
		 * Round down the round trip time to the nearest second.
		 */
		if (gettimeofday(&pkcb.pk_xtend, &zone) < 0) {
			syslog(LOG_ERR, "gettimeofday: %m\n");
			exit(1);
		}
		pkcb.pk_rtt = pkcb.pk_xtend.tv_sec - 
				pkcb.pk_xtstart.tv_sec;

		/*
		 * Determine the difference between the round trip time
		 * and the smoothed round trip time.
		 */
		delta = pkcb.pk_rtt - (pkcb.pk_srtt >> 3);

		/*
		 * Update srtt
		 */
		if ((pkcb.pk_srtt += delta) <= 0)
			pkcb.pk_srtt = 1 << 3;
#ifdef TFTPD_DEBUG
		if (debug_file != (FILE *)0) {
			fprintf(debug_file, 
				"       calculate delta:  %d\n", delta);
			fflush(debug_file);
		}
#endif /* TFTPD_DEBUG */
	
		/*
		 * Update srttd
		 */
		if (delta < 0)
			delta = -delta;
		delta -= pkcb.pk_srttd >> 2;
		if ((pkcb.pk_srttd += delta) <= 0)
			pkcb.pk_srttd = 1 << 2;

		/*
		 * Nullify the next packet transmission timeout value.
		 */
		pkcb.pk_rxtnxt = 0;
#ifdef TFTPD_DEBUG
		if (debug_file != (FILE *)0) {
			fprintf(debug_file, 
				"         calculate rtt:  %d\n", pkcb.pk_rtt);
			fprintf(debug_file, 
				"        calculate srtt:  %d+%d/8\n", 
					pkcb.pk_srtt >> 3, pkcb.pk_srtt & 7);
			fprintf(debug_file, 
				"       calculate srttd:  %d+%d/4\n", 
					pkcb.pk_srttd >> 2, pkcb.pk_srttd & 3);
			fflush(debug_file);
		}
#endif /* TFTPD_DEBUG */
	}
	/*
	 * If a retransmitted packet is being acknowledged, use
	 * the current retransmission time for the next packet.
	 */
	else {
		pkcb.pk_rxtnxt = pkcb.pk_rxtcur;
	}

}

/*
 * Send the requested file.
 */
sendfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *r_init();
	register struct tftphdr *ap;    /* ack packet */
	register int block = 1, size, n;


	signal(SIGALRM, timer);
	dp = r_init();
	ap = (struct tftphdr *)ackbuf;
	do {
		size = readit(file, &dp, pf->f_convert);
		if (size < 0) {
			nak(errno + 100);
			goto abort;
		}
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;
		pkcb.pk_rxtcnt = 0;
		(void) setjmp(timeoutbuf);

send_data:
#ifdef TFTPD_DEBUG
		if (debug_file != (FILE *)0) {
			fprintf(debug_file, 
				"\n                 block:  %d\n", block);
			fflush(debug_file);
		}
#endif /* TFTPD_DEBUG */
		if (send(peer, dp, size + 4, 0) != size + 4) {
			syslog(LOG_ERR, MSGSTR(SENDERR1, "tftpd: write: %m\n")); /*MSG*/
			if (frpthst && sbia) syslog(LOG_ERR, sbia);
			goto abort;
		}
		read_ahead(file, pf->f_convert);
		start_pk_time();	/* set packet start time */
		alarm(rexmtval);
		for ( ; ; ) {

			/* read the ack */
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			if (n < 0) {
				alarm(0);
				syslog(LOG_ERR, MSGSTR(RECERR1, "tftpd: read: %m\n")); /*MSG*/
				if (frpthst && sbia) syslog(LOG_ERR, sbia);
				goto abort;
			}
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == ERROR) {
				alarm(0);
				goto abort;
			}
			
			if (ap->th_opcode == ACK) {
				/* An ack for the current block is expected */
				if (ap->th_block == block) {
					alarm(0);
					end_pk_time(); /* set packet end time */
					break;
				}
				/* An ack for a future block is illegal */
				if (ap->th_block > block) {
					alarm(0);
					nak(4);
					goto abort;
				}
				/* An ack for a previous block is ignored */
			}

		}
		block++;
	} while (size == SEGSIZE);

	if (finform && sbia) {
	    sprintf(sbsyslog, MSGSTR(LOGSND, "Sent %s to %s\n"), filename, 
		    sbia);
	    syslog(LOG_INFO, sbsyslog);
	}
abort:
#ifdef TFTPD_DEBUG
	if (debug_file != (FILE *)0)
		fclose(debug_file);
#endif /* TFTPD_DEBUG */
	(void) fclose(file);
}

justquit()
{
	exit(0);
}


/*
 * Receive a file.
 */
recvfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *w_init();
	register struct tftphdr *ap;    /* ack buffer */
	register int block = 0, n, size;


	signal(SIGALRM, timer);
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	do {
		timeout = 0;
		pkcb.pk_rxtcnt = 0;
		ap->th_opcode = htons((u_short)ACK);
		ap->th_block = htons((u_short)block);
		block++;
		(void) setjmp(timeoutbuf);
send_ack:
#ifdef TFTPD_DEBUG
		if (debug_file != (FILE *)0) {
			fprintf(debug_file, 
				"\n                 block:  %d\n", block);
			fflush(debug_file);
		}
#endif /* TFTPD_DEBUG */
		if (send(peer, ackbuf, 4, 0) != 4) {
			syslog(LOG_ERR, MSGSTR(SENDERR2, "tftpd: write: %m\n")); /*MSG*/
			if (frpthst && sbia) syslog(LOG_ERR, sbia);
			goto abort;
		}
		if (block > 1)
		write_behind(file, pf->f_convert);
		start_pk_time();	/* set packet start time */
		alarm(rexmtval);
		for ( ; ; ) {
			n = recv(peer, dp, PKTSIZE, 0);
			if (n < 0) {            /* really? */
				alarm(0);
				syslog(LOG_ERR, MSGSTR(RECERR2, "tftpd: read: %m\n")); /*MSG*/
				if (frpthst && sbia) syslog(LOG_ERR, sbia);
				goto abort;
			}
			dp->th_opcode = ntohs((u_short)dp->th_opcode);
			dp->th_block = ntohs((u_short)dp->th_block);
			if (dp->th_opcode == ERROR) {
				alarm(0);
				goto abort;
			}
			if (block == 1) {
				close(fd);
				fd = open(filename, O_WRONLY|O_TRUNC);
				if (fd < 0) {
					syslog(LOG_ERR, "tftpd: error opening file %s\n",filename);
					alarm(0);
					return (errno + 100);
				}
				file = fdopen(fd, "w");
				if (file == NULL) {
					syslog(LOG_ERR, "tftpd: error opening file descriptor %d\n",fd);
					alarm(0);
					return errno+100;
				}
			}
			if (dp->th_opcode == DATA) {
				/* The next block is expected */
				if (dp->th_block == block) {
					alarm(0);
					end_pk_time(); /* set packet end time */
					break;
				}
				/* A future block is illegal */
				if (dp->th_block > block) {
					alarm(0);
					nak(4);
					goto abort;
				}
				/* A previous block is ignored */
			}
		}

		/*  size = write(file, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, pf->f_convert);
		if (size != (n-4) || errno == ENOSPC) {  /* ahem */
			if (size < 0) nak(errno + 100);
			else nak(ENOSPACE);
			goto abort;
		}
	} while (size == SEGSIZE);
	write_behind(file, pf->f_convert);
	(void) fclose(file);            /* close data file */

	if (finform && sbia) {
	    sprintf(sbsyslog, MSGSTR(LOGRCV, "Recv %s from %s\n"), filename, 
		    sbia);
	    syslog(LOG_INFO, sbsyslog);
	}

	ap->th_opcode = htons((u_short)ACK);    /* send the "final" ack */
	ap->th_block = htons((u_short)(block));
	(void) send(peer, ackbuf, 4, 0);

	alarm(0);
	signal(SIGALRM, justquit);      /* just quit on timeout */
	alarm(rexmtval);
	n = recv(peer, dp, PKTSIZE, 0);
	alarm(0);
	if (n >= 4 &&                   /* if read some data */
	    dp->th_opcode == DATA &&    /* and got a data block */
	    block == dp->th_block) {	/* then my last ack was lost */
		(void) send(peer, ackbuf, 4, 0);     /* resend final ack */
	}
abort:
#ifdef TFTPD_DEBUG
	if (debug_file != (FILE *)0)
		fclose(debug_file);
#endif /* TFTPD_DEBUG */
	return;
}

err_load()
{
	int i;

	for(i=0; i<8; i++) 
		errmsgs[i].e_msg = &err_msg[i][0];

	errmsgs[0].e_code = EUNDEF;
	strncpy(&err_msg[0][0], MSGSTR(UNDEFERR, "Undefined error code"), 1015); /*MSG*/
	errmsgs[1].e_code = ENOTFOUND;
	strncpy(&err_msg[1][0], MSGSTR(BADFILE, "File not found"), 1015); /*MSG*/
	errmsgs[2].e_code = EACCESS;
	strncpy(&err_msg[2][0], MSGSTR(ILLACESS, "Access violation"), 1015); /*MSG*/
	errmsgs[3].e_code = ENOSPACE;
	strncpy(&err_msg[3][0], MSGSTR(DISKFULL, "Disk full or allocation exceeded"), 1015); /*MSG*/
	errmsgs[4].e_code = EBADOP;
	strncpy(&err_msg[4][0], MSGSTR(ILLOPE, "Illegal TFTP operation"), 1015); /*MSG*/
	errmsgs[5].e_code = EBADID;
	strncpy(&err_msg[5][0], MSGSTR(BADID, "Unknown transfer ID"), 1015); /*MSG*/
	errmsgs[6].e_code = EEXISTS;
	strncpy(&err_msg[6][0], MSGSTR(FILEXIST, "File already exists"), 1015); /*MSG*/
	errmsgs[7].e_code = ENOUSER;
	strncpy(&err_msg[7][0], MSGSTR(BADUSER, "No such user"), 1015); /*MSG*/
	errmsgs[8].e_code = -1;
	errmsgs[8].e_msg = 0;
}

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
	extern char *sys_errlist[];


	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = sys_errlist[error - 100];
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (send(peer, buf, length, 0) != length) {
		syslog(LOG_ERR, "nak: %m\n");
		if (frpthst && sbia) syslog(LOG_ERR, sbia);
	}
}

