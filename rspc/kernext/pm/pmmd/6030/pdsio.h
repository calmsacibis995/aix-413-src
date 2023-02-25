/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: System I/O (82378IB) Chip Device Control Definitions
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
/* * Module Name: PDSIO.H                                                  * */
/* * Description: System I/O (82378IB) Chip Device Control Definitions     * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDSIO
#define  _H_PDSIO


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *   System-I/O Message Definition   * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                  /* System-I/O message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
   DWORD       pdwLongParm[1];                /* Extended parameters (DWORD) */
} MSGSYSTEMIO;
typedef MSGSYSTEMIO  *PMSGSYSTEMIO;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* System-I/O I/O class */
#define  MSGSIO_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGSIO_READ_PCICON               (0x0001)
                                                /* Read PCI control register */
#define  MSGSIO_READ_PAC                  (0x0002)
                                        /* Read PCI arbiter control register */
#define  MSGSIO_READ_PAPC                 (0x0003)
                               /* Read PCI arbiter priority control register */
#define  MSGSIO_READ_MCSCON               (0x0004)
                                             /* Read MEMCS# control register */
#define  MSGSIO_READ_MCSBOH               (0x0005)
                                      /* Read MEMCS# bottom of hole register */
#define  MSGSIO_READ_MCSTOH               (0x0006)
                                         /* Read MEMCS# top of hole register */
#define  MSGSIO_READ_MCSTOM               (0x0007)
                                       /* Read MEMCS# top of memory register */
#define  MSGSIO_READ_IADCON               (0x0008)
                                /* Read ISA address decoder control register */
#define  MSGSIO_READ_IADRBE               (0x0009)
                       /* Read ISA address decoder ROM block enable register */
#define  MSGSIO_READ_IADBOH               (0x000A)
                         /* Read ISA address decoder bottom of hole register */
#define  MSGSIO_READ_IADTOH               (0x000B)
                            /* Read ISA address decoder top of hole register */
#define  MSGSIO_READ_ICRT                 (0x000C)
                              /* Read ISA controller recovery timer register */
#define  MSGSIO_READ_ICD                  (0x000D)
                                          /* Read ISA clock divisor register */
#define  MSGSIO_READ_UBCSA                (0x000E)
                           /* Read utility bus chip select enable A register */
#define  MSGSIO_READ_UBCSB                (0x000F)
                           /* Read utility bus chip select enable B register */
#define  MSGSIO_READ_MAR1                 (0x0010)
                                        /* Read MEMCS# attribute #1 register */
#define  MSGSIO_READ_MAR2                 (0x0011)
                                        /* Read MEMCS# attribute #2 register */
#define  MSGSIO_READ_MAR3                 (0x0012)
                                        /* Read MEMCS# attribute #3 register */
#define  MSGSIO_READ_DMASCATTERBASEADDR   (0x0013)
                 /* Read DMA Scatter/Gather relocation base address register */
#define  MSGSIO_READ_PIRQ0                (0x0014)
                                       /* Read PIRQ0# route control register */
#define  MSGSIO_READ_BIOSTIMERBASEADDR    (0x0015)
                                    /* Read BIOS timer base address register */
#define  MSGSIO_READ_SMICNTL              (0x0016)
                                                /* Read SMI control register */
#define  MSGSIO_READ_SEE                  (0x0017)
                                        /* Read system event enable register */
#define  MSGSIO_READ_FTMR                 (0x0018)
                                             /* Read fast off timer register */
#define  MSGSIO_READ_CTLTMRL              (0x0019)
                           /* Read clock throttle STPCLK# low timer register */
#define  MSGSIO_READ_APM                  (0x001A)
                                    /* Read APM control/status port register */
#define  MSGSIO_READ_ELCR1                (0x001B)
                                       /* Read edge/level control 1 register */
#define  MSGSIO_READ_ELCR2                (0x001C)
                                       /* Read edge/level control 2 register */
