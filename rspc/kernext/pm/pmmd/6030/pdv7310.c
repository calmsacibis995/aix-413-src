static char sccsid[] = "@(#)91  1.2  src/rspc/kernext/pm/pmmd/6030/pdv7310.c, pwrsysx, rspc41J, 9517A_b 4/25/95 04:18:25";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: V7310 (Ascii Video Capture Controller) Chip Device
 *              Control Routines
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
/* * Module Name: PDV7310.C                                                * */
/* * Description: V7310 (Ascii Video Capture Controller) Chip Device       * */
/* *              Control Routines                                         * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDV7310.H/PDPCIBUS.H files should  * */
/* *              be included in this file                                 * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdv7310.h"
#include "pdpcibus.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *       V7310 Data Definition       * */
/* ************************************* */
/* --------------------------------------------------- */
/*  Power management register index/data port address  */
/* --------------------------------------------------- */
#define  PORTV73_BASE                     (0x30000000L)
#define  PORTV73_BASENULL                 (0xFFFFFFFFL)
                                                       /* V7310 base address */
                                                      /* & base null address */
#define  PORTV73_BASEMASK                 (0xFFFFFFFCL) /* Base address mask */
                                                 /* Offset from base address */
#define  PORTV73_COMMAND                  (0x04)     /* PCI command register */
#define  PORTV73_INDEX                    (0x00)           /* Index register */
#define  PORTV73_DATA                     (0x04)            /* Data register */
#define  PORTV73_SPACESIZE                (8)     /* Port address space size */

/* ------------------------------------------ */
/*  PCI command register (PCICMD) bit assign  */
/* ------------------------------------------ */
#define  BITV73PCICMD_IOAE                (0x0001)
                                                 /* (Bit-0) I/O space access */
#define  BITV73PCICMD_MAE                 (0x0002)
                                              /* (Bit-1) Memory space access */
#define  BITV73PCICMD_BME                 (0x0004)
                                             /* (Bit-2) Bus master operation */
#define  BITV73PCICMD_SPCE                (0x0008)
                                      /* (Bit-3) Operation for special cycle */
#define  BITV73PCICMD_MWIE                (0x0010)
                                      /* (Bit-4) Memory write and invalidate */
#define  BITV73PCICMD_RESERVED5           (0x0020)
                                                         /* (Bit-5) Reserved */
#define  BITV73PCICMD_PERRE               (0x0040)
                                            /* (Bit-6) Parity error reaction */
#define  BITV73PCICMD_ADSE                (0x0080)
                                     /* (Bit-7) Address/Data stepping enable */
#define  BITV73PCICMD_SERRE               (0x0100)
                                                     /* (Bit-8) SERR# enable */
#define  BITV73PCICMD_FBTBE               (0x0200)
                     /* (Bit-9) Fast back to back transaction command enable */
#define  BITV73PCICMD_RESERVED10          (0x0400)
                                                        /* (Bit-10) Reserved */
#define  BITV73PCICMD_RESERVED11          (0x0800)
                                                        /* (Bit-11) Reserved */
#define  BITV73PCICMD_RESERVED12          (0x1000)
                                                        /* (Bit-12) Reserved */
#define  BITV73PCICMD_RESERVED13          (0x2000)
                                                        /* (Bit-13) Reserved */
#define  BITV73PCICMD_RESERVED14          (0x4000)
                                                        /* (Bit-14) Reserved */
#define  BITV73PCICMD_RESERVED15          (0x8000)
                                                        /* (Bit-15) Reserved */

/* ------------------------------------- */
/*  Index data to index V7310 registers  */
/* ------------------------------------- */
#define  INDEXV73_FNCSEL                  (0x90)
                                                 /* Function select register */
#define  INDEXV73_PWRCTL                  (0x93)
                                                   /* Power control register */
#define  INDEXV73_GIOCTL0                 (0xB0)
                                           /* General I/O control 0 register */
#define  INDEXV73_GOTDAT0                 (0xB4)
                                           /* General output data 0 register */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    V7310 Controller Definition    * */
/* ************************************* */
/* ----------------- */
/*  V7310 I/O class  */
/* ----------------- */
VOID  V7310Io(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj);               /* Pointer to Common object */

/* --------------------- */
/*  V7310 control class  */
/* --------------------- */
VOID  V7310Ctl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *           V7310 Objects           * */
/* ************************************* */
/* ----------------- */
/*  V7310 I/O class  */
/* ----------------- */
OBJV7310IO  oV7310Io  = {(POBJCTL)V7310Io, PORTV73_BASENULL};

