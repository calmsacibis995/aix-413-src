#if !defined(lint)
static char sccsid[] = "@(#)20	1.7  src/tenplus/sf_progs/tree.c, tenplus, tenplus411, GOLD410 3/23/93 12:11:23";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	ancget, ancset, char_tree, do_ancestors, midnode,
 *		ptr_1child, ptr_children, ptr_tree, strcopy, tree
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
 
/****************************************************************************/
/* tree.c:  routines to print out structured file in tree form              */
/*                                                                          */
/*   usage:  tree file [file...]                                            */
/*                                                                          */
/* Modified from readfile                                                   */
/****************************************************************************/

#include <stdio.h>
#include <chardefs.h>
#include <database.h>
#include <stdlib.h>
#include <limits.h>
#include "readfile_msg.h"
#include "tree.h"

extern nl_catd g_catd;

#define FIRSTSON  (1)
#define LASTSON   (2)
#define ONLYSON   (3)
#define MIDDLESON (4)

#define MAXDEPTH (30)              /* maximum depth of recursion */
#define MAXWIDTH (18 * MB_LEN_MAX) /* maximum string length      */

				/* classes of tree node children */
extern FILE* outfp;             /* output file pointer          */

void fatal(char *, ...);

static int ancget (int , char *);
static int ancset (int, char *);
static int char_tree (POINTER *, int , int );
static void do_ancestors (int );
static int midnode (int , int );
static void ptr_1child (POINTER *, int , char *);
static void ptr_children (POINTER *,  int , char *, int , int );
static void ptr_tree (POINTER * , int , int , int );
static void strcopy (char *, char *);

/****************************************************************************/
/* Each ancestor is the name to be printed out for that ancestor - either a */
/* actual name, or a padding of the appropriate number of blanks.           */
/*                                                                          */
/* The variable "top" greatest holds the greatest stack top value seen so f */
/****************************************************************************/

static char   real_ancestors [MAXDEPTH] [MAXWIDTH];

static int    greatest = 0;    /* one above highest level seen so far */

/****************************************************************************/
/* ancget: get the ancestor at the given level                              */
/****************************************************************************/

int ancget (int level, char *ancestor)
    /* level:               at which to get ancestor     */
    /* ancestor:            to be returned               */
    {

#ifdef DEBUG
    debug ("ancget:  level = %d, greatest = %d", level, greatest);
#endif

    *ancestor = '\0';

    if ((level < 0) || (level > greatest))
	{
	(void) fprintf (outfp, MSGSTR(M_BAD_STACK,"Internal Error (ancget stack index %d not valid).\n"), level);
	return(ERROR);
	}

    strcopy (ancestor, real_ancestors[level]);
    return(RET_OK);
    }

/****************************************************************************/
/* ancset:  set the ancestor at the given level                             */
/****************************************************************************/

static int ancset (int level, char *ancestor)
    /* level:               at which to set ancestor     */
    /* ancestor:            to be set                    */
    {

#ifdef DEBUG
    debug ("ancset:  level = %d, ancestor = '%s', greatest = %d",
	     level, ancestor, greatest);
#endif

    if ((level < 0) || (level > (greatest + 1)) || (level >= MAXDEPTH))
	{
	(void) fprintf (outfp, MSGSTR(M_BAD_INDEX,"Internal Error (ancset index %d not valid).\n"), level);
	return(ERROR);
	}

    /* Preserve the value of greatest if this is the highest we have been */
    if (level > greatest)
	greatest = level;

    /* Write the new ancestor name into the ancestor stack */
    strcopy (real_ancestors[level], ancestor);
    return(RET_OK);
    }


/****************************************************************************/
/* char_tree:  print one character object in tree fashion                   */
/****************************************************************************/

static int char_tree (POINTER *object, int top, int sontyp)
    /* object:             to be prettyprinted           */
    /* top:                current top of ancestor stack */
    /* sontyp:             sibling class                 */
    {
	char         *name;             /* object name, if any           */

#ifdef DEBUG
    debug ("char_tree:  top = %d", top);
#endif

    do_ancestors (top);

    name = obj_name (object);

    if (name == NULL || *name == '\0')
	if (midnode (top, sontyp))
	    (void) fprintf (outfp, "|-->");
	else
	    (void) fprintf (outfp, "--->");
    else
	if (midnode (top, sontyp))
	    (void) fprintf (outfp, "|-->%-10.10s-->", name);
	else
	    (void) fprintf (outfp, "--->%-10.10s-->", name);

    (void) fprintf (outfp, "\"%s\"\n", object);
    }

/****************************************************************************/
/* do_ancestors:  print the ancestors at a given level                      */
/****************************************************************************/

