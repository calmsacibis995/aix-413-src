/* @(#)90  1.2 src/rspc/kernext/disp/pciwfg/inc/wfg_ras.h, pciwfg, rspc41J, 9507C 1/27/95 03:09:26 */
/* wfg_ras.h */
/*
 *
 * COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _WFG_RAS
#define _WFG_RAS


/*************************************************************************
        ERROR LOGGING
 *************************************************************************/

/*------------
 Unique RAS codes used to identify specific error locations for error logging
 ------------*/
#define RAS_UNIQUE_1            "1"
#define RAS_UNIQUE_2            "2"
#define RAS_UNIQUE_3            "3"
#define RAS_UNIQUE_4            "4"
#define RAS_UNIQUE_5            "5"
#define RAS_UNIQUE_6            "6"


#define WFG_INVALID_MODE             1000
#define WFG_BAD_XMALLOC              1001
#define WFG_BAD_CMD                  1002
#define WFG_DEVSWADD                 1003
#define WFG_IO_EXCEPT                1004
#define WFG_FONT_LOAD                1005
#define WFG_I_INIT                   1006
#define WFG_NODEV                    1007
#define WFG_PM_ERROR                 1008
#define WFG_RESIDUAL_DATA_NOT_FOUND  1009

#endif /* _WFG_RAS */
