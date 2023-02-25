static char sccsid[] = "@(#)93	1.1  src/rspc/usr/lib/boot/softros/aixmon/con_subs.c, rspc_softros, rspc411, GOLD410 4/18/94 15:41:03";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: abs_cursor
 *		absolute
 *		clear_window
 *		cursor
 *		cursor_size
 *		down
 *		get_cursor
 *		init_s3
 *		init_weitek
 *		left
 *		move_cursor
 *		open_window
 *		putbitmap
 *		rel_cursor
 *		right
 *		screen_setup
 *		scroll
 *		scroll_down
 *		scroll_up
 *		set_attribute
 *		tab
 *		up
 *		video_putc
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

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/* con_subs - Routines for console control taken from Sandalfoor ROS	    */
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "stdio.h"
#include "video.h"
#include "colors.h"
#include "console.h"
#include <ctype.h>
						// This is the main video routine
						// Library so declare everything 
						// Right here
#define _VMAIN 1
#include	<p9_video.h>
#include	<s3_video.h>

#define ASYNC  0
#define S3     1
#define WEITEK 2

#define NoERROR (0)
#define ERROR   (1)

static FONT_TABLE doubledot = {8, 16, fnt8x16d, 383};
static FONT_TABLE singledot = {8, 16, fnt8x16s, 383};
static FONT_TABLE asyncfont = {1,  1, fnt8x16d, 383};       // async font table

FONT_TABLE *current_font = &doubledot;

int active_console = ASYNC;			// Put in header file later
SCR_INFO screen;				// This one too
int     auto_scroll = TRUE;

/*------------------------------------------------------------------------------*/
/*      Internal Functions used in console.c                                    */
/*------------------------------------------------------------------------------*/
static void up( void );
static void down( void );
static void right( void );
static void left( void );
       void cls( void );
static void tab( void );

static void  scroll_up( void );
static void  scroll_down( void );

extern int   active_console;
extern int   auto_scroll;
extern FONT_TABLE *current_font;
extern SCR_INFO screen;

