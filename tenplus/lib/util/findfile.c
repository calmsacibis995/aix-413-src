#if !defined(lint)
static char sccsid[] = "@(#)83	1.5  src/tenplus/lib/util/findfile.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:18";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	findfile
 * 
 * ORIGINS:  9, 10, 27
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
#include <sys/types.h>
#include <database.h>
#include <sys/stat.h>

#include <libobj.h>

/****************************************************************************/
/* findfile:  find file with given name in given path list                  */
/* returns the full path to the indicated file (structured allocation) if   */
/* such exists, otherwise it returns (char *)NULL                           */
/****************************************************************************/

char *findfile (register char *prefix, register char *extension, register POINTER *pathlist)
 /* filename prefix    */
 /* filename extension */
 /* search directories */
{
    register int i;         /* pathlist index                */
    register int len;       /* number of objects in pathlist */
    struct stat buffer;     /* stat call buffer              */

#ifdef DEBUG
    debug ("findfile:  prefix = '%s', extension = '%s'", prefix, extension);
#endif

#ifdef CAREFUL
    (void) s_typecheck ((char *) pathlist, "findfile", T_POINTER);
#endif

    len = obj_count (pathlist);
    for (i = 0; i < len; i++) {
        char pathname [PATH_MAX];

	(void) sprintf (pathname, "%s/%s%s", pathlist [i], prefix, extension);
	if (stat (pathname, &buffer) != ERROR)
	    return (s_string (pathname));
	}

    return ((char *)NULL); /* not found */
}
