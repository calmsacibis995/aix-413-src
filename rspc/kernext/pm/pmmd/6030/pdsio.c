/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: System I/O (82378IB) Chip Device Control Routines
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
/* * Module Name: PDSIO.C                                                  * */
/* * Description: System I/O (82378IB) Chip Device Control Routines        * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDSIO.H/PDPCIBUS.H files should be * */
/* *              included in this file                                    * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdsio.h"
#include "pdpcibus.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *    System-I/O Data Definition     * */
/* ************************************* */
/* ------------------------------------- */
/*  Configuration register port address  */
/* ------------------------------------- */
#define  PORTSIO_BASE                     (0x80005800L)
#define  PORTSIO_BASENULL                 (0xFFFFFFFFL)
                                                  /* System-I/O base address */
                                                      /* & base null address */
                                                 /* Offset from base address */
#define  PORTSIO_PCICON                   (0x40)
                                                     /* PCI control register */
#define  PORTSIO_PAC                      (0x41)
                                             /* PCI arbiter control register */
#define  PORTSIO_PAPC                     (0x42)
                                    /* PCI arbiter priority control register */
#define  PORTSIO_ARBPRIX                  (0x43)
                          /* PCI arbiter priority control extension register */
#define  PORTSIO_MCSCON                   (0x44)
                                                  /* MEMCS# control register */
#define  PORTSIO_MCSBOH                   (0x45)
                                           /* MEMCS# bottom of hole register */
#define  PORTSIO_MCSTOH                   (0x46)
                                              /* MEMCS# top of hole register */
#define  PORTSIO_MCSTOM                   (0x47)
                                            /* MEMCS# top of memory register */
#define  PORTSIO_IADCON                   (0x48)
                                     /* ISA address decoder control register */
#define  PORTSIO_IADRBE                   (0x49)
                            /* ISA address decoder ROM block enable register */
#define  PORTSIO_IADBOH                   (0x4A)
                              /* ISA address decoder bottom of hole register */
#define  PORTSIO_IADTOH                   (0x4B)
                                 /* ISA address decoder top of hole register */
#define  PORTSIO_ICRT                     (0x4C)
                                   /* ISA controller recovery timer register */
#define  PORTSIO_ICD                      (0x4D)
                                               /* ISA clock divisor register */
#define  PORTSIO_UBCSA                    (0x4E)
                                /* Utility bus chip select enable A register */
#define  PORTSIO_UBCSB                    (0x4F)
                                /* Utility bus chip select enable B register */
#define  PORTSIO_MAR1                     (0x54)
                                             /* MEMCS# attribute #1 register */
#define  PORTSIO_MAR2                     (0x55)
                                             /* MEMCS# attribute #2 register */
#define  PORTSIO_MAR3                     (0x56)
                                             /* MEMCS# attribute #3 register */
#define  PORTSIO_DMASCATTERBASEADDR       (0x57)
                      /* DMA Scatter/Gather relocation base address register */
#define  PORTSIO_PIRQ0                    (0x60)
                                            /* PIRQ0# route control register */
#define  PORTSIO_PIRQ1                    (0x61)
                                            /* PIRQ1# route control register */
#define  PORTSIO_PIRQ2                    (0x62)
                                            /* PIRQ2# route control register */
#define  PORTSIO_PIRQ3                    (0x63)
                                            /* PIRQ3# route control register */
#define  PORTSIO_BIOSTIMERBASEADDR        (0x80)
                                         /* BIOS timer base address register */
#define  PORTSIO_SMICNTL                  (0xA0)
                                                     /* SMI control register */
#define  PORTSIO_SMIEN                    (0xA2)
                                                      /* SMI enable register */
#define  PORTSIO_SEE                      (0xA4)
                                             /* System event enable register */
#define  PORTSIO_FTMR                     (0xA8)
                                                  /* Fast off timer register */
#define  PORTSIO_SMIREQ                   (0xAA)
                                                     /* SMI request register */
