/* @(#)46  1.3  src/rspc/kernext/pm/pmmd/6030/pdcooper.h, pwrsysx, rspc41J, 9521A_all 5/18/95 23:50:54
 *
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Cooper I/O Support Chip Device Control Definitions
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
/* * Module Name: PDCOOPER.H                                               * */
/* * Description: Cooper I/O Support Chip Device Control Definitions       * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDCOOPER
#define  _H_PDCOOPER


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     Cooper Message Definition     * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                      /* Cooper message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[2];                       /* Extended parameters */
} MSGCOOPER;
typedef MSGCOOPER *PMSGCOOPER;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Cooper I/O class */
#define  MSGCOOP_INITIALIZE               (0x0000)
                                                               /* Initialize */
#define  MSGCOOP_GET_PANELINDEX           (0x0001)
                                                 /* Get panel index register */
#define  MSGCOOP_SET_PANELINDEX           (0x0002)
                                                 /* Set panel index register */
#define  MSGCOOP_READ_STORAGELIGHT        (0x0003)
                                              /* Read storage light register */
#define  MSGCOOP_READ_EQUIPMENTPRESENT    (0x0004)
                                          /* Read equipment present register */
#define  MSGCOOP_READ_SYSTEMCONTROL       (0x0005)
                                             /* Read system control register */
#define  MSGCOOP_READ_PCIINTAMAP          (0x0006)
                                          /* Read PCI INT-A mapping register */
#define  MSGCOOP_READ_PCIINTBMAP          (0x0007)
                                          /* Read PCI INT-B mapping register */
#define  MSGCOOP_READ_AUDIOSUPPORT        (0x0008)
                                              /* Read audio support register */
#define  MSGCOOP_READ_SIMM1MEMORYID       (0x0009)
                                      /* Read SIMM 1 base memory ID register */
#define  MSGCOOP_READ_SIMM2MEMORYID       (0x000A)
                                      /* Read SIMM 2 base memory ID register */
#define  MSGCOOP_READ_SIMM3MEMORYID       (0x000B)
                                           /* Read SIMM 3 memory ID register */
#define  MSGCOOP_READ_SIMM4MEMORYID       (0x000C)
                                           /* Read SIMM 4 memory ID register */
#define  MSGCOOP_READ_CMOSSECURITY        (0x000D)
                                              /* Read CMOS security register */
#define  MSGCOOP_READ_PLANARSETUPLOCK     (0x000E)
                                   /* Read planar setup lock/unlock register */
#define  MSGCOOP_READ_MONITORID           (0x000F)
                                                 /* Read monitor ID register */
#define  MSGCOOP_READ_SCSISECURITY        (0x0010)
                                              /* Read SCSI security register */
#define  MSGCOOP_READ_STORAGEPRESENCE     (0x0011)
                                    /* Read storage device presence register */
#define  MSGCOOP_READ_ADAPTERPRESENCE     (0x0012)
                                    /* Read adapter presence detect register */
#define  MSGCOOP_READ_SCSIIDSETUP         (0x0013)
                                              /* Read SCSI ID setup register */
#define  MSGCOOP_READ_PCICINTMAPPING      (0x0014)
                                     /* Read PCIC interrupt mapping register */
#define  MSGCOOP_READ_PCICINTSTATUS       (0x0015)
                                      /* Read PCIC interrupt status register */
#define  MSGCOOP_READ_FDCSUPPORT          (0x0016)
                                                /* Read FDC support register */
#define  MSGCOOP_READ_PANELGRAYLEVEL      (0x0017)
                                       /* Read gray level selection register */
#define  MSGCOOP_READ_PANELSIGNALSOURCE   (0x0018)
                         /* Read flat panel signal source selection register */
#define  MSGCOOP_READ_PANELBULLETINBD1    (0x0019)
                                           /* Read bulletin board 1 register */
#define  MSGCOOP_READ_PANELBULLETINBD2    (0x001A)
                                           /* Read bulletin board 2 register */
#define  MSGCOOP_READ_PANELBULLETINBD3    (0x001B)
                                           /* Read bulletin board 3 register */
#define  MSGCOOP_READ_PANELBULLETINBD4    (0x001C)
                                           /* Read bulletin board 4 register */
#define  MSGCOOP_READ_PANELTYPE           (0x001D)
                                            /* Read flat panel type register */
