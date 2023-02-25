/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Super I/O (PC87322/311) Chip Device Control Routines
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
/* * Module Name: PDSUPRIO.C                                               * */
/* * Description: Super I/O (PC87322/311) Chip Device Control Routines     * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDSUPRIO.H files should be included* */
/* *              in this file                                             * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdsuprio.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     Super-I/O Data Definition     * */
/* ************************************* */
/* ------------------------------------------------ */
/*  Configuration register index/data port address  */
/* ------------------------------------------------ */
#define  PORTSPIO_BASE                    (0x00000398L)
                                                   /* Super-I/O base address */
#define  PORTSPIO_INDEX                   (PORTSPIO_BASE + 0)
                                                           /* Index register */
#define  PORTSPIO_DATA                    (PORTSPIO_BASE + 1)
                                                            /* Data register */

/* ----------------------------------------- */
/*  Index data to index Super-I/O registers  */
/* ----------------------------------------- */
#define  INDEXSPIO_FER                    (0x00)
                                                 /* Function enable register */
#define  INDEXSPIO_FAR                    (0x01)
                                                /* Function address register */
#define  INDEXSPIO_PTR                    (0x02)
                                                  /* Power and test register */
#define  INDEXSPIO_FCR                    (0x03)
                                                /* Function control register */
#define  INDEXSPIO_PCR                    (0x04)
                                                 /* Printer control register */

/* ------------------------------------------- */
/*  Function enable register (FER) bit assign  */
/* ------------------------------------------- */
#define  BITSPIOFER_PARALLELENABLE        (0x01)
                                     /* (Bit-0) Parallel port enable         */
#define  BITSPIOFER_UART1ENABLE           (0x02)
                                     /* (Bit-1) UART 1 enable                */
#define  BITSPIOFER_UART2ENABLE           (0x04)
                                     /* (Bit-2) UART 2 enable                */
#define  BITSPIOFER_FDCENABLE             (0x08)
                                     /* (Bit-3) FDC enable                   */
#define  BITSPIOFER_FDCDRIVEENCODE        (0x10)
                                     /* (Bit-4) FDC 4-drive encoding         */
#define  BITSPIOFER_SELECTSECONDARYFDC    (0x20)
                                     /* (Bit-5) Select FDC secondary address */
#define  BITSPIOFER_IDEENABLE             (0x40)
                                     /* (Bit-6) IDE enable                   */
#define  BITSPIOFER_SELECTSECONDARYIDE    (0x80)
                                     /* (Bit-7) Select IDE secondary address */

/* -------------------------------------------- */
/*  Function address register (FAR) bit assign  */
/* -------------------------------------------- */
#define  BITSPIOFAR_PARALLEL              (0x03)
                                          /* (Bit-0,1) Parallel port address */
#define     BITSPIOFAR_PARALLEL_LPT2      (0x00)
                                          /*           LPT2 (378-F, IRQ5)    */
#define     BITSPIOFAR_PARALLEL_LPT1      (0x01)
                                          /*           LPT1 (3BC-E, IRQ7)    */
#define     BITSPIOFAR_PARALLEL_LPT3      (0x02)
                                          /*           LPT3 (278-F, IRQ5)    */
#define  BITSPIOFAR_UART1                 (0x0C)
                                          /* (Bit-2,3) UART 1 address        */
#define     BITSPIOFAR_UART1_COM1         (0x00)
                                          /*           COM1 (3F8-F)          */
#define     BITSPIOFAR_UART1_COM2         (0x04)
                                          /*           COM2 (2F8-F)          */
#define     BITSPIOFAR_UART1_COM3         (0x08)
                                          /*           COM3 (3E8-F)          */
#define     BITSPIOFAR_UART1_COM4         (0x0C)
                                          /*           COM4 (2E8-F)          */
#define  BITSPIOFAR_UART2                 (0x30)
                                          /* (Bit-4,5) UART 2 address        */
