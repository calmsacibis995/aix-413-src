static char sccsid[] = "@(#)23  1.62.1.9  src/bos/usr/sbin/tsm/tsmports.c, cmdsauth, bos41B, 9506B 2/2/95 10:02:16";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS:	convert, decipher, execute, findset_index, getinput, getmodes,
 *		getowner, getparms, getprotection, initset, isoctdigit,
 *		listdup, nextset, octatoi, ontimeout, setdefaultmodes,
 *		setmodes, setoptionalmodes, setownership, setprotection,
 *		setsecurity, settermtype, settimeout, setttymodes, tsm_parse,
 *		undev, write_timeout, writes, xread
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>
#include <errno.h>
#include <mbstr.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <userpw.h>
#include <usersec.h>
#include <sys/cfgodm.h>
#include <sys/termio.h>
#include "tsm.h"

#define isoctdigit(c) (isdigit(c) && (c) != '9' && (c) != '8')

#define CFLAG   0
#define IFLAG   1
#define LFLAG   2
#define OFLAG   3
#define NOFLAG  -1

#define MATCH           0
#define ATTR_NFOUND     -1
#define ODM_DELIM_CHARS ", ;|+"

/*
 * This is the value placed into the imap and omap fields of ODM instead of an
 * empty string when there are no input or output mapping for the port.  This
 * is so that SMIT can validate that the user has placed a correct value into
 * these fields.
 */
#define TSM_NOMAP       "none"

int login_timeout = FALSE;

/* objects from ODM */
char    *logger = NULL;
char    *herald = NULL;

static  int     timeout;
static  uid_t   owneruid    =   -1;
static  gid_t   ownergid    =   -1;
static  int     protection = 622;
static  struct  parms   *setmodep;

char    *termtype;      /* type of terminal on port */

/*
 * Default values for the ports physical configuration
 */

static  int     halfdup = 0;
static  int     speed = B9600;

/*
 * The ttymode structure is used to describe the various modes which
 * may be specified for the terminal
 */

struct   ttymode {
	char    *m_name;        /* name of mode */
	int     m_flag;         /* [CILO]FLAG value for c_[cilo]flag */
	int     m_sbits;        /* bits to set */
	int     m_cbits;        /* bits to clear */
};

/*
 * This structure is used to map the various types of parity functions
 * into the corresponding bits in the mode words.
 */

struct ttymode partab[] = {
	{ "mark",	CFLAG,	PARENB|PARODD|PAREXT,	0 },
	{ "space",	CFLAG,	PARENB|PAREXT,		PARODD },
	{ "even",	CFLAG,	PARENB,			PARODD|PAREXT },
	{ "odd",	CFLAG,	PARENB|PARODD,		PAREXT },
	{ "none",	CFLAG,	0,			PARODD|PARENB|PAREXT },
	{ "ignpar",	IFLAG,	IGNPAR,			0 },
	{ "parmrk",	IFLAG,	PARMRK,			0 },
	{ "inpck",	IFLAG,	INPCK,			0 },
	{ 0,		0,	0,			0 }
};

/*
 * This structure is used to map the names of modes into the
 *      corresponding bits in the mode word
 */

struct  ttymode modetab[] = {
	{ "parenb",	CFLAG,	PARENB,		0 },
	{ "parodd",	CFLAG,	PARENB|PARODD,	0 },
	{ "pareven",	CFLAG,	PARENB,		PARODD },
	{ "parext",	CFLAG,	PAREXT,		0 },
	{ "cs8",	CFLAG,	CS8,		CSIZE },
	{ "cs7",	CFLAG,	CS7,		CSIZE },
	{ "cs6",	CFLAG,	CS6,		CSIZE },
	{ "cs5",	CFLAG,	CS5,		CSIZE },
	{ "cstopb",	CFLAG,	CSTOPB,		0 },
	{ "hupcl",	CFLAG,	HUPCL,		0 },
	{ "clocal",	CFLAG,	CLOCAL,		0 },
	{ "cread",	CFLAG,	CREAD,		0 },
	{ "b0",		CFLAG,	B0,		CBAUD },
	{ "b50",	CFLAG,	B50,		CBAUD },
	{ "b75",	CFLAG,	B75,		CBAUD },
	{ "b110",	CFLAG,	B110,		CBAUD },
	{ "b134",	CFLAG,	B134,		CBAUD },
	{ "b150",	CFLAG,	B150,		CBAUD },
	{ "b200",	CFLAG,	B200,		CBAUD },
	{ "b300",	CFLAG,	B300,		CBAUD },
	{ "b600",	CFLAG,	B600,		CBAUD },
	{ "b1200",	CFLAG,	B1200,		CBAUD },
	{ "b1800",	CFLAG,	B1800,		CBAUD },
	{ "b2400",	CFLAG,	B2400,		CBAUD },
	{ "b4800",	CFLAG,	B4800,		CBAUD },
	{ "b9600",	CFLAG,	B9600,		CBAUD },
	{ "b19200",	CFLAG,	B19200,		CBAUD },
	{ "b38400",	CFLAG,	B38400,		CBAUD },
	{ "exta",	CFLAG,	EXTA,		CBAUD },
	{ "extb",	CFLAG,	EXTB,		CBAUD },
	{ "ignbrk",	IFLAG,	IGNBRK,		0 },
	{ "brkint",	IFLAG,	BRKINT,		0 },
	{ "ignpar",	IFLAG,	IGNPAR,		0 },
	{ "parmrk",	IFLAG,	PARMRK,		0 },
	{ "inpck",	IFLAG,	INPCK,		0 },
	{ "istrip",	IFLAG,	ISTRIP,		0 },
	{ "inlcr",	IFLAG,	INLCR,		0 },
	{ "igncr",	IFLAG,	IGNCR,		0 },
	{ "icrnl",	IFLAG,	ICRNL,		0 },
	{ "iuclc",	IFLAG,	IUCLC,		0 },
	{ "ixon",	IFLAG,	IXON,		0 },
	{ "ixany",	IFLAG,	IXANY,		0 },
	{ "ixoff",	IFLAG,	IXOFF,		0 },
	{ "imaxbel",	IFLAG,	IMAXBEL,	0 },
	{ "isig",	LFLAG,	ISIG,		0 },
	{ "icanon",	LFLAG,	ICANON,		0 },
	{ "xcase",	LFLAG,	XCASE,		0 },
	{ "echo",	LFLAG,	ECHO,		0 },
	{ "echoe",	LFLAG,	ECHOE,		0 },
	{ "echok",	LFLAG,	ECHOK,		0 },
	{ "echonl",	LFLAG,	ECHONL,		0 },
	{ "noflsh",	LFLAG,	NOFLSH,		0 },
	{ "opost",	OFLAG,	OPOST,		0 },
	{ "olcuc",	OFLAG,	OLCUC,		0 },
	{ "onlcr",	OFLAG,	ONLCR,		0 },
	{ "ocrnl",	OFLAG,	OCRNL,		0 },
	{ "onocr",	OFLAG,	ONOCR,		0 },
	{ "onlret",	OFLAG,	ONLRET,		0 },
	{ "ofill",	OFLAG,	OFILL,		0 },
	{ "ofdel",	OFLAG,	OFDEL,		0 },
	{ "cr0",	OFLAG,	CR0,		CRDLY },
	{ "cr1",	OFLAG,	CR1,		CRDLY },
	{ "cr2",	OFLAG,	CR2,		CRDLY },
	{ "cr3",	OFLAG,	CR3,		CRDLY },
	{ "tab0",	OFLAG,	TAB0,		TABDLY },
	{ "tab1",	OFLAG,	TAB1,		TABDLY },
	{ "tab2",	OFLAG,	TAB2,		TABDLY },
	{ "tab3",	OFLAG,	TAB3,		TABDLY },
	{ "nl0",	OFLAG,	NL0,		NLDLY },
	{ "nl1",	OFLAG,	NL1,		NLDLY },
	{ "ff0",	OFLAG,	FF0,		FFDLY },
	{ "ff1",	OFLAG,	FF1,		FFDLY },
	{ "vt0",	OFLAG,	VT0,		VTDLY },
	{ "vt1",	OFLAG,	VT1,		VTDLY },
	{ "bs0",	OFLAG,	BS0,		BSDLY },
	{ "bs1",	OFLAG,	BS1,		BSDLY },
	{ "halfdup",	LFLAG,	0,		ECHO|ECHOE|ECHOK|ECHONL },
	{ "fulldup",	NOFLAG,	0,		0 },
	{ "echoctl",	LFLAG,	ECHOCTL,	0 },
	{ "echoprt",	LFLAG,	ECHOPRT,	0 },
	{ "echoke",	LFLAG,	ECHOKE,		0 },
	{ "pending",	LFLAG,	PENDIN,		0 },
	{ "iexten",	LFLAG,	IEXTEN,		0 },
	{ "flusho",	LFLAG,	FLUSHO,		0 },
	{ "tostop",	LFLAG,	TOSTOP,		0 },
	{ 0,		0,	0,		0 }
};

