#if !defined(lint)
static char sccsid[] = "@(#)06	1.6  src/tenplus/e2/common/gng.c, tenplus, tenplus411, GOLD410 3/23/93 11:50:41";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	cache_line, checkprot, flush_window, getall, getgngs,
 *		getli, movdn, movup, putgng, putli, umod_ok
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
/* File:   gng.c                                                            */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

int umod_ok (int, ATTR *text);

/****************************************************************************/
/* cache_line:  write line into cache if possible                           */
/*                                                                          */
/* arguments:            int  line  - window line number                    */
/*                       ATTR *text - to write into cache                   */
/*                                                                          */
/* return value:         int TRUE or FALSE - whether successfully cached    */
/*                                                                          */
/* globals referenced:   g_wp                                               */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*   If the given line number is in the range of the current window, write  */
/*   the text into the cache and mark it as dirty.  If the window is also   */
/*   not invisible, do a sendbox to put the text on the screen.  Return     */
/*   TRUE if the line was cached, FALSE if it was not.                      */
/****************************************************************************/

int cache_line (int line, ATTR *text1)
    {
    WSTRUCT *wp=g_wp;
    int width = w_lrcol(wp) + w_ftcol(wp) + 1;

#ifdef DEBUG
    debug ("cache_line:  line = %d width = %d", line, width);
#endif

    /* in window cache? */
    if ((line >= 0) && (line <= w_lrline (wp)))
	{

	/* free the existing line and pad out the new line */
	s_free ((char *)w_text (wp, line));
	(wp->w__cache) [line] = (POINTER)i_pad ((short *)text1, width);
	w_setmod (wp, line);

	if (!getstat (w_flags (wp), INVISIBLE))      /* on screen? */
#ifdef VBLSCR
	    sendbox (line, line, 0, min (w_lrcol (wp), CO));
#else
	    sendbox (line, line, 0, w_lrcol (wp));
#endif
	return (TRUE);
	}

    return (FALSE);     /* not cached */
    }

/****************************************************************************/
/* checkprot:  see whether a user can write into a file                     */
/*                                                                          */
/* arguments:            none                                               */
/*                                                                          */
/* return value:         TRUE or FALSE - whether write is allowed           */
/*                                                                          */
/* globals referenced:   g_wp, g_warray, g_readonly                         */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*    checkprot checks the current file protection and issues an error      */
/*    message if it does not permit writing.  It returns TRUE if the user   */
/*    or helper can write in the file, or if the current field is the       */
/*    invariant text; otherwise it returns FALSE.                           */
/****************************************************************************/

int checkprot (void)
    {
    if (g_readonly && g_wp != g_warray)
	 {
	 if (g_readonly == DIR_NOREAD)
	     Eerror (M_EDIRREADONLY,
		    "You must have directory write permission to edit the specified file.","");
	 else
	     Eerror (M_EREADONLY, "Cannot modify this file.  Check permissions.","");
	 return (FALSE);
	 }
    return (TRUE);
    }

/****************************************************************************/
/* flush_window: invoke putgng for all lines of window                      */
/* written in order to save space; but a useful concept in its own right.   */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     flush_window invokes putgng over the entire height of the window,    */
/*     thus ensuring it is entirely flushed.                                */
/****************************************************************************/

void flush_window (void)
    {
	putgng (0, w_lrline (g_wp));
    }

/****************************************************************************/
/* getall:  do getgngs on all windows, one call per gang.                   */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_filename, g_wnode              */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     getall first sets a variable to the original value of g_wp.          */
/*     It then loops through all windows, finding the first member          */
/*     of each gang.  For each gang, it calls getgngs to display            */
/*     the window and its associated gangs.                                 */
/*                                                                          */
/*     If the filename as stored in the form (in g_warray[0]) corresponds   */
/*     to g_filename, getall waits until after the call to getgngs and      */
/*     then clears the reclocs array for the window.  If the filename       */
/*     in the form does not match, the reclocs array is unreliable and      */
/*     is cleared BEFORE the call to getgngs for each gang.                 */
/*                                                                          */
/*     When finished looping through the windows, getall restores           */
/*     g_wp to its original value and invalidates g_wnode.                  */
/****************************************************************************/

