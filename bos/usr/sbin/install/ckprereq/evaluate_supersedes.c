static char sccsid[] = "@(#)61  1.13  src/bos/usr/sbin/install/ckprereq/evaluate_supersedes.c, cmdinstl, bos411, 9437B411a 9/11/94 16:02:30";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: evaluate_superseding_updates
 *            check_for_broken_superseded_updts
 *            pre_process_sups_pair
 *            process_supersedes_pair
 *            process_anomalies
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

#include <check_prereq.h>

#define IS_BASE_OF_EXPL_SUPS_CHAIN(fix)                                    \
          ((fix -> superseded_by != NIL (supersede_list_type)) ||          \
           (fix -> supersedes == NIL (supersede_list_type)))

#define IS_BASE_OF_IMPL_SUPS_CHAIN(fix)                                     \
          ((fix->next_fix != NIL (fix_info_type))           &&              \
           (IF_UPDATE(fix->next_fix->op_type))              &&              \
           (strcmp (fix->name, fix->next_fix->name) == 0)   &&              \
           (! A_IS_PREREQ_OF_B(fix, fix->next_fix)))

#define IS_MOD_BUMP(old, new)                                             \
          ((old->level.ver == new->level.ver) &&                          \
           (old->level.rel == new->level.rel) &&                          \
           (old->level.mod < new->level.mod))

static void mark_sup_of_broken_updt (
fix_info_type * sup_of_broken,
boolean         ok_to_fix_brok);

static void 
check_for_broken_superseded_updts (void);

static void 
pre_process_sups_pair (
fix_info_type  * old,
fix_info_type  * new,
fix_info_type ** latest_explicit,
fix_info_type ** latest_sup,
boolean        * usr_only_seen,
boolean        * usr_root_seen,
boolean        * share_only_seen,
boolean        * anomalous_supersedes_flag,
boolean        * error
);

static void process_supersedes_pair (
fix_info_type * old,
fix_info_type * new,
fix_info_type * latest_epxlicit,
boolean       * processed_latest_explicit,
boolean       * error);

static void process_anomalies (
fix_info_type * old,
fix_info_type * new,
fix_info_type * latest_explicit,
fix_info_type * latest_supersede,
boolean       * processed_latest_explicit,
boolean       * error);

