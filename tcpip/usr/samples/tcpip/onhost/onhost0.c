static char sccsid[] = "@(#)08	1.9  src/tcpip/usr/samples/tcpip/onhost/onhost0.c, tcpip, tcpip411, GOLD410 3/8/94 19:18:49";
/*
 * COMPONENT_NAME: TCPIP onhost0.c
 *
 * FUNCTIONS: checkline, doem3278, dopath, getcmd, getem3278, getinp,
 *            getinput, Getline, getsocknm, immedinp, initialize, 
 *            intrsig, NextItem, openhostsocket, OpenLastAlias, openmsg,
 *            readhost, scanmsg, setecho, usage, writehost, main
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 * RISC System/6000 is a trademark of International Business Machines
   Corporation.
*/
/* Talk to hostconnect.
 */
       char version [] = "1.1";
/*
static char AIXwhatC[] = "onhost0.c	1.18 PASC 1.18";
 */
 
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
 
#ifdef	BSD42LIB
extern errno;
#include <strings.h>
#include <sgtty.h>
struct	sgttyb ttyb;
#include <sys/file.h>
#endif	BSD42LIB
 
#include <sys/socket.h>
#include <netinet/in.h>
 
#ifdef	SYSVLIB
#define u_short ushort
#define u_long ulong
#ifndef AIXV4
#define index strchr
#define rindex strrchr
#endif 
#include <string.h>
#include <sys/termio.h>
struct	termio ttyb;
#endif	SYSVLIB
 
/* sigvec is available on all IBM systems.  It may not be available
   on older System V where signal will be used instead */
 
#ifdef	SIGVEC  /* select the enhanced signal capability if possible */
struct	sigvec oldvec, newvec;
#endif	SIGVEC
 
#ifdef	LONGJMP  /* a long jump is needed to break a slow read */
int	sigjmpok;
jmp_buf	sigjmp;
#endif
 
#include "onhost0.h"
nl_catd catd;
 
#define	byte(x)	((x)&0377)
#define	SMALL 32
#define ESC	'\033'
int	argc;
char	*argp, **argv;
char *iam;
int	cmsretcode = 0;
int	cmsstate = 0;
int	dtype = CScmscmd;
int	hostsocket = -1;
char	sockfile[80];
FILE	*stddbg = NULL;
int	datatype;
int	debugvar = 0;
int	ndata = 0;
int	gotcmsretcode = 0;
int	gotint = 0;
int	gotsigint = 0;
FILE	*outstream;
char	sockp0[] = "noalias";
char	sockpath[SMALL];
int	newsockpath = 0;
int	xpertuser = 1;
int	fstest = 0;
int	fsmode = 0;
int prompt = 0;
int loud = 0;
int nonstop = 0;
int bury = 0;
int printcmsstatus = 0;
 
#define BFSIZE 4096
char	rdbuf[BFSIZE];
char	WRbuf[BFSIZE];
#define wrbuf (WRbuf+CShlen)
#define bfSIZE	(BFSIZE-CShlen)
 
char	aliasfile[] = "onhost.alias";
char	last_alias[] = "onhost.last";
FILE	*fds;
 
char	InetAddr[SMALL];
char	LPort[SMALL];
char	Auth[SMALL];
 
char	em3278[SMALL];

#ifdef	BSD42LIB
char	emdefault[] = "tn3270";
#else
char	emdefault[] = "tn";
#endif	BSD42LIB
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> checkline checks for the onhost end of command flag and, if    *
 *      found, extracts the host return code.  The result points to  *
 *      the beginning of the flag line.                              *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
