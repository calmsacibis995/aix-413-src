static char sccsid[] = "@(#)82  1.12  src/tenplus/e2/common/sp.c, tenplus, tenplus411, GOLD410 11/1/93 17:50:35";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	Eusefile, altfile, checkfile, checktext, closefile,
 *		defaultform, ghost, createfile, filemenu, makefile,
 *		makeprofile, tp_opendir, opentext, setfile, usefile 
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/
/* file:  sp.c                                                              */
/*                                                                          */
/* file opening and closing routines for the e2 editor                      */
/****************************************************************************/


#include <sys/stat.h>
extern int     modebits (char *, struct stat *);
#include <sys/access.h>
#include <signal.h>
#include <sys/wait.h>

#include "def.h"
#include "keynames.h"
#include "editorprf.h"
#include "INeditor_msg.h"

/* File types from file creation menu */
#define ASCII_FILE    (0)
#define STRUCT_FILE   (1)
#define DIR_FILE      (2)
#define RE_ENTER_FILE (3)

static g_newfake; /* TRUE if new file is fake */

LOCAL int checkfile (char *, struct stat *);
LOCAL int checktext (char *);
LOCAL int createfile (char *, int, char *);
LOCAL int filemenu (char *);
LOCAL int ghost (char *);
LOCAL int makefile (char *, int, int);
LOCAL int makeprofile (char *);
static void tp_opendir (char *, int);
LOCAL void opentext (char *, int, int);

/****************************************************************************/
/* Eusefile:  change the file and form (helper callable)                    */
/*                                                                          */
/* arguments:              char *filename - name of file to change to       */
/*                                                                          */
/* return value:           int - RET_OK if successfully set to the file,    */
/*                               or ERROR if failed                         */
/*                                                                          */
/* globals referenced:     g_curdir, g_filename, g_sfp, g_zoomstack         */
/*                                                                          */
/* globals changed:        g_curdir, g_zoomstack                            */
/*                                                                          */
/* synopsis:                                                                */
/*     Eusefile is a helper callable routine to move the editor to a        */
/*     different file.  It is not entirely analagous to the USE key,        */
/*     in that the file must not be a directory, does not become the        */
/*     alternate file, and does not affect the zoom stack.                  */
/*                                                                          */
/*     If the given filename corresponds to the current g_filename,         */
/*     Eusefile attempts to create the file if it does not exist.           */
/*     If the creation attempt fails, it issues a fatal error.              */
/*     If it succeeds, Eusefile first closes the file it just created       */
/*     and then does a setfile to take the editor there.  Eusefile          */
/*     returns whatever the setfile call returned (RET_OK or ERROR).        */
/*                                                                          */
/*     If the given filename corresponds to the current g_filename          */
/*     and the file does exist, Eusefile checks to see whether the          */
/*     file has been modified more recently than the time recorded          */
/*     in structured file pointer and, if so, does an Esync/Ereopen.        */
/*     It then returns RET_OK.                                              */
/*                                                                          */
/*     If the filename differs from g_filename, then Eusefile checks        */
/*     to see whether the named file does not exist or if it is a           */
/*     directory and, if so, returns ERROR.  Otherwise it calls             */
/*     setfile to move the editor to the named file.  If the new            */
/*     file is in a different directory than the old, Eusefile does         */
/*     a chdir to the new directory and returns the result of the           */
/*     setfile (RET_OK or ERROR).   If the chdir fails, Eusefile issues     */
/*     a fatal error.                                                       */
/****************************************************************************/

int Eusefile (char *filename)
    {
    char olddir [PATH_MAX]; /* g_curdir             */
    char *absname;          /* absolute path name   */
    int retval;             /* setfile return value */

    FILE *fp;               /* used to create file  */
    POINTER *stack;         /* save the zoomstack   */

    struct stat statbuf;    /* stat buffer for file */

    flush_window ();
    /* convert to absolute file name */
    absname = abspath (filename, g_curdir);

#ifdef DEBUG
    debug ("Eusefile:  filename = '%s', absname = '%s'", filename, absname);
#endif

    if (seq (absname, g_filename)) /* if already editing the file */
	{
	s_free (absname);

	if (stat (g_filename, &statbuf) == ERROR)
	    {
	    fp = ffopen (g_filename, "w", "Eusefile"); /* try to create file */

	    (void) fclose (fp);
	    return (setfile (g_filename, FALSE, TRUE));
	    }
	if (statbuf.st_mtime > sf_time (g_sfp)) /* if file has changed */
	    {
	    s_free (Esync ());
	    Ereopen ();
	    }
	return (RET_OK);
	}
    if ((stat (absname, &statbuf) == ERROR) || /* if file doesn't exist or */
	(statbuf.st_mode & S_IFDIR))
	{
	s_free (absname);
	return (ERROR);
	}

    (void) strcpy (olddir, g_curdir); /* save current directory */
    stack = g_zoomstack;              /* save current zoomstack */
    g_zoomstack = NULL;
    retval = setfile (absname, FALSE, TRUE);
    s_free ((char *)g_zoomstack);      /* restore current zoomstack */
    g_zoomstack = stack;
    s_free (absname);

    if (! seq (olddir, g_curdir))
	{
	(void) strcpy (g_curdir, olddir);

	if (chdir (olddir) == ERROR) /* change to the old directory */
	    fatal ("Eusefile:  cannot change to directory '%s'", olddir);
	}
    return (retval);
    }

