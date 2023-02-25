/* @(#)08  1.4 src/rspc/kernext/disp/pciwfg/inc/wfg_INCLUDES.h, pciwfg, rspc41J, 9512A_all 3/7/95 00:18:50 */
/* wfg_INCLUDES.h */
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

#ifndef _H_INCLUDES_WFG
#define _H_INCLUDES_WFG

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

#ifdef PM_SUPPORT
#include <sys/pmdev.h>
#endif /* PM_SUPPORT */

#include "wfg.h"
#include "wfgddf.h"
#include "wfgdds.h"
#include "wfgdef.h"
#include "wfgldat.h"
#include "wfg_reg.h"
#include "wfg_regval.h"
#include "wfg_macro.h"
#include "wfg_ras.h"
#include "wfg_pm.h"


/*
#define SELECT_NO_DISP(char_attr,tmp)    \
 tmp = ((char_attr << 27) >> 31);

#define SELECT_BRIGHT(char_attr,tmp)     \
 tmp = ((char_attr << 28) >> 31);

#define SELECT_BLINK(char_attr,tmp)      \
 tmp = ((char_attr << 29) >> 31);

#define SELECT_UNDERSCORE(char_attr,tmp) \
 tmp = ((char_attr << 31) >> 31);
*/

#define SET_ATTRIBUTES(attr,tmp_attr)                                        \
 tmp_attr = 0x0000;                                                          \
 if (attr & 0x0002)  /* in reverse video mode */                             \
 { tmp_attr = ((attr & 0xf000)>>4) | ((attr & 0x0f00)<<4) | (attr & 0x00ff); \
 }                                                                           \
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

#endif /* _H_INCLUDES_WFG */
