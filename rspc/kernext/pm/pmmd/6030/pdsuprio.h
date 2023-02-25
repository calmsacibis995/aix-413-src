/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Super I/O (PC87322/311) Chip Device Control Definitions
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
/* * Module Name: PDSUPRIO.H                                               * */
/* * Description: Super I/O (PC87322/311) Chip Device Control Definitions  * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDSUPRIO
#define  _H_PDSUPRIO


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *   Super-I/O Message Definition    * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                   /* Super-I/O message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGSUPERIO;
typedef MSGSUPERIO   *PMSGSUPERIO;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Super-I/O I/O class */
#define  MSGSPIO_INITIALIZE               (0x0000)
                                                               /* Initialize */
#define  MSGSPIO_GET_INDEX                (0x0001)
                                                       /* Get index register */
#define  MSGSPIO_SET_INDEX                (0x0002)
                                                       /* Set index register */
#define  MSGSPIO_READ_FER                 (0x0003)
                                            /* Read function enable register */
#define  MSGSPIO_READ_FAR                 (0x0004)
                                           /* Read function address register */
#define  MSGSPIO_READ_PTR                 (0x0005)
                                             /* Read power and test register */
#define  MSGSPIO_READ_FCR                 (0x0006)
                                           /* Read function control register */
#define  MSGSPIO_READ_PCR                 (0x0007)
                                            /* Read printer control register */
#define  MSGSPIO_WRITE_FER                (0x0008)
                                           /* Write function enable register */
#define  MSGSPIO_WRITE_FAR                (0x0009)
                                          /* Write function address register */
#define  MSGSPIO_WRITE_PTR                (0x000A)
                                            /* Write power and test register */
#define  MSGSPIO_WRITE_FCR                (0x000B)
                                          /* Write function control register */
#define  MSGSPIO_WRITE_PCR                (0x000C)
                                           /* Write printer control register */
#define  MSGSPIO_MODIFY_FER               (0x000D)
                                     /* Read/Modify function enable register */
#define  MSGSPIO_MODIFY_FAR               (0x000E)
                                    /* Read/Modify function address register */
#define  MSGSPIO_MODIFY_PTR               (0x000F)
                                      /* Read/Modify power and test register */
#define  MSGSPIO_MODIFY_FCR               (0x0010)
                                    /* Read/Modify function control register */
#define  MSGSPIO_MODIFY_PCR               (0x0011)
                                     /* Read/Modify printer control register */

/* Super-I/O control class */
#define  MSGSPIO_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#ifdef PMFUNCTIONS_FULL
#define  MSGSPIO_GET_PARALLELSTATUS       (0x0101)
                                                      /* Get parallel status */
#define  MSGSPIO_SET_PARALLELSTATUS       (0x0102)
                                                      /* Set parallel status */
#define     PRMSPIO_PARALLELENABLE        (0x0001)        /* Parallel enable */
#define     PRMSPIO_PARALLELDISABLE       (0x0002)       /* Parallel disable */
#define  MSGSPIO_GET_PARALLELADDRESS      (0x0103)
                                                /* Get parallel base address */
#define  MSGSPIO_SET_PARALLELADDRESS      (0x0104)
                                                /* Set parallel base address */
#define     PRMSPIO_PARALLELLPT1          (0x03BC)     /* LPT1 (3BC-E, IRQ7) */
#define     PRMSPIO_PARALLELLPT2          (0x0378)     /* LPT2 (378-F, IRQ5) */
#define     PRMSPIO_PARALLELLPT3          (0x0278)     /* LPT3 (278-F, IRQ5) */
#define  MSGSPIO_GET_PARALLELDIRECTION    (0x0105)
                                                   /* Get parallel direction */
#define  MSGSPIO_SET_PARALLELDIRECTION    (0x0106)
                                                   /* Set parallel direction */
#define     PRMSPIO_PARALLELEXTENDED      (0x0001)      /* Extended parallel */
#define     PRMSPIO_PARALLELCOMPATIBLE    (0x0002)    /* Compatible parallel */
#endif /* PMFUNCTIONS_FULL */

#define  MSGSPIO_GET_UART1STATUS          (0x0107)
                                                        /* Get UART 1 status */
#define  MSGSPIO_SET_UART1STATUS          (0x0108)
                                                        /* Set UART 1 status */
#define     PRMSPIO_UART1ENABLE           (0x0001)          /* UART 1 enable */
#define     PRMSPIO_UART1DISABLE          (0x0002)         /* UART 1 disable */
#define  MSGSPIO_GET_UART1ADDRESS         (0x0109)
                                                  /* Get UART 1 base address */
#define  MSGSPIO_SET_UART1ADDRESS         (0x010A)
                                                  /* Set UART 1 base address */
#define     PRMSPIO_UART1COM1             (0x03F8)           /* COM1 (3F8-F) */
#define     PRMSPIO_UART1COM2             (0x02F8)           /* COM2 (2F8-F) */
#define     PRMSPIO_UART1COM3             (0x03E8)           /* COM3 (3E8-F) */
#define     PRMSPIO_UART1COM4             (0x02E8)           /* COM4 (2E8-F) */

