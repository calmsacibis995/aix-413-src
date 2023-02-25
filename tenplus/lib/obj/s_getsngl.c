/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)00	1.5  src/tenplus/lib/obj/s_getsngl.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:43";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_getsingle
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
/* s_getsingle:  extracts a single line text field from a tree              */
/* Takes a field path and returns a structured character array.             */
/* Returns ERROR if there is no data for the field.  If field               */
/* corresponds to a POINTER array, returns the first line.                  */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

#include <libobj.h>

char *s_getsingle (POINTER *tree, char *datapath)
    {
    register char *cp1, *cp2;
    register POINTER *node;
    char newpath [PATH_MAX];

#ifdef TRACE
    debug ("s_getsingle:  tree = 0%o, datapath = '%s'\n",
	   tree, datapath);
#endif

    if (tree == NULL)
	return ((char *) ERROR);

#ifdef CAREFUL
    (void) s_typecheck ((char *)tree, "s_getsingle:  tree", T_ANY);
#endif

    if (obj_type (tree) == T_CHAR) /* character array case */
	if (*datapath == '\0')     /* check for empty path */
	    return (s_string ((char *)tree));
	else
	    return ((char *) ERROR);

#ifdef CAREFUL
    (void) s_typecheck ((char *)tree, "s_getsingle:  tree", T_POINTER);
#endif

    /***** make real path.  Convert asterisk in indexed path to '0' *****/


    for (cp1 = datapath, cp2 = newpath; *cp1; cp1++, cp2++)
	if (*cp1 == '*')
	    *cp2 = '0';
	else
	    *cp2 = *cp1;

    *cp2 = '\0';

    node = (POINTER *) s_findnode (tree, newpath);


    if (node == (POINTER *) ERROR)
	return ((char *) ERROR);

    if (obj_type (node) == T_CHAR)
	return (s_string ((char *)node));

    if (node [0] && (obj_type (node [0]) == T_CHAR))
	return (s_string (node [0]));

    return ((char *) ERROR);
    }
