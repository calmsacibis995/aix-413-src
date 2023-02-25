/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Programmable Interval Timer (PIT, 8254) Device Control
 *              Routines
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
/* * Module Name: PDPIT.C                                                  * */
/* * Description: Programmable Interval Timer (PIT, 8254) Device Control   * */
/* *              Routines                                                 * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDPIT.H/PDCARERA.H files should be * */
/* *              included in this file                                    * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdpit.h"
#include "pdcarera.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *        PIT Data Definition        * */
/* ************************************* */
/* --------------------------- */
/*  PIT register port address  */
/* --------------------------- */
#define  PORTPIT_BASE                     (0x00000040L)  /* PIT base address */
#define  PORTPIT_COUNTER0                 (PORTPIT_BASE + 0)
                                                       /* Counter 0 register */
#define  PORTPIT_COUNTER1                 (PORTPIT_BASE + 1)
                                                       /* Counter 1 register */
#define  PORTPIT_COUNTER2                 (PORTPIT_BASE + 2)
                                                       /* Counter 2 register */
#define  PORTPIT_TCW                      (PORTPIT_BASE + 3)
                                              /* Timer control word register */

/* ---------------------------------------------- */
/*  Timer control word register (TCW) bit assign  */
/* ---------------------------------------------- */
#define  BITPITTCW_BINARYBCD              (0x01)
                        /* (Bit-0) Binary/BCD countdown select               */
                        /*         (0=binary, 1=BCD)                         */
#define  BITPITTCW_COUNTERMODE            (0x0E)
                        /* (Bit-1,3) Counter mode selection                  */
#define  BITPITTCW_READWRITE              (0x30)
                        /* (Bit-4,5) Read/Write select                       */
#define     BITPITTCW_READWRITE_LATCH     (0x00)
                        /*           Counter latch command                   */
#define     BITPITTCW_READWRITE_LSB       (0x10)
                        /*           Read/Write least significant byte (LSB) */
#define     BITPITTCW_READWRITE_MSB       (0x20)
                        /*           Read/Write most significant byte (MSB)  */
#define     BITPITTCW_READWRITE_LSBMSB    (0x30)
                        /*           Read/Write LSB then MSB                 */
#define  BITPITTCW_COUNTER                (0xC0)
                        /* (Bit-6,7) Counter select                          */
#define     BITPITTCW_COUNTER_0           (0x00)
                        /*           Counter 0 select                        */
#define     BITPITTCW_COUNTER_1           (0x40)
                        /*           Counter 1 select                        */
#define     BITPITTCW_COUNTER_2           (0x80)
                        /*           Counter 2 select                        */
#define     BITPITTCW_COUNTER_READBACK    (0xC0)
                        /*           Read back command                       */
#define  BITPITTCW_READBACKCOMMAND        (BITPITTCW_COUNTER_READBACK | \
                                           BITPITTCW_READWRITE_LATCH  | \
                                           0x0E                       )
                        /* Read back command                                 */
                        /* (Bit-0) Must be 0                                 */
                        /* (Bit-1) Counter 0 select                          */
                        /* (Bit-2) Counter 1 select                          */
                        /* (Bit-3) Counter 2 select                          */

/* ------------------------------- */
/*  Timer byte pointer bit assign  */
/* ------------------------------- */
#define  BITPITTBP_COUNTER0LSB            (0x01)
                                                    /* (Bit-0) Counter 0 LSB */
                                                    /*         (1-not ready) */
#define  BITPITTBP_COUNTER0MSB            (0x02)
                                                    /* (Bit-1) Counter 0 MSB */
                                                    /*         (1-not ready) */
#define  BITPITTBP_COUNTER1LSB            (0x04)
                                                    /* (Bit-2) Counter 1 LSB */
                                                    /*         (1-not ready) */
#define  BITPITTBP_COUNTER1MSB            (0x08)
                                                    /* (Bit-3) Counter 1 MSB */
                                                    /*         (1-not ready) */
#define  BITPITTBP_COUNTER2LSB            (0x10)
                                                    /* (Bit-4) Counter 2 LSB */
                                                    /*         (1-not ready) */
#define  BITPITTBP_COUNTER2MSB            (0x20)
                                                    /* (Bit-5) Counter 2 MSB */
                                                    /*         (1-not ready) */
#define  BITPITTBP_RESERVED6              (0x40)
                                                    /* (Bit-6) Reserved      */
#define  BITPITTBP_RESERVED7              (0x80)
                                                    /* (Bit-7) Reserved      */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  CONTROLFLAG_PITSHADOW            (FALSE)
                                         /* PIT shadow register control flag */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     PIT Controller Definition     * */
/* ************************************* */
/* --------------- */
/*  PIT I/O class  */
/* --------------- */
VOID  PitIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj);                  /* Pointer to Common object */

/* ------------------- */
/*  PIT control class  */
/* ------------------- */
VOID  PitCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            PIT Objects            * */
/* ************************************* */
/* --------------- */
/*  PIT I/O class  */
/* --------------- */
OBJPITIO    oPitIo  = {(POBJCTL)PitIo, PORTISA_BASE};