/****************************************************************************/
/* altfile:  change to alternate file                                       */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_altstack, g_zoomstack                          */
/*                                                                          */
/* globals changed:        g_altstack, g_zoomstack                          */
/*                                                                          */
/* synopsis:                                                                */
/*     altfile first checks to see that the g_altstack global is            */
/*     not null.  If it is, it invokes Eerror and returns.  Otherwise       */
/*     it flushes the current window and looks at the alternate file        */
/*     name.  If it does not exist, it does an Eerror return (unless        */
/*     the alternate file is the special poundfile "#", which is the        */
/*     only file that can be the alternate file without existing.)          */
/*                                                                          */
/*     In the normal case, altfile does a call to pushstate, then swaps     */
/*     the g_altstack and g_zoomstack globals, and then calls popstate.     */
/****************************************************************************/

void altfile (void)
    {
    POINTER *stack; /* used to exchange stacks */
    char *name;

#ifdef DEBUG
    debug ("altfile:  g_altstack = 0%o", g_altstack);
#endif

    if (g_altstack == NULL)
	{
	Eerror (M_ENOALTFILE, "There is no alternate file.");
	return;
	}
    flush_window ();
    name = altname ();

    if (! (seq (name, "#") || fexists (name)))
	{
	Eerror (M_EBADALTFILE, "Alternate file %s does not exist.", name);
	return;
	}

    pushstate (TRUE); /* save current state on zoom stack */

    stack = g_zoomstack;     /* exchange stacks */
    g_zoomstack = g_altstack;
    g_altstack = stack;

    popstate (TRUE); /* pop state of zoom stack */
    }

/****************************************************************************/
/* checkfile:  determines if it is OK to edit a file                        */
/*                                                                          */
/* arguments:              char *filename - file name to check              */
/*                         struct stat *statptr - pointer to its stack buff */
/*                                                                          */
/* return value:           LOCAL int - YES if the file can be edited, or NO */
/*                                                                          */
/* globals referenced:     g_newfake                                        */
/*                                                                          */
/* globals changed:        g_newfake                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     checkfile returns YES or NO to indicate whether a file is one the    */
/*     editor can edit.  If the filename begins with "...", it returns NO.  */
/*     If it is a special file, it returns NO.  If it is a directory, it    */
/*     returns YES if the user has both read and execute permission on it,  */
/*     otherwise NO.  If it is not a directory and the user does not have   */
/*     read permission, it returns NO.  Otherwise it invokes checktext      */
/*     to finish checking the text file and returns its result.             */
/****************************************************************************/

LOCAL int checkfile (char *filename, struct stat *statptr)
    {
    int filemodes;         /* mode bits for file       */
    char *cp;              /* For file name checking   */

#ifdef DEBUG
    debug ("checkfile:  filename = '%s'", filename);
#endif

    /* Check for ... files */
    cp = strrchr (filename, DIR_SEPARATOR) + 1;
    if (strncmp (cp, "...", 3) == 0)
	{
	Eerror (M_EFILEDOTS, "Do not edit files beginning with the ... sequence.");
	return (FALSE);
	}

    filemodes = statptr->st_mode;
    g_newfake = FALSE;

    /* check for special file */
    if (filemodes & S_IFCHR) /* NOTE: this also checks for block special */
	{
	Eerror (M_EFILESPECIAL, "Cannot edit a special file.");
	return (FALSE);
	}

# ifdef DEBUG
    debug ("checkfile:  filemodes = 0%o", filemodes);
# endif

    /* if file is directory it must have read & execute permission for user */
    if (filemodes & S_IFDIR)
	{
	if (accessx (filename, R_OK | X_OK, ACC_SELF) != 0)
	    {
	    Eerror (M_EFILEDIR,
	 "Cannot edit directory %s\nwithout both read and execute permissions.",
		    filename);
	    return (FALSE);
	    }
	return (TRUE);
	}

    /* a text file must have read permission */
    if (accessx (filename, R_OK, ACC_SELF) != 0)
	{
	Eerror (M_EFILEMODES, "You must have read permission to edit file\n%s", filename);
	return (FALSE);
	}

    /* if its a text file, make sure its a delta file or OK text file */
    return (checktext (filename));
    }

/****************************************************************************/
/* checktext:  checks to see if file is text or delta file.                 */
/*                                                                          */
/* arguments:              char *filename - purported text or delta file    */
/*                                                                          */
/* return value:           int - YES if text or delta file, else NO         */
/*                                                                          */
/* globals referenced:     g_newfake                                        */
/*                                                                          */
/* globals changed:        g_newfake                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     checktext checks the given text file to determine whether it is      */
/*     a believable normal text file or a structured file and, if so,       */
/*     sets the g_newfake global to TRUE and returns YES.  If it appears    */
/*     to be a broken structured file, it invokes the ghost procedure       */
/*     to try to fix it.                                                    */
/*                                                                          */
/*     The criteria are:                                                    */
/*         If the file is empty, return YES.                                */
/*         If the file has an end cookie six characters from the end,       */
/*             followed by a pointer to a valid array cookie, return YES.   */
/*         If the file begins with a start cookie, assume it is a           */
/*             broken structured file, feed it to the ghost procedure,      */
/*             and return whatever ghost returns.                           */
/*         Otherwise, set g_newfake to TRUE, close the file, and return YES */
/****************************************************************************/

