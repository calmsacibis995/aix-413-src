/* @(#)86	1.3  src/rspc/kernext/disp/pciiga/inc/igadds.h, pciiga, rspc41J, 9515B_all 3/22/95 17:03:30 */
/*
 *
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

#ifndef _IGADDS
#define _IGADDS

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
   ulong    dpms_timeouts[3];	    /* LFT's DPMS time-out values        */
};

#endif /* _IGADDS */
