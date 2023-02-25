static char sccsid[] = "@(#)59  1.60.1.46  src/bos/usr/sbin/install/ckprereq/report_failures.c, cmdinstl, bos41J, 9517A_all 4/20/95 19:14:07";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *            report_failures
 *            print_ckp_non_req_failures
 *            print_requisite_failures
 *            print_superseded_pkgs_of_failed_auto_supersedes
 *            print_failed_requisite_list
 *            print_failed_subgraph
 *            print_dependents_list
 *            print_requisite_and_dependent_list
 *            print_group_requisites
 *            
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include <check_prereq.h>

/*
 * Prototypes and global variables.
 */

static void print_ckp_non_req_failures (
boolean   cmd_line_auto_sups_failure,
boolean * error);

static void print_dependents_list (
index_list_type * starting_dependent, 
boolean           is_missing_requisite,
boolean         * error);

static boolean print_failed_requisite_list ( 
short     failure_type,
boolean * error);

static void print_failed_subgraph (
fix_info_type       * fix_ptr, 
requisite_list_type * req_node,
short                 indent_lev,
short                 num_spaces,
boolean               const print_hdr_msg);

static void print_requisite_failures (
boolean   auto_sups_failed,
boolean * error);

/*
 * LOCAL MACROS:
 */
#define TYPE_A_FAILURE(failure_sym)                                       \
                        (((check_prereq.mode == OP_REJECT) &&             \
                          ((failure_sym == COM_PRQ_SYM) ||                \
                           (failure_sym == DEINSTBLTY_FAIL_SYM)))         \
                                            ||                            \
                         ((check_prereq.mode != OP_REJECT) &&             \
                          ((failure_sym == UNAVLBL_SYM) ||                \
                               (failure_sym == NOT_APPLD_CMD_LINE_SYM))))

#define TYPE_B_FAILURE(fix_ptr)                                           \
                        (fix_ptr->origin == DUMMY_GROUP)


/* See check_prereq.h for global TYPE_C definition */

#define TYPE_D_FAILURE(failure_sym)                                       \
                        (failure_sym == BROKEN_REQ_SYM)

#define TYPE_E_FAILURE(failure_sym)                                       \
                        (failure_sym == CONFL_LEVEL_SYM)

/* Used to identify the types of failures being printed */
#define TYPE_A   1
#define TYPE_B   2
#define TYPE_C   3
#define TYPE_D   4
#define TYPE_E   5
#define TYPE_Z   6

#define INIT_INDENT_LEV      0    /* used for printing requisite subgraph
                                     for V4 failures */
#define INIT_SPACES          4    /* same as above */               

/* Used to print indented group requisites. */
#define GROUP_INDENT_VAL     4
#define FIRST_TIME        TRUE    

/*--------------------------------------------------------------------------*
**
** NAME: report_failures
**
** FUNCTION:  Prints failures and warnings encountered in the process of 
**            checking requisites.  When called from installp, also prints
**            up-front failures found there (so that failures can be 
**            grouped together).
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
report_failures 
 (
boolean  instp_non_req_failures,
boolean  ckp_non_req_failures,
boolean  requisite_failures,
boolean  non_req_auto_sups_failure,
boolean  req_auto_sups_failure,
boolean * error
)
{

   if (! check_prereq.called_from_command_line)
   {
      print_failures_hdr ();
      if (instp_non_req_failures)
         print_instp_non_req_failures (check_prereq.Fail_SOP);

      if (ckp_non_req_failures) {
         /*
          * Now print non requisite pre-installation failures which 
          * are caught by inu_ckreq.  These should really be caught by
          * installp but it's more efficient if inu_ckreq does it.
          */
         print_ckp_non_req_failures (non_req_auto_sups_failure, error);
         RETURN_ON_ERROR;
      }
   }

   if (requisite_failures) 
   {
      print_requisite_failures (req_auto_sups_failure, error);
      RETURN_ON_ERROR;
   }
   
   if (! check_prereq.called_from_command_line)
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_E_O_FAILURES_I, CKP_E_O_FAILURES_D));

   /*
    * Add failure nodes to the fail sop so that installp may print its
    * re-cap of pre-installation failures.
    */
   if (! check_prereq.called_from_command_line)
      add_failures_to_fail_sop ();

   cleanup_report_failures ();

}  /* end report_failures */

