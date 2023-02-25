#if !defined(lint)
static char sccsid[] = "@(#)56	1.8  src/tenplus/lib/obj/fbackup.c, tenplus, tenplus411, GOLD410 3/3/94 16:26:13";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fbackup
 * 
 * ORIGINS:  9, 10
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

#include <stdio.h>
#include <string.h>
#include <chardefs.h>
#include <unistd.h>
#include <database.h>
#include <stdlib.h>
#include <libobj.h>

/****************************************************************************/
/* fbackup:  moves a file to a backup name                                  */
/* The backup name is the file name concatenated with ".bak"                */
/* Returns ERROR if file cannot be backed up.                               */
/****************************************************************************/


int fbackup (char *filename)
    {
    register char *cp;
    register char *limit;
    char bakname[PATH_MAX];    /*variable holding name of backup file */
    /* if backup name has format <path>/<basename>.bak, then
       max_bak_basename holds maximum number of bytes in <basename> */
    long max_bak_basename;

    if (!fexists (filename))
       return (RET_OK);

    max_bak_basename = pathconf(filename, _PC_NAME_MAX) - 4;

    (void) strcpy(bakname, filename);

    /* let cp point to the basename */
    cp = strrchr(bakname, DIR_SEPARATOR);
    if (cp != NULL)
         cp++; /* DIR_SEPARATOR has length 1 */
    else cp = bakname;

    /* if the existing filename is longer than the max basename for
       the backup file, then truncate the name at the last char that
       does not go beyond the cut-off point
    */
    if (strlen(cp) > max_bak_basename)
    {
        if (MultibyteCodeSet)
        {
	    int mlen;

            limit = cp + max_bak_basename;
	    mlen = mblen(cp, MB_CUR_MAX);
	    /* copy at least 1 char */
	    if (mlen < 1) mlen = 1;
            while ((cp + mlen) < limit) {
                cp += mlen;
                mlen = mblen(cp, MB_CUR_MAX);
		/* copy at least 1 char */
		if (mlen < 1) mlen = 1;
	    }
        }
        else
            cp += max_bak_basename;
        *cp = '\0';    /* truncate the name */
    }

     /* add .bak to name */
     (void) strcat(bakname, ".bak");   

    /* rename file to backup file and return */
    return (rename (filename, bakname));
    }
