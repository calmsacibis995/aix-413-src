static char sccsid[] = "@(#)82  1.1  src/rspc/kernext/pm/wakeup/debug.c, pwrsysx, rspc41J, 9510A 3/8/95 06:44:07";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: HrWaitLoop, HrCheckPoint
 * 
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   ABSTRACT:
 *
 *   This code is used for debug purpose only.
 */

#include "typeport.h"
#include "ioport.h"

VOID
HrWaitLoop(
   ULONG loop
   )
{
   ULONG i,j,k;
   for (i = 0  ; i < loop ; i++ )
   {
      for(j = 0 ; j < 0x1000000 ; j++) {k=k*k;}
   }
}

VOID 
HrCheckPoint(
   UCHAR data
   )
{
#define DEBUGPORT	0x3bc
   WritePortUchar(data,DEBUGPORT);
}

