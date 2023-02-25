/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Mouse/Pointing-stick Device Control Routines
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
/* * Module Name: PDMOUSE.C                                                * */
/* * Description: Mouse/Pointing-stick Device Control Routines             * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDMOUSE.H/PDKMC.H/PDUMCU.H files   * */
/* *              should be included in this file                          * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdmouse.h"
#include "pdkmc.h"
#include "pdumcu.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *       Mouse Data Definition       * */
/* ************************************* */
/* ---------------- */
/*  Mouse commands  */
/* ---------------- */
#define  CMDMOUSE_REQUESTSTATUS           (0xE9)    /* Request status report */
#define  CMDMOUSE_REQUESTDEVICETYPE       (0xF2)      /* Request device type */
#define  CMDMOUSE_SETREMOTEMODE           (0xF0)          /* Set remote mode */
#define  CMDMOUSE_SETSTREAMMODE           (0xEA)          /* Set stream mode */
#define  CMDMOUSE_SETRESOLUTION           (0xE8)           /* Set resolution */
#define  CMDMOUSE_SETSAMPLINGRATE         (0xF3)        /* Set sampling rate */
#define  CMDMOUSE_SETSCALING21            (0xE7)   /* Set scaling factor 2:1 */
#define  CMDMOUSE_RESETSCALING            (0xE6)     /* Reset scaling factor */
#define  CMDMOUSE_DISABLE                 (0xF5)                  /* Disable */
#define  CMDMOUSE_ENABLE                  (0xF4)                   /* Enable */
#define  CMDMOUSE_RESET                   (0xFF)                    /* Reset */

/* ------------------------- */
/*  Pointing-stick commands  */
/* ------------------------- */
#define  CMDPOINTSTK_ID                   (0xE2)
                                                                       /* ID */
#define  CMDPOINTSTK_DISABLEMOUSECOMM     (0x40)
                                              /* Disable mouse communication */
#define  CMDPOINTSTK_ENABLEMOUSECOMM      (0x41)
                                               /* Enable mouse communication */
#define  CMDPOINTSTK_DISABLEADCONV        (0x42)
                                                    /* Disable A/D converter */
#define  CMDPOINTSTK_ENABLEADCONV         (0x43)
                                                     /* Enable A/D converter */
#define  CMDPOINTSTK_POWERDOWN            (0x44)
                                                    /* Power-down controller */

/* ------------------------ */
/*  Command acknowledgment  */
/* ------------------------ */
#define  ACKDATA_OK                       (0xFA)
                                                              /* Command ack */
#define  ACKDATA_SENDERROR                (0xFE)
                                               /* Command ack for send error */
#define  ACKDATA_RECEIVEERROR             (0xFF)
                                            /* Command ack for receive error */

/* -------------------------- */
/*  Status report bit assign  */
/* -------------------------- */
#define  BITSTATUS_RESERVED0              (0x01)
                                            /* (Bit-0) Reserved              */
#define  BITSTATUS_RESERVED1              (0x02)
                                            /* (Bit-1) Reserved              */
#define  BITSTATUS_RESERVED2              (0x04)
                                            /* (Bit-2) Reserved              */
#define  BITSTATUS_RESERVED3              (0x08)
                                            /* (Bit-3) Reserved              */
#define  BITSTATUS_SCALING21              (0x10)
                                            /* (Bit-4) Scaling factor        */
                                            /*         (0=1:1, 1=2:1)        */
#define  BITSTATUS_ENABLE                 (0x20)
                                            /* (Bit-5) Enable/Disable flag   */
                                            /*         (0=disable, 1=enable) */
#define  BITSTATUS_REMOTEMODE             (0x40)
                                            /* (Bit-6) Remote/Stream mode    */
                                            /*         (0=stream, 1=remote)  */
#define  BITSTATUS_RESERVED7              (0x80)
                                            /* (Bit-7) Reserved              */

/* ------------- */
/*  Device type  */
/* ------------- */
#define  DEVTYPE_PS2MOUSE                 (0x00)               /* PS/2 mouse */
#define  DEVTYPE_BPMOUSE                  (0x02)         /* Ball-point mouse */

