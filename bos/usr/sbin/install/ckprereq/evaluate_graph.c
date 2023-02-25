static char sccsid[] = "@(#)59  1.18  src/bos/usr/sbin/install/ckprereq/evaluate_graph.c, cmdinstl, bos41J, 9517A_all 4/20/95 19:16:09";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            add_auto_include_nodes_to_graph
 *            add_auto_include_nodes_to_subgraph 
 *            is_conflicting_impl_requested_base
 *            consider_ifreqs
 *            consider_requisite_subgraph
 *            consider_unresolved_ifreqs 
 *            evaluate_graph 
 *            evaluate_subgraph
 *            inherit_subgraph_failures
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

/*########################## LOCAL MACROS ####################################*/
/*
 * Marks a node with FAILED_NODE tag bit.  If in particular phase of 
 * evaluating requisites (NOT "considering_unresolved_ifreqs"), also 
 * unmarks SUCCESSFUL_NODE bit and sets a flag indicating decision 
 * reversal.
 */ 
#define MARK_NODE_AS_FAILED(node)                                            \
                    {                                                        \
                     node->flags |= FAILED_NODE;                             \
                     if ((node->flags & SUCCESSFUL_NODE) &&                  \
                             ! check_prereq.consider_unresolved_ifreqs)      \
                     {check_prereq.decision_reversal_occurred = TRUE;        \
                        node->flags &= ~SUCCESSFUL_NODE;}                    \
                    }
/* 
 * Evaluates to TRUE or FALSE depending on whether or not node is applied,
 * has ifreqs, and is NOT in the graph.  We ignore DUMMY_GROUP nodes.
 */
#define IS_APPLIED_NODE_WITH_IFREQS(node)                                    \
                     ((node->apply_state == ALL_PARTS_APPLIED) && \
                      (node->flags & FIX_HAS_IFREQS) &&                      \
                      (node->origin != DUMMY_GROUP)  &&                      \
                      ! (node->flags & SUCCESSFUL_NODE))
/*
 * Evaluates to TRUE or FALSE depending upon whether or not:
 *          1. Node is an ifreq of parent (parent points to node via
 *             requisite).
 *          2. Base level of node is applied or is going to be applied.
 *          3. Node was not requested for installation (it is not an unresolved 
 *             ifreq if so.)
 *          4. Node has not already been selected for installation (implicitly
 *             in this case).
 */
#define IS_UNRESOLVED_IFREQ(requisite)                                       \
              ((requisite->type == AN_IFREQ || requisite->type == AN_IFFREQ) \
                                        &&                                   \
               (ck_base_lev_list (requisite, CBLL_UNRES_IFREQ, FALSE))       \
                                        &&                                   \
               (requisite->fix_ptr->origin != COMMAND_LINE)                  \
                                        &&                                   \
               (requisite->fix_ptr->apply_state != ALL_PARTS_APPLIED)        \
                                        &&                                   \
               ! (requisite->fix_ptr->flags & SUCCESSFUL_NODE))

/*########################## END OF MACROS ##################################*/

/*
 * Prototypes:
 */

static void add_auto_include_nodes_to_subgraph (
requisite_list_type * requisite,
fix_info_type       * fix,
boolean             * error);

static boolean is_conflicting_impl_requested_base (
fix_info_type * fix,
boolean       * error);

boolean consider_requisite_subgraph (
requisite_list_type * requisite);

static boolean inherit_subgraph_failures (
fix_info_type * fix);

static int oem_system = -1;    /* -1 is unknown, 0 is non-OEM, 1 is OEM */
static char oem_system_name[MAX_LANG_LEN+1]; /* Name of OEM system */

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_graph
**
** FUNCTION:  Traverses the graph to see if every requisite is marked
**            CANDIDATE_NODE or SUCCESSFUL_NODE.
**
** RETURNS:   Returns the result.
**
**--------------------------------------------------------------------------*/

