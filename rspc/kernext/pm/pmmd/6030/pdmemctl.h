/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Memory Controller Chip Device Control Definitions
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
/* * Module Name: PDMEMCTL.H                                               * */
/* * Description: Memory Controller Chip Device Control Definitions        * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDMEMCTL
#define  _H_PDMEMCTL


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *     Memory Message Definition     * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                      /* Memory message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[1];                       /* Extended parameters */
} MSGMEMORY;
typedef MSGMEMORY *PMSGMEMORY;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* Memory control class */
#define  MSGMEM_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGMEM_QUERY_CONTROLLERTYPE      (0x0101)
                                             /* Query memory controller type */
#define  MSGMEM_ENTER_SUSPEND             (0x0102)
                                                            /* Enter suspend */
#define  MSGMEM_SAVE_CONTEXT              (0x0103)
                                                      /* Save device context */
#define  MSGMEM_RESTORE_CONTEXT           (0x0104)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRMEM_NOERROR                   (0x0000) /* No error                */
#define  ERRMEM_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRMEM_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRMEM_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRMEM_NOT_INITIALIZED           (0x0004) /* Not initialized         */
#define  ERRMEM_CANNOT_INITIALIZE         (0x0005) /* Can not initialize      */

/* ************************************* */
/* *     Memory Objects Definition     * */
/* ************************************* */
/* ---------------------- */
/*  Memory control class  */
/* ---------------------- */
typedef enum                                      /* Memory controller types */
{
   ControllerIdaho,
   ControllerEagle
} MEMORYCONTROLLER, *PMEMORYCONTROLLER;

typedef struct                                      /* Memory control object */
{
   OBJCOMMON         oCommon;                               /* Common object */
   MEMORYCONTROLLER  ControllerType;               /* Memory controller type */
} OBJMEMORYCTL;
typedef OBJMEMORYCTL *POBJMEMORYCTL;

/* ************************************* */
/* *          Memory Objects           * */
/* ************************************* */
/* ---------------------- */
/*  Memory control class  */
/* ---------------------- */
extern OBJMEMORYCTL oMemoryCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDMEMCTL */
/* *****************************  End of File  ***************************** */
