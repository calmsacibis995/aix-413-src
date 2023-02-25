static char sccsid[] = "@(#)74  1.9  src/bos/usr/sbin/install/ckprereq/report_warnings.c, cmdinstl, bos411, 9428A410j 6/15/94 11:43:57";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: report_warnings
 *            print_ifreq_warnings
 *            print_superseded_pkgs
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include <check_prereq.h>

/*--------------------------------------------------------------------------*
**
** NAME: report_warnings
**
** FUNCTION:  Prints various warnings that were detected in inu_ckreq
**            and installp.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void report_warnings
 (
boolean   fix_info_warnings,
boolean   fail_sop_warnings,
boolean * error
)
{

   print_warnings_hdr ();
   if (fix_info_warnings)
   {
      /* First, print warnings about ifreqs of already installed pkgs. */
      print_ifreq_warnings (error);
      RETURN_ON_ERROR;
      print_index_list_msg (report_anchor.misc_warnings_hdr, 
                            report_anchor.misc_warnings_tail,
                            WARN_NO_USR_SUPS_INFO, 
                            error);
      RETURN_ON_ERROR;
      print_index_list_msg (report_anchor.misc_warnings_hdr, 
                            report_anchor.misc_warnings_tail,
                            WARN_NO_USR_PRQ_INFO,
                            error);
      RETURN_ON_ERROR;
      print_index_list_msg (report_anchor.misc_warnings_hdr, 
                            report_anchor.misc_warnings_tail,
                            WARN_NO_PRQ_BASE,
                            error);
      RETURN_ON_ERROR;
      print_index_list_msg (report_anchor.misc_warnings_hdr, 
                            report_anchor.misc_warnings_tail,
                            WARN_MISS_USR_APPLD_ROOT,
                            error);
      RETURN_ON_ERROR;
      print_index_list_msg (report_anchor.misc_warnings_hdr, 
                            report_anchor.misc_warnings_tail,
                            WARN_MISS_USR_COMTD_ROOT,
                            error);
      RETURN_ON_ERROR;
      free_index_nodes_between (report_anchor.misc_warnings_hdr,
                                report_anchor.misc_warnings_tail);
      free_fix_info_index_hdr (report_anchor.misc_warnings_hdr,
                               report_anchor.misc_warnings_tail);
   }

   if (fail_sop_warnings)
   {
      print_alrdy_instlds_or_comtds ();
      print_sop_list_msg (check_prereq.Fail_SOP, STAT_ALL_KW_FAILURE);
      print_sop_list_msg (check_prereq.Fail_SOP, STAT_NO_USR_MEANS_NO_ROOT);
      if (check_prereq.mode == OP_APPLY)
      {
         print_sop_list_msg (check_prereq.Fail_SOP, STAT_DUPE_VERSION);
         print_sop_list_msg (check_prereq.Fail_SOP, STAT_NUTTIN_TO_APPLY);
         print_superseded_pkgs ();
      }
      else if (check_prereq.mode == OP_COMMIT)
      {
         print_sop_list_msg (check_prereq.Fail_SOP, STAT_NUTTIN_TO_COMMIT);
      }
      else if (check_prereq.mode == OP_REJECT)
      {
         if (check_prereq.deinstall_submode)
         {
            print_sop_list_msg (check_prereq.Fail_SOP, STAT_WARN_DEINST_3_1);
            print_sop_list_msg (check_prereq.Fail_SOP, STAT_WARN_DEINST_3_2);
            print_sop_list_msg (check_prereq.Fail_SOP, STAT_WARN_DEINST_MIG);
            print_sop_list_msg (check_prereq.Fail_SOP, STAT_NUTTIN_TO_DEINSTL);
         }
         else
         {
            print_sop_list_msg (check_prereq.Fail_SOP, STAT_NUTTIN_TO_REJECT);
         }
      }
   }
   inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_E_O_WARNINGS_I, 
                              CKP_E_O_WARNINGS_D));

} /* report_warnings */

