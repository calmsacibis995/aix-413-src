static char sccsid[] = "@(#)79 1.24.1.42 src/bos/usr/sbin/bootinfo/bootinfo.c, bosboot, bos41J, 9522A_b 95/05/31 11:32:29";
/*
 * COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
 *
 * FUNCTIONS: bootinfo
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 *
 * This command is used to provide boot information used
 * during the boot process and also provide BOS install machine
 * defaults. The install defaults consist of root volume group
 * disks, paging disks, page size, and physical partition size.
 * Other information it may provide is the amount of
 * configured memory, boot device type, boot device, and
 * disk sizes and locations.
 *
 * EXECUTION ENVIRONMENT
 *
 *	The bootinfo command is usually run from the
 *	RAM file system during boot and BOS installation.
 *	The command can also be executed from a running
 *	system.
 *
 *	The device configuration databases and NVRAM are
 *	used to extract required information. The ODMDIR
 *	environment must be set to the appropriate path.
 *
 */

#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cf.h>
#include <cfgresid.h>
#include <odmi.h>
#include <lvm.h>
#include <macros.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <sys/utsname.h>
#define _KERNSYS
#define _RS6K
#define _RSPC
#define _RS6K_SMP_MCA
#include <sys/systemcfg.h>
#undef	_KERNSYS
#include <nl_types.h>
#include <locale.h>
#include <sys/errno.h>

#define MAX_SLOT	8
#define DSKMEG		1048576
#define MSGCAT		"bootinfo.cat"
#define NVLOC1		0x000102	/* NVRAM network boot device location */

/*
 * IPL control block, NET_DATA (net boot scratch pad area) defines
 */
#define ICB_NETD_SLOT_MSK       0x0f    /* slot_number field, slot # mask */
#define ICB_NETD_BUS_MSK	0x10    /* slot_number field, bus id mask */
#define ICB_NETD_IENET_SLOT     14	/* slot_number field, integrated */
					/*  ethernet slot number */
/*
 * Configuration database search strings
 */
#define SYS_SSTRING	"name = 'sys0' AND attribute = 'modelcode'"
#define MEM_SSTRING	"name = 'sys0' AND attribute = 'realmem'"
#define BADISK_SSTR	"PdDvLn = disk/mca/badisk AND connwhere = '%d' AND status = 1"
#define NETADAP_SSTR    "PdDvLn LIKE adapter/*/* AND location=%s"
#define SCSIDEV_SSTR	"PdDvLn LIKE */scsi/* AND location LIKE %s AND status=1"
#define DSKT_SSTR	"PdDvLn LIKE diskette/siofd/* AND connwhere = '%1d' AND status = 1"
#define SERDLOC_SSTR	"PdDvLn LIKE disk/serdasdc/* AND location=%s AND status = 1"

/* @@SSA_BEGIN@@ */
#define SSAR_SSTR	"connwhere='%15.15s'"\
			" AND PdDvLn LIKE 'disk/ssar/*' AND status=1"
#define SSA_SSTR	"connwhere='%15.15s'"\
			" AND PdDvLn LIKE '*/ssa/*' AND status=1"
/* @@SSA_END@@   */

/*
 * function prototype declarations
 */
int mp_capable(void);
int model_display(void);
int platform_display(char *platform);
int memory_display(void);
int devtype_display(void);
char *devboot_display(void);
long disksize_display(char *pagedv);
int device_display(char *diskid);
int network_display(void);
int ipaddr(IP_ADDR in, char *str);
int iplcb_get(void *dest, int address, int num_bytes, int iocall);
int bootable_dev(char *name);
int ros_bootable_adap(char *adap);
short is_eserver(char *modeltype);
int get_planar_id(ulong *id);
int rspc_bootable_adap(struct CuDv *, struct PdDv *);


/* Global variable */
char	bootname[16];


