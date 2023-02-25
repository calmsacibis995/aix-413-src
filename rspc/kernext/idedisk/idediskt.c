static char sccsid[] = "@(#)53  1.11  src/rspc/kernext/idedisk/idediskt.c, diskide, rspc41J, 9520A_all 5/16/95 16:29:26";
/*
 * COMPONENT_NAME: (IDEDISK) IDE Disk Device Driver Top Half Routines
 *
 * FUNCTIONS:
 *	idedisk_config
 *	idedisk_open
 *	idedisk_close
 *	idedisk_read
 *	idedisk_write
 *	idedisk_rdwr
 *	idedisk_mincnt
 *	idedisk_ioctl
 *	idedisk_alloc
 *	idedisk_init_cmds
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Included System Files: */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/i_machine.h>
#include <sys/systm.h>
#include <sys/m_param.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/devinfo.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/lvdd.h>
#include <sys/errids.h>
#include <sys/dump.h>
#include <sys/ddtrace.h>

#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/iostat.h>
#include <sys/pm.h>

#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif

#include <sys/ide.h>
#include "idediskdd.h"
#include "idedisk_errids.h"

/* End of Included System Files */

/**************************************************************************
 *
 *                        Static Structures
 *
 *      The idedisk_list data structure is an array of pointers to
 *      idediskinfo structures describing each of the disk device's
 *      currently configured in the system.
 *
 *      The idedisk_open_list data structure is similar to the
 *      idedisk_list the only exception being that it contains only
 *      those disk devices currently open in the system.
 *
 *      The idedd_info data structure is a structure used for storing
 *      general disk driver information, including global states,
 *      configuration counts, global locks, etc.
 *
 *      The diskinfo data structures are allocated on a one per device
 *      basis and are used for storing device-specific information.
 *      These structures are dynamically allocated when a device is
 *      configured into the system, and deallocated when the device
 *      is deleted. These structures are linked into the disk_list
 *      upon configuration and linked into the disk_open_list during
 *      any outstanding opens.
 *
 ************************************************************************/

extern struct idedd_info	idedd_info;
extern struct idediskinfo	*idedisk_list[DK_MAXCONFIG];
extern struct idediskinfo	*idedisk_open_list[DK_MAXCONFIG];

#ifdef DEBUG
extern	int			idedisk_debug;
extern	struct idedisk_trace	*idedisk_trace;

/*
 ********  strings for Internal Debug Trace Table *********
 * To use compile with the flags IDE_GOOD_PATH and IDE_ERROR_PATH
 * the variable idedisk_trace will be the beginning of the trace table in
 * memory.  The variable idedisk_trctop will point to location where the
 * next entry in the trace table will go.
 */

char     *opentrc	= "OPEN	   ";
char	 *closetrc	= "CLOSE   ";
char	 *read		= "READ    ";
char	 *write		= "WRITE   ";
char	 *rdwr		= "RDWR    ";
char	 *mincnt	= "MINCNT  ";
char	 *ioctls	= "IOCTL   ";
char     *entry         = " IN";	/* Entry into routine                */
char     *exit          = " EX";	/* Exit from routine                 */
char     *trc           = " TR";	/* Trace point in routine            */

#endif


/************************************************************************/
/*									*/
/*	NAME:	idedisk_config						*/
/*									*/
/*	FUNCTION: IDE disk device driver configuration routine.		*/
/*		During device INIT, this routine allocates and		*/
/*		initializes data structures required for processing	*/
/*		user requests. This routine will also delete an already	*/
/*		defined device and free these structures when called	*/
/*		for the TERM operation.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		idedisk_list	List of configured devices		*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		op	- what function to perform (INIT or TERM)	*/
/*		uiop	- uio pointer to configuration data		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*			EBUSY   - Attempt to TERM an open device	*/
/*			EEXIST  - Devswadd failed because the device	*/
/*				  is already in the devsw table		*/
/*			EINVAL  - Unknown operation			*/
/*			EIO     - Minor number out of range		*/
/*			ENODEV	- Attempt to TERM a nonexistent device	*/
/*			ENOMEM	- Xmalloc or pin failed			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devswadd	devswdel				*/
/*		xmalloc		xmfree					*/
/*		uiomove		pm_register_handle			*/
/*		bcopy		bzero					*/
/*		minor		lockl					*/
/*		unlockl		iostadd					*/
/*		iostdel							*/
/*									*/
/************************************************************************/

int
idedisk_config(
	dev_t		devno,
	int		op,
	struct uio	*uiop)
{
	int			errnoval, dev, rc, dev_id;
	uint			cdt_size;
	struct idediskinfo	*diskinfo;
	struct idedisk_dds	config_info;
	struct devsw		devsw_struct;
	struct cdt		*old_cdt;

	extern		nodev();

#ifdef DEBUG
	printf("Entering idedisk_config devno[0x%x] op[0x%x]\n", devno, op);
#endif

	DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_CONFIG, 0, devno, op, uiop, 0, 0);
	errnoval = 0;
	dev = minor(devno);

	(void) lockl(&(idedd_info.lock), LOCK_SHORT);

	switch (op) {
	case CFG_INIT:
		idedd_info.config_cnt++;
		if (idedd_info.config_cnt == 0x01) {

			/* Initialize devsw struct for idedisk */
			devsw_struct.d_open	= (int(*)())idedisk_open;
			devsw_struct.d_close	= (int(*)())idedisk_close;
			devsw_struct.d_read	= (int(*)())idedisk_read;
			devsw_struct.d_write	= (int(*)())idedisk_write;
			devsw_struct.d_ioctl	= (int(*)())idedisk_ioctl;
			devsw_struct.d_strategy	= (int(*)())idedisk_strategy;
			devsw_struct.d_ttys	= (struct tty *)NULL;
			devsw_struct.d_select	= (int(*)())nodev;
			devsw_struct.d_config	= (int(*)())idedisk_config;
			devsw_struct.d_print	= (int(*)())nodev;
			devsw_struct.d_dump	= (int(*)())idedisk_dump;
			devsw_struct.d_mpx	= (int(*)())nodev;
			devsw_struct.d_revoke	= (int(*)())nodev;
			devsw_struct.d_dsdptr	= NULL;
			devsw_struct.d_selptr	= NULL;
			devsw_struct.d_opts	= 0;

			/* Add devsw struct to system devsw table */
			errnoval = devswadd(devno, &devsw_struct);
			if (errnoval) {
				idedd_info.config_cnt--;
				unlockl(&(idedd_info.lock));
				DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG,
				    errnoval, devno);
				return(errnoval);
			}
		}

		/* Search for diskinfo structure in configured list */
		diskinfo = idedisk_list[dev];

		/* Verify device not already configured */
		if (diskinfo != NULL) {
			idedd_info.config_cnt--;
			unlockl(&(idedd_info.lock));
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(EINVAL);
		}
		/* Allocate memory for diskinfo structure and zero it */
		diskinfo = (struct idediskinfo *)
			xmalloc((uint) sizeof(struct idediskinfo), 2, 
			(heapaddr_t) pinned_heap);
		if (diskinfo == NULL) {
			idedd_info.config_cnt--;
			if (idedd_info.config_cnt == 0) {
				(void) devswdel(devno);
			}
			unlockl(&(idedd_info.lock));
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(ENOMEM);
		}
		bzero((caddr_t) diskinfo, sizeof(struct idediskinfo));

		/*
		 *  Allocate memory for component dump table.  Three entries
		 *  are needed for the static structures, and then one
		 *  additional entry for each disk structure.
		 */
		 
		cdt_size = (uint) sizeof(struct cdt_head) + (3 + 
		    idedd_info.config_cnt) * (uint) sizeof(struct cdt_entry);
		old_cdt = idedd_info.cdt;
		idedd_info.cdt = (struct cdt *) xmalloc(cdt_size, 2, 
			(heapaddr_t) pinned_heap);
		if (idedd_info.cdt == NULL) {
			idedd_info.cdt = old_cdt;
			(void) xmfree((caddr_t)diskinfo, (heapaddr_t)
			    pinned_heap);
			idedd_info.config_cnt--;
			if (idedd_info.config_cnt == 0) {
				(void) devswdel(devno);
			}
			unlockl(&(idedd_info.lock));
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(ENOMEM);
		}
		/*  Free the old component dump table */
		(void) xmfree((caddr_t)old_cdt, (heapaddr_t) pinned_heap);

		bzero((caddr_t) idedd_info.cdt, cdt_size);
		idedd_info.cdt->_cdt_head._cdt_magic = DMP_MAGIC;
		bcopy("idedisk", idedd_info.cdt->_cdt_head._cdt_name, 8);
		idedd_info.cdt->_cdt_head._cdt_len = cdt_size;

		uiomove((caddr_t)&config_info, (int) sizeof(struct idedisk_dds),
		    (enum uio_rw) UIO_WRITE, uiop);

		/* Move config data into diskinfo structure */
#ifdef DEBUG
		printf("idecfg: adap devno = %x\n",config_info.adapter_devno);
		printf("idecfg: device # = %x\n",config_info.device_id);
		printf("idecfg: block_size = %x\n",config_info.block_size);
		printf("idecfg: total_sectors = %x\n",config_info.total_sectors);
		printf("idecfg: chs_only = %x\n",config_info.chs_only);
