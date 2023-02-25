static char sccsid[] = "@(#)14  1.22  src/bos/usr/sbin/install/inulib/inu_archive.c, cmdinstl, bos411, 9428A410j 3/6/94 19:27:46";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_archive (inulib)
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
#include <inulib.h>

/*
#include <sys/limits.h>
#include <unistd.h>
#include "../inudef.h"
#include <inu_toc.h>
#include <inuerr.h>
#include "../messages/commonmsg.h"
#include "../messages/instlmsg.h"
*/

extern char *inu_cmdname;

/*
 * NAME: inu_archive
 *
 * FUNCTION: Archive the common members both in apply list and archive
         control file to the library defined in archive control file.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *  INUGOOD:    normal return; no error;
 *  INUORP2:    error;
 */

static int doarchive (char *command, char *old_arc, 
                      char *tmp_arc_file, char *arc_file)
{
   char cmd[PATH_MAX*3];
   int rc;
   if (*old_arc=='.')
      old_arc++;

   if ((rc = inu_file_eval_size (old_arc, old_arc, (int) 0)) != INUGOOD)
   {
      if (rc == INUFS)
         return (INUFS);
      else
      {
         char    *expand;
         if ((expand = getenv ("INUEXPAND")) != NIL (char))
            return (INUNORP2);
      }
   }

   if ((access (old_arc, F_OK)) == 0)
   {
      sprintf (cmd, "cp %s %s", old_arc, tmp_arc_file);
      if (inu_do_exec (cmd))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D),
                              inu_cmdname, command);
         unlink (tmp_arc_file);
         return (INUNORP2);
      }
   }

   if (inu_do_exec (command))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D), 
                                   inu_cmdname, command);
      unlink (tmp_arc_file);
      return (INUNORP2);
   }

   if (rename (tmp_arc_file, old_arc) == -1)
   {
      inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_RENAME_W, CMN_RENAME_D), 
                           inu_cmdname, tmp_arc_file, old_arc);
      unlink (tmp_arc_file);
      return (INUNORP2);
   }

   return (INUGOOD);

} /* doarchive */

int
inu_archive (FILE *acf_fp, FILE *apply_fp, FILE *del_fp)
{
    char    member[PATH_MAX+1];
    char    command[MAX_COM_LEN];       /* Command line length */
    char    old_arc[PATH_MAX+1];
    char    tmp_arc_file[PATH_MAX+1];
    char    arc_file[PATH_MAX+1];
    int rc;

    old_arc[0] = '\0';  /* Indicate the first read in while loop */

    while (fscanf (acf_fp, "%s %s", member, arc_file) != EOF)
    {
        if (in_al (member, apply_fp))
        {
            if (old_arc[0] == '\0')
            {
                strcpy (old_arc, arc_file);
                strcpy (tmp_arc_file, arc_file);
                strcat (tmp_arc_file, ".new");
                sprintf (command, "/usr/ccs/bin/ar -or %s", tmp_arc_file);
            }

            /************************************************/
            /* If not NULL then write member to delete file */
            /************************************************/
            if (del_fp != (FILE *) NULL)
            {
                fprintf (del_fp, "%s\n", member);
            }

            if (strcmp (old_arc, arc_file))
            {
               if ((rc = doarchive (command, old_arc, tmp_arc_file, arc_file))
                    != INUGOOD)
                  return (rc);

               strcpy (old_arc, arc_file);
               strcpy (tmp_arc_file, arc_file);
               strcat (tmp_arc_file, ".new");
               sprintf (command, "/usr/ccs/bin/ar -or %s", tmp_arc_file);
            }
            else
            {
               if ((strlen (command) + strlen (member) + 1) >= MAX_COM_LEN)
               {
                  if ((rc = doarchive (command, old_arc, tmp_arc_file, 
                                       arc_file)) != INUGOOD)
                     return (rc);

                  sprintf (command, "/usr/ccs/bin/ar -or %s", tmp_arc_file);
               }
            }
            strcat (command, " ");
            strcat (command, member);

        }/* if statement of in_al () */

    }   /* while */

    /* Clean the command for the last round */
    if (old_arc[0] != '\0')
    {
       if ((rc = doarchive (command, old_arc, tmp_arc_file, arc_file)) 
            != INUGOOD)
          return (rc);
    }

    return (INUGOOD);
}
