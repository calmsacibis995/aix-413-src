static char sccsid[] = "@(#)56  1.16  src/rspc/kernext/idedisk/idediskb.c, diskide, rspc41J, 9520A_all 5/16/95 16:57:23";
/*
 * COMPONENT_NAME: (IDEDISK) IDE Disk Device Driver Bottom Half Routines
 *
 * FUNCTIONS
 * 		idedisk_strategy
 * 		idedisk_pending_enqueue
 *		idedisk_pending_dequeue
 *		idedisk_dump
 *		idedisk_dump_write
 *		idedisk_process_dmp_error
 * 		idedisk_start_disable
 *		idedisk_build_cmd
 *		idedisk_start
 * 		idedisk_iodone
 * 		idedisk_process_good
 *		idedisk_process_error
 *		idedisk_process_dev_error
 *		idedisk_process_buf_error
 *		idedisk_process_verify_error
 *		idedisk_process_partial
 * 		idedisk_process_buf
 *		idedisk_coalesce
 *		idedisk_build_error
 *		idedisk_log_error
 * 		idedisk_cdt_func
 * 		idedisk_q_cmd
 * 		idedisk_d_q_cmd
 * 		idedisk_cmd_alloc_disable
 *		idedisk_cmd_alloc
 *		idedisk_free_cmd_disable
 * 		idedisk_free_cmd
 * 		idedisk_sleep
 * 		idedisk_trc_disable
 *		idedisk_trc
 *		idedisk_spinupdn
 *		idedisk_pm_watchdog
 *		idedisk_pm_handler
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

struct idedd_info	idedd_info = { 0, 0, LOCK_AVAIL, NULL };

struct idediskinfo	*idedisk_list[DK_MAXCONFIG] = { NULL };

struct idediskinfo	*idedisk_open_list[DK_MAXCONFIG] = { NULL };

#ifdef DEBUG
int			idedisk_debug = TRUE;

struct	idedisk_trace	*idedisk_trctop = NULL;
struct	idedisk_trace	*idedisk_trace;
	int		idedisk_trcindex = 0;

/*
 ******  strings for Internal Debug Trace Table *******
 * To use compile with the flags IDE_GOOD_PATH and IDE_ERROR_PATH
 * the variable idedisk_trace will be the beginning of the trace table in
 * memory.  The variable idedisk_trctop will point to location where the
 * next entry in the trace table will go.
 */

char      *topoftrc     = "*DKTOP**";

char     *strategy      = "STRATEGY";
char     *start         = "START   ";
char     *coales        = "COALESCE";
char     *interrupt     = "INTR    ";
char     *good          = "GOOD	   ";
char     *error         = "ERROR   ";
char     *idiodone      = "IODONE  ";
char     *failbuf       = "FAIL_BUF";
char     *faildisk      = "FAILDISK";
char     *ideerror      = "IDE_ERR ";
char     *partial       = "PARTIAL ";
char     *processbuf    = "BUFCMPLT";
char     *qcmd          = "Q_CMD   ";
char     *dqcmd         = "D_Q_CMD ";
char     *cmdalloc      = "CMDALLOC";
char     *cmdfree       = "FREE_CMD";
char     *spinupdn      = "SPINUPDN";
char     *logerr        = "LOG ERR ";
char     *buildcmd	= "BUILDCMD";
char	 *issue		= "ISSUE   ";
char	 *dump		= "DUMP    ";
char     *entry         = " IN";	/* Entry into routine                */
char     *exit          = " EX";	/* Exit from routine                 */
char     *trc           = " TR";	/* Trace point in routine            */
#endif


/************************************************************************/
/*									*/
/*	NAME:	idedisk_strategy					*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Strategy Routine.		*/
/*		This routine is called with a NULL terminated linked	*/
/*		list of buf structs. It scans the incoming list in an	*/
/*		attempt to coalesce contiguous write operations to the	*/
/*		same device together in a single operation.		*/
/*		Coalesced requests are placed in an IN_PROGRESS queue	*/
/*		where they can be processed by idedisk_start. Requests	*/
/*		that are not coalesced are placed in a PENDING queue	*/
/*		where they will be processed by idedisk_start in future	*/
/*		IO operations.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use i_disable, i_enable.				*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*		The device's busy flag is set and cleared during	*/
/*		coalescing to insure the request queues integrity	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		idediskinfo	General disk driver information		*/
/*		idedisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		bp	- Address of first buf struct in list		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned in the	*/
/*		buf struct b_error field:				*/
/*			EIO	- Disk is offline or starting to close	*/
/*			ENXIO	- Requested sector beyond end of media	*/
/*				- Disk is not open			*/
/*			EINVAL	- Attempt to reassign (not supported)	*/
/*				- Block size is not 512			*/
/*				- b_bcount is larger than maxrequest	*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable		i_enable			*/
/*		iodone			minor				*/
/*									*/
/************************************************************************/

int
idedisk_strategy(
	struct buf	*bp)
{
	int			blkno, count, rc;
	int			opri, last_blk, dev;
	dev_t			devno, first_devno;
	struct buf		*next_bp;
	struct idediskinfo	*diskinfo, *last_diskinfo = NULL;
	struct idedisk_stats	*stats;

	first_devno = bp->b_dev;
	
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(strategy, entry, (char) 0, (uint) bp, (uint) bp->b_dev,
	      (uint) bp->b_blkno, (uint) bp->av_forw, (uint) 0);
#endif	
#endif
	DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_STRATEGY, 0, first_devno, bp,
		bp->b_flags, bp->b_blkno, bp->b_bcount);
		
	while (bp != NULL)  {
		devno = bp->b_dev;
		count = bp->b_bcount;
		next_bp = bp->av_forw;

		/* Locate diskinfo structure in idedisk_open_list */
		dev = minor(devno);
		diskinfo = idedisk_open_list[dev];
 
		if (diskinfo == NULL) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			bp->b_resid = bp->b_bcount;
			iodone(bp);

		} else {
			opri = i_disable(INTTIMER);

			last_blk = diskinfo->capacity.lba;
			diskinfo->cmd_pending = TRUE;

		  	IDEDISK_GET_LBA(blkno,diskinfo,bp,rc);
			if (rc) {
			  	/*
                                 * Fail any buf if diskinfo->block_size
                                 * is not DK_BLOCKSIZE.
                                 */
                                bp->b_flags |= B_ERROR;
                                bp->b_error = EINVAL;
                                bp->b_resid = bp->b_bcount;
				iodone(bp);

			} else if (bp->b_bcount > diskinfo->max_request) {
				/* 
				 * Fail any buf that is greater than our 
				 * maximum transfer size
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EINVAL;
				bp->b_resid = bp->b_bcount;
				iodone(bp);

			} else if (blkno > (last_blk+1)) {
				/*
				 * Verify block address on device
				 */
				bp->b_resid = bp->b_bcount;
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
				iodone(bp);

			} else if (blkno == (last_blk+1)) {
				bp->b_resid = bp->b_bcount;
				if (!(bp->b_flags & B_READ)) {
					bp->b_flags |= B_ERROR;
					bp->b_error = ENXIO;
				}
				iodone(bp);

			} else if (bp->b_bcount == 0) {
				/*
				 * Check for zero length read or write
				 */
				bp->b_resid = 0;
				iodone(bp);

			} else if ((diskinfo->state == DK_OFFLINE) ||
					(diskinfo->starting_close)) {
				/*
				 * Verify transfer validity
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EIO;
				bp->b_resid = bp->b_bcount;
				iodone(bp);

			} else if ( (count & (diskinfo->block_size - 1)) ||
				(bp->b_options & (HWRELOC | UNSAFEREL)) ) {

				bp->b_flags |= B_ERROR;
				bp->b_error = EINVAL;
				bp->b_resid = bp->b_bcount;
				iodone(bp);

			} else {
				/*
				 * If we got here then we passed all 
				 *  of the above checks
				 */

				/* 
				 * Filter out operation beyond last block 
				 * of disk 
				 */
				if ((blkno + (count/diskinfo->block_size)) > 
							(last_blk + 1)) {
					bp->b_resid = ((blkno + 
						(count/diskinfo->block_size))
						- (last_blk + 1)) * 
						diskinfo->block_size;
				} else {
					bp->b_resid = 0x00;
				}
				
				/* Update transfer statistics for device */
				stats = &(diskinfo->stats);
				if ( (bp->b_flags & B_READ) &&
				     ((stats->byte_count += bp->b_bcount) >
				      stats->segment_size) ) {

					stats->segment_count++;
					stats->byte_count %= 
						stats->segment_size;
				}
				
				if ((diskinfo != last_diskinfo) &&
				    (last_diskinfo != NULL)) {

					idedisk_start(last_diskinfo);
				}

				/*
				 * Put buf on this disk's pending queue
				 */
				idedisk_pending_enqueue(diskinfo, bp);
			}
			/*
			 * Enable intrs and unlock for next
			 * pass thru
			 */

			i_enable(opri);

		} /* else: diskinfo != NULL */

		last_diskinfo = diskinfo;
		bp = next_bp;

		if (diskinfo != NULL) {
			diskinfo->cmd_pending = FALSE;
		}
	} /* while */

	if (diskinfo != NULL) {
		/*
		 * Call start for the last disk processed above
		 */
		idedisk_start_disable(diskinfo);

		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_STRATEGY, 0, first_devno);
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idedisk_trc(strategy, exit, (char) 0, (uint) diskinfo,
				(uint) 0, (uint) 0, (uint) 0, (uint) 0);
#endif	
#endif
	}

	return(0);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_pending_enqueue					*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Pending Enqueue Routine	*/
/*		This routine is called with a diskinfo structure pointer*/
/*		and a pointer to a buf structure to insert into the    	*/
/*		disk's pending queue. The insertion into the list is    */
/*		based on the target logical block address of the request*/
/*		The list is ordered in ascending logical block address  */
/*		to provide seek optimization when processing disk 	*/
/*		requests.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use i_disable, i_enable.				*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		buf		System I/O Request Buffer Structure	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	- Pointer to diskinfo structure		*/
/*		bp		- Pointer to buf to insert		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:	None				*/
/*									*/
/*	ERROR DESCRIPTION:		None				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

void
idedisk_pending_enqueue(
	struct idediskinfo	*diskinfo, 
	struct buf		*bp)	
{	
	struct buf		*curr, *prev;

        curr = diskinfo->low;	/* set current pointer to low_cyl pointer */ 
        prev = NULL;		/* set previous pointer to NULL */

        if ( curr == NULL) {
		/*
		 * we are starting with an empty list 
		 */
		diskinfo->low = bp;	/* set low ptr to this one */
		diskinfo->currbuf = bp;	/* set current ptr to this one */
                bp->av_forw = NULL;	/* set this ones forward (NULL) */
                bp->av_back = NULL;	/* set this ones back (NULL) */
		return;
	}
		
	do {
                if (curr->b_blkno <= bp->b_blkno) {
			/*
			 * if this one is less than or equal to the one we're
			 * trying to insert set prev to this one, and walk
			 * the curr pointer to the next one
			 */
			prev = curr;
			curr = curr->av_forw;
                } else {
			/*
			 * else it should go before this one 
			 */
                        if ( prev == NULL) {
				/*
				 * becomes new low request
				 */
				diskinfo->low = bp;
				bp->av_back = NULL;
			} else {
				prev->av_forw = bp;
				bp->av_back = prev;
			}
			bp->av_forw = curr;
			curr->av_back = bp;
			return;
        	}
        
	} while (curr != NULL);

	/*
	 * we're out of the while, so if curr is NULL, then we
	 * walked to the end of the list
	 */
	prev->av_forw = bp;	/* set prev's forward ptr to this */
	bp->av_back = prev;	/* set back pointer to prev */
	bp->av_forw = NULL;	/* set bp's forward to NULL */

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_pending_dequeue					*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Pending Dequeue Routine	*/
/*		This routine is called with a diskinfo structure 	*/
/*		pointer.  This routine is called to remove the current  */
/*		buf from the pending queue. 				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use i_disable, i_enable.				*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		buf		System I/O Request Buffer Structure     */
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Pointer to disk information structure   */	
/*									*/
/*	RETURN VALUE DESCRIPTION:	None				*/
/*									*/
/*	ERROR DESCRIPTION:		None				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

void
idedisk_pending_dequeue(
	struct idediskinfo	*diskinfo)
{				
	struct buf *bp;
	
	bp = diskinfo->currbuf;
	/*
	 * set current to next
	 */
	diskinfo->currbuf = bp->av_forw;	
	if (bp == diskinfo->low)  
		/*
		 * if this was our low buf, set low to next
		 */
		diskinfo->low = bp->av_forw;

	if (bp->av_forw != NULL)
		/*
		 * if this not end, set next in list back link
		 */
		bp->av_forw->av_back = bp->av_back;

	if (bp->av_back != NULL)
		/*
		 * if this has a previous link, set it to next
		 */
		bp->av_back->av_forw = bp->av_forw;

	if (diskinfo->currbuf == NULL)	
		/*
		 * if this is the end, set curr to list back link
		 */
		diskinfo->currbuf = diskinfo->low;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_dump						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is the entry point for the system		*/
/*		to utilize the disk as the dump device.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine should be considered part of the bottom	*/
/*		half of the driver. It may not rely on any intrs	*/
/*		and should refrain from using any system services.	*/
/*		It can not page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		idedisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- address of uio struct describing operation	*/
/*		cmd	- operation to be performed			*/
/*		arg	- address of caller argument structure		*/
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
/*			EINVAL		- invalid request		*/
/*			EIO		- adapter in unknown state	*/
/*			ENXIO		- disk is not open		*/
/*					- specified adapter devno	*/
/*					  doesn't exist			*/
/*			ETIMEDOUT	- operation timed-out		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devdump			minor				*/
/*		bzero							*/
/*									*/
/************************************************************************/

