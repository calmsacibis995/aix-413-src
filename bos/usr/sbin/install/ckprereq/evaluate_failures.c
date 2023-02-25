static char sccsid[] = "@(#)75  1.22  src/bos/usr/sbin/install/ckprereq/evaluate_failures.c, cmdinstl, bos41J, 9517A_all 4/20/95 19:52:25";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            evaluate_failures_and_warnings
 *            add_subgraph_root_to_dependents_list
 *            evaluate_group_failure_subgraph
 *            evaluate_requisite_failure_subgraph
 *            is_instreq_failure_subgraph
 *            mark_successful_subgraph
 *            remove_failures_and_successes_from_warning_list
 *            
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

#include <check_prereq.h>

/* Global variable to indicate failure to install products or updates
   input from the command line. (Patch for 3.2 SMIT processing.)  */
int cmd_line_prod_fail;

static void add_subgraph_root_to_dependents_list (
fix_info_type        * subgraph_root,
requisite_list_type  * req_node,
boolean              * error);

static void evaluate_group_failure_subgraph (
fix_info_type * subgraph_root,
fix_info_type * group_node,
boolean       * error);

static boolean is_instreq_failure_subgraph (
fix_info_type * cmd_line_pkg);

static void mark_successful_subgraph (
fix_info_type       * node,
requisite_list_type * req_node);

static void remove_failures_and_successes_from_warning_list ();

static boolean
is_instreq_failure_subgraph_be (
fix_info_type * parent,
boolean       * instreq_failure,
boolean       * other_failure);

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_failures_and_warnings
**
** FUNCTION:  Scans the requisite chain of the anchor node looking for 
**            failures which may have occurred.  Sets flags in fix_info nodes 
**            and flags passed as arguments according to the failures detected.
**            Also invokes a routine to check for failures and warnings on
**            failsop.  For requisite failures, this routine adds the structure
**            representing a pkgs requested by user ("command line fix") to a 
**            dependents list of each failed requisite in its subgraph.
**            This allows failed requisites to refer back to the command line 
**            fix which they're causing failure for.
**
** RETURNS:   TRUE if any failures or warnings were detected.  False otherwise.
** 
** SIDE EFFECTS:
**            Sets flags to TRUE or FALSE depending upon whether or not the
**            pertinent failure/warning was detected.
**
**--------------------------------------------------------------------------*/

