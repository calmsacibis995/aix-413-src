static char sccsid[] = "@(#)24	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pdrtc.c, pwrsysx, rspc41J, 9515B_all 4/14/95 10:55:47";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Serialized Real Time Clocks (DS1585/1587) Chip Device
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
/* * Module Name: PDRTC.C                                                  * */
/* * Description: Serialized Real Time Clocks (DS1585/1587) Chip Device    * */
/* *              Control Routines                                         * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDRTC.H files should be included in* */
/* *              this file                                                * */
/* *                                                                       * */
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
/* *        RTC Data Definition        * */
/* ************************************* */
/* ------------------------------------------ */
/*  Control register index/data port address  */
/* ------------------------------------------ */
#define  PORTRTC_BASE                     (0x00000070L)  /* RTC base address */
#define  PORTRTC_INDEX                    (PORTRTC_BASE + 0)
                                                           /* Index register */
#define  PORTRTC_DATA                     (PORTRTC_BASE + 1)
                                                            /* Data register */

/* ----------------------------------- */
/*  Index data to index RTC registers  */
/* ----------------------------------- */
#define  INDEXRTCBANK0_SECONDS            (0x00)
                                                                  /* Seconds */
#define  INDEXRTCBANK0_SECONDSALARM       (0x01)
                                                            /* Seconds alarm */
#define  INDEXRTCBANK0_MINUTES            (0x02)
                                                                  /* Minutes */
#define  INDEXRTCBANK0_MINUTESALARM       (0x03)
                                                            /* Minutes alarm */
#define  INDEXRTCBANK0_HOURS              (0x04)
                                                                    /* Hours */
#define  INDEXRTCBANK0_HOURSALARM         (0x05)
                                                              /* Hours alarm */
#define  INDEXRTCBANK0_DAYOFWEEK          (0x06)
                                                              /* Day of week */
#define  INDEXRTCBANK0_DAYOFMONTH         (0x07)
                                                             /* Day of month */
#define  INDEXRTCBANK0_MONTH              (0x08)
                                                                    /* Month */
#define  INDEXRTCBANK0_YEAR               (0x09)
                                                                     /* Year */
#define  INDEXRTCBANK0_REGISTERA          (0x0A)
                                                               /* Register A */
#define  INDEXRTCBANK0_REGISTERB          (0x0B)
                                                               /* Register B */
#define  INDEXRTCBANK0_REGISTERC          (0x0C)
                                                               /* Register C */
#define  INDEXRTCBANK0_REGISTERD          (0x0D)
                                                               /* Register D */

#define  INDEXRTCBANK1_MODELBYTE          (0x40)
                                                               /* Model byte */
#define  INDEXRTCBANK1_SERIALBYTE1        (0x41)
                                                        /* 1st byte serial # */
#define  INDEXRTCBANK1_SERIALBYTE2        (0x42)
                                                        /* 2nd byte serial # */
#define  INDEXRTCBANK1_SERIALBYTE3        (0x43)
                                                        /* 3rd byte serial # */
#define  INDEXRTCBANK1_SERIALBYTE4        (0x44)
                                                        /* 4th byte serial # */
#define  INDEXRTCBANK1_SERIALBYTE5        (0x45)
                                                        /* 5th byte serial # */
#define  INDEXRTCBANK1_SERIALBYTE6        (0x46)
                                                        /* 6th byte serial # */
#define  INDEXRTCBANK1_CRCBYTE            (0x47)
                                                                 /* CRC byte */
#define  INDEXRTCBANK1_CENTURY            (0x48)
                                                             /* Century byte */
#define  INDEXRTCBANK1_DATEALARM          (0x49)
                                                               /* Date alarm */
#define  INDEXRTCBANK1_EXTREGISTERA       (0x4A)
                                              /* Extended control register A */
#define  INDEXRTCBANK1_EXTREGISTERB       (0x4B)
                                              /* Extended control register B */
#define  INDEXRTCBANK1_EXTRAMADDRESSL     (0x50)
                                                 /* Extended RAM address LSB */
#define  INDEXRTCBANK1_EXTRAMADDRESSM     (0x51)
                                                 /* Extended RAM address MSB */
#define  INDEXRTCBANK1_EXTRAMDATA         (0x53)
                                                   /* Extended RAM data port */

/* ----------------------------------- */
/*  Register A (REGISTERA) bit assign  */
/* ----------------------------------- */
#define  BITRTCREGA_RS0                   (0x01)
                                               /* (Bit-0) Rate selection     */