static void do_ancestors (int top)
    /* top:                                     current top of ancestor stack */
    {
    int     index;                           /* looper through ancestors      */
    char    anc[MAXWIDTH];                   /* one ancestor                  */
    int     bufptr;                          /* pointer into buffer           */
    int     offset;                          /* pointer offset into buffer    */
    int     seenspace;                       /* whether we have seen spacename*/
    int     bufwidth;                        /* width of one ancestor cell    */

    char    buffer [MAXWIDTH * MAXDEPTH];    /* output line                   */
    char   *barname = "|                ";   /* constant name                 */
    char   *spacename = "                 "; /* constant name                 */

#ifdef DEBUG
    debug ("do_ancestors:  top = %d", top);
#endif

/****************************************************************************/
/* Read the ancestors into the buffer in reverse order.  If we encounter    */
/* any space name, any ancestors with literal names should also use         */
/* space names.  This is necessary to prevent a node with one child but     */
/* many grandchildren from appearing on each of its grandchildren's lines.  */
/****************************************************************************/

    bufwidth = MAXWIDTH - 1;
    buffer [top * bufwidth] = '\0';     /* ensure termination      */
    seenspace = FALSE;                  /* seen no space names yet */

    for (index = --top; index >= 0; index--)
	{
        int anc_1_is_space; /* TRUE if anc[1] equals space */

	offset = index * bufwidth;

	if (ancget (index, anc) != RET_OK)
	    fatal (MSGSTR(M_ANCESTOR,"Internal Error (Cannot get ancestor %d).\n"), index);

        /* Determine whether the second char in anc is white space */
	/* We look at anc[1] rather than anc[0] because anc[0] may be '|' */

        if (MultibyteCodeSet) {
             wchar_t wc;
             (void) mbtowc(&wc, anc + mblen(anc, MB_CUR_MAX), MB_CUR_MAX);
             anc_1_is_space = iswspace(wc);
        }
        else anc_1_is_space = isspace(anc[1]);

	if (seenspace)
	    if (!anc_1_is_space)    /* substitution required */
		if (anc[0] == '|')
		    strcopy (anc, barname);
		else
		    strcopy (anc, spacename);

	if (anc_1_is_space)
	    seenspace = TRUE;
	else                        /* write a spacename for future */
	    ancset (index, spacename);

	for (bufptr = 0; bufptr < bufwidth; bufptr++)

	    buffer [bufptr + offset] = anc [bufptr];
	}

    /* Finally we can print it */

    (void) fprintf (outfp, "%s", buffer);
    }


/****************************************************************************/
/* midnode:  determines whether node is of edge type or middle type         */
/****************************************************************************/

static int midnode (int top, int sontyp)
    /* top:         current top of ancestor stack */
    /* sontyp:      classification of son node    */
    {
    if (top == 0)
	return (FALSE);         /* all top-level nodes are edge type */

    if ((sontyp == MIDDLESON) || (sontyp == ONLYSON))
	return (TRUE);
    else
	return (FALSE);
    }

/****************************************************************************/
/* ptr_1child:  print a pointer object with only one child                  */
/****************************************************************************/

static void ptr_1child (POINTER *object, int top, char *nodename)
    /* object:            to be prettyprinted           */
    /* top:               current top of ancestor stack */
    /* nodename:          node name                     */
    {

#ifdef DEBUG
    debug ("ptr_1child:  top = %d, nodename = '%s'", top, nodename);
#endif

/* With only one child, we can just push this nodename onto the stack and
   recurse onto the child.  */

    if (ancset (top++, nodename) == RET_OK)
	tree ((POINTER *) object[0], top, 0, ONLYSON);

    }

/****************************************************************************/
/* ptr_children:  print a pointer node with more than one child             */
/****************************************************************************/

static void ptr_children (POINTER *object,  int top, char *nodename, 
                   int count, int sontyp)
    /* object:            to be prettyprinted           */
    /* top:               current top of ancestor stack */
    /* nodename:          node name                     */
    /* count:             number of children of node    */
    /* sontyp:            sibling class                 */
    {
    int              i;                 /* looper through children       */

    char  *myname;

    char  *midname = "|                ";
    char  *spcname = "                 ";

#ifdef DEBUG
    debug ("ptr_children:  top = %d, nodename = '%s', count = %d",
	     top, nodename, count);
#endif

    switch (sontyp) {
	case  FIRSTSON:
	case  ONLYSON:
	    myname = spcname;
	    break;
	case  MIDDLESON:
	case  LASTSON:
	    myname = midname;
	    break;
	default:
	    (void) fprintf (stderr, MSGSTR(M_SONTYP,"Internal Error (sontyp %d not expected).\n"), sontyp);
	    return;
	    /*NOTREACHED*/
	    break;
	}

    /* Do the first child with the name */
    if (ancset (top++, myname) == RET_OK)
	tree ((POINTER *) object[0], top, 0, FIRSTSON);
    else
	return;

    /* Put out the name of this node followed by end of line */
    do_ancestors (top - 1);
    (void) fprintf (outfp, "%s|\n", nodename);

    switch (sontyp) {
	case  FIRSTSON:
	case  MIDDLESON:
	    myname = midname;
	    break;
	case  ONLYSON:
	case  LASTSON:
	    myname = spcname;
	    break;
	default:
	    (void) fprintf (stderr, MSGSTR(M_SONTYP,"Internal Error (sontyp %d not expected).\n"), sontyp);
	    return;
	    /*NOTREACHED*/
	    break;
	}

    /* Do the first child with the name */
    /* Do the intermediate children with the midname, with one extra line
       between each and the next. */

    if (ancset (top - 1, myname) == RET_OK)
	for (i = 1; i + 1 < count; i++)
	    {
	    tree ((POINTER *) object[i], top, i, MIDDLESON);
	    do_ancestors(top);
	    (void) fprintf (outfp, "|\n");
	    }
    else
	return;

    /* Do the last child with the name */
    i = count - 1;
    tree ((POINTER *) object[i], top, i, LASTSON);
    }

