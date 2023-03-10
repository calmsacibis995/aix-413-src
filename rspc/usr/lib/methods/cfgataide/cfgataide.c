static char sccsid[] = "@(#)27  1.5  src/rspc/usr/lib/methods/cfgataide/cfgataide.c, rspccfg, rspc41J, 9515A_all 4/11/95 12:57:47";
/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS: 
 *              build_dds
 *		download_microcode
 *		generate_minor
 *		define_children
 *		make_special_files
 *		query_vpd
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* header files needed for compilation */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/device.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <sys/ide.h>
#include "cfgdebug.h"
#include "ataidecfg.h"

/*	extern declarations	*/
extern	long	*genminor();

/*      local declarations      */
#define CDROM		5		/* device_type for cdroms */

struct PdAt	Pd_attr;       /* PdAt for object matching search criteria */


/*
 * NAME   : generate_minor
 *
 * FUNCTION: This function generates minor device number for the 
 *	     adapter device.
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
 * FUNCTION: creates IDE device special file.
 *	     IDE has one charcter type device special file.
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
	return(mk_sp_file(devno,lname, 
                          ( S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
                                S_IWGRP | S_IROTH | S_IWOTH )));
   return(0);
}


/*
 * NAME	: query_vpd
 *
 * FUNCTION :
 *	Obtain vpd from common get_vpd routine.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called by generic config method.
 *
 * DATA STRUCTURES:
 *	vpd	: buffer to hold vpd.
 *
 * INPUT  : pointer to CuDv, driver kmid, devno
 * OUTPUT : vpd data 
 *
 * RETURNS: Returns 0  - vpd obtained successfully
 *		    E_VPD  - error obtaining vpd
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
int	rc = 0;			/* return code           */

	DEBUG_1("query_vpd: rc from getting subdvc VPD = %d\n",rc)
	return(rc) ;

} /* end query_vpd */

