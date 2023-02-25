static char sccsid[] = "@(#)58  1.20  src/bos/usr/sbin/install/ckprereq/evaluate_base_levels.c, cmdinstl, bos41J, 9518A_all 4/30/95 22:00:40";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            change_state_of_disabled_pkg
 *            disable_older_pkg
 *            evaluate_base_levels
 *            get_first_node_in_superseding_option
 *            is_newer_base_preferred
 *            pre_scan_cur_option
 *            process_base_level_conflict
 *            process_disabling_error
 *            set_restart_position
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

/*
 * Meaningful names for constants and return values to and from routines in 
 * this file.
 */
#define PKG_IS_OLDER               TRUE
#define PKG_IS_NEWER               FALSE
#define DISABLING_EXPL_SUPD_OPTION TRUE
#define BETTER        1
#define SAME          0
#define WORSE        -1 
#define LOOP_CONTINUE              TRUE
#define LOOP_BREAK                 FALSE

#define IS_NON_DUMMY_BASE_LEVEL(fix)                                        \
              ((IF_INSTALL(fix->op_type) && (fix->origin != DUMMY_FIX)) ||  \
               (COMPAT(fix->cp_flag)))

boolean recover_from_disabling_error = FALSE;

#define CONINUE_AFTER_DISABLING_ERR(error, loop_continue)                    \
     {                                                                       \
        if (* error)                                                         \
        {                                                                    \
           if (recover_from_disabling_error)                                 \
           {                                                                 \
              graph_dump ("Disabling Error.");                               \
              recover_from_disabling_error = FALSE;                          \
              *error = FALSE;                                                \
              if (loop_continue)                                             \
                 continue;                                                   \
              else                                                           \
                 break;                                                      \
           }                                                                 \
           else                                                              \
              return;                                                        \
        }                                                                    \
     }
/* 
 * Used for "parsing events" when looking for base levels to compare.
 */
typedef enum {SKIP_UPDATES, FOUND_BETTER_BASE_LEV, DISABLE_NEWER_PKGS}
              eval_base_event_type;

static  char print_string1 [MAX_LPP_FULLNAME_LEN];
static  char print_string2 [MAX_LPP_FULLNAME_LEN];

static void change_state_of_disabled_pkg (
fix_info_type * best, 
fix_info_type * current,
boolean         disabling_older_pkg,
boolean         disabling_expl_supd_option,
boolean       * error);

static fix_info_type * get_first_node_in_superseding_option (
fix_info_type * compat_entry);

static short is_newer_base_preferred (
fix_info_type * newer_base, 
fix_info_type * current_best,
boolean         only_updates_requested);

static void mark_expl_superseding_base_conflict (
fix_info_type * disabler, 
fix_info_type * disablee);

static void process_base_level_conflict (
fix_info_type         * newer_base,
fix_info_type        ** current_best,
eval_base_event_type  * event,
boolean                 only_updates_requested,
boolean               * error);

static void pre_scan_cur_option (
fix_info_type  * start_of_option,
boolean        * only_updates_requested,
fix_info_type ** first_node_in_unprocessed_expl_supd_option,
boolean        * error);

static void process_disabling_error (
fix_info_type * disabler,
fix_info_type * disablee,
boolean       * error);

static void set_restart_position (
fix_info_type  * branch_from_node,
fix_info_type  * branch_to_node,
fix_info_type ** restart_position);

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_base_levels
**
** FUNCTION:  Scans the fix info table trying to make up-front decisions 
**            about which base levels may be used when multiple versions of
**            a base level product are on the media OR when a base level is
**            on the media that is at a different level than a base
**            level that is already installed.  Goal is to prevent 
**            different versions of the same product and their updates from
**            interfering with subsequent requisite resolution.
**
** RETURNS:  void function 
**
**--------------------------------------------------------------------------*/

