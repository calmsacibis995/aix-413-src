#if !defined(lint)
static char sccsid[] = "@(#)37	1.5  src/tenplus/lib/obj/sf_stindx.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:30";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_setindex
 * 
 * ORIGINS:  9, 10, 27
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
 
/****************************************************************************/
/* sf_setindex:  set current index                                          */
/* Makes sure current index in sfile is up to date.  If not, it writes out  */
/* index cookie to the output file.  Puts out a start cookie if this is the */
/* first modification to the file                                           */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void sf_setindex (register SFILE *sfp, register index)
    /*sfp: structured file pointer */
    /*index:   new index               */
    {
#ifdef TRACE

    debug ("sf_setindex called with index = %d\n", index);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_setnode", T_SFILE);

#endif

    /* if this is the first modification, put out a start cookie */

    if (!obj_getflag (sfp, SF_MODIFIED))
	{
	sf_cookie (sfp, C_START);       /* output "start" cookie         */
	obj_setflag (sfp, SF_MODIFIED); /* set flag for next time        */
	}
    /* if index is not current index put out an index cookie */

    if (index != sf_index (sfp))
	{
	sf_cookie (sfp, C_INDEX, index);
	sfp->sf__index = index;
	}
    }

