static char sccsid[] = "@(#)14  1.8  src/bos/usr/sbin/uprintfd/uprintfd.c, cmdoper, bos411, 9428A410j 11/22/93 12:33:01";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: uprintfd
 *
 * ORIGINS: 27
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
 * Copyright (c) 1980,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/types.h>
#include <sys/uprintf.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/signal.h>
#include <sys/unixmsg.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <procinfo.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <a.out.h>
#include <locale.h>
#include <nl_types.h>

#define EVARBSIZE	4096		/* evars buffer size for getevars() */
#define ARGSIZE		(UP_MAXSTR+1)	/* max print argument size */
#define MSGSIZE		(ARGSIZE * (UP_MAXARGS+1))  /* message buffer size */
#define	MNTCTLSZ	1024		/* initial mntctl() buffer size */

/* enviromental variable strings.
 */
#define	EV_LC_MESSAGES	"LC_MESSAGES="		
#define	EV_LC_ALL	"LC_ALL="
#define	EV_NLSPATH	"NLSPATH="
#define	EV_LANG		"LANG="

extern int errno;
nl_catd catd;

char *geteval();
char *evmatch();

/*
 * NAME:        main() for uprintfd
 *
 * FUNCTION:    deliver uprintf messages to processes.
 *
 *		uprintfd calls the upfget() system call to get uprintf
 *		information for a queued message. after retrieving the
 *		information, a child process is created to build the
 *		message within the target process's NLS environment.
 *		once the message has been built, the child process
 *		calls the upfput() system call to write the message
 *		to the target process's controlling tty. following
 *		the call to upfput(), the child process exits.  the
 *		parent process continues to get messages and create
 *		child processes to deliver the messages.
 *
 *		NOTE - the parent process is a special system process. if
 *		a non-zero return code is returned from upfget() with an
 *		associated errno other than EINTR, the parent process MUST
 *		exit.  Also, the parent process MUST exit if the a child
 *		process can not be created (fork() fails).
 *
 * PARAMETERS:
 *              NONE.
 *
 * RETURN VALUE:
 *		-1 	- error encountered.
 *
 */

main (argc, argv)
int argc;
char **argv;
{
	struct upfbuf upfbuf;
	struct upfdata upfdata;
	char message[MSGSIZE];
	int rc;

	/* give up controlling tty.
	 */
	if (setpgrp() == -1)
		exit(-1);

	/* ignore the death of children.
	 */
	if (sigignore(SIGCHLD))
		exit(-1);

	/* do work.
	 */
	for (;;)
	{
		/* get uprintf information.
		 */
		if (rc = upfget(&upfbuf,&upfdata))
		{
			/* continue if upfget() was interrupted.
			 */
			if (errno == EINTR)
				continue;
		
			/* bad return code.  we MUST terminate.
			 */
			exit(-1);
		}
		  
		/* create a slave process to put the message.
		 */
		if ((rc = fork()) == 0)
		{
			setupenv(&upfbuf);

			(void)setlocale(LC_ALL,"");

			/* construct the message.
			 */
			if (rc = setupmsg(&upfbuf,&upfdata,message))
				exit(-1);

			/* put the message to the proccess controlling
			 * tty.
			 */
			if (upfput(message))
				exit(-1);
			else
				exit(0);
		}
		else
		{
			/* if fork() failed we MUST terminate.
			 */
			if (rc == -1)
				exit(-1);
		}
	}
}

/*
 * NAME:        setupmsg(up,ud,message)
 *
 * FUNCTION:    construct a message string for the specified upfbuf
 *		and upfdata.
 *
 *
 * PARAMETERS:
 *              up	- pointer to upfbuf
 *              ud	- pointer to uprintf data
 *              message	- pointer to buffer into which the constructed
 *			  message should be placed
 *
 * RETURN VALUE:
 *              0 	- message successful construct
 *		nonzero - error encountered - message not constructed
 *
 */

