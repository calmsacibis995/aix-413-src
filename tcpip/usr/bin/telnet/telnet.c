static char sccsid[] = "@(#)29	1.51.1.29  src/tcpip/usr/bin/telnet/telnet.c, tcptelnet, tcpip41J, 9519A_all 5/5/95 20:48:22";
/*
 *   COMPONENT_NAME: TCPTELNET
 *
 *   FUNCTIONS: strip
 *		strip
 *		cmdgets
 *		cmdgets
 *		special
 *		upcase
 *		netflush
 *		netqadj
 *		nextitem
 *		ttyqadj
 *		sigwinch
 *		deadpeer
 *		intr
 *		intr2
 *		doescape
 *		Dump
 *		Dump
 *		Dump
 *		Dump
 *		Dump
 *		Dump
 *		Dump
 *		strcmp
 *		strcmp
 *		strcmp
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		printsuboption
 *		mode
 *		getconnmode
 *		setconnmode
 *		setcommandmode
 *		telnet
 *		while
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		if
 *		sendwinsize
 *		wontoption
 *		doflush
 *		getnextsend
 *		getsend
 *		sendcmd
 *		lclchars
 *		togdebug
 *		togdebug
 *		togglehelp
 *		getnexttoggle
 *		gettoggle
 *		toggle
 *		toggle
 *		dolinemode
 *		docharmode
 *		docharmode
 *		getnextmode
 *		getmodecmd
 *		modecmd
 *		if
 *		if
 *		setupterminals
 *		emulateoff
 *		do3270
 *		do3270
 *		doemulate
 *		cantemulate
 *		getemulatecmd
 *		emulate
 *		if
 *		if
 *		if
 *		if
 *		display
 *		bye
 *		if
 *		tn
 *		tn
 *		cmdprintf
 *		cmdprintf
 *		help
 *		call
 *		makeargv
 *		getnextcmd
 *		getcmd
 *		command
 *		tcp_flush
 *		otoi
 *
 *   ORIGINS: 10,27,38
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
#ifdef _AIX
 *	AIX port of 4.3 BSD client.
#endif _AIX
 */

/* New vt100 additions */

#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the Univeristy of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#ifndef _AIX
static char sccsid[] = "telnet.c	5.16 (Berkeley) 5/27/86";
#endif /* _AIX */
#endif /* not lint */

/*
 * User telnet program.
 *
 * Many of the FUNCTIONAL changes in this newest version of telnet
 * were suggested by Dave Borman of Cray Research, Inc.
 */

#ifdef _AIX
/*
 *	AIX version 11/19/87	Mark E. Carson	IBM Gaithersburg
 *	Changes:
 *	1. Miscellaneous changes for sgtty/termio differences.
 *
 *	2. Add in support for vt100 and 3270 emulation.
 *
 *	3. Fix up the terminal type negotiation to be compatible with
 *	both old AIX and old BSD (as well as new AIX) servers.
 *
#ifdef _CSECURITY
 *	4. Add SAK support: on startup, request DO TELOPT_SAK from the
 *	remote side.  If we get WILL TELOPT_SAK in reply, then on receipt
 *	of a "send sak" command from the user, we send IAC SAK to the
 *	remote side.
#endif _CSECURITY
 *
 *	5. Fix negotiation for EOR and BINARY. Change fs3270 to use the right
 *	   buffer. Handle case of split 3270 data stream. Merge in fixes from
 *	   old version. 	Greg Joyce	6-30-88
 *
 */
#endif /* _AIX */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/param.h>


#include <netinet/in.h>

#define	TELOPTS
#include <arpa/telnet.h>
#include <arpa/inet.h>

#ifdef _AIX
#include "ibm3270.h"
#include "vtif.h"
#endif /* _AIX */
#define TN  "tn"

/* Made these global. _mvs */
char    sibuf[3 * SCREEN_SIZE], *sbp;
char    tibuf[3 * SCREEN_SIZE], *tbp;
int     scc, tcc;

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <netdb.h>
#ifdef AIX221
#include <string.h>
#else
#include <strings.h>

#define NO_TCAP
#include <cur00.h>
#include <cur02.h>

#define set_nl()	_pfast = FALSE, 0
#define set_nonl()	_pfast = TRUE, 0

#endif /* AIX221 */

#ifdef _AIX
#include <termio.h>
#ifndef _POWER
#include <sys/hft.h>
#endif /* _POWER */
#endif /* _AIX */

#ifdef _CSECURITY
#ifdef AIX221
#include "tcpipaudit.h"
#else
#include <sys/id.h>
#include <sys/priv.h>
#include "tcpip_audit.h"
#endif
#include <sys/syslog.h>
#endif /* _CSECURITY */


#include <stdlib.h>
#include <nl_types.h>
#include "telnet_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TELNET,n,s) 

#ifndef	FD_SETSIZE
/*
 * The following is defined just in case someone should want to run
 * this telnet on a 4.2 system.
 *
 */

#define	FD_SET(n, p)	((p)->fds_bits[0] |= (1<<(n)))
#define	FD_CLR(n, p)	((p)->fds_bits[0] &= ~(1<<(n)))
#define	FD_ISSET(n, p)	((p)->fds_bits[0] & (1<<(n)))
#define FD_ZERO(p)	((p)->fds_bits[0] = 0)

#endif

#define	strip(x)	((x)&0x7f)
#define   MAXDNAME  256            /*this variable has been added to be local to telnet*/

char	ttyobuf[3*SCREEN_SIZE], *tfrontp = ttyobuf, *tbackp = ttyobuf;
#define	TTYADD(c)	{ if (!(SYNCHing||flushout)) { *tfrontp++ = c; } }
#define	TTYLOC()	(tfrontp)
#define	TTYMAX()	(ttyobuf+sizeof ttyobuf-1)
#define	TTYMIN()	(netobuf)
#define	TTYBYTES()	(tfrontp-tbackp)
#define	TTYROOM()	(TTYMAX()-TTYLOC()+1)

char	netobuf[3*SCREEN_SIZE], *nfrontp = netobuf, *nbackp = netobuf;
#define	NETADD(c)	{ *nfrontp++ = c; }
#define	NET2ADD(c1,c2)	{ NETADD(c1); NETADD(c2); }
#define NETLOC()	(nfrontp)
#define	NETMAX()	(netobuf+sizeof netobuf-1)
#define	NETBYTES()	(nfrontp-nbackp)
#define	NETROOM()	(NETMAX()-NETLOC()+1)
char	*neturg = 0;		/* one past last byte of urgent data */

char	subbuffer[100], *subpointer, *subend;	/* buffer for sub-options */
#define	SB_CLEAR()	subpointer = subbuffer;
#define	SB_TERM()	subend = subpointer;
#define	SB_ACCUM(c)	if (subpointer < (subbuffer+sizeof subbuffer)) { \
				*subpointer++ = (c); \
			}

int     donaws=0;
char	hisopts[256];
char	myopts[256];

char	doopt[] = { IAC, DO, 0 };
char	dont[] = { IAC, DONT, 0 };
char	will[] = { IAC, WILL, 0 };
char	wont[] = { IAC, WONT, 0 };

struct cmd {
	char	*name;		/* command name */
	int	helpid;		/* help string catalog id */
	char	*help;		/* help string */
	int	(*handler)();	/* routine which executes command */
	int	dohelp;		/* Should we give general help information? */
	int	needconnect;	/* Do we need to be connected to execute? */
};

int tn3270flag = 0; /* this flags will be used in profile.c */
char *tn3270cr = NULL; /* this flag will be used in mode() */

int	connected;
int	net;
int	tout;
int	showoptions = 0;
int	debug = 0;
int	crmod = 0;
int	lineterm = 0;
int	netdata = 0;
static 	FILE *NetTrace;
int	telnetport = 1;

char	*prompt;
#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c)         (c&037)

cc_t	escape = (cc_t) CTRL(']');
cc_t	echoc = (cc_t) CTRL('E');

char	*offstring;		/* pointer to memory containing "off" in
				   national language */

int	SYNCHing = 0;		/* we are in TELNET SYNCH mode */
int	flushout = 0;		/* flush output */
int	autoflush = 0;		/* flush output when interrupting? */
int	autosynch = 0;		/* send interrupt characters with SYNCH? */
int	localchars = 1;		/* we recognize interrupt/quit */
int	donelclchars = 0;	/* the user has set "localchars" */
int	dontlecho = 0;		/* do we suppress local echoing right now? */
int 	linemode = FALSE;	/* char-at-a-time mode is default */

char	line[200];
int	margc;
char	*margv[20];

jmp_buf	toplevel;
jmp_buf	peerdied;

extern	int errno;


struct sockaddr_in sin;

struct	cmd *getcmd();
struct	servent *sp;

#ifdef _AIX

#define	t_intrc		c_cc[VINTR]
#define	t_quitc		c_cc[VQUIT]
/* t_startc, t_stopc - resetting these is not supported by SysV */
#define	t_eofc		c_cc[VEOF]
#define	t_brkc		c_cc[VEOL]
/* Terminal emulation stuff */

#define	NATIVE	0
#define	VT100	1
#define	IBM3270	2

int useremulate	= 0;		/* user specified emulation */

char *terminalname;

#define	MAXTERMINALS	3
#define MAXFORMS	2


struct termnames {
	char *name[MAXFORMS];
} terminalstotry[MAXTERMINALS] = {
	{NULL,	NULL},		/* supplied by getenv(TERM) */
	{"vt100", "DEC-VT100"},
	{NULL, NULL},		/* supplied by fs_select_term() */
};

int negotiatedterm = 0, formofname = -1, sent_last = 0, lastterm = 0,
    lastform = 0;

int currentterm = 0;

#define NOT_COMMAND_MODE (globalmode != 0)
#define COMMAND_MODE (globalmode == 0)
#define TRACING_TO_FILE  (NetTrace != stdout)
#define EMULATING_3270	(currentterm == IBM3270)
#define EMULATING_VT100  (currentterm == VT100)
#define EMULATING  (currentterm != NATIVE)

extern int Lines, Cols;  /* for vt100 window size */

/* sgtty/termio stuff */
struct	tn_tchars {	/* passed into termios structure */
	cc_t c_cc[NCCS];
};
struct tn_ltchars {	/* handled locally only! */
	cc_t t_flushc;
	cc_t t_suspc;
};
#endif /* _AIX */

struct	tn_tchars otc, ntc;
struct	tn_ltchars oltc, nltc;
struct	termios ottyb, nttyb;
int	globalmode = 0;
int	flushline = 1;
#ifndef _POWER
int	is_hft;		/* true if our tty is an hft: used in vt100 emulation */
#endif /* _POWER */

char	*hostname;
char	hnamebuf[MAXDNAME];       /*the length has been changed from 32 to 256 to  accept name over 32 char*/

char initscr_done = FALSE;
char tmp_str[200];

#ifdef _AIX
/* vt100 emulation functions */
int	vt1(), vt2();
/* 3270 emulation functions */
void	fs_command();
void	fs_end();
void	fs_init();
void	fs_input();
void	fs_redraw();
void	fs_schedredraw();
char	*fs_select_term();
#endif /* _AIX */

/* Lint error fixers! */
static void Dump();
void deadpeer(int);
void sigwinch(int);
void intr(int);
void intr2(int);
void doescape(int);


/*
 * The following are some clocks used to decide how to interpret
 * the relationship between various variables.
 */

struct {
    int
	system,			/* what the current time is */
	echotoggle,		/* last time user entered echo character */
	modenegotiated,		/* last time operating mode negotiated */
	didnetreceive,		/* last time we read data from network */
	gotDM;			/* when did we last see a data mark */
} clocks;

#define	settimer(x)	clocks.x = clocks.system++

/*
 * Various utility routines.
 */

char *ambiguous;		/* special return value */
#define Ambiguous(t)	((t)&ambiguous)

#ifdef _CSECURITY
ulong	remote_addr;
char	*audit_tail[TCPIP_MAX_TAIL_FIELDS];
extern	char *sys_errlist[];
#define SAK_NOT_SET 0xff
cc_t	telnet_sak = SAK_NOT_SET;
char	*sakopt = "SUPPORT SAK";
uid_t	saved_uid, effective_uid;
#endif /* _CSECURITY */

#ifdef DUMPWRITE
	FILE *fp, *fopen();
#endif /* DUMPWRITE */

char **
genget(name, table, next)
char	*name;		/* name to match */
char	**table;		/* name entry in table */
char	**(*next)();	/* routine to return next entry in table */
{
	register char *p, *q;
	register char **c, **found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = table; p = *c; c = (*next)(c)) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return Ambiguous(char **);
	return (found);
}

/*
 * cmdprintf
 *
 *	performs printf() depending on the currentterm value.
 */

/*VARARGS*/
cmdprintf(str,arg1,arg2,arg3,arg4,arg5)
  char *str,*arg1,*arg2,*arg3,*arg4,*arg5;	
{
	char buf[100];
	if (EMULATING_VT100 && NOT_COMMAND_MODE)
	{	char buf[256];
		short oldNL;
		oldNL = vtctl(VTC_NL);	

		vtctl(VTC_SNL,TRUE);	/* tell vtif to do newline processing */
		sprintf(buf,str,arg1,arg2,arg3,arg4,arg5);
		vtaddstr(buf);
		vtctl(VTC_SNL,oldNL);
	}
	else 	
	{	printf(str,arg1,arg2,arg3,arg4,arg5);			
		fflush(stdout);
	}
}

char *cmdgets(line,len)
  char *line;
  int  len;

{	char c;
	int rc,i; 
	int oldNL;

	if (EMULATING_VT100 && NOT_COMMAND_MODE) {
		/* Temporarily turn on Newline processing for VT100s */
		if (EMULATING_VT100)
		{	oldNL = vtctl(VTC_NL);
			vtctl(VTC_SNL,TRUE);
		}

		/* Pick up the line while deactivating escape sequences for
		 * vt100s and recognizing \r as an extra terminator for them.
		 */
		rc = TRUE;
		for (i=0;;)
		{ 	if (read(0,&c,1) < 1) 
			{	rc = FALSE;
				break;
			}
			if (i >= len) c == '\n';
			if (EMULATING_VT100 && c != ESC) {
				vtaddch(c);
				refresh();
			}
			if (c == '\n' || (EMULATING_VT100 && c == '\r')) 
				break;
			line[i++] = c;
		}

		/* Possibly reset the Newline status */	
		if (EMULATING_VT100) 
			vtctl(VTC_SNL,oldNL);

		/* Maybe quit on error/eof */	
		if (!rc) return(NULL);

		/* Terminate the line and maybe edit for backspaces */
		line[i] = '\0';
		
		/* Possibly reset the Newline status */	
		if (EMULATING_VT100)
		{ 	char *p,*q;
			for (p=q=line; *p; p++)
			{	if (*p != '\b') *q++ = *p;
				else if (q>line) q--;
			}
			*q = 0;
		}
		return(line);
	} else {
		return(gets(line));
	}
}


/*
 * cmdmsgenq(msg), cmdflush():
 *
 * Allow messages for viewing in command mode to be queued during
 * connection mode.
 *
 * It is possible to have an error during the negotiation of emulation
 * and we will have to fall out of emulation back into the command loop.
 * If any messages were issued during the connected session, we will 
 * need to save them.
 */

struct msgqent {
	struct msgqent 	*left, 	/* listhead                */
			*right; /* next or listend         */
	char   msg[1];		/* space for the null only */
};

/* I wish I could lexically scope this -- waltr */
struct msgqent cmdmsg_q = {NULL, NULL, ""};	

cmdmsgenq(msg)
  register char *msg;
{
	register struct msgqent *p;
	register int size;

	size = sizeof(struct msgqent) + strlen(msg);
	p    = (struct msgqent *) malloc(size);
	strcpy(p->msg, msg);
	
	if (!cmdmsg_q.left)		/* update listhead if he's undefined*/
		cmdmsg_q.left = p;
	if (cmdmsg_q.right)		/* append p if an old node is there */
		cmdmsg_q.right->right = p;
	cmdmsg_q.right = p;		/* update listend */
	p->right = NULL;		/* terminate list */
}