#endif
		diskinfo->devno = devno;
		diskinfo->adapter_devno = config_info.adapter_devno;
		diskinfo->device_id = config_info.device_id;
		diskinfo->max_coalesce = config_info.max_coalesce;
		diskinfo->cmds_out = 0;
		diskinfo->max_request = config_info.max_request;
		diskinfo->dma_supported = config_info.dma_supported;
		diskinfo->read_cmd = config_info.read_cmd;
		diskinfo->write_cmd = config_info.write_cmd;
		diskinfo->rw_timeout = config_info.rw_timeout;
		diskinfo->start_timeout = config_info.start_timeout;
                diskinfo->cfg_block_size = config_info.block_size;
		diskinfo->chs_only = config_info.chs_only;
		diskinfo->capacity.lba = config_info.total_sectors - 1;
		diskinfo->capacity.len = config_info.block_size;
		diskinfo->sectors_per_track = config_info.sectors_per_track;
		diskinfo->num_heads = config_info.num_heads;
		/*
		 * Assume this disk is ATA-2 compliant and, thus, supports
		 * the Read-Verify command.
		 */
		diskinfo->verify_flags = VERIFY_SUPPORTED;

		/*
		 * Start out with with the block_size specified 
		 * by the configuration method.
		 */
		diskinfo->block_size = diskinfo->cfg_block_size;
	
		/* Initialize remaining sections of disk info structure */
		diskinfo->dk_cmd_q_head = NULL;
		diskinfo->dk_cmd_q_tail = NULL;

		diskinfo->currbuf = NULL;
		diskinfo->low = NULL;
		diskinfo->dk_bp_queue.in_progress.head = NULL;
		diskinfo->dk_bp_queue.in_progress.tail = NULL;
		diskinfo->state = DK_OFFLINE;
		diskinfo->opened = FALSE;

		diskinfo->cmd_pending = FALSE;
		diskinfo->starting_close = FALSE;
		diskinfo->lock = LOCK_AVAIL;
		diskinfo->fp = NULL;
		diskinfo->dump_inited = FALSE;

		/* initialize fields for watchdog timer */
		diskinfo->pm_timer.watch.next = NULL;
		diskinfo->pm_timer.watch.prev = NULL;
		diskinfo->pm_timer.watch.func = (void (*)())idedisk_pm_watchdog;
		diskinfo->pm_timer.watch.count = 0;
		diskinfo->pm_timer.watch.restart = 2;
		diskinfo->pm_timer.pointer = (void *)diskinfo;

                w_init( (struct watchdog *) &diskinfo->pm_timer);

		/* Initialize cmd structs for use */
		idedisk_init_cmds(diskinfo);

		/* Initialize drive statistics */
		diskinfo->stats.segment_size = config_info.segment_size;
		diskinfo->stats.segment_count = config_info.segment_cnt;
		diskinfo->stats.byte_count = config_info.byte_count;

		/* Initialize drive error record */
		bcopy(config_info.resource_name,
			diskinfo->error_rec.entry.resource_name, 8);

		/* fill out fields in I/O statistics structure */
		bzero (&(diskinfo->dkstat), sizeof(struct dkstat));
		diskinfo->dkstat.dk_bsize = diskinfo->block_size;
		bcopy(config_info.resource_name, diskinfo->dkstat.diskname, 8);

		/* Register io statistics tracking structure */
		rc = iostadd(DD_DISK, &(diskinfo->dkstat));
		ASSERT(rc == 0);

		/* Link diskinfo structure in configured device list */
		idedisk_list[dev] = diskinfo;

#ifdef DEBUG
		/*
		 * Malloc Trace Buffer one time only, this is
		 * so we can have the buffer more readable 
		 * 32-byte aligned
		 */
		if (idedd_info.config_cnt == 0x01) {
			idedisk_trace=(struct idedisk_trace *)xmalloc(
			(uint)(sizeof(struct idedisk_trace) * TRCLNGTH), 
			 5, (heapaddr_t)pinned_heap);
		}
#endif
		diskinfo->pm_event = EVENT_NULL;

		/* Initialize Power Management handle */
		diskinfo->pmh.handle.pm_version = PM_PHASE2_SUPPORT;
		diskinfo->pmh.handle.activity = PM_ACTIVITY_OCCURRED;
		diskinfo->pmh.handle.mode = PM_DEVICE_FULL_ON;
		diskinfo->pmh.handle.device_idle_time =
					config_info.pm_dev_itime;
		diskinfo->pmh.handle.device_standby_time =
					config_info.pm_dev_stime;
		diskinfo->pmh.handle.handler = idedisk_pm_handler;
		diskinfo->pmh.handle.private = (uchar *)&diskinfo->pmh;
		diskinfo->pmh.handle.devno = devno;
		diskinfo->pmh.handle.attribute = config_info.pm_dev_att;
		diskinfo->pmh.handle.device_idle_time1 = 0;
		diskinfo->pmh.handle.device_idle_time2 = 0;
		diskinfo->pmh.handle.device_logical_name =
						diskinfo->dkstat.diskname;
		diskinfo->pmh.diskinfo = diskinfo;

		/* Register Power Management handle */
		rc = pm_register_handle((struct pm_handle *)&(diskinfo->pmh),
					PM_REGISTER);

		ASSERT(rc == PM_SUCCESS);

		break;
	case CFG_TERM:
		/* Search for diskinfo structure in configured device list */
		diskinfo = idedisk_list[dev];
		if (diskinfo == NULL) {
			unlockl(&(idedd_info.lock));
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG, 0, devno);
			return(ENODEV);
		}
		if (diskinfo->opened) {
			unlockl(&(idedd_info.lock));
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(EBUSY);
		}

		idedisk_list[dev] = NULL;

		idedd_info.config_cnt--;
		/* Check for last device being deleted */
		if (idedd_info.config_cnt == 0) {
			(void) devswdel(devno);
			(void) xmfree((caddr_t)idedd_info.cdt,
				      (heapaddr_t) pinned_heap);
			idedd_info.cdt = NULL;
#ifdef DEBUG
			xmfree((caddr_t)idedisk_trace,(heapaddr_t)pinned_heap);
#endif
		} else {
			/*
			 *  Allocate memory for component dump table.  Three
			 *  entries are needed for the static structures, and
			 *  then one additional entry for each disk structure.
			 */
		 
			cdt_size = (uint) sizeof(struct cdt_head) + (3 + 
			    idedd_info.config_cnt) * (uint) sizeof(struct 
			    cdt_entry);
			old_cdt = idedd_info.cdt;
			idedd_info.cdt = (struct cdt *) xmalloc(cdt_size, 2,
			    (heapaddr_t) pinned_heap);
			if (idedd_info.cdt == NULL) {
				idedd_info.cdt = old_cdt;
			} else {
				(void) xmfree((caddr_t)old_cdt,
				    (heapaddr_t) pinned_heap);
			}
			bzero((caddr_t) idedd_info.cdt, cdt_size);
			idedd_info.cdt->_cdt_head._cdt_magic = DMP_MAGIC;
			bcopy("idedisk",
				idedd_info.cdt->_cdt_head._cdt_name, 8);
			idedd_info.cdt->_cdt_head._cdt_len = cdt_size;
		}

		w_clear( (struct watchdog *) &diskinfo->pm_timer);
		
		/* Unregister Power Management handle */
		pm_register_handle((struct pm_handle *)&(diskinfo->pmh),
					PM_UNREGISTER);

		/* de-register the dkstat structure */
		iostdel(&(diskinfo->dkstat));
		(void) xmfree((caddr_t)diskinfo, (heapaddr_t)pinned_heap);
		/*
		 *  The unconfigure method checks for EBUSY to see if the
		 *  device unconfigured the device or not.  If we got this
		 *  far, we must have unconfigured it, so don't return
		 *  EBUSY (EIO will basically be ignored).
		 */
		if (errnoval == EBUSY) {
			errnoval = EIO;
		}
		break;

	default:
		/* Unknown op */
		errnoval = EINVAL;
	}
