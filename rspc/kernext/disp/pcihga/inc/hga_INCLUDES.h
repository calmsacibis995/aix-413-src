/* @(#)59  1.1  src/rspc/kernext/disp/pcihga/inc/hga_INCLUDES.h, pcihga, rspc411, GOLD410 4/15/94 18:25:55 */
/* hga_INCLUDES.h */
/*
based on @(#)59       1.1  src/bos/kernext/disp/wga/inc/INCLUDES.h, bos, bos410 10/28/93 18:12:22
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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

#ifndef _H_INCLUDES_HGA
#define _H_INCLUDES_HGA

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/except.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <vt.h>
#include <sys/display.h>
#define Bool unsigned
#include <sys/rcm.h>
#include <sys/aixfont.h>
#include <sys/xmem.h>
#include <sys/font.h>
#include <sys/disptrc.h>
#include "hga.h"
#include "hgaddf.h"
#include "hgadds.h"
#include "hgadef.h"
#include "hgaldat.h"
#include "hga_reg.h"
#include "hga_regval.h"
#include "hga_macro.h"
#include "hga_ras.h"





#define SELECT_NO_DISP(char_attr,tmp)    \
 tmp = ((char_attr << 27) >> 31);

#define SELECT_BRIGHT(char_attr,tmp) \
 tmp = ((char_attr << 28) >> 31);

#define SELECT_BLINK(char_attr,tmp) \
 tmp = ((char_attr << 29) >> 31);

#define SELECT_UNDERSCORE(char_attr,tmp) \
 tmp = ((char_attr << 31) >> 31);

#define SET_ATTRIBUTES(attr,tmp_attr)                                      \
 tmp_attr = 0x0000;                                                                         \
 if (attr & 0x0002)  /* in reverse video mode */                   \
 { tmp_attr = ((attr & 0xf000)>>4) | ((attr & 0x0f00)<<4) | (attr & 0x00ff);    \
 }                                                                                                         \
 else tmp_attr = attr;





unsigned long maskval[] = { 0x00000000,
                            0xFF000000,
                            0xFFFF0000,
                            0xFFFFFF00 };

#define SPACE_A 0x00200000      /* ASCII code for a space in the high     */
                                /* bytes, low bytes reserved for the      */
                                /* character attribute                    */
#define BLANKS_NULL_ATTR  0x00200000    /* one blank with null attrib   */




struct t_ps_s {
   long   ps_w,ps_h;
};

uint          tempful;
uchar         tempbyte;

#endif /* _H_INCLUDES_HGA */
