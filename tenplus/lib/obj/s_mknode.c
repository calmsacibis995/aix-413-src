/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)04	1.5  src/tenplus/lib/obj/s_mknode.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:54";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_mknode
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
 
/****************************************************************************/
/* s_mknode:  makes a node in a tree given its path                         */
/*  handles NULL path and tree values                                       */
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <chardefs.h>
#include "database.h"
#include <libobj.h>
#include <libutil.h>

char *s_mknode (POINTER *tree, char *path, int leaftype)
    /*tree:               the tree, possibly NULL      */
    /*path:               the path to create           */
    /*leaftype:           the leaf type                */
    {
    char nodename [NAMESIZE];   /* next node name in path       */
    char *np;                   /* used to get nodename         */
    register int indexflag;     /* is node an index?            */
    register int i;             /* index to tree pointer array  */
    register char *node;        /* used for character array case */

#ifdef DEBUG
    debug ("s_mknode on entry:  tree = 0%o, path = '%s', leaftype = %d",
	      tree, path, leaftype);
    if (tree)
	s_print (tree);
#endif

    if (path == NULL) /* treat NULL path as the empty string */
	path = "";

    while (*path == DIR_SEPARATOR) /* skip over leading slashes */
	path++;

    if (*path == '\0')   /* if path is empty, all done */
	return ((char *) tree);

    if (tree == NULL)    /* create tree if it doesn't exist */
	tree = (POINTER *) s_alloc (0, T_POINTER, NULL);

    /* move chars from path to nodename, looking for slash or NULL  */
    /* determine if name is an index                                */

    np = nodename;
    indexflag = TRUE;

    while (*path != '\0')
	{
	if (*path == DIR_SEPARATOR)
	    {
	    path++; /* skip over slash */
	    break;
	    }
        if (!isdigit(*path)) 
	    indexflag = FALSE;
	mbcpy(&np, &path);
	}
    *np = '\0'; /* terminate node name */

    /* at this point, the node name is in nodename and the rest   */
    /* of the path name is in path.                               */

#ifdef DEBUG
    debug ("nodename = '%s', path = '%s', indexflag = %d",
	nodename, path, indexflag);
#endif

    /**** if node is an index;                                *****/
    /*****  create a new node if one doesn't exist            *****/

    if (indexflag)
	{
	i = atoi (nodename);            /* get index value */
	if (i >= (obj_count (tree)))    /* create new node */
	    tree = (POINTER *) s_realloc ((char *)tree, (unsigned)i + 1);
	if (*path == '\0')              /* reached a leaf node */
	    {
	    tree [i] = s_alloc (0, leaftype, NULL);
	    return ((char *) tree);
	    }
	}
    else /* node is not an index */
	{
	for (i = 0; i < obj_count (tree); i++)  /* search tree for nodename */
	    if (tree [i] != NULL)
		if (strcmp(nodename, obj_name (tree [i])) == 0)
		    break;

	if (i >= (obj_count(tree)))     /* node was not found   */
	    {
	    tree = (POINTER *) s_realloc ((char *)tree, (unsigned)i + 1);
	    if (*path == '\0')          /* reached a leaf node */
		{
		tree [i] = s_alloc (0, leaftype, nodename);
		return ((char *) tree);
		}
	    tree [i] = s_alloc (0, T_POINTER, nodename);
	    }
	}

    /***** handle character array case *****/

    if ((tree [i] != NULL) && (obj_type (tree [i]) == T_CHAR))
	{
	if (*tree [i] == '\0')
	    node = s_alloc (0, T_POINTER, obj_name (tree [i]));
	else
	    {
	    node = (char *) s_pointer (s_string (tree [i]));
	    s_newname (node, obj_name (tree [i]));
	    }
	(void) s_fixtree (tree, tree [i], node, 0);
	/* s_fixtree (tree, tree [i], node); Missing arg in call*/
	}

    /***** recurse on rest of path *****/

    tree [i] = s_mknode ((POINTER *)tree [i], path, leaftype);
    return ((char *) tree);
    }
