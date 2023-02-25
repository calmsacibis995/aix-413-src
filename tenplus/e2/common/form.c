#if !defined(lint)
static char sccsid[] = "@(#)73	1.8  src/tenplus/e2/common/form.c, tenplus, tenplus411, GOLD410 1/21/94 15:39:43";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Eflushform, Euseform, expand_gangs, del_lines,
 *		del_cols, ins_cols, ins_lines, one_gang, compressform,
 *		expandform, save_reclocs, showform, setupfields
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
/* file:  form.c                                                            */
/*                                                                          */
/* new version of Euseform                                                  */
/****************************************************************************/

#include "def.h"

#define MAXFORMS 2 /* maximum number of forms in the cache */

void compressform (void);
void expandform (void);
void showform (char *);
void setupfields (void);

#ifdef VBLSCR
void ins_lines (WSTRUCT *, int);
void ins_cols (WSTRUCT *, int);
void del_lines (WSTRUCT *, int);
void del_cols (WSTRUCT *, int);
int  one_gang(WSTRUCT *);
void expand_gangs(void);
#endif 

/****************************************************************************/
/* Eflushform:  removes forms of given name from forms cache                */
/*                                                                          */
/* arguments:              char *formname - which form to remove            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_formcache                                      */
/*                                                                          */
/* globals changed:        g_formcache                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     Eflushform loops through the elements of the g_formcache global      */
/*     array, deleting each element whose name is the same as the given     */
/*     name.                                                                */
/****************************************************************************/

void Eflushform (char *formname)
    {
    int i;

#ifdef DEBUG
    debug ("Eflushform:  formname = '%s'", formname);

    for (i = 0; i < obj_count (g_formcache); i++)
	debug ("    [%d] = '%s' (file '%s')", i, obj_name (g_formcache [i]),
	       w_filename ((WSTRUCT *) g_formcache [i]));
#endif

    i = 0;
    while (i < obj_count (g_formcache))
	if (seq (formname, obj_name (g_formcache [i])))
	    {
	    s_free (g_formcache [i]);
	    g_formcache = (POINTER *) s_delete ((char *)g_formcache, i, 1);
	    }
	else
	    i++;

#ifdef DEBUG
    debug ("  after flush:");
    for (i = 0; i < obj_count (g_formcache); i++)
	debug ("    [%d] = '%s' (file '%s')", i, obj_name (g_formcache [i]),
	       w_filename ((WSTRUCT *) g_formcache [i]));
#endif
    }

/****************************************************************************/
/* Euseform: opens a form file, sets up window for it.                      */
/* Uses cached forms if possible.                                           */
/*                                                                          */
/* arguments:              char *formname - name of form to use             */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     g_formcache, g_errno, g_warray, g_filename,      */
/*                         g_fakefile, g_formname, g_curdir                 */
/*                                                                          */
/* globals changed:        g_formcache, g_warray, g_errno                   */
/*                                                                          */
/* synopsis:                                                                */
/*     Euseform opens the named form, re-using it from the cache if         */
/*     possible.  First, it flushes the current window and, if necessary,   */
/*     initializes the forms cache.                                         */
/*                                                                          */
/*     If the requested form name is the same as g_formname, and the file   */
/*     subfield of the current form agrees with g_filename, then we are     */
/*     home free - invoke showform and return immediately.                  */
/*                                                                          */
/*     If the requested form name is in the cache, and its cache entry      */
/*     filename agrees with g_filename, we can re-use a form from the       */
/*     cache.  To do so, perform a complicated little dance in which we     */
/*     do the following:                                                    */
/*         compress the current form                                        */
/*         temporarily append the existing window array to the forms cache  */
/*             (or, if the new formname is the "*win*" pseudo-form,         */
/*             simply discard the existing window array)                    */
/*         set the new window array to the updated forms cache              */
/*         invoke expandform to expand the form in the window array [0]     */
/*         discard the element from the form cache in which we found        */
/*             the form we just expanded                                    */
/*         invoke showform to cause the display of the new form             */
/*         return RET_OK.                                                   */
/*                                                                          */
/*     Otherwise we have to get the form from scratch.  To do so,           */
/*         use findfile to find the file corresponding to the desired form  */
/*         read the form from the file and check it for validity            */
/*             to be valid, the form must be readable and have the          */
/*             correct type, must have at least two elements, and,          */
/*             in the case where we are editing a fake file, have           */
/*             exactly two elements (the invariant text and the field).     */
/*         again, temporarily append the existing window array to the cache */
/*             (or, if the new formname is one of the "*win*" or            */
/*             "*wrongsize*" pseudo-forms, simply discard the existing      */
/*              window array)                                               */
/*         if the cache is getting too large, delete its oldest element     */
/*         mark the form with the current g_filename for later reference    */
/*         fix filenames in window array for abspaths forms and set         */
/*            and set the W_ABSPATHS bit.                                   */
/*         set g_warray to the new form and display it                      */
/*         finally, return RET_OK.                                          */
/*                                                                          */
/*     If any stage of this process fails, set the g_errno global to a      */
/*     code describing the nature of the problem and then return ERROR.     */
/****************************************************************************/

