/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Programmable Option Select (POS) Device Control Routines
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
/* * Module Name: PDPOS.C                                                  * */
/* * Description: Programmable Option Select (POS) Device Control Routines * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDPOS.H/PDCOOPER.H files should be * */
/* *              included in this file                                    * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdpos.h"
#include "pdcooper.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *        POS Data Definition        * */
/* ************************************* */
/* --------------------------- */
/*  POS register port address  */
/* --------------------------- */
#define  PORTPOS_PLANARSETUP              (0x000008B8L)
                                             /* Planar enable/setup register */
#define  PORTPOS_PLANAR                   (0x000008B2L)
                                                      /* Planar POS register */
#define  PORTPOS_VIDEOSUBSYSTEM           (0x000008B2L)
                                             /* Video subsystem POS register */
#define  PORTPOS_PENINTERFACE1            (0x000008B2L)
                                             /* Pen interface POS 1 register */
#define  PORTPOS_PENINTERFACE2            (0x000008B3L)
                                             /* Pen interface POS 2 register */
#define  PORTPOS_PENINTERFACE3            (0x000008B4L)
                                             /* Pen interface POS 3 register */
#define  PORTPOS_PCMCIACONTROLLER1        (0x000008B2L)
                                         /* PCMCIA controller POS 1 register */
#define  PORTPOS_PCMCIACONTROLLER2        (0x000008B3L)
                                         /* PCMCIA controller POS 2 register */
#define  PORTPOS_PCMCIACONTROLLER3        (0x000008B4L)
                                         /* PCMCIA controller POS 3 register */
#define  PORTPOS_COOPERENABLE             (0x000008B2L)
                                                   /* Cooper enable register */
#define  PORTPOS_COOPERINDEXL             (0x000008B6L)
                                       /* Cooper function index low register */
#define  PORTPOS_COOPERINDEXH             (0x000008B7L)
                                      /* Cooper function index high register */
#define  PORTPOS_FDCSETUP                 (0x000008B3L)
                                               /* FDC support setup register */
#define  PORTPOS_VIDEOSETUP               (0x000008B4L)
                                             /* Video support setup register */
#define  PORTPOS_H8SETUP                  (0x000008B3L)
                                              /* H8 interface setup register */
#define  PORTPOS_AUDIOSETUP               (0x000008B4L)
                                             /* Audio support setup register */
#define  PORTPOS_AUDIOADDRESSL            (0x000008B3L)
                                           /* Audio I/O address low register */
#define  PORTPOS_AUDIOADDRESSH            (0x000008B4L)
                                          /* Audio I/O address high register */
#define  PORTPOS_NVRAMSETUP               (0x000008B4L)
                                           /* NVRAM interface setup register */
#define  PORTPOS_DMACOMMANDSHADOW08       (0x000008B3L)
                                        /* DMA command shadow '08h' register */
#define  PORTPOS_DMACOMMANDSHADOWD0       (0x000008B4L)
                                        /* DMA command shadow 'D0h' register */
#define  PORTPOS_VIDEOSTATUS1             (0x000008B3L)
                                                  /* Video status 1 register */
#define  PORTPOS_VIDEOSTATUS2             (0x000008B4L)
                                                  /* Video status 2 register */

/* ----------------------------------- */
/*  Index data to index POS registers  */
/* ----------------------------------- */
#define  INDEXPOS_FDCSETUP                (0x0000)
                                               /* FDC support setup register */
#define  INDEXPOS_VIDEOSETUP              (0x0000)
                                             /* Video support setup register */
#define  INDEXPOS_H8SETUP                 (0x0001)
                                              /* H8 interface setup register */
#define  INDEXPOS_AUDIOSETUP              (0x0001)
                                             /* Audio support setup register */
#define  INDEXPOS_AUDIOADDRESSL           (0x0002)
                                           /* Audio I/O address low register */
#define  INDEXPOS_AUDIOADDRESSH           (0x0002)
                                          /* Audio I/O address high register */
#define  INDEXPOS_NVRAMSETUP              (0x0003)
                                           /* NVRAM interface setup register */
#define  INDEXPOS_DMACOMMANDSHADOW08      (0x0004)
                                        /* DMA command shadow '08h' register */
#define  INDEXPOS_DMACOMMANDSHADOWD0      (0x0004)
                                        /* DMA command shadow 'D0h' register */
#define  INDEXPOS_VIDEOSTATUS1            (0x0005)
                                                  /* Video status 1 register */
#define  INDEXPOS_VIDEOSTATUS2            (0x0005)
                                                  /* Video status 2 register */