struct ttymode bpctab[] = {
	{ "8",	CFLAG,	CS8,	CSIZE },     /* Regular characters       */
	{ "7",	CFLAG,	CS7,	CSIZE },     /* Stripped characters      */
	{ "6",	CFLAG,	CS6,	CSIZE },     /* 60 bit machines?         */
	{ "5",	CFLAG,	CS5,	CSIZE },     /* Baudot ?                 */
	{ 0,	0,	0,	0 }
};

struct ttymode stoptab[] = {
	{ "1",	CFLAG,	0,	CSTOPB },     /* 1 stop bit (default)     */
	{ "2",	CFLAG,	CSTOPB,	0 },          /* 2 stops bits             */
	{ 0,	0,	0,	0 }
};

struct ttymode flow_disptab[] = {
	{ "xon",        IFLAG,  IXON|IXOFF,     0 },    /* enable start/stop */
	{ "none",         IFLAG,  0,              IXON|IXOFF },     /* disable */
	{ 0,		0,	0,		0 }
};


/*
 * the settings structure describes the values of all parameters
 *      It is indexed into by the number (above) corresponding
 *      to the desired parameter
 *
 */

struct  parms {
	char *p_name;           /* Ascii name  of the attribute   */
	char *p_value;          /* Value of the attribute  */
	int p_termios;          /* termios.c_cc[index] */
	int p_flags;            /* flags indicating which parameters to set */
	#               define P_SETR   0x01    /* setttymodes, runmodes */
	#               define P_SETL   0x02    /* setttymodes, logmodes */
	#               define P_RUN    0x04    /* set runmodes */
	#               define P_LOG    0x08    /* set logmodes */
	int (*p_action)();    /* function for conversion or setting */
	struct ttymode *p_ttytab;       /* pointer to mode table */
} settings[] = {
	/* =SETS= is a link to the next setting (not used with odm)
         * logmodes and runmodes may be overridden by explicit values
         */

	/* ODM name, value, tty char, flags,   function,      parameter */
	{ "=SETS=",     0, 0,           0,              NULL,   NULL },
	{ "herald",     0, 0,           0,              NULL,   NULL },

	{ "speed",      0, 0,           0,              NULL,   NULL },
	{ "logmodes",   0, 0,           0,              NULL,   NULL },
	{ "runmodes",   0, 0,           0,              NULL,   NULL },
	{ "logger",     0, 0,           0,              NULL,   NULL },
	{ "timeout",    0, 0,           0,              NULL,   NULL },
	{ "term",       0, 0,           0,              NULL,   NULL },
	{ "protection", 0, 0,           0,              NULL,   NULL },
	{ "owner",      0, 0,           0,              NULL,   NULL },
	{ "sak",        0, 0,           0,              NULL,   NULL },
	{ "imap",       0, 0,           0,              NULL,   NULL },
	{ "omap",       0, 0,           0,              NULL,   NULL },
	{ "synonym",    0, 0,           0,              NULL,   NULL },
	{ "flow_disp",  0, 0,           P_SETR|P_SETL,  (int (*)())setttymodes,flow_disptab },

	{ "parity",     0, 0,           P_SETR|P_SETL,  (int (*)())setttymodes,partab },
	{ "erase",      0, VERASE,      P_RUN|P_LOG,    convert,NULL },
	{ "kill",       0, VKILL,       P_RUN|P_LOG,    convert,NULL },
	{ "intr",       0, VINTR,       P_RUN|P_LOG,    convert,NULL },
	{ "quit",       0, VQUIT,       P_RUN|P_LOG,    convert,NULL },
	{ "eol",        0, VEOL,        P_RUN,          convert,NULL },
	{ "eof",        0, VEOF,        P_RUN,          convert,NULL },
	{ "min",        0, VMIN,        P_LOG,          octatoi,NULL },
	{ "time",       0, VTIME,       P_LOG,          octatoi,NULL },
	{ "start",      0, VSTART,      P_RUN|P_LOG,    convert,NULL },
	{ "stop",       0, VSTOP,       P_RUN|P_LOG,    convert,NULL },
	{ "susp",       0, VSUSP,       P_RUN|P_LOG,    convert,NULL },
	{ "eol2",       0, VEOL2,       P_RUN|P_LOG,    convert,NULL },
	{ "dsusp",      0, VDSUSP,      P_RUN|P_LOG,    convert,NULL },
	{ "reprint",    0, VREPRINT,    P_RUN|P_LOG,    convert,NULL },
	{ "discard",    0, VDISCRD,     P_RUN|P_LOG,    convert,NULL },
	{ "werase",     0, VWERSE,      P_RUN|P_LOG,    convert,NULL },
	{ "lnext",      0, VLNEXT,      P_RUN|P_LOG,    convert,NULL },
	{ "stops",      0, 0,           P_SETR|P_SETL,  (int (*)())setttymodes,stoptab },
	{ "bpc",        0, 0,           P_SETR|P_SETL,  (int (*)())setttymodes,bpctab },
	{ NULL,         0, 0,           0,              NULL,NULL }
};

