/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Universal Micro Control Unit (UMCU) Chip Device Control
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
/* * Module Name: PDUMCU.C                                                 * */
/* * Description: Universal Micro Control Unit (UMCU) Chip Device Control  * */
/* *              Routines                                                 * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDUMCU.H files should be included  * */
/* *              in this file                                             * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdumcu.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *       UMCU Data Definition        * */
/* ************************************* */
/* ---------------------------------------- */
/*  Power management register port address  */
/* ---------------------------------------- */
#define  PORTUMCU_BASE                    (0x000015E8L) /* UMCU base address */
#define  PORTUMCU_INDEX                   (PORTUMCU_BASE + 0)
                                                   /* Index address register */
#define  PORTUMCU_DATA                    (PORTUMCU_BASE + 1)
                                                            /* Data register */
#define  PORTUMCU_BSR                     (PORTUMCU_BASE + 2)
                                                    /* Basic status register */

/* ------------------------------------ */
/*  Index data to index UMCU registers  */
/* ------------------------------------ */
#define  INDEXUMCU_BCR                    (0x01)
                                                   /* Basic control register */
#define  INDEXUMCU_SCR                    (0x02)
                                                  /* System command register */
#define  INDEXUMCU_SDR                    (0x03)
                                                     /* System data register */
#define  INDEXUMCU_PCR                    (0x04)
                                                     /* PMC command register */
#define  INDEXUMCU_PSR1                   (0x05)
                                                    /* PMC status register 1 */
#define  INDEXUMCU_RESERVED06             (0x06)
                                                                 /* Reserved */
#define  INDEXUMCU_RESERVED07             (0x07)
                                                                 /* Reserved */
#define  INDEXUMCU_PDR0                   (0x08)
                                                      /* PMC data register 0 */
#define  INDEXUMCU_PDR1                   (0x09)
                                                      /* PMC data register 1 */
#define  INDEXUMCU_PDR2                   (0x0A)
                                                      /* PMC data register 2 */
#define  INDEXUMCU_PDR3                   (0x0B)
                                                      /* PMC data register 3 */
#define  INDEXUMCU_PEDR0                  (0x0C)
                                             /* PMC extended data register 0 */
#define  INDEXUMCU_PEDR1                  (0x0D)
                                             /* PMC extended data register 1 */
#define  INDEXUMCU_PEDR2                  (0x0E)
                                             /* PMC extended data register 2 */
#define  INDEXUMCU_PEDR3                  (0x0F)
                                             /* PMC extended data register 3 */
#define  INDEXUMCU_PEDR4                  (0x10)
                                             /* PMC extended data register 4 */
#define  INDEXUMCU_PEDR5                  (0x11)
                                             /* PMC extended data register 5 */
#define  INDEXUMCU_PSR2                   (0x12)
                                                    /* PMC status register 2 */
#define  INDEXUMCU_BBR0                   (0x13)
                                                /* Bulletin-board register 0 */
#define  INDEXUMCU_BBR1                   (0x14)
                                                /* Bulletin-board register 1 */
#define  INDEXUMCU_BBR2                   (0x15)
                                                /* Bulletin-board register 2 */
#define  INDEXUMCU_PSR3                   (0x16)
                                                    /* PMC status register 3 */
#define  INDEXUMCU_BBR3                   (0x17)
                                                /* Bulletin-board register 3 */
#define  INDEXUMCU_PMMONITOR0             (0x18)
                                                    /* PM monitor register 0 */
#define  INDEXUMCU_PMMONITOR1             (0x19)
                                                    /* PM monitor register 1 */
#define  INDEXUMCU_MEMACCESS1             (0x1A)
                                                  /* Memory access counter 1 */
#define  INDEXUMCU_MEMACCESS2             (0x1B)
                                                  /* Memory access counter 2 */
#define  INDEXUMCU_PMCONTROL              (0x1C)
                                                      /* PM control register */
#define  INDEXUMCU_MEMACCESS3             (0x1D)
                                                  /* Memory access counter 3 */
#define  INDEXUMCU_EVENTSTATUS            (0x1E)
                                                    /* Event status register */
#define  INDEXUMCU_EVENTENABLE            (0x1F)
                                          /* Event interrupt enable register */
#define  INDEXUMCU_MAX                    (0x20)
                                                       /* Maximum index size */

/* ---------------------------------------- */
/*  Basic status register (BSR) bit assign  */
/* ---------------------------------------- */
#define  BITUMCUBSR_PMCBUSY               (0x01)
                               /* (Bit-0) PMC busy                           */
                               /*         (1=busy, 0=ready)                  */
#define  BITUMCUBSR_PMCENABLE             (0x02)
                               /* (Bit-1) PMC interrupt enable               */
                               /*         (1=enable, 0=POR)                  */
#define  BITUMCUBSR_PMCINTR               (0x04)
                               /* (Bit-2) PMC interrupt                      */
                               /*         (1=written by host, 0=read by PMC) */
#define  BITUMCUBSR_FIFOEMPTY             (0x08)
                               /* (Bit-3) Data buffer empty                  */
                               /*         (0=empty, 1=POR)                   */
#define  BITUMCUBSR_FIFOFULL              (0x10)
                               /* (Bit-4) Data buffer full                   */
                               /*         (1=full, 0=POR)                    */
#define  BITUMCUBSR_RESERVED5             (0x20)
                               /* (Bit-5) Reserved                           */
#define  BITUMCUBSR_PMI                   (0x40)
                               /* (Bit-6) External PM interrupt from PMC     */
                               /*         (1=active)                         */
#define  BITUMCUBSR_RESERVED7             (0x80)
                               /* (Bit-7) Reserved                           */

/* ----------------------------------------- */
/*  Basic control register (BCR) bit assign  */
/* ----------------------------------------- */
#define  BITUMCUBCR_PMCRESET              (0x01)
                                     /* (Bit-0) PMC interface reset          */
#define  BITUMCUBCR_RESERVED1             (0x02)
                                     /* (Bit-1) Reserved                     */
#define  BITUMCUBCR_PMIENABLE             (0x04)
                                     /* (Bit-2) External PM interrupt enable */
                                     /*         (1=enable)                   */
#define  BITUMCUBCR_RESERVED3             (0x08)
                                     /* (Bit-3) Reserved                     */
#define  BITUMCUBCR_RESERVED4             (0x10)
                                     /* (Bit-4) Reserved                     */
#define  BITUMCUBCR_RESERVED5             (0x20)
                                     /* (Bit-5) Reserved                     */
#define  BITUMCUBCR_RESERVED6             (0x40)
                                     /* (Bit-6) Reserved                     */
#define  BITUMCUBCR_RESERVED7             (0x80)
                                     /* (Bit-7) Reserved                     */

/* ------------------------------- */
/*  Device object identifications  */
/* ------------------------------- */
#define  DEVICEIDUMCU_MAINPOWER           (0x80)
                               /* Main power switch                          */
#define     BITDEVICEUMCUMAINPOWER_PUSH   (0x01)
                               /*    (Bit-0) Push                            */
#define  DEVICEIDUMCU_BEEP                (0x83)
                               /* Beep                                       */
#define  DEVICEIDUMCU_KEYBOARD            (0x85)
                               /* Keyboard                                   */
