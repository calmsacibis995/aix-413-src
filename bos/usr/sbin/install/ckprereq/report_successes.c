static char sccsid[] = "@(#)64  1.15  src/bos/usr/sbin/install/ckprereq/report_successes.c, cmdinstl, bos412, 9446B 11/11/94 10:19:52";

/*
 *   COMPONENT_NAME:  CMDINSTL
 *
 * FUNCTIONS:
 *            report_successes
 *            print_successes_by_request_type
 *            print_successes_in_sop_order
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
 * Prototypes
 */

static void print_successes_by_request_type (
index_list_type * sop_index_hdr,
index_list_type * sop_index_tail,
short             request_type,
boolean         * error);

static void print_successes_in_sop_order (
Option_t        * success_sop,
index_list_type * sop_index_hdr,
index_list_type * sop_index_tail,
boolean         * error);

/*--------------------------------------------------------------------------*
**
** NAME: report_successes
**
** FUNCTION:  Prints lists of packages which will be applied, committed,
**            rejected or deinstalled.  This information is obtained
**            from the SOP (ie. the list being passed back to installp).
**            The level of verbosity specified by installp affects the
**            type and quantity of information displayed.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
report_successes 
 (
Option_t * success_sop, 
boolean * error
)
{ 
   index_list_type * sop_index_hdr;
   index_list_type * sop_index_tail;
   Option_t        * sop_ptr;

   /*
    * Rules/Strategy:
    *   -- Results are printed from the sop (ie. the list being passed back to
    *      installp.
    *   -- Sop entries are expected to have been flagged in their Status 
    *      and/or flag fields in determine_order_of_fixes to aid in 
    *      reporting results.
    *   IF (lowest level of verbosity (ie. V1))
    *     -- print successful pkgs according to whether or not they 
    *        were requested on the cmd line (ie. passed from installp)
    *        or pulled in automatically.  
    *        -- explicitly requested packages include superseding pkgs that
    *           were requested instead of explicitly requested pkgs (these
    *           will be flagged with a *)
    *        -- "pulled in" packages include "non-requested"
    *           -- mandatory packages (ie. installp updates)
    *           -- auto-ifreqs (ifreqs of pkgs which are already installed)
    *           -- maintenance level pkgs (being automatically installed
    *              because all requisites have been/are being installed)
    *           -- requisites (of requested pkgs or other pkgs above)
    *              (dependents for reject and deinstall ops).
    *     -- within each section above, packages are printed in 
    *        alphanumeric order.
    *   ELSE  ** Higher levels of verbosity **
    *     -- packages will be listed in SOP order
    *     -- the "request type" (ie. explicit, auto-ifreq, etc.) will be
    *        indicated by an alphabetic character.
    *     -- For V3 and V4, requisites of packages being installed will
    *        be listed, indented beneath each package.  Requisite information
    *        is obtained from the requisite graph.  Requisites are listed
    *        in alphanumeric order.  The installed/being installed state
    *        of each requisite is also displayed.
    *     -- There is no difference between V3 and V4 for successful 
    *        package listings.
    * 
    */

   if (success_sop->next == NIL (Option_t))
   {
      return;  /* No need to print anything if the sop is empty. */
   }

   /*
    * Print our progress/status msg to indicate we're finished checking
    * requisites.  We do it here if we didn't already do it in
    * report_failures ().
    */
   if (! check_prereq.printed_finished_msg)
   {
      inu_msg (ckp_prog, MSGSTR (MS_INUCOMMON, CMN_DONE_I, CMN_DONE_D));
      inu_msg (ckp_prog, MSGSTR (MS_CKPREREQ, CKP_RESULTS_I, CKP_RESULTS_D));
   }

   print_success_section_header ();

   if (check_prereq.instp_verbosity == V1)  
   {
      /*
       * Create a sorted index into the sop.
       */
      create_sop_index (success_sop, &sop_index_hdr, &sop_index_tail, 
                        error);
      RETURN_ON_ERROR;

      /*
       * Print packages that were requested from installp cmd line.
       * (This will include auto supersede packages which were chosen
       * by ckprereq instead of the older requested packages.)
       */
      print_successes_by_request_type (sop_index_hdr, sop_index_tail, 
                                       EXPLICIT, error);
      RETURN_ON_ERROR;

      /*
       * Print mandatory packages (SR and SF) and unresolved ifreqs that were 
       * not requested explicitly.
       */
      if (check_prereq.mode == OP_APPLY)
      {
         print_successes_by_request_type (sop_index_hdr, sop_index_tail, 
                                          MANDATORY, error);
         RETURN_ON_ERROR;
         print_successes_by_request_type (sop_index_hdr, sop_index_tail, 
                                          AUTO_IFREQ, error);
         RETURN_ON_ERROR;
      } /* end if */

      /*
       * Print requisites/dependents being automatically pulled in. 
       */
      print_successes_by_request_type (sop_index_hdr, sop_index_tail, 
                                       IMPLICIT, error);
      RETURN_ON_ERROR;

      /*
       * Print any MC type updates (maintenance packaging ptfs) being 
       * automatically applied/committed.
       */
      if (check_prereq.mode == OP_APPLY ||
          check_prereq.mode == OP_COMMIT)
      {
         print_successes_by_request_type (sop_index_hdr, sop_index_tail, 
                                          AUTO_MC, error);
         RETURN_ON_ERROR;
      }

      /*
       * Free the index list.
       */
      free_index_nodes_between (sop_index_hdr, sop_index_tail);
      free_sop_index_hdrs (sop_index_hdr, sop_index_tail);
   }
   else
   { 
      /*
       * Higher level of verbosity.  Print things in order of installation.
       */
      print_successes_in_sop_order (success_sop, sop_index_hdr, 
                                    sop_index_tail, error);
      RETURN_ON_ERROR;
   } /* end if */

   inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_E_O_SUCCESS_I, 
                                           CKP_E_O_SUCCESS_D));

  /*
   * Reset the Status fields so that we don't confuse installp.  Some of these
   * may have been set in determine_order to report better messages here.
   * While doing that, set the flag bit of AUTO_SUPERSEDES updates to look
   * like they were explicitly requested at the installp cmd line.  This will
   * make printing the STATISTICS table back in installp more straight forward.
   */
  for (sop_ptr = success_sop;
       sop_ptr != NIL (Option_t);
       sop_ptr = sop_ptr->next)
  {
     if (IS_AUTO_SUPERSEDE_PKG (sop_ptr))
        sop_ptr->flag = SET_SELECTED_BIT (sop_ptr->flag);
     sop_ptr->Status = 0;
  }
} /* report_successes */

