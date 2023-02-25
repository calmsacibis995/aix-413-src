static char sccsid[] = "@(#)65  1.27.1.37  src/bos/usr/sbin/install/ckprereq/inu_check_prereq.c, cmdinstl, bos41J, 9517A_all 4/20/95 19:14:46";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            calculate_return_code
 *            initialize_inu_ckreq
 *            inu_ckreq  
 *            handle_error
 *            subgraph_return_code 
 *            uninitialize_inu_ckreq
 *
 * ORIGINS: 27
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

#define DEFINE_EXTERNALS
#include <check_prereq.h>

#include <instl_options.h>

static void initialize_inu_ckreq (
Option_t * SOP_Ptr,
Option_t * Fail_SOP_Ptr,
TOC_t    * TOC_Ptr,
boolean    verbose,
boolean    Auto_Include,
boolean    installp_update,
short      parts_to_operate_on,
boolean    called_from_cmd_line,
boolean  * error);

static void uninitialize_inu_ckreq (
Option_t * new_option_list,
boolean  * error);

static int calculate_return_code (
boolean * error);

static int subgraph_return_code (
fix_info_type * fix);

static int handle_error (
char * string);

static void disable_non_oem_updates ();

static long oem_with_same_name_in_subgraph (
char          * name,
fix_info_type * fix,
char          * oem_system_name
);

#define oem(fix)     (((fix)->TOC_Ptr->lang[0] == 'o') && \
	               (!strncmp((fix)->TOC_Ptr->lang,"oem_",4)))

/*--------------------------------------------------------------------------*
**
** NAME: inu_ckreq
**
** FUNCTION: inu_ckreq may be invoked from two callers:
**           * From ckprereq, which is a command level interface.  This command
**             allows for a systems administrator to easily determine if all
**             prereqs have been satisfied for an lpp.
**
**           * From installp, which gives a list of products and/or fixes
**             that are to be applied, commited, rejected or de-installed.  
**             check_prereq's job is to determine if the given list is valid.
**
**             For an apply operation, if there are any prerequisites that
**             are missing, inu_ckreq () will attempt to add them to the
**             install list.  After finishing the list, they are ordered so
**             that they may be applied without violating prereq rules and
**             reducing tape movement and disk swaps.
**
**             For a commit opereration, inu_ckreq () checks to see if any
**             prereqs of the given list are not committed.  Any uncommitted
**             fixes/products are added to the list.
**
**             For a reject operation, a "dependents" test is done for each item
**             on the list.  If any other uncommitted dependents need to be
**             rejected as a result of the requested packages being rejected, 
**             they are added to the list also.
**
**             For a de-install operation, dependents are also required to be
**             removed but unlike reject, committed products may be removed.
**             De-install only applies to base level products.
** 
**
** RETURNS:  For apply: an ordered list of products/fixes that can be
**                      applied.  Also, a return value indicating if there were
**                      any serious problems.
**
**           For commit: an ordered list of products/fixes that can be
**                       committed.  In addition, a return value indicating if
**                       there were any serious problems.
**
**           For reject: an ordered list of products/fixes that can be
**                       rejected.  In addition, a return value indicating if
**                       there were any serious problems.
**
**           For de-install: an ordered list of products that can be
**                       rejected.  In addition, a return value indicating if
**                       there were any serious problems.
**                 
**--------------------------------------------------------------------------*/