/*	Flush the queue */

cmdflush()
{	
	struct msgqent *p,*q = cmdmsg_q.left;
	while (p=q) {
		cmdprintf("%s",p->msg);
		q = p->right;
		free(p);
	}
	cmdmsg_q.left = cmdmsg_q.right = NULL;
}


/*
 * Make a character string into a number.
 *
 * Todo:  1.  Could take random integers (12, 0x12, 012, 0b1).
 */

special(s)
register char *s;
{
	register char c;
	char b;

	switch (*s) {
	case '^':
		b = *++s;
		if (b == '?') {
		    c = b | 0x40;		/* DEL */
		} else {
		    c = b & 0x1f;
		}
		break;
	default:
		c = *s;
		break;
	}
	return c;
}

/*
 * Construct a control character sequence
 * for a special character.
 */
char *
control(c)
	register cc_t c;
{
	static char buf[3];

	if (c == 0x7f)
		return ("^?");
	if (c == '\377') {
		return offstring;
	}
	if (c >= 0x20) {
		buf[0] = c;
		buf[1] = 0;
	} else {
		buf[0] = '^';
		buf[1] = '@'+c;
		buf[2] = 0;
	}
	return (buf);
}


/*
 * upcase()
 *
 *	Upcase (in place) the argument.
 */

void
upcase(argument)
register char *argument;
{
    if (MB_CUR_MAX > 1) {       /* multibyte code */
        wchar_t *wc, *tmpwc;
        int n, mbcount;

        n = (strlen(argument) + 1) * sizeof(wchar_t);
        tmpwc = wc = (wchar_t *)malloc(n);
        /* convert multibyte string to widechar string for processing */
        mbcount = mbstowcs(wc, argument, n);
        if (mbcount >=  0) {
                while (*wc) {
                        *wc = towupper((wint_t) *wc);
                        wc++;
                }
                /*convert widechar string to multibyte string */
                wcstombs(argument, tmpwc, n);
        }
	free(wc);
     }
     else {             /* single byte code */
        register int c;

        while (c = *argument) {
                if(islower(c))
                        *argument = toupper(c);
        argument++;
        }
     }
}

/*
 * Check to see if any out-of-band data exists on a socket (for
 * Telnet "synch" processing).
 */

int
stilloob(s)
int	s;		/* socket number */
{
    static struct timeval timeout = { 0 };
    fd_set	excepts;
    int value;


    do {
	FD_ZERO(&excepts);
	FD_SET(s, &excepts);
	value = select(s+1, (fd_set *)0, (fd_set *)0, &excepts, &timeout);
    } while ((value == -1) && (errno = EINTR));

    if (value < 0) {
	perror(MSGSTR(P_SELECT, "select")); /*MSG*/
	quit();
    }
    if (FD_ISSET(s, &excepts)) {
	return 1;
    } else {
	return 0;
    }
}


/*
 *  netflush
 *		Send as much data as possible to the network,
 *	handling requests for urgent data.
 */


netflush(fd)
{
    int n;


    if ((n = nfrontp - nbackp) > 0) {
	if (!neturg) {
	    n = write(fd, nbackp, n);	/* normal write */
	} else {
	    n = neturg - nbackp;
	    /*
	     * In 4.2 (and 4.3) systems, there is some question about
	     * what byte in a sendOOB operation is the "OOB" data.
	     * To make ourselves compatible, we only send ONE byte
	     * out of band, the one WE THINK should be OOB (though
	     * we really have more the TCP philosophy of urgent data
	     * rather than the Unix philosophy of OOB data).
	     */
	    if (n > 1) {
		n = send(fd, nbackp, n-1, 0);	/* send URGENT all by itself */
	    } else {
		n = send(fd, nbackp, n, MSG_OOB);	/* URGENT data */
	    }
	}
    }
    if (n < 0) {
	if (errno != ENOBUFS && errno != EWOULDBLOCK) {
	    setcommandmode();
	    perror(hostname);
	    close(fd);
	    neturg = 0;
	    longjmp(peerdied, -1);
	    /*NOTREACHED*/
	}
	n = 0;
    }
    if (netdata && n) {
	Dump('>', nbackp, n);
    }
    nbackp += n;
    if (nbackp >= neturg) {
	neturg = 0;
    }
    netqadj();
/*
    if (nbackp == nfrontp) {
        nbackp = nfrontp = netobuf;
    }
*/

}

/*
 * netqadj()
 *
 */
netqadj()
{
        int n = NETBYTES();
        if((n > 0) && (nbackp > netobuf)){
                memcpy(netobuf,nbackp,n);
                nbackp = netobuf ;
                nfrontp = netobuf+n ;
                return;
        }
        if(n == 0)
                nbackp = nfrontp = netobuf ;
        return;
}


/*
 * nextitem()
 *
 *	Return the address of the next "item" in the TELNET data
 * stream.  This will be the address of the next character if
 * the current address is a user data character, or it will
 * be the address of the character following the TELNET command
 * if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */

char *
nextitem(current)
char	*current;
{


    if ((*current&0xff) != IAC) {
	return current+1;
    }
    switch (*(current+1)&0xff) {
    case DO:
    case DONT:
    case WILL:
    case WONT:
	    return current+3;
    case SB:		/* loop forever looking for the SE */
	{
	    register char *look = current+2;

	    for (;;) {
		if ((*look++&0xff) == IAC) {
		    if ((*look++&0xff) == SE) {
			return look;
		    }
		}
	    }
	}
    default:
	return current+2;
    }
}
/*
 * netclear()
 *
 *	We are about to do a TELNET SYNCH operation.  Clear
 * the path to the network.
 *
 *	Things are a bit tricky since we may have sent the first
 * byte or so of a previous TELNET command into the network.
 * So, we have to scan the network buffer from the beginning
 * until we are up to where we want to be.
 *
 *	A side effect of what we do, just to keep things
 * simple, is to clear the urgent data pointer.  The principal
 * caller should be setting the urgent data pointer AFTER calling
 * us in any case.
 */

netclear()
{
    register char *thisitem, *next;
    char *good;
#define	wewant(p)	((nfrontp > p) && ((*p&0xff) == IAC) && \
				((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))


    thisitem = netobuf;

    while ((next = nextitem(thisitem)) <= nbackp) {
	thisitem = next;
    }

    /* Now, thisitem is first before/at boundary. */

    good = netobuf;	/* where the good bytes go */

    while (nfrontp > thisitem) {
	if (wewant(thisitem)) {
	    int length;

	    next = thisitem;
	    do {
		next = nextitem(next);
	    } while (wewant(next) && (nfrontp > next));
	    length = next-thisitem;
	    bcopy(thisitem, good, length);
	    good += length;
	    thisitem = next;
	} else {
	    thisitem = nextitem(thisitem);
	}
    }

    nbackp = netobuf;
    nfrontp = good;		/* next byte to be sent */
    neturg = 0;

}

/*
 * Send as much data as possible to the terminal.
 */


ttyflush()
{
    int n;


    if ((n = tfrontp - tbackp) > 0) {
	if (!(SYNCHing||flushout)) {
        if (EMULATING_VT100) { }
        else n = write(tout, tbackp, n);
	} else {
#ifdef _AIX
	    ioctl(tout, TCFLSH, 1);
#else
	    ioctl(fileno(stdout), TCFLUSH, (char *) 0);
#endif /* _AIX */
	    /* we leave 'n' alone! */
	}
    }
    if (n < 0) {
	if (errno == EIO)
		tbackp = tfrontp = ttyobuf;
	else
		perror(MSGSTR(P_TTYFLUSH, "ttyflush ")); /*MSG*/
	return;
    }
    tbackp += n;
    ttyqadj(); 
/*
    if (tbackp == tfrontp) {
	tbackp = tfrontp = ttyobuf;
    }
*/

}
/*
 * ttyqadj()
 *
 */
ttyqadj()
{
        int n = TTYBYTES();
        if((n > 0) && (tbackp > ttyobuf)){
                memcpy(ttyobuf,tbackp,n);
                tbackp = ttyobuf ;
                tfrontp = ttyobuf+n ;
                return;
        }
        if(n == 0) 
                tbackp = tfrontp = ttyobuf ;
	return;
}

/*
 * Various signal handling routines.
 */

void sigwinch(int dummy)
{
        if (donaws)
                sendwinsize();
}

void deadpeer(int dummy)
{
	setcommandmode();
	longjmp(peerdied, -1);
}

void intr(int dummy)
{
    signal(SIGINT, SIG_IGN);
    if (localchars) {
	intp();
        signal(SIGINT, (void(*)(int))intr);
	return;
    }
    setcommandmode();
    longjmp(toplevel, -1);
}

void intr2(int dummy)
{
    if (localchars) {
	sendbrk();
	return;
    }
}

void doescape(int dummy)
{
    command(0);
}

/*
 * The following are routines used to print out debugging information.
 */


static void
Dump(direction, buffer, length)
char	direction;
char	*buffer;
int	length;
{
#   define BYTES_PER_LINE	16
#   define min(x,y)	((x<y)? x:y)
    char *pThis;
    int offset;

    offset = 0;

    while (length) {
	/* print one line */
	if (TRACING_TO_FILE) 
		fprintf(NetTrace, "%c %02X\t", direction, offset);
	else
		cmdprintf("%c %02X\t", direction, offset);
	pThis = buffer;
	buffer = buffer+min(length, BYTES_PER_LINE);
	while (pThis < buffer) {
	    if (TRACING_TO_FILE) 
		fprintf(NetTrace, "%02X ", (*pThis) & 0xff);
	    else
		cmdprintf("%02X ", (*pThis) & 0xff); 
	    pThis++;
	}
        if (TRACING_TO_FILE) 
		fprintf(NetTrace, "\n");
	else
		cmdprintf("\r\n");
	length -= BYTES_PER_LINE;
	offset += BYTES_PER_LINE;
	if (length < 0) {
	    return;
	}
	/* find next unique line */
    }
}


static int	cat_telopts[][5] = {
/* BINARY */
    { BINARY_DO,   BINARY_DONT,   BINARY_WILL,   BINARY_WONT,   BINARY_UNKN  },
/* ECHO */
    { ECHO_DO,     ECHO_DONT,     ECHO_WILL,     ECHO_WONT,     ECHO_UNKN    },
/* RCP */
    { RCP_DO,      RCP_DONT,      RCP_WILL,      RCP_WONT,      RCP_UNKN     },
/* SUPPRESS GO AHEAD */
    { SUPPRESS_DO, SUPPRESS_DONT, SUPPRESS_WILL, SUPPRESS_WONT, SUPPRESS_UNKN },
/* NAME */
    { NAME_DO,     NAME_DONT,     NAME_WILL,     NAME_WONT,     NAME_UNKN     },
/* STATUS */
    { STATUS_DO,   STATUS_DONT,   STATUS_WILL,   STATUS_WONT,   STATUS_UNKN },
/* TIMING MARK */
    { TM_MARK_DO,  TM_MARK_DONT,  TM_MARK_WILL,  TM_MARK_WONT,  TM_MARK_UNKN },
/* RCTE */
    { RCTE_DO,     RCTE_DONT,     RCTE_WILL,     RCTE_WONT,     RCTE_UNKN },
/* NAOL */
    { NAOL_DO,     NAOL_DONT,     NAOL_WILL,     NAOL_WONT,     NAOL_UNKN },
/* NAOP */
    { NAOP_DO,     NAOP_DONT,     NAOP_WILL,     NAOP_WONT,     NAOP_UNKN },
/* NAOCRD */
    { NAOCRD_DO,   NAOCRD_DONT,   NAOCRD_WILL,   NAOCRD_WONT,   NAOCRD_UNKN },
/* NAOHTS */
    { NAOHTS_DO,   NAOHTS_DONT,   NAOHTS_WILL,   NAOHTS_WONT,   NAOHTS_UNKN },
/* NAOHTD */
    { NAOHTD_DO,   NAOHTD_DONT,   NAOHTD_WILL,   NAOHTD_WONT,   NAOHTD_UNKN },
/* NAOFFD */
    { NAOFFD_DO,   NAOFFD_DONT,   NAOFFD_WILL,   NAOFFD_WONT,   NAOFFD_UNKN },
/* NAOVTS */
    { NAOVTS_DO,   NAOVTS_DONT,   NAOVTS_WILL,   NAOVTS_WONT,   NAOVTS_UNKN },
/* NAOVTD */
    { NAOVTD_DO,   NAOVTD_DONT,   NAOVTD_WILL,   NAOVTD_WONT,   NAOVTD_UNKN },
/* NAOLFD */
    { NAOLFD_DO,   NAOLFD_DONT,   NAOLFD_WILL,   NAOLFD_WONT,   NAOLFD_UNKN },
/* EXTEND ASCII */
    { EXT_ASC_DO,  EXT_ASC_DONT,  EXT_ASC_WILL,  EXT_ASC_WONT,  EXT_ASC_UNKN },
/* LOGOUT */
    { LOGOUT_DO,   LOGOUT_DONT,   LOGOUT_WILL,   LOGOUT_WONT,   LOGOUT_UNKN },
/* BYTE MACRO */
    { BYTE_MAC_DO, BYTE_MAC_DONT, BYTE_MAC_WILL, BYTE_MAC_WONT, BYTE_MAC_UNKN },
/* DATA ENTRY TERMINAL */
    { DET_DO,      DET_DONT,      DET_WILL,      DET_WONT,      DET_UNKN },
/* SUPDUP */
    { SUPDUP_DO,   SUPDUP_DONT,   SUPDUP_WILL,   SUPDUP_WONT,   SUPDUP_UNKN },
/* SUPDUP OUTPUT */
    { SUPOUT_DO,   SUPOUT_DONT,   SUPOUT_WILL,   SUPOUT_WONT,   SUPOUT_UNKN },
/* SEND LOCATION */
    { SEND_LOC_DO, SEND_LOC_DONT, SEND_LOC_WILL, SEND_LOC_WONT, SEND_LOC_UNKN },
/* TERMINAL TYPE */
    { TTYPE_DO,    TTYPE_DONT,    TTYPE_WILL,    TTYPE_WONT,    TTYPE_UNKN },
/* END OF RECORD */
    { EOR_DO,      EOR_DONT,      EOR_WILL,      EOR_WONT,      EOR_UNKN },
};
static int	cat_nawsopts[5] = {
      DO_NAWS,     DONT_NAWS,     WILL_NAWS,     WONT_NAWS,     UNK_NAWS  
};
static int	cat_sakopts[5] = {
      DO_SAKFMT,   DONT_SAKFMT,   WILL_SAKFMT,   WONT_SAKFMT,   UNK_SAKFMT
};
static int	cat_decopts[5] = {
      DO_DECFMT,   DONT_DECFMT,   WILL_DECFMT,   WONT_DECFMT,   UNK_DECFMT
};