/* --------------------------------------------------------------------------*/
main(int argc, char *argv[])
{
extern int	optind;		/* for getopt function */
extern char	*optarg;	/* for getopt function */

/*
 * Option flags and variables
 */
int	boot_dev,mach_type,mach_mem,dev_type,dev_size,model_chk,errflg;
int	inst_opp;
int	machine;
char	*diskid;
nl_catd catd;
char	*msgfmt;
char	*bootdiskid;
int	net_info;
int	part_size;
int	rmemory;
int	boottype;
int	partsize;
long	devsize;
char	*bootdv;
char	*pagedv;
int	c;
int	supp_boot;
int	keyposn;
int	key;
int	rosbootable;
char	*bootadap;
int	fd;		/* file descriptor */
int	mp_test;
MACH_DD_IO	mdd;
int	platform_type;
char	platform[16];
char	modeltype[50];
short	rc;

	/*
	 * Internationalize!
	 */
	char *p;
	p = setlocale(LC_ALL, "");
	catd = catopen(MSGCAT, NL_CAT_LOCALE);

	errflg = boot_dev = mach_type = mach_mem = dev_type = 0;
	dev_size = part_size = partsize = inst_opp = model_chk = 0;
	net_info = supp_boot = keyposn = 0;
	mp_test = rosbootable = platform_type = 0;

	/* parse command line parameters */
	while ((c = getopt(argc,argv,"bB:ckMmo:P:q:rs:tTz")) != EOF) {
		switch (c) {
		case 'b':
			boot_dev = 1;
			break;
		case 'B':
			bootdiskid = optarg;
			supp_boot = 1;
			break;
		case 'c':
			net_info = 1;
			break;
		case 'k':
			keyposn = 1;
			break;
		case 'm':
			mach_type = 1;
			break;
		case 'M':
			model_chk = 1;
			break;
		case 'o':
			diskid = optarg;
			inst_opp = 1;
			break;
		case 'P':
			partsize = atoi(optarg);
			dev_size = 0;
			part_size = 1;
			break;
		case 'q':
			rosbootable = 1;
			bootadap = optarg;
			break;
		case 'r':
			mach_mem = 1;
			break;
		case 's':
			pagedv = optarg;
			if (!part_size)
				dev_size = 1;
			break;
		case 't':
			dev_type = 1;
			break;
		case 'T':
			platform_type=1;
			break;
		case 'z':
			mp_test = 1;
			break;
		case '?':
			errflg = 1;
		}
	}

	if (errflg) {
		/* error parsing parameters */
		msgfmt = catgets(catd, 250, 1,
		"\nusage: bootinfo [ -P psize -s disk ] |-b |-B disk |-c |\n");
		fprintf(stderr, msgfmt);
		msgfmt = catgets(catd, 250, 2,
		"\t\t-k |-m |-M |-q adapter |-r |-t |-T |-o disk |-z\n");
		fprintf(stderr, msgfmt);
		exit(1);
	}

	if (keyposn) {
		/*
		 * DISPLAY VALUES:
		 *	1 - secure or locked position
		 *	2 - service
		 *	3 - normal
		 */
		if ((fd = open("/dev/nvram", O_RDONLY)) != -1)
		{
			mdd.md_incr = MV_WORD;
			mdd.md_data = (uchar *)&key;
			mdd.md_size = 1;
			ioctl(fd, MIOGETKEY, &mdd);
			close(fd);
		}

		key &= 0x03;
		if (key > 3)
			exit(1);
		printf("%d\n", key);
		exit(0);
	}

	/* Initialize the ODM */
	if (odm_initialize() == -1) {
		msgfmt = catgets(catd, 250, 21,
		"\n 0301-268 Object Data Manager initialize failed!\n");
		fprintf(stderr, msgfmt);
		exit(15);
	}

	/*
	 *
	 * Call appropriate functions.
	 *
	 */
	if (dev_type) {
		/*
		 *
		 * Find type of boot media:
		 *
		 *	1 - disk
		 *	2 - diskette
		 *	3 - cdrom
		 *	4 - tape
		 *	5 - network
		 *
		 */
		if ((boottype = devtype_display())!=-1)
			printf("%d\n",boottype);
		else exit(2);
	}

	if (boot_dev) {
		/*
		 *
		 * Find actual boot device.
		 *
		 */
		if ((bootdv = devboot_display())!=NULL)
			printf("%s\n",bootdv);
		else exit(3);
	}

	if (dev_size) {
		/*
		 *
		 * Find disk size in Mbytes.
		 *
		 */
		if ((devsize = disksize_display(pagedv))!=-1)
			printf("%ld\n",devsize);
		else exit(4);
	}

	if (part_size) {
		/*
		 *
		 * Find default physical partition for disk.
		 *
		 */
		if ((devsize = disksize_display(pagedv))==-1)
			exit(5);
		if (partsize == 0) {
			if (devsize < 300)	/* small disk */
				partsize = 2;
			else {	/* disk is larger than 300M */
				partsize = 4;
				while ((partsize * LVM_MAXPPS) < devsize)
					partsize <<= 1;	/* multiply by 2 */
			}
		}
		printf("%d\n",partsize);
	}

	if (inst_opp) {
		/*
		 *
		 * Find location code corresponding to disk name
		 * or disk name corresponding to location code.
		 *
		 */
		device_display(diskid);
	}

	if (mach_mem) {
		/*
		 *
		 * Find real memory size in Kbytes.
		 *
		 */
		if ((rmemory = memory_display()) != -1)
			printf("%d\n",rmemory);
		else exit(11);
	}

	if (mach_type) {
		/*
		 *
		 * Find machine model code.
		 *
		 */
		if ((machine = model_display()) != -1)
			printf("%d\n",machine);
		else exit(12);
	}

	if (platform_type ) {
		if( platform_display(platform) != -1 )
			printf("%s\n",platform);
		else	exit(13);
	}

	if (net_info) {
		/*
		 *
		 * Display bootp packet in ROS iplcb.
		 *
		 */
		if (network_display() == -1 ) exit(14);
	}

	if (supp_boot) {
		/*
		 * determine whether bootable by calling bootable_dev
		 * where the parameter is a name like "hdisk0"
		 */
		printf("%d\n", bootable_dev(bootdiskid));
	}
	/* Perform model checking to determine if model type is allowed
	 * for entry server OS Install.  If undetermined, rc is -1, if
	 * eserver is allowed rc is 100 or 101, else rc is 102 or 103.
	 * If diagnostics are supported then rc will be 101 or 103.
	 */
	if (model_chk) {

		if((rc = is_eserver(modeltype)) != -1 )
			printf("%s\n",modeltype);
		odm_terminate();
		exit(rc);
	}
	if( rosbootable )
		printf("%d\n",ros_bootable_adap(bootadap));

	if( mp_test )
		printf("%d\n", mp_capable() );

	odm_terminate();
	exit(0);
}

/* --------------------------------------------------------------------------*/
/*
 *
 * Start of functions.
 *
 */

/* --------------------------------------------------------------------------*/
/*
 * mp_capable:	output 0 if system can support only one processor
 *		       1 if system can support multiple processors
 */

int
mp_capable(void)
{
IPL_DIRECTORY iplcb_dir;		/* IPL control block directory */
SYSTEM_INFO   system_info;	      /* IPL control blk info section */
int	result;

	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) {
		perror("Error reading IPL-Ctrl block directory");
		return(-1);
	}
	result = 0;	/* default = uni-processor */
	/*
	 * The ipl_directory varies in size, so the following calculation
	 * is intended to determine whether the iplcb_dir.system_info_offset
	 * field exists or not. Do this by comparing the address of the
	 * system_info_offset against the address of the ipl_info struct, which
	 * follows the ipl_directory in memory. The address of the ipl_info
	 * struct is calculated by adding the address of the ipl_directory
	 * (cast to int to prevent incrementing by size of the struct)
	 * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
	 * If the address of system_info_offset is less than the address of the
	 * ipl_info struct, assume existence and validity of the
	 * system_info_offset and system_info_size fields.
	 */
	if ((int)(&iplcb_dir.system_info_offset) <
		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) {
	    if((iplcb_dir.system_info_size > 0) &&
		(iplcb_dir.system_info_offset > 0)) {

		if (iplcb_get(&system_info,iplcb_dir.system_info_offset,
			sizeof(SYSTEM_INFO),MIOIPLCB)) {
			perror("Error reading IPL-Ctrl block system_info area");
			return(-1);
		}
		if (system_info.num_of_procs > 1)
			result = 1;	/* multi-processor capable */
	    }
	}
	return(result);
}

/* --------------------------------------------------------------------------*/
int
model_display(void)
{
struct  CuAt attobj;
int	rc;

	/* find out the machine type */
	rc = (int)odm_get_first(CuAt_CLASS,SYS_SSTRING,&attobj);
	if (rc==0 || rc==-1) {
		return(0);
	}

	/* get the machine type out of the object */
	return((int)strtol(attobj.value,(char **)NULL,0));
}

