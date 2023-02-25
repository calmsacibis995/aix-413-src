#if !defined(lint)
static char sccsid[] = "@(#)64	1.6  src/tenplus/e2/common/zoom.c, tenplus, tenplus411, GOLD410 3/23/93 11:54:57";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Esetpath, altname, checkpop, cleanstack, popstate,
 *		pushstate, ulhcfields, zi_free, zi_print, zoomin,
 *		zoominit, zoomout
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
/* file:  zoom.c                                                            */
/*                                                                          */
/* routines to handle zooming in and out                                    */
/****************************************************************************/

#include "def.h"
#include "INeditor_msg.h"

    /***** ZOOMINFO object definition *****/

static T_ZOOMINFO; /* datatype code for ZOOMINFO object */

typedef struct zoominfo ZOOMINFO;

struct zoominfo
    {
    char *zi__dir;          /* g_curdir                           */
    char *zi__name;         /* g_filename                         */
    char *zi__spath;        /* g_spath                            */
    char *zi__form;         /* form name                          */
    char *zi__helper;       /* helper name                        */
    char *zi__field;        /* field name                         */
    int   zi__line;         /* line number in field (zero based)  */
    int   zi__column;       /* column number in field (zero based */
    char *zi__tabs;         /* tab settings                       */
    int   zi__lmar;         /* left margin                        */
    int   zi__rmar;         /* right margin                       */
    char *zi__dispname;     /* g_dispname                         */
    char *zi__state;        /* state for Hsavestate/Hrestart      */
    };

#define zi_dir(zip)        ((zip)->zi__dir)
#define zi_name(zip)       ((zip)->zi__name)
#define zi_spath(zip)      ((zip)->zi__spath)
#define zi_form(zip)       ((zip)->zi__form)
#define zi_helper(zip)     ((zip)->zi__helper)
#define zi_field(zip)      ((zip)->zi__field)
#define zi_line(zip)       ((zip)->zi__line)
#define zi_column(zip)     ((zip)->zi__column)
#define zi_tabs(zip)       ((zip)->zi__tabs)
#define zi_lmar(zip)       ((zip)->zi__lmar)
#define zi_rmar(zip)       ((zip)->zi__rmar)
#define zi_dispname(zip)   ((zip)->zi__dispname)
#define zi_state(zip)      ((zip)->zi__state)

#define FILEFIELD "Files"       /* name of file field in index form     */



LOCAL void ulhcfields (void);
LOCAL void zi_free (ZOOMINFO *);
LOCAL void zi_print (ZOOMINFO *, int);


/****************************************************************************/
/* Esetpath:  changes g_spath and g_sform (helper callable)                 */
/*                                                                          */
/* arguments:              char *newpath - new value for g_spath            */
/*                         char *newform - new value for g_sform            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_spath                                          */
/*                                                                          */
/* globals changed:        g_spath                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     Flush the current window, set g_spath to the given value,            */
/*     invoke the digin procedure, and, if the new form is provided,        */
/*     attempt to change to that form.  Finally, invoke ulhcfields          */
/*     to force all fields to top of file.                                  */
/****************************************************************************/

void Esetpath (char *newpath, char *newform)
    /* new g_spath */
    /* new g_form  */
    {
    flush_window ();   /* make sure file is up to date */
    s_free (g_spath);
    g_spath = s_string (newpath);
    digin ();

    if (*newform != '\0' && Euseform (newform) == ERROR)
	fatal ("Esetpath:  cannot open form '%s'", newform);

    ulhcfields (); /* force fields to top of file */
    }

/****************************************************************************/
/* altname:  returns the alternate file name ( NOT A COPY!)                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - pointer to the start of alt file name,  */
/*                                  or "NO ALTERNATE FILE" if none exists   */
/*                                                                          */
/* globals referenced:     g_altstack                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If g_altstack is not null, return the name of its first element;     */
/*     otherwise return the no alternate file message.                      */
/****************************************************************************/

char *altname (void)
    {
    if (g_altstack)
	return (zi_name ((ZOOMINFO *) g_altstack [0]));

    return ("NO ALTERNATE FILE");
    }

