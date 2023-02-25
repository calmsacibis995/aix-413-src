static char sccsid[] = "@(#)56  1.19  src/rspc/kernext/idecdrom/idecdromb.c, cdide, rspc41J, 9525E_all 6/21/95 12:27:08";
/*
 * COMPONENT_NAME: (IDECD) IDE CD-ROM Device Driver Bottom Half Routines
 *
 * FUNCTIONS:	idecdrom_build_cmd		  idecdrom_build_error
 * 		idecdrom_cdt_func 		  idecdrom_cmd_alloc
 * 		idecdrom_cmd_alloc_disable 	  idecdrom_coalesce
 * 		idecdrom_d_q_cmd 		
 * 		idecdrom_fail_disk 		  idecdrom_format_mode_data
 * 		idecdrom_free_cmd 		  idecdrom_free_cmd_disable
 * 		idecdrom_iodone 		  idecdrom_log_error
 * 		idecdrom_mode_data_compare 	  idecdrom_mode_select
 * 		idecdrom_mode_sense 		  idecdrom_pending_dequeue
 * 		idecdrom_pending_enqueue 	  idecdrom_prevent_allow
 * 		idecdrom_prevent_allow_disable 	  idecdrom_process_adapter_error
 * 		idecdrom_process_buf 		  idecdrom_process_buf_error
 * 		                                  idecdrom_process_error
 * 		idecdrom_process_good 		  idecdrom_process_ioctl_error
 * 		idecdrom_process_reqsns_error 	  idecdrom_process_reset
 * 		idecdrom_process_reset_error 	  idecdrom_process_ide_error
 * 		idecdrom_process_sense 	
 * 		idecdrom_q_cmd 			 
 * 		idecdrom_read_cap 		
 * 		
 * 		idecdrom_request_sense 		  idecdrom_spindown_closeddisk
 * 		idecdrom_sleep 			  idecdrom_start
 * 		idecdrom_start_disable 		  idecdrom_start_unit
 * 		idecdrom_start_unit_disable 	  idecdrom_start_watchdog
 * 		idecdrom_strategy 		  idecdrom_test_unit_ready
 * 		idecdrom_test_unit_ready_disable  idecdrom_trc
 * 		idecdrom_trc_disable 		  idecdrom_watchdog
 * 			 			  idecdrom_raw_iodone
 *		idecdrom_read_disc_info		  idecdrom_pm_handler
 *		                       		  idecdrom_pm_watchdog
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
#include <sys/watchdog.h>
#include <sys/lvdd.h>
#include <sys/errids.h>
#include <sys/dump.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/iostat.h>

#include <sys/idecdrom.h>
#include "idecdromdd.h"
#include <sys/pm.h>



/* End of Included System Files */

/************************************************************************/
/*									*/
/*			Static Structures				*/
/*									*/
/*	The idedisk_list data structure is an array of pointers to	*/
/*	idecdrom_diskinfo structures describing each of the disk	*/
/*	device's currently configured in the system.			*/
/*									*/
/*	The idedisk_open_list data structure is similar to the disk_list*/
/*	the only exception being that it contains only those disk	*/
/*	devices currently open on the system.				*/
/*									*/
/*	The idecdrom_info data structure is a structure used for storing*/
/*	general disk driver information, including global states,	*/
/*	configuration counts, global locks, etc.			*/
/*									*/
/*	The diskinfo data structures are allcoated on a one per device	*/
/*	basis and are used for storing device specific information.	*/
/*	These structures are dynamically allocated when a device is	*/
/*	configured into the system, and deallocated when the device	*/
/*	is deleted. These structures are linked into the disk_list	*/
/*	upon configuration and linked into the disk_open_list during	*/
/*	any outstanding opens.						*/
/*									*/
/************************************************************************/

struct idecdrom_info	idecdrom_info = {
	0, 0, LOCK_AVAIL, NULL };
struct idecdrom_diskinfo	*idedisk_list[DK_MAXCONFIG] = {
	NULL };
struct idecdrom_diskinfo	*idedisk_open_list[DK_MAXCONFIG] = {
	NULL };

/*
 * To allow hardware folks to turn off device driver thresholding
 */
int idecdrom_threshold;
/* 
 * offset into diskinfo structure where our formatted mode data is located.
 * This is also for the hardware folks.
 */
int idecdrom_mode_data_offset; 

   
#ifdef DEBUG
int			idecdrom_debug = 0;


struct idecdrom_trace *idecdrom_trctop = NULL;
struct idecdrom_trace *idecdrom_trace;
int idecdrom_trcindex = 0;

/*
 ******  strings for Internal Debug Trace Table *******
 * To use compile with the flags IDE_GOOD_PATH and IDE_ERROR_PATH
 * the variable idecdrom_trace will be the beginning of the trace table in
 * memory.  The variable idecdrom_trctop will point to location where the
 * next entry in the trace table will go.
 */

char      *topoftrc     = "*DKTOP**";

char     *strategy      = "STRATEGY";
char     *insertq       = "INSERT_Q";
char     *dequeue       = "DEQUEUE ";
char     *start         = "START   ";
char     *coales        = "COALESCE";
char     *interrupt     = "INTR    ";
char     *good          = "GOOD	   ";
char     *error         = "ERROR   ";
char     *sdiodone      = "IODONE  ";
char     *rw_iodone     = "RAWIODON";
char     *bufretry      = "BUFRETRY";
char     *failbuf       = "FAIL_BUF";
char     *faildisk      = "FAILDISK";
char     *ideerror      = "IDE_ERR ";
char     *adaperror     = "ADAP_ERR";
char     *processbuf    = "BUFCMPLT";
char     *processreset  = "RESETCMP";
char     *qcmd          = "Q_CMD   ";
char     *dqcmd         = "D_Q_CMD ";
char     *cmdalloc      = "CMDALLOC";
char     *cmdfree       = "FREE_CMD";
char     *cmdfail       = "FAIL_CMD";
char     *prevent       = "PRV_ALLOW";
char     *startunit     = "STRTUNIT";
char     *testunit      = "TESTUNIT";
char     *reqsns        = "REQSNS  ";
char     *modesense     = "M_SENSE ";
char     *modeselect    = "M_SELECT";
char     *readcap       = "READ_CAP";
char	 *rdiscinfo	= "READINFO";
char     *inquiry       = "INQUIRY ";
char     *idesense      = "IDESENS ";
char     *logerr        = "LOG ERR ";
char     *cmdtimer	= "TIMER   ";
char     *specialerr	= "SPEC_ERR";
char     *reqsnserror   = "REQS_ERR";
char     *buildcmd	= "BUILDCMD";
char	 *ioctlerr	= "IOCTLERR";
char     *reseterr	= "RESETERR";
char     *sttimer	= "STRT_TMR";
char	 *issue		= "ISSUE   ";
char     *pmh_trc       = "PM_HANDL";
char     *pmh_timer     = "PM_TIMER";

char     *entry         = " IN";	/* Entry into routine                */
char     *exit          = " EX";	/* Exit from routine                 */
char     *trc           = " TR";	/* Trace point in routine            */
#endif


/************************************************************************/
/*									*/
/*	NAME:	idecdrom_strategy					*/
/*									*/
/*	FUNCTION: IDE CD-ROM Device Driver Strategy Routine.		*/
/*		This routine is called with a NULL terminated linked	*/
/*		list of buf structs. It scans the incoming list in an	*/
/*		attempt to coalesce contiguous read operations to the	*/
/*		same device together in a single ide operation.		*/
/*		Coalesced requests are placed in an IN_PROGRESS queue	*/
/*		where they can be processed by idecdrom_start. Requests	*/
/*		that are not coalesced are placed in a PENDING queue	*/
/*		where they will be processed by idecdrom_start in future*/
/*		IO operations.						*/
/*									*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use disable_lock, unlock_enable.			*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*		The device's busy flag is set and cleared during	*/
/*		coalescing to insure the request queues integrity	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		idecdrom_info	General disk driver information		*/
/*		idedisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		bp	- Address of first buf struct in list		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:	None				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned in the	*/
/*		buf struct b_error field:				*/
/*		EIO	- Device is offline or starting to close	*/
/*		ENXIO	- Invalid disk block address			*/
/*			- Device not found in open list			*/
/*		EINVAL	- Request does not start on a dev blk boundary	*/
/*		 	- b_bcount is larger than maxrequest		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable	i_enable				*/
/*		iodone		minor					*/
/*									*/
/*									*/
/************************************************************************/

int
idecdrom_strategy(
struct buf	*bp)
{
	int			blkno, count,rc;
	int			opri, last_blk, dev;
	dev_t			devno, first_devno;
	struct buf		*next_bp;
	struct idecdrom_diskinfo	*diskinfo, *last_diskinfo = NULL;
	struct idecdrom_stats	*stats;


	first_devno = bp->b_dev;

	
	opri = i_disable(INTIODONE);


#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(strategy, entry,(char)0, (uint)bp, (uint)bp->b_dev,
	      (uint)bp->b_blkno, (uint)bp->av_forw,(uint)0);
#endif	
#endif
	DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_STRATEGY, 0, first_devno, bp,
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
		}
		else {
			last_blk = diskinfo->capacity.lba;
			diskinfo->cmd_pending = TRUE;


		  	IDECDROM_GET_LBA(blkno,diskinfo,bp,rc);
			if (rc<0) {
			  	/*
                                 * Fail any buf if diskinfo->block_size
                                 * is not a multiple of DK_BLOCKSIZE,
                                 * since there is no way to convert
                                 * it to an integral block number
                                 * for this device's block size.
                                 */
                                bp->b_flags |= B_ERROR;
                                bp->b_error = EINVAL;
                                bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if (bp->b_bcount > diskinfo->max_request) {
				/* 
				 * Fail any buf that is greater than our 
				 * maximum transfer size
				 * 
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EINVAL;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if (blkno > (last_blk+1)) {
				/*
				 * Verify block address on device
				 */
				bp->b_resid = bp->b_bcount;
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
				iodone(bp);
			}
			else if (blkno == (last_blk+1)) {
				bp->b_resid = bp->b_bcount;
				if (!(bp->b_flags & B_READ)) {
					bp->b_flags |= B_ERROR;
					bp->b_error = ENXIO;
				}
				iodone(bp);
			}
			else if (bp->b_bcount == 0) {
				/*
				 * Check for zero length read
				 */
				bp->b_resid = 0;
				iodone(bp);
			}
			else if ((diskinfo->state == DK_OFFLINE) ||
				 (diskinfo->starting_close)) {
				/*
				 * Verify transfer validity
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EIO;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else {
				/*
				 * If we got here then we passed all 
				 *  of the above checks
				 */


				/* 
				 * Filter out operation beyond last block 
				 * of disk 
				 */
				if ((blkno + (count / diskinfo->block_size)) > 
				    (last_blk + 1)) {

					bp->b_resid = 
					  ((blkno + 
					    (count / diskinfo->block_size))
					   - (last_blk + 1)) * 
					     diskinfo->block_size;

				} else {
					bp->b_resid = 0x00;
				}
				
				/* Update transfer statistics for device */
				stats = &(diskinfo->stats);
				if ((bp->b_flags & B_READ) &&
				    ((stats->byte_count += bp->b_bcount) >
				     stats->segment_size)) {

					stats->segment_count++;
					stats->byte_count %= 
						stats->segment_size;
				}
				
				
				if ((diskinfo != last_diskinfo) &&
				    (last_diskinfo != NULL)) {
					idecdrom_start(last_diskinfo);
				}
				
				
				/*
				 * Put buf on this disk's pending queue
				 */
				idecdrom_pending_enqueue(diskinfo, 
						       bp);
				
				

			}
		} /* else: diskinfo != NULL */


		last_diskinfo = diskinfo;
		bp = next_bp;

		if (diskinfo != NULL) {
			diskinfo->cmd_pending = FALSE;
		}
	} /* while */

	/*
	 * Call start for the last disk processed above
	 */
	idecdrom_start(diskinfo);

	
	DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_STRATEGY, 0, first_devno);
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(strategy, exit,(char)0, (uint)diskinfo, (uint)0,
	      (uint)0,(uint)0,(uint)0);
#endif	
#endif

	/* 
	 * Enable intrs and unlock for next 
	 * pass thru 
	 */


	i_enable(opri);
	return(0);
}


/************************************************************************/
/*									*/
/*	NAME:	idecdrom_pending_enqueue				*/
/*									*/
/*	FUNCTION: IDE CD-ROM Device Driver Pending Enqueue Routine	*/
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
/*		must use disable_lock, unlock_enable.			*/
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
/*	EXTERNAL PROCEDURES CALLED:	None				*/
/*									*/
/*									*/
/************************************************************************/

void idecdrom_pending_enqueue(
struct idecdrom_diskinfo	*diskinfo, 
struct buf *bp)	
{	

	char	done = FALSE;
	struct buf *curr,
		   *prev;


        curr = diskinfo->low;     /* set current pointer to low_cyl pointer */ 
        prev = NULL;            /* set previous pointer to NULL */

        if ( curr == NULL) {
		/*
		 * we are starting with an empty list 
		 */
		diskinfo->low = bp;	/* set low ptr to this one */
		diskinfo->currbuf = bp;/* set current ptr to this one */
                bp->av_forw = NULL;  /* set this ones forward (NULL) */
                bp->av_back = NULL;  /* set this ones back (NULL) */
		return;
	}
		
	do {
		/*
		 * while not at end of list and not done. We can 
		 * use the buf's 512 blkno since the order still remains
		 * the same even if the device's block size is not 512.
		 */
                if (curr->b_blkno <= bp->b_blkno) {
			/*
			 * if this one is less or equal to the one we're trying
			 * to insert set prev to this one, and walk the
			 * curr pointer to the next one
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
			done = TRUE;
        	}
        
	} while ((curr != NULL) && (!done)) ;        

        if ( curr == NULL) {
		/*
		 * we're out of the while, so if curr is NULL, then we
		 * walked to the end of the list
		 */
		prev->av_forw = bp; /* set prev's forward ptr to this */
		bp->av_back = prev;	/* set back pointer to prev */
                bp->av_forw = curr;  /* set this ones forward to curr (NULL) */
        }

	return;
}
	
	

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_pending_dequeue				*/
/*									*/
/*	FUNCTION: IDE CD-ROM Device Driver Pending Dequeue Routine	*/
/*		This routine is called with a diskinfo structure 	*/
/*		pointer.  This routine is called to remove the current  */
/*		buf from the pending queue. 				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use disable_lock, unlock_enable.			*/
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
/*	EXTERNAL PROCEDURES CALLED:	None				*/
/*									*/
/*									*/
/************************************************************************/

void idecdrom_pending_dequeue(
struct idecdrom_diskinfo *diskinfo)
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
		 * if at end of elevator, goto to first floor
		 */
		diskinfo->currbuf = diskinfo->low;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_start_disable					*/
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
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable	i_enable				*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_start_disable(
struct idecdrom_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr; 		/* command block pointer*/
	int			opri;


	opri = i_disable(INTIODONE);
	idecdrom_start(diskinfo);
	i_enable(opri);
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_start						*/
/*									*/
/*	FUNCTION: IDE CD-ROM Device Driver Start IO Routine.		*/
/*		This routine will try to queue "queue_depth" commands	*/
/*		to the specified device (queue_depth is always 1 for 	*/
/*		the ataide adapter). It scans the command queue        	*/
/*		first.  This queue contains all error recovery commands	*/
/*		including normal reads to be retried.      		*/
/*									*/
/*		It next sees if there is an ioctl request pending if	*/
/*		so it will issue the request. Next the IN_PROGRESS queue*/
/*		is checked to start any ops that have already been	*/
/*		coalesced, finally the pending queue is checked, and	*/
/*		if it is not empty, idecdrom_coalesce is called to	*/
/*		coalesce ops into the IN_PROGRESS queue and		*/
/*		idecdrom_build_cmd is called to construct an op	        */
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
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devstrat						*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_start(
struct idecdrom_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr; 		/* command block pointer     */
	int	         opri;
        uchar           build_cmd_flag;


#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(start, entry,(char)0, (uint)diskinfo, (uint)diskinfo->state ,
	      (uint)diskinfo->dk_cmd_q_head, (uint)diskinfo->ioctl_cmd.flags,
	      (uint)diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

        /*
	 * We will loop until we have queue dept commands queued.  If certain
	 * diskinfo->state's exist then we will only issue on command at a time
	 * or starve certain requests until this condition is cleared
	 */
	
	while (TRUE) {
		

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(start, trc,(char)0, (uint)diskinfo->cmds_out, 
	      (uint)diskinfo->state,(uint)diskinfo->timer_status,
	      (uint)diskinfo->dk_cmd_q_head,
	      (uint)diskinfo->dk_bp_queue.in_progress.head);

   idecdrom_trc(start, trc,(char)1, (uint)diskinfo->queue_depth, 
	      (uint)diskinfo->raw_io_cmd,(uint)diskinfo->raw_io_cmd,
	      (uint)diskinfo->ioctl_pending,
	      (uint)diskinfo->ioctl_cmd.flags);

#endif
#endif

		if ((diskinfo->state & DK_RECOV_MASK) ||
		    (diskinfo->cmds_out >= diskinfo->queue_depth) ||
		    (diskinfo->timer_status)) {

			DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);

			/*
			 * If we have request sense, reset,
			 * or set our timer we do not issue 
			 * any more commands until this completes.  Also if we 
			 * have reached our queue depth do not issue any thing 
			 * else.
			 */


			DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
		}
		if (diskinfo->state & DK_RST_FAILED) { 

			/*
			 * An attempt to do the reset sequence has
			 * failed. Before we issue another command we
			 * must send a start unit and try the
			 * reset cycle over.
			 */
			DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);
			if ((diskinfo->starting_close) ||
			    ((diskinfo->dk_cmd_q_head == NULL) &&
			    (!(diskinfo->ioctl_cmd.flags & DK_READY)) &&
			    (diskinfo->dk_bp_queue.in_progress.head 
				                                == NULL) &&
			    (diskinfo->currbuf == NULL)))   { 
				/*
				 * If an attempt is being made to close this
				 * disk so do not send another start unit
				 * down, but state state back to offline.
				 * If a reset failed and we have no commands
				 * pending then exit.  We do not want to
				 * issue the start unit until some other
				 * command is to be sent.
				 */

				DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 
					0, diskinfo->devno);
				return;
			}

			/*
			 *  At some point we detected a unit attention 
			 *  or some other condition that required us
			 *  to go through the reset sequence, but
			 *  the sequence failed. Therefore, before
			 *  any command can be issued, we must
			 *  try again.
			 */
			diskinfo->state = DK_ONLINE;
			idecdrom_test_unit_ready(diskinfo);

			DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 
					0, diskinfo->devno);
			return;

		} 	
		diskinfo->dkstat.dk_status |= IOST_DK_BUSY;
		if ((diskinfo->dk_cmd_q_head != NULL)) {
			/* 
			 * Start cmd struct on top of command queue.  This
			 * queue is for error recovery commands.
			 */

			cmd_ptr = diskinfo->dk_cmd_q_head;

			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

			if (cmd_ptr->type & (DK_REQSNS|DK_RESET)) {
				/*
				 * If this is a request sense or reset 
				 * operation then set the state flag of this 
				 * disk accordingly.  NOTE: The cmd_ptr->type
				 * defines match the corresponding 
				 * diskinfo->state defines.
				 */
				diskinfo->state |= cmd_ptr->type;
			 }

                } else if (diskinfo->pm_pending & PM_IDECDROM_PENDING_SUSPEND) {
                        /*
                         * If we are trying to go into a PM_DEVICE_SUSPEND
                         * or PM_DEVICE_HIBERNATION mode then we don't want
                         * to issue any non-error recovery commands.
                         */
                        DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0,
                                diskinfo->devno,
                                0, 0, 0, 0);

                        DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 0,
                                diskinfo->devno);
                        break;

	        } else if (diskinfo->ioctl_cmd.flags & DK_READY) {
			/*
			 * Issue the ioctl that is ready for this
			 * disk
			 */
			if (!diskinfo->cmds_out) {
				/*
				 * If no outstanding commands then issue
				 * ioctl.
				 */
				cmd_ptr = &diskinfo->ioctl_cmd;
				cmd_ptr->flags &= ~DK_READY;
				diskinfo->ioctl_pending = TRUE;
			}
			else {
				/*
				 * Wait for outstanding commands to complete
				 */
				DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0, 
					diskinfo->devno,
					0, 0, 0, 0);

				DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 0, 
					diskinfo->devno);
				break;
			}
	        } else if (diskinfo->ioctl_pending ) {
			/*
			 * Allow only error recovery commands to be issued if
			 * an ioctl is pending
			 */
			DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);

			DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;	
                } else if ((diskinfo->raw_io_cmd != NULL)) {

                        /* 
			 * If raw queue has anything pending
			 * start single command structure on this queue 
			 */
                        cmd_ptr = diskinfo->raw_io_cmd;
                        diskinfo->raw_io_cmd = NULL;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(start, trc,(char)2, (uint)cmd_ptr, 
	      (uint)diskinfo->raw_io_cmd,(uint)0,
	      (uint)0,(uint)0);
#endif
#endif

	

		} else if (diskinfo->dk_bp_queue.in_progress.head != NULL) {
			/* 
			 * If a request was coalesced but there were no 
			 * resources available at the time then we will arrive
			 * here.  We must issue this request(s) before
			 * we call coalesce again.
			 */

			cmd_ptr = 
				idecdrom_build_cmd(diskinfo,
				    (struct buf **) 
				     &(diskinfo->dk_bp_queue.in_progress.head),
				     (char) TRUE);


		} else if (diskinfo->currbuf != NULL) {
                        /* Coalesce into IN PROGRESS queue and start */
                        build_cmd_flag = idecdrom_coalesce(diskinfo);
                        if (build_cmd_flag == 1) {
                           cmd_ptr =
                                idecdrom_fragmented_build_cmd(diskinfo,
                                     (char) TRUE);
                        }
                        else {
                           cmd_ptr =
                                idecdrom_build_cmd(diskinfo,(struct buf **)
                                  &(diskinfo->dk_bp_queue.in_progress.head),
                                     (char) TRUE);
                        }



		} else {
			DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno, 
				0, 0, 0, 0);
			if (!(diskinfo->cmds_out)) {
				/* 
				 * If no more commands outstanding clear 
				 * busy status for this disk
				 */
				diskinfo->dkstat.dk_status &= ~IOST_DK_BUSY;
			}

			/*
			 * Nothing on any of our queues so exit this loop
			 */
			DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
		}

		DDHKWD5(HKWD_DD_IDECDROMDD, DD_ENTRY_BSTART, 0, 
			diskinfo->devno,
			&cmd_ptr->ataidebuf, 
			cmd_ptr->ataidebuf.bufstruct.b_flags,
			cmd_ptr->ataidebuf.bufstruct.b_blkno,
			cmd_ptr->ataidebuf.bufstruct.b_bcount);

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
	
		if (diskinfo->restart_unit == TRUE) {
			/*
			 * If this is the first command sent
			 * afer an error occured then set SC_RESUME in the buf
			 */
			diskinfo->restart_unit = FALSE;
		}
		
		cmd_ptr->ataidebuf.bufstruct.b_flags &= ~(B_ERROR | B_DONE);
		cmd_ptr->ataidebuf.bufstruct.b_error = 0x00;
		cmd_ptr->status |= DK_ACTIVE;
		

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(issue, trc,(char)0, (uint)cmd_ptr, 
	      (uint)cmd_ptr->type,(uint)cmd_ptr->subtype,
	      (uint)cmd_ptr->status,
	      (uint)cmd_ptr->retry_count);
