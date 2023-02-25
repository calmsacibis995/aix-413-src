/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Programmable Option Select (POS) Device Control
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
/* * Module Name: PDPOS.H                                                  * */
/* * Description: Programmable Option Select (POS) Device Control          * */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPOS
#define  _H_PDPOS


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      POS Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* POS message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGPOS;
typedef MSGPOS *PMSGPOS;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* POS I/O class */
#define  MSGPOS_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGPOS_GET_PLANARSETUP           (0x0001)
                                         /* Get planar enable/setup register */
#define  MSGPOS_SET_PLANARSETUP           (0x0002)
                                         /* Set planar enable/setup register */
#define  MSGPOS_READ_PLANAR               (0x0003)
                                                 /* Read planar POS register */
#define  MSGPOS_READ_VIDEOSUBSYSTEM       (0x0004)
                                        /* Read video subsystem POS register */
#define  MSGPOS_READ_PENINTERFACE1        (0x0005)
                                        /* Read pen interface POS 1 register */
#define  MSGPOS_READ_PENINTERFACE2        (0x0006)
                                        /* Read pen interface POS 2 register */
#define  MSGPOS_READ_PENINTERFACE3        (0x0007)
                                        /* Read pen interface POS 3 register */
#define  MSGPOS_READ_PCMCIACONTROLLER1    (0x0008)
                                    /* Read PCMCIA controller POS 1 register */
#define  MSGPOS_READ_PCMCIACONTROLLER2    (0x0009)
                                    /* Read PCMCIA controller POS 2 register */
#define  MSGPOS_READ_PCMCIACONTROLLER3    (0x000A)
                                    /* Read PCMCIA controller POS 3 register */
#define  MSGPOS_READ_COOPERENABLE         (0x000B)
                                              /* Read cooper enable register */
#define  MSGPOS_READ_COOPERINDEXL         (0x000C)
                                  /* Read cooper function index low register */
#define  MSGPOS_READ_COOPERINDEXH         (0x000D)
                                 /* Read cooper function index high register */
#define  MSGPOS_READ_FDCSETUP             (0x000E)
                                          /* Read FDC support setup register */
#define  MSGPOS_READ_VIDEOSETUP           (0x000F)
                                        /* Read video support setup register */
#define  MSGPOS_READ_H8SETUP              (0x0010)
                                         /* Read H8 interface setup register */
#define  MSGPOS_READ_AUDIOSETUP           (0x0011)
                                        /* Read audio support setup register */
#define  MSGPOS_READ_AUDIOADDRESSL        (0x0012)
                                      /* Read audio I/O address low register */
#define  MSGPOS_READ_AUDIOADDRESSH        (0x0013)
                                     /* Read audio I/O address high register */
#define  MSGPOS_READ_NVRAMSETUP           (0x0014)
                                      /* Read NVRAM interface setup register */
#define  MSGPOS_READ_DMACOMMANDSHADOW08   (0x0015)
                                   /* Read DMA command shadow '08h' register */
#define  MSGPOS_READ_DMACOMMANDSHADOWD0   (0x0016)
                                   /* Read DMA command shadow 'D0h' register */
#define  MSGPOS_READ_VIDEOSTATUS1         (0x0017)
                                             /* Read video status 1 register */
#define  MSGPOS_READ_VIDEOSTATUS2         (0x0018)
                                             /* Read video status 2 register */
#define  MSGPOS_WRITE_PLANAR              (0x0019)
                                                /* Write planar POS register */
#define  MSGPOS_WRITE_VIDEOSUBSYSTEM      (0x001A)
                                       /* Write video subsystem POS register */
#define  MSGPOS_WRITE_PENINTERFACE1       (0x001B)
                                       /* Write pen interface POS 1 register */
#define  MSGPOS_WRITE_PENINTERFACE2       (0x001C)
                                       /* Write pen interface POS 2 register */
#define  MSGPOS_WRITE_PENINTERFACE3       (0x001D)
                                       /* Write pen interface POS 3 register */