LOCAL int checktext (char *filename)
    {
    FILE *fp;          /* text file pointer  */
    int i;             /* loop counter       */
    char c;            /* char from file     */
    long arrayloc;     /* array location     */

#ifdef DEBUG
    debug ("checktext:  filename = '%s'", filename);
#endif

    fp = ffopen (filename, "r", "checktext");

    /* see if it's a good delta file */
    (void) fseek (fp, 0L, 2); /* seek to end of file */
    if (ftell (fp) == 0L) /* if empty file, return YES */
	{
	(void) fclose (fp);

	/* This change declares 0 length files to be ASCII not structured */
	g_newfake = TRUE;
	return (TRUE);
	}

    (void) fseek (fp, -6L, 2); /* seek six chars from end of file */
    if ((fgetc (fp) == COOKIE) && (fgetc (fp) == C_END))
	{
	arrayloc = getl (fp);               /* get start of record array */
	(void) fseek (fp, arrayloc - 2, 0); /* seek to array cookie loc  */

	if ((fgetc (fp) == COOKIE) && (fgetc (fp) == C_ARRAY))
	    {
	    (void) fclose (fp);
	    return (TRUE);
	    }
	}

    /* see if it's a broken delta file */
    (void) fseek (fp, 0L, 0);
    if ((fgetc (fp) == COOKIE) && (fgetc (fp) == C_START))
	{
	(void) fclose (fp);
	return (ghost (filename));
	}
    g_newfake = TRUE; /* its an OK fake file */
    (void) fclose (fp);
    return (TRUE);
    }

/****************************************************************************/
/* closefile:  closes the current file if necessary                         */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_sfp, g_fakefile, g_diskfull, g_textfp          */
/*                                                                          */
/* globals changed:        g_sfp, g_diskfull                                */
/*                                                                          */
/* synopsis:                                                                */
/*     closefile closes the current file and the fake file if necessary.    */
/*     If the g_sfp global is null or does not point at a structured file,  */
/*     return with no action.  Otherwise flush the current window, flush    */
/*     the record array, attempt to close the structured file, and clear    */
/*     g_sfp.  If the attempt to close failed, set the g_diskfull global    */
/*     to TRUE and issue a fatal error.  If the g_fakefile global is set,   */
/*     also close the g_textfp file.                                        */
/****************************************************************************/

void closefile (void)
    {
    int retval;       /* sf_close return value */

#ifdef DEBUG
    debug ("closefile:  g_sfp = 0%o, g_fakefile = %s", g_sfp,
	g_fakefile ? "TRUE" : "FALSE");
#endif

    if ((g_sfp == NULL) || (obj_type (g_sfp) != T_SFILE))
	return;

    flush_window ();
    flushrecord (TRUE);

    retval = sf_close (g_sfp);
    g_sfp = NULL;

    if (retval == ERROR)
	{
	g_diskfull = TRUE; /* don't try to close file */
	Emessage (M_ESAVECLOSE, "Cannot close file %s.\n\
The disk may be full.  Use local problem reporting procedures.", g_filename);
	(void) sleep (5);
	fatal ("cannot close file '%s' (disk probably full)", g_filename);
	}
    if (g_fakefile)
	(void) fclose (g_textfp);
    }

/****************************************************************************/
/* defaultform: returns default form prefix for current file                */
/* Note:  g_sfp is assumed to be correct when this routine is called        */
/* Must be called AFTER digin.                                              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - form prefix "std" for array of lines    */
/*                                        or "sindex" for array of pointers */
/*                                                                          */
/* globals referenced:     g_sfp, g_snode, g_fakefile                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     defaultform categorizes the current file for which no explicit       */
/*     form is given into one of two classes - an array of text lines,      */
/*     for which the "std" form is appropriate, or an array of pointer      */
/*     objects, for which we want the "sindex" form.                        */
/*                                                                          */
/*     If the g_fakefile global is set, or g_snode is null, or g_snode      */
/*     is of type T_CHAR, use the standard form.  Note that for g_snode     */
/*     to be set properly, the digin procedure must already have been       */
/*     called                                                               */
/*                                                                          */
/*     If we are at the record level of the file, return the index form     */
/*     if the type of g_snode is RC_NONTEXT; otherwise return the standard. */
/*                                                                          */
/*     If digin has taken us below the record level, and if g_snode is a    */
/*     pointer object, loop through its children nodes.  If any of them     */
/*     is a non-null pointer object, or if any of them has its obj_name set */
/*     return the index form.  If all the children are null or text, or     */
/*     if g_snode itself is not a pointer, return the standard form.        */
/****************************************************************************/

char *defaultform (void)
    {
    POINTER *pointers;
    int i;

#ifdef DEBUG
    debug ("defaultform called");
#endif

    if ((g_snode == NULL) || (obj_type (g_snode) == T_CHAR) || (g_fakefile))
	return (s_string (STANDARDFORM));

    if (g_snode == (char *) sf_records (g_sfp))
	{
	if (obj_getflag (g_snode, RC_NONTEXT))
	    return (s_string (INDEXFORM));
	else
	    return (s_string (STANDARDFORM));
	}

    /* if all children of g_snode are character arrays or NULL */
    /* use std.form, else sindex.form                          */

    pointers = (POINTER *) g_snode;

    if (obj_type (pointers) == T_POINTER)
	for (i = 0; i < obj_count (pointers); i++)
	    if (pointers [i] != NULL)
		if ((obj_type (pointers [i]) != T_CHAR) ||
		    (obj_name (pointers [i]) != NULL))
		    return (s_string (INDEXFORM));

    return (s_string (STANDARDFORM));
    }

