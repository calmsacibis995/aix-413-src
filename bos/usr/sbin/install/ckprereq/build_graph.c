static char sccsid[] = "@(#)56  1.18  src/bos/usr/sbin/install/ckprereq/build_graph.c, cmdinstl, bos41J, 9521B_all 5/25/95 16:27:42";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: build_graph
 *            build_base_prereqs
 *            do_i_prereq_my_base_lev
 *            get_implied_base_level
 *            invert_requisites 
 *            check_base_lev_prereq
 *            update_has_expl_prereq_on_base
 *            verify_graph
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

static void invert_requisites (
fix_info_type * fix);

static void verify_graph (
boolean * error);

static void check_base_lev_prereq (
fix_info_type * fix,
boolean       * error);

static boolean do_i_prereq_my_base_lev (
fix_info_type * update,
fix_info_type * prereq,
boolean       * prereq_wrong_format,
boolean       * warning,
boolean       * error);

static void check_cross_prod_deps_caused_by_updates (
boolean * error);

static void get_implied_base_level (
fix_info_type * update,
Level_t       * level,
boolean       * error);

static boolean update_has_expl_prereq_on_base (
fix_info_type * update,
Level_t       * level,
boolean       * error);

/*--------------------------------------------------------------------------*
**
** NAME: build_graph
**
** FUNCTION:  Builds the requisite graph for every node in the fix_info table.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void build_graph 
(
boolean * error
)
{
   fix_info_type       * fix;

   /* Visit every node in the fix_info table and build it's requisite
      subgraph. */

   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      build_subgraph (fix, error);
      RETURN_ON_ERROR;
    } /* end for */

   /* When we built the subgraph we marked certain nodes with VISITING_THIS_NODE
      to stop recursion.  Clear those bits now. */

   unmark_graph (VISITING_THIS_NODE);

   /* Make sure the graph is legal (looks for prereq cycles, etc.).  Also
      verifies that ptfs prereq base levels, etc. */

   verify_graph (error);
   RETURN_ON_ERROR;

   /*
    * Explicit supersedes chains are no longer necessary.
    * Remove the supersedes nodes between explicit superseding updates 
    * (3.1 and 3.2 updates).
    */
   delete_supersedes_info();


   /* If we are doing a reject, then invert the subgraphs so that instead of
      each node pointing to it's requisites, each node will point to all of
      the nodes it is a requisite of. */

   if (check_prereq.mode == OP_REJECT)
    {
      /*
       * For de-install ops we need to check for updates which have hard
       * requisites on other products for which there is _NO_ corresponding 
       * requisite from the update's base level to the other product.  We will 
       * consider these internally as "hard" dependencies between
       * the two base products.  If -g is switched on, we will create
       * the requisite between the base level ourselves so that normal 
       * auto-deinstall processing can occur.  Regardless of -g, we will make 
       * the requisite->base_lev_list field of the requisite node linking the 
       * update and the "other product" point to the "offending update's" 
       * base product.  This will aid in detecting the failure in the report 
       * routines of ckprereq (since our graph is about to get inverted and we 
       * tend to lose such information).
       */

      if (check_prereq.deinstall_submode)
      {
         check_cross_prod_deps_caused_by_updates (error);
         RETURN_ON_ERROR;
      }

      for (fix = check_prereq.fix_info_anchor -> next_fix;
           fix != NIL (fix_info_type);
           fix = fix -> next_fix)
       {
         invert_requisites (fix);
       } /* end for */

      unmark_requisite_visited ();
    }

} /* end build_graph */

