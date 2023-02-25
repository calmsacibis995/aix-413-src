/* @(#)37	1.1  src/rspc/usr/lib/boot/softros/roslib/console.h, rspc_softros, rspc411, GOLD410 4/18/94 15:59:22  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: putbitmap
 *		rect_fill
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
/*      Console Interface                                                       */
/*                                                                              */
/*                                                                              */
/*------------------------------------------------------------------------------*/

#include <video.h>

#ifndef _H_CONSOLE
#define _H_CONSOLE

typedef struct _SCR_INFO {                      /* Character Screen Info. structure */
        int  width,height,mode;         /* pel x,y, and mode info           */

        int  scr_min_x, scr_max_x;
        int  scr_min_y, scr_max_y;

        int  win_min_x, win_max_x;
        int  win_min_y, win_max_y;

        int  tab;

        int  cur_x, cur_y;
} SCR_INFO;

#define CURSOR_OFF      0
#define CURSOR_NORMAL   1
#define CURSOR_HALF     2
#define CURSOR_FULL     3

int  console_init(int s_mode);          /* Initialize Console Communications    */

int  putchar_at(unsigned char chr,      /* Write a character to a spot on the   */
                int x,                  /* display                              */
                int y,
                int f_color,
                int b_color);

int  open_window( int x0,               /* Upper left Point                     */
                  int y0,
                  int x1,               /* Lower Right Point                    */
                  int y1 );

int  cursor_size(int new_size);         /* Set cursor shape                     */

int  cursor( int x, int y );            /* Move cursor to column x, row y       */

int  rel_cursor( int x, int y );        /* Move Cursor relative to current loc  */

void putbitmap(BITMAP * b,              /* Pointer to BITMAP structure          */
               int    x,                /* at this x position (0 = left)        */
               int    y);               /* at this y position (0 = top)         */

void load_palette(char * table);        /* Pointer to palette data (RGBRGB...)  */

void rect_fill(int ul_x,                /* Rectangle Fill Command - using       */
               int ul_y,                /* upper left and lower right (x,y).    */
               int lr_x,
               int lr_y,
               int color);
#endif