int Euseform (char *formname)
    {
    int i;                 /* used in debug printout  */
    char *filename;        /* full file name for form */
    char *rawline;         /* invtext line before unpacking */

    SFILE *sfp;            /* sfile object for form   */
    WSTRUCT *newform;      /* window array from file  */
    WSTRUCT *wp;           /* used to traverse windows*/


    flush_window (); /* for safety */

    if (g_formcache == NULL) /* if first time */
	g_formcache = (POINTER *) s_alloc (0, T_POINTER, "g_formcache");

#ifdef DEBUG
    debug ("Euseform: called w/ formname = '%s', g_formname = '%s'",
	formname, g_formname);
    debug ("there are %d forms in the cache:", obj_count (g_formcache));

    for (i = 0; i < obj_count (g_formcache); i++)
	debug ("    [%d] = '%s' (file '%s')", i, obj_name (g_formcache [i]),
	       w_filename ((WSTRUCT *) g_formcache [i]));
#endif

    /* if same form, reuse current form */
    if (seq (formname, g_formname) &&
	seq (w_filename (&g_warray [0]), g_filename))
	{
	showform (formname);
	return (RET_OK);
	}

    /* see if form is in the cache */
    for (i = 0; i < obj_count (g_formcache); i++)
	if (seq (formname, obj_name (g_formcache [i])) &&
	    seq (w_filename ((WSTRUCT *) g_formcache [i]), g_filename))
	    break;

#ifdef DEBUG
    debug ("Euseform:  i = %d, obj_count = %d", i, obj_count (g_formcache));
#endif

    if (i < obj_count (g_formcache)) /* get form from cache */
	{
	if (( !obj_getflag (g_warray, W_ABSPATHS)) && /* if normal form */

	    /* if this isn't a divided text screen */
	    ( !seq (g_formname, "*win*")) &&
	    /* and the window hasn't just changed size */
	    ( !seq (g_formname, WRONGSIZEFORM)))
	    {
	    compressform ();
	    g_formcache = s_append ((POINTER *)g_formcache, (char *)g_warray);
	    }
	else
	    s_free ((char *) g_warray);

	g_warray = (WSTRUCT *) g_formcache [i];
	expandform ();
	g_formcache = (POINTER *) s_delete ((char *)g_formcache, i, 1);
	showform (formname);
	return (RET_OK);
	}

    /* try to find form using form search path */
    filename = findfile (formname, FORMEXTENSION, g_formpath);
    if (filename == NULL) /* not found */
	{
	/* try to find form using default form search path */
	filename = findfile (formname, FORMEXTENSION, g_def_formpath);
	if (filename == NULL) /* can't be found */
	    {
	    g_errno = E_NOFILE;
	    return (ERROR);
	    }
	}
#ifdef DEBUG
    debug ("Euseform:  found form '%s' as file '%s'", formname, filename);
#endif

    /* try to read in new form */
    sfp = sf_open (filename, "r");
    if (sfp == (SFILE *) ERROR)
	{
	s_free (filename);
	return (ERROR); /* Note:  g_errno already set */
	}
    newform = (WSTRUCT *) sf_read (sfp, 0); /* get window array from file */
    (void) sf_close (sfp);

    if (newform == (WSTRUCT *) ERROR)
	{
	s_free (filename);
	g_errno = E_NOFORM;
	return (ERROR);
	}
    if ((obj_type (newform) != T_WSTRUCT) ||
	(obj_count (newform) < 2))
	{
	g_errno = E_BADFORM;
	s_free (filename);
	s_free ((char *) newform);
	return (ERROR);
	}

    /* if ASCII file, make sure the form is OK */
    if ((g_fakefile) &&
	((obj_count (newform) != 2) ||
	 (w_flags (&newform [1]) & INDEXED) ||
	 (w_pfx (&newform [1]))))
	{
	g_errno = E_NOFILE;
	s_free (filename);
	s_free ((char *) newform);
	return (ERROR);
	}

    /* found new form and it's OK
       and it isn't the multi-window form */

    if (g_formname &&
	( !obj_getflag (g_warray, W_ABSPATHS)) && /* if normal form */
	/* if this isn't a divided text screen */
	( !seq (g_formname, "*win*")) &&

	/* and the window hasn't just changed size */
	( !seq (g_formname, WRONGSIZEFORM)))
	{
	compressform ();
	g_formcache = s_append ((POINTER *)g_formcache, (char *)g_warray);
	}
    else
{
	s_free ((char *) g_warray);
}
    if (obj_count (g_formcache) > MAXFORMS) /* trim cache if too large */
	{
	s_free (g_formcache [0]);
	g_formcache = (POINTER *) s_delete ((char *)g_formcache, 0, 1);
	}

    s_newname ((char *) newform, formname);  /* fix names in form */

    /* save path of form in warray->w__zoom */
    newform [0].w__zoom = filename; /* note we don't want to free filename */

    s_free (w_filename (&newform [0]));
    newform [0].w__filename = s_string (g_filename);
    newform [0].w__name = s_string ("InvTxt");

#ifdef VBLSCR
    g_warray = newform; 
    /* note screen size */
    g_warray->w__LI = LI;
    g_warray->w__CO = CO;

    /* standard form must be dynamically resized */
    /* Vertically expand ganged fields and resize single-field forms. */

    expand_gangs ();

    /* allocate the line modification flags. may be ok to start at 1 since the
       mod flags for the invariant text window shouldn't be used. */

    for (i=0; i<obj_count (newform); i++)
	newform[i].w__modflgs = s_alloc (w_lrline (newform+i)+1, T_CHAR, NULL);
#endif

    /* Also, the forms compiler stores the invariant text as char *, not
       ATTR *, so we have to unpack it now too. */

    wp = &newform [0];
    for (i = 0; i < obj_count (w_cache (wp)); i++)
	{
	rawline = w_text (wp, i);
	wp->w__cache [i] = (char *) unpackup (rawline, 0);
	s_free (rawline);
	}

#ifndef VBLSCR
    g_warray = newform; /* display form */
#endif

    showform (formname);

#ifdef DEBUG
    objprint ("Euseform (g_warray)", g_warray);
    objprint ("Euseform (g_formcache)", g_formcache);
#endif
    return (RET_OK);
    }

