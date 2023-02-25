/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)73	1.6  src/tenplus/lib/util/Llock.c, tenplus, tenplus411, GOLD410 3/23/93 12:08:34";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Llock, Lunlock, Llockname
 * 
 * ORIGINS:  9, 10, 27
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
 
/****************************************************************************/
/* file:  Llock.c - routines for file locking                               */
/****************************************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <chardefs.h>
#include <database.h>

#define SYS_START "/etc/.sys_start"

static char *Llockname (char *);


/****************************************************************************/
/* Llock:  routine to lock a file                                           */
/*                                                                          */
/* Uses create mode 0 style of lock.  Return RET_OK, or ERROR if cannot make*/
/* lock                                                                     */
/****************************************************************************/

int Llock (char *filename)
    /* filename:                file to lock           */
    {
    register char *lockname; /* name of file to create */
    register fd;             /* file desciptor         */
    static long sys_start;   /* time system was brought up */
    struct stat statbuf;

    lockname = Llockname (filename);
    fd = open (lockname, O_WRONLY | O_CREAT | O_EXCL, 0);

    if (fd >= 0)
	{
	(void)close (fd);
	return (RET_OK);
	}

    if (sys_start == 0L)
	{
	if (stat (SYS_START, &statbuf) == ERROR)
	    sys_start = ERROR;
	else
	    sys_start = statbuf.st_ctime;
	}

    if (sys_start == ERROR)
	return (ERROR);

    /* If the lock file was generated before the system startup time, try to
       remove it; if removal is successful retry the locking. */

    if ((stat (lockname, &statbuf) != ERROR) &&
      (statbuf.st_ctime < sys_start) && (unlink (lockname) != ERROR))
	return (Llock (filename));

    return (ERROR);
    }

/****************************************************************************/
/* Lunlock:  removes lock created with Llock                                */
/*                                                                          */
/* Returns ERROR if lock cannot be removed (unlink fails)                   */
/****************************************************************************/

int Lunlock (char *filename)
    /* filename:           file to lock           */
    {
    return (unlink (Llockname (filename)));
    }

/****************************************************************************/
/* Llockname:  returns name of lock file                                    */
/****************************************************************************/

static char *Llockname (char *filename)
    {
    register char *start; /* start of file name (w/o directory) */
    register char *end;   /* end of file name                   */
    static char lockname [PATH_MAX];
    int name_max;	/* maximum length of terminal file name, on */
			/* file system containing this file. */

    name_max = (int) pathconf (filename, _PC_NAME_MAX);

    (void) strcpy (lockname, filename);
    end = lockname + strlen (lockname);
    start = strrchr (lockname, DIR_SEPARATOR);

    if (start == NULL)
	start = lockname;
    else
	start++;

    
    if ((end - start) > (name_max - 3))
	end = start + name_max - 3;

    (void) strcpy (end, ".lk");
    return (lockname);
    }
