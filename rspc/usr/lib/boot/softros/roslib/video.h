/* @(#)46	1.1  src/rspc/usr/lib/boot/softros/roslib/video.h, rspc_softros, rspc411, GOLD410 4/18/94 15:59:53  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------------------------------------------------------------------------*/
/*      Video.h                                                                 */
/*                                                                              */
/*      Video Interface.                                                        */
/*                                                                              */
/*                                                                              */
/*------------------------------------------------------------------------------*/

#ifndef _H_VIDEO
#define _H_VIDEO

#define MODE_VESA_201 0x201             /*  640 x  480 x  8 video mode  0       */
#define MODE_VESA_205 0x205             /* 1024 x  768 x  8 video mode  1       */
#define MODE_VESA_107 0x107             /* 1280 x 1024 x  8 video mode  2       */
#define MODE_VESA_274 0x274             /* 1600 x 1200 x  8 video mode  3       */
#define MODE_VESA_211 0x211             /*  640 x  480 x 16 video mode  4       */
#define MODE_VESA_117 0x117             /* 1024 x  768 x 16 video mode  5       */
#define MODE_VESA_21A 0x21a             /* 1280 x 1024 x 16 video mode  6       */
#define MODE_VESA_275 0x275             /* 1600 x 1200 x 16 video mode  7       */
#define MODE_VESA_220 0x220             /*  640 x  480 x 32 video mode  8       */
#define MODE_VESA_222 0x222             /* 1024 x  768 x 32 video mode  9       */
#define MODE_VESA_21B 0x21b             /* 1280 x 1024 x 24 video mode 10       */


//extern char FONT8X16[];
//extern char isofont[];
extern char fnt8x16s[];
extern char fnt8x16d[];

/*------------------------------------------------------------------------------*/
/*      The currently defined font is 8x16.  This font packs nicely for use     */
/*      with a graphics engine.  The character arrry must be padded on each     */
/*      row such that each row is on an INT boundary.  The fonts are stored     */
/*      left to right, top to bottom.                                           */
/*------------------------------------------------------------------------------*/
typedef struct _FONT_TABLE {
   int          width;                  /* Number of pels in width              */
   int          height;                 /* Number of pels in height             */
   char *       font;                   /* font array                           */
   int          entries;                /* number of chars in table             */
} FONT_TABLE ;

/*------------------------------------------------------------------------------*/
/*      The current bitmaps and icons are built using OS/2's applications.      */
/*      However, to minimize space, icons and bitmaps need to be built with     */
/*      special care.  The Dakota version of the bitmap ONLY stores the color   */
/*      information, and not the AND/XOR bitmaps.  This means that BLACK is     */
/*      used as the transparent color.  You must use one of the remaining       */
/*      colors as "black".  It is suggested that RGB (8,8,8) be used as black   */
/*      in a bitmap or icon.                                                    */
/*                                                                              */
/*                                                                              */
/*------------------------------------------------------------------------------*/
typedef struct _BITMAP {
   short        columns;                /* Number of pels in width              */
   short        rows;                   /* Number of pels in height             */
   short        colors;                 /* Number of colors used (16 or 256)    */
   short        cBitCount;              /* Number of bits/pel                   */
   short        transparent;            /* TRUE if background passthru          */
   char    *    bits;                   /* Bitmap Array - prefixed with 8 or    */
                                        /* 256 color indicies.                  */
} BITMAP ;

#endif