#define  BITRTCREGA_RS1                   (0x02)
                                               /* (Bit-1) Rate selection     */
#define  BITRTCREGA_RS2                   (0x04)
                                               /* (Bit-2) Rate selection     */
#define  BITRTCREGA_RS3                   (0x08)
                                               /* (Bit-3) Rate selection     */
#define  BITRTCREGA_DV0                   (0x10)
                                               /* (Bit-4) Bank select        */
#define  BITRTCREGA_DV1                   (0x20)
                                               /* (Bit-5) Oscillator enable  */
#define  BITRTCREGA_DV2                   (0x40)
                                               /* (Bit-6) Countdown chain    */
#define  BITRTCREGA_UIP                   (0x80)
                                               /* (Bit-7) Update in progress */

/* ----------------------------------- */
/*  Register B (REGISTERB) bit assign  */
/* ----------------------------------- */
#define  BITRTCREGB_DSE                   (0x01)
                                    /* (Bit-0) Daylight savings enable       */
#define  BITRTCREGB_2412                  (0x02)
                                    /* (Bit-1) 24/12 control                 */
#define  BITRTCREGB_DM                    (0x04)
                                    /* (Bit-2) Data mode                     */
#define  BITRTCREGB_SQWE                  (0x08)
                                    /* (Bit-3) Square wave enable            */
#define  BITRTCREGB_UIE                   (0x10)
                                    /* (Bit-4) Update ended interrupt enable */
#define  BITRTCREGB_AIE                   (0x20)
                                    /* (Bit-5) Alarm interrupt enable        */
#define  BITRTCREGB_PIE                   (0x40)
                                    /* (Bit-6) Periodic interrupt enable     */
#define  BITRTCREGB_SET                   (0x80)
                                    /* (Bit-7) SET                           */

/* ----------------------------------- */
/*  Register C (REGISTERC) bit assign  */
/* ----------------------------------- */
#define  BITRTCREGC_RESERVED0             (0x01)
                                      /* (Bit-0) Reserved                    */
#define  BITRTCREGC_RESERVED1             (0x02)
                                      /* (Bit-1) Reserved                    */
#define  BITRTCREGC_RESERVED2             (0x04)
                                      /* (Bit-2) Reserved                    */
#define  BITRTCREGC_RESERVED3             (0x08)
                                      /* (Bit-3) Reserved                    */
#define  BITRTCREGC_UF                    (0x10)
                                      /* (Bit-4) Update ended interrupt flag */
#define  BITRTCREGC_AF                    (0x20)
                                      /* (Bit-5) Alarm interrupt flag        */
#define  BITRTCREGC_PF                    (0x40)
                                      /* (Bit-6) Periodic interrupt flag     */
#define  BITRTCREGC_IRQF                  (0x80)
                                      /* (Bit-7) Interrupt request flag      */

/* ----------------------------------- */
/*  Register D (REGISTERD) bit assign  */
/* ----------------------------------- */
#define  BITRTCREGD_RESERVED0             (0x01)
                                               /* (Bit-0) Reserved           */
#define  BITRTCREGD_RESERVED1             (0x02)
                                               /* (Bit-1) Reserved           */
#define  BITRTCREGD_RESERVED2             (0x04)
                                               /* (Bit-2) Reserved           */
#define  BITRTCREGD_RESERVED3             (0x08)
                                               /* (Bit-3) Reserved           */
#define  BITRTCREGD_RESERVED4             (0x10)
                                               /* (Bit-4) Reserved           */
#define  BITRTCREGD_RESERVED5             (0x20)
                                               /* (Bit-5) Reserved           */
#define  BITRTCREGD_RESERVED6             (0x40)
                                               /* (Bit-6) Reserved           */
#define  BITRTCREGD_VRT                   (0x80)
                                               /* (Bit-7) Valid RAM and time */

/* ------------------------------------------------------- */
/*  Extended control register A (EXTREGISTERA) bit assign  */
/* ------------------------------------------------------- */
#define  BITRTCEXTREGA_KF                 (0x01)
                                     /* (Bit-0) Kickstart flag               */
#define  BITRTCEXTREGA_WF                 (0x02)
                                     /* (Bit-1) Wake up alarm flag           */
#define  BITRTCEXTREGA_RF                 (0x04)
                                     /* (Bit-2) RAM clear flag               */