#endif
#endif
		cmd_ptr->retry_count++;
		diskinfo->cmds_out++;

		DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_BSTART, 0, 
			diskinfo->devno);

                /*
		 * Note: We can't keep our simple lock across calls to 
		 * devstrat, but we must remained disabled.  We can't just
		 * use a simple_unlock, because on UP this will crash.
		 * Instead we will use an unlock_enable with the prioty the
		 * same as our current priority (INTIODONE).
		 */

		/* Power Management activity flag */
                diskinfo->pmh.handle.activity = PM_ACTIVITY_OCCURRED;

		/* Commands to the device will
		 * generally cause the device to spin up if it was in the
		 * PM_DEVICE_IDLE mode, so set the mode to PM_DEVICE_ENABLE.
		 */
		if (diskinfo->pmh.handle.mode != PM_DEVICE_FULL_ON) {
			diskinfo->pmh.handle.mode = PM_DEVICE_ENABLE;
		}

		devstrat(cmd_ptr);
	} /* end of while (TRUE) */  
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(start, exit,(char)0, (uint)diskinfo, (uint)diskinfo->state ,
	      (uint)diskinfo->dk_cmd_q_head, (uint)diskinfo->cmds_out,
	      (uint)diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_iodone						*/
/*									*/
/*	FUNCTION: IDE CD-ROM Device Driver IO Completion Routine.	*/
/*		This routine determines if the indicated operation	*/
/*		completed successfully or failed. If successful,	*/
/*		the appropriate routine is called to process the	*/
/*		specific type of cmd. If failed, the general failure	*/
/*		processing routine is called. Upon exiting this		*/
/*		routine, idecdrom_start is called to begin processing   */
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns a zero value.			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_iodone(
struct dk_cmd	*cmd_ptr)
{
	struct idecdrom_diskinfo	*diskinfo;
	struct buf		*bp;
	uint	      		old_pri;




	/* 
	 * disable my irpt level and spin lock
	 */

	/* Get diskinfo struct */
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;
	bp = &(cmd_ptr->ataidebuf.bufstruct);
	DDHKWD1(HKWD_DD_IDECDROMDD, DD_ENTRY_IODONE, 0, diskinfo->devno);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(interrupt, entry,(char)0, (uint)diskinfo, (uint)diskinfo->state ,
	      (uint)diskinfo->cmds_out, (uint)cmd_ptr,(uint)cmd_ptr->type);
#endif
#endif
	/* 
	 * decrement cmds_out  (i.e. the number of outstanding commands)
	 */

	diskinfo->cmds_out--;
	cmd_ptr->status &= ~DK_ACTIVE;

	/* Test for Successful Completion of operation */
	if ((bp->b_flags & B_ERROR) || ((bp->b_resid != 0) && 
	    (cmd_ptr->type != DK_REQSNS) && 
            (cmd_ptr->subtype != DK_READ_INFO) &&
	    (cmd_ptr->subtype != DK_MSENSE))) { /* Operation FAILED */
		/*
                 * Call idecdrom_process_error to handle error
                 * logging and determine what type of error
                 * recovery must be done.
                 */
		idecdrom_process_error(cmd_ptr);
	} else {
		/*
		 * This command completed successfully
		 * process it as such.
		 */
		idecdrom_process_good(cmd_ptr);
	}

	if (!(diskinfo->cmds_out)) {
		/* 
		 * If no more commands outstanding clear busy status for this 
		 * disk
		 */
		diskinfo->dkstat.dk_status &= ~IOST_DK_BUSY;

                /* Do pending PM operation */
                if (diskinfo->pm_pending & PM_IDECDROM_PENDING_OP) {
                        /*
                         * Our PM handler is waiting on I/O to complete.
                         * So let's wake it up.
                         * NOTE: This will cause the PM handler
                         * to return PM_SUCCESS, since we're
                         * clearing the PM_IDECDROM_PENDING_OP flag
                         * here.
                         */
                        diskinfo->pm_pending &= ~PM_IDECDROM_PENDING_OP;
                        e_wakeup((int *)&(diskinfo->pm_event));
                }

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(interrupt, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state ,
	      (uint)diskinfo->cmds_out, (uint)0,(uint)0);
#endif
#endif
	}
	/* 
	 * Start all the pending operations that we can.
	 */
	idecdrom_start(diskinfo);


#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(interrupt, exit,(char)0, (uint)diskinfo, (uint)diskinfo->state ,
	      (uint)diskinfo->cmds_out, (uint)0,(uint)0);
#endif
#endif
	DDHKWD1(HKWD_DD_IDECDROMDD, DD_EXIT_IODONE, 0, diskinfo->devno);



	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_good					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes successful completion of		*/
/*		an IDE request.						*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_good(
struct dk_cmd	*cmd_ptr)
{
	struct buf		*bp;
	struct idecdrom_diskinfo	*diskinfo;



#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(good, entry,(char)0, (uint)cmd_ptr->diskinfo, 
	      (uint)cmd_ptr->diskinfo->state ,(uint)cmd_ptr->diskinfo->cmds_out,
	      (uint)cmd_ptr,(uint)cmd_ptr->type);
#endif
#endif
	if (cmd_ptr->status & DK_QUEUED) {
		/*
		 * Commands with good completion are immediately dequeued 
		 * for processing
		 */
		idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
	}
	bp = &(cmd_ptr->ataidebuf.bufstruct);

	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;
	switch (cmd_ptr->type) {
	case DK_REQSNS:

		idecdrom_process_sense(cmd_ptr);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(idesense, exit,(char)0, (uint)diskinfo->state, 
	      (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif
#endif
		break;
	case DK_RESET:
		idecdrom_process_reset(cmd_ptr);
		break;
	case DK_IOCTL:
		cmd_ptr->flags &= ~DK_READY;
                cmd_ptr->intrpt = 0;
                e_wakeup(&(bp->b_event));
		diskinfo->reset_failures = 0;
		diskinfo->ioctl_pending = FALSE;
		break;
	case DK_BUF:
		idecdrom_process_buf(cmd_ptr);
		break;

	default:
		/* Process unknown cmd type */
		ASSERT(FALSE);
	};
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(good, exit,(char)0, (uint)diskinfo, (uint)diskinfo->state ,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,(uint)0);
#endif
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine examines the request sense data returned	*/
/*		from a device. It must determine what state the device	*/
/*		is in and what error recovery should be performed	*/
/*		on the device. Recovered and non-recovered errors	*/
/*		must be handled. Recovery may include:			*/
/*			retry operation					*/
/*			abort operation					*/
/*			issue recovery operation			*/
/*									*/
/*		When this routine exits, the device's cmd stack		*/
/*		should either be empty, or contain a cmd to be		*/
/*		started.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*   		IDE Specifies the following format for Sense Data:      */
/*									*/
/*	         Error Codes 70h and 71h Sense Data Format		*/
/* +=====-======-======-======-======-======-======-======-======+	*/
/* |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/* |Byte |      |      |      |      |      |      |      |      |	*/
/* |=====+======+================================================|	*/
/* | 0   | Valid|          Error Code (70h or 71h)               |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 1   |                 Segment Number (Reserved)             |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 2   |    Reserved |  ILI |Reserv|     Sense Key             |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 3   | (MSB)                                                 |	*/
/* |- - -+---              Information                        ---|	*/
/* | 6   |                                                 (LSB) |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 7   |                 Additional Sense Length (n-7)         |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 8   | (MSB)                                                 |	*/
/* |- - -+---              Command-Specific Information       ---|	*/
/* | 11  |                                                 (LSB) |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 12  |                 Additional Sense Code                 |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 13  |                 Additional Sense Code Qualifier       |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 14  |                 Field Replaceable Unit Code           |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 15  |  SKSV|                                                |	*/
/* |- - -+------------     Sense-Key Specific                 ---|	*/
/* | 17  |                                                       |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 18  |                                                       |	*/
/* |- - -+---              Additional Sense Bytes             ---|	*/
/* | n   |                                                       |	*/
/* +=============================================================+	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		xmemout							*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_process_sense(
struct dk_cmd	*cmd_ptr)
{
	struct idecdrom_diskinfo		*diskinfo;
	struct ataide_buf			*ataidebuf;
	struct buf			*bp;
	struct idecdrom_stats		*stats;
	struct idecdrom_req_sense_info	*sense_data;
	struct idecdrom_ioctl_req_sense	*ioctl_req_sense;
	struct dk_cmd			*log_cmd_ptr;
	uchar				sense_key, sense_code;
	uchar				sense_qualifier, verify_done;
	ulong				error_blkno, recovery_level;
	int				rc;
	int				sense_code_qualifier;/* sense_code    */
							     /* combined with */
							     /*sense_qualifier*/





	/* Get diskinfo struct for this device */
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;

	/* free REQSNS cmd  */

	idecdrom_free_cmd(cmd_ptr);
	diskinfo->state &= ~DK_REQSNS_PENDING;

	/* Get address of sense data buffer */
	sense_data = (struct idecdrom_req_sense_info *) diskinfo->sense_buf;

	/*
	 * Save the command pointer on top of the stack for logging.
	 * If an error is to be logged, it is for the command on the top of
	 * of the stack.
	 */
	log_cmd_ptr = diskinfo->checked_cmd;
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(idesense, entry,(char)0, (uint)diskinfo->state, 
	      (uint)sense_data ,(uint)diskinfo->checked_cmd, (uint)0,
	      (uint)diskinfo->dk_cmd_q_head);
#endif
#endif

	/*
	 * Set cmd_ptr to address of checked_cmd
         * This is the cmd that received the Check Condition
         * it is ready for a retry attempt to be performed
         */
	cmd_ptr = diskinfo->checked_cmd;	
	verify_done = FALSE;
	
	ASSERT(cmd_ptr != NULL);


	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	ataidebuf = &(cmd_ptr->ataidebuf);
	bp = &(ataidebuf->bufstruct);
	stats = &(diskinfo->stats);
	sense_data = (struct idecdrom_req_sense_info *) diskinfo->sense_buf;
	sense_key = sense_data->sense_key & 0x0f;
	sense_code = sense_data->add_sense_key;
	sense_qualifier = sense_data->add_sense_qualifier;

	/*
         * If this was a DKIORDSE, transfer the request sense
         * data to the user buffer and set the "valid sense" flag.
         */
	if (cmd_ptr->type == DK_IOCTL) {
		ioctl_req_sense = &(diskinfo->ioctl_req_sense);
		if (ioctl_req_sense->count != 0) {
			rc = xmemout((caddr_t) sense_data,
			    (caddr_t) ioctl_req_sense->buffer,
			    (int) ioctl_req_sense->count,
			    &(ioctl_req_sense->xmemd));
			if (rc == XMEM_SUCC) {
				cmd_ptr->ataidebuf.status_validity |= 
				    SC_VALID_SENSE;
			}
		}
	}
	/* Test for recovery level on error */

	/* If the sense key is RECOVERED ERROR, HARDWARE ERROR or 
	 * MEDIUM ERROR and if the SKSV bit is one, the sense-key specific 
	 * field shall be defined as shown below:
	 *
	 *              Actual Retry Count Bytes
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|  7   |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+======+================================================|
	 * | 15  | SKSV |  C/D | Resvd| Resvd| BPV  |    Bit Pointer     |
	 * |-----+-------------------------------------------------------|
	 * | 16  | (MSB)                                                 |
	 * |-----+---                 Actual Retry Count              ---|
	 * | 17  |                                                 (LSB) |
	 * +=============================================================+
	 */

	recovery_level = ((sense_data->field_ptrM << 0x08) |
	    (sense_data->field_ptrL));

	if (!verify_done) {
		/* Test for LBA valid in sense data */

		if (sense_data->err_code & 0x80) {
			error_blkno = ((sense_data->sense_byte0 << 0x18) |
			    (sense_data->sense_byte1 << 0x10) |
			    (sense_data->sense_byte2 << 0x08) |
			    (sense_data->sense_byte3));

			bp->b_resid = bp->b_bcount - 
			   ((error_blkno - bp->b_blkno) * diskinfo->block_size);
		} else {
			error_blkno = 0xffffffff;
			if (sense_key != DK_RECOVERED_ERROR) {
				/*
				 * If this was not a recovered error then set
				 * b_resid.  The recovered errors will handle
				 * the b_resid on a case by case basis.
				 */
				
				bp->b_resid = bp->b_bcount;
			}
		}
	}


#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(idesense, trc,(char)0, (uint)sense_key,(uint)sense_code, 
	      (uint)sense_qualifier, (uint)recovery_level,(uint)error_blkno);
#endif
#endif


	/* Evaluate sense data for recovery procedure determination */
	switch (sense_key) {
	case DK_NO_SENSE:
		/* Process unknown condition */
		if ((sense_code == 0x00) && (sense_qualifier == 0x11)) {
		  	/*
			 * Play Audio operation in progress for CD-ROM
			 * drive.
			 */
			bp->b_error = EINPROGRESS;
			break;
		}
		idecdrom_build_error(log_cmd_ptr, DK_UNKNOWN_ERR, 
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
		break;
	case DK_RECOVERED_ERROR:
		/*
                 * Cmd completed successfully with some recovery action
                 */

		/*
		 * Log all recovered
		 * errors, but return good completion.
		 */
		idecdrom_build_error(log_cmd_ptr,DK_RECOVERED_ERR,
				   DK_OVERLAY_ERROR,SC_VALID_SENSE);
		idecdrom_log_error(log_cmd_ptr, DK_SOFT_ERROR);
		log_cmd_ptr->error_type = 0;
		idecdrom_process_good(cmd_ptr);
		return;
	case DK_NOT_READY:
		/* Device was not ready for access */
		if ((cmd_ptr->subtype == DK_TUR) && 
		    (diskinfo->reset_count < DK_MAX_RESETS)) {
			/* issue a start unit */
			idecdrom_start_unit(diskinfo,
					  (uchar) DK_START);
			return;
		}
		switch (sense_code) {
		case 0x04:
			/* Given this special case, issue a start unit */
			
			bp->b_error = ENOTREADY;
			if (diskinfo->reset_count < DK_MAX_RESETS) {
				if (!(cmd_ptr->type & DK_RESET)) {

					idecdrom_start_unit(diskinfo,
					    (uchar) DK_START);
					return;
				}
				else if (cmd_ptr->retry_count < DK_MAX_RETRY) {
					idecdrom_start_watchdog(diskinfo,
							      (ulong)4);
	
					return;
				}
			}

			break;
                case 0x3a:
			/* Media is missing */
			bp->b_error = ENOTREADY;
			cmd_ptr->retry_count = DK_MAX_RETRY;
                        break;
                case 0x30:
			/* Media not formatted or incompatible format */
			bp->b_error = EFORMAT;
			cmd_ptr->retry_count = DK_MAX_RETRY;
                        break;
		case 0x53:
			/*
			 * Typically indicates problems with
			 * removable media.
			 */
			if ( sense_qualifier == 0x00) {
			  	/*
				 * Media unload or eject failed
				 */
			  	bp->b_error = ENOTREADY;
			}
			else {
				/* 
				 * Sense qualifier of 02 is the
				 * only other possible value.
				 * Sense qualifier
				 * of 02 indicates Media Removal prevented 
				 * on eject command 
				 * This can only occur due to some one
				 * using the CDEJECT ioctl command
				 * after some one has used the CDIOCMD
				 * interface to lock the drive.
				 * So issue an allow command an retry
				 * the failed command (eject).
				 */
				if (cmd_ptr->retry_count < DK_MAX_RETRY) {
					idecdrom_prevent_allow(diskinfo,
							     (uchar) DK_ALLOW);
					return;
				}
				bp->b_error = ENOTREADY;
			}
			break;
		case 0x40:
		case 0x44:
		default:
			idecdrom_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = ENOTREADY;
			break;
		}
		break;
	case DK_MEDIUM_ERROR:
		/* A fatal Media error has occurred */
		switch (sense_code) {
		case 0x02:
		case 0x09:
			if (cmd_ptr->retry_count < DK_MAX_RETRY) {

				/* The hardware hits this type error, but is okay on retry with delay*/
				idecdrom_build_error(log_cmd_ptr, DK_MEDIA_ERR,
			    		DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
				idecdrom_start_watchdog(diskinfo,(ulong)2);
				diskinfo->timer_status |= DK_TIMER_BUSY;
				return;
			}
		case 0x11:
		case 0x15:
		case 0x44:
		default:
			/* Mark Error type as a media error for processing */
			idecdrom_build_error(log_cmd_ptr, DK_MEDIA_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = EIO;        /* no relocation */
			break;
		}
		break;
	case DK_HARDWARE_ERROR:
		/* A fatal hardware error occurred at the device */
		switch (sense_code) {
		case 0x02:
		case 0x04:
		case 0x06:
		case 0x09:
		case 0x15:
		case 0x21:
		case 0x40:
		case 0x44:
		case 0x53:
		default:
			idecdrom_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = EIO;
			break;
		}
		break;
	case DK_ILLEGAL_REQUEST:
		if (cmd_ptr->subtype == DK_START) {
			/*
			 * if this was a start unit command that received 
			 * an illegal request, assume it is some  
			 * OEM device that doesn't support start units.
			 * Complete the start unit as good and bypass to 
			 * the next command in the reset sequence.
			 */
			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			idecdrom_free_cmd(cmd_ptr);
			idecdrom_prevent_allow(diskinfo, (uchar) DK_PREVENT);
			return;
		}
                if (cmd_ptr->subtype == DK_READ_INFO) {
                        /*
                         * if this was a read disk info command that received 
                         * an illegal request, assume it is a  
                         * IDE device that doesn't support this command.
                         * Complete the start unit as good and bypass to 
                         * the next command in the reset sequence.
                         */
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                        idecdrom_free_cmd(cmd_ptr);
			
			/*
			 * Since this command is invalid for this
			 * device, let's prevent the driver from
			 * issuing it again until it is unconfigured
			 * and reconfigured.
			 */
			diskinfo->multi_session = FALSE;

			/*
			 * Re-issue mode sense's since the mode sense
			 * data in the sense buffer will have been 
			 * overwritten by the request sense data.
			 */
			diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
			idecdrom_mode_sense(diskinfo);

                        
                        
                        return;
                }

		if ((cmd_ptr->subtype == DK_MSENSE) || 
		    (cmd_ptr->subtype == DK_SELECT)) {
			/*
			 * if this was a mode sense or select that received 
			 * an illegal request, assume it is some strange 
			 * OEM IDE Disk that doesn't support page code 0x3f, 
			 * or some setting that we tried to apply. Complete the
			 * mode sense/or select as good and bypass to the next
			 * command in the reset sequence.
			 */
			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			idecdrom_free_cmd(cmd_ptr);
			idecdrom_read_cap(diskinfo);
			return;
		}

		/* Illegal parameter in a cmd */
		switch (sense_code) {
		case 0x21:
			if (cmd_ptr->retry_count < DK_MAX_RETRY) {

				/* The hardware hits this type error, but is okay on retry with delay*/
				idecdrom_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
					   DK_NOOVERLAY_ERROR, 
					   SC_VALID_SENSE);
				idecdrom_start_watchdog(diskinfo,(ulong)2);
				diskinfo->timer_status |= DK_TIMER_BUSY;
				return;
			}
		case 0x1A:
		case 0x20:
		case 0x24:
		case 0x26:
		case 0x39:
		default:
			idecdrom_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
					   DK_NOOVERLAY_ERROR, 
					   SC_VALID_SENSE);

			/* 
			 * this is most likely due to the media
			 * containing audio tracks i.e. 
			 * audio CDROM 
			 */
			bp->b_error = EIO;
			break;
		}
		break;
	case DK_UNIT_ATTENTION:
		/*
                 * The device has been reset.
                 * We need to reinitialize the device through a RESET
                 * cmd cycle and then retry the operation.
                 * The watchdog timer is set for 2 secs. to allow the
                 * device to recover from the reset if this is a retry.
                 */
		if (sense_code == 0x28) {

			/*
			 * Media may have changed.
			 */
			bp->b_error = ESTALE;
			break;
		}
                else if ((sense_code == 0x5a) && (sense_qualifier == 01)) {
                        /*
                         * User has tried to eject media when media
                         * removal is prevented.  Let commmand be retried.
                         */
			bp->b_error = EIO;
                        break;
                }
		if (diskinfo->reset_count < DK_MAX_RESETS) {
			if (!(cmd_ptr->type & DK_RESET) ||
			    (cmd_ptr->subtype == DK_TUR)) {

				idecdrom_start_unit(diskinfo, (uchar) DK_START);
				return;
			}
			else if (cmd_ptr->retry_count < DK_MAX_RETRY) {

				idecdrom_start_watchdog(diskinfo,(ulong)2);
				return;
			}
		}
		break;
	case DK_DATA_PROTECT:
		/* Device is read-only */
		idecdrom_build_error(log_cmd_ptr, DK_MEDIA_ERR, 
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EWRPROTECT;
		break;
	case DK_ABORTED_COMMAND:
		/* The device aborted the cmd. May be able to recover by */
		/* trying the command again. */
		switch (sense_code) {
		case 0x00:
		case 0xB9:
		case 0xBA:
		case 0xBF:
		default:
			idecdrom_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			/* Mark Error in buf for processing */
			bp->b_error = EIO;
		}
		break;
	case DK_MISCOMPARE:
		/*
                 *  report a hard error.
                 */
		idecdrom_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
		break;
	default:
		/* Process unknown sense key */
		idecdrom_build_error(log_cmd_ptr, DK_UNKNOWN_ERR, 
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
	}

	switch (cmd_ptr->type) {
	case DK_BUF:
		idecdrom_process_buf_error(cmd_ptr);
		break;
	case DK_IOCTL:
		idecdrom_process_ioctl_error(cmd_ptr);
		break;
	case DK_RESET:
		idecdrom_process_reset_error(cmd_ptr);
		break;
	case DK_REQSNS:
		idecdrom_process_reqsns_error(cmd_ptr);
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}


}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine examines the return status of operations	*/
/*		that have been processed by the adapter driver and	*/
/*		have been returned in an error state.			*/
/*		This routine must examine the                   	*/
/*		IDE status, if available, and do any logging		*/
/*		that may be required. Also must determine what error	*/
/*		recovery procedure should be followed.			*/
/*			Retry operation					*/
/*			Abort operation					*/
/*			start diagnostic operation			*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_process_error(
struct dk_cmd	*cmd_ptr)
{
	struct ataide_buf	*ataidebuf;



	/* Set up misc pointers for structures */
	ataidebuf = &(cmd_ptr->ataidebuf);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(error, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr ,(uint)cmd_ptr->type, (uint)cmd_ptr->subtype,
	      (uint)ataidebuf->status_validity);
#endif	
#endif
	cmd_ptr->diskinfo->restart_unit = TRUE;
	cmd_ptr->status |= DK_RETRY;
	/*
	 * Since this command has an error lets put it on the head of the cmd_Q
	 * for possible retries.
	 */

	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	if (ataidebuf->status_validity & ATA_ERROR_VALID) {
		/* Process IDE error condition */
		idecdrom_process_ide_error(cmd_ptr);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(error, exit,(char)2, (uint)0, (uint)0 ,(uint)0, (uint)0,(uint)0);
#endif	
#endif

	}
	else {
		/*  The adapter driver returned a resid with no error. */
		idecdrom_process_adapter_error(cmd_ptr);
	}

	/*
         * Please note that when this routine exits, whatever
         * is on the top of the device's cmd queue will be started.
         * Falling through this routine will cause a retry of the
         * operation that failed.
         */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(error, exit,(char)3, (uint)0, (uint)0 ,(uint)0, (uint)0,(uint)0);
#endif	
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_adapter_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes IDE requests that have failed	*/
/*		due to an error at the adapter. This routine will log	*/
/*		the specific error in the system error log, and attempt	*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_adapter_error(
struct dk_cmd	*cmd_ptr)
{
	struct ataide_buf	*ataidebuf;
	struct buf	*bp;
	struct idecdrom_diskinfo	*diskinfo;
	int		max_retries;
	uchar		stack = FALSE;


	/* Set up misc pointers for structures */
	ataidebuf = &(cmd_ptr->ataidebuf);
	bp = &(ataidebuf->bufstruct);
	diskinfo = cmd_ptr->diskinfo;
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(adaperror, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr ,(uint)cmd_ptr->type, 
	      (uint)ataidebuf->status_validity,(uint)0);
#endif	
#endif
	if (bp->b_error == ENXIO) {

			
		if (cmd_ptr->type & (DK_REQSNS|DK_RESET)) {
			/*
			 * If this is a request sense or reset 
			 * operation then clear the state flag of this 
			 * disk accordingly.  NOTE: The cmd_ptr->type
			 * defines match the corresponding 
			 * diskinfo->state defines.
			 */
			stack = TRUE;
			cmd_ptr->diskinfo->state &= ~cmd_ptr->type;
		}


		/*
		 * Maximum number of retries for ENXIO is 2 times
		 * the queue_depth
		 */
		max_retries = cmd_ptr->diskinfo->queue_depth << 1; 

		if (cmd_ptr->retry_count >= max_retries) {
			idecdrom_build_error(cmd_ptr, 0,
					   DK_NOOVERLAY_ERROR, 0);
			bp->b_error = EBUSY;
		}
		else {

			if (!stack) {

				/*
				 * This command needs to be place at the
				 * tail of the command q since it is not
				 * a high priority command
				 */
				idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
				idecdrom_q_cmd(cmd_ptr,(char) DK_QUEUE,								      (uchar) DK_CMD_Q);
				
			}
			
			/*
			 * Retry Command.  Since it is on the 
			 * command q there is nothing  new to do here.
			 */

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(adaperror, trc,(char)0, (uint)bp->b_error, (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif	
#endif		
			return;
		}
	}
	else {
		/* 
		 * No error was reported, but the b_resid value was set. 
		 */
		idecdrom_build_error(cmd_ptr, DK_UNKNOWN_ERR,
		    DK_NOOVERLAY_ERROR, 0);
		bp->b_error = EIO;
	}

	/*
	 * No matter which error case we hit, the b_resid field should
	 * be set to the full count.  If not, a coalesced command will not
	 * be handled properly (we will assume that some of the data
	 * transfered successfully, when in fact there is no way to tell).
	 */

	bp->b_resid = bp->b_bcount;

	/*
         * Now we handoff to the cmd type specific error processor
         * for preparing the cmd for a retry attempt or aborting
         * the cmd back to the requestor.
         */

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(adaperror, trc,(char)1, (uint)bp, (uint)bp->b_error ,
	      (uint)bp->b_resid, (uint)cmd_ptr->type,(uint)0);
#endif	
#endif
	switch (cmd_ptr->type) {
	case DK_BUF:
		idecdrom_process_buf_error(cmd_ptr);
		break;
	case DK_IOCTL:
		idecdrom_process_ioctl_error(cmd_ptr);
		break;
	case DK_RESET:
		idecdrom_process_reset_error(cmd_ptr);
		break;
	case DK_REQSNS:
		idecdrom_process_reqsns_error(cmd_ptr);
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(adaperror, exit,(char)0, (uint)0, (uint)0 ,(uint)0, (uint)0,
	      (uint)0);
#endif	
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_ide_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes IDE requests that have failed	*/
/*		due to an error on the IDE Bus. This routine will log	*/
/*		the specific error in the system error log, and attempt	*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_ide_error(
struct dk_cmd	*cmd_ptr)
{
	struct ataide_buf		*ataidebuf;
	struct buf		*bp;
	struct idecdrom_diskinfo	*diskinfo;
	uchar			ata_status;
	int			max_retries;
	uchar			stack = FALSE;

	/* Set up misc pointers for structures */
	ataidebuf = &(cmd_ptr->ataidebuf);
	bp = &(ataidebuf->bufstruct);
	diskinfo = cmd_ptr->diskinfo;
	ata_status = ataidebuf->ata.status;
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(ideerror, entry,(char)0, (uint)cmd_ptr->type, 
	      (uint)ata_status,(uint)diskinfo->checked_cmd, (uint)0,(uint)0);
#endif	
#endif	
	if (ata_status & ATA_BUSY) {
		idecdrom_build_error(cmd_ptr, DK_HARDWARE_ERR,
		    DK_NOOVERLAY_ERROR, 0);
		bp->b_error = EBUSY;
	} else if (ata_status & ATA_ERROR) {
		if (cmd_ptr->type == DK_REQSNS) {
			/* Check condition on Reqest Sns is a Fatal error */
			/* Log it as such then retry original operation */

			idecdrom_build_error(cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, 0);
		} else {
			/* Do a Request Sense operation */

			
			if (cmd_ptr->type & (DK_RESET)) {
				/*
				 * If this is a reset 
				 * operation then clear the state flag of this 
				 * disk accordingly.  NOTE: The cmd_ptr->type
				 * defines match the corresponding 
				 * diskinfo->state defines.
				 */
				diskinfo->state &= ~cmd_ptr->type;
			}

			idecdrom_request_sense(cmd_ptr->diskinfo,cmd_ptr);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(ideerror, exit,(char)0, (uint)diskinfo->state, (uint)0,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif
			return;
		}
	} else {
		bp->b_error = EIO;
	}

	/*
	 * No matter which error case we hit, the b_resid field should
	 * be set to the full count.  If not, a coalesced command will not
	 * be handled properly (we will assume that some of the data
	 * transfered successfully, when in fact there is no way to tell).
	 */

	bp->b_resid = bp->b_bcount;

	/*
         * Now we handoff to the cmd type specific error processor
         * for preparing the cmd for a retry attempt or aborting
         * the cmd back to the requestor.
         */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(ideerror, trc,(char)1, (uint)bp, (uint)bp->b_error ,
	      (uint)bp->b_resid, (uint)cmd_ptr->type,(uint)diskinfo->state);
#endif	
#endif
	switch (cmd_ptr->type) {
	case DK_BUF:
		idecdrom_process_buf_error(cmd_ptr);
		break;
	case DK_IOCTL:
		idecdrom_process_ioctl_error(cmd_ptr);
		break;
	case DK_RESET:
		idecdrom_process_reset_error(cmd_ptr);
		/*
                 *  If the device was busy we want to timeout for 2 secs before
                 *  we retry the command.  This is the 
                 *  case when the cmd_ptr is still on top of the stack, and 
		 *  the status is busy.
                 */
		if ((diskinfo->retry_flag)&&(ata_status & ATA_BUSY)) {

			idecdrom_start_watchdog(diskinfo,(ulong)2);
			diskinfo->timer_status |= DK_TIMER_BUSY;
		}
		break;
	case DK_REQSNS:
		idecdrom_process_reqsns_error(cmd_ptr);
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(ideerror, exit,(char)1, (uint)diskinfo->state, (uint)0,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif	

}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_buf_error				*/
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
/* 									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_buf_error(
struct dk_cmd	*cmd_ptr)
{
	struct ataide_buf		*ataidebuf;
	struct idecdrom_diskinfo	*diskinfo;
	struct buf		*bp;
	uint			remaining_count;



	/* Set up misc pointers for structures */
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;

	ataidebuf = &(cmd_ptr->ataidebuf);
	bp = &(ataidebuf->bufstruct);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(failbuf, entry,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
	/* Check if cmd is single operation */
	if (cmd_ptr->group_type == DK_SINGLE) {
		/* Operation is a Single operation */
		if (cmd_ptr->retry_count >= DK_MAX_RETRY) {
			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idecdrom_log_error(cmd_ptr, DK_HARD_ERROR);
			}

			/* Remove op from device's cmd stack */
			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);


			/* flush request from IN_PROGRESS queue */

			if ((cmd_ptr->error_type == 0) && 
			    (bp->b_resid == 0)) {
				/* 
				 * If this was something that we did not log 
				 * and which completely transferred then
				 * make sure that b_error = 0
				 */
				cmd_ptr->bp->b_error = 0;
				cmd_ptr->bp->b_flags &= ~B_ERROR;
			}
			else {
				cmd_ptr->bp->b_flags |= B_ERROR;
				cmd_ptr->bp->b_error = bp->b_error;
				cmd_ptr->bp->b_resid += bp->b_resid;
			}
			DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE,
			    cmd_ptr->bp->b_error, cmd_ptr->bp->b_dev,
			    cmd_ptr->bp);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(sdiodone, trc,(char)2, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
			iodone(cmd_ptr->bp);
			
			/*
			 * If wa are sending iodones back then clear 
			 * the reset_failure flag, even though this 
			 * is an potential error
			 */
			diskinfo->reset_failures = 0;
			idecdrom_free_cmd(cmd_ptr);
		}
	} else if (cmd_ptr->group_type == DK_COALESCED) {
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
		bp_list = ataidebuf->bp;

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
		 * any successful bp's are in a NULL terminated
		 * list pointed to by ataidebuf->bp.
		 */

		/* Check for any completed requests and process */
		if (last_good_bp != NULL) {
			current_bp = bp_list;
			do {


				/* Return to system as complete */
				current_bp->b_resid = 0;
				current_bp->b_flags &= ~B_ERROR;
				current_bp->b_error = 0;
				DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE,
				    current_bp->b_error, current_bp->b_dev,
				    current_bp);
				prev_bp = current_bp;
				current_bp = current_bp->av_forw;
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(sdiodone, trc,(char)3, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
				iodone(prev_bp);
			
				/*
				 * If wa are sending iodones back then clear 
				 * the reset_failure flag, even though this 
				 * is an potential error
				 */
				diskinfo->reset_failures = 0;
			} while (prev_bp != last_good_bp);
		}





		/*
		 * At this point the bp list has been
		 * split into two lists.
		 * we must set ataidebuf.bp to point at the start of the
		 * list to be retried.
		 */
		ataidebuf->bp = first_bad_bp;
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
			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			idecdrom_free_cmd(cmd_ptr);
			cmd_ptr = 
				idecdrom_build_cmd(diskinfo,
				                (struct buf **) &first_bad_bp,
						 (char) FALSE);

			/*
			 * No resources free so exit this loop
			 */
			ASSERT(cmd_ptr != NULL);

			
			
			cmd_ptr->retry_count = current_retry_count;
			cmd_ptr->error_type = current_error_type;
			idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
			/*
			 * Falling out here will allow retry of
			 * updated operation.
			 */

		} else { /* retrys exceeded */


			/* setup to determine proper residual count */
			total_count = 0;
			current_bp = ataidebuf->bp;
			while (current_bp != NULL) {
				total_count += current_bp->b_bcount;
				current_bp = current_bp->av_forw;
			}


			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idecdrom_log_error(cmd_ptr, DK_HARD_ERROR);
			}





			/* Fail first bp in the chain of bp's  */
			/* and then retry the remaining bp's   */

			first_bad_bp =  first_bad_bp->av_forw;
			/* Set error values and flag bit */


			/*
			 *
			 *
			 *  Example (cont) of a failed transfer with 3 bufs.
			 *
			 *  B              B              
			 *  |-------|--------|--------| = Buf addr boundaries
			 *          ^-----------------^ = total_count
			 *  ^-------^                   = good count
			 *              ^-------------^ = residual
			 *  |XXXXXXX|                   = Blocks returned good
			 *          ^                   = ataidebuf->bp
			 *          ^-------^           =ataidebuf->bp->b_bcount
			 *              ^---^           = ataidebuf->bp->b_resid
			 *                  ^           = first_bad_bp
			 *
			 */


			ataidebuf->bp->b_flags |= B_ERROR;
			ataidebuf->bp->b_error = ataidebuf->bufstruct.b_error;
			ataidebuf->bp->b_resid = (ataidebuf->bp->b_bcount - 
			    (total_count - residual));
			if (ataidebuf->bp->b_resid == 0) {
				/*
				 * If no resid then we will set it to
				 * the entire transfer, since we know
				 * this buf has failed.
				 */
				ataidebuf->bp->b_resid = ataidebuf->bp->b_bcount;
			}

			DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE,
			    ataidebuf->bp->b_error, ataidebuf->bp->b_dev,
			    ataidebuf->bp);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(sdiodone, trc,(char)4, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)ataidebuf->bp->b_error, 
	      (uint)ataidebuf->bp->b_resid,(uint)ataidebuf->bp->b_bcount);
#endif	
#endif	

			iodone(ataidebuf->bp);
			
			/*
			 * If wa are sending iodones back then clear 
			 * the reset_failure flag, even though this 
			 * is an potential error
			 */
			diskinfo->reset_failures = 0;

			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			idecdrom_free_cmd(cmd_ptr);
			/* Check for IN_PROGRESS queue not empty */
			if (first_bad_bp != NULL) {
				/* Build cmd for remaining queue */
				cmd_ptr = 
					idecdrom_build_cmd(diskinfo,
					    (struct buf **) &first_bad_bp,
					    (char) FALSE);
				ASSERT(cmd_ptr != NULL);

				idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,
					     (uchar) DK_CMD_Q);
			}
		}
	} 
	else if (cmd_ptr->group_type == DK_FRAGMENTED) {
		/* Operation is a Fragmented operation */
		struct buf      *current_bp;

		if (cmd_ptr->retry_count >= DK_MAX_RETRY) {
			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				idecdrom_log_error(cmd_ptr, DK_HARD_ERROR);
			}

			/* Remove op from device's cmd stack */
			idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);


			current_bp = diskinfo->fragment.saved_bp;
			if ((cmd_ptr->error_type == 0) && 
			    (bp->b_resid == 0)) {
				/* 
				 * If this was something that we did not log 
				 * and which completely transferred then
				 * make sure that b_error = 0
				 */
				current_bp->b_error = 0;
				current_bp->b_flags &= ~B_ERROR;
			}
			else {
				current_bp->b_flags |= B_ERROR;
				current_bp->b_error = bp->b_error;
				current_bp->b_resid = bp->b_resid +
					diskinfo->fragment.saved_resid;
				if (diskinfo->fragment.issued == DK_FIRST) {
				   if (diskinfo->fragment.middle_len != 0) {
					current_bp->b_resid +=
			   		 diskinfo->fragment.middle_bcount;
				   }
				   if (diskinfo->fragment.last_partial_len !=0){
					current_bp->b_resid +=
			   		 diskinfo->fragment.last_partial_bcount;
				   }
				}
				else if (diskinfo->fragment.issued==DK_MIDDLE) {
				   if (diskinfo->fragment.last_partial_len !=0){
					current_bp->b_resid +=
			   		 diskinfo->fragment.last_partial_bcount;
				   }
				}
			}
			DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE,
			    current_bp->b_error, current_bp->b_dev,
			    current_bp);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(sdiodone, trc,(char)2, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
			/* must iodone the original bp and not a temp */
			iodone(current_bp);
			
			/*
			 * If wa are sending iodones back then clear 
			 * the reset_failure flag, even though this 
			 * is an potential error
			 */
			diskinfo->reset_failures = 0;
			idecdrom_free_cmd(cmd_ptr);
		}
	}

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(failbuf, exit,(char)0, (uint)diskinfo->state, 
	      (uint)0,(uint)0, (uint)0,(uint)0);
