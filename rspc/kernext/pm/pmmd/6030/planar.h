/* @(#)62       1.3  src/rspc/kernext/pm/pmmd/6030/planar.h, pwrsysx, rspc41J, 9511A_all 3/15/95 04:13:06 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management Kernel Extension Code
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 */
#ifndef _H_PLANAR
#define _H_PLANAR


/* --------------------------------------- */
/*  etc                                    */
/* --------------------------------------- */
#define SET_BITS        0xFF
#define RESET_BITS      0x00

struct _pm_pmc_data {
        Simple_lock     lock;
        int             base;
        int             bus_id;
        struct io_map   map;
};

struct _pm_isadev_data {
        Simple_lock     lock;
        int             bus_id;
        struct io_map   map;
};

/* --------------------------------------- */
/* --------------------------------------- */
/* -  Power Management Controller (PMC)  - */
/* --------------------------------------- */
/* --------------------------------------- */
/* --------------------------------------- */
/*  Index data to index register           */
/* --------------------------------------- */
#define  INDEXPMC_PM_CNTRL   (0x00)   /* Power management control register   */
#define  INDEXPMC_PM_REQ_STS (0x01)   /* Power management request status reg */
#define  INDEXPMC_CPUCLK_CNTRL (0x02) /* CPU clock control register          */
#define  INDEXPMC_BL_STS       (0x03) /* Backlight status register           */
#define  INDEXPMC_BL_CTRL      (0x04) /* Backlight control register          */
#define  INDEXPMC_BL_TMR       (0x05) /* Backlight idle timer register       */
#define  INDEXPMC_SUSPEND_TMRL (0x06) /* Suspend idle timer low register     */
#define  INDEXPMC_SUSPEND_TMRH (0x07) /* Suspend idle timer high register    */
#define  INDEXPMC_STI_ACT_MSK  (0x08) /* Suspend timer idle activity mask reg*/
#define  INDEXPMC_STI_ACT_STS  (0x09) /*Suspend timer idle activity status reg*/
#define  INDEXPMC_RESUME_MASK  (0x0A) /* Resume mask register                */
#define  INDEXPMC_RESUME_STS   (0x0B) /* Resume status register              */
#define  INDEXPMC_PMCO0        (0x0C) /* Power management control output 0 reg*/
#define  INDEXPMC_PMCO1        (0x0D) /* Power management control output 1 reg*/
#define  INDEXPMC_PMIN_CTRL    (0x0E) /* PMIN control register                */
#define  INDEXPMC_CC_PMI_CLR   (0x0F)   /* CC_PMI clear register              */
#define  INDEXPMC_PMI_CLR      (0x10)   /* PMI clear register                 */
#define  INDEXPMC_MISC_CTRL    (0x11)   /* Misc. control register             */
#define  INDEXPMC_SPAS         (0x12)   /* Serial port address status register*/
#define  INDEXPMC_SRPT         (0x13)   /* Suspend/Resume programmable timings*/
#define  INDEXPMC_CCCPTR       (0x14)   /* CPU clk change programmable timings*/
/* --------------------------------------------------------- */
/*  Power management control register (PM_CNTRL) bit assign  */
/* --------------------------------------------------------- */
#define  BITPMCPMCNTRL_PMI_CTRL     (0x01)   /* (Bit-0) PMI control         */
#define  BITPMCPMCNTRL_PMI_CC_PMI   (0x02)   /* (Bit-1) Clock change PMI    */
#define  BITPMCPMCNTRL_SUSP_TMR_EN  (0x04)   /* (Bit-2) Suspend timer enable*/
#define  BITPMCPMCNTRL_SUSP_CPU_PWR (0x08)   /* (Bit-3) CPU power in suspend*/
#define  BITPMCPMCNTRL_CC_EN        (0x10)   /* (Bit-4) Clock change enable */
#define  BITPMCPMCNTRL_BATTLOW_RSM  (0x20)   /* (Bit-5) Battery in critical */
#define  BITPMCPMCNTRL_SUSSTAT      (0x40)   /* (Bit-6) Suspend status      */
#define  BITPMCPMCNTRL_PM_EN        (0x80)   /* (Bit-7) PM enable           */

/* ------------------------------------------------------------------ */
/*  Power management request status register (PM_REQ_STS) bit assign  */
/* ------------------------------------------------------------------ */
#define  BITPMCPMREQSTS_CLK_CHG     (0x01)   /* (Bit-0) Clock changed       */
#define  BITPMCPMREQSTS_IO_ACCESS   (0x02)   /* (Bit-1) I/O access detected */
#define  BITPMCPMREQSTS_GP_TMR_EXP  (0x04)   /* (Bit-2) GP timer expired    */
#define  BITPMCPMREQSTS_EXT_PMI     (0x08)   /* (Bit-3) Ext PMI suspend req */
#define  BITPMCPMREQSTS_SUSP_TMR_EXP (0x10)  /* (Bit-4) Suspend timer expire*/
#define  BITPMCPMREQSTS_BL_REQ      (0x20)   /* (Bit-5) Backlight request   */
#define  BITPMCPMREQSTS_PMIN        (0x40)   /* (Bit-6) PMIN request        */