#define  BITRTCEXTREGA_PAB                (0x08)
                                     /* (Bit-3) Power active bar control     */
#define  BITRTCEXTREGA_RESERVED4          (0x10)
                                     /* (Bit-4) Reserved                     */
#define  BITRTCEXTREGA_RESERVED5          (0x20)
                                     /* (Bit-5) Reserved                     */
#define  BITRTCEXTREGA_INCR               (0x40)
                                     /* (Bit-6) Increment in progress status */
#define  BITRTCEXTREGA_VRT2               (0x80)
                                     /* (Bit-7) Auxiliary battery            */

/* ------------------------------------------------------- */
/*  Extended control register B (EXTREGISTERB) bit assign  */
/* ------------------------------------------------------- */
#define  BITRTCEXTREGB_KSE                (0x01)
                                   /* (Bit-0) Kickstart interrupt enable     */
#define  BITRTCEXTREGB_WIE                (0x02)
                                   /* (Bit-1) Wake up alarm interrupt enable */
#define  BITRTCEXTREGB_RIE                (0x04)
                                   /* (Bit-2) RAM clear interrupt enable     */
#define  BITRTCEXTREGB_RESERVED3          (0x08)
                                   /* (Bit-3) Reserved                       */
#define  BITRTCEXTREGB_RCE                (0x10)
                                   /* (Bit-4) RAM clear enable               */
#define  BITRTCEXTREGB_RESERVED5          (0x20)
                                   /* (Bit-5) Reserved                       */
#define  BITRTCEXTREGB_E32K               (0x40)
                                   /* (Bit-6) Enable 32,768 output           */
#define  BITRTCEXTREGB_ABE                (0x80)
                                   /* (Bit-7) Auxiliary battery enable       */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  WAITTIME_RTCUPDATE               (100)
                                                /* Update time-out wait time */
                                                /* (100 msec)                */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     RTC Controller Definition     * */
/* ************************************* */
/* --------------- */
/*  RTC I/O class  */
/* --------------- */
VOID  RtcIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj);                  /* Pointer to Common object */

/* ------------------- */
/*  RTC control class  */
/* ------------------- */
VOID  RtcCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            RTC Objects            * */
/* ************************************* */
/* --------------- */
/*  RTC I/O class  */
/* --------------- */
OBJRTCIO    oRtcIo  = {(POBJCTL)RtcIo, PORTISA_BASE};