#endif	
#endif	

}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_ioctl_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes the failure of an IOCTL type	*/
/*		request. The operation is rescheduled if a retry	*/
/*		is feasible, else it will be marked as failed and	*/
/*		the requesting process will be awakened.		*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_ioctl_error(
struct dk_cmd	*cmd_ptr)
{
	struct idecdrom_diskinfo	*diskinfo;


#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(ioctlerr, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr->retry_count,(uint)cmd_ptr->subtype, 
	      (uint)cmd_ptr->aborted,(uint)0);
#endif	
#endif
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;

	if ((cmd_ptr->retry_count >= DK_MAX_RETRY) ||
	    (cmd_ptr->subtype != DK_ERP_IOCTL) ||
	    (cmd_ptr->aborted)) {

		/*
		 * If maximum number of retries is reached or
		 * if this is an ioctl that does not get error
		 * recovery or if this is an aborted command
		 * then fail it.
		 */
		/* Log error for failure */
		if (cmd_ptr->error_type != 0) {
			idecdrom_log_error(cmd_ptr, DK_HARD_ERROR);
		}

		cmd_ptr->flags &= ~DK_READY;
		idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
		cmd_ptr->intrpt = 0;
		e_wakeup(&(cmd_ptr->ataidebuf.bufstruct.b_event));
			
		/*
		 * If we are sending iodones back then clear 
		 * the reset_failure flag, even though this 
		 * is an error
		 */

		diskinfo->reset_failures = 0;
		diskinfo->ioctl_pending = FALSE;
	}
		
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(ioctlerr, exit,(char)0, (uint)0, (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif	
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_reset_error                            */
/*									*/
/*	FUNCTION:                                                       */
/*		This routine processes the failure of a Reset cycle	*/
/*		type request. A retry is scheduled if feasible, else	*/
/*		the device is marked RST_FAILED, since it could not be	*/
/*		initialized for use.					*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_reset_error(
struct dk_cmd	*cmd_ptr)
{
	struct ataide_buf		*ataidebuf;
	struct idecdrom_diskinfo	*diskinfo;

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reseterr, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr,(uint)cmd_ptr->retry_count, 
	      (uint)cmd_ptr->diskinfo->reset_count,(uint)0);
#endif	
#endif
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;
	diskinfo->state &= ~DK_RESET_PENDING;

	ataidebuf = &(cmd_ptr->ataidebuf);
	if ((cmd_ptr->retry_count >= DK_MAX_RETRY) || 
	    (diskinfo->reset_count >= DK_MAX_RESETS)) {
		diskinfo->retry_flag = FALSE;
		/* Log error for failure */
		if (cmd_ptr->error_type != 0) {
			idecdrom_log_error(cmd_ptr, DK_HARD_ERROR);
		}

		diskinfo->errno = ataidebuf->bufstruct.b_error;
		/*
		 * Note: the following line will also clear DK_RESET_PENDING
		 */
		diskinfo->state = DK_RST_FAILED;
		diskinfo->reset_failures++;
		diskinfo->reset_count = 0;
		idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
		idecdrom_free_cmd(cmd_ptr);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reseterr, trc,(char)0, (uint)diskinfo->state, 
	      (uint)diskinfo->dk_cmd_q_head,(uint)0, (uint)0,(uint)0);
#endif	
#endif
		/* Set to next command on stack     */
		cmd_ptr = diskinfo->dk_cmd_q_head;
		if (cmd_ptr == NULL) {
			diskinfo->disk_intrpt = 0;
			e_wakeup((int *)&(diskinfo->open_event));
			if (diskinfo->reset_failures >= 
			     DK_MAX_RESET_FAILURES) {
				/*
				 * The reset cycle has failed too many
				 * times without any bufs or ioctls getting
				 * good completion. In other words we can't
				 * execute all commands required to have a disk
				 * open for operation.  Therefore we must give
				 * up and fail everything back to prevent a
				 * hang.  This disk is basically useless.
				 */
				diskinfo->reset_failures = 0;
				idecdrom_fail_disk(diskinfo);

			}
		} else if (diskinfo->reset_failures >= DK_MAX_RESET_FAILURES) {
				/*
				 * The reset cycle has failed too many
				 * times without any bufs or ioctls getting
				 * good completion. In other words we can't
				 * execute all commands required to have a disk
				 * open for operation. Therefore we must give
				 * up and fail everything back to prevent a
				 * hang.  This disk is basically useless.
				 */

			diskinfo->reset_failures = 0;	
			idecdrom_fail_disk(diskinfo);

			
		} else {
			cmd_ptr->retry_count = DK_MAX_RETRY;

			/* 
			 *  Set up the errno and resid in the ataidebuf for
			 *  this command.  Since the reset is the command
			 *  that failed, this information has not been set
			 *  up in the original ataidebuf, and the error routines
			 *  will be using it.
			 */

			cmd_ptr->ataidebuf.bufstruct.b_error = diskinfo->errno;
			cmd_ptr->ataidebuf.bufstruct.b_resid =
			    cmd_ptr->ataidebuf.bufstruct.b_bcount;

			/* Call the cmd specific error processor */
			switch (cmd_ptr->type) {
			case DK_BUF:
				idecdrom_process_buf_error(cmd_ptr);
				break;
			case DK_IOCTL:
				idecdrom_process_ioctl_error(cmd_ptr);
				break;
			case DK_RESET:
				idecdrom_process_reset_error(cmd_ptr);
				break;
			case DK_REQSNS:
				idecdrom_process_reqsns_error(cmd_ptr);
				break;
			default:
				/* Unknown Cmd type */
				ASSERT(FALSE);
			}
		}

	}
	else {
		diskinfo->retry_flag = TRUE;
	}

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reseterr, exit,(char)0, (uint)0, (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif	
#endif
}


/************************************************************************/
/*									*/
/*	NAME:	idecdrom_fail_disk		                        */
/*									*/
/*	FUNCTION:                                                       */
/*		This routine is called when it is impossible to		*/
/*		complete the necessary reset cycle for valid disk       */
/*		operations.  When this situation occurs we have no      */
/*		other option but to fail all pending commands for this  */
/*		disk and give up.  At least we will not causes a hang.  */
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup	iodone					*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_fail_disk(struct idecdrom_diskinfo *diskinfo)
{
        struct buf *next_bp,                     /* Next buf in chain      */
		   *tmp_bp;                      /* temporary buf pointer  */
        struct dk_cmd  *cmd_ptr;		 /* command block structure*/
	int	errnoval=EIO;


	if (diskinfo->errno) {
		errnoval = diskinfo->errno;
	}

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
        idecdrom_trc(faildisk, entry,(char)0,(uint)diskinfo, 
		   (uint)diskinfo->state, 
		   (uint)0, (uint)0,(uint)0);

#endif
#endif


	/*
	 * Set disk to DK_OFFLINE so no new commands will not be allowed  
	 * until we have flushed all pending commands back.
	 */
	diskinfo->state = DK_OFFLINE;
        while (diskinfo->dk_cmd_q_head != NULL) {
                /*
                 * while there are commands on this disks
                 * command queue, fail them all.
                 */
                cmd_ptr = diskinfo->dk_cmd_q_head;
		cmd_ptr->retry_count = DK_MAX_RETRY;
		cmd_ptr->ataidebuf.bufstruct.b_error = errnoval;
		cmd_ptr->ataidebuf.bufstruct.b_resid = cmd_ptr->ataidebuf.
			bufstruct.b_bcount;
		
		/* Call the cmd specific error processor */
		switch (cmd_ptr->type) {
		case DK_BUF:
			idecdrom_process_buf_error(cmd_ptr);
			break;
		case DK_IOCTL:
			idecdrom_process_ioctl_error(cmd_ptr);
			break;
		case DK_RESET:
			cmd_ptr->ataidebuf.bufstruct.b_flags |= B_DONE;
			idecdrom_process_reset_error(cmd_ptr);
			break;
		case DK_REQSNS:
			idecdrom_process_reqsns_error(cmd_ptr);
			break;
		default:
			/* Unknown Cmd type */
			ASSERT(FALSE);
		}
        }
        if (diskinfo->ioctl_cmd.flags & DK_READY) {
                /*
                 * If there is a pending ioctl then
		 * fail it.
                 */

                cmd_ptr = &diskinfo->ioctl_cmd;
		cmd_ptr->flags &= ~DK_READY;
		cmd_ptr->intrpt = 0;	
		cmd_ptr->ataidebuf.bufstruct.b_error = errnoval;
		e_wakeup(&(cmd_ptr->ataidebuf.bufstruct.b_event));
        }


	if (diskinfo->dk_bp_queue.in_progress.head != NULL) {
		/*
		 * Fail any bufs in the in_progress queue
		 */

		tmp_bp = diskinfo->dk_bp_queue.in_progress.head;
		while (tmp_bp != NULL) {
			tmp_bp->b_flags |= B_ERROR;
			tmp_bp->b_error = errnoval;
			tmp_bp->b_resid = tmp_bp->b_bcount;
			next_bp = tmp_bp->av_forw;


#ifdef DEBUG
#ifdef IDE_ERROR_PATH

			idecdrom_trc(sdiodone, trc,(char)0, 
				   (uint)0, 
				   (uint)0,(uint)tmp_bp->b_error, 
				   (uint)tmp_bp->b_resid,
				   (uint)tmp_bp->b_bcount);

#endif
#endif
			iodone(tmp_bp);
			tmp_bp = next_bp;
		}
		diskinfo->dk_bp_queue.in_progress.head = NULL;
		diskinfo->dk_bp_queue.in_progress.tail = NULL;
	}
        tmp_bp = diskinfo->low;
        while( tmp_bp != NULL) {
                /*
                 * while there are bufs in this disks elevator
		 * fail them all.
                 */
		tmp_bp->b_error = errnoval;
		tmp_bp->b_flags |= B_ERROR;
		tmp_bp->b_resid = tmp_bp->b_bcount;
		next_bp = tmp_bp->av_forw;
		iodone(tmp_bp);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH

		idecdrom_trc(sdiodone, trc,(char)1, 
			   (uint)0, 
			   (uint)0,(uint)tmp_bp->b_error, 
			   (uint)tmp_bp->b_resid,(uint)tmp_bp->b_bcount);
#endif
#endif

		tmp_bp = next_bp;
	}

        diskinfo->currbuf = diskinfo->low = NULL;
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
        idecdrom_trc(faildisk, exit,(char)0,(uint)diskinfo, 
		   (uint)diskinfo->state, 
	       (uint)0, (uint)0,(uint)0);

#endif
#endif
	diskinfo->state = DK_RST_FAILED;
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_reqsns_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes the failure of a Request Sense	*/
/*		operation. A retry is scheduled on this operation if	*/
/*		feasible, else the next operation on the stack is	*/
/*		scheduled for a retry.					*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
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
/*									*/
/************************************************************************/

void
idecdrom_process_reqsns_error(
struct dk_cmd	*cmd_ptr)
{
	struct idecdrom_diskinfo	*diskinfo;



	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;

	/* Log error for failure */
	if (cmd_ptr->error_type != 0) {
		idecdrom_log_error(cmd_ptr, DK_HARD_ERROR);
	}
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reqsnserror, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)cmd_ptr,(uint)0);
#endif	
#endif
	/*
	 * Remove reqsns op from stack and allow original
	 * Operation to retry...
	 */
	idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
	idecdrom_free_cmd(cmd_ptr);
	diskinfo->state &= ~DK_REQSNS_PENDING;
	/*
	 * Get command that originally received check condition
	 * and put it at the top of cmd_q
	 */
	cmd_ptr = diskinfo->checked_cmd;

	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reqsnserror, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)cmd_ptr,(uint)cmd_ptr->type);
