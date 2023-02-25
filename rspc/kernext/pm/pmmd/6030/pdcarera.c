static char sccsid[] = "@(#)41  1.2  src/rspc/kernext/pm/pmmd/6030/pdcarera.c, pwrsysx, rspc41J, 9517A_b 4/25/95 04:13:08";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Carrera (Power Management Controller) Chip Device Control
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
/* * Module Name: PDCARERA.C                                               * */
/* * Description: Carrera (Power Management Controller) Chip Device Control* */
/* *              Routines                                                 * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDCARERA.H/PDPCIBUS.H/PDRTC.H files* */
/* *              should be included in this file                          * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdcarera.h"
#include "pdpcibus.h"
#include "pdrtc.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      Carrera Data Definition      * */
/* ************************************* */
/* --------------------------------------------------- */
/*  Power management register index/data port address  */
/* --------------------------------------------------- */
#define  PORTCARR_BASE                    (0x00004100L)
#define  PORTCARR_BASENULL                (0xFFFFFFFFL)
                                                     /* Carrera base address */
                                                      /* & base null address */
#define  PORTCARR_BASEMASK                (0xFFFFFFFCL) /* Base address mask */
                                                 /* Offset from base address */
#define  PORTCARR_COMMAND                 (0x04)     /* PCI command register */
#define  PORTCARR_INDEX                   (0x00)           /* Index register */
#define  PORTCARR_DATA                    (0x01)            /* Data register */

/* ------------------------------------------ */
/*  PCI command register (PCICMD) bit assign  */
/* ------------------------------------------ */
#define  BITCARRPCICMD_IOSPACE            (0x0001)
                                                  /* (Bit-0) IO space enable */
#define  BITCARRPCICMD_MEMORYSPACE        (0x0002)
                                              /* (Bit-1) Memory space enable */
#define  BITCARRPCICMD_BUSMASTER          (0x0004)
                                                /* (Bit-2) Bus master enable */
#define  BITCARRPCICMD_SPECIALCYCLES      (0x0008)
                                             /* (Bit-3) Special cycle enable */
#define  BITCARRPCICMD_MEMORYWI           (0x0010)
                               /* (Bit-4) Memory write and invalidate enable */
#define  BITCARRPCICMD_VGASNOOP           (0x0020)
                                                /* (Bit-5) VGA palette snoop */
#define  BITCARRPCICMD_PARITYERRRESPONSE  (0x0040)
                              /* (Bit-6) Enable normal parity error response */
#define  BITCARRPCICMD_WAITCYCLE          (0x0080)
                                               /* (Bit-7) Wait cycle control */
#define  BITCARRPCICMD_SERR               (0x0100)
                                                     /* (Bit-8) SERR# enable */
#define  BITCARRPCICMD_FASTBACKTOBACK     (0x0200)
                                         /* (Bit-9) Fast back-to-back enable */
#define  BITCARRPCICMD_RESERVED10         (0x0400)
                                                        /* (Bit-10) Reserved */
#define  BITCARRPCICMD_RESERVED11         (0x0800)
                                                        /* (Bit-11) Reserved */
#define  BITCARRPCICMD_RESERVED12         (0x1000)
                                                        /* (Bit-12) Reserved */
#define  BITCARRPCICMD_RESERVED13         (0x2000)
                                                        /* (Bit-13) Reserved */
#define  BITCARRPCICMD_RESERVED14         (0x4000)
                                                        /* (Bit-14) Reserved */
#define  BITCARRPCICMD_RESERVED15         (0x8000)
                                                        /* (Bit-15) Reserved */

/* --------------------------------------- */
/*  Index data to index Carrera registers  */
/* --------------------------------------- */
#define  INDEXCARR_PM_CNTRL               (0x00)
                                        /* Power management control register */
#define  INDEXCARR_PM_REQ_STS             (0x01)
                                 /* Power management request status register */
#define  INDEXCARR_CPUCLK_CNTRL           (0x02)
                                               /* CPU clock control register */
#define  INDEXCARR_BL_STS                 (0x03)
                                                /* Backlight status register */
#define  INDEXCARR_BL_CTRL                (0x04)
                                               /* Backlight control register */
#define  INDEXCARR_BL_TMR                 (0x05)
                                            /* Backlight idle timer register */
#define  INDEXCARR_SUSPEND_TMRL           (0x06)
                                          /* Suspend idle timer low register */
#define  INDEXCARR_SUSPEND_TMRH           (0x07)
                                         /* Suspend idle timer high register */
#define  INDEXCARR_STI_ACT_MSK            (0x08)
                                /* Suspend timer idle activity mask register */
#define  INDEXCARR_STI_ACT_STS            (0x09)
                              /* Suspend timer idle activity status register */
#define  INDEXCARR_RESUME_MASK            (0x0A)
                                                     /* Resume mask register */
#define  INDEXCARR_RESUME_STS             (0x0B)
                                                   /* Resume status register */
#define  INDEXCARR_PMCO0                  (0x0C)
                               /* Power management control output 0 register */
#define  INDEXCARR_PMCO1                  (0x0D)
                               /* Power management control output 1 register */
#define  INDEXCARR_PMIN_CTRL              (0x0E)
                                                    /* PMIN control register */
#define  INDEXCARR_CC_PMI_CLR             (0x0F)
                                                    /* CC_PMI clear register */
#define  INDEXCARR_PMI_CLR                (0x10)
                                                       /* PMI clear register */
#define  INDEXCARR_MISC_CTRL              (0x11)
                                                   /* Misc. control register */
#define  INDEXCARR_SPAS                   (0x12)
                                      /* Serial port address status register */
#define  INDEXCARR_SRPT                   (0x13)
                             /* Suspend/Resume programmable timings register */
#define  INDEXCARR_CCCPTR                 (0x14)
                           /* CPU clock change programmable timings register */
#define  INDEXCARR_GP_TMR0_EN             (0x18)
                                               /* GP timer 0 enable register */
#define  INDEXCARR_GP_TMR0_MSK0           (0x19)
                                      /* GP timer 0 activity mask 0 register */
#define  INDEXCARR_GP_TMR0_MSK1           (0x1A)
                                      /* GP timer 0 activity mask 1 register */
#define  INDEXCARR_GP_TMR0_STS0           (0x1B)
                                    /* GP timer 0 activity status 0 register */
#define  INDEXCARR_GP_TMR0_STS1           (0x1C)
                                    /* GP timer 0 activity status 1 register */
#define  INDEXCARR_GP_TMR1_EN             (0x1D)
                                               /* GP timer 1 enable register */
#define  INDEXCARR_GP_TMR1_MSK0           (0x1E)
                                      /* GP timer 1 activity mask 0 register */
#define  INDEXCARR_GP_TMR1_MSK1           (0x1F)
                                      /* GP timer 1 activity mask 1 register */
#define  INDEXCARR_GP_TMR1_STS0           (0x20)
                                    /* GP timer 1 activity status 0 register */
#define  INDEXCARR_GP_TMR1_STS1           (0x21)
                                    /* GP timer 1 activity status 1 register */
#define  INDEXCARR_GP_TMR_STS             (0x22)
                                                 /* GP timer status register */
#define  INDEXCARR_GP_TMR_BYTE0           (0x23)
                                         /* GP timer counter byte 0 register */
#define  INDEXCARR_GP_TMR_BYTE1           (0x24)
                                         /* GP timer counter byte 1 register */
#define  INDEXCARR_GP_TMR_BYTE2           (0x25)
                                         /* GP timer counter byte 2 register */
#define  INDEXCARR_GP_TMR_BYTE3           (0x26)
                                         /* GP timer counter byte 3 register */
#define  INDEXCARR_PIOA_EN0               (0x27)
                                /* Programmable I/O access enable 0 register */
#define  INDEXCARR_PIOA_EN1               (0x28)
                                /* Programmable I/O access enable 1 register */
#define  INDEXCARR_PIOAR0                 (0x29)
                                 /* Programmable I/O access range 0 register */
#define  INDEXCARR_PIOAR1                 (0x2A)
                                 /* Programmable I/O access range 1 register */
#define  INDEXCARR_PIOAR2                 (0x2B)
                                 /* Programmable I/O access range 2 register */
#define  INDEXCARR_PIOAWRS0               (0x2C)
                     /* Programmable I/O access write/read select 0 register */
#define  INDEXCARR_PIOAWRS1               (0x2D)
                     /* Programmable I/O access write/read select 1 register */
#define  INDEXCARR_PIOAS0                 (0x2E)
                                /* Programmable I/O access status 0 register */
#define  INDEXCARR_PIOAS1                 (0x2F)
                                /* Programmable I/O access status 1 register */
#define  INDEXCARR_PIOA0BAL               (0x30)
                      /* Programmable I/O access 0 base address low register */
#define  INDEXCARR_PIOA0BAH               (0x31)
                     /* Programmable I/O access 0 base address high register */
#define  INDEXCARR_PIOA1BAL               (0x32)
                      /* Programmable I/O access 1 base address low register */
#define  INDEXCARR_PIOA1BAH               (0x33)
                     /* Programmable I/O access 1 base address high register */
#define  INDEXCARR_PIOA2BAL               (0x34)
                      /* Programmable I/O access 2 base address low register */
#define  INDEXCARR_PIOA2BAH               (0x35)
                     /* Programmable I/O access 2 base address high register */
#define  INDEXCARR_PIOA3BAL               (0x36)
                      /* Programmable I/O access 3 base address low register */
#define  INDEXCARR_PIOA3BAH               (0x37)
                     /* Programmable I/O access 3 base address high register */
#define  INDEXCARR_PIOA4BAL               (0x38)
                      /* Programmable I/O access 4 base address low register */
#define  INDEXCARR_PIOA4BAH               (0x39)
                     /* Programmable I/O access 4 base address high register */
#define  INDEXCARR_PIOA5BAL               (0x3A)
                      /* Programmable I/O access 5 base address low register */
#define  INDEXCARR_PIOA5BAH               (0x3B)
                     /* Programmable I/O access 5 base address high register */
#define  INDEXCARR_PIOA6BAL               (0x3C)
                      /* Programmable I/O access 6 base address low register */
#define  INDEXCARR_PIOA6BAH               (0x3D)
                     /* Programmable I/O access 6 base address high register */
#define  INDEXCARR_PIOA7BAL               (0x3E)
                      /* Programmable I/O access 7 base address low register */
#define  INDEXCARR_PIOA7BAH               (0x3F)
                     /* Programmable I/O access 7 base address high register */
#define  INDEXCARR_DMA0BASEADDR           (0x80)
                                     /* DMA CH0 base address shadow register */
