static char sccsid[] = "@(#)14  1.8  src/rspc/usr/lib/methods/cfgbus_pci/cfgbus_pci.c, rspccfg, rspc41J, 9523C_all 6/7/95 16:03:12";
/*
 *   COMPONENT_NAME: rspccfg     define_children for pci bus
 *
 *   FUNCTIONS: 
 *		define_children
 *		pciquery
 *		bus_depend 
 *		bus_depend2 
 *		mf_query 
 *		bus_dependent_cfg 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/ioacc.h>
#include <sys/iplcb.h>
#include <cfgresid.h>

#include "bcm.h"
#include "cfgdebug.h"


extern struct CuDv cudv;
extern struct PdDv pddv;

CARD_INFO       card_table[PCI_CARD_TABLE_SIZE];
static ulong num_of_devices;		/* Number of devices in resid data */
static CFG_DEVICE *devp;		/* Resid data device table */
static CFG_pci_descriptor_t  *pci;	/* PCI bridge descriptor */

/* residual data defines */
#define PCI_reserved			0	/* BaseType */
#define Memory_Controller		5	/* BaseType */
#define General_Power_Management        0       /* interface type */

/*
 * NAME: define_children
 *
 * FUNCTION: this routine detects and defines devices attached
 * 	     to the pci bus.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
define_children(bus_devname, phase, busname) 
char *bus_devname;
int  phase;
char *busname;
{
	int 	rc;
	char	conf_list[2000], not_res_list[2000];
	int	fd;		/* bus file descriptor */
	char	loc[16];	/* Device location code */
	int	i;		/* Loop variable for slot table */
	int	j;		/* Loop variable for resid data device table */
	int	multi;		/* MultiFunction indicator */
	int	func;		/* Function number */
	int	devfunc;	/* residual device function number */
	CFG_DEVICE *p;		/* Pointer to a device's resid data */
	int     pci_resid_offset; /* offset of intg pci resid entry */

	DEBUG_0("In define_children\n");

        bzero(card_table, (sizeof(CARD_INFO) * PCI_CARD_TABLE_SIZE));

	/* Open bus device */
        if ((fd = open(bus_devname, O_RDWR)) < 0) {
                return(E_DEVACCESS);
        }


	/* Find children of PCI bus using PCI Bridge descriptor.
	 * This depends on resid device table and PCI bridge descriptor
	 * already obtained by bus_depend().
	 */
	/* Loop through slot table in PCI bridge descriptor */
	for (i=0; i<pci->numslots; i++) {

		/* Save DevFunc number in local variable */
		devfunc = pci->slotdata[i].devfunc;

		/* If the device is integrated */
		if (pci->slotdata[i].slotnumber == 0) {

		    /* Find the integrated device in resid table */
		    pci_resid_offset = -1;	/* initialize offset value */
		    for (j=0; j<num_of_devices; j++) {

			/* Looking for an integrated PCI device with this bus's
			 * PCI bus number and a DevFuncNumber
			 * matching that in the PCI bridge desc
			 */
			p = devp + j;

			if ((p->deviceid.busid == PCI_DEVICE) &&
			    (p->deviceid.flags & INTEGRATED)) {

			    /* Count integrated devices for location code 
			     * Do not count if BaseType is:
			     *  - Reserved
			     *  - Memory Controllers
			     *  - System Peripherals (except Power Mgmt)
			     */
			    /* If its Reserved, Memory Controller, or Bridge */
			    /* Controller then go to next device */
			    if ( (p->deviceid.basetype == PCI_reserved) ||
		     		  (p->deviceid.basetype == Memory_Controller)) {
						continue;
			    }

			    /* If System Peripheral but not General Power */
			    /* Management device, then just go to next device */
			    else if ((p->deviceid.basetype
						== System_Peripheral) &&
			 	     ((p->deviceid.subtype
							!= Power_Management) ||
			  	      (p->deviceid.interface !=
						General_Power_Management))) {
					continue;
			    }

			    /* Otherwise, it is to be counted */
			    pci_resid_offset++;

			    /* See if DevFunc and Bus Number match */
			    if ((p->busaccess.pciaccess.busnumber
					== pci->busnum) &&
			        (p->busaccess.pciaccess.devfuncnumber
					== devfunc)) {

				sprintf(loc,"04-%c0",('A'+pci_resid_offset));
				DEBUG_1("Location code = %s\n",loc);
				pci_integrated_device(devfunc,p,fd,loc);
				break;
			    }
			}
		    }
		}

		/* Else it is for a card slot */
		else {

			/* Set up location code string */
			sprintf(loc,"04-%02d",pci->slotdata[i].slotnumber);

			/* See if a device is in slot */
			pciquery(devfunc,fd,loc);

			/* If a card is present, see if multifunction card */
			if (card_table[devfunc].slot_filled) {
			    multi = mf_query(devfunc,fd);

			    /* If multifunction, check other 7 functions */
			    if (multi) {
				for (func=1; func<=7; func++) {
				    pciquery(devfunc + func,fd,loc);
				}
			    }
			}
		}
	}

	/* Close bus device */
	close(fd);


	/* Match up devices with CuDv objects in data base */
	if (rc = sync_bus(busname, phase, PCI_CARD_TABLE_SIZE, PCI_TYPE))
		return(rc);

        /* Resolve bus resources */
        rc = busresolve(NULL, phase, conf_list, not_res_list, busname);
        if (rc){
		DEBUG_1("rc from busresolve = %d\n",rc);
                return (rc);
	}

        /* Print the list of adapters that were successfully resolved and
         * are to be configured to standard out.
         */
        fprintf(stdout, "%s\n", conf_list);
        return(0);
}


