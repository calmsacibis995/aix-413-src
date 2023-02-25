static char sccsid[] = "@(#)99	1.10  src/tcpip/usr/samples/tcpip/onhost/hostcon0.c, tcpip, tcpip411, GOLD410 3/8/94 19:17:54";
/*
 * COMPONENT_NAME: TCPIP hostcon0.c
 *
 * FUNCTIONS: intrsig, pass3270, anyio, closeit, datafromhost,
 *            dbgprint, filterhostcmd, readonhost, running, bailout,
 *            scanmsg, gettn, puttn, selecttn, init_tn3270, selectio,
 *            setecho, useduser, writetohost, writetouser, main
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
/* Set up a connection to an IBM host.
 */
       char version [] = "1.1";
/*
static char AIXwhatC[] = "hostcon0.c	1.20 PASC 1.20";
 */
/*
 * hostconnect nnn where nnn is the name of the remote machine
 * will connect to the remote machine and send messages until
 * it sees the login prompt.  It asks user for a userid and password,
 * sends the login and password messages, sets up the host, and if all
 * goes well, it will then put itself into the background and
 * wait for onhost action.
 */
 
 
#include <sys/types.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
 
#ifdef	BSD42LIB
#include <strings.h>
#include <sgtty.h>
struct	sgttyb ttyb;
#include <sys/file.h>
#include <sys/time.h>
#define memcpy(x,y,n) bcopy(y,x,n)
#endif	BSD42LIB
 
#ifdef	SYSVLIB
#ifndef AIXV4
#define index strchr
#define rindex strrchr
#endif 
#include <string.h>
#include <sys/termio.h>
struct	termio ttyb;
#include <fcntl.h>
#ifdef	AIXV3
#include <sys/time.h>
#else
#include <time.h>
#endif	AIXV3
#include <sys/select.h>
#endif	SYSVLIB
 
/*	has BSD sigvec operations */
#ifdef	SIGVEC
struct	sigvec oldvec, newvec;
#endif	SIGVEC
 
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
struct	sockaddr_in sin;
 
int ibits ,obits;
int rdnil; /* i/o deadman */
 
struct timeval time5 = {5,  0 };
struct timeval time2 = {2,  0 };
 
/* DOINIT causes values to initialized in hostcon0.h */
#define DOINIT
/* EXTERN is initialized to null in hostcon0.c, extern in hostcon*.c */
#define EXTERN
#include "hostcon0.h"
#include "onhost0.h"
 
nl_catd catd;
#define touserbuf (TOuserbuf+CShlen)
 
int	cmscc;
int	cmsstate;
int	cmsretcode;
int	spclcmd = 0;
extern int echoed;
extern char Auth[];
extern int authen;
extern char alias[];
int	cmsreadok, cmswriteok, localreadok, localwriteok;
jmp_buf ExitOne;
hosttype	hostsys = hostunk;
int	kbd = 0;
int	pidmain;
int tsomore = 0;
int NDflag = 0;
int	spmsgn;
char	spmsg[512];
int	timedout;
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
=====>intrsig called by system when certain signals are received     *
 *      see sigvec calls in main()                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#ifdef	SIGVEC

#ifdef  __SIGVOID  /* la1 AIX/370 twitch - see /usr/include/sys/signal.h */
__SIGVOID
#endif

