/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Universal Micro Control Unit (UMCU) Chip Device Control
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
/* * Module Name: PDUMCU.H                                                 * */
/* * Description: Universal Micro Control Unit (UMCU) Chip Device Control  * */
/* *              Definitions                                              * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDUMCU
#define  _H_PDUMCU


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      UMCU Message Definition      * */
/* ************************************* */
/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                        /* UMCU message packet */
{
   MSGCOMMON   mCommon;                             /* Common message packet */
   CPARAMETER  pcpExtension[24];                      /* Extended parameters */
} MSGUMCU;
typedef MSGUMCU   *PMSGUMCU;

/* ------------ */
/*  Message ID  */
/* ------------ */
/* UMCU I/O class */
#define  MSGUMCU_INITIALIZE               (0x0000)
                                                               /* Initialize */
#define  MSGUMCU_GET_INDEX                (0x0001)
                                               /* Get index address register */
#define  MSGUMCU_SET_INDEX                (0x0002)
                                               /* Set index address register */
#define  MSGUMCU_GET_DATA                 (0x0003)
                                                        /* Get data register */
#define  MSGUMCU_SET_DATA                 (0x0004)
                                                        /* Set data register */
#define  MSGUMCU_READ_BSR                 (0x0005)
                                              /* Read basic control register */
#define  MSGUMCU_READ_BCR                 (0x0006)
                                              /* Read basic control register */
#define  MSGUMCU_READ_PCR                 (0x0007)
                                                /* Read PMC command register */
#define  MSGUMCU_READ_PSR1                (0x0008)
                                               /* Read PMC status register 1 */
#define  MSGUMCU_READ_PMMONITOR0          (0x0009)
                                               /* Read PM monitor register 0 */
#define  MSGUMCU_READ_PMCONTROL           (0x000A)
                                                 /* Read PM control register */
#define  MSGUMCU_READ_EVENTSTATUS         (0x000B)
                                               /* Read event status register */
#define  MSGUMCU_READ_EVENTENABLE         (0x000C)
                                     /* Read event interrupt enable register */
#define  MSGUMCU_WRITE_BCR                (0x000D)
                                             /* Write basic control register */
#define  MSGUMCU_WRITE_SCR                (0x000E)
                                            /* Write system command register */
#define  MSGUMCU_WRITE_SDR                (0x000F)
                                               /* Write system data register */
#define  MSGUMCU_WRITE_PMMONITOR0         (0x0010)
                                              /* Write PM monitor register 0 */
#define  MSGUMCU_WRITE_PMCONTROL          (0x0011)
                                                /* Write PM control register */
#define  MSGUMCU_WRITE_EVENTSTATUS        (0x0012)
                                              /* Write event status register */
#define  MSGUMCU_WRITE_EVENTENABLE        (0x0013)
                                    /* Write event interrupt enable register */
#define  MSGUMCU_MODIFY_BCR               (0x0014)
                                       /* Read/Modify basic control register */
#define  MSGUMCU_MODIFY_PMMONITOR0        (0x0015)
                                        /* Read/Modify PM monitor register 0 */

/* UMCU control class */
#define  MSGUMCU_INITIALIZECTL            (0x0100)
                                             /* Initialize for control class */
#ifdef PMFUNCTIONS_FULL
#define  MSGUMCU_RESET_PMC                (0x0101)
                                                      /* Reset PMC interface */
#endif /* PMFUNCTIONS_FULL */
#define  MSGUMCU_GET_PMISTATUS            (0x0102)
                                         /* Get external PM interrupt status */
#define  MSGUMCU_SET_PMISTATUS            (0x0103)
                                         /* Set external PM interrupt status */
#define     PRMUMCU_PMIENABLE             (0x0001)   /* PM interrupt enable  */
#define     PRMUMCU_PMIDISABLE            (0x0002)   /* PM interrupt disable */
#define  MSGUMCU_QUERY_PMCREADY           (0x0104)
                                                /* Query PMC ready condition */
#define     PRMUMCU_PMCREADY              (0x0001)              /* PMC ready */
#define     PRMUMCU_PMCBUSY               (0x0002)               /* PMC busy */
#define  MSGUMCU_SEND_DATA                (0x0105)
                                                      /* Send parameter data */
#define  MSGUMCU_RECEIVE_DATA             (0x0106)
                                                   /* Receive parameter data */
#define  MSGUMCU_SEND_COMMAND             (0x0107)
                                                             /* Send command */
#define  MSGUMCU_RECEIVE_COMMAND          (0x0108)
                                                          /* Receive command */
#define     PRMUMCU_COMMANDNULL           (0x0000)
                                               /* Null command (no received) */
