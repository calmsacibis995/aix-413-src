static char sccsid[] = "@(#)00  1.37  src/bos/usr/sbin/install/installp/inu_reject_opts.c, cmdinstl, bos411, 9439C411a 9/29/94 16:49:54";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: do_status_file
 *		inu_get_reject_script
 *		inu_lpp_reject
 *		inu_reject_opts
 *		root_part
 *		usr_part
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *---------------------------------------------------------------------*/

#include <pwd.h>
#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>

static void do_status_file (Option_t *, Option_t *);

static int inu_lpp_reject (int, char, char *);

static int recreate_and_chdir_libdir (Option_t *);

/* path to reject script */
#define inu_get_reject_script(rj)  strcpy (rj, INU_LIBDIR);\
                                   strcat (rj, "/lpp.reject")

#define root_part(op)   (op->vpd_tree == VPDTREE_ROOT ? 1 : 0)
#define usr_part(op)    (op->vpd_tree == VPDTREE_USR ? 1 : 0)

static boolean any_failure = FALSE;


/*--------------------------------------------------------------------------*
**
** Function:     inu_reject_opts
**
** Purpose:      reject the options from cur_op to next_op
**
**-------------------------------------------------------------------------*/

int inu_reject_opts (Option_t * cur_op, Option_t * next_op)
{
   char      lppname [MAX_LPP_NAME];
   int       rc = INUGOOD;
   Option_t *op;

   if ( ! IF_C_UPDT (cur_op->op_type))
   {
      if ((rc = chdir (INU_LIBDIR)) != SUCCESS)
      {
         if (root_part (cur_op))
            rc = recreate_and_chdir_libdir (cur_op);         

         if (rc != SUCCESS)
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                              CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, INU_LIBDIR);

            for (op = cur_op; op != next_op; op = op->next)
               op->Status = STAT_FAILURE;

            return (INUCHDIR);
         }
      }

      inu_set_savdir (cur_op->vpd_tree, cur_op);

      inu_ignore_all_sigs ();
   }

   /* Update the vpd database stuff. we are also marking every option as
    * failing. */

   for (op = cur_op; op != next_op; op = op->next)
   {
      op->Status = STAT_FAILURE;
      inu_vpd_lpp_prod (op, ST_REJECTING);
      inu_vpd_history (op, HIST_PENDING, HIST_REJECT);
   }

   if ( ! IF_C_UPDT (cur_op->op_type))
   {
      inu_get_prodname (cur_op, lppname);
      rc = inu_lpp_reject (cur_op->op_type, 'r', lppname);
      inu_ignore_all_sigs ();
   }

   if (rc == INUGOOD)
      /* mark all options as successful */
      for (op = cur_op; op != next_op; op = op->next)
      {
         op->Status = STAT_SUCCESS;
         inu_vpd_lpp_prod (op, ST_AVAILABLE);
         if (IF_INSTALL (op->op_type))
            (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_PREVIOUS);
      }
   else
      do_status_file (cur_op, next_op);

   if ( ! IF_C_UPDT (cur_op->op_type))
      inu_init_sigs ();

   if (any_failure)
      inu_print_fail_stats (cur_op, next_op);

   /* for 3.1 we have some things we have to do
    *   1. remove the reject srcipt
    *   2. remove savedir
    *   3. save apars file
    */

   if (IF_3_1_UPDATE (cur_op->op_type))
   {
      char  apars [PATH_MAX + 1];
      char  apars_save [PATH_MAX + 1];
      char  inu_savdir [PATH_MAX + 1];
      char  reject_script [PATH_MAX + 1];

      /* get rid of save reject script and save dir */

      inu_get_reject_script (reject_script);
      unlink (reject_script);
      inu_get_savdir (cur_op->op_type, cur_op, inu_savdir);
      inu_rm_save_dir (inu_savdir);

      /* path to the lpp */

      sprintf (apars, "/usr/lpp/%s/apars", lppname);
      strcpy (apars_save, apars);
      strcat (apars_save, ".save");
      inu_rm (apars);
      rename (apars_save, apars);
   }

   rm_biron_files (cur_op, next_op, OP_REJECT);

   return rc;
} 


/*--------------------------------------------------------------------------*
**
** Function:     do_status_file
**
** Purpose:      read the status file and find successes on the reject
**
**-------------------------------------------------------------------------*/

