static char sccsid[] = "@(#)31  1.9  src/rspc/usr/lib/methods/cfgidedisk/cfgidedisk.c, rspccfg, rspc41J, 9519A_all 5/8/95 23:17:09";
/*
 * COMPONENT_NAME: (RSPCCFG) IDE Disk config method
 *
 * FUNCTIONS:
 *		generate_minor
 *		make_special_files
 *		issue_identify_drive
 *		query_vpd
 *		build_dds
 *		chkmodel
 *		disk_present
 *		get_pvidstr
 *		get_pvid
 *		read_ide_pvid
 *		idevpd
 *		get_vpd_value
 *		get_ident_data
 *		get_tokens
 *		get_dev_id
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */
#define _KERNSYS
#define _RS6K_SMP_MCA
#include	<sys/systemcfg.h>
#undef _RS6K_SMP_MCA
#undef _KERNSYS
#include	<stdio.h>
#include	<fcntl.h>
#include	<malloc.h>
#include	<sys/errno.h>
#include	<sys/types.h>
#include	<sys/device.h>
#include	<sys/sysmacros.h>
#include	<sys/cfgdb.h>
#include	<sys/stat.h>
#include	<sys/bootrecord.h>
#include	<sys/utsname.h>
#include	<sys/mdio.h>
#include	<string.h>
#include	<sys/cfgodm.h>
#include	<sys/pm.h>
#include	<cf.h>
#include	"cfgdebug.h"
#include	"cfgtoolsx.h"
#include	"idediskdd.h"

/*
 *---------------------------------------------------------------
 * The following defines are used in get_vpd_value to identify
 * the offsets to place the various parts of a formatted VPD
 * value.
 *---------------------------------------------------------------
 */
#define STAR_OFFSET		0
#define KEYWORD_OFFSET		1
#define LENGTH_OFFSET		3
#define DATA_OFFSET		4
#define VPD_OVERHEAD		DATA_OFFSET

/*
 *---------------------------------------------------------------
 * Misc. defines
 *---------------------------------------------------------------
 */
#define NULLPVID		"00000000000000000000000000000000"
#define BUF_SIZE		255
#define	IDENTSIZE		512
#define	SEG_SIZE		1000000		/* statistics segment size */
#define	MAX_REQUEST		0x20000		/* Max # bytes / request   */
#define	MODEL_NAME_LEN		40		/* # bytes in model name */
#define	GENERIC_DISK		"Generic_IDE_Disk                        "
#define	CALL_FINDDISK		-1

/*
 *---------------------------------------------------------------
 * Static variables
 *---------------------------------------------------------------
 */
static	uchar			id_data[IDENTSIZE];	/* identify drive */
							/* data storage area */
static	uchar			first_id_drv_request = TRUE;

static  uchar			pvid_str[33];

static	struct idedisk_dds	dds_buf;

/*
 *---------------------------------------------------------
 * Forward function definitions
 *---------------------------------------------------------
 */
static	void	get_vpd_value (uchar*, uchar*, char*, short*, char) ;
int		get_ident_data (struct CuDv*, uchar*, int*);
static	int	get_tokens (char*, char*, short*, short*, char*) ;
int		get_dev_id (char*, uchar*);
char		*pvidtoa();
void		get_pvid();

/*
 ***********************************************************************
 * NAME: generate_minor
 *
 * FUNCTION: Generates a minor number for the IDE disk being configured
 *
 * EXECUTION ENVIRONMENT:
 *	This function is part of the config method for the IDE
 *	disk.  It executes whenever an IDE disk is being configured
 *	into the system. This can be at IPL or run time.
 *
 * NOTES:
 *	This function uses the genminor() function to reserve the
 *	minor number used for this IDE disk. The reservation is
 *	logged in the configuration database by genminor().
 *
 *	generate_minor() is passed the logical name of the device
 *	that is being configured, and the device's major number.
 *	These values are used to log the minor numbers in the data-
 *	base.
 *
 *	This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *	0		- success (minor number stored via the last parameter)
 *	E_MINORNO	- unable to get a minor number for this major number
 ***********************************************************************
 */

long
generate_minor( logical_name, major_no, minor_dest )
	char	*logical_name;     /* Name of device                       */
	long	major_no;          /* Major no. already generated          */
	long	*minor_dest;       /* Destination for minor number         */
{
	long	*genret;
	long	*genminor();

	/*
 	* Request a minor number. Only one number is required, and there is
 	* no preferred number, nor is there a boundary
 	*/
	DEBUG_0( "generate_minor()\n" )

	if ((genret = genminor(logical_name, major_no,-1,1,1,1)) == NULL )
		return(E_MINORNO);

	*minor_dest = *genret;

	return 0;
}

/*
 ***********************************************************************
 * NAME: make_special_files
 *
 * FUNCTION: Generates two special files for the IDE disk being configured
 *
 * EXECUTION ENVIRONMENT:
 *	This function is part of the config method for the IDE
 *	disk. It executes whenever an IDE disk is being configured
 *	into the system.  This can be at IPL or run time.
 *
 * NOTES:
 *	This function creates both the raw and block device special
 *	files used to communicate with the IDE disk driver. It
 *	calls the function mk_sp_file() to actually create the special
 *	files.  The files generated are /dev/<logical_name>, and 
 *	/dev/r<logical_name>.
 * 
 *	This function gets passed the device's logical name (which
 *	is the basis for the special file name), and the devno of
 *	the device.
 *
 *	This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *	0		- the special files were made succesfully.
 *	E_MKSPECIAL	- the special files can't be made.
 ***********************************************************************
 */

