/* @(#)76  1.2  src/rspc/kernext/pm/pmmd/6030/pdpower.h, pwrsysx, rspc41J, 9521A_all 5/22/95 20:53:12
 *
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Device Power Control Definitions
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
/* * Module Name: PDPOWER.H                                                * */
/* * Description: Device Power Control Definitions                         * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPOWER
#define  _H_PDPOWER


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     Power Message Definition      * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                       /* Power message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGPOWER;
typedef MSGPOWER  *PMSGPOWER;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Power control class */
#define  MSGPWR_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGPWR_SET_MAINPOWER             (0x0101)
                                                           /* Set main power */
#define  MSGPWR_SET_SUSPENDPOWER          (0x0102)
                                                        /* Set suspend power */
#define  MSGPWR_GET_LCDPOWER              (0x0103)
                                                     /* Get LCD device power */
#define  MSGPWR_SET_LCDPOWER              (0x0104)
                                                     /* Set LCD device power */
#define  MSGPWR_GET_LCDBLPOWER            (0x0105)
                                                  /* Get LCD backlight power */
#define  MSGPWR_SET_LCDBLPOWER            (0x0106)
                                                  /* Set LCD backlight power */
#define  MSGPWR_GET_LCDPLPOWER            (0x0107)
                                                      /* Get LCD panel power */
#define  MSGPWR_SET_LCDPLPOWER            (0x0108)
                                                      /* Set LCD panel power */
#define  MSGPWR_GET_VIDEOPOWER            (0x0109)
                                                     /* Get video chip power */
#define  MSGPWR_SET_VIDEOPOWER            (0x010A)
                                                     /* Set video chip power */
#define  MSGPWR_GET_CAMERAPOWER           (0x010B)
                                                  /* Get camera device power */
#define  MSGPWR_SET_CAMERAPOWER           (0x010C)
                                                  /* Set camera device power */
#define  MSGPWR_GET_VIDEODIGITIZERPOWER   (0x010D)
                                         /* Get video digitizer device power */
#define  MSGPWR_SET_VIDEODIGITIZERPOWER   (0x010E)
                                         /* Set video digitizer device power */
#define  MSGPWR_GET_SCSICLOCKPOWER        (0x010F)
                                                     /* Get SCSI clock power */
#define  MSGPWR_SET_SCSICLOCKPOWER        (0x0110)
                                                     /* Set SCSI clock power */
#define  MSGPWR_GET_CDROMDRIVEPOWER       (0x0111)
                                                   /* Get CD-ROM drive power */
#define  MSGPWR_SET_CDROMDRIVEPOWER       (0x0112)
                                                   /* Set CD-ROM drive power */
#define  MSGPWR_GET_AUDIOPOWER            (0x0113)
                                                   /* Get audio device power */
#define  MSGPWR_SET_AUDIOPOWER            (0x0114)
                                                   /* Set audio device power */
#define  MSGPWR_GET_L2CACHEPOWER          (0x0115)
                                                /* Get L2-cache device power */
#define  MSGPWR_SET_L2CACHEPOWER          (0x0116)
                                                /* Set L2-cache device power */
#define     PRMPWR_POWERON                (0x0001)        /* Device power on */
#define     PRMPWR_POWEROFF               (0x0002)       /* Device power off */
#define  MSGPWR_GET_LCDBRIGHTNESS         (0x0117)
                                                 /* Get LCD brightness power */
#define  MSGPWR_SET_LCDBRIGHTNESS         (0x0118)
                                                 /* Set LCD brightness power */
#define     PRMPWR_FULLON                 (0x0001)  /* Brightness full power */
#define     PRMPWR_HALFON                 (0x0002)  /* Brightness half power */
#define  MSGPWR_GET_LCDSTATUS             (0x0119)
                                                           /* Get LCD status */
#define  MSGPWR_SET_LCDSTATUS             (0x011A)
                                                           /* Set LCD status */
#define     PRMPWR_LCDENABLE              (0x0001)             /* LCD enable */
#define     PRMPWR_LCDDISABLE             (0x0002)            /* LCD disable */
#define  MSGPWR_GET_SCSITERMPOWER         (0x011B)
                                                /* Get SCSI terminator power */
#define  MSGPWR_SET_SCSITERMPOWER         (0x011C)
                                                /* Set SCSI terminator power */
#define     PRMPWR_TERMINATORON           (0x0001)          /* Terminator on */
#define     PRMPWR_TERMINATORSTANDBY      (0x0002)    /* Terminator stand-by */
#define     PRMPWR_TERMINATOROFF          (0x0004)         /* Terminator off */
#define  MSGPWR_GET_CRTPOWER              (0x011D)
                                                     /* Get CRT device power */
#define  MSGPWR_SET_CRTPOWER              (0x011E)
                                                     /* Set CRT device power */
#define     PRMPWR_CRTON                  (0x0001)           /* CRT power on */
#define     PRMPWR_CRTSTANDBY             (0x0002)     /* CRT power stand-by */
#define     PRMPWR_CRTSUSPEND             (0x0003)      /* CRT power suspend */
#define     PRMPWR_CRTOFF                 (0x0004)          /* CRT power off */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRPWR_NOERROR                   (0x0000) /* No error                */
#define  ERRPWR_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRPWR_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRPWR_CANNOT_POWERCONTROL       (0x0003)
                                              /* Can not control device power */

/* ************************************* */
/* *     Power Objects Definition      * */
/* ************************************* */
/* --------------------- */
/*  Power control class  */
/* --------------------- */
typedef struct                                       /* Power control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   CPARAMETER  cpLcdStatus;        /* LCD device power status before suspend */
} OBJPOWERCTL;
typedef OBJPOWERCTL  *POBJPOWERCTL;

/* ************************************* */
/* *           Power Objects           * */
/* ************************************* */
/* --------------------- */
/*  Power control class  */
/* --------------------- */
extern OBJPOWERCTL oPowerCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDPOWER */
/* *****************************  End of File  ***************************** */