#define  INDEXCARR_DMA1BASEADDR           (0x81)
                                     /* DMA CH1 base address shadow register */
#define  INDEXCARR_DMA2BASEADDR           (0x82)
                                     /* DMA CH2 base address shadow register */
#define  INDEXCARR_DMA3BASEADDR           (0x83)
                                     /* DMA CH3 base address shadow register */
#define  INDEXCARR_DMA4BASEADDR           (0x84)
                                     /* DMA CH4 base address shadow register */
#define  INDEXCARR_DMA5BASEADDR           (0x85)
                                     /* DMA CH5 base address shadow register */
#define  INDEXCARR_DMA6BASEADDR           (0x86)
                                     /* DMA CH6 base address shadow register */
#define  INDEXCARR_DMA7BASEADDR           (0x87)
                                     /* DMA CH7 base address shadow register */
#define  INDEXCARR_DMA0COUNT              (0x88)
                                       /* DMA CH0 base count shadow register */
#define  INDEXCARR_DMA1COUNT              (0x89)
                                       /* DMA CH1 base count shadow register */
#define  INDEXCARR_DMA2COUNT              (0x8A)
                                       /* DMA CH2 base count shadow register */
#define  INDEXCARR_DMA3COUNT              (0x8B)
                                       /* DMA CH3 base count shadow register */
#define  INDEXCARR_DMA4COUNT              (0x8C)
                                       /* DMA CH4 base count shadow register */
#define  INDEXCARR_DMA5COUNT              (0x8D)
                                       /* DMA CH5 base count shadow register */
#define  INDEXCARR_DMA6COUNT              (0x8E)
                                       /* DMA CH6 base count shadow register */
#define  INDEXCARR_DMA7COUNT              (0x8F)
                                       /* DMA CH7 base count shadow register */
#define  INDEXCARR_DMA0MODE               (0x90)
                                             /* DMA CH0 mode shadow register */
#define  INDEXCARR_DMA1MODE               (0x91)
                                             /* DMA CH1 mode shadow register */
#define  INDEXCARR_DMA2MODE               (0x92)
                                             /* DMA CH2 mode shadow register */
#define  INDEXCARR_DMA3MODE               (0x93)
                                             /* DMA CH3 mode shadow register */
#define  INDEXCARR_DMA4MODE               (0x94)
                                             /* DMA CH4 mode shadow register */
#define  INDEXCARR_DMA5MODE               (0x95)
                                             /* DMA CH5 mode shadow register */
#define  INDEXCARR_DMA6MODE               (0x96)
                                             /* DMA CH6 mode shadow register */
#define  INDEXCARR_DMA7MODE               (0x97)
                                             /* DMA CH7 mode shadow register */
#define  INDEXCARR_DMA0EXTMODE            (0x98)
                                        /* DMA CH0 ext. mode shadow register */
#define  INDEXCARR_DMA1EXTMODE            (0x99)
                                        /* DMA CH1 ext. mode shadow register */
#define  INDEXCARR_DMA2EXTMODE            (0x9A)
                                        /* DMA CH2 ext. mode shadow register */
#define  INDEXCARR_DMA3EXTMODE            (0x9B)
                                        /* DMA CH3 ext. mode shadow register */
#define  INDEXCARR_DMA4EXTMODE            (0x9C)
                                        /* DMA CH4 ext. mode shadow register */
#define  INDEXCARR_DMA5EXTMODE            (0x9D)
                                        /* DMA CH5 ext. mode shadow register */
#define  INDEXCARR_DMA6EXTMODE            (0x9E)
                                        /* DMA CH6 ext. mode shadow register */
#define  INDEXCARR_DMA7EXTMODE            (0x9F)
                                        /* DMA CH7 ext. mode shadow register */
#define  INDEXCARR_DMABYTEPOINTER         (0xA0)
                                                /* DMA byte pointer register */
#define  INDEXCARR_DMACOMMAND_0_3         (0xA1)
                                        /* DMA command CH0-3 shadow register */
#define  INDEXCARR_DMACOMMAND_4_7         (0xA2)
                                        /* DMA command CH4-7 shadow register */
#define  INDEXCARR_PIC1ICW1               (0xA3)
                                               /* PIC 1 ICW1 shadow register */
#define  INDEXCARR_PIC1ICW2               (0xA4)
                                               /* PIC 1 ICW2 shadow register */
#define  INDEXCARR_PIC1ICW3               (0xA5)
                                               /* PIC 1 ICW3 shadow register */
#define  INDEXCARR_PIC1ICW4               (0xA6)
                                               /* PIC 1 ICW4 shadow register */
#define  INDEXCARR_PIC1OCW2               (0xA7)
                                               /* PIC 1 OCW2 shadow register */
#define  INDEXCARR_PIC1OCW3               (0xA8)
                                               /* PIC 1 OCW3 shadow register */
#define  INDEXCARR_PIC2ICW1               (0xA9)
                                               /* PIC 2 ICW1 shadow register */
#define  INDEXCARR_PIC2ICW2               (0xAA)
                                               /* PIC 2 ICW2 shadow register */
#define  INDEXCARR_PIC2ICW3               (0xAB)
                                               /* PIC 2 ICW3 shadow register */
#define  INDEXCARR_PIC2ICW4               (0xAC)
                                               /* PIC 2 ICW4 shadow register */
#define  INDEXCARR_PIC2OCW2               (0xAD)
                                               /* PIC 2 OCW2 shadow register */
#define  INDEXCARR_PIC2OCW3               (0xAE)
                                               /* PIC 2 OCW3 shadow register */
#define  INDEXCARR_PICSEQUENCE            (0xAF)
                                             /* PIC sequence status register */
#define  INDEXCARR_TIMERCOUNTER0L         (0xB0)
                              /* Timer 1 counter 0 count low shadow register */
#define  INDEXCARR_TIMERCOUNTER0H         (0xB1)
                             /* Timer 1 counter 0 count high shadow register */
#define  INDEXCARR_TIMERCOUNTER2L         (0xB2)
                              /* Timer 1 counter 2 count low shadow register */
#define  INDEXCARR_TIMERCOUNTER2H         (0xB3)
                             /* Timer 1 counter 2 count high shadow register */
#define  INDEXCARR_TIMERBYTEPOINTER       (0xB4)
                                       /* Timer byte pointer shadow register */
#define  INDEXCARR_SERIAL1FCR             (0xB5)
                                        /* Serial port 1 FCR shadow register */
#define  INDEXCARR_SERIAL2FCR             (0xB6)
                                        /* Serial port 2 FCR shadow register */
#define  INDEXCARR_PARALLELDIR            (0xB7)
                                  /* Parallel port (DIR bit) shadow register */
#define  INDEXCARR_VIDEODACPALETTEADDR    (0xB8)
                                /* Video DAC palette address shadow register */
#define  INDEXCARR_VIDEODACSTATUS         (0xB9)
                                         /* Video DAC status shadow register */
#define  INDEXCARR_ACRATCHPAD             (0xE0)
                                                     /* Scratch pad register */

/* --------------------------------------------------------- */
/*  Power management control register (PM_CNTRL) bit assign  */
/* --------------------------------------------------------- */
#define  BITCARRPMCNTRL_PMI_CTRL          (0x01)
                                          /* (Bit-0) PMI control             */
#define  BITCARRPMCNTRL_PMI_CC_PMI        (0x02)
                                          /* (Bit-1) Clock change PMI        */
#define  BITCARRPMCNTRL_SUSP_TMR_EN       (0x04)
                                          /* (Bit-2) Suspend timer enable    */
#define  BITCARRPMCNTRL_SUSP_CPU_PWR      (0x08)
                                          /* (Bit-3) CPU power in suspend    */
#define  BITCARRPMCNTRL_CC_EN             (0x10)
                                          /* (Bit-4) Clock change enable     */
#define  BITCARRPMCNTRL_BATTLOW_RSM       (0x20)
                                          /* (Bit-5) Battery in critical     */
#define  BITCARRPMCNTRL_SUSSTAT           (0x40)
                                          /* (Bit-6) Suspend status          */
#define  BITCARRPMCNTRL_PM_EN             (0x80)
                                          /* (Bit-7) Power management enable */

/* ------------------------------------------------------------------ */
/*  Power management request status register (PM_REQ_STS) bit assign  */
/* ------------------------------------------------------------------ */
#define  BITCARRPMREQSTS_CLK_CHG          (0x01)
                                     /* (Bit-0) Clock changed                */
#define  BITCARRPMREQSTS_IO_ACCESS        (0x02)
                                     /* (Bit-1) I/O access detected          */
#define  BITCARRPMREQSTS_GP_TMR_EXP       (0x04)
                                     /* (Bit-2) GP timer expired             */
#define  BITCARRPMREQSTS_EXT_PMI          (0x08)
                                     /* (Bit-3) External PMI suspend request */
#define  BITCARRPMREQSTS_SUSP_TMR_EXP     (0x10)
                                     /* (Bit-4) Suspend timer expired        */
#define  BITCARRPMREQSTS_BL_REQ           (0x20)
                                     /* (Bit-5) Backlight request            */
#define  BITCARRPMREQSTS_PMIN             (0x40)
                                     /* (Bit-6) PMIN request                 */
#define  BITCARRPMREQSTS_RESERVED7        (0x80)
                                     /* (Bit-7) Reserved                     */

/* ------------------------------------------------------ */
/*  CPU clock control register (CPUCLK_CNTRL) bit assign  */
/* ------------------------------------------------------ */
#define  BITCARRCPUCLKCNTRL_PLL_CFG       (0x0F)
                                  /* (Bit-0:3) Phase-lock loop configuration */
#define  BITCARRCPUCLKCNTRL_RESERVED4     (0x10)
                                  /* (Bit-4) Reserved                        */
#define  BITCARRCPUCLKCNTRL_RESERVED5     (0x20)
                                  /* (Bit-5) Reserved                        */
#define  BITCARRCPUCLKCNTRL_RESERVED6     (0x40)
                                  /* (Bit-6) Reserved                        */
#define  BITCARRCPUCLKCNTRL_RESERVED7     (0x80)
                                  /* (Bit-7) Reserved                        */

/* ----------------------------------------------- */
/*  Backlight status register (BL_STS) bit assign  */
/* ----------------------------------------------- */
#define  BITCARRBLSTS_BLTMR_REQ_STS       (0x01)
                                 /* (Bit-0) Backlight idle timer PMI request */
#define  BITCARRBLSTS_BLEVNT_REQ_STS      (0x02)
                                 /* (Bit-1) Backlight event PMI request      */
#define  BITCARRBLSTS_RESERVED2           (0x04)
                                 /* (Bit-2) Reserved                         */