/* --------------------------------------------------------------------------*/
int
platform_display(char *platform)
{
IPL_DIRECTORY iplcb_dir;		/* IPL control block directory */
SYSTEM_INFO   system_info;	      /* IPL control blk info section */

	strcpy(platform,"");
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) {
		perror("Error reading IPL-Ctrl block directory");
		return(-1);
	}

	strcpy(platform,"rs6k");
	/*
	 * The ipl_directory varies in size, so the following calculation
	 * is intended to determine whether the iplcb_dir.system_info_offset
	 * field exists or not. Do this by comparing the address of the
	 * system_info_offset against the address of the ipl_info struct, which
	 * follows the ipl_directory in memory. The address of the ipl_info
	 * struct is calculated by adding the address of the ipl_directory
	 * (cast to int to prevent incrementing by size of the struct)
	 * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
	 * If the address of system_info_offset is less than the address of the
	 * ipl_info struct, assume existence and validity of the
	 * system_info_offset and system_info_size fields.
	 */
	if ((int)(&iplcb_dir.system_info_offset) <
		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) {
	    if((iplcb_dir.system_info_size > 0) &&
		(iplcb_dir.system_info_offset > 0)) {

		if (iplcb_get(&system_info,iplcb_dir.system_info_offset,
			sizeof(SYSTEM_INFO),MIOIPLCB)) {
			perror("Error reading IPL-Ctrl block system_info area");
			return(-1);
		}

		/* Make sure pkg_desc field is there */
		if( ((long unsigned) &system_info.pkg_descriptor + 16 -
			(long unsigned) &system_info )
				<= iplcb_dir.system_info_size)  {
			if(system_info.pkg_descriptor[0] != '\0') {
				strcpy(platform,system_info.pkg_descriptor);
				return(0);
			}
		}
	    }
	}
	if( __rspc() ) strcpy(platform,"rspc");
	if( __rs6k_smp_mca() ) strcpy(platform,"rs6ksmp");

	if( (!strcmp(platform,"rs6k")) && (mp_capable() ==  1))
		strcpy(platform,"rs6ksmp");
	return 0;
}

/* --------------------------------------------------------------------------*/
int
memory_display(void)
{
struct  CuAt *attobj;
int	rc;

	/* find out the machine type */
	attobj = (struct CuAt *)getattr("sys0","realmem",FALSE,&rc);
	if (attobj == (struct CuAt *) NULL) {
		return 0;
	}

	/* get the machine real memory size in K out of the object */
	return((int)strtol(attobj->value,(char **)NULL,0));
}

/* --------------------------------------------------------------------------*/
int
devtype_display(void)
{
IPL_DIRECTORY iplcb_dir;		/* IPL control block directory */
IPL_INFO iplcb_info;			/* IPL control blk info section */
char	*ipl_ptr;
int iocall = MIOIPLCB;

	/*
	 * Check for existence of auxilary IPL control block
	 * and read in its directory section.
	 * If it exists, boot was a ROS Emulation boot, and
	 * we get the info area from the auxilary IPL control block.
	 */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOAIPLCB) == 0)
	{
		iocall = MIOAIPLCB; /* set up next call to use aux iplcb */
	}

	else if (errno != ENODEV)
	{
		perror("Error reading Auxilary IPL-Ctrl block directory");
		return -1;
	}

	/*
	 * otherwise get directory section of IPL control block
	 * it starts at offset of 128 into IPL control block
	 */
	else if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB))
	{
		perror("Error reading IPL-Ctrl block directory");
		return -1;
	}

	/*
	 *
	 * get iplinfo structure.
	 *
	 */
	if (iplcb_get(&iplcb_info,iplcb_dir.ipl_info_offset,
			sizeof(iplcb_info), iocall))
	{
		perror("Error reading IPL-Ctrl block info section");
		return -1;
	}


	ipl_ptr = iplcb_info.previpl_device;

#ifdef BOOT_DEBUG
if (__rspc())
	fprintf(stderr, "The previpl string is {%c%c%d%s}\n",ipl_ptr[0],ipl_ptr[1],ipl_ptr[2],&ipl_ptr[3]);
#endif

	switch(ipl_ptr[1]) {
	case 'K':
		/* type 1=internal disk */
		return(1);
	case 'N':
		/* type 2=diskette */
		return(2);
	case 'D':
	case 'O':
	case 'P':
		/* type 5=network ethernet, token ring, or fddi */
		return(5);
/*	case 'H':	Uncomment for Serial-Linked DASD */
	case 'S':
		/* type (1=disk,2=diskette,3=cdrom,4=tape) */
		return((int)ipl_ptr[6]);
/* @@SSA_BEGIN@@ */
	case 'T':
/************************************************************************/
/* Currently the only supported SSA devices are DASD. When other types	*/
/* of device are supported, it is planned to deduce the type from the	*/
/* last byte of the serial number stored in NVRAM. For now we always	*/
/* return 1.								*/
/************************************************************************/
		return 1;
/* @@SSA_END@@  */
	case 'V':
		/* type (1=disk,2=diskette,3=cdrom,4=tape) */
		return(ipl_ptr[ipl_ptr[0] - 2]);
	case '!':
		/* This is an RSPC model */
		/* type (1=disk,2=diskette,3=cdrom,4=tape,5=network) */
		return(ipl_ptr[2]);
	default:
		return -1;
	}
}

