static char sccsid[] = "@(#)63  1.35.1.33  src/bos/usr/sbin/install/ckprereq/parser.c, cmdinstl, bos41B, 412_41B_sync 1/4/95 20:23:14";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            build_subgraph  
 *            get_implied_ifreq_base_level
 *            parse_assertion
 *            parse_base_level 
 *            parse_dotted_number
 *            parse_or_expression
 *            parse_relation 
 *            parse_relation_expression 
 *            parse_requisite_item 
 *            parse_requisite_group 
 *            parsing_error
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
 * Static declarations.
 */
static void parse_requisite_item (
fix_info_type  * group_parent,
fix_info_type  * parent_fix,
boolean        * error);

static void parse_assertion (
fix_info_type   * parent,
fix_info_type  ** fix_chosen,
criteria_type   * ifreq_base_criteria,
criteria_type   * criteria,
requisite_type  * type,
int             * VRMFP_seen,
boolean         * error);

static boolean parse_relation_expression (
int                  * VRMFP_seen,
token_type           * VPD_field,
boolean                base_level_seen,
criteria_type        * criteria,
boolean                first_time_through,
boolean              * error);

static void parse_base_level (
boolean       * base_level_found,
criteria_type * criteria,
boolean       * error);

static void parse_dotted_number (
token_type      operator,
criteria_type * criteria,
boolean       * error);

static void parse_or_expression (
token_type      VPD_field,
criteria_type * criteria,
boolean       * error);

static void parse_relation (
token_type      VPD_field,
criteria_type * criteria,
boolean       * error);

static void parse_requisite_group (
fix_info_type   * group_parent,
fix_info_type  ** fix_chosen,
boolean         * error);

static void parsing_error (
parser_error_type   parsing_error,
boolean           * error);

void get_implied_ifreq_base_level (
criteria_type * base_criteria,
criteria_type * specified_level,
boolean       * error);

static void recover_from_parsing_error (
fix_info_type * error_node,
boolean         first_time,
boolean       * error);

#define VERSION_SEEN      0x1
#define RELEASE_SEEN      0x2
#define MODIFICATION_SEEN 0x4
#define PTF_SEEN          0x8
#define FIX_SEEN          0x10

/*--------------------------------------------------------------------------*
**
** NAME: build_subgraph
**
** FUNCTION:  This is the entry point to the parser for a prereq file.
**            Given a fix_info_type node, which represents a fix/product
**            that needs to have it's prereq file checked, build_subgraph
**            will add any additional prereq and coreq nodes that need to
**            be applied before this one.  Status information is added to
**            the given node indicating the results of the prereq check.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void build_subgraph 
(
fix_info_type * parent_fix,
boolean       * error
)
{
    boolean seen_same_name;

    /* 
     * Don't even waste time building the requisite chains of superseded
     * pkgs.  Why:  it's assumed that parents inherited the requisites and
     * so, any dependencies on or held by the superseded pkg will be satisfied 
     * by the superseding pkg.  Also, don't worry about available records, 
     * of which there shouldn't be any in the first place, for reject and 
     * deinstall ops.  Finally, ignore the subgraphs of MIGRATING filesets.
     * There's very little that can or should be done about their missing 
     * requisites.
     */
    if ((IS_SUPERSEDED (parent_fix)) 
                      ||
        (MIGRATING (parent_fix->cp_flag))
                      ||
        ((check_prereq.mode == OP_REJECT) &&     /* reject OR deinstall op */
         (parent_fix->apply_state == AVAILABLE)))
    {
       if (parent_fix -> prereq_file != NIL (char))
       {
          my_free (parent_fix -> prereq_file);
          parent_fix -> prereq_file = NIL (char);
       }
       return;
    }

    if (   (parent_fix -> prereq_file == NIL (char))
                             ||
         (* (parent_fix -> prereq_file) == NULL_CHAR))
    {
       return;   /* Nothing to do. */
    }

    if (check_prereq.called_from_command_line &&
        parent_fix->origin == COMMAND_LINE)
       check_prereq.mark_prereq_file = TRUE;
    else
       check_prereq.mark_prereq_file = FALSE;

    initialize_lexical_analyzer (parent_fix);

    /*  prereq_file     : requisite_items  */
    /*  requisite_items : requisite_item
                        | requisite_item requisite_items  */

    parsers.token = get_token (error);
    if (*error)
    {
       recover_from_parsing_error (parent_fix, TRUE, error);
       return;
    }
 
    while (parsers.token != END_OF_FILE)
    {
       parse_requisite_item (NIL (fix_info_type), parent_fix, error);
       if (*error)
       {
          recover_from_parsing_error (parent_fix, TRUE, error);
          return;
       }
    }

    /*
     * We only need the prereq string for the ckprereq command line 
     * "prereq file entry" in the fix_info table.  (We'll write some
     * prereq error codes there).
     */
    if (! check_prereq.mark_prereq_file) 
    {
       my_free (parent_fix -> prereq_file);
       parent_fix->prereq_file = NIL (char);
    }

    return;

} /* end build_subgraph */