#endif	
#endif
	/* Call the cmd specific error processor */
	switch (cmd_ptr->type) {
	      case DK_BUF:
		idecdrom_process_buf_error(cmd_ptr);
		break;
	      case DK_IOCTL:
		idecdrom_process_ioctl_error(cmd_ptr);
		break;
	      case DK_RESET:
		idecdrom_process_reset_error(cmd_ptr);
		break;
	      case DK_REQSNS:
		idecdrom_process_reqsns_error(cmd_ptr);
		break;
	      default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}


	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reqsnserror, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)0,(uint)0);
#endif	
#endif

}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_buf					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine performs completion processing on		*/
/*		Buf related requests that have been received by		*/
/*		idecdrom_iodone with good return status.		*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		iodone							*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_process_buf(
struct dk_cmd	*cmd_ptr)
{
	struct idecdrom_diskinfo	*diskinfo;
	struct ataide_buf		*ataidebuf;
	struct buf		*curbp, *nextbp;
	uint			total_count;
        int                     rc;
        int                     blk_cnt, lba, partial_count, block_number;
        struct ata_cmd          *ata;
        uchar                   user_data_type; /* Expected user data type */
        uchar                   flag_bits;      /* For M2F2 subheader      */
        uchar                   cyl_hi;         /* Sector size hi byte     */
        uchar                   cyl_lo;         /* Sector size lo byte     */



	/* Get diskinfo struct for device */
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;

	diskinfo->reset_failures = 0;

	/* Set up misc pointers for structs */
	ataidebuf = &(cmd_ptr->ataidebuf);

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(processbuf, entry,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)0, (uint)0,(uint)0);
#endif	
#endif	
	/* Check for a recovered error during processing, log any */
	if (cmd_ptr->error_type != 0) {
		idecdrom_log_error(cmd_ptr, DK_SOFT_ERROR);
	}

	if (cmd_ptr->group_type == DK_SINGLE) {
		/* Cmd is single operation */
		curbp = cmd_ptr->bp;
		/*
		 * Check the soft_resid variable to see if this command was
		 * recovered with retries and needs to be relocated by
		 * reporting an ESOFT.
		 */
		if (cmd_ptr->soft_resid) {
			curbp->b_flags |= B_ERROR;
			curbp->b_error = ESOFT;
			curbp->b_resid = cmd_ptr->soft_resid;
		} else {
			curbp->b_flags &= ~B_ERROR;
			curbp->b_error = 0;
		}

		DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE, curbp->b_error,
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
   idecdrom_trc(sdiodone, trc,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)curbp->b_error, 
	      (uint)curbp->b_resid,(uint)curbp->b_bcount);
#endif	
#endif	
		iodone(curbp);


		idecdrom_free_cmd(cmd_ptr);

	} else if (cmd_ptr->group_type == DK_COALESCED) {
		/*
		 * Cmd is coalesced operation
		 * Flush IN_PROGRESS queue
		 */
		curbp = ataidebuf->bp;
		total_count = ataidebuf->bufstruct.b_bcount;
		while (curbp != NULL) {
			total_count -= curbp->b_bcount;
			/*
			 *  Check the soft_resid variable to see if this is
			 *  a recovered media error that should be relocated
			 *  by reporting ESOFT.
			 */
			if (cmd_ptr->soft_resid > total_count) {
				curbp->b_flags |= B_ERROR;
				curbp->b_error = ESOFT;
				curbp->b_resid = cmd_ptr->soft_resid - 
				    total_count;
				cmd_ptr->soft_resid = 0;
			} else {
				curbp->b_flags &= ~B_ERROR;
				curbp->b_error = 0;
				curbp->b_resid = 0;
			}

			nextbp = curbp->av_forw;
			DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE,curbp->b_error,
			    curbp->b_dev, curbp);
			if (curbp->b_flags & B_READ) {
				diskinfo->dkstat.dk_rblks +=
				    (curbp->b_bcount) / diskinfo->block_size;
			} else {
				diskinfo->dkstat.dk_wblks +=
				    (curbp->b_bcount) / diskinfo->block_size;
			}
			iodone(curbp);
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(sdiodone, trc,(char)1, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)curbp->b_error, 
	      (uint)curbp->b_resid,(uint)curbp->b_bcount);
#endif	
#endif
			curbp = nextbp;
		}

		idecdrom_free_cmd(cmd_ptr);
	} else {
		/* Must be a fragmented operation */
		curbp = diskinfo->fragment.saved_bp;
		if (((diskinfo->fragment.issued == DK_FIRST) &&
		    ((diskinfo->fragment.middle_len != 0) ||
		     (diskinfo->fragment.last_partial_len != 0))) ||

		    ((diskinfo->fragment.issued == DK_MIDDLE) &&
		      (diskinfo->fragment.last_partial_len != 0))) {

			/* Must issue a middle or last fragment */
			if (diskinfo->fragment.issued == DK_FIRST) {	
				/* process first */
				/* move to user buffer */
				rc = xmemout((caddr_t)
				   (diskinfo->fragment.first_partial_baddr),
				   curbp->b_un.b_addr,
			   	   diskinfo->fragment.first_partial_bcount,
				   &(curbp->b_xmemd));
		    		if (diskinfo->fragment.middle_len != 0) {
					/* issue middle */
	      			   bcopy((caddr_t) &(curbp->b_xmemd), (caddr_t)
	     	 		   &(cmd_ptr->ataidebuf.bufstruct.b_xmemd),
	     	 		   sizeof(struct xmem));
		    			diskinfo->fragment.issued = DK_MIDDLE;
					ataidebuf->bufstruct.b_bcount = 
			   	   	   diskinfo->fragment.middle_bcount;
				     	ataidebuf->bufstruct.b_un.b_addr =
				           (caddr_t)
			   	   	   diskinfo->fragment.middle_baddr;
					ataidebuf->bufstruct.b_blkno = 
				     	   diskinfo->fragment.middle_blkno;
					lba = ataidebuf->bufstruct.b_blkno; 
				     	blk_cnt = diskinfo->fragment.middle_len;
				} else {
					/* issue last */
	     	 		   diskinfo->fragment.temp_raw_bp.bp.b_xmemd.aspace_id = XMEM_GLOBAL;
	     	 		   cmd_ptr->ataidebuf.bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
		    			diskinfo->fragment.issued = DK_LAST;
					ataidebuf->bufstruct.b_bcount = 
					 DK_BLOCKSIZE*diskinfo->mult_of_blksize;
				     	ataidebuf->bufstruct.b_un.b_addr =
				       	 (caddr_t)diskinfo->fragment.buffer;
					ataidebuf->bufstruct.b_blkno = 
				     	 diskinfo->fragment.last_partial_blkno;
					lba = ataidebuf->bufstruct.b_blkno; 
				     	blk_cnt = 1;
				}
			} else {
				/* process middle, nothing to copy */
				/* issue last */
	     	 		   diskinfo->fragment.temp_raw_bp.bp.b_xmemd.aspace_id = XMEM_GLOBAL;
	     	 		   cmd_ptr->ataidebuf.bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
		    		diskinfo->fragment.issued = DK_LAST;
				ataidebuf->bufstruct.b_bcount = 
				 DK_BLOCKSIZE*diskinfo->mult_of_blksize;
				ataidebuf->bufstruct.b_un.b_addr =
				  (caddr_t)diskinfo->fragment.buffer;
				ataidebuf->bufstruct.b_blkno = 
				   diskinfo->fragment.last_partial_blkno;
				lba = ataidebuf->bufstruct.b_blkno; 
				blk_cnt = 1;
			}
			/* Update remaining cmd block for next operation */

			ata = &(cmd_ptr->ataidebuf.ata);
			if (diskinfo->current_cd_mode == CD_DA) {
				user_data_type = 0x04;
				flag_bits = 0x10; /* get user data only */
				cyl_hi = 0x09;
				cyl_lo = 0x30;
			}	
			else if (diskinfo->current_cd_mode == CD_MODE2_FORM2) {
				user_data_type = 0x14;
				flag_bits = 0x50;/*get user data and subheader*/
				cyl_hi = 0x09;
				cyl_lo = 0x20;
			}
			else if (diskinfo->current_cd_mode == CD_MODE2_FORM1) {
				user_data_type = 0x10;
				flag_bits = 0x10; /* get user data only */
				cyl_hi = 0x10;
				cyl_lo = 0x00;
			}
			else {
				user_data_type = 0x08; /*Mode 1 */
				flag_bits = 0x10; /* get user data only */
				cyl_hi = 0x10;
				cyl_lo = 0x00;
			}
			ata->startblk.chs.cyl_hi = cyl_hi;
			ata->startblk.chs.cyl_lo = cyl_lo;
	  		ata->atapi.length = 12;
			ata->command = ATA_ATAPI_PACKET_COMMAND;
			ata->device = diskinfo->device_id;
			ata->feature = 0x00;
	
			ata->atapi.packet.op_code = ATAPI_READ_CD;
			ata->atapi.packet.bytes[0] = (0x1C & user_data_type);
			ata->atapi.packet.bytes[1] = ((lba >> 24) & 0xff);
			ata->atapi.packet.bytes[2] = ((lba >> 16) & 0xff);
			ata->atapi.packet.bytes[3] = ((lba >> 8) & 0xff);
			ata->atapi.packet.bytes[4] = (lba & 0xff);
			ata->atapi.packet.bytes[5] = ((blk_cnt >> 16) & 0xff);
			ata->atapi.packet.bytes[6] = ((blk_cnt >> 8) & 0xff);
			ata->atapi.packet.bytes[7] = (blk_cnt & 0xff);
			ata->atapi.packet.bytes[8] = flag_bits;
			/* Sub-Channel Data Selection Bits */
			ata->atapi.packet.bytes[9] = 0x00;
			ata->atapi.packet.bytes[10] = 0x00;

			/* Place cmd on device's operation stack */
			idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar)DK_CMD_Q);
	
		} else {
			/* Nothing left to issue so process completion */
			if (diskinfo->fragment.issued == DK_FIRST) {	
				/* process first */
				/* move to user buffer */
				rc = xmemout((caddr_t)
				   (diskinfo->fragment.first_partial_baddr),
				   curbp->b_un.b_addr,
			   	   diskinfo->fragment.first_partial_bcount,
				   &(curbp->b_xmemd));
			}
			else if (diskinfo->fragment.issued == DK_MIDDLE) {	
				/* process middle, nothing to copy */
			}
			else {
				/* process last */
				/* move to user buffer */
			      rc = xmemout((caddr_t)(diskinfo->fragment.buffer),
			       	   diskinfo->fragment.last_partial_baddr,
			   	   diskinfo->fragment.last_partial_bcount,
				   &(curbp->b_xmemd));
			}
			/* iodone entire read operation */
			curbp->b_flags &= ~B_ERROR;
			curbp->b_error = 0;
			curbp->b_resid = 0;
			if (diskinfo->fragment.saved_resid != 0) {
				curbp->b_resid = diskinfo->fragment.saved_resid;
			}

			DDHKWD2(HKWD_DD_IDECDROMDD, DD_SC_IODONE,curbp->b_error,
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
   idecdrom_trc(sdiodone, trc,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)curbp->b_error, 
	      (uint)curbp->b_resid,(uint)curbp->b_bcount);
#endif	
#endif	
		 	iodone(curbp);

			idecdrom_free_cmd(cmd_ptr);

		}
	}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(processbuf, exit,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)0,(uint)0, (uint)0,(uint)0);
#endif	
#endif

}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_process_reset					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine handles completion processing of		*/
/*		RESET type commands that have been received by		*/
/*		idecdrom_iodone with successful completion status.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup	bcopy					*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_process_reset(
struct dk_cmd	*cmd_ptr)
{
	struct idecdrom_diskinfo	*diskinfo;
	int	minutes,seconds,frames;
	int	bcd_minutes,bcd_seconds,bcd_frames;




	/* Get diskinfo struct for device */
	diskinfo = cmd_ptr->diskinfo;

	/*
	 * Determine what cmd, if any, is next in the cycle
	 * and set it up for starting on exit
	 */
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(processreset, entry,(char)0, (uint)cmd_ptr->diskinfo, 
	      (uint)cmd_ptr->subtype,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)0,(uint)0);
#endif	
#endif
	switch (cmd_ptr->subtype) {
	case DK_START:
		/* A START UNIT cmd has returned */

		idecdrom_free_cmd(cmd_ptr);
		idecdrom_test_unit_ready(diskinfo);
		break;
	case DK_TUR:
		/* A TEST UNIT READY cmd has returned */
		idecdrom_free_cmd(cmd_ptr);

		if (diskinfo->prevent_eject) {
		 	/*
		     	 * If prevent_eject is true then we must
		      	 * issue a prevent media removal command next.
		      	 */
		    	idecdrom_prevent_allow(diskinfo,
					     (uchar) DK_PREVENT);
		}
		else {
			/*	
			 * If prevent_eject is false then we will	
			 * skip prevent media removal command and 
			 * go on to mode sense command.
			 */
			diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
			idecdrom_mode_sense(diskinfo);
		}
		break;

	case DK_PREVENT:
		/* A Prevent Media removal cmd has returned */

		idecdrom_free_cmd(cmd_ptr);
		diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
		idecdrom_mode_sense(diskinfo);


		break;
	case DK_MSENSE:
		/* A MODE SENSE cmd has returned */

		idecdrom_free_cmd(cmd_ptr);

		if (diskinfo->m_sense_status == DK_SENSE_CHANGEABLE) {
			/*
			 * if this was the sense of changeable 
			 * attributes, then prepare to sense current
			 * attributes. First, copy changeable data
			 * into separate data buffer, and format the
			 * data
			 */
        		bcopy (diskinfo->sense_buf, diskinfo->ch_data, 256);
			idecdrom_format_mode_data((char *) diskinfo->ch_data, 
				(struct idecdrom_mode_format *)&diskinfo->ch, 
				(int) ((*((ushort *)(diskinfo->ch_data)))),
				(char) FALSE,
				(struct idecdrom_diskinfo *)NULL);
			diskinfo->m_sense_status = DK_SENSE_CURRENT;
                       	idecdrom_mode_sense(diskinfo);
		} else {
			/*
			 * this was the sense of current data, so
			 * format the data and then determine if
			 * a select is necessary
			 */
			idecdrom_format_mode_data((char *) diskinfo->sense_buf, 
				  (struct idecdrom_mode_format *) &diskinfo->cd,
				  (int)((*((ushort *)(diskinfo->sense_buf)))),
				  (char) FALSE,
				  (struct idecdrom_diskinfo *) NULL);

                        if ((diskinfo->multi_session) && 
			    (diskinfo->cd_mode2_form1_flag == TRUE) &&
			    (!(diskinfo->ioctl_chg_mode_flg) ||
			     (diskinfo->current_cd_mode == CD_MODE2_FORM1))) {
                                /*
                                 * If this drive supports multi-session CDs
				 * and CD-ROM XA Data Mode 2 Form1 is supported
				 * and we are not doing a change mode
				 * ioctl to a CD-ROM Data mode other then
				 * Mode 2 Form 1, then we need to determine 
				 * what data mode the CD is in.
                                 */
                		diskinfo->disc_info = 0x01;
                                idecdrom_read_disc_info(diskinfo);
                        }
                        else {
                                /*
                                 * If this is drive does not support
                                 * multi-session CDs then see if a 
                                 * mode select is necessary
                                 */
				diskinfo->last_ses_pvd_lba = 0;
                                if (idecdrom_mode_data_compare(diskinfo)) {
                                        /*
                                         * if a mode select is necessary
                                         */
                                        idecdrom_mode_select(diskinfo);
                                }
                                else {
                                        /*
                                         * bypass the mode select
                                         */
                                        idecdrom_read_cap(diskinfo);
                                }
                        }

		}
		break;
        case DK_READ_INFO:

                idecdrom_free_cmd(cmd_ptr);       
                if (diskinfo->disc_info == 0x01) {
		   /* Finished TOC read with format '00'b since no cache */
                   diskinfo->disc_info = 0x02;
		   /* TOC read with format '10'b for disc type */
                   idecdrom_read_disc_info(diskinfo);
		}

                else if (diskinfo->disc_info == 0x02) {
                   diskinfo->disc_info = 0xFF; /*assume this is last TOC read*/
		   /* check for XA disc */
                   if (diskinfo->ioctl_buf[13] == 0x20) {
                       	  /*
                           * This disk is a CD-ROM XA disc that
                           * may be a photo CD.
                           */
                       	   diskinfo->current_cd_mode = CD_MODE2_FORM1;
                       	   diskinfo->block_size = DK_M2F1_BLKSIZE;
                       	   diskinfo->mult_of_blksize = 
                               	diskinfo->block_size/DK_BLOCKSIZE;

                	   if (diskinfo->ioctl_buf[2] ==
                	                 diskinfo->ioctl_buf[3]) {
                      		/*
                         	 * First session == Last session
                       		 *
                         	 * This indicates one of the following
                       		 * situations:
                       		 * 1. The media is not a multi-session
                       		 *    photo CD.
                       		 *
                       		 * 2. The media has only one session on it.
                       		 *    For either of these cases, we do not need
                       		 *    to re-map lba . 
                       		 */
                        	diskinfo->last_ses_pvd_lba = 0;
		   	   } else {
                   		diskinfo->disc_info = 0x03;
		   		/* TOC read,format '01'b, last session address*/
                   		idecdrom_read_disc_info(diskinfo);
			   }
		   } else {
                        diskinfo->last_ses_pvd_lba = 0;
			if (!(diskinfo->ioctl_chg_mode_flg)) {
			/*
			 * If we have not done any change data
			 * mode ioctls then we want to 
			 * set up this drive for Mode 1.
			 *
			 * If we have used the change data mode
			 * ioctl then we don't want
			 * to overwrite the data mode. This
			 * is because we assume the user knows
			 * what they are doing.
			 */


			diskinfo->block_size = 
			     diskinfo->cfg_block_size;
			diskinfo->mult_of_blksize = 
			     diskinfo->block_size/DK_BLOCKSIZE;
			diskinfo->current_cd_mode = CD_MODE1;
			}
		   } /* check for XA disc */
		} else {
                   diskinfo->disc_info = 0xFF;
                   /*
                    * This disk is a CD-ROM XA disc that
                    * is a multi-session photo CD.
                    */
                          diskinfo->last_ses_pvd_lba =
                                  *((uint *) &(diskinfo->ioctl_buf[8]))
                                  + DK_CD_PVD; 
		}

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(processreset, trc,(char)0, (uint)diskinfo->block_size, 
	      (uint)diskinfo->mult_of_blksize,(uint)diskinfo->current_cd_mode, 
	      (uint)0,(uint)diskinfo->last_ses_pvd_lba);

   idecdrom_trc(processreset, trc,(char)1, (uint)diskinfo->disc_info, 
	      (uint)0,(uint)0, 
	      (uint)0,(uint)diskinfo->last_ses_pvd_lba);
#endif	
#endif
                if (diskinfo->disc_info == 0xFF) {
                	
                	/*
                	 * Determine if a mode select is necessary
                 	*/
                	if (idecdrom_mode_data_compare(diskinfo)) {
                        	/*
                         	* if a mode select is necessary
                         	*/
                        	idecdrom_mode_select(diskinfo);
                	}
                	else {
                        	/*
                         	* bypass the mode select
                         	*/
                        	idecdrom_read_cap(diskinfo);
                	}       
		}
                break;

	case DK_SELECT:
		/* A MODE SELECT cmd has returned */

		idecdrom_free_cmd(cmd_ptr);
		if (diskinfo->number_pages_to_select > 0) {
			idecdrom_mode_select(diskinfo);
		} else {
			idecdrom_read_cap(diskinfo);
		}
		break;
	case DK_READCAP:
		/*
		 * A READ CAPACITY cmd has returned
		 * This is the end of the RESET cycle
		 */
		diskinfo->reset_count = 0;

		diskinfo->disk_intrpt = 0;
		e_wakeup((int *)&(diskinfo->open_event));

		idecdrom_free_cmd(cmd_ptr);
		break;
	case DK_STOP:

		diskinfo->disk_intrpt = 0;
		e_wakeup((int *)&(diskinfo->open_event));
		idecdrom_free_cmd(cmd_ptr);
		break;
	case DK_ALLOW:
		idecdrom_free_cmd(cmd_ptr);	
		diskinfo->disk_intrpt = 0;
		e_wakeup((int *)&(diskinfo->open_event));
		break;

	default:
		/* Process unknown cmd subtype */
		ASSERT(FALSE);
		break;
	}
	diskinfo->state &= ~DK_RESET_PENDING;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(processreset, exit,(char)0, (uint)diskinfo, (uint)0,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,(uint)0);
#endif	
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_coalesce					*/
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
/*									*/
/************************************************************************/

uchar
idecdrom_coalesce(
struct idecdrom_diskinfo	*diskinfo)
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
   idecdrom_trc(coales, entry,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)bp, (uint)bp->b_bcount,(uint)bp->b_options);
