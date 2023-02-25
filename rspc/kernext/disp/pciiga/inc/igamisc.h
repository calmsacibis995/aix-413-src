/* @(#)90	1.1  src/rspc/kernext/disp/pciiga/inc/igamisc.h, pciiga, rspc411, 9436D411b 9/5/94 16:32:19 */
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

#ifndef _IGA_MISC
#define _IGA_MISC

#define WORDLEN    32
#define BYTELEN     8

#define TRUE        1
#define FALSE       0

#define uchar      unsigned char
#define ushort     unsigned short
#define ulong      unsigned long

#define BitsPP8    8

#define SETCURSOR 1 /* used in calls to draw_char() when drawing a cursor char*/
#define NOCURSOR  0 /* used in calls to draw_char() when not a cursor char*/

#endif /* _IGA_MISC */
