/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: IDAHO Memory Controller Chip Device Control Routines
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
/* * Module Name: PDIDAHO.C                                                * */
/* * Description: IDAHO Memory Controller Chip Device Control Routines     * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDIDAHO.H/PDPCIBUS.H files should  * */
/* *              be included in this file                                 * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdidaho.h"
#include "pdpcibus.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *       IDAHO Data Definition       * */
/* ************************************* */
/* ------------------------------------- */
/*  Configuration register port address  */
/* ------------------------------------- */
#define  PORTIDHO_BASE                    (0x80000000L)
#define  PORTIDHO_BASENULL                (0xFFFFFFFFL)
                                                       /* IDAHO base address */
                                                      /* & base null address */
                                                 /* Offset from base address */
#define  PORTIDHO_PCICOMMAND              (0x04)
                                                     /* PCI command register */
#define  PORTIDHO_PCIDEVICESTATUS         (0x06)
                                               /* PCI device status register */
#define  PORTIDHO_PCIBRIDGENUMBER         (0x40)
                                               /* PCI bridge number register */
#define  PORTIDHO_PCISUBORDINATEBUSNUMBER (0x41)
                                      /* PCI subordinate bus number register */
#define  PORTIDHO_PCIDISCONNECTCOUNTER    (0x42)
                                          /* PCI disconnect counter register */
#define  PORTIDHO_PCISPECIALCYCLEADDRESS  (0x44)
                                       /* PCI special cycle address register */
#define  PORTIDHO_STARTINGADDRESS         (0x80)
                                                /* Starting address register */
#define  PORTIDHO_ENDINGADDRESS           (0x90)
                                                  /* Ending address register */
#define  PORTIDHO_MEMORYBANKENABLE        (0xA0)
                                              /* Memory bank enable register */
#define  PORTIDHO_MEMORYTIMING1           (0xA1)
                                                 /* Memory timing 1 register */
#define  PORTIDHO_MEMORYTIMING2           (0xA2)
                                                 /* Memory timing 2 register */
#define  PORTIDHO_SIMMBANKDEFINITION      (0xA4)
                                            /* SIMM bank definition register */
#define  PORTIDHO_RESERVEDB0              (0xB0)
                                                                 /* Reserved */
#define  PORTIDHO_EXTERNALCACHESTATUS     (0xB1)
                                           /* External cache status register */
#define  PORTIDHO_REFRESHCYCLEDEFINITION  (0xB4)
                                        /* Refresh cycle definition register */
#define  PORTIDHO_REFRESHTIMER            (0xB5)
                                                   /* Refresh timer register */
#define  PORTIDHO_RASWATCHDOGTIMER        (0xB6)
                                              /* RAS watchdog timer register */
#define  PORTIDHO_PCIBUSTIMER             (0xB7)
                                                   /* PCI bus timer register */
#define  PORTIDHO_LOCALBUSTIMER           (0xB8)
                                                 /* Local bus timer register */
#define  PORTIDHO_LOCALBUSIDLETIMER       (0xB9)
                                            /* Local bus idle timer register */
#define  PORTIDHO_IDAHOOPTIONS1           (0xBA)
                                                 /* IDAHO options 1 register */
#define  PORTIDHO_IDAHOOPTIONS2           (0xBB)
                                                 /* IDAHO options 2 register */
#define  PORTIDHO_ENABLEDETECTION1        (0xC0)
                                              /* Enable detection 1 register */
#define  PORTIDHO_ERRORDETECTION1         (0xC1)
                                               /* Error detection 1 register */
#define  PORTIDHO_ERRORSIMULATION1        (0xC2)
                                              /* Error simulation 1 register */
#define  PORTIDHO_CPUBUSERRORSTATUS       (0xC3)
                                            /* CPU bus error status register */
#define  PORTIDHO_ENABLEDETECTION2        (0xC4)
                                              /* Enable detection 2 register */
#define  PORTIDHO_ERRORDETECTION2         (0xC5)
                                               /* Error detection 2 register */
#define  PORTIDHO_ERRORSIMULATION2        (0xC6)
                                              /* Error simulation 2 register */
#define  PORTIDHO_PCIBUSERRORSTATUS       (0xC7)
                                            /* PCI bus error status register */
#define  PORTIDHO_CPUPCIERRORADDRESS      (0xC8)
                                           /* CPU/PCI error address register */

/* ---------------------------------------------- */
/*  PCI command register (PCICOMMAND) bit assign  */
/* ---------------------------------------------- */
#define  BITIDHOCOMMAND_IOSPACE           (0x0001)
                                      /* (Bit-0) I/O space                   */
#define  BITIDHOCOMMAND_MEMORYSPACE       (0x0002)
                                      /* (Bit-1) Memory space                */
#define  BITIDHOCOMMAND_BUSMASTER         (0x0004)
                                      /* (Bit-2) Bus master                  */
