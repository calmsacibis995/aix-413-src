#if !defined(lint)
static char sccsid[] = "@(#)95	1.6  src/tenplus/e2/common/misc.c, tenplus, tenplus411, GOLD410 3/23/93 11:52:20";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Enumsiblings, Ereopen, Esavefile, Esync, findfile,
 *		firstnode, formprefix, gotosibling, inlist,
 *		nextsibling, Remove, restofpath, trimpath, Esetalarm,
 *		Eclearalarm, Ediskfull
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
/* misc.c                                                                   */
/*                                                                          */
/* miscellaneous routines for the e2 editor                                 */
/****************************************************************************/
#include "def.h"
#include "INeditor_msg.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

extern char *g_restart; /* for error recovery in Ereopen */

/****************************************************************************/
/* Enumsiblings:  returns number of nodes at current level                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - number of nodes at this level,             */
/*                               or ERROR if now at file level              */
/*                                                                          */
/* globals referenced:     g_snode, g_sfp, g_spath, g_recdirty,             */
/*                         g_recindex, g_record                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If currently at the file level, return ERROR.                        */
/*                                                                          */
/*     If at the record level, check to see if the g_recdirty               */
/*         global is set and, if so, whether g_recindex is off              */
/*         the end of the record array.  Return either the number           */
/*         of records in the file or g_recindex + 1, whichever is           */
/*         greater.                                                         */
/*                                                                          */
/*     If below the record level, consider the current node.                */
/*         If it is empty, return 0.  If it is a character array,           */
/*         return 1.  If it is a pointer array, return the number           */
/*         elements in it.                                                  */
/****************************************************************************/

int Enumsiblings (void)
    {
    register char *parent;  /* parent of current node        */
    register char *path;    /* path to parent                */
    register char *subpath; /* path to parent inside record  */
    register count;         /* number of records in file     */

#ifdef DEBUG
    debug ("Enumsiblings called");
#endif

    if (g_snode == (char *) sf_records (g_sfp)) /* if at file level */
	return (ERROR);

    flush_window ();

    path = trimpath (s_string (g_spath));

    /* If at record level, return number of records.                     */
    /* Note that the cache may contain a record off the end of the list  */
    /* If it's dirty and it's off the end of the record array, use the   */
    /* record index of the cache instead of the size of the record array.*/

    if (path == NULL)
	{
	count = obj_count (sf_records (g_sfp));

	if ((g_recdirty) && (g_recindex >= count))
	    return (g_recindex + 1);

	return (count);
	}
    subpath = restofpath (path);
    s_free (path);

    parent = s_findnode ((POINTER *)g_record, subpath);
    s_free (subpath);

    if (parent == (char *) ERROR) /* if parent does not exist, return 0 */
	return (0);

    if (obj_type (parent) == T_CHAR)
	return (1);

    return (obj_count (parent));
    }

/****************************************************************************/
/* Ereopen:  reopens file after a Esync                                     */
/* Works for either structured or fake files                                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_syncname, g_fakefile, g_sfp, g_readonly,       */
/*                         g_dfltfile, g_noindex, g_restart                 */
/*                                                                          */
/* globals changed:        g_syncname, g_sfp, g_restart                     */
/*                                                                          */
/* synopsis:                                                                */
/*     Look at the g_syncname global to see what file was Esync'ed.         */
/*     If g_syncname is null, issue a fatal error.  Otherwise open          */
/*     g_sfp to point to either the real or fake file being edited.         */
/*     If the attempt to open g_sfp fails, move to the default file         */
/*     named in the g_dfltfile global.  Otherwise expand the current        */
/*     form, invoke digin to get to the right level, and  get all           */
/*     the windows.                                                         */
/****************************************************************************/