void getall (void)
    {
    int i;                  /* window array index               */
    int flag;               /* TRUE if not first window in gang */
    int nameflag;           /* TRUE if filename if form OK      */

    WSTRUCT *oldwp=g_wp;    /* used to save W                   */

    nameflag = seq (g_filename, w_filename (&g_warray [0]));

#ifdef DEBUG
    debug ("getall:  nameflag = %s", nameflag ? "TRUE" : "FALSE");
#endif

    for (i = 1; i < obj_count (g_warray); i++)
	{
	g_wp = &g_warray [i];
	g_wnode = (char *) ERROR;

	flag = FALSE;

	thrugngs
	    if ((g_wp - g_warray) < i)
		flag = TRUE;
	endthrugngs;

	if (flag) /* if not first window */
	    continue;

	if ( !nameflag)
	    {
	    s_free ((char *) w_reclocs (g_wp));
	    g_wp->w__reclocs = NULL;
	    }
	getgngs (0, w_lrline (g_wp)); /* display window and its gangs */

	if (nameflag)
	    {
	    s_free ((char *) w_reclocs (g_wp));
	    g_wp->w__reclocs = NULL;
	    }
	}
    g_wp = oldwp;
    g_wnode = (char *) ERROR;
    }

/****************************************************************************/
/* getgngs:  display a window and all its gangs                             */
/* Uses w_reclocs array if possible                                         */
/*                                                                          */
/* arguments:              int start, stop - first and last lines to read   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_sfp, g_recindex                */
/*                                                                          */
/* globals changed:        g_wp, g_warray                                   */
/*                                                                          */
/* synopsis:                                                                */
/*     getgngs displays the current window together with any other fields   */
/*     ganged to it, using the reclocs array from the window if not null.   */
/*                                                                          */
/*     First, for any window in the gang whose cache is null, set its cache */
/*     to a pointer array filled with nulls.  This simplifies later logic.  */
/*                                                                          */
/*     If the reclocs array is present and the line is not null, see if the */
/*     reclocs entry agrees with the entry in the file's record array.  If  */
/*     it does, the cached line is correct and we don't need to read the    */
/*     line from the file.  If it doesn't, we have to read the line.  If th */
/*     line is read in, set its modification flag to null so that we know   */
/*     that cache line is current.  Note that if the reclocs array has been */
/*     set to null, we have no alternative but to read in the line again.   */
/*                                                                          */
/*     When finished looping through the lines, invoke sendbox for each     */
/*     member of the gang to force a display across the entire width of     */
/*     each window from the starting to stopping line.                      */
/****************************************************************************/

void getgngs (int start, int stop)
    {
    WSTRUCT *wp;        /* current window        */

    int recindex;
    int index1;

    RECORD *reclocs;   /* w_reclocs for field   */
    RECORD *records;   /* sf_records (g_sfp)    */

    reclocs = w_reclocs (g_wp);

#ifdef DEBUG
    debug ("getgngs: start=%d, stop=%d", start, stop);
    debug ("         g_wp = 0%o, window = %d", g_wp, g_wp - g_warray);
    objprint ("getgngs (reclocs)", reclocs);
#endif

    thrugngs
	{
	wp = g_wp;
	if (w_cache (wp) == NULL)
	    wp->w__cache = (POINTER *) s_alloc (w_lrline (wp) + 1,
						T_POINTER, NULL);
	}
    endthrugngs;

    for (index1 = start; index1 <= stop; index1++)
	{
	if (reclocs && ((ATTR *) w_text (g_wp, index1) != NULL))
	    {
	    wp = g_wp;
	    records = sf_records (g_sfp);
	    recindex = index1 + w_ftline (wp);
#ifdef DEBUG
	    debug ("getgngs: line = %d, record = %d, recloc = %ld",
		    index1, recindex, r_fileloc (&reclocs [index1]));
#endif
	    /*
	     * Only check cache for records other than the current one.  Since
	     * the current record may not have been flushed, it may have changed
	     * but not yet have a different index in the records array.  Also,
	     * refreshing the current record is cheap since it is in memory.
	     */
	    if (recindex != g_recindex)
		if (recindex < obj_count (records))
		    {
		    if (r_fileloc(&reclocs[index1]) == 
                                           r_fileloc (&records [recindex]))
			continue;
		    }
	    }

#ifdef DEBUG
	debug ("getgngs:  reading in line %d size %d", index1,
                          w_lrcol(wp)+w_ftcol(wp)+1);
#endif
	thrugngs
	    s_free ((char *) w_text (g_wp, index1));
	endthrugngs;

	thrugngs
	    {
	    wp = g_wp;
	    (wp->w__cache)[index1] = (char *) readline (index1+w_ftline (wp),
                                          w_lrcol(wp)+w_ftcol(wp) + 1);
	    (wp->w__modflgs)[index1] = 0;
	    }
	endthrugngs;
	}

    /* Display the window and gangs.  */
    thrugngs
	sendbox (start, stop, 0, w_lrcol (g_wp));
    endthrugngs;
    }

