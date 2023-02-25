static char sccsid[] = "@(#)65  1.64  src/bos/usr/sbin/install/ckprereq/report_utils.c, cmdinstl, bos41J, 9517A_all 4/20/95 19:13:45";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *            add_entry_to_index_list
 *            add_failures_to_fail_sop
 *            add_sym_to_rf_legend
 *            bypass_redundant_group_reporting
 *            check_for_cycle_or_ifreq_subgraph
 *            check_failures_on_fail_sop
 *            cleanup_report_failures
 *            create_index_node
 *            create_fix_info_index_from_subgraph
 *            create_sop_index
 *            determine_failure_reason
 *            flag_rename_conflicts
 *            flag_superseding_type_C_apply_failures
 *            format_pkg_name_and_desc
 *            free_fix_info_index_hdr
 *            free_index_nodes_between
 *            free_sop_index_hdrs
 *            get_criteria_string
 *            get_group_req_stats
 *            get_success_code
 *            ignore_ifreq_subgraph
 *            init_fix_info_index
 *            init_sop_index
 *            is_2nd_greater
 *            print_ckp_non_req_failures_hdr
 *            print_alrdy_instlds_or_comtds
 *            print_failed_pkg
 *            print_failed_requisite
 *            print_failures_hdr
 *            print_index_list_msg
 *            print_ifreq_warning_hdr1
 *            print_ifreq_warning_hdr2
 *            print_instp_failures_and_warnings
 *            print_instp_non_req_failures
 *            print_report_failure_legend
 *            print_requisite_failures_hdr
 *            print_subgraph_requisite
 *            print_success_hdr_by_requsite_type
 *            print_successful_requisite
 *            print_successful_sop_entry
 *            print_success_key
 *            print_success_section_hdr
 *            print_TYPE_A_FAILURE_hdr
 *            print_TYPE_B_FAILURE_hdr
 *            print_TYPE_C_FAILURE_hdr
 *            print_TYPE_C_FAILURE_msg
 *            print_TYPE_Z_FAILURE_hdr
 *            print_warnings_hdr
 *            refine_failsop_errors_for_reporting
 *            reset_report_failure_legend
 *            same_criteria
 *            same_relation
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

static char DESC_SEPARATOR [] = " # ";
static char DT_DT_DT [] = "...";
static char UPDT_PKG [] = "Fileset Update";
static char BASE_PKG [] = "Base Level Fileset";
static char LOW_STRING [] = "!";
static char HI_STRING [] = "~~~~~~~~~~~~~~~~~~~~~~~~~~";

#define LINE_LEN               80
#define MAX_DESC_LEN_DESC_1ST  60
#define MAX_PKG_LEN_NO_WRAP    45
#define DT_DT_DT_DESC_SEP_LEN   6
#define DESC_SEP_LEN            3
#define DESC_1ST_INDENT_STR     "    "  /* indent string printed before a
                                           successful updates description and 
                                           ptf id for V>V1 */
#define SUCCESS_INDENT_STR      "  "    /* indent string printed before all
					   other successful pkgs. */
#define FAILED_PKG_INDENT_STR   "    "  /* indent string printed before a
                                           failed pkg. */

static char rf_legend [MAX_LEGEND_SZ]; /* requisite failure legend array */
static short legend_top = -1;          /* points to last legend character */

static char * get_success_code (
char * out_buf, 
Option_t * sop_ptr);

static boolean same_relation (
relation_type * rel1,
relation_type * rel2);

void 
print_dependent_ref_msg ();

#define GROUP_INDENT_VAL  4

/*--------------------------------------------------------------------------*
**
** NAME: add_entry_to_index_list
**
** FUNCTION:  Adds a sop or fix_info entry (which ever is passed) to the 
**            index list headed by lst_hdr.  Entry is inserted in alphanumeric
**            order according to name, level, ptf id.  List is assumed to 
**            contain header and tail entries which will guarantee that there
**            is always a place to insert.
**
** RETURNS:  Void Function
**
**--------------------------------------------------------------------------*/

void 
add_entry_to_index_list
(
index_list_type  * lst_hdr,
fix_info_type    * fix_ptr,
Option_t         * sop_ptr,
criteria_type    * criteria,
boolean          * error
)
{
   boolean           found;
   boolean           seen_other_part;
   index_list_type * prev_node;
   index_list_type * next_node;
   index_list_type * insert_node;
   Option_t        * sop_ptr2;

   next_node = lst_hdr;
   prev_node = next_node;
   found = FALSE;

   /*
    * The goal is to search the list to find a place where a sop/fix_info
    * entry can be inserted.  The hdr and tail of list being search
    * was seeded with low and high (resp.) values.  This guarantees
    * that we don't run off the end of the list and that we always
    * find a place to insert.  Still,  we don't want to hang in case
    * our seed values are not low and big enough (highly unlikely),
    * so let's put the NIL check in just in case.
    */

   seen_other_part = FALSE;
   while ((! seen_other_part) &&(! found) && 
          (next_node != NIL (index_list_type)) )
   {
      if (sop_ptr != NIL (Option_t))
      {
         sop_ptr2 = next_node -> sop_ptr;
         if ((strcmp (sop_ptr -> name, sop_ptr2 -> name)== 0) &&
             (inu_level_compare (&(sop_ptr -> level),  
                                 &(sop_ptr2 -> level)) == 0))
         {
            seen_other_part = TRUE;
         }
         else
         {
            found = is_2nd_greater (sop_ptr -> name, sop_ptr2 -> name,
                                 &(sop_ptr -> level),  &(sop_ptr2 -> level),
                                 sop_ptr -> desc, sop_ptr2 -> desc);
         }
      }
      else
      {
         found = is_2nd_greater (fix_ptr -> name, 
                                 next_node -> fix_ptr -> name,
                                 &(fix_ptr -> level), 
                                 &(next_node -> fix_ptr -> level),
                                 fix_ptr -> description,
                                 next_node -> fix_ptr -> description);
      } /* end if */

      if (! found)
      {
         prev_node = next_node;
         next_node = next_node -> next_index_node;
      } /* end if */
   } /* end while */

   if (! seen_other_part)
   {
      insert_node = create_index_node (criteria, error);
      RETURN_ON_ERROR;
      insert_node -> sop_ptr = sop_ptr;
      insert_node -> fix_ptr = fix_ptr;
      insert_node -> next_index_node = next_node;
      prev_node -> next_index_node = insert_node;
   } /* end if */

} /* end add_entry_to_index_list */

/*--------------------------------------------------------------------------*
**
** NAME: add_failures_to_fail_sop
**
** FUNCTION:  Loops through the packages requested for installation/commit etc.
**            on the requisite list of the fix_info anchor.  Adds these entries
**            to the fail sop to be passed back to installp.
**
** RETURNS: Void function.
**
**--------------------------------------------------------------------------*/

void 
add_failures_to_fail_sop (void)
{
   fix_info_type       * fix_ptr;
   Option_t            * sop_ptr;
   requisite_list_type * requisite;

   /*
    * Add the command_line failure fixes, detected here in inu_ckreq.
    */
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      fix_ptr = requisite->fix_ptr;
      if (fix_ptr->flags & REPT_CMD_LINE_FAILURE)
      {
         if ((fix_ptr->apply_state == CANNOT_APPLY_CONFL_LEVEL)   &&
             (IF_UPDATE (fix_ptr->op_type)))
         {
            /*
             * Must be an update which was reported as a more meaningful
             * "Wrong level installed" failure, rather than a requisite
             * failure.  Mark it as a requisite failure in the
             * installp failure summary nonetheless.
             */
            sop_ptr = move_failed_sop_entry_to_fail_sop (fix_ptr,
                                                        STAT_REQUISITE_FAILURE);
            continue;
         }

         /*
          * Translate the failure_sym value in the fix_info structure to 
          * a STAT_ value to be used by installp.  (This switch will be 
          * expanded as the failures msgs are refined.)
          */
         switch (fix_ptr->failure_sym)  
         {
            case BROKEN_REQ_SYM:
               sop_ptr = move_failed_sop_entry_to_fail_sop (fix_ptr, 
                                                            STAT_BROKEN);
               break;

            case NO_PRQ_BASE_SYM:
               sop_ptr = move_failed_sop_entry_to_fail_sop (fix_ptr, 
                                                        STAT_REQUISITE_FAILURE);
               break;

            case RENAMED_SUPD_PKG_SYM:
               /*
                * This failure, though reported in report_failures, was actually
                * added to the failsop in evaluate_base_levels(), when the
                * failure was detected.  Don't add it again.
                */
               break;

            default:
               sop_ptr = move_failed_sop_entry_to_fail_sop (fix_ptr, 
                                                          STAT_PART_INCONSIST);
               break;
         }
         if (fix_ptr->flags & AUTO_SUPERSEDES_PKG)
            /*
             * Make it look like this failed auto supersedes package was 
             * explicitly requested so that installp can print out a sensible
             * looking STATISTICS table.
             */
            sop_ptr->flag = SET_SELECTED_BIT (sop_ptr->flag);
      } /* if */ 
   } /* end for */

   /*
    * Repeat the above for requisite failures.
    */
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
   {
      fix_ptr = requisite -> fix_ptr;
      if (fix_ptr->flags & REPT_REQUISITE_FAILURE)
      {
         sop_ptr = move_failed_sop_entry_to_fail_sop (fix_ptr, 
                                                      STAT_REQUISITE_FAILURE);
         /*
          * Make it look like this failed auto supersedes package was 
          * explicitly requested so that installp can print out a sensible
          * looking STATISTICS table.
          */
         if (fix_ptr->flags & AUTO_SUPERSEDES_PKG)
            sop_ptr->flag = SET_SELECTED_BIT (sop_ptr->flag);
      }
   } /* end for */

} /* end add_failures_to_fail_sop */

/*--------------------------------------------------------------------------*
**
** NAME: add_sym_to_rf_legend
**
** FUNCTION: Adds the failure_symbol passed to the "legend array" which will be 
**           printed as a key to show type of failure that occurred.
**
** RETURNS:  This is a void function.
**
**--------------------------------------------------------------------------*/

void 
add_sym_to_rf_legend 
 (
char failure_symbol
)
{
   short leg_index;
   /*
    * Do a quick scan just to make sure it's not already there.
    */
   for (leg_index = 0; leg_index <= legend_top; ++leg_index)
      if (rf_legend [leg_index] == failure_symbol)
         return;

   rf_legend[++legend_top] = failure_symbol;

} /* add_sym_to_rf_legend */

/*--------------------------------------------------------------------------*
**
** NAME: bypass_redundant_group_reporting
**
** FUNCTION: Scans the requisites of a group node in a requisite failure 
**           subgraph.  Determines if printing the requisite failure in 
**           group requisite format will lead to silly messages (such as
**           X requires "At least 0 of the following:" OR if group members
**           have already been accounted for (printed) in the current subgraph.
**
** RETURNS:  True if group failure reporting should be skipped for the group
**           node passed in to this routine.  False otherwise.
**  
**--------------------------------------------------------------------------*/

boolean 
bypass_redundant_group_reporting 
(
fix_info_type * node
)
{
  boolean               nested_group = FALSE;
  fix_info_type       * fix;
  int                   printable_count;
  int                   unsat_count;
  requisite_list_type * requisite;
  short                 satisfied = 0;
  short                 satisfiable = 0;
  short                 total = 0;

  if (node -> origin != DUMMY_GROUP)
     return (FALSE);

  get_group_req_stats (node, &satisfied, &satisfiable, &total, &nested_group);

  if ((node->passes_required - satisfied) <= 0)
     /*
      * There is nothing unsatisfied in this nested group -- it must not be the
      * reason for the subgraph's failure.
      */
     return (TRUE);

  /*
   * Scan the requisite chain looking for group members which have not already
   * been visited (hence accounted for) during traversal of this subgraph.
   */
  printable_count = 0;
  for (requisite = node -> requisites;
       requisite != NIL (requisite_list_type);
       requisite = requisite -> next_requisite)
  {
      fix = requisite -> fix_ptr;
      if ( ! ((fix -> flags & VISITING_THIS_SUBGRAPH)
                         ||
             (fix -> flags & SUBGRAPH_VISITED)))
      {
         switch (fix -> apply_state)
         {
            case ALL_PARTS_APPLIED :
               break;
            case TO_BE_EXPLICITLY_APPLIED :
               if (!(fix -> flags & SUCCESSFUL_NODE))
               {
                  printable_count++;
               }
               break;
            default :
               printable_count++;
               break;
         } /* end switch */
      }

  } /* end for */

  if (printable_count == 0)
  {
     return (TRUE);
  }
  else
  {
     return (FALSE);
  }

} /* bypass_redundant_group_reporting */

/*--------------------------------------------------------------------------*
**
** NAME: check_for_cycle_or_ifreq_subgraph
**
** FUNCTION:  Checks the fix_info structure passed in to see if it has been
**            "visited" before OR if this is the "head" of an ifreq subgraph 
**            which should be ignored.
**
** RETURNS:   TRUE if visiting this node OR if graph is an ifreq subgraph
**            FALSE otherwise
**
**--------------------------------------------------------------------------*/

boolean 
check_for_cycle_or_ifreq_subgraph 
(
fix_info_type       * current_fix,
requisite_list_type * req_node,
boolean               cld_by_rpt_routine
)
{
   if (current_fix -> flags &(VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED))
   {
      /*
       * We'll return unless this is a DUMMY_FIX.  If it is, check the
       * requisite_visited flag.  This is used to account for the fact that
       * we have at most ONE dummy node for each product option.  We may,
       * however, point to that node multiple times when several nodes reference
       * that node (implying several packages have missing requisites from the
       * same product).  We'll use the requisite_visited flag in the
       * requisite structure to keep track of visitation of a dummy_fix node.
       */
      if (current_fix->origin != DUMMY_FIX || req_node->flags.requisite_visited)
         return (TRUE);
   }

   /*
    * Check to see if we should ignore this subgraph.  This is based on
    * ifreqs that are in the graph which don't really need to be, and
    * in the case of reject, the selected_member_of_group flag (for processing
    * group requisites).
    */
   if (( (check_prereq.mode == OP_REJECT)                &&
        ((req_node -> flags.selected_member_of_group)    ||
                                 (ignore_ifreq_subgraph (TRUE, req_node, 
                                                          FALSE))))
                                ||
       ((check_prereq.mode != OP_REJECT) &&
        (ignore_ifreq_subgraph (FALSE, req_node, cld_by_rpt_routine))))
   {
      return (TRUE);
   }

   return (FALSE);

} /* check_for_cycle_or_ifreq_subgraph */

/*-------------------------------------------------------------------------*
**
** NAME: cleanup_report_failures
**
** FUNCTION:  Loops through the fix_info table deleting any "dependents
**            lists" (hanging off the rept_dependents pointers) that may
**            have been created while reporting requisite failures.
**
** RETURNS:   Void function.
**
**--------------------------------------------------------------------------*/

void 
cleanup_report_failures (void)
{
   index_list_type * del_dep_node;
   index_list_type * dep_node;
   fix_info_type   * fix_ptr;

   /*
    * Free the dependents lists we may have created.
    */
   for (fix_ptr = check_prereq.fix_info_anchor->next_fix;
        fix_ptr != NIL (fix_info_type);
        fix_ptr = fix_ptr->next_fix)
   {
      dep_node = fix_ptr->rept_dependents;
      while (dep_node != NIL (index_list_type))
      {
         del_dep_node = dep_node;
         dep_node = dep_node->next_index_node;

         /*
          * Zero out all the elements of criteria structure.  We were sharing
          * them with requisite nodes.  (Don't want to free them here.)
          */
         del_dep_node->fix_ptr = NIL (fix_info_type);
         del_dep_node->next_index_node = NIL (index_list_type);
         del_dep_node->criteria.version = NIL (relation_type);
         del_dep_node->criteria.release = NIL (relation_type);
         del_dep_node->criteria.modification = NIL (relation_type);
         del_dep_node->criteria.fix = NIL (relation_type);
         del_dep_node->criteria.ptf = NIL (relation_type);

         my_free (del_dep_node);  /* Zap! */

      } /* end while */
      fix_ptr -> rept_dependents = NIL (index_list_type);
   } /* end for */
 
} /* cleanup_report_failures */

