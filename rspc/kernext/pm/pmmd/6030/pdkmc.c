/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Keyboard/Mouse Controller (8042) Chip Device Control
 *              Routines
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
/* * Module Name: PDKMC.C                                                  * */
/* * Description: Keyboard/Mouse Controller (8042) Chip Device Control     * */
/* *              Routines                                                 * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDKMC.H files should be included in* */
/* *              this file                                                * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdkmc.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *        KMC Data Definition        * */
/* ************************************* */
/* --------------------------- */
/*  KMC register port address  */
/* --------------------------- */
#define  PORTKMC_COMMAND                  (0x00000064L)  /* Command register */
#define  PORTKMC_STATUS                   (0x00000064L)   /* Status register */
#define  PORTKMC_DATA                     (0x00000060L)     /* Data register */

/* ------------------------------------- */
/*  Command register (COMMAND) commands  */
/* ------------------------------------- */
#define  CMDKMCCOMMAND_READCCB            (0x20)
                                             /* Read controller command byte */
#define  CMDKMCCOMMAND_WRITECCB           (0x60)
                                            /* Write controller command byte */
#define  CMDKMCCOMMAND_WRITEAUX           (0xD4)
                                                  /* Write auxiliary command */
#define  CMDKMCCOMMAND_AUXDISABLE         (0xA7)
                                       /* Auxiliary device interface disable */
#define  CMDKMCCOMMAND_AUXENABLE          (0xA8)
                                        /* Auxiliary device interface enable */
#define  CMDKMCCOMMAND_KBDDISABLE         (0xAD)
                                               /* Keyboard interface disable */
#define  CMDKMCCOMMAND_KBDENABLE          (0xAE)
                                                /* Keyboard interface enable */

/* ------------------------------------- */
/*  Status register (STATUS) bit assign  */
/* ------------------------------------- */
#define  BITKMCSTATUS_OBF                 (0x01)
                                     /* (Bit-0) Output buffer full           */
                                     /*         (1=full)                     */
#define  BITKMCSTATUS_IBF                 (0x02)
                                     /* (Bit-1) Input buffer full            */
                                     /*         (1=full)                     */
#define  BITKMCSTATUS_SYSFLAG             (0x04)
                                     /* (Bit-2) System flag                  */
#define  BITKMCSTATUS_CMDDATA             (0x08)
                                     /* (Bit-3) Command/Data                 */
#define  BITKMCSTATUS_INHIBIT             (0x10)
                                     /* (Bit-4) Inhibit switch               */
                                     /*         (0=inhibit, password state)  */
#define  BITKMCSTATUS_OBFAUX              (0x20)
                                     /* (Bit-5) Auxiliary output buffer full */
#define  BITKMCSTATUS_GENERALTIMEOUT      (0x40)
                                     /* (Bit-6) General time-out             */
#define  BITKMCSTATUS_PARITYERROR         (0x80)
                                     /* (Bit-7) Parity error                 */

/* ------------------------------------- */
/*  Data register (DATA) acknowledgment  */
/* ------------------------------------- */
#define  ACKKMCDATA_OK                    (0xFA)
                                                              /* Command ack */
#define  ACKKMCDATA_SENDERROR             (0xFE)
                                               /* Command ack for send error */
#define  ACKKMCDATA_RECEIVEERROR          (0xFF)
                                            /* Command ack for receive error */

/* ------------------------------------------ */
/*  controller command byte (CCB) bit assign  */
/* ------------------------------------------ */
#define  BITKMCCCB_ENABLEKBDINT           (0x01)
                                       /* (Bit-0) Enable keyboard interrupt  */
#define  BITKMCCCB_ENABLEAUXINT           (0x02)
                                       /* (Bit-1) Enable auxiliary interrupt */
#define  BITKMCCCB_SYSFLAG                (0x04)
                                       /* (Bit-2) System flag                */
#define  BITKMCCCB_RESERVED3              (0x08)
                                       /* (Bit-3) Reserved                   */
#define  BITKMCCCB_DISABLEKBD             (0x10)
                                       /* (Bit-4) Disable keyboard           */
