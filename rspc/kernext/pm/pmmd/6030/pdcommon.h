/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Miscellaneous Common Definitions
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
/* * Module Name: PDCOMMON.H                                               * */
/* * Description: Miscellaneous Common Definitions   (for PowerPC 601/603) * */
/* * Restriction: PDPLANAR.H file should be included before this file      * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDCOMMON
#define  _H_PDCOMMON


/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *   Wait Time Control Definition    * */
/* ************************************* */
/* -------------------- */
/*  Check routine type  */
/* -------------------- */
typedef CRESULT (*PCHKFUNC)(VOID);            /* Check routine function type */

/* ************************************* */
/* *      I/O Control Definition       * */
/* ************************************* */
/* ----------------------- */
/*  Bus base port address  */
/* ----------------------- */
#define  PORTIO_BASE                      (0x80000000L)
                                                     /* I/O bus base address */
#define  PORTISA_BASE                     (0x80000000L)
                                                     /* ISA bus base address */
#define  MEMORYISA_BASE                   (0xC0000000L)
                                                  /* ISA memory base address */
#define  PORTIOBASE_SIZE                  (0x00010000L)
                                                /* I/O bus base address size */
#define  PORTISABASE_SIZE                 (0x00010000L)
                                                /* ISA bus base address size */


/* ************************************************************************* */
/* *                 Function Prototype Definition Section                 * */
/* ************************************************************************* */
/* ************************************* */
/* *    Wait Time Control Routines     * */
/* ************************************* */
/* ----------- */
/*  Wait time  */
/* ----------- */
VOID  WaitTime(WORD  wTimeValue);             /* Wait time value (msec unit) */

/* ------------------------------ */
/*  Wait time with check routine  */
/* ------------------------------ */
CRESULT  WaitTimeCheck(WORD      wTimeValue,  /* Wait time value (msec unit) */
                       PCHKFUNC  pfnCheckRoutine);
                                             /* Pointer to the check routine */
                                             /*      returning CRESULT value */

/* ----------------------------- */
/*  Timer frequency calibration  */
/* ----------------------------- */
VOID  CalibrateTime(VOID);

/* ************************************* */
/* *    Interrupt Control Routines     * */
/* ************************************* */
#include "pdppcfnc.h"

/* ----------------- */
/*  Interrupt macro  */
/* ----------------- */
#define  DisableInterrupt()               ResetMsrEe()
#define  EnableInterrupt()                SetMsrEe()
#define  ReadInterruptStatus()            ReadMsrEe()

/* ************************************* */
/* *       I/O Control Routines        * */
/* ************************************* */
/* #include "pdppcfnc.h"                  */

/* ---------------- */
/*  Port I/O macro  */
/* ---------------- */
#define  ReadControllerByte(addr)         ((BYTE)inp((UINT)(addr)))
#define  WriteControllerByte(addr, val)   outp((UINT)(addr), (val))
#define  ReadControllerWord(addr)         ((WORD)inpw((UINT)(addr)))
#define  WriteControllerWord(addr, val)   outpw((UINT)(addr), (val))
#define  ReadControllerDWord(addr)        ((DWORD)inpdw((UINT)(addr)))
#define  WriteControllerDWord(addr, val)  outpdw((UINT)(addr), (val))

/* ----------------- */
/*  I/O delay macro  */
/* ----------------- */
#define  IoDelay()                        ;

/* ------------------------- */
/*  Debug check-point macro  */
/* ------------------------- */
#ifdef DEBUG_CHECKPOINT
#define  DebugCheckPoint(val)             outp(GetIsaBaseAddress() + 0x03BCL, \
                                               (BYTE)(val))
#else
#define  DebugCheckPoint(val)             ;
#endif /* DEBUG_CHECKPOINT */

/* ------------------------ */
/*  I/O base address macro  */
/* ------------------------ */
extern PVOID eisa_control_base;
extern PVOID io_control_base;
extern PVOID pci_config_base;
extern PVOID io_memory_base;
void *
pm_get_memorymapiospace(
   unsigned long  address,
   unsigned long  length
   );

#define  GetIsaBaseAddress()              (DWORD)io_control_base
#define  GetIoBaseAddress()               (DWORD)io_control_base
#define  GetPciBaseAddress()              (DWORD)pci_config_base
#define  GetMemoryBaseAddress()           (DWORD)io_memory_base
#define  GetIoSpace(addr, size)           (DWORD)pm_get_memorymapiospace(addr, \
                                                                         size)

/* ************************************* */
/* *       BCD Control Routines        * */
/* ************************************* */
/* -------------------------------------------- */
/*  Decimal to BCD conversion macro for 8-bits  */
/* -------------------------------------------- */
#define  ConvertToBcdByte(val)           \
            (BYTE)((((val) / 10) << 4) | \
                    ((val) % 10)       )

/* --------------------------------------------- */
/*  Decimal to BCD conversion macro for 16-bits  */
/* --------------------------------------------- */
#define  ConvertToBcd(val)                                      \
            (WORD)(((WORD)ConvertToBcdByte((val) / 100) << 8) | \
                    (WORD)ConvertToBcdByte((val) % 100)       )

/* --------------------------------------------- */
/*  BCD to Decimal conversion macro for 16-bits  */
/* --------------------------------------------- */
#define  ConvertToDec(val)                                   \
            (WORD)(((((WORD)(val) >> 12) & 0x000F) * 1000) + \
                   ((((WORD)(val) >>  8) & 0x000F) *  100) + \
                   ((((WORD)(val) >>  4) & 0x000F) *   10) + \
                   (( (WORD)(val)        & 0x000F)       ) )

/* ************************************* */
/* *      Endian Control Routines      * */
/* ************************************* */
/* ------------------------------------- */
/*  Endian conversion macro for 32-bits  */
/* ------------------------------------- */
#define  ConvEndian32(value)                        \
            (((DWORD)(value) & 0x000000FFL) << 24 | \
             ((DWORD)(value) & 0x0000FF00L) <<  8 | \
             ((DWORD)(value) & 0x00FF0000L) >>  8 | \
             ((DWORD)(value) & 0xFF000000L) >> 24 )

/* ------------------------------------- */
/*  Endian conversion macro for 16-bits  */
/* ------------------------------------- */
#define  ConvEndian16(value)                 \
            (((WORD)(value) & 0x00FF) << 8 | \
             ((WORD)(value) & 0xFF00) >> 8 )


#endif /* _H_PDCOMMON */
/* *****************************  End of File  ***************************** */
