static char sccsid[] = "@(#)15	1.8  src/rspc/usr/lib/methods/cfgbus_pci/sync.c, rspccfg, rspc41J, 9524E_all 6/14/95 18:07:10";
/*
 *   COMPONENT_NAME: cmdbuscf    sync.c
 *
 *   FUNCTIONS: get_cudv_entry
 *		get_pddv_entry
 *		sync_bus
 *              disable_pci_devices
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <fcntl.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mdio.h>

#include "bcm.h"
#include "cfgdebug.h"

extern  CARD_INFO       card_table[PCI_CARD_TABLE_SIZE];

extern	int	allpkg;			/* packaging flag */

/* -------------------------------------------------------------------------- */
/*
 * NAME: get_pddv_entry 
 *
 * FUNCTION: Locates the correct predefine entry matching card id
 *
 * RETURNS: 0 if predefined device information found
 *          E_FINDCHILD if predefined device information NOT found
 *          E_ODMGET if other ODM error encountered
 */

int
get_pddv_entry(phase, busname, slot, pddv, bus_type)
int	phase;		/* IPL phase */
char	*busname;	/* parent bus name */
int	slot;		/* slot containing card */
struct  PdDv *pddv;	/* Returned PdDv for card */
int	bus_type;	/* 0=mca, 1=pci		*/
{
	struct CuAt	CuAt;	/* to determine if FRS should be called    */
	int    rc;
	char   sstr[80];	/* search string */


	DEBUG_0("in get_pddv_entry\n");

	/* Attempt to get device information from PdDv */
	rc = (int)odm_get_first(PdDv_CLASS, card_table[slot].pddv_sstr, pddv);
	if (rc==-1) {
		/* ODM failure */
		return(E_ODMGET);
	}
	else if (rc==0) {
		/* No PdDv object for this device */
 		return(E_FINDCHILD);
	}

	/* Trace the predefined data */
	if (prntflag)
		fprintf(trace_file,"\tuniquetype = %s\n\tbus_ext = %d\n",
			pddv->uniquetype,pddv->bus_ext);

	return(0);
}

/* ------------------------------------------------------------------------- */
/*
 * NAME: get_cudv_entry 
 *
 * FUNCTION: Locates the correct customized entry matching card type and slot.
 *           Defines new customized entry if necessary.
 *
 * RETURNS: 0 if customized device information found
 *          >0 if error encountered
 */

int
get_cudv_entry(busname, slot, pddv, cudv)

char	*busname;		/* parent bus name */
int	slot;			/* slot containing card */
struct PdDv *pddv;		/* PdDv for card */
struct CuDv *cudv;		/* Returned CuDv for card */
{
	int	rc;
	char	sstr[80];
	char	cmdarg[DEFAULTSIZE];
	char	*out_ptr = NULL;
	char	*err_ptr = NULL;

	DEBUG_0("in get_cudv_entry\n");

	/* Attempt to get CuDv for card */
	/* NOTE: Maybe should always attempt to find an AVAILABLE CuDv first*/
	sprintf(sstr, "parent=%s AND connwhere=%d AND PdDvLn=%s",
					busname, slot, pddv->uniquetype);
	rc = (int)odm_get_first(CuDv_CLASS, sstr, cudv);
	if (rc==-1) {
		/* ODM failure */
		return(E_ODMGET);
	}
	else if (rc==0) {

		/* No CuDv object for this device, need to define one */
		sprintf(cmdarg, "-c %s -s %s -t %s -p %s -w %d -L %s -d",
			pddv->class,pddv->subclass,pddv->type,busname,slot,
						card_table[slot].location);

		rc = odm_run_method(pddv->Define, cmdarg, &out_ptr, &err_ptr);

		if (prntflag) {
			fprintf(trace_file,"invoke: %s %s\nrc = %d\n",
						pddv->Define, cmdarg, rc);
			trace_output( out_ptr, err_ptr);
		}

		/* Attempt to get CuDv that was just defined for card */
		rc = (int)odm_get_first(CuDv_CLASS, sstr, cudv);
		if (rc==-1) {
			/* ODM failure */
			return(E_ODMGET);
		}
		else if (rc==0) {
			/* Shouldn't have happened */
			return(E_NOCuDv);
		}
	}

	/* Successfully obtained the card's matching CuDv object */
	/* Set the chgstatus field */
	if (cudv->chgstatus == MISSING) {
		cudv->chgstatus = SAME;
		if (odm_change_obj(CuDv_CLASS, cudv) == -1)
			return(E_ODMUPDATE);
	}

	/* Trace the customized data */
	if (prntflag)
		fprintf(trace_file,
			"\tname= %s\n\tstatus= %d\n\tchgstatus = %d\n",
			cudv->name,cudv->status,cudv->chgstatus);

	return(0);
}