/*--------------------------------------------------------------------------*
**
** NAME: build_base_prereqs
**
** FUNCTION:  Traverses the sorted fix_info table creating requisite links from
**            41 updates to their explicitly prereqed or implicitly implied
**            base level package.  If prereq is not installed and not on
**            media, a dummy fix_info entry is created and inserted in 
**            the fix_info table.
**
**            Implicit rules: 
**            A 4.1 format fileset update with non-zero fix value implicitly 
**              prereqs that fileset level with a zero fix value.
**            A 4.1-format fileset update with zero fix value but non-zero 
**              modification value implictly prereqs that fileset level with 
**              a zero modification value and zero fix value.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/
void
build_base_prereqs
(
boolean * error
)
{
   criteria_type   criteria;
   fix_info_type * current;
   fix_info_type * current2;
   fix_info_type * prereq;
   int             rc;
   int             strcmprc;
   Level_t         target_lev;
 
   for (current = check_prereq.fix_info_anchor->next_fix; /* Traverse table. */
        current != NIL (fix_info_type);
        current = current->next_fix)
   {
      if (IF_4_1 (current->op_type)  /* only concerned with 4.1 updates */
                    && 
          IF_UPDATE (current->op_type) 
                    &&
          (! IS_SUPERSEDED (current)))
      {
         /* 
          * Check for explicit base prereq.  target_lev will contain the
          * level of that prereq if one exists (ie. TRUE returned)
          */ 
         if (! update_has_expl_prereq_on_base (current, &target_lev, error))
         {
            RETURN_ON_ERROR;
            get_implied_base_level (current, &target_lev, error);
         } /* end if */
         RETURN_ON_ERROR;

         /*
          * Set up the criteria structure which is normally used to 
          * hold information about a requisite expression for requisite
          * failure messages.  Will be used here to look up a requisite
          * to the base level for the update. 
          */
         criteria = EMPTY_CRITERIA;
         create_relation (&criteria, VERSION, EQUALS, 
                          target_lev.ver, NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (&criteria, RELEASE, EQUALS, 
                          target_lev.rel, NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (&criteria, MODIFICATION, EQUALS, 
                          target_lev.mod, NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (&criteria, FIX, EQUALS, 
                          target_lev.fix, NIL (char), error);
         RETURN_ON_ERROR;

         evaluate_requisite (current, current->name, &prereq, &criteria,
                             NIL (criteria_type), A_PREREQ, TRUE, error);

         if (prereq->origin == DUMMY_FIX)
            /*
             * marking dummy as a base level will aid processing later.
             */
            prereq->op_type = OP_TYPE_INSTALL;

         /*
          * Link the update to its base level.
          */
         (void) link_requisite (current, prereq, &criteria, A_PREREQ, error);
         RETURN_ON_ERROR;

      } /* end if */
   } /* end for */

} /* build_base_prereq */

/*--------------------------------------------------------------------------*
**
** NAME: invert_requisites
**
** FUNCTION:  Inverts the given requisite subgraph so that instead of a node
**            pointing to it's requisites, the requisites point back to the
**            node.  This makes REJECT graphs look like apply/commit graphs.
**
**            Used only by REJECT.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void 
invert_requisites 
(
fix_info_type * fix
)
{
   fix_info_type       * child_fix;
   fix_info_type       * group_requisite;
   requisite_list_type * next_requisite;
   requisite_list_type * previous_requisite;
   requisite_list_type * requisite;

   requisite = fix -> requisites;
   fix -> requisites = NIL (requisite_list_type);
   while (requisite != NIL (requisite_list_type))
    {
      if (fix -> origin == DUMMY_GROUP)
      {
         /*
          * Bump up a count that helps us determine if a pkg with a group 
          * requisite should be rejected/de-installed when removing members of 
          * the group.  Bump the count of BROKEN pkgs for de-install ops if 
          * they were explicitly requested.  This will mean we don't count the
          * BROKEN pkg as a prereq which satisfies a member of the group.  We 
          * won't worry about them being implicitly requested -- it will
          * simply mean that the dependent pkg will need to be de-installed
          * when maybe it didn't need to be.
          */
         group_requisite = requisite -> fix_ptr;

         if (group_requisite -> apply_state == ALL_PARTS_APPLIED 
                                          ||
             group_requisite -> apply_state == PARTIALLY_APPLIED 
                                          ||
             (group_requisite->apply_state == BROKEN &&
              check_prereq.deinstall_submode &&
              group_requisite->reject_state == TO_BE_EXPLICITLY_REJECTED))
         {
            (fix -> num_rejectable_requisites)++;
         }
      }

      next_requisite = requisite -> next_requisite;
      if (requisite -> flags.requisite_visited)
       {
         requisite -> next_requisite = fix -> requisites;
         fix -> requisites = requisite;
       }
      else
       {
         requisite -> flags.requisite_visited = TRUE;    /* Once is enough. */

         /* Now change the node this requisite points to. */

         child_fix = requisite -> fix_ptr;
         requisite -> fix_ptr = fix;

         /* Now add the node as a requisite to the child. */

         requisite -> next_requisite = child_fix -> requisites;
         child_fix -> requisites = requisite;
       }
      requisite = next_requisite;
    } /* end while */
} /* end invert_requisites */