#define  BITIDHOCOMMAND_SPECIALCYCLES     (0x0008)
                                      /* (Bit-3) Special cycles              */
#define  BITIDHOCOMMAND_MEMWRINVALIDATE   (0x0010)
                                      /* (Bit-4) Memory write and invalidate */
#define  BITIDHOCOMMAND_VGAPALETTESNOOP   (0x0020)
                                      /* (Bit-5) VGA palette snoop           */
#define  BITIDHOCOMMAND_PARITYERRRESPONSE (0x0040)
                                      /* (Bit-6) Parity error response       */
#define  BITIDHOCOMMAND_WAITCYCLE         (0x0080)
                                      /* (Bit-7) Wait cycle control          */
#define  BITIDHOCOMMAND_SERRENABLE        (0x0100)
                                      /* (Bit-8) SERR# enable                */
#define  BITIDHOCOMMAND_FASTBACKTOBACK    (0x0200)
                                      /* (Bit-9) Fast back-to-back enable    */
#define  BITIDHOCOMMAND_RESERVED10        (0x0400)
                                      /* (Bit-10) Reserved                   */
#define  BITIDHOCOMMAND_RESERVED11        (0x0800)
                                      /* (Bit-11) Reserved                   */
#define  BITIDHOCOMMAND_RESERVED12        (0x1000)
                                      /* (Bit-12) Reserved                   */
#define  BITIDHOCOMMAND_RESERVED13        (0x2000)
                                      /* (Bit-13) Reserved                   */
#define  BITIDHOCOMMAND_RESERVED14        (0x4000)
                                      /* (Bit-14) Reserved                   */
#define  BITIDHOCOMMAND_RESERVED15        (0x8000)
                                      /* (Bit-15) Reserved                   */

/* --------------------------------------------------------- */
/*  PCI device status register (PCIDEVICESTATUS) bit assign  */
/* --------------------------------------------------------- */
#define  BITIDHOSTATUS_RESERVED0          (0x0001)
                                        /* (Bit-0) Reserved                  */
#define  BITIDHOSTATUS_RESERVED1          (0x0002)
                                        /* (Bit-1) Reserved                  */
#define  BITIDHOSTATUS_RESERVED2          (0x0004)
                                        /* (Bit-2) Reserved                  */
#define  BITIDHOSTATUS_RESERVED3          (0x0008)
                                        /* (Bit-3) Reserved                  */
#define  BITIDHOSTATUS_RESERVED4          (0x0010)
                                        /* (Bit-4) Reserved                  */
#define  BITIDHOSTATUS_RESERVED5          (0x0020)
                                        /* (Bit-5) Reserved                  */
#define  BITIDHOSTATUS_RESERVED6          (0x0040)
                                        /* (Bit-6) Reserved                  */
#define  BITIDHOSTATUS_FASTBACKTOBACK     (0x0080)
                                        /* (Bit-7) Fast back-to-back capable */
#define  BITIDHOSTATUS_DATAPARITY         (0x0100)
                                        /* (Bit-8) Data parity detected      */
#define  BITIDHOSTATUS_DEVSELTIMING       (0x0200)
                                        /* (Bit-9,10) DEVSEL timing          */
#define  BITIDHOSTATUS_SIGNALLEDTARGET    (0x0800)
                                        /* (Bit-11) Signalled target abort   */
#define  BITIDHOSTATUS_RECEIVEDTARGET     (0x1000)
                                        /* (Bit-12) Received target abort    */
#define  BITIDHOSTATUS_RECEIVEDMASTER     (0x2000)
                                        /* (Bit-13) Received master abort    */
#define  BITIDHOSTATUS_SIGNALLEDSYSTEM    (0x4000)
                                        /* (Bit-14) Signalled system error   */
#define  BITIDHOSTATUS_DETECTEDPARITY     (0x8000)
                                        /* (Bit-15) Detected parity error    */

/* ----------------------------------------------------------------- */
/*  External cache status register (EXTERNALCACHESTATUS) bit assign  */
/* ----------------------------------------------------------------- */
#define  BITIDHOEXTCACHE_L1ACTIVE         (0x01)
                                  /* (Bit-0) L1 active                       */
                                  /*         (0=disable, 1=enable)           */
#define  BITIDHOEXTCACHE_L2ACTIVE         (0x02)
                                  /* (Bit-1) L2 active                       */
                                  /*         (0=disable, 1=enable)           */
#define  BITIDHOEXTCACHE_L2TYPE           (0x04)
                                  /* (Bit-2) L2 cache type                   */
                                  /*         (0=write through, 1=write back) */
#define  BITIDHOEXTCACHE_LOCALBUSFIFOEN   (0x08)
                                  /* (Bit-3) Local bus FIFO enable           */
                                  /*         (0=disable, 1=enable)           */
#define  BITIDHOEXTCACHE_MEMORYBUSFIFOEN  (0x10)
                                  /* (Bit-4) Memory bus FIFO enable          */
                                  /*         (0=disable, 1=enable)           */
