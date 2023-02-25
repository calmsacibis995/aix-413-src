static char sccsid[] = "@(#)66	1.1  src/bos/usr/samples/fd/fd_bot.c, fdsamp, bos411, 9428A410j 10/12/93 14:35:59";
/*
 *   COMPONENT_NAME: FDSAMP (Diskette Device Driver)
 *
 *
 *   FUNCTIONS: fd_config, fd_open, fd_close, fd_read, fd_write,
 *              fd_strategy, fd_intr, fd_ioctl
 *
 *   INTERNAL FUNCTIONS: fdcheck_sequence, fdcleanup, fddisk_changed,
 *              fddoor_check, fderror, fdexecute_command,
 *              fdexecute_sequence, fdget_results, fdgetbyte, fdio,
 *              fdissue_command, fdload_floppy, fdlog_error, fdnull,
 *              fdputbyte, fdread,
 *              fdsettle_test, fdsettle_timer_handler, fdspeed_test,
 *              fdstart_sequence, fdstartio, fdtimeout, fdtimer,
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

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
 * FUNCTION: This device driver provides device support for the
 *           diskette controller and its attached drives.  Support
 *           is provided for one adapter and up to two drives.
 *
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/adspace.h>
#include <sys/buf.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/fd.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/ioctl.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/pri.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>

#include "fd_local.h"

struct adapter_structure *fdadapter;	/* allocate the adapter
						 structure pointer */
/*
 * keep config parameters around to reset to config values.
 */

struct fdconfig_parameters fdconfparms[FDMAXDRIVES];

/*
 * NAME: fd_strategy
 *
 * FUNCTION: The block i/o entry point.
 *  This routine handles all block i/o requests.  It checks the
 *  parameters of the i/o request and maintains a buffer queue for
 *  each drive.  If the queue for a drive is empty it calls fdio to
 *  try and start the i/o immediately.
 *  Otherwise it just sticks the i/o request at the
 *  end of the drive's queue.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.
 *  It is pinned in memory and can be called from the process and
 *  interrupt levels.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *
 * INPUTS:
 *  bp - a pointer to a buffer header structure.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: Value set in the buf struct by fdio.
 *
 * EXTERNAL PROCEDURES CALLED: i_enable, i_disable, iodone
 */

int	fd_strategy (register struct buf *bp)
{
	register struct floppy *fdp;
	register struct buf *nextbp;
	int	old_level, drive_number;


	/*
	 * Loop until there are no more buf structs to process.
	 */

	while (bp != NULL) {
		drive_number = minor(bp->b_dev) >> 6;
		nextbp = bp->av_forw;
		if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
			bp->b_error = EINVAL;
			bp->b_flags |= B_ERROR;
			bp->b_resid = bp->b_bcount;
			iodone(bp);
		} else
		 {
			old_level = i_disable(fdadapter->int_class);
			fdp = fdadapter->drive_list[drive_number];

			/*
			 * See if there are any buffer headers in the queue.
			 * If there are, put the buffer header at the end of
			 * the queue.  If the queue is empty, put the buffer
			 * header in the queue and call fdio().
			 */

			/*
			 * if queue is not empty, just put in queue
			 */

			if (fdp->headptr != NULL) {

				/*
				 * make old tail point to the new tail
				 */

				fdp->tailptr->av_forw = bp;
				fdp->tailptr = bp; /* set up new tail */
				i_enable(old_level);
			} else
			 {

				/*
				 * else start the i/o
				 */

				fdp->headptr = bp; /* start header queue */
				fdp->tailptr = fdp->headptr;
				i_enable(old_level);
				fdio(fdp->headptr);
			}
		} /* end of valid drive number processing */
		bp = nextbp;
	} /* end of while loop */
	return(FDSUCCESS);
} /* end of fd_strategy() routine */



