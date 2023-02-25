#if !defined(lint)
static char sccsid[] = "@(#)06	1.6  src/tenplus/e2/common/nfl.c, tenplus, tenplus411, GOLD410 3/23/93 11:52:31";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	datalength, delline, digin, findrecord, fixtree,
 *		flushrecord, getrecord, idxnode, insline, makerecord,
 *		pfxnode, readline, sfxnode, writeline, emptyrecord
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
/* File: nfl.c                                                              */
/****************************************************************************/

#include "def.h"
#include "INeditor_msg.h"

extern char g_helpstate;

void getrecord (int, int);
char *makerecord (char *);
POINTER *sfxnode (POINTER *, int);

/****************************************************************************/
/* datalength:  returns length of g_wnode after checking it                 */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - length of g_wnode in lines                 */
/*                                                                          */
/* globals referenced:     g_wnode, g_sfp, g_recindex, g_recdirty           */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     datalength returns the length, in lines, of the current g_wnode.     */
/*     If g_wnode is currently invalid, invoke pfxnode to do the prefix.    */
/*     If g_wnode is NULL, return 0.                                        */
/*     If g_wnode is a character array, return 1.                           */
/*     If g_wnode is at the record level of the file being edited,          */
/*         and the g_recdirty flag is set to indicate the record cache      */
/*         is dirty, and the g_recindex global is off the end of the        */
/*         record array, return 1 plus g_recindex.                          */
/*     In all other cases, return the number of elements in g_wnode.        */
/****************************************************************************/

int datalength (void)
    {

    if (g_wnode == (char *) ERROR) /* if g_wnode invalid  */
	pfxnode (FALSE);

#ifdef DEBUG
    debug ("datalength: g_wnode = 0%o", g_wnode);
    objprint ("g_wnode",g_wnode);
#endif

    if (g_wnode == NULL)
	return (0);

    if (obj_type (g_wnode) == T_CHAR)
	return (1);

    /* if g_wnode is record array, check to see if current record is */
    /* dirty and off the end of the array.                           */

    if ((g_wnode == (char *) sf_records (g_sfp)) &&
	(g_recindex >= obj_count (g_wnode)) &&
	(g_recdirty))
	return (g_recindex + 1);

    return (obj_count (g_wnode));
    }

/****************************************************************************/
/* delline: delete count lines at and following line                        */
/*                                                                          */
/* arguments:              int line  - line at which to start deletion      */
/*                         int count - number of lines to delete            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wnode, g_sfp, g_record, g_snode, g_recdirty    */
/*                                                                          */
/* globals changed:        g_record, g_snode, g_wnode, g_recdirty           */
/*                                                                          */
/* synopsis:                                                                */
/*     delline deletes the given number of elements from the g_wnode array  */
/*     starting at the given line number.  If g_wnode is currently invalid, */
/*     invoke pfxnode to do the prefix.  Then its behavior depends on       */
/*     the nature of g_wnode as follows:                                    */
/*                                                                          */
/*     If g_wnode is null, return immediately.                              */
/*                                                                          */
/*     If it is at the record level of the file, flush the current record.  */
/*                                                                          */
/*     If it is a character array, either delete the line corresponding     */
/*     to g_wnode if the given starting number is 0, updating g_record,     */
/*     g_snode, g_wnode, and g_recdirty, or return with no effect if        */
/*     the starting number is not 0.                                        */
/*                                                                          */
/*     If it is a record array, use sf_delete to delete the records from    */
/*     the file, update g_wnode and g_snode, and return.                    */
/*                                                                          */
/*     Finally, if it is a pointer array, use s_free and s_delete to delete */
/*     the requested pointers, and update g_record, g_snode, and g_wnode.   */
/****************************************************************************/

void delline (int line, int count)
    {
    POINTER *pointers;
    int i;

#ifdef CAREFUL
    if (line < 0)
	fatal ("delline:  negative index (%d)", line);

    if (count <= 0)
	fatal ("delline:  count (%d) not positive", count);
#endif

    if (g_wnode == (char *) ERROR) /* if g_wnode invalid */
	pfxnode (FALSE);              /* set up if possible */

#ifdef DEBUG
    debug("delline: line=%d,count=%d,g_wnode=0%o,wf=%s",line,count,g_wnode,
       w_name (g_wp));
#endif

    if (g_wnode == NULL) /* if window has no ulhc in data */
	return;

    if (g_wnode == (char *) sf_records (g_sfp))
	flushrecord (TRUE);

    pointers = (POINTER *) g_wnode;
    if (obj_type (pointers) == T_CHAR)
	{
	if (line == 0)
	    {
	    g_record = fixtree (g_record, (char *) pointers, (char *) NULL);
	    s_free ((char *) pointers);

	    if (g_snode == g_wnode) /* reset SNODE if necessary */
		g_snode = NULL;

	    g_wnode = NULL;
	    g_recdirty = TRUE; /* mark cache as dirty */
	    }
	return;
	}

    /* if line off end of node */
    if (line >= obj_count (pointers))
	return;

    /* make sure count isn't too big */
    if (line + count > obj_count (pointers))
	count = obj_count (pointers) - line;

    if (obj_type (pointers) == T_RECORD)
	{
	sf_delete (g_sfp, line, count);
	g_wnode = (char *) sf_records (g_sfp);
	g_snode = g_wnode;
	return;
	}

    /* g_wnode is a pointer array inside the record */
    g_recdirty = TRUE; /* mark cache as dirty */

    for (i = 0; i < count; i++)
	s_free (pointers [line + i]);

    pointers = (POINTER *) s_delete ((char *)pointers, line, count);
    g_record = fixtree (g_record, g_wnode, (char *) pointers);

    if (g_snode == g_wnode) /* reset SNODE if necessary */
	g_snode = (char *) pointers;

    g_wnode = (char *) pointers;
    }

