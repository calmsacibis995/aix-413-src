static char sccsid[] = "@(#)79  1.5  src/rspc/kernext/disp/pcigga/config/enable_gga.c, pcigga, rspc41J, 9513A_all 3/22/95 11:03:10";
/* enable_gga.c */
/*
 *
 *   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: enable_gga
 *		Prog_ICD
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
/*      Returns: -1 if can't find GGA, otherwise 0                            */
/*----------------------------------------------------------------------------*/

#include "gga_INCLUDES.h"

GS_MODULE(gga_enable);
GS_MODULE(gga_Prog_ICD);

void Prog_ICD ();

/***************************** PCI Bus Defines ********************************/

typedef struct {        /* PCI Config space */
       uint  VendorID;                          /* 0x00                 */
       ushort  Command;                         /* 0x04                 */
       ushort  Status;                          /* 0x06                 */
       uint  RevID;                             /* 0x08                 */
       uint  CacheLineSize;                     /* 0x0c                 */
       uint  BaseAddressRegs[6];                /* 0x10                 */
       long  reserved1;                         /* 0x28                 */
       long  reserved2;                         /* 0x2c                 */
       long  ExpanEpromAddr;                    /* 0x30                 */
       long  reserved3;                         /* 0x34                 */
       long  reserved4;                         /* 0x38                 */
       uint  InterruptLine;                     /* 0x3c                 */
       uchar Config64;                          /* 0x40                 */
       uchar Config65;                          /* 0x41                 */
       uchar Config66;                          /* 0x42                 */
} PCI_cnfg;

#define VENDOR_ID_REG           0x00
#define COMMAND_REG             0x04
#define REV_ID_REG              0x08
#define BASE_ADDRESS_REG_0      0x10
#define CONFIG_65               0x41
#define CONFIG_66               0x42

