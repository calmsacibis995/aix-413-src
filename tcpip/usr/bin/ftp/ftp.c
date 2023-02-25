static char sccsid[] = "@(#)12      1.26.2.15  src/tcpip/usr/bin/ftp/ftp.c, tcpfilexfr, tcpip41J, 9519A_all 5/1/95 11:21:40";
/* 
 * COMPONENT_NAME: TCPIP ftp.c
 * 
 * FUNCTIONS: MSGSTR, abortpt, abortrecv, abortsend, cmdabort, 
 *            command, dataconn, empty, getreply, gunique, hookup, 
 *            initconn, is_netrc_restricted, login, proxtrans, psabort, 
 *            pswitch, ptransfer, recvrequest, reset, sendrequest, 
 *            tvsub 
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

#include "ftp_var.h"

#ifdef _AIX
#include <sys/access.h>
#endif _AIX
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
/*** #include <sys/param.h> *** already included by ftp_var.h ***/

#include <netinet/in.h>
#include <arpa/ftp.h>
#include <arpa/telnet.h>
#include <arpa/nameser.h>  /* for MAXDNAME */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <pwd.h>
#include <paths.h>

#include <nl_types.h>
#include "ftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTP,n,s) 

#ifdef _CSECURITY
#include <syslog.h>
#include "tcpip_audit.h"
#endif _CSECURITY

struct	sockaddr_in hisctladdr;
struct	sockaddr_in data_addr;
int	data = -1;
int	abrtflag = 0;
int	ptflag = 0;
/*XYZ int	connected; *XYZ*/ /* is int connected in ftp_var.h */
struct	sockaddr_in myctladdr;
uid_t	getuid();

FILE	*cin, *cout;
FILE	*dataconn();

#ifdef _CSECURITY
extern struct hostent *audit_hostent;
extern char *audit_tail[];
extern char *sys_errlist[];
extern uid_t saved_uid, effective_uid;
extern char username[64];
#endif _CSECURITY

/* bit mask used for TYPE L formatting */
static unsigned char bit_mask[] =
{
	0,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff
};
/* Ebcdic new line character */
#define EBCDIC_NL 0x15
/* This is an EBCDIC to ASCII conversion table
*/
unsigned char etoa [] =
{
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0325,0056,0074,0050,0053,0174,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0041,0044,0052,0051,0073,0176,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0313,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0136,0314,0315,0316,0317,0320,
	0321,0345,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0133,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0135,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};

/* This is an ASCII to EBCDIC conversion table
*/
unsigned char atoe [] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0232,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0137,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0152,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0112,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0241,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};

/* This is another (slightly different) ASCII to EBCDIC conversion table
*/
unsigned char atoibm[] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};

char *
hookup(host, port)
	char *host;
	int port;
{
	register struct hostent *hp = 0;
	int s,len;
	static char hostnamebuf[MAXDNAME];

	user_port = port ;
	bzero((char *)&hisctladdr, sizeof (hisctladdr));
	hisctladdr.sin_addr.s_addr = inet_addr(host);

	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
                if ((hp = gethostbyname (host)) == (struct hostent *) 0) {
                        fprintf(stderr,MSGSTR(NME_NT_FND,
                                            "ftp: Unknown host %s\n"), host);
			code = -1;
			return((char *) 0);
                }
		hisctladdr.sin_family = hp->h_addrtype;
                bcopy(hp->h_addr_list[0],
                    (caddr_t)&hisctladdr.sin_addr, hp->h_length);
                (void) strncpy(hostnamebuf, hp->h_name, sizeof(hostnamebuf));
        }
        else {
                if (hisctladdr.sin_addr.s_addr == -1) {
                        fprintf(stderr, MSGSTR(UNKNOWNHOST,
                                        "ftp: Address %s misformed\n"),host);
			return((char *) 0);
                }
		hisctladdr.sin_family = AF_INET;
		(void) strncpy(hostnamebuf, host, sizeof (hostnamebuf)); /*SH*/
            }
        
	hostname = hostnamebuf;
	s = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
	if (s < 0) {
		perror("ftp: socket");
		code = -1;
		return (0);
	}
	hisctladdr.sin_port = port;
	while (connect(s, &hisctladdr, sizeof (hisctladdr)) < 0) {
		if (hp && hp->h_addr_list[1]) {
			int oerrno = errno;

			fprintf(stderr, MSGSTR(CONNECT, "ftp: connect to address %s: "), /*MSG*/
				inet_ntoa(hisctladdr.sin_addr));
			errno = oerrno;
			perror((char *) 0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0],
			     (caddr_t)&hisctladdr.sin_addr, hp->h_length);
			fprintf(stdout, MSGSTR(TRYING, "Trying %s...\n"), /*MSG*/
				inet_ntoa(hisctladdr.sin_addr));
			(void) close(s);
			s = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
			if (s < 0) {
				perror("ftp: socket");
				code = -1;
				return (0);
			}
			continue;
		}
		perror("ftp: connect");
		code = -1;
		goto bad;
	}
	len = sizeof (myctladdr);
	if (getsockname(s, (char *)&myctladdr, &len) < 0) {
		perror("ftp: getsockname");
		code = -1;
		goto bad;
	}
	cin = fdopen(s, "r");
	cout = fdopen(s, "w");
	if (cin == NULL || cout == NULL) {
		fprintf(stderr, MSGSTR(FDOPEN_FAIL, "ftp: fdopen failed.\n")); /*MSG*/
		if (cin)
			(void) fclose(cin);
		if (cout)
			(void) fclose(cout);
		code = -1;
		goto bad;
	}
	if (verbose)
		printf(MSGSTR(CONNECTED, "Connected to %s.\n"), hostname); /*MSG*/
	if (getreply(0) > 2) { 	/* read startup message from server */
		if (cin)
			(void) fclose(cin);
		if (cout)
			(void) fclose(cout);
		code = -1;
		goto bad;
	}
#ifdef SO_OOBINLINE
	{
	int on = 1;

	if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE, &on, sizeof(on))
		< 0 && debug) {
			perror("ftp: setsockopt");
		}
	}
#endif SO_OOBINLINE

	return (hostname);
bad:
	(void) close(s);
	return ((char *)0);
}

login(host)
	char *host;
{
	char tmp[80];
	char *user, *pass, *acct, *getlogin(), *getpass();
	int n, aflag = 0;

	user = pass = acct = 0;
#ifdef _CSECURITY
	(void) setuid(saved_uid);
	if (!(is_netrc_restricted("ftp"))) {
		(void) setuid(effective_uid);
#endif _CSECURITY
		/*
		 * check for host specified by user, as well as the
		 * result of gethostbyname(), saved by hookup() in
		 * hostname.
		 */
		/*if (ruserpass(host, &user, &pass, &acct) < 0) {*/
		if (rnetrc(host, &user, &pass, &acct) < 0) {
		    disconnect();
		    code = -1;
		    return(0);
		}
		if (!user && !pass && !acct) {
		    if (!proxy)
			macnum = 0;	/* defect 110604 */
		    if (rnetrc(hostname, &user, &pass, &acct) < 0) {
			disconnect();
			code = -1;
			return(0);
		    }
		}
#ifdef _CSECURITY
	}
	(void) setuid(effective_uid);
#endif _CSECURITY
	if (user == NULL) {
		char *myname = getlogin();

		if (myname == NULL) {
			struct passwd *pp = getpwuid(getuid());

			if (pp != NULL)
				myname = pp->pw_name;
		}
		bzero(tmp, sizeof(tmp));
		fflush(stdin);
		printf(MSGSTR(FTP_NAME, "Name (%s:%s): "), host, myname); /*MSG*/
		(void) fgets(tmp, sizeof(tmp) - 1, stdin);
		tmp[strlen(tmp) - 1] = '\0';
		if (*tmp == '\0')
			user = myname;
		else
			user = tmp;
	}
	n = command("USER %s", user);
	if (n == CONTINUE) {
		if (pass == NULL)
			pass = getpass("Password:");
		(void) signal(SIGINT,SIG_IGN);
		n = command("PASS %s", pass);
	}
	if (n == CONTINUE) {
		aflag++;
		if (acct == NULL)
			acct = getpass("Account:");
		n = command("ACCT %s", acct);
	}
	if (n != COMPLETE) {
		fprintf(stderr, MSGSTR(LOGIN_FAIL, "Login failed.\n")); /*MSG*/
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		NET_ACCESS_WRITE( make_in_addr(audit_hostent->h_addr),
		                  "ftp/tcp",
		                  user,
		                  MSGSTR(LOGIN_FAIL, "Login failed.\n"),
		                  -1);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return (0);
	}

#ifdef _CSECURITY
	(void) setuid(saved_uid);
	NET_ACCESS_WRITE( make_in_addr(audit_hostent->h_addr),
	                  "ftp/tcp",
	                  user,
	                  "Login accepted",
	                  0);
	(void) setuid(effective_uid);
#endif _CSECURITY

	if (!aflag && acct != NULL)
		(void) command("ACCT %s", acct);
	if (proxy)
		return(1);
	for (n = 0; n < macnum; ++n) {
		if (!strcmp("init", macros[n].mac_name)) {
			(void) strcpy(line, "$init");
			makeargv();
			domacro(margc, margv);
			break;
		}
	}
	return (1);
}

cmdabort()
{
	extern jmp_buf ptabort;

	printf("\n");
	(void) fflush(stdout);
	abrtflag++;
	if (ptflag)
		longjmp(ptabort,1);
}

/*VARARGS1*/
command(fmt, args)
	char *fmt;
{
	int r, (*oldintr)(), cmdabort();

	abrtflag = 0;
	if (debug) {
		printf("---> ");
		_doprnt(fmt, &args, stdout);
		printf("\n");
		(void) fflush(stdout);
	}
	if (cout == NULL) {
		perror (MSGSTR(NO_CONTROL, "No control connection for command")); /*MSG*/
		code = -1;
		return (0);
	}
	oldintr = signal(SIGINT,cmdabort);
	_doprnt(fmt, &args, cout);
	fprintf(cout, "\r\n");
	(void) fflush(cout);
	cpend = 1;
	r = getreply(!strcmp(fmt, "QUIT"));
	if (abrtflag && oldintr != SIG_IGN)
		(*oldintr)();
	(void) signal(SIGINT, oldintr);
	return(r);
}

#include <ctype.h>
char reply_string[BUFSIZ];