/*
 * Indexes into the settings structure for the various parameters
 * Must correspond with the positions in the array above.
 */

#define SETS            0
#define HERALD          1
#define SPEED           2
#define LOGMODES        3
#define RUNMODES        4
#define LOGGER          5
#define TIMEOUT         6
#define TERM            7
#define PROTECT         8
#define OWNER           9
#define SAK             10
#define IMAP            11
#define OMAP            12
#define SYNONYM         13
#define FLOW_DISP       14

#define ODM_START       SPEED   /* starting index for ODM stuff */

/*
 * This array is used to map speeds into UNIX speed codes
 */

int speedmap[] = {
	0,      B0,
	50,     B50,
	75,     B75,
	110,    B110,
	134,    B134,
	150,    B150,
	200,    B200,
	300,    B300,
	600,    B600,
	1200,   B1200,
	1800,   B1800,
	2400,   B2400,
	4800,   B4800,
	9600,   B9600,
	19200,  B19200,
	38400,  B38400,
	-1,     -1
};

struct  termios logtty;         /* Termio modes in use during login */
struct  termios runtty;         /* Termio modes set after login */
int local_port;                 /* true if CLOCAL is set in logmodes */
int local_hupcl;                /* true if HUPCL is set in logmodes */

int     crmod;                  /* Lines may terminate with CR */
int     upper;                  /* Upper case characters in input */
int     lower;                  /* Lower case characters in input */
int     eol;                    /* End of Line                    */

/*
 * The basename concept does not sit well with multiplexed devices.  Therefore,
 * we will therefore strip "/dev/" from the front of the device name and
 * consider that the 'basename' of the device (i.e. "/dev/pts/0" becomes
 * "pts/0").
 */
char *
undev(s)
register char *s;
{
        register char *p;

        return((p = strstr(s, TSM_DEV)) ? p + strlen(TSM_DEV) : s);
}

/*
 *  NAME:       writes
 *
 *  FUNCTION:   write a variable length string
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              Zero on success, non-zero otherwise
 */

#define WRITE_TIME 15

int
writes (fd, s, exp_bywrt)
int     fd;                /* File descriptor to write to */
char    *s;                /* String to write             */
int     exp_bywrt;	   /* expected # of bytes to write */
{
	int     act_bywrt; /* Actual   # of bytes written  */
	int     return_value = 0; /* value to return */
	int t_entry;                    /* time alarm has on entry */
	int t_left;                     /* time left after write */
	void (*o_sig)();                /* old signal catcher */
	char err_msg[255];

	errno = 0;

	o_sig = signal(SIGALRM, write_timeout);
	t_entry = alarm(0);
	alarm(WRITE_TIME);

	act_bywrt = write (fd , s, exp_bywrt);


	t_left = alarm(0);
	/* If alarm was set, then set it back up */
	if (t_entry) {
		t_entry -= WRITE_TIME - t_left;
		if (t_entry < 1)
			t_entry = 1;
		(void) signal(SIGALRM, o_sig);
		(void) alarm(t_entry);
	}

	if (act_bywrt == exp_bywrt)   {
		return_value = 0;
	}
	else if (act_bywrt == -1 && errno == EIO)   {
		sprintf(err_msg, MSGSTR(M_WRITEERR, DEF_WRITEERR), ttyname(fd));
		syslog(LOG_NOTICE, err_msg);
		exit (0);
	}
	else {
		return_value = -1;
	}
	return(return_value);
}

/*
 * NAME: listdup
 *
 * FUNCTION:    duplicate a list and return a pointer to malloc'd memory
 *
 * RETURN VALUE:
 *      A pointer to mallocated memory or NULL on failure
 */

char *
listdup (char *l)
{
	char    *cp;
	int     i;

	if (! l)
		return 0;

	for (i = 0;l[i] || l[i + 1];i++)
		;

	if (! (cp = (char*) malloc (i + 2)))
		return 0;

	memset (cp, 0, i + 2);
	memcpy (cp, l, i);
	return cp;
}

/*
 *  NAME:       settermtype
 *
 *  FUNCTION:   Place the terminal type environment variable into the given
 *		string.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *	Nothing.
 */

void
settermtype (input)
char    *input;
{
	if (! termtype)
		return;

	strcpy (input, "TERM=");
	strcat (input, termtype);
}

/*
 *  NAME:       ontimeout
 *
 *  FUNCTION:   set global indicating user defined occurred without
 *              a successful login
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value.
 */

