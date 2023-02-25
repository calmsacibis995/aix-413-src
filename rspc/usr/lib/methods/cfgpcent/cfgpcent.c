static char sccsid[] = "@(#)95  1.6  src/rspc/usr/lib/methods/cfgpcent/cfgpcent.c, pcmciaent, rspc41J, 9516A_all 4/18/95 17:56:54";
/*
static char sccsid[] = "@(#)57  1.4  src/rspc/usr/lib/methods/cfgisaent/cfgisaent.c, isaent, rspc41J 2/23/95 19:05:19";
 */
/*
 *   COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet device driver
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
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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
#include <sys/cdli_entuser.h> 
#ifdef PCMCIAENT
#include <ent/ient_ddi.h>
#include <sys/pcmciacs.h>
#else
#include <ient/ient_ddi.h>
#endif
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
  ddi_t       *ddi_ptr;                       /* DDS pointer            */
  char	           *parent=NULL, *location=NULL;
  struct CuDv      cust_obj, bus_obj;
  struct CuAt      *CuAt_obj;	        /* Customized Attributes data struct */
  struct PdAt      PA_obj;	 /* Customized Attributes data struct */
  struct attr_list *bus_attr_list = NULL;/* Bus attributes list.             */
  char             *parentdesc, srchstr[256],
                   utype[UNIQUESIZE], *ptr,
                   tmp_str[5];          /* yes,no value for flag fields      */
  int		   i,rc;		/* flags for getting parent attrib   */
  int              bus_num;             /* Bus number                        */
  int              num_bus_attrs;       /* Number of bus attributes          */
#ifdef PCMCIAENT
  struct CuDv      bus_obj2, cust_obj_pcm;
  int              devno;                /* parent's device number            */
  CSMapSktPkt      skt_pkt;              /* for useing svcCardServices()      */
  long             major_no;             /* major number of pcmcia adapter    */
  long             minor_no;             /* minor number of pcmcia adapter    */
  char             sstring[256];         /* search criteria string            */
#endif

  DEBUG_0("build_dds(): BEGIN build_dds()\n")

  *dds_length = sizeof(ddi_t);
  if ((ddi_ptr = (ddi_t *) malloc(sizeof(ddi_t))) ==
          (ddi_t *) NULL) {
    DEBUG_0 ("build_dds(): Malloc of dds failed\n")
    return E_MALLOC;
  }
  *dds_ptr = (uchar *) ddi_ptr;

  /* set default value for all attributes to zero */
  bzero(ddi_ptr,sizeof(ddi_t));

  /* get unique type from customized OC */
  sprintf(srchstr,"name = '%s'",logical_name);
  if ((rc=(int)odm_get_first(CuDv_CLASS,srchstr,&cust_obj)) == 0)
    return E_NOCuDv;
  else if (rc == -1)
    return E_ODMGET;

  strcpy(utype,cust_obj.PdDvLn_Lvalue);
#ifdef PCMCIAENT
  /* get information from parent */
  if(cust_obj.parent[0] == '\0'){
    DEBUG_0("build_dds():: error getting parent name \n")
    return E_PARENT;
  }

  /* get customized object of parent  */
  sprintf(sstring,"name = '%s'",cust_obj.parent);
  rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cust_obj_pcm,TRUE);
  if( rc == 0 ) {
    DEBUG_0("build_dds(): parent object not found\n")
    return E_NOCuDvPARENT;
  }
  else if(rc == -1){
    /* odm error occured */
    DEBUG_0("build_dds(): get_obj failed \n")
    return E_ODMGET;
  }
  /*  Get slot  */
  /* Note device driver uses slot numbers 0,1.. connwhere is 1,2.. */
  strcpy(srchstr,cust_obj.connwhere);
  DEBUG_1("build_dds(): connwhere=%s\n",srchstr)
  if((rc = convert_att( &(ddi_ptr->slot), 'i', cust_obj.connwhere, 'n')) != 0)
      return(rc);
  /* decrement connwhere to get hardware slot number */
  ddi_ptr->slot--;
  DEBUG_1("build_dds(): slot=%d\n",ddi_ptr->slot)

  /* Generate Parent's devno */
  DEBUG_1("ddins=%s\n", cust_obj_pcm.ddins);
  major_no = genmajor( cust_obj_pcm.ddins );
  minor_no = *getminor( major_no, &how_many, cust_obj_pcm.name );       
  devno = makedev( major_no, minor_no );
  DEBUG_4("maj = %d, min = %d, devno = %x, slot = %x\n",
  major_no, minor_no, devno, ddi_ptr->slot);

  /* Generate logical socket number */
  skt_pkt.PhyAdapter = devno;
  skt_pkt.PhySocket  = ddi_ptr->slot;          /* == CuDv.connwhere - 1 */
  if( !svcCardServices(CSMapPhySocket,NULL,NULL,sizeof(CSMapSktPkt), &skt_pkt)) {
    ddi_ptr->logical_socket = skt_pkt.LogSocket;
    DEBUG_1("build_dds(): logical_socket = %ld\n", ddi_ptr->logical_socket )
  }
  else {
    DEBUG_2("build_dds(): svcCardServices() ERROR !!  devno = %x, slot = %x\n", skt_pkt.PhyAdapter, skt_pkt.PhySocket )
    return E_DEVSTATE;
  }
