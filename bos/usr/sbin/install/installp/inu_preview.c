static char     sccsid[] = "@(#)80      1.23  src/bos/usr/sbin/install/installp/inu_preview.c, cmdinstl, bos411, 9428A410j 5/24/94 13:19:16";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_calc_stats_table_values
 *		inu_mark_all_as_selected
 *		inu_show_apply_stats_table
 *		inu_show_commit_stats_table
 *		inu_show_reject_stats_table
 *		inu_show_stats_table
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

/*--------------------------------------------------------------------------*
**                        Include Files 
**--------------------------------------------------------------------------*/
#include <installp.h>
#include <instl_define.h>
#include <instl_options.h>


/*--------------------------------------------------------------------------*
**                         Function Prototypes 
**--------------------------------------------------------------------------*/

void inu_show_apply_stats_table     (Option_t *, Option_t *, Option_t *, 
                                     stats_table *);
void inu_show_commit_stats_table    (Option_t *, Option_t *, Option_t *, 
                                     stats_table *);
void inu_show_reject_stats_table    (Option_t *, Option_t *, Option_t *, 
                                     stats_table *);
void inu_show_deinstall_stats_table (Option_t *, Option_t *, Option_t *, 
                                     stats_table *);
void inu_print_other_brokens        (Option_t *);
void inu_print_brokens              (Option_t *);
void inu_print_not_founds           (Option_t *);
void inu_print_mult_versions        (Option_t *);


/*--------------------------------------------------------------------------*
**                          Function Definitions 
**--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
**
**  Function:      inu_do_preview_stats_table
**
**  Description:   Calculates the values needed for the pre-installation
**                 processing statistics table.  Then displays the table
**                 in its entirety.
**
**  Parameters:     
**
**  Returns:       None
**
**--------------------------------------------------------------------------*/

void inu_show_stats_table 
 (Option_t *sop,     /* sop list containing passing options */
  Option_t *failsop, /* sop list containing all failing options */
  Option_t *savesop, /* sop list for when an installp fix is being installed */ 
  stats_table *sd)

{
  /** -------------------------------------------------------------------- *
   **  If no options were selected and none deferred 
   **  -or- if we had an "all" keyword failure, 
   **  then we don't wanna show the statistics table.
   ** -------------------------------------------------------------------- */


   if ((sd->selected == 0  &&  sd->deferred == 0) 
                       ||  
       (failsop->next != NIL (Option_t)  &&  
        failsop->next->Status == STAT_ALL_KW_FAILURE))
   {
      return;
   }

   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_STAT_HDR_I, INP_STAT_HDR_D));

   if      (a_FLAG)
      inu_show_apply_stats_table  (sop, failsop, savesop, sd);

   else if (c_FLAG)
      inu_show_commit_stats_table (sop, failsop, savesop, sd);

   else if (r_FLAG)
      inu_show_reject_stats_table (sop, failsop, savesop, sd);

   else if (u_FLAG)
      inu_show_deinstall_stats_table (sop, failsop, savesop, sd);

   else 
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_AND_MAJOR_FLAG_E, 
                                     INP_PREV_AND_MAJOR_FLAG_D));
}

/*--------------------------------------------------------------------------*
**
**  Function:      inu_show_apply_stats_table
**
**  Description:   Calculates the values needed for the pre-installation
**                 processing statistics table for the apply case.  Then 
**                 displays the table in its entirety.
**
**  Parameters:     
**
**  Returns:       None
**
**--------------------------------------------------------------------------*/

void inu_show_apply_stats_table 
 (Option_t *sop,     /* sop list containing passing options */
  Option_t *failsop, /* sop list containing all failing options */
  Option_t *savesop, /* sop list for when an installp fix is being installed */ 
  stats_table *pip_stats_data)

{
  /** --------------------------------------------------------------------- *
   **  Display the pip stats table to INFO_MSG
   ** --------------------------------------------------------------------- */

   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_SELECTED_I, 
                        INP_NUM_SELECTED_D), 
                        pip_stats_data->selected + pip_stats_data->deferred);

   if (pip_stats_data->passed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_PASSED_PIV_I, 
                           INP_NUM_PASSED_PIV_D), pip_stats_data->passed);

   if (pip_stats_data->failed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_FAILED_PIV_I, 
                           INP_NUM_FAILED_PIV_D), pip_stats_data->failed);

   if (pip_stats_data->to_be_seded > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_SUBSTITUTED_I, 
                           INP_NUM_SUBSTITUTED_D), pip_stats_data->to_be_seded);

   if (pip_stats_data->already_done > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_ALREADY_INST_I, 
                        INP_NUM_ALREADY_INST_D), pip_stats_data->already_done);

   if (pip_stats_data->deferred > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_DEFERRED_I, 
                           INP_NUM_DEFERRED_D), pip_stats_data->deferred); 
  
   if (pip_stats_data->requisites > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_ADDTL_REQS_I, 
                           INP_ADDTL_REQS_D), pip_stats_data->requisites);

   if (pip_stats_data->unsel_mand_failed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_ADDTL_MAND_I, 
                          INP_ADDTL_MAND_D), pip_stats_data->unsel_mand_failed);

   instl_msg (INFO_MSG, "  ----\n");

   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_TB_INST_I, 
                        INP_TOT_TB_INST_D), 
                        pip_stats_data->passed + pip_stats_data->requisites);

   instl_msg (INFO_MSG, "\n");

   if (pip_stats_data->deferred > 0)
   {
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_DEFERREDS_NOTE_I, 
                                                INP_DEFERREDS_NOTE_D));
      instl_msg (INFO_MSG, "\n");
   }
}

