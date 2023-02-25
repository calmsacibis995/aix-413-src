static char sccsid[] = "@(#)21	1.44.1.14  src/bos/usr/sbin/cron/at.c, cmdcntl, bos41B, 9504A 1/4/95 10:13:11";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS: at
 *
 * ORIGINS: 3, 18, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * OSF/1 1.1
 *
 * at.c	4.3 00:41:01 7/12/90 SecureWare 
 */

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/param.h>        /* for BSIZE needed in dirent.h */
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ulimit.h>
#include <unistd.h>
#include <nl_types.h>
#include <langinfo.h>
#include "cron.h"
#include "cron_msg.h"

#if SEC_BASE
#include <sys/secdefines.h>
#include <sys/security.h>
extern priv_t *privvec();
#else
#include <sys/id.h>
#include <sys/audit.h>
#include <usersec.h>
#include <sys/priv.h>
#endif /* SEC_BASE */

nl_catd catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
#define TMPFILE		"_at"	/* prefix for temporary files	*/

#if SEC_BASE
#define ATMODE		06040	/* Mode for creating files in ATDIR. */
#else
#define ATMODE		06444	/* Mode for creating files in ATDIR. */
#define ROOT            0       /* user-id of super-user */
#define AUDITRMV        1
#define AUDITADD        2
#endif /* SEC_BASE */

#define BUFSIZE		512	/* for copying files */
#define TIMESIZE	256	/* for making time string */
#define LINESIZE	130	/* for listing jobs */
#define	MAXTRYS		60	/* max trys to create at job file */
#define	MPWDTRYS	15	/* max trys to create at pipe for pwd */

#define BADDATE		"bad date specification"
#define BADMINUTES	"minutes field is too large"
#define CANTCD		"can't change directory to the at directory"
#define CANTCHOWN	"can't change the owner of your job to you"
#define	CANTCHMOD	"can't change the mode of your job"

#if SEC_BASE
#define	INSUFFPRIVS	"at: insufficient privileges"
#endif /* SEC_BASE */

#define BADSTAT		"can't access the %s file.\n"
#define CANTOPEN  	"can't open job file in the %s directory\n"
#define CANTLINK  	"can't link job file for you\n"
#define CANTCREATE	"can't create a job for you\n"
#define CANTCWD		"bad return code from the library routine: getcwd()\n"
#define CANTPEEK	"you may only look at your own jobs.\n"
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)\n"
#define	NONUMBER	"proper syntax is:\n\tat -ln\nwhere n is a number\n"
#define ATNOREADDIR	"can't read the at directory\n"
#define NOTALLOWED	"you are not authorized to use at.  Sorry.\n"
#define NOTHING		"nothing specified\n"
#define NOPROTO		"no prototype file\n"
#define PAST		"it's past that time\n"
#define CONFOPTS	"conflicting options\n"
#define BADQUE		"bad queue specification\n"
#define NOKSH 		"ksh is currently not available\n"
#define NOCSH		"csh is currently not available\n"
#define MALLOC		"out of space, can not continue\n"
#define BADCHOWN	"at: can not change atjob file owner\n"
#define ATRMDEL 	"at: only jobs belonging to user: %s may be deleted\n"
#define AUDITER         "%s: auditproc error: %d\n"
#define MAX_TIME	65  /* size of time syntax */

/* This message is mailed to the user with -m flag. */
#define MFLAG_BUFF     	1024
#define MAILJOB       	"cron: Job %s was run."

/*
	this data is used for parsing time
*/
#define dysize(A) (((1900+(A)) % 4 == 0 && (1900+(A)) % 100 != 0 || (1900+(A)) % 400 == 0) ? 366:365)

/* Current flags */
static int	lflag = 0;		/* list atjob files */
static int	rflag = 0;		/* remove atjobs files */
static int	cflag =	0;		/* execute in c shell */
static int     Fflag = 0;              /* suppress deletion info */
static int	kflag =	0;		/* execute in korn shell */
static int	nflag =	0;		/* list number of files */
static int	iflag =	0;		/* interactive delete */
static int	uflag =	0;		/* user flag */
static int	oflag =	0;		/* list in schedule order */
static int	shflag = 0;		/* bourne shell flag - default */
static int	qflag = 0;		/* select queue */
int	gmtflag = 0;		/* greenwich mean time */
static int	pathflg = 0;		/* pathname instead of stdin */
static int	mflag = 0;		/* send mail to the invoking user */
static int	tflag = 0;		/* submit the job according to time argument*/
static int	ls_qflag = CRON_QUE_A;	/* serch Jobs by the defined queue name */
static char	*sh_name;		/* shell name set to SHELL environment */

