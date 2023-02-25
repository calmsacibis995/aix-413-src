/* @(#)62  1.1  src/rspc/kernext/disp/pcihga/inc/hga_ras.h, pcihga, rspc411, GOLD410 4/15/94 18:26:02 */
/* hga_ras.h */
/*
based on @(#)89               1.1     src/bos/kernext/disp/wga/inc/wga_ras.h, bos, bos410 10/28/93 18:27:25
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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


#ifndef _HGA_RAS
#define _HGA_RAS


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


#define HGA_INVALID_MODE        1000
#define HGA_BAD_XMALLOC         1001
#define HGA_BAD_CMD             1002
#define HGA_DEVSWADD            1003
#define HGA_IO_EXCEPT           1004
#define HGA_FONT_LOAD           1005
#define HGA_I_INIT              1006
#define HGA_NODEV               1007

#endif /* _HGA_RAS */
