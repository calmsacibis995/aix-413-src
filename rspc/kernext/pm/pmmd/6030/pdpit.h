/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Programmable Interval Timer (PIT, 8254) Device Control
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
/* * Module Name: PDPIT.H                                                  * */
/* * Description: Programmable Interval Timer (PIT, 8254) Device Control   * */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPIT
#define  _H_PDPIT


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      PIT Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* PIT message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGPIT;
typedef MSGPIT *PMSGPIT;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* PIT I/O class */
#define  MSGPIT_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGPIT_READ_COUNTER0             (0x0001)
                                                  /* Read counter 0 register */
#define  MSGPIT_READ_COUNTER1             (0x0002)
                                                  /* Read counter 1 register */
#define  MSGPIT_READ_COUNTER2             (0x0003)
                                                  /* Read counter 2 register */
#define  MSGPIT_WRITE_COUNTER0            (0x0004)
                                                 /* Write counter 0 register */
#define  MSGPIT_WRITE_COUNTER1            (0x0005)
                                                 /* Write counter 1 register */
#define  MSGPIT_WRITE_COUNTER2            (0x0006)
                                                 /* Write counter 2 register */
#define  MSGPIT_WRITE_TCW                 (0x0007)
                                        /* Write timer control word register */

/* PIT control class */
#define  MSGPIT_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGPIT_SAVE_CONTEXT              (0x0101)
                                                      /* Save device context */
#define  MSGPIT_RESTORE_CONTEXT           (0x0102)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRPIT_NOERROR                   (0x0000) /* No error                */
#define  ERRPIT_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRPIT_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRPIT_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRPIT_CANNOT_READREGISTER       (0x0004) /* Can not read registers  */

/* ************************************* */
/* *      PIT Objects Definition       * */
/* ************************************* */
/* --------------- */
/*  PIT I/O class  */
/* --------------- */
typedef struct                                             /* PIT I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJPITIO;
typedef OBJPITIO  *POBJPITIO;

/* ------------------- */
/*  PIT control class  */
/* ------------------- */
typedef struct                                         /* PIT control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chCounter0CountLow;                    /* Counter-0 count low */
   BYTE        chCounter0CountHigh;                  /* Counter-0 count high */
   BYTE        chCounter0Tcw;                /* Counter-0 timer control word */
   BYTE        chCounter1CountLow;                    /* Counter-1 count low */
   BYTE        chCounter1CountHigh;                  /* Counter-1 count high */
   BYTE        chCounter1Tcw;                /* Counter-1 timer control word */
   BYTE        chCounter2CountLow;                    /* Counter-2 count low */
   BYTE        chCounter2CountHigh;                  /* Counter-2 count high */
   BYTE        chCounter2Tcw;                /* Counter-2 timer control word */
   BYTE        chTimerBytePointer;                     /* Timer byte pointer */
} OBJPITCTL;
typedef OBJPITCTL *POBJPITCTL;

/* ************************************* */
/* *            PIT Objects            * */
/* ************************************* */
/* ------------------- */
/*  PIT control class  */
/* ------------------- */
extern OBJPITCTL oPitCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDPIT */
/* *****************************  End of File  ***************************** */
