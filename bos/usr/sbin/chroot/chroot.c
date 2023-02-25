static char sccsid[] = "@(#)57	1.6.1.4  src/bos/usr/sbin/chroot/chroot.c, cmdfiles, bos411, 9428A410j 2/23/94 16:01:15";
/*
 * COMPONENT_NAME: (CMDFILES) commands that deal with the file system
 *
 * FUNCTIONS: chroot
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <locale.h>
#include <nl_types.h>
#include "chroot_msg.h"

#define MSGSTR(Num,Str) catgets(catd,MS_CHROOT,Num,Str)

main(argc, argv)
int argc;
char **argv;
{
	nl_catd catd;

	(void) setlocale (LC_ALL,"");
	catd = catopen((char *)MF_CHROOT, NL_CAT_LOCALE);
	if(argc < 3) {
		fprintf(stderr,MSGSTR(USAGE,"usage: chroot rootdir command\n"));
		exit(1);
	}
	argv[argc] = 0;
	if(argv[argc-1] == (char *) -1) /* don't ask why */
		argv[argc-1] = (char *) -2;
	if (chroot(argv[1]) < 0) {
		perror(argv[1]);
		exit(1);
	}
	if (chdir("/") < 0) {
		fprintf(stderr,MSGSTR(CANTCHROOT,"Can't chdir to new root\n"));
		exit(1);
	}
	execv(argv[2], &argv[2]);
	perror(argv[2]);
	exit(1);
}
