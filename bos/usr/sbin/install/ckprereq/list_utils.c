static char sccsid[] = "@(#)74  1.12.1.26  src/bos/usr/sbin/install/ckprereq/list_utils.c, cmdinstl, bos41J, 9517A_all 4/20/95 19:14:26";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *            add_fix_to_fix_info_table
 *            add_unique_node_to_base_lev_list
 *            ck_base_lev_list
 *            delete_prereq_coreq_chains
 *            delete_unneeded_requisites
 *            delete_unreferenced_nodes
 *            delete_supersedes_info
 *            delete_supersedes_chain
 *            get_base_level
 *            get_base_level_back_end
 *            impl_sups_lookup
 *            is_B_req_of_A
 *            link_requisite
 *            locate_fix
 *            locate_fix_info_entry
 *            mark_CANDIDATE_deinstall_updates
 *            mark_initial_CANDIDATE_NODEs
 *            purge_fix_list
 *            put_command_line_args_in_graph
 *            tag_subgraph_SUCCESSFUL_NODEs
 *            free_graph
 *            unmark_SUBGRAPH
 *            unmark_graph
 *            unmark_requisite_visited
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

static void delete_unneeded_requisites (fix_info_type const * fix,
                                     boolean             * error);

extern boolean consider_requisite_subgraph (requisite_list_type * requisite);

static void delete_supersedes_chain (supersede_list_type * sup_node);

static fix_info_type * get_base_level_back_end (
fix_info_type * update);

static void mark_CANDIDATE_deinstall_updates (fix_info_type * fix,
                                              boolean       * error);

/*--------------------------------------------------------------------------*
**
** NAME: purge_fix_list
**
** FUNCTION:  This function deletes every fix_list node, except for the
**            header to the list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void purge_fix_list (possible_fixes_type * const fail_list)
 {
    /* We need to delete everything on the list except the header. */

    possible_fixes_type * ptr;
    possible_fixes_type * next_ptr;

    ptr = fail_list -> next_fix;  /* Skip over the header */
    while (ptr != NIL (possible_fixes_type) )
     {
       next_ptr = ptr -> next_fix;
       free_possible_fix_node (ptr);
       ptr = next_ptr;
     }

    fail_list -> next_fix = NIL (possible_fixes_type);

 } /* end purge_fix_list */

/*--------------------------------------------------------------------------*
**
** NAME: add_fix_to_fix_info_table
**
** FUNCTION:  Adds the given fix_info entry to the fix_info table AND
**            its corresponding hash_table (which is assumed to exist at
**            this time.)  The sorted order of the fix_info table is preserved.
**
** RETURNS:   void function.
**
**--------------------------------------------------------------------------*/

void add_fix_to_fix_info_table (fix_info_type * const fix,
                                boolean       * const error)
{
   boolean         duplicate = FALSE;
   char            fix_name[MAX_LPP_FULLNAME_LEN];
   fix_info_type * fix2;
   fix_info_type * prev;
   int             namerc;
   int             levrc;

   add_to_fix_hash_table (fix, & duplicate, error);
   RETURN_ON_ERROR;

   if (duplicate)
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
               CKP_INT_CODED_MSG_D), inu_cmdname, FIX_ALREADY_IN_FINFO_TABLE);
      * error = TRUE;
      return;
   }

   /*
    * Whenever this routine is called, we are dealing with a sorted, doubly
    * linked fix_info table.  Let's preserve that.  Perform a scan 
    * from the anchor until we find an entry which is greater -- perhaps
    * costly, but this routine is currently only called to add dummy
    * and dummy superseded entries to the fix_info table.
    */
   prev = check_prereq.fix_info_anchor;
   for (fix2 = check_prereq.fix_info_anchor->next_fix;
        fix2 != NIL (fix_info_type);
        fix2 = fix2->next_fix)
   {
      if (((namerc = strcmp (fix2->name, fix->name)) > 0) 
                           ||
          ((namerc == 0) &&
           (levrc = inu_level_compare (&(fix2->level), &(fix->level))) > 0))
         break;

      prev = fix2;
   }
   fix->next_fix = prev->next_fix;
   prev->next_fix = fix;
   fix->prev_fix = prev;
   if (fix->next_fix != NIL (fix_info_type))
      fix->next_fix->prev_fix = fix; 

} /* end add_fix_to_fix_info_table */

/*--------------------------------------------------------------------------*
**
** NAME: add_unique_node_to_base_lev_list
**
** FUNCTION:  Creates a base_lev_list_type structure and adds it to the
**            base_lev_list chained off the requisite node if it is
**            not already present in the list.
**
** RETURNS:   void function.
**
**--------------------------------------------------------------------------*/

void 
add_unique_node_to_base_lev_list  
(
requisite_list_type * req_node, 
fix_info_type       * new_base_lev,
boolean             * error
)
{
   base_lev_list_type * base_lev;

   for (base_lev = req_node->base_lev_list;
        base_lev != NIL (base_lev_list_type);
        base_lev = base_lev->next_base_lev)
   {
      if (inu_level_compare (&(base_lev->fix_ptr->level), 
                             &(new_base_lev->level)) == 0)
         return;
   } /* for */

   base_lev = create_base_lev_list_node (error);
   RETURN_ON_ERROR;
   base_lev->fix_ptr = new_base_lev;
   base_lev->next_base_lev = req_node->base_lev_list;
   req_node->base_lev_list = base_lev;

} /* add_unique_node_to_base_lev_list */

