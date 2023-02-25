/* @(#)25	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pdrtc.h, pwrsysx, rspc41J, 9515B_all 4/14/95 10:55:49  */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Serialized Real Time Clocks (DS1585/1587) Chip Device
 *              Control Definitions
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
/* * Module Name: PDRTC.H                                                  * */
/* * Description: Serialized Real Time Clocks (DS1585/1587) Chip Device    * */
/* *              Control Definitions                                      * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDRTC
#define  _H_PDRTC


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      RTC Message Definition       * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                         /* RTC message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[5];                       /* Extended parameters */
} MSGRTC;
typedef MSGRTC *PMSGRTC;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* RTC I/O class */
#define  MSGRTC_INITIALIZE                (0x0000)
                                                               /* Initialize */
#define  MSGRTC_READ_RAM                  (0x0001)
                                                            /* Read user RAM */
#define  MSGRTC_READ_SECONDS              (0x0002)
                                                             /* Read seconds */
#define  MSGRTC_READ_SECONDSALARM         (0x0003)
                                                       /* Read seconds alarm */
#define  MSGRTC_READ_MINUTES              (0x0004)
                                                             /* Read minutes */
#define  MSGRTC_READ_MINUTESALARM         (0x0005)
                                                       /* Read minutes alarm */
#define  MSGRTC_READ_HOURS                (0x0006)
                                                               /* Read hours */
#define  MSGRTC_READ_HOURSALARM           (0x0007)
                                                         /* Read hours alarm */
#define  MSGRTC_READ_DAYOFWEEK            (0x0008)
                                                         /* Read day of week */
#define  MSGRTC_READ_DAYOFMONTH           (0x0009)
                                                        /* Read day of month */
#define  MSGRTC_READ_MONTH                (0x000A)
                                                               /* Read month */
#define  MSGRTC_READ_YEAR                 (0x000B)
                                                                /* Read year */
#define  MSGRTC_READ_REGISTERA            (0x000C)
                                                          /* Read register A */
#define  MSGRTC_READ_REGISTERB            (0x000D)
                                                          /* Read register B */
#define  MSGRTC_READ_REGISTERC            (0x000E)
                                                          /* Read register C */
#define  MSGRTC_READ_REGISTERD            (0x000F)
                                                          /* Read register D */
#define  MSGRTC_READ_CENTURY              (0x0010)
                                                        /* Read century byte */
#define  MSGRTC_READ_DATEALARM            (0x0011)
                                                          /* Read date alarm */
#define  MSGRTC_READ_EXTREGISTERA         (0x0012)
                                         /* Read extended control register A */
#define  MSGRTC_READ_EXTREGISTERB         (0x0013)
                                         /* Read extended control register B */
#define  MSGRTC_READ_EXTRAMADDRESSL       (0x0014)
                                            /* Read extended RAM address LSB */
#define  MSGRTC_READ_EXTRAMADDRESSM       (0x0015)
                                            /* Read extended RAM address MSB */
#define  MSGRTC_READ_EXTRAMDATA           (0x0016)
                                              /* Read extended RAM data port */
#define  MSGRTC_WRITE_RAM                 (0x0017)
                                                           /* Write user RAM */
#define  MSGRTC_WRITE_SECONDS             (0x0018)
                                                            /* Write seconds */
#define  MSGRTC_WRITE_SECONDSALARM        (0x0019)
                                                      /* Write seconds alarm */
#define  MSGRTC_WRITE_MINUTES             (0x001A)
                                                            /* Write minutes */
#define  MSGRTC_WRITE_MINUTESALARM        (0x001B)
                                                      /* Write minutes alarm */
#define  MSGRTC_WRITE_HOURS               (0x001C)
                                                              /* Write hours */
#define  MSGRTC_WRITE_HOURSALARM          (0x001D)
                                                        /* Write hours alarm */
#define  MSGRTC_WRITE_DAYOFWEEK           (0x001E)
                                                        /* Write day of week */
#define  MSGRTC_WRITE_DAYOFMONTH          (0x001F)
                                                       /* Write day of month */
#define  MSGRTC_WRITE_MONTH               (0x0020)
                                                              /* Write month */
#define  MSGRTC_WRITE_YEAR                (0x0021)
                                                               /* Write year */
#define  MSGRTC_WRITE_REGISTERA           (0x0022)
                                                         /* Write register A */
#define  MSGRTC_WRITE_REGISTERB           (0x0023)
                                                         /* Write register B */
#define  MSGRTC_WRITE_CENTURY             (0x0024)
                                                       /* Write century byte */
#define  MSGRTC_WRITE_DATEALARM           (0x0025)
                                                         /* Write date alarm */
#define  MSGRTC_WRITE_EXTREGISTERA        (0x0026)
                                        /* Write extended control register A */
#define  MSGRTC_WRITE_EXTREGISTERB        (0x0027)
                                        /* Write extended control register B */
#define  MSGRTC_WRITE_EXTRAMADDRESSL      (0x0028)
                                           /* Write extended RAM address LSB */
#define  MSGRTC_WRITE_EXTRAMADDRESSM      (0x0029)
                                           /* Write extended RAM address MSB */
#define  MSGRTC_WRITE_EXTRAMDATA          (0x002A)
                                             /* Write extended RAM data port */