void 
evaluate_base_levels 
(
boolean * error
)
{
   boolean               share_only_seen;
   boolean               usr_only_seen;
   boolean               usr_root_seen;
   boolean               only_updates_requested;
   eval_base_event_type  event;
   fix_info_type       * current_best;
   fix_info_type       * first_node_in_unprocessed_expl_supd_option;
   fix_info_type       * restart_position;
   fix_info_type       * newer_pkg;
   fix_info_type       * tmp;
   Option_t            * sop_ptr;
   short                 rc;

   if (check_prereq.called_from_command_line)
      /*
       * Isn't necessary to consider conflicting base levels when 
       * doing ckprereq command work.
       */
      return;

   /* 
    * Terminology:
    * "better base level" = A base level which is preferred from a 
    *     requisite resolution or user request stand-point.  ie. an 
    *     installed version of a base level option is is better at satisfying
    *     requisites than another version that's on the media.   An explicitly 
    *     requested base level is better than one that's already installed.
    * "disable pkgs" = mark the state of a pkg in such a way to prevent it 
    *     from being installed and/or satisfying requisites.
    *
    * Detailed Strategy:
    *
    * iterate from left to right on the sorted fix_info table.
    * compare base levels of same option 
    * if newer is better
    *    "disable" all older packages
    * if newer is worse
    *    "disable" all newer packages until a better base is found
    * else
    *    do nothing.
    *
    * This routine uses a finite state automaton to "parse events".
    * We can be in one of 3 states while traversing the fix_info table:
    * SKIP_UPDATES:  means we have yet to find a base level or we are skipping
    *                updates for the current base level (don't want to 
    *                disable them).  We leave this state when we find
    *                a better or worse base level than the previous base level.
    * FOUND_BETTER_BASE_LEVEL:
    *                means the current base level for the current option is
    *                in a "better state" with respect to requisite checking,
    *                than the previous base level for the current option.
    *                (Maybe it was explicitly requested where the previous one
    *                is merely on the installation media.)
    *                To get out of this state, we must come across updates for
    *                the current base level, or find a base level which is
    *                worse than this base level.  We disable older pkgs from 
    *                the same option when this state is detected. 
    * DISABLE_NEWER_PKGS:
    *                Means we found a newer base level which is worse than
    *                a previous base level.  We will disable newer packages
    *                until a better base level is found.
    */ 

   /*
    * Iterate on fix_info table.  Nested loop iterates on pkgs within the 
    * same option.  
    * 
    * ***NOTE***:  Iteration may not always proceed from left to right, 
    * from option to option.  When an explicit supersedes of a base level 
    * (compat) entry in the fix_info table is detected, evaluation 
    * continues with the option that supersedes the option being processed,
    * which may be at an earlier or later location in the fix_info table
    * (alphanumerically determined).  The compat entry is always guaranteed
    * to be the last entry in a sequence of nodes for a given option (since
    * its level should be higher than any released pkg for that option).
    * exs:
    *  1. Z.new --sups--> A.obj 4.1.0.0
    *
    *  fix_info_table:
    *  .......A.obj 3.2.0.0....A.obj 4.1.0.0....B.obj......Z.new....
    *  evaluation would skip from A.obj to Z.obj before visiting B.obj
    *
    * 2. A.new --sups--> Z.old 4.1.0.0
    *  .......A.new....B.obj......Z.old 3.2.0.0....Z.old 4.1.0.0
    *  processing would skip from A.new to Z.old 3.2.0.0 when the sups link
    *  between A.new and Z.old 4.1.0.0 is detected in the prescan.  The 
    *  order of evluation is Z.old 3.2.0.0, Z.old 4.1.0.0, A.new, B.obj.
    *
    */
   restart_position = NIL (fix_info_type);
   newer_pkg = check_prereq.fix_info_anchor->next_fix;
   while (newer_pkg!= NIL (fix_info_type))
   {
      /*
       * Since processing may skip throughout the fix_info table, use the 
       * VISITING_THIS_NODE tag to ensure we haven't been here before.  
       */
      if (newer_pkg->flags & VISITING_THIS_NODE)
      {
         newer_pkg = newer_pkg->next_fix;
         continue;
      }

      /*
       * scan all nodes in the current option.  Look for explicitly 
       * requested updates (to help with later processing) and for
       * supersedes chains between base levels and compatibility entries.
       * If a base level in the current option supersedes a compatibility
       * entry, the variable first_node_in.... will be set upon return,
       * and the pre_scan will be performed again beginning at this 
       * position in the fix_info table (logically, the oldest pkg in 
       * the option being considered).
       */
      pre_scan_cur_option (newer_pkg, 
                           &only_updates_requested, 
                           &first_node_in_unprocessed_expl_supd_option,
                           error);
      CONINUE_AFTER_DISABLING_ERR (error, LOOP_CONTINUE);

      if (first_node_in_unprocessed_expl_supd_option != NIL (fix_info_type))
      {
         /*
          * Set the variable restart_postion which may be used to resume 
          * iteration after the current option is processed (if an option 
          * will be skipped by the jump to the superseding option) 
          */
         set_restart_position (newer_pkg, 
                               first_node_in_unprocessed_expl_supd_option,
                               &restart_position);
         newer_pkg = first_node_in_unprocessed_expl_supd_option;
         pre_scan_cur_option (newer_pkg,    
                              &only_updates_requested, 
                              &first_node_in_unprocessed_expl_supd_option,
                              error);
         CONINUE_AFTER_DISABLING_ERR (error, LOOP_CONTINUE);
      }


      event = SKIP_UPDATES;                /* "parsing" state */
      current_best = NIL (fix_info_type);  /* pointer to best base in option */
      /*
       * The following varibles are used to detect illegal supersedes of
       * base levels for the option under consideration.
       */
      usr_only_seen = FALSE;
      usr_root_seen = FALSE;
      share_only_seen = FALSE;

      while (TRUE)           /* Iterate on current option; will break 
                                from loop when appropriate. */
      {
         newer_pkg->flags |= VISITING_THIS_NODE;
         switch (event)
         {
            case SKIP_UPDATES:
                if ((IF_INSTALL (newer_pkg->op_type)) 
                                     &&
                    /*
                     * Don't call base_level conflict processing routine 
                     * when there's no current base and the current 
                     * pkg has a "bad" state.
                     */
                    ((current_best != NIL (fix_info_type)) || 
                     (newer_pkg->apply_state <= IN_TOC)))
                {
                   process_base_level_conflict (
                                        newer_pkg, &current_best, &event, 
                                        only_updates_requested, error);
                }
                break;

            case FOUND_BETTER_BASE_LEV:
                if (IF_INSTALL (newer_pkg->op_type)) 
                {
                   process_base_level_conflict (
                                        newer_pkg, &current_best, &event, 
                                        only_updates_requested, error);
                }
                else
                   event = SKIP_UPDATES; 
                break;

            case DISABLE_NEWER_PKGS:
                if ((IF_INSTALL (newer_pkg->op_type)) &&
                    ((rc = is_newer_base_preferred (newer_pkg, current_best,
                                            only_updates_requested)) != WORSE))
                {
                   current_best = newer_pkg;
                   if (rc == BETTER)
                   {
                      disable_older_pkgs(newer_pkg, NIL (fix_info_type),
                                         newer_pkg->name, 
                                         IS_SUPD_COMPAT_ENTRY(newer_pkg), error);
                   }
                   event = FOUND_BETTER_BASE_LEV;
                }
                else
                {
                   /*
                    * Must be update from newer base level or worse base
                    * level.  Disable newer_pkg (don't disable 4.1 updates
                    * since they can belong to potentially numerous base levels
                    * "beneath" them.)  
                    */
                   if ((! IF_UPDATE (newer_pkg->op_type)) ||
                       (! IF_4_1 (newer_pkg->op_type)))
                      change_state_of_disabled_pkg (current_best, newer_pkg,
                                                    PKG_IS_NEWER, 
                                                    !DISABLING_EXPL_SUPD_OPTION,
                                                    error);
                }
                break;

         } /*switch */

         CONINUE_AFTER_DISABLING_ERR (error, LOOP_BREAK);

         RETURN_ON_ERROR;

         if (IS_SUPD_COMPAT_ENTRY (newer_pkg))
         {
           /*
            * Current entry is a superseded/compat entry.  Set the variable
            * restart_postion which will be used to resume iteration after
            * the current option is processed (if an option is being skipped
            * by the jump to the superseding option -- which is conceptually 
            * a different version of newer_pkg).  
            */
            set_restart_position (newer_pkg, 
                                  newer_pkg->superseded_by->superseding_fix,
                                  &restart_position);
            /*
             * Get the first node in the superseding option (should be the
             * actual superseding base level itself, but not necessarily --
             * there could be updates to a release older than the superseding
             * base level on the media)
             */
            newer_pkg = get_first_node_in_superseding_option (newer_pkg);

            continue; /* no need to perform any of remaining error checks */
         }
         else
            newer_pkg = newer_pkg->next_fix;   /* get next pkg */

         /*
          * Check for end of list or change of current option.  Before
          * breaking from the inner loop in either case, check to see if
          * we need to readjust our fix_info pointer to restart our scan
          * at a node we may have skipped while processing explicit 
          * supersedes chains (set in calls to set_restart_position()).
          */
         if ((newer_pkg == NIL (fix_info_type)) 
                               ||
             (strcmp (newer_pkg->name, newer_pkg->prev_fix->name) != 0))
         {
            if (restart_position != NIL (fix_info_type))
            {
                /*
                 * NOTE: Rescan will begin from beginning of table.
                 * This should be changed to start from restart_position
                 * when deferred defect  147697 is fixed.  (without changing 
                 * this, there's a very minor performance hit).
                 */
                newer_pkg = check_prereq.fix_info_anchor->next_fix;
                restart_position = NIL (fix_info_type);
            }
            break;
         }

         /*
          * We're still working on the same option.....
          * Perform error check to prevent a 3.1 or 3.2 formatted fileset 
          * from being produced after a 4.1 formattted fileset with the 
          * same name. (primarily due to requisite satisfying rule where 
          * *prereq A.obj p=U1 is satisfied by 1st installed/installable 
          * 4.1 pkg or compat entry with same name.
          */
         if ((IF_3_2 (newer_pkg->op_type) || IF_3_1 (newer_pkg->op_type))
                                   &&
             (IF_4_1 (newer_pkg->prev_fix->op_type)))
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                     CKP_NO_NEWER_32_THAN_41_E, CKP_NO_NEWER_32_THAN_41_D), 
                     inu_cmdname, 
                     get_fix_name_from_fix_info (newer_pkg->prev_fix, 
                                                     print_string1),
                     get_fix_name_from_fix_info (newer_pkg, print_string2));

            *error = TRUE;
            return;

         } /* if ((IF_3_2.... */
      } /* while (TRUE)*/
   } /* while (newer_pkg..)*/
   
   /*
    * Make one more pass of the fix_info table to do two things:
    * 1. unmark the fix_info table of VISITING_THIS_NODE bits. (set in 
    *    processing above).
    * 2. Add all requested, superseded updates to the failsop for reporting 
    *    purposes.  (This primarily catches 3.2 updates which are superseded by 
    *    compatibility entries).
    */
   for (tmp = check_prereq.fix_info_anchor->next_fix;
        tmp != NIL (fix_info_type);
        tmp = tmp->next_fix)
   {
      if (!(tmp->flags & VISITING_THIS_NODE))
      {
         /*
          * This is a sanity check to make sure that we did process all nodes 
          * in the fix_info table above (we may have done much skipping around 
          * due to explicit supersedes of base level compat entries.)
          */
          inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                   CKP_INT_CODED_MSG_D), inu_cmdname, 
                   FOUND_UNVISITED_NODE_IN_EVALUATE_BASE_LEVELS);
          *error = TRUE;
          return;
      }
      tmp->flags &= ~VISITING_THIS_NODE;

      if ((tmp->origin == COMMAND_LINE)                &&
          (tmp->apply_state == SUPD_BY_NEWER_BASE))
      {
         sop_ptr = move_failed_sop_entry_to_fail_sop (tmp, 
                                                      STAT_ALREADY_SUPERSEDED);
         /*
          * Need to get a superseded_by string to print in the installp
          * failure summary.
          */
         sop_ptr->supersedes = strdup (
                              get_superseding_name_or_lev (tmp, print_string1));
         if (IF_UPDATE (tmp->op_type))
            /*
             * We'll "hide" the fact that this fileset was requested
             * for installation from the rest of the code, if its an update.  
             * If it's base level, we'll leave it so that in addition to the 
             * "already superseded by" msg we'll get a message about the fact 
             * that you can't re-install it (since the older base level we're 
             * talking about must have been a compatible, differently named 
             * base level -- re-install of differently named, older base levels
             * is not supported).
             */
            tmp->origin = TOC;   

      } /* end if */
         
   } /* end for */ 

} /* evaluate_base_levels */

