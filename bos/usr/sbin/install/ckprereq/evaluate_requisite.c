static char sccsid[] = "@(#)60  1.20  src/bos/usr/sbin/install/ckprereq/evaluate_requisite.c, cmdinstl, bos41J, 9517A_all 4/21/95 20:37:42";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            comp_lev_with_req_expr
 *            determine_cause_of_failure 
 *            evaluate_requisite 
 *            get_failure_code
 *            get_lowest_relation_val
 *            is_compatible_lev_and_req_expr
 *            look_for_match_or_expl_sups
 *            look_for_match_or_impl_sups
 *            look_up_requisite 
 *            relation_matches
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

static boolean relation_matches (
int             value,
relation_type * relation);

static void determine_cause_of_failure (
char          * option_name,
criteria_type * criteria,
boolean       * error);

static char get_failure_code (
token_type   token,
boolean    * error);

static void look_for_match_or_expl_sups (
fix_info_type       * parent,
char                * option_name,
criteria_type       * criteria,
relation_type       * relation,
ENTRY              ** hash_entry,
possible_fixes_type * fix_header,
boolean             * error);

static void look_for_match_or_impl_sups (
fix_info_type       * parent,
char                * option_name,
criteria_type       * criteria,
criteria_type       * ifreq_updt_lev,
ENTRY               * hash_entry,
possible_fixes_type * fix_header,
boolean             * error);

static void look_up_requisite (
fix_info_type       * parent,
char                * option_name,
criteria_type       * criteria,
criteria_type       * ifreq_updt_lev,
possible_fixes_type * fix_header,
boolean               building_41_base_prq,
boolean             * error);

static short comp_lev_with_req_expr (
boolean         ptf_only,
fix_info_type * current, 
Level_t       * level,
criteria_type * req_expr);

/*
 * Used for flagging current event when looking for requisites.
 */
typedef enum {LOOKING_FOR_LOWEST_MATCH, LOOKING_FOR_SUPERSEDES} 
              req_match_event_type;

#define ONLY_PTF_IN_REQ_EXPR(criteria)                                        \
                  ((criteria->ptf          != NIL (relation_type))      &&    \
                   (criteria->version      == NIL (relation_type))      &&    \
                   (criteria->release      == NIL (relation_type))      &&    \
                   (criteria->modification == NIL (relation_type))      &&    \
                   (criteria->fix          == NIL (relation_type)))

/*--------------------------------------------------------------------------*
**
** NAME: evaluate_requisite
**
** FUNCTION:  This function determines which entries in the fix_info_table
**            satisfy the given criteria structure (representing a requisite 
**            expression from a prereq file) and links them as requisites (with
**            a group node if more than one matches).
**
**            NOTE: A visual representation of the criteria structure should
**            help:
**            ex.  *prereq A.obj v=3 o>3 r=2 would be saved as:
**
**            criteria = { version    release    modification   fix   ptf }
**         (criteria_type)    |         |             |          |     | 
**                            |         |             |          |     |
**                            v         v             v          v     v
**                         EQUALS     EQUALS         NIL        NIL   NIL
**                            3         2
**                            |         |
**                            v         v
**                      GREATER_THAN   NIL
**                            |
**                            3 
** 
**             each of ver, rel, mod, fix, ptf above are of "relation_type"
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void 
evaluate_requisite 
(
fix_info_type  * parent,
char           * option_name,
fix_info_type ** fix_chosen,
criteria_type  * criteria,
criteria_type  * ifreq_updt_lev,
requisite_type   type,
boolean          building_41_base_prq,
boolean        * error
)
{
   boolean               poss_fix_is_applied;
   possible_fixes_type   fixes_for_this_lpp;
   fix_info_type       * group_result;
   criteria_type         new_criteria;
   possible_fixes_type * possible_fix;
   short                 num_usable;
   short                 num_unusable;
   short                 total_num;

   /* We need to mark the prereq file with a dorky error code if this is
      the input file to the ckprereq command. */

   fixes_for_this_lpp.next_fix = NIL (possible_fixes_type);

   look_up_requisite (parent, option_name, criteria, ifreq_updt_lev,
                      & fixes_for_this_lpp, building_41_base_prq, error);
   RETURN_ON_ERROR;

  /* Do we have just one requisite to link into the graph, or is this a
     complex case?  If it is a complex case, then generate a group node with
     a required pass count of 1 */

   if (fixes_for_this_lpp.next_fix -> next_fix == NIL (possible_fixes_type) )
   {
     /* Normal case, just return the fix found. */

     * fix_chosen = fixes_for_this_lpp.next_fix -> fix_ptr;
   }
   else
   {
     /* Compound case. */
  
     /* 
      * When dealing with base levels..........
      * Before creating a group node, we want to see if we can avoid it
      * by seeing if we only REALLY have one choice to make, in an attempt
      * to reduce number of nodes created and also to reduce the potential
      * to spit out misleading group messages.  There are two ways in 
      * which a choice of two pkgs may really only be a choice of one:
      *   1. When we already made a decision in evaluate_base_levels (for
      *      apply operations)
      *   2. When one of our choices is already installed.
      */ 
     if (IF_INSTALL (fixes_for_this_lpp.next_fix->fix_ptr->op_type))
     {
        poss_fix_is_applied = FALSE;
        num_usable = num_unusable = total_num = 0;
        for (possible_fix = fixes_for_this_lpp.next_fix;
             possible_fix != NIL (possible_fixes_type);
             possible_fix = possible_fix->next_fix)
        {
           /* 
            * Look for base levels in possible_fixes list returned from
            * look_up_requisite ().
            */
           if (IF_INSTALL (possible_fix->fix_ptr->op_type))
           {
              /*
               * If this was tagged with our special tag bit, count it as
               * "unusable."  Otherwise, save it as the one to use and count
               * it as "usable."
               */
              if ( (possible_fix->fix_ptr->flags & CONFL_TOC_BASE_LEVEL) 
		                           ||
		  (possible_fix->fix_ptr->apply_state == CANNOT_APPLY))
              {
                 num_unusable++;
              }
              else
              {
                 if (possible_fix->fix_ptr->apply_state == ALL_PARTS_APPLIED)
                 {
                    poss_fix_is_applied = TRUE;
                 }
                 (* fix_chosen) = possible_fix->fix_ptr;
                 num_usable++;
              }
           } /* end if */
           total_num++;

        } /* end for */

        /*
         * If evaluate_base_level () eliminated all but one element on this
         * possible_fix_list, OR if one element is applied, then we should 
         * return that element.  Otherwise we'll go on to create the group node.
         */
        if ((poss_fix_is_applied) 
                         ||
            ((num_usable == 1) && ((num_unusable + num_usable) == total_num)))
        {
           return; 
        } /* end if */

     } /* end if */
       
     create_dummy_group (& group_result, error);
     RETURN_ON_ERROR;

     group_result -> passes_required = 1;

     /* Now link all of the requisites that we found into the graph as
        requisites of the parent_fix. */

     possible_fix = fixes_for_this_lpp.next_fix;
     while (possible_fix != NIL (possible_fixes_type) )
     {
        dupe_relations (criteria, & new_criteria, error);
        (void) link_requisite (group_result, possible_fix -> fix_ptr, 
                               & new_criteria, type, error);
        RETURN_ON_ERROR;

        possible_fix = possible_fix -> next_fix;
     }
     * fix_chosen = group_result;
   } /* end if */

   purge_fix_list (& fixes_for_this_lpp);

} /* end evaluate_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: look_up_requisite
**
** FUNCTION: This routine searches the fix_info table for matching requisites.
**           Matching entries are put on the fix_header linked list for 
**           further processing by callers.
**
** RETURNS:  Returns a pointer to the list of possible_fixes.  It may be nill.
**
**--------------------------------------------------------------------------*/

