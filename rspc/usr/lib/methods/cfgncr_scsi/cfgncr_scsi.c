static char sccsid[] = "@(#)07  1.5  src/rspc/usr/lib/methods/cfgncr_scsi/cfgncr_scsi.c, cfgmethods, rspc41J, 9516A_all 4/14/95 15:05:12";
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: build_dds
 *		download_microcode
 *		generate_minor
 *		define_children
 *		make_special_files
 *		query_vpd
 *              device_specific
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* header files needed for compilation */

#include <sys/stat.h>
#include <sys/device.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <sys/scsi.h>
#include <sys/mdio.h>
#include <sys/intr.h>
#include "cfghscsi.h"
#include "cfgtoolsx.h"
#include "cfgdebug.h"
#include "cfgcommon.h"
#include "ncr8xxcfg.h"

/*	extern declarations	*/
extern	long	*genminor();

static int allnumb(char *s);

/*
 * NAME   : generate_minor
 *
 * FUNCTION: This function generates minor device number for the 
 *	     SCSI adapter device.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	It makes use of generic genminor() function.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : device logical_name,major_no.
 *
 * RETURNS: an integer pointer to the minor number.
 *
 * RECOVERY OPERATION:
 * 
 */

long generate_minor(lname,major_no,minordest)
char	*lname;
long	major_no;
long	*minordest;
{
long	*minorno;

	minorno = genminor(lname,major_no,-1,1,1,1);
	if(minorno == (long *)NULL)
		return E_MINORNO;
	*minordest = *minorno;
	return 0;
}

/*
 *
 * NAME   : make_special_files
 * 
 * FUNCTION: creates scsi device special file.
 *	     scsi has one charcter type device special file.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : logical_name,devno
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 * 
 */

int make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
	DEBUG_2("make_special_files(%s,%x)\n",lname,devno)
	return(mk_sp_file(devno,lname,CRF));
}


/*
 * NAME	: query_vpd
 *
 * FUNCTION :
 *	This device does not support querying vpd.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called by generic config method.
 *
 * DATA STRUCTURES:
 *	bb_vpd	: structure to hold vpd.
 *
 * INPUT  : pointer to CuDv, driver kmid, devno
 *
 * RETURNS: Returns 0 
 *
 * RECOVERY OPERATION:
 *
 */

int query_vpd(cusobj,kmid,devno,vpd)
struct	CuDv *cusobj;
mid_t	kmid;
dev_t	devno;
char	*vpd;
{
    return(0);	
} /* end query_vpd */

/*
 * NAME : build_dds
 * 
 * FUNCTION :
 *	This function builds the dds from ODM database.
 *	DDS consists of attrbutes of the scsi adapter.
 *	It returns the pointer to dds to the generic config method
 *	which in turn passes it on to the device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	This in turn calls get_attr_list and getatt functions to
 *	get the attributes from the database.
 *
 * DATA STRUCTURES:
 *	CuDv, CuDv_bus : Customised device object structure
 *      at_list, bat_list : list of attributes for device and bus
 *	cusdev : pointer to CuDv object class
 *	dds : pointer to dds structure
 *	
 * INPUT  : logical_name, ptr to ddsptr, ptr to dds_length
 *
 * RETURNS: 0 if success else -1
 *
 * RECOVERY OPERATION:
 *
 */

