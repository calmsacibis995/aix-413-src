#if !defined(lint)
static char sccsid[] = "@(#)94	1.6  src/tenplus/lib/window/wn_read.c, tenplus, tenplus411, GOLD410 3/23/93 12:09:49";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	wn_read
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
 
#include <stdio.h>
#include <database.h>
#include <window.h>

#include <libobj.h>

/****************************************************************************/
/* wn_read: reads in a struct window from a file                            */
/****************************************************************************/

int wn_read (register WSTRUCT *windows, register FILE *fp)
    {
    register WSTRUCT *wp;
    register count;
		int i;
    int offset;
    int marker;     /* used to read the STOP or CONTINUE marker */

#ifdef CAREFUL
    (void) s_typecheck ((char *)windows, "wn_read", T_WSTRUCT );
#endif

#ifdef DEBUG
/*
    debug ("wn_read:  name = '%s', count = %d, flags = 0%o",
	   obj_name (windows), obj_count (windows), obj_flags (windows));
*/
#endif

    for (count = 0; count < obj_count (windows); count++)
	{

	wp = windows + count;

	wp->w__flags = geti (fp);
	wp->w__name = s_read (fp);
	wp->w__pfx = s_read (fp);
	wp->w__sfx = s_read (fp);
	wp->w__zoom = s_read (fp);
	wp->w__ttline = fgetc (fp);
	wp->w__ttcol = fgetc (fp);
	wp->w__tbline = fgetc (fp);
	wp->w__tbcol = fgetc (fp);
	wp->w__lrline = w_tbline (wp) - w_ttline (wp);
	wp->w__lrcol = w_tbcol (wp) - w_ttcol (wp);
	wp->w__cache = (POINTER *) s_read (fp);


	if (offset = fgetc (fp))
	    wp->w__nxtgang = windows + offset;
	else
	    wp->w__nxtgang = NULL;

	wp->w__tabs = s_read (fp);
	wp->w__lmar = fgetc (fp);
	wp->w__rmar = fgetc (fp);
	wp->w__filename = s_read (fp);
	wp->w__pickname = s_read (fp);
	wp->w__msgno = fgetc (fp);
	wp->w__deftxt = s_read (fp);

	/*** set everything else to NULL ***/

	wp->w__line = 0;
	wp->w__col = 0;
	wp->w__ftline = 0;
	wp->w__ftcol = 0;
	wp->w__reclocs = NULL;
#ifndef VBLSCR
	for (i = 0; i < W_SZ; i++)
	    wp->w__modflgs [i] = '\0';
#else
	wp->w__modflgs = NULL;
#endif
	}

    marker = fgetc (fp); /* read the STOP or CONTINUE marker */

    /*** if more elements are added to WSTRUCT, test  ***/
    /*** for marker == CONTINUE and read them here.   ***/
    /*** Also set up assigning of default values here ***/
    /*** if necessary.                                ***/

    if (marker == STOP)
	{
	if (feof (fp) || ferror (fp)) /* check for I/O error */
	    return (ERROR);

	return (RET_OK);
	}

    return (ERROR); /* STOP or CONTINUE marker was missing */
    }

