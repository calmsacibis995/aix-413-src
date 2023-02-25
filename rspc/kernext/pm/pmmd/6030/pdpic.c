/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Programmable Interrupt Controller (PIC, 8259) Device
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
/* * Module Name: PDPIC.C                                                  * */
/* * Description: Programmable Interrupt Controller (PIC, 8259) Device     * */
/* *              Control Routines                                         * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDPIC.H/PDCARERA.H files should be * */
/* *              included in this file                                    * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdpic.h"
#include "pdcarera.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *        PIC Data Definition        * */
/* ************************************* */
/* --------------------------- */
/*  PIC register port address  */
/* --------------------------- */
#define  PORTPIC_BASEPIC1                 (0x00000020L)
                                              /* PIC-1 (master) base address */
#define  PORTPIC_BASEPIC2                 (0x000000A0L)
                                               /* PIC-2 (slave) base address */
                                                 /* Offset from base address */
#define  PORTPIC_ICW1                     (0)
                                   /* Initialization command word 1 register */
#define  PORTPIC_ICW2                     (1)
                                   /* Initialization command word 2 register */
#define  PORTPIC_ICW3                     (1)
                                   /* Initialization command word 3 register */
#define  PORTPIC_ICW4                     (1)
                                   /* Initialization command word 4 register */
#define  PORTPIC_OCW1                     (1)
                                        /* Operation control word 1 register */
#define  PORTPIC_OCW2                     (0)
                                        /* Operation control word 2 register */
#define  PORTPIC_OCW3                     (0)
                                        /* Operation control word 3 register */

/* ---------------------------------------------------------- */
/*  Initialization command word 4 register (ICW4) bit assign  */
/* ---------------------------------------------------------- */
#define  BITPICICW4_FLAG                  (0x01)
                                       /* (Bit-0) Not used, must be set to 1 */

/* ----------------------------------------------------- */
/*  Operation control word 1 register (OCW1) bit assign  */
/* ----------------------------------------------------- */
#define  BITPICOCW1_MASKALL               (0xFF)
                               /* (Bit-0,7) Mask all (disable all interrupt) */

/* ----------------------------------------------------- */
/*  Operation control word 2 register (OCW2) bit assign  */
/* ----------------------------------------------------- */
#define  BITPICOCW2_MASK                  (0xE7)
                                              /* (Bit-0,7) Mask (valid bits) */

/* ----------------------------------------------------- */
/*  Operation control word 3 register (OCW3) bit assign  */
/* ----------------------------------------------------- */
#define  BITPICOCW3_MASK                  (0x67)
                                              /* (Bit-0,7) Mask (valid bits) */
#define  BITPICOCW3_FLAG                  (0x08)
                                              /* (Bit-0,7) Must be set to 1  */

/* ----------------------------------------- */
/*  PIC identification (PIC-1/2) definition  */
/* ----------------------------------------- */
#define  PICID_PIC1                       (1)                    /* PIC-1 ID */
#define  PICID_PIC2                       (2)                    /* PIC-2 ID */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  CONTROLFLAG_PICSHADOW            (FALSE)
                                         /* PIC shadow register control flag */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     PIC Controller Definition     * */
/* ************************************* */
/* --------------- */
/*  PIC I/O class  */
/* --------------- */
VOID  PicIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj);                  /* Pointer to Common object */

/* ------------------- */
/*  PIC control class  */
/* ------------------- */
VOID  PicCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            PIC Objects            * */
/* ************************************* */
/* --------------- */
/*  PIC I/O class  */
/* --------------- */
OBJPICIO    oPic1Io  = {(POBJCTL)PicIo, PORTISA_BASE | PORTPIC_BASEPIC1,
                                        PICID_PIC1};
OBJPICIO    oPic2Io  = {(POBJCTL)PicIo, PORTISA_BASE | PORTPIC_BASEPIC2,
                                        PICID_PIC2};

/* ------------------- */
/*  PIC control class  */
/* ------------------- */
OBJPICCTL   oPic1Ctl = {(POBJCTL)PicCtl, &oPic1Io,
                                         PICID_PIC1,
                                         0x15, 0x00, 0x04, 0x01,  /* ICW 1-4 */
                                         0xFF, 0x20, 0x22};       /* OCW 1-3 */