/*--------------------------------------------------------------------------*
**
** NAME: process_base_level_conflict
**
** FUNCTION: Given two base levels, decides which is preferred then
**           modifies the state of the other base level and its updates,
**           as appropriate.
**
** RETURNS:  void function 
**
** SIDE EFFECTS:
**           -- sets the state of the event variable passed for further 
**              processing by caller.
**           -- changes the state of less preferable base levels and their
**              updates, as appropriate.
**           -- Sets other key variables for use by caller.
**
**--------------------------------------------------------------------------*/

static void
process_base_level_conflict 
(
fix_info_type         * newer_base,
fix_info_type        ** current_best,
eval_base_event_type  * event,
boolean               only_updates_requested,
boolean               * error
)
{
   short           rc;
   fix_info_type * prev_best;
 
   /*
    * if newer base is better, reset the current_best pointer and disable 
    * older pkgs.
    * If it's the same, don't disable older pkgs, but reset pointers (may as
    * well let this be best for subsequent processing).
    * If it's worse, disable the newer pkg and set the event state to 
    * "disabling" for our caller.
    */
   if (((rc = is_newer_base_preferred (newer_base, (* current_best), 
                                          only_updates_requested )) == BETTER)
                       ||
       (rc == SAME)) 
   {
      prev_best = (*current_best);
      (* current_best) = newer_base;
      if (rc == BETTER) 
      {
         (* event) = FOUND_BETTER_BASE_LEV;
         /* 
          * Make appropriate call to disable_older_pkgs() depending upon 
          * whether or not we're starting from a superseding base level of a
          * compat entry.
          */
         if (newer_base->supersedes &&
             (newer_base->supersedes->superseded_fix == prev_best))
	 {
            disable_older_pkgs (newer_base, prev_best, prev_best->name,
                                DISABLING_EXPL_SUPD_OPTION, error);
            /*
             * We need to handle an anomalous case where there are 
             * entries to the left of the superseding base level which
             * are not accounted for in the supersedes chain.
             * The most likely scenario is the following:
             *  ....-> A.new 1112 -> A.new 1120 -> A.old 1100 ->....
             *  where:
             *    A.new 1112 is an update, A.new 1120 is a base level,
             *    A.old 1100 is a compat entry (ie supd by A.new 1120)
             * Hopefully, there will only ever be un-installed updates to 
             * the left of the superseding base lev and so it will be a simple
             * matter to disable them.  (There could be other base levels
             * if packaging errors were made.)
             */
            if (strcmp (newer_base->name, newer_base->prev_fix->name) == 0)
               disable_older_pkgs (newer_base, NIL (fix_info_type),
                                   newer_base->name, FALSE, error);
	 }
         else
            disable_older_pkgs (newer_base, NIL (fix_info_type), 
                                newer_base->name, 
                                IS_SUPD_COMPAT_ENTRY (newer_base), error);
         RETURN_ON_ERROR;
      }
   }
   else
   {
      change_state_of_disabled_pkg ((*current_best), newer_base, 
                                    PKG_IS_NEWER, 
                                    ! DISABLING_EXPL_SUPD_OPTION,
                                    error);
      RETURN_ON_ERROR;
      (* event) = DISABLE_NEWER_PKGS; 
   }

} /* process_base_level_conflict */

