/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: EAGLE Memory Controller Chip Device Control Definitions
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
/* * Module Name: PDEAGLE.H                                                * */
/* * Description: EAGLE Memory Controller Chip Device Control Definitions  * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDEAGLE
#define  _H_PDEAGLE


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     EAGLE Message Definition      * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                       /* EAGLE message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
   DWORD       pdwLongParm[2];                /* Extended parameters (DWORD) */
} MSGEAGLE;
typedef MSGEAGLE  *PMSGEAGLE;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* EAGLE I/O class */
#define  MSGEAGL_INITIALIZE               (0x0000)
                                                               /* Initialize */
#define  MSGEAGL_READ_POWERMGTCONFIG      (0x0001)
                             /* Read power management configuration register */
#define  MSGEAGL_READ_MEMCTRLCONFIG8F0    (0x0002)
                        /* Read memory control configuration 8 register (F0) */
#define  MSGEAGL_WRITE_POWERMGTCONFIG     (0x0003)
                            /* Write power management configuration register */
#define  MSGEAGL_WRITE_MEMCTRLCONFIG8F0   (0x0004)
                       /* Write memory control configuration 8 register (F0) */
#define  MSGEAGL_MODIFY_POWERMGTCONFIG    (0x0005)
                      /* Read/Modify power management configuration register */
#define  MSGEAGL_MODIFY_MEMCTRLCONFIG8F0  (0x0006)
                 /* Read/Modify memory control configuration 8 register (F0) */

/* EAGLE control class */
#define  MSGEAGL_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#define  MSGEAGL_SET_POWERMANAGEMENTMODE  (0x0101)
                                                /* Set power management mode */
#define     PRMEAGL_POWERMODEENABLE       (0x0001)      /* Power mode enable */
#define     PRMEAGL_POWERMODEDISABLE      (0x0002)     /* Power mode disable */
#define  MSGEAGL_SET_REFRESHMODE          (0x0102)
                                                         /* Set refresh mode */
#define     PRMEAGL_SELFREFRESH           (0x0001)      /* Self refresh mode */
#define     PRMEAGL_LOWPOWERREFRESH       (0x0002) /* Low power refresh mode */
#define  MSGEAGL_ENTER_SUSPEND            (0x0103)
                                                            /* Enter suspend */
#define  MSGEAGL_SAVE_CONTEXT             (0x0104)
                                                      /* Save device context */
#define  MSGEAGL_RESTORE_CONTEXT          (0x0105)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERREAGL_NOERROR                  (0x0000) /* No error                */
#define  ERREAGL_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERREAGL_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERREAGL_UNKNOWN                  (0x0003) /* Unknown port setting    */
#define  ERREAGL_NOT_INITIALIZED          (0x0004) /* Not initialized         */
#define  ERREAGL_CANNOT_INITIALIZE        (0x0005) /* Can not initialize      */
#define  ERREAGL_CANNOT_PCIACCESS         (0x0006) /* Can not access PCI bus  */

/* ************************************* */
/* *     EAGLE Objects Definition      * */
/* ************************************* */
/* ----------------- */
/*  EAGLE I/O class  */
/* ----------------- */
typedef struct                                           /* EAGLE I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJEAGLEIO;
typedef OBJEAGLEIO   *POBJEAGLEIO;

/* --------------------- */
/*  EAGLE control class  */
/* --------------------- */
typedef struct                                       /* EAGLE control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       dwOffset70h;    /* Offset-70h: Power management configuration */
   DWORD       dwOffsetF0h;    /* Offset-F0h: Memory control configuration 8 */
} OBJEAGLECTL;
typedef OBJEAGLECTL  *POBJEAGLECTL;

/* ************************************* */
/* *           EAGLE Objects           * */
/* ************************************* */
/* --------------------- */
/*  EAGLE control class  */
/* --------------------- */
extern OBJEAGLECTL oEagleCtl;

/* ************************************* */
/* * Object/Message Control Definition * */
/* ************************************* */
/* ------------------------ */
/*  Message control macros  */
/* ------------------------ */
#define  BuildEagleParm1(pkt, parm1)                                  \
            *(((PMSGEAGLE)&(pkt))->pdwLongParm)     = (DWORD)(parm1);

#define  BuildEagleParm2(pkt, parm1, parm2)                           \
            *(((PMSGEAGLE)&(pkt))->pdwLongParm)     = (DWORD)(parm1); \
            *(((PMSGEAGLE)&(pkt))->pdwLongParm + 1) = (DWORD)(parm2);

#define  SelectEagleParm1(pkt)   *(((PMSGEAGLE)&(pkt))->pdwLongParm)

#define  SelectEagleParm2(pkt)   *(((PMSGEAGLE)&(pkt))->pdwLongParm + 1)


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDEAGLE */
/* *****************************  End of File  ***************************** */