#ifdef VBLSCR
/*****************************************************************************
* expand_gangs: When the display medium has more lines than the form,        *
*		vertically expand single-field forms and forms with 	     *
*		horizontally ganged fields to take advantage of the extra    *
*		room.							     *
*                                                                            *
* arguments:              none						     *
*                                                                            *
* return value:           void						     *
*                                                                            *
* globals referenced:     g_warray, LI 					     *
*                                                                            *
* globals changed:        g_warray					     *
*                                                                            *
* synopsis:                                                                  *
*	Vertically expand horizontally ganged fields.			     *
*	Extra lines = # lines on screen (or window) - # lines in form.	     *
*	If extra lines > 0, distribute among ganged fields.		     *
*	Find total # lines in ganged fields in form and gang with 	     *
*	largest # lines.  Ignore gangs which are on the same line.	     *
*	If total > 0, expand ganged fields.				     *
*	Distribution for each gang is:	     				     *
*		    #lines in gang					     *
*         n =	---------------------  	*  extra lines			     *
*		 total lines in gangs					     *
*									     *	
*	Go through all fields:						     *
*		For each gang to be expanded:				     *
*		   Insert n copies of the last invariant text line of then   *
*			ganged field.					     *
*		   Fix up the # lines in the field (w__lrline) for each	     *
*			member of the gang				     *
*		Fix up the top and bottom screen(tv) line coordinates	     *
*		   for the field to adjust for the added lines.		     *
*	If there is any remainder left after this distribution, give it	     *
*	to the largest gang.						     *
*****************************************************************************/