extern int errno;
extern	time_t	timezone;
extern	char	*argp;
static char	*nargp;
static char	login[UNAMESIZE];
static char	argpbuf[PATH_MAX+MAX_TIME];  /* buf desc of time = 1023+time syntax */
static char	pname[PATH_MAX];
static char	pname1[PATH_MAX];
static char	jobname[PATH_MAX];
static char	*job;			/* file name in /usr/spool/cron/atjobs	*/
static time_t	when, now;
struct	tm	*tp, at, rt;
static struct	tm	*timptr;
int	mday[12] =
{
	31,28,31,
	30,31,30,
	31,31,30,
	31,30,31,
};
static int	mtab[12] =
{
	0,   31,  59,
	90,  120, 151,
	181, 212, 243,
	273, 304, 334,
};
static int     dmsize[12] = {
	31,28,31,30,31,30,31,31,30,31,30,31};

/* end of time parser */

static short	jobtype = ATEVENT;		/* set to 1 if batch job */
static short	exeflag = FALSE;		/* execute the filename given? */
static char	*tfname;			/* temporary file name */
static char	*jname = NULL;			/* job name (if existant) */
static char    queue = '\0';                   /* queue specified with -q flag */


extern	char *xmalloc();

char	*select_shell();
extern	int list_aj();
extern	int remove_aj();
extern	void strtolower();
extern  int send_msg();

main(argc, argv)
int	argc;
char	*argv[];
{
	struct passwd 	*nptr;
	int 		i, fd, catch(void);
	uid_t 		user;
	char 		pid[6], timestr[TIMESIZE + 1];
	char 		*pp;
	char 		*mkjobname();
	int  		rc, st = 0;
        int             c;                      /* getopt option */
	char		*tempdate;


	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CRON,NL_CAT_LOCALE);

#if SEC_BASE
#else
        /*
         * suspend auditing for this process - This process is trusted
         */
        if (auditproc(0,AUDIT_EVENTS,"cron",strlen("cron")+2) < 0) {
                fprintf(stderr,MSGSTR(MS_AUDITER, AUDITER),"at",errno);
        }
        if (auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0) {
                fprintf(stderr,MSGSTR(MS_AUDITER, AUDITER),"at",errno);
        }

	/*
	 * set process id back to real id, we will just re-acquire privileges
	 * as we need them
	 */
	setuidx(ID_EFFECTIVE, getuidx(ID_REAL));
        privilege(PRIV_LAPSE);          /* lapse all privileges */
