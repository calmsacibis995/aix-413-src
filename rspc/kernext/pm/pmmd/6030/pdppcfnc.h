/* @(#)77  1.2  src/rspc/kernext/pm/pmmd/6030/pdppcfnc.h, pwrsysx, rspc41J, 9517A_b 4/25/95 04:17:33
 *
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Miscellaneous Assembly Definitions
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
/* * Module Name: PDPPCFNC.H                                               * */
/* * Description: Miscellaneous Assembly Definitions (for PowerPC 601/603) * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPPCFNC
#define  _H_PDPPCFNC


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ***************************************** */
/* * External Interrupt Control Definition * */
/* ***************************************** */
/* ---------------------------------- */
/*  External interrupt enable status  */
/* ---------------------------------- */
#define  EXTERNALINT_ON                   (0x0001)       /* Interrupt enable */
#define  EXTERNALINT_OFF                  (0x0000)      /* Interrupt disable */

/* *************************************** */
/* * Power Management Control Definition * */
/* *************************************** */
/* -------------------------------- */
/*  Power management enable status  */
/* -------------------------------- */
#define  POWERMGT_ON                      (0x0001)
                                                  /* Power management enable */
#define  POWERMGT_OFF                     (0x0000)
                                                 /* Power management disable */

/* ------------------------------- */
/*  Power saving modes bit assign  */
/* ------------------------------- */
#define  BITPOWERMODE_SLEEP               (0x0001)     /* (Bit-0) Sleep mode */
#define  BITPOWERMODE_NAP                 (0x0002)     /* (Bit-1) Nap mode   */
#define  BITPOWERMODE_DOZE                (0x0004)     /* (Bit-2) Doze mode  */

/* ---------------------------------------- */
/*  Dynamic power management enable status  */
/* ---------------------------------------- */
#define  DPM_ON                           (0x0001)
                                          /* Dynamic power management enable */
#define  DPM_OFF                          (0x0000)
                                         /* Dynamic power management disable */

/* **************************************** */
/* * Processor Version Control Definition * */
/* **************************************** */
/* ------------------- */
/*  Processor version  */
/* ------------------- */
#define  CPUVERSION_MPC601                (0x0001)  /* MPC601 version number */
#define  CPUVERSION_MPC603                (0x0003)  /* MPC603 version number */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* *************************************** */
/* * External Interrupt Control Routines * */
/* *************************************** */
/* ------------------------------------------- */
/*  Read external interrupt enable bit in MSR  */
/* ------------------------------------------- */
UINT  ReadMsrEe(VOID);

/* ------------------------------------------ */
/*  Set external interrupt enable bit in MSR  */
/* ------------------------------------------ */
VOID  SetMsrEe(VOID);

/* -------------------------------------------- */
/*  Reset external interrupt enable bit in MSR  */
/* -------------------------------------------- */
VOID  ResetMsrEe(VOID);

/* ************************************* */
/* * Power Management Control Routines * */
/* ************************************* */
/* ----------------------------------------- */
/*  Read power management enable bit in MSR  */
/* ----------------------------------------- */
UINT  ReadMsrPow(VOID);

/* ---------------------------------------- */
/*  Set power management enable bit in MSR  */
/* ---------------------------------------- */
VOID  SetMsrPow(VOID);
VOID  SetMsrPowWithOutport(UINT Port, BYTE Value);

/* ------------------------------------------ */
/*  Reset power management enable bit in MSR  */
/* ------------------------------------------ */
VOID  ResetMsrPow(VOID);

/* ------------------------------------- */
/*  Read power saving mode bits in hid0  */
/* ------------------------------------- */
UINT  ReadHid0Power(VOID);

/* ------------------------------------ */
/*  Set power saving mode bits in hid0  */
/* ------------------------------------ */
VOID  SetHid0Power(UINT Mode);                                 /* Power mode */

/* ------------------------------------------- */
/*  Read dynamic power management bit in hid0  */
/* ------------------------------------------- */
UINT  ReadHid0Dpm(VOID);

/* ------------------------------------------ */
/*  Set dynamic power management bit in hid0  */
/* ------------------------------------------ */
VOID  SetHid0Dpm(VOID);

/* -------------------------------------------- */
/*  Reset dynamic power management bit in hid0  */
/* -------------------------------------------- */
VOID  ResetHid0Dpm(VOID);

/* ********************************************* */
/* * Real-Time Clock Register Control Routines * */
/* ********************************************* */
/* ------------------------------------- */
/*  Read real-time clock upper register  */
/* ------------------------------------- */
UINT  ReadRtcu(VOID);

/* ------------------------------------- */
/*  Read real-time clock lower register  */
/* ------------------------------------- */
UINT  ReadRtcl(VOID);

/* ************************************** */
/* * Processor Version Control Routines * */
/* ************************************** */
/* ------------------------ */
/*  Read processor version  */
/* ------------------------ */
UINT  ReadCpuVersion(VOID);

/* ************************************* */
/* *       I/O Control Routines        * */
/* ************************************* */
/* --------------------------------- */
/*  Read from I/O port (Byte order)  */
/* --------------------------------- */
BYTE  inp(UINT Port);                                    /* I/O port address */

/* -------------------------------- */
/*  Write to I/O port (Byte order)  */
/* -------------------------------- */
VOID  outp(UINT   Port,                                  /* I/O port address */
           BYTE   Value);                      /* Value to write to I/O port */

/* --------------------------------- */
/*  Read from I/O port (Word order)  */
/* --------------------------------- */
WORD  inpw(UINT   Port);                                 /* I/O port address */

/* -------------------------------- */
/*  Write to I/O port (Word order)  */
/* -------------------------------- */
VOID  outpw(UINT  Port,                                  /* I/O port address */
            WORD  Value);                      /* Value to write to I/O port */

/* ---------------------------------- */
/*  Read from I/O port (DWord order)  */
/* ---------------------------------- */
DWORD inpdw(UINT  Port);                                 /* I/O port address */

/* --------------------------------- */
/*  Write to I/O port (DWord order)  */
/* --------------------------------- */
VOID  outpdw(UINT    Port,                               /* I/O port address */
             DWORD   Value);                   /* Value to write to I/O port */

/* ----------------------------------- */
/*  Flush L1 cache (instruction/data)  */
/* ----------------------------------- */
VOID  FlushL1Cache(VOID);

/* ------------- */
/*  Synchronize  */
/* ------------- */
VOID  Sync(VOID);

/* --------------------- */
/*  Disable Decrementer  */
/* --------------------- */
VOID  DisableDecrementer(VOID);


#endif /* _H_PDPPCFNC */
/* *****************************  End of File  ***************************** */