void expand_gangs(void)
    {
    WSTRUCT *wp;
    WSTRUCT *largest;
    WSTRUCT **sptr, **single;

    int ins, n, total;
    int	lmax, lines, elines, cols, count;

#ifdef DEBUG
	debug("expand_gangs: called");
#endif

    lines = LI - (w_lrline (&g_warray[0]) + 1);
    if ((count = obj_count (g_warray)) == 2)
	{
	/* single field form */
	if (lines > 0)
	    ins_lines (&g_warray[1], lines);	
	else if (lines < 0 &&
	    w_tbline (&g_warray[1]) - w_ttline (&g_warray[1]) >= -lines)
	    del_lines (&g_warray[1], -lines);	

	cols = CO - (w_lrcol (&g_warray[0]) + 1);
	if (CO > w_lrcol (&g_warray[0]) + 1)
	    ins_cols (&g_warray[1], cols);
	else if (CO < w_lrcol (&g_warray[0]) + 1 &&
	    w_tbcol (&g_warray[1]) - w_ttcol (&g_warray[1]) >= -cols)
	    del_cols (&g_warray[1], -cols);
	return;
	}
    if (lines <= 0)
	return;
    /*  See if there are ganged fields which can be expanded. */
    lmax    = total = 0;
    largest = NULL;
    sptr    = single = (WSTRUCT **)calloc (count-1, sizeof (WSTRUCT*));
    for (wp = &g_warray[1]; wp < &g_warray[count]; wp++) {
	if (one_gang(wp)) {
	    n = wp->w__lrline + 1;
	    total += n;
	    if (n > lmax) {
		lmax = n;
		largest = wp;
	    }
	    *sptr++ = wp;	/* save for use when expanding */
	}
    }
#ifdef DEBUG
    debug ("expand_gangs:  LI=%d, form lines=%d, extra lines=%d, gang lines=%d",
	    LI, g_warray[0].w__lrline +1, lines, total);
#endif
    if (total == 0) {
	free ((char *) single);
	return;
    }
    elines = lines;
    ins = 0;
    for (wp = &g_warray[1]; wp < &g_warray[count]; wp++) 
	{
	wp->w__ttline += ins;
	wp->w__tbline = w_ttline (wp) + w_lrline (wp);
	if (elines <= 0) 
	    continue;
	for (sptr = single; *sptr; sptr++) 
	    {
	    if (*sptr == wp) {
		/* is only leftmost gang member on line -- insert proportional # of lines */
		n = ((w_lrline (wp) + 1) * lines)/total;
		if (elines < n)
		    n = elines;
		elines -= n;
		ins_lines (wp, n);
		ins += n;
		break;
	    } 
	    }
#ifdef DEBUG
	debug("		index=%d, ttline=%d, tbline=%d",wp - g_warray, 
	wp->w__ttline, wp->w__tbline);
#endif
	}
    if ((n = lines - ins) > 0) {	/* give remainder to largest gang */
	ins_lines (largest, n);
	for ( wp = largest + 1; wp < g_warray + count; wp++) {
	    wp->w__ttline += n;
	    wp->w__tbline = w_ttline (wp) + w_lrline (wp);
#ifdef DEBUG
	    debug("		index=%d, ttline=%d, tbline=%d",
		wp - g_warray, wp->w__ttline, wp->w__tbline);
#endif
	}
    }
    free ((char*)single);
    }


/*****************************************************************************
* del_lines: Delete the given # of lines in the invariant text cache.        *
*                                                                            *
* arguments:              WSTRUCT *wp - field from which to remove lines of  *
*			  invariant text				     *
*			  int       nlines - number of lines to delete	     *
*                                                                            *
* return value:           void						     * 
*                                                                            *
* globals referenced:     g_warray 					     *
*                                                                            *
* globals changed:        g_warray					     *
*                                                                            *
* synopsis:                                                                  *
*     Deletes nlines of invariant text.  Has to fix up the number of         *
*     lines in the form.                                                     *
*****************************************************************************/

void del_lines (WSTRUCT *wp, int nlines)

