/* @(#)84  1.2  src/rspc/kernext/disp/pcigga/inc/gga.h, pcigga, rspc411, 9436E411a 9/9/94 10:23:13 */
/* gga.h */
/*
based on @(#)87       1.1  src/bos/kernext/disp/wga/inc/wga.h, bos, bos410 10/28/93 18:24:43
 *
 * COMPONENT_NAME:  (PCIGGA) Weitek PCI Graphics Adapter Device Driver
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

#ifndef _H_GGA_WGA
#define _H_GGA_WGA

#define PACE_GGA 0x65
#define GGA_START_DIAG 0x66
#define GGA_QUERY_DIAG 0x67
#define GGA_STOP_DIAG 0x68

struct gga_map     /* structure for returning mapping info to apps */
{
        ulong base_address;             /* adapter offset in segment      */
        ulong screen_height_mm;         /* Height of screen in mm         */
        ulong screen_width_mm;          /* Height of screen in mm         */
        ulong screen_height_pix;        /* Height of screen in pixels     */
        ulong screen_width_pix;         /* Height of screen in pixels     */
        ulong color_mono_flag;          /* Flag for color = 1 or mono = 0 */
        ulong  monitor_type;            /* monitor type connected to GGA  */
};

/* GGA Diagnostic Interrupt structure */
struct gga_intr_count
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

struct crt_control
{
        ulong   monitor_id;     /* Combination of Cable id and syncs */
	ulong	srtctl2_reg;
	ulong	syscfg_reg;
	ulong	f0_reg;
	ulong	pixelFmt_reg;
	ulong	miscClk_reg;
        ulong   hrzt_reg;
        ulong   hrzsr_reg;
        ulong   hrzbr_reg;
        ulong   hrzbf_reg;
        ulong   vrtt_reg;
        ulong   vrtsr_reg;
        ulong   vrtbr_reg;
        ulong   vrtbf_reg;
        ulong   x_resolution;   /* pixels in x direction */
        ulong   y_resolution;   /* pixels in y direction */
#define COLOR 1
#define MONO  0
        ulong   color_mono;
};

#endif /* _H_GGA_WGA */
