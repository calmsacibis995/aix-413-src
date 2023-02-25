static char sccsid[] = "@(#)45  1.4  src/rspc/kernext/pm/pmmd/6030/pdcooper.c, pwrsysx, rspc41J, 9523B_all 6/6/95 19:41:06";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Cooper I/O Support Chip Device Control Routines
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
/* * Module Name: PDCOOPER.C                                               * */
/* * Description: Cooper I/O Support Chip Device Control Routines          * */
/* * Restriction: PDPLANAR.H/PDCOMMON.H/PDCOOPER.H/PDMEMCTL.H files should * */
/* *              be included in this file                                 * */
/* *                                                                       * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdcooper.h"
#include "pdmemctl.h"


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *      Cooper Data Definition       * */
/* ************************************* */
/* ------------------------------ */
/*  Planar register port address  */
/* ------------------------------ */
#define  PORTCOOP_STORAGELIGHT            (0x00000808L)
                                                   /* Storage light register */
#define  PORTCOOP_EQUIPMENTPRESENT        (0x0000080CL)
                                               /* Equipment present register */
#define  PORTCOOP_PASSWORDPROTECT1        (0x00000810L)
                                              /* Password protect 1 register */
#define  PORTCOOP_PASSWORDPROTECT2        (0x00000812L)
                                              /* Password protect 2 register */
#define  PORTCOOP_L2FLUSH                 (0x00000814L)
                                                        /* L2 flush register */
#define  PORTCOOP_SYSTEMCONTROL           (0x0000081CL)
                                                  /* System control register */
#define  PORTCOOP_EOIA                    (0x00000838L)
                                                    /* End of INT-A register */
#define  PORTCOOP_PCIINTAMAP              (0x00000839L)
                                               /* PCI INT-A mapping register */
#define  PORTCOOP_EOIB                    (0x0000083CL)
                                                    /* End of INT-B register */
#define  PORTCOOP_PCIINTBMAP              (0x0000083DL)
                                               /* PCI INT-B mapping register */
#define  PORTCOOP_AUDIOSUPPORT            (0x0000083EL)
                                                   /* Audio support register */
#define  PORTCOOP_SIMM1MEMORYID           (0x00000880L)
                                           /* SIMM 1 base memory ID register */
#define  PORTCOOP_SIMM2MEMORYID           (0x00000884L)
                                           /* SIMM 2 base memory ID register */
#define  PORTCOOP_SIMM3MEMORYID           (0x00000888L)
                                                /* SIMM 3 memory ID register */
#define  PORTCOOP_SIMM4MEMORYID           (0x0000088CL)
                                                /* SIMM 4 memory ID register */
#define  PORTCOOP_CMOSSECURITY            (0x000008A0L)
                                                   /* CMOS security register */
#define  PORTCOOP_PLANARSETUPLOCK         (0x000008A1L)
                                        /* Planar setup lock/unlock register */
#define  PORTCOOP_MONITORID               (0x000008A4L)
                                                      /* Monitor ID register */
#define  PORTCOOP_SCSISECURITY            (0x000008A8L)
                                                   /* SCSI security register */
#define  PORTCOOP_STORAGEPRESENCE         (0x000008A9L)
                                         /* Storage device presence register */
#define  PORTCOOP_ADAPTERPRESENCE         (0x000008AAL)
                                         /* Adapter presence detect register */
#define  PORTCOOP_SCSIIDSETUP             (0x000008ABL)
                                                   /* SCSI ID setup register */
#define  PORTCOOP_PCICINTMAPPING          (0x000008ACL)
                                          /* PCIC interrupt mapping register */
#define  PORTCOOP_PCICINTSTATUS           (0x000008ADL)
                                           /* PCIC interrupt status register */

/* ----------------------------- */
/*  FDD controller port address  */
/* ----------------------------- */
#define  PORTCOOP_FDCSUPPORT              (0x000003F3L)
                                                     /* FDC support register */

/* ------------------------------------- */
/*  VGA flat panel control port address  */
/* ------------------------------------- */
#define  PORTCOOP_PANELINDEX              (0x00000D00L)
                                                     /* Panel index register */
#define  PORTCOOP_PANELDATA               (0x00000D01L)
                                                      /* Panel data register */

/* -------------------------------------------------- */
/*  Index data to index flat panel control registers  */
/* -------------------------------------------------- */
#define  INDEXCOOP_PANELGRAYLEVEL         (0x10)
                                            /* Gray level selection register */
#define  INDEXCOOP_PANELSIGNALSOURCE      (0x11)
                              /* Flat panel signal source selection register */
#define  INDEXCOOP_PANELBULLETINBOARD1    (0x12)
                                                /* Bulletin board 1 register */
#define  INDEXCOOP_PANELBULLETINBOARD2    (0x13)
                                                /* Bulletin board 2 register */
#define  INDEXCOOP_PANELBULLETINBOARD3    (0x14)
                                                /* Bulletin board 3 register */
#define  INDEXCOOP_PANELBULLETINBOARD4    (0x15)
                                                /* Bulletin board 4 register */
#define  INDEXCOOP_PANELTYPE              (0xFF)
                                                 /* Flat panel type register */

/* ---------------------------------------------------- */
/*  System control register (SYSTEMCONTROL) bit assign  */
/* ---------------------------------------------------- */
#define  BITCOOPSYSCONTROL_RESERVED0      (0x01)
                                       /* (Bit-0) Reserved                   */
#define  BITCOOPSYSCONTROL_RESERVED1      (0x02)
                                       /* (Bit-1) Reserved                   */
#define  BITCOOPSYSCONTROL_RESERVED2      (0x04)
                                       /* (Bit-2) Reserved                   */
#define  BITCOOPSYSCONTROL_RESERVED3      (0x08)
                                       /* (Bit-3) Reserved                   */
#define  BITCOOPSYSCONTROL_L2FLUSH        (0x10)
                                       /* (Bit-4) L2 cache flush             */
                                       /*         (negatd(1) to asserted(0)) */