#endif /* SEC_BASE */

	jobname[0] = '\0';
	getnls();

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	if (forceprivs(privvec(SEC_CHMODSUGID, SEC_CHOWN,
#if SEC_MAC
				SEC_WRITEUPSYSHI,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0))
		atabort(MSGSTR(MS_ATPRIVS, INSUFFPRIVS));
#if SEC_MAC
	disablepriv(SEC_MULTILEVELDIR);
#endif

	if ((nptr = getpwuid(user = getluid())) == NULL)
#else /* !SEC_BASE */
	if ((nptr=getpwuid(user=getuid())) == NULL) /* ...LGM */
#endif /* !SEC_BASE */
		atabort(MSGSTR(MS_INVALIDUSER, INVALIDUSER));
	else 
		pp = nptr->pw_name;

	strcpy(login,pp);
	if (!allowed(login,ATALLOW,ATDENY)) 
		atabort(MSGSTR(MS_NOTALLOWED, NOTALLOWED));
	time(&now);

	/*
	 * Interpret command line flags if they exist.
	 */

        while ((c = getopt(argc, argv, ":cFf:iklmnoq:rst:u")) != EOF) {
            switch (c) {

	     case 'c' : /* Select csh */
		cflag = 1; 
	 	jobtype = CSHEVENT;
		break;
	     case 'k' : /* Select ksh */
	       	kflag = 1; 
		jobtype = KSHEVENT;
		break;
	     case 's' : /* Select sh */
	 	shflag = 1;
		break;
	     case 'u' : /* delete all for specific user */
		uflag = 1;
		break;
             case 'F' : /* suppress delete verification */
                Fflag = 1;
                break;
	     case 'f' : /* pathname instead of standard input 	    */
		pathflg++;
	  	if (access(optarg ,R_OK) != -1) {
			strncpy(jobname, optarg, strlen(optarg));
			if (access(optarg, X_OK) != -1)
				exeflag = TRUE;	/* execute this as a command */
			else
				exeflag = FALSE; /* copy the contents of
						    the file (ala BSD) */
	  	} else {
	    		fprintf(stderr, MSGSTR(MS_NOACCESS,
				"at: Cannot access %s\n"), optarg);
			exit(1);
		}
		break;
	      case 'i' : /* interactive delete */
		iflag = 1;
		break;
	      case 'l' : /* List entries */
		lflag = 1;
		break;
	      case 'n' : /* # of files */
		nflag = 1;
		break;
	      case 'o' : /* scheduled order */
		oflag = 1;
		break;
	      case 'r' : /* Remove entries */
		rflag = 1;
		break;
	      case 'q' : /* Select queue */
		qflag = 1;
		queue = *optarg;
	  	jobtype = queue - 'a';
	  	if ((jobtype < ATEVENT) || (jobtype > CSHEVENT))
	    		atabort(MSGSTR(MS_BADQUE1, BADQUE));
	  	if ((jobtype == CRONEVENT) || (jobtype == SYNCEVENT))
	    		atabort(MSGSTR(MS_BADQUE1, BADQUE));
               	switch (jobtype) {
                       	case CSHEVENT:
                             cflag = 1;
                             break;
                       	case KSHEVENT:
                             kflag = 1;
                             break;
                 	}
		break;
	      case 'm' : /* Send mail (BSD option, AIX default) */
		mflag = 1;
		break;
	      case 't' : /* timespec  */
		tflag = 1;
		tempdate = optarg; 
		break;
	      case ':':  /* missing option argument */
                fprintf(stderr,MSGSTR(MS_NOTHING, NOTHING));
	
	      default  : /* Unknown switch */
		usage();
	      }
	}
        argc -= optind;
        argv += optind;

	if (rflag) {
		/* remove jobs that are specified */
		if (argc == 0)
			atabort(MSGSTR(MS_NOTHING, NOTHING));
		i=0;
		do {
#if SEC_BASE
			if (uflag && strcmp(argv[i], login) && !at_authorized())
#else
			if (uflag && (user != ROOT) && (strcmp(argv[i], login)))
#endif
			{
			    /* called by atrm 	*/
			    fprintf(stderr,"at: ");
			    fprintf(stderr,MSGSTR(MS_ATRMDEL,ATRMDEL), login);
			    exit(1);
			}
			if (iflag) {
				if (uflag) {
				    rc = remove_aj(CRON_PROMPT|CRON_USER,
						argv[i]);
				    audit_pr(REMOVE_AT, argv[i]);
				} else {
				    if (argv[i] != NULL) {
					rc = remove_aj(CRON_PROMPT,argv[i]);
				        audit_pr(REMOVE_AT, argv[i]);
				    }
				}
			}
			else {
				if (Fflag) {
				    if (uflag) {
				    	rc = remove_aj(CRON_QUIET|CRON_USER,
						argv[i]);
				        audit_pr(REMOVE_AT, argv[i]);
				    } else {
				    	rc = remove_aj(CRON_QUIET,argv[i]);
				        audit_pr(REMOVE_AT, argv[i]);
				    }
				} else
				    if (uflag) {
				    	rc = remove_aj(CRON_NON_VERBOSE |
					      CRON_USER,argv[i]);
				        audit_pr(REMOVE_AT, argv[i]);
				    } else {
					if ((user != ROOT) && 
						(strncmp(argv[i], login, 
						strlen(login)))) {
			    		    fprintf(stderr,"at: ");
			    		    fprintf(stderr,
						MSGSTR(MS_ATRMDEL,ATRMDEL), 
						login);
			    		    exit(1);
					}
				    	rc = remove_aj(CRON_NON_VERBOSE,
							argv[i]);
				        audit_pr(REMOVE_AT, argv[i]);
				    }
			}
		}
		while (argv[++i]!=NULL);
		exit(rc);
	}

	if (lflag) {
		if (*argv == NULL && !qflag && !kflag && !cflag) {
		    if (user == ROOT) {
			if (oflag)
				rc = list_aj(CRON_SORT_M | CRON_USER, NULL);
			else
				rc = list_aj(CRON_SORT_E | CRON_USER, NULL);
		    } else {
			if (oflag)
				rc = list_aj(CRON_SORT_M | CRON_USER, login);
			else
				rc = list_aj(CRON_SORT_E | CRON_USER, login);
		    }
		} else if (qflag | kflag | cflag) {
			ls_qflag <<= jobtype;
		        if (user == ROOT) {
			    if (oflag)
				rc = list_aj(CRON_SORT_M | ls_qflag |
					CRON_USER, NULL);
			    else
				rc = list_aj(CRON_SORT_E | ls_qflag |
					CRON_USER, NULL);
			} else {
			    if (oflag)
				rc = list_aj(CRON_SORT_M | ls_qflag |
					CRON_USER, login);
			    else
				rc = list_aj(CRON_SORT_E | ls_qflag |
					CRON_USER, login);
			}
		} else {
			for (i=0; argv[i]!=NULL; i++) {
			    if ( strncmp(login, argv[i], strlen(login))
			    		&& (user != ROOT) )
				continue;
			    if ((strchr(argv[i], '.')) != NULL) {
				/* Job number	*/
				rc = list_aj(CRON_NOTHING,argv[i]);
			    } else {
				/* user name	*/
				if (oflag)
					rc = list_aj(CRON_SORT_M | CRON_USER,
						argv[i]);
				else
					rc = list_aj(CRON_SORT_E | CRON_USER,
						argv[i]);
			    }
			}

		}
		exit(rc);
	}

	if (nflag) {
		if (*argv == NULL)
			rc = list_aj(CRON_COUNT_JOBS | CRON_USER, login);
		else
#if SEC_BASE
		{
			rc = -1;
			for (i=0; argv[i]!=NULL; i++) {
				if (strcmp(argv[i], login) && !at_authorized())
					continue;
				rc = list_aj(CRON_COUNT_JOBS | CRON_USER,
					argv[i]);
			}
			if (rc == -1)
				atabort(MSGSTR(MS_CANTPEEK, CANTPEEK));
		}
#else
			for (i=0; argv[i]!=NULL; i++) 
				rc = list_aj(CRON_COUNT_JOBS | CRON_USER,
					argv[i]);
#endif
		 exit(rc);
	}

	fflush(stdout);
	/* figure out what time to run the job */

	sh_name = select_shell(shflag, cflag, kflag);

	if(argc == 0 && jobtype != BATCHEVENT && !tflag)
		atabort(MSGSTR(MS_NOTHING, NOTHING));
	time(&now);
	if(jobtype != BATCHEVENT) {	/* at job */
		int cnt=0;

		nargp = argpbuf;
		i = st;
		tp = localtime(&now);
		mday[1] = 28 + leap(tp->tm_year);
	  	if ((argc>0) && !pathflg && (access(argv[argc-1] ,R_OK) != -1)) {
			strncpy(jobname, argv[argc-1], strlen(argv[argc-1]));
			if (access(argv[argc-1], X_OK) != -1)
				exeflag = TRUE;	/* execute this as a command */
			else
				exeflag = FALSE; /* copy the contents of
						    the file (ala BSD) */
			argc--;
	  	}
		if (tflag) {
		    argp = tempdate;
		    if (gtime())
			usage();
		} else {
		    while(i < argc) {
			cnt += strlen(argv[i]);
			cnt++;
			if ( cnt < ( PATH_MAX + MAX_TIME ) ) {
			    strcat(nargp,argv[i]);
			    strcat(nargp, " ");
			    i++;
			}
			else {
			    break;
			}
		    }
		    if ((argp = malloc(strlen(nargp)+1)) == NULL)
			    atabort(MSGSTR(MS_MALLOC,MALLOC));
		    strtolower(argp,nargp);

		    yyparse();
		}

		atime(&at, &rt);
		mday[1] = 28 + leap(tp->tm_year);
		at.tm_isdst = -1;
		when = mktime(&at);
		if(gmtflag)
			when -= timezone;
	} else		/* batch job */
		when = now;
	if(when < now)	/* time has already past */
		atabort(MSGSTR(MS_TOOLATE, "too late\n"));
	timptr = localtime(&when);

	fflush(stdout);
	sprintf(pid,"%-5d",getpid());
	tfname=xmalloc(strlen(ATDIR)+strlen(TMPFILE)+7);
	strcpy(tfname, ATDIR);
	strcat(strcat(strcat(tfname, "/"), TMPFILE), pid);
	/* catch SIGINT, HUP, and QUIT signals */
	if (signal(SIGINT,(void(*)(int))catch)==SIG_IGN) signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP,(void(*)(int))catch)==SIG_IGN) signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,(void(*)(int))catch)==SIG_IGN)signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,(void(*)(int))catch)==SIG_IGN)signal(SIGTERM,SIG_IGN);
