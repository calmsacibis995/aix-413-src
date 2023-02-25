static char sccsid[] = "@(#)72  1.22.1.23  src/bos/usr/sbin/install/ckprereq/fix_state_utils.c, cmdinstl, bos411, 9428A410j 6/23/94 11:41:36";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *
 * FUNCTIONS:
 *            figure_out_state_of_fix 
 *            inconsistent_state_error_check
 *            reset_state_of_medialess_fixes
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

extern consider_requisite_subgraph (
requisite_list_type * requisite
);

static void set_implicit_states_backend (
requisite_list_type * req_node
);

/*--------------------------------------------------------------------------*
**
** NAME: figure_out_state_of_fix
**
** FUNCTION:  Sets the apply, commit and reject states as appropriate for
**            the fix_info structure passed.  This routine is called as a
**            final step when the fix_info_table is loaded and combined from
**            its various sources.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
figure_out_state_of_fix 
 (
fix_info_type * fix,
boolean       * error
)
{
   short parts_not_yet_committed;
   short parts_not_yet_installed;
   char  fix_name[MAX_LPP_FULLNAME_LEN];

   /**************************************************************
    * RETURN for "pseudo-command line" node when ckprereq is called
    * from command line.  (ie. if this is the node that fakes the
    * command line node, its state is already set adequately).
    **************************************************************/ 
   if (fix -> origin == COMMAND_LINE && check_prereq.called_from_command_line)
       return;

   /**************************************************************
    * Set states of BROKEN packages and return.
    **************************************************************/ 
   if (fix -> apply_state == BROKEN)
   {
      fix -> commit_state = CANNOT_COMMIT;
      if (check_prereq.deinstall_submode) {
         /* 
          * It's ok to de-install broken packages. Set state according to
          * whether or not pkg was requested.
          */
         if (fix->origin == COMMAND_LINE)
            fix->reject_state = TO_BE_EXPLICITLY_REJECTED;
         else
            fix->reject_state = NOT_REJECTED;
      } else {
         fix -> reject_state = CANNOT_REJECT;
      }
      return;
   }

   /**************************************************************
    * Set initial apply state if fix's info came from VPD.
    **************************************************************/ 
   if (fix -> origin == VPD)
   {
      /*
       * NODE CAME FROM THE VPD.
       */
      if (fix -> parts_applied == 0)  /* no parts applied? */
      {
         fix -> apply_state = AVAILABLE;
         fix -> commit_state = CANNOT_COMMIT;
      }

      if (fix -> parts_committed == 0)
      {
         fix -> commit_state = UNCOMMITTED;
      }
   } /* end if */

   /******************************************************************
    * Set/modify apply state according to parts in fix, parts installed
    * and parts being operated on.
    ******************************************************************/
   if (fix -> parts_applied == fix -> parts_in_fix)
   {
      if (fix -> parts_applied == 0)
         fix->apply_state = AVAILABLE;
      else
         fix->apply_state = ALL_PARTS_APPLIED;
   }
   else
   {
      parts_not_yet_installed = fix -> parts_applied ^ fix -> parts_in_fix;

      if ((check_prereq.mode == OP_APPLY) && (fix -> origin != VPD))
      {
         if (check_prereq.Updates_Only_Install &&
             IF_INSTALL (fix -> op_type))
         {
            fix -> apply_state = CANNOT_APPLY;
            fix -> flags |= UPDATES_ONLY;
         }
         else
         {
            if ((check_prereq.parts_to_operate_on | parts_not_yet_installed)
                                       !=
                        (check_prereq.parts_to_operate_on))
            {
               fix -> apply_state = CANNOT_COMPLETE_APPLY;
            }
         } /* end if */
      }
      else
      {
         if (fix -> parts_applied != 0)
         {
            /* 
             * For root part applies, we want to leave the state as 
             * TO_BE_EXPLICITLY_APPLIED. 
             */
            if (fix -> apply_state != TO_BE_EXPLICITLY_APPLIED)
            {
               fix -> apply_state = PARTIALLY_APPLIED;
            }
         }

         /* 
          * Set the state to CANNOT_COMPLETE_APPLY if the parts not installed
          * don't match the parts to operate on AND if this is not a
          * superseded AVAILABLE fix. 
          */
         if ((check_prereq.mode == OP_APPLY)                                 &&
             ((parts_not_yet_installed | check_prereq.parts_to_operate_on) 
                                              !=
                                       (check_prereq.parts_to_operate_on))  &&
             !(fix -> apply_state == AVAILABLE                   &&
             fix -> superseding_ptf[0] != '\0'))
         {
            fix -> apply_state = CANNOT_COMPLETE_APPLY;
         }
      } /* end if */
   } /* end if */

   /******************************************************************
    * Set initial commit state of command line nodes.
    ******************************************************************/
   if (check_prereq.mode == OP_COMMIT && fix -> origin == COMMAND_LINE)
   {
      fix -> commit_state = TO_BE_EXPLICITLY_COMMITTED;
   }

   /******************************************************************
    * Set/modify commit state according to parts already committed,
    * parts in fix, and parts being operated on.
    ******************************************************************/
   if (fix -> parts_committed == fix -> parts_in_fix)
   {
      if (fix -> parts_committed == 0)
         fix -> commit_state = CANNOT_COMMIT;
      else
         fix -> commit_state = ALL_PARTS_COMMITTED;
   }
   else
   {
      /* 
       * Is this guy partially committed?  Do not change the state of something
       * that was on the command line. 
       */
      if ((fix -> parts_committed != 0) &&
          (fix -> origin != COMMAND_LINE))
      {
         /* 
          * If it's partially committed in the USR part AND we are doing USR
          * part commits ONLY, then set it's state to ALL_PARTS_COMMITTED with
          * respect to the operation. 
          */
         if ((fix -> parts_committed == LPP_USER)            &&
             (check_prereq.parts_to_operate_on == LPP_USER)  &&
             (check_prereq.mode == OP_COMMIT))
            fix -> commit_state = ALL_PARTS_COMMITTED;
         else
            fix -> commit_state = PARTIALLY_COMMITTED;
      }
   }

   switch (check_prereq.mode)
   {
      case OP_APPLY :
         break;

      case OP_COMMIT :
         /******************************************************************
          * Make FINAL COMMIT STATE ADJUSTMENTS based on states 
          * determined above, parts_in_fix and parts_to_operate_on.
          ******************************************************************/

         /* 
          * We can not commit anything that is not completely applied UNLESS
          * we are doing USR part commits AND the USR part is applied.
          * We only commit the parts that we have been told to.
          * We can not commit a ROOT part if the USER part is not committed.
          * We can commit a USER part without committing the ROOT part. 
          */
         if ((fix -> apply_state != ALL_PARTS_APPLIED) 
                                      &&
             !(check_prereq.parts_to_operate_on & LPP_USER  &&
               fix -> parts_applied == LPP_USER))
         {
            fix -> commit_state = CANNOT_COMMIT;
            break;
         }

         parts_not_yet_committed = fix -> parts_committed ^ fix -> parts_in_fix;

         switch (parts_not_yet_committed)
         {
            case LPP_ROOT :
              /* 
               * The root part of this product is not yet committed.  If we
               * weren't asked to do root parts AND we weren't asked to
               * commit USR parts while we have an applied USR part, we should
               * say that this guy's commit can't be completed. 
               */
              if (!(check_prereq.parts_to_operate_on & LPP_ROOT) &&
                  !(check_prereq.parts_to_operate_on & LPP_USER  &&
                  fix -> parts_applied & LPP_USER))
              {
                 fix -> commit_state = CANNOT_COMPLETE_COMMIT;
              }
              break;

            case LPP_USER :
              if ( !(check_prereq.parts_to_operate_on & LPP_USER))
                 fix -> commit_state = CANNOT_COMPLETE_COMMIT;
              break;

            case (LPP_ROOT | LPP_USER) :
              if ( !(check_prereq.parts_to_operate_on & LPP_USER))
                 fix -> commit_state = CANNOT_COMPLETE_COMMIT;
              break;

            case LPP_SHARE :
              if ( !(check_prereq.parts_to_operate_on & LPP_SHARE))
                 fix -> commit_state = CANNOT_COMPLETE_COMMIT;
              break;

          } /* end switch */
         break;

      case OP_REJECT :
         /******************************************************************
          * Set reject states based on apply and commit states
          * determined above, parts_in_fix and parts_to_operate_on.
          ******************************************************************/

         /* 
          * We can not reject anything that is not in the APPLIED state.
          * We can not reject a USER part if the ROOT part is applied.
          * A USER part can be rejected even if the ROOT part is not applied.
          * We can reject a ROOT part even if the USER part is in the
          * applied or committed state.
          * A ROOT part can be rejected even if the USER part is not applied.
          */
         fix -> reject_state = NOT_REJECTED;  /* Start positive! */

         /* 
          * You can't reject a part that is committed.  Nor can you reject
          * a USER part if the ROOT part is committed.  But you can reject
          * a ROOT part even if the USER part is committed. 
          */
         if (check_prereq.deinstall_submode) /* We can deinstall anything... */
         {
            /*
             * except bos.rte.  However, this state should not matter since
             * a bos.rte update (or base level) should never point to 
             * anything in another product with anything other than an
             * ifreq.
             */
            if (strcmp (fix->name, "bos.rte") == 0)
            {
               fix->flags |= FAILED_DEINST_CHECK;  /* Set reporting bit. */
               fix->reject_state = NOT_REJECTABLE; 
            }
         }
         else
         {
            /*
             * Set states based on commit state.
             */
            switch (fix -> parts_committed)
            {
               case LPP_ROOT :
                 fix -> reject_state = NOT_REJECTABLE;
                 break;

               case LPP_USER :
                 if ((fix -> parts_in_fix & LPP_ROOT) &&
                     ((check_prereq.parts_to_operate_on & ~ LPP_SHARE)
                                                               == LPP_ROOT))
                 {
                    break;  /* It's okay to reject just the ROOT PART. */
                 }
                 else
                 {
                    fix -> reject_state = NOT_REJECTABLE;
                 }
                 break;

               case (LPP_ROOT | LPP_USER) :
                 fix -> reject_state = NOT_REJECTABLE;
                 break;

               case LPP_SHARE :
                 fix -> reject_state = NOT_REJECTABLE;
                 break;

             } /* end switch */
         } /* end if */

         /*
          * Modify state based on apply state.
          */
         switch (fix -> parts_applied)
         {
            case 0 :
              fix -> reject_state = DONT_NEED_TO_REJECT;
              break;

            case LPP_ROOT :
              if (!(check_prereq.parts_to_operate_on & LPP_ROOT))
                 fix -> reject_state = NOT_REJECTABLE;
              break;

            case LPP_USER :
              if (check_prereq.parts_to_operate_on == LPP_ROOT)
                 fix -> reject_state = REJECTED;
              else
                 if (!(check_prereq.parts_to_operate_on & LPP_USER))
                    fix -> reject_state = NOT_REJECTABLE;
              break;

            case (LPP_ROOT | LPP_USER) :
              if (!(check_prereq.parts_to_operate_on & LPP_ROOT))
                 fix -> reject_state = NOT_REJECTABLE;
              break;

            case LPP_SHARE :
              if (!(check_prereq.parts_to_operate_on & LPP_SHARE))
                 fix -> reject_state = NOT_REJECTABLE;
              break;

         } /* end switch */

         /*
          * Set the state of command_line fixes on reject based on the
          * results of the above checks.
          */
         if (fix -> origin == COMMAND_LINE)
         {
            if (fix -> reject_state == NOT_REJECTED)
               fix -> reject_state = TO_BE_EXPLICITLY_REJECTED;
            else
               fix -> reject_state = CANNOT_REJECT;
         }

         break;

   } /* end switch */

   /* Do a little error checking. */
   inconsistent_state_error_check (fix, error);
   RETURN_ON_ERROR;

 } /* end figure_out_state_of_fix */

