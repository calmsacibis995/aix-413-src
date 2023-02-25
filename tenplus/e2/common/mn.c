static char sccsid[] = "@(#)81  1.6  src/tenplus/e2/common/mn.c, tenplus, tenplus41J, 9523C_all 6/8/95 15:08:44";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	Ermfile, allocerr, closeall, copylist, coredump,
 *		delfiles, e2exit, fatal, Efixvars, fixvlist, hangup,
 *		interupt, logfatal, main, make_vector, mktimefile,
 *		quit, suspend, winchsz, readprofile, sigcatch,
 *		watchfiles, leak_cleanup
 * 
 * ORIGINS:  9, 10, 27
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *                      Copyrighted as an unpublished work.                 
 *                         INTERACTIVE TEN/PLUS System                      
 *                             TEN/PLUS INed Editor                         
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, 1988          
 *                             All rights reserved.                         
 *                                                                          
 *   RESTRICTED RIGHTS                                                      
 *   These programs are supplied under a license.  They may be used and/or  
 *   copied only as permitted under such license agreement.  Any copy must  
 *   contain the above notice and this restricted rights notice.  Use,      
 *   copying, and/or disclosure of the programs is strictly prohibited      
 *   unless otherwise provided in the license agreement.                    
 *                                                                          
 *   TEN/PLUS is a registered trademark of INTERACTIVE Systems Corporation  
 *	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988
 */

static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988\n"
	"(C) Copyright International Business Machines Corp. 1990, 1992";

#include "def.h"

#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <locale.h>

#include "editorprf.h"

int	MultibyteCodeSet;	/* global code set flag */

#include "INeditor_msg.h"       /* Editor catalogue desriptor header file */

/* catalog file descriptor */
nl_catd g_e2catd;


static char *version = "TEN/PLUS INed Editor Program, Version 3.2";

static char *invokename;         /* argv[0] */
static int term_mode = FALSE;    /* flag for terminal mode (-t flag) */

#define WATCHSECS     (60L)      /* watchfile frequency (in secs) */
#define DEFAULT_FILE  "edefault"
#define STATE_FILE    ".estate"

#define VECTORLEN     (30* MB_LEN_MAX)
#define CUR_DIR       "."
#define AIXTERM_CMD     "aixterm"
/* N.B. This was originally xterm, but a problem occured due to a change
   to xterm which caused INed to believe it was a vt100 */

static long watchtime;             /* time of last watch check          */
static char timefile [PATH_MAX];   /* name of .e2time file              */
static dorestart;                  /* tells fatal to restart the editor */
static POINTER *g_tmpfiles;        /* tmp files to remove on exit       */
static POINTER *tasknames = NULL;

extern char g_helpstate;           /* Helper state should be 0 at start */

static int taskbits [] =
    {
    USEPOPBOX,
    USESCREEN,
    USEQUIT,
    USEHELPER,
    USEFORM,
    };

void    closeall (void);
POINTER *copylist (POINTER *);
void    coredump (void);
void    delfiles (void);
POINTER *fixvlist (POINTER *);
void    hangup (void);
void    interupt (void);
void    mktimefile (void);
void    quit (void);

void    sigcatch (int);
#ifdef LEAK_FINDING
static void leak_cleanup (void);
#endif


/****************************************************************************/
/* Ermfile:  helper callable routine to delete a file on exit               */
/*                                                                          */
/* arguments:          char *name - name of file to be deleted              */
/*                                                                          */
/* return value:       void                                                 */
/*                                                                          */
/* globals referenced: g_tmpfiles                                           */
/*                                                                          */
/* globals changed:    g_tmpfiles                                           */
/*                                                                          */
/* synopsis:                                                                */
/*     If the name is already included in g_tmpfiles, no effect.            */
/*     Otherwise g_tmpfiles is changed to include the name.                 */
/****************************************************************************/

void Ermfile (char *name)
    {
#ifdef DEBUG
    debug ("Ermfile:  name = '%s'", name);
#endif

    if (g_tmpfiles == NULL)
	{
	g_tmpfiles = s_pointer (s_string (name));
	return;
	}
    if ( !inlist (g_tmpfiles, name))
	g_tmpfiles = s_append (g_tmpfiles, s_string (name));
    }

/****************************************************************************/
/* allocerr:  allocation failure handler                                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_warray, g_altstack, g_dispname                 */
/*                         g_formcache, g_formname, g_formpath, g_helpfiles */
/*                         g_hlprpath, g_lastarg, g_sysdir, g_zoomstack,    */
/*                                                                          */
/* globals changed:        g_warray, g_altstack, g_dispname                 */
/*                         g_formcache, g_formname, g_formpath, g_helpfiles */
/*                         g_hlprpath, g_lastarg, g_sysdir, g_zoomstack,    */
/*                                                                          */
/* synopsis:                                                                */
/*     allocerr is called when an attempt to allocate a new structure fails */
/*     because no more space is available, typically while running a        */
/*     leaking program.  It performs an s_free on all relevant editor       */
/*     globals, type error messages to the user, and invokes the fatal      */
/*     routine to attempt a restart of the editor.                          */
/****************************************************************************/

void allocerr (void)
    {
    s_free ((char *) w_cache (&g_warray [0])); /* free up a few things */

    tmclose ();
    {
    char *message=NULL;


    message = Egetmessage(M_E_NOROOM,"The editor has run out of space.#1",FALSE);

    (void) printf ("%s\n", message);
    s_free (message);
    }
    fatal ("Memory allocation failure:  ran out of room for data");
    }

/****************************************************************************/
/* closeall:  closes current file if necessary                              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_diskfull                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If global flag g_diskfull is set, no effect.                         */
/*     Otherwise invokes closefile to close the file being edited and       */
/*     savefakes to write out the new versions of non-structured files.     */
/****************************************************************************/

void closeall (void)
    {
    if (g_diskfull) /* if disk is full, don't try to save files */
	return;

    closefile (); /* close file */
    savefakes ();
    }

/****************************************************************************/
/* copylist:  copies a desc/name for the USE menu                           */
/* Also set the flag bits of object if "type" field is present to one of    */
/* USEFILE, USEPOPBOX, USESCREEN, USEHELPER, USEFORM or USEQUIT.            */
/* AND's in USESYNC and USESAVE bits if necessary.                          */
/*                                                                          */
/* arguments:              POINTER *oldlist - to be copied                  */
/*                                                                          */
/* return value:           POINTER * - the copied version                   */
/*                                                                          */
/* globals referenced:     tasknames, taskbits                              */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     copylist copies a structured allocation containing name, type, and   */
/*     desc subfields, such as those records in the .eprofile file holding  */
/*     information about the user's menu options and help options.  The     */
/*     name and desc subfields are copied from the original list into the   */
/*     new, and the type subfield is used to set the appropriate flags of   */
/*     the returned allocation to indicate whether each list element is     */
/*     a file, a popbox, a form, or whatever; and whether it is necessary   */
/*     to perform a SAVE or SYNC operation when invoking that menu choice.  */
/****************************************************************************/