static void look_up_requisite 
(
fix_info_type       * parent,
char                * option_name,
criteria_type       * criteria,
criteria_type       * ifreq_updt_lev,
possible_fixes_type * fix_header,
boolean               building_41_base_prq,
boolean             * error
)
{
   boolean               ptf_in_expression;
   boolean               mark_dummy_as_migrating = FALSE;
   fix_info_type       * dummy_fix;
   Level_t               dummy_level;
   relation_type         dummy_relation;
   ENTRY               * hash_entry;
   relation_type       * relation;

   /* Our fixes are in the fix_info table. */

   /* Use the hash table to find out if there are any matches for each
      option_name/PTF combination.  If we find one, look at it and each of it's
      siblings on the collision_list. */

   if (criteria->ptf == NIL (relation_type))
   {
      dummy_relation.ptf_value[0] = '\0';
      dummy_relation.next_relation = NIL (relation_type);
      relation = & dummy_relation;
      ptf_in_expression = FALSE;
   }
   else
   {
      ptf_in_expression = TRUE;
      relation = criteria -> ptf;
   }

   /*
    * Do one or both of the following:
    * 1. Look for an exact match 
    *    -- if EXACT_MATCH bit in criteria structure is set.
    *       This is set by earlier called routines when the criteria represents
    *       a) a requisite expression containing a < operator 
    *       b) the level of a base level in an ifreq expression.
    *       c) a base level explicitly prereqed by a 4.1 update.
    *          (we should have already parsed this prereq in 
    *           build_base_prereqs but we're skipping over the requisite 
    *           expression here anyway while parsing the rest of the prereq 
    *           file.)
    *    -- if requisite expression has a ptf
    *    -- if dealing with 3.1 pkgs (don't want to change the way this 
    *       currently works due to inconsistencies in the way 3.1 pkgs
    *       may or may not have prereqed their base level -- using group reqs
    *       sometimes.
    * 2. Look for an "implicit" match
    *    Do this when none of the above hold OR when the above didn't find
    *    a match and we have a ptf in the requisite expression (we'll try
    *    to find a superseding base level in the latter case).
    *    We won't look for implicit matches for 3.1 type packages. 
    */
   if ((criteria->flags & EXACT_MATCH_REQUIRED)  ||
       ptf_in_expression                         ||
       IF_3_1 (parent->op_type)                  ||
       (check_prereq.called_from_command_line &&
        parent->origin == COMMAND_LINE)) /* Likely to be a 3.1 prereq
                                                file.  Force it down the
                                                "old way" of matching reqs
                                                to minimize unexpected
                                                behavior! */
   {
      look_for_match_or_expl_sups (parent, option_name, criteria, relation, 
                                   &hash_entry, fix_header, error);
      RETURN_ON_ERROR;

      if ((fix_header->next_fix == NIL (possible_fixes_type)) && 
           (ptf_in_expression || IF_3_1 (parent->op_type)))
         hash_entry = locate_hash_entry (option_name, "");
   }
   else
   {
      hash_entry = locate_hash_entry (option_name, "");
   } 

   if ((fix_header->next_fix == NIL (possible_fixes_type))               &&
       (hash_entry != NIL (ENTRY))                                       &&
       (!(IF_3_1 (parent->op_type) && IF_UPDATE (parent->op_type)))      && 
       (! (criteria->flags & EXACT_MATCH_REQUIRED)))
   {
      look_for_match_or_impl_sups (parent, option_name, criteria, 
                                   ifreq_updt_lev, hash_entry, 
                                   fix_header, error);
      RETURN_ON_ERROR;
   }

   /*
    * PREVENT UPDATES FROM INSTALLING ON A MIGRATING LPP:
    * Check to see if the requisite found has the migrating bit set.  If so,
    * AND if the requisite was from an option with the same name, then this is 
    * more than likely an update prereqing its base or another update from the
    * same product.  Don't let such an update install.  Remove the 
    * requisite fix_info structure and let subsequent code create a dummy node 
    * which will be marked as migrating for error reporting purposes.  
    * NOTE: this logic does not handle multiple matches on the 
    * possible_fix_list -- a very rare occurrence and highly
    * unlikely for the migrating scenario we're trying to catch.
    */
   if ((check_prereq.mode == OP_APPLY)                                    &&
       (fix_header->next_fix != NIL (possible_fixes_type))                &&
       (MIGRATING (fix_header->next_fix->fix_ptr->cp_flag))               &&
       (parent->parts_applied == 0)                                       &&
       (strcmp (parent->name, fix_header->next_fix->fix_ptr->name) == 0))
   {
       /*
        * free existing possible_fix node since we'll create another below.
        */
       fix_header->next_fix->next_fix = NIL (possible_fixes_type);
       fix_header->next_fix->fix_ptr = NIL (fix_info_type);
       free_possible_fix_node (fix_header->next_fix);
       fix_header->next_fix = NIL (possible_fixes_type);

       mark_dummy_as_migrating = TRUE;  /* subsequent code below uses this */
   }
       
   if (fix_header -> next_fix == NIL (possible_fixes_type))
   {
      /* There is always a header to our list of possible_fixes.  If our
         list of possible_fixes is empty, then there is no known package on
         the system or VPD that can satisfy the requisite that is being
         worked on.  In this situation, we don't even know the full name of
         the package, since the parser builds the list of possible_fixes
         from a requisite expression that could have multiple solutions (i.e
         *prereq fortran, any version PTF, etc.  would match).

         So...  if we have an empty list of possible_fixes, we create a
         dummy node to put in the graph.  We DO know what the option_name is
         for the requisite, so we copy that info to the node.
 
         We try to use only one dummy entry for each unique option to cut
         down on redundancy.  The criteria which point to those dummy 
         entries will distinguish the different requisites that actually
         exist on that dummy node.  */

      fix_header -> next_fix = create_possible_fix_node (error);
      RETURN_ON_ERROR;

      /*
       * Create a vrmf for the dummy node.  Use the actual vrmf from the
       * requisite expression if called from the up-front "requisite builder",
       * build_base_prereqs().  This enables processing in 
       * evaluate_superseding_updates() to go more smoothly.  Otherwise, use
       * the traditional means of creating a dummy node in the fix_info
       * table. (using -1 for the vrmf values)
       */
      if (building_41_base_prq)
      {
         dummy_level.ver = criteria->version->int_value;
         dummy_level.rel = criteria->release->int_value;
         dummy_level.mod = criteria->modification->int_value;
         dummy_level.fix = criteria->fix->int_value;
      }
      else
      {
         dummy_level.ver = -1;
         dummy_level.rel = -1;
         dummy_level.mod = -1;
         dummy_level.fix = -1;
      }
      dummy_level.ptf[0] = '\0';
      
      dummy_fix = locate_fix (option_name, & dummy_level, USE_FIXID);
      if (dummy_fix == NIL (fix_info_type) )
      {
         create_DUMMY_FIX (&dummy_fix, option_name, &dummy_level, error);
         RETURN_ON_ERROR;

         add_fix_to_fix_info_table (dummy_fix, error);
         RETURN_ON_ERROR;
      }

      if (mark_dummy_as_migrating)
        dummy_fix->cp_flag |= LPP_MIGRATING;

      if (check_prereq.mark_prereq_file)
      {
         determine_cause_of_failure (option_name, criteria, error);
         RETURN_ON_ERROR;
         unmark_graph (VISITING_THIS_NODE);
      }
      fix_header -> next_fix -> fix_ptr = dummy_fix;
   }

} /* end look_up_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: relation_matches
**
** FUNCTION: Given the linked list of relations, find out if any of them
**           match the given value.
**
** RETURNS:  boolean indicating the result.
**
**--------------------------------------------------------------------------*/