boolean 
evaluate_failures_and_warnings 
(
boolean * ckp_non_req_failures,              
boolean * instp_non_req_failures,
boolean * requisite_failures,
boolean * non_req_auto_sups_failure,
boolean * req_auto_sups_failure,
boolean * fix_info_warnings,
boolean * fail_sop_warnings,
boolean * error
)
{
   fix_info_type       * cmd_line_pkg;
   requisite_list_type * requisite;
   requisite_list_type * requisite2;

   (*ckp_non_req_failures)      = FALSE;
   (*requisite_failures)        = FALSE; 
   (*non_req_auto_sups_failure) = FALSE;
   (*req_auto_sups_failure)     = FALSE;
   (*fix_info_warnings)         = FALSE;

   /*
    * Scan the list of pkgs on the anchor's requisite list.  (This includes 
    * non-requisite pkgs that were automatically pulled in such as installp
    * updates, mandatory updates, unresolved ifreqs, automatic supersedes.) 
    * Establish if that package failed based on state and the SUCCESS/FAILURE
    * tags set in the flag field. 
    * Rules:
    *       if pkg does not have a "this nodes state" of TO_BE_EXPLICITLY_XXXX 
    *          it failed in its own right.  ie.  It didn't fail because of 
    *          problems with requisites.  These states were set in earlier
    *          processing (see load_fix_info_table(), evaluate_base_levels(),
    *          etc.)
    *             EXCEPTION:  if pkg is flagged as an UNRESOLVED_IFREQ, then
    *              it more than likely did fail because of problems with 
    *              requisites.  However, these will be treated as warnings 
    *              rather than hard requisite failures.  They are left on 
    *              the requisite list of the anchor for convenience.
    *              ifreq warnings will ONLY be reported if the -v flag is
    *              specified.
    *       else
    *          if pkg has not been tagged as SUCCESSFUL
    *             it must be a requisite failure. 
    *             EXCEPTION: if pkg only failed because of instreqs, then
    *              the failure will not be reported (or treated as a failure)
    *              for a special level of verbosity specified by installp (V0) 
    *            
    */ 
   for (requisite = check_prereq.fix_info_anchor -> requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      cmd_line_pkg = requisite -> fix_ptr;

      /*
       * First, see if this node has failed.  If so, set a flag 
       * indicating the type of failure.
       */
      if (((check_prereq.mode == OP_APPLY)   &&
            (cmd_line_pkg->apply_state != TO_BE_EXPLICITLY_APPLIED))
                                       ||
           ((check_prereq.mode == OP_COMMIT) &&
            (cmd_line_pkg->commit_state != TO_BE_EXPLICITLY_COMMITTED))
                                       ||
           ((check_prereq.mode == OP_REJECT) &&
            (cmd_line_pkg->reject_state != TO_BE_EXPLICITLY_REJECTED)))
      {
         if ((check_prereq.mode == OP_APPLY) &&
             (cmd_line_pkg->flags & UNRESOLVED_IFREQ))
         {
            if (check_prereq.instp_verify)
            {
               cmd_line_pkg->flags |= REPT_IFREQ_WARNING;
               (* fix_info_warnings) = TRUE;
            }
         }
         else
         {
            /*
             * Note:  cmd_line_prod_fail is a global variable used to inform
             * installp (if applicable) that a product requested from the
             * cmd_line failed requisite checks. -- a patch for 3.2.0
             * SMIT pass/failure processing.
             */
            cmd_line_prod_fail = TRUE;
            cmd_line_pkg->flags |= REPT_CMD_LINE_FAILURE;
            (* ckp_non_req_failures) = TRUE;
            if (cmd_line_pkg->flags & AUTO_SUPERSEDES_PKG)
               (* non_req_auto_sups_failure) = TRUE;
         }
      }
      else
      {
         if (!(cmd_line_pkg -> flags & SUCCESSFUL_NODE))
         {
            /* 
             * Ignore instreq failures if instructed to do so.
             * (when -Q flag was specified at installp command line)
             */
            if (! (check_prereq.suppress_instreq_failures &&
                   is_instreq_failure_subgraph (cmd_line_pkg)))
            {
               cmd_line_pkg->flags |= REPT_REQUISITE_FAILURE;
               cmd_line_prod_fail = TRUE;
               (* requisite_failures) = TRUE;
               if (cmd_line_pkg->flags & AUTO_SUPERSEDES_PKG)
                  (* req_auto_sups_failure) = TRUE;
            }
         }
         else
         {
            mark_successful_subgraph (cmd_line_pkg, 
                                              NIL (requisite_list_type));
            unmark_SUBGRAPH (cmd_line_pkg, 
                             (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED));
         }
      } /* end if */

      /*
       * Make all requisites of the failing node (which represents a pkg 
       * requested from the installp cmd line) point back to it.
       */
      if (cmd_line_pkg->flags & REPT_REQUISITE_FAILURE)
      {
         for (requisite2 = cmd_line_pkg->requisites;
              requisite2 != NIL (requisite_list_type);
              requisite2 = requisite2->next_requisite)
         {
            evaluate_requisite_failure_subgraph (cmd_line_pkg, requisite2, 
                                                 error);
            RETURN_ON_ERROR;
         } /* end for */
         unmark_graph (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED);
         unmark_requisite_visited ();
      } /* end if */
   } /* end for */

   check_failures_on_Fail_SOP (instp_non_req_failures, fail_sop_warnings);

   /*
    * If any warnings were detected earlier, remove them from the warnings
    * list if they subsequently resulted in a failure or success.  
    * (determined in processing above).
    */
   remove_failures_and_successes_from_warning_list ();

   /*
    * Check the miscellaneous warnings list. 
    */
   (* fix_info_warnings) |= (report_anchor.misc_warnings_hdr->next_index_node !=
                             report_anchor.misc_warnings_tail);

   return ((*ckp_non_req_failures)    ||
           (*instp_non_req_failures)  ||
           (*requisite_failures)      ||
           (*fix_info_warnings)       ||
           (*fail_sop_warnings));

} /* evaluate_failures_and_warnings */

/*--------------------------------------------------------------------------*
**
** NAME: add_subgraph_root_to_dependents_list
**
** FUNCTION:  Creates and adds an index node to the rept_dependents list of
**            the failed requisite node passed as an argument (via req_node).
**            The index_node points to subgraph_root.
**
** RETURNS:   Void Function.
**
**--------------------------------------------------------------------------*/

