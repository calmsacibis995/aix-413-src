/* @(#)86  1.6  src/rspc/kernext/disp/pcigga/inc/gga_funct.h, pcigga, rspc41B, 9504A 1/12/95 17:14:17 */
/* gga_funct.h */
/*
based on @(#)62       1.1  src/bos/kernext/disp/wga/inc/funct.h, bos, bos410 10/28/93 18:15:53
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_GGA_FUNCT
#define _H_GGA_FUNCT

#define extern_static extern       /* replace all occurances with 'static'*/
#define extern_extern extern       /* replace all occurances with 'extern'*/
/*----------------------------------------------------------------------*/
/* Function prototypes                                                  */
/*----------------------------------------------------------------------*/
/*  The local functions start on or about line 200. */
/*  The global functions start on or about line 1143. */

extern_static void change_cursor_shape();
extern_static void copy_ps();
extern_static void draw_char();
extern_static void draw_text();
extern_static void load_font();
extern_static void load_palette();
extern_static void select_new_bg();
extern_static void select_new_fg();
extern_static void select_new_font();
extern_static void set_bold_attr();
extern_static void set_underline_attr();

extern_static long gga_reset();
extern_static long vttact();
extern_static long vttcfl();
extern_static long vttclr();
extern_static long vttcpl();
extern_static long vttdact();
extern_static long vttddf();
extern_static long vttdefc();
extern_static long vttdma();
extern_static long vttinit();
extern_static long vttmovc();
extern_static long vttrds();
extern_static long vttterm();
extern_static long vtttext();
extern_static long vttscr();
extern_static long vttsetm();
extern_static long vttstct();
extern_static long sync_mask();
extern_static long async_mask();
extern long gga_open();
extern long gga_init();
extern long gga_close();
extern long gga_ioctl();
extern long gga_make_gp();
extern_extern int gga_intr();
extern_extern void cda_timeout();

extern long vttdpm();
extern long gga_pm_handler();

#define VIDEO_MINOR 0x8000
extern long gga_mpx();

#endif /* _H_GGA_FUNCT */