/* --------------- */
/*  Sampling rate  */
/* --------------- */
#define  SAMPLRATE_BPMOUSE                (0x0A)
                                       /* Sampling rate for ball-point mouse */

/* ------------------- */
/*  BAT completion ID  */
/* ------------------- */
#define  BATCOMP_ID                       (0xAA)        /* BAT completion ID */
#define  BATFAIL_ID                       (0xFC)           /* BAT failure ID */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  DEFAULTMOUS_STATUS0              (0x00)
                                                /* Default status 0 (status) */
#define  DEFAULTMOUS_STATUS1              (0x02)
                                            /* Default status 1 (resolution) */
#define  DEFAULTMOUS_STATUS2              (0x64)
                                         /* Default status 2 (sampling rate) */
#define  DEFAULTMOUS_DEVICETYPE           (0x00)      /* Default device type */
#define  WAITTIME_BATCOMP                 (1000) /* BAT completion wait time */
                                                 /* (1000 msec = 1 sec)      */
#define  WAITTIME_RETRY                   (500)   /* Command retry wait time */
                                                  /* (500 msec)              */
#define  WAITTIME_PMCBUSY                 (2000)
                                         /* Busy time-out wait time for UMCU */
                                         /* (2000 msec = 2 sec)              */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    Mouse Controller Definition    * */
/* ************************************* */
/* --------------------- */
/*  Mouse control class  */
/* --------------------- */
VOID  MouseCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *           Mouse Objects           * */
/* ************************************* */
/* --------------------- */
/*  Mouse control class  */
/* --------------------- */
OBJMOUSECTL oMouseCtl = {(POBJCTL)MouseCtl, DEFAULTMOUS_STATUS0,
                                            DEFAULTMOUS_STATUS1,
                                            DEFAULTMOUS_STATUS2,
                                            DEFAULTMOUS_DEVICETYPE};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ********************************************** */