/****************************************************************************/
/* digin:  finds a node inside a record in the current file.                */
/* Sets up g_snode from g_spath.  If g_spath is NULL, g_snode               */
/* is set to the record array.                                              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_sfp, g_spath, g_snode, g_recindex, g_record    */
/*                                                                          */
/* globals changed:        g_snode                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     digin sets the g_snode global on the basis of the other globals.     */
/*     If g_sfp is null, bring down the editor with a call to fatal.        */
/*     If g_spath is null, set g_snode to point to the records array        */
/*     of the file.  Otherwise invoke findrecord to find the record path    */
/*     corresponding to g_spath.  If g_recindex is set to ERROR,            */
/*     set g_snode to null.  Otherwise use s_findnode to set g_snode to     */
/*     the node inside g_record corresponding to the path or, if the        */
/*     node is not found, set g_snode to NULL.  Thus, after digin,          */
/*     g_snode either points to the record array of the file, or to         */
/*     some node inside the file, or to null.                               */
/****************************************************************************/

void digin (void)
    {
    char *path; /* path inside record */

#ifdef DEBUG
    debug("digin: sfp=0%o,path='%s'",g_sfp,g_spath);
#endif

    if (g_sfp == NULL)
	fatal ("digin: NULL g_sfp");

    if ((g_spath == NULL) || (*g_spath == '\0'))
	{
	g_snode = (char *) sf_records (g_sfp);
	return;
	}

    /* try to find record corresponding to path */
    path = findrecord (g_spath);

    if (g_recindex == ERROR) /* if no such record yet */
	{
	g_snode = NULL;
	return;
	}

    g_snode = s_findnode ((POINTER *)g_record, path);
    if (g_snode == (char *) ERROR) /* if path inside record doesn't exist */
	g_snode = NULL;
    }

/****************************************************************************/
/* findrecord:  find sub-level path to record from prefix                   */
/*                                                                          */
/* arguments:              char *path - path into record array              */
/*                                                                          */
/* return value:           char * - portion of path below record level      */
/*                                                                          */
/* globals referenced:     g_recindex, g_record, g_sfp                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     findrecord uses getrecord to make g_record be the record named       */
/*     by the first part of the given path, and then returns the            */
/*     remainder of the path - the portion below record level.              */
/*     If the given path begins with a number, use that record number.      */
/*     As a heuristic, check to see if the existing current record          */
/*     has the right name and if so, use it.  Otherwise search linearly     */
/*     through the records of the file, looking for one with the right      */
/*     name.  If there is no record with the right name, flush the          */
/*     current record.  In any case, return the sub-record-level portion    */
/*     of the input path specification.                                     */
/****************************************************************************/

char *findrecord (char *path)
    {
    char *namep;             /* name buffer pointer    */
    int index1;              /* record index           */
    int count;               /* number of records      */
    char namebuf [NAMESIZE]; /* record name            */

#ifdef DEBUG
    debug ("findrecord:  path = '%s'", path);
#endif

#ifdef CAREFUL
    if (path == NULL)
	fatal ("findrecord:  path NULL");
#endif

    /* first we have to find the "right" record */
    namep = namebuf;
    while ((*path != DIR_SEPARATOR) && (*path != '\0'))
	*namep++ = *path++;

    *namep = '\0';

    /* skip over slash if there is more to the path */
    if (*path == DIR_SEPARATOR)
	path++;

    /* now namebuf has pathname up to first slash (record name) and */
    /* path has the rest                                            */

    if (isdigit(*namebuf))
	{
	index1 = atoi (namebuf);
	getrecord (index1, FALSE);
	return (path);
	}

    /* first check current record */
    if (g_recindex != -1)
	{
	if ((g_record) && (seq (obj_name (g_record), namebuf)))
	    return (path);
	}

    /* it's not the current record, so check all of them */
    count = obj_count (sf_records (g_sfp));

    /* current record may be off end of the array */
    if (g_recindex >= count)
	count = g_recindex + 1;

    for (index1 = 0; index1 < count; index1++)
	{
	getrecord (index1, FALSE);
	if ((g_record) && (seq (obj_name (g_record), namebuf)))
	    return (path);
	}

    flushrecord (TRUE); /* if no record of specified name */
    return (path);
    }

/****************************************************************************/
/* fixtree:  smart interface to s_fixtree                                   */
/*                                                                          */
/* arguments:              char *tree - pointer to tree to be fixed         */
/*                         char *oldnode - node to find references to       */
/*                         char *newnode - node to replace oldnode with     */
/*                                                                          */
/* return value:           char * - the fixed version of the tree           */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     fixtree invokes s_fixtree to fix the given tree, checking first      */
/*     for a few special cases.  If the tree itself is the old node,        */
/*     return the new node.  If oldnode and newnode are the same,           */
/*     return the original tree.  A null oldnode causes a fatal error.      */
/*     Otherwise s_fixtree does the fixing.                                 */
/****************************************************************************/

char *fixtree (char *tree, char *oldnode, char *newnode)
    {
#ifdef DEBUG
    debug ("fixtree:  g_record=0%o,tree=0%o,oldnode=0%o,newnode=0%o",
	g_record, tree, oldnode, newnode);

    objprint ("tree", tree);
    objprint ("oldnode", oldnode);
    objprint ("newnode", newnode);
#endif

    if (tree == oldnode)
	return (newnode);

    if (oldnode == newnode)
	return (tree);

    if (oldnode == NULL)
	fatal ("fixtree:  oldnode is NULL");

    (void) s_fixtree ((POINTER *)tree, oldnode, newnode, TRUE);
    return (tree);
    }

