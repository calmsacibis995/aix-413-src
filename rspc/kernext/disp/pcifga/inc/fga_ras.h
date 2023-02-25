/* @(#)01  1.1  src/rspc/kernext/disp/pcifga/inc/fga_ras.h, pcifga, rspc411, GOLD410 4/15/94 18:38:40 */
/* fga_ras.h */
/*
based on @(#)89               1.1     src/bos/kernext/disp/wga/inc/wga_ras.h, bos, bos410 10/28/93 18:27:25
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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


#ifndef _H_FGA_RAS
#define _H_FGA_RAS


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


#define FGA_INVALID_MODE        1000
#define FGA_BAD_XMALLOC         1001
#define FGA_BAD_CMD             1002
#define FGA_DEVSWADD            1003
#define FGA_IO_EXCEPT           1004
#define FGA_FONT_LOAD           1005
#define FGA_I_INIT              1006
#define FGA_NODEV               1007

#endif /* _H_FGA_RAS */