/*--------------------------------------------------------------------------*
**
** NAME:      evaluate_superseding_updates
**
** FUNCTION:  Makes preliminary, "up-front" decisions about what updates to 
**            apply/commit/reject in a chain of superseding_fixes.  This
**            routine is called prior to build_graph so that we know 
**            precisely which superseding_fix to pick from a chain when
**            another update prereqs any fix from the chain.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
evaluate_superseding_updates
(
boolean * error
)
{
   boolean               anomalous_supersedes_flag; /* set if "unusual" 
                                                       supersedes processing 
                                                       should occur */ 
   boolean               processed_latest_expl;

   boolean             usr_only_seen;
   boolean             usr_root_seen;
   boolean             share_only_seen;
   fix_info_type       * fix;
   fix_info_type       * new;
   fix_info_type       * old;
   fix_info_type       * old2;
   fix_info_type       * latest_explicit;  /* newest node in chain that has
                                              been explicitly requested. */
   fix_info_type       * latest_superseding_fix; /* newest node in chain. */
   supersede_list_type * supersede;

   /*
    * Rules:
    *       1. For apply operations:
    *          if (one or more fixes from a chain are explicitly requested)
    *             if (we're doing "auto_supersedes" -- currently specified
    *                 by the auto_include flag -g)
    *             {
    *                pick the latest/newest supersedes in the chain
    *             }
    *             else
    *             {
    *                pick the latest/newest explicitly requested
    *                if (there are newer ptfs than the latest explicit)
    *                   convert the supersedes links to prereq links
    *                if (there are older ptfs than the latest explicit)
    *                   mark the older ptfs as superseded/unusable
    *             }
    *          else
    *             if (we're doing auto_supersedes)
    *                mark all but the latest as superseded/unusable
    *             else
    *                convert supersedes links in the chain to prereq links
    *
    *          Special Anomalous processing:
    *            -- partially applied superseded ptfs will be converted to
    *               requisites of their superseding ptf (to ensure that
    *               root parts will be applied for consistency -- diskless)
    *  
    *       2. For commit/reject/de-install
    *          Make every ptf prereq its installed superseded ptf.  We do
    *          this because superseded ptfs must be committed before their
    *          superseding fix (as if they were prereqs) and in the case
    *          of reject, the prereq link will ultimately be reversed so that
    *          the superseding fix must be rejected first.
    */

   /*
    * Initialize a list used for reporting requested updates that are
    * being "preempted" for newer superseding updates.
    */
   init_sop_index (&report_anchor.superseded_index_hdr,
                  &report_anchor.superseded_index_tail,
                  error);
    RETURN_ON_ERROR;

   /*
    * Scan the fix_info table looking for BROKEN updates.  If there are any, 
    * mark them so that superseding updates can be installed implicitly on top 
    * of them OR, so that superseding updates already installed on top of
    * them can be committed.  Only mark them that way for the apply case when 
    * the correct flags are switched on (-acgN).
    * (Currently, updates cannot be explicitly installed on top of BROKEN pkgs 
    * due to up-front checks by installp which prevents installation of *any*
    * pkg associated with an update containing a BROKEN ptf.  That functionality
    * can be added here at a later date since those packages have been 
    * flagged and reside on the failsop passed to inu_ckreq). 
    */
   check_for_broken_superseded_updts ();

   /*
    * Loop through the fix info table looking for unexamined nodes that are 
    * "at the base" of a "good" supersedes chain.  These are nodes that:
    *       1. are not a member of some other supersedes chain which has
    *          been VISITED
    *       2. are distinguished as being at the base of an explicit or 
    *          implicit supersedes chain: are superseded and supersede no
    *          others. 
    *       3. have not been "disabled" by processing in evaluate_base_levels().
    */ 
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      
      if ((IF_INSTALL (fix->op_type))                          
                        ||
          (fix -> flags & VISITING_THIS_NODE)                  
                        ||
          ((!IS_BASE_OF_EXPL_SUPS_CHAIN(fix)) &&
           (!IS_BASE_OF_IMPL_SUPS_CHAIN(fix)))                 
                        ||
          (fix->apply_state == SUPD_BY_NEWER_BASE)             
                        ||
          (fix->apply_state == TO_BE_OVERWRITTEN_BY_SUP_BASE)  
                        ||
          (fix->apply_state == CANNOT_APPLY_CONFL_LEVEL)       
                        ||
          (fix->apply_state == TO_BE_OVERWRITTEN_BY_CONFL_LEVEL))
         continue;

       anomalous_supersedes_flag = FALSE;

       /*
        * Prime some variables used to see if we have mis-matching parts in
        * a supersedes chain.
        */
       usr_only_seen=FALSE;
       usr_root_seen=FALSE;
       share_only_seen=FALSE;
       check_for_illegal_superseding_parts(fix, &usr_only_seen, &usr_root_seen,
                                           &share_only_seen);
      /*
       * Find latest and latest requested in this chain of superseding updates.
       * NOTE:
       * 3.1/3.2 updates are linked by supersedes nodes.  
       * Supersedes relationships between 4.1 updates are implicit.  3.1 
       * updates could have followed many of the ground rules set by implicit 
       * supersedes but not all of them, since no assumption can be made about 
       * implicit vs explicit prereqs in 3.1.  As a result, they are still 
       * treated as always and the only real supersedes processing that occurs 
       * is prevention of allowing two requested 3.1 updates from installing in
       * the same session (ie. elimination of the older).
       */

      latest_explicit = NIL (fix_info_type);
      latest_superseding_fix = NIL (fix_info_type);
      if (! IF_4_1 (fix->op_type))
      {            
         for (supersede = fix->superseded_by;
              supersede != NIL (supersede_list_type);
              supersede = supersede->superseding_fix->superseded_by)
         {
             pre_process_sups_pair (supersede->superseded_fix,
                                    supersede->superseding_fix,
                                    &latest_explicit,
                                    &latest_superseding_fix,
                                    &usr_only_seen,
                                    &usr_root_seen,
                                    &share_only_seen,
                                    &anomalous_supersedes_flag,
                                    error);
             RETURN_ON_ERROR;
         } /* end for */
      }
      else
      {
         /*
          * Process implicitly superseding update pairs.
          */
         old = fix;
         new = fix->next_fix;
         while (IS_STILL_SUPS_CHAIN_LR_SCAN(old, new))
         {
            pre_process_sups_pair (old, new, &latest_explicit,
                                   &latest_superseding_fix,
                                   &usr_only_seen,
                                   &usr_root_seen,
                                   &share_only_seen,
                                   &anomalous_supersedes_flag, error);
            old = new;
            new = new->next_fix;

         } /* end while */

      } /* end if */

      if (check_prereq.mode != OP_APPLY)
         /*  
          * We've done our work for this supersedes chain.
          */  
         continue;

      /*
       * If there was an explicitly requested superseding node, and we are
       * doing auto supersedes, make sure we set/reset the latest_explicit
       * to the latest superseding node.  (This permits fewer conditions to
       * check in calls to the next two routines.)
       */
      if (check_prereq.Auto_Supersedes &&
         latest_explicit != NIL (fix_info_type))
      {
         latest_explicit = latest_superseding_fix;
         latest_explicit->apply_state = TO_BE_EXPLICITLY_APPLIED;
         latest_explicit->origin = COMMAND_LINE;
      }

      /*
       * Now that we know information about this supersedes chain,
       * lets process it (just means marking all but the latest 
       * as superseded and reflecting who supersedes who in the
       * failsop -- for reporting purposes).
       */
      processed_latest_expl = FALSE;
      if (! IF_4_1 (fix->op_type))
      {
         for (supersede = fix->superseded_by;
              supersede != NIL (supersede_list_type);
              supersede = supersede->superseding_fix->superseded_by)
         {
            process_supersedes_pair (supersede->superseded_fix, 
                                     supersede->superseding_fix,
                                     latest_explicit, 
                                     &processed_latest_expl, error);
            RETURN_ON_ERROR;
         } /* end for */
      }
      else
      {
         old = fix;
         new = fix->next_fix;
         while (IS_STILL_SUPS_CHAIN_LR_SCAN(old, new))
         {
            process_supersedes_pair (old, new, latest_explicit,
                                     &processed_latest_expl, error);

            /*
             * Bumps in mod level for updates imply a supersedes of
             * all previous updates, which are not prereqed by that mod bump.
             * Make sure we re-visit all older pkgs to ensure that
             * previous "mod level bumps", which may be part of a different
             * implicit supersedes chain,  are processed with respect to
             * the latest superseding fix (if it had a mod level increase). 
             */ 
            if (((check_prereq.Auto_Supersedes && 
                  new == latest_superseding_fix) ||
                 (new == latest_explicit))
                                &&
                (IS_MOD_BUMP(old, new)))
            {
               old2 = old->prev_fix; 
               /*
                * Scanning from Right to Left.
                */
               while (IS_STILL_SUPS_CHAIN_RL_SCAN(old2, new))
               {
                  process_supersedes_pair (old2, new, latest_explicit,
                                           &processed_latest_expl, error);
                  old2 = old2->prev_fix;

               } /* end while */

            } /* end if */
            old = new;
            new = new->next_fix;

         } /* end while */

      } /* end if */

      /*
       * Process the anomalies that we may have detected above.
       * Again, we need to take explicit and implicit supersedes
       * chains into account.
       */
      if (anomalous_supersedes_flag)
      {
         processed_latest_expl = FALSE;
         if (! IF_4_1(fix->op_type))
         {
            for (supersede = fix->superseded_by;
                 supersede != NIL (supersede_list_type);
                 supersede = supersede->superseding_fix->superseded_by)
            {
               process_anomalies (supersede->superseded_fix,
                                  supersede->superseding_fix,
                                  latest_explicit, 
                                  latest_superseding_fix, 
                                  &processed_latest_expl,
                                  error);
               RETURN_ON_ERROR;

            } /* for */
         }
         else
         {
            old = fix;
            new = fix->next_fix;
            while (IS_STILL_SUPS_CHAIN_LR_SCAN(old, new))
            {
               process_anomalies (old, new,
                                  latest_explicit, 
                                  latest_superseding_fix, 
                                  &processed_latest_expl,
                                  error);
               RETURN_ON_ERROR;
               old = new;
               new = new->next_fix;

            } /* end while */
         } /* end if */
      } /* end if */
     
   } /* end for (fix...*/

   unmark_graph (VISITING_THIS_NODE);

} /* evaluate_superseding_udpates */