/*--------------------------------------------------------------------------*
**
** NAME: create_index_node
**
** FUNCTION: Mallocs and initializes an index node.
**
** RETURNS:  A pointer to a index list node.
**
**--------------------------------------------------------------------------*/

index_list_type * 
create_index_node 
 (
criteria_type        * criteria,
boolean              * error
)
{
  index_list_type * node;

  node = (index_list_type *) my_malloc (sizeof (index_list_type), error);
  if (* error)
     return (NIL (index_list_type) );

  node->sop_ptr = NIL (Option_t);
  node->fix_ptr = NIL (fix_info_type);
  node->next_index_node = NIL (index_list_type);
  node->criteria = *criteria;
  node->unique_criteria = FALSE;

  return (node);

} /* end create_index_node */

/*--------------------------------------------------------------------------*
**
** NAME: create_fix_info_index_from_subgraph
**
** FUNCTION:  Given a fix_info node (a subgraph root) this routine traverses
**            the fix's subgraph adding subgraph members to a sorted index
**            list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
create_fix_info_index_from_subgraph 
 (
index_list_type * index_hdr,
fix_info_type   * subgraph_root,
fix_info_type   * current_fix,
boolean         * error
)
{
   index_list_type     * insert_node;
   requisite_list_type * requisite;

   /*
    * Stop the recursion if we've already been here.
    */
   if (current_fix -> flags &(VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED))
   {
      return;
   }

   current_fix -> flags |= VISITING_THIS_SUBGRAPH;

   /* 
    * Don't add the "subgraph root" to the index list.
    */
   if (subgraph_root != current_fix)
   {
      /*
       * Don't add an update's base level to the requisite list.
       * Don't add Group nodes either.
       */
      if (( (! IF_UPDATE (subgraph_root -> op_type)) ||
            (! IF_INSTALL (current_fix -> op_type)))
                             &&
           (current_fix->origin != DUMMY_GROUP)) 
      {
         add_entry_to_index_list (index_hdr, current_fix, NIL (Option_t),
                                 &EMPTY_CRITERIA, error);
         RETURN_ON_ERROR;
      } /* end if */
   } /* end if */

   /*
    * Recurse on the children of this node.
    */
   for (requisite = current_fix -> requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
   {
      create_fix_info_index_from_subgraph (index_hdr, subgraph_root, 
                                          requisite -> fix_ptr, error);
      RETURN_ON_ERROR;
   }

   current_fix -> flags &= ~VISITING_THIS_SUBGRAPH;
   current_fix -> flags |= SUBGRAPH_VISITED;
      
} /* create_fix_info_index_from_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: create_sop_index
**
** FUNCTION:  Traverses the SOP list pointed to by source_sop_hdr and 
**            creates a sorted index list pointing to members of the sop.
**
** RETURNS:   Pointer to index list.
**
**--------------------------------------------------------------------------*/

void 
create_sop_index
 (
Option_t         * source_sop_hdr,
index_list_type ** index_hdr,
index_list_type ** index_tail,
boolean * error
)
{
  Option_t        * sop_ptr;

  init_sop_index (index_hdr, index_tail, error);
  RETURN_ON_ERROR;

  for (sop_ptr = source_sop_hdr -> next;
       sop_ptr != NIL (Option_t);
       sop_ptr = sop_ptr -> next)
  {
     add_entry_to_index_list ((* index_hdr), NIL (fix_info_type), sop_ptr,
                               &EMPTY_CRITERIA, error);
     RETURN_ON_ERROR;
  } /* end for */

} /* create_sop_index */

/*--------------------------------------------------------------------------*
**
** NAME: determine_failure_reason
**
** FUNCTION:  Establishes reason for a pkg's failure according to parts
**            in the package, flags used, parts applied/committed and
**            special flags set and stored along the way during requisite
**            evaluation.  
**           
** RETURNS:   A character which will be used to flag the reason for the
**            failure of a pkg.
**
**--------------------------------------------------------------------------*/

char 
determine_failure_reason 
(
fix_info_type        * node,
requisite_list_type  * req_node,
boolean                doing_cmd_line_failures
)
{
   boolean doing_both_parts;
   char    vpd_tree;

   doing_both_parts = ((check_prereq.parts_to_operate_on & LPP_USER) &&
                       (check_prereq.parts_to_operate_on & LPP_ROOT));

   switch (check_prereq.mode)
   {
      case OP_APPLY :
       /*****************************************************************
        ************************* APPLY SECTION ************************
        *****************************************************************/
         if (node->flags & DISABLING_INT_ERR_FLAG)
         {
            return (DISABLING_INT_ERR_SYM);
         }

         if (node->cp_flag & LPP_MIGRATING)
         {
            return (MIGRATING_SYM);
         }

         if (node->apply_state == SUPD_BY_NEWER_BASE)
         {
            return (RENAMED_SUPD_PKG_SYM);
         }

         if (node -> flags & WARN_NO_PRQ_BASE)
         {
            return (NO_PRQ_BASE_SYM);
         }

         if (node -> apply_state == BROKEN)
         {
            return (BROKEN_REQ_SYM);
         }
  
         if (node -> flags & SUPERSEDES_BROKEN_PTF)
         {
            return (BROKEN_SUPS_SYM);
         }
 
         if (node -> flags & UPDATES_ONLY)
         {
            return (UPDATES_ONLY_SYM);
         }

         if (node -> flags & WARN_MISS_USR_APPLD_ROOT)
         {
            return (APPLIED_ROOT_ONLY_SYM);
         }

         if (node -> flags & WARN_MISS_USR_COMTD_ROOT)
         {
            return (COMMITTED_ROOT_ONLY_SYM);
         }

         if ((node-> apply_state == CANNOT_APPLY_CONFL_LEVEL) ||
             (node-> apply_state == TO_BE_OVERWRITTEN_BY_CONFL_LEVEL))
         {
            return (CONFL_LEVEL_SYM);
         } /* end if */

         if (node-> flags & FAILED_OEM_SPECIFIC)
         {
            return (OEM_SPECIFIC_SYM);
         }

         if (node-> flags & FAILED_OEM_MISMATCH )
         {
            return (OEM_MISMATCH_SYM);
         }

         if (node-> flags & FAILED_OEM_BASELEVEL )
         {
            return (OEM_BASELEVEL_SYM);
         } /* end if */

         /* If we still haven't found the reason for failure, let's see if the
            product is not available on the media.  This will be when the fix
            has a cp_flag of 0 OR a state of AVAILABLE when it's from the VPD.
            This latter case can occur when the product/update has been seen on
            "some" media, but NOT on the current media. */

         if ((node -> cp_flag == 0) 
                           ||
             ((node -> apply_state == AVAILABLE) && 
              (node -> origin == VPD)))
         {
            /* We need a different message if ckprereq is being called from
               the cmd_line since we have no notion of media. */

            if ( check_prereq.called_from_command_line
                                &&
                  ((node -> cp_flag == 0)  ||
                    (node -> apply_state == AVAILABLE)) )
            {
               return (NOT_APPLD_CMD_LINE_SYM);
            }
            else
            {
               /* Product is not available on the media */
               return (UNAVLBL_SYM);
            }
         } /* end if */

         if (req_node != NIL (requisite_list_type))
         {
            /*
             * Do we need to apply superseded root parts to be consistent 
             * with the applied, superseded USR part.  The flag bit to
             * indicate this is set in evaluate_supersedes. 
             */
            if (req_node->flags.superseded_consistency_apply)
               return (SUPRSD_CONSIST_SYM);
         }

         /* At this point we know that the product is available on the media and
            its not one of the special cases checked above.  We need to make 
            the following checks to see if parts were not requested but are 
            present.  */

          switch (node -> parts_in_fix)
          {
             case (LPP_USER | LPP_ROOT) :
               if (! doing_both_parts)
               {
                  return (NO_USR_ROOT_PART_RQSTD_SYM);
               }
               break;

             case LPP_USER :
               if (! (check_prereq.parts_to_operate_on & LPP_USER))
               {
                  return (NO_USR_PART_RQSTD_SYM);
               }
               break;

             case LPP_ROOT :
               /* This should never happen. */
               break;

             case LPP_SHARE :
               if (! (check_prereq.parts_to_operate_on & LPP_SHARE))
               {
                  return (NO_SHARE_PART_RQSTD_SYM);
               }
               break;

         } /* end switch */

         /* If we still haven't found the reason, let's see if we have a
            partially applied prereq. */

         if (node -> parts_in_fix == (LPP_USER | LPP_ROOT))
         {
            if (( (~node -> parts_applied & LPP_USER) &&
                 (node -> parts_applied & LPP_ROOT))
                                 ||
                ((node -> parts_applied & LPP_USER) &&
                 (~node -> parts_applied & LPP_ROOT)))
            {
               /* Product is partially applied */
               return (PARTLY_APPLD_SYM);
            }
         } /* end if */

         if (req_node != NIL (requisite_list_type))
         {
            /*
             * Check for instreq failures (can't use -g with this type of
             * requisite).  All the serious failures will have been detected
             * above.
             */
            if (req_node->type == AN_INSTREQ)
            {
               return (INSTREQ_SYM);
            }
         }
         /* If we STILL haven't found the reason, and the current node was not
            requested explicitly or via automatic inclusion, then we simply
            say that the current node needs to be applied and is available,
            OTHERWISE we give a generic messages saying that it's available, but
            it may have failed because products dependent upon it have failed,
            products upon which it depends have failed (they are typically
            members of groups which have requisites which cannot be satisfied)
            OR the product is a member of a group which has failed as a whole.*/
          if ((! check_prereq.Auto_Include)
                         &&
              (node -> origin != COMMAND_LINE))
          {
             return (AVLBL_SYM);
          }
          else
          {
             /*
              * If determine_failure_reason is being called with a COMMAND_LINE
              * node (while establishing reasons for command line failures)
              * return "unknown" to bring to the attention of the user that
              * something's probably wrong with this pkg's vpd and/or toc
              * entry.  Otherwise, flag this failure as one which would have
              * been installed had other requisites not failed (or in the 
              * case of groups, had requisites of this requisite not failed).
              */
             if (node->origin == COMMAND_LINE && doing_cmd_line_failures)
             { 
                return (UNKNOWN_SYM);
             }
             else
             {
                return (UNAVLBL_REQ_OF_REQ_SYM);
             }
          }
          break;

       case OP_COMMIT :
       /*****************************************************************
        ************************* COMMIT SECTION ************************
        *****************************************************************/
          if (node -> flags & WARN_NO_PRQ_BASE)
          {
             return (NO_PRQ_BASE_SYM);
          }

          if (node -> apply_state == BROKEN)
          {
             return (BROKEN_REQ_SYM);
          }

          if (node -> cp_flag == 0) 
          {
             /* Package is not installed. */
             return (UNAVLBL_SYM);
          }

          /* If we still haven't found the reason for failure, see if it's one
             of the specialised cases of a superseded USR part applied, when we
             are trying to commit the superseding product.  We set a bit to
             flag this in create_superseded_by_links. */

          if ((req_node != NIL (requisite_list_type)) &&
              (req_node->flags.superseded_usr_commit))
          {
             return (SUPRSD_USR_COMMIT_SYM);
          }

          switch (node -> parts_in_fix)
          {
             case (LPP_USER | LPP_ROOT) :
               /* We know this node has both parts.  If both parts weren't
                  requested ...*/

               if (! doing_both_parts)
               {
                  /* ...then, if it was the root part we requested... */
                  if (check_prereq.parts_to_operate_on & LPP_ROOT)
                  {
                     /* then, if both parts aren't applied, tell the user so. */
                     if (node->parts_applied != (LPP_USER | LPP_ROOT))
                     {
                        return (COM_PARTLY_APPLD_SYM);
                     }
                     else
                     {
                        /* ... else tell the usr we can't commit the
                               root part before the usr part. */
                        return (CANT_COMT_ROOT_PART_SYM);
                     }
                  }
                  else
                  {
                     /* ... otherwise, we must have requested a share part
                        so let's just say that we didn't request both 
                        parts.*/
                     return (NO_USR_ROOT_PART_RQSTD_SYM);
                  }
               }
               break;

             case LPP_USER :
               if (! (check_prereq.parts_to_operate_on & LPP_USER))
               {
                  return (NO_USR_PART_RQSTD_SYM);
               }
               break;

             case LPP_ROOT :
               /* This should never happen. */
               break;

             case LPP_SHARE :
               if (! (check_prereq.parts_to_operate_on & LPP_SHARE))
               {
                  return (NO_SHARE_PART_RQSTD_SYM);
               }
               break;

          } /* end switch */

          /* If we still haven't found a reason, we check to see if the product
             is in a partially applied or partially committed state. */
          if (node -> parts_in_fix == (LPP_USER | LPP_ROOT))
          {
             if (( (~node -> parts_applied & LPP_USER) &&
                  (node -> parts_applied & LPP_ROOT))
                                  ||
                 ((node -> parts_applied & LPP_USER) &&
                  (~node -> parts_applied & LPP_ROOT)))
             {
                /* Product is partially applied. */
                return (COM_PARTLY_APPLD_SYM);
             }
             else
             {
                if (( (~node -> parts_committed & LPP_USER) &&
                     (node -> parts_committed & LPP_ROOT))
                                     ||
                    ((node -> parts_committed & LPP_USER) &&
                     (~node -> parts_committed & LPP_ROOT)))
                {
                   /* Don't print partially committed msg for root part
                      commit. */
                   if (node->parts_committed != LPP_USER ||
                       check_prereq.parts_to_operate_on != LPP_ROOT)
                   { 
                      /* Product is partially committed. */
                      return (COM_PARTLY_COMTD_SYM);
                   }
                }  /* end if */
             } /* end if */
          } /* end if */
 
          /* If we STILL haven't found a reason, then we check to see if the
             product has even been applied.  (It really should have been since
             it is a prereq of some other node that's been applied.) */
          if (node -> apply_state == ALL_PARTS_APPLIED)
          {
             /* If we STILL haven't found the reason, and the current node was 
                not requested explicitly or via automatic inclusion, then we 
                say that the current node needs to be committed and is applied,
                OTHERWISE we give a generic messages saying that it's applied,
                but it may have failed because products dependent upon it have
                failed, products upon which it depends have failed (they are
                typically members of groups which have requisites that cannot
                be satisfied) OR the product is a member of a group which has
                failed as a whole. */
             if ((! check_prereq.Auto_Include)
                             &&
                  (node -> origin != COMMAND_LINE))
             {
                return (AVLBL_SYM);
             }
             else
             {
                return (UNAVLBL_REQ_OF_REQ_SYM);
             }
          }
          else
          {
             /* It appears that the product has an available record, but is not
                applied.  */
             return (UNAVLBL_SYM);
          }
          break;

       case OP_REJECT :
       /*****************************************************************
        ************************* REJECT SECTION ************************
        *****************************************************************/

         /*
          * Look for broken dependents preventing a reject.
          */
         if (node -> apply_state == BROKEN &&
             ! check_prereq.deinstall_submode)
         {
            return (BROKEN_REQ_SYM);

         } /* end if */

         if (MIGRATING (node->cp_flag))
         {
            return (MIGRATING_SYM);
         }

         /*
          * Look for deinstallability failures. (set in 
          * add_auto_include_nodes_to_subgraph ()).  If flag is not set, do
          * another check here, since the check is only performed when -g
          * is switched on.
          */
         if (node -> flags & FAILED_DEINST_CHECK)
            return (DEINSTBLTY_FAIL_SYM);
         else
            if (check_prereq.deinstall_submode && IF_INSTALL (node->op_type))
            {
               if (node->parts_in_fix == LPP_SHARE)
                  vpd_tree = VPDTREE_SHARE;
               else
                  vpd_tree = VPDTREE_USR;

               if (inu_deinst_chk (node->product_name, node->name, vpd_tree, 
                                   FALSE) != 0)
                  return (DEINSTBLTY_FAIL_SYM);
            }

          /* Now we see if the package is in a partially or fully committed 
             state (not for de-install though). */

          if ((! check_prereq.deinstall_submode) &&
              (node->parts_committed != 0))
          {
             if (node->parts_in_fix != node->parts_committed) 
             {
                if ((node->parts_committed != LPP_USER) ||
                    (check_prereq.parts_to_operate_on != LPP_ROOT)) 
                {
                   return (PARTLY_COM_SYM);
                }
                /* ELSE -- fall through this if.  We'll find another
                   reason for failure. */
             }  
             else 
             {
                return (COM_PRQ_SYM);
             }
          }
    
           /* If the reason was not determined by an earlier check, we see if
              some parts are applied which have not been requested to be
              rejected. */
              
           switch (node -> parts_applied)
           {
              case LPP_ROOT :
                 if (! (check_prereq.parts_to_operate_on & LPP_ROOT))
                 {
                    return (APPLD_ROOT_SYM);
                 }
                 break;

              case LPP_USER :
                 if (! (check_prereq.parts_to_operate_on & LPP_USER))
                 {
                    return (APPLD_USR_SYM);
                 }
                 break;

              case (LPP_ROOT | LPP_USER) :
                 if (! (check_prereq.parts_to_operate_on & LPP_ROOT))
                 {
                    return (APPLD_USR_ROOT_SYM);
                 }
                 break;

              case LPP_SHARE :
                 if (! (check_prereq.parts_to_operate_on & LPP_SHARE))
                 {
                    return (APPLD_SHARE_SYM);
                 }
                 break;
           } /*end switch */

           /* If we still haven't found the reason & all parts have been 
              applied or the parts applied match the parts_to_operate_on, then 
              its simply a case of rejecting all products which prereq the 
              current product. */

           if ((node->apply_state == ALL_PARTS_APPLIED)
                                    ||
               ((node->parts_applied & check_prereq.parts_to_operate_on)
                                    &&
                          node->parts_committed == 0))
           {
              if (! check_prereq.Auto_Include)
              {
                 return (APPLD_PRQ_SYM);
              }
              else
              {
                 return (REJECTABLE_REQ_SYM);
              }
           } /* end if */

           /* This final message is a catch-all in case there is some other
              peculiar reason why the current product cannot be rejected. */

           return (UNKNOWN_SYM);
           break;

    } /* end switch */

} /* end of determine_failure_reason */