getreply(expecteof)
	int expecteof;
{
	register int c, n;
	register int dig;
	register char *cp;
	int originalcode = 0, continuation = 0, (*oldintr)(), cmdabort();
	int pflag = 0;
	char *pt = pasv;

	cp = reply_string;
	oldintr = signal(SIGINT,cmdabort);
	for (;;) {
		dig = n = code = 0;
		while ((c = getc(cin)) != '\n') {
			if (c == IAC) {     /* handle telnet commands */
				switch (c = getc(cin)) {
				case WILL:
				case WONT:
					c = getc(cin);
					fprintf(cout, "%c%c%c",IAC,WONT,c);
					(void) fflush(cout);
					break;
				case DO:
				case DONT:
					c = getc(cin);
					fprintf(cout, "%c%c%c",IAC,DONT,c);
					(void) fflush(cout);
					break;
				default:
					break;
				}
				continue;
			}
			dig++;
			if (c == EOF) {
				if (expecteof) {
					(void) signal(SIGINT,oldintr);
					code = 221;
					return (0);
				}
				lostpeer();
				if (verbose) {
					printf(MSGSTR(SERVICE_421, "421 Service not available, remote server has closed connection\n")); /*MSG*/
					(void) fflush(stdout);
				}
				code = 421;
				cpend = 0;
				return(4);
			}
			if (c != '\r' && (verbose > 0 ||
			    (verbose > -1 && n == '5' && dig > 4))) {
				if (proxflag &&
				   (dig == 1 || dig == 5 && verbose == 0))
					printf("%s:",hostname);
				(void) putchar(c);
			}
			if (dig < 4 && isdigit(c))
				code = code * 10 + (c - '0');
			if (!pflag && code == 227)
				pflag = 1;
			if (dig > 4 && pflag == 1 && isdigit(c))
				pflag = 2;
			if (pflag == 2) {
				if (c != '\r' && c != ')')
					*pt++ = c;
				else {
					*pt = '\0';
					pflag = 3;
				}
			}
			if (dig == 4 && c == '-') {
				if (continuation)
					code = 0;
				continuation++;
			}
			if (n == 0)
				n = c;
			*cp++ = c;
		}
		if (verbose > 0 || verbose > -1 && n == '5') {
			(void) putchar(c);
			(void) fflush (stdout);
		}
		if (continuation && code != originalcode) {
			if (originalcode == 0)
				originalcode = code;
			continue;
		}
		*cp = '\0';
		if (n != '1')
			cpend = 0;
		(void) signal(SIGINT,oldintr);
		/* RFC 1123:  don't interpret 421 specially */
		/*
		if (code == 421 || originalcode == 421)
			lostpeer();
		*/
		if (abrtflag && oldintr != cmdabort && oldintr != SIG_IGN)
			(*oldintr)();
		return (n - '0');
	}
}

empty(mask, sec)
	struct fd_set *mask;
	int sec;
{
	struct timeval t;

	t.tv_sec = (long) sec;
	t.tv_usec = 0;
	return(select(32, mask, (struct fd_set *) 0, (struct fd_set *) 0,
	    sec >= 0 ? &t : (struct timeval *) 0));
}

jmp_buf	sendabort;

abortsend()
{

	mflag = 0;
	abrtflag = 0;
	printf(MSGSTR(NSEND_ABORT, "\nsend aborted\n")); /*MSG*/
	(void) fflush(stdout);
	longjmp(sendabort, 1);
}

/*
 * Determine if the server has sent a 110 MARK response message.
 * If so, save it in the restart control file.
 * Return values:  
 *	0:	No response message
 *	1:	110 MARK response message
 *	-1:	response message was not 110 MARK
 */
getmark(fd, pn)
FILE *fd;
char *pn;
{
	struct fd_set readlist;
	struct timeval timeout;
	int ret;
	char *ptr;
	char buf[BUFSIZ];
	FILE *rest_file;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&readlist);
	FD_SET(fileno(fd), &readlist);
			ret = getreply(0);
			if (code == 110) {
				ptr = strrchr(pn, '/');
				if (ptr == NULL)
					ptr = pn;
				else
					ptr += 1;
				strcpy(rest_name, "/usr/tmp/");
				strcat(rest_name, ptr);
				rest_file = fopen(rest_name, "w");
				if (rest_file != NULL) {
					ptr = strstr(reply_string, "MARK");
					if (ptr != NULL) {
						ptr += 4;
						ptr += strspn(ptr, " ");
					}
					strcpy(buf, ptr);
					ptr = strrchr(buf, '\r');
					if (ptr == NULL)
						ptr = buf + strlen(buf);
					strcpy(ptr, "\n");
					fwrite(buf, 1, strlen(buf), rest_file);
					fclose(rest_file);
				}
				return(1);
			}
			else
				return(-1);
}

sendrequest(cmd, local, remote)
	char *cmd, *local, *remote;
{
	register char *bufp;
	FILE *fin = 0, *dout = 0, *mypopen();
	int (*closefunc)(), mypclose(), fclose(), (*oldintr)(), (*oldintp)();
	int abortsend();
	char buf[BUFSIZ], hbuf[BUFSIZ*2];
	char myhost[32];
	long bytes = 0, hashbytes = HASHBYTES;
	register int c, cnt, d;
	int buf_char;		/* char being output */
	int block_count;	/* no. of chars in output buffer */
	char *block_ptr;	/* ptr to output buffer */
	int read_count;		/* no. of bytes read */
	int mark_count;		/* seek point at restart marker */
	int useful_bits;	/* no. of bits to use from the current char */
	int total_bits;		/* out_bits + useful_bits */
	int space_bits;		/* no. of available bits in output char */
	int avail_bits;		/* no. of available bits in input char */
	int blksize = sizeof buf;
	struct stat st;
	struct timeval start, stop;
	FILE *rest_file;	/* restart control file */
	char rest_buf[100];	/* restart control file info */
	int rest_error;		/* restart control file error flag */
	char *ptr;
	int comret;
	int err = 0;

	/* check to see if transfer is local P31549 */

        (void) gethostname(myhost, sizeof(myhost));

	if (!copyflag)
	{
        if ((!strcmp(myhost, hostname)) && (local != NULL || remote != NULL) && 
		(ftam_port != user_port)) {
             if (!strcmp(local, remote)) {
                   printf(MSGSTR(ILLEGAL_OP, "Illegal operation.  Use the COPY command \n")); /*MSG*/
                   printf(MSGSTR(ILLEGAL_OP_CONT, "local and remote hosts are the same as well as the filenames.\n")); /*MSG*/
                   return;
             }
        }
	}

	if (proxy) {
		proxtrans(cmd, local, remote);
#ifdef _CSECURITY
                syslog(LOG_INFO,
                        "ftp: EXPORT proxy %s, local %s, remote %s.",
                        username, audit_hostent->h_name, local, remote);
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(PROXY_TRANSFER, "Ftp proxy transfer"), 0);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return;
	}

	/*
	 * If doing restart, get the restart control info.
	 */
	if (rest_flag) {
	    rest_error = 1;
	    ptr = strrchr(local, '/');
	    if (ptr == NULL)
		ptr = local;
	    else
		ptr += 1;
	    strcpy(rest_name, "/usr/tmp/");
	    strcat(rest_name, ptr);
	    rest_file = fopen(rest_name, "r");
	    if (rest_file != NULL) {
		cnt = fread(rest_buf, 1, sizeof (rest_buf), rest_file);
		fclose(rest_file);
		if (cnt > 0) {
			cnt = sscanf(rest_buf, "%08d%02d%03d%03d%02d%03d",
				&rest_mark, &line_state, &car_ctl_char,
				&next_bits, &out_bits, &out_char);
			if ((cnt == 6) && (rest_mark > 0)) {
				ptr = strchr(rest_buf, '\n');
				if (ptr != NULL)
					*ptr = '\0';
				ptr = strchr(rest_buf, '=');
				if (ptr != NULL) {
					ptr++;
					ptr += strspn(ptr, " ");
					comret = command("REST %s", ptr);
					if (comret == CONTINUE) {
					    rest_error = 0;
					}
				}
			}
		}
	    }
	    if (rest_error) {
		rest_mark = 0;
		unlink(rest_name);
		fprintf(stdout, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid restart data\n"));
		return;
		}
	}
 
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	if (setjmp(sendabort)) {
		while (cpend) {
			(void) getreply(0);
		}
		if (data >= 0) {
			(void) close(data);
			data = -1;
		}
		if (oldintr)
			(void) signal(SIGINT,oldintr);
		if (oldintp)
			(void) signal(SIGPIPE,oldintp);
		code = -1;
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(FTP_ABORT, "Ftp abort"), -1);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return;
	}
	oldintr = signal(SIGINT, abortsend);
	if (strcmp(local, "-") == 0)
		fin = stdin;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE,SIG_IGN);
		fin = mypopen(local + 1, "r");
		if (fin == NULL) {
			perror(local + 1);
			(void) signal(SIGINT, oldintr);
			(void) signal(SIGPIPE, oldintp);
			code = -1;
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
				local, remote, sys_errlist[errno], errno);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
		closefunc = mypclose;
	} else {
		fin = fopen(local, "r");
		if (fin == NULL) {
			perror(local);
			(void) signal(SIGINT, oldintr);
			code = -1;
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
				local, remote, sys_errlist[errno], errno);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
		closefunc = fclose;
		if (fstat(fileno(fin), &st) < 0 ||
		    (st.st_mode&S_IFMT) != S_IFREG) {
			fprintf(stdout, MSGSTR(PLAIN_FILE, "%s: not a plain file.\n"), local);
			(void) signal(SIGINT, oldintr);
			fclose(fin);
			code = -1;
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(LOCAL_PLAIN, "Local file is not plain"), -1);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
	}
	if (initconn()) {
		(void) signal(SIGINT, oldintr);
		if (oldintp)
			(void) signal(SIGPIPE, oldintp);
		code = -1;
		if (closefunc != NULL)
		        (*closefunc) (fin);
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(LISTEN_CONN, "Can't listen for data connection"), -1);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return;
	}
	if (setjmp(sendabort))
		goto abort;
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if (oldintp)
				(void) signal(SIGPIPE, oldintp);
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(FTP_FAIL, "Ftp command fail"), -1);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
	} else
		if (command("%s", cmd) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if (oldintp)
				(void) signal(SIGPIPE, oldintp);
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(FTP_FAIL, "Ftp command fail"), -1);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
	dout = dataconn("w");
	if (dout == NULL)
		goto abort;

	(void) gettimeofday(&start, (struct timezone *)0);
	oldintp = signal(SIGPIPE, SIG_IGN);