int
make_special_files( logical_name, devno )
	char	*logical_name;
	dev_t	devno;
{
	char	basename[NAMESIZE+1];
	int	rc;

	DEBUG_0( "make_special_files()\n" )

	/* Generate block file */
	rc = mk_sp_file( devno, logical_name, (S_IRUSR | S_IWUSR | S_IFBLK));
	if (rc)
		return(rc);

	/* Create the character special file */
	sprintf( basename, "r%s", logical_name );
	rc = mk_sp_file( devno, basename, (S_IRUSR | S_IWUSR | S_IFCHR));

	return(rc);
}

/*
 ***********************************************************************
 * NAME: issue_identify_drive
 *
 * FUNCTION: Performs the low level work to get the drive
 *	identification data from the device.
 *
 * EXECUTION ENVIRONMENT:
 *	This is an internal subroutine called only from within
 *	the IDE disk configuration method.
 *
 * NOTES:
 *	This routine expects the adapter to be opened and started
 *	by the caller.
 *
 * RETURNS:
 *	0		- successful
 *	E_DEVACCESS	- adapter's IDEIOIDENT ioctl command failed
 ***********************************************************************
 */

int
issue_identify_drive(
	int	adapfd,
	uchar	*iddata,
	uchar	dev_id,
	int	*error)
{
struct	ide_identify	ident;
	ushort		tshort;
	int		try;
	int		rc;
	uint		i;

/* BEGIN issue_identify_drive */

	if (!first_id_drv_request) {
		/*
		 * Don't actually issue the command to device if
		 * this is not the first request.
		 */
		return(0);
	}

	/* Zero out ide_identify command structure */
	bzero(&ident, sizeof(struct ide_identify));

	*error = 0;

	/* build identify drive command structure */
	ident.ide_device = dev_id;
	ident.identify_len = 512;		/* always 1 sector returned */
	ident.identify_ptr = iddata;
	ident.ata_atapi = 0;
	ident.flags = 0;

	for (try=0; try<3; try++)
	{
		rc = ioctl(adapfd, IDEIOIDENT, &ident);
		if (rc == 0)
		{
			first_id_drv_request = FALSE;
			for ( i=0; i < ident.identify_len; i=i+2 ) {
				tshort =  *((uchar *)(iddata+i)) |
					 (*((short *)(iddata+i))<<8);
				*((short *)(iddata+i)) = tshort;
			}
			return(0);	/* Successful completion! */
		}

	} /* END for loop */

	if (errno == ENODEV) 
		*error = ENODEV;
	else
		*error = -1;

	return(E_DEVACCESS);

} /* END issue_identify_drive */

/*
 ***********************************************************************
 * NAME: query_vpd
 *
 * FUNCTION: Obtains the VPD data, formats the data and
 *           the VPD to cfgdisk.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *	0	- succeeded
 *	E_VPD	- could not obtain VPD data
 ***********************************************************************
 */

int
query_vpd( cusobj, kmid, devno, vpd_dest )
struct  CuDv    *cusobj;        /* Pointer to customized object rec     */
mid_t   kmid;                   /* kernel module I.D. for Dev Driver    */
dev_t   devno;                  /* Concatenated Major & Minor No.s      */
char    *vpd_dest;              /* Area to place VPD                    */
{
	char	fmt_str[] = "SN1414C,RL2e08C,TM3628C";
	int	vpd_len;
	int	rc;

	/* BEGIN query_vpd */

	vpd_len = idevpd(cusobj, vpd_dest, fmt_str) ;
	if (vpd_len > 0)
	{
		/* vpd_len > 0 is good, got some/all VPD data */
#ifdef CFGDEBUG
		hexdump(vpd_dest, vpd_len) ;
#endif
		rc = 0 ;
	}
	else
	{
		DEBUG_0("query_vpd: No VPD received from idevpd\n") ;
		rc = E_VPD ;
	}
	return(rc);

} /* END query_vpd */


/*
 ***********************************************************************
 * NAME: build_dds
 *
 * FUNCTION: Device dependent DDS build for IDE disks.
 *
 * EXECUTION ENVIRONMENT:
 *	This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *	E_OK		- success
 *	E_BADATTR	- attribute value conversion failed
 *	E_DEVACCESS	- adapter's IDEIOIDENT ioctl command failed
 *	E_INVCONNECT	- disk's connwhere is a null string
 *	E_MALLOC	- insufficient storage to malloc space for CuAt objects
 *	E_NOATTR	- Unable to find specified attribute
 *	E_ODMADD	- Unable to add CuAt object
 *	E_ODMGET	- failed to access ODM database
 *	E_ODMUPDATE	- failure changing CuAt object
 ***********************************************************************
 */

int
build_dds( cusobj, parobj, dds_data_ptr, dds_len )
struct	CuDv	*cusobj;	/* Disk's CuDv object 			*/
struct	CuDv	*parobj;	/* Disk's parent's CuDv object 		*/
	char	**dds_data_ptr;	/* Address to store pointer to DDS	*/
	int	*dds_len;	/* Address to store length of DDS	*/