boolean 
evaluate_graph 
(
boolean * error
)
{
   boolean               subgraph_passed;
   boolean               re_traverse_graph;
   boolean               result = TRUE;
   requisite_list_type * requisite;
   fix_info_type       * fix;

   /*
    * Global flag used to indicate whether or not a node went from  
    * SUCCESSFUL_NODE to FAILED_NODE during graph evaluation.
    */
   check_prereq.decision_reversal_occurred = FALSE;

   /*
    * Loop through the requisites of the anchor, the explicitly requested
    * packages, and evaluate their subgraphs.  Set a flag in the requisite 
    * structure indicating whether or not there was a failure in the 
    * subgraph.
    */
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
        requisite->flags.selected_member_of_group =
                               evaluate_subgraph (requisite, 
                                                  requisite->fix_ptr, error);
        RETURN_VALUE_ON_ERROR (FALSE);
        unmark_graph (VISITING_THIS_NODE);

   } /* end for */

   /*
    * If a node went from SUCCESSFUL to FAILED, re-traverse the graph and
    * make sure that the FAILED_NODE flag is inherited all the way up to the 
    * node which was explicitly requested.
    * NOTE: This pass is necessary AFTER we start considering ifreqs because
    *       graph traversal in the first pass may be in a different
    *       order when we are considering ifreqs than when we are not 
    *       considering ifreqs.  As a result some nodes in a subgraph that
    *       were marked successful before considering ifreqs, may need to
    *       be marked failed after we start considering ifreqs.  The 
    *       reason they did not get marked in the pass above is due to our
    *       optimization where we exit the subgraph as soon as failure is 
    *       detected.
    *       Ex.
    *           A -- p --> B -- p --> C -- if --> D
    *           |                     ^
    *           |                     |
    *           +-- if --> E -- p ----+ 
    *      Pass 1: order of traversal: A, B, C -- all marked successful
    *      Pass 2: consider ifreqs, order of traversal: A, E, C, D
    *              D fails, .: C, E, A fail,  B is left marked successful.
    */
   if (check_prereq.decision_reversal_occurred)
   {   
      re_traverse_graph = TRUE;
      check_prereq.decision_reversal_occurred = FALSE;

      /*
       * Loop until no more changes occur (hopefully never more than twice).
       */
      while (re_traverse_graph)
      {
         for (requisite = check_prereq.fix_info_anchor->requisites;
              requisite != NIL (requisite_list_type);
              requisite = requisite->next_requisite)
         {
            /*
             * Traverse the subgraph, resetting the success flag in the
             * requisite node if appropriate (inherit_.. returns TRUE if 
             * there were failures).
             */
            requisite->flags.selected_member_of_group =
                    ! inherit_subgraph_failures (requisite->fix_ptr);
         }
         re_traverse_graph = check_prereq.decision_reversal_occurred;
         check_prereq.decision_reversal_occurred = FALSE;
         unmark_graph (VISITING_THIS_NODE);

      } /* end while */
   } /* end if */

   /*
    * Now we mark the subgraph of all successful nodes linked as requisites
    * to the anchor, according to the flag in the requisite structure.
    */
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      if (requisite->flags.selected_member_of_group)
      {
         requisite->flags.selected_member_of_group = FALSE;
         tag_subgraph_SUCCESSFUL_NODEs (requisite, requisite->fix_ptr, error);
         RETURN_VALUE_ON_ERROR (FALSE);
      }
      else
      {
         result = FALSE;
      }
   } /* end for */

   /*
    * Reset the VISITING tag in addition to the FAILED tag since a node may
    * have failed this pass but may be successful in subsequent passes.
    */
   unmark_graph (VISITING_THIS_NODE | FAILED_NODE);
   return (result);

} /* end evaluate_graph */

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_subgraph
**
** FUNCTION:  Traverses the subgraph to see if every requisite is marked
**            SUCCESSFUL_NODE or CANDIDATE_NODE.
**
** RETURNS:   Returns the result.
**
**--------------------------------------------------------------------------*/