#define  BITCOOPSYSCONTROL_MASKTEA        (0x20)
                                       /* (Bit-5) MASK_TEA (not supported)   */
#define  BITCOOPSYSCONTROL_L2CACHEDISABLE (0x40)
                                       /* (Bit-6) L2 cache disable           */
                                       /*         (0=disable, 1=enable)      */
#define  BITCOOPSYSCONTROL_L2CACHEINHIBIT (0x80)
                                       /* (Bit-7) L2 cache miss inhibit      */
                                       /*         (0=bypass, 1=updated)      */

/* -------------------------------------------------- */
/*  Audio support register (AUDIOSUPPORT) bit assign  */
/* -------------------------------------------------- */
#define  BITCOOPAUDIOSUPPORT_RESERVED0    (0x01)
                                            /* (Bit-0) Reserved              */
#define  BITCOOPAUDIOSUPPORT_SPEAKERMUTE  (0x02)
                                            /* (Bit-1) Speaker mute          */
#define  BITCOOPAUDIOSUPPORT_SPEAKERLED0  (0x04)
                                            /* (Bit-2) Speaker LED control 0 */
#define  BITCOOPAUDIOSUPPORT_SPEAKERLED1  (0x08)
                                            /* (Bit-3) Speaker LED control 1 */
#define  BITCOOPAUDIOSUPPORT_RESERVED4    (0x10)
                                            /* (Bit-4) Reserved              */
#define  BITCOOPAUDIOSUPPORT_RESERVED5    (0x20)
                                            /* (Bit-5) Reserved              */
#define  BITCOOPAUDIOSUPPORT_RESERVED6    (0x40)
                                            /* (Bit-6) Reserved              */
#define  BITCOOPAUDIOSUPPORT_RESERVED7    (0x80)
                                            /* (Bit-7) Reserved              */

/* ---------------------------------------------------------------- */
/*  Planar setup lock/unlock register (PLANARSETUPLOCK) bit assign  */
/* ---------------------------------------------------------------- */
#define  BITCOOPPLANARSETUPLOCK_LOCK      (0x01)
                                       /* (Bit-0) Planar setup register lock */
                                       /*         (1=lock, 0=unlock)         */
#define  BITCOOPPLANARSETUPLOCK_RESERVED1 (0x02)
                                       /* (Bit-1) Reserved                   */
#define  BITCOOPPLANARSETUPLOCK_RESERVED2 (0x04)
                                       /* (Bit-2) Reserved                   */
#define  BITCOOPPLANARSETUPLOCK_RESERVED3 (0x08)
                                       /* (Bit-3) Reserved                   */
#define  BITCOOPPLANARSETUPLOCK_RESERVED4 (0x10)
                                       /* (Bit-4) Reserved                   */
#define  BITCOOPPLANARSETUPLOCK_RESERVED5 (0x20)
                                       /* (Bit-5) Reserved                   */
#define  BITCOOPPLANARSETUPLOCK_RESERVED6 (0x40)
                                       /* (Bit-6) Reserved                   */
#define  BITCOOPPLANARSETUPLOCK_RESERVED7 (0x80)
                                       /* (Bit-7) Reserved                   */

/* -------------------------------------------- */
/*  Monitor ID register (MONITORID) bit assign  */
/* -------------------------------------------- */
#define  BITCOOPMONITORID_ID              (0x0F)
                                                     /* (Bit-0:3) Monitor ID */
#define  BITCOOPMONITORID_RESERVED4       (0x10)
                                                     /* (Bit-4) Reserved     */
#define  BITCOOPMONITORID_RESERVED5       (0x20)
                                                     /* (Bit-5) Reserved     */
#define  BITCOOPMONITORID_RESERVED6       (0x40)
                                                     /* (Bit-6) Reserved     */
#define  BITCOOPMONITORID_RESERVED7       (0x80)
                                                     /* (Bit-7) Reserved     */

/* --------------------------------------------------------------- */
/*  Storage device presence register (STORAGEPRESENCE) bit assign  */
/* --------------------------------------------------------------- */
#define  BITCOOPSTORAGEPRESENCE_INTSCSI0  (0x01)
                                    /* (Bit-0) Internal SCSI device 0 detect */
                                    /*         (1=present, 0=not present)    */
#define  BITCOOPSTORAGEPRESENCE_INTSCSI1  (0x02)
                                    /* (Bit-1) Internal SCSI device 1 detect */
                                    /*         (1=present, 0=not present)    */
#define  BITCOOPSTORAGEPRESENCE_EXTSCSI   (0x04)
                                    /* (Bit-2) External SCSI device detect   */
                                    /*         (1=present, 0=not present)    */
#define  BITCOOPSTORAGEPRESENCE_INTDISKT  (0x08)
                                    /* (Bit-3) Internal diskette detect      */
                                    /*         (1=present, 0=not present)    */
#define  BITCOOPSTORAGEPRESENCE_EXTDISKT  (0x10)
                                    /* (Bit-4) External diskette detect      */
                                    /*         (1=present, 0=not present)    */
#define  BITCOOPSTORAGEPRESENCE_RESERVED5 (0x20)
                                    /* (Bit-5) Reserved                      */
#define  BITCOOPSTORAGEPRESENCE_RESERVED6 (0x40)
                                    /* (Bit-6) Reserved                      */
#define  BITCOOPSTORAGEPRESENCE_RESERVED7 (0x80)
                                    /* (Bit-7) Reserved                      */

/* ------------------------------------------------- */
/*  Flat panel type register (PANELTYPE) bit assign  */
/* ------------------------------------------------- */
#define  BITCOOPPANELTYPE_TYPE            (0x0F)
                                        /* (Bit-0:3) Flat panel type         */
#define     BITCOOPPANELTYPE_NOTATTACH    (0x0F)
                                        /*           Unknown or not attached */