/*--------------------------------------------------------------------------*
**
** NAME: print_ckp_non_req_failures
**
** FUNCTION:  Prints failures detected in evaluate failures, which do not fall
**            under the "requisite failure" category.  These are fairly rare
**            failures which installp doesn't catch up-front.  The failures
**            are printed in a form where a symbol denotes the type of failure
**            and a "legend" or key is printed to explain each failure in 
**            detail.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
print_ckp_non_req_failures 
 (
boolean auto_sups_failure,  /* flag used to print a msg */
boolean * error
)
{
   boolean               found;
   boolean               printed_hdr_msg;
   fix_info_type       * fix_ptr;
   requisite_list_type * requisite; 
   requisite_flags_type  req_flags;

   /*
    * First scan the fix_info table looking for updates that are being
    * requested for a different level than the base which is installed 
    * (means we don't flag the failure as a potentially confusing requisite
    * failure msg).
    */
   printed_hdr_msg = FALSE;
   for (fix_ptr = check_prereq.fix_info_anchor->next_fix;
        fix_ptr != NIL (fix_info_type);
        fix_ptr = fix_ptr->next_fix)
   {
      if ((fix_ptr->flags & REPT_CMD_LINE_FAILURE)           &&
          (fix_ptr->apply_state == CANNOT_APPLY_CONFL_LEVEL)   &&
          (IF_UPDATE (fix_ptr->op_type)))
      {
         if (!printed_hdr_msg)
         {
            /*
             * NOTE: Failure msg tag implies it is specific to 31 update
             * failures but it is generic enough to apply to all updates
             * that cannot install on a given base.
             */
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_31_UPDT_REQ_FAILURE_I,
                               CKP_31_UPDT_REQ_FAILURE_D));
    
            printed_hdr_msg = TRUE;
         }
         print_failed_pkg (fix_ptr, TRUE, &EMPTY_CRITERIA, error);
         RETURN_ON_ERROR;
      } /* end if */
   } /* end for */
   if (printed_hdr_msg)
   {
      inu_msg (ckp_errs, "\n");
   }

   /*
    * Re-Scan the fix_info table looking for other type REPT_CMD_LINE_FAILURES
    */
   printed_hdr_msg = FALSE;
   for (fix_ptr = check_prereq.fix_info_anchor->next_fix;
        fix_ptr != NIL (fix_info_type);
        fix_ptr = fix_ptr->next_fix)
   {
      if ((fix_ptr->flags & REPT_CMD_LINE_FAILURE) 
                          &&
          ! ((fix_ptr->apply_state == CANNOT_APPLY_CONFL_LEVEL)   &&
             (IF_UPDATE (fix_ptr->op_type))))
      {
         if (!printed_hdr_msg)
         {
            print_ckp_non_req_failures_hdr (auto_sups_failure);
            printed_hdr_msg = TRUE;
         }

         /*
          * Need to do a quick scan through the anchor's requisite list to 
          * get the requisite info for this fix (where we keep useful failure 
          * information).  This is ugly and potentially costly, but these
          * types of failures are rare enough not to care about performance.
          */
         requisite = check_prereq.fix_info_anchor->requisites;
         found = FALSE;
         while ( (! found) && (requisite != NIL (requisite_list_type)))
         {
            if (requisite->fix_ptr == fix_ptr)
               found = TRUE;
            else
               requisite = requisite->next_requisite;
         }

         /*
          * Find out the specific reason for failure and set a character in
          * a global array that will be used to print a key for all failures
          * that occurred.
          */
         fix_ptr->failure_sym = determine_failure_reason (fix_ptr, requisite,
                                                          TRUE);
         (void) add_sym_to_rf_legend (fix_ptr->failure_sym);

         print_failed_pkg (fix_ptr, TRUE, & (requisite->criteria), error);
         RETURN_ON_ERROR;
      } /* end if */
   } /* end for */
   if (printed_hdr_msg)
   {
      inu_msg (ckp_errs, "\n");
      print_report_failure_legend (FALSE);
   }

} /* end print_ckp_non_req_failures */