#define DOPUTC(ch) \
	while (hash && (bytes >= hashbytes)) {\
		(void) putchar('#');\
		(void) fflush(stdout);\
		hashbytes += HASHBYTES;\
	}\
	(void) putc(ch, dout);\
	if (ferror(dout))\
		goto data_err;\
	bytes++;

	mark_count = 0;
	sscanf(bytename, "%d", &bytesize);

	switch (mode) {

	/*
	 * Format the data accordint to mode, structure, type and form.
	 */
	case MODE_S:
	    switch (stru) {

	    case STRU_F:
		switch (type) {
	
		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
                        if (fin == stdin) {
                                while ((c = getc(fin)) != EOF) {
                                        if (c == '\n') {
                                                DOPUTC('\r');
                                        }
                                        DOPUTC(c);
                                }
                                break;
                        } else {
                        errno = d = 0;
                        while ((c = read(fileno(fin), buf, sizeof (buf))) > 0) {

                            /* do translation */
                            for (d = cnt = 0; d < c; ) {
                                if ((hbuf[cnt++] = buf[d++]) == '\n') {
                                    hbuf[cnt-1] = '\r';
                                    hbuf[cnt++] = '\n';
                                }
                            }
                            bytes += cnt;
                            for (bufp = hbuf; cnt > 0; cnt -= d, bufp += d)
                                if ((d = write(fileno(dout), bufp, cnt)) <= 0)
                                    break;
                            if (d <= 0)
                                break;
                            if (hash) {
                                while (bytes >= hashbytes) {
                                    (void) putchar('#');
                                    hashbytes += HASHBYTES;
                                }
                                (void) fflush(stdout);
                            }
                        }
                        if (hash && bytes > 0) {
                            if (bytes < HASHBYTES)
                                (void) putchar('#');
                            (void) putchar('\n');
                            (void) fflush(stdout);
                        }
                        if (c < 0)
                            perror(local);
                        if (d <= 0) {
                            if (d == 0)
                                fprintf(stderr, MSGSTR( NETOUT,"netout: write returned 0?\n"));
                            else if ((err = errno) != EPIPE)
                                perror("netout");
                            else
                                /*
                                 * if net went down then try to recover
                                 * by aborting; this should reset peer
                                 * to command mode if the net is back up
                                 */
                                abort_remote((FILE *)NULL, -1, 0);
                            bytes = -1;
                        }
                        goto done;
			}

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC('1');
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(car_ctl_char);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC('\r');
					DOPUTC('\n');
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC('\r');
					DOPUTC('\n');
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				DOPUTC(c);
				line_state = IN_LINE;
			}
			break;

		    default:
			break;

		    }
		    break;
	
		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(fin)) != EOF) {
				if (c == '\n') {
					DOPUTC(EBCDIC_NL);
				}
				else {
					DOPUTC(atoe[c]);
				}
			}
			break;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC(atoe['1']);
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(atoe[car_ctl_char]);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC(EBCDIC_NL);
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC(EBCDIC_NL);
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				DOPUTC(atoe[c]);
				line_state = IN_LINE;
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_I:
			errno = d = 0;
			while ((c = read(fileno(fin), buf, sizeof (buf))) > 0) {
				bytes += c;
				for (bufp = buf; c > 0; c -= d, bufp += d)
					if ((d = write(fileno(dout), bufp, c)) <= 0)
						break;
				if (d <= 0)
					break;
				if (hash) {
					while (bytes >= hashbytes) {
						(void) putchar('#');
						hashbytes += HASHBYTES;
					}
					(void) fflush(stdout);
				}
			}
			if (hash && bytes > 0) {
				if (bytes < HASHBYTES)
					(void) putchar('#');
				(void) putchar('\n');
				(void) fflush(stdout);
			}
			if (c < 0)
				perror(local);
			if (d <= 0) {
				if (d == 0)
					fprintf(stderr, MSGSTR( NETOUT,"netout: write returned 0?\n"));
				else if ((err = errno) != EPIPE) 
					perror("netout");
				else
					/*
					 * if net went down then try to recover
					 * by aborting; this should reset peer
					 * to command mode if the net is back up
					 */
					abort_remote((FILE *)NULL, -1, 0);
				bytes = -1;
			}
			goto done;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */
			next_bits = bytesize;
			useful_bits = 0;
			out_bits = 0;
			out_char = '\0';
			
			while ((c = getc(fin)) != EOF) {
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesize;
				}
				c &= bit_mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits == NBBY) {
					DOPUTC(out_char);
					out_char = '\0';
					out_bits = 0;
				}
				else if (total_bits > NBBY) {
					DOPUTC(out_char);
					out_char = c << (NBBY - out_bits);
					out_bits = total_bits - NBBY;
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesize) {
				while (next_bits > 0) {
					DOPUTC(out_char);
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
				}
			}
			else if (out_bits > 0) {
				DOPUTC(out_char);
			}
			break;

		default:
		    break;

		}
		break;

	    case STRU_R:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			line_state = START_OF_LINE;
			while ((c = getc(fin)) != EOF) {
				line_state = IN_LINE;
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
				}
				else if (c == REC_ESC) {
					DOPUTC(REC_ESC);
					DOPUTC(REC_ESC);
				}
				else {
					DOPUTC(c);
				}
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC('1');
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(car_ctl_char);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				line_state = IN_LINE;
				if (c == REC_ESC) {
					DOPUTC(REC_ESC);
				}
				DOPUTC(c);
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			line_state = START_OF_LINE;
			while ((c = getc(fin)) != EOF) {
				line_state = IN_LINE;
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
				}
				else if (c == REC_ESC) {
					DOPUTC(REC_ESC);
					DOPUTC(REC_ESC);
				}
				else {
					DOPUTC(atoe[c]);
				}
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC(atoe['1']);
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(atoe[car_ctl_char]);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				line_state = IN_LINE;
				if (c == REC_ESC) {
					DOPUTC(REC_ESC);
				}
				DOPUTC(atoe[c]);
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_I:
		    line_state = START_OF_LINE;
		    block_count = 2;
		    while ((c = getc(fin)) != EOF) {
			line_state = IN_LINE;
			if (c == REC_ESC) {
				DOPUTC(REC_ESC);
				++block_count;
			}
			DOPUTC(c);
			++block_count;
			if (block_count >= blksize) {
				DOPUTC(REC_ESC);
				DOPUTC(REC_EOR);
				block_count = block_count % blksize + 2;
				line_state = START_OF_LINE;
			}
		    }
		    DOPUTC(REC_ESC);
		    if (line_state != START_OF_LINE) {
			DOPUTC(REC_EOR | REC_EOF);
		    }
		    else {
			DOPUTC(REC_EOF);
		    }
		    break;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */
			line_state = START_OF_LINE;
			block_count = 2;
			next_bits = bytesize;
			useful_bits = 0;
			out_bits = 0;
			out_char = '\0';
			
			while ((c = getc(fin)) != EOF) {
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesize;
				}
				c &= bit_mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits >= NBBY) {
					line_state = IN_LINE;
					out_char &= 0xff;
					if (out_char == REC_ESC) {
						DOPUTC(REC_ESC);
						++block_count;
					}
					DOPUTC(out_char);
					++block_count;
					if (block_count >= blksize) {
						DOPUTC(REC_ESC);
						DOPUTC(REC_EOR);
						block_count = 
						    block_count % blksize + 2;
						line_state = START_OF_LINE;
					}
					if (total_bits == NBBY) {
						out_char = '\0';
					}
					else {
						out_char = 
						    c << (NBBY - out_bits);
					}
					out_bits = total_bits - NBBY;
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesize) {
				line_state = IN_LINE;
				while (next_bits > 0) {
					if (out_char == REC_ESC) {
						DOPUTC(REC_ESC);
					}
					DOPUTC(out_char);
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
				}
			}
			else if (out_bits > 0) {
				line_state = IN_LINE;
				if (out_char == REC_ESC) {
					DOPUTC(REC_ESC);
				}
				DOPUTC(out_char);
			}
		    	DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		default:
		    break;

		}
		break;

	    default:
		break;

	    }
	    break;

	case MODE_B:

#define DOBUF(ch,d,sp)\
	while (hash && (bytes >= hashbytes)) {\
		(void) putchar('#');\
		(void) fflush(stdout);\
		hashbytes += HASHBYTES;\
	}\
	*bufp++ = ch;\
	++block_count;\
	DOENDBUF(d,sp)\

#define DOENDBUF(d,sp)\
	if ((block_count >= blksize - sp) || d) {\
		block_ptr[0] = d;\
		block_ptr[1] = (block_count - 3) >> 8;\
		block_ptr[2] = (block_count - 3) & 0xff;\
		if (fwrite(block_ptr, block_count, 1, dout) == 0)\
			goto data_err;\
		bytes += block_count;\
		if (!(d & BLK_EOF)) {\
			DORESTMARK\
		}\
		block_count = 3;\
		bufp = block_ptr;\
		bufp += block_count;\
		while (getmark(cin, local) > 0)\
			;\
	}\

#define DORESTMARK\
	if (read_count > rest_count) {\
		mark_count += read_count;\
		block_ptr[0] = BLK_RESTART;\
		block_ptr[1] = 0;\
		block_ptr[2] = 21;\
		if (next_bits < 0)\
			++block_ptr[2];\
		sprintf(&block_ptr[3], "%08d%02d%03d%03d%02d%03d",\
			mark_count, line_state, car_ctl_char,\
			next_bits, out_bits, out_char & 0xff);\
		if (fwrite(block_ptr, block_ptr[2]+3, 1, dout) == 0)\
			goto data_err;\
		bytes += block_ptr[2]+3;\
		read_count = 0;\
	}\

            /* rest_mark and other state vars. are set by the REST command */
	    if (rest_mark > 0) {

		if (fseek(fin, rest_mark, 0) != 0) {
		    fprintf(stdout, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid restart data\n"));
		    goto file_err;
		}
	        mark_count = rest_mark;
	    }
	    else {
		line_state = START_OF_LINE;
		car_ctl_char = ' ';
		next_bits = bytesize;
		out_bits = 0;
		out_char = '\0';
	    }

	    bufp = buf;
	    block_ptr = buf;
	    block_count = 3;
	    bufp += block_count;
	    read_count = 0;

	    switch (stru) {

	    case STRU_F:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:

			while ((c = getc(fin)) != EOF) {
				if ((c == '\n') && (line_state != DID_CR)) {
					line_state = DID_CR;
					DOBUF('\r', 0, 0)
				}
				++read_count;
				line_state = 0;
				DOBUF(c, 0, 0)
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF('1', 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(car_ctl_char, 0, 0);
				}
				if ((c == '\n') || (c == '\r')) {
					if (line_state != DID_CR) {
						line_state = DID_CR;
						DOBUF('\r', 0, 0);
					}
					++read_count;
					line_state = START_OF_LINE;
					if (c == '\n') {
						car_ctl_char = ' ';
					}
					if (c == '\r') {
						car_ctl_char = '+';
					}
					DOBUF('\n', 0, 0);
					continue;
				}
				++read_count;
				line_state = IN_LINE;
				DOBUF(c, 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			break;

		    }

		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(fin)) != EOF) {
				++read_count;
				if (c == '\n') {
					DOBUF(EBCDIC_NL, 0, 0);
				}
				else {
					DOBUF(atoe[c], 0, 0);
				}
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF(atoe['1'], 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(atoe[car_ctl_char], 0, 0);
				}
				++read_count;
				if (c == '\n') {
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					DOBUF(EBCDIC_NL, 0, 0);
					continue;
				}
				if (c == '\r') {
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					DOBUF(EBCDIC_NL, 0, 0);
					continue;
				}
				line_state = IN_LINE;
				DOBUF(atoe[c], 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			break;

		    }

		    break;

		case TYPE_I:
		    while ((cnt = read(fileno(fin), bufp, (u_int)blksize-3)) 
				> 0){
			read_count += cnt;
			block_count += cnt;
			DOENDBUF(0, blksize)
			while (hash && (bytes >= hashbytes)) {
				(void) putchar('#');
				(void) fflush(stdout);
				hashbytes += HASHBYTES;
			}
		    }
		    DOENDBUF(BLK_EOF, 0)
		    break;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */
			
			while ((c = getc(fin)) != EOF) {
				++read_count;
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesize;
				}
				c &= bit_mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits == NBBY) {
					buf_char = out_char;
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
				else if (total_bits > NBBY) {
					buf_char = out_char;
					out_char = c << (NBBY - out_bits);
					out_bits = total_bits - NBBY;
					DOBUF(buf_char, 0, 0);
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesize) {
				while (next_bits > 0) {
					buf_char = out_char;
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
			}
			else if (out_bits > 0) {
				DOBUF(out_char, 0, 0)
			}
			DOENDBUF(BLK_EOF, 0)

			break;

		default:
		    break;
			
		}
		break;

	    case STRU_R:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(fin)) != EOF) {
				++read_count;
				if (c == '\n') {
					DOENDBUF(BLK_EOR, 0)
				}
				else {
					DOBUF(c, 0, 0)
				}
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF('1', 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(car_ctl_char, 0, 0);
				}
				if ((c == '\n') || (c == '\r')) {
					++read_count;
					line_state = START_OF_LINE;
					if (c == '\n') {
						car_ctl_char = ' ';
					}
					if (c == '\r') {
						car_ctl_char = '+';
					}
					DOENDBUF(BLK_EOR, 0)
					continue;
				}
				++read_count;
				line_state = IN_LINE;
				DOBUF(c, 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			break;

		    }

		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(fin)) != EOF) {
				++read_count;
				if (c == '\n') {
					DOENDBUF(BLK_EOR, 0)
				}
				else {
					DOBUF(atoe[c], 0, 0);
				}
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(fin)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF(atoe['1'], 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(atoe[car_ctl_char], 0, 0);
				}
				++read_count;
				if ((c == '\n') || (c == '\r')) {
					line_state = START_OF_LINE;
					if (c == '\n') {
						car_ctl_char = ' ';
					}
					if (c == '\r') {
						car_ctl_char = '+';
					}
					DOENDBUF(BLK_EOR, 0)
					continue;
				}
				line_state = IN_LINE;
				DOBUF(atoe[c], 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			break;

		    }

		    break;

		case TYPE_I:
		    while ((cnt = read(fileno(fin), bufp, (u_int)blksize-3)) 
				> 0){
			read_count += cnt;
			block_count += cnt;
			DOENDBUF(BLK_EOR, 0)
			while (hash && (bytes >= hashbytes)) {
				(void) putchar('#');
				(void) fflush(stdout);
				hashbytes += HASHBYTES;
			}
		    }
		    DOENDBUF(BLK_EOF, 0)
		    break;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */
			while ((c = getc(fin)) != EOF) {
				++read_count;
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesize;
				}
				c &= bit_mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits == NBBY) {
					buf_char = out_char;
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
				else if (total_bits > NBBY) {
					buf_char = out_char;
					out_char = c << (NBBY - out_bits);
					out_bits = total_bits - NBBY;
					DOBUF(buf_char, 0, 0);
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesize) {
				while (next_bits > 0) {
					buf_char = out_char;
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
			}
			else if (out_bits > 0) {
				DOBUF(out_char, 0, 0)
			}
			DOENDBUF(BLK_EOR | BLK_EOF, 0)

			break;

		default:
		    break;
			
		}
		break;

	    default:
		break;

	    }
	    break;

	default:
	    break;

	}