/*--------------------------------------------------------------------------*
**
** NAME: verify_graph
**
** FUNCTION:  Walks the graph to verify:
**            1.  that there are no prereq cycles
**            2.  every update prereqs its base.
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void 
verify_graph 
(
boolean * error
)
{
   fix_info_type       * fix;
   
   check_for_prereq_cycles (check_prereq.fix_info_anchor, error);
   RETURN_ON_ERROR;

    /* Now make sure that every fix prereqs it's base product. */

   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
       if (IF_UPDATE (fix -> op_type) )
       {
          check_base_lev_prereq (fix, error);
          RETURN_ON_ERROR;
       }
   } 

} /* end verify_graph */

/*--------------------------------------------------------------------------*
**
** NAME: check_base_lev_prereq
**
** FUNCTION:  The given fix is an update.  We need to verify that a 3.2 (and
**            above) update prereqs another update or install of the same
**            level as the product.  If the image is a 3.1 update then
**            we need to fake a prereq to the base product.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void 
check_base_lev_prereq 
(
fix_info_type * fix,
boolean       * error
)
{
   boolean                warning = FALSE;
   boolean                prereq_wrong_format = FALSE;
   char                   fix_name[MAX_LPP_FULLNAME_LEN];
   criteria_type          criteria;
   fix_info_type        * base_fix;

   /*
    * No check necessary for things marked superseded, since we didn't 
    * create a requisite subgraph, likewise for AVAILABLE filesets (though
    * there should never be any availables.
    */
   if ((IS_SUPERSEDED (fix))              || 
       (fix->apply_state == AVAILABLE)    ||
       (fix->apply_state == CANNOT_APPLY) ||
       (MIGRATING (fix->cp_flag)))
       return;

   if (! do_i_prereq_my_base_lev (fix, fix, &prereq_wrong_format, 
                                  &warning, error) )
   {
      if (*error)
      {
         /*
          * Must be a case of 3.2 updt prereqing 4.1 base or vice versa OR 
          * a 4.1 update prereqing greater vrmf.
          */
         if (prereq_wrong_format)
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                CKP_NO_MIX_N_MATCH_E, CKP_NO_MIX_N_MATCH_D), 
                inu_cmdname, get_fix_name_from_fix_info (fix, fix_name));
         }
         else
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                CKP_NO_PRQ_BASE_ERR_E, CKP_NO_PRQ_BASE_ERR_D), 
                inu_cmdname, get_fix_name_from_fix_info (fix, fix_name));
         }
         return;
      }
      /* 3.2 updates and above must have an explicit prereq of another update
         or install level of this same release level of this product.  Flag
         it as a warning (though its likely to cause failures elsewhere).*/

      if (warning ||  !(IF_3_1_UPDATE (fix -> op_type) ) )
      {
         fix->flags |= WARN_NO_PRQ_BASE;  
         if (!check_prereq.called_from_ls_programs)
         {
            add_entry_to_index_list (report_anchor.misc_warnings_hdr,
                                  fix, NIL (Option_t), &EMPTY_CRITERIA, error);
            RETURN_ON_ERROR;

            /*
             * Don't disable any that we explicitly marked as warnings.  They
             * may be legitimate, exploitation of failure to catch this error
             * in the past.  We will disable if it had no prereq whatsover
             * on a base level.
             */
            if (!warning)
            {
               fix -> apply_state = CANNOT_APPLY;
               fix -> commit_state = CANNOT_COMMIT;
               fix -> reject_state = CANNOT_REJECT;
            }
         }
      }
      else
      {

         /* 3.1 update: need to manufacture a prereq to the base product.

            Our base product's Version and Release levels must be the same as
            our's.  We'll falsify some criteria based on this and borrow
            the parser's routines to create the requisite record. */

         criteria = EMPTY_CRITERIA;
         create_relation (& criteria, VERSION, EQUALS, fix -> level.ver,
                          NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (& criteria, RELEASE, EQUALS, fix -> level.rel,
                          NIL (char), error);

         evaluate_requisite (fix, fix->name, & base_fix, & criteria, 
                             NIL (criteria_type), A_PREREQ, FALSE, error);
         RETURN_ON_ERROR;

         (void) link_requisite (fix, base_fix, & criteria, A_PREREQ, error);
         RETURN_ON_ERROR;
      }
   }
} /* end check_base_lev_prereq */

