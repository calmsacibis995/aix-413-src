static char     sccsid[] = "@(#)22  1.8  src/bos/usr/sbin/install/installp/inu_vpd_set_states.c, cmdinstl, bos411, 9428A410j 6/18/93 16:53:12";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_vpd_set_states
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>

/*--------------------------------------------------------------------------*
**
** NAME: inu_vpd_set_states
**
** FUNCTION:
**  Set the state of the LPPs being processed.
**
** EXECUTION ENVIRONMENT: Part of user command.
**
** NOTES:
**  The state parameter refers to the state that's valid for the LPP and
**  PRODUCT databases.
**
** ON ENTRY:
**  state   specifies the state to be set (comes from constants
**          defined in /usr/include/swvpd.h
**
**--------------------------------------------------------------------------*/

void inu_vpd_set_states (Option_t * op,     /* Ptr to the first option of this
                                             * operation. */
                         int        action) /* Which action that should be
                                             * performed. */
{
   prod_t          prod;
   short           sav_prod_state;
   int             rc;
   int             del_apply_hold = 0;  /* flag indicating we need to delete
                                         * row */
   int             del_commit_hold = 0; /* flag indicating we need to delete
                                         * row */
   char            sav_dir[PATH_MAX];   /* buffer for the save directory */

   /*-----------------------------------------------------------------------*
    * If we're dealing with an action that will require us to remove the
    * save files, the build the path to where the save directory resides.
    *-----------------------------------------------------------------------*/

   if (action == SET_HOLD_STATES_TO_BROKEN || action == DELETE_HOLD_STATES)
      (void) inu_get_savdir (op -> vpd_tree, op, sav_dir);

   /*-----------------*
    * PRODUCT DATABASE
    *-----------------------------------------------------------------------*
    * Change the state to parameter "state" in the PRODUCT database.
    *-----------------------------------------------------------------------*/

   strcpy (prod.lpp_name, op -> name);
   if (vpdget (PRODUCT_TABLE, PROD_LPP_NAME, &prod) == VPD_OK)
   {
      do
      {
         sav_prod_state = prod.state;
         switch (action)
         {
            case SET_STATES_TO_HOLD:
               /*-----------------------------------------------------------*
                * Change the state to HOLD for all product records that match
                * the lpp-option name.
                *-----------------------------------------------------------*/

               switch (prod.state)
               {
                  case ST_APPLIED:
                     prod.state = ST_APPLY_HOLD;
                     break;
                  case ST_COMMITTED:
                     prod.state = ST_COMMIT_HOLD;
                     break;
                  default:
                     break;
               }
               break;

            case SET_HOLD_STATES_TO_BROKEN:
               /*-----------------------------------------------------------*
                * Change the state to BROKEN for all product records that match
                * the lpp-option name and are currently in the HOLD state.
                * Also remove the save files for the option if it was in the
                * applied state.
                *-----------------------------------------------------------*/

               switch (prod.state)
               {
                  case ST_APPLY_HOLD:
                     prod.state = ST_BROKEN;
                     (void) rm_1_opt (sav_dir, op -> name);
                     break;
                  case ST_COMMIT_HOLD:
                  case ST_APPLYING:
                     prod.state = ST_BROKEN;
                     break;
                  default:
                     break;
               }
               break;

            case SET_HOLD_STATES_TO_PREVIOUS:
               /*-----------------------------------------------------------*
                * Change the state from HOLD to the previous state for all
                * product records that match the lpp-option name.
                *-----------------------------------------------------------*/

               switch (prod.state)
               {
                  case ST_APPLY_HOLD:
                     prod.state = ST_APPLIED;
                     break;
                  case ST_COMMIT_HOLD:
                     prod.state = ST_COMMITTED;
                     break;
                  default:
                     break;
               }
               break;

            case DELETE_HOLD_STATES:
               /*-----------------------------------------------------------*
                * Delete the product records that are in the HOLD state for
                * all product records that match the lpp-option name.
                * Also remove the save files for the option if it was in the
                * APPLY_HOLD state.
                *-----------------------------------------------------------*/

               switch (prod.state)
               {
                  case ST_APPLY_HOLD:
                     del_apply_hold = 1;
                     (void) rm_1_opt (sav_dir, op -> name);
                     break;
                  case ST_COMMIT_HOLD:
                     del_commit_hold = 1;
                     break;
                  default:
                     break;
               }
               break;

            default:
               break;

         }      /* end switch (action) */

         /*---------------------------------------------------------------*
          * If the state was changed, then put new state in the database.
          *---------------------------------------------------------------*/

         if (prod.state != sav_prod_state)
         {
            rc = vpdchgget (PRODUCT_TABLE, &prod);
            inu_ck_vpd_sys_rc (rc);
         } /* end if */

        vpd_free_vchars (PRODUCT_TABLE, &prod);

      } while (vpdgetnxt (PRODUCT_TABLE, &prod) == VPD_OK);

   } /* end if (vpdget(PRODUCT_TABLE, PROD_LPP_NAME, &prod) == VPD_OK) */

   /*-----------------------------------------------------------------------*
    * If we need to delete any vpd rows (based on the flags del_apply_hold
    * and del_commit_hold) then do it now...
    *-----------------------------------------------------------------------*/

   if (del_apply_hold)
   {
      strcpy (prod.lpp_name, op -> name);
      prod.state = ST_APPLY_HOLD;
      rc = vpddelall (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE, &prod);
      inu_ck_vpd_sys_rc (rc);
   }/* end if */

   if (del_commit_hold)
   {
      strcpy (prod.lpp_name, op -> name);
      prod.state = ST_COMMIT_HOLD;
      rc = vpddelall (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE, &prod);
      inu_ck_vpd_sys_rc (rc);
   } /* end if */
} /* end inu_vpd_set_states */
