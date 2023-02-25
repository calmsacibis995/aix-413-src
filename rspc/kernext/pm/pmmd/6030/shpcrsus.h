/* @(#)02       1.2  src/rspc/kernext/pm/pmmd/6030/shpcrsus.h, pwrsysx, rspc41J, 9511A_all 3/15/95 04:33:38 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: shpcrsus.h
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

#ifndef _H_SHPCRSUS
#define _H_SHPCRSUS

#include <sys/types.h>


/*  Definitions */
#ifndef PAGE_SIZE
#define PAGE_SIZE  4*1024        /* Page Size is 4k bytes */
#endif


/*  Declare Externals */

extern
int
ProcessorReset (
    int	
    );

extern
PVOID
GetReturnToVirtualEntry (
    VOID
    );

extern
VOID
RegisterReturnToVirtualEntry (
    PVOID
    );

extern
PVOID
GetResumeEntry (
    VOID
    );

extern
VOID
GetContiguousCodeSize (
    PULONG pCodeEntry,
    PULONG pCodeSize,
    int,
    int 
    );

extern
PVOID
GetRelocatableCodeStart (
    VOID
    );

extern
PVOID
GetRelocatableCodeExit (
    VOID
    );


typedef struct _PMSusHibCB {
  /* 
   * Hibernation entry or registering ResumeEntry into NVRAM for suspend 
   */ 
   int (*SuspendHibernationCallback)(); 

  /* 
   * main power off for hibernation or Vcc-B off(partial off) for suspend
   */
   void (*PowerControlCallback)(); 

  /*
   * wakeup code entry from Hibernation wakeup code or ROS(suspend)
   */ 
   void *ResumeEntry;       

  /* 
   * shift value of configuous code for page alignment of resume code 
   */
   ulong ContiguousAreaShift;   

} PMSusHibCB;


#endif	/* _H_SHPCRSUS */