/*--------------------------------------------------------------------------*
**
** NAME: flag_rename_conflicts
**
** FUNCTION:  Scans the fix_info table, beginning with fileset pointed
**            to by fix_ptr, terminating at end of list.
**            Returns TRUE if RENAMED_CONFLICTING_BASE tag bit is set,
**            FALSE otherwise. 
**
** RETURNS:   TRUE or FALSE (see above)
**
**--------------------------------------------------------------------------*/

boolean 
flag_rename_conflicts
(
fix_info_type * fix_ptr
)
{
   fix_info_type * tmp;
 
   for (tmp = fix_ptr;
        tmp != NIL (fix_info_type);
        tmp = tmp->next_fix)
   {
      if (tmp->flags & RENAMED_CONFLICTING_BASE)
         return (TRUE);
   } /* for */
   return (FALSE);

} /* flag_rename_conflicts */

/*--------------------------------------------------------------------------*
**
** NAME: flag_superseding_TYPE_C_apply_failures
**
** FUNCTION:  Determines if there's a newer, superseding update for any of
**            the TYPE_C failures (ie, a requisite which failed to install
**            because no -g was provided) in the fix_info_table.  Purpose
**            is so that a message can be printed by caller.
**
** RETURNS:   TRUE if a superseding fix of a TYPE_C failure is on the toc.
**            FALSE otherwise.
**
**--------------------------------------------------------------------------*/
boolean 
flag_superseding_TYPE_C_apply_failures
(
fix_info_type * first_TYPE_C   /* first TYPE_C failure in fix_info table */
)
{
   fix_info_type       * fix;
   fix_info_type       * old;
   fix_info_type       * new;
   supersede_list_type * sup;

   /*
    * Separate paths for 3.2 updates (expl sups chains) and 
    * all other pkgs. (impl sups chains on fix_info table.
    */
   if ((IF_3_2 (fix->op_type) && IF_UPDATE(fix->op_type)))
   {
      for (sup = fix->superseded_by;
           sup != NIL (supersede_list_type);
           sup = sup->superseding_fix->superseded_by)
      {
         if (IS_AUTO_APPLIABLE (sup->superseding_fix))
            return (TRUE);

      } /* end for (sup... */
      return (FALSE);
   }

   /*
    * Begin searching at the first type C failure in fix_info table.
    * Check the supersedes of every TYPE_C in fix_info  table until
    * we find one for which there is a superseding fix which would
    * be installed when -g is used.
    */
   for (fix =  first_TYPE_C;
        fix != NIL (fix_info_type);  
        fix = fix->next_fix)
   {
      if (TYPE_C_FAILURE (fix->failure_sym)) /* Can be auto included with -g */
      {
         if (IF_UPDATE (fix->op_type))
         {
            /*
             * Look for implicit sups for 4.1 updates 
             */
            old = fix;
            new = fix->next_fix;
            while (IS_STILL_SUPS_CHAIN_LR_SCAN (old,new))
            {
               if (IS_AUTO_APPLIABLE (new))
                  return (TRUE);

               old = new;
               new = new->next_fix;
            }
         }

         /*
          * see if there's a base level that can implicitly supersede "fix"
          */
         new = fix->next_fix;
         while ((new != NIL (fix_info_type)) &&
                (strcmp (fix->name, new->name) == 0))
         {
            if (IF_INSTALL (new->op_type)                    &&
                IS_AUTO_APPLIABLE (new)                      &&
                is_compat_levels (new->name,
                                  new->supersedes_string, 
                                  &fix->level))
              return (TRUE);
            new = new->next_fix;
         }

      } /* end if (TYPE_C... */
   } /* end for */
   return (FALSE);

} /* flag_superseding_TYPE_C_apply_failures */
/*--------------------------------------------------------------------------*
**
** NAME: format_pkg_name_and_desc
**
** FUNCTION:  Creates a string containing one of two formats:  A package name
**            and level followed by description OR A description followed by
**            ptf id.  The description is truncated if necessary.  Indentation
**            is provided for the DESC_1ST case but must be handled by the
**            caller in all other cases.
**
** RETURNS:   A pointer to the formatted string.
**
**--------------------------------------------------------------------------*/

char * 
format_pkg_name_and_desc 
(
boolean    base_lev,
char     * out_string,
char     * pkg_string,   /* Incoming name & lev (may have additional info
                            leading or trailing.) */
char     * desc_string,
boolean  * error
)
{
   char  desc_work [MAX_LPP_FULLNAME_LEN];
   char  fmt_str [20];   /* used to create dynamic format string */
   short desc_len;
   short pkg_len;
   short desc_copy_sz;

   desc_len = strlen (desc_string);
   pkg_len = strlen (pkg_string);

   /*
    * Format of string returned will be in the form shown in the 
    * in the following example:
    * 
Column #:
.........10........20........30........40........50........60........70........8
    X11rte.obj 1.2.0.0                        # AIX Windows Run Time Environ...
   
    */

   if (desc_len == 0)
   {
      /**
       ** Let's give it "some" description to print.
       **/
      if (base_lev) 
      {
         desc_len = strlen (BASE_PKG);
         strcpy (desc_work, BASE_PKG);
      }
      else
      {
         desc_len = strlen (UPDT_PKG);
         strcpy (desc_work, UPDT_PKG);
      }
   }
   else
   {
      /*
       * Won't print a description for DUMMY_FIXes since we don't
       * know if they're base levels or updates.
       */
      if (strcmp (desc_string, "DUMMY_FIX") == 0)
      {
         desc_work [0] = '\0'; 
         desc_len = 0;
      }
      else
         strcpy (desc_work, desc_string);
   } /* end if */

   /*
    * If pkg_len is above a certain length, let's just let things wrap. 
    * The following tests for this and it also tests to see if we need
    * to truncate (based on pkg name and description).  If not, we just 
    * copy the full description over to our output string.
    */
   if ((pkg_len > MAX_PKG_LEN_NO_WRAP) || 
       ((desc_len + MAX_PKG_LEN_NO_WRAP + DESC_SEP_LEN + 1) <= LINE_LEN))
   {
      desc_copy_sz = desc_len;  
   }
   else
   {
      desc_copy_sz = LINE_LEN - (MAX_PKG_LEN_NO_WRAP+DT_DT_DT_DESC_SEP_LEN+1);
   }

   if (desc_len > 0)
   {
      sprintf (fmt_str, "%%-%ds%%s", MAX_PKG_LEN_NO_WRAP);
      sprintf (out_string, fmt_str, pkg_string, DESC_SEPARATOR);
      strncat (out_string, desc_work, desc_copy_sz);
      if (desc_copy_sz != desc_len)
      {
         strcat (out_string, DT_DT_DT);
      }
   }
   else
   {
      sprintf (fmt_str, "%%-%ds", MAX_PKG_LEN_NO_WRAP);
      sprintf (out_string, fmt_str, pkg_string);
   }

} /* format_pkg_name_and_desc */

/*--------------------------------------------------------------------------*
**
** NAME: free_fix_info_index_hdr
**
** FUNCTION:  Frees up space used by the dummy header and tail structure of
**            an index list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
free_fix_info_index_hdr 
 (
index_list_type * hdr,
index_list_type * tail
) 
{
   if (hdr -> fix_ptr -> name != NIL (char))
      my_free (hdr -> fix_ptr -> name);

   if (hdr -> fix_ptr -> description != NIL (char))
      my_free (hdr -> fix_ptr -> description);

   my_free (hdr -> fix_ptr);

   if (tail -> fix_ptr -> name != NIL (char))
      my_free (tail -> fix_ptr -> name);

   if (tail -> fix_ptr -> description != NIL (char))
      my_free (tail -> fix_ptr -> description);

   my_free (tail -> fix_ptr);
} /* free_fix_info_index_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: free_index_nodes_between
**
** FUNCTION:  Free's the index nodes BETWEEN but not including, hdr and tail.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
free_index_nodes_between 
 (
index_list_type * hdr,
index_list_type * tail
) 
{
   index_list_type * index_node;
   index_list_type * del_index_node;

   index_node = hdr -> next_index_node;
   while (index_node != tail)
   {
      del_index_node = index_node;
      index_node = index_node -> next_index_node;
      del_index_node -> fix_ptr = NIL (fix_info_type);
      del_index_node -> sop_ptr = NIL (Option_t);
      del_index_node -> next_index_node = NIL (index_list_type);
      my_free (del_index_node);

   } /* end while */
   hdr -> next_index_node = tail;

} /* end free_index_nodes_between */

/*--------------------------------------------------------------------------*
**
** NAME: check_failures_on_Fail_SOP
**
** FUNCTION:  Looks for failures and warnings on the failsop.
**
** RETURNS:   Void function
**
** SIDE EFFECTS:
**            Sets the failure and/or warning flag to TRUE if appropriate.
**
**--------------------------------------------------------------------------*/

void 
check_failures_on_Fail_SOP 
 (
boolean * failures,
boolean * warnings
)
{
   Option_t * sop_ptr;

   (*failures) = FALSE;
   (*warnings) = FALSE;

   if (check_prereq.Fail_SOP == NIL (Option_t))
      return;

   for (sop_ptr = check_prereq.Fail_SOP->next;
        sop_ptr != NIL (Option_t);
        sop_ptr = sop_ptr->next)
   {
       if ((sop_ptr->Status == STAT_TO_BE_SUPERSEDED)       ||
           (sop_ptr->Status == STAT_ALREADY_SUPERSEDED)     ||
           (sop_ptr->Status == STAT_ALREADY_INSTALLED)      ||
           (sop_ptr->Status == STAT_BASE_ALREADY_INSTALLED) ||
           (sop_ptr->Status == STAT_DUPE_VERSION)           ||
           (sop_ptr->Status == STAT_ALL_KW_FAILURE)         ||
           (sop_ptr->Status == STAT_WARN_DEINST_MIG)        ||
           (sop_ptr->Status == STAT_WARN_DEINST_3_2)        ||
           (sop_ptr->Status == STAT_WARN_DEINST_3_1)        ||
           (sop_ptr->Status == STAT_NUTTIN_TO_APPLY)        ||
           (sop_ptr->Status == STAT_NUTTIN_TO_COMMIT)       ||
           (sop_ptr->Status == STAT_NUTTIN_TO_REJECT)       ||
           (sop_ptr->Status == STAT_NUTTIN_TO_DEINSTL)      ||
           (sop_ptr->Status == STAT_NO_USR_MEANS_NO_ROOT)   ||
           (sop_ptr->Status == STAT_ALREADY_COMMITTED))
       {
          (*warnings) = TRUE;
          if (*failures) /* no point in going any further */
             return;
       }
       else
       {
          (*failures) = TRUE;
          if (*warnings) /* no point in going any further */
                return; 
       }
   } /* end for */

} /* is_failure_on_Fail_SOP */

/*--------------------------------------------------------------------------*
**
** NAME: free_sop_index_hdrs
**
** FUNCTION:  Frees the two index nodes passed as arguments.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
free_sop_index_hdrs 
 (
index_list_type * hdr,
index_list_type * tail
) 
{
   if (hdr -> sop_ptr -> desc != NIL (char))
      my_free (hdr -> sop_ptr -> desc);

   my_free (hdr -> sop_ptr);

   if (tail -> sop_ptr -> desc != NIL (char))
      my_free (tail -> sop_ptr -> desc);

  my_free (tail -> sop_ptr);

} /* free_sop_index_hdrs */

/*--------------------------------------------------------------------------*
**
** NAME: get_criteria_string
**
** FUNCTION:  Creates a string from the contents of the criteria structure
**            passed.  (The string represents a requisite expression.  This
**            routine re-constructs the string from the criteria structure.)
**
** RETURNS:   Void function.
**
** SIDE EFFECT: level_buf is populated with a string representing the contents
**              of the criteria structure.
**
**--------------------------------------------------------------------------*/