#define  MSGUMCU_SEND_COMMANDDATA         (0x0109)
                                                    /* Send command and data */
#ifdef PMFUNCTIONS_FULL
#define  MSGUMCU_QUERY_KBDMOUSEACCESS     (0x010A)
                                       /* Query keyboard/mouse access status */
#define  MSGUMCU_CLEAR_KBDMOUSEACCESS     (0x010B)
                                       /* Clear keyboard/mouse access status */
#endif /* PMFUNCTIONS_FULL */
#define  MSGUMCU_CMD_EOPMI                (0x010C)
                                             /* Command: End of PM interrupt */
#ifdef PMFUNCTIONS_FULL
#define  MSGUMCU_CMD_POWEROFF             (0x010D)
                                                  /* Command: Main power-off */
#define  MSGUMCU_CMD_SUSPEND              (0x010E)
                                                    /* Command: Goto suspend */
#define  MSGUMCU_CMD_RESUME               (0x010F)
                                        /* Command: Resume completion notice */
#define  MSGUMCU_CMD_LED_SUSPEND          (0x0110)
                                             /* Command: Suspend LED request */
#define  MSGUMCU_CMD_LED_RESUME           (0x0111)
                                              /* Command: Resume LED request */
#define  MSGUMCU_CMD_LED_TRANSIT          (0x0112)
                     /* Command: Transition LED request (suspend <-> resume) */
#define  MSGUMCU_CMD_BEEP_POWEROFF        (0x0113)
                                          /* Command: Power-off beep request */
#define  MSGUMCU_CMD_BEEP_SUSPEND         (0x0114)
                                            /* Command: Suspend beep request */
#define  MSGUMCU_CMD_BEEP_RESUME          (0x0115)
                                             /* Command: Resume beep request */
#define  MSGUMCU_CMD_BEEP_HIBERNATE       (0x0116)
                                        /* Command: Hibernation beep request */
#define  MSGUMCU_CMD_BEEP_ACINOUT         (0x0117)
                                   /* Command: AC attach/detach beep request */
#define  MSGUMCU_CMD_BEEP_WARNING         (0x0118)
                                            /* Command: Warning beep request */
#define  MSGUMCU_CMD_BEEP_WARNINGREPEAT   (0x0119)
                               /* Command: Warning beep request (repeatedly) */
#define  MSGUMCU_CMD_BEEP_STOP            (0x011A)
                                               /* Command: Beep stop request */
#endif /* PMFUNCTIONS_FULL */
#define  MSGUMCU_CMD_RESET_EVENT          (0x011B)
                                                /* Command: Reset all events */
#define  MSGUMCU_CMD_START_EVENT          (0x011C)
                                   /* Command: Start all events notification */
#define  MSGUMCU_CMD_CLEAR_EVENT          (0x011D)
                                                /* Command: Clear all events */
#define  MSGUMCU_CMD_PWREVENT_POWEROFF    (0x011E)
                          /* Command: Power-off event request (power switch) */
#define  MSGUMCU_CMD_LIDEVENT_SUSPEND     (0x011F)
                                     /* Command: Suspend event request (LID) */
#define  MSGUMCU_CMD_BATEVENT_SUSPEND     (0x0120)
                        /* Command: Suspend event request (critical battery) */
#define  MSGUMCU_CMD_PWREVENT_RESUME      (0x0121)
                             /* Command: Resume event request (power switch) */
#define  MSGUMCU_CMD_LIDEVENT_RESUME      (0x0122)
                                      /* Command: Resume event request (LID) */
#define  MSGUMCU_CMD_KBDEVENT_RESUME      (0x0123)
                                 /* Command: Resume event request (keyboard) */
#define  MSGUMCU_CMD_PSDEVENT_RESUME      (0x0124)
                           /* Command: Resume event request (pointing-stick) */
#define  MSGUMCU_CMD_BATEVENT_RESUME      (0x0125)
                         /* Command: Resume event request (critical battery) */
#define  MSGUMCU_CMD_CVREVENT_NOTIFY      (0x0126)
                /* Command: Notify event request (keyboard-cover open/close) */
#define  MSGUMCU_CMD_KBDEVENT_NOTIFY      (0x0127)
                     /* Command: Notify event request (keyboard plug-in/out) */
#define  MSGUMCU_CMD_ACEVENT_NOTIFY       (0x0128)
                          /* Command: Notify event request (AC power change) */
#define  MSGUMCU_CMD_PWREVENT_TIMER       (0x0129)
                                    /* Command: Power-off switch event timer */