#endif	
#endif

	/* Remove bp from PENDING queue */
	idecdrom_pending_dequeue(diskinfo);

        IDECDROM_GET_LBA(blkno,diskinfo,bp,rc);
        if (rc<0) {
          /*
           * Not a multiple of block size.
           * This should be filtered at
           * idecdrom_strategy.
           */
           ASSERT(rc == 0);
        }
        if (rc == 1) {
          /*
           * Request does not start on device
           * block boundary.
           */
           diskinfo->fragment.saved_bp = bp;
           return(1);
        }

	/* Check for retried commands */
	if (bp->b_resid != 0) {

		/* Place bp IN_PROGRESS queue */

		IDECDROM_IN_PROG_ENQUEUE(diskinfo, bp);
		return(0);
	}

        /* Place bp IN_PROGRESS queue */

	IDECDROM_IN_PROG_ENQUEUE(diskinfo, bp);

	if ((bp->b_bcount <= diskinfo->max_coalesce) &&
	    (((int)bp->b_un.b_addr & 0x0fff) == 0) &&
	    ((bp->b_bcount & 0x0fff) == 0) &&
	    (bp->b_resid == 0)) {

		next_block = blkno + 
		  		(bp->b_bcount /diskinfo->block_size);
		total_count += bp->b_bcount;
		rwflag = (bp->b_flags & B_READ);

		/* Check next buf in list for continued coalescing */
		bp = diskinfo->currbuf;
		if (bp != NULL) {
			IDECDROM_GET_LBA(blkno,diskinfo,bp,rc); 
                        if (rc<0) {
                                /*
                                 * Not a multiple of block size.
                                 * This should be filtered at
                                 * idecdrom_strategy.
                                 */
                                ASSERT(rc == 0);
                        }
		}
		/* Begin coalescing operations */
                while ((bp != NULL) && (rc != 1) &&
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
                         * 1.5) begins and ends on device block boundary
                         * 2) the next request is sequential with current one
                         * 3) the next one is the same direction transfer 
			 *    as this one (i.e. rwflag is read, bp is read) 
                         * 4) the next one begins on a page boundary
			 * 5) the next one will end on a page boundary
			 * 6) the next one won't exceed coalesce maximum
                         */

			/* Remove bp from PENDING queue */
			idecdrom_pending_dequeue(diskinfo);
	
			/* Place bp IN_PROGRESS queue */

			IDECDROM_IN_PROG_ENQUEUE(diskinfo, bp);


			next_block = blkno + 
			    (bp->b_bcount / diskinfo->block_size);
			total_count += bp->b_bcount;
			rwflag = (bp->b_flags & B_READ);

			/* Check next buf in list for continued coalescing */
			bp = diskinfo->currbuf;
			if (bp != NULL) {

				IDECDROM_GET_LBA(blkno,diskinfo,bp,rc);
				if (rc<0) {
                                       /*
                                 	* Not a multiple of block size.
                                 	* This should be filtered at
                               	 	* idecdrom_strategy.
                                 	*/
					ASSERT(rc == 0);
				}
			}

		}
	}

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(coales, exit,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_bp_queue.in_progress.head, (uint)0,(uint)0);
#endif	
#endif
	return(0);
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_fragmented_build_cmd				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds the separate pieces needed for an	*/
/*		I/O request that is not 2K (device block) aligned or   	*/
/*		is not a multiple of the device block size		*/
/*									*/
/*		This routine scans the fragmented queue for the		*/
/*		specified device and builds a IDE operation to		*/
/*		satisfy the request. 					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*		This routine will build the following IDE commands:     */
/*									*/
/*									*/
/*                        READ CD Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (BEh)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |      Reserved      |Exp. User Data Type |  Reserved   |     */
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |           Starting Logical Block Address              |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   | (MSB)       Transfer Length in Blocks                 |	*/
/*  |-----+---                                                 ---|	*/
/*  | 7   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  |     |                    Flag Bits                          |	*/
/*  |     |-------------------------------------------------------|	*/
/*  | 9   |Synch |  Header(s)  | User | EDC &|    Error    |Rsvd  |	*/
/*  |     |Field |   Code      | Data | ECC  |   Flag(s)   |      |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 10  |            Reserved              |Sub-Channel Data Sel|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 11  |                    Reserved                           |	*/
/*  +=============================================================+	*/
/*									*/
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
/*		This routine returns the address of a disk cmd		*/
/*		structure which has been built to satisfy the		*/
/*		requests in the IN_PROGRESS queue for the device.	*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bcopy							*/
/*									*/
/*									*/
/************************************************************************/

struct dk_cmd *
idecdrom_fragmented_build_cmd(
struct idecdrom_diskinfo	*diskinfo,
char   good_path) 
{
	struct dk_raw_buf *     raw_bp;
	int			first_device_blkno, last_device_blkno;
	int			blk_cnt, lba, partial_count, block_number;
	struct dk_cmd		*cmd_ptr;
	struct buf		*cmd_bp,*bp, *tmp_bp;
	struct ata_cmd		*ata;
	int			middle,blkno,rc;
	uchar			user_data_type; /* Expected user data type */
	uchar			flag_bits;      /* For M2F2 subheader      */
	uchar			cyl_hi;         /* Sector size hi byte     */
	uchar			cyl_lo;         /* Sector size lo byte     */
	unsigned		temp_bcount;
	unsigned		last_device_byte, last_request_byte;


	bp = diskinfo->fragment.saved_bp;
	if (bp->b_flags & B_READ) {


		/* Update the byte count if beyond end of disk */
		last_device_byte = (DK_BLOCKSIZE * diskinfo->mult_of_blksize) *
			(diskinfo->capacity.lba + 1);
		last_request_byte = (bp->b_blkno * DK_BLOCKSIZE) +
			bp->b_bcount;
		if (last_request_byte > last_device_byte) {
			temp_bcount = bp->b_bcount - (last_request_byte -
				last_device_byte);
			diskinfo->fragment.saved_resid = 
				last_request_byte - last_device_byte;
		}
		else {
			temp_bcount = bp->b_bcount;
			diskinfo->fragment.saved_resid = 0;
		}

		first_device_blkno = bp->b_blkno / diskinfo->mult_of_blksize;
		last_device_blkno = (bp->b_blkno + (temp_bcount /DK_BLOCKSIZE))
	                             / diskinfo->mult_of_blksize;
		if (((bp->b_blkno + (temp_bcount /DK_BLOCKSIZE))
	               % diskinfo->mult_of_blksize) == 0) {
			last_device_blkno = last_device_blkno - 1;
		}

		diskinfo->fragment.first_partial_blkno = first_device_blkno;
		diskinfo->fragment.last_partial_blkno = last_device_blkno;

		/*
		 * Determine if read starts on a device block boundary
		 */
		if (bp->b_blkno % diskinfo->mult_of_blksize) {
			/*
			 * The start of this request does not begin on a
			 * device block boundary.
			 * Compute number of blocks up to first device block
			 * boundary.
			 */

			diskinfo->fragment.first_partial_len =
				diskinfo->mult_of_blksize -
			     	(bp->b_blkno % diskinfo->mult_of_blksize);
			diskinfo->fragment.first_partial_offset = 
				bp->b_blkno % diskinfo->mult_of_blksize;
			if ((diskinfo->fragment.first_partial_len*DK_BLOCKSIZE)>
				temp_bcount) {
				/* does not reach the end of this device block*/

				diskinfo->fragment.first_partial_len =
				   temp_bcount / DK_BLOCKSIZE;
			}	
			diskinfo->fragment.first_partial_bcount = DK_BLOCKSIZE *
				diskinfo->fragment.first_partial_len;
			diskinfo->fragment.first_partial_baddr =
				(int)&diskinfo->fragment.buffer +(DK_BLOCKSIZE *
				diskinfo->fragment.first_partial_offset);
		}
		else {
			/*
			 * The start of this request does begin on a
			 * device block boundary.
			 */

			diskinfo->fragment.first_partial_len = 0;
		}

		if ((first_device_blkno == last_device_blkno) &&
		     (diskinfo->fragment.first_partial_len != 0)) {
			/*
			 * The transfer does not span across multiple blocks.
			 */

			diskinfo->fragment.middle_len = 0;
			diskinfo->fragment.last_partial_len = 0; /*F__*/
		}
		else {
		        if ((bp->b_blkno + (temp_bcount /
				DK_BLOCKSIZE)) % diskinfo->mult_of_blksize) {
		    	   /*
			    * The request does not end on a device blk boundary.
			    */

		           diskinfo->fragment.last_partial_len =
		               (bp->b_blkno + (temp_bcount /
				DK_BLOCKSIZE)) % diskinfo->mult_of_blksize;
			   diskinfo->fragment.last_partial_bcount =
				DK_BLOCKSIZE *
				diskinfo->fragment.last_partial_len;
			   diskinfo->fragment.last_partial_baddr =
				(int)bp->b_un.b_addr + temp_bcount -
				diskinfo->fragment.last_partial_bcount;

			   if (first_device_blkno == last_device_blkno) {
				diskinfo->fragment.middle_len = 0; /*__L*/
			   }
			   else {
			      if ((first_device_blkno+1 == last_device_blkno)
		    	       && (diskinfo->fragment.first_partial_len != 0)) {
		    	        /*
			         * The request spans two device blocks but
				 * does not begin or end on a device block
			         * boundary.
			         */

				diskinfo->fragment.middle_len = 0; /*F_L*/
			      }
			      else {
		     	        if (diskinfo->fragment.first_partial_len != 0) {
				   /*FML*/

		           	   diskinfo->fragment.middle_len =
			             last_device_blkno - first_device_blkno - 1;
				   diskinfo->fragment.middle_blkno =
				     first_device_blkno + 1;
			   	   diskinfo->fragment.middle_bcount =
				     DK_BLOCKSIZE * diskinfo->mult_of_blksize *
				     diskinfo->fragment.middle_len;
			   	   diskinfo->fragment.middle_baddr =
				     (int)bp->b_un.b_addr +
				     diskinfo->fragment.first_partial_bcount;
				}
				else {
				   /*_ML*/
				   /* The request starts on a device block
				    * boundary, but does not end on one.
				    */

		           	   diskinfo->fragment.middle_len =
			             last_device_blkno - first_device_blkno;
				   diskinfo->fragment.middle_blkno =
				     first_device_blkno;
			   	   diskinfo->fragment.middle_bcount =
				     DK_BLOCKSIZE * diskinfo->mult_of_blksize *
				     diskinfo->fragment.middle_len;
			   	   diskinfo->fragment.middle_baddr =
				     (int)bp->b_un.b_addr;
				}
					
			      }	
			   }
			}
			else {
		    	   /*
			    * The request does end on a device block boundary.
			    */
				diskinfo->fragment.last_partial_len = 0;
				/* Note: could be a _M_ if a _ML had the last */
				/* part truncated due to end of disk.         */

				if (diskinfo->fragment.first_partial_len == 0) {
				 /* _M_ */

		           	 diskinfo->fragment.middle_len =
			          last_device_blkno - first_device_blkno + 1;
				 diskinfo->fragment.middle_blkno =
				  first_device_blkno;
			   	 diskinfo->fragment.middle_bcount =
				  DK_BLOCKSIZE * diskinfo->mult_of_blksize *
				  diskinfo->fragment.middle_len;
			   	 diskinfo->fragment.middle_baddr =
				  (int)bp->b_un.b_addr;
				}
				else {
				 /* FM_ */

		           	 diskinfo->fragment.middle_len =
			          last_device_blkno - first_device_blkno;
				 diskinfo->fragment.middle_blkno =
				  first_device_blkno + 1;
			   	 diskinfo->fragment.middle_bcount =
				  DK_BLOCKSIZE * diskinfo->mult_of_blksize *
				  diskinfo->fragment.middle_len;
			   	 diskinfo->fragment.middle_baddr =
				  (int)bp->b_un.b_addr +
				  diskinfo->fragment.first_partial_bcount;
				}
			}
		}

		/* set up build cmd parameters for fragment to be sent first */

		if (diskinfo->fragment.first_partial_len != 0) {
			diskinfo->fragment.issued = DK_FIRST;
			middle = FALSE;
			block_number = diskinfo->fragment.first_partial_blkno;
			partial_count = diskinfo->fragment.first_partial_bcount;
			blk_cnt = 1;
			/* address will be temporary buffer */
				
		}
		else if (diskinfo->fragment.middle_len != 0) {
			diskinfo->fragment.issued = DK_MIDDLE;
			middle = TRUE;
			block_number = diskinfo->fragment.middle_blkno;
			partial_count = diskinfo->fragment.middle_bcount;
		        blk_cnt = diskinfo->fragment.middle_len;
			/* address will be diskinfo->fragment.middle_baddr; */
		}
		else {
			diskinfo->fragment.issued = DK_LAST;
			middle = FALSE;
			block_number = diskinfo->fragment.last_partial_blkno;
			partial_count = diskinfo->fragment.last_partial_bcount;
			blk_cnt = 1;
			/* address will be temporary buffer */
		}

        /* 
	 * Build a bp which is for this single or middle block operation 
	 */
	raw_bp = (struct dk_raw_buf *)&diskinfo->fragment.temp_raw_bp;

        raw_bp->bp.b_dev = diskinfo->devno;
        raw_bp->bp.b_flags = bp->b_flags;
        raw_bp->bp.b_options = bp->b_options;
        raw_bp->bp.b_work = 0x00;
        raw_bp->bp.b_error = 0;
        raw_bp->bp.b_blkno = block_number;
        raw_bp->bp.b_bcount = partial_count;
	if (middle) {
              raw_bp->bp.b_bcount = partial_count;
	      raw_bp->bp.b_un.b_addr = (caddr_t)diskinfo->fragment.middle_baddr;
	      bcopy((caddr_t) &(bp->b_xmemd), (caddr_t)
	     	 &(raw_bp->bp.b_xmemd), sizeof(struct xmem));
	}
	else {
        	raw_bp->bp.b_bcount = DK_BLOCKSIZE * diskinfo->mult_of_blksize;
		raw_bp->bp.b_un.b_addr =(caddr_t)diskinfo->fragment.buffer;
        	raw_bp->bp.b_xmemd.aspace_id = XMEM_GLOBAL;
	}
        raw_bp->bp.b_iodone = ((void(*)())idecdrom_iodone);
        raw_bp->bp.b_event = EVENT_NULL;
        raw_bp->bp.b_forw = NULL;
        raw_bp->bp.b_back = NULL;
        raw_bp->bp.av_forw = NULL;
        raw_bp->bp.av_back = NULL;
	raw_bp->diskinfo = diskinfo;

        /* 
	 * Allocate a command structure for this command 
	 */
        cmd_ptr = idecdrom_cmd_alloc_disable(diskinfo,(uchar) DK_BUF);

        if (cmd_ptr == NULL) 
	  	return(cmd_ptr);

	/* 
	 * increment transfer count 
	 */
	diskinfo->dkstat.dk_xfers++;


        /* 
	 * Initialize command block for requested operation 
	 */
        cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
        bcopy( raw_bp, cmd_bp, sizeof(struct buf));
        cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);

        /* 
	 * Fill out ataidebuf structure 
	 */
        cmd_ptr->ataidebuf.timeout_value = diskinfo->rw_timeout;
        cmd_ptr->ataidebuf.bp = NULL;
        cmd_ptr->ataidebuf.status_validity = 0x00;


        /* 
	 * Setup rest of cmd_ptr structure 
	 */
        cmd_ptr->retry_count = 0;
        cmd_ptr->segment_count = 0x00;
        cmd_ptr->soft_resid = 0x00;
        cmd_ptr->group_type = DK_FRAGMENTED;
        cmd_ptr->bp = (struct buf *)raw_bp;

		lba = block_number;
		ata = &(cmd_ptr->ataidebuf.ata);
		if (diskinfo->current_cd_mode == CD_DA) {
			user_data_type = 0x04;
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x09;
			cyl_lo = 0x30;
		}
		else if (diskinfo->current_cd_mode == CD_MODE2_FORM2) {
			user_data_type = 0x14;
			flag_bits = 0x50; /* get user data and subheader */
			cyl_hi = 0x09;
			cyl_lo = 0x20;
		}
		else if (diskinfo->current_cd_mode == CD_MODE2_FORM1) {
			user_data_type = 0x10;
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x10;
			cyl_lo = 0x00;
		}
		else {
			user_data_type = 0x08; /*Mode 1 */
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x10;
			cyl_lo = 0x00;
		}
		ata->startblk.chs.cyl_hi = cyl_hi;
		ata->startblk.chs.cyl_lo = cyl_lo;
	  	ata->atapi.length = 12;
		ata->command = ATA_ATAPI_PACKET_COMMAND;
		ata->device = diskinfo->device_id;
		ata->feature = 0x00;

		ata->atapi.packet.op_code = ATAPI_READ_CD;
		ata->atapi.packet.bytes[0] = (0x1C & user_data_type);
		ata->atapi.packet.bytes[1] = ((lba >> 24) & 0xff);
		ata->atapi.packet.bytes[2] = ((lba >> 16) & 0xff);
		ata->atapi.packet.bytes[3] = ((lba >> 8) & 0xff);
		ata->atapi.packet.bytes[4] = (lba & 0xff);
		ata->atapi.packet.bytes[5] = ((blk_cnt >> 16) & 0xff);
		ata->atapi.packet.bytes[6] = ((blk_cnt >> 8) & 0xff);
		ata->atapi.packet.bytes[7] = (blk_cnt & 0xff);
		ata->atapi.packet.bytes[8] = flag_bits;
		/* Sub-Channel Data Selection Bits */
		ata->atapi.packet.bytes[9] = 0x00;
		ata->atapi.packet.bytes[10] = 0x00;

		return(cmd_ptr);
	}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(buildcmd, trc,(char)4, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);

#endif	
#endif


	return(cmd_ptr);
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_build_cmd					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine scans the IN_PROGRESS queue for the	*/
/*		specified device and builds a IDE operation to		*/
/*		satisfy the request list. The IN_PROGRESS queue		*/
/*		may contain only one type of cmd			*/
/*		list. The list will be :				*/
/*		    a group of contiguous read operations		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*		This routine will build the following IDE commands:     */
/*									*/
/*									*/
/*                        READ CD Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (BEh)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |      Reserved      |Exp. User Data Type |  Reserved   |     */
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |           Starting Logical Block Address              |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   | (MSB)       Transfer Length in Blocks                 |	*/
/*  |-----+---                                                 ---|	*/
/*  | 7   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  |     |                    Flag Bits                          |	*/
/*  |     |-------------------------------------------------------|	*/
/*  | 9   |Synch |  Header(s)  | User | EDC &|    Error    |Rsvd  |	*/
/*  |     |Field |   Code      | Data | ECC  |   Flag(s)   |      |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 10  |            Reserved              |Sub-Channel Data Sel|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 11  |                    Reserved                           |	*/
/*  +=============================================================+	*/
/*									*/
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
/*		This routine returns the address of a disk cmd		*/
/*		structure which has been built to satisfy the		*/
/*		requests in the IN_PROGRESS queue for the device.	*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bcopy							*/
/*									*/
/*									*/
/************************************************************************/