boolean 
evaluate_subgraph 
(
requisite_list_type * link_from_parent,
fix_info_type       * fix,
boolean             * error
)
{
   int                   number_of_passes = 0;
   boolean               passed = TRUE;
   boolean               forced_pass;
   requisite_list_type * requisite;
   boolean               result = TRUE;


   /***********************************************************
    ** SPECIAL HANDLING FOR GROUP NODES ON REJECT OPERATION. **
    ***********************************************************/
   if (fix->origin == DUMMY_GROUP && check_prereq.mode == OP_REJECT)
   {
      /*
       * For reject, our graph is built as normal, then inverted.
       * This means that we have several links "entering" our group node
       * and only one "exitting."  Our passes required on the group node
       * is the number of requisites that must REMAIN installed.  If we 
       * got to this group node, our parent, the group's child in reality,
       * is thus far ok to reject.  Here, we decide if rejecting our parent 
       * leaves the group node's count fulfilled, or if we should continue 
       * to evaluate along the group's subgraph (our single exit point, or 
       * the link to the installed package that actually contains the group 
       * requisite).  This is true when the number of rejectable_requisites 
       * of the group node, (determined when the graph is inverted) less the 
       * number of requisites we already decided to reject is less than or
       * equal to the passes required.
       *
       * The selected_member_of_group is used here to flag group members,
       * (REMEMBER: this is really the group parent with respect to the 
       *  graph structure) that are ok to reject without rejecting the 
       * package which has the group requisite (REMEMBER: the child of the
       * group with respect to graph structure).
       */
      if ((fix->num_rejectable_requisites - 
          (fix->num_rejected_requisites) > fix->passes_required) ||
          (link_from_parent->flags.selected_member_of_group))
      {
         /* 
          * It's still ok to reject this group member without rejecting 
          * the parent of the group (the pkg that has the group requisite).
          */
         if (! link_from_parent->flags.selected_member_of_group)
         {
            /* 
             * This is the first time we've considered this group member
             * so set the flag indicating otherwise, and bump up the 
             * number that we've said we'd reject.
             */
            link_from_parent->flags.selected_member_of_group = TRUE;
            (fix->num_rejected_requisites)++;
         }
            
         return (TRUE);
      }
      else
      {
         /*
          * We must evaluate the subgraph of the group since we have 
          * exceeded the number of group requisites that can be rejected.
          */
         passed = evaluate_subgraph (fix->requisites, 
                                     fix->requisites->fix_ptr, error);
         RETURN_VALUE_ON_ERROR (FALSE);

         /*
          * Set the flag indicating that this parent requires the group to
          * be rejected also.
          */
         link_from_parent->flags.selected_member_of_group = FALSE;

         /*
          * Return the result of the subgraph's evaluation to the parent.
          */
         if (passed)
         { 
            return (TRUE);
         }
         else
         {
            MARK_NODE_AS_FAILED (fix);
            return (FALSE);
         }
      }
   } /* end if (dummy group & reject) */
   /**********************************************
    ** END OF SPECIAL HANDLING FOR GROUP NODES. **
    **********************************************/

   /*
    * Check to see if we've been here previously during evaluation of the
    * current subgraph.  Return a result based on whether or not this node
    * is known to have failed.
    */
   if (fix->flags & VISITING_THIS_NODE)
   {
      if (fix->flags & FAILED_NODE)
         return (FALSE);
       else
         return (TRUE);
   }

   fix->flags |= VISITING_THIS_NODE;

   /*
    * Exit early if this is known to have failed OR if it should not be 
    * considered during this evaluation (if it's not in the graph or a 
    * candidate)
    */
   if ((fix->flags & FAILED_NODE)
                         ||
        (! (fix->flags & (CANDIDATE_NODE | SUCCESSFUL_NODE) ) ) )
   {
      MARK_NODE_AS_FAILED (fix);
      return (FALSE);
   }

   /*
    * Loop through the requisites of this node.  Decide if we should evaluate 
    * the subgraph, based on the type of requisite, then act accordingly.
    * If any requisite failed evaluation, except in the case of group 
    * requisites, return immediately.
    */
   for (requisite = fix->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
       forced_pass = FALSE;
       if (consider_requisite_subgraph (requisite))
       {
          passed = evaluate_subgraph (requisite, requisite->fix_ptr, error);
          RETURN_VALUE_ON_ERROR (FALSE);
       }
       else
       {
          /*
           * If we don't need to evaluate the subgraph, then it will not
           * prevent us from passing.
           */
          passed = TRUE;
          if (requisite->type == AN_IFREQ || requisite->type == AN_IFFREQ)
             forced_pass = TRUE;
       }

       if (fix->origin == DUMMY_GROUP)
       {
          /* 
           * Increment our "satisfied group counts" for dummy groups, and
           * return if we've met the number required.
           */
          if (passed)
          {
             if (! forced_pass)
                requisite->flags.selected_member_of_group = TRUE;
             number_of_passes++;
             if (number_of_passes >= fix->passes_required)
                return (TRUE);
          }
          else
          {
             requisite->flags.selected_member_of_group = FALSE;
          }
       }
       else
       {
          if (! passed)        /* Return if any requisite failed. */
          {
              MARK_NODE_AS_FAILED (fix);
              return (FALSE);
          }
       }
   } /* end for */

   if (fix->origin != DUMMY_GROUP)
   {
      return (passed);
   }
   else
   {                               
      /*  
       * If we got here, and we're a dummy group, return failure since we
       * must not have satisfied our count.
       */ 
      MARK_NODE_AS_FAILED (fix);
      return (FALSE);
   }

} /* end evaluate_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: add_auto_include_nodes_to_graph
**
** FUNCTION:  Traverses the graph, marking as many nodes as possible as
**            CANDIDATE_NODEs.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
add_auto_include_nodes_to_graph 
(
boolean * error
)
{
   fix_info_type        * fix;
   requisite_list_type  * requisite;
 
   /*
    * The goal is to traverse the "graph" marking any nodes that may be 
    * applied/committed/rejected with -g as CANDIDATEs.  In the case of
    * trying to resolve unmet ifreqs of packages that are already installed,
    * we actually begin traversal from the fix_info table, rather than from
    * the requisite chain of the anchor (since the packages with the 
    * unresolved ifreqs are not necessarily linked to the anchor).
    */
   if (check_prereq.consider_unresolved_ifreqs)
   {
      for (fix = check_prereq.fix_info_anchor->next_fix;
           fix != NIL (fix_info_type);
           fix = fix->next_fix)
      {
          if (IS_APPLIED_NODE_WITH_IFREQS (fix))
          {
             for (requisite = fix->requisites;
                  requisite != NIL (requisite_list_type);
                  requisite = requisite->next_requisite)
             { 
                 if (IS_UNRESOLVED_IFREQ (requisite))
                 {
                    add_auto_include_nodes_to_subgraph (requisite, 
                                                    requisite->fix_ptr, error);
                    RETURN_ON_ERROR;

                 } /* end if */
             } /* end for */
          } /* end if */
      } /* end for */
      unmark_graph (VISITING_THIS_NODE);
   }
   else
   { 
      for (requisite = check_prereq.fix_info_anchor->requisites;
           requisite != NIL (requisite_list_type);
           requisite = requisite->next_requisite)
      {
          if (check_prereq.consider_ifreqs ||
              ! (requisite->fix_ptr->flags & SUCCESSFUL_NODE) )
          {
             add_auto_include_nodes_to_subgraph (requisite, requisite->fix_ptr, 
                                                 error);
             RETURN_ON_ERROR;
             /*
              * Unmarking is done for the subgraph just processed now, rather
              * than after the entire anchor list is processed, so that we
              * don't have a problem with the order in which we visit nodes
              * for "instreq processing."  ie.  we don't want to mark 
              * instreq nodes as "auto-includable" but we don't want to 
              * NOT mark them if they're prereqed by other nodes.  This may
              * happen if we mark the node as VISITING when we see it via
              * an instreq pass first.
              */
             unmark_SUBGRAPH (requisite->fix_ptr, VISITING_THIS_NODE);
          }
      } /* end for */
   }

} /* end add_auto_include_nodes_to_graph */