/* ------------------- */
/*  PIT control class  */
/* ------------------- */
OBJPITCTL   oPitCtl = {(POBJCTL)PitCtl, 0x00, 0x00, 0x30,       /* Counter 0 */
                                        0x12, 0x00, 0x34,       /* Counter 1 */
                                        0x00, 0x00, 0x36,       /* Counter 2 */
                                        0x00};               /* Byte pointer */


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* **************************************** */
/* *          PIT I/O Controller          * */
/* **************************************** */
/* *                                      * */
/* * - Initialize                         * */
/* *     [ Input ]                        * */
/* *       Message    = MSGPIT_INITIALIZE * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read register                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGPIT_READ_xxx   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write register                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGPIT_WRITE_xxx  * */
/* *       Parameter1 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* **************************************** */
VOID  PitIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj)                   /* Pointer to Common object */
{
   DWORD       datap;
   CRESULT     rc;
   POBJPITIO   ppitio;
   DWORD       baseport;

   /* Get pointer to PIT I/O object */
   /*                               */
   ppitio = (POBJPITIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = GetIsaBaseAddress();

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPIT_INITIALIZE:
         /* Initialize */
         /*            */
         ppitio->BaseAddress = GetIsaBaseAddress();
         rc = ERRPIT_NOERROR;
         break;

      default:
         /* Get data-port register */
         /*                        */
         switch(SelectMessage(*pmMsg))
         {
            case MSGPIT_READ_COUNTER0:
            case MSGPIT_WRITE_COUNTER0:
               datap = PORTPIT_COUNTER0;
               rc    = ERRPIT_NOERROR;
               break;
            case MSGPIT_READ_COUNTER1:
            case MSGPIT_WRITE_COUNTER1:
               datap = PORTPIT_COUNTER1;
               rc    = ERRPIT_NOERROR;
               break;
            case MSGPIT_READ_COUNTER2:
            case MSGPIT_WRITE_COUNTER2:
               datap = PORTPIT_COUNTER2;
               rc    = ERRPIT_NOERROR;
               break;
            case MSGPIT_WRITE_TCW:
               datap = PORTPIT_TCW;
               rc    = ERRPIT_NOERROR;
               break;
            default:
               rc = ERRPIT_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case MSGPIT_READ_COUNTER0:
               case MSGPIT_READ_COUNTER1:
               case MSGPIT_READ_COUNTER2:
                  SelectParm1(*pmMsg) =
                              (CPARAMETER)ReadControllerByte(baseport + datap);
                  break;
               case MSGPIT_WRITE_COUNTER0:
               case MSGPIT_WRITE_COUNTER1:
               case MSGPIT_WRITE_COUNTER2:
               case MSGPIT_WRITE_TCW:
                  WriteControllerByte(baseport + datap,
                                      (BYTE)SelectParm1(*pmMsg));
                  break;
               default:
                  rc = ERRPIT_INVALID_MESSAGE;
            } /* endswitch */
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ********************************************* */
/* *          PIT Control Controller           * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPIT_INITIALIZECTL   * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Save device context                     * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPIT_SAVE_CONTEXT    * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Restore device context                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPIT_RESTORE_CONTEXT * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  PitCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGPIT      msg;
   CRESULT     rc;
   POBJPITCTL  ppitctl;
#if (CONTROLFLAG_PITSHADOW == TRUE)
   MSGCARRERA  mCarr;
#endif /* CONTROLFLAG_PITSHADOW */

   /* Get pointer to PIT control object */
   /*                                   */
   ppitctl = (POBJPITCTL)poObj;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPIT_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGPIT_INITIALIZE);
         SendMsg(msg, oPitIo);

         rc = SelectResult(msg);
         break;

      case MSGPIT_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                BITPITTCW_READBACKCOMMAND);
         SendMsg(msg, oPitIo);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsg(msg, MSGPIT_READ_COUNTER0);
            SendMsg(msg, oPitIo);
            if(!SelectResult(msg))
            {
               ppitctl->chCounter0Tcw = (BYTE)(SelectParm1(msg) &
                                               (BITPITTCW_BINARYBCD   |
                                                BITPITTCW_COUNTERMODE |
                                                BITPITTCW_READWRITE   ));
            }
            else
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsg(msg, MSGPIT_READ_COUNTER1);
            SendMsg(msg, oPitIo);
            if(!SelectResult(msg))
            {
               ppitctl->chCounter1Tcw = (BYTE)(SelectParm1(msg) &
                                               (BITPITTCW_BINARYBCD   |
                                                BITPITTCW_COUNTERMODE |
                                                BITPITTCW_READWRITE   ));
            }
            else
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsg(msg, MSGPIT_READ_COUNTER2);
            SendMsg(msg, oPitIo);
            if(!SelectResult(msg))
            {
               ppitctl->chCounter2Tcw = (BYTE)(SelectParm1(msg) &
                                               (BITPITTCW_BINARYBCD   |
                                                BITPITTCW_COUNTERMODE |
                                                BITPITTCW_READWRITE   ));
            }
            else
            {
               rc = SelectResult(msg);
            } /* endif */

