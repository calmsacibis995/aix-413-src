/* @(#)42  1.2  src/rspc/kernext/pm/pmmd/6030/pdcarera.h, pwrsysx, rspc41J, 9517A_b 4/25/95 04:13:51
 *
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Carrera (Power Management Controller) Chip Device Control
 *              Definitions
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
/* * Module Name: PDCARERA.H                                               * */
/* * Description: Carrera (Power Management Controller) Chip Device Control* */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDCARERA
#define  _H_PDCARERA

#define  PMFUNCTIONS_FULL

/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *    Carrera Message Definition     * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                     /* Carrera message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[17];                      /* Extended parameters */
} MSGCARRERA;
typedef MSGCARRERA   *PMSGCARRERA;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Carrera I/O class */
#define  MSGCARR_INITIALIZE               (0x0000)
                                                               /* Initialize */
#define  MSGCARR_GET_INDEX                (0x0001)
                                                       /* Get index register */
#define  MSGCARR_SET_INDEX                (0x0002)
                                                       /* Set index register */
#define  MSGCARR_GET_DATA                 (0x0003)
                                                        /* Get data register */
#define  MSGCARR_SET_DATA                 (0x0004)
                                                        /* Set data register */
#define  MSGCARR_GET_REVISION             (0x0005)
                                                          /* Get revision ID */
#define  MSGCARR_READ_DATA                (0x0006)
                                                   /* Read any register data */
#define  MSGCARR_READ_PM_CNTRL            (0x0007)
                                   /* Read power management control register */
#define  MSGCARR_READ_PM_REQ_STS          (0x0008)
                            /* Read power management request status register */
#define  MSGCARR_READ_CPUCLK_CNTRL        (0x0009)
                                          /* Read CPU clock control register */
#define  MSGCARR_READ_BL_STS              (0x000A)
                                           /* Read backlight status register */
#define  MSGCARR_READ_BL_CTRL             (0x000B)
                                          /* Read backlight control register */
#define  MSGCARR_READ_BL_TMR              (0x000C)
                                       /* Read backlight idle timer register */
#define  MSGCARR_READ_SUSPEND_TMRL        (0x000D)
                                     /* Read suspend idle timer low register */
#define  MSGCARR_READ_SUSPEND_TMRH        (0x000E)
                                    /* Read suspend idle timer high register */
#define  MSGCARR_READ_STI_ACT_MSK         (0x000F)
                           /* Read suspend timer idle activity mask register */
#define  MSGCARR_READ_STI_ACT_STS         (0x0010)
                         /* Read suspend timer idle activity status register */
#define  MSGCARR_READ_RESUME_MASK         (0x0011)
                                                /* Read resume mask register */
#define  MSGCARR_READ_RESUME_STS          (0x0012)
                                              /* Read resume status register */
#define  MSGCARR_READ_PMCO0               (0x0013)
                          /* Read power management control output 0 register */
#define  MSGCARR_READ_PMCO1               (0x0014)
                          /* Read power management control output 1 register */
#define  MSGCARR_READ_PMIN_CTRL           (0x0015)
                                               /* Read PMIN control register */
#define  MSGCARR_READ_MISC_CTRL           (0x0016)
                                              /* Read misc. control register */
#define  MSGCARR_READ_SPAS                (0x0017)
                                 /* Read serial port address status register */
#define  MSGCARR_READ_SRPT                (0x0018)
                        /* Read suspend/resume programmable timings register */
#define  MSGCARR_READ_CCCPTR              (0x0019)
                      /* Read CPU clock change programmable timings register */
#define  MSGCARR_WRITE_DATA               (0x001A)
                                                  /* Write any register data */
#define  MSGCARR_WRITE_PM_CNTRL           (0x001B)
                                  /* Write power management control register */
#define  MSGCARR_WRITE_CPUCLK_CNTRL       (0x001C)
                                         /* Write CPU clock control register */
#define  MSGCARR_WRITE_BL_CTRL            (0x001D)
                                         /* Write backlight control register */
#define  MSGCARR_WRITE_BL_TMR             (0x001E)
                                      /* Write backlight idle timer register */
#define  MSGCARR_WRITE_SUSPEND_TMRL       (0x001F)
                                    /* Write suspend idle timer low register */
#define  MSGCARR_WRITE_SUSPEND_TMRH       (0x0020)
                                   /* Write suspend idle timer high register */
#define  MSGCARR_WRITE_STI_ACT_MSK        (0x0021)
                          /* Write suspend timer idle activity mask register */