#if SEC_BASE
#else
	/* need to pick up some privileges to do the next bit */
	privilege(PRIV_ACQUIRE);
#endif
	if ((fd = open(tfname, O_CREAT|O_EXCL|O_WRONLY, ATMODE)) < 0) {
		fprintf(stderr,MSGSTR(MS_CANTOPEN, CANTOPEN), ATDIR); /*MSG*/
		exit(1);
	}

#if SEC_BASE
	if (fchown(fd, user, getegid()) != 0)
#else
	if (fchown(fd, user, getgid()) != 0)
#endif
	{
		unlink(tfname);
		atabort(MSGSTR(MS_CANTCHOWN, CANTCHOWN)); 
	}

	job = mkjobname(tfname, when);

#ifdef _OSF
	close(1);
	dup(fd);		/* stdout is now our temp-file */
	close(fd);
	sprintf(pname, "%s", PROTO);
	sprintf(pname1, "%s.%c", PROTO, 'a'+jobtype);
	copy();
#else
	copy(fd);
#endif /* _OSF */

	if (fflush(stdout) == EOF) {
		fprintf(stderr,MSGSTR(MS_CANTOPEN, CANTOPEN), ATDIR); /*MSG*/
		unlink(tfname);
		unlink(jname);
		exit(1);
	}
	/*
	 * Reset the mode of the job file since the SUID and SGID
	 * bits will have been cleared by writes in copy().
	 */
	if (chmod(tfname, ATMODE)) {
		unlink(tfname);
		atabort(MSGSTR(MS_CANTCHMOD, CANTCHMOD));
	}

	unlink(tfname);
	send_msg(AT, ADD, job, login);