#define  BITIDHOEXTCACHE_RESERVED5        (0x20)
                                  /* (Bit-5) Reserved                        */
#define  BITIDHOEXTCACHE_LOCALBUSPARKING  (0x40)
                                  /* (Bit-6) Local bus parking               */
                                  /*         (0=disable, 1=enable)           */
#define  BITIDHOEXTCACHE_RESERVED7        (0x80)
                                  /* (Bit-7) Reserved                        */

/* ----------------------------------------------------------------------- */
/*  Refresh cycle definition register (REFRESHCYCLEDEFINITION) bit assign  */
/* ----------------------------------------------------------------------- */
#define  BITIDHOREFCYCLE_REFRESH          (0x01)
                        /* (Bit-0) Enable/Disable refresh                    */
                        /*         (0=disable, 1=enable)                     */
#define  BITIDHOREFCYCLE_REFRESHTYPE      (0x06)
                        /* (Bit-1,2) Refresh type (normal/low-power)         */
#define     BITIDHOREFCYCLETYPE_NONSTAGG  (0x00)
                        /*           Non-staggered RAS only / Slow refresh   */
#define     BITIDHOREFCYCLETYPE_STAGGER   (0x02)
                        /*           Staggered RAS only / Slow refresh       */
#define     BITIDHOREFCYCLETYPE_CASRASSLF (0x04)
                        /*           CAS before RAS / Self refresh           */
#define     BITIDHOREFCYCLETYPE_CASRAS    (0x06)
                        /*           CAS before RAS / CAS before RAS         */
#define  BITIDHOREFCYCLE_RRP              (0x38)
                        /* (Bit-3,5) Refresh RAS precharge pulse width (RRP) */
#define     BITIDHOREFCYCLERRP_1CLK       (0x00)
                        /*           1 CLK                                   */
#define     BITIDHOREFCYCLERRP_2CLK       (0x08)
                        /*           2 CLK                                   */
#define     BITIDHOREFCYCLERRP_3CLK       (0x10)
                        /*           3 CLK                                   */
#define     BITIDHOREFCYCLERRP_4CLK       (0x18)
                        /*           4 CLK                                   */
#define     BITIDHOREFCYCLERRP_5CLK       (0x20)
                        /*           5 CLK                                   */
#define     BITIDHOREFCYCLERRP_6CLK       (0x28)
                        /*           6 CLK                                   */
#define     BITIDHOREFCYCLERRP_7CLK       (0x30)
                        /*           7 CLK                                   */
#define     BITIDHOREFCYCLERRP_8CLK       (0x38)
                        /*           8 CLK                                   */
#define  BITIDHOREFCYCLE_SLOWRATE         (0xC0)
                        /* (Bit-6,7) Slow refresh rate                       */
#define     BITIDHOREFCYCLERATE_15USEC    (0x00)
                        /*           15   usec                               */
#define     BITIDHOREFCYCLERATE_625USEC   (0x40)
                        /*           62.5 usec                               */
#define     BITIDHOREFCYCLERATE_125USEC   (0x80)
                        /*           125  usec                               */
#define     BITIDHOREFCYCLERATE_RESERVED  (0xC0)
                        /*           Reserved                                */

/* ----------------------------------------------------- */
/*  IDAHO options 1 register (IDAHOOPTIONS1) bit assign  */
/* ----------------------------------------------------- */
#define  BITIDHOOPTIONS1_MCP_EN           (0x01)
                                         /* (Bit-0) MCP# enable              */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOOPTIONS1_TEA_EN           (0x02)
                                         /* (Bit-1) TEA# enable              */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOOPTIONS1_PCIISAIOMAP      (0x04)
                                         /* (Bit-2) PCI/ISA I/O mapping      */
                                         /*         (0=spread, 1=contiguous) */
#define  BITIDHOOPTIONS1_PCIFIFOENABLE    (0x08)
                                         /* (Bit-3) PCI FIFO enable          */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOOPTIONS1_RESERVED4        (0x10)
                                         /* (Bit-4) Reserved                 */
#define  BITIDHOOPTIONS1_RESERVED5        (0x20)
                                         /* (Bit-5) Reserved                 */
#define  BITIDHOOPTIONS1_RESERVED6        (0x40)
                                         /* (Bit-6) Reserved                 */
#define  BITIDHOOPTIONS1_BUSTIMERS        (0x80)
                                         /* (Bit-7) Bus timers               */
                                         /*         (0=disable, 1=enable)    */

/* ----------------------------------------------------- */
/*  IDAHO options 2 register (IDAHOOPTIONS2) bit assign  */
/* ----------------------------------------------------- */
#define  BITIDHOOPTIONS2_FLASHWRITE       (0x01)
                                        /* (Bit-0) Flash write enable        */
                                        /*         (0=disable, 1=enable)     */
