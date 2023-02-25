static char     sccsid[] = "@(#)64  1.4  src/bos/usr/sbin/install/inulib/inu_fget_file_sizes.c, cmdinstl, bos410 11/4/92 16:19:54";

/* COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_fget_file_sizes
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when combined with the
 * aggregated modules for this product)                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991 All Rights
 * Reserved US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp. */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/access.h>

void inu_fget_file_sizes (FILE   * fp,
                          size_t   total[],
                          int      active)
{
   char            file[PATH_MAX];   /* dir name from size file */
   char           *file_ptr;
   int             j;                /* misc counter */
   struct stat     buf;

   /* for every file in the apply list get it's size */

   while (fscanf (fp, "%s", file) != EOF)
   {
      /* make file path an absolute path */

      file_ptr = file;
      if (file[0] == '.')
         file_ptr++;

      /* get file size in 512 byte blocks (round to block size) */

      if (stat (file_ptr, &buf) == 0
                      &&
         (!active || access (file_ptr, W_ACC) == 0))
      {
         j = find_lfs (file_ptr);
         if (j >= 0)  /* Kludge for if dir is not found by find_lfs. */
         {
           total[j] += ((((buf.st_size + buf.st_blksize - 1) / buf.st_blksize) *
                        buf.st_blksize) + 511) / 512;
         }
      } /* end if */

   } /* end while */

} /* end inu_fget_file_sizes */