#define  BITKMCCCB_DISABLEAUX             (0x20)
                                       /* (Bit-5) Disable auxiliary device   */
#define  BITKMCCCB_KBDTRANS               (0x40)
                                       /* (Bit-6) Keyboard translate         */
#define  BITKMCCCB_RESERVED7              (0x80)
                                       /* (Bit-7) Reserved                   */

/* -------------------------- */
/*  Miscellaneous definition  */
/* -------------------------- */
#define  DEFAULTKMC_CCB                   (0x07)
                                          /* Default controller command byte */
#define  WAITTIME_KMCBUSY                 (2)     /* Busy time-out wait time */
                                                  /* (2 msec)                */
#define  WAITTIME_DATAREADY               (2000)
                                    /* Receive data ready time-out wait time */
                                    /* (2000 msec = 2 sec)                   */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     KMC Controller Definition     * */
/* ************************************* */
/* --------------- */
/*  KMC I/O class  */
/* --------------- */
VOID  KmcIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj);                  /* Pointer to Common object */

/* ------------------- */
/*  KMC control class  */
/* ------------------- */
VOID  KmcCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            KMC Objects            * */
/* ************************************* */
/* --------------- */
/*  KMC I/O class  */
/* --------------- */
OBJKMCIO    oKmcIo  = {(POBJCTL)KmcIo, PORTISA_BASE};

