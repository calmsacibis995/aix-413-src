static char sccsid[] = "@(#)90  1.40  src/bos/usr/sbin/install/installp/inu_cleanup_opts.c, cmdinstl, bos411, 9436C411a 9/6/94 14:42:16";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_cleanup_opts
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *------------------------------------------------------------------*/

#include <installp.h>
#include <signal.h>
#include <instl_options.h>
#include <inu_vpd.h>
#include <inu_swvpd_check.h>
#include <inu_string.h>


/*--------------------------------------------------------------------------*
**
** NAME: inu_cleanup_opts ()
**
** FUNCTION: cleanup options...
**
** RETURN VALUE DESCRIPTION:  SUCCESS or FAILURE
**
**--------------------------------------------------------------------------*/

int inu_cleanup_opts (Option_t *cur_op, Option_t *next_op)
{
   char  status_file [L_tmpnam + 10];   /* path to the status file */
   char  buf [TMP_BUF_SZ];              /* generic character buffer */
   char  device [PATH_MAX];             /* path to device */
   char  lpp_opt_fil [L_tmpnam + 12];   /* path to lpp.options file */
   char  lppname [MAX_LPP_NAME + MAX_LEVEL_LEN]; /* top level lpp name */
   char  levname [MAX_LPP_NAME + MAX_LEVEL_LEN]; /* op name and level */
   char  opt_name [MAX_LPP_NAME];   /* option name from status file */
   char  status;                    /* status character from status file */
   FILE *status_fp;                 /* file ptr to failed lpp.options file */
   char *ptr;                       /* generic pointer. */
   Option_t *op;                    /* generic Option_t pointer. */
   int   any_succeed;               /* Flag for if any cleanups succeeded */
   int   any_failure;               /* Flag for if any cleanups failed */
   int   found;                     /* generic found flag. */
   int   rc;                        /* return code */


   inu_get_prodname (cur_op, lppname);

   (void) inu_set_savdir (cur_op->vpd_tree, cur_op);

  /*-------------------------------------------------------------------------
   * If the user has not give the -C flag, but we're cleaning up something
   * anyhow, print a message about what we're doing.  Explicitly requested
   * cleanup operations will generate a similar message in inu_process_opts
   *------------------------------------------------------------------------*/

   if (cur_op->operation != OP_CLEANUP_COMMIT  &&
       cur_op->operation != OP_CLEANUP_APPLY)
   {
      instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_CLEANING_I, 
                                                  INP_CLEANING_D));
      for (op = cur_op; op != next_op; op = op->next)
      {
         if (op->Status == STAT_CLEANUP)
         {
            inu_get_optlevname_from_Option_t (op, levname);
            instl_msg (SCREEN_LOG, "\t%s\n", levname);
         }
      }
      instl_msg (SCREEN_LOG, "\n");
   }

   /*-----------------------------------------------------------------------*
    * Build the path to where the "lpp.options" file is.
    *-----------------------------------------------------------------------*/
   strcpy (lpp_opt_fil, INU_TMPDIR);
   strcat (lpp_opt_fil, "/lpp.options");

   /*-----------------------------------------------------------------------*
    * Build the path to where the device is.  We provide the additional check
    * on vpd_tree to ensure that we don't de-reference a nil pointer when
    * we are applying the root part of a package for which the entire pkg
    * was requested (usr &root), when the usr part is already applied.
    *-----------------------------------------------------------------------*/
   if (NEEDS_DEVICE  &&  (cur_op->vpd_tree != VPDTREE_ROOT))
      strcpy (device, cur_op->bff->path);
   else
      strcpy (device, "NODEVICE");

   /*-----------------------------------------------------------------------*
    * Ignore all signals while we update the vpd...
    *-----------------------------------------------------------------------*/
   inu_ignore_all_sigs ();

   /*-----------------------------------------------------------------------*
    * Create a history record for the event HIST_CLEANUP...
    *-----------------------------------------------------------------------*/
   for (op = cur_op; op != next_op; op = op->next)
   {
      if (op->Status == STAT_CLEANUP)
      {
         /* 
         ** Determine whether this option has both LPP and PRODUCT 
         ** records in the SWVPD. If one of these records is missing 
         ** the SWVPD has gotten hosed in some way (perhaps the system
         ** crashed in the middle of an install). If either of the 
         ** above mentioned records are missing, the existing record is
         ** marked "broken" so the user can subsequently install the option.
         */

         if (inu_swvpd_check (op))
         {
            /* the SWVPD is hosed, update this option's */
            /* status and process the next one          */
            op->Status = STAT_CLEANUP_FAILED;
         }
         else
         { 
            /* Both the LPP and PRODUCT SWVPD records for this option exist */
            /* Create the history record...                                 */
            (void) inu_vpd_history (op, HIST_PENDING, HIST_CLEANUP);
         }
      }
   }  /* for */
 
   /*-----------------------------------------------------------------------*
    * cd to the directory where the cleanup control files reside.
    *-----------------------------------------------------------------------*/
   if ((rc = chdir (INU_LIBDIR)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                           CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, INU_LIBDIR);

      /*-------------------------------------------------------------------*
       * Update vpd to broken since we can't cleanup...
       *-------------------------------------------------------------------*/
      for (op = cur_op; op != next_op; op = op->next)
      {
         if (op->Status == STAT_CLEANUP)
         {
            (void) inu_vpd_lpp_prod (op, ST_BROKEN);
            (void) inu_vpd_history (op, HIST_BROKEN, HIST_CLEANUP);
            if (IF_INSTALL (op->op_type))
               (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);
            op->Status = STAT_CLEANUP_FAILED;
         }
      }
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * We're ready to call the cleanup script, 
    *-----------------------------------------------------------------------*
    * If 3.1 update:
    * If a "lpp.reject" script exists, use it, else just call inurecv...
    * In 3.1 updatep, the code put an "R" in as parameter 1 and the lpp.options
    * file path as parameter 2.  There were comments in the 3.1 code stating
    * that a "r" would be passed if a "soft" reject was requested and "R" is
    * for a "hard" reject. In looking at several 3.1 lpp.reject scripts, 
    * they defined parameter 1 as DEVICE, but never used it in the scripts.
    * We will assume some 3.1 reject scripts key off of the "R" and no scripts
    * used parameter 1 as a DEVICE.
    *-----------------------------------------------------------------------*/
   strcpy (buf, INU_LIBDIR);
   if (IF_3_1_UPDATE (cur_op->op_type))
   {
      /*------------------------------------------------------------------*
       * If 3.1 then the lpp.reject script is in one directory above the
       * inst_updt directory (the libdir) so back up one dir.
       *------------------------------------------------------------------*/
      if ((ptr = strrchr (buf, '/')) != NIL (char)) 
         *ptr = '\0';
      strcat (buf, "/lpp.reject");
      if (access (buf, R_OK) == 0)
      {
         strcat (buf, " R ");
         strcat (buf, lpp_opt_fil);
      }
      else
      {
         (void) strcpy (buf, "/usr/sbin/inurecv ");
         (void) strcat (buf, lppname);
      }
   }
   else
   {
      /*-------------------------------------------------------------------*
       * If a "lpp.cleanup" script exists, use that, else test for whether
       * this is a 3.1 install, if it is then mark options that were suppose
       * to be cleaned up as broken.
       *-------------------------------------------------------------------*/
      strcat (buf, "/lpp.cleanup");
      if (access (buf, R_OK) == 0)
      {
	 ptr = buf + strlen (buf);
         sprintf (ptr, " %s  %s", device, lpp_opt_fil);
      }
      else
      {
         if (IF_3_1_INSTALL (cur_op->op_type))
         {
            /*-----------------------------------------------------------*
             * Ignore all signals while we update the vpd...
             *-----------------------------------------------------------*/
            inu_ignore_all_sigs ();

            /*-----------------------------------------------------------*
             * Update vpd to broken since we can't cleanup...
             *-----------------------------------------------------------*/
            for (op = cur_op; op != next_op; op = op->next)
            {
               if (op->Status == STAT_CLEANUP)
               {
                  (void) inu_vpd_lpp_prod (op, ST_BROKEN);
                  (void) inu_vpd_history (op, HIST_BROKEN, HIST_CLEANUP);
                  if (IF_INSTALL (op->op_type))
                     (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);
                  op->Status = STAT_CLEANUP_FAILED;
               }
            }
	    instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_CLEANUP_NOTFND_E, 
                                 INP_CLEANUP_NOTFND_D), buf, lppname);

            /*-----------------------------------------------------------*
             * Set signal back to defaults...and return FAILURE
             *-----------------------------------------------------------*/
            inu_init_sigs ();
            return (FAILURE);
         }
         else
         {
            /*-----------------------------------------------------------*
             * ELSE it must be a 3.2 update or install...since there wasn't
             * a lpp.cleanup" script, then check to see if there's a default
             * cleanup script...if yes use it else return an error.
             * In this case we will leave the options in a "APPLYING" since
             * if the user would put the default scripts back where they
             * belong we could cleanup.  If we mark it as "BROKEN" then 
             * they won't be able to be cleaned up...
             *-----------------------------------------------------------*/
            strcpy (buf, DEF_CLEANUP_SCRIPT);
            if (access (buf, R_OK) == 0)
            {
	       ptr = buf + strlen (buf);
               sprintf (ptr, " %s %s", device, lpp_opt_fil);
            }
            else
            {
               instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                    CMN_CANT_OPEN_D), INU_INSTALLP_CMD, buf);
               return (FAILURE);
            }
         } /* else not a 3.1 install */
      } /* else access failed on "lpp.cleanup" file */
   } /* else not a 3.1 update */

   /*-----------------------------------------------------------------------*
    * Set signals for calling the cleanup script...
    *-----------------------------------------------------------------------*/
   inu_init_sigs ();

   /*-----------------------------------------------------------------------*
    * Call the ls for calling the cleanup script...
    *-----------------------------------------------------------------------*/
   rc = inu_do_exec (buf);

   /*-----------------------------------------------------------------------*
    * Set the following flags to indicate the different things that took
    * place when processing the status file.
    *-----------------------------------------------------------------------*/
   any_succeed = FALSE;
   any_failure = FALSE;

   /*-----------------------------------------------------------------------*
    * Ignore all signals while we update the vpd...
    *-----------------------------------------------------------------------*/
   inu_ignore_all_sigs ();

   if (rc == INUGOOD  ||  rc == INUCANCL)
   {
      /*-------------------------------------------------------------------*
       * Loop for every option and update the VPD to the correct state.
       * In this case we don't need a status file because all of the options
       * have the same status, (good or cancelled).
       *-------------------------------------------------------------------*/
      for (op = cur_op; op != next_op; op = op->next)
      {
         if (rc == INUGOOD) 
         {
            if (op->Status == STAT_CLEANUP) 
            {
               any_succeed = TRUE;
               op->Status = STAT_CLEANUP_SUCCESS;
               (void) inu_vpd_lpp_prod (op, ST_AVAILABLE);
               if (IF_INSTALL (op->op_type))
                  (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_PREVIOUS);
            }
         }
         else
         {
            if (op->Status == STAT_CLEANUP) 
            {
               any_failure = TRUE;
               op->Status = STAT_CLEANUP_FAILED;
               (void) inu_vpd_lpp_prod (op, ST_BROKEN);
               (void) inu_vpd_history (op, HIST_BROKEN, HIST_CLEANUP);
               if (IF_INSTALL (op->op_type))
                  (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);
            }
         }
      }  /* for */
   }  /* if (rc == INUGOOD  ||  rc == INUCANCL) */
   else
   {
      /*-------------------------------------------------------------------*
       * Build the path name for the status file.
       *-------------------------------------------------------------------*/
      if ( ! IF_3_1 (cur_op->op_type))
      {
         strcpy (status_file, INU_TMPDIR);
         strcat (status_file, "/status");
      }
      else
         strcpy (status_file, "./status");

      /*-------------------------------------------------------------------*
       * Check to see if there's a status file, if yes read it and update
       * the status for each option in the Option_t structure.
       *-------------------------------------------------------------------*/
      if ((status_fp = fopen (status_file, "r")) != NIL (FILE))
      {
         while (fgets (buf, TMP_BUF_SZ, status_fp) != NIL (char))
         {
            found = FALSE;
            status = '\0';
            opt_name[0] = '\0';
            sscanf (buf, "%c %s", &status, opt_name);

            for (op = cur_op; op != next_op; op = op->next) 
            {
               if (strcmp (op->name, opt_name) == 0)
               {
                  found = TRUE;
                  switch (status) 
                  {
                     case 's':
                        /*------------------------------------------*
                         * This option was cleaned up successfully.
                         *------------------------------------------*/
                        any_succeed = TRUE;
                        op->Status = STAT_CLEANUP_SUCCESS;
                        (void) inu_vpd_lpp_prod (op, ST_AVAILABLE);
			if (IF_INSTALL (op->op_type))
                           (void) inu_vpd_set_states (op, 
                                                  SET_HOLD_STATES_TO_PREVIOUS);
                        break;

                     case 'c':
                     case 'f':
                     default:
                        /*------------------------------------------*
                         * This option failed the cleanup attempt.
                         *------------------------------------------*/
                        any_failure = TRUE;
                        op->Status = STAT_CLEANUP_FAILED;
                        (void) inu_vpd_lpp_prod (op, ST_BROKEN);
                        (void) inu_vpd_history (op, HIST_BROKEN, HIST_CLEANUP);
			if (IF_INSTALL (op->op_type))
                           (void) inu_vpd_set_states (op, 
                                                    SET_HOLD_STATES_TO_BROKEN);
                        break;

                  } /* end switch (status) */

                  break; /* Option found, break out of "for" loop */

               } /* if (strcmp (option->name, opt_name) == 0) */

            } /* end for loop on options */

            if ( ! found)
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INTRN_OPTNOTFND_E, 
                                            INP_INTRN_OPTNOTFND_D), opt_name);

         } /* while (fgets (buf, 256, status_fp) != NIL (char)) */

         fclose (status_fp);

      } /* end if (fopen (status_file, "r") */
      else
      { 
         /*----------------------------------------------------------------*
          * Status file was not found, set all options that we were trying
	  * to cleanup to BROKEN.
          *----------------------------------------------------------------*/
	  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_CLEANUP_NOSTAT_E, 
                                                    INP_CLEANUP_NOSTAT_D)); 
	  for (op = cur_op; op != next_op; op = op->next) 
          {
             if (op->Status == STAT_CLEANUP) 
             {
                any_failure = TRUE;
                op->Status = STAT_CLEANUP_FAILED;
                (void) inu_vpd_lpp_prod (op, ST_BROKEN);
                (void) inu_vpd_history (op, HIST_BROKEN, HIST_CLEANUP);
		if (IF_INSTALL (op->op_type))
                   (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);
             }
          }
       }

    } /* end else not INUGOOD or INUCANCL */

   /*-----------------------------------------------------------------------*
    * The status file only reports failures so if there was a status file 
    * then the any_succeed flag won't be set so we need to scan the sop to 
    * find options that had a status or operation of CLEANUP and assume they 
    * succeeded since they weren't marked as FAILED in the status file.
    *-----------------------------------------------------------------------*/
    if ( ! any_succeed)
    {
       for (op = cur_op; op != next_op; op = op->next)
          if (op->Status == STAT_CLEANUP) 
          {
             any_succeed = TRUE;
             op->Status = STAT_CLEANUP_SUCCESS;
             (void) inu_vpd_lpp_prod (op, ST_AVAILABLE);
             if (IF_INSTALL (op->op_type))
                (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_PREVIOUS);
          }
    }

   /*-----------------------------------------------------------------------*
    * Set the signals back to their default settings...
    *-----------------------------------------------------------------------*/
    inu_init_sigs ();

   /*-----------------------------------------------------------------------*
    * Print out a message that the following options failed the cleanup.
    *-----------------------------------------------------------------------*/
    if (any_failure)
       inu_print_fail_stats (cur_op, next_op);

    rm_biron_files (cur_op, next_op, OP_CLEANUP_APPLY);

    return (SUCCESS);
}