/*--------------------------------------------------------------------------*
**
** NAME: parse_requisite_item
**
** FUNCTION:  This routine is responsible for parsing a requisite_item:
**
**
**              requisite_item : IFREQ assertion
**                             | IFREQ base_level_assertion 
**                             | PREREQ assertion
**                             | COREQ assertion
**                             | assertion
**                             | '>' requisite_group
**                             | END_OF_FILE
**
**            This routine receives a fix_info node as the result of
**            evaluating an assertion or requisite_group.  This fix node is
**            placed on the requisite list of the parent
**            node.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_requisite_item 
(
fix_info_type  * group_parent,
fix_info_type  * parent_fix,
boolean        * error
)
{
   char              fix_name[MAX_LPP_FULLNAME_LEN];
   criteria_type     criteria = EMPTY_CRITERIA;
   boolean           find_exact_match;
   fix_info_type   * fix_chosen;
   fix_info_type   * real_parent;
   fix_info_type   * base_lev;
   criteria_type     ifreq_base_criteria = EMPTY_CRITERIA;
   requisite_list_type * req_node;
   requisite_type    type;
   int               VRMFP_seen;  /* used to see if ptf exists in expression
                                     for illegal 4.1 ifreq test. */

   RETURN_ON_ERROR;

   parsers.start_of_current_expression = parsers.line_number;

   /*
    * Error check: 32 MC update can only contain prereqs.
    */
   if ((IF_C_UPDT (parent_fix->op_type))                &&
        (IF_3_2 (parent_fix->op_type))                  &&
        (parsers.token != PREREQ)                       &&
        (parsers.token != A_NAME))
   {
      /*
       * Environment variable causes this error to be ignored -- purely for
       * purposes of preserving test cases with data which already violated
       * these rules.
       */
      if (getenv ("IGNORE_86603_ERR") == NIL (char))
      {
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INVALID_MC_REQ1_E,
                  CKP_INVALID_MC_REQ1_D), inu_cmdname,
                  get_fix_name_from_fix_info (parent_fix, fix_name));
         check_prereq.function_return_code = SEMANTIC_ERROR_RC;
         *error = TRUE;
         return;
      }
   }

   /*
    * In routines which are called from here, we need to remember who
    * the parent of a group node was.  (You should always remember who your 
    * parents are anyway! :-))
    */
   if (parent_fix->origin == DUMMY_GROUP)
      real_parent = group_parent;
   else
      real_parent = parent_fix;
 
   switch (parsers.token)
   {
      case IFREQ  :
         parsers.token = get_token (error);
         RETURN_ON_ERROR;

         type = AN_IFREQ;
         parse_assertion (real_parent, & fix_chosen, & ifreq_base_criteria,
                          & criteria, & type, & VRMFP_seen, error);
         RETURN_ON_ERROR;

         /*
          * Make sure that 4_1 packages refer to a base level. 
          * (ie. AN_IFFREQ).  The dotted notation (or newest style requisite)
          * implies a base level.  Here we check for the old style requisite
          * in conjunction with a ptf id.  If this case exists, we need to 
          * have a parenthetic base level specified (ie. AN_IFFREQ).
          */ 
         if (IF_4_1 (parent_fix->op_type)           && 
             (criteria.flags & OLD_STYLE_REQ_EXPR)  && 
             (VRMFP_seen & PTF_SEEN)                &&
             type != AN_IFFREQ)
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_ILLEGAL_OLD_IFREQ_E,
                     CKP_ILLEGAL_OLD_IFREQ_D), 
                     inu_cmdname,
                     get_fix_name_from_fix_info (parent_fix, fix_name),
                     parent_fix->prereq_file);
            *error = TRUE;
            check_prereq.function_return_code = SEMANTIC_ERROR_RC;
            return;
         }
         parent_fix->flags |= FIX_HAS_IFREQS; /* used when considering
                                                 unresolved ifreqs.*/

         req_node = link_requisite (parent_fix, fix_chosen, & criteria, 
                                    type, error);
         RETURN_ON_ERROR;
 
         /* 
          * Since this is an ifreq, we want our requisite to point to the 
          * base level of its product (this will be used when evaluating
          * ifreqs in our graph).  A call to evaluate_requisite(), using the 
          * base level as search criteria (returned from parse_assertion())  
          * will find that information for us.
          *
          * First, set a flag telling the look-up routines that an exact match
          * should be found for the base level.  ONLY do this if dealing
          * with ifreqs on 3.2 ptfs, since for 4.1 updates, we want superseding
          * base levels to cause the ifreq to kick in also.  
          */

         if (VRMFP_seen & PTF_SEEN)
            ifreq_base_criteria.flags |= EXACT_MATCH_REQUIRED; 
         if (fix_chosen->origin == DUMMY_GROUP)
            /*
             * If we got back a dummy_group from parse_assertion, this means
             * we had an "OR" in our ifreq expression.  (meaning we had an
             * ifreq of the form *ifreq X p=U1 o=U2).  Make sure we use one
             * of the group members to establish the link to the base_product.
             */
            evaluate_requisite (real_parent, 
                                fix_chosen->requisites->fix_ptr->name,
                                & base_lev, & ifreq_base_criteria, 
                                & criteria, type, FALSE, error);
         else
            evaluate_requisite (real_parent,
                                parent_fix->requisites->fix_ptr->name,
                                & base_lev, & ifreq_base_criteria, 
                                & criteria, type, FALSE, error);
         RETURN_ON_ERROR;

         /*
          * Now add a pointer to the base level returned from the above call
          * to the base_lev_list of the requisite node returned from 
          * link_requisite.
          * 
          * NOTES:
          * The base_lev_list structure is a list chaining off
          * a requisite node.  It's primary purpose is to facilitate
          * ifreq processing and is used to point to the base level
          * specified in an ifreq expression.  The structure is a
          * list because it is possible to specify more than one
          * base level for a given update being ifreqed:
          *
          * ex.
          *  *ifreq A.rte (1.2.0.0) 1.2.1.1
          *  *ifreq A.rte (1.2.1.0) 1.2.1.1
          *
          * The intent of this ifreq is to force installation of the
          * update 1.2.1.1 if the fileset A.rte is installed at
          * 1.2.0.0 OR at 1.2.1.0.
          *
          * Internally, assuming the fileset Z.rte has the above
          * requisites, this would be represented as follows:
          *
          *  check_prereq.fix_info_anchor---+
          *                                 |
          *     +---------------------------+
          *     |
          *     v
          *           next            next            next           next
          *  F:A.rte ------> F:A.rte ------> F:A.rte ------> F:Z.rte----->...
          *  1.2.0.0          1.2.1.0        1.2.1.1         4.1.0.0
          *     ^               ^                ^              |
          *     |               |                |              |requisites
          *     |               |                |  fix_ptr     |
          *     |               |                +--------------R
          *     |fix_ptr        |fix_ptr                       /|
          *     |               |                             / |next_req
          *     |               |                            /  :
          *     | next_base_lev |      base_lev_list        /   :
          *     B<--------------B<-------------------------+
          *
          * Legend:
          *   F: fix_info_type structure
          *   R: requisite_list_type structure
          *   B: base_lev_list_type structure
          */
         add_unique_node_to_base_lev_list  (req_node, base_lev, error);
         RETURN_ON_ERROR;
                
         break;

      case COREQ  :
         parsers.token = get_token (error);
         RETURN_ON_ERROR;

         type = A_COREQ;
         parse_assertion (real_parent, & fix_chosen, NIL (criteria_type),
                          & criteria, & type, &VRMFP_seen, error);
         RETURN_ON_ERROR;

         (void) link_requisite (parent_fix, fix_chosen, &criteria, type, error);
         RETURN_ON_ERROR;
         break;

      case GREATER_THAN :
         parsers.token = get_token (error);
         RETURN_ON_ERROR;

         parse_requisite_group (real_parent, & fix_chosen, error);
         RETURN_ON_ERROR;

         (void) link_requisite (parent_fix, fix_chosen, &criteria, A_PREREQ,  
                                error);
         RETURN_ON_ERROR;

         break;

      case INSTREQ :
         /*
          * New requisite type.  This requisite requires that a package
          * be installed before or with the package with the instreq.
          * The only difference between instreq and prereq is that packages
          * will not be "pulled in" via auto-include with instreqs.
          * On non-apply ops, instreq will be treated like prereq.
          */
 
         /*
          * First check to make sure updates don't use the instreq type.
          * (This may seem somewhat arbitrary right now but an update must
          * prereq its own base level.  Code exists elsewhere to check this
          * and for now, we don't want to have to modify that code to
          * allow for when an update may instreq its own base product.  This
          * restriction may be lifted at a later date if someone has a good
          * reason for an update to instreq another.)
          */
         if (IF_UPDATE (parent_fix->op_type))
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NO_UPDATE_INSTREQS_E,
                     CKP_NO_UPDATE_INSTREQS_D), inu_cmdname,
                     get_fix_name_from_fix_info (parent_fix, fix_name));
            check_prereq.function_return_code = SEMANTIC_ERROR_RC;
            *error = TRUE;
            return;
         }

         /*
          * Only do the normal parse/link operation in the case of an apply.
          * For non-apply ops (and ckprereq command), fall into the next case 
          * branch and continue from there ie. treat like a prereq.
          */
         if (check_prereq.mode == OP_APPLY &&
             ! check_prereq.called_from_command_line)
         {
            parsers.token = get_token (error);
            RETURN_ON_ERROR;

            type = AN_INSTREQ;
            parse_assertion (real_parent, & fix_chosen, NIL (criteria_type),
                             & criteria, & type, & VRMFP_seen, error);
            RETURN_ON_ERROR;

            (void) link_requisite (parent_fix, fix_chosen, & criteria, type,
                            error);
            RETURN_ON_ERROR;
            break;
         }


      case PREREQ :
         parsers.token = get_token (error);
         RETURN_ON_ERROR;

      case A_NAME :  /* must be an assertion, default to PREREQ */

         if ((IF_C_UPDT (parent_fix->op_type))                &&
             (IF_3_2 (parent_fix->op_type))                   &&
             (strcmp (parsers.tokens_string_value, parent_fix -> name) != 0))
         {
             if (getenv ("IGNORE_86603_ERR") == NIL (char))
             {
                inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INVALID_MC_REQ2_E,
                         CKP_INVALID_MC_REQ2_D), inu_cmdname,
                         get_fix_name_from_fix_info (parent_fix, fix_name));
                check_prereq.function_return_code = SEMANTIC_ERROR_RC;
                *error = TRUE;
                return;
             }
         }

         type = A_PREREQ;
         parse_assertion (real_parent, & fix_chosen, NIL (criteria_type),
                          & criteria, & type, & VRMFP_seen, error);
         RETURN_ON_ERROR;

         (void) link_requisite (parent_fix, fix_chosen, & criteria, type,
                         error);
         RETURN_ON_ERROR;

         break;

      case END_OF_FILE :
        return;  /* We is done!  */

      default     :

         #pragma covcc !instr
         parsing_error (GROUP_OR_REQUISITE_TYPE, error);
         return;
         #pragma covcc instr

   } /* end switch (parsers.token) */

} /* end parse_requisite_item */

