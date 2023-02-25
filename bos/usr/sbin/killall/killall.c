static char sccsid[] = "@(#)09  1.12.1.2  src/bos/usr/sbin/killall/killall.c, cmdcntl, bos41B, 9504A 1/4/95 10:13:42";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 * Kill all processes except kprocs, 0, 1, this one, and its ancestors.
 *		killall 	sends SIGKILL
 *		killall [-]n	sends signal n
 *		killall -	sends SIGTERM, then SIGKILL to all killable
 *				processes that are still alive after a delay.
 *		killall - [-]n	as above, but signal n instead of SIGTERM
 */                                                                   

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/types.h>
#include <procinfo.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <locale.h>

#include <nl_types.h>
static nl_catd catd;
#include "killall_msg.h"
#define MSGS(n,s)	catgets(catd,MS_KILLALL,n,s)

#define PROCSIZE	sizeof (struct procinfo)

/* Intervals between testing for all processes killed */
static char sleepsched[] = {1, 2, 2, 5, 10, 10};


main(argc, argv)
char *argv[];
{
	register long i, j;
	register pid_t pid;
	long nleft;
	char *p;
	int signo = SIGKILL;
	int clobber = 0,sig;
		/* clobber != 0 to send SIGKILL to signallable processes
		 * that survive the sleep schedule.
		 */
	long nproc=0;			/* Number of active processes.      */
	struct procinfo	*Proc;		/* Pointer to array of active procs */
	long multiplier=1;		/* Factor in for busy system.       */
	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_KILLALL,NL_CAT_LOCALE);

			/* Minus and then a number says what signal */
			/* to send; whereas a minus alone says to   */
			/* follow with SIGTERM if the first signal  */
			/* fails.                                   */

	for (i = 1; i < argc; i++)
	{	p = argv[i];
		if (p[0] == '-')
		{	p++;
			if (*p == '\0')
			{	++clobber;
				if (signo == SIGKILL)
					signo = SIGTERM;
				continue;
			}
			else if ((sig=signum(p)) != -1)
			{	signo=sig;
				continue;
			}
			else
          		{       write2(MSGS(USAGE,"Usage: killall [-] [-Signal]\n")); /*MSG*/
				exit(254);
			}
		}
		else
          	{       write2(MSGS(USAGE,"Usage: killall [-] [-Signal]\n"));  /*MSG*/
			exit(254);
		}
	}

    pid = getpid();		/* This is me! */

    while (pid != 0) 		/* zero indicates we will try a second time. */
    {
	if ((Proc = (struct procinfo *) malloc ((size_t)sizeof(unsigned long))) == NULL)
	{
		perror ("malloc");
		exit (1);
	}
	while (((nproc = getproc (Proc, nproc, PROCSIZE)) == -1) &&
		errno == ENOSPC)
	{
		nproc = *(long *) Proc;      /* num of active proc structures */
		nproc += (multiplier <<= 1); /* Get extra in case busy system */

		Proc = (struct procinfo *) realloc ((void *)Proc, (size_t)(PROCSIZE * nproc));
		if ( Proc == NULL) 
		{		/* We ran out of space before we could read */
				/* in the entire proc structure.            */
			perror ("realloc");
			exit (1);
		}
	}

		/* Sanity check.  The first process is always the swapper */
		/* and the second is init.  This command will break if we */
		/* change this standard.                                  */

	if (Proc[0].pi_pid != 0 || Proc[1].pi_pid != 1)
	{       write2(MSGS(NOPRTBL,"Could not find process table.\n")); /*MSG*/
		exit (1);
	}


/*
 * Chase our ancestor list, marking all the ancestors as not-to-be-killed by
 * setting the pid field in our array to 0.  Also don't kill init, even if it
 * is not our ancestor (on systems where there is an underlying operating
 * system, there could be a master process that runs shutdown directly).
 * Also skip kernel processes, indicated by SSYS (now the SKPROC) flag.
 */
	while (pid)  /* eventually we get to process 0 */
	{	for (i = 0; i < nproc; i++)	/* don't kill ancestors */
		{	
			if (Proc[i].pi_pid == pid)
			{	Proc[i].pi_pid = 0;
				pid = Proc[i].pi_ppid;
				break;
			}
		}
		if(i >= nproc) { /* fell through loop?) */
			static tried_once = 0;
			if(tried_once++)
					exit(1);
			write2(MSGS(NOANC,"Cannot find an ancestor; retrying.\n"));	/*MSG*/
			break;
		}
	}
    }

#ifndef INITPID
#define INITPID 1
#endif
/*
 * Try to signal the unmarked processes.
 */
	for (i=0; i<nproc; i++)
		if (pid = Proc[i].pi_pid)
		{
			if (pid == INITPID ||
			Proc[i].pi_flag & SKPROC ||    /* kernel process  */
			    kill(pid, signo) != 0)   /* If not killable */
				Proc[i].pi_pid = 0;  /* Mark it not 2B killed */
		}

	if (clobber == 0)			     /* Didn't specify '-' */
		exit(0);

	for (j = 0; j < sizeof sleepsched; ++j)
	{       /* See if anyone killable is left */
		for (nleft = i = 0; i < nproc; i++)
		{	pid = Proc[i].pi_pid;
			if (pid != 0 && kill(pid, 0) == 0)
				++nleft;
			else
				Proc[i].pi_pid = 0;
		}
		if (nleft == 0)
			exit(0);
/* Remove this after sleep definition is out of included include file */
#undef sleep
		sleep( (unsigned) sleepsched[j] );
	}

	for (i=0; i<nproc; i++)
	{       if (pid = Proc[i].pi_pid)
		{
			kill(pid,SIGKILL);
		}
	}

	exit(0);
	return(0); /* keep lint happy */
}

/* Write strings.  Last one expected to end in \n. */
/*VARARGS*/
static write2(s1, s2, s3, s4)
char *s1;
char *s2;
char *s3;
char *s4;
{
	w("killall: ") && w(s1) && w(s2) && w(s3) && w(s4);
}

/* Write string onto fd 2, return nonzero if no newline */
static w(s)
char *s;
{
	register int len;

	len = strlen(s);
	write(2, s, len);
	return(s[len-1] != '\n');
}