/*--------------------------------------------------------------------------*
**
** NAME: print_successes_by_request_type
**
** FUNCTION:  Prints packages which were explicitly and any superseding 
**            packages (which substitute requested packages) chosen by
**            inu_ckreq.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
print_successes_by_request_type
 (
index_list_type * sop_index_hdr,
index_list_type * sop_index_tail,
short             request_type,
boolean         * error
)
{
  boolean           printed_hdr_msg;
  index_list_type * index_node;
  Option_t        * next_sop_ptr;
  Option_t        * sop_ptr;

   /*
    * Loop through the index list looking for packages matching the 
    * "request_type" argument passed.  When one is found, print all packages of
    * that type for that option.
    */
  printed_hdr_msg = FALSE;
  for (index_node = sop_index_hdr->next_index_node;
       index_node != sop_index_tail;
       index_node = index_node->next_index_node)
  {
     sop_ptr = index_node->sop_ptr;
     if (( (request_type == EXPLICIT) &&
          (IS_EXPL_REQUESTED_PKG (sop_ptr) || IS_AUTO_SUPERSEDE_PKG (sop_ptr)))
                                   ||
         (request_type == IMPLICIT && IS_IMPL_REQUESTED_PKG (sop_ptr))
                                   ||
         (request_type == AUTO_IFREQ && IS_AUTO_IFREQ_PKG (sop_ptr)) 
                                   ||
         (request_type == AUTO_MC && IS_AUTO_MAINT_PKG (sop_ptr)) 
                                   ||
         (request_type == MANDATORY && IS_AUTO_MANDATORY_PKG (sop_ptr)) )
     {
        if (! printed_hdr_msg)
        {
           print_success_hdr_by_request_type (request_type);
           printed_hdr_msg = TRUE;
        }

        print_successful_sop_entry (sop_ptr, error);
        RETURN_ON_ERROR;
     }

  } /* end for */

  if (printed_hdr_msg)
     inu_msg (ckp_succ, "\n");
     
} /* print_successes_by_request_type */

