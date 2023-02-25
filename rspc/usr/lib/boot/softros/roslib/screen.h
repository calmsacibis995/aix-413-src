/* @(#)43	1.2  src/rspc/usr/lib/boot/softros/roslib/screen.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:52:13  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  Header file for screen/window functions.
 *
 *
 */

#ifndef _H_SCREEN
#define _H_SCREEN

#include <sys/types.h>
#include "screen_fpd.h"

#define  NoERROR  (0)               /* Normal return code of screen function */
#define  ERROR    (1)               /* Error return code of screen function */

#define  VIDEO_BUFFER  (0x000B8000)    /* Beginning or video display address */

#define  SCR_MIN_X     (0)             /* Minimum range of horizontal cursor */
#define  SCR_MAX_X     (80)            /* Maximum range of horizontal cursor */
#define  SCR_MIN_Y     (0)             /* Minimum range of vertical cursor */
#define  SCR_MAX_Y     (25)            /* Maximum range of vertical cursor */

#define  CHAR_SIZE     (2)             /* Size of one character info. */
#define  SPACE_CHAR    (0x20)          /* Default character */
#define  DEFAULT_ATTR  (0x07)          /* Default attribute */
#define  TAB_STOP      (8)             /* Default tab stop length */
 
#define  UP            (+1)            /* Scroll up */
#define  DOWN          (-1)            /* Scroll down */
#define  LEFT          (+2)            /* RESERVED for Scroll left */
#define  RIGHT         (-2)            /* RESERVED for Scroll right */

struct  scr_info {                     /* Screen Info. structure */
	u_int  scr_min_x, scr_max_x;
	u_int  scr_min_y, scr_max_y;

	u_int  win_min_x, win_max_x;
	u_int  win_min_y, win_max_y;

	char   *video;
	u_char  attr;
	u_int   cur_x, cur_y;
	u_int   tab;
};

struct  win_info {                     /*** Window Info. structure ****/
	u_int   min_x, max_x;              /* Horizontal range (min, max) */
	u_int   min_y, max_y;              /* Vertical range   (min, max) */

	char   *video;                     /* Video buffer address */
	u_char  attr;                      /* Color & attribute */
	u_int   cur_x, cur_y;              /* Cursor position (x, y) */
	u_int   tab;                       /* Tab stop */
};

/*** Masking pattern for attributes ***/

#define  FG_MASK        (0xF0)
#define  BG_MASK        (0x8F)
#define  MD_MASK        (0x7F)

/*** Definition of colors ***/

#define  DONT_CARE      (0x00)    /* (= Do not modify attribute) */
#define  BLACK          (0xF0)    /* Used for foreground/background */   
#define  BLUE           (0xF1)
#define  GREEN          (0xF2)
#define  CYAN           (0xF3)
#define  RED            (0xF4)
#define  MAGENTA        (0xF5)
#define  BROWN          (0xF6)
#define  WHITE          (0xF7)

#define  GRAY           (0xF8)    /* Used for foreground ONLY */
#define  LIGHT_BLUE     (0xF9)
#define  LIGHT_GREEN    (0xFA)
#define  LIGHT_CYAN     (0xFB)
#define  LIGHT_RED      (0xFC)
#define  LIGHT_MAGENTA  (0xFD)
#define  YELLOW         (0xFE)
#define  HIGH_WHITE     (0xFF)

#define  NORMAL         (0xF0)    /* Used for display mode */
#define  BLINK          (0xF8)

#endif  /* _H_SCREEN */
