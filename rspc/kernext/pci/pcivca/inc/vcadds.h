/* @(#)75  1.3 src/rspc/kernext/pci/pcivca/inc/vcadds.h, pcivca, rspc41J, 9507C 1/27/95 04:10:46 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vcadds.h -  header file
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

struct	vca_dds {
	ulong	bus_io_addr;		/* base address for port i/o */
	ulong	frame_memory_addr;	/* not used */
	short	bus_number;		/* bus number */
	short	device_intr_level;	/* interrupt level */
	short 	device_intr_priority;	/* interrupt	priority */
	short	slot_number;		/* slot number */
	int	device_idle_time; /* idle timer value during system PM enable */
	int	device_stanby; 	     /* idle timer value during system stanby */
	int	pm_ccd_id;		/* PM CCD device ID */
	char	logical_name[16];	/* device logical name */
};