/****************************************************************************/
/* ghost:  attempts to reconstruct a broken delta file                      */
/*                                                                          */
/* arguments:              char *filename - broken file to fix              */
/*                                                                          */
/* return value:           LOCAL int - YES if successful, or NO             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     ghost is used to try to fix a file that begins with a start          */
/*     cookie but does not end with the correct array-cookie end-cookie     */
/*     combination.  It forks a new process and invokes the ghost           */
/*     program on the given file, redirecting its input and output          */
/*     to the null device.  If the fork attempt fails (typically            */
/*     because the system is out of process slots), it returns NO.          */
/*     If the external ghost program itself returns with non-zero           */
/*     status, this ghost issues a fatal error to stop the editor.          */
/*     (The ghost program typically fails only for lack of disk space.)     */
/*     Otherwise it returns YES.                                            */
/****************************************************************************/

LOCAL int ghost (char *filename)
    {

    int i;             /* loop variable                     */
    int child;         /* process id of child               */
    int zombie;        /* process id                        */
    int status;        /* return status of child            */

#ifdef DEBUG
    debug ("ghost:  filename = '%s'", filename);
#endif

    Emessage (M_EGHOST, "Trying to fix broken file %s.", filename);

    child = fork ();
    if (child == ERROR) /* check for fork error */
	{
	Eerror (M_EFORK, "Cannot create another process now.  Try again later.");
	return (FALSE);
	}
    if (child == 0) /* child process */
	{
	/* clear all signals that aren't being specifically ignored. */
	for (i = 0; i < NSIG; i++)
	    if (signal (i, SIG_IGN) != SIG_IGN)
		(void) signal (i, SIG_DFL);

	(void) close (0); /* standard input to the input file */

	open ("/dev/null", 0);

	(void) close (1); /* standard output to the output file */

	open ("/dev/null", 1);

	(void) close (2); /* standard error to output file */
	(void) dup (1);

	for (i = 3; i < _NFILE; i++) /* close all other channels */
	    (void) close (i);

	(void)execlp ("ghost", "ghost", filename, (char *)NULL);
	_exit (1); /* execl failure */
	}

    for (;;) /* parent */
	{
	zombie = wait (&status);

	if (zombie == child)
	    break;
	}
# ifdef DEBUG
    debug ("ghost:  child returned with status %d", status);
# endif

    if (status) /* if non zero status */
	fatal ("cannot fix broken file '%s' (disk probably full)", filename);

    return (TRUE);
    }

/****************************************************************************/
/* createfile:  create a file of the given type                             */
/*                                                                          */
/* arguments:            char *filename - to be created                     */
/*                       int   filetype - 0=ascii, 1=str, 2=dir             */
/*                       char *prfdir   - name of user's profile directory  */
/*                                                                          */
/* return value:         int - RET_OK or ERROR                              */
/*                                                                          */
/* globals referenced:   none                                               */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*   createfile actually creates a file of the given name and type.         */
/*   If creating a structured file in the user's profile directory, it      */
/*   tried to copy the corresponding file from the system profile directory.*/
/****************************************************************************/

LOCAL int createfile (char *filename, int filetype, char *prfdir)
    {
    FILE *fp;                 /* file pointer               */
    SFILE *sfp;               /* used to create struct file */
    char buffer [PATH_MAX];   /* sprintf buffer             */

    switch (filetype)
	{
	case ASCII_FILE:

	    Emessage (M_EMAKEFILE, "Creating file %s.", filename);
	    fp= efopen (filename, "w"); /* try to create file */

	    if (fp == NULL)
		return (FALSE);
	    (void) fclose (fp);
	    return (TRUE);

	case STRUCT_FILE:

	    Emessage (M_EMAKEFILE, "Creating file %s.", filename);

	    if (strncmp (filename, prfdir, obj_count (prfdir)) == 0)
		if (makeprofile (filename) == RET_OK)
		    return (TRUE);

	    sfp = esf_open (filename, "c");

	    if (sfp == (SFILE *) ERROR)
		return (FALSE);
	    (void) sf_close (sfp);
	    return (TRUE);

	case DIR_FILE:

	    Emessage (M_EMAKEDIR, "Creating directory %s.", filename);

	    (void) sprintf (buffer, "mkdir %s", filename);
	    filter (buffer, "/dev/null", "/dev/null");
	    if (! fexists (filename))

		{
		Eerror (M_EDIRCREATE,
			"Cannot create directory %s.", filename);
		return (FALSE);
		}
	    else
		return (TRUE);
	}
	return (FALSE);
    }

/****************************************************************************/
/* filemenu:  ask user what kind of file to make                            */
/*                                                                          */
/* arguments:            filename - of file being made                      */
/*                                                                          */
/* return value:         int - 0 = ascii, 1 = structured, 2 = directory,    */
/*                             3 = re-enter, ERROR = cancel                 */
/*                                                                          */
/* globals referenced:   g_noindex, g_windowfd                              */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*   filemenu puts up the file creation menu                                */
/****************************************************************************/

LOCAL int filemenu (char *filename)
    {
    int retval;               /* menu return value          */

    /* find out if user wants to make file */
    if (g_noindex) 
	retval = Emenu(MN_EFILERMENU, "You are attempting to create file\n%s.\n\n\
Select a menu option (for example, move the\n\
cursor to an item and press EXECUTE); or\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n\n\
@Create a text file (without history).\n\
@Create a structured file (with history).\n\
@Enter the file name again.#0", 0, filename);
     else 
	retval = Emenu(MN_EFILEDRMENU, "You are attempting to create file\n%s\n\n\
Select a menu option (for example, move the\n\
cursor to an item and press EXECUTE) or\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n\n\
@Create a text file (without history).\n\
@Create a structured file (with history).\n\
@Create a directory.\n\
@Enter the file name again.#0", 0, filename);

    if (g_noindex && retval > 1)
	retval++;

    return (retval);
    }