#define  PORTSIO_CTLTMRL                  (0xAC)
                                /* Clock throttle STPCLK# low timer register */
#define  PORTSIO_CTLTMRH                  (0xAE)
                               /* Clock throttle STPCLK# high timer register */
#define  PORTSIO_APM                      (0xB0)
                                  /* Advanced power management port register */
#define  PORTSIO_APMC                     (0xB2)
                          /* Advanced power management control port register */
#define  PORTSIO_APMS                     (0xB3)
                           /* Advanced power management status port register */
#define  PORTSIO_ELCR1                    (0x4D0)
                                            /* Edge/Level control 1 register */
#define  PORTSIO_ELCR2                    (0x4D1)
                                            /* Edge/Level control 2 register */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* * System-I/O Controller Definition  * */
/* ************************************* */
/* ---------------------- */
/*  System-I/O I/O class  */
/* ---------------------- */
VOID  SystemioIo(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj);            /* Pointer to Common object */

/* -------------------------- */
/*  System-I/O control class  */
/* -------------------------- */
VOID  SystemioCtl(PMSGCOMMON  pmMsg,     /* Pointer to Common message packet */
                  POBJCOMMON  poObj);            /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *        System-I/O Objects         * */
/* ************************************* */
/* ---------------------- */
/*  System-I/O I/O class  */
/* ---------------------- */
OBJSYSTEMIOIO  oSystemioIo  = {(POBJCTL)SystemioIo, PORTSIO_BASENULL};

