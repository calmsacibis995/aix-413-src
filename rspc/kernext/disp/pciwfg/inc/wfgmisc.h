/* @(#)04  1.4 src/rspc/kernext/disp/pciwfg/inc/wfgmisc.h, pciwfg, rspc41J, 9520A_all 5/16/95 02:49:05 */
/* wfgmisc.h */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WFG_MISC
#define _WFG_MISC

#define WORDLEN    32
#define BYTELEN     8

#define TRUE        1
#define FALSE       0

#define LCD         1
#define CRT         2
#define LCD_BACKLIGHT	3
#define DAC		4

#define uchar      unsigned char
#define ushort     unsigned short
#define ulong      unsigned long

#define BitsPP8    8

#define SETCURSOR 1 /* used in calls to draw_char() when drawing a cursor char*/
#define NOCURSOR  0 /* used in calls to draw_char() when not a cursor char*/

#endif /* _WFG_MISC */