/*--------------------------------------------------------------------------*
**
** NAME: parse_assertion
**
** FUNCTION:  This routine is responsible for parsing an assertion:
**
**                    assertion : option_name relation_expressions
**                              | option_name dotted_number
**                              | option_name
**
**         base_level_assertion : option_name base_level ptf_relation_expr
**                              | option_name base_level dotted_number
**
**         relation_expressions : relation_expression
**                              | relation_expressions relation_expression
**
**            This routine returns back via fix_chosen, a node that
**            represents the most appropriate fix/product that resolves, or
**            could resolve the relation_expressions parsed here.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_assertion 
(
fix_info_type   * parent,
fix_info_type  ** fix_chosen,
criteria_type   * ifreq_base_criteria,
criteria_type   * criteria,
requisite_type  * type,
int             * VRMFP_seen,
boolean         * error
)
{
   boolean                base_level_found;
   char                   fix_name [MAX_LPP_FULLNAME_LEN];
   fix_info_type        * child_fix;
   fix_info_type        * dummy_fix;
   Level_t                lev1;
   Level_t                lev2;
   possible_fixes_type  * fail_list;
   possible_fixes_type  * fix;
   fix_info_type        * group_result;
   possible_fixes_type  * possible_fix;
   char                   failure_code;
   boolean                first_time_through;
   char                 * option_name;
   int                    numeric_value;
   char                 * PTF_value;
   token_type             relation;
   boolean                relation_expression_present;
   token_type             VPD_field;


  
   (*VRMFP_seen) = 0; /* Used to detect erroneous repeats of V, R, M, F, or P
                         in an expression.  Also used to let one of our
                         callers know the contents of a requisite expression. */


    #pragma covcc !instr
    if (parsers.token != A_NAME)
    {
       parsing_error (LPP_NAME_EXPECTED, error);
       return;
    }
    #pragma covcc instr

   /*
    * Error check: a 4.1 update may only prereq a fileset with the same name.
    * ie. no coreq, ifreq etc.
    */
   if ((IF_4_1 (parent->op_type))                                &&
       (IF_UPDATE (parent->op_type))                             &&
       (strcmp (parent->name, parsers.tokens_string_value) == 0) &&
       (*type != A_PREREQ))
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ,
               CKP_41_UPDT_NO_PRQ_SAME_NAME_E,
               CKP_41_UPDT_NO_PRQ_SAME_NAME_D),
               inu_cmdname, get_fix_name_from_fix_info (parent, fix_name));
      check_prereq.function_return_code = SEMANTIC_ERROR_RC;
      *error = TRUE;
      return;
   } /* end if (IF_4_1... */



   /* First, query the various sources of fixes to get every entry for this
      LPP.

      Next, pass this table on, where all entries that fail the various tests
      have reduced the table to all fixes which have, or possibly could
      satisfy this assertion.  */

   option_name = dupe_string (parsers.tokens_string_value, error);
   RETURN_ON_ERROR;

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

   if (* type == AN_IFREQ)
   {
      parse_base_level (& base_level_found, ifreq_base_criteria, error);
      RETURN_ON_ERROR;

      if (base_level_found)
         * type = AN_IFFREQ;

   } 
   else
   {
      base_level_found = FALSE;
   }

   /*  Do a pretest to make sure that an 'or' condition is not the first */
   /*  Also verify that we don't have junk. */

   relation_expression_present = TRUE;
   switch (parsers.token)
   {
      #pragma covcc !instr
      case OR :
         parsing_error (OR_INVALID, error);
         return;
      #pragma covcc instr

      case VERSION :
      case RELEASE :
      case MODIFICATION :
      case PTF :
      case FIX :  
      break;

      default :
         relation_expression_present = FALSE;
   } /* end switch (parsers.token) */


   if (relation_expression_present)
   {
      criteria->flags |= OLD_STYLE_REQ_EXPR;
      first_time_through = TRUE;
      while (parse_relation_expression (VRMFP_seen, & VPD_field,
                                        base_level_found, criteria,
                                        first_time_through, error))
      {
         RETURN_ON_ERROR;

         first_time_through = FALSE;
         parsers.token = get_token (error);
         RETURN_ON_ERROR;
      }
      RETURN_ON_ERROR;

      if (* type == AN_IFFREQ)
      {
         /*
          * This must be an ifreq of the form *ifreq <name> (<level>) p=<ptf>
          * Save the base level version for requisite failure reports.
          */
         create_relation (criteria, VERSION, EQUALS, 
                          ifreq_base_criteria->version->int_value, 
                          NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (criteria, RELEASE, EQUALS, 
                          ifreq_base_criteria->release->int_value, 
                          NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (criteria, MODIFICATION, EQUALS, 
                          ifreq_base_criteria->modification->int_value, 
                          NIL (char), error);
         RETURN_ON_ERROR;
         create_relation (criteria, FIX, EQUALS, 
                          ifreq_base_criteria->fix->int_value, 
                          NIL (char), error);
         RETURN_ON_ERROR;
      }        
   }
   else
   {
      if (parsers.token == A_DOTTED_NUMBER)
      {
         if (parsers.dotted_number_count != 4)
         {
            inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                                       CKP_DOTTED_NUM_MUST_HAVE_4_PTS_E, 
                                       CKP_DOTTED_NUM_MUST_HAVE_4_PTS_D),
                     inu_cmdname, 
                     get_fix_name_from_fix_info (parent, fix_name));
            check_prereq.function_return_code = SEMANTIC_ERROR_RC;
            *error = TRUE;
            return;
         }
         /*
          * Check to see if the requisite we are parsing is an updates'
          * prereq on its base level.  If so, make sure the requisite 
          * lookup routines know that an exact match is required (ie. no
          * implicit supersedes).
          */
         if ((strcmp (parent->name, option_name) == 0) &&
             (IF_UPDATE (parent->op_type)))
         {
            parse_dotted_number (EQUALS, criteria, error); 
         }
         else
            parse_dotted_number (GTRTHAN_OR_EQUALS, criteria, error); 
         RETURN_ON_ERROR;

         if (*type == AN_IFREQ) 
         {
            /*
             * We have yet to establish base level criteria for this dotted 
             * level ifreq expression (since it's still marked as AN_IFREQ), 
             * meaning the ifreq expression was of the form:
             * *ifreq <name> <dotted_level>.  Let's figure out a base level
             * implied by the ifreq expression. (ie. what base level should
             * be checked to even consider this ifreq.)
             */
            get_implied_ifreq_base_level (ifreq_base_criteria, criteria, error); 
            RETURN_ON_ERROR;
            *type = AN_IFFREQ;
         }
         else
         {
            if (*type == AN_IFFREQ)
            {
              /*
               * requisite must have been of the form:
               * *ifreq <name> (<dotted_level>) <dotted_level>
               * Make sure the 2nd dotted level is greater than the 1st.
               */
              lev1.ver = ifreq_base_criteria->version->int_value;
              lev1.rel = ifreq_base_criteria->release->int_value;
              lev1.mod = ifreq_base_criteria->modification->int_value;
              lev1.fix = ifreq_base_criteria->fix->int_value;
              lev1.ptf[0] = '\0';

              lev2.ver = criteria->version->int_value;
              lev2.rel = criteria->release->int_value;
              lev2.mod = criteria->modification->int_value;
              lev2.fix = criteria->fix->int_value;
              lev2.ptf[0] = '\0';

              if (inu_level_compare (&lev1, &lev2) >= 0)
              {
                inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                         CKP_ILLEGAL_BASE_IFREQ2_E, 
                         CKP_ILLEGAL_BASE_IFREQ2_D),
                         inu_cmdname, 
                         get_fix_name_from_fix_info (parent, fix_name));
                 check_prereq.function_return_code = SEMANTIC_ERROR_RC;
                 *error = TRUE;
                 return;
              }
            }
         } /* end if */
      }  /* end if */
   } /* end if */

   /*
    * Now that we've parsed our requisite expression completely, let's
    * evaluate it and retrieve a single node that will represent the required
    * package.  Note: the returned node (fix_chosen) may be a group node 
    * pointing to multiple requisites.
    * 
    * First, check to see if the requisite we are parsing is an updates'
    * prereq on its base level.  If so, make sure the requisite 
    * lookup routines know that an exact match is required (ie. no
    * implicit supersedes).  NOTE: We only do this for non-41 updates.
    */
   if ((strcmp (parent->name, option_name) == 0) &&
       (IF_UPDATE (parent->op_type))             &&
       (! IF_4_1 (parent->op_type))              &&
       ! (*VRMFP_seen & PTF_SEEN))   /* don't worry about reqs on other ptfs */ 
   {
      criteria->flags |= EXACT_MATCH_REQUIRED;
   }

   /*
    * If we didn't parse a vrmf above, say that this is an old-style
    * requisite so that subsequent lookup routines will create a 
    * vrmf of 0.0.0.0 and implicit lookup mechanisms will function as
    * normal.
    */
   if ((criteria->version == NIL (relation_type))      &&
       (criteria->release == NIL (relation_type))      &&
       (criteria->modification == NIL (relation_type)) &&
       (criteria->fix == NIL (relation_type))          &&
       (criteria->ptf == NIL (relation_type)))
   {
      criteria->flags |= OLD_STYLE_REQ_EXPR;
   }

   evaluate_requisite (parent, option_name, fix_chosen, criteria, 
                       NIL (criteria_type), *type, FALSE, error);
   RETURN_ON_ERROR;

   my_free (option_name);
   option_name = NIL (char);
   return;

}  /* end parse assertion */