/*--------------------------------------------------------------------------*
**
** NAME: print_requisite_failures
**
** FUNCTION:  Prints all failures which fall under the category of 
**            "requisite failure."  
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void 
print_requisite_failures 
 (
boolean   auto_supersedes_failed,  /* flag used to print a msg */
boolean * error
)
{
   boolean                found;
   boolean                printed_group_failures = FALSE;
   boolean                printed_misc_failures = FALSE;
   boolean                print_hdr_msg;
   boolean                print_newline;
   char                   fix_name [MAX_LPP_FULLNAME_LEN];
   char                   name_lev [MAX_LPP_FULLNAME_LEN];
   char                   print_string [MAX_LPP_FULLNAME_LEN];
   char                   count_str [10];
   char                   tmp_str [10];
   fix_info_type        * fix_ptr;
   requisite_list_type  * requisite;
   requisite_flags_type   req_flags;
   short                  count;
   index_list_type      * index_node;
   index_list_type      * index_node2;

   if (! check_prereq.called_from_command_line)
   {
      print_requisite_failures_hdr (auto_supersedes_failed);

      /*
       * First, print the packages requested from the command line 
       * which failed requisite tests.
       */
      count = 1;
      for (fix_ptr = check_prereq.fix_info_anchor->next_fix;
           fix_ptr != NIL (fix_info_type);
           fix_ptr = fix_ptr -> next_fix)
      {
         if (fix_ptr->flags & REPT_REQUISITE_FAILURE)
         {
            if (check_prereq.instp_verbosity == V2 ||
                check_prereq.instp_verbosity == V3)
            {
               fix_ptr->rept_list_position = count;
               /*
                * Do a little bit of manual formatting -- could have done it
                * automatically in sprintf, but we would have had to include 
                * more cases for V1 vs V2 & V3.
                */
               count_str[0]='\0';
               if (count < 10)
                  strcat (count_str, "  #"); 
               else if (count < 100)
                  strcat (count_str, " #"); 
               else 
                  strcat (count_str, "#");
 
               sprintf (tmp_str, "%s%d", count_str, count++);
               strcpy (count_str, tmp_str); 
               strcat (count_str, " ");
            }
            else
               strcpy (count_str, "    ");

            if (fix_ptr->flags & AUTO_SUPERSEDES_PKG)
               sprintf (name_lev, "%s%s %s *", count_str, fix_ptr -> name,
                        get_level_from_fix_info (fix_ptr, fix_name));
            else
               sprintf (name_lev, "%s%s %s", count_str, fix_ptr -> name,
                        get_level_from_fix_info (fix_ptr, fix_name));

            format_pkg_name_and_desc (IF_INSTALL (fix_ptr -> op_type),
                                      print_string, 
                                      name_lev, 
                                      fix_ptr -> description,
                                      error);
            RETURN_ON_ERROR;
            inu_msg (ckp_errs, "%s\n", print_string);
            if (fix_ptr->flags & AUTO_SUPERSEDES_PKG)
               print_superseded_pkgs_of_failed_auto_supersedes (fix_ptr);

            if (check_prereq.instp_verbosity == V4)
            {
               print_failed_subgraph (fix_ptr, NIL (requisite_list_type), 
                                      INIT_INDENT_LEV, INIT_SPACES, FIRST_TIME);
               unmark_graph (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED);
               unmark_requisite_visited ();

            } /* end if */
         } /* end if */
      } /* end for */
      inu_msg (ckp_errs, "\n");
      if (check_prereq.instp_verbosity == V4)
      {
         /*
          * Print the requisite code key for the output above.
          */
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_CODES_I, CKP_REQ_CODES_D));
         if (check_prereq.mode == OP_APPLY)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_CODES_APPLY_ONLY_I, 
                     CKP_REQ_CODES_APPLY_ONLY_D));
         inu_msg (ckp_errs, "\n");

         return;
      }
   }
   else
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_FAIL_APPLY_TITLE_I, 
               CKP_REQ_FAIL_APPLY_TITLE_D));
   } /* end if */

   /*
    * Now start printing the requisites of the failed selected packages.
    * We'll repeatedly scan the fix_info table printing the following four
    * categories of requisites:
    *   
    * TYPE_A_FAILURE:
    *   APPLY and COMMIT: missing requisites.
    *   REJECT: committed dependents. 
    *   DEINSTALL: non-deinstallable dependents.
    *
    * TYPE_B_FAILURE:
    *   APPLY ONLY: Group requisite failures. 
    *
    * TYPE_C_FAILURE:
    *   APPLY: Other available requisites that would have been applied (maybe
    *          -g wasn't used or other requisites failed causing whole chain
    *          to fail).
    *   COMMIT: Other applied requisites that would have been committed 
    *   REJECT: Other installed packages that would have been rejected
    *   DE-INSTALL: Other installed products would have been de-installed
    *
    * TYPE_D_FAILURE:
    *   APPLY and COMMIT: BROKEN requisites.
    * 
    * TYPE_E_FAILURE:
    *   APPLY: Conflicting versions of base level requisites (trying to pull in
    *          a base level req when pkg is installed or being installed at
    *          another level.)
    * 
    * TYPE_Z_FAILURE: 
    *   ALL OPS: Those failures which don't fall into the above categories.
    */

   print_failed_requisite_list (TYPE_A, error);
   RETURN_ON_ERROR;

   print_failed_requisite_list (TYPE_D, error);
   RETURN_ON_ERROR;

   if (check_prereq.mode == OP_APPLY)
   {
      print_failed_requisite_list (TYPE_E, error);
      RETURN_ON_ERROR;

      /*
       * Group requisite failures.  Reset the legend array that may have been
       * used when printing command line failures earlier.
       */
      reset_report_failure_legend ();
      printed_group_failures = print_failed_requisite_list (TYPE_B, error);
      RETURN_ON_ERROR;
   }

   /*
    * We want to reset the failure key/legend array if we didn't print 
    * group failures.  This is so that we preserve any entries added for
    * group nodes (since we're only printing one legend for group and "Misc"
    * requisite failures) and so that we erase any entries that may have been
    * added from command line failures that might have been printed.
    */
   if (! printed_group_failures)  
      reset_report_failure_legend ();

   printed_misc_failures = print_failed_requisite_list (TYPE_Z, error);
   RETURN_ON_ERROR;

   if (printed_misc_failures || printed_group_failures)
   {
      print_report_failure_legend (TRUE);
   }

   print_failed_requisite_list (TYPE_C, error);
   RETURN_ON_ERROR;

} /* end print_requisite_failures */