/*--------------------------------------------------------------------------*
**
** NAME: locate_fix
**
** FUNCTION:  This function searches the fix_info table to see if there is
**            already a match for the given product/fix.  There should
**            never be more than one entry.  If the argument "use_fixid"
**            is set to TRUE, we enforce vrmf to match (in addition to 
**            ptf id).  Otherwise, we just require vrm to match.  
**
** RETURNS:   Returns a pointer to the found fix_info entry.  It will be
**            NIL if no matching entry is found.
**
**            
**
**--------------------------------------------------------------------------*/

fix_info_type * locate_fix (char    * const lpp_name,
                            Level_t * const level,
                            boolean   use_fixid) 

 {
   fix_info_type * fix;
   ENTRY         * hash_entry;

   hash_entry = locate_hash_entry (lpp_name, level -> ptf);
   if (hash_entry == NIL (ENTRY) )
      return (NIL (fix_info_type) );

   for (fix = (fix_info_type *) (hash_entry -> data);
        fix != NIL (fix_info_type);
        fix = fix -> collision_list)
    {
      if ((strcmp (fix -> name, lpp_name) == 0)
                             &&
           (fix -> level.ver == level -> ver)
                             &&
           (fix -> level.rel == level -> rel)
                             &&
           (fix -> level.mod == level -> mod)
                             &&
           ((fix -> level.fix == level -> fix) || ! use_fixid ))
      {
         return (fix);
      } /* end if */
    }

   return (NIL (fix_info_type) );

 } /* end locate_fix */

/*--------------------------------------------------------------------------*
**
** NAME: link_requisite
**
** FUNCTION:  This function links a chosen fix (child_fix) into the requisite
**            list of a parent_fix.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

requisite_list_type * 
link_requisite 
(
fix_info_type  * const parent_fix,
fix_info_type  * const child_fix,
criteria_type  * const criteria,
requisite_type   const type,
boolean        * const error
)
{
   boolean               duplicate;
   char                  fix_name[MAX_LPP_FULLNAME_LEN];
   requisite_list_type * insert_after;
   requisite_list_type * prev_req;
   requisite_list_type * requisite;


   /* Make sure that we are not about to commit a major blunder of reqing
      ourself! */

   if (parent_fix == child_fix)
   {
      /*
       * Note: instreq behaves like a prereq for these purposes.
       */
      if (type == A_PREREQ || type == AN_INSTREQ)
      {
         /* "ERROR: %s prereqs itself.\n" */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SELF_PREREQ_E,
                  CKP_SELF_PREREQ_D), inu_cmdname,
                  get_fix_name_from_fix_info (parent_fix, fix_name));
      }
      else
      {
         /* "ERROR: %s coreqs itself.\n" */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SELF_COREQ_E,
                  CKP_SELF_COREQ_D), inu_cmdname,
                  get_fix_name_from_fix_info (parent_fix, fix_name));
      }
      * error = TRUE;
      check_prereq.function_return_code = PREREQ_CYCLE_RC;
      return (NIL (requisite_list_type));
   } /* end if */

   /* Try to cut down the number of duplicate entries.  We can not do this
      if the parent is a DUMMY_GROUP because of the pass_count getting
      messed up.  We can't fix the pass_count because we don't know if this
      requisite will be a requisite_in_graph or not.  Also, we may go ahead
      and make the requisite link if the "child" is a dummy fix since we want
      to link all references to a single dummy fix for this product but use
      different criteria (requisite expressions) for reporting purposes. */
  
   insert_after = NIL (requisite_list_type);
   if (parent_fix -> origin != DUMMY_GROUP)
   {
      for (requisite = parent_fix -> requisites;
           requisite != NIL (requisite_list_type);
           requisite = requisite -> next_requisite)
      {
         if (requisite -> fix_ptr == child_fix &&
             (child_fix->origin != DUMMY_FIX || 
              IF_INSTALL (child_fix->op_type)))
         {
             /* 
              * Make sure if this requisite is already on our requisite chain
              * that we always have the "stronger" requisite.  This can happen
              * with different requisites in the case of supersedes but it is
              * highly unlikely. 
              */

             if (type == A_PREREQ)
                requisite -> type = A_PREREQ;

             if ((requisite->type == AN_IFREQ ||  
                                              requisite->type == AN_IFFREQ) &&  
                 (type == A_COREQ))
                requisite->type = A_COREQ;
                
             return (requisite);
         }
      } /* end for */
   }
   else
   {
      /*
       * Try to order the group members favorably.  Put "installed" members
       * before "to-be-installed" members -- this helps the simplified
       * group requisite evaluation algorithm by lowering the tendency to pick 
       * "to-be-installed" things when the group requirements are already
       * satisfied by things which are already installed.
       */
      prev_req = NIL (requisite_list_type);
      for (requisite = parent_fix -> requisites;
           requisite != NIL (requisite_list_type);
           requisite = requisite -> next_requisite)
      {
          /* stop when worse (larger) state is found. */
          if (requisite->fix_ptr->apply_state >= child_fix->apply_state)
          {
             insert_after = prev_req;
             break;
          }
          prev_req = requisite;

      } /* end for */

      /* handle insertions at end of list. */
      if ((prev_req != NIL (requisite_list_type)) &&
          (insert_after == NIL (requisite_list_type)))
         insert_after = prev_req;
   }

   requisite = create_requisite (criteria, error);
   RETURN_ON_ERROR;

   requisite -> type = type;
   requisite -> fix_ptr = child_fix;
   requisite -> next_requisite = parent_fix -> requisites;

   if (insert_after == NIL (requisite_list_type))
   {
      parent_fix -> requisites = requisite;
   }
   else
   {
      requisite->next_requisite = insert_after->next_requisite;
      insert_after->next_requisite = requisite;
   }

   return (requisite);

} /* end link_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: delete_unneeded_requisites
**
** FUNCTION:  Deletes all of the requisite list for a given fix.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void delete_unneeded_requisites (fix_info_type * const fix,
                                 boolean       * const error)
{
   char                * fix_name;
   requisite_list_type * requisite;
   requisite_list_type * next_requisite;

   if (fix -> flags & VISITING_THIS_NODE)
      return;

   fix -> flags |= VISITING_THIS_NODE;

   /* Do a sanity check on the node to verify that if 
      MEMBER_OF_SUCCESS_SUBGRAPH is selected, then SUCCESSFUL_NODE is also 
      selected. */

   if ((fix -> flags & MEMBER_OF_SUCCESS_SUBGRAPH)  && 
       ! (fix -> flags & SUCCESSFUL_NODE))
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
               CKP_INT_CODED_MSG_D), inu_cmdname, FIX_SHOULD_NOT_BE_IN_GRAPH);
      fix_name = (char *) my_malloc (MAX_LPP_FULLNAME_LEN+1, error); 
      inu_msg (ckp_errs, "\t%s\n", get_fix_name_from_fix_info (fix, fix_name));
      *error = TRUE;
      check_prereq.function_return_code = INTERNAL_ERROR_RC;
      return;
   }
 
   if ((fix -> flags & SUCCESSFUL_NODE)
                       &&
         (fix -> origin != DUMMY_GROUP) )
   {
        requisite = fix -> requisites;
        fix -> requisites = NIL (requisite_list_type);
        while (requisite != NIL (requisite_list_type))
        {
           next_requisite = requisite -> next_requisite;
           if (( (requisite->type == AN_IFREQ || requisite->type == AN_IFFREQ) &&
                ! (requisite -> fix_ptr -> flags & SUCCESSFUL_NODE))
                              ||
                ((check_prereq.mode == OP_REJECT) &&
                 (requisite -> fix_ptr -> origin == DUMMY_GROUP) &&
                 (requisite -> flags.selected_member_of_group)))
           {
              free_requisite (requisite);
           }
           else
           {
              requisite -> next_requisite = fix -> requisites;
              fix -> requisites = requisite;
           }
           requisite = next_requisite;
       } /* end while */
       
       return;
    }

    requisite = fix -> requisites;
    fix -> requisites = NIL (requisite_list_type);
    while (requisite != NIL (requisite_list_type))
    {
       next_requisite = requisite -> next_requisite;
       if ((! (fix -> flags & SUCCESSFUL_NODE) )
                        ||
            ! (requisite -> flags.selected_member_of_group) )
       {
          delete_unneeded_requisites (requisite -> fix_ptr, error);
          RETURN_ON_ERROR;

          free_requisite (requisite);
       }
       else
       {
          requisite -> next_requisite = fix -> requisites;
          fix -> requisites = requisite;
       }
       requisite = next_requisite;
    } /* end while */


} /* end delete_unneeded_requisites */