#define     BITCOOPPANELTYPE_TFTC640480   (0x0E)
                                        /*           TFT color (640x480)     */
#define     BITCOOPPANELTYPE_STNC640480   (0x0D)
                                        /*           STN color (640x480)     */
#define     BITCOOPPANELTYPE_TFTC800600   (0x0C)
                                        /*           TFT color (800x600)     */
#define  BITCOOPPANELTYPE_RESERVED4       (0x10)
                                        /* (Bit-4) Reserved                  */
#define  BITCOOPPANELTYPE_RESERVED5       (0x20)
                                        /* (Bit-5) Reserved                  */
#define  BITCOOPPANELTYPE_RESERVED6       (0x40)
                                        /* (Bit-6) Reserved                  */
#define  BITCOOPPANELTYPE_RESERVED7       (0x80)
                                        /* (Bit-7) Reserved                  */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *   Cooper Controller Definition    * */
/* ************************************* */
/* ------------------ */
/*  Cooper I/O class  */
/* ------------------ */
VOID  CooperIo(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj);               /* Pointer to Common object */

/* ---------------------- */
/*  Cooper control class  */
/* ---------------------- */
VOID  CooperCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj);               /* Pointer to Common object */


/* ************************************************************************* */
/* *                             Data Section                              * */
/* ************************************************************************* */
/* ************************************* */
/* *          Cooper Objects           * */
/* ************************************* */
/* ------------------ */
/*  Cooper I/O class  */
/* ------------------ */
OBJCOOPERIO    oCooperIo  = {(POBJCTL)CooperIo, PORTISA_BASE};

/* ---------------------- */
/*  Cooper control class  */
/* ---------------------- */
OBJCOOPERCTL   oCooperCtl = {(POBJCTL)CooperCtl};