static void do_status_file (Option_t * cur_op, Option_t * next_op)
{
   Option_t       *op;
   FILE           *fd;
   char            stat;
   char            status_file [PATH_MAX + 1];
   char            opt_name [MAX_LPP_NAME + 1];
   char            buf [TMP_BUF_SZ];

   /* find the location of the status file */
   if ( ! IF_3_1 (cur_op->op_type))
   {
      strcpy (status_file, INU_TMPDIR);
      strcat (status_file, "/status");
   }
   else
      strcpy (status_file, "./status");

   /* Try to see if any options were ok. */
   if ((fd = fopen (status_file, "r")) != NULL)
   {
      /* process the status file */
      while (fgets (buf, TMP_BUF_SZ, fd) != NULL)
      {
         /* update HISTORY for all options that rejected ok */
         if (sscanf (buf, "%c %s", &stat, opt_name) == 2  &&  stat == 's')
         {
            /* find the option in the cur_op list */
            for (op = cur_op;
                 op != next_op  &&  strcmp (op->name, opt_name) != 0;
                 op = op->next);

            /* did we find our option? */
            if (op != next_op)
               op->Status = STAT_SUCCESS;
         } /* if */
      } /* while */

      fclose (fd);
   }
   else
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_REJECT_NOSTAT_E, 
                           INP_REJECT_NOSTAT_D));

   /* Update vpd HISTORY datebase for successes and failures. */
   for (op = cur_op; op != next_op; op = op->next)
      if (op->Status == STAT_SUCCESS)
      {
         inu_vpd_lpp_prod (op, ST_AVAILABLE);
         if (IF_INSTALL (op->op_type))
            (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_PREVIOUS);
      }
      else
      {
         any_failure = TRUE;
         inu_vpd_lpp_prod (op, ST_BROKEN);
         inu_vpd_history (op, HIST_BROKEN, HIST_REJECT);
         if (IF_INSTALL (op->op_type))
            (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);
      }

} /* do_status_file */


/*--------------------------------------------------------------------------*
**
** Function:     inu_lpp_reject
**
** Purpose:      run the reject script for this option
**
**-------------------------------------------------------------------------*/

static int inu_lpp_reject (int op_type, char rej_type, char * lpp)
{
   char    command [3 * PATH_MAX];
   char    reject_script [PATH_MAX + 1];

   inu_get_reject_script (reject_script);

   /* What reject script are we going to use? */

   if (access (reject_script, F_OK) == 0)
      sprintf (command, "%s %c %s/lpp.options", reject_script, rej_type, 
               INU_TMPDIR);
   else
      if (IF_3_1_UPDATE (op_type))
         sprintf (command, "/usr/sbin/inurecv %s", lpp);
      else
         sprintf (command, "%s %c %s/lpp.options", DEF_REJECT_SCRIPT, rej_type, 
                  INU_TMPDIR);

   /* return status of running reject script */

   inu_init_sigs ();

   return ((int) (inu_do_exec (command)));

} /* inu_lpp_reject */

/*--------------------------------------------------------------------------*
**
** Function:     recreate_and_chdir_libdir
**
** Purpose:      Remake the path to INU_LIBDIR, cd there and attempt to
**               copy the corresponding update deinstall directory informa-
**               back to the INU_LIBDIR.
**
**-------------------------------------------------------------------------*/

static int recreate_and_chdir_libdir (Option_t * op)
{
   char cmd [PATH_MAX+1];
   char     lppname [MAX_LPP_NAME];
   int      rc;

   sprintf (cmd, "/usr/bin/mkdir -p %s", INU_LIBDIR);

   if ((rc = inu_do_exec (cmd)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E,
                           CMN_NO_MK_DIR_D), INU_INSTALLP_CMD, INU_LIBDIR);
      return (INUCREAT);
   }

   if ((rc = chdir (INU_LIBDIR)) == SUCCESS)
   {
      /* Form the command to copy information from the deinstall directory */
      inu_get_prodname (op, lppname);

      if (IF_3_X (op->op_type))
      {
         sprintf (cmd, "/usr/bin/cp /lpp/%s/deinstl/%s/%s.* . >/dev/null 2>&1",
                       lppname, op->level.ptf, op->name);
      }
      else
      {
         sprintf (cmd, "/usr/bin/cp /lpp/%s/deinstl/%s/%d.%d.%d.%d/* . >/dev/null 2>&1",
                       lppname, op->name, op->level.ver, op->level.rel,
                       op->level.mod, op->level.fix);
      }

      rc = inu_do_exec (cmd);
   }

   return (rc);
}