/**--------------------------------------------------------------------------*
**
** NAME:      pre_process_sups_pair
**
** FUNCTION:  Compares the old and new superseding packages passed as
**            arguments.  Looks for the latest and latest explicitly 
**            requested of the two.  Handles both explicit (3.1/3.2) and
**            implicit (4.1) supersedes chains.
**
** RETURNS:   This is a void function.
**--------------------------------------------------------------------------*/

static void 
pre_process_sups_pair
(
fix_info_type  * old,
fix_info_type  * new,
fix_info_type ** latest_explicit,
fix_info_type ** latest_superseding_fix,
boolean        * usr_only_seen,
boolean        * usr_root_seen,
boolean        * share_only_seen,
boolean        * anomalous_supersedes_flag,
boolean        * error
)
{
   boolean         do_link;
   char            fix_name[MAX_LPP_FULLNAME_LEN];
   fix_info_type * tmp_older;

   old->flags |= VISITING_THIS_NODE;

   /*
    * check that new does not superseede an older pkg with mismatching
    * parts (usr supersede usr-root for example).  Only do so if
    * new is not yet installed (since it could be a pre-existing problem
    * on a migrated machine.
    */
   if ((new->parts_applied == 0) &&
       (check_for_illegal_superseding_parts(new, usr_only_seen, 
                                            usr_root_seen, share_only_seen)))
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_ILLEGAL_SUPS_PARTS_E,
              CKP_ILLEGAL_SUPS_PARTS_D), inu_cmdname, 
              get_fix_name_from_fix_info (new, fix_name));
      *error = TRUE;
      return;
   }
   if ((check_prereq.mode != OP_APPLY) && (! IS_SUPERSEDED(old))) 
   {

#ifdef FUTURE_FUNCTION_FOR_DISKLESS_SUPS
      do_link = FALSE;
      if ((check_prereq.parts_to_operate_on == LPP_ROOT) && 
          USR_ROOT_PKG_W_NO_ROOT_INSTLD (old))
      {
         if (check_prereq.mode == OP_COMMIT)
         {
            if (! old->parts_committed == LPP_USER) 
               do_link = TRUE;
            /* UNFINISHED */
            /* Consider whether or not old should be made to look like it's
               superseded.  If other option reqs in to this node, req
               should probably only be satisfied by any roots that may be
               above this, as it would on apply.  May need a 
               seen_partially_applieds variable, so that if latest or 
               latest_explicit is a USR-ROOT, we mark all older partially
               applieds as SUPERSEDED so that req gets resolved to usr-root.
               Also, when usr-root is found, need to make sure previous 
               usr-root is linked to it, like for reject below.      
             */ 
         }
         else
         {
            if (new->parts_in_fix == (LPP_USER | LPP_ROOT))
            {
               old2 = old->prev_fix;
               new2 = old;
               while (IS_STILL_SUPS_CHAIN_RL_SCAN(old2, new2))
               {
                  if (old2->parts_applied & LPP_ROOT)
                  {
                     tmp_older = old2;
                     do_link = TRUE;
                     break;
                  }
                  new2 = old2;
                  old2 = old2->prev_fix;

               } /* end while */
            } /* end if */

         } /* end if */
      }
      else
      {
         do_link = TRUE;
         tmp_older = old;
      }
#else
    do_link = TRUE;
    tmp_older = old;
#endif

      if (do_link)
      {
         (void) link_requisite (new, tmp_older, & EMPTY_CRITERIA, 
                                A_PREREQ, error);
         RETURN_ON_ERROR;
      }
   }
   else
   {
      /* 
       * FUTURE_FUNCTION: Omit this check when no longer considered an anomaly.
       */
      if (USR_ROOT_PKG_W_NO_ROOT_INSTLD (old))
      {
         (* anomalous_supersedes_flag) = TRUE;
      }

      if (old->apply_state == TO_BE_EXPLICITLY_APPLIED)
      {
         /*
          * This is the latest explicit so far.
          */
         (* latest_explicit) = old;
      }

      /*
       * Check if this is the newest in the chain.
       */
      if (! IF_4_1 (old->op_type))
      {
         if (new->superseded_by == NIL (supersede_list_type))
         {
            (* latest_superseding_fix) = new;
            new->flags |= VISITING_THIS_NODE;
         } /* end if */
      }
      else
      {
         if ((new->next_fix == NIL(fix_info_type)) 
                        ||
             (strcmp (new->name, new->next_fix->name) != 0) 
                        ||
             (IF_INSTALL (new->next_fix->op_type))       
                        || 
             (A_IS_PREREQ_OF_B(new, new->next_fix)))
          {
             (* latest_superseding_fix) = new;
             new->flags |= VISITING_THIS_NODE;
          }
      }

      /*
       * Check if latest is also the latest explicitly requested.
       */
      if ((*latest_superseding_fix) != NIL (fix_info_type))
      {
         if ((*latest_superseding_fix)->apply_state == TO_BE_EXPLICITLY_APPLIED)
             (*latest_explicit) = (*latest_superseding_fix);
         /*
          * Repeat the illegal supersedes check on the latest (since we won't
          * come back to this routine for it.)
          */
         if (((*latest_superseding_fix)->parts_applied == 0) &&
             (check_for_illegal_superseding_parts (*latest_superseding_fix, 
                                                   usr_only_seen,
                                                   usr_root_seen, 
                                                   share_only_seen)))
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_ILLEGAL_SUPS_PARTS_E,
                    CKP_ILLEGAL_SUPS_PARTS_D), inu_cmdname,
                    get_fix_name_from_fix_info (*latest_superseding_fix, 
                                                fix_name));
            *error = TRUE;
            return;
         }
      }
   } /* end if */

} /* pre_process_sups_pair */