static void 
add_subgraph_root_to_dependents_list
(
fix_info_type        * subgraph_root,
requisite_list_type  * req_node,
boolean              * error
)
{
   fix_info_type   * failed_requisite;
   fix_info_type   * srch_fix;
   index_list_type * dep_node;
   index_list_type * insert_index;
   index_list_type * insert_prev;
   index_list_type * match_index;
   index_list_type * match_prev;
   index_list_type * srch_index;
   index_list_type * srch_index2;
   index_list_type * srch_prev;

   failed_requisite = req_node->fix_ptr;

   /*
    *  Notes:
    *  -- the goal is to add an index node to the "rept_dependents" list of 
    *     "failed_requisite". The index node will point to the subgraph_root.
    *  -- information in the criteria structure of the requisite node will be
    *     used to help determine when to add an entry to the dependents list.
    *  -- Special Case Scenario:
    *     A                               B
    *       *prereq Z v=1 r=2                *prereq Z v=1 r=2 m=3
    *
    *     If Z is not on the media/not installed (represented by a DUMMY_FIX 
    *     or fix from the vpd with apply_state == AVAILABLE)  we need to make 
    *     sure we report the two different criteria which satisfies A's and B's 
    *     requirements.  To implement this, we'll add separate entries to the 
    *     dependents list of Z.
    * 
    *  -- Normal Case :
    *     A                               B
    *       *prereq Z v=1 r=2                *prereq Z v=1 r=2
    *
    *     If Z is not on the media, we'll flag the first index_node on 
    *     the depedendents list for a given set of matching criteria.  This
    *     will prevent redundant failure messages when Z is "missing."
    *
    *  -- Example:  combination of the above 2 scenarios:
    *     A                      B                        C
    *       *prereq Z v=1 r=2      *prereq Z v=1 r=2 m=3     *prereq Z v=1 r=2
    *    
    *     A, B, and C were requested for installation, Z was not available. 
    *     The rept_dependents list of Z would look like the following:
    * 
    *                        +----------------------+ 
    *       rept_dependents  | criteria: v=1, r=2   |
    *     Z ---------------> |                      |------ ptr to A --->
    *                        | unique_criteria: TRUE|
    *                        +----------------------+
    *                                   | 
    *                                   | next
    *                                   v 
    *                        +----------------------+ 
    *                        | criteria: v=1, r=2   |
    *                        |                      |----- ptr to C --->
    *                        | unique_criteria:FALSE|
    *                        +----------------------+
    *                                   | 
    *                                   | next
    *                                   v 
    *                        +----------------------+ 
    *                        |criteria:v=1, r=2, m=3|
    *                        |                      |----- ptr to B --->
    *                        | unique_criteria: TRUE|
    *                        +----------------------+
    */ 



   /* 
    * Find out if there are any criteria matches.  If we find a match in 
    * criteria for the current subgraph_root, we'll just return.
    */
   match_index = NIL (index_list_type);
   srch_index = failed_requisite->rept_dependents;
   match_prev = NIL (index_list_type);
   while ((match_index == NIL (index_list_type)) && 
          (srch_index != NIL (index_list_type)))
   { 
      if (same_criteria (&req_node->criteria, &srch_index->criteria))
      {
         match_index = srch_index;
         if (match_index->fix_ptr == subgraph_root)
            return;
         else
         {
            /* 
             * This particular match (in criteria) doesn't match the 
             * subgraph root.  Scan the rest of the list to make sure
             * nothing else in the list does either.  
             */
            srch_index2 = match_index->next_index_node;
            while (srch_index2 != NIL (index_list_type))
            {
                if (same_criteria (&req_node->criteria, &srch_index2->criteria)
                                               &&
                    srch_index2->fix_ptr == subgraph_root)
                   return;
                srch_index2 = srch_index2->next_index_node;
            } 
         }
      }
      else
      {
         match_prev = srch_index;
         srch_index = srch_index->next_index_node;
      }
   } /* end while */

   /*  
    * Create the node for the dependents list.  Copy the criteria information
    * over to the new node's criteria structure.  (NOTE:  DO NOT FREE 
    * THESE DEPENDENT, index_list_type NODES WITHOUT SETTING THE ELEMENTS OF 
    * CRITERIA TO NIL -- since they are now being shared by the index_list 
    * structure and the requisite list structure.)
    */
   dep_node = (index_list_type *) my_malloc (sizeof (index_list_type), error);
   RETURN_ON_ERROR;
   dep_node->criteria.flags         = req_node->criteria.flags;
   dep_node->criteria.hash_id       = req_node->criteria.hash_id;
   dep_node->criteria.version       = req_node->criteria.version;
   dep_node->criteria.release       = req_node->criteria.release;
   dep_node->criteria.modification  = req_node->criteria.modification;
   dep_node->criteria.fix           = req_node->criteria.fix;
   dep_node->criteria.ptf           = req_node->criteria.ptf;
   dep_node->fix_ptr                = subgraph_root;
   dep_node->unique_criteria        = FALSE;


   /*
    * Now find a place to insert in the dependents list. We try to preserve 
    * alphanumeric order of fix_info nodes so that we can print better 
    * numeric references (lists of numbers) to nodes that are dependent on
    * the current node. But first, initialize our search pointers based on 
    * what we found above.
    */
   if (match_index != NIL (index_list_type))
   {
      insert_prev  = match_prev;  
      srch_index   = match_index;  
   }
   else
   {
      insert_prev  = NIL (index_list_type);
      srch_index   = failed_requisite->rept_dependents; 
      dep_node->unique_criteria = TRUE;
   } /* end if */

   insert_index    = NIL (index_list_type);
   while ((insert_index == NIL (index_list_type)) && 
          (srch_index != NIL (index_list_type)))
   {
      srch_fix = srch_index->fix_ptr;
      if (is_2nd_greater (subgraph_root->name, srch_fix->name,
                         &(subgraph_root->level), &(srch_fix->level),
                         subgraph_root->description, 
                         srch_fix->description))
      {
         /*
          * Yay!  We can insert here.
          */
         insert_index = srch_index;
      }
      else
      {
         /*
          * We haven't found our alphanumeric insertion point.  If this was a
          * matching_criteria insertion, stop searching if we've come upon
          * another set of unique criteria (ie. if we're trying to insert 
          * another v=1, r=2 in our example and srch_index is pointing to 
          * the v=1, r=2, m=3 entry.
          */
         if ((match_index != NIL (index_list_type)) &&
             (srch_index != match_index)           &&
             (srch_index->unique_criteria))
         {
            insert_index = srch_index;       
         } 
         else
         {
            insert_prev = srch_index;
            srch_index = srch_index->next_index_node;

         } /* end if */
      } /* end if */
   } /* end while */

   /*
    * If we had a match on criteria, reset which criteria is unique if
    * necessary -- that is, if we're inserting before the previous 
    * criteria that we considered to be unique.
    */
   if ((match_index == insert_index)  &&
       (match_index != NIL (index_list_type)))
   {
      match_index->unique_criteria = FALSE;
      dep_node->unique_criteria    = TRUE;
   }

   /*
    * Insert the index_node in the failed_requisite's dependents list.
    */
   dep_node->next_index_node = insert_index;
   if (insert_prev != NIL (index_list_type))
      insert_prev->next_index_node = dep_node;
   else
      req_node->fix_ptr->rept_dependents = dep_node;

} /* end add_subgraph_root_to_dependents_list */

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_group_failure_subgraph
**
** FUNCTION:  Given a group node in a failed requisite subgraph, determines 
**            if that  group should be printed in group-failure format.
**            If so, marks the group node, if not, treats the failed subgraph
**            as if the group node is not there (calls 
**            evaluate_requisite_failure_subgraph).
**
** RETURNS:   Void Function.
**
**--------------------------------------------------------------------------*/