#if SEC_BASE
#else
	/* done with privileges now */
	privilege(PRIV_LAPSE);
#endif /* SEC_BASE */
	strftime(timestr, TIMESIZE, nl_langinfo(D_T_FMT), timptr);
	fprintf(stderr,MSGSTR(MS_JOBAT, "job %s at %s\n"),
		job,timestr);

#if SEC_BASE
#else
        audit_pr(ADD_AT, job);
#endif /* SEC_BASE */

	exit(0);
}

/*
 * make a job name out of the time (when) and link it to tfname     
 *   returns the last component of the job name 		  
 */
static char *mkjobname(ttfname, t)
char	*ttfname;
time_t	t;
{
	int     i;
	char    *name;
	struct passwd *nptr;

	name = xmalloc(200);

	for (i = 0; i < MAXTRYS; i++) 
	{
		/* if file doesn't exist return */
		/*   if it does then try again  */

		if ((nptr=getpwuid(getuid())) == NULL) /* check the user id */
			atabort(MSGSTR(MS_INVALIDUSER, INVALIDUSER));
	
		sprintf(name,"%s/%s.%ld.%c", ATDIR, nptr->pw_name, t, 'a'+jobtype);
		if (link(ttfname,name) != -1) {
			jname = name;
			return (strrchr (name, '/') + 1);
		} else if (errno != EEXIST) {
			unlink(ttfname);
			atabort(MSGSTR( MS_CANTLINK, CANTLINK)); 
		}
		t += 1;
	}
	atabort(MSGSTR(MS_QFULL, "queue full\n"));
}


/****************/
static catch(void)
/****************/
{
	unlink(tfname);
	if (jname)
		unlink(jname);
	exit(1);
}


/****************/
atabort(msg)
/****************/
char	*msg;
{
	fprintf(stderr,"at: %s",msg);
	exit(1);
}

yywrap()
{
	return(1);
}

yyerror(s1)
char	*s1;
{
	if (s1 == NULL)
		atabort(MSGSTR(MS_BADDATE, BADDATE));
	else {
		fprintf(stderr,"at: %s\n",s1);
		exit(1);
	}
}

/*
 * add time structures logically
 * b is an increment of time
 */
static atime(a, b)
struct	tm	*a;
struct	tm	*b;
{
	if ((a->tm_sec += b->tm_sec) >= 60) {
		b->tm_min += a->tm_sec / 60;
		a->tm_sec %= 60;
	}
	if ((a->tm_min += b->tm_min) >= 60) {
		b->tm_hour += a->tm_min / 60;
		a->tm_min %= 60;
	}
	if ((a->tm_hour += b->tm_hour) >= 24) {
		b->tm_mday += a->tm_hour / 24;
		a->tm_hour %= 24;
	}
	a->tm_year += b->tm_year;
	if ((a->tm_mon += b->tm_mon) >= 12) {
		a->tm_year += a->tm_mon / 12;
		a->tm_mon %= 12;
	}
	a->tm_mday += b->tm_mday;
	while (a->tm_mday > mday[a->tm_mon]) {
		a->tm_mday -= mday[a->tm_mon++];
		if (a->tm_mon > 11) {
			a->tm_mon = 0;
			mday[1] = 28 + leap(++a->tm_year);
		}
	}
}

