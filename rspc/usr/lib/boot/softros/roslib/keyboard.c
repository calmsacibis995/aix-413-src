static char sccsid[] = "@(#)16	1.1  src/rspc/usr/lib/boot/softros/roslib/keyboard.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:48";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: Disable_Keyboard
 *		Disable_Mouse
 *		Enable_Keyboard
 *		Enable_Mouse
 *		Getchar
 *		Keyboard_Init
 *		check_key_avail
 *		flush
 *		hit
 *		i8042_Initialize
 *		process_keystroke
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

// Keyboard handler.

// Defines.

// These defines are completely arbitrary.

#define DEBUGGER_ACCESS 0x0fe5a
#define CTRL_C          0x0fe5b

//AIX// These defines are added to make control work correctly 

#define	L_CTRL	0x11
#define R_CTRL  0x58
#define L_ALT	0x19
#define R_ALT	0x39

// Externals.

extern int in_debug;

// The globals which reflect certain key states or key events.

volatile int break_detected = 0;
volatile int extended_code = 0;
volatile int ctrl_key = 0;
volatile int alt_key = 0;
volatile int shift_key = 0;
volatile int num_lock = 0;
volatile int ctrl_c = 0;
volatile int E0_prefix = 0;

// The buffer to hold the inbound stream of *processed* keystrokes.

#define kbd_bfr_size 64
#ifndef USE_POLLING
volatile static unsigned int kbd_bfr[kbd_bfr_size];
#else
volatile unsigned int kbd_bfr[kbd_bfr_size];
#endif
volatile unsigned int take_from = 0;
volatile unsigned int put_to = 0;

// Includes

#include <sys/types.h>
#include <misc_sub.h>
#include "keyboard.h"
#include "io_sub.h"
#include "int_hndlr.h"

// The primary conversion table.
// The scan code is an index into this table, hopefully giving
// us something useful, maybe even an ASCII character.