/*--------------------------------------------------------------------------*
**
** NAME: delete_unreferenced_nodes
**
** FUNCTION:  All of the unreferenced nodes in the fix_info table are deleted.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void delete_unreferenced_nodes (boolean * const error)
 {
  fix_info_type       * fix;
  requisite_list_type * next_requisite;
  fix_info_type       * del_fix;
  requisite_list_type * requisite;


  /*
   * Hash table not needed any more.
   */
  destroy_hash_table ();

  /* Remove any requisites of the anchor that we are not going to do. */

  requisite = check_prereq.fix_info_anchor -> requisites;
  check_prereq.fix_info_anchor -> requisites = NIL (requisite_list_type);
  while (requisite != NIL (requisite_list_type) )
   {
       next_requisite = requisite -> next_requisite;
       if (requisite -> fix_ptr -> flags & SUCCESSFUL_NODE)
        {
          requisite -> next_requisite =
                                    check_prereq.fix_info_anchor -> requisites;
          check_prereq.fix_info_anchor -> requisites = requisite;
        }
       else
        {
          free_requisite (requisite);
        }
       requisite = next_requisite;
   } /* end while */

  /* Delete the requisite chains of the unreferenced nodes. */

  for (fix = check_prereq.fix_info_anchor -> next_fix;
       fix != NIL (fix_info_type);
       fix = fix -> next_fix)
   {
     /* Burn any requisites that point to nodes we are going to delete. */

     delete_unneeded_requisites (fix, error);
     RETURN_ON_ERROR;

   } /* end for */

  /* Now, it is safe to delete the unreferenced nodes (could not do it
     earlier because a prereq/coreq chain may have referenced us. */

  fix = check_prereq.fix_info_anchor -> next_fix;
  while (fix != NIL (fix_info_type) )
   {
      if ( ! (fix -> flags & SUCCESSFUL_NODE) )
       {
          fix->prev_fix->next_fix = fix->next_fix;
          if (fix->next_fix != NIL (fix_info_type))
             fix->next_fix->prev_fix = fix->prev_fix;
          del_fix = fix;
          fix = fix -> next_fix;
          free_fix_info_node (del_fix);
       }
      else
       {
          fix = fix -> next_fix;
       }
   } /* end while */

   unmark_graph (VISITING_THIS_NODE);

   return;

 } /* end delete_unreferenced_nodes */

/*--------------------------------------------------------------------------*
**
** NAME:      move_failed_sop_entry_to_fail_sop
**
** FUNCTION:  Moves a sop entry from the "good" sop passed to inu_ckreq
**            to the "bad" sop passed to inu_ckreq (so that installp may
**            print a summary about failures).  If no "good" sop entry
**            existed (ex. for an auto superseding pkg picked from the
**            toc by inu_ckreq) then a sop entry is created.
**
** RETURNS:   A boolean value
**
**--------------------------------------------------------------------------*/