int build_dds(lname,dds_data_ptr,dds_len)
char	*lname;
char	**dds_data_ptr;
long	*dds_len;
{
int	rc;			/* return code			     */
struct	CuDv	CuDv_bus;	/* parent bus's customized object    */
int	scratch;		/* scratch variable: num atts; bytes */
struct  attr_list *at_list;	/* attribute list		     */
struct  attr_list *bat_list;	/* attribute list for bus	     */
char    sstr[512];		/* buffer for search string	     */
char    tmp_buffer[ATTRVALSIZE];/* buffer for search results         */
static  p8xx_ddi_t dds;		/* device dependent information      */


	memset (&dds, 0, sizeof(p8xx_ddi_t) );	
	  
	/* Set slot */
	dds.addi.slot = atoi(cudv.connwhere);
	DEBUG_2("bld_dds: connwhere=%s slot=%d\n",cudv.connwhere,dds.addi.slot)
	DEBUG_2("bld_dds: ut = %s parent = %s\n",cudv.PdDvLn_Lvalue,cudv.parent)

       /*
        * Get the parent bus so that we can set some attributes based
        *     on the bus we are attached to (either directly or indirectly).
        */
	if ((rc = Get_Parent_Bus(CuDv_CLASS, cudv.parent, &CuDv_bus)) != 0)
	{
	    DEBUG_1("cfgncr_scsi: Unable to get parent bus object; rc = %d\n",
		     rc) 
	    if (rc == E_PARENT)
	    {
		rc = E_NOCuDvPARENT ;
	    } 
	    return (rc) ;
	}

	/* Read the attributes from the customized and predefined classes */
	/* We expect there to be 8 different attributes */
	at_list = (struct attr_list *) get_attr_list(cudv.name,
	    cudv.PdDvLn_Lvalue, &scratch, 8);
	if (at_list == (struct attr_list *) NULL)
	{
	    DEBUG_0 ("bld_dds: get_attr_list failed.\n");
	    return (E_NOATTR);
	}

	/* read the bus's attributes so that we can set some of our  */
	/* attributes based on the bus's values.  We expect there to */
	/* be 3 attributes for the bus. */
	bat_list = (struct attr_list *) get_attr_list(CuDv_bus.name,
				CuDv_bus.PdDvLn_Lvalue, &scratch, 3);
	if (bat_list == (struct attr_list *) NULL)
	{
	    DEBUG_0 ("bld_dds: get_attr_list for parent failed.\n");
	    return (E_NOATTR);
	}
	DEBUG_1("bld_dds: pt = %s\n",CuDv_bus.PdDvLn_Lvalue)
	DEBUG_2("bld_dds: parent bus = %s for %s\n",CuDv_bus.name,lname)

	/* Use the getatt function to extract the proper customized */
	/* or predefined attribute values from the odm databases.   */

	/* Use the bus attribute list to get bus_id. */
	if (rc = (getatt(bat_list, "bus_id", (void *) &dds.addi.bus_id,
		  'l', &scratch)) != 0)
            return (rc);

	/* We are done with the bus attribute list, so free the memory. */
	/* Note that on an error, the memory isn't freed until it is    */
	/* reclaimed from the heap when the program ends.               */
	free(bat_list);

	/* Use the attribute list to get the SCLK for the chip */
	if ((rc = getatt(at_list, "scsi_clk", (void *) &dds.scsi_clk,
		'i', &scratch)) != 0) {
	    /* assume default for system board mounted chip */
	    dds.scsi_clk = NCR8XX_SCSI_CLK;
	    DEBUG_2("bld_dds: error %d getting scsi_clk, default to %dns\n",
		rc, dds.scsi_clk);
	}

	/* Use the attribute list to get the card's scsi id */
	if ((rc = getatt(at_list, "id", (void *) &dds.addi.card_scsi_id,'c',
		  &scratch)) != 0) 
	    return (rc);

	/* Continue getting attributes from the attribute list */
	if ((rc = getatt(at_list, "bus_io_addr", (void *) &dds.addi.base_addr,
		    'i', &scratch)) != 0)
		return (rc);

	if ((rc = getatt(at_list, "bus_intr_lvl", (void *) &dds.addi.int_lvl, 
		     'i', &scratch)) != 0)
		return (rc);

	/* bus_intr_lvl also encodes any needed interrupt flags, so get
	 * it again and parse the string...a "E" means INTR_EDGE
	 */
	if ((rc = getatt(at_list, "bus_intr_lvl", (void *) tmp_buffer, 
		     's', &scratch)) != 0) 
		return (rc);

	if (strstr(tmp_buffer, "E") != NULL)
	    	dds.intr_flags = INTR_EDGE;
	else
		dds.intr_flags = 0;

	if ((rc = getatt(at_list, "intr_priority", (void *) &dds.addi.int_prior, 
		     'i', &scratch)) != 0)
		return (rc);

        if ((rc = getatt(at_list,"pm_dev_itime", (void *) &dds.pm_dev_itime,
                        'i',&scratch)) == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just
                 * set dds.pm_dev_itime = 0.
                 */
                dds.pm_dev_itime = 0;
        } else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }

        if ((rc = getatt(at_list,"pm_dev_stime", (void *) &dds.pm_dev_stime,
                        'i',&scratch)) == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just
                 * set dds.pm_dev_stime = 0.
                 */
                dds.pm_dev_stime = 0;
        } else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }

	if ((rc = getatt(at_list, "bb", (void *) tmp_buffer, 's', 
			&scratch)) != 0)
		return (rc);
	if (strncmp (tmp_buffer, "yes", 4) == 0)
		dds.addi.battery_backed = TRUE;
	else
		dds.addi.battery_backed = FALSE;

	/* We are done with the attribute list, so free the memory. */
	/* Note that on an error (return prior to this point), the  */
        /* memory isn't freed until it is reclaimed from the heap   */
	/* when the program ends.                                   */
	free(at_list);

	/* set the resource name */
	strcpy(dds.addi.resource_name,lname);
      
	/* indicate the chip-type via the devid in the global pddv object */
	DEBUG_1("pddv.devid is %s\n", pddv.devid)
	switch (strtoul(pddv.devid, (char **) NULL, 0)) {
            case P810_SIGNATURE:
                dds.chip_type = IS_A_53C810;
                break;
            case P820_SIGNATURE:
                dds.chip_type = IS_A_53C820;
                break;
            case P825_SIGNATURE:
                dds.chip_type = IS_A_53C825;
                break;
            default:
	 	DEBUG_1("cfgncr_scsi unknown devid:  0x%x\n",pddv.devid)
		return(E_WRONGDEVICE);
        }

	/* start of hard coded ddi values for cfgncr_scsi */
        dds.addi.cmd_delay = 7;

        dds.addi.bb_stream = FALSE;

        dds.addi.cmd_queue = FALSE;

        dds.addi.has_fuse = FALSE;

        dds.addi.tm_enabled = FALSE;

        dds.addi.tm_tcw_percent = 0;

        dds.addi.tm_tcw_length = 0;

	/* set return variables */
	*dds_data_ptr = (char *)&dds;
	*dds_len = sizeof(dds);

	DEBUG_1("cfgncr_scsi DDS length:    %d\n",*dds_len)
	DEBUG_1("cfgncr_scsi base address:  0x%x\n",dds.addi.base_addr)
	DEBUG_1("cfgncr_scsi DMA  level:    0x%x\n",dds.addi.dma_lvl)
	DEBUG_1("cfgncr_scsi intr level:    0x%x\n",dds.addi.int_lvl)
	DEBUG_1("cfgncr_scsi intr flags:    0x%x\n",dds.intr_flags)
	DEBUG_1("cfgncr_scsi intr priority: 0x%x\n",dds.addi.int_prior)
	DEBUG_1("cfgncr_scsi bus id:        0x%x\n",dds.addi.bus_id)
	DEBUG_1("cfgncr_scsi scsi_clk:      0x%x\n",dds.scsi_clk)
	DEBUG_1("cfgncr_scsi bat backed:    0x%x\n",dds.addi.battery_backed)
	DEBUG_1("cfgncr_scsi chip type:     0x%x\n",dds.chip_type)
