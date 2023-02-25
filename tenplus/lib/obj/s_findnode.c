#if !defined(lint)
static char sccsid[] = "@(#)93	1.5  src/tenplus/lib/obj/s_findnode.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:30";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_findnode
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
/* s_findnode:  finds a node given a path name.                             */
/* Returns ERROR if node cannot be found                                    */
/* Does reverse search if G_REVERSE flag is set                             */
/* Handles NULL tree or tree with NULL's in it.                             */
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <chardefs.h>
#include "database.h"
#include <libobj.h>

char *s_findnode (register POINTER *tree, register char *pathname)
        /* tree to search                   */
       /* pathname to find                 */
    {
    register char *cp;         /* used to get childname            */
    register i;                /* index into tree pointer array    */
    register start;            /* start value for loop index       */
    register end;              /* end value for loop index         */
    register delta;            /* increment for loop index         */
    char childname [NAMESIZE]; /* next node name in pathname       */

#ifdef TRACE

    debug ("s_findnode:  called w/ tree = 0%o, pathname = '%s'\n",
	   tree, pathname);

#endif

    if (tree == NULL) /* watch out for NULL pointers */
	return ((char *) ERROR);

#ifdef CAREFUL

    (void) s_typecheck ((char *)tree, "s_findnode", T_ANY);

#endif

    if (*pathname == DIR_SEPARATOR) /* convert absolute path names to relative */
	pathname++;

    if (*pathname == '\0') /* if pathname is NULL, this is the node */
	return ((char *) tree);

    g_errno = E_MISC; /* all errors in this routine are miscellaneous */

    if (obj_type (tree) != T_POINTER)
	return ((char *) ERROR);

    /* move chars from pathname to childname, looking for slash or NULL */

    cp = childname;

    while (*pathname != '\0')
	{
	if (*pathname == DIR_SEPARATOR)
	    {
	    pathname++; /* skip over slash */
	    break;
	    }
	*cp++ = *pathname++;
	}
    *cp = '\0'; /* terminate child name */

    /* at this point, the child name is childname and the rest of */
    /* the pathname is in pathname.                               */

    if ((*childname >= '0') && (*childname <= '9')) /* if name is index */
	{
	i = atoi (childname); /* get index */

	if (i >= obj_count (tree)) /* make sure its valid */
	    return ((char *) ERROR);

	if (*pathname) /* if there is more path after the child name */
	    return(s_findnode ((POINTER *)tree [i], pathname));  /*search child*/ 
	else
	    return ((char *) tree [i]); /* if last node return child */
	}
    if (g_getflag (G_REVERSE)) /* if doing reverse search */
	{
	start = obj_count (tree) - 1;
	end = -1;
	delta = -1;
	}
    else
	{
	start = 0;
	end = obj_count (tree);
	delta = 1;
	}
    for (i = start; i != end; i += delta) /* search tree for childname */
	if (tree [i] != NULL)
	    if (strcmp (childname, obj_name (tree [i])) == 0)
		{
		if (*pathname) /* if there is more path after the child name */
		    return (s_findnode ((POINTER *)tree [i], pathname)); /* search child */
		else
		    return ((char *) tree [i]); /* if last node return child */
		}
    return ((char *) ERROR); /* if no child matched the child name, return ERROR */
    }