/*VARARGS*/
printoption(direction, fmt, option, what)
	char *direction, *fmt;
	int option, what;
{
	int	fmt_offset;		/* offset into array of message
					   catalog ids */
	char	*replymsg;		/* pointer to reply/noreply message */
	char	*send_rcvd;		/* pointer to SENT/RCVD tag */
	char	*t;			/* pointer to string returned by
					   MSGSTR() */
	char	*textstr;		/* pointer to format string returned
					   by MSGSTR() */
	static char	*yreplymsg = NULL;	/* pointer to reply message
						   from catalog */
	static char	*nreplymsg = NULL;	/* pointer to noreply message
						   from catalog */
	static char	*sendtag = NULL;	/* pointer to SENT tag
						   from catalog */
	static char	*rcvdtag = NULL;	/* pointer to RCVD tag
						   from catalog */
	if (!showoptions)
		return;
	if (*direction == '<') {
		replymsg = NULL;
	}
	else {
		if (what) {
			if (yreplymsg == NULL) {
				t = MSGSTR(YREPLYSTR, "(reply)");
				yreplymsg = (char *)malloc(strlen(t) + 1);
				strcpy(yreplymsg, t);
			}
			replymsg = yreplymsg;
		}
		else {
			if (nreplymsg == NULL) {
				t = MSGSTR(NREPLYSTR, "(don't reply)");
				nreplymsg = (char *)malloc(strlen(t)+ 1);
				strcpy(nreplymsg, t);
			}
			replymsg = nreplymsg;
		}
	}
	if (!strcmp(direction+1, "SENT")) {
		if (sendtag == NULL) {
			t = MSGSTR(SENDTAG, "SENT");
			sendtag = (char *)malloc(strlen(t) + 1);
			strcpy(sendtag, t);
		}
		send_rcvd = sendtag;
	}
	else if (!strcmp(direction+1, "RCVD")) {
		if (rcvdtag == NULL) {
			t = MSGSTR(RCVDTAG, "RCVD");
			rcvdtag = (char *)malloc(strlen(t) + 1);
			strcpy(rcvdtag, t);
		}
		send_rcvd = rcvdtag;
	}
	else {
		send_rcvd = direction+1;
	}
	if (fmt == doopt) {
		fmt_offset = 0;
		fmt = "do";
	}
	else if (fmt == dont) {
		fmt_offset = 1;
		fmt = "dont";
	}
	else if (fmt == will) {
		fmt_offset = 2;
		fmt = "will";
	}
	else if (fmt == wont) {
		fmt_offset = 3;
		fmt = "wont";
	}
	else {
		fmt_offset = 4;
		fmt = "???";
	}
	if (option < (sizeof telopts/sizeof telopts[0])) {
		textstr = MSGSTR(cat_telopts[option][fmt_offset], NULL);
		if (textstr != NULL) {
			cmdprintf(textstr, send_rcvd, replymsg);
		}
		else {
			cmdprintf("%s %s %s %s\r\n", send_rcvd, fmt,
				telopts[option], replymsg);
		}
	}
	else if (option == TELOPT_NAWS) {
		textstr = MSGSTR(cat_nawsopts[fmt_offset], NULL);
		if (textstr != NULL) {
			cmdprintf(textstr, send_rcvd, replymsg);
		}
		else {
			cmdprintf("%s %s %s %s\r\n", send_rcvd, fmt, sakopt,
				replymsg);
		}
	}
#ifdef _CSECURITY
	else if (option == TELOPT_SAK) {
		textstr = MSGSTR(cat_sakopts[fmt_offset], NULL);
		if (textstr != NULL) {
			cmdprintf(textstr, send_rcvd, replymsg);
		}
		else {
			cmdprintf("%s %s %s %s\r\n", send_rcvd, fmt, sakopt,
				replymsg);
		}
	}
#endif /* _CSECURITY */
	else {
		textstr = MSGSTR(cat_decopts[fmt_offset], NULL);
		if (textstr != NULL) {
			cmdprintf(textstr, send_rcvd, option, replymsg);
		}
		else {
			cmdprintf("%s %s %d %s\r\n", send_rcvd, fmt, option,
				replymsg);
		}
	}
}

#ifdef _AIX
/* VARARGS */
printsuboption(direction, suboption, string)
char *direction, *string;
int suboption;
{
	char	*send_rcvd;		/* pointer to SENT/RCVD tag */
	char	*t;			/* pointer to string returned by
					   MSGSTR() */
	static char	*sendtag = NULL;	/* pointer to SENT tag
						   from catalog */
	static char	*rcvdtag = NULL;	/* pointer to RCVD tag
						   from catalog */
	if (!showoptions)
		return;
	if (!strcmp(direction+1, "SENT")) {
		if (sendtag == NULL) {
			t = MSGSTR(SENDTAG, "SENT");
			sendtag = (char *)malloc(strlen(t) + 1);
			strcpy(sendtag, t);
		}
		send_rcvd = sendtag;
	}
	else if (!strcmp(direction+1, "RCVD")) {
		if (rcvdtag == NULL) {
			t = MSGSTR(RCVDTAG, "RCVD");
			rcvdtag = (char *)malloc(strlen(t) + 1);
			strcpy(rcvdtag, t);
		}
		send_rcvd = rcvdtag;
	}
	else
		send_rcvd = direction + 1;

	if (suboption == TELOPT_TTYPE)
		cmdprintf(MSGSTR(SUB_TELOPT, "%s suboption TELOPT_TTYPE %s\r\n"), /*MSG*/
			send_rcvd, string);
	else if (suboption == TELOPT_NAWS)
		cmdprintf(MSGSTR(SUB_NAWS, "%s suboption TELOPT_NAWS %s\r\n"),
			send_rcvd, string);
	else
		cmdprintf(MSGSTR(SUB_UNKNOWN, "%s suboption ???(%d) %s\r\n"), /*MSG*/
			send_rcvd, suboption, string);

}
#endif /* _AIX */


/*
 * Defect 171726 -- Since telnet changes the tty state out from underneath
 * curses, we need to let curses know about any tty characteristics which
 * will affect its internal processing.  We can't call the curses tty
 * state functions (like nl() and nonl(), for example) without adversely
 * affecting the current terminal state, so we need to call some functions
 * which set the appropriate curses state variables but do not affect the
 * tty.
 */
static void
update_curses(termio)
	struct termios *termio;
{
#ifndef AIX221
	if (!initscr_done)
		return;
	if ((termio->c_oflag & ONLCR) && (termio->c_oflag & OPOST))
		set_nl();
	else
		set_nonl();
#endif
}

/*
 * Mode - set up terminal to a specific mode.
 */


mode(f)
	register int f;
{

#define	NOCHAR	0
	static int prevmode = 0;
	struct tn_tchars *tc;
	struct tn_ltchars *ltc;
	struct termios sb;
	int onoff, old;
	struct	tn_tchars notc2;
	struct	tn_ltchars noltc2;
#ifdef _AIX
					/* cc[VMIN=4] = 1 */
	static struct tn_tchars notc = {-1, -1, 0, 0, 1, 0, 0, CSTART, CSTOP, -1, -1, 0, 0, 0, 0, 0};
	static struct tn_ltchars noltc = {NOCHAR, NOCHAR};
#else
	static struct	tchars notc =	{ -1, -1, -1, -1, -1, -1 };
	static struct	ltchars noltc =	{ -1, -1, -1, -1, -1, -1 };
#endif /* _AIX */


	globalmode = f;
	if (prevmode == f) {
		return;
	}
	old = prevmode;
	prevmode = f;
	sb = nttyb;
	sb.c_lflag |= ISIG ;
	sb.c_iflag &= ~BRKINT ;
	sb.c_cc[VSUSP] = -1;
	sb.c_cc[VDSUSP] = -1 ;
	sb.c_cc[VMIN] = 1;
	switch (f) {

	case 0:
		onoff = 0;
		tc = &otc;
		ltc = &oltc;
		sb.c_lflag |= ICANON ;
		if (EMULATING)
			sb.c_iflag |= ICRNL;
		break;

	case 1:		/* remote character processing, remote echo */
	case 2:		/* remote character processing, local echo */
#ifdef _AIX
		sb.c_lflag &= ~ICANON;

#else
		sb.sg_flags |= CBREAK;
#endif /* _AIX */
		if (f == 1)
#ifdef _AIX
			{
			sb.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
			sb.c_iflag &= ~ICRNL;
			sb.c_oflag &= ~ONLCR;
			}
#else
			sb.sg_flags &= ~(ECHO|CRMOD);
#endif /* _AIX */
		else
#ifdef _AIX
			{
			sb.c_lflag |= ECHO|ECHOE|ECHOK;
			sb.c_iflag |= ICRNL;
			sb.c_oflag |= ONLCR;
			sb.c_oflag &= ~ONLRET;
			}
#else
			sb.sg_flags |= ECHO|CRMOD;
#endif /* _AIX */
#ifdef _AIX
		/* We can't specify both VMIN and VTIME values and special
		 * chars at the same time!
		 */
#else
		sb.c_cc[VERASE] = sb.c_cc[VKILL] = -1;
#endif /* _AIX */
		tc = &notc;
		/*
		 * If user hasn't specified one way or the other,
		 * then default to not trapping signals.
		 */
		if (!donelclchars) {
			localchars = 0;
		}
#ifdef _AIX
		/* We can't specify both VMIN and VTIME values and special
		 * chars at the same time!
		 */
#else
		if (localchars) {
			notc2 = notc;
			notc2.t_intrc = ntc.t_intrc;
			notc2.t_quitc = ntc.t_quitc;
			tc = &notc2;
		} else
			tc = &notc;
#endif /* _AIX */
		ltc = &noltc;
		onoff = 1;
		break;
	case 3:		/* local character processing, remote echo */
	case 4:		/* local character processing, local echo */
	case 5:		/* local character processing, no echo */
#ifdef _AIX
		sb.c_lflag |= ICANON;
		sb.c_iflag |= ICRNL;
		sb.c_oflag |= ONLCR;
		sb.c_oflag &= ~ONLRET;
#else
		sb.sg_flags &= ~CBREAK;
		sb.sg_flags |= CRMOD;
#endif /* _AIX */
		if (f == 4)
#ifdef _AIX
			sb.c_lflag |= ICANON|ECHO|ECHOE|ECHOK;
#else
			sb.sg_flags |= ECHO;
#endif /* _AIX */
		else
#ifdef _AIX
			sb.c_lflag &= ~(ECHO|ECHOE|ECHOK);
#else
			sb.sg_flags &= ~ECHO;
#endif /* _AIX */
		notc2 = ntc;
		tc = &notc2;
		noltc2 = oltc;
		ltc = &noltc2;
		/*
		 * If user hasn't specified one way or the other,
		 * then default to trapping signals.
		 */
		if (!donelclchars) {
			localchars = 1;
		}
		if (localchars) {
			notc2.t_brkc = nltc.t_flushc;
			noltc2.t_flushc = NOCHAR;	/* CHANGED */
		} else {
			notc2.t_intrc = notc2.t_quitc = -1;	/* CHANGED */
		}
		noltc2.t_suspc = escape;
#ifndef _AIX
		noltc2.t_dsuspc = -1;
#endif /* not _AIX */
		onoff = 1;
		break;

#ifdef _AIX
	case 8:		/* suitable for fs_3270 */
			/*
			 * Gaithersburg sgtty -> termio mappings are too
			 * bizarre for me so I'll just use sysV termio
			 */
		{
		struct termios	termio;
		tcgetattr(0,(struct termios *)&termio);
		termio.c_oflag = ONLCR | OPOST;
		termio.c_lflag &= ~ICANON;
		termio.c_lflag &= ~ECHO;
		termio.c_lflag |= ISIG ;
		termio.c_iflag &= ~BRKINT ;
		if (tn3270cr && tn3270flag)
			termio.c_iflag &= ~ICRNL;
		else
			termio.c_iflag |= ICRNL;
		termio.c_cc[VMIN] = 1;
		termio.c_cc[VTIME] = 1;
		termio.c_cc[VSUSP] = -1;
		termio.c_cc[VDSUSP] = -1;
		termio.c_cc[VINTR] = ntc.t_intrc;
		tcsetattr(0,TCSANOW,(struct termios *)&termio);
		tcgetattr(1,(struct termios *)&termio);
		termio.c_oflag |= ONLCR;
		update_curses(&termio);
		tcsetattr(1,TCSANOW,(struct termios *)&termio);
		if (EMULATING_3270) {
			if (f == 0) /* schedule redraw to occur on return */
				fs_schedredraw();
			else
				fs_redraw();	/* redraw if scheduled */
		}
		return;
		}
#endif /* _AIX */

	default:
		return;
	}
#ifdef _AIX
	/* copy the tc array into the termio structure.  Note we can
	 * only use the ltc information locally, thus we don't worry
	 * about whether we trap it or not - we always "trap" it.
	 */
	(void) memcpy(sb.c_cc, tc->c_cc, NCCS * sizeof(cc_t));
#else
	ioctl(fileno(stdin), TIOCSLTC, (char *)ltc);
	ioctl(fileno(stdin), TIOCSETC, (char *)tc);
#endif /* _AIX */
	update_curses(&sb);
	tcsetattr(fileno(stdin),TCSANOW,(struct termios *)&sb);
#ifndef _AIX
	if (f >= 3)
		signal(SIGTSTP, (void(*)(int))doescape);
	else if (old >= 3) {
		signal(SIGTSTP, SIG_DFL);
		sigsetmask(sigblock(0) & ~(1<<(SIGTSTP-1)));
	}
#endif /* not _AIX */
#ifdef _AIX
	if (EMULATING_3270 || EMULATING_VT100) {
		if (f == 0) /* schedule redraw to occur on return */
			fs_schedredraw();
		else
			fs_redraw();	/* redraw if scheduled */
	}
#endif /* _AIX */
}

/*
 * These routines decides on what the mode should be (based on the values
 * of various global variables).
 */

char *modedescriptions[] = {
	"telnet command mode",					/* 0 */
	"character-at-a-time mode",				/* 1 */
	"character-at-a-time mode (local echo)",		/* 2 */
	"line-by-line mode (remote echo)",			/* 3 */
	"line-by-line mode",					/* 4 */
	"line-by-line mode (local echoing suppressed)",		/* 5 */
	"invalid mode?",					/* 6 */
	"invalid mode?",					/* 7 */
	"3270 mode",						/* 8 */
};

int	idmodedescript[] = {
	TEL_COM_MODE,					/* 0 */
	OP_CHAR_MODE,					/* 1 */
	OP_CHAR_LOCAL,					/* 2 */
	OP_LIN_REMOTE,					/* 3 */
	OP_LINE_MODE,					/* 4 */
	OP_LINE_SUPR,					/* 5 */
	OP_INV_MODE1,					/* 6 */
	OP_INV_MODE1,					/* 7 */
	OP_3270_MODE,					/* 8 */
};

getconnmode()
{
    static char newmode[9] = { 4, 5, 3, 3, 2, 2, 1, 1, 8 };
    int modeindex = 0;

    if (hisopts[TELOPT_ECHO]) {
	modeindex += 2;
    }
    if (hisopts[TELOPT_SGA]) {
	modeindex += 4;
    }
    if (dontlecho && (clocks.echotoggle > clocks.modenegotiated)) {
	modeindex += 1;
    }
    if (hisopts[TELOPT_EOR]) {
	modeindex = 8;
    }
    return newmode[modeindex];
}

setconnmode()
{
    mode(getconnmode());
}

/*****************************************************************************/
/*	restore_tty() 							     */
/*		This procedure is called before exiting to ensure that the   */
/*		tty is restored to its original settings.   		     */
/*****************************************************************************/
restore_tty()
{
	/* Restore tty to exactly the state it was in before telnet was
	   invoked. */
	memcpy(ottyb.c_cc, otc.c_cc, NCCS * sizeof(cc_t));
/*
	ioctl(fileno(stdin), TCSETA, &ottyb);
*/
	tcsetattr(fileno(stdin),TCSANOW,(struct termios *)&ottyb);
}

setcommandmode()
{
	mode(0);
}

/*     Already declared as global
char	sibuf[3 * SCREEN_SIZE], *sbp;
char	tibuf[3 * SCREEN_SIZE], *tbp;
int	scc, tcc; 
*/


/*
 * Select from tty and network...
 */
