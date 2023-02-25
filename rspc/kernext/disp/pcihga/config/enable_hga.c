static char sccsid[] = "@(#)77  1.4  src/rspc/kernext/disp/pcihga/config/enable_hga.c, pcihga, rspc41J, 9516A_all 4/13/95 15:18:12";
/* enable_hga.c */
/*
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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

#if 0           /* (Documentation) */
/*----------------------------------------------------------------------------*/

This device driver will support the S3 86C928 GUI Accelerator Chip.  This chip
is used on the Sandalfoot system in the Homestake adapter and on the Polo system
in the Pony adapter.

                ----------------------------------------
               | DIFFERENCE BETWEEN HOMESTAKE AND PONY  |
                ----------------------------------------

The Homestake adapter is a normal PCI adapter which uses the 86C928 and a
Brooktree 485 RAMDAC.  The Pony adapter is a special PCI adapter which is
essenitally the Homestake hardware, but uses a different memory subsystem
may be limited to 1MB, and is only used in the Polo system.

According to Mike Turner,

"The ROS IPL code detects the difference between Pony and Homestake
 by reading a port that has a pullup on Pony and nothing on Homestake:

    if (s3_device == s3_928PCI) {   /* Lets check for the polo version */
      outb(crt_indx,0x55);          /* with the 256 bit SAM.           */
      u = inb(crt_data);            /* Disable DAC Reads -use gpio port*/
      outb(crt_data,u | 0x04);
      if (0 != (inb(dac_general_rd) & 0x10)) /* If bit 4 == 1, then    */
         s3_mem_device = sam_256;   /* set as sam_256                  */
      outb(crt_data,u);             /* Restore to normal access        */
    } /* endif */

 Then, whenever a modeset is performed, the code does the following:

    if (s3_mem_device == sam_256) {
       outb(crt_indx, 0x58);
       outb(crt_data, 0x40 | inb(crt_data));
    } /* endif */"

Obviously, this "architecture" leaves something to be desired.

/*----------------------------------------------------------------------------*/
#endif  /* (Documentation) */

#include "hga_INCLUDES.h"
#include "IO_defs.h"

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

#define VENDOR_ID_REG           0x00
#define COMMAND_REG             0x04
#define REV_ID_REG              0x08
#define BASE_ADDRESS_REG_0      0x10

#define BYTE_REV(n)                                               \
        ((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) + \
        (((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

int enable_hga(ddf, dds)
struct hga_ddf *ddf;
struct fbdds   *dds;
{
    volatile PCI_cnfg this_slot;
    ulong map_size;
    int   rc;

    BUGLPR (dbg_enable, BUGGID, ("ddf = [%X]\n", ddf));

    BUGLPR (dbg_enable, BUGNTA, ("Configuring bus %d, slot %d\n",
                                  dds->bid, dds->slot_number));

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

    if (this_slot.VendorID != HGA_PCI_ID)
    {
        BUGLPR (dbg_enable, BUGNTA,
                ("Adapter[%d] is not a S3 adapter\n", dds->slot_number));
        BUGLPR (dbg_enable, BUGGID,
                ("Vendor id = [%X]\n", this_slot.VendorID));
        return -1;
    }
    /* dds->base_address was set to dds->bus_mem_start in cfghga */
    ddf->base_address = dds->bus_mem_address;

    /* Tell adapter where to go */
    this_slot.BaseAddressRegs[0] = BYTE_REV(ddf->base_address);
    rc = put_pci_cfg_long ( dds->bid, dds->slot_number,
                BASE_ADDRESS_REG_0, &this_slot.BaseAddressRegs[0] );

    BUGLPR (dbg_enable, BUGGID, ("\t60x address set to: [%X] \n", \
         ddf->base_address));
    BUGLPR (dbg_enable, BUGGID, ("\tPCI address set to: [%X] \n", \
         this_slot.BaseAddressRegs[0]));

    /* Attach the largest memory to start with */
    map_size = HGA_MAX_SIZE;

    /* initialize bus attach structure for PCI I/O space */
    ddf->pci_io_map.key   = IO_MEM_MAP;
    ddf->pci_io_map.flags = 0;          /* read/write */
    ddf->pci_io_map.size  = 0x10000;    /* get 64k */
    ddf->pci_io_map.bid   = BID_ALTREG(dds->bid, PCI_IOMEM);
    ddf->pci_io_map.busaddr = (long long)(ulong)(dds->IO_address);

    BUGLPR (dbg_enable, BUGGID, ("\tI/O space BID is: [%X] \n", \
         ddf->pci_io_map.bid));

    /* initialize bus attach structure for PCI memory space */
    ddf->pci_mem_map.key   = IO_MEM_MAP;
    ddf->pci_mem_map.flags = 0;         /* read/write */
    ddf->pci_mem_map.size  = map_size;
    ddf->pci_mem_map.bid   = BID_ALTREG(dds->bid, PCI_BUSMEM);
    ddf->pci_mem_map.busaddr = (long long)(ulong)(dds->bus_mem_address);

    BUGLPR (dbg_enable, BUGGID, ("\tmemory space BID is: [%X] \n", \
         ddf->pci_mem_map.bid));

    BUGLPR (dbg_enable, BUGNTA, ("Enabling adapter access\n"));

    /* Set bits in PCI command register to enable access */
    this_slot.command |= 0x0300;   /* (I/O and Memory) */
    rc |= put_pci_cfg_short ( dds->bid, dds->slot_number,
                COMMAND_REG, &this_slot.command );

    if ( rc != 0)
    {
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to write PCI config space; rc = %d\n", rc));
        return -1;
    }

    ddf->base_address     = dds->bus_mem_address;

    ddf->VendorID = this_slot.VendorID;         /* save for future comparisons */

    BUGLPR (dbg_enable, BUGGID,
           ("\tSlot %d: Vendor ID is [%X], Rev ID is [%X]\n",
             dds->slot_number, this_slot.VendorID, this_slot.revID));

    return 0;
}