int 
inu_ckreq 
(
Option_t * SOP_Ptr,
Option_t * Fail_SOP_Ptr,
TOC_t    * TOC_Ptr,
boolean    verbose,
boolean    Auto_Include,
boolean    installp_update,
short      parts_to_operate_on,
boolean    called_from_cmd_line
)
{
   boolean    ckp_non_req_failures;        /* need to report non-req failures
                                              detected by ckprereq? */
   boolean  * error;                       /* keeps track of fatal errors */
   boolean    error_value = FALSE;         /* base value for above */
   boolean    finished;                    /* helps us terminate early */
   boolean    fail_sop_warnings;           /* detects warnings on fail sop. */
   boolean    fix_info_warnings;           /* detects warnings in fix_info
                                              table. */
   boolean    instp_non_req_failures;      /* need to report non-req failures
                                              detected by installp? */
   boolean    non_req_auto_sups_failure;   /* detects failure of auto-supersede
                                              selected update. */
   boolean    refine_failsop_errors = FALSE;
                                           /* flags generic errors on failsop */
   boolean    failures_detected;           /* need to report fails. or warns? */
   boolean    req_auto_sups_failure;       /* detects requisite failure of 
                                              auto-supersedes update. */
   boolean    requisite_failures;          /* need to report req. failures?*/
   boolean    resolved_one = FALSE;        /* found & resolved missing updates*/
   boolean    sop_was_empty_on_entry = FALSE;      
                                           /* nothing on SOP when called. */
   boolean    unresolved_updates_on_failsop = FALSE;  
                                           /* flags missing updts on failsop */
   Option_t * new_option_list;             /* sop after its re-built */

   FILE     * oem_fp = NULL;		   /* File pointer to OEM indicator file. */

   error = & error_value;

   /*------------------------------------------------------------------------
    * STRATEGY: 
    *
    * The goal is to check that all requisites of pkgs contained on the SOP
    * list passed in are met.  If -g is switched on, "pull in" unmet 
    * requisites (or installed dependents for reject/deinstall ops).  Report
    * any failures detected and successes.  inu_ckreq () is called by the 
    * ckprereq command and by installp.  When called by the ckprereq command,
    * inu_ckreq fakes an apply operation.  Deviations in behavior can be
    * seen where the flag "check_prereq.called_from_command_line" is used.
    * 
    * This model of requisite checking and resolution consists of multiple 
    * discrete steps.  While often appearing costly and repetative, each
    * step serves a purpose and helps keep the complexity of the code down.
    *
    * The key steps involved in this model (ommitting many periphery tasks)
    * are as follows:
    * 
    * -- Construct a linked list data structure representing information from 
    *    our domain of discourse (pkgs requested by the user, pkgs already 
    *    installed, pkgs on the installation media if applicable). 
    *
    * -- build supersedes relationships between nodes in the data structure,
    *    from information contained in certain fields within nodes in the list. 
    * 
    * -- evaluate the supersedes relationships, making up-front decisions 
    *    concerning what should be applied in a supersedes chain (supersedes
    *    chain is list of nodes in the repository linked together by 
    *    supersedes links representing what a given node supersedes and what
    *    a given node is superseded by).  Links between nodes in a supersedes 
    *    chain are converted to prereq links for non-apply operations.
    *  
    * -- Construct the requisite graph.  Parse the requisite information 
    *    contained in each node in the repository of structures.  Link members
    *    of the repository list to each other to indicate the requisite 
    *    relationships.  Resolve unmet requisites with superseding pkgs when 
    *    possible.  (Invert the requisite graph for reject and deinstall ops 
    *    so that a prereq pkg of a node becomes the dependee of that node.)
    * 
    * -- Evaluate the requisite graph to establish what can be applied/
    *    committed/rejected/deinstalled:
    *    A "node tagging" scheme is employed, whereby any node that can satisfy 
    *    a requisite is marked as a CANDIDATE_NODE prior to a given traversal 
    *    of the graph.  The graph is traversed in subgraphs, where each pkg 
    *    requested at the installp cmd line is the root/head of a subgraph.  
    *    If "success" is returned from traversal of a subgraph, all 
    *    nodes in that subgraph are marked as SUCCESSFUL_NODEs and will be 
    *    applied/committed/rejected (if not already in the desired state).  A 
    *    FAILURE_NODE tag is used as an intermediate tag for a node which fails 
    *    during a given traversal of the graph.  Untagged nodes and 
    *    FAILURE_NODEs cause requisite/dependent failures for a given traversal
    *    of the graph.  Multiple passes of the graph can occur.  Prior to each 
    *    pass, a pre-scan of the graph is typically performed to mark a new set
    *    of CANDIDATE_NODEs for the pending evaluation.
    *    pass 1: see what, if anything, from the list of pkgs requested by the
    *            user at the installp cmd line, can be "done" (applied/commitd/
    *            rejected/deinstalled) without pulling in umet requisites
    *            (dependents for deinstall). 
    *    pass 2: if -g has been specified and something failed a requisite 
    *            check in pass 1, see what else can be "done" by pulling in 
    *            unmet requisites.   (This step is separate mainly
    *            for optimization -- no need for 2nd pass if 1st pass went ok.)
    *    pass 3: consider ifreqs.  Since we may have decided to install a base
    *            level package in either of the above two passes, traverse the
    *            graph a 3rd time to process ifreqs.  This is a separate pass
    *            to eliminate order of traversal as a factor when considering
    *            ifreqs in the subgraph.  ie.  ALL base levels being installed
    *            are now known about.
    *    pass 4: for apply ops only when -g is specified:  consider unresolved 
    *            ifreqs.  One more pass of the graph is made to see if any 
    *            packages which are already installed now need to have updates
    *            on the media installed to satisfy missing ifreqs. 
    *           
    * -- Report failures.  Traverse the graph again to determine if failures
    *    occurred or if any warnings need to be reported.  Report them.
    *
    * -- Re-build the sop according to prereq/dependent order.
    *
    * -- Report successful packages on the SOP.
    * 
    * DE-INSTALL NOTE:  De-install requisite checking is achieved using much
    *                   of reject's code.  Subtle differences are flagged 
    *                   througout the code with the 
    *                   check_prereq.deinstall_submode flag.
    *------------------------------------------------------------------------*/ 

   initialize_inu_ckreq (SOP_Ptr, Fail_SOP_Ptr, TOC_Ptr, verbose, Auto_Include, 
                         installp_update, parts_to_operate_on, 
                         called_from_cmd_line, error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after initializing"));

   /*
    * inu_ckreq prints all of installp's up-front failures.  If the SOP is
    * empty, then that's all we were called to do.  There are two cases
    * when we may want to call load_fix_info_table () even though we were
    * only called to print installp's failures:
    * 1. When the failsop contains an update which is not on the media.  We 
    *    may be able to install that update via a superseding fix which is
    *    on the media.  We want to try to find those superseding fixes.
    * 2. When the failsop contains entries with failure codes which will not
    *    yield specific enough error messages.  We will refine those 
    *    error codes here.
    * We want to know about both conditions regardless of whether or not we
    * were called with an empty SOP so let's first do a scan of the failsop.  
    */
   check_failsop_for_special_cases (Fail_SOP_Ptr,
                                    & unresolved_updates_on_failsop,
                                    & refine_failsop_errors, 
                                    parts_to_operate_on);

   /*
    * Print installp's errors now if the sop is empty and there is no further
    * processing to do for our "special cases."
    */
   if ( SOP_Ptr->next == NIL (Option_t)) {
      sop_was_empty_on_entry = TRUE;
      if (! (unresolved_updates_on_failsop || refine_failsop_errors)) {
         print_instp_failures_and_warnings (error);
         RETURN_VALUE_ON_ERROR (handle_error 
                             ("error after print_instp_failures_and_warnings"));
         return (SUCCESS);
      }
   }

   /*
    * Build our repository of nodes that we'll use to decide what can or
    * can't apply/commit/reject/deinstall.  After the call to 
    * load_fix_info_table, check_prereq.fix_info_anchor will point to a linear 
    * linked-list containing information about packages in the VPD, SOP and 
    * TOC as appropriate.  Supersedes relationships between those structures 
    * will have also been established.
    */
   load_fix_info_table ((SOP_Ptr->next == NIL (Option_t)), error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after load_fix_info"));
   DEBUG_DUMP ("load_fix_info_table");

   /*
    * For apply operations, make up-front decisions about which base level 
    * options will/won't be applied when there are multiple versions of the 
    * same option to consider (installed and/or on installation media).
    */
   if (check_prereq.mode == OP_APPLY)
   {
      evaluate_base_levels (error);
      RETURN_VALUE_ON_ERROR (handle_error ("error after evaluate_base_levels"));
      DEBUG_DUMP ("evaluate_base_levels");
   }

   /*
    * Create explicit and implicit prereq links for 4.1 updates to their
    * base levels.
    */
   build_base_prereqs (error);
   RETURN_ON_ERROR;

   /*
    * Make up-front decisions about which superseding fixes to install when
    * there are multiple superseding fixes to be installed.  The goal is to
    * eliminate supersedes relationships from the graph when the time comes to
    * examine requisites.  For non-apply ops, supersedes relationships imply
    * prereq from newer to older pkg.  These links will be created by this
    * call. 
    */
   evaluate_superseding_updates (error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after evaluate_supersedes"));
   DEBUG_DUMP ("evaluate_supersedes");

   /*
    * Make any generic failsop errors more specific now that we have
    * our fix_info table to work with.
    */
   if (refine_failsop_errors)
       refine_failsop_errors_for_reporting (Fail_SOP_Ptr);
   /*
    * Try to resolve "missing" updates on the failsop which may have superseding
    * updates on the media or already installed.  (See the note above where
    * we check for these unresolved updates.)
    */
   if (unresolved_updates_on_failsop) 
   {
      process_unresolved_updates_on_failsop (Fail_SOP_Ptr, &resolved_one, 
                                             error);
      RETURN_VALUE_ON_ERROR (handle_error 
                              ("error after process_unresolved...."));
   }

   /*
    * If we didn't resolve any missing updates and our SOP is empty (and was
    * empty when we entered inu_ckreq -- since it may be emptied during
    * evaluate_supersedes while there is still real processing to do ie. we
    * don't add auto superseding entries back to the sop and we remove the
    * superseded entries from the sop), print failure messages and return.
    */
   if ((! resolved_one) && (SOP_Ptr->next == NIL (Option_t)) && 
       sop_was_empty_on_entry) {
      print_instp_failures_and_warnings (error);
      RETURN_VALUE_ON_ERROR (handle_error 
                             ("error after print_instp_failures_and_warnings"));
      return (SUCCESS);
   }

   /*
    * Build the requisite relationships between nodes in the fix_info table.
    * Use the supersedes chains to resolve requisites on superseded nodes
    * (as established by evaluate_supersedes ()).  Supersedes nodes and chains
    * will be gone upon exit.
    */
   build_graph (error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after build_graph"));
   DEBUG_DUMP ("build_graph");

   /*
    * If this is an OEM-specific system, and if this is a regular apply operation,
    * then check the graph to make sure that dependents do not wipe out the
    * OEM modifications.
    */
   if (check_prereq.mode == OP_APPLY &&
       check_prereq.parts_to_operate_on != LPP_ROOT &&
       ((oem_fp = fopen("/usr/lib/objrepos/oem","r")) != NIL (FILE)))
   {
        disable_non_oem_updates ();
   }

   /*  
    * Mark nodes in the fix_info table that are already applied/committed/
    * "not installed" as candidates for the apply/commit/reject operation.
    */
   mark_initial_CANDIDATE_NODEs ();
   DEBUG_DUMP ("mark_initial_CANDIDATE_NODEs");

   /*
    * Link all packages that were requested for apply/commit/reject as 
    * requisites of the check_prereq anchor node.  (Mark them as candidates 
    * in the process. NOTE:  For a de-install operation, all updates to a 
    * product being de-installed will also be marked.)
    */
   put_command_line_args_in_graph (error);
   RETURN_VALUE_ON_ERROR (handle_error
                               ("error after put_command_line_args_in_graph"));
   DEBUG_DUMP ("put_command_line_args_in_graph");

   /* 
    * Make a first pass of the graph to see if we may apply/commit/reject
    * the requested packages without pulling in requisites (dependents for
    * reject).
    */
   finished = evaluate_graph (error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after evaluate graph"));
   DEBUG_DUMP ("evaluate_graph, first call");

   /*
    * If we didn't satisfy all of our command line nodes in the first call,
    * tag nodes (as candidates) that might be pulled in with -g and 
    * re-evaluate the graph.
    */
   if ((! finished) &&
       (check_prereq.Auto_Include))
   {
      add_auto_include_nodes_to_graph (error);
      RETURN_VALUE_ON_ERROR (handle_error ("Error after add_auto_include"));
      DEBUG_DUMP ("add_auto_include_nodes_to_graph");

      (void) evaluate_graph (error);
      RETURN_VALUE_ON_ERROR (handle_error ("Error after evaluate_graph"));
      DEBUG_DUMP ("evaluate_graph, -g call");
   }

   if (! check_prereq.called_from_command_line) 
   {
      /* 
       * Now that we have made decisions about all explicitly and implicitly
       * requested (via prereqs, coreqs and -g) nodes, re-traverse the graph
       * attempting to pull in any ifreqs that might be required for the 
       * apply/commit/reject operation.  (NOTE: This is done as a subsequent
       * explicit step to the above processing so that ALL base levels that
       * may be installed explicitly OR IMPLICITLY are known about before
       * doing ifreq tests -- this eliminates order of evaluation as a factor.)
       */
      consider_ifreqs (error);
      RETURN_VALUE_ON_ERROR (handle_error ("Error after consider_ifreqs."));
      DEBUG_DUMP ("consider_ifreqs");

      if (check_prereq.mode == OP_APPLY)
      {
         /*
          * A final pass of the graph is required for apply operations.  We'll
          * try to install any ifreqs of previously installed packages.  These
          * may be required if we decided to apply a base level in the above
          * steps.  The unresolved ifreqs will be added to the requisite 
          * chain of the anchor node as though they were requested from the
          * command line.  Warnings will be issued if they can't be installed 
          * and they will only be installed if -g was specified.  Failures
          * here should never affect the results of previous decisions made.
          */
         consider_unresolved_ifreqs (error);
         RETURN_VALUE_ON_ERROR (handle_error 
                                  ("Error after consider_unresolved_ifreqs."));
         DEBUG_DUMP ("consider_unresolved_ifreqs");
      }
   }

   /*
    * Print results.  (Only do so when called from command line if -v given).
    */
   if ((! check_prereq.called_from_command_line)
                     ||
         check_prereq.verbose)
   {
      /*
       * Change states of packages being pulled in with -g to help with 
       * error reporting.
       */
      set_implicit_states ();

      /*
       * Evaluate the classes of failures and warnings (if there are any).  
       * We'll simply mark the fix_info_table entries with a bit which will 
       * tell us why a node failed.  We'll use the bits later when we print 
       * results.  We'll also set flags which will be used to make decisions
       * here and in the called report routines.  If we detect requisite 
       * failures, we'll link the requisite back to its "command line ancestor"
       * (the fix that was requested by the user which is being "blocked" by 
       * the requisite).
       */
      failures_detected = evaluate_failures_and_warnings (
                               &ckp_non_req_failures, 
                               &instp_non_req_failures,
                               &requisite_failures,
                               &non_req_auto_sups_failure,
                               &req_auto_sups_failure, 
                               &fix_info_warnings, 
                               &fail_sop_warnings,
                               error);
      RETURN_VALUE_ON_ERROR (handle_error 
                              ("Error after evaluate_failures...."));

      if (failures_detected)
      {
         /*
          * We delayed printing the end of requisite checking msg until now.
          * Do it now and set a flag so we don't do it again in the success
          * routines.
          */
         if (! check_prereq.called_from_command_line)
         {
            inu_msg (ckp_prog, MSGSTR (MS_INUCOMMON, CMN_DONE_I, CMN_DONE_D));
            inu_msg (ckp_prog, MSGSTR (MS_CKPREREQ, CKP_RESULTS_I, 
                                                    CKP_RESULTS_D));
         }
         check_prereq.printed_finished_msg = TRUE;

         if ((ckp_non_req_failures)   || 
             (instp_non_req_failures) ||
             (requisite_failures))
         {
            report_failures (instp_non_req_failures, ckp_non_req_failures,
                             requisite_failures, non_req_auto_sups_failure,
                             req_auto_sups_failure, error);
            RETURN_VALUE_ON_ERROR (handle_error 
                                   ("Error after report_failures."));
            DEBUG_DUMP ("report_failures");
         }
 
         if (fix_info_warnings || fail_sop_warnings)
         {
            report_warnings (fix_info_warnings, fail_sop_warnings, error);
            RETURN_VALUE_ON_ERROR (handle_error 
                                   ("Error after report_warnings."));
            DEBUG_DUMP ("report_warnings");
         }

      } /* if (report_needed... */
   } /* if (! (check_prereq.... */

   /*
    * No need to continue if just called to check requisites. (from cmd line)
    */
   if (check_prereq.called_from_command_line)
   {
      print_output_file ();
      return (calculate_return_code (error));
   }

   /*
    * Eliminate, from the fix_info table, all nodes that are not referenced
    * by any other.  (Including nodes that are requisites of the anchor).
    * This makes life somewhat easier when ordering the packages for the 
    * apply/commit/reject operation.
    */
   delete_unreferenced_nodes (error);
   RETURN_VALUE_ON_ERROR (handle_error ("Delete_unreferenced_nodes."));
   DEBUG_DUMP ("delete_unreferenced_nodes");

   /* 
    * determine_order_of_fixes will create a new option list.  We
    * have to save the old one until determine_order_of_fixes is
    * finished since some data is shard between the two structures.
    */
   new_option_list = copy_option (check_prereq.SOP);
   new_option_list -> next = NIL (Option_t);
   determine_order_of_fixes (new_option_list, error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after determine_order"));
   DEBUG_DUMP ("determine_order");

   /*
    * Print successful pkgs (if there are any).
    */
   report_successes (new_option_list, error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after report_successes"));
   DEBUG_DUMP ("report_successes");

   /*
    * Clean up loose ends (free old sop, free graph if necessary, etc.)
    */
   uninitialize_inu_ckreq (new_option_list, error);
   RETURN_VALUE_ON_ERROR (handle_error ("error after uninit_inu_ckreq"));
   DEBUG_DUMP ("uninite_inu_ckreq");

   return (check_prereq.function_return_code);

} /* end inu_ckreq */

/*--------------------------------------------------------------------------*
**
** NAME: initialize_inu_ckreq
**
** FUNCTION: Initializes all of the static data for check_prereq so that
**           it may be invoked more than once (i.e. installp APPLY/COMMIT).
**
** RETURNS:  This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
initialize_inu_ckreq 
(
Option_t * SOP_Ptr,
Option_t * Fail_SOP_Ptr,
TOC_t    * TOC_Ptr,
boolean    verbose,
boolean    Auto_Include,
boolean    installp_update,
short      parts_to_operate_on,
boolean    called_from_cmd_line,
boolean  * error
)
{
   /*
    * Initialize the destinations for the various types of messages.  
    * This varies according to the value of instl_opt.V_flag and 
    * instl_opt.e_flag.  The comments along side each initialization
    * pertains to destination when logging (when not logging, errors
    * go to stderr, everything else goes to stdout).
    */
   ckp_errs = SCREEN_LOG;      /* errors go to screen and log */
   ckp_prog = SCREEN_MSG;      /* progress goes to screen */
   ckp_succ = LOG_MSG;         /* successes go to log */
   ckp_warn = LOG_MSG;         /* warnings go to log */

   /*
    * Clear the global check_prereq structure if we were not called from
    * the ckprereq command.  This will prevent confusion when inu_ckreq is
    * called multiple times in the same installp session (ie. for 
    * apply-commit and for failed calls to process SF ptfs, followed by 
    * calls to process "other" requested pkgs.
    */
			      
   if (called_from_cmd_line)
   {
      check_prereq.called_from_command_line = TRUE;
   }
   else
   {
      memset (&check_prereq, NULL, sizeof (check_prereq));
      /* 
       * Finalize where output is going based on e_flag and V_flag.
       */
      if (instl_opt.e_flag) {
         if (instl_opt.V_flag == V0) {
            ckp_prog = DEV_NULL;   /* progress will not be printed or logged. */
         } else { /* > V0 */
            ckp_succ = SCREEN_LOG;  /* successes will go to screen and log */
            ckp_warn = SCREEN_LOG;  /* warnings will go to screen and log */
         }
      } else {
         if (instl_opt.V_flag == V0) {
            ckp_prog = DEV_NULL;  /* Progress, successes, warnings will */
            ckp_succ = DEV_NULL;  /* not be printed.                    */
            ckp_warn = DEV_NULL;  
         }
      }
   }

   if (! check_prereq.called_from_command_line)
      /* Print "progress/status" message. */
      inu_msg (ckp_prog, MSGSTR (MS_CKPREREQ, CKP_VERIFYING_REQ_I, 
                                              CKP_VERIFYING_REQ_D));

   check_prereq.number_of_errors = 0;
   check_prereq.function_return_code = 0;
   check_prereq.dump_on_error = TRUE;

   /*  What were we called to do? */
   if (SOP_Ptr && SOP_Ptr->next)
      check_prereq.mode = SOP_Ptr->next->operation;
   else
      check_prereq.mode = Fail_SOP_Ptr->next->operation;

   if (check_prereq.mode == OP_DEINSTALL)
   {
       /*
        * Deinstall processing is almost identical to reject.  
        */
       check_prereq.mode = OP_REJECT;
       check_prereq.deinstall_submode = TRUE;
   }
   else
   {
       check_prereq.deinstall_submode = FALSE;
   }

   if ((char *) getenv ("INSTALL_DEBUG") != NIL (char))
   {
      check_prereq.debug_mode = TRUE;
   }
   else
   {
      check_prereq.debug_mode = FALSE;
   }

   if (! (check_prereq.debug_mode && check_prereq.called_from_command_line))
   {
      remove ("/tmp/ckprereq.graph.out");
   }

   if (check_prereq.mode == OP_APPLYCOMMIT)
      check_prereq.mode = OP_APPLY;

   /*
    * Set a flag that will help eliminate useless information (AVAILABLE
    * records) in our fix_info table.  Available records are not needed
    * for reject operations.  Normally, they are not needed for deinstall
    * ops but there's a rare case (when a usr-root ptf updates a usr base
    * level) when available records are useful.  Regardless, they are
    * very limited in number on a 4.1 system so whether or not they are
    * in the fix_info table is not that big of an issue.:
    */
   if (check_prereq.mode == OP_REJECT && !check_prereq.deinstall_submode)
      check_prereq.ignore_AVAILABLE_entries = TRUE;
   else
      check_prereq.ignore_AVAILABLE_entries = FALSE;

   check_prereq.parts_to_operate_on = parts_to_operate_on;
   check_prereq.SOP = SOP_Ptr;
   check_prereq.Fail_SOP = Fail_SOP_Ptr;
   if (TOC_Ptr == NIL (TOC_t))
   {
      check_prereq.First_Option = NIL (Option_t);
   }
   else
   {
      check_prereq.media_type = TOC_Ptr -> type;
      check_prereq.First_Option = TOC_Ptr -> options;
   }  /* end if */

   check_prereq.Auto_Include = Auto_Include;
   /*
    * Note: Currently, the "auto supersedes" feature is switched on by
    * the same flag as Auto_Include.  We'll keep a separate flag internal
    * to ckprereq in case this ever changes to use a separate flag in installp.
    */
   check_prereq.Auto_Supersedes            = Auto_Include;
   check_prereq.verbose                    = verbose;
   check_prereq.installp_update            = installp_update;
   check_prereq.commit                     = instl_opt.c_flag;
   check_prereq.no_save                    = instl_opt.N_flag;
   check_prereq.Force_Install              = instl_opt.F_flag;
   check_prereq.instp_verify               = instl_opt.v_flag;
   check_prereq.instp_preview              = instl_opt.p_flag;
   check_prereq.suppress_instreq_failures  = instl_opt.Q_flag;
   if (instl_opt.V_flag == V0)
   {
      /*
       * Internal to inu_ckreq, there are only 4 levels of verbosity which
       * control the type of output: V1-V4.  V0 is used above to 
       * determine the destination of various messages that will be printed. 
       */ 
      check_prereq.instp_verbosity            = V1;
      check_prereq.instp_V0_verb              = TRUE; /* remember that V0
						         was specified. */
   }
   else
   {
      check_prereq.instp_V0_verb              = FALSE;
      check_prereq.instp_verbosity            = instl_opt.V_flag;
   }
   check_prereq.printed_finished_msg       = FALSE;
   check_prereq.successful_auto_supersedes = FALSE;
   check_prereq.keep_description_info      = TRUE;
   check_prereq.keep_apar_info             = FALSE;

   if (! instl_opt.I_flag)
      check_prereq.Updates_Only_Install = TRUE;
   else
      check_prereq.Updates_Only_Install = FALSE;

   if (check_prereq.debug_mode)
   {
      dump_sop ("inu_ckreq: given args");
   }

   /*
    * Initialize a list used for reporting miscellaneous warnings.
    */
   init_fix_info_index (&report_anchor.misc_warnings_hdr,
                        &report_anchor.misc_warnings_tail,
                        error);

} /* end initialize_check_prereq */

/*--------------------------------------------------------------------------*
**
** NAME: calculate_return_code
**
** FUNCTION: This function calculates the return code when called by the
**           the command line interface (ckprereq command).  It is NOT called
**           when called by the installp command.
**
** RETURNS:  0, if all of the prereqs are applied, otherwise, it returns
**           a count of the number of failures.
**
**--------------------------------------------------------------------------*/

static int 
calculate_return_code 
(
boolean * error
)
{
   requisite_list_type * requisite;
   int                   return_code;

   return_code = 0;

   if (check_prereq.fix_info_anchor -> requisites == NIL (requisite_list_type))
    {
       check_prereq.function_return_code = INTERNAL_ERROR_RC;
       * error = TRUE;
       return (FATAL_ERROR_RC);
    }

   for (requisite = check_prereq.fix_info_anchor -> requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
    {
      return_code += subgraph_return_code (requisite -> fix_ptr);
    }

    if (check_prereq.number_of_errors != 0)
       return_code = FATAL_ERROR_RC;

    return (return_code);

} /* end calculate_return_code */

/*--------------------------------------------------------------------------*
**
** NAME: subgraph_return_code
**
** FUNCTION: This function calculates the return code when called by the
**           the command line interface (ckprereq command) for the given
**           subgraph.
**
** RETURNS:  0, if all of the prereqs are applied, otherwise, it returns
**           a count of the number of failures.
**
**--------------------------------------------------------------------------*/

static int 
subgraph_return_code 
(
fix_info_type * fix
)
{
   boolean               evaluate_fix;
   requisite_list_type * requisite;
   int                   return_code = 0;

   for (requisite = fix -> requisites;
        requisite != NIL (requisite_list_type);
        requisite = requisite -> next_requisite)
    {
       switch (requisite -> type)
        {
          case A_PREREQ :
          case AN_INSTREQ:
            evaluate_fix = TRUE;
            break;

          case A_COREQ  :
            if (check_prereq.called_from_command_line)
               evaluate_fix = FALSE;
             else
               evaluate_fix = TRUE;
            break;

          case AN_IFREQ  :
          case AN_IFFREQ :
            if (check_prereq.consider_ifreqs)
             {
               if (ck_base_lev_list (requisite, CBLL_SUCC, FALSE))
                  evaluate_fix = TRUE;
                else
                  evaluate_fix = FALSE;
             }
            else
             {
               evaluate_fix = FALSE;
             }
            break;
        } /* end switch */
      if (              evaluate_fix
                            &&
          ! (requisite -> fix_ptr -> flags & SUCCESSFUL_NODE))
       {
         return_code++;
       }
     } /* end for */

   return (return_code);

} /* end subgraph_return_code */

/*--------------------------------------------------------------------------*
**
** NAME: handle_error
**
** FUNCTION: This function dumps error information and determines the return
**           code.  Only called on error.
**
** RETURNS:  The appropriate return code for inu_ckreq.
**
**--------------------------------------------------------------------------*/

static int 
handle_error 
(
char * string
)
{
   if (check_prereq.dump_on_error)
    {
      graph_dump (string);
      dump_sop (string);
    }
   if (check_prereq.function_return_code == 0)
      check_prereq.function_return_code = UNKNOWN_ERROR_RC;

   return (check_prereq.function_return_code);

} /* end handle_error */

/*--------------------------------------------------------------------------*
**
** NAME: uninitialize_inu_ckreq
**
** FUNCTION: Cleans up a few lose ends before exitting inu_ckreq.
**
** RETURNS:  void function
**
**--------------------------------------------------------------------------*/

static void 
uninitialize_inu_ckreq 
(
Option_t * new_option_list,
boolean  * error
)
{
   /* Get rid of our old option list, it is no longer needed. */

   free_options (check_prereq.SOP);  /* Free all but header. */
   check_prereq.SOP -> next = new_option_list -> next;
   my_free (new_option_list);


   /* 
    * We free our graph if we're not doing an apply (since installp will
    * use the graph to perform "quick" in-progress requisite checking) OR
    * if we're doing an apply and there is nothing on the sop to 
    * process. 
    */

   if (check_prereq.mode != OP_APPLY || 
       check_prereq.SOP->next == NIL (Option_t))
   {
      free_graph ();
   }
   else
   {
      /*
       * There may still be group nodes in the requisite graph.
       * They serve no further useful purpose and accounting for them
       * in subsequent "in-progress" requisite checking in installp 
       * would unnecessarily complicate matters there.  Remove them from
       * the graph here and attach each member of the requisite group to
       * the parent of the group node.  (This will limit the "variable
       * number of passes" that may satisfy group requisites but that is a
       * rarely used feature -- ie.  if it says >0 and 4 requisites are
       * being installed, if any one fails during installation, the whole 
       * group will fail).  We don't need to do this for non-apply ops since
       * in-progress requisite checking does not occur.
       */
      remove_groups_from_graph (check_prereq.fix_info_anchor, 
                                check_prereq.fix_info_anchor,
                                error);
      RETURN_ON_ERROR;
      unmark_graph (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED);

      /*
       * The state of "medialess fixes" (ie. root part apply pkgs and 
       * MC pkgs were temporarily set to an intermediate value in 
       * determine_order_of_fixes ().  We need to reset them to a 
       * value which will make subsequent graph processing easier in 
       * installp.
       */
      reset_state_of_medialess_fixes ();

      /* 
       * Reset a few fields in ckprereq structure that may cause some harm if
       * they are left in tact for the remainder of the installp operation.
       * (ie.  Don't leave them there tempting later use when they may not
       * be accurate.)
       */
      check_prereq.First_Option = NIL (Option_t);
      check_prereq.SOP = NIL (Option_t);
      check_prereq.Fail_SOP = NIL (Option_t);
   }

} /* uninitialize_inu_ckreq */

/*--------------------------------------------------------------------------*
**
** NAME: disable_non_oem_updates
**
** FUNCTION: For systems which have been altered to be OEM-specific (usually
**           because of incompatible hardware), do not allow non-OEM specific
**           updates to be applied on top of OEM filesets.  To do this, we
**           check the subgraph of non-OEM updates to determine if there are
**           OEM requisites on that same fileset underneath.  If so, mark the update as
**           cannot apply.  This function should ONLY be called if this is an
**           OEM-modified system, as determined by the existence of
**           /usr/lib/objrepos/oem.
**
** RETURNS:  This is a void function.
**
**--------------------------------------------------------------------------*/

static void disable_non_oem_updates
(
)
{
    fix_info_type * fix;
    char oem_system_name [MAX_LANG_LEN+1];
    FILE * oem_fp;
    long oem_flag = 0;

    /* 
     * Get the OEM identifier name in order to give a better message in case
     * an OEM requisite getting pulled in does not match the name of the
     * OEM system supplier.
     */
    oem_system_name[0] = '\0';
    if ((oem_fp = fopen ("/usr/lib/objrepos/oem", "r")) != NIL (FILE))
    {
        fgets(&oem_system_name, MAX_LANG_LEN+1, oem_fp);
	strtok((char *) &oem_system_name," \n");
    }

    for (fix = check_prereq.fix_info_anchor->next_fix;
         fix != NIL (fix_info_type);
         fix = fix->next_fix)
    {
	  /* 
	   * Check every fix which is not an OEM fix to see if it is going to 
	   * overwrite any OEM modifications.
	   */ 
          if (IF_UPDATE(fix ->op_type) &&
	      !oem(fix) &&
              (fix->apply_state != ALL_PARTS_APPLIED))
              if ((oem_flag = oem_with_same_name_in_subgraph (fix->name,
						 fix, oem_system_name)) != 0)
              {
                 fix->apply_state = CANNOT_APPLY;
		 /*
                  * mark fix with appropriate reporting code;
		  */
		 fix->flags |= oem_flag;
              }
              unmark_SUBGRAPH (fix, VISITING_THIS_NODE);
    }
}

/*--------------------------------------------------------------------------*
**
** NAME: oem_with_same_name_in_subgraph
**
** FUNCTION: Check the subgraph of non-OEM updates to determine if there are
**           OEM requisites on that same fileset underneath.
**
** RETURNS:  TRUE if an OEM modification is in the subgraph
**           FALSE if no OEM modification is in the subgraph
**
**--------------------------------------------------------------------------*/

static long oem_with_same_name_in_subgraph
(
char          * name,
fix_info_type * fix,
char          * oem_system_name
)
{
    
    requisite_list_type * req;
    long      oem_flag;

    /*
     * Check to see if we've been here before.  No deja vu allowed.
     */
    if (fix->flags & VISITING_THIS_NODE)
       return (0);
    /*
     * We shall never pass this way again.
     */
    fix->flags |= VISITING_THIS_NODE;
    
    /*
     * If OEM requisite has same name as dependent, OEM specific or mismatch
     */
    if ((strcmp (name, fix->name) == 0) && oem(fix))
    {
	if (strcmp (&fix->TOC_Ptr->lang[4], oem_system_name) == 0)
            return (FAILED_OEM_SPECIFIC);
	else
            return (FAILED_OEM_MISMATCH);
    }

    /*
     * Check all of the requisites at this level...and the next... and the next...
     */
    for (req = fix->requisites;
         req != NIL (requisite_list_type);
         req = req->next_requisite)
    {
       if ((oem_flag = oem_with_same_name_in_subgraph(name,
					  req->fix_ptr, oem_system_name)) != 0)
          return (oem_flag);
    }

    /*
     * All clear.  No OEM requisites at this level.
     */
    return (0);
}
