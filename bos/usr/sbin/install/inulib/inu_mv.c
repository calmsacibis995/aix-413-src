static char sccsid[] = "@(#)68  1.9  src/bos/usr/sbin/install/inulib/inu_mv.c, cmdinstl, bos411, 9428A410j 1/29/94 18:48:56";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_mv (inulib)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
                                                                  
/*
#include <inuerr.h>
#include <stdio.h>
#include <unistd.h>
#include "../inudef.h"
#include <inu_toc.h>
#include <errno.h>
*/

#include <inulib.h>
#include <sys/stat.h>
#include <sys/limits.h>

/*
 * NAME: inu_mv
 *
 * FUNCTION: Moves a source file "from" to a destination file "to".
 *          
 * NOTES: Both "from" and "to" MUST be relative or absolute FILE path names.
 *        DIRECTORIES ARE NOT HANDLED.  If either of the arguments are 
 *        links to the other, or if "from's directory" is a link to to's  
 *        directory, (or vice versa) then inu_mv returns with failure.      
 *        The only exception is if to is a symbolic link to from, in which 
 *        case inu_mv emulates AIX's "mv", by performing the appropriate move.
 */ 


int inu_mv(char *from, char *to)
{

   struct stat fromstat,      /* records for from and to status checks */
	       tostat;
   char lnkbuff[PATH_MAX+1];  /* used in call to readlink */
   int rc;                    /* return codes */

/* ------------------------------------------------------------------
   The following section was added to detect the presence of links.
   If a symbolic or hard link is detected between the from file and the     
   to file then, in most cases, inu_mv returns with failure.  If, 
   however, 'to' is a symbolic link to 'from,' inu_mv proceeds as normal.
   -----------------------------------------------------------------*/
   if (stat(from,&fromstat) != 0)   
       return(-1);

   if (S_ISDIR(fromstat.st_mode))
       return(-1);         /* MOVEMENT OF DIRECTORIES IS NOT HANDLED! */


  /* Defect 109273 -- access tostat only if stat succeeds */

  if ( stat(to,&tostat) != 0) {
      if (errno != ENOENT)   /* it's ok if to doesn't exist */
          return(-1);
  }
  else {  /* stat succeeded */

    if ( S_ISDIR(tostat.st_mode)) 
         return(-1);         /* MOVING TO A DIRECTORY IS NOT HANDLED EITHER */

    /* if to exists....*/
    /* check if from and to have the same inode number.  This may be   */
    /* a result of one file being a link to the other or a directory   */
    /* in which one resides being a link to the other's.               */
    if (tostat.st_ino == fromstat.st_ino)  
       /* only proceed if we're moving a file to a link to that file */
       if ((rc = readlink(to,lnkbuff,PATH_MAX + 1)) <= 0) 
          return(-1);   
  }      

  /* ----------------------------------------------------------------- */
 
    if (access(to, F_OK) == 0 && unlink(to) != 0)
        return(-1);

    if (link(from, to) == 0)
	return(unlink(from));
    else
	return((inu_cp(from, to) != INUGOOD) ? -1 : unlink(from));
}