/* ------------------- */
/*  RTC control class  */
/* ------------------- */
OBJRTCCTL   oRtcCtl = {(POBJCTL)RtcCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* **************************************** */
/* *          RTC I/O Controller          * */
/* **************************************** */
/* *                                      * */
/* * - Initialize                         * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_INITIALIZE * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read user RAM                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_READ_RAM   * */
/* *       Parameter1 = address           * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write user RAM                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_WRITE_RAM  * */
/* *       Parameter1 = address           * */
/* *       Parameter2 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read & Modify user RAM             * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_MODIFY_RAM * */
/* *       Parameter1 = address           * */
/* *       Parameter2 = set bits value    * */
/* *       Parameter3 = mask bits value   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read register                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_READ_xxx   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write register                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_WRITE_xxx  * */
/* *       Parameter1 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read & Modify register             * */
/* *     [ Input ]                        * */
/* *       Message    = MSGRTC_MODIFY_xxx * */
/* *       Parameter1 = set bits value    * */
/* *       Parameter2 = mask bits value   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* **************************************** */
VOID  RtcIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj)                   /* Pointer to Common object */
{
   BYTE        index, srega;
   CRESULT     rc;
   POBJRTCIO   prtcio;
   DWORD       indexport, dataport;

   /* Get pointer to RTC I/O object */
   /*                               */
   prtcio = (POBJRTCIO)poObj;

   /* Get index/data port address */
   /*                             */
   indexport = GetIsaBaseAddress() + PORTRTC_INDEX;
   dataport  = GetIsaBaseAddress() + PORTRTC_DATA;

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGRTC_INITIALIZE:
         /* Initialize */
         /*            */
         prtcio->BaseAddress = GetIsaBaseAddress();
         rc = ERRRTC_NOERROR;
         break;

      default:
         /* Get index register */
         /*                    */
         switch(SelectMessage(*pmMsg))
         {
            case MSGRTC_READ_RAM:
            case MSGRTC_WRITE_RAM:
            case MSGRTC_MODIFY_RAM:
               index = (BYTE)SelectParm1(*pmMsg);
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_SECONDS:
            case MSGRTC_WRITE_SECONDS:
               index = INDEXRTCBANK0_SECONDS;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_SECONDSALARM:
            case MSGRTC_WRITE_SECONDSALARM:
               index = INDEXRTCBANK0_SECONDSALARM;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_MINUTES:
            case MSGRTC_WRITE_MINUTES:
               index = INDEXRTCBANK0_MINUTES;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_MINUTESALARM:
            case MSGRTC_WRITE_MINUTESALARM:
               index = INDEXRTCBANK0_MINUTESALARM;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_HOURS:
            case MSGRTC_WRITE_HOURS:
               index = INDEXRTCBANK0_HOURS;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_HOURSALARM:
            case MSGRTC_WRITE_HOURSALARM:
               index = INDEXRTCBANK0_HOURSALARM;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_DAYOFWEEK:
            case MSGRTC_WRITE_DAYOFWEEK:
               index = INDEXRTCBANK0_DAYOFWEEK;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_DAYOFMONTH:
            case MSGRTC_WRITE_DAYOFMONTH:
               index = INDEXRTCBANK0_DAYOFMONTH;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_MONTH:
            case MSGRTC_WRITE_MONTH:
               index = INDEXRTCBANK0_MONTH;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_YEAR:
            case MSGRTC_WRITE_YEAR:
               index = INDEXRTCBANK0_YEAR;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_REGISTERA:
            case MSGRTC_WRITE_REGISTERA:
            case MSGRTC_MODIFY_REGISTERA:
               index = INDEXRTCBANK0_REGISTERA;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_REGISTERB:
            case MSGRTC_WRITE_REGISTERB:
            case MSGRTC_MODIFY_REGISTERB:
               index = INDEXRTCBANK0_REGISTERB;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_REGISTERC:
               index = INDEXRTCBANK0_REGISTERC;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_REGISTERD:
               index = INDEXRTCBANK0_REGISTERD;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_CENTURY:
            case MSGRTC_WRITE_CENTURY:
               index = INDEXRTCBANK1_CENTURY;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_DATEALARM:
            case MSGRTC_WRITE_DATEALARM:
               index = INDEXRTCBANK1_DATEALARM;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_EXTREGISTERA:
            case MSGRTC_WRITE_EXTREGISTERA:
            case MSGRTC_MODIFY_EXTREGISTERA:
               index = INDEXRTCBANK1_EXTREGISTERA;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_EXTREGISTERB:
            case MSGRTC_WRITE_EXTREGISTERB:
            case MSGRTC_MODIFY_EXTREGISTERB:
               index = INDEXRTCBANK1_EXTREGISTERB;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_EXTRAMADDRESSL:
            case MSGRTC_WRITE_EXTRAMADDRESSL:
               index = INDEXRTCBANK1_EXTRAMADDRESSL;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_EXTRAMADDRESSM:
            case MSGRTC_WRITE_EXTRAMADDRESSM:
               index = INDEXRTCBANK1_EXTRAMADDRESSM;
               rc    = ERRRTC_NOERROR;
               break;
            case MSGRTC_READ_EXTRAMDATA:
            case MSGRTC_WRITE_EXTRAMDATA:
               index = INDEXRTCBANK1_EXTRAMDATA;
               rc    = ERRRTC_NOERROR;
               break;
            default:
               rc = ERRRTC_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case MSGRTC_READ_RAM:
               case MSGRTC_READ_SECONDS:
               case MSGRTC_READ_SECONDSALARM:
               case MSGRTC_READ_MINUTES:
               case MSGRTC_READ_MINUTESALARM:
               case MSGRTC_READ_HOURS:
               case MSGRTC_READ_HOURSALARM:
               case MSGRTC_READ_DAYOFWEEK:
               case MSGRTC_READ_DAYOFMONTH:
               case MSGRTC_READ_MONTH:
               case MSGRTC_READ_YEAR:
               case MSGRTC_READ_REGISTERA:
               case MSGRTC_READ_REGISTERB:
               case MSGRTC_READ_REGISTERC:
               case MSGRTC_READ_REGISTERD:
                  WriteControllerByte(indexport, index);
                  SelectParm1(*pmMsg) =
                                       (CPARAMETER)ReadControllerByte(dataport);
                  break;
               case MSGRTC_WRITE_RAM:
                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,  (BYTE)SelectParm2(*pmMsg));
                  break;
               case MSGRTC_WRITE_SECONDS:
               case MSGRTC_WRITE_SECONDSALARM:
               case MSGRTC_WRITE_MINUTES:
               case MSGRTC_WRITE_MINUTESALARM:
               case MSGRTC_WRITE_HOURS:
               case MSGRTC_WRITE_HOURSALARM:
               case MSGRTC_WRITE_DAYOFWEEK:
               case MSGRTC_WRITE_DAYOFMONTH:
               case MSGRTC_WRITE_MONTH:
               case MSGRTC_WRITE_YEAR:
               case MSGRTC_WRITE_REGISTERA:
               case MSGRTC_WRITE_REGISTERB:
                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,  (BYTE)SelectParm1(*pmMsg));
                  break;
               case MSGRTC_MODIFY_RAM:
                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,
                                      (BYTE)((ReadControllerByte(dataport) &
                                              ~SelectParm3(*pmMsg)         ) |
                                             (SelectParm2(*pmMsg)          &
                                              SelectParm3(*pmMsg)          ) ));
                  break;
               case MSGRTC_MODIFY_REGISTERA:
               case MSGRTC_MODIFY_REGISTERB:
                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,
                                      (BYTE)((ReadControllerByte(dataport) &
                                              ~SelectParm2(*pmMsg)         ) |
                                             (SelectParm1(*pmMsg)          &
                                              SelectParm2(*pmMsg)          ) ));
                  break;
               default:
                  /* Save register A                        */
                  /*   & select extended registers (BANK 1) */
                  /*                                        */
                  WriteControllerByte(indexport, INDEXRTCBANK0_REGISTERA);
                  srega = ReadControllerByte(dataport);
                  WriteControllerByte(dataport, srega | BITRTCREGA_DV0);

                  /* Input/Output extended registers */
                  /*                                 */
                  switch(SelectMessage(*pmMsg))
                  {
                     case MSGRTC_READ_CENTURY:
                     case MSGRTC_READ_DATEALARM:
                     case MSGRTC_READ_EXTREGISTERA:
                     case MSGRTC_READ_EXTREGISTERB:
                     case MSGRTC_READ_EXTRAMADDRESSL:
                     case MSGRTC_READ_EXTRAMADDRESSM:
                     case MSGRTC_READ_EXTRAMDATA:
                        WriteControllerByte(indexport, index);
                        SelectParm1(*pmMsg) =
                                       (CPARAMETER)ReadControllerByte(dataport);
                        break;
                     case MSGRTC_WRITE_CENTURY:
                     case MSGRTC_WRITE_DATEALARM:
                     case MSGRTC_WRITE_EXTREGISTERA:
                     case MSGRTC_WRITE_EXTREGISTERB:
                     case MSGRTC_WRITE_EXTRAMADDRESSL:
                     case MSGRTC_WRITE_EXTRAMADDRESSM:
                     case MSGRTC_WRITE_EXTRAMDATA:
                        WriteControllerByte(indexport, index);
                        WriteControllerByte(dataport,
                                            (BYTE)SelectParm1(*pmMsg));
                        break;
                     case MSGRTC_MODIFY_EXTREGISTERA:
                     case MSGRTC_MODIFY_EXTREGISTERB:
                        WriteControllerByte(indexport, index);
                        WriteControllerByte(dataport,
                                      (BYTE)((ReadControllerByte(dataport) &
                                              ~SelectParm2(*pmMsg)         ) |
                                             (SelectParm1(*pmMsg)          &
                                              SelectParm2(*pmMsg)          ) ));
                        break;
                     default:
                        rc = ERRRTC_INVALID_MESSAGE;
                  } /* endswitch */

                  /* Restore register A */
                  /*                    */
                  IoDelay();
                  WriteControllerByte(indexport, INDEXRTCBANK0_REGISTERA);
                  WriteControllerByte(dataport,  srega);
            } /* endswitch */
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* **************************************** */
/* * RTC Updating Query Call-back Routine * */
/* **************************************** */
/* *                                      * */
/* *  [ Input  ] none                     * */
/* *  [ Output ] update = no zero         * */
/* *             ready  = zero            * */
/* *                                      * */
/* **************************************** */
CRESULT  RtcQueryUpdate(VOID)
{
   MSGRTC   msg;
   CRESULT  rc;

   BuildMsg(msg, MSGRTC_QUERY_UPDATE);
   SendMsg(msg, oRtcCtl);

   if(!(rc = SelectResult(msg)))
   {
      rc = (SelectParm1(msg) == PRMRTC_RTCREADY)
           ? (CRESULT)ERRRTC_NOERROR
           : (CRESULT)ERRRTC_TIMEOUT;
   } /* endif */

   return(rc);
}

