static char     sccsid[] = "@(#)72      1.23  src/bos/usr/sbin/install/inulib/inu_restore.c, cmdinstl, bos411, 9428A410j 6/8/94 13:16:10";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_restore
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>

/* NAME:     inu_restore
 *
 * FUNCTION: Builds a shell command to run restore.
 *
 * RETURN VALUE DESCRIPTION: passed from restore command */

int inu_restore (char     * device,     /* device from which to restore */
                 int        quiet,      /* TRUE if 'q' flag to be specified */
                 int        type,       /* Type of media (from the toc
                                         * structure) */
                 Option_t * sop,        /* Pointer to the current selected
                                         * option */
                 char     * file_names) /* file name (s) to restore */
{
   char  * cmd;         /* command to execute */
   int     rc;          /* return code from runsh */
   char    q_flag[2];   /* -q flag */

   /* Begin inu_restore */

   /* build command */

   cmd = (char *) malloc ((size_t) (strlen (device) +
                                       strlen (file_names) + 255));

   if (cmd == NULL)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                           CMN_MALLOC_FAILED_D), INU_INSTALLP_CMD);
      return (INUNOSPC);
   }

   if (quiet)
      strcpy (q_flag, "q");
   else
      q_flag[0] = NUL;

   /* run command */

   /* restore options : -X indicates the volume to begin the restore from.
    *                   -M restores the access and modification dates of files.
    *                   -p "rewinds" the media to the front of the image.
    *                   -h restores the access and modification dates of files.
    *                   -q suppress the first query for device.
    *                   -f designates the device to be installed from.
    *                   -S stop on failure
    */


   if (type == TYPE_FLOP_SKD  &&  sop != NULL)
   {
      /* The wait is necessary in order to guarantee that the shell created by
         the system call does not terminate before the entire pipe-line is
         finished.  It is possible (and has happened) that the restore operation
         terminates before the inurdsd operation.  When that happens, the shell
         terminates and the diskette remains busy until inurdsd finally
         terminates. */

      sprintf (cmd, "/usr/sbin/inurdsd -d%s -v%d -b%d -n%d \
                     | (/usr/sbin/restbyname -S -xhqf - %s >/dev/null; exec < /dev/null; wait)", 
               device, sop->bff->vol, sop->bff->off, sop->bff->size, 
               file_names);
   }
   else if (type == TYPE_TAPE_SKD)
      sprintf (cmd, "/usr/lib/instl/inu_tape_dd -b 10 -r -d %s | \
/usr/sbin/restbyname -S -C 10 -qxhf - %s > /dev/null",device,file_names);

   else
   {
      sprintf (cmd, "/usr/sbin/restbyname -S -pxh%sf%s %s >/dev/null", 
               q_flag, device, file_names); 
   }
   rc = inu_do_exec (cmd);

   (void) free (cmd);

   return (rc);

} /* end inu_restore */