/* ----------------------------------------------- */
/*  Resume mask register (RESUME_MASK) bit assign  */
/* ----------------------------------------------- */
#define  BITPMCRESUMEMASK_RING_MSK  (0x01)   /* (Bit-0) Ring indicator mask */
#define  BITPMCRESUMEMASK_ALARM_MSK (0x02)   /* (Bit-1) Alarm mask          */
#define  BITPMCRESUMEMASK_H8_RSM_MSK (0x04)   /* (Bit-2) H8 resume mask      */

/* ------------------------------------------------ */
/*  Resume status register (RESUME_STS) bit assign  */
/* ------------------------------------------------ */
#define  BITPMCRESUMESTS_RING_STS   (0x01) /* (Bit-0) RI event status indicat*/
#define  BITPMCRESUMESTS_ALARM_STS  (0x02) /* (Bit-1) Alarm indicator        */
#define  BITPMCRESUMESTS_H8_RSM_STS (0x04) /* (Bit-2) H8 resume status       */

/* --------------------------------------------------------------- */
/*  Power management control output 0 register (PMCO0) bit assign  */
/* --------------------------------------------------------------- */
#define  BITPMCPMCO0_LCDBACKLIGHTOFF (0x01)   /* (Bit-0) LCD backlight off   */
                                              /*         (1=on, 0=off)       */
#define  BITPMCPMCO0_LCDOFF          (0x02)   /* (Bit-1) LCD power off       */
                                              /*         (1=on, 0=off) 	     */
#define  BITPMCPMCO0_LCDENABLE       (0x04)   /* (Bit-2) LCD enable          */
#define  BITPMCPMCO0_LCDHALFBRIGHTNESS (0x08) /* (Bit-3) LCD half brightness */
                                              /*         (1=full, 0=half)    */
#define  BITPMCPMCO0_VIDEOLOWPOWER   (0x10)   /* (Bit-4) Video chip low power*/
                                              /*         (1=on, 0=off)       */
#define  BITPMCPMCO0_CAMERAOFF       (0x20)   /* (Bit-5) Camera power off    */
                                              /*         (1=on, 0=off)       */
#define  BITPMCPMCO0_VIDEODIGITIZEROFF (0x40) /* (Bit-6) Video digitizer off */
                                              /*         (1=on, 0=off)       */
#define  BITPMCPMCO0_RESERVED7       (0x80)   /* (Bit-7) Reserved            */

/* --------------------------------------------------------------- */
/*  Power management control output 1 register (PMCO1) bit assign  */
/* --------------------------------------------------------------- */
#define  BITPMCPMCO1_SCSICLOCKSTOP   (0x01)  /* (Bit-0) SCSI clock stop     */
                                             /*         (1=on, 0=off)       */
#define  BITPMCPMCO1_SCSITERMLOWPOWER (0x06) /* (Bit-1,2) SCSI terminator low */
#define     BITPMCPMCO1_SCSITERMON110 (0x00) /*       Terminator on (110 ohm) */
#define     BITPMCPMCO1_SCSITERMON25K (0x02) /*       Terminator on (2.5 Kohm)*/
#define     BITPMCPMCO1_SCSITERM64    (0x04) /*       Terminator ???          */
#define     BITPMCPMCO1_SCSITERMDISC  (0x06)   /*     Terminator disconnect   */
#define  BITPMCPMCO1_CDROMDRIVELOWPOWER (0x08) /* (Bit-3) CD-ROM drive low pw */
#define  BITPMCPMCO1_AUDIOLOWPOWER    (0x10)   /* (Bit-4) Audio low power     */
                                                  /*         (1=on, 0=off)    */
#define  BITPMCPMCO1_CRTLOWPOWER      (0x60)   /* (Bit-5,6) CRT low power     */
                                                  /*   (Bit-5) Hsync          */
                                                  /*   (Bit-6) Vsync          */
#define     BITPMCPMCO1_CRTOFF        (0x00)   /*           CRT off           */
#define     BITPMCPMCO1_CRTSUSPEND    (0x20)   /*           CRT suspend       */
#define     BITPMCPMCO1_CRTSTANDBY    (0x40)   /*           CRT stand-by      */
#define     BITPMCPMCO1_CRTON         (0x60)   /*           CRT on            */
#define  BITPMCPMCO1_L2LOWPOWER       (0x80)   /* (Bit-7) L2 low power        */
                                               /*    (1=normal, 0=lowpower)   */