struct dk_cmd *
idecdrom_build_cmd(
struct idecdrom_diskinfo	*diskinfo,
struct buf **head,
char   good_path)
{
	int			blk_cnt, lba, total_count, verify;
	struct dk_cmd		*cmd_ptr;
	struct buf		*cmd_bp,*bp, *tmp_bp;
	struct ata_cmd		*ata;
	int			blkno,rc;
	uchar			user_data_type; /* Expected user data type */
	uchar			flag_bits;      /* For M2F2 subheader      */
	uchar			cyl_hi;         /* Sector size hi byte     */
	uchar			cyl_lo;         /* Sector size lo byte     */


	verify = FALSE;
        /* Start with head of IN_PROGRESS queue */
        bp = *head;


#ifdef DEBUG
#ifdef IDE_GOOD_PATH

   idecdrom_trc(buildcmd, trc,(char)0, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);
#endif	
#endif
	if (bp == NULL) {
#ifdef DEBUG
		DKprintf(("idecdrom_build_cmd: NULL in_progress.head\n"));
#endif
		ASSERT(FALSE);
	}

	IDECDROM_GET_LBA(blkno,diskinfo,bp,rc);
	if (rc) {
	  	/*
		 * Request does not start on device
		 * block boundary.  
		 * This should be filtered at 
		 * idecdrom_strategy.
		 */
	  	ASSERT(rc == 0);
	}
	if (bp->b_resid != 0) {     /* setup for end of file read */

		/* allocate cmd for read operation */
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
	idecdrom_trc(buildcmd, trc,(char)1, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);
#endif	
#endif

                cmd_ptr = idecdrom_cmd_alloc(diskinfo,(uchar) DK_BUF);
	        if (cmd_ptr == NULL) {
			return(cmd_ptr);
		}
	        if (good_path == TRUE) {
                         DDHKWD3(HKWD_DD_IDECDROMDD, DD_COALESCE, 0, 
				diskinfo->devno, cmd_ptr->bp, 
				&(cmd_ptr->ataidebuf));
		 }
		/* increment transfer count */
		diskinfo->dkstat.dk_xfers++;

		/* Initialize cmd block for READ op */
		cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = bp->b_flags;
		cmd_bp->b_options = bp->b_options;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = blkno;
		
		cmd_bp->b_bcount = bp->b_bcount - bp->b_resid;
		cmd_ptr->group_type = DK_SINGLE;

		cmd_bp->b_un.b_addr = bp->b_un.b_addr;
		bcopy((caddr_t) &(bp->b_xmemd), (caddr_t) &(cmd_bp->b_xmemd),
		    sizeof(struct xmem));
		cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
		cmd_ptr->ataidebuf.timeout_value = diskinfo->rw_timeout;
		cmd_ptr->ataidebuf.bp = NULL;
		cmd_ptr->ataidebuf.status_validity = 0x00;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		cmd_ptr->bp = bp;

		/* Initialize IDE cmd for operation */
		blk_cnt = cmd_bp->b_bcount / diskinfo->block_size;
		lba = blkno;
		ata = &(cmd_ptr->ataidebuf.ata);
		if (diskinfo->current_cd_mode == CD_DA) {
			user_data_type = 0x04;
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x09;
			cyl_lo = 0x30;
		}
		else if (diskinfo->current_cd_mode == CD_MODE2_FORM2) {
			user_data_type = 0x14;
			flag_bits = 0x50; /* get user data and subheader */
			cyl_hi = 0x09;
			cyl_lo = 0x20;
		}
		else if (diskinfo->current_cd_mode == CD_MODE2_FORM1) {
			user_data_type = 0x10;
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x10;
			cyl_lo = 0x00;
		}
		else {
			user_data_type = 0x08; /*Mode 1 */
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x10;
			cyl_lo = 0x00;
		}
		ata->startblk.chs.cyl_hi = cyl_hi;
		ata->startblk.chs.cyl_lo = cyl_lo;
	  	ata->atapi.length = 12;
		ata->command = ATA_ATAPI_PACKET_COMMAND;
		ata->device = diskinfo->device_id;
		ata->feature = 0x00;

		ata->atapi.packet.op_code = ATAPI_READ_CD;
		ata->atapi.packet.bytes[0] = (0x1C & user_data_type);
		ata->atapi.packet.bytes[1] = ((lba >> 24) & 0xff);
		ata->atapi.packet.bytes[2] = ((lba >> 16) & 0xff);
		ata->atapi.packet.bytes[3] = ((lba >> 8) & 0xff);
		ata->atapi.packet.bytes[4] = (lba & 0xff);
		ata->atapi.packet.bytes[5] = ((blk_cnt >> 16) & 0xff);
		ata->atapi.packet.bytes[6] = ((blk_cnt >> 8) & 0xff);
		ata->atapi.packet.bytes[7] = (blk_cnt & 0xff);
		ata->atapi.packet.bytes[8] = flag_bits;
		/* Sub-Channel Data Selection Bits */
		ata->atapi.packet.bytes[9] = 0x00;
		ata->atapi.packet.bytes[10] = 0x00;


		/*
		 * Clear in progress queue since we just processed it
		 */
		*head  = NULL;
		return(cmd_ptr);
	}
	if (bp->b_flags & B_READ) {
		/* allocate cmd for read operation */
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(buildcmd, trc,(char)2, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);

#endif	
#endif
		cmd_ptr = idecdrom_cmd_alloc(diskinfo,(uchar)DK_BUF);
		if (cmd_ptr == NULL) {
			return(cmd_ptr);
		}
	        if (good_path == TRUE) {
			DDHKWD3(HKWD_DD_IDECDROMDD, DD_COALESCE, 0, 
				diskinfo->devno, cmd_ptr->bp, 
				&(cmd_ptr->ataidebuf));
		}
		/* increment transfer count */
		diskinfo->dkstat.dk_xfers++;

		
#ifdef DEBUG
		DKprintf(("idecdrom_build_cmd building READ from \
				coalesced list\n"));
#endif
		cmd_ptr->ataidebuf.bp = bp;
		total_count = 0x00;
		tmp_bp = bp;
		while (tmp_bp != NULL) {
			total_count += tmp_bp->b_bcount;

			tmp_bp = tmp_bp->av_forw;
		}
		
		/* Initialize cmd block for entire READ op */
		cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = bp->b_flags;
		cmd_bp->b_options = bp->b_options;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = blkno;
		cmd_bp->b_bcount = total_count;
		cmd_bp->b_un.b_addr = bp->b_un.b_addr;
		bcopy((caddr_t) &(bp->b_xmemd), (caddr_t)
		      &(cmd_bp->b_xmemd), sizeof(struct xmem));
		cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
		cmd_ptr->ataidebuf.timeout_value = diskinfo->rw_timeout;
		cmd_ptr->ataidebuf.status_validity = 0x00;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		if (bp->av_forw != NULL) {
			cmd_ptr->group_type = DK_COALESCED;
			cmd_ptr->ataidebuf.bp = bp;
			cmd_ptr->bp = NULL;
		}
		else {
			cmd_ptr->group_type = DK_SINGLE;
			cmd_ptr->ataidebuf.bp = NULL;
			cmd_ptr->bp = bp;
		}
		
		/* Initialize IDE cmd for operation */
		blk_cnt = cmd_bp->b_bcount / diskinfo->block_size;
		lba = blkno;
		ata = &(cmd_ptr->ataidebuf.ata);
		if (diskinfo->current_cd_mode == CD_DA) {
			user_data_type = 0x04;
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x09;
			cyl_lo = 0x30;
		}
		else if (diskinfo->current_cd_mode == CD_MODE2_FORM2) {
			user_data_type = 0x14;
			flag_bits = 0x50; /* get user data and subheader */
			cyl_hi = 0x09;
			cyl_lo = 0x20;
		}
		else if (diskinfo->current_cd_mode == CD_MODE2_FORM1) {
			user_data_type = 0x10;
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x10;
			cyl_lo = 0x00;
		}
		else {
			user_data_type = 0x08; /*Mode 1 */
			flag_bits = 0x10; /* get user data only */
			cyl_hi = 0x10;
			cyl_lo = 0x00;
		}
		ata->startblk.chs.cyl_hi = cyl_hi;
		ata->startblk.chs.cyl_lo = cyl_lo;
	  	ata->atapi.length = 12;
		ata->command = ATA_ATAPI_PACKET_COMMAND;
		ata->device = diskinfo->device_id;
		ata->feature = 0x00;

		ata->atapi.packet.op_code = ATAPI_READ_CD;
		ata->atapi.packet.bytes[0] = (0x1C & user_data_type);
		ata->atapi.packet.bytes[1] = ((lba >> 24) & 0xff);
		ata->atapi.packet.bytes[2] = ((lba >> 16) & 0xff);
		ata->atapi.packet.bytes[3] = ((lba >> 8) & 0xff);
		ata->atapi.packet.bytes[4] = (lba & 0xff);
		ata->atapi.packet.bytes[5] = ((blk_cnt >> 16) & 0xff);
		ata->atapi.packet.bytes[6] = ((blk_cnt >> 8) & 0xff);
		ata->atapi.packet.bytes[7] = (blk_cnt & 0xff);
		ata->atapi.packet.bytes[8] = flag_bits;
		/* Sub-Channel Data Selection Bits */
		ata->atapi.packet.bytes[9] = 0x00;
		ata->atapi.packet.bytes[10] = 0x00;

		/*
		 * Clear in progress queue since we just processed it
		 */
		*head  = NULL;
		return(cmd_ptr);
	}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(buildcmd, trc,(char)4, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);

#endif	
#endif

	/*
	 * Clear in progress queue since we just processed it
	 */
	if (cmd_ptr != NULL)
		*head  = NULL;

	return(cmd_ptr);
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_request_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a request sense command block and	*/
/*		places it on the device's command stack so that the	*/
/*		request sense operation will be performed before the	*/
/*		current command will be retried.			*/
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
/*		checked_cmd	A pointer to the cmd that had the error	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bzero							*/
/*									*/
/************************************************************************/

void
idecdrom_request_sense(
struct idecdrom_diskinfo	*diskinfo,
struct dk_cmd  *checked_cmd)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;
	uchar		reqsns_in_progress = FALSE;


	/* Allocate a REQUEST type cmd for this operation */




        cmd_ptr = &(diskinfo->reqsns_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reqsns, entry,(char)0, (uint)diskinfo, (uint)checked_cmd,
	      (uint)cmd_ptr->status, (uint)0,
	      (uint)diskinfo->state);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is  
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        return;
                } else {
                        /*
                         * make sure command is at the head of dequeued
                         */
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			reqsns_in_progress = TRUE;
                }
        }
	if (!reqsns_in_progress) {
		diskinfo->checked_cmd = checked_cmd;
		idecdrom_d_q_cmd(checked_cmd,(uchar) DK_CMD_Q);
	}
        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_REQSNS; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for request sense cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = DK_REQSNS_LEN;
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->sense_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
	cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */

     /*
      *                        REQUEST SENSE  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (03h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                       Allocation Length                       |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 8   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 10  |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 11  |                           Reserved                            |
      *+=====================================================================+
      */

		/* Initialize IDE cmd for operation */
		ata = &(cmd_ptr->ataidebuf.ata);
		ata->startblk.chs.cyl_hi = 0x00;
		ata->startblk.chs.cyl_lo = DK_REQSNS_LEN;
	  	ata->atapi.length = 12;
		ata->command = ATA_ATAPI_PACKET_COMMAND;
		ata->device = diskinfo->device_id;
		ata->feature = 0x00;
		ata->atapi.packet.op_code = ATAPI_REQUEST_SENSE;
		ata->atapi.packet.bytes[0] = 0x00;
		ata->atapi.packet.bytes[1] = 0x00;
		ata->atapi.packet.bytes[2] = 0x00;
		ata->atapi.packet.bytes[3] = DK_REQSNS_LEN;
		ata->atapi.packet.bytes[4] = 0x00;
		ata->atapi.packet.bytes[5] = 0x00;
		ata->atapi.packet.bytes[6] = 0x00;
		ata->atapi.packet.bytes[7] = 0x00;
		ata->atapi.packet.bytes[8] = 0x00;
		ata->atapi.packet.bytes[9] = 0x00;
		ata->atapi.packet.bytes[10] = 0x00;
		ata->atapi.packet.bytes[11] = 0x00;

	/* Clear out the sense data buffer */
	bzero(diskinfo->sense_buf, DK_REQSNS_LEN);

	/* Place cmd on device's operation stack */
	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(reqsns, exit,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)diskinfo->state,(uint)0);
#endif	
#endif

	return;

}


/************************************************************************/
/*									*/
/*	NAME:	idecdrom_start_unit_disable				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a START UNIT cmd block for routines */
/*		that have not spin locked.				*/
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
/*		start_stop_flag	Flag to specify a start or stop request	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable	i_enable				*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_start_unit_disable(
struct idecdrom_diskinfo	*diskinfo,
uchar			start_stop_flag)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
	old_pri = i_disable(INTIODONE);
	idecdrom_start_unit(diskinfo,start_stop_flag);

	/* 
	 * reenable interrupts and unlock
	 */
	i_enable(old_pri);
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_start_unit					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a START UNIT cmd block which	*/
/*		starts the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and idecdrom_start is called	*/
/*		to initiate execution of the cmd.			*/
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
/*		start_stop_flag	Flag to specify a start or stop request	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_start_unit(
struct idecdrom_diskinfo	*diskinfo,
uchar			start_stop_flag)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;

	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(startunit,entry, (char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
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
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for start unit cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
	cmd_ptr->ataidebuf.timeout_value= diskinfo->start_timeout;/* 1 minute */
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */

      /*                   START STOP UNIT Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (1Bh)                |
       *|-----+---------------------------------------------------------------|
       *| 1   |                    Reserved                           | Immed |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved            |  LoEj |  Start|
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 6   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 7   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 8   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 9   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 10  |                           Reserved                            |
       *+=====================================================================+
       */

	/* Initialize IDE cmd for operation */
	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->atapi.length = 12;
	ata->atapi.packet.op_code = ATAPI_START_STOP_UNIT;
	ata->atapi.packet.bytes[0] = 0x01;  /* Immediate bit one */
	ata->atapi.packet.bytes[1] = 0x00;
	ata->atapi.packet.bytes[2] = 0x00;
	if (start_stop_flag == DK_START) {
		ata->atapi.packet.bytes[3] = 0x01; /* LoEj 0, start bit set */
        }  else  {
		ata->atapi.packet.bytes[3] = 0x00; /* LoEj 0, start bit zero */
        }
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00;
	ata->atapi.packet.bytes[6] = 0x00;
	ata->atapi.packet.bytes[7] = 0x00;
	ata->atapi.packet.bytes[8] = 0x00;
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;


	cmd_ptr->subtype = start_stop_flag;

	/* Place cmd on device's operation stack */

	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	/* Begin execution of this operation */

	idecdrom_start(diskinfo);

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_test_unit_ready_disable			*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a TEST UNIT READY cmd block for	*/
/*		routines not disabled and spin locked.			*/
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
/*		i_disable	i_enable				*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_test_unit_ready_disable(
struct idecdrom_diskinfo	*diskinfo)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
	old_pri = i_disable(INTIODONE);
	idecdrom_test_unit_ready(diskinfo);
	/* 
	 * reenable interrupts and unlock
	 */
	i_enable(old_pri);
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_test_unit_ready				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a TEST UNIT READY cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and idecdrom_start is called	*/
/*		to initiate execution of the cmd.			*/
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
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_test_unit_ready(
struct idecdrom_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;


	/*
	 * The indicated device has been determined to have been
	 * Reset or Power Cycled at this point. We must make sure
	 * that any cmds on the device's stack are valid.
	 * ie. Remove any RESET or REQSNS type cmds that are on
	 * the stack for this device and free the cmd blocks.
	 */

	diskinfo->reset_count++;
	cmd_ptr = diskinfo->dk_cmd_q_head;

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(testunit,entry, (char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif


	while ((cmd_ptr != NULL) && (cmd_ptr->type & (DK_RESET | DK_REQSNS))) {
		idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
		if ((diskinfo->starting_close) &&
		    ((cmd_ptr->subtype == DK_ALLOW))) {
			/*
			 * If we started closing the device and
			 * there is a reset command that is a an
			 * allow media removal command, then we
			 * know that close has already allowed
			 * all pending I/O to complete.  The only way we could
			 * get here is if some kind of error (i.e. Unit 
			 * Attention) occurred on either the release or allow
			 * media removal command.  However we do not want to
			 * issue the reset cycle, because this could re-issue
			 * the prevent media removal command. It
			 * is safest instead to assume that the unit attention
			 * cleared this and wakeup the close thread. Thus
			 * letting close complete.
			 */
			diskinfo->disk_intrpt = 0;
			e_wakeup((int *)&(diskinfo->open_event));
			idecdrom_free_cmd(cmd_ptr);	
			return;
		}
		idecdrom_free_cmd(cmd_ptr);
		cmd_ptr = diskinfo->dk_cmd_q_head;
	}

	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(testunit, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif



        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for test unit ready cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = (void(*)())idecdrom_iodone;
	cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */

      /*                  TEST UNIT READY Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (00h)                |
       *|-----+---------------------------------------------------------------|
       *| 1   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 6   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 7   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 8   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 9   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 10  |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 11  |                           Reserved                            |
       *+=====================================================================+
       */


	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->atapi.length = 12;
	ata->atapi.packet.op_code = ATAPI_TEST_UNIT_READY;
	ata->atapi.packet.bytes[0] = 0x00;
	ata->atapi.packet.bytes[1] = 0x00;
	ata->atapi.packet.bytes[2] = 0x00;
	ata->atapi.packet.bytes[3] = 0x00;
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00;
	ata->atapi.packet.bytes[6] = 0x00;
	ata->atapi.packet.bytes[7] = 0x00;
	ata->atapi.packet.bytes[8] = 0x00;
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;

	cmd_ptr->subtype = DK_TUR;

	/* Place cmd on device's operation stack */
	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);


	/* Begin execution of this operation */

	idecdrom_start(diskinfo);

	return;
}


/************************************************************************/
/*									*/
/*	NAME:	idecdrom_prevent_allow_disable         			*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds an IDE prevent allow media removal  */
/*		command for routines that are not disabled and spin 	*/
/*		locked.							*/
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
/*		prevent_allow_flag Flag to request prevent or allow	*/
/*				   media removal			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable	i_enable				*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_prevent_allow_disable(
struct idecdrom_diskinfo	*diskinfo,
uchar			prevent_allow_flag)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
	old_pri = i_disable(INTIODONE);
	idecdrom_prevent_allow(diskinfo,prevent_allow_flag);
	/* 
	 * reenable interrupts and unlock
	 */
	i_enable(old_pri);
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_prevent_allow             			*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds an IDE prevent allow media removal  */
/*		command.  If the prevent_allow_flag is set then it will */
/*		build a prevent media removal command.  Otherwise it    */
/*		will build an allow media removal command		*/
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
/*		prevent_allow_flag Flag to request prevent or allow	*/
/*				   media removal			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_prevent_allow(
struct idecdrom_diskinfo	*diskinfo,
uchar			prevent_allow_flag)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(prevent,trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);

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
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for prevent allow medium removal cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
	cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */

     /*
      *                 PREVENT ALLOW MEDIUM REMOVAL Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (1Eh)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                       Reserved                        |Prevent|
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 8   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 10  |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 11  |                           Reserved                            |
      *+=====================================================================+
      */

	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->atapi.length = 12;
	ata->atapi.packet.op_code = ATAPI_PREVENT_ALLOW_REMOVAL;
	ata->atapi.packet.bytes[0] = 0x00;
	ata->atapi.packet.bytes[1] = 0x00;
	ata->atapi.packet.bytes[2] = 0x00;
	if (prevent_allow_flag == DK_PREVENT) {
		cmd_ptr->subtype = DK_PREVENT;	
		ata->atapi.packet.bytes[3] = 0x01;
	}
	else  {
		cmd_ptr->subtype = DK_ALLOW;
		ata->atapi.packet.bytes[3] = 0x00;
	}
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00;
	ata->atapi.packet.bytes[6] = 0x00;
	ata->atapi.packet.bytes[7] = 0x00;
	ata->atapi.packet.bytes[8] = 0x00;
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;


	/* Place cmd on device's operation stack */
	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	return;
}


/************************************************************************/
/*									*/
/*	NAME:	idecdrom_mode_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a MODE SENSE cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and idedcrom_start is called	*/
/*		to initiate execution of the cmd.			*/
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
/*		bzero							*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_mode_sense(
struct idecdrom_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(modesense, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
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
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for mode sense cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 255; /* setup for max mode data transfer */
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->sense_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
	cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */

     /*
      *                            MODE SENSE  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (5Ah)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   |       Reserved        |Resvd  | Resvd |       Reserved        | 
      *|-----+---------------------------------------------------------------|
      *| 2   |      PC       |         Page Code                             |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                      Allocation Length (MSB)                  |
      *|-----+---------------------------------------------------------------|
      *| 8   |                      Allocation Length (LSB)                  |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 10  |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 11  |                           Reserved                            |
      *+=====================================================================+
      */

	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->atapi.length = 12;
	ata->atapi.packet.op_code = ATAPI_MODE_SENSE_10;
	ata->startblk.chs.cyl_hi = (((uchar)cmd_bp->b_bcount >> 8) & 0xff);
	ata->startblk.chs.cyl_lo = ((uchar)cmd_bp->b_bcount & 0xff);
        /*
         * Set Page Code byte to request all supported pages....set Page
         * Control bits appropriately depending if we want changeable or
         * current mode data
         */
	ata->atapi.packet.bytes[0] = 0x00;
	ata->atapi.packet.bytes[1] = (0x3F |
                ((diskinfo->m_sense_status == DK_SENSE_CHANGEABLE) ?
                DK_SENSE_CHANGEABLE : DK_SENSE_CURRENT));
	ata->atapi.packet.bytes[2] = 0x00;
	ata->atapi.packet.bytes[3] = 0x00;
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00;
	ata->atapi.packet.bytes[6] = (((uchar)cmd_bp->b_bcount >> 8) & 0xff);
	ata->atapi.packet.bytes[7] = ((uchar)cmd_bp->b_bcount & 0xff);
	ata->atapi.packet.bytes[8] = 0x00;
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;

	cmd_ptr->subtype = DK_MSENSE;

	/*
	 * Clear out the sense data buffer
	 */
	bzero(diskinfo->sense_buf, 256);

	/* Place cmd on device's operation stack */
	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
}


/************************************************************************/
/*									*/
/* NAME: idecdrom_format_mode_data					*/
/*									*/
/* FUNCTION:  This function parses a buffer of mode data into a standard*/ 
/*            control structure to prepare the mode data for comparison */   
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*									*/
/*      This routine is  called at the interrupt level and 		*/
/*	cannot page fault.						*/
/*									*/
/* (DATA STRUCTURES:)  idecdrom_diskinfo     Disk's information		*/
/*									*/
/* INPUTS:								*/
/*                    mode_data  - Pointer to mode data			*/
/*                    mf         - Pointer to mode format structure	*/
/*                    sense_length - Length of sense data to format	*/
/*                    over_ride    - Over ride volume control data	*/
/*		      diskinfo	 - Disk device specific information	*/
/*									*/
/* (RECOVERY OPERATION:)  None.						*/
/*									*/
/* RETURNS:	Void.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/************************************************************************/

void idecdrom_format_mode_data(
char *mode_data,
struct idecdrom_mode_format *mf,
int	sense_length,
char    over_ride,
struct idecdrom_diskinfo *diskinfo)
{
	char	i=0,page=0,page_len;
	short	bd_length,offset,p_length,length;

	for (i=0;i<DK_MAX_MODE_PAGES;i++)	
		/* 
		 * initialize all page indices to -1 
		 */
		mf->page_index[i] = (signed char)-1;

	mf->sense_length = sense_length;
	/* 
	 * compute offset to first page (i.e. skip Mode Parameter Header bytes)
	 */
	offset = CD_MODE_HEADER_LEN;   
/*	while (offset < mf->sense_length) { */
	while ((offset < mf->sense_length) && (offset < 256)) {
		/*
		 * For the remainder of the sense data...
		 * Store the index into the data buffer for each page
		 */
		page = (mode_data[offset] & DK_MAX_MODE_PAGES);	
		mf->page_index[page] = offset;
		p_length = mode_data[++offset];
		page_len = mode_data[offset];
		if (over_ride == TRUE) {
			if ((page == 0xe) && 
				 (p_length >= DK_VOL_START_PG_E)) {
			  	/*
				 * Save off config data base settings
				 * for page 0xE's volume control and
				 * output port map.
				 */
			  	if (p_length > DK_MAX_SIZE_PG_E) {
					length = DK_MAX_SIZE_PG_E;
				}
				else {
				  	length = p_length;
				}
				for (i=DK_VOL_START_PG_E;i<=length;i++) {
				   diskinfo->mode_page_e[i-DK_VOL_START_PG_E] = 
					   mode_data[offset + i];
			        }
					  

			}
		}
		for (i=0;i<=page_len;i++) {
			/*
			 * step over the data bytes for this page
			 */
			++offset;
		}
	}
	return;
}
		

/************************************************************************/
/*									*/
/* NAME: idecdrom_mode_data_compare					*/
/*									*/
/* FUNCTION:  Parses a devices changable mode parameters, current mode 	*/
/* 	      parameters, and the using systems desired mode parameters */
/* 	      to determine if a mode select is necessary.		*/
/*									*/
/* NOTES:    This routine uses 4 sets of mode data.  For each of 	*/
/*           these, the driver uses an idecdrom_mode_format structure.	*/
/*	     This structure contains an array (called page_index), which*/
/*	     gives the offset into a buffer of each mode data page.  A	*/
/*	     value of -1 in this array indicates the mode data page	*/
/*	     is not in the buffer.  The routine 			*/
/*	     idecdrom_format_mode_data is responsible for setting up the*/
/*	     idecdrom_mode_format structure.  The 4 types of mode	*/
/*	     data used by this driver and their corresponding 		*/
/*	     structures are:						*/
/*									*/
/*   		       - Desired Mode Data (this is sent to driver	*/
/*			 via the config method.  The config method	*/
/*			 uses the ODM mode_data attribute to set this.	*/
/*			 The diskinfo->dd field is the corresponding	*/
/*			 idedcrom_mode_format structure.  The buffer	*/
/*			 used for this mode data is diskinfo->mode_buf.	*/
/*			  						*/
/*									*/
/*		       - Default Mode Data (this is sent to driver	*/
/*			 via the config method.  The config method	*/
/*			 uses the ODM mode_default attribute to set 	*/
/*			 this. The diskinfo->df field is the            */
/*			 corresponding idedcrom_mode_format structure. 	*/
/*			 The buffer used for this mode data is 	  	*/
/*		 	 diskinfo->df_data.  				*/
/*									*/
/*		       - Changeable Mode Data. The driver fetches 	*/
/*		         this information via a mode sense which	*/
/*			 asks for just changeable mode data.		*/
/*			 The diskinfo->ch field is the corresponding	*/
/*			 idecdrom_mode_format structure. The buffer	*/
/*			 used for this mode data is diskinfo->ch_data.	*/
/*									*/
/*		       - Current Mode Data. The driver fetches 		*/
/*		         this information via a mode sense which	*/
/*			 asks for just current mode data.		*/
/*			 The diskinfo->cd field is the corresponding	*/
/*			 idecdrom_mode_format structure. The buffer	*/
/*			 used for this mode data is diskinfo->sense_buf	*/
/*			 (since the current data was the last data	*/
/*			 received).					*/
/*									*/
/*									*/ 
/*  ASSUMPTION:	 This code assumes that the the page length of the	*/
/*		 changeable mode data is less then or equal to the	*/
/* 		 page length of the current mode data for each page.  	*/
/*		 If this assumption is violated then the mode data	*/
/*		 for the mode select will be wrong.			*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT: This routine is called at the interrupt level */
/*            and can not page fault.					*/
/*									*/
/* DATA STRUCTURES: idecdrom_diskinfo	- Disk information structure	*/
/*		    idecdrom_mode_format- Mode data control information	*/
/*									*/
/* INPUTS:								*/
/*		      diskinfo	 - Disk device specific information	*/
/*									*/
/* (RECOVERY OPERATION:)   None.					*/
/*									*/
/* RETURNS:   0 - No mode select is necessary (or wouldn't do any good)	*/
/*	      1 - Perform mode select with altered current data		*/
/*									*/
/* EXTERNAL PROCEDURES CALLED:						*/
/*		None.							*/
/*									*/
/************************************************************************/

int idecdrom_mode_data_compare(
struct idecdrom_diskinfo *diskinfo)
{

	int 	select = 0;
	char	page,i,ch_index,cd_index,dd_index,df_index,made_change;
	uchar	diff,changeable,def;
	short	ch_length,dd_length,df_length,cd_length;
	int	offset = 0; /* offset into the select_buf being built */  



	/*
	 *                      Mode Parameter Header
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+=======================================================|
	 * | 0   |                Mode Data Length                       |
	 * |-----+--                                                   --|
	 * | 1   |                                                       |
	 * |-----+-------------------------------------------------------|
	 * | 2   |                Medium Type                            |
	 * |-----+-------------------------------------------------------|
	 * | 3   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 4   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 5   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 6   |                Reserved                               |
	 * |-----+--                                                   --|
	 * | 7   |                                                       |
	 * +=============================================================+
	 *
	 */

	/*
	 * Check each changeable page
	 */

	/*
	 *                       Mode Page Format
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+======+======+=========================================|
	 * | 0   |  PS  |Reserv|       Page Code                         |
	 * |-----+-------------------------------------------------------|
	 * | 1   |                     Page Length                       |
	 * |-----+-------------------------------------------------------|
	 * | 2   |                                                       |
	 * |-----+--                   Mode Parameters                ---|
	 * | n   |                                                       |
	 * +=============================================================+
	 */