/****************************************************************************/
/* ptr_tree:  print one pointer object in tree fashion                      */
/****************************************************************************/

static void ptr_tree (POINTER * object, int top, int objnum, int sontyp)
    /* object:            to be prettyprinted           */
    /* top:               current top of ancestor stack */
    /* objnum:            object number (default name)  */
    /* sontyp:            sibling class of this tree    */
    {
    int              count;             /* how many children of object   */
    char             *name;             /* object name, if any           */
    char             nodename[MAXWIDTH];         /* node name            */

#ifdef DEBUG
    debug ("ptr_tree:  top = %d, objnum = %d", top, objnum);
#endif

    name = obj_name (object);

#ifdef DEBUG
    debug ("ptr_tree:  obj_name = '%s'", name);
#endif

    if (midnode (top, sontyp))
	if (name == NULL || *name == '\0')
	    (void) sprintf (nodename, "|-->%10d-->", objnum);
	else
	    (void) sprintf (nodename, "|-->%-10.10s-->", name);
    else
	if (name == NULL || *name == '\0')
	    (void) sprintf (nodename, "--->%10d-->", objnum);
	else
	    (void) sprintf (nodename, "--->%-10.10s-->", name);

#ifdef DEBUG
    debug ("ptr_tree:  nodename = '%s'", nodename);
#endif

    count = obj_count (object);

    if (count < 1)
	{
	do_ancestors (top);
	(void) fprintf (outfp, "%s", nodename);
	if (midnode (top, sontyp))
	    (void) fprintf (outfp, "|-->");
	else
	    (void) fprintf (outfp, "--->");
	(void) fprintf (outfp, MSGSTR(M_NO_DESC,"Internal Error (no descendants).\n"));
	}

    else if (count == 1)
	ptr_1child (object, top, nodename);

    else
	ptr_children (object, top, nodename, count, sontyp);
    }

/****************************************************************************/
/* strcopy: string copier cribbed from C manual  (with length check added)  */
/****************************************************************************/

static void strcopy (char *s, char *t)
    {
    int   i = 0;

    while ((*s++ = *t++) && (i++ < MAXWIDTH))
	;
    }

/****************************************************************************/
/* tree:  print one object in tree fashion                                  */
/****************************************************************************/

void tree (POINTER *object, int top, int objnum, int sontyp)
    /* object:            to be prettyprinted           */
    /* top:               current top of ancestor stack */
    /* objnum:            object number (default name)  */
    /* sontyp:            first, last, only, or middle  */
    {

#ifdef DEBUG
    debug ("tree:  top = %d, objnum = %d", top, objnum);
#endif

    if (object == NULL)
	{
	do_ancestors (top);
	if (midnode (top, sontyp))
	    (void) fprintf (outfp, "|-->NULL\n");
	else
	    (void) fprintf (outfp, "--->NULL\n");
	return;
	}
    else if (object == (POINTER *) ERROR)
	{
	do_ancestors (top);
	if (midnode (top, sontyp))
	    (void) fprintf (outfp, "|-->");
	else
	    (void) fprintf (outfp, "--->");

	(void) fprintf (outfp, MSGSTR(M_ERR_MSG,"Error\n"));
	return;
	}

    /* The object is healthy enough to examine its type */

    switch (obj_type (object))   {

	case T_CHAR:

	    char_tree (object, top, sontyp);
	    break;

	case T_POINTER:

	    ptr_tree (object, top, objnum, sontyp);
	    break;

	default:

	    do_ancestors (top);
	    if (midnode (top, sontyp))
		(void) fprintf (outfp, "|-->");
	    else
		(void) fprintf (outfp, "--->");
	    (void) fprintf(outfp, MSGSTR(M_UNEXP_TYPE,"Internal Error (object of unexpected type %d).\n"), obj_type (object));
	    break;
	}
    }
