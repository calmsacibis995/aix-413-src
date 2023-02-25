/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)70	1.5  src/tenplus/lib/util/abspath.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:03";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	abspath
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <chardefs.h>
#include <database.h>
#include <libobj.h>



/****************************************************************************/
/* abspath:  converts relative pathname to absolute                         */
/* Takes a path and a directory.  Returns an allocated object.              */
/*                                                                          */
/* arguments:              char *patharg   - relative or absolte path       */
/*                         char *directory - starting directory             */
/*                                                                          */
/* return value:           char * - allocated absolute path                 */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given path starts with slash (i.e. is itself an absolute      */
/*     path), start the result path at the given path; otherwise start      */
/*     with the given directory and append the path to it.  The only        */
/*     complicated special cases arise when the given path includes         */
/*     "." or ".." references, which must be appropriately unwound.         */
/*                                                                          */
/*     The returned absolute path is guaranteed not to contain ".", "..",   */
/*     or consecutive slashes (null paths).                                 */
/****************************************************************************/

char *abspath (char *patharg, char *directory)
    /* patharg:                    relative or absolute path */
    /* directory:                  starting directory        */
    {
    register char *path;        /* local copy of patharg     */
    register char *field;       /* field in path             */
    register char *cp;          /* search pointer            */
    register char *pathptr;     /* pointer to path           */
    char result [PATH_MAX];   /* total result              */

    path = s_string (patharg); /* make copy of path so we can change it */
    pathptr = path;

# ifdef DEBUG
    debug ("abspath:  path = '%s', directory = '%s'", path, directory);
# endif

    if (*directory != DIR_SEPARATOR)
        fatal ("abspath:  directory (%s) must start with slash", directory);

    if (*path == DIR_SEPARATOR)
        (void) strcpy (result, "/");
    else
        (void) strcpy (result, directory);

    while (path)
        {
        field = path;
        path = strchr (path, DIR_SEPARATOR);

        if (path)
	    *path++ = '\0'; /* replace slash with NULL to terminate */

	if ((*field == '\0') || (strcmp (field, ".") == 0)) /* empty field or dot */
            continue;

        if (! strcmp(field, "..") == 0) /* if not .. */
            {
	    if (! strcmp (result, "/") == 0) /* append slash if necessary */
                (void) strcat (result, "/");

            (void) strcat (result, field);
            continue;
            }
        cp = strrchr (result, DIR_SEPARATOR); /* try to trim field off result */

        if (cp == NULL)
            fatal ("abspath:  trim failure, result = '%s', path = '%s', dir = '%s'",
                   result, patharg, directory);

        if (cp == result) /* if last field in result */
	    result [1] = '\0'; /* leave the slash */
        else
	    *cp = '\0'; /* else trim off the last field */
        }
    s_free (pathptr);
    return (s_string (result));
    }