static boolean 
relation_matches 
(
int             value,
relation_type * relation
)
{
    if (relation == NIL (relation_type) )
       return (TRUE);

    for (relation = relation;
         relation != NIL (relation_type);
         relation = relation -> next_relation)
    {
        switch (relation -> operator)
        {
          case EQUALS :
            if (value == relation -> int_value)
               return (TRUE);
            break;

          case LESS_THAN :
            if (value < relation -> int_value)
               return (TRUE);
            break;

          case GREATER_THAN :
            if (value > relation -> int_value)
               return (TRUE);
            break;

          default :
            break;
        } /* end switch */

    } /* end for */
    return (FALSE);

} /* end relation_matches */

/*--------------------------------------------------------------------------*
**
** NAME: determine_cause_of_failure
**
** FUNCTION:  This routine is called when a requisite check has failed.
**            We analyize the criteria to figure out which search caused us to
**            fail.  This is a silly feature, but you gotta provide backwards
**            compatability.
**
** RETURNS:   A token_type indicating the search field that caused us to not
**            find anything.
**
**--------------------------------------------------------------------------*/

static void 
determine_cause_of_failure 
(
char          * option_name,
criteria_type * criteria,
boolean       * error
)
{
   boolean         at_least_one_found;
   token_type      failure_code;
   fix_info_type * fix;

   /* When called from the ckprereq command and while parsing the
      top-level prereq file, we have to mark column 1 of the prereq file
      with a code of what failed.  This is a real drag, because we have
      to figure out which search field caused us to not find any entries.
      Do this by repeating the lousy search, but using a much slower
      algorithm that mimics this behavior.  Technically, we could still
      have a compatability problem because we may be changing the order
      the search is done over what was specified by the customer (i.e.
      we always do it option_name.ver.rel.mod.fix.ptf whereas the customer
      can give almost any order option_name ptf ver fix mod rel).  I don't
      see why this should be a big problem.

      This is NOT a great feature, but was provided for backwards
      compatability. */

   /* Was it the option_name field that caused us to fail? */

   at_least_one_found = FALSE;
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      if ((strcmp (fix -> name, option_name) == 0) &&
          (fix->origin != DUMMY_FIX))
      {
         at_least_one_found = TRUE;
         fix -> flags |= VISITING_THIS_NODE;
      }
   } /* end for */

   if (! at_least_one_found)
   {
      failure_code = get_failure_code (A_NAME, error);
      RETURN_ON_ERROR;

      set_failure_code (failure_code, FALSE);
      return;
   }

   /* Was it the version field that caused us to fail? */

   at_least_one_found = FALSE;
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      if (fix -> flags & VISITING_THIS_NODE)
         if (! relation_matches (fix -> level.ver, criteria -> version))
         {
            fix -> flags &= ~ VISITING_THIS_NODE;
         }
         else
         {
            at_least_one_found = TRUE;
         }
   }

    if (! at_least_one_found)
    {
      failure_code = get_failure_code (VERSION, error);
      RETURN_ON_ERROR;

      set_failure_code (failure_code, FALSE);
      return;
    }

   /* Was it the release field that caused us to fail? */

   at_least_one_found = FALSE;
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      if (fix -> flags & VISITING_THIS_NODE)
         if (! relation_matches (fix -> level.rel, criteria -> release))
         {
           fix -> flags &= ~ VISITING_THIS_NODE;
         }
        else
         {
           at_least_one_found = TRUE;
         }
    }

    if (! at_least_one_found)
    {
      failure_code = get_failure_code (RELEASE, error);
      RETURN_ON_ERROR;

      set_failure_code (failure_code, FALSE);
      return;
    }

    /* Was it the modification field that caused us to fail? */

    at_least_one_found = FALSE;
    for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
    {
      if (fix -> flags & VISITING_THIS_NODE)
         if (! relation_matches (fix -> level.mod, criteria -> modification))
        {
           fix -> flags &= ~ VISITING_THIS_NODE;
        }
        else
        {
           at_least_one_found = TRUE;
        }
    }

    if (! at_least_one_found)
    {
      failure_code = get_failure_code (MODIFICATION, error);
      RETURN_ON_ERROR;

      set_failure_code (failure_code, FALSE);
      return;
    }

   /* Was it the fix field that caused us to fail? */

   at_least_one_found = FALSE;
   for (fix = check_prereq.fix_info_anchor -> next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      if (fix -> flags & VISITING_THIS_NODE)
         if (! relation_matches (fix -> level.fix, criteria -> fix))
         {
           fix -> flags &= ~ VISITING_THIS_NODE;
         }
        else
         {
           at_least_one_found = TRUE;
         }
   }

   if (! at_least_one_found)
   {
      failure_code = get_failure_code (FIX, error);
      RETURN_ON_ERROR;

      set_failure_code (failure_code, FALSE);
      return;
   }

    /* Ergo, it must have been the PTF field. */

   {
      failure_code = get_failure_code (PTF, error);
      RETURN_ON_ERROR;

      set_failure_code (failure_code, FALSE);
      return;
   }
} /* determine_case_of_failure */