#define     BITSPIOFAR_UART2_COM1         (0x00)
                                          /*           COM1 (3F8-F)          */
#define     BITSPIOFAR_UART2_COM2         (0x10)
                                          /*           COM2 (2F8-F)          */
#define     BITSPIOFAR_UART2_COM3         (0x20)
                                          /*           COM3 (3E8-F)          */
#define     BITSPIOFAR_UART2_COM4         (0x30)
                                          /*           COM4 (2E8-F)          */
#define  BITSPIOFAR_SELECTCOM34           (0xC0)
                                          /* (Bit-6,7) Select COM 3,4        */
#define     BITSPIOFAR_SELECT_3E82E8      (0x00)
                                          /*           Select 3E8,2E8h       */
#define     BITSPIOFAR_SELECT_338238      (0x40)
                                          /*           Select 338,238h       */
#define     BITSPIOFAR_SELECT_2E82E0      (0x80)
                                          /*           Select 2E8,2E0h       */
#define     BITSPIOFAR_SELECT_220228      (0xC0)
                                          /*           Select 220,228h       */

/* ------------------------------------------ */
/*  Power and test register (PTR) bit assign  */
/* ------------------------------------------ */
#define  BITSPIOPTR_POWERDOWN             (0x01)
                         /* (Bit-0) Power down                               */
#define  BITSPIOPTR_XTALCLKPOWERDOWN      (0x02)
                         /* (Bit-1) XTAL/CLK power down                      */
#define  BITSPIOPTR_RESERVED2             (0x04)
                         /* (Bit-2) Reserved                                 */
#define  BITSPIOPTR_RESERVED3             (0x08)
                         /* (Bit-3) Reserved                                 */
#define  BITSPIOPTR_RESERVED4             (0x10)
                         /* (Bit-4) Reserved                                 */
#define  BITSPIOPTR_RESERVED5             (0x20)
                         /* (Bit-5) Reserved                                 */
#define  BITSPIOPTR_RESERVED6             (0x40)
                         /* (Bit-6) Reserved                                 */
#define  BITSPIOPTR_EXTENDEDPARALLEL      (0x80)
                         /* (Bit-7) Extended/Compatible parallel port select */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  WAITTIME_STABILIZATION           (8)     /* Stabilization wait time */
                                                  /* (8 msec)                */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *  Super-I/O Controller Definition  * */
