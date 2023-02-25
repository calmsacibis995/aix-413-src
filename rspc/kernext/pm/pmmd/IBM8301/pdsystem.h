/* @(#)27	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pdsystem.h, pwrsysx, rspc41J, 9515B_all 4/14/95 10:55:53  */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: System Power State Control Definitions
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
/* * Module Name: PDSYSTEM.H                                               * */
/* * Description: System Power State Control Definitions                   * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDSYSTEM
#define  _H_PDSYSTEM


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ***************************************** */
/* * System Power State Control Definition * */
/* ***************************************** */
/* -------------------------------------------- */
/*  Alarm date/time structure for alarm resume  */
/* -------------------------------------------- */
typedef struct                                  /* Alarm date/time structure */
{
   BYTE  chSeconds;                              /* Seconds alarm (BCD 0-59) */
   BYTE  chMinutes;                              /* Minutes alarm (BCD 0-59) */
   BYTE  chHours;                                  /* Hours alarm (BCD 0-23) */
   BYTE  chDay;                                      /* Day alarm (BCD 1-31) */
   BYTE  chMonth;                                  /* Month alarm (BCD 1-12) */
   WORD  wYear;                                   /* Year alarm (BCD 0-9999) */
} ALARMDATETIME;
typedef ALARMDATETIME   *PALARMDATETIME;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* *************************************** */
/* * System Power State Control Routines * */
/* *************************************** */
/* --------------------------- */
/*  System initialize control  */
/* --------------------------- */
VOID  PmSystemInitialize(VOID);

/* ------------------------ */
/*  System suspend control  */
/* ------------------------ */
VOID  PmSystemSuspend(PALARMDATETIME   pAlarm);
                                     /* Pointer to Alarm date/time structure */

/* ----------------------- */
/*  System resume control  */
/* ----------------------- */
VOID  PmSystemResume(VOID);


#endif /* _H_PDSYSTEM */
/* *****************************  End of File  ***************************** */
