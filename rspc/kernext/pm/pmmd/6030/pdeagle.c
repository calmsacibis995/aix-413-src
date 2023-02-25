/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: EAGLE Memory Controller Chip Device Control Routines
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
/* * Module Name: PDEAGLE.C                                                * */
/* * Description: EAGLE Memory Controller Chip Device Control Routines     * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDEAGLE.H/PDPCIBUS.H files should  * */
/* *              be included in this file                                 * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdeagle.h"
#include "pdpcibus.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *       EAGLE Data Definition       * */
/* ************************************* */
/* ------------------------------------- */
/*  Configuration register port address  */
/* ------------------------------------- */
#define  PORTEAGL_BASE                    (0x80000000L)
#define  PORTEAGL_BASENULL                (0xFFFFFFFFL)
                                                       /* EAGLE base address */
                                                      /* & base null address */
                                                 /* Offset from base address */
#define  PORTEAGL_BRIDGENUMBER            (0x40)
                                                   /* Bridge number register */
#define  PORTEAGL_SUBORDINATEBUSNUMBER    (0x41)
                                          /* Subordinate bus number register */
#define  PORTEAGL_DISCONNECTCOUNTER       (0x42)
                                              /* Disconnect counter register */
#define  PORTEAGL_SPECIALCYCLEADDRESS     (0x44)
                                           /* Special cycle address register */
#define  PORTEAGL_POWERMANAGEMENTCONFIG   (0x70)
                                  /* Power management configuration register */
#define  PORTEAGL_MEMSTARTINGADDRESS      (0x80)
                                         /* Memory starting address register */
#define  PORTEAGL_EXTMEMSTARTINGADDRESS   (0x88)
                                /* Extended memory starting address register */
#define  PORTEAGL_MEMENDINGADDRESS        (0x90)
                                           /* Memory ending address register */
#define  PORTEAGL_EXTMEMENDINGADDRESS     (0x98)
                                  /* Extended memory ending address register */
#define  PORTEAGL_MEMORYENABLE            (0xA0)
                                                   /* Memory enable register */
#define  PORTEAGL_PROCESSORCONFIGA8       (0xA8)
                               /* Processor interface configuration register */
#define  PORTEAGL_PROCESSORCONFIGAC       (0xAC)
                               /* Processor interface configuration register */
#define  PORTEAGL_ALTOSVISIBLEPARM1       (0xBA)
                               /* Alternate OS visible parameters 1 register */
#define  PORTEAGL_ALTOSVISIBLEPARM2       (0xBB)
                               /* Alternate OS visible parameters 2 register */
#define  PORTEAGL_ERRORENABLING1          (0xC0)
                                                /* Error enabling 1 register */
#define  PORTEAGL_ERRORDETECTION1         (0xC1)
                                               /* Error detection 1 register */
#define  PORTEAGL_CPUBUSERRORSTATUS       (0xC3)
                                            /* CPU bus error status register */
#define  PORTEAGL_ERRORENABLING2          (0xC4)
                                                /* Error enabling 2 register */
#define  PORTEAGL_ERRORDETECTION2         (0xC5)
                                               /* Error detection 2 register */
#define  PORTEAGL_PCIBUSERRORSTATUS       (0xC7)
                                            /* PCI bus error status register */
#define  PORTEAGL_CPUPCIERRORADDRESS      (0xC8)
                                           /* CPU/PCI error address register */
#define  PORTEAGL_MEMORYCONTROLCONFIG8F0  (0xF0)
                                  /* Memory control configuration 8 register */
#define  PORTEAGL_MEMORYCONTROLCONFIG8F4  (0xF4)
                                  /* Memory control configuration 8 register */
#define  PORTEAGL_MEMORYCONTROLCONFIG9F8  (0xF8)
                                  /* Memory control configuration 9 register */
#define  PORTEAGL_MEMORYCONTROLCONFIG9FC  (0xFC)
                                  /* Memory control configuration 9 register */

/* --------------------------------------------------------------- */
/*  Power management configuration register (PMCONFIG) bit assign  */
/* --------------------------------------------------------------- */
#define  BITEAGLPMCONFIG_BR1_WAKE         (0x00000001L)
                          /* (Bit-0) Wake from nap or sleep                  */
#define  BITEAGLPMCONFIG_CKO_SEL          (0x00000002L)
                          /* (Bit-1) Test clock output select                */
#define  BITEAGLPMCONFIG_CKO_EN           (0x00000004L)
                          /* (Bit-2) Test clock output enable                */
#define  BITEAGLPMCONFIG_SLEEP            (0x00000008L)
                          /* (Bit-3) Sleep mode                              */