void 
get_criteria_string 
 (
criteria_type * criteria, 
char          * level_buf
)
{
  char            tmp_buf[MAX_LPP_FULLNAME_LEN];
  relation_type * rel;
  boolean         first_time; 

  level_buf[0] = '\0';
  if (criteria == NIL (criteria_type))
  {
     return;
  }

  if (!(criteria->flags & OLD_STYLE_REQ_EXPR))
  {
     sprintf (level_buf, "%d.%d.%d.%d", 
              criteria->version->int_value, 
              criteria->release->int_value, 
              criteria->modification->int_value, 
              criteria->fix->int_value);
     return;
  }
 
  first_time = TRUE; 
  for (rel = criteria -> version;
       rel != NIL (relation_type);
       rel = rel -> next_relation)
  {
     if (!first_time)
        strcat (level_buf, " OR ");  
     else 
        first_time = FALSE;

     switch (rel -> operator)
     {
        case LESS_THAN :
             sprintf (tmp_buf, "v<%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;

        case GREATER_THAN :
             sprintf (tmp_buf, "v>%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
 
        case EQUALS :
             sprintf (tmp_buf, "v=%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
     }
  }
  if ((!first_time) &&((criteria -> release != NIL (relation_type)) || 
                        (criteria -> modification != NIL (relation_type)) ||
                        (criteria -> fix != NIL (relation_type)) ||
                        (criteria -> ptf != NIL (relation_type))))
     strcat (level_buf, ", ");

  first_time = TRUE; 
  for (rel = criteria -> release;
       rel != NIL (relation_type);
       rel = rel -> next_relation)
  {
     if (!first_time)
        strcat (level_buf, " OR ");  
     else 
        first_time = FALSE;

     switch (rel -> operator)
     {
        case LESS_THAN :
             sprintf (tmp_buf, "r<%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;

        case GREATER_THAN :
             sprintf (tmp_buf, "r>%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
 
        case EQUALS :
             sprintf (tmp_buf, "r=%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
     }
  }
  if ((!first_time) &&((criteria -> modification != NIL (relation_type)) ||
                        (criteria -> fix != NIL (relation_type)) ||
                        (criteria -> ptf != NIL (relation_type))))
     strcat (level_buf, ", ");

  first_time = TRUE; 
  for (rel = criteria -> modification;
       rel != NIL (relation_type);
       rel = rel -> next_relation)
  {
     if (!first_time)
        strcat (level_buf, " OR ");  
     else 
        first_time = FALSE;

     switch (rel -> operator)
     {
        case LESS_THAN :
             sprintf (tmp_buf, "m<%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;

        case GREATER_THAN :
             sprintf (tmp_buf, "m>%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
 
        case EQUALS :
             sprintf (tmp_buf, "m=%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
     }
  }
  if ((!first_time) &&((criteria -> fix != NIL (relation_type)) ||
                        (criteria -> ptf != NIL (relation_type))))
     strcat (level_buf, ", ");

  first_time = TRUE; 
  for (rel = criteria -> fix;
       rel != NIL (relation_type);
       rel = rel -> next_relation)
  {
     if (!first_time)
        strcat (level_buf, " OR ");  
     else 
        first_time = FALSE;

     switch (rel -> operator)
     {
        case LESS_THAN :
             sprintf (tmp_buf, "f<%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;

        case GREATER_THAN :
             sprintf (tmp_buf, "f>%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
 
        case EQUALS :
             sprintf (tmp_buf, "f=%d", rel -> int_value);
             strcat (level_buf, tmp_buf); 
             break;
     }
  }
  if ((!first_time) &&(criteria -> ptf != NIL (relation_type)))
     strcat (level_buf, ", ");

  first_time = TRUE; 
  for (rel = criteria -> ptf;
       rel != NIL (relation_type);
       rel = rel -> next_relation)
  {
     if (!first_time)
        strcat (level_buf, " OR ");  
     else 
        first_time = FALSE;

     switch (rel -> operator)
     {
        case LESS_THAN :
             sprintf (tmp_buf, "p<%s", rel -> ptf_value);
             strcat (level_buf, tmp_buf); 
             break;

        case GREATER_THAN :
             sprintf (tmp_buf, "p>%s", rel -> ptf_value);
             strcat (level_buf, tmp_buf); 
             break;
 
        case EQUALS :
             sprintf (tmp_buf, "p=%s", rel -> ptf_value);
             strcat (level_buf, tmp_buf); 
             break;
     }
  }
} /* get_criteria_string */

/*--------------------------------------------------------------------------*
**
** NAME: get_success_code
**
** FUNCTION:  Figures out the "type of request" of the successful sop entry
**            passed.  That is, whether it was explicitly requested 
**            ("Requested"), implicitly requested ("Requsite") etc.
**
** RETURNS:   Pointer to a character string containing the "success code"
**
**--------------------------------------------------------------------------*/

static char * 
get_success_code 
 (
char * out_buf, 
Option_t * sop_ptr
)
{
  out_buf[0]= '\0';

  switch (check_prereq.mode)
  {
     case OP_APPLY :
        
        if (IS_EXPL_REQUESTED_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_RQSTD_CODE_I,
                                    CKP_SUCC_RQSTD_CODE_D));
        else if (IS_AUTO_MAINT_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_MAINT_CODE_I,
                                    CKP_SUCC_MAINT_CODE_D));
        else if (IS_AUTO_MANDATORY_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_MAND_CODE_I,
                                    CKP_SUCC_MAND_CODE_D));
        else if (IS_AUTO_IFREQ_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_P_REQ_CODE_I,
                                    CKP_SUCC_P_REQ_CODE_D));
        else if (IS_AUTO_SUPERSEDE_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_SUPS_CODE_I,
                                    CKP_SUCC_SUPS_CODE_D));
        else if (IS_IMPL_REQUESTED_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQ_CODE_I,
                                    CKP_SUCC_REQ_CODE_D));
        else
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_UNK_CODE_I,
                                    CKP_SUCC_UNK_CODE_D));
        break;
   
     case OP_COMMIT :
        if (IS_EXPL_REQUESTED_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_RQSTD_CODE_I,
                                    CKP_SUCC_RQSTD_CODE_D));
        else if (IS_AUTO_IFREQ_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_P_REQ_CODE_I,
                                    CKP_SUCC_P_REQ_CODE_D));
        else if (IS_IMPL_REQUESTED_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQ_CODE_I,
                                    CKP_SUCC_REQ_CODE_D));
        else
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_UNK_CODE_I,
                                    CKP_SUCC_UNK_CODE_D));
        break;

     case OP_REJECT :
        if (IS_EXPL_REQUESTED_PKG (sop_ptr))
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_RQSTD_CODE_I,
                                    CKP_SUCC_RQSTD_CODE_D));
        else 
           strcpy (out_buf, MSGSTR (MS_CKPREREQ, CKP_SUCC_DEP_CODE_I,
                                    CKP_SUCC_DEP_CODE_D));
        break;
  } /* switch */
  return (out_buf);

} /* get_success_code */

/*--------------------------------------------------------------------------*
**
** NAME: get_group_req_stats
**
** FUNCTION:  This function scans the requisite list of the
**            DUMMY_GROUP passed as an argument, counting satisfied and
**            satisfiable group requisite members.  Sets the values of
**            the appropriate variables passed and sets a flag to indicate
**            whether or not a nested group was detected.
**
** RETURNS:   void function.
**
**--------------------------------------------------------------------------*/

void
get_group_req_stats
(
fix_info_type * node,
short         * satisfied,
short         * satisfiable,
short         * total,
boolean       * nested_group
)
{
  requisite_list_type  * requisite;
  fix_info_type        * fix;             

  requisite = node -> requisites;
  while (requisite != NIL (requisite_list_type))
  {
     fix = requisite->fix_ptr;
     if (fix->origin == DUMMY_GROUP)
     {
        (* nested_group) = TRUE;
        get_group_req_stats (fix, satisfied, satisfiable, total, nested_group);
        if ((fix->passes_required - (* satisfied)) <= 0)
           /*
            * Nested groups count as 1.
            */
           (* satisfied)++;
     }
     else
     {
        if (fix->parts_applied != 0 || fix->apply_state == BROKEN)
           /*
            * This is satisfied in the sense that it was already chosen
            * as a desirable member of the group.  It's either installed,
            * partially installed or BROKEN (needing to be fixed in the
            * latter two cases).
            */
           (* satisfied)++;
        else
           if (fix->origin != DUMMY_FIX &&
               fix->apply_state != AVAILABLE)
              (* satisfiable)++;
     }
     (* total)++;
     requisite = requisite -> next_requisite;
  } /* end while */

} /* get_group_req_stats */

/*--------------------------------------------------------------------------*
**
** NAME: ignore_ifreq_subgraph
**
** FUNCTION:  Determines wether or not the subgraph, pointed to by the 
**            requisite node passed, should be ignored for purposes of
**            evaluating and reporting requisites.  (Since the entire
**            subgraph is constructed regardless of whether a base level
**            is applied or being applied.)
**
** RETURNS:   A boolean value.
**
**--------------------------------------------------------------------------*/

boolean 
ignore_ifreq_subgraph 
(
boolean               doing_successes,
requisite_list_type * requisite,
boolean               cld_by_rpt_routine
)
{
   /*
    * First, look for the ifreq link. (subgraph must not be ignored if 
    * the req is not an ifreq.)
    */
   if ((requisite -> type != AN_IFREQ) &&(requisite -> type != AN_IFFREQ))
       return (FALSE);

   /*
    * If we're not considering ifreqs, we don't care about this ifreq link.
    */
   if (! check_prereq.consider_ifreqs)
      return (TRUE);

   /*
    * Migrating filesets should never result in ifreq dependencies.
    */
   if (ck_base_lev_list (requisite, CBLL_MIGRATING, FALSE))
      return (TRUE);

   /*
    * For reject (and deinstall) we DONT care about the ifreq subgraph if the 
    * base product (of the pkg being ifreqed) is also being removed.  ie.
    * The ifreq will no longer hold if the base is being removed.
    */
   if (check_prereq.mode == OP_REJECT)
      return (ck_base_lev_list (requisite, CBLL_SUCC_CAND, FALSE));
   else
   {
       /* 
        * APPLY or COMMIT ops:
        * We don't care about the subgraph of a dummy fix (ie. ifreq is not 
        * on media and not installed) when the base level of the update
        * is not installed (indicated by all base_lev_list entries pointing 
        * to the dummy fix's fix_info node) OR if we're doing a commit op 
        * (since we never block commit ops across missing ifreqs and since
        * base levels will be committed anyway.)
        */
       if ((requisite->fix_ptr->origin == DUMMY_FIX) 
                             &&
           ((check_prereq.mode == OP_COMMIT) ||
            (ck_base_lev_list (requisite, CBLL_MISSING, FALSE))))
          return (TRUE);
       else
          /*
           * All that remains is to make the decision whether or not to ignore
           * the ifreq subgraph based upon whether or not the base pkg
           * has been applied or is being applied.
           */ 
          return (! ck_base_lev_list (requisite, CBLL_SUCC_CAND, 
                                      cld_by_rpt_routine));
   }
} /* ignore_ifreq_subgraph */                

/*--------------------------------------------------------------------------*
**
** NAME: init_fix_info_index
**
** FUNCTION:  Creates and initializes the dummy header and tail nodes of a 
**            fix_info_index list.
**
** RETURNS:   A pointer to the head of a two element dummy print_list.
**
**--------------------------------------------------------------------------*/

void 
init_fix_info_index
 (
index_list_type ** index_hdr,
index_list_type ** index_tail,
boolean * error
)
{
   (* index_hdr) = create_index_node (&EMPTY_CRITERIA, error);
   RETURN_ON_ERROR;
   (* index_hdr) -> fix_ptr = create_fix_info_node (error);
   RETURN_ON_ERROR;
   (* index_hdr) -> fix_ptr -> name = dupe_string (LOW_STRING, error);
   RETURN_ON_ERROR;
   (* index_hdr) -> fix_ptr -> description = dupe_string (LOW_STRING, error);
   RETURN_ON_ERROR;
   (* index_hdr) -> sop_ptr = NIL (Option_t);

   (* index_tail)  = create_index_node (&EMPTY_CRITERIA, error);
   RETURN_ON_ERROR;
   (* index_tail) -> fix_ptr = create_fix_info_node (error);
   RETURN_ON_ERROR;
   (* index_tail) -> fix_ptr -> name = dupe_string (HI_STRING, error);
   RETURN_ON_ERROR;
   (* index_tail) -> fix_ptr -> description = dupe_string (HI_STRING, error);
   RETURN_ON_ERROR;
  (* index_hdr) -> sop_ptr = NIL (Option_t);

   /**
    ** The following initializations are not really necessary, since all
    ** sorting should be achieved with name and description but are
    ** included for completeness (and perhaps debug purposes).
    **/
   (* index_hdr) -> fix_ptr -> level.ver = 0;
   (* index_hdr) -> fix_ptr -> level.rel = 0;
   (* index_hdr) -> fix_ptr -> level.mod = 0;
   (* index_hdr) -> fix_ptr -> level.fix = 0;
   strcpy ((* index_hdr) -> fix_ptr -> level.ptf, LOW_STRING);

   (* index_tail) -> fix_ptr -> level.ver = 99;
   (* index_tail) -> fix_ptr -> level.rel = 99;
   (* index_tail) -> fix_ptr -> level.mod = 9999;
   (* index_tail) -> fix_ptr -> level.fix = 9999;
   strncpy ((* index_tail) -> fix_ptr -> level.ptf, HI_STRING, 7);
   (* index_tail) -> fix_ptr -> level.ptf[7] = '\0';

   (* index_hdr) -> next_index_node = (* index_tail);
   (* index_tail) -> next_index_node = NIL (index_list_type);

} /* end init_fix_info_index */

/*--------------------------------------------------------------------------*
**
** NAME: init_sop_index
**
** FUNCTION:  Creates and initializes the dummy header and tail nodes of a 
**            sop index list.
**
** RETURNS:   A pointer to the head of a two element dummy print_list.
**
**--------------------------------------------------------------------------*/

void 
init_sop_index
 (
index_list_type ** index_hdr,
index_list_type ** index_tail,
boolean * error
)
{
   (* index_hdr) = create_index_node (&EMPTY_CRITERIA, error);
   RETURN_ON_ERROR;

   (* index_hdr) -> sop_ptr = create_option (
                              LOW_STRING,      /* Low ASCII value name */
                              NO,              /* Not important */
                              NO,              /* Not important */
                              CONTENT_UNKNOWN, /* Not important */
                              "",              /* Not important */
                              NIL (Level_t),   /* fill-in later */
                              LOW_STRING,      /* Low ASCII value description */
                              NIL (BffRef_t)   /* Not important */);
   (* index_hdr) -> fix_ptr = NIL (fix_info_type);

   (* index_tail)  = create_index_node (&EMPTY_CRITERIA, error);
   RETURN_ON_ERROR;
   (* index_tail) -> sop_ptr = create_option (
                              HI_STRING,       /* High ASCII value name */
                              NO,              /* Not important */
                              NO,              /* Not important */
                              CONTENT_UNKNOWN, /* Not important */
                              "",              /* Not important */
                              NIL (Level_t),   /* fill-in later */
                              HI_STRING,       /* Hi ASCII value description */
                              NIL (BffRef_t)   /* Not important */);

   (* index_hdr) -> fix_ptr = NIL (fix_info_type);

   /**
    ** The following initializations are not really necessary but are
    ** included for completeness (and perhaps debug purposes).
    **/
   (* index_hdr) -> sop_ptr -> level.ver = 0;
   (* index_hdr) -> sop_ptr -> level.rel = 0;
   (* index_hdr) -> sop_ptr -> level.mod = 0;
   (* index_hdr) -> sop_ptr -> level.fix = 0;
   strcpy ((* index_hdr) -> sop_ptr -> level.ptf, LOW_STRING);

   (* index_tail) -> sop_ptr -> level.ver = 99;
   (* index_tail) -> sop_ptr -> level.rel = 99;
   (* index_tail) -> sop_ptr -> level.mod = 9999;
   (* index_tail) -> sop_ptr -> level.fix = 9999;
   strncpy ((* index_tail) -> sop_ptr -> level.ptf, HI_STRING, 7);
   (* index_tail) -> sop_ptr -> level.ptf[7] = '\0';

   (* index_hdr) -> next_index_node = (* index_tail);
   (* index_tail) -> next_index_node = NIL (index_list_type);

} /* init_sop_index */

/*--------------------------------------------------------------------------*
**
** NAME: is_2nd_greater
**
** FUNCTION:  This function returns TRUE if the name and level passed of the
**            second set of arguments passed is alphanumerically greater than
**            the first.  
**
** RETURNS:  TRUE if 2nd is "greater" otherwise FALSE
**
**--------------------------------------------------------------------------*/

boolean 
is_2nd_greater
 (
char * name1,
char * name2,
Level_t * level1,
Level_t * level2,
char * desc1,
char * desc2
)
{
   boolean           is_gtr;
   int               rc;
   int               level_rc;
      
   level_rc = inu_level_compare (level1, level2);
   rc = strcmp (name1, name2);
   
   is_gtr = FALSE;
   /**
    ** First, check for exact match. 
    **/
   if ((rc != 0) || (level_rc != 0))
   {
      if (rc < 0)
      {
         /**
          ** 2nd name is greater than first.
          **/
         is_gtr = TRUE;
      }
      else
      {
         if (rc == 0)
         {
            /** 
             ** Hmmm.  Same name.  Let's check for same level.  Though
             ** there should never be different levels of the same
             ** product being installed. (except maybe for 3.1 pkgs and
             ** and their updates).
             **/
            if (level_rc == 0)
            { 
               /**
                ** Same levels.  Let's check descriptions.
                **/
               rc = strcmp (desc1, desc2);
               if (rc < 0)
               { 
                /**
                 ** Description of 2nd is greater than 1st (let's us see
                 ** things better by subsystem).
                 **/
                  is_gtr = TRUE;
               }
               else
               {
                  if (rc == 0)
                  { 
                     /** 
                      ** Aha. Descriptions match too.  Let's see if the 
                      ** ptf of the 2nd is greater.
                      **/
                     if (strcmp ((* level1).ptf, (* level2).ptf) < 0)
                     {
                        is_gtr = TRUE;
                     }
                     /* else ptf1 > ptf2  (we know they don't match as a
                        result of our very first test).*/
                  }
                  /* else rc > 0, first description is greater*/
               } /* end if */
            }
            else
            { 
               /** 
                ** We have same name but different levels.
                **/
               if (level_rc < 0) 
               {
                 /**
                  ** level of the second is greater.
                  **/
                 is_gtr = TRUE;
               }
               /* else first level is greater */
            } /* end if */    
         } /* end if */
         /* else rc > 0, first name is greater */ 
      } /* end if */
   } /* end if */

   return (is_gtr);

} /* is_2nd_greater */