void
ontimeout()
{
	login_timeout = TRUE;
}

/*
 *  NAME:       settimeout
 *
 *  FUNCTION:   set login timeout alarm
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value.
 */

void
settimeout()
{
	login_timeout = FALSE;
	if (timeout) {
		signal (SIGALRM , ontimeout);
		alarm (timeout);
	}
}

/*
 *  NAME:       getprotection
 *
 *  FUNCTION:   convert mode string to mode value
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              Converts the octal or mode string passwd as string
 *              into an integer representation.
 */

int
getprotection (string)
char    *string;
{
	char    *modes = "rwxrwxrwx";
	int     i;
	int     perms;
	int     protection = 0;

	/*
         * if string is numeric, get modes directly
         */

	if (*string >= '0'  &&  *string <= '7')         {
		return (octatoi(string));
	}

	/*
         * Should be a nine character string containing "rwxrwxrwx" type
         * permissions.
         */

	for (i = 0, perms = 0400;i < 9 && string[i];i++, perms >>= 1) {
		if (string[i] == modes[i]) {
			protection |= perms;
		}

	}

	return protection;
}

/*
 *  NAME:       setprotection
 *
 *  FUNCTION:   set file access permissions on terminal file
 *
 *  NOTES:
 *              "protection" is set during port setup to be the
 *              permissions for this port.  If the default mode
 *              is being used this function does nothing.  Only
 *              if an overriding permission is specified does
 *              anything happen.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
setprotection ()
{
	if (protection)
		portprotection (protection, portname);
}

/*
 *  NAME:       getowner
 *
 *  FUNCTION:   initialize port owner and group
 *
 *  NOTES:
 *              This function fetches the default owner and group values
 *              for this port.  It does not actually change the port modes.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
getowner (string)
char    *string;
{
	struct passwd *pw;

	if (! string)
		return;

	if (*string >= '0'  &&  *string <= '9')
		pw = getpwuid (strtol (string, NULL, 10));
	else
		pw = getpwnam (string);

	if (pw) {
		owneruid = pw->pw_uid;
		ownergid = pw->pw_gid;
	}
}

/*
 *  NAME:       setownership
 *
 *  FUNCTION:   set the default owner and group for the port
 *
 *  NOTES:
 *              This routines takes the default UID and GID values and
 *              changes the port owner and group accordingly.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
setownership ()
{
	uid_t   uid = 0;
	gid_t   gid = 0;
	int     ind;

	/*
         * Change the ownership on the port  (and any synonyms )if there
         * is an overriding port owner and group in ODM.
         * Trusted path overrides this - root
         * must own the port on the trusted path.
         */

	if (! sakrcvd && owneruid != -1)
		uid = owneruid;

	if (! sakrcvd && ownergid != -1)
		gid = ownergid;

	(void) chown (portname, uid, gid);

	/*
         * Change the ownership on all of the synonyms, if any
         */

	for (ind = 0; ind < nsyns; ind++) {
		(void) chown (synonym_list[ind], uid, gid);
	}
}

/*
 *  NAME:       setsecurity
 *
 *  FUNCTION:   Turn on SAK processing for this port if required.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
setsecurity ()
{
	int     sakval = TCSAKOFF;

	/*
         * Must do a frevoke() if :
         *      1 - SAK requested in /etc/security/login.cfg.
         *      2 - If SAK was ever enabled on this port.
         *      In addition, set the terminal trustedness if sak is requested
         */

	if (portsak && strcmp (portsak, "true") == 0) {
		sakval = TCSAKON;
	}
	else {
		portsak = NULL;
	}

	/*
         * If SAK has never been received do not make the terminal
         * trusted.  The user MUST press SAK to get on the trusted
         * path.
         */

	xioctl (INFD, TCSAK, &sakval);
}

/*
 *  NAME:       setttymodes
 *
 *  FUNCTION:   set and clear bits in the termio structure according to
 *              a user supplied string.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value.  The termio structure is modified in
 *              place.
 */

void
setttymodes (string, tab, tty)
char    *string;
struct  ttymode *tab;
struct  termios *tty;
{
	char    *s;
	char    *temp_string;
	int     i;
	int     clear;
	int     set;

	/*
         * Search for each mode specified in the given string in the
         * mode table.  If the mode is found, set the appropriate bits
         * in the tty structure.
         */

	temp_string = (char *) malloc (strlen (string) + 1);
	strcpy (temp_string , string);
	while (temp_string) {

		/*
                 * Search for the mode and NULL terminate so it can
                 * be searched for.
                 */

		s = tsm_parse(&temp_string);

		for(i = 0;tab[i].m_name; i++) {
			if (strcmp (tab[i].m_name, s) == 0)
				break;
		}

		if (! tab[i].m_name)
			continue;

		/*
                 * Each table entry contains a group of bits to clear and
                 * a group of bits to set.  The clear bits are cleared first
                 * and then the set bits are added in.
                 */

		clear = tab[i].m_cbits;
		set = tab[i].m_sbits;

		/*
                 * Find the appropriate element in the termio structure and
                 * make the requested change.
                 */

		switch (tab[i].m_flag) {
		case IFLAG:
			tty->c_iflag = (tty->c_iflag & ~clear) | set;
			break;
		case OFLAG:
			tty->c_oflag = (tty->c_oflag & ~clear) | set;
			break;
		case LFLAG:
			tty->c_lflag = (tty->c_lflag & ~clear) | set;
			break;
		case CFLAG:
			tty->c_cflag = (tty->c_cflag & ~clear) | set;
			break;
		}
	}
}


/*
 *  NAME:       getmodes
 *
 *  FUNCTION:   initialize run modes and login modes values for termio
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value.  Modifies a termio structure in place
 *              according to a user supplied string.
 */

void
getmodes (string, tty)
char    *string;
struct  termios *tty;
{
	if (string) {
		tty->c_cflag = 0;
		tty->c_iflag = 0;
		tty->c_lflag = 0;
		tty->c_oflag = 0;

		setttymodes (string, modetab, tty);
	}
}

/*
 *  NAME:       decipher
 *
 *  FUNCTION:   sort attributes into meaningful values
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value.
 */

