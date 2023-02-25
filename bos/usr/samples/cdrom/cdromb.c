static char sccsid[] = "@(#)46	1.1  src/bos/usr/samples/cdrom/cdromb.c, cdrmsamp, bos411, 9428A410j 4/27/90 13:07:49";
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
 * COMPONENT_NAME: SCSI CDROM Device Driver Bottom Half Routines
 *
 * FUNCTIONS:
 *		cd_strategy
 *              cd_release
 *		cd_pass_thru
 *              cd_device_reset
 *		cd_mode_select
 *              cd_read_capacity
 * 		cd_reserve
 *              cd_start_stop
 * 		cd_start
 *              cd_busy
 *              cd_watchdog
 *              cd_scsi_error
 * 		cd_adapter_error
 *              cd_iodone
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
#include <sys/types.h>
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
#include <sys/lockl.h>
#include <sys/device.h>
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

struct cd_hash_table_df   cd_disk_list[CD_HASHSIZE] = { NULL };
int  cd_disk_chain_lock = LOCK_AVAIL; /* lock structure for disk array    */
int  cd_config_count = 0;             /* # of devices configured          */

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_strategy                                                   */
/*                                                                        */
/*   FUNCTION:  The strategy entry point.                                 */
/*                                                                        */
/*      This routine accepts linked buf structs and scans them to see     */
/*      if consecutive ones should be consolidated into a single SCSI     */
/*      command for a chain of buf structs.  The chains are added to      */
/*      the queue with the end of each chain marked by a flag in the      */
/*      b_work field of the last buf struct in each chain.  If there's no */
/*      command in progress, cd_start will be called for the first chain  */
/*      on the queue.  If consecutive buf structs on the chain are for    */
/*      different devices, the chain is broken there and the command      */
/*      is either issued or queued for the first device.                  */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*      cd_disk_list            Common device driver information.         */
/*                                                                        */
/*   INPUTS:                                                              */
/*      buf_ptr - the address of the first buf struct in the chain        */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned in the */
/*      buf struct b_error field:                                         */
/*      ENXIO   - The device is not opened or not defined.                */
/*              - A bad logical block address has been specified.         */
/*      EINVAL  - The device has been opened in diag mode.                */
/*              - The request is not a multiple of the blocksize.         */
/*      EACCES  - An illegal access is attempted while the device is      */
/*                opened in the wrong mode.                               */
/*      Also if a reset has failed, the same errno will be returned as    */
/*      on the command that the reset failed on.                          */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  i_disable, i_enable,                    */
/*                                iodone                                  */
/*                                                                        */
/**************************************************************************/