{
    g_warray->w__cache = (POINTER *) s_delete ((char *)w_cache (g_warray),
	w_tbline (wp) - nlines, nlines);

    g_warray[0].w__lrline -= nlines;
    g_warray[0].w__tbline -= nlines;
    wp->w__lrline -= nlines;
    wp->w__tbline -= nlines;
}

/*****************************************************************************
* del_cols: Delete the given # of columns in the invariant text cache.       *
*                                                                            *
* arguments:              WSTRUCT *wp - field from which to remove columns of*
*			  invariant text				     *
*			  int       ncols - number of columns to delete	     *
*                                                                            *
* return value:           void						     * 
*                                                                            *
* globals referenced:     g_warray 					     *
*                                                                            *
* globals changed:        g_warray					     *
*                                                                            *
* synopsis:                                                                  *
*     Deletes ncol columns of invariant text.  Has to fix up the number of   *
*     columns in the form.                                                   *
*****************************************************************************/

void del_cols (WSTRUCT *wp, int ncols)
{
    int i;

    for (i = 0; i < LI; i++) 
	g_warray->w__cache[i] = (POINTER) s_delete (w_text (g_warray, i),
	    w_tbcol (wp) - ncols, ncols);

    /* update the cursor column position */
    g_warray[0].w__lrcol -= ncols;
    g_warray[0].w__tbcol -= ncols;
    wp->w__lrcol -= ncols;
    wp->w__tbcol -= ncols;
}

/*****************************************************************************
* ins_cols: Insert the given # of columns into the invariant text cache      *
*		and assign them duplicates of the last column of the given   *
*		field's invariant text.					     *
*                                                                            *
* arguments:              WSTRUCT *wp - field to receive extra columns of    *
*			  invariant text				     *
*			  int       ncols - number of columns to insert	     *
*                                                                            *
* return value:           void						     * 
*                                                                            *
* globals referenced:     g_warray 					     *
*                                                                            *
* globals changed:        g_warray					     *
*                                                                            *
* synopsis:                                                                  *
*     Inserts ncol new columns of invariant text.  Has to fix up the number  *
*     of columns in the form.                                                *
*****************************************************************************/

void ins_cols (WSTRUCT *wp, int ncols)

{
      int i, j;
  
      for (i = 0; i < LI; i++) {
  
#ifdef DEBUG
          debug ("ins_cols: i = %d, g_warray->w__cache[i] = %s", i,
                     g_warray->w__cache[i]);
#endif 
  
  	g_warray->w__cache[i] = (POINTER) s_insert (w_text (g_warray, i),
  	    w_tbcol (wp) + 1, ncols);
  	for (j = 0; j < ncols; j++)
  	    g_warray[0].w__cache[i][w_tbcol (wp) + j + 1] =
  		g_warray->w__cache[i][w_tbcol (wp)];
      }

    /* update the cursor column position */
    g_warray[0].w__lrcol += ncols;
    g_warray[0].w__tbcol += ncols;
    wp->w__lrcol += ncols;
    wp->w__tbcol += ncols;
}

/*****************************************************************************
* ins_lines: Insert the given # of lines into the invariant text cache       *
*		and assign them duplicates of the last line of the given     *
*		field's invariant text.					     *
*                                                                            *
* arguments:              WSTRUCT *wp - field to receive extra lines of      *
*			  invariant text				     *
*			  int       n - number of lines to insert	     *
*                                                                            *
* return value:           void						     * 
*                                                                            *
* globals referenced:     g_warray 					     *
*                                                                            *
* globals changed:        g_warray					     *
*                                                                            *
* synopsis:                                                                  *
*     Inserts n new lines of invariant text.  Has to fix up the number of    *
*     lines in each field in the gang.  Also subtracts n from the            *
*     beginning screen lines for other fields in the gang since later	     * 
*     these will be adjusted for all lines added before them.		     *
*****************************************************************************/

