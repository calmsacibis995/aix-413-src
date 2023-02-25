/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Peripheral Component Interconnect (PCI) Bus Control
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
/* * Module Name: PDPCIBUS.C                                               * */
/* * Description: Peripheral Component Interconnect (PCI) Bus Control      * */
/* *              Routines                                                 * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDPCIBUS.H files should be included* */
/* *              in this file                                             * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdpcibus.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *        PCI Data Definition        * */
/* ************************************* */
/* ------------------------------------- */
/*  Configuration register port address  */
/* ------------------------------------- */
#define  PORTPCI_BASE                     (0x00000CF8L)  /* PCI base address */
#define  PORTPCI_ADDRESS                  (PORTPCI_BASE + 0)
                                                         /* Address register */
#define  PORTPCI_DATA                     (PORTPCI_BASE + 4)
                                                            /* Data register */
#define  PORTPCI_CONFIGSPACEBASE          (0x80000000L)
                                         /* Configuration space base address */
#define  PORTPCI_CONFIGSPACESTART         (0x00000000L)
                                        /* Configuration space start address */
#define  PORTPCI_CONFIGSPACEEND           (0x0000F800L)
                                          /* Configuration space end address */
#define  PORTPCI_CONFIGSPACENEXT          (0x00000800L)
                              /* Configuration space delta to next device ID */

/* -------------------------------------- */
/*  Configuration space header structure  */
/* -------------------------------------- */
#pragma pack(1)
typedef struct                             /* PCI configuration space header */
{
   WORD  VendorID;                                              /* Vendor ID */
   WORD  DeviceID;                                              /* Device ID */
   WORD  Command;                                                 /* Command */
   WORD  Status;                                                   /* Status */
   BYTE  RevisionID;                                          /* Revision ID */
   BYTE  ClassCode[3];                                         /* Class code */
   BYTE  CacheLine;                                            /* Cache line */
   BYTE  Latency;                                                 /* Latency */
   BYTE  HeaderType;                                          /* Header type */
   BYTE  Bist;                                                       /* BIST */
   DWORD BaseAddress0;                                     /* Base address 0 */
   DWORD BaseAddress1;                                     /* Base address 1 */
   DWORD BaseAddress2;                                     /* Base address 2 */
   DWORD BaseAddress3;                                     /* Base address 3 */
   DWORD BaseAddress4;                                     /* Base address 4 */
   DWORD BaseAddress5;                                     /* Base address 5 */
   DWORD Reserved1;                                              /* Reserved */
   DWORD Reserved2;                                              /* Reserved */
   DWORD ExpansionRomControl;                       /* Expansion ROM control */
   DWORD ExecutableMemoryBaseAddress;      /* Executable memory base address */
   DWORD Reserved3;                                              /* Reserved */
   BYTE  InterruptLine;                                    /* Interrupt line */
   BYTE  InterruptPin;                                      /* Interrupt pin */
   BYTE  MinGnt;                                                  /* Min_gnt */
   BYTE  MaxLat;                                                  /* Max_lat */
} PCICONFIGSPACE;
typedef PCICONFIGSPACE  *PPCICONFIGSPACE;
#pragma pack()

/* --------------------------------- */
/*  Configuration address structure  */
/* --------------------------------- */
typedef struct                                  /* PCI configuration address */
{
   UINT  Reserved1      : 8;                                     /* Reserved */
   UINT  FunctionNumber : 3;                              /* Function number */
   UINT  DeviceNumber   : 5;                                /* Device number */
   UINT  BusNumber      : 8;                                   /* Bus number */
   UINT  Reserved2      : 8;                                     /* Reserved */
} PCICONFIGADDRESS;
typedef PCICONFIGADDRESS   *PPCICONFIGADDRESS;


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *     PCI Controller Definition     * */
/* ************************************* */
/* --------------- */
/*  PCI I/O class  */
/* --------------- */
VOID  PciIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj);                  /* Pointer to Common object */

