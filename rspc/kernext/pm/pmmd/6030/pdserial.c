/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Serial Port Device Control Routines
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
/* * Module Name: PDSERIAL.C                                               * */
/* * Description: Serial Port Device Control Routines                      * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDSERIAL.H/PDSUPRIO.H/PDCARERA.H   * */
/* *              files should be included in this file                    * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdserial.h"
#include "pdsuprio.h"
#include "pdcarera.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      Serial Data Definition       * */
/* ************************************* */
/* --------------------- */
/*  Serial port address  */
/* --------------------- */
#define  PORTSER_BASE                     (0x000003F8L)
#define  PORTSER_BASENULL                 (0xFFFFFFFFL)
                                                      /* Serial base address */
                                                      /* & base null address */

/* -------------------------------------- */
/*  Index data to index Serial registers  */
/* -------------------------------------- */
#define  INDEXSER_RECEIVER                (0x00)
                                                 /* Receiver buffer register */
#define  INDEXSER_TRANSMITTER             (0x00)
                                             /* Transmitter holding register */
#define  INDEXSER_INTERRUPTENABLE         (0x01)
                                                /* Interrupt enable register */
#define  INDEXSER_INTERRUPTID             (0x02)
                                        /* Interrupt identification register */
#define  INDEXSER_FIFOCONTROL             (0x02)
                                                    /* FIFO control register */
#define  INDEXSER_LINECONTROL             (0x03)
                                                    /* Line control register */
#define  INDEXSER_MODEMCONTROL            (0x04)
                                                   /* MODEM control register */
#define  INDEXSER_LINESTATUS              (0x05)
                                                     /* Line status register */
#define  INDEXSER_MODEMSTATUS             (0x06)
                                                    /* MODEM status register */
#define  INDEXSER_SCRATCHPAD              (0x07)
                                                     /* Scratch pad register */
#define  INDEXSER_DIVISORLATCHLSB         (0x00)
                                               /* Divisor latch LSB register */
#define  INDEXSER_DIVISORLATCHMSB         (0x01)
                                               /* Divisor latch MSB register */

/* ------------------------------------------------ */
/*  FIFO control register (FIFOCONTROL) bit assign  */
/* ------------------------------------------------ */
#define  BITSERFIFOCONTROL_FIFOENABLE     (0x01)
                                 /* (Bit-0) FIFO mode enable                 */
#define  BITSERFIFOCONTROL_RCVFIFORESET   (0x02)
                                 /* (Bit-1) Receiver FIFO register reset     */
#define  BITSERFIFOCONTROL_XMITFIFORESET  (0x04)
                                 /* (Bit-2) Transmitter FIFO register reset  */
#define  BITSERFIFOCONTROL_RESERVED3      (0x08)
                                 /* (Bit-3) Reserved                         */
#define  BITSERFIFOCONTROL_RESERVED4      (0x10)
                                 /* (Bit-4) Reserved                         */
#define  BITSERFIFOCONTROL_RESERVED5      (0x20)
                                 /* (Bit-5) Reserved                         */
#define  BITSERFIFOCONTROL_RCVFIFOTRIGGER (0xC0)
                                 /* (Bit-6,7) Receiver FIFO register trigger */

/* ------------------------------------------------ */
/*  Line control register (LINECONTROL) bit assign  */
/* ------------------------------------------------ */
#define  BITSERLINECONTROL_WORDLENGTH     (0x03)
                                             /* (Bit-0,1) Word length select */
#define  BITSERLINECONTROL_WORDLENGTH5    (0x00)
                                             /*           5 bits             */
#define  BITSERLINECONTROL_WORDLENGTH6    (0x01)
                                             /*           6 bits             */
#define  BITSERLINECONTROL_WORDLENGTH7    (0x02)
                                             /*           7 bits             */
#define  BITSERLINECONTROL_WORDLENGTH8    (0x03)
                                             /*           8 bits             */
#define  BITSERLINECONTROL_STOPBIT        (0x04)
                                             /* (Bit-2) Number of stop bits  */
#define  BITSERLINECONTROL_STOPBIT1       (0x00)
                                             /*         1 bit                */
