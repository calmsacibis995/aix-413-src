static char sccsid[] = "@(#)69	1.1  src/rspc/usr/lib/methods/cfgkent/cfgkent.c, pcient, rspc41J, 9507B 1/25/95 11:33:46";
/*
 *   COMPONENT_NAME: pcient
 *
 *   FUNCTIONS: GETATT
 *		build_dds
 *		define_children
 *		download_microcode
 *		query_vpd
 *              device_specific
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
#include <stdio.h>
#include <fcntl.h>
#include <cf.h>         /* Error codes */

#include <sys/cfgodm.h>
#include <sys/adspace.h>
#include <sys/comio.h>
#include <sys/cfgdb.h>
#include <sys/ndd.h>
#include <sys/cdli_entuser.h> 
#include <kent/kent_dds.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <string.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif


#include "pparms.h"
#include "cfgtoolsx.h"
#include "cfgdebug.h"
#include "cfg_ndd.h"
#include "cfgcommon.h"


extern	long		*genminor();
static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int	how_many;		/* Used by getattr routine.            */
int	byte_cnt;		/* Byte count of attributes retrieved  */

#define GETATT(A,B,C)		getatt(alist,C,A,B,&byte_cnt)

/*
 * NAME:  build_dds()
 *
 * FUNCTION:
 *	This builds the dds for the IBM ethernet device.
 *
 * EXECUTION ENVIRONMENT:
 *	This is a common build dds routine for the configuration and
 *	change methods.  The only difference between the two is that
 *	the configuration method passes a null attribute list pointer
 *	because all variable attributes are retieved from the data base.
 *	The change method may have changed attributes passed in.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int build_dds(char          *logical_name,
              uchar         **dds_ptr,
              long          *dds_length)
{
	kent_dds_t      *p_dds;		
  	char		*parent=NULL, *location=NULL;
  	struct CuDv     cust_obj, bus_obj;
  	struct CuAt     *CuAt_obj;	
  	struct PdAt     PA_obj;	 
  	struct attr_list *bus_attr_list = NULL;
  	char            *parentdesc, srchstr[256], utype[UNIQUESIZE], *ptr,
                   	tmp_str[5];	/* yes,no value for flag fields      */
  	int		i,rc;		
  	int             bus_num;	/* Bus number                        */
  	int             num_bus_attrs;	/* Number of bus attributes          */

  	DEBUG_0("build_dds(): BEGIN build_dds()\n")

  	*dds_length = sizeof(kent_dds_t);
  	if ((p_dds = (kent_dds_t *) malloc(sizeof(kent_dds_t))) == NULL) 
	{
    		DEBUG_0 ("build_dds(): Malloc of dds failed\n")
    		return E_MALLOC;
  	}
  	*dds_ptr = (uchar *) p_dds;

  	/* set default value for all attributes to zero */
  	bzero(p_dds,sizeof(kent_dds_t));

  	/* get unique type from customized OC */
  	sprintf(srchstr,"name = '%s'",logical_name);
  	if ((rc=(int)odm_get_first(CuDv_CLASS,srchstr,&cust_obj)) == 0)
	{
    		return E_NOCuDv;
	}
  	else if (rc == -1)
	{
    		return E_ODMGET;
	}

  	strcpy(utype,cust_obj.PdDvLn_Lvalue);

  	/* Get attribute list */
  	if ((alist=(struct attr_list *)get_attr_list(logical_name,utype,
			&how_many,16)) == NULL)
    		return(E_ODMGET);


  	/* Regular Attributes */
  	if (rc=GETATT(&p_dds->busio,'i',"busio"))
	{
		DEBUG_1("Failed to get the busio, rc = %d",rc);
    		return(rc);
	}

  	if (rc=GETATT(&p_dds->busintr,'i',"busintr"))
	{
		DEBUG_1("Failed to get the busintr, rc = %d",rc);
    		return(rc);
	}

	/* adapter specific attributes */
  	if (rc=GETATT(&p_dds->tx_que_sz,'i',"tx_que_size"))
	{
		DEBUG_1("Failed to get the tx_que_size, rc = %d",rc);
    		return(rc);
	}

	/* verify the attributes values */
	if (!((p_dds->tx_que_sz==16) || (p_dds->tx_que_sz==32) ||
	     (p_dds->tx_que_sz==64) || (p_dds->tx_que_sz==128) ||
	     (p_dds->tx_que_sz==256)))
	{
		DEBUG_1("Invalid value in tx_que_size, tx_que_size = %d",
			p_dds->tx_que_sz);
		return(E_INVATTR);
	}

  	if (rc=GETATT(&p_dds->rx_que_sz,'i',"rx_que_size"))
	{
		DEBUG_1("Failed to get the rx_que_size, rc = %d",rc);
    		return(rc);
	}

	if (!((p_dds->rx_que_sz==16) || (p_dds->rx_que_sz==32) ||
	     (p_dds->rx_que_sz==64) || (p_dds->rx_que_sz==128) ||
	     (p_dds->rx_que_sz==256)))
	{
		DEBUG_1("Invalid value in rx_que_size, rx_que_size = %d",
			p_dds->rx_que_sz);
		return(E_INVATTR);
	}

	tmp_str[0] = NULL;
	GETATT(&tmp_str[0], 's', "use_alt_addr");
	p_dds->use_alt_addr = (tmp_str[0] == 'y') ? 1 : 0;

	if (p_dds->use_alt_addr)
	{

		rc = GETATT(p_dds->alt_addr,'b',"alt_addr");
                /*
                 * do address check: only 6 byte addresses are valid
                 */
                if (byte_cnt != 6)
                {
                        DEBUG_0("failed getting the alt_addr\n");
                        DEBUG_1("dds: len of alt_addr = %d\n", byte_cnt);
                        return (E_INVATTR);
                }
	}


  	/* Attributes from other Object Classes/Places */
  	/* Get bus type, bus id, and bus number.  */
  	rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj); 
  	if (rc) 
	{
		DEBUG_1("get parent failed, rc = %d\n",rc);
    		if (rc == E_PARENT) 
      			rc = E_NOCuDvPARENT;
  		return(rc);
  	}

  	if ((bus_attr_list = (struct attr_list *)get_attr_list(bus_obj.name, 
		bus_obj.PdDvLn_Lvalue, &num_bus_attrs, 4)) == NULL)  
	{
		DEBUG_0("bus_attr_list failed on bus\n");
    		return(E_ODMGET);   
	}

	/* To access the adapter the driver needs the bus id of the bus */
	/* the adapter is plugged into (its odm parent). */
  	if (rc = getatt(bus_attr_list, "bus_id", &p_dds->busid, 'l'))
	{
		DEBUG_1("getatt busid failed, rc = %d\n",rc);
    		return(rc);
	}

  	bus_num = bus_obj.location[3] - '0';

	/* To access the adapter's configuration space the driver needs the */
	/* md_sla value.  This replaces the microchannel's slot number */
	p_dds->md_sla = atoi(cust_obj.connwhere);
        DEBUG_1("md_sla indicated as - %d\n",p_dds->md_sla);

  	/* Put the logical name in the DDS for use by error logging */
  	strcpy(p_dds->lname,logical_name);

  	rc = strncmp(pddv.prefix,logical_name,
		   strlen(pddv.prefix));

  	strcpy(p_dds->alias,"en");

  	if (rc == 0) 
	{
    		/* logical name created from prefix, copy only extension */
    		strcat(p_dds->alias,logical_name+strlen(pddv.prefix));
  	} 
	else 
	{
      		/* logical name not created from prefix, append entire string */
      		strcat(p_dds->alias,logical_name);
    	}
  	DEBUG_1("build_dds(): alias name is %s\n",p_dds->alias)