/* ************************************************************************* */
/* *                             Code Section                              * */
/* ************************************************************************* */
/* ********************************************* */
/* *           Cooper I/O Controller           * */
/* ********************************************* */
/* *                                           * */
/* * - Initialize                              * */
/* *     [ Input ]                             * */
/* *       Message    = MSGCOOP_INITIALIZE     * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Get index register                      * */
/* *     [ Input ]                             * */
/* *       Message    = MSGCOOP_GET_PANELINDEX * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *       Parameter1 = index value            * */
/* *                                           * */
/* * - Set index register                      * */
/* *     [ Input ]                             * */
/* *       Message    = MSGCOOP_SET_PANELINDEX * */
/* *       Parameter1 = index value            * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Read register                           * */
/* *     [ Input ]                             * */
/* *       Message    = MSGCOOP_READ_xxx       * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *       Parameter1 = register value         * */
/* *                                           * */
/* * - Write register                          * */
/* *     [ Input ]                             * */
/* *       Message    = MSGCOOP_WRITE_xxx      * */
/* *       Parameter1 = register value         * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* * - Read & Modify register                  * */
/* *     [ Input ]                             * */
/* *       Message    = MSGCOOP_MODIFY_xxx     * */
/* *       Parameter1 = set bits value         * */
/* *       Parameter2 = mask bits value        * */
/* *     [ Output ]                            * */
/* *       Result     = result code            * */
/* *                                           * */
/* ********************************************* */
VOID  CooperIo(PMSGCOMMON  pmMsg,        /* Pointer to Common message packet */
               POBJCOMMON  poObj)                /* Pointer to Common object */
{
   BYTE           index, sindex;
   DWORD          datap, indexp;
   CRESULT        rc;
   POBJCOOPERIO   pcoopio;
   DWORD          baseport;

   /* Get pointer to Cooper I/O object */
   /*                                  */
   pcoopio = (POBJCOOPERIO)poObj;

   /* Get device base-address */
   /*                         */
   baseport = GetIsaBaseAddress();

   /* Control registers */
   /*                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGCOOP_INITIALIZE:
         /* Initialize */
         /*            */
         pcoopio->BaseAddress = GetIsaBaseAddress();
         rc = ERRCOOP_NOERROR;
         break;

      default:
         /* Get data-port register */
         /*   or index register    */
         /*                        */
         switch(SelectMessage(*pmMsg))
         {
            case MSGCOOP_READ_STORAGELIGHT:
            case MSGCOOP_WRITE_STORAGELIGHT:
               datap = PORTCOOP_STORAGELIGHT;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_EQUIPMENTPRESENT:
               datap = PORTCOOP_EQUIPMENTPRESENT;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_WRITE_PASSWORDPROTECT1:
               datap = PORTCOOP_PASSWORDPROTECT1;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_WRITE_PASSWORDPROTECT2:
               datap = PORTCOOP_PASSWORDPROTECT2;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_WRITE_L2FLUSH:
               datap = PORTCOOP_L2FLUSH;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SYSTEMCONTROL:
            case MSGCOOP_WRITE_SYSTEMCONTROL:
            case MSGCOOP_MODIFY_SYSTEMCONTROL:
               datap = PORTCOOP_SYSTEMCONTROL;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_WRITE_EOIA:
               datap = PORTCOOP_EOIA;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PCIINTAMAP:
            case MSGCOOP_WRITE_PCIINTAMAP:
               datap = PORTCOOP_PCIINTAMAP;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_WRITE_EOIB:
               datap = PORTCOOP_EOIB;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PCIINTBMAP:
            case MSGCOOP_WRITE_PCIINTBMAP:
               datap = PORTCOOP_PCIINTBMAP;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_AUDIOSUPPORT:
            case MSGCOOP_WRITE_AUDIOSUPPORT:
            case MSGCOOP_MODIFY_AUDIOSUPPORT:
               datap = PORTCOOP_AUDIOSUPPORT;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SIMM1MEMORYID:
               datap = PORTCOOP_SIMM1MEMORYID;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SIMM2MEMORYID:
               datap = PORTCOOP_SIMM2MEMORYID;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SIMM3MEMORYID:
               datap = PORTCOOP_SIMM3MEMORYID;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SIMM4MEMORYID:
               datap = PORTCOOP_SIMM4MEMORYID;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_CMOSSECURITY:
            case MSGCOOP_WRITE_CMOSSECURITY:
               datap = PORTCOOP_CMOSSECURITY;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PLANARSETUPLOCK:
            case MSGCOOP_WRITE_PLANARSETUPLOCK:
            case MSGCOOP_MODIFY_PLANARSETUPLOCK:
               datap = PORTCOOP_PLANARSETUPLOCK;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_MONITORID:
            case MSGCOOP_WRITE_MONITORID:
               datap = PORTCOOP_MONITORID;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SCSISECURITY:
            case MSGCOOP_WRITE_SCSISECURITY:
               datap = PORTCOOP_SCSISECURITY;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_STORAGEPRESENCE:
               datap = PORTCOOP_STORAGEPRESENCE;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_ADAPTERPRESENCE:
               datap = PORTCOOP_ADAPTERPRESENCE;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_SCSIIDSETUP:
            case MSGCOOP_WRITE_SCSIIDSETUP:
               datap = PORTCOOP_SCSIIDSETUP;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PCICINTMAPPING:
            case MSGCOOP_WRITE_PCICINTMAPPING:
               datap = PORTCOOP_PCICINTMAPPING;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PCICINTSTATUS:
               datap = PORTCOOP_PCICINTSTATUS;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_FDCSUPPORT:
            case MSGCOOP_WRITE_FDCSUPPORT:
               datap = PORTCOOP_FDCSUPPORT;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_GET_PANELINDEX:
            case MSGCOOP_SET_PANELINDEX:
               datap = PORTCOOP_PANELINDEX;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELGRAYLEVEL:
            case MSGCOOP_WRITE_PANELGRAYLEVEL:
               index = INDEXCOOP_PANELGRAYLEVEL;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELSIGNALSOURCE:
            case MSGCOOP_WRITE_PANELSIGNALSOURCE:
               index = INDEXCOOP_PANELSIGNALSOURCE;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELBULLETINBD1:
            case MSGCOOP_WRITE_PANELBULLETINBD1:
               index = INDEXCOOP_PANELBULLETINBOARD1;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELBULLETINBD2:
            case MSGCOOP_WRITE_PANELBULLETINBD2:
               index = INDEXCOOP_PANELBULLETINBOARD2;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELBULLETINBD3:
            case MSGCOOP_WRITE_PANELBULLETINBD3:
               index = INDEXCOOP_PANELBULLETINBOARD3;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELBULLETINBD4:
            case MSGCOOP_WRITE_PANELBULLETINBD4:
               index = INDEXCOOP_PANELBULLETINBOARD4;
               rc    = ERRCOOP_NOERROR;
               break;
            case MSGCOOP_READ_PANELTYPE:
               index = INDEXCOOP_PANELTYPE;
               rc    = ERRCOOP_NOERROR;
               break;
            default:
               rc = ERRCOOP_INVALID_MESSAGE;
         } /* endswitch */

         if(!rc)
         {
            /* Input/Output registers */
            /*                        */
            switch(SelectMessage(*pmMsg))
            {
               case MSGCOOP_READ_STORAGELIGHT:
               case MSGCOOP_READ_EQUIPMENTPRESENT:
               case MSGCOOP_READ_SYSTEMCONTROL:
               case MSGCOOP_READ_PCIINTAMAP:
               case MSGCOOP_READ_PCIINTBMAP:
               case MSGCOOP_READ_AUDIOSUPPORT:
               case MSGCOOP_READ_SIMM1MEMORYID:
               case MSGCOOP_READ_SIMM2MEMORYID:
               case MSGCOOP_READ_SIMM3MEMORYID:
               case MSGCOOP_READ_SIMM4MEMORYID:
               case MSGCOOP_READ_CMOSSECURITY:
               case MSGCOOP_READ_PLANARSETUPLOCK:
               case MSGCOOP_READ_MONITORID:
               case MSGCOOP_READ_SCSISECURITY:
               case MSGCOOP_READ_STORAGEPRESENCE:
               case MSGCOOP_READ_ADAPTERPRESENCE:
               case MSGCOOP_READ_SCSIIDSETUP:
               case MSGCOOP_READ_PCICINTMAPPING:
               case MSGCOOP_READ_PCICINTSTATUS:
               case MSGCOOP_READ_FDCSUPPORT:
               case MSGCOOP_GET_PANELINDEX:
                  SelectParm1(*pmMsg) =
                               (CPARAMETER)ReadControllerByte(baseport + datap);
                  break;
               case MSGCOOP_WRITE_STORAGELIGHT:
               case MSGCOOP_WRITE_PASSWORDPROTECT1:
               case MSGCOOP_WRITE_PASSWORDPROTECT2:
               case MSGCOOP_WRITE_L2FLUSH:
               case MSGCOOP_WRITE_SYSTEMCONTROL:
               case MSGCOOP_WRITE_EOIA:
               case MSGCOOP_WRITE_PCIINTAMAP:
               case MSGCOOP_WRITE_EOIB:
               case MSGCOOP_WRITE_PCIINTBMAP:
               case MSGCOOP_WRITE_AUDIOSUPPORT:
               case MSGCOOP_WRITE_CMOSSECURITY:
               case MSGCOOP_WRITE_PLANARSETUPLOCK:
               case MSGCOOP_WRITE_MONITORID:
               case MSGCOOP_WRITE_SCSISECURITY:
               case MSGCOOP_WRITE_SCSIIDSETUP:
               case MSGCOOP_WRITE_PCICINTMAPPING:
               case MSGCOOP_WRITE_FDCSUPPORT:
               case MSGCOOP_SET_PANELINDEX:
                  WriteControllerByte(baseport + datap,
                                      (BYTE)SelectParm1(*pmMsg));
                  break;
               case MSGCOOP_MODIFY_SYSTEMCONTROL:
               case MSGCOOP_MODIFY_AUDIOSUPPORT:
               case MSGCOOP_MODIFY_PLANARSETUPLOCK:
                  WriteControllerByte(baseport + datap,
                  (BYTE)((ReadControllerByte(baseport + datap) &
                          ~SelectParm2(*pmMsg)                 ) |
                         (SelectParm1(*pmMsg)                  &
                          SelectParm2(*pmMsg)                  ) ));
                  break;
               default:
                  /* Get panel index/data port address */
                  /*                                   */
                  indexp = baseport + PORTCOOP_PANELINDEX;
                  datap  = baseport + PORTCOOP_PANELDATA;

                  /* Save index register */
                  /*                     */
                  sindex = ReadControllerByte(indexp);

                  /* Input/Output registers */
                  /*                        */
                  switch(SelectMessage(*pmMsg))
                  {
                     case MSGCOOP_READ_PANELGRAYLEVEL:
                     case MSGCOOP_READ_PANELSIGNALSOURCE:
                     case MSGCOOP_READ_PANELBULLETINBD1:
                     case MSGCOOP_READ_PANELBULLETINBD2:
                     case MSGCOOP_READ_PANELBULLETINBD3:
                     case MSGCOOP_READ_PANELBULLETINBD4:
                     case MSGCOOP_READ_PANELTYPE:
                        WriteControllerByte(indexp, index);
                        SelectParm1(*pmMsg) =
                                          (CPARAMETER)ReadControllerByte(datap);
                        break;
                     case MSGCOOP_WRITE_PANELGRAYLEVEL:
                     case MSGCOOP_WRITE_PANELSIGNALSOURCE:
                     case MSGCOOP_WRITE_PANELBULLETINBD1:
                     case MSGCOOP_WRITE_PANELBULLETINBD2:
                     case MSGCOOP_WRITE_PANELBULLETINBD3:
                     case MSGCOOP_WRITE_PANELBULLETINBD4:
                        WriteControllerByte(indexp, index);
                        WriteControllerByte(datap,  (BYTE)SelectParm1(*pmMsg));
                        break;
                     default:
                        rc = ERRCOOP_INVALID_MESSAGE;
                  } /* endswitch */

                  /* Restore index register */
                  /*                        */
                  IoDelay();
                  WriteControllerByte(indexp, sindex);
            } /* endswitch */
         } /* endif */
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}