telnet()
{
	register int c;
	int tin = fileno(stdin);
	int on = 1;
	fd_set ibits, obits, xbits;
#ifdef _AIX
	int inescape = 0;
#endif /* _AIX */
	int nfd;

	tout = fileno(stdout);
	setconnmode();
	scc = 0;
	tcc = 0;
	FD_ZERO(&ibits);
	FD_ZERO(&obits);
	FD_ZERO(&xbits);

#if	defined(SO_OOBINLINE)
	setsockopt(net, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif	/* defined(SO_OOBINLINE) */
	if (telnetport) {
            if (!hisopts[TELOPT_ECHO]) {
                        willoption(TELOPT_ECHO, 0, 0);
            }
	    if (!hisopts[TELOPT_SGA]) {
			willoption(TELOPT_SGA, 0, 0);
	    }
	    if (!myopts[TELOPT_TTYPE]) {
			dooption(TELOPT_TTYPE, 0);
	    }
#ifdef _CSECURITY
	    if (!hisopts[TELOPT_SAK]) {
			willoption(TELOPT_SAK, 0, 0);
	    }
#endif /* _CSECURITY */
            if (!myopts[TELOPT_NAWS] ) {
                    dooption(TELOPT_NAWS, 0);
            }

	}
	if (tout > net)
		nfd = tout + 1;
	else
		nfd = net + 1;
	for (;;) {
		if (scc < 0 && tcc < 0) {
			break;
		}

		if (((globalmode < 4) || flushline) && (NETBYTES() || tcc>0)) {
			FD_SET(net, &obits);
		} else if (tcc == 0) {
			FD_SET(tin, &ibits);
		}
		if ((TTYBYTES() || scc > 0) && !(EMULATING_3270)) {
			FD_SET(tout, &obits);
		} else {
			FD_SET(net, &ibits);
		}
		/* If we're doing 3270 then we need to set net ibits even 
		 * though there may data in ttyobuf. This is because we may
		 * be (and probably are) waiting for EOR from the net before
		 * can flush ttyobuf.
		 */
		if (EMULATING_3270)
			FD_SET(net, &ibits);
		if (!SYNCHing) {
			FD_SET(net, &xbits);
		}
		if ((c = select(nfd, &ibits, &obits, &xbits, 0)) < 1) {
			if (c == -1) {
				/*
				 * we can get EINTR if we are in line mode,
				 * and the user does an escape (TSTP), or
				 * some other signal generator.
				 */
				if (errno == EINTR) {
					continue;
				}
			}
			sleep(5);
			continue;
		}

		/*
		 * Any urgent data?
		 */
		if (FD_ISSET(net, &xbits)) {
		    FD_CLR(net, &xbits);
		    SYNCHing = 1;
		    if (EMULATING_VT100) { }
		    else ttyflush();        /* flush already enqueued data */
		}

		/*
		 * Something to read from the network...
		 */
		if (FD_ISSET(net, &ibits)) {
			int canread;

			FD_CLR(net, &ibits);
			if (scc == 0) {
			    sbp = sibuf;
			}
			canread = sibuf + sizeof sibuf - sbp ;  
#if	!defined(SO_OOBINLINE)
			/*
			 * In 4.2 (and some early 4.3) systems, the
			 * OOB indication and data handling in the kernel
			 * is such that if two separate TCP Urgent requests
			 * come in, one byte of TCP data will be overlaid.
			 * This is fatal for Telnet, but we try to live
			 * with it.
			 *
			 * In addition, in 4.2 (and...), a special protocol
			 * is needed to pick up the TCP Urgent data in
			 * the correct sequence.
			 *
			 * What we do is:  if we think we are in urgent
			 * mode, we look to see if we are "at the mark".
			 * If we are, we do an OOB receive.  If we run
			 * this twice, we will do the OOB receive twice,
			 * but the second will fail, since the second
			 * time we were "at the mark", but there wasn't
			 * any data there (the kernel doesn't reset
			 * "at the mark" until we do a normal read).
			 * Once we've read the OOB data, we go ahead
			 * and do normal reads.
			 *
			 * There is also another problem, which is that
			 * since the OOB byte we read doesn't put us
			 * out of OOB state, and since that byte is most
			 * likely the TELNET DM (data mark), we would
			 * stay in the TELNET SYNCH (SYNCHing) state.
			 * So, clocks to the rescue.  If we've "just"
			 * received a DM, then we test for the
			 * presence of OOB data when the receive OOB
			 * fails (and AFTER we did the normal mode read
			 * to clear "at the mark").
			 */
		    if (SYNCHing) {
			int atmark;

			ioctl(net, SIOCATMARK, (char *)&atmark);
			if (atmark) {
			    c = recv(net, sibuf, canread, MSG_OOB);
			    if ((c == -1) && (errno == EINVAL)) {
				c = read(net, sibuf, canread);
				if (clocks.didnetreceive < clocks.gotDM) {
				    SYNCHing = stilloob(net);
				}
			    }
			} else {
			    c = read(net, sibuf, canread);
			}
		    } else {
			c = read(net, sibuf, canread);
		    }
		    settimer(didnetreceive);
#else	/* !defined(SO_OOBINLINE) */
		    c = read(net, sbp, canread);
#endif	/* !defined(SO_OOBINLINE) */
		    if (c < 0 && errno == EWOULDBLOCK) {
			c = 0;
		    } else if (c <= 0) {
			break;
		    }
		    if (netdata) {
			Dump('<', sbp, c);
		    }
		    scc += c;
		    FD_SET(tout, &obits);	/* try write */
		}

		/*
		 * Something to read from the tty...
		 */
		if (FD_ISSET(tin, &ibits)) {
			FD_CLR(tin, &ibits);
			if (tcc == 0) {
			    tbp = tibuf;	/* nothing left, reset */
			}
			c = read(tin, tbp, tibuf+sizeof tibuf - tbp);
			if (c < 0 && errno == EWOULDBLOCK) {
				c = 0;
			} else {
				/* EOF detection for line mode!!!! */
				if (c == 0 && globalmode >= 3) {
					/* must be an EOF... */
					*tbp = ntc.t_eofc;
					c = 1;
				}
				if (c <= 0) {
					tcc = c;
					break;
				}
			}
			tcc += c;
		        FD_SET(net, &obits);	/* try write */
		}

		while (tcc > 0) {
			register cc_t sc;
			static char *ptr;
			static int mbcount, multibyte = FALSE;

			if (NETROOM() < 2) {
				flushline = 1;
				break;
			}

        /* 'ptr' points to first byte of a char, this helps to keep track */
        /* of first byte in a char when multibyte locales are used        */
                        ptr = tbp;
                        c = *tbp++ & 0xff, sc = (c), tcc--;

                        if (mbcount=mblen(ptr, MB_CUR_MAX) > 1 || !isascii(c)) {

/*the following two if statments were changed from EMULATING to EMULATING_VT100
  to allow the full use of 8 bit character under tn3270
*/
                                if (EMULATING_VT100) {
                                        fprintf(stderr,
                                                MSGSTR(EM_ASCII_ONLY,
        "Only ascii characters supported in emulation mode.\r\n")); /*MSG*/
                                        if (mbcount > 1) {
                                                tbp = tbp + mbcount - 1;
                                                tcc = tcc - mbcount + 1;
                                                continue;
                                        }
                                }
                                if (mbcount > 1)
                                        multibyte = TRUE;
                        }

#ifdef _AIX
			if (inescape) {
				switch(currentterm) {
				case VT100:
					inescape = vt1(c);
					continue;
				default:	/* "impossible" */
					inescape = 0;
					break;
				}
			}
#endif /* _AIX */
			if (!multibyte) {	/* do only if single byte code */
			if (sc == escape) {
				command(0);
				tcc = 0;
				flushline = 1;
				break;
			} else if ((globalmode >= 4) && (sc == echoc)) {
				if (tcc > 0 && strip(*tbp) == echoc) {
					tbp++;
					tcc--;
				} else {
					dontlecho = !dontlecho;
					settimer(echotoggle);
					setconnmode();
					tcc = 0;
					flushline = 1;
					break;
				}
			}
			if (localchars) {
				if (sc == ntc.t_intrc) {
					intp();
					break;
				} else if (sc == ntc.t_quitc) {
					sendbrk();
					break;
				} else if (sc == nltc.t_flushc) {
					NET2ADD(IAC, AO);
					if (autoflush) {
					    doflush();
					}
					break;
				} else if (globalmode > 2) {
					;
				} else if (sc == nttyb.c_cc[VKILL]) {
					NET2ADD(IAC, EL);
					break;
				} else if (sc == nttyb.c_cc[VERASE]) {
					NET2ADD(IAC, EC);
					break;
				}
			}
#ifdef _CSECURITY
			if (c == telnet_sak) {
				if (telnet_sak != SAK_NOT_SET) {
					sendsak();
					break;
				}
			}
#endif /* _CSECURITY */
			}		/* end !multibyte block */
#ifdef _AIX
			if (EMULATING_3270)
				fs_input(c);
			else {
#endif /* _AIX */
			/*
			 * In binary mode we don't do any character
			 * interpretation (only doubling of embedded
			 * IACs), as per rfc-1123.
			 */
			if (myopts[TELOPT_BINARY]) {
				NETADD(c);
				if (c == IAC)
					NETADD(c);

			} else switch (c) {
#ifdef _AIX
			case ESC:
				switch(currentterm) {
					/* FALL THROUGH! */
				case VT100:
					/* don't stuff here (??) */
					inescape = 1;
					break;
				default:
					NETADD(ESC);
					break;
				}
				break;
#endif /* _AIX */
			case '\n':
				/*
				 * If we are in CRMOD mode (\r ==> \n)
				 * on our local machine, then probably
				 * a newline (unix) is CRLF (TELNET).
				 */
				if ((globalmode >= 3) || (lineterm)) {
					NETADD('\r');
				}
				NETADD('\n');
				flushline = 1;
				break;
			case '\r':
				if (lineterm) {
					NET2ADD('\r', '\n');
				}
				else {
					NET2ADD('\r', '\0');
				}
				flushline = 1;
				break;
			case IAC:
				NET2ADD(IAC, IAC);
				break;
			default:
				NETADD(c);
				break;
			}
#ifdef _AIX
			}
#endif /* _AIX */
		}
		if (((globalmode < 4) || flushline) &&
		    FD_ISSET(net, &obits) && (NETBYTES() > 0)) {
			netflush(net);
		}
		FD_CLR(net, &obits);
		if ((scc > 0) || (TTYBYTES())) {
			telrcv();
    			if (tbackp == tfrontp) {
				tbackp = tfrontp = ttyobuf;
    			}
		}
		if (FD_ISSET(tout, &obits) && (TTYBYTES() > 0)) {
			/* Don't flush 3270 data stream! */
                        if (!EMULATING)
                                ttyflush();
		}
		FD_CLR(tout, &obits);
	}
	setcommandmode();

}

/*
 * Telnet receiver states for fsm
 */
#define	TS_DATA		0
#define	TS_IAC		1
#define	TS_WILL		2
#define	TS_WONT		3
#define	TS_DO		4
#define	TS_DONT		5
#define	TS_CR		6
#define	TS_SB		7		/* sub-option collection */
#define	TS_SE		8		/* looking for sub-option end */
#ifdef _AIX
#define	TS_ESC		9		/* escape sequence processing */
#endif /* _AIX */


/*
 ************************************************************************
 *	telrcv - deal with what we got from the net			*
 ************************************************************************
 *
 * A few words about 3270 emulation :
 * 	Characters from the net are TTYADD'ed into ttyobuf until we get 
 * 	IAC EOR. At that point fs_command() is called to process the 3270
 *	data stream. The interesting part comes when what we read from the
 * 	net doesn't end with IAC EOR. This means that (hopefully) the IAC
 *	will come in some future read. Data in ttyobuf between tbackp and
 *	tfrontp is valid, unprocessed 3270 stuff that will be processed by
 *	fs_command() when the IAC EOR arrives. Data between ttyobuf[0] and
 *	tbackp is already processed. Last but not least ; if tfrontp ==
 * 	tbackp (ie no data), tfrontp and tbackp should be set to ttyobuf
 *	so that we don't overrun the buffer. (gsj)
 */	

#ifdef VT100_DEBUG
#define DEBUG_FILE      "./vt100.trace"
        FILE *debug_fd = NULL;
#endif /* VT100_DEBUG */

telrcv()
{
	register int c;
	static int state = TS_DATA;


#ifdef DUMPWRITE
cmdprintf(fp,"\nentering telrcv");
cmdprintf(fp,"\ntfrontp - ttyobuf = %x", tfrontp - ttyobuf);
cmdprintf(fp,"\nttyobuf = %x", ttyobuf);
cmdprintf(fp,"\ntbackp = %x", tbackp);
cmdprintf(fp,"\ninitial scc = %x", scc);
#endif
#ifdef VT100_DEBUG
      if (debug_fd == NULL) {
         if ((debug_fd = fopen( DEBUG_FILE, "a")) == NULL) {
           fprintf(stderr, "Can't open file %s\n", DEBUG_FILE);
           exit(1);
         }
      }
#endif /* VT100_DEBUG */
	while (scc > 0) {
		if (TTYROOM() <= 2)  
			ttyflush();
		c = *sbp++ & 0xff, scc--;
#ifdef VT100_DEBUG
                (void) putc(c, debug_fd);
#endif /* VT100_DEBUG */
		switch (state) {

		case TS_CR:
			state = TS_DATA;
			if (c == 0) {
				continue;
			} else if (!hisopts[TELOPT_ECHO] && (c == '\n')) {
				if (EMULATING_VT100)
					vtaddch('\n');
				else
				        TTYADD('\n');
			        continue;
			/*
			 * The 'crmod' hack (see following) is needed
			 * since we can't * set CRMOD on output only.
			 * Machines like MULTICS like to send \r without
			 * \n; since we must turn off CRMOD to get proper
			 * input, the mapping is done here (sigh).
			 */
			} else if (crmod) {
				if (EMULATING_VT100)
					vtaddch('\n');
			        else
				        TTYADD('\n');
		 	}
			/* Fall through to handle this character */

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				continue;
			}
			/* check for 3270 here and avoid the hacks below */
			if (EMULATING_3270) {
				TTYADD(c);
				continue;
			}
			if (c == '\r') {
			        state = TS_CR;
			        if (EMULATING_VT100)
				        vtaddch('\r');
			        else
					TTYADD('\r');
			        continue;
#ifdef _AIX
			} else if (c == ESC) {
				switch (currentterm) {
				case VT100: 
					state = TS_ESC;
					break;
				default:
					TTYADD(ESC);
					break;
				}
#endif /* _AIX */
			} else {
                                if (EMULATING_VT100)
                                        vtaddch(c);
                                else
                                        TTYADD(c);
			}
			continue;

		case TS_IAC:
			switch (c) {
			
			case WILL:
				state = TS_WILL;
				continue;

			case WONT:
				state = TS_WONT;
				continue;

			case DO:
				state = TS_DO;
				continue;

			case DONT:
				state = TS_DONT;
				continue;

			case DM:
				/*
				 * We may have missed an urgent notification,
				 * so make sure we flush whatever is in the
				 * buffer currently.
				 */
				SYNCHing = 1;
                                if (EMULATING_VT100) { }
                                  else ttyflush();
				SYNCHing = stilloob(net);
				settimer(gotDM);
				break;

			case NOP:
			case GA:
				break;

			case SB:
				SB_CLEAR();
				state = TS_SB;
				continue;
			
#ifdef _AIX
			case EOR:
				if (EMULATING_3270) {
					fs_command();
				}
				state = TS_DATA;
				continue;
#endif /* _AIX */

			/* added this case since IAC IAC is valid - gsj */
			case IAC:
				TTYADD(c);
				state = TS_DATA;
				continue;

			default:
				break;
			}
			state = TS_DATA;
			continue;

		case TS_WILL:
			printoption(">RCVD", will, c, !hisopts[c]);
			if (c == TELOPT_TM) {
				if (flushout) {
					flushout = 0;
				}
			} else if (!hisopts[c]) {
				willoption(c, 1, 1);
			}
			state = TS_DATA;
			continue;

		case TS_WONT:
			printoption(">RCVD", wont, c, hisopts[c]);
			if (c == TELOPT_TM) {
				if (flushout) {
					flushout = 0;
				}
			} else if (hisopts[c]) {
				wontoption(c, 1);
			}
			state = TS_DATA;
			continue;

		case TS_DO:
			printoption(">RCVD", doopt, c, !myopts[c]);
			if (!myopts[c])
				dooption(c);
			else if (donaws && c == TELOPT_NAWS) 
				sendwinsize();
			state = TS_DATA;
			continue;

		case TS_DONT:
			printoption(">RCVD", dont, c, myopts[c]);
			if (myopts[c]) {
				/*
				 * if we are in 3270 mode and receive a DONT
				 * BINARY, we must get out of 3270 mode back
				 * to NVT
				 */
				if (c == TELOPT_BINARY && EMULATING_3270)
					doemulate(NATIVE);
				myopts[c] = 0;
				if (c == TELOPT_NAWS) donaws = 0;
				sprintf(nfrontp, "%s%c", wont, c);
				nfrontp += sizeof (wont);
				flushline = 1;
				setconnmode();	/* set new tty mode (maybe) */
				printoption(">SENT", wont, c);
			}
			state = TS_DATA;
			continue;
#ifdef _AIX
		case TS_ESC:
			{
			int result;

			switch(currentterm) {
			case VT100:
				result = vt2(c);
				break;
			default:
				result = -1;
				break;
			}
			switch(result) {
			case -1:	/* abort */
                                if (EMULATING_VT100) {
                                        vtaddch(ESC); /* Kent-added to send out the original ESC char */
                                        vtaddch(c);
                                }
                                else TTYADD(c);
				/* FALL THROUGH */
			case 0:		/* normal end */
				state = TS_DATA;
				break;
			default:	/* still handling */
				break;
			}
			break;
			}
#endif /* _AIX */
		case TS_SB:
			if (c == IAC) {
				state = TS_SE;
			} else {
				SB_ACCUM(c);
			}
			continue;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					SB_ACCUM(IAC);
				}
				SB_ACCUM(c);
				state = TS_SB;
			} else {
				SB_TERM();
				suboption();	/* handle sub-option */
				state = TS_DATA;
			}
		}
	}

	if (EMULATING_VT100)  /* must refresh screen */
		refresh();

