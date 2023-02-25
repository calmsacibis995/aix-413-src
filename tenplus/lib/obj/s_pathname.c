/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)08	1.5  src/tenplus/lib/obj/s_pathname.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:02";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_pathname
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
 
/****************************************************************************/
/* s_pathname:  returns path name of node in tree                           */
/* Returns allocated character array, or ERROR if node is not in tree       */
/****************************************************************************/

#include <stdio.h>
#include <chardefs.h>
#include "database.h"

#include <string.h>
#include <libobj.h>

char *s_pathname (POINTER *tree, char *node)
    /* tree:                       tree pointer                   */
    /* node:                       node to find                   */
    {
    register POINTER *path;     /* pointer array of nodes in tree */
    register i;                 /* index into pointer array       */
    register char *cp;          /* pointer into the name buffer   */
    register char *childname;   /* name of a node in the path     */
    char pathname [PATH_MAX];   /* path name                      */

#ifdef TRACE

    debug ("s_pathname:  tree = 0%o, node = 0%o\n", tree, node);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)tree, "s_pathname (tree)", T_ANY);
    (void) s_typecheck (node, "s_pathname (node)", T_ANY);

#endif

    path = s_path (tree, node);

    if (path == (POINTER *) ERROR) /* if node not in tree */
	{
	g_errno = E_MISC;
	return ((char *) ERROR);
	}

    cp = pathname;

    for (i = 1; i < obj_count (path); i++) /* for all nodes in path after root */
	{
	*cp++ = DIR_SEPARATOR; /* prefix all node names with slash */

	childname = obj_name (path [i]);

	if (childname != NULL) /* if node has name, append it to pathname */
	    (void) strcpy (cp, childname);
	else /* if no name, append index to pathname */
	    s_indexname (cp, (POINTER *)path [i - 1], (char *)path [i]);

	cp += strlen (cp); /* move pointer to end of string */
	}
    s_freenode ((char *)path); /* free path array without recursion */

    if (cp == pathname) /* if only one node in path, call it DIR_SEPARATOR */
	*cp++ = DIR_SEPARATOR;

    *cp = '\0'; /* terminate path name */

    return (s_string (pathname));
    }