#ifdef CFGDEBUG
	hexdump(p_dds,(long) sizeof(kent_dds_t));
#endif

  	return(0);
}

/*
 * NAME:
 *	download_microcode()
 *
 * FUNCTION:
 *	This downloads microcode to a ethernet ring adapter.
 *	Note: this function is not used by the PCI Ethernet adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int download_microcode(char            *logical_name,
                       struct cfg_kmod *cfg_k)
{

  	return(0);
}

/*
 * NAME:
 *	query_vpd()
 *
 * FUNCTION:
 *	Retrieve Vital Product Data (VPD) from the adapter card in
 *	order to store it in the database from later use.
 *
 * RETURNS:
 *		0 on Success, >0 Error code
 */

int query_vpd(char            *logical_name,
              mid_t		k_mid,
              dev_t		devno,
              char            *vpd)
{
  	char		vpd_area [100];		/* Storage area for VPD */
  	ulong	        ul;
  	struct cfg_kmod cfg_k;          /* sysconfig command structure */
  	ndd_config_t ndd_config;

  	DEBUG_0("query_vpd(): BEGIN query_vpd()\n")

  	/* Initialize the cfg structure for a call to sysconfig(), which */
  	/* will in turn call the ddconfig() section of the appropriate	 */
  	/* device driver.						 */

  	ndd_config.seq_number = devinst;
  	ndd_config.p_vpd = &vpd_area[0];   	/* Storage space for VPD */
  	ndd_config.l_vpd = 100; 	/* Size of VPD storage area */
  	cfg_k.mdiptr = (char *) &ndd_config;
  	cfg_k.mdilen = sizeof (struct ndd_config);
  	cfg_k.kmid = kmid;
  	cfg_k.cmd = CFG_QVPD;             /* Command to read VPD */

  	/* Make the call to sysconfig: */
  	if (sysconfig(SYS_CFGKMOD, &cfg_k, sizeof(struct cfg_kmod))<0) 
	{
#ifdef CFGDEBUG
    		DEBUG_0("query_vpd(): Dump of vpd_area\n")
    		hexdump(&vpd_area,(long) sizeof(vpd_area));
    		switch(errno) 
		{
      		case EINVAL:
        		DEBUG_1("query(): invalid kmid = %d\n",cfg_k.kmid)
        		break;
      		case EACCES:
        		DEBUG_0("query(): not privileged\n")
        		break;
      		case EFAULT:
        		DEBUG_0("query(): i/o error\n")
        		break;
      		default:
        		DEBUG_1("query(): error = 0x%x\n",errno)
        		break;
    		}
#endif
DEBUG_0("Error in vpd JEM\n");
   	return E_VPD;
  	}

  	/* Store the VPD in the database */
  	put_vpd(vpd,vpd_area, ndd_config.l_vpd);
  	return(0);
}

/*
 * NAME:
 *	define_children()
 *
 * FUNCTION:
 *	Note: There are no children for the PCI Ethernet Adapter.
 *
 * RETURNS:
 *	Always returns 0 - Success.
 */

int define_children(char *logical_name,
                    int           phase)
{
  	return(0);
}

/*
 * NAME: device_specific
 *
 * FUNCTION:This is a null function for the time being and returns success
 * always
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function operates as a device dependent subroutine called by the
 *      generic configure method for all devices.
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: Returns 0 (success), if called.
 */

int device_specific()
{
  	return(0);
}