#define  BITCARRBLSTS_RESERVED3           (0x08)
                                 /* (Bit-3) Reserved                         */
#define  BITCARRBLSTS_RESERVED4           (0x10)
                                 /* (Bit-4) Reserved                         */
#define  BITCARRBLSTS_RESERVED5           (0x20)
                                 /* (Bit-5) Reserved                         */
#define  BITCARRBLSTS_RESERVED6           (0x40)
                                 /* (Bit-6) Reserved                         */
#define  BITCARRBLSTS_RESERVED7           (0x80)
                                 /* (Bit-7) Reserved                         */

/* ------------------------------------------------- */
/*  Backlight control register (BL_CTRL) bit assign  */
/* ------------------------------------------------- */
#define  BITCARRBLCTRL_KM_BL_EVNT_EN      (0x01)
                            /* (Bit-0) Keyboard/Mouse backlight event enable */
#define  BITCARRBLCTRL_RESERVED1          (0x02)
                            /* (Bit-1) Reserved                              */
#define  BITCARRBLCTRL_SERIAL_BL_EVNT_EN  (0x04)
                            /* (Bit-2) Serial port backlight event enable    */
#define  BITCARRBLCTRL_RESERVED3          (0x08)
                            /* (Bit-3) Reserved                              */
#define  BITCARRBLCTRL_RESERVED4          (0x10)
                            /* (Bit-4) Reserved                              */
#define  BITCARRBLCTRL_RESERVED5          (0x20)
                            /* (Bit-5) Reserved                              */
#define  BITCARRBLCTRL_BLTMR_PMI_EN       (0x40)
                            /* (Bit-6) Backlight timer PMI enable            */
#define  BITCARRBLCTRL_BLEVNT_PMI_EN      (0x80)
                            /* (Bit-7) Backlight event PMI enable            */

/* ----------------------------------------------- */
/*  Resume mask register (RESUME_MASK) bit assign  */
/* ----------------------------------------------- */
#define  BITCARRRESUMEMASK_RING_MSK       (0x01)
                                              /* (Bit-0) Ring indicator mask */
#define  BITCARRRESUMEMASK_ALARM_MSK      (0x02)
                                              /* (Bit-1) Alarm mask          */
#define  BITCARRRESUMEMASK_H8_RSM_MSK     (0x04)
                                              /* (Bit-2) H8 resume mask      */
#define  BITCARRRESUMEMASK_RESERVED3      (0x08)
                                              /* (Bit-3) Reserved            */
#define  BITCARRRESUMEMASK_RESERVED4      (0x10)
                                              /* (Bit-4) Reserved            */
#define  BITCARRRESUMEMASK_RESERVED5      (0x20)
                                              /* (Bit-5) Reserved            */
#define  BITCARRRESUMEMASK_RESERVED6      (0x40)
                                              /* (Bit-6) Reserved            */
#define  BITCARRRESUMEMASK_RESERVED7      (0x80)
                                              /* (Bit-7) Reserved            */

/* ------------------------------------------------ */
/*  Resume status register (RESUME_STS) bit assign  */
/* ------------------------------------------------ */
#define  BITCARRRESUMESTS_RING_STS        (0x01)
                                      /* (Bit-0) Ring event status indicator */
#define  BITCARRRESUMESTS_ALARM_STS       (0x02)
                                      /* (Bit-1) Alarm indicator             */
#define  BITCARRRESUMESTS_H8_RSM_STS      (0x04)
                                      /* (Bit-2) H8 resume status            */
#define  BITCARRRESUMESTS_RESERVED3       (0x08)
                                      /* (Bit-3) Reserved                    */
#define  BITCARRRESUMESTS_RESERVED4       (0x10)
                                      /* (Bit-4) Reserved                    */
#define  BITCARRRESUMESTS_RESERVED5       (0x20)
                                      /* (Bit-5) Reserved                    */
#define  BITCARRRESUMESTS_RESERVED6       (0x40)
                                      /* (Bit-6) Reserved                    */
#define  BITCARRRESUMESTS_RESERVED7       (0x80)
                                      /* (Bit-7) Reserved                    */

/* --------------------------------------------------------------- */
/*  Power management control output 0 register (PMCO0) bit assign  */
/* --------------------------------------------------------------- */
#define  BITCARRPMCO0_LCDBACKLIGHTOFF     (0x01)
                                        /* (Bit-0) LCD backlight off         */
                                        /*         (1=on, 0=off)             */
#define  BITCARRPMCO0_LCDOFF              (0x02)
                                        /* (Bit-1) LCD power off             */
                                        /*         (1=on, 0=off)             */
#define  BITCARRPMCO0_LCDENABLE           (0x04)
                                        /* (Bit-2) LCD enable                */
#define  BITCARRPMCO0_LCDHALFBRIGHTNESS   (0x08)
                                        /* (Bit-3) LCD half brightness       */
                                        /*         (1=full, 0=half)          */
#define  BITCARRPMCO0_VIDEOLOWPOWER       (0x10)
                                        /* (Bit-4) Video chip low power      */
                                        /*         (1=on, 0=off)             */
#define  BITCARRPMCO0_CAMERAOFF           (0x20)
                                        /* (Bit-5) Camera power off          */
                                        /*         (1=on, 0=off)             */
#define  BITCARRPMCO0_VIDEODIGITIZEROFF   (0x40)
                                        /* (Bit-6) Video digitizer power off */
                                        /*         (1=on, 0=off)             */
#define  BITCARRPMCO0_RESERVED7           (0x80)
                                        /* (Bit-7) Reserved                  */

/* --------------------------------------------------------------- */
/*  Power management control output 1 register (PMCO1) bit assign  */
/* --------------------------------------------------------------- */
#define  BITCARRPMCO1_SCSICLOCKSTOP       (0x01)
                                      /* (Bit-0) SCSI clock stop             */
                                      /*         (1=on, 0=off)               */
#define  BITCARRPMCO1_SCSITERMLOWPOWER    (0x06)
                                      /* (Bit-1,2) SCSI terminator low power */
#define     BITCARRPMCO1_SCSITERMON110    (0x00)
                                      /*           Terminator on (110 ohm)   */
#define     BITCARRPMCO1_SCSITERMON25K    (0x02)
                                      /*           Terminator on (2.5 Kohm)  */
#define     BITCARRPMCO1_SCSITERM64       (0x04)
                                      /*           Terminator ???            */
#define     BITCARRPMCO1_SCSITERMDISC     (0x06)
                                      /*           Terminator disconnect     */
#define  BITCARRPMCO1_CDROMDRIVELOWPOWER  (0x08)
                                      /* (Bit-3) CD-ROM drive low power      */
#define  BITCARRPMCO1_AUDIOLOWPOWER       (0x10)
                                      /* (Bit-4) Audio low power             */
                                      /*         (1=on, 0=off)               */
#define  BITCARRPMCO1_CRTLOWPOWER         (0x60)
                                      /* (Bit-5,6) CRT low power             */
                                      /*   (Bit-5) Hsync                     */
                                      /*   (Bit-6) Vsync                     */
#define     BITCARRPMCO1_CRTOFF           (0x00)
                                      /*           CRT off                   */
#define     BITCARRPMCO1_CRTSUSPEND       (0x20)
                                      /*           CRT suspend               */
#define     BITCARRPMCO1_CRTSTANDBY       (0x40)
                                      /*           CRT stand-by              */
#define     BITCARRPMCO1_CRTON            (0x60)
                                      /*           CRT on                    */
#define  BITCARRPMCO1_L2CACHELOWPOWER     (0x80)
                                      /* (Bit-7) L2 cache low power          */
                                      /*         (1=on, 0=off)               */

/* ---------------------------------------------- */
/*  PMIN control register (PMIN_CTRL) bit assign  */
/* ---------------------------------------------- */
#define  BITCARRPMINCTRL_HDDACTIVE        (0x01)
                                         /* (Bit-0) Internal HDD activity    */
#define  BITCARRPMINCTRL_CDROMACTIVE      (0x02)
                                         /* (Bit-1) Internal CD-ROM activity */
#define  BITCARRPMINCTRL_RESERVED2        (0x04)
                                         /* (Bit-2) Reserved                 */
#define  BITCARRPMINCTRL_RESERVED3        (0x08)
                                         /* (Bit-3) Reserved                 */
#define  BITCARRPMINCTRL_RESERVED4        (0x10)
                                         /* (Bit-4) Reserved                 */
#define  BITCARRPMINCTRL_RESERVED5        (0x20)
                                         /* (Bit-5) Reserved                 */
#define  BITCARRPMINCTRL_RESERVED6        (0x40)
                                         /* (Bit-6) Reserved                 */
#define  BITCARRPMINCTRL_RESERVED7        (0x80)
                                         /* (Bit-7) Reserved                 */

/* ----------------------------------------------- */
/*  Misc. control register (MISC_CTRL) bit assign  */
/* ----------------------------------------------- */
#define  BITCARRMISCCTRL_SET_DSKCHG       (0x01)
                                      /* (Bit-0) Set DSKCHG#                 */
#define  BITCARRMISCCTRL_BLMASK228        (0x02)
                                      /* (Bit-1) Backlight mask address 228h */
#define  BITCARRMISCCTRL_SUSMASK228       (0x04)
                                      /* (Bit-2) Suspend mask address 228h   */
#define  BITCARRMISCCTRL_EN_GATECLK       (0x08)
                                      /* (Bit-3) Clock gate off enable       */
#define  BITCARRMISCCTRL_CC_PMI_PMI       (0x10)
                                      /* (Bit-4) CC_PMI to PMI               */
#define  BITCARRMISCCTRL_RESERVED5        (0x20)
                                      /* (Bit-5) Reserved                    */
#define  BITCARRMISCCTRL_RESERVED6        (0x40)
                                      /* (Bit-6) Reserved                    */
#define  BITCARRMISCCTRL_RESERVED7        (0x80)
                                      /* (Bit-7) Reserved                    */

/* ---------------------------------------------------------------- */
/*  Suspend/Resume programmable timings register (SRPT) bit assign  */
/* ---------------------------------------------------------------- */
#define  BITCARRSRPT_HS                   (0x03)
                       /* (Bit-0,1) HOST_SUSSTAT# to SUSSTAT# timing control */
#define     BITCARRSRPT_HS_1RTCCLK        (0x00)
                       /*           1 RTCCLK                                 */
#define     BITCARRSRPT_HS_2RTCCLK        (0x01)
                       /*           2 RTCCLKs (default)                      */
#define     BITCARRSRPT_HS_4RTCCLK        (0x02)
                       /*           4 RTCCLKs                                */
#define     BITCARRSRPT_HS_8RTCCLK        (0x03)
                       /*           8 RTCCLKs                                */