/*--------------------------------------------------------------------------*
**
** NAME: get_failure_code
**
** FUNCTION:  This routine is called when a requisite check has failed
**            We make a wimpy attempt to figure out why it failed by marking
**            column 1 with a code.  Given a token_type, we return a character
**            to mark the line with.
**
** RETURNS:   A character indicating the failure.
**
**--------------------------------------------------------------------------*/

char get_failure_code 
(
token_type   token,
boolean    * error
)
{
   char failure_code;

   switch (token)
   {
       case A_NAME :
          failure_code = PREREQ_ERROR_FLAG;
          break;

       case FIX :
          failure_code = FIX_ERROR_FLAG;
          break;

       case MODIFICATION :
          failure_code = MODIFICATION_ERROR_FLAG;
          break;

       case PTF :
          failure_code = PTF_ERROR_FLAG;
          break;

       case RELEASE :
          failure_code =  RELEASE_ERROR_FLAG;
          break;

       case VERSION :
          failure_code = VERSION_ERROR_FLAG;
          break;

       default :
          /* "Internal error: Bad failure code in get_failure_code.\n" */
          inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                  CKP_INT_CODED_MSG_D), inu_cmdname, BAD_FAIL_CODE);

          check_prereq.function_return_code = INTERNAL_ERROR_RC;
          * error = TRUE;

   } /* end switch */

   return (failure_code);

} /* end get_failure_code */