/*--------------------------------------------------------------------------*
**
** NAME: inconsistent_state_error_check
**
** FUNCTION:  Performs error checks for inconsistent/invalid states across
**            usr/root/share parts, with respect to the operation being
**            performed (eg. Can't commit something with an applied root part
**            only).  This is called from merge_vpd_and_option_lists () AND
**            from figure_out_state_of_fix ().  The former call is made in some
**            cases when the merge process deletes some info that is
**            vital to the error checking.
**
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
inconsistent_state_error_check 
 (
fix_info_type * fix,
boolean       * error
)
{

   char  fix_name[MAX_LPP_FULLNAME_LEN];

   /* Make sure that we don't have a SHARE part marked with anything
      else. */

   #pragma covcc !instr
   if (( (fix -> parts_in_fix & LPP_SHARE) &&
                                     (fix -> parts_in_fix != LPP_SHARE)) ||
        ((fix -> parts_applied & LPP_SHARE) &&
                                   (fix -> parts_applied != LPP_SHARE))  ||
        ((fix -> parts_committed & LPP_SHARE) &&
                                   (fix -> parts_committed != LPP_SHARE)))
   {
      get_fix_name_from_fix_info (fix, fix_name);

      /* The Software Vital Product Data database indicates
            that %s has a SHARE and ROOT PART
            Use local problem reporting procedures. */

      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_SWVPD_2PARTS_E,
               CKP_SWVPD_2PARTS_D), inu_cmdname, fix_name);
      check_prereq.function_return_code = ILLEGAL_SHARE_PART_RC;

      /* Don't set the error flag if we're being called by a "listing program"
         (installp -l  or lslpp). */

      if (! check_prereq.called_from_ls_programs)
      {
         * error = TRUE;
         return;
      }
   }
   #pragma covcc instr

   if (fix -> parts_applied == LPP_ROOT)
   {
      /* USR part AVAILABLE, ROOT part APPLIED or COMMITTED */

      if (check_prereq.mode != OP_REJECT)
      {
         get_fix_name_from_fix_info (fix, fix_name);
         if (fix -> parts_committed == 0)
         {
            /* USR part AVAILABLE, ROOT part APPLIED. */

            fix->flags |= WARN_MISS_USR_APPLD_ROOT;

            /* Let's generate an entry on the warning index list, if -v
               flag was specified to installp. */
            if ((fix -> origin != COMMAND_LINE)         &&
                (!check_prereq.called_from_ls_programs) &&
                (check_prereq.instp_verify)) {
                add_entry_to_index_list (report_anchor.misc_warnings_hdr,
                                         fix, NIL (Option_t),
                                         &EMPTY_CRITERIA, error);
                RETURN_ON_ERROR;
            }

            if (check_prereq.called_from_ls_programs) {
               fix -> apply_state = PARTIALLY_APPLIED;
            } else {
               fix -> apply_state = CANNOT_APPLY;
            }
         }
         else
         {
            /* USR part AVAILABLE, ROOT part COMMITTED */

            fix->flags |= WARN_MISS_USR_COMTD_ROOT;
            if ((fix -> origin != COMMAND_LINE)         &&
                (!check_prereq.called_from_ls_programs) &&
                (check_prereq.instp_verify)) {
               add_entry_to_index_list (report_anchor.misc_warnings_hdr,
                                        fix, NIL (Option_t),
                                        &EMPTY_CRITERIA, error);
               RETURN_ON_ERROR;
            }

            if (!check_prereq.Force_Install) {
               if (check_prereq.called_from_ls_programs) {
                  fix -> apply_state = PARTIALLY_APPLIED;
               } else {
                  fix -> apply_state = CANNOT_APPLY;
               }
            }
         }
      }
   }
   else
   {
      if (fix -> parts_committed == LPP_ROOT &&
          check_prereq.called_from_ls_programs)
      {
         /* USR part APPLIED, ROOT part COMMITTED */

         fix -> apply_state = PARTIALLY_APPLIED;
      }
   }

} /* end inconsistent_state_error_check */