/*=========================================================================*/
/* Function:     cursor                                                    */
/* Description:  Move cursor to absolute (x,y) position                    */
/*               (x,y) must be within the window range                     */
/* Input:        Horizontal location on screen                             */
/*               Vertical location on screen                               */
/* Ouput:        NoERROR                                                   */
/*               ERROR                                                     */
/*                                                                         */
int  cursor( int x, int y )
{
        x += screen.win_min_x;
        y += screen.win_min_y;
        // Error checking
        if ( (x < screen.win_min_x) || (screen.win_max_x < x) ||
             (y < screen.win_min_y) || (screen.win_max_y < y)) {
                return(ERROR);
        }
        else {
                screen.cur_x = x;  screen.cur_y = y;
                move_cursor( screen.cur_x, screen.cur_y );
                return(NoERROR);
        }
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// abs_cursor - Move cursor without changing screen variables
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

int  abs_cursor( int x, int y )
{
        // Error checking
        if ( (x < screen.win_min_x) || (screen.win_max_x < x) ||
             (y < screen.win_min_y) || (screen.win_max_y < y)) {
                return(ERROR);
        }
        else {
                screen.cur_x = x;  screen.cur_y = y;
                move_cursor( screen.cur_x, screen.cur_y );
                return(NoERROR);
        }
}

/*=========================================================================*/
/* Function:     rel_cursor                                                */
/* Description:  Move cursor to relative (x,y) position                    */
/* Input:        Horizontal displacement of cursor                         */
/*               Vertical displacement of cursor                           */
/* Ouput:        NoERROR                                                   */
/*                                                                         */
int  rel_cursor( int x, int y )
{
        if ( x > 0 )  for ( ; x > 0; x-- ) right();
        else          for ( ; x < 0; x++ ) left();

        if ( y > 0 )  for ( ; y > 0; y-- ) down();
        else          for ( ; y < 0; y++ ) up();

        return(NoERROR);
}

/*=========================================================================*/
/* Function:     get_cursor                                                */
/* Description:  Return current cursor position                            */
/* Input:        Pointer to horizontal value                               */
/*               Pointer to vertical value                                 */
/* Ouput:        NoERROR                                                   */
/*                                                                         */
int  get_cursor( int *x, int *y )
{
        *x = screen.cur_x - screen.win_min_x;
        *y = screen.cur_y - screen.win_min_y;
        return(NoERROR);
}

//===========================================================================
// Function:     move_cursor
// Description:  Move cursor to absolute (x,y) position
//               (x,y) must be within screen range ... No error checking
// Input:        Horizontal location on screen
//               Vertical location on screen
// Ouput:        NoERROR
//
static int  move_cursor( int x, int y )
{
switch(active_console)
	{
	case	ASYNC:
printf("This area under construction\n");
	break;

	case	S3:
	s3_goto_xy(TRUE, x*current_font->width, y*current_font->height);
	break;

	case	WEITEK:
	p9_goto_xy(TRUE, x*current_font->width, y*current_font->height);
	break;
	}

return( NoERROR );
}

//===========================================================================
// Function:     right
// Description:  Move cursor right, goto next line if at end of line
// Input:        none
// Ouput:        none
//
static void  right( void )
{
        if ( screen.cur_x < (screen.win_max_x ) ) {
                screen.cur_x++;
                move_cursor(screen.cur_x, screen.cur_y);
        }
        else {
                screen.cur_x = screen.win_min_x;
                down();
        }
}

//===========================================================================
// Function:     left
// Description:  Move cursor left, goto previous line if at beginning of line
// Input:        none
// Ouput:        none
//
static void  left( void )
{
        if ( screen.win_min_x < screen.cur_x ) {
                screen.cur_x--;
                move_cursor(screen.cur_x, screen.cur_y);
        }
        else {
                screen.cur_x = (screen.win_max_x - 1);
                up();
        }
}

//===========================================================================
// Function:     up
// Description:  Move cursor up, scroll down if necessary
// Input:        none
// Ouput:        none
//
static void  up( void )
{
        if ( screen.win_min_y < screen.cur_y ) {
                screen.cur_y--;
        }
        else {
                scroll_down();
        }
        move_cursor(screen.cur_x, screen.cur_y);
}

//===========================================================================
// Function:     down
// Description:  Move cursor down, scroll up if necessary
// Input:        none
// Ouput:        none
//
void  down( void )
{
        if ( screen.cur_y < (screen.win_max_y ) ) {
                screen.cur_y++;
        }
        else {
                scroll_up();
        }
        move_cursor(screen.cur_x, screen.cur_y);
}

/*=========================================================================*/
/* Function:     scroll                                                    */
/* Description:  Scroll up/down within the window                          */
/*               Right/Left to be supported later.                         */
/* Input:        none                                                      */
/* Ouput:        none                                                      */
/*                                                                         */
int  scroll( int dir )
{

  if (dir)
     scroll_up();
  else
     scroll_down();

  return( NoERROR );
}

/*=========================================================================*/
/* Function:     scroll_up                                                 */
/* Description:  Scroll up within the window                               */
/* Input:        none                                                      */
/* Ouput:        none                                                      */
/*                                                                         */

void  scroll_up( void ) 
{
if (auto_scroll)
	{
	switch(active_console)
		{
		case	ASYNC:
printf("This area under construction\n");
		break;

		case	S3:
		s3_scroll_window(screen.win_min_x * current_font->width,
                             screen.win_min_y * current_font->height,
                             (screen.win_max_x-screen.win_min_x+1) * current_font->width,
                             (screen.win_max_y-screen.win_min_y+1) * current_font->height,
                              0,
                             current_font->height);
		break;

		case	WEITEK:
		p9_scroll_window(screen.win_min_x * current_font->width,
                             screen.win_min_y * current_font->height,
                             (screen.win_max_x-screen.win_min_x+1) * current_font->width,
                             (screen.win_max_y-screen.win_min_y+1) * current_font->height,
                              0,
                             current_font->height);
		break;
		}
	}
}

/*=========================================================================*/
/* Function:     scroll_down                                               */
/* Description:  Scroll down within the window                             */
/* Input:        none                                                      */
/* Ouput:        none                                                      */
/*                                                                         */
void  scroll_down( void ) 
{

if (auto_scroll)
	{
	switch(active_console)
		{
		case	ASYNC:
printf("This area under construction\n");
		break;

		case	S3:
		s3_scroll_window(screen.win_min_x * current_font->width,
                              screen.win_min_y * current_font->height,
                              (screen.win_max_x-screen.win_min_x+1) * current_font->width,
                              (screen.win_max_y-screen.win_min_y+1) * current_font->height,
                              0,
                              -current_font->height);
		break;

		case	WEITEK:
		p9_scroll_window(screen.win_min_x * current_font->width,
                              screen.win_min_y * current_font->height,
                              (screen.win_max_x-screen.win_min_x+1) * current_font->width,
                              (screen.win_max_y-screen.win_min_y+1) * current_font->height,
                              0,
                              -current_font->height);
		break;
		}
	}
}

//===========================================================================
// Function:     tab
// Description:  Forward one tab character.
// Input:        none
// Ouput:        none
//
static void  tab( void )
{
        screen.cur_x = ((screen.cur_x / screen.tab) + 1) * screen.tab;
        if (screen.cur_x >= screen.win_max_x)
           screen.cur_x = screen.win_max_x -1;

        abs_cursor( screen.cur_x, screen.cur_y );
}


/*=========================================================================*/
/*  generic bitmap routine
/*=========================================================================*/

void    putbitmap(BITMAP * b,           /* Pointer to BITMAP structure          */
                   int    x,            /* at this x position (0 = left)        */
                   int    y) {          /* at this y position (0 = top)         */

   if (active_console == S3)
      s3_putbitmap( b, x, y);
   else
   if (active_console == WEITEK)
      p9_putbitmap( b, x, y);

}

/*=========================================================================*/
/*  generic cursor size routine
/*=========================================================================*/

int     cursor_size(int new_size) 
{     				   /* Set cursor to this new shape         */
switch(active_console)
	{
	case	ASYNC:
printf("This area under construction\n");
	break;

	case	S3:
	s3_cursor_size(new_size);
	break;

	case	WEITEK:
	p9_cursor_size(new_size);
	break;
	}
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// putc function for the p9 display adapter
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

video_putc(uchar chr)
{

switch ( chr ) 
	{
	case '\b': rel_cursor(-1,0); putchar(' '); rel_cursor(-1,0); break;
	case '\r': abs_cursor( screen.win_min_x, screen.cur_y ); break;
	case '\n': abs_cursor( screen.win_min_x, screen.cur_y ); down();  break;
	case '\v':  down();  break;
	case '\t':  tab();   break;

	default:
	switch(active_console)
                {
                case    ASYNC:
printf("This area under construction\n");
                break;

                case    S3:
		s3_putchar(chr, -1, -1,
			   screen.cur_x * current_font->width,
			   screen.cur_y * current_font->height,current_font);
		rel_cursor(1,0);
                break;

                case    WEITEK:
		p9_putchar(chr, -1, -1,
			   screen.cur_x * current_font->width,
			   screen.cur_y * current_font->height,current_font);
		rel_cursor(1,0);
		break;
                }
	}

return( NoERROR );
}
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// set_attribute - Set Screen Colors
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
int  set_attribute( u_char fg, u_char bg)
{
switch(active_console)
	{
	case	ASYNC:
printf("This area under construction\n");
	break;

	case	S3:
	s3_set_colors(fg,bg);
	break;

	case	WEITEK:
	p9_set_colors(fg,bg);
	break;
	}
return( NoERROR );
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//	screen_setup() - Routine initializes screen structure
//			 for use in display control routines
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

screen_setup(int s_mode)
{

screen.cur_x = 0;
screen.cur_y = 0;
screen.scr_min_x = 0;
screen.scr_min_y = 0;

screen.win_min_x = 0;
screen.win_min_y = 0;

screen.tab       = 8;

switch(s_mode)
	{
	case	0: 			/* 640x480x8 Mode               */
	screen.width  = 640;
	screen.height = 480;
	screen.mode   = MODE_VESA_201; 
	break;

	case	1:			/* 1024x768x8 Mode              */
	screen.width  = 1024;
	screen.height = 768;
	screen.mode   = MODE_VESA_205; 
	break;

	case	2:			/* 1280x1024x8 Mode             */
	screen.width  = 1280;
	screen.height = 1024;
	screen.mode   = MODE_VESA_107; 
	break;
	
	default:
	printf("Bad screen mode selected\n");
	return FALSE;
	break;
	}

screen.scr_max_y = (screen.height / current_font->height) - 1;
screen.scr_max_x = (screen.width  / current_font->width ) - 1;
screen.win_max_y = (screen.height / current_font->height) - 1;
screen.win_max_x = (screen.width  / current_font->width ) - 1;

return(TRUE);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// open_window - Opens a display window ala curses
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

int  open_window( int x0, int y0, int x1, int y1 )
{
        if ( (x0 < screen.scr_min_x) || (y0 < screen.scr_min_y) ||
             (x1 > screen.scr_max_x) || (y1 > screen.scr_max_y) ) {
                return( ERROR );
        }
        else {
                screen.win_min_x = x0;
                screen.win_min_y = y0;
                screen.win_max_x = x1;
                screen.win_max_y = y1;
                clear_window();
                return( NoERROR );
        }
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// clear_window - Clears a screen region 
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

clear_window(void)
{
switch(active_console)
	{
	case	ASYNC:
	break;

	case	S3:
	s3_scroll_window (screen.win_min_x * current_font->width,
                          screen.win_min_y * current_font->height,
                          (screen.win_max_x-screen.win_min_x+1) * current_font->width,
                          (screen.win_max_y-screen.win_min_y+1) * current_font->height,
                          0,
                          (screen.win_max_y-screen.win_min_y+1) * current_font->height);

	screen.cur_x = screen.win_min_x;
	screen.cur_y = screen.win_min_y;
	move_cursor ( screen.cur_x, screen.cur_y );
	break;

	case	WEITEK:
	p9_scroll_window (screen.win_min_x * current_font->width,
                          screen.win_min_y * current_font->height,
                          (screen.win_max_x-screen.win_min_x+1) * current_font->width,
                          (screen.win_max_y-screen.win_min_y+1) * current_font->height,
                          0,
                          (screen.win_max_y-screen.win_min_y+1) * current_font->height);

	screen.cur_x = screen.win_min_x;
	screen.cur_y = screen.win_min_y;
	move_cursor ( screen.cur_x, screen.cur_y );
	break;
	}
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// init_weitek - Initialize the Weitek adapter and blank the screen
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

init_weitek(void)
{
int	save_con;
					/* Try to initialize the Weitek     */
screen_setup(0);
if(weitek_init(screen.mode))
	{
	save_con = active_console;
	active_console = WEITEK;
   	set_attribute(CLR_BLACK,CLR_BLACK);
	open_window(0,0, screen.scr_max_x,screen.scr_max_y);
	video_putc(' ');
	active_console = save_con;
	return(WEITEK);
	}
return(0);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// init_s3 - Initialize the S3 adapter and blank the screen
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

init_s3(void)
{
int	save_con;
					/* Try to initialize the S3	     */
screen_setup(0);
if(s3_init(screen.mode))
	{
	save_con = active_console;
	active_console = S3;
   	set_attribute(CLR_BLACK,CLR_BLACK);
	open_window(0,0, screen.scr_max_x,screen.scr_max_y);
	video_putc(' ');
	active_console = save_con;
	return(S3);
	}
return(0);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// new_video_screen - 	This module initialized the screen 
//			For the moment, we are using the screen structure from
//			the boca ROS. It allows only one screen definition for
//			all windows. SO when we change screens, we will rbuild
//			the entire screen. Let's see if anyone notices. 
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

new_video_screen(int console)
{


if((console == WEITEK) || (console == S3))
	{
	active_console = console;
	video_console(active_console);
	screen_setup(0);
	version();
	set_attribute(CLR_BLACK,CLR_WHITE);
	open_window(0,3, screen.scr_max_x,screen.scr_max_y);
	return(TRUE);
	}
if(console == ASYNC)
	{
	active_console = console;
	return(TRUE);
	}

printf("Invalid Graphics Console Selected - AIXMON internal error\n");
return(FALSE);
}


