/* @(#)97  1.1  src/rspc/kernext/disp/pcifga/inc/fga.h, pcifga, rspc411, GOLD410 4/15/94 18:38:30 */
/* fga.h */
/*
based on @(#)87       1.1  src/bos/kernext/disp/wga/inc/wga.h, bos, bos410 10/28/93 18:24:43
 *
 * COMPONENT_NAME:  (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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

#ifndef _H_FGA_WGA
#define _H_FGA_WGA

#define PACE_FGA 0x65
#define FGA_START_DIAG 0x66
#define FGA_QUERY_DIAG 0x67
#define FGA_STOP_DIAG 0x68

struct fga_map     /* structure for returning mapping info to apps */
{
        ulong base_address;             /* adapter offset in segment      */
        ulong screen_height_mm;         /* Height of screen in mm         */
        ulong screen_width_mm;          /* Height of screen in mm         */
        ulong screen_height_pix;        /* Height of screen in pixels     */
        ulong screen_width_pix;         /* Height of screen in pixels     */
        ulong color_mono_flag;          /* Flag for color = 1 or mono = 0 */
        ulong  monitor_type;            /* monitor type connected to FGA  */
};

/* FGA Diagnostic Interrupt structure */
struct fga_intr_count
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
#define SYNC_ON_GREEN    0x28
#define NO_SYNC_ON_GREEN 0x00
        ulong   sync_on_green;
        ulong   dot_clock_reg;
        ulong   double_dot_clock;
        uchar   adcntl_reg;     /* sync polarities */
};

#endif /* _H_FGA_WGA */