void Ereopen (void)
    {
    register char *filename;

    filename = g_syncname;

#ifdef DEBUG
    debug ("Ereopen:  called, g_syncname = '%s'", filename);
#endif

#ifdef CAREFUL
    if (filename == NULL) /* if file was not Esync'ed */
	fatal ("Ereopen:  g_syncname NULL");
#endif

    if ( fexists( filename ) )
	{
	if (g_fakefile)
	    g_sfp = openfake (filename);
	else
	    g_sfp = sf_open (filename, g_readonly ? "r" : "u");
	}
    else
	g_sfp = NULL;

    g_syncname = NULL;

    /*  g_sfp will be NULL if file has vanished, ERROR if it didn't open */

    if (g_sfp == (SFILE *)ERROR || g_sfp == NULL)
	{
	Emessage (M_EREOPENDEL, "%s has been deleted.", filename);
	s_free (filename);
	filename = abspath (g_dfltfile, g_curdir);

	closehelper ();  /* turn off helper (if any) that called Ereopen */

	if ( !g_noindex)  /* start index helper back in action loop */
	    g_restart = s_string (DIRHELPER);

	Emessage (M_EREOPENCHANGE, "Changing to %s.", filename);
	if (setfile (filename, FALSE, TRUE) == ERROR)
	    fatal ("Ereopen:  setfile failure on file '%s'", filename);
	}
    else
	{
	digin ();
	getall (); /* do getlines on all windows */
	}
    s_free (filename);
    }

/****************************************************************************/
/* Esavefile:  saves current file                                           */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - RET_OK if saved, ERROR if file has not been*/
/*                               modified                                   */
/*                                                                          */
/* globals referenced:     g_recdirty, g_sfp, g_fakefile, g_filename,       */
/*                         g_wnode                                          */
/*                                                                          */
/* globals changed:        g_sfp, g_wnode                                   */
/*                                                                          */
/* synopsis:                                                                */
/*     Esavefile looks at the current file to see if it has been            */
/*     changed.  If so, it is closed and, if a fake file, saved,            */
/*     and then reopened.  If it has not been changed, invoke Eerror.       */
/****************************************************************************/

int Esavefile (void)
    {
    register char *realname; /* real file name                  */

#ifdef DEBUG
    debug ("Esavefile:  g_filename = '%s', g_fakefile = %d",
	g_filename, g_fakefile);
#endif

    flush_window ();

    if (( !g_recdirty) &&          /* if file has not been modified */
	((g_sfp == NULL) ||
	 (obj_type (g_sfp) != T_SFILE) ||
	 (( !g_fakefile) && ( !obj_getflag (g_sfp, SF_MODIFIED))) ||
	 (g_fakefile && (sf_eofloc (g_sfp) == EMPTYEOF))))
	return (ERROR);

    realname = s_string (obj_name (g_sfp));

    closefile ();

    if (g_fakefile) /* if fake file, convert it back to ASCII and reopen */
	{
	saveascii (g_filename);

	/***** Note:  since file was modified, directory not read-only *****/

	g_sfp = openfake (g_filename);
	}
    else
	g_sfp = sf_open (realname, "u"); /* reopen file */

    digin ();                    /* set up g_snode     */
    g_wnode = (char *) ERROR;    /* invalidate g_wnode */
    s_free (realname);
    return (RET_OK);
    }

/****************************************************************************/
/* Esync:  sync current file                                                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - allocated copy of filename being        */
/*                                  sync'ed                                 */
/*                                                                          */
/* globals referenced:     g_sfp, g_syncname, g_fakefile, g_filename        */
/*                                                                          */
/* globals changed:        g_syncname                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     If the g_sfp global is null, or g_syncname is NOT null, issue a      */
/*     fatal error.  (The only way g_syncname could have a value is if      */
/*     there has been a sync without a corresponding reopen).  Close the    */
/*     current file and, if we are editing a fake file, invoke saveascii to */
/*     back it up.  Finally, set g_syncname to the name of the file sync'ed */
/*     and return i							    */
/****************************************************************************/