/*--------------------------------------------------------------------------*
**
** NAME: print_ckp_non_req_failures_hdr
**
** FUNCTION:  Prints the Miscellaneous Failures header for non-requisite
**            failures detected by inu_ckreq ().  (ex.   Ask to install 
**            a pkg that's already installed in root part but not usr.) 
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_ckp_non_req_failures_hdr (boolean auto_sups_failure)
{
   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_FAILURES_TITLE_I, 
            CKP_MISC_FAILURES_TITLE_D));
  
   switch (check_prereq.mode)
   {
      case OP_APPLY :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_APPLY_FAILURES_HDR_I, 
                  CKP_MISC_APPLY_FAILURES_HDR_D));
         /*
          * Append additional msg warning about any auto supersedes that may
          * have occurred. 
          */ 
         if (auto_sups_failure)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_AUTO_SUPS_WARNING1_W, 
                     CKP_AUTO_SUPS_WARNING1_D));
         break;

      case OP_COMMIT :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_COMMIT_FAILURES_HDR_I, 
                  CKP_MISC_COMMIT_FAILURES_HDR_D));
         break;

      case OP_REJECT :
         if (check_prereq.deinstall_submode)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_DEINST_FAILURES_HDR_I, 
                     CKP_MISC_DEINST_FAILURES_HDR_D));
         else
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_REJECT_FAILURES_HDR_I, 
                     CKP_MISC_REJECT_FAILURES_HDR_D));
         break;

   } /* end switch */

} /* end print_ckp_non_req_failures_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: print_alrdy_instlds_or_comtds
**
** FUNCTION:  Scans the fail sop looking for entries that have been 
**            marked as already installed or already committed (depending
**            on the operation) and prints an appropriate msg. 
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_alrdy_instlds_or_comtds
(
)
{
   Option_t *top;
   int num_already_done=0;
 
   for (top=check_prereq.Fail_SOP->next;  top != NIL (Option_t); top=top->next)
   {
      if (top->Status == STAT_ALREADY_INSTALLED      ||
          top->Status == STAT_ALREADY_SUPERSEDED     ||
          top->Status == STAT_BASE_ALREADY_INSTALLED ||
          top->Status == STAT_ALREADY_COMMITTED)
      {
         num_already_done++;
      }
   } /* for */

   if (check_prereq.instp_preview && (check_prereq.mode == OP_APPLY))
   {
      /*
       * Preview op.  List the already installed filesets.
       */
      print_sop_list_msg (check_prereq.Fail_SOP, STAT_ALREADY_INSTALLED);
   }
   else
   {
      /*
       * Just print a single header msg.
       */

      if (num_already_done > 0) {
         if (check_prereq.mode == OP_APPLY)
         {
            inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_ALRDY_INSTLD_HDR_I, 
                     CKP_ALRDY_INSTLD_HDR_D), num_already_done);
            inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_ALRDY_INSTLD_NOTE_I, 
                     CKP_ALRDY_INSTLD_NOTE_D));
         }
         else
            inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_ALRDY_COMTD_HDR_I, 
                     CKP_ALRDY_COMTD_HDR_D), num_already_done);
      }
   }

} /* end print_alrdy_instlds_or_comtds */

/*--------------------------------------------------------------------------*
**
** NAME: print_dependent_ref_msg
**
** FUNCTION:  Prints the msg following a failed requisite header explaining 
**            the list "(dep: #" lists that may follow.  Otherwise prints
**            a newline.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_dependent_ref_msg ()
{
   if (check_prereq.instp_verbosity > V1)
   {
      if (check_prereq.mode != OP_REJECT)
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REF_TO_DEP_LIST1_I, 
                  CKP_REF_TO_DEP_LIST1_D)); 
      else
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REF_TO_DEP_LIST2_I, 
                  CKP_REF_TO_DEP_LIST2_D)); 
   }
   else
      inu_msg (ckp_errs, "\n");

} /* print_dependent_ref_msg */

/*--------------------------------------------------------------------------*
**
** NAME: print_failed_pkg
**
** FUNCTION:  Prints a string representing the name and level of the 
**            failed_pkg passed as an argument.  Prepends a "failure
**            symbol" if instructed to do so.  Interprets the call
**            as a request to print a string representing the contents
**            of the criteria structure (a requisite expression) if the
**            failed_pkg is a "DUMMY_FIX".
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_failed_pkg 
 (
fix_info_type      * failed_pkg,
boolean              print_failure_sym,
criteria_type      * criteria,
boolean            * error
)
{
   boolean           base_lev_pkg;
   char              name_lev  [MAX_LPP_FULLNAME_LEN];
   char              level_buf [MAX_LEVEL_LEN*5];
   char              name_desc [MAX_LPP_FULLNAME_LEN];

   /*
    * Format our failure symbol if necessary.
    */
   if (print_failure_sym)
      sprintf (name_lev, "%s%c ", FAILED_PKG_INDENT_STR, 
               failed_pkg->failure_sym);
   else
      sprintf (name_lev, "%s", FAILED_PKG_INDENT_STR);

   /*
    * Add the product name and a blank to the string.
    */
   strcat (name_lev, failed_pkg->name);
   strcat (name_lev, " ");

   /*
    * If we have a DUMMY_FIX, get a criteria string, else use the level from
    * the fix_info structure.  Add it to the string we're building.
    * Also, set a flag telling whether or not this is a base level pkg.
    */
   if (failed_pkg->origin == DUMMY_FIX)
   {
      get_criteria_string (criteria, level_buf);
      base_lev_pkg = ((criteria->ptf == NIL (relation_type)) 
                                             &&
                      ((criteria->fix == NIL (relation_type)) ||
                       (criteria->fix->int_value == 0)));
   }
   else
   {
      get_level_from_fix_info (failed_pkg, level_buf);
      base_lev_pkg = IF_INSTALL (failed_pkg->op_type);
   }
   strcat (name_lev, level_buf);

   /*
    * Format the name, level and description string.
    */
   format_pkg_name_and_desc (base_lev_pkg, 
                             name_desc, 
                             name_lev,
                             failed_pkg->description, 
                             error);
   RETURN_ON_ERROR;

   /*
    * Print our result.
    */
   inu_msg (ckp_errs, "%s\n", name_desc);

} /* print_failed_pkg */

/*--------------------------------------------------------------------------*
**
** NAME: print_failed_requisite
**
** FUNCTION:  Print a requisite's name and level info.
**            example formats printed:
**            #  bos.obj v=3, r=2 OR r>3, m=0000, f=0000
**            *  bos.obj v=3
**            !  bos.obj p=U123456
**            where the character in column 1 is a code explaining
**            why the requisite failed (table will be followed by a legend
**            explaining the codes).
**            When called for commit and reject operations, the format in the
**            following example will print (in almost all cases).
**
**            !  hcon.obj 3.2.0.0.U400007
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_failed_requisite 
(
fix_info_type       * node,
criteria_type       * criteria,
fix_info_type       * parent,
short                 indent_lev,
requisite_list_type * req_node
)
{
   char      legend_char;
   char      level_buf[MAX_LEVEL_LEN*5];
                       /* This length should handle most variations
                          of multiple versions, releases, etc. */
   char    * indent_str;
   boolean   error;
   short     count;
   boolean   print_criteria = FALSE;

   /*
    * Use the criteria (requisite expression) if this is a dummy fix.
    * Otherwise, we'll use the actual level.
    */
   if (node -> origin == DUMMY_FIX)
    {
      print_criteria = TRUE;
      level_buf[0] = '\0';
      get_criteria_string (criteria, level_buf);
   } /* end if */

   /*
    * Now determine the reason for the requisite's failure and add it to
    * the global legend array.  (SPECIAL PROCESSING for calls from
    * "ls_programs" (in particular lppchk).
    */
   if (! check_prereq.called_from_ls_programs)
   {
      legend_char = determine_failure_reason (node, req_node, FALSE);
      add_sym_to_rf_legend (legend_char);
   }
   else
      legend_char = ' ';

   if (parent -> origin == DUMMY_GROUP)
   {
      /* Members of groups require extra indentation.  We also handle the
         case of nested groups, inserting vertical bar symbols to allign
         members of groups. */

      indent_str = my_malloc ((GROUP_INDENT_VAL * indent_lev)+1, &error);
      memset (indent_str, BLANK_SYM, GROUP_INDENT_VAL * indent_lev);
      indent_str[GROUP_INDENT_VAL * indent_lev] = '\0';
      count = GROUP_INDENT_VAL-1;
      while (count < (indent_lev * GROUP_INDENT_VAL))
      {
        indent_str[count] = '|';
        count += GROUP_INDENT_VAL;
      }
      if (print_criteria)
       {
         /* Print the requisite with requisite criteria
                                (ie. v=2 or v>2, etc.)*/

         inu_msg (ckp_errs, "%s %c %s %s\n", indent_str, legend_char,
                  node->name,  level_buf);
       }
      else
       {
         /* Print the requisite with format
                              <name> <ver>.<rel>.<mod>.<fix>.<ptf>*/

         inu_msg (ckp_errs, "%s %c %s %s\n", indent_str, legend_char,
                  node -> name, get_level_from_fix_info (node, level_buf));
       }
      my_free (indent_str);
   }
   else
   {
      if (print_criteria)
         inu_msg (ckp_errs, " %c %s %s\n", legend_char, node->name, 
                  level_buf);
      else
         inu_msg (ckp_errs, " %c %s %s\n", legend_char, node -> name,
                       get_level_from_fix_info (node, level_buf));
   }

} /* end print_failed_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: print_failures_hdr
**
** FUNCTION:  Prints the main FAILURES header for the current operation. 
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_failures_hdr (void)
{
  
   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MAIN_FAILURES_TITLE_I, 
            CKP_MAIN_FAILURES_TITLE_D)); 

   switch (check_prereq.mode)
   {
      case OP_APPLY :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLY_FAILURES_HDR_I, 
                  CKP_APPLY_FAILURES_HDR_D)); 
         break;

      case OP_COMMIT :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_COMT_FAILURES_HDR_I, 
                  CKP_COMT_FAILURES_HDR_D)); 
         break;

      case OP_REJECT :
         if (check_prereq.deinstall_submode)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEINST_FAILURES_HDR_I, 
                     CKP_DEINST_FAILURES_HDR_D)); 
         else 
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REJ_FAILURES_HDR_I, 
                     CKP_REJ_FAILURES_HDR_D)); 
         break;

   } /* end switch */

} /* end print_failures_hdr */

/*--------------------------------------------------------------------------*
**
** NAME:     print_index_list_msg
**
** FUNCTION: Scans the index list passed looking for the warning type specified
**           printing a message corresponding to that warning type, followed
**           by a list of all members of the index list which match that type.
**
** RETURNS:  Void function.  
**
**--------------------------------------------------------------------------*/

void  
print_index_list_msg 
 (
index_list_type * index_hdr,
index_list_type * index_tail,
flags_type const  warning_type,
boolean         * error
)
{
   index_list_type * index;
   boolean           hdr_printed = FALSE;

   for (index = index_hdr->next_index_node;
        index != index_tail;
        index = index->next_index_node)
   {
      if (index->fix_ptr->flags & warning_type)
      {
         if (! hdr_printed)
         {
            switch (warning_type)
            {
               case WARN_NO_USR_SUPS_INFO:
                  inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NO_SUPS_IN_USR_W, 
                           CKP_NO_SUPS_IN_USR_D)); 
                  break;

               case WARN_NO_USR_PRQ_INFO:
                  inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NO_PRQ_IN_USR_W, 
                           CKP_NO_PRQ_IN_USR_D)); 
                  break;

               case WARN_NO_PRQ_BASE:
                  inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NO_PRQ_BASE_WARN_W, 
                           CKP_NO_PRQ_BASE_WARN_D)); 
                  break;

               case WARN_MISS_USR_APPLD_ROOT:
                  inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_MISS_USR_APPLD_ROOT_W, 
                           CKP_MISS_USR_APPLD_ROOT_D)); 
                  break;
 
               case WARN_MISS_USR_COMTD_ROOT:
                  inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_MISS_USR_COMTD_ROOT_W, 
                           CKP_MISS_USR_COMTD_ROOT_D)); 
                  break;
 
               default:
                  break;            
            } /* switch */
            hdr_printed = TRUE;
         } /* end if */
         print_failed_pkg (index->fix_ptr, FALSE, &EMPTY_CRITERIA, error);
         RETURN_ON_ERROR;
      } /* end if */
   } /* end for */

   if (hdr_printed)
      inu_msg (ckp_warn, "\n");

} /* end print_index_list_msg */

/*--------------------------------------------------------------------------*
**
** NAME: print_ifreq_warning_hdr1
**
** FUNCTION:  Prints the warning header message for uninstalled ifreqs of
**            pkgs that were previously installed.  Addresses packages 
**            that could be installed. 
**
** RETURNS:   A boolean value.
**
**--------------------------------------------------------------------------*/

void 
print_ifreq_warning_hdr1 (void)
{  
    inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_AVLBL_IFREQ_HDR_W, CKP_AVLBL_IFREQ_HDR_D)); 
    if (check_prereq.Auto_Include)
       /* 
        * If auto-include was on, then these pkgs must have requisite failures
        * of their own (otherwise they wouldn't be showing up in this failure/
        * warning list).  Append a message to indicate this. 
        */ 
       inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_AVLBL_IFREQ_FAIL_W, 
                CKP_AVLBL_IFREQ_FAIL_D)); 
    else
       /*
        * print msg explaining that they will be auto-installed with -g.
        */ 
       inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_AVLBL_IFREQ_USE_GFLAG_W, 
                CKP_AVLBL_IFREQ_USE_GFLAG_D)); 

} /* print_ifreq_warning_hdr1 */

/*--------------------------------------------------------------------------*
**
** NAME: print_ifreq_warning_hdr2
**
** FUNCTION:  Prints the warning header message for uninstalled ifreqs of
**            pkgs that were previously installed.  Addresses packages 
**            that are not available on current media.
**
** RETURNS:   A boolean value.
**
**--------------------------------------------------------------------------*/

void 
print_ifreq_warning_hdr2 (void)
{   
    inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_MSNG_IFREQ_TITLE_W, 
             CKP_MSNG_IFREQ_TITLE_D)); 
    /*
     * Print message specific to client operation if applicable.
     */
    if (check_prereq.parts_to_operate_on == LPP_ROOT)
       inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_MSNG_IFREQ_CLIENT_W, 
                CKP_MSNG_IFREQ_CLIENT_D)); 
    else
       inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_MSNG_IFREQ_W, CKP_MSNG_IFREQ_D)); 

} /* print_ifreq_warning_hdr2 */

/*--------------------------------------------------------------------------*
**
** NAME: print_instp_failures_and_warnings
**
** FUNCTION: Prints failures and warnings detected in installp (rather
**           than here in inu_ckreq ()) when there is nothing to process
**           in inu_ckreq ().  
**
** RETURNS:   A void function.
**
**--------------------------------------------------------------------------*/

void 
print_instp_failures_and_warnings 
 (
boolean * error
)
{   
   boolean failures;
   boolean warnings;

   inu_msg (ckp_prog, MSGSTR (MS_INUCOMMON, CMN_DONE_I, CMN_DONE_D)); /* Completion msg */
   inu_msg (ckp_prog, MSGSTR (MS_CKPREREQ, CKP_RESULTS_I, CKP_RESULTS_D));

   check_failures_on_Fail_SOP (&failures, &warnings);

   if (failures)
   {
      print_failures_hdr ();
      print_instp_non_req_failures (check_prereq.Fail_SOP);
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_E_O_FAILURES_I, CKP_E_O_FAILURES_D));
   }

   if (warnings)
   {
      report_warnings (FALSE, TRUE, error); 
      RETURN_ON_ERROR;
   }
} /* print_instp_failures_and_warnings */