#define  MSGRTC_MODIFY_RAM                (0x002B)
                                                     /* Read/Modify user RAM */
#define  MSGRTC_MODIFY_REGISTERA          (0x002C)
                                                   /* Read/Modify register A */
#define  MSGRTC_MODIFY_REGISTERB          (0x002D)
                                                   /* Read/Modify register B */
#define  MSGRTC_MODIFY_EXTREGISTERA       (0x002E)
                                  /* Read/Modify extended control register A */
#define  MSGRTC_MODIFY_EXTREGISTERB       (0x002F)
                                  /* Read/Modify extended control register B */

/* RTC control class */
#define  MSGRTC_INITIALIZECTL             (0x0100)
                                             /* Initialize for control class */
#define  MSGRTC_QUERY_UPDATE              (0x0101)
                                                 /* Query update in progress */
#define     PRMRTC_RTCREADY               (0x0001)              /* RTC ready */
#define     PRMRTC_RTCUPDATE              (0x0002) /* RTC update in progress */
#define  MSGRTC_GET_SQUAREWAVEOUTPUT      (0x0102)
                                      /* Get square wave output (SQW output) */
#define  MSGRTC_SET_SQUAREWAVEOUTPUT      (0x0103)
                                      /* Set square wave output (SQW output) */
#define     PRMRTC_SQW32768HZ             (0x0001)    /* 32,768 Hz frequency */
#define     PRMRTC_SQWDISABLE             (0x0002)      /* disable frequency */
#define  MSGRTC_SET_AUXBATTERYSTATUS      (0x0104)
                                             /* Set auxiliary battery status */
#define     PRMRTC_AUXBATENABLE           (0x0001)     /* Aux battery enable */
#define     PRMRTC_AUXBATDISABLE          (0x0002)    /* Aux battery disable */
#define  MSGRTC_SET_WAKEUPALARMSTATUS     (0x0105)
                                                 /* Set wake-up alarm status */
#define     PRMRTC_ALARMENABLE            (0x0001)   /* Wake-up alarm enable */
#define     PRMRTC_ALARMDISABLE           (0x0002)  /* Wake-up alarm disable */
#define  MSGRTC_GET_WAKEUPALARM           (0x0106)
                                                   /* Get wake-up alarm flag */
#define  MSGRTC_SET_WAKEUPALARM           (0x0107)
                                                   /* Set wake-up alarm flag */
#define     PRMRTC_ALARMON                (0x0001)       /* Wake-up alarm on */
#define     PRMRTC_ALARMOFF               (0x0002)      /* Wake-up alarm off */
#define  MSGRTC_CLEAR_INTERRUPT           (0x0108)
                                                          /* Clear interrupt */
#define  MSGRTC_DISABLE_INTERRUPT         (0x0109)
                                                    /* Disable all interrupt */
#define  MSGRTC_GET_DATETIME              (0x010A)
                                                            /* Get date/time */
#define  MSGRTC_SET_DATETIME              (0x010B)
                                                            /* Set date/time */
#define  MSGRTC_GET_ALARMDATETIME         (0x010C)
                                                      /* Get alarm date/time */
#define  MSGRTC_SET_ALARMDATETIME         (0x010D)
                                                      /* Set alarm date/time */
#define  MSGRTC_SAVE_CONTEXT              (0x010E)
                                                      /* Save device context */
#define  MSGRTC_RESTORE_CONTEXT           (0x010F)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRRTC_NOERROR                   (0x0000) /* No error                */
#define  ERRRTC_INVALID_MESSAGE           (0x0001) /* Invalid message ID      */
#define  ERRRTC_INVALID_PARAMETER         (0x0002) /* Invalid parameter value */
#define  ERRRTC_UNKNOWN                   (0x0003) /* Unknown port setting    */
#define  ERRRTC_TIMEOUT                   (0x0004) /* Time-out                */

/* ************************************* */
/* *      RTC Objects Definition       * */
/* ************************************* */
/* --------------- */
/*  RTC I/O class  */
/* --------------- */
typedef struct                                             /* RTC I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJRTCIO;
typedef OBJRTCIO  *POBJRTCIO;

/* ------------------- */
/*  RTC control class  */
/* ------------------- */
typedef struct                                         /* RTC control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chAlarmSeconds;                              /* Seconds alarm */
   BYTE        chAlarmMinutes;                              /* Minutes alarm */
   BYTE        chAlarmHours;                                  /* Hours alarm */
   BYTE        chAlarmDate;                                    /* Date alarm */
   BYTE        chRegisterA;                                    /* Register A */
   BYTE        chRegisterB;                                    /* Register B */
   BYTE        chExtRegisterA;                /* Extended control register A */
   BYTE        chExtRegisterB;                /* Extended control register B */
   BYTE        chExtRamAddressL;                 /* Extended RAM address LSB */
   BYTE        chExtRamAddressM;                 /* Extended RAM address MSB */
} OBJRTCCTL;
typedef OBJRTCCTL *POBJRTCCTL;

/* ************************************* */
/* *            RTC Objects            * */
/* ************************************* */
/* ------------------- */
/*  RTC control class  */
/* ------------------- */
extern OBJRTCCTL oRtcCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */


#endif /* _H_PDRTC */
/* *****************************  End of File  ***************************** */
