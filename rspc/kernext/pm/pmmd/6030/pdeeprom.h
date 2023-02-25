/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: C2 Security (EEPROM) Device Control Definitions
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
/* * Module Name: PDEEPROM.H                                               * */
/* * Description: C2 Security (EEPROM) Device Control Definitions          * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDEEPROM
#define  _H_PDEEPROM


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     EEPROM Message Definition     * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                      /* EEPROM message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[20];                      /* Extended parameters */
} MSGEEPROM;
typedef MSGEEPROM *PMSGEEPROM;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* EEPROM I/O class */
#define  MSGEEPROM_INITIALIZE             (0x0000)
                                                               /* Initialize */
#define  PORTEEPROM_READ_CMDADDRMSB       (0x0001)
                                    /* Read command and address MSB register */
#define  PORTEEPROM_READ_SIZELOCK         (0x0002)
                                                  /* Read size lock register */
#define  PORTEEPROM_READ_CMDADDRLSB       (0x0003)
                                    /* Read command and address LSB register */
#define  PORTEEPROM_READ_ADDRESSLOCKS     (0x0004)
                                              /* Read address locks register */
#define  PORTEEPROM_READ_DATALSB          (0x0005)
                                                   /* Read data LSB register */
#define  PORTEEPROM_READ_DATAMSB          (0x0006)
                                                   /* Read data MSB register */
#define  PORTEEPROM_READ_STATUS           (0x0007)
                                                     /* Read status register */
#define  PORTEEPROM_READ_COMMANDSTATUS    (0x0008)
                                         /* Read command and status register */
#define  PORTEEPROM_WRITE_CMDADDRMSB      (0x0009)
                                   /* Write command and address MSB register */
#define  PORTEEPROM_WRITE_SIZELOCK        (0x000A)
                                                 /* Write size lock register */
#define  PORTEEPROM_WRITE_CMDADDRLSB      (0x000B)
                                   /* Write command and address LSB register */
#define  PORTEEPROM_WRITE_ADDRESSLOCKS    (0x000C)
                                             /* Write address locks register */
#define  PORTEEPROM_WRITE_DATALSB         (0x000D)
                                                  /* Write data LSB register */
#define  PORTEEPROM_WRITE_DATAMSB         (0x000E)
                                                  /* Write data MSB register */
#define  PORTEEPROM_WRITE_STATUS          (0x000F)
                                                    /* Write status register */
#define  PORTEEPROM_WRITE_COMMANDSTATUS   (0x0010)
                                        /* Write command and status register */

/* EEPROM control class */
#define  MSGEEPROM_INITIALIZECTL          (0x0100)
                                             /* Initialize for control class */
#define  MSGEEPROM_SAVE_CONTEXT           (0x0101)
                                                      /* Save device context */
#define  MSGEEPROM_RESTORE_CONTEXT        (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERREEPROM_NOERROR                (0x0000) /* No error                */
#define  ERREEPROM_INVALID_MESSAGE        (0x0001) /* Invalid message ID      */
#define  ERREEPROM_INVALID_PARAMETER      (0x0002) /* Invalid parameter value */
#define  ERREEPROM_UNKNOWN                (0x0003) /* Unknown port setting    */

/* ************************************* */
/* *     EEPROM Objects Definition     * */
/* ************************************* */
/* ------------------ */
/*  EEPROM I/O class  */
/* ------------------ */
typedef struct                                          /* EEPROM I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJEEPROMIO;
typedef OBJEEPROMIO  *POBJEEPROMIO;

/* ---------------------- */
/*  EEPROM control class  */
/* ---------------------- */
typedef struct                                      /* EEPROM control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chSizeLock;                             /* Size lock register */
   BYTE        chAddressLocks;                     /* Address locks register */
   BYTE        chStatus;                                  /* Status register */
   BYTE        chCommandStatus;               /* Command and status register */
} OBJEEPROMCTL;
typedef OBJEEPROMCTL *POBJEEPROMCTL;

/* ************************************* */
/* *          EEPROM Objects           * */
/* ************************************* */
/* ---------------------- */
/*  EEPROM control class  */
/* ---------------------- */
extern OBJEEPROMCTL oEepromCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDEEPROM */
/* *****************************  End of File  ***************************** */