/*--------------------------------------------------------------------------*
**
**  Function:    inu_show_reject_stats_table
**
**  Purpose:     Displays statistics for all packages that went thru 
**               pre-reject processing.
**
**  Notes:       None
**
**  Returns:     None
**
**--------------------------------------------------------------------------*/

void inu_show_reject_stats_table 
 (Option_t *sop,     /* sop list containing passing options */
  Option_t *failsop, /* sop list containing all failing options */
  Option_t *savesop, /* sop list for when an installp fix is being installed */ 
  stats_table *prp_stats_data)

{
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_SEL_REJ_I, 
                        INP_TOT_SEL_REJ_D), prp_stats_data->selected);

   if (prp_stats_data->passed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_PASS_PRV_I, 
                           INP_TOT_PASS_PRV_D), prp_stats_data->passed);

   if (prp_stats_data->failed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_FAIL_PRV_I, 
                           INP_TOT_FAIL_PRV_D), prp_stats_data->failed);

   if (prp_stats_data->requisites > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_ADTL_DEPEND_I, 
                           INP_ADTL_DEPEND_D), prp_stats_data->requisites); 
 
   instl_msg (INFO_MSG, "  ----\n");
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_TBR_I, INP_TOT_TBR_D), 
                        prp_stats_data->passed + prp_stats_data->requisites);

}
/*--------------------------------------------------------------------------*
**
**  Function:    inu_show_deinstall_stats_table
**
**  Purpose:     Displays statistics for all packages that went thru 
**               pre-deinstall processing.
**
**  Notes:       None
**
**  Returns:     None
**
**--------------------------------------------------------------------------*/

void inu_show_deinstall_stats_table 
 (Option_t *sop,     /* sop list containing passing options */
  Option_t *failsop, /* sop list containing all failing options */
  Option_t *savesop, /* sop list for when an installp fix is being installed */ 
  stats_table *prp_stats_data)

{
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_SEL_TBD_I, 
                        INP_TOT_SEL_TBD_D), prp_stats_data->selected);

   if (prp_stats_data->passed> 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_PASS_PDV_I, 
                           INP_TOT_PASS_PDV_D), prp_stats_data->passed);

   if (prp_stats_data->failed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_FAIL_PDV_I, 
                           INP_TOT_FAIL_PDV_D), prp_stats_data->failed);

   if (prp_stats_data->requisites > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_ADTL_DEP_DEINST_I, 
                           INP_ADTL_DEP_DEINST_D), prp_stats_data->requisites); 
 
   instl_msg (INFO_MSG, "  ----\n");

   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_TBD_I, INP_TOT_TBD_D), 
                        prp_stats_data->passed + prp_stats_data->requisites);
}

/*--------------------------------------------------------------------------*
**
**  Function:    inu_show_commit_stats_table
**
**  Purpose:     Displays stats for the commit operation.
**
**  Notes:       None
**
**  Returns:     None
**
**--------------------------------------------------------------------------*/

void inu_show_commit_stats_table 
 (Option_t *sop,     /* sop list containing passing options */
  Option_t *failsop, /* sop list containing all failing options */
  Option_t *savesop, /* sop list for when an installp fix is being installed */ 
  stats_table *pcp_stats_data)

{
  /** --------------------------------------------------------------------- *
   **  Display the actual stats table. 
   ** --------------------------------------------------------------------- */
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_SELECTED_TBC_I, 
                     INP_TOT_SELECTED_TBC_D), pcp_stats_data->selected);

   if (pcp_stats_data->passed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_PASSED_PIV_I, 
                           INP_NUM_PASSED_PIV_D), pcp_stats_data->passed);

   if (pcp_stats_data->failed > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_FAILED_PIV_I, 
                           INP_NUM_FAILED_PIV_D), pcp_stats_data->failed);

   if (pcp_stats_data->already_done > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NUM_ALREADY_COMMITTED_I, 
                    INP_NUM_ALREADY_COMMITTED_D), pcp_stats_data->already_done);
   
   if (pcp_stats_data->requisites > 0)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_ADTL_REQ_COMMIT_I, 
                           INP_ADTL_REQ_COMMIT_D), pcp_stats_data->requisites);

   instl_msg (INFO_MSG, "  ----\n");
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOT_TBC_I, INP_TOT_TBC_D), 
                        pcp_stats_data->passed + pcp_stats_data->requisites);

   instl_msg (INFO_MSG, "\n");
}

