#if !defined(lint)
static char sccsid[] = "@(#)95	1.8  src/tenplus/e2/common/imports.c, tenplus, tenplus411, GOLD410 3/3/94 18:48:45";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Ebell, Ecallfun, Echangepfx, Ecolumn, Egetttline,
 *		Egettbline, Edelline, Edispname, Eemptyrecord,
 *		Efieldname, Efilename, Eformname, Egetflags, Egetlist,
 *		Egetpath, Egetprefix, Egetsuffix, Egettext, Einsline,
 *		Ekeyvalue, Eline, Enumlines, Eoldhelper, Eputcursor,
 *		Eputformat, Eputlist, Eputtext, Ereadonly,
 *		Ewritefield, gowin, puttext, strip, Ecopyprofile
 *	        Egetmessage, Ecountlines, Esizelines 
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* file:  imports.c                                                         */
/*                                                                          */
/* helper interface routines                                                */
/*                                                                          */
/*   NOTES:                                                                 */
/*                                                                          */
/*   All helper callable editor functions have corresponding routines in th */
/*   editor that start with "E", such as "Eerror".  Since the current pipe  */
/*   protocol (funp) does not allow POINTER arrays or routines that return  */
/*   either a string or ERROR (an int), both the editor and helper have     */
/*   interface special interface routines.  The editor interface routines   */
/*   start with "I", and the helper routines start with "E".  This is done  */
/*   so that when the helpers are built into the editor (without their "E"  */
/*   routines) they can use the editor "E" routines directly, instead of    */
/*   having to go through the pipe protocol conversions.                    */
/*                                                                          */
/*   The ask, getprefix, getsuffix and gettext functions all return either  */
/*   string or ERROR.  The editor has both "E" and "I" routines for all of  */
/*   them.  The "E" routine matches the protocol and the "I" routine in the */
/*   editor converts the ERROR integer into a special error string.  On the */
/*   helper side, there is also a special "E" routine which converts the    */
/*   error string back to ERROR.                                            */
/*                                                                          */
/*   Similarly, the getlist routine returns a pointer array (or ERROR).     */
/*   The Egetlist routine in the editor does that, and there are Igetlist   */
/*   in the editor and Egetlist is the helper to get it through the pipe.   */
/*                                                                          */
/*   The putlist and menu routines pass POINTER arrays to the editor.  The  */
/*   Eputlist and Emenu routines in the editor takes POINTER arrays, while  */
/*   the Igetlist and Imenu routines take string versions of the POINTER    */
/*   arrays which are produced by the Eputlist and Emenu routines in the    */
/*   helper.                                                                */
/*                                                                          */
/*   The error, fatal and message routines have helper interface routines   */
/*   for converting their printf type arguments into a single string which  */
/*   is required by the pipe protocol.                                      */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

extern nl_catd g_e2catd;
extern nl_catd g_helpcatd;

/****************************************************************************/
/* Ebell:  rings the bell                                                   */
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
/*     Ebell simply sends the ascii BEL (^g) to standard error.             */
/****************************************************************************/

void Ebell (void)
    {
    (void) fputc (007, stderr);
    (void) fflush (stderr);
    }

/****************************************************************************/
/* Ecallfun:  calls an editor command function                              */
/*                                                                          */
/* arguments:              int   keyvalue - function code from keynames.h   */
/*                         int   type     - NOARGS or EMPTYARG or ...       */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_edfunction, g_hooks                */
/*                                                                          */
/* globals changed:        g_keyvalue, g_hooks                              */
/*                                                                          */
/* synopsis:                                                                */
/*     Ecallfun temporarily sets the value of g_keyvalue to the given       */
/*     value, invokes callfunction to perform the action of the requested   */
/*     key with the given arguments, and then restores g_keyvalue.          */
/*     It also saves and restores the g_hooks entry for the requested key,  */
/*     so that a helper's Ecallfun call does not trigger Hbefore or Hafter. */
/*     Calling Ecallfun with the LEARN key is illegal.                      */
/****************************************************************************/

void Ecallfun (int keyvalue, int type, char *strvalue, int numvalue)
    {
    int oldkey;        /* local g_keyvalue           */
    char oldhook;      /* local control hook on key  */


    oldkey  = g_keyvalue;               /* save current keyvalue     */
    oldhook = g_hooks [keyvalue];       /* save current hooks on key */

    g_keyvalue = keyvalue;      /* pretend function key was touched  */
    g_hooks [keyvalue] = '\0';  /* but ignore hooks during Ecallfun  */

    callfunction (g_edfunction [keyvalue], type, strvalue, numvalue);

    g_keyvalue = oldkey;
    g_hooks [keyvalue] = oldhook;
    }

/****************************************************************************/
/* Echangepfx:  change prefix in all windows in current form                */
/*                                                                          */
/* arguments:              int base     - starting prefix number            */
/*                         char *string - end of prefix                     */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_warray                                         */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     The prefix of each window in the g_warray is changed to be           */
/*     of the form <number>/<string>, where <number> is each consecutive    */
/*     number starting at the given base and <string> is the input          */
/*     string, if provided.                                                 */
/****************************************************************************/

void Echangepfx (int base, char *string)
    {
    int i;
    char pfxbuf [50 * MB_LEN_MAX];

#ifdef DEBUG
    debug ("Echangepfx:  base = %d, string = '%s'", base, string);
#endif

    flush_window ();   /* make sure file is up to date */

    for (i = 1; i < obj_count (g_warray); i++)
	{
	(void) sprintf (pfxbuf, "%d/%s", base++, string);
	s_free (w_pfx (&g_warray [i]));
	g_warray [i].w__pfx = s_string (pfxbuf);
	}
    getall (); /* get all gangs into cache and screen */
    }