#define  BITCARRSRPT_SH                   (0x0C)
                       /* (Bit-2,3) SUSSTAT# to HOST_SUSSTAT# timing control */
#define     BITCARRSRPT_SH_30MSEC         (0x00)
                       /*            30 msec (default)                       */
#define     BITCARRSRPT_SH_100MSEC        (0x04)
                       /*           100 msec                                 */
#define     BITCARRSRPT_SH_200MSEC        (0x08)
                       /*           200 msec                                 */
#define     BITCARRSRPT_SH_400MSEC        (0x0C)
                       /*           400 msec                                 */
#define  BITCARRSRPT_RP                   (0x30)
                       /* (Bit-4,5) RSM_RST# pulse width control             */
#define     BITCARRSRPT_RP_1MSEC          (0x00)
                       /*           1 msec (default)                         */
#define     BITCARRSRPT_RP_2MSEC          (0x10)
                       /*           2 msec                                   */
#define     BITCARRSRPT_RP_4MSEC          (0x20)
                       /*           4 msec                                   */
#define     BITCARRSRPT_RP_8MSEC          (0x30)
                       /*           8 msec                                   */
#define  BITCARRSRPT_RESERVED6            (0x40)
                       /* (Bit-6) Reserved                                   */
#define  BITCARRSRPT_RESERVED7            (0x80)
                       /* (Bit-7) Reserved                                   */

/* -------------------------------------------------------------------- */
/*  CPU clock change programmable timings register (CCCPTR) bit assign  */
/* -------------------------------------------------------------------- */
#define  BITCARRCCCPTR_QL                 (0x03)
                     /* (Bit-0,1) QREQ# to CCC_BUF loaded timing control     */
#define     BITCARRCCCPTR_QL_8SYSCLK      (0x00)
                     /*            8 SYSCLKs                                 */
#define     BITCARRCCCPTR_QL_12SYSCLK     (0x01)
                     /*           12 SYSCLKs (default)                       */
#define     BITCARRCCCPTR_QL_16SYSCLK     (0x02)
                     /*           16 SYSCLKs                                 */
#define     BITCARRCCCPTR_QL_32SYSCLK     (0x03)
                     /*           32 SYSCLKs                                 */
#define  BITCARRCCCPTR_LI                 (0x0C)
                     /* (Bit-2,3) CCC_BUF loaded to interrupt timing control */
#define     BITCARRCCCPTR_LI_60USEC       (0x00)
                     /*            60 usec                                   */
#define     BITCARRCCCPTR_LI_80USEC       (0x04)
                     /*            80 usec                                   */
#define     BITCARRCCCPTR_LI_100USEC      (0x08)
                     /*           100 usec (default)                         */
#define     BITCARRCCCPTR_LI_150USEC      (0x0C)
                     /*           150 usec                                   */
#define  BITCARRCCCPTR_RESERVED4          (0x10)
                     /* (Bit-4) Reserved                                     */
#define  BITCARRCCCPTR_RESERVED5          (0x20)
                     /* (Bit-5) Reserved                                     */
#define  BITCARRCCCPTR_RESERVED6          (0x40)
                     /* (Bit-6) Reserved                                     */
#define  BITCARRCCCPTR_RESERVED7          (0x80)
                     /* (Bit-7) Reserved                                     */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *   Carrera Controller Definition   * */