int leap(year)
int	year;
{
	return(((1900+year) % 4 == 0 && (1900+year) % 100 != 0 || (1900+year) % 400 == 0));
}

#ifdef _OSF
/*
 * make job file from user enviro + stdin
 *
 * AIX code replaced by (almost) straight SYS V.2 code.
 */
static copy()
{
	register int c;
	register FILE *pfp;
	register FILE *xfp;
	register char **ep;
	char	dirbuf[PATH_MAX];
	char	tmpchar;
	char *getwd();
	mode_t um;
	char *val;
	char	shellname[20];
	char	*shellenv[2];
	char	*mailmsg1, *mailmsg2; /* temporary buffer for -m flag	*/
	extern char **environ;

	printf(": %s job\n",jobtype ? "batch" : "at");
	for (ep=environ; *ep; ep++) {
		if (strncmp(*ep, "TERMCAP=", 8) == 0)
			continue;
		if (strncmp(*ep, "TERM=", 5) == 0) {
			printf("TERM='dumb'\n");
			printf("export TERM\n");
			continue;
		}
		if ( strchr(*ep,'\'')!=NULL )
			continue;
		if ((val=strchr(*ep,'='))==NULL)
			continue;
		if (strncmp(*ep, "SHELL=", 6) == 0) {
			strcpy(shellname, ep);
			shellenv[0] = shellname;
			shellenv[1] = NULL;
			replace_shell(shellenv);
			printf("export SHELL; %s\n", shellname);
			continue;
		}
		*val++ = '\0';
		printf("export %s; %s='%s'\n",*ep,*ep,val);
		*--val = '=';
	}

	if (mflag) {
		/* If -m flag was specified, at mails that to the user. */
		if ((mailmsg1 = (char *)malloc(MFLAG_BUFF)) != NULL) {
        		sprintf(mailmsg1, MSGSTR(MS_MAILJOB, MAILJOB), job);
			if ((mailmsg2 = (char *)malloc(MFLAG_BUFF)) != NULL) {
        			sprintf(mailmsg2,"echo \"%s\" | mail %s",
					mailmsg1, login);
        			write(fd, mailmsg2, strlen(mailmsg2));
        			write(fd, "\n", 1);
				free(mailmsg1);
				free(mailmsg2);
			}
		}
	}

	if((pfp = fopen(pname1,"r")) == NULL && (pfp=fopen(pname,"r"))==NULL)
		atabort(MSGSTR(MS_NOPROTO, NOPROTO));

	um = umask(0);
	(void) umask(um);
	while ((c = getc(pfp)) != EOF) {
		if (c != '$')
			putchar(c);
		else switch (c = getc(pfp)) {
		case EOF:
			goto out;
		case 'd':
			dirbuf[0] = '\0';
			if (getwd(dirbuf) == NULL)
				atabort(MSGSTR(MS_CANTCWD, CANTCWD));
			printf("%s", dirbuf);
			break;
		case 'l':
			printf("%ld",ulimit(UL_GETFSIZE,-1L));
			break;
		case 'm':
			printf("%o", um);
			break;
		case '<':
			printf("%s << 'QAZWSXEDCRFVTGBYHNUJMIKOLP'\n",
				shellname+6);
			if (exeflag && (jobname[0] != '\0')) {
				printf("%s\n", jobname);
				break;
			}
			xfp = stdin;		/* job from stdin by default */
			if (jobname[0] != '\0') {
  				/* job from a file */
				if ((xfp=fopen(jobname, "r")) == NULL) {
					perror(jobname);
				}

			}
			while ((c = getc(xfp)) != EOF) {
				tmpchar = c;
				printf("%c",tmpchar);
			}
			/* don't put on single quotes, csh doesn't like it */
			printf("QAZWSXEDCRFVTGBYHNUJMIKOLP\n");
			break;
		case 't':
			printf(":%lu", when);
			break;
		default:
			putchar(c);
		}
	}
out:
	fclose(pfp);
}
#else /* !_OSF */
/*
 * make job file from user enviro and cred info + stdin
 * This is AIX code.
 */