#define  MSGCARR_WRITE_STI_ACT_STS        (0x0022)
                        /* Write suspend timer idle activity status register */
#define  MSGCARR_WRITE_RESUME_MASK        (0x0023)
                                               /* Write resume mask register */
#define  MSGCARR_WRITE_PMCO0              (0x0024)
                         /* Write power management control output 0 register */
#define  MSGCARR_WRITE_PMCO1              (0x0025)
                         /* Write power management control output 1 register */
#define  MSGCARR_WRITE_PMIN_CTRL          (0x0026)
                                              /* Write PMIN control register */
#define  MSGCARR_WRITE_CC_PMI_CLR         (0x0027)
                                              /* Write CC_PMI clear register */
#define  MSGCARR_WRITE_PMI_CLR            (0x0028)
                                                 /* Write PMI clear register */
#define  MSGCARR_WRITE_MISC_CTRL          (0x0029)
                                             /* Write misc. control register */
#define  MSGCARR_WRITE_SRPT               (0x002A)
                       /* Write suspend/resume programmable timings register */
#define  MSGCARR_WRITE_CCCPTR             (0x002B)
                     /* Write CPU clock change programmable timings register */
#define  MSGCARR_MODIFY_DATA              (0x002C)
                                            /* Read/Modify any register data */
#define  MSGCARR_MODIFY_PM_CNTRL          (0x002D)
                            /* Read/Modify power management control register */
#define  MSGCARR_MODIFY_CPUCLK_CNTRL      (0x002E)
                                   /* Read/Modify CPU clock control register */
#define  MSGCARR_MODIFY_BL_CTRL           (0x002F)
                                   /* Read/Modify backlight control register */
#define  MSGCARR_MODIFY_STI_ACT_MSK       (0x0030)
                    /* Read/Modify suspend timer idle activity mask register */
#define  MSGCARR_MODIFY_RESUME_MASK       (0x0031)
                                         /* Read/Modify resume mask register */
#define  MSGCARR_MODIFY_PMCO0             (0x0032)
                   /* Read/Modify power management control output 0 register */
#define  MSGCARR_MODIFY_PMCO1             (0x0033)
                   /* Read/Modify power management control output 1 register */
#define  MSGCARR_MODIFY_PMIN_CTRL         (0x0034)
                                        /* Read/Modify PMIN control register */
#define  MSGCARR_MODIFY_MISC_CTRL         (0x0035)
                                       /* Read/Modify misc. control register */
#define  MSGCARR_MODIFY_SRPT              (0x0036)
                 /* Read/Modify suspend/resume programmable timings register */
#define  MSGCARR_MODIFY_CCCPTR            (0x0037)
               /* Read/Modify CPU clock change programmable timings register */

/* Carrera control class */
#define  MSGCARR_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#define  MSGCARR_GET_PMSTATUS             (0x0101)
                                              /* Get power management status */
#define  MSGCARR_SET_PMSTATUS             (0x0102)
                                              /* Set power management status */
#define     PRMCARR_PMENABLE              (0x0001)              /* PM enable */
#define     PRMCARR_PMDISABLE             (0x0002)             /* PM disable */
#ifdef PMFUNCTIONS_FULL
#define  MSGCARR_GET_PMISTATUS            (0x0103)
                                    /* Get power management interrupt status */
#define  MSGCARR_SET_PMISTATUS            (0x0104)
                                    /* Set power management interrupt status */
#define     PRMCARR_PMISMI                (0x0001)       /* PM interrupt SMI */
#define     PRMCARR_PMIIRQ                (0x0002)       /* PM interrupt IRQ */
#define  MSGCARR_GET_CCSTATUS             (0x0105)
                                                  /* Get clock change status */
#define  MSGCARR_SET_CCSTATUS             (0x0106)
                                                  /* Set clock change status */
#define     PRMCARR_CCENABLE              (0x0001)    /* Clock change enable */
#define     PRMCARR_CCDISABLE             (0x0002)   /* Clock change disable */
#define  MSGCARR_GET_BLEVENTSTATUS        (0x0107)
                                               /* Get backlight event status */
#define  MSGCARR_SET_BLEVENTSTATUS        (0x0108)
                                               /* Set backlight event status */
#define     PRMCARR_BLEVENTENABLE         (0x0001) /* Backlight event enable */
#define     PRMCARR_BLEVENTDISABLE        (0x0002)/* Backlight event disable */
#define  MSGCARR_GET_BLEVENTKMSTATUS      (0x0109)
                                /* Get backlight keyboard/mouse event status */