/*--------------------------------------------------------------------------*
**
** NAME: look_for_match_or_expl_sups
**
** FUNCTION:  Looks for an exact match on the criteria provided, or if the
**            the criteria contains a ptf, looks for an explicit superseding
**            update for that ptf (ie. old style supersedes).
**
** RETURNS:   void function
** 
** SIDE EFFECTS:
**            fix_header may point to a list of matching fixes for the
**            criteria upon return.
**
**--------------------------------------------------------------------------*/

static void 
look_for_match_or_expl_sups
(
fix_info_type       * parent,
char                * option_name,
criteria_type       * req_expr,
relation_type       * relation,
ENTRY              ** hash_entry,
possible_fixes_type * fix_header,
boolean             * error
)
{
   fix_info_type       * fix;
   possible_fixes_type * possible_fix;
   possible_fixes_type * next_possible_fix;
   boolean               install_image_seen = FALSE;

   while (relation != NIL (relation_type) )
   {
      (*hash_entry) = locate_hash_entry (option_name, relation -> ptf_value);
      if ((*hash_entry) != NIL (ENTRY) )
      {
         for (fix = (fix_info_type *) ((*hash_entry) -> data);
              fix != NIL (fix_info_type);
              fix = fix -> collision_list)
         {
            /* Verify that the option_name is the same.  (Only do so if we
               didn't hash on a ptf id.)  */

            if ((relation->ptf_value[0] != '\0') &&
                strcmp (fix -> name, option_name) != 0)
               continue;

            /* Now see if the version, release, mod and fix fields match any
               of the set of versions that we are looking for in our
               req_expr. */

            if (! relation_matches (fix -> level.ver, req_expr -> version) )
               continue;

            if (! relation_matches (fix -> level.rel, req_expr -> release) )
               continue;

            if (! relation_matches (fix -> level.mod, req_expr->modification) )
               continue;

            /*
             * We won't require fixid matches for packages with ptfid.  (This
             * recognizes that superseding fixes may have different fix ids.)
             */
            if ((relation->ptf_value[0] == '\0') &&
                ! relation_matches (fix -> level.fix, req_expr -> fix) )
               continue;

            /*
             * Make sure we don't let 31 updates, which ambiguously prereq
             * their base, match with themselves.
             */
            if (IF_3_1_UPDATE (fix->op_type))
               if (fix == parent)
                  continue;
 
            if (((fix->apply_state == SUPD_BY_NEWER_BASE) &&
                 (!(req_expr->flags & EXACT_MATCH_REQUIRED)))
                                   ||
                IS_SUPD_COMPAT_ENTRY (fix))
               /* 
                * We'll leave resolution of this to the implicit sups routines.
                */
               return;

            /* Wow, this one made it, put it on the list. */
        
            possible_fix = create_possible_fix_node (error);
            RETURN_ON_ERROR;

            /*
             * If matched with a 3.2 ptf, resolve on the explicit supersedes
             * chain if necessary.
             */
            if ((fix->level.ptf[0] != '\0')  
                            &&
                (fix->apply_state == SUPD_BY_NEWER_UPDT ||
                 is_dummy_supd_vpd_rec (fix)))
            {
               fix = get_superseding_fix (fix);
            }

            possible_fix -> fix_ptr = fix;
            possible_fix -> next_fix = fix_header -> next_fix;
            fix_header -> next_fix = possible_fix;

            /* There is an obscure problem with 3.1 updates.  We want to only
               return just updates or just install images, never both.  The
               reason that this is a problem is that a requisite specification
               can be ambigious (i.e. v=1 r=2) which will match both install
               images and updates for 3.1 images. This can lead to us prereqing
               ourself.  So.. if there is an install image, return only it,
               otherwise, return the list. */

            if (IF_INSTALL (possible_fix -> fix_ptr -> op_type) )
               install_image_seen = TRUE;

         } /* end for */
      } /* end if */
      relation = relation -> next_relation;
   } /* end while */

   if (install_image_seen)
   {
      /* Get rid of everything that is not an install image. */

      possible_fix = fix_header -> next_fix;
      fix_header -> next_fix = NIL (possible_fixes_type);
      while (possible_fix != NIL (possible_fixes_type) )
      {
         next_possible_fix = possible_fix -> next_fix;
         if (! IF_INSTALL (possible_fix -> fix_ptr -> op_type) )
         {
            free (possible_fix);
         }
         else
         {
            possible_fix -> next_fix = fix_header -> next_fix;
            fix_header -> next_fix = possible_fix;
         }
         possible_fix = next_possible_fix;
      } /* end while */
   }
} /* look_for_match_or_expl_sups */