/* --------------------------------------------------------------------------*/
char *
devboot_display(void)
{
IPL_DIRECTORY iplcb_dir;		/* IPL control block directory */
IPL_INFO iplcb_info;			/* IPL control blk info section */
NET_DATA iplcb_net;			/* IPL control blk network boot area */
char	*ipl_ptr, *src_ptr;		/* Pointers into previpl_device */
char	loc[16];
char	*devicename,sstr[256];
char	sid,lun;
struct  CuDv *bootdev;
/* @@SSA_BEGIN@@ */
struct	CuAt *cuat;
struct  listinfo cuat_info;
/* @@SSA_END@@   */
struct  listinfo cudv_info;
int i, rc, slot;
int bus,busext, busid;
int iocall = MIOIPLCB;

	/*
	 * Check for existence of auxilary IPL control block
	 * and read in its directory section.
	 * If it exists, boot was a ROS Emulation boot, and
	 * we get the information from the auxilary IPL control block.
	 */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOAIPLCB) == 0)
	{
		iocall = MIOAIPLCB;   /* subsequent calls use aux iplcb */
	}

	else if (errno != ENODEV)
	{
		perror("Error reading Auxilary IPL-Ctrl block directory");
		return (void *)NULL;
	}

	/*
	 * otherwise get directory section of IPL control block
	 * it starts at offset of 128 into IPL control block
	 */
	else if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB))
	{
		perror("Error reading IPL-Ctrl block directory");
		return NULL;
	}

	/*
	 *
	 * get iplinfo structure.
	 *
	 */
	if (iplcb_get(&iplcb_info,iplcb_dir.ipl_info_offset,
			sizeof(iplcb_info), iocall))
	{
		perror("Error reading IPL-Ctrl block info section");
		return NULL;
	}


	ipl_ptr = iplcb_info.previpl_device;

#ifdef BOOT_DEBUG
if (__rspc())
	fprintf(stderr, "The previpl string is {%c%c%d%s}\n",ipl_ptr[0],ipl_ptr[1],ipl_ptr[2],&ipl_ptr[3]);
#endif

	devicename = NULL;

	switch(ipl_ptr[1]) {
	case 'D':
	case 'O':
	case 'P':
		/*
		 *
		 * ethernet/tokenring boot device ?
		 *
		 */
		/* Need to get slot number from IPL control block */
		  if (iplcb_get(&iplcb_net, iplcb_dir.net_boot_results_offset,
				sizeof(iplcb_net), iocall))
		  {
		    perror("Error reading IPL-Ctrl block network boot area");
		    return NULL;
		  }
		  slot = iplcb_net.slot_number;
		  bus = slot & ICB_NETD_BUS_MSK;
		  if (bus) bus = 1;
		  slot = slot & ICB_NETD_SLOT_MSK;

		  /* build location code from this information */
		  if (slot == ICB_NETD_IENET_SLOT) {
			/* It's integrated ethernet */
			sprintf(loc,"00-00-0E");
		  } else {
			/* It's an mca adapter card
			 * add 1 to slot because ROS doesn't match config.
			 */
			slot = slot + 1;
			sprintf(loc,"00-%1x%1x",bus,slot);
		  }
		sprintf(sstr,NETADAP_SSTR,loc);
		bootdev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,
								&cudv_info,2,1);
		/* First look for AVAILABLE device */
		for (i=0; i<cudv_info.num; i++) {
			if (bootdev[i].status == AVAILABLE) {
				devicename = bootdev[i].name;
				break;
			}
		}

		/* If we didn't find an AVAILABLE device, look for DEFINED */
		if (devicename == NULL) {
			for (i=0; i<cudv_info.num; i++) {
				if (bootdev[i].status == DEFINED) {
					devicename = bootdev[i].name;
					break;
				}
			}
		}
		break;


/*	case 'H':	Uncomment for Serial-Linked DASD	*/
	case 'V':
	case 'S':
		/*
		 *
		 * SCSI or Serial-Linked boot device ?
		 *
		 */

		if (ipl_ptr[0] == 17)
			src_ptr = ipl_ptr + 9;
		else if (ipl_ptr[0] == 25)
			src_ptr = ipl_ptr + 17;
		else
			src_ptr = ipl_ptr;

		bus = src_ptr[2];
		if (bus < 0x20)
			busid = bus;		/* rspc */
		else
			busid = bus - 0x20;	/* rs6k and rs6ksmp */
		slot = src_ptr[4] + 1;
		sid = src_ptr[7];
		lun = src_ptr[8];

		busext =  (src_ptr[3] != 'I') ? 1:0;  /* See Notes below */

		/* build location code from this information */
		if (slot == 14) {
			/* It's integrated SCSI */
			/* Note: On non-corvette scsi adapters, back-level ROS
			   may have left src_ptr[3] un-initialized. This causes
			   busext value to be invalid if booted from one of
			   these adapters with a back-level ROS.
			   The following check should circumvent this:
				First take busext value as is, and a locate
				the boot device.
			   If none is found, try non-corvette scsi.
			   Essentially, the location code (loc) of the
				device got searched in this manner:
			   if busext = 0,
				loc=00-00-00-sss,lll - integrated corvette, internal bus
			   if busext = 1,
				loc=00-00-01-sss,lll - integrate corvette, external bus
			   if( any of the above failed )
				loc=00-00-0S-sss,lll - integrated non-corvette

				 where sss is 1 to 3 decimal digit scsi id
				 where lll is 1 to 3 decimal digit lun */


			sprintf(loc,"?0-00-?%1x-%d,%d",busext,sid,lun);
			sprintf(sstr,SCSIDEV_SSTR,loc);
			rc = (int)odm_get_first(CuDv_CLASS,sstr,&cudv_info);

			/* if no device found, look for non-corvette
			   integrated scsi */
			if( !rc )
			    sprintf(loc,"?0-00-?S-%d,%d",sid,lun);

		} else {
			/* It's a SCSI adapter card */
			/* Note: The problem described in the above note exists
			   in non-integrated SCSI adapters card as well.
			   The following check should circumvent this:
			   First take busext value as is, and a locate the boot device.
			   If not found and the bus is external, try internal bus.
			   Essentially, the location code of the device got
			   searched in this manner:
				00-xx-00-sss,lll  or 00-xx-01-sss,lll  internal/external bus
			   if( the above failed and busext is 1 )
			   	00-xx-00-sss,lll  Try internal bus */

			sprintf(loc,"?0-%1x%1x-?%1x-%d,%d",busid,slot,busext,sid,lun);
			sprintf(sstr,SCSIDEV_SSTR,loc);
			rc = (int)odm_get_first(CuDv_CLASS,sstr,&cudv_info);
			if( !rc && (busext == 1)) {
			   busext = 0; /* Try internal bus, if external failed */
			   sprintf(loc,"?0-%1x%1x-?%1x-%d,%d",busid,slot,busext,sid,lun);
		}
		}

		sprintf(sstr,SCSIDEV_SSTR,loc);
		bootdev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,
								&cudv_info,2,1);

		if (cudv_info.num > 0) {
			devicename = bootdev[0].name;
		}
		else {   /* Is it a Serial-Linked Card  */
			sprintf(loc,"00-%1d%1d-%02d-%02d",busid,slot,sid,lun);
			sprintf(sstr,SERDLOC_SSTR,loc);
			bootdev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,
								&cudv_info,2,1);
			if (cudv_info.num > 0) {
				devicename = bootdev[0].name;
			}
		}
		break;