/**--------------------------------------------------------------------------*
**
** NAME:      process_supersedes_pair
**
** FUNCTION:  Considers the pair of fixes "old" and "new" representing a
**            superseded and its superseding update.  Makes decision 
**            regarding whether or not the older needs to be marked
**            as superseded so that it is never used when resolving requisites.
**            (The older should never be applied in the same installation
**            as the newer.)
**            The goal is to mark the "older" in a pair as being superseded
**            by the "newer", IF the newer is the latest in a chain of 
**            superseding fixes OR if the newer is the latest explicitly
**            requested in a chain (the two differ when Auto Supersedes
**            flag -g is not specified).
**
** RETURNS:   This is a void function.
**--------------------------------------------------------------------------*/

static void 
process_supersedes_pair
(
fix_info_type     * old,
fix_info_type     * new,
fix_info_type     * latest_expl_request,
boolean           * processed_latest_expl,
boolean           * error
)
{
   char                  sup_string[MAX_LPP_FULLNAME_LEN];
   Option_t            * sop_ptr;

   /* 
    * THIS CHECK WILL BE REMOVED WHEN NEW DISKLESS SUPERSEDES FUNCTION
    * IS IMPLEMENTED.
    * First, check to see if this is a superseded update that requires 
    * special handling.  Ignore it if so.
    */
   if (USR_ROOT_PKG_W_NO_ROOT_INSTLD (old))
   {
      if (old == latest_expl_request)
      {
         (*processed_latest_expl) = TRUE;
      }
      return;
   }

   if (old->apply_state == ALL_PARTS_APPLIED)
      return;

   if (latest_expl_request != NIL (fix_info_type))
   {
      /* 
       * EXPLICTLY REQUESTED UPDATE IN THE CHAIN....
       */
      if (old == latest_expl_request)
      {
         /* 
          * This is our latest eplicitly requested fix in the supersedes
          * chain.  (NOTE: This is NOT the latest superseding fix in the 
          * chain, since it's the older of a supersedes pair, therefore we
          * must not be doing auto supersedes).  
          */
         (*processed_latest_expl) = TRUE;
      }
      else
      {
         /*
          * THIS IS NOT THE LATEST EXPLICIT FIX....
          */
         if (old->apply_state == TO_BE_EXPLICITLY_APPLIED)
         {
            /* 
             * THIS FIX WAS REQUESTED BY THE USER....
             * Set a flag indicating that we did auto supersede a pkg.
             * (This helps with success/failure reporting).
             */
            latest_expl_request->flags |= AUTO_SUPERSEDES_PKG;

            /* 
             * Mark it as unusable.
             */
            old->apply_state = SUPD_BY_NEWER_UPDT;
            old->origin      = IN_TOC;

            /*
             * Move its SOP entry to the Fail_SOP for installp reporting 
             * purposes.  Also, add it to an index list for reporting by
             * inu_ckreq.
             */
            sop_ptr = move_failed_sop_entry_to_fail_sop (old,
                                                     STAT_TO_BE_SUPERSEDED);
             /*
              * Unset the selected bit.  This helps installp print its
              * STATS table.
              */
            sop_ptr->flag = ~SET_SELECTED_BIT(~(sop_ptr->flag));
 
            add_entry_to_index_list (report_anchor.superseded_index_hdr,
                                 NIL (fix_info_type), sop_ptr, &EMPTY_CRITERIA,
                                 error);
            RETURN_ON_ERROR;

            /*
             * Remember who superseded it (need to use level info for non
             * 3.2 updates).
             */
            if (IF_3_2 (sop_ptr->op_type) && IF_UPDATE (sop_ptr->op_type))
               strcpy (sup_string, latest_expl_request->level.ptf); 
            else
               get_level_from_fix_info (latest_expl_request, sup_string);

            /*
             * Free what may already be there (this is info about who
             * this ptf supersedes.
             */
            if (sop_ptr->supersedes != NIL (char))
               my_free (sop_ptr->supersedes);
            sop_ptr->supersedes = strdup (sup_string); 
         }
         else
         {
            /* 
             * THIS FIX WAS NOT EXPLICITLY REQUESTED....
             * If it's on the media and its older than the latest explicit
             * Mark it as unusable.
             */
            if ((old->apply_state == IN_TOC) && (!*processed_latest_expl))
/********** HERE1 ***********/
                old->apply_state = SUPD_BY_NEWER_UPDT;

         }/* end if */
      } /* end if */
   }
   else
   {
      /* 
       * NO EXPLICITLY REQUESTED UPDATE IN THE CHAIN....  If we're
       * doing auto supersedes, mark all but the newest as "unusable."
       */
       if ((old->apply_state == IN_TOC) && (check_prereq.Auto_Supersedes))
          old->apply_state = SUPD_BY_NEWER_UPDT;

   } /* end if */
/********** HERE2 ***********/

#ifdef FUTURE_FUNCTION_FOR_DISKLESS_SUPS_PROCESSING
 ****************NOTE*****************
 REPLACE HERE1 TO HERE2 WITH THE FOLLOWING WHEN IMPLEMENTING NEW DISKLESS
 SUPERSEDES FUNCTION:
                                        ||
                        USR_ROOT_PKG_W_NO_ROOT_INSTLD (old)*/)
                                       &&
                        (! (*processed_latest_expl)
                {
                      old->apply_state = SUPD_BY_NEWER_UPDT;
                } /* end if */
             }/* end if */
          } /* end if */
       }
       else
       {
          /*
           * NO EXPLICITLY REQUESTED UPDATE IN THE CHAIN....  If we're
           * doing auto supersedes, mark all but the newest as "unusable."
           */
           if ((old->apply_state == IN_TOC) /* ||
               (USR_ROOT_PKG_W_NO_ROOT_INSTLD (old) */)
                              &&
              (check_prereq.Auto_Supersedes)
           {
              old->apply_state = SUPD_BY_NEWER_UPDT;
           }
       } /* end if */
   } /* end for */