/****************************************************************************/
/* makefile:  asks whether to create file and, if so, does it               */
/*                                                                          */
/* arguments:              char *filename - file name to make               */
/*                         int helperflag - TRUE if helper should be start- */
/*                                          ed                              */
/*                         int formflag   - TRUE if Euseform should be      */
/*                                          called                          */
/*                                                                          */
/* return value:           int - YES if user confirmed request and file mad */
/*                               NO if user canceled out                    */
/*                               ERROR if user re-entered the filename      */
/*                                                                          */
/* globals referenced:     g_helper, g_popflag, g_keyvalue, g_curdir,       */
/*                         g_edfunction, g_filename, g_noindex              */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     makefile is invoked when the user has asked to move to a file that   */
/*     does not exist.  It asks what to do and then either makes the file,  */
/*     returning YES, or cancels the request, returning NO.  The details    */
/*     of how it works depend on whether the FILE_CREATION_HELPER #ifdef    */
/*     is in effect.                                                        */
/*                                                                          */
/*     If the file creation helper is enabled, makefile closes the current  */
/*     helper, opens the file creation helper, and iterates on a command-   */
/*     action-display loop until the user hits USE.  makefile then restores */
/*     the original helper and returns YES or NO depending on whether the   */
/*     file was created during the course of the action loop.               */
/*                                                                          */
/*     If the file creation helper is disabled, makefile walks up the       */
/*     absolute pathname to the given file, making sure that every          */
/*     directory on the way exists and is not protected against the user.   */
/*     If there is anything wrong with the path, it invokes Eerror and      */
/*     returns NO.                                                          */
/*                                                                          */
/*     If the path is all right, makefile then determines what kind of file */
/*     to make, either by the g_inmenu global or by calling filemenu.       */
/*     If the user wants to re-enter the filename, do so and return ERROR   */
/*     if a recursive call to setfile to the new filename succeeds.         */
/*     Finally, it calls createfile to actually make the file.              */
/****************************************************************************/

LOCAL int makefile (char *filename, int helperflag, int formflag)
    {
    char *dirname;              /* used to check ancestors    */
    struct stat statbuf;        /* stat buffer for dirname    */
    char *prfdir;               /* name of profile directory  */
    char *profiledir;           /* Efixvar of profile dir     */
    int   prfflag = FALSE;      /* whether to make prfdir     */
    int retval;                 /* menu return value          */

#ifdef DEBUG
    debug ("makefile:  filename = '%s'", filename);
#endif

    /* check ancestor directories to make sure they are directories
       and that the user has execute permission in them.  This is
       done to produce clearer error messages than "cannot create file" */

    dirname = s_string (filename);

    /* Since dirname is an abspath, prfdir must be also for the seq below
       to work.  This is crucial when $HOME is slash. */

    profiledir = Efixvars ("$HOME/profiles");
    prfdir = abspath (profiledir, "/");
    s_free (profiledir);


     /* If creating profiles directory from task menu, need to create it
       if it doesn't exist.  */

    prfflag = seq (prfdir, dirname) && g_inmenu; 

    while (obj_count (dirname) > 1)
	{
	dirname = trimpath (dirname);

	if (stat (dirname, &statbuf) == ERROR)
	    {
	    prfflag = seq (prfdir, dirname);
	    if (! prfflag)
		{
		Eerror (M_EBADPATH, "Directory %s does not exist.", dirname);
		s_free (prfdir);
		s_free (dirname);
		return (FALSE);
		}
	    }
	if (! prfflag && (statbuf.st_mode & S_IFDIR) == 0)
	    {
	    Eerror (M_EBADDIR, "File %s is not a directory.", dirname);
	    s_free (prfdir);
	    s_free (dirname);
	    return (FALSE);
	    }
	if (! prfflag && (modebits (dirname, &statbuf) & EXEC) == 0)
	    {
	    Eerror (M_EDIRMODES,
		    "You must have execute permission in directory\n%s to edit the specified file.",
		    dirname);
	    s_free (prfdir);
	    s_free (dirname);
	    return (FALSE);
	    }
	}
    s_free (dirname);


    /* If creating a file from the task menu, assume it is structured.
       Otherwise ask the user what kind of file to make. */

    retval = g_inmenu ? STRUCT_FILE : filemenu (filename);

    if (retval == ERROR) /* if user pressed CANCEL */
	{
	s_free (prfdir);
	return (FALSE);
	}

    /* If user wants to re-type filename, recursively invoke setfile here */

    if (retval == RE_ENTER_FILE)
	{
	s_free (prfdir);
	filename = Eask (M_ERE_ENTER_FILE, "New file name:                  ");
	if (filename == (char *) ERROR)         /* User canceled out */
	    return (FALSE);

	return ((setfile (filename, helperflag, formflag) == RET_OK) ? ERROR : FALSE);
	}

    /* Perform automatic creation of $HOME/profiles if necessary */

    if (prfflag)
	if ((retval = createfile (prfdir, DIR_FILE, prfdir)) == ERROR)
	    {
	    s_free (prfdir);
	    return (FALSE);
	    }

     /*  Now create whatever file we are making -- if our primary task
	 was not creating the profiles directory from the task menu. */

    if (! seq (filename, prfdir) || ! g_inmenu)	   
        retval = createfile (filename, retval, prfdir);
    s_free (prfdir);
    return (retval);
    }

