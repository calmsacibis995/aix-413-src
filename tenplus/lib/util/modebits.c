/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)78	1.7  src/tenplus/lib/util/modebits.c, tenplus, tenplus411, GOLD410 3/23/93 12:09:12";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	modebits, ingroup
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/stat.h>
#include <unistd.h>
#include <database.h>
#include <libutil.h>

/***** stat mode bits (modebits return values) *****/

#define READ  (4)
#define WRITE (2)
#define EXEC  (1)

/****************************************************************************/
/* modebits:  return mode bits for user from stat buffer                    */
/*                                                                          */
/* arguments:              struct stat *statptr - stat call buffer          */
/*                                                                          */
/* return value:           int - the mode bits from the stat buffer         */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     modebits returns the read, write, and execute permissions for        */
/*     the file being stat'ed.  If the user is running as super-user,       */
/*     return all permissions.  Otherwise, if the user owns the file,       */
/*     return the user permissions subfield; if the user is in the          */
/*     same group as the maker of the file, return the group permissions    */
/*     subfield; and otherwise return the world permissions subfield.       */
/****************************************************************************/

/* This version is from the AIX 2.0 version of INED.  It contains DFS
	and multiple-group functionality, and was contributed by IBM. */

int modebits (register char *filename, register struct stat *statptr)
    /*filename;        file name        */
    /*statptr;  stat call buffer */
    {
    register modes;     /* file modes               */
    static uid = ERROR;

#ifdef DEBUG
    debug ("modebits:  modes = 0%o", statptr->st_mode);
#endif

    modes = statptr->st_mode & 0777; /* get rwx bits for user, group & world */

    if (uid == ERROR)     /* get user id if necessary */
	uid = geteuid ();

    if (uid == 0) /* if super-user */
	if ((statptr->st_dev & 0xffff0000) == 0) /* if local */
	     return (READ | WRITE | EXEC);
	else
	     if (access(filename, WRITE) == 0)
		  return (READ | WRITE | EXEC);

    if (uid == statptr->st_uid) /* if file owned by current user */
	return (modes >> 6);

    if (ingroup (statptr->st_gid)) /* if file owned by user in same group */
	return ((modes >> 3) & 7);

    return (modes & 7); /* use world protections */
    }

#include <grp.h>
#define	TWODOZEN 24

int ingroup(int file_gid)
    {
    int gids[TWODOZEN];
    int i;

    for (i = getgroups(TWODOZEN, (gid_t *)gids), --i; i >= 0; --i)
	if (file_gid == gids[i])
		return(TRUE);
    return(FALSE);
    }