static void 
evaluate_group_failure_subgraph
(
fix_info_type * subgraph_root,
fix_info_type * group_node, 
boolean       * error
)
{
   boolean               nested_group = FALSE;
   fix_info_type       * fix;
   index_list_type     * dep_node;
   requisite_list_type * requisite;
   short                 satisfied = 0;
   short                 satisfiable = 0;
   short                 total = 0;
   short                 eval_count = 0;

   if (group_node->flags & SUCCESSFUL_NODE)
      /*
       * Don't process successful groups -- could be successful even though
       * it's a member of an unsuccessful subgraph.
       */
      return;

   if (check_prereq.mode == OP_APPLY)
   {
      get_group_req_stats (group_node, &satisfied, &satisfiable, &total,
                           &nested_group);
      /*
       * First, we'll check if we really need to report things in 
       * group requisite format.  
       * Rules:
       * --Always print nested groups in group format -- not worth
       *   the complexity to unravel nested groups -- subgroups may be
       *   optimized in the group reporting code however.
       * --Don't print a group failure in group format if the number of
       *   satisfied and satisfiable group members is greater than or 
       *   equal to the number of passes required.
       * --passes required by group expression is the same as the number
       *   of members of the group (common packaging error).
       */
      if ((! nested_group) 
                 &&
          ((group_node->passes_required <= (satisfied + satisfiable)) ||
           (group_node->passes_required == total)))
      {
         /*
          * This appears to be a case of a failed group node, with enough
          * of its children installed or installable to pass.  (Installed 
          * children may have missing requisites of their own.)  
          * Treat these satsified/satisfiable group members as normal children,
          * descending their subgraphs and attaching subgraph members to the 
          * subgraph root.
          */
         for (requisite = group_node -> requisites;
              requisite != NIL (requisite_list_type);
              requisite = requisite -> next_requisite)
         {
             if (requisite->fix_ptr->parts_applied != 0    
                                   ||
                 requisite->fix_ptr->apply_state == BROKEN 
                                   ||
                 (requisite->fix_ptr->origin != DUMMY_FIX   &&
                 requisite->fix_ptr->apply_state != AVAILABLE)
                                   ||
                 (group_node->passes_required == total) )
             {
                if ((satisfied < group_node->passes_required)
                                  ||
                   ((satisfied >= group_node->passes_required) &&
                   (requisite->flags.selected_member_of_group)))
                {

                    evaluate_requisite_failure_subgraph (subgraph_root, 
                                                         requisite, error);
                    RETURN_ON_ERROR;
                    eval_count++;
                    if (eval_count >= group_node->passes_required)
                       /*
                       * Quit when we've printed enough -- since this is how
                       * group memebers will be automatically selected anyway.
                       */
                       break;
                 }
             }
         } /* end for */
      }
      else
      {
         /*
          * We will print this group requisite.  Mark it and create a dependent 
          * link to its subgraph_root.  NOTE: Make sure there is only one
          * dependent node on a given group's dependents list.  It's feasible 
          * that we could get to this group from numerous subgraph roots, but 
          * that's going to lead to lots of silly looking messages.
          */
         group_node->flags |= REPT_FAILED_REQUISITE;
         if (group_node->rept_dependents == NIL (index_list_type))
         {
            dep_node = (index_list_type *) my_malloc (sizeof (index_list_type),
                                                      error);
            RETURN_ON_ERROR;
            dep_node->fix_ptr = subgraph_root;
            dep_node->next_index_node = NIL (index_list_type);
            group_node->rept_dependents = dep_node;
         }
      } /* end if */
   } 
   else
   {
      /* 
       * We won't print these failures in group format.  Let's just call the
       * normal subgraph failure processing routine.
       */
      for (requisite = group_node -> requisites;
           requisite != NIL (requisite_list_type);
           requisite = requisite -> next_requisite)
      {
         evaluate_requisite_failure_subgraph (subgraph_root, requisite, error);
         RETURN_ON_ERROR;
      } /* end for */

   } /* end if */

} /* end evaluate_group_failure_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_requisite_failure_subgraph
**
** FUNCTION:  Called by evaluate_failures, this routine traverses the 
**            subgraph of a pkg requested at the installp cmd line, 
**            learning and saving information about the nodes in that failed
**            subgraph.  The information will be printed later.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
evaluate_requisite_failure_subgraph
(
fix_info_type       * subgraph_root, /* cmd line pkg which failed */
requisite_list_type * req_node,     /* node which points to requsite pkg */
boolean             * error
)
{
  boolean                ignore_subgraph;
  boolean                failure_detected;
  fix_info_type        * current_fix = req_node->fix_ptr;
  requisite_list_type  * requisite;

  /*
   * Return if:
   *   -- we've been here 
   *   -- we don't want to consider an ifreq subgraph
   */
  if (check_for_cycle_or_ifreq_subgraph (current_fix, req_node, TRUE))
     return; 
  /*
   * Set cycle detection flags.
   */
  current_fix -> flags |= VISITING_THIS_SUBGRAPH; 
  req_node->flags.requisite_visited = TRUE;

  if (current_fix -> origin == DUMMY_GROUP)
  {
     evaluate_group_failure_subgraph (subgraph_root, current_fix, error);
     RETURN_ON_ERROR;
  }
  else
  {
     failure_detected = FALSE;
     ignore_subgraph = FALSE;
     switch (check_prereq.mode)
     {
        case OP_APPLY:
           switch (current_fix -> apply_state)
           {
              case ALL_PARTS_APPLIED :
              case TO_BE_IMPLICITLY_APPLIED :
                 /* 
                  * don't need to print this one - get out of switch & continue
                  * to look at remaining requisites. 
                  */
                 break;

              case TO_BE_EXPLICITLY_APPLIED :
                 /* 
                  * We don't want to mark this pkg (and print it) as a failed 
                  * requisite of the subgraph_root, since it is a command line 
                  * pkg and will be reported seperately.  If this node has 
                  * failed, we do want to look at its subgraph, since it may 
                  * be the cause of the current subgraph_root's failure.
                  */ 
                 if (current_fix->flags & SUCCESSFUL_NODE)
                    ignore_subgraph = TRUE;
                 break;

              default :
                 failure_detected = TRUE;
                 break;
           } /* end switch */
           break;  /* case OP_APPLY */

        case OP_COMMIT:
           switch (current_fix -> commit_state)
           {
              case ALL_PARTS_COMMITTED :
              case TO_BE_IMPLICITLY_COMMITTED :
                 /* 
                  * don't need to print this one - get out of switch & continue
                  * to look at remaining requisites. 
                  */
                 break;

              case TO_BE_EXPLICITLY_COMMITTED :
                 /* 
                  * (See equivalent code comment for apply above.)
                  */ 
                 if (current_fix->flags & SUCCESSFUL_NODE)
                    ignore_subgraph = TRUE;
                 break;

              default :
                 failure_detected = TRUE;
                 break;
           } /* end switch */
           break;  /* case OP_COMMIT */

        case OP_REJECT:
           switch (current_fix -> reject_state)
           {
              case TO_BE_IMPLICITLY_REJECTED :
                 /* 
                  * This pkg is being pulled in as someone elses
                  * requisite and is not part of this subgraph's failure.
                  */
                 break;
              case TO_BE_EXPLICITLY_REJECTED :
                 /* 
                  * (See equivalent code comment for apply and commit above.)
                  */ 
                 if (current_fix->flags & SUCCESSFUL_NODE)
                    ignore_subgraph = TRUE;
                 break;

              default :
                 /*
                  * We only want to report failures that hit this path
                  * for deinstall ops, if an update prereqs/coreqs 
                  * another product/option when its base level does not.
                  */
                 if (IF_INSTALL (current_fix->op_type) ||
                     !check_prereq.deinstall_submode) {
                    /*
                     * This is a base level pkg or we're doing a reject -- need
                     * to report this as a dependency failure.
                     */
                    failure_detected = TRUE;
                 } else { 
                    /*
                     * Must be an update on a de-install op.  Only report it
                     * if the update's base level does not prereq/coreq the
                     * base level of the pkg that is dependent on the 
                     * subgraph_root.
                     * ex:
                     *       CUP1 --prereqs--> D
                     * CUP1 (which is now linked to D as its prereq --  see
                     * invert_requisites()) should be reported as a dependent
                     * of D if C does not prereq D.  Back in build_graph(),
                     * we linked the base level of CUP1 to the requisite node
                     * joining CUP1 to D prior to the graph being inverted.
                     */
                    if (req_node->base_lev_list
                                && 
                        ! is_B_req_of_A (subgraph_root, 
                                         req_node->base_lev_list->fix_ptr))
                    {
                       check_prereq.deinst_dependencies_via_updates = TRUE;
                       failure_detected = TRUE;
                    }
                 }
                 break;

              } /* end switch */
              break;  /* case OP_REJECT */
     } /* end switch */

     if (failure_detected)
     {
        current_fix->flags |= REPT_FAILED_REQUISITE;
        current_fix->failure_sym = determine_failure_reason (current_fix,  
                                                             req_node,
                                                             FALSE);
        add_subgraph_root_to_dependents_list (subgraph_root, req_node, error);
        RETURN_ON_ERROR;
     }

     /* 
      * Keep recursing if we didn't decide not to above.
      */
     if (! ignore_subgraph)
     {
        for (requisite = current_fix -> requisites;
             requisite != NIL (requisite_list_type);
             requisite = requisite -> next_requisite)
        {
           evaluate_requisite_failure_subgraph (subgraph_root, requisite, 
                                               error);
           RETURN_ON_ERROR;
        } /* end for */
     } /* end if */
  } /* end if */

  current_fix -> flags &= ~VISITING_THIS_SUBGRAPH; /* Cycle detection */
  current_fix -> flags |= SUBGRAPH_VISITED;

} /* evaluate_requisite_failure_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: remove_failures_and_successes_from_warning_list
**
** FUNCTION:  Removes any REPT_REQUISITE_FAILURES or REPT_CMD_LINE_FAILURES
**            from the miscellaneous failures, warnings list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
remove_failures_and_successes_from_warning_list
(
)
{
   index_list_type  * index;
   index_list_type  * prev_index;
   fix_info_type    * fix_ptr;

   prev_index = report_anchor.misc_warnings_hdr;

   /*
    * Loop through the warnings list.  If an entry has been marked as a
    * FAILURE, it will be printed as such.  Let's not print it as a warning
    * also.  Also, force installs can often result in a warning flagged 
    * earlier which is going to be 
    */
   for (index = report_anchor.misc_warnings_hdr->next_index_node;
        index != report_anchor.misc_warnings_tail;
        index = index->next_index_node)
   {
      fix_ptr = index->fix_ptr;
      if ((fix_ptr->flags & REPT_CMD_LINE_FAILURE)                         ||
          (fix_ptr->flags & REPT_REQUISITE_FAILURE)                        ||
          (fix_ptr->failure_sym != '\0')                                   ||
          ((check_prereq.Force_Install)       && 
           (fix_ptr->flags & SUCCESSFUL_NODE) &&
           (fix_ptr->origin == COMMAND_LINE))) 
      {
          prev_index->next_index_node = index->next_index_node; 
                                                          /* point around it */
          index->next_index_node = NIL (index_list_type); /* clean up */ 
          index->fix_ptr = NIL (fix_info_type); 
          my_free (index);                                /* zap! */
          index = prev_index;                             /* re-adjust ptr */
      }
   }  /* for */

} /* remove_failures_and_successes_from_warning_list */

