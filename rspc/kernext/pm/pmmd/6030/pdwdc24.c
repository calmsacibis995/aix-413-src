/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Video (Western Digital 90C24) Chip Device Control
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
/* * Module Name: PDWDC24.C                                                * */
/* * Description: Video (Western Digital 90C24) Chip Device Control        * */
/* *              Routines                                                 * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDWDC24.H/PDPCIBUS.H files should  * */
/* *              be included in this file                                 * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdwdc24.h"
#include "pdpcibus.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      WD90C24 Data Definition      * */
/* ************************************* */
/* ------------------------------------- */
/*  Configuration register port address  */
/* ------------------------------------- */
#define  PORTC24_BASE                     (0x00000000L)
#define  PORTC24_BASENULL                 (0xFFFFFFFFL)
                                                     /* WD90C24 base address */
                                                      /* & base null address */
#define  PORTC24_BASEMASK                 (0xFFFFFFFCL) /* Base address mask */
                                                 /* Offset from base address */
#define  PORTC24_P3MODE                   (0x80)         /* P3 mode register */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *   WD90C24 Controller Definition   * */
/* ************************************* */
/* ------------------- */
/*  WD90C24 I/O class  */
/* ------------------- */
VOID  Wd90c24Io(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */

/* ----------------------- */
/*  WD90C24 control class  */
/* ----------------------- */
VOID  Wd90c24Ctl(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj);            /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *          WD90C24 Objects          * */
/* ************************************* */
/* ------------------- */
/*  WD90C24 I/O class  */
/* ------------------- */
OBJWD90C24IO   oWd90c24Io  = {(POBJCTL)Wd90c24Io, PORTC24_BASENULL};

/* ----------------------- */
/*  WD90C24 control class  */
/* ----------------------- */
OBJWD90C24CTL  oWd90c24Ctl = {(POBJCTL)Wd90c24Ctl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************ */
/* *            WD90C24 I/O Controller            * */
/* ************************************************ */
/* *                                              * */
/* * - Initialize                                 * */
/* *     [ Input ]                                * */
/* *       Message    = MSGC24_INITIALIZE         * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read register                              * */
/* *     [ Input ]                                * */
/* *       Message    = MSGC24_READ_xxx           * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       Parameter1 = register value            * */
/* *       C24Parm1   = register value (32bits)   * */
/* *                                              * */
/* * - Write register                             * */
/* *     [ Input ]                                * */
/* *       Message    = MSGC24_WRITE_xxx          * */
/* *       Parameter1 = register value (not used) * */
/* *       C24Parm1   = register value (32bits)   * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* ************************************************ */
VOID  Wd90c24Io(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   MSGPCI         mPci;
   WORD           off;
   CRESULT        rc;
   POBJWD90C24IO  pc24io;

   /* Get pointer to WD90C24 I/O object */
   /*                                   */
   pc24io = (POBJWD90C24IO)poObj;

   /* Check if completing for initialization */
   /*                                        */
   if((pc24io->BaseAddress   != PORTC24_BASENULL) ||
      (SelectMessage(*pmMsg) == MSGC24_INITIALIZE)  )
   {
      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGC24_INITIALIZE:
            /* Initialize */
            /*            */
            BuildMsg(mPci, MSGPCI_QUERY_DEVICE);
            SendMsg(mPci, oPciWd90c24Ctl);

            if(!(rc = SelectResult(mPci)))
            {
               BuildMsg(mPci, MSGPCI_GET_BASEADDRESS0);
               SendMsg(mPci, oPciWd90c24Ctl);

               if(!(rc = SelectResult(mPci)))
               {
                  pc24io->BaseAddress =
                                     (SelectPciParm1(mPci) & PORTC24_BASEMASK)
                                                           + GetIoBaseAddress();
               }
               else
               {
                  rc = ERRC24_CANNOT_INITIALIZE;
               } /* endif */
            }
            else
            {
               rc = ERRC24_CANNOT_INITIALIZE;
            } /* endif */
            break;

         default:
            /* Get offset to data-port register */
            /*                                  */
            switch(SelectMessage(*pmMsg))
            {
               case MSGC24_READ_P3MODE:
               case MSGC24_WRITE_P3MODE:
                  off = PORTC24_P3MODE;
                  rc  = ERRC24_NOERROR;
                  break;
               default:
                  rc = ERRC24_INVALID_MESSAGE;
            } /* endswitch */

            if(!rc)
            {
               /* Input/Output registers */
               /*                        */
               switch(SelectMessage(*pmMsg))
               {
                  case MSGC24_READ_P3MODE:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciWd90c24Ctl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        SelectC24Parm1(*pmMsg) = SelectPciParm1(mPci);
                        SelectParm1(*pmMsg)    =
                                      (CPARAMETER)SelectPciParm1(mPci) & 0x00FF;
                     }
                     else
                     {
                        rc = ERRC24_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGC24_WRITE_P3MODE:
                     BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                             off);
                     BuildPciParm1(mPci, SelectC24Parm1(*pmMsg));
                     SendMsg(mPci, oPciWd90c24Ctl);

                     if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                     {
                        rc = ERRC24_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  default:
                     rc = ERRC24_INVALID_MESSAGE;
               } /* endswitch */
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERRC24_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ********************************************* */
/* *        WD90C24 Control Controller         * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGC24_INITIALIZECTL   * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Save device context                     * */
/* *     [ Input ]                             * */
/* *       Message    = MSGC24_SAVE_CONTEXT    * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Restore device context                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGC24_RESTORE_CONTEXT * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  Wd90c24Ctl(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj)             /* Pointer to Common object */
{
   MSGWD90C24     msg;
   CRESULT        rc;
   POBJWD90C24CTL pc24ctl;

   /* Get pointer to WD90C24 control object */
   /*                                       */
   pc24ctl = (POBJWD90C24CTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGC24_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGC24_INITIALIZE);
         SendMsg(msg, oWd90c24Io);

         rc = SelectResult(msg);
         break;

      case MSGC24_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRC24_NOERROR;

         BuildMsg(msg, MSGC24_READ_P3MODE);
         SendMsg(msg, oWd90c24Io);
         if(!SelectResult(msg))
         {
            pc24ctl->dwOffset80h = SelectC24Parm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGC24_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRC24_NOERROR;

         BuildMsg(msg, MSGC24_WRITE_P3MODE);
         BuildC24Parm1(msg, pc24ctl->dwOffset80h);
         SendMsg(msg, oWd90c24Io);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oWd90c24Io);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