/*--------------------------------------------------------------------------*
**
** NAME: is_newer_base_preferred
**
** FUNCTION: Given two base levels, decides which is more preferable based
**           on apply_state.
**  
** RETURNS:  BETTER, WORSE, or SAME.
**
**--------------------------------------------------------------------------*/

static short
is_newer_base_preferred 
(
fix_info_type * newer_base, 
fix_info_type * current_best,
boolean         only_updates_requested
)
{
   /*
    * Preferences:
    *    -- ALL_PARTS_APPLIED is better than IN_TOC (on the media)
    *    -- TO_BE_EXPLICITLY_APPLIED is better than ALL_PARTS_APPLIED
    *    -- newer is better than older (when state is equal).  
    *    -- For all other states, a lower state (as defined by the enumerated
    *       type in ckp_common_defs.h) is better.
    * 
    *    NOTE: Two conflicting base levels will never be marked
    *    TO_BE_EXPLICITLY_APPLIED since installp throws out the older base 
    *    level nor will they ever be ALL_PARTS_APPLIED (since this is an 
    *    impossible condition).
    *
    * Exceptions: 
    * When multiple, conflicting base levels are on the media, if none are
    * installed and none were explicitly requested for installation, choice of
    * best base level is deferred by this routine under the following 
    * circumstances:
    * 1. -g is not provided -- (-g is the same as auto-supersedes)  All 
    *    implicitly requested base levels will be listed as required by those  
    *    which were explicitly requested.
    * OR
    * 2. -g is provided, and:
    *    A. updates to one of the base levels were explicitly requested 
    *       (lets an update implicitly request its base level)
    *    OR
    *    B. A base level has a barrier on older base levels. (choice between
    *       the incompatible base levels is deferred).
    *
    * The choice is deferred to the routine 
    * is_conflicting_implicit_base_level() which will pick the first
    * level implicitly requested (referenced) in the evaluation process.
    * All other base levels will be disabled at that time.
    */

   if (current_best == NIL (fix_info_type))
      return (BETTER);

   if ((newer_base -> apply_state == IN_TOC) 
                      &&
       (current_best->apply_state == IN_TOC) 
                      &&
       ((! check_prereq.Auto_Supersedes) || 
        (! is_compat_levels (newer_base->name,
                             newer_base->supersedes_string, 
                             &(current_best->level))) ||
        (only_updates_requested)))
       return (SAME);

   /*
    * A requested pkg is always more preferable than a pkg which is
    * already installed.  Handle the case of installing a newer version
    * over an older, and force installing an older over a newer.
    */
   if ((current_best->apply_state == ALL_PARTS_APPLIED) &&
        (newer_base->apply_state == TO_BE_EXPLICITLY_APPLIED))
      return (BETTER); 
   else
      if ((current_best->apply_state == TO_BE_EXPLICITLY_APPLIED) &&
          (newer_base->apply_state == ALL_PARTS_APPLIED))
          return (WORSE);

   if ((newer_base->apply_state > IN_TOC) &&
       (current_best->apply_state > IN_TOC))
      /*
       * For "bad" states (worse than IN_TOC), we don't necessarily want to 
       * pick one over the other.  This could cause invocation of the disabling
       * routines, causing unexpected and undesirable disabling to occur.
       * Returning SAME will prevent calling the disabling code.
       */
      return (SAME);

   if ((newer_base->apply_state == PARTIALLY_APPLIED)   
                             &&
       (current_best->apply_state == ALL_PARTS_APPLIED) )
      /*
       * This condition should only be encountered when processing an 
       * explict superseding base level and the differently named option 
       * which it supersedes.  (ex.  the root part of the newer pkg is 
       * BROKEN on a standalone machine OR the newer pkg is not fully 
       * installed on a diskless machine.)   The partially installed
       * newer pkg is better.
       */
      return (BETTER);

   if (newer_base->apply_state <= current_best->apply_state)
      return (BETTER);
   else
      return (WORSE);

} /*  is_newer_base_preferred */