/*--------------------------------------------------------------------------*
**
** NAME: add_auto_include_nodes_to_subgraph
**
** FUNCTION:  Traverses the subgraph, marking as many nodes as possible with
**            CANDIDATE_NODE.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
add_auto_include_nodes_to_subgraph 
(
requisite_list_type * link_from_parent,
fix_info_type       * fix,
boolean             * error
)
{
   boolean               mark_fix;
   boolean               result = TRUE;
   char                  vpd_tree;
   int                   number_of_passes = 0;
   requisite_list_type * requisite;
   FILE		       * fp;

   /*
    * Here, we perform more special handling of DUMMY_GROUP nodes on 
    * a reject operation.  We only traverse the subgraph of the group node
    * (REMEMBER: the package with the group requisites), when the 
    * selected_member_of_group node from the group's parent (REMEMBER: one
    * of the group's members) has not been set.  
    */
   if (fix->origin == DUMMY_GROUP && check_prereq.mode == OP_REJECT)
   {
      if (! (link_from_parent -> flags.selected_member_of_group))
      {
         add_auto_include_nodes_to_subgraph (fix->requisites, 
                                             fix->requisites->fix_ptr, error);
      }
      return;
   } /* end if (dummy group & reject) */

   if (fix->flags & VISITING_THIS_NODE) /* Cycle detection */
      return;

   fix->flags |= VISITING_THIS_NODE;

   /*
    * Make a preliminary decision on whether or not to mark the fix (simply
    * based upon whether the state will allow it to be pulled in as a 
    * requisite).
    */
   mark_fix = FALSE;
   switch (check_prereq.mode)
   {
      case OP_APPLY :
        if (IS_AUTO_APPLIABLE (fix))
        {
           /*
            * We dont pull in instreqs automatically.
            * (Note.  There are no special instreq considerations for
            * non-apply ops.  Rather, instreqs are treated like prereqs.)
            */
           if (link_from_parent->type != AN_INSTREQ) {
              mark_fix = TRUE;

	      /* If we don't know if this is an OEM-specific system or not,
	       * find out now.  We need to know this because you cannot apply
	       * an OEM-specific fileset or fileset update on a system which
	       * is not specific to that OEM.
	       */
	      if (oem_system == -1) {
   		 if ((fp = fopen("/usr/lib/objrepos/oem","r")) != NIL (FILE)) {
       		     oem_system = 1;
       		     fgets((char *) &oem_system_name, MAX_LANG_LEN+1, fp);
       		     /* Separate out the first word and remove white space. */
       		     strtok((char *) &oem_system_name," \n");
   		 } else {
       		     oem_system = 0;
		 }
	      }

	      /* Filesets on the media must be applicable to the
	       * system manufacturer.
	       */
	      if (fix -> TOC_Ptr != NIL (Option_t)) {
		 if ((oem_system) || ((fix->TOC_Ptr->lang[0] == 'o') &&
				     (!strncmp(fix->TOC_Ptr->lang,"oem_",4)))) {
		   /* Make sure that we look for the data in the right VPD */
                   if (fix->parts_in_fix == LPP_SHARE) {
		       vpdremotepath (SHARE_VPD_PATH);
		       vpdremote ();
		   } else {
		       vpdremotepath (USR_VPD_PATH);
		       vpdremote ();
		   }
		   if (inu_oem_chk (fix->TOC_Ptr, oem_system_name) != SUCCESS) {
		     /* Set reporting bit. */
		     if (fix->TOC_Ptr->Status == STAT_OEM_MISMATCH) {
                         fix->flags |= FAILED_OEM_MISMATCH;
		     } else {
                         fix->flags |= FAILED_OEM_SPECIFIC;
		     }
                     mark_fix = FALSE;
		   }
		 }
	      }
	   }

        }
        break;

      case OP_COMMIT :
        if (IS_AUTO_COMMITTABLE (fix))
        {
           mark_fix = TRUE;
        }
        break;

      case OP_REJECT :
        if (IS_AUTO_REJECTABLE (fix))
        {
           if (MIGRATING (fix->cp_flag))
              break;   /* Don't mark this one, can never remove a migrating
                          fileset (update or base level). */
           /*
            * Need to perform a further check to see if the pkg is
            * deinstallable.  Call the libraray routine inu_deinst_chk ().
            */
           if (fix->flags & FAILED_DEINST_CHECK)
              mark_fix = FALSE;
           else
           {
              if (check_prereq.deinstall_submode && IF_INSTALL (fix->op_type))
              {
                 if (fix->parts_in_fix == LPP_SHARE)
                    vpd_tree = VPDTREE_SHARE;
                 else
                    vpd_tree = VPDTREE_USR;
   
                 if (inu_deinst_chk (fix->product_name, fix->name, vpd_tree,
                                     TRUE) == 0)
                 {
                    mark_fix = TRUE;
                 }
                 else
                 {
                    fix->flags |= FAILED_DEINST_CHECK;  /* Set reporting bit */
                    mark_fix = FALSE;
                 }
              }
              else
              {
                 mark_fix = TRUE;
              }
           }
        }
        break;
   } /* end switch */

   if (mark_fix)
   {
      /* 
       * Proceed to mark the node IF WE CAN: based on the "Updates Only" (B)
       * flag and whether or not there are conflicts regarding multiple base
       * levels.  (NOTE: We don't care about the "B" check if we're doing
       * a commit.  ie. -g wins and base levels are always committed anyway).
       */
      if ((check_prereq.mode != OP_COMMIT)  &&
           (check_prereq.Updates_Only_Install) &&
           (IF_INSTALL (fix->op_type)) )
      {
         MARK_NODE_AS_FAILED (fix);
      }
      else
      {
         if (check_prereq.mode == OP_APPLY &&
             is_conflicting_impl_requested_base (fix, error) )
         {
            RETURN_ON_ERROR;

            MARK_NODE_AS_FAILED (fix);
         }
         else
         {
            RETURN_ON_ERROR;
            fix->flags |= CANDIDATE_NODE;
         }
      }
   }

   /*
    * Get out if this node has not been marked at all (not a candidate nor a
    * success).
    */
   if ( ! (fix->flags & (CANDIDATE_NODE | SUCCESSFUL_NODE) ) )
      return;

   /*
    * Now look at the node's requisites if applicable.
    */
   for (requisite = fix->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
       if (consider_requisite_subgraph (requisite))
       {
          add_auto_include_nodes_to_subgraph (requisite, 
                                              requisite->fix_ptr, error);
          RETURN_ON_ERROR;
       }
   } /* end for */

} /* end add_auto_include_nodes_to_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: consider_ifreqs
**
** FUNCTION:  Revisit the graph, this time taking ifreqs into consideration.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
consider_ifreqs 
(
boolean * error
)
{
   /*
    * Set the flag that we've been using to ignore ifreqs thus far.
    */
   check_prereq.consider_ifreqs = TRUE;          

   if (check_prereq.Auto_Include)
   {
     /*
      * Mark any ifreqs that can be pulled in with -g.
      */
      add_auto_include_nodes_to_graph (error);
      RETURN_ON_ERROR;
   }
   /*
    * Re-traverse our graph to see if we can pull in our requisites or 
    * if we need to fail some nodes that we had already passed.
    */
   (void) evaluate_graph (error);
   RETURN_ON_ERROR;

} /* end consider_ifreqs */