/* *************************************************** */
/* *             RTC Control Controller              * */
/* *************************************************** */
/* *                                                 * */
/* * - Initialize                                    * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_INITIALIZECTL         * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Query update in progress                      * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_QUERY_UPDATE          * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = condition                    * */
/* *                    (PRMRTC_RTCREADY )           * */
/* *                    (PRMRTC_RTCUPDATE)           * */
/* *                                                 * */
/* * - Get square wave output                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_GET_SQUAREWAVEOUTPUT  * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = frequency                    * */
/* *                    (PRMRTC_SQW32768HZ)          * */
/* *                    (PRMRTC_SQWDISABLE)          * */
/* *                                                 * */
/* * - Set square wave output                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_SET_SQUAREWAVEOUTPUT  * */
/* *       Parameter1 = frequency                    * */
/* *                    (PRMRTC_SQW32768HZ)          * */
/* *                    (PRMRTC_SQWDISABLE)          * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Set auxiliary battery status                  * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_SET_AUXBATTERYSTATUS  * */
/* *       Parameter1 = status                       * */
/* *                    (PRMRTC_AUXBATENABLE )       * */
/* *                    (PRMRTC_AUXBATDISABLE)       * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Set wake-up alarm status                      * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_SET_WAKEUPALARMSTATUS * */
/* *       Parameter1 = status                       * */
/* *                    (PRMRTC_ALARMENABLE )        * */
/* *                    (PRMRTC_ALARMDISABLE)        * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Get wake-up alarm flag                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_GET_WAKEUPALARM       * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = flag                         * */
/* *                    (PRMRTC_ALARMON )            * */
/* *                    (PRMRTC_ALARMOFF)            * */
/* *                                                 * */
/* * - Set wake-up alarm flag                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_SET_WAKEUPALARM       * */
/* *       Parameter1 = flag                         * */
/* *                    (PRMRTC_ALARMON )            * */
/* *                    (PRMRTC_ALARMOFF)            * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Clear interrupt                               * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_CLEAR_INTERRUPT       * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Disable all interrupt                         * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_DISABLE_INTERRUPT     * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Get date/time                                 * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_GET_DATETIME          * */
/* *                    MSGRTC_GET_ALARMDATETIME     * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = seconds                      * */
/* *       Parameter2 = minutes                      * */
/* *       Parameter3 = hours                        * */
/* *       Parameter4 = day                          * */
/* *       Parameter5 = month                        * */
/* *       Parameter6 = year                         * */
/* *                                                 * */
/* * - Set date/time                                 * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_SET_DATETIME          * */
/* *                    MSGRTC_SET_ALARMDATETIME     * */
/* *       Parameter1 = seconds                      * */
/* *       Parameter2 = minutes                      * */
/* *       Parameter3 = hours                        * */
/* *       Parameter4 = day                          * */
/* *       Parameter5 = month                        * */
/* *       Parameter6 = year                         * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Save device context                           * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_SAVE_CONTEXT          * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Restore device context                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGRTC_RESTORE_CONTEXT       * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* *************************************************** */
VOID  RtcCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGRTC      msg;
   CRESULT     rc;
   POBJRTCCTL  prtcctl;

   /* Get pointer to RTC control object */
   /*                                   */
   prtcctl = (POBJRTCCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGRTC_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGRTC_INITIALIZE);
         SendMsg(msg, oRtcIo);

         rc = SelectResult(msg);
         break;

      case MSGRTC_QUERY_UPDATE:
         /* Query update in progress */
         /*                          */
         BuildMsg(msg, MSGRTC_READ_REGISTERA);
         SendMsg(msg, oRtcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITRTCREGA_UIP)
                                  ? (CPARAMETER)PRMRTC_RTCUPDATE
                                  : (CPARAMETER)PRMRTC_RTCREADY;
         } /* endif */
         break;

      case MSGRTC_GET_SQUAREWAVEOUTPUT:
         /* Get square wave output (SQW output) */
         /*                                     */
         BuildMsg(msg, MSGRTC_READ_REGISTERB);
         SendMsg(msg, oRtcIo);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) & BITRTCREGB_SQWE)
            {
               BuildMsg(msg, MSGRTC_READ_EXTREGISTERB);
               SendMsg(msg, oRtcIo);

               if(!(rc = SelectResult(msg)))
               {
                  SelectParm1(*pmMsg) = (SelectParm1(msg) & BITRTCEXTREGB_E32K)
                                        ? (CPARAMETER)PRMRTC_SQW32768HZ
                                        : (CPARAMETER)PRMRTC_SQWDISABLE;
               } /* endif */
            }
            else
            {
               SelectParm1(*pmMsg) = PRMRTC_SQWDISABLE;
            } /* endif */
         } /* endif */
         break;

      case MSGRTC_SET_SQUAREWAVEOUTPUT:
         /* Set square wave output (SQW output) */
         /*                                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMRTC_SQW32768HZ:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERB,
                                      BITRTCEXTREGB_E32K,
                                      BITRTCEXTREGB_E32K);
               SendMsg(msg, oRtcIo);

               if(!(rc = SelectResult(msg)))
               {
                  BuildMsgWithParm2(msg, MSGRTC_MODIFY_REGISTERB,
                                         BITRTCREGB_SQWE,
                                         BITRTCREGB_SQWE);
                  SendMsg(msg, oRtcIo);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            case PRMRTC_SQWDISABLE:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_REGISTERB,
                                      0,
                                      BITRTCREGB_SQWE);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRRTC_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGRTC_SET_AUXBATTERYSTATUS:
         /* Set auxiliary battery status */
         /*                              */
         switch(SelectParm1(*pmMsg))
         {
            case PRMRTC_AUXBATENABLE:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERB,
                                      BITRTCEXTREGB_ABE,
                                      BITRTCEXTREGB_ABE);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            case PRMRTC_AUXBATDISABLE:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERB,
                                      0,
                                      BITRTCEXTREGB_ABE);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRRTC_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGRTC_SET_WAKEUPALARMSTATUS:
         /* Set wake-up alarm status */
         /*                          */
         switch(SelectParm1(*pmMsg))
         {
            case PRMRTC_ALARMENABLE:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERB,
                                      BITRTCEXTREGB_WIE,
                                      BITRTCEXTREGB_WIE);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            case PRMRTC_ALARMDISABLE:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERB,
                                      0,
                                      BITRTCEXTREGB_WIE);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRRTC_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGRTC_GET_WAKEUPALARM:
         /* Get wake-up alarm flag */
         /*                        */
         BuildMsg(msg, MSGRTC_READ_EXTREGISTERA);
         SendMsg(msg, oRtcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITRTCEXTREGA_WF)
                                  ? (CPARAMETER)PRMRTC_ALARMON
                                  : (CPARAMETER)PRMRTC_ALARMOFF;
         } /* endif */
         break;

      case MSGRTC_SET_WAKEUPALARM:
         /* Set wake-up alarm flag */
         /*                        */
         switch(SelectParm1(*pmMsg))
         {
            case PRMRTC_ALARMON:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERA,
                                      BITRTCEXTREGA_WF,
                                      BITRTCEXTREGA_WF | BITRTCEXTREGA_PAB);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            case PRMRTC_ALARMOFF:
               BuildMsgWithParm2(msg, MSGRTC_MODIFY_EXTREGISTERA,
                                      BITRTCEXTREGA_PAB,
                                      BITRTCEXTREGA_WF | BITRTCEXTREGA_PAB);
               SendMsg(msg, oRtcIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRRTC_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGRTC_CLEAR_INTERRUPT:
         /* Clear interrupt */
         /*                 */
         BuildMsg(msg, MSGRTC_READ_REGISTERC);
         SendMsg(msg, oRtcIo);

         rc = SelectResult(msg);
         break;

      case MSGRTC_DISABLE_INTERRUPT:
         /* Disable all interrupt */
         /*                       */
         BuildMsgWithParm2(msg, MSGRTC_MODIFY_REGISTERB,
                                0,
                                BITRTCREGB_PIE |
                                BITRTCREGB_AIE |
                                BITRTCREGB_UIE );
         SendMsg(msg, oRtcIo);

         rc = SelectResult(msg);
         break;

      case MSGRTC_GET_DATETIME:
         /* Get date/time */
         /*               */
         WaitTimeCheck(WAITTIME_RTCUPDATE, RtcQueryUpdate);

         rc = ERRRTC_NOERROR;

         BuildMsg(msg, MSGRTC_READ_SECONDS);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_MINUTES);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_HOURS);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_DAYOFMONTH);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_MONTH);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_YEAR);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = SelectParm1(msg);

            BuildMsg(msg, MSGRTC_READ_CENTURY);
            SendMsg(msg, oRtcIo);
            if(!SelectResult(msg))
            {
               SelectParm6(*pmMsg) |= SelectParm1(msg) << 8;
            }
            else
            {
               rc = SelectResult(msg);
            } /* endif */
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGRTC_SET_DATETIME:
         /* Set date/time */
         /*               */
         WaitTimeCheck(WAITTIME_RTCUPDATE, RtcQueryUpdate);

         rc = ERRRTC_NOERROR;

         BuildMsgWithParm1(msg, MSGRTC_WRITE_SECONDS,
                                SelectParm1(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_MINUTES,
                                SelectParm2(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_HOURS,
                                SelectParm3(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_DAYOFMONTH,
                                SelectParm4(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_MONTH,
                                SelectParm5(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_YEAR,
                                SelectParm6(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         }
         else
         {
            BuildMsgWithParm1(msg, MSGRTC_WRITE_CENTURY,
                                   SelectParm6(*pmMsg) >> 8);
            SendMsg(msg, oRtcIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */
         } /* endif */
         break;

      case MSGRTC_GET_ALARMDATETIME:
         /* Get alarm date/time */
         /*                     */
         WaitTimeCheck(WAITTIME_RTCUPDATE, RtcQueryUpdate);

         rc = ERRRTC_NOERROR;

         BuildMsg(msg, MSGRTC_READ_SECONDSALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_MINUTESALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_HOURSALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_DATEALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         SelectParm5(*pmMsg) = 0;
         SelectParm6(*pmMsg) = 0;
         break;

      case MSGRTC_SET_ALARMDATETIME:
         /* Set alarm date/time */
         /*                     */
         WaitTimeCheck(WAITTIME_RTCUPDATE, RtcQueryUpdate);

         rc = ERRRTC_NOERROR;

         BuildMsgWithParm1(msg, MSGRTC_WRITE_SECONDSALARM,
                                SelectParm1(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_MINUTESALARM,
                                SelectParm2(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_HOURSALARM,
                                SelectParm3(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_DATEALARM,
                                SelectParm4(*pmMsg));
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGRTC_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         WaitTimeCheck(WAITTIME_RTCUPDATE, RtcQueryUpdate);

         rc = ERRRTC_NOERROR;

         BuildMsg(msg, MSGRTC_READ_SECONDSALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chAlarmSeconds = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_MINUTESALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chAlarmMinutes = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_HOURSALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chAlarmHours = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_DATEALARM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chAlarmDate = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_REGISTERA);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chRegisterA = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_REGISTERB);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chRegisterB = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_EXTREGISTERA);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chExtRegisterA = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_EXTREGISTERB);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chExtRegisterB = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_EXTRAMADDRESSL);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chExtRamAddressL = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_READ_EXTRAMADDRESSM);
         SendMsg(msg, oRtcIo);
         if(!SelectResult(msg))
         {
            prtcctl->chExtRamAddressM = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGRTC_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         WaitTimeCheck(WAITTIME_RTCUPDATE, RtcQueryUpdate);

         rc = ERRRTC_NOERROR;

         BuildMsgWithParm1(msg, MSGRTC_WRITE_SECONDSALARM,
                                prtcctl->chAlarmSeconds);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_MINUTESALARM,
                                prtcctl->chAlarmMinutes);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_HOURSALARM,
                                prtcctl->chAlarmHours);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_DATEALARM,
                                prtcctl->chAlarmDate);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_REGISTERA,
                                prtcctl->chRegisterA);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_REGISTERB,
                                prtcctl->chRegisterB);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_EXTREGISTERA,
                                prtcctl->chExtRegisterA);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_EXTREGISTERB,
                                prtcctl->chExtRegisterB);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_EXTRAMADDRESSL,
                                prtcctl->chExtRamAddressL);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGRTC_WRITE_EXTRAMADDRESSM,
                                prtcctl->chExtRamAddressM);
         SendMsg(msg, oRtcIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGRTC_CLEAR_INTERRUPT);
         SendMsg(msg, oRtcCtl);
         break;

      default:
         SendMsg(*pmMsg, oRtcIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