#ifdef DEBUG
	printf("Exiting idedisk_config errnoval[0x%x]\n", errnoval);
#endif
	unlockl(&(idedd_info.lock));
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CONFIG, errnoval, devno);
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_open						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Open Routine.			*/
/*		This routine prepares a device for utilization by the	*/
/*		system. The disk's information structure is located	*/
/*		in the idedisk_list.					*/
/*		The open is checked for validity, and if the first	*/
/*		valid open, the diskinfo structure is pinned and placed	*/
/*		on the idedisk_open_list. The adapter driver is called	*/
/*		to open and start the device.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo		Disk device specific information*/
/*		idedisk_list		List of configured devices	*/
/*		idedisk_open_list	List of open devices		*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		rwflag	- FREAD for read only, FWRITE for read/write	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter, only 0 is valid		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*			EINVAL	- ext non-zero, OPENX is not supported	*/
/*			EIO	- Minor number out of range		*/
/*				- Device not configured			*/
/*				- Devopen to adapter failed		*/
/*				- Devioctl to start device failed	*/
/*			ENODEV	- Invalid devno				*/
/*				- Device not configured			*/
/*			ENFILE	- Indicates that the system file table	*/
/*				  is full				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		fp_opendev	fp_close				*/
/*		fp_ioctl	xmfree					*/
/*		pincode		unpincode				*/
/*		dmp_add		dmp_del					*/
/*		minor							*/
/*									*/
/************************************************************************/

int
idedisk_open(
	dev_t	devno,
	int	rwflag, 
	int	chan, 
	int	ext)
{
	int			errnoval, rc, dev, dev_id;
	struct idediskinfo	*diskinfo;
	struct devinfo		iocinfo_buf;

	DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_OPEN, 0, devno, rwflag, chan, ext,
	    0);
	errnoval = 0;
	dev = minor(devno);

	(void) lockl(&(idedd_info.lock), LOCK_SHORT);

	/* Locate diskinfo structure in device list */
	diskinfo = idedisk_list[dev];
	if (diskinfo == NULL) {
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, ENODEV, devno);
		return(ENODEV);
	}

	if (!ext) {
		/*
		 * This is a normal open (not OPENX)
		 */
		if (diskinfo->opened == TRUE) {
			unlockl(&(idedd_info.lock));
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, 0,
				devno);
			return(0);
		}
	} else {
		/*
		 * OPENX not supported
		 */
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, EINVAL, devno);
		return(EINVAL);
	}


	/*
	 * Start out with with the block_size specified 
	 * by the configuration method.
	 */
	diskinfo->block_size = diskinfo->cfg_block_size;

	/*
	 * Allocate command block pool
	 */

	if ((rc = idedisk_alloc(diskinfo))) {
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, EINVAL, devno);
		return(rc);
	}

	/* Open adapter for device and store filepointer */
	errnoval = fp_opendev(diskinfo->adapter_devno, (int) FWRITE,
	    (caddr_t) NULL, (int) NULL, &(diskinfo->fp));
	if (errnoval != 0) {
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}

	dev_id = diskinfo->device_id;

	/*
	 * Issue IOCINFO to the adapter driver to find out the maximum request
	 * size.
	 */
	errnoval = fp_ioctl(diskinfo->fp, (uint) IOCINFO,
			(caddr_t) &iocinfo_buf, (int) NULL);
	if (errnoval != 0) {
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	} else {

		/*
	         *  Max_request can not exceed adapter's max transfer size
	         */
		if (diskinfo->max_request > iocinfo_buf.un.ide.max_transfer) 
			diskinfo->max_request =
					iocinfo_buf.un.ide.max_transfer;
	}

	if (diskinfo->max_coalesce > diskinfo->max_request) {
		diskinfo->max_coalesce = diskinfo->max_request;
	}

	/* Issue a start IOCTL to the adapter driver */
	errnoval = fp_ioctl(diskinfo->fp, (uint) IDEIOSTART, (caddr_t) dev_id,
	    (int) NULL);
	if (errnoval != 0) {
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}

	/* pin bottom half routines */
	errnoval = pincode((int(*)())idedisk_iodone);
	if (errnoval) {
		fp_ioctl(diskinfo->fp, (uint) IDEIOSTOP, (caddr_t) dev_id,
		    (int) NULL);
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(opentrc, trc, (char) 0, (uint) devno, (uint) rwflag,
		(uint) chan, (uint) ext, (uint) 0);
#endif	
#endif
#endif
	/*  Register component dump table function on first open */
	if (idedd_info.open_cnt == 0) {
		(void) dmp_add(idedisk_cdt_func);
	}

	/* No need to do equivalent of Start Unit.  Disk will spin-up */
	/* upon receipt of any command. */

	/* Place diskinfo struct in idedisk_open_list */
	idedisk_open_list[dev] = diskinfo;
	diskinfo->state = DK_ONLINE;
	diskinfo->opened = TRUE;
	idedd_info.open_cnt++;

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(opentrc, trc, (char) 1, (uint) errnoval, 
		(uint) 0, (uint) 0, (uint) 0, (uint) 0);
#endif	
#endif
#endif
	unlockl(&(idedd_info.lock));
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_OPEN, errnoval, devno);

	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_close						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Close Routine.			*/