char *Esync (void)
    {
    register char *filename; /* current real file name */

#ifdef CAREFUL
    if (g_sfp == NULL)
	fatal ("attempt to use Esync when g_sfp is NULL");

    if (g_syncname != NULL)
	fatal ("Esync called before Ereopen");
#endif

    /***** use file name if ASCII file, else name of "real" file *****/
    /***** NOTE:  this is important for directories              *****/

    filename = s_string (g_fakefile ? g_filename : obj_name (g_sfp));

#ifdef DEBUG
    debug ("Esync:  g_syncname = '%s'", filename);
#endif

    closefile ();

    if (g_fakefile) /* convert fake files back to ASCII */
	saveascii (filename);

    g_syncname = s_string (filename);
    return (filename);
    }

/****************************************************************************/
/* findfile:  finds form/helper file from prefix and extension              */
/*                                                                          */
/* arguments:              char *prefix      - filename prefix to find      */
/*                         char *extension   - filename extension           */
/*                         POINTER *pathlist - search directories           */
/*                                                                          */
/* return value:           char * - full path name to the file              */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     For each of the directories in the given pathlist, findfile          */
/*     sees if a file with the given prefix and extension exists            */
/*     there.  It returns the full pathname for the first one found.        */
/*     If the file does not exist in any of the directories, findfile       */
/*     returns NULL.                                                        */
/****************************************************************************/

char *findfile (register char *prefix, register char *extension, register POINTER *pathlist)
     /* filename prefix    */
   /* filename extension */
 /* search directories */
    {
    register i;                 /* pathlist index     */
    char pathname [PATH_MAX];   /* full path name     */
    struct stat buffer;         /* stat call buffer   */

#ifdef DEBUG
    debug ("findfile:  prefix = '%s', extension = '%s'", prefix, extension);
#endif

    (void) s_typecheck ((char *) pathlist, "findfile", T_POINTER);

    for (i = 0; i < obj_count (pathlist); i++)
	{
	(void) sprintf (pathname, "%s/%s%s", pathlist [i], prefix, extension);

#ifdef DEBUG
	debug ("findfile:  checking file '%s'", pathname);
#endif

	if (stat (pathname, &buffer) != ERROR)
	    return (s_string (pathname));
	}
    return (NULL);
    }

/****************************************************************************/
/* firstnode: finds part of pathname up to the first slash                  */
/*                                                                          */
/* arguments:              char *path - in which to find first part         */
/*                                                                          */
/* return value:           char * - allocated string with the first node    */
/*                                  in the path, or NULL                    */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     firstnode loops through the characters of the given string,          */
/*     stopping at the first slash or at the end of the string.             */
/*     If the result is non-null, an allocated string is returned;          */
/*     otherwise a non-allocated null string.                               */
/****************************************************************************/

char *firstnode (register char *path)
           /* path into record array  */
    {
    register char *cp;          /* name buffer pointer     */
    char namebuf [NAMESIZE];    /* record name             */

#ifdef DEBUG
    debug ("firstnode:  path = '%s'", path);
#endif

    cp = namebuf;

    while ((*path != DIR_SEPARATOR) && (*path != '\0'))
	*cp++ = *path++;

    if (cp == namebuf)
	return (NULL);

    *cp = '\0';
    return (s_string (namebuf));
    }

/****************************************************************************/
/* formprefix: return part of file name after last slash or dot             */
/*                                                                          */
/* arguments:              char *path - file name ending in form prefix     */
/*                                                                          */
/* return value:           char * - allocated copy of form prefix           */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     formprefix loops backwards through the characters of the given path, */
/*     stopping at the first dot or slash character, or at the beginning of */
/*     the string.  The portion of the filename after the last dot or slash */
/*     is returned as an allocated string.                                  */
/*                                                                          */
/*     One exception:  if the last character of the given path is a dot,    */
/*     that dot is ignored and formprefix proceeds as above.                */
/****************************************************************************/

char *formprefix (register char *path)

    {
    register char *string;
    register char *cp;

#ifdef DEBUG
    debug ("formprefix:  path = '%s'", path);
#endif

    string = s_string (path);
    cp = string + obj_count (string) - 1; /* get pointer to last character */

    if (*cp == '.') /* treet "foo." as "foo" */
	*cp-- = '\0';

    while (cp > string)
	if ((*cp == DIR_SEPARATOR) || (*cp == '.'))
	    {
	    cp++;
	    break;
	    }
	else
	    cp--;

#ifdef DEBUG
    debug ("formprefix:  cp = '%s'", cp);
#endif

    cp = s_string (cp);
    s_free (string);
    return (cp);
    }

