/* @(#)89	1.1  src/rspc/kernext/disp/pciiga/inc/igaldat.h, pciiga, rspc411, 9436D411b 9/5/94 16:32:17 */
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

#ifndef _IGA_LDAT
#define _IGA_LDAT

/******************************************************************
*   IDENTIFICATION: IGALDAT.H                                     *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*                     data structures for the S3 adapters         *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all VDD procedures.                       *
*                                                                 *
******************************************************************/

#include "igaenv.h"
#include "IO_defs.h"

struct igapal
{
  short  num_entries;
  long  rgbval[16];
};

struct iga_data {

    struct igapal ksr_color_table;

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
    uchar monitor;               /* monitor type connected to IGA           */
    VESA_MODE *v_mode;           /* vesa display mode selected              */

    long  screen_width_pix;      /* number of pels in x dimension of screen */
    long  screen_height_pix;     /* number of pels in y dimension of screen */
    struct iga_ddf * ddf;        /* pointer to ddf structure                */
    ulong bus_base_addr;         /* PCI bus memory effective address        */
    ulong IO_base_address;       /* PCI bus IO space effective address      */
    ulong vert_stride;           /* number of pixels between adjacent vert. */
};

#endif /*  _IGA_LDAT  */
