/* @(#)42	1.1  src/rspc/usr/lib/boot/softros/roslib/s3_video.h, rspc_softros, rspc411, GOLD410 4/18/94 15:59:41  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: s3_putbitmap
 *		s3_putchar
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
/*                                                                              */
/*      S3 Video Interface.                                                     */
/*                                                                              */
/*      found = s3_init(mode);                                                  */
/*                                                                              */
/*      The PCI slots are examined.  If a PCI S3 Video adapter is present,      */
/*      it is initialized to the mode requested and a value of TRUE is returned.*/
/*                                                                              */
/*      If no S3 Video is found, then FALSE is returned.                        */
/*                                                                              */
/*      Both slots are examined.  A second S3 Video adapter is disabled.  The   */
/*      order of examination is PCI SLOT1 then PCI SLOT2.                       */
/*                                                                              */
/*------------------------------------------------------------------------------*/

#ifndef _H_S3_VIDEO
#define _H_S3_VIDEO

int     s3_init(int);                   /* Initialize s3 to MODE_VESA_xxx       */

int     s3_Found(int setup);            /* return TRUE if any S3 found          */

void    s3_clear_screen(void);          /* Clear the Display                    */

void    s3_goto_xy(int con,int x,int y);/* Place Cursor on Display (con = TRUE) */
                                        /* Turn off cursor (con = FALSE)        */

void    s3_putchar(int    char_code,    /* The character to place on the screen */
                   int    color_f,      /* Color to use for drawing the char    */
                   int    color_b,      /* Color to use for drawing the backgrnd*/
                   int    x,            /* at this x position (0 = left)        */
                   int    y,            /* at this y position (0 = top)         */
                   FONT_TABLE * f);     /* The font table                       */

void    s3_load_palette(char * table);  /* Pointer to RGBRGBRGB... 256*3 bytes  */

void    s3_putbitmap(BITMAP * b,        /* Pointer to BITMAP structure          */
                   int    x,            /* at this x position (0 = left)        */
                   int    y);           /* at this y position (0 = top)         */

void    s3_scroll_window(int window_x,  /* Left pixel of window                 */
                         int window_y,  /* Top pixel of window                  */
                         int window_cx, /* Width of window                      */
                         int window_cy, /* Height of window                     */
                         int scroll_x,  /* Number of horizontal pixels to move  */
                         int scroll_y); /* Number of vertical pixels to move    */

void    s3_set_colors(int f_color,
                      int b_color);     /* Set foreground and background colors */

void    s3_rect_fill(int ul_x,          /* Rectangle Fill Command               */
                     int ul_y,
                     int lr_x,
                     int lr_y,
                     int color);

int     s3_cursor_size(int new_size);   /* Set Cursor Shape                     */
#endif
