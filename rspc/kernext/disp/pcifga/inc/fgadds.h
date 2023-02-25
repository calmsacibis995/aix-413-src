/* @(#)05  1.2  src/rspc/kernext/disp/pcifga/inc/fgadds.h, pcifga, rspc411, GOLD410 4/29/94 11:52:25 */
/* fgadds.h */
/*
based on @(#)94               1.1     src/bos/kernext/disp/wga/inc/wgadds.h, bos, bos410 10/28/93 18:54:06
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

#ifndef _H_FGADDS
#define _H_FGADDS

struct  fbdds
{
   ulong    bus_mem_address;        /* bus_mem_addr from ODM             */
   ulong    bid;		    /* bus bid                           */
   short    slot_number;            /* slot number                       */
   short    screen_width_mm;        /* screen width in millimeters       */
   short    screen_height_mm;       /* screen heigth in millimeters      */
   ulong    devid;                  /* device id                         */
   ulong    display_id;             /* display id                        */
   ulong    ksr_color_table[16];    /* ksr color pallet                  */
   char     component[16];          /* component name                    */
};

#endif /* _H_FGADDS */
