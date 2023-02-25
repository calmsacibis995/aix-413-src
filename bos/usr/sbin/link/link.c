static char sccsid[] = "@(#)61	1.6.1.2  src/bos/usr/sbin/link/link.c, cmdfiles, bos411, 9428A410j 11/16/93 14:00:44";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: link, unlink
 *
 * ORIGINS: 3, 18, 27
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 * 
 * static char rcsid[] = "RCSfile: link.c,v Revision: 1.4 (OSF) Date: 90/10/07 22:16:45 "
 */

#if SEC_BASE
#include <sys/secdefines.h>
#include <sys/security.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "link_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_LINK,Num,Str)

/*
 * NAME: link file1 file2
 *       unlink file
 *
 * FUNCTION: links or unlinks files
 */

main(int argc, char *argv[]) 
{
    char *progname;

    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_LINK, NL_CAT_LOCALE);

#if SEC_BASE
    set_auth_parameters(argc, argv);
    initprivs();
#endif

    /* get last name in path */
    progname = strrchr(argv[0],'/'); 
    if (progname) progname++; else progname = argv[0];

#if SEC_BASE
    if (authorized_user("linkdir") && !forcepriv(SEC_LINKDIR)) {
        fprintf(stderr,
	    MSGSTR(INSUF_PRIV,"%s: insufficient privileges\n"), progname);	
        exit(1);
    }
#endif

    /* identify our name */
    if (!strcmp(progname, "unlink")) {
	if (argc != 2) {
	    fprintf(stderr,MSGSTR(UNLINK_USAGE, "Usage: unlink name\n"));
	    exit(1);
	}
#if SEC_BASE
	disablepriv(SEC_SUSPEND_AUDIT);
#endif
	if (unlink(argv[1]) < 0)
	    exit(2);
    } else {
	if (strcmp(progname, "link"))
	    fprintf(stderr, MSGSTR(OOPS,"link/unlink: I should be called link or unlink. defaulting to link\n"));
	if (argc != 3) {
	    fprintf(stderr,MSGSTR(LINK_USAGE, "Usage: link from to\n"));
	    exit(1);
	}
#if SEC_BASE
	disablepriv(SEC_SUSPEND_AUDIT);
#endif
	if (link(argv[1],argv[2]) < 0)
	    exit(2);
    }

    exit(0);
    /*NOTREACHED*/
}
