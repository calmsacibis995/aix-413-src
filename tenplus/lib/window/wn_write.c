#if !defined(lint)
static char sccsid[] = "@(#)95	1.6  src/tenplus/lib/window/wn_write.c, tenplus, tenplus411, GOLD410 3/23/93 12:09:56";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	wn_write
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
/* wn_write: writes a struct window to a file                               */
/****************************************************************************/

int wn_write (register WSTRUCT *windows, register FILE *fp)
    {
    register WSTRUCT *wp;
    register count;

#ifdef CAREFUL
    (void) s_typecheck ((char *)windows, "wn_write", T_WSTRUCT );
#endif

#ifdef DEBUG
/*
    debug ("wn_write:  name = '%s', count = %d, flags = 0%o",
	     obj_name (windows), obj_count (windows), obj_flags (windows));
*/
#endif

    for (count = 0; count < obj_count (windows); count++)
	{
	wp = windows + count;

	(void) puti (w_flags (wp), fp);
	(void) s_write (w_name (wp), fp);
	(void) s_write (w_pfx (wp), fp);
	(void) s_write (w_sfx (wp), fp);
	(void) s_write (w_zoom (wp), fp);
	(void) fputc (w_ttline (wp), fp);
	(void) fputc (w_ttcol (wp), fp);
	(void) fputc (w_tbline (wp), fp);
	(void) fputc (w_tbcol (wp), fp);
	(void) s_write ((char *) w_cache (wp), fp);

	if (w_nxtgang (wp))
	    {
	    (void) fputc (w_nxtgang (wp) - windows, fp);
	    }
	else
	    (void) fputc ('\0', fp);

	(void) s_write (w_tabs (wp), fp);
	(void) fputc (w_lmar (wp), fp);
	(void) fputc (w_rmar (wp), fp);
	(void) s_write (w_filename (wp), fp);
	(void) s_write (w_pickname (wp), fp);
	(void) fputc (w_msgno (wp), fp);
	(void) s_write (w_deftxt (wp), fp);
	}

    /*** if other structure elements are added, put out CONTINUE ***/
    /*** and then write the additional elements.                 ***/

    (void) fputc (STOP, fp);    /* end of the data */

    if (ferror (fp)) /* check for I/O error */
	return (ERROR);

    return (RET_OK);
    }