#ifdef DUMPWRITE
cmdprintf(fp,"\nending scc = %x", scc);
cmdprintf(fp,"\ntfrontp - ttyobuf = %x", tfrontp - ttyobuf);
cmdprintf(fp,"\nttyobuf = %x", ttyobuf);
cmdprintf(fp,"\ntbackp = %x", tbackp);
cmdprintf(fp,"\nleaving telrcv\n");
#endif
}


getwinsize(rows, cols)
int *rows, *cols;
{
	struct winsize winsize;
	int ioctl_ret;
	
	if (EMULATING_VT100) {  /* use vt100 window size */
	    *rows = Lines;
	    *cols = Cols;
	} else {
#ifdef TIOCGWINSZ
	    ioctl_ret = ioctl(0, TIOCGWINSZ, &winsize);
#else
	    ioctl_ret = ioctl(0, TIOCGSIZE, &winsize);
#endif
	    if (ioctl_ret == -1) {
        	perror(MSGSTR(P_GETWINSIZE, "getwinsize:ioctl")); /*MSG*/
		*rows = 24;
		*cols = 80;
	    } else {
	    	*rows = htons(winsize.ws_row);
	    	*cols = htons(winsize.ws_col);
	    }
	}
	return(0);
}
	
sendwinsize()
{
	int width, height;
	char buf[50];

	if (!getwinsize(&height, &width))
	{
		/* Subnegotiation BEGIN */
		NET2ADD(IAC, SB);
		
		/* Option being subnegotiated */
		NETADD(TELOPT_NAWS);
		
		/* high-order byte WIDTH */
		/* Double the byte iff it is the same as IAC */
		if ((char)(width>>8) == IAC)
			NET2ADD(IAC, IAC)
		else
			NETADD((char)(width>>8));
		
		/* low-order byte WIDTH */
		/* Double the byte iff it is the same as IAC */
		if ((char)width == IAC)
			NET2ADD(IAC, IAC)
		else
			NETADD((char)width);

		/* high-order byte HEIGHT */
		/* Double the byte iff it is the same as IAC */
		if ((char)(height>>8) == IAC)
			NET2ADD(IAC, IAC)
		else
			NETADD((char)(height>>8));
		
		/* low-order byte HEIGHT */
		/* Double the byte iff it is the same as IAC */
		if ((char)height == IAC)
			NET2ADD(IAC, IAC)
		else
			NETADD((char)height);
		/* Subnegotiation END */
		NET2ADD(IAC, SE);
		sprintf(buf, "Width %d, Height %d", width, height);
		printsuboption(">SENT", TELOPT_NAWS, buf);
		return(0);
	}
	else
		return(-1);
}

willoption(option, reply, now)
	int option, reply, now;
{
	char *fmt;

	switch (option) {

	case TELOPT_ECHO:
	case TELOPT_SGA:
		settimer(modenegotiated);
		if(now)
		    hisopts[option] = 1;
		fmt = doopt;
		setconnmode();		/* possibly set new tty mode */
		break;

#ifdef _CSECURITY
	case TELOPT_SAK:
		if(now)
		    hisopts[option] = 1;
		fmt = doopt;
		break;
#endif /* _CSECURITY */

	case TELOPT_TM:
		return;			/* Never reply to TM will's/wont's */

#ifdef _AIX
	case TELOPT_BINARY:
		if(now)
		    hisopts[option] = 1;
		fmt = doopt;
		setconnmode(); 		/* possibly set new tty mode */
		break;
#endif /* _AIX */
#ifdef _AIX
	case TELOPT_EOR:
		if (EMULATING_3270) {
			if(now)
			    hisopts[option] = 1;
			fmt = doopt;
			break;
		}
		/* FALL THROUGH */
#endif /* _AIX */

	default:
		fmt = dont;
		break;
	}
	sprintf(nfrontp, "%s%c", fmt, option);
	nfrontp += sizeof (dont);
	if (reply)
		printoption(">SENT", fmt, option);
	else
		printoption("<SENT", fmt, option);
}

wontoption(option, reply)
	int option, reply;
{
	char *fmt;

	switch (option) {

	case TELOPT_ECHO:
	case TELOPT_SGA:
		settimer(modenegotiated);
		hisopts[option] = 0;
		fmt = dont;
		setconnmode();			/* Set new tty mode */
		break;

#ifdef _CSECURITY
	case TELOPT_SAK:
		hisopts[option] = 0;
		fmt = dont;
		break;
#endif /* _CSECURITY */

	case TELOPT_TM:
		return;		/* Never reply to TM will's/wont's */

	default:
		fmt = dont;
	}
	sprintf(nfrontp, "%s%c", fmt, option);
	nfrontp += sizeof (doopt);
	if (reply)
		printoption(">SENT", fmt, option);
	else
		printoption("<SENT", fmt, option);
}

dooption(option)
	int option;
{
	char *fmt;
	char action;
	int sendnawsnow = 0;

	switch (option) {

        case TELOPT_NAWS:
		if (donaws) sendnawsnow = 1;
                fmt = will;
                myopts[option] = 1;
                donaws=1;
                break;

	case TELOPT_TM:
		fmt = will;
		action = WILL;
		break;

	case TELOPT_TTYPE:		/* terminal type option */
	case TELOPT_SGA:		/* no big deal */
		fmt = will;
		action = WILL;
		myopts[option] = 1;
		break;
#ifdef _AIX
	case TELOPT_BINARY:
		myopts[option] = 1;
		fmt = will;
		action = WILL;
		break;
#endif /* _AIX */
#ifdef _AIX
	case TELOPT_EOR:
		if (EMULATING_3270) {
			myopts[option] = 1;
			fmt = will;
			action = WILL;
			break;
		}
		/* FALL THROUGH */
#endif /* _AIX */

	case TELOPT_ECHO:		/* We're never going to echo... */
	default:
		fmt = wont;
		action = WONT;
		break;
	}
	sprintf(nfrontp, "%s%c", fmt, option);
	nfrontp += sizeof (doopt); 
	printoption(">SENT", fmt, option);
	if (sendnawsnow)
		sendwinsize();
}

stopterminalnegotiation()
{
	sprintf(nfrontp, "%s%c", wont, TELOPT_TTYPE);
	nfrontp += sizeof (doopt);
	printoption(">SENT", wont, TELOPT_TTYPE);
	myopts[TELOPT_TTYPE] = 0;
}

/*
 * suboption()
 *
 *	Look at the sub-option buffer, and try to be helpful to the other
 * side.
 *
 *	Currently we recognize:
 *
 *		Terminal type, send request.
 */

suboption()
{

    switch (subbuffer[0]&0xff) {
    case TELOPT_TTYPE:
	if ((subbuffer[1]&0xff) != TELQUAL_SEND) {
	    ;
	} else {
	    char *name;
	    char namebuf[41];
	    char *getenv();
	    static int currenttermtotry = 0;
	    int len, i;
#define	MAX_RETRYS 3

#ifdef _AIX
	    printsuboption(">RCVD", TELOPT_TTYPE, "SEND");
	    /* In the AIX version, we try all the different terminals we
	     * can emulate, in both termcap and RFC 1010 forms.  We first
	     * do the termcap form for compatibility with the BSD server.
	     * If ALL term types are refused, we send the last term twice
	     * as per rfc-1091, then revert to the first term on the
	     * next query.
	     */
	    do {
		++formofname;
		if (useremulate) {	/* if emulating, try this one only */
		    if (formofname == MAXFORMS)
			    if (sent_last)  /* reset to first form */
				    formofname = sent_last = 0;
			    else {  /* send last form twice */
				    --formofname;
				    sent_last = 1;
			    }
		} else {
		    if (formofname == MAXFORMS) {  /* bump to next term */
			    /* scuzzy */
                            if (negotiatedterm)
				    emulateoff();
			    i = 0;  /* can't emulate flag */
                            do {
				    /* check for last term */
				    if (negotiatedterm == MAXTERMINALS-1) {
					/*
					 * if we can't emulate the last term,
					 * then we must re-send the previous
					 * one (since it is the last one we
					 * can emulate).
					 */
					if (i) {
					    negotiatedterm = lastterm;
					    formofname = lastform;
					    sent_last = 1;
					/*
					 * otherwise we make sure we send the
					 * last form of the last term in the
					 * list twice.
					 */
					} else if (sent_last) {
					    negotiatedterm = NATIVE;
					    sent_last = formofname = 0;
					} else {
					    --formofname;
					    sent_last = 1;
					}
				    } else {
					negotiatedterm++;
					formofname = 0;
				    }
			    } while (i = cantemulate(negotiatedterm));
			    doemulate(negotiatedterm);
		    }
		}
		name = terminalstotry[negotiatedterm].name[formofname];
	    } while (!name);

	    lastterm = negotiatedterm;  /* save the last term and form sent */
	    lastform = formofname;
#else
	    name = getenv("TERM");
#endif /* _AIX */
	    if ((name == 0) || ((len = strlen(name)) > 40)) {
		name = "UNKNOWN";
	    }
	    if ((len + 4+2) < NETROOM()) {
		strcpy(namebuf, name);
#ifdef _AIX
		/* this is a bug in BSD, really ... */
#else
		upcase(namebuf);
#endif /* _AIX */
		sprintf(nfrontp, "%c%c%c%c%s%c%c", IAC, SB, TELOPT_TTYPE,
				    TELQUAL_IS, namebuf, IAC, SE);
		nfrontp += 4+strlen(namebuf)+2;
#ifdef _AIX
		printsuboption(">SENT", TELOPT_TTYPE, namebuf);
#endif /* _AIX */
	    }
	}

    default:
	break;
    }

}

/*
 *	The following are data structures and routines for
 *	the "send" command.
 *
 */
 
struct sendlist {
    char	*name;		/* How user refers to it (case independent) */
    int		what;		/* Character to be sent (<0 ==> special) */
    int		helpid;		/* Help messagea catalog id */
    char	*help;		/* Help information (0 ==> no help) */
    int		(*routine)();	/* Routine to perform (for special ops) */
};

/*ARGSUSED*/
dosynch(s)
struct sendlist *s;
{
    netclear();			/* clear the path to the network */
    NET2ADD(IAC, DM);
    neturg = NETLOC()-1;	/* Some systems are off by one XXX */
}

doflush()
{
    NET2ADD(IAC, DO);
    NETADD(TELOPT_TM);
    flushline = 1;
    flushout = 1;
    if (EMULATING_VT100) { }
    else ttyflush();
    /* do printoption AFTER flush, otherwise the output gets tossed... */
    printoption("<SENT", doopt, TELOPT_TM);
}

intp()
{
    if (connected) {
	NET2ADD(IAC, IP);
    }
    if (autoflush) {
	doflush();
    }
    if (autosynch) {
	dosynch();
    }
}

sendbrk()
{
    if (connected) {
	NET2ADD(IAC, BREAK);
    }
    if (autoflush) {
	doflush();
    }
    if (autosynch) {
	dosynch();
    }
}

#ifdef _CSECURITY
sendsak()
{
    /* assurance */
    if (!hisopts[TELOPT_SAK]) {
	cmdprintf(MSGSTR(CS_REM_NO_SAK, "Remote side does not support SAK\n")); /*MSG*/
	return;
    }
    NET2ADD(IAC, SAK);
}
#endif /* _CSECURITY */

#define	SENDQUESTION	-1
#define	SENDESCAPE	-3

struct sendlist Sendlist[] = {
    { "ao",	AO,	SEND_ABORT,	"Send Telnet Abort output" },
    { "ayt",	AYT,	SEND_AYT,	"Send Telnet 'Are You There'" },
    { "brk",	BREAK,	SEND_BREAK,	"Send Telnet Break" },
    { "ec",	EC,	SEND_ERASE,	"Send Telnet Erase Character" },
    { "el",	EL,	SEND_E_LINE,	"Send Telnet Erase Line" },
    { "escape", SENDESCAPE,	SEND_ESCAPE,	"Send current escape character" },
    { "ga",	GA,	SEND_GO_AHEAD,	"Send Telnet 'Go Ahead' sequence" },
    { "ip",	IP,	SEND_INTR,	"Send Telnet Interrupt Process" },
    { "nop",	NOP,	SEND_NOP,	"Send Telnet 'No operation'" },
#ifdef _CSECURITY
    { "sak",	SAK,	SEND_SAK,	"Send Telnet Secure Attention Key", sendsak },
#endif /* _CSECURITY */
    { "synch",	SYNCH,	SEND_SYNCH,	"Perform Telnet 'Synch operation'", dosynch },
    { "?",	SENDQUESTION,	SEND_DISPLAY,	"Display send options" },
    { 0 }
};

struct sendlist Sendlist2[] = {		/* some synonyms */
	{ "break",	BREAK,	SEND_BREAK,	0 },
	{ "intp",	IP,	SEND_INTR,	0 },
	{ "interrupt",	IP,	SEND_INTR,	0 },
	{ "intr",	IP,	SEND_INTR,	0 },
	{ "help",	SENDQUESTION,	SEND_DISPLAY,	0 },
	{ 0 }
};

char **
getnextsend(name)
char *name;
{
    struct sendlist *c = (struct sendlist *) name;

    return (char **) (c+1);
}

struct sendlist *
getsend(name)
char *name;
{
    struct sendlist *sl;

    if (sl = (struct sendlist *)
				genget(name, (char **) Sendlist, getnextsend)) {
	return sl;
    } else {
	return (struct sendlist *)
				genget(name, (char **) Sendlist2, getnextsend);
    }
}