/*
 * NAME : build_dds
 * 
 * FUNCTION :
 *	This function builds the dds from ODM database.
 *	DDS consists of attrbutes of the IDE adapter.
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
struct	CuDv	CuDv;		/* customized device object          */
struct	CuDv	CuDv_bus;	/* parent bus's customized object    */
int	scratch;		/* scratch variable: num atts; bytes */
struct  attr_list *at_list;	/* attribute list		     */
struct  attr_list *bat_list;	/* attribute list for bus	     */
char    sstr[512];		/* buffer for search string	     */
char    bb[4];			/* value of battery-backed attribute */
struct	Class   *cusdev;	/* customized device class pointer   */
struct  Class   *predev;        /* predefined device class pointer   */
static  struct ide_adap_ddi dds;/* device dependent information      */
int     bus_num;		/* bus number		     	     */

	/* open CuDv_CLASS to get handle so we can get customized */
	/* device object for lname. */
	if((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open CuDv\n")
	    return E_ODMOPEN;
	}
	  
	sprintf(sstr,"name = '%s'",lname);
	rc = (int)odm_get_first(cusdev,sstr,&CuDv);
	if(rc == 0){
	    DEBUG_1("bld_dds: No entry in CuDv for %s\n",sstr)
	    return E_NOCuDv;
	}
	if(rc == -1){
	    DEBUG_1("bld_dds: err in getobj from CuDv for %s\n",sstr)
	    return E_ODMGET;
	}

	/* set slot */
     /* dds.slot = atoi(CuDv.connwhere) - 1; */
        dds.adap.slot = 0;
	DEBUG_2("bld_dds: conwhere=%s slot=%d\n",CuDv.connwhere,dds.adap.slot)
	DEBUG_2("bld_dds: ut = %s parent = %s\n",CuDv.PdDvLn_Lvalue,CuDv.parent)

       /*
        * Get the parent bus so that we can set some attributes based
        *     on the bus we are attached to (either directly or
        *     indirectly).
        */
	if ((rc = Get_Parent_Bus(cusdev, CuDv.parent, &CuDv_bus)) != 0)
	{
	    DEBUG_1("cfgataide: Unable to get parent bus object; rc = %d\n",
		     rc) 
	    if (rc == E_PARENT)
	    {
		rc = E_NOCuDvPARENT ;
	    } 
	    return (rc) ;
	}
	odm_close_class(cusdev);

	/* Read the attributes from the customized and predefined classes */
	/* We expect there to be 6 different attributes */
	at_list = (struct attr_list *) get_attr_list(CuDv.name,
				     CuDv.PdDvLn_Lvalue, &scratch, 6);
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

	/* open CuAt_CLASS to get handle so we can get customized */
	/* attributes for lname. */
	if((int)(cusdev = odm_open_class(CuAt_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open CuAt\n")
	    return E_ODMOPEN;
	}
	  
	/* open PdAt_CLASS to get handle so we can get predefined */
	/* attributes for uniquetype. */
	if((int)(predev = odm_open_class(PdAt_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open PdAt\n")
	    return E_ODMOPEN;
	}
	  
	/* Use the getatt function to extract the proper customized */
	/* or predefined attribute values from the odm databases.   */

	/* Use the bus attribute list to get bus_id. For IOCC to use */
	/* the bus, the id has to be ORed with a #defined string.    */
	if (rc = (getatt(bat_list, "bus_id", (void *) &dds.adap.bus_id,
		  'l', &scratch)) != 0)  {
            return (rc);
	}
        DEBUG_1("bld_dds: bus_id is set to %x\n",dds.adap.bus_id);

	/* Use the bus attribute list to get the bus type */
	if ((rc = getatt(bat_list, "bus_type", (void *) &dds.adap.bus_type,
		  'h', &scratch)) != 0)
            dds.adap.bus_type = 0;
        DEBUG_1("         bus_type is set to %x\n",dds.adap.bus_type);

	/* We are done with the bus attribute list, so free the memory. */
	/* Note that on an error, the memory isn't freed until it is    */
	/* reclaimed from the heap when the program ends.               */
	free(bat_list);

	/* Use the 'at_list' to retrieve the card attributes            */

	if ((rc = getatt(at_list, "bus_io_addr", (void *) &dds.adap.base_addr,
		    'i', &scratch)) != 0)
		return (rc);
        DEBUG_1("         base_addr is set to %x\n",dds.adap.base_addr);
	
        if ((rc = getatt(at_list, "dma_lvl", (void *) &dds.adap.dma_lvl, 'i',
                     &scratch)) != 0)
		return (rc);
        DEBUG_1("         dma_lvl is set to %x\n",dds.adap.dma_lvl);

	if ((rc = getatt(at_list, "bus_intr_lvl", (void *) &dds.adap.int_lvl, 
		     'i', &scratch)) != 0)
		return (rc);
        DEBUG_1("         int_lvl is set to %x\n",dds.adap.int_lvl);

	if ((rc = getatt(at_list, "intr_priority", (void *)&dds.adap.int_prior, 
		     'i', &scratch)) != 0)
		return (rc);
        DEBUG_1("         intr_priority is set to %x\n",dds.adap.int_prior);

	if ((rc = getatt(at_list, "pm_dev_itime", (void *)&dds.pm_dev_itime, 
		     'i', &scratch)) != 0) {
		if (rc == E_NOATTR)
			dds.pm_dev_stime = 0;
		else
			return (rc);
	}
        DEBUG_1("         pm_dev_itime is set to %x\n",dds.pm_dev_itime);

	if ((rc = getatt(at_list, "pm_dev_stime", (void *)&dds.pm_dev_stime, 
		     'i', &scratch)) != 0) {
		if (rc == E_NOATTR)
			dds.pm_dev_stime = 0;
		else
			return (rc);
	}
        DEBUG_1("         pm_dev_stime is set to %x\n",dds.pm_dev_stime);


	/* We are done with the attribute list, so free the memory. */
	/* Note that on an error (return prior to this point), the  */
        /* memory isn't freed until it is reclaimed from the heap   */
	/* when the program ends.                                   */
	free(at_list);

	odm_close_class(cusdev);
	odm_close_class(predev);

	/* set the resource name */
	strcpy(dds.adap.resource_name,lname);
      
	/* start of hard coded ddi values for ataide */
        dds.adap.cmd_delay = 30;

        dds.adap.bb_stream = FALSE;

        dds.adap.cmd_queue = FALSE;

        dds.adap.has_fuse = FALSE;

        dds.adap.tm_enabled = FALSE;

        dds.adap.tm_tcw_percent = 0;

        dds.adap.tm_tcw_length = 0;

	/* set return variables */
	*dds_data_ptr = (char *)&dds;
	*dds_len = sizeof(struct ide_adap_ddi);

	DEBUG_1("IDE DDS length: %d\n",*dds_len)
#ifdef	CFGDEBUG
	hexdump(*dds_data_ptr,(long)*dds_len);
#endif
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
 * FUNCTION : This routine determines and defines child devices of the IDE
 *	      adapter device driver.
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
	char		*lname;
	int		ipl_phase;
{
	int		rc;                     /* for return codes */

        rc = def_ide_children (lname, ipl_phase);
        return (rc);

}