#define  MSGPOS_WRITE_PCMCIACONTROLLER1   (0x001E)
                                   /* Write PCMCIA controller POS 1 register */
#define  MSGPOS_WRITE_PCMCIACONTROLLER2   (0x001F)
                                   /* Write PCMCIA controller POS 2 register */
#define  MSGPOS_WRITE_PCMCIACONTROLLER3   (0x0020)
                                   /* Write PCMCIA controller POS 3 register */
#define  MSGPOS_WRITE_COOPERENABLE        (0x0021)
                                             /* Write cooper enable register */
#define  MSGPOS_WRITE_COOPERINDEXL        (0x0022)
                                 /* Write cooper function index low register */
#define  MSGPOS_WRITE_COOPERINDEXH        (0x0023)
                                /* Write cooper function index high register */
#define  MSGPOS_WRITE_FDCSETUP            (0x0024)
                                         /* Write FDC support setup register */
#define  MSGPOS_WRITE_VIDEOSETUP          (0x0025)
                                       /* Write video support setup register */
#define  MSGPOS_WRITE_H8SETUP             (0x0026)
                                        /* Write H8 interface setup register */
#define  MSGPOS_WRITE_AUDIOSETUP          (0x0027)
                                       /* Write audio support setup register */
#define  MSGPOS_WRITE_AUDIOADDRESSL       (0x0028)
                                     /* Write audio I/O address low register */
#define  MSGPOS_WRITE_AUDIOADDRESSH       (0x0029)
                                    /* Write audio I/O address high register */
#define  MSGPOS_WRITE_NVRAMSETUP          (0x002A)
                                     /* Write NVRAM interface setup register */

/* POS control class */
#define  MSGPOS_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGPOS_SAVE_CONTEXT              (0x0101)
                                                      /* Save device context */
#define  MSGPOS_RESTORE_CONTEXT           (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRPOS_NOERROR                   (0x0000) /* No error                */
#define  ERRPOS_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRPOS_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRPOS_UNKNOWN                   (0x0003) /* Unknown port setting    */

/* ************************************* */
/* *      POS Objects Definition       * */
/* ************************************* */
/* --------------- */
/*  POS I/O class  */
/* --------------- */
typedef struct                                             /* POS I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJPOSIO;
typedef OBJPOSIO  *POBJPOSIO;

/* ------------------- */
/*  POS control class  */
/* ------------------- */
typedef struct                                         /* POS control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chPlanarSetup;                /* Planar enable/setup register */
   BYTE        chPlanar;                              /* Planar POS register */
   BYTE        chVideoSubsystem;             /* Video subsystem POS register */
   BYTE        chPenInterface1;              /* Pen interface POS 1 register */
   BYTE        chPenInterface2;              /* Pen interface POS 2 register */
   BYTE        chPenInterface3;              /* Pen interface POS 3 register */
   BYTE        chPcmciaController1;      /* PCMCIA controller POS 1 register */
   BYTE        chPcmciaController2;      /* PCMCIA controller POS 2 register */
   BYTE        chPcmciaController3;      /* PCMCIA controller POS 3 register */
   BYTE        chCooperEnable;                     /* Cooper enable register */
   BYTE        chCooperIndexL;         /* Cooper function index low register */
   BYTE        chCooperIndexH;        /* Cooper function index high register */
   BYTE        chFdcSetup;                     /* FDC support setup register */
   BYTE        chVideoSetup;                 /* Video support setup register */
   BYTE        chH8Setup;                     /* H8 interface setup register */
   BYTE        chAudioSetup;                 /* Audio support setup register */
   BYTE        chAudioAddressL;            /* Audio I/O address low register */
   BYTE        chAudioAddressH;           /* Audio I/O address high register */
   BYTE        chNvramSetup;               /* NVRAM interface setup register */
} OBJPOSCTL;
typedef OBJPOSCTL *POBJPOSCTL;

/* ************************************* */
/* *            POS Objects            * */
/* ************************************* */
/* ------------------- */
/*  POS control class  */
/* ------------------- */
extern OBJPOSCTL oPosCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDPOS */
/* *****************************  End of File  ***************************** */
