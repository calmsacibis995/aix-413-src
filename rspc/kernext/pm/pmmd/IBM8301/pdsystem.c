static char sccsid[] = "@(#)26	1.4  src/rspc/kernext/pm/pmmd/IBM8301/pdsystem.c, pwrsysx, rspc41J, 9516A_all 4/18/95 07:49:14";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: System Power State Control Routines
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
/* * Module Name: PDSYSTEM.C                                               * */
/* * Description: System Power State Control Routines                      * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDSYSTEM.H/PDPCIBUS.H/PDSIO.H/     * */
/* *              PDPIC.H/PDDMAC.H/PDCOOPER.H/PDPOS.H/PDNVRAM.H/           * */
/* *              PDEEPROM.H/PDRTC.H/PDUMCU.H/PDKMC.H/PDMOUSE.H/PDSUPRIO.H * */
/* *              PDSERIAL.H/PDCARERA.H/PDMEMCTL.H/PDCPU.H/PDPOWER.H/      * */
/* *              /PDV7310.H/PDWDC24.H files should be included in this    * */
/* *              file                                                     * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdsystem.h"
#include "pdpic.h"
#include "pdrtc.h"
#include "pdkmc.h"
#include "pdmouse.h"

extern void init_signetics();

/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      System Data Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  WAITTIME_PMCBUSY                 (5000)
                                         /* Busy time-out wait time for UMCU */
                                         /* (5000 msec = 5 sec)              */
#define  CONTROLFLAG_INPUTDEVICE          (FALSE)
                               /* Input device (keyboard/mouse) control flag */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *        System Control Data        * */
/* ************************************* */
/* ------------------- */
/*  Interrupt control  */
/* ------------------- */
UINT  InterruptStatus;                       /* Interrupt status (0=disable) */


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *     System Initialize Control     * */
/* ************************************* */
/* *                                   * */
/* * [ Input  ] none                   * */
/* * [ Output ] none                   * */
/* *                                   * */
/* ************************************* */
VOID  PmSystemInitialize(VOID)
{
   MSGRTC      mRtc;
   MSGKMC      mKmc;
   MSGMOUSE    mMous;
   MSGPIC      mPic;

   /* Save interrupt status */
   /*   & disable interrupt */
   /*                       */
   InterruptStatus = ReadInterruptStatus();
   DisableInterrupt();

   /* Calibrate timer frequency */
   /*   after RTC initializing  */
   BuildMsg(mRtc, MSGRTC_INITIALIZECTL);
   SendMsg(mRtc, oRtcCtl);

   CalibrateTime();

   /* Initialize all device objects */
   /*                               */

   BuildMsg(mRtc, MSGRTC_INITIALIZECTL);
   SendMsg(mRtc, oRtcCtl);

   BuildMsg(mKmc, MSGKMC_INITIALIZECTL);
   SendMsg(mKmc, oKmcCtl);

   BuildMsg(mMous, MSGMOUS_INITIALIZECTL);
   SendMsg(mMous, oMouseCtl);

   BuildMsg(mPic, MSGPIC_INITIALIZECTL);
   SendMsg(mPic, oPic1Ctl);
   SendMsg(mPic, oPic2Ctl);

   if(InterruptStatus)
   {
      EnableInterrupt();
   } /* endif */

   return;
}