#define  MSGCOOP_WRITE_STORAGELIGHT       (0x001E)
                                             /* Write storage light register */
#define  MSGCOOP_WRITE_PASSWORDPROTECT1   (0x001F)
                                        /* Write password protect 1 register */
#define  MSGCOOP_WRITE_PASSWORDPROTECT2   (0x0020)
                                        /* Write password protect 2 register */
#define  MSGCOOP_WRITE_L2FLUSH            (0x0021)
                                                  /* Write L2 flush register */
#define  MSGCOOP_WRITE_SYSTEMCONTROL      (0x0022)
                                            /* Write system control register */
#define  MSGCOOP_WRITE_EOIA               (0x0023)
                                              /* Write end of INT-A register */
#define  MSGCOOP_WRITE_PCIINTAMAP         (0x0024)
                                         /* Write PCI INT-A mapping register */
#define  MSGCOOP_WRITE_EOIB               (0x0025)
                                              /* Write end of INT-B register */
#define  MSGCOOP_WRITE_PCIINTBMAP         (0x0026)
                                         /* Write PCI INT-B mapping register */
#define  MSGCOOP_WRITE_AUDIOSUPPORT       (0x0027)
                                             /* Write audio support register */
#define  MSGCOOP_WRITE_CMOSSECURITY       (0x0028)
                                             /* Write CMOS security register */
#define  MSGCOOP_WRITE_PLANARSETUPLOCK    (0x0029)
                                  /* Write planar setup lock/unlock register */
#define  MSGCOOP_WRITE_MONITORID          (0x002A)
                                                /* Write monitor ID register */
#define  MSGCOOP_WRITE_SCSISECURITY       (0x002B)
                                             /* Write SCSI security register */
#define  MSGCOOP_WRITE_SCSIIDSETUP        (0x002C)
                                             /* Write SCSI ID setup register */
#define  MSGCOOP_WRITE_PCICINTMAPPING     (0x002D)
                                    /* Write PCIC interrupt mapping register */
#define  MSGCOOP_WRITE_FDCSUPPORT         (0x002E)
                                               /* Write FDC support register */
#define  MSGCOOP_WRITE_PANELGRAYLEVEL     (0x002F)
                                      /* Write gray level selection register */
#define  MSGCOOP_WRITE_PANELSIGNALSOURCE  (0x0030)
                        /* Write flat panel signal source selection register */
#define  MSGCOOP_WRITE_PANELBULLETINBD1   (0x0031)
                                          /* Write bulletin board 1 register */
#define  MSGCOOP_WRITE_PANELBULLETINBD2   (0x0032)
                                          /* Write bulletin board 2 register */
#define  MSGCOOP_WRITE_PANELBULLETINBD3   (0x0033)
                                          /* Write bulletin board 3 register */
#define  MSGCOOP_WRITE_PANELBULLETINBD4   (0x0034)
                                          /* Write bulletin board 4 register */
#define  MSGCOOP_MODIFY_SYSTEMCONTROL     (0x0035)
                                      /* Read/Modify system control register */
#define  MSGCOOP_MODIFY_AUDIOSUPPORT      (0x0036)
                                       /* Read/Modify audio support register */
#define  MSGCOOP_MODIFY_PLANARSETUPLOCK   (0x0037)
                            /* Read/Modify planar setup lock/unlock register */

/* Cooper control class */
#define  MSGCOOP_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#define  MSGCOOP_GET_PLANARSETUPSTATUS    (0x0101)
                                                  /* Get planar setup status */
#define  MSGCOOP_SET_PLANARSETUPSTATUS    (0x0102)
                                                  /* Set planar setup status */
#define     PRMCOOP_SETUPLOCK             (0x0001)      /* Planar setup lock */
#define     PRMCOOP_SETUPUNLOCK           (0x0002)    /* Planar setup unlock */
#ifdef PMFUNCTIONS_FULL
#define  MSGCOOP_GET_MUTESTATUS           (0x0103)
                                                  /* Get speaker mute status */
#define  MSGCOOP_SET_MUTESTATUS           (0x0104)
                                                  /* Set speaker mute status */
#define     PRMCOOP_MUTEON                (0x0001)                /* Mute on */
#define     PRMCOOP_MUTEOFF               (0x0002)               /* Mute off */
#define  MSGCOOP_QUERY_MONITORID          (0x0105)
                                                         /* Query monitor ID */
