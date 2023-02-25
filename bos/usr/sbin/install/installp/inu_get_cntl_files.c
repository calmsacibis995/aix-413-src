static char sccsid[] = "@(#)92  1.20  src/bos/usr/sbin/install/installp/inu_get_cntl_files.c, cmdinstl, bos411, 9428A410j 5/31/94 08:55:40";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_get_cntl_files, inu_get_member_list
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/signal.h>
#include <installp.h>
#include <instl_options.h>

static char * inu_get_member_list (Option_t *);

/*--------------------------------------------------------------------------*
**
** NAME: inu_get_cntl_files ()
**
** FUNCTION:
**
** RETURN VALUE DESCRIPTION:
**
**--------------------------------------------------------------------------*/

int inu_get_cntl_files (Option_t *sop,      /* Ptr to 1st option in the sop for
                                             * this oper */
                        Option_t *next_op)  /* Ptr to 1st option in the sop for
                                             * the next oper */
{
   char       cmd [PATH_MAX];
   Option_t * op;
   char       buf[TMP_BUF_SZ];
   char       root_lib[TMP_BUF_SZ];
   char       main_lib[TMP_BUF_SZ];
   char       root_libdir[TMP_BUF_SZ];
   char       lppname[MAX_LPP_OPTIONNAME_LEN];
   int        rc;

   /*----------------------------------------------------------------------*
    * Require a minimum of 1000 free blocks in the /usr file system
    *----------------------------------------------------------------------*/

   if (inu_get_min_space ("/usr", MIN_SPACE_USR) == FAILURE)
      return (INUNOSPC); /* error msg is displayed in inu_get_min_space()*/

   /*----------------------------------------------------------------------*
    * Build the path name for the liblpp.a file to be passed to restore.
    *----------------------------------------------------------------------*/

   strcpy (buf, ".");
   strcat (buf, INU_LIBDIR);

   /*----------------------------------------------------------------------*
    * Then see if there is a root part for this lpp and if yes then build
    * the inst_root/liblpp.a path.
    *----------------------------------------------------------------------*/

   root_lib[0] = '\0';
   if (sop->vpd_tree == VPDTREE_USR)
   {
      for (op = sop; op != next_op; op = op->next)
      {
         if (op->content == CONTENT_BOTH)
         {
            strcat (root_lib, buf);
            strcat (root_lib, "/inst_root");
            strcpy (root_libdir, &root_lib[1]);
            strcat (root_lib, "/liblpp.a");
            break;
         }
      }
   }

   strcat (buf, "/liblpp.a");
   strcpy (main_lib, buf);
   strcat (buf, " ");
   strcat (buf, root_lib);

   /*----------------------------------------------------------------------*
    * cd to root directory and restore the liblpp.a from the media...
    *----------------------------------------------------------------------*/

   if ((rc = chdir ("/")) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                               CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, "/");
      return (INUCHDIR);
   }

   /*----------------------------------------------------------------------*
    * Restore the liblpp.a file into the libdir directory
    *----------------------------------------------------------------------*/

   if ((rc = inu_restore (sop->bff->path, TRUE, TocPtr->type, sop, 
                          buf)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_RESTORE_E, 
                               INP_BAD_RESTORE_D));
      if (rc == EFBIG  ||  rc == ENOSPC)
         return (INUFULLD);
      else
         return (INURESTR);
   }

   /*----------------------------------------------------------------------*
    * cd to the directory where liblpp.a was put and unarchive the library.
    *----------------------------------------------------------------------*/

   if ((rc = chdir (INU_LIBDIR)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                           CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, INU_LIBDIR);
      return (INUCHDIR);
   }

   /** ----------------------------------------------------------------------*
    **  If the -A or -i flag is specified, then only unarchive the files from
    **  the liblpp.a library that we need  (as opposed to unarchiving every
    **  member in the liblpp.a library).   This could result in a bad space
    **  problem.
    ** ----------------------------------------------------------------------*/

   if (A_FLAG  ||  i_FLAG)
   {
      sprintf (cmd, "/usr/ccs/bin/ar -xl liblpp.a %s > /dev/null 2>&1", 
                    (inu_get_member_list (sop)));
   }
   else
   {
      strcpy (cmd, "/usr/ccs/bin/ar -xl liblpp.a >&-");
   }

   /*----------------------------------------------------------------------*
    * Un-archive liblpp.a where liblpp.a was put and unarchive the library.
    *----------------------------------------------------------------------*/

   rc = inu_do_exec (cmd);
   (void) unlink (&main_lib[1]);
   if (rc != SUCCESS)
   {
      /** -------------------------------------------------------------------*
       **  Since some of the member files we asked to be unarchived are not
       **  required to exist if we specified the -A or -i flags, we CANNOT
       **  error off if the system call to unarchive them failed.
       ** -------------------------------------------------------------------*/

      if ( ! (A_FLAG  ||  i_FLAG))
      {
         switch (rc)
         {
            case SIGINT:
               (void) inu_sig_abort (rc);
               break;
            default:
               /* can't un-ar liblpp.a */
               inu_get_prodname (sop, lppname);
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOLIBLPP_E, 
                                    INP_NOLIBLPP_D), lppname);
               return (rc);
         } /* end switch */
      } /* end if */
   } /* end if */

   /*----------------------------------------------------------------------*
    * If there was no root part then return...
    *----------------------------------------------------------------------*/

   if (root_lib[0] == '\0')
      return (SUCCESS);

   /*----------------------------------------------------------------------*
    * cd to the directory where liblpp.a was put and unarchive the library.
    *----------------------------------------------------------------------*/

   if ((rc = chdir (root_libdir)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                           CMN_CANT_CHDIR_D), 
               INU_INSTALLP_CMD, root_libdir);
      return (INUCHDIR);
   }

   /** ----------------------------------------------------------------------*
    **  If the -A or -i flag is specified, then only unarchive the files from
    **  the liblpp.a library that we need  (as opposed to unarchiving every
    **  member in the liblpp.a library).   This could result in a bad space
    **  problem.
    ** ----------------------------------------------------------------------*/

   if (A_FLAG  ||  i_FLAG)
   {
      sprintf (cmd, "/usr/ccs/bin/ar -xl liblpp.a %s > /dev/null 2>&1", 
                    (inu_get_member_list (sop)));
   }
   else
   {
      strcpy (cmd, "/usr/ccs/bin/ar -xl liblpp.a >&-");
   }

   /*----------------------------------------------------------------------*
    * Un-archive liblpp.a where liblpp.a was put and unarchive the library.
    *----------------------------------------------------------------------*/

   rc = inu_do_exec (cmd);
   (void) unlink (&root_lib[1]);
   if (rc != SUCCESS)
   {
      /** -------------------------------------------------------------------*
       **  Since some of the member files we asked to be unarchived are not
       **  required to exist if we specified the -A or -i flags, we CANNOT
       **  error off if the system call to unarchive them failed.
       ** -------------------------------------------------------------------*/

      if ( ! (A_FLAG || i_FLAG))
      {
         switch (rc)
         {
            case SIGINT:
               (void) inu_sig_abort (rc);
               break;
            default:
               /* can't un-ar liblpp.a */

               inu_get_prodname (sop, lppname);
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOLIBLPP_E, 
                                    INP_NOLIBLPP_D), lppname);
               return (rc);
         }
      } /* end if */
   }
   return (SUCCESS);

} /* end inu_get_cntl_files */

/** ------------------------------------------------------------------------ *
 **
 **  Function:   inu_get_member_list
 **
 **  Purpose:    Gets a list of the member files to unarchive for the
 **              -A and -i flags.  Currently this is as follows:
 **              -A flag:  "fixdata"
 **              -i flag:  "README", "lpp.README", "lpp.doc", "lpp.instr", and
 **                        "lpp.lps".
 **
 **  Returns:    a string containing the files to be restored
 **
 **  Notes:
 **
 ** ------------------------------------------------------------------------ */

static char * inu_get_member_list (Option_t *op)
{

  static char buf[128];
  /** ----------------------------------------------------- *
   **  The -A flag and the -i flag are mutually exclusive.
   ** ----------------------------------------------------- */



   if (A_FLAG)
   {
       strcpy(buf,op->name);
       strcat(buf,".fixdata");
       return(buf);
   }
   else if (i_FLAG)
   {
       strcpy(buf,op->name);
       strcat(buf,".fixdata");
       strcat(buf," README lpp.README lpp.doc lpp.instr lpp.lps");
       return(buf);
   }

	
   return ("");
}
