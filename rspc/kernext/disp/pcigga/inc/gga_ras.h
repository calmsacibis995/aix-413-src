/* @(#)88  1.1  src/rspc/kernext/disp/pcigga/inc/gga_ras.h, pcigga, rspc411, 9435D411a 9/2/94 15:59:27 */
/* gga_ras.h */
/*
based on @(#)89               1.1     src/bos/kernext/disp/wga/inc/wga_ras.h, bos, bos410 10/28/93 18:27:25
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_GGA_RAS
#define _H_GGA_RAS


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


#define GGA_INVALID_MODE        1000
#define GGA_BAD_XMALLOC         1001
#define GGA_BAD_CMD             1002
#define GGA_DEVSWADD            1003
#define GGA_IO_EXCEPT           1004
#define GGA_FONT_LOAD           1005
#define GGA_I_INIT              1006
#define GGA_NODEV               1007

#endif /* _H_GGA_RAS */