void ins_lines (WSTRUCT *wp, int n)

    {
    WSTRUCT   *t;
    int       start,stop;

#ifdef DEBUG
    debug ("ins_lines:  %d lines inserted; duplicating line %d", n,
	    wp->w__tbline);
#endif
    g_warray->w__cache = (POINTER *)s_insert ((char *)w_cache (g_warray),
		         w_tbline (wp) + 1, n);
    start = wp->w__tbline + 1;
    stop = start + n;
    for (; start < stop; start ++) {
	g_warray[0].w__cache[start] = (char*)s_string (g_warray->
				      w__cache[wp->w__tbline]);
    }
    g_warray[0].w__lrline += n;
    g_warray[0].w__tbline += n;
    t = wp;
    /*  Fix # lines in this field and in any gang members'. */
    do {
	t->w__lrline += n;
	/*  Have to subtract n since it will be included in the 
	 *  total # lines inserted thus far in the form, which is added 
	 *  to each field's beginning screen line.  Do not do this for
	 *  wp, the leftmost gang member, since its total has already
	 *  been incremented.
	 */
	if (t != wp)
	    t->w__ttline -= n;
    } while ((w_nxtgang(t) != (WSTRUCT*) NULL) 
	    && ((t = w_nxtgang (t)) != wp));
    wp->w__tbline += n;
    }


/*****************************************************************************
* one_gang:  determines whether the given field is the leftmost gang      *
*		member (visible) and is on a line on which there are no      *
*		other gangs						     *
*                                                                            *
* arguments:              WSTRUCT *gwp - field				     *
*                                                                            *
* return value:           int TRUE or FALSE - whether meets above criteria   *
*                                                                            *
* globals referenced:     g_warray 					     *
*                                                                            *
* globals changed:        none						     *
*                                                                            *
* synopsis:                                                                  *
*     First determines that the given field is the leftmost member of a      *
*     gang and then loops through the fields to see if there is another      *
*     leftmost gang member on the same line.  Can quit looking after we      *
*     find a field on a line > the starting line of the given field.         *
*****************************************************************************/

int one_gang (WSTRUCT *gwp)
    {
    WSTRUCT *wp;
    int     i;
    
    /*  If this is not a (visible) leftmost gang member, forget it. */
    if (getstat (w_flags (gwp), INVISIBLE) 
	|| !w_nxtgang(gwp) 
	|| !getstat (w_flags(gwp), HGANGED))
	return FALSE;
    for (i = 1, wp = &g_warray[1];
	(w_ttline (wp) <= w_ttline (gwp)) && (i < obj_count (g_warray));
	 i++, wp = &g_warray[i]) {
	if (wp == gwp) {
	    continue; 
	}
	/* if another leftmost gang member on same line, return FALSE. */
	if (w_nxtgang (wp) && getstat (w_flags (wp), HGANGED) 
	    && (w_ttline (wp) == w_ttline (gwp))) {
	    return FALSE;
	}
    }
    return TRUE;
    }
#endif

/****************************************************************************/
/* compressform:  removes excess information from current form              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     compressform is used to remove excess information from the           */
/*     current form.  It returns immediately if there is no g_sfp, since    */
/*     there is nothing to do in this case.                                 */
/*                                                                          */
/*     First, free all caches where save_reclocs hasn't constructed a       */
/*     reclocs array.  These will be fields that have a prefix at the       */
/*     record level of the file, or all fields below the record level.      */
/*                                                                          */
/*     Finally, compress the invariant text window itself, in g_warray[0].  */
/*     To do so, retain only those cache elements that differ from the      */
/*     previous one - i.e. set all duplicate cache lines to null.           */
/*     expandform() will re-create these duplicate lines when the form is   */
/*     once again used from the forms cache.                                */
/****************************************************************************/

void compressform (void)
    {
    int i;
    WSTRUCT *wp;
    POINTER *cache;

#ifdef DEBUG
    debug ("compressform:  form = '%s', flag = 0%o", obj_name (g_warray),
	   obj_flag (g_warray));
    debug ("    g_filename = '%s'", g_filename);
#endif

    if (g_sfp == NULL)
	return;

    for (i = 1; i < obj_count (g_warray); i++)
	{
	wp = &g_warray [i];

	/* Free caches where the reclocs haven't been saved. */
	if (w_reclocs (wp) == NULL)
	    {
#ifdef DEBUG
	    debug ("compressform: freeing cache for window %d", i);
#endif
	    s_free ((char *) w_cache (wp));
	    wp->w__cache = NULL;
	    }
	}

    /* Compress the invariant text */
    cache = w_cache (&g_warray [0]);
    for (i = obj_count (cache) - 1; i > 0; i--)
	{
	if (i_seq ((short *)cache [i], (short *)cache [i - 1]))
	    {
	    s_free (cache [i]);
	    cache [i] = NULL;
	    }
	}
    }