/*--------------------------------------------------------------------------*
**
** NAME: is_instreq_failure_subgraph 
**
** FUNCTION:  Determines if the only failure within cmd_line_pkg's subgraph
**            is an "instreq failure" ie: a pkg which is instreqed by another 
**            is neither installed nor going to be installed as an explicitly 
**            requested pkg.  (-g can't pull in instreqs automatically.)
**            Note: we also consider a "missing instreq" as an instreq failure. 
**
** RETURNS:   TRUE if cmd_line_pkg is only being held up by an un-installed
**            instreq, FALSE otherwise.
**
**--------------------------------------------------------------------------*/

static boolean
is_instreq_failure_subgraph 
(
fix_info_type * cmd_line_pkg
)
{
   boolean               instreq_failure_in_sg = FALSE;
   boolean               other_failure_in_sg = FALSE;

   /*
    * Look at the requisites for cmd_line_pkg.  If any one is an instreq 
    * failure, which wasn't requested at the installp cmd_line, and no other's 
    * are non-instreq failures, then we return TRUE.  Otherwise return FALSE.
    * Examples:
    *   Scenario: trying to install A.  B and C are not installed:
    *   - A instreqs B, B is not explicitly requested, return TRUE
    *   - A instreqs B, B prereqs C.  B and C are not explicitly requested,
    *                                                            return TRUE
    *   - A prereqs B, B instreqs C.  B and C are not explicitly requested,
    *                                                            return TRUE
    *   - A prereqs B, A instreqs C.  B and C are not explicitly requested,
    *                                                            return FALSE
    */

   /*
    * Call a recursive back-end routine to figure out if it's just an instreq
    * failure in the current subgraph.
    */
   (void) is_instreq_failure_subgraph_be (cmd_line_pkg, 
                                          &instreq_failure_in_sg,
                                          &other_failure_in_sg);
   /*
    * Reset recursion tag-bit.
    */
   unmark_SUBGRAPH (cmd_line_pkg, VISITING_THIS_NODE);

   if (instreq_failure_in_sg && !other_failure_in_sg)
      return (TRUE);
   else
      return (FALSE);

} /* is_instreq_failure_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: is_instreq_failure_subgraph_be 
**
** FUNCTION:  Recursive, back-end routine to traverse the entire subgraph
**            of the node passed as an argument.
**
** RETURNS:   FALSE if a non-instreq failure was detected.
**            TRUE, otherwise.
**            Flags are set to indicate the types of failure detected in the
**            subgraph and are used by the top-most caller.
**
**--------------------------------------------------------------------------*/