/*--------------------------------------------------------------------------*
**
** NAME: look_for_match_or_impl_sups
**
** FUNCTION:  Looks for an exact match on the criteria provided.  If no
**            exact match is found, looks for the first "usable"
**            implicitly superseding match (ie. greater v.r.m.f) that will 
**            satisfy the requisite.  ("usability" is established in the 
**            routines evaluate_base_levels() and 
**            evaluate_superseding_updates())
**
** RETURNS:   void function
** 
** SIDE EFFECTS:
**            fix_header may point to a list containing a single fix
**            which matches the criteria (exactly or as an implicit supersedes).
**
**--------------------------------------------------------------------------*/

static void 
look_for_match_or_impl_sups
(
fix_info_type       * parent,
char                * option_name,
criteria_type       * orig_req_expr,
criteria_type       * ifreq_updt_lev,   /* When non-zero, orig_req_expr is the
                                           parenthetic base level being 
                                           searched for for an ifreq.  This is 
                                           the non-paren vrmf of ifreq. */
ENTRY               * hash_entry,
possible_fixes_type * fix_header,
boolean             * error
)
{
   boolean               found = FALSE;
   boolean               ptf_only=FALSE;
   criteria_type         req_expr = EMPTY_CRITERIA;
   fix_info_type       * hash_fix = (fix_info_type *) ((hash_entry) -> data);
   fix_info_type       * current;
   fix_info_type       * fix;
   int                   rc;
   possible_fixes_type * possible_fix;
   req_match_event_type  event;

   if (orig_req_expr->flags & OLD_STYLE_REQ_EXPR)
   { 
      /*
       * We need to make our requisite expression structure conform to the
       * implicit matching we are about to perform.
       * (OLD_STYLE_REQ_EXPR is set when building the requisite expression in 
       *  the parsing routines.  Set if criteria contains v=, m=, etc.)
       * No need to do any conversion if the expression ONLY contained a ptf.
       */
      if (! (ptf_only = ONLY_PTF_IN_REQ_EXPR (orig_req_expr)))
      {
         /*
          * Conversion examples:
          * exs. v=2 o v=3   ----> 2.0.0.0
          *      v>3         ----> 4.0.0.0
          *      v=3 r=4 o>2 ----> 3.3.0.0    This one's a bit silly but needs
          *                                   to be handled just the same
          */
         req_expr = EMPTY_CRITERIA;
         req_expr.flags = orig_req_expr->flags;
         create_relation (&req_expr, VERSION, GTRTHAN_OR_EQUALS,
                          get_lowest_relation_val (orig_req_expr->version),
                          NIL (char), error);
                          RETURN_ON_ERROR;

         create_relation (&req_expr, RELEASE, GTRTHAN_OR_EQUALS,
                          get_lowest_relation_val (orig_req_expr->release),
                          NIL (char), error);
                          RETURN_ON_ERROR;

         create_relation (&req_expr, MODIFICATION, GTRTHAN_OR_EQUALS, 
                          get_lowest_relation_val (orig_req_expr->modification),
                          NIL (char), error);
                          RETURN_ON_ERROR;

         create_relation (&req_expr, FIX, GTRTHAN_OR_EQUALS,
                          get_lowest_relation_val (orig_req_expr->fix),
                          NIL (char), error);
                          RETURN_ON_ERROR;

         if (orig_req_expr->ptf != NIL (relation_type))
         {
            /*
             * The ptf id will not be used in the search, but will be used
             * to prevent a match on a base level with the SAME vrmf as 
             * that specified in the requisite expression.
             */
            create_relation (&req_expr, PTF, EQUALS, -1,
                             orig_req_expr->ptf->ptf_value, error);
                             RETURN_ON_ERROR;
         }
      }
      else
      {
         req_expr = (* orig_req_expr);
      }
   }
   else
   {
      req_expr = (* orig_req_expr);
   }

   /* 
    * We can't be sure which entry in the fix_info table the hash will 
    * return -- all we are sure about is that the name matches.  Still, we
    * have a sorted fix_info table and so...
    * If the current pkg is newer than the level specified by req_expr.....
    * (OR if we're only dealing with an old style req which just contained
    * a ptf id in the expression)......
    */ 
   if (ptf_only || (comp_lev_with_req_expr (FALSE, hash_fix, &hash_fix->level,
                                            &req_expr) > 0))
   {
      /*
       * Look (to the left) for 1st pkg with vrmf < req_expr OR
       * 1st pkg with a different name.
       */
      for (fix = hash_fix;
           fix != NIL (fix_info_type); /* not really necessary */
           fix = fix->prev_fix)
      {
         if ((strcmp (fix->name, hash_fix->name) != 0) /* guaranteed to stop
                                                          at list anchor */
                           ||
             ((! ptf_only)  &&
              ((comp_lev_with_req_expr (FALSE, fix, &fix->level, &req_expr)) 
                                                                       == -1)))
         { 
             /*
              * The fix we want is the previous one (accessible by the
              * next pointer!)
              */
             current = fix->next_fix;
             break;

         } /* end if */
      } /* end for */
   } 
   else 
   {
      /*
       * Hash entry must be <= to level in req expression.
       * We be looking right until we find our match
       * OR change of option OR end of table.
       */
      current = hash_fix;
   } /* end if */
  
   /* 
    * Loop (right) on fix_info table looking for lowest match on requisite expr.
    * At that point, either exit the loop (with our match) or
    * keep looking to the right (if the match is marked superseded).
    */ 
   event = LOOKING_FOR_LOWEST_MATCH; 
   while ((current != NIL (fix_info_type))  &&  (! found)  &&
          (strcmp (option_name, current->name) == 0))
   {
      switch (event)
      {
         case  LOOKING_FOR_LOWEST_MATCH:

         if ((rc = comp_lev_with_req_expr (ptf_only, current, &current->level,
                                           &req_expr)) == 0)
            /*
             * EXACT MATCH!
             */
            if (IS_SUPERSEDED(current)) 
               event = LOOKING_FOR_SUPERSEDES; 
            else
               found = TRUE;
         else
            if (rc == 1) 
            {
               /* 
                * The level of the current entry in the fix_info table is
                * greater than our requisite expression.  This could be the
                * match we're looking for (ie. an implicitly superseding
                * vrmf to that which we are looking for).
                *
                * Perform additional specialized checks to see if the current 
                * entry is that which is desired or if we should stop or
                * continue looking.
                */
               if (
                   /*
                    * Stop looking if the requisite expression has matched
                    * with it's parent.  This is normally an error indicating
                    * that something has prereqed itself.  For 4.1 updates,
                    * however, we could be looking for the base level which
                    * may not currently be known about (ie. not on media or
                    * installed) so stop here indicating that there is no
                    * match for the base level requisite.
                    */
                   ((parent == current) &&
                    (IF_UPDATE (current->op_type)) &&
                    (IF_4_1 (current->op_type)))

                             ||  /********* OR **************/

                   /*
                    * If the ifreq_updt_lev pointer has a value, we must be
                    * looking for the base level in an ifreq expression.
                    * (See parser.c for details of the base_lev_list which is
                    * used for ifreq processing.)
                    * ex. if the req is *ifreq A.obj (1.1.0.0) 1.1.1.1 , then
                    * ifreq_updt_lev will point to a structure for 1.1.1.1 
                    * while req_expr points to 1.1.0.0, which is what 
                    * we're trying to resolve against (with a superseding vrmf 
                    * if necessary).  STOP looking if the level of the current
                    * entry in the fix_info table is NOT less than the vrmf
                    * in ifreq_updt_lev (1.1.1.1 in the example above).
                    * Put another way, stop looking if some level >= to that in 
                    * ifreq_updt_lev is installed or being installed in which
                    * case the ifreq should not come into play.  We will stop
                    * the loop here, our caller will see nothing was returned,
                    * create a dummy entry, which will cause subsequent 
                    * ifreq processing to ignore the ifreq branch.
                    */
                   ((ifreq_updt_lev != NIL (criteria_type)) &&
                    ((rc = comp_lev_with_req_expr (FALSE, current, 
                                                  &current->level,
                                                  ifreq_updt_lev)) != -1)))
 
               {
                  found = TRUE;                        /* stop the loop */
                  current = NIL (fix_info_type);       /* return nothing */
               }
               else
               {
                  /* 
                   * This is a potential match.  If it's superseded, keep 
                   * looking.  If it's incompatible, return nothing.
                   * else return this entry.
                   */
                  if (IS_SUPERSEDED(current))
                     event = LOOKING_FOR_SUPERSEDES; 
                  else
                  {
                     found = TRUE;                        /* stop the loop */

                     if (IF_INSTALL (current->op_type) &&
                         ! is_compat_lev_and_req_expr (current, &req_expr))
                        current = NIL (fix_info_type);    /* make sure nothing
                                                             is returned since
                                                             this pkg is 
                                                             incompatible */
                  }
               }
            }
         break;     

         case LOOKING_FOR_SUPERSEDES:
          /*
           * We're in a valid supersedes chain, as determined by
           * evaluate_base_levels() and evaluate_superseding_updates().
           * Just need to look for 1st non-superseded pkg.
           */
         if (! IS_SUPERSEDED(current))
            found = TRUE;
         break;

      } /* switch */
      if (! found)
      {
         /*
          * Adjust lookup pointers and reference names if this is a 
          * compat entry (with a different name) so that we continue 
          * looking along the superseding branch.
          */
         if (IS_SUPD_COMPAT_ENTRY(current))
         {
            current = current->superseded_by->superseding_fix;
            option_name = current->name;
         }
         else
            current = current->next_fix;
      }

   } /* end while */ 
   
   if (found && (current != NIL (fix_info_type))) {  

      /* return match found on poss_fix list */ 
      possible_fix = create_possible_fix_node (error);
      RETURN_ON_ERROR;
      possible_fix -> fix_ptr = current;
      possible_fix -> next_fix = NIL (possible_fixes_type);
      fix_header -> next_fix = possible_fix;

   } else {                                          

      /* internal error check */
      if (event == LOOKING_FOR_SUPERSEDES)
      {
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                  CKP_INT_CODED_MSG_D), inu_cmdname, 
                  LOOKING_FOR_SUPS_IN_REQ_MATCH);
         * error = TRUE;

      }  /* end if */

   } /* end if */

} /* look_for_match_or_impl_sups */

