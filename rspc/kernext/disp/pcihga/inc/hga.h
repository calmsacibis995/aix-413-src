/* @(#)58  1.1  src/rspc/kernext/disp/pcihga/inc/hga.h, pcihga, rspc411, GOLD410 4/15/94 18:25:52 */
/* hga.h */
/*
based on @(#)87       1.1  src/bos/kernext/disp/wga/inc/wga.h, bos, bos410 10/28/93 18:24:43
 *
 * COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _HGA_WGA
#define _HGA_WGA

#define PACE_HGA 0x65
#define HGA_START_DIAG 0x66
#define HGA_QUERY_DIAG 0x67
#define HGA_STOP_DIAG 0x68

struct hga_map     /* structure for returning mapping info to apps */
{
    ulong IO_segment;           /* effective address for adapter I/O space */
    ulong screen_height_mm;     /* Height of screen in mm                  */
    ulong screen_width_mm;      /* Height of screen in mm                  */
    ulong screen_height_pix;    /* Height of screen in pixels              */
    ulong screen_width_pix;     /* Height of screen in pixels              */
    ulong color_mono_flag;      /* Flag for color = 1 or mono = 0          */
    ulong pixel_depth;          /* Number of bits per pixel                */
    ulong MEM_segment;          /* effective address for adapter MEM space */
    ulong  monitor_type;        /* monitor type connected to HGA           */
};

/* HGA Diagnostic Interrupt structure */
struct hga_intr_count
{
  ulong erraddr;
  ulong parity_err;
  ulong inval_cmd;
  ulong resv_access;
  ulong bad_string_op;
  ulong inval_value;
  ulong intr_set;
  ulong blank_set;
};

#endif /* _HGA_WGA */
