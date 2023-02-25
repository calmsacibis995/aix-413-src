/* @(#)98  1.3 src/rspc/kernext/disp/pciwfg/inc/wfgdds.h, pciwfg, rspc41J, 9507C 1/27/95 03:11:47 */
/* wfgdds.h */
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

#ifndef _WFGDDS
#define _WFGDDS

struct  fbdds
{
   ulong    bus_mem_address;        /* addr where configured on bus      */
   short    bid;                    /* bus bid                           */
   ulong    IO_address;             /* Start addr of IO space            */
   ulong    ksr_color_table[16];    /* ksr color pallet                  */
   ulong    devid;                  /* device id                         */
   ulong    display_id;             /* display id                        */
   short    screen_width_mm;        /* screen width in millimeters       */
   short    screen_height_mm;       /* screen heigth in millimeters      */
   short    slot_number;            /* slot number                       */
   int      pm_dev_itime[3];        /* PM: device idle time (New)        */
   int      pm_dev_att;             /* PM: device attribute              */
   int      pm_lcd_device_id;       /* PM: PMDEV_LCD value               */
   int      pm_crt_device_id;       /* PM: PMDEV_CRT value               */
   int      pm_gcc_device_id;       /* PM: PMDEV_GCC value               */
   int      pm_dac_device_id;       /* PM: PMDEV_DAC value               */
   int      pm_vram_device_id;      /* PM: PMDEV_VRAM value              */
   char     component[16];          /* component name                    */
};

#endif /* _WFGDDS */