POINTER *copylist (POINTER *oldlist)
    {
    int i,j,k;         /* loop counter             */
    int flagbits;      /* flag bits for object     */

    POINTER *newlist;  /* new list being generated */
    char *desc;        /* olditem desc field       */
    char *name;        /* olditem name field       */
    char *type;        /* olditem type field       */
    char *newname;     /* item name                */
    char *cp;          /* used to find save & sync */

#ifdef DEBUG
    debug ("copylist:  oldlist = 0%o", oldlist);
    objprint ("copylist (oldlist)", oldlist);
#endif

    newlist = (POINTER *) s_alloc (obj_count (oldlist), T_POINTER,
				   obj_name (oldlist));

    for (i = k = 0; i < obj_count (oldlist); i++)
	{
	if (oldlist [i] == NULL)
	    continue;

	desc = s_findnode ((POINTER *)oldlist [i], "desc");
	name = s_findnode ((POINTER *)oldlist [i], "name");
	type = s_findnode ((POINTER *)oldlist [i], "type");

	if (desc == (char *) ERROR || type == (char *)ERROR ||
	  desc == NULL || type == NULL ||
	  *desc == '\0' || *type == '\0')
	    continue;

	newlist [k] = s_string (desc);

	if (name == (char *) ERROR || name == NULL) /* get name for entry */
	    newname = s_string ("");
	else
	    newname = s_string (name);

	s_newname ((char *) newlist [k], newname);
	s_free (newname);
	flagbits = USEFILE; /* flag bits default to USEFILE */

	if (tasknames == NULL)
	    tasknames = s2pointer("popbox\nscreen\nexit\nhelper\nform\ngly");

	for (j = 0; j < obj_count (tasknames); j++)
	    if (seq (type, tasknames [j]))
		{
		flagbits = taskbits [j];
		break;
		}

	cp = s_findnode ((POINTER *)oldlist [i], "sync");

	if ((cp != (char *) ERROR) && (*cp))
	    flagbits |= USESYNC;

	cp = s_findnode ((POINTER *)oldlist [i], "save");

	if ((cp != (char *) ERROR) && (*cp))
	    flagbits |= USESAVE;

	obj_setflag (newlist [k], flagbits);
	k++;

#ifdef DEBUG
    objprint ("copylist (oldlist)", oldlist);
    objprint ("copylist (newlist)", newlist);
#endif
	}

    if (k != i) /* make correct size */
	newlist = (POINTER *)s_realloc ((char *)newlist, k);

    return (newlist);
    }

/****************************************************************************/
/* coredump: called when the editor aborts.                                 */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     coredump causes a core dump and terminates the editor process.       */
/*     On vms, this is simply an exit (-1).  On the vax, it is also         */
/*     necessary to trap SIGIOT to prevent an infinite abort loop.          */
/****************************************************************************/

void coredump (void)
{
    int i;

    /* On the vax, the abort routine causes an IOT.  To prevent looping
       the signal must be defaulted here.                               */

    for (i = 0; i < NSIG; i++)
	(void) signal (i, SIG_DFL);
    abort ();
}

/****************************************************************************/
/* delfiles:  routine to delete files listed in g_tmpfiles                  */
/* Should be called before exit.                                            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_tmpfiles                                       */
/*                                                                          */
/* globals changed:        g_tmpfiles                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     delfiles deletes each of the files listed in g_tmpfiles in           */
/*     preparation for leaving the editor.                                  */
/****************************************************************************/

void delfiles (void)
    {
    int i;

    if (g_tmpfiles == NULL)
	return;

    for (i = 0; i < obj_count (g_tmpfiles); i++)
	{
	Remove (g_tmpfiles [i]);
	}
    }