#if (CONTROLFLAG_PITSHADOW == TRUE)
            if(!rc)
            {
               BuildMsg(mCarr, MSGCARR_GET_PIT1REGISTERS);
               SendMsg(mCarr, oCarreraCtl);

               if(!(rc = SelectResult(mCarr)))
               {
                  ppitctl->chCounter0CountLow  = (BYTE)SelectParm1(mCarr);
                  ppitctl->chCounter0CountHigh = (BYTE)SelectParm2(mCarr);
                  ppitctl->chCounter2CountLow  = (BYTE)SelectParm3(mCarr);
                  ppitctl->chCounter2CountHigh = (BYTE)SelectParm4(mCarr);
                  ppitctl->chTimerBytePointer  = (BYTE)SelectParm5(mCarr);
               }
               else
               {
                  rc = ERRPIT_CANNOT_READREGISTER;
               } /* endif */
            } /* endif */
#endif /* CONTROLFLAG_PITSHADOW */
         } /* endif */
         break;

      case MSGPIT_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRPIT_NOERROR;

         /* Counter 0 */
         if((ppitctl->chTimerBytePointer                    &
             (BITPITTBP_COUNTER0MSB | BITPITTBP_COUNTER0LSB)) !=
             (BITPITTBP_COUNTER0MSB | BITPITTBP_COUNTER0LSB)   )
         {
            BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                   (ppitctl->chCounter0Tcw &
                                    ~BITPITTCW_READWRITE   )  |
                                   BITPITTCW_READWRITE_LSBMSB |
                                   BITPITTCW_COUNTER_0        );
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER0,
                                   ppitctl->chCounter0CountLow);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER0,
                                   ppitctl->chCounter0CountHigh);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                   ppitctl->chCounter0Tcw |
                                   BITPITTCW_COUNTER_0    );
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            if(((ppitctl->chCounter0Tcw & BITPITTCW_READWRITE) ==
                 BITPITTCW_READWRITE_LSBMSB                     )   &&
               (ppitctl->chTimerBytePointer & BITPITTBP_COUNTER0LSB) )
            {
               BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER0,
                                      ppitctl->chCounter0CountLow);
               SendMsg(msg, oPitIo);
            } /* endif */
         } /* endif */

         /* Counter 1 */
         if((ppitctl->chTimerBytePointer                    &
             (BITPITTBP_COUNTER1MSB | BITPITTBP_COUNTER1LSB)) !=
             (BITPITTBP_COUNTER1MSB | BITPITTBP_COUNTER1LSB)   )
         {
            BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                   (ppitctl->chCounter1Tcw &
                                    ~BITPITTCW_READWRITE   )  |
                                   BITPITTCW_READWRITE_LSBMSB |
                                   BITPITTCW_COUNTER_1        );
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER1,
                                   ppitctl->chCounter1CountLow);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER1,
                                   ppitctl->chCounter1CountHigh);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                   ppitctl->chCounter1Tcw |
                                   BITPITTCW_COUNTER_1    );
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            if(((ppitctl->chCounter1Tcw & BITPITTCW_READWRITE) ==
                BITPITTCW_READWRITE_LSBMSB                      )   &&
               (ppitctl->chTimerBytePointer & BITPITTBP_COUNTER1LSB) )
            {
               BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER1,
                                      ppitctl->chCounter1CountLow);
               SendMsg(msg, oPitIo);
            } /* endif */
         } /* endif */

         /* Counter 2 */
         if((ppitctl->chTimerBytePointer                    &
             (BITPITTBP_COUNTER2MSB | BITPITTBP_COUNTER2LSB)) !=
             (BITPITTBP_COUNTER2MSB | BITPITTBP_COUNTER2LSB)   )
         {
            BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                   (ppitctl->chCounter2Tcw &
                                    ~BITPITTCW_READWRITE   )  |
                                   BITPITTCW_READWRITE_LSBMSB |
                                   BITPITTCW_COUNTER_2        );
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER2,
                                   ppitctl->chCounter2CountLow);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER2,
                                   ppitctl->chCounter2CountHigh);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            BuildMsgWithParm1(msg, MSGPIT_WRITE_TCW,
                                   ppitctl->chCounter2Tcw |
                                   BITPITTCW_COUNTER_2);
            SendMsg(msg, oPitIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */

            if(((ppitctl->chCounter2Tcw & BITPITTCW_READWRITE) ==
                BITPITTCW_READWRITE_LSBMSB                      )   &&
               (ppitctl->chTimerBytePointer & BITPITTBP_COUNTER2LSB) )
            {
               BuildMsgWithParm1(msg, MSGPIT_WRITE_COUNTER2,
                                      ppitctl->chCounter2CountLow);
               SendMsg(msg, oPitIo);
            } /* endif */
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oPitIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