/*--------------------------------------------------------------------------*
**
** NAME: reset_state_of_medialess_fixes
**
** FUNCTION:  This function resets the state of a medialess fix from
**            ALL_PARTS_APPLIED to TO_BE_EXPLICITLY_APPLIED.
**            It's sole purpose is to enable installp to make correct
**            decisions when it performs its pseudo-requisite checking
**            operation.  (determine_order_of_fixes temporarily set it
**            to ALL_PARTS...)
**
** RETURNS:   void function
**
**--------------------------------------------------------------------------*/

void 
reset_state_of_medialess_fixes ()
{
   fix_info_type * fix;

   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      if ((fix -> apply_state == ALL_PARTS_APPLIED)
                                  &&
           ((fix -> parts_applied != fix -> parts_in_fix) ||
            (fix -> flags & AUTO_C_UPDATE)))
      {
         fix -> apply_state = TO_BE_EXPLICITLY_APPLIED;
      }
    } /* end for */

} /* reset_state_of_medialess_fixes */

/*--------------------------------------------------------------------------*
**
** NAME: set_implicit_states
**
** FUNCTION:  This function traverses the graph setting states of pkgs being
**            pulled in with -g to "TO_BE_IMPLICITLY_XXXXXX" for apply/
**            commit/reject/de-install ops.  
**
** RETURNS:   void function
**
**--------------------------------------------------------------------------*/