/*--------------------------------------------------------------------------*
**
** NAME: consider_unresolved_ifreqs
**
** FUNCTION:  Revisit the graph, this time taking unresolved ifreqs into 
**            consideration.  (That is, ifreqs from updates that are already
**            installed.).  We only pull in updates if -g is set.  If -g is 
**            not set, we still perform this operation so that we can issue 
**            warning messages about the need to apply these requisites.  
**            We'll add the unresolved ifreq to the anchor node's requisite
**            list and treat it as though it was requested from the command
**            line for all subsequent processing (failure reporting will be 
**            treated slightly differently).
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
consider_unresolved_ifreqs 
(
boolean * error
)
{
   boolean               ifreqs_passed = FALSE;
   boolean               processed_ifreqs = FALSE;
   criteria_type         ifreq_criteria;
   fix_info_type       * fix;
   requisite_list_type * requisite;
   requisite_list_type * requisite2;

   /*
    * No need to do anything if updating installp fileset.
    */
   if (check_prereq.installp_update)
      return;

   /*
    * Set another global flag which is only used to tell us NOT to undo
    * any SUCCESSES that we may have previously set, should we encounter
    * failures in trying to pull in unresolved ifreqs.
    */
   check_prereq.consider_unresolved_ifreqs = TRUE;
   if (check_prereq.Auto_Include)
   {
      /*
       * Mark our candidates that may be pulled in.
       */
      add_auto_include_nodes_to_graph (error);
      RETURN_ON_ERROR;
   }

   for (fix = check_prereq.fix_info_anchor->next_fix;
        fix != NIL (fix_info_type);
        fix = fix->next_fix)
   {
       if (IS_APPLIED_NODE_WITH_IFREQS (fix))
       {
          for (requisite = fix->requisites;
               requisite != NIL (requisite_list_type);
               requisite = requisite->next_requisite)
          { 
              if (IS_UNRESOLVED_IFREQ (requisite))
              {
                 /*
                  * Mark the node for error reporting.
                  */
                 requisite->fix_ptr->flags |= UNRESOLVED_IFREQ; 
                 processed_ifreqs = TRUE;
                 /*
                  * Make the ifreq look like it was explicitly requested.
                  */ 
                 requisite->fix_ptr->apply_state = TO_BE_EXPLICITLY_APPLIED;

                 /*
                  * Copy the criteria for error reporting.
                  */
                 dupe_relations (&requisite->criteria, & ifreq_criteria,
                                 error);
                 RETURN_ON_ERROR;

                 requisite2 = link_requisite (check_prereq.fix_info_anchor, 
                                 requisite->fix_ptr,
                                 & ifreq_criteria, A_PREREQ, error);
                 RETURN_ON_ERROR;

                 /*
                  * Evaluate the unresolved ifreq's subgraph, flagging the
                  * result in the requisite structure (as usual).
                  */
                 if (requisite2->flags.selected_member_of_group =
                            evaluate_subgraph (NIL (requisite_list_type), 
                                               requisite->fix_ptr, error))
                 {
                    ifreqs_passed = TRUE;
                 }
                 else
                 {
                    /*
                     * The subgraph failed, mark the node's state 
                     * appropriately.
                     */
                    requisite->fix_ptr->apply_state = CANNOT_APPLY; 
                 } 
                 RETURN_ON_ERROR;
                 unmark_graph (VISITING_THIS_NODE);
              }
          } /* end for */
       } /* end if */
   } /* end for */

   /*
    * Use the flag set above to determine if we need to tag successful 
    * subgraph's (like evaluate_graph () does).
    */
   if (ifreqs_passed)
   {
       for (requisite = check_prereq.fix_info_anchor->requisites;
           requisite != NIL (requisite_list_type);
           requisite = requisite->next_requisite)
       {
           if (requisite->flags.selected_member_of_group)
           {
              requisite->flags.selected_member_of_group = FALSE;
              tag_subgraph_SUCCESSFUL_NODEs (requisite, requisite->fix_ptr, 
                                             error);
              RETURN_ON_ERROR;
           }
       } /* end for */
   }
 
   /*
    * Only unmark the graph if we did any unresolved ifreq processing.
    */
   if (processed_ifreqs)
      unmark_graph (VISITING_THIS_NODE | FAILED_NODE);


} /* end consider_unresolved_ifreqs */