/* **************************************************** */
/* *            Cooper Control Controller             * */
/* **************************************************** */
/* *                                                  * */
/* * - Initialize                                     * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_INITIALIZECTL         * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get planar setup status                        * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_GET_PLANARSETUPSTATUS * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_SETUPLOCK  )         * */
/* *                    (PRMCOOP_SETUPUNLOCK)         * */
/* *                                                  * */
/* * - Set planar setup status                        * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_SET_PLANARSETUPSTATUS * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_SETUPLOCK  )         * */
/* *                    (PRMCOOP_SETUPUNLOCK)         * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Get speaker mute status                        * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_GET_MUTESTATUS        * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_MUTEON )             * */
/* *                    (PRMCOOP_MUTEOFF)             * */
/* *                                                  * */
/* * - Set speaker mute status                        * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_SET_MUTESTATUS        * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_MUTEON )             * */
/* *                    (PRMCOOP_MUTEOFF)             * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Query monitor ID                               * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_QUERY_MONITORID       * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = id                            * */
/* *                                                  * */
/* * - Query flat panel type                          * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_QUERY_PANELTYPE       * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = type                          * */
/* *       Parameter2 = resolution (horizontal)       * */
/* *       Parameter3 = resolution (vertical)         * */
/* *                                                  * */
/* * - Query external SCSI device presence            * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_QUERY_EXTERNALSCSI    * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_DEVICEPRESENT   )    * */
/* *                    (PRMCOOP_DEVICENOTPRESENT)    * */
/* *                                                  * */
/* * - Get L2 cache status                            * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_GET_L2CACHESTATUS     * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_CACHEENABLE )        * */
/* *                    (PRMCOOP_CACHEDISABLE)        * */
/* *                                                  * */
/* * - Set L2 cache status                            * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_SET_L2CACHESTATUS     * */
/* *       Parameter1 = status                        * */
/* *                    (PRMCOOP_CACHEENABLE )        * */
/* *                    (PRMCOOP_CACHEDISABLE)        * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Flush L2 cache                                 * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_FLUSH_L2CACHE         * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Flush and disable L2 cache                     * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_FLUSHDISABLE_L2CACHE  * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Reset L2 TAG RAM data                          * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_RESET_L2TAGRAM        * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Save device context                            * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_SAVE_CONTEXT          * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* * - Restore device context                         * */
/* *     [ Input ]                                    * */
/* *       Message    = MSGCOOP_RESTORE_CONTEXT       * */
/* *     [ Output ]                                   * */
/* *       Result     = result code                   * */
/* *                                                  * */
/* **************************************************** */
VOID  CooperCtl(PMSGCOMMON pmMsg,        /* Pointer to Common message packet */
                POBJCOMMON poObj)                /* Pointer to Common object */
{
   MSGCOOPER      msg;
   MSGMEMORY      mMem;
   CRESULT        rc;
   POBJCOOPERCTL  pcoopctl;

   /* Get pointer to Cooper control object */
   /*                                      */
   pcoopctl = (POBJCOOPERCTL)poObj;

   /* Control specific bits in register */
   /*                                   */
   switch(SelectMessage(*pmMsg))
   {
      case MSGCOOP_INITIALIZECTL:
         /* Initialize for control class */
         /*                              */
         BuildMsg(msg, MSGCOOP_INITIALIZE);
         SendMsg(msg, oCooperIo);

         rc = SelectResult(msg);
         break;

      case MSGCOOP_GET_PLANARSETUPSTATUS:
         /* Get planar setup status */
         /*                         */
         BuildMsg(msg, MSGCOOP_READ_PLANARSETUPLOCK);
         SendMsg(msg, oCooperIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCOOPPLANARSETUPLOCK_LOCK)
                                  ? (CPARAMETER)PRMCOOP_SETUPLOCK
                                  : (CPARAMETER)PRMCOOP_SETUPUNLOCK;
         } /* endif */
         break;

      case MSGCOOP_SET_PLANARSETUPSTATUS:
         /* Set planar setup status */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCOOP_SETUPLOCK:
               BuildMsgWithParm2(msg, MSGCOOP_MODIFY_PLANARSETUPLOCK,
                                      BITCOOPPLANARSETUPLOCK_LOCK,
                                      BITCOOPPLANARSETUPLOCK_LOCK);
               SendMsg(msg, oCooperIo);

               rc = SelectResult(msg);
               break;
            case PRMCOOP_SETUPUNLOCK:
               BuildMsgWithParm2(msg, MSGCOOP_MODIFY_PLANARSETUPLOCK,
                                      0,
                                      BITCOOPPLANARSETUPLOCK_LOCK);
               SendMsg(msg, oCooperIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCOOP_INVALID_PARAMETER;
         } /* endswitch */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGCOOP_GET_MUTESTATUS:
         /* Get speaker mute status */
         /*                         */
         BuildMsg(msg, MSGCOOP_READ_AUDIOSUPPORT);
         SendMsg(msg, oCooperIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCOOPAUDIOSUPPORT_SPEAKERMUTE)
                                  ? (CPARAMETER)PRMCOOP_MUTEON
                                  : (CPARAMETER)PRMCOOP_MUTEOFF;
         } /* endif */
         break;

      case MSGCOOP_SET_MUTESTATUS:
         /* Set speaker mute status */
         /*                         */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCOOP_MUTEON:
               BuildMsgWithParm2(msg, MSGCOOP_MODIFY_AUDIOSUPPORT,
                                      BITCOOPAUDIOSUPPORT_SPEAKERMUTE,
                                      BITCOOPAUDIOSUPPORT_SPEAKERMUTE);
               SendMsg(msg, oCooperIo);

               rc = SelectResult(msg);
               break;
            case PRMCOOP_MUTEOFF:
               BuildMsgWithParm2(msg, MSGCOOP_MODIFY_AUDIOSUPPORT,
                                      0,
                                      BITCOOPAUDIOSUPPORT_SPEAKERMUTE);
               SendMsg(msg, oCooperIo);

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCOOP_INVALID_PARAMETER;
         } /* endswitch */
         break;

      case MSGCOOP_QUERY_MONITORID:
         /* Query monitor ID */
         /*                  */
         BuildMsg(msg, MSGCOOP_READ_MONITORID);
         SendMsg(msg, oCooperIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = SelectParm1(msg) & BITCOOPMONITORID_ID;
         } /* endif */
         break;

      case MSGCOOP_QUERY_PANELTYPE:
         /* Query flat panel type */
         /*                       */
         BuildMsg(msg, MSGCOOP_READ_PANELTYPE);
         SendMsg(msg, oCooperIo);

         if(!(rc = SelectResult(msg)))
         {
            switch(SelectParm1(msg) & BITCOOPPANELTYPE_TYPE)
            {
               case BITCOOPPANELTYPE_NOTATTACH:
                  SelectParm1(*pmMsg) = PRMCOOP_TYPENOTATTACH;
                  SelectParm2(*pmMsg) = 0;
                  SelectParm3(*pmMsg) = 0;
                  break;
               case BITCOOPPANELTYPE_TFTC640480:
                  SelectParm1(*pmMsg) = PRMCOOP_TYPETFTCOLOR;
                  SelectParm2(*pmMsg) = 640;
                  SelectParm3(*pmMsg) = 480;
                  break;
               case BITCOOPPANELTYPE_STNC640480:
                  SelectParm1(*pmMsg) = PRMCOOP_TYPESTNCOLOR;
                  SelectParm2(*pmMsg) = 640;
                  SelectParm3(*pmMsg) = 480;
                  break;
               case BITCOOPPANELTYPE_TFTC800600:
                  SelectParm1(*pmMsg) = PRMCOOP_TYPETFTCOLOR;
                  SelectParm2(*pmMsg) = 800;
                  SelectParm3(*pmMsg) = 600;
                  break;
               default:
                  SelectParm1(*pmMsg) = PRMCOOP_TYPEUNKNOWN;
                  SelectParm2(*pmMsg) = (CPARAMETER)(-1);
                  SelectParm3(*pmMsg) = (CPARAMETER)(-1);
            } /* endswitch */
         } /* endif */
         break;

      case MSGCOOP_QUERY_EXTERNALSCSI:
         /* Query external SCSI device presence */
         /*                                     */
         BuildMsg(msg, MSGCOOP_READ_STORAGEPRESENCE);
         SendMsg(msg, oCooperIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCOOPSTORAGEPRESENCE_EXTSCSI)
                                  ? (CPARAMETER)PRMCOOP_DEVICEPRESENT
                                  : (CPARAMETER)PRMCOOP_DEVICENOTPRESENT;
         } /* endif */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGCOOP_GET_L2CACHESTATUS:
         /* Get L2 cache status */
         /*                     */
         BuildMsg(msg, MSGCOOP_READ_SYSTEMCONTROL);
         SendMsg(msg, oCooperIo);

         if(!(rc = SelectResult(msg)))
         {
            SelectParm1(*pmMsg) = (SelectParm1(msg) &
                                   BITCOOPSYSCONTROL_L2CACHEDISABLE)
                                  ? (CPARAMETER)PRMCOOP_CACHEENABLE
                                  : (CPARAMETER)PRMCOOP_CACHEDISABLE;
         } /* endif */
         break;

      case MSGCOOP_SET_L2CACHESTATUS:
         /* Set L2 cache status */
         /*                     */
         switch(SelectParm1(*pmMsg))
         {
            case PRMCOOP_CACHEENABLE:
               BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                      BITCOOPSYSCONTROL_L2CACHEDISABLE |
                                      BITCOOPSYSCONTROL_L2CACHEINHIBIT ,
                                      BITCOOPSYSCONTROL_L2CACHEINHIBIT |
                                      BITCOOPSYSCONTROL_L2CACHEDISABLE |
                                      BITCOOPSYSCONTROL_MASKTEA        |
                                      BITCOOPSYSCONTROL_L2FLUSH        |
                                      BITCOOPSYSCONTROL_RESERVED3      |
                                      BITCOOPSYSCONTROL_RESERVED2      |
                                      BITCOOPSYSCONTROL_RESERVED1      |
                                      BITCOOPSYSCONTROL_RESERVED0      );
               SendMsg(msg, oCooperIo);
               Sync();

               rc = SelectResult(msg);
               break;
            case PRMCOOP_CACHEDISABLE:
               BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                      0,
                                      BITCOOPSYSCONTROL_L2CACHEINHIBIT |
                                      BITCOOPSYSCONTROL_L2CACHEDISABLE |
                                      BITCOOPSYSCONTROL_MASKTEA        |
                                      BITCOOPSYSCONTROL_L2FLUSH        |
                                      BITCOOPSYSCONTROL_RESERVED3      |
                                      BITCOOPSYSCONTROL_RESERVED2      |
                                      BITCOOPSYSCONTROL_RESERVED1      |
                                      BITCOOPSYSCONTROL_RESERVED0      );
               SendMsg(msg, oCooperIo);
               Sync();

               rc = SelectResult(msg);
               break;
            default:
               rc = ERRCOOP_INVALID_PARAMETER;
         } /* endswitch */
         break;

