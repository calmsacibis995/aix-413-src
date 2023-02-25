/* @(#)82	1.1  src/rspc/kernext/disp/pciiga/inc/iga_ras.h, pciiga, rspc411, 9436D411b 9/5/94 16:32:03 */
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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


#ifndef _IGA_RAS
#define _IGA_RAS


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


#define IGA_INVALID_MODE        1000
#define IGA_BAD_XMALLOC         1001
#define IGA_BAD_CMD             1002
#define IGA_DEVSWADD            1003
#define IGA_IO_EXCEPT           1004
#define IGA_FONT_LOAD           1005
#define IGA_I_INIT              1006
#define IGA_NODEV               1007

#endif /* _IGA_RAS */