/*--------------------------------------------------------------------------*
**
** NAME: disable_older_pkgs
**
** FUNCTION: Traverses the fix_info_table from right to left disabling
**           all pkgs presumably with worse state.  States of disabled
**           pkgs are set according to the state of the newer pkg and
**           whether or not it is compatible with the newer pkg.
**           If explicitly superseding base levels are detected,
**           disabling continues along the supersedes chain.
**  
** RETURNS:  void function
**
** SIDE EFFECTS:
**           apply_state of older pkgs are modified.
**
**--------------------------------------------------------------------------*/

void
disable_older_pkgs 
(
fix_info_type * new_best_base,                 /* base level dictating 
                                                  disabled state. */
fix_info_type * start_node,                    /* If NIL, disabling begins at
                                                  prev_fix of new_best_base,
                                                  else begins at start_node */

char          * reference_name,                /* needed since name of 
                                                  disablees may be different
                                                  than new_best_base */
boolean         disabling_expl_supd_option,    /* tells us if ok to disable
                                                  apparently good states. */
boolean       * error
)
{
   fix_info_type * current;

   if (start_node != NIL (fix_info_type))
      current = start_node;
   else
      current = new_best_base->prev_fix;

   while (strcmp (current->name, reference_name) == 0)
   {
      /*
       * Check for a very rare and specific problem, where a base level
       * has a requisite on an update from a previous level.  
       * (cics6000 did this in 3.2.)  The older update should be installed 
       * with the newer base level, and therefore should not be disabled here.
       * This will not be permitted with 4.1 pkgs since those updates
       * ultimately have to prereq some base and an update may not prereq
       * a newer base (caught in verify_graph()).
       */
      if ((! IF_3_2 (new_best_base->op_type)) || 
          ! (IF_UPDATE (current->op_type) && 
             strstr (new_best_base->prereq_file, current->level.ptf) != 
                                                                  NIL (char)))
      {
         change_state_of_disabled_pkg (new_best_base, current, 
                                       PKG_IS_OLDER, 
                                       disabling_expl_supd_option, error);
         RETURN_ON_ERROR;
      }

      /*
       * Recurse on this routine if we're at a "supersedes junction." ie.
       * if we need to disable down a supersedes, compat entries option. 
       * Upon return, we'll continue disabling on the current option if 
       * necessary.
       */
      if (IF_INSTALL (current->op_type) &&
          current->supersedes != NIL (supersede_list_type))
      {
          disable_older_pkgs (new_best_base, 
                              current->supersedes->superseded_fix, 
                              current->supersedes->superseded_fix->name, 
                              DISABLING_EXPL_SUPD_OPTION, error);
          RETURN_ON_ERROR;
      }
      current=current->prev_fix;

   } /* end while */

} /* disable_older_pkgs */

/*--------------------------------------------------------------------------*
**
** NAME: change_state_of_disabled_pkg
**
** FUNCTION: Changes the state of disablee with respect to the state of
**           disabler and whether or not we're disabling an older or a 
**           newer pkg than disabler.
**           See is_newer_preferred() for state explanation.
**           Disabled pkgs will be set to:
**           SUPD_BY_NEWER_BASE, or TO_BE_OVERWRITTEN_BY_SUP_BASE 
**           to allow subsequent requisite resolution with newer base levels.
**                                    OR
**           TO_BE_OVERWRITTEN_BY_CONFL_LEV or CANNOT_APPLY_CONFL_LEV
**           to prevent requisite resolution with the disabled pkg.
**  
** RETURNS:  void function
**
** SIDE EFFECTS:
**           apply_state of older pkgs is modified.
**
**--------------------------------------------------------------------------*/

