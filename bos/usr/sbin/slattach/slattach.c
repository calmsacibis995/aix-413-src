static char sccsid[] = "@(#)56  1.10.2.7  src/bos/usr/sbin/slattach/slattach.c, cmdnet, bos411, 9438B411a 9/20/94 14:54:32";
/* 
 * COMPONENT_NAME: (CMDNET) Network commands
 * 
 * FUNCTIONS: slattach, assert, cleanup, loadflds,
 *            logent, sigcatch, sighupcatch
 *
 * ORIGINS: 26 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/* Modified 7/18/91 to retry connections on sighup. */

/*
 * Must run ifconfig first.
 * Slattach opens the tty port specified on the command line, sets
 * the line discipline to SLIPDISC, and then reads from the port.  This
 * is still a rather primitive version of what could be done.  First of
 * all, when a sighup is received, the program currently terminates.  It
 * should probably attempt to reopen the port under some conditions.
 * Finally, if the auto-disconnect ever
 * becomes an auto-disconnect with reconnect, it will have to be done
 * here.  Also, since slip assumes that this program has the port opened,
 * other signals besides sighup (like interrupt and quit) could be sent
 * up for other conditions like the auto-disconnect feature.
 *
 */


#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/dir.h>
#include <sys/errno.h>
#include <varargs.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>

#include <locale.h>
#include <nl_types.h>
#include "slattach_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SLATTACH,n,s)
#define CNULL (char *) 0

extern void sigcatch(int);
extern void sighupcatch(int);
extern int errno;
volatile int CONNECTED;
extern int Debug = 0;
void loadflds(char *);

/* need pointer for NLS instead of [] */
char *usage = "Usage: slattach ttyname [baudrate dialstring [debuglevel]]   \n";

char *progname;
/*
 * chat in		altconn in
 * ../uucp/conn.o	../uucp/altconn.o
 *
chat(int nf, char *flds, int fn, char *phstr1, char *phstr2);
 *
 * These dummy routines are needed because the uucp routines
 * used by chat reference them, but they will never be
 * called when executing from ct
 */
void assert () { }              /* for ASSERT in gnamef.c */
void logent () { }              /* so we can load ulockf() */
extern void cleanup (int );               

char   *Myline = NULL;      /* referenced in sconn.c */
jmp_buf Sjbuf;              /* used by uucp routines */

#define YES     1               /* mnemonic */
#define NO      0               /* mnemonic */


extern int
        Child,                  /* pid for receive process */
        Rtn_code=0;             /* default return code */

extern int Verbose = 1;         /*for uucp callers,  dialers feedback*/
static struct call {            /*NOTE-also included in altconn.c--> */
                                /*any changes must be made in both places*/
        char *speed;            /* transmission baud rate */
        char *line;             /* device name for outgoing line */
        char *telno;            /* ptr to tel-no digit string */
        char *class;            /* call class */
        }slcall;


#define MAX_CHAT_FLDS 20
char *flds[MAX_CHAT_FLDS];
int nf;
int fd;	

char filename[81];

/* 
 * usage: slattach ttyname [baudrate dialstring[debuglevel]]
 */