/*
 * NAME: fd_intr
 *
 * FUNCTION: The interrupt handler.
 *  This routine handles all device interrupts. The operating
 *  system's first level interrupt handler calls this routine and
 *  fd_intr() first must decide if the interrupt was from the diskette
 *  controller.  If the interrupt was from the diskette, this
 *  routine clears the interrupt, processes the interrupt, and
 *  decides what should be done next.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.
 *  It is pinned in memory and can be called only by the operating
 *  system's first level interrupt handler.
 *
 * DATA STRUCTURES:
 *  fdadapter    struct - Common device driver information.
 *  floppy       struct - Per device information.
 *  result_bytes struct - Controller result phase data.
 *  buf          struct - A system structure that describes i/o
 *                        operations.  This is what is stored in
 *                        the queue created by the strategy
 *                        routine.
 *  dummy        struct - A floppy type structure allocated in the
 *                        stack to hold information after an
 *                        unsolicited reset has caused an
 *                        interrupt.  This information is just
 *                        thrown away, but needed a place to be
 *                        sent to.
 * 
 * INPUTS: none.
 *
 * RETURN VALUE DESCRIPTION:
 *  INTR_SUCC - this was a diskette interrupt and it has been
 *              processed.
 *  INTR_FAIL - this was not a diskette interrupt.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  i_reset, tstart, tstop, d_slave, d_complete, iodone, curtime,
 *  e_wakeup, w_stop
 *
 * NOTES:
 *  Errors are handled by either setting a bit that will be
 *  read when an FDIOCSTATUS ioctl is made or by other
 *  routines that fd_intr() calls.  This routine does not set
 *  any errno values directly.
 */