Option_t * move_failed_sop_entry_to_fail_sop (fix_info_type * fix_ptr,
                                              int             failure_status)
{
   boolean    found;
   char       lev_buf[MAX_LPP_FULLNAME_LEN];
   Option_t * sop_ptr;
   Option_t * prev_sop_ptr;

   /*
    * Let's find the fix's sop entry.  It may not have one if this is an
    * automatic supersedes node.
    */
   found = FALSE;
   prev_sop_ptr = check_prereq.SOP;
   sop_ptr = check_prereq.SOP -> next;
   while ((! found) && (sop_ptr != NIL (Option_t)))
   {
      if ((strcmp (fix_ptr -> name, sop_ptr -> name) == 0) &&
          (inu_level_compare (&(fix_ptr -> level), & (sop_ptr -> level)) == 0))
      {
         found = TRUE;
      }
      else
      {
         prev_sop_ptr = sop_ptr;
         sop_ptr = sop_ptr -> next;
      }
   } /* end while */

   if (found)
   {
      /*
       * unlink the existing entry from the sop.
       */
      prev_sop_ptr -> next = sop_ptr -> next;
   }
   else
   {
      sop_ptr = create_option (fix_ptr -> name, FALSE,
                               QUIESCE_NO,
                               convert_cp_flag_type_to_content_type (
                                               fix_ptr -> cp_flag),
                               "", & fix_ptr -> level, "", NIL (BffRef_t));
   }
   /*
    * Add it to the failure sop.
    */
   sop_ptr -> next = check_prereq.Fail_SOP -> next;
   check_prereq.Fail_SOP -> next = sop_ptr;

   /*
    * Set the failure code for the installp summary.
    */
   sop_ptr -> Status = failure_status;

   /*
    * Add superseded by string for error reporting.
    */
   if (failure_status == STAT_ALREADY_SUPERSEDED &&
       sop_ptr->level.ptf == NIL (char))
      sop_ptr->supersedes = strdup (get_level_from_fix_info (fix_ptr, lev_buf));

   return (sop_ptr);

} /* move_failed_sop_entry_to_fail_sop */

/*--------------------------------------------------------------------------*
**
** NAME: unmark_SUBGRAPH
**
** FUNCTION:  Traverses the entire subgraph and removes the given bit-pattern
**            flag from each node.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void unmark_SUBGRAPH (fix_info_type * const node,
                      int             const bit_pattern)
 {
   requisite_list_type * requisite;

   if (! (node -> flags & bit_pattern) )
      return;

   node -> flags &= ~ bit_pattern;

   for (requisite = node -> requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
    {
      unmark_SUBGRAPH (requisite -> fix_ptr, bit_pattern);
    }

 } /* end unmark_SUBGRAPH */

/*--------------------------------------------------------------------------*
**
** NAME: unmark_graph
**
** FUNCTION:  Traverses the entire graph and removes the given bit-pattern
**            flag from each node.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void unmark_graph (int const bit_pattern)
 {
   fix_info_type * fix;

   for (fix = check_prereq.fix_info_anchor;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      fix -> flags &= ~ bit_pattern;
    }

 } /* end unmark_graph */

/*--------------------------------------------------------------------------*
**
** NAME: unmark_requisite_visited
**
** FUNCTION:  Scans the fix_info_table unmarking the requisite_visited flag
**            for the requisites of each node.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/
void unmark_requisite_visited ()
{
   fix_info_type       * fix;
   requisite_list_type * requisite;

   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      for (requisite = fix -> requisites;
           requisite != NIL (requisite_list_type);
           requisite = requisite -> next_requisite)
      {
           requisite -> flags.requisite_visited = FALSE;
      } /* end for */
   } /* end for */
}

/*--------------------------------------------------------------------------*
**
** NAME: mark_initial_CANDIDATE_NODEs
**
** FUNCTION:  on APPLY  : Tag ALL_PARTS_APPLIED nodes as CANDIDATE_NODE
**            on COMMIT : Tag ALL_PARTS_COMMITTED nodes as CANDIDATE_NODE
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void mark_initial_CANDIDATE_NODEs (void)
 {
   fix_info_type * fix;

   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
       switch (check_prereq.mode)
        {
           case OP_APPLY :
             if (fix -> apply_state == ALL_PARTS_APPLIED)
                fix -> flags |= CANDIDATE_NODE;
                break;

           case OP_COMMIT :
             if (fix -> commit_state == ALL_PARTS_COMMITTED)
                fix -> flags |= CANDIDATE_NODE;
                break;

           case OP_REJECT :
             if (fix -> reject_state == REJECTED)
                fix -> flags |= CANDIDATE_NODE;
                break;
        } /* end switch */
    } /* end for */

 } /* end mark_initial_CANDIDATE_NODEs */

