#if !defined(lint)
static char sccsid[] = "@(#)72	1.7  src/tenplus/lib/util/fakename.c, tenplus, tenplus411, GOLD410 3/3/94 17:58:12";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fakename
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
 
/****************************************************************************/
/* file:  fakename.c                                                        */
/*                                                                          */
/* routine for generating fake (ASCII) filenames in the e2 editor           */
/****************************************************************************/

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <database.h>
#include <chardefs.h>
#include <libobj.h>

#define NTIMES 3          /*used to force three iterations of for loop */

/****************************************************************************/
/* fakename:  makes a tmp file name for an ASCII file                       */
/*                                                                          */
/* arguments:              char *filename - real name to get fake name for  */
/*                                                                          */
/* return value:           LOCAL char *   - temporary file name for fake    */
/*                                          file                            */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     fakename generates the name of the "fake" structured file the editor */
/*     will use to edit a file that is actually non-structured.  The fake   */
/*     file name is three dots "..." followed  by the first three character */
/*     of the real filename, followed by a hashcode computed from the       */
/*     filename.                                                            */
/****************************************************************************/

char *fakename (char *filename)
    {
    register char *tmpname;  /* tmp file name           */
    register char *cp;       /* used to find length     */
    register indx;          /* index for "..." in name */
    register long hashcode = 1L; /* hash code corresponding to file name */
    char fname[18 * MB_LEN_MAX];          /* temporary file name */
    int nbytes = 0;        /* number of bytes in each of first three multibyte 
                              characters of filename  */
    int totbytes = 0;      /* total number of bytes in first three  
                              characters of filename */
    int iters;             /* count to force three iterations of for loop */

    /* build the encrypted name for the ...'s file */
    tmpname = s_string (filename);
    cp = strrchr (tmpname, DIR_SEPARATOR);
    if(cp == NULL) /* no slash in name */
	indx = 0;
    else
	indx = cp - tmpname + 1;

    /* we may not always get the full pathname. Only use filename itself */
    for(cp = (filename + indx); *cp; cp++)
	{
	hashcode *= (long)(*cp);
	hashcode++;
	}

    /* !!! note potential problems with various file mechansism which use fewer
       characters in a file name or differenc formats (name.ext) */
    /* for loop forced to iterate three times, in order to process first
       three multibyte characters of filename */

    (void) strcpy(fname, "...");
    if (MultibyteCodeSet) {
      cp = filename + indx;
      for (iters = 0; iters < NTIMES; iters++) {     
           nbytes = mblen(cp, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (nbytes < 1) nbytes = 1;
           totbytes += nbytes;
           cp += nbytes;
      } 
    }
    else
       totbytes = 3;
    (void) strncpy(fname + 3, filename+indx, totbytes);
    fname[totbytes + 3] = '\0';
    (void) sprintf(fname + strlen(fname), "%lx", hashcode);

    tmpname = s_delete (tmpname, indx, -1);
    tmpname = s_insert (tmpname, indx, strlen(fname) + 1);
    (void) strcpy (tmpname + indx, fname);

# ifdef DEBUG
    debug ("fakename:  filename = '%s', tmpname = '%s'", filename, tmpname);
# endif

    return (tmpname);
    }