#define  MSGCARR_SET_BLEVENTKMSTATUS      (0x010A)
                                /* Set backlight keyboard/mouse event status */
#define     PRMCARR_BLEVENTKMENABLE       (0x0001)
                                    /* Backlight keyboard/mouse event enable */
#define     PRMCARR_BLEVENTKMDISABLE      (0x0002)
                                   /* Backlight keyboard/mouse event disable */
#define  MSGCARR_GET_BLEVENTSERSTATUS     (0x010B)
                                        /* Get backlight serial event status */
#define  MSGCARR_SET_BLEVENTSERSTATUS     (0x010C)
                                        /* Set backlight serial event status */
#define     PRMCARR_BLEVENTSERENABLE      (0x0001)
                                            /* Backlight serial event enable */
#define     PRMCARR_BLEVENTSERDISABLE     (0x0002)
                                           /* Backlight serial event disable */
#define  MSGCARR_GET_BLTIMERSTATUS        (0x010D)
                                               /* Get backlight timer status */
#define  MSGCARR_SET_BLTIMERSTATUS        (0x010E)
                                               /* Set backlight timer status */
#define     PRMCARR_BLTIMERENABLE         (0x0001) /* Backlight timer enable */
#define     PRMCARR_BLTIMERDISABLE        (0x0002)/* Backlight timer disable */
#define  MSGCARR_GET_BLTIMER              (0x010F)
                                                 /* Get backlight idle timer */
#define  MSGCARR_SET_BLTIMER              (0x0110)
                                                 /* Set backlight idle timer */
#define     PRMCARR_BLTIMERRESOLUTION     (0x0005)
                         /* Backlight idle timer resolution (5 seconds unit) */
#endif /* PMFUNCTIONS_FULL */
#define  MSGCARR_GET_RISTATUS             (0x0111)
                                         /* Get ring indicator resume status */
#define  MSGCARR_SET_RISTATUS             (0x0112)
                                         /* Set ring indicator resume status */
#define     PRMCARR_RIENABLE              (0x0001)              /* RI enable */
#define     PRMCARR_RIDISABLE             (0x0002)             /* RI disable */
#define  MSGCARR_GET_ALARMSTATUS          (0x0113)
                                                  /* Get alarm resume status */
#define  MSGCARR_SET_ALARMSTATUS          (0x0114)
                                                  /* Set alarm resume status */
#define     PRMCARR_ALARMENABLE           (0x0001)           /* Alarm enable */
#define     PRMCARR_ALARMDISABLE          (0x0002)          /* Alarm disable */
#define  MSGCARR_GET_H8RSMSTATUS          (0x0115)
                                                     /* Get H8 resume status */
#define  MSGCARR_SET_H8RSMSTATUS          (0x0116)
                                                     /* Set H8 resume status */
#define     PRMCARR_H8RSMENABLE           (0x0001)       /* H8 resume enable */
#define     PRMCARR_H8RSMDISABLE          (0x0002)      /* H8 resume disable */
#ifdef PMFUNCTIONS_FULL
#define  MSGCARR_GET_CPUCLOCKRATE         (0x0117)
                                                       /* Get CPU clock rate */
#define  MSGCARR_SET_CPUCLOCKRATE         (0x0118)
                                                       /* Set CPU clock rate */
#define  MSGCARR_SET_SUSPENDRESUMETIMING  (0x0119)
                                               /* Set suspend/resume timings */
#define  MSGCARR_SET_CPUCLOCKCHANGETIMING (0x011A)
                                             /* Set CPU clock change timings */
#define  MSGCARR_QUERY_PMREQUEST          (0x011B)
                                    /* Query power management request status */
#define     PRMCARR_PMREQBIT_CLKCHG       (0x0001)
                                                    /* Clock changed (Bit-0) */
#define     PRMCARR_PMREQBIT_IOACCESS     (0x0002)
                                              /* I/O access detected (Bit-1) */
#define     PRMCARR_PMREQBIT_GPTMREXP     (0x0004)
                                                 /* GP timer expired (Bit-2) */
#define     PRMCARR_PMREQBIT_EXTPMI       (0x0008)
                                     /* External PMI suspend request (Bit-3) */
#define     PRMCARR_PMREQBIT_SUSPTMREXP   (0x0010)
                                            /* Suspend timer expired (Bit-4) */
#define     PRMCARR_PMREQBIT_BLREQ        (0x0020)
                                                /* Backlight request (Bit-5) */
#define     PRMCARR_PMREQBIT_PMIN         (0x0040)
                                                     /* PMIN request (Bit-6) */