	diskinfo->number_pages_to_select = 0;
	for (page=0;page<DK_MAX_MODE_PAGES;page++) {
	    made_change = FALSE; /* flag to tell us whether we changed page*/
	    if (diskinfo->ch.page_index[page] != (signed char) -1) {
		/*
		 * if this page is supported
		 */

		/*
		 * get changeable data for this page
		 */

		ch_index = diskinfo->ch.page_index[page];
		ch_length = diskinfo->ch_data[ch_index+1];

		/*
		 * get current data for this page
		 */

		cd_index = diskinfo->cd.page_index[page];
		if (cd_index == (signed char) -1) {
			/*
			 * If the current data doesn't contain
			 * this page then continue to the next
			 * page.
			 */
			continue;
		}
		cd_length = diskinfo->sense_buf[cd_index+1];

		/*
		 * The following assertion should never happen,
		 * since it doesn't make sense for the page length
		 * of the changeable mode data to be longer then
		 * the page length of the same page in the current
		 * values mode data.
		 */

		ASSERT(cd_length>=ch_length);

		/* 
		 * mask off reserved bits 6 and 7 of the page code
		 */
		diskinfo->sense_buf[cd_index] &= DK_MAX_MODE_PAGES; 
		if (diskinfo->dd.page_index[page] != (signed char) -1) {
			/*
			 * if this page is in our desired data base pages, 
			 * then we can and may potentially change this page 
			 * of data.
			 */
			dd_index = diskinfo->dd.page_index[page];
			dd_length = diskinfo->mode_buf[dd_index+1];

			if (dd_length < ch_length) {
				/*
				 * if our data base has fewer bytes for this
				 * page than the device supports, only look at
				 * those bytes.
				 */
				ch_length = dd_length;
			}
			for (i=2;i < ch_length+2; i++) {
				/*
				 * for each changeable byte of this page
				 */
                                def = 0;
                                if (diskinfo->df.page_index[page] !=
                                    (signed char) -1) {
                                    /*
                                     * if there is default data specified
                                     * for this page
                                     */
                                    df_index = diskinfo->df.page_index[page];
                                    df_length = diskinfo->df_data[df_index +1];
                                    if (i < (df_length+2))
                                          /*
                                           * if there is default data specified
                                           * for this byte
                                           */
                                          def = diskinfo->df_data[df_index+i];
                                }

				/*
				 * diff = a bitmask of bits we would like to
				 * change. (i.e. current_data XOR desired_data)
				 */
				diff = diskinfo->sense_buf[cd_index + i] ^ 
				       diskinfo->mode_buf[dd_index + i];
				/*
				 * changeable = a bitmask of bits we would like
				 * to change and can change. (i.e. difference
				 * between current and desired ANDed with 
				 * changeable bits)
				 */
				changeable = 
				    diff & diskinfo->ch_data[ch_index + i];
                                /*
                                 * Now, make sure we don't clobber any bits
                                 * that the data base has specified that we
                                 * should use device default parameters
                                 */
                                changeable &= ~def;
				
				if (changeable) {
				    /*
				     * means we would like to make a change
				     */
				    /*
				     * Clear and Set desired bits
				     * (i.e. current data XOR changeable)
				     */
				    diskinfo->sense_buf[cd_index +i] ^=
					changeable;
				    select = 1;
				    made_change = TRUE;
				} /* if changeable */
			} /* for each byte of this page */
		} /* if this page in data base */	
            	if (!made_change) {
			/*
			 * we didn't change anything in this
			 * page, so shift it out of the current and
			 * changeable data
			 */
			ch_index = diskinfo->ch.page_index[page];
			ch_length = diskinfo->ch_data[ch_index+1];

			/*
			 * Shift changeable mode data page out.
			 */
			for (i=ch_index;i<diskinfo->ch.sense_length;i++)
			 	diskinfo->ch_data[i] = 
			 		diskinfo->ch_data[i+ch_length+2];

			cd_index = diskinfo->cd.page_index[page];
			cd_length = diskinfo->sense_buf[cd_index+1];

			/*
			 * Shift current mode data page out.
			 */
			for (i=cd_index;i< diskinfo->cd.sense_length;i++)
			 	diskinfo->sense_buf[i] = 
			 		diskinfo->sense_buf[i+cd_length+2];

			/*
			 * Rebuild page index for modified changeable mode
			 * data (i.e. with mode page shifted out).
			 */
			idecdrom_format_mode_data((char *) diskinfo->ch_data,
			    (struct idecdrom_mode_format *) &diskinfo->ch,
				       (int)(diskinfo->ch.sense_length - 
					     (ch_length + 2)),
					      (char) FALSE,
					      (struct idecdrom_diskinfo *)NULL);

			/*
			 * Rebuild page index for modified current mode
			 * data (i.e. with mode page shifted out).
			 */
			idecdrom_format_mode_data((char *) diskinfo->sense_buf, 
			    (struct idecdrom_mode_format *)&diskinfo->cd, 
			    (int)(diskinfo->cd.sense_length - (cd_length + 2)),
			    (char) FALSE,
			    (struct idecdrom_diskinfo *) NULL);
		} else {

			/* The page has changed so put in select buffer */
			diskinfo->number_pages_to_select++;
			diskinfo->select_page_offset[diskinfo->number_pages_to_select] =
				offset;
			/* Set header bytes */
			/* Mode Data Length is reserved so equal zero */
			/* Medium Type is obtained from sense buf */
			for (i=0; i<CD_MODE_HEADER_LEN; i++) {
				diskinfo->select_buf[offset] = 0;
				offset++;
			}
			diskinfo->select_buf[offset-6] = diskinfo->sense_buf[2];
			/* Set mode select page parameters */
			for (i=0; i<cd_length+2; i++) {
				diskinfo->select_buf[offset] =
					diskinfo->sense_buf[cd_index+i];
				offset++;
			}
		}
	    } /* if this page supported */	
	} /* for each possible page */
	return(select);
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_mode_select					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a MODE SELECT cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and idecdrom_start is called	*/
/*		to initiate execution of the cmd.			*/
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
/*									*/
/************************************************************************/

void
idecdrom_mode_select(
struct idecdrom_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;
	uchar	offset;
	ushort	length;
	uchar	page_code;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(modeselect, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
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
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }
	offset = diskinfo->select_page_offset[diskinfo->number_pages_to_select];
	diskinfo->number_pages_to_select--;
	page_code = diskinfo->select_buf[offset+CD_MODE_HEADER_LEN];
	length = (ushort)diskinfo->select_buf[offset+CD_MODE_HEADER_LEN+1] +
	         2 + CD_MODE_HEADER_LEN;

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for mode select cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_WRITE | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = length;
	cmd_bp->b_un.b_addr = (caddr_t)&(diskinfo->select_buf[offset]);
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
	cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */

     /*
      *                            MODE SELECT  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (55h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   |       Reserved        |   1   |       Reserved         |  SP  | 
      *|-----+---------------------------------------------------------------|
      *| 2   |                          Page Code                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                     Paramenter List Length (MSB)              |
      *|-----+---------------------------------------------------------------|
      *| 8   |                     Paramenter List Length (LSB)              |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 10  |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 11  |                           Reserved                            |
      *+=====================================================================+
      */

	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->atapi.length = 12;
	ata->startblk.chs.cyl_hi = (((uchar)length >> 8) & 0xff);
	ata->startblk.chs.cyl_lo = ((uchar)length & 0xff);
	ata->atapi.packet.op_code = ATAPI_MODE_SELECT_10;

	ata->atapi.packet.bytes[0] = (0x10 | DK_COMPLIES_PF);
	ata->atapi.packet.bytes[1] = (0x3F & page_code);
	ata->atapi.packet.bytes[2] = 0x00;
	ata->atapi.packet.bytes[3] = 0x00;
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00;
	ata->atapi.packet.bytes[6] = (((uchar)length >> 8) & 0xff); 
	ata->atapi.packet.bytes[7] = ((uchar)length  & 0xff); 
	ata->atapi.packet.bytes[8] = 0x00;
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;

	cmd_ptr->subtype = DK_SELECT;

	/* Place cmd on device's operation stack */
	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_read_cap					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a READ CAPACITY cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and idecdrom_start is called	*/
/*		to initiate execution of the cmd.			*/
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
/*									*/
/************************************************************************/

void
idecdrom_read_cap(
struct idecdrom_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct ata_cmd	*ata;






	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(readcap, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
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
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for read capacity cmd */
	cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x08;
	cmd_bp->b_un.b_addr = (caddr_t) &(diskinfo->capacity);
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())idecdrom_iodone);
	cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->ataidebuf.bp = NULL;
	cmd_ptr->ataidebuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize IDE cmd for operation */
     /*
      *                READ CAPACITY  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (25h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 8   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 10  |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 11  |                           Reserved                            |
      *+=====================================================================+
      */

	/* Initialize IDE cmd for operation */
	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->startblk.chs.cyl_hi = 0x00;
	ata->startblk.chs.cyl_lo = 0x08;
	ata->atapi.length = 12;
	ata->atapi.packet.op_code = ATAPI_READ_CAPACITY;
	ata->atapi.packet.bytes[0] = 0x00;
	ata->atapi.packet.bytes[1] = 0x00;
	ata->atapi.packet.bytes[2] = 0x00;
	ata->atapi.packet.bytes[3] = 0x00;
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00;
	ata->atapi.packet.bytes[6] = 0x00;
	ata->atapi.packet.bytes[7] = 0x00;
	ata->atapi.packet.bytes[8] = 0x00;
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;

	cmd_ptr->subtype = DK_READCAP;

	/* Place cmd on device's operation stack */
	idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	return;
}

/************************************************************************/
/*                                                                      */
/*      NAME:   idedcrom_read_disc_info                                 */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine builds the READ Disc information IDE       */
/*              command which is READ TOC (format = 01b). The cmd block */
/*              is constructed, placed on the top                       */
/*              of the device's cmd queue and idecdrom_start is called  */
/*              to initiate execution of the cmd.                       */
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine is called on an interrupt level and        */
/*              process level.  It cannot page fault.                   */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*              diskinfo        Disk device specific information        */
/*                                                                      */
/*      INPUTS:                                                         */
/*              diskinfo        Disk device specific information        */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns no value.                          */
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*              None.                                                   */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void
idecdrom_read_disc_info(
struct idecdrom_diskinfo  *diskinfo)
{
        struct dk_cmd   *cmd_ptr;
        struct buf      *cmd_bp;
        struct ata_cmd     *ata;
	int		allocation_length = 0x0A;
	uchar		format = 0x40;
	uchar		msf = 0x00; /* return LBA instead of MSF */




        /* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(rdiscinfo, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
              (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
              (uint)cmd_ptr->status);
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
                        idecdrom_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
        cmd_ptr->aborted = FALSE;

        /* Initialize cmd block for read disc info cmd */
        cmd_bp = &(cmd_ptr->ataidebuf.bufstruct);
        cmd_bp->b_dev = diskinfo->adapter_devno;
        cmd_bp->b_flags = (B_READ | B_BUSY);
        cmd_bp->b_options = 0x00;
        cmd_bp->b_work = 0x00;
        cmd_bp->b_error = 0x00;
        cmd_bp->b_blkno = 0x00;
       	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->ioctl_buf;
        if (diskinfo->disc_info == 0x01) {
		allocation_length = 0x0A;
		format = 0x00;
        } else if (diskinfo->disc_info == 0x02) {
		allocation_length = 0x0E;
		format = 0x80;
	} else {
		allocation_length = 0x0C;
		format = 0x40;
	}
        cmd_bp->b_bcount = allocation_length;
        cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
        cmd_bp->b_iodone = ((void(*)()) idecdrom_iodone);
        cmd_ptr->ataidebuf.timeout_value = DK_TIMEOUT;
        cmd_ptr->ataidebuf.bp = NULL;
        cmd_ptr->ataidebuf.status_validity = 0x00;
        cmd_ptr->retry_count = 0;
        cmd_ptr->segment_count = 0x00;
        cmd_ptr->soft_resid = 0x00;
        cmd_ptr->bp = NULL;


       /*
	*          READ TOC Data Format (With Format Field = 01b)
	* +=====-======-======-======-======-======-======-======-======+
	* |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	* |Byte |      |      |      |      |      |      |      |      |
	* |=====+=======================================================|
	* | 0   | (MSB)                                                 |
	* |-----+---                TOC Data Length (0Ah)            ---|
	* | 1   |                                                 (LSB) |
	* |-----+-------------------------------------------------------|
	* | 2   |                   First Session Number                |
	* |-----+-------------------------------------------------------|
	* | 3   |                   Last Session Number                 |
	* |=====+=======================================================|
	* |     |                   TOC Track Descriptor(s)             |
	* |=====+=======================================================|
	* | 0   |                   Reserved                            |
	* |-----+-------------------------------------------------------|
	* | 1   |              ADR           |         Control          |
	* |-----+-------------------------------------------------------|
	* | 2   |                   First Track Number in Last Session  |
	* |-----+-------------------------------------------------------|
	* | 3   |                   Reserved                            |
	* |-----+-------------------------------------------------------|
	* | 4   | (MSB)                                                 |
	* |- - -+---                Absolute CD-ROM Address of       ---|
	* | 7   |                   First Track in Last Session   (LSB) |
	* +=============================================================+
	*/

     /*                         READ TOC Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                     Operation Code (43h)                      |
      *|-----+---------------------------------------------------------------|
      *| 1   |                   Reserved                    |  MSF  |  RSVD |
      *|-----+---------------------------------------------------------------|
      *| 2   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 3   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 4   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 5   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 6   |               Reserved (was Starting Track/Session Number)    |
      *|-----+---------------------------------------------------------------|
      *| 7   | (MSB)                                                         |
      *|-----+---                    Allocation Length                  -----|
      *| 8   |                                                       (LSB)   |
      *|-----+---------------------------------------------------------------|
      *| 9   |    Format     |                  Reserved                     |
      *|-----+---------------------------------------------------------------|
      *| 10  |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 11  |                            Reserved                           |
      *+=====================================================================+
      */

	/* Initialize IDE cmd for operation */
	ata = &(cmd_ptr->ataidebuf.ata);
	ata->command = ATA_ATAPI_PACKET_COMMAND;
	ata->device = diskinfo->device_id;
	ata->feature = 0x00;
	ata->startblk.chs.cyl_hi = (((uchar)allocation_length >> 8) & 0xff);
	ata->startblk.chs.cyl_lo = ((uchar)allocation_length & 0xff);
	ata->atapi.length = 12;
	ata->atapi.packet.op_code = ATAPI_READ_TOC;
	ata->atapi.packet.bytes[0] = (msf & 0x02);
	ata->atapi.packet.bytes[1] = 0x00;
	ata->atapi.packet.bytes[2] = 0x00;
	ata->atapi.packet.bytes[3] = 0x00;
	ata->atapi.packet.bytes[4] = 0x00;
	ata->atapi.packet.bytes[5] = 0x00; /* first track or start from ses. 0*/
	ata->atapi.packet.bytes[6] = ((allocation_length >> 8) & 0xff);
	ata->atapi.packet.bytes[7] = (allocation_length & 0xff);
	ata->atapi.packet.bytes[8] = (format & 0xC0);
	ata->atapi.packet.bytes[9] = 0x00;
	ata->atapi.packet.bytes[10] = 0x00;


        cmd_ptr->subtype = DK_READ_INFO;

        /* Place cmd on device's operation stack */
        idecdrom_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);


        /* Begin execution of this operation */
        idecdrom_start(diskinfo);

        return;
}



/************************************************************************/
/*									*/
/*	NAME:	idecdrom_build_error					*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*		type - Type of error being logged.			*/
/*		priority - Priority of error being logged.		*/
/*		valid_sense - Flag to decide whether to copy sense data	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bcopy		bzero					*/
/*									*/
/*									*/
/************************************************************************/