intrsig(sig,code,scp)
int sig, code;
struct sigcontext *scp;
#else	~SIGVEC
intrsig(sig)
int sig;
#endif	~SIGVEC
{
	if ( sig == SIGALRM ) {
		return;
	}
	if ( ( sig == SIGINT ) || ( sig == SIGTERM ) )
		longjmp( (jmp_buf *) ExitOne, -1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> pass3270 passes data between the sockets.  The host data in    *
 *      fromcmsbuf goes to the application (em3278, for example) and *
 *      application data in fromuserbuf goes to the host.            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
pass3270()
{
	int n;
 
	if (cmsreadok && !ntouser) {
		ntouser = read(remotesock, fromcmsbuf,sizeof(fromcmsbuf));
		debug2((stddbg,"tn3270 CMSread  ntouser=%d errno=%d\n",ntouser,errno));
		if (debugvar>3) dbgprcms(fromcmsbuf,ntouser,"remoteread datastream %d bytes:\n");
		if (ntouser==0)
			if ( 3 < rdnil) {
				UserStage = LostHost;
				if (!automaticlogin)
					quitanyio++;
				return(-1);
			}
			else rdnil++;
		else rdnil = 0;
		if (ntouser < 0) {
			if (errno==ENODEV) {  /* connection closed */
				UserStage = LostHost;
				return(-1);
			}
			ntouser = 0;
		}
		nnltouser = ntouser;
	}
 
	if (ntouser && localwriteok) {
		n = write(localwrite,fromcmsbuf,ntouser);
		debug2((stddbg,"tn3270 localwrite n=%d errno=%d\n",n,errno));
		if (debugvar>3) dbgprcms(fromcmsbuf,n,"localwrite datastream %d bytes:\n");
		ntouser -= n;
		nnltouser = ntouser;
	}
 
	if (localreadok && !ntocms) {
		ntocms = read(localread,fromuserbuf,sizeof(fromcmsbuf));
		debug2((stddbg,"tn3270 localread ntocms=%d errno=%d\n",ntocms,errno));
		if (debugvar>3) dbgprcms(fromuserbuf,ntocms,"localread datastream %d bytes:\n");
		if (ntocms <= 0)
			return(-1);
#ifdef LDSF
		if (isldsf) {  /* strip IAC EOR */
			n = ntocms - 1;
			if (byte(fromuserbuf[n])==EOR)
				n--;
			if (byte(fromuserbuf[n])==IAC)
				n--;
			if (n == (ntocms - 3))
				ntocms -= 2;
		}	
#endif LDSF
	}
 
	if (ntocms && cmswriteok) {
		n = write(remotesock,fromuserbuf,ntocms);
		ntocms -= n;
		debug2((stddbg,"tn3270 CMSwrite n=%d errno=%d\n",n,errno));
		if (debugvar>3) dbgprcms(fromuserbuf,n,"remotewrite datastream %d bytes:\n");
	}
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> anyio contains the select loop for read or write to host and   *
 *      read or write to onhost - exits if quitarg.  anyio is called *
 *      from main for login and from running (in main).  quitarg is  *
 *      set if connection lost to host, connection lost to onhost,   *
 *      login completes.                                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
anyio()
{
	int usercc = 0;
	int	settimeout;
	struct timeval *timeptr;
 
	timeptr = (struct timeval *) 0;
	debug2((stddbg,"anyio: read=%d write=%d remote=%d \n",
		localread,localwrite,remotesock));
	timedout = settimeout = quitanyio = 0;
 
	while (1) {
		timedout = selectio(timeptr);
		if (tn3270 == 2) {
			if ( pass3270())
				return;
		} else {
			if (cmsreadok)
				if ( datafromhost())  /* reads from host, increment cmscc */
					break;
			/* filter fromcmsbuf into touserbuf
			 * increment ntouser, possibly print some data
			 * note touserbuf will not be sent to user until
			 * next iteration of anyio
			 */
			if ( 0 < cmscc) {
				cmscc = cmstouserbuf(cmscc);
				debug2((stddbg,"anyio: cmstouserbuf processed\n"));
			}
			if (localwriteok)
				writetouser();
			if (localreadok) {
				if (Running)
					if ( (usercc = readonhost()) < 0)
						return;  /* local read broke - exit from anyio */
			}
			if (iplrequested)  {
				iplrequested = 0;
				(void) strcpy(fromuserbuf,iplcmd);
				usercc = strlen(fromuserbuf);
				fromuserbuf[usercc++] = '\n';
			cmsstate &=  ~(needNDread | needCMSinp | needCMScmd | needCPcmd);
			telluser = 0;
			}
			if (automaticlogin)  {
				if (ntocms == 0 )
					usercc = StartupCommands(fromuserbuf,timedout, &settimeout);
				timedout = 0;
				if (settimeout)
					timeptr = &time2;
				else
					timeptr = (struct timeval *) 0;
			}
			if (quitanyio)
				return;
			if ( usercc)  /* fromuserbuf --> fromuserbuf */
					usercc = filterhostcmd(usercc);
			if (0 < usercc)  /*  fromuserbuf --> tocmsbuf */
				usercc = usertocmsbuf(usercc);
			if ( cmswriteok && ntocms ) {
				writetohost();  /* tocmsbuf --> host */
			}
		}
	}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> closeit close all sockets and files                            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
closeit()
{
	if (remotesock>0) {
#ifdef	LDSF
		if (isldsf == 0)
#endif	LDSF
		(void) shutdown(remotesock,2);
		(void) close(remotesock);
		remotesock = -1;
	}
	(void) docmssock(oncmsUnmake);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> datafromhost called by anyio to read data from host            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
datafromhost()
{
	int n;
 
	/* read data from host into fromcmsbuf */
 
	errno = 0;
	n = read(remotesock, &fromcmsbuf[cmscc],sizeof(fromcmsbuf)-(cmscc+1));
	if (n <= 0 ) {
		debug2((stddbg,"hostcon0:datafromhost: read returns %d (errno %d)\n",
			n,errno));
		if (errno == EWOULDBLOCK)
			n = 0;
		else if (errno==ENODEV) {  /* connection closed */
			debug2((stddbg,"datafromhost: connection closed\n"));
			UserStage = LostHost;
			if (automaticlogin)
				return(0);
			return(1);
		}
		else {
			if (tn3270 && ConnectOnly) {
				debug2((stddbg,"No datafromhost connectonly errno=%d\n",errno));
				return(0);
			}
			debug((stddbg,"datafromhost got n=%d errno=%d \n",n,errno));
			UserStage = LostHost;
			if (automaticlogin)
				return(0);
			return(1);
		}
	}
	if (n  && (debugvar > 2) )
		if (debugvar>3) dbgprcms(&fromcmsbuf[cmscc],n,"<===From CMS n=%d\n");
	
	if ( n > sizeof(fromcmsbuf) ) {
		printf(catgets(catd,HOSTCON0_ERR,HC_BAD_VALUE,
			"weird value on read fd=%d n=%d max=%d\n"),
			remotesock,n,sizeof(fromcmsbuf));
		n = 0;
	}
	cmscc += n;
	debug2((stddbg,"datafromhost n=%d cmscc=%d errno=%d \n",n, cmscc, errno));
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> dbgprint writes hex characters to stddbg                       *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
dbgprint(a,n,msg)
char a[], *msg;
int n;
{
	/* write a[0..n-1] to stddbg in hex */
	int j, k;
 
	debug((stddbg,"dbgprint: %d %s\n",n,msg));
	for(k=1, j = 0; (j<n)&&(j<16); j++, k++) {  /* limit array print */
		fprintf(stddbg,"%x ",byte(a[j]));
		if ( (k % 60) == 0)
			putc('\n',stddbg);
	}
	putc('\n',stddbg);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> filterhostcmd looks for commands arriving from onhost          *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
filterhostcmd(n0)
int n0;
{
	/* got command for VM but see if it is special
	 * return n1 = number of characters in result
	 */
	register int n1;
 
	n1 = n0;
	fromuserbuf[n1] = '\0';
	n1 = srch(fromuserbuf,spclcmd);
 
	if (n1) {
		spclcmd = 0;
		cmsstate &=  ~(needNDread | needCMSinp | needCMScmd | needCPcmd);
		telluser = 0;
		if ((fromuserbuf[0] == AsciiEsc) || (fromuserbuf[0] == '^')) {
			if (dokeys(fromuserbuf) == 0)  {
			/* immed pf produces host state change, otherwise input area change */
				n1 = 0;
				echoed = 0;
			}
		}
	}
	else /* returns a message in spmsg */
		GotNucxRetCode =  1;
	return(n1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> readonhost data from onhost and put it in fromuserbuf.         *
 *      localread is a socket to onhost.  if return is < 0, then     *
 *      anyio will return to its caller.                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
readonhost()
{
	int n;
	int m;
 
	errno = 0;
	for(n=0; n >=0;) {
		m = read(localread,fromuserbuf+n,sizeof(fromuserbuf)-n);
		if (m < 0 && errno == EWOULDBLOCK)
			m = 0;
		else if (m <= 0)
			n = -1;
		if ( m > 0) {
			n += m;
			if ( scanmsg(&n) )  /* will have set n if non-zero return */
				break;
		}
	}
	debug2((stddbg,"readonhost: errno=%d, m=%d, n=%d\n",errno,m,n));
	return(n);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> running is called by main when login is complete.  here is the *
 *      main loop to create local socket, wait for onhost to open    *
 *      the socket, then call anyio and on return close socket and   *
 *      loop while userstage is GotNucx.                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
running()
{
	int	rc;
 
	/* now we are running - ignore all signals except SIGTERM */
#ifdef	SIGVEC
	newvec.sv_handler = SIG_IGN;
	for (rc=1; rc <= 32; rc++) {
		/* sigvec will ignore some values of rc such as 9 */
		if (rc != SIGTERM)
			sigvec(rc, &newvec, &oldvec);
	}
#else	~SIGVEC
	for (rc=1; rc <= 32; rc++) {
		/* signal will ignore some values of rc such as 9 */
		if (rc != SIGTERM)
			(void) signal(rc, SIG_IGN);
	}
#endif	SIGVEC
 
	GotNucxRetCode = 0;
	pendofmsg = NULL;
	cmsstate = 0;
	Running = 1;
 
	/* If localread or localwrite breaks then onhost has terminated
	 * but we can close then open to wait for a next onhost interaction
	 * If hostread or hostwrite breaks then the host connection has been
	 * lost so we close everything and exit.
	 */
	while (UserStage == GotNucx) {
		errno = 0;
		/* if 4.2, oncmsOpen will wait until onhost does an open */
		if (docmssock(oncmsOpen) )
			break;
		if ( tn3270 == 1) {
			tn3270++;
			if (init_tn3270()) {
				fprintf(stderr,catgets(catd,HOSTCON0_ERR,HC_EMUL_FAIL,
					"Expected full-screen emulator negotiation has failed.\n"));
				(void) docmssock(oncmsClose);
				tn3270 = 0;
				continue;
			}
		}
		anyio();
 
		(void) docmssock(oncmsClose);
		cmscc = ntocms = ntouser = nnltouser = 0;
		if ( tn3270 == 2) {
			tn3270 = 0;
			havecursor = kbd = 0;
			ScreenStatus = UndefinedScreen;
			putiacbuf(ClearKey,IAC,EOR);
			debug2((stddbg,"Send ClearKey after em3278\n"));
			writetohost();
		}
	} /* end while */
 
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> bailout of host connection because we have not authenticated   *
 *      the onhost connection.                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
bailout()
{
	printf(catgets(catd,HOSTCON0_ERR,HC_UN_AUTH,
		"\nYour host connection to the alias named %s has been closed\nbecause of an unauthenticated attempt to use it.\n"),alias);
	closeit();
	exit(1);
	return(1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> scanmsg to pickup any messages between onhost and hostconnect  *
 *      from the data flowing between the user and the host.  Once   *
 *      one host data message is processed the rest is trashed!      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
scanmsg(pn)
int *pn;
{
	/* Look for msgs from onhost, when first data msg is found, move
	 * data to front of buffer, zap number of characters, trash remainder.
	 * Msg may not be complete within one packet so return 0 and expect
	 * the caller to come back after (append) reading more data.
	 */
	register char *p, *q;
	register int nr, ndata, datatype ;
 
	nr = *pn;
	*pn = 0;
	p = fromuserbuf;
	ndata = datatype = 0;
	if (debugvar > 2)
		dbgprint(p,nr,"scammsg (hostcon0):");
again:
/* debug2((stddbg,"scanmsg: p=%d, nr=%d, byte(*p)=%d\n",p,nr,byte(*p))); */
	if ((!authen) && (byte(*p) != CSopen))
		bailout();
	switch( byte(*p) ) {
	case CSclose:
		debug2((stddbg,"scanmsg: CSclose message\n"));
		*pn = -1;
		return(1);
 
	case CSopen:
		if (nr < 8)
			return(0);
		p++;
		debug((stddbg,"authentication received %s, expected %s.\n",p,Auth));
		if ( 0 != strcmp( p, Auth) )
			bailout();
		authen++;
		p += 7;
		nr -= 8;
		debug2((stddbg,"scanmsg: CSopen message\n"));
		telluser = 0;
		if (nr) goto again;
		break;
 
	case CSipl:
		p++;
		nr--;
		iplrequested++;
		debug2((stddbg,"scanmsg: CSipl message\n"));
		if (nr) goto again;
		break;
 
	case CSgetstate:
		p++;
		nr--;
		debug2((stddbg,"scanmsg: CSgetstate message\n"));
		telluser = 1;
		if (nr) goto again;
		break;
 
	case CSem3278:
		p++;
		nr--;
		debug2((stddbg,"scanmsg: CSem3270 message\n"));
		tn3270 = 1;	/* next connection is 3270 full-screen emulator */
		tn3270n = byte(*p++);  /* 0 standard binary eor 3278 negotiation */
		if (--nr) goto again;
		break;
 
	case CSdata:
		if (nr < 4)
			return(0);
		p++;
		ndata = byte(*p++);
		ndata = 256 * ndata + byte(*p++);
		datatype = byte(*p++);
		nr -= 4;
		debug2((stddbg,"scanmsg: CSdata message\n"));
		break;
 
	default:
		debug2((stddbg,"scanmsg: switch default\n"));
		printf(catgets(catd,HOSTCON0_ERR,HC_UNEXPECTED,
			"hostconnect received unexpected data.\n"));
 		*pn = -1;
		return(1);
	}
	if (ndata) {
		if (ndata > nr)
			return(0);
 
		switch(datatype) {
		case CScwd:
			(void) strcpy(wwdir," ");
			if (0 != strlen(p))
				(void) strcat(wwdir,p);
			else
				(void) strcat(wwdir,"? ?");
			nr -= ndata;
			p += ndata;
			ndata = 0;
			if (nr) goto again;
			break;
 
		case CScmscmd:
		spclcmd = 1;
		default:  /* normal input or host command */
			*pn = ndata;
			/* move everything to front of buffer
			 * nr should be equal to ndata
			 */
			if (nr != ndata)
				debug((stddbg,"scanmsg: possible problem, nr=%d != ndata=%d bytes\n",
					nr,ndata));
			q = fromuserbuf;
			for (nr = 0; nr < ndata; nr++)
				*q++ = *p++;
			debug2((stddbg,"hostcon0:scanmsg: gets %d bytes, state=%d\n",ndata,cmsstate));
			ndata = 0;
		}
	}
	return(1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> gettn reads from the full-screen terminal emulator             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
gettn (item)
int item;
{
	int ctr, i, limit, n;
	struct timeval *timeptr;
 
	timeptr = (struct timeval *) 0;
	ctr = 0;
	for (limit=0;limit<10;limit++) {
		(void) selecttn(timeptr);
		if (!localreadok)
			continue;
		n = read(localread,&fromuserbuf[ctr],sizeof(fromuserbuf)-ctr);
		debug2((stddbg,"gettn read n=%d,errno=%d\n",n,errno));
		if (n<0) {
			debug2((stddbg,"gettn errno=%d\n",errno));
			n = 0;
		}
		for (i=ctr;i<ctr+n;i++) {
			debug2((stddbg,"%d=%x,",i,fromuserbuf[i]));
			if (byte(fromuserbuf[i])==item)
				return(i+1);
		}
		ctr += n;
	}
	debug2((stddbg,"gettn limit\n"));
	return(-1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> puttn writes to the full-screen terminal emulator              *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
puttn ()
{
	int n, limit;
	struct timeval *timeptr;
 
	timeptr = (struct timeval *) 0;
	for (limit=0;limit<10;limit++) {
		(void) selecttn(timeptr);
		if (!localwriteok)
			continue;
		n = write(localwrite,TOuserbuf,ntouser);
		ntouser -= n;
		return(n);
	}
	debug2((stddbg,"puttn limit\n"));
	return(-1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> selecttn waits for full-screen terminal emulator read or write *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
selecttn(timeptr)
struct timeval *timeptr;
{
int  selret;
 
	if (ntouser) {
		ibits = 0;
		obits = (1 << localwrite);
	} else {
		ibits = (1 << localread);
		obits = 0;
	}
again:
	errno = 0;
 
	/* debug2((stddbg,"selecttn0 user=%d,%d tell=%d timep=%x\n",
		ibits, obits, telluser, timeptr)); */
	selret = select(16, &ibits, &obits, (int *) 0,timeptr);
	localreadok = ibits & (1 << localread);
	localwriteok = obits & (1 << localwrite);
	if (errno == EINTR)
		goto again;
	return(!selret); /* return 1 if timer expired */
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> init_tn3270 negotiates binary telnet with the full-screen      *
 *      terminal emulator after onhost has given it our socket       *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int	
init_tn3270()
{
	int n, ni;
#define	TTLEN 11
	char lasttt[TTLEN], wanttt[TTLEN];
	lasttt[0] = IAC;
	(void) strcpy(wanttt,"IBM-3278-2");  wanttt[TTLEN-1] = IAC;
	debug2((stddbg,"init_tn3270\n"));
 
	ntouser = 0;
	putiactn(IAC,DO,Terminal_type);
	n = puttn();
	debug2((stddbg,"write (iac) do termtype = %d\n",n));
	n = gettn(Terminal_type);
	if (n<=0) return(-1);
	ni = 0;
	if (!(byte(fromuserbuf[ni++])==IAC)) return(-1);	
	if (!(byte(fromuserbuf[ni++])==WILL)) return(-1);	
	if (!(byte(fromuserbuf[ni++])==Terminal_type)) return(-1);	
	debug2((stddbg,"read (iac) will termtype = %d\n",n));
 
/* repeat until terminal type is IBM-3278-2 (pass) or repeats (fail) */
/*   this code accepts any repeated terminal type */
	do {
		putiactn(IAC,SB,Terminal_type);
		putiactn(1,IAC,SE);
		n = puttn();
		debug2((stddbg,"write sb termtype 1 iac se = %d\n",n));
		n = gettn(SE);
		if (n<=0) return(-1);
		ni = 0;
		if (!(byte(fromuserbuf[ni++])==IAC)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==SB)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==Terminal_type)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==0)) return(-1);	
		debug2((stddbg,"read sb termtype  .. 16 se = %d\n",n));
	} while ( strncmp(wanttt,&fromuserbuf[ni],TTLEN) &&
		  strncmp(lasttt,&fromuserbuf[ni],TTLEN) &&
		  strlen(strncpy(lasttt,&fromuserbuf[ni],TTLEN)) );

	if ( tn3270n == 0) {
		putiactn(IAC,DO,End_of_record);
		n = puttn();
		debug2((stddbg,"write do eor = %d\n",n));
		n = gettn(End_of_record);
		if (n<=0) return(-1);
		ni = 0;
		if (!(byte(fromuserbuf[ni++])==IAC)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==WILL)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==End_of_record)) return(-1);	
		debug2((stddbg,"read will eor = %d\n",n));
 
		putiactn(IAC,WILL,End_of_record);
		n = puttn();
		debug2((stddbg,"write will eor = %d\n",n));
		n = gettn(End_of_record);
		if (n<=0) return(-1);
		ni = 0;
		if (!(byte(fromuserbuf[ni++])==IAC)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==DO)) return(-1);	
		if (!(byte(fromuserbuf[ni++])==End_of_record)) return(-1);	
		debug2((stddbg,"read do eor = %d\n",n));
	}
 
	putiactn(IAC,DO,Transmit_binary);
	n = puttn();
	debug2((stddbg,"write do binary = %d\n",n));
	n = gettn(Transmit_binary);
	if (n<=0) return(-1);
	ni = 0;
	if (!(byte(fromuserbuf[ni++])==IAC)) return(-1);	
	if (!(byte(fromuserbuf[ni])==WILL) && !(byte(fromuserbuf[ni])==DO)) return(-1);
	ni++;
	if (!(byte(fromuserbuf[ni++])==Transmit_binary)) return(-1);	
	debug2((stddbg,"read will/do binary = %d\n",n));
 
	putiactn(IAC,WILL,Transmit_binary);
	n = puttn();
	debug2((stddbg,"write will binary = %d\n",n));
	n = gettn(Transmit_binary);
	if (n<=0) return(-1);
	ni = 0;
	if (!(byte(fromuserbuf[ni++])==IAC)) return(-1);	
	if (!(byte(fromuserbuf[ni])==DO) && !(byte(fromuserbuf[ni])==WILL)) return(-1);
	ni++;
	if (!(byte(fromuserbuf[ni++])==Transmit_binary)) return(-1);	
	debug2((stddbg,"read do/will binary = %d\n",n));
	
	putiacbuf(ClearKey,IAC,EOR);
	debug2((stddbg,"Send ClearKey befor em3278\n"));
	writetohost();
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> selectio is called from anyio to select and wait for data      *
 *      to send or receive between user and host                     *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
selectio(timeptr)
struct timeval *timeptr;
{
int  selret;
 
	/* when running == 0
	 * localread = fileno(stdin) if userdoes login else - 1
	 * localwrite = fileno(stdout)
	 * when running == 1
	 * localread = socket number of active onhost socket
	 * localwrite = ditto
	 */
 
	ibits =  obits = 0;
	/*if something to send to host, send it else read user */
	if (ntocms )
		obits |= (1 << remotesock);
	else if (localread >= 0)
		ibits |= (1 << localread);
	/*if something to send to user, send it else read host*/
	if ( nnltouser > 0 )
			obits |= (1 << localwrite);
	else
		ibits |= (1 << remotesock);
	/* this creates output if screen has VMREAD or CPREAD */
	if (telluser )
		obits |= (1 << localwrite);
	/* we usually have timeptr == 0 except for initial login
	 * when some systems suppress the VM read
	 */
again:
	errno = 0;
	debug2((stddbg,"selectio0 cms=%d,%d user=%d,%d tell=%d timep=%x\n",
	( ibits & (1 << remotesock) ), ( obits & (1 << remotesock) ),
	( ibits & (1 << localread) ), ( obits & (1 << localwrite) ),
	telluser, timeptr));
	selret = select(16, &ibits, &obits, (int *) 0,timeptr);
	cmsreadok = ibits & (1 << remotesock);
	cmswriteok =  obits & (1 << remotesock);
	localreadok = ibits & (1 << localread);
	localwriteok = obits & (1 << localwrite);
	debug2((stddbg,"selectio1 cms=%d,%d user=%d,%d errno=%d timeout=%d ntouser=%d\n",
	cmsreadok, cmswriteok, localreadok, localwriteok,
	errno,!selret, ntouser));
	if (errno == EINTR)
		goto again;
	if (automaticlogin)
		localwriteok = 1; /* because write command is suppressed */
	return(!selret); /* return 1 if timer expired */
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> setecho turns local echo on and off to protect password entry  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int oldechoon = 1;
void 
setecho(echoon)
int echoon;
{
	if (echoon == oldechoon)
		return;
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
	oldechoon = echoon;
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> useduser cleans up touserbuf after nwbytes have been used up   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
useduser(nw)
int	nw;
{
	/* we used nw bytes in buffer. There are ntouser in buffer */
	register char *p, *q, *pmax;
	if (ntouser - nw) {
		debug2((stddbg,"useduser leave %d-%d\n",ntouser,nw));
		if (nw) {
			q = touserbuf + nw; /* first unused byte */
			pmax = touserbuf + ntouser-1-nw;
			for( p = touserbuf; p <= pmax;)
				*p++ = *q++;
			q = pmax;
			while ((q>=touserbuf) && ( byte(*q) != '\n')) q--;
			nnltouser = q - touserbuf;
			ntouser -= nw;
		}
	}
	else nnltouser = ntouser = 0;
	debug2((stddbg,"useduser n= %d %d %d\n",ntouser,nnltouser,nw));
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> writetohost writes ntocms characters from tocmsbuf to the host *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
writetohost()
{
	int n;
	register char	*p, *q, *pmax;
 
	errno = 0;
	n = write(remotesock, tocmsbuf, ntocms);
	debug2((stddbg,"writetohost %d n=%d errno=%d\n",ntocms,n,errno));
	if (n <= 0) {
		if (errno == ENOBUFS || errno == EWOULDBLOCK)
			n = 0;
		else {
			debug((stddbg,"write to remote errno=%d\n",errno));
			UserStage = LostHost;
			if (!automaticlogin)
				quitanyio++;
			n = 0;
		}
	}
	if (n  && (debugvar > 3))
		dbgprcms(tocmsbuf,n,"===>To CMS n=%d\n");
	if (ntocms-n) {
		q = tocmsbuf+n;
		ntocms -= n;
		for(p = tocmsbuf, pmax = p+ntocms; p < pmax;)
				*p++ = *q++;
	}
	else {
		ntocms = 0;
	}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> writetouser scan touserbuf for keywords during login, write    *
 *      nnltouser characters from touserbuf to onhost                *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
writetouser()
{
	/*  nnltouser characters in touserbuf ready to write */
 
	register int j, nw, nb, na;
	char c;
 
	debug2((stddbg, "writetouser errno=%d n=%d %d run=%d %d\n",
		errno,nnltouser,ntouser,Running,telluser));
		if (debugvar>3) dbgprint(touserbuf,ntouser,"write to user");
 
	j = nw = na = 0;
	nb = 0;
	if (Running )
		goto part2;
 
	if ( nnltouser > 0) {
		nb = nnltouser + CShlen;
 
		/* StartupCommands uses keyword search */
		if (keywordi < 0)
			findkeyword(touserbuf,nnltouser);
		if (automaticlogin) {
			/* do not write */
			nw = nnltouser;
			c = touserbuf[nw];
			touserbuf[nw] = '\0';
			debug((stddbg,"autologin buried these %d bytes from CMS\n%s\n",nw,touserbuf));
			touserbuf[nw] = c;
		}
		else nw = write(localwrite, touserbuf, nw);
	}
	goto out;
 
part2:
	/* nb = number of bytes to use from buffer
	 * na = number of additional bytes to write
	 */
	na = 0;
	if (pendofmsg) {
		debug2((stddbg,"pendof %d %d %d\n",
			GotNucxRetCode,nnltouser,pendofmsg-touserbuf));
		if ( nnltouser > (pendofmsg - touserbuf) )
			nnltouser = pendofmsg - touserbuf;
		pendofmsg = NULL;
	}
	if (nnltouser) {
		nb = nnltouser + CShlen;
		TOuserbuf[0] = CSdata;
		TOuserbuf[1] = nnltouser / 256;
		TOuserbuf[2] = nnltouser % 256;
		TOuserbuf[3] = CSoutput;
	}
	if (spmsgn) {
		debug2((stddbg,"writeuser: copy %d bytes at %d\n",spmsgn,nb));
		(void) memcpy(&TOuserbuf[nb],spmsg,spmsgn);
		debug2((stddbg,"writeuser: copy T=%s= s=%s=\n",&TOuserbuf[nb],spmsg+CShlen));
		na += spmsgn;
		spmsgn = 0;
	}
 
	/* if host wants to read then inform onhost */
 
	telluser = (
		telluser ||
		( ScreenStatus == VMread) ||
		( ScreenStatus == CPread) ||
		( ( ScreenStatus == UndefinedScreen) && kbd && havecursor)
	);
 
	if ( telluser) {
		if (ScreenStatus == CPread) {
			cmsstate |= needCPcmd;
		}
		else if (ScreenStatus == VMread) {
			cmsstate |= needCMSinp;
		}
		else if (ScreenStatus == UndefinedScreen) {
			if ( kbd && havecursor && !tsomore ) {
				cmsstate |= needCMScmd;
				kbd = havecursor = 0;
			}
		}
		if (NDflag &&
		    (cmsstate & (needCMSinp | needCMScmd | needCPcmd)) ) {
			NDflag = 0;
			cmsstate |= needNDread;
		}
 
		if(ntouser != nnltouser ) {
			debug2((stddbg,"writetouser: possible error, flushed %d after nl\n",
				ntouser-nnltouser));
			ntouser = nnltouser;
		}
		TOuserbuf[nb+(na++)] = CSstate;
		TOuserbuf[nb+(na++)] = cmsstate;
		debug2((stddbg,"writetouser cmsstate=%d  SpecialRc=%d\n",cmsstate, SpecialRc));
		GotNucxRetCode =  0;
	}
	telluser = 0;
 
	j = na+nb;
	if ( j > 0 ) {
		nw = write(localwrite, TOuserbuf, j);
out:
		if (nw == 0)
			nb = 0;
		else if ( nw < 0){
			if (errno == EMSGSIZE) {
				debug2((stddbg,"write to local errno=EMSGSIZE j=%d\n",j));
				errno = nw = nb = 0;
#ifndef	SIGVEC
				(void) signal(SIGALRM, intrsig);
#endif	~SIGVEC
				(void) alarm( (unsigned int) 2);
				pause();
				(void) alarm( (unsigned int) 0);
			}
			else {
				debug2((stddbg,"write to local errno=%d\n",errno));
				quitanyio++;
				return;
			}
		}
		if (debugvar > 3) {
			char *p;
			p = TOuserbuf;
			fprintf(stddbg,"writetouser errno=%d wrote nw=%d na=%d nb=%d c=",errno, nw, na, nb);
			if (nw > 20)
				nw = 20;
			for (j=0; j<nw; j++, p++)
				fprintf(stddbg, "%x ", *p);
			putc( '\n', stddbg);
		}
	}
	if (nb)
		useduser(nb-CShlen);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> main gets args, reads onhost.profil, opens a socket to the     *
 *      host, forks a child, and exits on signal from child.         *
 *      Child connects to host and does login, then calls running    *
 *      to handle onhost interaction                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
main(argc,argv)
int argc;
char *argv[];
{
 
	int rand(), srand();
	long time();
	int rc;
	/* set the locale to the default */
	setlocale(LC_ALL, "");

	/* open the message catalog */
	catd = catopen("hostconn.cat",NL_CAT_LOCALE);
	rc = 0;
	stddbg = stderr;
	ScreenStatus = UndefinedScreen;
	localread = remotesock = -1;
	localwrite = fileno(stdout);
	automaticlogin = 1;
	iplrequested = ConnectOnly = 0;
	forkid = -1;
	if ( getargs(argc, argv) )
		exit(1);
	if ( hostsys == hostunk)
		ConnectOnly = 1;  /* do not try automatic login for unknown */
	if ( ConnectOnly ) {
		printf(catgets(catd,HOSTCON0_ERR,HC_CONN_ONLY,
			"Connection only - use onhost to login to the host and execute onhostld.\n"));
	}
	if (findprofile() )
		exit(1);
	if (opensockfile() )
		exit(1);
	init3270();
	if (setjmp(ExitOne) != 0) {
		if (forkid == 0) {
			if (Running) {
				printf(catgets(catd,HOSTCON0_ERR,HC_INTER,
					"%s got interrupt and closes\n"),argv[0]);
				(void) strcpy(fromuserbuf,"logoff\n");
				ntocms = 0;
				(void) usertocmsbuf( strlen(fromuserbuf) );
				writetohost();
				closeit();
				exit(1);
			}
			exit(0);
		}
		exit(0);
	}
#ifdef	SIGVEC
	newvec.sv_mask = 1;
	newvec.sv_onstack = 0;
	newvec.sv_handler = intrsig;
	sigvec(SIGTERM, &newvec, &oldvec);
	sigvec(SIGALRM, &newvec, &oldvec);
#else	~SIGVEC
	(void) signal(SIGTERM, intrsig);
#endif	~SIGVEC
	if (debugvar)
		(void) fflush(stddbg);
	pidmain = getpid();
	if (0 != (forkid = fork()))  {
		for(;;) {
			/* parent waits for child
			 * child does a kill(pidmain,SIGTERM) and
			 * we exit via longjmp in intrsig
			 */
			if (debugvar)
				(void) fclose(stddbg);
			pause();
		}
	}
#ifdef	SIGVEC
	sigvec(SIGINT, &newvec, &oldvec);
#else	~SIGVEC
	(void) signal(SIGINT, intrsig);
	(void) signal(SIGPIPE, SIG_IGN);
#endif	~SIGVEC
	makeconnection(phost);
	if ( 0 != (rc = docmssock(oncmsMake)) )
		connected = 0;
	if (connected) {
		keywordset(GotNothing);
 
		anyio();  /* connecting to host */
		printf("\n");
		kill(pidmain,SIGTERM);  /* kill the main process */
		automaticlogin = 0;
		if (UserStage == GotNucx) {
 
			rc = running();
		}
		else rc = 4;
	}
	else {
		kill(pidmain,SIGTERM);  /* kill the main process */
		if (0 == rc)
			rc = 3;
	}
	closeit();
#ifdef	AIXV3
	/* close the message catalog */
	catclose(catd);
#endif	AIXV3
	exit(rc);
}