unsigned int CONV1[] = {
   0x00,        // 0x00, 000
   323 ,        // 0x01, 001 F9
   0x00,        // 0x02, 002
   319 ,        // 0x03, 003 F5
   317 ,        // 0x04, 004 F3
   315 ,        // 0x05, 005 F1
   316 ,        // 0x06, 006 F2
   390 ,        // 0x07, 007 F12
   0x1b,        // 0x08, 008
   324 ,        // 0x09, 009 F10
   322 ,        // 0x0a, 010 F8
   320 ,        // 0x0b, 011 F6
   318 ,        // 0x0c, 012 F4
   0x09,        // 0x0d, 013 Tab
   0x60,        // 0x0e, 014 (thing under tilde)
   0x00,        // 0x0f, 015
   0x00,        // 0x10, 016
   0x00,        // 0x11, 017
   0x00,        // 0x12, 018
   0x00,        // 0x13, 019
   0x00,        // 0x14, 020
   'q' ,        // 0x15, 021
   '1' ,        // 0x16, 022
   0x00,        // 0x17, 023
   0x00,        // 0x18, 024
   0x00,        // 0x19, 025
   'z' ,        // 0x1a, 026
   's' ,        // 0x1b, 027
   'a' ,        // 0x1c, 028
   'w' ,        // 0x1d, 029
   '2' ,        // 0x1e, 030
   0x00,        // 0x1f, 031
   0x00,        // 0x20, 032
   'c' ,        // 0x21, 033
   'x' ,        // 0x22, 034
   'd' ,        // 0x23, 035
   'e' ,        // 0x24, 036
   '4' ,        // 0x25, 037
   '3' ,        // 0x26, 038
   0x00,        // 0x27, 039
   0x00,        // 0x28, 040
   ' ' ,        // 0x29, 041 Space
   'v' ,        // 0x2a, 042
   'f' ,        // 0x2b, 043
   't' ,        // 0x2c, 044
   'r' ,        // 0x2d, 045
   '5' ,        // 0x2e, 046
   0x00,        // 0x2f, 047
   0x00,        // 0x30, 048
   'n' ,        // 0x31, 049
   'b' ,        // 0x32, 050
   'h' ,        // 0x33, 051
   'g' ,        // 0x34, 052
   'y' ,        // 0x35, 053
   '6' ,        // 0x36, 054
   0x00,        // 0x37, 055
   0x00,        // 0x38, 056
   0x00,        // 0x39, 057
   'm' ,        // 0x3a, 058
   'j' ,        // 0x3b, 059
   'u' ,        // 0x3c, 060
   '7' ,        // 0x3d, 061
   '8' ,        // 0x3e, 062
   0x00,        // 0x3f, 063
   0x00,        // 0x40, 064
   ',' ,        // 0x41, 065
   'k' ,        // 0x42, 066
   'i' ,        // 0x43, 067
   'o' ,        // 0x44, 068
   '0' ,        // 0x45, 069
   '9' ,        // 0x46, 070
   0x00,        // 0x47, 071
   0x00,        // 0x48, 072
   '.' ,        // 0x49, 073
   0x2f,        // 0x4a, 074 forward slash
   'l' ,        // 0x4b, 075
   ';' ,        // 0x4c, 076
   'p' ,        // 0x4d, 077
   '-' ,        // 0x4e, 078
   0x00,        // 0x4f, 079
   0x00,        // 0x50, 080
   0x00,        // 0x51, 081
   0x27,        // 0x52, 082 single quote
   0x00,        // 0x53, 083
   '[' ,        // 0x54, 084
   '=' ,        // 0x55, 085
   0x00,        // 0x56, 086
   0x00,        // 0x57, 087
   0x00,        // 0x58, 088
   0x00,        // 0x59, 089
//AIX//    0x0d,        // 0x5a, 090 Enter
   0x0a,        // 0x5a, 090 Enter
   ']' ,        // 0x5b, 091
   0x00,        // 0x5c, 092
   0x5c,        // 0x5d, 093 back slash
   0x00,        // 0x5e, 094
   0x00,        // 0x5f, 095
   0x00,        // 0x60, 096
   0x00,        // 0x61, 097
   0x00,        // 0x62, 098
   0x00,        // 0x63, 099
   0x00,        // 0x64, 100
   0x00,        // 0x65, 101
   0x08,        // 0x66, 102 Backspace
   0x00,        // 0x67, 103
   0x00,        // 0x68, 104
   335 ,        // 0x69, 105 End
   0x00,        // 0x6a, 106
   331 ,        // 0x6b, 107 Left
   327 ,        // 0x6c, 108 Home
   0x00,        // 0x6d, 109
   0x00,        // 0x6e, 110
   0x00,        // 0x6f, 111
   338 ,        // 0x70, 112 Ins
   339 ,        // 0x71, 113 Del
   336 ,        // 0x72, 114 Down
   0x00,        // 0x73, 115
   333 ,        // 0x74, 116
   328 ,        // 0x75, 117 Up
   0x1b,        // 0x76, 118 Esc
   0x00,        // 0x77, 119
   389 ,        // 0x78, 120 F11
   '+' ,        // 0x79, 121
   337 ,        // 0x7a, 122 PgDn
   '-' ,        // 0x7b, 123
   '*' ,        // 0x7c, 124
   329 ,        // 0x7d, 125 PgUp
   0x00,        // 0x7e, 126
   0x00,        // 0x7f, 127
   0x00,        // 0x80, 128
   0x00,        // 0x81, 129
   0x00,        // 0x82, 130
   321          // 0x83, 131 F7
   };

//------------------------------------------------------------------------
// PROCESS KEYSTROKE: this is where all the work gets done
//------------------------------------------------------------------------

// We need to add the keyboard LED processing.

