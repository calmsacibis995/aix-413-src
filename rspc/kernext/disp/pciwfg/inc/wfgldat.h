/* @(#)02  1.4 src/rspc/kernext/disp/pciwfg/inc/wfgldat.h, pciwfg, rspc41J, 9512A_all 3/7/95 00:19:13 */
/* wfgldat.h */
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

#ifndef _WFG_LDAT
#define _WFG_LDAT

/******************************************************************
*   IDENTIFICATION: WFGLDAT.H                                     *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*        data structures for the Western Digital adapters         *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all VDD procedures.                       *
*                                                                 *
******************************************************************/

#include "wfgenv.h"
#include "IO_defs.h"

struct wfgpal
{
  short  num_entries;
  long  rgbval[16];
};

struct wfg_pmdata {
  ulong vvt;
};

struct wfg_data {

    struct wfgpal ksr_color_table;

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
    long offset_x, offset_y;     /* number of pels to center picture        */
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

#define ENTRY_MODEL	0x01     /* Woodfield Entry Model                   */
#define ADVANCED_MODEL	0x02     /* Woodfield Entry Model                   */
#define UNKNOWN_MODEL	0xFF     /* Unknown model                           */
    uchar model_type;            /* Entry/Advanced model differentiator     */

#define IBM_F8515	0xfe     /*  Color TFT 640x480 F8515   10.4 inch    */
#define IBM_F8532	0xfc     /*  Color TFT 800x600 F8532   10.4 inch    */
#define UNKNOWN_LCD  	0x0      /*  Unknown panel                          */
#define NO_LCD       	0xff     /*  Unknown panel                          */
    uint panel;                  /* build-in LCD panel type                 */

#define ISO_MON		0x00     /* ISO Fixed-sync monitor                  */
#define NON_ISO_MON	0x01     /* Non-ISO Multi-sync monitor              */
#define VESA_MON	0x02     /* VESA Type monitor                       */
#define NO_CRT		0x03     /* No connected monitor                    */
    uchar monitor;               /* monitor type connected to WFG           */

    V_MODE *v_mode;              /* vesa display mode selected              */

#define WFG_NGP		0
#define WFG_GP 		1
#define WFG_GP_TERM 	2
    uchar gp_flag;               /* A flag reflecting the state of display  */

    struct pm_handle *wfg_pmh;   /* Power Management Structure              */
#define TURN_ON         1
#define TURN_OFF        0
    uchar lcd_power;             /* LCD panel power status flag             */
    uchar crt_power;             /* CRT display power status flag           */

#define NOT_REGISTER    0
#define REGISTERED      1
    int   pm_register;           /* PM structure registration flag          */
    long  screen_width_pix;      /* # of pels in x dimension of screen (mon)*/
    long  screen_height_pix;     /* # of pels in y dimension of screen (mon)*/
    long  panel_scrn_width_pix;  /* # of pels in x dimension of screen (LCD)*/
    long  panel_scrn_height_pix; /* # of pels in y dimension of screen (LCD)*/
    long  panel_scrn_width_mm;   /* mm in x dimension of screen (LCD)       */
    long  panel_scrn_height_mm;  /* mm in y dimension of screen (LCD)       */
    ulong vram_size;             /* WD90C24A(2) Video RAM Size              */
    struct wfg_ddf * ddf;        /* pointer to ddf structure                */
    ulong bus_base_addr;         /* PCI bus memory effective address        */
    ulong IO_base_address;       /* PCI bus IO space effective address      */
    ulong vert_stride;           /* number of pixels between adjacent vert. */
};

#endif /*  _WFG_LDAT  */