/*--------------------------------------------------------------------------*
**
** NAME: print_superseded_pkgs_of_failed_auto_supersedes
**
** FUNCTION:  Prints an additional message for "auto-supersedes" packages
**            which failed requisite checks.  These are superseding packages 
**            which were chosen automatically by ckprereq instead of a package
**            which was selected by the user.  The message is printed in the
**            form (supersedes: <PTF1>, <PTF2>, ..., <PTFN>)
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static boolean 
print_superseded_pkgs_of_failed_auto_supersedes 
(
fix_info_type * failed_fix
)
{
   boolean           print_hdr_msg;
   char              sup_id [MAX_LPP_FULLNAME_LEN];
   char              supd_id [MAX_LPP_FULLNAME_LEN];
   index_list_type * index_node;
   Option_t        * sop_ptr;

   /*
    * get the supersedes id from failed fix (different for 3.2 and 4.1 pkgs).
    */
   if (IF_3_2 (failed_fix->op_type))
      strcpy (sup_id, failed_fix->level.ptf);
   else
       get_level_from_fix_info (failed_fix, sup_id);

   /*
    * Print pkgs which we threw out that are being superseded.
    */
   print_hdr_msg = TRUE;
   for (index_node = report_anchor.superseded_index_hdr -> next_index_node;
        index_node != report_anchor.superseded_index_tail;
        index_node = index_node -> next_index_node)
   {
      sop_ptr = index_node->sop_ptr;

      /*
       * If the supersedes string on the failsop entry matches our supersedes
       * id, we need to print reference to this update.
       */
      if (strstr (sop_ptr->supersedes, sup_id) != NIL (char))
      {
         if (IF_3_2 (failed_fix->op_type))
            strcpy (supd_id, sop_ptr->level.ptf);
         else
            sprintf (supd_id, "%d.%d.%d.%d",
                     sop_ptr->level.ver, sop_ptr->level.rel,
                     sop_ptr->level.mod, sop_ptr->level.fix);

         if (print_hdr_msg)
         {

            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SUPERSEDES_I, 
                     CKP_SUPERSEDES_D), supd_id);
            print_hdr_msg = FALSE; 
         }
         else
         {
            inu_msg (ckp_errs, ", %s", supd_id);
         }
      } /* end if */
   } /* end for */
   if (! print_hdr_msg)
      inu_msg (ckp_errs, ")\n");

} /* print_superseded_pkgs_of_failed_auto_supersedes */