#define     BITDEVICEUMCUKBD_ACTIVE       (0x01)
                               /*    (Bit-0)  Any key activity               */
#define     BITDEVICEUMCUKBD_PLUGIN       (0x02)
                               /*    (Bit-1)  External keyboard plug-in      */
#define     BITDEVICEUMCUKBD_PLUGOUT      (0x04)
                               /*    (Bit-2)  External keyboard plug-out     */
#define     BITDEVICEUMCUKBD_ANYKEYRESUME (0x08)
                               /*    (Bit-3)  Any key resume                 */
#define     BITDEVICEUMCUKBD_EVRESET      (0x80)
                               /*    (Bit-15) Event status reset             */
#define  DEVICEIDUMCU_BATTERY             (0x87)
                               /* Battery                                    */
#define  DEVICEIDUMCU_KEYBOARDCOVER       (0x8C)
                               /* Keyboard cover                             */
#define     BITDEVICEUMCUKBDCOVER_CLOSE   (0x01)
                               /*    (Bit-0)  Close                          */
#define     BITDEVICEUMCUKBDCOVER_OPEN    (0x02)
                               /*    (Bit-1)  Open                           */
#define     BITDEVICEUMCUKBDCOVER_EVRESET (0x80)
                               /*    (Bit-15) Event status reset             */
#define  DEVICEIDUMCU_LID                 (0x8D)
                               /* LID                                        */
#define     BITDEVICEUMCULID_OPEN         (0x01)
                               /*    (Bit-0)  Open                           */
#define     BITDEVICEUMCULID_CLOSE        (0x02)
                               /*    (Bit-1)  Close                          */
#define     BITDEVICEUMCULID_EVRESET      (0x80)
                               /*    (Bit-15) Event status reset             */
#define  DEVICEIDUMCU_IPD                 (0x8E)
                               /* Internal pointing device                   */
#define     BITDEVICEUMCUIPD_CLICKRELEASE (0x01)
                               /*    (Bit-0)  Click released                 */
#define     BITDEVICEUMCUIPD_CLICKDEPRESS (0x02)
                               /*    (Bit-1)  Click depressed                */
#define     BITDEVICEUMCUIPD_EVRESET      (0x80)
                               /*    (Bit-15) Event status reset             */
#define  DEVICEIDUMCU_POWERLED            (0x90)
                               /* Power LED                                  */
#define  DEVICEIDUMCU_SUSPENDLED          (0x91)
                               /* Suspend LED                                */
#define  DEVICEIDUMCU_POWERMONITOR        (0x96)
                               /* Power monitor                              */
#define     BITDEVICEUMCUPWRMON_CRITICALS (0x08)
                               /*    (Bit-3)  Critical battery (suspend)     */
#define     BITDEVICEUMCUPWRMON_CRITICALH (0x10)
                               /*    (Bit-4)  Critical battery (hibernation) */
#define     BITDEVICEUMCUPWRMON_CRITICALR (0x40)
                               /*    (Bit-6)  Critical battery (resume)      */
#define     BITDEVICEUMCUPWRMON_ACATTACH  (0x80)
                               /*    (Bit-7)  AC power attach                */
#define     BITDEVICEUMCUPWRMON_ACDETACH  (0x01)
                               /*    (Bit-8)  AC power detach                */
#define     BITDEVICEUMCUPWRMON_EVRESET   (0x80)
                               /*    (Bit-15) Event status reset             */

/* ---------------------------------------- */
/*  System command register (SCR) commands  */
/* ---------------------------------------- */
#define  CMDUMCUSCR_EOPMI                 (0x02)
                                /* End of PM interrupt                       */
#define  CMDUMCUSCR_FIFOCLEAR             (0x03)
                                /* Clear FIFO counter                        */
#define  CMDUMCUSCR_PMIGENERATE           (0x0E)
                                /* Generate PM interrupt                     */
                                /* (set bit-6 in BSR)                        */
#define  CMDUMCUSCR_ACTION                (0x20)
                                /* Action manager service request            */
#define     CMDUMCUACTION_SETONLINEEVENT  (0x10)
                                /*    Set event flags in on-line             */
#define     CMDUMCUACTION_SETOFFLINEEVENT (0x11)
                                /*    Set event flags in off-line            */
#define     CMDUMCUACTION_ENABLEEVENT     (0x12)
                                /*    Enable event notice                    */
#define     CMDUMCUACTION_SETOFFTIMER     (0x13)
                                /*    Set power-off watchdog timer           */
#define     CMDUMCUACTION_RESETEVENT      (0x14)
                                /*    Reset action                           */
#define  CMDUMCUSCR_STATE                 (0x40)
                                /* State service request                     */
#define     CMDUMCUSTATE_STARTDISCONN     (0x10)
                                /*    Start disconnection                    */
#define     CMDUMCUSTATE_COMPDISCONN      (0x11)
                                /*    Complete disconnection                 */
#define     CMDUMCUSTATE_STARTRECONN      (0x12)
                                /*    Start reconnection                     */
#define     CMDUMCUSTATE_COMPRECONN       (0x13)
                                /*    Complete reconnection                  */
#define     CMDUMCUSTATE_POWEROFF         (0x14)
                                /*    Main power-off                         */
#define  CMDUMCUSCR_BEEP                  DEVICEIDUMCU_BEEP
                                /* Beep service request                      */
#define     CMDUMCUBEEP_MAKEBEEP          (0x10)
                                /*    Make beep                              */
#define     CMDUMCUBEEP_STOPBEEP          (0x11)
                                /*    Stop beep                              */
#define  CMDUMCUSCR_BATTERY               DEVICEIDUMCU_BATTERY
                                /* Battery service request                   */
#define     CMDUMCUBATTERY_REFRESHENABLE  (0x10)
                                /*    Refresh enable                         */
#define     CMDUMCUBATTERY_REFRESHDISABLE (0x11)
                                /*    Refresh disable                        */
#define     CMDUMCUBATTERY_STATUS         (0x12)
                                /*    Battery status                         */
                                /*    (Bit-0) Attach/Detach    (1=attach)    */
                                /*    (Bit-1) Type             (1=NiMH)      */
                                /*    (Bit-2) Charge status    (1=charge)    */
                                /*    (Bit-3) Discharge status (1=discharge) */
#define     CMDUMCUBATTERY_TOTALCAPA      (0x13)
                                /*    Total capacity                         */
#define     CMDUMCUBATTERY_CURRENTCAPA    (0x14)
                                /*    Current capacity                       */
#define     CMDUMCUBATTERY_DISCHARGECAPA  (0x15)
                                /*    Discharging capacity                   */
#define     CMDUMCUBATTERY_DISCHARGETIME  (0x16)
                                /*    Discharging time                       */
#define  CMDUMCUSCR_IPD                   DEVICEIDUMCU_IPD
                                /* Internal pointing device service request  */
#define     CMDUMCUIPD_RESET              (0x10)
                                /*    Reset device                           */
#define  CMDUMCUSCR_LED_POWER             DEVICEIDUMCU_POWERLED
                                /* LED service request (power)               */
#define  CMDUMCUSCR_LED_SUSPEND           DEVICEIDUMCU_SUSPENDLED
                                /* LED service request (suspend)             */
#define     CMDUMCUSCRLED_TURNON          (0x10)
                                /*    Turn on                                */