/*--------------------------------------------------------------------------*
**
**  Function:       print_instp_non_req_failures
**
**  Description:    Prints the cmd line failures caught in installp.  This
**                  amounts to everything on the failure sop not caught by
**                  ckprereq.
**
**  Parameters:     sop  - ptr to sop linked list
**               failsop - ptr to failure sop linked list
**
**  Returns:        None
**
**--------------------------------------------------------------------------*/

void 
print_instp_non_req_failures 
 (
Option_t       * failsop
)
{
   print_sop_list_msg (failsop, STAT_BROKEN);
   switch (check_prereq.mode)
   {
      case OP_APPLY:
         print_sop_list_msg (failsop, STAT_NOT_FOUND_ON_MEDIA);
         print_sop_list_msg (failsop, STAT_OTHER_BROKENS);
         print_sop_list_msg (failsop, STAT_SUP_OF_BROKEN);
         print_sop_list_msg (failsop, STAT_CAN_BE_SUPERSEDED);
         print_sop_list_msg (failsop, STAT_ROOT_CAN_BE_SUPERSEDED);
         print_sop_list_msg (failsop, STAT_MUST_APPLY_ROOT_TOO);
         print_sop_list_msg (failsop, STAT_REQUISITE_FAILURE);
         print_sop_list_msg (failsop, STAT_NO_FORCE_APPLY_PTF);
         print_sop_list_msg (failsop, STAT_OEM_BASELEVEL);
         print_sop_list_msg (failsop, STAT_OEM_MISMATCH);
         print_sop_list_msg (failsop, STAT_OEM_REPLACED);
         break;

      case OP_COMMIT:
         break;

      case OP_REJECT:
         if (check_prereq.deinstall_submode) {
            print_sop_list_msg (failsop, STAT_FAILED_PRE_D);
            print_sop_list_msg (failsop, STAT_NO_DEINST_BOS);
         } else {
            print_sop_list_msg (failsop, STAT_COMMITTED_CANT_REJECT);
         }
         break;

      default:
         /* not applicable */
         break; 
   } /* switch */

} /* print_instp_non_req_failures */

/*--------------------------------------------------------------------------*
**
**  Function:       print_sop_list_msg
**
**  Description:    Prints a message followed by a list of packages.
**
**  Parameters:     failsop - failure sop list
**            target_status - Status field value to check for
**                  hdr_msg - message to be printed before list of pkgs.
**
**  Returns:        None
**
**--------------------------------------------------------------------------*/

void 
print_sop_list_msg
(
Option_t *failsop,
int       target_status 
)
{
  Option_t *op;
  char     namelev [MAX_LPP_NAME + 50];
  char     print_string [MAX_LPP_NAME * 2];
  int      hdr_printed=FALSE;
  boolean  error=FALSE;
  
  /*
   * Loop through the failsop looking for the desired failure status.
   */
  for (op  = failsop->next;
       op != NIL (Option_t);
       op  = op->next)
  {
     if (op->Status == target_status ||
         /*
          * "Already Installed" msg items can have other Status values.
          */
         (target_status == STAT_ALREADY_INSTALLED &&
          (op->Status == STAT_ALREADY_SUPERSEDED     ||
           op->Status == STAT_BASE_ALREADY_INSTALLED)))
     {
        /*
         * Print the header, according to the type of failure, if this is 
         * the first failure of this type that was found.
         */
        if (hdr_printed == FALSE)
        {
           switch (target_status)
           {
              case STAT_ALREADY_INSTALLED  :
              case STAT_ALREADY_SUPERSEDED :
              case STAT_BASE_ALREADY_INSTALLED :
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ,
                                            CKP_ALRDY_INSTLD_HDR_PREVIEW_I,
                                            CKP_ALRDY_INSTLD_HDR_PREVIEW_D));
                 break;

              case STAT_OTHER_BROKENS:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_OTHER_BROKENS_I, 
                          CKP_OTHER_BROKENS_D));
                 break;

              case STAT_SUP_OF_BROKEN:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                          CKP_SELECTED_SUPS_BROKEN_I,
                          CKP_SELECTED_SUPS_BROKEN_D));
                 break;

              case STAT_BROKEN:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_BROKEN_PKGS_I, 
                          CKP_BROKEN_PKGS_D));
                 break;

              case STAT_NOT_FOUND_ON_MEDIA:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISSING_PKGS_TITLE_I, 
                          CKP_MISSING_PKGS_TITLE_D));
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISSING_PKGS_HDR_I, 
                          CKP_MISSING_PKGS_HDR_D));
                 break;

              case STAT_DUPE_VERSION:
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_CONFL_VERSIONS_I, 
                          CKP_CONFL_VERSIONS_D));
                 break;

              case STAT_CAN_BE_SUPERSEDED:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MSNG_SUPERSEDED_TITLE_I, 
                          CKP_MSNG_SUPERSEDED_TITLE_D));
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MSNG_SUPERSEDED_HDR_I, 
                          CKP_MSNG_SUPERSEDED_HDR_D));
                 break;

              case STAT_ROOT_CAN_BE_SUPERSEDED:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                          CKP_MSNG_SUPERSEDED_ROOT_HDR_I, 
                          CKP_MSNG_SUPERSEDED_ROOT_HDR_D));
                 break;

              case STAT_NUTTIN_TO_APPLY:  /* This stat value should be set in
				             *very* rare cases, so we'll just
                                             use the message for STAT_NO_USR_
                                             MEANS_NO_ROOT which used to be
                                             set for many STAT_NUTTIN_TO_APPLY
                                             cases */
              case STAT_NO_USR_MEANS_NO_ROOT:
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_MISSING_PKGS_CLIENT_I, 
                          CKP_MISSING_PKGS_CLIENT_D));
                 break;

              case STAT_MUST_APPLY_ROOT_TOO:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_ROOT_PART_NOT_RQSTD_I, 
                          CKP_ROOT_PART_NOT_RQSTD_D));
                 break;

              case STAT_NUTTIN_TO_DEINSTL:
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NUTTIN_TO_DEINSTL_I, 
                          CKP_NUTTIN_TO_DEINSTL_D));
                 break;

              case STAT_NUTTIN_TO_COMMIT:
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NUTTIN_TO_COMMIT_I, 
                          CKP_NUTTIN_TO_COMMIT_D));
                 break;
          
              case STAT_COMMITTED_CANT_REJECT: 
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_COMTD_NOT_REJCTBL_I,
                          CKP_COMTD_NOT_REJCTBL_D));
                 break;

              case STAT_NUTTIN_TO_REJECT: 
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NUTTIN_TO_REJECT_I, 
                          CKP_NUTTIN_TO_REJECT_D));
                 break;

              case STAT_WARN_DEINST_3_1:
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_DEINST_3_1_I,
                         CKP_DEINST_3_1_D));
                 break;

              case STAT_WARN_DEINST_3_2:
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_DEINST_3_2_I,
                         CKP_DEINST_3_2_D));
                 break;

              case STAT_WARN_DEINST_MIG: 
                 inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_DEINST_MIG_I,
                          CKP_DEINST_MIG_D));
                 break;

              case STAT_FAILED_PRE_D:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_DEINST_TITLE_I, 
                          CKP_NO_DEINST_TITLE_D));
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NON_DEINST_PKG_I,
                         CKP_NON_DEINST_PKG_D));
                 break;

              case STAT_NO_DEINST_BOS:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_DEINST_TITLE_I, 
                         CKP_NO_DEINST_TITLE_D));
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_DEINST_BOSRTE_I,
                         CKP_NO_DEINST_BOSRTE_D)); 
                 return; /* Special case, no need to print any list */ 
                 break;

              case STAT_REQUISITE_FAILURE:
                 /* 
                  * This is a special requisite failure detected up-front by
                  * installp.  A 3.1 update was requested to be installed on
                  * top of a different vrmf.
                  */
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_31_UPDT_REQ_FAILURE_I,
                          CKP_31_UPDT_REQ_FAILURE_D));
                 break;

              case STAT_ALL_KW_FAILURE:
                 if (check_prereq.mode == OP_APPLY)
                    inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NOTHING_TO_APPLY_I,
                            CKP_NOTHING_TO_APPLY_D));
                 else  
                    inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NOTHING_TO_COMMIT_I,
                            CKP_NOTHING_TO_COMMIT_D));
                 return; /* Special case, no need to print any list */ 
                 break;

              case STAT_NO_FORCE_APPLY_PTF:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_CANT_FORCE_APPLY_PTF_I,
                          CKP_CANT_FORCE_APPLY_PTF_D));
                 break;

              case STAT_OEM_BASELEVEL:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_OEM_BASELEVEL_HDR_I,
                          CKP_OEM_BASELEVEL_HDR_D));
                 break;

              case STAT_OEM_MISMATCH:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_OEM_MISMATCH_HDR_I,
                          CKP_OEM_MISMATCH_HDR_D));
                 break;

              case STAT_OEM_REPLACED:
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_OEM_REPLACED_HDR_I,
                          CKP_OEM_REPLACED_HDR_D));
                 break;

              default:
                 /* Nothing to do */
                 break;
           } /* end switch */
           hdr_printed = TRUE;
        } /* end if */

        /*
         * Print the product and its description if available.
         */
        inu_get_optlevname_from_Option_t (op, print_string);
        strcpy (namelev, FAILED_PKG_INDENT_STR);
        strcat (namelev, print_string);
        if ((op->desc == NIL (char)) || (op->desc[0] == '\0'))
           format_pkg_name_and_desc (FALSE, 
                                     print_string, 
                                     namelev, 
                                     "DUMMY_FIX",
                                     &error);
        else
           format_pkg_name_and_desc (IF_INSTALL (op->op_type), 
                                     print_string, 
                                     namelev, 
                                     op->desc,
                                     &error);

        inu_msg (ckp_errs, "%s\n", print_string); 

        /*
         * Print additional information for supersedes failures.
         */
        if (target_status == STAT_CAN_BE_SUPERSEDED ||
            target_status == STAT_ROOT_CAN_BE_SUPERSEDED)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SUPERSEDED_BY1_I, 
                     CKP_SUPERSEDED_BY1_D), op->supersedes);

     } /* end if (op->Status... */
  } /* end for */

  /*
   * Print trailer messages (msgs that appear after the list of products)
   * for certain error types.
   */
  if (hdr_printed)
  {
     switch (target_status)
     {
        case STAT_ALREADY_INSTALLED:
           inu_msg (ckp_warn, "\n");
           inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_ALRDY_INSTLD_NOTE_I,
                   CKP_ALRDY_INSTLD_NOTE_D));
           break;

        case STAT_NUTTIN_TO_DEINSTL:
           inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NUTTIN_TO_DEINSTL_TRLR_I, 
                    CKP_NUTTIN_TO_DEINSTL_TRLR_D));
           break;

        case STAT_NUTTIN_TO_COMMIT:
           inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NUTTIN_TO_COMMIT_TRLR_I, 
                    CKP_NUTTIN_TO_COMMIT_TRLR_D));
           break;

        case STAT_COMMITTED_CANT_REJECT: 
           inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_COMTD_NOT_REJCTBL_TRLR_I,
                    CKP_COMTD_NOT_REJCTBL_TRLR_D));
           break;

        case STAT_BROKEN: 
        case STAT_OTHER_BROKENS: 
           inu_msg (ckp_errs, "\n");
           inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_RECOVER_BROKEN_I,
                   CKP_RECOVER_BROKEN_D));
           break;

        case STAT_NUTTIN_TO_REJECT: 
           inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_NUTTIN_TO_REJECT_TRLR_I, 
                    CKP_NUTTIN_TO_REJECT_TRLR_D));
           break;
     } /* switch */
     inu_msg (ckp_errs, "\n");
  }

} /* print_sop_list_msg */