int	fd_intr ()
{
	register struct floppy *fdp;
	int	rc, temp;
	uchar	change_line_value, interrupt_check;
	struct buf *bp;
	ulong	elapsed_time;
	struct	pio_parms piop;

	/* Machine specific code to determine the ownership of the 
	 * interrupt.
	 */
	w_stop(&(fdadapter->inttimer));
	fdp = fdadapter->drive_list[fdadapter->active_drive];
	switch(fdadapter->state) {
	case FD_INITIAL_INTERRUPT:
		fdsense_interrupt();
		i_reset(fdadapter->fdhandler);
		fdadapter->state = FD_NO_STATE;
		if (fdadapter->first_open == FALSE) {
			fdlog_error(fdp, 0xff); /* log spurious interrupt */
		} else
		 {
			e_wakeup(&(fdadapter->sleep_anchor));
		}
		break;
	case FD_TYPE1_WAKEUP:
		fdadapter->command.total_bytes = 0;
		fdadapter->results.total_bytes = 7;
		piop.bus_val =
		    BUSIO_ATT(fdadapter->bus_id, fdadapter->io_address);
		fdadapter->error_value = pio_assist((caddr_t)(&piop), 
		    fddata_reg_pio, fddata_reg_pio_recovery);
		BUSIO_DET(piop.bus_val);
		i_reset(fdadapter->fdhandler);
		e_wakeup(&(fdadapter->sleep_anchor));
		break;
	case FD_TYPE2_WAKEUP:
		fdsense_interrupt();
		i_reset(fdadapter->fdhandler);
		e_wakeup(&(fdadapter->sleep_anchor));
		break;
	case FD_FORMAT:
		/*
		 * Code removed dealing with handling of interrupts
		 * during the formating of disks.
		 */
		break;
	case FD_RW:
		switch(fdadapter->sub_state) {
		case FD_RW_SEEK1:
			fdadapter->error_value = 0;
			fdsense_interrupt();
			if (fdadapter->error_value != 0) {
			    return(fdrw_exit(fdp, fdadapter->error_value));
			}
			i_reset(fdadapter->fdhandler);
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				if (fdp->retry_flag == FALSE) {
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				fdadapter->sub_state = FD_RW_RECAL1;
				fdadapter->error_value = 0;
				fdrecalibrate();
				if (fdadapter->error_value != 0) {
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
			} else
		 	 {
				fdadapter->error_value = 0;
				change_line_value = fdread_change_line();
				if (fdadapter->error_value != 0) {
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				if (change_line_value & FDDISK_CHANGE) {
					if (fdp->first_move == TRUE) {
						fdadapter->sub_state =
						    FD_RW_DRCHK_RECAL;
						fdadapter->error_value = 0;
						fdrecalibrate();
						if (fdadapter->error_value
						    != 0) {
			    				return(fdrw_exit(fdp,
					    		    fdadapter->
							    error_value));
						}
						fdp->first_move = FALSE;
					} else
					 {
			    			return(fdrw_exit(fdp,
					    	    fdadapter-> error_value));
					}
				} else
			 	 {
					if (fdp->headptr->b_flags & B_READ) 
						fdp->dma_flags = DMA_READ;
					else 
						fdp->dma_flags = 0;
					d_slave(fdadapter->dma_id,
					    fdp->dma_flags, 
	    				    (char *)(fdp->headptr->b_un.b_addr
					    + fdp->buf_offset), 
	    				    fdp->current.transfer_length,
					    &(fdp->headptr->b_xmemd));
					fdadapter->sub_state = FD_RW_SUB;
					fdadapter->error_value = 0;
					fdrw();
					if (fdadapter->error_value != 0) {
			    			return(fdrw_exit(fdp,
						    fdadapter->error_value));
					}
				}
			}
			break;
		case FD_RW_SUB:
			rc = d_complete(fdadapter->dma_id, fdp->dma_flags, 
	    		    (char *)(fdp->headptr->b_un.b_addr +
			    fdp->buf_offset), fdp->current.transfer_length,
			    &(fdp->headptr->b_xmemd), NULL);
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val= BUSIO_ATT(fdadapter->bus_id,
			    fdadapter->io_address);
			fdadapter->error_value = pio_assist((caddr_t)(&piop),
			    fddata_reg_pio, fddata_reg_pio_recovery);
			BUSIO_DET(piop.bus_val);
			i_reset(fdadapter->fdhandler);
			if (fdadapter->error_value == 0)
				if (rc != DMA_SUCC)
					fdadapter->error_value = EIO;
			if (fdadapter->error_value != 0) {
				return(fdrw_exit(fdp, fdadapter->error_value));
			}
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				if (fdp->retry_flag == FALSE) {
					fdassign_error(fdp);
					return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				if (fdp->simple_count1 >=
				    FDMAX_SIMPLE_RETRIES1) {
					/* 
					 * Code was removed dealing
					 * with retry recovery.
					 */

					fdp->complex_count++;
					fdadapter->sub_state =
					    FD_RW_RESET;
					fdadapter->error_value = 0;
					fdreset();
					if (fdadapter->error_value != 0) {
						return(fdrw_exit(fdp,
						    fdadapter->error_value));
					}
				} else {
					fdp->simple_count1++;
					if (fdp->headptr->b_flags & B_READ) 
						fdp->dma_flags = DMA_READ;
					else 
						fdp->dma_flags = 0;
					d_slave (fdadapter->dma_id, 
					    fdp->dma_flags, 
					    (char *)(fdp->headptr->b_un.b_addr
					    + fdp->buf_offset), 
	    				    fdp->current.transfer_length,
					    &(fdp->headptr->b_xmemd));
					fdadapter->sub_state = FD_RW_SUB;
					fdadapter->error_value = 0;
					fdrw();
					if (fdadapter->error_value != 0) {
			    			return(fdrw_exit(fdp,
			   			    fdadapter->error_value));
					}
				}
			} else {
				fdp->total_bcount +=
				    fdp->current.transfer_length;

				/*
				 * Code was removed dealing with 
				 * updating the data transfer counters.
				 */

			}	
			break;
		}
		/* Removed code dealing with other conditions. */
	default:
		break;
	}
	return(INTR_SUCC);
} /* end of fd_intr() */


/*
 * The following are the bottom half internal device driver routines.
 */


/*
 * NAME: fdcleanup
 *
 * FUNCTION: Cleans up initialization stuff prior to exiting.
 *  This routine is called just before exiting in fd_open and
 *  fd_close.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only from the process level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS: none.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  i_clear, d_clear, w_clear, tstop, tfree, unpincode
 */

void fdcleanup (int drive_number)
{
	int	i, cleanup_flag, rc;

	cleanup_flag = TRUE;
	for (i = 0; i < FDMAXDRIVES; i++) {
		if (i != drive_number)
			if (fdadapter->drive_list[i] != NULL)
				if (fdadapter->drive_list[i]->drive_state
				    != FDCLOSED)
					cleanup_flag = FALSE;
	}
	if (cleanup_flag == TRUE) {
		if (fdadapter->fderrptr != NULL) {
			rc = xmfree((caddr_t)fdadapter->fderrptr, pinned_heap);
		}
		if (fdadapter->int_init) {
			i_clear(fdadapter->fdhandler);
			fdadapter->int_init = FALSE;
		}
		if (fdadapter->dma_init) {
			d_clear(fdadapter->dma_level);
			fdadapter->dma_init = FALSE;
		}
		if (fdadapter->inttimer.func != NULL) {
			w_stop(&fdadapter->inttimer);
			w_clear(&fdadapter->inttimer);
			fdadapter->inttimer.func = NULL;
		}
		if (fdadapter->mottimer.func != NULL) {
			w_stop(&fdadapter->mottimer);
			w_clear(&fdadapter->mottimer);
			fdadapter->mottimer.func = NULL;
		}
		if (fdadapter->fdstart_timer != NULL) {
			tstop(fdadapter->fdstart_timer);
			tfree(fdadapter->fdstart_timer);
			fdadapter->fdstart_timer = NULL;
		}
		if (fdadapter->pinned == TRUE) {
			rc = unpin((caddr_t)fdadapter,
			    sizeof(struct adapter_structure));
			fdadapter->pinned = FALSE;
		}
		rc = unpincode(fd_intr);
		fdadapter->initialized = FALSE;
	}
} /* end of fdcleanup() */

/*
 * NAME: fdread_change_line
 *
 * FUNCTION: Reads the diskette changed register
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called from the process or the interrupt level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS:
 *
 * RETURN VALUE DESCRIPTION:
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

uchar	fdread_change_line ()
{
	uchar	rc;
	struct	pio_parms piop;

	piop.bus_val= BUSIO_ATT(fdadapter->bus_id,
	    fdadapter->io_address);
	piop.reg = FDDISK_CHANGED_REG;
	piop.read = TRUE;
	fdadapter->error_value = pio_assist((caddr_t)(&piop), fdcntl_reg_pio,
	     fdcntl_reg_pio_recovery);
	rc = piop.data;
	BUSIO_DET(piop.bus_val);
	return(rc);
} /* end of fdread_change_line() */

/*
 * NAME: fdio
 *
 * FUNCTION: Sets up the floppy structure for i/o and calls fdstartio.
 *  This routine translates the starting block of a read or write
 *  into the cylinder, head, sector address required by the
 *  controller.  It then computes the total number of reads or
 *  writes needed and initializes the fields in the floppy structure
 *  that are used to execute a read or write.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called at the process or interrupt level.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *
 * INPUTS:
 *  bp - a pointer to the buf struct describing the transfer.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 *
 * NOTES:
 *  This routine is called once for each buf struct
 *  processed.  It is called by both fd_strategy() and
 *  fd_intr() to initiate a new i/o sequence for a given
 *  buf struct.
 */

void fdio (register struct buf *bp)
{
	register struct floppy *fdp;
	daddr_t last_block;
	uchar	drive_number;
	ushort	temp;
	ulong	bytes_left;
	int	old_level, evalue;

	old_level = i_disable(fdadapter->int_class);
	drive_number = minor(bp->b_dev) >> 6;
	if (fdadapter->adapter_busy == FALSE) {
		fdadapter->adapter_busy = TRUE;
		fdadapter->active_drive = drive_number;
		fdp = fdadapter->drive_list[drive_number];
		i_enable(old_level);
		fdp->simple_count1 = 0;
		fdp->complex_count = 0;
		fdp->simple_count2 = 0;
		fdp->total_bcount = 0;
		bp->b_resid = 0;

		/*
		 * If the byte transfer length is not a multiple of 512 bytes,
		 * set b_error to EINVAL and return.
		 */

		if (bp->b_bcount % fdp->bytes_per_sector) {
			bp->b_flags |= B_ERROR;
			bp->b_error = EINVAL;
			bp->b_resid += bp->b_bcount;
			fdp->headptr = fdp->headptr->av_forw;
			iodone(bp);
			fdunlock_adapter(drive_number);
			return;
		}

		/*
		 * Verify that i/o does not start past the end of the diskette.
		 * Also, verify that the starting block number is non-negative.
		 * If it is not, return ENXIO.
		 */

		if ((bp->b_blkno < 0) || (bp->b_blkno >=
		    fdp->number_of_blocks)) {
			if (bp->b_blkno == fdp->number_of_blocks) {
				if (bp->b_flags & B_READ) {
					bp->b_error = 0;
				} else
		 		 {
					bp->b_flags |= B_ERROR;
					bp->b_error = ENXIO;
				}
				bp->b_resid = bp->b_bcount;
			} else
		 	 {
				bp->b_flags |= B_ERROR;
				bp->b_resid = bp->b_bcount;
				bp->b_error = ENXIO;
			}
			fdp->headptr = fdp->headptr->av_forw;
			evalue = bp->b_error;
			iodone(bp);
			fdunlock_adapter(drive_number);
			return;
		}

		/* 
		 * See if a request goes past the end of the
		 * diskette.
		 */

		fdp->modified_bcount = bp->b_bcount;
		last_block = bp->b_blkno + (fdp->modified_bcount /
		    fdp->bytes_per_sector);
		if (last_block > fdp->number_of_blocks) {

			/* 
			 * Request starts before the end of the
			 * diskette but runs over the end.
			 * Set up to do a partial data transfer
			 * and return the number of bytes
			 * not transferred in the b_resid
			 * field.
			 */

			bp->b_resid = (last_block - fdp->number_of_blocks) *
		 	    fdp->bytes_per_sector;
				fdp->modified_bcount -= bp->b_resid;
		}


		/*
		 * Translate block number into physical address.
		 */

		/*
		 * Set up the other fields in 'floppy' that are used for
		 * keeping track of reads and writes.
		 */

		if (fdadapter->motor_on != drive_number) {

			/*
			 * If the motor is not on, set a timer to pop when the
			 * drive has spun up.
			 */

			if (fdadapter->reset_needed == TRUE) {
				fdadapter->state = FD_IO_RESET;
				fdadapter->error_value = 0;
				fdreset();
				evalue = fdadapter->error_value;
				if (evalue != 0) {
					fdadapter->reset_needed = TRUE;
					bp->b_flags |= B_ERROR;
					bp->b_error = evalue;
					bp->b_resid = bp->b_bcount;
					fdp->headptr = fdp->headptr->av_forw;
					iodone(bp);
					fdunlock_adapter(drive_number);
				}
				return;
			}
			fdadapter->error_value = 0;
			fdselect_drive();
			evalue = fdadapter->error_value;
			if (evalue != 0) {
				fdadapter->reset_needed = TRUE;
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				iodone(bp);
				fdunlock_adapter(drive_number);
				return;
			}
			fdadapter->error_value = 0;
			fdspecify();
			evalue = fdadapter->error_value;
			if (evalue != 0) {
				fdadapter->reset_needed = TRUE;
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				iodone(bp);
				fdunlock_adapter(drive_number);
				return;
			}
			fdadapter->error_value = 0;
			fdset_data_rate();
			evalue = fdadapter->error_value;
			if (evalue != 0) {
				fdadapter->reset_needed = TRUE;
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				iodone(bp);
				fdunlock_adapter(drive_number);
				return;
			}
			fdadapter->fdstart_timer->timeout.it_value.tv_sec = 0;
			fdadapter->fdstart_timer->timeout.it_value.tv_nsec =
			    fdp->motor_start * 1000000;
			fdadapter->fdstart_timer->func_data = (ulong)fdp;
			fdadapter->state = FD_RW;
			fdadapter->sub_state = FD_RW_DELAY_TIMER;
			tstart(fdadapter->fdstart_timer);
		} else
		 {
			old_level = i_disable(fdadapter->int_class);
			fdadapter->state = FD_RW;
			fdadapter->error_value = 0;
			if (fdadapter->reset_needed == TRUE) {
				fdadapter->sub_state = FD_RW_RESET;
				fdreset();
			} else
			 {
				fdadapter->sub_state = FD_RW_SEEK1;
				fdseek();
			}
			evalue = fdadapter->error_value;
			i_enable(old_level);
			if (evalue != 0) {
				fdadapter->reset_needed = TRUE;
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				iodone(bp);
				fdunlock_adapter(drive_number);
				return;
			}
		}
	} else
	 {
		i_enable(old_level);
	}
} /* end of fdio() */




/*
 * NAME: fdload_floppy
 *
 * FUNCTION: Loads the proper diskette characteristics.
 *  This routine loads the floppy structure with the correct
 *  diskette characteristics for the type indicated.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called from the process or the interrupt level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS:
 *  value - the code of the diskette type to use.
 *  fdp   - a pointer to the floppy structure to use.
 *
 * RETURN VALUE DESCRIPTION:
 *  FDSUCCESS - the characteristics were successfully loaded.
 *
 * ERROR CODES: The following errno values may be returned:
 *  EINVAL - the 'value' parameter passed was invalid.
 *         - errno returned by fdissue_command.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

int	fdload_floppy (int value, register struct floppy *fdp)
{
	int	rc = FDSUCCESS;
	uchar drive_number;

	drive_number = (uchar)(minor(fdp->device_number) >> 6);
	fdadapter->mottimer.restart = fdp->motor_off_time;
	switch (value) {
	case FDGENERIC:
		/* Code has been removed to set the data structures to
		 * the default values.
		 */
		break; /* end of FDGENERIC case */
	case FD1440_3:
		/* Code has been removed to set the data structures to the
		 * proper values for a 3.5" 1.44Mb floppy.
		 */
		break; /* end of FD1440_3 case */
	case FD720_3:
		/* Code has been removed to set the data structures to the
		 * proper values for a 3.5" 720Kb floppy.
		 */
		break; /* end of FD720_3 case */
	case FD1200_5:
		/* Code has been removed to set the data structures to the
		 * proper values for a 5.25" 1.2Mb floppy.
		 */
		break; /* end of FD1200_5 case */
	case FD360_5:
		/* Code has been removed to set the data structures to the
		 * proper values for a 5.25" 360Kb floppy.
		 */
		break; /* end of FD360_5 case */
	default:
		fdp->drive_state = FDCLOSED;
		fdp->diskette_type = FDUNKNOWN;
		rc = EINVAL;
		break;
	}
	return(rc);
} /* end of fdload_floppy() */


	/* Various utility functions were removed from here. */