/*--------------------------------------------------------------------------*
**
** NAME: parse_base_level
**
** FUNCTION:  This routine is responsible for parsing a base_level.
**
**
**                   base_level : '(' dotted_number ')'
**                              | <null>
**
**                dotted_number : number
**                              | dotted_number '.' number
**
**            This routine parses an optional base_level expression.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_base_level (boolean       * base_level_found,
                              criteria_type * criteria,
                              boolean       * error)
 {
   if (parsers.token != OPEN_PAREN)
    {
      * base_level_found = FALSE;
      return;
    } /* end if */

   * base_level_found = TRUE;

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

   parse_dotted_number (EQUALS, criteria, error);
   RETURN_ON_ERROR;

   #pragma covcc !instr
   if (parsers.token != CLOSE_PAREN)
    {
       parsing_error (CLOSE_PAREN_EXPECTED, error);
       return;
    }
   #pragma covcc instr

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

 } /* end parse_base_level */

/*--------------------------------------------------------------------------*
**
** NAME: parse_dotted_number
**
** FUNCTION:  This routine is responsible for parsing a dotted_number.
**
**
**                dotted_number : number
**                              | dotted_number '.' number
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_dotted_number 
(
token_type      operator,
criteria_type * criteria,
boolean       * error
)
{
   Level_t level;

   level.ver = 0;
   level.rel = 0;
   level.mod = 0;
   level.fix = 0;

   #pragma covcc !instr
   if (    (parsers.token != A_NUMBER)
                      &&
        (parsers.token != A_DOTTED_NUMBER))
   {
       parsing_error (DOTTED_NUMBER_EXPECTED, error);
       return;
   }
   #pragma covcc instr

   sscanf (parsers.tokens_string_value, "%hd.%hd.%hd.%hd", & (level.ver),
           & (level.rel), & (level.mod), & (level.fix));

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

   create_relation (criteria, VERSION, operator, level.ver, NIL (char), error);
   RETURN_ON_ERROR;

   create_relation (criteria, RELEASE, operator, level.rel, NIL (char), error);
   RETURN_ON_ERROR;

   create_relation (criteria, MODIFICATION, operator, level.mod, NIL (char),
                    error);
   RETURN_ON_ERROR;

   create_relation (criteria, FIX, operator, level.fix, NIL (char), error);
   RETURN_ON_ERROR;

} /* end parse_dotted_number */

