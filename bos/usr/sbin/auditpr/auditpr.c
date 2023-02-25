static char sccsid[] = "@(#)20  1.21  src/bos/usr/sbin/auditpr/auditpr.c, cmdsaud, bos411, 9428A410j 5/11/94 22:09:48";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: auditpr command
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include	<fcntl.h>
#include	<pwd.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<sys/audit.h>
#include	<sys/errno.h>
#include	<sys/priv.h>
#include	<ctype.h>
#include	<sys/stat.h>
#include	<sys/param.h>		
#include 	<locale.h>
#include	"audutil.h"
#include	"values.h"
#include 	<nl_types.h>

DEBUGNAME("AUDITPR")

#include "auditpr_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDITPR,n,s)

#define	NONE	0
#define	EXEC	1
#define	PRINTF	2
#define	SYSCALL	3

struct	auditpr {
	char	*event;
	int	line;
	int	prog;
	int	argc;
	char	**argv;
};

struct	auditfile {
	char	*data;
	char	*end;
	char	*ptr;
};

struct	prtbl_t{
	char	id;
	char	*header;
 	int	width;
	int	(* prfunc)();
};

char		*Message;
char		indent_hack[BUFSIZ];
char		title1[BUFSIZ];
char		title2[BUFSIZ];
int		MessageType = -1;
static	struct	auditfile	auditfile;
static	struct	auditpr	*auditpr;
static	int 	selflag = FALSE, rawflag = FALSE, tailflag = FALSE;
static	int	tailindent = 8;
static  char	nulldev[] = "/dev/null";
static  char	sformat[] = "%-*.*s ";
static  char	dformat[] = "%-*d ";
static  char	dash[] = "----------------------------------------------------";
static	char	**argvbuf;
static	int	LineNum;
static	int	maxauditpr;
static	int	nauditpr;
static	int	nargvbuf;


prevent(struct prtbl_t *pt, struct aud_rec *pa){

	printf(sformat, pt->width, pt->width, pa->ah_event);
}

prcommand(struct prtbl_t *pt, struct aud_rec *pa){

	printf(sformat, pt->width, pt->width, pa->ah_name);
}

prlogin(struct prtbl_t *pt, struct aud_rec *pa){

	dopruid(pt->width, pa->ah_luid);
}

prreal(struct prtbl_t *pt, struct aud_rec *pa){

	dopruid(pt->width, pa->ah_ruid);
}

prprocess(struct prtbl_t *pt, struct aud_rec *pa){

	printf(dformat, pt->width, pa->ah_pid);
}

prparent(struct prtbl_t *pt, struct aud_rec *pa){

	printf(dformat, pt->width, pa->ah_ppid);
}

prthread(struct prtbl_t *pt, struct aud_rec *pa){

	printf(dformat, pt->width, pa->ah_tid);
}

prstatus(struct prtbl_t *pt, struct aud_rec *pa){

	switch(pa->ah_result){

		int i;

		i = pt->width - 6;

		case AUDIT_OK:
			printf("OK          ");
			break;
		case AUDIT_FAIL:
			printf("FAIL        ");
			break;
		case AUDIT_FAIL_AUTH:
			printf("FAIL_AUTH   ");
			break;
		case AUDIT_FAIL_PRIV:
			printf("FAIL_PRIV   ");
			break;
		case AUDIT_FAIL_ACCESS:
			printf("FAIL_ACCESS ");
			break;
		case AUDIT_FAIL_DAC:
			printf("FAIL_DAC    ");
			break;
		default:
			printf("FAIL        ");
			break;
	}
}

prtime(struct prtbl_t *pt, struct aud_rec *pa){

	extern	struct tm *gmtime();
	struct tm *timestr;
	char timeR[26];

	timestr = localtime(&pa->ah_time);

	strftime(timeR, 26, "%a %h %d %H:%M:%S %Y", timestr);
	printf(sformat, pt->width, pt->width, timeR);
}