/****************************************************************************/
/* checkpop:  see whether frame on zoomstack refers to current file         */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - TRUE or FALSE                              */
/*                                                                          */
/* globals referenced:     g_zoomstack                                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     checkpop is used to determine whether it is approriate to pop the    */
/*     zoomstack when killing a helper.  It returns TRUE only if the        */
/*     filename in the topmost zoomstack element corresponds to the current */
/*     filename.  If the zoomstack is empty, it returns FALSE.              */
/****************************************************************************/

int checkpop (void)
    {
    register ZOOMINFO *zip;  /* zoominfo object for g_zoomstack */

    if ((g_zoomstack) && (obj_count (g_zoomstack) > 0))
	zip = (ZOOMINFO *) g_zoomstack [0];
    else
	return (FALSE);

#ifdef DEBUG
    debug ("checkpop:  zi_name (zip) = '%s', g_filename = '%s'",
	    zi_name (zip), g_filename);
    debug ("checkpop:  return value = %s",
	    seq (zi_name (zip), g_filename) ? "TRUE" : "FALSE");
#endif

    if (seq (zi_helper (zip), DIRHELPER))
	return (seq (zi_name (zip), g_curdir));
    else
	return (seq (zi_name (zip), g_filename));
    }

/****************************************************************************/
/* cleanstack:  flush references to current file from top of zoomstack      */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_zoomstack, g_filename                          */
/*                                                                          */
/* globals changed:        g_zoomstack                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     Remove all stack frames from the zoomstack whose filename subfield   */
/*     corresponds to the current filename.                                 */
/****************************************************************************/

void cleanstack (void)
    {
    while (seq (g_filename, zi_name ((ZOOMINFO *) g_zoomstack[0])))
	{
	s_free (g_zoomstack [0]);
	g_zoomstack = (POINTER *) s_delete ((char *)g_zoomstack, 0, 1);
	}
    }

/****************************************************************************/
/* popstate:  restore current state from zoom stack                         */
/*                                                                          */
/* arguments:              int helperflag - TRUE if helper should           */
/*                                          always be started               */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_zoomstack, g_debugfp, g_filename, g_spath,     */
/*                         g_wp, g_warray, g_helper, g_dispname, g_curdir   */
/*                                                                          */
/* globals changed:        g_zoomstack, g_spath, g_wp, g_dispname, g_curdir */
/*                                                                          */
/* synopsis:                                                                */
/*     Pop the zero'th element from g_zoomstack and use it to set the       */
/*     editor state.  If the filename of that element is different from     */
/*     the current g_filename, try to change to that file.  Make sure       */
/*     the directory is correct in case Eusefile changed it.  Make sure     */
/*     the correct form is in use, move to the zero'th window, and put      */
/*     the cursor in the right spot.  If the input helperflag is TRUE,      */
/*     or if the helper has changed, open or restart the new helper.        */
/*     Finally, reset the tabs, the margins, and the g_dispname global.     */
/****************************************************************************/

