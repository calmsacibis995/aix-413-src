static char sccsid[] = "@(#)63  1.6  src/rspc/usr/lib/methods/cfgisatok/cfgisatok.c, isatok, rspc41J, 9523C_all 6/7/95 13:34:34";
/*
 *   COMPONENT_NAME: SYSXITOK
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
#include <sys/comio.h>
#include <sys/cfgdb.h>
#include <sys/ndd.h>
#include <sys/cdli_tokuser.h>
#include <itok/itok_ddi.h>
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
extern  struct attr_list *get_attr_list(char *, char *, int *, int);

static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int	how_many;		/* Used by getattr routine.            */
int	byte_cnt;		/* Byte count of attributes retrieved  */

#define GETATT(A,B,C)		getatt(alist,C,A,B,&byte_cnt)


/*
 * NAME:  build_dds()
 *
 * FUNCTION:
 *	This builds the dds for the token ring device.
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

/* DDS structure to be a static declaration */
static  itok_ddi_t       ddi_ptr;        /* DDS structure */

int build_dds(char          *logical_name,
              uchar         **dds_ptr,
              long          *dds_length)
{

  struct CuAt      *CuAt_obj;	         /* Customized Attributes data struct */
  struct attr_list *bus_attr_list = NULL; /* Bus attributes list.             */
  char             tmp_str[5];           /* yes,no value for flag fields      */
  int		   i,rc;		 /* flags for getting parent attrib   */
  int		   ring_speed;	         /* Ring speed (4 or 16 mbits/sec.).  */
  int		   boot_ring_speed;      /* Ring speed used by firmware       */
  int              num_bus_attrs;        /* Number of bus attributes          */
  char		   parms[256];		 /* place to put boot parameters      */

  DEBUG_0("build_dds(): BEGIN build_dds()\n")

  *dds_length = sizeof(itok_ddi_t);
  *dds_ptr = (uchar *) &ddi_ptr;

  /* set default value for all attributes to zero */
  bzero(&ddi_ptr,sizeof(itok_ddi_t));

  /* Get attribute list */
  if ((alist=get_attr_list(logical_name,cudv.PdDvLn_Lvalue,&how_many,16))==NULL)
    return(E_ODMGET);

  /* Regular Attributes */

  if (rc=GETATT(&ddi_ptr.shared_mem_leng,'i',"shared_mem_leng"))
    return(rc);
  DEBUG_1("ddi_ptr.shared_mem_leng = %x\n",ddi_ptr.shared_mem_leng)
  if (rc=GETATT(&ddi_ptr.intr_level,'i',"intr_level"))
    return(rc);
  if (rc=GETATT(&ddi_ptr.intr_priority,'i',"intr_priority"))
    return(rc);
  if (rc=GETATT(&ddi_ptr.xmt_que_size,'i',"xmt_que_size"))
    return(rc);
  if (rc=GETATT(&ddi_ptr.io_addr,'a',"bus_io_addr"))
    return(rc);
  if (rc=GETATT(&ddi_ptr.bios_addr,'i',"bios_addr"))
    return(rc);
  DEBUG_1("ddi_ptr.bios_addr = %x\n",ddi_ptr.bios_addr)
  if (rc=GETATT(&ddi_ptr.shared_mem_addr,'i',"shared_mem_addr"))
    return(rc);
  DEBUG_1("ddi_ptr.shared_mem_addr = %x\n",ddi_ptr.shared_mem_addr)

#ifdef NEED_CHG
  /* this check not required as it is performed by bus resolve */
  /* the device driver will need to make sure info in DDS is   */
  /* matching the jumpers on the adapter                       */


  /* Check that the ROM and RAM areas don't overlap.                        */
  /* We cannot at this time do a complete check.. we don't know the         */
  /* width of the RAM yet, min. size is 8K, the size of the ROM is always   */
  /* 8K, so just assume that the RAM is 8K wide .. the device driver itself */
  /* must catch overlap problems for RAM size > 8K.                         */
  if (ddi_ptr.shared_mem_addr >= ddi_ptr.bios_addr) {
    if ((ddi_ptr.shared_mem_addr - ddi_ptr.bios_addr) < 0x2000)
      return(E_BADATTR);
  } else {
      if ((ddi_ptr.bios_addr - ddi_ptr.shared_mem_addr) < 0x2000)
        return(E_BADATTR);
    }
#endif
      
  if (rc=GETATT(tmp_str,'s',"use_alt_addr"))
    return(rc);
  ddi_ptr.use_alt_addr = (tmp_str[0] == 'y')? 1 : 0;
  if (ddi_ptr.use_alt_addr) {
    rc=GETATT(ddi_ptr.alt_addr,'b',"alt_addr");
    if (byte_cnt != 6) {
      if (rc > 0)
        return rc;
      else {
	DEBUG_1("build_dds(): getatt() bytes=%d\n",rc)
	return(E_BADATTR);
      }
    }
    DEBUG_6("build_dds():alt_addr=%02x %02x %02x %02x %02x %02x\n",
	  (int) ddi_ptr.alt_addr[0],(int) ddi_ptr.alt_addr[1],
	  (int) ddi_ptr.alt_addr[2],(int) ddi_ptr.alt_addr[3],
	  (int) ddi_ptr.alt_addr[4],(int) ddi_ptr.alt_addr[5])
  }

  /* Translated Attributes */
  if (rc = GETATT(tmp_str,'s',"attn_mac"))
    return(rc);
  ddi_ptr.attn_mac = (tmp_str[0] == 'y')? 1 : 0;
  DEBUG_1("ddi_ptr.attn_mac = %x\n",ddi_ptr.attn_mac)
  if (rc = GETATT(tmp_str,'s',"beacon_mac"))
    return(rc);
  ddi_ptr.beacon_mac = (tmp_str[0] == 'y')? 1 : 0;
  DEBUG_1("ddi_ptr.beacon_mac = %x\n",ddi_ptr.beacon_mac)

  /* Attributes from other Object Classes/Places */
  /* Get bus id */
  if ((bus_attr_list = get_attr_list(pcudv.name, pcudv.PdDvLn_Lvalue,
                                           &num_bus_attrs, 4)) == NULL)  
    return(E_ODMGET);   
  if (rc = getatt(bus_attr_list, "bus_id", &ddi_ptr.bus_id, 'l'))
    return(rc);  


  /*
   *  Check to see if this device is the boot device.  If it is, get
   *  ring speed from boot information.  Set ODM to this ring speed.
   *  If device is not boot device, use ring speed from ODM.
   */
  DEBUG_0("build_dds(): checking ring_speed\n")

  /* Get ring speed from ODM database.  */
  if (!(CuAt_obj=getattr(logical_name,"ring_speed",FALSE,&how_many)))
    return(E_ODMGET);

  ring_speed = atoi(CuAt_obj->value);

  /* Get boot parameters */
  /* parms will be a NULL string if this is not the boot device */
  if (!get_bootparms(&cudv,&pddv,&parms)) {

	/* boot_ring_speed will be 0 if this is not the boot device */
	boot_ring_speed = (int)strtoul(parms,NULL,16);

	if (boot_ring_speed != 0 && boot_ring_speed != ring_speed) {
		/* ODM ring speed does not match boot ring speed */
		ring_speed = boot_ring_speed;
		DEBUG_0("build_dds(): resetting ring_speed in ODM\n")
		sprintf(CuAt_obj->value,"%d",ring_speed);
		if (putattr(CuAt_obj)<0)
			return(E_ODMUPDATE);

	}
  }

  /* Now put ring speed value into DDS */
  if (ring_speed == 4)
    ddi_ptr.ring_speed = 0;
  else
    ddi_ptr.ring_speed = 1;

  /* Put the logical name in the DDI for use by error logging */
  strcpy(ddi_ptr.lname,logical_name);

  /* create alias name, "tr" appended with sequence number */
  rc = strncmp(pddv.prefix,logical_name,
		   strlen(pddv.prefix));

  strcpy(ddi_ptr.alias,"tr");
  if (rc == 0) {
    /* logical name created from prefix, copy only extension */
    strcat(ddi_ptr.alias,logical_name+strlen(pddv.prefix));
  } else {
      /* logical name not created from prefix, append entire string */
      strcat(ddi_ptr.alias,logical_name);
    }
  DEBUG_1("build_dds(): alias name is %s\n",ddi_ptr.alias)

#ifdef CFGDEBUG
	hexdump(ddi_ptr,(long) sizeof(itok_ddi_t));
#endif

  return(0);
}