/*--------------------------------------------------------------------------*
**
**  Function:    inu_calc_stats_table_values
**
**  Purpose:     Calculates the statistics for the pre-installation stats
**               report.
**
**  Notes:       Assumes that the bff linked list hasn't been freed (by
**               the inu_cp_toc_to_vpd routine).
**
**  Returns:     None
**
**--------------------------------------------------------------------------*/

void  inu_calc_stats_table_values 
 (
  Option_t           *sop,     /* all options that passed pre-inst processing */
  Option_t           *failsop, /* all options that failed pre-inst processing */
  Option_t           *savesop, /* non-installp fix items saved on this sop */ 
  stats_table        *sd       /* statistics data */
)

{
   Option_t *op;

  /** --------------------------------------------------------------------- *
   **  Initialize all the to-be-calculated values to 0.
   ** --------------------------------------------------------------------- */

   sd->selected          = 0;
   sd->passed            = 0;
   sd->failed            = 0;
   sd->to_be_seded       = 0;
   sd->deferred          = 0;
   sd->requisites        = 0;
   sd->already_done      = 0;
   sd->unsel_mand_failed = 0;
   sd->unsel_mand_passed = 0;

  /** ---------------------------------------------------------------------- *
   **  Process the failure sop for the appropriate statistics. 
   ** ---------------------------------------------------------------------- */

   for (op  = failsop->next; 
        op != NIL (Option_t);
        op  = op->next) 
   {
      if (IF_OTHER_PART_ON_SOP (op->flag))
         continue;

     /** ----------------------------------------------------------------- *
      **  Don't wanna count any of the following warning cases
      ** ----------------------------------------------------------------- */

      if (op->Status == STAT_WARN_DEINST_MIG  ||
          op->Status == STAT_WARN_DEINST_3_1  ||
          op->Status == STAT_WARN_DEINST_3_2)
         continue;

     /** ----------------------------------------------------------------- *
      **  Don't wanna count dupes, unless they were explicitly asked 
      **  for by level. 
      ** ----------------------------------------------------------------- */

      if (op->Status == STAT_DUPE_VERSION  &&  ! IF_SELECTED (op->flag))
         continue;

     /** ----------------------------------------------------------------- *
      **  Set the selected count ... 
      ** ----------------------------------------------------------------- */

      if (op->bff != NIL (BffRef_t)  
                    &&  
          op->bff->action == ACT_REQUIRED_UPDT)
      {
         if (IF_SELECTED (op->flag))
            sd->selected ++;
         else
            sd->unsel_mand_failed ++;
      }
      else
      {
         sd->selected ++;

         if (op->Status == STAT_TO_BE_SUPERSEDED) 
         {
            sd->to_be_seded ++;
            continue;
         }
      }

     /** ----------------------------------------------------------------- *
      **  Set the fail count ... 
      ** ----------------------------------------------------------------- */

      if (op->Status != STAT_ALREADY_INSTALLED  
                        &&  
          op->Status != STAT_BASE_ALREADY_INSTALLED
                        &&  
          op->Status != STAT_ALREADY_SUPERSEDED
                        && 
          op->Status != STAT_ALREADY_COMMITTED)
      {
         sd->failed ++;
      }
      else
      {
         sd->already_done ++;
      }
   }

  /** ---------------------------------------------------------------------- *
   **  Process the saved sop (the sop that's put on the back-burner so that 
   **  installp fixes can first be processed) to get the number of deferred
   **  packages. 
   ** ---------------------------------------------------------------------- */

   for (op  = savesop->next; 
        op != NIL (Option_t);
        op  = op->next) 
   {
      sd->deferred ++;
   }

  /** ---------------------------------------------------------------------- *
   **  Process the (success) sop for the appropriate statistics.
   ** ---------------------------------------------------------------------- */

   for (op  = sop->next; 
        op != NIL (Option_t);
        op  = op->next) 
   {

      if (IF_OTHER_PART_ON_SOP (op->flag))
         continue;

      if (op->bff != NIL (BffRef_t)  &&  (op->bff->action == ACT_REQUIRED_UPDT))
      {
         if (IF_SELECTED (op->flag))
            sd->passed ++;
         else
            sd->unsel_mand_passed ++;
      }
      else 
      {
         if (IF_SELECTED (op->flag))
            sd->passed ++;
      }

      if (IF_SELECTED (op->flag))
         sd->selected ++;
      else
         sd->requisites ++;
   } 

}

/*--------------------------------------------------------------------------*
**
**  Function:    inu_mark_all_as_selected 
**
**  Purpose:     Marks all options in the selected options list as being
**               explicitly selected by the user.  
**
**  Notes:       This function should be called before the call to inu_ckreq 
**               which pulls in requisites (which are not considered to be
**               explicitly asked for by the user).
**
**  Returns:     None
**
**--------------------------------------------------------------------------*/

void inu_mark_all_as_selected (Option_t *sop)
{
   Option_t *op;
   char s1[500];

   for (op  = sop;
        op != NIL (Option_t);
        op  = op->next)
   {
      if (op->Status != STAT_DUPE_VERSION)
         op->flag = SET_SELECTED_BIT (op->flag);
   }
}