/*--------------------------------------------------------------------------*
**
** NAME: parse_relation_expression
**
** FUNCTION:  This routine is responsible for parsing relation_expressions
**
**
**         relation_expression : object relation
**                             | 'o' or_expression
**
**                      object : 'v'
**                             | 'r'
**                             | 'm'
**                             | 'f'
**                             | 'p'
**
**
**            This routine builds up a search criteria that will
**            be later be used by evaluate_relaton to link all of the
**            fixes that satisfy it as requisites to the parent_fix.
**
**            VRMFP_seen is used to detect multiple occurances of the same
**            object (v,r,m,f or p), which is an error.  It is implemented
**            using bit-fields.
**
**            VPD_field is used when an OR_expression is being processed.
**            It is used to detect the erroneous changing of objects during
**            an OR_expression (i.e. it is legal to have: V=3 o V=4
**                                   but illegal to have: V=4 o R=5)
**
**
** RETURNS:   TRUE, if another relation_expression exists, otherwise FALSE.
**
**
**--------------------------------------------------------------------------*/

static boolean parse_relation_expression
                                    (int                  * VRMFP_seen,
                                     token_type           * VPD_field,
                                     boolean                base_level_seen,
                                     criteria_type        * criteria,
                                     boolean                first_time_through,
                                     boolean              * error)
 {
          char                  failure_code;
          int                   numeric_value;
          boolean               OR_operator = FALSE;
          char                  PTF_value[MAX_LEX_STRING_LENGTH];
          token_type            relation;
          boolean               time_to_exit = FALSE;

   switch (parsers.token)
    {
      case OR           :
         parsers.token = get_token (error);
         RETURN_ON_ERROR;

         OR_operator = TRUE;
         break;

      case VERSION      :
         if (base_level_seen)
          {
            parsing_error (BASE_LEVEL_AND_VRMF_ILLEGAL, error);
            return (FALSE);
          }

         if (*VRMFP_seen & VERSION_SEEN)
          {
            parsing_error (VERSION_SEEN_AGAIN, error);
            return (FALSE);
          }
         *VRMFP_seen |= VERSION_SEEN;
         break;

      case RELEASE      :
         if (base_level_seen)
          {
            parsing_error (BASE_LEVEL_AND_VRMF_ILLEGAL, error);
            return (FALSE);
          }

         if (*VRMFP_seen & RELEASE_SEEN)
          {
            parsing_error (RELEASE_SEEN_AGAIN, error);
            return (FALSE);
          }
         *VRMFP_seen |= RELEASE_SEEN;
         break;

      case MODIFICATION :
         if (base_level_seen)
          {
            parsing_error (BASE_LEVEL_AND_VRMF_ILLEGAL, error);
            return (FALSE);
          }

         if (*VRMFP_seen & MODIFICATION_SEEN)
          {
            parsing_error (MODIFICATION_SEEN_AGAIN, error);
            return (FALSE);
          }
         *VRMFP_seen |= MODIFICATION_SEEN;
         break;

      case PTF          :
         if (*VRMFP_seen & PTF_SEEN)
          {
            parsing_error (PTF_SEEN_AGAIN, error);
            return (FALSE);
          }
         *VRMFP_seen |= PTF_SEEN;
         break;

      case FIX          :
         if (base_level_seen)
          {
            parsing_error (BASE_LEVEL_AND_VRMF_ILLEGAL, error);
            return (FALSE);
          }

         if (*VRMFP_seen & FIX_SEEN)
          {
            parsing_error (FIX_SEEN_AGAIN, error);
            return (FALSE);
          }
         *VRMFP_seen |= FIX_SEEN;
         break;

      default :
         time_to_exit = TRUE;

    } /* end switch (parsers.token) */

   if (time_to_exit)
      return (FALSE);  /* Ain't no more (legal) expressions here */

   if (OR_operator)
    {
      parse_or_expression (* VPD_field, criteria, error);
      RETURN_ON_ERROR;
    }
   else
    {
      * VPD_field = parsers.token;
      parsers.token = get_token (error);
      RETURN_ON_ERROR;

      parse_relation (* VPD_field, criteria, error);
      RETURN_ON_ERROR;

    }
   return (TRUE);  /* We haven't hit the end of the expressions yet */

 } /* end parse_relation_expression */

