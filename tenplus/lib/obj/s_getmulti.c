/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)99	1.6  src/tenplus/lib/obj/s_getmulti.c, tenplus, tenplus411, GOLD410 3/23/93 12:05:52";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_getmulti
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
/* s_getmulti:  extracts a multi-line field from a tree                     */
/* Takes a field path and returns a structured POINTER array.               */
/* Returns ERROR if there is no data for the field.  If field               */
/* corresponds to a POINTER array, returns the first line.                  */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

#include <libobj.h>

POINTER *s_getmulti (POINTER *tree, char *datapath)
    {
    register i, j;
    register char *cp1, *cp2;
    register POINTER *node;
    register POINTER *list;
    register POINTER *field;
    register POINTER *data;
    char prefix [PATH_MAX];

#ifdef TRACE
    debug ("s_getmulti:  tree = 0%o, datapath = '%s'\n", tree, datapath);
#endif

    if (tree == NULL)
	return ((POINTER *) ERROR);

#ifdef CAREFUL
    (void) s_typecheck ((char *)tree, "s_getmulti:  tree", T_ANY);
#endif

    if (obj_type (tree) == T_CHAR) /* character array case */
	if (*datapath == '\0')     /* check for empty path */
	    return (s_pointer (s_string ((char *)tree)));
	else
	    return ((POINTER *) ERROR);

    if (obj_type (tree) != T_POINTER)
	fatal ("s_getmulti:  tree not a POINTER array");

    /***** break up path into its parts *****/


    for (cp1 = datapath, cp2 = prefix; *cp1; cp1++, cp2++)
	if (*cp1 == '*')
	    {
	    cp1++;
	    break;
	    }
	else
	    *cp2 = *cp1;

    *cp2 = '\0';

    /***** Note:  cp1 points at the suffix (the part after the asterisk) */

    node = (POINTER *) s_findnode (tree, prefix); /* go to index point */


    if (node == (POINTER *) ERROR)
	return ((POINTER *) ERROR);

    if (obj_type (node) == T_CHAR)
	{
	if (*cp1 == '\0') /* if no suffix */
	    return (s_pointer (s_string ((char *)node)));
	else
	    return ((POINTER *) ERROR);
	}
#ifdef CAREFUL
    (void) s_typecheck ((char *)node, "s_getmulti:  node", T_POINTER);
#endif

    /***** we now have a POINTER node and must do the suffix processing *****/

    list = (POINTER *) s_alloc (obj_count (node), T_POINTER, NULL);

    for (i = 0; i < obj_count (node); i++)
	{
	if (node [i])
	    {
	    /***** Note:  findnode w/ empty path returns the tree *****/

	    field = (POINTER *) s_findnode ((POINTER *)node [i], cp1); /* find suffix */

	    if (field != (POINTER *) ERROR)
		{
		if (obj_type (field) == T_CHAR)
		    {
		    list [i] = s_string ((char *)field);
		    continue;
		    }
#ifdef CAREFUL
		(void) s_typecheck ((char *)field, "s_getmulti:  field", T_POINTER);
#endif
		if (field [0] && (obj_type (field [0]) == T_CHAR))
		    {
		    list [i] = s_alloc(obj_count (field), T_POINTER, NULL);
		    data = list[i];
                    for (j = 0; j < obj_count (field); j++)
			{
		        if (obj_type (field[j]) == T_CHAR)
		            data [j] = s_string ((char *)field[j]);
			else
			    data [j] = s_alloc (0, T_CHAR, NULL);
                        }
		    continue;
		    }
		}
	    }
	/***** if no field or bad data shape, put in empty line *****/

	list [i] = s_alloc (0, T_CHAR, NULL);
	}
    return (list);
    }
