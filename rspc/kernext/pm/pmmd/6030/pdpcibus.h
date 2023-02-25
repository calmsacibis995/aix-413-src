/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Peripheral Component Interconnect (PCI) Bus Control
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
/* * Module Name: PDPCIBUS.H                                               * */
/* * Description: Peripheral Component Interconnect (PCI) Bus Control      * */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPCIBUS
#define  _H_PDPCIBUS


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      PCI Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* PCI message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[2];                       /* Extended parameters */
   DWORD       pdwLongParm[2];                /* Extended parameters (DWORD) */
} MSGPCI;
typedef MSGPCI *PMSGPCI;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* PCI I/O class */
#define  MSGPCI_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGPCI_GET_ADDRESS               (0x0001)
                                       /* Get configuration address register */
#define  MSGPCI_SET_ADDRESS               (0x0002)
                                       /* Set configuration address register */
#define  MSGPCI_FIND_DEVICE               (0x0003)
                                                        /* Find a PCI device */
#define  MSGPCI_READ_CONFIGDATA           (0x0004)
                                            /* Read configuration space data */
#define  MSGPCI_READ_COMMAND              (0x0005)
                                  /* Read PCI command in configuration space */
#define  MSGPCI_READ_BASEADDRESS0         (0x0006)
                               /* Read base address 0 in configuration space */
#define  MSGPCI_READ_BASEADDRESS1         (0x0007)
                               /* Read base address 1 in configuration space */
#define  MSGPCI_READ_REVISIONID           (0x0008)
                                                         /* Read revision ID */
#define  MSGPCI_WRITE_CONFIGDATA          (0x0009)
                                           /* Write configuration space data */
#define  MSGPCI_WRITE_COMMAND             (0x000A)
                               /* Write PCI command into configuration space */
#define  MSGPCI_WRITE_BASEADDRESS0        (0x000B)
                            /* Write base address 0 into configuration space */
#define  MSGPCI_WRITE_BASEADDRESS1        (0x000C)
                            /* Write base address 1 into configuration space */

/* PCI control class */
#define  MSGPCI_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGPCI_GET_CONFIGADDRESS         (0x0101)
                                          /* Get configuration space address */
#define  MSGPCI_GET_DEVICENUMBER          (0x0102)
                                          /* Get configuration device number */
#define  MSGPCI_GET_DEVICEID              (0x0103)
                                                        /* Get PCI device ID */
#define  MSGPCI_GET_BASEADDRESS0          (0x0104)
                                                       /* Get base address 0 */
#define  MSGPCI_GET_BASEADDRESS1          (0x0105)
                                                       /* Get base address 1 */
#define  MSGPCI_GET_REVISIONID            (0x0106)
                                                          /* Get revision ID */
#define  MSGPCI_GET_CONFIGDATA            (0x0107)
                                             /* Get configuration space data */
#define  MSGPCI_SET_CONFIGDATA            (0x0108)
                                             /* Set configuration space data */
#define  MSGPCI_SAVE_CONTEXT              (0x0109)
                                                      /* Save device context */
#define  MSGPCI_RESTORE_CONTEXT           (0x010A)
                                                   /* Restore device context */
#define  MSGPCI_QUERY_DEVICE              (0x010B)
                                                    /* Query device existing */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRPCI_NOERROR                   (0x0000) /* No error                */
#define  ERRPCI_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRPCI_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRPCI_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRPCI_DEVICE_NOT_FOUND          (0x0004)
                                                /* Specified device not found */

/* ************************************* */
/* *      PCI Objects Definition       * */
/* ************************************* */
/* --------------- */
/*  PCI I/O class  */
/* --------------- */
typedef struct                                             /* PCI I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
   DWORD       ConfigSpaceBaseAddress;        /* Configuration space address */
} OBJPCIIO;
typedef OBJPCIIO  *POBJPCIIO;

/* ------------------- */
/*  PCI control class  */
/* ------------------- */
typedef struct                                         /* PCI control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   WORD        wVendorID;                                       /* Vendor ID */
   WORD        wDeviceID;                                       /* Device ID */
   DWORD       dwConfigAddress;               /* Configuration space address */
   DWORD       dwAddress;                     /* Configuration space address */
   DWORD       dwOffset04h;                       /* Offset-04h: PCI command */
                                            /* Offset-06h: PCI device status */
   DWORD       dwOffset0Ch;                        /* Offset-0Ch: Cache line */
                                                      /* Offset-0Dh: Latency */
                                                  /* Offset-0Eh: Header type */
                                                         /* Offset-0Fh: BIST */
   DWORD       dwOffset10h;                    /* Offset-10h: Base address 0 */
   DWORD       dwOffset14h;                    /* Offset-14h: Base address 1 */
   DWORD       dwOffset3Ch;                    /* Offset-3Ch: Interrupt line */
                                                /* Offset-3Dh: Interrupt pin */
                                                      /* Offset-3Eh: Min_Gnt */
                                                      /* Offset-3Fh: Max_Lat */
} OBJPCICTL;
typedef OBJPCICTL *POBJPCICTL;

/* ************************************* */
/* *            PCI Objects            * */
/* ************************************* */
/* ------------------- */
/*  PCI control class  */
/* ------------------- */
extern OBJPCICTL oPciIdahoCtl;
extern OBJPCICTL oPciEagleCtl;
extern OBJPCICTL oPciSystemioCtl;
extern OBJPCICTL oPciScsiCtl;
extern OBJPCICTL oPciWd90c24Ctl;
extern OBJPCICTL oPciS386c928Ctl;
extern OBJPCICTL oPciS386c864Ctl;
extern OBJPCICTL oPciWeitekP9100Ctl;
extern OBJPCICTL oPciCarreraCtl;
extern OBJPCICTL oPciCaptureCtl;

/* ************************************* */
/* * Object/Message Control Definition * */
/* ************************************* */
/* ------------------------ */
/*  Message control macros  */
/* ------------------------ */
#define  BuildPciParm1(pkt, parm1)                                  \
            *(((PMSGPCI)&(pkt))->pdwLongParm)     = (DWORD)(parm1);

#define  BuildPciParm2(pkt, parm1, parm2)                           \
            *(((PMSGPCI)&(pkt))->pdwLongParm)     = (DWORD)(parm1); \
            *(((PMSGPCI)&(pkt))->pdwLongParm + 1) = (DWORD)(parm2);

#define  SelectPciParm1(pkt)  *(((PMSGPCI)&(pkt))->pdwLongParm)

#define  SelectPciParm2(pkt)  *(((PMSGPCI)&(pkt))->pdwLongParm + 1)


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDPCIBUS */
/* *****************************  End of File  ***************************** */