/*--------------------------------------------------------------------------*
**
** NAME: is_conflicting_impl_requested_base
**
** FUNCTION:  Verify that we are not trying to install more than one level
**            of a product.
**
**            If there is more than one version of this product, return
**            whether or not any of the other versions are already tagged to
**            be put into the graph.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static boolean 
is_conflicting_impl_requested_base 
(
fix_info_type * fix,
boolean       * error
)
{
   fix_info_type * current;

   if (! IF_INSTALL (fix->op_type)) 
      return (FALSE);

   if (fix->apply_state == CANNOT_APPLY_CONFL_LEVEL) 
      /* Already marked it here or in evaluate_base_levels() */
      return (TRUE);

   /* 
    * This is not a conflicting base level, but we need to "disable" remaining
    * base levels for this option which may be implicitly requested.
    * There's only a need to look if this pkg is marked with the CONFLICT 
    * bit set in copy_TOC_to_fix_info() (meaning installp detected multiple 
    * versions of this on the toc).  We want to do anything if auto-supersedes 
    * is NOT on, since we're going to list all implicitly requested base levels
    * and warn that the latest will be picked when -g is on.
    */ 

   if ((! check_prereq.Auto_Supersedes) ||
       ! (fix->flags & CONFL_TOC_BASE_LEVEL))
      return (FALSE);

   /* 
    * Find beginning of this option in fix_info table.
    */
   current = fix->prev_fix;
   while (strcmp (current->name, fix->name) == 0)
         current = current->prev_fix;
  
   /*
    * Loop through fix_info table on all pkgs in this option, looking for
    * base levels.
    */ 
   for (current = current->next_fix;
        (current != NIL (fix_info_type)) && 
         (strcmp (current->name, fix->name)==0);
        current = current->next_fix)
   {
      if ((fix != current)                 && 
          (IF_INSTALL (current->op_type))  &&
          (fix->apply_state == IN_TOC))
      {
         /*
          * NOTE:
          * It doesn't do any good to mark older compatible pkgs as
          * SUPERSEDED since the graph is already built and anyone pointing
          * to the "eliminated base level" cannot be changed to point to
          * something else.   At worst, we may get some requisite failures
          * about conflicting base levels when it turns out that the 
          * conflicting base level is superseded by the one which we
          * picked first -- a fairly rare incident.
          */
         current->apply_state = CANNOT_APPLY_CONFL_LEVEL;

      } /* end if */
   } /* end for */

   return (FALSE);

} /* is_conflicting_impl_requested_base */

