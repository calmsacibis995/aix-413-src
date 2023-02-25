static char sccsid[] = "@(#)47  1.2  src/rspc/kernext/pm/pmmd/6030/pdcpu.c, pwrsysx, rspc41J, 9517A_b 4/25/95 04:16:11";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: CPU (MPC603) Chip Device Control Routines
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
/* * Module Name: PDCPU.C                                                  * */
/* * Description: CPU (MPC603) Chip Device Control Routines                * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDCPU.H/PDCARERA.H/PDCOOPER.H files* */
/* *              should be included in this file                          * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdcpu.h"
#include "pdcarera.h"
#include "pdcooper.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *        CPU Data Definition        * */
/* ************************************* */
/* ------------------------- */
/*  PLL configuration table  */
/* ------------------------- */
typedef struct                                          /* PLL configuration */
{
   WORD  BusClock;                                        /* Bus clock (MHz) */
   BYTE  PllCfg11;                   /* PLL_CFG 3-0 for CPU/SYSCLK ratio 1:1 */
   BYTE  PllCfg21;                   /* PLL_CFG 3-0 for CPU/SYSCLK ratio 2:1 */
   BYTE  PllCfg31;                   /* PLL_CFG 3-0 for CPU/SYSCLK ratio 3:1 */
   BYTE  PllCfg41;                   /* PLL_CFG 3-0 for CPU/SYSCLK ratio 4:1 */
} PLL_CONFIGURATION;
typedef PLL_CONFIGURATION  *PPLL_CONFIGURATION;

PLL_CONFIGURATION PllConfigurationTable[] =
                  {
                     {16, 0x04, 0x0A, 0x09, 0x03},               /* 16.6 MHz */
                     {20, 0x04, 0x0A, 0xFF, 0x03},               /* 20   MHz */
                     {25, 0x04, 0x0A, 0x01, 0x03},               /* 25   MHz */
                     {33, 0x08, 0x02, 0x01, 0xFF},               /* 33.3 MHz */
                     {40, 0x08, 0x02, 0xFF, 0xFF},               /* 40   MHz */
                     {50, 0x08, 0x02, 0xFF, 0xFF},               /* 50   MHz */
                     {66, 0x00, 0xFF, 0xFF, 0xFF},               /* 66.6 MHz */
                     {0}                                       /* Terminator */
                  };