/****************************************************************************/
/* e2exit:  handles the EXIT command                                        */
/*                                                                          */
/* arguments:              int   argtype  - argument type code              */
/*                         char *strvalue - value of argument as string     */
/*                         int   numvalue - value of argument as number     */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     e2exit is called for a normal editor exit when the user hits the     */
/*     EXIT key.  It calls various subsidiary routines to close the file,   */
/*     clean up temporary files, reset the terminal, do the appropriate     */
/*     timestamp accounting, and unfake any fake (i.e. non-structured)      */
/*     files that are open.                                                 */
/*                                                                          */
/*     Currently all possible argtypes are equivalent to the NOARGS exit,   */
/*     except for the STRINGARG case wherein the string value is "q".       */
/****************************************************************************/
/*ARGSUSED*/
void e2exit (int argtype, char *strvalue, int numvalue)
    {
#ifdef DEBUG
    debug ("e2exit called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    mktimefile ();
    closefile (); /* close file */

    /* on QUIT sequence, don't do savefakes     */
    if ( !(argtype == STRINGARG && seq (strvalue, "q")))
	savefakes (); /* save all the fake files */
    else
	bagfakes ();


    killbox (TRUE);/* wait for popup box display */
    tmclose ();   /* reset terminal */
    delfiles ();  /* delete tmp files from helpers */

#ifdef LEAK_FINDING
    leak_cleanup ();
#endif
    catclose(g_e2catd);
    exit (0);
    }

/****************************************************************************/
/* fatal:  print message, close terminal and do error exit                  */
/*                                                                          */
/* arguments:              char *format, *arg1, *arg2, *arg3, *arg4         */
/*                         (printf style variable number of arguments)      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_debugfp, g_diskfull, g_helper,                 */
/*                         g_curdir, g_filename, g_dfltfile,                */
/*                         dorestart                                        */
/*                                                                          */
/* globals changed:        g_diskfull                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     fatal begins by doing printf-style output of its arguments           */
/*     (with "fatal:  " prepended) using debug.  It sets g_diskfull         */
/*     to prevent lower-level procedures from trying to save files,         */
/*     closes up shop much like a normal exit, decides whether or           */
/*     not to do a core dump, and finally decides either to try to          */
/*     restart the editor or give up with exit (-1).  Care is taken         */
/*     to prevent unnecessary work on cascading calls to fatal,             */
/*     and to prevent attempting a restart if the main procedure            */
/*     never got started in the first place.                                */
/****************************************************************************/

void fatal (char *format1, ...)
    {
    char *filename;          /* editor restart file    */
    static firstfatal;       /* fatal recursion flag   */
    char message [100];
    int nargs;
    char *arg1, *arg2, *arg3, *arg4;
    char *args[10];

    extern FILE *g_debugfp;
    extern FILE *delfp;
    extern FILE *putfp;
    extern int g_St_fd;

    va_list ap;
    va_start(ap, format1);
    arg1 = va_arg (ap, char *);
    arg2 = va_arg (ap, char *);
    arg3 = va_arg (ap, char *);
    arg4 = va_arg (ap, char *);
    va_end(ap);

    (void) sprintf (message, format1, arg1, arg2, arg3, arg4);
    debug ("fatal:  %s", message);

    if ( !firstfatal) /* if first time fatal has been called */
	{
	firstfatal = TRUE; /* set recursion flag */

	logfatal (message, FALSE);
	tmclose ();
	closeall ();
	(void) fputs (message, stderr);
	(void) fputc ('\n', stderr);
        delfiles (); /* delete tmp files from helpers */
	}

#ifdef COREDUMP
    coredump ();
#else
    if (g_debugfp) /* if user has given the "-" switch */
	coredump ();
#endif

    g_diskfull = TRUE; /* don't save files on abort */
    killhelpers ();

    if (seq (g_helper, DIRHELPER))
	filename = g_curdir;
    else
	filename = (g_filename && *g_filename) ? g_filename : g_dfltfile;

    /* if we didn't get through main to action () */
    /* or if quit received, don't try to restart  */

    if ( !dorestart)
    {
	catclose(g_e2catd);
	exit (-1);
    }

    (void) fprintf (stderr, "Restarting editor on file \"%s\"...\n",
	filename);

    tmclose ();
    /* clean up so we don't leak file descriptors when restarting */

    if (g_St_fd)
	(void)close (g_St_fd);
    if (g_debugfp)
	(void)fclose (g_debugfp);
    if (delfp)
       (void)fclose (delfp);
    if (putfp)
	(void)fclose (putfp);

    nargs = 0;
    args[nargs++] = invokename;
    if (term_mode)
	args[nargs++] = "-t";
    args[nargs++] = filename;
    args[nargs++] = (char *) NULL;
    (void)execvp (invokename, args);

    coredump ();
    }

/****************************************************************************/
/* Efixvars:  fix string with $HOME, $SYS or env variables                  */
/* Returns an allocated object.                                             */
/*                                                                          */
/* arguments:              char *string - to be fixed                       */
/*                                                                          */
/* return value:           (structured) char * - with variables resolved    */
/*                                                                          */
/* globals referenced:     g_homedir, g_sysdir, g_filename, g_helper,       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Efixvars walks down the input string, copying characters into the    */
/*     output string.  Wherever it encounters a $ character, the following  */
/*     group of characters up to the next non-alphanumeric or underscore    */
/*     is taken to be the name of a variable whose value is to be           */
/*     substituted into the string.  The variables $HOME, $SYS, $FILE,      */
/*     $ALTFILE, $HELPER, and $FORM are resolved by referring to editor     */
/*     globals; other variables are resolved via a call to getenv.          */
/*     The result is returned as a structured allocation.                   */
/****************************************************************************/

#include <pwd.h>

char *Efixvars (char *string)
    {
    char *buffp;     /* pointer to buffer   */
    char *varp;      /* pointer to variable */
    char *value;     /* getenv value        */
    char *buffer;
    char *variable;
    char type;

    int  lstring;
    int  tildok = TRUE;

#ifdef DEBUG
    debug ("Efixvars:  string = '%s', g_homedir = '%s', g_sysdir = '%s'",
	   string, g_homedir, g_sysdir);
#endif

    lstring = strlen (string);          /* not safe to use obj_count here */
    buffer = s_nzalloc (lstring, T_CHAR, "");
    buffp = buffer;

    while (*string)
	{
	if (!(*string == '$' || (tildok && *string == '~')))
	    {
	    tildok = (*string == ' ');
            mbcpy(&buffp, &string);

	    continue;
            }

	type = *string;
	string++; /* increment pointer to char after the $ */

	variable = s_nzalloc (lstring, T_CHAR, "");
	varp = variable; /* get variable name from string (into 'variable') */

	while (ismbalnum(string) || (*string == '_'))
            mbcpy(&varp, &string);

	*varp = '\0';

	if (type == '$')
	    {
	    if (seq (variable, "HOME")) /* evaluate variable */
		value = g_homedir;

	    else if (seq (variable, "SYS"))
		value = g_sysdir;

	    else if (seq (variable, "FILE"))
		value = g_filename;

	    else if (seq (variable, "ALTFILE"))
		value = altname ();

	    else if (seq (variable, "HELPER"))
		value = g_helper ? g_helper : "NO HELPER";

	    else if (seq (variable, "FORM"))
		value = g_formname ? g_formname : "NO FORM";

	    else if (seq (variable, "ABSFORM"))
		value = w_zoom (&g_warray [0]);     /* always defined */
            else if (seq(variable, "NLSPATH"))
                value = nls_path(string);
	    else
		value = getenv (variable);

	    }
	else
	    {
	    
	    struct passwd *pw;
	    char c;
	    char *cp;
	    char name[PATH_MAX];
            char *namep;

	    c = *variable;
	    if (c == '\0') 
		value = g_homedir;
	    else
		{
		for (namep = name, cp = variable; ismbalnum (cp);  
                     mbcpy(&namep, &cp) ); 
		*namep = '\0';

		pw = getpwnam (name);
		if (pw)
		    value = pw->pw_dir;
		else
		    value = NULL;
		} 
	    }

	if (value == NULL) /* if cannot get value, put in $variable */
	    {
	    *buffp++ = type;
	    value = variable;
	    }
	else
	    {
	    int growth = strlen (value) - (varp - variable) - 1;
	    int offset = buffp - buffer;

	    buffer = s_realloc (buffer, obj_count (buffer) + growth);
	    buffp = buffer + offset;
	    }

	(void) strcpy (buffp, value);
	buffp += strlen (buffp);
	s_free (variable);
	} /* while */
    *buffp = '\0'; /* terminate buffer string */

#ifdef DEBUG
    debug ("Efixvars:  buffer = '%s'", buffer);
#endif

    return (buffer);
    }

/****************************************************************************/
/* fixvlist:  makes a copy of a POINTER array, via Efixvars                 */
/* Handles list being a character array                                     */
/*                                                                          */
/* arguments:          POINTER *list - list of strings to fix               */
/*                                     (or, one string to fix)              */
/*                                                                          */
/* return value:       POINTER *fixedlist - each member fixed               */
/*                         (or, one-element POINTER array pointing to       */
/*                         the one fixed string)                            */
/*                                                                          */
/* globals referenced: none                                                 */
/*                                                                          */
/* globals changed:    none                                                 */
/*                                                                          */
/* synopsis:                                                                */
/*     fixvlist simply performs a call to Efixvars on each element of       */
/*     the input list.  If the input turns out to be a string rather        */
/*     than a pointer array, fixvlist fixes that string and returns         */
/*     a one-element pointer array pointing to the fixed string.            */
/****************************************************************************/

POINTER *fixvlist (POINTER *list)
    {
    POINTER *newlist;
    int i;
    int count;

if (list == NULL)
    return ((POINTER *) s_alloc (0, T_POINTER, NULL));

#ifdef DEBUG
    debug ("fixvlist:  list = 0%o, type = %d", list, obj_type (list));
#endif

    if (obj_type (list) == T_CHAR)
	return (s_pointer (Efixvars ((char *) list)));

#ifdef CAREFUL
    if (obj_type (list) != T_POINTER)
	fatal ("fixvlist:  list type (%d) not T_POINTER", obj_type (list));
#endif

    count = obj_count (list);
    newlist = (POINTER *) s_alloc (count, T_POINTER, NULL);

    for (i = 0; i < count; i++)
	newlist [i] = Efixvars (list [i]);

    return (newlist);
    }

/****************************************************************************/
/* hangup:  special signal catcher for hangup                               */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Cleans up editor state and does exit (0)                             */
/****************************************************************************/
void hangup (void)
    {
#ifdef DEBUG
    debug ("hangup called");
#endif
    closefile (); /* close file */
    savefakes ();
    tmclose ();
    delfiles (); /* delete tmp files from helpers */
    catclose(g_e2catd);
    exit (0);
    }

/****************************************************************************/
/* interupt:  interupt (break) catcher                                      */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_intflag                                        */
/*                                                                          */
/* globals changed:        g_intflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     Sets the global g_intflag TRUE on receipt of a break signal          */
/****************************************************************************/

void interupt (void)
    {


#ifdef DEBUG
    debug ("interupt called");
#endif

    g_intflag = TRUE;
    (void) signal (SIGINT, (void (*)(int))interupt);
    }

/****************************************************************************/
/* logfatal:  adds entry to fatal log file                                  */
/*                                                                          */
/* arguments:              char *message - fatal error message              */
/*                         int   flag    - TRUE if helper error,            */
/*                                         FALSE or not provided otherwise  */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_sysdir, g_filename, g_spath                    */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     logfatal adds a record to the file /var/INed.FATAL.LOG               */
/*     The record indicates when and to whom the error happened,            */
/*     what the error message was, and what the user was editing.           */
/*     INed.FATAL.LOG is an ordinary ascii file, not structured.            */
/****************************************************************************/

void logfatal (char *message, int flag)
    {
    FILE *fp;                     /* FATALFILE file pointer    */
    long curtime;                 /* current time              */
    char fatalfile [PATH_MAX] = "/var/INed.FATAL.LOG";
    char date_buffer[100];

#ifdef DEBUG
    debug ("logfatal:  message = '%s', flag = %d", message, flag);
#endif

    fp = fopen (fatalfile, "a");

    if (fp == NULL) /* if fatalfile can't be opened */
	return;

    curtime = time ((long *) 0);
    strftime(date_buffer,100,"%c", localtime(&curtime));
    (void) fprintf (fp, "user %d had a fatal%s error at time %s\n",

    getuid (), flag ? " HELPER" : "", date_buffer);

    (void) fprintf (fp, "%s\n", message);
    (void) fprintf (fp, "filename = '%s', path = '%s'\n",
       g_filename ? g_filename : "(null)" ,
       g_spath ? g_spath : "(null)");
    (void) fprintf (fp, "-------------------------------\n\n");
    (void) fclose (fp);
    (void) chmod (fatalfile, 0666); /* make INed.FATAL.LOG writable by all */
    }

/****************************************************************************/
/* main:  top level routine of the editor                                   */
/*                                                                          */
/* arguments:              int    argc                                      */
/*                         char **argv                                      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:                                                      */
/*                    g_allocfcn      allocation failure handler            */
/*                    g_curdir        current directory                     */
/*                    g_debugfp       pointer to debugging output file      */
/*                    g_dfltfile      pointer to default file to edit       */
/*                    g_homedir       value of $HOME - user's directory     */
/*                    g_noindex       whether there is no .index file       */
/*                    g_objflag       whether to do objprint ("+" arg)      */
/*                    g_searchstr     initial search string                 */
/*                    g_warray        pointer to array of windows           */
/*                    g_wp            pointer to current window             */
/*                    dorestart       whether successfully reached action() */
/*                    timefile        name of editor state file             */
/*                    watchtime       time of last watch check              */
/*                                                                          */
/* globals changed:                                                         */
/*                    g_allocfcn      allocation failure handler            */
/*                    g_curdir        current directory                     */
/*                    g_debugfp       pointer to debugging output file      */
/*                    g_dfltfile      pointer to default file to edit       */
/*                    g_homedir       value of $HOME - user's directory     */
/*                    g_noindex       whether there is no .index file       */
/*                    g_objflag       whether to do objprint ("+" arg)      */
/*                    g_searchstr     initial search string                 */
/*                    g_sysdir        value of $SYS variable                */
/*                    g_warray        pointer to array of windows           */
/*                    g_wp            pointer to current window             */
/*                    dorestart       whether successfully reached action() */
/*                    timefile        name of editor state file             */
/*                    watchtime       time of last watch check              */
/*                    MultibyteCodeSet                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     This is the main routine for the editor, invoked directly            */
/*     from the shell by the user.  Essentially, it parses the input        */
/*     arguments, sets up the signal catchers needed by the editor,         */
/*     initializes a host of variables, and then calls the action ()        */
/*     routine.  If anything goes wrong before invoking action, the         */
/*     global dorestart is not yet set and the error processing             */
/*     will not attempt a restart.                                          */
/*                                                                          */
/*     The input argument syntax is:                                        */
/*                                                                          */
/*         All arguments are optional and are intended to default gracefull */
/*                                                                          */
/*	   The -d and -D flags cause debug output to be written to the file */
/*	   "editor.trc".  -D specifies that object descriptions             */
/*	   should be dumped as well.					    */
/*                                                                          */
/*         The remaining arguments are, in order,                           */
/*                                                                          */
/*             the name of the file to edit (g_filename)                    */
/*             the starting line index                                      */
/*             the starting column index                                    */
/*             the starting search string                                   */
/****************************************************************************/

 int main (int argc, char **argv)
    {
    WSTRUCT *wp;                /* used for fake window    */
    FILE *fp;                   /* used to read timefile   */

    int i;                      /* used for signals        */
    int line;                   /* starting cursor line    */
    int column;                 /* starting cursor column  */
    int index1;                 /* index counter           */
    int InX;                    /* tells if in Xwindows    */
    int parm;                   /* command-line parameter */

    char *searchstr;            /* search string           */
    char *profile;              /* name of profile file    */
    char *exec_args[VECTORLEN]; /* vector of arguments     */
    char *cp;                   /* used to avoid a leak    */
    char *sysprofile;		/* system default profile  */
    char filename [PATH_MAX];   /* file name to edit       */
    char buffer [PATH_MAX];     /* fgetline buffer         */
    char *tmp;                  /* holds HOME temporarily  */
    struct stat statbuf;        /* stat buffer for .e2time */

    extern char *g_searchstr;   /* pointer to search stg   */
    extern int gmode;           /* output mode NORMAL, UNDERLINED, GRAPHIC */
    extern int optind;		/* used by getopts() */
    extern char *optarg;	/* used by getopts() */

    gmode = NORMAL;             /* initialize output mode       */
    InX = FALSE;                /* initialize in Xwindows */


    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    /* open the message catalog */
    g_e2catd = catopen (MF_INEDITOR, NL_CAT_LOCALE); 
    (void) printf ("%s\n%s\n", version, copyright + 4);
    invokename = argv[0];

    /* determine whether the local codeset is single or multibyte. Single
       byte codesets WORDWRAP by default and multibyte codesets CHARWRAP
       by default */

    if (MB_CUR_MAX ==1)
	{
	MultibyteCodeSet=0;
	g_wmodeflag = WORDWRAP;
	}
    else
	{
	MultibyteCodeSet=1;
	g_wmodeflag = CHARWRAP;
	}
   
/* This flag will be present only if aixterm has been invoked.  It is put 
   into the argument list in the procedure make_vector.
   Thus if found, the variable term_mode gets set. 
   We have to find this -t flag early, to know whether or not to fork
   off a child process. */
    for ( index1 = 1; index1 < argc; index1++) {
        if ( strcmp( argv[index1], "-t" ) == 0 ) {
           term_mode = TRUE;
           break;
	 }
      }

    /* set the g_sysdir variable */
    g_sysdir = getenv ("SYS");
    if ((g_sysdir == NULL) || (*g_sysdir == '\0'))
	{
	g_sysdir = SYSTEM_DIR;
	}

    /* set the g_def_formpath variable */
    g_def_formpath = s_pointer(Efixvars("$SYS/forms"));

/*  In the case the user is in Xwindows and term_mode has not been set,
    make_vector will parse the argument list and set up the appropriate
    argument vector.  A child process is then invoked to create a new 
    process which runs aixterm.   The user will experience a new window which
    will be separate from the initial starting window.  Editing activities
    for the user will take place as usual.  */

     if (getenv("WINDOWID") != NULL)
         InX = TRUE;
     if (InX && !term_mode) {
        make_vector( argc, argv, exec_args );
        (void)execvp( AIXTERM_CMD, exec_args );
        perror( invokename );
        fatal("Could Not Execute AIXTERM ");
        catclose(g_e2catd);
        exit(0);     
      }

    g_debugfp = 0;
    g_objflag = FALSE;

    if (getcwd (g_curdir, PATH_MAX) ==  NULL)
	fatal ("main:  cannot get current directory");

    g_homedir = s_string ((tmp = getenv ("HOME")) ? tmp : CUR_DIR);
    (void) sprintf (timefile, "%s%c%s", g_homedir, DIR_SEPARATOR, STATE_FILE);
    /* get time of last exit */

    if (stat (timefile, &statbuf) != ERROR)
	watchtime = statbuf.st_mtime;
    else
	watchtime = 0L;

    while ((parm = getopt (argc, argv, "dDt")) != EOF)
	{
	switch (parm)
	    {
	    case 'D':	/* Debugging, including object dumps */
	        g_objflag = TRUE;
	    case 'd':	/* debugging */
		g_debugfp = fopen ("editor.trc","w");
		(void) chmod ("editor.trc",0666);

		if (g_debugfp == NULL)
		    fatal("cannot open editor.trc");
		break;

	    case 't':	/* work in terminal mode; this was handled */
			/* above. */
		break;
	    }
	}

    /* Parse the remaining arguments as: filename, starting line,
       starting column, string to search for. */

    line = -1;
    column = 0;
    searchstr = (char *) NULL;

    if (argc > optind)
	{
	(void) strcpy (filename, argv [optind++]);
	if (argc > optind)
	    line = atoi (argv [optind++]) - 1;
	if (argc > optind)
	    column = atoi (argv [optind++]) - 1;
	if (argc > optind)
	    searchstr = argv [optind++];
	}
    else
	{
	fp = fopen (timefile, "r");
	*filename = '\0';

	if (fp != NULL)
	    {
	    (void)fgetline (fp, filename, PATH_MAX);
	    (void)fgetline (fp, buffer, PATH_MAX);
	    line = atoi (buffer);
	    (void)fgetline (fp, buffer, PATH_MAX);
	    column = atoi (buffer);
	    (void)fclose (fp);
	    }
	}

    /* catch unexpected signals and do a fatal call */
    for (i = 1; i < SIGMAX; i++) /* catch all others */
        {

#ifdef  SIG11_DEBUG     /* handy for catching wild pointers  */
	if (i != 11 || !g_debugfp)
#endif
	    {
	        /* only catch the signal if it wasn't already set to */
		/* be ignored */
	        if (signal (i, SIG_IGN) != SIG_IGN)
		    (void)signal (i, sigcatch);
	    }
	}

    if (signal (SIGHUP, SIG_IGN) != SIG_IGN)
        (void) signal (SIGHUP, (void(*)(int))hangup);     /* catcher for hangup signal    */
    if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
        (void) signal (SIGTERM, (void(*)(int))hangup);    /* clean quit from SIGTERM      */
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
        (void) signal (SIGQUIT, (void(*)(int))quit);      /* catcher for control/|        */
    (void) signal (SIGPIPE, SIG_IGN);   /* ignore pipe problems          */
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
        (void) signal (SIGINT, (void (*)(int))interupt);   /* catch interupt signal (BREAK)*/
    if (signal (SIGTSTP, SIG_IGN) != SIG_IGN)
        (void) signal (SIGTSTP, (void(*)(int))suspend);   /* catcher for control Z        */
    if (signal (SIGCONT, SIG_IGN) != SIG_IGN)
        (void) signal (SIGCONT, SIG_DFL);   /* default for the sig continue */
    (void) signal (SIGCHLD, SIG_DFL);   /* ignore dead children 	 */
    if (signal (SIGWINCH, SIG_IGN) != SIG_IGN)
        (void) signal (SIGWINCH, (void(*)(int))winchsz);  /* catch window-size-change sig */

    wn_init ();                 /* initialize the window database type */
    g_allocfcn = allocerr;      /* allocation failure handler          */

    /* full disk error handler  */
    g_diskfcn = (int(*)(char *, char *))Ediskfull;

    zoominit ();                /* initialize the T_ZOOMINFO object type */
    i_addtype ();               /* initialize the T_ATTR object type     */
    openput ();                 /* open put file */

    /* read  user's profile if there is one, else the system profile */
    profile = Efixvars (USER_PROFILE);
    if (readprofile (profile) == ERROR)
	{
	sysprofile = Efixvars (SYS_PROFILE);
	if (readprofile (sysprofile) == ERROR) 
	    fatal ("cannot open system editor profile from either of:\n\t%s\n\t%s",
	            profile, sysprofile);
	s_free (sysprofile);
	}
    s_free (profile);

    /* Determine the default file */

# ifdef BLTHELP
    g_noindex = !builtin_helper (DIRHELPER);
# endif
# ifdef PIPEHELP
    if (g_noindex)
	{
	cp = findfile (DIRHELPER, ".help", g_hlprpath);
	g_noindex = (cp == NULL);
	s_free (cp);
	}
# endif

    if (g_noindex)
	{
	static char default_file [PATH_MAX];  /* name of default file    */
	(void) sprintf (default_file, "%s%c%s", g_sysdir, DIR_SEPARATOR, DEFAULT_FILE);
	g_dfltfile = default_file;
	}
    else
	g_dfltfile = CUR_DIR;

    if (*filename == '\0')
	(void) strcpy (filename, g_dfltfile);

    tmopen (); 

    wp = (WSTRUCT *) s_alloc (1, T_WSTRUCT, NULL);
    g_wp = wp;
    g_warray = wp;

    wp->w__lrline = LI - 2;
    wp->w__lrcol  = CO - 1;
    wp->w__ttline = 0;
    wp->w__ttcol  = 0;
    wp->w__ftline = 0;
    wp->w__ftcol  = 0;

    if (setfile (filename, TRUE, TRUE) == ERROR)
	{
	delfiles ();
	tmclose ();
        catclose(g_e2catd);
	exit (-1);
	}
    else
	{
	if (line > -1)
	    putcurs (line, column);

	if (searchstr)
	    {
	    search (searchstr, 1);

	    /* Note: g_searchstr is the address of a static char buffer */
	    (void) strcpy (g_searchstr, searchstr);
	    }
	}

    dorestart = TRUE;
    action ();
    return(0);           /* return put here to make lint happy */
    }


/****************************************************************************/
/* make_vector():  sets up the vector for the execvp in main                */
/*                                                                          */
/* arguments:              int argc - argument counter			    */
/*                         char **argv    -  argument vector		    */
/*                        char **exec_args - new argument vector	    */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     make_vector uses argv from main to parse the elements into           */
/*     a usable vector to make an execvp call.                              */
/*     This vector will include the aixterm command followed the -e         */
/*     command,  the INed invokename, the -t flag, any other INed flags,    */
/*     the name of the file to edit, line #, column #, and searchkey.	    */
/*     The INed flags may contain a space between the flag and		    */
/*     corresponding number or fontname.  The case statement will	    */
/*     check for this and place the number or fontname as the next	    */
/*     argument in the vector.						    */
/****************************************************************************/

void make_vector(int argc, char **argv, char **exec_args )
{
  int argno, argno2, index2;
  char *exec_args2[VECTORLEN];
  int tn_flag = 0;
  char *term_name;

  argno=argno2=index2 = 0;

  /* In order to get all the parameters in the appropriate order when */
  /* they are passed to aixterm, we build two vectors and then */
  /* concatenate them later.  exec_args contains stuff aimed at the */
  /* aixterm command, exec_args2 contains stuff to be passed through to */
  /* the new invocation of INed. */
  exec_args[ argno++] = AIXTERM_CMD;

  exec_args2[argno2++] = "-e";
  exec_args2[argno2++] = invokename;
  exec_args2[argno2++] = "-t";
  
  for ( index2=1; index2 < argc; index2++ )
    {
    /* if the argument is one that INed recognizes, put it into */
    /* exec_args2 */
    if ((strcmp (argv[index2], "-d") == 0) ||
	(strcmp (argv[index2], "-D") == 0))
	{
	exec_args2[argno2++] = argv[index2];
	continue;
        }

    /* If it is a '-' flag or a '=' parameter, put it in exec_args for */
    /* aixterm to interpret. */
    if ( (argv[index2][0] == '-') || (argv[index2][0] == '=') )
	{
	exec_args[argno++] = argv[index2]; 

	/*
	 * Ref. defect 66981 & 80029.
	 * the "-fn", "-fi", "-fb", "-fs", "-tn", "-b", and "-bw" arguments all
	 * take values, so copy those values onto the exec_args list as well.
	 */
	if (( (strcmp (argv[index2], "-tn") == 0) && ( tn_flag=1 )) ||
	    (strcmp (argv[index2], "-fn") == 0) ||
	    (strcmp (argv[index2], "-fb") == 0) ||
	    (strcmp (argv[index2], "-fi") == 0) ||
	    (strcmp (argv[index2], "-fs") == 0) ||
	    (strcmp (argv[index2], "-b") == 0) ||
	    (strcmp (argv[index2], "-bw") == 0))
	    exec_args[argno++] = argv[++index2];
	}
    else	/* If the argument doesn't have a '-' or '=', put it */
		/* into exec_args2 to be passed through to INed */
	exec_args2[argno2++] = argv[index2];
  }
  /*
   * if -tn flag not passed, default to 'aixterm'.
   */
  if (tn_flag == 0) {
	exec_args[argno++] = "-tn" ;
	exec_args[argno++] = "aixterm";
    }

/*  add exec_args2 to the end of exec_args.  */
   for (index2 = 0; index2 < argno2; index2++)
       exec_args[argno++] = exec_args2[index2];

   exec_args[argno] = NULL;
}

  
/****************************************************************************/
/* mktimefile:  make new time stamp file with editor state                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_curdir                                         */
/*                         g_filename                                       */
/*                         g_helper                                         */
/*                         g_spath                                          */
/*                         g_wp                                             */
/*                         timefile                                         */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     mktimefile makes a new time stamp file to retain editor state        */
/*     information from one invocation of the editor to the next.           */
/*     "Time stamp" is rather a misnomer - it is the editor state           */
/*     the file is concerned with.  The file contains the defaults          */
/*     for the filename, line, and column number to be used if the          */
/*     user does not include them in argv when the editor is next           */
/*     started.                                                             */
/****************************************************************************/

void mktimefile (void)
    {
    WSTRUCT *wp;
    FILE *fp;
    char *alt_filename;
    int line;
    int column;

#ifdef DEBUG
    debug ("mktimefile:  timefile = '%s'", timefile);
#endif
    fp = efopen (timefile, "w");

    if (fp == NULL)
	return;
    wp = g_wp;

    if (g_spath && *g_spath) /* if inside of a structured file */
	{
	line = 0;
	column = 0;
	}
    else
	{
	line = w_line (wp) + w_ftline (wp);
	column = w_col (wp) + w_ftcol (wp);
	}

    if (g_helpstate == NOTHELPING)
        (void) fprintf (fp, "%s\n%d\n%d\n", g_filename, line, column);
    else 
	{
	alt_filename = altname();
	if (seq(alt_filename, "NO ALTERNATE FILE"))
		alt_filename = Efixvars("$HOME/.index");
        (void) fprintf (fp, "%s\n%d\n%d\n", alt_filename, line, column);
        }

    (void) fclose (fp);
    (void) chmod (timefile, 0666);
    }

/****************************************************************************/
/* quit:  handles the quit signal (control/|)                               */
/*                                                                          */
/* arguments:            none                                               */
/*                                                                          */
/* return value:         void                                               */
/*                                                                          */
/* globals referenced:   none                                               */
/*                                                                          */
/* globals changed:      dorestart                                          */
/*                                                                          */
/* synopsis:                                                                */
/*    quit simply closes the editor files, shuts down the terminal, types a */
/*    message, and exits with -1 status.  It resets the dorestart global    */
/*    so that fatal errors closing files will not cause a restart.          */
/****************************************************************************/

void quit (void)
    {
#ifdef DEBUG
    debug ("quit called");
#endif

    dorestart = FALSE;
    closeall ();
    tmclose ();
    delfiles();
    (void) fprintf (stderr, "Quit received, stopping editor session\n");
    catclose(g_e2catd);
    exit (-1);
    }



/****************************************************************************/
/*  suspend -- called when user types control-Z to suspend the editor       */
/*  temporarily.  Resets terminal modes, suspends this process.  When the   */
/*  process comes back to life, it set the terminal modes and repaints the  */
/*  screen.								    */
/****************************************************************************/

void suspend(void)
{
  static int beenhere = 0;

  (void) signal(SIGTSTP, (void(*)(int))suspend);

#ifdef DEBUG
  debug ("suspend: ");
#endif
  if (beenhere)
      return;

  beenhere = TRUE;
  s_free (Esync());
  flush_window ();

  /* this avoids some funny-looking repainting        */
  killbox (FALSE);
  Ettyfix ();

  /* Set up so that SIGTSTP will cause us to suspsend, and then send */
  /* ourselves (and any children) another SIGTSTP signal. */
  (void) signal (SIGTSTP, SIG_DFL);
  kill( 0, SIGTSTP);

  /* arrange to catch them again */
  (void) signal(SIGTSTP, (void(*)(int))suspend);

  Ettyunfix ();

  if (g_syncname) /* if file has been sync'ed */
  {
    Ereopen ();
    Refresh ();
  }

  /* If the window size has changed, winsize() will redisplay the */
  /* screen; otherwise call Edisplay to redisplay it. */
  if (!winsize())
      Edisplay();
  beenhere = FALSE;
}


/****************************************************************************/
/* winchsz - handle SIGWINCH, the signal that says the window changed       */
/* size.                                                                    */
/****************************************************************************/

void winchsz(void)
{
  (void) signal (SIGWINCH, (void(*)(int))winchsz);
#ifdef	DEBUG
  debug ("winchsz");
#endif

  /* Call routine to repaint screen if necessary */
  winsize();
} /* winchsz() */


/****************************************************************************/
/* readprofile:  reads an editor profile file and sets up globals           */
/* Returns ERROR if file cannot be opened, else RET_OK                      */
/*                                                                          */
/* arguments:              char *filename - of editor profile               */
/*                                                                          */
/* return value:           int RET_OK or ERROR                              */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        g_formpath, g_hlprpath                           */
/*                         g_usefiles, g_helpfiles, g_watchfile  g_fmtmode  */
/*                                                                          */
/* synopsis:                                                                */
/*     readprofile reads a series of records from the editor profile file   */
/*     and then invokes copylist to set the appropriate editor profile      */
/*     variables. The particular set of records it reads is determined by   */
/*     #ifdefs depending on what records are of interest.                   */
/*                                                                          */
/*     readprofile returns ERROR if it cannot open the profile file.  This  */
/*     provides a mechanism for defaulting to a system profile if the user  */
/*     does not have one.                                                   */
/****************************************************************************/

static char *recnames [] =
    {
    "menu",
    "helpmenu",
    "watchfiles",
    "paths",
    "color",
    "other",
    };

#define P_MENU 0
#define P_HELPMENU 1
#define P_WATCHFILES 2
#define P_PATHS 3
#define P_COLOR 4
#define P_OTHER 5
#define NUM_ENTRIES  (sizeof (recnames) / sizeof (char *))

int readprofile (char *filename)
    {
    POINTER *record;       /* record from profile  */
    POINTER *data;
    POINTER *data_elem;    /* data element of a record */
    SFILE *sfp;            /* profile sfile object */

    int i;
    int j;
    char *name;

#ifdef DEBUG
    debug ("readprofile:  filename = '%s'", filename);
#endif

    sfp = sf_open (filename, "r");
    if (sfp == (SFILE *) ERROR)
	return (ERROR);

    for (i = 0; i < obj_count (sf_records (sfp)); i++)
	{
	record = (POINTER *) sf_read (sfp, i);

	/* check for bad record */
	if ((record == NULL) || (record == (POINTER *) ERROR))
	    continue;

	name = obj_name (record);

	record =catscan(record,g_e2catd,MS_PROFILES);

	data = (POINTER *) s_findnode (record, "data");

	/* which record do we have? */
	for (j = 0; j < NUM_ENTRIES; j++)
	    if (seq (name, recnames [j]))
		break;

	/* process the record */
	switch (j)
	    {
	    case P_MENU: /* record EP_TASKS is the USE menu record */
		s_free ((char *) g_usefiles);
		g_usefiles = data;
		(void) s_fixtree ((POINTER *)record, (char *)data, (char *)NULL, TRUE);
		break;

	    case P_HELPMENU: /* record EP_HELP is the helpfiles */
		s_free ((char *) g_helpfiles);
		g_helpfiles = data;
		(void) s_fixtree ((POINTER *)record, (char *)data, (char *)NULL, TRUE);
		break;

	    case P_WATCHFILES: /* record EP_WATCH is the watchfiles */
		s_free ((char *) g_watchfiles);
		g_watchfiles = data;
		(void) s_fixtree ((POINTER *)record, (char *)data, (char *)NULL, TRUE);
		break;

	    case P_PATHS:
		if (data == (POINTER *)ERROR) /* check validity */
		    break;

		for (j = 0; j < obj_count (data); j++)
		    {
		    if ((data_elem = (POINTER *) data [j]) == NULL)
			continue;

		    if (seq (obj_name (data_elem), "forms"))
			{
			s_free ((char *) g_formpath);
			g_formpath = fixvlist (data_elem);
			}
		    else if (seq (obj_name (data_elem), "helpers"))
			{
			s_free ((char *) g_hlprpath);
			g_hlprpath = fixvlist (data_elem);
			}
		    }
		break;

	    case P_OTHER: /* record EP_OTHER has misc editor profile info */

		if (data == (POINTER *)ERROR) /* check validity */
		    g_fmtmode = NOWRAPPUNCT;
		else if (data && (obj_count(data) != 0) &&
			 *data && ( *((char *)data) != SPACE))
		    g_fmtmode = WRAPPUNCT;
		else
		    g_fmtmode = NOWRAPPUNCT;

		break;
	    }

	s_free ((char *)record);
	}

    (void) sf_close (sfp);

    /* now that we have path information we can do the copylist stuff */
    if ((data_elem = g_usefiles) != NULL)
	{
	g_usefiles = copylist (data_elem);
	s_free ((char *)data_elem);
	}

    if ((data_elem = g_helpfiles) != NULL)
	{
	g_helpfiles = copylist (data_elem);
	s_free ((char *)data_elem);
	}

    /* Reject this profile file if it doesn't contain reasonable */
    /* search paths */
    if ((g_hlprpath == NULL) || (g_formpath == NULL))
	return (ERROR);

    return (RET_OK);
    }

/****************************************************************************/
/* sigcatch:  signal catcher for unexpected signals                         */
/*                                                                          */
/* arguments:              int number - the signal number                   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     sigcatch is called only when we have an unexpected signal.           */
/*     Simply call fatal immediately.                                       */
/****************************************************************************/

void sigcatch (int number)
    {
    fatal ("editor interrupted by signal %d", number);
    }

/****************************************************************************/
/* watchfiles:  checks to see if any g_watchfiles have changed              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_watchfiles, watchtime                          */
/*                                                                          */
/* globals changed:        watchtime                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     It checks watchtime to see how long ago we last did watchfiles.      */
/*     If it happened too recently, as defined by WATCHSECS, just return.   */
/*                                                                          */
/*     It loops through the records in g_watchfiles, pulling the file,      */
/*     program, and desc field out of each one.  watchfiles then looks      */
/*     up each file and, if it has changed since the last watchtime,        */
/*     invokes the prog subfield if there, or puts up the desc subfield     */
/*     in a box for three seconds.  Finally, the watchtime global is reset. */
/****************************************************************************/

void watchfiles (void)
    {
    int i;               /* g_watchfiles index            */
    char *name;          /* file name                     */
    char *prog;          /* program field                 */
    char *desc;          /* desc field                    */
    char *newname;       /* fixvar'ed name                */
    char *newprog;       /* fixvar'ed prog                */
    long curtime;        /* current time                  */

    struct stat statbuf; /* stat buffer for dirname       */

    (void) time (&curtime);

#ifdef DEBUG
    debug ("watchfile:  watchtime = %.24s", ctime (&watchtime));
    debug ("watchfile:  curtime = %.24s", ctime (&curtime));
#endif

    if (curtime - watchtime < WATCHSECS)
	return;

    if (g_intflag)
        g_intflag = FALSE;

    for (i = 0; i < obj_count (g_watchfiles); i++)
	{
	name = s_findnode ((POINTER *)g_watchfiles [i], "name");
	desc = s_findnode ((POINTER *)g_watchfiles [i], "desc");
	prog = s_findnode ((POINTER *)g_watchfiles [i], "prog");

	if ((name == (char *) ERROR) ||
	    ((desc == (char *) ERROR) && (prog == (char *) ERROR)))
	    continue;

	newname = Efixvars (name);

#ifdef DEBUG
	debug ("watchfile:  checking file '%s'", newname);
#endif

	if ((stat (newname, &statbuf) != ERROR) &&
	    (statbuf.st_mtime > watchtime))
	    {

#ifdef DEBUG
	    debug ("watchfile:  file '%s' has changed", newname);
#endif
	    if ((prog != (char *) ERROR) && (*prog != '\0'))
		{
		if ((desc != (char *) ERROR) && (*desc != '\0'))
		    Emessage (M_EWATCHMSG, "%s\nPlease wait.", desc);

		newprog = Efixvars (prog);
		usepop (newprog, NULL, TRUE, TRUE); /* ring the bell */
		s_free (newprog);
		}
	    else
		Eerror (M_EWATCHFILES, "%s\nPress CANCEL to continue editing.",
			(desc != (char *)ERROR) ? desc : "");

	    Edisplay (); /* clear the box from the screen */
	    }
	s_free (newname);
	}
    watchtime = curtime;
    }


#ifdef LEAK_FINDING
/* leak_cleanup: clean up memory allocations on exit for leak finding */
static void leak_cleanup (void)
    {
    extern POINTER *termdesc;
    extern char    *delname;
    extern char    *poundname;

    s_free ((char *) g_altstack);
    s_free ((char *) g_dispname);
    s_free ((char *) g_formcache);
    s_free ((char *) g_formname);
    s_free ((char *) g_lastarg);
    s_free ((char *) g_warray);
    s_free ((char *) g_homedir);
    s_free ((char *) delname);
    s_free ((char *) poundname);
    s_free (g_filename);
    s_free (g_record);
    s_free (g_helper);

    /* free up profile information */
    s_free ((char *) g_formpath);
    s_free ((char *) g_helpfiles);
    s_free ((char *) g_hlprpath);
    s_free ((char *) g_usefiles);
    s_free ((char *) g_watchfiles);
    s_free ((char *) tasknames);
    s_free ((char *) g_tmpfiles);

    s_free ((char *) g_zoomstack);

    /* free up display code allocations */
    s_free (g_goalscreen);
    s_free (g_goalcrc);
    s_free (g_curscreen);
    s_free (g_curcrc);
    s_free (termdesc);

    }
#endif