void popstate (int helperflag)
    /* TRUE if helper should always be started */
    {
    register WSTRUCT *wp;     /* current window                  */
    register ZOOMINFO *zip;  /* zoominfo object for g_zoomstack */
    register POINTER *stack; /* used to save the zoom stack     */
    register formflag;       /* TRUE if form is *win*           */

    zip = (ZOOMINFO *) g_zoomstack [0];
    g_zoomstack = (POINTER *) s_delete ((char *)g_zoomstack, 0, 1);
    formflag = seq (zi_form (zip), "*win*");

#ifdef DEBUG
    debug ("popstate:  zip = 0%o", zip);

    if (g_debugfp && zip)
	s_print (zip);
#endif

    if (!seq (g_filename, zi_name (zip))) /* if switching to different file */
	{
	stack = g_zoomstack; /* save zoom stack, so setfile won't free it */
	g_zoomstack = NULL;

	if (setfile (zi_name (zip), FALSE, formflag) == ERROR)
	    fatal ("popstate:  setfile failure on file '%s'", zi_name (zip));

	g_zoomstack = stack;

	/* If we are in a directory, we must start the index helper
	   immediately so that it is safe to call digin () below. */

	/* provide state information for Hrestart if helperflag is set  */

	if (seq (zi_helper (zip), DIRHELPER))
	    openhelper (DIRHELPER, zi_name (zip),
				       helperflag ? zi_state (zip) : NULL);
	}
    s_free (g_spath);
    g_spath = s_string (zi_spath (zip));
    digin ();

    /* If the environment we are restoring had an Eusefile call, g_curdir
       may no longer agree with the directory of g_filename, and we need
       to fix it now. */

    if (!seq (g_curdir, zi_dir (zip)))
	{
	(void) strcpy (g_curdir, zi_dir (zip));

	if (chdir (g_curdir) == ERROR)
	    fatal ("popstate:  cannot chdir to '%s'", g_curdir);
	}

    /***** if setfile did not open form ********/

    if (!formflag && Euseform (zi_form (zip)) == ERROR)
	fatal ("popstate:  cannot open form '%s'", zi_form (zip));

    /***** ???is this necessary??? *****/
    g_wp = &g_warray [1];

#ifdef CAREFUL
    if (Eputcursor (zi_field(zip), zi_line (zip), zi_column (zip))
	== ERROR)
	 fatal ("popstate: Eputcursor failed on field '%s'", zi_field (zip));
#else
    Eputcursor (zi_field(zip), zi_line (zip), zi_column (zip));
#endif

    /***** if helper wrong, start it now *****/

    if ((helperflag) || (!seq (zi_helper (zip), g_helper)))
	{
	/* don't "start" the index helper because we did it above.      */

	/* provide state information for Hrestart if helperflag is set  */

	if (!seq (zi_helper (zip), DIRHELPER))
	    openhelper (zi_helper (zip), zi_name (zip),
				       helperflag ? zi_state (zip) : NULL);
	}

    wp = g_wp;
    s_free (w_tabs (wp));
    wp->w__tabs = makestring (zi_tabs (zip));
    wp->w__lmar = zi_lmar (zip);
    wp->w__rmar = zi_rmar (zip);

    s_free (g_dispname);
    g_dispname = makestring (zi_dispname (zip));

    s_free ((char *) zip);
    }

/****************************************************************************/
/* pushstate:  saves the current state on the zoom stack                    */
/*                                                                          */
/* arguments:              int dosavestate - TRUE if should call Hsavestate */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_zoomstack, g_wp, g_helper, g_curdir, g_filenam */
/*                         g_spath, g_formname, g_dispname, g_debugfp,      */
/*                         T_ZOOMINFO                                       */
/*                                                                          */
/* globals changed:        g_zoomstack                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     The editor state variables are bundled together into a zoominfo      */
/*     object and pushed into the g_zoomstack global as the new zero'th     */
/*     element.  The information in the object include the current file     */
/*     name, current spath, form name, helper name, field, cursor position, */
/*     tab stops and margins, and g_dispname.   If there is a helper, call  */
/*     Hsavestate to provide it with an opportunity to save state info.     */
/****************************************************************************/

void pushstate (int dosavestate)
        /* TRUE if should call Hsavestate; else FALSE */
    {
    register WSTRUCT *wp;    /* current window                 */
    register ZOOMINFO *zip; /* zoominfo object for zoom stack */
    register char *filename;

    zip = (ZOOMINFO *) s_alloc (1, T_ZOOMINFO, NULL);
    wp = g_wp;

    if (seq (g_helper, DIRHELPER))
	filename = g_curdir;
    else
	{
	filename = g_filename;
	}

    zip->zi__dir = s_string (g_curdir);
    zip->zi__name = s_string (filename);
    zip->zi__spath = s_string (g_spath);
    zip->zi__form = s_string (g_formname);
    zip->zi__helper = s_string (g_helper);
    zip->zi__field = s_string (w_name (wp));
    zip->zi__line = w_ftline (wp) + w_line (wp);
    zip->zi__column = w_ftcol (wp) + w_col (wp);
    zip->zi__tabs = makestring (w_tabs (wp));
    zip->zi__lmar = w_lmar (wp);
    zip->zi__rmar = w_rmar (wp);
    zip->zi__dispname = makestring (g_dispname);

    zip->zi__state = ((dosavestate) && (g_helper)) ? Hsavestate ()
							    : (char *) NULL;

#ifdef DEBUG
    debug ("pushstate:  zip = 0%o", zip);

    if (g_debugfp)
	s_print (zip);

    debug ("pushstate:  dosavestate = %s",
	    dosavestate ? "TRUE" : "FALSE");
#endif

    if (g_zoomstack)
	g_zoomstack = s_prefix ((POINTER *)g_zoomstack, (char *)zip);
    else
	g_zoomstack = s_pointer ((char *)zip);
    }