void
decipher (parm)
struct  parms   *parm;
{
	char    *s;
	struct parms *pparm;

	/*
         * Check if initial group of setting is around
         * if so these become out initial settings else
         * we start with TSM predefined defaults
        */

	if (s = parm[LOGMODES].p_value) {
		getmodes(s,&logtty);
		/*
                 * LOGMODES cannot be cycled (upon reading of a break from the
                 * port * - mostly used for autobauding) therefore lets set
                 * them to NULL to prevent reuse on the next cycle.
                 */
		parm[LOGMODES].p_value = NULL;
	}

	if (s = parm[RUNMODES].p_value) {
		getmodes(s,&runtty);
		/*
                 * RUNMODES cannot be cycled either.
                 */
		parm[RUNMODES].p_value = NULL;
	}


	/* set up some defaults */
	{
		char	*args[3];

		args[0] = "/usr/bin/setmaps";
		args[1] = "-c";
		args[2] = NULL;
		execute(args);
	}
	protection = 0;
	ownergid = -1;
	owneruid = -1;

	/*
         * Get the values for all the tty parameters.
         * The logtty entry is used during getty and login processing
         * and operates in rare mode.  The runtty entry will be set after
         * login authentication has been completed.
         */
	for (pparm=parm+ODM_START; pparm->p_name != NULL; pparm++) {
		if ((s = tsm_parse(&pparm->p_value)) == NULL)
			continue;
		switch (pparm-parm) {
			int i, j;
		case SPEED:
			/* Locate the correct speed in the baudrate
                         * table.  It is a  fatal error for an illegal
                         * speed to be specified.
                         */
			j = (int) strtol (s, NULL, 10);

			for (speed=-1, i=0; speedmap[i] != -1; i+=2) {
				if (j == speedmap[i]) {
					speed = speedmap[i + 1];
					break;
				}
			}
			if (speed == -1) {
				speed = B300;
			}
			break;
		case TERM:
			/* If terminal type is explicitly set then
                         * override initial setting obtained from
                         * termdef.
                         */
			termtype = s;
			break;
		case PROTECT:
			/* If protection type is explicitly set then
                         * override initial setting.
                         */
			protection = getprotection (s);
			break;
		case OWNER:
			/* Check if a default owner is specifed
                         * if so set the owner?id fields to that id
                         * else reinitialize them (Last setting may
                         * have had a default owner
                        */
			getowner (s);
			break;
		case IMAP:
			if (strcmp (s , TSM_NOMAP) != MATCH)
			{
				char	*args[4];
				char	argbuf[64];

				args[0] = "/usr/bin/setmaps";
				args[1] = (*s == '/') ? "-I" : "-i";
				strcpy(argbuf, s);
				strcat(argbuf, ".in");
				args[2] = argbuf;
				args[3] = NULL;
				execute(args);
			}
			break;
		case OMAP:
			if (strcmp (s , TSM_NOMAP) != MATCH)
			{
				char	*args[4];
				char	argbuf[64];

				args[0] = "/usr/bin/setmaps";
				args[1] = (*s == '/') ? "-O" : "-o";
				strcpy(argbuf, s);
				strcat(argbuf, ".out");
				args[2] = argbuf;
				args[3] = NULL;
				execute(args);
			}
			break;
		case LOGGER:
			logger = s;
			break;
		case TIMEOUT:
			timeout = (int) strtol (s, NULL, 10);
			break;
		case FLOW_DISP:
			/* If there is more than one parameter, then undo
			 * parsing of the original string before proceeding
			 */
			if (*pparm->p_value != NULL) {
				pparm->p_value = pparm->p_value -1;
				*pparm->p_value = ',';
			}
			(*pparm->p_action)(s, pparm->p_ttytab, &runtty);
			(*pparm->p_action)(s, pparm->p_ttytab, &logtty);

			break;
		default:
			/* one of many tty modes */
			if (pparm->p_flags & (P_SETR|P_SETL)) {
				if (pparm->p_flags & P_SETR) {
					(*pparm->p_action)
					    (s, pparm->p_ttytab,
					    &runtty);
				}
				if (pparm->p_flags & P_SETL) {
					(*pparm->p_action)
					    (s, pparm->p_ttytab,
					    &logtty);
				}
			}
			else if (pparm->p_flags & (P_LOG|P_RUN)) {
				int rc;
				if ((rc = (*pparm->p_action) (s)) != -1) {
					int tio = pparm->p_termios;
					if (pparm->p_flags & P_LOG)
						logtty.c_cc[tio] = rc;
					if (pparm->p_flags & P_RUN)
						runtty.c_cc[tio] = rc;
				}
			}
			break;
		}
	}
}


/*
 *  NAME:       setdefaultmodes
 *
 *  FUNCTION:   initialize the port to some reasonable default value
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
setdefaultmodes()
{
	/*
	 * If the herald did not exist in /etc/security/login.cfg, fetch the
	 * default from the message catalog.
	 */

	if (herald == NULL)
	{
		if(tsm_iscurr_console(portname, NULL))
			herald = MSGSTR(M_HERALD, DEF_HERALD);
		else
			herald = MSGSTR(M_CONHERALD, DEF_CONHERALD);
	}

	xtcgetattr(INFD, &logtty);

	/*
         * IN getty we want to force log and runmodes
         * login leaves logmodes and runmodes
         * alone for rlogind and telnetd.
         */

	if (tsm_prog == TSMGETTY) {
		logtty.c_cc[VINTR]   = CINTR;
		logtty.c_cc[VQUIT]   = CQUIT;
		logtty.c_cc[VERASE]  = CERASE;
		logtty.c_cc[VKILL]   = CKILL;
		logtty.c_cc[VEOF]    = CEOF;
		logtty.c_cc[VEOL]    = CNUL;
		runtty = logtty;

		/*
                * Set the default post-login mode values.  These flags will be
                * set after the user is authenticated.  These are the values the
                * user will have initially.
                */

		runtty.c_cflag &= CLOCAL;
		runtty.c_cflag |= HUPCL|CREAD|CS8;
		runtty.c_iflag = IXON|BRKINT|ICRNL|IMAXBEL;
		runtty.c_oflag = OPOST|TAB3|ONLCR;
		runtty.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE|IEXTEN;

		/*
		 * Grab the terminal type from the device driver
		 */

		termtype = termdef (INFD , 't');


		logtty.c_cflag &= CLOCAL;
		logtty.c_cflag |= HUPCL|CREAD|CS8;
		logtty.c_iflag = IXON;
		logtty.c_oflag = OPOST|TAB3;
		logtty.c_lflag = ECHOE|ECHOK;
		logtty.c_cc[VMIN] = 1;
		logtty.c_cc[VTIME] = 0;
	} else {
		runtty = logtty;
	}
}