static void
change_state_of_disabled_pkg 
(
fix_info_type * disabler, 
fix_info_type * disablee,            
boolean         disabling_older_pkg,
boolean         disabling_expl_supd_option,  /* permits disabling of apparently
                                                good states */
boolean       * error
)
{
   boolean unexpected_state = FALSE;
   fix_info_type            * fix;
   char                     * rescan_name;
 
   switch (disabler->apply_state)
   {
      case IN_TOC:
         if (disablee->apply_state >= IN_TOC)
         {
            if (disabling_older_pkg)
               if (is_compat_levels (disabler->name,
                                     disabler->supersedes_string, 
                                     &disablee->level))
                  disablee->apply_state = SUPD_BY_NEWER_BASE;
               else
                  disablee->apply_state = CANNOT_APPLY_CONFL_LEVEL;
            /* else, newer pkg probably has a "bad" state already. Leave it
               like that. */
         }
         else
         {
            /*
             * If the package being disabled is NOT an explicitly requested
             * update, then we have an internal error -- didn't pick the right
             * best base level.  If it is, don't disable the state, leave 
             * requisite failure mechanisms to catch the problem.
             */
            if ((! IF_UPDATE(disablee->op_type))  ||
                (disablee->apply_state != TO_BE_EXPLICITLY_APPLIED))
            {
               unexpected_state = TRUE;
            }
         }
         break;

      case ALL_PARTS_APPLIED:
      case PARTIALLY_APPLIED:
         if ((disablee->apply_state >= IN_TOC) /* disablee state should be
                                                  IN_TOC or worse */
                             ||
             (disabling_expl_supd_option)      /* OR should be disabling
                                                  superseded option (base level
                                                  sups */
                             ||
                                               /* OR should be disabling an 
                                                  older installed or requested
                                                  update */
             (IF_UPDATE (disablee->op_type)                     &&
              ((disablee->apply_state == TO_BE_EXPLICITLY_APPLIED) ||
               (IF_UPDATE (disablee->op_type)     &&
                (disablee->parts_applied != 0)    &&
                disabling_older_pkg))))
         {
            if (disabling_older_pkg && 
                (is_compat_levels (disabler->name,
                                   disabler->supersedes_string, 
                                   &(disablee->level) )))
            {
               disablee->apply_state = SUPD_BY_NEWER_BASE;
            }
            else
            {
               disablee->apply_state = CANNOT_APPLY_CONFL_LEVEL;
               mark_expl_superseding_base_conflict (disabler, disablee);
            }
         }
         else
         {
            unexpected_state = TRUE;
         }
         break;

      case TO_BE_EXPLICITLY_APPLIED:
         if (disablee->apply_state >= IN_TOC)    /* disablee state could be
                                                    IN_TOC or worse. */
         {
            if (disabling_older_pkg &&
                (is_compat_levels (disabler->name,
                                  disabler->supersedes_string, 
                                  &(disablee->level))) )
               disablee->apply_state = SUPD_BY_NEWER_BASE;
            else
               disablee->apply_state = CANNOT_APPLY_CONFL_LEVEL;
         }
         else
         {
            if (disablee->apply_state == ALL_PARTS_APPLIED ||  /* disablee may*/
                disablee->apply_state == PARTIALLY_APPLIED)    /* be installed*/
            {
               if (disabling_older_pkg && (is_compat_levels 
                                               (disabler->name,
                                                disabler->supersedes_string, 
                                                &(disablee->level))))
                  disablee->apply_state = TO_BE_OVERWRITTEN_BY_SUP_BASE; 
               else
                  disablee->apply_state = TO_BE_OVERWRITTEN_BY_CONFL_LEVEL;
            }
            else
            {
               if ((disabling_expl_supd_option)        /* we better be disabling
                                                          a superseded option */
                                 ||
 
                   (IF_UPDATE (disablee->op_type)  &&  /* OR we better be dis-
                                                          abling a requested
                                                          update */

                    disablee->apply_state == TO_BE_EXPLICITLY_APPLIED)) 
               {
                  if (disabling_older_pkg && (is_compat_levels 
                                               (disabler->name,
                                                disabler->supersedes_string, 
                                                &(disablee->level))))
                     disablee->apply_state = SUPD_BY_NEWER_BASE; 
                  else
                     disablee->apply_state = CANNOT_APPLY_CONFL_LEVEL;
               }
               else
               {
                  unexpected_state = TRUE;
               }
            }
         }         
         break;
      default:
         /*
          * At this point we know our state is not one of particular interest.
          * (could be picking the best from among some really bad states.)
          * Only concern ourselves if the so called best is worse than disablee
          * and if both aren't one of the SUPERSEDED states (which are
          * effectively the same).
          */
         if (disabler->apply_state > disablee->apply_state)
         {
            unexpected_state = TRUE;
         }
   } /* switch */

   if (unexpected_state)
   {
      process_disabling_error (disabler, disablee, error);
      RETURN_ON_ERROR;
   }
} /* change_state_of_disabled_pkg */

/*--------------------------------------------------------------------------*
**
** NAME: pre_scan_cur_option
**
** FUNCTION:  Check the packages in this option to see if the
**            only thing that was explicitly requested was an update.
**            Set boolean flag if so.  (This will help us when picking 
**            between two base levels with the same state -- see 
**            is_newer_base_preferred.) Also, check to see if a base level
**            in the current option EXPLICITLY supersedes a different option,
**            via a dummy compat entry, and set a pointer to the 1st entry
**            for that superseded option for our caller.
**            While scanning this option, scan explicit supersedes 
**            chains of dummy compat entries since they are conceptually
**            part of the current option to be considered.
**            When a compat entry is detected, disable the older option
**            here if the superseding option has a base level which is
**            applied or is going to be applied.
**             
** RETURNS:  void function
**
**--------------------------------------------------------------------------*/