#define  BITIDHOOPTIONS2_POWERMODE        (0x02)
                                        /* (Bit-1) Power mode enable         */
                                        /*         (0=disable, 1=enable)     */
#define  BITIDHOOPTIONS2_SNOOPTYPE        (0x04)
                                        /* (Bit-2) Snoop type for PCI reads  */
                                        /*         (0=flush, 1=clean)        */
#define  BITIDHOOPTIONS2_RASWATCHDOGTIMER (0x08)
                                        /* (Bit-3) RAS watchdog timer enable */
                                        /*         (0=disable, 1=enable)     */
#define  BITIDHOOPTIONS2_603CLOCKMODE     (0x10)
                                        /* (Bit-4) 603 1:1 clock mode        */
                                        /*         (0=not 1:1, 1=1:1)        */
#define  BITIDHOOPTIONS2_RESERVED5        (0x20)
                                        /* (Bit-5) Reserved                  */
#define  BITIDHOOPTIONS2_RESERVED6        (0x40)
                                        /* (Bit-6) Reserved                  */
#define  BITIDHOOPTIONS2_RESERVED7        (0x80)
                                        /* (Bit-7) Reserved                  */

/* ----------------------------------------------------------- */
/*  Enable detection 1 register (ENABLEDETECTION1) bit assign  */
/* ----------------------------------------------------------- */
#define  BITIDHOENDETECTION1_LOCALBUSERR  (0x01)
                                         /* (Bit-0) Local bus error          */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOENDETECTION1_RESERVED1    (0x02)
                                         /* (Bit-1) Reserved                 */
#define  BITIDHOENDETECTION1_MEMPARITYERR (0x04)
                                         /* (Bit-2) Memory read parity error */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOENDETECTION1_RESERVED3    (0x08)
                                         /* (Bit-3) Reserved                 */
#define  BITIDHOENDETECTION1_REFRESHERR   (0x10)
                                         /* (Bit-4) Refresh timeout error    */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOENDETECTION1_MEMSELECTERR (0x20)
                                         /* (Bit-5) Memory select error      */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOENDETECTION1_PCIPERR      (0x40)
                                         /* (Bit-6) PCI PERR#                */
                                         /*         (0=disable, 1=enable)    */
#define  BITIDHOENDETECTION1_PCISERR      (0x80)
                                         /* (Bit-7) PCI SERR#                */
                                         /*         (0=disable, 1=enable)    */

/* --------------------------------------------------------- */
/*  Error detection 1 register (ERRORDETECTION1) bit assign  */
/* --------------------------------------------------------- */
#define  BITIDHOERRDETECT1_UNSUPPORTCYCLE (0x03)
                                /* (Bit-0,1) Unsupported local bus cycle     */
#define     BITIDHOERRDETECT1CYCLE_NOERR  (0x00)
                                /*           No error detected               */
#define     BITIDHOERRDETECT1CYCLE_TRANS  (0x01)
                                /*           Unsupported transfer attributes */
#define     BITIDHOERRDETECT1CYCLE_XATS   (0x02)
                                /*           XATS# detected                  */
#define     BITIDHOERRDETECT1CYCLE_RSVD   (0x03)
                                /*           Reserved                        */
#define  BITIDHOERRDETECT1_MEMPARITYERR   (0x04)
                                /* (Bit-2) Memory read parity error          */
                                /*         (0=no error, 1=parity error)      */
#define  BITIDHOERRDETECT1_LOCALPCICYCLE  (0x08)
                                /* (Bit-3) Local/PCI cycle                   */
                                /*         (0=local cycle, 1=PCI cycle)      */
#define  BITIDHOERRDETECT1_REFRESHERR     (0x10)
                                /* (Bit-4) Refresh timeout error             */
                                /*         (0=no error, 1=timeout error)     */
#define  BITIDHOERRDETECT1_MEMSELECTERR   (0x20)
                                /* (Bit-5) Memory select error               */
                                /*         (0=no error, 1=select error)      */
#define  BITIDHOERRDETECT1_PCIPERR        (0x40)
                                /* (Bit-6) PCI PERR#                         */
                                /*         (0=no error, 1=error)             */
#define  BITIDHOERRDETECT1_PCISERR        (0x80)
                                /* (Bit-7) PCI SERR#                         */
                                /*         (0=no error, 1=error)             */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    IDAHO Controller Definition    * */
/* ************************************* */
/* ----------------- */
/*  IDAHO I/O class  */
/* ----------------- */
VOID  IdahoIo(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj);               /* Pointer to Common object */

/* --------------------- */
/*  IDAHO control class  */
/* --------------------- */
VOID  IdahoCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *           IDAHO Objects           * */
/* ************************************* */
/* ----------------- */
/*  IDAHO I/O class  */
/* ----------------- */
OBJIDAHOIO  oIdahoIo  = {(POBJCTL)IdahoIo, PORTIDHO_BASENULL};