/* ------------------- */
/*  KMC control class  */
/* ------------------- */
OBJKMCCTL   oKmcCtl = {(POBJCTL)KmcCtl, DEFAULTKMC_CCB};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* **************************************** */
/* *          KMC I/O Controller          * */
/* **************************************** */
/* *                                      * */
/* * - Initialize                         * */
/* *     [ Input ]                        * */
/* *       Message    = MSGKMC_INITIALIZE * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* * - Read register                      * */
/* *     [ Input ]                        * */
/* *       Message    = MSGKMC_READ_xxx   * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *       Parameter1 = register value    * */
/* *                                      * */
/* * - Write register                     * */
/* *     [ Input ]                        * */
/* *       Message    = MSGKMC_WRITE_xxx  * */
/* *       Parameter1 = register value    * */
/* *     [ Output ]                       * */
/* *       Result     = result code       * */
/* *                                      * */
/* **************************************** */
VOID  KmcIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj)                   /* Pointer to Common object */
{
   CRESULT     rc;
   POBJKMCIO   pkmcio;
   DWORD       baseport;

   /* Get pointer to KMC I/O object */
   /*                               */
   pkmcio = (POBJKMCIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = GetIsaBaseAddress();

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGKMC_INITIALIZE:
         /* Initialize */
         /*            */
         pkmcio->BaseAddress = GetIsaBaseAddress();
         rc = ERRKMC_NOERROR;
         break;

      case MSGKMC_READ_STATUS:
         /* Read status register */
         /*                      */
         SelectParm1(*pmMsg) =
                      (CPARAMETER)ReadControllerByte(baseport + PORTKMC_STATUS);
         rc = ERRKMC_NOERROR;
         break;

      case MSGKMC_READ_DATA:
         /* Read data register */
         /*                    */
         SelectParm1(*pmMsg) =
                        (CPARAMETER)ReadControllerByte(baseport + PORTKMC_DATA);
         SelectParm1(*pmMsg) =
                        (CPARAMETER)ReadControllerByte(baseport + PORTKMC_DATA);
         rc = ERRKMC_NOERROR;
         break;

      case MSGKMC_WRITE_COMMAND:
         /* Write command register */
         /*                        */
         WriteControllerByte(baseport + PORTKMC_COMMAND,
                             (BYTE)SelectParm1(*pmMsg));
         rc = ERRKMC_NOERROR;
         break;

      case MSGKMC_WRITE_DATA:
         /* Write data register */
         /*                     */
         WriteControllerByte(baseport + PORTKMC_DATA,
                             (BYTE)SelectParm1(*pmMsg));
         rc = ERRKMC_NOERROR;
         break;

      default:
         rc = ERRKMC_INVALID_MESSAGE;
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* **************************************** */
/* * KMC IBF Busy Query Call-back Routine * */
/* **************************************** */
/* *                                      * */
/* *  [ Input  ] none                     * */
/* *  [ Output ] busy  = no zero          * */
/* *             ready = zero             * */
/* *                                      * */
/* **************************************** */
CRESULT  KmcQueryIbfBusy(VOID)
{
   MSGKMC   msg;
   CRESULT  rc;

   BuildMsg(msg, MSGKMC_QUERY_IBF);
   SendMsg(msg, oKmcCtl);

   if(!(rc = SelectResult(msg)))
   {
      rc = (SelectParm1(msg) == PRMKMC_EMPTY)
           ? (CRESULT)ERRKMC_NOERROR
           : (CRESULT)ERRKMC_TIMEOUT;
   } /* endif */

   return(rc);
}

/* ******************************************** */
/* * KMC IBF/OBF Busy Query Call-back Routine * */
/* ******************************************** */
/* *                                          * */
/* *  [ Input  ] none                         * */
/* *  [ Output ] busy  = no zero              * */
/* *             ready = zero                 * */
/* *                                          * */
/* ******************************************** */
CRESULT  KmcQueryIbfObfBusy(VOID)
{
   MSGKMC   msg;
   CRESULT  rc;

   BuildMsg(msg, MSGKMC_QUERY_IBFOBF);
   SendMsg(msg, oKmcCtl);

   if(!(rc = SelectResult(msg)))
   {
      rc = (SelectParm1(msg) == PRMKMC_EMPTY)
           ? (CRESULT)ERRKMC_NOERROR
           : (CRESULT)ERRKMC_TIMEOUT;
   } /* endif */

   return(rc);
}

/* ***************************************** */
/* * KMC OBF Ready Query Call-back Routine * */
/* ***************************************** */
/* *                                       * */
/* *  [ Input  ] none                      * */
/* *  [ Output ] ready = no zero           * */
/* *             busy  = zero              * */
/* *                                       * */
/* ***************************************** */
CRESULT  KmcQueryObfReady(VOID)
{
   MSGKMC   msg;
   CRESULT  rc;

   BuildMsg(msg, MSGKMC_QUERY_OBF);
   SendMsg(msg, oKmcCtl);

   if(!(rc = SelectResult(msg)))
   {
      rc = (SelectParm1(msg) == PRMKMC_FULL)
           ? (CRESULT)ERRKMC_NOERROR
           : (CRESULT)ERRKMC_TIMEOUT;
   } /* endif */

   return(rc);
}

/* **************************************************** */
/* *              KMC Control Controller              * */
/* **************************************************** */
/* *                                                  * */
/* * - Initialize                                     * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_INITIALIZECTL          * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Query KMC status                               * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_QUERY_INHIBIT          * */
/* *                    MSGKMC_QUERY_IBF              * */
/* *                    MSGKMC_QUERY_OBF              * */
/* *                    MSGKMC_QUERY_IBFOBF           * */
/* *                    MSGKMC_QUERY_DATAERROR        * */
/* *                    MSGKMC_QUERY_DATATYPE         * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMKMC_NOINHIBIT)            * */
/* *                    (PRMKMC_INHIBIT  )            * */
/* *                    (PRMKMC_EMPTY    )            * */
/* *                    (PRMKMC_FULL     )            * */
/* *                    (PRMKMC_NOERROR  )            * */
/* *                    (PRMKMC_DATAERROR)            * */
/* *                    (PRMKMC_KEYBOARD )            * */
/* *                    (PRMKMC_AUXILIARY)            * */
/* *                                                  * */
/* * - Send command                                   * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_SEND_COMMAND           * */
/* *       Parameter1 = command                       * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Send command and data                          * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_SEND_COMMANDDATA       * */
/* *       Parameter1 = command                       * */
/* *       Parameter2 = data                          * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Send command and data for auxiliary device     * */
/* *                    (with waiting acknowledgment) * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_SEND_COMMANDDATAAUX    * */
/* *                    MSGKMC_SEND_COMMANDDATAAUXACK * */
/* *       Parameter1 = auxiliary command             * */
/* *                    auxiliary data                * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Receive data                                   * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_RECEIVE_DATA           * */
/* *       Parameter1 = receive buffer size           * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = received data size            * */
/* *       Parameter2 = received data 1               * */
/* *       Parameter3 = received data 2               * */
/* *       Parameter4 = received data 3               * */
/* *           :      =        :                      * */
/* *       ParameterN = received data (N-1)           * */
/* *                    (First  byte = data)          * */
/* *                    (Second byte = type)          * */
/* *                                                  * */
/* * - Clear received garbage data                    * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_CLEAR_DATA             * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get controller command byte                    * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_GET_CCB                * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = CCB value                     * */
/* *                                                  * */
/* * - Set controller command byte                    * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_SET_CCB                * */
/* *       Parameter1 = CCB value                     * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get device status                              * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_GET_xxxSTATUS          * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMKMC_xxxENABLE )           * */
/* *                    (PRMKMC_xxxDISABLE)           * */
/* *                                                  * */
/* * - Set device status                              * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_SET_xxxSTATUS          * */
/* *       Parameter1 = status                        * */
/* *                    (PRMKMC_xxxENABLE )           * */
/* *                    (PRMKMC_xxxDISABLE)           * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Save device context                            * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_SAVE_CONTEXT           * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Restore device context                         * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGKMC_RESTORE_CONTEXT        * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* **************************************************** */
VOID  KmcCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGKMC      msg;
   CRESULT     rc;
   UINT        retry, i;
   CPARAMETER  type, len, ccb;
   PCPARAMETER pdata;
   POBJKMCCTL  pkmcctl;

   /* Get pointer to KMC control object */
   /*                                   */
   pkmcctl = (POBJKMCCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGKMC_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGKMC_INITIALIZE);
         SendMsg(msg, oKmcIo);

         rc = SelectResult(msg);
         break;

      case MSGKMC_QUERY_INHIBIT:
         /* Query inhibit switch */
         /*                      */
         BuildMsg(msg, MSGKMC_READ_STATUS);
         SendMsg(msg, oKmcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITKMCSTATUS_INHIBIT)
                                  ? (CPARAMETER)PRMKMC_NOINHIBIT
                                  : (CPARAMETER)PRMKMC_INHIBIT;
         } /* endif */
         break;

      case MSGKMC_QUERY_IBF:
         /* Query input buffer full */
         /*                         */
         BuildMsg(msg, MSGKMC_READ_STATUS);
         SendMsg(msg, oKmcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITKMCSTATUS_IBF)
                                  ? (CPARAMETER)PRMKMC_FULL
                                  : (CPARAMETER)PRMKMC_EMPTY;
         } /* endif */
         break;

      case MSGKMC_QUERY_OBF:
         /* Query output buffer full */
         /*                          */
         BuildMsg(msg, MSGKMC_READ_STATUS);
         SendMsg(msg, oKmcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITKMCSTATUS_OBF)
                                  ? (CPARAMETER)PRMKMC_FULL
                                  : (CPARAMETER)PRMKMC_EMPTY;
         } /* endif */
         break;

      case MSGKMC_QUERY_IBFOBF:
         /* Query input/output buffer full */
         /*                                */
         BuildMsg(msg, MSGKMC_READ_STATUS);
         SendMsg(msg, oKmcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & (BITKMCSTATUS_IBF |
                                                       BITKMCSTATUS_OBF ))
                                  ? (CPARAMETER)PRMKMC_FULL
                                  : (CPARAMETER)PRMKMC_EMPTY;
         } /* endif */
         break;

      case MSGKMC_QUERY_DATAERROR:
         /* Query parity error and general time-out */
         /*                                         */
         BuildMsg(msg, MSGKMC_READ_STATUS);
         SendMsg(msg, oKmcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   (BITKMCSTATUS_PARITYERROR   |
                                    BITKMCSTATUS_GENERALTIMEOUT))
                                  ? (CPARAMETER)PRMKMC_DATAERROR
                                  : (CPARAMETER)PRMKMC_NOERROR;
         } /* endif */
         break;

      case MSGKMC_QUERY_DATATYPE:
         /* Query received data type */
         /*                          */
         BuildMsg(msg, MSGKMC_READ_STATUS);
         SendMsg(msg, oKmcIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITKMCSTATUS_OBFAUX)
                                  ? (CPARAMETER)PRMKMC_AUXILIARY
                                  : (CPARAMETER)PRMKMC_KEYBOARD;
         } /* endif */
         break;

      case MSGKMC_SEND_COMMAND:
         /* Send command */
         /*              */
         BuildMsg(msg, MSGKMC_QUERY_INHIBIT);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) == PRMKMC_NOINHIBIT)
            {
               if(!(rc = WaitTimeCheck(WAITTIME_KMCBUSY, KmcQueryIbfBusy)))
               {
                  BuildMsgWithParm1(msg, MSGKMC_WRITE_COMMAND,
                                         SelectParm1(*pmMsg));
                  SendMsg(msg, oKmcIo);

                  rc = SelectResult(msg);

                  WaitTimeCheck(WAITTIME_KMCBUSY, KmcQueryIbfBusy);
               } /* endif */
            }
            else
            {
               rc = ERRKMC_ACCESS_DENIED;
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_SEND_COMMANDDATA:
         /* Send command and data */
         /*                       */
         BuildMsg(msg, MSGKMC_QUERY_INHIBIT);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) == PRMKMC_NOINHIBIT)
            {
               if(!(rc = WaitTimeCheck(WAITTIME_KMCBUSY, KmcQueryIbfObfBusy)))
               {
                  BuildMsgWithParm1(msg, MSGKMC_WRITE_COMMAND,
                                         SelectParm1(*pmMsg));
                  SendMsg(msg, oKmcIo);

                  if(!(rc = SelectResult(msg)))
                  {
                     WaitTimeCheck(WAITTIME_KMCBUSY, KmcQueryIbfBusy);

                     BuildMsgWithParm1(msg, MSGKMC_WRITE_DATA,
                                            SelectParm2(*pmMsg));
                     SendMsg(msg, oKmcIo);

                     rc = SelectResult(msg);
                  } /* endif */
               } /* endif */
            }
            else
            {
               rc = ERRKMC_ACCESS_DENIED;
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_SEND_COMMANDDATAAUX:
         /* Send command and data for auxiliary device */
         /*                                            */
         BuildMsgWithParm2(msg, MSGKMC_SEND_COMMANDDATA,
                                CMDKMCCOMMAND_WRITEAUX,
                                SelectParm1(*pmMsg));
         SendMsg(msg, oKmcCtl);

         rc = SelectResult(msg);
         break;

      case MSGKMC_SEND_COMMANDDATAAUXACK:
         /* Send command and data for auxiliary device */
         /*                with waiting acknowledgment */
         BuildMsgWithParm2(msg, MSGKMC_SEND_COMMANDDATA,
                                CMDKMCCOMMAND_WRITEAUX,
                                SelectParm1(*pmMsg));
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGKMC_RECEIVE_DATA,
                                   PRMKMC_DATASIZEMAX);
            SendMsg(msg, oKmcCtl);

            if(!(rc = SelectResult(msg)))
            {
               for(i = 0, pdata = &(SelectParm2(msg)), rc = ERRKMC_DATAERROR;
                   (i < SelectParm1(msg)) && rc;
                   i++, pdata++)
               {
                  if(*pdata == ((PRMKMC_AUXILIARY << 8) | ACKKMCDATA_OK))
                  {
                     rc = ERRKMC_NOERROR;
                  } /* endif */
               } /* endfor */
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_RECEIVE_DATA:
         /* Receive data */
         /*              */
         BuildMsg(msg, MSGKMC_QUERY_INHIBIT);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) == PRMKMC_NOINHIBIT)
            {
               for(len = 0, pdata = &(SelectParm2(*pmMsg));
                   (len < SelectParm1(*pmMsg)) &&
                   (len < PRMKMC_DATASIZEMAX)  && !rc;
                   len++, pdata++)
               {
                  if(!(rc = WaitTimeCheck(WAITTIME_DATAREADY,
                                          KmcQueryObfReady  )))
                  {
                     BuildMsg(msg, MSGKMC_QUERY_DATAERROR);
                     SendMsg(msg, oKmcCtl);

                     if(!(rc = SelectResult(msg)))
                     {
                        if(SelectParm1(msg) == PRMKMC_NOERROR)
                        {
                           BuildMsg(msg, MSGKMC_QUERY_DATATYPE);
                           SendMsg(msg, oKmcCtl);

                           if(!(rc = SelectResult(msg)))
                           {
                              type = SelectParm1(msg);

                              BuildMsg(msg, MSGKMC_READ_DATA);
                              SendMsg(msg, oKmcIo);

                              if(!(rc = SelectResult(msg)))
                              {
                                 *pdata = (type << 8) | SelectParm1(msg);

                                 if(WaitTimeCheck(WAITTIME_KMCBUSY,
                                                  KmcQueryObfReady))
                                 {
                                    len++;
                                    break;
                                 } /* endif */
                              } /* endif */
                           } /* endif */
                        }
                        else
                        {
                           BuildMsg(msg, MSGKMC_READ_DATA);
                           SendMsg(msg, oKmcIo);

                           rc = ERRKMC_DATAERROR;
                        } /* endif */
                     } /* endif */
                  } /* endif */
               } /* endfor */

               SelectParm1(*pmMsg) = len;
            }
            else
            {
               rc = ERRKMC_ACCESS_DENIED;
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_CLEAR_DATA:
         /* Clear received garbage data */
         /*                             */
         BuildMsg(msg, MSGKMC_QUERY_INHIBIT);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            if(SelectParm1(msg) == PRMKMC_NOINHIBIT)
            {
               while(!rc)
               {
                  BuildMsg(msg, MSGKMC_QUERY_OBF);
                  SendMsg(msg, oKmcCtl);

                  if(!(rc = SelectResult(msg)))
                  {
                     if(SelectParm1(msg) == PRMKMC_FULL)
                     {
                        BuildMsg(msg, MSGKMC_READ_DATA);
                        SendMsg(msg, oKmcIo);

                        if(!(rc = SelectResult(msg)))
                        {
                           if(WaitTimeCheck(WAITTIME_KMCBUSY, KmcQueryObfReady))
                           {
                              break;
                           } /* endif */
                        } /* endif */
                     }
                     else
                     {
                        break;
                     } /* endif */
                  } /* endif */
               } /* endwhile */
            }
            else
            {
               rc = ERRKMC_ACCESS_DENIED;
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_GET_CCB:
         /* Get controller command byte */
         /*                             */
         BuildMsg(msg, MSGKMC_CLEAR_DATA);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm1(msg, MSGKMC_SEND_COMMAND,
                                   CMDKMCCOMMAND_READCCB);
            SendMsg(msg, oKmcCtl);

            if(!(rc = SelectResult(msg)))
            {
               BuildMsgWithParm1(msg, MSGKMC_RECEIVE_DATA,
                                      PRMKMC_DATASIZEMAX);
               SendMsg(msg, oKmcCtl);

               if(!(rc = SelectResult(msg)))
               {
                  if( (SelectParm1(msg) == 1)                  &&
                     ((SelectParm2(msg) >> 8) == PRMKMC_KEYBOARD) )
                  {
                     pkmcctl->chCcb = (BYTE)(SelectParm2(msg) &
                                             PRMKMC_MASKDATA);
                  } /* endif */

                  SelectParm1(*pmMsg) = (CPARAMETER)(pkmcctl->chCcb);
               } /* endif */
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_SET_CCB:
         /* Set controller command byte */
         /*                             */
         BuildMsg(msg, MSGKMC_CLEAR_DATA);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm2(msg, MSGKMC_SEND_COMMANDDATA,
                                   CMDKMCCOMMAND_WRITECCB,
                                   SelectParm1(*pmMsg));
            SendMsg(msg, oKmcCtl);

            if(!(rc = SelectResult(msg)))
            {
               pkmcctl->chCcb = (BYTE)(SelectParm1(*pmMsg));
            } /* endif */
         } /* endif */
         break;

      case MSGKMC_GET_KBDSTATUS:
         /* Get keyboard status */
         /*                     */
         BuildMsg(msg, MSGKMC_GET_CCB);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITKMCCCB_DISABLEKBD)
                                  ? (CPARAMETER)PRMKMC_KBDDISABLE
                                  : (CPARAMETER)PRMKMC_KBDENABLE;
         } /* endif */
         break;

      case MSGKMC_SET_KBDSTATUS:
         /* Set keyboard status */
         /*                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMKMC_KBDENABLE:
               BuildMsg(msg, MSGKMC_GET_CCB);
               SendMsg(msg, oKmcCtl);

               if(!(rc = SelectResult(msg)))
               {
                  ccb = (SelectParm1(msg) | BITKMCCCB_ENABLEKBDINT) &
                         ~BITKMCCCB_DISABLEKBD;

                  BuildMsgWithParm1(msg, MSGKMC_SET_CCB,
                                         ccb);
                  SendMsg(msg, oKmcCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            case PRMKMC_KBDDISABLE:
               BuildMsg(msg, MSGKMC_GET_CCB);
               SendMsg(msg, oKmcCtl);

               if(!(rc = SelectResult(msg)))
               {
                  ccb = (SelectParm1(msg) & ~BITKMCCCB_ENABLEKBDINT) |
                         BITKMCCCB_DISABLEKBD;

                  BuildMsgWithParm1(msg, MSGKMC_SET_CCB,
                                         ccb);
                  SendMsg(msg, oKmcCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            default:
               rc = ERRKMC_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGKMC_GET_AUXSTATUS:
         /* Get auxiliary device status */
         /*                             */
         BuildMsg(msg, MSGKMC_GET_CCB);
         SendMsg(msg, oKmcCtl);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) & BITKMCCCB_DISABLEAUX)
                                  ? (CPARAMETER)PRMKMC_AUXDISABLE
                                  : (CPARAMETER)PRMKMC_AUXENABLE;
         } /* endif */
         break;

      case MSGKMC_SET_AUXSTATUS:
         /* Set auxiliary device status */
         /*                             */
         switch(SelectParm1(*pmMsg))
         {
            case PRMKMC_AUXENABLE:
               BuildMsg(msg, MSGKMC_GET_CCB);
               SendMsg(msg, oKmcCtl);

               if(!(rc = SelectResult(msg)))
               {
                  ccb = (SelectParm1(msg) | BITKMCCCB_ENABLEAUXINT) &
                         ~BITKMCCCB_DISABLEAUX;

                  BuildMsgWithParm1(msg, MSGKMC_SET_CCB,
                                         ccb);
                  SendMsg(msg, oKmcCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            case PRMKMC_AUXDISABLE:
               BuildMsg(msg, MSGKMC_GET_CCB);
               SendMsg(msg, oKmcCtl);

               if(!(rc = SelectResult(msg)))
               {
                  ccb = (SelectParm1(msg) & ~BITKMCCCB_ENABLEAUXINT) |
                         BITKMCCCB_DISABLEAUX;

                  BuildMsgWithParm1(msg, MSGKMC_SET_CCB,
                                         ccb);
                  SendMsg(msg, oKmcCtl);

                  rc = SelectResult(msg);
               } /* endif */
               break;
            default:
               rc = ERRKMC_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGKMC_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         for(retry = 10, rc = ERRKMC_TIMEOUT; retry && rc; retry--)
         {
            BuildMsg(msg, MSGKMC_GET_CCB);
            SendMsg(msg, oKmcCtl);

            if((rc = SelectResult(msg)) != ERRKMC_NOERROR)
            {
               if(rc == ERRKMC_ACCESS_DENIED)
               {
                  break;
               } /* endif */
            } /* endif */
         } /* endfor */
         break;

      case MSGKMC_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         for(retry = 10, rc = ERRKMC_TIMEOUT; retry && rc; retry--)
         {
            BuildMsgWithParm1(msg, MSGKMC_SET_CCB,
                                   pkmcctl->chCcb);
            SendMsg(msg, oKmcCtl);

            if((rc = SelectResult(msg)) != ERRKMC_NOERROR)
            {
               if(rc == ERRKMC_ACCESS_DENIED)
               {
                  break;
               } /* endif */
            } /* endif */
         } /* endfor */
         break;

      default:
         SendMsg(*pmMsg, oKmcIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