/****************************************************************************/
/* flushrecord:  flushes current record cache                               */
/*                                                                          */
/* arguments:              int flag - TRUE if record should be freed        */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_recindex, g_record, g_recdirty, g_sfp,         */
/*                         g_diskfull, g_filename, g_snode, g_wnode         */
/*                                                                          */
/* globals changed:        g_diskfull, g_snode, g_wnode, g_recdirty,        */
/*                         g_record, g_recindex                             */
/*                                                                          */
/* synopsis:                                                                */
/*     flushrecord flushes the current record to its file and optionally    */
/*     frees it.  If g_recindex is ERROR, do nothing.  If the record        */
/*     has been modified, write it to the file, updating g_snode and        */
/*     g_wnode if necessary.  If the write fails, set the g_diskfull        */
/*     global to TRUE so that fatal doesn't try to close the file and       */
/*     then issue a fatal error.                                            */
/*                                                                          */
/*     Finally, set the g_recdirty global false to show the cache is        */
/*     up to date and, if the flag argument is set, clear g_record and      */
/*     set g_recindex to ERROR.                                             */
/****************************************************************************/

void flushrecord (int flag)
    {
    RECORD *oldrecords; /* records array pointer */
    SFILE *sfp;         /* g_sfp                 */

#ifdef DEBUG
    debug ("flushrecord:  g_recindex = %d, g_record = 0%o, dirty = %s",
	g_recindex, g_record, g_recdirty ? "YES" : "NO");
    debug (" sf_records (sfp) = 0%o, g_snode = 0%o", sf_records (g_sfp), g_snode);
#endif

    if (g_recindex == ERROR) /* if no record in cache */
	return;

    sfp = g_sfp;

    if (g_recdirty) /* if record has been modified */
	{
	oldrecords = sf_records (sfp);

	/* reorder fields in record if we have a field list */
	if (g_fieldlist)
	    {
	    char *oldrecord;

	    oldrecord = g_record;
	    g_record = (char *) s_reorder ((POINTER *)g_record, g_fieldlist);

	    /***** fix up g_snode and g_wnode, which may have pointed */
	    /***** into the old version of g_record                   */

	    if (g_snode == oldrecord)
		g_snode = g_record;

	    if (g_wnode == oldrecord)
		g_wnode = g_record;
	    }

	if (sf_write (sfp, g_recindex, g_record) == ERROR)
	    {
	    g_diskfull = TRUE; /* don't try to close file */

	    Emessage (M_ESAVE, "Cannot write to file %s\n(disk probably full).", g_filename);

	    (void) sleep (5);
	    fatal ("error writing record on file '%s' (disk probably full)",
		   g_filename);
	    }

	if (g_snode == (char *) oldrecords) /* update g_snode if necessary */
	    g_snode = (char *) sf_records (sfp);

	if (g_wnode == (char *) oldrecords) /* update g_wnode if necessary */
	    g_wnode = (char *) sf_records (sfp);
	}

    g_recdirty = FALSE;
    if (flag)
	{
	if (g_record == g_wnode)
	    g_wnode = (char *) ERROR;
	s_free (g_record);
	g_record = NULL;
	g_recindex = -1;
	}
    }

/****************************************************************************/
/* getrecord:  gets a record into the record cache                          */
/* Allows index to be off the end of the records array.  The cache is set   */
/* to NULL in this case.                                                    */
/*                                                                          */
/* arguments:              int index - record number to get                 */
/*                         int flag - true if doing a write operation       */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_recindex, g_record, g_fakefile, g_sfp,         */
/*                         g_textfp, g_recdirty, g_errno, g_fieldlist       */
/*                                                                          */
/* globals changed:        g_recindex, g_record, g_recdirty                 */
/*                                                                          */
/* synopsis:                                                                */
/*     If the index is negative, issue a fatal error.  If the given         */
/*     index is already in the cache, simply return.  Otherwise flush       */
/*     the current record and get a new one, using either readfake or       */
/*     sf_read depending on whether the current file is fake.  If the       */
/*     read failed because the record was null, set g_record to NULL        */
/*     and return.  If it failed for any other reason, issue a fatal        */
/*     error.  If it did not fail, g_record is set to the desired record.   */
/*                                                                          */
/*     If flag is false, g_fieldlist is not NULL and the current editing    */
/*     level is the top of the file, sf_getfields is used instead of        */
/*     sf_read to read in the record using partial record reading.          */
/****************************************************************************/

void getrecord (int index2, int flag)
    {
    SFILE *sfp;      /* local g_sfp */
    static partial; /* true if partial record in core */

#ifdef CAREFUL
    if (index2 < 0)
	fatal ("getrecord:  negative index (%d)", index2);
#endif

#ifdef DEBUG
    debug ("getrecord:  index = %d, g_recindex = %d", index2, g_recindex);

    debug ("getrecord:  index = %d, g_recindex = %d, partial = %s",
	   index2, g_recindex, partial ? "TRUE" : "FALSE");
    debug ("g_spath = '%s'", g_spath);
#endif

    sfp = g_sfp;

    /* if reading current record and its complete or reading at
       file level, return */

    if ((index2 == g_recindex) &&
	((! partial) ||
	 ((! flag) && ((g_spath == NULL) || (*g_spath == '\0')))))
	return;

    partial = FALSE;
    flushrecord (TRUE);
    g_recindex = index2;

    if (g_fakefile) /* if ascii file */
    {
	g_record = readfake (sfp, g_textfp, index2);
    }
    else
	{ /* see if partial record reading is appropriate */
	if ((flag == FALSE) && g_fieldlist &&
	    ((g_spath == NULL) || (*g_spath == '\0')))
	    {
	    g_record = (char *) sf_getfields (sfp, index2, g_fieldlist);

	    /* if partial failed, try full read */
	    if ((g_record == (char *) ERROR) && (g_errno == E_MISC))
		g_record = sf_read (sfp, index2);
	    else
		partial = TRUE;
	    }
	else
	    g_record = sf_read (sfp, index2);
	}


    g_recdirty = FALSE;
    if (g_record == (char *) ERROR)
	{
	if (g_errno == E_MISC)
	    g_record = NULL; /* non-existent index */
	else
	    fatal ("getrecord:  I/O failure in sf_read");
	}
    else
        if (g_helpstate == INHELPFILE)
        {
            /* N.B. Help Text message catalogues only have Set = 1 messages */
            g_record = (char *)catscan((POINTER *)g_record,g_helpfilecatd, 1);
        }
    }

