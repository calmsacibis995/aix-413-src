/* @(#)96	1.1  src/rspc/usr/lib/boot/softros/aixmon/esck.h, rspc_softros, rspc411, GOLD410 4/18/94 15:41:15  */
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

/* esck.h -- Header file for the esc-k command line editor	*/

#include	<stdio.h>
#include 	<ctype.h>

/*=*=*=  Program control defines  =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#define MAXCMDL		40	// Max command length
#define HIST_SIZE	10	// Size of history Records

#define TRUE	1
#define FALSE	0

#define COMMAND 	0	// Editor Modes
#define ENTRY   	1
#define HALF_CMD 	2

/*=*=*=  Control Key bindings  *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#define	ENTER		0x0A	// UNIX enter key
#define DOS_ENTER	0x0D	// DOS Enter Key
#define ESC		0x1b	// Escape Key
#define BACKSPC		0x08	// Backspace key

/*=*=*=  Editor Key bindings  =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#define VI_DOWN		'j' 	// Define VI key bindings
#define VI_UP		'k'
#define VI_LEFT		'h'
#define VI_RIGHT	'l'
#define VI_HOME		'0'
#define VI_DEOL		'D'
#define VI_APPEND	'A'
#define VI_append	'a'
#define VI_REPLACE	'R'
#define VI_replace	'r'
#define VI_INSERT	'i'
#define VI_DEL		'x'
#define VI_BACKW	'b'
#define VI_FORWW	'w'

#define	EMACS_DOWN	0x0E 	// Define EMACS key bindings
#define	EMACS_UP	0x10
#define	EMACS_LEFT	0x02
#define	EMACS_RIGHT	0x06
#define	EMACS_HOME	0x01
#define	EMACS_DEOL	0x0B
#define	EMACS_DC	0x04
#define	EMACS_END	0x05

/*=*=*=  Function definitions =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

static void cursor_left(int);
static void cursor_right(int);
static void cmd_up(void);
static void cmd_down(void);
static void cmd_backspace(int);
static void cmd_deol(void);