/* --------------------- */
/*  Time-base frequency  */
/* --------------------- */
extern UINT TimebaseFrequencyTime;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     CPU Controller Definition     * */
/* ************************************* */
/* ------------------- */
/*  CPU control class  */
/* ------------------- */
VOID  CpuCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            CPU Objects            * */
/* ************************************* */
/* ------------------- */
/*  CPU control class  */
/* ------------------- */
OBJCPUCTL   oCpuCtl = {(POBJCTL)CpuCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************** */
/* *             CPU Control Controller             * */
/* ************************************************** */
/* *                                                * */
/* * - Initialize                                   * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_INITIALIZECTL        * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Set low-power mode                           * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_SET_DOZE             * */
/* *                    MSGCPU_SET_NAP              * */
/* *                    MSGCPU_SET_SLEEP            * */
/* *                    MSGCPU_SET_SLEEPANDSUSPEND  * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Set/Reset dynamic power management           * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_SET_DPM              * */
/* *                    MSGCPU_RESET_DPM            * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Set clock rate                               * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_SET_CLOCKX1          * */
/* *                    MSGCPU_SET_CLOCKX2          * */
/* *                    MSGCPU_SET_CLOCKX3          * */
/* *                    MSGCPU_SET_CLOCKX4          * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Change clock rate                            * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_CHANGE_CLOCKRATE     * */
/* *       Parameter1 = clock rate                  * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* * - Get PLL-configuration                        * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_GET_PLLCONFIGURATION * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *       Parameter1 = 1:1 PLL_CFG                 * */
/* *       Parameter2 = 2:1 PLL_CFG                 * */
/* *       Parameter3 = 3:1 PLL_CFG                 * */
/* *       Parameter4 = 4:1 PLL_CFG                 * */
/* *       Parameter5 = bus clock                   * */
/* *                                                * */
/* * - Get CPU clock                                * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_GET_CPUCLOCK         * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *       Parameter1 = external clock              * */
/* *       Parameter2 = internal clock              * */
/* *       Parameter3 = maximum clock               * */
/* *                                                * */
/* * - Flush L1 cache                               * */
/* *     [ Input ]                                  * */
/* *       Message    = MSGCPU_FLUSH_L1CACHE        * */
/* *     [ Output ]                                 * */
/* *       Result     = result code                 * */
/* *                                                * */
/* ************************************************** */
VOID  CpuCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGCARRERA           mCarr;
   MSGCOOPER            mCoop;
   MSGCPU               msg;
   PPLL_CONFIGURATION   plltable;
   WORD                 clock;
   PCPARAMETER          pparm;
   WORD                 i;
   CPARAMETER           l2status;
   CRESULT              rc;
   POBJCPUCTL           pcpuctl;

   /* Get pointer to CPU control object */
   /*                                   */
   pcpuctl = (POBJCPUCTL)poObj;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGCPU_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         SetHid0Power(0);

         BuildMsg(msg, MSGCPU_SET_DPM);
         SendMsg(msg, oCpuCtl);

         /* BuildMsg(msg, MSGCPU_SET_CLOCKX3); */
         /* SendMsg(msg, oCpuCtl);             */

         rc = SelectResult(msg);
         break;

      case MSGCPU_SET_DOZE:
         /* Set doze mode */
         /*               */
         SetHid0Power(BITPOWERMODE_DOZE);

         do
         {
            SetMsrPow();
         } while(ReadMsrPow() == POWERMGT_ON); /* enddo */

         SetHid0Power(0);

         rc = ERRCPU_NOERROR;
         break;

      case MSGCPU_SET_NAP:
         /* Set nap mode */
         /*              */
         SetHid0Power(BITPOWERMODE_NAP);

         do
         {
            SetMsrPow();
         } while(ReadMsrPow() == POWERMGT_ON); /* enddo */

         SetHid0Power(0);

         rc = ERRCPU_NOERROR;
         break;

      case MSGCPU_SET_SLEEP:
         /* Set sleep mode */
         /*                */
         SetHid0Power(BITPOWERMODE_SLEEP);

         do
         {
            SetMsrPow();
         } while(ReadMsrPow() == POWERMGT_ON); /* enddo */

         SetHid0Power(0);

         rc = ERRCPU_NOERROR;
         break;

      case MSGCPU_SET_SLEEPANDSUSPEND:
         /* Set sleep mode and suspend */
         /*                            */
         SetHid0Power(BITPOWERMODE_SLEEP);

         do
         {
            SetMsrPowWithOutport(GetIsaBaseAddress() + 0x0092L,
                               ReadControllerByte(GetIsaBaseAddress() + 0x0092L)
                               & ~0x02);
         } while(ReadMsrPow() == POWERMGT_ON); /* enddo */

         SetHid0Power(0);

         rc = ERRCPU_NOERROR;
         break;

      case MSGCPU_SET_DPM:
         /* Set dynamic power management */
         /*                              */
         SetHid0Dpm();
         rc = ERRCPU_NOERROR;
         break;

      case MSGCPU_RESET_DPM:
         /* Reset dynamic power management */
         /*                                */
         ResetHid0Dpm();
         rc = ERRCPU_NOERROR;
         break;

      case MSGCPU_SET_CLOCKX1:
         /* Set clock rate x1 */
         /*                   */
         BuildMsg(msg, MSGCPU_GET_PLLCONFIGURATION);
         SendMsg(msg, oCpuCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGCPU_CHANGE_CLOCKRATE,
                                   SelectParm1(msg));
            SendMsg(msg, oCpuCtl);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCPU_SET_CLOCKX2:
         /* Set clock rate x2 */
         /*                   */
         BuildMsg(msg, MSGCPU_GET_PLLCONFIGURATION);
         SendMsg(msg, oCpuCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGCPU_CHANGE_CLOCKRATE,
                                   SelectParm2(msg));
            SendMsg(msg, oCpuCtl);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCPU_SET_CLOCKX3:
         /* Set clock rate x3 */
         /*                   */
         BuildMsg(msg, MSGCPU_GET_PLLCONFIGURATION);
         SendMsg(msg, oCpuCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGCPU_CHANGE_CLOCKRATE,
                                   SelectParm3(msg));
            SendMsg(msg, oCpuCtl);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCPU_SET_CLOCKX4:
         /* Set clock rate x4 */
         /*                   */
         BuildMsg(msg, MSGCPU_GET_PLLCONFIGURATION);
         SendMsg(msg, oCpuCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGCPU_CHANGE_CLOCKRATE,
                                   SelectParm4(msg));
            SendMsg(msg, oCpuCtl);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCPU_CHANGE_CLOCKRATE:
         /* Change clock rate */
         /*                   */
         if(SelectParm1(*pmMsg) != 0xFF)
         {
            BuildMsg(mCarr, MSGCARR_SET_CPUCLOCKCHANGETIMING);
            SendMsg(mCarr, oCarreraCtl);

            BuildMsgWithParm1(mCarr, MSGCARR_SET_CPUCLOCKRATE,
                                     SelectParm1(*pmMsg));
            SendMsg(mCarr, oCarreraCtl);

            if(!(rc = SelectResult(mCarr)))
            {
               BuildMsgWithParm1(mCarr, MSGCARR_SET_CCSTATUS,
                                        PRMCARR_CCENABLE);
               SendMsg(mCarr, oCarreraCtl);

               if(!(rc = SelectResult(mCarr)))
               {
                  DisableInterrupt();

                  BuildMsg(mCoop, MSGCOOP_GET_L2CACHESTATUS);
                  SendMsg(mCoop, oCooperCtl);
                  l2status = SelectParm1(mCoop);

                  BuildMsg(mCoop, MSGCOOP_FLUSHDISABLE_L2CACHE);
                  SendMsg(mCoop, oCooperCtl);

                  EnableInterrupt();

                  BuildMsg(msg, MSGCPU_SET_SLEEP);
                  SendMsg(msg, oCpuCtl);

                  BuildMsgWithParm1(mCoop, MSGCOOP_SET_L2CACHESTATUS,
                                           l2status);
                  SendMsg(mCoop, oCooperCtl);
               }
               else
               {
                  rc = ERRCPU_CANNOT_CPUCONTROL;
               } /* endif */
            }
            else
            {
               rc = ERRCPU_CANNOT_CPUCONTROL;
            } /* endif */
         }
         else
         {
            rc = ERRCPU_CANNOT_CPUCONTROL;
         } /* endif */
         break;

      case MSGCPU_GET_PLLCONFIGURATION:
         /* Get PLL-configuration */
         /*                       */
         clock = (WORD)(4000L / TimebaseFrequencyTime);
         rc    = ERRCPU_CANNOT_CPUCONTROL;

         for(plltable = PllConfigurationTable; plltable->BusClock; plltable++)
         {
            if((clock >= plltable->BusClock - 1) &&
               (clock <= plltable->BusClock + 1)  )
            {
               SelectParm1(*pmMsg) = (CPARAMETER)plltable->PllCfg11;
               SelectParm2(*pmMsg) = (CPARAMETER)plltable->PllCfg21;
               SelectParm3(*pmMsg) = (CPARAMETER)plltable->PllCfg31;
               SelectParm4(*pmMsg) = (CPARAMETER)plltable->PllCfg41;
               SelectParm5(*pmMsg) = (CPARAMETER)plltable->BusClock;

               rc = ERRCPU_NOERROR;
               break;
            } /* endif */
         } /* endfor */
         break;

      case MSGCPU_GET_CPUCLOCK:
         /* Get CPU clock */
         /*               */
         BuildMsg(msg, MSGCPU_GET_PLLCONFIGURATION);
         SendMsg(msg, oCpuCtl);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm5(msg);

            BuildMsg(mCarr, MSGCARR_GET_CPUCLOCKRATE);
            SendMsg(mCarr, oCarreraCtl);

            if(!(rc = SelectResult(mCarr)))
            {
               SelectParm2(*pmMsg) = SelectParm3(*pmMsg) = 0;

               for(i = 1, pparm = &SelectParm1(msg); i <= 4; i++, pparm++)
               {
                  if(*pparm == SelectParm1(mCarr))
                  {
                     SelectParm2(*pmMsg) = SelectParm5(msg) * i;
                  } /* endif */
                  if(*pparm != 0xFF)
                  {
                     SelectParm3(*pmMsg) = SelectParm5(msg) * i;
                  } /* endif */
               } /* endfor */
            }
            else
            {
               rc = ERRCPU_CANNOT_CPUCONTROL;
            } /* endif */
         } /* endif */
         break;

      case MSGCPU_FLUSH_L1CACHE:
         /* Flush L1 cache */
         /*                */
         FlushL1Cache();
         rc = ERRCPU_NOERROR;
         break;

      default:
         rc = ERRCPU_INVALID_MESSAGE;
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