sendcmd(argc, argv)
int	argc;
char	**argv;
{
    int what;		/* what we are sending this time */
    int count;		/* how many bytes we are going to need to send */
    int hadsynch;	/* are we going to process a "synch"? */
    int i;
    int question = 0;	/* was at least one argument a question */
    struct sendlist *s;	/* pointer to current command */


    if (argc < 2) {
                /* Added to support vt100 emulation in command mode */
		cmdprintf(MSGSTR(INV_NO_ARGS, 
			"need at least one argument for 'send' command\n")); 
		cmdprintf(MSGSTR(HELP_SEND, "'send ?' for help\n")); /*MSG*/
                return 0;
    }
    /*
     * First, validate all the send arguments.
     * In addition, we see how much space we are going to need, and
     * whether or not we will be doing a "SYNCH" operation (which
     * flushes the network queue).
     */
    count = 0;
    hadsynch = 0;
    for (i = 1; i < argc; i++) {
	s = getsend(argv[i]);
	if (s == 0) {
		cmdprintf(MSGSTR(UNK_SEND_ARG, 
			"Unknown send argument '%s'\n'send ?' for help.\n"), 
                        argv[i]);
	    	return 0;
	} else if (s == Ambiguous(struct sendlist *)) {
		cmdprintf(MSGSTR(AMB_SEND_ARG, 
			"Ambiguous send argument '%s'\n'send ?' for help.\n"), 
			argv[i]);
		return 0;
	}
	switch (s->what) {
	case SENDQUESTION:
	    break;
	case SENDESCAPE:
	    count += 1;
	    break;
	case SYNCH:
	    hadsynch = 1;
	    count += 2;
	    break;
	default:
	    count += 2;
	    break;
	}
    }
    /* Now, do we have enough room? */
    if (NETROOM() < count) {
	char	*t;

	if ((t = MSGSTR(NO_ROOM_MSG, NULL)) == NULL) {
		cmdprintf("There is not enough room in the buffer TO the network\n");
		cmdprintf("to process your request.  Nothing will be done.\n");
		cmdprintf("('send synch' will throw away most data in the network\n");
		cmdprintf("buffer, if this might help.)\n");
	}
	else
		cmdprintf(t);

	return 0;
    }
    /* OK, they are all OK, now go through again and actually send */
    for (i = 1; i < argc; i++) {
	if (!(s = getsend(argv[i]))) {
	    fprintf(stderr, MSGSTR(SND_ERR_ARG_GONE,
		    "Telnet 'send' error - argument disappeared!\n")); /*MSG*/
	    quit();
	    /*NOTREACHED*/
	}
	if (s->routine) {
	    (*s->routine)(s);
	} else {
	    switch (what = s->what) {
	    case SYNCH:
		dosynch();
		break;
	    case SENDQUESTION:
		for (s = Sendlist; s->name; s++) {
		    if (s->help) {
			cmdprintf(MSGSTR(s->helpid, "%s\t%s\n"), s->name, 
				s->help);
		    }
		}
		question = 1;
		break;
	    case SENDESCAPE:
		NETADD(escape);
		break;
	    default:
		NET2ADD(IAC, what);
		break;
	    }
	}
    }
    return !question;
}

/*
 * The following are the routines and data structures referred
 * to by the arguments to the "toggle" command.
 */

lclchars()
{
    donelclchars = 1;
    return 1;
}