/****************************************************************************/
/* ulhcfields: force all fields to top, cursor to 1st window                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     g_warray, g_wp, g_wnode                          */
/*                                                                          */
/* globals changed:        g_warray, g_wp, g_wnode                          */
/*                                                                          */
/* synopsis:                                                                */
/*     Set the saved node (g_wnode) to ERROR.  For each window in the       */
/*     g_warray global, put the cursor in the upper left.                   */
/*     At the end of the loop, the cursor is in the upper left corner of    */
/*     the first "tabable" (i.e. writeable or zoomable) window in the list. */
/****************************************************************************/

LOCAL void ulhcfields (void)
    {
    register i;
    WSTRUCT *first_tabable;

#ifdef DEBUG
    debug ("ulhcfields called");
#endif

    first_tabable = &g_warray [1];
    g_wnode = (char *) ERROR;

    for (i = obj_count (g_warray) - 1; i > 0; i--)
	{
	g_wp = &g_warray [i];
	if (!getstat (w_flags (g_wp), INVISIBLE | USERRO | CATENTRY) ||
	    !getstat (w_flags (g_wp), INVISIBLE | NOZOOM | CATENTRY))
	    first_tabable = g_wp;
	putcurs (0, 0);
	}
    g_wp = first_tabable;
    putcurs (0, 0);
    }

/****************************************************************************/
/* zi_free:  frees a zoominfo object                                        */
/*                                                                          */
/* arguments:              ZOOMINFO *zip - the zoom info object to be freed */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     T_ZOOMINFO                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     zi_free frees a zoominfo object by invoking s_free on its name,      */
/*     spath, form, helper, field, tabs, and dispname subfields.            */
/****************************************************************************/

LOCAL void zi_free (register ZOOMINFO *zip)
     /* zoominfo object pointer        */
    {
#ifdef CAREFUL
    (void) s_typecheck (zip, "zi_free", T_ZOOMINFO);

    if (obj_count (zip) != 1)
	fatal ("zi_free:  obj_count (%d) not one", obj_count (zip));
#endif

#ifdef DEBUG
    debug ("zi_free:  called with zip = 0%o", zip);
#endif

    s_free (zi_dir (zip));
    s_free (zi_name (zip));
    s_free (zi_spath (zip));
    s_free (zi_form (zip));
    s_free (zi_helper (zip));
    s_free (zi_field (zip));
    s_free (zi_tabs (zip));
    s_free (zi_dispname (zip));

    s_free (zi_state (zip));
    }

#ifdef DEBUG
/****************************************************************************/
/* zi_print:  print out ZOOMINFO object (for DEBUG only)                    */
/*                                                                          */
/* arguments:              ZOOMINFO *zip - the object to print              */
/*                         int indent    - printout indentation level       */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     T_ZOOMINFO                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     zi_print prints out the names and contents of the name and path,     */
/*     form and helper, field, line, and col, margins, tab stops, and       */
/*     dispname subfields of the given zoominfo object, writing onto        */
/*     the debug output channel.                                            */
/****************************************************************************/

LOCAL void zi_print (register ZOOMINFO *zip, register int indent)
     /* zoominfo object pointer        */
     /* indentation level for printout */
    {
# ifdef CAREFUL
    (void) s_typecheck (zip, "zi_print", T_ZOOMINFO);

    if (obj_count (zip) != 1)
	fatal ("zi_print:  obj_count (%d) not one", obj_count (zip));
# endif

    pindent (indent);
    debug ("dir = '%s'", zi_dir (zip));

    pindent (indent);
    debug ("name = '%s', spath = '%s'",
	   zi_name (zip), zi_spath (zip));

    pindent (indent);
    debug ("form = '%s', helper = '%s'",
	   zi_form (zip), zi_helper (zip));

    pindent (indent);
    debug ("field = '%s', line = %d, col = %d",
	   zi_field (zip), zi_line (zip), zi_column (zip));

    pindent (indent);
    debug ("lmar = %d, rmar = %d, tabs = '%s'",
	   zi_lmar (zip), zi_rmar (zip), zi_tabs (zip));

    pindent (indent);
    debug ("dispname = '%s'", zi_dispname (zip));
    }
#endif

