static char sccsid[] = "@(#)29	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_compat.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:12";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: get_bus_speed_info
 *		mem_dump
 *		mem_fill
 *		spin
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef DEBUG
/******************************************************************
   This file contains a set of routines to use for compatibility 
   the BOCA runtime libraries. Since we have calls with the same
   function but with different names, this set of compatibility 
   routines will map the boca calls to AIXMON calls.
******************************************************************/


#include "misc_sub.h"

#define	TRUE	1
#define FALSE	0

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/* int mem_dump() - Dumps memory contents to stdout		 */
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

mem_dump(char *addr, int length)
{
dbugout(addr,length);
}

#endif /* DEBUG */

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/* get_bus_speed_info() - Return correct answer for now		 */
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

get_bus_speed_info()
{
return(0x32191900);
}

#ifdef DEBUG

/*-- spin ---------- "diag_parallel.c" ---------------------------*/
void spin (unsigned int Count)
  {
  rtc_value   rvRTC;

  rvRTC = get_timeout_value((unsigned int)Count, MILLISECONDS);
  while(timeout(rvRTC) == FALSE);

  }  /* END spin */


/*--------------------------- "debcmds.c" ---------------------------*/
void mem_fill( char *RAM_Address, int length, unsigned char fill_byte) {
   int i;
   if (length <= 0) return;
   for (i = 0; i < length; i++)
      *(unsigned char *) (RAM_Address + i) = fill_byte;
}



#endif /*DEBUG*/