static boolean
is_instreq_failure_subgraph_be
(
fix_info_type * parent,
boolean       * instreq_failure,
boolean       * other_failure
)
{
   char                  failure_sym;
   fix_info_type       * child;
   requisite_list_type * req_node;

   /*
    * Check for cycles and set tag bit accordingly.
    */
   if (parent->flags & VISITING_THIS_NODE)
      return (TRUE);
   parent->flags |= VISITING_THIS_NODE;

   /*
    * Loop through the requisite chain of the node passed as an argument.
    * Inspect their immediate children for instreq failures then recurse 
    * to look at their subgraphs.
    */ 
   for (req_node = parent->requisites;
        req_node != NIL (requisite_list_type);
        req_node = req_node->next_requisite)
   {
      child = req_node->fix_ptr;

      /*
       * Only consider unsuccessful requisites.  (Even though this is
       * an unsuccessful subgraph, some requisites may be members of 
       * other successful subgraphs.)
       */
      if (! (child->flags & SUCCESSFUL_NODE))
      {
         failure_sym = determine_failure_reason (child, req_node, FALSE);

         if (req_node->type == AN_INSTREQ)                   
         {
               /*
                * Not concerned with instreq'd pkgs requested on cmd line.  
                * These can never be the source of an instreq failure.
                */
            if ((child->origin != COMMAND_LINE)                      
                                        &&
                /*
                 * Pkgs which are already applied may not be marked as 
                 * a SUCCESSFUL_NODE but they still can't be classified
                 * as having caused an instreq failure.
                 */
                (child->apply_state != ALL_PARTS_APPLIED) 
                                        &&
                /*
                 * A final check: instreq failures are created by:  
                 * available instreqs, missing instreqs, or instreqs to
                 * conflicting base levels.
                 */ 
                ((failure_sym == INSTREQ_SYM) || 
                 (failure_sym == UNAVLBL_SYM) ||
                 (failure_sym == CONFL_LEVEL_SYM)))
            {
               *instreq_failure = TRUE;   /* used by top-most caller */
               return (TRUE);
            } /* if */
         }
         else
         { 
            /*
             * Not an instreq.  Must be a regular requisite failure if:
             *  1. not a "dummy ifreq subgraph."    ie. an ifreq for which
             *     the base is not installed or being installed.
             *                 AND
             *  2. not a "on the media" requisite which would have been 
             *     pulled in with -g had other failures not occurred 
             *     (UNAVLBL_REQ_OF_REQ)           
             *                 AND
             *  3. not a "on the media" requisite which could be pulled in
             *     if -g were specified.
             * If so, return FALSE to indicate that there are non-instreq 
             * failures in this subgraph.
             */
            if ((! ignore_ifreq_subgraph (FALSE, req_node, FALSE)) &&
                ((failure_sym != UNAVLBL_REQ_OF_REQ_SYM)    &&
                 ((failure_sym != AVLBL_SYM) && !check_prereq.Auto_Include)))
            {

               /*
                * We've found a failure other than an instreq failure.
                */
               *other_failure = TRUE;  /* used by top-most caller */
               return (FALSE);         /* short-circuit the recursion */
            }

         } /* if */

         /*
          * Recurse on this requisite.  If we get a failure back, return
          * from here since there's no point in going on -- we found a 
          * non-instreq failure.
          */       
         if (! is_instreq_failure_subgraph_be (child, instreq_failure,
                                               other_failure))
            return (FALSE);

      } /* if */
   } /* for */

   /*
    * We didn't return false above.  We must still be on track for an
    * instreq failure.  return TRUE.
    */
   return (TRUE);

} /* is_instreq_failure_subgraph_be */