/*--------------------------------------------------------------------------*
**
** NAME: parse_or_expression
**
** FUNCTION:  This routine is responsible for parsing an OR_expression
**
**            or_expression : operator number_or_PTF
**
**            This routine simply parses an OR_expression and passes back
**            the number or PTF_value found.  The real work is handled by
**            the caller, parse_relation_expression.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_or_expression (token_type      VPD_field,
                                 criteria_type * criteria,
                                 boolean       * error)
 {
    short           int_value = -1;
    char          * PTF_value = NIL (char);
    token_type      operator;

    switch (parsers.token)
     {
       case VERSION :
       case RELEASE :
       case MODIFICATION :
       case FIX :
       case PTF :
          if (VPD_field != parsers.token)
           {
             parsing_error (VPD_FIELDS_MUST_MATCH, error);
             return;
           }
          parsers.token = get_token (error);
          RETURN_ON_ERROR;

          break;

       default :
          break;  /* Not an error */

     }  /* end switch */

    operator = parsers.token;
    switch (parsers.token)
     {
       case LESS_THAN    :
       case GREATER_THAN :
          if (VPD_field == PTF)
           {
             parsing_error (MUST_USE_EQUALS_ON_PTF, error);
             return;
           }
       case EQUALS       :
          parsers.token = get_token (error);
          RETURN_ON_ERROR;

          switch (parsers.token)
           {
             case A_NUMBER :
                int_value = parsers.tokens_number_value;
                if (VPD_field == PTF)
                 {
                    parsing_error (ILLEGAL_PTF, error);
                    return;
                 }
                break;

             case A_NAME :
                PTF_value = parsers.tokens_string_value;
                if (VPD_field != PTF)
                 {
                   parsing_error (NUMBER_EXPECTED, error);
                   return;
                 }
                break;

             default :
                parsing_error (NUMBER_OR_PTF_EXPECTED, error);
                return;
           }  /* end switch */
          break;

       default :
          parsing_error (EXPECTED_RELATION, error);
          return;
     } /* end switch */

   create_relation (criteria, VPD_field, operator, int_value,
                    PTF_value, error);
   RETURN_ON_ERROR;

   return;

 } /* end parse_or_expression */

/*--------------------------------------------------------------------------*
**
** NAME: parse_relation
**
** FUNCTION:  This routine is responsible for parsing a relation.
**
**            relation : operator number_or_PTF
**
**            operator : '<'
**                     | '='
**                     | '>'
**
**           number_or_PTR : A_NUMBER
**                         | A_STRING
**
**            This routine simply parses a relation.  It passes back the
**            operator and value (numeric_value or PTF_value) found.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_relation (token_type      VPD_field,
                            criteria_type * criteria,
                            boolean       * error)
 {
   short        numeric_value = -1;
   char       * PTF_value = NIL (char);
   token_type   operator;

    switch (parsers.token)
     {
       case LESS_THAN    :
       case GREATER_THAN :
          if (VPD_field == PTF)
           {
             parsing_error (MUST_USE_EQUALS_ON_PTF, error);
             return;
           }
       case EQUALS       :
          operator = parsers.token;
          parsers.token = get_token (error);
          RETURN_ON_ERROR;

          switch (parsers.token)
           {
             case A_NUMBER :
                numeric_value = parsers.tokens_number_value;
                if (VPD_field == PTF)
                 {
                    parsing_error (ILLEGAL_PTF, error);
                    return;
                  }
                break;

             case A_NAME :
                PTF_value = parsers.tokens_string_value;
                if (VPD_field != PTF)
                 {
                  parsing_error (NUMBER_EXPECTED, error);
                  return;
                 }
                break;

             default :
                parsing_error (NUMBER_OR_PTF_EXPECTED, error);
                return;
           }  /* end switch */
          break;

       default :
          parsing_error (EXPECTED_RELATION, error);
          return;
     } /* end switch */

   create_relation (criteria, VPD_field, operator, numeric_value,
                    PTF_value, error);
   RETURN_ON_ERROR;
 } /* end parse_relation */