#define     CMDUMCUSCRLED_TURNOFF         (0x11)
                                /*    Turn off                               */
#define     CMDUMCUSCRLED_BLINK           (0x12)
                                /*    Blink                                  */

/* --------------------------------- */
/*  System data register (SDR) data  */
/* --------------------------------- */
#define  SIZEUMCUSDR_FIFODATA             (4)
                                                           /* FIFO data size */

/* ------------------------------------- */
/*  PMC command register (PCR) commands  */
/* ------------------------------------- */
#define  CMDUMCUPCR_NULL                  (0x00)
                                               /* Null command (no received) */

/* ----------------------------------------------- */
/*  PM monitor register 0 (PMMONITOR0) bit assign  */
/* ----------------------------------------------- */
#define  BITUMCUPMMONITOR0_KBDDATA        (0x01)
                                   /* (Bit-0) Keyboard data                  */
#define  BITUMCUPMMONITOR0_MOUSEDATA      (0x02)
                                   /* (Bit-1) Mouse data                     */
#define  BITUMCUPMMONITOR0_KBDMOUSEACCESS (0x04)
                                   /* (Bit-2) Keyboard/Mouse register access */
#define  BITUMCUPMMONITOR0_FDCACCESS      (0x08)
                                   /* (Bit-3) FDC/FDD register access        */
#define  BITUMCUPMMONITOR0_PENACCESS      (0x10)
                                   /* (Bit-4) Pen interface access           */
#define  BITUMCUPMMONITOR0_RESERVED5      (0x20)
                                   /* (Bit-5) Reserved                       */
#define  BITUMCUPMMONITOR0_RESERVED6      (0x40)
                                   /* (Bit-6) Reserved                       */
#define  BITUMCUPMMONITOR0_AUDIOACCESS    (0x80)
                                   /* (Bit-7) Audio access                   */

/* ----------------------------------------------- */
/*  PM monitor register 1 (PMMONITOR1) bit assign  */
/* ----------------------------------------------- */
#define  BITUMCUPMMONITOR1_KBDID0         (0x01)
                                                 /* (Bit-0) Keyboard ID0     */
#define  BITUMCUPMMONITOR1_KBDID1         (0x02)
                                                 /* (Bit-1) Keyboard ID1     */
#define  BITUMCUPMMONITOR1_KBDID2         (0x04)
                                                 /* (Bit-2) Keyboard ID2     */
#define  BITUMCUPMMONITOR1_TVCONTROL      (0x08)
                                                 /* (Bit-3) Video/TV control */
#define  BITUMCUPMMONITOR1_FDDMOTOR       (0x10)
                                                 /* (Bit-4) FDD motor enable */
#define  BITUMCUPMMONITOR1_MOUSECLOCK     (0x20)
                                                 /* (Bit-5) Mouse clock      */
#define  BITUMCUPMMONITOR1_RESERVED6      (0x40)
                                                 /* (Bit-6) Reserved         */
#define  BITUMCUPMMONITOR1_RESERVED7      (0x80)
                                                 /* (Bit-7) Reserved         */

/* -------------------------------------------- */
/*  PM control register (PMCONTROL) bit assign  */
/* -------------------------------------------- */
#define  BITUMCUPMCONTROL_PMOUT0          (0x01)
                                   /* (Bit-0) PM output pin                  */
                                   /*         (external auxiliary device on) */
#define  BITUMCUPMCONTROL_RESERVED1       (0x02)
                                   /* (Bit-1) Reserved                       */
#define  BITUMCUPMCONTROL_RESERVED2       (0x04)
                                   /* (Bit-2) Reserved                       */
#define  BITUMCUPMCONTROL_RESERVED3       (0x08)
                                   /* (Bit-3) Reserved                       */
#define  BITUMCUPMCONTROL_RESERVED4       (0x10)
                                   /* (Bit-4) Reserved                       */
#define  BITUMCUPMCONTROL_RESERVED5       (0x20)
                                   /* (Bit-5) Reserved                       */
#define  BITUMCUPMCONTROL_RESERVED6       (0x40)
                                   /* (Bit-6) Reserved                       */
#define  BITUMCUPMCONTROL_RESERVED7       (0x80)
                                   /* (Bit-7) Reserved                       */
#define  BITUMCUPMCONTROL_EXTAUXON        BITUMCUPMCONTROL_PMOUT0

/* ------------------------------------------------ */
/*  Event status register (EVENTSTATUS) bit assign  */
/* ------------------------------------------------ */
#define  BITUMCUEVENTSTATUS_RESERVED0     (0x01)
                                                   /* (Bit-0) Reserved       */
#define  BITUMCUEVENTSTATUS_RESERVED1     (0x02)
                                                   /* (Bit-1) Reserved       */
#define  BITUMCUEVENTSTATUS_RESERVED2     (0x04)
                                                   /* (Bit-2) Reserved       */
#define  BITUMCUEVENTSTATUS_RESERVED3     (0x08)
                                                   /* (Bit-3) Reserved       */
#define  BITUMCUEVENTSTATUS_TURBO         (0x10)
                                                   /* (Bit-4) Turbo status   */
#define  BITUMCUEVENTSTATUS_PCICINT       (0x20)
                                                   /* (Bit-5) PCIC interrupt */
#define  BITUMCUEVENTSTATUS_RESERVED6     (0x40)
                                                   /* (Bit-6) Reserved       */
#define  BITUMCUEVENTSTATUS_RESERVED7     (0x80)
                                                   /* (Bit-7) Reserved       */

/* ---------------------------------------------------------- */
/*  Event interrupt enable register (EVENTENABLE) bit assign  */
/* ---------------------------------------------------------- */
#define  BITUMCUEVENTENABLE_MEMCNTENABLE  (0x01)
                                            /* (Bit-0) Memory counter enable */
#define  BITUMCUEVENTENABLE_MEMCNTSTART   (0x02)
                                            /* (Bit-1) Start memory count    */
#define  BITUMCUEVENTENABLE_LATCHCOUNTER  (0x04)
                                            /* (Bit-2) Latch counter data    */
#define  BITUMCUEVENTENABLE_RESERVED3     (0x08)
                                            /* (Bit-3) Reserved              */
#define  BITUMCUEVENTENABLE_RESERVED4     (0x10)
                                            /* (Bit-4) Reserved              */
#define  BITUMCUEVENTENABLE_PCICINTENABLE (0x20)
                                            /* (Bit-5) PCIC interrupt enable */
#define  BITUMCUEVENTENABLE_RESERVED6     (0x40)
                                            /* (Bit-6) Reserved              */
#define  BITUMCUEVENTENABLE_RESERVED7     (0x80)
                                            /* (Bit-7) Reserved              */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  WAITTIME_PMCBUSY                 (1000)  /* Busy time-out wait time */
                                                  /* (1000 msec = 1 sec)     */
#define  WAITTIME_DATAREADY               (100)      /* Data ready wait time */
                                                     /* (100 msec)           */
#define  TIMEOUT_POWEROFF                 (2)
                                           /* Power-off switch time-out time */
                                           /* (2 sec)                        */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    UMCU Controller Definition     * */
