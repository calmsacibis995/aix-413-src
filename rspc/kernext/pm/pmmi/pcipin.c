static char sccsid[] = "@(#)87  1.1  src/rspc/kernext/pm/pmmi/pcipin.c, pwrsysx, rspc41J 6/7/95 23:39:54";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: disable_pci_devices
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/pm.h>
#include "pmmi.h"

pcidev_t	pci_devices;

/*
 * NAME:  disable_pci_devices
 *
 * FUNCTION:  disables all PCI devices.
 *
 * DATA STRUCTURES:
 *      none
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 */
void
disable_pci_devices()
{
	MACH_DD_IO	mdd;
	ushort		control;
	int		i;

	/* build mdd record */
	mdd.md_size = 1;
	mdd.md_incr = MV_SHORT;
	mdd.md_addr = 4;
	mdd.md_data = (uchar*)(&control);

	for(i = 0 ; i < pci_devices.num; i++){
		mdd.md_sla = pci_devices.devfunc[i];
		if(pci_cfgrw(pci_devices.bid[i], &mdd, 0) == 0){
			/* remove lower 3 bits of PCI command register */
			control = control & 0xf8ff;
			pci_cfgrw(pci_devices.bid[i], &mdd, 1);
			/* if result != 0, error - device not disabled */
			/* do not report error to continue working */
			/* on all devices */
		}
	}

	return;
}