/*--------------------------------------------------------------------------*
**
** NAME: put_command_line_args_in_graph
**
** FUNCTION:  Links all command line args as requisites of the anchor.
**            Also tags them with CANDIDATE_NODE.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void put_command_line_args_in_graph (boolean * const error)
{
   fix_info_type * fix;
   fix_info_type * prev;

   /*
    * We're gonna try to preserve the alphabetic order of the command line 
    * requests.  Not only does it not hurt anything, it can impose some
    * predictability on the requisite evaluation process.
    * Find the last node in the fix_info_table and then scan backwards.
    * (Link requisite always adds to the beginning of the requisite list).
    */
   for ( fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
       prev = fix; 
   }
   for (fix = prev;
        fix != check_prereq.fix_info_anchor;
        fix = fix->prev_fix)
   {
       if (fix -> origin == COMMAND_LINE)
       {
          switch (check_prereq.mode)
          {
             case OP_APPLY: 
                  if (fix -> apply_state == TO_BE_EXPLICITLY_APPLIED)
                     fix -> flags |= CANDIDATE_NODE;
                  break;

             case OP_COMMIT: 
                  if (fix -> commit_state == TO_BE_EXPLICITLY_COMMITTED)
                     fix -> flags |= CANDIDATE_NODE;
                  break;

             case OP_REJECT: 
                  if (fix -> reject_state == TO_BE_EXPLICITLY_REJECTED)
                  {
                     fix -> flags |= CANDIDATE_NODE;

                    /*
                     * We don't want to have to rely on -g being switched on 
                     * to pull in updates of the product that was requested to
                     * be de-installed.  We'll mark them here as candidates.
                     */
                    if (check_prereq.deinstall_submode)
                    {
                       mark_CANDIDATE_deinstall_updates (fix, error);
                       RETURN_ON_ERROR;
                    }
                  }
                  break;
          }
          (void) link_requisite (check_prereq.fix_info_anchor,
                                 fix, & EMPTY_CRITERIA, A_PREREQ, error);
          RETURN_ON_ERROR;
       }
   } /* end for */

} /* end put_command_line_args_in_graph */

/*--------------------------------------------------------------------------*
**
** NAME: remove_groups_from_graph
**
** FUNCTION:  Removes "group nodes" from graph so that installp's in-line
**            requisite checks need not worry about them.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void remove_groups_from_graph 
(
fix_info_type * parent, 
fix_info_type * node,
boolean       * error
)
{
   boolean               found;
   criteria_type         dupe_criteria;
   fix_info_type       * fix_ptr;
   fix_info_type       * prev_fix_ptr;
   requisite_list_type * next_req_ptr;
   requisite_list_type * prev_req_ptr;
   requisite_list_type * requisite;

    /*
     * Cycle detection.
     */ 
    if (node->flags & (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED))
       return;
    node->flags |= VISITING_THIS_SUBGRAPH;

    /*
     * First, ensure that there's no groups in subgraph (by recursing on 
     * requisites of the current node).
     */
    for (requisite = node->requisites;
         requisite != NIL (requisite_list_type);
         requisite = requisite->next_requisite)
    {
        remove_groups_from_graph (node, requisite->fix_ptr, error);
        RETURN_ON_ERROR;
    } /* end for */

    /*
     * Remove this node if it's a dummy group.
     */
    if (node->origin == DUMMY_GROUP) 
    {
       /*
        * Start by attaching all of it's children to its parent.
        */
       requisite = node -> requisites;
       while (requisite != NIL (requisite_list_type) )
       {
          next_req_ptr = requisite->next_requisite;
          dupe_relations (& requisite->criteria, & dupe_criteria, error);
          (void) link_requisite (parent, requisite->fix_ptr, & dupe_criteria, 
                                 requisite->type, error);
          RETURN_ON_ERROR;
          free_requisite (requisite);
          requisite = next_req_ptr;
       } /* end while */
       node->requisites = NIL (requisite_list_type);

       /*
        * Next, find it's position on the requisite chain of its parent.
        */
       found = FALSE;
       prev_req_ptr = NIL (requisite_list_type);
       requisite = parent->requisites;
       while ((! found) && (requisite != NIL (requisite_list_type)))
       {
          if (requisite->fix_ptr == node) {
             found = TRUE;
          } else {
             prev_req_ptr = requisite;
             requisite = requisite->next_requisite;
          }
       } /* end while */

       /*
        * Remove it from the parent's requisite list.
        */
       if (found) {
          if (prev_req_ptr == NIL (requisite_list_type))
             parent->requisites = requisite->next_requisite;
          else
             prev_req_ptr->next_requisite = requisite->next_requisite;
          free_requisite (requisite);

       } else  {
          /*
           * A little sanity checking so we don't end up with some 
           * hard-to-find errors elsewhere.
           */
          inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                   CKP_INT_CODED_MSG_D), inu_cmdname, CANT_FIND_FIX);
          *error = TRUE;
          return;
       }

       /*
        * Finally, remove the group node from the fix_info_table.
        */
       node->prev_fix->next_fix = node->next_fix;
       if (node->next_fix != NIL (fix_info_type))
          node->next_fix->prev_fix = node->prev_fix;
       free_fix_info_node (node);
    }
    else
    { 
       node->flags &= ~VISITING_THIS_SUBGRAPH;
       node->flags |=  SUBGRAPH_VISITED;

    } /* end if */

} /* remove_groups_from_graph */

