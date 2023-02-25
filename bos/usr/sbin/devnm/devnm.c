static char sccsid[] = "@(#)59  1.6.1.4  src/bos/usr/sbin/devnm/devnm.c, cmdfiles, bos411, 9428A410j 1/10/94 18:16:51";
/*
 * COMPONENT_NAME: (CMDFILES) commands that deal with files
 *
 * FUNCTIONS: devnm
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* 
 * devnm.c: print name of device associated with file(system).
 *
 * notes: assumes devices are in /dev.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vmount.h>
#include <dirent.h>
#include <locale.h>
#include <nl_types.h>
#include "devnm_msg.h"

#define MSGSTR(Num,Str) catgets(catd,MS_DEVNM,Num,Str)

main(argc, argv)
int	argc;
char	*argv[];
{
    nl_catd catd;
    struct  stat sbuf;
    struct  dirent *dbuf;
    DIR	*dv;
    dev_t   fno;
    int     errflag = 0;

    (void) setlocale (LC_ALL,"");
    catd = catopen((char *)MF_DEVNM, NL_CAT_LOCALE);


    if ((chdir("/dev") == -1) || ((dv = opendir(".")) == NULL )) {
	fprintf(stderr, MSGSTR(CANTOPEN,"Cannot open /dev\n"));
	exit(1);
    }
    while(--argc) {
	seekdir(dv,(long)0);
	if (stat(*++argv, &sbuf) == -1) {
	    fprintf(stderr,"devnm: ");
	    perror(*argv);
	    exit(1);
	}
	if (sbuf.st_vfstype != MNT_JFS) {
	    fprintf(stderr, MSGSTR(NOTLOCAL,
			"devnm: %s is not in a local file system.\n"), *argv);
	    errflag++;
	    continue;
	}
	fno = sbuf.st_dev;
	while((dbuf = readdir(dv)) != NULL ) {
	    if (stat(dbuf->d_name, &sbuf) == -1) {
		fprintf(stderr, MSGSTR(STATERR,"/dev stat error on file %s\n"),dbuf->d_name);
		exit(1);
	    }
	    if ((fno != sbuf.st_rdev) || !S_ISBLK(sbuf.st_mode))
		continue;
	    printf("%s %s\n", dbuf->d_name, *argv);
	}
    }
    exit (errflag ? 1 : 0);
}