/****************************************************************************/
/* zoomin:  zoom in from current editing location                           */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           VOID                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_helper, g_snode, g_sfp, g_curdir,        */
/*                         g_debugfp, g_zoomstack, g_wnode, g_spath,        */
/*                         g_record                                         */
/*                                                                          */
/* globals changed:        g_snode, g_zoomstack, g_spath                    */
/*                                                                          */
/* synopsis:                                                                */
/*     zoomin handles the zoomin operation from whatever context the        */
/*     cursor is currently sitting in.  If the user is currently looking    */
/*     at a directory, attempty to change to the file whose name is on      */
/*     the same line as the cursor.                                         */
/*                                                                          */
/*     If the user is not looking at a directory, the cursor should be      */
/*     positioned on a zoomable field which is either indexed or prefixed.  */
/*     If not, issue an Eerror message.                                     */
/*                                                                          */
/*     Determine which form to use, either by reading it from the window    */
/*     itself or figuring out a reasonable default.  Finally, if the        */
/*     window's flag is set which says to change helpers when zooming       */
/*     into the field, open a new helper.                                   */
/****************************************************************************/

void zoomin (void)

    {
    register WSTRUCT *wp;      /* current window                        */
    register char *prefix;     /* copy of window prefix (wpfx)          */
    register char *formname;   /* name of new form                      */
    register int index1;            /* cursor line (zero based)              */
    register char *first;      /* first node of the prefix              */
    register char *rest;       /* rest of prefix                        */
    register ATTR *filename;   /* file name for dir zoom                */
    register char *efilename;  /* file name for dir zoom, external form */
    char buffer [50 * MB_LEN_MAX];
    register int wflags;       /* for w_flags (wp)                      */
    register char *oldsnode;   /* Old value of g_snode                  */
    register char *oldspath;   /* Old value of g_spath                  */

#ifdef DEBUG
    debug ("zoomin:  called");
#endif

    flush_window ();   /* make sure file is up to date */

    wp = g_wp;
    wflags = w_flags (wp);

    index1 = w_ftline (wp) + w_line (wp); /* get index from line number */
    prefix = w_pfx (wp);

#ifdef DEBUG
    debug ("zoomin:  index = %d, status = 0%o, g_spath = '%s'",
	index1, wflags, g_spath);
    debug ("zoomin:  pfx = '%s', sfx = '%s', zoom = '%s'",
	prefix, w_sfx (wp), w_zoom (wp));
#endif

    /***** if zooming in from directory, try to open file *****/

    if ((seq (g_helper, DIRHELPER)) &&
	(g_snode == (char *) sf_records (g_sfp)))
	{
	if (!seq (w_name (wp), FILEFIELD))
	    {
	    if (gowin (FILEFIELD) == ERROR)
		fatal ("zoomin:  cannot go to window '%s'", FILEFIELD);

	    putcurs (index1, 0);
	    }
	filename = readline (index1, 0);

	if (*filename == (ATTR) '\0')
	    Eerror (M_EFILEZOOM, "There is no file name on current line.");
	else
	    {
	    efilename = packup (filename);
	    efilename = pathcat (s_string (g_curdir), efilename);
	    /* setfile will complain if the file can't be edited        */
	    setfile (efilename, TRUE, TRUE);
	    s_free (efilename);
	    }
	s_free ((char *)filename);
	return;
	}
    /***** Not editing a directory file. *****/
    /***** see if zoomin is legal from this field *****/

    if ((getstat (wflags, NOZOOM) ||
	(!getstat (wflags, INDEXED)) && (prefix == NULL)))
	{
	Eerror (M_EZOOMIN, "Cannot ZOOM IN any further.");
	return;
	}

    formname = makestring (w_zoom (wp));

    save_reclocs ();

    pushstate (FALSE); /* save current state of zoom stack, don't Hsavestate */

    oldsnode = g_snode;

#ifdef DEBUG
    if (g_debugfp)
	{
	debug ("zoomin:  g_zoomstack = 0%o:", g_zoomstack);
	s_print (g_zoomstack);
	}
#endif
    if (getstat (wflags, INDEXED))
	{
	/* if there is a prefix, use it in making the g_spath */
	/* g_spath is set up first for partial record reading */

	oldspath = s_string (g_spath); /* remember old spath */

	if (prefix)
	    (void) sprintf (buffer, "%s/%d", prefix, index1);
	else
	    (void) sprintf (buffer, "%d", index1);

	g_spath = pathcat (g_spath, s_string (buffer));

	if (g_wnode == (char *) ERROR) /* if g_wnode invalid */
	    pfxnode (FALSE);              /* set up if possible */

	if (g_wnode == NULL) /* if no data for window, don't try to descend */
	    g_snode = NULL;
	else
	    g_snode = (char *) idxnode (index1, FALSE);

	if (formname == NULL)  /* choose the right default form if none specified */
	    {
	    if (getstat (wflags, NAMEFIELD))
		formname = defaultform ();
	    else if (w_sfx (wp) != NULL)
		formname = s_string (INDEXFORM);
	    else /* not name field and empty suffix */
		formname = s_string (STANDARDFORM);
	    }
	if (Euseform (formname) == ERROR)
	    {
	    Eerror (M_EZOOMFORM, "Cannot open form %s.", formname);

	    s_free (g_zoomstack [0]); /* clean up zoom stack */
	    g_zoomstack = (POINTER *) s_delete ((char *)g_zoomstack, 0, 1);
	    s_free (formname);
	    g_snode = oldsnode;
	    s_free (g_spath);
	    g_spath = oldspath;
	    return;
	    }
	ulhcfields (); /* force fields to top of file */

	if (getstat (wflags, CHANGEHELPER))
	    openhelper (formname, g_filename, NULL);

	s_free (formname);
	s_free (oldspath);
	return;
	}
    /* if not an indexed window */

    first = firstnode (prefix); /* this is s_alloc'ed */
    rest = restofpath (prefix); /* this is s_alloc'ed */

    if (formname == NULL) /* if no zoom form use correct default */
	formname = rest ? s_string (INDEXFORM) : s_string (STANDARDFORM);

    s_free (rest);

#ifdef DEBUG
    debug ("zoomin:  non-indexed field, formname = '%s'", formname);
#endif

    if (g_snode == (char *) sf_records (g_sfp)) /* if at record level */
	{
	(void) findrecord (first);
	g_snode = g_record;
	}
    else /* if not indexed and below the record level */
	if (g_snode != NULL) /* if data at current level, try to descend */
	    {
	    g_snode = s_findnode ((POINTER *)g_snode, first);
	    if (g_snode == (char *) ERROR)
		g_snode = NULL; /* make it NULL if not found */
	    }
    if (Euseform (formname) == ERROR) /* Note helper is closed */
	{
	Eerror (M_EZOOMFORM, "Cannot open form %s.", formname);
	s_free (first);
	s_free (formname);
	s_free (g_zoomstack [0]); /* clean up zoom stack */
	g_zoomstack = (POINTER *) s_delete ((char *)g_zoomstack, 0, 1);
	g_snode = oldsnode;
	return;
	}
    ulhcfields (); /* force fields to top of file */
    g_spath = pathcat (g_spath, first);

    if (getstat (wflags, CHANGEHELPER))
	openhelper (formname, g_filename, NULL);

    s_free (formname);
    }