/*
 *  NAME:       initset, nextset
 *
 *  FUNCTION:   rotate through sets of termio modes
 *
 *  NOTES:
 *              initset() sets the current set to be the first set of
 *              modes in the list.  nextset() selects the next collection
 *              of flags, unless the end has been reached in which case
 *              it goes back to the beginning.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value.
 */

void
initset ()
{
	int i;
	int count;
	char *s;

	/*
	 * Determine number of sets we are rotating
	 */

	setmodep = settings;
	setmodep[SETS].p_value = (char *)1;
	for (i = ODM_START; setmodep[i].p_name != NULL; i++) {
		if (i != RUNMODES && i != LOGMODES)  {
			s = strpbrk (setmodep[i].p_value, ODM_DELIM_CHARS);
			for (count = 1; (s != NULL && s+1 != NULL); count++) {
				s = strpbrk (s + 1, ODM_DELIM_CHARS);
			}
			if (count > (int)setmodep[SETS].p_value)  {
				setmodep[SETS].p_value = (char *)count;
			}
		}
	}
}

int
nextset ()
{
	int return_value;

	/*
         * If there are more sets of settings incrment the
         * modes to point at the next set and return TRUE
         * indicating there is another set
         * Else return FALSE saying no more sets
         */

	setmodep[SETS].p_value--;
	if ((int)setmodep[SETS].p_value <= 0) {
		return_value = FALSE;
	}
	else {
		return_value = TRUE;
	}

	return (return_value);
}

/*
 *  NAME:       setmodes
 *
 *  FUNCTION:   Convert mode strings into termio structures
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
setmodes()
{
	/*
         * Convert the mode strings into termio structures and set
         * up the baud rates.
         */

	decipher (setmodep);

	if (speed) {
		logtty.c_cflag &= ~CBAUD;
		logtty.c_cflag |= speed;

		runtty.c_cflag &= ~CBAUD;
		runtty.c_cflag |= speed;
	}

	/* make sure BRKINT gets set in the logmodes */
	logtty.c_iflag |= BRKINT;
}

/*
 * NAME:	xread
 *
 * FUNCTION:	read() with error checking.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS:
 *	number of bytes read.
 */

int
xread (fd, cp, len)
int     fd;
char    *cp;
int     len;
{
	int     i;
	char err_msg[255];

	if ((i = read (fd, cp, len)) < 0) {
		if (errno == EINTR)
			return -1;
		sprintf(err_msg, MSGSTR(M_READERR, DEF_READERR), ttyname(fd));
		syslog(LOG_NOTICE, err_msg);
		DPRINTF (("read return <= 0 errno = %d\n", errno));
		exit (0);
	}
	return i;
}

/*
 *  NAME:       getinput
 *
 *  FUNCTION:   read a line of input prompting and performing line editing
 *
 *  NOTES:
 *              This is a super-simple line input routine.  It should only
 *              be used for the simplest of input requirements.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              Non-zero on successful input, zero otherwise
 */

int
getinput (buf, len, prompt)
char    *buf;
int     len;
char    *prompt;
{
	int     canon = 0;              /* Canonical processing enabled */
	struct  termios curtty;         /* Current termio values */
	char    c;
	int	nread;
	char    *cp;                    /* Pointer into single char buf */

	/*
         * Find the ICANON flag and see if line editing must be
         * handled in here.
         */

	xtcgetattr (0, &curtty);
	canon = (curtty.c_lflag & ICANON) != 0;

begin:
	cp = buf;

	/*
         * Output the prompt and reset the input mode flags.  The meaning
         * of these flags is as follows:
         *
         *      crmod - a carriage return was seen
         *      upper - an upper case character was seen
         *      lower - a lower case character was seen
         */

	writes (OUTFD, prompt, strlen (prompt));

	crmod = upper = lower = 0;

	/*
	 * check to see if break already received. if there has
	 * been, then decrement the counter and act like we found
	 * one on the queue.
	 */
	if (brkrcvd > 0) {
		DPRINTF(("in getinput and brkrcvd=%d\n",brkrcvd));
		brkrcvd--;
		return(0);
	}

	while (1)
	{
		if ((nread = xread (INFD, &c, 1)) < 0)
		{
			if (saked && !login_timeout)
			{
				saked = 0;
				continue;
			}
			return 0;
		}

		/*
		 * If we read zero bytes, then we are in canonical mode and the
		 * user typed EOF.  Therefore, we exit.
		 */

		if (nread == 0)
			tsmlogout();

		/*
                 * Check for BREAK and other framing errors.  If a
                 * BREAK is received [ and BRKINT and IGNBRK reset ]
                 * 'c' will be zero.  Just return and let autobauding
                 * or whatever take place.
                 */

		if (c == 0)
			return 0;

		/*
                 * Check for EOF; exit if it is.
                 */
		if (c == runtty.c_cc[VEOF])
			tsmlogout();

		/*
                 * This routine can be called with the terminal in raw
                 * or cooked mode.  If raw then the line editing functions
                 * need to be handled here.
                 */

		if (! canon)
		{
			/*
                         * Handle ERASE character processing.
			 *
			 * When erasing a multibyte character, ensure you erase
 			 * all the bytes that makeup the character, not just
			 * one byte. If the character to erase is a multibyte
			 * character, then 
			 *	1) determine the number of bytes 
 			 *	2) put spaces in all of them, put the cursor at
			 * 	   the first byte of the erase character.
			*/

			if (c == logtty.c_cc[VERASE])
			{
				if (cp > buf)
				{
					int i;
					int j;
					char *p;

					p = buf;
					do
					{
						i = mblen(p, MB_CUR_MAX);
						if(p + i == cp) break;
						p += i;
					}while(p < cp);

					cp = p;

					if (! halfdup)
						if (logtty.c_lflag & ECHOE)
							for (j=0;
							     j<mbswidth(cp, i);
							     j++)
								writes (OUTFD,
									"\b \b",
									3);
						else 
						{
							writes(OUTFD, "\\", 1);
							for (j=0;j<i;j++)
								writes (OUTFD,
									cp+j,
									1);
						}
				}
				continue;
			}

			/*
	                 * Handle KILL character processing
			 */

			if (c == logtty.c_cc[VKILL])
			{
				if (! halfdup && (logtty.c_lflag & ECHOK))
					writes (OUTFD, "\r\n", 2);
				goto begin;
			}

			if (! halfdup)
				writes (OUTFD, &c, 1);
		}

		/*
                 * Was this the end of the line?  If so, break out of
                 * the input loop and don't add this character to the
                 * end of the string.
                 */

		if (c == '\n' || c == '\r')
		{
			break;
		}

		/*
                 * Add the character, checking for overflow.
                 * On overflow, exit and bail out.
                 *
                 */

		if ((1 + cp) >= (buf + len))
		{
			writes (OUTFD, "\r\n", 2);
			return 0;
		}
		*cp++ = c;
	}

	/*
         * NULL terminate the string and figure out what character
         * was the terminator.  crmod will have to be set if '\r'
         * was the terminator.
         */

	*cp = '\0';
	if (c == '\r')
		crmod++;
	if (!halfdup && !canon)
		if (c == '\r')
			writes (OUTFD, "\n", 1);
		else
			writes (OUTFD, "\r", 1);

	return 1;
}

