static char sccsid[] = "@(#)89	1.25  src/bos/usr/sbin/rdump/dumpoptr.c, cmdarch, bos411, 9428A410j 4/26/93 17:21:43";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
static char sccsid[] = "(#)dumpoptr.c	5.1 (Berkeley) 6/5/85";
#endif not lint
*/

#include <nl_types.h>
#include "dumpi_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_DUMP,N,S)
nl_catd catd;

#include <time.h>
#include "dump.h"
#include <sys/audit.h>
#include <sys/priv.h>
#include <errno.h>

struct	pfstab {
	struct	pfstab *pf_next;
	struct	fstab *pf_fstab;
};

int alarmcatch(void);
void (*dump_signal()) ();
struct pfstab *getfstab();

extern int Batch_flag;
 


/*
 *	Query the operator; This fascist piece of code requires
 *	an exact response.
 *	It is intended to protect dump aborting by inquisitive
 *	people banging on the console terminal to see what is
 *	happening which might cause dump to croak, destroying
 *	a large number of hours of work.
 *
 *	Every 2 minutes we reprint the message, alerting others
 *	that dump needs attention.
 */
int	timeout;
char	*attnmessage;		/* attention message */
query(question, default_ans )
	char	*question;
	int 	default_ans;
{
	char	replybuffer[64];
	int	back;
	FILE	*mytty;
	int	i;
	char	*ptr;
	char * catmsg_str; 


		/* if non-interactive mode , return default answer */
	if ( Batch_flag )
		return ( default_ans );		

	if ( (mytty = fopen("/dev/tty", "r")) == NULL){
		msg(MSGSTR(FOPENTTY, "fopen on /dev/tty fails\n"));
		dumpabort();
	}
	attnmessage = question;
	timeout = 0;
	alarmcatch();
	catmsg_str = MSGSTR(YESNO,""); /* This will tell if catalog is being used. */
	for(;;){
		if ( fgets(replybuffer, 63, mytty) == NULL){
			if (ferror(mytty)){
				clearerr(mytty);
				continue;
			}
		}
		ptr = replybuffer;
		while (*ptr != '\n') ptr++;
		*ptr = '\0';
		 
        if (*catmsg_str == NULL)  /* no msg. catalog, force English response.*/ 
		  {
			if (strcmp(replybuffer,"yes") == 0) 
			  i = 1;
            else
			  if (strcmp(replybuffer,"no") == 0)
				i = 0;
			  else 
				i = -1;  
           }
        else
	      i = rpmatch(replybuffer);
		if (i == 1) {back = 1; goto done;}
		else if (i == 0) {back = 0; goto done;}
		else {
			msg(MSGSTR(YESNO,"\"yes\" or \"no\"?\n"));
			alarmcatch();
		}
	}
    done:
	/*
	 *	Turn off the alarm, and reset the signal to trap out..
	 */
	alarm(0);
	if (dump_signal(SIGALRM, (void (*)(int))sigalrm) == SIG_IGN)
		dump_signal(SIGALRM, SIG_IGN);
	fclose(mytty);
	return(back);
}


/*
 *	Alert the console operator, and enable the alarm clock to
 *	sleep for 2 minutes in case nobody comes to satisfy dump
 */
alarmcatch(void)
{
	if (timeout)
		msgtail("\n");
	msg(MSGSTR(NEEDATT, "NEEDS ATTENTION: %s: (\"yes\" or \"no\") "),
		attnmessage);
	dump_signal(SIGALRM, (void (*)(int))alarmcatch);
	alarm(120);
	timeout = 1;
}
/*
 *	Here if an inquisitive operator interrupts the dump program
 */
interrupt(void)
{
	msg(MSGSTR(INTR, "Interrupt received.\n"));
	if (query(MSGSTR(ABORTDUM, "Do you want to abort backup?"), YES))
		dumpabort();
	dump_signal(SIGINT, (void (*)(int))interrupt);
}