#endif

} /* process_supersedes_pair */

/*--------------------------------------------------------------------------*
**
** NAME:      process_anomalies
**
** FUNCTION:  Scans the superseded_by list of "oldest_fix" looking for 
**            superseded nodes that need special handling.  These nodes
**            include:
**               -- partially applied superseded fixes (needing root part apply)
**               -- fixes in a chain of MC type updates.
**            These nodes are linked to "some" superseding fix as a prereq.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void process_anomalies 
(
fix_info_type * old,
fix_info_type * new,
fix_info_type * latest_explicit,
fix_info_type * latest_supersede,
boolean       * processed_latest_expl,
boolean       * error
)
{ 
   fix_info_type       * chosen_supersedes;

   if (old == latest_explicit)
      (* processed_latest_expl) = TRUE;

   if (USR_ROOT_PKG_W_NO_ROOT_INSTLD(old))
   {
      /* 
       * A SUPERSEDED FIX NEEDS ITS ROOT PART APPLIED.... (for usr-root
       * consistency).  Let's find out which superseding fix should be
       * the parent (for the prereq link).
       */
      if (USR_ROOT_PKG_W_NO_ROOT_INSTLD(new))
      {
         /*
          * IMMEDIATE SUPERSEDING FIX NEEDS ITS ROOT PART APPLIED ALSO....
          * we'll use it as the parent.
          */
         chosen_supersedes = new;
      }
      else
      { 
         if (latest_explicit != NIL (fix_info_type) )
         {
            /* 
             * A SUPERSEDING FIX IN THIS CHAIN WAS EXPLICITLY REQUESTED...
             * pick either the immediate supersede (if this is newer than 
             * the latest explict and we're NOT doing auto supersedes),
             * the latest supersede or the latest explicit fix. 
             */ 
            if (*processed_latest_expl)
               if (check_prereq.Auto_Supersedes)
                  chosen_supersedes = latest_supersede;
               else
                  chosen_supersedes = new;
            else
               chosen_supersedes = latest_explicit;
         }
         else
         {
            /*
             * NOTHING IN CHAIN WAS EXPLICITLY REQUESTED.... pick either
             * the immediate or latest superseding fix as the prereq
             * parent.
             */
            if (check_prereq.Auto_Supersedes)
               chosen_supersedes = latest_supersede; 
            else
               chosen_supersedes = new;
         }
      }
      (void) link_requisite (chosen_supersedes, old, & EMPTY_CRITERIA,
                             A_PREREQ, error);
      RETURN_ON_ERROR;

      /*
       * Set a flag used in the report routines to alert the user to this
       * anomalous condition.
       */
      chosen_supersedes->requisites->flags.superseded_consistency_apply = TRUE;
   } /* if */

} /* process_anomalies */