/* *          Mouse Control Controller          * */
/* ********************************************** */
/* *                                            * */
/* * - Initialize                               * */
/* *     [ Input ]                              * */
/* *       Message    = MSGMOUS_INITIALIZECTL   * */
/* *     [ Output ]                             * */
/* *       Result     = result code             * */
/* *                                            * */
/* * - Reset device                             * */
/* *     [ Input ]                              * */
/* *       Message    = MSGMOUS_RESET_DEVICE    * */
/* *     [ Output ]                             * */
/* *       Result     = result code             * */
/* *       Parameter1 = BAT status              * */
/* *                    (PRMMOUS_BATERROR)      * */
/* *                    (PRMMOUS_BATOK   )      * */
/* *                                            * */
/* * - Disable device                           * */
/* *     [ Input ]                              * */
/* *       Message    = MSGMOUS_DISABLE_DEVICE  * */
/* *     [ Output ]                             * */
/* *       Result     = result code             * */
/* *                                            * */
/* * - Save device context                      * */
/* *     [ Input ]                              * */
/* *       Message    = MSGMOUS_SAVE_CONTEXT    * */
/* *     [ Output ]                             * */
/* *       Result     = result code             * */
/* *                                            * */
/* * - Restore device context                   * */
/* *     [ Input ]                              * */
/* *       Message    = MSGMOUS_RESTORE_CONTEXT * */
/* *     [ Output ]                             * */
/* *       Result     = result code             * */
/* *                                            * */
/* ********************************************** */
VOID  MouseCtl(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   MSGKMC         mKmc;
   MSGUMCU        mUmcu;
   MSGMOUSE       msg;
   CRESULT        rc;
   UINT           retry, i;
   BYTE           devtype;
   CPARAMETER     cmd;
   PCPARAMETER    pparm;
   POBJMOUSECTL   pmousctl;

   /* Get pointer to Mouse control object */
   /*                                     */
   pmousctl = (POBJMOUSECTL)poObj;

   /* Control device */
   /*                */
   switch(SelectMessage(*pmMsg))
   {
      case MSGMOUS_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         rc = ERRMOUS_NOERROR;
         break;

      case MSGMOUS_RESET_DEVICE:
         /* Reset device */
         /*              */
         /* Reset pointing stick by H/W reset */
         BuildMsgWithParm1(mKmc, MSGKMC_SET_AUXSTATUS,
                                 PRMKMC_AUXENABLE);
         SendMsg(mKmc, oKmcCtl);
         BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
         SendMsg(mKmc, oKmcCtl);

         BuildMsg(mUmcu, MSGUMCU_CMD_PSDRESET);
         SendMsg(mUmcu, oUmcuCtl);
         WaitTimeCheck(WAITTIME_PMCBUSY, UmcuQueryBusy);

         /* Wait to receive the BAT completion */
         WaitTime(WAITTIME_BATCOMP);

         BuildMsgWithParm1(mKmc, MSGKMC_RECEIVE_DATA,
                                 PRMKMC_DATASIZEMAX);
         SendMsg(mKmc, oKmcCtl);

         if(!(rc = SelectResult(mKmc)))
         {
            if(SelectParm1(mKmc) >= 1)
            {
               for(i = 0, pparm = &(SelectParm2(mKmc));
                   i < SelectParm1(mKmc); i++, pparm++)
               {
                  if((*pparm >> 8) == PRMKMC_AUXILIARY)
                  {
                     if((*pparm == ((PRMKMC_AUXILIARY << 8) | BATCOMP_ID)) ||
                        (*pparm == ((PRMKMC_AUXILIARY << 8) | BATFAIL_ID))  )
                     {
                        /* Enable mouse communication */
                        for(retry = 10, rc = ERRMOUS_TIMEOUT;
                            retry && rc; retry--)
                        {
                           BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                           SendMsg(mKmc, oKmcCtl);

                           BuildMsgWithParm1(mKmc,
                                             MSGKMC_SEND_COMMANDDATAAUXACK,
                                             CMDPOINTSTK_ID);
                           SendMsg(mKmc, oKmcCtl);

                           if(!(rc = SelectResult(mKmc)))
                           {
                              BuildMsgWithParm1(mKmc,
                                                MSGKMC_SEND_COMMANDDATAAUXACK,
                                                CMDPOINTSTK_ENABLEMOUSECOMM);
                              SendMsg(mKmc, oKmcCtl);

                              rc = SelectResult(mKmc);
                           }
                           else
                           {
                              if(rc == ERRKMC_ACCESS_DENIED)
                              {
                                 rc = ERRMOUS_NOERROR;
                              }
                              else
                              {
                                 WaitTime(WAITTIME_RETRY);
                              } /* endif */
                           } /* endif */
                        } /* endfor */

                        break;
                     } /* endif */
                  } /* endif */
               } /* endfor */
            } /* endif */
         } /* endif */

         /* Reset device by S/W reset command */
         for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
         {
            BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
            SendMsg(mKmc, oKmcCtl);

            BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                    CMDMOUSE_RESET);
            SendMsg(mKmc, oKmcCtl);

            if(!(rc = SelectResult(mKmc)))
            {
               WaitTime(WAITTIME_BATCOMP);

               BuildMsgWithParm1(mKmc, MSGKMC_RECEIVE_DATA,
                                       PRMKMC_DATASIZEMAX);
               SendMsg(mKmc, oKmcCtl);

               if(!(rc = SelectResult(mKmc)))
               {
                  if(SelectParm1(mKmc) >= 1)
                  {
                     for(i = 0, pparm = &(SelectParm2(mKmc)),
                         SelectParm1(*pmMsg) = PRMMOUS_BATERROR;
                         i < SelectParm1(mKmc);
                         i++, pparm++)
                     {
                        if((*pparm >> 8) == PRMKMC_AUXILIARY)
                        {
                           rc = ERRMOUS_TIMEOUT;

                           if(*pparm == ((PRMKMC_AUXILIARY << 8) | BATCOMP_ID))
                           {
                              SelectParm1(*pmMsg) = PRMMOUS_BATOK;
                              rc = ERRMOUS_NOERROR;
                              break;
                           } /* endif */

                           if(*pparm == ((PRMKMC_AUXILIARY << 8) | BATFAIL_ID))
                           {
                              SelectParm1(*pmMsg) = PRMMOUS_BATERROR;
                              rc = ERRMOUS_NOERROR;
                              break;
                           } /* endif */
                        } /* endif */
                     } /* endfor */
                  }
                  else
                  {
                     rc = ERRMOUS_CANNOT_RECEIVED;
                  } /* endif */
               }
               else
               {
                  rc = ERRMOUS_CANNOT_RECEIVED;
                  break;
               } /* endif */
            }
            else
            {
               if(rc == ERRKMC_ACCESS_DENIED)
               {
                  rc = ERRMOUS_ACCESS_DENIED;
                  break;
               }
               else
               {
                  WaitTime(WAITTIME_RETRY);
                  rc = ERRMOUS_CANNOT_ISSUED;
               } /* endif */
            } /* endif */
         } /* endfor */

         BuildMsgWithParm1(mKmc, MSGKMC_SET_AUXSTATUS,
                                 PRMKMC_AUXDISABLE);
         SendMsg(mKmc, oKmcCtl);
         break;

      case MSGMOUS_DISABLE_DEVICE:
         /* Disable device */
         /*                */
         BuildMsgWithParm1(mKmc, MSGKMC_SET_AUXSTATUS,
                                 PRMKMC_AUXDISABLE);
         SendMsg(mKmc, oKmcCtl);

         if((rc = SelectResult(mKmc)) != ERRKMC_NOERROR)
         {
            rc = (SelectResult(mKmc) == ERRKMC_ACCESS_DENIED)
                 ? (CRESULT)ERRMOUS_ACCESS_DENIED
                 : (CRESULT)ERRMOUS_CANNOT_ISSUED;
         } /* endif */
         break;

      case MSGMOUS_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         /* Get/Save controller command byte */
         BuildMsg(mKmc, MSGKMC_SAVE_CONTEXT);
         SendMsg(mKmc, oKmcCtl);

         /* Get/Store auxiliary device status in CCB */
         BuildMsg(mKmc, MSGKMC_GET_AUXSTATUS);
         SendMsg(mKmc, oKmcCtl);

         if(!SelectResult(mKmc))
         {
            pmousctl->cpStatusCcb = SelectParm1(mKmc);
         }
         else
         {
            pmousctl->cpStatusCcb = PRMKMC_AUXDISABLE;
         } /* endif */

         /* Get/Store device type */
         for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
         {
            BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
            SendMsg(mKmc, oKmcCtl);

            BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUX,
                                    CMDMOUSE_REQUESTDEVICETYPE);
            SendMsg(mKmc, oKmcCtl);

            if(!(rc = SelectResult(mKmc)))
            {
               BuildMsgWithParm1(mKmc, MSGKMC_RECEIVE_DATA,
                                       PRMKMC_DATASIZEMAX);
               SendMsg(mKmc, oKmcCtl);

               if(!(rc = SelectResult(mKmc)))
               {
                  if((SelectParm1(mKmc) == 2)                      &&
                     (SelectParm2(mKmc) ==
                      ((PRMKMC_AUXILIARY << 8) | ACKDATA_OK))      &&
                     ((SelectParm3(mKmc) >> 8) == PRMKMC_AUXILIARY)  )
                  {
                     pmousctl->chDeviceType =
                                    (BYTE)(SelectParm3(mKmc) & PRMKMC_MASKDATA);
                  }
                  else
                  {
                     rc = ERRMOUS_TIMEOUT;
                  } /* endif */
               }
               else
               {
                  rc = ERRMOUS_NOERROR;
               } /* endif */
            }
            else
            {
               if(rc == ERRKMC_ACCESS_DENIED)
               {
                  rc = ERRMOUS_NOERROR;
               } /* endif */
            } /* endif */
         } /* endfor */

         /* Get/Store status 0-2 */
         for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
         {
            BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
            SendMsg(mKmc, oKmcCtl);

            BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUX,
                                    CMDMOUSE_REQUESTSTATUS);
            SendMsg(mKmc, oKmcCtl);

            if(!(rc = SelectResult(mKmc)))
            {
               BuildMsgWithParm1(mKmc, MSGKMC_RECEIVE_DATA,
                                       PRMKMC_DATASIZEMAX);
               SendMsg(mKmc, oKmcCtl);

               if(!(rc = SelectResult(mKmc)))
               {
                  if((SelectParm1(mKmc) == 4)                       &&
                     (SelectParm2(mKmc) ==
                      ((PRMKMC_AUXILIARY << 8) | ACKDATA_OK))       &&
                     ((SelectParm3(mKmc) >> 8) == PRMKMC_AUXILIARY) &&
                     ((SelectParm4(mKmc) >> 8) == PRMKMC_AUXILIARY) &&
                     ((SelectParm5(mKmc) >> 8) == PRMKMC_AUXILIARY)   )
                  {
                     pmousctl->chStatus0 =
                                    (BYTE)(SelectParm3(mKmc) & PRMKMC_MASKDATA);
                     pmousctl->chStatus1 =
                                    (BYTE)(SelectParm4(mKmc) & PRMKMC_MASKDATA);
                     pmousctl->chStatus2 =
                                    (BYTE)(SelectParm5(mKmc) & PRMKMC_MASKDATA);
                  }
                  else
                  {
                     rc = ERRMOUS_TIMEOUT;
                  } /* endif */
               }
               else
               {
                  rc = ERRMOUS_NOERROR;
               } /* endif */
            }
            else
            {
               if(rc == ERRKMC_ACCESS_DENIED)
               {
                  rc = ERRMOUS_NOERROR;
               } /* endif */
            } /* endif */
         } /* endfor */

         /* Restore controller command byte */
         BuildMsg(mKmc, MSGKMC_RESTORE_CONTEXT);
         SendMsg(mKmc, oKmcCtl);

         rc = ERRMOUS_NOERROR;
         break;

      case MSGMOUS_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         /* Get/Save controller command byte */
         BuildMsg(mKmc, MSGKMC_SAVE_CONTEXT);
         SendMsg(mKmc, oKmcCtl);

         /* Get device type */
         for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
         {
            BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
            SendMsg(mKmc, oKmcCtl);

            BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUX,
                                    CMDMOUSE_REQUESTDEVICETYPE);
            SendMsg(mKmc, oKmcCtl);

            if(!(rc = SelectResult(mKmc)))
            {
               BuildMsgWithParm1(mKmc, MSGKMC_RECEIVE_DATA,
                                       PRMKMC_DATASIZEMAX);
               SendMsg(mKmc, oKmcCtl);

               if(!(rc = SelectResult(mKmc)))
               {
                  if((SelectParm1(mKmc)       == 2)           &&
                     (SelectParm2(mKmc)       ==
                      ((PRMKMC_AUXILIARY << 8) | ACKDATA_OK)) &&
                     ((SelectParm3(mKmc) >> 8) == PRMKMC_AUXILIARY) )
                  {
                     devtype = (BYTE)(SelectParm3(mKmc) & PRMKMC_MASKDATA);
                  }
                  else
                  {
                     rc = ERRMOUS_TIMEOUT;
                  } /* endif */
               }
               else
               {
                  break;
               } /* endif */
            }
            else
            {
               if(rc == ERRKMC_ACCESS_DENIED)
               {
                  break;
               } /* endif */
            } /* endif */
         } /* endfor */

         if(!rc)
         {
            /* Confirm if the same auxiliary device with the saved contexts */
            /*         or the saved device context is the ball point mouse  */
            if((pmousctl->chDeviceType == devtype)      ||
               (pmousctl->chDeviceType == DEVTYPE_BPMOUSE) )
            {
               /* Setup the ball point mouse (if attached) */
               if(pmousctl->chDeviceType == DEVTYPE_BPMOUSE)
               {
                  for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
                  {
                     BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                     SendMsg(mKmc, oKmcCtl);

                     BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                             CMDMOUSE_SETSAMPLINGRATE);
                     SendMsg(mKmc, oKmcCtl);

                     if(!(rc = SelectResult(mKmc)))
                     {
                        BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                                SAMPLRATE_BPMOUSE);
                        SendMsg(mKmc, oKmcCtl);

                        rc = SelectResult(mKmc);
                     } /* endif */
                  } /* endfor */
               } /* endif */

               /* Set remote/stream mode */
               cmd = (pmousctl->chStatus0 & BITSTATUS_REMOTEMODE)
                     ? (CPARAMETER)CMDMOUSE_SETREMOTEMODE
                     : (CPARAMETER)CMDMOUSE_SETSTREAMMODE;
               for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
               {
                  BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                  SendMsg(mKmc, oKmcCtl);

                  BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                          cmd);
                  SendMsg(mKmc, oKmcCtl);

                  rc = SelectResult(mKmc);
               } /* endfor */

               /* Set scaling factor */
               cmd = (pmousctl->chStatus0 & BITSTATUS_SCALING21)
                     ? (CPARAMETER)CMDMOUSE_SETSCALING21
                     : (CPARAMETER)CMDMOUSE_RESETSCALING;
               for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
               {
                  BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                  SendMsg(mKmc, oKmcCtl);

                  BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                          cmd);
                  SendMsg(mKmc, oKmcCtl);

                  rc = SelectResult(mKmc);
               } /* endfor */

               /* Set resolution */
               for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
               {
                  BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                  SendMsg(mKmc, oKmcCtl);

                  BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                          CMDMOUSE_SETRESOLUTION);
                  SendMsg(mKmc, oKmcCtl);

                  if(!(rc = SelectResult(mKmc)))
                  {
                     BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                             pmousctl->chStatus1);
                     SendMsg(mKmc, oKmcCtl);

                     rc = SelectResult(mKmc);
                  } /* endif */
               } /* endfor */

               /* Set sampling rate */
               for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
               {
                  BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                  SendMsg(mKmc, oKmcCtl);

                  BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                          CMDMOUSE_SETSAMPLINGRATE);
                  SendMsg(mKmc, oKmcCtl);

                  if(!(rc = SelectResult(mKmc)))
                  {
                     BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                             pmousctl->chStatus2);
                     SendMsg(mKmc, oKmcCtl);

                     rc = SelectResult(mKmc);
                  } /* endif */
               } /* endfor */

               /* Set enable/disable flag */
               cmd = (pmousctl->chStatus0 & BITSTATUS_ENABLE)
                     ? (CPARAMETER)CMDMOUSE_ENABLE
                     : (CPARAMETER)CMDMOUSE_DISABLE;
               for(retry = 10, rc = ERRMOUS_TIMEOUT; retry && rc; retry--)
               {
                  BuildMsg(mKmc, MSGKMC_CLEAR_DATA);
                  SendMsg(mKmc, oKmcCtl);

                  BuildMsgWithParm1(mKmc, MSGKMC_SEND_COMMANDDATAAUXACK,
                                          cmd);
                  SendMsg(mKmc, oKmcCtl);

                  rc = SelectResult(mKmc);
               } /* endfor */
            } /* endif */
         } /* endif */

         /* Restore controller command byte */
         BuildMsg(mKmc, MSGKMC_RESTORE_CONTEXT);
         SendMsg(mKmc, oKmcCtl);

         /* Restore auxiliary device status in CCB */
         BuildMsgWithParm1(mKmc, MSGKMC_SET_AUXSTATUS,
                                 pmousctl->cpStatusCcb);
         SendMsg(mKmc, oKmcCtl);

         rc = ERRMOUS_NOERROR;
         break;

      default:
         rc = ERRMOUS_INVALID_MESSAGE;
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