/* -------------------------- */
/*  System-I/O control class  */
/* -------------------------- */
OBJSYSTEMIOCTL oSystemioCtl = {(POBJCTL)SystemioCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************ */
/* *          System-I/O I/O Controller           * */
/* ************************************************ */
/* *                                              * */
/* * - Initialize                                 * */
/* *     [ Input ]                                * */
/* *       Message    = MSGSIO_INITIALIZE         * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read register                              * */
/* *     [ Input ]                                * */
/* *       Message    = MSGSIO_READ_xxx           * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       Parameter1 = register value            * */
/* *       SioParm1   = register value (32bits)   * */
/* *                                              * */
/* * - Write register                             * */
/* *     [ Input ]                                * */
/* *       Message    = MSGSIO_WRITE_xxx          * */
/* *       Parameter1 = register value            * */
/* *       SioParm1   = register value (32bits)   * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* ************************************************ */
VOID  SystemioIo(PMSGCOMMON   pmMsg,     /* Pointer to Common message packet */
                 POBJCOMMON   poObj)             /* Pointer to Common object */
{
   MSGPCI         mPci;
   WORD           off;
   CRESULT        rc;
   POBJSYSTEMIOIO psioio;
   DWORD          baseport;

   /* Get pointer to System-I/O I/O object */
   /*                                      */
   psioio = (POBJSYSTEMIOIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = GetIsaBaseAddress();

   /* Check if completing for initialization */
   /*                                        */
   if((psioio->BaseAddress   != PORTSIO_BASENULL) ||
      (SelectMessage(*pmMsg) == MSGSIO_INITIALIZE)  )
   {
      /* Control registers */
      /*                   */
      switch(SelectMessage(*pmMsg))
      {
         case MSGSIO_INITIALIZE:
            /* Initialize */
            /*            */
            BuildMsg(mPci, MSGPCI_QUERY_DEVICE);
            SendMsg(mPci, oPciSystemioCtl);

            if(!(rc = SelectResult(mPci)))
            {
               BuildMsg(mPci, MSGPCI_GET_CONFIGADDRESS);
               SendMsg(mPci, oPciSystemioCtl);

               if(!(rc = SelectResult(mPci)))
               {
                  /* psioio->BaseAddress = SelectPciParm1(mPci); */
                  psioio->BaseAddress = GetIsaBaseAddress();
               }
               else
               {
                  rc = ERRSIO_CANNOT_INITIALIZE;
               } /* endif */
            }
            else
            {
               rc = ERRSIO_CANNOT_INITIALIZE;
            } /* endif */
            break;

         default:
            /* Get offset to data-port register */
            /*                                  */
            switch(SelectMessage(*pmMsg))
            {
               case MSGSIO_READ_PCICON:
               case MSGSIO_WRITE_PCICON:
                  off = PORTSIO_PCICON;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_PAC:
               case MSGSIO_WRITE_PAC:
                  off = PORTSIO_PAC;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_PAPC:
               case MSGSIO_WRITE_PAPC:
                  off = PORTSIO_PAPC;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MCSCON:
               case MSGSIO_WRITE_MCSCON:
                  off = PORTSIO_MCSCON;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MCSBOH:
               case MSGSIO_WRITE_MCSBOH:
                  off = PORTSIO_MCSBOH;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MCSTOH:
               case MSGSIO_WRITE_MCSTOH:
                  off = PORTSIO_MCSTOH;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MCSTOM:
               case MSGSIO_WRITE_MCSTOM:
                  off = PORTSIO_MCSTOM;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_IADCON:
               case MSGSIO_WRITE_IADCON:
                  off = PORTSIO_IADCON;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_IADRBE:
               case MSGSIO_WRITE_IADRBE:
                  off = PORTSIO_IADRBE;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_IADBOH:
               case MSGSIO_WRITE_IADBOH:
                  off = PORTSIO_IADBOH;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_IADTOH:
               case MSGSIO_WRITE_IADTOH:
                  off = PORTSIO_IADTOH;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_ICRT:
               case MSGSIO_WRITE_ICRT:
                  off = PORTSIO_ICRT;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_ICD:
               case MSGSIO_WRITE_ICD:
                  off = PORTSIO_ICD;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_UBCSA:
               case MSGSIO_WRITE_UBCSA:
                  off = PORTSIO_UBCSA;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_UBCSB:
               case MSGSIO_WRITE_UBCSB:
                  off = PORTSIO_UBCSB;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MAR1:
               case MSGSIO_WRITE_MAR1:
                  off = PORTSIO_MAR1;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MAR2:
               case MSGSIO_WRITE_MAR2:
                  off = PORTSIO_MAR2;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_MAR3:
               case MSGSIO_WRITE_MAR3:
                  off = PORTSIO_MAR3;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_DMASCATTERBASEADDR:
               case MSGSIO_WRITE_DMASCATTERBASEADDR:
                  off = PORTSIO_DMASCATTERBASEADDR;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_PIRQ0:
               case MSGSIO_WRITE_PIRQ0:
                  off = PORTSIO_PIRQ0;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_BIOSTIMERBASEADDR:
               case MSGSIO_WRITE_BIOSTIMERBASEADDR:
                  off = PORTSIO_BIOSTIMERBASEADDR;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_SMICNTL:
               case MSGSIO_WRITE_SMICNTL:
                  off = PORTSIO_SMICNTL;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_SEE:
               case MSGSIO_WRITE_SEE:
                  off = PORTSIO_SEE;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_FTMR:
               case MSGSIO_WRITE_FTMR:
                  off = PORTSIO_FTMR;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_CTLTMRL:
               case MSGSIO_WRITE_CTLTMRL:
                  off = PORTSIO_CTLTMRL;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_APM:
               case MSGSIO_WRITE_APM:
                  off = PORTSIO_APM;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_ELCR1:
               case MSGSIO_WRITE_ELCR1:
                  off = PORTSIO_ELCR1;
                  rc  = ERRSIO_NOERROR;
                  break;
               case MSGSIO_READ_ELCR2:
               case MSGSIO_WRITE_ELCR2:
                  off = PORTSIO_ELCR2;
                  rc  = ERRSIO_NOERROR;
                  break;
               default:
                  rc = ERRSIO_INVALID_MESSAGE;
            } /* endswitch */

            if(!rc)
            {
               /* Input/Output registers */
               /*                        */
               switch(SelectMessage(*pmMsg))
               {
                  case MSGSIO_READ_PCICON:
                  case MSGSIO_READ_PAC:
                  case MSGSIO_READ_PAPC:
                  case MSGSIO_READ_MCSCON:
                  case MSGSIO_READ_MCSBOH:
                  case MSGSIO_READ_MCSTOH:
                  case MSGSIO_READ_MCSTOM:
                  case MSGSIO_READ_IADCON:
                  case MSGSIO_READ_IADRBE:
                  case MSGSIO_READ_IADBOH:
                  case MSGSIO_READ_IADTOH:
                  case MSGSIO_READ_ICRT:
                  case MSGSIO_READ_ICD:
                  case MSGSIO_READ_UBCSA:
                  case MSGSIO_READ_UBCSB:
                  case MSGSIO_READ_MAR1:
                  case MSGSIO_READ_MAR2:
                  case MSGSIO_READ_MAR3:
                  case MSGSIO_READ_DMASCATTERBASEADDR:
                  case MSGSIO_READ_PIRQ0:
                  case MSGSIO_READ_SMICNTL:
                  case MSGSIO_READ_SEE:
                  case MSGSIO_READ_FTMR:
                  case MSGSIO_READ_CTLTMRL:
                  case MSGSIO_READ_APM:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciSystemioCtl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        SelectSioParm1(*pmMsg) = SelectPciParm1(mPci);
                        SelectParm1(*pmMsg)    =
                                      (CPARAMETER)SelectPciParm1(mPci) & 0x00FF;
                     }
                     else
                     {
                        rc = ERRSIO_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGSIO_READ_BIOSTIMERBASEADDR:
                     BuildMsgWithParm1(mPci, MSGPCI_GET_CONFIGDATA,
                                             off);
                     SendMsg(mPci, oPciSystemioCtl);

                     if(!(rc = SelectResult(mPci)))
                     {
                        SelectSioParm1(*pmMsg) = SelectPciParm1(mPci);
                        SelectParm1(*pmMsg)    =
                                               (CPARAMETER)SelectPciParm1(mPci);
                     }
                     else
                     {
                        rc = ERRSIO_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGSIO_READ_ELCR1:
                  case MSGSIO_READ_ELCR2:
                     SelectParm1(*pmMsg) =
                          (CPARAMETER)ReadControllerByte(baseport + (DWORD)off);
                     break;
                  case MSGSIO_WRITE_PCICON:
                  case MSGSIO_WRITE_PAC:
                  case MSGSIO_WRITE_PAPC:
                  case MSGSIO_WRITE_MCSCON:
                  case MSGSIO_WRITE_MCSBOH:
                  case MSGSIO_WRITE_MCSTOH:
                  case MSGSIO_WRITE_MCSTOM:
                  case MSGSIO_WRITE_IADCON:
                  case MSGSIO_WRITE_IADRBE:
                  case MSGSIO_WRITE_IADBOH:
                  case MSGSIO_WRITE_IADTOH:
                  case MSGSIO_WRITE_ICRT:
                  case MSGSIO_WRITE_ICD:
                  case MSGSIO_WRITE_UBCSA:
                  case MSGSIO_WRITE_UBCSB:
                  case MSGSIO_WRITE_MAR1:
                  case MSGSIO_WRITE_MAR2:
                  case MSGSIO_WRITE_MAR3:
                  case MSGSIO_WRITE_DMASCATTERBASEADDR:
                  case MSGSIO_WRITE_PIRQ0:
                  case MSGSIO_WRITE_BIOSTIMERBASEADDR:
                  case MSGSIO_WRITE_SMICNTL:
                  case MSGSIO_WRITE_SEE:
                  case MSGSIO_WRITE_FTMR:
                  case MSGSIO_WRITE_CTLTMRL:
                  case MSGSIO_WRITE_APM:
                     BuildMsgWithParm1(mPci, MSGPCI_SET_CONFIGDATA,
                                             off);
                     BuildPciParm1(mPci, SelectSioParm1(*pmMsg));
                     SendMsg(mPci, oPciSystemioCtl);

                     if((rc = SelectResult(mPci)) != ERRPCI_NOERROR)
                     {
                        rc = ERRSIO_CANNOT_PCIACCESS;
                     } /* endif */
                     break;
                  case MSGSIO_WRITE_ELCR1:
                  case MSGSIO_WRITE_ELCR2:
                     WriteControllerByte(baseport + (DWORD)off,
                                         (BYTE)SelectParm1(*pmMsg));
                     break;
                  default:
                     rc = ERRSIO_INVALID_MESSAGE;
               } /* endswitch */
            } /* endif */
      } /* endswitch */
   }
   else
   {
      rc = ERRSIO_NOT_INITIALIZED;
   } /* endif */

   SelectResult(*pmMsg) = rc;
   return;
}

/* ********************************************* */
/* *       System-I/O Control Controller       * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGSIO_INITIALIZECTL   * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Save device context                     * */
/* *     [ Input ]                             * */
/* *       Message    = MSGSIO_SAVE_CONTEXT    * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Restore device context                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGSIO_RESTORE_CONTEXT * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  SystemioCtl(PMSGCOMMON  pmMsg,     /* Pointer to Common message packet */
                  POBJCOMMON  poObj)             /* Pointer to Common object */
{
   MSGSYSTEMIO       msg;
   CRESULT           rc;
   POBJSYSTEMIOCTL   psioctl;

   /* Get pointer to System-I/O control object */
   /*                                          */
   psioctl = (POBJSYSTEMIOCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGSIO_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGSIO_INITIALIZE);
         SendMsg(msg, oSystemioIo);

         rc = SelectResult(msg);
         break;

      case MSGSIO_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRSIO_NOERROR;

         BuildMsg(msg, MSGSIO_READ_PCICON);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset40h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_MCSCON);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset44h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_IADCON);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset48h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_ICRT);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset4Ch = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_MAR1);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset54h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_PIRQ0);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset60h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_BIOSTIMERBASEADDR);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffset80h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_SMICNTL);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffsetA0h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_SEE);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffsetA4h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_FTMR);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffsetA8h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_CTLTMRL);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffsetACh = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_APM);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->dwOffsetB0h = SelectSioParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_ELCR1);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->chElcr1 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_READ_ELCR2);
         SendMsg(msg, oSystemioIo);
         if(!SelectResult(msg))
         {
            psioctl->chElcr2 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGSIO_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRSIO_NOERROR;

         BuildMsg(msg, MSGSIO_WRITE_PCICON);
         BuildSioParm1(msg, psioctl->dwOffset40h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_MCSCON);
         BuildSioParm1(msg, psioctl->dwOffset44h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_IADCON);
         BuildSioParm1(msg, psioctl->dwOffset48h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_ICRT);
         BuildSioParm1(msg, psioctl->dwOffset4Ch);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_MAR1);
         BuildSioParm1(msg, psioctl->dwOffset54h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_PIRQ0);
         BuildSioParm1(msg, psioctl->dwOffset60h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_BIOSTIMERBASEADDR);
         BuildSioParm1(msg, psioctl->dwOffset80h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_SMICNTL);
         BuildSioParm1(msg, psioctl->dwOffsetA0h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_SEE);
         BuildSioParm1(msg, psioctl->dwOffsetA4h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_FTMR);
         BuildSioParm1(msg, psioctl->dwOffsetA8h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_CTLTMRL);
         BuildSioParm1(msg, psioctl->dwOffsetACh);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGSIO_WRITE_APM);
         BuildSioParm1(msg, psioctl->dwOffsetB0h);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSIO_WRITE_ELCR1,
                                psioctl->chElcr1);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGSIO_WRITE_ELCR2,
                                psioctl->chElcr2);
         SendMsg(msg, oSystemioIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oSystemioIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