/*--------------------------------------------------------------------------*
**
** NAME: print_report_failure_legend
**
** FUNCTION:  Print explanations of flagged products and requisites.
**            This routine uses the global array rf_legend to determine,
**            based on position in the array, which flags have been set
**            and which messages should be printed.
**            (THE FLAGS ARE SET IN determine_failure_reason WHERE CONDITIONS
**             FOR PRINTING THE VARIOUS MESSAGES ARE DOCUMENTED).
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_report_failure_legend 
(
boolean print_requisite_key /* determines type of header to print */
)
{
   short leg_index;

   if (print_requisite_key)
   {
      if (check_prereq.mode != OP_REJECT)
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_FAIL_KEY_HDR_I, 
                  CKP_REQ_FAIL_KEY_HDR_D));
      else
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEP_FAIL_KEY_HDR_I, 
                  CKP_DEP_FAIL_KEY_HDR_D));
   }
   else
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_FAIL_KEY_HDR_I, 
               CKP_FAIL_KEY_HDR_D));
   }

   /* We need to loop through the global array, printing messages when an
      element of that array has been set to TRUE.  The messages are printed
      according to the position of values in the array. */

   for (leg_index = 0; leg_index <= legend_top; ++leg_index)
   {
      switch (rf_legend [leg_index])
      {
         case UNAVLBL_SYM : /* or NOT_APPLD_CMD_LINE_SYM */ 
           
            if (check_prereq.called_from_command_line) 
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PKG_NOT_INSTLD_I,
                        CKP_PKG_NOT_INSTLD_D), NOT_APPLD_CMD_LINE_SYM);
            else
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQS_NOT_AVAILABL_I,
                          CKP_REQS_NOT_AVAILABL_D), UNAVLBL_SYM);


            break;

         case PARTLY_APPLD_SYM : /* or COM_PARTLY_APPLD_SYM or 
                                    APPLD_USR_ROOT_SYM */
            switch (check_prereq.mode)
            {
                case OP_APPLY :
                     inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQS_PARTLY_APPLD_I,
                           CKP_REQS_PARTLY_APPLD_D), PARTLY_APPLD_SYM);
                    inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_RECOVER_PARTLY_INSTLD_I,
                            CKP_RECOVER_PARTLY_INSTLD_D));
                   break;

                case OP_COMMIT :
                    inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_COMT_PARTLY_APPLD_I,
                            CKP_NO_COMT_PARTLY_APPLD_D), COM_PARTLY_APPLD_SYM);
                    inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_RECOVER_PARTLY_INSTLD_I,
                            CKP_RECOVER_PARTLY_INSTLD_D));
                   break;

                case OP_REJECT :
                     inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_REMOVE_USR_B4_ROOT_I,
                            CKP_NO_REMOVE_USR_B4_ROOT_D), APPLD_USR_ROOT_SYM);
                   break;

            } /* end switch */
            break;

         case AVLBL_SYM : /* or APPLD_PRQ_SYM */
            switch (check_prereq.mode)
            {
               case OP_APPLY :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQS_AVLBL_NO_G_I,
                           CKP_REQS_AVLBL_NO_G_D), AVLBL_SYM);
                  break;

               /*
                * We should never have to print this type of message for 
                * commit and reject.  Lets use the "unclear" msg instead.
                */
               case OP_COMMIT :
               case OP_REJECT :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_UNCLEAR_FAILURE_I,
                     CKP_UNCLEAR_FAILURE_D), UNKNOWN_SYM);
                  break;

            } /* end switch */
            break;

         case CANT_COMT_ROOT_PART_SYM : /* or  APPLD_ROOT_SYM */
            switch (check_prereq.mode)
            {
               case OP_APPLY :
                  break;

               case OP_COMMIT :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_COMT_ROOT_B4_USR_I,
                          CKP_NO_COMT_ROOT_B4_USR_D), CANT_COMT_ROOT_PART_SYM);
                  break;

               case OP_REJECT :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_REMOVE_USR_B4_ROOT_I,
                           CKP_NO_REMOVE_USR_B4_ROOT_D), APPLD_ROOT_SYM);
                  break;

             } /* end switch */
             break;

         case NO_USR_ROOT_PART_RQSTD_SYM :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_USR_ROOT_PARTS_NOT_RQSTD_I,
                     CKP_USR_ROOT_PARTS_NOT_RQSTD_D),
                     NO_USR_ROOT_PART_RQSTD_SYM);
            break;

         case NO_USR_PART_RQSTD_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_USR_PART_NOT_RQSTD_I,
                       CKP_USR_PART_NOT_RQSTD_D), NO_USR_PART_RQSTD_SYM);
             break;

         case NO_SHARE_PART_RQSTD_SYM : /* or  APPLD_SHARE_SYM */
             inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SHARE_PART_NOT_RQSTD_I,
                      CKP_SHARE_PART_NOT_RQSTD_D), NO_SHARE_PART_RQSTD_SYM);
            break;

         case COM_PARTLY_COMTD_SYM : /* or  PARTLY_COM_SYM  */
            switch (check_prereq.mode)
            {
               case OP_APPLY :
                    break;
               case OP_COMMIT :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQS_PARTLY_COMTD_I,
                          CKP_REQS_PARTLY_COMTD_D), COM_PARTLY_COMTD_SYM);
                    break;
               case OP_REJECT :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_REJ_PRTLY_COMMTD_I,
                           CKP_NO_REJ_PRTLY_COMMTD_D), PARTLY_COM_SYM);
                    break;
            } /* end switch */
            break;

         case UPDATES_ONLY_SYM : /* or AVLBL_NOT_APPLD_SYM or  COM_PRQ_SYM */
            switch (check_prereq.mode)
            {
               case OP_APPLY :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_UPDATES_ONLY_I,
                           CKP_UPDATES_ONLY_D), UPDATES_ONLY_SYM);
                  break;

               case OP_COMMIT :
                  break;

               case OP_REJECT :
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_REJ_COMMTD_PKG_I,
                           CKP_NO_REJ_COMMTD_PKG_D), COM_PRQ_SYM);
                  break;
            } /* end switch */
            break;

         case CONFL_LEVEL_SYM :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NON_UNIQUE_VRMF_PRQ_I,
                     CKP_NON_UNIQUE_VRMF_PRQ_D), CONFL_LEVEL_SYM);
            break;

         case UNAVLBL_REQ_OF_REQ_SYM :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_AVAIL_OTHER_PROBLEMS_I,
                     CKP_REQ_AVAIL_OTHER_PROBLEMS_D), UNAVLBL_REQ_OF_REQ_SYM);
            break;

         case SUPRSD_CONSIST_SYM : /* or SUPRSD_USR_COMMIT_SYM  */
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SUPD_MUST_APP_FOR_CONSIST_I,
                     CKP_SUPD_MUST_APP_FOR_CONSIST_D), SUPRSD_CONSIST_SYM);
            break;

         case APPLIED_ROOT_ONLY_SYM :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_REAPPLY_INSTLD_ROOT_I,
                     CKP_NO_REAPPLY_INSTLD_ROOT_D), APPLIED_ROOT_ONLY_SYM);
            break;

         case COMMITTED_ROOT_ONLY_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_COMMITTED_ROOT_ONLY_I,
                     CKP_COMMITTED_ROOT_ONLY_D), COMMITTED_ROOT_ONLY_SYM);
            break;

         case BROKEN_REQ_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_BROKEN_PKG_I,
                      CKP_BROKEN_PKG_D), BROKEN_REQ_SYM);
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_RECOVER_BROKEN_I,
                      CKP_RECOVER_BROKEN_D));
            break;

         case BROKEN_SUPS_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_BROKEN_SUPS_I,
                       CKP_BROKEN_SUPS_D), BROKEN_SUPS_SYM);
              break;
  
         case NO_PRQ_BASE_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_PRQ_BASE_I,
                       CKP_NO_PRQ_BASE_D), NO_PRQ_BASE_SYM);
              break;

         case INSTREQ_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INSTREQ_FAILURE_I,
                       CKP_INSTREQ_FAILURE_D), INSTREQ_SYM);
            break;

         case MIGRATING_SYM :
              if (check_prereq.mode == OP_REJECT)
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MIGRATING_DEP_I,
                          CKP_MIGRATING_DEP_D), MIGRATING_SYM);
              else
                 inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MIGRATING_REQ_I,
                          CKP_MIGRATING_REQ_D), MIGRATING_SYM);
            break;

         case RENAMED_SUPD_PKG_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_RENAMED_SUPD_PKG_I,
                       CKP_RENAMED_SUPD_PKG_D), RENAMED_SUPD_PKG_SYM);
            break;

         case DISABLING_INT_ERR_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                       CKP_DISABLING_INT_ERR_I,
                       CKP_DISABLING_INT_ERR_D), DISABLING_INT_ERR_SYM);
            break;

         case OEM_BASELEVEL_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                       CKP_OEM_BASELEVEL_ERR_I,
                       CKP_OEM_MISMATCH_ERR_D), OEM_BASELEVEL_SYM);
            break;

         case OEM_MISMATCH_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                       CKP_OEM_MISMATCH_ERR_I,
                       CKP_OEM_MISMATCH_ERR_D), OEM_MISMATCH_SYM);
            break;

         case OEM_SPECIFIC_SYM :
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                       CKP_OEM_SPECIFIC_ERR_I,
                       CKP_OEM_SPECIFIC_ERR_D), OEM_SPECIFIC_SYM);
            break;

         case UNKNOWN_SYM :
         default :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_UNCLEAR_FAILURE_I,
                     CKP_UNCLEAR_FAILURE_D), UNKNOWN_SYM);
            break;

     } /* end switch */
  } /* end for */

   inu_msg (ckp_errs, "\n");
} /* end print_report_failure_legend */

/*--------------------------------------------------------------------------*
**
** NAME: print_requisite_failures_hdr
**
** FUNCTION:  Prints requisite/dependent failure header for the current
**            operation.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_requisite_failures_hdr 
 (
boolean auto_supersedes_failed
)
{
   switch (check_prereq.mode)
   {
      case OP_APPLY :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_FAIL_APPLY_TITLE_I,
                  CKP_REQ_FAIL_APPLY_TITLE_D));
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_FAIL_APPLY_HDR_I,
                  CKP_REQ_FAIL_APPLY_HDR_D));
         if (auto_supersedes_failed)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_AUTO_SUPS_WARNING1_W, 
                     CKP_AUTO_SUPS_WARNING1_D));
         inu_msg (ckp_errs, "\n");
         break;
    
      case OP_COMMIT :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQ_FAIL_COMT_HDR_I,
                  CKP_REQ_FAIL_COMT_HDR_D));
         break;

      case OP_REJECT :
         if (check_prereq.deinstall_submode)
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEP_FAIL_DEINST_HDR_I,
                     CKP_DEP_FAIL_DEINST_HDR_D));
         }
         else
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEP_FAIL_REJECT_HDR_I,
                     CKP_DEP_FAIL_REJECT_HDR_D));
         }
         break;
       
   } /* end switch */

} /* end print_requisite_failures_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: print_subgraph_requisite
**
** FUNCTION:  Prints the subgraph member pointed to by fix_ptr for failed
**            requisite subgraphs with installp verbosity set at V4.
**            The subgraph node is printed indented according to indent_lev
**            and a colon-separated code is printed of the form
**            (prereq type:state).
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_subgraph_requisite 
 (
fix_info_type       * fix_ptr,
requisite_list_type * req_node,
short                 indent_lev,
short                 num_spaces
)
{
   char   print_string [MAX_LPP_FULLNAME_LEN];
   char   buff [MAX_LPP_FULLNAME_LEN];

   /*
    * Print dummy nodes with the criteria used in the requisite expression.
    */
   if (fix_ptr->origin == DUMMY_FIX)
   {
      get_criteria_string (&(req_node->criteria), buff);
      sprintf (print_string, "%s %s", fix_ptr->name, buff);
   }
   else
   {
      get_fix_name_from_fix_info (fix_ptr, print_string);
   }

   /*
    * Calculate the number of spaces to print (will be determined by the
    * number of digits in the indent level being printed, in addition to 
    * indent_lev's value. 
    */
   if   (indent_lev < 10)
        num_spaces = num_spaces - 1;
   else if (indent_lev < 100)
        num_spaces = num_spaces - 2;
   else
        num_spaces = num_spaces - 3;

   /*
    * Create a format string then print it.
    */
   sprintf (buff, "%d%%%ds %%s",indent_lev, num_spaces);
   inu_msg (ckp_errs, buff, "", print_string);

    /*
     * print: requisite type followed by applied/committed/"availability" state.
     */
    inu_msg (ckp_errs, " (");
    switch (req_node->type)
    {
      case A_PREREQ: 
           inu_msg (ckp_errs, "p");
           break;
      case AN_INSTREQ:
           inu_msg (ckp_errs, "I");
           break;
      case A_COREQ:
           inu_msg (ckp_errs, "c");
           break;
      case AN_IFREQ:
           inu_msg (ckp_errs, "oi");
           break;
      case AN_IFFREQ:
           inu_msg (ckp_errs, "i");
           break;
    } /* switch */

    inu_msg (ckp_errs, ":");

    if (fix_ptr->apply_state == BROKEN)
    {
       inu_msg (ckp_errs, "b");
    }
    else
    {
       if (fix_ptr->parts_applied != 0)    /* any parts installed? */
       {
          if (fix_ptr->parts_committed != 0)  /* how about committed */
          {
             if (fix_ptr->parts_committed == fix_ptr->parts_in_fix)
                inu_msg (ckp_errs, "c");
             else
                inu_msg (ckp_errs, "pc");
          }
          else
          {
             if (fix_ptr->parts_applied == fix_ptr->parts_in_fix)
                inu_msg (ckp_errs, "a");
             else
                inu_msg (ckp_errs, "pa");
          }
       }
       else
       {
         /*
          * No parts installed, must be either on the media or missing.
          */
         if (fix_ptr->origin == TOC ||
             (fix_ptr->origin == COMMAND_LINE && check_prereq.mode == OP_APPLY))
            inu_msg (ckp_errs, "o");
         else
            inu_msg (ckp_errs, "m");

       } /* end if */

    } /* end if */
    inu_msg (ckp_errs, ")\n");

} /* print_subgraph_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: print_success_hdr_by_request_type
**
** FUNCTION:  Prints header messages for success section based on 
**            request type flag passed and the type of operation.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_success_hdr_by_request_type 
 (
short request_type
)
{

   if (request_type == EXPLICIT)
   {
      inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_RQSTD_PKGS_HDR_I,
               CKP_SUCC_RQSTD_PKGS_HDR_D));
      /*
       * Print an extra message to alert users that the packages being
       * installed are not necessarily those requested (supersedes case)
       * The flag used is set in determine_order_of_fixes ().
       */
      if (check_prereq.successful_auto_supersedes)
         inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_AUTO_SUPS_WARNING2_W, 
                  CKP_AUTO_SUPS_WARNING2_D));
   }
   else if (request_type == IMPLICIT)
   {
      switch (check_prereq.mode)
      {
         case OP_APPLY :
         case OP_COMMIT :  
            inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQUISITES_TITLE_I,
                     CKP_SUCC_REQUISITES_TITLE_D));

            if (check_prereq.mode == OP_APPLY)
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQS_APPLY_HDR_I,
                        CKP_SUCC_REQS_APPLY_HDR_D));
            else
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQS_COMT_HDR_I,
                        CKP_SUCC_REQS_COMT_HDR_D));
            break;

         case OP_REJECT :
            inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_DEPS_TITLE_I,
                     CKP_SUCC_DEPS_TITLE_D));
            if (check_prereq.deinstall_submode)
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_DEPS_DEINST_HDR_I,
                        CKP_SUCC_DEPS_DEINST_HDR_D));
            else
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_DEPS_REJECT_HDR_I,
                        CKP_SUCC_DEPS_REJECT_HDR_D));
            break;

      } /* end switch */
   }
   else if (request_type == AUTO_IFREQ)
   {
      inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_AUTO_IFREQS_HDR_I,
               CKP_SUCC_AUTO_IFREQS_HDR_D));
   }
   else if (request_type == AUTO_MC)
   {
      inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_AUTO_MLEV_TITLE_I,
               CKP_SUCC_AUTO_MLEV_TITLE_D));

      if (check_prereq.mode == OP_APPLY)
         inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_AUTO_MLEV_APPLY_HDR_I,
                  CKP_SUCC_AUTO_MLEV_APPLY_HDR_D));
      else
         inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_AUTO_MLEV_COMT_HDR_I,
                  CKP_SUCC_AUTO_MLEV_COMT_HDR_D));
   }
   else if (request_type == MANDATORY)
   {
        inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_AUTO_MAND_HDR_I,
                 CKP_SUCC_AUTO_MAND_HDR_D));
   }

} /* end print_success_hdr_by_requisite_type */

/*--------------------------------------------------------------------------*
**
** NAME: print_successful_requisite 
**
** FUNCTION:  Prints the name and level of the fix_ptr passed, with its
**            INSTALLED/COMMITTED status for apply and commit ops.
**            The pkg passed represents a requisite/dependent of a successful 
**            member of the SOP being passed back to installp. 
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_successful_requisite 
 (
fix_info_type * fix_ptr
)
{
   char  name_lev [MAX_LPP_FULLNAME_LEN];

   if (check_prereq.mode == OP_REJECT)
   {
      inu_msg (ckp_succ, "      %s %s\n", fix_ptr -> name,
               get_level_from_fix_info (fix_ptr, name_lev));
   }
   else
   {
      if (fix_ptr->flags & MEMBER_OF_SOP)
      {
         switch (check_prereq.mode)
         {
            case OP_APPLY :
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQ_TO_BE_INST_I,
                        CKP_SUCC_REQ_TO_BE_INST_D),
                        fix_ptr -> name,
                        get_level_from_fix_info (fix_ptr, name_lev));
               break;

            case OP_COMMIT :
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQ_TO_BE_COMTD_I,
                        CKP_SUCC_REQ_TO_BE_COMTD_D),
                        fix_ptr -> name,
                        get_level_from_fix_info (fix_ptr, name_lev));
               break;

         } /* end switch */
      }
      else
      {
         switch (check_prereq.mode)
         {
            case OP_APPLY :
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQ_INSTLD_I,
                        CKP_SUCC_REQ_INSTLD_D),
                        fix_ptr -> name,
                        get_level_from_fix_info (fix_ptr, name_lev));
               break;

            case OP_COMMIT :
               inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCC_REQ_COMTD_I,
                        CKP_SUCC_REQ_COMTD_D),
                        fix_ptr -> name,
                        get_level_from_fix_info (fix_ptr, name_lev));
               break;
         } /* end switch */
      }
  } /* end if */

} /* end print_successful_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: print_successful_sop_entry
**
** FUNCTION:  Formats the lpp_name, level and description from the sop_ptr
**            passed then prints the formatted string. 
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_successful_sop_entry 
(
Option_t * sop_ptr,
boolean  * error
)
{
   boolean   desc_first;
   char      name_lev [MAX_LPP_FULLNAME_LEN * 2];
   char      print_string [MAX_LPP_FULLNAME_LEN * 2];
   char      ptfid [MAX_PROD_PTF * 2];

   inu_get_optlevname_from_Option_t (sop_ptr, name_lev);

   if (check_prereq.instp_verbosity > V1)
   {
      /*
       * for V>V1, we'll print name and level on first line followed by
       * description on next line.
       */ 

      /*
       * First get "success code" string.
       */
      strcat (name_lev, " (");
      strcat (name_lev, get_success_code (print_string, sop_ptr));
      strcat (name_lev, ")\n    ");

      /*
       * Get a description to print.
       */
      if (sop_ptr->desc == NIL (char))
      { 
         if (IF_INSTALL (sop_ptr -> op_type))
            strcat (name_lev, BASE_PKG); 
         else
            strcat (name_lev, UPDT_PKG); 
      }
      else
      {
         strcat (name_lev, sop_ptr->desc); 
      }
      inu_msg (ckp_succ, "%s%s\n", SUCCESS_INDENT_STR, name_lev);
   }
   else
   {
      desc_first = FALSE;
      ptfid[0]='\0';
      /*
       * Add indentation to the name_lev string using print_string as a
       * temp location.
       */
      strcpy (print_string, SUCCESS_INDENT_STR); 
      strncat (print_string, name_lev, strlen (name_lev));
      strcpy (name_lev, print_string);
      print_string[0]='\0';
      /*
       * Add "automatic supersedes" flag.
       */
      if (check_prereq.instp_verbosity == V1 &&
          IS_AUTO_SUPERSEDE_PKG (sop_ptr))
         strcat (name_lev, " *");

      format_pkg_name_and_desc (IF_INSTALL (sop_ptr->op_type), 
                                print_string, 
                                name_lev, 
                                sop_ptr->desc,
                                error);
      RETURN_ON_ERROR;
      inu_msg (ckp_succ, "%s\n", print_string);
   }
} /* print_successful_sop_entry */

