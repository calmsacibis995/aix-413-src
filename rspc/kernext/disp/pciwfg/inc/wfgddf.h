/* @(#)97  1.2 src/rspc/kernext/disp/pciwfg/inc/wfgddf.h, pciwfg, rspc41J, 9507C 1/27/95 03:11:20 */
/* wfgddf.h */
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

#ifndef _WFG_DDF
#define _WFG_DDF

/******************************************************************
*   identification: WFGDDF.H                                      *
*   descriptive name: Defines for WFG device driver               *
*   function: Device values for use by the DMA and DDI routines   *
*              of the WFG device driver                           *
*                                                                 *
******************************************************************/

#define DMA_IN_PROGRESS 0x80000000
#define DMA_WAIT_REQ    0x01000000
#define DMA_COMPLETE    0x00FFFFFF
#define DIAG_MODE       0x00010000
#define DIAG_OFF        0xFF00FFFF
#define EVENT_MODE      0x00008000
#define EVENT_OFF       0xFFFF00FF
#define ASYNC_REQ       0x00000100
#define ASYNC_OFF       0xFFFFFEFF
#define SYNC_REQ        0x00000800
#define SYNC_WAIT_REQ   0x00000200
#define SYNC_NOWAIT_REQ 0x00000400
#define SYNC_OFF        0xFFFFF1FF
#define NORPT_EVENT     0x00000080
#define NORPT_OFF       0xFFFFFF7F

struct wfg_ddf
{
#if 0
        long   cmd;
        caddr_t sleep_addr;
        int (*callback)();        /* Callback routine for event support */
        uchar  s_event_mask;
        uchar  a_event_mask;
        uchar  e_event_mask;
        uchar  poll_type;
        caddr_t callbackarg;
        ulong  diaginfo[8];
        label_t jmpbuf;
        eventReport report;
        char  jumpmode,timerset;
        char  scrolltime,jthreshold;
        long  jumpcount,lastcount;
        struct trb *cdatime;
        int   (*sm_enq)();
#endif
        ulong  bpp;                 /* bits per pel (vram dependent)         */
        ulong  panel_type;          /* built-in LCD panel type               */
        ulong  monitor_type;        /* monitor type connected to WFG         */
        ulong  screen_height_mm;    /* Height of screen in mm                */
        ulong  screen_width_mm;     /* Height of screen in mm                */
        ulong  base_address;        /* real address of PCI mem space         */
        ulong  IO_segment;          /* copy of segment for adapter I/O space */
        ulong  MEM_segment;         /* copy of segment for adapter MEM space */
        ulong  VendorID;            /* copy of vendor id from card           */
        ulong  model_type;          /* Entry/Advanced differentiator         */
	ulong  IO_in_progress;      /* flag to indicate IO in KSR mode needs
							       to be quiesce */
        uchar  dpms_enabled;        /* Enable DPMS or not                    */
        long   fb_size;             /* amount of memory discovered           */
        struct io_map pci_mem_map;  /* memory mapping structure              */
        struct io_map pci_io_map;   /* memory mapping structure              */
};

#endif /* _WFG_DDF */