/****************************************************************************/
/* makeprofile:  make a structured file in user profile directory           */
/*                                                                          */
/* arguments:            char *filename - to create                         */
/*                                                                          */
/* return value:         int - RET_OK or ERROR                              */
/*                                                                          */
/* globals referenced:   none                                               */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*   makeprofile attempts to copy the corresponding file from the system    */
/*   profiles directory into the new file.  If there is no such system file */
/*   it returns ERROR.                                                      */
/****************************************************************************/

LOCAL int makeprofile (char *filename)
    {
    char *basename;     /* base name of file to create       */
    char *sysprofile;   /* corresponding system profile file */
    int   result;    

    sysprofile = Efixvars (SYS_PROFILE_DIR);

    basename = strrchr (filename, DIR_SEPARATOR) + 1;
    sysprofile = pathcat (sysprofile, s_string (basename));

#ifdef	DEBUG
    debug ("makeprofile:  sysprofile %s, new profile %s", sysprofile, basename);
#endif


    Emessage (M_EPROFCOPY, "Copying standard profile %s.", sysprofile);

    result = Ecopyprofile(sysprofile,filename);

    return (result);
    }

/****************************************************************************/
/* tp_opendir:  opens a directory file (the tp_ is just to distinguish      */
/* 		this from the libc opendir()).                              */
/*                                                                          */
/* arguments:              char *filename - name of directory to open       */
/*                         int   helperflag - whether to start helper       */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     g_sfp, g_curdir, g_filename, g_dispname, g_snode */
/*                                                                          */
/* globals changed:        g_sfp, g_curdir, g_filename, g_dispname, g_snode */
/*                                                                          */
/* synopsis:                                                                */
/*     tp_opendir opens the given directory file, which is assumed to exist.*/
/*     It clears g_sfp, does a chdir to the given directory, and then       */
/*     clears the globals g_filename, g_dispname, and g_snode.  The         */
/*     g_curdir global is set to the given filename.  If the chdir fails,   */
/*     tp_opendir issues a fatal error.                                     */
/****************************************************************************/

static void tp_opendir (char *filename, int helperflag)
    {

#ifdef DEBUG
    debug ("tp_opendir:  filename = '%s'", filename);
    debug ("          helperflag = %s", helperflag ? "TRUE" : "FALSE");
#endif

    g_sfp = NULL;

    if (chdir (filename) == ERROR) /* change to the new directory */
	fatal ("tp_opendir:  cannot change to directory '%s'", filename);

    (void) strcpy (g_curdir, filename); /* remember name of current directory */

    s_free (g_filename);
    g_filename = NULL;

    s_free (g_dispname);
    g_dispname = NULL;

    g_snode = NULL;

    if (helperflag)
	openhelper (DIRHELPER, filename, NULL);
    }

/****************************************************************************/
/* opentext:  opens a text or delta file                                    */
/*                                                                          */
/* arguments:              char *filename - text or delta file to open      */
/*                         int helperflag - TRUE if should start helper     */
/*                         int formflag   - TRUE if should set new form     */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     g_readonly, g_curdir, g_fakefile, g_sfp,         */
/*                         g_snode, g_filename, g_errno, g_wp, g_dispname   */
/*                                                                          */
/* globals changed:        g_readonly, g_curdir, g_sfp, g_snode,            */
/*                         g_filename, g_errno, g_wp, g_dispname            */
/*                                                                          */
/* synopsis:                                                                */
/*     opentext opens the named text or structured file, optionally         */
/*     changing to a new helper and/or a new form.                          */
/*                                                                          */
/*     It begins by finding the directory containing the file.  If          */
/*     the directory is write-protected against the user, it sets           */
/*     the g_readonly flag TRUE.  It then does a chdir to that              */
/*     directory and sets the g_curdir global for future reference.         */
/*                                                                          */
/*     opentext then actually opens the file, setting the g_sfp global      */
/*     to point into either the file itself or the corresponding fake       */
/*     file.  g_snode is set to point at the record level of the file,      */
/*     and g_filename is set to the name of the file.                       */
/*                                                                          */
/*     If the input flag to change forms is set, opentext determines        */
/*     the appropriate form for the file and invokes Euseform to            */
/*     display the file with using that form.  If the input flag to         */
/*     change helpers is set, it invokes openhelper to start up the         */
/*     helper for the file.                                                 */
/****************************************************************************/

LOCAL void opentext (char *filename, int helperflag, int formflag)
    {
    char *formname;     /* form name for file    */
    int formret;        /* Euseform return value */
    char *dirname;      /* directory name        */
    char *oldname;      /* old g_filename        */
    struct stat dirbuf; /* realname stat buffer  */

#ifdef DEBUG
    debug ("opentext:  filename = '%s', helperflag = %d, formflag = %d",
	   filename, helperflag, formflag);
#endif

    dirname = trimpath (s_string (filename));

    if (dirname == NULL)
	fatal ("opentext:  dirname NULL for file '%s'", filename);


    if (stat (dirname, &dirbuf) == ERROR)
	fatal ("opentext:  cannot stat directory '%s'", dirname);

    /* make available info indicating that the directory is not writable
       so the error message indicating that the file cannot be opened
       is properly stated */

    if (! (modebits (dirname, &dirbuf) & WRITE))
	g_readonly = DIR_NOREAD;

    /* change to new directory */
    if (chdir (dirname) == ERROR) /* change to the directory of the file */
	fatal ("opentext:  cannot change to directory '%s'", dirname);

    (void) strcpy (g_curdir, dirname); /* remember name of current directory */
    s_free (dirname);

    /* open file */
    oldname = s_string (g_filename);
    s_free (g_filename);
    g_filename = s_string (filename);

    if (g_fakefile) /* if not a delta file, make it into one */
	g_sfp = openfake (filename);
    else
	{
	g_sfp = sf_open (filename, g_readonly ? "r" : "u");

	if (g_sfp == (SFILE *) ERROR) /* read-only file system fix */
	    {
	    g_sfp = fsf_open (filename, "r","");
	    if (! g_readonly) /* does it need to be 2 or something? */
		g_readonly = TRUE;
	    }
	}

    g_snode = (char *) sf_records (g_sfp);

    /* open form */
    formname = formprefix (filename);

    if (formflag)
	{
	formret = Euseform (formname); /* try to open file specific form */
	if (formret == ERROR)
	    if (g_errno == E_NOFILE)
		{
		s_free (formname);
		formname = defaultform ();
		formret = Euseform (formname); /* try default form */
		}
	if (formret == ERROR) /* if bad form file and/or bad default form */
	    {
	    g_filename = oldname;
	    fatal ("opentext:  bad form file '%s'", formname);
	    }

	/* NOTE:  Euseform sets g_wp to point at g_warray [1]  */
	s_free (w_filename (g_wp));
	g_wp->w__filename = s_string (g_filename);
	}
    s_free (oldname);
    s_free (g_dispname);
    g_dispname = NULL;

    /* start helper */
    if (helperflag)
	openhelper (formname, filename, NULL);

    s_free (formname);
    }

