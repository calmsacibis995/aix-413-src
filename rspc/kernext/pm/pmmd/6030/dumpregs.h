/* @(#)38   1.1  src/rspc/kernext/pm/pmmd/6030/dumpregs.h, pwrsysx, rspc41J, 9517A_all 4/24/95 11:00:32 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for register dump debug function 
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

#ifndef _H_DUMPREG
#define _H_DUMPREG

#include <sys/types.h>
#include "pdplanar.h"


/* Define Block Address Translation (IBAT or DBAT) register */

typedef struct _BAT {
    struct {
        UINT  Pp        :  2;           /* protection bits */
        UINT  Fill1     :  1;
        UINT  G         :  1;           /* guard flag */
        UINT  M         :  1;           /* memory coherent flag */
        UINT  I         :  1;           /* cache inhibited flag */
        UINT  W         :  1;           /* write-thru flag      */
        UINT  Fill2     : 10;
        UINT  Brpn      : 15;           /* block real page number */
    } Lower;
    struct {
        UINT  Vp        :  1;           /* problem state valid */
        UINT  Vs        :  1;           /* supervisor state valid */
        UINT  Bl        : 11;           /* block length */
        UINT  Fill1     :  4;
        UINT  Bepi      : 15;           /* block effective page index */
    } Upper;
} BAT;

void KiGetIbat0 (BAT *);             /* function to read an IBAT pair */
void KiGetIbat1 (BAT *);             /* function to read an IBAT pair */
void KiGetIbat2 (BAT *);             /* function to read an IBAT pair */
void KiGetIbat3 (BAT *);             /* function to read an IBAT pair */
void KiGetDbat0 (BAT *);             /* function to read a DBAT pair */
void KiGetDbat1 (BAT *);             /* function to read a DBAT pair */
void KiGetDbat2 (BAT *);             /* function to read a DBAT pair */
void KiGetDbat3 (BAT *);             /* function to read a DBAT pair */

ULONG KiGetXer   ();                    /* Xer */
ULONG KiGetXer   ();                    /* Xer */
void  KiSetXer   (ULONG);               /**/
ULONG KiGetDar   ();                    /* Dar */
void  KiSetDar   (ULONG);               /**/
ULONG KiGetDec   ();                    /* Dec */
void  KiSetDec   (ULONG);               /**/
ULONG KiGetSdr1  ();                    /* Sdr1 */
void  KiSetSdr1  (ULONG);               /**/
ULONG KiGetSrr0  ();                    /* Srr0 */
void  KiSetSrr0  (ULONG);               /**/
ULONG KiGetSrr1  ();                    /* Srr0 */
void  KiSetSrr1  (ULONG);               /**/
ULONG KiGetSprg  (ULONG num);           /**/
void  KiSetSprg  (ULONG num,ULONG);     /**/
ULONG KiGetEar   ();                    /* Ear */
void  KiSetEar   (ULONG);               /**/
ULONG KiGetTb    ();                    /* Tb */
void  KiSetTb    (ULONG);               /**/
ULONG KiGetTbu   ();                    /* Tbu */
void  KiSetTbu   (ULONG);               /**/
ULONG KiGetSr    (ULONG num);           /**/
void  KiSetSr    (ULONG num,ULONG);     /**/
ULONG KiGetMsr   ();                    /* Msr */
void  KiSetMsr   (ULONG);               /**/

ULONG KiGetRsp   ();                    /* r.sp */
ULONG KiGetRtoc  ();                    /* r.toc */

ULONG KiGetHID   (ULONG num);           /* HID */
void  KiSetHID   (ULONG num,ULONG);     /**/
ULONG KiGetRPA   ();                    /* RPA */
void  KiSetRPA   (ULONG);               /**/
ULONG KiGetICMP  ();                    /* ICMP */
void  KiSetICMP  (ULONG);               /**/
ULONG KiGetDCMP  ();                    /* DCMP */
void  KiSetDCMP  (ULONG);               /**/
ULONG KiGetIMISS ();                    /* IMISS */
void  KiSetIMISS (ULONG);               /**/
ULONG KiGetDMISS ();                    /* DMISS */
void  KiSetDMISS (ULONG);               /**/

#endif	/* _H_DUMPREG */
