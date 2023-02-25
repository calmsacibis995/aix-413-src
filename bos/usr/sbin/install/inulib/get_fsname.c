static char sccsid[] = "@(#)15  1.2  src/bos/usr/sbin/install/inulib/get_fsname.c, cmdinstl, bos411, 9428A410j 3/6/94 19:27:44";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: get_fsname 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------------------------------------------------------------------
 *
 * Function:  get_fsname
 *
 * Purpose:
 *            Find out which filesystem a given path belongs to.
 * Notes:
 * Calls get_stat which was borrowed from AIX V3 df.c.
 *
 * Returns:
 *     A char * to name of the filesystems.
 *     NIL (char) for failure.
 *
 * Parameters:
 *  path   -- path for which we want to to find
 *                        out the filesystem.
 *  
 *---------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/mntctl.h>
#include <sys/stat.h>
#include <inu_eval_size.h>
#include <inulib.h>
#include <inudef.h>

extern char     *inu_cmdname; /* Name of command calling these routines */

char * get_fsname (char *path)
{
   int count;        /* n of vmounts on sys (nfs, ds, etc.)*/
   struct vmount *p; /* pointer into buf vmount structs   */
   char *fs = NIL (char);
   struct stat stat_buf;

   if (stat (path, &stat_buf) != 0) 
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INSTALLP_CMD, path);
      return (NIL (char));
   }

   count = get_stat (&p);

   while (count > 0) 
   { /* for each one returned */
   
      if (stat_buf.st_vfs == p->vmt_vfsnumber) 
      {
         if ((fs = (char *) malloc (strlen (vmt2dataptr (p, VMT_STUB)) 
                    * sizeof (char))) ==  NIL (char)) 
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                         CMN_MALLOC_FAILED_D), inu_cmdname);
            return (NIL (char));
         }
         strcpy (fs, vmt2dataptr (p, VMT_STUB)); 
         break;
      }

      p = (struct vmount *) ((char *)p + p->vmt_length);  /* p++  */
      count--;
   }

   if (count <= 0) 
      return (NIL (char));

   return (fs);
}
