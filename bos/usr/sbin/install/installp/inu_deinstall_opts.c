static char     sccsid[] = "@(#)62      1.27  src/bos/usr/sbin/install/installp/inu_deinstall_opts.c, cmdinstl, bos411, 9439B411a 9/27/94 15:00:52";

/*---------------------------------------------------------------------
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_deinstall_opts
 *		inu_get_deinstall_script
 *              inu_rm_vpd_info
 *		process_status_file
 *		
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *---------------------------------------------------------------------*/

#include <stdio.h>
#include <pwd.h>
#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>

void inu_rm_vpd_info         (Option_t * op);

static void inu_set_deinst_dir      (Option_t * cur_op);
static int  inu_get_deinstal_script (char * deinstal_script);
static void process_status_file     (Option_t * cur_op, Option_t * next_op);

/* full path to custom deinstall script (if it exists) */
#define inu_get_custom_deinstal_script(custom) strcpy (custom, DEINST_DIR);\
                                               strcat (custom, "/lpp.deinstal")

static boolean any_failure = FALSE;
static char    DEINST_DIR [2 * MAX_LPP_NAME];

/*=========================================================================*
**
**  Function:  inu_deinstall_opts
**
**  Purpose:  Deinstall each of the options on the selected options list.
**
**  Notes:  Calling function
**
**  Returns:  Returns the return code from the call to inu_lpp_deinstall
**
**=========================================================================*/

int inu_deinstall_opts (Option_t * cur_op, Option_t * next_op)
{
   char            deinstal_script [2 * MAX_LPP_NAME];
   char            deinst_cmd [PATH_MAX + 1];
   Option_t       *op;
   int             rc;

   /*------------------------------------------------------------------------*/
   /* Determine the deinstl directory and set the global variable DEINST_DIR */
   /*------------------------------------------------------------------------*/
   inu_set_deinst_dir (cur_op);

   /*-------------------------------------------*/
   /* cd to DEINST_DIR.  Make it if you have to */
   /*-------------------------------------------*/
   if ((rc = chdir (DEINST_DIR)) != SUCCESS)
   {
      if (inu_mk_dir (DEINST_DIR) != SUCCESS)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                              CMN_NO_MK_DIR_D), INU_INSTALLP_CMD, DEINST_DIR);

         for (op = cur_op; op != next_op; op = op->next)
            op->Status = STAT_CANCEL;

         return (INUCREAT);
      }
      if ((rc = chdir (DEINST_DIR)) != SUCCESS)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                              CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, DEINST_DIR);

         for (op = cur_op; op != next_op; op = op->next)
            op->Status = STAT_CANCEL;

         return (INUCHDIR);
      }
   }

   inu_set_savdir (cur_op->vpd_tree, cur_op);

   /*-----------------------------------------------------------*/
   /* Get the deinstal script to use (custom or default script) */
   /*-----------------------------------------------------------*/
   if ((rc = inu_get_deinstal_script (deinstal_script)) != INUGOOD)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_DEINST_SCRIPT_E, 
                           INP_NO_DEINST_SCRIPT_D), DEF_DEINSTAL_SCRIPT);

      for (op = cur_op; op != next_op; op = op->next)
         op->Status = STAT_CANCEL;

      return (INUACCS);
   }

   inu_ignore_all_sigs ();

   /*----------------------------------------------------*/
   /* Update the swvpd and mark every option as FAILURE. */
   /*----------------------------------------------------*/
   for (op = cur_op; op != next_op; op = op->next)
   {
      op->Status = STAT_FAILURE;
      inu_vpd_lpp_prod (op, ST_DEINSTALLING);
      inu_vpd_history (op, HIST_PENDING, HIST_DEINSTALL);
   }

   /*------------------------------------------------*/
   /* Create the system call to the deinstall script */
   /*------------------------------------------------*/
   sprintf (deinst_cmd, "%s %s/lpp.options", deinstal_script, INU_TMPDIR);

   /*------------------------------*/
   /* Execute the deinstall script */
   /*------------------------------*/
   inu_init_sigs ();
   rc = inu_do_exec (deinst_cmd);

   inu_ignore_all_sigs ();

   /*----------------------------------------*/
   /* If the deinstall script was successful */
   /*----------------------------------------*/
   if (rc == INUGOOD)
   {
      inu_remove_all_save_files (cur_op->vpd_tree, cur_op, next_op);

      /*-----------------------------------------------------------*/
      /* Mark all options as successful and remove vpd information */
      /*-----------------------------------------------------------*/
      for (op = cur_op; op != next_op; op = op->next)
      {
         op->Status = STAT_SUCCESS;
         inu_rm_vpd_info (op);     
      }
   }

   /*--------------------------------------*/
   /* else deinstallation was unsuccessful */
   /*--------------------------------------*/
   else   
      process_status_file (cur_op, next_op);

   inu_init_sigs ();

   if (any_failure)
      inu_print_fail_stats (cur_op, next_op);

   return (any_failure);

} /* inu_deinstall_opts */


/*=========================================================================*
** NAME: process_status_file
**
** FUNCTION: This function is only called when there was some failure
**           from inu_lpp_deinstall.  This would imply that there are
**           failures marked in the status file.  It processes the status
**           file and for each success it marks the op->status = SUCCESS.
**           Then for each op it updates the history VPD with the AVAILABLE
**           state upon success, BROKEN state upon failure.
**
** RETURNS: None.
**
**=========================================================================*/