/*--------------------------------------------------------------------------*
**
** NAME: do_i_prereq_my_base_lev
**
** FUNCTION:  We are trying to verify if this 3.1 update prereqs another
**            install or update level of this same level of the product.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static boolean 
do_i_prereq_my_base_lev 
(
fix_info_type * update,
fix_info_type * prereq,
boolean       * prereq_wrong_format,
boolean       * warning,
boolean       * error
)
{
   requisite_list_type * requisite;
   boolean               prereq_found;

   prereq_found = FALSE;
   for (requisite = prereq->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
   {
      if (requisite -> type == A_PREREQ)
      {
         /* If we have a DUMMY_FIX as a prereq, then that means that we did not
            find any record of something that we are prereqing, this means we
            can not tell if we prereqed our base product or not because a
            DUMMY_FIX does not carry the version and release info. */

         if (requisite->fix_ptr->origin == DUMMY_FIX)
         {
            prereq_found = TRUE;
            break;
         }
         if (requisite -> fix_ptr -> origin == DUMMY_GROUP)
         {
            prereq_found |= do_i_prereq_my_base_lev (update,
                                                     requisite -> fix_ptr,
                                                     prereq_wrong_format,
                                                     warning,
                                                     error);
            RETURN_ON_ERROR;
         }
         else
         {
            if (strcmp (update -> name, requisite -> fix_ptr -> name) == 0)
            {
               if ( (update -> level.ver == requisite -> fix_ptr -> level.ver)
                                        &&
                    (update -> level.rel == requisite -> fix_ptr -> level.rel) )
               {
                  /*
                   * 4.1 update can't prereq non-41 base and vice versa.
                   */
                  if (((! IF_4_1 (update->op_type)) && 
                       (IF_4_1 (requisite->fix_ptr->op_type)))
                                    ||
                      (IF_4_1 (update->op_type) &&
                       ! (IF_4_1 (requisite->fix_ptr->op_type))))
                  {
                     *prereq_wrong_format = TRUE;
                     *error=TRUE;
                     break;
                  } /* end if */

                  /*
                   * 4.1 update can't prereq base/update with greater mod 
                   * and/or fix  (3.2 may have already done it so make that
                   * a warning.
                   */
                  if (IF_4_1(update->op_type) &&
                      ( (update->level.mod < requisite->fix_ptr->level.mod)
                                        ||
                      ((update->level.mod == requisite->fix_ptr->level.mod) &&
                       (update->level.fix < requisite->fix_ptr->level.fix))) )
                  {
                        *error=TRUE;
                        break;
                  }
                  prereq_found = TRUE;
                  break;
               }
               else
               {
                  /*
                   * update prereqs something with same name, but NOT with same
                   * version and release.  This is an error.
                   */
                  if (IF_4_1(update->op_type)) {
                     *error=TRUE;
                     break;
                  } else {
                     if (update->parts_applied == 0)
                        /*
                         * Flag as a warning if not already installed.
                         */
                        *warning=TRUE;
                     else
                        prereq_found = TRUE;
                     break;
                  }
               }
            }
         }
      } /* end if */
   } /* end for */
   if (prereq_found)
      return (TRUE);
   else
   {
      if (IF_4_1 (update->op_type) && !(*error))
      {
         /*
          * We didn't see an error above, yet we don't have a base level
          * prereq for a 4.1 update.  We should have built one in 
          * build_base_prereq.  Something must be wrong -- internal error. 
          */
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                  CKP_INT_CODED_MSG_E, CKP_INT_CODED_MSG_D), 
                  inu_cmdname, NO_PRQ_FOR_41_UPDT);
         * error = TRUE;
      }   
      return (FALSE);
   }

} /* end do_i_prereq_my_base_lev */