/*--------------------------------------------------------------------------*
**
** NAME: comp_lev_with_req_expr
**
** FUNCTION:  Compares the contents of the Level_t structure in the fix_info 
**            structure argument with each member of the requisite expression
**            argument.  
** RETURNS:   1: if current->level > req_expr
**           -1: if current->level < req_expr
**            0: if current->level = req_expr
**            
**--------------------------------------------------------------------------*/

static short 
comp_lev_with_req_expr 
(
boolean         ptf_only,
fix_info_type * current, 
Level_t       * target_lev,
criteria_type * req_expr
)
{
   char barr_level [MAX_LEVEL_LEN];

   if (req_expr->flags & OLD_STYLE_REQ_EXPR)
   {
      if (! IF_INSTALL (current->op_type))
         /*
          * Can't match an old style req with anything but a base level.
          */
         return (-1);

      if (ptf_only)
      {
         /*
          * Match a requisite on a "ptf only" expression with an installed or 
          * installable 4.1 base level (which includes compatibility
          * entries) which does not have a supersedes entry (implying that 
          * it is compatible with some earlier level (including a 3.2 level)).
          */
         if (((IF_4_1 (current->op_type)) && 
              (current->apply_state <= IN_TOC)) /* Installed or installable? */
                               &&
             (get_barrier_from_sups_str (current->name,
                                         current->supersedes_string,
                                         barr_level)
                                == NIL (char))) /* no barrier? */
            return (0);    /* call this a match */                    
         else
            return (-1);   /* current will be considered older than target
                              requisite expression. */
      } /* end if */
   } /* end if */

   if (target_lev->ver > req_expr->version->int_value)
      return (1);
   else
      if (target_lev->ver < req_expr->version->int_value)
         return (-1);

   if (target_lev->rel > req_expr->release->int_value)
      return (1);
   else
      if (target_lev->rel < req_expr->release->int_value)
         return (-1);

   if (target_lev->mod > req_expr->modification->int_value)
      return (1);
   else
      if (target_lev->mod < req_expr->modification->int_value)
         return (-1);

   if (target_lev->fix > req_expr->fix->int_value)
      return (1);
   else
      if (target_lev->fix < req_expr->fix->int_value)
          return (-1);

   if (req_expr->ptf != NIL (relation_type))  /* req had a ptf id, can't match
                                                 on something with same vrmf */
      return (-1);
   else
      return (0); /* must be equal */

} /* comp_lev_with_req_expr */

