/* @(#)95  1.1  src/rspc/kernext/disp/pcigga/inc/ggaldat.h, pcigga, rspc411, 9435D411a 9/2/94 16:01:21 */
/* ggaldat.h */
/*
based on @(#)02               1.1     src/bos/kernext/disp/wga/inc/wgaldat.h, bos, bos410 10/29/93 09:28:50
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

#ifndef _H_GGA_LDAT
#define _H_GGA_LDAT

/******************************************************************
*   IDENTIFICATION: GGALDAT.H                                     *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*                     data structures for the Glacier adapter     *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all VDD procedures.                       *
*                                                                 *
******************************************************************/

#include "ggaenv.h"


struct ggapal
{
  short  num_entries;
  long  rgbval[16];
};

struct gga_data {

   struct ggapal ksr_color_table;

   long  Scroll_offset;         /* number of cells into the presentation   */

   ushort reverse_video;        /* TRUE for reverse;False for Normal       */
                                /* set in SET_ATTRIBUTES                   */
   ushort fgcol;                /* foreground color                        */
   ushort bgcol;                /* background color                        */

   aixFontInfoPtr fonthptr;     /* Pointer to font head                    */
   aixCharInfoPtr fontcindex;   /* Pointer to char index                   */
   aixGlyphPtr glyphptr;        /* Pointer to glyph pointer                */

   char * Font_map_start;       /* Pointer to font in memory               */
   ushort volatile *io_idata;   /* pointer to index and datab              */
   long center_x, center_y;     /* number of pels to center picture        */
   long cursor_x, cursor_y;     /* values to center cursor                 */
   uchar colset,hwset,opdimset,resv; /* flags indicating cop status        */
   ushort old_bgcol;            /* background color                        */
   char cur_font;
   char disp_type;              /* Flag for mono or color                  */
   long poll_type;              /* Flag for old or new polling method      */
   char    *comp_name;          /* Pointer to dds for error logging        */
   long    charwd2;
   struct vttenv Vttenv;
   ulong bpp;                   /* bits per pel (vram dependent)           */
   ulong monitor_type;          /* monitor type connected to GGA           */

   long  fb_width;              /* number of pels in x dimension of frame buffer */
   long  fb_size;               /* number of pels in frame buffer          */
   long  screen_width_pix;      /* number of pels in x dimension of screen */
   long  screen_height_pix;     /* number of pels in y dimension of screen */
   ulong wtk_memcfg_reg;        /* Memory configuration                    */
   ulong wtk_sys_config_reg;    /* System configuration                    */
   ulong wtk_srtctl_reg;        /* Screen Repaint Timing                   */
   ulong wtk_srtctl2_reg;       /* Screen Repaint Timing                   */
   ulong mem_shift;             /* shift val for addressing                */
   ulong bus_mem_addr;          /* PCI bus effective memory address        */
   struct gga_ddf *ddf;         
   struct crt_control *P_CRT_CONTROL;
   ulong wtk_vram_end;
   ulong wtk_prevrtc_reg;
   ulong wtk_prehrzc_reg;
   ulong wtk_intr_enbl_reg;
   ulong wtk_rfperiod_reg;
   ulong wtk_rlmax_reg;
   ulong wtk_plane_mask;
   ulong wtk_draw_mode;
   ulong wtk_coor_offs;
   ulong wtk_w_min_xy;
   ulong wtk_w_max_xy;

   uchar rgb525_misc_clk;
   uchar rgb525_pixel_fmt;
   uchar rgb525_f0;
   uchar rgb525_cur_ctl;
   uchar rgb525_cur_x_low;
   uchar rgb525_cur_x_high;
   uchar rgb525_cur_y_low;
   uchar rgb525_cur_y_high;
   uchar rgb525_misc_ctl2;
};

#endif /* _H_GGA_LDAT  */