#define  BITSERLINECONTROL_STOPBIT2       (0x04)
                                             /*         2 bit                */
#define  BITSERLINECONTROL_PARITYENABLE   (0x08)
                                             /* (Bit-3) Parity enable        */
#define  BITSERLINECONTROL_EVENPARITY     (0x10)
                                             /* (Bit-4) Even parity select   */
#define  BITSERLINECONTROL_STICKPARITY    (0x20)
                                             /* (Bit-5) Stick parity         */
#define  BITSERLINECONTROL_SETBREAK       (0x40)
                                             /* (Bit-6) Set break            */
#define  BITSERLINECONTROL_DIVISORACC     (0x80)
                                             /* (Bit-7) Divisor latch access */

/* -------------------------------------------------- */
/*  MODEM control register (MODEMCONTROL) bit assign  */
/* -------------------------------------------------- */
#define  BITSERMODEMCONTROL_DTR           (0x01)
                                        /* (Bit-0) Data terminal ready (DTR) */
#define  BITSERMODEMCONTROL_RTS           (0x02)
                                        /* (Bit-1) Request to send (RTS)     */
#define  BITSERMODEMCONTROL_OUT1          (0x04)
                                        /* (Bit-2) Out 1                     */
#define  BITSERMODEMCONTROL_OUT2          (0x08)
                                        /* (Bit-3) Out 2                     */
#define  BITSERMODEMCONTROL_LOOPTEST      (0x10)
                                        /* (Bit-4) Loop test                 */
#define  BITSERMODEMCONTROL_RESERVED5     (0x20)
                                        /* (Bit-5) Reserved                  */
#define  BITSERMODEMCONTROL_RESERVED6     (0x40)
                                        /* (Bit-6) Reserved                  */
#define  BITSERMODEMCONTROL_RESERVED7     (0x80)
                                        /* (Bit-7) Reserved                  */

/* ----------------------------------------------- */
/*  Serial identification (Serial-A/B) definition  */
/* ----------------------------------------------- */
#define  SERIALID_SERIALA                 (1)                 /* Serial-A ID */
#define  SERIALID_SERIALB                 (2)                 /* Serial-A ID */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  CONTROLFLAG_SERIALSHADOW         (FALSE)
                                      /* Serial shadow register control flag */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *   Serial Controller Definition    * */