/****************************************************************************/
/* gotosibling:  moves editing level to sibling of current node             */
/*                                                                          */
/* arguments:              char *name - name of sibling to move to          */
/*                                                                          */
/* return value:           int - RET_OK if moved; ERROR if at file level    */
/*                                                                          */
/* globals referenced:     g_snode, g_sfp, g_spath, g_warray                */
/*                                                                          */
/* globals changed:        g_spath, g_warray                                */
/*                                                                          */
/* synopsis:                                                                */
/*     If the editor is currently at file level, return ERROR.              */
/*     Otherwise concatenate the given name to g_spath, invoke              */
/*     digin to go there, and loop through the windows in g_warray,         */
/*     moving the cursor and window to the upper left corner of             */
/*     each field.  Finally, invoke getall to do getlines on all            */
/*     the windows, and return RET_OK.                                      */
/****************************************************************************/

int gotosibling (register char *name)
        /* name of sibling */
    {
    register WSTRUCT *wp;
    register i;

#ifdef DEBUG
    debug ("gotosibling:  name = '%s'", name);
#endif

    flush_window ();   /* make sure file is up to date */

    if (g_snode == (char *) sf_records (g_sfp))
	return (ERROR);

    g_spath = pathcat (trimpath (g_spath), makestring (name));
    digin ();

    for (i = 1; i < obj_count (g_warray); i++)
	{
	wp = &g_warray [i];
	wp->w__line = 0;   /* move cursor to ulhc */
	wp->w__col = 0;
	wp->w__ftline = 0; /* move window to ulhc of file */
	wp->w__ftcol = 0;
	}
    getall ();             /* do getlines on all windows */
    g_cursorflag = TRUE;   /* this ensures fix cursor in Edisplay */
    return (RET_OK);
    }

/****************************************************************************/
/* inlist:  determines if a string is in a list of strings                  */
/*                                                                          */
/* arguments:              POINTER *list - in which to search               */
/*                         char  *string - to search for                    */
/*                                                                          */
/* return value:           int - YES if string is in list, or NO            */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     inlist simply loops through the strings in the given list,           */
/*     comparing each one to the given string.  If the string is            */
/*     found, inlist returns YES.  If it gets all the way through           */
/*     the list without finding a match, it returns NO.                     */
/****************************************************************************/

int inlist (register POINTER *list, register char *string)

    {
    register i;

#ifdef CAREFUL
    (void) s_typecheck (list, "inlist", T_POINTER);
#endif

#ifdef DEBUG
    debug ("inlist:  list = 0%o, string = '%s'", list, string);
#endif

    for (i = 0; i < obj_count (list); i++)
	if (seq (string, list [i]))
	    return (TRUE);

    return (FALSE);
    }

/****************************************************************************/
/* nextsibling:  goes to next or previous sibling                           */
/*                                                                          */
/* arguments:              int flag - TRUE for next, FALSE for previous     */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_snode, g_spath                                 */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     nextsibling goes to the next or previous sibling of the current      */
/*     node, or invokes Eerror to complain of an error condition.           */
/*                                                                          */
/*     The error conditions are:                                            */
/*         attempt to go to previous item in a list of files                */
/*         attempt to go to next item in a list of files                    */
/*         attempt to go to previous item of a non-indexed node             */
/*         attempt to go to next item of a non-indexed node                 */
/*         attempt to go to previous item of the first node in a list       */
/*                                                                          */
/*     Note that it is not an error to go to the next item after the end    */
/*     of a list.  nextsibling flushes the current window, determines       */
/*     which sibling is desired, and invokes gotosibling to go there.       */
/****************************************************************************/