/*
 *  NAME:       setoptionalmodes
 *
 *  FUNCTION:   optionally set UPPERCASE, crmod, etc. modes.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              No return value
 */

void
setoptionalmodes (char *username)
{
	/*
         * figure out if the optional modes are needed
         *   philosophy on optional modes is:
         *      if you NEED it, you get it
         *         otherwise, you get what you ask for
         */

	if (crmod) {
		runtty.c_iflag |= ICRNL;
		runtty.c_oflag |= ONLCR;
	}

	if (upper && !lower) {

	    /*
	     * Some users may have legitimate all-uppercase usernames.
	     */

          if (! username || ! _getpwnam_shadow (username,0)) {
			printf(MSGSTR(M_ALLCAPS, DEF_ALLCAPS));
			runtty.c_iflag |= IUCLC;
			runtty.c_oflag |= OLCUC;
			runtty.c_lflag |= XCASE;
		}
	}

	/*
         * Set the [ possibly ] changed termio modes
         */

	DEBUGMODES ("setoptionalmodes: tcsetaf ", &runtty);

	xtcsetattr(INFD, TCSANOW, &runtty);
}

/*
 *  NAME:       getparms
 *
 *  FUNCTION:   look up the parameters associated with the current port
 *
 *  NOTES:
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              Zero if no parameters, non-zero if any parameters
 */

int
getparms ()
{
	/*
	 * Temp storage area for conversion of portname to ODM criteria.
	 */
	char    stpdportnm[64];
	/*
	 * Stanza attribute retrieval routine.
	 */
	char    * afgetatr();
	/*
	 * Scratch pointer for conversion of portname to ODM style of portname.
	 */
	char    *tptr, *tptr1;
	/*
	 * Index counters.
	 */
	int     i, ind;
	/*
	 * Number of attributes returned by getattr.
	 */
	int     how_many;
	/*
	 * Pointer to ODM list.
	 */
	struct CuAt *parmlist;
	/*
	 * Pointer used to traverse ODM list.
	 */
	struct CuAt *parm_entry;
	/*
	 * Pointer to a stanza file object.
	 */
	AFILE_t af;
	/*
	 * Routine to get a stanza file record.
	 */
	ATTR_t afgetrec();
	/*
	 * Pointers to stanza file objects.
	 */
	ATTR_t ar;
	ATTR_t ar1;

	/*
         * Initialize Object Data Manager (odm)
         */

	odm_initialize();

	/*
         * Load up the stpdportnm parameter causing us to only retrieve
         * parameters for our tty
         * Remove the first "/dev/" string and anything that preceeds it,
         * and then lop it off at the first '/'
         */

	strcpy(stpdportnm , portname);

	/*
         * Strip "/dev/"
         */

	tptr = strstr (stpdportnm , TSM_DEV);
	if (tptr != NULL) {
		strcpy (stpdportnm , tptr + strlen (TSM_DEV));
	}

	/*
         * Lop it off at first '/'.  This causes attributes for
         * specific channels on a multiplexed device to be
         * retrieved from the base device if attributes exist
         * for the head of the multiplexed device
         */

	if (tptr = strchr(stpdportnm, '/')) {
		*tptr = '\0';
	}

	/*
         * Retrieve list of port settings from Odm
         * Call getattr instead of an ODM routine because
         * the config people play games with the database
         * gettattr is supplied by them and will get all the
         * attributes no matter where they are hidden
         * With true set we don't need an attribute name
         * as we are requesting them all
         */

	parmlist = getattr ( stpdportnm , NULL , TRUE , &how_many);

	switch ((int) parmlist) {
	case 0:
	case -1:
		break;

		/*
		 * Load the port settings into the
		 * setting structure
		 */

	default :
		for (parm_entry = parmlist, i=0;
		    i < how_many; i++, parm_entry++) {
			ind = findset_index(parm_entry -> attribute);
			if (ind != ATTR_NFOUND) {
				settings[ind].p_value =
				    listdup (parm_entry -> value);

			}
		}
		break;
	}

	{
		/*
                 * This the first part of a patch to fix the HUPCL/CLOCAL
		 * problem (defect 7035).  The first open of a port will
		 * automatically reset the CLOCAL flag; so if CLOCAL is set in
		 * LOGMODES, we open the port non-blocking, set the CLOCAL
		 * flag in the device driver, and then do a blocking open of
		 * the port.  This way, we end up with an open file descriptor
		 * to the port with CLOCAL set appropriately; CLOCAL is also
		 * set correctly during the blocking open.  Here, we are just
		 * checking to see if CLOCAL and HUPCL are set in the logmodes
		 * for this port; the actual open happens in tsmgetty().
		 */
		struct termios t;

		if (settings[LOGMODES].p_value) {
			getmodes(settings[LOGMODES].p_value, &t);
			local_port = t.c_cflag & CLOCAL;
			local_hupcl = t.c_cflag & HUPCL;
		} else {
			local_port = local_hupcl = 0;
		}
	}

	/*
         * Terminate odm processing
         */

	odm_terminate();

	/*
         * Open the login configuration file.  It is used to get
         * the port configuration, including SAK, login herald,
         * and any port synonyms.
         */

	af = afopen ("/etc/security/login.cfg");
	if (af == NULL) return 1;

	ar = afgetrec (af, pseudopname);

	/*
         * Get the login banner from the login configuration file.
         * Use a default value if unable to get the proper one.
         */

	if ((herald = afgetatr (ar, "herald")) == 0 || ! *herald)
	{
		ar1 = afgetrec (af, "default");
		if ((herald = afgetatr (ar1, "herald")) != 0 && *herald)
			herald = strdup (herald);
	}
	else
		herald = strdup (herald);

	/*
         * See if there are any synonyms for this port.
         */

	if (tptr1 = afgetatr (ar, "synonym")) {
		int len;
		/*
		 * We have to parse the synonym list, it comes in a NULL
		 * separated, double NULL terminated list return from afgetatr.
		 * Each element of the list may contain one or several entires
		 * seperated by any of the characters defined in ODM_DELIM_CHARS
		 * Hence we will loop through each element until a NULL is hit
		 * (returned from tsm_parse) and then bop on to the next element
		 * until we hit a double NULL indicating the end of the list.
		 */
		tptr = tptr1;
		len = strlen(tptr);
		/*
		 * While not DOUBLE NULL
		 */
		while (*tptr1 != '\0') {
			/*
			 * Parse the list element
			 */
			while (tptr && nsyns < TSM_MAX_SYNS)  {
				synonym_list[nsyns] = strdup (tsm_parse(&tptr));
				nsyns++;
			}
			/*
			 * Grab the next element
			 */

			tptr = tptr1 + len + 1;
			len = strlen(tptr);
			tptr1 = tptr;
		}
	}

	afclose (af);
	return 1;
}