/*--------------------------------------------------------------------------*
**
** NAME: consider_requisite_subgraph
**
** FUNCTION:  Determines, based on requisite type, operation being performed
**            and a few other bits and pieces, whether or not a subgraph
**            should be traversed. 
**
** RETURNS:   TRUE requisite subgraph should be examined, FALSE otherwise.
**
**--------------------------------------------------------------------------*/

boolean 
consider_requisite_subgraph 
(
requisite_list_type * requisite
)
{
   switch (requisite->type)
   {
      case AN_INSTREQ:
      case A_PREREQ :
        return (TRUE);
        break;

      case A_COREQ  :
        /*
         * As always, coreqs are ignored when ckprereq is called from 
         * the command line.
         */
        if (check_prereq.called_from_command_line)
           return (FALSE);
         else
           return (TRUE);
        break;

      case AN_IFREQ  :
      case AN_IFFREQ :
        /*
         * First, only consider the ifreq subgraph if the time is appropriate.
         */
        if (check_prereq.consider_ifreqs)
        {
           if (ck_base_lev_list (requisite, CBLL_MIGRATING, FALSE))
              /*
               * Migrating base level filesets should not result in ifreq 
               * dependencies.
               */
              return (FALSE);

           /*
            * If the requisite's base level has or is being applied/committed/
            * rejected/de-installed.....
            */ 
           if (ck_base_lev_list (requisite, CBLL_SUCC_CAND, FALSE))
           {
              if (check_prereq.mode != OP_REJECT)
              {
                 if (check_prereq.mode == OP_COMMIT)
                 {
                    /* 
                     * Don't consider an ifreq subgraph on a commit operation if
                     * the requisite is "missing".
                     */
                    if ((requisite->fix_ptr->origin == DUMMY_FIX) ||
                        (requisite->fix_ptr->apply_state == AVAILABLE))
                       return (FALSE);
                    else
                       return (TRUE);
                 }
                 else
                 {
                    /*
                     * On op apply, we always wan't to consider ifreq 
                     * subgraphs (pull in the ifreqs or fail the subgraph!).
                     */
                    return (TRUE);
                 }
              }
              else
              {
                 /*
                  * We're doing a reject/de-install.  The base level is in 
                  * the graph, meaning its being rejected/de-installed.
                  * Don't go down this ifreq subgraph.
                  */
                 return (FALSE);
              }
           }
           else
           {
              /*
               * The base level is not in the graph (or a candidate to go in
               * graph).  If this is not a reject, don't consider the subgraph.
               * otherwise, we should.
               */
              if (check_prereq.mode != OP_REJECT)
                 return (FALSE);
              else
                 return (TRUE);
           }
        }
        else
        {
           return (FALSE);
        }

   } /* end switch */
} /* consider_requisite_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: inherit_subgraph_failures
**
** FUNCTION:  Traverses (recursively) the subgraph of "fix" until a leaf node 
**            is detected (effectively a base level product) or until a cycle 
**            is detected.  Any failed states in the subgraph are passed back 
**            to the parent.
**
** RETURNS:   
**
**--------------------------------------------------------------------------*/