togdebug()
{
#ifndef	NOT43
    if (net > 0 &&
	setsockopt(net, SOL_SOCKET, SO_DEBUG, (char *)&debug, sizeof(debug))
									< 0) {
	    perror(MSGSTR(P_SETSOCK_DBG, "setsockopt (SO_DEBUG)")); /*MSG*/
    }
#else	NOT43
    if (debug) {
	if (net > 0 && setsockopt(net, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
	    perror(MSGSTR(P_SETSOCK_DBG, "setsockopt (SO_DEBUG)")); /*MSG*/
    } else
	cmdprintf(MSGSTR(SOCKET_DEBUG, "Cannot turn off socket debugging\n")); /*MSG*/
#endif	/* NOT43 */
    return 1;
}



int togglehelp();

struct togglelist {
    char	*name;		/* name of toggle */
    int		helpid;		/* help message catalog id */
    char	*help;		/* help message */
    int		(*handler)();	/* routine to do actual setting */
    int		dohelp;		/* should we display help information */
    int		*variable;
    int		actionid[2];	/* Catalog message ids */
    int		wactionid[2];	/* Catalog message ids will/won't */
    char	*actionexplanation;
};

struct togglelist Togglelist[] = {
    {   "autoflush",
	TOG_FLUSH,
	"toggle flushing of output when sending interrupt characters",
	0,
	1,
	&autoflush,
	FLUSH_ACT_WILL, FLUSH_ACT_WONT,
	FLUSH_ACT_wILL, FLUSH_ACT_wONT,
			"flush output when sending interrupt characters" },
    { "autosynch",
	TOG_INTR_URG,
	"toggle automatic sending of interrupt characters in urgent mode",
	0,
	1,
	&autosynch,
	FL_WILL_INT_URG, FL_WONT_INT_URG,
	FL_wILL_INT_URG, FL_wONT_INT_URG,
			"send interrupt characters in urgent mode" },
    /* CHANGED - straightened out display: */
    { "crmod   ",
	TOG_MAP_CR,
	"toggle mapping of received carriage returns",
	0,
	1,
	&crmod,
	ACT_WILL_MAP_CR, ACT_WONT_MAP_CR,
	ACT_wILL_MAP_CR, ACT_wONT_MAP_CR,
			"map carriage return on output" }, 
    { "lineterm",
        TOG_DEF_LT,
        "toggle default end-of-line terminator",
        0,
        1,
        &lineterm,      
        ACT_WILL_CRLF_LT, ACT_WONT_CRLF_LT,
        ACT_wILL_CRLF_LT, ACT_wONT_CRLF_LT,
                "toggle default end of line terminator" },
    { "localchars",
	TOG_REC_CNTL,
	"toggle local recognition of certain control characters",
	lclchars,
	1,
	&localchars,
	ACT_WILL_REC_CNTL, ACT_WONT_REC_CNTL,
	ACT_wILL_REC_CNTL, ACT_wONT_REC_CNTL,
	"recognize certain control characters" },
    { " ",
	BLANKLINE,
		"", 0, 1 },		/* empty line */
    { "debug",
	TOG_SOCK_DBG,
	"(debugging) toggle debugging",
	togdebug,
	1,
	&debug,
	ACT_WILL_SOCK_DBG, ACT_WONT_SOCK_DBG,
	ACT_wILL_SOCK_DBG, ACT_wONT_SOCK_DBG,
	"turn on socket level debugging" },
    { "netdata",
	TOG_PR_HEX,
	"(debugging) toggle printing of hexadecimal network data",
	0,
	1,
	&netdata,
	ACT_WILL_PR_HEX, ACT_WONT_PR_HEX,
	ACT_wILL_PR_HEX, ACT_wONT_PR_HEX,
	"print hexadecimal representation of network traffic"}, 
    { "options",
	TOG_VW_OPTS,
	"(debugging) toggle viewing of options processing",
	0,
	1,
	&showoptions,
	ACT_WILL_SHOW_OPT, ACT_WONT_SHOW_OPT,
	ACT_wILL_SHOW_OPT, ACT_wONT_SHOW_OPT,
	"show option processing" },
    { " ",
	BLANKLINE,
		"", 0, 1 },		/* empty line */
    { "?",
	DSP_HELP,
	"display help information",
	togglehelp,
	1 },
    { "help",
	DSP_HELP,
	"display help information",
	togglehelp,
	0 },
    { 0 }
};

togglehelp()
{
    struct togglelist *c;

    for (c = Togglelist; c->name; c++) {
	if (c->dohelp) {
		cmdprintf(MSGSTR(c->helpid, "%s\t%s\n"), c->name, c->help); 
	}
    }
    return 0;
}

char **
getnexttoggle(name)
char *name;
{
    struct togglelist *c = (struct togglelist *) name;

    return (char **) (c+1);
}

struct togglelist *
gettoggle(name)
char *name;
{
    return (struct togglelist *)
			genget(name, (char **) Togglelist, getnexttoggle);
}

toggle(argc, argv)
int	argc;
char	*argv[];
{
    int retval = 1;
    char *name;
    struct togglelist *c;

    if (argc < 2) {
	fprintf(stderr, MSGSTR(NO_TOG_ARG,
	    "Need an argument to 'toggle' command.  'toggle ?' for help.\n")); /*MSG*/
	return 0;
    }
    argc--;
    argv++;
    while (argc--) {
	name = *argv++;
	c = gettoggle(name);
	if (c == Ambiguous(struct togglelist *)) {
	    fprintf(stderr, MSGSTR(AMB_TOG_ARG,
		    "'%s': ambiguous argument ('toggle ?' for help).\n"), /*MSG*/
	            name);
	    return 0;
	} else if (c == 0) {
	    fprintf(stderr, MSGSTR(UNK_TOG_ARG,
		    "'%s': unknown argument ('toggle ?' for help).\n"), /*MSG*/
					name);
	    return 0;
	} else {
	    if (c->variable) {
		*c->variable = !*c->variable;		/* invert it */
		/* MSG - The message strings in the catalog do not have 			   any arguments.  Therefore, the two arguments in the
		   cmdprintf statement are not used.  If the message is
		   not found in the catalog then the default format
		   string is used which does have two arguments. */
		cmdprintf(MSGSTR(c->actionid[*c->variable? 0 : 1], 
			"%s %s.\n"), /*MSG*/
			*c->variable? "Will" : "Won't", 
			c->actionexplanation);
	    }
	    if (c->handler) {
		retval &= (*c->handler)(c);
	    }
	}
    }
    return retval;
}

/*
 * The following perform the "set" command.
 */

struct setlist {
    char *name;				/* name */
    int	 helpid;			/* help message catalog id */
    char *help;				/* help information */
    cc_t *charp;			/* where it is located at */
};

struct setlist Setlist[] = {
    { "echo",	TOG_LECHO_CHAR, 
		"character to toggle local echoing on/off", &echoc },
    { "escape",	TN_ESC_CHAR,
		"character to escape back to telnet command mode", &escape },
    { " ",	BLANKLINE,
		0, (cc_t *) 0 },
    { " ",	SET_LOCALCHARS,
	"The following need 'localchars' to be toggled true", (cc_t *) 0 },
    { "erase",	ERASE_CHAR,
		"character to cause an Erase Character", &nttyb.c_cc[VERASE] },
    { "flushoutput",	ABRT_OUTPUT_CHAR,
		"character to cause an Abort Output", &nltc.t_flushc },
    { "interrupt",	INTR_CHAR,
		"character to cause an Interrupt Process", &ntc.t_intrc },
    { "kill",	ERASE_LINE_CHAR,
		"character to cause an Erase Line", &nttyb.c_cc[VKILL] },
    { "quit",	BREAK_CHAR,
		"character to cause a Break", &ntc.t_quitc },
    { "eof",	EOF_CHAR,
		"character to cause an EOF ", &ntc.t_eofc },
#ifdef _CSECURITY
    { "sak",	SAK_SEND_CHAR,
		"character to cause remote host SAK sequence to be sent",
			&telnet_sak },
#endif /* _CSECURITY */
    { (char *)0 }
};

char **
getnextset(name)
char *name;
{
    struct setlist *c = (struct setlist *)name;

    return (char **) (c+1);
}

struct setlist *
getset(name)
char *name;
{
    return (struct setlist *) genget(name, (char **) Setlist, getnextset);
}

setcmd(argc, argv)
int	argc;
char	*argv[];
{
    cc_t value;
    struct setlist *ct;

    /* XXX back we go... sigh */
    if (argc != 3) {
	if ((argc == 2) &&
		    ((!strcmp(argv[1], "?")) || (!strcmp(argv[1], "help")))) {
	    for (ct = Setlist; ct->name; ct++) {
		/* MSG - The help string in the catalog will only have 
			 one %s which will be the name.  If the catgets
			 succeeds then only the name is used, else if 
			 the catgets fails then the default format will
			 use the help string from the setlist structure
		*/
		cmdprintf(MSGSTR(ct->helpid,"%s\t%s\n"), ct->name, ct->help);
	    }
		cmdprintf(MSGSTR(DSP_HELP_INFO, "?\tdisplay help information\n")); /*MSG*/
	} else {
		cmdprintf(MSGSTR(DISP_HELP_SET, 
			"Format is 'set Name Value'\n'set ?' for help.\n")); 
	}
	return 0;
    }

    ct = getset(argv[1]);
    if (ct == 0) {
	fprintf(stderr, MSGSTR(UNK_SET_ARG,
		"'%s': unknown argument ('set ?' for help).\n"), /*MSG*/
			argv[1]);
	return 0;
    } else if (ct == Ambiguous(struct setlist *)) {
	fprintf(stderr, MSGSTR(AMB_SET_ARG,
		"'%s': ambiguous argument ('set ?' for help).\n"), /*MSG*/
			argv[1]);
	return 0;
    } else {
	if (strcmp(offstring, argv[2]) == 0 || strcmp("off", argv[2]) == 0) {
		value = -1;
	}
	else {
		if (mblen(&argv[2][0], MB_CUR_MAX) > 1) {
			cmdprintf(MSGSTR(SET_ASCII_2, "Cannot set %s to %s (%2x%2x).\nOnly ascii characters allowed.\n"), /*MSG*/
			ct->name, argv[2], argv[2][0], argv[2][1]);
			return 0;
		}
		else if (!isascii(argv[2][0])) {
			cmdprintf(MSGSTR(SET_ASCII_1, "Cannot set %s to %s (%2x).\nOnly ascii characters allowed.\n"), /*MSG*/
			ct->name, argv[2], argv[2][0]);
			return 0;
		}
		value = special(argv[2]);
	}

	*(ct->charp) = value;
	cmdprintf(MSGSTR(SET_CHAR_IS, "%s character is '%s'.\n"),
		ct->name, control(*(ct->charp))); /*MSG*/
    }
    return 1;
}

/*
 * The following are the data structures and routines for the
 * 'mode' command.
 */

dolinemode()
{
    if (EMULATING_VT100) {
	cmdprintf(MSGSTR(VT100_LINE, 
		"Line mode NOT supported while emulating VT-100.\n"));
    } else {
	    if (hisopts[TELOPT_SGA]) {
		wontoption(TELOPT_SGA, 0);
	    }
	    if (hisopts[TELOPT_ECHO]) {
		wontoption(TELOPT_ECHO, 0);
	    }
	    linemode = TRUE;
    }
}

docharmode()
{
    if (!hisopts[TELOPT_SGA]) {
	willoption(TELOPT_SGA, 0, 1);
    }
    if (!hisopts[TELOPT_ECHO]) {
	willoption(TELOPT_ECHO, 0, 1);
    }
    linemode = FALSE;
}

struct cmd Modelist[] = {
    { "character",	CHAR_ATA_TIME,
		"character-at-a-time mode",	docharmode, 1, 1 },
    { "line",	LINE_BY_LINE,
		"line-by-line mode",		dolinemode, 1, 1 },
    { 0 },
};

char **
getnextmode(name)
char *name;
{
    struct cmd *c = (struct cmd *) name;

    return (char **) (c+1);
}

struct cmd *
getmodecmd(name)
char *name;
{
    return (struct cmd *) genget(name, (char **) Modelist, getnextmode);
}

modecmd(argc, argv)
int	argc;
char	*argv[];
{
    struct cmd *mt;

    if ((argc != 2) || !strcmp(argv[1], "?") || !strcmp(argv[1], "help")) {
	cmdprintf(MSGSTR(MODE_FMT, "format is:  'mode Mode', where 'Mode' is one of:\n\n"));/*MSG*/
	for (mt = Modelist; mt->name; mt++) {
		cmdprintf(MSGSTR(mt->helpid, "%s\t%s\n"), mt->name, mt->help);/*MSG*/
	}
	return 0;
    }
    mt = getmodecmd(argv[1]);
    if (mt == 0) {
	fprintf(stderr, MSGSTR(UNK_MODE,
		"Unknown mode '%s' ('mode ?' for help).\n"), argv[1]); /*MSG*/
	return 0;
    } else if (mt == Ambiguous(struct cmd *)) {
	fprintf(stderr, MSGSTR(AMB_MODE,
		"Ambiguous mode '%s' ('mode ?' for help).\n"), argv[1]); /*MSG*/
	return 0;
    } else {
	(*mt->handler)();
    }
    return 1;
}


#ifdef _AIX

setupterminals()
{
	register int i;
	char *getenv();
	char local[41];
	static char kludge[80];
	extern struct cmd Emulatelist[];

	terminalname = getenv("TERM");
	terminalstotry[NATIVE].name[0] = terminalname;
	if (terminalstotry[IBM3270].name[1] = fs_select_term()) {
		sprintf(kludge, MSGSTR(HELP_3270_FMT, "%s terminal"), /*MSG*/
			terminalstotry[IBM3270].name[1]);
		Emulatelist[IBM3270].help = kludge;
	}
	for (i=0; i < MAXTERMINALS; ++i) {
		if (!terminalstotry[i].name[1])
			(void) terminfototelnet(terminalstotry[i].name[0],
				&terminalstotry[i].name[1]);
		/* make sure we don't have a duplicate */
		strcpy(local, terminalstotry[i].name[0]);
		upcase(local);
		if (!strcmp(local, terminalstotry[i].name[1]))
			terminalstotry[i].name[1] = NULL;
	}
}

emulateoff()
{
	/* if we're in command mode, then we got here via the emulate 
		command & we must to the tty_litout(1) to straighten out
		the tty (see mode()). */
	if (COMMAND_MODE) 
		tty_litout(1);
	if (EMULATING_3270)
		fs_end();
	if (EMULATING_VT100) 
		vtclose();		/* turn off keyboard remap */
}

donone(fromuser)
int fromuser;
{
	if (connected)
		emulateoff();
	useremulate = fromuser;
	currentterm = negotiatedterm = NATIVE;
}

dovt100(fromuser)
int fromuser;
{
	useremulate = fromuser;
	currentterm = negotiatedterm = VT100;
	if (connected) {
		vtinit();
		/*
		 * If any data was not written to the screen when we get
		 * here, then it was received before we changed to vt100
		 * emulation. You cannot be sure the data can be displayed
		 * properly on a vt100, so just throw it away.
		 */
		if ( TTYBYTES() ) {
			tbackp = tfrontp = ttyobuf;
			refresh();
		}
		if (linemode)   {
			cmdprintf(MSGSTR(VT100_CHAR, 
				"Selecting character mode.\n"));
			docharmode();
		}
	}
}

do3270(fromuser)
int fromuser;
{
	useremulate = fromuser;
	currentterm = negotiatedterm = IBM3270;
	if (connected) {
		fs_init();
	}
}

struct cmd Emulatelist[] = {
    { "none",	EMULATE_OFF,
		"turn off emulation",	donone, 1, 0 },
    { "vt100",	EMULATE_VT100,
		"DEC VT100 terminal",	dovt100, 1, 0 },
    { "3270",	EMULATE_3270,
		"IBM 3278 or 3277 terminal",	do3270, 1, 0 },
    { 0 },
};

doemulate(terminal)
int terminal;
{
	(*Emulatelist[terminal].handler)(0);
}

cantemulate(terminal)
int terminal;
{
	if (terminal == IBM3270)
		return terminalstotry[IBM3270].name[1] == NULL;
	else return 0;
}

struct cmd *
getemulatecmd(name)
char *name;
{
    return (struct cmd *) genget(name, (char **) Emulatelist, getnextmode);
}

emulate(argc, argv)
int	argc;
char	*argv[];
{
	struct cmd *mt;
	int desired_term;

	if ((argc != 2) || !strcmp(argv[1], "?") || !strcmp(argv[1], "help")) {
		cmdprintf(MSGSTR(EML_LIST, "format is:  'emulate Terminal', where 'Terminal' is one of:\n\n")); /*MSG*/
		for (mt = Emulatelist; mt->name; mt++) {
			cmdprintf(MSGSTR(mt->helpid, "%s\t%s\n"), 
				mt->name, mt->help); /*MSG*/
		}
		return 0;
	}
	mt = getemulatecmd(argv[1]);
	if (mt == 0) {
		fprintf(stderr, MSGSTR(UNK_EML_TERM,
			"Unknown terminal '%s' ('emulate ?' for help).\n"), argv[1]); /*MSG*/
		return 0;
	} else if (mt == Ambiguous(struct cmd *)) {
		fprintf(stderr, MSGSTR(AMB_EML_TERM,
			"Ambiguous terminal name '%s' ('emulate ?' for help).\n"), argv[1]); /*MSG*/
		return 0;
	} else {
                if (!strcmp(mt->name, "vt100"))
			desired_term = VT100;
                else if (!strcmp(mt->name, "3270"))
			desired_term = IBM3270;
                else {
			desired_term = NATIVE;
                }

                if (cantemulate(desired_term)) {
                        cmdprintf(
        MSGSTR(BAD_EML_TERM, "Can't emulate a %s terminal on a %s terminal\n"),
                                Emulatelist[negotiatedterm].name, terminalname);
                        if (EMULATING_3270) {
                                cmdprintf(
                                    MSGSTR(EML_LINE_MODE, "Using line mode\n"));
                                stopterminalnegotiation();
                        }
                        useremulate = 0;
                        negotiatedterm = NATIVE;
                }
                else {
                        (*mt->handler)(1);
			switch(desired_term) {
				case VT100:
					negotiatedterm = currentterm = VT100;
					break;
				case IBM3270:
					negotiatedterm = currentterm = IBM3270;
					break;
				case NATIVE:
					negotiatedterm = currentterm = NATIVE;
					break;
			}
		}

	}

        if (negotiatedterm) {
		cmdprintf(MSGSTR (EML_VERIF, "Now emulating a %s terminal\n"),
			Emulatelist[negotiatedterm].name);
        } else {
                cmdprintf(MSGSTR (NOT_EML_TERM, "Not emulating (native type %s)\n"), terminalname);
        }

	return 1;
}
#endif /* _AIX */

/*
 * The following data structures and routines implement the
 * "display" command.
 */

display(argc, argv)
int	argc;
char	*argv[];
{
#define	dotog(tl)	\
	if (tl->variable && tl->actionexplanation) { \
		cmdprintf(MSGSTR(tl->wactionid[*tl->variable?0:1], \
			"%s %s.\n"), *tl->variable ? "will" : "won't", \
			tl->actionexplanation); \
	}
#define	doset(sl)   \
	if (sl->name && *sl->name != ' ') { \
		cmdprintf(MSGSTR(DOSET_FMT, "[%s]\t%s.\n"), \
			control(*sl->charp), sl->name);  /*MSG*/ \
    	}

    struct togglelist *tl;
    struct setlist *sl;

    if (argc == 1) {
	for (tl = Togglelist; tl->name; tl++) {
	    dotog(tl);
	}
	cmdprintf(MSGSTR(SEP_TLST_SETLST, "\n")); /*MSG*/
	for (sl = Setlist; sl->name; sl++) {
	    doset(sl);
	}
#ifdef _AIX
	if (negotiatedterm) {
		cmdprintf(MSGSTR(EML_TERM, "Emulating a %s terminal\n"), 
			Emulatelist[negotiatedterm].name); /*MSG*/
        } else {
		cmdprintf(MSGSTR(NOT_EML_TERM,
			"Not emulating (native type %s)\n"), /*MSG*/
			terminalname);
	}
#endif /* _AIX */
    } else {
	int i;

	for (i = 1; i < argc; i++) {
	    sl = getset(argv[i]);
	    tl = gettoggle(argv[i]);
	    if ((sl == Ambiguous(struct setlist *)) ||
				(tl == Ambiguous(struct togglelist *))) {
		cmdprintf(MSGSTR(AMB_DSP_ARG, 
			"?Ambiguous argument '%s'.\n"),
			argv[i]); /*MSG*/
		return 0;
	    } else if (!sl && !tl) {
#ifdef _AIX
		if (!strncmp(argv[i], "emulate", strlen(argv[i]))) {
			if (negotiatedterm) {
				cmdprintf(MSGSTR(EML_TERM, 
					"Emulating a %s terminal\n"), 
					Emulatelist
					[negotiatedterm].name); /*MSG*/
			} else {
				cmdprintf(MSGSTR(NOT_EML_TERM,
				   "Not emulating (native type %s)\n"), 
				   terminalname);    /*MSG*/
					
			}
		} else {
#endif /* _AIX */
			cmdprintf(MSGSTR(UNK_DSP_ARG,
				"?Unknown argument '%s'.\n"), argv[i]);
				/*MSG*/
			return 0;
#ifdef _AIX
		}
#endif /* _AIX */
	    } else {
		if (tl) {
		    dotog(tl);
		}
		if (sl) {
		    doset(sl);
		}
	    }
	}
    }
    return 1;
#undef	doset(sl)
#undef	dotog(tl)
}

/*
 * The following are the data structures, and many of the routines,
 * relating to command processing.
 */

/*
 * Set the escape character.
 */
setescape(argc, argv)
	int argc;
	char *argv[];
{
	register char *arg;
	char buf[50];

	cmdprintf(MSGSTR(SET_ESC_INFO,
		"Deprecated usage - please use 'set escape%s%s' in the future.\n"), /*MSG*/
			(argc > 2)? " ":"", (argc > 2)? argv[1]: "");
	if (argc > 2)
		arg = argv[1];
	else {
		cmdprintf(MSGSTR(NEW_ESC_CHAR, "new escape character: ")); /*MSG*/
		cmdgets(buf, sizeof(buf) - 1);
		arg = buf;
	}
	if (arg[0] != '\0')
		escape = (cc_t) arg[0];
	cmdprintf(MSGSTR(ESC_CHAR, "Escape character is '%s'.\n"),
		control(escape)); /*MSG*/
	fflush(stdout);
	return 1;
}
/*VARARGS*/
toglineterm()
{
        lineterm = !lineterm;
        cmdprintf(MSGSTR(OLD_DEF_LT,
		"Deprecated usage - please use 'toggle lineterm' in the future.\n"));
        if (lineterm)
                cmdprintf(MSGSTR(ACT_WILL_CRLF_LT,
			"Will set default line terminator to CR-LF.\n"));
        else
                cmdprintf(MSGSTR(ACT_WONT_CRLF_LT,
			"Won't set default line terminator to CR-LF.\n"));
        fflush(stdout);
        return 1;
}
/*VARARGS*/
togcrmod()
{
	crmod = !crmod;
	cmdprintf(MSGSTR(OLD_TOG_CR,
		"Deprecated usage - please use 'toggle crmod' in the future.\n")); /*MSG*/
	if (crmod)
		cmdprintf(MSGSTR(ACT_WILL_MAP_CR, "Will map carriage return on output.\n")); /*MSG*/
	else
		cmdprintf(MSGSTR(ACT_WONT_MAP_CR, "Won't map carriage return on output.\n")); /*MSG*/
	fflush(stdout);
        return 1;
}

/*VARARGS*/
suspend()
{

	setcommandmode();
	kill(getpid(), SIGTSTP);
	/* reget parameters in case they were changed */
/*
	ioctl(0, TIOCGETP, (char *)&ottyb);
*/
	tcgetattr(0,(struct termios *)&ottyb);
#ifdef _AIX
	memcpy(otc.c_cc, ottyb.c_cc, NCCS * sizeof(cc_t));
#else
	ioctl(0, TCGETA, (char *)&otc);
	ioctl(0, TIOCGLTC, (char *)&oltc);
#endif /* _AIX */
	return 1; 
}

/*VARARGS*/
bye()
{
	register char *op;

	if (connected) {
		shutdown(net, 2);
                if (EMULATING_VT100) {
			vtclose();
			restore_tty();
		}
		if (EMULATING_3270) fs_end();
		cmdprintf(MSGSTR(CONC_CLOSED, "Connection closed.\n")); /*MSG*/
		close(net);
		connected = 0;
		/* reset his options */
		for (op = hisopts; op < &hisopts[256]; op++)
			*op = 0;
#ifdef _AIX
		/* reset my options - this is a BSD bug, isn't it?? */
		for (op = myopts; op < &myopts[256]; op++)
			*op = 0;
		if (!useremulate && negotiatedterm)
			donone(0);
		formofname = -1;
		sent_last = lastterm = lastform = 0;
#endif /* _AIX */
	}
	return 1;
}

/*VARARGS*/
quit()
{
	(void) call(bye, MSGSTR(BYE, "bye"), 0); /*MSG*/
#ifdef _CSECURITY
	(void) setuid(saved_uid);
	CONNECTION_WRITE(remote_addr, "telnet/tcp", "close", "", 0);
	(void) setuid(effective_uid);
#endif /* _CSECURITY */
	cmdflush();
	if (EMULATING_VT100) vtclose();
	restore_tty();
	exit(0);
	/*NOTREACHED*/
}

/*
 * Print status about the connection.
 */
/*ARGSUSED*/
status(argc, argv)
int	argc;
char	*argv[];
{
    int i = getconnmode();
    if (connected) {
	cmdprintf(MSGSTR(CONC_TO, "Connected to %s.\n"), hostname); /*MSG*/
	if (argc < 2) {
		cmdprintf(MSGSTR(idmodedescript[i],"Operating in %s.\n"),
			modedescriptions[i]);
		if (localchars) {
			cmdprintf(MSGSTR(CATCH_SIG_LOC, "Catching signals locally.\n")); /*MSG*/
		}

	    }
    } else {
	cmdprintf(MSGSTR(NO_CONC, "No connection.\n")); /*MSG*/
    }
    cmdprintf(MSGSTR(ESC_CHAR, "Escape character is '%s'.\n"), control(escape)); /*MSG*/
    fflush(stdout);
    return 1;
}

/* cleanup() - shuts down connection, returns to main via longjmp */
cleanup()
{
	bye();
#ifdef _CSECURITY
	(void) setuid(saved_uid);
	CONNECTION_WRITE(remote_addr, "telnet/tcp", "close",
		MSGSTR(C_CLSD_FRGN, "Connection closed by foreign host"), -1); /*MSG*/
	(void) setuid(effective_uid);
#endif /* _CSECURITY */
	longjmp(toplevel, -1);
}

tn(argc, argv)
	int argc;
	char *argv[];
{
	register struct hostent *host = 0;
	

	if (connected) {
		cmdprintf(MSGSTR(ALRDY_CONC, "?Already connected to %s\n"),
		hostname); /*MSG*/
		return 0;
	}
	if (argc < 2) {
		(void) strcpy(line, "Connect ");
		cmdprintf(MSGSTR(TO, "(to) ")); /*MSG*/
		cmdgets(&line[strlen(line)], sizeof(line) - strlen(line) - 1);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc > 3) {
		cmdprintf(MSGSTR(USAGE, "usage: %s host-name [port]\n"), argv[0]); /*MSG*/
		return 0;
	}

	sin.sin_addr.s_addr = inet_addr(argv[1]);
	/* see if it's a name or an address */
        if (!isinet_addr(argv[1])) {
                if ((host = gethostbyname (argv[1])) == (struct hostent *) 0) {
                        cmdprintf(MSGSTR(NME_NT_FND,
                                        "telnet: Unknown host %s\n"), argv[1]);
                        return 0;
                }
                sin.sin_family = host->h_addrtype;
#ifndef NOT43
                bcopy(host->h_addr_list[0],
                    (caddr_t)&sin.sin_addr, host->h_length);
#else   NOT43
		bcopy(host->h_addr, (caddr_t)&sin.sin_addr,
                      host->h_length);
#endif  /* NOT43 */
                hostname = host->h_name;
        }
        else {
                if (sin.sin_addr.s_addr == -1) {
                     cmdprintf(MSGSTR(UNK_HOST, 
				"telnet: Address %s misformed\n"),argv[1]);
                     return 0;
                }
                sin.sin_family = AF_INET;
		(void) strcpy(hnamebuf, argv[1]); 
		hostname = hnamebuf;
        }
	sin.sin_port = sp->s_port;
	if (argc == 3) {
		sin.sin_port = atoi(argv[2]);
		if (sin.sin_port == 0) {
			sp = getservbyname(argv[2], "tcp");
			if (sp)
				sin.sin_port = sp->s_port;
			else {
				cmdprintf(MSGSTR(BAD_PORT_NO, "%s: bad port number\n"), argv[2]); /*MSG*/
				return 0;
			}
		} else {
			sin.sin_port = atoi(argv[2]);
			sin.sin_port = htons(sin.sin_port);
		}
		telnetport = 0;
	} else {
		telnetport = 1;
	}
	(void) signal(SIGINT, (void(*)(int))intr);
	(void) signal(SIGQUIT, (void(*)(int))intr2);
	(void) signal(SIGPIPE, (void(*)(int))deadpeer);
        (void) signal(SIGWINCH, (void(*)(int))sigwinch);
	cmdprintf(MSGSTR(TRYING, "Trying...\n")); /*MSG*/
#ifdef _CSECURITY
       	remote_addr = sin.sin_addr.s_addr;
#endif /* _CSECURITY */
	do {
		net = socket(AF_INET, SOCK_STREAM, 0);
		if (net < 0) {
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		CONNECTION_WRITE(remote_addr, "telnet/tcp", "close",
					sys_errlist[errno], errno);
		(void) setuid(effective_uid);
#endif /* _CSECURITY */
			perror(MSGSTR(P_TN_SOCKET, "telnet: socket")); /*MSG*/
			return 0;
		}
#ifndef	NOT43
		if (debug &&
				setsockopt(net, SOL_SOCKET, SO_DEBUG,
					(char *)&debug, sizeof(debug)) < 0)
#else	NOT43
		if (debug && setsockopt(net, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
#endif	/* NOT43 */
			perror(MSGSTR(P_SETSOCK_DBG, "setsockopt (SO_DEBUG)")); /*MSG*/

		if (connect(net, (struct sockaddr *)&sin, sizeof (sin)) < 0) {
#ifndef	NOT43
			if (host && host->h_addr_list[1]) {
				int oerrno = errno;

				fprintf(stderr,
				    MSGSTR(CONC_TO_ADD,
				    "telnet: connect to address %s: "), /*MSG*/
				    inet_ntoa(sin.sin_addr));
				errno = oerrno;
				perror((char *)0);
				host->h_addr_list++;
				bcopy(host->h_addr_list[0],
				    (caddr_t)&sin.sin_addr, host->h_length);
				fprintf(stderr, MSGSTR(TRYING_HOST,
					"Trying %s...\n"), /*MSG*/
					inet_ntoa(sin.sin_addr));
				(void) close(net);
				continue;
			}
#endif	/* NOT43 */
			if (errno != EINTR)
				perror(MSGSTR(P_TN_CONNECT, "telnet: connect")); /*MSG*/
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
#ifdef _CSECURITY
			(void) setuid(saved_uid);
			CONNECTION_WRITE(remote_addr, "telnet/tcp", "close",
					sys_errlist[errno], errno);
			(void) setuid(effective_uid);
#endif /* _CSECURITY */
			return 0;
		}
		connected++;
	} while (connected == 0);
	call(status, "status", "notmuch", 0);
#ifdef _AIX
	if (EMULATING_3270) {
		fs_init();
	}
	if (EMULATING_VT100) {
		vtinit();
		if (linemode)   {
			cmdprintf(MSGSTR(VT100_CHAR, 
				"Selecting character mode.\n"));
			docharmode();
		}
	}
#endif /* _AIX */
	if (setjmp(peerdied) == 0) {
#ifdef _CSECURITY
		(void) setuid(saved_uid);
		CONNECTION_WRITE(remote_addr, "telnet/tcp", "open", "", 0);
		(void) setuid(effective_uid);
#endif /* _CSECURITY */

		telnet();
	}
	cleanup();
}


/* If MSG is defined the following define is not used.  The format string
to display the command help should be changed to reflect the length of the
longest command name ("connect").  See help() routine below */

char	openhelp[] =	"connect to a site";
char	closehelp[] =	"close current connection";
char	quithelp[] =	"exit telnet";
char	zhelp[] =	"suspend telnet";
char	statushelp[] =	"print status information";
char	helphelp[] =	"print help information";
char	sendhelp[] =	"transmit special characters ('send ?' for more)";
char	sethelp[] = 	"set operating parameters ('set ?' for more)";
char	togglestring[] ="toggle operating parameters ('toggle ?' for more)";
char	displayhelp[] =	"display operating parameters";
char	modehelp[] =
		"try to enter line-by-line or character-at-a-time mode";
#ifdef _AIX
char	emulatehelp[] = "emulate a vt100 or 3270 terminal";
#endif /* _AIX */

int	help();

struct cmd cmdtab[] = {
	{ "close",	CLOSE_H_ID,	closehelp,      bye,            1, 1 },
	{ "display",	DISPLAY_H_ID,	displayhelp,    display,        1, 0 },
#ifdef _AIX
	{ "emulate",	EMULATE_H_ID,	emulatehelp,    emulate,        1, 0 },
#endif /* _AIX */
	{ "mode",	MODE_H_ID,	modehelp,       modecmd,        1, 1 },
	{ "open",	OPEN_H_ID,	openhelp,       tn,             1, 0 },
	{ "quit",	EXIT_H_ID,	quithelp,       quit,           1, 0 },
	{ "send",	SEND_H_ID,	sendhelp,       sendcmd,        1, 1 },
	{ "set",	SET_H_ID,	sethelp,        setcmd,         1, 0 },
	{ "status",	STATUS_H_ID,	statushelp,     status,         1, 0 },
	{ "toggle",	TOGGLE_H_ID,	togglestring,   toggle,         1, 0 },
	{ "z",		Z_H_ID,		zhelp,          suspend,        1, 0 },
	{ "?",		HELP_H_ID,	helphelp,       help,           1, 0 },
	0
};

char	crmodhelp[] =	"deprecated command -- use 'toggle crmod' instead";
char	escapehelp[] =	"deprecated command -- use 'set escape' instead";

struct cmd cmdtab2[] = {
	{ "help",	HELP_H_ID,	helphelp,       help,           0, 0 },
	{ "escape",	OESCAPE_H_ID,	escapehelp,     setescape,      1, 0 },
	{ "crmod",	OCR_H_ID,	crmodhelp,      togcrmod,       1, 0 },
	0
};

/*
 * Help command.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;
	char	*frmt;
	char	*t;

	if (argc == 1) {
		cmdprintf(MSGSTR(CMD_HELP,
			"Commands may be abbreviated.  Commands are:\n\n")); /*MSG*/
		/* The original format string will not work with positional
		   parameters.  "%-*s\t%s\n"  The asterisk should be replaced
		   by the length of the longest command name. */
		t = MSGSTR(HELP_FMT, "%-8s\t%s\n");
		frmt = (char *)malloc(strlen(t) + 1);
		strcpy(frmt, t);
		for (c = cmdtab; c->name; c++)
			if (c->dohelp) {
				cmdprintf(frmt, c->name,
					MSGSTR(c->helpid, c->help));
			}
		free(frmt);
		return 0;
	}

	t = MSGSTR(HELP_FMT_S, "%s\n");
	frmt = (char *)malloc(strlen(t) + 1);
	strcpy(frmt, t);

	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == Ambiguous(struct cmd *))
			cmdprintf(MSGSTR(AMB_HELP_CMD, "?Ambiguous help command %s\n"), arg); /*MSG*/
		else if (c == (struct cmd *)0)
			cmdprintf(MSGSTR(INV_HELP_CMD, "?Invalid help command %s\n"), arg); /*MSG*/
		else
			cmdprintf(frmt, MSGSTR(c->helpid, c->help));
	}
	free(frmt);
	return 0;
}
/*
 * Call routine with argc, argv set from args (terminated by 0).
 * VARARGS2
 */