#ifdef	CFGDEBUG
/*	hexdump(*dds_data_ptr,(long)*dds_len); */
#endif	/* CFGDEBUG */
	return(0);
}


/*
 * NAME : download_microcode
 * 
 * FUNCTION :
 *      No download of microcode is done for this device. 
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : logical_name
 *
 * RETURNS: Returns 0 if success else error indication
 *
 * RECOVERY OPERATION:
 *
 */

int download_microcode(lname)
char	*lname;
{
	return(0);
}

int device_specific()
{
	return(0);
}


/*
 * NAME   : define_children
 *
 * FUNCTION : This routine determines whether or not to download
 *            microcode to the SCSI adapter.  It then calls the
 *            routine to define child devices on a SCSI bus.
 *
 * EXECUTION ENVIRONMENT:
 *      This function operates as a device dependent subroutine
 *      called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : logical_name,ipl_phase
 *
 * RETURNS: Returns 0 if success
 *
 * RECOVERY OPERATION:
 *
 */

int
define_children (lname, ipl_phase)
char    *lname;
int     ipl_phase;
{
        int     rc;                     /* for return codes */
        char    sstr[256];              /* ODM search string               */
	int     num_luns;               /* number of luns device supports  */
	int     num_sids;               /* number of scsi ids device supports */

        DEBUG_0 ("cfgncr_scsi: entering define children routine\n");

	/* If this is a NCR53C810, it's a narrow bus with 8 scsi ids */
        if (!strcmp(pddv.type, "ncr810"))
	    num_sids = 8;
	else
	    num_sids = 16;

	num_luns = 8;
	rc = def_scsi_children (lname, ipl_phase, num_sids, num_luns);
	return (rc);
}