/* ---------------------------------------------- */
/*  PMIN control register (PMIN_CTRL) bit assign  */
/* ---------------------------------------------- */
#define  BITPMCPMINCTRL_HDDACTIVE     (0x01) /* (Bit-0) Intnl HDD activity */
#define  BITPMCPMINCTRL_CDROMACTIVE   (0x02) /* (Bit-1) Intnl CD-ROM activity */

/* ----------------------------------------------- */
/*  Misc. control register (MISC_CTRL) bit assign  */
/* ----------------------------------------------- */
#define  BITPMCMISCCTRL_SET_DSKCHG    (0x01)  /* (Bit-0) Set DSKCHG#     */ 
#define  BITPMCMISCCTRL_BLMASK228     (0x02)  /* (Bit-1) Backlt mask adr228h */
#define  BITPMCMISCCTRL_SUSMASK228    (0x04)  /* (Bit-2) Suspend mask adr228h*/
#define  BITPMCMISCCTRL_EN_GATECLK    (0x08)  /* (Bit-3) Clock gate off en  */ 
#define  BITPMCMISCCTRL_CC_PMI_PMI    (0x10)   /* (Bit-4) CC_PMI to PMI      */

/* --------------------------------------- */
/* --------------------------------------- */ 
/* -    ISA  Device Control              - */
/* --------------------------------------- */
/* --------------------------------------- */
/* ------------------------------ */
/*   XIOC device  port address    */
/* ------------------------------ */
#define  PORTXIOC_STORAGELIGHT        (0x0808)  /* Storage light register   */
#define  PORTXIOC_EQUIPMENTPRESENT    (0x080C)  /* Equipment present register*/
#define  PORTXIOC_PASSWORDPROTECT1    (0x0810)  /* Password protect 1 register*/
#define  PORTXIOC_PASSWORDPROTECT2    (0x0812)  /* Password protect 2 register*/
#define  PORTXIOC_L2FLUSH             (0x0814)  /* L2 flush register          */
#define  PORTXIOC_SYSTEMCONTROL       (0x081C)  /* System control register    */
#define  PORTXIOC_EOIA                (0x0838)  /* End of INT-A register      */
#define  PORTXIOC_PCIINTAMAP          (0x0839)  /* PCI INT-A mapping register */
#define  PORTXIOC_EOIB                (0x083C)  /* End of INT-B register      */
#define  PORTXIOC_PCIINTBMAP          (0x083D)  /* PCI INT-B mapping register */
#define  PORTXIOC_AUDIOSUPPORT        (0x083E)  /* Audio support register     */
#define  PORTXIOC_SIMM1MEMORYID       (0x0880)  /* SIMM 1 base memory ID reg  */
#define  PORTXIOC_SIMM2MEMORYID       (0x0884)  /* SIMM 2 base memory ID reg  */
#define  PORTXIOC_SIMM3MEMORYID       (0x0888)  /* SIMM 3 memory ID register  */
#define  PORTXIOC_SIMM4MEMORYID       (0x088C)  /* SIMM 4 memory ID register  */
#define  PORTXIOC_CMOSSECURITY        (0x08A0)  /* CMOS security register     */
#define  PORTXIOC_PLANARSETUPLOCK     (0x08A1) /* Planar setup lock/unlock reg*/
#define  PORTXIOC_MONITORID           (0x08A4)  /* Monitor ID register        */
#define  PORTXIOC_SCSISECURITY        (0x08A8)  /* SCSI security register     */
#define  PORTXIOC_STORAGEPRESENCE     (0x08A9)  /* Storage device presence reg*/
#define  PORTXIOC_ADAPTERPRESENCE     (0x08AA)  /* Adapter presence detect reg*/
#define  PORTXIOC_SCSIIDSETUP         (0x08AB)  /* SCSI ID setup register     */
#define  PORTXIOC_PCICINTMAPPING      (0x08AC)  /* PCIC interrupt mapping reg */
#define  PORTXIOC_PCICINTSTATUS       (0x08AD)  /* PCIC interrupt status reg  */


/* ---------------------------------------------------------------- */
/*  System control register  (PORTXIOC_SYSTEMCONTROL) bit assign    */
/* ---------------------------------------------------------------- */
#define  BITXIOCSYSTEMCONTROL_L2ENABLE   (0x40) /* (Bit-6) L2 cache Enable    */
                                                /*  (1=enable, 0=disable)     */
#define  BITXIOCSYSTEMCONTROL_L2MISSUPDATE (0x80) /* (Bit-7) L2 cache Updated */
                                              /*(1=all update,0=allows bypass)*/

/* ---------------------------------------------------------------- */
/*  Audio support register   (PORTXIOC_AUDIOSUPPORT) bit assign     */
/* ---------------------------------------------------------------- */
#define  BITXIOCAUDIOSUPPORT_SPEAKERMUTE (0x02) /* (Bit-1) Speaker Mute      */
                                                /*         (1=on, 0=off)     */

#endif /* _H_PLANAR */