#define  BITEAGLPMCONFIG_NAP              (0x00000010L)
                          /* (Bit-4) Nap mode                                */
#define  BITEAGLPMCONFIG_DOZE             (0x00000020L)
                          /* (Bit-5) Doze mode                               */
#define  BITEAGLPMCONFIG_RESERVED6        (0x00000040L)
                          /* (Bit-6) Reserved                                */
#define  BITEAGLPMCONFIG_PM               (0x00000080L)
                          /* (Bit-7) Power management                        */
#define  BITEAGLPMCONFIG_RESERVED8        (0x00000100L)
                          /* (Bit-8) Reserved                                */
#define  BITEAGLPMCONFIG_RESERVED9        (0x00000200L)
                          /* (Bit-9) Reserved                                */
#define  BITEAGLPMCONFIG_601_NEED_QREQ    (0x00000400L)
                          /* (Bit-10) QREQ_ signal need                      */
#define  BITEAGLPMCONFIG_NO_604_RUN       (0x00000800L)
                          /* (Bit-11) RUN signal to force CPU                */
#define  BITEAGLPMCONFIG_LP_REF_EN        (0x00001000L)
                          /* (Bit-12) Memory refresh cycle in low power mode */
#define  BITEAGLPMCONFIG_SLEEP_MSG_TYPE   (0x00002000L)
                          /* (Bit-13) Broadcast message type on PCI          */
#define  BITEAGLPMCONFIG_NO_SLEEP_MSG     (0x00004000L)
                          /* (Bit-14) Broadcast indicator on sleep           */
#define  BITEAGLPMCONFIG_NO_NAP_MSG       (0x00008000L)
                          /* (Bit-15) Broadcast indicator on nap             */

/* --------------------------------------------------------------------- */
/*  Memory control configuration 8 register (MEMCTRLCONFIG8) bit assign  */
/* --------------------------------------------------------------------- */
#define  BITEAGLMEMCTRL8F0_RAMBANK0ROW    (0x00000003L)
                          /* (Bit-0,1) RAM bank 0 row address bit count      */
#define  BITEAGLMEMCTRL8F0_RAMBANK1ROW    (0x0000000CL)
                          /* (Bit-2,3) RAM bank 1 row address bit count      */
#define  BITEAGLMEMCTRL8F0_RAMBANK2ROW    (0x00000030L)
                          /* (Bit-4,5) RAM bank 2 row address bit count      */
#define  BITEAGLMEMCTRL8F0_RAMBANK3ROW    (0x000000C0L)
                          /* (Bit-6,7) RAM bank 3 row address bit count      */
#define  BITEAGLMEMCTRL8F0_RAMBANK4ROW    (0x00000300L)
                          /* (Bit-8,9) RAM bank 4 row address bit count      */
#define  BITEAGLMEMCTRL8F0_RAMBANK5ROW    (0x00000C00L)
                          /* (Bit-10,11) RAM bank 5 row address bit count    */
#define  BITEAGLMEMCTRL8F0_RAMBANK6ROW    (0x00003000L)
                          /* (Bit-12,13) RAM bank 6 row address bit count    */
#define  BITEAGLMEMCTRL8F0_RAMBANK7ROW    (0x0000C000L)
                          /* (Bit-14,15) RAM bank 7 row address bit count    */
#define  BITEAGLMEMCTRL8F0_PCKEN          (0x00010000L)
                          /* (Bit-16) Parity checking on memory interface    */
#define  BITEAGLMEMCTRL8F0_RAMTYP         (0x00020000L)
                          /* (Bit-17) Memory type                            */
#define  BITEAGLMEMCTRL8F0_SREN           (0x00040000L)
                          /* (Bit-18) Dynamic memory self refresh capability */
#define  BITEAGLMEMCTRL8F0_MEMGO          (0x00080000L)
                          /* (Bit-19) RAM interface                          */
#define  BITEAGLMEMCTRL8F0_BURST          (0x00100000L)
                          /* (Bit-20) Burst mode ROM                         */
#define  BITEAGLMEMCTRL8F0_32N64          (0x00200000L)
                          /* (Bit-21)                                        */
#define  BITEAGLMEMCTRL8F0_FNR            (0x00400000L)
                          /* (Bit-22)                                        */

#define  BITEAGLMEMCTRL8F4_WMODE          (0x00000000L)
                          /* (Bit-0)                                         */
#define  BITEAGLMEMCTRL8F4_BUF            (0x00000000L)
                          /* (Bit-1)                                         */