/*
 *  NAME:       findset_index
 *
 *  FUNCTION:   Check to see if a passed setting is legal and get its
 *              index into our array if it is
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              ATTR_NFOUND  if the attribute is illegal or the index
 *              into the setting array if it is legal
 */

int
findset_index (char * attribute)
{
	int i;

	for (i = ODM_START; settings[i].p_name != NULL; i++) {
		if (strcmp (attribute, settings[i].p_name) == MATCH) {
			return (i);
		}
	}

	return (ATTR_NFOUND);
}

/*
 *  NAME:       octatoi
 *
 *  FUNCTION:   Change an acsii string of an octal number to an int.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              return the converted value
 */

int
octatoi (char * s)
{
	return (int) strtol (s, NULL, 8);
}

/*
 *  NAME:       convert
 *
 *  FUNCTION:   Change a character string to the value of the sequence.
 *		Valid string formats are:
 *			"^X" == <ctrl>X
 *			  c  ==  c (c is any ascii character)
 *			octal number value, ex: 0177 == 0x7F
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *              return the converted value
 */

int
convert (char * s)
{
	int return_value = *s;
	/*
	 * Control character we took the conversion method from stty
	 */
	if (s[0] == '^')        {
		if (s[1] == '?')  {
			return_value = 0177;
		}
		else if (s[1] == '-')  {
			return_value = 0377;
		}
		else {
			return_value = s[1] & 037;
		}
	}
	/*
	 * Not a control maybe it's an octal character
	 */
	else if (isoctdigit(*s)) {
		return_value = octatoi(s);
	}
	return (return_value);
}

/*
 * NAME: tsm_parse
 *
 * FUNCTION: Remove and return the first element of an ODM attribute string
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	An ODM attribute string is a list of elements separated by any of the
 *	characters specified in ODM_DELIM_CHARS; this routine removes the first
 *	element of an ODM attribute string and returns it to the caller.
 *
 * RETURNS:
 *	First attribute of a string of attributes.
 */

char *
tsm_parse (char ** values)
{
	char *return_value;
	char *tptr;

	/*
	 * return the first value
	 */
	return_value = *values;

	/*
	 * Find the delimiting character
	 */
	tptr = strpbrk (*values , ODM_DELIM_CHARS);

	if (tptr == NULL) {
		/*
		 * There was no delimiter character; therefore there was only
		 * one attribute and the string becomes the NULL string once
		 * that attribute is removed from it.
		 */
		*values = NULL;
	} else {
		/*
		 * There was a delimiting character; replace it with a NULL
		 * character so the first attribute becomes a NULL terminated
		 * string.  Then, set the remainder of the string to point to
		 * the first character after the delimiter.
		 */
		*tptr = '\0';
		*values = tptr + 1;
	}
	return (return_value);
}

/*
 * The write did not finish presumably because of noise on the tty
 * port.  We carefully turn off echo, flush input and output, do a
 * resume and an unblock, and then exit.
 */
void
write_timeout()
{
	logtty.c_iflag = 0;                 /* turn off everything */
	logtty.c_oflag = 0;
	logtty.c_lflag = 0;
	xtcsetattr(INFD, TCSANOW, &logtty);
	tcflush(INFD, TCIOFLUSH);           /* flush input and output */
	tcflow(INFD, TCOON);                /* resume */
	tcflow(INFD, TCION);                /* unblock */
	tsm_err(MSGSTR(M_NOISE, DEF_NOISE), 1, FALSE);
}

void
execute(char **args)
{
	int	pid;
	int	status;

	if ((pid = fork()) == 0)	/* child */
	{
		execv(args[0], args);
		_exit(127);
	}

	if (pid == -1)
		return;

	while (waitpid(pid, &status, 0) == -1)
		if (errno != EINTR)
			break;
}