/* ************************************* */
/* ---------------- */
/*  UMCU I/O class  */
/* ---------------- */
VOID  UmcuIo(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */

/* -------------------- */
/*  UMCU control class  */
/* -------------------- */
VOID  UmcuCtl(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *           UMCU Objects            * */
/* ************************************* */
/* ---------------- */
/*  UMCU I/O class  */
/* ---------------- */
OBJUMCUIO   oUmcuIo  = {(POBJCTL)UmcuIo, PORTISA_BASE};

/* -------------------- */
/*  UMCU control class  */
/* -------------------- */
OBJUMCUCTL  oUmcuCtl = {(POBJCTL)UmcuCtl,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ***************************************** */
/* *          UMCU I/O Controller          * */
/* ***************************************** */
/* *                                       * */
/* * - Initialize                          * */
/* *     [ Input ]                         * */
/* *       Message    = MSGUMCU_INITIALIZE * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* * - Get index/data register             * */
/* *     [ Input ]                         * */
/* *       Message    = MSGUMCU_GET_INDEX  * */
/* *                    MSGUMCU_GET_DATA   * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *       Parameter1 = index/data value   * */
/* *                                       * */
/* * - Set index/data register             * */
/* *     [ Input ]                         * */
/* *       Message    = MSGUMCU_SET_INDEX  * */
/* *                    MSGUMCU_SET_DATA   * */
/* *       Parameter1 = index/data value   * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* * - Read register                       * */
/* *     [ Input ]                         * */
/* *       Message    = MSGUMCU_READ_xxx   * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *       Parameter1 = register value     * */
/* *                                       * */
/* * - Write register                      * */
/* *     [ Input ]                         * */
/* *       Message    = MSGUMCU_WRITE_xxx  * */
/* *       Parameter1 = register value     * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* * - Read & Modify register              * */
/* *     [ Input ]                         * */
/* *       Message    = MSGUMCU_MODIFY_xxx * */
/* *       Parameter1 = set bits value     * */
/* *       Parameter2 = mask bits value    * */
/* *     [ Output ]                        * */
/* *       Result     = result code        * */
/* *                                       * */
/* ***************************************** */
VOID  UmcuIo(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   BYTE        index, sindex;
   CRESULT     rc;
   POBJUMCUIO  pumcuio;
   DWORD       indexport, dataport, statusport;

   /* Get pointer to UMCU I/O object */
   /*                                */
   pumcuio = (POBJUMCUIO)poObj;

   /* Get index/data/status port address */
   /*                                    */
   indexport  = GetIsaBaseAddress() + PORTUMCU_INDEX;
   dataport   = GetIsaBaseAddress() + PORTUMCU_DATA;
   statusport = GetIsaBaseAddress() + PORTUMCU_BSR;

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGUMCU_INITIALIZE:
         /* Initialize */
         /*            */
         pumcuio->BaseAddress = GetIsaBaseAddress();
         rc = ERRUMCU_NOERROR;
         break;

      case MSGUMCU_GET_INDEX:
         /* Get index address register */
         /*                            */
         SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(indexport);
         rc = ERRUMCU_NOERROR;
         break;

      case MSGUMCU_SET_INDEX:
         /* Set index address register */
         /*                            */
         WriteControllerByte(indexport, (BYTE)SelectParm1(*pmMsg));
         rc = ERRUMCU_NOERROR;
         break;

      case MSGUMCU_GET_DATA:
         /* Get data register */
         /*                   */
         SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(dataport);
         rc = ERRUMCU_NOERROR;
         break;

      case MSGUMCU_SET_DATA:
         /* Set data register */
         /*                   */
         WriteControllerByte(dataport, (BYTE)SelectParm1(*pmMsg));
         rc = ERRUMCU_NOERROR;
         break;

      case MSGUMCU_READ_BSR:
         /* Read basic control register */
         /*                             */
         SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(statusport);
         rc = ERRUMCU_NOERROR;
         break;

      default:
         /* Get index register */
         /*                    */
         switch(SelectMessage(*pmMsg))
         {
            case MSGUMCU_READ_BCR:
            case MSGUMCU_WRITE_BCR:
            case MSGUMCU_MODIFY_BCR:
               index = INDEXUMCU_BCR;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_WRITE_SCR:
               index = INDEXUMCU_SCR;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_WRITE_SDR:
               index = INDEXUMCU_SDR;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_READ_PCR:
               index = INDEXUMCU_PCR;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_READ_PSR1:
               index = INDEXUMCU_PSR1;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_READ_PMMONITOR0:
            case MSGUMCU_WRITE_PMMONITOR0:
            case MSGUMCU_MODIFY_PMMONITOR0:
               index = INDEXUMCU_PMMONITOR0;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_READ_PMCONTROL:
            case MSGUMCU_WRITE_PMCONTROL:
               index = INDEXUMCU_PMCONTROL;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_READ_EVENTSTATUS:
            case MSGUMCU_WRITE_EVENTSTATUS:
               index = INDEXUMCU_EVENTSTATUS;
               rc    = ERRUMCU_NOERROR;
               break;
            case MSGUMCU_READ_EVENTENABLE:
            case MSGUMCU_WRITE_EVENTENABLE:
               index = INDEXUMCU_EVENTENABLE;
               rc    = ERRUMCU_NOERROR;
               break;
            default:
               rc = ERRUMCU_INVALID_MESSAGE;
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
               case MSGUMCU_READ_BCR:
               case MSGUMCU_READ_PCR:
               case MSGUMCU_READ_PSR1:
               case MSGUMCU_READ_PMMONITOR0:
               case MSGUMCU_READ_PMCONTROL:
               case MSGUMCU_READ_EVENTSTATUS:
               case MSGUMCU_READ_EVENTENABLE:
                  WriteControllerByte(indexport, index);
                  SelectParm1(*pmMsg) =
                                       (CPARAMETER)ReadControllerByte(dataport);
                  break;
               case MSGUMCU_WRITE_BCR:
               case MSGUMCU_WRITE_SCR:
               case MSGUMCU_WRITE_SDR:
               case MSGUMCU_WRITE_PMMONITOR0:
               case MSGUMCU_WRITE_PMCONTROL:
               case MSGUMCU_WRITE_EVENTSTATUS:
               case MSGUMCU_WRITE_EVENTENABLE:
                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,  (BYTE)SelectParm1(*pmMsg));
                  break;
               case MSGUMCU_MODIFY_BCR:
               case MSGUMCU_MODIFY_PMMONITOR0:
                  WriteControllerByte(indexport, index);
                  WriteControllerByte(dataport,
                                      (BYTE)((ReadControllerByte(dataport) &
                                              ~SelectParm2(*pmMsg)         ) |
                                             (SelectParm1(*pmMsg)          &
                                              SelectParm2(*pmMsg)          ) ));
                  break;
               default:
                  rc = ERRUMCU_INVALID_MESSAGE;
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

/* ************************************* */
/* * UMCU Busy Query Call-back Routine * */
/* ************************************* */
/* *                                   * */
/* *  [ Input  ] none                  * */
/* *  [ Output ] busy  = no zero       * */
/* *             ready = zero          * */
/* *                                   * */
/* ************************************* */
CRESULT  UmcuQueryBusy(VOID)
{
   MSGUMCU  msg;
   CRESULT  rc;

   BuildMsg(msg, MSGUMCU_QUERY_PMCREADY);
   SendMsg(msg, oUmcuCtl);

   if(!(rc = SelectResult(msg)))
   {
      rc = (SelectParm1(msg) == PRMUMCU_PMCREADY)
           ? (CRESULT)ERRUMCU_NOERROR
           : (CRESULT)ERRUMCU_TIMEOUT;
   } /* endif */

   return(rc);
}

/* *********************************************** */
/* * UMCU Data Not-Ready Query Call-back Routine * */
/* *********************************************** */
/* *                                             * */
/* *  [ Input  ] none                            * */
/* *  [ Output ] ready     = zero                * */
/* *             not ready = no zero             * */
/* *                                             * */
/* *********************************************** */
CRESULT  UmcuQueryNoData(VOID)
{
   MSGUMCU  msg;
   CRESULT  rc;

   BuildMsg(msg, MSGUMCU_READ_BSR);
   SendMsg(msg, oUmcuIo);

   if(!(rc = SelectResult(msg)))
   {
      rc = (SelectParm1(msg) & BITUMCUBSR_PMI)
           ? (CRESULT)ERRUMCU_NOERROR
           : (CRESULT)ERRUMCU_TIMEOUT;
   } /* endif */

   return(rc);
}

/* *************************************************** */
/* *             UMCU Control Controller             * */
/* *************************************************** */
/* *                                                 * */
/* * - Initialize                                    * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_INITIALIZECTL        * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Reset PMC interface                           * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_RESET_PMC            * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Get external PM interrupt status              * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_GET_PMISTATUS        * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = status                       * */
/* *                    (PRMUMCU_PMIENABLE )         * */
/* *                    (PRMUMCU_PMIDISABLE)         * */
/* *                                                 * */
/* * - Set external PM interrupt status              * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_SET_PMISTATUS        * */
/* *       Parameter1 = status                       * */
/* *                    (PRMUMCU_PMIENABLE )         * */
/* *                    (PRMUMCU_PMIDISABLE)         * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Query PMC ready condition                     * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_QUERY_PMCREADY       * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = condition                    * */
/* *                    (PRMUMCU_PMCREADY)           * */
/* *                    (PRMUMCU_PMCBUSY )           * */
/* *                                                 * */
/* * - Send parameter data                           * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_SEND_DATA            * */
/* *       Parameter1 = parameter 1                  * */
/* *       Parameter2 = parameter 2                  * */
/* *       Parameter3 = parameter 3                  * */
/* *       Parameter4 = parameter 4                  * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Receive parameter data                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_RECEIVE_DATA         * */
/* *       Parameter1 = receive buffer size          * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = received data size           * */
/* *       Parameter2 = received data 1              * */
/* *       Parameter3 = received data 2              * */
/* *       Parameter4 = received data 3              * */
/* *           :      =        :                     * */
/* *       ParameterN = received data (N-1)          * */
/* *                                                 * */
/* * - Send command                                  * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_SEND_COMMAND         * */
/* *       Parameter1 = command                      * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Receive command                               * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_RECEIVE_COMMAND      * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = command from PMC             * */
/* *                                                 * */
/* * - Send command and data                         * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_SEND_COMMANDDATA     * */
/* *       Parameter1 = command                      * */
/* *       Parameter2 = parameter 1                  * */
/* *       Parameter3 = parameter 2                  * */
/* *       Parameter4 = parameter 3                  * */
/* *       Parameter5 = parameter 4                  * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Query keyboard/mouse access status            * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_QUERY_KBDMOUSEACCESS * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *       Parameter1 = status                       * */
/* *                    (no zero = access   )        * */
/* *                    (zero    = no access)        * */
/* *                                                 * */
/* * - Clear keyboard/mouse access status            * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_CLEAR_KBDMOUSEACCESS * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Control commands                              * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_CMD_xxx              * */
/* *       Parameter1 = option 1                     * */
/* *       Parameter2 = option 2                     * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Save device context                           * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_SAVE_CONTEXT         * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* * - Restore device context                        * */
/* *     [ Input ]                                   * */
/* *       Message    = MSGUMCU_RESTORE_CONTEXT      * */
/* *     [ Output ]                                  * */
/* *       Result     = result code                  * */
/* *                                                 * */
/* *************************************************** */
VOID  UmcuCtl(PMSGCOMMON   pmMsg,        /* Pointer to Common message packet */
              POBJCOMMON   poObj)                /* Pointer to Common object */
{
   MSGUMCU     msg;
   CRESULT     rc;
   CPARAMETER  data[SIZEUMCUSDR_FIFODATA], sindex, index, len;
   PCPARAMETER pdata;
   POBJUMCUCTL pumcuctl;
   UINT        i;

   /* Get pointer to UMCU control object */
   /*                                    */
   pumcuctl = (POBJUMCUCTL)poObj;

   /* Control specific bits in register */
   /*   & Send/Receive commands         */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGUMCU_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGUMCU_INITIALIZE);
         SendMsg(msg, oUmcuIo);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGUMCU_SET_PMISTATUS,
                                   PRMUMCU_PMIENABLE);
            SendMsg(msg, oUmcuCtl);

            if(!(rc = SelectResult(msg)))
            {
               BuildMsg(msg, MSGUMCU_CMD_RESET_EVENT);
               SendMsg(msg, oUmcuCtl);

               BuildMsgWithParm1(msg, MSGUMCU_CMD_PWREVENT_TIMER,
                                      PRMUMCU_EVENTENABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_PWREVENT_POWEROFF,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_LIDEVENT_SUSPEND,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_BATEVENT_SUSPEND,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_ACEVENT_NOTIFY,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_CVREVENT_NOTIFY,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_KBDEVENT_NOTIFY,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_PWREVENT_RESUME,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm2(msg, MSGUMCU_CMD_LIDEVENT_RESUME,
                                      PRMUMCU_EVENTENABLE,
                                      PRMUMCU_EVENTNORMAL);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_KBDEVENT_RESUME,
                                      PRMUMCU_EVENTENABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_PSDEVENT_RESUME,
                                      PRMUMCU_EVENTENABLE);
               SendMsg(msg, oUmcuCtl);
               BuildMsgWithParm1(msg, MSGUMCU_CMD_BATEVENT_RESUME,
                                      PRMUMCU_EVENTDISABLE);
               SendMsg(msg, oUmcuCtl);

               BuildMsg(msg, MSGUMCU_CMD_START_EVENT);
               SendMsg(msg, oUmcuCtl);
               BuildMsg(msg, MSGUMCU_CMD_CLEAR_EVENT);
               SendMsg(msg, oUmcuCtl);

               rc = SelectResult(msg);
            } /* endif */
         } /* endif */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGUMCU_RESET_PMC:
         /* Reset PMC interface */
         /*                     */
         BuildMsgWithParm2(msg, MSGUMCU_MODIFY_BCR,
                                BITUMCUBCR_PMCRESET,
                                BITUMCUBCR_PMCRESET);
         SendMsg(msg, oUmcuIo);

         rc = SelectResult(msg);
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGUMCU_GET_PMISTATUS:
         /* Get external PM interrupt status */
         /*                                  */
         BuildMsg(msg, MSGUMCU_READ_BCR);
         SendMsg(msg, oUmcuIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITUMCUBCR_PMIENABLE)
                                  ? (CPARAMETER)PRMUMCU_PMIENABLE
                                  : (CPARAMETER)PRMUMCU_PMIDISABLE;
         } /* endif */
         break;

      case MSGUMCU_SET_PMISTATUS:
         /* Set external PM interrupt status */
         /*                                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMUMCU_PMIENABLE:
               BuildMsgWithParm2(msg, MSGUMCU_MODIFY_BCR,
                                      BITUMCUBCR_PMIENABLE,
                                      BITUMCUBCR_PMIENABLE);
               SendMsg(msg, oUmcuIo);

               rc = SelectResult(msg);
               break;
            case PRMUMCU_PMIDISABLE:
               BuildMsgWithParm2(msg, MSGUMCU_MODIFY_BCR,
                                      0,
                                      BITUMCUBCR_PMIENABLE);
               SendMsg(msg, oUmcuIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRUMCU_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGUMCU_QUERY_PMCREADY:
         /* Query PMC ready condition */
         /*                           */
         BuildMsg(msg, MSGUMCU_READ_BSR);
         SendMsg(msg, oUmcuIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & (BITUMCUBSR_PMCBUSY |
                                                       BITUMCUBSR_PMCINTR ))
                                  ? (CPARAMETER)PRMUMCU_PMCBUSY
                                  : (SelectParm1(msg) & BITUMCUBSR_PMCENABLE)
                                    ? (CPARAMETER)PRMUMCU_PMCREADY
                                    : (CPARAMETER)PRMUMCU_PMCBUSY;
         } /* endif */
         break;

      case MSGUMCU_SEND_DATA:
         /* Send parameter data */
         /*                     */
         if(!(rc = WaitTimeCheck(WAITTIME_PMCBUSY, UmcuQueryBusy)))
         {
            BuildMsgWithParm1(msg, MSGUMCU_WRITE_SCR,
                                   CMDUMCUSCR_FIFOCLEAR);
            SendMsg(msg, oUmcuIo);

            if(!(rc = SelectResult(msg)))
            {
               GetParm4(*pmMsg, *data, *(data + 1), *(data + 2), *(data + 3));

               for(i = 0; (i < SIZEUMCUSDR_FIFODATA) && !rc; i++)
               {
                  BuildMsgWithParm1(msg, MSGUMCU_WRITE_SDR,
                                         *(data + i));
                  SendMsg(msg, oUmcuIo);

                  rc = SelectResult(msg);
               } /* endfor */
            } /* endif */
         } /* endif */
         break;

      case MSGUMCU_RECEIVE_DATA:
         /* Receive parameter data */
         /*                        */
         BuildMsg(msg, MSGUMCU_READ_BSR);
         SendMsg(msg, oUmcuIo);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) & BITUMCUBSR_PMI)
            {
               BuildMsg(msg, MSGUMCU_GET_INDEX);
               SendMsg(msg, oUmcuIo);

               if(!(rc = SelectResult(msg)))
               {
                  /* Save index */
                  sindex = SelectParm1(msg);

                  /* Get data */
                  for(index = INDEXUMCU_PDR0, pdata = &(SelectParm2(*pmMsg)),
                      len = 0;
                      (index < INDEXUMCU_MAX) && (len < SelectParm1(*pmMsg)) &&
                      !rc;
                      index++, pdata++, len++)
                  {
                     BuildMsgWithParm1(msg, MSGUMCU_SET_INDEX,
                                            index);
                     SendMsg(msg, oUmcuIo);

                     if(!(rc = SelectResult(msg)))
                     {
                        BuildMsg(msg, MSGUMCU_GET_DATA);
                        SendMsg(msg, oUmcuIo);

                        if(!(rc = SelectResult(msg)))
                        {
                           *pdata = SelectParm1(msg);
                        } /* endif */
                     } /* endif */
                  } /* endfor */

                  SelectParm1(*pmMsg) = len;

                  /* Restore index */
                  BuildMsgWithParm1(msg, MSGUMCU_SET_INDEX,
                                         sindex);
                  SendMsg(msg, oUmcuIo);

                  rc = SelectResult(msg);
               } /* endif */
            }
            else
            {
               SelectParm1(*pmMsg) = 0;
            } /* endif */
         } /* endif */
         break;

      case MSGUMCU_SEND_COMMAND:
         /* Send command */
         /*              */
         if(!(rc = WaitTimeCheck(WAITTIME_PMCBUSY, UmcuQueryBusy)))
         {
            BuildMsgWithParm1(msg, MSGUMCU_WRITE_SCR,
                                   SelectParm1(*pmMsg));
            SendMsg(msg, oUmcuIo);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGUMCU_RECEIVE_COMMAND:
         /* Receive command */
         /*                 */
         BuildMsg(msg, MSGUMCU_READ_BSR);
         SendMsg(msg, oUmcuIo);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) & BITUMCUBSR_PMI)
            {
               BuildMsg(msg, MSGUMCU_READ_PCR);
               SendMsg(msg, oUmcuIo);

               if(!(rc = SelectResult(msg)))
               {
                  SelectParm1(*pmMsg) = SelectParm1(msg);
               } /* endif */
            }
            else
            {
               SelectParm1(*pmMsg) = PRMUMCU_COMMANDNULL;
            } /* endif */
         } /* endif */
         break;

      case MSGUMCU_SEND_COMMANDDATA:
         /* Send command and data */
         /*                       */
         BuildMsgWithParm4(msg, MSGUMCU_SEND_DATA,
                                SelectParm2(*pmMsg),
                                SelectParm3(*pmMsg),
                                SelectParm4(*pmMsg),
                                SelectParm5(*pmMsg));
         SendMsg(msg, oUmcuCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGUMCU_SEND_COMMAND,
                                   SelectParm1(*pmMsg));
            SendMsg(msg, oUmcuCtl);

            rc = SelectResult(msg);
         } /* endif */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGUMCU_QUERY_KBDMOUSEACCESS:
         /* Query keyboard/mouse access status */
         /*                                    */
         BuildMsg(msg, MSGUMCU_READ_PMMONITOR0);
         SendMsg(msg, oUmcuIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (CPARAMETER)(SelectParm1(msg) &
                                   (BITUMCUPMMONITOR0_KBDDATA       |
                                    BITUMCUPMMONITOR0_MOUSEDATA     |
                                    BITUMCUPMMONITOR0_KBDMOUSEACCESS));
         } /* endif */
         break;

      case MSGUMCU_CLEAR_KBDMOUSEACCESS:
         /* Clear keyboard/mouse access status */
         /*                                    */
         BuildMsgWithParm2(msg, MSGUMCU_MODIFY_PMMONITOR0,
                                0,
                                BITUMCUPMMONITOR0_KBDDATA   |
                                BITUMCUPMMONITOR0_MOUSEDATA |
                                BITUMCUPMMONITOR0_KBDMOUSEACCESS);
         SendMsg(msg, oUmcuIo);

         rc = SelectResult(msg);
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGUMCU_CMD_EOPMI:
         /* End of PM interrupt */
         /*                     */
         BuildMsgWithParm1(msg, MSGUMCU_SEND_COMMAND,
                                CMDUMCUSCR_EOPMI);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGUMCU_CMD_POWEROFF:
         /* Request main power-off */
         /*                        */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_STATE,
                                CMDUMCUSTATE_POWEROFF,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_SUSPEND:
         /* Goto suspend */
         /*              */
         rc = ERRUMCU_NOERROR;

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_STATE,
                                CMDUMCUSTATE_STARTDISCONN,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_STATE,
                                CMDUMCUSTATE_COMPDISCONN,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGUMCU_CMD_RESUME:
         /* Resume completion notice */
         /*                          */
         rc = ERRUMCU_NOERROR;

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_STATE,
                                CMDUMCUSTATE_STARTRECONN,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         WaitTimeCheck(WAITTIME_PMCBUSY, UmcuQueryBusy);
         WaitTimeCheck(WAITTIME_PMCBUSY, UmcuQueryNoData);

         BuildMsg(msg, MSGUMCU_RECEIVE_COMMAND);
         SendMsg(msg, oUmcuCtl);

         if((SelectParm1(msg) != PRMUMCU_COMMANDNULL) &&
            (SelectParm1(msg) != CMDUMCUSCR_ACTION  )  )
         {
            pumcuctl->chResumeCommand = (BYTE)SelectParm1(msg);

            BuildMsgWithParm1(msg, MSGUMCU_RECEIVE_DATA,
                                   4);
            SendMsg(msg, oUmcuCtl);

            pumcuctl->chResumeData0 = (BYTE)SelectParm2(msg);
            pumcuctl->chResumeData1 = (BYTE)SelectParm3(msg);
            pumcuctl->chResumeData2 = (BYTE)SelectParm4(msg);
            pumcuctl->chResumeData3 = (BYTE)SelectParm5(msg);

            BuildMsg(msg, MSGUMCU_CMD_CLEAR_EVENT);
            SendMsg(msg, oUmcuCtl);
         }
         else
         {
            pumcuctl->chResumeCommand = 0;
            pumcuctl->chResumeData0   = 0;
            pumcuctl->chResumeData1   = 0;
            pumcuctl->chResumeData2   = 0;
            pumcuctl->chResumeData3   = 0;
         } /* endif */

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_STATE,
                                CMDUMCUSTATE_COMPRECONN,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_CMD_EOPMI);
         SendMsg(msg, oUmcuCtl);
         break;

      case MSGUMCU_CMD_LED_SUSPEND:
         /* Suspend LED request */
         /*                     */
         rc = ERRUMCU_NOERROR;

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_LED_SUSPEND,
                                CMDUMCUSCRLED_TURNON,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_LED_POWER,
                                CMDUMCUSCRLED_TURNOFF,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGUMCU_CMD_LED_RESUME:
         /* Resume LED request */
         /*                    */
         rc = ERRUMCU_NOERROR;

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_LED_POWER,
                                CMDUMCUSCRLED_TURNON,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_LED_SUSPEND,
                                CMDUMCUSCRLED_TURNOFF,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGUMCU_CMD_LED_TRANSIT:
         /* Transition LED request (suspend <-> resume) */
         /*                                             */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_LED_SUSPEND,
                                CMDUMCUSCRLED_BLINK,
                                0x81,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_POWEROFF:
         /* Power-off beep request */
         /*                        */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x07,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_SUSPEND:
         /* Suspend beep request */
         /*                      */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x03,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_RESUME:
         /* Resume beep request */
         /*                     */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x05,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_HIBERNATE:
         /* Hibernation beep request */
         /*                          */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x0B,
                                0x0A,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_ACINOUT:
         /* AC attach/detach beep request */
         /*                               */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x06,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_WARNING:
         /* Warning beep request */
         /*                      */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x08,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_WARNINGREPEAT:
         /* Warning beep request (repeatedly) */
         /*                                   */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_MAKEBEEP,
                                0x08,
                                0x08,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BEEP_STOP:
         /* Beep stop request */
         /*                   */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BEEP,
                                CMDUMCUBEEP_STOPBEEP,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGUMCU_CMD_RESET_EVENT:
         /* Reset all events */
         /*                  */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_RESETEVENT,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_START_EVENT:
         /* Start all events notification */
         /*                               */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_ENABLEEVENT,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_CLEAR_EVENT:
         /* Clear all events */
         /*                  */
         do
         {
            BuildMsg(msg, MSGUMCU_CMD_EOPMI);
            SendMsg(msg, oUmcuCtl);

            WaitTimeCheck(WAITTIME_PMCBUSY,   UmcuQueryBusy);
            WaitTimeCheck(WAITTIME_DATAREADY, UmcuQueryNoData);

            BuildMsg(msg, MSGUMCU_RECEIVE_COMMAND);
            SendMsg(msg, oUmcuCtl);
            rc = SelectResult(msg);
         } while(!rc && (SelectParm1(msg) != PRMUMCU_COMMANDNULL)); /* enddo */
         break;

      case MSGUMCU_CMD_PWREVENT_POWEROFF:
         /* Power-off event request (power switch) */
         /*                                        */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETONLINEEVENT,
                                DEVICEIDUMCU_MAINPOWER,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCUMAINPOWER_PUSH
                                : 0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_LIDEVENT_SUSPEND:
         /* Suspend event request (LID) */
         /*                             */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETONLINEEVENT,
                                DEVICEIDUMCU_LID,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (BITDEVICEUMCULID_CLOSE |
                                   BITDEVICEUMCULID_OPEN  )
                                : 0x00,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCULID_EVRESET
                                : 0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BATEVENT_SUSPEND:
         /* Suspend event request (critical battery) */
         /*                                          */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETONLINEEVENT,
                                DEVICEIDUMCU_POWERMONITOR,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (pumcuctl->chOnLineParm1PowerMonitor |=
                                   BITDEVICEUMCUPWRMON_CRITICALS        )
                                : (pumcuctl->chOnLineParm1PowerMonitor &=
                                   ~BITDEVICEUMCUPWRMON_CRITICALS       ),
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (pumcuctl->chOnLineParm2PowerMonitor |
                                   BITDEVICEUMCUPWRMON_EVRESET         )
                                :  pumcuctl->chOnLineParm2PowerMonitor);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_PWREVENT_RESUME:
         /* Resume event request (power switch) */
         /*                                     */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETOFFLINEEVENT,
                                DEVICEIDUMCU_MAINPOWER,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCUMAINPOWER_PUSH
                                : 0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_LIDEVENT_RESUME:
         /* Resume event request (LID) */
         /*                            */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETOFFLINEEVENT,
                                DEVICEIDUMCU_LID,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCULID_OPEN
                                : 0x00,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (SelectParm2(*pmMsg) == PRMUMCU_EVENTRESET)
                                  ? BITDEVICEUMCULID_EVRESET
                                  : 0x00
                                : 0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_KBDEVENT_RESUME:
         /* Resume event request (keyboard) */
         /*                                 */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETOFFLINEEVENT,
                                DEVICEIDUMCU_KEYBOARD,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCUKBD_ANYKEYRESUME
                                : 0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_PSDEVENT_RESUME:
         /* Resume event request (pointing-stick) */
         /*                                       */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETOFFLINEEVENT,
                                DEVICEIDUMCU_IPD,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (BITDEVICEUMCUIPD_CLICKRELEASE |
                                   BITDEVICEUMCUIPD_CLICKDEPRESS )
                                : 0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_BATEVENT_RESUME:
         /* Resume event request (critical battery) */
         /*                                         */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETOFFLINEEVENT,
                                DEVICEIDUMCU_POWERMONITOR,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCUPWRMON_CRITICALR
                                : 0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_CVREVENT_NOTIFY:
         /* Notify event request (keyboard-cover open/close) */
         /*                                                  */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETONLINEEVENT,
                                DEVICEIDUMCU_KEYBOARDCOVER,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (BITDEVICEUMCUKBDCOVER_CLOSE |
                                   BITDEVICEUMCUKBDCOVER_OPEN  )
                                : 0x00,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCUKBDCOVER_EVRESET
                                : 0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_KBDEVENT_NOTIFY:
         /* Notify event request (keyboard plug-in/out) */
         /*                                             */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETONLINEEVENT,
                                DEVICEIDUMCU_KEYBOARD,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (BITDEVICEUMCUKBD_PLUGIN |
                                   BITDEVICEUMCUKBD_PLUGOUT)
                                : 0x00,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? BITDEVICEUMCUKBD_EVRESET
                                : 0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_ACEVENT_NOTIFY:
         /* Notify event request (AC power change) */
         /*                                        */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETONLINEEVENT,
                                DEVICEIDUMCU_POWERMONITOR,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (pumcuctl->chOnLineParm1PowerMonitor |=
                                   BITDEVICEUMCUPWRMON_ACATTACH         )
                                : (pumcuctl->chOnLineParm1PowerMonitor &=
                                   ~BITDEVICEUMCUPWRMON_ACATTACH        ),
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (pumcuctl->chOnLineParm2PowerMonitor |=
                                   BITDEVICEUMCUPWRMON_ACDETACH         ) |
                                  BITDEVICEUMCUPWRMON_EVRESET
                                : (pumcuctl->chOnLineParm2PowerMonitor &=
                                   ~BITDEVICEUMCUPWRMON_ACDETACH));
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_PWREVENT_TIMER:
         /* Power-off switch event timer */
         /*                              */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_ACTION,
                                CMDUMCUACTION_SETOFFTIMER,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (TIMEOUT_POWEROFF & 0x00FF)
                                : 0x00,
                                (SelectParm1(*pmMsg) == PRMUMCU_EVENTENABLE)
                                ? (TIMEOUT_POWEROFF >> 8)
                                : 0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_PSDRESET:
         /* Pointing-stick device reset */
         /*                             */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_IPD,
                                CMDUMCUIPD_RESET,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGUMCU_CMD_QUERY_BATSTATUS:
         /* Query battery status */
         /*                      */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BATTERY,
                                CMDUMCUBATTERY_STATUS,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_QUERY_BATTOTALCAPA:
         /* Query total capacity */
         /*                      */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BATTERY,
                                CMDUMCUBATTERY_TOTALCAPA,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_QUERY_BATCURRENTCAPA:
         /* Query current capacity */
         /*                        */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BATTERY,
                                CMDUMCUBATTERY_CURRENTCAPA,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_QUERY_BATDISCCAPA:
         /* Query discharging capacity */
         /*                            */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BATTERY,
                                CMDUMCUBATTERY_DISCHARGECAPA,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_QUERY_BATDISCTIME:
         /* Query discharging time */
         /*                        */
         BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                CMDUMCUSCR_BATTERY,
                                CMDUMCUBATTERY_DISCHARGETIME,
                                0x00,
                                0x00,
                                0x00);
         SendMsg(msg, oUmcuCtl);

         rc = SelectResult(msg);
         break;

      case MSGUMCU_CMD_QUERY_RESUMEEVENT:
         /* Query resume event */
         /*                    */
         SelectParm1(*pmMsg) = (CPARAMETER)(pumcuctl->chResumeCommand);
         SelectParm2(*pmMsg) = (CPARAMETER)(pumcuctl->chResumeData0);
         SelectParm3(*pmMsg) = (CPARAMETER)(pumcuctl->chResumeData1);
         SelectParm4(*pmMsg) = (CPARAMETER)(pumcuctl->chResumeData2);
         SelectParm5(*pmMsg) = (CPARAMETER)(pumcuctl->chResumeData3);

         rc = ERRUMCU_NOERROR;
         break;

      case MSGUMCU_CMD_CONTROL_BATTERY:
         /* Control battery */
         /*                 */
         switch(SelectParm1(*pmMsg))
         {
            case PRMUMCU_BATDISCHARGESTART:
               BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                      CMDUMCUSCR_BATTERY,
                                      CMDUMCUBATTERY_REFRESHENABLE,
                                      0x00,
                                      0x00,
                                      0x00);
               SendMsg(msg, oUmcuCtl);

               rc = SelectResult(msg);
               break;
            case PRMUMCU_BATDISCHARGESTOP:
               BuildMsgWithParm5(msg, MSGUMCU_SEND_COMMANDDATA,
                                      CMDUMCUSCR_BATTERY,
                                      CMDUMCUBATTERY_REFRESHDISABLE,
                                      0x00,
                                      0x00,
                                      0x00);
               SendMsg(msg, oUmcuCtl);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRUMCU_INVALID_PARAMETER;
         } /* endswitch */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGUMCU_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRUMCU_NOERROR;

         BuildMsg(msg, MSGUMCU_GET_INDEX);
         SendMsg(msg, oUmcuIo);
         if(!SelectResult(msg))
         {
            pumcuctl->chIndex = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_READ_BCR);
         SendMsg(msg, oUmcuIo);
         if(!SelectResult(msg))
         {
            pumcuctl->chBcr = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_READ_PMMONITOR0);
         SendMsg(msg, oUmcuIo);
         if(!SelectResult(msg))
         {
            pumcuctl->chPmMonitor0 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_READ_PMCONTROL);
         SendMsg(msg, oUmcuIo);
         if(!SelectResult(msg))
         {
            pumcuctl->chPmControl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_READ_EVENTSTATUS);
         SendMsg(msg, oUmcuIo);
         if(!SelectResult(msg))
         {
            pumcuctl->chEventStatus = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_READ_EVENTENABLE);
         SendMsg(msg, oUmcuIo);
         if(!SelectResult(msg))
         {
            pumcuctl->chEventEnable = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGUMCU_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRUMCU_NOERROR;

         BuildMsgWithParm1(msg, MSGUMCU_WRITE_BCR,
                                pumcuctl->chBcr);
         SendMsg(msg, oUmcuIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGUMCU_WRITE_PMMONITOR0,
                                pumcuctl->chPmMonitor0);
         SendMsg(msg, oUmcuIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGUMCU_WRITE_PMCONTROL,
                                pumcuctl->chPmControl);
         SendMsg(msg, oUmcuIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGUMCU_WRITE_EVENTSTATUS,
                                pumcuctl->chEventStatus);
         SendMsg(msg, oUmcuIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGUMCU_WRITE_EVENTENABLE,
                                pumcuctl->chEventEnable);
         SendMsg(msg, oUmcuIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGUMCU_SET_INDEX,
                                pumcuctl->chIndex);
         SendMsg(msg, oUmcuIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGUMCU_CMD_EOPMI);
         SendMsg(msg, oUmcuCtl);
         break;

      default:
         SendMsg(*pmMsg, oUmcuIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
