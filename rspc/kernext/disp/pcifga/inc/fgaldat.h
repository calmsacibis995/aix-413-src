/* @(#)08  1.1  src/rspc/kernext/disp/pcifga/inc/fgaldat.h, pcifga, rspc411, GOLD410 4/15/94 18:38:56 */
/* fgaldat.h */
/*
based on @(#)02               1.1     src/bos/kernext/disp/wga/inc/wgaldat.h, bos, bos410 10/29/93 09:28:50
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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

#ifndef _H_FGA_LDAT
#define _H_FGA_LDAT

/******************************************************************
*   IDENTIFICATION: FGALDAT.H                                     *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*                     data structures for the Fairway adapter     *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all VDD procedures.                       *
*                                                                 *
******************************************************************/

#include "fgaenv.h"


struct fgapal
{
  short  num_entries;
  long  rgbval[16];
};

struct fga_data {

   struct fgapal ksr_color_table;

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
   ulong monitor_type;          /* monitor type connected to FGA           */

   long  fb_width;              /* number of pels in x dimension of frame buffer */
   long  fb_size;               /* number of pels in frame buffer          */
   long  screen_width_pix;      /* number of pels in x dimension of screen */
   long  screen_height_pix;     /* number of pels in y dimension of screen */
   ulong wtk_memcfg_reg;        /* Memory configuration                    */
   ulong wtk_sys_config_reg;    /* System configuration                    */
   ulong wtk_srtctl_reg;        /* Screen Repaint Timing                   */
   ulong mem_shift;             /* shift val for addressing                */
   ulong bus_mem_addr;          /* PCI bus effective memory address        */
   struct fga_ddf *ddf;         
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

   uchar bt_command_reg_0;
   uchar bt_command_reg_1;
   uchar bt_command_reg_2;
   uchar bt_command_reg_3;
   uchar bt_pixel_mask_reg;
};

#endif /* _H_FGA_LDAT  */