/* @@SSA_BEGIN@@ */
	case 'T':
/************************************************************************/
/* Assume that this is a disk owned by the SSA router to start with	*/
/************************************************************************/

		sprintf( sstr, SSAR_SSTR, &ipl_ptr[ 2 ] );

		bootdev = (struct CuDv *)odm_get_list( CuDv_CLASS,
						       sstr,
						       &cudv_info,
						       2, 1 );
		if( cudv_info.num > 0 )
		{
		    devicename = bootdev[ 0 ].name;
		}
		else
		{
/************************************************************************/
/* It must therefore be a device like a tape - not owned by the router	*/
/************************************************************************/

		    sprintf( sstr, SSA_SSTR, &ipl_ptr[ 2  ] );

		    bootdev = (struct CuDv *)odm_get_list( CuDv_CLASS,
							   sstr,
							   &cudv_info,
							   2, 1 );
		    if( cudv_info.num > 0 )
		    {
			devicename = bootdev[ 0 ].name;
		    }
		}
		break;
/* @@SSA_END@@   */

	case 'N':
		/*
		 *
		 * diskette boot device ?
		 *
		 */
		sprintf(sstr,DSKT_SSTR,ipl_ptr[2]);
		bootdev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,
								&cudv_info,2,1);
		if (cudv_info.num > 0) {
			devicename = bootdev[0].name;
		}
		break;

	case 'K':
		/*
		 *
		 * internal disk boot device ?
		 *
		 */
		slot = ipl_ptr[2] + 6;
		sprintf(sstr,BADISK_SSTR,slot);
		bootdev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,
								&cudv_info,2,1);
		if (cudv_info.num > 0) {
			devicename = bootdev[0].name;
		}
		break;

	case '!':
		/*
		 *
		 * RSPC model
		 *
		 */
		/* fw-boot-name starts in fourth byte...convert to ODM name */
		rc = get_bootdev_odm_name(&ipl_ptr[3],bootname);
		if (rc == 0) {
			devicename = bootname;
		}
		break;


	default:
		return NULL;
	}

	return devicename;
}

/* --------------------------------------------------------------------------*/
long
disksize_display(char *pagedv)
{
struct  devinfo devinfo;
char	sstr[256];
int	rc, fd;
long disksize;

	strcpy(sstr,"/dev/");
	strcat(sstr,pagedv);
	fd = open( sstr, O_RDONLY );
	if (fd < 0) return -1;
	rc = ioctl(fd,IOCINFO,&devinfo);
	if (rc < 0) return -1;

	switch(devinfo.devtype) {
	case DD_SCDISK:
		disksize = ((float)devinfo.un.scdk.numblks / (float)DSKMEG)
				* UBSIZE;
		break;
	case DD_DISK:
		disksize = ((float)devinfo.un.dk.numblks / (float)DSKMEG)
				* UBSIZE;
		break;
	default:
		disksize = 0;
		break;
	}
	return disksize;
}

/* --------------------------------------------------------------------------*/
int
device_display(char *diskid)
{
struct  CuDv *dev;
struct  listinfo cudv_info;
char	sstr[256];
int	i;


	sprintf(sstr,"name='%s'",diskid);
	dev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,&cudv_info,8,1);
	if (cudv_info.num != 0) {
		for (i=0; i<cudv_info.num; i++) {
			printf("%s\n",dev[i].location);
		}
	} else {
		sprintf(sstr,"location='%s'",diskid);
		dev = (struct CuDv *)odm_get_list(CuDv_CLASS,sstr,
								&cudv_info,8,1);
		for (i=0; i<cudv_info.num; i++) {
			printf("%s\n",dev[i].name);
		}
	}
	return(0);
}