/*
 *
 * NAME: pciquery
 *
 * FUNCTION: query PCI bus and fill table.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: -1 : Failure 0 : Success
 *
 */
int
pciquery( devfunc, fd, loc)
int	devfunc;	/* DevFuncNumber of device to be queried */
int	fd;		/* Bus file descriptor */
char	*loc;		/* Location code for device */
{
        MACH_DD_IO      mddRecord;
	int		dev_num, function;
	int             rc;     /* return code from ioctl */

	DEBUG_0("In pciquery\n");

	/* build mdd record */
	mddRecord.md_size = 1;  
	mddRecord.md_incr = MV_WORD;
	mddRecord.md_addr = 0;
	mddRecord.md_sla = devfunc;
	mddRecord.md_data = (uchar*)(&(card_table[devfunc].cardid));



	/* the ioctl is called first so we can check return code */
	/* and the value of cardid returned */
	rc = ioctl(fd, MIOPCFGET, &mddRecord);

	if ((rc < 0) || (card_table[devfunc].cardid == -1)) {

		/* Then no device to configure */
		DEBUG_0("No device in slot\n");
		card_table[devfunc].cardid = PCI_BADVENDEV;
		card_table[devfunc].slot_filled = FALSE;
	}

	else {
		/* There is a device present, fill in card table */
		DEBUG_1("card id 0x%08x found\n", card_table[devfunc].cardid);
		card_table[devfunc].slot_filled = TRUE;
		sprintf(card_table[devfunc].pddv_sstr,
					"devid=0x%08x AND subclass=pci",
					card_table[devfunc].cardid);
		sprintf(card_table[devfunc].pkg_name,
					"%s.pci.%08x",DEVPKG_PREFIX,
					card_table[devfunc].cardid);
		strcpy(card_table[devfunc].location,loc);

       		if (prntflag) {
			fprintf(trace_file, "DevFunc %d = 0x%08x\n ",
				devfunc, *(int*)(mddRecord.md_data));

		}
			
        }
        return(0);
}

/*
 * NAME     : bus_depend()
 * FUNCTION : For the pci bus configurator, need to get resid data device table
 *	      and bus's PCI bridge descriptor.
 *
 * INPUTS   :
 *      busname : bus being configured
 *      devname : bus's special file name
 *
 * RETURNS : 0     
 *
 */
