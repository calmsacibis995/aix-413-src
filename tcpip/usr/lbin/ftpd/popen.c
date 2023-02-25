static char sccsid[] = "@(#)44        1.6  src/tcpip/usr/lbin/ftpd/popen.c, tcpfilexfr, tcpip41J, 9524G_all 6/9/95 10:02:36";
/* 
 * COMPONENT_NAME: TCPIP popen.c
 * 
 * FUNCTIONS: ftpd_pclose, ftpd_popen 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software written by Ken Arnold and
 * published in UNIX Review, Vol. 6, No. 8.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * static char sccsid[] = "popen.c	5.7 (Berkeley) 9/1/88";
 */
/*
#ifndef lint
static char sccsid[] = "popen.c	5.2 (Berkeley) 9/22/88";
#endif  not lint */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <stdio.h>
#include <glob.h>

extern  char    tempstr[];
extern  char    tempuser[];
extern  int     gethdir();
extern	char 	*home;		/* pointer to home directory for glob */

/*
 * Special version of popen which avoids call to shell.  This insures noone
 * may create a pipe to a hidden program as a side effect of a list or dir
 * command.
 */
static uid_t *pids;
static int fds;

FILE *
ftpd_popen(program, type)
	char *program, *type;
{
    register char *cp;
    FILE *iop;
    int argc, gargc, pdes[2], pid;
    char *argv[100], *vv[2];	
    char **newgargv;
    char **tmpgargv;
    extern char *strtok();
    glob_t globbuf;
    int	   globbed = 0;
    int    i, j;

    
    if (*type != 'r' && *type != 'w' || type[1])
	return(NULL);
    
    if (!pids) {
	if ((fds = getdtablesize()) <= 0)
	    return(NULL);
	if (!(pids =
	      (uid_t *)malloc((u_int)(fds * sizeof(uid_t)))))
	    return(NULL);
	bzero(pids, fds * sizeof(uid_t));
    }
    if (pipe(pdes) < 0)
	return(NULL);

    argc = 0;

    for(argv[argc] = strtok(program, " \t\n");
	argv[argc];
	argc++, argv[argc] = strtok(NULL, " \t\n"));

    argc--;				/* get argc back to real number of args */
    
    /* break up string into pieces */
    if (strpbrk(argv[argc], "*?[{")) {
	globbed = 1;
	/* glob the last piece */
	/*
	 * First we need to do tilde expansion since glob()
	 * in libc.a is to dumb to do it for us.
	 */

	cp = argv[argc];
	if (*cp == '~') {       /* we found a tilde */
	    cp++;
	    if (*cp == '\0' || *cp == '/') {
		/*
		 * we found a null,
		 * or / after the tilde.
		 */
		strcpy(tempstr, home);
		strcat(tempstr, cp);
	    } else {
		/* we might of found a user after the tilde */
		for (i = 0; i < MAXPATHLEN &&
		     *cp != '/' && *cp != '\0'; i++)
		    tempuser[i] = *cp++;
		tempuser[i] = '\0';
		if (gethdir(tempuser))
		    return(NULL);
		else {
		    strcpy(tempstr, tempuser);/* user dir */
		    strcat(tempstr, cp); /* rest of path */
		}
	    }
	} else {
	    strcpy(tempstr, cp);
	}
	if (strpbrk(tempstr, "*?[{")) {
	    int	glob_num;
	    /* we have glob characters in tempstr */
	    if (glob_num = glob(tempstr, GLOB_QUOTE|GLOB_NOCHECK,
				NULL, &globbuf) != 0) {
		/* globbing failed */
		return(NULL);
	    }
	    if (globbuf.gl_pathc == 0) {
		/* globbing failed */
		return(NULL);
	    }
	    
	    tmpgargv = (char **) malloc((unsigned) sizeof(tmpgargv)
					* globbuf.gl_pathc);
	    
	    for (j = 0; j < globbuf.gl_pathc && 
		 globbuf.gl_pathv[j] != NULL; j++) {
		if ((tmpgargv[j] = (char *) malloc((unsigned)
						   strlen(globbuf.gl_pathv[j])+1)) != NULL)
		    strcpy(tmpgargv[j], globbuf.gl_pathv[j]);

	    }
	    tmpgargv[j] = NULL;
	} else {
	    tmpgargv = (char**)malloc(sizeof(char*) * 2);
	    tmpgargv[0] = (char*) malloc(strlen(tempstr));
	    strcpy(tmpgargv[0], tempstr);
	    tmpgargv[1] = NULL;
	}
    }
    if (globbed) {
	newgargv = (char**) malloc(sizeof(argv[0]) +
				   sizeof(argv[1]) +
				   (sizeof(newgargv) * (globbuf.gl_pathc+1)));
	newgargv[0] = argv[0];
	newgargv[1] = argv[1];
	for (i = argc, j = 0; j < globbuf.gl_pathc; i++, j++) {
	    newgargv[i] = tmpgargv[j];
	}
	newgargv[i] = NULL;

    }
    else {
	newgargv = argv;
    }
	
    iop = NULL;
    switch(pid = vfork()) {
    case -1:			/* error */
	(void)close(pdes[0]);
	(void)close(pdes[1]);
	goto free;
	/* NOTREACHED */
    case 0:				/* child */
	if (*type == 'r') {
	    if (pdes[1] != 1) {
		dup2(pdes[1], 1);
		(void)close(pdes[1]);
	    }
	    (void)close(pdes[0]);
	} else {
	    if (pdes[0] != 0) {
		dup2(pdes[0], 0);
		(void)close(pdes[0]);
	    }
	    (void)close(pdes[1]);
	}
	execv(newgargv[0], newgargv);
	_exit(1);
    }
    /* parent; assume fdopen can't fail...  */

    if (*type == 'r') {
	iop = fdopen(pdes[0], type);
	(void)close(pdes[1]);
    } else {
	iop = fdopen(pdes[1], type);
	(void)close(pdes[0]);
    }
    pids[fileno(iop)] = pid;
    
 free:	
    return(iop);
}

ftpd_pclose(iop)
	FILE *iop;
{
	register int fdes;
	long omask;
	int pid, stat_loc;
	u_int waitpid();

	/*
	 * pclose returns -1 if stream is not associated with a
	 * `popened' command, or, if already `pclosed'.
	 */
	if (pids[fdes = fileno(iop)] == 0)
		return(-1);
	(void)fclose(iop);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = wait(&stat_loc)) != pids[fdes] && pid != -1);
	(void)sigsetmask(omask);
	pids[fdes] = 0;
	return(stat_loc);
}
