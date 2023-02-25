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
/* *              PDPIC.H/PDPIT.H/PDDMAC.H/PDCOOPER.H/PDPOS.H/PDNVRAM.H/   * */
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
#include "pdpcibus.h"
#include "pdsio.h"
#include "pdpic.h"
#include "pdpit.h"
#include "pdcooper.h"
#include "pdpos.h"
#include "pdeeprom.h"
#include "pdrtc.h"
#include "pdumcu.h"
#include "pdkmc.h"
#include "pdmouse.h"
#include "pdsuprio.h"
#include "pdserial.h"
#include "pdcarera.h"
#include "pdmemctl.h"
#include "pdv7310.h"
#include "pdwdc24.h"


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
   MSGPCI      mPci;
   MSGMEMORY   mMem;
   MSGSYSTEMIO mSio;
   MSGCOOPER   mCoop;
   MSGPOS      mPos;
   MSGEEPROM   mEeprom;
   MSGRTC      mRtc;
   MSGUMCU     mUmcu;
   MSGKMC      mKmc;
   MSGMOUSE    mMous;
   MSGSUPERIO  mSpio;
   MSGCARRERA  mCarr;
   MSGSERIAL   mSer;
   MSGPIC      mPic;
   MSGPIT      mPit;
   MSGV7310    mV73;
   MSGWD90C24  mC24;

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
   BuildMsg(mPci, MSGPCI_INITIALIZECTL);
   SendMsg(mPci, oPciIdahoCtl);
   SendMsg(mPci, oPciEagleCtl);
   SendMsg(mPci, oPciSystemioCtl);
   SendMsg(mPci, oPciScsiCtl);
   SendMsg(mPci, oPciWd90c24Ctl);
   SendMsg(mPci, oPciS386c928Ctl);
   SendMsg(mPci, oPciS386c864Ctl);
   SendMsg(mPci, oPciWeitekP9100Ctl);
   SendMsg(mPci, oPciCarreraCtl);
   SendMsg(mPci, oPciCaptureCtl);

   BuildMsg(mMem, MSGMEM_INITIALIZECTL);
   SendMsg(mMem, oMemoryCtl);

   BuildMsg(mSio, MSGSIO_INITIALIZECTL);
   SendMsg(mSio, oSystemioCtl);

   BuildMsg(mCoop, MSGCOOP_INITIALIZECTL);
   SendMsg(mCoop, oCooperCtl);

   BuildMsg(mPos, MSGPOS_INITIALIZECTL);
   SendMsg(mPos, oPosCtl);

   BuildMsg(mEeprom, MSGEEPROM_INITIALIZECTL);
   SendMsg(mEeprom, oEepromCtl);

   BuildMsg(mRtc, MSGRTC_INITIALIZECTL);
   SendMsg(mRtc, oRtcCtl);

   BuildMsg(mUmcu, MSGUMCU_INITIALIZECTL);
   SendMsg(mUmcu, oUmcuCtl);

   BuildMsg(mKmc, MSGKMC_INITIALIZECTL);
   SendMsg(mKmc, oKmcCtl);

   BuildMsg(mMous, MSGMOUS_INITIALIZECTL);
   SendMsg(mMous, oMouseCtl);

   BuildMsg(mSpio, MSGSPIO_INITIALIZECTL);
   SendMsg(mSpio, oSuperioCtl);

   BuildMsg(mCarr, MSGCARR_INITIALIZECTL);
   SendMsg(mCarr, oCarreraCtl);

   BuildMsg(mSer, MSGSER_INITIALIZECTL);
   SendMsg(mSer, oSerialACtl);
   SendMsg(mSer, oSerialBCtl);

   BuildMsg(mPic, MSGPIC_INITIALIZECTL);
   SendMsg(mPic, oPic1Ctl);
   SendMsg(mPic, oPic2Ctl);

   BuildMsg(mPit, MSGPIT_INITIALIZECTL);
   SendMsg(mPit, oPitCtl);

   BuildMsg(mV73, MSGV73_INITIALIZECTL);
   SendMsg(mV73, oV7310Ctl);

   BuildMsg(mC24, MSGC24_INITIALIZECTL);
   SendMsg(mC24, oWd90c24Ctl);

   /* EnableInterrupt(); */

   /* Restore interrupt status */
   /*                          */
   /* if(!InterruptStatus)   */
   /* {                      */
   /*    DisableInterrupt(); */
   /* } */ /* endif */
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
   MSGSERIAL   mSer;
   MSGSUPERIO  mSpio;
   MSGUMCU     mUmcu;
   MSGRTC      mRtc;
   MSGEEPROM   mEeprom;
   MSGPOS      mPos;
   MSGCOOPER   mCoop;
   MSGPIT      mPit;
   MSGSYSTEMIO mSio;
   MSGMEMORY   mMem;
   MSGCARRERA  mCarr;
   MSGPCI      mPci;
   MSGV7310    mV73;
   MSGWD90C24  mC24;

   DebugCheckPoint(0x80);

   /* Save interrupt status */
   /*   & disable interrupt */
   /*                       */
   InterruptStatus = ReadInterruptStatus();
   DisableInterrupt();
   DebugCheckPoint(0x81);

   /* Save video chip contexts */
   /*                          */
   BuildMsg(mV73, MSGV73_SAVE_CONTEXT);
   SendMsg(mV73, oV7310Ctl);

   BuildMsg(mC24, MSGC24_SAVE_CONTEXT);
   SendMsg(mC24, oWd90c24Ctl);

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
   BuildMsg(mSer, MSGSER_SAVE_CONTEXT);
   SendMsg(mSer, oSerialACtl);
   SendMsg(mSer, oSerialBCtl);
   DebugCheckPoint(0x87);

   BuildMsg(mSpio, MSGSPIO_SAVE_CONTEXT);
   SendMsg(mSpio, oSuperioCtl);
   DebugCheckPoint(0x88);

   BuildMsg(mUmcu, MSGUMCU_SAVE_CONTEXT);
   SendMsg(mUmcu, oUmcuCtl);
   DebugCheckPoint(0x89);

   BuildMsg(mRtc, MSGRTC_SAVE_CONTEXT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0x8A);

   BuildMsg(mEeprom, MSGEEPROM_SAVE_CONTEXT);
   SendMsg(mEeprom, oEepromCtl);
   DebugCheckPoint(0x8B);

   BuildMsg(mPos, MSGPOS_SAVE_CONTEXT);
   SendMsg(mPos, oPosCtl);
   DebugCheckPoint(0x8D);

   BuildMsg(mCoop, MSGCOOP_SAVE_CONTEXT);
   SendMsg(mCoop, oCooperCtl);
   DebugCheckPoint(0x8E);

   BuildMsg(mPit, MSGPIT_SAVE_CONTEXT);
   SendMsg(mPit, oPitCtl);
   DebugCheckPoint(0x8F);

   BuildMsg(mSio, MSGSIO_SAVE_CONTEXT);
   SendMsg(mSio, oSystemioCtl);
   DebugCheckPoint(0x92);

   BuildMsg(mMem, MSGMEM_SAVE_CONTEXT);
   SendMsg(mMem, oMemoryCtl);
   DebugCheckPoint(0x93);

   BuildMsg(mCarr, MSGCARR_SAVE_CONTEXT);
   SendMsg(mCarr, oCarreraCtl);
   DebugCheckPoint(0x94);

   BuildMsg(mPci, MSGPCI_SAVE_CONTEXT);
   SendMsg(mPci, oPciCaptureCtl);
   SendMsg(mPci, oPciCarreraCtl);
   SendMsg(mPci, oPciWd90c24Ctl);
   SendMsg(mPci, oPciS386c928Ctl);
   SendMsg(mPci, oPciS386c864Ctl);
   SendMsg(mPci, oPciWeitekP9100Ctl);
   SendMsg(mPci, oPciScsiCtl);
   SendMsg(mPci, oPciSystemioCtl);
   SendMsg(mPci, oPciEagleCtl);
   SendMsg(mPci, oPciIdahoCtl);
   DebugCheckPoint(0x95);

   /* Inhibit PMI sources */
   /*                     */
   BuildMsg(mCarr, MSGCARR_INHIBIT_PMI);
   SendMsg(mCarr, oCarreraCtl);
   DebugCheckPoint(0x96);

   /* Set alarm condition in suspend state */
   /*                                      */
   BuildMsg(mCarr, MSGCARR_GET_ALARMSTATUS);
   SendMsg(mCarr, oCarreraCtl);
   DebugCheckPoint(0x97);

   if((pAlarm != NULL) && (SelectParm1(mCarr) == PRMCARR_ALARMENABLE))
   {
      /* Alarm resume enable */
      BuildMsgWithParm6(mRtc, MSGRTC_SET_ALARMDATETIME,
                              pAlarm->chSeconds,
                              pAlarm->chMinutes,
                              pAlarm->chHours,
                              pAlarm->chDay,
                              pAlarm->chMonth,
                              pAlarm->wYear);
      SendMsg(mRtc, oRtcCtl);

      BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARMSTATUS,
                              PRMRTC_ALARMENABLE);
      SendMsg(mRtc, oRtcCtl);

      BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARM,
                              PRMRTC_ALARMOFF);
      SendMsg(mRtc, oRtcCtl);
   }
   else
   {
      /* Alarm resume disable */
      BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARMSTATUS,
                              PRMRTC_ALARMDISABLE);
      SendMsg(mRtc, oRtcCtl);
   } /* endif */
   DebugCheckPoint(0x98);

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
   MSGPCI      mPci;
   MSGMEMORY   mMem;
   MSGCARRERA  mCarr;
   MSGSYSTEMIO mSio;
   MSGPIT      mPit;
   MSGCOOPER   mCoop;
   MSGPOS      mPos;
   MSGEEPROM   mEeprom;
   MSGRTC      mRtc;
   MSGUMCU     mUmcu;
   MSGKMC      mKmc;
   MSGMOUSE    mMous;
   MSGSUPERIO  mSpio;
   MSGSERIAL   mSer;
   MSGPIC      mPic;
   MSGV7310    mV73;
   MSGWD90C24  mC24;

   DebugCheckPoint(0xB0);

   /* Save interrupt status */
   /*   & disable interrupt */
   /*                       */
   InterruptStatus = ReadInterruptStatus();
   DisableInterrupt();
   DebugCheckPoint(0xB1);

   /* Restore PCI/Memory Controller/                 */
   /*   Power management Controller devices contexts */
   /*                                                */
   BuildMsg(mPci, MSGPCI_RESTORE_CONTEXT);
   SendMsg(mPci, oPciIdahoCtl);
   SendMsg(mPci, oPciEagleCtl);
   SendMsg(mPci, oPciSystemioCtl);
   SendMsg(mPci, oPciScsiCtl);
   SendMsg(mPci, oPciWd90c24Ctl);
   SendMsg(mPci, oPciS386c928Ctl);
   SendMsg(mPci, oPciS386c864Ctl);
   SendMsg(mPci, oPciWeitekP9100Ctl);
   SendMsg(mPci, oPciCarreraCtl);
   SendMsg(mPci, oPciCaptureCtl);
   DebugCheckPoint(0xB2);

   BuildMsg(mMem, MSGMEM_RESTORE_CONTEXT);
   SendMsg(mMem, oMemoryCtl);
   DebugCheckPoint(0xB3);

   BuildMsg(mCarr, MSGCARR_RESTORE_CONTEXT);
   SendMsg(mCarr, oCarreraCtl);
   DebugCheckPoint(0xB4);

   /* Restore all device contexts */
   /*                             */
   BuildMsg(mSio, MSGSIO_RESTORE_CONTEXT);
   SendMsg(mSio, oSystemioCtl);
   DebugCheckPoint(0xB6);

   BuildMsg(mPit, MSGPIT_RESTORE_CONTEXT);
   SendMsg(mPit, oPitCtl);
   DebugCheckPoint(0xB7);

   BuildMsg(mCoop, MSGCOOP_RESTORE_CONTEXT);
   SendMsg(mCoop, oCooperCtl);
   DebugCheckPoint(0xB9);

   BuildMsg(mPos, MSGPOS_RESTORE_CONTEXT);
   SendMsg(mPos, oPosCtl);
   DebugCheckPoint(0xBA);

   BuildMsg(mEeprom, MSGEEPROM_RESTORE_CONTEXT);
   SendMsg(mEeprom, oEepromCtl);
   DebugCheckPoint(0xBC);

   BuildMsg(mRtc, MSGRTC_RESTORE_CONTEXT);
   SendMsg(mRtc, oRtcCtl);
   DebugCheckPoint(0xBD);

   BuildMsg(mUmcu, MSGUMCU_RESTORE_CONTEXT);
   SendMsg(mUmcu, oUmcuCtl);
   DebugCheckPoint(0xBE);

   BuildMsg(mSpio, MSGSPIO_RESTORE_CONTEXT);
   SendMsg(mSpio, oSuperioCtl);
   DebugCheckPoint(0xBF);

   BuildMsg(mSer, MSGSER_RESTORE_CONTEXT);
   SendMsg(mSer, oSerialACtl);
   SendMsg(mSer, oSerialBCtl);
   DebugCheckPoint(0xC0);

   /* Restore PIC context */
   /*                     */
   BuildMsg(mPic, MSGPIC_RESTORE_CONTEXT);
   SendMsg(mPic, oPic1Ctl);
   SendMsg(mPic, oPic2Ctl);
   DebugCheckPoint(0xC1);

   /* Wait for UMCU ready */
   /*                     */
   WaitTimeCheck(WAITTIME_PMCBUSY, UmcuQueryBusy);
   DebugCheckPoint(0xC2);

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

   /* Restore video chip contexts */
   /*                             */
   BuildMsg(mV73, MSGV73_RESTORE_CONTEXT);
   SendMsg(mV73, oV7310Ctl);

   BuildMsg(mC24, MSGC24_RESTORE_CONTEXT);
   SendMsg(mC24, oWd90c24Ctl);

   /* Restore interrupt status */
   /*                          */
   if(InterruptStatus)
   {
      EnableInterrupt();
   } /* endif */
   DebugCheckPoint(0xC7);

   return;
}


/* *****************************  End of File  ***************************** */
