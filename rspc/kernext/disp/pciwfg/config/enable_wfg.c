static char sccsid[] = "@(#)82  1.5 src/rspc/kernext/disp/pciwfg/config/enable_wfg.c, pciwfg, rspc41J, 9512A_all 3/19/95 23:06:13";
/* enable_wfg.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: enable_wfg
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "wfg_INCLUDES.h"
#include "IO_defs.h"

/* ---------------------------------------- *
 *         PCI Bus Defines                  *
 * ---------------------------------------- */
typedef struct {
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
#define BASE_ADDRESS_REG_0      0x14
#define BASE_ADDRESS_REG_1      0x10

#define BYTE_REV(n)                                               \
        ((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) + \
        (((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

BUGXDEF (dbg_enable);
/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : enable_wfg()                                         *
 *                                                                         *
 * DESCRIPTIVE NAME : Western Digital PCI Graphic Adapter                  *
 *                    device enable routine                                *
 *                                                                         *
 * FUNCTIONS : Sets up the "pos" register for this adapter                 *
 *               1. Gets the PCI register data                             *
 *                           (vendor_id, command, revision_id)             *
 *               2. Check the data                                         *
 *               3. Put the bus memory base address into PCI configuration *
 *                  register                                               *
 *               4. Initialize bus attach structure for PCI I/O space      *
 *               5. Initialize bus attach structure for PCI memory space   *
 *               6. Set bits in PCI command register to enable access      *
 *                                                                         *
 * INPUTS    : Pointer to ddf                                              *
 *             Pointer to dds                                              *
 *                                                                         *
 * OUTPUTS   : -1 if can't find WFG, otherwise 0                           *
 * CALLED BY : define() and pm_restore_register()                          *
 * CALLS     : None                                                        *
 *                                                                         *
 **************************************************************************/

int 
enable_wfg(ddf, dds)
    struct wfg_ddf *ddf;
    struct fbdds   *dds;
{
    volatile PCI_cnfg this_slot;
    ulong map_size;
    int   rc;

    BUGLPR (dbg_enable, BUGGID, ("ddf = [%X]\n", ddf));
    BUGLPR (dbg_enable, BUGNTA, ("Configuring bus %d, slot %d\n",
                                  dds->bid, dds->slot_number));

    /* ---------------------------------- *
     *   Get the interesting values       *
     * ---------------------------------- */
    rc = get_pci_cfg_long ( dds->bid, dds->slot_number,
                VENDOR_ID_REG, &this_slot.VendorID );
    rc |= get_pci_cfg_short ( dds->bid, dds->slot_number,
                COMMAND_REG, &this_slot.command );
    rc |= get_pci_cfg_long ( dds->bid, dds->slot_number,
                REV_ID_REG, &this_slot.revID );  /* (should be 0x01000003) */

    if ( rc != 0)
    {
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to read PCI config space; rc = %d\n", rc));
        return -1;
    }

    BUGLPR (dbg_enable, BUGNTA,
                ("Vendor id = [%X]\n", this_slot.VendorID));

    if (this_slot.VendorID != WFG_PCI_ID)
    {
        BUGLPR (dbg_enable, BUGGID,
                ("Adapter[%d] is not a Western Digital adapter\n", 
			           dds->slot_number));
        BUGLPR (dbg_enable, BUGNTA,
                ("Vendor id = [%X]\n", this_slot.VendorID));
        return -1;
    }

    /* ---------------------------------------------------------------- *
     *    ddf->base_address was set to dds->bus_mem_start in cfgwfg     *
     * ---------------------------------------------------------------- */
    ddf->base_address = dds->bus_mem_address;

    /* ------------------------------- *
     *    Tell adapter where to go     *
     * ------------------------------- */

    /* ---------------------------------------------------------------- *
     *       Bit 23-4 of the memory address register should be 0.       *
     *               (See the Woodfield H/W spec.)                      *
     * ---------------------------------------------------------------- */
    this_slot.BaseAddressRegs[0] = BYTE_REV((ddf->base_address & 0xFF000000)); 

    rc = put_pci_cfg_long ( dds->bid, dds->slot_number,
                BASE_ADDRESS_REG_0, &this_slot.BaseAddressRegs[0] );

    BUGLPR (dbg_enable, BUGGID, ("\t60x address set to: [%X] \n", \
         ddf->base_address));
    BUGLPR (dbg_enable, BUGGID, ("\tPCI address set to: [%X] \n", \
         this_slot.BaseAddressRegs[0]));

    /* ---------------------------------------------- *
     *    Attach the largest memory to start with     *
     * ---------------------------------------------- */
    map_size = WFG_MAX_SIZE;

    /* ----------------------------------------------------- *
     *   Initialize bus attach structure for PCI I/O space   *
     * ----------------------------------------------------- */
    ddf->pci_io_map.key     = IO_MEM_MAP;
    ddf->pci_io_map.flags   = 0;              /* Read/Write */
    ddf->pci_io_map.size    = 0x10000;        /* Get 64k    */
    ddf->pci_io_map.bid     = BID_ALTREG( dds->bid, PCI_IOMEM );
    ddf->pci_io_map.busaddr = (long long)(ulong)(dds->IO_address);

    BUGLPR (dbg_enable, BUGGID, ("\tI/O space BID is: [%X] \n", \
         ddf->pci_io_map.bid));

    /* --------------------------------------------------------- *
     *   Initialize bus attach structure for PCI memory space    *
     * --------------------------------------------------------- */
    ddf->pci_mem_map.key     = IO_MEM_MAP;
    ddf->pci_mem_map.flags   = 0;             /* Read/Write */
    ddf->pci_mem_map.size    = map_size;
    ddf->pci_mem_map.bid     = BID_ALTREG( dds->bid, PCI_BUSMEM );
    ddf->pci_mem_map.busaddr = (long long)(dds->bus_mem_address);

    BUGLPR (dbg_enable, BUGGID, ("\tmemory space BID is: [%X] \n", \
         ddf->pci_mem_map.bid));
    BUGLPR (dbg_enable, BUGNTA, ("Enabling adapter access\n"));

    /* ------------------------------------------------------- *
     *    Set bits in PCI command register to enable access    *
     * ------------------------------------------------------- */
    this_slot.command |= 0x0300;              /* (I/O and Memory) */
    rc |= put_pci_cfg_short ( dds->bid, dds->slot_number,
                COMMAND_REG, &this_slot.command );

    if ( rc != 0)
    {
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to write PCI config space; rc = %d\n", rc));
        return -1;
    }

    /* --------------------------------- *
     *    Save for future comparisons    *
     * --------------------------------- */
    ddf->VendorID = this_slot.VendorID;		

    BUGLPR (dbg_enable, BUGGID,
           ("\tSlot %d: Vendor ID is [%X], Rev ID is [%X]\n",
             dds->slot_number, this_slot.VendorID, this_slot.revID));

    return 0;
}