/* --------------------- */
/*  IDAHO control class  */
/* --------------------- */
OBJIDAHOCTL oIdahoCtl = {(POBJCTL)IdahoCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************ */
/* *             IDAHO I/O Controller             * */
/* ************************************************ */
/* *                                              * */
/* * - Initialize                                 * */
/* *     [ Input ]                                * */
/* *       Message    = MSGIDHO_INITIALIZE        * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read register                              * */
/* *     [ Input ]                                * */
/* *       Message    = MSGIDHO_READ_xxx          * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       Parameter1 = register value            * */
/* *       IdahoParm1 = register value (32bits)   * */
/* *                                              * */
/* * - Write register                             * */
/* *     [ Input ]                                * */
/* *       Message    = MSGIDHO_WRITE_xxx         * */
/* *       Parameter1 = register value (not used) * */
/* *       IdahoParm1 = register value (32bits)   * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read & Modify register                     * */
/* *     [ Input ]                                * */
/* *       Message    = MSGIDHO_MODIFY_xxx        * */
/* *       Parameter1 = register value (not used) * */
/* *       IdahoParm1 = set bits value  (32bits)  * */
/* *       IdahoParm2 = mask bits value (32bits)  * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* ************************************************ */
VOID  IdahoIo(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj)                /* Pointer to Common object */
{
   MSGPCI      mPci;
   WORD        off;
   DWORD       data;
   CRESULT     rc;
   POBJIDAHOIO pidahoio;

   /* Get pointer to IDAHO I/O object */
   /*                                 */
   pidahoio = (POBJIDAHOIO)poObj;

   /* Check if completing for initialization */
   /*                                        */
   if((pidahoio->BaseAddress != PORTIDHO_BASENULL ) ||
      (SelectMessage(*pmMsg) == MSGIDHO_INITIALIZE)   )
   {
      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGIDHO_INITIALIZE:
            /* Initialize */
            /*            */
            BuildMsg(mPci, MSGPCI_QUERY_DEVICE);
            SendMsg(mPci, oPciIdahoCtl);

            if(!(rc = SelectResult(mPci)))
            {
               BuildMsg(mPci, MSGPCI_GET_CONFIGADDRESS);
               SendMsg(mPci, oPciIdahoCtl);

               if(!(rc = SelectResult(mPci)))
               {
                  pidahoio->BaseAddress = SelectPciParm1(mPci);
               }
               else
               {
                  rc = ERRIDHO_CANNOT_INITIALIZE;
               } /* endif */
            }
            else
            {
               rc = ERRIDHO_CANNOT_INITIALIZE;
            } /* endif */
            break;

         default:
            /* Get offset to data-port register */
            /*                                  */
            switch(SelectMessage(*pmMsg))
            {
               case MSGIDHO_READ_RESERVEDB0:
               case MSGIDHO_WRITE_RESERVEDB0:
               case MSGIDHO_MODIFY_RESERVEDB0:
                  off = PORTIDHO_RESERVEDB0;
                  rc  = ERRIDHO_NOERROR;
                  break;
               case MSGIDHO_READ_REFRESHCYCLE:
               case MSGIDHO_WRITE_REFRESHCYCLE:
               case MSGIDHO_MODIFY_REFRESHCYCLE:
                  off = PORTIDHO_REFRESHCYCLEDEFINITION;
                  rc  = ERRIDHO_NOERROR;
                  break;
               case MSGIDHO_READ_LOCALBUSTIMER:
               case MSGIDHO_WRITE_LOCALBUSTIMER:
               case MSGIDHO_MODIFY_LOCALBUSTIMER:
                  off = PORTIDHO_LOCALBUSTIMER;
                  rc  = ERRIDHO_NOERROR;
                  break;
               case MSGIDHO_READ_ENABLEDETECTION1:
               case MSGIDHO_WRITE_ENABLEDETECTION1:
               case MSGIDHO_MODIFY_ENABLEDETECTION1:
                  off = PORTIDHO_ENABLEDETECTION1;
                  rc  = ERRIDHO_NOERROR;
                  break;
               case MSGIDHO_MODIFY_PCICOMMAND:
                  off = PORTIDHO_PCICOMMAND;
                  rc  = ERRIDHO_NOERROR;
                  break;
               default:
                  rc = ERRIDHO_INVALID_MESSAGE;
            } /* endswitch */

            if(!rc)
            {
               /* Input/Output registers */
               /*                        */
               switch(SelectMessage(*pmMsg))
               {
                  case MSGIDHO_READ_RESERVEDB0:
                  case MSGIDHO_READ_REFRESHCYCLE:
                  case MSGIDHO_READ_LOCALBUSTIMER:
                  case MSGIDHO_READ_ENABLEDETECTION1:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciIdahoCtl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        SelectIdahoParm1(*pmMsg) = SelectPciParm1(mPci);
                        SelectParm1(*pmMsg)      =
                                      (CPARAMETER)SelectPciParm1(mPci) & 0x00FF;
                     }
                     else
                     {
                        rc = ERRIDHO_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGIDHO_WRITE_RESERVEDB0:
                  case MSGIDHO_WRITE_REFRESHCYCLE:
                  case MSGIDHO_WRITE_LOCALBUSTIMER:
                  case MSGIDHO_WRITE_ENABLEDETECTION1:
                     BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                             off);
                     BuildPciParm1(mPci, SelectIdahoParm1(*pmMsg));
                     SendMsg(mPci, oPciIdahoCtl);

                     if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                     {
                        rc = ERRIDHO_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGIDHO_MODIFY_RESERVEDB0:
                  case MSGIDHO_MODIFY_REFRESHCYCLE:
                  case MSGIDHO_MODIFY_LOCALBUSTIMER:
                  case MSGIDHO_MODIFY_ENABLEDETECTION1:
                  case MSGIDHO_MODIFY_PCICOMMAND:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciIdahoCtl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        data = (SelectPciParm1(mPci)       &
                                ~SelectIdahoParm2(*pmMsg)  ) |
                               (SelectIdahoParm1(*pmMsg)   &
                                SelectIdahoParm2(*pmMsg)   );

                        BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                                off);
                        BuildPciParm1(mPci, data);
                        SendMsg(mPci, oPciIdahoCtl);

                        if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                        {
                           rc = ERRIDHO_CANNOT_PCIACCESS;
                        } /* endif */
                     }
                     else
                     {
                        rc = ERRIDHO_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  default:
                     rc = ERRIDHO_INVALID_MESSAGE;
               } /* endswitch */
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERRIDHO_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ***************************************************** */
/* *             IDAHO Control Controller              * */
/* ***************************************************** */
/* *                                                   * */
/* * - Initialize                                      * */
/* *     [ Input ]                                     * */
/* *       Message    = MSGIDHO_INITIALIZECTL          * */
/* *     [ Output ]                                    * */
/* *       Result     = result code                    * */
/* *                                                   * */
/* * - Set power mode                                  * */
/* *     [ Input ]                                     * */
/* *       Message    = MSGIDHO_SET_POWERMODE          * */
/* *       Parameter1 = power mode                     * */
/* *                    (PRMIDHO_POWERMODEENABLE )     * */
/* *                    (PRMIDHO_POWERMODEDISABLE)     * */
/* *     [ Output ]                                    * */
/* *       Result     = result code                    * */
/* *                                                   * */
/* * - Set cache active status                         * */
/* *     [ Input ]                                     * */
/* *       Message    = MSGIDHO_SET_L1CACHESTATUS      * */
/* *                    MSGIDHO_SET_L2CACHESTATUS      * */
/* *       Parameter1 = status                         * */
/* *                    (PRMIDHO_CACHEENABLE )         * */
/* *                    (PRMIDHO_CACHEDISABLE)         * */
/* *     [ Output ]                                    * */
/* *       Result     = result code                    * */
/* *                                                   * */
/* * - Set refresh type                                * */
/* *     [ Input ]                                     * */
/* *       Message    = MSGIDHO_SET_REFRESHTYPE        * */
/* *       Parameter1 = type                           * */
/* *                    (PRMIDHO_REFRESHNONSTAGGESLOW) * */
/* *                    (PRMIDHO_REFRESHSTAGGERSLOW  ) * */
/* *                    (PRMIDHO_REFRESHCASRASSELF   ) * */
/* *                    (PRMIDHO_REFRESHCASRASCASRAS ) * */
/* *     [ Output ]                                    * */
/* *       Result     = result code                    * */
/* *                                                   * */
/* * - Save device context                             * */
/* *     [ Input ]                                     * */
/* *       Message    = MSGIDHO_SAVE_CONTEXT           * */
/* *     [ Output ]                                    * */
/* *       Result     = result code                    * */
/* *                                                   * */
/* * - Restore device context                          * */
/* *     [ Input ]                                     * */
/* *       Message    = MSGIDHO_RESTORE_CONTEXT        * */
/* *     [ Output ]                                    * */
/* *       Result     = result code                    * */
/* *                                                   * */
/* ***************************************************** */
VOID  IdahoCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   MSGIDAHO       msg;
   CRESULT        rc;
   POBJIDAHOCTL   pidahoctl;

   /* Get pointer to IDAHO control object */
   /*                                     */
   pidahoctl = (POBJIDAHOCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGIDHO_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGIDHO_INITIALIZE);
         SendMsg(msg, oIdahoIo);

         rc = SelectResult(msg);
         break;

      case MSGIDHO_SET_POWERMODE:
         /* Set power mode */
         /*                */
         switch(SelectParm1(*pmMsg))
         {
            case PRMIDHO_POWERMODEENABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_LOCALBUSTIMER);
               BuildIdahoParm2(msg, (DWORD)BITIDHOOPTIONS2_POWERMODE
                                    << ((PORTIDHO_IDAHOOPTIONS2 -
                                         PORTIDHO_LOCALBUSTIMER ) * 8),
                                    (DWORD)BITIDHOOPTIONS2_POWERMODE
                                    << ((PORTIDHO_IDAHOOPTIONS2 -
                                    PORTIDHO_LOCALBUSTIMER      ) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_POWERMODEDISABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_LOCALBUSTIMER);
               BuildIdahoParm2(msg, 0,
                                    (DWORD)BITIDHOOPTIONS2_POWERMODE
                                    << ((PORTIDHO_IDAHOOPTIONS2 -
                                         PORTIDHO_LOCALBUSTIMER ) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRIDHO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGIDHO_SET_L1CACHESTATUS:
         /* Set L1 cache active status */
         /*                            */
         switch(SelectParm1(*pmMsg))
         {
            case PRMIDHO_CACHEENABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_RESERVEDB0);
               BuildIdahoParm2(msg, (DWORD)BITIDHOEXTCACHE_L1ACTIVE
                                    << ((PORTIDHO_EXTERNALCACHESTATUS -
                                         PORTIDHO_RESERVEDB0          ) * 8),
                                    (DWORD)BITIDHOEXTCACHE_L1ACTIVE
                                    << ((PORTIDHO_EXTERNALCACHESTATUS -
                                         PORTIDHO_RESERVEDB0          ) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_CACHEDISABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_RESERVEDB0);
               BuildIdahoParm2(msg, 0,
                                    (DWORD)BITIDHOEXTCACHE_L1ACTIVE
                                    << ((PORTIDHO_EXTERNALCACHESTATUS -
                                         PORTIDHO_RESERVEDB0          ) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRIDHO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGIDHO_SET_L2CACHESTATUS:
         /* Set L2 cache active status */
         /*                            */
         switch(SelectParm1(*pmMsg))
         {
            case PRMIDHO_CACHEENABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_RESERVEDB0);
               BuildIdahoParm2(msg, (DWORD)BITIDHOEXTCACHE_L2ACTIVE
                                    << ((PORTIDHO_EXTERNALCACHESTATUS -
                                         PORTIDHO_RESERVEDB0          ) * 8),
                                    (DWORD)BITIDHOEXTCACHE_L2ACTIVE
                                    << ((PORTIDHO_EXTERNALCACHESTATUS -
                                         PORTIDHO_RESERVEDB0          ) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_CACHEDISABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_RESERVEDB0);
               BuildIdahoParm2(msg, 0,
                                    (DWORD)BITIDHOEXTCACHE_L2ACTIVE
                                    << ((PORTIDHO_EXTERNALCACHESTATUS -
                                         PORTIDHO_RESERVEDB0          ) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRIDHO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGIDHO_SET_REFRESHTYPE:
         /* Set refresh type */
         /*                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMIDHO_REFRESHNONSTAGGERSLOW:
               BuildMsg(msg, MSGIDHO_MODIFY_REFRESHCYCLE);
               BuildIdahoParm2(msg, (DWORD)(BITIDHOREFCYCLETYPE_NONSTAGG |
                                            BITIDHOREFCYCLERATE_15USEC   )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION ) * 8),
                                    (DWORD)(BITIDHOREFCYCLE_REFRESHTYPE |
                                            BITIDHOREFCYCLE_SLOWRATE    )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_REFRESHSTAGGERSLOW:
               BuildMsg(msg, MSGIDHO_MODIFY_REFRESHCYCLE);
               BuildIdahoParm2(msg, (DWORD)(BITIDHOREFCYCLETYPE_STAGGER |
                                            BITIDHOREFCYCLERATE_15USEC  )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION ) * 8),
                                    (DWORD)(BITIDHOREFCYCLE_REFRESHTYPE |
                                            BITIDHOREFCYCLE_SLOWRATE    )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_REFRESHCASRASSELF:
               BuildMsg(msg, MSGIDHO_MODIFY_REFRESHCYCLE);
               BuildIdahoParm2(msg, (DWORD)(BITIDHOREFCYCLETYPE_CASRASSLF |
                                            BITIDHOREFCYCLERATE_15USEC    )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION ) * 8),
                                    (DWORD)(BITIDHOREFCYCLE_REFRESHTYPE |
                                            BITIDHOREFCYCLE_SLOWRATE    )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_REFRESHCASRASCASRAS:
               BuildMsg(msg, MSGIDHO_MODIFY_REFRESHCYCLE);
               BuildIdahoParm2(msg, (DWORD)(BITIDHOREFCYCLETYPE_CASRAS |
                                            BITIDHOREFCYCLERATE_15USEC )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION ) * 8),
                                    (DWORD)(BITIDHOREFCYCLE_REFRESHTYPE |
                                            BITIDHOREFCYCLE_SLOWRATE    )
                                    << ((PORTIDHO_REFRESHCYCLEDEFINITION -
                                         PORTIDHO_REFRESHCYCLEDEFINITION) * 8));
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRIDHO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGIDHO_SET_BUSPARITYRESPONSE:
         /* Parity error response on PCI bus */
         /*                                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMIDHO_BUSPARITYENABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_PCICOMMAND);
               BuildIdahoParm2(msg, (DWORD)BITIDHOCOMMAND_PARITYERRRESPONSE
                                    << ((PORTIDHO_PCICOMMAND -
                                         PORTIDHO_PCICOMMAND ) * 8),
                                    (DWORD)BITIDHOCOMMAND_PARITYERRRESPONSE
                                    << ((PORTIDHO_PCICOMMAND -
                                         PORTIDHO_PCICOMMAND ) * 8)        );
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            case PRMIDHO_BUSPARITYDISABLE:
               BuildMsg(msg, MSGIDHO_MODIFY_PCICOMMAND);
               BuildIdahoParm2(msg, 0,
                                    (DWORD)BITIDHOCOMMAND_PARITYERRRESPONSE
                                    << ((PORTIDHO_PCICOMMAND -
                                         PORTIDHO_PCICOMMAND ) * 8)        );
               SendMsg(msg, oIdahoIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRIDHO_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGIDHO_CLEAR_ERRORSTATUS:
         /* Clear error status */
         /*                    */
         BuildMsg(msg, MSGIDHO_MODIFY_PCICOMMAND);
         BuildIdahoParm2(msg, (DWORD)(BITIDHOSTATUS_DETECTEDPARITY  |
                                      BITIDHOSTATUS_SIGNALLEDSYSTEM |
                                      BITIDHOSTATUS_RECEIVEDMASTER  |
                                      BITIDHOSTATUS_RECEIVEDTARGET  |
                                      BITIDHOSTATUS_SIGNALLEDTARGET )
                              << ((PORTIDHO_PCIDEVICESTATUS -
                                   PORTIDHO_PCICOMMAND      ) * 8    ),
                              (DWORD)(BITIDHOSTATUS_DETECTEDPARITY  |
                                      BITIDHOSTATUS_SIGNALLEDSYSTEM |
                                      BITIDHOSTATUS_RECEIVEDMASTER  |
                                      BITIDHOSTATUS_RECEIVEDTARGET  |
                                      BITIDHOSTATUS_SIGNALLEDTARGET )
                              << ((PORTIDHO_PCIDEVICESTATUS -
                                   PORTIDHO_PCICOMMAND      ) * 8)   );
         SendMsg(msg, oIdahoIo);

         rc = SelectResult(msg);
         break;

      case MSGIDHO_ENTER_SUSPEND:
         /* Enter suspend */
         /*               */
         rc = ERRIDHO_NOERROR;

         BuildMsgWithParm1(msg, MSGIDHO_SET_POWERMODE,
                                PRMIDHO_POWERMODEENABLE);
         SendMsg(msg, oIdahoCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGIDHO_SET_BUSPARITYRESPONSE,
                                PRMIDHO_BUSPARITYDISABLE);
         SendMsg(msg, oIdahoCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         /* BuildMsgWithParm1(msg, MSGIDHO_SET_REFRESHTYPE,      */
         /*                        PRMIDHO_REFRESHCASRASCASRAS); */
         /* SendMsg(msg, oIdahoCtl);                             */
         /* if(SelectResult(msg))                                */
         /* {                                                    */
         /*    rc = SelectResult(msg);                           */
         /* } */ /* endif */
         break;

      case MSGIDHO_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRIDHO_NOERROR;

         BuildMsg(msg, MSGIDHO_READ_RESERVEDB0);
         SendMsg(msg, oIdahoIo);
         if(!SelectResult(msg))
         {
            pidahoctl->dwOffsetB0h = SelectIdahoParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGIDHO_READ_REFRESHCYCLE);
         SendMsg(msg, oIdahoIo);
         if(!SelectResult(msg))
         {
            pidahoctl->dwOffsetB4h = SelectIdahoParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGIDHO_READ_LOCALBUSTIMER);
         SendMsg(msg, oIdahoIo);
         if(!SelectResult(msg))
         {
            pidahoctl->dwOffsetB8h = SelectIdahoParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGIDHO_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRIDHO_NOERROR;

         BuildMsg(msg, MSGIDHO_WRITE_RESERVEDB0);
         BuildIdahoParm1(msg, pidahoctl->dwOffsetB0h);
         SendMsg(msg, oIdahoIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGIDHO_WRITE_REFRESHCYCLE);
         BuildIdahoParm1(msg, pidahoctl->dwOffsetB4h);
         SendMsg(msg, oIdahoIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGIDHO_WRITE_LOCALBUSTIMER);
         BuildIdahoParm1(msg, pidahoctl->dwOffsetB8h);
         SendMsg(msg, oIdahoIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGIDHO_CLEAR_ERRORSTATUS);
         SendMsg(msg, oIdahoCtl);
         break;

      default:
         SendMsg(*pmMsg, oIdahoIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
