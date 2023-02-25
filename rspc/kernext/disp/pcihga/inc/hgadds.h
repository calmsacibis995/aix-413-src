/* @(#)66  1.2  src/rspc/kernext/disp/pcihga/inc/hgadds.h, pcihga, rspc411, GOLD410 4/29/94 12:05:38 */
/* hgadds.h */
/*
based on @(#)94               1.1     src/bos/kernext/disp/wga/inc/wgadds.h, bos, bos410 10/28/93 18:54:06
 *
 * COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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

#ifndef _HGADDS
#define _HGADDS

struct  fbdds
{
   ulong    bus_mem_address;        /* addr where configured on bus      */
   short    bid;                    /* bus bid                           */
   ulong    IO_address;             /* Start addr of IO space            */
   ulong    display_mode;           /* desired display mode from ODM     */
   ulong    ksr_color_table[16];    /* ksr color pallet                  */
   ulong    devid;                  /* device id                         */
   ulong    display_id;             /* display id                        */
   short    screen_width_mm;        /* screen width in millimeters       */
   short    screen_height_mm;       /* screen heigth in millimeters      */
   short    slot_number;            /* slot number                       */
   char     component[16];          /* component name                    */
};

#endif /* _HGADDS */
