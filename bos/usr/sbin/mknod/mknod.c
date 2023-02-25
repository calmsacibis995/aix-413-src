static char sccsid [] = "@(#)05	1.8.1.2  src/bos/usr/sbin/mknod/mknod.c, cmdfiles, bos411, 9428A410j 11/16/93 08:30:40";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: mknod
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <grp.h>
#include <sys/limits.h>			/* need NGROUPS_MAX for array defn */

#include <ctype.h>
#include <locale.h>

#include <nl_types.h>
#include "mknod_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_MKNOD,Num,Str)

/*
 * NAME:     mknod Name {b | c | p}  Major Minor
 * 
 * FUNCTION: creates a special file
 *
 * NOTES:    b  -  indicates that the special file is a block oriented 
 *                 device (disk, diskette, or tape)
 *           c  -  indicates that the special file is a character oriented
 *                 device (other devices than block)
 *           p  -  creates FIFO's (named pipelines)
 */

main(argc, argv)
int argc;
char **argv;
{
    register  int  m, a, b, i;
    gid_t cgl[NGROUPS_MAX+2]; 		/* define concurrent group array */
    int ngr;
    int in_sysgrp = 0;

    (void) setlocale (LC_ALL,"");
    catd = catopen((char *)MF_MKNOD, NL_CAT_LOCALE);

    /* Added code to allow mknod to run suid for group system, and still */ 
    /* work normally for non-root and non-group-system users		 */   
    if(getuid())			/* if real uid != 0(root) then   */
	if (getgid()) {			/* if real gid != 0(system) then */

	   /* get user's concurrent group list 				 */
	   if (-1 != (ngr = getgroups(NGROUPS_MAX, cgl))) {
		/* check if one of the groups is the system group (0)    */
		for (i = 0; i < ngr; i++) 
		    if(!cgl[i]) {	/* group 0 is a mem. of cgl      */
			in_sysgrp++;	
			break;
		    }
	   }
	   /* if getgroups fails, the following code will set euid=ruid  */ 

	   if (!in_sysgrp)			/* normal user		 */
	   	if (seteuid(getuid())) { 	/* set euid=ruid         */
		    fileerror(argv[1], "setreuid"); /* print error & exit*/
		} 
	}

    if(argc == 3 && !strcmp(argv[2], "p")) { /* fifo */
	a = mknod(argv[1], S_IFIFO|0666, 0);
	chown(argv[1], getuid(), getgid());
	if (a) {
	    fileerror(argv[1], "mknod"); /* print error message and exit */
	}
	exit(0);
    }

    /* Look at the effective uid instead of the real uid here. We have 	*/
    /* already done the right checking above.			  	*/
    if(geteuid()) {
	fprintf(stderr,MSGSTR(USRUSAGE,"Usage: mknod  Name  p\n"));
	exit(2);
    }
    if(argc != 5) 
	usage();
    if(*argv[2] == 'b') 
	m = S_IFBLK|0666;
    else if(*argv[2] == 'c')
	m = S_IFCHR|0666;
    else
	usage();
    a = number(argv[3]);
    if(a < 0) usage();
    b = number(argv[4]);
    if(b < 0) usage();
    if(mknod(argv[1], m, makedev(a,b)) < 0) {
	fileerror(argv[1], "mknod");  /* print error message and exit */
    }
    exit(0);
}

/*
 * NAME:  number
 *
 * FUNCTION: convert the Major and Minor arguments to long integers.
 */

number(s)
register  char  *s;
{
    char *str;
    register  int  n;

    n = strtol(s,&str,0);

    if (*str != 0) return(-1);

    return(n);
}

/*
 * NAME:  usage
 *
 * FUNCTION: prints usage message
 */

usage()
{
    fprintf(stderr,MSGSTR(SYSUSAGE,"Usage: mknod Name {b|c} Major Minor\n\
       mknod Name p\n"));
    exit(2);
}

fileerror(filename, reason)
char *filename;
char *reason;
{
    char buf[MAXPATHLEN+10];
    sprintf(buf, "%s: %s", reason, filename);
    perror(buf);
    exit(1);
}