/****************************************************************************/
/* zoominit:  initializes ZOOMINFO object                                   */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     T_ZOOMINFO                                       */
/*                                                                          */
/* globals changed:        T_ZOOMINFO                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     Set the T_ZOOMINFO type global to the result of invoking s_addtype   */
/*     to add the zoominfo data type.                                       */
/****************************************************************************/

void zoominit (void)
    {
#ifdef DEBUG
    debug ("zoominit called");
#endif

#ifdef DEBUG
    T_ZOOMINFO = s_addtype ("zoominfo", sizeof (ZOOMINFO), zi_print,
		 noop, noop, zi_free);
#else
    T_ZOOMINFO = s_addtype ("zoominfo", sizeof (ZOOMINFO), (void(*)(void *, int))noop, (int (*)(void *, FILE *))noop, (int (*)(void *, FILE *))noop, (void(*)(void *))zi_free);
#endif
    }

/****************************************************************************/
/* zoomout: move to the level above current file                            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           VOID                                             */
/*                                                                          */
/* globals referenced:     g_snode, g_spath, g_noindex, g_sfp, g_zoomstack, */
/*                         g_helper, g_curdir, g_filename, g_debugfp, g_wp  */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     zoomout zooms out to the next outer level.  Its action depends on    */
/*     whether the current level is somewhere inside the structured file    */
/*     being edited, or whether it is actually at top file level.           */
/*                                                                          */
/*     If the current level is at file level, zoomout looks at the          */
/*     g_curdir global to find the next level up.  It succeeds unless       */
/*     the current file is the root "/", or if the index helper is          */
/*     not available (the g_noindex flag is set).                           */
/*                                                                          */
/*     If the current level is inside a structured file, zoomout looks      */
/*     at the g_spath global to find the next level up.  It restores the    */
/*     previous state from the g_zoomstack stack.                           */
/****************************************************************************/