int cd_strategy( struct buf *buf_ptr )
{
	struct buf              *cur_bp, *next_ptr;
	struct cd_disk_df       *disk_ptr;
	int                     old_ilevel, blocksize, last_lba;

	cur_bp = buf_ptr;
	/*
	 * Loop through the entire chain of buf structs passed to this routine.
	 */
	do
	{
	/*
	 * Find the disk structure for this device in the 'opened' list.
	 */
		old_ilevel = i_disable(INTIODONE);
		disk_ptr =cd_disk_list[minor(cur_bp->b_dev) & 0x0F].opened_head;
		while (disk_ptr != NULL)
		{
			if (disk_ptr->devno == cur_bp->b_dev)
				break;
			else
				disk_ptr = disk_ptr->next_opened;
		}
		if (disk_ptr == NULL)
		{
		/*
		 * The device is not opened or not even defined so
		 * return an error.
		 */
			cur_bp->b_flags |= B_ERROR;
			cur_bp->b_error = ENXIO;
			i_enable(old_ilevel);
		}
		else
		{
			disk_ptr->cmd_pending = TRUE;
			i_enable(old_ilevel);
			cur_bp->b_resid = 0;
			blocksize = disk_ptr->disk_nblks.len;
			last_lba = disk_ptr->disk_nblks.lba;
/* Code was removed dealing with the diagnostic mode. */
			if (disk_ptr->reset_failed)
			{
			/*
			 * The device is flagged as 'dead' so return
			 * the last errno returned (that caused the
			 * failure in the first place).
			 */
				cur_bp->b_error = disk_ptr->failed_errno;
				cur_bp->b_flags |= B_ERROR;
			}
			else if (cur_bp->b_bcount % blocksize)
			{
			/*
			 * The request is not a multiple of the
			 * block size so return an error.
			 */
				cur_bp->b_error = EINVAL;
				cur_bp->b_flags |= B_ERROR;
			}
		}
		next_ptr = cur_bp->av_forw;
		cur_bp->av_forw = NULL;
		if ((cur_bp->b_flags & B_ERROR) ||
		   (cur_bp->b_blkno > last_lba) ||
		   (cur_bp->b_bcount == 0))
		{
		/*
		 * Either an error was flagged above, or the user
		 * is attempting to read the 1st block beyond the end
		 * of the disc.  In the case of an error, the errno
		 * is already set from above.  In the second case,
		 * no errno should be set but the resid value should
		 * be set to indicate EOF to the user.  Call iodone
		 * on the bufstruct.
		 */
			cur_bp->b_resid = cur_bp->b_bcount;
			/*
 			 * If b_blkno starts right at the end of media
 			 * (last_lba +1), then let it return with length 0
 			 * and no error
 			 */
			if ( cur_bp->b_blkno > (last_lba + 1) )
			{
				cur_bp->b_error = ENXIO;
				cur_bp->b_flags |= B_ERROR;
			}
			iodone(cur_bp);
			if (disk_ptr != NULL)
			{
				disk_ptr->cmd_pending = FALSE;
			}
		}
		else
		{ /* no error */
			old_ilevel = i_disable(INTIODONE);
			if (disk_ptr->head == NULL)
			{
				disk_ptr->head = cur_bp;
			}
			else
			{
				disk_ptr->tail->av_forw = cur_bp;
			}
			disk_ptr->tail = cur_bp;
			if (!disk_ptr->busy)
			{
			/*
			 * The device is not busy, so the command can be issued
			 * by calling cd_start.
			 */
				disk_ptr->busy = TRUE;
				cd_start(disk_ptr, (uchar)CD_ORIGINAL_REQUEST);
			}
			disk_ptr->cmd_pending = FALSE;
			i_enable(old_ilevel);
		}
		cur_bp = next_ptr;
	} while (cur_bp != NULL);
	return(0);
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_release                                                    */
/*                                                                        */
/*   FUNCTION:  Builds and sends a RELEASE command to the CD ROM device.  */
/*                                                                        */
/*      This routine fills out a sc_buf struct complete with the          */
/*      SCSI command block for a RELEASE and sends it to the SCSI         */
/*      adapter driver, which then passes the command on to the device.   */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr   - a pointer to the disk structure for this device      */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_release( struct cd_disk_df *disk_ptr )
{
}


/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_read_capacity                                              */
/*                                                                        */
/*   FUNCTION:  Builds a READ CAPACITY command.                           */
/*                                                                        */
/*      This routine fills out a sc_buf struct complete with the          */
/*      SCSI command block for a READ CAPACITY command.  This finds out   */
/*      the disk's capacity for data and passes this into the structures  */
/*      for later use.                                                    */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr   - a pointer to the disk structure for this device      */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_read_capacity( struct cd_disk_df *disk_ptr )
{
}


/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_pass_thru                                                  */
/*                                                                        */
/*   FUNCTION:  Builds and sends a pass-thru command to the CD ROM device.*/
/*                                                                        */
/*      This routine fills out a sc_buf struct complete with the          */
/*      SCSI command block for a pass-thru command and sends it to the    */
/*      SCSI adapter driver, which then passes the command on to the      */
/*      device.                                                           */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the top half of the device driver.  It    */
/*      can be called only by a process and can page fault.               */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr      - a pointer to the disk structure for this device   */
/*      sc_iocmd_ptr  - pointer to the user structure with the command    */
/*      xmem_desc_ptr - pointer to xmem struct (xmattach already called)  */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_pass_thru( struct cd_disk_df *disk_ptr,
      struct sc_iocmd *sc_iocmd_ptr,
      struct xmem *xmem_desc_ptr )
{
	struct sc_buf   *scsi_buf_ptr;
	int             i;
	/*
	 * Fill in the sc_buf, set the command state, and issue the command
	 * by calling the adapter driver's strategy routine via devstrat.
	 */
	scsi_buf_ptr = &disk_ptr->ioctl_buf.scsi_buf;
	scsi_buf_ptr->bufstruct.b_flags = sc_iocmd_ptr->flags;
	scsi_buf_ptr->bufstruct.b_blkno = 0x00;
	/*
 	 * The adapter driver should never look at this field
 	 */
	scsi_buf_ptr->bufstruct.b_error = 0x00;
	scsi_buf_ptr->bufstruct.b_un.b_addr = sc_iocmd_ptr->buffer;
	scsi_buf_ptr->bufstruct.b_bcount = sc_iocmd_ptr->data_length;

	bcopy((caddr_t) (xmem_desc_ptr),(caddr_t)
		&(scsi_buf_ptr->bufstruct.b_xmemd), sizeof(struct xmem));

	scsi_buf_ptr->bp = NULL;
	scsi_buf_ptr->scsi_command.scsi_length = sc_iocmd_ptr->command_length;
	scsi_buf_ptr->scsi_command.scsi_cmd.scsi_op_code = sc_iocmd_ptr->
								scsi_cdb[0];
	scsi_buf_ptr->scsi_command.scsi_cmd.lun = sc_iocmd_ptr->scsi_cdb[1];
	for (i=2 ; i < sc_iocmd_ptr->command_length ; i++)
	{
		scsi_buf_ptr->scsi_command.scsi_cmd.scsi_bytes[i-2] =
						sc_iocmd_ptr->scsi_cdb[i];
	}
	disk_ptr->cmd_state = CD_PASS_THRU;
return;
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_device_reset                                               */
/*                                                                        */
/*   FUNCTION:  Sends a TEST_UNIT_READY command to the CD ROM device.     */
/*                                                                        */
/*      When this command completes, the iodone routine will cause the    */
/*      device to go through the entire reset procedure:                  */
/*      RESERVE, MODE_SENSE, MODE_SELECT, READ_CAPACITY, and START_UNIT.  */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr        Pointer to the disk structure for this device.    */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_device_reset( struct cd_disk_df *disk_ptr )
{
	/*
	 * Fill in the sc_buf, set the command state, and issue the command
	 * by calling the adapter driver's strategy routine via devstrat.
	 */
	disk_ptr->reset_buf.scsi_buf.bufstruct.b_flags = B_READ;
	disk_ptr->reset_buf.scsi_buf.bufstruct.b_blkno = 0x00;
	disk_ptr->reset_buf.scsi_buf.bufstruct.b_un.b_addr = 0x00;
	disk_ptr->reset_buf.scsi_buf.bufstruct.b_bcount = 0x00;
	disk_ptr->reset_buf.scsi_buf.bufstruct.b_error = 0x00;
	disk_ptr->reset_buf.scsi_buf.bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
	disk_ptr->reset_buf.scsi_buf.bp = NULL;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_length = 0x06;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_cmd.scsi_op_code =
							SCSI_TEST_UNIT_READY;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_cmd.lun = disk_ptr->
							disk_ddi.lun_id<<5;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_cmd.scsi_bytes[0] = 0x00;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_cmd.scsi_bytes[1] = 0x00;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_cmd.scsi_bytes[2] = 0x00;
	disk_ptr->reset_buf.scsi_buf.scsi_command.scsi_cmd.scsi_bytes[3] = 0x00;
	disk_ptr->reset_buf.retry_count = 0;
	disk_ptr->cmd_state = SCSI_TEST_UNIT_READY;
	disk_ptr->reset_pending = TRUE;
	/* call devstrat */
	(void) devstrat(&disk_ptr->reset_buf.scsi_buf.bufstruct);
return;
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_start_stop                                                 */
/*                                                                        */
/*   FUNCTION:  Builds a START_STOP_UNIT command.                         */
/*                                                                        */
/*      This routine fills out a sc_buf struct complete with the          */
/*      SCSI command block for a START_STOP_UNIT command.  This command   */
/*      spins the disk up if it is opened up for the first time or        */
/*      prepares the disk to be spun down if the device is to be closed   */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr        - a pointer to the disk structure for this device */
/*      scsi_buf_ptr    - a pointer to the sc_buf to be used for this cmd */
/*      start_stop_flag - a flag to indicate either a START or a STOP     */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_start_stop( struct cd_disk_df *disk_ptr,
    struct sc_buf *scsi_buf_ptr,
    uchar start_stop_flag )
{
	/*
	 * Fill in the sc_buf, set the command state, and issue the command
	 * by calling the adapter driver's strategy routine via devstrat.
	 */
	(void) devstrat(&scsi_buf_ptr->bufstruct);
return;
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_start                                                      */
/*                                                                        */
/*   FUNCTION:  Sends a command to the CD ROM device.                     */
/*                                                                        */
/*      This routine is called when a command is to be issued to the      */
/*      specified device.  The routine begins by checking for the case    */
/*      of an ioctl.  For all ioctls other than reads, the scsi buf will  */
/*      already be filled out and only needs to be issued by calling the  */
/*      adapter driver's strategy routine.  For reads (either normal or   */
/*      ioctl read with sense),the routine begins by adjusting the length */
/*      of a read that will go past the end of the disk so that it will   */
/*      only read up to the last block.  If this is not a spanned command,*/
/*      the length is checked and adjusted so that it is not larger than  */
/*      the maximum that the adapter driver can handle.  Next the sc_buf  */
/*      is filled in for the read request and the initial retry count is  */
/*      set up.  Finally the command is issued via a call to the adapter  */
/*      driver's strategy routine.                                        */
/*                                                                        */
/*      One unique aspect of this routine is that in the case of a single */
/*      command that is too large to execute in one SCSI command, this    */
/*      routine will get called repetitively until the request is         */
/*      complete.  The active_resid variable is used to store the number  */
/*      of bytes remaining after each sub-command completes.  The blkno   */
/*      and usr_bfr_ptr paramaters are also needed exclusively for this   */
/*      situation so this routine knows where to start the read, and      */
/*      where to send the data (since these values may be different than  */
/*      what is in the buf struct).                                       */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr    - a pointer to the disk structure for this device     */
/*      bp          - first buf struct in a chain that this read is for   */
/*                    (this value will be NULL in the case of an ioctl)   */
/*      length      - total length of the SCSI command to issue (bytes)   */
/*                    (this value will be 0 in the case of an ioctl)      */
/*      blkno       - logical block address to start reading from         */
/*                    (this value will be 0 in the case of an ioctl)      */
/*      usr_bfr_ptr - where the read data is to be sent                   */
/*                    (this value will be NULL in the case of an ioctl)   */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_start( struct cd_disk_df *disk_ptr, uchar flag )
{
	struct sc_buf   *scsi_buf_ptr;
	struct buf      *bp;
	ushort          xfer_length;
	struct xmem     *xmemd_ptr;
	daddr_t         blkno;
	uint            length, transfer;
	caddr_t         usr_bfr_ptr;

	disk_ptr->cd_dkstat.dk_status |= IOST_DK_BUSY;
	if (disk_ptr->ioctl_pending)
	{
	/*
	 * This is an ioctl operation.
	 */
		if (disk_ptr->ioctl_buf.scsi_buf.scsi_command.scsi_cmd.
			scsi_op_code == SCSI_READ_EXTENDED)
		{
		/*
	 	 * This is a read with sense operation. Set up the bp,length,and
		 * blkno values so this operation can use the same path as the
		 * regular read.
	 	 */
			bp = &disk_ptr->rdse_buf_struct;
			xmemd_ptr = &bp->b_xmemd;
		}
		else
		{
		/*
	 	 * This is a pass-through command.
	 	 */
			bp = NULL;
			xmemd_ptr = &disk_ptr->ioctl_buf.scsi_buf.bufstruct.
					b_xmemd;
		}
		/*
	 	 * For an ioctl operation, retries are not done.
	 	 * The sc_buf used is the ioctl_buf.
	 	 */
		disk_ptr->ioctl_buf.retry_count = CD_NO_RETRY;
		scsi_buf_ptr = &disk_ptr->ioctl_buf.scsi_buf;
	}
	else
	{

/* Code was removed dealing with the diagnostic mode. */

		disk_ptr->norm_buf.retry_count = 0;
		scsi_buf_ptr = &disk_ptr->norm_buf.scsi_buf;
		bp = disk_ptr->head;
		xmemd_ptr = &bp->b_xmemd;
	}
	if (bp != NULL)
	{
	
	/*
	 * This is either a normal read or an ioctl read with sense.
	 * If this read will go beyond the end of the disc, shorten
	 * the length of the read to go only to the end and set the
	 * resid value to the remainder.
	 */
		if (flag == CD_ORIGINAL_REQUEST)
		{
			if (!(disk_ptr->ioctl_pending) && (bp->b_blkno + bp->
				b_bcount / disk_ptr->disk_nblks.len > disk_ptr->
				disk_nblks.lba+1))
			{
				length = (disk_ptr->disk_nblks.lba+1 -
						bp->b_blkno) * disk_ptr->
						disk_nblks.len;
			}
			else
			{
				length = bp->b_bcount;
			}
			blkno = bp->b_blkno;
			usr_bfr_ptr = bp->b_un.b_addr;
			bp->b_resid = bp->b_bcount - length;
		}
		else
		{ /* this is the continuation of a fragmented command */
			transfer = *(int *)(disk_ptr->disk_ddi.
					mode_select_data + 8);
			blkno = scsi_buf_ptr->bufstruct.b_blkno + (disk_ptr->
					max_transfer / transfer);
			usr_bfr_ptr = scsi_buf_ptr->bufstruct.b_un.b_addr +
					disk_ptr->max_transfer;
			length = bp->b_un.b_addr + bp->b_bcount - usr_bfr_ptr -
					bp->b_resid;
		}
		if (length > disk_ptr->max_transfer)
			length = disk_ptr->max_transfer;
		/*
		 * The xfer_length is the number of blocks to read.  This is
		 * used in the scsi command block.
		 */
		transfer = *(int *)(disk_ptr->disk_ddi.mode_select_data + 8);
		xfer_length = (ushort)(length/transfer);
		disk_ptr->cd_dkstat.dk_rblks += xfer_length;
		disk_ptr->cd_dkstat.dk_xfers++;
		/*
		 * Fill out the sc_buf for the read command and set the
		 * command state.
		 */
		disk_ptr->cmd_state = SCSI_READ_EXTENDED;
	}
	else
	{
	}
	/*
	 * Issue the command by calling the adapter driver's strategy routine
	 * via devstrat.
	 */
	(void) devstrat(&scsi_buf_ptr->bufstruct);
	return;
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_busy                                                       */
/*                                                                        */
/*   FUNCTION:  Called by cd_ioctl to check if a command is in progress.  */
/*                                                                        */
/*      The purpose of this routine is to provide a service to the ioctl  */
/*      routine to check if the queue is empty.  Interrupts must be       */
/*      disabled before the queue is checked by checking the busy flag.   */
/*      If it is busy, the routine sets the ioctl_rqst flag and returns   */
/*      a value of true.  If it is not busy, the busy flag and is set so  */
/*      the ioctl can issue the command, and the return value is set to   */
/*      FALSE (not busy).                                                 */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver      */
/*      because it must be pinned in memory and can not page fault.       */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr - a pointer to the disk structure for this device.       */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The return value is either TRUE if a command is already in        */
/*      progress, or FALSE if not.                                        */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  i_disable, i_enable                     */
/*                                                                        */
/**************************************************************************/

uchar cd_busy( struct cd_disk_df *disk_ptr )
{
	int     old_ilevel;
	uchar   return_value;

	old_ilevel = i_disable(INTIODONE);
	if (disk_ptr->busy)
	{
	/*
	 * There is currently a normal read in progress, so set the
	 * flag to indicate to the iodone routine that an ioctl
	 * operation needs to be kicked off.  Return the TRUE value
	 * to indicate to the ioctl routine to not start the operation.
	 */
		disk_ptr->ioctl_rqst = TRUE;
		return_value = TRUE;
	}
	else
	{
	/*
	 * No command is in progress, so it is ok to issue the ioctl
	 * command.  The busy and ioctl_pending flags are set to
	 * prevent a normal read from being issued(they will be
	 * queued up.  The return value is set to indicate to the
	 * ioctl routine that it is ok to call start to kick off
	 * the operation.
	 */
		disk_ptr->busy = TRUE;
		disk_ptr->ioctl_pending = TRUE;
		return_value = FALSE;
	}
	i_enable(old_ilevel);

	return(return_value);
}


/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_watchdog                                                   */
/*                                                                        */
/*   FUNCTION:  This routine is called when the watchdog timer pops.      */
/*                                                                        */
/*      The purpose of this routine is to call the cd_device_reset        */
/*      routine when the timer pops.  The purpose of this delay is to     */
/*      wait for the device to get back in the ready state after a bus    */
/*      reset.                                                            */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      watchdog_timer - address of the watchdog struct (this is also     */
/*                       the address of the disk structure.               */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION:  none                                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                        */
/**************************************************************************/

void cd_watchdog( struct watchdog *watchdog_timer )
{
	/*
	 * Call the reset procedure with the address of the disk structure.
	 * The watchdog timer structure was put at the front of the
	 * disk structure so that the addresses would be the same.
	 */
	cd_device_reset((struct cd_disk_df *)watchdog_timer);
	return;
}

/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_scsi_error                                                 */
/*                                                                        */
/*   FUNCTION:  Logs and determines how to handle SCSI errors.            */
/*                                                                        */
/*      This routine is called when a CHECK_CONDITION is detected.        */
/*      The sense key is checked to determine if the error should be      */
/*      logged when there is a CHECK_CONDITION.  If the command should    */
/*      be retried, CD_RETRY is returned.  If the device should be reset  */
/*      before a retry is attempted, CD_RESET is returned.  If a timer    */
/*      should be set before the reset is done, CD_WAIT is returned.      */
/*      Otherwise, the proper errno value to return to the user is        */
/*      returned to the caller.                                           */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      disk_ptr       - pointer to the disk structure for this device.   */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The return value is either the errno value that should be         */
/*      returned to the user,  a RESET flag to indicate that the device   */
/*      should be reset before the command is retried, a RETRY flag       */
/*      to indicate that the command should be retried, a WAIT flag to    */
/*      indicate that a timer should be set before the reset is attempted,*/
/*      or a 0 to indicate a recovered error.                             */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned to     */
/*      the calling routine (not directly to the user):                   */
/*      EIO       - Device is unable to receive cmds (stuck on unit attn.)*/
/*                - Hardware error.                                       */
/*                - Illegal request.                                      */
/*                - Aborted command.                                      */
/*      ENOTREADY - The device is not ready.                              */
/*      EMEDIA    - The CD has been changed.                              */
/*                - Media error.                                          */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  errsave                                 */
/*                                                                        */
/**************************************************************************/
uchar cd_scsi_error( struct cd_disk_df *disk_ptr )
{
}


/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_adapter_error                                              */
/*                                                                        */
/*   FUNCTION:  Logs and determines how to handle adapter errors.         */
/*                                                                        */
/*      This routine is called when a SCSI adapter error is detected and  */
/*      logs the error if necessary and determines what action to take.   */
/*      If the command should be retried, CD_RETRY is returned.  If the   */
/*      device should be reset before a retry is attempted, CD_RESET is   */
/*      returned.  If a timer should be set before the reset is done,     */
/*      CD_WAIT will be returned.  Otherwise, the proper errno value to   */
/*      return to the user is returned to the caller.                     */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      buf_ptr        - pointer to the buf_block used to issue the cmd.  */
/*      disk_ptr       - pointer to the disk structure for this device.   */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:                                            */
/*      The return value is either the errno value that should be         */
/*      returned to the user,  a RESET flag to indicate that the device   */
/*      should be reset before the command is retried, a RETRY flag       */
/*      to indicate that the command should be retried, a WAIT flag to    */
/*      indicate that a timer should be set before the reset is attempted,*/
/*      or a 0 to indicate a recovered error.                             */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned to     */
/*      the calling routine (not directly to the user):                   */
/*      EIO       - SCSI bus fault.                                       */
/*                - No device response.                                   */
/*                - Host i/o bus error.                                   */
/*                - SCSI adapter hardware failure.                        */
/*                - SCSI adapter software failure.                        */
/*                - SCSI adapter fuse or terminal power.                  */
/*                - SCSI bus reset (stuck on).                            */
/*      ETIMEDOUT - The command timed out.                                */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  errsave                                 */
/*                                                                        */
/**************************************************************************/
uchar cd_adapter_error( struct cd_buf_block *buf_ptr,
				struct cd_disk_df *disk_ptr)
{
}


/**************************************************************************/
/*                                                                        */
/*   NAME:  cd_iodone                                                     */
/*                                                                        */
/*   FUNCTION:  The iodone entry point (called by SCSI adapter driver).   */
/*                                                                        */
/*      This routine acts as the interrupt handler for this driver.       */
/*      There are several oddities about this routine.  First in the case */
/*      of read requests that are too big, a completed SCSI command does  */
/*      not necessarily mean that the user request is complete.  The      */
/*      active_resid field in the disk structure indicates how much of    */
/*      the original command remains.  Another problem is in the case of  */
/*      spanned commands. In this case, one SCSI command completion must  */
/*      result in calling iodone on more than one buf struct.             */
/*                                                                        */
/*      Before any other processing, this routine checks for a pass-thru  */
/*      command, and wakes up the ioctl process if that is the case.      */
/*      Next, the routine considers the good completion case.  The first  */
/*      condition checked is the case where a command resulted in a check */
/*      condition, the request sense was issued and just completed.       */
/*      The cd_scsi_error routine is called to determine how to handle the*/
/*      error.  In the case of an ioctl where the request sense data is   */
/*      needed, the SC_VALID_SENSE flag is set to indicate that the data  */
/*      is valid.  The returned error value is also checked to see if a   */
/*      recovered error is indicated.  In this case the command state is  */
/*      restored to its initial value (before the request sense) so that  */
/*      normal completion handling can be done.  Next the case of a read  */
/*      is handled by checking that the entire user request has completed.*/
/*      If not (the request was to big to be done in one command), the    */
/*      start routine is called again with the remainder of the request.  */
/*      Otherwise, iodone is called on all the bufs in the active chain.  */
/*      If there is an ioctl waiting to be executed, it is kicked off by  */
/*      calling start before the next read in the queue is executed.      */
/*      If the read was from an ioctl, iodone is not called, but the      */
/*      ioctl routine is woken up.  In either the normal or ioctl case,   */
/*      if the queue is not empty for this device, the first chain in     */
/*      the queue will be issued by calling the start routine.            */
/*                                                                        */
/*      If the command wasn't a read, it was either part of the reset     */
/*      procedure, a stop unit, release, or request sense.  If in the     */
/*      middle of a device reset, the next command in the reset procedure */
/*      needs to be kicked off.  If the reset procedure is done, either   */
/*      the process needs to be woken up (in the case of an open), or the */
/*      command issued before the reset needs to be re-issued. A complete */
/*      stop unit needs to kick off a release command.  A completed       */
/*      release command simply needs to have the process woken up.  The   */
/*      case of a request sense completion occurs only after a check      */
/*      condition occured on a previous command.  The error routine was   */
/*      called to determine how to handle the error.  If the request      */
/*      was a read, successful completion handling must be done for the   */
/*      portion of the read that was successful.  Then either the command */
/*      is retried, the device is reset, or it was a fatal error and will */
/*      be handled below.                                                 */
/*                                                                        */
/*      In the case of an error condition resulting from the command, the */
/*      first check is for an adapter error.  The error routine is called */
/*      to determine how to handle the error, and the command is retried, */
/*      the device is reset, or the error is fatal.  The next check is for*/
/*      scsi status of INTMD_GOOD or DEVICE_BUSY or    */
/*      RESERVATION_CONFLICT.  Any of these conditions are fatal.  The    */
/*      next check is for a check condition.  If this is the case, a      */
/*      request sense will be issued and it will go through the path      */
/*      mentioned above.                                                  */
/*                                                                        */
/*      For a fatal error, if a reset failed, a flag is set that the      */
/*      device is dead.  If the original request wasn't a read, the error */
/*      is set in the buf struct and the process is woken up.  For an     */
/*      ioctl, if the request sense data was requested, it is copied to   */
/*      another buffer.  The next command in the queue is then kicked off.*/
/*      In the case of a failed read, if a reset failed, iodone is called */
/*      on all the buf structs in the active queue and the normal queue.  */
/*      If the reset did not fail, the error value is set and iodone      */
/*      is called on the first buf struct in the active queue.  If the    */
/*      active queue is not empty, start is called for the remaining      */
/*      chain.  If the active queue is empty, then a waiting ioctl is     */
/*      started or the next command in the normal queue is started.       */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*      This routine is part of the bottom half of the device driver.     */
/*      It can be called on an interrupt level, is pinned in memory,      */
/*      and can not page fault.                                           */
/*                                                                        */
/*   DATA STRUCTURES:  none                                               */
/*                                                                        */
/*   INPUTS:                                                              */
/*      buf_ptr - the address of the buf struct, which is also the        */
/*                address of the sc_buf and the buf_block                 */
/*                                                                        */
/*   RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                        */
/*   ERROR DESCRIPTION: The following errno values may be returned in the */
/*      b_error field of the buf struct:                                  */
/*      EIO     - The request sense failed with a CHECK_CONDITION.        */
/*              - The scsi status was not expected (should never happen). */
/*      EBUSY   - Device is reserved by someone else, or device is busy.  */
/*      Plus others from cd_scsi_error or cd_adapter_error                */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  iodone,                                 */
/*                                e_wakeup,                               */
/*                                devstrat                                */
/*                                                                        */
/**************************************************************************/

void cd_iodone( struct buf *bp )
{
	struct cd_buf_block     *buf_ptr;
	struct buf              *cur_bp, *next_bp;
	struct sc_buf           *scsi_buf_ptr;
	struct cd_disk_df       *disk_ptr;
	int                     return_code;
	uchar                   error;

	/*
	 * The B_DONE flag must be cleared to force iowait to wait
	 * on this buf struct.  The flag will be set when processing
	 * is complete.
	 */
	bp->b_flags &= ~B_DONE;
	buf_ptr = (struct cd_buf_block *) bp;
	scsi_buf_ptr = (struct sc_buf *) bp;
	disk_ptr = buf_ptr->disk_ptr;
	error = FALSE;
	if (disk_ptr->ioctl_pending)
	{
		cur_bp = &disk_ptr->rdse_buf_struct;
	}
	else
	{
		cur_bp = disk_ptr->head;
	}
	if (disk_ptr->cmd_state == CD_PASS_THRU)
	{
	/*
	 * In the case of a pass-through command, the waiting process
	 * is simply woken up.  No error handling is done.
	 */
		bp->b_flags |= B_DONE;
		e_wakeup(&bp->b_event);
		disk_ptr->ioctl_pending = FALSE;
	}
	else
	{
		if (!(bp->b_flags & B_ERROR) && (bp->b_resid == 0))
		{
		/*
		 * The command completed successfully.
		 */
			if (disk_ptr->cmd_state == SCSI_REQUEST_SENSE)
			{
			/*
			 * The command was a request sense.  If this
			 * was a read with sense, the valid_sense flag
			 * is set and the sense data is moved to a
			 * different buffer to be sent to the user.
			 * The scsi_error routine is called and
			 * if it was a recovered error, the command
			 * state is restored to its original value so
			 * that normal completion of the original
			 * command can proceed.
			 */
			}
			switch (disk_ptr->cmd_state)
			{
				case SCSI_READ_EXTENDED:
				/*
				 * The command was a read.  Decrement
				 * the total byte count to see if the
				 * whole command is complete or if the
				 * command had to be broken up.
				 */
				/*
				 * If the command is done and ioctl
				 * pending is not set, then this
				 * was a normal read. Call iodone 
				 * on the buf struct and start the next
				 * ioctl if one is waiting or set the 
				 * flag if there is a read command
				 * waiting. If a recovered cmd, log the
				 * error.
				 */
				    if (disk_ptr->error_pending) {
				        errsave(&disk_ptr->log_struct,
				       	    sizeof(struct sc_error_log_df));
				        disk_ptr->error_pending = FALSE;
				    }
				    disk_ptr->head = disk_ptr->head->av_forw;
				    iodone(cur_bp);
				    if (disk_ptr->ioctl_rqst) {
				        disk_ptr->ioctl_rqst = FALSE;
				        disk_ptr->ioctl_pending = TRUE;
				        cd_start(disk_ptr, 
				    	    (uchar)CD_ORIGINAL_REQUEST);
				        return;
				    }
				    if (disk_ptr->head != NULL) {
				        cd_start(disk_ptr,
					    (uchar)CD_ORIGINAL_REQUEST);
				        return;
				    }
				/*
				 * Otherwise, this was a command that had to be
				 * broken up and is not actually complete yet.
				 * The rest of the command is  started.
				 */
				    break;
				case SCSI_TEST_UNIT_READY:
				/*
				 * A reset is in process and the next command
				 * needs to be started. Log the error if
				 * recovery was  necessary.
				 */
				case SCSI_RESERVE_UNIT:
				/*
				 * A reset is in process and the next
				 * command needs to be started.
				 * Log the error if recovery was necessary.
				 */
				case SCSI_MODE_SELECT:
				/*
				 * A reset is in process and the next
				 * command needs to be started.
				 * Log the error if recovery was necessary.
				 */
				case SCSI_READ_CAPACITY:
				/*
				 * A reset is in process and the next
				 * command needs to be started.
				 * Log the error if recovery was necessary.
				 */
					if (disk_ptr->error_pending)
					{
						errsave(&disk_ptr->log_struct,
							sizeof(struct
							sc_error_log_df));
						disk_ptr->error_pending = FALSE;
					}
					buf_ptr->retry_count = 0;
					cd_start_stop(disk_ptr, scsi_buf_ptr,
							(uchar)CD_START_UNIT);
					return;
				case SCSI_START_STOP_UNIT:
				/*
				 * Log the error if recovery was  necessary.
 				 * A reset just completed. Either this
				 * is the open process and the open
				 * needs to be woken up, or this was a
				 * recovery reset and the original
				 * command needs to be retried.
				 */
				case SCSI_RELEASE_UNIT:
				/*
				 * The release just completed.  Wake up the
				 * waiting process. Log the error if recovery
				 * was necessary.
				 */
					if (disk_ptr->error_pending)
					{
						errsave(&disk_ptr->log_struct,
							sizeof(struct
							sc_error_log_df));
						disk_ptr->error_pending = FALSE;
					}
					bp->b_flags |= B_DONE;
					e_wakeup(&bp->b_event);
					break;
				case SCSI_REQUEST_SENSE:
				/*
				 * The command was a request sense (and not a
				 * recovered error).  The error routine has
				 * already been called and the return value is
				 * in 'error'.
				 */
					if (!disk_ptr->reset_pending)
					{
						disk_ptr->cmd_state = disk_ptr->
							old_cmd_state;
					}
					else
					{
						disk_ptr->cmd_state = disk_ptr->
							reset_state;
					}
					if (error == CD_WAIT)
					{
					/*
					 * A reset needs to be attempted
					 * but the device is not ready
					 * so timeout is called to
					 * schedule the reset in 5 secs.
					 */
						w_start(&disk_ptr->
								watchdog_timer);
						return;
					}
					if (error == CD_RESET)
					{
					/*
					 * A reset needs to be attempted
					 * so call the routine to start the
					 * procedure.
					 */
						cd_device_reset(disk_ptr);
						return;
					}
					if (error == CD_RETRY)
					{
					/*
					 * The command should be retried so call
 					 * devstrat.
					 */
						if (disk_ptr->cmd_state ==
							SCSI_READ_EXTENDED)
						{
							(void)devstrat(
							    &disk_ptr->norm_buf.
							    scsi_buf.bufstruct);
						}
						else
						{
							(void)devstrat(
							     &disk_ptr->
							     reset_buf.scsi_buf.
							     bufstruct);
						}
						return;
					}
					/*
					 * A scsi bus reset recovery failed.
					 * The cmd_state is set to the original
					 * command for fatal error processing.
					 */
					if (disk_ptr->reset_pending &&
						!disk_ptr->open_pending)
					{
						disk_ptr->cmd_state = disk_ptr->
							old_cmd_state;
					}
					break;
			}
		}
		else
		{
		/*
		 * The command resulted in some sort of error (B_ERROR
		 * is set or there is a resid value).  First check for
		 * an adapter error, and then for a SCSI error.
		 */
			bp->b_flags &= ~B_ERROR;
			if (scsi_buf_ptr->status_validity & SC_ADAPTER_ERROR)
			{
			/*
			 * An adapter error was detected.  Call the
			 * adapter_error routine to determine if the
			 * command should be retried, the device should
			 * be reset, or if the error is fatal.
			 */
/* The function cd_adapter error was removed.  It is a hardware specific
   routine that returns information about error occuring in the SCSI adapter
   driver.  The routine also logs the error if necessary and determine 
   the appropriate action to take.
*/
				error = cd_adapter_error(buf_ptr, disk_ptr);
				if (!disk_ptr->reset_pending && disk_ptr->
					cmd_state != SCSI_REQUEST_SENSE)
				{
					disk_ptr->old_cmd_state = disk_ptr->
								     cmd_state;
				}
				if (error == CD_WAIT)
				{
					w_start(&disk_ptr->watchdog_timer);
					return;
				}
				if (error == CD_RESET)
				{
					cd_device_reset(disk_ptr);
					return;
				}
				if (error == CD_RETRY)
				{
				/* call devstrat to re-issue the command */
					(void) devstrat(bp);
					return;
				}
			}
			else if (scsi_buf_ptr->status_validity & SC_SCSI_ERROR)
			{ /* valid SCSI status */
			/*
			 * There was no adapter error but there was
			 * a SCSI error, so check the SCSI status.
			 */
/* Code was removed dealing with validating an error condition. */

			}
			else
			{
			/*
			 * The adapter indicated an error by setting the resid
			 * value with no other status.  Call adapter_error
			 * routine to determine if the command should be
			 * retried, the device should be reset, or if the error
			 * is fatal.
			 */
			}
			/*
			 * If the command was a request sense, the command state
			 * is restored to its original value before the fatal
			 * error processing below.
			 */
			if (disk_ptr->cmd_state == SCSI_REQUEST_SENSE)
			{
				if (!disk_ptr->open_pending)
				{
				/*
				 * The original command was the one
				 * issued before the req. sense was  issued.
				 */
					disk_ptr->cmd_state = disk_ptr->
								old_cmd_state;
				}
				else
				{
				/*
				 * An open was in progress, so the "original"
				 * command was the reset procedure.
				 */
					disk_ptr->cmd_state = disk_ptr->
								reset_state;
				}
			}
		}
		if (error)
		{
		/*
		 * The error was not recoverable.
		 */
/* The code removed deals with how to handle a non-recoverable error. */
		}
	}
	/*
	 * There is no ioctl waiting to start, and no read requests waiting
	 * in the queue, so the device is no longer busy and the busy flag
	 * can be cleared.
	 */
	disk_ptr->busy = FALSE;
	disk_ptr->cd_dkstat.dk_status &= ~IOST_DK_BUSY;
return;
}