/*
 * NAME: fdwatchdog
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called from the process or the interrupt level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS: none.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

void fdwatchdog (struct watchdog *timer)
{
	register struct floppy *fdp;
	int 	rc, old_level;

	old_level = i_disable(fdadapter->int_class);
	fdp = fdadapter->drive_list[fdadapter->active_drive];
	switch (fdadapter->state) {
	case FD_RW:
		rc = d_complete(fdadapter->dma_id, fdp->dma_flags, 
	    	    (char *)(fdp->headptr->b_un.b_addr + fdp->buf_offset),
		    fdp->current.transfer_length, &(fdp->headptr->b_xmemd),
		    NULL);
		fdrw_exit(fdp, ENOTREADY);
		break;
	case FD_SPEED:
		break;
	case FD_SETTLE:
		switch (fdadapter->sub_state) {
		case FD_SETTLE_READ1:
		case FD_SETTLE_READ2:
			rc = d_complete(fdadapter->dma_id, DMA_READ,
			    (char *)fdadapter->speed_buffer,
			    fdp->bytes_per_sector, &fdadapter->xdp, NULL);
			break;
		case FD_SETTLE_WRITE:
			rc = d_complete(fdadapter->dma_id, 0,
			    (char *)fdadapter->speed_buffer,
			    fdp->bytes_per_sector, &fdadapter->xdp, NULL);
			break;
		}
		break;
	case FD_FORMAT:
		break;
	case FD_IO_RESET:
		break;
	case FD_TYPE1_WAKEUP:
	case FD_TYPE2_WAKEUP:
	case FD_INITIAL_INTERRUPT:
		break;
	default:
		break;
	}
	fdlog_error(fdp, 0xef);
	fdadapter->reset_needed = TRUE;
	fdadapter->error_value = ENOTREADY;
	e_wakeup(&(fdadapter->sleep_anchor));
	fdadapter->state = FD_NO_STATE;
	fdadapter->sub_state = NULL;
	i_enable(old_level);
} /* end of fdwatchdog() */