static copy(fd)
int fd;
{
        register int  c;

        char 	**cred, **enviro;
        char 	tmpchar;
        char 	space[100];
        char 	pchPath[MAXPATHLEN+2];
	char	*mailmsg1, *mailmsg2; /* temporary buffer for -m flag	*/
        mode_t 	mask;
	FILE	*xfp;			/* open a script file	*/

        if (cred = getpcred(0))  {
                while(*(cred)) {
                        write(fd, *cred, strlen(*cred)+1);
                        cred++;
                }
                write(fd, "\0\n", 2);
        }

        if (enviro = getpenv(PENV_USR | PENV_SYS)) {
                replace_shell(enviro);
                while(*(enviro)) {
                        write(fd, *enviro, strlen(*enviro)+1);
                        enviro++;
                }
                write(fd, "\0\n", 2);
        }
        mask = umask(0666);
        sprintf(pchPath,"umask %.3o\n",mask);
        mask = umask(mask);
        write(fd, pchPath, strlen(pchPath));
        getcwd(pchPath,MAXPATHLEN+2);
        write(fd, "cd ", 3);
        write(fd, pchPath, strlen(pchPath));
        write(fd, "\n", 1);
	
	if (mflag) {
		/* If -m flag was specified, at mails that to the user. */
		if ((mailmsg1 = (char *)malloc(MFLAG_BUFF)) != NULL) {
        		sprintf(mailmsg1, MSGSTR(MS_MAILJOB, MAILJOB), job);
			if ((mailmsg2 = (char *)malloc(MFLAG_BUFF)) != NULL) {
        			sprintf(mailmsg2,"echo \"%s\" | mail %s",
					mailmsg1, login);
        			write(fd, mailmsg2, strlen(mailmsg2));
        			write(fd, "\n", 1);
				free(mailmsg1);
				free(mailmsg2);
			}
		}
	}

        if (jobname[0] == '\0') {
                while ((c = getchar()) != EOF) {
                        tmpchar = c;
                        if (write (fd, &tmpchar, 1) != 1)
				wrabort();
                }
        } else if (exeflag == TRUE) {
		/* jobname file is a command.	*/
                if (write (fd, jobname, strlen(jobname)) != strlen(jobname))
			wrabort();
                tmpchar = (char )0x0a;
                if (write (fd, &tmpchar, (unsigned)1) != 1)
			wrabort();
        } else {
		/* jobname file is a script file.*/
		if ((xfp=fopen(jobname, "r")) == NULL) {
			perror(jobname);
		}
		while ((c = getc(xfp)) != EOF) {
			tmpchar = c;
                        if (write (fd, &tmpchar, 1) != 1)
				wrabort();
		}
	}

	if (close(fd) < 0)
		wrabort();
}

static wrabort()
{
	fprintf(stderr,MSGSTR(MS_CANTOPEN, CANTOPEN), ATDIR); /*MSG*/
	unlink(tfname);
	unlink(jname);
	exit(1);
}
#endif /* _OSF */

/*
 * If the user specified on the command line, that the job should
 * be executed in a specific shell then change the default shell
 */
static replace_shell(env_str)
char **env_str;
{
        register int i;
	char	*shell_str;

        for (i=0;env_str[i]!=NULL;i++) {
                if ( !strncmp(env_str[i],"SHELL=",6) ) {
		    if ( sh_name != NULL )
			env_str[i] = sh_name;
		    else {
			shell_str = env_str[i] + 6;
				if ( *shell_str == '\0' )
                                	strcpy(env_str[i],SHELL);
		    }
                    return(0);
                }
        }
        return(1);
}

#define	SHELL_ENV	32
static char *select_shell(fbsh,fcsh,fksh)
int fbsh,fcsh,fksh;
{
        register int i;
	char	*shell_str;
	int	select_sh=0;
	char	*select_str, *shell_env;
	char	*exsh_s;

	select_str = NULL;
	shell_env = NULL;

	if (fbsh) {
		exsh_s = BSH;
		select_sh = 1;
	} else if (fcsh) {
		exsh_s = CSH;
		select_sh = 1;
	} else if (fksh) {
		exsh_s = KSH;
		select_sh = 1;
	}

	if ( select_sh ) {
		if ( select_str == NULL ) {
		    if ( access(exsh_s, X_OK) != -1 )
			select_str = exsh_s;
		    else {
			fprintf(stderr, MSGSTR(MS_BADSTAT, BADSTAT), exsh_s);
			exit(1);
		    }
		}
		if ((shell_env = malloc(SHELL_ENV + 1)) == NULL)
			atabort(MSGSTR(MS_MALLOC,MALLOC));
		strncpy( shell_env, "SHELL=", 6 );
		strcpy( &shell_env[6], select_str );
	}

	return ( shell_env );
}

/*
 * NAME: usage
 * FUNCTION: display a usage statement to the user
 */