/****************************************************************************/
/* setfile:  sets up editing for a given file.                              */
/*     NOTE: this routine handles the setting of the g_spath,               */
/*     g_fakefile, g_readonly and g_zoomstack globals.  The tp_opendir and  */
/*     opentext routines are responsible for the g_filename, g_snode and    */
/*     g_sfp globals.  The checkfile routine sets up the g_newfake          */
/*     global flag which is the new value for the g_fakefile flag.          */
/*                                                                          */
/* arguments:              char *filename - file to be edited               */
/*                         int helperflag - TRUE if helper should be start- */
/*                                          ed                              */
/*                         int formflag   - TRUE if Euseform should be      */
/*                                          called                          */
/*                                                                          */
/* return value:           int - RET_OK if successful, else ERROR           */
/*                                                                          */
/* globals referenced:     g_filename, g_sfp, g_curdir, g_noindex, g_hooks, */
/*                         g_fakefile, g_newfake, g_spath, g_zoomstack,     */
/*                         g_readonly                                       */
/*                                                                          */
/* globals changed:        g_filename, g_fakefile, g_spath, g_zoomstack,    */
/*                         g_readonly                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     setfile sets up editing for the given file, optionally starting      */
/*     a new form or new helper, and sets various globals appropriately.    */
/*     If the given filename is the special poundfile name "#", it          */
/*     invokes opendelfile, sets g_filename to "#", and returns YES.        */
/*                                                                          */
/*     If there is currently a file open, flush the current window and      */
/*     the record array.  If the new filename is the same as g_filename,    */
/*     it is safe to close the file immediately.                            */
/*                                                                          */
/*     If the given file does not exist, invoke makefile to make it         */
/*     or, if the user changed their mind, return ERROR.  If it is a          */
/*     directory and the directory helper is disabled, invoke Eerror        */
/*     and then return ERROR.  If checkfile indicates the file is           */
/*     not one the editor can handle, return ERROR.  Only after all         */
/*     these tests are we sure we are going to the named file.              */
/*                                                                          */
/*     If the user has set a hook on the CHANGEFILE pseudo-key, invoke      */
/*     Hbefore to do whatever is desired.  Finally, close the file          */
/*     we are leaving.                                                      */
/*                                                                          */
/*     To actually move to the new file, set g_fakefile to g_newfake,       */
/*     discard the previous values of g_spath and g_zoomstack, reset        */
/*     the value of g_readonly, and invoke either tp_opendir or opentext    */
/*     to open the new directory or file, passing opentext the flags        */
/*     for changing the helper and form.  When opened, return RET_OK.       */
/****************************************************************************/

int setfile (char *filename, int helperflag, int formflag)
    {
    char *absname;          /* absolute path name                */
    int directory;          /* TRUE if file is directory         */
    struct stat statbuf;    /* stat call buffer   */
    int     makestatus;     /* result of makefile */

#ifdef DEBUG
    debug ("setfile: filename='%s', helperflag = %d, formflag = %d",
	   filename, helperflag, formflag);
#endif

    if ((filename == NULL) || (*filename == '\0'))
	return (ERROR);

    if (seq (filename, "#"))
	{
	opendelfile (); /* this recursively calls setfile on poundsign file */
	g_filename = s_string ("#");
	return (RET_OK);
	}
    if (g_sfp) /* if there already is a file open, sync it up */
	{
	flush_window ();
	flushrecord (TRUE);
	save_reclocs ();
	}
    absname = abspath (filename, g_curdir);

    if (seq (absname, g_filename)) /* safe to close if reopening file */
	closefile ();

    if (stat (absname, &statbuf) == ERROR) /* if file doesn't exist */
	{
	if ((makestatus = makefile (absname, helperflag, formflag)) != TRUE) /* cancel or re-enter */
	    {
	    s_free (absname);
	    return (makestatus == FALSE ? ERROR : TRUE);
	    }
	if (stat (absname, &statbuf) == ERROR) /* try again */
	    fatal ("setfile:  stat failure on file '%s' after makefile",
		   absname);
	}

    directory = statbuf.st_mode & S_IFDIR;

    if (g_noindex && directory)
	{
	Eerror (M_ENODIREDIT, "Cannot edit a directory.  Edit a text or structured file.");
	s_free (absname);
	return (ERROR);
	}

    if (! checkfile (absname, &statbuf))
	{
	s_free (absname); /* NOTE:  checkfile does error message */
	return (ERROR);
	}

    /* set off the CHANGEFILE hook only after we're sure we're switching
       to the new file                                                  */

    if (g_hooks [CHANGEFILE])
	Hbefore (CHANGEFILE, STRINGARG, filename, 0);

    closefile (); /* safe to close the current file now */

    g_fakefile = g_newfake;

    s_free (g_spath);
    g_spath = NULL;

    s_free ((char *) g_zoomstack);
    g_zoomstack = NULL;

    g_readonly = ! (modebits (absname, &statbuf) & WRITE);

    if (directory) /* if file is directory */
	tp_opendir (absname, helperflag);
    else
	opentext (absname, helperflag, formflag);

    s_free (absname);
    return (RET_OK);
    }