/*--------------------------------------------------------------------------*
**
** NAME: print_failed_subgraph
**
** FUNCTION:  Called for V4 invocations of installp, this routine prints,
**            in indented form, the subgraph of a package which had 
**            requisite failures.  Also prints a code giving some indication
**            as to why the requisite failed and the type of requisite.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void 
print_failed_subgraph 
 (
fix_info_type       * fix_ptr, 
requisite_list_type * req_node,
short                 indent_lev,
short                 num_spaces,
boolean               const first_time
)
{
   requisite_list_type * requisite;

   if (first_time) 
   {
      if (check_prereq.mode == OP_REJECT)
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEPENDENTS_COLON_I,
                  CKP_DEPENDENTS_COLON_D));
      else
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQUISITES_COLON_I,
                  CKP_REQUISITES_COLON_D));
   }
   else
   { 
      print_subgraph_requisite (fix_ptr, req_node, indent_lev, num_spaces);
      if (check_for_cycle_or_ifreq_subgraph (fix_ptr, req_node, TRUE))
         return;
      req_node->flags.requisite_visited = TRUE;
   }

   /*
    * Set other cycle detection flags.
    */
   fix_ptr->flags |= VISITING_THIS_SUBGRAPH; 

   for (requisite = fix_ptr->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      print_failed_subgraph (requisite->fix_ptr, requisite, (indent_lev+1), 
                             (num_spaces + 2), ! FIRST_TIME);
   } /* end for */ 

   fix_ptr->flags &= ~VISITING_THIS_SUBGRAPH;
   fix_ptr->flags |= SUBGRAPH_VISITED;

} /* print_failed_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: print_dependents_list
**
** FUNCTION: Prints a numeric string representing the positional value, in a
**           previously printed list, of packages which depend upon a failed 
**           requisite.  The list of index_nodes, which point to the 
**           fix_info structures representing the various dependents, is 
**           headed by the starting_dependent pointer.
**
** RETURNS:  void function  
**           
**
**--------------------------------------------------------------------------*/

static void 
print_dependents_list 
 (
index_list_type * starting_dependent, 
boolean           is_missing_requisite,
boolean         * error
)
{
   
   boolean           first_entry;
   boolean           found;
   char              work_str [20];
   index_list_type * dep_node;
   short             column;
   short             MAX_COLUMN = 75;
   short             START_COLUMN = 10;
   short             position;
   short             prev_position;
   typedef           enum {start_state, process_range} states;
   states            state;

   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEP_ABBREV_I, CKP_DEP_ABBREV_D));
   column = 19;

   /* 
    * Loop through the dependents list printing the number, in range format,
    * of packages which depend upon the current failed requisite.  The 
    * number refers to a positional value in a list of printed packages
    * which failed requisite checks.  That number is set in evaluate_failures ().
    * 
    * The algorithm uses a finite state automaton to process the positional
    * values.  Embedded within the automaton is code to detect end of line.
    */
   first_entry = TRUE;
   prev_position = -1;
   state = start_state;
   
   dep_node = starting_dependent;
   while (dep_node != NIL (index_list_type))
   {
      position = dep_node->fix_ptr->rept_list_position;
      
      /*
       * Increment the dependent pointer here.  Recall, we may be processing
       * the dependents list of a dummy fix.  If we hit the "next set" of 
       * criteria, set our dep_node to NIL so that the loop exits.  See the
       * routine add_subgraph_root_to_dependents_list for more info.  Only
       * perform this additional work if we're dealing with a missing 
       * requisite.
       */
      dep_node = dep_node->next_index_node;
      if (is_missing_requisite)
         if (dep_node != NIL (index_list_type))
            if (dep_node->unique_criteria)
               dep_node = NIL (index_list_type);
   
      switch (state)
      {
        case start_state :
           if (position == (prev_position + 1))
           {
              state = process_range;
           }
           else
           {
              if (first_entry)
              {
                 first_entry = FALSE;
                 sprintf (work_str, "%d", position);
              }
              else
              {
                 sprintf (work_str, ", %d", position);
              }

              if ( (column + strlen (work_str)) > MAX_COLUMN)
              {
                 inu_msg (ckp_errs, ",\n");
                 sprintf (work_str, "         %d", position);
                 column = START_COLUMN + strlen (work_str);
              }
              else
              {
                 column += strlen (work_str);
              }
              inu_msg (ckp_errs, "%s", work_str);
           }
           break;

        case process_range :
           if (position != (prev_position + 1))
           {
              sprintf (work_str, "-%d, %d", prev_position, position);
              if ( (column + strlen (work_str)) > MAX_COLUMN)
              {
                 inu_msg (ckp_errs, "\n         ");
                 column = START_COLUMN + strlen (work_str);
              }
              else
              {
                 column += strlen (work_str);
              }
              inu_msg (ckp_errs, "%s", work_str);
              state = start_state;
           }
           break;
      } /* end switch */
      prev_position = position;
   } /* end for */

   /*
    * Let's finish off any unfinished ranges.
    */
   if (state == process_range)
   {
      sprintf (work_str, "-%d)", position);
   }
   else
   {
      sprintf (work_str, ")");
   }

   if ( (column + strlen (work_str)) > MAX_COLUMN)
   {
      inu_msg (ckp_errs, "\n         %s\n\n", work_str);
   }
   else
   {
      inu_msg (ckp_errs, "%s\n\n", work_str);
   }

} /* print_dependents_list */