#define  MSGSPIO_GET_UART2STATUS          (0x010B)
                                                        /* Get UART 2 status */
#define  MSGSPIO_SET_UART2STATUS          (0x010C)
                                                        /* Set UART 2 status */
#define     PRMSPIO_UART2ENABLE           (0x0001)          /* UART 2 enable */
#define     PRMSPIO_UART2DISABLE          (0x0002)         /* UART 2 disable */
#define  MSGSPIO_GET_UART2ADDRESS         (0x010D)
                                                  /* Get UART 2 base address */
#define  MSGSPIO_SET_UART2ADDRESS         (0x010E)
                                                  /* Set UART 2 base address */
#define     PRMSPIO_UART2COM1             (0x03F8)           /* COM1 (3F8-F) */
#define     PRMSPIO_UART2COM2             (0x02F8)           /* COM2 (2F8-F) */
#define     PRMSPIO_UART2COM3             (0x03E8)           /* COM3 (3E8-F) */
#define     PRMSPIO_UART2COM4             (0x02E8)           /* COM4 (2E8-F) */

#ifdef PMFUNCTIONS_FULL
#define  MSGSPIO_GET_FDCSTATUS            (0x010F)
                                                           /* Get FDC status */
#define  MSGSPIO_SET_FDCSTATUS            (0x0110)
                                                           /* Set FDC status */
#define     PRMSPIO_FDCENABLE             (0x0001)             /* FDC enable */
#define     PRMSPIO_FDCDISABLE            (0x0002)            /* FDC disable */
#define  MSGSPIO_GET_FDCADDRESS           (0x0111)
                                                     /* Get FDC base address */
#define  MSGSPIO_SET_FDCADDRESS           (0x0112)
                                                     /* Set FDC base address */
#define     PRMSPIO_FDCPRIMARY            (0x03F0)    /* FDC primary (3F0-7) */
#define     PRMSPIO_FDCSECONDARY          (0x03F0)  /* FDC secondary (3F0-7) */
#define     PRMSPIO_FDCSECONDARYSET       (0x03F1)  /* FDC secondary (3F0-7) */
                                                        /* (only at setting) */

#define  MSGSPIO_GET_IDESTATUS            (0x0113)
                                                           /* Get IDE status */
#define  MSGSPIO_SET_IDESTATUS            (0x0114)
                                                           /* Set IDE status */
#define     PRMSPIO_IDEENABLE             (0x0001)             /* IDE enable */
#define     PRMSPIO_IDEDISABLE            (0x0002)            /* IDE disable */
#define  MSGSPIO_GET_IDEADDRESS           (0x0115)
                                                     /* Get IDE base address */
#define  MSGSPIO_SET_IDEADDRESS           (0x0116)
                                                     /* Set IDE base address */
#define     PRMSPIO_IDEPRIMARY            (0x01F0)
                                               /* IDE primary (1F0-7, 3F6-7) */
#define     PRMSPIO_IDESECONDARY          (0x0170)
                                             /* IDE secondary (170-7, 376-7) */
#endif /* PMFUNCTIONS_FULL */

#define  MSGSPIO_GET_EXTERNALCLOCK        (0x0117)
                                                 /* Get external clock power */
#define  MSGSPIO_SET_EXTERNALCLOCK        (0x0118)
                                                 /* Set external clock power */
#define     PRMSPIO_CLOCKPOWERUP          (0x0001)
                                                  /* External clock power-up */
#define     PRMSPIO_CLOCKPOWERDOWN        (0x0002)
                                                /* External clock power-down */

#define  MSGSPIO_SAVE_CONTEXT             (0x0119)
                                                      /* Save device context */
#define  MSGSPIO_RESTORE_CONTEXT          (0x011A)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRSPIO_NOERROR                  (0x0000) /* No error                */
#define  ERRSPIO_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERRSPIO_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERRSPIO_UNKNOWN                  (0x0003) /* Unknown port setting    */

/* ************************************* */
/* *   Super-I/O Objects Definition    * */
/* ************************************* */
/* --------------------- */
/*  Super-I/O I/O class  */
/* --------------------- */
typedef struct                                       /* Super-I/O I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJSUPERIOIO;
typedef OBJSUPERIOIO *POBJSUPERIOIO;

/* ------------------------- */
/*  Super-I/O control class  */
/* ------------------------- */
typedef struct                                   /* Super-I/O control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chIndex;                                    /* Index register */
   BYTE        chFer;                            /* Function enable register */
   BYTE        chFar;                           /* Function address register */
   BYTE        chPtr;                             /* Power and test register */
   BYTE        chFcr;                           /* Function control register */
   BYTE        chPcr;                            /* Printer control register */
} OBJSUPERIOCTL;
typedef OBJSUPERIOCTL   *POBJSUPERIOCTL;

/* ************************************* */
/* *         Super-I/O Objects         * */
/* ************************************* */
/* ------------------------- */
/*  Super-I/O control class  */
/* ------------------------- */
extern OBJSUPERIOCTL oSuperioCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDSUPRIO */
/* *****************************  End of File  ***************************** */