#define     PRMCARR_BLREQBIT_BLTMRREQ     (0x0001)
                                 /* Backlight idle timer PMI request (Bit-0) */
#define     PRMCARR_BLREQBIT_BLEVNTREQ    (0x0002)
                                      /* Backlight event PMI request (Bit-1) */
#define  MSGCARR_QUERY_RESUMEREQUEST      (0x011C)
                                              /* Query resume request status */
#define     PRMCARR_RSMREQBIT_RING        (0x0001)
                                      /* Ring event status indicator (Bit-0) */
#define     PRMCARR_RSMREQBIT_ALARM       (0x0002)
                                                  /* Alarm indicator (Bit-1) */
#define     PRMCARR_RSMREQBIT_H8RSM       (0x0004)
                                                 /* H8 resume status (Bit-2) */
#define  MSGCARR_CLEAR_PMI                (0x011D)
                                         /* Clear power management interrupt */
#define     PRMCARR_CLRPMISMI             (0x0001)       /* PM interrupt SMI */
#define     PRMCARR_CLRPMIIRQ             (0x0002)       /* PM interrupt IRQ */
#endif /* PMFUNCTIONS_FULL */
#define  MSGCARR_INHIBIT_PMI              (0x011E)
                                       /* Inhibit power management interrupt */
#ifdef PMFUNCTIONS_FULL
#define  MSGCARR_ENTER_SUSPEND            (0x011F)
                                                      /* Enter suspend state */
#define  MSGCARR_EXIT_SUSPEND             (0x0120)
                                                       /* Exit suspend state */
#define  MSGCARR_SET_DEVICEALLOFF         (0x0121)
                                                 /* Set all device power off */
#define  MSGCARR_GET_LCDBACKLIGHTOFF      (0x0122)
                                        /* Get LCD backlight off line status */
#define  MSGCARR_SET_LCDBACKLIGHTOFF      (0x0123)
                                        /* Set LCD backlight off line status */
#define  MSGCARR_GET_LCDOFF               (0x0124)
                                                  /* Get LCD off line status */
#define  MSGCARR_SET_LCDOFF               (0x0125)
                                                  /* Set LCD off line status */
#define  MSGCARR_GET_LCDENABLE            (0x0126)
                                               /* Get LCD enable line status */
#define  MSGCARR_SET_LCDENABLE            (0x0127)
                                               /* Set LCD enable line status */
#define  MSGCARR_GET_LCDHALFBRIGHTNESS    (0x0128)
                                      /* Get LCD half brightness line status */
#define  MSGCARR_SET_LCDHALFBRIGHTNESS    (0x0129)
                                      /* Set LCD half brightness line status */
#define  MSGCARR_GET_VIDEOLOWPOWER        (0x012A)
                                     /* Get video chip low power line status */
#define  MSGCARR_SET_VIDEOLOWPOWER        (0x012B)
                                     /* Set video chip low power line status */
#define  MSGCARR_GET_CAMERAOFF            (0x012C)
                                               /* Get camera off line status */
#define  MSGCARR_SET_CAMERAOFF            (0x012D)
                                               /* Set camera off line status */
#define  MSGCARR_GET_VIDEODIGITIZEROFF    (0x012E)
                                      /* Get video digitizer off line status */
#define  MSGCARR_SET_VIDEODIGITIZEROFF    (0x012F)
                                      /* Set video digitizer off line status */
#define  MSGCARR_GET_SCSICLOCKSTOP        (0x0130)
                                          /* Get SCSI clock stop line status */
#define  MSGCARR_SET_SCSICLOCKSTOP        (0x0131)
                                          /* Set SCSI clock stop line status */
#define  MSGCARR_GET_CDROMDRIVELOWPOWER   (0x0132)
                                   /* Get CD-ROM drive low power line status */
#define  MSGCARR_SET_CDROMDRIVELOWPOWER   (0x0133)
                                   /* Set CD-ROM drive low power line status */
#define  MSGCARR_GET_AUDIOLOWPOWER        (0x0134)
                                          /* Get audio low power line status */
#define  MSGCARR_SET_AUDIOLOWPOWER        (0x0135)
                                          /* Set audio low power line status */
#define  MSGCARR_GET_L2CACHELOWPOWER      (0x0136)
                                       /* Get L2 cache low power line status */
#define  MSGCARR_SET_L2CACHELOWPOWER      (0x0137)
                                       /* Set L2 cache low power line status */