call(routine, args)
	int (*routine)();
	char *args;
{
	register char **argp;
	register int argc;


	for (argc = 0, argp = &args; *argp++ != 0; argc++)
		;
	return (*routine)(argc, &args);

}

makeargv()
{
	register char *cp;
	register char **argp = margv;

	margc = 0;
	for (cp = line; *cp;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;
}

char **
getnextcmd(name)
char *name;
{
    struct cmd *c = (struct cmd *) name;

    return (char **) (c+1);
}

struct cmd *
getcmd(name)
char *name;
{
    struct cmd *cm;

    if (cm = (struct cmd *) genget(name, (char **) cmdtab, getnextcmd)) {
	return cm;
    } else {
	return (struct cmd *) genget(name, (char **) cmdtab2, getnextcmd);
    }
}

command(top)
	int top;
{
	register struct cmd *c;

	static	char	*prompt_fmt = NULL;
	char	*t;

	if (prompt_fmt == NULL) {
		t = MSGSTR(TN_PROMPT, "%s> "); /*MSG*/
		prompt_fmt = (char *)malloc(strlen(t) + 1);
		strcpy(prompt_fmt, t);
	}

	if (EMULATING)
		tty_litout(0);		/* save screen */

	setcommandmode();
	if (!top) {
		cmdprintf("\n");
	} else {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}
	for (;;) {
		cmdflush();
		cmdprintf(prompt_fmt, prompt);
		if (cmdgets(line, sizeof(line) - 1) == 0) {
			if (feof(stdin))
				quit();
			break;
		}
		if (line[0] == 0)
			break;
		makeargv();
		c = getcmd(margv[0]);
		if (c == Ambiguous(struct cmd *)) {
			cmdprintf(MSGSTR(AMB_CMD, "?Ambiguous command\n")); /*MSG*/
			continue;
		}
		if (c == 0) {
			cmdprintf(MSGSTR(INV_CMD, "?Invalid command\n")); /*MSG*/
			continue;
		}
		if (c->needconnect && !connected) {
			cmdprintf(MSGSTR(CONC_FIRST, 
				"?Need to be connected first.\n")); /*MSG*/
			continue;
		}
		if ((*c->handler)(margc, margv) && !top) {
			break;
		}
	}
	if (EMULATING)
		tty_litout(1);		/* restore screen */

	if (!top) {
		if (!connected) {
			longjmp(toplevel, 1);
			/*NOTREACHED*/
		}
		setconnmode();
	}
}

/*
 * main.  Parse arguments, invoke the protocol or command parser.
 */

#include <locale.h>

main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef _AIX
	char *getenv(), *temp[2];
	char *octal_escape;
	int c;
	extern int optind;
	extern char *optarg;
	int eflag = 0;
#endif /* _AIX */
	char	*t;
#ifndef _POWER
	struct hftgetid hftgetid;
#endif /* _POWER */

	setlocale(LC_ALL,"");
	catd = catopen(MF_TELNET,NL_CAT_LOCALE);

#ifdef DUMPWRITE
	fp = fopen("debug1","w");
#endif /* DUMPWRITE */


#ifdef _CSECURITY
	if ((auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0)) < 0) {
		syslog(LOG_ALERT,
		    MSGSTR(SLOG_AUDIT_PROC, "telnet: auditproc: %m")); /*MSG*/
		exit(1);
	}
	saved_uid = getuidx(ID_SAVED);
	effective_uid = getuid();
        if (setuidx(ID_REAL|ID_EFFECTIVE, effective_uid) < 0) {
		perror(MSGSTR(P_SETUID, "setuid")); /*MSG*/
		exit(1);
        }
        privilege(PRIV_LAPSE);
#else
	if (setuid(getuid())) {
		perror(MSGSTR(P_SETUID, "setuid")); /*MSG*/
		exit(1);
	}
#endif /* _CSECURITY */

	sp = getservbyname("telnet", "tcp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNK_SERVICE,
			"telnet: tcp/telnet: unknown service\n")); /*MSG*/
		exit(1);
	}
	NetTrace = stdout;
/*
	ioctl(0, TIOCGETP, (char *)&ottyb);
*/
	tcgetattr(0,(struct termios *)&ottyb);

#ifndef _POWER
	/* set flag that indicates whether we're on an hft or not */
	is_hft = !ioctl(0, HFTGETID, &hftgetid);
#endif /* _POWER */

        prompt = rindex(argv[0], '/');
        if (prompt)
                prompt++;
        else
                prompt = argv[0];

	if (!strcmp(prompt, TN))
		escape = (cc_t) CTRL('t');

#ifdef _AIX
	/*
	 * Environment variable allows tn3270 users to turn on/off
	 * ICRNL on the tty.  See the mode() subroutine.
	 * Only effective if we are called as tn3270 command.
	 */
	tn3270cr = getenv("TN3270CR");

	/* AIX has the "interesting" feature that the user can specify the
	 * desired escape key in OCTAL through the TNESC environmental 
	 * variable!
	 */
	if (octal_escape = getenv("TNESC")) {
		if (!(escape = (cc_t) otoi(octal_escape))) {
			cmdprintf(MSGSTR(INV_OCT_ESC,
			"Invalid octal escape value %s\n"), octal_escape); /*MSG*/
			escape = (cc_t) CTRL(']');
		}
	}
	memcpy(otc.c_cc, ottyb.c_cc, NCCS * sizeof(cc_t));
	oltc.t_flushc = (cc_t) CTRL('O');
	oltc.t_suspc = escape;
	setupterminals();
#else
	ioctl(0, TCGETA, (char *)&otc);
	ioctl(0, TIOCGLTC, (char *)&oltc);
#endif /* _AIX */
#if	defined(LNOFLSH)
	ioctl(0, TIOCLGET, (char *)&autoflush);
	autoflush = !(autoflush&LNOFLSH);	/* if LNOFLSH, no autoflush */
#else	/* LNOFLSH */
	autoflush = 1;
#endif	/* LNOFLSH */
	ntc = otc;
	nltc = oltc;
	nttyb = ottyb;
	setbuf(stdin, (char *)0);
#ifdef NEVER
        /* Setbuf() for stdout call is being ifdef'ed out for performance
         * reasons, mostly for 3270 emulation.  This gets us 3-4x improvement
         * for 3270 and a little for the more general UNIX -> UNIX telnet
         * connection.  Hopefully this will not cause any problems....
         */
        setbuf(stdout, (char *)0);
#endif /* NEVER */
#ifdef _AIX
	if (!strcmp(prompt, TN3270)) {
		char tempstr[5] = "3270";
		temp[1] = tempstr;
		++tn3270flag;
		++eflag;
		if(octal_escape == NULL)
			escape = (cc_t) CTRL('c');
		ntc.t_intrc = (cc_t) CTRL(']');
		if (emulate(2, temp) == 0)
			exit(1);
	}

	/* use getopt, modularity fans! */
	while ((c = getopt(argc, argv, "dn:e:")) != EOF) {
		switch(c) {
		case 'd':
			debug = 1;
			break;
		case 'n':
			NetTrace = fopen(optarg, "w");
			if (NetTrace == NULL) {
				perror(MSGSTR(NULL_NT, "")); /*MSG*/
				NetTrace = stdout;
			}
			break;
		case 'e':
			if (!tn3270flag) {
				++eflag;
				temp[1] = optarg;
				if (emulate(2, temp) == 0)
					exit(1);
			}
			break;
		case '?':
			fprintf(stderr, MSGSTR(USAGE_AIX,
	"Usage: %s [-d] [-n NetTrace] [-e terminal] [host] [port]\n"), prompt); /*MSG*/
			exit(1);
		}
	}
	if (!tn3270flag && !eflag && (temp[1] = getenv("EMULATE"))) {
		if (emulate(2, temp) == 0)
			exit(1);
	}
	argc -= optind-1;
	argv += optind-1;
#else
	if (argc > 1 && !strcmp(argv[1], "-d")) {
		debug = 1;
		argv++;
		argc--;
	}
	if (argc > 1 && !strcmp(argv[1], "-n")) {
	    argv++;
	    argc--;
	    if (argc > 1) {		/* get file name */
		NetTrace = fopen(argv[1], "w");
		argv++;
		argc--;
		if (NetTrace == NULL) {
		    NetTrace = stdout;
		}
	    }
	}
#endif /* _AIX */

	t = MSGSTR(OFF_STRING, "off");
	if (offstring = (char *)malloc(strlen(t) + 1)) {
		strcpy(offstring, t);
	}
	else {
		offstring = "off";
	}

	if (argc != 1) {
		if (setjmp(toplevel) != 0)
			quit();
		if (tn(argc, argv) == 0)
			quit();
	}

        if (setjmp(toplevel) == -1) {
		command(0);
                signal(SIGINT, (void(*)(int))intr);
		telnet();
		cleanup();
	}

	for (;;)
		command(1);
}

#ifdef _AIX

/* Interface programs for old AIX code */

/* ARGSUSED */
tcp_flush(dummy)
int dummy;
{
	netflush(net);
}

/* ARGSUSED */
tcp_putc(dummy, character)
int dummy, character;
{
	NETADD(character);
}

/* silly */
otoi(string)
register char *string;
{
	register int answer = 0;

	while (isdigit(*string)) {
		answer <<= 3;
		answer += (*string) - '0';
		++string;
	}
	return answer;
}

#endif /* _AIX */