/****************************************************************************/
/* usefile:  change to a new file                                           */
/*                                                                          */
/* arguments:              char *name - file name to change to              */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_curdir, g_sysdir, g_filename,                  */
/*                         g_altstack, g_zoomstack, g_font                  */
/*                                                                          */
/* globals changed:        g_altstack, g_zoomstack, g_font                  */
/*                                                                          */
/* synopsis:                                                                */
/*     usefile begins by examining the given filename.  If it is a          */
/*     .index file, it issues an Eerror message and returns.                */
/*                                                                          */
/*     If the user is currently in the system help library, usefile         */
/*     invokes setfile to move to the named file immediately, thus          */
/*     preventing the help library from becoming the alternate file.        */
/*     If the setfile call fails, usefile leaves the user in the help       */
/*     library and returns.                                                 */
/*                                                                          */
/*     If the user is editing any other file, save the current zoomstack    */
/*     in g_altstack, clear g_zoomstack, and compress the current form.     */
/*     Invoke setfile to move to the requested file.  If it succeeds,       */
/*     set g_font to zero to get into normal font and then return.          */
/*                                                                          */
/*     If the call to setfile fails, restore the original altstack and      */
/*     zoomstack if they existed, expand the form again, and invoke         */
/*     getall to restore the window caches to their original state.         */
/****************************************************************************/

void usefile (char *name)
    {
    char *filename;             /* full file name              */
    char *string;               /* converted string            */
    char *form;                 /* forms prefix                */
    extern char g_helpstate;    /* HELP key context            */
    POINTER *altstack;          /* used to save the g_altstack */
    POINTER *stack;             /* to keep setfil from breaking*/
    WSTRUCT *wp;                /* used to discard reclocs ... */
    int      i;                 /*  ...  after setfile failure */

    flush_window ();

    if (seq (name, "#"))
	filename = s_string (name);
    else
	{
	string = Efixvars (name);
	filename = abspath (string, g_curdir);

#ifdef DEBUG
	debug ("usefile:  name = '%s', string = '%s', filename = '%s'",
	    name, string, filename);
#endif

	s_free (string);

	/* make sure its not a .index file */
	form = formprefix (filename);

	if (seq (form, DIRHELPER))
	    {
	    Eerror (M_EINDEXFILE, "Cannot edit a .%s file.", form);
	    s_free (form);
	    s_free (filename);
	    return;
	    }
	s_free (form);
	}
    Emessage (M_EUSEFILE, "Changing to file %s.", filename);

    switch (g_helpstate)
    {
    case INHELPMENU:    /* move g_helpstate to INHELPFILE and go to file */
	g_helpstate ++;
	pushstate (TRUE);
	stack = g_zoomstack;
	g_zoomstack = NULL;
	setfile (filename, TRUE, TRUE);
	g_zoomstack = stack;
	return;

    case INHELPFILE:    /* return to context prior to going to help file */
	cleanstack();
	popstate (TRUE);
	g_helpstate = NOTHELPING;
	break;           /* note user must have specified file to get here */

    case IN2NDHMENU:    /* abandon this helpfile context and go to new one */
	g_helpstate --;         /* new g_helpstate is INHELPFILE */
	cleanstack();
	stack = g_zoomstack;
	g_zoomstack = NULL;
	setfile (filename, TRUE, TRUE);
	g_zoomstack = stack;
	return;

    /* case NOTHELPING falls through into the code below */
    }
    if (g_filename) /* if currently editing a file */
	{
	altstack = g_altstack;    /* save the alternate stack     */
	pushstate (TRUE);          /* save current state on stack  */
	g_altstack = g_zoomstack; /* move zoom stack to alt stack */
	}
    else
	altstack = NULL;

    g_zoomstack = NULL;

    if (setfile (filename, TRUE, TRUE) == RET_OK)
	{
	s_free (filename);
	s_free ((char *) altstack); /* free old alternate stack */
	return;
	}

    /* setfile failed.  Restore the original zoomstack and altstack, and
       discard the reclocs and do digin and getall again to get back to
       where we started.  */

    if (g_filename) /* if there is an alternate file */
	{
	g_zoomstack = g_altstack; /* reverse the stack switch & pushstate */
	s_free (g_zoomstack [0]); /* free ZOOM frame made by pushstate */
	g_zoomstack = (POINTER *) s_delete ((char *)g_zoomstack, 0, 1);
	g_altstack = altstack;
	}

    for (i = 1; i < obj_count (g_warray); i++)
	{
	wp = &g_warray [i];
	s_free ((char *) w_reclocs (wp));
	wp->w__reclocs = NULL;
	}
    s_free (filename);

    /* This fills the caches & makes sure that g_wp is reasonable       */
    digin ();
    getall ();
    }