/*--------------------------------------------------------------------------*
**
** NAME: is_compatible_lev_and_req_expr
**
** FUNCTION:  Compares the barrier/supersedes info in the fix_info_structure
**            argument with the level in req_expr argument.
** 
** RETURNS:   TRUE if no supersedes info  OR if barrier <= req_expression_level
**            FALSE otherwise.
**
**--------------------------------------------------------------------------*/

static boolean 
is_compat_lev_and_req_expr 
(
fix_info_type * fix, 
criteria_type * req_expr
)
{
   char       barrier_str [MAX_LEVEL_LEN];
   Level_t    barrier_lev;

   if (get_barrier_from_sups_str (fix->name,
                                  fix->supersedes_string,
                                  barrier_str) == NIL (char))
      /*
       * No barrier/supersedes info.  Must be compatible.
       */
      return (TRUE); 

   /*
    * Get barrier info in a level structure.
    */
   inu_level_convert (barrier_str, &barrier_lev); 

   /*
    * Return result of comparison.
    */
   return (comp_lev_with_req_expr (FALSE, fix, &barrier_lev, req_expr) <= 0 );

} /* is_compatible_lev_and_req_expr */

/*--------------------------------------------------------------------------*
**
** NAME: get_lowest_relation_val
**
** FUNCTION:  Given a list of relations for a version, release, mod, or fix 
**            this routine determines the lowest integer value
**            in that list.
** 
** RETURNS:   int representing the lowest value in the list of relations.
**            
**--------------------------------------------------------------------------*/

static int
get_lowest_relation_val
(
relation_type * relation
)
{
    boolean         seen_equals     = FALSE;
    boolean         seen_gtr_than   = FALSE;
    int             lowest_equals   = 0;
    int             lowest_gtr_than = 0;
    relation_type * tmprel;
    
    if (relation == NIL (relation_type) )
       return (0);

    for (tmprel = relation;
         tmprel != NIL (relation_type);
         tmprel = tmprel -> next_relation)
    {
        switch (tmprel -> operator)
        {
          case EQUALS :
               if ((! seen_equals) || (tmprel->int_value < lowest_equals))
                  lowest_equals = tmprel->int_value;
               seen_equals = TRUE;

            break;

          case GREATER_THAN :
            if ((! seen_gtr_than) || (tmprel->int_value < lowest_gtr_than))
                lowest_gtr_than = tmprel->int_value; 
             seen_gtr_than = TRUE;
            break;

          default :
            break;
        } /* end switch */

    } /* end for */
  
    if (seen_equals)
       if (seen_gtr_than)
          if (lowest_equals <= lowest_gtr_than)
             return (lowest_equals);
          else
             return (lowest_gtr_than+1);
       else
          return (lowest_equals);
    else
       return (lowest_gtr_than+1);

} /* get_lowest_relation_val */