/* ------------------------------------------------------- */
/*  Planar enable/setup register (PLANARSETUP) bit assign  */
/* ------------------------------------------------------- */
#define  BITPOSPLANARSETUP_RESERVED0      (0x01)
                                       /* (Bit-0) Reserved (always set to 1) */
#define  BITPOSPLANARSETUP_EXPANSION      (0x02)
                                       /* (Bit-1) Expansion subsystem setup  */
#define  BITPOSPLANARSETUP_PCMCIA         (0x04)
                                       /* (Bit-2) PCMCIA subsystem setup     */
#define  BITPOSPLANARSETUP_RESERVED3      (0x08)
                                       /* (Bit-3) Reserved (always set to 1) */
#define  BITPOSPLANARSETUP_PEN            (0x10)
                                       /* (Bit-4) Pen interface setup        */
#define  BITPOSPLANARSETUP_VIDEO          (0x20)
                                       /* (Bit-5) Video subsystem setup      */
#define  BITPOSPLANARSETUP_COOPER         (0x40)
                                       /* (Bit-6) Cooper functions setup     */
#define  BITPOSPLANARSETUP_PLANAR         (0x80)
                                       /* (Bit-7) Planar functions setup     */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     POS Controller Definition     * */
/* ************************************* */
/* --------------- */
/*  POS I/O class  */
/* --------------- */
VOID  PosIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj);                  /* Pointer to Common object */

/* ------------------- */
/*  POS control class  */
/* ------------------- */
VOID  PosCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            POS Objects            * */
/* ************************************* */
/* --------------- */
/*  POS I/O class  */
/* --------------- */
OBJPOSIO    oPosIo  = {(POBJCTL)PosIo, PORTISA_BASE};

