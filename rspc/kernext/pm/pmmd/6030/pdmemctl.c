/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Memory Controller Chip Device Control Routines
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ************************************************************************* */
/* *                                                                       * */
/* * Module Name: PDMEMCTL.C                                               * */
/* * Description: Memory Controller Chip Device Control Routines           * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDMEMCTL.H/PDIDAHO.H/PDEAGLE.H     * */
/* *              files should be included in this file                    * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdmemctl.h"
#include "pdidaho.h"
#include "pdeagle.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *   Memory Controller Definition    * */
/* ************************************* */
/* ---------------------- */
/*  Memory control class  */
/* ---------------------- */
VOID  MemoryCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *          Memory Objects           * */
/* ************************************* */
/* ---------------------- */
/*  Memory control class  */
/* ---------------------- */
OBJMEMORYCTL   oMemoryCtl = {(POBJCTL)MemoryCtl, ControllerIdaho};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************** */
/* *           Memory Control Controller            * */
/* ************************************************** */
/* *                                                * */
/* * - Initialize                                   * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGMEM_INITIALIZECTL        * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Query memory controller type                 * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGMEM_QUERY_CONTROLLERTYPE * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *       Parameter1 = type                        * */
/* *                                                * */
/* * - Enter suspend                                * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGMEM_ENTER_SUSPEND        * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Save device context                          * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGMEM_SAVE_CONTEXT         * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Restore device context                       * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGMEM_RESTORE_CONTEXT      * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* ************************************************** */
VOID  MemoryCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   MSGIDAHO       mIdaho;
   MSGEAGLE       mEagle;
   CRESULT        rc;
   POBJMEMORYCTL  pmemoryctl;

   /* Get pointer to Memory control object */
   /*                                      */
   pmemoryctl = (POBJMEMORYCTL)poObj;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGMEM_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(mIdaho, MSGIDHO_INITIALIZECTL);
         SendMsg(mIdaho, oIdahoCtl);
         if(!(rc = SelectResult(mIdaho)))
         {
            pmemoryctl->ControllerType = ControllerIdaho;
         }
         else
         {
            BuildMsg(mEagle, MSGEAGL_INITIALIZECTL);
            SendMsg(mEagle, oEagleCtl);
            if(!(rc = SelectResult(mEagle)))
            {
               pmemoryctl->ControllerType = ControllerEagle;
            }
            else
            {
               rc = ERRMEM_CANNOT_INITIALIZE;
            } /* endif */
         } /* endif */
         break;

      case MSGMEM_QUERY_CONTROLLERTYPE:
         /* Query memory controller type */
         /*                              */
         SelectParm1(*pmMsg) = (CPARAMETER)pmemoryctl->ControllerType;
         rc = ERRMEM_NOERROR;
         break;

      case MSGMEM_ENTER_SUSPEND:
         /* Enter suspend */
         /*               */
         switch(pmemoryctl->ControllerType)
         {
            case ControllerIdaho:
               BuildMsg(mIdaho, MSGIDHO_ENTER_SUSPEND);
               SendMsg(mIdaho, oIdahoCtl);
               rc = SelectResult(mIdaho);
               break;
            case ControllerEagle:
               BuildMsg(mEagle, MSGEAGL_ENTER_SUSPEND);
               SendMsg(mEagle, oEagleCtl);
               rc = SelectResult(mEagle);
               break;
            default:
               rc = ERRMEM_NOT_INITIALIZED;
         } /* endswitch */
         break;

      case MSGMEM_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         switch(pmemoryctl->ControllerType)
         {
            case ControllerIdaho:
               BuildMsg(mIdaho, MSGIDHO_SAVE_CONTEXT);
               SendMsg(mIdaho, oIdahoCtl);
               rc = SelectResult(mIdaho);
               break;
            case ControllerEagle:
               BuildMsg(mEagle, MSGEAGL_SAVE_CONTEXT);
               SendMsg(mEagle, oEagleCtl);
               rc = SelectResult(mEagle);
               break;
            default:
               rc = ERRMEM_NOT_INITIALIZED;
         } /* endswitch */
         break;

      case MSGMEM_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         switch(pmemoryctl->ControllerType)
         {
            case ControllerIdaho:
               BuildMsg(mIdaho, MSGIDHO_RESTORE_CONTEXT);
               SendMsg(mIdaho, oIdahoCtl);
               rc = SelectResult(mIdaho);
               break;
            case ControllerEagle:
               BuildMsg(mEagle, MSGEAGL_RESTORE_CONTEXT);
               SendMsg(mEagle, oEagleCtl);
               rc = SelectResult(mEagle);
               break;
            default:
               rc = ERRMEM_NOT_INITIALIZED;
         } /* endswitch */
         break;

      default:
         rc = ERRMEM_INVALID_MESSAGE;
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