main(argc, argv)
int argc;
char *argv[];
{
    register char *dev;
    int pid, i; 
    int dfd;
    int unit;
    int result;
    tcflag_t c_cflag = 0;       /* place to save tty c_cflag */
    extern tcflag_t chgc_cflag();
    struct termios term;

    setlocale(LC_ALL,"");
    catd = catopen(MF_SLATTACH,NL_CAT_LOCALE);
    usage = MSGSTR(USAGE,usage);    /* usage defined globally - MSG */
    
    slcall.speed = "Any";       /*default speed*/
    slcall.line  = NULL;
    slcall.telno = NULL;
    slcall.class = NULL;

    if (progname = strrchr(argv[0], '/'))
        ++progname;
    else
        progname = argv[0];

    if (argc < 2 || argc > 5 || argc == 3) {
     	fprintf(stderr, MSGSTR(USAGE,
	"usage: %s ttyname [baudrate dialstring[debuglevel]]\n"),
		progname);  /*MSG*/
	exit(1);
    }
    if (dev = strrchr(argv[1], '/'))
    	++dev;
    else
    	dev = argv[1];
    strcpy(filename,"/dev/");
    strcat(filename,dev);

    setpgrp(getpid()); 
	
    (void) signal(SIGPIPE, SIG_IGN); 
    (void) signal(SIGQUIT, SIG_IGN); 
    (void) signal(SIGCHLD, SIG_IGN); 
    (void) signal(SIGINT,  SIG_IGN); 
    (void) signal(SIGTSTP, SIG_IGN); 
    (void) signal(SIGTERM, sigcatch);
    (void) signal(SIGHUP, SIG_IGN);

    /*
     * Half hearted attempt at loading the slip module with strload.
     */
    (void) system("/usr/sbin/strinfo -m | grep 'slip' > /dev/null || /usr/sbin/strload -m /usr/lib/drivers/slip");

    CONNECTED = 0;

    /*
     * The fork puts slip in the background so we can set a
     * new process group master, needs to be done here since
     * uucp services (altconn, chat) will create lock file with
     * pid
     */
    if ((pid = fork()) > 0) {
	exit(0);
    }
   
    pid = setsid();
    if (pid == -1) {
        fprintf(stderr, MSGSTR(SETSID, "setsid failed\n"));
        exit(1);
    }

    /* If tty is already locked, quit. */
    if (ttylocked(dev)) { 
 	fprintf(stderr, MSGSTR(LOCKS, "%s: Device already in use (locked) \n"),
 		progname);
 	exit(1);
    }

    /* open tty */
    if (0 > (fd = open(filename,O_RDWR|O_NDELAY))) {
    fprintf(stderr, MSGSTR(DEVICE, "%s: could not open %s\n"),
	progname, filename);
	perror(filename);
	exit(1);
    }
    
    /* clean up tty before pushing ldterm and tioc */
    while (ioctl(fd, I_POP, 0) == 0) {
	continue;
    }

    /*
     *  Use ldterm and tioc line disciplines for altconn and chat
     */
    if (ioctl(fd, I_PUSH, "ldterm") < 0) { 
     	fprintf(stderr, MSGSTR(POSIXDD,
	"%s: could not set posix line discipline for %s.\n"),
	progname, filename);
	exit(1);
    }

    if (ioctl(fd, I_PUSH, "tioc") < 0) { 
     	fprintf(stderr, MSGSTR(POSIXDD,
	"%s: could not set posix line discipline for %s.\n"),
	progname, filename);
	exit(1);
    }

    setpgrp(pid);
    tcsetpgrp(fd, pid);
   
    tcgetattr(fd, &term);

    /* argc > 3 send chat string via altconn and chat */	
    if (argc > 3)  {

        /* restore CREAD, might have been turned off */
	term.c_lflag &= ((~ECHO) & (~ECHOE) & (~ECHOK) & (~ECHONL));
        term.c_cflag |= CREAD;
        tcsetattr(fd, TCSAFLUSH, &term);

	/*
	 *  set Debug level prior to calling altconn and chat
	 */
	if (argc >= 5) {
	    Debug = atoi(argv[4]);
     	    fprintf(stderr, MSGSTR(DEBUGL, "%s: debug level set to %d\n"),
		    progname, Debug);
	}
	
	slcall.line = filename;
	slcall.speed = argv[2];

	if ( 0 > (dfd = altconn(&slcall))) {
	    fprintf(stderr, MSGSTR(ALTCONN,
		"%s: Open of %s for a 'direct' line failed.\nCheck BNU Devices file(s)\n"),
		progname, filename);
	    perror("altconn"); 
	    exit(1);
	}

	if (argc >= 4) {
            int rc;
            c_cflag = chgc_cflag(dfd, CLOCAL);
	    loadflds(argv[3]);
	    if (Debug)
		fprintf(stderr, MSGSTR(DEBUGCHAT,
				"%s: chat(%d, %x, %d)\n"), progname,
				nf, flds, dfd);
            rc = chat(nf, flds, dfd, "", "");
            (void) chgc_cflag(dfd, c_cflag);
            if (rc) {
                NLprintf(MSGSTR(CONNTERM, "%s: %s connection terminated\n"),
                         progname, filename);
                exit(0);
            }
	}
    } 
    else if (ttylock(dev)) { 
	fprintf(stderr, MSGSTR(LOCKS, "%s: Device already in use (locked) \n"),
		progname);
	exit(1);
    }

    /* gotta flush the term so that I_PUSH "slip" will succeed */
    /* we also gotta turn off all output and input processing in the adapter */
    tcgetattr(fd, &term);             /* get the tty attributes */
    term.c_iflag = 0;                 /* turn off all input processing */
    term.c_oflag = 0;                 /* turn off all output processing */
    term.c_lflag = 0;                 /* turn off local options */
    term.c_cflag |= CREAD;            /* make sure we receive characters */
    for (i = 0; i < NCCS; i++)
      term.c_cc[i] = 0xFF;            /* disable control characters */
    tcsetattr(fd, TCSAFLUSH, &term);  /* set the new tty attributes */

    /* NOW we want to get an error message if we are hung up on */	
    (void) signal(SIGHUP, sighupcatch);

    /* loop until you get/re-establish connection */
    while (!CONNECTED) {
        /* open tty, hang waiting for carrier */
        if ((dfd = open(filename,O_RDWR)) < 0) {
         	fprintf(stderr, MSGSTR(DEVICE, "%s: could not open %s\n"),
        		progname, filename);
	        perror(filename);
		exit(1);
    	}
        fprintf(stderr, MSGSTR(SUCC_DEVICE, "%s: successfully opened %s\n"),
        	progname, filename);

	/* clean up tty before pushing slip */
	while (ioctl(dfd, I_POP, 0) == 0) {
		continue;
    	}

	/* set line discipline to slip */
    	if (ioctl(dfd, I_PUSH, "slip") < 0) {
     		fprintf(stderr, MSGSTR(SLIPDD,
			"%s:could not set line discipline for %s to 'slip.'\n"),
		progname, filename);
		perror("ioctl(I_PUSH)");
		exit(1);
    	}
	else {
		CONNECTED = 1;
	}

	unit = getunit(dev);
	if (unit < 0) {
		fprintf(stderr, MSGSTR(NOIU, 
				"Could not retrieve interface unit.\n"),
			progname);
    		ioctl(fd, I_POP, "slip");
		exit(1);
	}
	fprintf(stderr, MSGSTR(WHICHIU, "%s: Using interface sl%d for %s\n"), 
		progname, unit, filename);

	result = ioctl(dfd, SLIOCSATTACH, &unit);
	if (result < 0) {
		fprintf(stderr, MSGSTR(NOATTACH, 
				"ioctl(SLIOCSATTACH) failed. errno is %d\n"), 
			errno);
    		ioctl(dfd, I_POP, "slip");
		exit(1);
	}

    	fprintf(stderr, MSGSTR(CONNEST, "%s: %s connection established\n"),
   		 progname, filename);

	/* hang around for connection to terminate   
	 * also keeps tty port locked 
	 */ 
	pause();
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

		
void sighupcatch(int s)
{
    signal(SIGHUP, sighupcatch);
    CONNECTED = 0 ;
    fprintf(stderr, MSGSTR(CONNTERM, "%s: %s sighup terminated\n"),
   	 progname, filename);
/* For 4.1, don't do the following I_POP, since it appears that the
 * streamhead cleans up the protocol stack before notifying us.  The
 * following ioctl just nets a failure.
 *  if (ioctl(fd, I_POP, "slip") < 0) {	/% set it up %/
 *   	fprintf(stderr, MSGSTR(NOPOP,
 *		"%s: could not remove %s line discipline from %s\n"),
 *		progname, "slip", filename);
 *	perror("ioctl(I_POP)");
 *  }
 */
}


void sigcatch(int s)
{
    signal(SIGTERM, sigcatch);
    if (ioctl(fd, I_POP, "ldterm") < 0) {	/* set it up */
     	fprintf(stderr, MSGSTR(NOPOP,
		"%s: could not remove %s line discipline from %s\n"),
		progname, "posix", filename);
	perror("ioctl(I_POP)");
	exit(1);
    }
    cleanup(s);
    exit(0);
}

void 
cleanup (register int  code)
{
	clrlock (CNULL);
	rmlock (CNULL);
}


char dummy_field[] = "\"\"";
/*
 * split dial string into fields for uucp "chat"
 */
void loadflds(char *s) {
	if (NULL == s) {
     		fprintf(stderr, MSGSTR(NULLFLDS,
		"%s: NULL dial string; bailing out\n"),
		progname);
		exit(1);
	}
	
	nf = 0;
	while ( ('\0' != *s) && (nf <= MAX_CHAT_FLDS - 2) ) {

		/*
		 *  skip leading white space
		 */
		while ((*s == ' ') || (*s == '\t'))
			++s;

		/*
		 *  if we ended in blanks, quit
		 */
		if ('\0' == *s) 
			break;

		flds[nf] = s;

		/*
		 *  find next white space
		 */
		while(*s > ' ') 
			++s;

		if ('\0' != *s) {
			*s = '\0';
			++s;
		}
	
		++nf;
	}
	if (nf & 1) {
    		fprintf(stderr, MSGSTR(ODDFLDS,
	     "%s: loadflds: odd number of fields = %d\nbailing out\n"), 
		  progname, nf);
		exit(1);
	}
	flds[nf++] = dummy_field;
	flds[nf++] = dummy_field;
}
