/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Programmable Interrupt Controller (PIC, 8259) Device
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
/* * Module Name: PDPIC.H                                                  * */
/* * Description: Programmable Interrupt Controller (PIC, 8259) Device     * */
/* *              Control Definitions                                      * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPIC
#define  _H_PDPIC


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      PIC Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* PIC message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGPIC;
typedef MSGPIC *PMSGPIC;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* PIC I/O class */
#define  MSGPIC_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGPIC_READ_OCW1                 (0x0001)
                                   /* Read operation control word 1 register */
#define  MSGPIC_READ_OCW3                 (0x0002)
                                   /* Read operation control word 3 register */
#define  MSGPIC_WRITE_ICW1                (0x0003)
                             /* Write initialization command word 1 register */
#define  MSGPIC_WRITE_ICW2                (0x0004)
                             /* Write initialization command word 2 register */
#define  MSGPIC_WRITE_ICW3                (0x0005)
                             /* Write initialization command word 3 register */
#define  MSGPIC_WRITE_ICW4                (0x0006)
                             /* Write initialization command word 4 register */
#define  MSGPIC_WRITE_OCW1                (0x0007)
                                  /* Write operation control word 1 register */
#define  MSGPIC_WRITE_OCW2                (0x0008)
                                  /* Write operation control word 2 register */
#define  MSGPIC_WRITE_OCW3                (0x0009)
                                  /* Write operation control word 3 register */

/* PIC control class */
#define  MSGPIC_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGPIC_DISABLE_INTERRUPT         (0x0101)
                                                    /* Disable all interrupt */
#define  MSGPIC_SAVE_CONTEXT              (0x0102)
                                                      /* Save device context */
#define  MSGPIC_RESTORE_CONTEXT           (0x0103)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRPIC_NOERROR                   (0x0000) /* No error                */
#define  ERRPIC_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRPIC_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRPIC_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRPIC_CANNOT_READREGISTER       (0x0004) /* Can not read registers  */

/* ************************************* */
/* *      PIC Objects Definition       * */
/* ************************************* */
/* --------------- */
/*  PIC I/O class  */
/* --------------- */
typedef struct                                             /* PIC I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
   BYTE        chPicId;                      /* PIC identification (PIC-1/2) */
} OBJPICIO;
typedef OBJPICIO  *POBJPICIO;

/* ------------------- */
/*  PIC control class  */
/* ------------------- */
typedef struct                                         /* PIC control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   POBJPICIO   poPicIo;                         /* Pointer to PIC I/O object */
   BYTE        chPicId;                      /* PIC identification (PIC-1/2) */
   BYTE        chIcw1;             /* Initialization command word 1 register */
   BYTE        chIcw2;             /* Initialization command word 2 register */
   BYTE        chIcw3;             /* Initialization command word 3 register */
   BYTE        chIcw4;             /* Initialization command word 4 register */
   BYTE        chOcw1;                  /* Operation control word 1 register */
   BYTE        chOcw2;                  /* Operation control word 2 register */
   BYTE        chOcw3;                  /* Operation control word 3 register */
} OBJPICCTL;
typedef OBJPICCTL *POBJPICCTL;

/* ************************************* */
/* *            PIC Objects            * */
/* ************************************* */
/* ------------------- */
/*  PIC control class  */
/* ------------------- */
extern OBJPICCTL oPic1Ctl;
extern OBJPICCTL oPic2Ctl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDPIC */
/* *****************************  End of File  ***************************** */
