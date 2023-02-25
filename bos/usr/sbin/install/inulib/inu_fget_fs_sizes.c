static char     sccsid[] = "@(#)65      1.6  src/bos/usr/sbin/install/inulib/inu_fget_fs_sizes.c, cmdinstl, bos410 11/4/92 16:20:01";

/* COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_fget_fs_sizes
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
#include <macros.h>

void inu_fget_fs_sizes (FILE   * fp,         /* size file pointer. */
                        size_t   total[],    /* total block usage for each. */
                        size_t   max_temp[]) /* max temp block usage for each */
{
   char            dir[PATH_MAX];    /* dir name from size file       */
   size_t          size = 0;         /* size read from size file      */
   size_t          tsize = 0;        /* size read from size file      */
   int             j;                /* misc counter                  */

   /* add file system size for every dir in size file */

   while ((fscanf (fp, "%s %d %d %[\n]", dir, &size, &tsize)) != EOF)
   {
      /* Use absolute path for directory */

      if (dir[0] == '.')
         j = find_lfs (dir + 1);    /* find appropriate fs on this sys */
      else
         j = find_lfs (dir);        /* find appropriate fs on this sys  */

      if (j >= 0)  /* Kludge for if dir is not found by find_lfs. */
      {
         total[j] += size;             /* add to its total                 */
         max_temp[j] = max (max_temp[j], tsize);
      }
      size = 0;                     /* in case field is blank next time */
      tsize = 0;
   } /* end while */

} /* end inu_fget_fs_sizes */
