static char sccsid[] = "@(#)67  1.8  src/bos/usr/sbin/install/inulib/inu_lock.c, cmdinstl, bos411, 9428A410j 6/17/93 16:15:19";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_testsetlock
 *		inu_unsetlock
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/mode.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <inudef.h>

/* NAME:       inu_testsetlock
 *
 * FUNCTION:   This routines sets up a semaphore passed on the argument path.
 *
 * PARAMETERS: path is the full path of the location of the lock.
 *
 * RETURNS:    -1 if error.
 *             File descriptor if successful.
 */

int inu_testsetlock (char * path)
{
   register int    fd;
   int             fmodes = O_WRONLY | O_CREAT;
   char           *ptr;
   struct flock    flock_struct;
   char            temp_path[PATH_MAX + 1];

   strcpy (temp_path, path);
   ptr = strrchr (temp_path, '/');
   *ptr = '\0';
   if (inu_mk_dir (temp_path) != 0)
      return (-1);

   /* AIX security enhancement                           */
   /* create with no permissions. They're added below    */

   if ((fd = open (path, fmodes)) == -1)
      return (-1);

   flock_struct.l_type = F_WRLCK;
   flock_struct.l_whence = 0;
   flock_struct.l_start = 0;
   flock_struct.l_len = 0;
   if (-1 == fcntl (fd, F_SETLK, &flock_struct))
      return (-1);
   return (fd);

} /* end inu_testsetlock */

/* NAME:       inu_unsetlock
 *
 * FUNCTION:   Releases the semaphore test by inu_testsetlock
 *
 * PARAMETERS: SemID Return value of inu_testsetlock.
 */

void inu_unsetlock (int fildes)
{
   close (fildes);
} /* end inu_unsetlock */
