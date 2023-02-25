static char sccsid[] = "@(#)03  1.4  src/bos/usr/sbin/install/installp/getspace_migdir.c, cmdinstl, bos411, 9428A410j 6/3/94 09:35:20";

/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: 
 *		get_sizeof_file
 *		get_vfsnumber
 *		getspace_migdir
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <installp.h>
#include <inulib.h>
#include <instl_options.h>
#include <instl_define.h>

extern int errno;

/* ------------------------------------------------------------------------ *
 *
 *  Function:    getspace_migdir 
 *
 *  Purpose:     To ensure enough space exists in the MIGSAVE
 *               dir to save the files listed in INULIBDIR/option.cfgfiles  
 *              
 *
 *  Parameters:  cur_op  -- Current option 
 *               next_op -- Next option
 *
 *  Returns:     boolean
 *                   -- TRUE on  SUCCESS
 *                   -- FALSE on FAILURE
 * ------------------------------------------------------------------------ */

boolean
getspace_migdir (Option_t * cur_op, 
                 Option_t * next_op)
{
   Option_t *op;
   FILE * fp;
   char buf [PATH_MAX]="/tmp", 
        *migfs, 
        *mdir, 
        migdir [PATH_MAX];

   long mig_vfsnum=0, 
        vnum, 
        num_of_blox_needed=0;
        
   /* Figure out target save directory */
   switch (cur_op->vpd_tree)
   {
      case VPDTREE_USR:
           sprintf (migdir, "%s", USR_MIGSAVE);
           break;

      case VPDTREE_ROOT:
           sprintf (migdir, "%s", ROOT_MIGSAVE);
           break;

      case VPDTREE_SHARE:
           sprintf (migdir, "%s", SHARE_MIGSAVE);
           break;

      default:
           return (FALSE);
           break;
   }

   /* create it */
   if (inu_mk_dir (migdir) != INUGOOD)
       return (FALSE);

   if ((mig_vfsnum = get_vfsnumber (migdir)) == 0)
         return (FALSE);
   
   /* to get to migdir */
   if ((mdir = get_migdir_prefix (cur_op)) == NIL (char))
       return (FALSE);

   for (op = cur_op; op != next_op; op = op->next) {

        /* save was not requested */
        sprintf (buf, "%s/%s/.%s.installed_list", mdir, op->prodname, op->name);
        if ( ! path_exists (buf))
            continue;

        /* cfgfiles present? */
        sprintf (buf, "%s/%s.%s", INU_LIBDIR, op->name, CFGFILES);
        if ( ! path_exists (buf))
            continue;

        if ((fp=fopen (buf, "r")) == NIL (FILE)) {
             perror ("open");
             return (FALSE);
        }

        while (fscanf (fp, "%s %*s", buf) != EOF) 
         {
           /* -------------------------------------------------------------
           *
           * since we are doing a mv, don't count the files in the same fs
           * as the MIGSAVE dir. We may have to worry about the busy files
           *
           * ------------------------------------------------------------- */
      
           if (((vnum = get_vfsnumber (buf)) == mig_vfsnum)  ||  vnum == 0)   
               continue;
           else 
              num_of_blox_needed += get_sizeof_file (buf);

        }  /* of while ! EOF */

        fclose (fp);
   }  /* of for */


  if ( ! num_of_blox_needed)
       return (TRUE);

  /*
   * Call the routine inu_get_min_space to ensure that enough space 
   * exists.  (expects 1024 byte blocks as input)
   */
  return (inu_get_min_space (migdir, num_of_blox_needed * 2)); 
}

/* ------------------------------------------------------------------------ *
 *
 *  Function:    get_sizeof_file
 *
 *  Purpose:     This gets the size of the file in 512-byte blocks 
 *
 *  Parameters:  path -- Name of the file 
 *
 *  Returns:     long
 *               number of free 512 byte blocks
 *
 * ------------------------------------------------------------------------ */

get_sizeof_file (char *path)
{
   struct stat sbuf;

   if (stat (path, &sbuf) != 0)
      return (0);

   return (((((sbuf.st_size + sbuf.st_blksize - 1) / sbuf.st_blksize) *
                        sbuf.st_blksize) + 511) / 512);

}

/* ------------------------------------------------------------------------ *
 *
 *  Function:    get_vfsnumber
 *
 *  Purpose:     This gets vfs number of the given path 
 *
 *  Parameters:  path -- Name of the file/dir
 *
 *  Returns:     long
 *               vfsnumber 
 *
 * ------------------------------------------------------------------------ */

static long 
get_vfsnumber (char * path)
{
   struct statfs sbuf;

   if (statfs (path, &sbuf) != 0) 
      if (errno == ENOENT)
         return (0);
      else {
         perror ("stat");
         exit (-1);
      }

   return (sbuf.f_vfsnumber);
}