#endif

  /* Get attribute list */
  if ((alist=get_attr_list(logical_name,utype,&how_many,16)) == NULL)
    return(E_ODMGET);

  /* Regular Attributes */

  if (rc=GETATT(&ddi_ptr->intr_level,'i',"bus_intr_lvl"))
    return(rc);
  if (rc=GETATT(&ddi_ptr->intr_priority,'i',"intr_priority"))
    return(rc);
  if (rc=GETATT(&ddi_ptr->xmt_que_size,'i',"xmt_que_size"))
    return(rc);
  if (rc=GETATT(&ddi_ptr->io_addr,'a',"bus_io_addr"))
    return(rc);
  if (rc=GETATT(&ddi_ptr->shared_mem_addr,'i',"bus_mem_addr"))
    return(rc);
  if(rc=GETATT(tmp_str,'s',"media_type"))
    return(rc);
  if (tmp_str[0] == 't') 	/* twisted-pair */
    ddi_ptr->media_type = 0;
  else {
  	if (tmp_str[0] == 'b')  /* bnc */
    		ddi_ptr->media_type = 1;
	else
    		ddi_ptr->media_type = 2;	/* dix */
  }

  if (rc=GETATT(tmp_str,'s',"use_alt_addr"))
    return(rc);
  ddi_ptr->use_alt_addr = (tmp_str[0] == 'y')? 1 : 0;
  if (ddi_ptr->use_alt_addr) {
    rc=GETATT(ddi_ptr->alt_addr,'b',"alt_addr");
    if (byte_cnt != 6) {
      if (rc > 0)
        return(rc);
      else {
	DEBUG_1("build_dds(): getatt() bytes=%d\n",rc)
	return(E_BADATTR);
      }
    }
    DEBUG_6("build_dds():alt_addr=%02x %02x %02x %02x %02x %02x\n",
	  (int) ddi_ptr->alt_addr[0],(int) ddi_ptr->alt_addr[1],
	  (int) ddi_ptr->alt_addr[2],(int) ddi_ptr->alt_addr[3],
	  (int) ddi_ptr->alt_addr[4],(int) ddi_ptr->alt_addr[5])
  }

  /* Attributes from other Object Classes/Places */
  /* Get bus type, bus id, and bus number.  */
#ifdef PCMCIAENT
  rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj2); 
  DEBUG_1( "parent = %s\n", cust_obj.parent );
  if (rc) {
    if (rc == E_PARENT) 
      rc = E_NOCuDvPARENT;
    return(rc);
  }
  rc = Get_Parent_Bus(CuDv_CLASS, bus_obj2.parent, &bus_obj); 
  DEBUG_1( "grandparent = %s\n", bus_obj2.parent );
  if (rc) {
    if (rc == E_PARENT) 
      rc = E_NOCuDvPARENT;
    return(rc);
  }
#else
  rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj);
  if (rc) {
    if (rc == E_PARENT)
      rc = E_NOCuDvPARENT;
    return(rc);
  }
#endif
  if ((bus_attr_list = get_attr_list(bus_obj.name, bus_obj.PdDvLn_Lvalue,
                                           &num_bus_attrs, 4)) == NULL)  
    return(E_ODMGET);   
  if (rc = getatt(bus_attr_list, "bus_id", &ddi_ptr->bus_id, 'l'))
    return(rc);
#ifdef PCMCIAENT
  ddi_ptr->bus_num = bus_num = bus_obj2.location[3] - '0';
  DEBUG_1( "bus_num = %x\n", bus_num );
#else
  bus_num = bus_obj.location[3] - '0';
#endif

#ifdef PM_SUPPORT
  if (rc=GETATT(&ddi_ptr->pm_dev_itime,'i',"pm_dev_itime"))
    return(rc);
  if (rc=GETATT(&ddi_ptr->pm_dev_stime,'i',"pm_dev_stime"))
    return(rc);
#endif /* PM_SUPPORT */

  /* Put the logical name in the DDI for use by error logging */
  strcpy(ddi_ptr->lname,logical_name);

  /* create alias name, "en" appended with sequence number */
  rc = strncmp(pddv.prefix,logical_name,
		   strlen(pddv.prefix));

  strcpy(ddi_ptr->alias,"en");
  if (rc == 0) {
    /* logical name created from prefix, copy only extension */
    strcat(ddi_ptr->alias,logical_name+strlen(pddv.prefix));
  } else {
      /* logical name not created from prefix, append entire string */
      strcat(ddi_ptr->alias,logical_name);
    }
  DEBUG_1("build_dds(): alias name is %s\n",ddi_ptr->alias)

#ifdef CFGDEBUG
	hexdump(ddi_ptr,(long) sizeof(ddi_t));
#endif

  return(0);
}

/*
 * NAME:
 *	download_microcode()
 *
 * FUNCTION:
 *	This downloads microcode to a ethernet ring adapter.
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
              mid_t		k_mid,
              dev_t		devno,
              char            *vpd)
{
  ibm_isa_vpd_t vpd_area;		/* Storage area for VPD */
  ulong	        ul;
  struct cfg_kmod cfg_k;          /* sysconfig command structure */
  ndd_config_t ndd_config;


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
	DEBUG_0("query_vpd(): Dump of vpd\n")
	hexdump(vpd_area.vpd,(long) vpd_area.length);
#endif

  /* Store the VPD in the database */
  put_vpd(vpd,vpd_area.vpd,vpd_area.length);
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
 *      generic configure method for all devices. For the time being, as
 *      there is not any vpd, this a null function.
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
  DEBUG_0("device_specific(): NULL function, returning 0\n")
  return(0);
}
