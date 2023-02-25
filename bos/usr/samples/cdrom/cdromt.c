static char sccsid[] = "@(#)45	1.2  src/bos/usr/samples/cdrom/cdromt.c, cdrmsamp, bos411, 9428A410j 10/16/90 16:40:52";
/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
*/
/*
 * COMPONENT_NAME: SCSI CDROM Device Driver Top Half Routines
 *
 * FUNCTIONS:   cd_config               cd_open
 *              cd_close                cd_read
 *              cd_ioctl                cd_mincnt
 *
 * ORIGINS: 27
 *
 */

/*
 *	This sample device driver is an example of the interface between
 *	a SCSI device driver head and a SCSI adapter device driver.  The 
 *	head (which is what this sample represents) is the upper layer in
 *	the AIX V3 SCSI subsystem and it talks to the adapter driver.
 *	It does not talk directly to the SCSI adapter.  The adapter driver 
 *	is the lower layer which talks to the SCSI adapter and other
 *	system hardware.  The two layers communicate via system calls.
 */

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/i_machine.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/errids.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/iostat.h>

#include <sys/scsi.h>
#include <sys/cdrom.h>
#include "cdromdd.h"

/* END OF INCLUDED SYSTEM FILES  */

/**************************************************************************/
/*                                                                        */
/*                             Static Structures                          */
/*                                                                        */
/*   The disk_list variable is an array of linked lists of disk           */
/*   structures.  All of the defined devices that hash to a specific      */
/*   array element are linked together.  In addition, all of the opened   */
/*   devices are connected using seperate links. The disk structure for a */
/*   specific device is accessed by hashing to the correct array element  */
/*   based on the last 4 bits of the devno, and then sequentially         */
/*   searching either the 'defined' or the 'opened list for the correct   */
/*   devno.  The disk_chain_lock is a global lock for this driver that    */
/*   is used to insure that only one process is using the disk_list at    */
/*   a time.  The config_count is used to count the number of devices     */
/*   configured.  This info is needed for the devswadd and devswdel calls.*/
/*                                                                        */
/**************************************************************************/

extern struct cd_hash_table_df   cd_disk_list[CD_HASHSIZE];
extern int  cd_disk_chain_lock; /* lock structure for disk array    */
extern int  cd_config_count;             /* # of devices configured          */


/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_config                                                     */
/*                                                                        */
/*   FUNCTION:  The config entry point.                                   */
/*                                                                        */
/*      This routine begins by searching for the disk structure for the   */
/*      specified device.  If it is found, the device has already been    */
/*      defined.  For a delete request, the disk structure is removed     */
/*      from the list, and freed.  If this is the last disk to be         */
/*      deleted, the devswdel service is called to delete the entry       */
/*      points from the device switch table.  To define a new device, the */
/*      storage for the new disk structure is allocated and the disk      */
/*      structure is added to the list.  If this is the first device to   */
/*      be configured, the devswadd service is called to add the entry    */
/*      points to the device switch table.                                */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*      disk_list               Array of all the disk structures.         */
/*      disk structure          Individual disk information.              */
/*      config_count            Number of devices configured.             */
/*                                                                        */
/*   INPUTS:                                                              */
/*      devno - device major/minor number                                 */
/*      op    - what function to perform (initialize or terminate)        */
/*      uiop  - pointer to structure with device dependent info           */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The errno values listed in the 'error description' will be        */
/*      returned to the caller if there is an error.  Otherwise a         */
/*      value of 0 will be returned to indicate successful completion.    */
/*                                                                        */
/*   ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      ENOMEM  - Kernel service failure (xmalloc).                       */
/*      EINVAL  - Deleting a device that is still open.                   */
/*              - Defining a device that is already defined.              */
/*              - Illegal op paramater.                                   */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  xmalloc, xmfree,                        */
/*                                lockl, unlockl,                         */
/*                                devswadd, devswdel,                     */
/*                                uiomove                                 */
/*                                                                        */
/**************************************************************************/

cd_config( dev_t devno, int op, struct uio *uiop )
/* devno - major and minor device numbers             		*/
/* op - CFG_INIT = new device, CFG_TERM = delete device         */
/* *uiop - pointer to device dependent information    		*/