setupmsg(up,ud,message)
struct upfbuf *up;
struct upfdata *ud;
char *message;
{
	char *fmt;
	char *prtarg[UP_MAXARGS];
	int arg, rc;
	char *dptr;     /*  Pointer to parse the print arguments in upfdata buffer 
			    defined in uprintf.h */
	char **argp;	/*  Pointer to access the argument pointed to by dptr */

	/* check if this is an NLS message.
	 */
	if (up->up_flags & UP_NLS)
	{

		catd = catopen(ud->upd_NLcatname, NL_CAT_LOCALE);

		/* check if additional formatting is required.
		 */
		if (strcmp(ud->upd_NLcatname,MF_UNIX) == 0)
		{
			/* special handling for jfs messages.
			 */
			if (up->up_NLsetno == SET_SYSPFS)
			{
				/* get file system pathname for
				 * quota messages.
				 */
				if (up->up_NLmsgno >= MSG_SYSPFS_01 && 
				    up->up_NLmsgno <= MSG_SYSPFS_16)
				{
					if (rc = fsname(up,ud))
						return(rc);
				}
			}
		}
	}

	/* setup print arguments : 

	        Two pointers are defined to parse the upfdata buffer: 
	        char *dptr and char **argp.

	        dptr is required to parse the buffer correctly.
		argp is required to access the argument pointed to
		by dptr.  If the argument is a string, then get the
		pointer to the string; if the argument is a 
	        'by value' argument, then get its value.
		
		Note :  Just using argp to parse the buffer causes 
			incorrect computation of consecutive argument
			addresses because adding ARGSIZE to argp to 
			get the next argument causes the pointer to
			be moved by 4 times of ARGSIZE.        
	 */

	dptr = ud->upd_prtargs;
	for (arg = 0; arg < up->up_nargs; arg++)
	{
		argp = (char **)dptr;
		/*  check if the argument is a string argument or
		    a 'by value' argument */
		if (UPF_ISSTR(up->up_fmtvec,arg))
			prtarg[arg] = (char *)argp;
		else
			prtarg[arg] = *argp;
		dptr = dptr + ARGSIZE;
	}

	/* get the format string.
	 */
	if (up->up_flags & UP_NLS)
		fmt = NLcatgets(catd,up->up_NLsetno,up->up_NLmsgno,
					ud->upd_defmsg);
	else
		fmt = ud->upd_defmsg;

	/* build the message.
	 */
	sprintf(message,fmt,prtarg[0],prtarg[1],prtarg[2],prtarg[3],prtarg[4],
		prtarg[5],prtarg[6],prtarg[7]);


	if (up->up_flags & UP_NLS)
		catclose(catd);

	return(0);
}

/*
 * NAME:        setupenv(up)
 *
 * FUNCTION:    setup enviromental variables.
 *
 *
 * PARAMETERS:
 *              up	- pointer to upfbuf 
 *
 * RETURN VALUE:
 *              NONE
 *
 */

setupenv(up)
struct upfbuf *up;
{
	char *lang, *nlspath, *lc_all, *lc_messages;
	struct procinfo proc;
	char evarbuf[EVARBSIZE];
	
	/* no environment setup required if NLS not required.
	 */
	if (!(up->up_flags & UP_NLS))
		return;

	/* get environmental variables for the process.
	 */
	bzero(evarbuf,EVARBSIZE);
	proc.pi_pid = up->up_pid;
	if (getevars(&proc, sizeof(struct procinfo), evarbuf, EVARBSIZE))
		return;
	
	/* setup the LANG variable.
	 */
	if ((lang = geteval(EV_LANG,evarbuf,EVARBSIZE)) != NULL)
		putenv(lang);
	
	/* setup the NLSPATH variable.
	 */
	if ((nlspath = geteval(EV_NLSPATH,evarbuf,EVARBSIZE)) != NULL)
		putenv(nlspath);
	
	/* setup the LC_ALL variable.
	 */
	if ((lc_all = geteval(EV_LC_ALL,evarbuf,EVARBSIZE)) != NULL)
		putenv(lc_all);
	
	/* setup the LC_MESSAGES variable.
	 */
	if ((lc_messages = geteval(EV_LC_MESSAGES,evarbuf,EVARBSIZE)) != NULL)
		putenv(lc_messages);
	return;
}

/*
 * NAME:        geteval(str,buf,len)
 *
 * FUNCTION:    search buffer for a specified evar string.
 *		if found, a pointer to the string is return;
 *		otherwise, NULL is returned.
 *
 */

char *geteval(str, buf, len)
char *str;
char *buf;
int len;
{
        char *bp;
	
	for (bp = buf; bp < buf + len && *bp != '\0'; bp++)
	{
                if (evmatch(str,bp) != NULL)
                        return(bp);

                while (bp < buf + len && *bp != '\0')             
                        bp++;
	}

        return (NULL);
}

/*
 * NAME:        evmatch(s1,s2)
 *
 * FUNCTION:    check if evar string pointed to by s1 matches string
 *		pointed to by s2.  if they match, return pointer
 *		to evar value; otherwise, return NULL.
 *
 */

char *evmatch(s1, s2)
char *s1, *s2;
{
        while (*s1 == *s2++)
	{
                if (*s1++ == '=')
                        return(s2);
	}

        return(NULL);
}

/*
 * NAME:        fsname(up,ud)
 *
 * FUNCTION:    reformat uprintf for jfs quota message.  reformatting
 *		consists of finding the filesystem pathname for the
 *		fsid.dev contained with the uprintf data prtagrs and
 *		replacing the fsid.dev with the pathname.
 *
 * PARAMETERS:
 *              up	- pointer to upfbuf 
 *		ud	- pointer to uprintf data 
 *
 * RETURN VALUE:
 *              0	- uprintf data successfully formatted.
 *		1	- error occurred
 *
 */

