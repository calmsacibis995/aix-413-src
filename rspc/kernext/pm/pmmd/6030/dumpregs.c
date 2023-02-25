static char sccsid[] = "@(#)37  1.1  src/rspc/kernext/pm/pmmd/6030/dumpregs.c, pwrsysx, rspc41J, 9517A_all 4/24/95 10:59:08";
/*
 *   COMPONENT_NAME: PWRSYSX
 *   FUNCTIONS: register dump for debug  
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include "dumpregs.h"
#include "pdplanar.h"


#ifdef PM_DEBUG
VOID
sDumpRegisters (
    VOID
    )
{
    BAT IBAT0;
    BAT IBAT1;
    BAT IBAT2;
    BAT IBAT3;
    BAT DBAT0;
    BAT DBAT1;
    BAT DBAT2;
    BAT DBAT3;
    ULONG i;

    printf("MSR   = %08x\n",KiGetMsr()); 
    printf("HID0  = %08x\n",KiGetHID(0));
    printf("SDR1  = %08x\n",KiGetSdr1());
    printf("RPA   = %08x\n",KiGetRPA()); 
    printf("ICMP  = %08x\n",KiGetICMP());
    printf("DCMP  = %08x\n",KiGetDCMP());
    printf("IMISS = %08x\n",KiGetIMISS()); 
    printf("DMISS = %08x\n",KiGetDMISS()); 
    printf("TBL   = %08x\n",KiGetTbl());  
    printf("TBU   = %08x\n",KiGetTbu()); 
    printf("DEC   = %08x\n",KiGetDec()); 


    KiGetIbat0(&IBAT0);
    KiGetDbat0(&DBAT0);
    KiGetIbat1(&IBAT1);
    KiGetDbat1(&DBAT1);
    KiGetIbat2(&IBAT2);
    KiGetDbat2(&DBAT2);
    KiGetIbat3(&IBAT3);
    KiGetDbat3(&DBAT3);



    printf("IBAT0 = %08x %08x  DBAT0 = %08x %08x\n", \
              IBAT0.Upper,IBAT0.Lower,DBAT0.Upper,DBAT0.Lower); 
    printf("IBAT1 = %08x %08x  DBAT1 = %08x %08x\n", \
              IBAT1.Upper,IBAT1.Lower,DBAT1.Upper,DBAT1.Lower); 
    printf("IBAT2 = %08x %08x  DBAT2 = %08x %08x\n", \
              IBAT2.Upper,IBAT2.Lower,DBAT2.Upper,DBAT2.Lower); 
    printf("IBAT3 = %08x %08x  DBAT3 = %08x %08x\n", \
              IBAT3.Upper,IBAT3.Lower,DBAT3.Upper,DBAT3.Lower); 


    printf("Sr0-Sr15"); 
    for (i=0;i<16 ;i++ ) {
       if (i%8==0) {
          printf("\n");  
       } /* endif */
       printf("%08x ",KiGetSr(i)); 
    } /* endfor */
    printf("\n"); 

} /* sDumpRegisters */

#endif
