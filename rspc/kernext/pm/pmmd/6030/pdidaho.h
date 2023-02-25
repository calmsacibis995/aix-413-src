/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: IDAHO Memory Controller Chip Device Control Definitions
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
/* * Module Name: PDIDAHO.H                                                * */
/* * Description: IDAHO Memory Controller Chip Device Control Definitions  * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDIDAHO
#define  _H_PDIDAHO


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     IDAHO Message Definition      * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                       /* IDAHO message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
   DWORD       pdwLongParm[2];                /* Extended parameters (DWORD) */
} MSGIDAHO;
typedef MSGIDAHO  *PMSGIDAHO;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* IDAHO I/O class */
#define  MSGIDHO_INITIALIZE               (0x0000)
                                                               /* Initialize */
#define  MSGIDHO_READ_RESERVEDB0          (0x0001)
                                              /* Read B0 (reserved) register */
#define  MSGIDHO_READ_REFRESHCYCLE        (0x0002)
                                   /* Read refresh cycle definition register */
#define  MSGIDHO_READ_LOCALBUSTIMER       (0x0003)
                                            /* Read local bus timer register */
#define  MSGIDHO_READ_ENABLEDETECTION1    (0x0004)
                                         /* Read enable detection 1 register */
#define  MSGIDHO_WRITE_RESERVEDB0         (0x0005)
                                             /* Write B0 (reserved) register */
#define  MSGIDHO_WRITE_REFRESHCYCLE       (0x0006)
                                  /* Write refresh cycle definition register */
#define  MSGIDHO_WRITE_LOCALBUSTIMER      (0x0007)
                                           /* Write local bus timer register */
#define  MSGIDHO_WRITE_ENABLEDETECTION1   (0x0008)
                                        /* Write enable detection 1 register */
#define  MSGIDHO_MODIFY_RESERVEDB0        (0x0009)
                                       /* Read/Modify B0 (reserved) register */
#define  MSGIDHO_MODIFY_REFRESHCYCLE      (0x000A)
                            /* Read/Modify refresh cycle definition register */
#define  MSGIDHO_MODIFY_LOCALBUSTIMER     (0x000B)
                                     /* Read/Modify local bus timer register */
#define  MSGIDHO_MODIFY_ENABLEDETECTION1  (0x000C)
                                  /* Read/Modify enable detection 1 register */
#define  MSGIDHO_MODIFY_PCICOMMAND        (0x000D)
                                         /* Read/Modify PCI command register */

/* IDAHO control class */
#define  MSGIDHO_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#define  MSGIDHO_SET_POWERMODE            (0x0101)
                                                           /* Set power mode */
#define     PRMIDHO_POWERMODEENABLE       (0x0001)      /* Power mode enable */
#define     PRMIDHO_POWERMODEDISABLE      (0x0002)     /* Power mode disable */
#define  MSGIDHO_SET_L1CACHESTATUS        (0x0102)
                                               /* Set L1 cache active status */
#define  MSGIDHO_SET_L2CACHESTATUS        (0x0103)
                                               /* Set L2 cache active status */
#define     PRMIDHO_CACHEENABLE           (0x0001)           /* Cache enable */
#define     PRMIDHO_CACHEDISABLE          (0x0002)          /* Cache disable */
#define  MSGIDHO_SET_REFRESHTYPE          (0x0104)
                                                         /* Set refresh type */
#define     PRMIDHO_REFRESHNONSTAGGERSLOW (0x0001)
                                    /* Non-staggered RAS only / Slow refresh */
#define     PRMIDHO_REFRESHSTAGGERSLOW    (0x0002)
                                        /* Staggered RAS only / Slow refresh */
#define     PRMIDHO_REFRESHCASRASSELF     (0x0003)
                                            /* CAS before RAS / Self refresh */
#define     PRMIDHO_REFRESHCASRASCASRAS   (0x0004)
                                          /* CAS before RAS / CAS before RAS */
#define  MSGIDHO_SET_BUSPARITYRESPONSE    (0x0105)
                                         /* Parity error response on PCI bus */
#define     PRMIDHO_BUSPARITYENABLE       (0x0001)          /* Parity enable */
#define     PRMIDHO_BUSPARITYDISABLE      (0x0002)         /* Parity disable */
#define  MSGIDHO_CLEAR_ERRORSTATUS        (0x0106)
                                                       /* Clear error status */
#define  MSGIDHO_ENTER_SUSPEND            (0x0107)
                                                            /* Enter suspend */
#define  MSGIDHO_SAVE_CONTEXT             (0x0108)
                                                      /* Save device context */
#define  MSGIDHO_RESTORE_CONTEXT          (0x0109)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRIDHO_NOERROR                  (0x0000) /* No error                */
#define  ERRIDHO_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERRIDHO_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERRIDHO_UNKNOWN                  (0x0003) /* Unknown port setting    */
#define  ERRIDHO_NOT_INITIALIZED          (0x0004) /* Not initialized         */
#define  ERRIDHO_CANNOT_INITIALIZE        (0x0005) /* Can not initialize      */
#define  ERRIDHO_CANNOT_PCIACCESS         (0x0006) /* Can not access PCI bus  */

/* ************************************* */
/* *     IDAHO Objects Definition      * */
/* ************************************* */
/* ----------------- */
/*  IDAHO I/O class  */
/* ----------------- */
typedef struct                                           /* IDAHO I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJIDAHOIO;
typedef OBJIDAHOIO   *POBJIDAHOIO;

/* --------------------- */
/*  IDAHO control class  */
/* --------------------- */
typedef struct                                       /* IDAHO control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       dwOffsetB0h;                          /* Offset-B0h: Reserved */
                                        /* Offset-B1h: External cache status */
                                                     /* Offset-B2h: Reserved */
                                                     /* Offset-B3h: Reserved */
   DWORD       dwOffsetB4h;          /* Offset-B4h: Refresh cycle definition */
                                                /* Offset-B5h: Refresh timer */
                                           /* Offset-B6h: RAS watchdog timer */
                                                /* Offset-B7h: PCI bus timer */
   DWORD       dwOffsetB8h;                   /* Offset-B8h: Local bus timer */
                                         /* Offset-B9h: Local bus idle timer */
                                              /* Offset-BAh: IDAHO options 1 */
                                              /* Offset-BBh: IDAHO options 2 */
} OBJIDAHOCTL;
typedef OBJIDAHOCTL  *POBJIDAHOCTL;

/* ************************************* */
/* *           IDAHO Objects           * */
/* ************************************* */
/* --------------------- */
/*  IDAHO control class  */
/* --------------------- */
extern OBJIDAHOCTL oIdahoCtl;

/* ************************************* */
/* * Object/Message Control Definition * */
/* ************************************* */
/* ------------------------ */
/*  Message control macros  */
/* ------------------------ */
#define  BuildIdahoParm1(pkt, parm1)                                  \
            *(((PMSGIDAHO)&(pkt))->pdwLongParm)     = (DWORD)(parm1);

#define  BuildIdahoParm2(pkt, parm1, parm2)                           \
            *(((PMSGIDAHO)&(pkt))->pdwLongParm)     = (DWORD)(parm1); \
            *(((PMSGIDAHO)&(pkt))->pdwLongParm + 1) = (DWORD)(parm2);

#define  SelectIdahoParm1(pkt)   *(((PMSGIDAHO)&(pkt))->pdwLongParm)

#define  SelectIdahoParm2(pkt)   *(((PMSGIDAHO)&(pkt))->pdwLongParm + 1)


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDIDAHO */
/* *****************************  End of File  ***************************** */