/*--------------------------------------------------------------------------*
**
** NAME: print_failed_requisite_list
**
** FUNCTION:  scans the fix_info table looking for the "failure_type" passed,
**            printing members of the fix_info list which match that type
**            and have been flagged as a requisite failure.  
**
** RETURNS:   TRUE if members of the fix_info_table were printed, false
**            otherwise.
**
**--------------------------------------------------------------------------*/
static boolean 
print_failed_requisite_list
( 
short     failure_type,
boolean * error
)
{
   boolean         printed_hdr_msg = FALSE;
   boolean         print_legend_sym;
   short           type_C_count = 0;

   fix_info_type * fix_ptr;

   for (fix_ptr = check_prereq.fix_info_anchor->next_fix;
        fix_ptr != NIL (fix_info_type);
        fix_ptr = fix_ptr->next_fix)
   {
      if ( (fix_ptr->flags & REPT_FAILED_REQUISITE) 
                               &&
          ( ( (failure_type == TYPE_A) && TYPE_A_FAILURE (fix_ptr->failure_sym))
           ||
           ( (failure_type == TYPE_B) && TYPE_B_FAILURE (fix_ptr))
           ||
           ( (failure_type == TYPE_C) && (TYPE_C_FAILURE (fix_ptr->failure_sym)))
           ||
           ( (failure_type == TYPE_D) && (TYPE_D_FAILURE (fix_ptr->failure_sym)))
           ||
           ( (failure_type == TYPE_E) && (TYPE_E_FAILURE (fix_ptr->failure_sym)))
           ||
           (failure_type == TYPE_Z && ! (TYPE_A_FAILURE (fix_ptr->failure_sym)
                                                          ||
                                         TYPE_B_FAILURE (fix_ptr)
							  ||
					 TYPE_C_FAILURE (fix_ptr->failure_sym)
							  ||
					 TYPE_D_FAILURE (fix_ptr->failure_sym)
							  |
					 TYPE_E_FAILURE (fix_ptr->failure_sym)))))
      {
         /*
          * Msg format may be slightly different for TYPE_C.
          * We print a summary of the TYPE_C failures (a single header msg)
          * if ! doing deinstall, and doing -g, and
          * level of verbosity == V1.  (PURPOSE: Trying to hide details of
          * TYPE_C and only display the real cause of failure.)
          */
         if ( ( (failure_type == TYPE_C)           &&
              (! check_prereq.deinstall_submode) &&
              (check_prereq.Auto_Include)        &&
              (check_prereq.instp_verbosity == V1)))
         {
            type_C_count++;
            continue;
         }

         if (! printed_hdr_msg)
         {
            /*
             * first print the appropriate header msg.
             */
            switch (failure_type)
            {
               case TYPE_A:
                  print_TYPE_A_FAILURE_hdr ();
                  break;

               case TYPE_B:
                  print_TYPE_B_FAILURE_hdr ();
                  break;

               case TYPE_C:
                  print_TYPE_C_FAILURE_hdr ();
                  /*
                   * Print an extra msg to warn that new updates will be
                   * automatically installed when -g is used.
                   */
                  if ((check_prereq.mode == OP_APPLY) && 
                      (! check_prereq.Auto_Include)   &&
                      (flag_superseding_TYPE_C_apply_failures(fix_ptr)))
                  {
                    inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                             CKP_WILL_PULL_IN_NEWER_I,
                             CKP_WILL_PULL_IN_NEWER_D));
                    inu_msg (ckp_errs, "\n");
                  } 
                  break;

               case TYPE_D:
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                           CKP_BROKEN_REQUISITES_I,
                           CKP_BROKEN_REQUISITES_D));
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_RECOVER_BROKEN_I,
                          CKP_RECOVER_BROKEN_D));
                  print_dependent_ref_msg ();
                  break;

               case TYPE_E:
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_CONFL_BASE_LEV_I,
                          CKP_CONFL_BASE_LEV_D));
                  print_dependent_ref_msg ();
                  if (flag_rename_conflicts(fix_ptr))
                     inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                          CKP_CONFL_BASE_LEV_NOTE_I,
                          CKP_CONFL_BASE_LEV_NOTE_D));
                  break;

               case TYPE_Z:
                  print_TYPE_Z_FAILURE_hdr ();
                  break;

            } /* switch */
            printed_hdr_msg = TRUE;
         } /* if (print_hdr_msg*/

         if (failure_type == TYPE_B )
         {
            if (check_prereq.instp_verbosity > V1)
            {
               /*
                * We need to print the positional number of the package that 
                * has this group dependency.
                */
                inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NEEDS_I, CKP_NEEDS_D),
                        fix_ptr->rept_dependents->fix_ptr->rept_list_position);
             }
             print_group_requisites (fix_ptr, (short) 0, 
                                     NIL (requisite_list_type), error);
             RETURN_ON_ERROR;
             unmark_graph (SUBGRAPH_VISITED);
             inu_msg (ckp_errs, "\n");
         }
         else
         {
            if (failure_type == TYPE_Z)
            {
               /*
                * Set the failure symbol in the global legend array.
                */
               add_sym_to_rf_legend (fix_ptr->failure_sym);
               print_legend_sym = TRUE;
            }
            else
               print_legend_sym = FALSE;

            print_requisite_and_dependent_list (fix_ptr, print_legend_sym, 
                                                NIL (char), error);
            RETURN_ON_ERROR;
            if (failure_type == TYPE_E && check_prereq.mode == OP_APPLY)
            {
               if (fix_ptr->flags & RENAMED_CONFLICTING_BASE)
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                           CKP_CONFL_WITH_I, CKP_CONFL_WITH_D),
                           fix_ptr->supersedes_string); 
            }
         }

      } /* if (fix_ptr->flags*/
   } /* for */

   if (printed_hdr_msg)
   {
      if (check_prereq.instp_verbosity == V1) /* Newline is handled by */
         inu_msg (ckp_errs, "\n");            /* dep list for non V1 reports */
      return (TRUE);
   }
   else
   {
      if (type_C_count > 0)
         /*
          * Print the number of TYPE_C failures found above.
          */
         print_TYPE_C_FAILURE_msg (type_C_count);

      return (FALSE);
   }
} /* print_failed_requisite_list */

