/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)68	1.5  src/tenplus/lib/obj/isdelta.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:21";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	isdelta
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
/* isdelta:  returns TRUE if file is empty or a delta file, else FALSE          */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

int isdelta (char *filename)
    /* filename:  name of file to check */
    {
    register FILE *fp;       /* file pointer for file */

#ifdef TRACE

    debug ("isdelta:  filename = '%s'\n", filename);

#endif

    fp = fopen (filename, "r");

    if (fp == NULL) /* if no such file */
        return (FALSE);

    (void) fseek (fp, 0L, 2); /* seek to end of file */

    if (ftell (fp) == 0L) /* if empty file, return FALSE */
        {
        (void)fclose (fp);
        return(FALSE);
        }
    (void) fseek (fp, -6L, 2); /* seek six chars from end of file */


    if ((fgetc (fp) == COOKIE) && (fgetc (fp) == C_END))
        {
        (void)fclose (fp);
        return (TRUE);
        }
    (void)fclose (fp);
    return(FALSE);
    }