/****************************************************************************/
/* idxnode:  gets one element of g_wnode from its index                     */
/*                                                                          */
/* arguments:              int index - g_wnode subrecord to get             */
/*                         int flag  - TRUE if node should be created       */
/*                                     if it does not already exist         */
/*                                                                          */
/* return value:           POINTER * - the index'th node of g_wnode,        */
/*                                     or NULL if not flag and didn't exist */
/*                                                                          */
/* globals referenced:     g_wnode, g_sfp, g_record, g_snode                */
/*                                                                          */
/* globals changed:        g_record, g_wnode, g_snode                       */
/*                                                                          */
/* synopsis:                                                                */
/*     idxnode gets the record from g_wnode with the given index,           */
/*     creating it if necessary if the flag argument is set TRUE.           */
/*                                                                          */
/*     If g_wnode is at the record level of the file, use getrecord         */
/*     to get the given record.  If the flag is set and the record          */
/*     does not exist, return a null allocation but do NOT set the          */
/*     g_recdirty global true, because the record cache is not dirty        */
/*     until it is really written into.                                     */
/*                                                                          */
/*     If g_wnode is a character array and the requested index is 0,        */
/*     return it.  (A character array is, in this context, the same         */
/*     as a one-element pointer array with the character array in           */
/*     element zero.)  If the index is not zero and the flag is set,        */
/*     re-allocate g_wnode and, if necessary, g_snode, so that the          */
/*     character array is really element zero and return a null             */
/*     allocation as above.                                                 */
/*                                                                          */
/*     Otherwise g_wnode is a pointer array.  If the requested element      */
/*     exists, return a pointer to it.  If it does not exist and the        */
/*     flag is set, re-allocate the array and, if necessary, g_snode        */
/*     and return a null allocation as above.                               */
/****************************************************************************/

POINTER *idxnode (int index3, int flag)
    {
    POINTER *wnode;    /* g_wnode                        */
    POINTER *node;     /* node corresponding to index    */

#ifdef DEBUG
    debug ("idxnode:  index3=%d,flag=%s", index3, flag ? "YES" : "NO");
#endif

    wnode = (POINTER *) g_wnode;

    if (wnode == (POINTER *) sf_records (g_sfp)) /* if ulhc is records array */
	{
	getrecord (index3, flag);

	if (g_record == NULL)
	    {
	    if (flag == FALSE)
		return (NULL);

	    g_record = s_alloc (0, T_POINTER, (char *) NULL);
	    }
	return ((POINTER *) g_record);
	}

    /* if g_wnode is inside a record */
    if (obj_type (wnode) == T_CHAR)
	{
	if (index3 == 0)
	    return (wnode); /* don't index through char array if index 0 */

	if (flag == FALSE) /* if not creating, no valid data node */
	    return (NULL);

	/* Convert char array to pointer array of right size.   */
	/* Move char array to first child, and make last child. */

	wnode = (POINTER *) s_alloc (index3 + 1, T_POINTER, obj_name (g_wnode));
	wnode [0] = g_wnode;
	s_newname (g_wnode, (char *) NULL);
	g_record = fixtree (g_record, g_wnode, (char *) wnode);

	if (g_snode == g_wnode) /* reset SNODE if necessary */
	    g_snode = (char *) wnode;

	g_wnode = (char *) wnode;
	wnode [index3] = s_alloc (0, T_POINTER, (char *) NULL);
	return ((POINTER *) wnode [index3]); /* return child at index */
	}

    /* g_wnode a pointer array */
    if (index3 >= obj_count (wnode)) /* if index too large */
	{
	if (flag == FALSE) /* if not creating, do nothing */
	    return (NULL);

	wnode = (POINTER *) s_realloc ((char *) wnode, index3 + 1);
	g_record = fixtree (g_record, g_wnode, (char *) wnode);

	if (g_snode == g_wnode) /* reset SNODE if necessary */
	    g_snode = (char *) wnode;

	g_wnode = (char *) wnode;
	}

    node = (POINTER *) wnode [index3]; /* descend to child */
    if (node == NULL) /* if child is NULL */
	{
	if (flag == FALSE)
	    return (NULL);

	/* make new child in create case */

	node = (POINTER *) s_alloc (0, T_POINTER, (char *) NULL);
	wnode [index3] = (char *) node;
	}
    return (node);
    }

/****************************************************************************/
/* insline: insert count lines before given line                            */
/*                                                                          */
/* arguments:              int line  - before which to do insertion         */
/*                         int count - how many lines to insert             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wnode, g_sfp, g_record, g_recdirty, g_snode    */
/*                                                                          */
/* globals changed:        g_record, g_snode, g_wnode, g_recdirty           */
/*                                                                          */
/* synopsis:                                                                */
/*     insline inserts the given number of elements from the g_wnode array  */
/*     starting at the given line number.  If g_wnode is currently invalid  */
/*     invoke pfxnode to do the prefix.  Then its behavior depends on       */
/*     the nature of g_wnode as follows:                                    */
/*                                                                          */
/*     If g_wnode is null, return immediately.                              */
/*                                                                          */
/*     If it is at the record level of the file, flush the current record.  */
/*                                                                          */
/*     If it is a character array, either re-allocate g_wnode and put the   */
/*     existing line after count blank lines, updating g_record,            */
/*     g_snode, g_wnode, and g_recdirty, or return with no effect if        */
/*     the starting line number is not 0.                                   */
/*                                                                          */
/*     If it is a record array, use sf_insert to insert the records into    */
/*     the file, update g_wnode and g_snode, and return.                    */
/*                                                                          */
/*     Finally, if it is a pointer array, use s_insert to insert the        */
/*     requested blank lines, and update g_record, g_snode, and g_wnode.    */
/****************************************************************************/