/*--------------------------------------------------------------------------*
**
** NAME: parse_requisite_group
**
** FUNCTION:  This routine is responsible for parsing a requisite_group
**
**            requisite_group : A_NUMBER '{' requisite_items '}'
**
**
**            This routine has to evaluate a group of ifreqs, coreqs or
**            prereqs, and then choose the best set of fixes which will
**            allow at more than A_NUMBER of them to pass.
**
**            A DUMMY_GROUP is created to hang all of the elements of the
**            group off of as prereqs.  The fixes are ordered in decreasing
**            desirability on list dummy_group->prereqs.
**
**            The subgraph_state of the DUMMY_GROUP is the aggregate of the
**            first n best fixes in the group, where n is the minimum number
**            of fixes required for passing.
**
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

static void parse_requisite_group (fix_info_type   * group_parent, 
                                   fix_info_type  ** fix_chosen,
                                   boolean         * error)
 {
    requisite_list_type  * requisite;
    fix_info_type        * group_result;
    int                    minimum_number_of_passes;
    int                    number_of_items;

   if (parsers.token != A_NUMBER)
    {
      parsing_error (PASS_COUNT_REQUIRED, error);
      return;
    }
   else
      minimum_number_of_passes = parsers.tokens_number_value + 1;

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

   if (parsers.token != OPEN_BRACE)
    {
      parsing_error (OPEN_BRACE_EXPECTED, error);
      return;
    } /* end if */

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

   create_dummy_group (fix_chosen, error);
   RETURN_ON_ERROR;

   group_result = * fix_chosen;
   group_result -> passes_required = minimum_number_of_passes;
   number_of_items = 0;

   while (parsers.token != CLOSE_BRACE && parsers.token != END_OF_FILE)
    {
     parse_requisite_item (group_parent, group_result, error);
     RETURN_ON_ERROR;

     number_of_items++;
    } /* end while */

   if (parsers.token != CLOSE_BRACE)
    {
      parsing_error (CLOSE_BRACE_EXPECTED, error);
      return;
    }

   parsers.token = get_token (error);
   RETURN_ON_ERROR;

   if (number_of_items < group_result -> passes_required)
    {
      parsing_error (PASS_COUNT_TOO_SMALL, error);
      return;
    }

 }  /* end parse_requisite_group */

/*--------------------------------------------------------------------------*
**
** NAME: parsing_error
**
** FUNCTION:  This routine is called when a parsing error is detected.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void parsing_error (parser_error_type  parsing_error,
                           boolean           * error)

 {
   fix_info_type * ptr;
   char            fix_name[MAX_LPP_FULLNAME_LEN];

   check_prereq.number_of_errors++;
   ptr = parsers.prereq_node;

   get_fix_name_from_fix_info (ptr, fix_name);
   set_failure_code (SYNTAX_ERROR_FLAG, TRUE);

   switch (parsing_error)
    {

      case GROUP_OR_REQUISITE_TYPE:

         /*  A parsing error occured for %s
                on line %d, column %d.
                Instead of %s, one of the following was expected:
                    a '>' (to start a group) or
                    *ifreq, *prereq, or *coreq or
                    a product name.
                    Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_UNXTYP_E, CKP_PARS_UNXTYP_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);


         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case LPP_NAME_EXPECTED:

         /* A parsing error occured for %s
               on line %d, column %d.
               A product name was expected instead of %s.
               Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_EXPNAM_E, 
                  CKP_PARS_EXPNAM_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case OR_INVALID:

         /* A parsing error occured for %s
               on line %d, column %d.
               An OR expression, 'O', is invalid at this point.
               Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_INVOR_E, 
                  CKP_PARS_INVOR_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case VRMFP_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                A 'V', 'R', 'M', 'F', or 'P' was expected
                instead of %s.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_EXPVRM_E, 
                  CKP_PARS_EXPVRM_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case VERSION_SEEN_AGAIN:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The version field, 'V', has already been found.
                Use the OR expression, 'O', to compare multiple values
                of the version field.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_VRSRPT_E, 
                  CKP_PARS_VRSRPT_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case RELEASE_SEEN_AGAIN:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The release field, 'R', has already been found.
                Use the OR expression, 'O', to compare multiple values
                of the release field.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_RLSRPT_E, 
                  CKP_PARS_RLSRPT_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case MODIFICATION_SEEN_AGAIN:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The modification field, 'M', has already been found.
                Use the OR expression, 'O', to compare multiple values
                of the modification field.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_MODRPT_E, 
                  CKP_PARS_MODRPT_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case PTF_SEEN_AGAIN:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The PTF field, 'P', has already been found.
                Use the OR expression, 'O', to compare multiple values
                of the PTF field.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_PTFRPT_E, 
                  CKP_PARS_PTFRPT_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case FIX_SEEN_AGAIN:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The fix field, 'F', has already been found.
                Use the OR expression, 'O', to compare multiple values
                of the fix field.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_FIXRPT_E, 
                  CKP_PARS_FIXRPT_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case BASE_LEVEL_AND_VRMF_ILLEGAL:

         /*  A parsing error occured for %s
                on line %d, column %d.
                It is illegal to specify version, release, modification or fix
                fields when a base level is given in an *ifreq expression.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_VRMF_LEVEL_E,
                  CKP_PARS_VRMF_LEVEL_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case VPD_FIELDS_MUST_MATCH:

         /*  A parsing error occured for %s
                on line %d, column %d.
                All operands of an OR expression, 'O', must operate
                on the same field.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_OROPER_E, 
                  CKP_PARS_OROPER_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case MUST_USE_EQUALS_ON_PTF:

         /*  A parsing error occured for %s
                on line %d, column %d.
                PTF fields can only be compared for equality.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_OROPER_E, 
                  CKP_PARS_OROPER_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case EXPECTED_RELATION:

         /*  A parsing error occured for %s
                on line %d, column %d.
                A relation operator ('<' '=' '>') was expected
                instead of '%s'.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_OROPER_E, 
                  CKP_PARS_OROPER_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case ILLEGAL_PTF:

         /*  A parsing error occured for %s
                on line %d, column %d.
                An illegal PTF value, %s, was found.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_ILLPTF_E, 
                  CKP_PARS_ILLPTF_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case NUMBER_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                A version, release, fix, or modification number
                was expected where %s was found.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_NONUMB_E, 
                  CKP_PARS_NONUMB_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case DOTTED_NUMBER_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                A base level was expected following an open parenthesis '(' in
                an *ifreq expresson instead of %s.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_DOT_NUMB_E, 
                  CKP_PARS_DOT_NUMB_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case NUMBER_OR_PTF_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                A number or PTF ID was expected where %s was found.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_NOPTFN_E, 
                  CKP_PARS_NOPTFN_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case OPEN_BRACE_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                There is a missing open brace '{'.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_OPENB_E, 
                  CKP_PARS_OPENB_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case CLOSE_BRACE_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                There is a missing close brace '}'.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_CLOSEB_E, 
                  CKP_PARS_CLOSEB_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case CLOSE_PAREN_EXPECTED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                Missing close parenthesis ')' after base level in an *ifreq
                expression.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_CLOSEP_E, 
                  CKP_PARS_CLOSEP_D),
                 inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case PASS_COUNT_REQUIRED:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The number of required passing items for a group
                was expected instead of %s.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_PASSC_E, 
                  CKP_PARS_PASSC_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column,
                  parsers.tokens_string_value);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      case PASS_COUNT_TOO_SMALL:

         /*  A parsing error occured for %s
                on line %d, column %d.
                The group has fewer items than was specified for the number
                required to pass.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_PASSLESS_E, 
                  CKP_PARS_PASSLESS_D),
                  inu_cmdname, fix_name, parsers.line_number, parsers.column);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
         break;

      default :

         /*  A parsing error occured for %s
                on line %d, column %d.
                An internal error, %d, occured during parsing.
                Use local problem reporting procedures. */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_PARS_PASSLESS_E,
                  CKP_PARS_PASSLESS_D), inu_cmdname, fix_name,
                  parsers.line_number, parsers.column, parsing_error);
         * error = TRUE;
         check_prereq.function_return_code = PARSING_ERROR_RC;
    } /* end switch */
  
  if (check_prereq.called_from_command_line)
     print_output_file ();

  return;

 } /* end parsing_error */