data_err:

file_err:

done:

	if (hash) {
		if (bytes < hashbytes)
			(void) putchar('#');
		(void) putchar('\n');
		(void) fflush(stdout);
	}
	if (ferror(fin))
		perror(local);
	if (ferror(dout)) {
		if ((err = errno) != EPIPE)
			perror("netout");
		else
			/*
			 * if net went down then try to recover by
			 * aborting; this should reset peer to command
			 * mode if the net is back up
			 */
			abort_remote((FILE *)NULL, -1, 0);
		bytes = -1;
	}
	(void) gettimeofday(&stop, (struct timezone *)0);
	if (closefunc != NULL)
		(*closefunc)(fin);
	(void) fclose(dout);
	/* Get remaining 110 MARK responses. */
	if (mode == MODE_B) {
		while (cpend)
			getmark(cin, local);
		/*
		 * Keep restart file if the transfer failed or
		 * the debug level is 2.
		 */
		if ((code >= 200) && (code <= 299) && (debug != 2)) {
			if (*rest_name)
				unlink(rest_name);
		}
	}
	else if (err != EPIPE)  /* if EPIPE then we already aborted */
		(void) getreply(0);
	rest_mark = 0;
	rest_name[0] = '\0';
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	if (bytes > 0)
		ptransfer(0, bytes, &start, &stop, local, remote);
#ifdef _CSECURITY
	if (bytes > 0) {
                syslog(LOG_INFO,
                        "ftp: EXPORT user %s, host %s, local %s, remote %s.",
                        username, audit_hostent->h_name, local ? local:"NULL",
                                                remote ? remote:"NULL");
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
			local, remote, "", 0);
		(void) setuid(effective_uid);
	} else {
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
			local, remote,
			MSGSTR(ZERO_TRANS, "Zero bytes transferred"),-1);
		(void) setuid(effective_uid);
	}
#endif _CSECURITY
	return;
abort:
#ifdef _CSECURITY
	if (bytes > 0) {
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
			local,remote,MSGSTR(PARTIAL_ABORT,
			"Ftp abort with partial file transfer"),-1);
		(void) setuid(effective_uid);
	} else {
		(void) setuid(saved_uid);
		EXPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
			local,remote,MSGSTR(FTP_ABORT,
			"Ftp abort"), -1);
		(void) setuid(effective_uid);
	}
#endif _CSECURITY
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	if (!cpend) {
		code = -1;
		return;
	}
	if (data >= 0) {
		(void) close(data);
		data = -1;
	}
	if (dout)
		(void) fclose(dout);
	/* Get remaining 110 MARK responses. */
	if (mode == MODE_B) {
		while (cpend)
			getmark(cin, local);
		/*
		 * Keep restart file if the transfer failed or
		 * the debug level is 2.
		 */
		if ((code >= 200) && (code <= 299) && (debug != 2)) {
			if (*rest_name)
				unlink(rest_name);
		}
	}
	else
		(void) getreply(0);
	code = -1;
	if (closefunc != NULL && fin != NULL)
		(*closefunc)(fin);
	if (bytes > 0 )
		ptransfer(0, bytes, &start, &stop, local, remote);
}

jmp_buf	recvabort;

abortrecv()
{

	mflag = 0;
	abrtflag = 0;
	printf("\n");
	(void) fflush(stdout);
	longjmp(recvabort, 1);
}

/*
 * Restore the state to that saved in the following variables.
 */
static int oldtype = 0, oldmode = 0, oldstru = 0, oldform = 0;
static char oldbytename[4];
rest_state()
{
	char *argp[3];
	switch (oldmode) {
	case MODE_S:
		setstream();
		break;
	case MODE_B:
		setblock();
		break;
	}
	switch (oldstru) {
	case STRU_F:
		setfile();
		break;
	case STRU_R:
		setrecord();
		break;
	}
	switch (oldtype) {
	case TYPE_A:
		setascii();
		break;
	case TYPE_E:
		setebcdic();
		break;
	case TYPE_I:
		setbinary();
		break;
	case TYPE_L:
		argp[0] = "local";
		argp[1] = oldbytename;
		argp[2] = (char *)0;
		setlocal(2, argp);
		break;
	}
	switch (oldform) {
	case FORM_N:
		setnon_print();
		break;
	case FORM_T:
		settelnet();
		break;
	case FORM_C:
		setcar_ctl();
		break;
	}
}

/*
 * Write into to the restart control file.
 */
domark(send_mark, pn)
char *send_mark;
char *pn;
{
	char *ptr;
	char buf[BUFSIZ];
	FILE *rest_file;

	ptr = strrchr(pn, '/');
	if (ptr == NULL)
		ptr = pn;
	else
		ptr += 1;
	strcpy(rest_name, "/usr/tmp/");
	strcat(rest_name, ptr);
	rest_file = fopen(rest_name, "w");
	if (rest_file != NULL) {
		strcpy(buf, send_mark);
		sprintf(buf + strlen(buf),
			" = %08d%02d%03d%03d%02d%03d",
               		write_count, line_state, car_ctl_char,
               		next_bits, out_bits, out_char & 0xff);
		strcpy(buf + strlen(buf), "\n");
		fwrite(buf, 1, strlen(buf), rest_file);
		fclose(rest_file);
	}
}

recvrequest(cmd, local, remote, file_mode, share)
	char *cmd, *local, *remote, *file_mode;
	int share;
{
	FILE *fout, *din = 0, *mypopen();
	int (*closefunc)(), mypclose(), fclose(), (*oldintr)(), (*oldintp)(); 
	int abortrecv(), oldverbose; 
	int is_retr, tcrflag, nfnd, fdout, oflag;
	char *bufp, *gunique(), msg;
	static char *buf, *hbuf;
	static int bufsize = 0;
	long bytes = 0, hashbytes = HASHBYTES;
	char myhost[32];
	struct fd_set mask;
	register int c, cnt, d;
	int block_count;	/* no. of chars in output buffer */
	int block_code;		/* the opcode in the block header */
	int read_count;		/* no. of bytes read */
	struct timeval start, stop;
	struct stat st;
	FILE *rest_file;	/* restart control file */
	char rest_buf[100];	/* restart control file info */
	int rest_error;		/* restart control file error flag */
	char *ptr;
	int comret;		/* command return value */

	/* check to see if transfer is local P34218 */

        if (!copyflag)
	{
        (void) gethostname(myhost, sizeof(myhost));
        if ((!strcmp(myhost, hostname)) && (local != NULL || remote != NULL) &&
		(ftam_port != user_port)) { 
             if (!strcmp(local, remote)) {
		   printf(MSGSTR(ILLEGAL_OP, "Illegal operation.  Use the COPY command \n")); /*MSG*/
		   printf(MSGSTR(ILLEGAL_OP_CONT, "local and remote hosts are the same as well as the filenames.\n")); /*MSG*/
		   return;
	      }
        }
	}
 
	is_retr = strcmp(cmd, "RETR") == 0;
	if (proxy && is_retr) {
		proxtrans(cmd, local, remote);
#ifdef _CSECURITY
                syslog(LOG_INFO,
                        "ftp: IMPORT proxy %s, local %s, remote %s.",
                        username, audit_hostent->h_name, local, remote);
		(void) setuid(saved_uid);
		IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
			local, remote, MSGSTR(PROXY_TRANSFER,
			"Ftp proxy transfer"), 0);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return;
	}

        /*
         * If doing restart, get the restart control info.
         */
	if (rest_flag) {
	    rest_error = 1;
	    ptr = strrchr(local, '/');
	    if (ptr == NULL)
		ptr = local;
	    else
		ptr += 1;
	    strcpy(rest_name, "/usr/tmp/");
	    strcat(rest_name, ptr);
	    rest_file = fopen(rest_name, "r");
	    if (rest_file != NULL) {
		cnt = fread(rest_buf, 1, sizeof (rest_buf), rest_file);
		fclose(rest_file);
		if (cnt > 0) {
			ptr = strchr(rest_buf, '\n');
			if (ptr != NULL)
				*ptr = '\0';
			ptr = strchr(rest_buf, '=');
			if (ptr != NULL) {
				ptr++;
				ptr += strspn(ptr, " ");
				cnt = sscanf(ptr, 
					"%08d%02d%03d%03d%02d%03d",
					&rest_mark, &line_state, &car_ctl_char,
					&next_bits, &out_bits, &out_char);
				if (rest_mark > 0) {
					ptr = strchr(rest_buf, ' ');
					if (ptr != NULL)
						*ptr = '\0';
					comret = command("REST %s", rest_buf);
					if (comret == CONTINUE) {
						file_mode = "r+";
						rest_error = 0;
					}
				}
			}
		}
	    }
	    if (rest_error) {
		rest_mark = 0;
		unlink(rest_name);
		fprintf(stdout, MSGSTR(BAD_REST,
	   		"Requested action not taken:  invalid restart data\n"));
		return;
		}
	}

	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	tcrflag = !crflag && is_retr;
	if (setjmp(recvabort)) {
		while (cpend) {
			(void) getreply(0);
		}
		if (data >= 0) {
			(void) close(data);
			data = -1;
		}
		if (oldintr)
			(void) signal(SIGINT, oldintr);
		code = -1;
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local,
			remote, MSGSTR(FTP_ABORT, "Ftp abort"), -1);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return;
	}
	oldintr = signal(SIGINT, abortrecv);
	if (strcmp(local, "-") && *local != '|') {
		if (access(local, 2) < 0) {
			char *dirp, *dir;

			if (dir = (char *) malloc((unsigned) strlen(local) + 1))
				(void) strcpy(dir, local);
			else {
				perror("malloc");
				exit(1);
			}

			if (errno != ENOENT && errno != EACCES) {
				perror(local);
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef _CSECURITY
				(void) setuid(saved_uid);
				IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
					local,remote,sys_errlist[errno], errno);
				(void) setuid(effective_uid);
#endif _CSECURITY
				return;
			}
			if ((dirp = rindex(dir, '/')) != NULL) {
				/*
				 * Now see if the directory is root
				 */
				if ((dirp == dir) && (*dirp == '/')) 
					d = access("/", 2);
				else {
					/*
					 * Otherwise set the end of dir to null
					 */
					*dirp = 0;
					d = access(dir, 2);
				}
			} else
				d = access(".", 2);
			(void) free(dir);
			if (d < 0) {
				perror(local);
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef _CSECURITY
				(void) setuid(saved_uid);
				IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
					local,remote,sys_errlist[errno], errno);
				(void) setuid(effective_uid);
#endif _CSECURITY
				return;
			}
			if (!runique && errno == EACCES &&
#ifdef _AIX
			    acl_set(local, R_ACC|W_ACC, NO_ACC, NO_ACC) < 0)