fsname(up,ud)
struct upfbuf *up;
struct upfdata *ud; 
{
	struct vmount *vmountp, *vm;
	int size, nmounts, len ;
	char *fsname;
	uint dev;

	size = MNTCTLSZ;

	/* get mount info for all filesystems.
	 */
	for(;;)
	{
		/* get storage for the info.
		 */
		if ((vmountp = (struct vmount *)malloc(size)) == NULL)
			return(1);

		/* get mount info.
		 */
	  	nmounts = mntctl(MCTL_QUERY, size, vmountp);

		/* check for revision problems.
		 */
		if (nmounts > 0 && vmountp->vmt_revision != VMT_REVISION)
			return(1);

		/* all ok - continue on.
		 */
		if (nmounts > 0)
		{
			break;
		}
		/* try again with a bigger buffer.
		 */
		else if (nmounts == 0)
		{
			size = vmountp->vmt_revision;
			free((void *)vmountp);
		}
		/* error return from mntctl.
		 */
		else
		{
			return(1);
		}
	}

	dev = *((uint *)ud->upd_prtargs);

	/* search the mount info for the filesystem of interest.
	 */
	for (vm = vmountp; nmounts; nmounts--,
		vm = (struct vmount *)((char *)vm + vm->vmt_length))
	{
		if (vm->vmt_fsid.fsid_dev == dev) 
		{
			/* make sure that it is a jfs filesystem.
			 */
			if (vm->vmt_gfstype != MNT_JFS)
	 			return(1);
			
			/* get the pathname and pathname length.
			 */
			fsname = (char *) vmt2dataptr(vm, VMT_STUB);
			if ((len = strlen(fsname)) > UP_MAXSTR)
				return(1);

			/* copy the pathname to upfdata and update
			 * the format vec to reflect the string.
			 */
			bcopy(fsname,ud->upd_prtargs,len+1);
			getfsstr(up->up_NLmsgno,ud->upd_defmsg);
			up->up_fmtvec |= UPF_VBIT(0);

			return(0);
		}
	}
	return(1);
}

/*
 * NAME:        getfsstr(msgno,defmsg)
 *
 * FUNCTION:    get jfs disk quota msg for msgno.
 *
 */

getfsstr(msgno,defmsg)
int msgno;
char *defmsg;
{
	char *msg;
	int len;

	switch(msgno)
	{
	case MSG_SYSPFS_01:
		msg = 
		"\n%s: operation failed, user disk quota limit reached\n";
		break;

	case MSG_SYSPFS_02:
		msg = 
		"\n%s: operation failed, group disk quota limit reached\n";
		break;

	case MSG_SYSPFS_03:
		msg = 
		"\n%s: operation failed, user disk quota time limit reached\n";
		break;

	case MSG_SYSPFS_04:
		msg = 
		"\n%s: operation failed, group disk quota time limit reached\n";
		break;

	case MSG_SYSPFS_05:
		msg = 
		"\n%s: warning, user disk quota exceeded\n";
		break;

	case MSG_SYSPFS_06:
		msg = 
		"\n%s: warning, group disk quota exceeded\n";
		break;

	case MSG_SYSPFS_07:
		msg = 
		"\n%s: operation failed, user inode quota limit reached\n";
		break;

	case MSG_SYSPFS_08:
		msg = 
		"\n%s: operation failed, group inode quota limit reached\n";
		break;

	case MSG_SYSPFS_09:
		msg = 
		"\n%s: operation failed, user inode quota time limit reached\n";
		break;

	case MSG_SYSPFS_10:
		msg = 
		"\n%s: operation failed, group inode quota time limit reached\n";
		break;

	case MSG_SYSPFS_11:
		msg = 
		"\n%s: warning, user inode quota exceeded\n";
		break;

	case MSG_SYSPFS_12:
		msg = 
		"\n%s: warning, group inode quota exceeded\n";
		break;

	case MSG_SYSPFS_13:
		msg = 
		"\n%s: operation failed, user disk quota time limit exceeded\n";
		break;

	case MSG_SYSPFS_14:
		msg = 
		"\n%s: operation failed, group disk quota time limit exceeded\n";
		break;

	case MSG_SYSPFS_15:
		msg = 
		"\n%s: operation failed, user inode quota time limit exceeded\n";
		break;

	case MSG_SYSPFS_16:
		msg = 
		"\n%s: operation failed, group inode quota time limit exceeded\n";
		break;

	}
	len = strlen(msg);
	bcopy(msg,defmsg,len);
}