void insline (int line, int count)
    {
    POINTER *pointers;
    char *wnode;

    if (g_wnode == (char *) ERROR) /* if g_wnode invalid */
	pfxnode (FALSE);              /* set up if possible */

    wnode = g_wnode;

#ifdef DEBUG
    debug ("insline: line=%d, count=%d, wnode=0%o, type=%d",
	line,count,wnode, wnode != NULL ? obj_type (wnode) : ERROR);
    debug ("         sf_records (g_sfp) = 0%o", sf_records (g_sfp));
#endif

#ifdef CAREFUL
    if (line < 0)
	fatal ("insline:  negative line (%d)", line);

    if (count <= 0)
	fatal ("insline:  count (%d) not positive", count);
#endif

    if (wnode == NULL) /* if window has no ulhc in data */
	return;         /* let wline make node if necessary later */

    if ((datalength () + count) > OBJSIZE)
	{
	Emessage (M_E_LINES,
     "The file has too many lines.\nThe number of lines cannot exceed %d.",
	OBJSIZE);

	 (void) sleep (5);
	fatal ("insline: number of lines exceeded %d.", OBJSIZE);
	}

    if (wnode == (char *) sf_records (g_sfp))
	{
	flushrecord (TRUE);
	wnode = (char *) sf_records (g_sfp); /* in case it expanded */
	}
    if (obj_type (wnode) == T_CHAR)
	{
	if (line == 0)
	    {
	    pointers = (POINTER *) s_alloc(count + 1, T_POINTER, (char *) NULL);
	    pointers [count] = wnode;
	    s_newname ((char *) pointers, obj_name (wnode));
	    s_newname (wnode, (char *) NULL);
	    g_record = fixtree (g_record, wnode, (char *) pointers);

	    if (g_snode == wnode) /* reset SNODE if necessary */
		g_snode = (char *) pointers;

	    g_wnode = (char *) pointers;
	    g_recdirty = TRUE; /* mark cache as dirty */
	    }
	return;
	}
    pointers = (POINTER *) wnode;

    if (line >= obj_count (pointers)) /* if off end of array do nothing */
	return;                /* let wline add extra nodes if necessary */

    if (obj_type (pointers) == T_RECORD)
	{
	sf_insert (g_sfp, line, count);
	g_wnode = (char *) sf_records (g_sfp);
	g_snode = g_wnode;
	return;
	}
    pointers = (POINTER *) s_insert ((char *)pointers, line, count);
    g_record = fixtree (g_record, wnode, (char *) pointers);

    if (g_snode == wnode) /* reset SNODE if necessary */
	g_snode = (char *) pointers;

    g_wnode = (char *) pointers;
    g_recdirty = TRUE; /* mark cache as dirty */
    }

/****************************************************************************/
/* makerecord:  makes record corresponding to path                          */
/* Returns path below record level.  Makes full path into record.           */
/*                                                                          */
/* arguments:              char *path - full path name for record           */
/*                                                                          */
/* return value:           char * - portion of path below record level      */
/*                                                                          */
/* globals referenced:     g_recindex, g_record, g_recdirty                 */
/*                                                                          */
/* globals changed:        g_record, g_recdirty                             */
/*                                                                          */
/* synopsis:                                                                */
/*     makerecord uses findrecord to find the specified record if it        */
/*     exists and to determine what portion of the given path is below      */
/*     record level.  If the record is found and there is an internal       */
/*     path, use s_findnode to look for the given node.  If it is there,    */
/*     return it; otherwise use s_mknode to create it, set the g_recdirty   */
/*     global TRUE, and return the internal path.                           */
/*                                                                          */
/*     If the record is not found, construct it and append it to the        */
/*     records array of the file.  If there is an internal path, again      */
/*     use s_mknode to create the desired node.  Return the internal        */
/*     path.                                                                */
/****************************************************************************/

char *makerecord (char *path)
    {
    char *namep;              /* name buffer pointer     */
    char *retval;             /* return from findrecord  */
    char *node;               /* return from findnode    */
    char namebuf [NAMESIZE];  /* record name             */

#ifdef DEBUG
    debug ("makerecord:  path = '%s'", path);
#endif

#ifdef CAREFUL
    if (path == NULL)
	fatal ("makerecord:  path NULL");
#endif

    retval = findrecord (path); /* try to find record */

    if (g_recindex != ERROR) /* if record was found */
	{
	if (g_record == NULL)
	    g_record = s_alloc (0, T_POINTER, (char *) NULL);

	if (*retval == '\0') /* if no internal path */
	    return (retval);

	node = s_findnode ((POINTER *)g_record, retval);

	if ((node != (char *) ERROR) && (node != NULL))
	    return (retval);

	g_record = s_mknode ((POINTER *)g_record, retval, T_POINTER);
	g_recdirty = TRUE; /* mark cache as dirty */
	return (retval);
	}

    /* The record was not found, which means that it was not an index, */
    /* so get name from path and create new record of that name.       */

    namep = namebuf;

    while ((*path != DIR_SEPARATOR) && (*path != '\0'))
	*namep++ = *path++;

    *namep = '\0';

    getrecord (obj_count (sf_records (g_sfp)), TRUE); /* make new record  */
    g_record = s_alloc (0, T_POINTER, namebuf);   /* give it the name */

    if (*retval != '\0') /* if internal path, make it */
	{
	g_record = s_mknode ((POINTER *)g_record, retval, T_POINTER); 
	g_recdirty = TRUE; /* mark cache as dirty */
	}
    return (retval);
    }

