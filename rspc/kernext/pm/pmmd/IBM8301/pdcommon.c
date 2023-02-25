static char sccsid[] = "@(#)09	1.3  src/rspc/kernext/pm/pmmd/IBM8301/pdcommon.c, pwrsysx, rspc41J, 9516A_all 4/18/95 08:41:22";
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

/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *      Wait Time Control Data       * */
/* ************************************* */

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

	/*
	 *  Use io_delay(usec) service.  If >= 1 second, must loop.
	 */
	while (wTimeValue > 0) {
		if (wTimeValue < 1000) {
			io_delay(wTimeValue * 1000);
			break;
		} else {
			io_delay(999000);
			wTimeValue -= 999;
		}
	}

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
	int rc;

	/*
	 *  Use io_delay(usec) service.  If >= 1 second, must loop.
	 */

	rc = (*pfnCheckRoutine)();

	while ((wTimeValue > 0) && rc) {
		if (wTimeValue < 1000) {
			io_delay(wTimeValue * 1000);
			wTimeValue = 0;
		} else {
			io_delay(999000);
			wTimeValue -= 999;
		}
		rc = (*pfnCheckRoutine)();
	}

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

   } /* endif */

   return;
}


/* *****************************  End of File  ***************************** */