struct	prtbl_t	prtbl[] = {
	{'e', "event",		15,	prevent},	/* event */
	{'c', "command",	31,	prcommand},	/* command */
	{'l', "login",		8,	prlogin},	/* login */
	{'r', "real",		8,	prreal},	/* real */
	{'p', "process",	8,	prprocess},	/* process */
	{'P', "parent",		8,	prparent},	/* parent */
	{'T', "thread",		8,	prthread},	/* thread */
	{'R', "status",		11,	prstatus},	/* status */
	{'t', "time",		24,	prtime},	/* time */
	{NULL}						/* terminator */
};

struct prtbl_t *prorder[] = {
	&(prtbl[0]),	/* event */
	&(prtbl[2]),	/* login */
	&(prtbl[7]),	/* result */
	&(prtbl[8]),	/* time */
	&(prtbl[1]),	/* command */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static int prordersize = sizeof (prorder) / sizeof (struct prtbl_t *);

char	vbuf[BUFSIZ*2];


main(int argc, char *argv[]){

	extern int	optind;
	extern char	*optarg;
	
	int	c;
	char	*selections = NULL;

	/* 
	 * Lose controlling terminal & change process group
	 * we must do this VERY early 
	 * (before any interesting files are opened) 
	 */

	setsid();
	kleenup(3, 0, 0);

	(void) setlocale (LC_ALL, "");
	catd = catopen(MF_AUDITPR, NL_CAT_LOCALE);

	/* 
	 *  Privilege check
	 */

	if(privcheck(SET_PROC_AUDIT)){
		errno = EPERM;

		exit1(EPERM, 
		MSGSTR(ERRPRIV, 
		"** SET_PROC_AUDIT privilege required"), 
		0);

	}

        /*
         * Suspend auditing
         */

        if(auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0){
                errno = EPERM;

                exit1(errno,
                MSGSTR(ERRSUSPEND,
                "** cannot suspend auditing "),
                0);

        }

	/*
 	 * if not associated with a tty then close 
	 * to protect against frevoke
	 * (exit1 does a freopen())
	 */

/*
	if(isatty(STDERR_FILENO)){
		fclose(stderr);
	}
*/

	while((c = getopt(argc, argv, "t:h:m:rv")) != EOF){
		switch(c){
			case 't':
				switch(optarg[0]){

					case '0':

						if(MessageType != -1){
					  		errno = EINVAL;

							exit1(EINVAL, 
							MSGSTR(USAGE, 
"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"),
							0);

						}

						MessageType = 0;

						break;

					case '1':

						if(MessageType != -1){
							errno = EINVAL;

							exit1(EINVAL, 
							MSGSTR(USAGE, 
"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"),
							0);

						}

						MessageType = 1;
						break;

					case '2':

						if(MessageType != -1){
							errno = EINVAL;

							exit1(EINVAL, 
							MSGSTR(USAGE, 
"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"),
							0);
						}

						MessageType = 2;
						break;

					default:
						errno = EINVAL;

						exit1(EINVAL, 
						MSGSTR(USAGE, 
"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"),
						0);

				}
				break;
					
			case 'h':
				selflag = TRUE;
				selections = optarg;
				break;
			case 'r':
				rawflag = TRUE;
				break;
			case 'm':
				Message = optarg;
				break;
			case 'v':
				tailflag = TRUE;
				break;
			default:
				errno = EINVAL;

				exit1(EINVAL, 
				MSGSTR(USAGE, 
"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"),
				0);

		}
	}

	if(MessageType == -1){

		MessageType = 1;
	}
	
	if(argc > optind){
		errno = EINVAL;

		exit1(EINVAL, 
		MSGSTR(USAGE, 
"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"),
		0);

	}

	/* 
	 * Turn off line buffering for stdout 
	 */

	setvbuf(stdout, vbuf, _IOFBF, sizeof(vbuf));

	if(tailflag){

		if(tailinit ()){

			fprintf(stderr, "%s\n",
			MSGSTR(NOTAILS, 
			"** warning - no tails will be printed")
			);

			tailflag = 0;
		}
	}

	if(selflag){

		char fld;
		char *temp = selections;
		int  idx; 
		int  i = 0;

		while((fld = *temp++) != NULL){

			if(fld == ',')continue;

			for(idx = 0; prtbl[idx].id != NULL; ++idx){

				if(prtbl[idx].id == fld){

					if(i < prordersize){
						prorder[i++] = &(prtbl[idx]);
					}
					break;
				}
			}
			if(prtbl[idx].id == NULL){

				selecterr();
			}
		}

		prorder[i] = NULL;

	}

	if(MessageType){

		title_setup();
	}

	if(MessageType == 1){

		title_print ();
	}

	while(1){

		char	*tail;
		struct	aud_rec	ah;
		extern	char	*auditread();

		if ((tail = auditread(stdin, &ah)) == NULL){

			break;

		}

		if(MessageType > 1)
			title_print ();

		/*
		 * Format and output the audit record header.  This happens
		 * for every header.
		 */

		doprinthead(&ah);
		fflush(stdout);

		/*
		 * If there is a tail and the user has selected the -v
		 * flag (tailflag), output and flush the tail.
		 */

		if(tailflag && ah.ah_length){
			tailprint((char *)ah.ah_event, (char *)tail, 
			(ushort_t)ah.ah_length, (ulong_t)ah.ah_result);
			fflush(stdout);
		}
	}
	catclose (catd);
	exit(errno);
}           


static
selecterr(){
	int	idx;

	fprintf(stderr, "%s\n",
MSGSTR(HERROR,  "** invalid fields in -h parameter"));
	fprintf(stderr, 
MSGSTR(FIELDS,  "** valid fields are -- "));
	for (idx=0; prtbl[idx].id != NULL; ++idx)
		fprintf(stderr, "%c", prtbl[idx].id);
	fprintf(stderr, "\n");
	exit(1);
}

static
title_setup(){

	struct	prtbl_t	*pt;
	char	*t1;
	char	*t2;
	int	i;

	t1 = title1, t2 = title2;
	for(i = 0; pt = prorder[i]; i++){
		t1 += sprintf (t1, sformat, pt->width, pt->width, pt->header);
		t2 += sprintf(t2, sformat, pt->width, pt->width, dash);
	}
	*t1 = *t2 = 0;
}

title_print(){

	char	*fmt;

	if(Message)printf ("%s\n", Message);

	fmt = "%s\n%s\n";
	printf (fmt, title1, title2);
}

static
doprinthead(struct aud_rec *audrec){

	struct prtbl_t	*pt;
	int 	i;

	for (i = 0; pt = prorder[i]; i++) {
		(*pt->prfunc)(pt, audrec);
	};

	printf("\n");
}

static
dopruid(int width, uid_t uid){
	struct passwd	*pwrec;
	extern struct passwd	*getpwuid();

	if (rawflag || (pwrec=getpwuid(uid)) == NULL){
		printf(dformat, width, uid);
		return;
	}
	printf(sformat, width, width, pwrec->pw_name);
	return;
}

/*
 * parse a C-style string.
 * returns a pointer to the next character in the input
 * (not involved in the string)
 */
static
char *
cstring(char *input, char *output){

	char	c;
	char	quote;

	quote = *input;
	if (*input == '"')
		quote = *input++;
	else
		quote = '\0';

	while (1) {
		c = *input++;
		if ((c == '\0') || (c == quote)) {
			*output++ = '\0';
			return (input);
		}
		if (c != '\\') {
			*output++ = c;
			continue;
		}
		c = *input++;
		switch (c) {
			case 'b':
				*output++ = '\b';
				continue;
			case 'f':
				*output++ = '\f';
				continue;
			case 'n':
				*output++ = '\n';
				continue;
			case 'r':
				*output++ = '\r';
				continue;
			case 't':
				*output++ = '\t';
				continue;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			{
				int	n;

				/*
				 * at most, two more digits ...
				 * might as well code it inline
				 */
				n = c - '0';
				c = *input;
				if (('0' <= c) && (c <= '9')) {
					input++;
					n *= 8;
					n += (c - '0');
					c = *input;
					if (('0' <= c) && (c <= '9'))
					{
						input++;
						n *= 8;
						n += (c - '0');
					}
				}
				*output++ = n;
				continue;
			}
			case '\\':
				*output++ = '\\';
				continue;
			case '"':
				*output++ = '"';
				continue;

			default:
				*output++ = '\\';
				input--;
				continue;
		}
	}
}

static
readinit(){
	extern	char	*xalloc ();

	int	fd;
	struct	stat	stbuf;

	/* open the data file */
	if((fd = open (PATH_EVENT, O_RDONLY)) == -1) {
		perror (PATH_EVENT);
		return (-1);
	}

	/* allocate a buffer to hold all the data (+1 for extra '\n') */
	if(fstat (fd, &stbuf)) {
		perror (PATH_EVENT);
		goto	error;
	}
	auditfile.data = xalloc (stbuf.st_size + 1);
	/* fetch the data */
	if(read (fd, auditfile.data, stbuf.st_size) != stbuf.st_size){
		fprintf(stderr, "%s\"%s\"\n",
MSGSTR(ERRREAD, "** cannot read "), PATH_EVENT);
		free (auditfile.data);
		goto	error;
	}
	auditfile.ptr = auditfile.data;
	auditfile.end = auditfile.data + stbuf.st_size;
	*auditfile.end = '\n';
	close (fd);
	LineNum = 0;
	return (0);
error:
	close (fd);
	return (-1);
}

/*
 * read a line from the input file:
 *	1) ignore blank lines
 *	2) trim leading blanks
 *	3) trim trailing blanks
 * returns a pointer to the start of the line,
 * or NULL on EOF/error
 */

static
char *
readln() {

	char	*line;
	char	*endp;

retry:
	while (1) {
		/* EOF ? */
		if (auditfile.ptr >= auditfile.end) {
			return (NULL);
		}
		LineNum++;
		/* record start of line (and skip leading white space) */
		for (line = auditfile.ptr; isspace (*line); line++)
			;
		/* find end of line (and assure NULL terminated) */
		for ( ; 1; auditfile.ptr++) {
			if (*auditfile.ptr == '\0')
				break;
			if (*auditfile.ptr == '\n') {
				*auditfile.ptr = '\0';
				break;
			}
		}
		/* remember '\0', but advance pointer to next line */
		endp = auditfile.ptr++;
		/* skip comment lines */
		if ((*line == '*') || (*line == '#')) {
			continue;
		}
		/* strip trailing blanks */
		while (1) {
			if (endp <= line) {
				goto	retry;
			}
			if (!isspace (endp[-1]))
				break;
			*--endp = '\0';
		}
		/* return stripped line */
		return (line);
	}
}

/* record an event description */
static
install(char *event, int prog, int argc, char **argv){

	extern	char	*xalloc ();
	extern	char	*xralloc ();

	/* assure that have an available <auditpr> structure */
	if(auditpr == NULL){
		maxauditpr = 256;
		auditpr = (struct auditpr *) xalloc 
		(maxauditpr * sizeof (struct auditpr));

		nauditpr = 0;
	}
	else if (nauditpr >= maxauditpr) {
		maxauditpr += 128;
		auditpr = (struct auditpr *) 
		xralloc((char *) auditpr, maxauditpr * sizeof (struct auditpr));
	}

	/* assure that have space for <argv> (with NULL) */
	if((argc + 1) > nargvbuf) {
		nargvbuf = NCARGS;
		argvbuf = (char **) xalloc (nargvbuf * sizeof (char **));
	}

	/* build the <auditpr> structure */
	auditpr[nauditpr].event = event;
	auditpr[nauditpr].line = LineNum;
	auditpr[nauditpr].prog = prog;
	if (argc == 0) {
		auditpr[nauditpr].argc = 0;
		auditpr[nauditpr].argv = NULL;
	}
	else {
		register int	i;

		auditpr[nauditpr].argc = argc;
		auditpr[nauditpr].argv = argvbuf;

		for (i = 0; i < argc; i++)
			*argvbuf++ = *argv++;
		*argvbuf++ = NULL;
		nargvbuf -= (argc + 1);
	}

	nauditpr++;
	return (0);
}

/*
 * compare two <auditpr> structures;
 * this is used to sort the structures by event name.
 * if identical names are found,
 * the entry which appeared earlier in the original file wins
 * (by overwriting the other entry).
 */
static
audins(struct auditpr *p, struct auditpr *q){

	int	rc;
	struct	auditpr	*from;
	struct	auditpr	*to;

	if (rc = strcmp (p -> event, q -> event))
		return (rc);

	/* replace the 2nd entry with a copy of the 1st */
	if (p -> line < q -> line) {
		from = p;
		to = q;
	}
	else {
		from = q;
		to = p;
	}
	*to = *from;
	return (0);
}

static
audcmp(struct auditpr *p, struct auditpr *q){
	return (strcmp (p -> event, q -> event));
}

/* parse the data file into the array of event descriptions */

tailinit() {

	char	*line;

	/* will be added as argument to programs */
	sprintf (indent_hack, "%d", tailindent);

	if (readinit ()) {
		/* readinit() prints an error message */
		return (-1);
	}

	/*
	 * expect to see:
	 *	auditpr:
	 */
	if ((line = readln()) == NULL)
		goto	nostanza;

	/* match the "stanza" name */
	if (strncmp (line, STZ_AUDITPR, sizeof (STZ_AUDITPR) - 1) != 0)
		goto	nostanza;

	/* skip white space */
	for (line += sizeof (STZ_AUDITPR) - 1; isspace (*line); line++);

	/* match the ':' */
	if (*line++ != ':') {
nostanza:
		fprintf (stderr, "%s\"%s\"%s\"%s\"\n",
MSGSTR(ERRSTANZA1, "** cannot find "), STZ_AUDITPR,
MSGSTR(ERRSTANZA2, " in "), PATH_EVENT);
		return (-1);
	}
	/* ignore any additional junk on this line */

	/*
	 * now expect to see lines of the form:
	 *	event = <token> <token> ...
	 */
	while (1) {
		char	*event;
		int	prog;
		char	*argv[NCARGS];
		int	argc;

		if ((line = readln()) == NULL)
			break;
		event = line;
		/*
		 * delimit event name (and consume '=')
		 * we DON'T require an '=' ...
		 */
		for ( ; 1; line++) {
			if (*line == '\0')
				/* event name alone on line */
				break;
			if (*line == '=') {
				/* event name followed immediately by '=' */
				*line++ = '\0';
				break;
			}
			if (isspace (*line)) {
				*line++ = '\0';
				while (isspace (*line))
					line++;
				if (*line == '=')
					/* consume the '=' */
					line++;
				break;
			}
			/* not at end of event name yet */
		}
		if (*event == '\0') {
			continue;
		}

		/* scan blank- or quote-delimited program and arguments */
		for (argc = 0; 1; argc++) {
			/* HACK: insert "-i <indent>" as argument 1 */
			if (argc == 1) {
				argv[argc++] = "-i";
				argv[argc++] = indent_hack;
			}
			/* skip white space before next argument */
			while (isspace (*line))
				line++;
			/* done ? */
			if (*line == '\0')
				break;
			/* record this argument, and delimit */
			argv[argc] = line;
			/* quoted argument */
			if (*line == '"') {
				/* this will copy down over '"' */
				line = cstring (line, line);
				continue;
			}
			/* blank- or eoln-delimited argument */
			while (1) {
				if (*line == '\0')
					/* musn't advance past '\0' */
					break;
				if (isspace (*line))
				{
					*line++ = '\0';
					break;
				}
				line++;
			}
		}
		if (argc == 0) {
			prog = NONE;
		}
		else {
			prog = EXEC;
			if (strcmp (argv[0], "printf") == 0)
				prog = PRINTF;
			else if (strcmp (argv[0], "syscall") == 0)
				prog = SYSCALL;
		}
		install (event, prog, argc, argv);
	}
	qsort ((char *) auditpr, nauditpr, sizeof (struct auditpr), audins);
	return (0);
}

static
indent(char *nbuf){

	int	n;

	n = atoi (nbuf);
	while (n-- > 0)
		putc (' ', stdout);
}

static
tailprint(event, tail, len, status)
char *event;
char *tail;
int len;
uchar status;{

	extern	char	*bsearch ();
	struct	auditpr	key;
	struct	auditpr	*p;

	key.event = event;
	p = (struct auditpr *) bsearch ((char *) &key, 
	(char *) auditpr, nauditpr, sizeof (struct auditpr), audcmp);

	if (p == NULL) {
		indent (indent_hack);
		fprintf (stdout, "%s\n",
MSGSTR (NOTAIL, "<tail format undefined>"));
		return;
	}
	if (p -> prog == NONE) {
		/* nothing to do */;
	}
	else if (p -> prog == EXEC) {
		run (p -> argc, p -> argv, tail, len);
	}
	else if (p -> prog == PRINTF) {
		audprintf (p -> argc, p -> argv, tail, len);
	}
	else if (p -> prog == SYSCALL) {
		audsyscall (p -> argc, p -> argv, tail, len);
	}
	else {
		indent (indent_hack);
		fprintf (stdout, "%s\n",
MSGSTR (INTERNAL, "<INTERNAL ERROR - unknown program type>"));
	}
}

static
run (int ac, char **av, char *data, int len){

	int	p[2];
	int	pid;

	/* fork pgm to do print */
	pipe(p);

	pid = xfork ();
	if (pid == 0) {
		/* child */
		register char	*fname;
		extern	char	*strrchr ();

		/* set up stdin */
		close (p[1]);
		close (0);
		dup (p[0]);
		close (p[0]);

		/* this is in child - we can munge av[] */
		fname = av[0];
		av[0] = strrchr (av[0], '/');
		if (av[0] == NULL)
			av[0] = fname;
		else
			av[0]++;
		execv (fname, av);
		if (strcmp (av[1], "-i") == 0)
			indent (av[2]);
		fprintf (stdout, "%s\"%s\"\n",
MSGSTR(NORUN, "** cannot run "),fname);
		exit (errno);
	}

	/* parent */
	
	/* pass on data */
	close (p[0]);
	if (len > 0)
		write (p[1], data, len);
	close (p[1]);
	/* and wait for child */
	while (1) {
		int	r;

		r = wait ((int *) 0);
		if ((r == pid) || (r == -1))
			break;
	}
}

#define	tonumber(c)	((c) - '0')

#define	align(to, data, dlen) \
	{ \
		if (dlen < sizeof (*(to))) \
			memset ((to), sizeof (*(to)), 0); \
		else \
		{ \
			bcopy((data), (to), sizeof (*(to))); \
			data += sizeof (*(to)); \
			dlen -= sizeof (*(to)); \
		} \
	}

#define	fetch(c,data,dlen) \
	{ \
		if (dlen <= 0) \
			c = 0; \
		else \
		{ \
			c = *data++; \
			dlen--; \
		} \
	}

/* audprintf -i indent fmt */
int
audprintf (int ac, char **av, char *data, int dlen){

	/* format string */
	char	*format;

	/* Starting and ending points for value to be printed */
	char	*bp = (char *) 0;
	char	*p = (char *) 0;

	/* Field width and precision */
	int	width;
	int	prec;

	int	c;

	/* Number of padding zeroes required on the left and right */
	int	lzero;
	int	rzero;

	/* Flags - nonzero if corresponding character is in format */
	int	fplus;	/* + */
	int	fminus;	/* - */
	int	fblank;	/* blank */
	int	fsharp;	/* # */

	/* Values are developed in this buffer */
	char	buf[BUFSIZ];

	/* Pointer to sign, "0x", "0X", or empty */
	char	*prefix;

	/* The value being converted, if integer */
	long	val;

	/* Pointer to a translate table for digits of whatever radix */
	char	*tab;

	/* Work variables */
	int	k;
	int	n;
	int	hradix;

	/* number of spaces to indent the output */
	int	nindent;
	/*
	 * pretty much everywhere we put out a character for the user,
	 * we must check whether it was a '\n' or '\r'.
	 * if so, <indent_needed> should be set.
	 * before we put out the next character,
	 * this will cause indentation to be done.
	 */
	int	indent_needed;

#define	indent_chk() \
	{ \
		if (indent_needed) \
		{ \
			register int	i; \
			for (i = 0; i < nindent; i++) \
				putc (' ', stdout); \
			indent_needed = 0; \
		} \
	}

	/*
	 * interpret arguments
	 */
	if (strcmp (av[1], "-i") == 0) {
		if (ac < 4)
			/* "no" arguments */
			return;
		nindent = atoi (av[2]);
		format = av[3];
	}
	else {
		if (ac < 2)
			/* "no" arguments */
			return;
		nindent = 0;
		format = av[1];
	}

	/*
	 *	The main loop -- this loop goes through one iteration
	 *	for each string of ordinary characters or format specification.
	 */
	indent_needed = 1;
	for (;;) {
		/* extract the next character of the format string */
		c = *format++;

		/* done ? */
		if (c == '\0') {
			/* append new-line, if didn't just do so */
			if (!indent_needed) {
				fputc ('\n', stdout);
			}
			return;
		}

		indent_chk ();

		/* if not an escape, nothing to do */
		if (c != '%') {
			putc (c, stdout);
			/* need to precede <cf> and <lf> with <indent> */
			if ((c == '\n') || (c == '\r'))
				indent_needed = 1;
			continue;
		}

		/*
		 * % has been found.
		 * First, parse the format specification.
		 */
		fplus = fminus = fblank = fsharp = lzero = 0;

		/* Scan the <flags> */
		for (;;) {
			c = *format++;
			switch (c) {
				case '+':
					fplus++;
					continue;
				case '-':
					fminus++;
					continue;
				case ' ':
					fblank++;
					continue;
				case '#':
					fsharp++;
					continue;
			}
			break;
		}

		/* Scan the field width */
		if (c == '*') {
			align (&width, data, dlen);
			if (width < 0) {
				width = -width;
				fminus++;
			}
			c = *format++;
		}
		else if (isdigit (c)) {
			/* obsolescent spec. */
			if (c == '0')
				/* pad with leading zeros */
				lzero++;
			width = 0;
			while (1) {
				c = *format++;
				if (!isdigit(c))
					break;
				width = width * 10 + tonumber (c);
			}
		}
		else
			width = 0;

		/* Scan the precision */
		if (c == '.') {
			c = *format++;
			if (c == '*') {
				align (&prec, data, dlen)
				c = *format++;
			}
			else {
				prec = 0;
				while (1) {
					c = *format++;
					if (!isdigit (c))
						break;
					prec = prec * 10 + tonumber (c);
				}
			}
		}
		else
			prec = lzero ? width : -1;

		/* consume length modifier - everything's a <long> */
		if ((c == 'l') || (c == 'h'))
			c = *format++;

		prefix = "";
		lzero = rzero = 0;

		switch (c) {

			case 'd':
			case 'u':
				hradix = 10 / 2;
				goto fixed;

			case 'o':
				hradix = 8 / 2;
				goto fixed;

			case 'X':
			case 'x':
				hradix = 16 / 2;

		fixed:
				/* Establish default precision */
				if (prec < 0)
					prec = 1;

				align (&val, data, dlen);

				/* If signed conversion, make sign */
				if (c == 'd') {
					if (val < 0) {
						prefix = "-";
						if (val != HIBITL)
							val = -val;
					}
					else if (fplus)
						prefix = "+";
					else if (fblank)
						prefix = " ";
				}

				/* Set translate table for digits */
				tab = (c == 'X') ?
					"0123456789ABCDEF" : "0123456789abcdef";

				/* Develop the digits of the value */
				p = bp = &(buf[sizeof(buf)]);
				while (val != 0) {
					int	lowbit;

					lowbit = val & 1;
					val = (val >> 1) & ~HIBITL;
					*--bp = tab[val % hradix * 2 + lowbit];
					val /= hradix;
				}

				/* Calculate padding zero requirement */
				lzero = bp - p + prec;

				/* Handle the # flag */
				if (fsharp && bp != p)
					switch (c) {
						case 'o':
							if (lzero < 1)
								lzero = 1;
							break;
						case 'x':
							prefix = "0x";
							break;
						case 'X':
							prefix = "0X";
							break;
					}

				break;

			/*  case '%': */
			default:
				buf[0] = c;
				goto c_merge;

			case 'c':
				fetch (buf[0], data, dlen);
		c_merge:
				p = (bp = &buf[0]) + 1;
				break;

			case 's':
			{
				bp = data;
				while (1) {
					if (dlen <= 0)
						break;
					if (*data == '\0') {
						/* consume the '\0' */
						*data = ' ';
						data++, dlen--;
						break;
					}
					data++, dlen--;
				}
				if (prec < 0)
					p = data;
				else {
					if (data <= bp + prec)
						p = data;
					else
						p = bp + prec;
				}
				break;
			}

			/* unexpected end of format; return error */
			case '\0':
				return;
		}

		/* Calculate number of padding blanks */
		if (lzero < 0)
			lzero = 0;
		if (rzero < 0)
			rzero = 0;
		k = (n = p - bp) + lzero + rzero +
			(prefix[0] == '\0' ? 0 : prefix[1] == '\0' ? 1 : 2);

		/* Blanks on left if required */
		if (!fminus) {
			while (--width >= k)
				(void) putc (' ', stdout);
		}

		/* Prefix, if any */
		while (*prefix != '\0')
			(void) putc (*prefix++, stdout);

		/* Zeroes on the left */
		while (--lzero >= 0)
			(void) putc ('0', stdout);

		/*
		 * The value itself.
		 * This may have been supplied by the user (%s/%c),
		 * so it could have imbedded '\n's.
		 */
		for ( ; n > 0; n--) {
			char	ch;

			indent_chk ();
			ch = *bp++;
			putc (ch, stdout);
			if ((ch == '\n') || (ch == '\r'))
				indent_needed = 1;
		}
		if ((rzero > 0) || (width > 0))
			/* this would be pretty weird, but ... */
			indent_chk ();

		/* Zeroes on the right */
		while (--rzero >= 0) {
			(void) putc ('0', stdout);
		}

		/* Blanks on the right if required */
		while (--width >= k)
			(void) putc (' ', stdout);
	}
}
#define	indent() \
	{ \
		register int	i; \
		for (i = 0; i < nindent; i++) \
			putc (' ', stdout); \
	}

audsyscall(int ac, char **av, char *data, int dlen){

	int	nindent;

	if ((strcmp (av[1], "-i") == 0) && (ac >= 2))
		nindent = atoi (av[2]);
	else
		nindent = 0;
	
	/*
	 * svc audit record format:
	 *	nargs
	 *	args[]
	 *	npath
	 *	paths[]
	 *		each path is followed by:
	 *			char symlink
	 *			char errno
	 */

	/* scan number of arguments from data and print */
	{
		int	nargs;
		int	i;

		align (&nargs, data, dlen);
		indent ();
		fprintf (stdout, "%d args:", nargs);

		/* scan each argument from data and print */
		for (i = 0; i < nargs; i++) {
			int	arg;

			align (&arg, data, dlen);
			fprintf (stdout, " 0x%X", arg);
		}

		/* end of arguments line */
		fprintf (stdout, "\n");
	}

	/* scan number of paths from data and print; if none, are done */
	{
		int	npath;
		int	i;

		align (&npath, data, dlen);
		if (npath == 0)
			return;
		indent ();
		fprintf (stdout, "%d paths: \n", npath);

		/* scan each pathname from data and print */
		for (i = 0; i < npath; i++) {
			char	syml;
			char	errno;

			indent ();
			putc ('\t', stdout);
			if (dlen <= 0) {
				fprintf (stdout, "%d%s\n",
MSGSTR(NOPATHS, " paths not in record"), npath - i);
				break;
			}
			while (1) {
				char	c;

				if (dlen <= 0)
					break;
				fetch (c, data, dlen);
				if (c == '\0')
					break;
				putc (c, stdout);
			}
			fetch (syml, data, dlen);
			fprintf (stdout, " %d ", syml);
			fetch (errno, data, dlen);
			fprintf (stdout, " %d ", errno);
			putc ('\n', stdout);
		}
	}
}
/*
dumpauditpr()
{
	int i;
	printf("\n");
	for (i=0; i<nauditpr; i++)
		printf(" i=%d  %X:  event=<%s>\n",i,auditpr[i],auditpr[i].event);
}
*/
#undef	tonumber
#undef	indent_chk
#undef fetch
#undef align
#undef	indent