/****************************************************************************/
/* getli: return the ith line of the current field i=0... from tv or file   */
/*                                                                          */
/* arguments:              int index - zero based field line to get         */
/*                                                                          */
/* return value:           ATTR * - the value of the line                   */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the requested index line is currently on the screen, read the     */
/*     line from the window.  Otherwise read the line out of the field.     */
/*     If the requested line is outside the field boundaries, return the    */
/*     null string.                                                         */
/****************************************************************************/

ATTR  *getli (int index2)
    {
    WSTRUCT *wp; /* current window      */

    wp = g_wp;

#ifdef DEBUG
    debug(" getli: index = %d, length = %d", index2, datalength ());
#endif

    if (( !getstat (w_flags (wp), INVISIBLE)) && /* on screen? */
	(index2 >= w_ftline (wp)) && (index2 <= w_ftline (wp) + w_lrline (wp)))
	return (strip ((ATTR *) w_text (wp, index2 - w_ftline (wp))));

    if ((index2 < 0) || (index2 > datalength ()))
	return ((ATTR *) i_string (0));

    return (readline (index2, 0));
    }

/****************************************************************************/
/* movdn:  move window down in file given number of lines                   */
/*                                                                          */
/* arguments:              int n - the number of lines to move down         */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     For each window ganged to the current window, including the current  */
/*     window itself, invoke putgng to uncache the top part of the window   */
/*     and then invoke tvup to scroll up the screen (i.e. scroll down the   */
/*     window).  When finished scrolling, invoke getgngs to bring onto the  */
/*     screen those lines that have just come into view.                    */
/****************************************************************************/

void movdn (int n)
    {
    WSTRUCT *wp = g_wp;     /* current window */
    int m = n-1;

    if (n <= 0)
	return;

    if (m > w_lrline (wp))
	m = w_lrline (wp);

#ifdef DEBUG
    debug ("movdn:n=%d,m=%d,wl=%d,g_warray=0%o", n, m ,w_lrline (wp), g_warray);
#endif

    thrugngs
	{
	wp = g_wp;
	putgng (0, m);
	wp->w__ftline += n;

        /* scroll wG, tv and screen */
	if (n <= w_lrline (wp))
	    tvup (n, 0, w_lrline (wp)); 
	}
    endthrugngs

    /* redisplay current window and gangs */
    wp = g_wp;
    getgngs (w_lrline (wp) - m, w_lrline (wp));
    }

/****************************************************************************/
/* movup:  move window up in file given number of lines                     */
/*                                                                          */
/* arguments:              int n - the number of lines to move up           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     For each window ganged to the current window, including the current  */
/*     window itself, invoke putgng to uncache the bottom part of the       */
/*     window and then invoke tvdn to scroll the screen down (i.e. scroll   */
/*     the window up).  When finished scrolling, invoke getgngs to bring    */
/*     onto the screen those lines that have just come into view.           */
/****************************************************************************/

int movup (int n)
    {
    WSTRUCT *wp = g_wp;      /* current window */
    int m;

    if (n > w_ftline (wp))
	n = w_ftline (wp);

    if (n <= 0)
	return (0);

    m = n - 1;

    if (m > w_lrline (wp))
	m = w_lrline (wp);

#ifdef DEBUG
    debug ("movup:n=%d,m=%d", n, m);
#endif

    thrugngs
	{
	wp = g_wp;
	putgng (w_lrline (wp) - m, w_lrline (wp));
	wp->w__ftline -= n;

        /* scroll cache, tv and screen */
	if (n <= w_lrline (wp))
	    tvdn (n, 0, w_lrline (wp));
	}
    endthrugngs

    /* redisplay this window and all gangs */
    getgngs (0, m); 
    return (n);
    }

/****************************************************************************/
/* putgng:  write data from array to file for given lines                   */
/*                                                                          */
/* arguments:              int start, stop - limits of lines to put         */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     putgng loops through the lines from start to stop, stopping          */
/*     at each line whose cache entry is marked modified.  It marks         */
/*     each line as clean, and calls writeline as long as Hmod does         */
/*     not reject the change.                                               */
/****************************************************************************/