void 
set_implicit_states ()
{
   requisite_list_type * requisite;

  /*
   * The whole purpose of this routine is to facilitate better error reporting
   * for the following scenario:
   *
   *    A --prereq--> C <--prereq-- D
   *    |
   * prereqs
   *    |
   *    v
   *    B
   *  
   * If A and D are requested for installation, and B is the cause for A
   * failing, yet C and D will be installed (C by virtue of -g and D),
   * then we need to know that C is not part of the cause for A's failure.
   * This is achieved by setting the state of C to TO_BE_IMPLICITLY_APPLIED
   * (without this routine, it would remain as IN_TOC, and be reported as 
   * part of the reason for A's failure).  
   *  
   */

  if (! check_prereq.Auto_Include)  /* Don't bother if no -g */
     return;

  /*
   * Loop through the requisites of the anchor (pkgs which were requested)
   * and traverse the subgraph of each failed node.
   */
   for (requisite = check_prereq.fix_info_anchor->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      /*
       * Only look at successful paths.
       */
      if (requisite->fix_ptr->flags & SUCCESSFUL_NODE)
      {    
         set_implicit_states_backend (requisite);
      }
   } /* for */

   /*
    * SUBGRAPH_VISTED bits were set in the recursion.
    */
   unmark_graph (SUBGRAPH_VISITED);

} /* set_implicit_states */

