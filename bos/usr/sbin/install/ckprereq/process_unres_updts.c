static char sccsid[] = "@(#)90  1.6  src/bos/usr/sbin/install/ckprereq/process_unres_updts.c, cmdinstl, bos412, 9446B 11/11/94 10:19:29";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            process_unresolved_updates_on_failsop
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

static boolean process_supersede_of_missing_updt (
Option_t      * op,
fix_info_type * sup_fix,
boolean       * resolved_one,
boolean       * error);

/*--------------------------------------------------------------------------*
**
** NAME:      process_unresolved_updates_on_failsop
**
** FUNCTION:  Scans the failsop looking for "missing" updates for which there
**            may be superseding ptfs that are either already on the media
**            or already installed.  If it finds failsop entries that are
**            already installed via a superseding update, their Status fields
**            are set accordingly, for later error reporting.  If it finds
**            failsop entries for which there is a superseding update on the
**            media, the superseding update will be used instead (assuming
**            "auto-supersedes" is switched on -- otherwise, the superseding
**            update will be reported as being available).
**
** RETURNS:   void function
**
**--------------------------------------------------------------------------*/

void
process_unresolved_updates_on_failsop
(
Option_t * failsop,
boolean  * resolved_one,   /* flag telling caller if any were resolved */
boolean  * error
)
{

  fix_info_type        * new;           /* temp list traversal ptr */
  fix_info_type        * old;           /* temp list traversal ptr */
  fix_info_type        * failed_fix;    /* fail_sop fix entry */
  fix_info_type        * sup_fix;       /* fix entry for superseding ptfs */
  supersede_list_type  * superseded_by;
  Option_t             * op;            /* Option located in Option_t struct */

  /*   Examine entries on the failsop.
   *
   *   Search the fix_info table for updates that are not in the TOC (or in 1 
   *   case, not in the VPD -- for root part apply).
   *
   *   If the search succeeds in fix_info table, check for a superseding fix.
   *
   *   There are 5 possible outcomes of searching for a supersede
   *   and op->Status is set/left as follows:
   *   STAT_NOT_FOUND_ON_MEDIA    - not in TOC nor installed
   *   STAT_NUTTIN_TO_APPLY       - not in usr VPD for root part apply
   *   STAT_ALREADY_SUPERSEDED    - supersede PTF already installed
   *   STAT_CAN_BE_SUPERSEDED     - use -g to install superseding fix
   *   STAT_TO_BE_SUPERSEDED      - supersede PTF will be installed
   *
   *   When the Auto_Supersede flag is false, don't automatically use a 
   *   superseding PTF.
   *
   *   If a supersede fix is selected, the failsop's op->supersedes is set
   *   to the fix->TOC_Ptr->level.ptf so that a message can display the 
   *   superseding PTF number.  
   */

   for (op = failsop->next; 
        op != NIL (Option_t); 
        op = op->next) 
   {
      if (IS_UNRESOLVED_UPDATE(op, check_prereq.parts_to_operate_on)) 
      {
         /*
          * Look for the "missing" update in the fix info table.  If it has
          * an entry there, chances are it is an update which is either
          * superseded by a package on the media or a package that's already
          * installed.  (Look for exact match for 32 updates, look for 
          * implicit supersedes for 4.1).
          */
         if (op->level.ptf[0] != '\0')
         {
            if (failed_fix = fix_info_lookup (op->name, &(op->level),
                                                             USE_FIXID)) 
            {
               for (superseded_by = failed_fix->superseded_by; 
                    superseded_by != NIL (supersede_list_type);
                    superseded_by = 
                                superseded_by->superseding_fix->superseded_by)
               {
                   if (sup_fix = superseded_by->superseding_fix) 
                      if (! process_supersede_of_missing_updt (op, sup_fix, 
                                                               resolved_one,
                                                               error))
                      {
                         RETURN_ON_ERROR;
                         break;
                      }
               } /* for */
            }
         }
         else
         {
            if (sup_fix = impl_sups_lookup (op->name, &(op->level)))
            {
               if (! process_supersede_of_missing_updt (op, sup_fix, 
                                                        resolved_one, error))
               {
                  RETURN_ON_ERROR;
                  continue;
               }
               RETURN_ON_ERROR;

               old = sup_fix;
               new = sup_fix->next_fix;
               while (IS_STILL_SUPS_CHAIN_LR_SCAN (old, new))
               {
                  if (! process_supersede_of_missing_updt (op, new, 
                                                           resolved_one, error))
                  {  
                     RETURN_ON_ERROR;
                     break;
                  }
                  RETURN_ON_ERROR;
                  old = new;
                  new = new->next_fix;

               }/*          end while                                         */
            }   /*        end if (failed_fix = impl_sups_lookup               */
         }      /*        end if op->level.ptf                                */
      }         /*     end if IS_UNRESOLVED_UPDATE                            */
   }		/* end for loop examining options on failsop                  */
} /* process_unresolved_updates_on_failsop */