int process_keystroke(unsigned char data) {
   int stroke;

// if this is a break code, record that event and return

     if (data == 0xf0 ) {
        break_detected = 1;
        return -1;
     }

// if this is an extended code, well... throw it out
// (no reason, other than I'm not handling them right now)

// We'll need to handle these for at least
//  left/right ctrl, left/right alt, left/right enter

     if (data == 0xe0 ) {
        return -1;
     }

// if we are in a break state, throw away the data but
// track special keys

     if (break_detected) {
        break_detected = 0;
        switch(data) {
        case 0x59:
                shift_key &= ~2;
                break;
        case 0x12:
                shift_key &= ~1;
                break;
        case L_ALT:
        case R_ALT:
                alt_key = 0;
                break;
        case L_CTRL:
        case R_CTRL:
                ctrl_key = 0;
                break;
        }

        return -1;
     }

// monitor the shift and control keys

     switch(data) {
     case 0x59:
                shift_key |= 2;
                return -1;
     case 0x12:
                shift_key |= 1;
                return -1;
//AIX//    case 0x58:
     case 0x14:				// I assume you ment caps_lock
                if ( shift_key && 4 ) {
                   shift_key &= ~4;
                } else {
                   shift_key |= 4;
                } /* endif */
                return -1;
     case L_ALT:
     case R_ALT:
                alt_key = 1;
                return -1;
     case L_CTRL:
     case R_CTRL:
                ctrl_key = 1;
                return -1;
     case 0x77:
                num_lock = ~num_lock;
                return -1;
     default:
                break;
     }

// range check the table lookup

   if (data <= 131 ) {
      stroke = CONV1[ (unsigned int) data];
   } else {
      return -1;
   }


// The special Ctrl-Alt-<something> processing

   if (ctrl_key && alt_key) {
      if ( stroke == 339 ) reboot_cmd();
      if ( stroke == 0x1b ) return DEBUGGER_ACCESS;
   } /* endif */


// control key processing

   if (ctrl_key) {
	if(stroke > 0x40 && stroke < 0x61)
		return(stroke - 0x40);
	if(stroke > 0x60 && stroke < 0x80)
		return(stroke - 0x60);
	return -1;

//AIX//       if ( stroke == 0x0d ) return 10;
//AIX//       if ( stroke == 'j' ) return 10;
//AIX//       if ( stroke == 0x08 ) return 127;
//AIX//       if ( stroke == (256+71) ) return 256+119;
//AIX//       if ( stroke == (256+79) ) return 256+117;
//AIX//       if ( stroke == (256+75) ) return 256+115;
//AIX//       if ( stroke == (256+77) ) return 256+116;
//AIX//       if ( stroke == 'e' ) return 5;
//AIX//       if ( stroke == 'f' ) return 6;
//AIX//       if ( stroke == 'h' ) return 8;
//AIX//       if ( stroke == 'i' ) return 9;
//AIX//       if ( stroke == 'j' ) return 10;
//AIX//       if ( stroke == 'm' ) return 13;
//AIX//       if ( stroke == '[' ) return 27;
//AIX//       if ( stroke == 'c' ) return CTRL_C;

   }

// alt key processing

   if (alt_key) {
      if ( stroke == 'r' ) return 256+19;
      if ( stroke == 'y' ) return 256+21;
      if ( stroke == 'u' ) return 256+22;
      if ( stroke == 's' ) return 256+31;
      if ( stroke == 'd' ) return 256+32;
      if ( stroke == 'j' ) return 256+36;
      if ( stroke == 'l' ) return 256+38;
      if ( stroke == 'x' ) return 256+45;
      if ( stroke == 'c' ) return 256+46;
      if ( stroke == 'm' ) return 256+50;

      if ( stroke == '1' ) return 192;
      if ( stroke == '2' ) return 193;
      if ( stroke == '3' ) return 217;
      if ( stroke == '4' ) return 195;
      if ( stroke == '5' ) return 197;
      if ( stroke == '6' ) return 180;
      if ( stroke == '7' ) return 218;
      if ( stroke == '8' ) return 194;
      if ( stroke == '9' ) return 191;

      if ( stroke == 324 ) return 256+113;
   }

// Handle alpha here, special case becuase of caps lock
// shift toggles case if caps lock on

   if ( isalpha(stroke) && shift_key) {
      if ( (shift_key & 4) && (shift_key & 3) ) {
         return stroke;
      } else {
         return toupper(stroke);
      } /* endif */
   } /* endif */


// shift key processing (left or right shift)

   if (shift_key & 3) {
      if ( stroke == 0x09) return 256+15;
      if ( stroke == 315 ) return 256+84;
      if ( stroke == 316 ) return 256+85;
      if ( stroke == 317 ) return 256+86;
      if ( stroke == 318 ) return 256+87;
      if ( stroke == 0x09) return 256+15;
      if ( stroke == 0x09) return 256+15;
      if ( stroke == 0x09) return 256+15;
      if ( stroke == 0x60) return '~';
      if ( stroke == '1' ) return '!';
      if ( stroke == '2' ) return '@';
      if ( stroke == '3' ) return '#';
      if ( stroke == '4' ) return '$';
      if ( stroke == '5' ) return '%';
      if ( stroke == '6' ) return '^';
      if ( stroke == '7' ) return '&';
      if ( stroke == '8' ) return '*';
      if ( stroke == '9' ) return '(';
      if ( stroke == '0' ) return ')';
      if ( stroke == '-' ) return '_';
      if ( stroke == '=' ) return '+';
      if ( stroke == '[' ) return '{';
      if ( stroke == ']' ) return '}';
      if ( stroke == 0x5c) return '|';
      if ( stroke == ';' ) return ':';
      if ( stroke == 0x27) return '"';
      if ( stroke == ',' ) return '<';
      if ( stroke == '.' ) return '>';
      if ( stroke == 0x2f) return '?';
   }

// if we don't know what it is, return -1

   if (stroke == 0 ) return -1;

// it must be something, return it

   return stroke;

}