/*--------------------------------------------------------------------------*
**
** NAME: set_implicit_states_backend
**
** FUNCTION:  This function traverses the subgraph headed by fix setting 
**            states of pkgs being pulled in with -g to 
**            "TO_BE_IMPLICITLY_XXXXXX" for apply/commit/reject/de-install ops. 
**
** RETURNS:   void function
**
**--------------------------------------------------------------------------*/

static void 
set_implicit_states_backend 
(
requisite_list_type * req_node
)
{
   requisite_list_type * requisite;
   fix_info_type       * fix = req_node->fix_ptr;

   /*
    * Cycle detection.
    */
   if (fix->flags & (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED))
      return;
   fix->flags |= VISITING_THIS_SUBGRAPH;

   /*
    * Set state if the node is an "_AUTO_" pkg.  The macros used 
    * are defined in check_prereq.h and are used elsewhere in evaluate_graph.c
    * to determine if a pkg is "auto-includable."
    */
   switch (check_prereq.mode)
   {
      case OP_APPLY :
         if (IS_AUTO_APPLIABLE (fix) && req_node->type != AN_INSTREQ)
            fix->apply_state = TO_BE_IMPLICITLY_APPLIED;
         break;

      case OP_COMMIT :
         if (IS_AUTO_COMMITTABLE (fix))
            fix->commit_state = TO_BE_IMPLICITLY_COMMITTED;
         break;

      case OP_REJECT :
         if (IS_AUTO_REJECTABLE (fix))
            fix->reject_state = TO_BE_IMPLICITLY_REJECTED;
         break;

   } /* end switch */

   /* 
    * Recurse if subgraph is a valid requisite (ie.  not a "dead-end" ifreq
    * subgraph -- ifreq link where base level is not applied.)  See file
    * evaluate_graph.c for details.
    */
   for (requisite = fix->requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite->next_requisite)
   {
      if (consider_requisite_subgraph (requisite))
         set_implicit_states_backend (requisite);
   } /* end for */

   /*
    * Cycle detection.
    */
   fix->flags |= SUBGRAPH_VISITED;
   fix->flags &= ~VISITING_THIS_SUBGRAPH;

} /* set_implicit_states */
