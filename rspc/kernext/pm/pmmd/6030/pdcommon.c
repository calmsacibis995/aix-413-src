/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Miscellaneous Common Routines
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
/* * Module Name: PDCOMMON.C                                               * */
/* * Description: Miscellaneous Common Routines      (for PowerPC 601/603) * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDRTC.H files should be included in* */
/* *              this file                                                * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                             Memo Section                              * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdrtc.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *   Wait Time Control Definition    * */
/* ************************************* */
/* ------------------ */
/*  Time-base timing  */
/* ------------------ */
#define  TIMEBASE_BUSSPEED                CLOCK_CPUBUSSPEED
                                                      /* Processor-bus speed */
#define  TIMEBASE_FREQUENCY               (4000 / TIMEBASE_BUSSPEED)
                                     /* Time-base frequency (4000/bus-speed) */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *      Wait Time Control Data       * */
/* ************************************* */
/* --------------------- */
/*  Time-base frequency  */
/* --------------------- */
UINT  TimebaseFrequencyTime = TIMEBASE_FREQUENCY;


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ****************************************** */
/* *               Wait Time                * */
/* ****************************************** */
/* * (using real-time clock lower value)    * */
/* *                                        * */
/* * [ Input  ] wait time value (msec unit) * */
/* * [ Output ] none                        * */
/* *                                        * */
/* ****************************************** */
VOID  WaitTime(WORD  wTimeValue)              /* Wait time value (msec unit) */
{
   UINT  rtcl_curr, rtcl_prev, rtcl, timer, delta, ver;

   /* Get processor version */
   /*                       */
   ver = ReadCpuVersion();

   /* Wait for specified time by using the real-time clock lower register */
   /*                                                                     */
   for(rtcl_prev = ReadRtcl(), timer = 0; wTimeValue; rtcl_prev = rtcl_curr)
   {
      rtcl_curr = rtcl = ReadRtcl();

      if(ver == CPUVERSION_MPC601)
      {
         if(rtcl_curr < rtcl_prev)
         {
            rtcl += 1000000000;
         } /* endif */

         delta = rtcl - rtcl_prev;
      }
      else
      {
         delta = (rtcl - rtcl_prev) * TimebaseFrequencyTime;
      } /* endif */

      if((timer += delta) >= 1000000)
      {
         timer -= 1000000;
         wTimeValue--;
      } /* endif */
   } /* endfor */

   return;
}

/* **************************************************** */
/* *           Wait Time with Check Routine           * */
/* **************************************************** */
/* * (using real-time clock lower value)              * */
/* *                                                  * */
/* * [ Input  ] wait time value (msec unit)           * */
/* *            check routine returning CRESULT value * */
/* * [ Output ] result                                * */
/* *                                                  * */
/* **************************************************** */
CRESULT  WaitTimeCheck(WORD      wTimeValue,  /* Wait time value (msec unit) */
                       PCHKFUNC  pfnCheckRoutine)
                                             /* Pointer to the check routine */
                                                  /* returning CRESULT value */
{
   CRESULT  rc;
   UINT     rtcl_curr, rtcl_prev, rtcl, timer, delta, ver;

   /* Get processor version */
   /*                       */
   ver = ReadCpuVersion();

   /* Wait for specified time by using the real-time clock lower register */
   /*                                                                     */
   rc = (*pfnCheckRoutine)();

   for(rtcl_prev = ReadRtcl(), timer = 0; wTimeValue && rc;
       rtcl_prev = rtcl_curr                               )
   {
      rtcl_curr = rtcl = ReadRtcl();

      if(ver == CPUVERSION_MPC601)
      {
         if(rtcl_curr < rtcl_prev)
         {
            rtcl += 1000000000;
         } /* endif */

         delta = rtcl - rtcl_prev;
      }
      else
      {
         delta = (rtcl - rtcl_prev) * TimebaseFrequencyTime;
      } /* endif */

      if((timer += delta) >= 1000000)
      {
         timer -= 1000000;
         wTimeValue--;
      } /* endif */

      rc = (*pfnCheckRoutine)();
   } /* endfor */

   return(rc);
}

/* *************************************** */
/* *     Timer Frequency Calibration     * */
/* *************************************** */
/* * (using real-time clock lower value) * */
/* *                                     * */
/* * [ Input  ] none                     * */
/* * [ Output ] none                     * */
/* *                                     * */
/* *************************************** */
VOID  CalibrateTime(VOID)
{
   MSGRTC   mRtc;
   UINT     rtcl_curr, rtcl_prev;
   WORD     seconds1, seconds2;

   /* Check processor version */
   /*                         */
   if(ReadCpuVersion() != CPUVERSION_MPC601)
   {
      BuildMsg(mRtc, MSGRTC_GET_DATETIME);
      SendMsg(mRtc, oRtcCtl);
      seconds1 = seconds2 = ConvertToDec(SelectParm1(mRtc));

      while((seconds2 - seconds1) == 0)
      {
         BuildMsg(mRtc, MSGRTC_GET_DATETIME);
         SendMsg(mRtc, oRtcCtl);
         seconds2 = ConvertToDec(SelectParm1(mRtc));
      } /* endfor */

      seconds1  = seconds2;
      rtcl_prev = ReadRtcl();

      while((seconds2 - seconds1) == 0)
      {
         BuildMsg(mRtc, MSGRTC_GET_DATETIME);
         SendMsg(mRtc, oRtcCtl);
         seconds2 = ConvertToDec(SelectParm1(mRtc));
      } /* endfor */

      rtcl_curr = ReadRtcl();

      TimebaseFrequencyTime = 1000000000 / (rtcl_curr - rtcl_prev);
   } /* endif */

   return;
}


/* *****************************  End of File  ***************************** */
