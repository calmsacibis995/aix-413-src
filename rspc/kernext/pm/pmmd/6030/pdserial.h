/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Serial Port Device Control Definitions
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
/* * Module Name: PDSERIAL.H                                               * */
/* * Description: Serial Port Device Control Definitions                   * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDSERIAL
#define  _H_PDSERIAL


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     Serial Message Definition     * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                      /* Serial message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGSERIAL;
typedef MSGSERIAL *PMSGSERIAL;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Serial I/O class */
#define  MSGSER_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGSER_READ_RECEIVER             (0x0001)
                                            /* Read receiver buffer register */
#define  MSGSER_READ_INTERRUPTENABLE      (0x0002)
                                           /* Read interrupt enable register */
#define  MSGSER_READ_INTERRUPTID          (0x0003)
                                   /* Read interrupt identification register */
#define  MSGSER_READ_LINECONTROL          (0x0004)
                                               /* Read line control register */
#define  MSGSER_READ_MODEMCONTROL         (0x0005)
                                              /* Read MODEM control register */
#define  MSGSER_READ_LINESTATUS           (0x0006)
                                                /* Read line status register */
#define  MSGSER_READ_MODEMSTATUS          (0x0007)
                                               /* Read MODEM status register */
#define  MSGSER_READ_SCRATCHPAD           (0x0008)
                                                /* Read scratch pad register */
#define  MSGSER_READ_DIVISORLATCHLSB      (0x0009)
                                          /* Read divisor latch LSB register */
#define  MSGSER_READ_DIVISORLATCHMSB      (0x000A)
                                          /* Read divisor latch MSB register */
#define  MSGSER_WRITE_TRANSMITTER         (0x000B)
                                       /* Write transmitter holding register */
#define  MSGSER_WRITE_INTERRUPTENABLE     (0x000C)
                                          /* Write interrupt enable register */
#define  MSGSER_WRITE_FIFOCONTROL         (0x000D)
                                              /* Write FIFO control register */
#define  MSGSER_WRITE_LINECONTROL         (0x000E)
                                              /* Write line control register */
#define  MSGSER_WRITE_MODEMCONTROL        (0x000F)
                                             /* Write MODEM control register */
#define  MSGSER_WRITE_MODEMSTATUS         (0x0010)
                                              /* Write MODEM status register */
#define  MSGSER_WRITE_SCRATCHPAD          (0x0011)
                                               /* Write scratch pad register */
#define  MSGSER_WRITE_DIVISORLATCHLSB     (0x0012)
                                         /* Write divisor latch LSB register */
#define  MSGSER_WRITE_DIVISORLATCHMSB     (0x0013)
                                         /* Write divisor latch MSB register */
#define  MSGSER_MODIFY_LINECONTROL        (0x0014)
                                        /* Read/Modify line control register */
#define  MSGSER_MODIFY_MODEMCONTROL       (0x0015)
                                       /* Read/Modify MODEM control register */

/* Serial control class */
#define  MSGSER_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGSER_SAVE_CONTEXT              (0x0101)
                                                      /* Save device context */
#define  MSGSER_RESTORE_CONTEXT           (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRSER_NOERROR                   (0x0000) /* No error                */
#define  ERRSER_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRSER_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRSER_NOT_INITIALIZED           (0x0003) /* Not initialized yet     */
#define  ERRSER_CANNOT_INITIALIZE         (0x0004) /* Can not initialize      */
#define  ERRSER_CANNOT_READREGISTER       (0x0005) /* Can not read registers  */

/* ************************************* */
/* *     Serial Objects Definition     * */
/* ************************************* */
/* ------------------ */
/*  Serial I/O class  */
/* ------------------ */
typedef struct                                          /* Serial I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
   BYTE        chSerialId;             /* Serial identification (Serial-A/B) */
} OBJSERIALIO;
typedef OBJSERIALIO  *POBJSERIALIO;

/* ---------------------- */
/*  Serial control class  */
/* ---------------------- */
typedef struct                                      /* Serial control object */
{
   OBJCOMMON      oCommon;                                  /* Common object */
   POBJSERIALIO   poSerialIo;                /* Pointer to Serial I/O object */
   BYTE           chSerialId;          /* Serial identification (Serial-A/B) */
   BYTE           chInterruptEnable;            /* Interrupt enable register */
   BYTE           chFIFOControl;                    /* FIFO control register */
   BYTE           chLineControl;                    /* Line control register */
   BYTE           chMODEMControl;                  /* MODEM control register */
   BYTE           chScratchPad;                      /* Scratch pad register */
   BYTE           chDivisorLatchLSB;           /* Divisor latch LSB register */
   BYTE           chDivisorLatchMSB;           /* Divisor latch MSB register */
} OBJSERIALCTL;
typedef OBJSERIALCTL *POBJSERIALCTL;

/* ************************************* */
/* *          Serial Objects           * */
/* ************************************* */
/* ---------------------- */
/*  Serial control class  */
/* ---------------------- */
extern OBJSERIALCTL oSerialACtl;
extern OBJSERIALCTL oSerialBCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDSERIAL */
/* *****************************  End of File  ***************************** */
