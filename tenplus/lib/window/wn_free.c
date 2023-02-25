#if !defined(lint)
static char sccsid[] = "@(#)90	1.6  src/tenplus/lib/window/wn_free.c, tenplus, tenplus411, GOLD410 3/23/93 12:09:36";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	wn_free
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
 
#include <stdio.h>
#include <database.h>
#include "window.h"

#include <libobj.h>

/****************************************************************************/
/* wn_free: frees a window and recursively frees the following              */
/*          substructures:                                                  */
/*             Wf,WF,WG,Ws,Tabs                                             */
/****************************************************************************/

void wn_free (register WSTRUCT *windows)
    {
    register i;
    register WSTRUCT *wp;
    register count;

#ifdef DEBUG
    debug("wn_free : Called");
#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)windows, "wn_free", T_WSTRUCT );

#endif

    count = obj_count (windows);

    for (i = 0; i < count; i++ )
	{
	wp = &windows [i];

	s_free (w_name (wp));
	s_free (w_zoom (wp));
	s_free ((char *) w_cache (wp));
	s_free (w_tabs (wp));
	s_free (w_pfx (wp));
	s_free (w_sfx (wp));
	s_free (w_filename (wp));
	s_free (w_pickname (wp));
	s_free (w_deftxt (wp));
	s_free ((char *) w_reclocs (wp));
#ifdef VBLSCR
	s_free ((char *) w_modflgs (wp));
#endif
	}
    }