/* --------------------- */
/*  V7310 control class  */
/* --------------------- */
OBJV7310CTL oV7310Ctl = {(POBJCTL)V7310Ctl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* **************************************** */
/* *         V7310 I/O Controller         * */
/* **************************************** */
/* *                                      * */
/* * - Initialize                         * */
/* *     [ Input ]                        * */
/* *       Message    = MSGV73_INITIALIZE * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Get index register                 * */
/* *     [ Input ]                        * */
/* *       Message    = MSGV73_GET_INDEX  * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = index value       * */
/* *                                      * */
/* * - Set index register                 * */
/* *     [ Input ]                        * */
/* *       Message    = MSGV73_SET_INDEX  * */
/* *       Parameter1 = index value       * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read register                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGV73_READ_xxx   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write register                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGV73_WRITE_xxx  * */
/* *       Parameter1 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read & Modify register             * */
/* *     [ Input ]                        * */
/* *       Message    = MSGV73_MODIFY_xxx * */
/* *       Parameter1 = set bits value    * */
/* *       Parameter2 = mask bits value   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* **************************************** */
VOID  V7310Io(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj)                /* Pointer to Common object */
{
   MSGPCI      mPci;
   BYTE        index, sindex;
   CRESULT     rc;
   POBJV7310IO pv73io;
   DWORD       indexport, dataport, baseport;

   /* Get pointer to V7310 I/O object */
   /*                                 */
   pv73io = (POBJV7310IO)poObj;

   /* Check if completing for initialization */
   /*                                        */
   if((pv73io->BaseAddress   != PORTV73_BASENULL) ||
      (SelectMessage(*pmMsg) == MSGV73_INITIALIZE)  )
   {
      /* Get index/data port address */
      /*                             */
      baseport = (pv73io->BaseAddress < PORTIOBASE_SIZE)
                  ? pv73io->BaseAddress + GetIoBaseAddress()
                  : GetIoSpace(pv73io->BaseAddress, PORTV73_SPACESIZE);

      indexport = baseport + PORTV73_INDEX;
      dataport  = baseport + PORTV73_DATA;

      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGV73_INITIALIZE:
            /* Initialize */
            /*            */
            BuildMsg(mPci, MSGPCI_QUERY_DEVICE);
            SendMsg(mPci, oPciCaptureCtl);

            if(!(rc = SelectResult(mPci)))
            {
               BuildMsg(mPci, MSGPCI_GET_BASEADDRESS1);
               SendMsg(mPci, oPciCaptureCtl);

               if(!(rc = SelectResult(mPci)))
               {
                  pv73io->BaseAddress = SelectPciParm1(mPci) & PORTV73_BASEMASK;

                  BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                          PORTV73_COMMAND);
                  BuildPciParm1(mPci, BITV73PCICMD_IOAE |
                                      BITV73PCICMD_BME  );
                  SendMsg(mPci, oPciCaptureCtl);

                  if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                  {
                     rc = ERRV73_CANNOT_PCIACCESS;
                  } /* endif */
               }
               else
               {
                  rc = ERRV73_CANNOT_INITIALIZE;
               } /* endif */
            }
            else
            {
               rc = ERRV73_CANNOT_INITIALIZE;
            } /* endif */
            break;

         case MSGV73_GET_INDEX:
            /* Get index register */
            /*                    */
            SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(indexport);
            rc = ERRV73_NOERROR;
            break;

         case MSGV73_SET_INDEX:
            /* Set index register */
            /*                    */
            WriteControllerByte(indexport, (BYTE)SelectParm1(*pmMsg));
            rc = ERRV73_NOERROR;
            break;

         default:
            /* Get index register */
            /*                    */
            switch(SelectMessage(*pmMsg))
            {
               case MSGV73_READ_FNCSEL:
               case MSGV73_WRITE_FNCSEL:
               case MSGV73_MODIFY_FNCSEL:
                  index = INDEXV73_FNCSEL;
                  rc    = ERRV73_NOERROR;
                  break;
               case MSGV73_READ_PWRCTL:
               case MSGV73_WRITE_PWRCTL:
               case MSGV73_MODIFY_PWRCTL:
                  index = INDEXV73_PWRCTL;
                  rc    = ERRV73_NOERROR;
                  break;
               case MSGV73_READ_GIOCTL0:
               case MSGV73_WRITE_GIOCTL0:
               case MSGV73_MODIFY_GIOCTL0:
                  index = INDEXV73_GIOCTL0;
                  rc    = ERRV73_NOERROR;
                  break;
               case MSGV73_READ_GOTDAT0:
               case MSGV73_WRITE_GOTDAT0:
               case MSGV73_MODIFY_GOTDAT0:
                  index = INDEXV73_GOTDAT0;
                  rc    = ERRV73_NOERROR;
                  break;
               default:
                  rc = ERRV73_INVALID_MESSAGE;
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
                  case MSGV73_READ_FNCSEL:
                  case MSGV73_READ_PWRCTL:
                  case MSGV73_READ_GIOCTL0:
                  case MSGV73_READ_GOTDAT0:
                     WriteControllerByte(indexport, index);
                     SelectParm1(*pmMsg) =
                                       (CPARAMETER)ReadControllerByte(dataport);
                     break;
                  case MSGV73_WRITE_FNCSEL:
                  case MSGV73_WRITE_PWRCTL:
                  case MSGV73_WRITE_GIOCTL0:
                  case MSGV73_WRITE_GOTDAT0:
                     WriteControllerByte(indexport, index);
                     WriteControllerByte(dataport,  (BYTE)SelectParm1(*pmMsg));
                     break;
                  case MSGV73_MODIFY_FNCSEL:
                  case MSGV73_MODIFY_PWRCTL:
                  case MSGV73_MODIFY_GIOCTL0:
                  case MSGV73_MODIFY_GOTDAT0:
                     WriteControllerByte(indexport, index);
                     WriteControllerByte(dataport,
                                      (BYTE)((ReadControllerByte(dataport) &
                                              ~SelectParm2(*pmMsg)         ) |
                                             (SelectParm1(*pmMsg)          &
                                              SelectParm2(*pmMsg)          ) ));
                     break;
                  default:
                     rc = ERRV73_INVALID_MESSAGE;
               } /* endswitch */

               /* Restore index register */
               /*                        */
               IoDelay();
               WriteControllerByte(indexport, sindex);
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERRV73_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ********************************************* */
/* *         V7310 Control Controller          * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGV73_INITIALIZECTL   * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Save device context                     * */
/* *     [ Input ]                             * */
/* *       Message    = MSGV73_SAVE_CONTEXT    * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Restore device context                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGV73_RESTORE_CONTEXT * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  V7310Ctl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   MSGV7310       msg;
   CRESULT        rc;
   POBJV7310CTL   pv73ctl;

   /* Get pointer to V7310 control object */
   /*                                     */
   pv73ctl = (POBJV7310CTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGV73_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGV73_INITIALIZE);
         SendMsg(msg, oV7310Io);

         rc = SelectResult(msg);
         break;

      case MSGV73_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRV73_NOERROR;

         BuildMsg(msg, MSGV73_GET_INDEX);
         SendMsg(msg, oV7310Io);
         if(!SelectResult(msg))
         {
            pv73ctl->chIndex = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGV73_READ_PWRCTL);
         SendMsg(msg, oV7310Io);
         if(!SelectResult(msg))
         {
            pv73ctl->chPwrctl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGV73_READ_GOTDAT0);
         SendMsg(msg, oV7310Io);
         if(!SelectResult(msg))
         {
            pv73ctl->chGotdat0 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGV73_READ_GIOCTL0);
         SendMsg(msg, oV7310Io);
         if(!SelectResult(msg))
         {
            pv73ctl->chGioctl0 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGV73_READ_FNCSEL);
         SendMsg(msg, oV7310Io);
         if(!SelectResult(msg))
         {
            pv73ctl->chFncsel = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGV73_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRV73_NOERROR;

         BuildMsgWithParm1(msg, MSGV73_WRITE_PWRCTL,
                                pv73ctl->chPwrctl);
         SendMsg(msg, oV7310Io);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGV73_WRITE_GOTDAT0,
                                pv73ctl->chGotdat0);
         SendMsg(msg, oV7310Io);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGV73_WRITE_GIOCTL0,
                                pv73ctl->chGioctl0);
         SendMsg(msg, oV7310Io);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGV73_WRITE_FNCSEL,
                                pv73ctl->chFncsel);
         SendMsg(msg, oV7310Io);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGV73_SET_INDEX,
                                pv73ctl->chIndex);
         SendMsg(msg, oV7310Io);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oV7310Io);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