void zoomout (void)

    {
    register i;              /* used to search for name in directory */
    register ATTR *name;     /* file name from record                */
    register char *ename;    /* file name from record, external form */
    register length;         /* length of index file                 */
    register char *filename; /* used for file zooming                */
    register flag;           /* used to search for name in directory */
    register char *onlyname; /* last field in filename               */
    register char *oldspath; /* used for in file zooming             */
    register char *endpath;  /* part of spath that was removed       */
    extern char g_helpstate; /* HELP key context                     */

#ifdef DEBUG
    debug ("zoomout:  g_snode = 0%o, g_spath = '%s'", g_snode, g_spath);
#endif
    if ((g_helpstate == INHELPFILE) && ((!g_spath) || (!*g_spath)))
	{
	g_helpstate = NOTHELPING;
        catclose(g_helpfilecatd);
	popstate (TRUE);
	return;
	}
    flush_window (); /* make sure file is up to date */

    if (g_noindex && (g_snode == (char *) sf_records (g_sfp)))
	{
	Eerror (M_EZOOMOUT, "Cannot ZOOM OUT any further.");
	return;
	}

    if ((g_snode == (char *) sf_records (g_sfp)) || /* if at file level */
	(g_zoomstack == NULL) || (obj_count (g_zoomstack) == 0))
	{
	if (seq (g_helper, DIRHELPER))
	    {
	    if (seq (g_curdir, "/"))
		{
	        Eerror (M_EZOOMOUT, "Cannot ZOOM OUT any further.");
		return;
		}
	    filename = trimpath (s_string (g_curdir));
	    onlyname = strrchr (g_curdir, DIR_SEPARATOR);
	    }
	else
	    {
	    filename = trimpath (s_string (g_filename));
	    onlyname = strrchr (g_filename, DIR_SEPARATOR);
	    }

	if (onlyname == NULL) /* if can't find slash */
	    fatal ("zoomout:  can't find last field in '%s'", g_filename);
	else
	    onlyname = s_string (++onlyname); /* make a copy of the name */

	if (setfile (filename, TRUE, TRUE) == ERROR)
	    {
	    s_free (filename);
	    s_free (onlyname);
	    return;
	    }
	/***** search for line w/ name on it *****/

	if (gowin (FILEFIELD) == ERROR)
	    fatal ("zoomout:  gowin failure to field '%s'", FILEFIELD);

	length = Enumlines ("");

	for (i = 0; i < length; i++)
	    {
	    name = getli (i);
	    ename = packup (name);
	    flag = seq (ename, onlyname);
	    s_free (ename);
	    s_free ((char *)name);

	    if (flag)
		break;
	    }
	if (i == length)
	    i = 0;

	putcurs (i, 0);
	s_free (filename);
	s_free (onlyname);
	return;
	}

    /***** Not at file level, so zoomout one level using zoom stack info ***/

#ifdef DEBUG
    if (g_debugfp)
	{
	debug ("zoomout:  g_zoomstack = 0%o:", g_zoomstack);
	s_print (g_zoomstack);
	}
#endif

    oldspath = s_string (g_spath);

    /* pass over any stack frames from helpers pushed from task menu */
    while (seq (oldspath, g_spath))
	popstate (FALSE); /* restore current state from g_zoomstack */

    if (getstat (w_flags (g_wp), INDEXED))
	{
	endpath = strrchr (oldspath, DIR_SEPARATOR);

	if (endpath == NULL)
	    endpath = oldspath;
	else
	    ++endpath;  /* pass slash */

#ifdef DEBUG
	debug ("zoomout:  oldspath = '%s', endpath = '%s'", oldspath, endpath);
#endif

	putcurs (atoi (endpath), 0);
	}
    s_free (oldspath);
    }