char *
checkline(q,r)
char *q, *r;
{
#define distinct0 "RetCode:"
#define distinct1 "; OnHost End of Command Flag"
#define len0  8
#define len1 28
#define lenr  8
#define len  44
	int j, k, n;
	char *p, *pd;
 
	if (1 < debugvar) {
		fprintf(stddbg,"checkline: %d long==>",1+(r-q));
		(void) fwrite(q,1,1+(r-q),stddbg);
		fprintf(stddbg,"<==\n");
	}
 
	n = len;  /* tso hack - skip leading and trailing blank */
	if ( ( *q == ' ') && ( *( r - 1) == ' ')) {
		p = q + 1;
		n += 2;
		}
	else
		p = q;
 
	if (   n != ( r - q))
		return ( NULL);
 
	if ( ( *p == distinct0[0]) && ( strncmp( distinct0,p,len0) == 0)) {
		pd = p + len0;
		if ( strncmp( distinct1,pd+lenr,len1) == 0) {
			n = 0;
			for ( j = 0; j < lenr; pd++, j++) {
				if (*pd >= 'A')
					k = 10 + *pd - 'A';
				else k = *pd - '0';
				n = ( n << 4) | k;
			}
			cmsretcode = n;
			gotcmsretcode++;
			if (debugvar)
				fprintf(stddbg,"checkline: retcode %d is %d\n",gotcmsretcode,n);
			return ( q);
		}
	}
	return ( NULL);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> doem3278 tells hostconnect to expect a full-screen emulator,   *
 *      then onhost replaces itself with the emulator.               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
doem3278()
{
	int n;
 
	getem3278();  /* get the IBM-3278-2 full-screen emulator name */
	WRbuf[0] = CSem3278;  /* tell hostconnect full-screen emulator mode */
	WRbuf[1] = '\000';  /* negotiation style.  0 standard */
	if (fstest)
		WRbuf[1] = fstest - 1;
	n = writehost(2);
	if (debugvar)
		fprintf(stddbg,"doem3278: sends CSem3278 %x\n",WRbuf[1]);
 
	close(hostsocket);  /* close socket so emulator can connect */
	if (debugvar)
		fprintf(stddbg,"doem3278: starting %s for %s:%s\n",
			em3278,InetAddr,LPort);
 
	if (fstest) {
		fprintf(stdout,catgets(catd,ONHOST_ERR,OH_EMUL_WAIT,
			"hostconnect is waiting for a full-screen emulator\n to call host %s port %s as an IBM-3278-2 emulator\n"),
			InetAddr,LPort);
		exit(0);
	}
	if (debugvar)
		(void) fflush(stddbg);
	if ( strcmp( em3278,"tn.old") == 0)  /* AIX/RT pre 2.2.1 */
		n = execlp(em3278,em3278,"-p",LPort,InetAddr,0);
	else if ( strcmp( em3278,"tn.new") == 0)  /* AIX merged tn */
		n = execlp(em3278,em3278,"-e3270",InetAddr,LPort,0);
	else if ( strcmp( em3278,"tn") == 0)
#ifdef	TNOLD
		n = execlp(em3278,em3278,"-p",LPort,InetAddr,0);
#else
		n = execlp(em3278,em3278,"-e3270",InetAddr,LPort,0);
#endif
	else
		n = execlp(em3278,em3278,InetAddr,LPort,0);
 
	if (n<0) {
		fprintf(stderr,catgets(catd,ONHOST_ERR,OH_EXECLP_FAIL,
			"execlp failed for full-screen emulator "));
		perror(em3278);
		if ( 1 < debugvar)
			fprintf(stddbg,"doem3278 %s failed errno=%d\n",em3278,errno);
		return(-1);
	}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> dopath gets current working directory and default user for ftp *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
dopath(line)
char *line;
{
	char cwdenv[1024], *cwd;
	if ( (NULL != (cwd = getenv("LOGNAME"))) ||
		(NULL != (cwd = getenv("USER"))) ) {
		if (strlen(cwd))
			(void) strcpy(line,cwd);
		else
			(void) strcpy(line,"?");
		(void) strcat(line," ");
	}
	else
		(void) strcpy(line,"? ");
	if ( (NULL != (cwd = getenv("CWD"))) ||
#ifdef	SYSVLIB
		(NULL != (cwd = getcwd(cwdenv, 1024))) )
#else	!SYSVLIB
		(NULL != (cwd = getwd(cwdenv))) )
#endif	SYSVLIB
	{
		if ( (strlen(line) + strlen(cwd)) < 80 )
			(void) strcat(line,cwd);
		else  {
			(void) strcat(line,"??");  /* cwd too long */
		}
	} else {
		(void) strcat(line,"?");  /* cwd not found */
	}
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getcmd gets a command from argv and puts it in cmdbuf where it *
 *      will be processed and sent to the host.                      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
getcmd(cmdbuf,szcmdbuf)
char	*cmdbuf;
int	szcmdbuf;
{
	int nc;
	register char *p, *q, *qlast;
 
	/* return nc > 0: valid string and cmdbuf[nc] == \n */
	/*        nc= -1: end of file or read error */
	/*        nc= -2: cmdbuf too small to hold valid string */
	if (! *argv )
		return(-1);
	nc = 0;
	q = cmdbuf;
	qlast = q + szcmdbuf -2;
	p = argp;
	do {
		while ((*p) && (q <= qlast))
			*q++ = *p++;
		if (q>qlast) nc = -2;
		else {
			if (*argv) {
				p = *++argv;
				if (!p) {
					*q++ = '\n';
					nc = q-cmdbuf;
					*q++ = '\0';
				}
				else *q++ = ' ';
			}
			else nc--;
		}
	} while (!nc);
	argp = p;
	return(nc);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getem3278 sets the IBM-3278-2 full-screen emulator name        *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void getem3278()
{
	static char buf[80];
	char *b, *s, *s1, *s2;
 
	strcpy (em3278,emdefault);  /* set default emulator */
	(void) strcpy(sockfile,aliasfile);
	if ((fds=fopen(sockfile,"r"))==NULL) {  /* try to find default */
		if ( (s = getenv ("HOME")) != 0)
			(void) strcpy(sockfile,s);
		else
			(void) strcpy(sockfile,".");
		(void) strcat(sockfile,"/");
		(void) strcat(sockfile,aliasfile);
		if ((fds=fopen(sockfile,"r"))==NULL)
			if (debugvar) 
				fprintf(stddbg,"%s not found in current or home directory.\n",
					aliasfile);
	}
	if (fds != NULL) {
		GetLine (fds,buf);
		b = buf;
		/* 1st line of onhost.alias is [user [pass]] emulator */
		if (0 != (s1 = NextItem(&b)))
			if (0 != (s2 = NextItem(&b)))
				if (0 != (s = NextItem(&b)))
					strcpy (em3278,s);
				else	strcpy (em3278,s2);
			else	strcpy (em3278,s1);
		else {} /* use default emulator */
		fclose(fds);
	}
	if (debugvar) 
		fprintf(stddbg,"getem3278 sets emulator name to %s\n",em3278);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getinp reads a line from stdin.                                *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
getinp()
{
	int nc;
	register char *p;
 
	if ( gotint) {
		gotint = 0;
		return ( immedinp());
	}
	if ( fgets(wrbuf,bfSIZE,stdin) == NULL)
		wrbuf[0] = '\0';
	if ( (p=index(wrbuf,'\n')) != 0)
		nc = p+1-wrbuf;
	else {
		nc = -1;  /* e.g., end of file or control-D */
		printf("\n");
	}
	if (debugvar)
		fprintf(stddbg,"getinp: line read from stdin, %d long\n",nc);
	return(nc);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getinput reads a line in display or non-display mode           *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
getinput()
{
int n;
if (cmsstate & needNDread) {
	setecho(0); n = getinp(); setecho(1);
}
else
	n = getinp();
return(n);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> Getline is used to read the first line of onhost.profil        *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
GetLine (fds,line)
FILE *fds;
char *line;
{
	int n;
	char *p;
	int t;
 
	p = line;
	do {
		*p++ = t = fgetc (fds);
	} while ( (t!=EOF) && (t!='\n'));
 
	*--p = '\0';
 
	n = p - line;
 
	if (t==EOF)
		return(EOF);
	else
		return(n);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getsocknm makes name of file containing hostconnect socket     *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void getsocknm()
{
	char *p;
 
	if (sockpath[0] != '/') {
		if ( (p = getenv("HOME")) != 0)
			(void) strcpy(sockfile,p);
		else (void) strcpy(sockfile,".");
		(void) strcat(sockfile,"/hostcon.");
#ifdef	SHORT
		sockpath[6] = '\0';  /* limit file name (hostcon.xxxxxx) to 14 chars */
#endif	SHORT
		(void) strcat(sockfile,sockpath);
	}
	else (void) strcpy(sockfile,sockpath);
	if (debugvar)
		fprintf(stddbg,"getsocknm: %s contains hostconnect socket\n",sockfile);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> immedinp prompts for stdin line after user sent an interrupt   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
immedinp()
{
	int n;
 
	printf(catgets(catd,ONHOST_ERR,OH_FULL_SCREEN,
		" (Press enter for full-screen or enter a host line.)\n"));
 
	if ( (n = getinp() ) < 1 )
		return(n);
 
	if (n==1)  /* full-screen request */
		if (doem3278())
			exit(1);  /* problem */
 
	return( n);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> initialize scans the opts and args                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void initialize()
{
	register char *p;
 
	while ( (--argc > 0) && ((*++argv)[0] == '-') ) {
		p = *argv;
		while (p && *++p) {
			if (*p == '?')  /* query usage of onhost */
				usage(1);
			else if (*p == 'a') {  /* alias option */
				if (!--argc)  /* request alias */
					if (!OpenLastAlias("r")) {
						(void) fscanf(fds,"%s\n",sockpath);
						fclose(fds);
						fprintf(stderr,catgets(catd,ONHOST_ERR,
							OH_CURR_ALIAS,"current alias is %s\n"),sockpath);
						exit(0);
					}
					else  {
						fprintf(stderr,catgets(catd,ONHOST_ERR,
							OH_NO_ALIAS,"no current alias\n"));
						exit(0);
					}
				else  {  /* set new alias */
					(void) strcpy(sockpath,*++argv);
					++newsockpath;
				}
			}
			else if (*p == 'b')  /* bury all host output */
				bury++;
			else if (*p == 'c')  /* continue until end of file */
				nonstop++;
			else if (*p == 'd')  {  /* debug option */
				char c;
				debugvar = 1;
				c = *(p+1);
				if ( (c >= '0') && (c <= '9') ) {  /* optional debug digit */
					debugvar += c - '0';
					p++;
				}
			}
			else if (*p == 'f')  /* full-screen option */
				fsmode++;
			else if (*p == 'i')  /* ipl if cp read */
				xpertuser = 0;
			else if (*p == 'l')  /* loud display of all communication */
				loud++;
			else if (*p == 'n')  /* normal host line - no processing */
				dtype = CScmsinp;
			else if (*p == 'o')  /* one onhost command only */
				nonstop = 0;
			else if (*p == 'p')  /* prompt for input */
				prompt++;
			else if (*p == 'q')  /* quiet prompt option */
				prompt = 0;
			else if (*p == 'r')  /* repress return code display */
				printcmsstatus = 0;
			else if (*p == 's')  /* show return code option */
				printcmsstatus++;
			else if (*p == 't')  /* full-screen test option */
				fstest++;
			else if (*p == 'x')  /* expert cp user */
				xpertuser++;
			else {
				fprintf(stderr,catgets(catd,ONHOST_ERR,OH_BAD_OPTION,
					"unrecognized option: %c\n"),*p);
				usage(0);
			}
		}
	} /* end while doing options */
 
	if ( 1 == debugvar) { /* debug print on stderr */
		stddbg = stderr;
	}
	else if (debugvar) {
			if ( (stddbg = fopen("onhost.debug","w")) == NULL) {
				stddbg = stderr;
				fprintf(stderr,catgets(catd,ONHOST_ERR,OH_CANT_DEBUG,
					"can not open onhost.debug\n"));
			} else
				(void) chmod("onhost.debug",0600);
	}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> intrsig handles interrupt signals                              *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#ifdef	SIGVEC  /* select the enhanced signal capability if possible */

#ifdef  __SIGVOID  /* la1 AIX/370 twitch - see /usr/include/sys/signal.h */
__SIGVOID
#endif

intrsig(sig,code,scp)
int sig, code;
struct sigcontext *scp;
 
#else	!SIGVEC
intrsig(sig)
int sig;
#endif	SIGVEC
 
{
	/* got a SIGINT */
	gotsigint++;
 
#ifdef	LONGJMP  /* use long jump to break read - bsd 4.2 only */
	if (sigjmpok)  /* if longjmp is required to break read, do it */
		longjmp( (jmp_buf *) sigjmp, -1);
#endif	LONGJMP
 
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> NextItem is used to get tokens from a profile line             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
char *
NextItem (line)
char **line;
{
	char *p, *start;
	int something;
 
	something = 0;
	p = start = *line;
	for (;;) {
 
		if (*p=='\0')
			break;
		
		if ((*p==' ') || (*p=='\t'))
			if (something)
				break;
			else
				start++;
	
		something = 1;
		p++;
	}
 
	if ( *p != '\0') {
		*p = '\0';
		*line = ++p;
	} else
		*line = p;
 
	if (something)
		return(start);
	else
		return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> openhostsocket opens connection to hostconnect.  If successful *
 *      hostsocket is the fileno of connection, else -1              *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void openhostsocket()
{
	int	af = AF_INET;
	u_short	LocalPort;
	struct	sockaddr_in sname;
	FILE	*cmsfds;
	char *hs;
 
	if ( (cmsfds = fopen(sockfile,"r")) == 0) {
		if (strcmp(sockp0,sockpath))
			hs = "";
				else
			hs = " hostname";
		fprintf(stderr,catgets(catd,ONHOST_ERR,OH_NOT_CONN,
			"The host with alias %s is probably not connected.\n"),sockpath);
		fprintf(stderr,catgets(catd,ONHOST_ERR,OH_ENTER_CMD,
			"Enter hostconnect -a %s%s to make a connection\n or retry onhost with the alias of an active connection.\n"),
			sockpath,hs);
		fprintf(stderr,catgets(catd,ONHOST_ERR,OH_ACTIVE_ALIAS,
			"(ls hostcon.* in your home directory to find an active alias.)\n"));
		return;
	}
	(void) fscanf(cmsfds,"%s %s %s\n", InetAddr, LPort, Auth);
	(void) fclose(cmsfds);
 
	if ( (hostsocket = socket(af,SOCK_STREAM,0)) < 0) {
		perror("socket call failed");
		return;
	}
	LocalPort = (u_short) atoi(LPort);
	sname.sin_family = AF_INET;
	sname.sin_port = htons(LocalPort);
	sname.sin_addr.s_addr = inet_addr(InetAddr);
	if ( 1 < debugvar)
		fprintf(stddbg,"InetAddr =%s LocalPort =%d\n",InetAddr,LocalPort);
	if ( connect(hostsocket,(struct sockaddr*) &sname,sizeof(sname)) == 0 )
		openmsg();
	else {
		perror("connect call failed");
		fprintf(stderr,catgets(catd,ONHOST_ERR,OH_BACKGROUND,
			"\n Hostconnect should be running as a background process at %s.\n\t If it is not, remove hostcon.%s from your home directory and try again.\n\t Otherwise you can kill hostconnect (do not use kill -9) and try again.\n"),
			InetAddr,sockpath);
		(void) close(hostsocket);
		hostsocket = -1;
	}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> OpenLastAlias is used to open file containing alias last used  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int OpenLastAlias(open_type)
char *open_type;
{
	char *s;
 
	if ( (s=getenv("HOME")) != 0)
		(void) strcpy(sockfile,s);
	else
		(void) strcpy(sockfile,".");
 
	(void) strcat(sockfile,"/");
	(void) strcat(sockfile,last_alias);
 
	if ((fds=fopen(sockfile,open_type))==NULL)
		return(-1);
 
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> openmsg sends the initial message to hostconnect which         *
 *      includes the current working directory.  hostconnect expands *
 *      the onhost special commands so that onhost.profil needs to   *
 *      be read once per connection, not once per command.  But the  *
 *      cwd is needed so hostconnect can send it to FTP.             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void openmsg()
{
	int n, m;
 
	WRbuf[0] = CSopen;  /* open and authenticate connection */
	(void) strncpy(&WRbuf[1],Auth,6);
	WRbuf[7] = '\0';
	n = 8;
	(void) dopath(&WRbuf[n+4]);  /* store current working directory */
	m = 1 + strlen(&WRbuf[n+4]);
	WRbuf[n++] = CSdata;
	WRbuf[n++] = m / 256;
	WRbuf[n++] = m % 256;
	WRbuf[n++] = CScwd;
		n += m;
	n = writehost(n);
	if (n < 0)
		exit(-1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> readhost reads from hostconnect, scans the data, loops until   *
 *      scanmsg sets cmsstate                                        *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
readhost(i0)
int i0;
{
	int nr;
 
	while ( !cmsstate) {
	nr = 0;
	if ( i0)
		break;
			if (debugvar) 
				fprintf(stddbg,"readhost: waiting for line from host\n");
 
#ifdef	LONGJMP  /* use long jump to break read - bsd 4.2 only */
		if (setjmp(sigjmp) == 0) {
			sigjmpok = 1;
			if (debugvar) 
				fprintf(stddbg,"readhost: long jump set\n");
			nr = read(hostsocket,&rdbuf[i0],bfSIZE - i0);
		}
		else {
			nr = -1;  /* return -1 as would the other systems */
			if (debugvar) 
				fprintf(stddbg,"readhost: long jump taken\n");
		}
		sigjmpok = 0;
 
#else	!LONGJMP
		nr = read(hostsocket,&rdbuf[i0],bfSIZE - i0);
 
#endif	LONGJMP
 
			if (debugvar) 
				fprintf(stddbg,"readhost: line read from host, %d long\n",nr);
		if (gotsigint) {
			gotsigint = 0;
 
#ifndef	SIGVEC  /* enhanced signal not available, use signal */
			signal(SIGINT, intrsig);  /* tell system we still want signal */
 
#endif	SIGVEC
 
			(void) fputs("\n",stdout);
			gotint++;
			cmsstate |= needCMScmd;
			return( i0);
		}
		if (nr <= 0) {
			if (debugvar)
				fprintf(stddbg,"onhost lost connection nr=%d i0=%d errno=%d\n",
				nr,i0,errno);
			exit(3);
		}
		nr += i0;
		i0 = scanmsg(rdbuf,nr);
		if (debugvar)
			fprintf(stddbg,"readhost: state of host is %d\n",cmsstate);
	} /* end while (!cmsstate) */
	return ( i0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> scanmsg scans data from hostconnect for hostconnect messages.  *
 *      Output from the host is sent to stdout or buried.            *
 *      Unprocessed characters are shifted to front of buffer.       *
 *      Error messages from host go to stderr.                       *
 *      (Messages may not be complete this time through code.)       *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
scanmsg(rbuf,nr0)
char *rbuf;
int  nr0;
{
	char *p, *q, *r, *s, *q1;
	int nr, k;
 
	nr = nr0;
	p = rbuf;
	if (debugvar)
		fprintf(stddbg,"scanmsg: looking at data, %d long (ndata: %d)\n",
			nr0,ndata);
 
while ( nr) {
	if (ndata == 0)
 
		switch( byte(*p) ) {
		case CSstate:
			if (nr < 2) goto out;
			p++;
			cmsstate = *p++;
			nr -= 2;
			break;
 
		case CSdata:
			if (nr < 4) goto out;
			p++;
			ndata = byte(*p++);
			ndata = 256 * ndata + byte(*p++);
			datatype = byte(*p++);
			if ( datatype == CSerrmsg)
				fprintf(stderr,"onhost: ");
			nr -= 4;
			if ( 1 < debugvar)
				fprintf(stddbg,"scanmsg: ndata=%d type=%d\n",ndata,datatype);
			break;
 
		default:
			printf(catgets(catd,ONHOST_ERR,OH_OUT_PHASE,
				"scanmsg: out of phase nr=%d c=0x%x\n"),nr,*p);
			exit(-1);
	}  /* end if (!ndata) */
	if ( ndata) {
		if ( nr < ndata)
			k = nr;
		else k = ndata;
		if ( datatype == CSerrmsg) {
			(void) fwrite(p,1,k,stderr);
		}
		else {
			/* Scan data for the onhost end of command flag line and, if found,
			 * extract the host return code.  Write the characters up to the
			 * flag line, set print suppression if required, and write remaining
			 * complete lines.
			 *
			 * This code assumes that the flag line is not split across
			 * a CSdata block although the block may be split across TCP packets.
			 *
			 * p points to the beginning of the output characters
			 * q points to the beginning of a line; r points to the end
			 * q1 points to first flag line found
			 */
			r = q = p;
			s = k + r;
			q1 = NULL;
			while ( r < s) {
				while ( ( q < s) && ( *q == '\n'))
					q++;
				r = q;
				while ( ( r < s) && ( *r != '\n'))
					r++;
				if ( q1)
					(void) checkline(q,r);
				else
					q1 = checkline(q,r);
				q = r;
			}
			if ( nr < ndata) {
				s = q;
				k -= ( r - s);
			}
			if ( q1)
				q = q1;
			if ( ( outstream) && ( p != q))
				if ( !bury)
					(void) fwrite(p,1,(q-p),outstream);
			if ( ( gotcmsretcode) && ( !loud))
				outstream = NULL;
			if ( ( outstream) && ( q != s))
				if ( !bury)
					(void) fwrite(p,1,(s-q),outstream);
		}
		nr -= k;
		ndata -= k;
		p += k;
		if ( ndata)
			goto out;
	}
}
 
	/* move remaining nr characters to front of buffer */
out:
	for (k=0; k<nr; k++)
		rbuf[k++] = *p++;
	return(k);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> setecho turns local echo on and off to protect password entry  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void setecho(echoon)
int echoon;
{
 
#ifdef	BSD42LIB
	ioctl(0,TIOCGETP,&ttyb);
	if (echoon)
		ttyb.sg_flags |= ECHO;
	else ttyb.sg_flags &= ~ECHO;
	ioctl(0,TIOCSETP,&ttyb);
 
#else	SYSVLIB
	ioctl(0,TCGETA,&ttyb);
	if (echoon)
		ttyb.c_lflag |= ECHO;
	else ttyb.c_lflag &= ~ECHO;
	ioctl(0,TCSETA,&ttyb);
#endif	SYSVLIB
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> usage prints a bit of information about the onhost options     *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void usage(zip)
int zip;
{
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP1,
		"(%s) usage: "),version);
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP2,
		"%s [-?abcd[n]lnpqst] [alias] [command]\n"),iam);
	if (zip == 0)
		exit(1);
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP3,
		"where \ta \t= use alias if present or display current alias\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP4,
		"\tb \t= bury all host output\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP5,
		"\tc \t= continue after one onhost command until input end of file\n\t or interrupt (see note 1) ends onhost\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP6,
		"\td, d0 \t= debug output to stddbg\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP7,
		"\td1 \t= debug output to onhost.debug\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP8,
		"\tl \t= loud display of all host output\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP9,
		"\tn \t= normal line for host, no special treatment (see note 2)\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP10,
		"\tp \t= prompt for host input - prompt will show\n\t\t>> for onhost command, > for normal line\n\t\t if no prompt appears, a line can be forced (see note 1)\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP11,
		"\tq \t= quiet host input prompt\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP12,
		"\ts \t= show host return code\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP13,
		"\tt \t= test mode for full-screen emulator\n"));
	getem3278();
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP14,
		"will send command to the host identified by alias or to the\n\tlast used host.  If no command is present, then a full-screen\n\tsession is started using the IBM-3278-2 emulator named %s\n"),em3278);
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP15,
		"note 1: an interrupt (ctrl-c) will terminate onhost if onhost is waiting\n\tfor your input, otherwise onhost will force a line to the host\n"));
	fprintf(stderr,catgets(catd,ONHOST_ERR,OH_HELP16,
		"note 2: a normal line of hat(^) followed by a character produces a\n\tpf1..pf12(1234567890-=), pa1..pa2(ab), enter(e), or clear(z) at the host\n"));
	exit(1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> writehost writes a buffer load to hostconnect                  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
writehost(n)
int n;
{
	int m;
	
	if ( ( m = write(hostsocket,WRbuf,n)) <= 0 ) {
		if ( 1 < debugvar)
			fprintf(stddbg,"writehost: m=%d, errno= %d\n",m,errno);
		return(-1);
	}
	return(m);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> main opens a connection to hostconnect, runs the connection    *
 *      between host and user, and then exits with a return code.    *
 *                                                                   *
 *    Characters are sent to hostconnect in response to status data  *
 *    about the host.  If the command flag is on, then hostconnect   *
 *    will process the next group of characters using onhost.profil  *
 *    to generate data for the host.  Otherwise, the next group of   *
 *    characters will be passed straight to the host.                *
 *                                                                   *
 *    When the host status shows that input is required, onhost will *
 *    try to get some characters to send.  Otherwise, onhost will    *
 *    listen to hostconnect for any host output or status.           *
 *                                                                   *
 *    Either read is broken by a user interrupt.  Termination of the *
 *    local read terminates onhost.  Termination of the hostconnect  *
 *    read produces a prompt for immediate input.                    *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
main(argc0,argv0)
int argc0;
char *argv0[];
{
int	i0 = 0;
	int n, triedpwd;
        /* set the locale to the default */
        setlocale(LC_ALL, "");

        /* open the message catalog */
        catd = catopen("onhost.cat",NL_CAT_LOCALE);
	gotcmsretcode = triedpwd = 0;
	argv = argv0;
	iam = argv0[0];
	argc = argc0;
	initialize();
	if (debugvar) 
		fprintf(stddbg,"%s (onhost version %s) completed initialize()\n",
			iam,version);
	if (!newsockpath)  /* get the current alias */
		if (!OpenLastAlias("r")) {
			(void) fscanf(fds,"%s\n",sockpath);
			fclose(fds);
		}
		else
			(void) strcpy (sockpath,sockp0);  /* no current alias, use default */
	getsocknm();
	hostsocket = -1;
	openhostsocket();  /* check this alias */
	if (hostsocket < 0) {  /* no good */
		if (debugvar) 
			fprintf(stddbg,"main: can't connect to hostconnect\n");
		exit(2);
	}
 
	if (debugvar) 
		fprintf(stddbg,"main: connected to hostconnect\n");
	if (newsockpath)
		if (!OpenLastAlias("w"))  {  /* save alias */
			(void) fprintf(fds,"%s\n",sockpath);
			fclose(fds);
		}
		else  /* can't save current alias - no big deal (system told user) */
			if (debugvar) 
				fprintf(stddbg,"main: current alias not saved\n");
 
	if ( fsmode || ( argc == 0))  /* no command means full-screen */
		if (doem3278())
			exit(1);  /* problem */
 
	argp = *argv;
	outstream = stdout;
	gotsigint = 0;
 
#ifdef	SIGVEC  /* select the enhanced signal capability if possible */
	newvec.sv_onstack = 0;  /* signal.h may define sv_onstack to sv_flags */
#endif	SIGVEC

#ifdef	LONGJMP
	sigjmpok = 0;
 
#else	!LONGJMP
#ifdef	BSD42LIB  /*  BSD system */
	newvec.sv_flags |= SV_INTERRUPT;  /* tell BSD 4.3 to break slow read */
 
#endif	BSD42LIB
#endif	LONGJMP
 
#ifdef	SIGVEC  /* select the enhanced signal capability if possible */
	newvec.sv_mask = 0;
	newvec.sv_handler = intrsig;
	sigvec(SIGINT, &newvec, &oldvec);
 
#else	!SIGVEC  /* otherwise use signal */
	signal(SIGINT, intrsig);
 
#endif	SIGVEC
 
	WRbuf[0] = CSgetstate;
	n = writehost(1);
	if (debugvar)
		fprintf(stddbg,"main: requesting host state from hostconnect\n");
 
/* This is the main loop of onhost */
 
	while ( 0 <= n) {
		cmsstate = n = 0;
		i0 = readhost(i0);  /* read hostconnect until cmsstate <> 0 */
 
		if (printcmsstatus)
			if (gotcmsretcode)
				printf("host rc=%d\n",cmsretcode);
 
		if (cmsstate & FullScreen )
			cmsstate &= ~(needCPcmd);
 
		if (cmsstate & lostconnection )
			break;
 
		if ( gotcmsretcode && nonstop) {
			gotcmsretcode = 0;
			dtype = CScmscmd;
		}
 
		if ( cmsstate & ( needCMScmd | needCMSinp)) {
			if ( gotcmsretcode)
				break;
			outstream = stdout;
			triedpwd = 0;
			}
 
		if ( cmsstate & needCPcmd) {
			if (triedpwd) {
				if (xpertuser) {  /* give expert more than one CP read */
					if ( loud || prompt)
						printf("CP>");
					n = getinput();
				} else {
					printf(catgets(catd,ONHOST_ERR,OH_CP_READ,
					"CP read without expert option - onhost requesting ipl\n"));
					WRbuf[0] = CSipl;
					if ( 0 < (n = writehost(1)) )
						n = 0;
					outstream = stdout;  /* NULL to bury ipl output, stdout to show */
				}
			} else {  /* treat first CP read as a password request */
			/* triedpwd++; suspend for the nonce so *all* CP reads are okay */
				if ( loud || prompt)
					printf("CP>");
				n = getinput();
				if (n == 1)
					n = strlen(strcpy(wrbuf,"b"));  /* feed begin (also dummy pwd) */
			}
		}
 
		if (cmsstate & (needCMScmd | needCMSinp)) {
			if ( loud || prompt) {
				printf(">");
				if (dtype == CScmscmd)
					printf(">");
			}
			if ( (n = getcmd(wrbuf,bfSIZE) ) < 0)  /* use command line first */
					n = getinput();
			else
				if ( loud || prompt)
					printf(wrbuf);
			gotint = 0;
		}
 
		if (debugvar)
			fprintf(stddbg,"main: line for host, %d long\n",n);
 
		if (0 < n) {
			wrbuf[n] = '\0';
			if (debugvar) {
				fprintf(stddbg,"main: line sent to host, %d long: %s\n",
					n,wrbuf);
			}
			if (loud && !(cmsstate & needNDread))
				printf(wrbuf);
 
			WRbuf[0] = CSdata;  /* these four bytes prefix wrbuf */
			WRbuf[1] = n / 256;
			WRbuf[2] = n % 256;
			WRbuf[3] = dtype;
			n += CShlen;
			n = writehost(n);
			dtype = CScmsinp;
		}
 
	}  /* end while */
	if (debugvar) {
		fprintf(stddbg,"main: exit with n= %d and cmsretcode= %d\n",
			n,cmsretcode);
		if (4 < n)
			fprintf(stddbg,"main: string (less 4 byte header) = %s\n",
				wrbuf[0]);
	}
#ifdef  AIXV3
        /* close the message catalog */
        catclose(catd);
#endif  AIXV3
	if (gotcmsretcode)
		return(cmsretcode);
	else return(-1);
}