#define  MSGCOOP_QUERY_PANELTYPE          (0x0106)
                                                    /* Query flat panel type */
#define     PRMCOOP_TYPENOTATTACH         (0x0001)           /* Not attached */
#define     PRMCOOP_TYPEUNKNOWN           (0x0002)                /* Unknown */
#define     PRMCOOP_TYPETFTCOLOR          (0x0003)         /* TFT color type */
#define     PRMCOOP_TYPESTNCOLOR          (0x0004)         /* STN color type */
#define     PRMCOOP_TYPESTNMONO           (0x0005)         /* STN mono  type */
#define  MSGCOOP_QUERY_EXTERNALSCSI       (0x0107)
                                      /* Query external SCSI device presence */
#define     PRMCOOP_DEVICEPRESENT         (0x0001)         /* Device present */
#define     PRMCOOP_DEVICENOTPRESENT      (0x0002)     /* Device not present */
#endif /* PMFUNCTIONS_FULL */
#define  MSGCOOP_GET_L2CACHESTATUS        (0x0108)
                                                      /* Get L2 cache status */
#define  MSGCOOP_SET_L2CACHESTATUS        (0x0109)
                                                      /* Set L2 cache status */
#define     PRMCOOP_CACHEENABLE           (0x0001)           /* Cache enable */
#define     PRMCOOP_CACHEDISABLE          (0x0002)          /* Cache disable */
#ifdef PMFUNCTIONS_FULL
#define  MSGCOOP_FLUSH_L2CACHE            (0x010A)
                                                           /* Flush L2 cache */
#endif /* PMFUNCTIONS_FULL */
#define  MSGCOOP_FLUSHDISABLE_L2CACHE     (0x010B)
                                               /* Flush and disable L2 cache */
#define  MSGCOOP_RESET_L2TAGRAM           (0x010C)
                                                    /* Reset L2 TAG RAM data */
#define  MSGCOOP_SAVE_CONTEXT             (0x010D)
                                                      /* Save device context */
#define  MSGCOOP_RESTORE_CONTEXT          (0x010E)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRCOOP_NOERROR                  (0x0000) /* No error                */
#define  ERRCOOP_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERRCOOP_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERRCOOP_UNKNOWN                  (0x0003) /* Unknown port setting    */

/* ************************************* */
/* *     Cooper Objects Definition     * */
/* ************************************* */
/* ------------------ */
/*  Cooper I/O class  */
/* ------------------ */
typedef struct                                          /* Cooper I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJCOOPERIO;
typedef OBJCOOPERIO  *POBJCOOPERIO;

/* ---------------------- */
/*  Cooper control class  */
/* ---------------------- */
typedef struct                                      /* Cooper control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chStorageLight;                     /* Storage light register */
   BYTE        chSystemControl;                   /* System control register */
   BYTE        chPciIntAMap;                   /* PCI INT-A mapping register */
   BYTE        chPciIntBMap;                   /* PCI INT-B mapping register */
   BYTE        chAudioSupport;                     /* Audio support register */
   BYTE        chPlanarSetupLock;       /* Planar setup lock/unlock register */
   BYTE        chScsiIdSetup;                      /* SCSI ID setup register */
   BYTE        chPcicIntMapping;          /* PCIC interrupt mapping register */
   BYTE        chFdcSupport;                         /* FDC support register */
   BYTE        chPanelIndex;                         /* Panel index register */
   BYTE        chPanelGrayLevel;            /* Gray level selection register */
   BYTE        chPanelSignalSource;
                              /* Flat panel signal source selection register */
   BYTE        chPanelBulletinBoard1;           /* Bulletin board 1 register */
   BYTE        chPanelBulletinBoard2;           /* Bulletin board 2 register */
   BYTE        chPanelBulletinBoard3;           /* Bulletin board 3 register */
   BYTE        chPanelBulletinBoard4;           /* Bulletin board 4 register */
} OBJCOOPERCTL;
typedef OBJCOOPERCTL *POBJCOOPERCTL;

/* ************************************* */
/* *          Cooper Objects           * */
/* ************************************* */
/* ---------------------- */
/*  Cooper control class  */
/* ---------------------- */
extern OBJCOOPERCTL oCooperCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDCOOPER */
/* *****************************  End of File  ***************************** */