{
	int                     return_code, return_value;
	struct cd_disk_df       *current_ptr, *back_ptr, *disk_ptr;
	char                    found;
	char                    disk_offset;
	struct devsw            dsw_struct;
	extern int		nodev();

	/*
	 * Check the list of defined disk structures to see if this device
	 * has already been defined.
	 */
	disk_offset = minor(devno) & 0x0f;
	(void) lockl(&cd_disk_chain_lock, LOCK_SHORT);
	found = FALSE;
	current_ptr = cd_disk_list[disk_offset].defined_head;
	back_ptr = current_ptr;
	return_value = 0;
	while ( !found && (current_ptr != NULL) )
	{
		if (current_ptr->devno == devno)
		{
			found = TRUE;
		}
		else
		{
			back_ptr = current_ptr;
			current_ptr = current_ptr->next_defined;
		}
	}
	if (op == CFG_TERM)
	{
	/*
	 * This is a request to delete this device.  First check if
	 * it has been defined.
	 */
		if (found)
		{
       		/*
	 	 * If it has been defined and is not opened, then
		 * delete the device by freeing the disk structure, 
		 * deleting the watchdog timer, and (if this is the
		 * last cdrom) deleting the switch table entries.
		 */
			if (!current_ptr->opened)
			{
				if (--cd_config_count == 0)
				{
					return_value = devswdel(devno);
				}
				if (!return_value)
				{
					w_clear(&current_ptr->watchdog_timer);
					if (back_ptr == current_ptr)
					{
						cd_disk_list[disk_offset].
							defined_head = NULL;
					}
					else
					{
						back_ptr->next_defined =
     							current_ptr->
							next_defined;
					}
					/* de-register cd_dkstat structure */
					iostdel( &(current_ptr->cd_dkstat) );
					return_code = (int)xmfree(
							(caddr_t)current_ptr,
							(caddr_t)pinned_heap );
				}
			}
			else
			{
			/*
			 * If the device is still open, return an error.
			 */
			return_value = EINVAL;
			}
		}
	}
	else if (op == CFG_INIT)
	{
	/*
	 * This is a request to add this device.  First make sure the device has
	 * not already been defined.
	 */
		if (!found)
		{
       		/*
		 * To define this device, allocate storage for the
		 * disk structure (already pinned), add this structure
		 * to the linked list of defined devices, define the
	 	 * watchdog timer, fill in the disk structure, and
		 * (if this is the first cdrom to be defined) add
		 * the device switch table entries for this device.
		 */
			disk_ptr = (struct cd_disk_df *)xmalloc(
					(int)sizeof(struct cd_disk_df),(int)2, 
					(caddr_t)pinned_heap );
			if (disk_ptr != NULL)
			{
			/*
			 * The xmalloc was successful, now continue.
			 * uiomove is called to move the device
			 * dependent structure into the disk structure.
			 */
				uiomove( (caddr_t)&(disk_ptr->disk_ddi),
					 (int)sizeof(struct cdrom_dds_df),
					 UIO_WRITE,
					 uiop);
				if (++cd_config_count == 1)
				{
				/*
				 * This is the first cdrom defined. Add the
				 * entry points to the device switch table by
				 * calling devswadd.
				 */
					dsw_struct.d_open     = cd_open;
					dsw_struct.d_close    = cd_close;
					dsw_struct.d_read     = cd_read;
					dsw_struct.d_write    = nodev;
					dsw_struct.d_ioctl    = cd_ioctl;
					dsw_struct.d_strategy = cd_strategy;
					dsw_struct.d_ttys     = NULL;
					dsw_struct.d_select   = nodev;
					dsw_struct.d_config   = cd_config;
					dsw_struct.d_print    = nodev;
					dsw_struct.d_dump     = nodev;
					dsw_struct.d_mpx      = nodev;
					dsw_struct.d_revoke   = nodev;
					dsw_struct.d_dsdptr   = NULL;
					dsw_struct.d_opts     = NULL;
					return_value = devswadd(devno,
							&dsw_struct);
				}
				if (!return_value)
				{
					if (back_ptr == NULL)
					{
						cd_disk_list[disk_offset].
							defined_head = disk_ptr;
					}
					else
					{
						back_ptr->next_defined = 
							disk_ptr;
					}
/* Code would go here to initialize the various CD_ROM structures. */
				}
				else
				{
				/*
				 * The devswadd failed, so free the allocated
				 * space (return value is already set).
				 */
					return_code = (int)xmfree(
							(caddr_t)disk_ptr,
							pinned_heap );
				}
			}
			else
			{
			/*
			 * The xmalloc failed, so return an error.
			 */
				return_value = ENOMEM;
			}
		}
		else
		{
		/*
		 * This device has already been defined, so return an error.
		 * The device can only be re-defined by deleting it first.
		 */
			return_value = EINVAL;
		}
	}
	else
	{
	/*
	 * An illegal operation was requested so return an error.
	 */
		return_value = EINVAL;
	}
	unlockl(&cd_disk_chain_lock);
	return(return_value);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_open                                                       */
/*                                                                        */
/*   FUNCTION:  The open entry point.                                     */
/*                                                                        */
/*      This routine begins by searching for the disk structure for the   */
/*      specified device.  If the disk structure is found, as many        */
/*      values are initialized as possible.  If this is the first open    */
/*      for a CD-ROM, the bottom half routines are pinned along with      */
/*      the global data structures (the hash table array and the global   */
/*      lock).  Next the adapter device is opened and a START ioctl is    */
/*      issued to the adapter device driver.  Finally, the device is      */
/*      reset via the test unit ready, reserve, mode sense, mode select   */
/*      read capacity, and start unit commands.                           */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*      disk_list               Array of all the disk structures.         */
/*      disk structure          Individual disk information.              */
/*                                                                        */
/*   INPUTS:                                                              */
/*      devno   - device major/minor number                               */
/*      devflag - FREAD for read only                                     */
/*		- FKERNEL opened by another kernel service		  */
/*      chan    - not used                                                */
/*      ext     - extended system call paramater;  this will be set to    */
/*                SC_DIAGNOSTIC when opening in diagnostic mode.          */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The errno values listed in the 'error description' will be        */
/*      returned to the caller if there is an error.  Otherwise a         */
/*      value of 0 will be returned to indicate successful completion.    */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned:       */
/*      EINVAL  - Opening the device in some mode other than read only.   */
/*      ENXIO   - The device has not been defined.                        */
/*      EPERM   - Use of ext paramater without CTL_DEV authority.         */
/*      EACCES  - The device has already been opened in diagnostic mode.  */
/*              - Opening in diagnostic mode when already opened.         */
/*      *       - Kernel service failure (pincode).                       */
/*      *       - Failure of fp_ioctl					  */
/*      Plus others set in the cd_iodone routine when the device is reset.*/
/*      * = the errno returned from these services is returned.           */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  lockl, unlockl,                         */
/*                                pincode, unpincode,                     */
/*                                pinu,unpinu,                            */
/*                                xmalloc, xmfree,                        */
/*                                fp_opendev, fp_ioctl, fp_close          */
/*                                                                        */
/**************************************************************************/

int cd_open( dev_t devno, int devflag, int chan, int ext )
/* devno - i	major/minor number                              */
/* devflag - 	FREAD for read                                  */
/*	   - 	FKERNEL for use by other kernel services	*/
/* chan    -	not used                                        */
/* ext     - 	SC_DIAGNOSTIC indicates open in diagnostic mode */
/* 		SC_FORCED_OPEN indicates BDR on open            */
/* 		SC_RETAIN_RESERVATION - no release on close     */
{
struct cd_disk_df       *disk_ptr;
struct devinfo          unique_devinfo;
int                     return_code, return_value;
char                    disk_offset, flag;


	return_value = 0x00;
	if ( (devflag != FREAD) 		&&
	     (devflag != FKERNEL)		&&
	     (devflag != (FREAD | FKERNEL)) )
	{
	/*
	 * This device can only be opened in read-only mode or by another
	 * kernel service, so return an error if any other mode is attempted.
	 */
		return_value = EINVAL;
		flag = 0;
	}
	else
	{
	/*
	 * Find the disk structure for this device in the linked list of defined
	 * devices.
	 */
		(void) lockl(&cd_disk_chain_lock, LOCK_SHORT);
		disk_offset = minor(devno) & 0x0F;
		disk_ptr = cd_disk_list[disk_offset].defined_head;
		while (disk_ptr != NULL)
		{
			if (disk_ptr->devno == devno)
				break;
			else
				disk_ptr = disk_ptr->next_defined;
		}
		if (disk_ptr == NULL)
		{
		/*
		 * The device has not been defined, so return an error.
		 */
			flag = 1;
			return_value = ENODEV;
		}
		else
		{
		/*
		 * The device is defined, now check if it is already open.
	 	 */
			if (!disk_ptr->opened)
			{
			/*
			 * The device is not open yet. Next check that the user
			 * has CTL_DEV authority if ext paramaters are used.
			 */
				if (ext && privcheck(RAS_CONFIG))
				{
					flag = 1;
					return_value = EPERM;
				}
				else
					flag=cd_open_support(disk_ptr,ext,
					    disk_offset);
			}
			else
			{  /* already opened */
			/*
			 * The device is already opened.  Check to make sure it
			 * was not already in diag mode, or if it is being
			 * opened in diag mode (both conditions are illegal).
			 * Otherwise, nothing needs to be done for multiple
			 * opens.
			 */
/* Code was removed dealing with diagnostics mode. */

				if (ext & SC_RETAIN_RESERVATION)
				{
					disk_ptr->retain_reservation = TRUE;
				}
			}
		}
	}
	switch (flag) {
	/*
	 * If an error occured somewhere in the open procedure the flag is used
	 * to determine how much processing needs to be done to 'back out' to
	 * get back to the same state as before the open was called.
	 */
	case 4:
	/*
	 * The reset failed at some point so a stop-unit command
	 * needs to be issued, along with a release.  Also the
	 * bottom half routines need to be unpinned.  Next this
	 * case falls thru to the next case (pincode failure).
	 */
		disk_ptr->norm_buf.retry_count = 0;
		cd_start_stop(disk_ptr, &disk_ptr->norm_buf.
				scsi_buf,(uchar)CD_STOP_UNIT);
		iowait(&disk_ptr->norm_buf.scsi_buf.
				bufstruct);
		return_code = unpincode( (int(*)())cd_iodone );
	case 3:
	/*
	 * The call to pincode failed.  The STOP ioctl must be
	 * issued to the adapter driver.
	 */
		(void) fp_ioctl(disk_ptr->fp, SCIOSTOP, IDLUN(
					disk_ptr->
					disk_ddi.scsi_id,
					disk_ptr->
					disk_ddi.lun_id ),NULL);
	case 2:
	/*
	 * The START ioctl to the adapter driver failed.
	 * The adapter driver must now be closed.
	 */
		(void) fp_close(disk_ptr->fp);
	case 1:
	/*
	 * The device was not defined.The lock must be released.
	 */
	case 0:
	/*
	 * The open was attempted in some mode other than
	 * read-only. Or there is no error. No 'backing-out' is necessary.
	 */
		unlockl(&cd_disk_chain_lock);
		break;
	default:
		unlockl(&cd_disk_chain_lock);
		break;
	}
	return(return_value);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_open_support                                               */
/*                                                                        */
/*   FUNCTION: Support for the open entry point.			  */ 
/*                                                                        */
/*	This routine is used to perform several of the open entry points  */
/*	functions.							  */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*	disk_ptr - Cd ROM information					  */
/*                                                                        */
/*   INPUTS:                                                              */
/*	disk_ptr - Cd ROM information					  */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*	A value of 0 to 4 will be returned depending on success or amount */
/*	of backing out that will need to be done to recover from an error */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned:       */
/*	0 - No error							  */
/*	1 - Level one backout						  */
/*	2 - Level two backout						  */
/*	3 - Level three backout						  */
/*	4 - Level four backout						  */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED: 					  */ 
/*	fp_opendev, pincode						  */
/*	fp_ioctl							  */
/*                                                                        */
/**************************************************************************/
char
cd_open_support(struct cd_disk_df *disk_ptr, int ext, char disk_offset) {
int			return_value;	/* Return code from function calls */
struct devinfo          unique_devinfo;

/*
 * The adapter driver must be opened via the 'fp_opendev' call.  This 
 * will have no effect if the driver is already open.
 */
	return_value = fp_opendev(disk_ptr-> disk_ddi.adapter_devno, FREAD,
	    NULL, NULL, &disk_ptr->fp);
	if (return_value) {
		/*
 		 * The open failed, so set return value for error handling.
 		 */
		return(1);
	}
	/*
	 * The fp_opendev call succeded, now issue an IOCINFO ioctl 
	 * to the adapter to inquire as to the maximum data transfer 
	 * size allowed by the adapter.
	 */
	return_value = fp_ioctl( disk_ptr->fp, IOCINFO, &unique_devinfo, NULL);
	if (return_value) {
		/*
 		 * The IOCINFO ioctl failed. Set the transfer rate to 
		 * the default value of SC_MAXREQUEST. The adapter card 
		 * SCSI ID (unique_devinfo.un.scsi.card_scsi_id) that 
		 * the device is attached to is left blank because of 
		 * the fp_ioctl failure.
 		 */
		disk_ptr->max_transfer = (int)SC_MAXREQUEST;
	}
	else {
		/*
 		 * The IOCINFO ioctl succeded.  Transfer the device 
		 * specific data transfer size and the SCSI ID of 
		 * the adapter controlling this device to the device 
		 * structure.
 		 */
		disk_ptr->max_transfer = unique_devinfo.un.scsi.max_transfer;
		disk_ptr->card_scsi_id = unique_devinfo.un.scsi.card_scsi_id;
	}

	/*
	 * The fp_opendev call succeded, now issue a START ioctl to the adapter 
	 * driver.
	 */
	return_value = fp_ioctl( disk_ptr->fp, SCIOSTART, 
	    IDLUN(disk_ptr->disk_ddi.scsi_id,disk_ptr->disk_ddi.lun_id), NULL);
	if (return_value) {
		/*
		 * The START ioctl failed.  Set the flag for error handling.
		 */
		return(2);
	}
	/*
	 * The START ioctl succeded
	 */
	if (ext & SC_FORCED_OPEN) {
		/*
		 * Issue a bus device reset for the forced
		 * open.
		 */
		return_value = fp_ioctl( disk_ptr->fp, SCIORESET, 
		    IDLUN(disk_ptr->disk_ddi.scsi_id,
		    disk_ptr->disk_ddi.lun_id),NULL);
		if(return_value) {
			/* 
 			 * The BDR failed.
			 */
		       return(3);
		}
	}
	/*
 	 * Call pincode to pin the lower half of the driver.
	 */
	return_value = pincode((int(*)())cd_iodone);
	if(return_value) {
		/*
		 * Pincode failed.
		 */
	       return(3);
	}
/* Code was removed dealing with diagnostics mode. */

	disk_ptr->reset_failed = FALSE;
	disk_ptr->reset_count = 0;
	disk_ptr->diag_mode = FALSE;
	disk_ptr->open_pending= TRUE;
	cd_device_reset(disk_ptr);
	iowait(&disk_ptr->reset_buf.scsi_buf.bufstruct);
	disk_ptr->open_pending = FALSE;
	return_value = disk_ptr->reset_buf.scsi_buf.bufstruct.b_error;

	if(return_value) {
		/*
 		 * The reset failed.
 		 */
		return(4);
	}
/* The open was successful. Set the flag to retain the reservation, if
 * requested. Add this device to the linked list of opened devices.
 */
	disk_ptr-> opened =TRUE;
	if (ext & SC_RETAIN_RESERVATION) {
		disk_ptr->retain_reservation = TRUE;
	}
	else {
		disk_ptr-> retain_reservation=FALSE;
	}
	disk_ptr-> next_opened = cd_disk_list[disk_offset].opened_head;
	cd_disk_list[disk_offset].opened_head = disk_ptr;
	return(0);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_close                                                      */
/*                                                                        */
/*   FUNCTION:  The close entry point.                                    */
/*                                                                        */
/*      This routine begins by searching for the disk structure for the   */
/*      specified device.  If the structure is found, the routine waits   */
/*      for the queue to become empty and then issues a stop unit and a   */
/*      release command to the device.  Next, a STOP ioctl is issued to   */
/*      the adapter driver and the adapter device is closed. Finally, if  */
/*      this is the last CD-ROM to be closed, the bottom half routines    */
/*      and global data structures are unpinned.                          */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*      disk_list               Array of all the disk structures.         */
/*      disk structure          Individual disk information.              */
/*                                                                        */
/*   INPUTS:                                                              */
/*      devno   - device major/minor number                               */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The errno values listed in the 'error description' will be        */
/*      returned to the caller if there is an error.  Otherwise a         */
/*      value of 0 will be returned to indicate successful completion.    */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned:       */
/*      ENXIO   - The device is not defined or not opened.                */
/*      *       - A fp_ioctl call failed.                                 */
/*      Plus others set in cd_iodone for the stop unit and release.       */
/*      * = the errno returned from these services is returned.           */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  lockl, unlockl,                         */
/*                                unpincode,                              */
/*                                delay,                                  */
/*                                fp_close, fp_ioctl                      */
/*                                                                        */
/**************************************************************************/
int cd_close( dev_t devno, int chan, int ext )
/* devno - major/minor number                             */
{
struct cd_disk_df       *disk_ptr, *back_ptr;
int                     return_code, return_value;
char                    disk_offset;

	/*
	 * Find the disk structure for this device in the 'opened device' list.
	 */
	disk_offset = minor(devno) & 0x0f;
	(void) lockl(&cd_disk_chain_lock, LOCK_SHORT);
	disk_ptr = cd_disk_list[disk_offset].opened_head;
	back_ptr = disk_ptr;
	return_value = 0x00;
	while (disk_ptr != NULL)
	{
		if (disk_ptr->devno == devno)
			break;
		else
		{
			back_ptr = disk_ptr;
			disk_ptr = disk_ptr->next_opened;
		}
	}
	if (disk_ptr == NULL)
	{
	/*
	 * If the disk structure was not found, either the device has
	 * not been defined, or it is not open, so return an error.
	 */
		return_value = ENXIO;
	}
	else
	{
	/*
	 * The disk structure was found.  Now delete it from the
	 * 'opened device' list.  The structure is not lost because
	 *  it is still in the 'defined device' list.
	 */
		if (back_ptr == disk_ptr)
		{
			cd_disk_list[disk_offset].opened_head = NULL;
		}
		else
		{
			back_ptr->next_opened = disk_ptr->next_opened;
		}
		(void) lockl(&disk_ptr->lock_word, LOCK_SHORT);
		while (disk_ptr->busy || disk_ptr->cmd_pending)
		{
			delay(60);
		}

/* Code was removed dealing with diagnostics mode. */
       	/*
	 * The device is open in normal mode so a stop_unit and
	 * a release must be issued to the device.  If the
	 * device was opened in 'retain reservation' mode,
	 * the iodone routine will prevent the issuing of the release
	 * command.
	 */
		disk_ptr->norm_buf.retry_count = 0;
		disk_ptr->norm_buf.scsi_buf.bufstruct.b_error = 0x00;
		cd_start_stop(disk_ptr, &disk_ptr->norm_buf.scsi_buf,
		    (uchar)CD_STOP_UNIT);
		iowait(&disk_ptr->norm_buf.scsi_buf.bufstruct);
		if (disk_ptr->norm_buf.scsi_buf.bufstruct.b_flags & B_ERROR)
			return_value = disk_ptr->norm_buf.scsi_buf.
			    bufstruct.b_error;
	}
	/*
	 * The STOP ioctl must be issued to the adapter driver,
	 * the driver must be closed via the fp_close call,
	 * the opened flag is set to false, and the code is unpinned.
	 * The unpincode routine keeps a counts so that the code will
	 * actually be unpinned when the final cdrom is closed.
	 */
	return_code = fp_ioctl(disk_ptr->fp, SCIOSTOP, IDLUN(
	    disk_ptr->disk_ddi.scsi_id, disk_ptr->disk_ddi.lun_id ),NULL);
	if (( return_code != 0) && (return_value == 0))
		return_value = return_code;
	return_code = fp_close(disk_ptr->fp);
	if ((return_code != 0) && (return_value == 0))
		return_value = return_code;
	/*
	 * Set the flag to indicate that this device is closed.
	 */
	disk_ptr->opened = FALSE;
	return_code = unpincode( (int(*)())cd_iodone );

	unlockl(&cd_disk_chain_lock);
	return(return_value);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_read                                                       */
/*                                                                        */
/*   FUNCTION:  Raw read entry point.                                     */
/*                                                                        */
/*      This routine simply calls the uphysio kernel service to handle    */
/*      the raw read.  Any large commands that must be broken up before   */
/*      they are issued, are handled in the cd_start and cd_iodone        */
/*      routines.                                                         */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      devno   - device major/minor number                               */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      This routine will return the errno value returned from uphysio.   */
/*      The possible values include those listed in the iodone routine.   */
/*                                                                        */
/*   ERROR DESCRIPTION:  See above.                                       */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  uphysio                                 */
/*                                                                        */
/**************************************************************************/

int cd_read( dev_t devno, struct uio *uiop, int chan, int ext )
/* devno - major/minor number                                           */
/* chan - not used in this routine, but required for parameter matching */
/* ext - not used in this routine, but required for parameter matching  */
{
int     return_code;
	/*
	 * The paramaters to the uphysio routine are:
	 * uio      -  pointer to the uio structure.
	 * rw       -  access mode (read or write).
	 * buf_cnt  -  degree of concurrency (number of buf structs to use).
	 * dev      -  device major/minor number.
	 * strat    -  pointer to strategy routine to be called.
	 * mincnt   -  pointer to request check routine.
	 * minparms -  paramaters for mincnt routine.
	 * This driver does not need the funcionality available through
	 * the mincnt routine, so it simply returns.
	 */
	return_code = uphysio(uiop, B_READ, 1,devno,cd_strategy,cd_mincnt,NULL);
	return(return_code);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_mincnt                                                     */
/*                                                                        */
/*   FUNCTION:  Mincnt routine required by uphysio kernel service.        */
/*                                                                        */
/*      This routine is called only by uphysio.  This driver has no       */
/*      need for this routine, so it simply returns a good return code.   */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      bp       - buf struct allocated in uphysio                        */
/*      minparms - extended paramaters                                    */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                        */
/**************************************************************************/

int cd_mincnt( struct buf bp, void *minparms )
/* bp - not used in this routine, but required for parameter matching   */
/* miniparms - not used in this routine, but required for parameter matching */
{
return(0);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_ioctl                                                      */
/*                                                                        */
/*   FUNCTION:  The ioctl entry point.                                    */
/*                                                                        */
/*      This routine handles the following ioctl commands:                */
/*         IOCINFO    - returns info about the device such as blocksize   */
/*                      and number of blocks                              */
/*         CDIORDSE   - issues a read command and returns sense data if   */
/*                      an error occurs                                   */
/*         CDIOCMD    - issues any SCSI command to the device (pass-thru) */
/*                                                                        */
/*      The IOCINFO command simply fill in the structure provided         */
/*      by the user.  The CDIORDSE issues the read command by filling     */
/*      in a buf struct, calling xmattach and pinu for the user buffer,   */
/*      and calling the cd_start routine.  The request sense is issued in */
/*      the normal error handling of cd_iodone, and the sense data is	  */
/*      copied to the user space if it is valid. The CDIOCMD (pass-thru)  */
/*      command requires that the device is open in diagnostic mode.	  */
/*	If there is a user buffer for the operation, xmattach and pinu are*/
/*	called.  Then the command is issued by calling cd_start.  No	  */
/*	error handling is done in cd_iodone for this command.		  */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*      disk_list               Array of all the disk structures.         */
/*      disk structure          Individual disk information.              */
/*                                                                        */
/*   INPUTS:                                                              */
/*      devno - the device major/minor number                             */
/*      op    - the operation to be performed (see list above)            */
/*      arg   - the address of the structure provided by the user         */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The errno values listed in the 'error description' will be        */
/*      returned to the caller if there is an error.  Otherwise a         */
/*      value of 0 will be returned to indicate successful completion.    */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned:       */
/*      ENXIO   - The device is not opened or not defined.                */
/*      EINVAL  - Attempting a diagnostic command while not in diag mode. */
/*              - Illegal ioctl command.                                  */
/*              - Read request is not a multiple of block size.           */
/*      EFAULT  - Failure of kernel service (copyout, copyin, xmattach)   */
/*                (illegal user buffer address)                           */
/*      *       - Pin kernel service failed.                              */
/*      Plus others that may be returned when the command is executed,    */
/*      or if a reset has failed, the errno that originally caused it.    */
/*      * = the errno returned from this kernel service will be returned. */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  lockl, unlockl,                         */
/*                                copyin, copyout,                        */
/*                                xmattach, xmdetach                      */
/*                                pinu, unpinu                            */
/*                                                                        */
/**************************************************************************/

int cd_ioctl( dev_t devno, int op, int arg, ulong dev_flag )
{
struct cd_disk_df       *disk_ptr;
struct sc_rdwrt         *sc_rdwrt_ptr;
struct sc_iocmd         *sc_iocmd_ptr;
struct devinfo          *devinfo_ptr;
struct xmem             xmem_desc;
int                     return_code, return_value;
uint                    transfer, usr_struct[10];  /* 40 byte storage space */

	/*
	 * Find the disk structure for this device in the linked list of opened
	 * devices.
	 */
	return_value = 0;
	(void) lockl(&cd_disk_chain_lock, LOCK_SHORT);
	disk_ptr = cd_disk_list[minor(devno) & 0x0f].opened_head;
	while (disk_ptr != NULL)
	{
		if (disk_ptr->devno == devno)
			break;
		else
			disk_ptr = disk_ptr->next_opened;
	}
	if (disk_ptr == NULL)
	{
	/*
	 * The device is either not open or not even defined, return an error.
	 */
		unlockl(&cd_disk_chain_lock);
		return(ENXIO);
	}
	(void) lockl(&disk_ptr->lock_word, LOCK_SHORT);
	unlockl(&cd_disk_chain_lock);
	if (op == IOCINFO)
	{
	/*
	 * Fill user structure with the values defined for cdrom in devinfo.h
	 */
		devinfo_ptr = (struct devinfo *) usr_struct;
		devinfo_ptr->devtype = DD_SCCD;
		devinfo_ptr->flags = DF_RAND;
		devinfo_ptr->devsubtype = 0x00;
		devinfo_ptr->un.sccd.blksize = disk_ptr->disk_nblks.len;
		devinfo_ptr->un.sccd.numblks = disk_ptr->disk_nblks.lba + 1;
		if ( dev_flag & FKERNEL )
		{ /* kernel process */
			bcopy((caddr_t)devinfo_ptr, (caddr_t) arg,
				sizeof(struct devinfo));
		}
		else
		{ /* user process */
			return_code = copyout((caddr_t)devinfo_ptr,
					(caddr_t) arg, sizeof(struct devinfo));
		/*
 		 * If the copyout service failed (most likely due to an
 		 * illegal user address) return an error.
		 */
			if (return_code == 0)
				return_value = 0x00;
			else
				return_value = EFAULT;
			unlockl(&disk_ptr->lock_word);
			return(return_value);
		}
	}
	if (disk_ptr->reset_failed)
	{
	/*
	 * The device is flagged as 'dead' and the user wants to issue
	 * some command to the device.  Return the errno that was
	 * returned on the last reset attempt.
	 */
	       unlockl(&disk_ptr->lock_word);
	       return(disk_ptr->failed_errno);
	}
	if (op == CDIORDSE)
	{
	/*
	 * A read with sense has been requested.  First copy the user
	 * structure into a local (temporary) buffer.
	 */
		sc_rdwrt_ptr = (struct sc_rdwrt *) usr_struct;
		if ( dev_flag & FKERNEL )
		{ /* kernel process */
			bcopy((caddr_t) arg, (caddr_t) sc_rdwrt_ptr,
				sizeof(struct sc_rdwrt));
		}
		else
		{ /* user process */
			return_code = copyin((caddr_t)arg,(caddr_t)sc_rdwrt_ptr,
						sizeof(struct sc_rdwrt));
			if (return_code != 0)
			{
			/*
 			 * The copyin failed (probably due to a bad
 			 * user address).  Return an error.
 			 */
			       unlockl(&disk_ptr->lock_word);
			       return(EFAULT);
			}
		}
		transfer = *(int *)(disk_ptr->disk_ddi.mode_select_data + 8);
		if (sc_rdwrt_ptr->data_length % transfer)
		{
 		/*
  		 * The user buffer is not on a multiple of the
  		 * current blocksize.
  		 */
			unlockl(&disk_ptr->lock_word);
			return(EINVAL);
		}
		/*
		 *  Fill in the buf struct to be passed to the start routine.
		 */
		disk_ptr->rdse_buf_struct.b_blkno = sc_rdwrt_ptr->
							logical_blk_addr;
		disk_ptr->rdse_buf_struct.b_un.b_addr = (caddr_t)sc_rdwrt_ptr->
								buffer;
		disk_ptr->rdse_buf_struct.b_bcount = sc_rdwrt_ptr->data_length;
		disk_ptr->rdse_buf_struct.b_error = 0x00;
		disk_ptr->rdse_buf_struct.b_resid = 0x00;
		disk_ptr->ioctl_buf.scsi_buf.scsi_command.scsi_cmd.
			scsi_op_code = SCSI_READ_EXTENDED;
		if (sc_rdwrt_ptr->data_length > 0)
		{
		/*
		 * xmattach must be called with the user buffer to allow
		 * the bottom half routines to transfer data to the
		 * user space.  The user buffer must also be pinned.
		 */
			disk_ptr->rdse_buf_struct.b_xmemd.aspace_id =XMEM_INVAL;
			if ( dev_flag & FKERNEL ) /* attach kernel memory */
				return_code = xmattach(sc_rdwrt_ptr->buffer,
							sc_rdwrt_ptr->
								data_length,
							&disk_ptr->
								rdse_buf_struct.
								b_xmemd,
							SYS_ADSPACE);
			else /* attach user memory */
				return_code = xmattach(sc_rdwrt_ptr->buffer,
							sc_rdwrt_ptr->
								data_length,
							&disk_ptr->
								rdse_buf_struct.
								b_xmemd,
							USER_ADSPACE);
			if (return_code != XMEM_SUCC)
			{
			       unlockl(&disk_ptr->lock_word);
			       return(EFAULT);
			}
			if ( dev_flag & FKERNEL ) /* pinning kernel memory */
				return_code = pinu((caddr_t) sc_rdwrt_ptr->
								buffer,
							sc_rdwrt_ptr->
								data_length,
							UIO_SYSSPACE);
			else /* pinning user memory */
				return_code = pinu((caddr_t) sc_rdwrt_ptr->
								buffer,
							sc_rdwrt_ptr->
								data_length,
							UIO_USERSPACE);
			if (return_code != 0)
			{
				xmdetach(&(disk_ptr->rdse_buf_struct.b_xmemd));
				unlockl(&disk_ptr->lock_word);
				return(return_code);
			}
		}
		if (sc_rdwrt_ptr->req_sense_length > 0)
		{
			disk_ptr->ioctl_req_sense.xmemd.aspace_id = XMEM_INVAL;
			if ( dev_flag & FKERNEL ) /* attach kernel memory */
				return_code = xmattach(sc_rdwrt_ptr->
							  request_sense_ptr,
							sc_rdwrt_ptr->
							  req_sense_length,
							&disk_ptr->
							  ioctl_req_sense.xmemd,
							SYS_ADSPACE);
			else /* attach user memory */
				return_code = xmattach(sc_rdwrt_ptr->
							  request_sense_ptr,
							sc_rdwrt_ptr->
							  req_sense_length,
							&disk_ptr->
							  ioctl_req_sense.xmemd,
							USER_ADSPACE);
			if (return_code != XMEM_SUCC)
			{
				if (sc_rdwrt_ptr->data_length > 0)
				{
					if ( dev_flag & FKERNEL )
					/* unpin kernel memory */
						(void)unpinu(sc_rdwrt_ptr->
								buffer,
							sc_rdwrt_ptr->
								data_length,
							UIO_SYSSPACE);
					else /* unpin user memory */
						(void)unpinu(sc_rdwrt_ptr->
								buffer,
							sc_rdwrt_ptr->
								data_length,
							UIO_USERSPACE);
					(void)xmdetach(&disk_ptr->
						       rdse_buf_struct.b_xmemd);
				}
				unlockl(&disk_ptr->lock_word);
				return(EFAULT);
			}
			if ( dev_flag & FKERNEL ) /* pin kernel memory */
				return_code = pinu((caddr_t)sc_rdwrt_ptr->
							request_sense_ptr,
						sc_rdwrt_ptr->
							req_sense_length,
						UIO_SYSSPACE);
			else /* pin user memory */
				return_code = pinu((caddr_t) sc_rdwrt_ptr->
							request_sense_ptr,
						sc_rdwrt_ptr->
							req_sense_length,
						UIO_USERSPACE);
			if (return_code != 0)
			{
				if (sc_rdwrt_ptr->data_length > 0)
				{
					if ( dev_flag & FKERNEL )
					/* unpin kernel memory */
						(void)unpinu(sc_rdwrt_ptr->
								buffer,
							sc_rdwrt_ptr->
								data_length,
							UIO_SYSSPACE);
					else /* unpin user memory */
						(void)unpinu(sc_rdwrt_ptr->
								buffer,
							sc_rdwrt_ptr->
								data_length,
							UIO_USERSPACE);
					(void)xmdetach(&disk_ptr->
						       rdse_buf_struct.b_xmemd);
				}
				(void)xmdetach(&disk_ptr->
					       ioctl_req_sense.xmemd);
				unlockl(&disk_ptr->lock_word);
				return(return_code);
			}
		}
		/*
		 * Set up the timeout value and the count and buffer address
		 * for the request sense information (this is needed in iodone
		 * to transfer the data).
		 */
		disk_ptr->ioctl_buf.scsi_buf.timeout_value =sc_rdwrt_ptr->
							timeout_value;
		disk_ptr->ioctl_req_sense.count =sc_rdwrt_ptr->req_sense_length;
		disk_ptr->ioctl_req_sense.buffer =sc_rdwrt_ptr->
							request_sense_ptr;
		/*
		 * If the device is busy (a regular read is in progress),
		 * the cd_busy routine will set a flag to indicate to the
		 * iodone routine that an ioctl is waiting to be started.
		 * If the device is not busy, the command is started by
		 * calling start directly.
		 */
   
		if (!cd_busy(disk_ptr))
		{ /* no command is currently in progress */
			cd_start(disk_ptr, (uchar)CD_ORIGINAL_REQUEST);
		}
		iowait(&disk_ptr->ioctl_buf.scsi_buf.bufstruct);
		/*
		 * The command has completed.  Restore the timeout value to its
		 * original value, unpinu and xmdetach the user buffer.  The
		 * return value will be set to whatever errno the lower half
		 * routines returned (or 0 for good completion).
		 */
		disk_ptr->ioctl_buf.scsi_buf.timeout_value = CD_TIMEOUT;
		if (sc_rdwrt_ptr->data_length > 0)
		{
			if ( dev_flag & FKERNEL ) /* unpin kernel memory */
				return_code = unpinu(sc_rdwrt_ptr->buffer,
						sc_rdwrt_ptr->data_length,
						UIO_SYSSPACE);
			else /* unpin user memory */
				return_code = unpinu(sc_rdwrt_ptr->buffer,
						sc_rdwrt_ptr->data_length,
						UIO_USERSPACE);
			(void) xmdetach(&disk_ptr->rdse_buf_struct.b_xmemd);
		}
		if (sc_rdwrt_ptr->req_sense_length > 0)
		{
			if ( dev_flag & FKERNEL ) /* unpin kernel memory */
				return_code = unpinu(sc_rdwrt_ptr->
							request_sense_ptr,
						sc_rdwrt_ptr->req_sense_length,
						UIO_SYSSPACE);
			else /* unpin user memory */
				return_code = unpinu(sc_rdwrt_ptr->
							request_sense_ptr,
						sc_rdwrt_ptr->req_sense_length,
						UIO_USERSPACE);
			(void) xmdetach(&disk_ptr->ioctl_req_sense.xmemd);
		}
		/*
		 * Fill in the flags in the local structure and copy the whole
		 *  structure back to the user space.  The return value is the
		 *  errno from the sc_buf (or 0 if no error).
		 */
		sc_rdwrt_ptr->status_validity = disk_ptr->ioctl_buf.scsi_buf.
							status_validity;
		sc_rdwrt_ptr->scsi_bus_status = disk_ptr->ioctl_buf.scsi_buf.
							scsi_status;
		sc_rdwrt_ptr->adapter_status = disk_ptr->ioctl_buf.scsi_buf.
							general_card_status;
		if ( dev_flag & FKERNEL )
		{ /* kernel process */
			bcopy((caddr_t)sc_rdwrt_ptr, (caddr_t) arg,
				sizeof(struct sc_rdwrt));
		}
		else
		{ /* user process */
			return_code = copyout((caddr_t)sc_rdwrt_ptr,(caddr_t)
					arg, sizeof(struct sc_rdwrt));
			if (return_code != 0)
			{
				return_value = EFAULT;
			}
		}
		if (disk_ptr->ioctl_buf.scsi_buf.bufstruct.b_flags & B_ERROR)
		{
			return_value = disk_ptr->ioctl_buf.scsi_buf.bufstruct.
						b_error;
		}
		unlockl(&disk_ptr->lock_word);
		return(return_value);
	}
	if (op == CDIOCMD)
	{
	/*
	 * The device must be open in diag mode for the user to issue
	 * pass-thru commands, so check for that first.
	 */
		if (!disk_ptr->diag_mode)
		{
			unlockl(&disk_ptr->lock_word);
			return(EACCES);
		}
		/*
	  	 * Copy the user structure to a local buffer.
	 	 */
		sc_iocmd_ptr = (struct sc_iocmd *) usr_struct;
		if ( dev_flag & FKERNEL )
		{ /* kernel process */
			bcopy((caddr_t) arg, (caddr_t)sc_iocmd_ptr,
					sizeof(struct sc_iocmd));
		}
		else
		{ /* user process */
			return_code = copyin((caddr_t) arg,
					(caddr_t) sc_iocmd_ptr,
					sizeof(struct sc_iocmd));
			if (return_code != 0)
			{
			       unlockl(&disk_ptr->lock_word);
			       return(EFAULT);
			}
		}
		/*
	 	 * If there is a user buffer for this command, it must be
	 	 * xmattach'd and pinned.
		 */
		if (sc_iocmd_ptr->data_length > 0)
		{
			if (sc_iocmd_ptr->data_length > disk_ptr->max_transfer)
			{
			/*
 		 	 * We cannot allow data reads of more than the maximum
 		 	 * transfer rate set by the device.  If that rate is
 		 	 * not provided by the device, the default maximum
 	 	 	 * transfer rate is 32 Kbytes.
 	 	 	 */
			       unlockl(&disk_ptr->lock_word);
			       return(EINVAL);
			}
			xmem_desc.aspace_id = XMEM_INVAL;
			if ( dev_flag & FKERNEL ) /* attach kernel memory */
				return_code = xmattach((caddr_t) sc_iocmd_ptr->
								buffer,
							sc_iocmd_ptr->
								data_length,
							&xmem_desc,SYS_ADSPACE);
			else /* attach user memory */
				return_code = xmattach((caddr_t) sc_iocmd_ptr->
								buffer,
							sc_iocmd_ptr->
								data_length,
							&xmem_desc,
							USER_ADSPACE);
			if (return_code != XMEM_SUCC)
			{
			       unlockl(&disk_ptr->lock_word);
			       return(EFAULT);
			}
			if ( dev_flag & FKERNEL ) /* pinning kernel memory */
				return_code = pinu((caddr_t) sc_iocmd_ptr->
								buffer,
							sc_iocmd_ptr->
								data_length,
							UIO_SYSSPACE);
			else /* pinning user memory */
				return_code = pinu((caddr_t) sc_iocmd_ptr->
								buffer,
							sc_iocmd_ptr->
								data_length,
							UIO_USERSPACE);
			if (return_code != 0)
			{
				(void)xmdetach(&xmem_desc);
				unlockl(&disk_ptr->lock_word);
				return(return_code);
			}
		}
		/*
	 	 * Set the timeout value and command flags (ASYNC and NODISC)
	 	 * to the requested values and call cd_pass_thru to fill in
	 	 * the rest of the sc_buf.
	 	 */
		disk_ptr->ioctl_buf.scsi_buf.timeout_value = sc_iocmd_ptr->
								timeout_value;
		/*
 	 	 * Set the sync/async request to synchronous with the NULL flag
 	 	 */

		disk_ptr->ioctl_buf.scsi_buf.scsi_command.flags =
			(ushort)(sc_iocmd_ptr->flags & (char)(~B_READ) );
		cd_pass_thru(disk_ptr, sc_iocmd_ptr, &xmem_desc);
		/*
	 	 * If the device is busy (a regular read is in progress),
	 	 * the cd_busy routine will set a flag to indicate to the
	 	 * iodone routine that an ioctl is waiting to be started.
	 	 * If the device is not busy, the command is started by
	 	 * calling start directly.
	 	 */

		if (!cd_busy(disk_ptr))
		{
			cd_start( disk_ptr, (uchar)CD_ORIGINAL_REQUEST );
		}
		iowait( &disk_ptr->ioctl_buf.scsi_buf.bufstruct );
		/*
	 	 * The command has completed.  Restore the timeout and command
	 	 * flag to their original values, unpinu and xmdetach the user
	 	 * buffer.
	 	 */
		disk_ptr->ioctl_buf.scsi_buf.timeout_value = CD_TIMEOUT;
		/*
 	 	 * Set the sync/async request to synchronous with the NULL flag
 	 	 */
		disk_ptr->ioctl_buf.scsi_buf.scsi_command.flags = NULL;
		if (sc_iocmd_ptr->data_length > 0)
		{
			if ( dev_flag & FKERNEL ) /* unpin kernel memory */
				return_code = unpinu((caddr_t) sc_iocmd_ptr->
								buffer,
							sc_iocmd_ptr->
								data_length,
							UIO_SYSSPACE);
			else /* unpin user memory */
				return_code = unpinu((caddr_t) sc_iocmd_ptr->
								buffer,
							sc_iocmd_ptr->
								data_length,
							UIO_USERSPACE);
			(void) xmdetach(&xmem_desc);
		}
		/*
	 	 * Fill in the rest of the struct, and copy it back to the
	 	 * user space.  The return value is the errno returned in the
	 	 * sc_buf (or 0 if no error).
	 	 */
		sc_iocmd_ptr->status_validity = disk_ptr->ioctl_buf.scsi_buf.
							status_validity;
		sc_iocmd_ptr->scsi_bus_status = disk_ptr->ioctl_buf.scsi_buf.
							scsi_status;
		sc_iocmd_ptr->adapter_status = disk_ptr->ioctl_buf.scsi_buf.
							general_card_status;
		if ( dev_flag & FKERNEL )
		{ /* kernel process */
			bcopy((caddr_t)sc_iocmd_ptr, (caddr_t) arg,
					sizeof(struct sc_iocmd));
		}
		else
		{ /* user process */
			return_code = copyout((caddr_t)sc_iocmd_ptr,
						(caddr_t)arg,
						sizeof(struct sc_iocmd));
			if (return_code != 0)
			{
				return_value = EFAULT;
			}
		}
		if (disk_ptr->ioctl_buf.scsi_buf.bufstruct.b_flags & B_ERROR)
		{
			return_value = disk_ptr->ioctl_buf.scsi_buf.bufstruct.
						b_error;
		}
		unlockl(&disk_ptr->lock_word);
		return(return_value);
	}
	/* else illegal op */
	unlockl(&disk_ptr->lock_word);
	return(EINVAL);
}