/*--------------------------------------------------------------------------*
**
** NAME: get_implied_ifreq_base_level
**
** FUNCTION:  Determines the base level implied by the dotted number
**            requisite expression provided in the criteria structure. 
**            Returns the base level in the base_criteria structure.
**
** RETURNS:   Void function
**
**--------------------------------------------------------------------------*/

void 
get_implied_ifreq_base_level 
(
criteria_type * base_criteria,
criteria_type * specified_level,
boolean       * error
)
{
   Level_t    level;
   token_type operator;

   level.ver = specified_level->version->int_value;
   level.rel = specified_level->release->int_value;
   level.mod = specified_level->modification->int_value;
   level.fix = specified_level->fix->int_value;

   (* base_criteria) = EMPTY_CRITERIA; 

   operator = EQUALS;
   if (level.fix != 0)
   {
      level.fix = 0;
   }
   else
   {
      if (level.mod != 0)
         level.mod = 0;
      else
      {
         /*
          * Must mean we were given an expression of the form *ifreq A v.r.0.0.
          * Let's just use this as a check to make sure that if A is installed
          * or being installled, it must be at least at level v.r.0.0.
          */
         operator = GTRTHAN_OR_EQUALS;
         return;
      }
   } 
   
   create_relation (base_criteria, VERSION, operator, level.ver, 
                    NIL (char), error);
   RETURN_ON_ERROR;

   create_relation (base_criteria, RELEASE, operator, level.rel, 
                    NIL (char), error);
   RETURN_ON_ERROR;

   create_relation (base_criteria, MODIFICATION, operator, level.mod,
                    NIL (char), error);
   RETURN_ON_ERROR;

   create_relation (base_criteria, FIX, operator, level.fix, 
                    NIL (char), error);
   RETURN_ON_ERROR;

} /* get_implied_ifreq_base_level */

/*--------------------------------------------------------------------------*
**
** NAME: recover_from_parsing_error
**
** FUNCTION:  Checks if the error detected was a parsing error (rather than
**            something more serious).  Removes all requisite links from
**            the offending node and disables the node for the current
**            operation. 
**
**            The purpose of recovery (as opposed to erroring off) is to 
**            prevent an entire installation from failing when a "strange"
**            condition is detected in the vpd or on the media.  This permits
**            installp/ckprereq to be updated, for example, to recognize
**            a new requisite type in the same toc as the installp update.
**
** RETURNS:   Void function
**
**--------------------------------------------------------------------------*/

static void 
recover_from_parsing_error 
(
fix_info_type * error_node,
boolean         first_time,
boolean       * error
)
{

   requisite_list_type * requisite;
   requisite_list_type * del_req;
   
   /*
    * Return without recovering if we were called to do ckprereq command line
    * checking OR if we didn't detect a parsing, lexical, or semantic error
    * OR if we're doing a reject/deinstall op.  (In the latter case, we may
    * be left with graph fragments and we don't want these playing significant
    * roles when the graph is inverted.)
    */ 
   if ((check_prereq.mode == OP_REJECT)
                         ||
       (check_prereq.called_from_command_line) 
                         || 
       (check_prereq.function_return_code != PARSING_ERROR_RC  &&
        check_prereq.function_return_code != LEXICAL_ERROR_RC  &&
        check_prereq.function_return_code != SEMANTIC_ERROR_RC))

      return;

   /*
    * Break all requisite links that may have been created.  Recurse if
    * a group node was created.
    */
   requisite = error_node->requisites;
   while (requisite != NIL (requisite_list_type))
   {
      if (requisite->fix_ptr->origin == DUMMY_GROUP)
         recover_from_parsing_error (requisite->fix_ptr, FALSE, error);

      del_req = requisite;
      requisite = requisite->next_requisite;
      del_req->fix_ptr = NIL (fix_info_type);
      del_req->next_requisite = NIL (requisite_list_type); 
      free_requisite (del_req); 
                  
   } /* while */
   error_node->requisites = NIL (requisite_list_type);

   /*
    * Disable the node for the current operation.
    */
   error_node->apply_state  = CANNOT_APPLY;
   error_node->commit_state = CANNOT_COMMIT;
   error_node->reject_state = CANNOT_REJECT;

   if (first_time)
   {
     /*
      * Top-most call (in case we recursed for groups).  Reset the error flags.
      */
     *error = FALSE;
     check_prereq.function_return_code = 0;
   }

} /* recover_from_parsing_error */