/* ************************************* */
/* *      System Suspend Control       * */
/* ************************************* */
/* *                                   * */
/* * [ Input  ] Alarm date/time        * */
/* * [ Output ] none                   * */
/* *                                   * */
/* ************************************* */
VOID  PmSystemSuspend(PALARMDATETIME   pAlarm)
                                     /* Pointer to Alarm date/time structure */
{
   MSGPIC      mPic;
   MSGMOUSE    mMous;
   MSGKMC      mKmc;
   MSGRTC      mRtc;

   DebugCheckPoint(0x80);

   /* Save interrupt status */
   /*   & disable interrupt */
   /*                       */
   InterruptStatus = ReadInterruptStatus();
   DisableInterrupt();
   DebugCheckPoint(0x81);

   /* Save PIC context          */
   /*   & disable all interrupt */
   /*                           */
   BuildMsg(mPic, MSGPIC_SAVE_CONTEXT);
   SendMsg(mPic, oPic1Ctl);
   SendMsg(mPic, oPic2Ctl);
   DebugCheckPoint(0x82);

   BuildMsg(mPic, MSGPIC_DISABLE_INTERRUPT);
   SendMsg(mPic, oPic1Ctl);
   SendMsg(mPic, oPic2Ctl);
   DebugCheckPoint(0x83);

   /* Save input devices context */
   /*   & disable devices        */
   /*                            */
#if (CONTROLFLAG_INPUTDEVICE == TRUE)
   BuildMsg(mMous, MSGMOUS_SAVE_CONTEXT);
   SendMsg(mMous, oMouseCtl);
   DebugCheckPoint(0x84);
#endif /* CONTROLFLAG_INPUTDEVICE */

   BuildMsg(mMous, MSGMOUS_DISABLE_DEVICE);
   SendMsg(mMous, oMouseCtl);
   DebugCheckPoint(0x85);

   BuildMsg(mKmc, MSGKMC_SAVE_CONTEXT);
   SendMsg(mKmc, oKmcCtl);
   DebugCheckPoint(0x86);

   /* Save all device contexts */
   /*                          */
   BuildMsg(mRtc, MSGRTC_SAVE_CONTEXT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0x8A);

   /* Clean-up RTC */
   BuildMsg(mRtc, MSGRTC_DISABLE_INTERRUPT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0x99);

   BuildMsg(mRtc, MSGRTC_CLEAR_INTERRUPT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0x9A);

   /* Restore interrupt status */
   /*                          */
   if(InterruptStatus)
   {
      EnableInterrupt();
   } /* endif */
   DebugCheckPoint(0x9B);

   return;
}

/* ************************************* */
/* *       System Resume Control       * */
/* ************************************* */
/* *                                   * */
/* * [ Input  ] none                   * */
/* * [ Output ] none                   * */
/* *                                   * */
/* ************************************* */
VOID  PmSystemResume(VOID)
{
   MSGRTC      mRtc;
   MSGKMC      mKmc;
   MSGMOUSE    mMous;
   MSGPIC      mPic;

   DebugCheckPoint(0xB0);

   /* Save interrupt status */
   /*   & disable interrupt */
   /*                       */
   InterruptStatus = ReadInterruptStatus();
   DisableInterrupt();
   DebugCheckPoint(0xB1);

   /* Restore all device contexts */
   /*                             */
   BuildMsg(mRtc, MSGRTC_RESTORE_CONTEXT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0xBD);

   /* Restore PIC context */
   /*                     */
   BuildMsg(mPic, MSGPIC_RESTORE_CONTEXT);
   SendMsg(mPic, oPic1Ctl);
   SendMsg(mPic, oPic2Ctl);
   DebugCheckPoint(0xC1);

   /* Restore input devices context */
   /*                               */
   BuildMsg(mKmc, MSGKMC_RESTORE_CONTEXT);
   SendMsg(mKmc, oKmcCtl);
   DebugCheckPoint(0xC3);

   BuildMsg(mMous, MSGMOUS_RESET_DEVICE);
   SendMsg(mMous, oMouseCtl);
   if(!SelectResult(mMous) && (SelectParm1(mMous) == PRMMOUS_BATOK))
   {
#if (CONTROLFLAG_INPUTDEVICE == TRUE)
      BuildMsg(mMous, MSGMOUS_RESTORE_CONTEXT);
      SendMsg(mMous, oMouseCtl);
#endif /* CONTROLFLAG_INPUTDEVICE */
   } /* endif */
   DebugCheckPoint(0xC4);

   /* Reset alarm condition */
   /*                       */
   BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARMSTATUS,
                           PRMRTC_ALARMDISABLE);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0xC5);

   BuildMsg(mRtc, MSGRTC_CLEAR_INTERRUPT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0xC6);

   /* Restore interrupt status */
   /*                          */
   if(InterruptStatus)
   {
      EnableInterrupt();
   } /* endif */
   DebugCheckPoint(0xC7);

   init_signetics();

   return;
}


/* *****************************  End of File  ***************************** */