/****************************************************************************/
/* Ecolumn:  returns current column in 0-based field coords                 */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - the zero-based column number of cursor     */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     The cursor column is converted from window coordinates into field    */
/*     coordinates and returned.                                            */
/****************************************************************************/

int Ecolumn (void)
    {
    WSTRUCT *wp; /* current window */

#ifdef DEBUG
    debug ("Ecolumn called");
#endif

    wp = g_wp;
    return (w_col (wp) + w_ftcol (wp));
    }

/*****************************************************************************
* Egetttline:  return ttline of ith window in g_warray                       *
*                                                                            *
* arguments:              i - the window number                              *
*                                                                            *
* return value:           int - the ttline of window i                       *
*                                                                            *
* globals referenced:     g_wp                                               *
*                                                                            *
* globals changed:        none                                               *
*                                                                            *
* synopsis:                                                                  *
*     The ttline of the ith window is returned.  Used by the print helper    *
*     for variable sized screens                                             *
*****************************************************************************/

int Egetttline (int i)
    {
    WSTRUCT *wp;

#ifdef DEBUG
    debug ("Egetttline called");
#endif

    wp = &g_warray [i];
    return (w_ttline (wp));
    }

/*****************************************************************************
* Egettbline:  return tbline of ith window in g_warray                       *
*                                                                            *
* arguments:              i - the window number                              *
*                                                                            *
* return value:           int - the tbline of window i                       *
*                                                                            *
* globals referenced:     g_wp                                               *
*                                                                            *
* globals changed:        none                                               *
*                                                                            *
* synopsis:                                                                  *
*     The tbline of the ith window is returned.  Used by the print helper    *
*     for variable sized screens                                             *
****************************************************************************/

int Egettbline (int i)
    {
    WSTRUCT *wp;

#ifdef DEBUG
    debug ("Egettbline called");
#endif

    wp = &g_warray [i];
    return (w_tbline (wp));
    }

/****************************************************************************/
/* Edelline:  helper interface routine for deleting lines                   */
/*                                                                          */
/* arguments:              char *fieldname - name of field to modify, or    */
/*                                           NULL for current field         */
/*                         int   line      - zero based field line at which */
/*                                           to start deletion, or -1 for   */
/*                                           current line                   */
/*                         int   count     - number of lines to delete      */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     g_readonly, g_wp, g_wnode                        */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Edelline returns ERROR if the current file is readonly, or if the    */
/*     user has specified a bad fieldname.  Otherwise it invokes delli      */
/*     to delete the appropriate number of lines starting at the given line */
/*     If the user specified a different fieldname, the g_wnode global is   */
/*     set to ERROR.                                                        */
/****************************************************************************/

