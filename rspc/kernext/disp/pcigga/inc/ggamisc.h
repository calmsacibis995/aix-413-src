/* @(#)96  1.1  src/rspc/kernext/disp/pcigga/inc/ggamisc.h, pcigga, rspc411, 9435D411a 9/2/94 16:01:29 */
/* ggamisc.h */
/*
based on @(#)03       1.1  src/bos/kernext/disp/wga/inc/wgamisc.h, bos, bos410 10/29/93 09:29:57
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

#ifndef _H_GGA_MISC
#define _H_GGA_MISC

#define WORDLEN    32
#define BYTELEN     8

#define TRUE        1
#define FALSE       0

#define uchar      unsigned char
#define ushort     unsigned short
#define ulong      unsigned long

#define BitsPP8 8

#define SETCURSOR 1 /* used in calls to draw_char() when drawing a cursor char*/
#define NOCURSOR  0 /* used in calls to draw_char() when not a cursor char*/

#endif /* _H_GGA_MISC */
