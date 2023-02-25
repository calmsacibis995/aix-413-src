/* @(#)92  1.2  src/rspc/kernext/pm/pmmd/6030/pdv7310.h, pwrsysx, rspc41J, 9517A_b 4/25/95 04:18:56
 *
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: V7310 (Ascii Video Capture Controller) Chip Device
 *              Control Definitions
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
/* * Module Name: PDV7310.H                                                * */
/* * Description: V7310 (Ascii Video Capture Controller) Chip Device       * */
/* *              Control Definitions                                      * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDV7310
#define  _H_PDV7310


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     V7310 Message Definition      * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                       /* V7310 message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGV7310;
typedef MSGV7310  *PMSGV7310;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* V7310 I/O class */
#define  MSGV73_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGV73_GET_INDEX                 (0x0001)
                                                       /* Get index register */
#define  MSGV73_SET_INDEX                 (0x0002)
                                                       /* Set index register */
#define  MSGV73_READ_FNCSEL               (0x0003)
                                            /* Read function select register */
#define  MSGV73_READ_PWRCTL               (0x0004)
                                              /* Read power control register */
#define  MSGV73_READ_GIOCTL0              (0x0005)
                                      /* Read general I/O control 0 register */
#define  MSGV73_READ_GOTDAT0              (0x0006)
                                      /* Read general output data 0 register */
#define  MSGV73_WRITE_FNCSEL              (0x0007)
                                           /* Write function select register */
#define  MSGV73_WRITE_PWRCTL              (0x0008)
                                             /* Write power control register */
#define  MSGV73_WRITE_GIOCTL0             (0x0009)
                                     /* Write general I/O control 0 register */
#define  MSGV73_WRITE_GOTDAT0             (0x000A)
                                     /* Write general output data 0 register */
#define  MSGV73_MODIFY_FNCSEL             (0x000B)
                                     /* Read/Modify function select register */
#define  MSGV73_MODIFY_PWRCTL             (0x000C)
                                       /* Read/Modify power control register */
#define  MSGV73_MODIFY_GIOCTL0            (0x000D)
                               /* Read/Modify general I/O control 0 register */
#define  MSGV73_MODIFY_GOTDAT0            (0x000E)
                               /* Read/Modify general output data 0 register */

/* V7310 control class */
#define  MSGV73_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGV73_SAVE_CONTEXT              (0x0101)
                                                      /* Save device context */
#define  MSGV73_RESTORE_CONTEXT           (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRV73_NOERROR                   (0x0000) /* No error                */
#define  ERRV73_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRV73_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRV73_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRV73_NOT_INITIALIZED           (0x0004) /* Not initialized yet     */
#define  ERRV73_CANNOT_INITIALIZE         (0x0005) /* Can not initialize      */
#define  ERRV73_CANNOT_PCIACCESS          (0x0006) /* Can not access PCI bus  */

/* ************************************* */
/* *     V7310 Objects Definition      * */
/* ************************************* */
/* ----------------- */
/*  V7310 I/O class  */
/* ----------------- */
typedef struct                                           /* V7310 I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJV7310IO;
typedef OBJV7310IO   *POBJV7310IO;

/* --------------------- */
/*  V7310 control class  */
/* --------------------- */
typedef struct                                       /* V7310 control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chIndex;                                    /* Index register */
   BYTE        chFncsel;                         /* Function select register */
   BYTE        chPwrctl;                           /* Power control register */
   BYTE        chGioctl0;                  /* General I/O control 0 register */
   BYTE        chGotdat0;                  /* General output data 0 register */
} OBJV7310CTL;
typedef OBJV7310CTL  *POBJV7310CTL;

/* ************************************* */
/* *           V7310 Objects           * */
/* ************************************* */
/* --------------------- */
/*  V7310 control class  */
/* --------------------- */
extern OBJV7310CTL oV7310Ctl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDV7310 */
/* *****************************  End of File  ***************************** */