/*--------------------------------------------------------------------------*
**
** NAME: check_for_broken_superseded_updts
**
** FUNCTION:  Scans the fix_info_table for updates that are marked BROKEN,
**            which are superseded by a fix on the media (in the case of apply)
**            or which are superseded by fixes that are already installed
**            (for all operations).  The superseded fixes are marked as
**            AVAILABLE (so that they will be ignored for further processing)
**            and in the case of apply, the superseding fixes are marked
**            as CAN_NOT_APPLY IF the appropriate flags are not specified.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
check_for_broken_superseded_updts ()
{
   boolean               ok_to_fix_brok;
   fix_info_type       * fix;
   fix_info_type       * old;
   fix_info_type       * new;
   fix_info_type       * superseding_fix;
   fix_info_type       * sup_of_brok;
   supersede_list_type * sup_node;

   /*
    * See if correct flags were set to correct brokens on apply ops.
    * (ls_programs implies that BROKEN's are already installed, so
    * let's set the flag to TRUE indicating the BROKEN is ok if superseded).
    * 
    */
   ok_to_fix_brok = ((check_prereq.commit && check_prereq.no_save && 
                                                 check_prereq.Auto_Include)
                                   ||
                     (check_prereq.called_from_ls_programs));

   /* 
    * BROKEN superseded updates can appear at the base of a chain or "somewhere
    * in the middle" -- for the case when a root part is BROKEN but the usr
    * part successfully supersedes some other update.  HENCE, we'll iterate 
    * through the fix_info table looking for any BROKEN updates -- the 
    * other routines in this file typically begin processing a supersedes
    * chain at the bottom-most (oldest) ptf.
    */ 
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
       /*
        * See if this is a BROKEN superseded update.
        */
       sup_of_brok = NIL (fix_info_type);
       if ((fix->apply_state == BROKEN) && (IF_UPDATE (fix->op_type)))
       {
          /* 
           * Different way of checking for 4.1 and 3.2 updates.
           */
          if (IF_4_1 (fix->op_type))
          {
             /*
              * Loop on the implicit supersedes chain.  Break out when
              * supersedes chain ends or when a superseding update in 
              * a "good" state is detected.
              */
             old = fix;
             new = fix->next_fix;
             while (IS_STILL_SUPS_CHAIN_LR_SCAN (old,new)) 
             {
                if (new->apply_state <= IN_TOC)
                {
                   sup_of_brok = new;
                   break;
                }
                old = new;
                new = new->next_fix;  
             }

             if (sup_of_brok == NIL (fix_info_type))
             {
                /*
                 * Didn't find a superseding update above.  Re-scan the 
                 * chain of this option looking for a bump in mod level
                 * (there could be one beyond the implicit supersedes chain).
                 */ 
                old = fix;
                new = fix->next_fix;
                while ((new != NIL (fix_info_type)) && 
                       (strcmp (old->name, new->name) == 0))
                {
                   if ((IS_MOD_BUMP (fix,new))        &&
                       (! A_IS_PREREQ_OF_B (fix, new)) &&
                       (new->apply_state <= IN_TOC))
                   {
                      sup_of_brok = new;
                      break;
                   }
                   old = new;
                   new = new->next_fix;  

                } /* end while */
             } /* end if (sup_of_brok... */
          }
          else
          {
             /*
              * Simple matter of checking for an explicit supersedes
              * link for non-41 updates.
              */
             if (fix->superseded_by != NIL (supersede_list_type))
                sup_of_brok = fix->superseded_by->superseding_fix;
          }

          if (fix->origin == COMMAND_LINE) 
          {
             /*
              * If trying to re-install a BROKEN update, make sure the
              * correct flags are set.  Put an entry on the failsop if not.
              */
             if (! ok_to_fix_brok)
             {
                move_failed_sop_entry_to_fail_sop (fix, STAT_BROKEN);
                fix->origin = IN_TOC;    /* Makes sure we don't consider it
                                            in "command line" processing */
             }
             else
             {
                /*
                 * change the BROKEN apply_state.
                 */
                fix->apply_state = TO_BE_EXPLICITLY_APPLIED; 
             }
          }
       }
       
       if (sup_of_brok != NIL (fix_info_type))
       {
          /*
           * Found a valid supersedes of a BROKEN update.  Set the BROKEN 
           * update's state so that we ignore it in further processing
           * (requisite checks).  Don't do it if we're applying root 
           * parts to 3.2 updates.  We need to try to fix the root part 
           * on the client and that will be handled by process_anomalies. 
           * (Currently, this is done for no other reason than to prevent 
           * resulting in a usr-root supersedes mis-match.  
           * ie. U3-sups->U2-sups->U1.  If U2 is BROKEN on root part, we 
           * don't want to install U3 without fixing U2.  This would leave 
           * us with U1 superseded by U3 on root part and superseded by U2 
           * on usr part -- an error condition in most cases.
           */
          if (check_prereq.mode != OP_APPLY || 
              ! (IF_3_2 (fix->op_type) && USR_ROOT_PKG_W_NO_ROOT_INSTLD(fix)))
          {
             if (IF_4_1 (fix->op_type))
                fix->apply_state = SUPD_BY_NEWER_UPDT; 
             else
                fix->apply_state = AVAILABLE;

             /*
              * This is all we need to do for non-apply ops.  For apply ops...
              */
             if (check_prereq.mode == OP_APPLY)
             {
                /*
                 * Mark the superseding fixes according to whether or 
                 * not the "apply-commit-nosave" flags are set."
                 * (This loop will only work for explicit supersedes.)
                 */
                for (sup_node = fix->superseded_by;
                     sup_node != NIL (supersede_list_type);
                     sup_node = sup_node->superseding_fix->superseded_by)
                {
                   mark_sup_of_broken_updt (sup_node->superseding_fix,
                                            ok_to_fix_brok);

                }    /* end for */

                /*
                 * Process implicit supersedes chains.
                 */
                if (IF_4_1 (fix->op_type))
                {
                   /*
                    * First mark updates in the immediate supersedes chain.
                    */
                   old = fix;
                   new = fix->next_fix;
                   while (IS_STILL_SUPS_CHAIN_LR_SCAN (old, new)) 
                   {
                      if (new->apply_state <= IN_TOC)
                      {
                         mark_sup_of_broken_updt (new, ok_to_fix_brok);
                      }
                      old = new;
                      new = new->next_fix;  
                   }

                   old = fix;
                   new = fix->next_fix;
                   while ((new != NIL (fix_info_type)) && 
                          (strcmp (old->name, new->name) == 0))
                   {
                      if (IS_MOD_BUMP (fix, new) &&
                          new->apply_state <= IN_TOC)
                      {
                         mark_sup_of_broken_updt (new, ok_to_fix_brok);
                      }
                      old = new;
                      new = new->next_fix;  

                   } /* end while */
                }    /* end if */
             }       /* end if */
          }          /* end if */
       }             /* end if */
   }                 /* end for */
} /* check_for_broken_superseded_updts */