void nextsibling (int flag)
     /* TRUE if next, FALSE if previous */
    {
    register char *lastnode; /* last node in g_spath      */
    register char *path;     /* g_spath pointer        */
    register index1;          /* numeric value of lastnode */
    char newnode [10 * MB_LEN_MAX]; /* ascii version of new node */

#ifdef DEBUG
    debug ("nextsibling:  flag = %s", flag ? "TRUE" : "FALSE");
#endif

    flush_window ();   /* make sure file is up to date */

    if (g_snode == (char *) sf_records (g_sfp))
	{
	if (flag)
	    Eerror (M_ENEXT1, "Cannot go to the next item when not in a list.");
	else
	    Eerror (M_ENEXT2, "Cannot go to the previous item when not in a list.");

	return;
	}
    path = g_spath;

    for (lastnode = path + strlen (path) - 1; lastnode > path; lastnode--)
	if (*lastnode == DIR_SEPARATOR)
	    {
	    lastnode++;
	    break;
	    }
#ifdef DEBUG
    debug ("nextsibling:  path = '%s', lastnode = '%s'", path, lastnode);
#endif

    if ( !isdigit(*lastnode)) /* if node not number */
	{
	if (flag)
	    Eerror (M_ENEXT3, "Cannot go to the next item of this list.");
	else
	    Eerror (M_ENEXT4, "Cannot go to the previous item of this list.");

	return;
	}
    index1 = atoi (lastnode);

    if (flag)
	index1++;
    else
	{
	index1--;

	if (index1 < 0)
	    {
	    Eerror (M_EPREVITEM, "There is no previous item in this list.");
	    return;
	    }
	}
    (void) sprintf (newnode, "%d", index1);
    (void) gotosibling (newnode);
    }

/****************************************************************************/
/* Remove: unlinks a file.  If a directory, take no action.                 */
/*                                                                          */
/* arguments:              char *filename - to be removed                   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Remove first stats the file to see that it exists and is not a       */
/*     directory.  If it does not exist, or if it is a directory, it        */
/*     does nothing.  Otherwise it invokes unlink to remove the file.       */
/****************************************************************************/

void Remove (char *filename)
    {
    struct stat statbuf;

    if (stat (filename, &statbuf) == ERROR)
	return;

    if (statbuf.st_mode & S_IFDIR)
	return;

    (void) unlink (filename);
    }

/****************************************************************************/
/* restofpath:  returns rest of path after first slash                      */
/*                                                                          */
/* arguments:              char *path - from which to get rest of path      */
/*                                                                          */
/* return value:           char * - allocated copy of part after slash,     */
/*                                  or NULL if path has no slash            */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     restofpath loops through the characters of the given string,         */
/*     stopping at the first slash or at the end of the string.             */
/*     If the string had a slash, it returns an allocated copy of           */
/*     the part of the string after (and not including) the slash.          */
/*     Otherwise it returns null.                                           */
/****************************************************************************/

char *restofpath (register char *path)
    {
    register char *cp;

#ifdef DEBUG
    debug ("restofpath:  path = '%s'", path);
#endif

    for (cp = path; *cp != '\0'; cp++)
	if (*cp == DIR_SEPARATOR)
	    return (s_string (++cp));

    return (NULL);
    }

/****************************************************************************/
/* trimpath:  trims last name from path.                                    */
/*                                                                          */
/* arguments:              char *path - to have last name removed           */
/*                                                                          */
/* return value:           char * - allocated copy of path less last name,  */
/*                                  or NULL if path has no slash            */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     trimpath removes the last name from the given path, does             */
/*     s_realloc to reallocate the given path, and returns it.              */
/*     If the path has no slash in it, trimpath returns null.               */
/*     If the only slash in the path is at its beginning, the               */
/*     returned path consists of a single slash.  Otherwise trimpath        */
/*     removes both the last name and the last slash from the path.         */
/****************************************************************************/

char *trimpath (register char *path)
    {
    register char *cp;

#ifdef DEBUG
    debug ("trimpath:  path = '%s'", path);
#endif

    if (path == NULL)
	return (NULL);

    for (cp = path + strlen (path) - 1; cp > path; cp--)
	if (*cp == DIR_SEPARATOR)
	    break;

    if (cp == path)
	{
	if (*cp != DIR_SEPARATOR) /* if not "/foo" */
	    {
	    s_free (path);
	    return (NULL);
	    }
	else
	    cp++; /* if "/foo", trim to "/" */
	}

    path = s_realloc (path, cp - path);
    return (path);
    }


