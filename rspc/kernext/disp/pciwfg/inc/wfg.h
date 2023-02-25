/* @(#)07  1.1  src/rspc/kernext/disp/pciwfg/inc/wfg.h, pciwfg, rspc411, 9434B411a 8/24/94 07:41:40 */
/* wfg.h */
/*
 *
 * COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
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

#ifndef _WFG_WGA
#define _WFG_WGA
/*
#define PACE_WFG 0x65
#define WFG_START_DIAG 0x66
#define WFG_QUERY_DIAG 0x67
#define WFG_STOP_DIAG 0x68
*/

struct wfg_map     /* structure for returning mapping info to apps */
{
    ulong IO_segment;           /* effective address for adapter I/O space */
    ulong screen_height_mm;     /* Height of Monitor screen in mm          */
    ulong screen_width_mm;      /* Height of Monitor screen in mm          */
    ulong screen_height_pix;    /* Height of Monitor screen in pixels      */
    ulong screen_width_pix;     /* Height of Monitor screen in pixels      */
    ulong panel_scrn_height_mm; /* Height of LCD panel screen in mm        */
    ulong panel_scrn_width_mm;  /* Height of LCD panel screen in mm        */
    ulong panel_scrn_height_pix;/* Height of LCD panel screen in pixels    */
    ulong panel_scrn_width_pix; /* Height of LCD panel screen in pixels    */
    ulong color_mono_flag;      /* Flag for color = 1 or mono = 0          */
    ulong pixel_depth;          /* Number of bits per pixel                */
    ulong MEM_segment;          /* effective address for adapter MEM space */
    ulong monitor_type;         /* monitor type connected to WFG           */
    ulong lcd_type;             /* lcd panel type connected to Woodfield   */
    ulong vram_size;            /* Video RAM size                          */
    ulong model_type;           /* Entry/Advanced model differentiator     */
};

/* WFG Diagnostic Interrupt structure 
struct wfg_intr_count
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
*/

#endif /* _WFG_WGA */