/* --------------------------------------------------------------------------*/
int
network_display(void)
{
IPL_DIRECTORY iplcb_dir;		/* IPL control block directory */
NET_DATA iplcb_net;			/* IPL control blk network boot area */
char	ip_str[16];
int iocall = MIOIPLCB;
char    hw_attrib;
int     i, slot;
int     prntcode, bus;



	/*
	 * Check for existence of auxilary IPL control block
	 * and read in its directory section.
	 * If it exists, boot was a ROS Emulation boot, and
	 * we get the information from the auxilary IPL control block.
	 */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOAIPLCB) == 0)
	{
		iocall = MIOAIPLCB; /* set up next call to use aux iplcb */
	}

	else if (errno != ENODEV)
	{
		perror("Error reading Auxilary IPL-Ctrl block directory");
		return(-1);
	}

	/*
	 * otherwise get directory section of IPL control block
	 * it starts at offset of 128 into IPL control block
	 */
	else if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB))
	{
		perror("Error reading IPL-Ctrl block directory");
		return NULL;
	}

	/*
	 *
	 * get network boot area.
	 *
	 */
	if (iplcb_get(&iplcb_net,iplcb_dir.net_boot_results_offset,
					sizeof(iplcb_net), iocall))
	{
		perror("Error reading IPL-Ctrl block network boot area");
		return NULL;
	}


	/*
	 * Print Client IP address
	 *	Server IP address
	 *	Gateway IP address
	 *	Type of network hardware boot (1=ethernet, 0=token ring)
	 *	Slot Number of network adapter
	 *	802_3 indicator (0=ethernet en0 else et0)
	 *	Bootfile name
	 *	Vendor tag for the subnetmask
	 */
	ipaddr(iplcb_net.bootpr.yiaddr, ip_str);
	if (! strcmp(ip_str,"0.0.0.0"))
		ipaddr(iplcb_net.bootpr.ciaddr, ip_str);
	printf("%s ", ip_str);

	ipaddr(iplcb_net.bootpr.siaddr, ip_str);
	printf("%s ", ip_str);

	ipaddr(iplcb_net.bootpr.giaddr, ip_str);
	printf("%s ", ip_str);

	printf("%d ", iplcb_net.network_type);
	printf("%d ", iplcb_net.slot_number);
	printf("%d ", iplcb_net.is_802_3);
	printf("%s ", iplcb_net.bootpr.file);

	for (i = 0; i < 64; i++)
		printf("%d.", iplcb_net.bootpr.vend[i]);

	/*
	 * Get hardware attribute from NVRAM.
	 * There are 8 bytes for the hw attribute in NVRAM, indexed by slot #.
	 * Within each byte, the low nibble is for bus 0 (I/O planar 1),
	 *   and the high nibble is for bus 1 (I/O planar 2).
	 * Each nibble is defined as follows:
	 *
	 *	bit 0 = 1 => 4MB token ring
	 *	bit 1 = 1 => 16MB token ring
	 *	bit 2 = 1 => thin ethernet
	 *	bit 3 = 1 => thick ethernet
	 *
	 * The ROS code is careful to preserve the token ring bits when
	 *  setting the ethernet bits, and vice versa.
	 *  This means we can not assume the token ring bits are zero, just
	 *  because we are dealing with an ethernet, and vice versa. This
	 *  has been done to allow for future boards that contain both
	 *  token ring and ethernet capabability.
	 */
	slot = iplcb_net.slot_number;
	bus = slot & ICB_NETD_BUS_MSK;
	slot = slot & ICB_NETD_SLOT_MSK;
	if (slot > 7)		/* Are only 8 bytes for hw attrib in NVRAM */
		prntcode = 0;	/* so do not access it if integrated device */
	else {
		if (iplcb_get(&hw_attrib,NVLOC1+slot,1,MIONVGET))
		{
			perror("Error reading IPL-Ctrl block directory");
			return NULL;
		}
		if (bus)	/* for bus 1, hw attribute is in high nibble */
			hw_attrib = hw_attrib >> 4;
		switch( (char)iplcb_net.network_type ) {
			case 'O':	/* Token ring */
				switch( hw_attrib & 0x3 ) {
					case 1:		/* 4MB token ring */
						prntcode = 4;
						break;
					case 2:		/* 16MB token ring */
						prntcode = 5;
						break;
					default:
						prntcode = 0;
						break;
				}
				break;
			case 'D':	/* Ethernet */
				switch( hw_attrib & 0xC ) {
					case 4:		/* thin ethernet */
						prntcode = 7;
						break;
					case 8:		/* thick ethernet */
						prntcode = 8;
						break;
					case 12:	/* twisted pair */
						prntcode = 9;
						break;
					default:
						prntcode = 0;
						break;
				}
				break;
			default:
				prntcode = 0;
				break;
		}
	}
	printf(" %x ", prntcode);
	printf("\n");

	exit(0);
}

/* --------------------------------------------------------------------------*/
/*
 *  ipaddr: put text internet address in str
 */
int
ipaddr(IP_ADDR in, char *str)
{
	int	i, len = 0;

	if (!str)
		len = sprintf(str, "00");
	else
		for (i = 0; i < 4; i++)
		len += sprintf(str + len, "%d.", (in >>(long)(3 - i) * 8) & 0xFF );

	*(str + len - 1) = '\0';

	return 0;
}

/* --------------------------------------------------------------------------*/
/*
 * iplcb_get: Read in section of the IPL control block.  The directory
 *		section starts at offset 128.
 */
int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram",0)) < 0) {
		return(-1);
	}

	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	if (ioctl(fd,iocall,&mdd)) {
		return(-1);
	}

	close(fd);
	return(0);
}