/*		This routine searches the idedisk_open_list for		*/
/*		the specified device, and unlinks it when found.	*/
/*		The device is then released for other initiators.	*/
/*		The device's diskinfo structure is unpinned from	*/
/*		memory, device open flag is cleared, and the		*/
/*		adapter driver is called via fp_ioctl to stop the	*/
/*		device and via fp_close to close if necessary.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		idedisk_list	List of configured devices		*/
/*		idedisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*			EFAULT	- copyin or copyout failed		*/
/*			EIO     - Minor number out of range		*/
/*				- Device not configured			*/
/*				- Device not open			*/
/*			ENXIO 	- Invalid devno				*/
/*				- Device not configured			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		xmfree		unpincode				*/
/*		fp_ioctl	fp_close				*/
/*		delay		minor					*/
/*		dmp_del							*/
/*									*/
/************************************************************************/

int
idedisk_close(
	dev_t	devno,
	int	chan, 
	int	ext)
{
	int			errnoval, dev, rc, dev_id;
	struct idediskinfo	*diskinfo;
	short			i,index;
	struct dk_cmd		*cmd_ptr;


	DDHKWD1(HKWD_DD_IDEDISKDD, DD_ENTRY_CLOSE, 0, devno);
	errnoval = 0;
	dev = minor(devno);


	(void) lockl(&(idedd_info.lock), LOCK_SHORT);
#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(closetrc, trc, (char) 0, (uint) devno, (uint) 0,
		(uint) chan, (uint) ext, (uint) 0);
#endif	
#endif
#endif
	/* Locate diskinfo structure in idedisk_open_list */
	diskinfo = idedisk_open_list[dev];

	if (diskinfo == NULL) {
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CLOSE, ENXIO, devno);
		return(ENXIO);
	}

	/* Unlink diskinfo struct from idedisk_open_list */
	idedisk_open_list[dev] = NULL;

	/*
	 * Lock the disk lock for this device to wait for a request to complete
	 */
	(void) lockl(&(diskinfo->lock), LOCK_SHORT);

	/*
	 * Set the following flags to prevent any new commands from
	 * being issued to the device driver
	 */

	diskinfo->opened = FALSE;
	diskinfo->starting_close = TRUE;
	diskinfo->state = DK_OFFLINE;

	/* Check for requests awaiting completion */

	while (diskinfo->cmds_out || 
	       (diskinfo->dk_bp_queue.in_progress.head != NULL) ||
	       (diskinfo->currbuf != NULL) ||
	       diskinfo->cmd_pending || 
	       diskinfo->dump_inited) {

		if (diskinfo->dk_cmd_q_head != NULL) {
			cmd_ptr = diskinfo->dk_cmd_q_head;
		}

		/*
		 * We will wait until all outstanding commands complete, 
		 * all pending commands (i.e. bufs and error recovery
		 * commands) get issued and complete, and for a dump to 
		 * to complete.
		 */
		delay(HZ);  /* wait one second for operation to complete */
	}

	diskinfo->starting_close = FALSE;

	/* Terminate adapter connection for this device */
	dev_id = diskinfo->device_id;
	rc = fp_ioctl(diskinfo->fp,(uint)IDEIOSTOP,(caddr_t)dev_id,(int)NULL);
	errnoval = (errnoval == 0) ? rc : errnoval;
	rc = fp_close(diskinfo->fp);
	errnoval = (errnoval == 0) ? rc : errnoval;
	idedd_info.open_cnt--;
	if (idedd_info.open_cnt == 0) {
		(void) dmp_del(idedisk_cdt_func);
	}
	(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(closetrc, trc, (char) 1, (uint) devno, (uint) errnoval,
		(uint) rc, (uint) 0, (uint) 0);
#endif
#endif	
#endif
	rc = unpincode((int(*)())idedisk_iodone);
	errnoval = (errnoval == 0) ? rc : errnoval;

	unlockl(&(diskinfo->lock));
	unlockl(&(idedd_info.lock));
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_CLOSE, errnoval, devno);
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_read						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Raw Read Routine.		*/
/*		This routine processes raw I/O requests from the system.*/
/*		A buf struct is allocated, and the b_options field	*/
/*		is initialized with the ext argument. Uphysio is called	*/
/*		to begin processing of the raw IO request.		*/
/*		Upon completion of the operation, the buf struct is	*/
/*		freed.							*/
/*                                                                      */
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- Address of uio struct describing request	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the value received from uphysio.	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*              See ERROR DESCRIPTION for idedisk_rdwr			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