#ifdef PMFUNCTIONS_FULL
      case MSGCOOP_FLUSH_L2CACHE:
         /* Flush L2 cache */
         /*                */
         BuildMsg(mMem, MSGMEM_QUERY_CONTROLLERTYPE);
         SendMsg(mMem, oMemoryCtl);
         if(!SelectResult(mMem) && (SelectParm1(mMem) == ControllerEagle))
         {
            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   0,
                                   BITCOOPSYSCONTROL_L2CACHEINHIBIT |
                                   BITCOOPSYSCONTROL_MASKTEA        |
                                   BITCOOPSYSCONTROL_L2FLUSH        |
                                   BITCOOPSYSCONTROL_RESERVED3      |
                                   BITCOOPSYSCONTROL_RESERVED2      |
                                   BITCOOPSYSCONTROL_RESERVED1      |
                                   BITCOOPSYSCONTROL_RESERVED0      );
            SendMsg(msg, oCooperIo);
            Sync();

            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   BITCOOPSYSCONTROL_L2FLUSH,
                                   BITCOOPSYSCONTROL_MASKTEA   |
                                   BITCOOPSYSCONTROL_L2FLUSH   |
                                   BITCOOPSYSCONTROL_RESERVED3 |
                                   BITCOOPSYSCONTROL_RESERVED2 |
                                   BITCOOPSYSCONTROL_RESERVED1 |
                                   BITCOOPSYSCONTROL_RESERVED0 );
            SendMsg(msg, oCooperIo);
            Sync();

            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   0,
                                   BITCOOPSYSCONTROL_MASKTEA   |
                                   BITCOOPSYSCONTROL_L2FLUSH   |
                                   BITCOOPSYSCONTROL_RESERVED3 |
                                   BITCOOPSYSCONTROL_RESERVED2 |
                                   BITCOOPSYSCONTROL_RESERVED1 |
                                   BITCOOPSYSCONTROL_RESERVED0 );
            SendMsg(msg, oCooperIo);
            Sync();

            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   BITCOOPSYSCONTROL_L2CACHEINHIBIT,
                                   BITCOOPSYSCONTROL_L2CACHEINHIBIT |
                                   BITCOOPSYSCONTROL_MASKTEA        |
                                   BITCOOPSYSCONTROL_L2FLUSH        |
                                   BITCOOPSYSCONTROL_RESERVED3      |
                                   BITCOOPSYSCONTROL_RESERVED2      |
                                   BITCOOPSYSCONTROL_RESERVED1      |
                                   BITCOOPSYSCONTROL_RESERVED0      );
            SendMsg(msg, oCooperIo);
            Sync();

            rc = SelectResult(msg);
         }
         else
         {
            BuildMsgWithParm1(msg, MSGCOOP_WRITE_L2FLUSH,
                                   0);
            SendMsg(msg, oCooperIo);

            rc = SelectResult(msg);
         } /* endif */
         break;
