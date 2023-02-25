/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: C2 Security (EEPROM) Device Control Routines
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
/* * Module Name: PDEEPROM.C                                               * */
/* * Description: C2 Security (EEPROM) Device Control Routines             * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDEEPROM.H files should be included* */
/* *              in this file                                             * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdeeprom.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      EEPROM Data Definition       * */
/* ************************************* */
/* ------------------------------- */
/*  Control register port address  */
/* ------------------------------- */
#define  PORTEEPROM_BASE                  (0x00000078L)
                                                      /* EEPROM base address */
#define  PORTEEPROM_COMMANDADDRESSMSB     (PORTEEPROM_BASE + 0)
                                         /* Command and address MSB register */
#define  PORTEEPROM_SIZELOCK              (PORTEEPROM_BASE + 0)
                                                       /* Size lock register */
#define  PORTEEPROM_COMMANDADDRESSLSB     (PORTEEPROM_BASE + 1)
                                         /* Command and address LSB register */
#define  PORTEEPROM_ADDRESSLOCKS          (PORTEEPROM_BASE + 1)
                                                   /* Address locks register */
#define  PORTEEPROM_DATALSB               (PORTEEPROM_BASE + 2)
                                                        /* Data LSB register */
#define  PORTEEPROM_DATAMSB               (PORTEEPROM_BASE + 3)
                                                        /* Data MSB register */
#define  PORTEEPROM_STATUS                (PORTEEPROM_BASE + 3)
                                                          /* Status register */
#define  PORTEEPROM_COMMANDSTATUS         (PORTEEPROM_BASE + 4)
                                              /* Command and status register */

/* -------------------------------------------------------- */
/*  Command and status register (COMMANDSTATUS) bit assign  */
/* -------------------------------------------------------- */
#define  BITEEPROMCOMMANDSTATUS_ACCINDEX  (0x08)
                                         /* (Bit-3) Access indexed registers */
                                         /*         (1=index, 0=base)        */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *   EEPROM Controller Definition    * */