int
idedisk_read(
	dev_t		devno,
	struct uio	*uiop,
	int		chan, 
	int		ext)
{
	int			errnoval;
	dev_t			dev;
	struct idediskinfo	*diskinfo;

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(read, entry, (char) 0, (uint) devno, (uint) uiop,
		(uint) chan, (uint) ext, (uint) 0);
#endif	
#endif
#endif
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_ENTRY_READ, 0, devno);

	errnoval = idedisk_rdwr(devno,uiop,chan,ext,B_READ);

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(read, exit, (char) 0, (uint) devno, (uint) uiop,
		(uint) chan, (uint) ext, (uint) errnoval);
#endif
#endif
#endif

	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_READ, errnoval, devno);
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_write						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Raw Write Routine.		*/
/*		This routine processes raw IO requests from the system.	*/
/*		A buf struct is allocated, and the b_options field	*/
/*		is initialized with the ext argument. Uphysio is called	*/
/*		to begin processing of the raw IO request.		*/
/*		Upon completion of the operation, the buf struct is	*/
/*		freed.							*/
/*                                                                      */
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- Address of uio struct describing request	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the value received from uphysio	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*              See ERROR DESCRIPTION for idedisk_rdwr			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

int
idedisk_write(
	dev_t		devno,
	struct uio	*uiop,
	int		chan, 
	int		ext)
{
	int		errnoval;

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(write, entry, (char) 0, (uint) devno, (uint) uiop,
		(uint) chan, (uint) ext, (uint) 0);
#endif
#endif	
#endif
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_ENTRY_WRITE, 0, devno);

	errnoval = idedisk_rdwr(devno,uiop,chan,ext,B_WRITE);

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(write, exit, (char) 0, (uint) devno, (uint) uiop,
		(uint) chan, (uint) ext, (uint) errnoval);
#endif	
#endif
#endif

	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_WRITE, errnoval, devno);
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_rdwr						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Raw Read/Write Routine.	*/
/*		This routine processes raw IO requests from the system.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- Address of uio struct describing request	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*		rw_flag - Equals B_WRITE for writes and B_READ for reads*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the value received from uphysio	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			ENXIO  - No open disk for specified device num	*/
/*			EIO    - I/O Failed.				*/
/*			       - The device is offline.			*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                             - Invalid access authority		*/
/*		               - Pinning of user's buffer failed.	*/
/*      		EAGAIN - Pin count exceeded on buffer		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		uphysio		minor					*/
/*									*/
/************************************************************************/

int
idedisk_rdwr(
	dev_t		devno,
	struct uio	*uiop,
	int		chan, 
	int		ext,
	int		rw_flag)
{
	int			errnoval, dev;
	struct idediskinfo	*diskinfo;

        /* 
	 * Lock to access structure 
	 */
        (void) lockl(&(idedd_info.lock),LOCK_SHORT);

        /* 
	 * Need to determine diskinfo structure for devno in order 
         * to tell if the block size is 512
	 */
        dev = minor(devno);
        diskinfo = idedisk_open_list[dev];

	/*
         * Check to be sure we found a diskinfo structure.
	 */
        if (diskinfo == NULL) {
                /* 
		 * No open disk at this devno 
		 */ 
                unlockl(&(idedd_info.lock));
                return(ENXIO);
        }               

        /* 
	 * Lock  the device's lock 
	 */
        (void) lockl(&(diskinfo->lock), LOCK_SHORT);

        /* 
	 * Finished with idedisk_open_list 
	 */
        unlockl(&(idedd_info.lock));

	if (diskinfo->block_size == DK_BLOCKSIZE) { 

	 	/* 
	 	 * Check to see if the current block size is 512 bytes 
	 	 */

                /* 
		 * Give up our lock 
		 */
                unlockl(&(diskinfo->lock));
#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(rdwr, trc, (char) 0, (uint) devno, (uint) uiop,
		(uint) chan, (uint) ext, (uint) diskinfo->block_size);
#endif
#endif	
#endif
                /* 
		 * Process the 512 blocksize request using uphysio 
		 */
                errnoval = uphysio(uiop, rw_flag, 9, devno, idedisk_strategy,
	                           idedisk_mincnt, ext);
        } else { 

                /* 
		 * Device is using a block size other then 512 bytes
		 * per block.  We do not support this.
		 */
#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(rdwr, trc, (char) 1, (uint) devno, (uint) uiop,
		(uint) chan, (uint) ext, (uint) diskinfo->block_size);
#endif
#endif	
#endif
                errnoval = EINVAL;

                /* 
		 * Give up our lock 
		 */
                unlockl(&(diskinfo->lock));
        }

	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_mincnt						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is used to breakup raw requests for	*/
/*		processing by uphysio.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		bp		- Pointer to buf structure for request	*/
/*		minparms	- Parameters for breakup		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno value may be returned:		*/
/*			ENXIO   - Device not open			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/************************************************************************/

int
idedisk_mincnt(
	struct buf	*bp,
	void		*minparms)
{
	struct idediskinfo	*diskinfo;	/* disk information structure*/

#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(mincnt, entry, (char) 0, (uint) bp, (uint) bp->b_bcount,
		(uint) minparms, (uint) 0, (uint) 0);
