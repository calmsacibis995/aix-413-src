static char sccsid[] = "@(#)93  1.3  src/rspc/kernext/disp/pcifga/config/enable_fga.c, pcifga, rspc411, GOLD410 5/27/94 15:39:56";
/* enable_fga.c */
/*
 *
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: enable the hardware
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

/*----------------------------------------------------------------------------*/
/*      Needs:  pointer to ddf, dds                                           */
/*                                                                            */
/*      Returns: -1 if can't find FGA, otherwise 0                            */
/*----------------------------------------------------------------------------*/

#include "fga_INCLUDES.h"

BUGXDEF (dbg_enable);

/***************************** PCI Bus Defines ********************************/
typedef struct {        /* PCI Config space */
       unsigned long  VendorID;
       unsigned short command;
       unsigned short status;
       unsigned long  revID;
       unsigned long  CacheLineSize;
       unsigned long  BaseAddressRegs[6];
} PCI_cnfg;

#define VENDOR_ID_REG		0x00
#define COMMAND_REG		0x04
#define REV_ID_REG		0x08
#define BASE_ADDRESS_REG_0	0x10

#define BYTE_REV(n)                                               \
        ((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) + \
        (((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

int enable_fga(ddf, dds)
struct fga_ddf *ddf;
struct fbdds   *dds;
{
    volatile PCI_cnfg this_slot;
    ulong map_size;
    int   rc;

    BUGLPR (dbg_enable, BUGNTA, ("Configuring bus %d, slot %d\n",
                                  dds->bid, dds->slot_number));

    /* get the interesting values */
    rc = get_pci_cfg_long ( dds->bid, dds->slot_number, 
		VENDOR_ID_REG, &this_slot.VendorID );
    rc |= get_pci_cfg_short ( dds->bid, dds->slot_number, 
		COMMAND_REG, &this_slot.command );
    rc |= get_pci_cfg_long ( dds->bid, dds->slot_number, 
		REV_ID_REG, &this_slot.revID );

    if ( rc != 0)
    { 
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to read PCI config space; rc = %d\n", rc));
        return -1;
    }
    if (this_slot.VendorID != FGA_PCI_ID)
    {
        BUGLPR (dbg_enable, BUGNFO,
                ("Adapter[%d] is not a Weitek adapter\n", dds->slot_number));
        BUGLPR (dbg_enable, BUGNFO,
                ("Vendor id = [%X]\n", this_slot.VendorID));
        return -1;
    }

    /* Check for the old Altera/Fairway card */
    if ((this_slot.VendorID == FGA_PCI_ID) && ((this_slot.revID & 0xff000000) == 0))
    { /* rev level 0 is the old Altera/Fairway */
        BUGLPR (dbg_enable, BUGNFO,
                ("The adapter in slot %d is an obsolete adapter\n",
                        dds->slot_number));

        /* The old Altera adapter is fixed at location 0x01000000. */
        /*  if the address in the dds doesn't match, you're out of luck. */
        if( dds->bus_mem_address != 0x01000000 )
        {
                /* This message will disappear when the old cards disappear */
                printf ("enable_fga: Unable to configure the obsolete adapter located in slot %d.\n",
			dds->slot_number);
                return -1;
        }
    }

    /* dds->bus_mem_address was set to dds->bus_mem_start in cfgfga */
    ddf->base_address = dds->bus_mem_address;
    
    /* Tell adapter where to go */
    this_slot.BaseAddressRegs[0] = BYTE_REV(ddf->base_address);
    rc = put_pci_cfg_long ( dds->bid, dds->slot_number, 
		BASE_ADDRESS_REG_0, &this_slot.BaseAddressRegs[0] );

    BUGLPR (dbg_enable, BUGGID, ("\t60x address set to: [%X] \n", \
         ddf->base_address));
    BUGLPR (dbg_enable, BUGGID, ("\tPCI address set to: [%X] \n", \
         this_slot.BaseAddressRegs[0]));

    /* Figure out how much memory to attach */
    map_size = FGA_MAP_SIZE;

    /* initialize bus attach structure for PCI memory space for PCI_MEM_ATT() */
    ddf->pci_mem_map.key   = IO_MEM_MAP;
    ddf->pci_mem_map.flags = 0;         /* read/write */
    ddf->pci_mem_map.size  = map_size;
    ddf->pci_mem_map.bid   = BID_ALTREG(dds->bid, PCI_BUSMEM);
    ddf->pci_mem_map.busaddr = (long long)(dds->bus_mem_address);

    BUGLPR (dbg_enable, BUGNTA, ("Enabling adapter access\n"));

    /* Set bit in PCI command register to enable access */
    this_slot.command |= 0x0200; /* memory access */
    rc |= put_pci_cfg_short ( dds->bid, dds->slot_number, 
		COMMAND_REG, &this_slot.command );

    if ( rc != 0)
    { 
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to write PCI config space; rc = %d\n", rc));
        return -1;
    }

    ddf->VendorID = this_slot.VendorID;  /* save for future comparisons */

    BUGLPR (dbg_enable, BUGGID,
           ("\tSlot %d: Vendor ID is [%X], Rev ID is [%X]\n",
             dds->slot_number, this_slot.VendorID, this_slot.revID));

    return 0;
}
