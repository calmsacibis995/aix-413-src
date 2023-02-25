static char sccsid[] = "@(#)53	1.1  src/bos/diag/tu/fddi/tu002.c, tu_fddi, bos411, 9428A410j 7/9/91 12:43:16";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: norm_mode
 *		tu002
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************

Function(s) Test Unit 002 - Normal Mode

Module Name :  tu002.c
SCCS ID     :  1.12

Current Date:  5/23/90, 11:17:52
Newest Delta:  5/1/90, 09:50:34

This test unit places the adapter in normal operational mode. This is 
accomplished by writing the value of 0 (zero) to bit 4 in POS register 2. 
The register will then be read to insure the write occured successfully.

POS registers will be accessed via the machine device driver.
*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif



/*****************************************************************************

norm_mode
This routine will read POS 2, clear bit 4 of POS 2, and reread POS 2 to 
confirm this occured correctly. 

*****************************************************************************/

int norm_mode(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int		mode;
	unsigned char	p0;
	unsigned long	status;
	struct htx_data *htx_sp;

	extern int pos_wr();
	extern unsigned char pos_rd();

	/*
	 * Set up a pointer to HTX data structure to increment
	 * counters in case TU was invoked by hardware exerciser.
	 */
	 htx_sp = tucb_ptr->fddi_s.htx_sp;

	/*
	 * First, read POS register 2.
	 */

	p0 = pos_rd(2, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Initial POS2 read failed. POS2 = %x\n", p0);
		detrace(0,"POS2 read returned status = %x\n", status);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_RD_ERR));
	   }

	/*
	 * Set the Download/Diagnostic bit to 1 and clear out
	 * the Reset bit.
	 */

	p0 = p0 & MASK_POS2_CLEAR_D_D;
	p0 = p0 & MASK_POS2_CLEAR_RESET;
	if (pos_wr( 2, &p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Resetting of Download/Diagnostic bit failed.");
		detrace(0,"POS2 = %x\n", p0);
		detrace(0,"POS2 write returned status = %x\n", status);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_WR_ERR));
	   }
	
	/*
	 * Check the new value by rereading POS register 2.
	 */

	p0 = pos_rd(2, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"2nd read of POS2 failed. POS2 = %x\n", p0);
		detrace(0,"POS2 read returned status = %x\n", status);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_RD_ERR));
	   }

	/*
	 * Check to see if Download/Diagnostic bit in POS2 is cleared.
	 */
	if (p0 & MASK_POS2_SET_D_D)
	   {
#ifdef debugg
		detrace(0,"D/D bit not cleared. POS2 = %x\n", p0);
#endif
		if (htx_sp != NULL)
		   (htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_CMP_ERR));
	   }

	return(0);
   }



/*****************************************************************************

tu002

*****************************************************************************/

int tu002 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;

	rc = norm_mode(fdes, tucb_ptr);
	return(rc);
   }