//------------------------------------------------------------------------
//  KEYBOARD INTERRUPT HANDLER
//------------------------------------------------------------------------
static int IRQ(void){
   unsigned char data;
   int key;

// first, get the data from the controller

   data = inb(0x60);

// now, try to make sense of the scan code

   key = process_keystroke(data);

// we simply return if:
//  we didn't like what we saw
//  we have no data yet (just a state change)

   if (key == -1) return 0;

// Is this the special debugger invocation call ?
// the return code 88 is a message to the FLIH (first-level interrupt handler)

   if ( (key == DEBUGGER_ACCESS) && (in_debug == 0) ) {
      in_debug = 0x08;
      return 88;
   } /* endif */

// Might this be crtl_c ?
// If it is, clear the keyboard buffer, and set ctrl_c flag.
// We clear ctrl_c in check_key_avail (proc name may change).
// We also clear it in Getchar.  Why, when normal interaction resumes
// we do not want a stale break lying around.

   if ( key == CTRL_C ) {
      take_from = 0;
      put_to = 0;
      ctrl_c = 1;
      return 0;
   } /* endif */


// OK, try to add the keystroke to the buffer.
// If its full, it gets thrown away.

   if((take_from == put_to) || ((put_to+1)%kbd_bfr_size != take_from)) {
      kbd_bfr[put_to] = key;
      put_to = ++put_to % kbd_bfr_size;
   }

// back to the first-level interrupt handler

   return 0;

}


//------------------------------------------------------------------------
// Issue the enable keyboard command to the 8042
//------------------------------------------------------------------------

// This will need a timeout and return code.

static void Enable_Keyboard(void){
   while( (inb(i8042_Command_Port) & 0x2) == 2);
   outb(i8042_Command_Port, 0xAE);
}

//------------------------------------------------------------------------
// Issue the disable keyboard command to the 8042
//------------------------------------------------------------------------

// This will need a timeout and return code.

#if 0 /* see ktsm_compat.c */
static void Disable_Keyboard(void){
   while( (inb(i8042_Command_Port) & 0x2) == 2);
   outb(i8042_Command_Port, 0xAD);
}
#endif /* 0 */

//------------------------------------------------------------------------
// Issue the enable mouse command to the 8042
//------------------------------------------------------------------------

// This will need a timeout and return code.

static void Enable_Mouse(void){
   while( (inb(i8042_Command_Port) & 0x2) == 2);
   outb(i8042_Command_Port, 0xA8);
}

