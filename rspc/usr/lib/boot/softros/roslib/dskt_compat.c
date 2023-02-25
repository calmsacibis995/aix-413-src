static char sccsid[] = "@(#)21	1.1  src/rspc/usr/lib/boot/softros/roslib/dskt_compat.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:58";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: flop_rdwr
 *		flop_reset
 *		flop_turn_off_motor
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
#ifdef DEBUG	/* only include this module for DEBUG */

/******************************************************************
   This file contains a set of routines to use for compatibility
   the BOCA runtime libraries. Since we have calls with the same
   function but with different names, this set of compatibility
   routines will map the boca calls to AIXMON calls.
******************************************************************/

#include	<diskette.h>

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Declare Global stuff
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

extern DSKT_CB dskt_cb;

/****************************************************************************
*
*   Function : flop_rdwr
*
*   Description : read / write sectors from/to  the floppy drive
*
*   Input       : drive   - drive number ( 0 - 3 ) 
*                 sect    - starting logical sector to be read / written
*               nosect    - number of sectors, including the starting sector
*                           to be read / written
*               buffer    - pointer to the memory are where the data should
*                           be put into/ read from . It should be large enough
*                           to hold the data
*                   rw    - read = 0;   write = 1
*
*    Returns    : > 0     - number of sectors read / written
*                   0     - read / write  failed
*
****************************************************************************/


unsigned short flop_rdwr ( drive,  sect,  nosect, buffer , rw)
unsigned char drive ;
unsigned short sect ;
unsigned short nosect ;
unsigned char * buffer ;
unsigned char rw ;
{
int	rc;

if(rw)					// Write
	rc =  diskette_write_RBA(sect, nosect, drive, buffer);
else
	rc =  diskette_read_RBA(sect, nosect, drive, buffer);

if(rc == OK)
	return(nosect);
else
	return(0);

}

/*------------------------------------------------------------------
*   Function    : flop_reset
*
*   Description : Reset the floppy controller
*   Input       : drive - drive nubber ( 0 - 3 ) 
*   Returns     : 1 - Reset is ok
*                 0 - Reset Failed
*
------------------------------------------------------------------*/


unsigned char   flop_reset( unsigned char drive)
{
if(diskette_init(drive) == 0)
	return(1);
return(0);
}

/*------------------------------------------------------------------
*   Function    : flop_turn_off_motor
*
*   Description : Turn off the specified driver's motor.
*                  (This also turns off the drive's LED)
*   Input       : drive - drive number ( 0 - 3 ) 
*   Returns     : 1 - Turn off ok
*                 0 - Turn off failed
*
------------------------------------------------------------------*/

unsigned char   flop_turn_off_motor( unsigned char drive)
{
CB_PTR  cb_ptr;

cb_ptr = &dskt_cb;

dskt_stop_motor(cb_ptr);
return(1);			// No return so always return 1
}
#endif /* DEBUG */