#endif	
#endif
#endif
	diskinfo = idedisk_open_list[minor(bp->b_dev)];
	if (diskinfo == NULL) {
		return(ENXIO);
	}

	if (bp->b_bcount > diskinfo->max_request)
		/*
		 * If this buf is bigger then our max transfer
		 * size then change count to our max transfer
		 */
		bp->b_bcount = diskinfo->max_request;

	bp->b_options = (int) minparms;
#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(mincnt, exit, (char) 0, (uint) bp, (uint) bp->b_bcount,
		(uint) bp->b_options, (uint) 0, (uint) 0);
#endif	
#endif
#endif
	return(0);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_ioctl						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver IOCTL Routine.			*/
/*		This routine supports the following ioctl commands:	*/
/*									*/
/*		IOCINFO	  - Returns info about the device such as block */
/*			    size and number of blocks.			*/
/*		SEL_DMA_PIO -						*/
/*			    Allows the caller to select whether DMA or	*/
/*			    PIO transfers should be used.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	  Disk device specific information	*/
/*		idedisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		op	- operation to be performed			*/
/*		arg	- address of user argument structure		*/
/*		devflag	- File parameter word				*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*			EINVAL	- Invalid parameter in request		*/
/*			ENXIO   - Device not open			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		bcopy		copyout					*/
/*		minor							*/
/*									*/
/************************************************************************/

int
idedisk_ioctl(
	dev_t	devno,
	int	op, 
	int	arg, 
	int	devflag, 
	int	chan, 
	int	ext)
{
	int			errnoval, dev;
	struct idediskinfo	*diskinfo;

	DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_IOCTL, 0, devno, op, arg, 0, 0);

	errnoval = 0;

	/* 
	 * Lock the disk driver's common lock 
	 */
	(void) lockl(&(idedd_info.lock), LOCK_SHORT);
#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(ioctls, trc, (char) 0, (uint) devno, (uint) op,
		(uint) arg, (uint) devflag, (uint) chan);
#endif	
#endif
#endif
	/* 
	 * Get diskinfo structure from open list 
	 */
	dev = minor(devno);
	diskinfo = idedisk_open_list[dev];
	if (diskinfo == NULL) {
		unlockl(&(idedd_info.lock));
		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_IOCTL, ENXIO, devno);
		return(ENXIO);
	}

	/* 
	 * Lock the device's lock 
	 */
	(void) lockl(&(diskinfo->lock), LOCK_SHORT);

	/* 
	 * Unlock the disk driver's common lock 
	 */
	unlockl(&(idedd_info.lock));

	/* 
	 * Determine operation to perform 
	 */
	switch (op) {
	case IOCINFO:
		{
			struct devinfo  d_info;
			
			d_info.devtype = DD_SCDISK;
			d_info.flags = (uchar) (DF_RAND | DF_FAST | DF_FIXED);
			d_info.devsubtype = (uchar) DS_PV;
			d_info.un.idedk.blksize = diskinfo->block_size;
			d_info.un.idedk.numblks = diskinfo->capacity.lba + 1;
			d_info.un.idedk.max_request = diskinfo->max_request;
			d_info.un.idedk.byte_count = diskinfo->stats.byte_count;
			d_info.un.idedk.segment_size = 
					diskinfo->stats.segment_size;
			d_info.un.idedk.segment_count = 
					diskinfo->stats.segment_count;
			COPY_TO_ARG( &d_info, sizeof(struct devinfo) );
			break;
		}

#ifdef DEV_IOCTL
	case SEL_DMA_PIO:
		switch (*((uint *)arg)) {
		case IDE_DISK_DMA:
			if (diskinfo->dma_supported) {
				diskinfo->read_cmd = ATA_READ_DMA_RETRY;
				diskinfo->write_cmd = ATA_WRITE_DMA_RETRY;
			} else {
				errnoval = EINVAL;
			}
			break;
		case IDE_DISK_PIO:
			diskinfo->read_cmd = ATA_READ_SECTOR_RETRY;
			diskinfo->write_cmd = ATA_WRITE_SECTOR_RETRY;
			break;
		default:
			errnoval = EINVAL;
		}
		break;
#endif
	default:
		errnoval = EINVAL;
	}
#ifdef DEBUG
#ifdef IDE_GOOD_LOCK_PATH
#ifdef IDE_GOOD_PATH
   idedisk_trc_disable(ioctls, trc, (char) 1, (uint) devno, (uint) op,
		(uint) arg, (uint) devflag, (uint) errnoval);
#endif	
#endif
#endif

	unlockl(&(diskinfo->lock));
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_IOCTL, errnoval, devno);
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_alloc						*/
/*									*/
/*	FUNCTION: Allocates memory from the pinned heap for		*/
/*		  buf command blocks.  It also initializes the		*/
/*		  command blocks					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		dk_cmd	        Disk device command block               */
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*			ENOMEM  - Xmalloc failed			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		xmalloc			bzero				*/
/*									*/
/************************************************************************/