/*--------------------------------------------------------------------------*
**
** NAME: print_success_key
**
** FUNCTION:  Prints the "Success Key" displayed when installp is invoked
**            with verbosity >= V2.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_success_key (void)
{
   inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_KEY_HDR_I, CKP_SUCCESS_KEY_HDR_D));
   switch (check_prereq.mode)
   {
      case OP_APPLY:
         inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_APPLY_SUCCESS_KEY_I, 
                  CKP_APPLY_SUCCESS_KEY_D));
         break;

      case OP_COMMIT:
         inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_COMT_SUCCESS_KEY_I, 
                  CKP_COMT_SUCCESS_KEY_D));
         break;

      case OP_REJECT:
         if (check_prereq.deinstall_submode)
            inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_DEINST_SUCCESS_KEY_I, 
                     CKP_DEINST_SUCCESS_KEY_D));
         else
            inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_REJECT_SUCCESS_KEY_I, 
                     CKP_REJECT_SUCCESS_KEY_D));
         break;

   } /* end switch */
} /* print_success_key */

/*--------------------------------------------------------------------------*
**
** NAME: print_success_section_header
**
** FUNCTION:  Prints the main SUCCESS section header for the current 
**            operation. 
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_success_section_header (void)
{

   inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_TITLE_I, 
            CKP_SUCCESS_TITLE_D));
   switch (check_prereq.mode)
   {
      case OP_APPLY :
           inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_APPLY_HDR_I, 
                    CKP_SUCCESS_APPLY_HDR_D));
           if (check_prereq.instp_verbosity > V1)
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_APPLY_HDR_GTR_V0_I, 
                       CKP_SUCCESS_APPLY_HDR_GTR_V0_D));
           if (check_prereq.instp_verbosity > V2) 
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_APPLY_HDR_GTR_V1_I, 
                       CKP_SUCCESS_APPLY_HDR_GTR_V1_D));
           inu_msg (ckp_succ, "\n");
           break;

      case OP_COMMIT :
           inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_COMT_HDR_I, 
                    CKP_SUCCESS_COMT_HDR_D));

           if (check_prereq.instp_verbosity > V1)
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_COMT_HDR_GTR_V0_I, 
                       CKP_SUCCESS_COMT_HDR_GTR_V0_D));

           if (check_prereq.instp_verbosity > V2) 
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_COMT_HDR_GTR_V1_I, 
                       CKP_SUCCESS_COMT_HDR_GTR_V1_D));
           inu_msg (ckp_succ, "\n");
  
           break;

      case OP_REJECT :
           if (check_prereq.deinstall_submode)
           {
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_DEINST_HDR_I, 
                       CKP_SUCCESS_DEINST_HDR_D));
              if (check_prereq.instp_verbosity > V1)
                 inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_DEINST_HDR_GTR_V0_I, 
                          CKP_SUCCESS_DEINST_HDR_GTR_V0_D));
           }
           else
           {
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_REJECT_HDR_I, 
                       CKP_SUCCESS_REJECT_HDR_D));
              if (check_prereq.instp_verbosity > V1)
                 inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_REJECT_HDR_GTR_V0_I, 
                          CKP_SUCCESS_REJECT_HDR_GTR_V0_D));
           }
           if (check_prereq.instp_verbosity > V2) 
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_SUCCESS_REMOVE_HDR_GTR_V1_I, 
                       CKP_SUCCESS_REMOVE_HDR_GTR_V1_D));
           inu_msg (ckp_succ, "\n");
  
           break;
   } /* end switch */

} /* end print_success_section_header */

/*--------------------------------------------------------------------------*
**
** NAME: print_TYPE_A_FAILURE_hdr
**
** FUNCTION:  Prints the header to the list of "type A" requisite 
**            failures.  For apply and commit, these are misssing 
**            requisites.  For reject, these are committed dependents. 
**            (not called for deinstall).
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_TYPE_A_FAILURE_hdr (void)
{
   if (check_prereq.called_from_command_line)
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_REQUIRED_PKGS_I, CKP_REQUIRED_PKGS_D)); 
      return;
   }

   switch (check_prereq.mode)
   {
      case OP_APPLY :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISS_REQ_APPLY_I, 
                  CKP_MISS_REQ_APPLY_D)); 
         break;

      case OP_COMMIT :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISS_REQ_COMT_I, 
                  CKP_MISS_REQ_COMT_D)); 
         break; 
    
      case OP_REJECT :
         if (check_prereq.deinstall_submode)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NON_DEINST_DEP_I, 
                    CKP_NON_DEINST_DEP_D)); 
         else
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_COMTD_DEPENDENTS_I, 
                    CKP_COMTD_DEPENDENTS_D)); 
         break;

   } /* end switch */
 
   print_dependent_ref_msg ();
 
} /* end print_TYPE_A_FAILURE_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: print_TYPE_B_FAILURE_hdr
**
** FUNCTION:  Prints group requisite failure header.  (called "type B" for
**            consistency with other related routines.) 
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_TYPE_B_FAILURE_hdr (void)
{
   if (check_prereq.called_from_command_line)
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_GROUP_FAIL_HDR1_I, 
               CKP_GROUP_FAIL_HDR1_D)); 
   else
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_GROUP_FAIL_HDR2_I, 
               CKP_GROUP_FAIL_HDR2_D)); 
} /* end print_TYPE_B_FAILURE_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: print_TYPE_C_FAILURE_hdr
**
** FUNCTION:  Prints header  to the list of "type C" requisite failures.
**            For apply, these are requisites will be applied if -g is used.
**            If -g has been used, these are the requisites that would have
**            been installed if other failures hadn't occurred. 
**            Same for requisites which need to be committed w.r.t. commit 
**            operation.  For reject and de-install, these are dependents
**            that "would have" or "should have" been committed (depending
**            on whether or not -g is used). 
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_TYPE_C_FAILURE_hdr (void)
{
   if (check_prereq.called_from_command_line)
      /* Should never get here. */
      return;

   switch (check_prereq.mode)
   {
      case OP_APPLY :
         if (check_prereq.Auto_Include)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_AVLBL_REQS_HDR1_I,
                     CKP_AVLBL_REQS_HDR1_D));
         else
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_AVLBL_REQS_HDR2_I,
                     CKP_AVLBL_REQS_HDR2_D));
         break;

      case OP_COMMIT :
         if (check_prereq.Auto_Include)
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLD_REQS_HDR1_I,
                     CKP_APPLD_REQS_HDR1_D));
         else
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLD_REQS_HDR2_I,
                     CKP_APPLD_REQS_HDR2_D));
         break;

      case OP_REJECT :
         if (check_prereq.Auto_Include)
         {
            if (check_prereq.deinstall_submode) 
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INSTLD_DEPS_HDR1_I,
                        CKP_INSTLD_DEPS_HDR1_D));
            else
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLD_DEPS_HDR1_I,
                        CKP_APPLD_DEPS_HDR1_D));
         }
         else
         {
            if (check_prereq.deinstall_submode) {
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INSTLD_DEPS_HDR2_I,
                        CKP_INSTLD_DEPS_HDR2_D));
               /*
                * Print an additional message to flag any updates that may
                * be printed as dependents of the pkgs selected.
                */
               if (check_prereq.deinst_dependencies_via_updates)
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_DEINST_DEPS_VIA_UPDTS_W,
                           CKP_DEINST_DEPS_VIA_UPDTS_D));
            } else {       
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLD_DEPS_HDR2_I,
                        CKP_APPLD_DEPS_HDR2_D));
            }
         }
         break;

   } /* end switch */

   print_dependent_ref_msg ();

} /* end print_TYPE_C_FAILURE_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: print_TYPE_C_FAILURE_msg
**
** FUNCTION:  Prints messages for the same type of failures described in
**            print_TYPE_C_FAILURE_hdr but prints a message with a numeric
**            reference to the packages which failed -- ie. the packages
**            are not listed after this header. 
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_TYPE_C_FAILURE_msg 
 (
short count
)
{
   switch (check_prereq.mode)
   {
      case OP_APPLY :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_AVLBL_REQ_MSG_I, 
                     CKP_AVLBL_REQ_MSG_D), count); 
         break;

      case OP_COMMIT :
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLD_REQ_MSG_I, 
                     CKP_APPLD_REQ_MSG_D), count); 
         break;

      case OP_REJECT :
            if (check_prereq.deinstall_submode)
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INSTLD_DEP_MSG_I, 
                        CKP_INSTLD_DEP_MSG_D), count); 
            else
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_APPLD_DEP_MSG_I, 
                        CKP_APPLD_DEP_MSG_D), count); 
         break;

   } /* end switch */

} /* end print_TYPE_C_FAILURE_msg */

/*--------------------------------------------------------------------------*
**
** NAME: print_TYPE_Z_FAILURE_hdr
**
** FUNCTION:  Prints the header to the list of "type D" requisite failures.
**            These are the miscellaneous coded requisite and dependent
**            failures.  ie.  The header is printed here, followed by the
**            list of failing requisites, with a symbol prepending each 
**            line, followed by a key explaining the meaning of each 
**            symbol. 
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
print_TYPE_Z_FAILURE_hdr 
 (
void
)
{
   if (check_prereq.called_from_command_line)
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_CMD_LINE_MISC_FAILING_REQ_HDR_I,
               CKP_CMD_LINE_MISC_FAILING_REQ_HDR_D)); 
      return;
   }

   switch (check_prereq.mode)
   {
      case OP_APPLY :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_FAILING_REQ_APPLY_HDR_I,
                  CKP_MISC_FAILING_REQ_APPLY_HDR_D)); 
         break;

      case OP_COMMIT :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_FAILING_REQ_COMT_HDR_I,
                  CKP_MISC_FAILING_REQ_COMT_HDR_D)); 
          break;

      case OP_REJECT :
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_MISC_FAILING_REQ_REMOVE_HDR_I,
                  CKP_MISC_FAILING_REQ_REMOVE_HDR_D)); 
         break;
   } /* end switch */

   print_dependent_ref_msg ();

} /* end print_TYPE_Z_FAILURE_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: print_warnings_hdr
**
** FUNCTION:  Prints the header for the WARNINGS section.  
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
print_warnings_hdr ()
{

   inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_WARNING_TITLE_I, CKP_WARNING_TITLE_D));
   inu_msg (ckp_warn, MSGSTR (MS_CKPREREQ, CKP_WARNING_HDR_I, CKP_WARNING_HDR_D));

} /* print_warnings_hdr */

/*--------------------------------------------------------------------------*
**
** NAME: refine_failsop_errors_for_reporting
**
** FUNCTION:  Scans the failsop looking for certain generic errors which
**            could be made more specific for better error reporting.
**
** RETURNS:   This is a void function.
**
** SIDE EFFECTS:
**            The Status field of op structures on the failsop may be changed.
**
**--------------------------------------------------------------------------*/

void 
refine_failsop_errors_for_reporting (Option_t * failsop)
{
   fix_info_type * fix;
   Option_t      * op;

   for (op = failsop->next;
        op != NIL (Option_t);
        op = op->next)
   {
      switch (op->Status)
      {
         case STAT_NUTTIN_TO_COMMIT:
            fix = locate_fix (op->name, &(op->level), USE_FIXID);
            if (fix != NIL (fix_info_type)) {
               if ((fix->parts_committed != 0) && 
                   ((fix->parts_committed == fix->parts_in_fix) ||
                    (fix->apply_state == BROKEN))) {
                  if ((op->desc == NIL (char)) || (op->desc[0] == '\0'))
                     op->desc = strdup (fix->description);
                  if (fix->apply_state != BROKEN) {
                     op->Status = STAT_ALREADY_COMMITTED;
                  } else {
                     op->Status = STAT_BROKEN;
                  }
               }
            }
            break;

         case STAT_NUTTIN_TO_REJECT:
            fix = locate_fix (op->name, &(op->level), USE_FIXID);
            if (fix != NIL (fix_info_type)) {
               if (fix->parts_committed != 0) {
                  if ((op->desc == NIL (char)) || (op->desc[0] == '\0'))
                     op->desc = strdup (fix->description);
                  if (fix->apply_state != BROKEN) {
                     op->Status = STAT_COMMITTED_CANT_REJECT;
                  } else {
                     op->Status = STAT_BROKEN;
                  }
               }
            }
            break;
  
         default:
            /* do nothing */
            break;

      } /* end switch */  
   } /* end for */

} /* end refine_failsop_errors_for_reporting */

/*--------------------------------------------------------------------------*
**
** NAME: reset_report_failure_legend
**
** FUNCTION:  This function sets the global flags, used to print the
**            report_failure legend, to FALSE
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
reset_report_failure_legend (void)
{
  legend_top = -1;

} /* end of reset_report_failure_legend */

/*--------------------------------------------------------------------------*
**
** NAME: same_criteria
**
** FUNCTION:  Compares the contents of the criteria structure arguments
**            for equality (ie.  checks if two requisite expressions are
**            the same).
**
** RETURNS:   TRUE if match, FALSE otherwise.
**
**--------------------------------------------------------------------------*/

boolean 
same_criteria 
(
criteria_type * crit1,
criteria_type * crit2
)
{
   /*
    * Don't go any further if our hash values (computed in create_relation())
    * don't match.  If they do match, proceed to check if each member of
    * each relation list in crit1 is contained in the corresponding relation
    * in crit2.  That is, if every member of crit1->version list is contained 
    * in crit2->version, crit1->release is contained in crit2->release, etc.
    * We need to do this since hash values for the level in a requisite 
    * expression 4.1.1.0 is equal to 4.1.0.1.
    */
   if (crit1->hash_id != crit2->hash_id)
      return (FALSE);

   if (! same_relation (crit1->version, crit2->version))
      return  (FALSE);

   if (! same_relation (crit1->release, crit2->release))
      return  (FALSE);

   if (! same_relation (crit1->modification, crit2->modification))
      return  (FALSE);

   if (! same_relation (crit1->fix, crit2->fix))
      return  (FALSE);

   /*
    * Return the equality based on matching ptf ids.  Since most 4.1 req 
    * expressions will not contain ptf id, don't bother doing strcmp if both 
    * are empty.
    */
   return ((crit1->ptf->ptf_value[0] == '\0' && 
            crit2->ptf->ptf_value[0] == '\0') 
                                ||
           (strcmp (crit1->ptf->ptf_value, crit2->ptf->ptf_value)==0));

} /* same_criteria */

/*--------------------------------------------------------------------------*
**
** NAME: same_relation
**
** FUNCTION:  Compares each member of list rel1 with members of list rel2.
**            NOTE: This does not check for true equality.  rel2 may contain
**            more members.  Than rel1, and this list will still report
**            that the two are the same.  This is an acceptable limitation
**            since most 4.1 requisites will contain the same number of 
**            elements in rel1 and rel2. (since requisites are more than
**            likely of the form v.r.m.f).
**
** RETURNS:   TRUE if rel1 is contained in rel2.
**
**--------------------------------------------------------------------------*/

static boolean 
same_relation 
(
relation_type * rel1,
relation_type * rel2
)
{
   relation_type * relation1;
   relation_type * relation2;

    /* 
     * loop on the current relation list:  Could be a list of expressions
     * representing the version part of a requisite expression:
     * ex. v=1 o>1  would be stored as relation1:(=, 1)--next_rel--> (>, 1)
     */
    for (relation1 = rel1;
         relation1 != NIL (relation_type);
         relation1 = relation1 -> next_relation)
    {
       for (relation2 = rel2;
            relation2 != NIL (relation_type);
            relation2 = relation2 -> next_relation)
       {
           if (relation1->operator == relation2->operator &&
              relation1->int_value == relation2->int_value)
              break;
       }
       if (relation2 == NIL (relation_type))
          return (FALSE);
    }
    return (TRUE);

} /* same_criteria */