/* -------------------------------------------------------------------------- */
/*
 * NAME: sync_bus 
 *
 * FUNCTION: This function finds the CuDv objects corresponding to the
 *           adapter cards that have been found to be present on the bus.
 *           It defines new CuDv objects for new adapters.  It sets the
 *           chgstatus in the CuDv object so that busresolve() will
 *           know what adapters need to have their bus resources
 *           resolved.
 *
 * RETURNS: 0 if successful
 *          >0 if error encountered
 * successful. 
 *
 */

int
sync_bus(busname, phase, card_table_size, bus_type)

char	*busname;	/* the name of the bus being configured */
int	phase;		/* The phase currently being executed   */
int	card_table_size;
int	bus_type;	/* 0=mca, 1=pci				*/

{
	struct PdDv	pddv;
	struct CuDv	cudv;
	register int    slot;	/* card slot number */
	int		rc;

	DEBUG_0("In sync_bus\n");

	/* Process each card that was found to be present */
	for (slot = 0; slot < card_table_size; slot++) {


		/* Skip slots that have no cards */
		if (!card_table[slot].slot_filled )
			continue;	/* slot was empty, next iteration */

		if (prntflag)
			fprintf(trace_file,
				"\nFound device at %d\n\tId = 0x%08x\n",
				slot, card_table[slot].cardid);

		/* Get PdDv for card in slot */
		rc = get_pddv_entry(phase,busname,slot,&pddv,PCI_TYPE);

		if (rc == E_FINDCHILD || allpkg) {

		    /* output the pkg name */
		    printf(":%s ",card_table[slot].pkg_name);

		    if (rc == E_FINDCHILD) {
			/* card has no PdDv installed so continue */
			if (prntflag)
				fprintf(trace_file,"\tNo PdDv found - need to install pkg: %s\n",card_table[slot].pkg_name);

			/* Disable PCI devices in case */
			/* firmware left it enabled */
			if (phase == 1 )
			    disable_pci_devices(slot, busname);
			continue;
		    }

		}

		if (rc)
			/* fatal error */
			return(rc);

		/* Got the card's PdDv, now get its CuDv object */
		rc = get_cudv_entry(busname, slot, &pddv, &cudv);
		if (rc == E_NOCuDv) {
			/* If no CuDv, just go on to next device */
			/* Disable PCI devices in case */
			/* firmware left it enabled */
			if (phase == 1 )
			    disable_pci_devices(slot, busname);
			continue;
		}
		else if (rc)
			/* fatal error */
			return(rc);

		/* Disable PCI devices in case firmware left it enabled */
		if (phase == 1 && cudv.status == DEFINED)
			disable_pci_devices(slot, busname);

		rc = bus_depend2( bus_type, phase, cudv, slot, busname);
		if (rc)
			/* fatal error */
			return(rc);
		

		/* If card is a bus extender, need to invoke its    */
		/* configure method in order to define its children */
		/* Note : ignore errors from invoke_adapt_cfg() to  */
		/* ensure all devices are processed if a Configure  */
		/* method fails.                                    */
		if (pddv.bus_ext) {
			rc = invoke_adapt_cfg(phase, &pddv, &cudv);

			/* Need to restore bus's LED value */
			if (phase)
				setleds(BUS_CONFIG_METHOD);
		}
	}

	return(0);
}

/*
 * Disable PCI devices in case firmware left them enabled.
 * (Specifically need to disable graphics adapters that firmware
 * does not disable due to OS/2.)
 */
static int
disable_pci_devices(slot, busname)
int     slot;
char    *busname;

{
	MACH_DD_IO      mddRecord;
	ushort          control;
	int             fd;
	char            name[100];

	DEBUG_0("In disable_pci_devices\n");

	if ( card_table[slot].cardid == 0)
		return(0);

	sprintf(name, "/dev/%s", busname);
	if ((fd = open(name, O_RDWR)) == -1)
		return(-1);

	/* build mdd record */
	mddRecord.md_size = 1;
	mddRecord.md_incr = MV_SHORT;
	mddRecord.md_addr = 0x4;
	mddRecord.md_sla = slot;
	mddRecord.md_data = (uchar*)(&control);

	if (ioctl(fd, MIOPCFGET, &mddRecord) < 0) {

		/* Then no device to configure */
		DEBUG_1("    card id 0x%08x not found\n", card_table[slot].cardid);
		card_table[slot].cardid = PCI_BADVENDEV;
		card_table[slot].slot_filled = FALSE;
	}

	else {
		/* There is a device present, disable it */
		DEBUG_2("    card id 0x%08x found, control = 0x%x\n",
			     card_table[slot].cardid,control);

		control = control & 0xf8ff;
		DEBUG_1("    control value to be set to 0x%x\n", control);
		ioctl(fd, MIOPCFPUT, &mddRecord);
		    /* if result < 0, error - device not disabled */
		    /* do not report error to continue working */
		    /* on all devices */

	}
	close(fd);
	return(0);

}
