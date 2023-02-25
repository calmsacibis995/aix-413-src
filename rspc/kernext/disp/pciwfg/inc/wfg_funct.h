/* @(#)88  1.4 src/rspc/kernext/disp/pciwfg/inc/wfg_funct.h, pciwfg, rspc41J, 9507C 1/27/95 03:12:08 */
/* wfg_funct.h */
/*
 *
 *  COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *  FUNCTIONS: none
 *
 *  ORIGINS: 27
 *
 *  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *  combined with the aggregated modules for this product)
 *                   SOURCE MATERIALS
 *
 *  (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *  All Rights Reserved
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_WFG_FUNCT
#define _H_WFG_FUNCT

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
extern_static void get_VDD_info();
extern_static int load_font();
extern_static void load_palette();

extern_static long wfg_reset();
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
extern_static long vttdpm();
extern_static int  wfg_pm_handler();
extern_static int  power_control();
extern_static int  device_power();
extern_static long setup_powermgt();
extern_static long pm_save_register();
extern_static long pm_restore_register();
extern long wfg_open();
extern long wfg_init();
extern long wfg_close();
extern long wfg_ioctl();
extern long wfg_make_gp();
extern long wfg_unmake_gp();
extern int  enable_wfg();
extern_extern int wfg_intr();

#endif /* _H_WFG_FUNCT */