/*--------------------------------------------------------------------------*
**
** NAME:      process_supersede_of_missing_updt
**
** FUNCTION:  Looks at the superseding fix of the missing update to see if
**            it is one which should be applied.
**
** RETURNS:   TRUE if processing on the supersedes chain should continue,
**            FALSE otherwise
** 
** SIDE EFFECTS:
**            Changes the Status field of op if necessary.
**            Adds superseded fix to supersedes warning list.
**
**--------------------------------------------------------------------------*/

static boolean 
process_supersede_of_missing_updt
(
Option_t      * op,
fix_info_type * sup_fix,
boolean       * resolved_one,
boolean       * error
)
{
   char lev_buf [MAX_LPP_FULLNAME_LEN];

   /* 
    * We continue in for loop if supersede apply state is 
    * IN_TOC.  
    */
   switch (sup_fix->apply_state) 
   {
      case ALL_PARTS_APPLIED:
         op->Status = STAT_ALREADY_SUPERSEDED; /* sup aready appld*/
         break;

      case APPLY_TO_BE_COMPLETED:
      case TO_BE_EXPLICITLY_APPLIED:
         op->Status = STAT_TO_BE_SUPERSEDED; /* sup is being appld*/
         break;

      case IN_TOC:
      case PARTIALLY_APPLIED:

         if (sup_fix->apply_state == PARTIALLY_APPLIED &&
             check_prereq.parts_to_operate_on != LPP_ROOT)
            /* 
             * Tricky, only valid for root part apply.
             */
            break;

         /* 
          * if a newer supersede exists, examine it.
          */
         if (IF_3_2 (sup_fix->op_type))
         {
            if (sup_fix->superseded_by) 
               return (TRUE);
         }
         else
         { 
            if (IS_STILL_SUPS_CHAIN_LR_SCAN(sup_fix, sup_fix->next_fix))
               return (TRUE);
         }

         /*
          * Auto apply the superseding update (if we can).
          */
         if (check_prereq.Auto_Supersedes) 
         {
            /* flag for caller */
            *resolved_one                   = TRUE;

            /* flags for report routines */
            op->Status                      = STAT_TO_BE_SUPERSEDED;
            sup_fix->flags                 |= AUTO_SUPERSEDES_PKG;
 
            /* fix_info status for the rest of ckprereq */ 
            sup_fix->origin                 = COMMAND_LINE;
            sup_fix->apply_state =          TO_BE_EXPLICITLY_APPLIED;

            /* put on sorted index list for superseded_by msgs */
            add_entry_to_index_list (
                               report_anchor.superseded_index_hdr,
                               NIL (fix_info_type), op, &EMPTY_CRITERIA,
                               error);
            RETURN_ON_ERROR;

	  } 
          else                            
          {
            /* don't do auto apply, but report that you can.*/  
             if (sup_fix->apply_state == PARTIALLY_APPLIED
                                           &&
                 check_prereq.parts_to_operate_on == LPP_ROOT)
	        op->Status             = STAT_ROOT_CAN_BE_SUPERSEDED;
             else
                op->Status             = STAT_CAN_BE_SUPERSEDED;
          }
	  break;

      default:
          break;   /* supersede is not on media, being applied, and
                      not being applied -- there may be a newer one.*/
    } /* end switch apply_state */

   /* 
    * save identify of superseding update for later reports
    */
   switch (op->Status) 
   {
      case STAT_ALREADY_SUPERSEDED: 
         if (op->level.ptf[0] == '\0')
            op->supersedes = strdup(get_level_from_fix_info (sup_fix, lev_buf));
         else
            op->supersedes = strdup(sup_fix->level.ptf);
         break;
      case STAT_CAN_BE_SUPERSEDED: 
      case STAT_ROOT_CAN_BE_SUPERSEDED:
      case STAT_TO_BE_SUPERSEDED: 
         if (op->level.ptf[0] != '\0')
            op->supersedes = strdup(sup_fix->level.ptf);
         else
            op->supersedes = strdup(get_level_from_fix_info (sup_fix, lev_buf));
         break;
   }
   return (FALSE);  /* Causes caller to exit loop. */

} /* process_supersede_of_missing_updt */