void putgng (int start, int stop)
    {
    WSTRUCT *wp;         /* current window                        */
    int wline;           /* zero based window line                */
    int fline;           /* zero based field line                 */

#ifdef DEBUG
    debug ("putgng:start=%d,stop=%d", start, stop);
    debug ("putgng:g_wnode=0%o, W=0%o", g_wnode, g_wp);
#endif

    wp = g_wp;

    if (wp == g_warray) /* putgng is unnecessary for invariant text */
	return;

    for (wline = start; wline <= stop; wline++)
	if (w_testmod (wp, wline)) /* if mod bit set for line */
	    {
	    (wp->w__modflgs) [wline] = 0;
	    fline = w_ftline (wp) + wline;
	    if (umod_ok (fline, (ATTR *) w_text (wp, wline)))
		writeline (fline, (ATTR *) w_text (wp, wline));
	    }
    }

/****************************************************************************/
/* putli:  write line to cache or file, perhaps calling Hmod                */
/*                                                                          */
/* arguments:           int   line - 0-based field line number              */
/*                      char *text -                                        */
/*                                                                          */
/* return value:        void                                                */
/*                                                                          */
/* globals referenced:  g_wp                                                */
/*                                                                          */
/* globals changed:     none                                                */
/*                                                                          */
/* synopsis:                                                                */
/*    putli is used by various command handlers to write new lines into the */
/* file.  If the line is currently in the window cache, putli caches it the */
/* and returns.  If it is not in the cache, it calls Hmod if appropriate    */
/* and if Hmod does not reject the change, calls writeline to put it in the */
/* file.                                                                    */
/****************************************************************************/

void putli (int line, ATTR *text2)
    {
    if ( !checkprot ())
	return;

    if (line < 0)
	return;

    if (cache_line (line - w_ftline (g_wp), text2))  /* put into window cache? */
	return;

    if (g_wp == g_warray)
	return;

    if (umod_ok (line, text2))
	writeline (line, text2);
    }

/****************************************************************************/
/* umod_ok:  call Hmod on given line if appropriate, return whether okay    */
/*                                                                          */
/* arguments:            int   fline - field-based line number              */
/*                       ATTR *text - new text to check with Hmod           */
/*                                                                          */
/* return value:         int - FALSE if Hmod rejected the change, else TRUE */
/*                                                                          */
/* globals referenced:   g_wp, g_helper, g_hooks                            */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*    umod_ok checks whether Hmod is relevant in this field and, if not,    */
/* returns TRUE.  If Hmod is relevant, it is called.  If Hmod rejects the   */
/* new line, the old version is restored to the screen and umod_ok returns  */
/* FALSE.  If Hmod accepts the change, umod_ok returns TRUE.                */
/****************************************************************************/

int umod_ok (int fline, ATTR *text3)
    {
    WSTRUCT *wp = g_wp;
    ATTR    *oldtext;             /* old text of same line   */
    int     wline;                /* window line number      */
    char    *ptext;               /* text after packup       */
    char    *poldtext;            /* oldtext after packup    */

    if ((getstat (w_flags (wp), DOUMOD)) && /* if Hmod in field */
	(g_helper) && (g_hooks [UMOD])) /* if helper wants UMOD */
	{
	if (fline < 0 || fline >= datalength ())
	    oldtext = (ATTR *) s_alloc (2, T_ATTR, NULL);
	else
	    oldtext = readline (fline, 0);

	poldtext = packup(oldtext);
	s_free ((char *)oldtext);

	ptext = packup(text3);

	if (Hmod (w_name (wp), fline, poldtext, ptext))
	    {
	    s_free (poldtext);
	    s_free (ptext);

	    /* Hmod may have done an Eputtext - don't rely on oldtext */
	    if (fline < 0 || fline >= datalength ())
		oldtext = (ATTR *) s_alloc (2, T_ATTR, NULL);
	    else
		oldtext = readline (fline, 0);

	    wline = fline - w_ftline (wp);

	    if (cache_line (wline, oldtext))   /* restore old to cache   */
		(wp->w__modflgs) [wline] = 0;  /* old is already in file */

	    s_free ((char *)oldtext);
	    return (FALSE);
	    }
	s_free (poldtext);
	s_free (ptext);
	}
    return (TRUE);     /* Hmod not relevant, or did not reject the change */
    }