void  (*dump_signal(int signo, void  (*fun)(int)))(int)
/* signo:    signal whose action is to be changed   */
/* fun  :    pointer to function or SIG_IGN/SIG_DFL */
{
	static sigset_t mask[NSIG];	/* remembered signal handler masks */
	static int 	flags[NSIG];	/* remembered signal action flags */

	struct sigaction act;	/* new signal action structure */
	struct sigaction oact;  /* returned signal action structure */
	void 	(*rc)(int);	/* return value */


	/* Signal bound checking is required to prevent an invalid    
	 * memory access while accessing the mask and flags arrays.
	 *
	 * SIGCONT handling is required here as BSD sigvec does not 
	 * allow this signal to be ignored, whereas POSIX sigaction
	 * does.
	 */
	if(signo <= 0 || signo >= NSIG || (signo==SIGCONT && fun==SIG_IGN))
	{
		errno=EINVAL;
		return(BADSIG);
	}

	act.sa_handler = (void (*)(int))fun;
	act.sa_mask = mask[signo];
	act.sa_flags = (flags[signo] & ~SA_OLDSTYLE) | SA_RESTART;


	/* use the sigaction() system call to set new and get old action */

	if ( sigaction(signo, &act, &oact) == 0 )
	{	
		rc = (void (*)(int))oact.sa_handler;


		/* if the "remembered" mask and flags were out of sync with
		 * real mask and flags, update "remembered" stuff and put
		 * mask and flags back to correct (unchanged) values */

		if ( (mask[signo].losigs != oact.sa_mask.losigs) ||
		     (mask[signo].hisigs != oact.sa_mask.hisigs) ||
	             (flags[signo] != oact.sa_flags) )
		{
			act.sa_mask = oact.sa_mask;
			mask[signo] = oact.sa_mask;

			/* Always forcing SA_RESTART in signal is incorrect
			 * however for release one we will let this pass.
			 * P36643.
			 */
			act.sa_flags = (oact.sa_flags & ~SA_OLDSTYLE) 
							| SA_RESTART;
			flags[signo] = act.sa_flags;
			sigaction(signo, &act,(struct sigaction *)NULL);
		}
	}
	else
		return(BADSIG);

	return (rc);
}

/*
 *	print out an estimate of the amount of time left to do the dump
 */

time_t	tschedule = 0;

timeest()
{
	time_t	tnow, deltat;

	time (&tnow);
	if (tnow >= tschedule){
		tschedule = tnow + 300;
		if (blockswritten < 500)
			return;	
		deltat = (((tnow - tstart_writing) * 100) /
			 ((blockswritten*100.0)/esize)) -
			 (tnow - tstart_writing);
			
		if (deltat >= 0) {
			msg(MSGSTR(FINISH,
				"%3.2f%% done, finished in %d:%02d\n"),
				(blockswritten*100.0)/esize,
				deltat/60, (deltat%60));
		} else {
			deltat = -deltat;
			msg(MSGSTR(FINISH2,
				"%3.2f%% done, overdue by %d:%02d\n"),
				(blockswritten*100.0)/esize,
				deltat/60, (deltat%60));
		}
	}
}

	/* VARARGS1 */
	/* ARGSUSED */
msg(fmt, a1, a2, a3, a4, a5)
	char	*fmt;
	int	a1, a2, a3, a4, a5;
{
#ifdef RDUMP
	fprintf(stderr,MSGSTR(RDUMP1, "rdump: "));
#else
	fprintf(stderr,MSGSTR(DUMP1, "backup: "));
#endif
#ifdef TDEBUG
	fprintf(stderr,"pid=%d ", getpid());
#endif
	fprintf(stderr, fmt, a1, a2, a3, a4, a5);
	fflush(stdout);
	fflush(stderr);
}

	/* VARARGS1 */
	/* ARGSUSED */
msgtail(fmt, a1, a2, a3, a4, a5)
	char	*fmt;
	int	a1, a2, a3, a4, a5;
{
	fprintf(stderr, fmt, a1, a2, a3, a4, a5);
}
/*
 *	Tell the operator what has to be done;
 *	we don't actually do it
 */

struct fstab *
allocfsent(fs)
	register struct fstab *fs;
{
	register struct fstab *new;
	register char *cp;
	char *malloc();

	new = (struct fstab *)malloc(sizeof (*fs));
	cp = malloc(strlen(fs->fs_file) + 1);
	strcpy(cp, fs->fs_file);
	new->fs_file = cp;
	cp = malloc(strlen(fs->fs_type) + 1);
	strcpy(cp, fs->fs_type);
	new->fs_type = cp;
	cp = malloc(strlen(fs->fs_spec) + 1);
	strcpy(cp, fs->fs_spec);
	new->fs_spec = cp;
	new->fs_passno = fs->fs_passno;
	/*  new->fs_freq = fs->fs_freq;  fs_freq is not assured a value until /etc/fstab is implemented */
	new->fs_freq = 0;
	return (new);
}

static	struct pfstab *table = NULL;

struct pfstab *getfstab()
{
	register struct fstab *fs;
	register struct pfstab *pf;