void
idecdrom_build_error(
struct dk_cmd	*cmd_ptr,
int		type,
int		priority,
int		valid_sense)
{
	struct idecdrom_diskinfo	*diskinfo;
	struct ataide_buf		*ataidebuf;
	struct idecdrom_error_rec	*erec;

	diskinfo = cmd_ptr->diskinfo;
	if ((cmd_ptr->error_type == 0) ||
	    (priority == DK_OVERLAY_ERROR)) {

		/* Store type for use by logger */
		cmd_ptr->error_type = type;

		/* Setup pointers for faster access */
		ataidebuf = &(cmd_ptr->ataidebuf);
		erec = &(diskinfo->error_rec);

		erec->status_validity = ataidebuf->status_validity;
		erec->ata_status = ataidebuf->ata.status;
		erec->ata_error = ataidebuf->ata.errval;
		erec->reserved1 = 0x00;

		bcopy(&(ataidebuf->ata.atapi.packet.op_code), &(erec->cmd), 
		    sizeof(struct atpkt_cmd));

		if (valid_sense & SC_VALID_SENSE) {
			bcopy(diskinfo->sense_buf, erec->req_sense_data, 128);
		}
		else {
			bzero(erec->req_sense_data, 128);
		}
		erec->reserved2 = diskinfo->stats.segment_count;
		erec->reserved3 = diskinfo->stats.byte_count;
	}
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_log_error					*/
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
/*		cmd_ptr-Address of ataidebuf struct describing operation*/
/*		sev    - severity of error (hard error or not)		*/
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
/*									*/
/************************************************************************/

void
idecdrom_log_error(
struct dk_cmd	*cmd_ptr,
int		sev)
{
	struct idecdrom_error_rec	*erec;
	struct idecdrom_diskinfo	*diskinfo;

	/* Setup pointers for faster access */
	diskinfo = (struct idecdrom_diskinfo *) cmd_ptr->diskinfo;
	erec = &(cmd_ptr->diskinfo->error_rec);
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(logerr, trc,(char)0, (uint)diskinfo, (uint)cmd_ptr->error_type,
	      (uint)cmd_ptr, (uint)cmd_ptr->status,(uint)sev);
#endif	
#endif
	switch(cmd_ptr->error_type) {
	case DK_MEDIA_ERR:
		if (sev == DK_HARD_ERROR)
			erec->entry.error_id = ERRID_DISK_ERR1;
		else
			erec->entry.error_id = ERRID_DISK_ERR4;
		break;
	case DK_HARDWARE_ERR:
		if (sev == DK_HARD_ERROR)
			erec->entry.error_id = ERRID_DISK_ERR2;
		else
			erec->entry.error_id = ERRID_DISK_ERR4;
		break;
	case DK_RECOVERED_ERR:
		erec->entry.error_id = ERRID_DISK_ERR4;
		break;
	case DK_UNKNOWN_ERR:
	default:
		if (sev == DK_HARD_ERROR)
			erec->entry.error_id = ERRID_DISK_ERR5;
		else
			erec->entry.error_id = ERRID_DISK_ERR4;
		break;
	}

	cmd_ptr->error_type = 0;
	errsave(erec, sizeof(struct idecdrom_error_rec));
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_watchdog					*/
/*									*/
/*	FUNCTION:							*/
/*		When a UNIT ATTENTION is encountered, a 		*/
/*		timeout is invoked and this routine is called when	*/
/*		the timer pops.  Notice that the watchdog_timer field	*/
/*		in the diskinfo structure must be at the beginning so	*/
/*		the address of the diskinfo structure can be determined.*/
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
/*		watchdog_timer - pointer to watchdog structure at the	*/
/*				 top of the diskinfo structure.		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		None.						 	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		i_disable	i_enable				*/
/*									*/
/*									*/
/************************************************************************/
void idecdrom_watchdog(
struct watchdog	*watchdog_timer)
{
	struct idecdrom_timer     *timer;
	struct idecdrom_diskinfo	*diskinfo;
	uint	      		old_pri;




	/* 
	 * disable my irpt level and spin lock
	 */
	old_pri = i_disable(INTIODONE);
	timer = (struct idecdrom_timer *) (watchdog_timer);
	diskinfo = (struct idecdrom_diskinfo *)(timer->pointer);


	/*
	 *  If this timeout resulted from busy scsi status, we want to retry
	 *  the original command.  Otherwise the timeout came from a unit
	 *  attention, and the test unit ready needs to be issued to restart the
	 *  device.
	 */

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(cmdtimer, entry,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,
	      (uint)diskinfo->timer_status);
#endif	
#endif

	if (diskinfo->timer_status & DK_TIMER_BUSY) {
		diskinfo->timer_status = 0;
		idecdrom_start(diskinfo);
	}
	else {
		diskinfo->timer_status = 0;
		idecdrom_test_unit_ready(diskinfo);
	}
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(cmdtimer, exit,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,(uint)0);
#endif	
#endif		
	
	i_enable(old_pri);

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	idecdrom_cdt_func					*/
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
/*		bcopy							*/
/*									*/
/*									*/
/************************************************************************/
struct cdt *idecdrom_cdt_func(
int	arg)
{
	struct idecdrom_diskinfo	*diskinfo;
	struct cdt_entry	*entry_ptr;
	int			entry_count, i;

#ifdef DEBUG
	DKprintf(("Entering idecdrom_cdt_func arg[0x%x]\n", 
	    arg));
#endif
	if (arg == 1) {
		/*  First dump entry is global info structure */
		entry_ptr = &idecdrom_info.cdt->cdt_entry[0];
		bcopy("cdinfo", entry_ptr->d_name, 7);
		entry_ptr->d_len = sizeof(struct idecdrom_info);
		entry_ptr->d_ptr = (caddr_t) &idecdrom_info;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Second dump entry is configured disk list */
		entry_ptr = &idecdrom_info.cdt->cdt_entry[1];
		bcopy("cfglist", entry_ptr->d_name, 8);
		entry_ptr->d_len = 4 * DK_MAXCONFIG;
		entry_ptr->d_ptr = (caddr_t) idedisk_list;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Third dump entry is opened disk list */
		entry_ptr = &idecdrom_info.cdt->cdt_entry[2];
		bcopy("oplist", entry_ptr->d_name, 7);
		entry_ptr->d_len = 4 * DK_MAXCONFIG;
		entry_ptr->d_ptr = (caddr_t) idedisk_open_list;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Rest of entries are disk structures */
		entry_count = 2;
		for (i = 0 ; i < DK_MAXCONFIG ; i++) {
			diskinfo = idedisk_list[i];
			while (diskinfo != NULL) {
				entry_ptr = &idecdrom_info.cdt->cdt_entry[
				    ++entry_count];
				bcopy(diskinfo->error_rec.entry.resource_name,
				    entry_ptr->d_name, 8);
				entry_ptr->d_len = sizeof(struct 
				    idecdrom_diskinfo);
				entry_ptr->d_ptr = (caddr_t) diskinfo;
				entry_ptr->d_segval = (int) NULL;
				diskinfo = diskinfo->next;
			}
		}
	}

#ifdef DEBUG
	DKprintf(("Exiting idecdrom_cdt_func\n"));
#endif
	return(idecdrom_info.cdt);
}

/*************************************************************************
 *
 * NAME: idecdrom_q_cmd
 *
 * FUNCTION: Places a command block on the specified disk's queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)    idecdrom_diskinfo  - Disk device specific information
 *		         dk_cmd           - Command block
 *
 * INPUTS:
 *		cmd_ptr		pointer to command to be queued
 *		queue		specifies QUEUE or STACK request
 *		which_q		indicates the target queue
 *
 * RETURNS:     Void.
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		None.
 *
 *******************************************************************/

void idecdrom_q_cmd(struct  dk_cmd *cmd_ptr,
		  char    queue,
		  uchar	  which_q)
{
	struct idecdrom_diskinfo *diskinfo = NULL; /* disk device information */
						 /* structure		    */
        struct dk_cmd **head_ptr,
                      **tail_ptr;


        ASSERT(cmd_ptr);

        diskinfo = cmd_ptr->diskinfo;             /* set disk pointer */


        cmd_ptr->next = NULL;/* set next to null, this will be put on end */
	cmd_ptr->prev = NULL;    

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(qcmd, entry,(char)0, (uint)diskinfo, (uint)queue,
	      (uint)which_q, (uint)cmd_ptr->status,(uint)cmd_ptr->type);
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
		}
                else {
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
		}
                else {
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
   idecdrom_trc(qcmd, exit,(char)0, (uint)cmd_ptr, (uint)cmd_ptr->diskinfo,
	      (uint)which_q, (uint)cmd_ptr->prev,(uint)cmd_ptr->next);
#endif	
#endif
	return;
}
/**********************************************************************
 *
 * NAME: idecdrom_d_q_cmd_disable
 *
 * FUNCTION: Removes the specified command from the specified disk's  queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. Plus from close on the process level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)  idecdrom_diskinfo  - Disk device specific information
 *		       dk_cmd           - Command block
 *
 * INPUTS:
 *		cmd_ptr		pointer to command to be queued
 *		which_q		indicates where to queue cmd_ptr
 *
 * RETURNS:     Void.
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		i_disable	i_enable	
 *
 ********************************************************************/

void idecdrom_d_q_cmd_disable(struct  dk_cmd *cmd_ptr,
		    uchar    which_q)
{

	uint    old_pri;			/* Old interrupt priority*/


        /*
         * disable interrupts and spin lock
         */

	old_pri = i_disable(INTIODONE);
	idecdrom_d_q_cmd(cmd_ptr, which_q);

        /*
         * enable interrupts and unlock
         */
	i_enable(old_pri);

	return;
}

/*********************************************************************
 *
 * NAME: idecdrom_d_q_cmd
 *
 * FUNCTION: Removes the specified command from the specified disk's  queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. Plus from close on the process level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)  idecdrom_diskinfo  - Disk device specific information
 *		       dk_cmd           - Command block
 *
 * INPUTS:
 *		cmd_ptr		pointer to command to be queued
 *		which_q		indicates where to queue cmd_ptr
 *
 * RETURNS:     Void.
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		None.
 *
 ********************************************************************/

void idecdrom_d_q_cmd(struct  dk_cmd *cmd_ptr,
		    uchar    which_q)
{
	struct idecdrom_diskinfo *diskinfo = NULL; /* disk device information */
						 /* structure		    */

        struct dk_cmd **head_ptr,
                      **tail_ptr;


        ASSERT(cmd_ptr);
        diskinfo = cmd_ptr->diskinfo;             /* set disk pointer */

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
   idecdrom_trc(dqcmd, entry,(char)0, (uint)cmd_ptr, (uint)cmd_ptr->diskinfo,
	      (uint)which_q, (uint)cmd_ptr->status,(uint)cmd_ptr->type);
#endif	
#endif

	if (which_q == DK_CMD_Q) {
		head_ptr = &(diskinfo->dk_cmd_q_head);
		tail_ptr = &(diskinfo->dk_cmd_q_tail);
		cmd_ptr->status &= ~DK_QUEUED;
	} 
	else {
		ASSERT(which_q == DK_CMD_Q);
	}
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(dqcmd, trc,(char)0, (uint)cmd_ptr, (uint)*head_ptr,
	      (uint)which_q, (uint)cmd_ptr->prev,(uint)cmd_ptr->next);
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
   idecdrom_trc(dqcmd, exit,(char)0, (uint)cmd_ptr, (uint)cmd_ptr->diskinfo,
	      (uint)*head_ptr, (uint)cmd_ptr->prev,(uint)cmd_ptr->next);
#endif	
#endif
        return;
}

/************************************************************************
 *
 * NAME: idecdrom_cmd_alloc_disable
 *
 * FUNCTION: Allocates a command block if available for the routines that
 *	     have not disabled and spin locked.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If no cmd structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   idecdrom_devinfo  - disk info structure
 *                      dk_cmd          - Command structure
 *
 * INPUTS:
 *		diskinfo	pointer to disk info structure
 *		cmd_type	indicates command type
 *
 * RETURNS:
 *           Pointer to allocated cmd   - successful completion
 *           NULL                       - no free structures
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		i_disable	i_enable	
 *
 ***********************************************************************/
struct  dk_cmd *idecdrom_cmd_alloc_disable(struct idecdrom_diskinfo *diskinfo,
				 uchar cmd_type)
{
	uint    old_pri;			/* Old interrupt priority*/
	struct dk_cmd *cmd_ptr;

        /*
         * disable interrupts and spin lock
         */
	old_pri = i_disable(INTIODONE);
	cmd_ptr = idecdrom_cmd_alloc(diskinfo,cmd_type);

        /*
         * enable interrupts and unlock
         */
	i_enable(old_pri);
	return(cmd_ptr);

}
/********************************************************************
 * NAME: idecdrom_cmd_alloc
 *
 * FUNCTION: Allocates a command block if available
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If no cmd structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   idecdrom_diskinfo  - disk info structure
 *                      dk_cmd          - Command structure
 *
 * INPUTS:
 *		diskinfo	pointer to disk info structure
 *		cmd_type	indicates command type
 *
 * RETURNS:
 *           Pointer to allocated cmd   - successful completion
 *           NULL                       - no free structures
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		None.
 *
 **********************************************************************/
struct  dk_cmd *idecdrom_cmd_alloc(struct idecdrom_diskinfo *diskinfo,
				 uchar cmd_type)
{
	int     counted_cmds = 0;               /* Number of cmd_ptr's counted*/
	struct dk_cmd *pool;			/* Pointer to pool of command */
						/*  blocks		      */
	struct dk_cmd *cmd_ptr;
	int     found = FALSE;
        struct buf      *cmd_bp;
	struct ata_cmd	*ata;



	cmd_ptr = NULL;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(cmdalloc, trc,(char)0, (uint)diskinfo, (uint)cmd_type,
	      (uint)diskinfo->pool_index, (uint)0,(uint)counted_cmds);
#endif	
#endif
	if (cmd_type == DK_BUF) {
		pool = &diskinfo->cmd_pool[0];
		while (counted_cmds < (diskinfo->queue_depth)) {
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
				found = TRUE;
				break;
			}
			else {
				if ((diskinfo->pool_index +1) == diskinfo->queue_depth)
					diskinfo->pool_index = 0;
				else
					diskinfo->pool_index++;
			}
			counted_cmds++;

		}
		if (found) {
			cmd_ptr = &pool[diskinfo->pool_index];

		}
		else
			cmd_ptr = NULL;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
   idecdrom_trc(cmdalloc, trc,(char)1, (uint)diskinfo, 
	      (uint)diskinfo->queue_depth,(uint)diskinfo->pool_index, 
	      (uint)cmd_ptr,(uint)counted_cmds);
#endif	
#endif
	}
	else if ((cmd_type == DK_IOCTL) && (!(diskinfo->ioctl_cmd.status))) {
		cmd_ptr = &diskinfo->ioctl_cmd;
		cmd_ptr->status = DK_IN_USE;
		cmd_ptr->aborted = FALSE;
		cmd_ptr->type = cmd_type;
		cmd_ptr->next = NULL;
		cmd_ptr->prev = NULL;

	}

	if (cmd_ptr != NULL) {
		/*	
		 * Set lun for cmd_ptr
	  	 */	

		cmd_ptr->ataidebuf.ata.device = diskinfo->device_id;
	}

	return(cmd_ptr);
	
}
/*******************************************************************
 *
 * NAME: idecdrom_free_cmd_disable
 *
 * FUNCTION: Frees a Command structure for the routines that
 *	     have not disabled and spin locked.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If Command pointer to free is NULL, return.
 *
 * (DATA STRUCTURES:)   idecdrom_diskinfo  - disk info structure
 *                      dk_cmd             - Command structure
 *
 * INPUTS:
 *		cmd_ptr		pointer to command to be freed
 *
 * RETURNS:     Void.
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		i_disable	i_enable	
 *
 **********************************************************************/
void idecdrom_free_cmd_disable(
struct dk_cmd *cmd_ptr)
{
        uint    old_pri;

        /*
         * disable interrupts and spin lock
         */
	old_pri = i_disable(INTIODONE);

	idecdrom_free_cmd(cmd_ptr);
        /*
         * enable interrupts and unlock
         */
	i_enable(old_pri);
	return;

}  
/*********************************************************************
 *
 * NAME: idecdrom_free_cmd
 *
 * FUNCTION: Frees a Command structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If Command pointer to free is NULL, return.
 *
 * (DATA STRUCTURES:)   idecdrom_diskinfo  - disk info structure
 *                      dk_cmd             - Command structure
 *
 * INPUTS:
 *		cmd_ptr		pointer to command to be freed
 *
 * RETURNS:     Void.
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		bzero
 *
 *********************************************************************/
void idecdrom_free_cmd(
struct dk_cmd *cmd_ptr)
{
        struct  idecdrom_diskinfo *diskinfo;


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
   idecdrom_trc(cmdfree, trc,(char)0, (uint)diskinfo, (uint)0,
	      (uint)cmd_ptr->status, (uint)cmd_ptr->type,(uint)0);
#endif	
#endif
 
	if (cmd_ptr->type == DK_RESET) {
		cmd_ptr->status = DK_FREE;

		return;
	}
	
        bzero(cmd_ptr, sizeof(struct dk_cmd)); /* clear entire command struc */
        cmd_ptr->diskinfo = diskinfo;          /* restore diskinfo pointer   */
	cmd_ptr->ataidebuf.bufstruct.b_event = EVENT_NULL;
	
	return;
}  /* end idecdrom_free_cmd */


/**********************************************************************
 *
 * NAME: idecdrom_sleep
 *
 * FUNCTION: Used to sleep on an event
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 *
 *
 * (DATA STRUCTURES:)  struct buf          - buf structure for I/O
 *                     
 *
 * INPUTS:
 *		intrpt		pointer to flag
 *		event		pointer to event upon which to wait
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *     i_disable                i_enable
 *     e_sleep
 *
 * RETURNS:      Void
 *******************************************************************/

void idecdrom_sleep(
uchar   *intrpt,
uint    *event)
{
        int    old_pri;   /* old interrupt level */



	old_pri = i_disable(INTIODONE);
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


/********************************************************************
 *
 * NAME: idecdrom_start_watchdog
 *
 * FUNCTION: Start the disk's watchdog timer to wait before issuing
 *	     any new commands
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 *
 *
 * (DATA STRUCTURES:)  struct idecdrom_diskinfo    disk's information
 *                     
 *
 * INPUTS:
 *		diskinfo	pointer to disk info structure
 *		timeout		timeout in seconds
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *     w_start
 *
 * RETURNS:      Void
 *********************************************************************/

void idecdrom_start_watchdog(struct idecdrom_diskinfo *diskinfo,
			   ulong timeout)
{




	if (diskinfo->timer_status)  {
		/*
		 * If watchdog timer is already set for this
		 * disk then do not restart it.
		 */
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(sttimer, trc,(char)0, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,
	      (uint)0);
#endif	
#endif
		return;
	}
#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(sttimer, trc,(char)1, (uint)diskinfo, (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,
	      (uint)0);
#endif	
#endif
	diskinfo->timer_status |= DK_TIMER_PENDING;
	diskinfo->watchdog_timer.watch.restart = timeout;
	w_start(&(diskinfo->watchdog_timer.watch));



        return;
}

/************************************************************************/
/*                                                                      */
/*      NAME:   idecdrom_raw_iodone                                     */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine will be called whenever a command built    */
/*              for diskinfo->raw_io_cmd is iodoned. It just wakes up   */
/*              the sleeping process thread in idedcrom_raw_io.         */
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine is called on an interrupt level.           */
/*              It cannot page fault.                                   */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              raw_bp - A pointer to the raw buf   			*/
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns a zero value.                      */
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*              i_disable	i_enable                                */
/*              e_wakeup                                                */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void
idecdrom_raw_iodone(
struct dk_raw_buf *raw_bp)
{

        uint    old_pri;

	old_pri = i_disable(INTIODONE);


        /* 
	 * Wakeup sleeping process 
	 */
	raw_bp->diskinfo->raw_io_intrpt =  0;
        e_wakeup(&(raw_bp->bp.b_event));

	i_enable(old_pri);

        return;
}

#ifdef DEBUG
/**********************************************************************
 *
 * NAME: idecdrom_trc_disable
 *
 * FUNCTION: Builds an internal Trace Entry for Debug for the routines that
 *	     have not disabled and spin locked. 
 *	     
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 *
 * EXTERNAL PROCEDURES CALLED:
 *     i_disable                i_enable
 *
 * (DATA STRUCTURES:) None.
 *
 * INPUTS:
 *		desc		points to string identifying trace source
 *		type		points to string describing entry type
 *		count		number of values to store in entry
 *		word1-word5	uint-length values to store in trace entry
 *
 * RETURNS:     Void.
 *********************************************************************/

void idecdrom_trc_disable(
char    *desc,
char    *type,
char    count,
uint    word1,
uint    word2,
uint    word3,
uint    word4,
uint    word5)
{
        uint    old_pri;

	old_pri = i_disable(INTIODONE);
	idecdrom_trc(desc,type,count,word1,word2,word3,word4,word5);

	i_enable(old_pri);

}
/**********************************************************************
 *
 * NAME: idecdrom_trc
 *
 * FUNCTION: Builds an internal Trace Entry for Debug.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      bcopy
 *
 * (DATA STRUCTURES:) None.
 *
 * INPUTS:
 *		desc		points to string identifying trace source
 *		type		points to string describing entry type
 *		count		number of values to store in entry
 *		word1-word5	uint-length values to store in trace entry
 *
 * RETURNS:     Void.
 ********************************************************************/

void idecdrom_trc(
char    *desc,
char    *type,
char    count,
uint    word1,
uint    word2,
uint    word3,
uint    word4,
uint    word5)
{
        uint    old_pri;

	bcopy(desc,idecdrom_trace[idecdrom_trcindex].desc, IDECDROM_TRC_STR_LENGTH);
        bcopy(type,idecdrom_trace[idecdrom_trcindex].type, 3);
        idecdrom_trace[idecdrom_trcindex].count = count;
        idecdrom_trace[idecdrom_trcindex].word1 = word1;
        idecdrom_trace[idecdrom_trcindex].word2 = word2;
        idecdrom_trace[idecdrom_trcindex].word3 = word3;
        idecdrom_trace[idecdrom_trcindex].word4 = word4;
        idecdrom_trace[idecdrom_trcindex].word5 = word5;
        if(++idecdrom_trcindex >= TRCLNGTH)
                idecdrom_trcindex = 0;
        bcopy(topoftrc,idecdrom_trace[idecdrom_trcindex].desc, 
	      IDECDROM_TRC_STR_LENGTH);
        idecdrom_trctop = (struct idecdrom_trace *)&idecdrom_trace[idecdrom_trcindex];

}

#endif


/************************************************************************
 *
 *	NAME:	idecdrom_pm_watchdog
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
void idecdrom_pm_watchdog(
	struct watchdog		*watchdog_timer)
{
	struct idecdrom_timer	*timer;
	struct idecdrom_diskinfo	*diskinfo;
	uint			old_pri;

	timer = (struct idecdrom_timer *) (watchdog_timer);
	diskinfo = (struct idecdrom_diskinfo *) (timer->pointer);

	old_pri = i_disable(INTTIMER);

#ifdef DEBUG
#ifdef IDE_ERROR_PATH
   idecdrom_trc(pmh_timer, trc, (char) 0,
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
	 * IDECDROM_PM_PENDING_OP flag here.
	 */
	e_wakeup( (int *) &(diskinfo->pm_event) );

	i_enable(old_pri);

	return;
}

/************************************************************************
 *
 *	NAME:	idecdrom_spindown_closeddisk
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
 *		struct idecdrom_diskinfo disk's information
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
idecdrom_spindown_closeddisk(
	struct idecdrom_diskinfo	*diskinfo)
{
	int	errnoval = 0;
	int	opri;

	/* 
	 * Open adapter for device and store filepointer
	 */
	errnoval = fp_opendev(diskinfo->adapter_devno, (int) FREAD,
			      (caddr_t) NULL, (int) NULL, &(diskinfo->fp));
	
	if (errnoval == 0) {
		/* 
		 * Issue a start IOCTL to the 
		 * adapter driver 
		 */
		
		errnoval = fp_ioctl(diskinfo->fp, (uint) IDEIOSTART,
				    (caddr_t) diskinfo->device_id, (int) NULL);

		if (errnoval == 0) {

			diskinfo->errno = 0x00;
			
			diskinfo->disk_intrpt = 1;
			idecdrom_start_unit_disable(diskinfo,
					  (uchar) DK_STOP);
			idecdrom_sleep(&(diskinfo->disk_intrpt),
					&(diskinfo->open_event));

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
 *	NAME:	idecdrom_pm_handler
 *
 *	FUNCTION:
 *		IDE CDROM Power Management function.
 *
 *	EXECUTION ENVIRONMENT:
 *		This routine is called in the process environment.  It
 *		can not page fault and is pinned
 *
 *	NOTES:
 *		The device must be opened in order to issue any
 *		ATAPI commands to the device.  If it is not open then
 *		all ATAPI commands will be failed by the adapter driver,
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

int idecdrom_pm_handler(caddr_t private, int mode)
{
	struct idecdrom_pm_handle *pmh = (struct idecdrom_pm_handle *) private;
	struct idecdrom_diskinfo *diskinfo;
	int		opri;
	int		errnoval = 0;
	int		rc = 0;
	
	

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

		errnoval = pincode((int(*)())idecdrom_iodone);
		if (errnoval) {
			return PM_ERROR;
		}

		opri = i_disable(INTTIMER);
		
		/*
		 * Allow any halted I/O due to suspend or hibernate
		 * to resume.
		 */

		diskinfo->pm_pending &= ~PM_IDECDROM_PENDING_SUSPEND;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idecdrom_trc(diskinfo,pmh_trc, trc,(char)1, 
				   (uint)diskinfo, (uint)diskinfo->cmds_out,
				   (uint)pmh, (uint)pmh->handle.mode,
				   (uint)mode);
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
			 * there are no outstanding commands
			 * then issue test unit ready command. This
			 * will cause the disc to be spun up if not already.
			 * The first command to cause a disc access will
			 * spin up the disc. In the reset sequence this
			 * will be the mode sense which retrieves the disc
			 * type.
			 * NOTE: we don't need to wait for the
			 * completion of this command, since
			 * the IDE adapter driver will not
			 * fail a PM_DEVICE_IDLE request if
			 * if has pending I/O.
			 */
			idecdrom_test_unit_ready(diskinfo);
		} 
		pmh->handle.mode = PM_DEVICE_ENABLE;
		i_enable(opri);

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode((int(*)())idecdrom_iodone);
			
		break;
	case PM_DEVICE_IDLE :
		/*
		 * Pin the bottom half of the driver, since
		 * we may not be open.
		 */

		errnoval = pincode((int(*)())idecdrom_iodone);
		if (errnoval) {
			return PM_ERROR;
		}
		opri = i_disable(INTTIMER);

		/*
		 * Allow any halted I/O due to suspend or hibernate
		 * to resume.
		 */

		diskinfo->pm_pending &= ~PM_IDECDROM_PENDING_SUSPEND;
#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idecdrom_trc(diskinfo,pmh_trc, trc,(char)2, 
				   (uint)diskinfo, (uint)diskinfo->cmds_out,
				   (uint)pmh, (uint)pmh->handle.mode,
				   (uint)mode);
#endif	
#endif

		if (!diskinfo->cmds_out) {

			if (diskinfo->opened) {
				/*
				 * If this device is open and
				 * there are no outstanding commands
				 * then issue the STOP UNIT
				 * command.
				 */

				if ((pmh->handle.mode == PM_DEVICE_SUSPEND) ||
				   (pmh->handle.mode == PM_DEVICE_HIBERNATION)){
					/* If mode was suspend or hibernate
					 * do restore of mode pages.
					 */

					diskinfo->disk_intrpt = 1;
					idecdrom_test_unit_ready(diskinfo);
					idecdrom_sleep(&(diskinfo->disk_intrpt),
						&(diskinfo->open_event));
				}

				idecdrom_start_unit(diskinfo,
						  (uchar) DK_STOP);

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
				 * CDROM DD here, but a check for a change in
				 * the disk's open status would then be
				 * necessary after the lockl call.  To minimize
				 * differences between this driver and the
				 * SCSI disk driver, and to ease future
				 * conversion of this driver to MP safe or
				 * MP efficient status, LOCK_NDELAY is used.
				 */

				rc = lockl(&(idecdrom_info.lock), LOCK_NDELAY);

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
							   idecdrom_iodone);
			
					return (PM_ERROR);
				}

				/*
				 * If we get here then, our lockl
				 * was successful. So we are now
				 * guarded against open/unconfigure
				 */
				(void) idecdrom_spindown_closeddisk(diskinfo);

				opri = i_disable(INTTIMER);

				unlockl(&(idecdrom_info.lock));
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
			(void *) unpincode((int(*)())idecdrom_iodone);
			
			return (PM_ERROR);
		}
		i_enable(opri);

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode((int(*)())idecdrom_iodone);
			
		break;
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
		opri = i_disable(INTTIMER);
		diskinfo->pm_pending |= PM_IDECDROM_PENDING_SUSPEND;

#ifdef DEBUG
#ifdef IDE_GOOD_PATH
		idecdrom_trc(diskinfo,pmh_trc, trc,(char)3, 
				   (uint)diskinfo, (uint)diskinfo->cmds_out,
				   (uint)pmh, (uint)pmh->handle.mode,
				   (uint)mode);
#endif	
#endif

		if (diskinfo->cmds_out) {
			
			/*
			 * If there is outstanding I/O, then
			 * we will wait a little while to see
			 * if it completes.
			 */

			diskinfo->pm_pending |= PM_IDECDROM_PENDING_OP;
			
			/*
			 * Set timer in case we sleep too
			 * long.
			 */


			diskinfo->pm_timer.watch.restart = 4;
			w_start(&(diskinfo->pm_timer.watch));
			
			
			/*
			 * We will sleep here. Either idecdrom_iodone
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
					~PM_IDECDROM_PENDING_OP;
				diskinfo->pm_pending &= 
					~PM_IDECDROM_PENDING_SUSPEND;
				i_enable(opri);
				return (PM_ERROR);

			} else if (diskinfo->pm_pending &
					PM_IDECDROM_PENDING_OP) {
				/*
				 * If the pending idle flag
				 * is still set, then our
				 * watchdog timer has popped.
				 * So fail this with PM_ERROR.
				 */
				diskinfo->pm_pending &= 
					~PM_IDECDROM_PENDING_OP;
				diskinfo->pm_pending &= 
					~PM_IDECDROM_PENDING_SUSPEND;

				/*
				 * We need to restart any I/O
				 * that may have been suspended
				 * here.
				 */
				idecdrom_start(diskinfo);
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
				diskinfo->disk_intrpt = 1;
				idecdrom_start_unit(diskinfo,
						  (uchar) DK_STOP);
				idecdrom_sleep(&(diskinfo->disk_intrpt),
					&(diskinfo->open_event));

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
				 * CDROM DD here, but a check for a change in
				 * the disk's open status would then be
				 * necessary after the lockl call.  To minimize
				 * differences between this driver and the
				 * SCSI disk driver, and to ease future
				 * conversion of this driver to MP safe or
				 * MP efficient status, LOCK_NDELAY is used.
				 */

				rc = lockl(&(idecdrom_info.lock), LOCK_NDELAY);

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
							   idecdrom_iodone);
			
					return (PM_ERROR);
				}

				/*
				 * If we get here then, our lockl
				 * was successful. So we are now
				 * guarded against open/unconfigure
				 */
				(void) idecdrom_spindown_closeddisk(diskinfo);

				opri = i_disable(INTTIMER);

				unlockl(&(idecdrom_info.lock));
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

		errnoval = pincode((int(*)())idecdrom_iodone);
		if (errnoval) {
			return PM_ERROR;
		}
		return PM_SUCCESS;
		break;
	case PM_PAGE_UNFREEZE_NOTICE:
		

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode((int(*)())idecdrom_iodone);
			
		return PM_SUCCESS;
		
	default :
		return PM_ERROR;
	}

	return PM_SUCCESS;

}