/*--------------------------------------------------------------------------*
**
** NAME: print_group_requisites
**
** FUNCTION:  Recursively traverses the subgraph of a group requisite node
**            printing failures in group requisite format.  Members of the
**            group are flagged with characters indicating reasons for failure
**            and a failure key is printed the set of group members.
**
** RETURNS:   void function
**
**--------------------------------------------------------------------------*/
void 
print_group_requisites 
(  
fix_info_type        * parent,
short                  indent_lev,
requisite_list_type  * req_node,
boolean              * error
)
{
    boolean               nested_group = FALSE;
    char                * indent_str; 
    criteria_type       * criteria;
    fix_info_type       * fix_ptr;
    requisite_list_type * requisite;
    short                 count;
    short                 satisfied = 0;
    short                 satisfiable = 0;
    short                 total = 0;

    if (req_node != NIL (requisite_list_type)) {  /* will be NIL 1st time */
       fix_ptr = req_node->fix_ptr;
       criteria = & req_node->criteria;
    } else {
       fix_ptr = parent;
    }

    if (fix_ptr->flags & (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED))
       return;

    /*
     * See if we can avoid printing nested groups (all members may 
     * already be met or there may be nothing original to print).
     */
    if (bypass_redundant_group_reporting (fix_ptr))
       return;

    fix_ptr->flags |= VISITING_THIS_SUBGRAPH;
    /*
     * We indent nested groups according to indent_lev.  We need
     * to create a string of blanks as appropriate.
     */
    indent_str = my_malloc ( ( (GROUP_INDENT_VAL * indent_lev) + 1), error);
    RETURN_ON_ERROR;

    memset (indent_str, BLANK_SYM, GROUP_INDENT_VAL * indent_lev);
    indent_str[GROUP_INDENT_VAL * indent_lev] = '\0';

    /* 
     * Now we need to insert vertical pipes in the indent string
     * to delineate levels of indentation.
     */
    count = GROUP_INDENT_VAL-1;
    while (count < (indent_lev * GROUP_INDENT_VAL))
    {
       indent_str[count] = '|';
       count += GROUP_INDENT_VAL;
    }

    if (fix_ptr -> origin == DUMMY_GROUP)
    {
       get_group_req_stats (fix_ptr, &satisfied, &satisfiable, &total,
                            &nested_group);

       /* 
        * Print out a message saying that we need at least x of the
        * following products. 
        */
       inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_AT_LEAST_FOLLOWNG_I,
                CKP_AT_LEAST_FOLLOWNG_D), indent_str,
                fix_ptr->passes_required - satisfied);

       for (requisite = fix_ptr -> requisites;
            requisite != NIL (requisite_list_type);
            requisite = requisite -> next_requisite)
       {
          print_group_requisites (fix_ptr, (indent_lev + 1), requisite, error);
          RETURN_ON_ERROR;
       } /* end for */
    }
    else
    {
       if ( (fix_ptr->apply_state != ALL_PARTS_APPLIED) &&
           ! (fix_ptr->flags & SUCCESSFUL_NODE))
          print_failed_requisite (fix_ptr, criteria, parent, indent_lev,  
                                  req_node);
    }
    my_free (indent_str);

    fix_ptr->flags &= ~VISITING_THIS_SUBGRAPH;
    fix_ptr->flags |= SUBGRAPH_VISITED;

} /* end print_group_requisites */