/****************************************************************************/
/* pfxnode:  sets up g_wnode for current window                             */
/*                                                                          */
/* arguments:              flag - TRUE if node should be created            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wnode, g_snode, g_formname, g_wp,              */
/*                         g_recindex, g_spath, g_record, g_recdirty        */
/*                                                                          */
/* globals changed:        g_wnode, g_spath, g_record, g_recdirty           */
/*                                                                          */
/* synopsis:                                                                */
/*     Starts at g_snode and finds node corresponding to prefix in window   */
/*     In the read case (flag false) this routine is called when g_wnode    */
/*        is ERROR.                                                         */
/*     In the write case (flag true) it may be ERROR or NULL.               */
/*                                                                          */
/*     There are eight cases to handle:                                     */
/*                                                                          */
/*     if Flag is TRUE, the prefix should be created.                       */
/*     if g_snode is NULL, no data corresponds to g_spath                   */
/*     if g_wnode is NULL, no node corresponds to the window                */
/*                                                                          */
/*     Flag    g_snode     g_wnode     operation                            */
/*     ~~~~    ~~~~~~~     ~~~~~~~     ~~~~~~~~~                            */
/*     FALSE   NULL        NULL        nothing                              */
/*     FALSE   NULL        unknown     g_wnode = NULL                       */
/*     FALSE   known       NULL        nothing                              */
/*     FALSE   known       unknown     find prefix path in g_snode          */
/*     TRUE    NULL        NULL        create g_snode and g_wnode           */
/*     TRUE    NULL        unknown     create g_snode and g_wnode           */
/*     TRUE    known       NULL        create g_wnode                       */
/*     TRUE    known       unknown     find or create g_wnode               */
/*                                                                          */
/****************************************************************************/

void pfxnode (int flag)
    {
    char *snode;    /* g_snode     */
    char *wnode;    /* g_wnode     */
    char *path;     /* prefix path */
    char *newsnode; /* new snode   */
    char *newrecord;/* new record  */

    /* if window switch may => new file */
    if (obj_getflag (g_warray, W_ABSPATHS) || seq (g_formname, "*win*"))
	switchwin ();

    snode = g_snode;
    wnode = g_wnode;
    path = w_pfx (g_wp);

#ifdef DEBUG
    debug ("pfxnode:  flag = %s, g_snode = 0%o, g_wnode = 0%o, w_pfx = '%s'",
	flag ? "YES" : "NO", snode, wnode, path);
    debug (" sf_records (g_sfp) = 0%o", sf_records (g_sfp));
#endif

    if (flag == FALSE) /* handle read cases first */
	{
	if (snode == NULL) /* if no ulhc for form in data */
	    {
	    g_wnode = NULL; /* no ulhc for window either */
	    return;
	    }
	if (wnode == (char *) ERROR) /* if ulhc of window unknown */
	    {
	    if (path == NULL) /* if no prefix */
		{
		g_wnode = g_snode;
		return;
		}
	    if (snode == (char *) sf_records (g_sfp)) /* if record array */
		{
		path = findrecord (path);

		if (g_recindex == -1) /* if findrecord failed */
		    {
		    g_wnode = NULL;
		    return;
		    }
		snode = g_record;
		}
	    g_wnode = s_findnode ((POINTER *)snode, path);

	    if (g_wnode == (char *) ERROR) /* if search failed */
		g_wnode = NULL;
	    }
	return;
	}
    /* now the write cases.  If g_snode is null, create it and g_wnode */

    if (snode == NULL)
	if (g_spath == NULL) /* if at the record level */
	    g_snode = (char *) sf_records (g_sfp);
	else /* if path is inside a record */
	    {
	    /* Note:  makerecord returns path inside record */
	    newrecord = makerecord (g_spath);
	    snode = s_findnode ((POINTER *)g_record, newrecord);

#ifdef CAREFUL
	    if (snode == (char *) ERROR)
		fatal ("pfxnode:  s_findnode failure after makerecord");
#endif

	    if (path == NULL) /* if no prefix path, set g_wnode to g_snode */
		{
		g_snode = snode;
		g_wnode = g_snode;
		return;
		}
	    newsnode = s_mknode ((POINTER *)snode, path, T_POINTER);
	    g_record = fixtree (g_record, snode, newsnode);
	    g_wnode = s_findnode ((POINTER *)newsnode, path);
	    g_snode = newsnode;
	    g_recdirty = TRUE; /* mark cache as dirty */
	    return;
	    }

    /* g_snode is known, find or create g_wnode */
    if (path == NULL)
	{
	g_wnode = g_snode;
	return;
	}
    if (snode == (char *) sf_records (g_sfp)) /* if snode is record array */
	{
	path = makerecord (path); /* this returns internal path in record */
	g_wnode = s_findnode ((POINTER *)g_record, path);
	return;
	}
    /* g_spath is not NULL (i.e:  g_snode is below the record level */
    /* so we find or create g_wnode in the current record           */

    g_wnode = s_findnode ((POINTER *)snode, path); /* first try to find it */

    if ((g_wnode != (char *) ERROR) && (g_wnode != NULL))
	return;

    newsnode = s_mknode ((POINTER *)snode, path, T_POINTER);
    g_record = fixtree (g_record, snode, newsnode);  /* fix record cache */
    g_wnode = s_findnode ((POINTER *)newsnode, path);
    g_snode = newsnode;
    g_recdirty = TRUE; /* mark cache as dirty */
    }

