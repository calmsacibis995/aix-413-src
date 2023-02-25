/* @(#)80	1.20.2.8  src/bos/usr/sbin/tsm/tsm.h, cmdsauth, bos411, 9428A410j 3/15/94 18:05:19 */
/*
 * COMPONENT_NAME: (CMDSAUTH) Terminal State Manager
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TSM    
#define _H_TSM

#define DEF_LOGIN   \
	"login: "
#define DEF_PASSWD \
        "%s's Password: "
#define DEF_NOSESSIONS \
	"All available login sessions are in use"
#define DEF_NOTLOWEST \
	"You must \"exec\" login from the lowest login shell."
#define DEF_OPENWIN \
	"You must close all windows before logging out."
#define DEF_NOSAK \
	"You must use the SAK key to get on the trusted path."
#define DEF_BADLOGIN \
	"You entered an invalid login name or password."
#define DEF_FAILCRED  \
	"Failed setting credentials."
#define DEF_FAILEXEC  \
	"Failed running login shell."
#define DEF_FAILTERM  \
	"Failed setting terminal ownership and mode."
#define DEF_OFFTPATH \
	"Trusted path required for this terminal."
#define DEF_NOTPATH \
	"Trusted path not allowed for this terminal."
#define DEF_NOTSH \
	"Trusted shell not allowed for this account."
#define DEF_NOARGS \
	"TSM was invoked without a port name"
#define DEF_CANTOPEN \
	"TSM was unable to open port \"%s\""
#define DEF_CANTCLEAN \
	"TSM was unable to establish a trusted path."
#define DEF_BADBAUD \
	"TSM was invoked with an illegal baud rate."
#define DEF_NOLOGGER \
	"TSM was unable to execute \"%s\"."
#define DEF_PLEASEFIX \
	"TSM encountered an error on terminal \"%s\"."
#define DEF_BADENV \
	"The variable \"%s\" may not be set.\n"
#define	DEF_NOPRIV \
	"TSM lacks a required privilege.\n"
#define	DEF_FAILREMLOG \
	"Remote login failed.\n"
#define DEF_UNKLOGOPT \
	"-\"%c\" is not a valid option to login.\n"
#define DEF_IOCTLFAIL \
	"ioctl failed errno \"%d\".\n"
#define DEF_TCGATTRFAIL \
	"tcgetattr  failed errno \"%d\".\n"
#define DEF_TCSATTRFAIL \
	"tcsetattr  failed errno \"%d\".\n"
#define DEF_ALLOCFAIL \
	"malloc failed.\n"
#define DEF_DUPFAIL \
	"dup failed.\n"
#define DEF_SEMFAIL \
	"sem op failed.\n"
#define DEF_LINEDISPF \
	"Failure setting line disp.\n"
#define DEF_SLOGIN \
	 "Last login: %s on %s\n"
#define DEF_SLOGINHT \
	 "Last login: %s on %s from %s\n"
#define DEF_FLOGIN \
	"Last unsuccessful login: %s on %s\n"
#define DEF_FLOGINHT \
	"Last unsuccessful login: %s on %s from %s\n"
#define DEF_FCOUNT \
	"1 unsuccessful login attempt since last login\n"
#define DEF_FCOUNTS \
	"%d unsuccessful login attempts since last login\n"
#define DEF_NOISE \
	"Write timed out -- possible noise on port"
#define DEF_NOPAS \
        "Password read timed out -- possible noise on port"
#define DEF_ALLCAPS \
	"YOU LOGGED IN USING ALL UPPERCASE CHARACTERS.\r\nIF YOUR WORKSTATION ALSO SUPPORTS LOWERCASE\r\nCHARACTERS, LOG OFF, THEN LOG IN AGAIN USING\r\nLOWERCASE CHARACTERS.\r\n"
#define DEF_HERALD \
        "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\rAIX Version 4\n\r(C) Copyrights by IBM and by others 1982, 1994.\n\rlogin: "
#define DEF_CONHERALD \
        "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\rAIX Version 4\n\r(C) Copyrights by IBM and by others 1982, 1994.\n\rConsole login: "
#define DEF_SETTINGS \
	"TSM: All possible term settings tried unsuccessfully."
#define DEF_WRITEERR \
	"TSM: write to %s failed."
#define DEF_READERR \
	"TSM: read from %s failed."

/*
 *   TSM flags from options passed in
 */

#define	GETTY_DELAYED	1
#define	GETTY_UU	2

#define TSM_CONSOLE	"/dev/console"	/* The console */
#define TSM_LOGIN	"login"	/* login program not getty   */
#define TSM_SET		1	/* Save the last login information */
#define TSM_GET		2	/* Retreive the last login information */
#define TSM_MAX_SYNS	10	/* Max number of synonym for a port */
#define TSM_DEV		"/dev/"
#define TSM_SAK_NOTSET	1     
#define TSM_SAK		2     
#define TSM_SAK_CH	3     
#define TSM_NOSAK	4     
#define TSM_NOSAK_CH	5     
#define TSM_NOEXIT	-1     
#define TSMLOGIN	1      
#define TSMGETTY	2      

#define MAXTRIES	3	/* maximum number of login attempts */

#define UTSIZ	(sizeof (struct utmp))