#define     PRMCARR_LINEHIGH              (0x0001)       /* Line high status */
#define     PRMCARR_LINELOW               (0x0002)        /* Line low status */
#define  MSGCARR_GET_SCSITERMLOWPOWER     (0x0138)
                                /* Get SCSI terminator low power line status */
#define  MSGCARR_SET_SCSITERMLOWPOWER     (0x0139)
                                /* Set SCSI terminator low power line status */
#define     PRMCARR_TERMINATORON          (0x0001)   /* Terminator on status */
#define     PRMCARR_TERMINATORSTANDBY     (0x0002)
                                               /* Terminator stand-by status */
#define     PRMCARR_TERMINATOR3           (0x0003)  /* Terminator ??? status */
#define     PRMCARR_TERMINATOROFF         (0x0004)  /* Terminator off status */
#define  MSGCARR_GET_CRTLOWPOWER          (0x013A)
                                            /* Get CRT low power line status */
#define  MSGCARR_SET_CRTLOWPOWER          (0x013B)
                                            /* Set CRT low power line status */
#define     PRMCARR_CRTPOWERON            (0x0001)    /* CRT power on status */
#define     PRMCARR_CRTPOWERSTANDBY       (0x0002)
                                                /* CRT power stand-by status */
#define     PRMCARR_CRTPOWERSUSPEND       (0x0003)
                                                 /* CRT power suspend status */
#define     PRMCARR_CRTPOWEROFF           (0x0004)   /* CRT power off status */
#endif /* PMFUNCTIONS_FULL */
#define  MSGCARR_GET_PIC1REGISTERS        (0x013C)
                 /* Get programmable interrupt controller registers (Master) */
#define  MSGCARR_GET_PIC2REGISTERS        (0x013D)
                  /* Get programmable interrupt controller registers (Slave) */
#define  MSGCARR_GET_PIT1REGISTERS        (0x013E)
                                /* Get programmable interval timer registers */
#define  MSGCARR_GET_DMA1REGISTERS        (0x013F)
                   /* Get direct memory access controller registers (Master) */
#define  MSGCARR_GET_DMA2REGISTERS        (0x0140)
                    /* Get direct memory access controller registers (Slave) */
#define  MSGCARR_GET_SERIAL1REGISTERS     (0x0141)
                       /* Get serial port A register (FIFO control register) */
#define  MSGCARR_GET_SERIAL2REGISTERS     (0x0142)
                       /* Get serial port B register (FIFO control register) */
#define  MSGCARR_SAVE_CONTEXT             (0x0143)
                                                      /* Save device context */
#define  MSGCARR_RESTORE_CONTEXT          (0x0144)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRCARR_NOERROR                  (0x0000) /* No error                */
#define  ERRCARR_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERRCARR_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERRCARR_UNKNOWN                  (0x0003) /* Unknown port setting    */
#define  ERRCARR_NOT_INITIALIZED          (0x0004) /* Not initialized yet     */
#define  ERRCARR_CANNOT_INITIALIZE        (0x0005) /* Can not initialize      */
#define  ERRCARR_CANNOT_PCIACCESS         (0x0006) /* Can not access PCI bus  */

/* ************************************* */
/* *    Carrera Objects Definition     * */
/* ************************************* */
/* ------------------- */
/*  Carrera I/O class  */
/* ------------------- */
typedef struct                                         /* Carrera I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
   WORD        Revision;                               /* Device revision ID */
} OBJCARRERAIO;
typedef OBJCARRERAIO *POBJCARRERAIO;

/* ----------------------- */
/*  Carrera control class  */
/* ----------------------- */
typedef struct                                     /* Carrera control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chIndex;                                    /* Index register */
   BYTE        chPmCntrl;               /* Power management control register */
   BYTE        chBlCtrl;                       /* Backlight control register */
   BYTE        chBlTmr;                     /* Backlight idle timer register */
   BYTE        chResumeMask;                         /* Resume mask register */
   BYTE        chPmco0;        /* Power management control output 0 register */
   BYTE        chPmco1;        /* Power management control output 1 register */
   BYTE        chPminCtrl;                          /* PMIN control register */
   BYTE        chMiscCtrl;                         /* Misc. control register */
} OBJCARRERACTL;
typedef OBJCARRERACTL   *POBJCARRERACTL;

/* ************************************* */
/* *          Carrera Objects          * */
/* ************************************* */
/* ----------------------- */
/*  Carrera control class  */
/* ----------------------- */
extern OBJCARRERACTL oCarreraCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDCARERA */
/* *****************************  End of File  ***************************** */