/* ************************************* */
/* ------------------ */
/*  EEPROM I/O class  */
/* ------------------ */
VOID  EepromIo(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */

/* ---------------------- */
/*  EEPROM control class  */
/* ---------------------- */
VOID  EepromCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *          EEPROM Objects           * */
/* ************************************* */
/* ------------------ */
/*  EEPROM I/O class  */
/* ------------------ */
OBJEEPROMIO    oEepromIo  = {(POBJCTL)EepromIo, PORTISA_BASE};

/* ---------------------- */
/*  EEPROM control class  */
/* ---------------------- */
OBJEEPROMCTL   oEepromCtl = {(POBJCTL)EepromCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ******************************************* */
/* *          EEPROM I/O Controller          * */
/* ******************************************* */
/* *                                         * */
/* * - Initialize                            * */
/* *     [ Input ]                           * */
/* *       Message    = MSGEEPROM_INITIALIZE * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* * - Read register                         * */
/* *     [ Input ]                           * */
/* *       Message    = MSGEEPROM_READ_xxx   * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *       Parameter1 = register value       * */
/* *                                         * */
/* * - Write register                        * */
/* *     [ Input ]                           * */
/* *       Message    = MSGEEPROM_WRITE_xxx  * */
/* *       Parameter1 = register value       * */
/* *     [ Output ]                          * */
/* *       Result     = result code          * */
/* *                                         * */
/* ******************************************* */
VOID  EepromIo(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   BYTE           scmdstat;
   DWORD          datap;
   CRESULT        rc;
   POBJEEPROMIO   peepromio;
   DWORD          baseport;

   /* Get pointer to EEPROM I/O object */
   /*                                 */
   peepromio = (POBJEEPROMIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = GetIsaBaseAddress();

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGEEPROM_INITIALIZE:
         /* Initialize */
         /*            */
         peepromio->BaseAddress = GetIsaBaseAddress();
         rc = ERREEPROM_NOERROR;
         break;

      case PORTEEPROM_READ_COMMANDSTATUS:
         /* Read command and status register */
         /*                                  */
         SelectParm1(*pmMsg) =
            (CPARAMETER)ReadControllerByte(baseport + PORTEEPROM_COMMANDSTATUS);
         rc = ERREEPROM_NOERROR;
         break;

      case PORTEEPROM_WRITE_COMMANDSTATUS:
         /* Write command and status register */
         /*                                   */
         WriteControllerByte(baseport + PORTEEPROM_COMMANDSTATUS,
                             (BYTE)SelectParm1(*pmMsg));
         rc = ERREEPROM_NOERROR;
         break;

      default:
         /* Get data-port register */
         /*                        */
         switch(SelectMessage(*pmMsg))
         {
            case PORTEEPROM_READ_CMDADDRMSB:
            case PORTEEPROM_WRITE_CMDADDRMSB:
               datap = PORTEEPROM_COMMANDADDRESSMSB;
               rc    = ERREEPROM_NOERROR;
               break;
            case PORTEEPROM_READ_SIZELOCK:
            case PORTEEPROM_WRITE_SIZELOCK:
               datap = PORTEEPROM_SIZELOCK;
               rc    = ERREEPROM_NOERROR;
               break;
            case PORTEEPROM_READ_CMDADDRLSB:
            case PORTEEPROM_WRITE_CMDADDRLSB:
               datap = PORTEEPROM_COMMANDADDRESSLSB;
               rc    = ERREEPROM_NOERROR;
               break;
            case PORTEEPROM_READ_ADDRESSLOCKS:
            case PORTEEPROM_WRITE_ADDRESSLOCKS:
               datap = PORTEEPROM_ADDRESSLOCKS;
               rc    = ERREEPROM_NOERROR;
               break;
            case PORTEEPROM_READ_DATALSB:
            case PORTEEPROM_WRITE_DATALSB:
               datap = PORTEEPROM_DATALSB;
               rc    = ERREEPROM_NOERROR;
               break;
            case PORTEEPROM_READ_DATAMSB:
            case PORTEEPROM_WRITE_DATAMSB:
               datap = PORTEEPROM_DATAMSB;
               rc    = ERREEPROM_NOERROR;
               break;
            case PORTEEPROM_READ_STATUS:
            case PORTEEPROM_WRITE_STATUS:
               datap = PORTEEPROM_STATUS;
               rc    = ERREEPROM_NOERROR;
               break;
            default:
               rc = ERREEPROM_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Save Command/Status register */
            /*                              */
            scmdstat = ReadControllerByte(baseport + PORTEEPROM_COMMANDSTATUS);

            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case PORTEEPROM_READ_CMDADDRMSB:
               case PORTEEPROM_READ_CMDADDRLSB:
               case PORTEEPROM_READ_DATALSB:
               case PORTEEPROM_READ_DATAMSB:
                  WriteControllerByte(baseport + PORTEEPROM_COMMANDSTATUS,
                                      scmdstat &
                                      ~BITEEPROMCOMMANDSTATUS_ACCINDEX);
                  SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + datap);
                  break;
               case PORTEEPROM_READ_SIZELOCK:
               case PORTEEPROM_READ_ADDRESSLOCKS:
               case PORTEEPROM_READ_STATUS:
                  WriteControllerByte(baseport + PORTEEPROM_COMMANDSTATUS,
                                      scmdstat |
                                      BITEEPROMCOMMANDSTATUS_ACCINDEX);
                  SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + datap);
                  break;
               case PORTEEPROM_WRITE_CMDADDRMSB:
               case PORTEEPROM_WRITE_CMDADDRLSB:
               case PORTEEPROM_WRITE_DATALSB:
               case PORTEEPROM_WRITE_DATAMSB:
                  WriteControllerByte(baseport + PORTEEPROM_COMMANDSTATUS,
                                      scmdstat &
                                      ~BITEEPROMCOMMANDSTATUS_ACCINDEX);
                  WriteControllerByte(baseport + datap,
                                      (BYTE)SelectParm1(*pmMsg));
                  break;
               case PORTEEPROM_WRITE_SIZELOCK:
               case PORTEEPROM_WRITE_ADDRESSLOCKS:
               case PORTEEPROM_WRITE_STATUS:
                  WriteControllerByte(baseport + PORTEEPROM_COMMANDSTATUS,
                                      scmdstat |
                                      BITEEPROMCOMMANDSTATUS_ACCINDEX);
                  WriteControllerByte(baseport + datap,
                                      (BYTE)SelectParm1(*pmMsg));
                  break;
               default:
                  rc = ERREEPROM_INVALID_MESSAGE;
            } /* endswitch */

            /* Restore Command/Status register */
            /*                                 */
            IoDelay();
            WriteControllerByte(baseport + PORTEEPROM_COMMANDSTATUS, scmdstat);
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ************************************************ */
/* *          EEPROM Control Controller           * */
/* ************************************************ */
/* *                                              * */
/* * - Initialize                                 * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEEPROM_INITIALIZECTL   * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Save device context                        * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEEPROM_SAVE_CONTEXT    * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Restore device context                     * */
/* *     [ Input ]                                * */
/* *       Message    = MSGEEPROM_RESTORE_CONTEXT * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* ************************************************ */
VOID  EepromCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   MSGEEPROM      msg;
   CRESULT        rc;
   POBJEEPROMCTL  peepromctl;

   /* Get pointer to EEPROM control object */
   /*                                      */
   peepromctl = (POBJEEPROMCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGEEPROM_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGEEPROM_INITIALIZE);
         SendMsg(msg, oEepromIo);

         rc = SelectResult(msg);
         break;

      case MSGEEPROM_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERREEPROM_NOERROR;

         BuildMsg(msg, PORTEEPROM_READ_COMMANDSTATUS);
         SendMsg(msg, oEepromIo);
         if(!SelectResult(msg))
         {
            peepromctl->chCommandStatus = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, PORTEEPROM_READ_SIZELOCK);
         SendMsg(msg, oEepromIo);
         if(!SelectResult(msg))
         {
            peepromctl->chSizeLock = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, PORTEEPROM_READ_ADDRESSLOCKS);
         SendMsg(msg, oEepromIo);
         if(!SelectResult(msg))
         {
            peepromctl->chAddressLocks = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, PORTEEPROM_READ_STATUS);
         SendMsg(msg, oEepromIo);
         if(!SelectResult(msg))
         {
            peepromctl->chStatus = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGEEPROM_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERREEPROM_NOERROR;

         BuildMsgWithParm1(msg, PORTEEPROM_WRITE_SIZELOCK,
                                peepromctl->chSizeLock);
         SendMsg(msg, oEepromIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         if(peepromctl->chAddressLocks)
         {
            BuildMsgWithParm1(msg, PORTEEPROM_WRITE_ADDRESSLOCKS,
                                   peepromctl->chAddressLocks);
            SendMsg(msg, oEepromIo);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */
         } /* endif */

         BuildMsgWithParm1(msg, PORTEEPROM_WRITE_STATUS,
                                peepromctl->chStatus);
         SendMsg(msg, oEepromIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, PORTEEPROM_WRITE_COMMANDSTATUS,
                                peepromctl->chCommandStatus);
         SendMsg(msg, oEepromIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oEepromIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