//------------------------------------------------------------------------
// Issue the disable mouse command to the 8042
//------------------------------------------------------------------------

// This will need a timeout and return code.

#if 0 /* see ktsm_compat.c */
static void Disable_Mouse(void){
   while( (inb(i8042_Command_Port) & 0x2) == 2);
   outb(i8042_Command_Port, 0xA7);
}
#endif /* 0 */

//------------------------------------------------------------------------
// See if the break sequence has been hit (ctrl-c).
//------------------------------------------------------------------------
int check_key_avail(void){
   if (ctrl_c) {
      ctrl_c = 0;
      return TRUE;
   } else {
      return FALSE;
   } /* endif */
}

//------------------------------------------------------------------------
// Clear the 8042 output buffer if there is data pending
//------------------------------------------------------------------------

// This will need a timeout and return code.

static void flush(void) {
  unsigned char status;

  status = out_buf_full;
  while (status & out_buf_full) {
    status = inb(i8042_Command_Port);
    if (status & out_buf_full) inb(i8042_Data_Port);
  } /* endwhile */
}

//------------------------------------------------------------------------
// Initialize the 8042 by flushing its output buffer and setting its
//  command byte.
//------------------------------------------------------------------------
void i8042_Initialize(void){
unsigned char tmp_value;
int i;
unsigned char c;
rtc_value my_timeout;

   break_detected = 0;
   extended_code = 0;
   ctrl_key = 0;
   alt_key = 0;
   shift_key = 0;
   num_lock = 0;

   take_from = 0;
   put_to = 0;

   for(i=0;i<5000;i++) inb(i8042_Data_Port);

// issue reset to the 8042
   outb(i8042_Command_Port, 0xaa);

   for(i=0;i<5000;i++) inb(0x64);

// wait for the 8042 to complete self test
   while( (inb(i8042_Command_Port) & out_buf_full) == 0);

   if( c=inb(i8042_Data_Port) != 0x55) printf("8042 Reset Error: %x\n", c);

   for(i=0;i<5000;i++) inb(i8042_Data_Port);

// write command byte

   while( (inb(i8042_Command_Port) & 0x2) == 2);
   outb(i8042_Command_Port, 0x60);

// wait for 8042 to accept command

   for(i=0;i<5000;i++) inb(0x64);

   while( (inb(i8042_Command_Port) & 0x2) == 2);

// disable both mouse and keyboard, enable interrupt mode

   outb(i8042_Data_Port, 0x33);

// we should add a check which makes sure 0x33 got out there
// read command byte
   for(i=0;i<5000;i++) inb(i8042_Data_Port);
   while( (inb(i8042_Command_Port) & 0x2) == 2);
   outb(i8042_Command_Port, 0x20);
// wait for 8042 to accept command
   my_timeout = get_timeout_value(15,MILLISECONDS);
   while(timeout(my_timeout) == FALSE);
// wait until output buffer full flag bit = 1
   while( (inb(i8042_Command_Port) & out_buf_full) == 0);
   c = inb(i8042_Data_Port);
   if (c != 0x33) {printf("Enable Interrupt Error: %x\n", c);}

}

//------------------------------------------------------------------------
// Initialize the 8042 then enable the keyboard.
//------------------------------------------------------------------------
void Keyboard_Init(void){

   i8042_Initialize();

#ifndef USE_POLLING
   ext_int_registrar(ASSIGN_INTERRUPT_HANDLER_ADDRESS, 12, IRQ);
#endif
   Enable_Mouse();

#ifndef USE_POLLING
   ext_int_registrar(ASSIGN_INTERRUPT_HANDLER_ADDRESS, 1, IRQ);
#endif
   Enable_Keyboard();

}

//------------------------------------------------------------------------
// get a character from the keyboard
//------------------------------------------------------------------------
int Getchar(void) {
   int akey;

   ctrl_c = 0;

// if there is a key in the buffer, return it

   if(take_from != put_to){
     akey = kbd_bfr[take_from];
     take_from = ++take_from % kbd_bfr_size;
     return akey;
   }
   else
   {
     return -1;
   }
}