/*--------------------------------------------------------------------------*
**
** NAME: check_cross_prod_deps_caused_by_updates
**
** FUNCTION:  Searches for hard requisites between updates and updates or
**            base levels of other products.  Creates a prereq link between
**            the base level of the update and the "other product" if one
**            doesn't exist.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void
check_cross_prod_deps_caused_by_updates (boolean * error)
{
   criteria_type         criteria;
   fix_info_type       * base_of_req;
   fix_info_type       * base_of_update;
   fix_info_type       * fix;
   requisite_list_type * requisite;

   criteria = EMPTY_CRITERIA;

   for (fix = check_prereq.fix_info_anchor;
        fix != NIL (fix_info_type);
        fix = fix->next_fix)
   {
      if (IF_UPDATE(fix->op_type))
      {
         /*
          * Look at the "hard" requisites of this update, on pkgs with
          * different names which have some part installed.
          */
         for (requisite = fix->requisites;
              requisite != NIL (requisite_list_type);
              requisite = requisite->next_requisite)
         {
            if ((requisite->type == A_PREREQ || requisite->type == A_COREQ)
                                           &&
                (strcmp (fix->name, requisite->fix_ptr->name) != 0)
                                           &&
                (requisite->fix_ptr->parts_applied != 0))
            {
               /*
                * Get the base level of both the update and the requisite
                * (may already be a base level).
                */
               base_of_update = get_base_level (fix);
               base_of_req = get_base_level (requisite->fix_ptr);
               if (base_of_update && base_of_req) {
                  /*
                   * If the base levels don't have a hard dependency....
                   */
                  if (! is_B_req_of_A (base_of_update, base_of_req)) {
                     /*
                      * Remember the update's base level to aid in error  
                      * reporting.
                      * NOTE: The base_lev_list structure is intended to
                      *       facilitate a linked list for processing ifreqs. 
                      *       However, for our purposes, a single node 
                      *       will suffice.
                      */ 
                     requisite->base_lev_list = create_base_lev_list_node
                                                                     (error);
                     RETURN_ON_ERROR;
                     requisite->base_lev_list->fix_ptr = base_of_update;

                     /*
                      * Create the requisite between the base levels if
                      * we're doing auto-include (makes the rest of processing
                      * much easier).  Don't do it if no -g so that we can
                      * correctly detect that there is no req between the 
                      * base level at error reporting time.
                      */
                     if (check_prereq.Auto_Include) {
                        (void) link_requisite (base_of_update, base_of_req, 
                                       &criteria, requisite->type, error);
                        RETURN_ON_ERROR;
                     } /* end if */
                  }    /* end if */
               }       /* end if */
            }          /* end if */
         }             /* end for */
      }                /* end IF_UPDATE */
   }                   /* end for */
} /* check_cross_prod_deps_caused_by_updates */

/*--------------------------------------------------------------------------*
**
** NAME: get_implied_base_level
**
** FUNCTION:  Figures out the implied base level of the update argument
**            based on the level of the update.
**
** RETURNS:   This is a void function.
**            The implied level is returned in the Level_t struct.
**
**--------------------------------------------------------------------------*/