static usage()
{
    fprintf(stderr, MSGSTR(MS_USAGE4,
	"Usage: at [-c|-k|-s|-q {a|b|e|f}] [-m] [-f file] time [day] [inc]\n"));
    fprintf(stderr, MSGSTR(MS_USAGE8,
	"Usage: at [-c|-k|-s|-q {a|b|e|f}] [-m] [-f file] -t [[CC]YY]MMDDhhmm[.SS]\n"));
    fprintf(stderr, MSGSTR(MS_USAGE5,
	"Usage: at -l [-o] [ job ... | -q {a|b|e|f} ]\n"));
    fprintf(stderr, MSGSTR(MS_USAGE6,
	"Usage: at -r [-Fi] job ...| [-u user]\n"));
    fprintf(stderr, MSGSTR(MS_USAGE7,
	"Usage: at -n [user]\n"));
    exit(1);
}

/*
 * NAME: gtime
 *                                                                    
 * RETURN VALUE:	1 failure
 *			0 success
 */  
static int
gtime()
{
	register int y, t, cc;
	int d, h, m, ss;
	long nt;
	int ccflg = 0;
	int minarg = 0;
	char *s1;
	char yy[4];
	char	*cp;

	cp = argp;
	minarg = strlen(cp);
	/*
	 * get seconds
	 */
	if ( (s1 = strrchr(cp, '.')) != NULL) {
		s1++;
		ss = atoi(s1);
		if ( (minarg < 11) || (ss < 0) || (ss > 61) ) {
	    	    fprintf(stderr,"at: %s", MSGSTR(MS_BADDATE,
			"The specified date is not in the correct format.\n"));
		    return(1);
		}
		minarg-=3;
	}
	else
		ss = 0;

	if (minarg != 12 && minarg != 10 && minarg != 8 ) {
	    fprintf(stderr, "at: %s", MSGSTR(MS_BADDATE,
		"The specified date is not in the correct format.\n"));
  	    return(1);
	}

	/*
	 * calculate the century and the year
	 */
	if (minarg == 8) {
		/*
	 	 * year and century not given (MMDDhhmm)
	 	 */
		(void) time(&nt);
		y = localtime(&nt)->tm_year + 1900;
	} else if (minarg == 10) {
		/*
		 * century not given (YYMMDDhhmm)
		 */
		y = gpair(&cp);		/* get the year */
		if (y>68 && y<=99)
			cc = 19;
		else if	 (y>=0 && y<69)
			cc = 20;
		sprintf(yy,"%02d%02d", cc, y);
		y = atoi(yy);
	} else { /* minarg = 12 */
		/* 
		 * year and century given (CCYYMMDDhhmm)
		 */
		cc = gpair(&cp);	/* get the century 	*/
		y = gpair(&cp);		/* get the year 	*/
		sprintf(yy,"%02d%02d", cc, y);
		y = atoi(yy);
	}

	if( y < 1970 || y > 2037 ) {
		/* The valid year is 1970 - 2037.       */
		fprintf(stderr, "at: %s", MSGSTR(MS_BYEAR,
			"bad year\n"));
	    	return(1);
	}

	t = gpair(&cp);		/* get the month */
	d = gpair(&cp);		/* get the day   */

	if(t<1 || t>12 || d<1 || d>31) {
	    fprintf(stderr, "at: %s", MSGSTR(MS_BDATE, 
		"There is an error in the month or the day specification.\n"));
	    return(1);
	}

	h = gpair(&cp);		/* get the hours   */
	m = gpair(&cp);		/* get the minutes */

	if(h == 24) {
		h = 0;
		d++;
	}
	if (m == -1) {
		/*
		 * back up and make sure
		 * tflag didn't mess up ptr to
		 * cbp
		 */
		cp--;
		m = gpair(&cp);
	}
	if(m<0 || m>59 || h<0 || h>23) {
	    fprintf(stderr, "at: %s", MSGSTR(MS_BTIME,
		"There is an error in the minute or hour specification.\n"));
	    return(1);
	}

	at.tm_sec = ss;
	at.tm_min = m;
	at.tm_hour = h;
	at.tm_mday = d;
	at.tm_mon = t - 1;
	at.tm_year = y - 1900;
	at.tm_isdst = -1;

	return(0);
}

/*
 * NAME: gpair
 *                                                                    
 * FUNCTION: 		Converts the first two characters in a given parameter
 *			string into an integer.
 *                                                                   
 * RETURN VALUE:	success:  integer cooresponding to the first two digits
 *			failure:  -1
 */  
static int
gpair(cpp)
char	**cpp;
{
	register int c, d;
	register char *cp;

	cp = *cpp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	(*cpp)++;
	(*cpp)++;
	return (c+d);
}