#define  MSGSIO_WRITE_PCICON              (0x001D)
                                               /* Write PCI control register */
#define  MSGSIO_WRITE_PAC                 (0x001E)
                                       /* Write PCI arbiter control register */
#define  MSGSIO_WRITE_PAPC                (0x001F)
                              /* Write PCI arbiter priority control register */
#define  MSGSIO_WRITE_MCSCON              (0x0020)
                                            /* Write MEMCS# control register */
#define  MSGSIO_WRITE_MCSBOH              (0x0021)
                                     /* Write MEMCS# bottom of hole register */
#define  MSGSIO_WRITE_MCSTOH              (0x0022)
                                        /* Write MEMCS# top of hole register */
#define  MSGSIO_WRITE_MCSTOM              (0x0023)
                                      /* Write MEMCS# top of memory register */
#define  MSGSIO_WRITE_IADCON              (0x0024)
                               /* Write ISA address decoder control register */
#define  MSGSIO_WRITE_IADRBE              (0x0025)
                      /* Write ISA address decoder ROM block enable register */
#define  MSGSIO_WRITE_IADBOH              (0x0026)
                        /* Write ISA address decoder bottom of hole register */
#define  MSGSIO_WRITE_IADTOH              (0x0027)
                           /* Write ISA address decoder top of hole register */
#define  MSGSIO_WRITE_ICRT                (0x0028)
                             /* Write ISA controller recovery timer register */
#define  MSGSIO_WRITE_ICD                 (0x0029)
                                         /* Write ISA clock divisor register */
#define  MSGSIO_WRITE_UBCSA               (0x002A)
                          /* Write utility bus chip select enable A register */
#define  MSGSIO_WRITE_UBCSB               (0x002B)
                          /* Write utility bus chip select enable B register */
#define  MSGSIO_WRITE_MAR1                (0x002C)
                                       /* Write MEMCS# attribute #1 register */
#define  MSGSIO_WRITE_MAR2                (0x002D)
                                       /* Write MEMCS# attribute #2 register */
#define  MSGSIO_WRITE_MAR3                (0x002E)
                                       /* Write MEMCS# attribute #3 register */
#define  MSGSIO_WRITE_DMASCATTERBASEADDR  (0x002F)
                /* Write DMA Scatter/Gather relocation base address register */
#define  MSGSIO_WRITE_PIRQ0               (0x0030)
                                      /* Write PIRQ0# route control register */
#define  MSGSIO_WRITE_BIOSTIMERBASEADDR   (0x0031)
                                   /* Write BIOS timer base address register */
#define  MSGSIO_WRITE_SMICNTL             (0x0032)
                                               /* Write SMI control register */
#define  MSGSIO_WRITE_SEE                 (0x0033)
                                       /* Write system event enable register */
#define  MSGSIO_WRITE_FTMR                (0x0034)
                                            /* Write fast off timer register */
#define  MSGSIO_WRITE_CTLTMRL             (0x0035)
                          /* Write clock throttle STPCLK# low timer register */
#define  MSGSIO_WRITE_APM                 (0x0036)
                                   /* Write APM control/status port register */
#define  MSGSIO_WRITE_ELCR1               (0x0037)
                                      /* Write edge/level control 1 register */
#define  MSGSIO_WRITE_ELCR2               (0x0038)
                                      /* Write edge/level control 2 register */

/* System-I/O control class */
#define  MSGSIO_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGSIO_SAVE_CONTEXT              (0x0101)
                                                      /* Save device context */
#define  MSGSIO_RESTORE_CONTEXT           (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRSIO_NOERROR                   (0x0000) /* No error                */
#define  ERRSIO_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRSIO_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRSIO_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRSIO_NOT_INITIALIZED           (0x0004) /* Not initialized         */
#define  ERRSIO_CANNOT_INITIALIZE         (0x0005) /* Can not initialize      */
#define  ERRSIO_CANNOT_PCIACCESS          (0x0006) /* Can not access PCI bus  */