{

struct	idedisk_dds	*dds;		/* DDS for IDE disk in idediskdd.h*/
	int		rc;		/* Return code			  */
	long		major_no;	/* Major number of adapter	  */
	long		minor_no;	/* Minor number of adapter	  */
struct	attr_list	*alist;
	char		addr_mode[12];
	char		sstring[256];
struct	PdAt		preattr;
struct	CuAt		cuattr;
struct	identify_device	*ident;
	int		how_many;
	int		bc;
	uchar		dev_id;
	long		*getminor();
	long		genmajor();
	int		i, id_len;

	DEBUG_0( "build_dds()\n" )

	dds = &dds_buf;

	DEBUG_1("build_dds: dds address = %x\n",(int)dds)

	/* Zero out dds structure */
	bzero(dds, sizeof(struct idedisk_dds));

	/* put device name into DDS */
	strncpy(dds->resource_name, cusobj->name, sizeof(dds->resource_name));

	/* initialize some DDS fields */
	/* These are hard coded here rather than in the PdAt database */
	/* to save save disk space */
	dds->segment_cnt = 0;
	dds->byte_count = 0;
	dds->segment_size = SEG_SIZE;
	dds->max_request = MAX_REQUEST;

	/* Read the attributes from the customized & predefined classes */
	alist = (struct	attr_list *) get_attr_list(cusobj->name,
						   cusobj->PdDvLn_Lvalue,
						   &how_many,10);
	if (alist == (struct attr_list *) NULL) {
		if (how_many == 0)
			return(E_NOATTR);
		else
			return(how_many);
	}

	/* 
	 * Get identification data for the disk and save it in a
	 * static buffer so that the query_vpd() routine can make use of it.
	 */
	if (rc = get_ident_data(cusobj,id_data,&id_len))
		return(rc);

	rc = chkmodel(id_data,cusobj->PdDvLn_Lvalue,(char *)cusobj->name);

	/*
	 * use (struct identify_device *) to ease access
	 */

	ident = (struct identify_device *) id_data;

	/*
	 * Determine if the device supports DMA.  Bit 8 of the data word
	 * at byte offset 98 in the drive identification data defines this
	 * capability (1 => DMA supported, 0 => PIO only).
	 */

	if ( ident->capabilities & ID_CAP_DMA ) {
		dds->dma_supported = TRUE;
		dds->read_cmd = ATA_READ_DMA_RETRY;
		dds->write_cmd = ATA_WRITE_DMA_RETRY;
	} else {
		dds->dma_supported = FALSE;
		dds->read_cmd = ATA_READ_SECTOR_RETRY;
		dds->write_cmd = ATA_WRITE_SECTOR_RETRY;
	}

	/*
	 * Determine whether the device supports LBA.
	 * (1 => LBA supported, 0 => CHS addressing only).
	 */

	DEBUG_1 ("Checking supported addressing modes: %x\n",
			ident->capabilities)

	if ( ident->capabilities & ID_CAP_LBA ) {

		/*
		 * LBA is supported:
		 */

		dds->chs_only = FALSE;
		DEBUG_0("\t> CHS or LBA\n");
	} else {

		/*
		 * LBA is NOT supported:
		 */

		dds->chs_only = TRUE;
		DEBUG_0("\t> CHS only\n");
	}

	/*
	 * Get the number of sectors per track and number of heads from
	 * the drive identification data.
	 */
	dds->num_heads = (uchar) (ident->num_of_heads & 0x00ff);
	dds->sectors_per_track = (uchar) (ident->sectors_per_track & 0x00ff);
	DEBUG_2("Device's # heads = %d, sectors/track = %d\n", dds->num_heads,
		dds->sectors_per_track)
	dds->total_sectors = (uint) (ident->num_of_cyl * dds->num_heads *
					dds->sectors_per_track);
	DEBUG_1("Total # of sectors = 0x%x\n",dds->total_sectors)

	/*
	 * For certain attributes, assume a default value if no PdAt exists.
	 */

	if ((rc=getatt(alist,"start_timeout",&dds->start_timeout,'h',
		       &bc)) == E_NOATTR) {
		/*
		 * Do not fail if start unit timeout is not in ODM, just assume
		 * default.
		 */
		dds->start_timeout = DK_START_TIMEOUT;
	} else if (rc != 0) {
		/* 
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}
		
	if ((rc=getatt(alist,"rw_timeout",&dds->rw_timeout,'h',
		       &bc)) == E_NOATTR) {
		/*
		 * Do not fail if read/write attribute is not in ODM, just 
		 * assume default.
		 */
		dds->rw_timeout = DK_TIMEOUT;
	} else if (rc != 0) {
		/* 
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}

        if ((rc=getatt(alist,"block_size",&dds->block_size,'i',&bc))
            == E_NOATTR) {
                /*
                 * Do not fail if block_size is not in ODM, just assume
                 * default.
                 */
                dds->block_size = DK_BLOCKSIZE;
        } else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
	
        if ((rc=getatt(alist,"max_coalesce",&dds->max_coalesce,'i',&bc))
            == E_NOATTR) {
                /*
                 * Do not fail if max_coalesce is not in ODM, just assume
                 * default
                 */
                dds->max_coalesce = 0x20000;
        } else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }

	/* device idle time */
	if ((rc=getatt(alist,"pm_dev_itime",&dds->pm_dev_itime,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_itime = 0.
		 */
		dds->pm_dev_itime = 0;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}

	/* device standby time */
	if ((rc=getatt(alist,"pm_dev_stime",&dds->pm_dev_stime,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_stime = 0.
		 */
		dds->pm_dev_stime = 0;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}

	/* device attribute */
	if ((rc=getatt(alist,"pm_dev_att",&dds->pm_dev_att,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_att = 0. (no attribute)
		 * If no attribute, it is handled as PM_OTHER_DEVICE.
		 */
		dds->pm_dev_att = PM_OTHER_DEVICE;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}

	/* Get device's unique ID and save in DDS */
	if ( get_dev_id( cusobj->connwhere, &dev_id) )
                return( E_INVCONNECT );
	dds->device_id = dev_id;

	/* Generate Parent's devno */
	major_no = genmajor( parobj->ddins );
	minor_no = *getminor( major_no, &how_many, parobj->name );	
	dds->adapter_devno = makedev( major_no, minor_no );
	
	DEBUG_1( "dds->adapter_devno = %ld\n", dds->adapter_devno)
		
	*dds_data_ptr = (char *)dds;		 /* Store address of struct*/
	*dds_len = sizeof( struct idedisk_dds ); /* Store size of structure*/

#ifdef CFGDEBUG
	hexdump(*dds_data_ptr, (long)*dds_len);
#endif

	DEBUG_0( "build_dds() returning 0\n" )

	return(E_OK);
}

/*
 ***********************************************************************
 * NAME: chkmodel
 *
 * FUNCTION: Validates that the model number reported by the 
 *	identify drive command agrees with the model_name
 *	attribute in the PdAt.
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This is an internal subroutine called by disk_present()
 *    in the device configuration methods.
 *
 * RETURNS:
 *	0		- success
 *	E_ODMGET	- couldn't access ODM 
 *	E_ODMUPDATE	- couldn't update ODM CuAt object
 *	E_WRONGDEVICE	- identification data type does not match PdAt's
 ***********************************************************************
 */

int
chkmodel(id_data,utype,lname)
	uchar	*id_data;			/* ptr to identification data */	
	char	*utype;				/* ptr to device's uniquetype */
	char	*lname;                         /* disk's logical name */
{
        int     	i,rc;
struct	PdAt		PdAt;
struct	CuAt		cuattr;
	char		*tmpstr;
        char    	srch_str[256];
	char		dev_model[42];

	/* search for the model_name attribute for this device  */

	sprintf(srch_str, "uniquetype='%s' AND attribute=model_name", utype);
	rc = (int) odm_get_first(PdAt_CLASS, srch_str, &PdAt);

	if ( rc == -1 ) {
		/* ODM failure */
		DEBUG_0("ODM error getting PdAt info\n")
		return(E_ODMGET);
	}

	tmpstr = (char *) &(((struct identify_device *) id_data)->model_number);
	strncpy ( dev_model, tmpstr, MODEL_NAME_LEN );
	i = MODEL_NAME_LEN;
	dev_model[i] = '\0';
	/*
	 * Remove trailing blanks from disk's model name, use at lease 20
	 * characters.
	 */
	while (((dev_model[--i] == ' ') || (dev_model[i] == 0)) && (i > 19))
		dev_model[i] = '\0';

        /* See if the disk's model name matches the one from the PdAt */
        if (strncmp(PdAt.deflt,dev_model,strlen(PdAt.deflt))) {
		/*
		 * Disk's model name does not match that indicated in PdAt
		 */
               	DEBUG_1("chkmodel: disk's = '%s'\n",dev_model)
               	DEBUG_1("          PdAt's = '%s'\n",PdAt.deflt)

		if (strncmp( PdAt.deflt, GENERIC_DISK, strlen(PdAt.deflt) )) {
			/*
			 * This is not a generic disk.  So this cannot be
			 * what we were looking for.
			 */
                	return(E_WRONGDEVICE);
		} else {
			/*
			 * This is a generic disk.  Determine CuAt's
			 * perception of model name.
			 */
			sprintf(srch_str, "name='%s' AND attribute=model_name",
				lname);
			rc = (int)odm_get_first(CuAt_CLASS,srch_str,&cuattr);
			if (rc == 0) {
				/*
				 * No CuAt object yet exists.  Create one.
				 */
				strcpy(cuattr.name,lname);
				strcpy(cuattr.attribute,"model_name");
				strcpy(cuattr.value,dev_model);
				strcpy(cuattr.type,"R");
				strcpy(cuattr.generic,"D");
				strcpy(cuattr.rep,"s");
				cuattr.nls_index = 11;
				if ((rc=odm_add_obj(CuAt_CLASS,&cuattr))==-1){
					DEBUG_0("Failure adding CuAt object\n")
					return(E_ODMADD);
				}
			} else if (rc==-1) {
                		/* ODM failure */
                		DEBUG_1("ODM failure accessing CuAt for %s\n",
					srch_str)
                		return(E_ODMGET);
        		} else if (rc != 0) {
				/*
				 * A CuAt match was returned.  Check to see if
				 * it is the same as the disk's
				 */
				if (strncmp(cuattr.value, dev_model,
							strlen(cuattr.value)))
					return(E_WRONGDEVICE);
			}
		}
        }
        return(0);
}

/*
 ***********************************************************************
 * NAME: disk_present
 *
 * FUNCTION: Device dependent routine that checks to see that the disk
 *	device is still at the same connection location on the same parent
 *	as is indicated in the disk's CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *	This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *	E_OK		- upon success
 *	E_AVAILCONNECT	- another device is configured w/ same parent/connwhere
 *	CALL_FINDDISK	- disk's pvid doesn't match CuDv's, does match another's
 *			- CuDv's pvid doesn't match disk's
 *			- adapter's IDEIOSTART ioctl command failed
 *	E_DEVACCESS	- adapter's IDEIOIDENT ioctl command failed
 *	E_INVCONNECT	- disk's connwhere is a null string
 *	E_ODMGET	- failure access ODM database
 *	E_ODMUPDATE	- couldn't update ODM CuAt object
 *	E_PARENTSTATE	- parent adapter is not AVAILABLE
 *			- parent adapter failed open
 *			- adapter's IDEIOSTOP ioctl command failed
 *	E_WRONGDEVICE	- disk's model name does not match PdAt's
 ***********************************************************************
 */

int
disk_present(disk,preobj,parent,pvidattr)
struct	CuDv	*disk;		/* disk's CuDv object */
struct	PdDv	*preobj;	/* disk's PdDv object */
struct	CuDv	*parent;	/* disk's parents CuDv object */
	char	*pvidattr;	/* disk's CuAt pvid attribute value */
{
struct	CuAt	dpvidattr;
struct	CuDv	tmpcudv;
	int	dpvid, adap;
	int	id_len, rc;
	char	pvidstr[33];
	char	adapname[64];
	char	sstr[256];
	uchar	dev_id;
	int	error;

	/* make sure parent is configured */
	if (parent->status != AVAILABLE) {
		DEBUG_1("disk_present: parent status=%d\n",parent->status)
		return(E_PARENTSTATE);
	}

	/* Check to see if there is another disk already AVAILABLE at the */
	/* same parent/device connection.  If there is, then will need to */
	/* exit with an error */
	sprintf(sstr,"parent = '%s' AND connwhere = '%s' AND status = '%d'",
	disk->parent, disk->connwhere, AVAILABLE);
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&tmpcudv);
	if (rc == -1) {
		DEBUG_0("disk_present: ODM error.\n")
		return(E_ODMGET);
	}
	else if (rc) {
		DEBUG_1("disk_present: %s already cfged at this location.\n",
			tmpcudv.name)
		return(E_AVAILCONNECT);
	}

	/* get device ID from connwhere field */
	if ( get_dev_id( disk->connwhere, &dev_id) )
		return( E_INVCONNECT );

	/* get identification data from drive */

	if (rc = get_ident_data(disk, id_data, &id_len))
		return (rc);

        /* issue OPEN for parent device */
	sprintf(sstr, "/dev/%s", disk->parent);
	if ((adap = open(sstr, O_RDWR)) < 0) {
                DEBUG_1("disk_present: Failed to open %s\n", sstr)
                return(E_PARENTSTATE);
	}

        /* issue IDEIOSTART for this device */
        if (ioctl(adap,IDEIOSTART,dev_id)<0) {
                DEBUG_1("disk_present: Failed to IDEIOSTART device %d\n",
                        (int) dev_id)
                close(adap);
                return(CALL_FINDDISK);
        }

        /* get pvid from the disk */
        get_pvid(adap,dev_id,pvidstr);

        /* issue IDEIOSTOP for this device */
        if (ioctl(adap,IDEIOSTOP,dev_id)<0) {
                DEBUG_1("disk_present: Failed to IDEIOSTOP device %d\n",
                        (int) dev_id)
                close(adap);
                return(E_PARENTSTATE);
        }

        close(adap);

	/*
         * if the drive has a pvid, see if there is an object
         * for it in the database
	 */

        dpvid = 0;                      /* initialize to no pvid attr */
        if (*pvidstr) {
                sprintf(sstr,"attribute = 'pvid' AND value = '%s'",pvidstr);
                dpvid = (int)odm_get_first(CuAt_CLASS,sstr,&dpvidattr);
                if (dpvid == -1) {
                        return(E_ODMGET);
                }
        }

        /* If the CuDv disk object we want to configure has NO pvid attribute
         * but the disk has a pvid on it and there's a matching pvid
         * attribute in the database
         *
         * OR
         *
         * If the CuDv disk object we want to configure HAS a pvid attribute
         * and the disk has a pvid attribute on it but they don't match
         *
         * THEN
         *     the disk is the wrong device
         */
        DEBUG_1("disk pvid = %s\n",pvidstr)
        DEBUG_1("CuAt pvid = %s\n",pvidattr)
        if (((pvidattr[0]=='\0') && *pvidstr && dpvid) ||
            ((pvidattr[0]!='\0') && strcmp(pvidattr,pvidstr))) {
		rc = CALL_FINDDISK;
		/*
		 * Force the drive identification data to be retrieved
		 * from the proper device after finddisk completes.
		 */
		first_id_drv_request = TRUE;
        	DEBUG_1("disk_present: returning %d\n",rc)
                return(rc);
        } else {
                rc = E_OK;
        }

        /* make sure drive is the right model */
        DEBUG_0("disk_present: calling chkmodel\n")
        if (rc = chkmodel(id_data,disk->PdDvLn_Lvalue,(char *)disk->name)) {
		return(rc);
	}

        DEBUG_1("disk_present: returning %d\n",rc)
        return(rc);
}

/*
 ***********************************************************************
 * NAME: get_pvidstr
 *
 * FUNCTION: Copies the pvid string into the input string.
 *
 * EXECUTION ENVIRONMENT:
 *	This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *	 0		- success
 *	 E_INVCONNECT	- CuDv's connwhere is invalid
 *	-1		- open of adapter failed
 *			- IDEIOSTART of device failed
 *			- IDEIOSTOP of device failed
 ***********************************************************************
 */

int
get_pvidstr(disk, dest_pvid)
struct	CuDv	*disk;
	char	*dest_pvid;
{
	int	adap;
	int	rc;
	char	adapname[64];
	char	sstr[256];
	uchar	dev_id;

        /* BEGIN get_pvidstr */

        /* get the device number from connwhere field */
        if ( get_dev_id( disk->connwhere, &dev_id) )
                return( E_INVCONNECT );

        /* get identification data from drive */
        sprintf(adapname,"/dev/%s",disk->parent);
        if ((adap = open(adapname,O_RDWR))<0) {
                /* error opening adapter */
                DEBUG_2("get_pvidstr: error opening %s, errno=%d\n",adapname,errno)
                return(-1);
        }

        /* issue IDEIOSTART for this device */
        if (ioctl(adap,IDEIOSTART,dev_id)< 0) {
                DEBUG_2("get_pvidstr: Failed to IDEIOSTART device %d, errno=%d\n",
                        dev_id,errno)
                close(adap);
                return(-1);
        }
 
        /* get pvid from the disk */
        get_pvid(adap,dev_id,pvid_str);
        strcpy(dest_pvid, pvid_str) ;

        /* issue IDEIOSTOP for this device */
        if (ioctl(adap,IDEIOSTOP,dev_id)<0) {
                DEBUG_1("get_pvidstr: Failed to IDEIOSTOP device %d\n",
                        (int)dev_id)
                close(adap);
                return(-1);
        }

        close(adap);
        return(0) ;
}

/*
 ***********************************************************************
 * NAME: get_pvid
 *
 * FUNCTION: Low level routine used to format pvid from disk's boot record.
 *
 * EXECUTION ENVIRONMENT:
 *	This is an internal subroutine called only by disk_present() and
 *	get_ident_data().
 *
 * RETURNS:
 *	None
 ***********************************************************************
 */

void
get_pvid(adap,dev_id,pvidstr)
	int	adap;
	uchar	dev_id;
	char	*pvidstr;
{
	int	rc;
	uchar	pvidbuf[4096];
	IPL_REC	*iplrec;

	/* get pvid from the disk */
	if ( read_ide_pvid(adap,dev_id,pvidbuf) != 0 ) {
		/* error reading pvid, assume NO pvid */
		DEBUG_0("get_pvid: error reading pvid")
		pvidstr[0] = '\0';
	} else {
		iplrec = (IPL_REC *)pvidbuf;
		if (iplrec->IPL_record_id == (uint)IPLRECID) {
			strcpy(pvidstr,pvidtoa(&(iplrec->pv_id)));
			if (!strcmp(pvidstr,NULLPVID))
				pvidstr[0] = '\0';
		} else
			pvidstr[0] = '\0';
		}
	return;
}

/*
 ***********************************************************************
 * NAME: read_ide_pvid
 *
 * FUNCTION: Low level routine used to read boot record from disk.
 *
 * EXECUTION ENVIRONMENT:
 *	This is an internal subroutine called only by get_pvid(),
 *
 * RETURNS:
 *	0 on success
 *	return code of adapter's IDEIOREAD ioctl command on failure
 ***********************************************************************
 */

int
read_ide_pvid(adapter, dev_id, ptr)
	int	adapter;
	uchar	dev_id;
	uchar	*ptr;
{
struct	ide_readblk	pvid_data;
        int		rc;
        int		tries;

        DEBUG_3("read_ide_pvid: adapter_fd=%d, dev_id=%d, ptr=%x\n",
                adapter, dev_id, ptr);

        pvid_data.ide_device = dev_id;
        pvid_data.blksize = 512;
        pvid_data.blkno = 0;
        pvid_data.timeout_value = DK_TIMEOUT;
        pvid_data.data_ptr = ptr;
        pvid_data.flags = ATA_LBA_MODE;		/* Changed by adapter driver */
						/* if device requires CHS */
        for (tries = 0; tries < 3; tries++) {
                rc = ioctl(adapter, IDEIOREAD, &pvid_data);
                if (rc == 0) {
                        return(rc);
                }
                DEBUG_2("read_ide_pvid: blksize=%d,tries = %d\n",
				pvid_data.blksize,tries);
                /* else error condition */
                if (tries > 0) {
                        switch (errno) {
                              case ETIMEDOUT:
                                pvid_data.timeout_value = DK_TIMEOUT;
                                break;
                              case EIO:
                                DEBUG_1("read_ide_pvid: EIO on device %d\n",
						dev_id);
                                break;
                              default:
                                DEBUG_2("read_ide_pvid: err=%d on device %d\n",
						errno, dev_id);
                                rc = -1;
                        }
                }
        }
        return(rc);
}

/*
 ************************************************************************
 * NAME: idevpd
 *
 * FUNCTION : Get the drive identification data and parse it
 *	out into IBM VPD format.  The calling routine may
 *	then save it in the CuVPD.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is to be run from the
 *	IDE disk device configure method only.
 *
 * NOTES :
 *
 * RETURNS:
 *	length of the VPD string.
 *	0 if unsuccessful.
 ************************************************************************
 */
int
idevpd(struct CuDv	*cudv,          /* Ptr to cudv of dvc being config'd */
	char		*vpd_str,       /* ptr to vpd str               */
	char		*def_format)    /* default VPD format string    */
{
	int	id_len;			/* len of identification data	*/
	uchar	temp_buf[BUF_SIZE];	/* temp buffer so can calc len  */
	char	*vpd_end;		/* ptr to next free byte.       */
	ushort	vpd_len;		/* current len of formatted VPD */

	char	sstr[64];		/* search string for ODM queries*/
	char	keyword[3];		/* VPD keyword.                 */
	char	more;			/* Loop control flag.           */
	char	dtype;			/* data type of the VPD field   */
	short	offset;			/* offset of value in id data. */
	short	len;			/* len of value in id data     */

	int	rc;			/* Rtn code from ODM routines   */
struct	PdAt	attr;			/* PdAt for type 'V' format attr*/

	char	*format_str;		/* Ptr to format str in use     */

/* BEGIN idevpd */

	bzero (keyword, 3);
	id_len = 0;
	vpd_len = 0;
	vpd_end = vpd_str;		/* Ptr to last byte written     */

	/*
	 *----------------------------------------------------------------------
	 * Get the 1st instance of a vpd_map attribute for this device.
	 * Then go into a loop so that we can handle multiple instances
	 * of the vpd_map attribute (loop exits when ODM returns 0 - no
	 * objects returned).
	 *----------------------------------------------------------------------
	 */
	sprintf(sstr, "uniquetype=%s and attribute=vpd_map", cudv->PdDvLn_Lvalue) ;
	rc = (int)odm_get_first(PdAt_CLASS, sstr, &attr) ;
   
	if (rc = get_ident_data(cudv, id_data, &id_len))
		return(0);
	do {
		/*
		 *------------------------------------------------------------
		 * Set format_str to appropriate format string based on
		 * results of getting the vpd_map attribute.
		 *------------------------------------------------------------
		 */
		format_str = ((rc == 0) || (rc == -1)) ? def_format : attr.deflt;

		more = (char)get_tokens(format_str, keyword, &offset,
				&len, &dtype) ;
		while (more) {

			DEBUG_4("idevpd: keywd=%s, offset=%x, len=%x, type=%c\n"
					,keyword, offset, len, dtype )

			/*
			 *---------------------------------------------------
			 * check validity of offset and length
			 *---------------------------------------------------
			 */
			if ((offset >= 0) && ((offset + len) <= id_len)) {
		
				get_vpd_value((uchar *)&temp_buf,
					id_data + offset,
					keyword, &len, dtype ) ;
				DEBUG_1("idevpd: parsed string = %s\n",
						(uchar *)&temp_buf)
				if ((vpd_len + len + VPD_OVERHEAD) > VPDSIZE) {
					/*
                       			 *------------------------------------
					 * don't have enough room for any more
					 * VPD in buffer for this keyword and
					 * the following words-- return length 
                        		 *------------------------------------
	                		 */
					return (vpd_len);
				} else {
					bcopy( &temp_buf, vpd_end,
						len+VPD_OVERHEAD);
					vpd_len += len + VPD_OVERHEAD;
					vpd_end += len + VPD_OVERHEAD;
				}
			}
			/*
			 *----------------------------------------------------
			 * Get the next keyword tokens.
			 * If no more tokens in this string, check the "values"
			 * field.  If we are already processing the "values"
			 * field, then we are done with this attribute.
			 *----------------------------------------------------
			 */
			if (!get_tokens(NULL, keyword, &offset,
					&len, &dtype)) {
				if (format_str == attr.deflt) {
					format_str = attr.values ;
					more = (char)get_tokens(format_str,
								keyword,
								&offset,
								&len,
								&dtype);
				} else {
					more = FALSE ;
				}
			}
		} /* END while */

		if (format_str != def_format) {
			/*
			 *----------------------------------------------------
			 * Check for more vpd_map attributes for this device -
			 * ONLY if not using the default format string.
			 *----------------------------------------------------
			 */
			rc = (int)odm_get_next(PdAt_CLASS, &attr);
		}
	} while ((rc != 0) && (rc != -1)) ;

	return(vpd_len) ;

} /* END idevpd */

/*
 ************************************************************************
 * NAME: get_vpd_value
 *
 * FUNCTION: Adds indicated identification data to formated VPD string with
 *	given keyword prefix.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is with in this module only.
 *
 * NOTES:
 *
 * RETURNS:
 *	None
 ************************************************************************
 */
static void
get_vpd_value(uchar     *vpd,		/* addr at which to put VPD value*/
              uchar     *id_data,	/* identification data address	*/
              char      *keyword,	/* keyword prefix to use        */
              short     *len,		/* identification data length	*/
              char      dtype)		/* data type                    */
{

    int       i;
    int       cnt=0;
    int       vpd_offset=0;
    char      hex_digits[] = "0123456789ABCDEF";

    /* BEGIN get_vpd_value */

    bzero(vpd, BUF_SIZE);

   /*
    *-------------------------------------------------------------
    * Set the VPD value "header".  Has the format of "*KKL"
    * where
    *   *  = character '*'
    *   KK = 2 character VPD keyword
    *   L  = 1 byte length in shorts (16 bit words).
    *
    * Then copy the identification data in place after the 'L'.
    *-------------------------------------------------------------
    */

    vpd[STAR_OFFSET] = '*' ;
    bcopy(keyword, &vpd[KEYWORD_OFFSET], 2)  ;

    /*
    *-------------------------------------------------------------
    *  Check the data type of the keyword field.  If it is hex 'X',
    *  change each nibble to get the ascii equivalent and increment
    *  the length.
    *-------------------------------------------------------------
    */

    if( dtype == 'X' || dtype == 'x' ) {
      for ( i=0; i< *len; i++) {
         vpd[DATA_OFFSET+vpd_offset++] = hex_digits[(id_data[i] & 0xf0 )
                                           >> 4] ;
         vpd[DATA_OFFSET+vpd_offset++] = hex_digits[(id_data[i] & 0x0f )];
         cnt++ ;
      }  /* end for */
      *len += cnt;                   /* increment the total length         */

    } /* end if */
    else {                           /* dtype is already ascii, no convert */

       bcopy(id_data, &vpd[DATA_OFFSET], *len) ;

    } /* end else */

    if ( *len%2 != 0 ) {                    /* if len is odd        */
        vpd[DATA_OFFSET + *len] = 0x00;     /* pad with a null      */
        (*len)++;                           /* take one more byte   */
    }

    vpd[LENGTH_OFFSET] = (uchar)((*len +VPD_OVERHEAD) >> 1) ;

} /* END get_vpd_value */

/*
 ************************************************************************
 * NAME: get_ident_data
 *
 * FUNCTION: Set up and request the adapter for identification data.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is to be run from the
 *	IDE disk device configure method only.
 *
 * NOTES:
 *
 * RETURNS:
 *	Zero indicates success.
 *	Error as defined in errno.h
 ************************************************************************
 */
int
get_ident_data(
struct	CuDv	*cudv,		/* Dvc's customized object */
	uchar	*id_data,	/* ptr to ident buffer */
	int	*len)		/* output param for length of data */
{
struct	identify_device	*device_ident;
	uchar		dev_id;		/* IDE device ID */
	int		fd;		/* file desc for ATA/IDE adptr */
	int		i, rc;
	char		adp_name[32];	/* spec file name of ATA/IDE adptr */
	int		error;		/* Error from issue_identify_drive */

/* BEGIN get_ident_data */

    if (!first_id_drv_request) {
	/*
	 * Don't actually issue the command to device if
	 * this is not the first request.
	 */
	*len = IDENTSIZE;
	return(0);
    }

    dev_id = strtoul(cudv->connwhere, NULL, 16);
    *len = 0;
    sprintf(adp_name, "/dev/%s", cudv->parent) ;
    if ((fd = open(adp_name, O_RDWR)) >= 0)
    {
        if (ioctl(fd, IDEIOSTART, dev_id) == 0)
        {
            /* now get drive identification data */
            if ((rc = issue_identify_drive(fd,id_data,dev_id,&error)) == 0)
            {
                *len = IDENTSIZE;
		rc = 0;
#ifdef CFGDEBUG
                /* look at identification data */
                hexdump(id_data, (long)IDENTSIZE);
#endif

		device_ident = (struct identify_device *) id_data;
		if (!(device_ident->capabilities & (ID_CAP_DMA | ID_CAP_LBA))) {
			for (i=0; i<20; i++)
			    device_ident->model_number[i] =
				(device_ident->model_number[i] >> 8) |
				((device_ident->model_number[i] & 0x00ff) << 8);
		}
            }
            else /* error getting identification data from drive */
            {
                DEBUG_0("get_id_data: error issuing identify; ") ;
                DEBUG_3("rc=%d adap=%s device=%d\n",
                        rc, adp_name, dev_id) ;
		rc = E_DEVACCESS;
    
            }

            if (ioctl(fd, IDEIOSTOP, dev_id) < 0)
            {
                DEBUG_1("Failed to IDEIOSTOP device %d\n",
                	(int)dev_id) ;
		rc = E_PARENTSTATE;
            }
        }
        else /* Error starting device */
        {
            DEBUG_2(
                "get_id_data: Failed IDEIOSTART for device %d, errno=%d\n",
                dev_id,errno) ;
	    rc = CALL_FINDDISK;

        }
        close(fd) ;
    }
    else /* error opening adapter */
    {
        DEBUG_2("get_id_data: error opening %s, errno=%d\n", adp_name, errno) ;
	rc = E_PARENTSTATE;
    }
    return(rc) ;
} /* END get_ident_data */

/*
 ***********************************************************************
 * NAME: get_tokens
 *
 * FUNCTION: break out the pieces of the VPD format string that
 *	is passed in.  (see NOTES for details for the format)
 *
 * EXECUTION ENVIRONMENT:
 *	Should only be used as part of IDE disk VPD processing.
 *
 * NOTES:
 *        The format of the format string is a comma seperated
 *        string of keyword location identifiers.  The format of
 *        of the keyword location identifier is as follows:
 *          KKOOLLD
 *             where
 *               KK is the 2 character keyword name to be associated
 *                  with the data.
 *               OO is the 2 hexadecimal digit offset into the identification
 *		    data where the VPD keyword value resides.
 *               LL is the 2 hexadecimal digit length of the keyword value
 *                  in the identification data.  This length is in bytes.
 *               D  is the 1 character data type, C for char ASCII, X hex.
 *                  This will be used for conversion.
 *
 * RETURNS:
 *	TRUE  for success
 *	FALSE for failure
 ***********************************************************************
 */
static int
get_tokens(char   *format_str,          /* the format string            */
	char	*keyword,	/* return var for keyword name  */
	short	*offset,	/* return var for offset        */
	short	*len,		/* return var for len of value  */
	char	*dtype)		/* return var for string data typ*/
{
	char	*loc_id;	/* beginning of this keyword     */
	int	rc;
	char	part[3];

	/* BEGIN get_tokens */

	bzero(part, sizeof(part));

	loc_id = strtok(format_str, ", ");	/* search for next , or blank */

	if (loc_id != NULL) {
		/* get the C or X data type   */
		*dtype = loc_id[strlen(loc_id) -1] ; 
		strncpy(keyword, loc_id, 2);
		*offset = (short)strtol(strncpy(part,loc_id+2,2),NULL,16);

		*len =    (short)strtol(strncpy(part,loc_id+4,2),NULL,16);

        	rc = TRUE ;
    	} else {
        	rc = FALSE ;
    	}

	return(rc) ;

} /* END get_tokens */

/*
 ***********************************************************************
 * NAME: get_dev_id
 *
 * FUNCTION: Extracts the device ID from the connwhere string provided.
 *
 * EXECUTION ENVIRONMENT:
 *	Dynamically loaded & run by generic config
 *
 * NOTES:
 *	1. This code is designed to be loadable, therefore no global variables 
 *	   are assumed.
 *	2. The basic function is to convert the input string
 *	   a uchar values (i.e. the device ID)
 *
 * RETURNS:
 *	 0 for success
 *	-1 for failure
 ***********************************************************************
 */
int
get_dev_id( ideaddr, dev_id )
char	*ideaddr;
uchar	*dev_id;
{
	if (*ideaddr == '\0') return -1;

/* We utilize the behavior of strtoul which stops converting characters at
   the first non-base character Thus after the start position is set, the
   conversion stops either at the ',' or at the NULL end-of-string */

	*dev_id = (uchar)strtoul(ideaddr,NULL,10);

	return 0;
}