int
bus_depend(bus, devname)
char	*bus;		/* Name of bus device */
char	*devname;	/* /dev name for bus */
{
	int rc;
	uchar	devfuncnum;	/* DevFuncNumber */
	uchar	busnum;		/* PCI bus number of this bus */
	uchar	par_bus_num;	/* PCI bus number of parent PCI bus */
	int	i;		/* Loop variable for resid device table */
	struct CuAt *cuat;	/* For getting parent bus_number attribute */
	int	num_packets;	/* Return value from get_pci_descriptor() */
	int	cnt;		/* Return value from getattr() */


	/* get the number of devices and the device info */
	if (rc=get_resid_dev(&num_of_devices, &devp) != 0) {
		DEBUG_0("error calling get_resid_dev");
		return(E_DEVACCESS);
	}

	/* Now need to find bus's entry in resid data device table.  This
	 * differs for a bus of subclass 'sys' from one of subclass 'pci'.
	 */

	/* If device is of subclass 'sys' then ... */
	if (!strcmp(pddv.subclass,"sys")) {

		/* PCI bus number is in connwhere */
		busnum = (uchar)strtoul(strchr(cudv.connwhere,'.')+1,NULL,10);

		/* Loop through device table looking for processor device
		 * PCI bridges which have the correct PCI bus number
		 */
		for (i=0; i<num_of_devices; i++) {
			if (((devp+i)->deviceid.busid == PROCESSOR_DEVICE) &&
			   ((devp+i)->deviceid.basetype == Bridge_Controller) &&
			   ((devp+i)->deviceid.subtype == PCI_Bridge)) {

				/* Check PCI bus num in bridge descriptor */
				get_pci_descriptor(i,'a', &num_packets, &pci);
				if (pci->busnum == busnum) {
					/* Found correct device entry */
					break;
				}
			}
		}
	}

	/* Else it is of subclass 'pci' */
	else {
		/* DevFuncNumber comes from connwhere */
		devfuncnum = (uchar)strtoul(cudv.connwhere,NULL,10);

		/* Get parent PCI bus number from parent attribute */
		cuat = getattr(cudv.parent, "bus_number", FALSE, &cnt);
		if (cuat == NULL)
			return(E_NOATTR);
		par_bus_num = (uchar)strtoul(cuat->value, (char **)NULL, 0);

		/* Loop through device table looking for a PCI device whose
                 * DevFuncNumber and parent PCI bus number match for this bus
		 */
		for (i=0; i<num_of_devices; i++) {
			if (((devp+i)->deviceid.busid == PCI_DEVICE) &&
			   ((devp+i)->busaccess.pnpaccess.csn == par_bus_num) &&
			   ((devp+i)->busaccess.pnpaccess.logicaldevnumber ==
				devfuncnum)) {

				/* Found match, so get PCI bridge descriptor */
				get_pci_descriptor(i,'a', &num_packets, &pci);
				break;
			}
		}
	}

	return(0);
} 


/*
 * NAME     : bus_depend2()
 * FUNCTION : For the pci bus configurator, nothing 
 *	      needs to be done.
 *
 * INPUTS   :
 *      bus_type : 0=mca, 1=pci type of bus
 *      phase : 1,2,3
 *      cudv : pointer to cudv structure
 *      slot : slot that contains the adapter
 *      busname : bus being configured
 *
 * RETURNS : 0     
 *
 */
int
bus_depend2(bus_type, phase, cudv, slot, busname)
int	bus_type;
int	phase;
struct 	CuDv	*cudv;
int	slot;
char	*busname;
{
	return(0);
} 


/*
 *
 * NAME: mf_query
 *
 * FUNCTION:  determine whether adapter is multifunction or not
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: 0: Not multifunction   1: Multifunction
 *
 */
int
mf_query(devfunc, fd)
int	devfunc;	/* DevFuncNumber */
int	fd;		/* Bus file descriptor */
{
	MACH_DD_IO	mddHeader;
	char		header_type=0;


	DEBUG_0("In mf_query\n");

	/* set up mdd header for reading in the header type     */
	mddHeader.md_size = 1;
	mddHeader.md_incr = MV_BYTE;
	mddHeader.md_addr = 14;
	mddHeader.md_data = (uchar *)&header_type;
	mddHeader.md_sla = devfunc;
	ioctl(fd, MIOPCFGET, &mddHeader);

	/* The header_type will tell us if this is a multi-function device. */
	if ((header_type & 0x80)) {
		/* it is multifunction */
		return(1);
	}

	/* It is NOT multifunction */
	return(0);

}