static void
pre_scan_cur_option
(
fix_info_type  * start_of_option,
boolean        * only_updates_requested,
fix_info_type ** first_node_in_unprocessed_expl_supd_option,
boolean        * error
)
{
   boolean         base_requested   = FALSE;
   boolean         scanning_expl_sups_branch = FALSE;
   boolean         update_requested = FALSE;
   boolean         expl_request_on_seded_branch = FALSE;
   fix_info_type * next_node;
   fix_info_type * reference_node;
   fix_info_type * tmp;
   fix_info_type * tmp2;

   (*first_node_in_unprocessed_expl_supd_option) = NIL (fix_info_type);
   for (tmp = start_of_option, reference_node = start_of_option;
        (tmp != NIL (fix_info_type)) &&
          (strcmp (reference_node->name, tmp->name)== 0);
        tmp = next_node)
   {
      tmp->flags |= VISITING_THIS_SUBGRAPH; /* tag entries that we visit on
                                               this pre-scan -- helps when
                                               deciding if we need to re-scan*/

      /*
       * Disable a superseded option where a request has been made to install
       * a base level when the superseding option is being installed or has
       * been installed.  inu_ckreq() has to worry about this here since
       * installp normally catches attempts to install older versions of 
       * an installed base level (it cannot in this case since they have
       * different names.)
       */
      if ((scanning_expl_sups_branch)
                         &&
          (expl_request_on_seded_branch)
                         &&
          IF_INSTALL (tmp->op_type)
                         &&
          (tmp->apply_state == ALL_PARTS_APPLIED ||
           tmp->apply_state == TO_BE_EXPLICITLY_APPLIED))
      {
          /*
           * Make appropriate call to disable older pkgs, depending upon
           * whether or not this is the immediate supersedes of the compat
           * entry or some later base level which implicitly supersedes the
           * compat entry.
           */
          if (tmp->supersedes)
             disable_older_pkgs (tmp, tmp->supersedes->superseded_fix,
                                 tmp->supersedes->superseded_fix->name,
                                 DISABLING_EXPL_SUPD_OPTION,
                                 error);
          else
             disable_older_pkgs(tmp, NIL (fix_info_type),
                                tmp->name, 
                                IS_SUPD_COMPAT_ENTRY (tmp), error);
          RETURN_ON_ERROR;
      }

      /*
       * make check to see if we can ultimately set the only_updates_requested
       * variable.
       */
      if (tmp->apply_state == TO_BE_EXPLICITLY_APPLIED)
      {
         if (IF_INSTALL (tmp->op_type) ) {
            base_requested = TRUE;
         } else { 
            update_requested = TRUE;
         }
         /*
          * Set flag to tell if we need to disable older pkgs in subsequent
          * iterations along the superseding option.  (Note: we're not
          * really on the sceded_branch if we're looking at the compat entry.)
          */
         if (!(scanning_expl_sups_branch || IS_SUPD_COMPAT_ENTRY (tmp)))
            expl_request_on_seded_branch = TRUE;

      } /* if */

      if (IF_INSTALL (tmp->op_type))
      {
         /*
          * Check for supersedes or superseded_by chains for this base level.  
          * If we detect a supersedes chain, and we haven't processed the
          * superseded option, find the 1st node in that option and return
          * its pointer to our caller.
          * If we detect a superseded_by chain, adjust pointers so that we
          * continue this pre-scan along the superseding option.
          */
         if ((tmp->supersedes) &&
             !(tmp->supersedes->superseded_fix->flags & 
                             (VISITING_THIS_SUBGRAPH | VISITING_THIS_NODE)))
         {
           for (tmp2 = tmp->supersedes->superseded_fix, tmp = tmp2;
                (strcmp (tmp2->name, tmp->name) == 0);
                tmp2 = tmp2->prev_fix)
           {
               (*first_node_in_unprocessed_expl_supd_option) = tmp2;
           }
           break;
         }
         else
         {
            if ((tmp->superseded_by) &&
                !(tmp->superseded_by->superseding_fix->flags & 
                             (VISITING_THIS_SUBGRAPH | VISITING_THIS_NODE)))
            {
               scanning_expl_sups_branch = TRUE;
               reference_node = tmp->superseded_by->superseding_fix;
               next_node = reference_node;
               continue;
            }
         }
      } /* if (IF_INSTALL... */

      next_node = tmp->next_fix;

   } /* for */

   (* only_updates_requested) = (update_requested && ! base_requested);

   /*
    * reset the tag bit used to mark nodes being visited.
    */
   for (tmp = start_of_option, reference_node = start_of_option;
        (tmp != NIL (fix_info_type)) &&
          (strcmp (reference_node->name, tmp->name)== 0);
        tmp = next_node)
   {
      tmp->flags &= ~VISITING_THIS_SUBGRAPH;
      if (IS_SUPD_COMPAT_ENTRY(tmp))
      {
         reference_node = tmp->superseded_by->superseding_fix;
         next_node = reference_node;
         continue;
      }
      next_node = tmp->next_fix;

   } /* for */

} /* pre_scan_cur_option */

/*--------------------------------------------------------------------------*
**
** NAME: get_first_node_in_superseding_option
**
** FUNCTION:  given a fix_info node, representing a base level, with a 
**            superseded_by link to an option with a different name, finds the 
**            first fix_info structure for the superseding option.
**             
** RETURNS:  returns the fix_info_entry found.
**
**--------------------------------------------------------------------------*/

static fix_info_type *
get_first_node_in_superseding_option
(
fix_info_type * compat_entry
)
{
   fix_info_type * tmp;

   for (tmp = compat_entry->superseded_by->superseding_fix;
        ; /* will exit from function when stopping condition met */
        tmp=tmp->prev_fix)
   {
     if (strcmp (tmp->prev_fix->name, 
                 compat_entry->superseded_by->superseding_fix->name) != 0)
        return (tmp);

   } /* end for */

} /* get_first_node_in_superseding_option */

/*--------------------------------------------------------------------------*
**
** NAME: set_restart_position
**
** FUNCTION:  Checks if the next_fix to the right of the branch_from_node is
**            is the branch_to_node or if it has been processed.  If not, and
**            if the restart_position argument has not been set, sets the
**            restart_position pointer to the next_fix of branch_from_node. 
**             
** RETURNS:  void function; side effect: assigns a value to the restart_position
**           argument.
**
**--------------------------------------------------------------------------*/