/*--------------------------------------------------------------------------*
**
** NAME: mark_sup_of_broken_updt
**
** FUNCTION:  Marks a node, representing a superseding fix of a broken 
**            update, as being such.  Disables it from the requisite
**            evaluation process if correct flags aren't specified.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
mark_sup_of_broken_updt
(
fix_info_type * sup_of_broken,
boolean         ok_to_fix_brok
)
{
   if (sup_of_broken->apply_state == ALL_PARTS_APPLIED)
      return;  /* Already supersedes the older update */

   /*
    * Tag bit used for error reporting.
    */
   sup_of_broken->flags |= SUPERSEDES_BROKEN_PTF;

   /*
    * Mark it as uninstallable if correct flags aren't given.
    */
   if (! ok_to_fix_brok)
   {
      sup_of_broken->apply_state = CANNOT_APPLY;
      if (sup_of_broken->origin == COMMAND_LINE)
      {
         /*
          * Handle explicitly requested updates appropriately.
          * (ie. don't let them get onto fix_info's requisite list and
          * put a corresponding entry on the failsop). 
          */
         move_failed_sop_entry_to_fail_sop (sup_of_broken, STAT_SUP_OF_BROKEN);
         sup_of_broken->origin = IN_TOC;   
      } 
   } /* end if */
         
} /* mark_sup_of_broken_updt */
