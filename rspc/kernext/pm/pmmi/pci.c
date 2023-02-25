static char sccsid[] = "@(#)86  1.1  src/rspc/kernext/pm/pmmi/pci.c, pwrsysx, rspc41J 6/7/95 23:39:32";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: busnum_to_bid, save_pcidev
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
#include <sys/ioacc.h>
#include <sys/residual.h>
#include <sys/pm.h>
#include "iplcb_init.h"
#include "pmmi.h"

extern pcidev_t		pci_devices;

/*
 * NAME:  busnum_to_bid
 *
 * FUNCTION:  converts PCI bus number to BID of the PCI bus.
 *
 * DATA STRUCTURES:
 *      int busnum     PCI bus number
 *
 * RETURN VALUE DESCRIPTION:
 *      -1    : error
 *      not -1: BID of the bus
 */
int
busnum_to_bid(int busnum)
{
	IPL_DIRECTORY	iplcb_dir;
	BUC_DATA	buc[MAX_BUC_SLOTS];
	int		i;

	/* get BUC table */
	if(iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB)){
		return -1;
	}
	if(iplcb_get(buc, iplcb_dir.buc_info_offset, iplcb_dir.buc_info_size,
								MIOIPLCB)){
		return -1;
	}

	for(i = 0; i < buc[0].num_of_structs; i++){
		if(IO_PCI == BID_TYPE(buc[i].buid_data[0].buid_value) &&
						busnum == buc[i].bscr_value){
			return buc[i].buid_data[0].buid_value;
		}
	}

	return -1;
}


/*
 * NAME:  save_pcidev
 *
 * FUNCTION:  saves information of PCI devices which ROS doesn't disable.
 *
 * DATA STRUCTURES:
 *      none
 *
 * RETURN VALUE DESCRIPTION:
 *      -1: error
 *      0 : success
 */
save_pcidev()
{
	RESIDUAL	residual;
	int		i, bid;

	/* get residual data */
	if(get_residual(&residual) == -1){
                return -1;
        }

	pci_devices.num = 0;
	/* check all device data. */
	for(i = 0; i < residual.ActualNumDevices; i++){
		/* PCI_DEVICE == 0x04 */
		if(residual.Devices[i].DeviceId.BusId == 0x04 &&
			(residual.Devices[i].DeviceId.Flags & Enabled) != 0){
			bid = busnum_to_bid(residual.Devices[i].BusAccess.
						PCIAccess.BusNumber);
			if(bid != -1){
				pci_devices.devfunc[pci_devices.num] =
					residual.Devices[i].BusAccess.
						PCIAccess.DevFuncNumber;
				pci_devices.bid[pci_devices.num] = bid;
				(pci_devices.num)++;
			}
		}
	}
}