/* ************************************* */
/* *   System-I/O Objects Definition   * */
/* ************************************* */
/* ---------------------- */
/*  System-I/O I/O class  */
/* ---------------------- */
typedef struct                                      /* System-I/O I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJSYSTEMIOIO;
typedef OBJSYSTEMIOIO   *POBJSYSTEMIOIO;

/* -------------------------- */
/*  System-I/O control class  */
/* -------------------------- */
typedef struct                                  /* System-I/O control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       dwOffset40h;                       /* Offset-40h: PCI control */
                                          /* Offset-41h: PCI arbiter control */
                                 /* Offset-42h: PCI arbiter priority control */
   DWORD       dwOffset44h;                    /* Offset-44h: MEMCS# control */
                                        /* Offset-45h: MEMCS# bottom of hole */
                                           /* Offset-46h: MEMCS# top of hole */
                                         /* Offset-47h: MEMCS# top of memory */
   DWORD       dwOffset48h;       /* Offset-48h: ISA address decoder control */
                         /* Offset-49h: ISA address decoder ROM block enable */
                           /* Offset-4Ah: ISA address decoder bottom of hole */
                              /* Offset-4Bh: ISA address decoder top of hole */
   DWORD       dwOffset4Ch;     /* Offset-4Ch: ISA controller recovery timer */
                                            /* Offset-4Dh: ISA clock divisor */
                             /* Offset-4Eh: Utility bus chip select enable A */
                             /* Offset-4Fh: Utility bus chip select enable B */
   DWORD       dwOffset54h;               /* Offset-54h: MEMCS# attribute #1 */
                                          /* Offset-55h: MEMCS# attribute #2 */
                                          /* Offset-56h: MEMCS# attribute #3 */
                   /* Offset-57h: DMA Scatter/Gather relocation base address */
   DWORD       dwOffset60h;              /* Offset-60h: PIRQ0# route control */
                                         /* Offset-61h: PIRQ1# route control */
                                         /* Offset-62h: PIRQ2# route control */
                                         /* Offset-63h: PIRQ3# route control */
   DWORD       dwOffset80h;           /* Offset-80h: BIOS timer base address */
   DWORD       dwOffsetA0h;                       /* Offset-A0h: SMI control */
                                                   /* Offset-A2h: SMI enable */
   DWORD       dwOffsetA4h;               /* Offset-A4h: System event enable */
   DWORD       dwOffsetA8h;                    /* Offset-A8h: Fast off timer */
                                                  /* Offset-AAh: SMI request */
   DWORD       dwOffsetACh;  /* Offset-ACh: Clock throttle STPCLK# low timer */
                            /* Offset-AEh: Clock throttle STPCLK# high timer */
   DWORD       dwOffsetB0h;                  /* Offset-B2h: APM control port */
                                              /* Offset-B3h: APM status port */
   BYTE        chElcr1;                     /* Edge/Level control 1 register */
   BYTE        chElcr2;                     /* Edge/Level control 2 register */
} OBJSYSTEMIOCTL;
typedef OBJSYSTEMIOCTL  *POBJSYSTEMIOCTL;

/* ************************************* */
/* *        System-I/O Objects         * */
/* ************************************* */
/* -------------------------- */
/*  System-I/O control class  */
/* -------------------------- */
extern OBJSYSTEMIOCTL oSystemioCtl;

/* ************************************* */
/* * Object/Message Control Definition * */
/* ************************************* */
/* ------------------------ */
/*  Message control macros  */
/* ------------------------ */
#define  BuildSioParm1(pkt, parm1)                                   \
            *(((PMSGSYSTEMIO)&(pkt))->pdwLongParm) = (DWORD)(parm1);

#define  SelectSioParm1(pkt)  *(((PMSGSYSTEMIO)&(pkt))->pdwLongParm)


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDSIO */
/* *****************************  End of File  ***************************** */
