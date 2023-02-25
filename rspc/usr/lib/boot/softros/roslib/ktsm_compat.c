static char sccsid[] = "@(#)25	1.1  src/rspc/usr/lib/boot/softros/roslib/ktsm_compat.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:05";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: sio_getc
 *		sio_putc
 *		sio_stat
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
/******************************************************************
   This file contains a set of routines to use for compatibility 
   the BOCA runtime libraries. Since we have calls with the same
   function but with different names, this set of compatibility 
   routines will map the boca calls to AIXMON calls.
******************************************************************/

#include  <io_sub.h>
#include  <misc_sub.h>

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Defines and Includes for serial port
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include  <uart.h>

extern int 	iSerialPort;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Defines for Keyboard
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#define DEBUGGER_ACCESS 0x0fe5a
#define CTRL_C          0x0fe5b

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Externals for Keyboard
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

extern int break_detected;
extern int extended_code;
extern int ctrl_key;
extern int alt_key;
extern int shift_key;
extern int num_lock;
extern int ctrl_c;
extern int E0_prefix;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// THE END OF KEYBOARD DEFINES
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#ifdef DEBUG
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// sio_stat() - Check for data waiting on a port
//		Returns: TRUE  if Data Waiting
//			 FALSE if no data waiting
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

sio_stat(int port)
{
				// Current boca libs only support
				// COM1 so port is ignored for now
if((inb(iSerialPort + COM1_LSTAT) & 0x01)==0)
	return(FALSE);
return(TRUE);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// sio_getc() - Get a character from a port
//		Returns: Character 
//		Note: This is a blocking read
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

sio_getc(int port)
{
				// Current boca libs only support
				// COM1 so port is ignored for now
while(!sio_stat(port));
return(rxchar());
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// sio_putc() - Put a character to a port
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

sio_putc(int port, int value)
{
				// Current boca libs only support
				// COM1 so port is ignored for now
value &= 0xFF;			// Make sure it's a byte
txchar(value);			// Send it
}

#endif /* DEBUG */

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// reboot_cmd() -- Dummy command to stop the linker from complaining
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

reboot_cmd()
{
return(0);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Disable_Keyboard - Until real keyboard stuff works
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

Disable_Keyboard(void){
   rtc_value my_timeout;
   my_timeout = get_timeout_value(15, MILLISECONDS);
   while(timeout(my_timeout) == FALSE);
   while( (inb(0x80000064) & 0x2) == 2);
   outb(0x80000064, 0xAD);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Disable_Mouse - Until real mouse stuff works
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

Disable_Mouse(void){
   rtc_value my_timeout;

   while((inb(0x80000064) & 0x02) == 2);
   outb(0x80000064, 0xA7);
   my_timeout = get_timeout_value(100, MILLISECONDS);
   while(timeout(my_timeout) == FALSE);
}
