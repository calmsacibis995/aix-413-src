static char sccsid[] = "@(#)08  1.5  src/bos/usr/sbin/install/installp/inu_print_fail_stats.c, cmdinstl, bos411, 9428A410j 3/7/94 17:01:59";

/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inu_print_fail_stats 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include  <installp.h>

/*--------------------------------------------------------------------------*
**
** NAME:  inu_print_stat
**
** FUNCTION:  Prints summary of successes or failures from the operation
**            (ie. apply, commit, reject, deinstall, cleanup) which was
**            just performed for each op between cur_op and next_op.
**
** EXECUTION ENVIRONMENT:  called from inu_apply_opts, inu_commit_opts, 
**            inu_reject_opts, inu_deinstall_opts, and inu_cleanup_opts.
**
** RETURNS: void
**
**--------------------------------------------------------------------------*/

void inu_print_fail_stats (Option_t * cur_op, Option_t * next_op)
{
   Option_t * op;
   char       levname [MAX_LPP_NAME + MAX_LEVEL_LEN];
   boolean    printed_header = FALSE;
   
   /* print a list of each failed levname */
   for (op = cur_op; op != next_op; op = op->next)
      if (op->Status != STAT_SUCCESS  &&  op->Status != STAT_CLEANUP_SUCCESS)
      { 
         if ( ! printed_header)
         {
            /* derive op_string for printing header below */
            switch (cur_op->operation)
            {
               case OP_APPLY:
               case OP_APPLYCOMMIT:
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                             INP_APPLY_FAIL_PART_E, INP_APPLY_FAIL_PART_D), 
                             inu_tree_string (cur_op->vpd_tree));
                  break;
               case OP_CLEANUP_APPLY:
               case OP_CLEANUP_COMMIT:
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                             INP_CLEAN_FAIL_PART_E, INP_CLEAN_FAIL_PART_D), 
                             inu_tree_string (cur_op->vpd_tree));
                  break;
               case OP_REJECT:
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                             INP_REJECT_FAIL_PART_E, INP_REJECT_FAIL_PART_D), 
                             inu_tree_string (cur_op->vpd_tree));
                  break;
               case OP_DEINSTALL:
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                             INP_DEINSTALL_FAIL_PART_E, 
                             INP_DEINSTALL_FAIL_PART_D), 
                             inu_tree_string (cur_op->vpd_tree));
                  break;
               default:
                  return;
                  break;
            }
 
            printed_header = TRUE;
         }

         (void) inu_get_optlevname_from_Option_t (op, levname);
         instl_msg (FAIL_MSG, "\t%s\n", levname);
      }
}