#define  BITEAGLMEMCTRL8F4_REFINT         (0x00000000L)
                          /* (Bit-2,15)                                      */
#define  BITEAGLMEMCTRL8F4_RESERVED16     (0x00000000L)
                          /* (Bit-16)                                        */
#define  BITEAGLMEMCTRL8F4_RESERVED17     (0x00000000L)
                          /* (Bit-17)                                        */
#define  BITEAGLMEMCTRL8F4_SRF            (0x00000000L)
                          /* (Bit-18,31)                                     */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    EAGLE Controller Definition    * */
/* ************************************* */
/* ----------------- */
/*  EAGLE I/O class  */
/* ----------------- */
VOID  EagleIo(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj);               /* Pointer to Common object */

/* --------------------- */
/*  EAGLE control class  */
/* --------------------- */
VOID  EagleCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *           EAGLE Objects           * */
/* ************************************* */
/* ----------------- */
/*  EAGLE I/O class  */
/* ----------------- */
OBJEAGLEIO  oEagleIo  = {(POBJCTL)EagleIo, PORTEAGL_BASENULL};

/* --------------------- */
/*  EAGLE control class  */
/* --------------------- */
OBJEAGLECTL oEagleCtl = {(POBJCTL)EagleCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************ */
/* *             EAGLE I/O Controller             * */
/* ************************************************ */
/* *                                              * */
/* * - Initialize                                 * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEAGL_INITIALIZE        * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read register                              * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEAGL_READ_xxx          * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       Parameter1 = register value            * */
/* *       EagleParm1 = register value (32bits)   * */
/* *                                              * */
/* * - Write register                             * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEAGL_WRITE_xxx         * */
/* *       Parameter1 = register value (not used) * */
/* *       EagleParm1 = register value (32bits)   * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read & Modify register                     * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEAGL_MODIFY_xxx        * */
/* *       Parameter1 = register value (not used) * */
/* *       EagleParm1 = set bits value  (32bits)  * */
/* *       EagleParm2 = mask bits value (32bits)  * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* ************************************************ */
VOID  EagleIo(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj)                /* Pointer to Common object */
{
   MSGPCI      mPci;
   WORD        off;
   DWORD       data;
   CRESULT     rc;
   POBJEAGLEIO peagleio;

   /* Get pointer to EAGLE I/O object */
   /*                                 */
   peagleio = (POBJEAGLEIO)poObj;

   /* Check if completing for initialization */
   /*                                        */
   if((peagleio->BaseAddress != PORTEAGL_BASENULL ) ||
      (SelectMessage(*pmMsg) == MSGEAGL_INITIALIZE)   )
   {
      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGEAGL_INITIALIZE:
            /* Initialize */
            /*            */
            BuildMsg(mPci, MSGPCI_QUERY_DEVICE);
            SendMsg(mPci, oPciEagleCtl);

            if(!(rc = SelectResult(mPci)))
            {
               BuildMsg(mPci, MSGPCI_GET_CONFIGADDRESS);
               SendMsg(mPci, oPciEagleCtl);

               if(!(rc = SelectResult(mPci)))
               {
                  peagleio->BaseAddress = SelectPciParm1(mPci);
               }
               else
               {
                  rc = ERREAGL_CANNOT_INITIALIZE;
               } /* endif */
            }
            else
            {
               rc = ERREAGL_CANNOT_INITIALIZE;
            } /* endif */
            break;

         default:
            /* Get offset to data-port register */
            /*                                  */
            switch(SelectMessage(*pmMsg))
            {
               case MSGEAGL_READ_POWERMGTCONFIG:
               case MSGEAGL_WRITE_POWERMGTCONFIG:
               case MSGEAGL_MODIFY_POWERMGTCONFIG:
                  off = PORTEAGL_POWERMANAGEMENTCONFIG;
                  rc  = ERREAGL_NOERROR;
                  break;
               case MSGEAGL_READ_MEMCTRLCONFIG8F0:
               case MSGEAGL_WRITE_MEMCTRLCONFIG8F0:
               case MSGEAGL_MODIFY_MEMCTRLCONFIG8F0:
                  off = PORTEAGL_MEMORYCONTROLCONFIG8F0;
                  rc  = ERREAGL_NOERROR;
                  break;
               default:
                  rc = ERREAGL_INVALID_MESSAGE;
            } /* endswitch */

            if(!rc)
            {
               /* Input/Output registers */
               /*                        */
               switch(SelectMessage(*pmMsg))
               {
                  case MSGEAGL_READ_POWERMGTCONFIG:
                  case MSGEAGL_READ_MEMCTRLCONFIG8F0:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciEagleCtl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        SelectEagleParm1(*pmMsg) = SelectPciParm1(mPci);
                        SelectParm1(*pmMsg)      =
                                      (CPARAMETER)SelectPciParm1(mPci) & 0x00FF;
                     }
                     else
                     {
                        rc = ERREAGL_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGEAGL_WRITE_POWERMGTCONFIG:
                  case MSGEAGL_WRITE_MEMCTRLCONFIG8F0:
                     BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                             off);
                     BuildPciParm1(mPci, SelectEagleParm1(*pmMsg));
                     SendMsg(mPci, oPciEagleCtl);

                     if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                     {
                        rc = ERREAGL_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGEAGL_MODIFY_POWERMGTCONFIG:
                  case MSGEAGL_MODIFY_MEMCTRLCONFIG8F0:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciEagleCtl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        data = (SelectPciParm1(mPci)     &
                                ~SelectEagleParm2(*pmMsg)) |
                               (SelectEagleParm1(*pmMsg) &
                                SelectEagleParm2(*pmMsg) );

                        BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                                off);
                        BuildPciParm1(mPci, data);
                        SendMsg(mPci, oPciEagleCtl);

                        if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                        {
                           rc = ERREAGL_CANNOT_PCIACCESS;
                        } /* endif */
                     }
                     else
                     {
                        rc = ERREAGL_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  default:
                     rc = ERREAGL_INVALID_MESSAGE;
               } /* endswitch */
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERREAGL_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ****************************************************** */
/* *              EAGLE Control Controller              * */
/* ****************************************************** */
/* *                                                    * */
/* * - Initialize                                       * */
/* *     [ Input ]                                      * */
/* *       Message    = MSGEAGL_INITIALIZECTL           * */
/* *     [ Output ]                                     * */
/* *       Result     = result code                     * */
/* *                                                    * */
/* * - Set power management mode                        * */
/* *     [ Input ]                                      * */
/* *       Message    = MSGEAGL_SET_POWERMANAGEMENTMODE * */
/* *       Parameter1 = power mode                      * */
/* *                    (PRMEAGL_POWERMODEENABLE )      * */
/* *                    (PRMEAGL_POWERMODEDISABLE)      * */
/* *     [ Output ]                                     * */
/* *       Result     = result code                     * */
/* *                                                    * */
/* * - Set refresh mode                                 * */
/* *     [ Input ]                                      * */
/* *       Message    = MSGEAGL_SET_REFRESHMODE         * */
/* *       Parameter1 = refresh mode                    * */
/* *                    (PRMEAGL_SELFREFRESH    )       * */
/* *                    (PRMEAGL_LOWPOWERREFRESH)       * */
/* *     [ Output ]                                     * */
/* *       Result     = result code                     * */
/* *                                                    * */
/* * - Save device context                              * */
/* *     [ Input ]                                      * */
/* *       Message    = MSGEAGL_SAVE_CONTEXT            * */
/* *     [ Output ]                                     * */
/* *       Result     = result code                     * */
/* *                                                    * */
/* * - Restore device context                           * */
/* *     [ Input ]                                      * */
/* *       Message    = MSGEAGL_RESTORE_CONTEXT         * */
/* *     [ Output ]                                     * */
/* *       Result     = result code                     * */
/* *                                                    * */
/* ****************************************************** */
VOID  EagleCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   MSGEAGLE       msg;
   CRESULT        rc;
   POBJEAGLECTL   peaglectl;

   /* Get pointer to EAGLE control object */
   /*                                     */
   peaglectl = (POBJEAGLECTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGEAGL_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGEAGL_INITIALIZE);
         SendMsg(msg, oEagleIo);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGEAGL_SET_POWERMANAGEMENTMODE,
                                   PRMEAGL_POWERMODEENABLE);
            SendMsg(msg, oEagleCtl);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGEAGL_SET_POWERMANAGEMENTMODE:
         /* Set power management mode */
         /*                           */
         switch(SelectParm1(*pmMsg))
         {
            case PRMEAGL_POWERMODEENABLE:
               BuildMsg(msg, MSGEAGL_MODIFY_POWERMGTCONFIG);
               BuildEagleParm2(msg, (DWORD)BITEAGLPMCONFIG_PM
                                    << ((PORTEAGL_POWERMANAGEMENTCONFIG -
                                         PORTEAGL_POWERMANAGEMENTCONFIG ) * 8),
                                    (DWORD)BITEAGLPMCONFIG_PM
                                    << ((PORTEAGL_POWERMANAGEMENTCONFIG -
                                         PORTEAGL_POWERMANAGEMENTCONFIG ) * 8));
               SendMsg(msg, oEagleIo);

               rc = SelectResult(msg);
               break;
            case PRMEAGL_POWERMODEDISABLE:
               BuildMsg(msg, MSGEAGL_MODIFY_POWERMGTCONFIG);
               BuildEagleParm2(msg, 0,
                                    (DWORD)BITEAGLPMCONFIG_PM
                                    << ((PORTEAGL_POWERMANAGEMENTCONFIG -
                                         PORTEAGL_POWERMANAGEMENTCONFIG ) * 8));
               SendMsg(msg, oEagleIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERREAGL_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGEAGL_SET_REFRESHMODE:
         /* Set refresh mode */
         /*                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMEAGL_SELFREFRESH:
               BuildMsg(msg, MSGEAGL_MODIFY_MEMCTRLCONFIG8F0);
               BuildEagleParm2(msg, (DWORD)BITEAGLMEMCTRL8F0_SREN
                                    << ((PORTEAGL_MEMORYCONTROLCONFIG8F0 -
                                         PORTEAGL_MEMORYCONTROLCONFIG8F0 ) * 8),
                                    (DWORD)BITEAGLMEMCTRL8F0_SREN
                                    << ((PORTEAGL_MEMORYCONTROLCONFIG8F0 -
                                         PORTEAGL_MEMORYCONTROLCONFIG8F0) * 8));
               SendMsg(msg, oEagleIo);

               BuildMsg(msg, MSGEAGL_MODIFY_POWERMGTCONFIG);
               BuildEagleParm2(msg, 0,
                                    (DWORD)BITEAGLPMCONFIG_LP_REF_EN
                                    << ((PORTEAGL_POWERMANAGEMENTCONFIG -
                                         PORTEAGL_POWERMANAGEMENTCONFIG ) * 8));
               SendMsg(msg, oEagleIo);

               rc = SelectResult(msg);
               break;
            case PRMEAGL_LOWPOWERREFRESH:
               BuildMsg(msg, MSGEAGL_MODIFY_MEMCTRLCONFIG8F0);
               BuildEagleParm2(msg, 0,
                                    (DWORD)BITEAGLMEMCTRL8F0_SREN
                                    << ((PORTEAGL_MEMORYCONTROLCONFIG8F0 -
                                         PORTEAGL_MEMORYCONTROLCONFIG8F0) * 8));
               SendMsg(msg, oEagleIo);

               BuildMsg(msg, MSGEAGL_MODIFY_POWERMGTCONFIG);
               BuildEagleParm2(msg, (DWORD)BITEAGLPMCONFIG_LP_REF_EN
                                    << ((PORTEAGL_POWERMANAGEMENTCONFIG -
                                         PORTEAGL_POWERMANAGEMENTCONFIG ) * 8),
                                    (DWORD)BITEAGLPMCONFIG_LP_REF_EN
                                    << ((PORTEAGL_POWERMANAGEMENTCONFIG -
                                         PORTEAGL_POWERMANAGEMENTCONFIG ) * 8));
               SendMsg(msg, oEagleIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERREAGL_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGEAGL_ENTER_SUSPEND:
         /* Enter suspend */
         /*               */
         rc = ERREAGL_NOERROR;

         BuildMsgWithParm1(msg, MSGEAGL_SET_POWERMANAGEMENTMODE,
                                PRMEAGL_POWERMODEENABLE);
         SendMsg(msg, oEagleCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGEAGL_SET_REFRESHMODE,
                                PRMEAGL_LOWPOWERREFRESH);
         SendMsg(msg, oEagleCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGEAGL_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERREAGL_NOERROR;

         BuildMsg(msg, MSGEAGL_READ_POWERMGTCONFIG);
         SendMsg(msg, oEagleIo);
         if(!SelectResult(msg))
         {
            peaglectl->dwOffset70h = SelectEagleParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGEAGL_READ_MEMCTRLCONFIG8F0);
         SendMsg(msg, oEagleIo);
         if(!SelectResult(msg))
         {
            peaglectl->dwOffsetF0h = SelectEagleParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGEAGL_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERREAGL_NOERROR;

         BuildMsg(msg, MSGEAGL_WRITE_POWERMGTCONFIG);
         BuildEagleParm1(msg, peaglectl->dwOffset70h);
         SendMsg(msg, oEagleIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGEAGL_WRITE_MEMCTRLCONFIG8F0);
         BuildEagleParm1(msg, peaglectl->dwOffsetF0h);
         SendMsg(msg, oEagleIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oEagleIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