/*--------------------------------------------------------------------------*
**
** NAME: mark_successful_subgraph
**
** FUNCTION:  sets MEMBER_OF_SUCCESS_SUBGRAPH bit in each successful member 
**            of node's subgraph. (assumed to be a successful subgraph.) This 
**            will as a very useful sanity check in subsequent processing to 
**            remove unreferenced nodes  (see delete_unreferenced_nodes() in 
**            list_utils.c).  Recursion is used to traverse the graph.
**            Dummy ifreq branches and unselected member of group requisites
**            are ignored.
**
** RETURNS:   void function.
**
**--------------------------------------------------------------------------*/

static void
mark_successful_subgraph
(
fix_info_type       * node,
requisite_list_type * req_node
)
{
    requisite_list_type * req;

    /*
     * Return if:
     *   -- we've been here 
     *   -- we don't want to consider an ifreq subgraph
     */
    if ((req_node != NIL (requisite_list_type)) &&
        (check_for_cycle_or_ifreq_subgraph (node, req_node, FALSE)))
       return; 
 
    node->flags |= VISITING_THIS_SUBGRAPH;    /* Cycle detection. */
    node->flags |= MEMBER_OF_SUCCESS_SUBGRAPH;

    for (req = node->requisites;
         req != NIL (requisite_list_type);
         req = req->next_requisite)
    {
        /*
         * Don't traverse the subgraph of unselected group members.
         */
         if ((node->origin != DUMMY_GROUP) ||
            (req->flags.selected_member_of_group))
         {
           mark_successful_subgraph (req->fix_ptr, req);
         }
    } /* for */

    node->flags &= ~VISITING_THIS_SUBGRAPH;
    node -> flags |= SUBGRAPH_VISITED;

} /* mark_successful_subgraph */