#else
			    chmod(local,0600) < 0)
#endif _AIX
						   {
				perror(local);
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef _CSECURITY
				(void) setuid(saved_uid);
				IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
					local,remote,sys_errlist[errno], errno);
				(void) setuid(effective_uid);
#endif _CSECURITY
				return;
			}
			if (runique && errno == EACCES &&
			   (local = gunique(local)) == NULL) {
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef _CSECURITY
				(void) setuid(saved_uid);
				IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr),
					local,remote,sys_errlist[errno], errno);
				(void) setuid(effective_uid);
#endif _CSECURITY
				return;
			}
		}
		else if (runique && (local = gunique(local)) == NULL) {
			(void) signal(SIGINT, oldintr);
			code = -1;
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(NO_OVRWT, "Can't overwrite"), -1);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
	}
	if (initconn()) {
		(void) signal(SIGINT, oldintr);
		code = -1;
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(LISTEN_CONN, "Can't listen for data connection"), -1);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return;
	}
	if (setjmp(recvabort))
		goto abort;
	/*
	 * If not doing get, save the current transfer state and set to
	 * stream, file, ascii, non-print.
	 */
	if ( !is_retr) {
		oldverbose = verbose;
		if (!debug)
			verbose = 0;
		if (mode == MODE_S)
			oldmode = 0;
		else {
			oldmode = mode;
			setstream();
		}
		if (stru == STRU_F)
			oldstru = 0;
		else {
			oldstru = stru;
			setfile();
		}
		if (type == TYPE_A)
			oldtype = 0;
		else {
			oldtype = type;
			if (type == TYPE_L)
				strcpy(oldbytename, bytename);
			setascii();
		}
		if (form == FORM_N)
			oldform = 0;
		else {
			oldform = form;
			setnon_print();
		}
		verbose = oldverbose;
	}
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if ( !is_retr) {
				if (!debug)
					verbose = 0;
				rest_state();
				verbose = oldverbose;
			}
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(FTP_FAIL, "Ftp command fail"), -1);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
	} else {
		if (command("%s", cmd) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if ( !is_retr) {
				if (!debug)
					verbose = 0;
				rest_state();
				verbose = oldverbose;
			}
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(FTP_FAIL, "Ftp command fail"), -1);
			(void) setuid(effective_uid);
#endif _CSECURITY
			return;
		}
	}
	din = dataconn("r");
	if (din == NULL)
		goto abort;
	if (strcmp(local, "-") == 0)
		fout = stdout;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE, SIG_IGN);
		fout = mypopen(local + 1, "w");
		if (fout == NULL) {
			perror(local+1);
			goto abort;
		}
		closefunc = mypclose;
	}
	else {
		fout = fopen(local, file_mode);
		if (fout == NULL) {
			perror(local);
			goto abort;
		}
		closefunc = fclose;
	}
	if (fstat(fileno(fout), &st) < 0 || st.st_blksize == 0)
		st.st_blksize = BUFSIZ;
	if (st.st_blksize > bufsize) {
		if (buf)
			(void) free(buf);
                if (hbuf)
                        (void) free(hbuf);
                buf = malloc(st.st_blksize +4);
		if (buf == NULL) {
			perror("malloc");
			bufsize = 0;
			goto abort;
		}
                hbuf = malloc(st.st_blksize +4);
                if (hbuf == NULL) {
                        perror("malloc");
                        bufsize = 0;
                        goto abort;
                }
		bufsize = st.st_blksize;
	}
	(void) gettimeofday(&start, (struct timezone *)0);

#define DOPUTC2(ch)\
	while (hash && (bytes >= hashbytes)) {\
		(void) putchar('#');\
		(void) fflush(stdout);\
		hashbytes += HASHBYTES;\
	}\
	(void) putc (ch, fout);\
	if (ferror(fout)) {\
		bytes = -1;\
		goto file_err;\
	}\
	write_count++;