/* --------------------------------------------------------------------------*/
int
bootable_dev(char *name)
{
IPL_DIRECTORY iplcb_dir;	/* IPL control block directory */
GLOBAL_DATA iplcb_glob;		/* IPL control blk GLOBAL DATA  */
struct  adapt_info *adapter_info;
struct listinfo cudv_info;
char	sstr[256];
char scsi, bad, serd;		/* flags indicating disk type */
struct CuDv dev, parent_cudv;
struct PdDv parent_pddv;
int	rc, busid, slot, boot_ok, array_indx;
/* @@SSA_BEGIN@@ */
char ssa;
struct CuAt * cuat;
struct CuDv * cudv;
struct listinfo cuat_info;
int    i;
/* @@SSA_END@@ */

	/*
	 * ------- return values -----------
	 * 0 == CANNOT boot from that device
	 * 1 == CAN boot from that device
	 * ------- function design ---------
	 * check for device existence; if no such device, exit 0
	 * determine disk type and set boolean flag
	 *   if scsi, get parent device information, exit 0 on failure
	 * get iplcb; exit 0 on failure
	 * determine whether new or old ros
	 *   new: read adapter info and return supports_ipl field
	 *   old: regular scsi - supports boot
	 *	  bus attached disk - supports boot
	 *	  serial attached disk - not bootable
	 *	  SSA attached disk - not bootable
	 */

	/* assume device does NOT support boot */
	boot_ok = 0;

	/* Get CuDv for device */
	sprintf(sstr, "name=%s", name);
	rc = (int)odm_get_first(CuDv_CLASS,sstr, &dev);
	if (rc == 0 || rc == -1)
		return(boot_ok);

	/* Device must be a disk */
	if (strncmp(dev.PdDvLn_Lvalue,"disk/",5))
		return(boot_ok);

	/* Special case RSPC models */
	if (__rspc()) {
		/* Get CuDv for parent adapter */
		sprintf(sstr,"name=%s",dev.parent);
		rc = (int)odm_get_first(CuDv_CLASS,sstr,&parent_cudv);
		if (rc == 0 || rc == -1)
			return(boot_ok);

		/* Get PdDv for parent adapter */
		sprintf(sstr,"uniquetype=%s",parent_cudv.PdDvLn_Lvalue);
		rc = (int)odm_get_first(PdDv_CLASS,sstr,&parent_pddv);
		if (rc == 0 || rc == -1)
			return(boot_ok);

#ifdef BOOT_DEBUG
fprintf(stderr,"Parent name = %s\nParent uniquetype = %s\n",parent_cudv.name,parent_pddv.uniquetype);
#endif

		boot_ok = rspc_bootable_adap(&parent_cudv,&parent_pddv);
		return(boot_ok);
	}


/* @@SSA_BEGIN@@ */
	scsi = bad = serd = ssa = 0;	/* initialize all to false */
/* @@SSA_END@@ */
	if (!strncmp(dev.PdDvLn_Lvalue, "disk/scsi/", 10)) {
		scsi = 1;
		sprintf(sstr, "name='%s'", dev.parent);
		rc=(int)odm_get_first(CuDv_CLASS, sstr, &parent_cudv);
		if (rc == 0 || rc == -1)
			return(0);
	}
	else if (!strcmp(dev.PdDvLn_Lvalue, "disk/mca/badisk"))
		bad = 1;
	else if (!strncmp(dev.PdDvLn_Lvalue, "disk/serdasdc/", 13))
		serd = 1;
/* @@SSA_BEGIN@@ */
	else if( !strncmp( dev.PdDvLn_Lvalue, "disk/ssar/", 10 ) )
	{
		ssa = 1;
	}
/* @@SSA_END@@ */

	/*
	 * get directory section of IPL control block
	 * it starts at offset of 128 into IPL control block
	 */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) {
		perror("Error reading IPL-Ctrl block directory");
		return(boot_ok);  /* Does not support boot */
	}

	/*
	 * the ipl_directory varies in size, so the following calculation
	 * is intended to determine whether the iplcb_dir.global_offset
	 * field exists or not.  do this by comparing the address of the
	 * global_offset against the address of the ipl_info struct, which
	 * follows the ipl_directory in memory.  the address of the ipl_info
	 * struct is calculated by adding the address of the ipl_directory
	 * (cast int to prevent incrementing by size of the struct)
	 * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
	 * if the address of global_offset is less than the address of the
	 * ipl_info struct, assume existence and validity of the global_offset
	 */
	if ((int)(&iplcb_dir.global_offset) <
		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) {
		/*
		 * get GLOBAL Data structure.
		 */
		if (iplcb_get(&iplcb_glob,iplcb_dir.global_offset,
				sizeof(iplcb_glob),MIOIPLCB)) {
			perror("Error reading IPL-Ctrl block Global section");
			return(boot_ok);  /* Does not support boot */
		}
/* @@SSA_BEGIN@@ */
/************************************************************************/
/* For SSA hdisks we have to find one bootable adapter			*/
/************************************************************************/

		if( ssa == 1 )
		{
		    sprintf( sstr,
			     "name='%s' AND attribute LIKE 'adapter_?'",
			     name );

		    cuat = (struct CuAt *)odm_get_list( CuAt_CLASS,
							sstr,
							&cuat_info,
							1, 1 );

/************************************************************************/
/* Check each adapter in turn to see if it is bootable			*/
/************************************************************************/

		    for( i = 0; (i < cuat_info.num) && (boot_ok == 0); i++ )
		    {
			sprintf( sstr,
				 "name='%s' AND status='1'",
				 cuat[ i ].value );

			cudv = (struct CuDv *)odm_get_list( CuDv_CLASS,
							    sstr,
							    &cudv_info,
							    1, 1 );
			if( (int)cudv < 0 )
			{
			    return boot_ok;
			}
			else if( cudv != NULL )
			{
			    busid	= cudv->location[ 3 ] - '0';
			    slot	 = cudv->location[ 4 ] - '0' - 1;
			    array_indx   = busid * (MAX_SLOT_NUM +1) + slot;
			    adapter_info = (struct adapt_info *)
				    (&iplcb_glob.fm2_adapt_info[array_indx]);
			    boot_ok =
				(adapter_info->supports_ipl==0) ? 0 : 1;
			}
		    }

		    return boot_ok;
		}
		else
		{
		    slot = dev.location[4] - '0';  /* get odm slot number */
		    if( slot == 0) slot = 14;  /* integrated scsi ? */
		    slot--;		       /* adjust 1 for hw slot number */

		    busid = dev.location[3] - '0';

		    array_indx = busid * (MAX_SLOT_NUM +1) + slot;
		    adapter_info = (struct adapt_info *)
				    (&iplcb_glob.fm2_adapt_info[array_indx]);
		    boot_ok = (adapter_info->supports_ipl==0) ? 0 : 1;
		}
/* @@SSA_END@@    */

	}
	else {
		/*
		 * the area does not exist, and we need to use
		 * default values for scsi, bus attached, and serial
		 */
		if (scsi == 1)
			if (strcmp(parent_cudv.PdDvLn_Lvalue,
					"adapter/mca/hscsi"))
				boot_ok=0;
			else
				boot_ok=1;
		else if (bad == 1)
			boot_ok=1;
		else
			/* No other type of boot disk supported on this model */
			boot_ok=0;
	}
	return(boot_ok);
}

/* --------------------------------------------------------------------------*/
/*
 * ros_bootable_adap: For a given adapter, output 1 if ROS
 *		      supports booting from the adapter.
 *		      Otherwise it returns 0.
 *		      This is meant to be used by NIM for ROS emulation.
 */