static void 
set_restart_position
(
fix_info_type  * branch_from_node,
fix_info_type  * branch_to_node,
fix_info_type ** restart_position
)
{
   if ((branch_from_node->next_fix != NIL (fix_info_type))
                     &&
       (branch_from_node->next_fix != branch_to_node) 
                     &&
       ((*restart_position) == NIL (fix_info_type)) )
   {
      (* restart_position) = branch_from_node->next_fix;
   }

} /* set_restart_position */

/*--------------------------------------------------------------------------*
**
** NAME: process_disabling_error
**
** FUNCTION:  Prints an internal error message reflecting that an unexpected
**            condition was detected when disabling pkgs for the current
**            installation.  If the pkg being disabled was a base level,
**            the entire option is disabled for the current installation and
**            will not be allowed to be installed or satisfy requisites.
**            If the pkg is not a requisite of another or was not requested,
**            then the error will not cause any failures to occur, though
**            the internal error message printed here will be displayed.
**             
** RETURNS:  void function
**
**--------------------------------------------------------------------------*/

static void process_disabling_error 
(
fix_info_type * disabler,
fix_info_type * disablee,
boolean       * error
)
{
   fix_info_type * fix;
   char          * rescan_name;

   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_ERR_DISABLING_PKGS_E,
            CKP_INT_ERR_DISABLING_PKGS_D), 
            inu_cmdname,
            disabler->name, get_level_from_fix_info (disabler, print_string1),
            disablee->name, get_level_from_fix_info (disablee, print_string2));

   /* 
    * If the disablee is an update, let's not error off.  Chances are
    * we have just encountered a problem which wasn't really a problem.
    * (ie. a valid unexpected condition).
    * The update will not have been disabled and will more than likely
    * fail to install for requisite reasons (ie. wrong base installed).
    */
   if (IF_INSTALL (disablee->op_type))
   {
      *error = TRUE;
      recover_from_disabling_error = TRUE;  /* Global flag used to tell 
                                               top-most caller that even though
                                               *error flag is TRUE we can
                                               still recover. */
      /*
       * Scan the fix_info table looking for all occurrences of the
       * disabler or the disablee.
       */
      rescan_name = NIL (char);   /* Used to trigger rescan if there's a 
                                     superseded or superseding base level
                                     which does not match the disabler or
                                     disablee name. */
      for (fix = check_prereq.fix_info_anchor->next_fix;
           fix != NIL (fix_info_type);
           fix = fix->next_fix)
      {
         if ((strcmp (fix->name, disabler->name) == 0)
                                  ||
             (strcmp (fix->name, disablee->name) == 0))
         {
            fix->apply_state = CANNOT_APPLY;      /* Disable pkg.             */
            fix->flags |= VISITING_THIS_NODE;     /* Don't process again.     */
            fix->flags |= DISABLING_INT_ERR_FLAG; /* Flag for subsequent error
                                                     report, should this pkg
                                                     be a requisite of another
                                                     or if it was requested
                                                     for installation. */
      
            /*
             * Set rescan_name if necessary.
             */ 
            if (IS_SUPD_COMPAT_ENTRY (fix))
            { 
               if ((strcmp (fix->superseded_by->superseding_fix->name, 
                            disabler->name) != 0) &&
                   (strcmp (fix->superseded_by->superseding_fix->name,
                            disablee->name) != 0))
                  rescan_name = fix->superseded_by->superseding_fix->name;
            }
            else
            {
               if ((fix->supersedes)  
                           &&
                   (strcmp (fix->supersedes->superseded_fix->name, 
                                                     disabler->name) != 0) 
                           &&
                   (strcmp (fix->supersedes->superseded_fix->name,
                                                     disablee->name) != 0))
                  rescan_name = fix->supersedes->superseded_fix->name;
            }
         }
      } 
      if (rescan_name == NIL (char))
         return;

      for (fix = check_prereq.fix_info_anchor->next_fix;
           fix != NIL (fix_info_type);
           fix = fix->next_fix)
      {
         if (strcmp (fix->name, rescan_name) == 0)
         {
            fix->apply_state = CANNOT_APPLY;
            fix->flags |= VISITING_THIS_NODE;
            fix->flags |= DISABLING_INT_ERR_FLAG;
         }
      }
   }

} /* process_disabling_error */

/*--------------------------------------------------------------------------*
**
** NAME: mark_expl_superseding_base_conflict
**
** FUNCTION:  Determines if the fileset being disabled is a renamed, 
**            superseding base level (as opposed to a superseded base level).
**            If so, saves the name of the superseded base level in the
**            supersedes string for error reporting purposes.  (a tag bit in 
**            the flags field is also set for reporting purposes.)
**             
** RETURNS:  void function
**
**--------------------------------------------------------------------------*/

static void 
mark_expl_superseding_base_conflict
(
fix_info_type * disabler, 
fix_info_type * disablee
)
{
   char            name_lev_buf [MAX_LPP_FULLNAME_LEN];

   /*
    * Looking for disabler-disablee pairs with differnent names when disablee
    * is a base level. 
    */
   if (strcmp (disabler->name, disablee->name) == 0 ||  
       (!IF_INSTALL (disablee->op_type)))
      return;

   /*
    * Remember the superseded fileset's name and level for error reporting.
    * First, free up the existing supersedes string if anything present.  
    * (might contain barrier information, which we won't be needing after this
    * is disabled.)
    */
   if (disablee->supersedes_string != NIL (char))
   {
      free (disablee->supersedes_string);
      disablee->supersedes_string = NIL (char);
   }
   get_fix_name_from_fix_info (disabler, name_lev_buf);
   disablee->supersedes_string = strdup (name_lev_buf);
   disablee->flags |= RENAMED_CONFLICTING_BASE; 

} /* mark_expl_superseding_base_conflict */
