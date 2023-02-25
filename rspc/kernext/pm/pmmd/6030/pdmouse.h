/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Mouse/Pointing-stick Device Control Definitions
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
/* * Module Name: PDMOUSE.H                                                * */
/* * Description: Mouse/Pointing-stick Device Control Definitions          * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDMOUSE
#define  _H_PDMOUSE


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     Mouse Message Definition      * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                       /* Mouse message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGMOUSE;
typedef MSGMOUSE  *PMSGMOUSE;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Mouse control class */
#define  MSGMOUS_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#define  MSGMOUS_RESET_DEVICE             (0x0101)
                                                             /* Reset device */
#define     PRMMOUS_BATERROR              (0x0001)
                                                                /* BAT error */
#define     PRMMOUS_BATOK                 (0x0002)
                                                 /* Bat ok (device detected) */
#define  MSGMOUS_DISABLE_DEVICE           (0x0102)
                                                           /* Disable device */
#define  MSGMOUS_SAVE_CONTEXT             (0x0103)
                                                      /* Save device context */
#define  MSGMOUS_RESTORE_CONTEXT          (0x0104)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRMOUS_NOERROR                  (0x0000) /* No error                */
#define  ERRMOUS_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERRMOUS_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERRMOUS_CANNOT_ISSUED            (0x0003) /* Cannot issued           */
#define  ERRMOUS_CANNOT_RECEIVED          (0x0004) /* Cannot received         */
#define  ERRMOUS_ACCESS_DENIED            (0x0005) /* Access denied           */
#define  ERRMOUS_TIMEOUT                  (0x0006) /* Time-out                */

/* ************************************* */
/* *     Mouse Objects Definition      * */
/* ************************************* */
/* --------------------- */
/*  Mouse control class  */
/* --------------------- */
typedef struct                                       /* Mouse control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   CPARAMETER  cpStatusCcb;                /* Auxiliary device status in CCB */
   BYTE        chStatus0;                               /* Status 0 (status) */
   BYTE        chStatus1;                           /* Status 1 (resolution) */
   BYTE        chStatus2;                        /* Status 2 (sampling rate) */
   BYTE        chDeviceType;                                  /* Device type */
} OBJMOUSECTL;
typedef OBJMOUSECTL  *POBJMOUSECTL;

/* ************************************* */
/* *           Mouse Objects           * */
/* ************************************* */
/* --------------------- */
/*  Mouse control class  */
/* --------------------- */
extern OBJMOUSECTL oMouseCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDMOUSE */
/* *****************************  End of File  ***************************** */