/*
 *
 * NAME: bus_dependent_cfg
 *
 * FUNCTION: Determines if correct BUC table entry is found for this PCI
 *           bus, and if so, configures the bus.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  0: configured bus
 *			     -1: not correct BUC table entry for this bus
 *			     >0: error
 *
 */
int
bus_dependent_cfg(buc,index,bus_devname)
struct buc_info *buc;		/* IPL cntrl block buc info section */
int		index;		/* Index of BUC entry in BUC table */
char		*bus_devname;	/* /dev name for bus */
{
	int		rc;
	ulong		buid;
	struct CuAt	*cuat;
	ulong		cuat_busnumber;
	int	cnt;		/* Return value from getattr() */


	/* If BUC not for a PCI bus or PCI bus number not one we need ... */
	if (((buc->device_id_reg !=  PCI_BUS_ID) ||
	   (buc->bscr_value != pci->busnum))) {
		return(-1);
	}


	/* This is the correct BUC table entry for this PCI bus.  Get
	 * the buid value from BUC entry.
	 */
        buid = (ulong)BID_ALTREG(buc->buid_data[0].buid_value, 0);

	/* Load DMA kernel extension and initialize it */
        rc = load_kernel_mod(buc,RSPC_DMA_KERNEL_EXT);
        if (rc)
		return(rc);

	/* Set the bus's 'bus_id' attribute */
	rc = set_busid(buid);
	if (rc)
		return(rc);

	/* Set the 'bus_number' attribute */
	/* First get current value from ODM */
	cuat = getattr(cudv.name, "bus_number", FALSE, &cnt);
	if (cuat == NULL)
		return(E_NOATTR);

	/* See if current bus number is already correct */
	cuat_busnumber = strtoul(cuat->value, (char **)NULL, 0);
	if (cuat_busnumber != pci->busnum){
		/* The busnumber is not correct */

		/* Need to set busnumber to correct value */
		sprintf(cuat->value, "0x%x", pci->busnum);
		putattr(cuat) ;
	}

	/*
	 *  Make the special file for the bus.  Remember that
	 *  the Bus Id is the same as the connwhere.
	 */
	if (!mk_sp_file(bus_devname, index))
		return(E_MKSPECIAL);  /* Couldn't make special files */

	return(0);
}


/*
 *
 * NAME: pci_integrated_device
 *
 * FUNCTION: Fill in card table for integrated PCI device
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: -1 : Failure 0 : Success
 *
 */

int
pci_integrated_device(devfunc, p, fd, loc)
int	devfunc;	/* residual device function number */
CFG_DEVICE *p;		/* Pointer to device's resid data */
int	fd;		/* Bus file descriptor */
char    *loc;           /* Device location code */
{


	/* if device is a Bridge Controller, use PNP number (instead of
	   PCI ID) for devid and package name */

	if (p->deviceid.basetype == Bridge_Controller) {

		/* NOTE: cardid is set to 0 for bridge controllers */
		DEBUG_1("PCI to X Bridge Controller %s found\n", p->pnpid);
		card_table[devfunc].slot_filled = TRUE;
		card_table[devfunc].cardid = 0;
		sprintf(card_table[devfunc].pddv_sstr,
					"devid=%s AND subclass=pci", p->pnpid);
		sprintf(card_table[devfunc].pkg_name,
					"%s.pci.%s",DEVPKG_PREFIX, p->pnpid);
		strcpy(card_table[devfunc].location,loc);

		/* EISA bus treated as an ISA bus */
		if (p->deviceid.subtype==2) { /* change when get header file */

			/* alter PdDv search strings and package names */
			strcpy(card_table[devfunc].pddv_sstr,
				"devid=PNP0A00 AND subclass=pci");
			sprintf(card_table[devfunc].pkg_name,
				"%s.pci.PNP0A00",DEVPKG_PREFIX);

		}

       		if (prntflag) {
			fprintf(trace_file, "DevFunc %d = %s\n",
				devfunc, devfunc, p->pnpid);

		}

	} else {

		/* Its not a bridge controller */

		/* Get PCI device Id */
		pciquery(devfunc, fd, loc);
	}

	return(0);
}