/*
 * NAME:
 *	download_microcode()
 *
 * FUNCTION:
 *	This downloads microcode to a token ring adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int download_microcode(char            *logical_name,
                       struct cfg_kmod *cfg_k)
{
  DEBUG_0("download_microcode(): not required\n")
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
              mid_t           k_mid,
              dev_t           devno,
              char            *vpd)
{
  itok_vpd_t   vpd_area;		/* Storage area for VPD */
  ulong	       ul;
  struct cfg_kmod cfg_k;          /* sysconfig command structure */
  ndd_config_t    ndd_config;     /* devices cfg init struct */
	char    *p_vpd;         /* temporary pointer to copy vpd */
	char    *p_adap_vpd;    /* pointer to adapters vpd */

  DEBUG_0("query_vpd(): BEGIN query_vpd()\n")

  /* Initialize the cfg structure for a call to sysconfig(), which */
  /* will in turn call the ddconfig() section of the appropriate	 */
  /* device driver.						 */

  ndd_config.seq_number = devinst;
  ndd_config.p_vpd = (caddr_t) &vpd_area;   /* Storage space for VPD */
  ndd_config.l_vpd = sizeof(vpd_area);      /* Size of VPD storage area */
  cfg_k.mdiptr = (char *) &ndd_config;
  cfg_k.mdilen = sizeof (struct ndd_config);
  cfg_k.kmid = kmid;
  cfg_k.cmd = CFG_QVPD;             /* Command to read VPD */

  /* Make the call to sysconfig: */
  if (sysconfig(SYS_CFGKMOD, &cfg_k, sizeof(struct cfg_kmod))<0) {
#ifdef CFGDEBUG
    DEBUG_0("query_vpd(): Dump of vpd_area\n")
    hexdump(&vpd_area,(long) sizeof(vpd_area));
    switch(errno) {
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
   return E_VPD;
  }

#ifdef CFGDEBUG
	DEBUG_1("query_vpd(): status=%d     ",(int) vpd_area.status)
	switch((int) vpd_area.status) {
	case TOK_VPD_INVALID:
		DEBUG_0("query_vpd(): status=VPD INVALID\n")
		break;
	case TOK_VPD_VALID:
		DEBUG_0("query_vpd(): status=VPD VALID\n")
		break;
	default:
		DEBUG_0("query_vpd(): status=?\n")
		break;
	}
	DEBUG_0("query_vpd(): Dump of vpd\n")
	/* hexdump(vpd_area.vpd,(long) vpd_area.vpd_len); */
	hexdump(vpd_area.vpd,(long) 6);
#endif

  /* the vpd returned should be a pointer to 6 bytes of Network address */
	/* Store the VPD in the database */
	/*     convert to proper format */
	p_vpd = vpd;
	p_adap_vpd = vpd_area.vpd;

	/*    copy in Network Address */
	strncpy(p_vpd,"*NA",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x05;  /* length = 10 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x0,6);
	p_vpd = p_vpd + 6;

	/*    copy in a Brief Description */
	strncpy(p_vpd,"*DS",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x9;  /* length = 18 characters */
	p_vpd = p_vpd + 1;

	strncpy(p_vpd,"ISA Token Ring",14);
	p_vpd = p_vpd + 14;

	DEBUG_0("query_vpd(): Dump of vpd to put in database\n")

#ifdef CFGDEBUG
	hexdump(vpd,28);
#endif

  return(0);
}

/*
 * NAME:
 *	define_children()
 *
 * FUNCTION:
 *	There are no children.
 *
 * RETURNS:
 *	Always returns 0 - Success.
 */

int define_children(char *logical_name,
                    int           phase)
{
  DEBUG_0("define_children(): NULL function, returning 0\n")
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
 *      generic configure method for all devices. This is a null
 *      function.
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
