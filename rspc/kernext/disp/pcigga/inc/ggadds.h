/* @(#)92  1.5  src/rspc/kernext/disp/pcigga/inc/ggadds.h, pcigga, rspc41J, 9513A_all 3/22/95 11:07:57 */
/* ggadds.h */
/*
based on @(#)94               1.1     src/bos/kernext/disp/wga/inc/wgadds.h, bos, bos410 10/28/93 18:54:06
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

#ifndef _H_GGADDS
#define _H_GGADDS

struct  gbdds
{
   ulong    bus_mem_address;        /* bus_mem_addr from ODM             */
   ulong    bid;		    /* bus bid                           */
   short    slot_number;            /* slot number                       */
   short    screen_width_mm;        /* screen width in millimeters       */
   short    screen_height_mm;       /* screen heigth in millimeters      */
   ulong    devid;                  /* device id                         */
   ulong    display_id;             /* display id                        */
   ulong    display_mode;           /* display mode                      */
   ulong    ksr_color_table[16];    /* ksr color pallet                  */
   char     component[16];          /* component name                    */

   ulong    dpms_timeouts[3];       /* LFT's DPMS time-out values        */

   /* 
    * -----> Note this variable has to be last (see config. method code) 
    *
    *        If you have to add something, it has to be above this 
    *        variable
    */
   ulong    vram_config;      	    /* 0 - 2-meg,  1 - 4-meg             */

};

#endif /* _H_GGADDS */
