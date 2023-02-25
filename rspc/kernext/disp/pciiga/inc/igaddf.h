/* @(#)85	1.4  src/rspc/kernext/disp/pciiga/inc/igaddf.h, pciiga, rspc41J, 9515B_all 3/22/95 17:02:16 */
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

#ifndef _IGA_DDF
#define _IGA_DDF

/******************************************************************
*   identification: IGADDF.H                                      *
*   descriptive name: Defines for IGA device driver               *
*   function: Device values for use by the DMA and DDI routines   *
*              of the IGA device driver                           *
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



struct iga_ddf
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
	struct watchdog	wd;         /* watchog used for AIX Power Managemet  */
                                    /* Note nothing could be define above    */
                                    /* the watchdog!                         */ 

        ulong  bpp;                 /* bits per pel (vram dependent)         */
        ulong  monitor_type;        /* monitor type connected to IGA         */
        ulong  screen_height_mm;    /* Height of screen in mm                */
        ulong  screen_width_mm;     /* Height of screen in mm                */
        ulong  base_address;        /* real address of PCI mem space         */
        ulong  IO_segment;          /* copy of segment for adapter I/O space */
        ulong  MEM_segment;         /* copy of segment for adapter MEM space */
        ulong  VendorID;            /* copy of vendor id from card           */
        ulong  dev_type;            /* Homestake/Pony differentiator         */
        long   fb_size;	    	    /* amount of memory discovered           */
        struct io_map pci_mem_map;  /* memory mapping structure              */
        struct io_map pci_io_map;   /* memory mapping structure              */


        /* The definitions below are needed to support AIX Power Management  */

	struct pm_handle * p_aixpm_data;  /* pointer to PM handle structure  */
                                          /* for AIX Power Management        */

	int    pm_sleep_word;        /* IO in KSR mode                       */

	int  IO_in_progress;         /* flag to indicate IO in KSR mode needs*/
                                     /* to be quiesced                       */
                                  
	int  dpms_enable;            /* enable DPMS or not                   */

	igadd_trace_header_t memtrace;  /* trace buffer                      */ 
};

#endif /* _IGA_DDF */