/* ************************************* */
/* ------------------- */
/*  Carrera I/O class  */
/* ------------------- */
VOID  CarreraIo(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */

/* ----------------------- */
/*  Carrera control class  */
/* ----------------------- */
VOID  CarreraCtl(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj);            /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *          Carrera Objects          * */
/* ************************************* */
/* ------------------- */
/*  Carrera I/O class  */
/* ------------------- */
OBJCARRERAIO   oCarreraIo  = {(POBJCTL)CarreraIo, PORTCARR_BASENULL, 0};

/* ----------------------- */
/*  Carrera control class  */
/* ----------------------- */
OBJCARRERACTL  oCarreraCtl = {(POBJCTL)CarreraCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ******************************************* */
/* *         Carrera I/O Controller          * */
/* ******************************************* */
/* *                                         * */
/* * - Initialize                            * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_INITIALIZE   * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* * - Get index/data register               * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_GET_INDEX    * */
/* *                    MSGCARR_GET_DATA     * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *       Parameter1 = index/data value     * */
/* *                                         * */
/* * - Set index/data register               * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_SET_INDEX    * */
/* *                    MSGCARR_SET_DATA     * */
/* *       Parameter1 = index/data value     * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* * - Get revision ID                       * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_GET_REVISION * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *       Parameter1 = revision ID          * */
/* *                                         * */
/* * - Read any register                     * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_READ_DATA    * */
/* *       Parameter1 = register index       * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *       Parameter1 = register value       * */
/* *                                         * */
/* * - Write any register                    * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_WRITE_DATA   * */
/* *       Parameter1 = register index       * */
/* *       Parameter2 = register value       * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* * - Read & Modify any register            * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_MODIFY_DATA  * */
/* *       Parameter1 = register index       * */
/* *       Parameter2 = set bits value       * */
/* *       Parameter3 = mask bits value      * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* * - Read register                         * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_READ_xxx     * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *       Parameter1 = register value       * */
/* *                                         * */
/* * - Write register                        * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_WRITE_xxx    * */
/* *       Parameter1 = register value       * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* * - Read & Modify register                * */
/* *     [ Input ]                           * */
/* *       Message    = MSGCARR_MODIFY_xxx   * */
/* *       Parameter1 = set bits value       * */
/* *       Parameter2 = mask bits value      * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* ******************************************* */
VOID  CarreraIo(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   MSGPCI         mPci;
   BYTE           index, sindex;
   CRESULT        rc;
   POBJCARRERAIO  pcarrio;
   DWORD          pmindex, pmdata;

   /* Get pointer to Carrera I/O object */
   /*                                   */
   pcarrio = (POBJCARRERAIO)poObj;

   /* Check if completing for initialization */
   /*                                        */
   if((pcarrio->BaseAddress  != PORTCARR_BASENULL) ||
      (SelectMessage(*pmMsg) == MSGCARR_INITIALIZE)  )
   {
      /* Get index/data port address */
      /*                             */
      pmindex = pcarrio->BaseAddress + PORTCARR_INDEX + GetIoBaseAddress();
      pmdata  = pcarrio->BaseAddress + PORTCARR_DATA  + GetIoBaseAddress();

      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGCARR_INITIALIZE:
            /* Initialize */
            /*            */
            BuildMsg(mPci, MSGPCI_QUERY_DEVICE);
            SendMsg(mPci, oPciCarreraCtl);

            if(!(rc = SelectResult(mPci)))
            {
               BuildMsg(mPci, MSGPCI_GET_BASEADDRESS0);
               SendMsg(mPci, oPciCarreraCtl);

               if(!(rc = SelectResult(mPci)))
               {
                  pcarrio->BaseAddress = SelectPciParm1(mPci) &
                                         PORTCARR_BASEMASK;

                  BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                          PORTCARR_COMMAND);
                  BuildPciParm1(mPci, BITCARRPCICMD_IOSPACE);
                  SendMsg(mPci, oPciCarreraCtl);

                  if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                  {
                     rc = ERRCARR_CANNOT_PCIACCESS;
                  } /* endif */
               }
               else
               {
                  rc = ERRCARR_CANNOT_INITIALIZE;
               } /* endif */

               BuildMsg(mPci, MSGPCI_GET_REVISIONID);
               SendMsg(mPci, oPciCarreraCtl);
               pcarrio->Revision = !SelectResult(mPci) ? SelectParm1(mPci) : 0;
            }
            else
            {
               rc = ERRCARR_CANNOT_INITIALIZE;
            } /* endif */
            break;

         case MSGCARR_GET_INDEX:
            /* Get index register */
            /*                    */
            SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(pmindex);
            rc = ERRCARR_NOERROR;
            break;

         case MSGCARR_SET_INDEX:
            /* Set index register */
            /*                    */
            WriteControllerByte(pmindex, (BYTE)SelectParm1(*pmMsg));
            rc = ERRCARR_NOERROR;
            break;

         case MSGCARR_GET_DATA:
            /* Get data register */
            /*                   */
            SelectParm1(*pmMsg) = (CPARAMETER)ReadControllerByte(pmdata);
            rc = ERRCARR_NOERROR;
            break;

         case MSGCARR_SET_DATA:
            /* Set data register */
            /*                   */
            WriteControllerByte(pmdata, (BYTE)SelectParm1(*pmMsg));
            rc = ERRCARR_NOERROR;
            break;

         case MSGCARR_GET_REVISION:
            /* Get revision ID */
            /*                 */
            SelectParm1(*pmMsg) = (CPARAMETER)pcarrio->Revision;
            rc = ERRCARR_NOERROR;
            break;

         default:
            /* Get index register */
            /*                    */
            switch(SelectMessage(*pmMsg))
            {
               case MSGCARR_READ_DATA:
               case MSGCARR_WRITE_DATA:
               case MSGCARR_MODIFY_DATA:
                  index = (BYTE)SelectParm1(*pmMsg);
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_PM_CNTRL:
               case MSGCARR_WRITE_PM_CNTRL:
               case MSGCARR_MODIFY_PM_CNTRL:
                  index = INDEXCARR_PM_CNTRL;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_PM_REQ_STS:
                  index = INDEXCARR_PM_REQ_STS;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_CPUCLK_CNTRL:
               case MSGCARR_WRITE_CPUCLK_CNTRL:
               case MSGCARR_MODIFY_CPUCLK_CNTRL:
                  index = INDEXCARR_CPUCLK_CNTRL;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_BL_STS:
                  index = INDEXCARR_BL_STS;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_BL_CTRL:
               case MSGCARR_WRITE_BL_CTRL:
               case MSGCARR_MODIFY_BL_CTRL:
                  index = INDEXCARR_BL_CTRL;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_BL_TMR:
               case MSGCARR_WRITE_BL_TMR:
                  index = INDEXCARR_BL_TMR;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_SUSPEND_TMRL:
               case MSGCARR_WRITE_SUSPEND_TMRL:
                  index = INDEXCARR_SUSPEND_TMRL;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_SUSPEND_TMRH:
               case MSGCARR_WRITE_SUSPEND_TMRH:
                  index = INDEXCARR_SUSPEND_TMRH;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_STI_ACT_MSK:
               case MSGCARR_WRITE_STI_ACT_MSK:
               case MSGCARR_MODIFY_STI_ACT_MSK:
                  index = INDEXCARR_STI_ACT_MSK;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_STI_ACT_STS:
               case MSGCARR_WRITE_STI_ACT_STS:
                  index = INDEXCARR_STI_ACT_STS;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_RESUME_MASK:
               case MSGCARR_WRITE_RESUME_MASK:
               case MSGCARR_MODIFY_RESUME_MASK:
                  index = INDEXCARR_RESUME_MASK;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_RESUME_STS:
                  index = INDEXCARR_RESUME_STS;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_PMCO0:
               case MSGCARR_WRITE_PMCO0:
               case MSGCARR_MODIFY_PMCO0:
                  index = INDEXCARR_PMCO0;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_PMCO1:
               case MSGCARR_WRITE_PMCO1:
               case MSGCARR_MODIFY_PMCO1:
                  index = INDEXCARR_PMCO1;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_PMIN_CTRL:
               case MSGCARR_WRITE_PMIN_CTRL:
               case MSGCARR_MODIFY_PMIN_CTRL:
                  index = INDEXCARR_PMIN_CTRL;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_WRITE_CC_PMI_CLR:
                  index = INDEXCARR_CC_PMI_CLR;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_WRITE_PMI_CLR:
                  index = INDEXCARR_PMI_CLR;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_MISC_CTRL:
               case MSGCARR_WRITE_MISC_CTRL:
               case MSGCARR_MODIFY_MISC_CTRL:
                  index = INDEXCARR_MISC_CTRL;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_SPAS:
                  index = INDEXCARR_SPAS;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_SRPT:
               case MSGCARR_WRITE_SRPT:
               case MSGCARR_MODIFY_SRPT:
                  index = INDEXCARR_SRPT;
                  rc    = ERRCARR_NOERROR;
                  break;
               case MSGCARR_READ_CCCPTR:
               case MSGCARR_WRITE_CCCPTR:
               case MSGCARR_MODIFY_CCCPTR:
                  index = INDEXCARR_CCCPTR;
                  rc    = ERRCARR_NOERROR;
                  break;
               default:
                  rc = ERRCARR_INVALID_MESSAGE;
            } /* endswitch */

            if(!rc)
            {
               /* Save index register */
               /*                     */
               sindex = ReadControllerByte(pmindex);

               /* Input/Output registers */
               /*                        */
               switch(SelectMessage(*pmMsg))
               {
                  case MSGCARR_READ_DATA:
                  case MSGCARR_READ_PM_CNTRL:
                  case MSGCARR_READ_PM_REQ_STS:
                  case MSGCARR_READ_CPUCLK_CNTRL:
                  case MSGCARR_READ_BL_STS:
                  case MSGCARR_READ_BL_CTRL:
                  case MSGCARR_READ_BL_TMR:
                  case MSGCARR_READ_SUSPEND_TMRL:
                  case MSGCARR_READ_SUSPEND_TMRH:
                  case MSGCARR_READ_STI_ACT_MSK:
                  case MSGCARR_READ_STI_ACT_STS:
                  case MSGCARR_READ_RESUME_MASK:
                  case MSGCARR_READ_RESUME_STS:
                  case MSGCARR_READ_PMCO0:
                  case MSGCARR_READ_PMCO1:
                  case MSGCARR_READ_PMIN_CTRL:
                  case MSGCARR_READ_MISC_CTRL:
                  case MSGCARR_READ_SPAS:
                  case MSGCARR_READ_SRPT:
                  case MSGCARR_READ_CCCPTR:
                     WriteControllerByte(pmindex, index);
                     SelectParm1(*pmMsg) =
                                         (CPARAMETER)ReadControllerByte(pmdata);
                     break;
                  case MSGCARR_WRITE_PM_CNTRL:
                  case MSGCARR_WRITE_CPUCLK_CNTRL:
                  case MSGCARR_WRITE_BL_CTRL:
                  case MSGCARR_WRITE_BL_TMR:
                  case MSGCARR_WRITE_SUSPEND_TMRL:
                  case MSGCARR_WRITE_SUSPEND_TMRH:
                  case MSGCARR_WRITE_STI_ACT_MSK:
                  case MSGCARR_WRITE_STI_ACT_STS:
                  case MSGCARR_WRITE_RESUME_MASK:
                  case MSGCARR_WRITE_PMCO0:
                  case MSGCARR_WRITE_PMCO1:
                  case MSGCARR_WRITE_PMIN_CTRL:
                  case MSGCARR_WRITE_CC_PMI_CLR:
                  case MSGCARR_WRITE_PMI_CLR:
                  case MSGCARR_WRITE_MISC_CTRL:
                  case MSGCARR_WRITE_SRPT:
                  case MSGCARR_WRITE_CCCPTR:
                     WriteControllerByte(pmindex, index);
                     WriteControllerByte(pmdata,  (BYTE)SelectParm1(*pmMsg));
                     break;
                  case MSGCARR_WRITE_DATA:
                     WriteControllerByte(pmindex, index);
                     WriteControllerByte(pmdata,  (BYTE)SelectParm2(*pmMsg));
                     break;
                  case MSGCARR_MODIFY_PM_CNTRL:
                  case MSGCARR_MODIFY_CPUCLK_CNTRL:
                  case MSGCARR_MODIFY_BL_CTRL:
                  case MSGCARR_MODIFY_STI_ACT_MSK:
                  case MSGCARR_MODIFY_RESUME_MASK:
                  case MSGCARR_MODIFY_PMCO0:
                  case MSGCARR_MODIFY_PMCO1:
                  case MSGCARR_MODIFY_PMIN_CTRL:
                  case MSGCARR_MODIFY_MISC_CTRL:
                  case MSGCARR_MODIFY_SRPT:
                  case MSGCARR_MODIFY_CCCPTR:
                     WriteControllerByte(pmindex, index);
                     WriteControllerByte(pmdata,
                                         (BYTE)((ReadControllerByte(pmdata) &
                                                 ~SelectParm2(*pmMsg))      |
                                                (SelectParm1(*pmMsg)        &
                                                 SelectParm2(*pmMsg))        ));
                     break;
                  case MSGCARR_MODIFY_DATA:
                     WriteControllerByte(pmindex, index);
                     WriteControllerByte(pmdata,
                                         (BYTE)((ReadControllerByte(pmdata) &
                                                 ~SelectParm3(*pmMsg))      |
                                                (SelectParm2(*pmMsg)        &
                                                 SelectParm3(*pmMsg))        ));
                     break;
                  default:
                     rc = ERRCARR_INVALID_MESSAGE;
               } /* endswitch */

               /* Restore index register */
               /*                        */
               IoDelay();
               WriteControllerByte(pmindex, sindex);
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERRCARR_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ******************************************************* */
/* *             Carrera Control Controller              * */
/* ******************************************************* */
/* *                                                     * */
/* * - Initialize                                        * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_INITIALIZECTL            * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get PM status                                     * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_xxxSTATUS            * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_xxxENABLE )             * */
/* *                    (PRMCARR_xxxDISABLE)             * */
/* *                                                     * */
/* * - Set PM status                                     * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_xxxSTATUS            * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_xxxENABLE )             * */
/* *                    (PRMCARR_xxxDISABLE)             * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get power management interrupt status             * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_PMISTATUS            * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_PMISMI)                 * */
/* *                    (PRMCARR_PMIIRQ)                 * */
/* *                                                     * */
/* * - Set power management interrupt status             * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_PMISTATUS            * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_PMISMI)                 * */
/* *                    (PRMCARR_PMIIRQ)                 * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get backlight idle timer                          * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_BLTIMER              * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = timer value                      * */
/* *                                                     * */
/* * - Set backlight idle timer                          * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_BLTIMER              * */
/* *       Parameter1 = timer value                      * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get CPU clock rate                                * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_CPUCLOCKRATE         * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = rate (PLL_CFG)                   * */
/* *                                                     * */
/* * - Set CPU clock rate                                * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_CPUCLOCKRATE         * */
/* *       Parameter1 = rate (PLL_CFG)                   * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Set timings                                       * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_SUSPENDRESUMETIMING  * */
/* *                    MSGCARR_SET_CPUCLOCKCHANGETIMING * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Query power management request status             * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_QUERY_PMREQUEST          * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = PM request                       * */
/* *       Parameter2 = Backlight request                * */
/* *                                                     * */
/* * - Query resume request status                       * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_QUERY_RESUMEREQUEST      * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = resume request                   * */
/* *                                                     * */
/* * - Clear power management interrupt                  * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_CLEAR_PMI                * */
/* *       Parameter1 = pmi                              * */
/* *                    (PRMCARR_CLRPMISMI)              * */
/* *                    (PRMCARR_CLRPMIIRQ)              * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Inhibit power management interrupt                * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_INHIBIT_PMI              * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Enter suspend state                               * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_ENTER_SUSPEND            * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Exit suspend state                                * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_EXIT_SUSPEND             * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Set all device power off                          * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_DEVICEALLOFF         * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get power line status                             * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_LCDBACKLIGHTOFF      * */
/* *                    MSGCARR_GET_LCDOFF               * */
/* *                    MSGCARR_GET_LCDENABLE            * */
/* *                    MSGCARR_GET_LCDHALFBRIGHTNESS    * */
/* *                    MSGCARR_GET_VIDEOLOWPOWER        * */
/* *                    MSGCARR_GET_CAMERAOFF            * */
/* *                    MSGCARR_GET_VIDEODIGITIZEROFF    * */
/* *                    MSGCARR_GET_SCSICLOCKSTOP        * */
/* *                    MSGCARR_GET_CDROMDRIVELOWPOWER   * */
/* *                    MSGCARR_GET_AUDIOLOWPOWER        * */
/* *                    MSGCARR_GET_L2CACHELOWPOWER      * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_LINEHIGH)               * */
/* *                    (PRMCARR_LINELOW )               * */
/* *                                                     * */
/* * - Set power line status                             * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_LCDBACKLIGHTOFF      * */
/* *                    MSGCARR_SET_LCDOFF               * */
/* *                    MSGCARR_SET_LCDENABLE            * */
/* *                    MSGCARR_SET_LCDHALFBRIGHTNESS    * */
/* *                    MSGCARR_SET_VIDEOLOWPOWER        * */
/* *                    MSGCARR_SET_CAMERAOFF            * */
/* *                    MSGCARR_SET_VIDEODIGITIZEROFF    * */
/* *                    MSGCARR_SET_SCSICLOCKSTOP        * */
/* *                    MSGCARR_SET_CDROMDRIVELOWPOWER   * */
/* *                    MSGCARR_SET_AUDIOLOWPOWER        * */
/* *                    MSGCARR_SET_L2CACHELOWPOWER      * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_LINEHIGH)               * */
/* *                    (PRMCARR_LINELOW )               * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get SCSI terminator power status                  * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_SCSITERMLOWPOWER     * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_TERMINATORON )          * */
/* *                    (PRMCARR_TERMINATORSTANDBY)      * */
/* *                    (PRMCARR_TERMINATOROFF)          * */
/* *                                                     * */
/* * - Set SCSI terminator power status                  * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_SCSITERMLOWPOWER     * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_TERMINATORON )          * */
/* *                    (PRMCARR_TERMINATORSTANDBY)      * */
/* *                    (PRMCARR_TERMINATOROFF)          * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get CRT power status                              * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_CRTLOWPOWER          * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_CRTPOWERON     )        * */
/* *                    (PRMCARR_CRTPOWERSTANDBY)        * */
/* *                    (PRMCARR_CRTPOWERSUSPEND)        * */
/* *                    (PRMCARR_CRTPOWEROFF    )        * */
/* *                                                     * */
/* * - Set CRT power status                              * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SET_CRTLOWPOWER          * */
/* *       Parameter1 = status                           * */
/* *                    (PRMCARR_CRTPOWERON     )        * */
/* *                    (PRMCARR_CRTPOWERSTANDBY)        * */
/* *                    (PRMCARR_CRTPOWERSUSPEND)        * */
/* *                    (PRMCARR_CRTPOWEROFF    )        * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Get programmable interrupt controller registers   * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_PIC1REGISTERS        * */
/* *                    MSGCARR_GET_PIC2REGISTERS        * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = ICW 1                            * */
/* *       Parameter2 = ICW 2                            * */
/* *       Parameter3 = ICW 3                            * */
/* *       Parameter4 = ICW 4                            * */
/* *       Parameter5 = OCW 2                            * */
/* *       Parameter6 = OCW 3                            * */
/* *       Parameter7 = PIC sequence status              * */
/* *                                                     * */
/* * - Get programmable interval timer registers         * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_PIT1REGISTERS        * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = counter 0 count low              * */
/* *       Parameter2 = counter 0 count high             * */
/* *       Parameter3 = counter 2 count low              * */
/* *       Parameter4 = counter 2 count high             * */
/* *       Parameter5 = Timer byte pointer               * */
/* *                                                     * */
/* * - Get serial port register                          * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_SERIAL1REGISTERS     * */
/* *                    MSGCARR_GET_SERIAL2REGISTERS     * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = FIFO control                     * */
/* *                                                     * */
/* * - Get direct memory access controller registers     * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_GET_DMA1REGISTERS        * */
/* *                    MSGCARR_GET_DMA2REGISTERS        * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *       Parameter1 = channel 0 base address           * */
/* *       Parameter2 = channel 1 base address           * */
/* *       Parameter3 = channel 2 base address           * */
/* *       Parameter4 = channel 3 base address           * */
/* *       Parameter5 = channel 0 base count             * */
/* *       Parameter6 = channel 1 base count             * */
/* *       Parameter7 = channel 2 base count             * */
/* *       Parameter8 = channel 3 base count             * */
/* *       Parameter9 = channel 0 mode                   * */
/* *       Parameter10= channel 1 mode                   * */
/* *       Parameter11= channel 2 mode                   * */
/* *       Parameter12= channel 3 mode                   * */
/* *       Parameter13= channel 0 extended mode          * */
/* *       Parameter14= channel 1 extended mode          * */
/* *       Parameter15= channel 2 extended mode          * */
/* *       Parameter16= channel 3 extended mode          * */
/* *       Parameter17= DMA command                      * */
/* *       Parameter18= DMA byte pointer                 * */
/* *                                                     * */
/* * - Save device context                               * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_SAVE_CONTEXT             * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* * - Restore device context                            * */
/* *     [ Input ]                                       * */
/* *       Message    = MSGCARR_RESTORE_CONTEXT          * */
/* *     [ Output ]                                      * */
/* *       Result     = result code                      * */
/* *                                                     * */
/* ******************************************************* */
VOID  CarreraCtl(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj)             /* Pointer to Common object */
{
   MSGRTC         mRtc;
   MSGCARRERA     msg;
   CRESULT        rc;
   POBJCARRERACTL pcarrctl;

   /* Get pointer to Carrera control object */
   /*                                       */
   pcarrctl = (POBJCARRERACTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGCARR_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGCARR_INITIALIZE);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGCARR_WRITE_PM_CNTRL,
                                   0);
            SendMsg(msg, oCarreraIo);

            BuildMsgWithParm1(msg, MSGCARR_WRITE_MISC_CTRL,
                                   BITCARRMISCCTRL_EN_GATECLK);
            SendMsg(msg, oCarreraIo);

            BuildMsgWithParm1(msg, MSGCARR_SET_RISTATUS,
                                   PRMCARR_RIENABLE);
            SendMsg(msg, oCarreraCtl);

            BuildMsgWithParm1(msg, MSGCARR_SET_ALARMSTATUS,
                                   PRMCARR_ALARMENABLE);
            SendMsg(msg, oCarreraCtl);

            BuildMsgWithParm1(msg, MSGCARR_SET_H8RSMSTATUS,
                                   PRMCARR_H8RSMENABLE);
            SendMsg(msg, oCarreraCtl);

            BuildMsgWithParm1(msg, MSGCARR_SET_PMSTATUS,
                                   PRMCARR_PMENABLE);
            SendMsg(msg, oCarreraCtl);

            if(!(rc = SelectResult(msg)))
            {
               BuildMsgWithParm1(mRtc, MSGRTC_SET_AUXBATTERYSTATUS,
                                       PRMRTC_AUXBATENABLE);
               SendMsg(mRtc, oRtcCtl);

               BuildMsgWithParm1(mRtc, MSGRTC_SET_SQUAREWAVEOUTPUT,
                                       PRMRTC_SQW32768HZ);
               SendMsg(mRtc, oRtcCtl);
            } /* endif */
         } /* endif */
         break;

      case MSGCARR_GET_PMSTATUS:
         /* Get power management status */
         /*                             */
         BuildMsg(msg, MSGCARR_READ_PM_CNTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITCARRPMCNTRL_PM_EN)
                                  ? (CPARAMETER)PRMCARR_PMENABLE
                                  : (CPARAMETER)PRMCARR_PMDISABLE;
         } /* endif */
         break;

      case MSGCARR_SET_PMSTATUS:
         /* Set power management status */
         /*                             */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_PMENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                      BITCARRPMCNTRL_PM_EN,
                                      BITCARRPMCNTRL_PM_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_PMDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                      0,
                                      BITCARRPMCNTRL_PM_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGCARR_GET_PMISTATUS:
         /* Get power management interrupt status */
         /*                                       */
         BuildMsg(msg, MSGCARR_READ_PM_CNTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITCARRPMCNTRL_PMI_CC_PMI)
                                  ? (CPARAMETER)PRMCARR_PMISMI
                                  : (CPARAMETER)PRMCARR_PMIIRQ;
         } /* endif */
         break;

      case MSGCARR_SET_PMISTATUS:
         /* Set power management interrupt status */
         /*                                       */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_PMISMI:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_MISC_CTRL,
                                      0,
                                      BITCARRMISCCTRL_CC_PMI_PMI);
               SendMsg(msg, oCarreraIo);

               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                      BITCARRPMCNTRL_PMI_CC_PMI,
                                      BITCARRPMCNTRL_PMI_CTRL |
                                      BITCARRPMCNTRL_PMI_CC_PMI);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_PMIIRQ:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_MISC_CTRL,
                                      BITCARRMISCCTRL_CC_PMI_PMI,
                                      BITCARRMISCCTRL_CC_PMI_PMI);
               SendMsg(msg, oCarreraIo);

               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                      BITCARRPMCNTRL_PMI_CTRL,
                                      BITCARRPMCNTRL_PMI_CTRL |
                                      BITCARRPMCNTRL_PMI_CC_PMI);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_CCSTATUS:
         /* Get clock change status */
         /*                         */
         BuildMsg(msg, MSGCARR_READ_PM_CNTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITCARRPMCNTRL_CC_EN)
                                  ? (CPARAMETER)PRMCARR_CCENABLE
                                  : (CPARAMETER)PRMCARR_CCDISABLE;
         } /* endif */
         break;

      case MSGCARR_SET_CCSTATUS:
         /* Set clock change status */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_CCENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                      BITCARRPMCNTRL_CC_EN,
                                      BITCARRPMCNTRL_CC_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_CCDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                      0,
                                      BITCARRPMCNTRL_CC_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_BLEVENTSTATUS:
         /* Get backlight event status */
         /*                            */
         BuildMsg(msg, MSGCARR_READ_BL_CTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRBLCTRL_BLEVNT_PMI_EN)
                                  ? (CPARAMETER)PRMCARR_BLEVENTENABLE
                                  : (CPARAMETER)PRMCARR_BLEVENTDISABLE;
         } /* endif */
         break;

      case MSGCARR_SET_BLEVENTSTATUS:
         /* Set backlight event status */
         /*                            */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_BLEVENTENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      BITCARRBLCTRL_BLEVNT_PMI_EN,
                                      BITCARRBLCTRL_BLEVNT_PMI_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_BLEVENTDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      0,
                                      BITCARRBLCTRL_BLEVNT_PMI_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_BLEVENTKMSTATUS:
         /* Get backlight keyboard/mouse event status */
         /*                                           */
         BuildMsg(msg, MSGCARR_READ_BL_CTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRBLCTRL_KM_BL_EVNT_EN)
                                  ? (CPARAMETER)PRMCARR_BLEVENTKMENABLE
                                  : (CPARAMETER)PRMCARR_BLEVENTKMDISABLE;
         } /* endif */
         break;

      case MSGCARR_SET_BLEVENTKMSTATUS:
         /* Set backlight keyboard/mouse event status */
         /*                                           */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_BLEVENTKMENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      BITCARRBLCTRL_KM_BL_EVNT_EN,
                                      BITCARRBLCTRL_KM_BL_EVNT_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_BLEVENTKMDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      0,
                                      BITCARRBLCTRL_KM_BL_EVNT_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_BLEVENTSERSTATUS:
         /* Get backlight serial event status */
         /*                                   */
         BuildMsg(msg, MSGCARR_READ_BL_CTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRBLCTRL_SERIAL_BL_EVNT_EN)
                                  ? (CPARAMETER)PRMCARR_BLEVENTSERENABLE
                                  : (CPARAMETER)PRMCARR_BLEVENTSERDISABLE;
         } /* endif */
         break;

      case MSGCARR_SET_BLEVENTSERSTATUS:
         /* Set backlight serial event status */
         /*                                   */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_BLEVENTSERENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      BITCARRBLCTRL_SERIAL_BL_EVNT_EN,
                                      BITCARRBLCTRL_SERIAL_BL_EVNT_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_BLEVENTSERDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      0,
                                      BITCARRBLCTRL_SERIAL_BL_EVNT_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_BLTIMERSTATUS:
         /* Get backlight timer status */
         /*                            */
         BuildMsg(msg, MSGCARR_READ_BL_CTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRBLCTRL_BLTMR_PMI_EN)
                                  ? (CPARAMETER)PRMCARR_BLTIMERENABLE
                                  : (CPARAMETER)PRMCARR_BLTIMERDISABLE;
         } /* endif */
         break;

      case MSGCARR_SET_BLTIMERSTATUS:
         /* Set backlight timer status */
         /*                            */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_BLTIMERENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      BITCARRBLCTRL_BLTMR_PMI_EN,
                                      BITCARRBLCTRL_BLTMR_PMI_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_BLTIMERDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_BL_CTRL,
                                      0,
                                      BITCARRBLCTRL_BLTMR_PMI_EN);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_BLTIMER:
         /* Get backlight idle timer */
         /*                          */
         BuildMsg(msg, MSGCARR_READ_BL_TMR);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         } /* endif */
         break;

      case MSGCARR_SET_BLTIMER:
         /* Set backlight idle timer */
         /*                          */
         BuildMsgWithParm1(msg, MSGCARR_WRITE_BL_TMR,
                                SelectParm1(*pmMsg) );
         SendMsg(msg, oCarreraIo);

         rc = SelectResult(msg);
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGCARR_GET_RISTATUS:
         /* Get ring indicator resume status */
         /*                                  */
         BuildMsg(msg, MSGCARR_READ_RESUME_MASK);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRRESUMEMASK_RING_MSK)
                                  ? (CPARAMETER)PRMCARR_RIDISABLE
                                  : (CPARAMETER)PRMCARR_RIENABLE;
         } /* endif */
         break;

      case MSGCARR_SET_RISTATUS:
         /* Set ring indicator resume status */
         /*                                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_RIENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_RESUME_MASK,
                                      0,
                                      BITCARRRESUMEMASK_RING_MSK);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_RIDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_RESUME_MASK,
                                      BITCARRRESUMEMASK_RING_MSK,
                                      BITCARRRESUMEMASK_RING_MSK);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_ALARMSTATUS:
         /* Get alarm resume status */
         /*                         */
         BuildMsg(msg, MSGCARR_READ_RESUME_MASK);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRRESUMEMASK_ALARM_MSK)
                                  ? (CPARAMETER)PRMCARR_ALARMDISABLE
                                  : (CPARAMETER)PRMCARR_ALARMENABLE;
         } /* endif */
         break;

      case MSGCARR_SET_ALARMSTATUS:
         /* Set alarm resume status */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_ALARMENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_RESUME_MASK,
                                      0,
                                      BITCARRRESUMEMASK_ALARM_MSK);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_ALARMDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_RESUME_MASK,
                                      BITCARRRESUMEMASK_ALARM_MSK,
                                      BITCARRRESUMEMASK_ALARM_MSK);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_H8RSMSTATUS:
         /* Get H8 resume status */
         /*                      */
         BuildMsg(msg, MSGCARR_READ_RESUME_MASK);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRRESUMEMASK_H8_RSM_MSK)
                                  ? (CPARAMETER)PRMCARR_H8RSMDISABLE
                                  : (CPARAMETER)PRMCARR_H8RSMENABLE;
         } /* endif */
         break;

      case MSGCARR_SET_H8RSMSTATUS:
         /* Set H8 resume status */
         /*                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_H8RSMENABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_RESUME_MASK,
                                      0,
                                      BITCARRRESUMEMASK_H8_RSM_MSK);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_H8RSMDISABLE:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_RESUME_MASK,
                                      BITCARRRESUMEMASK_H8_RSM_MSK,
                                      BITCARRRESUMEMASK_H8_RSM_MSK);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGCARR_GET_CPUCLOCKRATE:
         /* Get CPU clock rate */
         /*                    */
         BuildMsg(msg, MSGCARR_READ_CPUCLK_CNTRL);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg) & BITCARRCPUCLKCNTRL_PLL_CFG;
         } /* endif */
         break;

      case MSGCARR_SET_CPUCLOCKRATE:
         /* Set CPU clock rate */
         /*                    */
         BuildMsgWithParm2(msg, MSGCARR_MODIFY_CPUCLK_CNTRL,
                                SelectParm1(*pmMsg),
                                BITCARRCPUCLKCNTRL_PLL_CFG);
         SendMsg(msg, oCarreraIo);

         rc = SelectResult(msg);
         break;

      case MSGCARR_SET_SUSPENDRESUMETIMING:
         /* Set suspend/resume timings */
         /*                            */
         BuildMsgWithParm2(msg, MSGCARR_MODIFY_SRPT,
                                BITCARRSRPT_HS_8RTCCLK |
                                BITCARRSRPT_SH_400MSEC |
                                BITCARRSRPT_RP_8MSEC,
                                BITCARRSRPT_HS         |
                                BITCARRSRPT_SH         |
                                BITCARRSRPT_RP          );
         SendMsg(msg, oCarreraIo);

         rc = SelectResult(msg);
         break;

      case MSGCARR_SET_CPUCLOCKCHANGETIMING:
         /* Set CPU clock change timings */
         /*                              */
         BuildMsgWithParm2(msg, MSGCARR_MODIFY_CCCPTR,
                                BITCARRCCCPTR_QL_12SYSCLK |
                                BITCARRCCCPTR_LI_100USEC,
                                BITCARRCCCPTR_QL          |
                                BITCARRCCCPTR_LI           );
         SendMsg(msg, oCarreraIo);

         rc = SelectResult(msg);
         break;

      case MSGCARR_QUERY_PMREQUEST:
         /* Query power management request status */
         /*                                       */
         BuildMsg(msg, MSGCARR_READ_PM_REQ_STS);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);

            if(SelectParm1(msg) & BITCARRPMREQSTS_BL_REQ)
            {
               BuildMsg(msg, MSGCARR_READ_BL_STS);
               SendMsg(msg, oCarreraIo);

               if(!(rc = SelectResult(msg)))
               {
                  SelectParm2(*pmMsg) = SelectParm1(msg);
               } /* endif */
            } /* endif */
         } /* endif */
         break;

      case MSGCARR_QUERY_RESUMEREQUEST:
         /* Query resume request status */
         /*                             */
         BuildMsg(msg, MSGCARR_READ_RESUME_STS);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         } /* endif */
         break;

      case MSGCARR_CLEAR_PMI:
         /* Clear power management interrupt */
         /*                                  */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_CLRPMISMI:
               BuildMsgWithParm1(msg, MSGCARR_WRITE_CC_PMI_CLR,
                                      0);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_CLRPMIIRQ:
               BuildMsgWithParm1(msg, MSGCARR_WRITE_PMI_CLR,
                                      0);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGCARR_INHIBIT_PMI:
         /* Inhibit power management interrupt */
         /*                                    */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                0,
                                BITCARRPMCNTRL_SUSP_TMR_EN);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_BL_CTRL,
                                0);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_PM_REQ_STS);
         SendMsg(msg, oCarreraIo);
         BuildMsg(msg, MSGCARR_READ_BL_STS);
         SendMsg(msg, oCarreraIo);
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGCARR_ENTER_SUSPEND:
         /* Enter suspend state */
         /*                     */
         BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                BITCARRPMCNTRL_SUSSTAT,
                                BITCARRPMCNTRL_SUSSTAT);
         SendMsg(msg, oCarreraIo);

         rc = SelectResult(msg);
         break;

      case MSGCARR_EXIT_SUSPEND:
         /* Exit suspend state */
         /*                    */
         BuildMsgWithParm2(msg, MSGCARR_MODIFY_PM_CNTRL,
                                0,
                                BITCARRPMCNTRL_SUSSTAT);
         SendMsg(msg, oCarreraIo);

         rc = SelectResult(msg);
         break;

      case MSGCARR_SET_DEVICEALLOFF:
         /* Set all device power off */
         /*                          */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_WRITE_PMCO0,
                                0);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_PMCO1,
                                0);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_GET_LCDBACKLIGHTOFF:
         /* Get LCD backlight off line status */
         /*                                   */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO0_LCDBACKLIGHTOFF)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_LCDBACKLIGHTOFF:
         /* Set LCD backlight off line status */
         /*                                   */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_LCDBACKLIGHTOFF,
                                      BITCARRPMCO0_LCDBACKLIGHTOFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_LCDBACKLIGHTOFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_LCDOFF:
         /* Get LCD off line status */
         /*                         */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITCARRPMCO0_LCDOFF)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_LCDOFF:
         /* Set LCD off line status */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_LCDOFF,
                                      BITCARRPMCO0_LCDOFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_LCDOFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_LCDENABLE:
         /* Get LCD enable line status */
         /*                            */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITCARRPMCO0_LCDENABLE)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_LCDENABLE:
         /* Set LCD enable line status */
         /*                            */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_LCDENABLE,
                                      BITCARRPMCO0_LCDENABLE);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_LCDENABLE);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_LCDHALFBRIGHTNESS:
         /* Get LCD half brightness line status */
         /*                                     */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO0_LCDHALFBRIGHTNESS)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_LCDHALFBRIGHTNESS:
         /* Set LCD half brightness line status */
         /*                                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_LCDHALFBRIGHTNESS,
                                      BITCARRPMCO0_LCDHALFBRIGHTNESS);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_LCDHALFBRIGHTNESS);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_VIDEOLOWPOWER:
         /* Get video chip low power line status */
         /*                                      */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO0_VIDEOLOWPOWER)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_VIDEOLOWPOWER:
         /* Set video chip low power line status */
         /*                                      */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_VIDEOLOWPOWER,
                                      BITCARRPMCO0_VIDEOLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_VIDEOLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_CAMERAOFF:
         /* Get camera off line status */
         /*                            */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITCARRPMCO0_CAMERAOFF)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_CAMERAOFF:
         /* Set camera off line status */
         /*                            */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_CAMERAOFF,
                                      BITCARRPMCO0_CAMERAOFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_CAMERAOFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_VIDEODIGITIZEROFF:
         /* Get video digitizer off line status */
         /*                                     */
         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO0_VIDEODIGITIZEROFF)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_VIDEODIGITIZEROFF:
         /* Set video digitizer off line status */
         /*                                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      BITCARRPMCO0_VIDEODIGITIZEROFF,
                                      BITCARRPMCO0_VIDEODIGITIZEROFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO0,
                                      0,
                                      BITCARRPMCO0_VIDEODIGITIZEROFF);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_SCSICLOCKSTOP:
         /* Get SCSI clock stop line status */
         /*                                 */
         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO1_SCSICLOCKSTOP)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_SCSICLOCKSTOP:
         /* Set SCSI clock stop line status */
         /*                                 */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_SCSICLOCKSTOP,
                                      BITCARRPMCO1_SCSICLOCKSTOP);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      0,
                                      BITCARRPMCO1_SCSICLOCKSTOP);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_CDROMDRIVELOWPOWER:
         /* Get CD-ROM drive low power line status */
         /*                                        */
         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO1_CDROMDRIVELOWPOWER)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_CDROMDRIVELOWPOWER:
         /* Set CD-ROM drive low power line status */
         /*                                        */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_CDROMDRIVELOWPOWER,
                                      BITCARRPMCO1_CDROMDRIVELOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      0,
                                      BITCARRPMCO1_CDROMDRIVELOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_AUDIOLOWPOWER:
         /* Get audio low power line status */
         /*                                 */
         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO1_AUDIOLOWPOWER)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_AUDIOLOWPOWER:
         /* Set audio low power line status */
         /*                                 */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_AUDIOLOWPOWER,
                                      BITCARRPMCO1_AUDIOLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      0,
                                      BITCARRPMCO1_AUDIOLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_L2CACHELOWPOWER:
         /* Get L2 cache low power line status */
         /*                                    */
         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCARRPMCO1_L2CACHELOWPOWER)
                                  ? (CPARAMETER)PRMCARR_LINEHIGH
                                  : (CPARAMETER)PRMCARR_LINELOW;
         } /* endif */
         break;

      case MSGCARR_SET_L2CACHELOWPOWER:
         /* Set L2 cache low power line status */
         /*                                    */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_LINEHIGH:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_L2CACHELOWPOWER,
                                      BITCARRPMCO1_L2CACHELOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_LINELOW:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      0,
                                      BITCARRPMCO1_L2CACHELOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_SCSITERMLOWPOWER:
         /* Get SCSI terminator low power line status */
         /*                                           */
         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            switch(SelectParm1(msg) & BITCARRPMCO1_SCSITERMLOWPOWER)
            {
               case BITCARRPMCO1_SCSITERMON110:
                  SelectParm1(*pmMsg) = PRMCARR_TERMINATORON;
                  break;
               case BITCARRPMCO1_SCSITERMON25K:
                  SelectParm1(*pmMsg) = PRMCARR_TERMINATORSTANDBY;
                  break;
               case BITCARRPMCO1_SCSITERM64:
                  SelectParm1(*pmMsg) = PRMCARR_TERMINATOR3;
                  break;
               case BITCARRPMCO1_SCSITERMDISC:
                  SelectParm1(*pmMsg) = PRMCARR_TERMINATOROFF;
                  break;
               default:
                  SelectParm1(*pmMsg) = 0;
            } /* endswitch */
         } /* endif */
         break;

      case MSGCARR_SET_SCSITERMLOWPOWER:
         /* Set SCSI terminator low power line status */
         /*                                           */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_TERMINATORON:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_SCSITERMON110,
                                      BITCARRPMCO1_SCSITERMLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_TERMINATORSTANDBY:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_SCSITERMON25K,
                                      BITCARRPMCO1_SCSITERMLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_TERMINATOR3:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_SCSITERM64,
                                      BITCARRPMCO1_SCSITERMLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_TERMINATOROFF:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_SCSITERMDISC,
                                      BITCARRPMCO1_SCSITERMLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCARR_GET_CRTLOWPOWER:
         /* Get CRT low power line status */
         /*                               */
         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            switch(SelectParm1(msg) & BITCARRPMCO1_CRTLOWPOWER)
            {
               case BITCARRPMCO1_CRTON:
                  SelectParm1(*pmMsg) = PRMCARR_CRTPOWERON;
                  break;
               case BITCARRPMCO1_CRTSTANDBY:
                  SelectParm1(*pmMsg) = PRMCARR_CRTPOWERSTANDBY;
                  break;
               case BITCARRPMCO1_CRTSUSPEND:
                  SelectParm1(*pmMsg) = PRMCARR_CRTPOWERSUSPEND;
                  break;
               case BITCARRPMCO1_CRTOFF:
                  SelectParm1(*pmMsg) = PRMCARR_CRTPOWEROFF;
                  break;
               default:
                  SelectParm1(*pmMsg) = 0;
            } /* endswitch */
         } /* endif */
         break;

      case MSGCARR_SET_CRTLOWPOWER:
         /* Set CRT low power line status */
         /*                               */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCARR_CRTPOWERON:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_CRTON,
                                      BITCARRPMCO1_CRTLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_CRTPOWERSTANDBY:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_CRTSTANDBY,
                                      BITCARRPMCO1_CRTLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_CRTPOWERSUSPEND:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_CRTSUSPEND,
                                      BITCARRPMCO1_CRTLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            case PRMCARR_CRTPOWEROFF:
               BuildMsgWithParm2(msg, MSGCARR_MODIFY_PMCO1,
                                      BITCARRPMCO1_CRTOFF,
                                      BITCARRPMCO1_CRTLOWPOWER);
               SendMsg(msg, oCarreraIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCARR_INVALID_PARAMETER;
         } /* endswitch */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGCARR_GET_PIC1REGISTERS:
         /* Get programmable interrupt controller registers (Master) */
         /*                                                          */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC1ICW1);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC1ICW2);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC1ICW3);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC1ICW4);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC1OCW2);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC1OCW3);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PICSEQUENCE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm7(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_GET_PIC2REGISTERS:
         /* Get programmable interrupt controller registers (Slave) */
         /*                                                         */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC2ICW1);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC2ICW2);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC2ICW3);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC2ICW4);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC2OCW2);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PIC2OCW3);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_PICSEQUENCE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm7(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_GET_PIT1REGISTERS:
         /* Get programmable interval timer registers */
         /*                                           */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_TIMERCOUNTER0L);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_TIMERCOUNTER0H);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_TIMERCOUNTER2L);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_TIMERCOUNTER2H);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_TIMERBYTEPOINTER);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_GET_DMA1REGISTERS:
         /* Get direct memory access controller registers (Master) */
         /*                                                        */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA0BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA0BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = (SelectParm1(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA1BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA1BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = (SelectParm2(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA2BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA2BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = (SelectParm3(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA3BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA3BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = (SelectParm4(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA0COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA0COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = (SelectParm5(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA1COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA1COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = (SelectParm6(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA2COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm7(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA2COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm7(*pmMsg) = (SelectParm7(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA3COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm8(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA3COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm8(*pmMsg) = (SelectParm8(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA0MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm9(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA1MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm10(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA2MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm11(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA3MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm12(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA0EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm13(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA1EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm14(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA2EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm15(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA3EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm16(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMACOMMAND_0_3);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm17(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMABYTEPOINTER);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm18(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_GET_DMA2REGISTERS:
         /* Get direct memory access controller registers (Slave) */
         /*                                                       */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA4BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA4BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm1(*pmMsg) = (SelectParm1(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA5BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA5BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm2(*pmMsg) = (SelectParm2(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA6BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA6BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm3(*pmMsg) = (SelectParm3(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA7BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA7BASEADDR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm4(*pmMsg) = (SelectParm4(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA4COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA4COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm5(*pmMsg) = (SelectParm5(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA5COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA5COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm6(*pmMsg) = (SelectParm6(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA6COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm7(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA6COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm7(*pmMsg) = (SelectParm7(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA7COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm8(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA7COUNT);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm8(*pmMsg) = (SelectParm8(*pmMsg) & 0x00FF) |
                                  (SelectParm1(msg)   << 8     ) ;
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA4MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm9(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA5MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm10(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA6MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm11(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA7MODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm12(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA4EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm13(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA5EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm14(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA6EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm15(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMA7EXTMODE);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm16(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMACOMMAND_4_7);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm17(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_DMABYTEPOINTER);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            SelectParm18(*pmMsg) = SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_GET_SERIAL1REGISTERS:
         /* Get serial port A register (FIFO control register) */
         /*                                                    */
         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_SERIAL1FCR);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         } /* endif */
         break;

      case MSGCARR_GET_SERIAL2REGISTERS:
         /* Get serial port B register (FIFO control register) */
         /*                                                    */
         BuildMsgWithParm1(msg, MSGCARR_READ_DATA,
                                INDEXCARR_SERIAL2FCR);
         SendMsg(msg, oCarreraIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         } /* endif */
         break;

      case MSGCARR_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRCARR_NOERROR;

         BuildMsg(msg, MSGCARR_GET_INDEX);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chIndex = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_PM_CNTRL);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chPmCntrl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_BL_CTRL);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chBlCtrl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_BL_TMR);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chBlTmr = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_RESUME_MASK);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chResumeMask = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_PMCO0);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chPmco0 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_PMCO1);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chPmco1 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_PMIN_CTRL);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chPminCtrl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_MISC_CTRL);
         SendMsg(msg, oCarreraIo);
         if(!SelectResult(msg))
         {
            pcarrctl->chMiscCtrl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCARR_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRCARR_NOERROR;

         BuildMsgWithParm1(msg, MSGCARR_WRITE_PM_CNTRL,
                                pcarrctl->chPmCntrl);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_BL_CTRL,
                                pcarrctl->chBlCtrl);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_BL_TMR,
                                pcarrctl->chBlTmr);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_RESUME_MASK,
                                pcarrctl->chResumeMask);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_PMCO0,
                                pcarrctl->chPmco0 &
                                ~(BITCARRPMCO0_LCDBACKLIGHTOFF |
                                  BITCARRPMCO0_LCDOFF          |
                                  BITCARRPMCO0_LCDENABLE       ));
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_PMCO1,
                                pcarrctl->chPmco1);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_PMIN_CTRL,
                                pcarrctl->chPminCtrl);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_WRITE_MISC_CTRL,
                                pcarrctl->chMiscCtrl);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCARR_SET_INDEX,
                                pcarrctl->chIndex);
         SendMsg(msg, oCarreraIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCARR_READ_PM_REQ_STS);
         SendMsg(msg, oCarreraIo);
         BuildMsg(msg, MSGCARR_READ_BL_STS);
         SendMsg(msg, oCarreraIo);

         BuildMsgWithParm1(msg, MSGCARR_WRITE_CC_PMI_CLR,
                                0);
         SendMsg(msg, oCarreraIo);
         BuildMsgWithParm1(msg, MSGCARR_WRITE_PMI_CLR,
                                0);
         SendMsg(msg, oCarreraIo);
         break;

      default:
         SendMsg(*pmMsg, oCarreraIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