/****************************************************************************/
/* readline: return text for line in current field                          */
/*                                                                          */
/* arguments:              int line  - line number to read                  */
/*                         int width - number of screen columns to fill     */
/*                                                                          */
/* return value:           ATTR * - the line contents, or NULL              */
/*                                                                          */
/* globals referenced:     g_e2catd, g_wnode, g_wp                          */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the g_wnode is currently in error, invoke pfxnode to do the pre-  */
/*     fix. If it is then null, return null.  Otherwise invoke idxnode to   */
/*     do index processing and, if it fails, return null.                   */
/*                                                                          */
/*     After index processing, readline considers the data path suffix.     */
/*     If the current field is not indexed, unpack and return the requested */
/*     line if it is a character array, or else the dummy line that says    */
/*     [ STRUCTURED OBJECT ].  If the field is indexed and also has the     */
/*     NAMEFIELD bit set, unpack and return either the name of the field or */
/*     the dummy line [NO NAME].                                            */
/*                                                                          */
/*     The normal case is a nonindexed, unnamed field.  Invoke sfxnode      */
/*     to do suffix processing and, if it fails, return null.  If after     */
/*     suffix processing the node is a pointer array, return its zero'th    */
/*     element; otherwise return the node itself.  In either case,          */
/*     unpack and return the node contents only if it is a character array, */
/*     and the dummy line [STRUCTURED OBJECT] otherwise.                    */
/****************************************************************************/

ATTR *readline (int line, int width)
    {
    WSTRUCT *wp=g_wp;               /* current window              */
    POINTER *node;                  /* node corresponding to index */
    POINTER *pointers;              /* POINTER version of g_wnode  */
    extern nl_catd g_e2catd;

    static char *noname, *stobject; /* save str from Egetmessage */

#ifdef DEBUG
    debug ("readline: line %d, width %d", line, width);
    debug (" g_wp = 0%o, g_wnode = 0%o, wf = %s", g_wp, g_wnode, w_name (g_wp));
    debug (" g_sfp = 0%o, g_snode = 0%o", g_sfp, g_snode);
#endif

    if (getstat (w_flags (wp), CATENTRY))
	{
	int msgno;
	char *mess;

	    msgno = w_msgno(wp);
	    mess = w_deftxt(wp);

            mess = catgets(g_e2catd,MS_FORMS,msgno,mess);

	    return (unpackup (mess, width));
	}

    /* if g_wnode invalid - set up if possible */
    if (g_wnode == (char *) ERROR)
	pfxnode (FALSE); 

    /* if window has no ulhc in data */
    if (g_wnode == NULL)
	return (unpackup ("", width));

    /* do index processing */
    node = idxnode (line, FALSE); 

    /* if no data for index */
    if (node == NULL)
	return (unpackup ("", width));

    /* we now are ready to process the suffix */
    if (! getstat (w_flags (wp), INDEXED))
	{
	if (obj_type (node) == T_CHAR)
	    return(unpackup((char *)node, width));

	if (stobject == NULL)
	    stobject = Egetmessage(M_ESTOBJECT,"[ STRUCTURED OBJECT ]",FALSE);

	return (unpackup (stobject, width));
	}

    if (getstat (w_flags (wp), NAMEFIELD))
	{
	if (obj_name (node) == NULL)
	    {
	    if (noname == NULL)
		noname = Egetmessage (M_ENONAME,"[NO NAME]",FALSE);

	    return (unpackup (noname, width));
	    }
	else
	    return (unpackup (obj_name(node), width));
	}


    /* do suffix processing */
    node = sfxnode (node, FALSE);

    /* if suffix processing failed */
    if (node == NULL)
	return (unpackup ("", width));

    if (obj_type (node) == T_POINTER)
	{

        /* empty pointer array */
	if (obj_count (node) == 0) 
	    return (unpackup ("", width));

	pointers = node;
	node = (POINTER *) pointers[0];

	if (node == NULL)
	    return (unpackup ("", width));
	}

    if (obj_type (node) == T_CHAR)
	return (unpackup ((char *) node, width));

    return (unpackup ("[STRUCTURED OBJECT]", width));
    }

/****************************************************************************/
/* sfxnode:  finds suffix of node for current window                        */
/* Returns suffix node, or NULL if no such node and flag is FALSE.          */
/*                                                                          */
/* arguments:              POINTER *node - from which to go to suffix       */
/*                         int      flag - TRUE if node should be created   */
/*                                                                          */
/* return value:           POINTER * - the final destination node           */
/*                                                                          */
/* globals referenced:     g_wp, g_record, g_recdirty                       */
/*                                                                          */
/* globals changed:        g_record, g_recdirty                             */
/*                                                                          */
/* synopsis:                                                                */
/*     sfxnode refers to the suffix subfield of the current window and      */
/*     uses s_findnode to find the node offset from the given node by       */
/*     that suffix.  If the given node is null, sfxnode issues a fatal      */
/*     error.  If the suffix node is found, it is returned.  If the         */
/*     suffix node does not exist and the input flag is not set, sfxnode    */
/*     returns null.  Otherwise it uses s_mknode to construct the           */
/*     new node and modifies g_record and g_recdirty accordingly            */
/*     before returning a pointer to the new (empty) node.                  */
/****************************************************************************/

POINTER *sfxnode (POINTER *node, int flag)
    {
    char *path;    /* suffix path                    */
    char *endnode; /* end node of suffix path        */
    char *newnode; /* node after path creation       */

#ifdef DEBUG
    debug ("sfxnode:  node = 0%o, flag = %s, g_snode = 0%o, g_wnode = 0%o",
	node, flag ? "YES" : "NO", g_snode, g_wnode);
#endif

#ifdef CAREFUL
    if (node == NULL)
	fatal ("sfxnode:  NULL node");
#endif

    path = w_sfx (g_wp);

#ifdef DEBUG
    debug ("sfxnode:  wf = '%s', path = '%s'", w_name (g_wp), path);
    objprint ("g_record", g_record);
    objprint ("node", node);
#endif

    if (path == NULL) /* if no suffix, return node */
	return (node);

    endnode = s_findnode (node, path); /* first try to find suffix */

    if ((endnode != (char *) ERROR) && (endnode != NULL))
	return ((POINTER *) endnode);

    if (flag == FALSE) /* if not creating, too bad */
	return (NULL);

    newnode = s_mknode (node, path, T_POINTER);
    g_record = fixtree (g_record, (char *) node, newnode);
    g_recdirty = TRUE; 
    return ((POINTER *) s_findnode ((POINTER *)newnode, path));
    }

