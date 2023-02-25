/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)35	1.5  src/tenplus/lib/obj/sf_seek.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:22";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_seek
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
/* sf_seek:  seek to specified location in delta file                       */
/* Note: this routine optimizes seeks in delta files.  It takes a character */
/* to indicate if a read 'r' or 'w' is about to happen.  On consecutive     */
/* write operations, no seeking is necessary.                               */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void sf_seek (SFILE *sfp, long fileloc, char modechar)
    /* sfp:                 sfile object to do seek in */
    /* fileloc:             absolute file location     */
    /* modechar:            'r' or 'w'                 */
    {
    register FILE *fp;   /* file pointer for sfp       */

    fp = sf_fp (sfp);

#ifdef DEBUG
    debug ("sf_seek:  sfp = 0%o, fileloc = %ld, modechar = '%c'\n",
	   sfp, fileloc, modechar);
    debug ("sf_seek:  SF_READ flag = %s, current fileloc = %ld\n",
	   obj_getflag (sfp, SF_READ) ? "TRUE" : "FALSE", ftell (fp));
#endif

#ifdef CAREFUL
    (void) s_typecheck ((char *)sfp, "sf_seek (sfp)", T_SFILE);
#endif

    switch (modechar)
	{
	case 'r': /* about to do a read */
	    if (!obj_getflag (sfp, SF_READ)) /* if last op was a write */
		obj_setflag (sfp, SF_READ);

	    (void) fseek (fp, fileloc, 0); /* always seek on read */
	    return;

	case 'w': /* about to do a write */
	    if (!obj_getflag (sfp, SF_READ)) /* if last op was a write */
		return;

	    obj_clearflag (sfp, SF_READ);
	    (void) fseek (fp, fileloc, 0);
	    return;

	default:
	    fatal ("sf_seek:  illegal mode character '%c'", modechar);
	}
    }