/****************************************************************************/
/* Esetalarm:  set the helper's alarm frequency                             */
/*                                                                          */
/* arguments:            int freq - alarm frequency, in seconds             */
/*                                                                          */
/* return value:         int - RET_OK if reasonable frequency, else ERROR   */
/*                                                                          */
/* globals referenced:   g_afreq, g_atime                                   */
/*                                                                          */
/* globals changed:      g_afreq, g_atime                                   */
/*                                                                          */
/* synopsis:                                                                */
/*   Esetalarm sets the necessary alarm globals so that the helper will be  */
/*   given a Ualarm call at the given frequency starting from now.          */
/****************************************************************************/
/*ARGSUSED*/
int Esetalarm (int freq)
         /* alarm frequency, in seconds  */
    {
    }

/****************************************************************************/
/* Eclearalarm:  clear the helper's alarm if it was set                     */
/*                                                                          */
/* arguments:            none                                               */
/*                                                                          */
/* return value:         int - RET_OK if alarm was set, else ERROR          */
/*                                                                          */
/* globals referenced:   g_afreq                                            */
/*                                                                          */
/* globals changed:      g_afreq                                            */
/*                                                                          */
/* synopsis:                                                                */
/*   Eclearalarm first checks the alarm frequency global to make sure an    */
/*   alarm is actually currently set.  If not, it returns ERROR.  Otherwise */
/*   it sets the alarm frequency to zero, turning off future alarms, and    */
/*   returns RET_OK.                                                        */
/****************************************************************************/

int Eclearalarm (void)
    {
    }


/****************************************************************************/
/* Ediskfull: handles the full disk situation                               */
/*                                                                          */
/* arguments:              int filename - file that was being written/      */
/*                                        opened                            */
/*                                                                          */
/* return value:           none                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Ediskfull is called when the system returns an error upon a file     */
/*     written or open.  If the user wishes, a shell is presented at which  */
/*     time file space can be reclaimed.  If not, then the editor is exited */
/*     without saving any of the opened files.                              */
/****************************************************************************/

void Ediskfull(char *filename)
{
    if (Econfirm (M_EDISKFULL,
"Cannot write to file %s.\nTouch EXECUTE to escape to the shell and\
 fix\n or touch CANCEL to exit editor without saving files.",filename))
	{
	int pid;
	int count = 0;
	void (*func)(int);

	func = signal (SIGQUIT, SIG_IGN);

	for (;;)
	    {
	    pid = fork();
	    if (pid != ERROR)
		break;
	    if (count++ > 2)
		{
		Emessage (M_EFORK, "Cannot create another process now.  Try again later.");
		exit (1);
		}
	    (void) sleep (2);
	    }
	if (pid != 0)      /* we are now the proud parent */
	    {
	    int ret_val;

	    while (wait (&ret_val) != pid);

	    if (ret_val != 0)         /* shell returned failure */
		{
		Emessage (M_ESHELLFAIL, "Shell failed with return value = %d", ret_val);
		exit (1);
		}
	    else    /* shell has returned successfully */
		{
		term_enter ();
		Si_display (FALSE);
		Emessage (M_ERETRYING, "Trying again.");
		(void) signal (SIGQUIT, func);
		return;
		}
	    } /* end    if (pid != 0) */
	else    /* here we're the child process */
	    {
	    term_exit ();       /* clean up the terminal modes */
	    (void)close (0);
	    (void)close (1);
	    (void)close (2);
	    open ("/dev/tty", 0);
	    open ("/dev/tty", 1);
	    open ("/dev/tty", 1);
	    execl ("/bin/sh", "sh", 0);
	    exit (1);   /* if it gets here, execl failed - return to parent*/
	    }
	}
    else
	Si_quit ();
	exit(1);
    }
