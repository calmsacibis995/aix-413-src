/* @(#)78	1.1  src/rspc/kernext/disp/pciiga/inc/iga.h, pciiga, rspc411, 9436D411b 9/5/94 16:31:55 */
/*
 *
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

#ifndef _IGA_WGA
#define _IGA_WGA

#define PACE_IGA 0x65
#define IGA_START_DIAG 0x66
#define IGA_QUERY_DIAG 0x67
#define IGA_STOP_DIAG 0x68

struct iga_map     /* structure for returning mapping info to apps */
{
    ulong IO_segment;           /* effective address for adapter I/O space */
    ulong screen_height_mm;     /* Height of screen in mm                  */
    ulong screen_width_mm;      /* Height of screen in mm                  */
    ulong screen_height_pix;    /* Height of screen in pixels              */
    ulong screen_width_pix;     /* Height of screen in pixels              */
    ulong color_mono_flag;      /* Flag for color = 1 or mono = 0          */
    ulong pixel_depth;          /* Number of bits per pixel                */
    ulong MEM_segment;          /* effective address for adapter MEM space */
    ulong  monitor_type;        /* monitor type connected to IGA           */
};

/* IGA Diagnostic Interrupt structure */
struct iga_intr_count
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

#endif /* _IGA_WGA */
