/* @(#)14	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pdkmc.h, pwrsysx, rspc41J, 9515B_all 4/14/95 10:55:29  */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Keyboard/Mouse Controller (8042) Chip Device Control
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
/* * Module Name: PDKMC.H                                                  * */
/* * Description: Keyboard/Mouse Controller (8042) Chip Device Control     * */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDKMC
#define  _H_PDKMC


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      KMC Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* KMC message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[10];                      /* Extended parameters */
} MSGKMC;
typedef MSGKMC *PMSGKMC;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* KMC I/O class */
#define  MSGKMC_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGKMC_READ_STATUS               (0x0001)
                                                     /* Read status register */
#define  MSGKMC_READ_DATA                 (0x0002)
                                                       /* Read data register */
#define  MSGKMC_WRITE_COMMAND             (0x0003)
                                                   /* Write command register */
#define  MSGKMC_WRITE_DATA                (0x0004)
                                                      /* Write data register */

/* KMC control class */
#define  MSGKMC_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGKMC_QUERY_INHIBIT             (0x0101)
                                                     /* Query inhibit switch */
#define     PRMKMC_NOINHIBIT              (0x0001)             /* No inhibit */
#define     PRMKMC_INHIBIT                (0x0002)/* Inhibit, password state */
#define  MSGKMC_QUERY_IBF                 (0x0102)
                                                  /* Query input buffer full */
#define  MSGKMC_QUERY_OBF                 (0x0103)
                                                 /* Query output buffer full */
#define  MSGKMC_QUERY_IBFOBF              (0x0104)
                                           /* Query input/output buffer full */
#define     PRMKMC_EMPTY                  (0x0001)           /* Buffer empty */
#define     PRMKMC_FULL                   (0x0002)            /* Buffer full */
#define  MSGKMC_QUERY_DATAERROR           (0x0105)
                                  /* Query parity error and general time-out */
#define     PRMKMC_NOERROR                (0x0001)               /* No error */
#define     PRMKMC_DATAERROR              (0x0002)             /* Data error */
#define  MSGKMC_QUERY_DATATYPE            (0x0106)
                                                 /* Query received data type */
#define     PRMKMC_KEYBOARD               (0x0001)               /* Keyboard */
#define     PRMKMC_AUXILIARY              (0x0002)              /* Auxiliary */
#define  MSGKMC_SEND_COMMAND              (0x0107)
                                                             /* Send command */
#define  MSGKMC_SEND_COMMANDDATA          (0x0108)
                                                    /* Send command and data */
#define  MSGKMC_SEND_COMMANDDATAAUX       (0x0109)
                               /* Send command and data for auxiliary device */
#define  MSGKMC_SEND_COMMANDDATAAUXACK    (0x010A)
                               /* Send command and data for auxiliary device */
                                              /* with waiting acknowledgment */
#define  MSGKMC_RECEIVE_DATA              (0x010B)
                                                             /* Receive data */
#define     PRMKMC_MASKDATATYPE           (0xFF00)
                                                  /* Mask bits for data type */
#define     PRMKMC_MASKDATA               (0x00FF)
                                                       /* Mask bits for data */
#define     PRMKMC_DATASIZEMAX            (0x000A)
                                       /* Receive data buffer size (maximum) */
                                         /* (must not be over parameter size */
#define  MSGKMC_CLEAR_DATA                (0x010C)
                                              /* Clear received garbage data */
#define  MSGKMC_GET_CCB                   (0x010D)
                                              /* Get controller command byte */
#define  MSGKMC_SET_CCB                   (0x010E)
                                              /* Set controller command byte */
#define  MSGKMC_GET_KBDSTATUS             (0x010F)
                                                      /* Get keyboard status */
#define  MSGKMC_SET_KBDSTATUS             (0x0110)
                                                      /* Set keyboard status */
#define     PRMKMC_KBDENABLE              (0x0001)        /* Keyboard enable */
#define     PRMKMC_KBDDISABLE             (0x0002)       /* Keyboard disable */
#define  MSGKMC_GET_AUXSTATUS             (0x0111)
                                              /* Get auxiliary device status */
#define  MSGKMC_SET_AUXSTATUS             (0x0112)
                                              /* Set auxiliary device status */
#define     PRMKMC_AUXENABLE              (0x0001)       /* Auxiliary enable */
#define     PRMKMC_AUXDISABLE             (0x0002)      /* Auxiliary disable */
#define  MSGKMC_SAVE_CONTEXT              (0x0113)
                                                      /* Save device context */
#define  MSGKMC_RESTORE_CONTEXT           (0x0114)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRKMC_NOERROR                   (0x0000) /* No error                */
#define  ERRKMC_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRKMC_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRKMC_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRKMC_ACCESS_DENIED             (0x0004) /* Access denied           */
#define  ERRKMC_DATAERROR                 (0x0005) /* Data error              */
#define  ERRKMC_TIMEOUT                   (0x0006) /* Time-out                */

/* ************************************* */
/* *      KMC Objects Definition       * */
/* ************************************* */
/* --------------- */
/*  KMC I/O class  */
/* --------------- */
typedef struct                                             /* KMC I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJKMCIO;
typedef OBJKMCIO  *POBJKMCIO;

/* ------------------- */
/*  KMC control class  */
/* ------------------- */
typedef struct                                         /* KMC control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chCcb;                             /* Controller command byte */
} OBJKMCCTL;
typedef OBJKMCCTL *POBJKMCCTL;

/* ************************************* */
/* *            KMC Objects            * */
/* ************************************* */
/* ------------------- */
/*  KMC control class  */
/* ------------------- */
extern OBJKMCCTL oKmcCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDKMC */
/* *****************************  End of File  ***************************** */