/*--------------------------------------------------------------------------*
**
** NAME: print_requisite_and_dependent_list
**
** FUNCTION:  Prints the name and description of a failed requisite package.
**            For higher levels of verbosity (>V1) a list of numbers is 
**            printed (using ranges when possible) which indicates which 
**            packages requested from the command line depend (directly 
**            or indirectly) this failed requisite.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_requisite_and_dependent_list 
(
fix_info_type        * failed_requisite,
boolean                print_failure_sym,
char                 * specl_lppchk_output,
boolean              * error
)
{ 
   boolean           base_lev_pkg;
   char              name_lev  [MAX_LPP_FULLNAME_LEN];
   char              level_buf [MAX_LEVEL_LEN*5];
   char              name_desc [MAX_LPP_FULLNAME_LEN];
   index_list_type * dep_node;
 
   /* 
    * For a given failed requisite, we may have multiple criteria (requisite
    * strings) which need to be printed.  (This ONLY applies to missing
    * requisites -- dummy fixes.)   We need to set up a loop on the 
    * dependents list of the failed requisite, looking for unique criteria.
    * A "unique_criteria" flag in the requisite structure, set during
    * evaluate failures, will help us do so.
    */ 

   dep_node = failed_requisite->rept_dependents;
   while (dep_node != NIL (index_list_type))
   {
      if (specl_lppchk_output == NIL (char))
      {
         print_failed_pkg (failed_requisite, print_failure_sym, 
                           & (dep_node->criteria), error);
         RETURN_ON_ERROR; 
      }
      else
      {
         strcpy (name_desc, failed_requisite->name);
         strcat (name_desc, " ");
         get_criteria_string (&(dep_node->criteria), name_lev);
         strcat (name_desc, name_lev);
         inu_msg (ckp_errs,"\
  %-39s (%s)\n", name_desc, specl_lppchk_output);

      }

      /*
       * For V2 or V3, print a list of numbers which refers to
       * pkgs which depend upon this requisite.
       */ 
      if ( (check_prereq.instp_verbosity > V1)
                                 && 
           (! check_prereq.called_from_command_line) 
                                 &&
           (specl_lppchk_output == NIL (char)))
      {
         print_dependents_list (dep_node,
                                (failed_requisite->origin == DUMMY_FIX ||
                                 failed_requisite->apply_state == AVAILABLE),
                                error);
         RETURN_ON_ERROR;
      }
    
      if (failed_requisite->origin == DUMMY_FIX ||
          failed_requisite->apply_state == AVAILABLE)
      { 
         /*
          * Recall there may be multiple references (prereqs) to a dummy fix
          * meaning rept_dependents list may contain multiple entries.  There 
          * may be multiple "sections" where a section is a set of dependent 
          * nodes which contain the same criteria from one or more nodes.  The 
          * routine print_dependents_list prints dependents for a given set
          * of crtiteria.  Here, we need to either loop to the end of the 
          * dependents list OR to the next set of criteria.
          */
         dep_node = dep_node->next_index_node;
         while (dep_node != NIL (index_list_type) &&
                ! (dep_node->unique_criteria))
            dep_node = dep_node->next_index_node;
      }
      else
         /* 
          * Not dealing with dummy requisite.  Force loop to exit. 
          */
         dep_node = NIL (index_list_type);
   } /* end while */

} /* end print_requisite_and_dependent_list */