/* ************************************* */
/* ------------------ */
/*  Serial I/O class  */
/* ------------------ */
VOID  SerialIo(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */

/* ---------------------- */
/*  Serial control class  */
/* ---------------------- */
VOID  SerialCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *          Serial Objects           * */
/* ************************************* */
/* ------------------ */
/*  Serial I/O class  */
/* ------------------ */
OBJSERIALIO    oSerialAIo  = {(POBJCTL)SerialIo, PORTSER_BASENULL,
                                                 SERIALID_SERIALA};
OBJSERIALIO    oSerialBIo  = {(POBJCTL)SerialIo, PORTSER_BASENULL,
                                                 SERIALID_SERIALB};

/* ---------------------- */
/*  Serial control class  */
/* ---------------------- */
OBJSERIALCTL   oSerialACtl = {(POBJCTL)SerialCtl, &oSerialAIo,
                                                  SERIALID_SERIALA,
                                                  0x00, 0xC7, 0x00, 0x00,
                                                  0x00, 0x00, 0x00};
OBJSERIALCTL   oSerialBCtl = {(POBJCTL)SerialCtl, &oSerialBIo,
                                                  SERIALID_SERIALB,
                                                  0x00, 0xC7, 0x00, 0x00,
                                                  0x00, 0x00, 0x00};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* **************************************** */
/* *        Serial I/O Controller         * */
/* **************************************** */
/* *                                      * */
/* * - Initialize                         * */
/* *     [ Input ]                        * */
/* *       Message    = MSGSER_INITIALIZE * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read register                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGSER_READ_xxx   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write register                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGSER_WRITE_xxx  * */
/* *       Parameter1 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read & Modify register             * */
/* *     [ Input ]                        * */
/* *       Message    = MSGSER_MODIFY_xxx * */
/* *       Parameter1 = set bits value    * */
/* *       Parameter2 = mask bits value   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* **************************************** */
VOID  SerialIo(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   MSGSUPERIO     mSpio;
   UINT           index;
   BYTE           linectl;
   CRESULT        rc;
   POBJSERIALIO   pserio;
   DWORD          baseport;

   /* Get pointer to Serial I/O object */
   /*                                  */
   pserio = (POBJSERIALIO)poObj;

   /* Check if completing for initialization */
   /*                                        */
   if((pserio->BaseAddress   != PORTSER_BASENULL) ||
      (SelectMessage(*pmMsg) == MSGSER_INITIALIZE)  )
   {
      /* Get base-port address */
      /*                       */
      baseport = pserio->BaseAddress + GetIsaBaseAddress();

      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGSER_INITIALIZE:
            /* Initialize */
            /*            */
            switch(pserio->chSerialId)
            {
               case SERIALID_SERIALA:
                  BuildMsg(mSpio, MSGSPIO_GET_UART1STATUS);
                  SendMsg(mSpio, oSuperioCtl);
                  if(!SelectResult(mSpio) &&
                     (SelectParm1(mSpio) == PRMSPIO_UART1ENABLE))
                  {
                     BuildMsg(mSpio, MSGSPIO_GET_UART1ADDRESS);
                     SendMsg(mSpio, oSuperioCtl);
                  }
                  else
                  {
                     SelectResult(mSpio) = ERRSER_CANNOT_INITIALIZE;
                  } /* endif */
                  break;
               case SERIALID_SERIALB:
                  BuildMsg(mSpio, MSGSPIO_GET_UART2STATUS);
                  SendMsg(mSpio, oSuperioCtl);
                  if(!SelectResult(mSpio) &&
                     (SelectParm1(mSpio) == PRMSPIO_UART2ENABLE))
                  {
                     BuildMsg(mSpio, MSGSPIO_GET_UART2ADDRESS);
                     SendMsg(mSpio, oSuperioCtl);
                  }
                  else
                  {
                     SelectResult(mSpio) = ERRSER_CANNOT_INITIALIZE;
                  } /* endif */
                  break;
               default:
                  SelectResult(mSpio) = ERRSER_CANNOT_INITIALIZE;
            } /* endswitch */

            if(!(rc = SelectResult(mSpio)))
            {
               pserio->BaseAddress = SelectParm1(mSpio);
            }
            else
            {
               rc = ERRSER_CANNOT_INITIALIZE;
            } /* endif */
            break;

         default:
            /* Get index register */
            /*                    */
            switch(SelectMessage(*pmMsg))
            {
               case MSGSER_READ_RECEIVER:
                  index = INDEXSER_RECEIVER;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_WRITE_TRANSMITTER:
                  index = INDEXSER_TRANSMITTER;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_INTERRUPTENABLE:
               case MSGSER_WRITE_INTERRUPTENABLE:
                  index = INDEXSER_INTERRUPTENABLE;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_INTERRUPTID:
                  index = INDEXSER_INTERRUPTID;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_WRITE_FIFOCONTROL:
                  index = INDEXSER_FIFOCONTROL;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_LINECONTROL:
               case MSGSER_WRITE_LINECONTROL:
               case MSGSER_MODIFY_LINECONTROL:
                  index = INDEXSER_LINECONTROL;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_MODEMCONTROL:
               case MSGSER_WRITE_MODEMCONTROL:
               case MSGSER_MODIFY_MODEMCONTROL:
                  index = INDEXSER_MODEMCONTROL;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_LINESTATUS:
                  index = INDEXSER_LINESTATUS;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_MODEMSTATUS:
               case MSGSER_WRITE_MODEMSTATUS:
                  index = INDEXSER_MODEMSTATUS;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_SCRATCHPAD:
               case MSGSER_WRITE_SCRATCHPAD:
                  index = INDEXSER_SCRATCHPAD;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_DIVISORLATCHLSB:
               case MSGSER_WRITE_DIVISORLATCHLSB:
                  index = INDEXSER_DIVISORLATCHLSB;
                  rc    = ERRSER_NOERROR;
                  break;
               case MSGSER_READ_DIVISORLATCHMSB:
               case MSGSER_WRITE_DIVISORLATCHMSB:
                  index = INDEXSER_DIVISORLATCHMSB;
                  rc    = ERRSER_NOERROR;
                  break;
               default:
                  rc = ERRSER_INVALID_MESSAGE;
            } /* endswitch */

            if(!rc)
            {
               /* Input/Output registers */
               /*                        */
               switch(SelectMessage(*pmMsg))
               {
                  case MSGSER_READ_LINECONTROL:
                     SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + index);
                     break;
                  case MSGSER_READ_RECEIVER:
                  case MSGSER_READ_INTERRUPTENABLE:
                  case MSGSER_READ_INTERRUPTID:
                  case MSGSER_READ_MODEMCONTROL:
                  case MSGSER_READ_LINESTATUS:
                  case MSGSER_READ_MODEMSTATUS:
                  case MSGSER_READ_SCRATCHPAD:
                     linectl =
                            ReadControllerByte(baseport + INDEXSER_LINECONTROL);
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl &
                                         ~BITSERLINECONTROL_DIVISORACC);

                     SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + index);

                     IoDelay();
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl);
                     break;
                  case MSGSER_READ_DIVISORLATCHLSB:
                  case MSGSER_READ_DIVISORLATCHMSB:
                     linectl =
                            ReadControllerByte(baseport + INDEXSER_LINECONTROL);
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl |
                                         BITSERLINECONTROL_DIVISORACC);

                     SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + index);

                     IoDelay();
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl);
                     break;
                  case MSGSER_WRITE_LINECONTROL:
                     WriteControllerByte(baseport + index,
                                         (BYTE)SelectParm1(*pmMsg));
                     break;
                  case MSGSER_WRITE_TRANSMITTER:
                  case MSGSER_WRITE_INTERRUPTENABLE:
                  case MSGSER_WRITE_FIFOCONTROL:
                  case MSGSER_WRITE_MODEMCONTROL:
                  case MSGSER_WRITE_MODEMSTATUS:
                  case MSGSER_WRITE_SCRATCHPAD:
                     linectl =
                            ReadControllerByte(baseport + INDEXSER_LINECONTROL);
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl &
                                         ~BITSERLINECONTROL_DIVISORACC);

                     WriteControllerByte(baseport + index,
                                         (BYTE)SelectParm1(*pmMsg));

                     IoDelay();
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl);
                     break;
                  case MSGSER_WRITE_DIVISORLATCHLSB:
                  case MSGSER_WRITE_DIVISORLATCHMSB:
                     linectl =
                            ReadControllerByte(baseport + INDEXSER_LINECONTROL);
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl |
                                         BITSERLINECONTROL_DIVISORACC);

                     WriteControllerByte(baseport + index,
                                         (BYTE)SelectParm1(*pmMsg));

                     IoDelay();
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl);
                     break;
                  case MSGSER_MODIFY_LINECONTROL:
                     WriteControllerByte(baseport + index,
                              (BYTE)((ReadControllerByte(baseport + index) &
                                      ~SelectParm2(*pmMsg)                 ) |
                                     (SelectParm1(*pmMsg)                  &
                                      SelectParm2(*pmMsg)                  ) ));
                     break;
                  case MSGSER_MODIFY_MODEMCONTROL:
                     linectl =
                            ReadControllerByte(baseport + INDEXSER_LINECONTROL);
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                       linectl & ~BITSERLINECONTROL_DIVISORACC);

                     WriteControllerByte(baseport + index,
                              (BYTE)((ReadControllerByte(baseport + index) &
                                      ~SelectParm2(*pmMsg)                 ) |
                                     (SelectParm1(*pmMsg)                  &
                                      SelectParm2(*pmMsg)                  ) ));

                     IoDelay();
                     WriteControllerByte(baseport + INDEXSER_LINECONTROL,
                                         linectl);
                     break;
                  default:
                     rc = ERRSER_INVALID_MESSAGE;
               } /* endswitch */
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERRSER_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ********************************************* */
/* *         Serial Control Controller         * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGSER_INITIALIZECTL   * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Save device context                     * */
/* *     [ Input ]                             * */
/* *       Message    = MSGSER_SAVE_CONTEXT    * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Restore device context                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGSER_RESTORE_CONTEXT * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  SerialCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   MSGSERIAL      msg;
   CRESULT        rc;
   POBJSERIALCTL  pserctl;
   POBJSERIALIO   pserio;
#if (CONTROLFLAG_SERIALSHADOW == TRUE)
   MSGCARRERA     mCarr;
#endif /* CONTROLFLAG_SERIALSHADOW */

   /* Get pointer to Serial control object */
   /*                                      */
   pserctl = (POBJSERIALCTL)poObj;

   /* Get pointer to Serial I/O object */
   /*                                  */
   pserio = pserctl->poSerialIo;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGSER_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGSER_INITIALIZE);
         SendMsg(msg, *pserio);

         rc = SelectResult(msg);
         break;

      case MSGSER_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRSER_NOERROR;

         BuildMsg(msg, MSGSER_READ_LINECONTROL);
         SendMsg(msg, *pserio);
         if(!SelectResult(msg))
         {
            pserctl->chLineControl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSER_READ_INTERRUPTENABLE);
         SendMsg(msg, *pserio);
         if(!SelectResult(msg))
         {
            pserctl->chInterruptEnable = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSER_READ_MODEMCONTROL);
         SendMsg(msg, *pserio);
         if(!SelectResult(msg))
         {
            pserctl->chMODEMControl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSER_READ_SCRATCHPAD);
         SendMsg(msg, *pserio);
         if(!SelectResult(msg))
         {
            pserctl->chScratchPad = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSER_READ_DIVISORLATCHLSB);
         SendMsg(msg, *pserio);
         if(!SelectResult(msg))
         {
            pserctl->chDivisorLatchLSB = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSER_READ_DIVISORLATCHMSB);
         SendMsg(msg, *pserio);
         if(!SelectResult(msg))
         {
            pserctl->chDivisorLatchMSB = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

#if (CONTROLFLAG_SERIALSHADOW == TRUE)
         BuildMsg(mCarr, (pserctl->chSerialId == SERIALID_SERIALA)
                         ? MSGCARR_GET_SERIAL1REGISTERS
                         : MSGCARR_GET_SERIAL2REGISTERS);
         SendMsg(mCarr, oCarreraCtl);
         if(!SelectResult(mCarr))
         {
            pserctl->chFIFOControl = (BYTE)SelectParm1(mCarr);
         }
         else
         {
            rc = ERRSER_CANNOT_READREGISTER;
         } /* endif */
#endif /* CONTROLFLAG_SERIALSHADOW */
         break;

      case MSGSER_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRSER_NOERROR;

         BuildMsgWithParm1(msg, MSGSER_WRITE_DIVISORLATCHLSB,
                                pserctl->chDivisorLatchLSB);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSER_WRITE_DIVISORLATCHMSB,
                                pserctl->chDivisorLatchMSB);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSER_WRITE_INTERRUPTENABLE,
                                pserctl->chInterruptEnable);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSER_WRITE_FIFOCONTROL,
                                pserctl->chFIFOControl         |
                                BITSERFIFOCONTROL_RCVFIFORESET |
                                BITSERFIFOCONTROL_XMITFIFORESET);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSER_WRITE_MODEMCONTROL,
                                pserctl->chMODEMControl);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSER_WRITE_SCRATCHPAD,
                                pserctl->chScratchPad);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSER_WRITE_LINECONTROL,
                                pserctl->chLineControl);
         SendMsg(msg, *pserio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSER_READ_LINESTATUS);
         SendMsg(msg, *pserio);
         BuildMsg(msg, MSGSER_READ_MODEMSTATUS);
         SendMsg(msg, *pserio);
         BuildMsg(msg, MSGSER_READ_RECEIVER);
         SendMsg(msg, *pserio);
         break;

      default:
         SendMsg(*pmMsg, *pserio);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