#define MYMIN(a,b) ((a) < (b) ? (a) : (b))

	errno = 0;
	c = 0;
	block_code = 0;
	sscanf(bytename, "%d", &bytesize);

	switch (mode) {

        /*
         * Format the data accordint to mode, structure, type and form.
         */
	case MODE_S:
	    switch (stru) {

	    case STRU_F:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
                        if ( fout == stdout ) {
                                line_state = 0;
                                while ((c = getc(din)) != EOF) {
                                        bytes++;
                                        if (line_state == DID_CR) {
                                                if (c == '\n')
                                                        line_state = 0;
                                                else if ( c != '\r') {
                                                        line_state = 0;
                                                        DOPUTC2('\r')
                                                }
                                        }
                                        else {
                                                if ((!tcrflag) && (c == '\r')) {
                                                        line_state = DID_CR;
                                                        continue;
                                                }
                                        }
                                        DOPUTC2(c)
                                }
                                if (line_state == DID_CR) {
                                        DOPUTC2('\r')
                                }

                                break;
                        } else {

			line_state = 0;
                        while ((cnt = read(fileno(din), buf, bufsize)) > 0) {
                            int hind, i;

                            buf[cnt] = buf[cnt + 1] = '\n';
                            bytes += cnt;
                            hind = 0;
                            if(line_state == DID_CR) {
                                hbuf[0] = '\r';
                                hind = 1;
                            }

                            if(!tcrflag) {
                                for (i = 0; i < cnt; ) {
                                    if(((hbuf[hind++] = buf[i++])== '\n')
                                             && (hbuf[hind - 2] == '\r')) {
                                        hbuf[hind-- - 2] = '\n';
                                    }
				}
                                if(hbuf[hind - 1] == '\r') {
                                    hind--; /* save for possible overwrite */
                                    line_state = DID_CR;
                                }
                                else line_state = 0;
                            }

                            else {
                                for (i = 0; i < cnt; i++) {
                                    hbuf[hind] = buf[i];
                                    hind++;
                                }
                            }

                            if ((d = write(fileno(fout), hbuf, hind)) <= 0)
                                goto file_err;

                            if (d < hind) {
                                fprintf(stderr, MSGSTR(SHORTWRITE, "%s: short write\n"), local);
                                fprintf(stderr, MSGSTR(NOSPACE, "No space left on device/file size limit exceeded \n"));
                                goto abort;
                            }
                            while (hash && (bytes >= hashbytes)) {
                                (void) putchar('#');
                                (void) fflush(stdout);
                                hashbytes += HASHBYTES;
                            }
                        }  /* end of file */

			break;
			}

		    case FORM_C:
			c = getc(din);
			bytes++;
			line_state = IN_LINE;
			while ((c = getc(din)) != EOF) {
				bytes++;
				switch (line_state) {
				case START_OF_LINE:
				    switch (c) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == '\r') {
						line_state = DID_CR;
						continue;
					}
					break;
				case DID_CR:
					if (c == '\n') {
						line_state = START_OF_LINE;
						continue;
					}
					else if (c != '\r') {
						DOPUTC2('\r')
						line_state = IN_LINE;
					}
					break;
				default:
					break;
				}
				DOPUTC2(c)
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(din)) != EOF) {
				bytes++;
				if (c == EBCDIC_NL) {
					DOPUTC2('\n')
				}
				else {
					DOPUTC2(etoa[c])
				}
			}
			break;

		    case FORM_C:
			c = getc(din);
			bytes++;
			line_state = IN_LINE;
			while ((c = getc(din)) != EOF) {
				bytes++;
				switch (line_state) {
				case START_OF_LINE:
				    switch (etoa[c]) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == EBCDIC_NL) {
						line_state = START_OF_LINE;
						continue;
					}
					break;
				default:
					break;
				}
				DOPUTC2(etoa[c])
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_I:
			errno = d= 0 ;
			while ((cnt = read(fileno(din), buf, bufsize)) 
					> 0) {
				bytes += cnt;
				for(bufp=buf;cnt > 0; cnt -=d, bufp +=d) {
					if ((d=write(fileno(fout), bufp, cnt))
									   <= 0)
							goto file_err;
					if (d < cnt) {
						fprintf(stderr, MSGSTR(SHORTWRITE, "%s: short write\n"), local);
						fprintf(stderr, MSGSTR(NOSPACE, "No space left on device/filesize limit exceeded \n"));
						goto abort;
					}
				}
				while (hash && (bytes >= hashbytes)) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += HASHBYTES;
				}
			}
			if (cnt < 0)
				goto data_err;
			goto done;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */

			next_bits = bytesize;
			out_bits = 0;
			out_char = '\0';
			while ((c = getc(din)) != EOF) {
			    bytes++;
			    space_bits = NBBY - out_bits;
			    avail_bits = NBBY;
			    do {
				useful_bits = MYMIN(space_bits, avail_bits);
				useful_bits = MYMIN(useful_bits, next_bits);
				out_char |= (c & bit_mask[useful_bits]) >> out_bits;
				out_bits += useful_bits;
				avail_bits -= useful_bits;
				next_bits -= useful_bits;
				if (out_bits == NBBY || next_bits == 0) {
				    DOPUTC2(out_char);
				    out_char = '\0';
				    out_bits = 0;
				}
				c = (c << useful_bits);
				if (next_bits <= 0)
				    next_bits = bytesize;
			    } while (avail_bits >= next_bits);
			    out_bits = avail_bits;
			    next_bits -= out_bits;
			    out_char = c & bit_mask[out_bits];
			}
		    break;
	
		default:
		    break;

		}
		break;

	    case STRU_R:
		switch (type) {
		case TYPE_A:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			line_state = 0;
			while ((c = getc(din)) != EOF) {
				bytes++;
				if (line_state == DID_ESC) {
					line_state = 0;
					if (c == REC_ESC) {
						DOPUTC2(c)
						continue;
					}
					if (c & REC_EOR) {
						DOPUTC2('\n')
					}
					if (c & REC_EOF) {
						break;
					}
				}
				else {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
				}
			}
			break;

		    case FORM_C:
			c = getc(din);
			bytes++;
			line_state = IN_LINE;
			while ((c = getc(din)) != EOF) {
				bytes++;
				if (line_state == START_OF_LINE) {
				    switch (c) {
				    case REC_ESC:
					DOPUTC2('\n')
					line_state = DID_ESC;
					continue;
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				}
				else if (line_state == IN_LINE) {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
					continue;
				}
				else if (line_state == DID_ESC) {
					if (c == REC_ESC) {
						DOPUTC2(c)
						line_state = IN_LINE;
						continue;
					}
					if (c & REC_EOR) {
						line_state = START_OF_LINE;
					}
					if (c & REC_EOF) {
						break;
					}
					continue;
				}
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_E:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			line_state = 0;
			while ((c = getc(din)) != EOF) {
				c = etoa[c];
				bytes++;
				if (line_state == DID_ESC) {
					line_state = 0;
					if (c == REC_ESC) {
						DOPUTC2(c)
						continue;
					}
					if (c & REC_EOR) {
						DOPUTC2('\n')
					}
					if (c & REC_EOF) {
						break;
					}
				}
				else {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
				}
			}
			break;

		    case FORM_C:
			c = getc(din);
			bytes++;
			line_state = IN_LINE;
			while ((c = getc(din)) != EOF) {
				bytes++;
				c = etoa[c];
				if (line_state == START_OF_LINE) {
				    switch (c) {
				    case REC_ESC:
					DOPUTC2('\n')
					line_state = DID_ESC;
					continue;
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				}
				else if (line_state == IN_LINE) {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
					continue;
				}
				else if (line_state == DID_ESC) {
					if (c == REC_ESC) {
						DOPUTC2(c)
						line_state = IN_LINE;
						continue;
					}
					if (c & REC_EOR) {
						line_state = START_OF_LINE;
					}
					if (c & REC_EOF) {
						break;
					}
					continue;
				}
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_I:
			line_state = 0;
			while ((c = getc(din)) != EOF) {
				bytes++;
				if (line_state == DID_ESC) {
					line_state = 0;
					if (c == REC_ESC) {
						DOPUTC2(c)
						continue;
					}
					if (c & REC_EOF) {
						break;
					}
				}
				else {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
				}
			}
			break;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */

			next_bits = bytesize;
			out_bits = 0;
			out_char = '\0';
			while ((c = getc(din)) != EOF) {
			    bytes++;
			    if (c == REC_ESC) {
				c = getc(din);
			        bytes++;
				if (c == EOF)
				    break;
				if (c != REC_ESC) {
				    if (c & REC_EOF)
					break;
				    continue;
				}
			    }
			    space_bits = NBBY - out_bits;
			    avail_bits = NBBY;
			    do {
				useful_bits = MYMIN(space_bits, avail_bits);
				useful_bits = MYMIN(useful_bits, next_bits);
				out_char |= (c & bit_mask[useful_bits]) >> out_bits;
				out_bits += useful_bits;
				avail_bits -= useful_bits;
				next_bits -= useful_bits;
				if (out_bits == NBBY || next_bits == 0) {
				    DOPUTC2(out_char);
				    out_char = '\0';
				    out_bits = 0;
				}
				c = (c << useful_bits);
				if (next_bits <= 0)
				    next_bits = bytesize;
			    } while (avail_bits >= next_bits);
			    out_bits = avail_bits;
			    next_bits -= out_bits;
			    out_char = c & bit_mask[out_bits];
			}

			break;

		default:
		    break;

		}
		break;

	    default:
		break;

	    }
	    break;

	case MODE_B:

#define GETBLKHDR\
	if (fread(buf, 1, 3, din) != 3)\
		goto data_err;\
	bytes += 3;\
	block_code = buf[0];\
	block_count = buf[1] << NBBY | buf[2];

#define DORESTART\
	if (block_code & BLK_RESTART) {\
		if (fread(buf, 1, block_count, din) != block_count)\
		goto data_err;\
		bytes += block_count;\
		if (fsync(fileno(fout)) < 0)\
			goto file_err;\
		buf[block_count] = '\0';\
		domark(buf, local);\
		continue;\
	}

            /* rest_mark and other state vars. are set by the REST command */
	    if (rest_mark > 0) {

		if (fseek(fout, rest_mark, 0) != 0) {
		    fprintf(stdout, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid restart data\n"));
		    return(-1);
		}
	    }
	    else {
		line_state = START_OF_FILE;
		car_ctl_char = ' ';
		next_bits = bytesize;
		out_bits = 0;
		out_char = '\0';
	    }
	    write_count = ftell(fout);

	    switch (stru) {
	    case STRU_F:
		switch (type) {
		case TYPE_A:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				if (line_state == DID_CR) {
					if (c == '\n')
						line_state = 0;
					else if ( c != '\r') {
						line_state = 0;
						DOPUTC2('\r')
					}
				}
				else {
					if (c == '\r') {
						line_state = DID_CR;
						continue;
					}
				}
				DOPUTC2(c)
			    }
			}
			if (line_state == DID_CR) {
				DOPUTC2('\r')
			}
			break;

		    case FORM_C:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (c) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == '\r') {
						line_state = DID_CR;
						continue;
					}
					break;
				case DID_CR:
					if (c == '\n') {
						line_state = START_OF_LINE;
						continue;
					}
					else if (c != '\r') {
						DOPUTC2('\r')
						line_state = IN_LINE;
					}
					break;
				default:
					break;
				}
				DOPUTC2(c)
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_E:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				if (c == EBCDIC_NL) {
					DOPUTC2('\n')
				}
				else {
					DOPUTC2(etoa[c])
				}
			    }
			    /* The following check of BLK_EOR & BLK_EOF */
			    /* could be added because the ftp server on VM */
			    /* sets BLK_EOR instead of using ebcdic NL */
			    /* and does not set BLK_EOR when BLK_EOF is set. */
			 /*
			    if (block_code & BLK_EOR) {
				DOPUTC2('\n')
			    }
			    else if (block_code & BLK_EOF) {
				DOPUTC2('\n')
			    }
			 */
			}
			break;

		    case FORM_C:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (etoa[c]) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == EBCDIC_NL) {
						line_state = START_OF_LINE;
						continue;
					}
					break;
				default:
					break;
				}
				DOPUTC2(etoa[c])
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_I:
			errno = d= 0 ;
			while ((block_code & BLK_EOF) != BLK_EOF) {
			    GETBLKHDR
			    DORESTART
			    while (block_count > 0) {
			        read_count = MYMIN(block_count, bufsize);
				if ((cnt = fread(buf, 1, read_count, din)) 
					> 0) {
				    block_count -= cnt;
				    bytes += cnt;
				    write_count += cnt;
				    for(bufp=buf;cnt > 0; cnt -=d, bufp +=d) {
					if ((d = write(fileno(fout), 
						bufp, cnt)) <= 0)
					goto file_err;
					if (d < cnt) {
						fprintf(stderr, MSGSTR(SHORTWRITE, "%s: short write\n"), local);
						fprintf(stderr, MSGSTR(NOSPACE, "No space left on device/filesize limit exceeded \n"));
						goto abort;
					}
				    }
				    while (hash && (bytes >= hashbytes)) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += HASHBYTES;
				    }
				}
				else
				    goto data_err;
			    }
			}
			goto done;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				space_bits = NBBY - out_bits;
				avail_bits = NBBY;
				do {
				    useful_bits = MYMIN(space_bits, avail_bits);
				    useful_bits = MYMIN(useful_bits, next_bits);
				    out_char |= (c & bit_mask[useful_bits]) 
					>> out_bits;
				    out_bits += useful_bits;
				    avail_bits -= useful_bits;
				    next_bits -= useful_bits;
				    if (out_bits == NBBY || next_bits == 0) {
					DOPUTC2(out_char);
					out_char = '\0';
					out_bits = 0;
				    }
				    c = (c << useful_bits);
				    if (next_bits <= 0)
					next_bits = bytesize;
				} while (avail_bits >= next_bits);
				out_bits = avail_bits;
				next_bits -= out_bits;
				out_char = c & bit_mask[out_bits];
			    }
			}
		    break;

		default:
		    break;

		}
		break;

	    case STRU_R:
		switch (type) {
		case TYPE_A:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				DOPUTC2(c)
			    }
			    if (block_code & BLK_EOR) {
				DOPUTC2('\n')
			    }
			}
			break;

		    case FORM_C:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (c) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				default:
					break;
				}
				DOPUTC2(c)
			    }
			    if (block_code & BLK_EOR) {
				line_state = START_OF_LINE;
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_E:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				DOPUTC2(etoa[c])
			    }
			    if (block_code & BLK_EOR) {
				DOPUTC2('\n')
			    }
			}
			break;

		    case FORM_C:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (etoa[c]) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				default:
					break;
				}
				DOPUTC2(etoa[c])
			    }
			    if (block_code & BLK_EOR) {
				line_state = START_OF_LINE;
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			break;

		    }
		    break;

		case TYPE_I:
			errno = d= 0 ;
			while ((block_code & BLK_EOF) != BLK_EOF) {
			    GETBLKHDR
			    DORESTART
			    while (block_count > 0) {
			        read_count = MYMIN(block_count, bufsize);
				if ((cnt = fread(buf, 1, read_count, din)) 
					> 0) {
				    block_count -= cnt;
				    bytes += cnt;
				    write_count += cnt;
				    for(bufp=buf;cnt > 0; cnt -=d, bufp +=d) {
					if ((d = write(fileno(fout), 
						bufp, cnt)) <= 0)
					goto file_err;
					if (d < cnt) {
						fprintf(stderr, MSGSTR(SHORTWRITE, "%s: short write\n"), local);
						fprintf(stderr, MSGSTR(NOSPACE, "No space left on device/filesize limit exceeded \n"));
						goto abort;
					}
				    }
				    while (hash && (bytes >= hashbytes)) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += HASHBYTES;
				    }
				}
				else
				    goto data_err;
			    }
			}
			goto done;

		case TYPE_L:
                        /*
                         * Data is assumed to be stored so that each logical
                         * byte takes an integral number of storage bytes.
                         * For example, if the logical byte size were 36,
                         * each logical byte would take 5 storage bytes with
                         * the last 4 bits of the 5th byte unused.
                         */
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(din)) != EOF)) {
				bytes++;
				space_bits = NBBY - out_bits;
				avail_bits = NBBY;
				do {
				    useful_bits = MYMIN(space_bits, avail_bits);
				    useful_bits = MYMIN(useful_bits, next_bits);
				    out_char |= (c & bit_mask[useful_bits]) 
					>> out_bits;
				    out_bits += useful_bits;
				    avail_bits -= useful_bits;
				    next_bits -= useful_bits;
				    if (out_bits == NBBY || next_bits == 0) {
					DOPUTC2(out_char);
					out_char = '\0';
					out_bits = 0;
				    }
				    c = (c << useful_bits);
				    if (next_bits <= 0)
					next_bits = bytesize;
				} while (avail_bits >= next_bits);
				out_bits = avail_bits;
				next_bits -= out_bits;
				out_char = c & bit_mask[out_bits];
			    }
			}
		    break;

		default:
		    break;

		}
		break;

	    default:
		break;

	    }
	    break;

	default:
	    break;

	}

data_err:

file_err:

done:

	if (hash) {
		if (bytes < hashbytes)
			(void) putchar('#');
		(void) putchar('\n');
		(void) fflush(stdout);
	}
	if (ferror(din)) {
		if (errno != EPIPE)
			perror("netin");
		bytes = -1;
	}
	if (ferror(fout)) {
		perror(local);
		goto abort;
	}

	if (closefunc != NULL)
               if ((*closefunc)(fout) < 0 ) {
                        perror("Error writing file:");
                        goto abort;
                }
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) fclose(din);
	(void) getreply(0);
	/*
 	* Keep restart file if the transfer failed or
 	* the debug level is 2.
 	*/
	if ((code >= 200) && (code <= 299) && (debug != 2)) {
		if (*rest_name)
			unlink(rest_name);
	}
	rest_mark = 0;
	rest_name[0] = '\0';
	if (bytes > 0 && is_retr)
		ptransfer(1, bytes, &start, &stop, local, remote);
	if ( !is_retr) {
		if (!debug)
			verbose = 0;
		rest_state();
		verbose = oldverbose;
	}
#ifdef _CSECURITY
        syslog(LOG_INFO, "ftp: IMPORT user %s, host %s, local %s, remote %s.",
                        username, audit_hostent->h_name, local ? local:"NULL",
                                                remote ? remote:"NULL");
	(void) setuid(saved_uid);
	IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local,
		remote, "", 0);
	(void) setuid(effective_uid);
#endif _CSECURITY
	return;
abort:

#ifdef _CSECURITY
	if (bytes > 0) {
		(void) setuid(saved_uid);
		IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(PARTIAL_ABORT, "Ftp abort with partial file transfer"), -1);
		(void) setuid(effective_uid);
	} else {
		(void) setuid(saved_uid);
		IMPORT_DATA_WRITE(make_in_addr(audit_hostent->h_addr), local, remote, MSGSTR(FTP_ABORT, "Ftp abort"), -1);
		(void) setuid(effective_uid);
	}
#endif _CSECURITY

/* abort using RFC959 recommended IP,SYNC sequence  */

	(void) gettimeofday(&stop, (struct timezone *)0);
	if (oldintp)
		(void) signal(SIGPIPE, oldintr);
	(void) signal(SIGINT,SIG_IGN);
	if ( !is_retr) {
		if (!debug)
			verbose = 0;
		rest_state();
		verbose = oldverbose;
	}
	if (!cpend) {
		code = -1;
		(void) signal(SIGINT,oldintr);
		return;
	}

	abort_remote(din, 10, 1);
	code = -1;
	if (data >= 0) {
		(void) close(data);
		data = -1;
	}
	if (closefunc != NULL && fout != NULL)
		(*closefunc)(fout);
	if (din)
		(void) fclose(din);
	if (bytes > 0 )
		ptransfer(1, bytes, &start, &stop, local, remote);
	(void) signal(SIGINT,oldintr);
}

/*
 * Need to start a listen on the data channel
 * before we send the command, otherwise the
 * server's connect may fail.
 */
#ifdef aiws
int sendport = -1;
#else
static int sendport = -1;
#endif

initconn()
{
	register char *p, *a;
	int result, len, tmpno = 0;
	int on = 1;

noport:
	data_addr = myctladdr;
	if (sendport)
		data_addr.sin_port = 0;	/* let system pick one */ 
	if (data != -1)
		(void) close (data);
	data = socket(AF_INET, SOCK_STREAM, 0);
	if (data < 0) {
		perror("ftp: socket");
		if (tmpno)
			sendport = 1;
		return (1);
	}
	if (!sendport)
		if (setsockopt(data, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0) {
			perror(MSGSTR(REUSE_ADDR, "ftp: setsockopt (reuse address)")); /*MSG*/
			goto bad;
		}
	if (bind(data, (struct sockaddr *)&data_addr, sizeof (data_addr)) < 0) {
		perror("ftp: bind");
		goto bad;
	}
	if (options & SO_DEBUG &&
	    setsockopt(data, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof (on)) < 0)
		perror(MSGSTR(IGNORED, "ftp: setsockopt (ignored)")); /*MSG*/
	len = sizeof (data_addr);
	if (getsockname(data, (char *)&data_addr, &len) < 0) {
		perror("ftp: getsockname");
		goto bad;
	}
	if (listen(data, 1) < 0)
		perror("ftp: listen");
	if (sendport) {
		a = (char *)&data_addr.sin_addr;
		p = (char *)&data_addr.sin_port;
#define	UC(b)	(((int)b)&0xff)
		result =
		    command("PORT %d,%d,%d,%d,%d,%d",
		      UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]),
		      UC(p[0]), UC(p[1]));
		if (result == ERROR && sendport == -1) {
			sendport = 0;
			tmpno = 1;
			goto noport;
		}
		return (result != COMPLETE);
	}
	if (tmpno)
		sendport = 1;
	return (0);
bad:
	(void) close(data), data = -1;
	if (tmpno)
		sendport = 1;
	return (1);
}

FILE *
dataconn(mode)
	char *mode;
{
	struct sockaddr_in from;
	int s, fromlen = sizeof (from);

	s = accept(data, (struct sockaddr *) &from, &fromlen);
	if (s < 0) {
		perror("ftp: accept");
		(void) close(data), data = -1;
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		CONNECTION_WRITE(from.sin_addr, "ftp/tcp", "open",
			sys_errlist[errno], errno);
		(void) setuid(effective_uid);
#endif _CSECURITY
		return (NULL);
	}
#ifdef _CSECURITY
	(void) setuid(saved_uid);
	CONNECTION_WRITE(from.sin_addr, "ftp/tcp", "open", "", 0);
	(void) setuid(effective_uid);
#endif _CSECURITY
	(void) close(data);
	data = s;
	return (fdopen(data, mode));
}

ptransfer(direction, bytes, t0, t1, local, remote)
	char *local, *remote;
	int direction;
	long bytes;
	struct timeval *t0, *t1;
{
	struct timeval td;
	float s, bs;

	if (verbose) {
	    tvsub(&td, t1, t0);
	    s = (float)td.tv_sec + ((float)td.tv_usec / 1000000.);
#define	nz(x)	((x) == 0 ? 1 : (x))
	    bs = ((float)bytes) / nz(s);
	    if (direction) {
	    	printf(MSGSTR(BYTES_SEC_REC,"%ld bytes received in %.4g seconds (%.4g Kbytes/s)\n"),
		bytes, s, bs / 1024.);
	    }
	    else {
	    	printf(MSGSTR(BYTES_SEC_SND,"%ld bytes sent in %.4g seconds (%.4g Kbytes/s)\n"),
		bytes, s, bs / 1024.);
	    }
	    if (local && *local != '-')
                printf(MSGSTR(LOCAL, "local: %s "), local);
            if (remote)
                printf(MSGSTR(REMOTE, "remote: %s\n"), remote);
	}
}

/*tvadd(tsum, t0)
	struct timeval *tsum, *t0;
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
} */

tvsub(tdiff, t1, t0)
	struct timeval *tdiff, *t1, *t0;
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

psabort()
{
	extern int abrtflag;

	abrtflag++;
}

/*
 * Save state for current connection, restore state for other connection.
 * New values saved per RFC 1123.
 */
pswitch(flag)
	int flag;
{
	extern int proxy, abrtflag;
	int (*oldintr)();
	static struct comvars {
		int connect;
		char name[MAXDNAME];
		struct sockaddr_in mctl;
		struct sockaddr_in hctl;
		FILE *in;
		FILE *out;
		int com_mode;
		int com_stru;
		int com_type;
		int com_form;
		char com_modename[10];
		char com_structname[10];
		char com_typename[10];
		char com_formname[20];
		char com_bytename[4];
		char com_typecode[2];
		char com_formcode[2];
		int cpnd;
		int sunqe;
		int runqe;
		int mcse;
		int ntflg;
		char nti[17];
		char nto[17];
		int mapflg;
		char mi[MAXPATHLEN];
		char mo[MAXPATHLEN];
		} proxstruct, tmpstruct;
	struct comvars *ip, *op;

	abrtflag = 0;
	oldintr = signal(SIGINT, psabort);
	if (flag) {
		if (proxy)
			return;
		ip = &tmpstruct;
		op = &proxstruct;
		proxy++;
	}
	else {
		if (!proxy)
			return;
		ip = &proxstruct;
		op = &tmpstruct;
		proxy = 0;
	}
	ip->connect = connected;
	connected = op->connect;
	if (hostname) {
		(void) strncpy(ip->name, hostname, sizeof(ip->name) - 1);
		ip->name[strlen(ip->name)] = '\0';
	} else
		ip->name[0] = 0;
	hostname = op->name;
	ip->hctl = hisctladdr;
	hisctladdr = op->hctl;
	ip->mctl = myctladdr;
	myctladdr = op->mctl;
	ip->in = cin;
	cin = op->in;
	ip->out = cout;
	cout = op->out;
	ip->com_mode = mode;
	strcpy(ip->com_modename, modename);
	mode = op->com_mode;
	if (!mode) {
		mode = MODE_S;
		strcpy(modename, "stream");
	}
	else
		strcpy(modename, op->com_modename);
	ip->com_stru = stru;
	strcpy(ip->com_structname, structname);
	stru = op->com_stru;
	if (!stru) {
		stru = STRU_F;
		strcpy(structname, "file");
	}
	else
		strcpy(structname, op->com_structname);
	ip->com_type = type;
	strcpy(ip->com_typename, typename);
	strcpy(ip->com_typecode, typecode);
	if (type == TYPE_L)
		strcpy(ip->com_bytename, bytename);
	type = op->com_type;
	if (!type) {
		type = TYPE_A;
		strcpy(typename, "ascii");
		strcpy(typecode, "A");
	}
	else {
		strcpy(typename, op->com_typename);
		strcpy(typecode, op->com_typecode);
		if (type == TYPE_L)
			strcpy(bytename, op->com_bytename);
	}
	ip->com_form = form;
	strcpy(ip->com_formname, formname);
	strcpy(ip->com_formcode, formcode);
	form = op->com_form;
	if (!form) {
		form = FORM_N;
		strcpy(formname, "non-print");
		strcpy(formcode, "N");
	}
	else {
		strcpy(formname, op->com_formname);
		strcpy(formcode, op->com_formcode);
	}
	ip->cpnd = cpend;
	cpend = op->cpnd;
	ip->sunqe = sunique;
	sunique = op->sunqe;
	ip->runqe = runique;
	runique = op->runqe;
	ip->mcse = mcase;
	mcase = op->mcse;
	ip->ntflg = ntflag;
	ntflag = op->ntflg;
	(void) strncpy(ip->nti, ntin, 16);
	(ip->nti)[strlen(ip->nti)] = '\0';
	(void) strcpy(ntin, op->nti);
	(void) strncpy(ip->nto, ntout, 16);
	(ip->nto)[strlen(ip->nto)] = '\0';
	(void) strcpy(ntout, op->nto);
	ip->mapflg = mapflag;
	mapflag = op->mapflg;
	(void) strncpy(ip->mi, mapin, MAXPATHLEN - 1);
	(ip->mi)[strlen(ip->mi)] = '\0';
	(void) strcpy(mapin, op->mi);
	(void) strncpy(ip->mo, mapout, MAXPATHLEN - 1);
	(ip->mo)[strlen(ip->mo)] = '\0';
	(void) strcpy(mapout, op->mo);
	(void) signal(SIGINT, oldintr);
	if (abrtflag) {
		abrtflag = 0;
		(*oldintr)();
	}
}