int
ros_bootable_adap(char *adap)
{
IPL_DIRECTORY iplcb_dir;	/* IPL control block directory */
GLOBAL_DATA iplcb_glob;		/* IPL control blk GLOBAL DATA  */
struct  adapt_info *adapter_info;
char	sstr[256];
struct CuDv cudv;
struct PdDv pddv;
int	rc, busid, slot, boot_ok, array_indx;

	boot_ok = 0;   /* assume device does NOT support boot */
	sprintf(sstr, "name='%s'", adap);
	if( (rc = (int)odm_get_first(CuDv_CLASS,sstr, &cudv)) == -1 ) {
			perror("Error reading CuDv Object Class");
			exit(1);
	}
	if( !rc ) exit(1);

	sprintf(sstr, "uniquetype='%s'", cudv.PdDvLn_Lvalue);
	if( (rc = (int)odm_get_first(PdDv_CLASS,sstr, &pddv)) == -1 ) {
			perror("Error reading PdDv Object Class");
			exit(1);
	}
	if( !rc ) exit(1);

	/* Special case RSPC models */
	if (__rspc()) {
		boot_ok = rspc_bootable_adap(&cudv,&pddv);
		return(boot_ok);
	}

	/*  if subclass is not mca or sio, cannot boot from them */
	if( strcmp(pddv.subclass,"mca") && strcmp(pddv.subclass,"sio"))
		return(0);

	/* Some of the devices on sio have non-numeric connwhere values.
	   atoi() will return 0 and set errno for these, but slot will
	   work out ok for sio.  */
	slot = atoi(cudv.connwhere);
	busid = cudv.location[3] - '0';
	if( slot > 15 ) return(0); 			   /* sgabus */
	if( !slot) {
		/* return bootable for sio and diskette adapter */
		if( !strncmp(cudv.PdDvLn_Lvalue,"adapter/mca/sio",15) ||
		    !strncmp(cudv.PdDvLn_Lvalue,"adapter/sio/fda",15) )
			return(1); /* sio or fda adapter */
		else    return(0); /* other adapters at slot 0 is not bootable*/
	}
	slot--;				/* adjust 1 for hw slot number */

	/*
	 * get directory section of IPL control block
	 * it starts at offset of 128 into IPL control block
	 */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) {
		perror("Error reading IPL-Ctrl block directory");
		return(0);  /* Does not support boot */
	}

	if ((int)(&iplcb_dir.global_offset) <
		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) {
		/*
		 * get GLOBAL Data structure.
		 */
		if (iplcb_get(&iplcb_glob,iplcb_dir.global_offset,
				sizeof(iplcb_glob),MIOIPLCB)) {
			perror("Error reading IPL-Ctrl block Global section");
			return(0);  /* Does not support boot */
		}

		array_indx = busid * (MAX_SLOT_NUM +1) + slot;
		adapter_info = (struct adapt_info *)
				(&iplcb_glob.fm2_adapt_info[array_indx]);
		boot_ok = (adapter_info->supports_ipl==0) ? 0 : 1;
	}
	else {
		/* return bootable for  bluebonnet, lace or badisk */
		if( !strcmp(cudv.PdDvLn_Lvalue,"adapter/mca/hscsi") ||
		    !strcmp(cudv.PdDvLn_Lvalue,"disk/mca/badisk"))
			boot_ok = 1;
	}

	return(boot_ok);
}

/****************************************************************
* NAME: is_eserver
*
* FUNCTION: Determine if system is an entry server type system
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	100     (= machine is a entry server type system WITHOUT DIAGS)
*	101	(= machine is a entry server type system with DIAGS supported)
*	102	(= machine is NOT an entry server and is WITHOUT DIAGS)
*	103     (= machine is NOT an entry server with DIAGS supported)
*	-1	(= error occured so indeterminate)
*
****************************************************************/
#define POWERPC	   0x0800
#define RSC	   0x0200
#define MODEL_MASK 0x0003
#define TOWER	   0x0000
#define DESK	   0x0001
#define RACK	   0x0002
#define ENTRY      0x0003
#define DIAGS      0x000C
/* Unique type code out of bytes 8 and 9 in uname */
#define MOD_C10	   0x0048

short is_eserver(char *modeltype)
{
  int  rc;
  short ret_val;
  char criteria[80];
  char uniquestr[10];
  ulong unique_type;
  uint modelcode;
  struct CuAt *cuat;
  struct listinfo cuat_info;

	/****************************************************************
	* One of the custom attributes of "sys0" is the model number.	*
	* In this modelcode, the last 2 bits represents the type of	*
	* system, i.e., tower, desktop, or rack mounted.  The value 	*
	* of this attribute is a 2, 4 or 8 digit hex string, e.g. "0x01"*
	****************************************************************/
 	ret_val = 102;
	strcpy(modeltype,"");
	strcpy(criteria, "name=sys0 AND attribute=modelcode");
	if ((int )(cuat = get_CuAt_list(CuAt_CLASS, criteria,
		  &cuat_info, 1, 1)) < 0)
		return (-1);
	if (cuat_info.num != 1) return (-1);
	strncpy(modeltype,cuat->value,6);
	modelcode = (int )strtoul(cuat->value, (char **)NULL, 16);
	if (strlen(cuat->value) > 6)
		modelcode = modelcode >>16;
	if (modelcode & RSC )
		ret_val = 100;
	if ((modelcode & POWERPC ) && ((modelcode & MODEL_MASK) == DESK))
		ret_val = 100;
	if ((modelcode & POWERPC ) && ((modelcode & MODEL_MASK) == ENTRY))
		ret_val = 100;
	if ((modelcode & POWERPC ) && ((modelcode & MODEL_MASK) == TOWER))
	{
		if( rc=get_planar_id(&unique_type) == 0)
		{
			if (unique_type == MOD_C10)
				ret_val = 100;
			else
			{
				sprintf(uniquestr,"-%x",unique_type);
				strcat(modeltype,uniquestr);
			}

		}
		else
			ret_val = 100;
	}
	/* Increment return value to indicate diagnostics are supported */
	if (modelcode & DIAGS)
		ret_val += 1;
	odm_free_list(cuat, &cuat_info);
	return ret_val;
}

/*
 * NAME:  get_planar_id
 *
 * FUNCTION: get model number from uname stracture.
 *
 * DATA STRUCTURES:
 *	struct utsname: uname structure.
 *
 * RETURN VALUE DESCRIPTION:
 *	0:  success to get model number.
 *	-1: error
 */
int
get_planar_id(ulong *id)
{
	struct utsname	name;
	char unique_value[8];

	if(uname(&name) < 0){
		return -1;
	}else{
		unique_value[0]='\0';
		strcpy(unique_value,"0x");
		strncat(unique_value,&name.machine[8],2);
		*id = (int )strtoul(unique_value, (char **)NULL, 16);
		return 0;
	}
}


/* --------------------------------------------------------------------------*/
/*
 * rspc_bootable_adap: For a given adapter, output 1 if RSPC firmware
 *		      supports booting from the adapter.
 *		      Otherwise it returns 0.
 */
int
rspc_bootable_adap(cudv, pddv)
struct CuDv *cudv;
struct PdDv *pddv;

{
	CFG_DEVICE *devp;
	int	num;
	int	rc;
	int	index;

	/* get the number of devices and the device info from resid data */
	rc = get_resid_dev(&num, &devp);	/* libcfg function */
	if (rc == 0) {
		index = get_resid_index(cudv,pddv,devp,num);

#ifdef BOOT_DEBUG
fprintf(stderr, "Residual data device index = %d\n",index);
#endif
		if (index != -1) {
#ifdef BOOT_DEBUG
fprintf(stderr, "Residual data device flags = %x\n",devp[index].deviceid.flags);
#endif
			if (devp[index].deviceid.flags & 0x0200)
				return(1);
		}
	}

	return(0);
}

/* End of bootinfo.c */