int Edelline (char *fieldname, int line, int count)
    {
    WSTRUCT *wp; /* current window              */

#ifdef DEBUG
    debug ("Edelline:  fieldname = '%s', line = %d, count = %d",
	    fieldname, line, count);
    debug ("Edelline:  current field = '%s'", w_name (g_wp));
#endif

    if (g_readonly)
	 {
	 if(g_readonly == DIR_NOREAD)
	     Eerror (M_EDIRREADONLY, "You must have directory write permission to edit the specified file.");
	 else
	     Eerror (M_EREADONLY, "Cannot modify this file.  Check permissions");
	 return (ERROR);
	 }
    if (count > MAXFLINE)
	return (ERROR);

    wp = g_wp;

    if (line < 0)
	line = w_ftline (wp) + w_line (wp);

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    if (count == ERROR)
	count = Enumlines ("") - line;

    if ((line >= w_ftline (wp)) && (line <= w_ftline (wp) + w_lrline (wp)))
	{
	char hook; /* UDEL hook */

	hook = g_hooks [UDEL]; /* temporarily disable UDEL control hooks */
	g_hooks [UDEL] = '\0';

	/***** NOTE:  the NO means do not save the lines on the stack *****/

	delli (line - w_ftline (wp), count, FALSE);
	g_hooks [UDEL] = hook;
	}
    else
	delline (line, count);

    if (*fieldname)
	{
	/* flush_window is not necessary here because
	    delli and delline have already updated the file */
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (RET_OK);
    }

/****************************************************************************/
/* Edispname:  routine to set g_dispname                                    */
/*                                                                          */
/* arguments:              char *name - to be displayed on status line      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_dispname                                       */
/*                                                                          */
/* globals changed:        g_dispname                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     The global string g_dispname is set to the given string, with        */
/*     any spaces replaced with minus ('-') characters.                     */
/****************************************************************************/

void Edispname (char *name)
    {
    char *cp; /* used to convert blanks to dashes */

#ifdef DEBUG
    debug ("Edispname:  name = '%s'", name);
#endif

    s_free (g_dispname);
    g_dispname = s_string (name);

    for (cp = g_dispname; *cp; skipmb(&cp))
        {
           if (MultibyteCodeSet)
                  {
                  /* a multibyte space takes less bytes than a '-',
                     so shift the string to fill the gap. */
                  if (ismbspace(cp)) 
                       {
		       int i;
                       size_t clen = mblen(cp, MB_CUR_MAX);
                       *cp = '-';
                       if (clen > 1) {
			   for(i=0; cp[i+clen]; i++)
				cp[i+1] = cp[i+clen];
			    cp[i+1] = cp[i+clen];
			   }
                       }
                   } 
            else 
               {
	       if (isspace(*cp))
	          *cp = '-';
               }
         }                       
    }


/****************************************************************************/
/* Eemptyrecord: returns whether the current record is empty or not         */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           TRUE if the current record is empty              */
/*                         FALSE if the current record isn't empty          */
/*                                                                          */
/* globals referenced:     g_record                                         */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Eemptyrecord returns TRUE if g_record is empty, otherwise it         */
/*     returns FALSE.                                                       */
/****************************************************************************/

int Eemptyrecord(void)
{
    POINTER *record;

#ifdef DEBUG
    debug ("Eemptyrecord:");
#endif

    record = (POINTER *) g_record;
    if (record == NULL)
        return (TRUE);
    else if (obj_count (record) == 0)
        return (TRUE);
    else 
        return (emptyrecord (record));
}



/****************************************************************************/
/* Efieldname:  returns the name of the current field                       */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - the current field name                  */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Efieldname returns a structured string with the name subfield        */
/*     of the current window.                                               */
/****************************************************************************/

char *Efieldname (void)
    {
#ifdef DEBUG
    debug ("Efieldname called");
#endif

    return s_string (w_name (g_wp));
    }

/****************************************************************************/
/* Efilename: returns g_filename                                            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - the current filename                    */
/*                                                                          */
/* globals referenced:     g_filename                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Efilename returns a structured string copy of g_filename.            */
/****************************************************************************/

char *Efilename (void)
    {
#ifdef DEBUG
    debug ("Efilename called, returning\"%s\"", g_filename);
#endif

    return (s_string (g_filename));
    }

/****************************************************************************/
/* Eformname:  returns current form name (to the helper)                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - the current form name                   */
/*                                                                          */
/* globals referenced:     g_formname                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Eformname returns a structured string copy of g_formname.            */
/****************************************************************************/

char *Eformname (void)
    {
#ifdef DEBUG
    debug ("Eformname called");
#endif

    return s_string (g_formname);
    }

/****************************************************************************/
/* Egetflags: returns flags for current field                               */
/*                                                                          */
/* arguments:              char *fieldname - name of field, or NULL         */
/*                                           for current field              */
/*                                                                          */
/* return value:           int - the flag values                            */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Egetflags gets the flag bits for the given fieldname.                */
/*     If the user specified a fieldname, the g_wnode global is set to      */
/*     ERROR                                                                */
/****************************************************************************/

int Egetflags (char *fieldname)
    {
    WSTRUCT *wp;    /* current window */
    int retval;     /* return value   */

#ifdef DEBUG
    debug ("Egetflags called");
#endif

    wp = g_wp;

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    retval = w_flags (g_wp);

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (retval);
    }

/****************************************************************************/
/* Egetlist:  routine to get a list of lines                                */
/* Returns ERROR if fieldname is invalid                                    */
/*                                                                          */
/* arguments:              char *fieldname - name of field to read, or      */
/*                                           NULL for current field         */
/*                         int   line      - zero based field number of     */
/*                                           starting line, or -1 for       */
/*                                           current                        */
/*                         int   count     - number of lines to get, or     */
/*                                           -1 for all to end of field     */
/*                                                                          */
/* return value:           POINTER *       - the list of lines, or ERROR    */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Egetlist gets the requested number of lines from the field, starting */
/*     at the given line number.  It returns (POINTER *) ERROR if the user  */
/*     has provided a bad field name.  If the user provided a field name,   */
/*     the g_wnode global is set to ERROR.                                  */
/****************************************************************************/

POINTER *Egetlist (char *fieldname, int line, int count)
    {
    WSTRUCT *wp;         /* current window   */
    int index1;          /* index being read */
    POINTER *list;       /* list of lines    */
    ATTR    *iline;      /* unpacked line    */

#ifdef DEBUG
    debug ("Egetlist:  fieldname = '%s', line = %d, count = %d",
	   fieldname, line, count);
#endif

    wp = g_wp;

    if (line == ERROR)
	line = w_ftline (wp) + w_line (wp);

    if (gowin (fieldname) == ERROR)
	return ((POINTER *) ERROR);

    if (count == ERROR)
	count = Enumlines ("") - line;

    list = (POINTER *) s_alloc (count, T_POINTER, NULL);

    for (index1 = 0; index1 < count; index1++)
	{
	iline = getli (line + index1);
	list [index1] = packup (iline);
	s_free ((char *)iline);
	}
    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (list);
    }

/****************************************************************************/
/* Egetpath: returns g_spath                                                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           char * - the current path                        */
/*                                                                          */
/* globals referenced:     g_spath                                          */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Egetpath returns a structured string copy of the g_spath global.     */
/****************************************************************************/

char *Egetpath (void)
    {
#ifdef DEBUG
    debug ("Egetpath called");
#endif

    return (s_string (g_spath));
    }

/****************************************************************************/
/* Egetprefix: returns prefix of current window                             */
/*                                                                          */
/* arguments:              char *fieldname - which window's prefix to get,  */
/*                                           or NULL for current window.    */
/*                                                                          */
/* return value:           char * - the prefix, or ERROR                    */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Egetprefix returns a structured string copy of the prefix of the     */
/*     given field, or ERROR if a bad fieldname is given.  If the user      */
/*     provides a fieldname, the g_wnode global is set to ERROR.            */
/****************************************************************************/

char *Egetprefix (char *fieldname)
    {
    char *retval;  /* prefix string  */
    WSTRUCT *wp;   /* current window */

#ifdef DEBUG
    debug ("Egetprefix called");
#endif

    wp = g_wp;

    if (gowin (fieldname) == ERROR)
	return ((char *) ERROR);

    retval = s_string (w_pfx (g_wp));

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (retval);
    }

/****************************************************************************/
/* Egetsuffix: returns the suffix of the current window                     */
/* Returns ERROR if fieldname is invalid                                    */
/*                                                                          */
/* arguments:              char *fieldname - window whose suffix to get,    */
/*                                           or NULL for current window     */
/*                                                                          */
/* return value:           char * - the suffix, or ERROR if bad fieldname   */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Egetsuffix returns a structured string copy of the suffix of the     */
/*     given fieldname, or ERROR if the fieldname was invalid.  If the      */
/*     user provided a fieldname, the g_wnode global is set to ERROR.       */
/****************************************************************************/

char *Egetsuffix (char *fieldname)
    {
    char *retval;  /* suffix string  */
    WSTRUCT *wp;   /* current window */

#ifdef DEBUG
    debug ("Egetsuffix called");
#endif

    wp = g_wp;

    if (gowin (fieldname) == ERROR)
	return ((char *) ERROR);

    retval = s_string (w_sfx (g_wp));

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (retval);
    }

/****************************************************************************/
/* Egettext:  gets a line of text from a given field                        */
/* Returns ERROR if fieldname is invalid                                    */
/*                                                                          */
/* arguments:              char *fieldname - field from which to get text,  */
/*                                           or NULL for current field      */
/*                         int   line      - zero based field number of     */
/*                                           line or -1 for current line    */
/*                                                                          */
/* return value:           char * - the line, or ERROR if bad fieldname.    */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Egettext returns a structured string copy of the indicated line      */
/*     from the given field, or ERROR if the fieldname was invalid.  If     */
/*     the user provided a fieldname, the g_wnode global is set to ERROR.   */
/****************************************************************************/

char *Egettext (char *fieldname, int line)
    {
    WSTRUCT *wp;    /* current window     */
    char *retval;   /* return string      */
    ATTR  *iline;   /* getli return value */

#ifdef DEBUG
    debug ("Egettext:  fieldname = '%s', line = %d", fieldname, line);
#endif

    wp = g_wp;

    if (line == ERROR)
	line = w_ftline (wp) + w_line (wp);

    if (gowin (fieldname) == ERROR)
	return ((char *) ERROR);

    iline = getli (line);
    retval = packup (iline);
    s_free ((char *)iline);

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (retval);
    }

/****************************************************************************/
/* Einsline:  inserts lines in zero based field coordinates                 */
/*                                                                          */
/* arguments:              char *fieldname - name of field to read from,    */
/*                                           or NULL for current window     */
/*                         int   line      - zero based field line number   */
/*                                           to start at, or -1 for current */
/*                         int   count     - number of lines to insert      */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode, g_readonly                        */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Einsline inserts the requested number of lines into the given field  */
/*     starting at the given line number. It returns ERROR if the fieldname */
/*     was invalid or the file is readonly.  If the fieldname was given,    */
/*     the g_wnode global is set to ERROR.                                  */
/****************************************************************************/

int Einsline (char *fieldname, int line, int count)
    {
    WSTRUCT *wp;    /* current window              */
    int length;     /* max of field length and cursor position */

#ifdef DEBUG
    debug ("Einsline:  fieldname = '%s', line = %d, count = %d",
	    fieldname, line, count);
    debug ("Einsline:  current field = '%s'", w_name (g_wp));
#endif

    if (g_readonly)
	 {
	 if(g_readonly == DIR_NOREAD)
	     Eerror (M_EDIRREADONLY, "You must have directory write permission to edit the specified file.");
	 else
	     Eerror (M_EREADONLY, "Cannot modify this file.  Check permissions");
	 return (ERROR);
	 }
    wp = g_wp;

    if (line < 0)
	line = w_ftline (wp) + w_line (wp);

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    length = datalength ();

    if ((count + max (line, length)) >= MAXFLINE)
	return (ERROR);

    if ((line >= w_ftline (wp)) && (line <= (w_ftline (wp) + w_lrline (wp))))
	{
	char hook; /* UINS hook */

	hook = g_hooks [UINS]; /* temporarily disable UINS control hooks */
	g_hooks [UINS] = '\0';
	insli (line - w_ftline (wp), count);
	g_hooks [UINS] = hook;
	}
    else
	insline (line, count);

    if (*fieldname)
	{
	/* flush_window is not necessary here because
	    insli and insline have already updated the file */
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (RET_OK);
    }

/****************************************************************************/
/* Ekeyvalue:  returns g_keyvalue                                           */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - the key value code from keynames.h         */
/*                                                                          */
/* globals referenced:     g_keyvalue                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Ekeyvalue simply returns the g_keyvalue global.                      */
/****************************************************************************/

int Ekeyvalue (void)
    {
#ifdef DEBUG
    debug ("Ekeyvalue called");
#endif

    return (g_keyvalue);
    }

/****************************************************************************/
/* Eline:  returns current line in 0-based field coords                     */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - zero based line number of cursor           */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Eline converts the cursor line number from window coordinates        */
/*     into field coordinates and returns the result.                       */
/****************************************************************************/

int Eline (void)
    {
    WSTRUCT *wp; /* current window */

#ifdef DEBUG
    debug ("Eline called");
#endif

    wp = g_wp;
    return (w_line (wp) + w_ftline (wp));
    }

/****************************************************************************/
/* Enumlines:  returns number of lines in current field                     */
/*                                                                          */
/* arguments:              char *fieldname - name of field whose lines to   */
/*                                           count, or NULL for current     */
/*                                           field                          */
/*                                                                          */
/* return value:           int - number of lines in the field, or ERROR     */
/*                               if given invalid fieldname                 */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Enumlines finds the number of lines in the given field by walking    */
/*     backward through the lines of its window to find the number of       */
/*     the last modified line, and returning the maximum of that value      */
/*     and the value of the datalength function for that field.  If the     */
/*     user specified the fieldname, the g_wnode global is set to ERROR.    */
/****************************************************************************/

int Enumlines (char *fieldname)
    {
    int i;
    int retval;
    int length;

    WSTRUCT *wp;
    WSTRUCT *savewp;

#ifdef DEBUG
    debug ("Enumlines ('%s') called", fieldname);
#endif

    savewp = g_wp;

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    wp = g_wp;
    i = w_lrline (wp);

    while ((i >= 0) && (w_testmod (wp, i) == 0))
	i--;

    length = datalength ();
    retval = max (length, i >= 0 ? i + w_ftline (wp) + 1 : ERROR);

    if (*fieldname)
	{
	g_wp = savewp;
	g_wnode = (char *) ERROR; /* invalidate g_wnode */
	}
    return (retval);
    }

/****************************************************************************/
/* Eoldhelper:  trap calls to incompatible (old) helpers                    */
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
/*     Eoldhelper simply invokes Efatal to get out of trouble               */
/*     immediately.  It is a sneaky way to get rid of                       */
/*     old-pipe-protocol helpers without getting in it ourselves.           */
/*     This function can be called correctly via doremote (2) from          */
/*     the helper, although who would want to?  The trick is that           */
/*     doremote (2) from the editor is Hinit, so helpers which echo         */
/*     unknown things (i.e.  anything built with funp) will kill            */
/*     themselves.                                                          */
/****************************************************************************/

void Eoldhelper (void)
    {
    Efatal (M_EOLDHELPER, "The helper program is not consistent with the editor.");
    }

/****************************************************************************/
/* Eputcursor: moves cursor to given location                               */
/*                                                                          */
/* arguments:              char *fieldname - field to move into, or NULL    */
/*                         int   line      - new cursor line position in    */
/*                                           zero-based field coordinates,  */
/*                                           or -1 for no change            */
/*                         int   column    - new cursor column position in  */
/*                                           zero-based field coordinates,  */
/*                                           or -1 for no change            */
/*                                                                          */
/* return value:           int - RET_OK or ERROR if given bad fieldname     */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Eputcursor moves to the given field, converts the given field        */
/*     cursor coordinates into window coordinates, and puts the cursor      */
/*     at the requested spot.  It returns ERROR if the fieldname is invalid */
/****************************************************************************/

int Eputcursor (char *fieldname, int line, int column)
    {
    WSTRUCT *wp; /* current window */

#ifdef DEBUG
    debug ("Eputcursor:  fieldname = '%s', line = %d, column = %d",
	   fieldname, line, column);
#endif

    if (line >= MAXFLINE)
	return (ERROR);

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    wp = g_wp;

    if (line == ERROR)
	line = w_ftline (wp) + w_line (wp);

    if (column == ERROR)
	column = w_ftcol (wp) + w_col (wp);

    putcurs (line, column);
    return (RET_OK);
    }

/****************************************************************************/
/* Eputformat:  sets tab stops and margins for a field (STUB)               */
/* $$$ stub $$$                                                             */
/*                                                                          */
/* arguments:              char *fieldname  - for which to set tabs &       */
/*                                            margin or NULL for current    */
/*                                            field                         */
/*                         char *formatline - tab and margin rack line      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     This procedure is not yet implemented.  It currently does nothing.   */
/*                                                                          */
/*     When implemented, Eputformat will set the left and right margin      */
/*     and the tab stops of the given field from the given string contain-  */
/*     ing 'l', 'r', 't', and SPACE characters defining tab stops.            */
/****************************************************************************/
/*ARGSUSED*/
void Eputformat (char *fieldname, char *formatline)
     /* field name                 */
    /* tab and margin format line */
    {
#ifdef DEBUG
    debug ("Eputformat:  fieldname = '%s', formatline = '%s'",
	   fieldname, formatline);
#endif
    }

/****************************************************************************/
/* Eputlist:  routine to put a list of lines                                */
/* Returns ERROR if fieldname is invalid                                    */
/*                                                                          */
/* arguments:              char *fieldname - to put list into, or NULL for  */
/*                                           current field                  */
/*                         int   line      - zero-based field line number   */
/*                                           to start at, or -1 for current */
/*                                           line                           */
/*                         POINTER *list   - to put into field              */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Eputlist puts the given list into the field starting at the given    */
/*     line.  It returns ERROR if an invalid fieldname is given.  If the    */
/*     user provided a fieldname, the g_wnode global is set to ERROR.       */
/****************************************************************************/

int Eputlist (char *fieldname, int line, POINTER *list)
    {
    WSTRUCT *wp;        /* current window        */
    ATTR *iline;        /* unpacked form of line */

    int index2;         /* index being written   */
    int   retval;       /* return value          */

#ifdef DEBUG
    debug ("Eputlist:  fieldname = '%s', line = %d",
	   fieldname, line);
    objprint ("Eputlist (list)", list);
#endif

    if ((line + obj_count (list)) > MAXFLINE)
	return (ERROR);

    wp = g_wp;

    if (line == ERROR)
	line = w_ftline (wp) + w_line (wp);

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    if (! checkprot())
	retval = ERROR;
    else
	{
	retval = RET_OK;

	for (index2 = 0; index2 < obj_count (list); index2++)
	    {
	    iline = (ATTR *)unpackup (list [index2], 0);
	    puttext (line + index2, iline);
	    s_free ((char *)iline);
	    }
	}

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (retval);
    }

/****************************************************************************/
/* Eputtext:  puts a line of text into a given field                        */
/*                                                                          */
/* arguments:              char *fieldname - to put text into, or NULL for  */
/*                                           current field                  */
/*                         int   line      - zero-based field line number,  */
/*                                           or -1 for current line         */
/*                         char *text      - to put into field              */
/*                                                                          */
/* return value:           int - RET_OK or ERROR if given invalid fieldname */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode                                    */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Eputtext puts the given text into the field at the given line number */
/*     It returns ERROR if given an invalid fieldname.  If the user provide */
/*     a fieldname, the g_wnode global is set to ERROR.                     */
/****************************************************************************/

int Eputtext (char *fieldname, int line, char *text1)
    {
    WSTRUCT *wp;       /* current window   */
    ATTR    *iline;    /* unpacked form    */
    int     retval;    /* return value     */

#ifdef DEBUG
    debug ("Eputtext:  fieldname = '%s', line = %d, text = '%s'",
	   fieldname, line, text1);
#endif

    if (line >= MAXFLINE)
	return (ERROR);

    wp = g_wp;

    if (line == ERROR)
	line = w_ftline (wp) + w_line (wp);

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    if (! checkprot())
	retval = ERROR;
    else
	{
	retval = RET_OK;
	iline = (ATTR *)unpackup (text1, 0);
	puttext (line, iline);
	s_free ((char *)iline);
	}

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (retval);
    }

/****************************************************************************/
/* Ereadonly:  returns g_readonly flag to helper                            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int TRUE or FALSE value of g_readonly            */
/*                                                                          */
/* globals referenced:     g_readonly                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Ereadonly returns the current value of the g_readonly global to the  */
/*     helper.                                                              */
/****************************************************************************/

int Ereadonly (void)
    {
    return (g_readonly ? TRUE : FALSE);
    }

/****************************************************************************/
/* Ewritefield:   write the current field onto a file                       */
/* Returns ERROR if fieldname is bad or file cannot be opened               */
/*                                                                          */
/* arguments:          char *fieldname - whose contents to write, or NULL   */
/*                                       for current field                  */
/*                     char *filename  - file to write into (append mode)   */
/*                                                                          */
/* return value:       int - RET_OK or ERROR                                */
/*                                                                          */
/* globals referenced: g_wp, g_wnode                                        */
/*                                                                          */
/* globals changed:    g_wp, g_wnode                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     Ewritefield opens the given filename in append mode and appends to   */
/*     the file the contents of the indicated field.  It returns ERROR if   */
/*     the file cannot be opened or the given fieldname is invalid.  If     */
/*     the user specified a fieldname, the g_wnode global is set to ERROR.  */
/****************************************************************************/

int Ewritefield (char *fieldname, char *filename)
    {
    int i;             /* zero based field line */
    int length;        /* length of field       */
    FILE *fp;          /* output file pointer   */
    ATTR  *text2;      /* text of line          */
    char *packed;      /* file version of text  */
    WSTRUCT *wp;       /* current window        */

#ifdef DEBUG
    debug ("Ewritefield:  field = '%s', file = '%s'", fieldname, filename);
#endif

    fp = fopen (filename, "a");

    if (fp == NULL)
	return (ERROR);

    wp = g_wp;

    if (gowin (fieldname) == ERROR)
	return (ERROR);

    length = datalength ();

    for (i = 0; i < length; i++)
	{
	text2 = getli (i);
	packed = packup (text2);
	(void) fprintf (fp, "%s\n", packed);
	s_free ((char *)text2);
	s_free (packed);
	}
    if (fclose (fp) == EOF)
	return (ERROR);

    if (*fieldname)
	{
	g_wp = wp;
	g_wnode = (char *) ERROR;
	}
    return (RET_OK);
    }

/****************************************************************************/
/* gowin:  makes window of given name the current window                    */
/* Returns if fieldname is empty or if already in that field                */
/*                                                                          */
/* arguments:              char *fieldname - field to go into, or           */
/*                                           NULL for current window        */
/*                                                                          */
/* return value:           int RET_OK or ERROR                              */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_wnode                          */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given fieldname is NULL, or if it is the same as the name     */
/*     of the current field, simply return RET_OK immediately.  Otherwise   */
/*     look through the window array for the requested field.  If it        */
/*     is not there, return ERROR.  Otherwise flush the old current window  */
/*     into its field, set g_wnode to ERROR, and set g_wp to the requested  */
/*     window.                                                              */
/****************************************************************************/

int gowin (char *fieldname)
    {
    int i;

#ifdef DEBUG
    debug ("gowin:  fieldname = '%s'", fieldname);
#endif

    if ((*fieldname == '\0') || (seq (fieldname, w_name (g_wp))))
	return (RET_OK);

    for (i = 0; i < obj_count (g_warray); i++) /* find subwindow with name */
	if (seq (w_name (&g_warray [i]), fieldname))
	    break;

    if (i == obj_count (g_warray)) /* if couldn't find window */
	{
#ifdef DEBUG
	debug ("gowin:  cannot find field '%s'", fieldname);
#endif
	return (ERROR);
	}
#ifdef DEBUG
    debug ("gowin:  i = %d", i);
#endif

    flush_window ();
    g_wnode = (char *) ERROR; /* invalidate g_wnode */
    g_wp = & g_warray [i];
    return (RET_OK);
    }

/****************************************************************************/
/* puttext:  replace line using zero based field coordinates                */
/*                                                                          */
/* arguments:              int   line - zero based field line number to rep */
/*                         ATTR  *text - new contents of line               */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the line number is less than zero, return with no further ado.    */
/*     If the line is currently on the screen, free the old cache contents  */
/*     and stuff the new line into the cache.  If the current window is     */
/*     not invariant text, write the line to the file regardless of whether */
/*     it is on screen.  By writing the line immediately, we avoid the      */
/*     possibility of a spurious call to Hmod later.                        */
/****************************************************************************/

void puttext (int line, ATTR *text3)
    {
    WSTRUCT *wp;     /* current window         */
    int wline;       /* zero based window line */

#ifdef DEBUG
    debug ("puttext:  line = %d, text =", line);
    if (text3 && g_debugfp)
	s_print (text3);
#endif

    wp = g_wp;

    if (line < 0)
	return;

    wline = line - w_ftline (wp);

    if (wp != g_warray)       /* write to file unless invariant text */
	writeline(line, text3);

    if (cache_line (wline, text3))       /* written into window cache */
	(wp->w__modflgs) [wline] = 0;   /* mark as clean immediately */
    }

/****************************************************************************/
/* strip: return copy of string w/o ending spaces                           */
/*                                                                          */
/* arguments:              ATTR  *a - the string to be stripped             */
/*                                                                          */
/* return value:           ATTR  *  - the stripped string                   */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Loop backwards through the unstripped string, stopping at the first  */
/*     non-space character.  Return a structured string copy of that port   */
/*     ion of the unstripped string up through the last non-space character */
/****************************************************************************/

ATTR  *strip (ATTR *a)
    {
    int i;
     ATTR  *q;
    int non_space_found = FALSE;

#ifdef CAREFUL
    (void) s_typecheck (a, "strip", T_ATTR);
#endif

#ifdef DEBUG
    debug ("strip: begin");
#endif

    i = (obj_count (a) - 2); 
    for (;;) {
	if (non_space_found = !isattrspace(&a [i]))
	    break;

        if (i <= 0)
            break;

        backupattr_idx(a, &i);
    }

    if (non_space_found)
    { /* positioned on the last non space multi attr */
        skipattr_idx(a, &i);
    }

    q = (ATTR *) s_alloc (i + 1, T_ATTR, NULL);
    (void) i_strncpy((short *)q, (short *)a, i);

#ifdef DEBUG
    debug ("strip: end, i = %d", i);
#endif

    return (q);
    }

/****************************************************************************/
/* Ecopyprofile:  Copy the system profile to a user profile                 */
/*                                                                          */
/* arguments:              char *profile - system profile                   */
/*                         char *newprofile - user profile                  */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     g_e2catd                                         */
/*                                                                          */
/* globals changed:        None                                             */
/*                                                                          */
/* synopsis:                                                                */
/*           Open the profile files                                         */
/*           Copy the system profile extracting message catalog entries as  */
/*           found.  The INeditor.cat will always be used.                  */
/****************************************************************************/

int Ecopyprofile(char *profile, char *newprofile)
{
SFILE *isfp;
SFILE *osfp;
int recno;
POINTER *object;

#ifdef DEBUG
	debug("Ecopyprofile: Default profile '%s', New profile '%s'",profile,newprofile);
#endif
	if (((isfp = sf_open(profile,"r")) == NULL) ||
            ((osfp = sf_open(newprofile,"c")) == NULL))

            return(ERROR);


	for (recno = 0 ; recno < obj_count(sf_records(isfp)) ; recno++)
	{
	    object = (POINTER *)sf_read(isfp,recno);

	    object = catscan(object,g_e2catd,MS_PROFILES);
	    /* Read always from the INeditor.cat MS_PROFILES set */

	    (void) sf_write(osfp,recno,(char *)object);

	}

	sf_close(isfp);
	sf_close(osfp);

	return(1);

}

/****************************************************************************/
/* Egetmessage:  Read a message from the appropriate message catalog        */
/*               do some formatting and return.                             */
/*                                                                          */
/* arguments:    int msg_no                                                 */
/*               char * def_str                                             */
/*               int help_flag                                              */
/*                                                                          */
/*                                                                          */
/* return value:           char *                                           */
/*                                                                          */
/* globals referenced:     g_help_count, g_e2catd, g_helpcatd               */
/*                                                                          */
/* globals changed:        g_help_count                                     */
/*                                                                          */
/* synopsis:                                                                */
/*           Egetmessage gets a message from either the g_e2catd catalog    */
/*           or the g_helpcatd depending on the number of the message.      */
/*           The message retrieved from the catalog is then searched for    */
/*           help text information and reformatted.                         */
/*                                                                          */
/****************************************************************************/

char *Egetmessage (int msg_no, char *def_str, int help_flag)
{

nl_catd current_cat;   /* Will be either the INeditor.cat or a helper .cat */
char *msg_ptr;         /* Message returned from the catalogue              */
char *srch_ptr;        /* Area to search for help infomation               */
int length;
int srch_space;
int cat_set;           /* Message catalogue set to use                     */


#ifdef DEBUG
	debug("Egetmessage: msg_no %d, default message %s, help_flag %d",
			    msg_no,def_str,help_flag);
#endif

	if (msg_no < HELPER_MSG_OFFSET)
	{
	   current_cat=g_e2catd;
	   cat_set = MS_INEDITOR;
	}
        else
	{
	   current_cat=g_helpcatd;
	   cat_set = 1; /* Each helper has consists of one set */
			/* so it is safe to extract using #1   */
	}

	msg_ptr=s_string(catgets(current_cat, cat_set, msg_no, def_str));

#ifdef DEBUG
	debug("Egetmessage: Message returned from catalog '%s'",
	       msg_ptr);
#endif

	if (help_flag == TRUE)
	   return (msg_ptr);

	g_help_count=0;
	length=strlen(msg_ptr);
	srch_ptr=msg_ptr+length;
	srch_space = length - 5;/* Help indicators only in last few chars */

	while ((*srch_ptr != '#') && (length > srch_space))
	{
	      srch_ptr--;
	      length--;
	}

	if (*srch_ptr == '#') /* Message with help text */
	{
              if (isdigit(*(srch_ptr+1)))  /* if a digit follows '#' */  
	            g_help_count= atoi(srch_ptr+1);
	      *srch_ptr = '\0'; /* Remove the '#', digits and any other
	      				trailing chars */
	}
	return(msg_ptr);
}

/****************************************************************************/
/* Esizelines:  Counts the number of lines in  a menu item and the per      */
/*              menu option characters.                                     */
/*                                                                          */
/* arguments:    char * msg_ptr                                             */
/*               int  * item_size                                           */
/*                                                                          */
/*                                                                          */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     None                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*           Esizelines counts the number of lines in a menu,  the number   */
/*           of menu options and the number of characters per menu item.    */
/*           The menu option is passed in via msg_ptr.                      */ 
/*                                                                          */
/****************************************************************************/
int Esizelines (char *msg_ptr,int item_size[])
{
char *tmp_ptr;       /* temporary pointer into msg_ptr */
char *opt_ptr;       /* pointer to current menu option */
int line_count = 1;  /* number of lines in menu */
int menu_opt = -1;   /* number of menu options */
char save_tmp;
int mlen;

#ifdef DEBUG
	debug("Esizelines: message %s ",msg_ptr);
#endif

	tmp_ptr = msg_ptr;

	while (*tmp_ptr != '\0')
	{
	   if (*tmp_ptr == '\n')
	   {
	      /* Count the number of menu options */
	      if (menu_opt >=0)
	      {
		 save_tmp = *tmp_ptr;
		 *tmp_ptr = '\0';
	         item_size[menu_opt] = text_width(opt_ptr);
		 *tmp_ptr = save_tmp;
	      }
	      tmp_ptr++;
	      line_count++;
	      continue;
	   }
	   if (*tmp_ptr == '@')
	   {
	      /* Count the menu options */
	      menu_opt++;
	      *tmp_ptr++ = SPACE;
	      opt_ptr = tmp_ptr;
              continue;
	   }

           mlen = mblen(tmp_ptr, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (mlen < 1) mlen = 1;
           tmp_ptr += mlen;
	}
	if (menu_opt >= 0) /* Just incase a menu option was nul terminated */
	{
	    item_size[menu_opt] = text_width(opt_ptr);
        }

#ifdef DEBUG
	debug("Esizelines: return line count %d",line_count);
#endif
	return(line_count);
}

/****************************************************************************/
/* Ecountlines:  Counts the number of lines in a menu and the number of     */
/*               options or counts the number of lines in a message.        */
/*                                                                          */
/* arguments:    char * msg_ptr                                             */
/*               int * item_count                                           */
/*                                                                          */
/*                                                                          */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     None                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*           Ecountlines counts the number of lines in a menu or message    */
/*           and the number of menu options in a menu.                      */
/*           Search the message for '\n' counting the number of lines       */
/*           Search the string for '@' representing a menu option.          */
/*                                                                          */
/****************************************************************************/
int Ecountlines (char *msg_ptr, int *item_count)
{
char * tmp_ptr;
int line_count = 1;
int menu_opt = 0;
int menu = TRUE;
int mlen;

#ifdef DEBUG
	debug("Ecountlines: message %s item_count %d",msg_ptr,*item_count);
#endif

	if (item_count == NULL) 
	/* A message only is expected */
	    menu=FALSE;

	tmp_ptr = msg_ptr;

	while (*tmp_ptr != '\0')
	{
	   if (*tmp_ptr == '\n')
	   {
	      /* Count the line numbers */
	      line_count++;
	   }
	   if (*tmp_ptr == '@')
	   {
	      /* Count the menu options */
	      menu_opt++;
	   }

           mlen = mblen(tmp_ptr, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (mlen < 1) mlen = 1;
           tmp_ptr += mlen;
	}
	if (menu==TRUE)
	{
            *item_count=menu_opt;
	}
#ifdef DEBUG
  	debug("Ecountlines: line count %d, item count '%d'",line_count,*item_count);
#endif
	return(line_count);
}