/****************************************************************************/
/* expandform:  expands the invariant text of current form                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_warray                                         */
/*                                                                          */
/* globals changed:        g_warray                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     expandform expands the invariant text in g_warray[0].                */
/*     To expand the invariant text, it loops forward through the           */
/*     zero'th window, replacing every null line with a copy of the         */
/*     contents of the previous line.                                       */
/****************************************************************************/

void expandform (void)
    {
    int i;
    POINTER *cache;

#ifdef DEBUG
    debug ("expandform:  form = '%s'", obj_name (g_warray));
    objprint ("expandform (g_warray)", g_warray);
#endif

    cache = w_cache (&g_warray [0]);

    for (i = 1; i < obj_count (cache); i++)
	if (cache [i] == NULL)
	    cache [i] = (char *) (ATTR *) i_string ((short *)cache [i - 1]);

#ifdef DEBUG
    objprint ("expandform (cache of g_warray [0]) after expansion",
	      w_cache (&g_warray [0]));
#endif
    }

/****************************************************************************/
/* save_reclocs: save file locations of cached lines in w__reclocs.         */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     save_reclocs saves the file locations from the records array for the */
/*     lines in the w__cache in w__reclocs, for each window in the form.    */
/*     This routine is called when the form is about to change, and thus be */
/*     put into the global forms cache.  By saving the file locations, we   */
/*     will be able to tell if the cache is still accurate if we return to  */
/*     this form, and thus save reading the file again by simply displaying */
/*     what is already in w__cache.  This reusing of the cache requires the */
/*     cooperation of getgngs().                                            */
/*                                                                          */
/*     This is only relevant if we are at the record level of the file      */
/*     (g_spath is null), and the file contains homogeneous records (for    */
/*     each window w__pfx is null).  This is true because below the record  */
/*     level record caching will save us the file I/O, and prefixed fields  */
/*     at the record level are rare enough that it doesn't make sense to    */
/*     optimize for them.                                                   */
/*                                                                          */
/*     Note that we also only do this if the form is not ABSPATHS and is    */
/*     not the *win* form, since these are not cached anyway.               */
/****************************************************************************/

void save_reclocs (void)
    {
    int     i;
    int     number;
    int     line;
    char    *pfx;

    WSTRUCT *wp;
    RECORD  *reclocs;
    RECORD  *records;

    if ((g_spath && *g_spath) ||
	(g_formname == NULL) || (obj_getflag (g_warray, W_ABSPATHS)) ||
	(seq (g_formname, "*win*")))
	return;

    records = sf_records (g_sfp);

    for (i = 1; i < obj_count (g_warray); i++)
	{
	wp = &g_warray [i];
	pfx = w_pfx (wp);

	if (pfx && *pfx)
	    /* Skip prefixed fields. */
	    continue;

	if (((w_flags (wp) & (INDEXED | HGANGED)) == (INDEXED | HGANGED)) ||
	    ( !(w_flags (wp) & INDEXED)))
	    {
	    /*
	     * Field has no prefix, but is either the leftmost member of a gang
	     * or is not indexed.  In this case we want to save the reclocs for
	     * all the lines in the cache, each of which is a separate record.
	     * Note that we can save the reclocs for the entire gang in the
	     * leftmost member's window, since if any field in the gang changes
	     * the whole record will move in the file.
	     */
#ifdef DEBUG
	    debug ("save_reclocs: saving reclocs for window %d, flags = 0%o",
		    i, w_flags (wp));
#endif
	    if (w_reclocs (wp) == NULL)
		wp->w__reclocs = (RECORD *) s_alloc (w_lrline (wp) + 1,
							T_RECORD, NULL);
	    reclocs = w_reclocs (wp);

	    for (line = 0; line <= w_lrline (wp); line++)
		{
		number = line + w_ftline (wp);

		if (number < obj_count (records))
		    reclocs [line].r__fileloc = r_fileloc (&records [number]);
		else
		    reclocs [line].r__fileloc = -1L;
		}
	    }
	else
	    /*
	     * Field is indexed, but not leftmost.  The reclocs are in the
	     * leftmost member, but we need to keep this field's cache also.
	     * Therefore, we allocate an empty reclocs array to tell
	     * compressform not to free the cache.
	     */
	    wp->w__reclocs = (RECORD *) s_alloc (0, T_RECORD, NULL);
	}
    }