int	fdcntl_reg_pio (caddr_t piop)
{
	if (((struct pio_parms *)(piop))->read == TRUE) {
		((struct pio_parms *)(piop))->data =
		    FDREAD((ulong)((struct pio_parms *)(piop))->bus_val +
		    ((struct pio_parms *)(piop))->reg);
	} else
	 {
		FDWRITE((ulong)((struct pio_parms *)
		    (piop))->bus_val + ((struct pio_parms *)(piop))->reg, 
		    ((struct pio_parms *)(piop))->data);
	}
	return(FDSUCCESS);
}



int	fdcntl_reg_pio_recovery (caddr_t piop, int action)
{
	if (action == PIO_NO_RETRY) {
		fdadapter->reset_needed = TRUE;
		fdlog_error(NULL, 2); /* log permanent PIO error */
	} else
	 {
		fdlog_error(NULL, 1); /* log temporary PIO error */
		if (((struct pio_parms *)(piop))->read == TRUE) {
			FDREAD((ulong)((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg);
		} 
		else {
			FDWRITE((ulong)((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg,
			    ((struct pio_parms *)(piop))->data);
		}
	}
	return(FDSUCCESS);
}



void	fdset_data_rate ()
{
	/* Code for setting the data rate was removed from here. */

} 

void	fdreset ()
{
	/* Machine dependent reset code was removed from here. */
}

void	fdsoft_reset ()
{
	/* Machine dependent reset code was removed from here. */
}


void	fdselect_drive ()
{
	/* Machine dependent code to select the active drive. */
}

void	fdenable_controller ()
{
	/* Machine dependent code to enable the controller was removed. */
}

void	fddisable_controller ()
{
	/* Machine dependent code to disable the controller was removed. */
}

	/* Several machine dependent utility routines have been removed. */

void	fdrw ()
{
	uchar	drive_number, command;
	register struct floppy *fdp;
	struct	pio_parms piop;
	int	old_level;

	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->d_comp_needed = FALSE;
	if (fdp->headptr->b_flags & B_READ)
		command = FDREAD_DATA;
	else
		command = FDWRITE_DATA;
	fdadapter->command.total_bytes = 9;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = command | FDMT | FDMFM;
	fdadapter->command.un1.cmds.command2 = (fdp->current.head << 2) |
	     drive_number;
	fdadapter->command.un1.cmds.byte2.track = fdp->current.cylinder;
	fdadapter->command.un1.cmds.byte3.head = fdp->current.head;
	fdadapter->command.un1.cmds.byte4.sector = fdp->current.sector;
	fdadapter->command.un1.cmds.byte5.sector_size = fdp->sector_size;
	fdadapter->command.un1.cmds.eot_sector = fdp->sectors_per_track;
	fdadapter->command.un1.cmds.gap_length = fdp->gap;
	fdadapter->command.un1.cmds.data_length = fdp->data_length;
	w_start(&(fdadapter->inttimer));
	piop.bus_val = BUSIO_ATT(fdadapter->bus_id,
	    fdadapter->io_address);
	old_level = i_disable(fdadapter->int_class);
	fdadapter->error_value = pio_assist((caddr_t)(&piop), fddata_reg_pio,
	    fddata_reg_pio_recovery);
	BUSIO_DET(piop.bus_val);
	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
		fdadapter->d_comp_needed = TRUE;
	} else
	 {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	i_enable(old_level);
} /* end of fdrw() */

	/* Several machine dependent utility routines have been removed. */

/* process level only */
int	fdlock_adapter (int drive_number)
{
	int	old_level, rc;
	
	old_level = i_disable(fdadapter->int_class);
	while (fdadapter->adapter_busy == TRUE) {
		rc = e_sleep(&(fdadapter->adapter_sleep_anchor), EVENT_SIGRET);
		if (rc == EVENT_SIG) {
			i_enable(old_level);
			return(EINTR);
		}
	}
	fdadapter->adapter_busy = TRUE;
	fdadapter->active_drive = drive_number;
	i_enable(old_level);
	return(FDSUCCESS);
}

void	fdunlock_adapter (int drive_number)
{
	int	old_level, i;
	struct floppy *fdp;
	
	old_level = i_disable(fdadapter->int_class);
	fdadapter->adapter_busy = FALSE;
	for (i = 0; i < FDMAXDRIVES; i++) {
		if (i != drive_number) {
			fdp = fdadapter->drive_list[i];
			if (fdp != NULL) {
				if (fdp->headptr != NULL) {
					fdio(fdp->headptr);
					i_enable(old_level);
					return;
				}
			}
		}
	}
	fdp = fdadapter->drive_list[drive_number];
	if (fdp->headptr != NULL) {
		fdio(fdp->headptr);
	} else
	 {
		e_wakeup(&(fdadapter->adapter_sleep_anchor));
	}
	i_enable(old_level);
}
		

void	fdtimeout (register struct trb *fdstart_timer_ptr)
{
	uchar	evalue, drive_number;
	register struct buf *bp;
	register struct floppy *fdp;
	int	old_level;

	old_level = i_disable(fdadapter->int_class);
	fdadapter->state = FD_RW;
	fdadapter->error_value = 0;
	if (fdadapter->reset_needed == TRUE) {
		fdadapter->sub_state = FD_RW_RESET;
		fdreset();
	} else
	 {
		fdadapter->sub_state = FD_RW_SEEK1;
		fdseek();
	}
	evalue = fdadapter->error_value;
	if (evalue != 0) {
		fdadapter->reset_needed = TRUE;
		drive_number = fdadapter->active_drive;
		fdp = (register struct floppy * )fdstart_timer_ptr->func;
		bp = (register struct buf * )fdp->headptr;
		bp->b_flags |= B_ERROR;
		bp->b_error = evalue;
		bp->b_resid += fdp->modified_bcount - fdp->total_bcount;
		fdp->headptr = fdp->headptr->av_forw;
		iodone(bp);
		fdunlock_adapter(drive_number);
	}
	i_enable(old_level);
}



int	fdqueue_check (register struct floppy *fdp, int drive_number)
{
	int 	old_level, rc, local_error_value;

	old_level = i_disable(fdadapter->int_class);
	while (fdp->headptr != NULL) {
		rc = e_sleep(&(fdadapter->adapter_sleep_anchor), EVENT_SIGRET);
		if (rc == EVENT_SIG) {
			fdp->drive_state = FDOPEN;
			i_enable(old_level);
			return(EINTR);
		}
	}

	/* 
	 * Get control of the adapter, turn off the drive, and
	 * call fdcleanup to finish processing.
	 */
	
	rc = fdlock_adapter(drive_number);
	if (rc != FDSUCCESS) {
		fdp->drive_state = FDOPEN;
		i_enable(old_level);
		return(rc);
	}
	i_enable(old_level);
	fdadapter->error_value = 0;
	fddisable_controller();
	if (fdadapter->error_value != 0) {
		fdp->drive_state = FDOPEN;
		local_error_value = fdadapter->error_value;
		fdunlock_adapter(drive_number);
		return(local_error_value);
	}
	fdcleanup(drive_number);
	fdunlock_adapter(drive_number);
	return(FDSUCCESS);
}



void	fdassign_error(register struct floppy *fdp)
{
	switch(fdadapter->results.un1.names.byte1.status1) {
	case 0x02: /* Write protected */
		fdadapter->error_value = EWRPROTECT;
		break;
	case 0x01: /* Missing address mark */
		if (fdadapter->results.un1.names.status2 == 0x01)
			fdadapter->error_value = EIO;
		else
			fdadapter->error_value = EFORMAT;
		break;
	default:
		fdadapter->error_value = EIO;
		break;
	}
}

	/* Several utility routines have been removed. */