int
idedisk_alloc(struct idediskinfo *diskinfo)
{
	int 	i;				/* general counter 	*/

	/* 
	 * Allocate cmd blocks for buf's
	 */
	diskinfo->cmd_pool = (struct dk_cmd *) xmalloc(
				(uint) (sizeof(struct dk_cmd)),
				2, pinned_heap);

	if (diskinfo->cmd_pool == NULL) {
		/*
		 * Xmalloc failed
		 */
		return(ENOMEM);
	}

	/*
	 * Initialize_cmds
	 */
	bzero((caddr_t) diskinfo->cmd_pool, (sizeof(struct dk_cmd)));

	diskinfo->cmd_pool->status = DK_FREE;
	diskinfo->cmd_pool->diskinfo = diskinfo;
	diskinfo->cmd_pool->idebuf.bufstruct.b_event = EVENT_NULL;

	diskinfo->pool_index = 0;

	return(0);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_init_cmds					*/
/*									*/
/*	FUNCTION: This routine initializes the diskinfo structure's    	*/
/*		  commands at config init time.			       	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		None							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

void
idedisk_init_cmds(struct idediskinfo *diskinfo)
{
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;


        diskinfo->pm_cmd.type = DK_PM; 
        diskinfo->pm_cmd.subtype = 0;  
	diskinfo->pm_cmd.status = DK_FREE;
	diskinfo->pm_cmd.diskinfo = diskinfo;
	diskinfo->pm_cmd.retry_count = 0;
	diskinfo->pm_cmd.soft_resid = 0;
	diskinfo->pm_cmd.segment_count = 0;
	diskinfo->pm_cmd.group_type = 0;
	diskinfo->pm_cmd.aborted = FALSE;
	diskinfo->pm_cmd.queued = FALSE;
	diskinfo->pm_cmd.error_type = 0;
        diskinfo->pm_cmd.next = NULL;
        diskinfo->pm_cmd.prev = NULL;    

	/* Initialize cmd block for Power Management cmd */
	cmd_bp = &(diskinfo->pm_cmd.idebuf.bufstruct);
	bzero((caddr_t) cmd_bp, sizeof(struct buf));

	cmd_bp->b_event = EVENT_NULL;
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_iodone = ((void(*)())idedisk_iodone);

	diskinfo->pm_cmd.idebuf.timeout_value = diskinfo->start_timeout;
	diskinfo->pm_cmd.idebuf.bp = NULL;
	diskinfo->pm_cmd.idebuf.status_validity = 0;
	diskinfo->pm_cmd.bp = cmd_bp;

	ata = &(diskinfo->pm_cmd.idebuf.ata);
	bzero((caddr_t) ata, sizeof(struct ata_cmd));

	ata->device = diskinfo->device_id;


        diskinfo->verify_cmd.type = DK_VERIFY; 
        diskinfo->verify_cmd.subtype = 0;  
	diskinfo->verify_cmd.status = DK_FREE;
	diskinfo->verify_cmd.diskinfo = diskinfo;
	diskinfo->verify_cmd.retry_count = 0;
	diskinfo->verify_cmd.soft_resid = 0;
	diskinfo->verify_cmd.segment_count = 0;
	diskinfo->verify_cmd.group_type = 0;
	diskinfo->verify_cmd.aborted = FALSE;
	diskinfo->verify_cmd.queued = FALSE;
	diskinfo->verify_cmd.error_type = 0;
        diskinfo->verify_cmd.next = NULL;
        diskinfo->verify_cmd.prev = NULL;    

	/* Initialize cmd block for Read-Verify cmd */
	cmd_bp = &(diskinfo->verify_cmd.idebuf.bufstruct);
	bzero((caddr_t) cmd_bp, sizeof(struct buf));

	cmd_bp->b_event = EVENT_NULL;
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_iodone = ((void(*)())idedisk_iodone);

	diskinfo->verify_cmd.idebuf.timeout_value = diskinfo->rw_timeout;
	diskinfo->verify_cmd.idebuf.bp = NULL;
	diskinfo->verify_cmd.idebuf.status_validity = 0;
	diskinfo->verify_cmd.bp = cmd_bp;

	ata = &(diskinfo->verify_cmd.idebuf.ata);
	bzero((caddr_t) ata, sizeof(struct ata_cmd));

	ata->device = diskinfo->device_id;
	ata->command = ATA_READ_VERIFY_SECTOR_RETRY;


        diskinfo->dmp_cmd.type = DK_BUF;
        diskinfo->dmp_cmd.subtype = 0;  
	diskinfo->dmp_cmd.status = DK_FREE;
	diskinfo->dmp_cmd.diskinfo = diskinfo;
	diskinfo->dmp_cmd.retry_count = 0;
	diskinfo->dmp_cmd.soft_resid = 0;
	diskinfo->dmp_cmd.segment_count = 0;
	diskinfo->dmp_cmd.group_type = 0;
	diskinfo->dmp_cmd.aborted = FALSE;
	diskinfo->dmp_cmd.queued = FALSE;
	diskinfo->dmp_cmd.error_type = 0;
        diskinfo->dmp_cmd.next = NULL;
        diskinfo->dmp_cmd.prev = NULL;

	/* Initialize cmd block for dump's Write cmd */
	cmd_bp = &(diskinfo->dmp_cmd.idebuf.bufstruct);
	bzero((caddr_t) cmd_bp, sizeof(struct buf));

	cmd_bp->b_event = EVENT_NULL;
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_iodone = ((void(*)())idedisk_iodone);

	diskinfo->dmp_cmd.idebuf.timeout_value = diskinfo->start_timeout;
	diskinfo->dmp_cmd.idebuf.bp = NULL;
	diskinfo->dmp_cmd.idebuf.status_validity = 0;
	diskinfo->dmp_cmd.bp = cmd_bp;

	ata = &(diskinfo->dmp_cmd.idebuf.ata);
	bzero((caddr_t) ata, sizeof(struct ata_cmd));

	ata->device = diskinfo->device_id;
	ata->command = diskinfo->write_cmd;

	return;
}