int
idedisk_dump(
	dev_t		devno,
	struct uio	*uiop,
	int		cmd, 
	int		arg, 
	int		chan, 
	int		ext)
{
	struct dmp_query	*dump_ptr, tmp_buf;
	struct iovec		*iovp;
	struct idediskinfo	*diskinfo;
	struct ataide_buf	*idebuf;
	struct dk_cmd		cmd_ptr;
	int			dev, i, blk_cnt, errnoval, opri;
	uint			abs_track, lba;
	ushort			cylinder;

	/*
         *  Check for a valid minor number.
         */
	errnoval = 0x00;
	bzero((char *) &cmd_ptr, (int)sizeof(struct dk_cmd));
	dev = minor(devno);

	/*
         * Locate diskinfo structure in opened device list.
         */
	diskinfo = idedisk_open_list[dev];

	/*
         *  Return an error if the disk has not been opened.
         */
	if (diskinfo == NULL) {
		return(ENXIO);
	}

	switch (cmd) {
	case DUMPINIT:
		/*
                 *  Initialize the adapter driver for a dump.  No other action
                 *  is necessary.
                 */
		errnoval = devdump(diskinfo->adapter_devno, 0, DUMPINIT,
					arg, chan, ext);
		if (!errnoval) {
			diskinfo->dump_inited = TRUE;
		}
		break;
	case DUMPSTART:
		/*
                 *  Tell the adapter driver a dump is about to happen. No other
                 *  action is necessary.  This will also prevent start from
		 *  issuing any more commands
                 */
		diskinfo->state |= DK_DUMP_PENDING;

                if (ext == PM_DUMP) {
                        errnoval = devdump(diskinfo->adapter_devno, 0,
                                           DUMPSTART,
                                           diskinfo->device_id, chan, ext);
                } else {
                        errnoval = devdump(diskinfo->adapter_devno, 0,
                                           DUMPSTART,
                                           arg, chan, ext);
                }
		break;
	case DUMPQUERY:
		/*
                 *  Return blocksize and maximum transfer size to the caller.
                 */
		dump_ptr = (struct dmp_query *) arg;
		errnoval = devdump(diskinfo->adapter_devno, 0, DUMPQUERY,
					&tmp_buf, chan, ext);
		if (errnoval == 0) {
			if (tmp_buf.min_tsize > diskinfo->block_size) {
				errnoval = EINVAL;
			} else {
				dump_ptr->min_tsize = diskinfo->block_size;
				if (tmp_buf.max_tsize>diskinfo->max_request) {
					dump_ptr->max_tsize = 
					    diskinfo->max_request;
				} else {
					dump_ptr->max_tsize=tmp_buf.max_tsize;
				}
			}
		}
		break;
	case DUMPWRITE:
		/*
                 *  A dump is in progress.  We do not have to worry about
                 *  breaking up large commands, so just fill in an ataide_buf
                 *  and issue the command via devdump for each iovec.
                 */
		iovp = uiop->uio_iov;
		lba = uiop->uio_offset / diskinfo->block_size;
		for (i = 0; i < uiop->uio_iovcnt; i++) {
			if (((int) iovp->iov_len & 0x1ff) || 
			    (!(diskinfo->state & DK_ONLINE))) {
				/*
				 * if the data length is not a multiple
				 * of 512 or the disk is not online,
				 * return an error
				 */
				return(EINVAL);
			}

			blk_cnt = iovp->iov_len / diskinfo->block_size;

			if ((blk_cnt > 256) || (lba & 0xf0000000)) {
				/*
				 * block count must be <= 256 and LBA
				 * must be < 2^28 (ATA-2)
				 */
				return(EINVAL);

			}

			idebuf = &(cmd_ptr.idebuf);
			idebuf->bufstruct.b_flags = B_WRITE;
			idebuf->bufstruct.b_dev = diskinfo->adapter_devno;
			idebuf->bufstruct.b_blkno = lba;
			idebuf->bufstruct.b_un.b_addr = iovp->iov_base;
			idebuf->bufstruct.b_bcount = iovp->iov_len;
			idebuf->bufstruct.b_error = 0x00;
			idebuf->bufstruct.b_resid = 0x00;
			idebuf->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
			idebuf->bp = NULL;
			idebuf->ata.device = diskinfo->device_id;
			idebuf->ata.command = ATA_WRITE_DMA_RETRY;
			idebuf->ata.sector_cnt_cmd = blk_cnt & 0xFF;
			/*
			 * Determine start address using the appropriate
			 * format (CHS/LBA)
			 */
			CONVERT_IF_CHS((&(idebuf->ata)),lba);

			idebuf->timeout_value = diskinfo->rw_timeout;

			cmd_ptr.diskinfo = diskinfo;
			cmd_ptr.type = DK_BUF;

			errnoval = idedisk_dump_write(diskinfo, &cmd_ptr,
							chan, ext);
			if (errnoval) {
				return(errnoval);
			}
			/* 
			 * get lba for next iovec struct 
			 */

			lba += iovp->iov_len / diskinfo->block_size;

			/* 
			 * get next iovec struct 
			 */
			uiop->uio_resid -= iovp->iov_len;
			iovp->iov_len = 0;
			iovp = (struct iovec *) (((int) iovp) + 
			    sizeof(struct iovec));
		} /* for loop */
		break;
	case DUMPEND:
		/*
                 *  Tell the adapter driver the dump is over.  No other action
                 *  is necessary.
                 */
		errnoval = devdump(diskinfo->adapter_devno, 0, DUMPEND,
					arg, chan, ext);
		diskinfo->state &= ~DK_DUMP_PENDING;
		break;
	case DUMPTERM:
		/*
                 *  Tell the adapter driver that this device will no longer
                 *  be used as a dump device, no other action is necessary.
                 */
		errnoval = devdump(diskinfo->adapter_devno, 0, DUMPTERM,
					arg, chan, ext);
		diskinfo->dump_inited = FALSE;
		break;
	default:
		/*
                 *  Invalid dump command.
                 */
		errnoval = EINVAL;
		break;
	}

	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_dump_write					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is responsible for issuing write requests	*/
/*		for DUMPWRITE and doing some minimal error recover	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine should be considered part of the bottom	*/
/*		half of the driver. It may not rely on any interrupts	*/
/*		and should refrain from using any system services.	*/
/*		It can not page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		idebuf		ATA/IDE buf used to interface with IDE	*/
/*				adapter driver.				*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*		write_cmd_ptr	Write request for dump			*/
/*									*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		Any errno values returned from devdump may be returned	*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devdump							*/
/*									*/
/************************************************************************/
int
idedisk_dump_write(
	struct idediskinfo	*diskinfo,
	struct dk_cmd		*write_cmd_ptr,
	int			chan, 
	int			ext)
{
	int		errnoval = 0;	 /* return code of this routine */
	struct dk_cmd	*dmp_cmd_ptr;	 /* command to be issued to 	*/
					 /* the IDE adapter driver	*/
	dmp_cmd_ptr = write_cmd_ptr;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH

	/*
	 * On the dump write path interrupts are disable at INTMAX
	 * and the dump device driver locks out all other CPU's.
	 * Thus we can access our trace table without locking here.
	 */
	idedisk_trc(dmpwrt, entry, (char) 0,
		(uint) diskinfo, 
		(uint) dmp_cmd_ptr,
		(uint) dmp_cmd_ptr->type, 
		(uint) dmp_cmd_ptr->retry_count,
		(uint) 0);
#endif	
#endif

	/*
	 * Loop until all retries and error recovery are exhausted
	 */
	
	while (TRUE) {

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idedisk_trc(dmpissue, trc, (char) 0, 
			   (uint) dmp_cmd_ptr, 
			   (uint) dmp_cmd_ptr->type,
			   (uint) dmp_cmd_ptr->subtype, 
			   (uint) dmp_cmd_ptr->retry_count,
			   (uint) dmp_cmd_ptr->status);
#endif	
#endif
   		/*
		 * Issue a command to the IDE adapter driver's
		 * dump entry point.
		 */

		errnoval = devdump(diskinfo->adapter_devno, 0, DUMPWRITE,
					&(dmp_cmd_ptr->idebuf), chan, ext);
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idedisk_trc(dmpissue, trc, (char) 1, 
				(uint) dmp_cmd_ptr, 
				(uint) dmp_cmd_ptr->type,
				(uint) dmp_cmd_ptr->subtype,
				(uint) errnoval,
				(uint) dmp_cmd_ptr->status);
#endif	
#endif
		if (!errnoval) {
			if (dmp_cmd_ptr->type == DK_BUF) {
				/*
				 * If type is DK_BUF, then this must be
				 * original dump write command.
				 */
			  	if (dmp_cmd_ptr->idebuf.bufstruct.b_resid != 0){
				  	/*
					 * This command completed with
					 * a resid so we must retry
					 * it.
					 */
				  	dmp_cmd_ptr = write_cmd_ptr;
					dmp_cmd_ptr->retry_count++;
				} else {
				       	/*
					 * This command completed successfully
					 * break and return
					 */
				  	break;  
				}
			} else {
				/* 
				 * This shouldn't happen, but just in
				 * case it does, lets retry the original
				 * dump write command.
				 */
				ASSERT(FALSE);
				dmp_cmd_ptr = write_cmd_ptr;
			}
		} else {
			/*
			 * An error occurred on the DUMPWRITE call
			 * to the adapter driver.  Determine type of
			 * error.
			 */
			dmp_cmd_ptr->retry_count++;
		  	dmp_cmd_ptr = write_cmd_ptr;

			/*
			 * Clear the error from other fields in command.
			 */ 
			dmp_cmd_ptr->idebuf.bufstruct.b_flags &=
						~(B_ERROR | B_DONE);
			dmp_cmd_ptr->idebuf.bufstruct.b_error = 0x00;
			dmp_cmd_ptr->idebuf.status_validity = 0;
		}
		if (write_cmd_ptr->retry_count >= DK_MAX_RETRY) {
			/*
			 * When we've exceeded our retries on the original
			 * command, then exit from the loop.  We should
			 * never loop forever, because the dump error
			 * recovery will eventually retry the original command
			 * and hence increment retry_count.
			 */
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
			idedisk_trc(dmpwrt, trc, (char) 0, 
				   (uint) diskinfo, 
				   (uint) dmp_cmd_ptr,
				   (uint) dmp_cmd_ptr->retry_count, 
				   (uint) write_cmd_ptr->retry_count,
				   (uint) 0);
#endif	
#endif
			if (!errnoval) {
			  	/*
				 * If errnoval is not set (for
				 * example due to a resid failure),
				 * then set errnoval to EIO.
				 */
			  	errnoval = EIO;
			}
			break;
		}
	}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
	idedisk_trc(dmpwrt, exit, (char) 0,
			(uint) diskinfo, 
			(uint) dmp_cmd_ptr,
			(uint) dmp_cmd_ptr->type, 
			(uint) dmp_cmd_ptr->retry_count,
			(uint) errnoval);
#endif	
#endif

	return (errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_start_disable					*/
/*									*/
/*	FUNCTION: Calls start for pageable routines			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called at process level.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo - Address of disk information structure	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable		i_enable			*/
/*									*/
/************************************************************************/

void
idedisk_start_disable(
	struct idediskinfo	*diskinfo)
{
	struct dk_cmd		*cmd_ptr; 	/* command block pointer*/
	int			opri;

	opri = i_disable(INTTIMER);

	idedisk_start(diskinfo);

	i_enable(opri);

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_build_cmd					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine scans the IN_PROGRESS queue for the	*/
/*		specified device and builds an IDE operation to		*/
/*		satisfy the request list. The IN_PROGRESS queue		*/
/*		may contain one of two different types of cmd		*/
/*		lists. The list will be either				*/
/*		1 - a group of contiguous write operations		*/
/*		    (possibly including a verify flag)			*/
/*		2 - a group of contiguous read operations		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*		This routine will build either a read or write		*/
/*		request as specified.					*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*		head		pointer to the pointer to the buf	*/
/*				 struct for which the command is being	*/
/*				 built.					*/
/*		good_path	Boolean specifying whether the special	*/
/*				 performance trace should be invoked	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		pointer to disk cmd	- successfully created		*/
/*		NULL			- no disk cmd structs available	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		printf			bcopy				*/
/*									*/
/************************************************************************/

struct dk_cmd *
idedisk_build_cmd(
	struct	idediskinfo	*diskinfo,
	struct	buf		**head,
	char			good_path)
{
	int		blk_cnt, lba, total_count;
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp, *bp, *tmp_bp;
	struct ata_cmd	*ata, *verify_ata;
	int		rc;
	uint		blkno;
	uint		abs_track;	/* absolute track offset of request */
	ushort		cylinder;	/* cylinder of request */
	uchar		read_command, write_command;

        /* Start with head of IN_PROGRESS queue */
        bp = *head;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
	idedisk_trc(buildcmd, trc, (char) 0, (uint) diskinfo->state, (uint) bp,
		(uint) bp->b_flags, (uint) bp->b_resid, (uint) bp->b_options);
#endif	
	if (bp == NULL) {
		printf("idedisk_build_cmd: NULL in_progress.head\n");
		ASSERT(FALSE);
	}
#endif

	IDEDISK_GET_LBA(blkno,diskinfo,bp,rc);

#ifdef DEBUG
	if (rc) {
	  	/*
		 * Request does not start on device
		 * block boundary.  
		 * This should be filtered at 
		 * idedisk_strategy.
		 */
	  	ASSERT(rc == 0);
	}
#endif

	/* allocate cmd for read/write operation */

	cmd_ptr = idedisk_cmd_alloc(diskinfo,(uchar)DK_BUF);
	if (cmd_ptr == NULL) {
		return(cmd_ptr);
	}
	if (good_path)
		DDHKWD3(HKWD_DD_IDEDISKDD, DD_COALESCE, 0,
			diskinfo->devno, cmd_ptr->bp, &(cmd_ptr->idebuf));

	cmd_bp = &(cmd_ptr->idebuf.bufstruct);

	/* increment transfer count */
	diskinfo->dkstat.dk_xfers++;

	/* Initialize cmd block for the operation */
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = bp->b_flags;
	cmd_bp->b_options = bp->b_options;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = blkno;
	cmd_bp->b_un.b_addr = bp->b_un.b_addr;
	bcopy((caddr_t) &(bp->b_xmemd), (caddr_t)
	      &(cmd_bp->b_xmemd), sizeof(struct xmem));
	/*
	 * If the request is to or from an odd-byte aligned buffer, we
	 * must use PIO commands.  This is because the IDE DMA controller
	 * cannot handle DMA requests to or from buffers that are not
	 * word-aligned.
	 */
	if ((uint) bp->b_un.b_addr & 1) {
		/* Buffer is odd-byte aligned */
		read_command = ATA_READ_SECTOR_RETRY;
		write_command = ATA_WRITE_SECTOR_RETRY;
	} else {
		/* Buffer is word-aligned */
		read_command = diskinfo->read_cmd;
		write_command = diskinfo->write_cmd;
	}
	cmd_bp->b_iodone = ((void(*)())idedisk_iodone);
	cmd_ptr->idebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->error_type = 0;
	cmd_ptr->soft_resid = 0;

	/* use start-up timeout if device is not spun-up */
	if ((diskinfo->pmh.handle.mode != PM_DEVICE_FULL_ON) &&
		(diskinfo->pmh.handle.mode != PM_DEVICE_ENABLE)) {

		diskinfo->pmh.handle.mode = PM_DEVICE_ENABLE;
		cmd_ptr->idebuf.timeout_value = diskinfo->start_timeout;
	} else
		cmd_ptr->idebuf.timeout_value = diskinfo->rw_timeout;

	if (bp->b_resid != 0) {
		/*
		 * Request extends beyond end of media
		 */
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
	idedisk_trc(buildcmd, trc, (char) 1, (uint) diskinfo->state, (uint) bp,
		(uint) bp->b_flags, (uint) bp->b_resid, (uint) bp->b_options);
#endif	
#endif
		cmd_bp->b_bcount = bp->b_bcount - bp->b_resid;
		cmd_ptr->group_type = DK_SINGLE;
		cmd_ptr->idebuf.bp = NULL;
		cmd_ptr->bp = bp;
	} else {

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
	idedisk_trc(buildcmd, trc, (char) 2, (uint) diskinfo->state, (uint) bp,
		(uint) bp->b_flags, (uint) bp->b_resid, (uint) bp->b_options);
#endif	
#endif
		total_count = 0x00;
		tmp_bp = bp;
		while (tmp_bp != NULL) {
			total_count += tmp_bp->b_bcount;
			tmp_bp = tmp_bp->av_forw;
		}

		if (bp->av_forw != NULL) {
			cmd_ptr->group_type = DK_COALESCED;
			cmd_ptr->idebuf.bp = bp;
			cmd_ptr->bp = NULL;
		} else {
			cmd_ptr->group_type = DK_SINGLE;
			cmd_ptr->idebuf.bp = NULL;
			cmd_ptr->bp = bp;
		}
		cmd_bp->b_bcount = total_count;
	}

	/* Initialize IDE cmd for operation */
	blk_cnt = cmd_bp->b_bcount / diskinfo->block_size;
	ata = &(cmd_ptr->idebuf.ata);
	ata->sector_cnt_cmd = blk_cnt & 0xFF;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;

	/*
	 * Determine start address using the appropriate format (CHS/LBA)
	 */
	CONVERT_IF_CHS(ata,blkno);

	if (bp->b_flags & B_READ) {
		ata->command = read_command;
	} else {
		ata->command = write_command;

		if ((bp->b_options & WRITEV) &&
			(diskinfo->verify_flags & VERIFY_SUPPORTED)) {
			/*
			 * Write-Verify has been requested.  Change the cmd
			 * type to reflect the desire for Read-Verify upon
			 * successful completion of the Write command.  Also,
			 * copy the target address information into the verify
			 * command.  Lastly, save the cmd pointer to the write
			 * in diskinfo so that it may be easily retrieved upon
			 * successful completion of the Read-Verify command.
			 */
			cmd_ptr->type = DK_WRITEV;

			verify_ata = &(diskinfo->verify_cmd.idebuf.ata);
			verify_ata->flags = ata->flags;
			verify_ata->startblk = ata->startblk;
			verify_ata->sector_cnt_cmd = ata->sector_cnt_cmd;

			diskinfo->verify_cmd.soft_resid = 0;
			diskinfo->verify_cmd.retry_count = 0;
			diskinfo->verify_cmd.queued = 0;
			diskinfo->verify_cmd.aborted = 0;
			diskinfo->verify_cmd.error_type = 0;
			diskinfo->writev_cmd = cmd_ptr;
		}
	}

	/*
	 * Clear in-progress queue since we just processed it
	 */

	*head  = NULL;

	return(cmd_ptr);
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_start						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver Start IO Routine.		*/
/*		This routine will try to queue one command		*/
/*		to the specified device.  It scans the command queue 	*/
/*		first.  This queue contains all error recovery commands	*/
/*		including normal reads/writes to be retried.		*/
/*									*/
/*		Next the IN_PROGRESS queue is checked to start		*/
/*		any ops that have already been coalesced. Finally,	*/
/*		the pending queue is checked, and if it is not		*/
/*		empty, idedisk_coalesce is called to coalesce		*/
/*		ops into the IN_PROGRESS queue and			*/
/*		idedisk_build_cmd is called to construct a IDE op	*/
/*		to satisfy the requests in the IN_PROGRESS queue.	*/
/*		When a request has been found and built, the adapter	*/
/*		is called via devstrategy to begin processing of the	*/
/*		operation.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called at both a process.		*/
/*		and an interrupt level.					*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo - Address of disk information structure	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devstrat						*/
/*									*/
/************************************************************************/

void
idedisk_start(
	struct idediskinfo	*diskinfo)
{
	struct dk_cmd		*cmd_ptr; 	/* command block pointer     */
	int			opri;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(start, entry, (char) 0, (uint) diskinfo, (uint) diskinfo->state,
	      (uint) diskinfo->dk_cmd_q_head, (uint) 0,
	      (uint) diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

        /*
	 * If certain diskinfo->state's exist, we will only issue one command
	 * at a time or starve certain requests until this condition is cleared
	 */

	while (TRUE) {

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(start, trc, (char) 0, (uint) diskinfo->cmds_out, 
	      (uint) diskinfo->state, (uint) 0,
	      (uint) diskinfo->dk_cmd_q_head,
	      (uint) diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

		if ((diskinfo->state & DK_DUMP_PENDING) ||
		    (diskinfo->cmds_out >= 1)) {

			DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_BSTART, 0,
				diskinfo->devno, cmd_ptr,
				diskinfo->state, diskinfo->cmds_out, 0);
			/*
			 * If we have dump pending
			 * or if our timer is set do not issue 
			 * any more commands until this completes.  Also if we 
			 * have reached our queue depth do not issue any thing 
			 * else.
			 */
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
		}
		diskinfo->dkstat.dk_status |= IOST_DK_BUSY;

		if ((diskinfo->dk_cmd_q_head != NULL)) {
			/* 
			 * Start cmd struct on top of command queue.  This
			 * queue is for error recovery commands.
			 */
			cmd_ptr = diskinfo->dk_cmd_q_head;
			idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

		} else if (diskinfo->pm_pending & PM_IDEDISK_PENDING_SUSPEND) {
			/*
			 * If we are trying to go into a PM_DEVICE_SUSPEND
			 * or PM_DEVICE_HIBERNATION mode then we don't want
			 * to issue any non-error recovery commands.
			 */
			DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno, 0, 0, 0, 0);

			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
	       
		} else if (diskinfo->dk_bp_queue.in_progress.head != NULL) {
			/* 
			 * If a request was coalesced but there were no 
			 * resources available at the time then we will arrive
			 * here.  We must issue this request(s) before
			 * we call coalesce again.
			 */

			cmd_ptr = idedisk_build_cmd(diskinfo,
				    (struct buf **) 
				    &(diskinfo->dk_bp_queue.in_progress.head),
				    (char) TRUE);

		} else if (diskinfo->currbuf != NULL) {
			/* Coalesce into IN PROGRESS queue and start */

			idedisk_coalesce(diskinfo);

			cmd_ptr = idedisk_build_cmd(diskinfo,
				    (struct buf **)
		                    &(diskinfo->dk_bp_queue.in_progress.head),
				    (char) TRUE);

		} else {
			DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno, cmd_ptr, 0, 0, 0);

			if (diskinfo->cmds_out == 0) {
				/* 
				 * If no more commands outstanding clear 
				 * busy status for this disk
				 */
				diskinfo->dkstat.dk_status &= ~IOST_DK_BUSY;
			}

			/*
			 * Nothing on any of our queues so exit this loop
			 */
			DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
		}

		DDHKWD5(HKWD_DD_IDEDISKDD, DD_ENTRY_BSTART, 0, 
			diskinfo->devno, &cmd_ptr->idebuf,
			cmd_ptr->idebuf.bufstruct.b_blkno,
			cmd_ptr->idebuf.bufstruct.b_bcount,
			cmd_ptr->idebuf.bufstruct.b_flags);

		/*
		 * Issue the command block selected above
		 */
		if (cmd_ptr == NULL) {
			/*
			 * No resources for for DK_BUF commands available
			 * so exit this loop
			 */

			break;
		}	
	
		cmd_ptr->idebuf.bufstruct.b_flags &= ~(B_ERROR | B_DONE);
		cmd_ptr->idebuf.bufstruct.b_error = 0x00;
		cmd_ptr->status |= DK_ACTIVE;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(issue, trc, (char) 0, (uint) cmd_ptr, 
	      (uint) cmd_ptr->type, (uint) cmd_ptr->subtype,
	      (uint) cmd_ptr->status,
	      (uint) cmd_ptr->retry_count);
#endif
#endif
		cmd_ptr->retry_count++;
		diskinfo->cmds_out++;

		DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_BSTART, 0, 
			diskinfo->devno);

		/* Power Management activity flag */
		diskinfo->pmh.handle.activity = PM_ACTIVITY_OCCURRED;

		devstrat(cmd_ptr);
	}  
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(start, exit, (char) 0, (uint) diskinfo, (uint) diskinfo->state,
	      (uint) diskinfo->dk_cmd_q_head, (uint) diskinfo->cmds_out,
	      (uint) diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_iodone						*/
/*									*/
/*	FUNCTION: IDE Disk Device Driver I/O Completion Routine.	*/
/*		This routine determines if the indicated operation	*/
/*		completed successfully or failed. If successful,	*/
/*		the appropriate routine is called to process the	*/
/*		specific type of cmd. If failed, the general failure	*/
/*		processing routine is called. Upon exiting this		*/
/*		routine, idedisk_start is called to begin processing    */
/*		of the cmd at the top of the device's cmd stack.      	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of idebuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns a zero value.			*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/************************************************************************/

void
idedisk_iodone(
	struct	dk_cmd	*cmd_ptr)
{
	struct idediskinfo	*diskinfo;
	struct buf		*bp, *cmd_bp;
	struct ata_cmd		*ata;
	uint			old_pri;
	uint			abs_track;	/* absolute track of request */
	ushort			cylinder;	/* cylinder of request */
	uint			blkno;

	/* Get diskinfo struct */
	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;
	bp = &(cmd_ptr->idebuf.bufstruct);
	DDHKWD3(HKWD_DD_IDEDISKDD, DD_ENTRY_IODONE, 0, diskinfo->devno,
		bp->b_resid, bp->b_error);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
  idedisk_trc(interrupt, entry, (char) 0, (uint) diskinfo,
		(uint) diskinfo->state, (uint) diskinfo->cmds_out,
		(uint) cmd_ptr, (uint) cmd_ptr->type);
#endif
#endif
	/* 
	 * decrement cmds_out  (i.e. the number of outstanding commands)
	 */

	diskinfo->cmds_out--;
	cmd_ptr->status &= ~DK_ACTIVE;

	/* Test for Successful Completion of operation */
	if ((bp->b_flags & B_ERROR) || (bp->b_resid != 0)) {
		/*
		 * Operation FAILED:
                 * Call idedisk_process_error to handle error
                 * logging and determine what type of error
                 * recovery must be done.
                 */
		idedisk_process_error(cmd_ptr);
	} else {
		/*
		 * This command completed successfully
		 * process it as such.
		 */
		idedisk_process_good(cmd_ptr);
	}

	if (!(diskinfo->cmds_out)) {
		/* 
		 * If no more commands outstanding clear busy status for this 
		 * disk
		 */
		diskinfo->dkstat.dk_status &= ~IOST_DK_BUSY;

                /* Do pending PM operation */
                if (diskinfo->pm_pending & PM_IDEDISK_PENDING_OP) {
			/*
			 * Our PM handler is waiting on I/O to complete.
			 * So let's wake it up.
			 * NOTE: This will cause the PM handler
			 * to return PM_SUCCESS, since we're
			 * clearing the PM_IDEDISK_PENDING_OP flag
			 * here.
			 */
			diskinfo->pm_pending &= ~PM_IDEDISK_PENDING_OP;
			e_wakeup((int *)&(diskinfo->pm_event));
                }
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(interrupt, trc, (char) 0, (uint) diskinfo, (uint) diskinfo->state,
	      (uint) diskinfo->cmds_out, (uint) 0, (uint) 0);
#endif
#endif
	}
	/* 
	 * Start all the pending operations that we can.
	 */

	idedisk_start(diskinfo);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
  idedisk_trc(interrupt, exit, (char) 0, (uint) diskinfo,
		(uint) diskinfo->state, (uint) diskinfo->cmds_out,
		(uint) 0, (uint) 0);
#endif
#endif
	DDHKWD1(HKWD_DD_IDEDISKDD, DD_EXIT_IODONE, 0, diskinfo->devno);

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_good					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes successful completion of		*/
/*		a IDE request.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of idebuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/************************************************************************/

void
idedisk_process_good(
	struct dk_cmd		*cmd_ptr)
{
	struct idediskinfo	*diskinfo;
	struct buf		*curbp;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(good, entry, (char) 0, (uint) cmd_ptr->diskinfo, 
		(uint) cmd_ptr->diskinfo->state,
		(uint) cmd_ptr->diskinfo->cmds_out,
		(uint) cmd_ptr,(uint)cmd_ptr->type);
#endif
#endif
	if (cmd_ptr->status & DK_QUEUED) {
		/*
		 * Commands with good completion are immediately dequeued 
		 * for processing
		 */
		idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
	}

	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;
	switch (cmd_ptr->type) {
	case DK_BUF:
		idedisk_process_buf(cmd_ptr);
		break;
	case DK_WRITEV:
		/*
		 * The Write command of a Write-Verify request has completed.
		 * Now issue the ATA-2 Read-Verify command to verify the
		 * data is not corrupted.
		 */
		idedisk_q_cmd(	&(diskinfo->verify_cmd),
				(char) DK_STACK, (uchar) DK_CMD_Q );
		break;
	case DK_VERIFY:
		/*
		 * See if a recoverable error was encountered during the
		 * verify.  If so, modify the appropriate fields in the
		 * associated write command's buf structure.
		 */
		if ((cmd_ptr->idebuf.status_validity & ATA_IDE_STATUS) &&
			(cmd_ptr->idebuf.ata.status & ATA_CORRECTED_DATA)) {

			curbp = &((diskinfo->writev_cmd)->idebuf.bufstruct);
			curbp->b_flags |= B_ERROR;
			curbp->b_error = ESOFT;
			curbp->b_resid = cmd_ptr->soft_resid;
		}

		/*
		 * The Read-Verify command succeeded.  Finish processing
		 * the write command that has been verified.
		 */
		idedisk_process_buf(diskinfo->writev_cmd);
		diskinfo->writev_cmd = NULL;
		break;
	case DK_PM:
		/*
		 * "Free" the reserved PM command
		 */
		cmd_ptr->status = DK_FREE;
		e_wakeup( (int *) &(diskinfo->pm_event) );
		break;
	default:
		/* Process unknown cmd type */
		ASSERT(FALSE);
	};
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(good, exit, (char) 0, (uint) diskinfo, (uint) diskinfo->state,
	      (uint) diskinfo->dk_cmd_q_head, (uint) 0, (uint) 0);
#endif
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine places the failed command on the cmd_Q	*/
/*		so that it may be retried.  The command's ataide_buf's	*/
/*		status_validity field is examined to determine whether	*/
/*		the failure was a partial transfer due to a lack of DMA */
/*		resources.  If so, the appropriate routine will be	*/
/*		called to retry the portion of the request which was	*/
/*		not transferred.  Such a "failure" is not logged nor	*/
/*		penalized incrementation of the command's retry count	*/
/*		if any portion of the request is successfully		*/
/*		transferred.  Any other type of failure is logged and	*/
/*		retried.						*/
/*									*/
/*		When this routine exits, the device's cmd stack		*/
/*		should either be empty, or contain an operation		*/
/*		to be started.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of idebuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		printf							*/
/*									*/
/************************************************************************/

void
idedisk_process_error(
	struct dk_cmd		*cmd_ptr)
{
	struct ataide_buf	*idebuf;

	/* Set up misc pointers for structures */
	idebuf = &(cmd_ptr->idebuf);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(error, entry, (char) 0, (uint) cmd_ptr->diskinfo->state, 
	      (uint) cmd_ptr, (uint) cmd_ptr->type, (uint) cmd_ptr->subtype,
	      (uint) idebuf->status_validity);
#endif	
#endif

	cmd_ptr->status |= DK_RETRY;

	/*
	 * Since this command had an error, let's put it on the head of the
	 * cmd_Q for possible retries.
	 */
	idedisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	/*
	 * If part of the transfer succeeded and the cause of the failure
	 * was an insufficient number of DMA resources, handle it as a 
	 * partial success rather than a normal failure.
	 */
	if ((idebuf->status_validity & ATA_IDE_DMA_NORES) &&
		(idebuf->bufstruct.b_resid != idebuf->bufstruct.b_bcount)) {

		idedisk_process_partial(cmd_ptr);
	} else {
#ifdef DEBUG
		printf("<<ERROR>> cmd=%2x dev=%2x blkno=%4x bcount=%4x\n",
			cmd_ptr->idebuf.ata.command,
			cmd_ptr->idebuf.ata.device,
			cmd_ptr->idebuf.bufstruct.b_blkno,
                        cmd_ptr->idebuf.bufstruct.b_bcount);
#endif
		idedisk_process_dev_error(cmd_ptr);
	}

	/*
         * Please note that when this routine exits, whatever
         * is on the top of the device's cmd queue will be started.
         * Falling through this routine will cause a retry of the
         * operation that failed.
         */

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(error, exit, (char) 0, (uint) 0, (uint) 0 ,(uint) 0,
		(uint) 0, (uint) 0);
#endif	
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_dev_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes IDE requests that have failed	*/
/*		due to an error on the device. This routine will log	*/
/*		an error in the system error log, and attempt		*/
/*		to reschedule the operation for a retry.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of dk_cmd struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

void
idedisk_process_dev_error(
	struct dk_cmd		*cmd_ptr)
{
	struct ataide_buf	*idebuf;
	struct buf		*bp;
	struct idediskinfo	*diskinfo;
	int			max_retries;
	uchar			stack = FALSE;

	/* Set up misc pointers for structures */
	idebuf = &(cmd_ptr->idebuf);
	bp = &(idebuf->bufstruct);
	diskinfo = cmd_ptr->diskinfo;

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(ideerror, entry, (char) 0, (uint) cmd_ptr->type, 
		(uint) idebuf->ata.status, (uint) idebuf->ata.errval,
		(uint) 0, (uint) 0);
#endif	
#endif	
	/*
	 * Save relevant error record information for logging later
	 */

	idedisk_build_error ( cmd_ptr, DK_NOOVERLAY_ERROR );

	/*
         * Now we handoff to the cmd type specific error processor
         * for preparing the cmd for a retry attempt or aborting
         * the cmd back to the requestor.
         */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(ideerror, trc, (char) 0, (uint) bp, (uint) bp->b_error,
	      (uint) bp->b_resid, (uint) cmd_ptr->type, (uint) diskinfo->state);
#endif	
#endif
	switch (cmd_ptr->type) {
	case DK_BUF:
	case DK_WRITEV:
		idedisk_process_buf_error(cmd_ptr);
		break;
	case DK_VERIFY:
		idedisk_process_verify_error(cmd_ptr);
		break;
	case DK_PM:
		if (cmd_ptr->retry_count >= DK_MAX_RETRY) {
			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idedisk_log_error(cmd_ptr, DK_HARD_ERROR);
			}
			idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

			/*
			 * "Free" the reserved PM command
			 */
			cmd_ptr->status = DK_FREE;
			diskinfo->errno = bp->b_error;
			e_wakeup( (int *) &(diskinfo->pm_event) );
		}
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(ideerror, exit, (char) 1, (uint) diskinfo->state, (uint) 0,
	      (uint) 0, (uint) 0, (uint) 0);
#endif	
#endif	
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_buf_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine does error processing specifically for	*/
/*		requests that are related to a Buf structure. The	*/
/*		failure is evaluated, and if a retry is feasible	*/
/*		the cmd is updated to reflect the current state of	*/
/*		the request, ie. the residual, and reissued to the	*/
/*		controller.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*                                                                      */
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of dk_cmd struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		iodone							*/
/*									*/
/************************************************************************/

void
idedisk_process_buf_error(
	struct dk_cmd		*cmd_ptr)
{
	struct ataide_buf	*idebuf;
	struct idediskinfo	*diskinfo;
	struct buf		*bp;
	uint			remaining_count;

	/* Set up misc pointers for structures */
	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;

	idebuf = &(cmd_ptr->idebuf);
	bp = &(idebuf->bufstruct);

#ifdef DEBUG
	printf("\nERROR\n");
	printf("\terror = %x\n",idebuf->bufstruct.b_error);
	printf("\tstatus_validity = %x\n",idebuf->status_validity);
	printf("\tATA .status reg = %x\n",idebuf->ata.status);
	printf("\tATA .error reg = %x\n",idebuf->ata.errval);
#endif

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(failbuf, entry, (char) 0, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint) bp->b_error, 
	      (uint) bp->b_resid, (uint) bp->b_bcount);
#endif	
#endif	
	/* Check if cmd is single operation */
	if (cmd_ptr->group_type & DK_SINGLE) {

		/* Operation is a Single operation */
		if (cmd_ptr->retry_count >= DK_MAX_RETRY) {

			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idedisk_log_error(cmd_ptr, DK_HARD_ERROR);
			}

			/* Remove op from device's cmd stack */
			idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

			/* Clear write cmd to verify pointer */
			diskinfo->writev_cmd = NULL;

			/* flush request from IN_PROGRESS queue */

			if ((cmd_ptr->error_type == 0) && (bp->b_resid == 0)) {
				/* 
				 * If this was something that we did not log 
				 * and which completely transferred then
				 * make sure that b_error = 0
				 */
				cmd_ptr->bp->b_error = 0;
				cmd_ptr->bp->b_flags &= ~B_ERROR;
			} else {
				cmd_ptr->bp->b_flags |= B_ERROR;
				cmd_ptr->bp->b_error = bp->b_error;
				cmd_ptr->bp->b_resid += bp->b_resid;
			}
			DDHKWD2(HKWD_DD_IDEDISKDD, DD_SC_IODONE,
			    cmd_ptr->bp->b_error, cmd_ptr->bp->b_dev,
			    cmd_ptr->bp);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(idiodone, trc, (char) 2, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint)bp->b_error, 
	      (uint) bp->b_resid, (uint) bp->b_bcount);
#endif	
#endif	
			iodone(cmd_ptr->bp);
			
			idedisk_free_cmd(cmd_ptr);
		}

	} else if (cmd_ptr->group_type & DK_COALESCED) {
		/* Operation is a Coalesced operation */
		struct buf      *bp_list;
		struct buf      *first_bad_bp;
		struct buf      *last_good_bp;
		struct buf      *current_bp;
		struct buf      *prev_bp;
		int             total_count;
		int             good_count;
		int             residual;
		int             current_retry_count;
		uchar		current_error_type;

		/*
		 * we need to try to determine the extent
		 * of the failure, indicating which parts
		 * of the coalesced operation were successful
		 * and which parts need to be retried
		 */

		/* Set up a pointer to the list of requests */
		bp_list = idebuf->bp;

		/* Determine how much of operation failed */
		residual = bp->b_resid;

		/*
		 * Determine if any part of the operation
		 * was successful, and process completion
		 * of any that were.
		 */
		first_bad_bp = NULL;
		last_good_bp = NULL;
		current_bp = bp_list;
		total_count = bp->b_bcount;
		good_count = 0;


		/*
		 *
		 *
		 *  Example of a failed transfer with 3 bufs.
		 *
		 *  B              B              
		 *  |-------|--------|--------|  = Buf addr boundaries
		 *  ^-------------------------^  = total count
		 *  ^-------^                    = good count
		 *              ^-------------^  = residual
		 *  |XXXXXXX|                    = Blocks returned as good
		 *          ^                    = first_bad_bp
		 *  ^                            = last_good_bp
		 *
		 */

		while ((total_count - (good_count + current_bp->b_bcount)) >
		    residual) {
			good_count += current_bp->b_bcount;
			last_good_bp = current_bp;
			current_bp = current_bp->av_forw;
		}
		first_bad_bp = current_bp;

		/*
		 * At this point, first_bad_bp points at the
		 * list of bp's that need to be retried, and
		 * any successful bp's are in a list starting
		 * with idebuf->bp and ending with last_good_bp.
		 */

		/* Check for any completed requests and process */
		if (last_good_bp != NULL) {
			current_bp = bp_list;
			do {
				/* Return to system as complete */
				current_bp->b_resid = 0;
				current_bp->b_flags &= ~B_ERROR;
				current_bp->b_error = 0;
				DDHKWD2(HKWD_DD_IDEDISKDD, DD_SC_IODONE,
				    current_bp->b_error, current_bp->b_dev,
				    current_bp);
				prev_bp = current_bp;
				current_bp = current_bp->av_forw;
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(idiodone, trc, (char) 3, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint) bp->b_error, 
	      (uint) bp->b_resid, (uint) bp->b_bcount);
#endif	
#endif	
				iodone(prev_bp);
			
			} while (prev_bp != last_good_bp);
		}

		/*
		 * At this point the bp list has been split into two lists.
		 * we must set idebuf->bp to point at the start of the
		 * list to be retried.
		 */
		idebuf->bp = first_bad_bp;
		/*
		 * Save off some current values for the
		 * new cmd_ptr that we will create.
		 */
		current_error_type = cmd_ptr->error_type;
		current_retry_count = cmd_ptr->retry_count;
		if (current_retry_count < DK_MAX_RETRY) {
			/*
			 * we now need to update the
			 * cmd block to reflect the
			 * remaining list for the retry
			 */
			idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			idedisk_free_cmd(cmd_ptr);
			cmd_ptr = idedisk_build_cmd(diskinfo,
					(struct buf **) &first_bad_bp,
					(char) FALSE);
			/*
			 * No resources free so exit this loop
			 */
			ASSERT(cmd_ptr != NULL);

			cmd_ptr->retry_count = current_retry_count;
			cmd_ptr->error_type = current_error_type;
			idedisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
			/*
			 * Falling out here will allow retry of
			 * updated operation.
			 */

		} else { /* max retries exceeded */

			/* setup to determine proper residual count */
			total_count = 0;
			current_bp = idebuf->bp;
			while (current_bp != NULL) {
				total_count += current_bp->b_bcount;
				current_bp = current_bp->av_forw;
			}

			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idedisk_log_error(cmd_ptr, DK_HARD_ERROR);
			}

			/* Fail first bp in the chain of bp's  */
			/* and then retry the remaining bp's   */

			first_bad_bp =  first_bad_bp->av_forw;
			/* Set error values and flag bit */

			/*
			 *  Example (cont) of a failed transfer with 3 bufs.
			 *
			 *  B              B              
			 *  |-------|--------|--------|  = Buf addr boundaries
			 *          ^-----------------^  = total_count
			 *  ^-------^                    = good count
			 *              ^-------------^  = residual
			 *  |XXXXXXX|                    = Blocks returned good
			 *          ^                    = idebuf->bp
			 *          ^--------^           = idebuf->bp->b_bcount
			 *              ^----^           = idebuf->bp->b_resid
			 *                   ^           = first_bad_bp
			 */

			idebuf->bp->b_flags |= B_ERROR;
			idebuf->bp->b_error = idebuf->bufstruct.b_error;
			idebuf->bp->b_resid = (idebuf->bp->b_bcount - 
						(total_count - residual));
			if (idebuf->bp->b_resid == 0) {
				/*
				 * If no resid then we will set it to
				 * the entire transfer, since we know
				 * this buf has failed.
				 */
				idebuf->bp->b_resid = idebuf->bp->b_bcount;
			}

			DDHKWD2(HKWD_DD_IDEDISKDD, DD_SC_IODONE,
			    idebuf->bp->b_error, idebuf->bp->b_dev,
			    idebuf->bp);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(idiodone, trc, (char) 4, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint) idebuf->bp->b_error, 
	      (uint) idebuf->bp->b_resid, (uint) idebuf->bp->b_bcount);
#endif	
#endif	
			iodone(idebuf->bp);
			
			/* Clear write cmd to verify pointer */
			diskinfo->writev_cmd = NULL;

			idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			idedisk_free_cmd(cmd_ptr);
			/* Check for IN_PROGRESS queue not empty */
			if (first_bad_bp != NULL) {
				/* Build cmd for remaining queue */
				cmd_ptr = idedisk_build_cmd(diskinfo,
					    (struct buf **) &first_bad_bp,
					    (char) FALSE);
				ASSERT(cmd_ptr != NULL);

				idedisk_q_cmd(cmd_ptr,(char) DK_STACK,
					     (uchar) DK_CMD_Q);
			}
		}
	}

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idedisk_trc(failbuf, exit, (char) 0, (uint) diskinfo->state, 
	      (uint) 0, (uint) 0, (uint) 0, (uint) 0);
#endif	
#endif	
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_verify_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine does error processing specifically for	*/
/*		Read-Verify requests.  These are special commands	*/
/*		issued soley to verify the data written by previous	*/
/*		Write commands contains no uncorrectable data errors.	*/
/*									*/
/*		The error register's contents (errval) are examined to	*/
/*		determine whether the failure was due to the device	*/
/*		aborting the command.  If it is, the Read-Verify	*/
/*		command is retried up to DK_MAX_VERIFY_RETRY times.	*/
/*		If the Read-Verify continues to be aborted by the disk, */
/*		it must be malfunctioning or not ATA-2 compliant (ie -	*/
/*		it does not support the Read-Verify command).		*/
/*		An error is logged and all subsequent Write-Verify	*/
/*		requests are honored WITHOUT VERIFICATION.		*/
/*									*/
/*		If the failure is not due to a command abort, the	*/
/*		associated Write command's retry counter is tested	*/
/*		against DK_MAX_RETRY.  If another retry is allowed,	*/
/*		the Write command is reissued.  Throughout this process,*/
/*		the Write command's retry counter is never reset.	*/
/*		Thus, the maximum number of times it will be attempted	*/
/*		is always DK_MAX_RETRY.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*                                                                      */
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of dk_cmd struct for the verify cmd	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/************************************************************************/

void
idedisk_process_verify_error(
	struct dk_cmd		*cmd_ptr)
{
	struct idediskinfo	*diskinfo;
	struct dk_cmd		*write_cmd;
	struct buf		*write_cmd_buf, *verify_cmd_buf;
	struct ata_cmd		*write_ata;

	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;

	/* Set up misc pointers for structures */
	if ((cmd_ptr->idebuf.ata.status & ATA_ERROR) && 
	    (cmd_ptr->idebuf.ata.errval & ATA_ABORTED_CMD) ) {

		/*
		 * Increment consecutive Read-Verify aborted count.
		 */
		diskinfo->verify_flags++;

		if ((diskinfo->verify_flags & ~VERIFY_SUPPORTED)
				>= DK_MAX_VERIFY_RETRY) {
			/*
			 * Dequeue the Verify cmd to prevent another retry.
			 */
			idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idedisk_log_error(cmd_ptr, DK_HARD_ERROR);
			}
			idedisk_process_buf(diskinfo->writev_cmd);
			diskinfo->writev_cmd = NULL;
			diskinfo->verify_flags &= ~VERIFY_SUPPORTED;
		}

	} else {
		/*
		 * Clear consecutive Verify-aborted count.
		 */
		diskinfo->verify_flags &= VERIFY_SUPPORTED;

		/*
		 * Dequeue the Verify cmd to prevent another retry.
		 */
		idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

		/*
		 * Enqueue the Write cmd
		 * 	For < MAX_RETRY:  This allows the write/verify cycle
		 *			  to be retried.
		 *	For >= MAX_RETRY: This lets idedisk_process_buf_error
		 *			  pull-off and process the right cmd.
		 */
		write_cmd = diskinfo->writev_cmd;
		idedisk_q_cmd(write_cmd, (char) DK_STACK,
				(uchar) DK_CMD_Q );

		if (write_cmd->retry_count >= DK_MAX_RETRY) {

			/*
			 * Assign the write command's b_error to that of the
			 * verify command and b_resid equal to the # of sectors
			 * which had not been verified (from the sector count
			 * register).
			 */
			write_cmd->idebuf.bufstruct.b_error =
				cmd_ptr->idebuf.bufstruct.b_error;
			write_cmd->idebuf.bufstruct.b_resid =
				(cmd_ptr->idebuf.ata.sector_cnt_ret *
				 diskinfo->block_size);
			/*
			 * Ensure that idedisk_process_buf_error logs the
			 * last error encountered
			 */
			write_cmd->error_type = cmd_ptr->error_type;

			idedisk_process_buf_error(write_cmd);
		}
	}
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_partial					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine does error processing specifically for	*/
/*		requests that did not completely transfer due to	*/
/*		a deficiency of DMA resources.  This condition is	*/
/*		temporary.  The portion of the request which was not	*/
/*		transferred is retried without penalty to the retry	*/
/*		counter.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*                                                                      */
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of dk_cmd struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/************************************************************************/

void
idedisk_process_partial(
	struct dk_cmd		*cmd_ptr)
{
	struct idediskinfo	*diskinfo;
	struct buf		*bp;
	int			good_cnt;
	long			blkno;
	uint			abs_track;
	ushort			cylinder;

	/* Set up misc pointers for structures */
	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;

	bp = &(cmd_ptr->idebuf.bufstruct);

#ifdef DEBUG
	printf("\nPARTIAL\n");
	printf("\tstatus_validity = 0x%x\n",cmd_ptr->idebuf.status_validity);
	printf("\ttarget address = 0x%x\n",bp->b_un.b_addr);
	printf("\tb_bcount = 0x%x\n",bp->b_bcount);
	printf("\tresidual = 0x%x\n",bp->b_resid);
#ifdef IDE_ERROR_PATH
  idedisk_trc(partial, entry, (char) 0, (uint) cmd_ptr->group_type, 
	      (uint) bp->b_resid, (uint) bp->b_un.b_addr,
	      (uint) bp->b_blkno, (uint) bp->b_bcount);
#endif	
#endif	
	/* Check if cmd is single operation */
	if (cmd_ptr->group_type & DK_SINGLE) {

		/*
		 * Do not count these as error retries
		 */
		cmd_ptr->retry_count--;

		/*
		 * Only request what has not been successfully transferred
		 */
		good_cnt = (bp->b_bcount - bp->b_resid);
		blkno = bp->b_blkno + good_cnt/diskinfo->block_size;
		bp->b_blkno = blkno;
		bp->b_un.b_addr += good_cnt;
		bp->b_bcount = bp->b_resid;
		bp->b_resid = 0;
		cmd_ptr->idebuf.ata.sector_cnt_cmd =
			bp->b_bcount/diskinfo->block_size;
		CONVERT_IF_CHS((&cmd_ptr->idebuf.ata), blkno );
	} else {
		ASSERT(TRUE);
	}

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
  idedisk_trc(partial, exit, (char) 0, (uint) cmd_ptr->group_type, 
	      (uint) bp->b_resid, (uint) bp->b_un.b_addr,
	      (uint) bp->b_blkno, (uint) bp->b_bcount);
#endif	
#endif	
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_process_buf					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine performs completion processing on		*/
/*		Buf related requests that have been received by		*/
/*		idedisk_iodone with good return status.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of idebuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		iodone							*/
/*									*/
/************************************************************************/

void
idedisk_process_buf(
	struct dk_cmd		*cmd_ptr)
{
	struct idediskinfo	*diskinfo;
	struct ataide_buf	*idebuf;
	struct buf		*curbp, *nextbp;
	uint			total_count;

	/* Get diskinfo struct for device */
	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;

	/* Set up misc pointers for structs */
	idebuf = &(cmd_ptr->idebuf);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(processbuf, entry, (char) 0, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint) 0, (uint) 0, (uint) 0);
#endif	
#endif	
	/* Check for a recovered error during processing, log any */
	if (cmd_ptr->error_type != 0) {
		idedisk_log_error(cmd_ptr, DK_SOFT_ERROR);
	}

	if (cmd_ptr->group_type == DK_SINGLE) {
		/* Cmd is single operation */
		curbp = cmd_ptr->bp;

		/*
 		 * Check the soft_resid variable to see if this command was
 		 * recovered with retries and needs to be relocated by
 		 * reporting an ESOFT.
		 * 
		 * The CORR bit in the status register is not examined
		 * to determine if a correctable error has occurred
		 * during the transfer, because that bit is not valid
		 * for DMA transfers (all "officially" supported disks
		 * support DMA).  Even for PIO disks, the status
		 * register returned from the adapter driver only
		 * reflects the status for the last sector transfered.
		 * If a recovered error is encountered accessing any
		 * sector other than the last in a single request, it
		 * cannot be detected.
 		 */
		if (cmd_ptr->soft_resid) {
			curbp->b_flags |= B_ERROR;
			curbp->b_error = ESOFT;
			curbp->b_resid = cmd_ptr->soft_resid;
		} else {
			curbp->b_flags &= ~B_ERROR;
			curbp->b_error = 0;
		}

		DDHKWD2(HKWD_DD_IDEDISKDD, DD_SC_IODONE, curbp->b_error,
		    curbp->b_dev, curbp);
		if (curbp->b_flags & B_READ) {
			diskinfo->dkstat.dk_rblks += (curbp->b_bcount) / 
			    diskinfo->block_size;
		} else {
			diskinfo->dkstat.dk_wblks += (curbp->b_bcount) / 
			    diskinfo->block_size;
		}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(idiodone, trc, (char) 0, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint) curbp->b_error, 
	      (uint) curbp->b_resid, (uint) curbp->b_bcount);
#endif	
#endif	
		iodone(curbp);

		idedisk_free_cmd(cmd_ptr);

	} else if (cmd_ptr->group_type == DK_COALESCED) {
		/*
		 * Cmd is coalesced operation
		 * Flush IN_PROGRESS queue
		 */
		curbp = idebuf->bp;
		total_count = idebuf->bufstruct.b_bcount;
		while (curbp != NULL) {
			total_count -= curbp->b_bcount;
			/*
			 *  Check the soft_resid variable to see if this is
			 *  a recovered media error that should be relocated
			 *  by reporting ESOFT.
			 * 
			 * The CORR bit in the status register is not examined
			 * to determine if a correctable error has occurred
			 * during the transfer, because that bit is not valid
			 * for DMA transfers (all "officially" supported disks
			 * support DMA).  Even for PIO disks, the status
			 * register returned from the adapter driver only
			 * reflects the status for the last sector transfered.
			 * If a recovered error is encountered accessing any
			 * sector other than the last in a single request, it
			 * cannot be detected.
			 */
			if (cmd_ptr->soft_resid > total_count) {
				curbp->b_flags |= B_ERROR;
				curbp->b_error = ESOFT;
				curbp->b_resid = cmd_ptr->soft_resid - total_count;
				cmd_ptr->soft_resid = 0;
			} else {
				curbp->b_flags &= ~B_ERROR;
				curbp->b_error = 0;
				curbp->b_resid = 0;
			}

			nextbp = curbp->av_forw;
			DDHKWD2(HKWD_DD_IDEDISKDD, DD_SC_IODONE,
				curbp->b_error, curbp->b_dev, curbp);
			if (curbp->b_flags & B_READ) {
				diskinfo->dkstat.dk_rblks +=
				    (curbp->b_bcount) / diskinfo->block_size;
			} else {
				diskinfo->dkstat.dk_wblks +=
				    (curbp->b_bcount) / diskinfo->block_size;
			}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(idiodone, trc, (char) 1, (uint) cmd_ptr->error_type, 
	      (uint) cmd_ptr->group_type, (uint) curbp->b_error, 
	      (uint) curbp->b_resid, (uint) curbp->b_bcount);
#endif	
#endif
			iodone(curbp);

			curbp = nextbp;
		}

		idedisk_free_cmd(cmd_ptr);
	} 
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(processbuf, exit, (char) 0, (uint) cmd_ptr->error_type, 
	      (uint) 0, (uint) 0, (uint) 0, (uint) 0);
#endif	
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_coalesce					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine scans the PENDING queue for the		*/
/*		specified device. The goal is to minimize the number of	*/
/*		IDE operations that must be performed to satisfy	*/
/*		the requests in the PENDING queue. Requests will	*/
/*		grouped by one of the following rules:			*/
/*									*/
/*		1 - Contiguous write operations				*/
/*		2 - Operations requiring special processing		*/
/*									*/
/*		The coalesced requests will be removed from the		*/
/*		PENDING queue and placed in the IN_PROGRESS queue	*/
/*		so that a single IDE command may be built to		*/
/*		satisfy the requests.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
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
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/************************************************************************/

void
idedisk_coalesce(
	struct idediskinfo	*diskinfo)
{
	struct buf		*bp;
	int			total_count, next_block, rwflag;
	int			rc;
	int			blkno;

	total_count = 0;
	next_block = 0;
	rwflag = 0;

	/* Start coalescing with current buf of pending queue*/
	bp = diskinfo->currbuf;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(coales, entry, (char) 0, (uint) diskinfo, (uint) diskinfo->state,
	      (uint) bp, (uint) bp->b_bcount, (uint) bp->b_options);
#endif	
#endif
	/* Check for retried commands */
	if (bp->b_resid != 0) {
		/* Remove bp from PENDING queue */
		idedisk_pending_dequeue(diskinfo);

		/* Place bp IN_PROGRESS queue */

		IDEDISK_IN_PROG_ENQUEUE(diskinfo, bp);
		return;
	}

	/* Remove bp from PENDING queue */
	idedisk_pending_dequeue(diskinfo);

	/* Place bp IN_PROGRESS queue */

	IDEDISK_IN_PROG_ENQUEUE(diskinfo, bp);

	if ((bp->b_bcount <= diskinfo->max_coalesce) &&
	    (((int)bp->b_un.b_addr & 0x0fff) == 0) &&
	    ((bp->b_bcount & 0x0fff) == 0) &&
	    (bp->b_resid == 0)) {

		 IDEDISK_GET_LBA(blkno,diskinfo,bp,rc); 
		 if (rc) {
			  /*
			   * Request does not start on device
			   * block boundary.  
                           * This should be filtered at 
			   * idedisk_strategy.
                           */
		   	   ASSERT(rc == 0);
		}
		next_block = blkno + (bp->b_bcount /diskinfo->block_size);
		total_count += bp->b_bcount;
		rwflag = (bp->b_flags & B_READ);

		/* Check next buf in list for continued coalescing */
		bp = diskinfo->currbuf;
		if (bp != NULL) {
			IDEDISK_GET_LBA(blkno,diskinfo,bp,rc); 
			if (rc) {
				/*
				 * Request does not start on device
				 * block boundary.  
				 * This should be filtered at 
				 * idedisk_strategy.
				 */
				ASSERT(rc == 0);
			}
		}

		/* Begin coalescing operations */
		while ((bp != NULL) &&
		       (rwflag == (bp->b_flags & B_READ)) &&
		       (blkno == next_block) &&
		       ((total_count + bp->b_bcount) <= 
			diskinfo->max_coalesce) &&
		       (((int)bp->b_un.b_addr & 0x0fff) == 0) &&
		       ((bp->b_bcount & 0x0fff) == 0) &&
		       (bp->b_resid == 0)) {
	
                        /*
			 * Conditions to be able to coalesce:
                         * 1) there is a next one to look at
                         * 2) the next request is sequential with current one
                         * 3) the next one is the same direction transfer 
			 *    as this one (i.e. rwflag is read, bp is read) 
                         * 4) the next one begins on a page boundary
			 * 5) the next one will end on a page boundary
			 * 6) the next one won't exceed coalesce maximum
                         */

			/* Remove bp from PENDING queue */
			idedisk_pending_dequeue(diskinfo);
	
			/* Place bp IN_PROGRESS queue */

			IDEDISK_IN_PROG_ENQUEUE(diskinfo, bp);

			next_block = blkno + 
			    (bp->b_bcount / diskinfo->block_size);
			total_count += bp->b_bcount;

			/* Check next buf in list for continued coalescing */
			bp = diskinfo->currbuf;
			if (bp != NULL) {

				IDEDISK_GET_LBA(blkno,diskinfo,bp,rc);
				if (rc) {
					/*
					 * Request does not start on device
					 * block boundary.  
					 * This should be filtered at 
					 * idedisk_strategy.
					 */
					ASSERT(rc == 0);
				}
			}
		}
	}

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(coales, exit, (char) 0, (uint) diskinfo, (uint) diskinfo->state,
		(uint) diskinfo->dk_bp_queue.in_progress.head,
		(uint) 0, (uint) 0);
#endif	
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_build_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds an error record for a failed	*/
/*		operation to be placed in the system error log.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of idebuf struct describing operation	*/
/*		priority - Indicates whether an existing entry may be	*/
/*			   overwritten					*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bcopy							*/
/*									*/
/************************************************************************/

void
idedisk_build_error(
	struct dk_cmd	*cmd_ptr,
	int		priority)
{
	struct buf			*bp;
	struct ataide_buf		*idebuf;
	struct idedisk_error_rec	*erec;

	if ((cmd_ptr->error_type == 0) || (priority == DK_OVERLAY_ERROR)) {

		/* Setup pointers for faster access */
		idebuf = &(cmd_ptr->idebuf);
		bp = &(idebuf->bufstruct);
		erec = &(cmd_ptr->diskinfo->error_rec);

		/* Store type for use by logger */

		if (cmd_ptr->idebuf.status_validity & ATA_ERROR_VALID) {

			if ( !(cmd_ptr->idebuf.ata.errval &
					(ATA_MEDIA_CHANGE_REQ |
					 ATA_MEDIA_CHANGED |
					 ATA_ABORTED_CMD))){

				cmd_ptr->error_type = DK_MEDIA_ERR;
				cmd_ptr->soft_resid = cmd_ptr->bp->b_bcount -
						      cmd_ptr->segment_count -
						      bp->b_bcount +
						      bp->b_resid;

			} else if
				(cmd_ptr->idebuf.ata.errval & ATA_ABORTED_CMD) {

				cmd_ptr->error_type = DK_SOFTWARE_ERR;

			} else {

				cmd_ptr->error_type = DK_HARDWARE_ERR;
			}
		} else {
			/*
			 * This is an error detected by the adapter before
			 * the command was issued.  Use status_validity to
			 * determine the cause.
			 */
			if (cmd_ptr->idebuf.status_validity &
				(ATA_CMD_TIMEOUT | ATA_NO_DEVICE_RESPONSE)) {

				cmd_ptr->error_type = DK_HARDWARE_ERR;
			} else {
				cmd_ptr->error_type = DK_SOFTWARE_ERR;
			}
		}

		bcopy(&(idebuf->ata), &(erec->ata), sizeof(struct ata_cmd));
		erec->status_validity = idebuf->status_validity;
		erec->b_error = bp->b_error;
		erec->rsvd0 = 0;
		erec->b_flags = bp->b_flags;
		erec->b_addr = bp->b_un.b_addr;
		erec->b_resid = bp->b_resid;
	}
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_log_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine places a previously recorded error record	*/
/*		in the system error log.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of idebuf struct describing operation	*/
/*		sev	- Specifies HARD or SOFT error			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		errsave							*/
/*									*/
/************************************************************************/

void
idedisk_log_error(
	struct dk_cmd	*cmd_ptr,
	int		sev)
{
	struct idedisk_error_rec	*erec;
	struct idediskinfo		*diskinfo;

	/* Setup pointers for faster access */
	diskinfo = (struct idediskinfo *) cmd_ptr->diskinfo;
	erec = &(cmd_ptr->diskinfo->error_rec);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idedisk_trc(logerr, trc, (char) 0, (uint) diskinfo,
		(uint) cmd_ptr->error_type, (uint) cmd_ptr,
		(uint) cmd_ptr->status, (uint) sev);
#endif	
#endif
	if (sev == DK_HARD_ERROR) {
		switch(cmd_ptr->error_type) {
		case DK_MEDIA_ERR:
			erec->entry.error_id = ERRID_IDE_DISK_ERR1;
			break;
		case DK_SOFTWARE_ERR:
			erec->entry.error_id = ERRID_IDE_DISK_ERR2;
			break;
		case DK_HARDWARE_ERR:
			erec->entry.error_id = ERRID_IDE_DISK_ERR3;
			break;
		default:
			break;
		}
	} else {
		erec->entry.error_id = ERRID_IDE_DISK_ERR4;
	}

	cmd_ptr->error_type = 0;
	errsave(erec, sizeof(struct idedisk_error_rec));
}

/************************************************************************/
/*									*/
/*	NAME:	idedisk_cdt_func					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is called at dump time.  The component   	*/
/*		dump table has been allocated at config time, so this	*/
/*		routine fills in the table and returns the address.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		arg - 1 indicates the start of a dump			*/
/*		      2 indicates the end of a dump			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the address of the component 	*/
/*		dump table.						*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bcopy		printf					*/
/*									*/
/************************************************************************/
struct cdt *
idedisk_cdt_func(
	int	arg)
{
	struct idediskinfo	*diskinfo;
	struct cdt_entry	*entry_ptr;
	int			entry_count, i;

#ifdef DEBUG
	printf("Entering idedisk_cdt_func arg[0x%x]\n", arg);
#endif
	if (arg == 1) {
		/*  First dump entry is global info structure */
		entry_ptr = &idedd_info.cdt->cdt_entry[0];
		bcopy("dd_info", entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct idedd_info);
		entry_ptr->d_ptr = (caddr_t) &idedd_info;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Second dump entry is configured disk list */
		entry_ptr = &idedd_info.cdt->cdt_entry[1];
		bcopy("cfglist", entry_ptr->d_name, 8);
		entry_ptr->d_len = 4 * DK_MAXCONFIG;
		entry_ptr->d_ptr = (caddr_t) idedisk_list;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Third dump entry is opened disk list */
		entry_ptr = &idedd_info.cdt->cdt_entry[2];
		bcopy("opnlist", entry_ptr->d_name, 8);
		entry_ptr->d_len = 4 * DK_MAXCONFIG;
		entry_ptr->d_ptr = (caddr_t) idedisk_open_list;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Rest of entries are disk structures */
		entry_count = 2;
		for (i = 0 ; i < DK_MAXCONFIG ; i++) {
			diskinfo = idedisk_list[i];
			if (diskinfo != NULL) {
				entry_ptr = &idedd_info.cdt->cdt_entry[
				    ++entry_count];
				bcopy(diskinfo->error_rec.entry.resource_name,
				    entry_ptr->d_name, 8);
				entry_ptr->d_len = sizeof(struct idediskinfo);
				entry_ptr->d_ptr = (caddr_t) diskinfo;
				entry_ptr->d_segval = (int) NULL;
			}
		}
	}

#ifdef DEBUG
	printf("Exiting idedisk_cdt_func\n");
#endif
	return(idedd_info.cdt);
}

/************************************************************************
 *
 *	NAME:	idedisk_q_cmd
 *
 *	FUNCTION: Places a command block on the specified disk's queue
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *	      interrupt levels, and can not page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		dk_cmd		- Command structure
 *		diskinfo	- disk info structure
 *
 *	INPUTS:
 *		cmd_ptr		pointer to command to be queued
 *		queue		specifies QUEUE or STACK request
 *		which_q		indicates the target queue
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		None.
 *
 ************************************************************************/

void
idedisk_q_cmd(
	struct dk_cmd	*cmd_ptr,
	char		queue,
	uchar		which_q)
{
	struct idediskinfo	*diskinfo = NULL;	/* disk device info */
							/* structure */
	struct dk_cmd		**head_ptr,
				**tail_ptr;
	uchar			head_is_qrecov = FALSE;

        ASSERT(cmd_ptr);

        diskinfo = cmd_ptr->diskinfo;             /* set disk pointer */

        cmd_ptr->next = NULL;			/* set next to null, this */
						/* will be put on end */
	cmd_ptr->prev = NULL;    

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(qcmd, entry, (char) 0, (uint) diskinfo, (uint) queue,
	(uint) which_q, (uint) cmd_ptr->status, (uint) cmd_ptr->type);
#endif	
#endif
	if (which_q == DK_CMD_Q) {
		head_ptr = &(diskinfo->dk_cmd_q_head);
		tail_ptr = &(diskinfo->dk_cmd_q_tail);
		cmd_ptr->status |= DK_QUEUED;
	}
	else {
		ASSERT(which_q == DK_CMD_Q);
	}

        if (queue == DK_QUEUE) {
                /*
                 * Queue this command
                 */
                if (*head_ptr == NULL) {
                        /*
                         * if q is empty, set
                         * head to this one
                         */
                        *head_ptr = cmd_ptr;
		} else {
                        /*
                         * else q is not empty, so
                         * put on tail of q
                         */
			cmd_ptr->prev = *tail_ptr;
			if (*tail_ptr != NULL)
				(*tail_ptr)->next = cmd_ptr;
		}
                /*
                 * set new tail to this one
                 */
                *tail_ptr = cmd_ptr;
        } else {
                /*
                 * else Stack this command
                 */
                if (*head_ptr == NULL) {
                        /*
                         * if q is empty, set
                         * tail to this one also
                         */

                        *tail_ptr = cmd_ptr;
		} else {
                        /*
                         * else not empty, stack this command
                         * at the head
                         */
			cmd_ptr->next = *head_ptr;
			(*head_ptr)->prev = cmd_ptr;
		}
                /*
                 * set new head to this one
                 */
		*head_ptr = cmd_ptr;
        }
	/*
	 * Change Commands Status as Queued
	 */
	cmd_ptr->queued = TRUE;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(qcmd, exit, (char) 0, (uint) cmd_ptr, (uint) cmd_ptr->diskinfo,
	      (uint) which_q, (uint) cmd_ptr->prev, (uint) cmd_ptr->next);
#endif	
#endif
	return;
}

/************************************************************************ 
 *
 *	NAME:	idedisk_d_q_cmd
 *
 *	FUNCTION:
 *		Removes the specified command from the specified disk's queue
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on the interrupt level,
 *		and can not page fault. Plus from close on the process level.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		dk_cmd		- Command structure
 *		diskinfo	- disk info structure
 *
 *	INPUTS:
 *		cmd_ptr		pointer to command to be queued
 *		which_q		indicates where to queue cmd_ptr
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		None.
 *
 ************************************************************************/

void
idedisk_d_q_cmd(
	struct dk_cmd	*cmd_ptr,
	uchar		which_q)
{
	struct idediskinfo	*diskinfo = NULL;	/* disk device info */
							/* structure */
	struct dk_cmd		**head_ptr,
				**tail_ptr;

       	ASSERT(cmd_ptr);

        if (!(cmd_ptr->queued))
		return;
        /*
         * set head pointer to prev initially. this will catch an unknown
         * cmd type when compiled without DEBUG
         */

        /*
         * Unlink this command from its list
         */
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(dqcmd, entry, (char) 0, (uint) cmd_ptr, (uint) cmd_ptr->diskinfo,
	      (uint) which_q, (uint) cmd_ptr->status, (uint) cmd_ptr->type);
#endif	
#endif

        diskinfo = cmd_ptr->diskinfo;             /* set disk pointer */

	if (which_q == DK_CMD_Q) {
		head_ptr = &(diskinfo->dk_cmd_q_head);
		tail_ptr = &(diskinfo->dk_cmd_q_tail);
		cmd_ptr->status &= ~DK_QUEUED;
	} else {
		ASSERT(which_q == DK_CMD_Q);
	}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(dqcmd, trc, (char) 0, (uint) cmd_ptr, (uint) *head_ptr,
	      (uint) which_q, (uint) cmd_ptr->prev, (uint) cmd_ptr->next);
#endif	
#endif
	if (cmd_ptr->prev != NULL)
		(cmd_ptr->prev)->next = cmd_ptr->next;

	if (cmd_ptr->next != NULL)
		(cmd_ptr->next)->prev = cmd_ptr->prev;

	if (*head_ptr == cmd_ptr)
		*head_ptr = cmd_ptr->next;

	if (*tail_ptr == cmd_ptr)
		*tail_ptr = cmd_ptr->prev;

	cmd_ptr->prev = NULL;
	cmd_ptr->next = NULL;

	/*
	 * Change Commands Status as UnQueued
	 */
	cmd_ptr->queued = FALSE;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(dqcmd, exit, (char) 0, (uint) cmd_ptr, (uint) cmd_ptr->diskinfo,
	      (uint) *head_ptr, (uint) cmd_ptr->prev, (uint) cmd_ptr->next);
#endif	
#endif
        return;
}

/************************************************************************
 *
 *	NAME:	idedisk_cmd_alloc_disable
 *
 *	FUNCTION:
 *		Allocates a command block if available for the routines that
 *		have not disabled and spin locked.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *		interrupt levels, and can not page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		dk_cmd		- Command structure
 *		diskinfo	- disk info structure
 *
 *	INPUTS:
 *		diskinfo	pointer to disk info structure
 *		cmd_type	indicates command type
 *
 *	RETURN VALUE DESCRIPTION:
 *		Pointer to allocated cmd	- successful completion
 *		NULL				- no free structures
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		i_disable		i_enable
 *
 ************************************************************************/
struct	dk_cmd *
idedisk_cmd_alloc_disable(
	struct idediskinfo	*diskinfo,
	uchar			cmd_type)
{
	uint		old_pri;		/* Old interrupt priority*/
	struct dk_cmd	*cmd_ptr;

        /*
         * disable interrupts
         */
	old_pri = i_disable(INTTIMER);

	cmd_ptr = idedisk_cmd_alloc(diskinfo,cmd_type);

        /*
         * enable interrupts
         */
	i_enable(old_pri);

	return(cmd_ptr);
}
/************************************************************************
 *
 *	NAME:	idedisk_cmd_alloc
 *
 *	FUNCTION:
 *		Allocates a command block if available
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *		interrupt levels, and can not page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		dk_cmd		- Command structure
 *		diskinfo	- disk info structure
 *
 *	INPUTS:
 *		diskinfo	pointer to disk info structure
 *		cmd_type	indicates command type
 *
 *	RETURN VALUE DESCRIPTION:
 *		Pointer to allocated cmd	- successful completion
 *		NULL				- no free structures
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		None.
 *
 ************************************************************************/
struct	dk_cmd *
idedisk_cmd_alloc(
	struct idediskinfo	*diskinfo,
	uchar			cmd_type)
{
	struct	dk_cmd	*pool;		/* Pointer to pool of command blocks */
	struct	dk_cmd	*cmd_ptr;
	struct	buf     *cmd_bp;
	struct	ata_cmd	*ata;

	cmd_ptr = NULL;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(cmdalloc, trc, (char) 0, (uint) diskinfo, (uint) cmd_type,
	      (uint) diskinfo->pool_index, (uint) 0, (uint) 0);
#endif	
#endif
	if (cmd_type == DK_BUF) {
		pool = &diskinfo->cmd_pool[0];

		if (!(pool[diskinfo->pool_index].status)) {
			/*
			 * If not in use then pick it
			 */
			pool[diskinfo->pool_index].status = DK_IN_USE;
			pool[diskinfo->pool_index].type = cmd_type;
			pool[diskinfo->pool_index].aborted = FALSE;
			pool[diskinfo->pool_index].subtype = 0x00;
			pool[diskinfo->pool_index].next = NULL;
			pool[diskinfo->pool_index].prev = NULL;
			cmd_ptr = &pool[diskinfo->pool_index];
		} else {
			cmd_ptr = NULL;
		}

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(cmdalloc, trc, (char) 1, (uint) diskinfo, 
	      (uint) 0, (uint) diskinfo->pool_index, 
	      (uint) cmd_ptr, (uint) 0);
#endif	
#endif
	}

	if (cmd_ptr != NULL) {
		/*
		 * Set device ID for cmd_ptr
		 */
		cmd_ptr->idebuf.ata.device = diskinfo->device_id;
	}

	return(cmd_ptr);
}
/************************************************************************
 *
 *	NAME:	idedisk_free_cmd_disable
 *
 *	FUNCTION:
 *		Frees a Command structure for the routines that
 *		have not disabled and spin locked.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *		interrupt levels, and can not page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		dk_cmd		- Command structure
 *
 *	INPUTS:
 *		cmd_ptr		pointer to command to be freed
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		i_disable		i_enable
 *
 ************************************************************************/
void
idedisk_free_cmd_disable(
	struct dk_cmd	*cmd_ptr)
{
        uint		old_pri;

	if (cmd_ptr == NULL) {
		ASSERT(cmd_ptr);
		return;
	}
        /*
         * disable interrupts
         */
	old_pri = i_disable(INTTIMER);

	idedisk_free_cmd(cmd_ptr);

        /*
         * restore interrupts
         */
	i_enable(old_pri);
	return;
}  
/************************************************************************
 *
 *	NAME:	idedisk_free_cmd
 *
 *	FUNCTION:
 *		Frees a Command structure.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *		interrupt levels, and cannot page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		idediskinfo	- disk info structure
 *		dk_cmd		- Command structure
 *
 *	INPUTS:
 *		cmd_ptr		pointer to command to be freed
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		bzero
 *
 ************************************************************************/
void
idedisk_free_cmd(
	struct dk_cmd		*cmd_ptr)
{
	struct idediskinfo	*diskinfo;

        if (cmd_ptr == NULL)
                /*
                 * if cmd is NULL, assume that it came from the cmd map and
                 * is already free
                 */
                return;
        /*
         * get diskinfo structure
         */

	diskinfo = cmd_ptr->diskinfo;

        ASSERT(cmd_ptr->status);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(cmdfree, trc, (char) 0, (uint) diskinfo, (uint) 0,
		(uint) cmd_ptr->status, (uint) cmd_ptr->type, (uint) 0);
#endif	
#endif
 
        bzero(cmd_ptr, sizeof(struct dk_cmd)); /* clear entire command struc */
        cmd_ptr->diskinfo = diskinfo;          /* restore diskinfo pointer   */
	cmd_ptr->idebuf.bufstruct.b_event = EVENT_NULL;
	return;
}  /* end idedisk_free_cmd */


/************************************************************************
 *
 *	NAME:	idedisk_sleep
 *
 *	FUNCTION: Used to sleep on an event
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called in the process environment.  It
 *		can not page fault and is pinned
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		None.
 *
 *	INPUTS:
 *		intrpt		pointer to flag
 *		event		pointer to event upon which to wait
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		e_sleep			i_disable
 *		i_enable
 *
 ************************************************************************/

void
idedisk_sleep(
	uchar	*intrpt,
	uint	*event)
{
        int    old_pri;   /* old interrupt level */

	old_pri = i_disable(INTTIMER);
        if (*intrpt)
                /*
                 * if flag still set, go to sleep
                 */

		 e_sleep((int *)event,EVENT_SHORT);
	/* 
	 * reenable interrupts and unlock
	 */

	i_enable(old_pri);

        return;
}

#ifdef DEBUG
/************************************************************************
 *
 *	NAME:	idedisk_trc_disable
 *
 *	FUNCTION:
 *		Builds an internal Trace Entry for Debug for the
 *		routines that have not disabled and spin locked. 
 *	     
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *		interrupt levels, and can not page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		idedisk_trace	IDE disk trace table
 *
 *	INPUTS:
 *		desc		points to string identifying trace source
 *		type		points to string describing entry type
 *		count		number of values to store in entry
 *		word1-word5	uint-length values to store in trace entry
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		i_disable		i_enable
 *
 ************************************************************************/

void
idedisk_trc_disable(
	char	*desc,
	char	*type,
	char	count,
	uint	word1,
	uint	word2,
	uint	word3,
	uint	word4,
	uint	word5)
{
        uint    old_pri;

	old_pri = i_disable(INTTIMER);

	idedisk_trc(desc,type,count,word1,word2,word3,word4,word5);

	i_enable(old_pri);

	return;
}
/************************************************************************
 *
 *	NAME:	idedisk_trc
 *
 *	FUNCTION:
 *		Builds an internal Trace Entry for Debug.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on both the process and
 *		interrupt levels, and can not page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		idedisk_trace	IDE disk trace table
 *
 *	INPUTS:
 *		desc		points to string identifying trace source
 *		type		points to string describing entry type
 *		count		number of values to store in entry
 *		word1-word5	uint-length values to store in trace entry
 *
 *	RETURN VALUE DESCRIPTION:
 *		This routine returns no value.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		bcopy
 *
 ************************************************************************/

void
idedisk_trc(
	char	*desc,
	char	*type,
	char	count,
	uint	word1,
	uint	word2,
	uint	word3,
	uint	word4,
	uint	word5)
{
        uint    old_pri;

	bcopy(desc,idedisk_trace[idedisk_trcindex].desc,IDEDISK_TRC_STR_LEN);
        bcopy(type,idedisk_trace[idedisk_trcindex].type, 3);
        idedisk_trace[idedisk_trcindex].count = count;
        idedisk_trace[idedisk_trcindex].word1 = word1;
        idedisk_trace[idedisk_trcindex].word2 = word2;
        idedisk_trace[idedisk_trcindex].word3 = word3;
        idedisk_trace[idedisk_trcindex].word4 = word4;
        idedisk_trace[idedisk_trcindex].word5 = word5;
        if (++idedisk_trcindex >= TRCLNGTH)
                idedisk_trcindex = 0;
        bcopy(topoftrc,idedisk_trace[idedisk_trcindex].desc, 
			IDEDISK_TRC_STR_LEN);
        idedisk_trctop = (struct idedisk_trace *)
			&idedisk_trace[idedisk_trcindex];
	return;
}

#endif

/************************************************************************/
/*									*/
/*	NAME:	idedisk_spinupdn					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds either an IDLE IMMEDIATE, STANDBY	*/
/*		IMMEDIATE, or SLEEP command for the specified		*/
/*		device.  The cmd block is constructed, placed on the	*/
/*		top of the device's cmd stack and idedisk_start is	*/
/*		called to initiate execution of the cmd.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on both interrupt level and      */
/*		process level.  It cannot page fault.			*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*		req_cmd		Specifies which ATA-2 power management	*/
/*				command has been requested		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/************************************************************************/

void
idedisk_spinupdn(
	struct idediskinfo	*diskinfo,
	uchar			req_cmd)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;

	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->pm_cmd);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(spinupdn, entry, (char) 0, (uint) diskinfo,
		(uint) diskinfo->state, (uint) diskinfo->dk_cmd_q_head,
		(uint) cmd_ptr, (uint) cmd_ptr->status);
#endif	
#endif

        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */
                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        idedisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
	cmd_ptr->retry_count = 0;
	cmd_ptr->idebuf.bufstruct.b_flags &= ~(B_ERROR | B_DONE);
	cmd_ptr->idebuf.bufstruct.b_error = 0;
	cmd_ptr->idebuf.status_validity = 0;
	cmd_ptr->soft_resid = 0;
	cmd_ptr->aborted = FALSE;
	cmd_ptr->queued = FALSE;
	cmd_ptr->error_type = 0;
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    

	ata = &(cmd_ptr->idebuf.ata);
	ata->status = 0;
	ata->errval = 0;

	switch (req_cmd) {
		case DK_IDLE:
			ata->command = ATA_IDLE_IMMEDIATE;
			break;
		case DK_STANDBY:
			ata->command = ATA_STANDBY_IMMEDIATE;
			break;
		case DK_SLEEP:
			ata->command = ATA_SLEEP;
			break;
		default:
			ASSERT(req_cmd);
			return;
	}

	cmd_ptr->subtype = req_cmd;

	/* Place cmd on device's operation stack */

	idedisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	/* Begin execution of this operation */

	idedisk_start(diskinfo);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idedisk_trc(spinupdn, exit, (char) 0, (uint) diskinfo,
		(uint) diskinfo->state, (uint) diskinfo->dk_cmd_q_head,
		(uint) cmd_ptr, (uint) cmd_ptr->status);
#endif	
#endif
	return;
}

/************************************************************************
 *
 *	NAME:	idedisk_pm_watchdog
 *
 *	FUNCTION:
 *		This routine is used so our PM handler will not
 * 		sleep too long when waiting for a device to go
 *		idle.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called on an interrupt level.
 *		It cannot page fault.
 *
 *	NOTES:
 *
 *	DATA STRUCTURES:
 *		diskinfo	Disk device specific information
 *
 *	INPUTS:
 *		watchdog_timer - pointer to watchdog structure at the
 *				 top of the diskinfo structure.
 *
 *	RETURN VALUE DESCRIPTION:
 *		None.
 *
 *	ERROR DESCRIPTION:
 *		None.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		None.
 *
 ************************************************************************/
void idedisk_pm_watchdog(
	struct watchdog		*watchdog_timer)
{
	struct idedisk_timer	*timer;
	struct idediskinfo	*diskinfo;
	uint			old_pri;

	timer = (struct idedisk_timer *) (watchdog_timer);
	diskinfo = (struct idediskinfo *) (timer->pointer);

	old_pri = i_disable(INTTIMER);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idedisk_trc(pmh_timer, trc, (char) 0,
		(uint) diskinfo, 
		(uint) diskinfo->state,
		(uint) diskinfo->dk_cmd_q_head,
		(uint) 0,
		(uint) diskinfo->timer_status);
#endif	
#endif
	/*
	 * Our PM handler is waiting on I/O to complete.
	 * So let's wake it up.  
	 * NOTE: The PM handler will return PM_ERROR
	 * in this case, since we're not clearing the
	 * IDEDISK_PM_PENDING_OP flag here.
	 */
	e_wakeup( (int *) &(diskinfo->pm_event) );

	i_enable(old_pri);

	return;
}

/************************************************************************
 *
 *	NAME:	idedisk_spindown_closeddisk
 *
 *	FUNCTION:
 *		Issue IDE STANDBY IMMEDIATE command to devices
 *		that are not opened.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called in the process environment.  It
 *		can not page fault and is pinned.
 *
 *	NOTES:
 *		This routine assumes the caller is not i_disabled
 *		and has done any necessary lockls.
 *
 *	DATA STRUCTURES:
 *		struct idediskinfo	disk's information
 *
 *	INPUTS:
 *		diskinfo		Disk device specific information
 *
 *	RETURN VALUE DESCRIPTION:
 *		The return values listed in the 'error description'
 *		will be returned to the caller if there is an error.
 *		Otherwise a value of zero will be returned to indicate
 *		successful completion.
 *
 *	ERROR DESCRIPTION:
 *		EINVAL	- Indicates that the major portion of the devno
 *			  parameter exceeds the maximum number allowed, or
 *			  the devflags parameter is not valid
 *		ENODEV	- Indicates that the device does not exist.
 *		EINTR	- Indicates that the signal was caught while processing
 *			  the fp_opendev request.
 *		ENFILE	- Indicates that the system file table is full.
 *		ENXIO	- Indicates that the device is multiplexed and unable
 *			  to allocate the channel.
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		fp_opendev		fp_ioctl
 *
 ************************************************************************/

int
idedisk_spindown_closeddisk(
	struct idediskinfo	*diskinfo)
{
	int	errnoval = 0;
	int	opri;

	/* 
	 * Open adapter for device and store filepointer
	 */
	errnoval = fp_opendev(diskinfo->adapter_devno, (int) FWRITE,
			      (caddr_t) NULL, (int) NULL, &(diskinfo->fp));
	
	if (errnoval == 0) {
		/* 
		 * Issue a start IOCTL to the 
		 * adapter driver 
		 */
		
		errnoval = fp_ioctl(diskinfo->fp, (uint) IDEIOSTART,
				    (caddr_t) diskinfo->device_id, (int) NULL);

		if (errnoval == 0) {

			diskinfo->errno = 0;
			
			opri = i_disable(INTTIMER);

			idedisk_spinupdn(diskinfo, (uchar) DK_STANDBY);

			/*
			 * We will sleep here. idedisk_process_good
			 * or idedisk_process_error will wake us when
			 * the command has completed.
			 */
			e_sleep( (int *) &(diskinfo->pm_event), EVENT_SHORT );

			i_enable(opri);

			errnoval = diskinfo->errno;
			
			fp_ioctl(diskinfo->fp, (uint) IDEIOSTOP,
				 (caddr_t) diskinfo->device_id, (int) NULL);
		}
		fp_close(diskinfo->fp);
	}
	
	return (errnoval);
}

/************************************************************************
 *
 *	NAME:	idedisk_pm_handler
 *
 *	FUNCTION:
 *		IDE disk Power Management function.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called in the process environment.  It
 *		can not page fault and is pinned
 *
 *	NOTES:
 *		The device must be opened in order to issue any
 *		ATA commands to the device.  If it is not open then
 *		all ATA commands will be failed by the adapter driver,
 *		since it requires an IDEIOSTART ioctl (which is issued by
 *		this driver at open time).
 *
 *	DATA STRUCTURES:
 *		diskinfo	Disk device specific information
 *
 *	INPUTS:
 *		private		Disk device specific information
 *		mode		Specifies the requested PM mode
 *
 *	RETURN VALUE DESCRIPTION:
 *		The return values listed in the 'error description'
 *		will be returned to the caller if there is an error.
 *		Otherwise PM_SUCCESS will be returned to indicate
 *		successful completion.
 *
 *	ERROR DESCRIPTION:
 *		The following errno values may be returned by the function:
 *			PM_ERROR	- Unsupported mode
 *
 *	EXTERNAL PROCEDURES CALLED:
 *		i_disable		i_enable
 *
 ************************************************************************/

int
idedisk_pm_handler(
	caddr_t	private,
	int	mode)
{
	struct idedisk_pm_handle *pmh = (struct idedisk_pm_handle *) private;
	struct idediskinfo	 *diskinfo;
	int			 opri;
	int			 errnoval = 0;
	int			 rc = 0;

	/*
	 * Validate information received from PM core
	 */
	if (pmh == NULL) {
		ASSERT(FALSE);
		return (PM_ERROR);
	}

	diskinfo = pmh->diskinfo;

	if (diskinfo == NULL) {
		ASSERT(FALSE);
		return (PM_ERROR);
	}

	switch(mode){
	case PM_DEVICE_FULL_ON:
		diskinfo->pmh.handle.mode = PM_DEVICE_FULL_ON;
		break;

	case PM_DEVICE_ENABLE:
		/*
		 * Pin the bottom half of the driver, since
		 * we may not be open.
		 */
		errnoval = pincode((int(*)())idedisk_iodone);
		if (errnoval) {
			return PM_ERROR;
		}

		opri = i_disable(INTTIMER);

		/*
		 * Allow any halted I/O due to suspend or hibernate
		 * to resume.
		 */

		diskinfo->pm_pending &= ~PM_IDEDISK_PENDING_SUSPEND;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idedisk_trc(pmh_trc, trc, (char) 1, 
				(uint) diskinfo,
				(uint) diskinfo->cmds_out,
				(uint) pmh,
				(uint) pmh->handle.mode,
				(uint) mode);
#endif	
#endif
		if ((pmh->handle.mode == PM_DEVICE_SUSPEND) ||
		    (pmh->handle.mode == PM_DEVICE_HIBERNATION)) {
			/*
			 * This is to prevent the PM core from
			 * prematurely issuing an PM_DEVICE_IDLE
			 */
			pmh->handle.activity =  PM_ACTIVITY_OCCURRED;
		}
		if ((diskinfo->opened) && (!diskinfo->cmds_out)) {
			/*
			 * If this device is open and
			 * there are no outstanding commands,
			 * issue an ATA IDLE_IMMEDIATE command to
			 * insure the disk is spinning.
			 */
			idedisk_spinupdn(diskinfo, (uchar) DK_IDLE);
		}
		pmh->handle.mode = PM_DEVICE_ENABLE;
		i_enable(opri);

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode( (int(*)()) idedisk_iodone);
		break;

	case PM_DEVICE_IDLE:
		/*
		 * Pin the bottom half of the driver, since
		 * we may not be open.
		 */
		errnoval = pincode( (int(*)()) idedisk_iodone);
		if (errnoval) {
			return PM_ERROR;
		}

		opri = i_disable(INTTIMER);

		/*
		 * Allow any halted I/O due to suspend or hibernate
		 * to resume.
		 */

		diskinfo->pm_pending &= ~PM_IDEDISK_PENDING_SUSPEND;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idedisk_trc(pmh_trc, trc, (char) 2, 
				(uint) diskinfo,
				(uint) diskinfo->cmds_out,
				(uint) pmh,
				(uint) pmh->handle.mode,
				(uint) mode);
#endif	
#endif
		if (!diskinfo->cmds_out) {
			if (diskinfo->opened) {
				/*
				 * If this device is open and
				 * there are no outstanding commands
				 * then issue the ATA STANDBY_IMMEDIATE
				 * command.
				 */
				idedisk_spinupdn(diskinfo, (uchar) DK_STANDBY);
			} else {
				/*
				 * If the device is not open
				 * then we need to do some special
				 * stuff to spin it down. First we
				 * must i_enable, because fp_ioctl
				 * and fp_opendev can't be called while
				 * i_disabled.  However since this device
				 * is not open, we need to guard against
				 * a possible open/unconfigure while 
				 * i_enabled.  We will try to take the
				 * driver's global lock using LOCK_NDELAY.
				 * If some one has this lock (i.e some
				 * one is attempting an open/unconfigure), 
				 * it will fail immediately and will will exit
				 * this handler with PM_ERROR.
				 *
				 * NOTE: A LOCK_SHORT would work for the IDE
				 * disk DD here, but a check for a change in
				 * the disk's open status would then be
				 * necessary after the lockl call.  To minimize
				 * differences between this driver and the
				 * SCSI disk driver, and to ease future
				 * conversion of this driver to MP safe or
				 * MP efficient status, LOCK_NDELAY is used.
				 */

				rc = lockl(&(idedd_info.lock), LOCK_NDELAY);

				i_enable(opri);
				if (rc == LOCK_FAIL) {
					/*
					 * Some one else has the device
					 * lock (i.e. someone is opening
					 * or unconfiguring this device.
					 * So fail the IDLE.
					 */
					/*
					 * Unpin the bottom half of the driver
					 */
					(void *) unpincode((int(*)())
							   idedisk_iodone);
			
					return (PM_ERROR);
				}

				/*
				 * If we get here then, our lockl
				 * was successful. So we are now
				 * guarded against open/unconfigure
				 */
				(void) idedisk_spindown_closeddisk(diskinfo);

				opri = i_disable(INTTIMER);

				unlockl(&(idedd_info.lock));
			}
			pmh->handle.mode = PM_DEVICE_IDLE;
		} else {

			/*
			 * There is an outstanding command. 
			 */
			i_enable(opri);
			/*
			 * Unpin the bottom half of the driver
			 */
			(void *) unpincode( (int(*)()) idedisk_iodone);
			
			return (PM_ERROR);
		}
		i_enable(opri);
		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode( (int(*)()) idedisk_iodone);
		break;

	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
		opri = i_disable(INTTIMER);
		diskinfo->pm_pending |= PM_IDEDISK_PENDING_SUSPEND;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idedisk_trc(pmh_trc, trc, (char) 3, 
				(uint) diskinfo,
				(uint) diskinfo->cmds_out,
				(uint) pmh,
				(uint) pmh->handle.mode,
				(uint) mode);
#endif	
#endif
		if (diskinfo->cmds_out) {
			/*
			 * If there is outstanding I/O, then
			 * we will wait a little while to see
			 * if it completes.
			 */
			diskinfo->pm_pending |= PM_IDEDISK_PENDING_OP;
			
			/*
			 * Set timer (to 4 seconds) in case we sleep too
			 * long.
			 */
			diskinfo->pm_timer.watch.restart = 4;
			w_start(&(diskinfo->pm_timer.watch));

			/*
			 * We will sleep here. Either idedisk_iodone
			 * or our watchdog timer routine will wake us
			 * up.
			 */
			e_sleep((int *)&(diskinfo->pm_event),EVENT_SHORT);
		
			if (diskinfo->cmds_out) {
				/*
				 * If cmds_out is not zero, a command
				 * is still outstanding.  Turn off the
				 * watchdog timer and return with PM_ERROR.
				 */
				w_stop(&(diskinfo->pm_timer.watch));
				diskinfo->pm_pending &= 
					~PM_IDEDISK_PENDING_OP;
				diskinfo->pm_pending &= 
					~PM_IDEDISK_PENDING_SUSPEND;
				i_enable(opri);
				return (PM_ERROR);

			} else if (diskinfo->pm_pending &
					PM_IDEDISK_PENDING_OP) {
				/*
				 * If the pending idle flag
				 * is still set, then our
				 * watchdog timer has popped.
				 * So fail this with PM_ERROR.
				 */
				diskinfo->pm_pending &= 
					~PM_IDEDISK_PENDING_OP;
				diskinfo->pm_pending &= 
					~PM_IDEDISK_PENDING_SUSPEND;

				/*
				 * We need to restart any I/O
				 * that may have been suspended
				 * here.
				 */
				idedisk_start(diskinfo);
				i_enable(opri);
				return (PM_ERROR);
			} else {
				/*
				 * I/O finished within our timeout
				 * value so stop the watchdog timer
				 * here.
				 */
				w_stop(&(diskinfo->pm_timer.watch));
			}
		}
		/*
		 * If we get here, then there are no outstanding commands.
		 * So set mode to the requested mode.
		 */
		if (mode == PM_DEVICE_SUSPEND) {
			if (diskinfo->opened) {
				/*
				 * Spin down disk to save power. 
				 * For a suspend request, we must
				 * wait for this command to complete,
				 * because the IDE adapter driver
				 * will be called next by the PM core.
				 * If it has active commands, then
				 * it fails the suspend.
				 */
				idedisk_spinupdn(diskinfo, (uchar) DK_STANDBY);

				/*
				 * We will sleep here. idedisk_process_good
				 * or idedisk_process_error will wake us when
				 * the command has completed.
				 */
				e_sleep( (int *) &(diskinfo->pm_event),
						EVENT_SHORT );
			} else {
				/*
				 * If the device is not open
				 * then we need to do some special
				 * stuff to spin it down. First we
				 * must i_enable, because fp_ioctl
				 * and fp_opendev can't be called while
				 * i_disabled.  However since this device
				 * is not open, we need to guard against
				 * a possible open/unconfigure while 
				 * i_enabled.  We will try to take the
				 * driver's global lock using LOCK_NDELAY.
				 * If some one has this lock (i.e some
				 * one is attempting an open/unconfigure), 
				 * it will fail immediately and will will exit
				 * this handler with PM_ERROR.
				 *
				 * NOTE: A LOCK_SHORT would work for the IDE
				 * disk DD here, but a check for a change in
				 * the disk's open status would then be
				 * necessary after the lockl call.  To minimize
				 * differences between this driver and the
				 * SCSI disk driver, and to ease future
				 * conversion of this driver to MP safe or
				 * MP efficient status, LOCK_NDELAY is used.
				 */

				rc = lockl(&(idedd_info.lock), LOCK_NDELAY);

				i_enable(opri);
				if (rc == LOCK_FAIL) {
					/*
					 * Some one else has the device
					 * lock (i.e. someone is opening
					 * or unconfiguring this device.
					 * So fail the IDLE.
					 */

					diskinfo->pm_pending &= 
						~PM_IDEDISK_PENDING_OP;

					diskinfo->pm_pending &= 
						~PM_IDEDISK_PENDING_SUSPEND;
			
					return (PM_ERROR);
				}

				/*
				 * If we get here then, our lockl
				 * was successful. So we are now
				 * guarded against open/unconfigure
				 */
				(void) idedisk_spindown_closeddisk(diskinfo);

				opri = i_disable(INTTIMER);

				unlockl(&(idedd_info.lock));
			}

		}
		
		if (pmh->handle.mode == PM_DEVICE_ENABLE) {
			pmh->handle.activity =  PM_ACTIVITY_NOT_SET;
		}
		pmh->handle.mode = mode;
		i_enable(opri);
		break;
	
	case PM_PAGE_FREEZE_NOTICE:	
		/*
		 * Pin the bottom half of the driver
		 */
		errnoval = pincode( (int(*)()) idedisk_iodone);
		if (errnoval) {
			return PM_ERROR;
		}
		return PM_SUCCESS;
		break;

	case PM_PAGE_UNFREEZE_NOTICE:
		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode( (int(*)()) idedisk_iodone);
		return PM_SUCCESS;
		break;
		
	default:
		return PM_ERROR;
	}

	return PM_SUCCESS;
}