/* ************************************* */
/* --------------------- */
/*  Super-I/O I/O class  */
/* --------------------- */
VOID  SuperioIo(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */

/* ------------------------- */
/*  Super-I/O control class  */
/* ------------------------- */
VOID  SuperioCtl(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj);            /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *         Super-I/O Objects         * */
/* ************************************* */
/* --------------------- */
/*  Super-I/O I/O class  */
/* --------------------- */
OBJSUPERIOIO   oSuperioIo  = {(POBJCTL)SuperioIo, PORTISA_BASE};

/* ------------------------- */
/*  Super-I/O control class  */
/* ------------------------- */
OBJSUPERIOCTL  oSuperioCtl = {(POBJCTL)SuperioCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ***************************************** */
/* *       Super-I/O I/O Controller        * */
/* ***************************************** */
/* *                                       * */
/* * - Initialize                          * */
/* *     [ Input ]                         * */
/* *       Message    = MSGSPIO_INITIALIZE * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* * - Get index register                  * */
/* *     [ Input ]                         * */
/* *       Message    = MSGSPIO_GET_INDEX  * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *       Parameter1 = index value        * */
/* *                                       * */
/* * - Set index register                  * */
/* *     [ Input ]                         * */
/* *       Message    = MSGSPIO_SET_INDEX  * */
/* *       Parameter1 = index value        * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* * - Read register                       * */
/* *     [ Input ]                         * */
/* *       Message    = MSGSPIO_READ_xxx   * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *       Parameter1 = register value     * */
/* *                                       * */
/* * - Write register                      * */
/* *     [ Input ]                         * */
/* *       Message    = MSGSPIO_WRITE_xxx  * */
/* *       Parameter1 = register value     * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* * - Read & Modify register              * */
/* *     [ Input ]                         * */
/* *       Message    = MSGSPIO_MODIFY_xxx * */
/* *       Parameter1 = set bits value     * */
/* *       Parameter2 = mask bits value    * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* ***************************************** */
VOID  SuperioIo(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   BYTE           index, data, sindex;
   CRESULT        rc;
   POBJSUPERIOIO  pspioio;
   DWORD          indexport, dataport;

   /* Get pointer to Super-I/O I/O object */
   /*                                     */
   pspioio = (POBJSUPERIOIO)poObj;

   /* Get index/data port address */
   /*                             */
   indexport = GetIsaBaseAddress() + PORTSPIO_INDEX;
   dataport  = GetIsaBaseAddress() + PORTSPIO_DATA;

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGSPIO_INITIALIZE:
         /* Initialize */
         /*            */
         pspioio->BaseAddress = GetIsaBaseAddress();
         rc = ERRSPIO_NOERROR;
         break;

      case MSGSPIO_GET_INDEX:
         /* Get index register */
         /*                    */
         SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(indexport);
         rc = ERRSPIO_NOERROR;
         break;

      case MSGSPIO_SET_INDEX:
         /* Set index register */
         /*                    */
         WriteControllerByte(indexport, (BYTE)SelectParm1(*pmMsg));
         rc = ERRSPIO_NOERROR;
         break;

      default:
         /* Get index register */
         /*                    */
         switch(SelectMessage(*pmMsg))
         {
            case MSGSPIO_READ_FER:
            case MSGSPIO_WRITE_FER:
            case MSGSPIO_MODIFY_FER:
               index = INDEXSPIO_FER;
               rc    = ERRSPIO_NOERROR;
               break;
            case MSGSPIO_READ_FAR:
            case MSGSPIO_WRITE_FAR:
            case MSGSPIO_MODIFY_FAR:
               index = INDEXSPIO_FAR;
               rc    = ERRSPIO_NOERROR;
               break;
            case MSGSPIO_READ_PTR:
            case MSGSPIO_WRITE_PTR:
            case MSGSPIO_MODIFY_PTR:
               index = INDEXSPIO_PTR;
               rc    = ERRSPIO_NOERROR;
               break;
            case MSGSPIO_READ_FCR:
            case MSGSPIO_WRITE_FCR:
            case MSGSPIO_MODIFY_FCR:
               index = INDEXSPIO_FCR;
               rc    = ERRSPIO_NOERROR;
               break;
            case MSGSPIO_READ_PCR:
            case MSGSPIO_WRITE_PCR:
            case MSGSPIO_MODIFY_PCR:
               index = INDEXSPIO_PCR;
               rc    = ERRSPIO_NOERROR;
               break;
            default:
               rc = ERRSPIO_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Save index register */
            /*                     */
            sindex = ReadControllerByte(indexport);

            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case MSGSPIO_READ_FER:
               case MSGSPIO_READ_FAR:
               case MSGSPIO_READ_PTR:
               case MSGSPIO_READ_FCR:
               case MSGSPIO_READ_PCR:
                  WriteControllerByte(indexport, index);
                  SelectParm1(*pmMsg) =
                                       (CPARAMETER)ReadControllerByte(dataport);
                  break;
               case MSGSPIO_WRITE_FER:
               case MSGSPIO_WRITE_FAR:
               case MSGSPIO_WRITE_PTR:
               case MSGSPIO_WRITE_FCR:
               case MSGSPIO_WRITE_PCR:
                  data = (BYTE)SelectParm1(*pmMsg);

                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,  data);
                  WriteControllerByte(dataport,  data);
                  break;
               case MSGSPIO_MODIFY_FER:
               case MSGSPIO_MODIFY_FAR:
               case MSGSPIO_MODIFY_PTR:
               case MSGSPIO_MODIFY_FCR:
               case MSGSPIO_MODIFY_PCR:
                  WriteControllerByte(indexport, index);
                  data = (BYTE)((ReadControllerByte(dataport) &
                                 ~SelectParm2(*pmMsg)         ) |
                                (SelectParm1(*pmMsg)          &
                                 SelectParm2(*pmMsg)          ) );

                  WriteControllerByte(dataport, data);
                  WriteControllerByte(dataport, data);
                  break;
               default:
                  rc = ERRSPIO_INVALID_MESSAGE;
            } /* endswitch */

            /* Restore index register */
            /*                        */
            IoDelay();
            WriteControllerByte(indexport, sindex);
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* **************************************************** */
/* *           Super-I/O Control Controller           * */
/* **************************************************** */
/* *                                                  * */
/* * - Initialize                                     * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_INITIALIZECTL         * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get device status                              * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_GET_xxxSTATUS         * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMSPIO_xxxENABLE )          * */
/* *                    (PRMSPIO_xxxDISABLE)          * */
/* *                                                  * */
/* * - Set device status                              * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_SET_xxxSTATUS         * */
/* *       Parameter1 = status                        * */
/* *                    (PRMSPIO_xxxENABLE )          * */
/* *                    (PRMSPIO_xxxDISABLE)          * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get base address                               * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_GET_xxxADDRESS        * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = base address                  * */
/* *                    (PRMSPIO_xxx)                 * */
/* *                                                  * */
/* * - Set base address                               * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_SET_xxxADDRESS        * */
/* *       Parameter1 = base address                  * */
/* *                    (PRMSPIO_xxx)                 * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get parallel direction                         * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_GET_PARALLELDIRECTION * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = direction                     * */
/* *                    (PRMSPIO_xxx)                 * */
/* *                                                  * */
/* * - Set parallel direction                         * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_SET_PARALLELDIRECTION * */
/* *       Parameter1 = direction                     * */
/* *                    (PRMSPIO_xxx)                 * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get external clock power                       * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_GET_EXTERNALCLOCK     * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = clock status                  * */
/* *                    (PRMSPIO_CLOCKxxx)            * */
/* *                                                  * */
/* * - Set external clock power                       * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_SET_EXTERNALCLOCK     * */
/* *       Parameter1 = clock status                  * */
/* *                    (PRMSPIO_CLOCKxxx)            * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Save device context                            * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_SAVE_CONTEXT          * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Restore device context                         * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGSPIO_RESTORE_CONTEXT       * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* **************************************************** */
VOID  SuperioCtl(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj)             /* Pointer to Common object */
{
   MSGSUPERIO     msg;
   CRESULT        rc;
   POBJSUPERIOCTL pspioctl;

   /* Get pointer to Super-I/O control object */
   /*                                         */
   pspioctl = (POBJSUPERIOCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGSPIO_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGSPIO_INITIALIZE);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                   BITSPIOFAR_SELECT_3E82E8,
                                   BITSPIOFAR_SELECTCOM34);
            SendMsg(msg, oSuperioIo);

            rc = SelectResult(msg);
         } /* endif */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGSPIO_GET_PARALLELSTATUS:
         /* Get parallel status */
         /*                     */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITSPIOFER_PARALLELENABLE)
                                  ? (CPARAMETER)PRMSPIO_PARALLELENABLE
                                  : (CPARAMETER)PRMSPIO_PARALLELDISABLE;
         } /* endif */
         break;

      case MSGSPIO_SET_PARALLELSTATUS:
         /* Set parallel status */
         /*                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_PARALLELENABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_PARALLELENABLE,
                                      BITSPIOFER_PARALLELENABLE);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_PARALLELDISABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_PARALLELENABLE);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_PARALLELADDRESS:
         /* Get parallel base address */
         /*                           */
         BuildMsg(msg, MSGSPIO_READ_FAR);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            switch(SelectParm1(msg) & BITSPIOFAR_PARALLEL)
            {
               case BITSPIOFAR_PARALLEL_LPT1:
                  SelectParm1(*pmMsg) = PRMSPIO_PARALLELLPT1;
                  break;
               case BITSPIOFAR_PARALLEL_LPT2:
                  SelectParm1(*pmMsg) = PRMSPIO_PARALLELLPT2;
                  break;
               case BITSPIOFAR_PARALLEL_LPT3:
                  SelectParm1(*pmMsg) = PRMSPIO_PARALLELLPT3;
                  break;
               default:
                  rc = ERRSPIO_UNKNOWN;
            } /* endswitch */
         } /* endif */
         break;

      case MSGSPIO_SET_PARALLELADDRESS:
         /* Set parallel base address */
         /*                           */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_PARALLELLPT1:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_PARALLEL_LPT1,
                                      BITSPIOFAR_PARALLEL);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_PARALLELLPT2:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_PARALLEL_LPT2,
                                      BITSPIOFAR_PARALLEL);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_PARALLELLPT3:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_PARALLEL_LPT3,
                                      BITSPIOFAR_PARALLEL);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_PARALLELDIRECTION:
         /* Get parallel direction */
         /*                        */
         BuildMsg(msg, MSGSPIO_READ_PTR);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITSPIOPTR_EXTENDEDPARALLEL)
                                  ? (CPARAMETER)PRMSPIO_PARALLELEXTENDED
                                  : (CPARAMETER)PRMSPIO_PARALLELCOMPATIBLE;
         } /* endif */
         break;

      case MSGSPIO_SET_PARALLELDIRECTION:
         /* Set parallel direction */
         /*                        */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_PARALLELEXTENDED:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_PTR,
                                      BITSPIOPTR_EXTENDEDPARALLEL,
                                      BITSPIOPTR_EXTENDEDPARALLEL);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_PARALLELCOMPATIBLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_PTR,
                                      0,
                                      BITSPIOPTR_EXTENDEDPARALLEL);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGSPIO_GET_UART1STATUS:
         /* Get UART 1 status */
         /*                   */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) & BITSPIOFER_UART1ENABLE)
            {
               BuildMsg(msg, MSGSPIO_GET_EXTERNALCLOCK);
               SendMsg(msg, oSuperioCtl);

               if(!(rc = SelectResult(msg)))
               {
                  SelectParm1(*pmMsg) = (SelectParm1(msg) ==
                                         PRMSPIO_CLOCKPOWERDOWN)
                                        ? (CPARAMETER)PRMSPIO_UART1DISABLE
                                        : (CPARAMETER)PRMSPIO_UART1ENABLE;
               } /* endif */
            }
            else
            {
               SelectParm1(*pmMsg) = PRMSPIO_UART1DISABLE;
            } /* endif */
         } /* endif */
         break;

      case MSGSPIO_SET_UART1STATUS:
         /* Set UART 1 status */
         /*                   */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_UART1ENABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_UART1ENABLE,
                                      BITSPIOFER_UART1ENABLE);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm1(msg, MSGSPIO_SET_EXTERNALCLOCK,
                                         PRMSPIO_CLOCKPOWERUP);
                  SendMsg(msg, oSuperioCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            case PRMSPIO_UART1DISABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_UART1ENABLE);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm1(msg, MSGSPIO_SET_EXTERNALCLOCK,
                                         PRMSPIO_CLOCKPOWERDOWN);
                  SendMsg(msg, oSuperioCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_UART1ADDRESS:
         /* Get UART 1 base address */
         /*                         */
         BuildMsg(msg, MSGSPIO_READ_FAR);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            switch(SelectParm1(msg) & BITSPIOFAR_UART1)
            {
               case BITSPIOFAR_UART1_COM1:
                  SelectParm1(*pmMsg) = PRMSPIO_UART1COM1;
                  break;
               case BITSPIOFAR_UART1_COM2:
                  SelectParm1(*pmMsg) = PRMSPIO_UART1COM2;
                  break;
               case BITSPIOFAR_UART1_COM3:
                  SelectParm1(*pmMsg) = PRMSPIO_UART1COM3;
                  break;
               case BITSPIOFAR_UART1_COM4:
                  SelectParm1(*pmMsg) = PRMSPIO_UART1COM4;
                  break;
               default:
                  rc = ERRSPIO_UNKNOWN;
            } /* endswitch */
         } /* endif */
         break;

      case MSGSPIO_SET_UART1ADDRESS:
         /* Set UART 1 base address */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_UART1COM1:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART1_COM1,
                                      BITSPIOFAR_UART1);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_UART1COM2:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART1_COM2,
                                      BITSPIOFAR_UART1);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_UART1COM3:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART1_COM3,
                                      BITSPIOFAR_UART1);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_UART1COM4:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART1_COM4,
                                      BITSPIOFAR_UART1);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_UART2STATUS:
         /* Get UART 2 status */
         /*                   */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) & BITSPIOFER_UART2ENABLE)
            {
               BuildMsg(msg, MSGSPIO_GET_EXTERNALCLOCK);
               SendMsg(msg, oSuperioCtl);

               if(!(rc = SelectResult(msg)))
               {
                  SelectParm1(*pmMsg) = (SelectParm1(msg) ==
                                         PRMSPIO_CLOCKPOWERDOWN)
                                        ? (CPARAMETER)PRMSPIO_UART2DISABLE
                                        : (CPARAMETER)PRMSPIO_UART2ENABLE;
               } /* endif */
            }
            else
            {
               SelectParm1(*pmMsg) = PRMSPIO_UART2DISABLE;
            } /* endif */
         } /* endif */
         break;

      case MSGSPIO_SET_UART2STATUS:
         /* Set UART 2 status */
         /*                   */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_UART2ENABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_UART2ENABLE,
                                      BITSPIOFER_UART2ENABLE);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm1(msg, MSGSPIO_SET_EXTERNALCLOCK,
                                         PRMSPIO_CLOCKPOWERUP);
                  SendMsg(msg, oSuperioCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            case PRMSPIO_UART2DISABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_UART2ENABLE);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm1(msg, MSGSPIO_SET_EXTERNALCLOCK,
                                         PRMSPIO_CLOCKPOWERDOWN);
                  SendMsg(msg, oSuperioCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_UART2ADDRESS:
         /* Get UART 2 base address */
         /*                         */
         BuildMsg(msg, MSGSPIO_READ_FAR);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            switch(SelectParm1(msg) & BITSPIOFAR_UART2)
            {
               case BITSPIOFAR_UART2_COM1:
                  SelectParm1(*pmMsg) = PRMSPIO_UART2COM1;
                  break;
               case BITSPIOFAR_UART2_COM2:
                  SelectParm1(*pmMsg) = PRMSPIO_UART2COM2;
                  break;
               case BITSPIOFAR_UART2_COM3:
                  SelectParm1(*pmMsg) = PRMSPIO_UART2COM3;
                  break;
               case BITSPIOFAR_UART2_COM4:
                  SelectParm1(*pmMsg) = PRMSPIO_UART2COM4;
                  break;
               default:
                  rc = ERRSPIO_UNKNOWN;
            } /* endswitch */
         } /* endif */
         break;

      case MSGSPIO_SET_UART2ADDRESS:
         /* Set UART 2 base address */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_UART2COM1:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART2_COM1,
                                      BITSPIOFAR_UART2);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_UART2COM2:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART2_COM2,
                                      BITSPIOFAR_UART2);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_UART2COM3:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART2_COM3,
                                      BITSPIOFAR_UART2);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_UART2COM4:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FAR,
                                      BITSPIOFAR_UART2_COM4,
                                      BITSPIOFAR_UART2);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGSPIO_GET_FDCSTATUS:
         /* Get FDC status */
         /*                */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) & BITSPIOFER_FDCENABLE)
            {
               BuildMsg(msg, MSGSPIO_GET_EXTERNALCLOCK);
               SendMsg(msg, oSuperioCtl);

               if(!(rc = SelectResult(msg)))
               {
                  SelectParm1(*pmMsg) = (SelectParm1(msg) ==
                                         PRMSPIO_CLOCKPOWERDOWN)
                                        ? (CPARAMETER)PRMSPIO_FDCDISABLE
                                        : (CPARAMETER)PRMSPIO_FDCENABLE;
               } /* endif */
            }
            else
            {
               SelectParm1(*pmMsg) = PRMSPIO_FDCDISABLE;
            } /* endif */
         } /* endif */
         break;

      case MSGSPIO_SET_FDCSTATUS:
         /* Set FDC status */
         /*                */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_FDCENABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_FDCENABLE,
                                      BITSPIOFER_FDCENABLE);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm1(msg, MSGSPIO_SET_EXTERNALCLOCK,
                                         PRMSPIO_CLOCKPOWERUP);
                  SendMsg(msg, oSuperioCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            case PRMSPIO_FDCDISABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_FDCENABLE);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm1(msg, MSGSPIO_SET_EXTERNALCLOCK,
                                         PRMSPIO_CLOCKPOWERDOWN);
                  SendMsg(msg, oSuperioCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_FDCADDRESS:
         /* Get FDC base address */
         /*                      */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITSPIOFER_SELECTSECONDARYFDC)
                                  ? (CPARAMETER)PRMSPIO_FDCSECONDARY
                                  : (CPARAMETER)PRMSPIO_FDCPRIMARY;
         } /* endif */
         break;

      case MSGSPIO_SET_FDCADDRESS:
         /* Set FDC base address */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_FDCPRIMARY:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_SELECTSECONDARYFDC);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_FDCSECONDARYSET:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_SELECTSECONDARYFDC,
                                      BITSPIOFER_SELECTSECONDARYFDC);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_IDESTATUS:
         /* Get IDE status */
         /*                */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITSPIOFER_IDEENABLE)
                                  ? (CPARAMETER)PRMSPIO_IDEENABLE
                                  : (CPARAMETER)PRMSPIO_IDEDISABLE;
         } /* endif */
         break;

      case MSGSPIO_SET_IDESTATUS:
         /* Set IDE status */
         /*                */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_IDEENABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_IDEENABLE,
                                      BITSPIOFER_IDEENABLE);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_IDEDISABLE:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_IDEENABLE);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_GET_IDEADDRESS:
         /* Get IDE base address */
         /*                      */
         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITSPIOFER_SELECTSECONDARYIDE)
                                  ? (CPARAMETER)PRMSPIO_IDESECONDARY
                                  : (CPARAMETER)PRMSPIO_IDEPRIMARY;
         } /* endif */
         break;

      case MSGSPIO_SET_IDEADDRESS:
         /* Set IDE base address */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_IDEPRIMARY:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      0,
                                      BITSPIOFER_SELECTSECONDARYIDE);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            case PRMSPIO_IDESECONDARY:
               BuildMsgWithParm2(msg, MSGSPIO_MODIFY_FER,
                                      BITSPIOFER_SELECTSECONDARYIDE,
                                      BITSPIOFER_SELECTSECONDARYIDE);
               SendMsg(msg, oSuperioIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGSPIO_GET_EXTERNALCLOCK:
         /* Get external clock power */
         /*                          */
         BuildMsg(msg, MSGSPIO_READ_PTR);
         SendMsg(msg, oSuperioIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITSPIOPTR_POWERDOWN)
                                  ? (CPARAMETER)PRMSPIO_CLOCKPOWERDOWN
                                  : (CPARAMETER)PRMSPIO_CLOCKPOWERUP;
         } /* endif */
         break;

      case MSGSPIO_SET_EXTERNALCLOCK:
         /* Set external clock power */
         /*                          */
         switch(SelectParm1(*pmMsg))
         {
            case PRMSPIO_CLOCKPOWERUP:
               /* Power-up the external clock */
               /*                             */
               BuildMsg(msg, MSGSPIO_READ_PTR);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  if(SelectParm1(msg) & BITSPIOPTR_POWERDOWN)
                  {
                     BuildMsgWithParm2(msg, MSGSPIO_MODIFY_PTR,
                                            0,
                                            BITSPIOPTR_POWERDOWN |
                                            BITSPIOPTR_XTALCLKPOWERDOWN);
                     SendMsg(msg, oSuperioIo);
                     WaitTime(WAITTIME_STABILIZATION);

                     rc = SelectResult(msg);
                  } /* endif */
               } /* endif */
               break;
            case PRMSPIO_CLOCKPOWERDOWN:
               /* Power-down the external clock */
               /*                               */
               BuildMsg(msg, MSGSPIO_READ_PTR);
               SendMsg(msg, oSuperioIo);

               if(!(rc = SelectResult(msg)))
               {
                  if(!(SelectParm1(msg) & BITSPIOPTR_POWERDOWN))
                  {
                     BuildMsg(msg, MSGSPIO_READ_FER);
                     SendMsg(msg, oSuperioIo);

                     if(!(rc = SelectResult(msg)))
                     {
                        if(!(SelectParm1(msg) & (BITSPIOFER_UART1ENABLE |
                                                 BITSPIOFER_UART2ENABLE |
                                                 BITSPIOFER_FDCENABLE   )))
                        {
                           BuildMsgWithParm2(msg, MSGSPIO_MODIFY_PTR,
                                                  BITSPIOPTR_POWERDOWN |
                                                  BITSPIOPTR_XTALCLKPOWERDOWN,
                                                  BITSPIOPTR_POWERDOWN |
                                                  BITSPIOPTR_XTALCLKPOWERDOWN);
                           SendMsg(msg, oSuperioIo);

                           rc = SelectResult(msg);
                        } /* endif */
                     } /* endif */
                  } /* endif */
               } /* endif */
               break;
            default:
               rc = ERRSPIO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGSPIO_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRSPIO_NOERROR;

         BuildMsg(msg, MSGSPIO_GET_INDEX);
         SendMsg(msg, oSuperioIo);
         if(!SelectResult(msg))
         {
            pspioctl->chIndex = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSPIO_READ_FER);
         SendMsg(msg, oSuperioIo);
         if(!SelectResult(msg))
         {
            pspioctl->chFer = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSPIO_READ_FAR);
         SendMsg(msg, oSuperioIo);
         if(!SelectResult(msg))
         {
            pspioctl->chFar = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSPIO_READ_PTR);
         SendMsg(msg, oSuperioIo);
         if(!SelectResult(msg))
         {
            pspioctl->chPtr = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSPIO_READ_FCR);
         SendMsg(msg, oSuperioIo);
         if(!SelectResult(msg))
         {
            pspioctl->chFcr = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSPIO_READ_PCR);
         SendMsg(msg, oSuperioIo);
         if(!SelectResult(msg))
         {
            pspioctl->chPcr = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGSPIO_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRSPIO_NOERROR;

         BuildMsgWithParm1(msg, MSGSPIO_WRITE_FER,
                                pspioctl->chFer);
         SendMsg(msg, oSuperioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSPIO_WRITE_FAR,
                                pspioctl->chFar);
         SendMsg(msg, oSuperioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSPIO_WRITE_PTR,
                                pspioctl->chPtr);
         SendMsg(msg, oSuperioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSPIO_WRITE_FCR,
                                pspioctl->chFcr);
         SendMsg(msg, oSuperioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSPIO_WRITE_PCR,
                                pspioctl->chPcr);
         SendMsg(msg, oSuperioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSPIO_SET_INDEX,
                                pspioctl->chIndex);
         SendMsg(msg, oSuperioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oSuperioIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