#endif /* PMFUNCTIONS_FULL */

      case MSGCOOP_FLUSHDISABLE_L2CACHE:
         /* Flush and disable L2 cache */
         /*                            */
         BuildMsg(mMem, MSGMEM_QUERY_CONTROLLERTYPE);
         SendMsg(mMem, oMemoryCtl);
         if(!SelectResult(mMem) && (SelectParm1(mMem) == ControllerEagle))
         {
            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   0,
                                   BITCOOPSYSCONTROL_L2CACHEINHIBIT |
                                   BITCOOPSYSCONTROL_MASKTEA        |
                                   BITCOOPSYSCONTROL_L2FLUSH        |
                                   BITCOOPSYSCONTROL_RESERVED3      |
                                   BITCOOPSYSCONTROL_RESERVED2      |
                                   BITCOOPSYSCONTROL_RESERVED1      |
                                   BITCOOPSYSCONTROL_RESERVED0      );
            SendMsg(msg, oCooperIo);
            Sync();

            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   BITCOOPSYSCONTROL_L2FLUSH,
                                   BITCOOPSYSCONTROL_MASKTEA   |
                                   BITCOOPSYSCONTROL_L2FLUSH   |
                                   BITCOOPSYSCONTROL_RESERVED3 |
                                   BITCOOPSYSCONTROL_RESERVED2 |
                                   BITCOOPSYSCONTROL_RESERVED1 |
                                   BITCOOPSYSCONTROL_RESERVED0 );
            SendMsg(msg, oCooperIo);
            Sync();

            BuildMsgWithParm2(msg, MSGCOOP_MODIFY_SYSTEMCONTROL,
                                   0,
                                   BITCOOPSYSCONTROL_MASKTEA   |
                                   BITCOOPSYSCONTROL_L2FLUSH   |
                                   BITCOOPSYSCONTROL_RESERVED3 |
                                   BITCOOPSYSCONTROL_RESERVED2 |
                                   BITCOOPSYSCONTROL_RESERVED1 |
                                   BITCOOPSYSCONTROL_RESERVED0 );
            SendMsg(msg, oCooperIo);
            Sync();

            BuildMsgWithParm1(msg, MSGCOOP_SET_L2CACHESTATUS,
                                   PRMCOOP_CACHEDISABLE);
            SendMsg(msg, oCooperCtl);

            rc = SelectResult(msg);
         }
         else
         {
            BuildMsgWithParm1(msg, MSGCOOP_SET_L2CACHESTATUS,
                                   PRMCOOP_CACHEDISABLE);
            SendMsg(msg, oCooperCtl);

            BuildMsgWithParm1(msg, MSGCOOP_WRITE_L2FLUSH,
                                   0);
            SendMsg(msg, oCooperIo);

            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCOOP_RESET_L2TAGRAM:
         /* Reset L2 TAG RAM data */
         /*                       */
         BuildMsg(mMem, MSGMEM_QUERY_CONTROLLERTYPE);
         SendMsg(mMem, oMemoryCtl);
         if(!SelectResult(mMem) && (SelectParm1(mMem) == ControllerEagle))
         {
            BuildMsgWithParm1(msg, MSGCOOP_WRITE_L2FLUSH,
                                   0);
            SendMsg(msg, oCooperIo);
            Sync();

            rc = SelectResult(msg);
         }
         else
         {
            rc = ERRCOOP_NOERROR;
         } /* endif */
         break;

      case MSGCOOP_SAVE_CONTEXT:
         /* Save device context */
         /*                     */
         rc = ERRCOOP_NOERROR;

         BuildMsg(msg, MSGCOOP_READ_STORAGELIGHT);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chStorageLight = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_SYSTEMCONTROL);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chSystemControl = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PCIINTAMAP);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPciIntAMap = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PCIINTBMAP);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPciIntBMap = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_AUDIOSUPPORT);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chAudioSupport = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PLANARSETUPLOCK);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPlanarSetupLock = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_SCSIIDSETUP);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chScsiIdSetup = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PCICINTMAPPING);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPcicIntMapping = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_FDCSUPPORT);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chFdcSupport = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_GET_PANELINDEX);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelIndex = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PANELGRAYLEVEL);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelGrayLevel = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PANELSIGNALSOURCE);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelSignalSource = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PANELBULLETINBD1);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelBulletinBoard1 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PANELBULLETINBD2);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelBulletinBoard2 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PANELBULLETINBD3);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelBulletinBoard3 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsg(msg, MSGCOOP_READ_PANELBULLETINBD4);
         SendMsg(msg, oCooperIo);
         if(!SelectResult(msg))
         {
            pcoopctl->chPanelBulletinBoard4 = (BYTE)SelectParm1(msg);
         }
         else
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      case MSGCOOP_RESTORE_CONTEXT:
         /* Restore device context */
         /*                        */
         rc = ERRCOOP_NOERROR;

         BuildMsg(msg, MSGCOOP_GET_L2CACHESTATUS);
         SendMsg(msg, oCooperCtl);
         if(!SelectResult(msg) && (SelectParm1(msg) == PRMCOOP_CACHEDISABLE))
         {
            BuildMsg(msg, MSGCOOP_RESET_L2TAGRAM);
            SendMsg(msg, oCooperCtl);
            if(SelectResult(msg))
            {
               rc = SelectResult(msg);
            } /* endif */
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_STORAGELIGHT,
                                pcoopctl->chStorageLight);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_SYSTEMCONTROL,
                                pcoopctl->chSystemControl &
                                (BITCOOPSYSCONTROL_L2CACHEINHIBIT |
                                 BITCOOPSYSCONTROL_L2CACHEDISABLE ));
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PCIINTAMAP,
                                pcoopctl->chPciIntAMap);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PCIINTBMAP,
                                pcoopctl->chPciIntBMap);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_AUDIOSUPPORT,
                                pcoopctl->chAudioSupport);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PLANARSETUPLOCK,
                                pcoopctl->chPlanarSetupLock);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_SCSIIDSETUP,
                                pcoopctl->chScsiIdSetup);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PCICINTMAPPING,
                                pcoopctl->chPcicIntMapping);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_FDCSUPPORT,
                                pcoopctl->chFdcSupport);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PANELGRAYLEVEL,
                                pcoopctl->chPanelGrayLevel);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PANELSIGNALSOURCE,
                                pcoopctl->chPanelSignalSource);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PANELBULLETINBD1,
                                pcoopctl->chPanelBulletinBoard1);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PANELBULLETINBD2,
                                pcoopctl->chPanelBulletinBoard2);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PANELBULLETINBD3,
                                pcoopctl->chPanelBulletinBoard3);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_WRITE_PANELBULLETINBD4,
                                pcoopctl->chPanelBulletinBoard4);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */

         BuildMsgWithParm1(msg, MSGCOOP_SET_PANELINDEX,
                                pcoopctl->chPanelIndex);
         SendMsg(msg, oCooperIo);
         if(SelectResult(msg))
         {
            rc = SelectResult(msg);
         } /* endif */
         break;

      default:
         SendMsg(*pmMsg, oCooperIo);
         rc = SelectResult(*pmMsg);
   } /* endswitch */

   SelectResult(*pmMsg) = rc;
   return;
}


/* *****************************  End of File  ***************************** */