static void 
get_implied_base_level 
(
fix_info_type * update,
Level_t       * level,
boolean       * error
)
{
   char            print_string [MAX_LPP_FULLNAME_LEN];

   level->ver = update->level.ver;   /* Always use the version and release */
   level->rel = update->level.rel;   /*                      as the update. */
   level->fix = 0;                   /* <- Fix id always 0 */
   level->ptf[0] = '\0';             /* <- Not applicable */

   if (update->level.fix != 0)
      level->mod = update->level.mod;  /* Use mod from the update */ 
   else
   {
      if (update->level.mod != 0)  
      {
         level->mod = 0;               /* Base must have zero mod */
      }
      /* else mod == 0 and f==0 -- cannot happen, installp catches this. */

   } /* end if */
} /* get_implied_base_level */

/*--------------------------------------------------------------------------*
**
** NAME: update_has_expl_prereq_on_base
**
** FUNCTION:  Performs a mini-parse of the prereq file of the update
**            argument.  Looks for the name of the update in the
**            prereq file and if present, parses the subsequent dotted level. 
**            NOTE: The expression will be re-parsed during build_graph
**            (as a means of skipping over the prereq expression)
**            where and semantic error checks will be performed.
**
** RETURNS:   TRUE if the update has a valid prereq on its own option.
**            FALSE otherwise
**
**--------------------------------------------------------------------------*/

static boolean
update_has_expl_prereq_on_base
(
fix_info_type * update,
Level_t       * prq_level,
boolean       * error
)
{
   char * beginning_of_prq_expr;
   char   fix_name [MAX_LPP_FULLNAME_LEN];
   short name_count = 0;

   if ((beginning_of_prq_expr = 
        strstr (update->prereq_file, update->name)) == NIL (char))
      return (FALSE); 

   initialize_lexical_analyzer (update);
     
   /*
    * We'll assume that the prereq token was specified and start
    * parsing at the beginning of the name of the prereq.  If there
    * are any errors, they will be caught during the re-parse of the
    * requisite expression during the build_graph() phase.
    */
   parsers.prereq_file = beginning_of_prq_expr; 
   parsers.virtual_file = beginning_of_prq_expr;
   
   parsers.token = get_token (error);         /* get the name */
   RETURN_VALUE_ON_ERROR (FALSE);
  
   /*
    * Make sure the strstr didn't just find a substring of the name we
    * want.  Continue to parse the prereq string.  Do so until EOF so
    * that we can count the number of occurrences of reqs on the 
    * name sought for error checking purposes.
    */ 
   while (parsers.token != END_OF_FILE)
   {
      if ((parsers.token == A_NAME)                                 &&
          (strcmp (parsers.tokens_string_value, update->name) == 0))
      {
         if (name_count == 0)
         {
            parsers.token = get_token(error); /* get level */
            RETURN_ON_ERROR;
            if (parsers.token == A_DOTTED_NUMBER)
            {
               inu_level_convert (parsers.tokens_string_value, prq_level); 
            }
            else
            {
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                        CKP_ILLEGAL_LEVEL_FOR_BASE_E,
                        CKP_ILLEGAL_LEVEL_FOR_BASE_D), inu_cmdname,
                        get_fix_name_from_fix_info (update, fix_name),
                        beginning_of_prq_expr);   
               *error = TRUE;
               return (FALSE);
            } /* end if */
         }
         name_count++;
      }
      parsers.token = get_token (error);
      RETURN_VALUE_ON_ERROR (FALSE);
   } /* end while */

   if (name_count == 0) 
      return (FALSE);
   else
   {
      if (name_count > 1)
      {
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ,
                  CKP_TOO_MANY_REQS_ON_SAME_FILESET1_E,
                  CKP_TOO_MANY_REQS_ON_SAME_FILESET1_D),
                  inu_cmdname,
                  get_fix_name_from_fix_info (update, fix_name));
         *error = TRUE;
         return;
      }
   }
   /*
    * Make sure we have don't have base level that's lower than
    * the update's level.
    */
   if (inu_level_compare (&update->level, prq_level) <= 0)
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
               CKP_NO_PRQ_BASE_ERR_E, CKP_NO_PRQ_BASE_ERR_D), 
      inu_cmdname, get_fix_name_from_fix_info (update, fix_name));
      *error = TRUE;
      return (FALSE);
   }
   return (TRUE);
} /* update_has_expl_prereq_on_base */
