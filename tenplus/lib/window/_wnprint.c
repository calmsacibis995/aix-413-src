#if !defined(lint)
static char sccsid[] = "@(#)89	1.6  src/tenplus/lib/window/_wnprint.c, tenplus, tenplus411, GOLD410 7/30/91 14:14:07";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	_wnprint
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <stdio.h>
#include <database.h>
#include "window.h"

#include <libobj.h>

/****************************************************************************/
/* _wnprint:  prints the contents of an individual window                   */
/****************************************************************************/

void _wnprint (register WSTRUCT *wp, register indent)
    {
    register i;
    register char *modflgs;
    register char *tabstring;

    if (g_debugfp == NULL)
	return;

    pindent (indent);
    (void) fprintf (g_debugfp,
	     "name = '%s', flags = 0%o, pfx = '%s', sfx = '%s', zoom = '%s'\n",
	     w_name (wp), w_flags (wp), w_pfx (wp), w_sfx (wp), w_zoom (wp));

    pindent (indent);
    (void) fprintf (g_debugfp, "lmar = %d, rmar = %d\n",
	w_lmar (wp), w_rmar (wp));

    pindent (indent);
    (void) fprintf (g_debugfp, "modflgs: '");
    modflgs = w_modflgs (wp);

#ifdef VBLSCR
    if (modflgs == NULL)
        fprintf(g_debugfp, "is NULL");
    else
    for (i = 0; i < obj_count (modflgs); i++)
#else
    for (i = 0; i < W_SZ; i++)
#endif
	(void) fprintf (g_debugfp, "%c", modflgs [i] ? 'M' : '.');

    (void) fprintf (g_debugfp, "'\n");

    pindent (indent);
    (void) fprintf (g_debugfp,
	     "line = %d, col = %d, lrline = %d, lrcol = %d, ftline = %d, ftcol = %d\n",
	     w_line (wp), w_col (wp), w_lrline (wp), w_lrcol (wp),
	     w_ftline (wp), w_ftcol (wp));

    pindent (indent);
    (void) fprintf (g_debugfp,
	     "ttline = %d, ttcol = %d, tbline = %d, tbcol = %d\n",
	     w_ttline (wp), w_ttcol (wp), w_tbline (wp), w_tbcol (wp));

    pindent (indent);
    (void) fprintf (g_debugfp,
	     "window = 0%o, nxtgang = 0%o, tvloc = 0%o\n",
	     wp, w_nxtgang (wp), w_tvloc (wp));

    pindent (indent);
    (void) fprintf (g_debugfp,
	     "w_filename = '%s', w_pickname = '%s', w_reclocs = 0%o\n",
	     w_filename (wp), w_pickname (wp), w_reclocs (wp));

    if (w_reclocs (wp))
	s_print ((char *)w_reclocs (wp));

    pindent (indent);
    tabstring = w_tabs (wp);

    if (tabstring == NULL)
	(void) fprintf (g_debugfp, "tabs NULL\n");
    else
	(void) fprintf (g_debugfp, "tabs: '%s'\n", tabstring);
    }