/*--------------------------------------------------------------------------*
**
** NAME: print_ifreq_warnings
**
** FUNCTION:  Prints warnings about missing ifreqs of packages which are
**            already installed.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void print_ifreq_warnings 
 (
boolean * error
)
{
   boolean               print_hdr_msg;
   boolean               print_newline;
   char                  level_buf [MAX_LPP_FULLNAME_LEN];
   char                  name_lev [MAX_LPP_FULLNAME_LEN];
   char                  print_string [MAX_LPP_FULLNAME_LEN];
   requisite_list_type * requisite;
   fix_info_type       * fix_ptr;


   print_hdr_msg = TRUE;
   print_newline = FALSE;
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      fix_ptr = requisite->fix_ptr;
      if ( (fix_ptr->flags & REPT_IFREQ_WARNING) &&
          ! ( (fix_ptr->origin == DUMMY_FIX) ||
             (fix_ptr->origin == VPD && fix_ptr->parts_applied == 0)))
      {
         if (print_hdr_msg)
         {
            print_ifreq_warning_hdr1 ();
            print_hdr_msg = FALSE;
            print_newline = TRUE;
         } /* end if */
         sprintf (name_lev, "    %s %s", fix_ptr -> name,
                  get_level_from_fix_info (fix_ptr, level_buf));
         format_pkg_name_and_desc (FALSE, 
                                   print_string, 
                                   name_lev, 
                                   fix_ptr->description,
                                   error);
         RETURN_ON_ERROR;
         inu_msg (ckp_warn, "%s\n", print_string);

      } /* end if */
   } /* end for */   
   if (print_newline)
      inu_msg (ckp_warn, "\n");

   print_hdr_msg = TRUE;
   print_newline = FALSE;
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      fix_ptr = requisite->fix_ptr;
      if ( (fix_ptr->flags & REPT_IFREQ_WARNING) &&
          ( (fix_ptr->origin == DUMMY_FIX) ||
           (fix_ptr->origin == VPD && fix_ptr->parts_applied == 0)))
      {
         if (print_hdr_msg)
         {
            print_ifreq_warning_hdr2 ();
            print_hdr_msg = FALSE;
            print_newline = TRUE;

         } /* end if */
         if (fix_ptr->origin == DUMMY_FIX)
            get_criteria_string (&requisite->criteria, level_buf);
         else
            get_level_from_fix_info (fix_ptr, level_buf);

         sprintf (name_lev, "    %s %s", fix_ptr->name, level_buf);
         format_pkg_name_and_desc (FALSE, 
                                   print_string, 
                                   name_lev, 
                                   fix_ptr->description, 
                                   error);
         RETURN_ON_ERROR;
         inu_msg (ckp_warn, "%s\n", print_string);
      
      } /* end if */
   } /* end for */   
   if (print_newline)
      inu_msg (ckp_warn, "\n");

} /* print_ifreq_warnings */

/*--------------------------------------------------------------------------*
**
** NAME: print_superseded_pkgs
**
** FUNCTION:  Prints packages which were "discarded" because they were
**            superseded by other packages on the media.  This includes 
**            packages which were preempted by newer packages which were 
**            explicitly selected (ie. user explicitly picked both superseding
**            and superseded packages) and packages preempted by newer 
**            packages that were automatically picked by inu_ckreq (in 
**            evaluate_supersedes). 
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_superseded_pkgs (void)
{
   boolean           print_hdr_msg;
   char              name_lev [MAX_LPP_FULLNAME_LEN];
   index_list_type * index_node;
   Level_t           look_up_level;
   Option_t        * sop_ptr;
   fix_info_type   * fix_ptr;

   if (report_anchor.superseded_index_hdr == NIL (index_list_type))
      return;

   /*
    * Print pkgs which we threw out that are being superseded. 
    */
   print_hdr_msg = TRUE; 
   for (index_node = report_anchor.superseded_index_hdr -> next_index_node;
        index_node != report_anchor.superseded_index_tail;  
        index_node = index_node -> next_index_node)
   {
      sop_ptr = index_node->sop_ptr;
      if (print_hdr_msg)
      {
         inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_SUPERSEDED_UPDATES_HDR_I,
                  CKP_SUPERSEDED_UPDATES_HDR_D));
         print_hdr_msg = FALSE;
      }
      inu_get_optlevname_from_Option_t (sop_ptr, name_lev);

      inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_SUPERSEDED_BY2_I, 
                         CKP_SUPERSEDED_BY2_D),
               name_lev, sop_ptr->supersedes);

   } /* end if (fix_ptr...*/
   if (! print_hdr_msg)
      inu_msg (ckp_warn, "\n");

   free_index_nodes_between (report_anchor.superseded_index_hdr, 
                             report_anchor.superseded_index_tail);
   free_fix_info_index_hdr (report_anchor.superseded_index_hdr,
                            report_anchor.superseded_index_tail);

} /* print_superseded_pkgs */