#define INFD	0
#define OUTFD	1
#define ERRFD	2

#include "tsm_msg.h"		/* message definitions */
#	define	MSGSTR(Num, Str) catgets (catd, MS_TSM, Num, Str)

#ifdef	DEBUGX
#	define		DEBUGMODES(a, b)	Debugmodes(a, b)
#	define		DPRINTF(args)		dprintf args
#	define		PRINTARGS(args)		printargs args
#else
#	define		DPRINTF(args)
#	define		DEBUGMODES(a, b)
#	define		PRINTARGS(args)	
#endif

struct lastlogin {
	time_t	ftime;
	time_t	stime;
	int	fcount;
	char	user[32];
	char	*stty;
	char	*ftty;
	char	*shost;
	char	*fhost;
};

struct tsmsak_status {
	char	port[32];
	int	status;
	struct	tsmsak_status *next;
};

typedef struct tsmsak_status * TSMSAK_LIST_t;
typedef struct tsmsak_status  TSMSAK_LIST;
typedef	enum { 
	TPATH_NOSAK, TPATH_NOTSH, TPATH_ALWAYS, TPATH_ON } 
tpath_t;

/*
 * In system libraries
 */
extern int acl_fput();
extern struct acl *acl_get();
extern int acl_put();
extern int append_wtmp();
extern int auditproc();
extern int auditwrite();
extern void endutent();
extern void exit();
extern int frevoke();
extern key_t ftok();
extern struct CuAt *getattr();
extern int gethostname();
extern char *getpass();
extern int getpriv();
extern struct passwd *getpwnam();
extern struct passwd *getpwuid();
extern struct utmp *getutent();
extern int grouptoID();
extern int iswlower();
extern int iswspace();
extern int iswupper();
extern int killpg();
extern int revoke();
extern int seteuid();
extern int setpgrp();
extern int setpriv();
extern void setutent();
extern int syslog();
extern int tcb();
extern char *termdef();
extern wint_t towlower();
extern int ttylock();
extern int ttylocked();
extern char *ttyname();
extern int ttyunlock();
extern int ttywait();
extern int usleep();

extern int errno;

/*
 * Defined in tsm.c
 */
extern void onsak();
extern void onlogout();
extern void setsignals();
extern int xioctl();
extern int xtcgetattr();
extern int xtcsetattr();
extern char *xalloc();
extern void tsm();
extern void tsmlogout();
extern void tsmsakmgr();
extern int runtsh();
extern int getusername();
extern int tpath();
extern int portprotection();
extern int revoke_line();
extern int tsm_sakenabled();
extern void settsmenv();
extern void tsmbld_saklist();
extern void tsmset_sakval();
extern int tsmqry_saklist();
extern void tsmnotty();

extern nl_catd catd;
extern int multibytecodeset;
extern char *sak_default;
extern TSMSAK_LIST_t list_head;
extern TSMSAK_LIST_t list_tail;
extern int tsm_prog;
#ifdef DEBUGX
extern FILE *errfp;
#endif
extern char *Progname;
extern char *portname;
extern char *pseudopname;
extern char *synonym_list[];
extern char *portsak;
extern int nsyns;
extern int rootwndw_indx;
extern int saked;
extern int sakrcvd;
extern int logout;
extern int fd;
extern int root_trusted;
extern tpath_t user_tpath;
extern pid_t childpid;

/*
 * Defined in tsm_su_util.c
 */
extern int tsm_iscurr_console();

/*
 * Defined in tsmerr.c
 */
extern void tsm_err();
extern void pmsg();

/*
 * Defined in tsmgetty.c
 */
extern void catch_brk();
extern void saklongjmp();
extern void gettysetup();
extern void tsmgetty();
extern void gettylog();
extern void utmp_fill();

extern char *logger;
extern int brkrcvd;

/*
 * Defined in tsmlogin.c
 */
extern void getenvvars();
extern void tsmlogin();
extern int login();
extern void interruptmotd();
extern void showmotd();
extern void exitlogin();
extern void auditlogin();
extern int check_utmp();
extern void loginlog();
extern int chgportaccess();
extern void faillogin();
extern void setdefsenv();
extern void updateloginlog();
extern int tsm_port_in_utmp();
extern void tsmloginsetup();
extern void passwd_timeout();
extern void readpasswd();

/*
 * Defined in tsmports.c
 */
extern char *undev();
extern int writes();
extern char *listdup();
extern void settermtype();
extern void ontimeout();
extern void settimeout();
extern int getprotection();
extern void setprotection();
extern void getowner();
extern void setownership();
extern void setsecurity();
extern void setttymodes();
extern void getmodes();
extern void decipher();
extern void setdefaultmodes();
extern void initset();
extern int nextset();
extern void setmodes();
extern int xread();
extern int getinput();
extern void setoptionalmodes();
extern int getparms();
extern int findset_index();
extern int octatoi();
extern int convert();
extern char *tsm_parse();
extern void write_timeout();
extern void execute();

extern int login_timeout;
extern char *logger;
extern char *herald;
extern char *termtype;
extern int speedmap[];
extern struct termios logtty;
extern struct termios runtty;
extern int local_port;
extern int local_hupcl;
extern int crmod;
extern int upper;
extern int lower;
extern int eol;

#endif /* _H_TSM */