jmp_buf ptabort;
int ptabflg;

abortpt()
{
	printf("\n");
	(void) fflush(stdout);
	ptabflg++;
	mflag = 0;
	abrtflag = 0;
	longjmp(ptabort, 1);
}

/*
 * Do a proxy transfer.  This may include the restart command per RFC 1123.
 */
proxtrans(cmd, local, remote)
	char *cmd, *local, *remote;
{
	int (*oldintr)(), abortpt(), secndflag = 0, nfnd;
	int tmpmode, tmpstru, tmptype, tmpform;
	char tmpbytename[4];
	extern jmp_buf ptabort;
	char *cmd2;
	struct fd_set mask;
	char *argp[3];
	int is_sending[2];
	int cnt;
	FILE *rest_file;
	char rest_buf[100];
	int rest_error;
	char *ptr;
	char *ptr2;
	int comret;
	char mark_r[40];
	char mark_s[40];

	/*
	 * DAB - since nobody has yet defined what stdin and stdout mean
	 *	 during a proxy command, we disallow it.
	 */
	if ((strcmp(local, "-") == 0) || (strcmp(remote, "-") == 0)) {
		printf(MSGSTR(PROXY, "Proxy does not allow use of \"-\".\n")); /*MSG*/
		return;
	}
	/* we also disallow pipes */
        if (local[0] == '|') {
		printf(MSGSTR(PROXY_PIPES, "Proxy does not allow use of pipes.\n")); /*MSG*/
		return;
	}
	    
	if (strcmp(cmd, "RETR")) {
		cmd2 = "RETR";
		is_sending[0] = 0;
		is_sending[1] = 1;
	}
	else {
		cmd2 = runique ? "STOU" : "STOR";
		is_sending[0] = 1;
		is_sending[1] = 0;
	}
	if (command("PASV") != COMPLETE) {
		printf(MSGSTR(PROXY_3RD, "proxy server does not support third part transfers.\n")); /*MSG*/
		return;
	}
	tmpmode = mode;
	tmpstru = stru;
	tmptype = type;
	if (type == TYPE_L)
		strcpy(tmpbytename, bytename);
	tmpform = form;
	pswitch(0);
	if (!connected) {
		printf(MSGSTR(PRIMARY, "No primary connection\n")); /*MSG*/
		pswitch(1);
		code = -1;
		return;
	}
	if (mode != tmpmode) {
		oldmode = mode;
		switch (tmpmode) {
		case MODE_S:
			setstream();
			break;
		case MODE_B:
			setblock();
			break;
		}
	}
	else
		oldmode = 0;
	if (stru != tmpstru) {
		oldstru = stru;
		switch (tmpstru) {
		case STRU_F:
			setfile();
			break;
		case STRU_R:
			setrecord();
			break;
		}
	}
	else
		oldstru = 0;
	if (type != tmptype) {
		oldtype = type;
		if (type == TYPE_L)
			strcpy(oldbytename, bytename);
		switch (tmptype) {
		case TYPE_A:
			setascii();
			break;
		case TYPE_I:
			setbinary();
			break;
		case TYPE_E:
			setebcdic();
			break;
		case TYPE_L:
			argp[0] = "local";
			argp[1] = tmpbytename;
			argp[2] = (char *)0;
			setlocal(2, argp);
			break;
		}
	}
	else
		oldtype = 0;
	if (form != tmpform) {
		oldform = form;
		switch (tmpform) {
		case FORM_N:
			setnon_print();
			break;
		case FORM_T:
			settelnet();
			break;
		case FORM_C:
			setcar_ctl();
			break;
		}
	}
	else
		oldform = 0;
	if (command("PORT %s", pasv) != COMPLETE) {
		rest_state();
		pswitch(1);
		return;
	}
	if (setjmp(ptabort))
		goto abort;
	oldintr = signal(SIGINT, abortpt);

        /*
         * If doing restart, get the restart control info.
	 * Send REST rrrr to receiving server, REST ssss to sending server.
         */
	if (rest_flag) {
	    rest_error = 1;
	    ptr = strrchr(local, '/');
	    if (ptr == NULL)
		ptr = local;
	    else
		ptr += 1;
	    strcpy(rest_name, "/usr/tmp/");
	    strcat(rest_name, ptr);
	    rest_file = fopen(rest_name, "r");
	    if (rest_file != NULL) {
		cnt = fread(rest_buf, 1, sizeof (rest_buf), rest_file);
		fclose(rest_file);
		if (cnt > 0) {
			ptr = strchr(rest_buf, '\n');
			if (ptr != NULL)
				*ptr = '\0';
			ptr = strchr(rest_buf, ' ');
			if (ptr != NULL) {
				*ptr = '\0';
				strcpy(mark_s, rest_buf);
				ptr++;
				ptr2 = strchr(ptr, '=');
				if (ptr2 != NULL) {
					ptr2++;
					ptr2 += strspn(ptr2, " ");
					strcpy(mark_r, ptr2);
				}
			}
			if (is_sending[0]) {
				comret = command("REST %s", mark_s);
				if (comret == CONTINUE)
					rest_error = 0;
			}
			else {
				comret = command("REST %s", mark_r);
				if (comret == CONTINUE)
					rest_error = 0;
			}
			pswitch(1);
			if (!rest_error) {
			    if (is_sending[1]) {
				comret = command("REST %s", mark_s);
				if (comret == CONTINUE)
					rest_error = 0;
			    }
			    else {
				comret = command("REST %s", mark_r);
				if (comret == CONTINUE)
					rest_error = 0;
			    }
			}
			pswitch(0);
		}
	    }
	    if (rest_error) {
		rest_mark = 0;
		unlink(rest_name);
		fprintf(stdout, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid restart data\n"));
		(void) signal(SIGINT, oldintr);
		rest_state();
		pswitch(1);
		return;
		}
	}

	if (command("%s %s", cmd, remote) != PRELIM) {
		(void) signal(SIGINT, oldintr);
		rest_state();
		pswitch(1);
		return;
	}
	sleep(2);
	pswitch(1);
	secndflag++;
	if (command("%s %s", cmd2, local) != PRELIM)
		goto abort;
	ptflag++;
	/*
	 * Get 110 MARK responses from the receiving server.
	 */
	if (mode == MODE_B) {
		if (is_sending[0]) {
			while (cpend)
				getmark(cin, local);
			/*
		 	* Keep restart file if the transfer failed or
		 	* the debug level is 2.
		 	*/
			if ((code >= 200) && (code <= 299) && (debug != 2)) {
				if (*rest_name)
					unlink(rest_name);
			}
			pswitch(0);
			(void) getreply(0);
		}
		else {
			pswitch(0);
			while (cpend)
				getmark(cin, local);
			/*
		 	* Keep restart file if the transfer failed or
		 	* the debug level is 2.
		 	*/
			if ((code >= 200) && (code <= 299) && (debug != 2)) {
				if (*rest_name)
					unlink(rest_name);
			}
			pswitch(1);
			(void) getreply(0);
			pswitch(0);
		}
	}
	else {
		(void) getreply(0);
		pswitch(0);
		(void) getreply(0);
	}
	(void) signal(SIGINT, oldintr);
	rest_state();
	pswitch(1);
	ptflag = 0;
	printf(MSGSTR(LOCAL_REMOTE, "local: %s remote: %s\n"), local, remote); /*MSG*/
	return;
abort:
	(void) signal(SIGINT, SIG_IGN);
	ptflag = 0;
	if (strcmp(cmd, "RETR") && !proxy)
		pswitch(1);
	else if (!strcmp(cmd, "RETR") && proxy)
		pswitch(0);
	if (!cpend && !secndflag) {  /* only here if cmd = "STOR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			rest_state();
			if (cpend)
				abort_remote((FILE *) NULL, 10, 1);
		}
		pswitch(1);
		if (ptabflg)
			code = -1;
		(void) signal(SIGINT, oldintr);
		return;
	}
	if (cpend)
		abort_remote((FILE *) NULL, 10, 1);
	pswitch(!proxy);
	if (!cpend && !secndflag) {  /* only if cmd = "RETR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			rest_state();
			if (cpend)
				abort_remote((FILE *) NULL, 10, 1);
			pswitch(1);
			if (ptabflg)
				code = -1;
			(void) signal(SIGINT, oldintr);
			return;
		}
	}
	if (cpend)
		abort_remote((FILE *) NULL, 10, 1);
	pswitch(!proxy);
	if (cpend) {
		FD_ZERO(&mask);
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask,10)) <= 0) {
			if (nfnd < 0) {
				perror("abort");
			}
			if (ptabflg)
				code = -1;
			lostpeer();
		}
		(void) getreply(0);
		(void) getreply(0);
	}
	if (proxy)
		pswitch(0);
	rest_state();
	pswitch(1);
	if (ptabflg)
		code = -1;
	(void) signal(SIGINT, oldintr);
}

reset()
{
	struct fd_set mask;
	int nfnd = 1;

	FD_ZERO(&mask);
	while (nfnd > 0) {
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask,0)) < 0) {
			perror("reset");
			code = -1;
			lostpeer();
		}
		else if (nfnd) {
			(void) getreply(0);
		}
	}
}

char *
gunique(local)
	char *local;
{
	static char new[MAXPATHLEN];
	char *cp = rindex(local, '/');
	int d, count=0;
	char ext = '1';

	if (cp)
		*cp = '\0';
	d = access(cp ? local : ".", 2);
	if (cp)
		*cp = '/';
	if (d < 0) {
		perror(local);
		return((char *) 0);
	}
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	while (!d) {
		if (++count == 100) {
			printf(MSGSTR(RUNIQUE, "runique: can't find unique file name.\n")); /*MSG*/
			return((char *) 0);
		}
		*cp++ = ext;
		*cp = '\0';
		if (ext == '9')
			ext = '0';
		else
			ext++;
		if ((d = access(new, 0)) < 0)
			break;
		if (ext != '0')
			cp--;
		else if (*(cp - 2) == '.')
			*(cp - 1) = '1';
		else {
			*(cp - 2) = *(cp - 2) + 1;
			cp--;
		}
	}
	return(new);
}

static
abort_remote(din, sec, nic_abort)
FILE *din;
int sec;
int nic_abort;
{
	char buf[BUFSIZ];
	int nfnd;
	struct fd_set mask;

	/*
	 * send IAC in urgent mode instead of DM because 4.3BSD places oob mark
	 * after urgent byte rather than before as is protocol now
	 */
	sprintf(buf, "%c%c%c", IAC, IP, IAC);
	if (send(fileno(cout), buf, 3, MSG_OOB) != 3)
		perror("abort");
	fprintf(cout,"%cABOR\r\n", DM);
	(void) fflush(cout);
	FD_ZERO(&mask);
	FD_SET(fileno(cin), &mask);
	if (din) { 
		FD_SET(fileno(din), &mask);
	}
	if ((nfnd = empty(&mask, sec)) <= 0) {
		if (nfnd < 0) {
			perror("abort");
		}
		if (ptabflg)
			code = -1;
		lostpeer();
	}
	if (din && FD_ISSET(fileno(din), &mask)) {
		while (read(fileno(din), buf, BUFSIZ) > 0)
			/* LOOP */;
	}
	if (getreply(0) == ERROR && code == 552 && nic_abort) {
		/* 552 needed for nic style abort */
		(void) getreply(0);
	}
	if (code != 421)  /* don't read from peer if it isn't there */
		(void) getreply(0);
}