static void process_status_file (Option_t * cur_op, Option_t * next_op)
{
   Option_t       *op;
   FILE           *fd;
   char            stat;
   char            status_file [PATH_MAX + 1];
   char            opt_name [MAX_LPP_NAME + 1];
   char            buf [TMP_BUF_SZ + 1];

   strcpy (status_file, INU_TMPDIR);
   strcat (status_file, "/status");

   if ((fd = fopen (status_file, "r")) != NULL)
   {
      /*-----------------------------------------------*/
      /* Process each status record in the status file */
      /*-----------------------------------------------*/
      while (fgets (buf, TMP_BUF_SZ, fd) != NULL)
         /*----------------------------*/
         /* Get a single status record */
         /*----------------------------*/
         if (sscanf (buf, "%c %s", &stat, opt_name) == 2  &&  stat == 's')
         {
            /*---------------------------------*/
            /* Find the option in the sop list */
            /*---------------------------------*/
            for (op = cur_op;
                 op != next_op  &&  strcmp (op->name, opt_name) != 0;
                 op = op->next);

            /*--------------------------*/
            /* If we found one, mark it */
            /*--------------------------*/
            if (op != next_op)
               op->Status = STAT_SUCCESS;

         } /* end if */
      fclose (fd);
   }
   else
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_DEINST_NOSTAT_E, 
                                                INP_DEINST_NOSTAT_D));

   /*------------------------------------------------*/
   /* Update vpd HISTORY datebase with any failures. */
   /*------------------------------------------------*/
   for (op = cur_op; op != next_op; op = op->next)
      if (op->Status != STAT_SUCCESS)
      {
         any_failure = TRUE;
         inu_vpd_lpp_prod (op, ST_BROKEN);
         inu_vpd_history (op, HIST_BROKEN, HIST_DEINSTALL);
         (void) inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);
      }

} /* process_status_file */


/*=========================================================================*
** NAME:      inu_rm_vpd_info
**
** FUNCTION:  Removes records corresponding to op from the HISTORY, PRODUCT, 
**            and LPP tables.
**
** RETURNS:   None.
**
**=========================================================================*/

void inu_rm_vpd_info (Option_t * op)
{
   lpp_t           lpp;
   prod_t          prod;
   hist_t          hist;
   int             vpdrc;

  /*--------------------------------------------------------------------*
   * Get the lpp_id of the product option to access the history records 
   *--------------------------------------------------------------------*/
   vpdrc = vpdreslpp_name (op->name, &hist.lpp_id);
   inu_ck_vpd_ok (vpdrc, "\tvpdreslpp_name LPP_TABLE.name = %s\n", op->name) ;

  /*------------------------------------------------------*
   * Remove all HISTORY records matching the lpp_id of op
   *------------------------------------------------------*/
   vpdrc = vpddelall (HISTORY_TABLE, HIST_LPP_ID, &hist);
   inu_ck_vpd_sys_rc (vpdrc);

  /*-----------------------------------------------*
   * Remove all PRODUCT records matching op's name
   *-----------------------------------------------*/
   strcpy (prod.lpp_name, op->name);
   vpdrc = vpddelall (PRODUCT_TABLE, PROD_LPP_NAME, &prod);
   inu_ck_vpd_sys_rc (vpdrc);

  /*-------------------------------------------*
   * Remove all LPP records matching op's name
   *-------------------------------------------*/
   strcpy (lpp.name, op->name);
   vpdrc = vpddelall (LPP_TABLE, LPP_NAME, &lpp);
   inu_ck_vpd_sys_rc (vpdrc);

} /* inu_rm_vpd_info */


/*=========================================================================*
**
** NAME: inu_get_deinstal_script
**
** FUNCTION:  Returns the full path to the custom deinstall script if it 
**            is accessible (and executable), otherwise the path of the 
**            default deinstall script.
**
** RETURNS: INUGOOD if either script is executable.
**          INUACCS if neither is executable.
**
**=========================================================================*/

static int inu_get_deinstal_script (char * deinstal_script)
{
   char   custom_deinstal_script[2 * MAX_LPP_NAME];

   inu_get_custom_deinstal_script (custom_deinstal_script);

   if (access (custom_deinstal_script, X_OK) == 0)
      strcpy (deinstal_script, custom_deinstal_script);
   else if (access (DEF_DEINSTAL_SCRIPT, X_OK) == 0) 
      strcpy (deinstal_script, DEF_DEINSTAL_SCRIPT);
   else
      return (INUACCS);

   return (INUGOOD);

} /* inu_get_deinstal_script */


/*=========================================================================*
**
** NAME: inu_set_deinst_dir
**
** FUNCTION:  Returns the full path to the custom deinstall script if it
**
** RETURNS:  None.
**
**=========================================================================*/

static void inu_set_deinst_dir (Option_t * op)
{
    switch (op->vpd_tree)
    {
        case VPDTREE_USR:
            strcpy(DEINST_DIR, "/usr/lpp/");
            break;
        case VPDTREE_ROOT:
            strcpy(DEINST_DIR, "/lpp/");
            break;
        case VPDTREE_SHARE:
            strcpy(DEINST_DIR, "/usr/share/lpp/");
            break;
    }

    strcat (DEINST_DIR, op->prodname);
    strcat (DEINST_DIR, "/deinstl");

} /* inu_set_deinst_dir */
