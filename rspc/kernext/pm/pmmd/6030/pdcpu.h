/* @(#)48  1.2  src/rspc/kernext/pm/pmmd/6030/pdcpu.h, pwrsysx, rspc41J, 9517A_b 4/25/95 04:16:33
 *
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: CPU (MPC603) Chip Device Control Definitions
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
/* * Module Name: PDCPU.H                                                  * */
/* * Description: CPU (MPC603) Chip Device Control Definitions             * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDCPU
#define  _H_PDCPU


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      CPU Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* CPU message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[4];                       /* Extended parameters */
} MSGCPU;
typedef MSGCPU *PMSGCPU;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* CPU control class */
#define  MSGCPU_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGCPU_SET_DOZE                  (0x0101)
                                                            /* Set doze mode */
#define  MSGCPU_SET_NAP                   (0x0102)
                                                             /* Set nap mode */
#define  MSGCPU_SET_SLEEP                 (0x0103)
                                                           /* Set sleep mode */
#define  MSGCPU_SET_SLEEPANDSUSPEND       (0x0104)
                                               /* Set sleep mode and suspend */
#define  MSGCPU_SET_DPM                   (0x0105)
                                             /* Set dynamic power management */
#define  MSGCPU_RESET_DPM                 (0x0106)
                                           /* Reset dynamic power management */
#define  MSGCPU_SET_CLOCKX1               (0x0107)
                                                        /* Set clock rate x1 */
#define  MSGCPU_SET_CLOCKX2               (0x0108)
                                                        /* Set clock rate x2 */
#define  MSGCPU_SET_CLOCKX3               (0x0109)
                                                        /* Set clock rate x3 */
#define  MSGCPU_SET_CLOCKX4               (0x010A)
                                                        /* Set clock rate x4 */
#define  MSGCPU_CHANGE_CLOCKRATE          (0x010B)
                                                        /* Change clock rate */
#define  MSGCPU_GET_PLLCONFIGURATION      (0x010C)
                                                    /* Get PLL-configuration */
#define  MSGCPU_GET_CPUCLOCK              (0x010D)
                                                            /* Get CPU clock */
#define  MSGCPU_FLUSH_L1CACHE             (0x010E)
                                                           /* Flush L1 cache */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRCPU_NOERROR                   (0x0000) /* No error                */
#define  ERRCPU_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRCPU_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRCPU_CANNOT_CPUCONTROL         (0x0003) /* Can not control CPU     */

/* ************************************* */
/* *      CPU Objects Definition       * */
/* ************************************* */
/* ------------------- */
/*  CPU control class  */
/* ------------------- */
typedef struct                                         /* CPU control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
} OBJCPUCTL;
typedef OBJCPUCTL *POBJCPUCTL;

/* ************************************* */
/* *            CPU Objects            * */
/* ************************************* */
/* ------------------- */
/*  CPU control class  */
/* ------------------- */
extern OBJCPUCTL oCpuCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDCPU */
/* *****************************  End of File  ***************************** */