/* ------------------- */
/*  PCI control class  */
/* ------------------- */
VOID  PciCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj);                  /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *            PCI Objects            * */
/* ************************************* */
/* --------------- */
/*  PCI I/O class  */
/* --------------- */
OBJPCIIO    oPciIo = {(POBJCTL)PciIo, PORTIO_BASE,
                                      PORTPCI_CONFIGSPACEBASE};

/* ------------------- */
/*  PCI control class  */
/* ------------------- */
OBJPCICTL   oPciCtl            = {(POBJCTL)PciCtl, 0x0000,      /* Vendor ID */
                                                   0x0000,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciIdahoCtl       = {(POBJCTL)PciCtl, 0x1014,      /* Vendor ID */
                                                   0x0015,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciEagleCtl       = {(POBJCTL)PciCtl, 0x1057,      /* Vendor ID */
                                                   0x0001,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciSystemioCtl    = {(POBJCTL)PciCtl, 0x8086,      /* Vendor ID */
                                                   0x0484,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciScsiCtl        = {(POBJCTL)PciCtl, 0x1000,      /* Vendor ID */
                                                   0x0001,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciWd90c24Ctl     = {(POBJCTL)PciCtl, 0x101C,      /* Vendor ID */
                                                   0xC24A,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciS386c928Ctl    = {(POBJCTL)PciCtl, 0x5333,      /* Vendor ID */
                                                   0x88B0,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciS386c864Ctl    = {(POBJCTL)PciCtl, 0x5333,      /* Vendor ID */
                                                   0x88C0,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciWeitekP9100Ctl = {(POBJCTL)PciCtl, 0x100E,      /* Vendor ID */
                                                   0x9100,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciCarreraCtl     = {(POBJCTL)PciCtl, 0x1014,      /* Vendor ID */
                                                   0x001C,      /* Device ID */
                                                   0L};
OBJPCICTL   oPciCaptureCtl     = {(POBJCTL)PciCtl, 0x10ED,      /* Vendor ID */
                                                   0x7310,      /* Device ID */
                                                   0L};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ************************************************ */
/* *              PCI I/O Controller              * */
/* ************************************************ */
/* *                                              * */
/* * - Initialize                                 * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_INITIALIZE         * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Get configuration address register         * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_GET_ADDRESS        * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       PciParm1   = address value             * */
/* *                                              * */
/* * - Set configuration address register         * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_SET_ADDRESS        * */
/* *       PciParm1   = address value             * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Find a PCI device                          * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_FIND_DEVICE        * */
/* *       Parameter1 = vendor ID                 * */
/* *       Parameter2 = device ID                 * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       PciParm1   = config address            * */
/* *                                              * */
/* * - Read configuration space data              * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_READ_CONFIGDATA    * */
/* *       PciParm1   = config address            * */
/* *       Parameter1 = offset                    * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       PciParm1   = data value                * */
/* *                                              * */
/* * - Write configuration space data             * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_WRITE_CONFIGDATA   * */
/* *       PciParm1   = config address            * */
/* *       PciParm2   = data value                * */
/* *       Parameter1 = offset                    * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read PCI command                           * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_READ_COMMAND       * */
/* *       PciParm1   = config address            * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       Parameter1 = command value             * */
/* *                                              * */
/* * - Write PCI command                          * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_WRITE_COMMAND      * */
/* *       PciParm1   = config address            * */
/* *       Parameter1 = command value             * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read base address                          * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_READ_BASEADDRESS0  * */
/* *                    MSGPCI_READ_BASEADDRESS1  * */
/* *       PciParm1   = config address            * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       PciParm1   = base address              * */
/* *                                              * */
/* * - Write base address                         * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_WRITE_BASEADDRESS0 * */
/* *                    MSGPCI_WRITE_BASEADDRESS1 * */
/* *       PciParm1   = config address            * */
/* *       PciParm2   = base address              * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *                                              * */
/* * - Read revision ID                           * */
/* *     [ Input ]                                * */
/* *       Message    = MSGPCI_READ_REVISIONID    * */
/* *       PciParm1   = config address            * */
/* *     [ Output ]                               * */
/* *       Result     = result code               * */
/* *       Parameter1 = revision ID               * */
/* *                                              * */
/* ************************************************ */
VOID  PciIo(PMSGCOMMON  pmMsg,           /* Pointer to Common message packet */
            POBJCOMMON  poObj)                   /* Pointer to Common object */
{
   DWORD             addr, saddr, off, device_vendor;
   PPCICONFIGSPACE   pconfig;
   CRESULT           rc;
   POBJPCIIO         ppciio;
   DWORD             addrport, dataport;
   DWORD             configspace;

   /* Get pointer to PCI I/O object */
   /*                               */
   ppciio = (POBJPCIIO)poObj;

   /* Get address/data port address */
   /*                               */
   addrport = GetIoBaseAddress() + PORTPCI_ADDRESS;
   dataport = GetIoBaseAddress() + PORTPCI_DATA;

   /* Get configuration space base address */
   /*                                      */
   configspace = ppciio->ConfigSpaceBaseAddress;

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPCI_INITIALIZE:
         /* Initialize */
         /*            */
         ppciio->BaseAddress = GetIoBaseAddress();
         /* ppciio->ConfigSpaceBaseAddress = GetPciBaseAddress(); */
         rc = ERRPCI_NOERROR;
         break;

      case MSGPCI_GET_ADDRESS:
         /* Get configuration address register */
         /*                                    */
#if (SYSTEM_BIGENDIAN == TRUE)
         SelectPciParm1(*pmMsg) = ConvEndian32(ReadControllerDWord(addrport));
#else
         SelectPciParm1(*pmMsg) = ReadControllerDWord(addrport);
#endif /* SYSTEM_BIGENDIAN */
         rc = ERRPCI_NOERROR;
         break;

      case MSGPCI_SET_ADDRESS:
         /* Set configuration address register */
         /*                                    */
#if (SYSTEM_BIGENDIAN == TRUE)
         WriteControllerDWord(addrport, ConvEndian32(SelectPciParm1(*pmMsg)));
#else
         WriteControllerDWord(addrport, SelectPciParm1(*pmMsg));
#endif /* SYSTEM_BIGENDIAN */
         rc = ERRPCI_NOERROR;
         break;

      case MSGPCI_FIND_DEVICE:
         /* Save address register */
         /*                       */
         saddr = ReadControllerDWord(addrport);

         /* Find a PCI device */
         /*                   */
         for(pconfig =
             (PPCICONFIGSPACE)(configspace + PORTPCI_CONFIGSPACESTART),
             rc = ERRPCI_DEVICE_NOT_FOUND;
             (pconfig < (PPCICONFIGSPACE)(configspace + PORTPCI_CONFIGSPACEEND))
             && rc;
             pconfig =
             (PPCICONFIGSPACE)((PBYTE)pconfig + PORTPCI_CONFIGSPACENEXT)
            )
         {
#if (SYSTEM_BIGENDIAN == TRUE)
            WriteControllerDWord(addrport,
                                 ConvEndian32((DWORD)(&(pconfig->VendorID))));
            device_vendor = ConvEndian32(ReadControllerDWord(dataport));
#else
            WriteControllerDWord(addrport, (DWORD)(&(pconfig->VendorID)));
            device_vendor = ReadControllerDWord(dataport);
#endif /* SYSTEM_BIGENDIAN */

            if(device_vendor ==
               (((DWORD)SelectParm2(*pmMsg) << 16) |
                 (DWORD)SelectParm1(*pmMsg)))
            {
               SelectPciParm1(*pmMsg) = (DWORD)pconfig;

               rc = ERRPCI_NOERROR;
            } /* endif */
         } /* endfor */

         /* Restore address register */
         /*                          */
         IoDelay();
         WriteControllerDWord(addrport, saddr);
         break;

      default:
         /* Get address register */
         /*                      */
         pconfig = (PPCICONFIGSPACE)(SelectPciParm1(*pmMsg));

         switch(SelectMessage(*pmMsg))
         {
            case MSGPCI_READ_CONFIGDATA:
            case MSGPCI_WRITE_CONFIGDATA:
               addr = ((DWORD)pconfig +
                       (DWORD)SelectParm1(*pmMsg)) & ~0x00000003L;
               off  = ((DWORD)pconfig + (DWORD)SelectParm1(*pmMsg)) - addr;
               rc   = ERRPCI_NOERROR;
               break;
            case MSGPCI_READ_COMMAND:
            case MSGPCI_WRITE_COMMAND:
               addr = (DWORD)(&(pconfig->Command)) & ~0x00000003L;
               off  = (DWORD)(&(pconfig->Command)) - addr;
               rc   = ERRPCI_NOERROR;
               break;
            case MSGPCI_READ_BASEADDRESS0:
            case MSGPCI_WRITE_BASEADDRESS0:
               addr = (DWORD)(&(pconfig->BaseAddress0)) & ~0x00000003L;
               off  = (DWORD)(&(pconfig->BaseAddress0)) - addr;
               rc   = ERRPCI_NOERROR;
               break;
            case MSGPCI_READ_BASEADDRESS1:
            case MSGPCI_WRITE_BASEADDRESS1:
               addr = (DWORD)(&(pconfig->BaseAddress1)) & ~0x00000003L;
               off  = (DWORD)(&(pconfig->BaseAddress1)) - addr;
               rc   = ERRPCI_NOERROR;
               break;
            case MSGPCI_READ_REVISIONID:
               addr = (DWORD)(&(pconfig->RevisionID)) & ~0x00000003L;
               off  = (DWORD)(&(pconfig->RevisionID)) - addr;
               rc   = ERRPCI_NOERROR;
               break;
            default:
               rc = ERRPCI_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
#if (SYSTEM_BIGENDIAN == TRUE)
            addr = ConvEndian32(addr);
#endif /* SYSTEM_BIGENDIAN */

            /* Save address register */
            /*                       */
            saddr = ReadControllerDWord(addrport);

            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case MSGPCI_READ_REVISIONID:
                  WriteControllerDWord(addrport, addr);
#if (SYSTEM_BIGENDIAN == TRUE)
                  SelectParm1(*pmMsg) =
                  (CPARAMETER)(ConvEndian32(ReadControllerByte(dataport)) >>
                               (off * 8));
#else
                  SelectParm1(*pmMsg) =
                        (CPARAMETER)(ReadControllerByte(dataport) >> (off * 8));
#endif /* SYSTEM_BIGENDIAN */
                  break;
               case MSGPCI_READ_COMMAND:
                  WriteControllerDWord(addrport, addr);
#if (SYSTEM_BIGENDIAN == TRUE)
                  SelectParm1(*pmMsg) =
                     (CPARAMETER)(ConvEndian32(ReadControllerDWord(dataport)) >>
                                  (off * 8));
#else
                  SelectParm1(*pmMsg) =
                       (CPARAMETER)(ReadControllerDWord(dataport) >> (off * 8));
#endif /* SYSTEM_BIGENDIAN */
                  break;
               case MSGPCI_READ_CONFIGDATA:
               case MSGPCI_READ_BASEADDRESS0:
               case MSGPCI_READ_BASEADDRESS1:
                  WriteControllerDWord(addrport, addr);
#if (SYSTEM_BIGENDIAN == TRUE)
                  SelectPciParm1(*pmMsg) =
                       ConvEndian32(ReadControllerDWord(dataport)) >> (off * 8);
#else
                  SelectPciParm1(*pmMsg) =
                                     ReadControllerDWord(dataport) >> (off * 8);
#endif /* SYSTEM_BIGENDIAN */
                  break;
               case MSGPCI_WRITE_COMMAND:
                  WriteControllerDWord(addrport, addr);
#if (SYSTEM_BIGENDIAN == TRUE)
                  WriteControllerWord(dataport,
                                      ConvEndian16((WORD)SelectParm1(*pmMsg)));
#else
                  WriteControllerWord(dataport, (WORD)SelectParm1(*pmMsg));
#endif /* SYSTEM_BIGENDIAN */
                  break;
               case MSGPCI_WRITE_CONFIGDATA:
               case MSGPCI_WRITE_BASEADDRESS0:
               case MSGPCI_WRITE_BASEADDRESS1:
                  WriteControllerDWord(addrport, addr);
#if (SYSTEM_BIGENDIAN == TRUE)
                  WriteControllerDWord(dataport,
                     ConvEndian32((ConvEndian32(ReadControllerDWord(dataport)) &
                                  (0xFFFFFFFFL >> ((4 - off) * 8))) |
                                  (SelectPciParm2(*pmMsg) << (off * 8))));
#else
                  WriteControllerDWord(dataport,
                                       (ReadControllerDWord(dataport) &
                                        (0xFFFFFFFFL >> ((4 - off) * 8))) |
                                       (SelectPciParm2(*pmMsg) << (off * 8)));
#endif /* SYSTEM_BIGENDIAN */
                  break;
               default:
                  rc = ERRPCI_INVALID_MESSAGE;
            } /* endswitch */

            /* Restore address register */
            /*                          */
            IoDelay();
            WriteControllerDWord(addrport, saddr);
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* *********************************************** */
/* *           PCI Control Controller            * */
/* *********************************************** */
/* *                                             * */
/* * - Initialize                                * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_INITIALIZECTL     * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Get configuration space address           * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_GET_CONFIGADDRESS * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *       PciParm1   = config address           * */
/* *                                             * */
/* * - Get configuration device number           * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_GET_DEVICENUMBER  * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *       Parameter1 = device number            * */
/* *       Parameter2 = function number          * */
/* *       Parameter3 = bus number               * */
/* *                                             * */
/* * - Get PCI device ID                         * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_GET_DEVICEID      * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *       Parameter1 = device ID                * */
/* *       Parameter2 = vendor ID                * */
/* *                                             * */
/* * - Get base address                          * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_GET_BASEADDRESS0  * */
/* *                    MSGPCI_GET_BASEADDRESS1  * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *       PciParm1   = base address             * */
/* *                                             * */
/* * - Get revision ID                           * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_GET_REVISIONID    * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *       Parameter1 = revision ID              * */
/* *                                             * */
/* * - Get configuration space data              * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_GET_CONFIGDATA    * */
/* *       Parameter1 = offset                   * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *       PciParm1   = data value               * */
/* *                                             * */
/* * - Set configuration space data              * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_SET_CONFIGDATA    * */
/* *       Parameter1 = offset                   * */
/* *       PciParm1   = data value               * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Save device context                       * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_SAVE_CONTEXT      * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Restore device context                    * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_RESTORE_CONTEXT   * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* * - Query device existing                     * */
/* *     [ Input ]                               * */
/* *       Message    = MSGPCI_QUERY_DEVICE      * */
/* *     [ Output ]                              * */
/* *       Result     = result code              * */
/* *                                             * */
/* *********************************************** */
VOID  PciCtl(PMSGCOMMON pmMsg,           /* Pointer to Common message packet */
             POBJCOMMON poObj)                   /* Pointer to Common object */
{
   MSGPCI            msg;
   CRESULT           rc;
   PPCICONFIGSPACE   pconfig = 0;
   POBJPCICTL        ppcictl;

   /* Get pointer to PCI control object */
   /*                                   */
   ppcictl = (POBJPCICTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGPCI_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGPCI_INITIALIZE);
         SendMsg(msg, oPciIo);

         if(!(rc = SelectResult(msg)))
         {
            BuildMsgWithParm2(msg, MSGPCI_FIND_DEVICE,
                                   ppcictl->wVendorID,
                                   ppcictl->wDeviceID);
            SendMsg(msg, oPciIo);

            if(!(rc = SelectResult(msg)))
            {
               ppcictl->dwConfigAddress = SelectPciParm1(msg);
            } /* endif */
         } /* endif */
         break;

      case MSGPCI_GET_CONFIGADDRESS:
         /* Get configuration space address */
         /*                                 */
         SelectPciParm1(*pmMsg) = ppcictl->dwConfigAddress;
         rc = ERRPCI_NOERROR;
         break;

      case MSGPCI_GET_DEVICENUMBER:
         /* Get configuration device number */
         /*                                 */
         SelectParm1(*pmMsg) =
                 (CPARAMETER)
                 ((PPCICONFIGADDRESS)&(ppcictl->dwConfigAddress))->DeviceNumber;
         SelectParm2(*pmMsg) =
               (CPARAMETER)
               ((PPCICONFIGADDRESS)&(ppcictl->dwConfigAddress))->FunctionNumber;
         SelectParm3(*pmMsg) =
                    (CPARAMETER)
                    ((PPCICONFIGADDRESS)&(ppcictl->dwConfigAddress))->BusNumber;
         rc = ERRPCI_NOERROR;
         break;

      case MSGPCI_GET_DEVICEID:
         /* Get PCI device ID */
         /*                   */
         SelectParm1(*pmMsg) = (CPARAMETER)ppcictl->wDeviceID;
         SelectParm2(*pmMsg) = (CPARAMETER)ppcictl->wVendorID;
         rc = ERRPCI_NOERROR;
         break;

      case MSGPCI_GET_BASEADDRESS0:
         /* Get base address 0 */
         /*                    */
         BuildMsg(msg, MSGPCI_READ_BASEADDRESS0);
         BuildPciParm1(msg, ppcictl->dwConfigAddress);
         SendMsg(msg, oPciIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectPciParm1(*pmMsg) = SelectPciParm1(msg);
         } /* endif */
         break;

      case MSGPCI_GET_BASEADDRESS1:
         /* Get base address 1 */
         /*                    */
         BuildMsg(msg, MSGPCI_READ_BASEADDRESS1);
         BuildPciParm1(msg, ppcictl->dwConfigAddress);
         SendMsg(msg, oPciIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectPciParm1(*pmMsg) = SelectPciParm1(msg);
         } /* endif */
         break;

      case MSGPCI_GET_REVISIONID:
         /* Get revision ID */
         /*                 */
         BuildMsg(msg, MSGPCI_READ_REVISIONID);
         BuildPciParm1(msg, ppcictl->dwConfigAddress);
         SendMsg(msg, oPciIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg);
         } /* endif */
         break;

      case MSGPCI_GET_CONFIGDATA:
         /* Get configuration space data */
         /*                              */
         BuildMsgWithParm1(msg, MSGPCI_READ_CONFIGDATA,
                                SelectParm1(*pmMsg));
         BuildPciParm1(msg, ppcictl->dwConfigAddress);
         SendMsg(msg, oPciIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectPciParm1(*pmMsg) = SelectPciParm1(msg);
         } /* endif */
         break;

      case MSGPCI_SET_CONFIGDATA:
         /* Set configuration space data */
         /*                              */
         BuildMsgWithParm1(msg, MSGPCI_WRITE_CONFIGDATA,
                                SelectParm1(*pmMsg));
         BuildPciParm2(msg, ppcictl->dwConfigAddress,
                            SelectPciParm1(*pmMsg));
         SendMsg(msg, oPciIo);

         rc = SelectResult(msg);
         break;

      case MSGPCI_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRPCI_NOERROR;

         BuildMsg(msg, MSGPCI_GET_ADDRESS);
         SendMsg(msg, oPciIo);
         if(!SelectResult(msg))
         {
            ppcictl->dwAddress = SelectPciParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_GET_CONFIGDATA,
                                (DWORD)(&(pconfig->Command )) -
                                (DWORD)(&(pconfig->VendorID)) );
         SendMsg(msg, *poObj);
         if(!SelectResult(msg))
         {
            ppcictl->dwOffset04h = SelectPciParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_GET_CONFIGDATA,
                                (DWORD)(&(pconfig->CacheLine)) -
                                (DWORD)(&(pconfig->VendorID )) );
         SendMsg(msg, *poObj);
         if(!SelectResult(msg))
         {
            ppcictl->dwOffset0Ch = SelectPciParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_GET_CONFIGDATA,
                                (DWORD)(&(pconfig->BaseAddress0)) -
                                (DWORD)(&(pconfig->VendorID    )) );
         SendMsg(msg, *poObj);
         if(!SelectResult(msg))
         {
            ppcictl->dwOffset10h = SelectPciParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_GET_CONFIGDATA,
                                (DWORD)(&(pconfig->BaseAddress1)) -
                                (DWORD)(&(pconfig->VendorID    )) );
         SendMsg(msg, *poObj);
         if(!SelectResult(msg))
         {
            ppcictl->dwOffset14h = SelectPciParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_GET_CONFIGDATA,
                                (DWORD)(&(pconfig->InterruptLine)) -
                                (DWORD)(&(pconfig->VendorID     )) );
         SendMsg(msg, *poObj);
         if(!SelectResult(msg))
         {
            ppcictl->dwOffset3Ch = SelectPciParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGPCI_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRPCI_NOERROR;

         BuildMsgWithParm1(msg, MSGPCI_SET_CONFIGDATA,
                                (DWORD)(&(pconfig->CacheLine)) -
                                (DWORD)(&(pconfig->VendorID )) );
         BuildPciParm1(msg, ppcictl->dwOffset0Ch);
         SendMsg(msg, *poObj);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_SET_CONFIGDATA,
                                (DWORD)(&(pconfig->BaseAddress0)) -
                                (DWORD)(&(pconfig->VendorID    )) );
         BuildPciParm1(msg, ppcictl->dwOffset10h);
         SendMsg(msg, *poObj);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_SET_CONFIGDATA,
                                (DWORD)(&(pconfig->BaseAddress1)) -
                                (DWORD)(&(pconfig->VendorID    )) );
         BuildPciParm1(msg, ppcictl->dwOffset14h);
         SendMsg(msg, *poObj);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_SET_CONFIGDATA,
                                (DWORD)(&(pconfig->InterruptLine)) -
                                (DWORD)(&(pconfig->VendorID     )) );
         BuildPciParm1(msg, ppcictl->dwOffset3Ch);
         SendMsg(msg, *poObj);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGPCI_SET_CONFIGDATA,
                                (DWORD)(&(pconfig->Command )) -
                                (DWORD)(&(pconfig->VendorID)) );
         BuildPciParm1(msg, ppcictl->dwOffset04h);
         SendMsg(msg, *poObj);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGPCI_SET_ADDRESS);
         BuildPciParm1(msg, ppcictl->dwAddress);
         SendMsg(msg, oPciIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGPCI_QUERY_DEVICE:
         /* Query device existing */
         /*                       */
         rc = (ppcictl->dwConfigAddress)
              ? (CRESULT)ERRPCI_NOERROR
              : (CRESULT)ERRPCI_DEVICE_NOT_FOUND;
         break;

      default:
         SendMsg(*pmMsg, oPciIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
