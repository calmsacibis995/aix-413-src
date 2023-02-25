static char sccsid[] = "@(#)75  1.3  src/rspc/kernext/pm/pmmd/6030/pdpower.c, pwrsysx, rspc41J, 9521A_all 5/22/95 20:52:52";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Device Power Control Routines
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
/* * Module Name: PDPOWER.C                                                * */
/* * Description: Device Power Control Routines                            * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDPOWER.H/PDCARERA.H/PDCOOPER.H/   * */
/* *              PDUMCU.H/PDCPU.H/PDRTC.H files should be included in this* */
/* *              file                                                     * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdpower.h"
#include "pdcarera.h"
#include "pdcooper.h"
#include "pdumcu.h"
#include "pdcpu.h"
#include "pdrtc.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *       Power Data Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  WAITTIME_LCDON                   (100)    /* Wait time after resume */
                                                   /* (100 msec)             */
#define  WAITTIME_LCDPANEL                (5)      /* Wait time to stabilize */
                                                   /* (5 msec)               */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    Power Controller Definition    * */
/* ************************************* */
/* --------------------- */
/*  Power control class  */
/* --------------------- */
VOID  PowerCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *           Power Objects           * */
/* ************************************* */
/* --------------------- */
/*  Power control class  */
/* --------------------- */
OBJPOWERCTL oPowerCtl = {(POBJCTL)PowerCtl, PRMPWR_POWERON};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************* */
/* *           Power Control Controller            * */
/* ************************************************* */
/* *                                               * */
/* * - Initialize                                  * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_INITIALIZECTL       * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *                                               * */
/* * - Get device power                            * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_GET_xxxPOWER        * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_POWERON )          * */
/* *                    (PRMPWR_POWEROFF)          * */
/* *                                               * */
/* * - Set device power                            * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_SET_xxxPOWER        * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_POWERON )          * */
/* *                    (PRMPWR_POWEROFF)          * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *                                               * */
/* * - Get LCD brightness power                    * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_GET_LCDBRIGHTNESS   * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_FULLON)            * */
/* *                    (PRMPWR_HALFON)            * */
/* *                                               * */
/* * - Set LCD brightness power                    * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_SET_LCDBRIGHTNESS   * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_FULLON)            * */
/* *                    (PRMPWR_HALFON)            * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *                                               * */
/* * - Get LCD status                              * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_GET_LCDSTATUS       * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_LCDENABLE )        * */
/* *                    (PRMPWR_LCDDISABLE)        * */
/* *                                               * */
/* * - Set LCD status                              * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_SET_LCDSTATUS       * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_LCDENABLE )        * */
/* *                    (PRMPWR_LCDDISABLE)        * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *                                               * */
/* * - Get SCSI terminator power                   * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_GET_SCSITERMPOWER   * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_TERMINATORON )     * */
/* *                    (PRMPWR_TERMINATORSTANDBY) * */
/* *                    (PRMPWR_TERMINATOROFF)     * */
/* *                                               * */
/* * - Set SCSI terminator power                   * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_SET_SCSITERMPOWER   * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_TERMINATORON )     * */
/* *                    (PRMPWR_TERMINATORSTANDBY) * */
/* *                    (PRMPWR_TERMINATOROFF)     * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *                                               * */
/* * - Get CRT device power                        * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_GET_CRTPOWER        * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_CRTON     )        * */
/* *                    (PRMPWR_CRTSTANDBY)        * */
/* *                    (PRMPWR_CRTSUSPEND)        * */
/* *                    (PRMPWR_CRTOFF    )        * */
/* *                                               * */
/* * - Set CRT device power                        * */
/* *     [ Input ]                                 * */
/* *       Message    = MSGPWR_SET_CRTPOWER        * */
/* *       Parameter1 = power                      * */
/* *                    (PRMPWR_CRTON     )        * */
/* *                    (PRMPWR_CRTSTANDBY)        * */
/* *                    (PRMPWR_CRTSUSPEND)        * */
/* *                    (PRMPWR_CRTOFF    )        * */
/* *     [ Output ]                                * */
/* *       Result     = result code                * */
/* *                                               * */
/* ************************************************* */
VOID  PowerCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   MSGCARRERA     mCarr;
   MSGCOOPER      mCoop;
   MSGUMCU        mUmcu;
   MSGCPU         mCpu;
   MSGRTC         mRtc;
   MSGPOWER       msg;
   CRESULT        rc;
   POBJPOWERCTL   ppwrctl;

   /* Get pointer to Power control object */
   /*                                     */
   ppwrctl = (POBJPOWERCTL)poObj;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPWR_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsgWithParm1(msg, MSGPWR_SET_MAINPOWER,
                                PRMPWR_POWERON);
         SendMsg(msg, oPowerCtl);
         BuildMsgWithParm1(msg, MSGPWR_SET_SUSPENDPOWER,
                                PRMPWR_POWERON);
         SendMsg(msg, oPowerCtl);

         rc = SelectResult(msg);
         break;

      case MSGPWR_SET_MAINPOWER:
         /* Set main power */
         /*                */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               rc = ERRPWR_NOERROR;
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mRtc, MSGRTC_SET_AUXBATTERYSTATUS,
                                       PRMRTC_AUXBATDISABLE);
               SendMsg(mRtc, oRtcCtl);

               BuildMsgWithParm1(mRtc, MSGRTC_SET_SQUAREWAVEOUTPUT,
                                       PRMRTC_SQWDISABLE);
               SendMsg(mRtc, oRtcCtl);

               BuildMsg(mUmcu, MSGUMCU_CMD_POWEROFF);
               SendMsg(mUmcu, oUmcuCtl);

               if(!(rc = SelectResult(mUmcu)))
               {
                  while(1)
                  {
                     DisableInterrupt();

                     BuildMsg(mCpu, MSGCPU_SET_SLEEP);
                     SendMsg(mCpu, oCpuCtl);
                  } /* endwhile */
               }
               else
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_SET_SUSPENDPOWER:
         /* Set suspend power */
         /*                   */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsg(mCarr, MSGCARR_EXIT_SUSPEND);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */

               if(ppwrctl->cpLcdStatus == PRMPWR_POWERON)
               {
                  WaitTime(WAITTIME_LCDON);
                  BuildMsgWithParm1(msg, MSGPWR_SET_LCDPLPOWER,
                                         PRMPWR_POWERON);
                  SendMsg(msg, oPowerCtl);
                  WaitTime(WAITTIME_LCDPANEL);
                  BuildMsgWithParm1(msg, MSGPWR_SET_LCDPOWER,
                                         PRMPWR_POWERON);
                  SendMsg(msg, oPowerCtl);
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mRtc, MSGRTC_SET_AUXBATTERYSTATUS,
                                       PRMRTC_AUXBATENABLE);
               SendMsg(mRtc, oRtcCtl);

               BuildMsgWithParm1(mRtc, MSGRTC_SET_SQUAREWAVEOUTPUT,
                                       PRMRTC_SQW32768HZ);
               SendMsg(mRtc, oRtcCtl);

               BuildMsg(msg, MSGPWR_GET_LCDPOWER);
               SendMsg(msg, oPowerCtl);
               ppwrctl->cpLcdStatus = !SelectResult(msg) ? SelectParm1(msg)
                                                         : PRMPWR_POWERON;

               BuildMsgWithParm1(msg, MSGPWR_SET_LCDBRIGHTNESS,
                                      PRMPWR_HALFON);
               SendMsg(msg, oPowerCtl);
               BuildMsgWithParm1(msg, MSGPWR_SET_LCDPOWER,
                                      PRMPWR_POWEROFF);
               SendMsg(msg, oPowerCtl);

               BuildMsg(mCarr, MSGCARR_SET_DEVICEALLOFF);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsg(mCarr, MSGCARR_GET_REVISION);
               SendMsg(mCarr, oCarreraCtl);
               if(!SelectResult(mCarr) && !SelectParm1(mCarr))
               {
                  BuildMsgWithParm1(mCarr, MSGCARR_SET_SCSICLOCKSTOP,
                                           PRMCARR_LINEHIGH);
                  SendMsg(mCarr, oCarreraCtl);
               } /* endif */

               BuildMsg(mCarr, MSGCARR_SET_SUSPENDRESUMETIMING);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsg(mCarr, MSGCARR_ENTER_SUSPEND);
               SendMsg(mCarr, oCarreraCtl);

               if(!(rc = SelectResult(mCarr)))
               {
                  BuildMsg(mCarr, MSGCARR_GET_INDEX);
                  SendMsg(mCarr, oCarreraCtl);

                  while(1)
                  {
                     EnableInterrupt();

                     BuildMsg(mCpu, MSGCPU_SET_SLEEPANDSUSPEND);
                     SendMsg(mCpu, oCpuCtl);
                  } /* endwhile */
               }
               else
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_LCDPOWER:
         /* Get LCD device power */
         /*                      */
      case MSGPWR_GET_LCDBLPOWER:
         /* Get LCD backlight power */
         /*                         */
         BuildMsg(mCarr, MSGCARR_GET_LCDBACKLIGHTOFF);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_LCDPOWER:
         /* Set LCD device power */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDOFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDENABLE,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDBACKLIGHTOFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDBACKLIGHTOFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDENABLE,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDOFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_SET_LCDBLPOWER:
         /* Set LCD backlight power */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDBACKLIGHTOFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDBACKLIGHTOFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_LCDPLPOWER:
         /* Get LCD panel power */
         /*                     */
         BuildMsg(mCarr, MSGCARR_GET_LCDOFF);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_LCDPLPOWER:
         /* Set LCD panel power */
         /*                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDOFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDOFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_VIDEOPOWER:
         /* Get video chip power */
         /*                      */
         BuildMsg(mCarr, MSGCARR_GET_VIDEOLOWPOWER);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_VIDEOPOWER:
         /* Set video chip power */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_VIDEOLOWPOWER,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_VIDEOLOWPOWER,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_CAMERAPOWER:
         /* Get camera device power */
         /*                         */
         BuildMsg(mCarr, MSGCARR_GET_CAMERAOFF);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_CAMERAPOWER:
         /* Set camera device power */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CAMERAOFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CAMERAOFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_VIDEODIGITIZERPOWER:
         /* Get video digitizer device power */
         /*                                  */
         BuildMsg(mCarr, MSGCARR_GET_VIDEODIGITIZEROFF);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_VIDEODIGITIZERPOWER:
         /* Set video digitizer device power */
         /*                                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_VIDEODIGITIZEROFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_VIDEODIGITIZEROFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_SCSICLOCKPOWER:
         /* Get SCSI clock power */
         /*                      */
         BuildMsg(mCarr, MSGCARR_GET_SCSICLOCKSTOP);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_SCSICLOCKPOWER:
         /* Set SCSI clock power */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_SCSICLOCKSTOP,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_SCSICLOCKSTOP,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_CDROMDRIVEPOWER:
         /* Get CD-ROM drive power */
         /*                        */
         BuildMsg(mCarr, MSGCARR_GET_CDROMDRIVELOWPOWER);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_CDROMDRIVEPOWER:
         /* Set CD-ROM drive power */
         /*                        */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CDROMDRIVELOWPOWER,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CDROMDRIVELOWPOWER,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_AUDIOPOWER:
         /* Get audio device power */
         /*                        */
         BuildMsg(mCarr, MSGCARR_GET_AUDIOLOWPOWER);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_AUDIOPOWER:
         /* Set audio device power */
         /*                        */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_AUDIOLOWPOWER,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_AUDIOLOWPOWER,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_L2CACHEPOWER:
         /* Get L2-cache device power */
         /*                           */
         BuildMsg(mCarr, MSGCARR_GET_L2CACHELOWPOWER);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_L2CACHEPOWER:
         /* Set L2-cache device power */
         /*                           */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_POWERON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_L2CACHELOWPOWER,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_POWEROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_L2CACHELOWPOWER,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_LCDBRIGHTNESS:
         /* Get LCD brightness power */
         /*                          */
         BuildMsg(mCarr, MSGCARR_GET_LCDHALFBRIGHTNESS);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_FULLON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_HALFON;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_LCDBRIGHTNESS:
         /* Set LCD brightness power */
         /*                          */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_FULLON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDHALFBRIGHTNESS,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_HALFON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDHALFBRIGHTNESS,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_LCDSTATUS:
         /* Get LCD status */
         /*                */
         BuildMsg(mCarr, MSGCARR_GET_LCDOFF);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_LINEHIGH:
                  SelectParm1(*pmMsg) = PRMPWR_POWERON;
                  break;
               case PRMCARR_LINELOW:
                  SelectParm1(*pmMsg) = PRMPWR_POWEROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_LCDSTATUS:
         /* Set LCD status */
         /*                */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_LCDENABLE:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDOFF,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDENABLE,
                                        PRMCARR_LINEHIGH);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_LCDDISABLE:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDENABLE,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               BuildMsgWithParm1(mCarr, MSGCARR_SET_LCDOFF,
                                        PRMCARR_LINELOW);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_SCSITERMPOWER:
         /* Get SCSI terminator power */
         /*                           */
         BuildMsg(mCarr, MSGCARR_GET_SCSITERMLOWPOWER);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_TERMINATORON:
                  SelectParm1(*pmMsg) = PRMPWR_TERMINATORON;
                  break;
               case PRMCARR_TERMINATORSTANDBY:
                  SelectParm1(*pmMsg) = PRMPWR_TERMINATORSTANDBY;
                  break;
               case PRMCARR_TERMINATOROFF:
                  SelectParm1(*pmMsg) = PRMPWR_TERMINATOROFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_SCSITERMPOWER:
         /* Set SCSI terminator power */
         /*                           */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_TERMINATORON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_SCSITERMLOWPOWER,
                                        PRMCARR_TERMINATORON);

               SendMsg(mCarr, oCarreraCtl);
               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_TERMINATORSTANDBY:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_SCSITERMLOWPOWER,
                                        PRMCARR_TERMINATORSTANDBY);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_TERMINATOROFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_SCSITERMLOWPOWER,
                                        PRMCARR_TERMINATOROFF);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGPWR_GET_CRTPOWER:
         /* Get CRT device power */
         /*                      */
         BuildMsg(mCarr, MSGCARR_GET_CRTLOWPOWER);
         SendMsg(mCarr, oCarreraCtl);

         if(!(rc = SelectResult(mCarr)))
         {
            switch(SelectParm1(mCarr))
            {
               case PRMCARR_CRTPOWERON:
                  SelectParm1(*pmMsg) = PRMPWR_CRTON;
                  break;
               case PRMCARR_CRTPOWERSTANDBY:
                  SelectParm1(*pmMsg) = PRMPWR_CRTSTANDBY;
                  break;
               case PRMCARR_CRTPOWERSUSPEND:
                  SelectParm1(*pmMsg) = PRMPWR_CRTSUSPEND;
                  break;
               case PRMCARR_CRTPOWEROFF:
                  SelectParm1(*pmMsg) = PRMPWR_CRTOFF;
                  break;
               default:
                  rc = ERRPWR_CANNOT_POWERCONTROL;
            } /* endswitch */
         }
         else
         {
            rc = ERRPWR_CANNOT_POWERCONTROL;
         } /* endif */
         break;

      case MSGPWR_SET_CRTPOWER:
         /* Set CRT device power */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMPWR_CRTON:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CRTLOWPOWER,
                                        PRMCARR_CRTPOWERON);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_CRTSTANDBY:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CRTLOWPOWER,
                                        PRMCARR_CRTPOWERSTANDBY);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_CRTSUSPEND:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CRTLOWPOWER,
                                        PRMCARR_CRTPOWERSUSPEND);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            case PRMPWR_CRTOFF:
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CRTLOWPOWER,
                                        PRMCARR_CRTPOWEROFF);
               SendMsg(mCarr, oCarreraCtl);

               if((rc = SelectResult(mCarr)) != ERRCARR_NOERROR)
               {
                  rc = ERRPWR_CANNOT_POWERCONTROL;
               } /* endif */
               break;
            default:
               rc = ERRPWR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      default:
         rc = ERRPWR_INVALID_MESSAGE;
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