/* ------------------- */
/*  POS control class  */
/* ------------------- */
OBJPOSCTL   oPosCtl = {(POBJCTL)PosCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ********************************************* */
/* *            POS I/O Controller             * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_INITIALIZE      * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Get planar enable/setup register        * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_GET_PLANARSETUP * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *       Parameter1 = register value         * */
/* *                                           * */
/* * - Set planar enable/setup register        * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_SET_PLANARSETUP * */
/* *       Parameter1 = register value         * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Read register                           * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_READ_xxx        * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *       Parameter1 = register value         * */
/* *                                           * */
/* * - Write register                          * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_WRITE_xxx       * */
/* *       Parameter1 = register value         * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  PosIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj)                   /* Pointer to Common object */
{
   MSGCOOPER   mCooper;
   CPARAMETER  sstatus;
   BYTE        setup, ssetup;
   WORD        index, sindex;
   DWORD       datap;
   CRESULT     rc;
   POBJPOSIO   pposio;
   DWORD       baseport;

   /* Get pointer to POS I/O object */
   /*                               */
   pposio = (POBJPOSIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = GetIsaBaseAddress();

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPOS_INITIALIZE:
         /* Initialize */
         /*            */
         pposio->BaseAddress = GetIsaBaseAddress();
         rc = ERRPOS_NOERROR;
         break;

      case MSGPOS_GET_PLANARSETUP:
         /* Get planar enable/setup register */
         /*                                  */
         /* Get/Save planar setup status */
         BuildMsg(mCooper, MSGCOOP_GET_PLANARSETUPSTATUS);
         SendMsg(mCooper, oCooperCtl);
         sstatus = SelectParm1(mCooper);

         /* Unlock planar setup status */
         BuildMsgWithParm1(mCooper, MSGCOOP_SET_PLANARSETUPSTATUS,
                                    PRMCOOP_SETUPUNLOCK);
         SendMsg(mCooper, oCooperCtl);

         SelectParm1(*pmMsg) =
                             ReadControllerByte(baseport + PORTPOS_PLANARSETUP);

         /* Restore planar setup status */
         BuildMsgWithParm1(mCooper, MSGCOOP_SET_PLANARSETUPSTATUS,
                                    sstatus);
         SendMsg(mCooper, oCooperCtl);

         rc = ERRPOS_NOERROR;
         break;

      case MSGPOS_SET_PLANARSETUP:
         /* Set planar enable/setup register */
         /*                                  */
         /* Get/Save planar setup status */
         BuildMsg(mCooper, MSGCOOP_GET_PLANARSETUPSTATUS);
         SendMsg(mCooper, oCooperCtl);
         sstatus = SelectParm1(mCooper);

         /* Unlock planar setup status */
         BuildMsgWithParm1(mCooper, MSGCOOP_SET_PLANARSETUPSTATUS,
                                    PRMCOOP_SETUPUNLOCK);
         SendMsg(mCooper, oCooperCtl);

         WriteControllerByte(baseport + PORTPOS_PLANARSETUP,
                             (BYTE)SelectParm1(*pmMsg));

         /* Restore planar setup status */
         BuildMsgWithParm1(mCooper, MSGCOOP_SET_PLANARSETUPSTATUS,
                                    sstatus);
         SendMsg(mCooper, oCooperCtl);

         rc = ERRPOS_NOERROR;
         break;

      default:
         /* Get setup/data-port/index register */
         /*                                    */
         switch(SelectMessage(*pmMsg))
         {
            case MSGPOS_READ_PLANAR:
            case MSGPOS_WRITE_PLANAR:
               setup = (BYTE)~BITPOSPLANARSETUP_PLANAR;
               datap = PORTPOS_PLANAR;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_VIDEOSUBSYSTEM:
            case MSGPOS_WRITE_VIDEOSUBSYSTEM:
               setup = (BYTE)~BITPOSPLANARSETUP_VIDEO;
               datap = PORTPOS_VIDEOSUBSYSTEM;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_PENINTERFACE1:
            case MSGPOS_WRITE_PENINTERFACE1:
               setup = (BYTE)~BITPOSPLANARSETUP_PEN;
               datap = PORTPOS_PENINTERFACE1;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_PENINTERFACE2:
            case MSGPOS_WRITE_PENINTERFACE2:
               setup = (BYTE)~BITPOSPLANARSETUP_PEN;
               datap = PORTPOS_PENINTERFACE2;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_PENINTERFACE3:
            case MSGPOS_WRITE_PENINTERFACE3:
               setup = (BYTE)~BITPOSPLANARSETUP_PEN;
               datap = PORTPOS_PENINTERFACE3;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_PCMCIACONTROLLER1:
            case MSGPOS_WRITE_PCMCIACONTROLLER1:
               setup = (BYTE)~BITPOSPLANARSETUP_PCMCIA;
               datap = PORTPOS_PCMCIACONTROLLER1;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_PCMCIACONTROLLER2:
            case MSGPOS_WRITE_PCMCIACONTROLLER2:
               setup = (BYTE)~BITPOSPLANARSETUP_PCMCIA;
               datap = PORTPOS_PCMCIACONTROLLER2;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_PCMCIACONTROLLER3:
            case MSGPOS_WRITE_PCMCIACONTROLLER3:
               setup = (BYTE)~BITPOSPLANARSETUP_PCMCIA;
               datap = PORTPOS_PCMCIACONTROLLER3;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_COOPERENABLE:
            case MSGPOS_WRITE_COOPERENABLE:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_COOPERENABLE;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_COOPERINDEXL:
            case MSGPOS_WRITE_COOPERINDEXL:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_COOPERINDEXL;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_COOPERINDEXH:
            case MSGPOS_WRITE_COOPERINDEXH:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_COOPERINDEXH;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_FDCSETUP:
            case MSGPOS_WRITE_FDCSETUP:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_FDCSETUP;
               index = INDEXPOS_FDCSETUP;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_VIDEOSETUP:
            case MSGPOS_WRITE_VIDEOSETUP:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_VIDEOSETUP;
               index = INDEXPOS_VIDEOSETUP;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_H8SETUP:
            case MSGPOS_WRITE_H8SETUP:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_H8SETUP;
               index = INDEXPOS_H8SETUP;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_AUDIOSETUP:
            case MSGPOS_WRITE_AUDIOSETUP:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_AUDIOSETUP;
               index = INDEXPOS_AUDIOSETUP;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_AUDIOADDRESSL:
            case MSGPOS_WRITE_AUDIOADDRESSL:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_AUDIOADDRESSL;
               index = INDEXPOS_AUDIOADDRESSL;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_AUDIOADDRESSH:
            case MSGPOS_WRITE_AUDIOADDRESSH:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_AUDIOADDRESSH;
               index = INDEXPOS_AUDIOADDRESSH;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_NVRAMSETUP:
            case MSGPOS_WRITE_NVRAMSETUP:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_NVRAMSETUP;
               index = INDEXPOS_NVRAMSETUP;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_DMACOMMANDSHADOW08:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_DMACOMMANDSHADOW08;
               index = INDEXPOS_DMACOMMANDSHADOW08;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_DMACOMMANDSHADOWD0:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_DMACOMMANDSHADOWD0;
               index = INDEXPOS_DMACOMMANDSHADOWD0;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_VIDEOSTATUS1:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_VIDEOSTATUS1;
               index = INDEXPOS_VIDEOSTATUS1;
               rc    = ERRPOS_NOERROR;
               break;
            case MSGPOS_READ_VIDEOSTATUS2:
               setup = (BYTE)~BITPOSPLANARSETUP_COOPER;
               datap = PORTPOS_VIDEOSTATUS2;
               index = INDEXPOS_VIDEOSTATUS2;
               rc    = ERRPOS_NOERROR;
               break;
            default:
               rc = ERRPOS_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Get/Save planar setup status */
            /*                              */
            BuildMsg(mCooper, MSGCOOP_GET_PLANARSETUPSTATUS);
            SendMsg(mCooper, oCooperCtl);
            sstatus = SelectParm1(mCooper);

            /* Unlock planar setup status */
            /*                            */
            BuildMsgWithParm1(mCooper, MSGCOOP_SET_PLANARSETUPSTATUS,
                                       PRMCOOP_SETUPUNLOCK);
            SendMsg(mCooper, oCooperCtl);

            /* Save setup register */
            /*                     */
            ssetup = ReadControllerByte(baseport + PORTPOS_PLANARSETUP);

            /* Input/Output POS registers */
            /*                            */
            switch(SelectMessage(*pmMsg))
            {
               case MSGPOS_READ_PLANAR:
               case MSGPOS_READ_VIDEOSUBSYSTEM:
               case MSGPOS_READ_PENINTERFACE1:
               case MSGPOS_READ_PENINTERFACE2:
               case MSGPOS_READ_PENINTERFACE3:
               case MSGPOS_READ_PCMCIACONTROLLER1:
               case MSGPOS_READ_PCMCIACONTROLLER2:
               case MSGPOS_READ_PCMCIACONTROLLER3:
               case MSGPOS_READ_COOPERENABLE:
               case MSGPOS_READ_COOPERINDEXL:
               case MSGPOS_READ_COOPERINDEXH:
                  WriteControllerByte(baseport + PORTPOS_PLANARSETUP, setup);
                  SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + datap);
                  break;
               case MSGPOS_WRITE_PLANAR:
               case MSGPOS_WRITE_VIDEOSUBSYSTEM:
               case MSGPOS_WRITE_PENINTERFACE1:
               case MSGPOS_WRITE_PENINTERFACE2:
               case MSGPOS_WRITE_PENINTERFACE3:
               case MSGPOS_WRITE_PCMCIACONTROLLER1:
               case MSGPOS_WRITE_PCMCIACONTROLLER2:
               case MSGPOS_WRITE_PCMCIACONTROLLER3:
               case MSGPOS_WRITE_COOPERENABLE:
               case MSGPOS_WRITE_COOPERINDEXL:
               case MSGPOS_WRITE_COOPERINDEXH:
                  WriteControllerByte(baseport + PORTPOS_PLANARSETUP, setup);
                  WriteControllerByte(baseport + datap,
                                      (BYTE)SelectParm1(*pmMsg));
                  break;
               default:
                  WriteControllerByte(baseport + PORTPOS_PLANARSETUP, setup);

                  /* Save index register */
                  /*                     */
                  sindex =
                   ((WORD)ReadControllerByte(baseport + PORTPOS_COOPERINDEXH)
                    << 8                                                     ) |
                   (WORD)ReadControllerByte(baseport + PORTPOS_COOPERINDEXL)   ;

                  /* Input/Output extended POS registers */
                  /*                                     */
                  switch(SelectMessage(*pmMsg))
                  {
                     case MSGPOS_READ_FDCSETUP:
                     case MSGPOS_READ_VIDEOSETUP:
                     case MSGPOS_READ_H8SETUP:
                     case MSGPOS_READ_AUDIOSETUP:
                     case MSGPOS_READ_AUDIOADDRESSL:
                     case MSGPOS_READ_AUDIOADDRESSH:
                     case MSGPOS_READ_NVRAMSETUP:
                     case MSGPOS_READ_DMACOMMANDSHADOW08:
                     case MSGPOS_READ_DMACOMMANDSHADOWD0:
                     case MSGPOS_READ_VIDEOSTATUS1:
                     case MSGPOS_READ_VIDEOSTATUS2:
                        WriteControllerByte(baseport + PORTPOS_COOPERINDEXH,
                                            (BYTE)(index >> 8));
                        WriteControllerByte(baseport + PORTPOS_COOPERINDEXL,
                                            (BYTE)index);
                        SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + datap);
                        break;
                     case MSGPOS_WRITE_FDCSETUP:
                     case MSGPOS_WRITE_VIDEOSETUP:
                     case MSGPOS_WRITE_H8SETUP:
                     case MSGPOS_WRITE_AUDIOSETUP:
                     case MSGPOS_WRITE_AUDIOADDRESSL:
                     case MSGPOS_WRITE_AUDIOADDRESSH:
                     case MSGPOS_WRITE_NVRAMSETUP:
                        WriteControllerByte(baseport + PORTPOS_COOPERINDEXH,
                                            (BYTE)(index >> 8));
                        WriteControllerByte(baseport + PORTPOS_COOPERINDEXL,
                                            (BYTE)index);
                        WriteControllerByte(baseport + datap,
                                            (BYTE)SelectParm1(*pmMsg));
                        break;
                     default:
                        rc = ERRPOS_INVALID_MESSAGE;
                  } /* endswitch */

                  /* Restore index register */
                  /*                        */
                  IoDelay();
                  WriteControllerByte(baseport + PORTPOS_COOPERINDEXH,
                                      (BYTE)(sindex >> 8));
                  WriteControllerByte(baseport + PORTPOS_COOPERINDEXL,
                                      (BYTE)sindex);
            } /* endswitch */

            /* Restore setup register */
            /*                        */
            IoDelay();
            WriteControllerByte(baseport + PORTPOS_PLANARSETUP, ssetup);

            /* Restore planar setup status */
            /*                             */
            BuildMsgWithParm1(mCooper, MSGCOOP_SET_PLANARSETUPSTATUS,
                                       sstatus);
            SendMsg(mCooper, oCooperCtl);
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ********************************************* */
/* *          POS Control Controller           * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_INITIALIZECTL   * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Save device context                     * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_SAVE_CONTEXT    * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Restore device context                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGPOS_RESTORE_CONTEXT * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  PosCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGPOS      msg;
   CRESULT     rc;
   POBJPOSCTL  pposctl;

   /* Get pointer to POS control object */
   /*                                   */
   pposctl = (POBJPOSCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPOS_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGPOS_INITIALIZE);
         SendMsg(msg, oPosIo);

         rc = SelectResult(msg);
         break;

      case MSGPOS_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRPOS_NOERROR;

         BuildMsg(msg, MSGPOS_GET_PLANARSETUP);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPlanarSetup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PLANAR);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPlanar = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_VIDEOSUBSYSTEM);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chVideoSubsystem = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PENINTERFACE1);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPenInterface1 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PENINTERFACE2);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPenInterface2 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PENINTERFACE3);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPenInterface3 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PCMCIACONTROLLER1);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPcmciaController1 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PCMCIACONTROLLER2);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPcmciaController2 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_PCMCIACONTROLLER3);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chPcmciaController3 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_COOPERENABLE);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chCooperEnable = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_COOPERINDEXL);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chCooperIndexL = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_COOPERINDEXH);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chCooperIndexH = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_FDCSETUP);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chFdcSetup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_VIDEOSETUP);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chVideoSetup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_H8SETUP);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chH8Setup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_AUDIOSETUP);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chAudioSetup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_AUDIOADDRESSL);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chAudioAddressL = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_AUDIOADDRESSH);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chAudioAddressH = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPOS_READ_NVRAMSETUP);
         SendMsg(msg, oPosIo);
         if(!SelectResult(msg))
         {
            pposctl->chNvramSetup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGPOS_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRPOS_NOERROR;

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PLANAR,
                                pposctl->chPlanar);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_VIDEOSUBSYSTEM,
                                pposctl->chVideoSubsystem);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PENINTERFACE1,
                                pposctl->chPenInterface1);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PENINTERFACE2,
                                pposctl->chPenInterface2);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PENINTERFACE3,
                                pposctl->chPenInterface3);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PCMCIACONTROLLER1,
                                pposctl->chPcmciaController1);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PCMCIACONTROLLER2,
                                pposctl->chPcmciaController2);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_PCMCIACONTROLLER3,
                                pposctl->chPcmciaController3);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_COOPERENABLE,
                                pposctl->chCooperEnable);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_COOPERINDEXL,
                                pposctl->chCooperIndexL);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_COOPERINDEXH,
                                pposctl->chCooperIndexH);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_FDCSETUP,
                                pposctl->chFdcSetup);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_VIDEOSETUP,
                                pposctl->chVideoSetup);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_H8SETUP,
                                pposctl->chH8Setup);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_AUDIOSETUP,
                                pposctl->chAudioSetup);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_AUDIOADDRESSL,
                                pposctl->chAudioAddressL);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_AUDIOADDRESSH,
                                pposctl->chAudioAddressH);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_WRITE_NVRAMSETUP,
                                pposctl->chNvramSetup);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPOS_SET_PLANARSETUP,
                                pposctl->chPlanarSetup);
         SendMsg(msg, oPosIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oPosIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