/*--------------------------------------------------------------------------*
**
** NAME: print_successes_in_sop_order
**
** FUNCTION:  Prints packages being applied/committed/rejected/deinstalled
**            in the order in which they will be acted upon (by installp).
**            For higher levels of verbosity (V3 and V4), requisites/dependents
**            of packages being operated upon are also printed.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
print_successes_in_sop_order 
 (
Option_t * success_sop,
index_list_type * sop_index_hdr,
index_list_type * sop_index_tail,
boolean         * error
)
{
   boolean           print_hdr_msg;
   fix_info_type   * fix_info;
   index_list_type * fix_info_index_hdr;
   index_list_type * fix_info_index_tail;
   index_list_type * index_node;
   Option_t        * sop_ptr;

   /*
    * We'll loop through the sop printing each package we find.  A code 
    * will be printed to the RHS of each package and a key will be printed
    * after all successes are printed explaining the various codes.  For higher
    * levels of verbosity, (>V2) the requisites/dependents of each package being
    * applied/committed/rejected/deinstalled will be printed.  
    */

   if (check_prereq.instp_verbosity > V2)
   {
      /*
       * Initialize an index list used to save requisites of a given package
       * from the sop.
       */
      init_fix_info_index (&fix_info_index_hdr, &fix_info_index_tail, error);
      RETURN_ON_ERROR;
   }

   for (sop_ptr = success_sop -> next;
        sop_ptr != NIL (Option_t);
        sop_ptr = sop_ptr -> next)
   {
     /*
      * Make sure we don't print both parts for usr-root pkgs.  The 
      * "OTHER_PART" bit was set in determine_order_of_fixes ().
      */
     if (IF_OTHER_PART_ON_SOP (sop_ptr->flag))
        continue;

     /*
      * Print the successful sop entry.  This routine also adds the
      * code explaining whether the package is being explicitly installed,
      * an auto_ifreq, etc.
      */
     print_successful_sop_entry (sop_ptr, error); 
     RETURN_ON_ERROR;

     
     /*
      * Now that we've printed the package, let's print its requisites for
      * > V2.  We'll do this by gathering the requisites from the graph
      * on a sorted index list.  We DON'T perform this for requisites 
      * being pulled in with -g.
      */
     if ((check_prereq.instp_verbosity > V2) &&
         ! (IS_IMPL_REQUESTED_PKG (sop_ptr)))
     {
        fix_info = fix_info_lookup (sop_ptr -> name, &(sop_ptr -> level),
                                    USE_FIXID);
        if (fix_info != NIL (fix_info_type))
        {
           create_fix_info_index_from_subgraph (fix_info_index_hdr, 
                                                fix_info, fix_info, error);
           RETURN_ON_ERROR;
           unmark_graph (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED);
        } /* end if */

        /*
         * Print the requisites/dependents (if there are any). 
         */
        if (fix_info_index_hdr->next_index_node != fix_info_index_tail)
        {
           if (check_prereq.mode == OP_REJECT)
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_DEPENDENTS_COLON_I,
                       CKP_DEPENDENTS_COLON_D));
           else
              inu_msg (ckp_succ, MSGSTR (MS_CKPREREQ, CKP_REQUISITES_COLON_I,
                       CKP_REQUISITES_COLON_D));
        } /* end if */

        for (index_node = fix_info_index_hdr->next_index_node;
             index_node != fix_info_index_tail;
             index_node = index_node->next_index_node)
        {
           fix_info = index_node->fix_ptr;
           print_successful_requisite (fix_info);

        } /* end for */
        free_index_nodes_between (fix_info_index_hdr, fix_info_index_tail);

     } /* end if */
     inu_msg (ckp_succ, "\n");
   } /* end for */

   if (check_prereq.instp_verbosity > V2)
      free_fix_info_index_hdr (fix_info_index_hdr, fix_info_index_tail);

   inu_msg (ckp_succ, "\n");

   print_success_key ();

} /* print_successes_in_sop_order */