OBJPICCTL   oPic2Ctl = {(POBJCTL)PicCtl, &oPic2Io,
                                         PICID_PIC2,
                                         0x15, 0x08, 0x02, 0x01,  /* ICW 1-4 */
                                         0xFF, 0x20, 0x22};       /* OCW 1-3 */


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* **************************************** */
/* *          PIC I/O Controller          * */
/* **************************************** */
/* *                                      * */
/* * - Initialize                         * */
/* *     [ Input ]                        * */
/* *       Message    = MSGPIC_INITIALIZE * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read register                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGPIC_READ_xxx   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write register                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGPIC_WRITE_xxx  * */
/* *       Parameter1 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* **************************************** */
VOID  PicIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj)                   /* Pointer to Common object */
{
   DWORD       off;
   CRESULT     rc;
   POBJPICIO   ppicio;
   DWORD       baseport;

   /* Get pointer to PIC I/O object */
   /*                               */
   ppicio = (POBJPICIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = ppicio->BaseAddress + GetIsaBaseAddress();

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPIC_INITIALIZE:
         /* Initialize */
         /*            */
         ppicio->BaseAddress = (ppicio->chPicId == PICID_PIC1)
                                ? PORTPIC_BASEPIC1
                                : PORTPIC_BASEPIC2;
         rc = ERRPIC_NOERROR;
         break;

      default:
         /* Get data-port register */
         /*                        */
         switch(SelectMessage(*pmMsg))
         {
            case MSGPIC_WRITE_ICW1:
               off = PORTPIC_ICW1;
               rc  = ERRPIC_NOERROR;
               break;
            case MSGPIC_WRITE_ICW2:
               off = PORTPIC_ICW2;
               rc  = ERRPIC_NOERROR;
               break;
            case MSGPIC_WRITE_ICW3:
               off = PORTPIC_ICW3;
               rc  = ERRPIC_NOERROR;
               break;
            case MSGPIC_WRITE_ICW4:
               off = PORTPIC_ICW4;
               rc  = ERRPIC_NOERROR;
               break;
            case MSGPIC_READ_OCW1:
            case MSGPIC_WRITE_OCW1:
               off = PORTPIC_OCW1;
               rc  = ERRPIC_NOERROR;
               break;
            case MSGPIC_WRITE_OCW2:
               off = PORTPIC_OCW2;
               rc  = ERRPIC_NOERROR;
               break;
            case MSGPIC_READ_OCW3:
            case MSGPIC_WRITE_OCW3:
               off = PORTPIC_OCW3;
               rc  = ERRPIC_NOERROR;
               break;
            default:
               rc = ERRPIC_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case MSGPIC_READ_OCW1:
               case MSGPIC_READ_OCW3:
                  SelectParm1(*pmMsg) =
                                 (CPARAMETER)ReadControllerByte(baseport + off);
                  break;
               case MSGPIC_WRITE_ICW1:
               case MSGPIC_WRITE_ICW2:
               case MSGPIC_WRITE_ICW3:
               case MSGPIC_WRITE_ICW4:
               case MSGPIC_WRITE_OCW1:
               case MSGPIC_WRITE_OCW2:
               case MSGPIC_WRITE_OCW3:
                  WriteControllerByte(baseport + off,
                                      (BYTE)SelectParm1(*pmMsg));
                  break;
               default:
                  rc = ERRPIC_INVALID_MESSAGE;
            } /* endswitch */
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* *********************************************** */
/* *           PIC Control Controller            * */
/* *********************************************** */
/* *                                             * */
/* * - Initialize                                * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPIC_INITIALIZECTL     * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Disable all interrupt                     * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPIC_DISABLE_INTERRUPT * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Save device context                       * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPIC_SAVE_CONTEXT      * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Restore device context                    * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPIC_RESTORE_CONTEXT   * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* *********************************************** */
VOID  PicCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGPIC      msg;
   CRESULT     rc;
   POBJPICCTL  ppicctl;
   POBJPICIO   ppicio;
#if (CONTROLFLAG_PICSHADOW == TRUE)
   MSGCARRERA  mCarr;
#endif /* CONTROLFLAG_PICSHADOW */

   /* Get pointer to PIC control object */
   /*                                   */
   ppicctl = (POBJPICCTL)poObj;

   /* Get pointer to PIC I/O object */
   /*                               */
   ppicio = ppicctl->poPicIo;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPIC_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGPIC_INITIALIZE);
         SendMsg(msg, *ppicio);

         rc = SelectResult(msg);
         break;

      case MSGPIC_DISABLE_INTERRUPT:
         /* Disable all interrupt */
         /*                       */
         BuildMsgWithParm1(msg, MSGPIC_WRITE_OCW1,
                                BITPICOCW1_MASKALL);
         SendMsg(msg, *ppicio);

         rc = SelectResult(msg);
         break;

      case MSGPIC_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         BuildMsg(msg, MSGPIC_READ_OCW1);
         SendMsg(msg, *ppicio);

         if(!(rc = SelectResult(msg)))
         {
            ppicctl->chOcw1 = (BYTE)SelectParm1(msg);

#if (CONTROLFLAG_PICSHADOW == TRUE)
            BuildMsg(mCarr, (ppicctl->chPicId == PICID_PIC1)
                            ? MSGCARR_GET_PIC1REGISTERS
                            : MSGCARR_GET_PIC2REGISTERS);
            SendMsg(mCarr, oCarreraCtl);

            if(!(rc = SelectResult(mCarr)))
            {
               ppicctl->chIcw1 = (BYTE)SelectParm1(mCarr);
               ppicctl->chIcw2 = (BYTE)SelectParm2(mCarr);
               ppicctl->chIcw3 = (BYTE)SelectParm3(mCarr);
               ppicctl->chIcw4 = (BYTE)SelectParm4(mCarr);
               ppicctl->chOcw2 = (BYTE)SelectParm5(mCarr);
               ppicctl->chOcw3 = (BYTE)SelectParm6(mCarr);
            }
            else
            {
               rc = ERRPIC_CANNOT_READREGISTER;
            } /* endif */
#endif /* CONTROLFLAG_PICSHADOW */
         } /* endif */
         break;

      case MSGPIC_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRPIC_NOERROR;

         BuildMsgWithParm1(msg, MSGPIC_WRITE_ICW1,
                                ppicctl->chIcw1);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPIC_WRITE_ICW2,
                                ppicctl->chIcw2);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPIC_WRITE_ICW3,
                                ppicctl->chIcw3);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPIC_WRITE_ICW4,
                                ppicctl->chIcw4 | BITPICICW4_FLAG);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPIC_DISABLE_INTERRUPT);
         SendMsg(msg, *poObj);

         BuildMsgWithParm1(msg, MSGPIC_WRITE_OCW2,
                                ppicctl->chOcw2 & BITPICOCW2_MASK);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPIC_WRITE_OCW3,
                                (ppicctl->chOcw3 & BITPICOCW3_MASK) |
                                BITPICOCW3_FLAG);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPIC_WRITE_OCW1,
                                ppicctl->chOcw1);
         SendMsg(msg, *ppicio);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, *ppicio);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