/****************************************************************************/
/* writeline:  writes line in current window                                */
/*                                                                          */
/* arguments:              int   line - 0-based field coord of line to writ */
/*                         ATTR *text - line contents (in internal form)    */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wnode, g_snode, g_recdirty, g_wp, g_record     */
/*                                                                          */
/* globals changed:        g_recdirty, g_record, g_wnode, g_snode           */
/*                                                                          */
/* synopsis:                                                                */
/*     writeline writes out the given text into the given line of the       */
/*     current field.  If the g_wnode global is currently in error,         */
/*     writeline invokes pfxnode to process the prefix.  It also invokes    */
/*     idxnode to do the index processing.  The text is converted to        */
/*     external (file) form using packup.  The g_recdirty flag is set       */
/*     TRUE to indicate we are writing into the cache.                      */
/*                                                                          */
/*     If the field is not indexed, write the text in the line as a         */
/*     character array, freeing whatever was there before, and updating     */
/*     g_wnode and g_snode if they correspond to this line.                 */
/*                                                                          */
/*     If the field is indexed and also named, write the line into the      */
/*     name of the current line (unless it is the dummy [NO NAME]).         */
/*     If it is indexed and not named, invoke sfxnode to do suffix          */
/*     processing and then write the text into the appropriate line         */
/*     of the suffix node.                                                  */
/****************************************************************************/

void writeline (int line, ATTR *text1)
    {
    WSTRUCT *wp;       /* current window                 */
    char *newnode;     /* converted version of text      */
    POINTER *node;     /* node corresponding to index    */
    POINTER *prevnode; /* temporary for node             */

#ifdef DEBUG
    debug ("writeline:  line = %d", line);
    if (text1 && g_debugfp)
	s_print (text1);
#endif

    if (line >= OBJSIZE)
	{
	Emessage (M_E_LINES, "The file has too many lines.\nThe number of lines cannot exceed %d.", OBJSIZE);
	(void) sleep (5);
	fatal ("writeline: number of lines exceeded %d.", OBJSIZE);
	}

    if ((g_wnode == (char *) ERROR) || (g_wnode == NULL)) /* set up g_wnode */
	pfxnode (TRUE);                                    /* if needed      */

    newnode = packup(text1);
    node = idxnode (line, TRUE); /* find the indexed child (put into node) */

#ifdef DEBUG
    objprint ("wline: node returned by idxnode--", node);
    objprint ("wline: g_wnode after idxnode--", g_wnode);
    debug ("wline: (after idxnode:  g_wnode = 0%o, g_snode = 0%o",
	g_wnode, g_snode);
#endif

    /* we now are ready to process the suffix (relative to node) */
    /* Note:  if parent equal node, we didn't descend            */

    g_recdirty = TRUE; /* mark cache as dirty */
    wp = g_wp;

    if (! getstat (w_flags (wp), INDEXED)) /* replace node w/ copy of text */
	{
	/* force single child pointer arrays to become character arrays */

	if ((obj_count (g_wnode) == 1) && (obj_type (g_wnode) == T_POINTER))
	    {
	    node = (POINTER *) g_wnode;
	    s_free (node [0]);
	    node [0] = NULL;
	    }
	s_newname (newnode, obj_name (node)); /* copy node name to newnode */
	g_record = fixtree (g_record, (char *) node, newnode);

	/* if we didn't descend or we created a character array in */
	/* place of g_wnode, update g_wnode                        */

	if (g_wnode == (char *) node)
	    {
	    g_wnode = newnode;

	    if (g_snode == (char *) node) /* update g_snode if necessary */
		g_snode = newnode;
	    }
	s_free ((char *) node);
	return;
	}
    /* if selector, change name */

    if (getstat (w_flags (wp), NAMEFIELD))
	{
	if (! seq (newnode, "[NO NAME]"))
	    s_newname ((char *) node, (char *) newnode);

	return;
	}

    node = sfxnode (prevnode = node, TRUE); /* do suffix processing */

    if (node == NULL)
	{
	/* node == NULL implies indexed field with NULL entry in the
	   pointer array. In this case we want to put newnode in place
	   of the NULL entry */

	prevnode [atoi (w_sfx (wp))] = newnode;
	return;
	}

    if ((obj_type (node) == T_POINTER) && (obj_count (node) > 1))
	{
	s_newname (newnode, obj_name (node [0]));
	s_free (node [0]);
	node [0] = newnode;
	return;
	}
    /* if ended up at char array or length <= 1 pointer node, */
    /* replace it with a copy of the text.                    */

    s_newname (newnode, obj_name (node));
    g_record = fixtree (g_record, (char *) node, newnode);
    s_free ((char *) node);
    }

/****************************************************************************/
/* emptyrecord: determine if the record is empty                            */
/*                                                                          */
/* arguments:              POINTER *record - record to be checked           */
/*                                                                          */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     emptyrecord recursively searches record to determine if it is        */
/*     empty or not.  The record is considered empty if every pointer       */
/*     has either a count of 0, or if the first character is '\0'.          */
/****************************************************************************/

int emptyrecord (POINTER *record)
{
    int i;

#ifdef DEBUG
    debug ("emptyrecord: ");
#endif

    /* return TRUE if record is a NULL pointer */
    if (record == NULL)
        return (TRUE);

    /* if record is a POINTER, recursively search it for any contents */
    if (obj_type (record) == T_POINTER)
        {
        for (i = 0; i < obj_count (record); i++)
            {
            if (! emptyrecord ((POINTER *)record [i]))
                return (FALSE);
            }
        return (TRUE);
        }

    /* must be T_CHAR */
    else
        {
        if (obj_count (record) == 0 || *record == '\0')
            return (TRUE);
        else
            return (FALSE);
        }
}
