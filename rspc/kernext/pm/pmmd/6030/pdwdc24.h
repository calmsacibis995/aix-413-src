/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Video (Western Digital 90C24) Chip Device Control
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
/* * Module Name: PDWDC24.H                                                * */
/* * Description: Video (Western Digital 90C24) Chip Device Control        * */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDWDC24
#define  _H_PDWDC24


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *    WD90C24 Message Definition     * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                     /* WD90C24 message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
   DWORD       pdwLongParm[1];                /* Extended parameters (DWORD) */
} MSGWD90C24;
typedef MSGWD90C24   *PMSGWD90C24;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* WD90C24 I/O class */
#define  MSGC24_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGC24_READ_P3MODE               (0x0001)
                                                    /* Read P3 mode register */
#define  MSGC24_WRITE_P3MODE              (0x0002)
                                                   /* Write P3 mode register */

/* WD90C24 control class */
#define  MSGC24_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGC24_SAVE_CONTEXT              (0x0101)
                                                      /* Save device context */
#define  MSGC24_RESTORE_CONTEXT           (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRC24_NOERROR                   (0x0000) /* No error                */
#define  ERRC24_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRC24_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRC24_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRC24_NOT_INITIALIZED           (0x0004) /* Not initialized         */
#define  ERRC24_CANNOT_INITIALIZE         (0x0005) /* Can not initialize      */
#define  ERRC24_CANNOT_PCIACCESS          (0x0006) /* Can not access PCI bus  */

/* ************************************* */
/* *    WD90C24 Objects Definition     * */
/* ************************************* */
/* ------------------- */
/*  WD90C24 I/O class  */
/* ------------------- */
typedef struct                                         /* WD90C24 I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJWD90C24IO;
typedef OBJWD90C24IO *POBJWD90C24IO;

/* ----------------------- */
/*  WD90C24 control class  */
/* ----------------------- */
typedef struct                                     /* WD90C24 control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       dwOffset80h;                           /* Offset-80h: P3 mode */
} OBJWD90C24CTL;
typedef OBJWD90C24CTL   *POBJWD90C24CTL;

/* ************************************* */
/* *          WD90C24 Objects          * */
/* ************************************* */
/* ----------------------- */
/*  WD90C24 control class  */
/* ----------------------- */
extern OBJWD90C24CTL oWd90c24Ctl;

/* ************************************* */
/* * Object/Message Control Definition * */
/* ************************************* */
/* ------------------------ */
/*  Message control macros  */
/* ------------------------ */
#define  BuildC24Parm1(pkt, parm1)                                  \
            *(((PMSGWD90C24)&(pkt))->pdwLongParm) = (DWORD)(parm1);

#define  SelectC24Parm1(pkt)  *(((PMSGWD90C24)&(pkt))->pdwLongParm)


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDWDC24 */
/* *****************************  End of File  ***************************** */