#define BYTE_REV(n)                                               \
        ((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) + \
        (((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

BUGXDEF (dbg_enable);


int enable_gga(ddf, dds)
struct gga_ddf *ddf;
struct gbdds   *dds;
{
    volatile PCI_cnfg this_slot;
    ulong map_size;
    ulong command_buf;
    int   rc;

    BUGLPR (dbg_enable, BUGNTA, ("Configuring bus %d, slot %d\n",
                                  dds->bid, dds->slot_number));

    GGA_ENTER_TRC(ddf,gga_enable,2,GGA_ENABLE,dds->bid,dds->slot_number,0,0,0);

    ddf->IO_in_progress += 1;     /* set flag to indicate IO happening */

    /* get the interesting values */
    rc = get_pci_cfg_long ( dds->bid, dds->slot_number, 
		VENDOR_ID_REG, &this_slot.VendorID );

        BUGLPR (dbg_enable, BUGNFO,
                ("VENDOR_ID_REG = %x\n", this_slot.VendorID));

    rc |= get_pci_cfg_short ( dds->bid, dds->slot_number, 
		COMMAND_REG, &this_slot.Command );

        BUGLPR (dbg_enable, BUGNFO,
                ("COMMAND_REG = %x\n", this_slot.Command));

    command_buf = this_slot.Command;

    rc |= get_pci_cfg_long ( dds->bid, dds->slot_number, 
		REV_ID_REG, &this_slot.RevID );

        BUGLPR (dbg_enable, BUGNFO,
                ("REV_ID_REG = %x\n", this_slot.RevID));

    if ( rc != 0)
    { 
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to read PCI config space; rc = %d\n", rc));

	GGA_PARM_TRC0(ddf,gga_enable,2,GGA_ENABLE,GGA_FAILED_RD_PCI_CFG_REGS);

	ddf->IO_in_progress -= 1;     /* reset flag */

        return -1;
    }
    if (this_slot.VendorID != GGA_PCI_ID)
    {
        BUGLPR (dbg_enable, BUGNFO,
                ("Adapter[%d] is not a Weitek adapter\n", dds->slot_number));
        BUGLPR (dbg_enable, BUGNFO,
                ("Vendor id = [%X]\n", this_slot.VendorID));

	GGA_PARM_TRC0(ddf,gga_enable,2,GGA_ENABLE,GGA_UNKNOWN_VENDOR_ID);

	ddf->IO_in_progress -= 1;     /* reset flag */

        return -1;
    }

    /* dds->bus_mem_address was set to dds->bus_mem_start in cfggga */
    ddf->base_address = dds->bus_mem_address;

    /* Figure out how much memory to attach */
    map_size = GGA_MAP_SIZE;

    /* initialize bus attach structure for PCI memory space for PCI_MEM_ATT() */
    ddf->pci_mem_map.key   = IO_MEM_MAP;
    ddf->pci_mem_map.flags = 0;         /* read/write */
    ddf->pci_mem_map.size  = map_size;
    ddf->pci_mem_map.bid   = BID_ALTREG(dds->bid, PCI_BUSMEM);
    ddf->pci_mem_map.busaddr = (long long)(dds->bus_mem_address);

    ddf->VendorID = this_slot.VendorID;  /* save for future comparisons */

    /* Tell adapter where to go */
   this_slot.BaseAddressRegs[0] = BYTE_REV(ddf->base_address);

    rc = put_pci_cfg_long ( dds->bid, dds->slot_number, 
		BASE_ADDRESS_REG_0, &this_slot.BaseAddressRegs[0] );

    BUGLPR (dbg_enable, BUGNFO, ("60x address set to: [%X] \n", \
         ddf->base_address));
    BUGLPR (dbg_enable, BUGNFO, ("PCI address set to: [%X] \n", \
         this_slot.BaseAddressRegs[0]));

    BUGLPR (dbg_enable, BUGNTA, ("Enabling adapter access\n"));

    /* Set bit in PCI command register to enable access */
    this_slot.Command = 0x0200; /* memory access */
    rc |= put_pci_cfg_short ( dds->bid, dds->slot_number, 
		COMMAND_REG, &this_slot.Command );

    this_slot.Config65 = 0x00;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number, 
		CONFIG_65, &this_slot.Config65 );

    rc |= get_pci_cfg_long ( dds->bid, dds->slot_number, 
		BASE_ADDRESS_REG_0, &this_slot.BaseAddressRegs[0] );

        BUGLPR (dbg_enable, BUGNFO,
                ("BASE_ADDRESS_REG_0 = %x\n", this_slot.BaseAddressRegs[0]));

    rc |= get_pci_cfg_short ( dds->bid, dds->slot_number, 
		COMMAND_REG, &this_slot.Command );

        BUGLPR (dbg_enable, BUGNFO,
                ("COMMAND_REG = %x\n", this_slot.Command));

    rc |= get_pci_cfg_byte ( dds->bid, dds->slot_number, 
		CONFIG_65, &this_slot.Config65 );

        BUGLPR (dbg_enable, BUGNFO,
                ("CONFIG_65 = %x\n", this_slot.Config65));

    if ( rc != 0)
    { 
        BUGLPR (dbg_enable, BUGNFO,
                ("Unable to write PCI config space; rc = %d\n", rc));

	GGA_PARM_TRC0(ddf,gga_enable,2,GGA_ENABLE,GGA_FAILED_WR_PCI_CFG_REGS)

	ddf->IO_in_progress -= 1;     /* reset flag */

        return -1;
    }

    BUGLPR (dbg_enable, BUGNFO,
           ("Slot %d: Vendor ID is [%X], Rev ID is [%X]\n",
             dds->slot_number, this_slot.VendorID, this_slot.RevID));

    EIEIO;

    if ( ( command_buf & 0xFF00 ) == 0x8200 )
    {
        ddf->IO_in_progress -= 1;     /* reset flag */
	return (0);
    }

    Prog_ICD( ddf, dds, 0x0001ac3d );
    Prog_ICD( ddf, dds, 0x0021ac3d );
    Prog_ICD( ddf, dds, 0x0041ac3d );
    Prog_ICD( ddf, dds, 0x00779c9f );

    ddf->IO_in_progress -= 1;     /* reset flag */

    GGA_EXIT_TRC0(ddf,gga_enable,2,GGA_ENABLE);

    return 0;
}



/*----------------------------------------------------------------------------*/
/*               ICD2061A CLOCK INITIALIZATION ROUTINE                        */
/*----------------------------------------------------------------------------*/
void
Prog_ICD( ddf, dds, serial_data )
	struct gga_ddf *ddf;
        struct gbdds   *dds;
        unsigned long serial_data;
{
    volatile PCI_cnfg this_slot;
    int i, rc = 0;
    unsigned char start_val, Temp;

    GGA_ENTER_TRC1(ddf,gga_Prog_ICD,2,GGA_PROG_ICD,serial_data);

    if (serial_data == 0)
    {                               /* reset the control chip */
        this_slot.Config66 = 0x00;  /* default freq. for 640 x 480 */
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );

        GGA_EXIT_TRC1(ddf,gga_Prog_ICD,2,GGA_PROG_ICD,0);

        return;
    }

    /* Preserve the current value of GGA_ADCNTL */
    rc |= get_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );

    start_val = this_slot.Config66 & (uchar) ~(GGA_ser_clock_on | GGA_ser_data_on);

    /* Unlock the ICD chip */
    this_slot.Config66  = start_val | (uchar) GGA_ser_data_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
    EIEIO;

    for( i=0; i<5; i++)  /* do 5 times */
    {
        this_slot.Config66 = start_val | (uchar) GGA_ser_data_on;
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

        this_slot.Config66 = start_val | (uchar) GGA_ser_clock_on | (uchar) GGA_ser_data_on;
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;
    }

    /* Send Start Bit */
    this_slot.Config66   = start_val;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66   = start_val | (uchar) GGA_ser_clock_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66   = start_val;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66   = start_val | (uchar) GGA_ser_clock_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    /* Send Data Bits */
    for( i=0; i<24; i++) /*  do 24 times */
    {
	Temp = (uchar) serial_data;
        this_slot.Config66 = start_val | (uchar) GGA_ser_clock_on | ((~(Temp<<3)) & (uchar) GGA_ser_data_on);
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

        this_slot.Config66 = start_val | ((~(Temp<<3)) & (uchar) GGA_ser_data_on);
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

        this_slot.Config66 = start_val | ((Temp<<3) & (uchar) GGA_ser_data_on);
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

        this_slot.Config66 = start_val | (uchar) GGA_ser_clock_on | ((Temp<<3) & (uchar) GGA_ser_data_on);
        rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

        serial_data >>=1;
    }

    /* Send Stop Bit */
    this_slot.Config66 = start_val | (uchar) GGA_ser_clock_on | (uchar) GGA_ser_data_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66 = start_val | (uchar) GGA_ser_data_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66 = start_val | (uchar) GGA_ser_clock_on | (uchar) GGA_ser_data_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66 = start_val | (uchar) GGA_ser_clock_on | (uchar) GGA_ser_data_on;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
	EIEIO;

    this_slot.Config66 = start_val;
    rc |= put_pci_cfg_byte ( dds->bid, dds->slot_number,
                        CONFIG_66, &this_slot.Config66 );
    EIEIO;

    GGA_EXIT_TRC1(ddf,gga_Prog_ICD,2,GGA_PROG_ICD,rc);

    return;  /* end of Prog_ICD () */
}