static boolean 
inherit_subgraph_failures 
(
fix_info_type * fix
)
{ 
   boolean               subgraph_failed = FALSE;
   requisite_list_type * requisite; 
 
   /*
    * Terminate recursion.  
    */ 

   if (fix->requisites == NIL (requisite_list_type) || 
       fix->flags & VISITING_THIS_NODE) 
   {
      if (fix->flags & FAILED_NODE)
         return (TRUE);
      else
         return (FALSE);
   }

   fix->flags |= VISITING_THIS_NODE; /* cycle detection */

   /* 
    * If the requisite subgraph of this node should be examined, (ie. 
    * requisites are prereqs or coreqs, OR ifreqs and we're considering 
    * ifreqs) then inherit any failure status from the subgraph.
    */
   for (requisite = fix->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      if (consider_requisite_subgraph (requisite))
      {
         if (inherit_subgraph_failures (requisite->fix_ptr))
            subgraph_failed = TRUE;

      } /* end if */
   } /* end for */

   if (subgraph_failed)
   {
      MARK_NODE_AS_FAILED (fix);
      return (TRUE);
   }
   else
   {
      /* 
       * Make sure we returned the correct result from this subgraph if this 
       * node is the cause of failure (rather than its subgraph).
       */
      if (fix->flags & FAILED_NODE)
         return (TRUE); 
      else
         return (FALSE);
   }
   
} /* inherit_subgraph_failures */