	if (setfsent() == 0) {
		msg(MSGSTR(CANTODT, "Can't open %s for backup table information.\n"), FSYSname);
		return;
	}
	while (fs = getfsent()) {
		if (!isjfs(fs->fs_file)) /* Only return jfs filesystems */
			continue;
		if (strcmp(fs->fs_type, FSTAB_RW) &&
		    strcmp(fs->fs_type, FSTAB_RO) &&
		    strcmp(fs->fs_type, FSTAB_RQ))
			continue;
		fs = allocfsent(fs);
		pf = (struct pfstab *)malloc(sizeof (*pf));
		pf->pf_fstab = fs;
		pf->pf_next = table;
		table = pf;
	}
	endfsent();
	return(pf);
}

isjfs(fsname)
char *fsname;
{
	static AFILE_t	attrfile = NULL;
	char	*attrval;

	if (attrfile == NULL)
		if ((attrfile = AFopen(FSYSname, MAXREC, MAXATR)) == NULL)
			return 0;
	attrval = AFgetatr(AFgetrec(attrfile, fsname), "vfs");

	return (!strcmp(attrval, "jfs"));
}

/*
 * Search in the fstab for a file name.
 * This file name can be either the special or the path file name.
 *
 * The entries in the fstab are the BLOCK special names, not the
 * character special names.
 * The caller of fstabsearch assures that the character device
 * is dumped (that is much faster)
 *
 * The file name can omit the leading '/'.
 */
struct fstab *
fstabsearch(key)
	char *key;
{
	register struct pfstab *pf;
	register struct fstab *fs;
	char *rawname();

	if (table == NULL)
		return ((struct fstab *)0);
	for (pf = table; pf; pf = pf->pf_next) {
		fs = pf->pf_fstab;
		if (strcmp(fs->fs_file, key) == 0)
			return (fs);
		if (strcmp(fs->fs_spec, key) == 0){
		        statfs(fs->fs_file, &fs_statbuf, sizeof(struct statfs));
		  	if(fs_statbuf.f_vfstype != MNT_JFS)
				continue;
			return (fs);
		}
		if (strcmp(rawname(fs->fs_spec), key) == 0)
			return (fs);
		if (key[0] != '/'){
			if (*fs->fs_spec == '/' &&
			    strcmp(fs->fs_spec + 1, key) == 0)
				return (fs);
			if (*fs->fs_file == '/' &&
			    strcmp(fs->fs_file + 1, key) == 0)
				return (fs);
		}
	}
	return (0);
}

/*
 *	Tell the operator what to do
 */
lastdump(arg)
	char	arg;		/* w ==> just what to do; W ==> most recent dumps */
{
			char	*lastname;
			char	date[100];
	register	int	i;
			time_t	tnow;
	register	struct	fstab	*dt;
/*			int	dumpme;     commented out until fs_freq has good value, when /etc/fstab is implemented */
	register	struct	idates	*itwalk;

	int	idatesort();
	static struct pfstab *pf;

	time(&tnow);
	pf = getfstab();		/* /etc/fstab input */
	inititimes();		/* /etc/dumpdates input */
	qsort(idatev, nidates, sizeof(struct idates *), idatesort);

/*	if (arg == 'w')
		fprintf(stdout, MSGSTR(DUMPFSYS, "Backup these file systems:\n"));
	else
 commented out until fs_freq has good value - when /etc/fstab is implemented */
	fprintf(stdout, MSGSTR(LASTDU, "Last backup(s) done (Backup '>' file systems):\n"));
	lastname = "??";
	ITITERATE(i, itwalk){
		if (strncmp(lastname, itwalk->id_name, sizeof(itwalk->id_name)) == 0)
			continue;
        {
			struct tm *t;

			t = localtime(&itwalk->id_ddate);
			strftime(date, 64, "%a %sD %sT\0", t);
        }
		lastname = itwalk->id_name;
		dt = fstabsearch(itwalk->id_name);
	 /* dumpme = (  (dt != 0)
			 && (dt->fs_freq != 0)
			 && (itwalk->id_ddate < tnow - (dt->fs_freq*DAY)));
		if ( (arg != 'w') || dumpme)
    commented out until fs_freq field has a good value - when /etc/fstab is implemented */
		if  (arg == 'W')
		  fprintf(stdout,MSGSTR(DUMPLL, "%c %8s\t(%6s) Last backup: Level %c, Date %s\n"),
		/*	dumpme && (arg != 'w') ? '>' : ' ',    commented out as above */
	        ' ',
			itwalk->id_name,
			dt ? dt->fs_file : "",
			itwalk->id_incno,
			date
		    );
	}
}

int	idatesort(p1, p2)
	struct	idates	**p1, **p2;
{
	int	diff;

	diff = strncmp((*p1)->id_name, (*p2)->id_name, sizeof((*p1)->id_name));
	if (diff == 0)
		return ((*p2)->id_ddate - (*p1)->id_ddate);
	else
		return (diff);
}