#define     PRMUMCU_EVENTENABLE           (0x0001)
                                                  /* Event enable (option 1) */
#define     PRMUMCU_EVENTDISABLE          (0x0002)
                                                 /* Event disable (option 1) */
#define     PRMUMCU_EVENTNORMAL           (0x0001)
                                                  /* Event normal (option 2) */
#define     PRMUMCU_EVENTRESET            (0x0002)
                                                   /* Event reset (option 2) */
#define  MSGUMCU_CMD_PSDRESET             (0x012A)
                                     /* Command: Pointing-stick device reset */
#ifdef PMFUNCTIONS_FULL
#define  MSGUMCU_CMD_QUERY_BATSTATUS      (0x012B)
                                            /* Command: Query battery status */
#define  MSGUMCU_CMD_QUERY_BATTOTALCAPA   (0x012C)
                                            /* Command: Query total capacity */
#define  MSGUMCU_CMD_QUERY_BATCURRENTCAPA (0x012D)
                                          /* Command: Query current capacity */
#define  MSGUMCU_CMD_QUERY_BATDISCCAPA    (0x012E)
                                      /* Command: Query discharging capacity */
#define  MSGUMCU_CMD_QUERY_BATDISCTIME    (0x012F)
                                          /* Command: Query discharging time */
#define  MSGUMCU_CMD_QUERY_RESUMEEVENT    (0x0130)
                                              /* Command: Query resume event */
#define  MSGUMCU_CMD_CONTROL_BATTERY      (0x0131)
                                                 /* Command: Control battery */
#define     PRMUMCU_BATDISCHARGESTART     (0x0001)
                                       /* Start battery discharge (option 1) */
#define     PRMUMCU_BATDISCHARGESTOP      (0x0002)
                                       /* Stop battery discharge  (option 1) */
#endif /* PMFUNCTIONS_FULL */
#define  MSGUMCU_SAVE_CONTEXT             (0x0132)
                                                      /* Save device context */
#define  MSGUMCU_RESTORE_CONTEXT          (0x0133)
                                                   /* Restore device context */

/* ------------- */
/*  Result code  */
/* ------------- */
#define  ERRUMCU_NOERROR                  (0x0000) /* No error                */
#define  ERRUMCU_INVALID_MESSAGE          (0x0001) /* Invalid message ID      */
#define  ERRUMCU_INVALID_PARAMETER        (0x0002) /* Invalid parameter value */
#define  ERRUMCU_UNKNOWN                  (0x0003) /* Unknown port setting    */
#define  ERRUMCU_TIMEOUT                  (0x0004) /* Time-out                */

/* ************************************* */
/* *      UMCU Objects Definition      * */
/* ************************************* */
/* ---------------- */
/*  UMCU I/O class  */
/* ---------------- */
typedef struct                                            /* UMCU I/O object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   DWORD       BaseAddress;                           /* Device base address */
} OBJUMCUIO;
typedef OBJUMCUIO *POBJUMCUIO;

/* -------------------- */
/*  UMCU control class  */
/* -------------------- */
typedef struct                                        /* UMCU control object */
{
   OBJCOMMON   oCommon;                                     /* Common object */
   BYTE        chIndex;                            /* Index address register */
   BYTE        chBcr;                              /* Basic control register */
   BYTE        chPmMonitor0;                        /* PM monitor register 0 */
   BYTE        chPmControl;                           /* PM control register */
   BYTE        chEventStatus;                       /* Event status register */
   BYTE        chEventEnable;             /* Event interrupt enable register */
   BYTE        chOnLineParm1PowerMonitor; /* Power monitor object parameter1 */
   BYTE        chOnLineParm2PowerMonitor; /* Power monitor object parameter2 */
   BYTE        chResumeCommand;                     /* PMC command at resume */
   BYTE        chResumeData0;                        /* PMC data 0 at resume */
   BYTE        chResumeData1;                        /* PMC data 1 at resume */
   BYTE        chResumeData2;                        /* PMC data 2 at resume */
   BYTE        chResumeData3;                        /* PMC data 3 at resume */
} OBJUMCUCTL;
typedef OBJUMCUCTL   *POBJUMCUCTL;

/* ************************************* */
/* *           UMCU Objects            * */
/* ************************************* */
/* -------------------- */
/*  UMCU control class  */
/* -------------------- */
extern OBJUMCUCTL oUmcuCtl;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *           UMCU Routine            * */
/* ************************************* */
/* ------------ */
/*  Busy query  */
/* ------------ */
CRESULT  UmcuQueryBusy(VOID);


#endif /* _H_PDUMCU */
/* *****************************  End of File  ***************************** */