/****************************************************************************/
/* showform:  causes display of new form                                    */
/*                                                                          */
/* arguments:              char *formname - name of form to show            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_formname, g_warray, g_wp, g_dispflag, g_wnode  */
/*                                                                          */
/* globals changed:        g_formname, g_wp, g_dispflag, g_wnode            */
/*                                                                          */
/* synopsis:                                                                */
/*     showform causes the named form to be displayed.                      */
/*                                                                          */
/*     use sendbox to send the entire window, including invariant text.     */
/*     Then set g_formname to the given name, invoke getall to display      */
/*     all the other windows, set g_wp to the first ( NOT zero'th) field,    */
/*     and set g_wnode to ERROR so that the display will work properly.     */
/****************************************************************************/

void showform (char *formname)
    {
    WSTRUCT *wp;
    int i;

    wp = g_warray;
    g_wp = wp;

#ifdef VBLSCR
    if ((w_lrline (wp) < LI -2) || (w_lrcol (wp) < CO - 1))
	{   /* form is smaller than display, need to blank excess */
	int inum;
	Si_dirty (0, LI - 2);
	for (inum = 0; inum <= LI-2; inum++)
	    {
	    i_smear ((short)(SPACE | FIRST), CO, (short *)g_goalscreen[inum]);
	    g_goalcrc[inum] = 0;
	    }
	}
#endif

    /* Reserve line for tab rack on SHOWTABS-type forms */
    sendbox ((getstat (w_flags (wp), SHOWTABS)) ? 1 : 0,
		      w_lrline (wp), 0, w_lrcol (wp));

    /***** fix up the globals *****/

    s_free (g_formname); /* remember form name in global */
    g_formname = s_string (formname);

    setupfields ();

    getall (); /* display all other windows */

    /* Search for the first window without CATENTRY */

    for ( i = 1; i < obj_count(g_warray); i++)
    {
       g_wp = &g_warray[i];
       if (!getstat (w_flags(&g_warray[i]),CATENTRY))
	  break;
    }


    g_wnode = (char *) ERROR;    /* invalidate g_wnode */
    }


/****************************************************************************/
/* setupfields:  sets up the g_fieldlist global                             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           none                                             */
/*                                                                          */
/* globals referenced:     g_warray, g_fieldlist                            */
/*                                                                          */
/* globals changed:        g_fieldlist                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     setupfields scans the current window arrays and sees if all fields   */
/*     are indexed w/ no prefix.  If this is true it looks at the suffix    */
/*     for each field and generates a list of fields for partial record     */
/*     reading.  It does nothing if the current editing location is not     */
/*     the top of the file.  If the form is not of the correct type, it     */
/*     sets g_fieldlist to an empty pointer array.                          */
/****************************************************************************/

void setupfields (void)
    {
    int i;
    WSTRUCT *warray;
    WSTRUCT *wp;

#ifdef DEBUG
    debug ("setupfields: g_spath = 0%o (%s)", g_spath, g_spath);
#endif

    warray = g_warray;

    if (g_spath && *g_spath) /* if not at top of file */
	return;

    s_free ((char *)g_fieldlist);
    g_fieldlist = NULL;

    for (i = 1; i < obj_count (warray); i++)
	{
	wp = &warray [i];

	/***** make sure field is indexed w/ no prefix and has a suffix *****/

	if (((w_flags (wp) & INDEXED) == 0) ||
	    (w_pfx (wp) && *w_pfx (wp)) ||
	    (w_sfx (wp) == NULL) || (*w_sfx (wp) == '\0'))
	    return;
	}
    g_fieldlist = (POINTER *) s_alloc (0, T_POINTER, NULL);

    for (i = 1; i < obj_count (warray); i++)
	{
	wp = &warray [i];

	g_fieldlist = s_append (g_fieldlist, firstnode (w_sfx (wp)));
	}
#ifdef DEBUG
    debug ("g_fieldlist:");

    for (i = 0; i < obj_count (g_fieldlist); i++)
	debug ("fieldlist [%d] = '%s'", i, g_fieldlist [i]);
#endif
    }