/*--------------------------------------------------------------------------*
**
** NAME: tag_subgraph_SUCCESSFUL_NODEs
**
** FUNCTION:  This subgraph passes the requisite test and needs to be marked
**            SUCCESSFUL_NODE.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void tag_subgraph_SUCCESSFUL_NODEs (requisite_list_type * link_from_parent,
                                 fix_info_type       * const fix,
                                 boolean             * const error)
 {
   boolean               evaluate_fix;
   int                   number_of_passes = 0;
   requisite_list_type * requisite;
   boolean               result = TRUE;

   if (check_prereq.mode == OP_REJECT && fix -> origin == DUMMY_GROUP)
   {
      if (link_from_parent -> flags.selected_member_of_group)
         return;
   }

   if (fix -> flags & VISITING_THIS_NODE)
      return;

   fix -> flags |= VISITING_THIS_NODE;

   if (! (fix -> flags & (CANDIDATE_NODE | SUCCESSFUL_NODE) ) )
    {
       /* 
        * Internal Error:  Expected a member of subgraph to be marked.
        */
       * error = TRUE;
       inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
                CKP_INT_CODED_MSG_D), inu_cmdname, EXPECTED_NODE_TO_BE_MARKED);
       check_prereq.function_return_code = INTERNAL_ERROR_RC;
       return;
    }
   fix -> flags &= ~CANDIDATE_NODE;
   fix -> flags |= SUCCESSFUL_NODE;

   for (requisite = fix -> requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
    {

       if ((fix -> origin == DUMMY_GROUP)
                     &&
            (check_prereq.mode != OP_REJECT)
                     &&
            ! (requisite -> flags.selected_member_of_group) )
        {
           evaluate_fix = FALSE;
        }
        else
        {
           evaluate_fix = consider_requisite_subgraph (requisite);
        }

       if (evaluate_fix)
        {
          tag_subgraph_SUCCESSFUL_NODEs (requisite, requisite -> fix_ptr, error);
          RETURN_ON_ERROR;
        }

    } /* end for */

 } /* end tag_subgraph_SUCCESSFUL_NODEs */

/*--------------------------------------------------------------------------*
**
** NAME: free_graph
**
** FUNCTION:  Free all requisite and fix_info structures still in the graph.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void free_graph ()
 {
  fix_info_type       * fix_info;
  fix_info_type       * next_fix;
  requisite_list_type * next_requisite;
  requisite_list_type * requisite;

  for (fix_info = check_prereq.fix_info_anchor -> next_fix;
       fix_info != NIL (fix_info_type);
       fix_info = fix_info -> next_fix)
  {
      requisite = fix_info -> requisites;
      while (requisite != NIL (requisite_list_type) )
      {
         next_requisite = requisite -> next_requisite;
         free_requisite (requisite);
         requisite = next_requisite;
      } /* end while */

      fix_info -> requisites = NIL (requisite_list_type);

  } /* end for */

  fix_info = check_prereq.fix_info_anchor;
  check_prereq.fix_info_anchor = NIL (fix_info_type);
  while (fix_info != NIL (fix_info_type))
  {
     next_fix = fix_info -> next_fix;
     free_fix_info_node (fix_info);
     fix_info = next_fix;
  } /* end while */

 } /* end free_graph */

/*--------------------------------------------------------------------------*
**
** NAME: fix_info_lookup
**
** FUNCTION:  This function searches the fix_info table to see if there is
**            already a match for the given product/fix.  There should
**            never be more than one entry.
**
** RETURNS:   Returns a pointer to the found fix_info entry.  It will be
**            NIL if no matching entry is found.
**
**
**--------------------------------------------------------------------------*/

fix_info_type * fix_info_lookup (char    * lpp_name, 
                                 Level_t * level,
                                 boolean   use_fix_id)

{
   /* Our job here is to scan the fix_info table to see if we can
      find a match for lpp_name, level */

   fix_info_type * entry;

   for (entry = check_prereq.fix_info_anchor -> next_fix;
        entry != NIL (fix_info_type);
        entry = entry -> next_fix)
    {
       if ((strcmp (entry -> level.ptf, level -> ptf) == 0)
                              &&
            (strcmp (entry -> name, lpp_name) == 0)
                              &&
              (entry -> level.ver == level -> ver)
                              &&
              (entry -> level.rel == level -> rel)
                              &&
              (entry -> level.mod == level -> mod)
                              &&
              ((! use_fix_id) || (entry -> level.fix == level -> fix)) )
        {
          /* Whoo doggy!  This is our fix!  */

          return (entry);
        }
    } /* end for */
   /* Tough luck */

   return (NIL (fix_info_type));

} /* end fix_info_lookup */

/*--------------------------------------------------------------------------*
**
** NAME: impl_sups_lookup
**
** FUNCTION:  Looks for the lowest entry in the fix_info_table with
**            vrmf > level passed matching the name passed.
**
** RETURNS:   Returns a pointer to the found fix_info entry.  It will be
**            NIL if no matching entry is found.
**
**--------------------------------------------------------------------------*/

fix_info_type * impl_sups_lookup 
(
char    * lpp_name, 
Level_t * level
)
{
   fix_info_type * fix;
   fix_info_type * hash_fix;
   ENTRY         * hash_entry;
   int             rc;

   hash_entry = locate_hash_entry (lpp_name, "");

   if (hash_entry == NIL (ENTRY) )
      return (NIL (fix_info_type));

   hash_fix = (fix_info_type *) (hash_entry->data);
   if ((rc = inu_level_compare (&(hash_fix->level), level)) > 0)
   {
      /*
       * hashed fix is > what we want.  Let's look to the left until
       * we find a fix with a different name OR a fix that's lower.
       * Then we'll return the next fix to that.
       */
      for (fix = hash_fix->prev_fix; 
           ;                     /* No stopping condition necessary, guaranteed
                                    to stop on name change of anchor. */
           fix = fix->prev_fix)
      {
         if ((strcmp (fix->name, lpp_name) != 0) ||
             (inu_level_compare (&(fix->level), level) < 0))
            return (fix->next_fix);

      } /* end for */
   }
   else
   {
      if (rc < 0)
      {
         /*
          * hashed fix < what we want.  scan fix info to the right until
          * find a different name OR a fix that's >= to level.
          */
         for (fix = hash_fix->next_fix; 
              fix != NIL (fix_info_type);
              fix = fix->next_fix)
         {
             if (strcmp (fix->name, lpp_name) != 0) 
                return (NIL (fix_info_type));
             if (inu_level_compare (&(fix->level), level) >= 0)
                return (fix);
             
         } /* end for */
      }
      else
         return (hash_fix);
   }
   return (NIL (fix_info_type));
 
} /* impl_sups_lookup */

