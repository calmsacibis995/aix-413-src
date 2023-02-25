static char sccsid[] = "@(#)36	1.5  src/rspc/kernext/disp/common/rw_pci_cfg.c, rspcdisp, rspc411, GOLD410 5/10/94 10:58:16";
/*
 *   COMPONENT_NAME: rspcdisp
 *
 *   FUNCTIONS: rw_pci_cfg_regs
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef DEBUG
#include <stdio.h>
#endif

#include <sys/types.h>
#include <sys/ioacc.h>
#include <sys/mdio.h>                /* Machine dd        */

/*----------------------------------------------
|
|	FUNCTION DESCRIPTION
|
|   This function is ONLY for reading or writing 
|   the PCI Standard Configuration Space.  
|   All accesses are relative to the beginning of the
|   configuration space for the bus/slot.
|
|-----------------------------------------------*/
int
rw_pci_cfg_regs(
	ulong			slot,       /* PCI Slot             */
	ulong			addr,       /* offset from base     */
	uchar 	 	       *p_buf,      /* buffer to read/write */
	ulong 			bus_bid,    /* Bus BID              */
	int 			size,       /* size of access       */
	int 			read_write  /* read or write        */

	       )
{
	int		rc;
	struct mdio	mdd;

#ifdef DEBUG
	printf (
		"rw_pci_cfg_regs: bus_bid =%x, slot =%x, addr =%x, pbuf =%x, size =%x, r/w =%x\n",
		bus_bid, slot, addr, p_buf, size, read_write
		);
#endif

	mdd.md_addr		= addr;
	mdd.md_data		= (char *) p_buf;
	mdd.md_sla	 	= slot; 

	switch ( size )
	{
		case 4:
			mdd.md_incr = MV_WORD; 
			mdd.md_size = size/4;
			break;
		case 2:
			mdd.md_incr = MV_SHORT; 
			mdd.md_size = size/2;
			break;
		case 1:
		default:
			mdd.md_incr = MV_BYTE; 
			mdd.md_size = size;
			break;
	}

	rc = pci_cfgrw(bus_bid, & mdd, read_write);	/* move data */

#ifdef DEBUG
	if ( rc != 0 )
		printf ("rw_pci_cfg_regs: pci_cfgrw() failed. rc = %d\n",rc);
#endif

	return (rc);
}