/*--------------------------------------------------------------------------*
**
** NAME:      delete_supersedes_info
**
** FUNCTION:  Scans the fix_info table deleting requisite chains and deleting
**            any fix_info structures that are no longer needed.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void delete_supersedes_info ()
{
  fix_info_type       * fix;
  fix_info_type       * del_fix;
  supersede_list_type * next_supersede;
  supersede_list_type * supersede;

  /*
   * Scan fix_info table.  Look for the base of explicit supersedes chains
   * (ie. nodes that are superseded by another but which don't supersede
   * others).  Recurse along that chain, deleting the supersedes 
   * information (leaving the fix_info structures in tact.)
   */
  for (fix = check_prereq.fix_info_anchor -> next_fix;
       fix != NIL (fix_info_type);
       fix = fix->next_fix)
  {
     if ((fix->supersedes == NIL (supersede_list_type)) &&
         (fix->superseded_by != NIL (supersede_list_type)))
     {
        delete_supersedes_chain (fix->superseded_by);
     }
  } /* end for */

} /* delete_supersedes_info */

/*--------------------------------------------------------------------------*
**
** NAME:      get_base_level
**
** FUNCTION:  Gets the base level package from the subgraph of the 
**            fix_ptr passed in the call.
**
** RETURNS:   Ptr to the fix_info node representing the base level package
**            of the update in question.
**
**--------------------------------------------------------------------------*/

fix_info_type *
get_base_level
(
fix_info_type * update
)
{
   fix_info_type * ret_fix;

   if (IF_INSTALL (update->op_type))
      return (update);

   ret_fix = get_base_level_back_end (update);
   unmark_graph (VISITING_THIS_NODE);
   return (ret_fix);

} /* get_base_level */

/*--------------------------------------------------------------------------*
**
** NAME:      get_base_level_back_end
**
** FUNCTION:  see front end routine.
**
** RETURNS:   see front end routine.
**
**--------------------------------------------------------------------------*/

static fix_info_type *
get_base_level_back_end
(
fix_info_type * update
)
{
   requisite_list_type * requisite;

   /*
    * Cycle detection.
    */
   if (update->flags & VISITING_THIS_NODE)
      return (NIL (fix_info_type));

   update->flags |= VISITING_THIS_NODE;

   /*
    * Scan the requisite list of the update passed.  look for a base level
    * pkg, with the same name, version and release.
    */
   for (requisite = update->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      if ((IF_INSTALL (requisite->fix_ptr->op_type)) &&
          (strcmp (update->name, requisite->fix_ptr->name) == 0) &&
          (update->level.ver == requisite->fix_ptr->level.ver) &&
          (update->level.rel == requisite->fix_ptr->level.rel))
         return (requisite->fix_ptr);
   } /* for */

   /*
    * Since an update doesn't have to immediately prereq its base level,
    * recurse on requisites with same name (ie. didn't find it above).
    */
   for (requisite = update->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      if (strcmp (update->name, requisite->fix_ptr->name) == 0)
         return (get_base_level_back_end (requisite->fix_ptr));

   } /* for */

   return (NIL (fix_info_type));  /* should rarely get here */

} /* get_base_level_back_end */

/*--------------------------------------------------------------------------*
**
** NAME:      is_B_req_of_A
**
** FUNCTION:  Scans the requisite list of the A argument passed for 
**            the existence of the B argument.  B must also be connected
**            to A by a PREREQ, COREQ or INSTREQ link. (ie. not an ifreq)
**
** RETURNS:   TRUE if B is a req of A.
**            FALSE otherwise.
**
**--------------------------------------------------------------------------*/

boolean
is_B_req_of_A
(
fix_info_type * A,
fix_info_type * B
)
{
   requisite_list_type * requisite;

   for (requisite = A->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      if ((requisite->fix_ptr == B)
                   &&
          (requisite->type == A_PREREQ   || 
           requisite->type == AN_INSTREQ ||
           requisite->type == A_COREQ))
         return (TRUE);
   }
   return (FALSE);

} /* is_B_req_of_A */

/*--------------------------------------------------------------------------*
**
** NAME:      delete_supersedes_chain
**
** FUNCTION:  Traverses (recursively) the superseded_by chain of the sup_node
**            passed as an argument, all the way to the latest fix in the 
**            chain.  "On the way back", the supersedes nodes are deleted.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void delete_supersedes_chain (supersede_list_type * sup_node)
{

  /*
   * STOP THE RECURSION!!!!
   */
  if (sup_node == NIL (supersede_list_type))
     return;

  /*
   * RRRecurse along the superseded_by chain.
   */
  delete_supersedes_chain (sup_node->superseding_fix->superseded_by);

  /* 
   * Take care of next and previous pointers which point to THIS node.
   */ 
  if (sup_node->next_supersede != NIL (supersede_list_type))
     sup_node->next_supersede->previous_supersede = NIL (supersede_list_type);
  if (sup_node->previous_supersede != NIL (supersede_list_type))
     sup_node->previous_supersede->next_supersede = NIL (supersede_list_type);

  /*
   * Zero out my supersedes pointers. 
   */
  sup_node->superseding_fix->supersedes = NIL (supersede_list_type);
  sup_node->superseded_fix->superseded_by = NIL (supersede_list_type);

  my_free (sup_node); /* zap! */

} /* delete_supersedes_chain */

/*--------------------------------------------------------------------------*
**
** NAME:      mark_CANDIDATE_deinstall_updates
**
** FUNCTION:  Marks all updates to the base_fix passed as an argument as
**            CANDIDATE_NODEs.
**           
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void mark_CANDIDATE_deinstall_updates (fix_info_type * base_fix,
                                              boolean       * error)
{
   char            fix_name [MAX_LPP_FULLNAME_LEN];
   fix_info_type * hashed_fix;
   fix_info_type * other_fix;
   ENTRY         * hash_entry;


   hash_entry = locate_hash_entry (base_fix->name, "");

   if (hash_entry == NIL (ENTRY) )
   {
      /* We could not find this entry that should already be in the table! */

      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
               CKP_INT_CODED_MSG_D), inu_cmdname,
               CANT_FIND_FIX);
      inu_msg (ckp_errs, "%s\n", get_fix_name_from_fix_info (base_fix, 
                                                                   fix_name));
      * error = TRUE;
      check_prereq.function_return_code = INTERNAL_ERROR_RC;
      return;
   }

   hashed_fix = (fix_info_type *) (hash_entry->data);
   /*
    * Mark all entries in the fix_info table with the same name.
    * First scan left, then scan right.
    */
   other_fix = hashed_fix->prev_fix;
   while (strcmp (base_fix->name, other_fix->name) == 0)
   {
       other_fix->flags |= CANDIDATE_NODE;      
       other_fix = other_fix->prev_fix;
   }
   other_fix = hashed_fix;
   while ((other_fix != NIL (fix_info_type)) &&
          (strcmp (base_fix->name, other_fix->name) == 0))
   {
       other_fix->flags |= CANDIDATE_NODE;      
       other_fix = other_fix->next_fix;
   }

} /* mark_CANDIDATE_deinstall_updates */

/*----------------------------------------------------------------------*
 *
 * Function:    ck_base_lev_list
 *
 * Purpose:     Given a base_lev_list structure and an check_id,
 *              iterates through the list checking the contents of
 *              the base level list.  A TRUE or FALSE value will be
 *              returned depending upon the check being performed.
 *              The base_lev_list facilitates multi-base level
 *              specificiations for an ifreq on a given update.
 *              See parser.c for more details on the base_lev_list 
 *              structure.
 * 
 * Returns:     TRUE or FALSE depending upon the specified check_id
 *              (See each case of switch statement for details.)
 *
 *----------------------------------------------------------------------*/

boolean
ck_base_lev_list 
(
requisite_list_type * req_node,
short                 check_id,
boolean               cld_by_rpt_routine
)
{
   base_lev_list_type * base_lev;
   boolean              in_graph;

   for (base_lev = req_node->base_lev_list;
        base_lev != NIL (base_lev_list_type);
        base_lev = base_lev->next_base_lev)
   {
      switch (check_id)
      {
         case CBLL_UNRES_IFREQ : 
              /*
               * If any base levels of the "multi-base" ifreq are installed
               * or were selected to be installed, return true.
               */
              if ((base_lev->fix_ptr->apply_state == ALL_PARTS_APPLIED) ||
                  (base_lev->fix_ptr->flags & SUCCESSFUL_NODE))
                 return (TRUE);
              break;

         case CBLL_MIGRATING : 
              /*
               * If any base levels of the "multi-base" ifreq are 
               * MIGRATING, return TRUE.
               */
               if (MIGRATING (base_lev->fix_ptr->cp_flag))
                  return (TRUE);
              break;

         case CBLL_SUCC : 
              /*
               * If any base levels of the "multi-base" ifreq are marked
               * SUCCESSFUL, return TRUE;
               */
              if (base_lev->fix_ptr->flags & SUCCESSFUL_NODE)
                 return (TRUE);

              break;

         case CBLL_SUCC_CAND : 
              /*
               * If any base levels of the "multi-base" ifreq were selected
               * to be applied/committed/deinstalled, return TRUE;
               * If it's a reject op, if any "non-missing" base_lev
               * of the ifreq are *NOT* in the graph (ie not being removed), 
               * return FALSE. (Note that these multiple-installed base levels 
               * can exist because the parenthetic level of an ifreq can 
               * reference an update as its base.)
               */
              in_graph = base_lev->fix_ptr->flags & 
                                        (SUCCESSFUL_NODE | CANDIDATE_NODE);
              if (check_prereq.mode != OP_REJECT ||
                  check_prereq.deinstall_submode) {

                 if ((in_graph) || 
                      ((cld_by_rpt_routine) && 
                       (base_lev->fix_ptr->origin == COMMAND_LINE)))
                    return (TRUE);

              } else {
                 /*
                  * Doing reject.  All base levels must be in process of being
                  * removed.  Don't count "missing base levels".
                  */
                 if ((! in_graph) &&
                     (base_lev->fix_ptr->origin != DUMMY_FIX))
                    return (FALSE);
              }
              break;

         case CBLL_MISSING :
              /*
               * If any base levels of the "multi-base" ifreq are not mssing 
               * (ie. the base_lev ptr does not point to the requisite)
               * return FALSE.
               */
              if (base_lev->fix_ptr != req_node->fix_ptr)
                 return (FALSE);
      } /* switch */
   } /* for */

   /*
    * Now return the complement of the values which should have been
    * returned above.
    */
   switch (check_id)
   {
      case CBLL_SUCC_CAND : 
           if (check_prereq.mode == OP_REJECT &&
               !check_prereq.deinstall_submode)
              return (TRUE);
           else
              return (FALSE);
           break;
      case CBLL_UNRES_IFREQ : 
      case CBLL_SUCC : 
      case CBLL_MIGRATING : 
           return (FALSE);
           break;
      case CBLL_MISSING :
           return (TRUE);
           break;
   } /* switch */

} /* ck_base_lev_list */
