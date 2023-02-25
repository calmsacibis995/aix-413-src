#ifndef lint
static char sccsid[] = "@(#)98 1.5 src/rspc/kernext/isa/fd/fd_bot.c, isafd, rspc41J, 9513A_all 3/28/95 13:56:13";
#endif
/*
 * COMPONENT_NAME: (isafd) ISA Diskette Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * FUNCTION: This device driver provides device support for the
 *           diskette controller and its attached drives.  Support
 *           is provided for one adapter and up to two drives.  This
 *           module contains the bottom half of the device driver and
 *           must be pinned in memory.
 *
 * ROUTINES: fd_strategy, fd_intr, fd_pmh
 *
 * INTERNAL SUBROUTINES: 
 *		fdassign_error 
 *		fdcfg_cntl_pio
 *           	fdcfg_cntl_pio_recovery 
 *		fdcfg_data_pio 
 *		fdcfg_data_pio_recovery
 *		fdcheck_pm_mode
 *           	fdcleanup 
 *		fdcntl_reg_pio 
 *		fdcntl_reg_pio_recovery
 *           	fddata_reg_pio 
 *		fddata_reg_pio_recovery 
 *		fddisable_controller
 *           	fdenable_controller 
 *		fdexecute_int_cmd 
 *		fdformat_track 
 *		fdio
 *           	fdio_exit 
 *		fdload_floppy 
 *		fdlock_adapter 
 *		fdlog_error
 *           	fdmotor_timer 
 *		fdnull 
 *		fdqueue_check 
 *		fdread_change_line
 *           	fdread_main_status 
 *		fdreadid 
 *		fdrecalibrate
 *		fdreset 
 *		fdrw
 *           	fdrw_exit 
 *		fdrw_ioctl 
 *		fdseek 
 *		fdselect_drive
 *           	fdsense_drive_status 
 *		fdsense_interrupt 
 *		fdset_data_rate
 *           	fdsettle_timer_handler 
 *		fdsoft_reset 
 *		fdspecify 
 *		fdtimeout
 *		fdunlock_adapter 
 *		fdwatchdog 
 *		fdset_config 
 *		fdset_perp
 *           	fdtop_enable_controller 
 *		fdtop_select_drive 
 *		fdtop_unlock_adapter
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/adspace.h>
#include <sys/buf.h>
#include <sys/ddtrace.h>
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
#include <sys/pmdev.h>
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

struct adapter_structure *fdadapter = NULL;	/* allocate the adapter
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
 *  try and start the i/o immediately.  Otherwise it just sticks the i/o
 *  request at the end of the drive's queue.
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

	DDHKWD5(HKWD_DD_FDDD, DD_ENTRY_STRATEGY, 0, bp->b_dev, bp, bp->b_flags,
	     bp->b_blkno, bp->b_bcount);

	/*
	 * Loop until there are no more buf structs to process.
	 */
	FD_TRACE1("in strat", bp->b_dev);

	while (bp != NULL) {
		drive_number = minor(bp->b_dev) >> 6;
		nextbp = bp->av_forw;
		if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
			bp->b_error = EINVAL;
			bp->b_flags |= B_ERROR;
			bp->b_resid = bp->b_bcount;
			FD_TRACE1("call iodone", (dev_t)(bp->b_error));
			FD_TRACE2("bp:", (ulong)bp->b_flags, (caddr_t)bp,
			    (ulong)bp->b_bcount);
			iodone(bp);
		} else
		 {
			fdp = fdadapter->drive_list[drive_number];
			old_level = disable_lock(fdadapter->int_class, 
						 &fdadapter->intr_lock);

			/* check if the I/O should be blocked due to PM mode */
			fdcheck_pm_mode (fdp);

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
				unlock_enable(old_level, &fdadapter->intr_lock);
			} else
			 {

				/*
				 * else start the i/o
				 */

				fdp->headptr = bp; /* start header queue */
				fdp->tailptr = fdp->headptr;
				fdio(fdp->headptr);
				unlock_enable(old_level, &fdadapter->intr_lock);
			}
		} /* end of valid drive number processing */
		bp = nextbp;
	} /* end of while loop */
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_STRATEGY, 0, 0);
	FD_TRACE1("out strat", bp->b_dev);
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
 * 
 * INPUTS: none.
 *
 * RETURN VALUE DESCRIPTION:
 *  INTR_SUCC - this was a diskette interrupt and it has been
 *              processed.
 *  INTR_FAIL - this was not a diskette interrupt.
 *
 * ERROR CODES:
 *  EIO - an i/o error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  i_reset, tstart, tstop, D_MAP_SLAVE, D_UNMAP_SLAVE, iodone, curtime,
 *  e_wakeup, w_stop, disable_lock, unlock_enable
 *
 * NOTES:
 *  Errors are handled by either setting a bit that will be
 *  read when an FDIOCSTATUS ioctl is made or by other
 *  routines that fd_intr() calls.
 */

int	fd_intr ()
{
	register struct floppy *fdp;
	int     i, dma_rc, rc, temp;
	uchar   change_line_value, interrupt_check, mask;
	struct buf *bp;
	ulong	elapsed_time;
	struct	pio_parms piop;
	uint old_pri;	

	DDHKWD1(HKWD_DD_FDDD, DD_ENTRY_INTR, 0, 0);
	FD_TRACE1("in intr", FDA_DEV);

	fdp = fdadapter->drive_list[fdadapter->active_drive];

	old_pri= disable_lock(fdadapter->int_class, &fdadapter->intr_lock);	

#ifdef _POWER
	/*
	 * Check main status register to see if this is a type 1 interrupt.
	 * If MSR cannot be read then return INTR_FAIL.
	 */

	fdadapter->error_value = 0;
	interrupt_check = fdread_main_status();
	if (fdadapter->error_value != 0) {
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_FAIL,
		    fdadapter->drive_list[fdadapter->active_drive]);
		FD_TRACE1("NOT MY INT!",FDA_DEV);
		FD_TRACE1("out intr",FDA_DEV);
		unlock_enable(old_pri, &fdadapter->intr_lock);
		return(INTR_FAIL);
	}

	/*
	 * issue a sense interrupt
	 */

	if ((interrupt_check & FDREAD_MASK) != FDREAD_MASK) {
		fdadapter->error_value = 0;
		fdadapter->reset_active = FALSE;
		fdsense_interrupt();

		/*
		 * If sense interrput cannot be
		 * issued return INTR_FAIL
		 */

		if (fdadapter->error_value != 0) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR,
			    INTR_FAIL, fdadapter->
			    drive_list[fdadapter->active_drive]);
			FD_TRACE1("NOT MY INT!",FDA_DEV);
			FD_TRACE1("out intr",FDA_DEV);
			unlock_enable(old_pri, &fdadapter->intr_lock);
			return(INTR_FAIL);
		}

		/*
		 * Check the return status from the sense interrupt.  If the
		 * two high order bits are 10 this indicates an invalid
		 * command.  In this case return INTR_FAIL.  Otherwise
		 * continue remembering that the data from the sense interrupt
		 * is already in the results structure so no sense interrupt
		 * need be issue by interrupt cases that process type 2
		 * interrupts.
		 */

		if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL)
		    == FDINVALID_MASK) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_FAIL,
			    fdadapter->drive_list[fdadapter->active_drive]);
			FD_TRACE1("NOT MY INT!",FDA_DEV);
			FD_TRACE1("out intr",FDA_DEV);
			unlock_enable(old_pri, &fdadapter->intr_lock);
			return(INTR_FAIL);
		}
	}
    
#endif
	/* It's the driver's interrupt, so...	 */
	/* - stop the interrupt timer		 */
	/* - tell the PM core about the activity */
	/* - service the interrupt		 */
	w_stop(&(fdadapter->inttimer));
	fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;
	fdp->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;
	ASSERT(fdadapter->state != FD_NO_STATE);

	switch(fdadapter->state) {
	/* The driver takes an interrupt as soon as "i_init" */
	/* is called.  This case handles that interrupt      */
	case FD_DUMMY_INTERRUPT:
		FD_TRACE1("DUMMY", FDP_DEV);
		i_reset(fdadapter->fdhandler);
		fdadapter->state = FD_NO_STATE;
		break;

	/* This interrupt is taken on the first reset of the first open. */
	case FD_INITIAL_INTERRUPT:
		FD_TRACE1("INITIAL", FDP_DEV);
		i_reset(fdadapter->fdhandler);
		fdadapter->state = FD_NO_STATE;
		if (fdadapter->first_open == FALSE) {
			fdlog_error(fdp, 0xff); /* log spurious interrupt */
			FD_TRACE1("SPURIOUS!!!", FDP_DEV);
		} else
		 {
			e_wakeup(&(fdadapter->sleep_anchor));
		}
		break;

	case FD_TYPE1_WAKEUP:
		FD_TRACE1("TYPE1 WAKE", FDP_DEV);
		fdadapter->command.total_bytes = 0;
		fdadapter->results.total_bytes = 7;
		piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
		fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
		FD_BUSIO_DET(piop.bus_val);
		i_reset(fdadapter->fdhandler);
		e_wakeup(&(fdadapter->sleep_anchor));
		break;

	case FD_TYPE2_WAKEUP:
		FD_TRACE1("TYPE2 WAKE", FDP_DEV);
		i_reset(fdadapter->fdhandler);
		e_wakeup(&(fdadapter->sleep_anchor));
		break;

	case FD_FORMAT:
		FD_TRACE1("FORMAT", FDP_DEV);

		rc = D_UNMAP_SLAVE (fdadapter->dma_id);
		FD_TRACE1("call d_comp", rc);
		fdadapter->command.total_bytes = 0;
		fdadapter->results.total_bytes = 7;

		/* read the result data from the controller */
		piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
		fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
		FD_BUSIO_DET(piop.bus_val);

		if (fdadapter->error_value == 0) {
			if (rc != DMA_SUCC) {

				/*
			 	 * Only report DMA errors if there is no
			 	 * corresponding diskette controller error.
			 	 * Otherwise just go through normal retry
				 * logic.  
				 */

				if ((fdadapter->
				    results.un1.names.byte0.status0 &
				    FDNORMAL) == 0) {
					fdadapter->error_value = EIO;
				}
			}
		}

		i_reset(fdadapter->fdhandler);
		e_wakeup(&(fdadapter->sleep_anchor));
		break;

	case FD_SPEED:
		FD_TRACE1("SPEED", FDP_DEV);
		switch(fdadapter->sub_state) {
		case FD_SPEED_READ1:
			FD_TRACE1("SPDREAD1SUB", FDP_DEV);
			curtime(&(fdadapter->start_time));

			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);

			/* read in the result data from the controller */
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
			fdadapter->error_value = 
					fddata_reg_pio((caddr_t) &piop);
			FD_BUSIO_DET(piop.bus_val);

			if (fdadapter->error_value == 0)
				if (rc != DMA_SUCC)
					fdadapter->error_value = EIO;

			i_reset(fdadapter->fdhandler);
			if (fdadapter->error_value != 0) {
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				fdadapter->error_value = EIO;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			fdadapter->xdp.aspace_id = XMEM_GLOBAL;
			fdadapter->dio.used_iovecs = 1;
			fdadapter->dio.dvec->iov_base = fdadapter->speed_buffer;
			fdadapter->dio.dvec->iov_len = fdp->bytes_per_sector;
			fdadapter->dio.dvec->xmp = &fdadapter->xdp;
			dma_rc = D_MAP_SLAVE (fdadapter->dma_id, DMA_READ,
				     fdp->bytes_per_sector, &fdadapter->dio,
				     (CH_SINGLE | CH_ADDR_INC | CH_EOP_OUTPUT
				      | CH_TYPE_B | CH_8BIT_BYTES) );
			if (dma_rc != DMA_SUCC) {
				fdadapter->error_value = EIO;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			
			w_start(&(fdadapter->inttimer));
			fdadapter->sub_state = FD_SPEED_READ2;
			fdp->current.cylinder = 0;
			fdp->current.head = 0;
			fdp->current.sector = 1;
			fdadapter->error_value = 0;
			fdrw_ioctl();
			if (fdadapter->error_value != 0) {
				w_stop(&(fdadapter->inttimer));
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;

				rc = D_UNMAP_SLAVE (fdadapter->dma_id);
				FD_TRACE1("call d_comp", rc);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			break;
		case FD_SPEED_READ2:
			FD_TRACE1("SPDREAD2SUB", FDP_DEV);
			curtime(&(fdadapter->end_time));

			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);

			/* read in the result data from the controller */
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
			fdadapter->error_value = 
					fddata_reg_pio((caddr_t) &piop);
			FD_BUSIO_DET(piop.bus_val);

			if (fdadapter->error_value == 0)
				if (rc != DMA_SUCC)
					fdadapter->error_value = EIO;

			i_reset(fdadapter->fdhandler);
			if (fdadapter->error_value != 0) {
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0)
				fdadapter->error_value = EIO;
			e_wakeup(&(fdadapter->sleep_anchor));
			fdadapter->state = FD_NO_STATE;
			fdadapter->sub_state = NULL;
			break;
		default:
			ASSERT(fdadapter->sub_state != fdadapter->sub_state);
			break;
		}
		break;
	case FD_SETTLE:
		FD_TRACE1("SETTLE", FDP_DEV);
		switch(fdadapter->sub_state) {
		case FD_SETTLE_READ1:
			FD_TRACE1("SETREAD1SUB", FDP_DEV);

			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);

			/* read in the result data */
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
			fdadapter->error_value = 
					fddata_reg_pio((caddr_t) &piop);
			FD_BUSIO_DET(piop.bus_val);

			if (fdadapter->error_value == 0)
				if (rc != DMA_SUCC)
					fdadapter->error_value = EIO;

			i_reset(fdadapter->fdhandler);
			if (fdadapter->error_value != 0) {
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				fdadapter->error_value = EIO;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			fdadapter->sub_state = FD_SETTLE_TIMER;
			fdadapter->fdsettle_timer->func =
			    fdsettle_timer_handler;
			fdadapter->fdsettle_timer->func_data = (ulong)fdp;
			fdadapter->fdsettle_timer->flags = T_MPSAFE;
			fdadapter->fdsettle_timer->ipri =
			    fdadapter->int_class;
			fdadapter->fdsettle_timer->timeout.it_value.tv_sec = 0;
			fdp->settle_delay = (6 * (fdp->motor_speed_time /
			    fdp->sectors_per_track) - fdp->step_rate_time *
			    1000 - fdp->head_settle_time * 1000) * 1000 -
			    fdadapter->fudge_factor;
			fdadapter->fdsettle_timer->timeout.it_value.tv_nsec =
			    fdp->settle_delay;
			curtime(&fdadapter->start_time);
			tstart(fdadapter->fdsettle_timer);
			break;
		case FD_SETTLE_SEEK:
			FD_TRACE1("SETSEEKSUB", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			fdadapter->xdp.aspace_id = XMEM_GLOBAL;
			fdadapter->dio.used_iovecs = 1;
			fdadapter->dio.dvec->iov_base = fdadapter->speed_buffer;
			fdadapter->dio.dvec->iov_len = fdp->bytes_per_sector;
			fdadapter->dio.dvec->xmp = &fdadapter->xdp;
			dma_rc = D_MAP_SLAVE (fdadapter->dma_id, 0,
				     fdp->bytes_per_sector, &fdadapter->dio,
				     (CH_SINGLE | CH_ADDR_INC | CH_EOP_OUTPUT
				      | CH_TYPE_B | CH_8BIT_BYTES) );
			if (dma_rc != DMA_SUCC) {
				fdadapter->error_value = EIO;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;

				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			w_start(&(fdadapter->inttimer));
			curtime(&(fdadapter->start_time));
			fdadapter->sub_state = FD_SETTLE_WRITE;
			fdp->current.cylinder = 1;
			fdp->current.head = 0;
			fdp->current.sector = 8;
			fdadapter->error_value = 0;
			fdrw_ioctl();
			if (fdadapter->error_value != 0) {
				w_stop(&(fdadapter->inttimer));
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;

				rc = D_UNMAP_SLAVE (fdadapter->dma_id);
				FD_TRACE1("call d_comp", rc);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			break;
		case FD_SETTLE_WRITE:
			FD_TRACE1("SETWRITESUB", FDP_DEV);
			curtime(&(fdadapter->end_time));

			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);

			/* read in the result data */
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
			fdadapter->error_value = 
					fddata_reg_pio((caddr_t) &piop);
			FD_BUSIO_DET(piop.bus_val);

			if (fdadapter->error_value == 0)
				if (rc != DMA_SUCC)
					fdadapter->error_value = EIO;

			i_reset(fdadapter->fdhandler);
			if (fdadapter->error_value != 0) {
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				fdadapter->error_value = EIO;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			if (fdadapter->start_time.tv_sec ==
			    fdadapter->end_time.tv_sec) {
				elapsed_time = fdadapter->end_time.tv_nsec -
		    		    fdadapter->start_time.tv_nsec;
			} else
	 		{
				elapsed_time = NS_PER_SEC -
				    fdadapter->start_time.tv_nsec;
				elapsed_time += (fdadapter->end_time.tv_nsec +
		    		    (fdadapter->end_time.tv_sec -
				    fdadapter->start_time.tv_sec - 1) *
				    NS_PER_SEC);
			}

			/* 
	 		 * convert elapsed time to microseconds
	 		 */

			elapsed_time /= 1000;
			if (elapsed_time > (fdp->motor_speed_time / 2)) {
				fdadapter->error_value = ENOTREADY;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}

			/*
			 * fdp->head_load must have been set to the value
			 * under test by the calling routing.
			 */

			fdadapter->error_value = 0;
			fdspecify();
			if (fdadapter->error_value != 0) {
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			fdadapter->xdp.aspace_id = XMEM_GLOBAL;
			fdadapter->dio.used_iovecs = 1;
			fdadapter->dio.dvec->iov_base = fdadapter->speed_buffer;
			fdadapter->dio.dvec->iov_len = fdp->bytes_per_sector;
			fdadapter->dio.dvec->xmp = &fdadapter->xdp;
			dma_rc = D_MAP_SLAVE (fdadapter->dma_id, DMA_READ,
				     fdp->bytes_per_sector, &fdadapter->dio,
				     (CH_SINGLE | CH_ADDR_INC | CH_EOP_OUTPUT
				      | CH_TYPE_B | CH_8BIT_BYTES) );
			if (dma_rc != DMA_SUCC) {
				fdadapter->error_value = EIO;
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;

				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}

			w_start(&(fdadapter->inttimer));
			fdadapter->sub_state = FD_SETTLE_READ2;
			fdp->current.cylinder = 1;
			fdp->current.head = 0;
			fdp->current.sector = 8;
			fdadapter->error_value = 0;
			fdrw_ioctl();
			if (fdadapter->error_value != 0) {
				w_stop(&(fdadapter->inttimer));
				e_wakeup(&(fdadapter->sleep_anchor));
				fdadapter->state = FD_NO_STATE;
				fdadapter->sub_state = NULL;

				rc = D_UNMAP_SLAVE (fdadapter->dma_id);
				FD_TRACE1("call d_comp", rc);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC,
				    fdp->device_number);
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(INTR_SUCC);
			}
			break;
		case FD_SETTLE_READ2:
			FD_TRACE1("SETREAD2SUB", FDP_DEV);

			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);

			/* read in the result data */
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
			fdadapter->error_value = 
					fddata_reg_pio((caddr_t) &piop);
			FD_BUSIO_DET(piop.bus_val);

			if (fdadapter->error_value == 0)
				if (rc != DMA_SUCC)
					fdadapter->error_value = EIO;

			i_reset(fdadapter->fdhandler);
			e_wakeup(&(fdadapter->sleep_anchor));
			fdadapter->state = FD_NO_STATE;
			fdadapter->sub_state = NULL;
			break;
		default:
			ASSERT(fdadapter->sub_state != fdadapter->sub_state);
			break;
		}
		break;
	case FD_RW:
		FD_TRACE1("RW", FDP_DEV);
		switch(fdadapter->sub_state) {
		case FD_RW_SEEK1:
			FD_TRACE1("RW_SEEK1", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				if (fdp->retry_flag == FALSE) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri,
					    &fdadapter->intr_lock);
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				fdadapter->sub_state = FD_RW_RECAL1;
				fdadapter->error_value = 0;
				fdrecalibrate();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri,
					     &fdadapter->intr_lock);
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
			} else
		 	 {
				fdadapter->error_value = 0;
				change_line_value = fdread_change_line();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri, 
						&fdadapter->intr_lock);
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
							FD_TRACE1("out intr",
							    FDP_DEV);
							unlock_enable(old_pri,
						 	    &fdadapter->
								intr_lock);
			    				return(fdrw_exit(fdp,
					    		    fdadapter->
							    error_value));
						}
						fdp->first_move = FALSE;
					} else
					 {
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp,
					    	    ENOTREADY));
					}
				} else
			 	 {
					if (fdp->headptr->b_flags & B_READ) 
						fdp->dma_flags = DMA_READ;
					else 
						fdp->dma_flags = 0;
					fdadapter->dio.used_iovecs = 1;
					fdadapter->dio.dvec->iov_base = 
					     (char *)(fdp->headptr->b_un.b_addr 
						+ fdp->buf_offset),
					fdadapter->dio.dvec->iov_len = 
						fdp->current.transfer_length;
					fdadapter->dio.dvec->xmp = 
						&(fdp->headptr->b_xmemd);
					dma_rc = D_MAP_SLAVE (fdadapter->dma_id,
						fdp->dma_flags,
						fdp->bytes_per_sector,
						&fdadapter->dio,
						(CH_SINGLE | CH_ADDR_INC
						| CH_EOP_OUTPUT | CH_TYPE_B 
						| CH_8BIT_BYTES) );
					if (dma_rc != DMA_SUCC) {
						/* cannot satisfy the DMA  */
						/* request, send back 'bp' */
						/* with an EIO error       */
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp, EIO));
					}
					     
					fdadapter->sub_state = FD_RW_SUB;
					fdadapter->error_value = 0;
					fdrw();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp,
						    fdadapter->error_value));
					}
				}
			}
			break;
		case FD_RW_RECAL1:
			FD_TRACE1("RW_RECAL1", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			fdadapter->sub_state = FD_RW_RECAL2;
			fdadapter->error_value = 0;
			fdrecalibrate();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			}
			break;
		case FD_RW_RECAL2:
			FD_TRACE1("RW_RECAL2", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			fdadapter->sub_state = FD_RW_RECAL3;
			fdadapter->error_value = 0;
			fdrecalibrate();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			}
			break;
		case FD_RW_RECAL3:
			FD_TRACE1("RW_RECAL3", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			} else
		 	 {
				fdadapter->sub_state = FD_RW_SEEK2;
				fdp->cylinder_id = fdp->current.cylinder;
				fdadapter->error_value = 0;
				fdseek();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri, 
						      &fdadapter->intr_lock);
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
			}
			break;
		case FD_RW_SEEK2:
			FD_TRACE1("RW_SEEK2", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			} else
		 	 {
				fdadapter->error_value = 0;
				change_line_value = fdread_change_line();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri,
					    &fdadapter->intr_lock);
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				if (change_line_value & FDDISK_CHANGE) { 
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri, 
					    &fdadapter->intr_lock);
			    		return(fdrw_exit(fdp, ENOTREADY));
				} else
			 	 {
					if (fdp->headptr->b_flags & B_READ) 
						fdp->dma_flags = DMA_READ;
					else 
						fdp->dma_flags = 0;
					fdadapter->dio.used_iovecs = 1;
					fdadapter->dio.dvec->iov_base = 
					     (char *)(fdp->headptr->b_un.b_addr 
						+ fdp->buf_offset),
					fdadapter->dio.dvec->iov_len = 
						fdp->current.transfer_length;
					fdadapter->dio.dvec->xmp = 
						&(fdp->headptr->b_xmemd);
					dma_rc = D_MAP_SLAVE (fdadapter->dma_id,
						fdp->dma_flags,
						fdp->bytes_per_sector,
						&fdadapter->dio,
						(CH_SINGLE | CH_ADDR_INC
						| CH_EOP_OUTPUT | CH_TYPE_B 
						| CH_8BIT_BYTES) );
					if (dma_rc != DMA_SUCC) {
						/* cannot satisfy the DMA  */
						/* request, send back 'bp' */
						/* with an EIO error       */
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp, EIO));
					}

					fdadapter->sub_state = FD_RW_SUB;
					fdadapter->error_value = 0;
					fdrw();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp,
						    fdadapter->error_value));
					}
				}
			}
			break;
		case FD_RW_DRCHK_RECAL:
			FD_TRACE1("RW_DRCHK_RC", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			fdadapter->sub_state = FD_RW_DRCHK_SEEK;
			fdp->cylinder_id = 1;
			fdadapter->error_value = 0;
			fdseek();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			}
			break;
		case FD_RW_DRCHK_SEEK:
			FD_TRACE1("RW_DRCHK_SK", FDP_DEV);
			fdadapter->error_value = 0;
			i_reset(fdadapter->fdhandler);
			fdadapter->error_value = 0;
			change_line_value = fdread_change_line();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
		    		return(fdrw_exit(fdp, fdadapter->error_value));
			}
			if (change_line_value & FDDISK_CHANGE) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
		    		return(fdrw_exit(fdp, ENOTREADY));
			} else
		 	 {
				if (fdp->headptr->b_flags & B_READ) 
					fdp->dma_flags = DMA_READ;
				else 
					fdp->dma_flags = 0;
				fdadapter->dio.used_iovecs = 1;
				fdadapter->dio.dvec->iov_base = 
					(char *)(fdp->headptr->b_un.b_addr + 
						fdp->buf_offset);
				fdadapter->dio.dvec->iov_len = 
					fdp->current.transfer_length;
				fdadapter->dio.dvec->xmp = 
					&(fdp->headptr->b_xmemd);
				dma_rc = D_MAP_SLAVE (fdadapter->dma_id, 
						fdp->dma_flags,
						fdp->bytes_per_sector,
						&fdadapter->dio,
						(CH_SINGLE | CH_ADDR_INC 
						| CH_EOP_OUTPUT | CH_TYPE_B 
						| CH_8BIT_BYTES) );
				if (dma_rc != DMA_SUCC) {
					/* cannot satisfy the DMA  */
					/* request, send back 'bp' */
					/* with an EIO error       */
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri,
					    &fdadapter->intr_lock);
		    			return(fdrw_exit(fdp, EIO));
				}

				fdadapter->sub_state = FD_RW_SUB;
				fdadapter->error_value = 0;
				fdrw();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri, 
					    &fdadapter->intr_lock);
			    		return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
			}
			break;
		case FD_RW_SUB:
			FD_TRACE1("RW_SUB", FDP_DEV);

			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);

			/* read in the result data */
			fdadapter->command.total_bytes = 0;
			fdadapter->results.total_bytes = 7;
			piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
						fdadapter->io_address);
			fdadapter->error_value = 
					fddata_reg_pio((caddr_t) &piop);
			FD_BUSIO_DET(piop.bus_val);

			i_reset(fdadapter->fdhandler);
			if (fdadapter->error_value == 0)

				/*
				 * Only report DMA errors if there is no
				 * corresponding diskette controller error.
				 * Otherwise just go through normal retry
				 * logic.  Also only retry DMA errors that
				 * are retryable (rc == DMA_TC_NOTREACHED).
				 */

				if (rc != DMA_SUCC) {
					if (((fdadapter->
					    results.un1.names.byte0.status0 &
					    FDNORMAL) == 0) ||
					    (rc != DMA_TC_NOTREACHED)) {
						fdadapter->error_value = EIO;
					}
				}
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			}
			fdadapter->error_value = 0;
			change_line_value = fdread_change_line();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
		    		return(fdrw_exit(fdp, fdadapter->error_value));
			}
			if (change_line_value & FDDISK_CHANGE) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
		    		return(fdrw_exit(fdp, ENOTREADY));
			}
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != 0) {
				if (fdp->retry_flag == FALSE) {
					FD_TRACE1("out intr", FDP_DEV);
					fdassign_error(fdp);
					unlock_enable(old_pri, 
					    &fdadapter->intr_lock);
					return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				if (fdp->simple_count1 >=
				    FDMAX_SIMPLE_RETRIES1) {
/* starting line wrapping */
if (fdp->complex_count >= FDMAX_COMPLEX_RETRIES) {
	if (fdp->simple_count2 >= FDMAX_SIMPLE_RETRIES2) {
		if (fdp->last_error1 ==
		    fdadapter->results.un1.names.byte1.status1) {
			if (fdp->last_error2 ==
			    fdadapter->results.un1.names.status2) {
				if (fdp->last_error1 == 0x10)
					/* log overrun error */
					fdlog_error(fdp, 0x10);
				else
					/* log media error */
					fdlog_error(fdp, 0);
			}
		}
		fdp->last_error1 = fdadapter->results.un1.names.byte1.status1;
		fdp->last_error2 = fdadapter->results.un1.names.status2;
		FD_TRACE1("out intr", FDP_DEV);
		fdassign_error(fdp);
		unlock_enable(old_pri, &fdadapter->intr_lock);
		return(fdrw_exit(fdp, fdadapter->error_value));
	} else
  	 {
		fdp->simple_count2++;	
		if (fdp->headptr->b_flags & B_READ) 
			fdp->dma_flags = DMA_READ;
		else 
			fdp->dma_flags = 0;
		fdadapter->dio.used_iovecs = 1;
		fdadapter->dio.dvec->iov_base = 
			(char *)(fdp->headptr->b_un.b_addr + fdp->buf_offset);
		fdadapter->dio.dvec->iov_len = fdp->current.transfer_length;
		fdadapter->dio.dvec->xmp = &(fdp->headptr->b_xmemd);
		dma_rc = D_MAP_SLAVE (fdadapter->dma_id, fdp->dma_flags,
			     fdp->bytes_per_sector, &fdadapter->dio,
			     (CH_SINGLE | CH_ADDR_INC | CH_EOP_OUTPUT
			      | CH_TYPE_B | CH_8BIT_BYTES) );
		if (dma_rc != DMA_SUCC) {
			/* cannot satisfy the DMA request,  */
			/* send back 'bp' with an EIO error */
			FD_TRACE1("out intr", FDP_DEV);
			unlock_enable(old_pri, &fdadapter->intr_lock);
    			return(fdrw_exit(fdp, EIO));
		}

		fdadapter->sub_state = FD_RW_SUB;
		fdadapter->error_value = 0;
		fdrw();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out intr", FDP_DEV);
			unlock_enable(old_pri, &fdadapter->intr_lock);
			return(fdrw_exit(fdp, fdadapter->error_value));
		}
	}
/* ending line wrapping */
					} else
 					 {
						fdp->complex_count++;
						fdadapter->sub_state =
						    FD_RW_RESET;
						fdadapter->error_value = 0;
						fdreset();
						if (fdadapter->error_value
						    != 0) {
							FD_TRACE1("out intr",
							    FDP_DEV);
							unlock_enable(old_pri,
							    &fdadapter->
								intr_lock);
							return(fdrw_exit(fdp,
							    fdadapter->
							    error_value));
						}
					}
				} else
			 	 {
					fdp->simple_count1++;
					if (fdp->headptr->b_flags & B_READ) 
						fdp->dma_flags = DMA_READ;
					else 
						fdp->dma_flags = 0;
					fdadapter->dio.used_iovecs = 1;
					fdadapter->dio.dvec->iov_base = 
					     (char *)(fdp->headptr->b_un.b_addr 
						+ fdp->buf_offset),
					fdadapter->dio.dvec->iov_len = 
						fdp->current.transfer_length;
					fdadapter->dio.dvec->xmp = 
						&(fdp->headptr->b_xmemd);
					dma_rc = D_MAP_SLAVE (fdadapter->dma_id,
						fdp->dma_flags,
						fdp->bytes_per_sector,
						&fdadapter->dio,
						(CH_SINGLE | CH_ADDR_INC
						| CH_EOP_OUTPUT | CH_TYPE_B 
						| CH_8BIT_BYTES) );
					if (dma_rc != DMA_SUCC) {
						/* cannot satisfy the DMA  */
						/* request, send back 'bp' */
						/* with an EIO error       */
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp, EIO));
					}

					fdadapter->sub_state = FD_RW_SUB;
					fdadapter->error_value = 0;
					fdrw();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri,
						    &fdadapter->intr_lock);
			    			return(fdrw_exit(fdp,
				   		    fdadapter->error_value));
					}
				}
			} else
		 	 {
				fdp->total_bcount +=
				    fdp->current.transfer_length;

				/*
				 * Update the data transfer counters.
				 */

				if (fdp->headptr->b_flags & B_READ) {
					fdp->rcount_bytes +=
					    fdp->current.transfer_length;
					if (fdp->rcount_bytes >= 1000000) {
						fdp->rcount_megabytes +=
						    fdp->rcount_bytes /
						    1000000;
						fdp->rcount_bytes %= 1000000;
					}
				} else
				 {
					fdp->wcount_bytes +=
					    fdp->current.transfer_length;
					if (fdp->wcount_bytes >= 1000000) {
						fdp-> wcount_megabytes +=
						    fdp->wcount_bytes /
						    1000000;
						fdp->wcount_bytes %= 1000000;
					}
				}
				if (fdp->total_bcount != fdp->modified_bcount)
				    {	
					fdp->simple_count1 = 0;
					fdp->complex_count = 0;
					fdp->simple_count2 = 0;
					fdp->current.cylinder += 1;
					fdp->current.head = 0;
					fdp->current.sector = 1;
					fdp->buf_offset +=
					    fdp->current.transfer_length;
					temp = fdp->modified_bcount -
					    fdp->total_bcount;
					if ((temp / fdp->bytes_per_sector) >
					    fdp->sectors_per_cylinder)
						fdp->current.transfer_length = 
						    fdp->sectors_per_cylinder *
						    fdp->bytes_per_sector;
					else
						fdp->current.transfer_length =
						    temp;
					fdadapter->sub_state = FD_RW_SEEK1;
					fdp->cylinder_id = 
					    fdp->current.cylinder;
					fdadapter->error_value = 0;
					fdseek();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out intr", FDP_DEV);
						unlock_enable(old_pri, 
						    &fdadapter->intr_lock);
						return(fdrw_exit(fdp,
						    fdadapter->error_value));
					}
				} else
			 	 {

					/*
					 * This interrupt state is set here to
					 * catch spurious interrupts after a
					 * seek command.
					 */

					fdadapter->state =
					    FD_INITIAL_INTERRUPT;
					fdadapter->sub_state = NULL;
					bp = fdp->headptr;
					fdp->headptr = fdp->headptr->av_forw;
					bp->b_error = 0;
					bp->b_resid = bp->b_bcount -
					    fdp->total_bcount;
					FD_TRACE1("call iodone",
					    (dev_t)(bp->b_error));
					FD_TRACE2("bp:", (ulong)bp->b_flags,
					    (caddr_t)bp, (ulong)bp->b_bcount);
					iodone(bp);
					ASSERT((minor(fdp->device_number) >> 6)
					    == fdadapter->active_drive);
					fdunlock_adapter(fdadapter->
					    active_drive);
				}
			}	
			break;
		case FD_RW_RESET:
			FD_TRACE1("RW_RESET", FDP_DEV);
			fdadapter->error_value = 0;
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			}
			i_reset(fdadapter->fdhandler);
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) != FDNORMAL) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdrw_exit(fdp, fdadapter->error_value));
			} else
			 {
				fdadapter->error_value = 0;
				fdset_config();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri,
					       &fdadapter->intr_lock);
					return(fdrw_exit(fdp,
					       fdadapter->error_value));
				}
				fdadapter->error_value = 0;
				fdset_perp();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri, 
					    &fdadapter->intr_lock);
					return(fdrw_exit(fdp,
					    fdadapter->error_value));
				}
				fdadapter->error_value = 0;
				fdspecify();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out intr", FDP_DEV);
					unlock_enable(old_pri, 
					    &fdadapter->intr_lock);
					return(fdrw_exit(fdp,
					       fdadapter->error_value));
				}
				fdadapter->fdstart_timer->timeout.it_value.tv_sec = 0;
				fdadapter->fdstart_timer->timeout.it_value.tv_nsec =
				    fdp->motor_start * 1000000;
				fdadapter->fdstart_timer->func_data = (ulong)fdp;
				fdadapter->state = FD_RW;
				fdadapter->sub_state = FD_RW_RECAL1;
				tstart(fdadapter->fdstart_timer);
				FD_TRACE1("out intr", FDP_DEV);
			}
			break;
		default:
			ASSERT(fdadapter->sub_state != fdadapter->sub_state);
			break;
		}
		break;
	case FD_IO_RESET:
		FD_TRACE1("FDIO_RESET", FDP_DEV);
		fdadapter->error_value = 0;
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out intr", FDP_DEV);
			unlock_enable(old_pri, &fdadapter->intr_lock);
			return(fdio_exit(fdp, fdadapter->error_value));
		}
		i_reset(fdadapter->fdhandler);
		if ((fdadapter->results.un1.names.byte0.status0 &
		    FDNORMAL) != FDNORMAL) {
			FD_TRACE1("out intr", FDP_DEV);
			unlock_enable(old_pri, &fdadapter->intr_lock);
			return(fdio_exit(fdp, fdadapter->error_value));
		} else
		 {
			fdadapter->error_value = 0;
			fdselect_drive();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdio_exit(fdp, fdadapter->error_value));
			}
			fdadapter->error_value = 0;
			fdset_config();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdio_exit(fdp, fdadapter->error_value));
			}
			fdadapter->error_value = 0;
			fdset_perp();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdio_exit(fdp, fdadapter->error_value));
			}
			fdadapter->error_value = 0;
			fdspecify();
			if (fdadapter->error_value != 0) {
				FD_TRACE1("out intr", FDP_DEV);
				unlock_enable(old_pri, &fdadapter->intr_lock);
				return(fdio_exit(fdp, fdadapter->error_value));
			}
			fdadapter->fdstart_timer->timeout.it_value.tv_sec = 0;
			fdadapter->fdstart_timer->timeout.it_value.tv_nsec =
			    fdp->motor_start * 1000000;
			fdadapter->fdstart_timer->func_data = (ulong)fdp;
			fdadapter->state = FD_RW;
			fdadapter->sub_state = FD_RW_DELAY_TIMER;
			tstart(fdadapter->fdstart_timer);
		}	
		break;
	default:
		ASSERT(fdadapter->state != fdadapter->state);
		break;
	} /* switch adapter state */
	FD_TRACE1("out intr", FDP_DEV);
	unlock_enable(old_pri, &fdadapter->intr_lock);
	return(INTR_SUCC);
} /* end of fd_intr() */



/*
 * NAME: fd_pmh
 *
 * FUNCTION: The Power Management Handler
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.
 *
 * DATA STRUCTURES:
 * 
 * INPUTS: 
 *	private - pointer to the devices data structure.
 *		  either a "floppy" structure for the drives
 *		  or "adapter_structure" for the controller.
 *	ctrl - the PM mode to which to transition.
 *
 * RETURN VALUE DESCRIPTION:
 *	PM_SUCCESS - the mode was successfully changed
 *	PM_ERROR   - an error occurred and the mode was not changed
 *
 * ERROR CODES:
 *	PM_ERROR 
 *
 * NOTES:
 *	This function depends on the pm_handle being the first data
 *	entry for the "floppy" structure and the "adapter_structure" structure.
 */

int	fd_pmh (caddr_t private, int ctrl)
{
	int	adap_flag;
	int	i;
	int	old_level;
	int	rc;

	FD_TRACE1("in pmh", ctrl);

	/* check if it is a request for the adapter or the drive */
	adap_flag = ((struct pm_handle *) private)->devno & FD_ADAP_MINOR_NUM;

	switch (ctrl) {
	   case PM_DEVICE_IDLE:
		/* code is not guaranteed to be pinned for idle, 	*/
		/* so we need to pin the code and the adapter structure */
		rc = pincode (fd_intr);
		if (rc != 0) 
			return (PM_ERROR);

		/* pin the adapter (and PM) data structure */
		rc = pin ((caddr_t) fdadapter, 
			  sizeof(struct adapter_structure));			
		if (rc != 0) {
			unpincode (fd_intr);
			return (PM_ERROR);
		}

		/* pin the floppy structure if necessary */
		if (!adap_flag) {
			FD_TRACE1 ("ID pin dr", rc);
			rc = pin ((caddr_t) private, 
				  sizeof(struct floppy));			
			if (rc != 0) {
				unpincode (fd_intr);
				unpin ((caddr_t) fdadapter, 
					sizeof(struct adapter_structure));
				return (PM_ERROR);
			}
		}
			
		FD_TRACE1 ("IDLE pin", rc);

		/* for IDLE pin the code and treat like a */
		/* SUSPEND or HIBERNATE			  */
		
	   case PM_DEVICE_SUSPEND:
	   case PM_DEVICE_HIBERNATION:
		FD_TRACE1 ("ISH start", ctrl);
		/* serialize access to the controller */
		old_level = disable_lock(fdadapter->int_class, 
			 		 &fdadapter->intr_lock); 

		/* check for illegal transition */
		if (((struct pm_handle *) private)->mode == PM_DEVICE_FULL_ON) {
			FD_TRACE1 ("ISH igl", ctrl);
			unlock_enable(old_level, 
				      &fdadapter->intr_lock);
			if (ctrl == PM_DEVICE_IDLE) {
				rc = unpincode (fd_intr);
				ASSERT(rc == 0);
				rc = unpin ((caddr_t) fdadapter, 
					    sizeof(struct adapter_structure));
				ASSERT(rc == 0);
				if (!adap_flag) {
					rc = unpin ((caddr_t) private,
						sizeof(struct floppy));
					ASSERT(rc == 0);
				}
			}
			return (PM_ERROR);
		}

		/* check if adapter or drive */
		if (adap_flag) {
			
			/* check if any of the drives are active  */
			/* Do not power off the controller if the */
			/* drives are not powered off		  */
			for (i = 0; i < FDMAXDRIVES; i++) {
			    if (fdadapter->drive_list[i] != NULL) {

				/* check if not in powered down mode */
				if (fdadapter->drive_list[i]->pwr_mgt.mode == 
							PM_DEVICE_FULL_ON ||
				    fdadapter->drive_list[i]->pwr_mgt.mode == 
							PM_DEVICE_ENABLE) {
					FD_TRACE1 ("ISH busy", i);
					unlock_enable(old_level, 
						      &fdadapter->intr_lock);

					/* adapter check already done */
					/* do not unpin floppy struct */
					if (ctrl == PM_DEVICE_IDLE) {
						rc = unpincode (fd_intr);
						ASSERT (rc == 0);
						rc = unpin ((caddr_t) fdadapter,
						       sizeof(struct 
							adapter_structure));
						ASSERT (rc == 0);
					}
					return (PM_ERROR);
				}
			    }
			}
			/* kill power to the controller */
			pm_planar_control (fdadapter->pwr_mgt.devno,
					   PMDEV_FDC, PM_PLANAR_OFF);
		}
		/* just power off the drive if it is closed */
		else {
			/* check if the drive is open */
			if (!(((struct floppy *) private)->
					drive_state & (FDCLOSED | FDOPENING))) {
				FD_TRACE1 ("ISH busy", private);
				unlock_enable(old_level, 
					      &fdadapter->intr_lock);
				if (ctrl == PM_DEVICE_IDLE) {
					rc = unpincode (fd_intr);
					ASSERT (rc == 0);
					rc = unpin ((caddr_t) fdadapter, 
					      sizeof(struct adapter_structure));
					ASSERT (rc == 0);

					/* adapter check already done */
					/* unpin floppy struct 	      */
					rc = unpin ((caddr_t) private,
						sizeof(struct floppy));
					ASSERT(rc == 0);
				}
				return (PM_ERROR);
			}
			pm_planar_control(((struct pm_handle *) private)->devno,
					  ((struct floppy *) private)->pm_devid,
					  PM_PLANAR_OFF);

			FD_TRACE1 ("kill dr", private);

			/* check if any I/O requests were blocked while in */
			/* suspend or hibernate.  If so (and switching to  */
			/* idle), then wake the processes up.  This will   */
			/* eventually set the mode to enable.		   */
			if (ctrl == PM_DEVICE_IDLE && 
			    ((struct floppy *) private)->pwr_mgt_block == TRUE) {
				((struct floppy *) private)->pwr_mgt_block = FALSE;
				FD_TRACE1("wake proc", ctrl);
				e_wakeup (&((struct floppy *) private)->
							pwr_mgt_sleep_anchor);
			}
		}

		((struct pm_handle *) private)->mode = ctrl;
		FD_TRACE1("pm-ISH", ctrl);
		unlock_enable(old_level, &fdadapter->intr_lock);

		/* unpin code if we switched to idle mode */		
		if (ctrl == PM_DEVICE_IDLE) {
			rc = unpincode (fd_intr);
			ASSERT (rc == 0);
			rc = unpin ((caddr_t) fdadapter, 
				sizeof(struct adapter_structure));
			ASSERT (rc == 0);
			if (!adap_flag) {
				rc = unpin ((caddr_t) private,
					sizeof(struct floppy));
				ASSERT(rc == 0);
			}
			FD_TRACE1("pmh IDLE", ctrl);
		}
		break;

	   case PM_DEVICE_ENABLE:
		/* code is not guaranteed to be pinned for enable,	*/
		/* so we need to pin the code and the adapter structure */
		rc = pincode (fd_intr);
		if (rc != 0) 
			return (PM_ERROR);

		/* pin the adapter (and PM) data structure */
		rc = pin ((caddr_t) fdadapter, 
			  sizeof(struct adapter_structure));			
		if (rc != 0) {
			unpincode (fd_intr);
			return (PM_ERROR);
		}

		/* pin the floppy structure if necessary */
		if (!adap_flag) {
			rc = pin ((caddr_t) private, 
				  sizeof(struct floppy));			
			if (rc != 0) {
				unpincode (fd_intr);
				unpin ((caddr_t) fdadapter, 
					sizeof(struct adapter_structure));
				return (PM_ERROR);
			}
		}

		/* serialize access to the controller */
		old_level = disable_lock(fdadapter->int_class, 
			 		 &fdadapter->intr_lock); 

		switch (((struct pm_handle *) private)->mode) {
		   case PM_DEVICE_FULL_ON:
		   case PM_DEVICE_ENABLE:
			break;

		   /* this transition to enable should be handled */
		   /* by the I/O activity points, which call the  */
		   /* function fdcheck_pm_mode()	          */
		   /* but, just in case ...			  */
		   case PM_DEVICE_IDLE:
		   case PM_DEVICE_SUSPEND:
		   case PM_DEVICE_HIBERNATION:
			/* check if adapter or drive */
			if (adap_flag) {
				FD_TRACE1("pm I-E adp", ctrl);
				pm_planar_control (fdadapter->pwr_mgt.devno,
						   PMDEV_FDC, PM_PLANAR_ON);
			}
			else {
				/* check if adapter is active */
				/* if so, activate the drive  */
				if (fdadapter->pwr_mgt.mode == 
							PM_DEVICE_FULL_ON ||
				    fdadapter->pwr_mgt.mode == 
							PM_DEVICE_ENABLE) {
					FD_TRACE1("pm I-E flp", ctrl);
					pm_planar_control (((struct pm_handle *) 
								private)->devno,
						   ((struct floppy *) private)->pm_devid,
						   PM_PLANAR_ON);
				}
				else {
					unlock_enable(old_level, 
						      &fdadapter->intr_lock);
					rc = unpincode (fd_intr);
					ASSERT (rc == 0);
					rc = unpin ((caddr_t) fdadapter, 
					      sizeof(struct adapter_structure));
					ASSERT (rc == 0);
					if (!adap_flag) {
						rc = unpin ((caddr_t) private,
							sizeof(struct floppy));
						ASSERT(rc == 0);
					}
					FD_TRACE1("pm I-E no", 
						   fdadapter->pwr_mgt.mode);
					return (PM_ERROR);
				}
			}
			break;

		   default:
			unlock_enable(old_level, 
				      &fdadapter->intr_lock);
			rc = unpincode (fd_intr);
			ASSERT (rc == 0);
			rc = unpin ((caddr_t) fdadapter, 
				sizeof(struct adapter_structure));
			ASSERT (rc == 0);
			if (!adap_flag) {
				rc = unpin ((caddr_t) private,
					sizeof(struct floppy));
				ASSERT(rc == 0);
			}
			FD_TRACE1("out pmh", ctrl);
			return (PM_ERROR);
			break;
		} /* switch */
		
		((struct pm_handle *) private)->mode = PM_DEVICE_ENABLE;
		unlock_enable(old_level, &fdadapter->intr_lock);

		rc = unpincode (fd_intr);
		ASSERT (rc == 0);
		rc = unpin ((caddr_t) fdadapter, 
			sizeof(struct adapter_structure));
		ASSERT (rc == 0);
		if (!adap_flag) {
			rc = unpin ((caddr_t) private,
				sizeof(struct floppy));
			ASSERT(rc == 0);
		}
		FD_TRACE1("pmh ENABLE", ctrl);
		break;

	   case PM_DEVICE_FULL_ON:
		FD_TRACE1 ("FULL start", ctrl);
		if (((struct pm_handle *) private)->mode != PM_DEVICE_ENABLE) {
			FD_TRACE1("out pmh", ((struct pm_handle *) private)->mode);
			return (PM_ERROR);
		}
		((struct pm_handle *) private)->mode = PM_DEVICE_FULL_ON;
		FD_TRACE1 ("FULL end", ctrl);
		break;

	   case PM_PAGE_FREEZE_NOTICE:
		FD_TRACE1 ("FRZ start", ctrl);

		/* pin the code */
		rc = pincode (fd_intr);
		if (rc != 0) {
			FD_TRACE1("opmh pcd", rc);
			return (PM_ERROR);
		}

		/* pin the adapter (and PM) data structure */
		rc = pin ((caddr_t) fdadapter, 
			  sizeof(struct adapter_structure));			
		if (rc != 0) {
			unpincode (fd_intr);
			FD_TRACE1("opmh pad", rc);
			return (PM_ERROR);
		}

		/* pin the floppy structure if necessary */
		if (!adap_flag) {
			rc = pin ((caddr_t) private, 
				  sizeof(struct floppy));			
			if (rc != 0) {
				unpincode (fd_intr);
				unpin ((caddr_t) fdadapter, 
					sizeof(struct adapter_structure));
				FD_TRACE1("opmh pdr", rc);
				return (PM_ERROR);
			}
		}
		FD_TRACE1 ("FRZ end", ctrl);
		break;

	   case PM_PAGE_UNFREEZE_NOTICE:
		/* unpin the structure and code pinned by */
		/* PM_PAGE_FREEZE_NOTICE		  */
		FD_TRACE1 ("UFRZ strt", ctrl);
		rc = unpincode (fd_intr);
		ASSERT (rc == 0);
		rc = unpin ((caddr_t) fdadapter, 
				sizeof(struct adapter_structure));
		ASSERT (rc == 0);
		if (!adap_flag) {
			rc = unpin ((caddr_t) private,
				sizeof(struct floppy));
			ASSERT (rc == 0);
		}
		break;

	   default:
		break;
	}

	FD_TRACE1("out pmh", ctrl);
	return (PM_SUCCESS);
} /* fd_pmh() */

/*
 * The following are the bottom half internal device driver routines.
 */

/*
 * NAME: fdcheck_pm_mode
 *
 * FUNCTION: 
 *	This is called by the I/O activity points (fd_strategy and 
 *	fdlock_adapter) to check if it is possible to complete the
 *	I/O due to the power management state.
 *
 * EXECUTION ENVIRONMENT:
 *  This function can be called from a interrupt level.
 *  Interrupts are assumed to be disabled.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS: 
 *	fdp - a pointer to the "floppy" structure for the selected drive.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * ERROR CODES: none.
 *
 */

void	fdcheck_pm_mode (struct floppy *fdp)
{
	int	i;
	
	FD_TRACE1 ("chk pmmd", fdp);

	switch (fdp->pwr_mgt.mode) {
	   case PM_DEVICE_FULL_ON:
		if (fdadapter->pwr_mgt.mode == PM_DEVICE_IDLE) {
			/* indicate use of the controller */
			fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;

			/* activate the controller and the drives */
			pm_planar_control (fdadapter->pwr_mgt.devno, 
					   PMDEV_FDC, PM_PLANAR_ON);

			/* since the device is active again, */
			/* change the state to enable        */
			fdadapter->pwr_mgt.mode = PM_DEVICE_ENABLE;

		} else if (fdadapter->pwr_mgt.mode == PM_DEVICE_ENABLE) {
			/* indicate use of the controller */
			fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;
		}
		break;

	   case PM_DEVICE_ENABLE:
		if (fdadapter->pwr_mgt.mode == PM_DEVICE_IDLE) {
			/* indicate use of the controller */
			fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;

			/* activate the controller and the drives */
			pm_planar_control (fdadapter->pwr_mgt.devno, 
					   PMDEV_FDC, PM_PLANAR_ON);

			/* since the device is active again, */
			/* change the state to enable        */
			fdadapter->pwr_mgt.mode = PM_DEVICE_ENABLE;

		} else if (fdadapter->pwr_mgt.mode == PM_DEVICE_ENABLE) {
			/* indicate use of the controller */
			fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;
		}

		/* indicate use of the drive */
		fdp->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;
		break;

	   case PM_DEVICE_SUSPEND:
	   case PM_DEVICE_HIBERNATION:
		/* indicate that a process is blocked */
		fdp->pwr_mgt_block = TRUE;

		/* put the process to sleep */
		FD_TRACE1 ("pm sleep", fdp->pwr_mgt.mode);
		e_sleep_thread (&fdp->pwr_mgt_sleep_anchor, 
				&fdadapter->intr_lock, LOCK_HANDLER);
		FD_TRACE1 ("pm wake", fdp->pwr_mgt.mode);

		/* once the process is awakened, process the I/O  */
		/* by falling through to the IDLE state		  */
		/* the state has been switched to IDLE the fd_pmh */

	   case PM_DEVICE_IDLE:
		if (fdadapter->pwr_mgt.mode == PM_DEVICE_IDLE) {
			/* indicate use of the controller */
			fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;

			/* activate the controller and the drives */
			pm_planar_control (fdadapter->pwr_mgt.devno, 
					   PMDEV_FDC, PM_PLANAR_ON);

			/* since the device is active again, */
			/* change the state to enable        */
			fdadapter->pwr_mgt.mode = PM_DEVICE_ENABLE;
		} else if (fdadapter->pwr_mgt.mode == PM_DEVICE_ENABLE) {
			/* indicate use of the controller */
			fdadapter->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;
		}

		/* indicate use of the drive */
		fdp->pwr_mgt.activity = PM_ACTIVITY_OCCURRED;

		/* activate the controller and the drives */
		pm_planar_control (fdp->pwr_mgt.devno, 
				   fdp->pm_devid, PM_PLANAR_ON);

		/* since the device is active again, */
		/* change the state to enable        */
		fdp->pwr_mgt.mode = PM_DEVICE_ENABLE;
		break;

	   default:
		break;
	} /* switch */
} /* fdcheck_pm_mode () */

/*
 * NAME: fdcleanup
 *
 * FUNCTION: Cleans up initialization stuff prior to exiting.
 *  This routine is called just before exiting in fd_open and
 *  fd_close.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called from a process or interrupt level.
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

	FD_TRACE1("in cleanup", FDA_DEV);
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
			ASSERT(rc == 0);
			fdadapter->fderrptr = NULL;
		}
		if (fdadapter->int_init) {
			i_clear(fdadapter->fdhandler);
			fdadapter->int_init = FALSE;
		}
		if (fdadapter->fdhandler != NULL) {
			rc = xmfree((caddr_t)fdadapter->fdhandler, pinned_heap);
			ASSERT(rc == 0);
			fdadapter->fdhandler = NULL;
		}
		if (fdadapter->dma_init) {
			D_MAP_CLEAR(fdadapter->dma_id);
			fdadapter->dma_init = FALSE;
		}
		if (fdadapter->inttimer.func != NULL) {
			w_stop(&fdadapter->inttimer);
			while(w_clear(&fdadapter->inttimer));
			fdadapter->inttimer.func = NULL;
		}
		if (fdadapter->mottimer.func != NULL) {
			w_stop(&fdadapter->mottimer);
			while(w_clear(&fdadapter->mottimer));
			fdadapter->mottimer.func = NULL;
		}
		if (fdadapter->fdstart_timer != NULL) {
			while (tstop(fdadapter->fdstart_timer));
			tfree(fdadapter->fdstart_timer);
			fdadapter->fdstart_timer = NULL;
		}
		if (fdadapter->pinned == TRUE) {
			rc = unpin((caddr_t)fdadapter,
			    sizeof(struct adapter_structure));
			ASSERT(rc == 0);
			fdadapter->pinned = FALSE;
		}
		if (fdadapter->initialized == TRUE) {
			rc = unpincode(fd_intr);
			ASSERT(rc == 0);
			fdadapter->initialized = FALSE;
		}
	}
	FD_TRACE1("out cleanup",FDA_DEV);
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
 a
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

uchar	fdread_change_line ()
{
	uchar	rc;
	struct	pio_parms piop;

	FD_TRACE1("in chgline",FDA_DEV);
	piop.reg = FDDISK_CHANGED_REG;
	piop.read = TRUE;
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);
	rc = piop.data;
	FD_BUSIO_DET(piop.bus_val);

	FD_TRACE1("out chgline", FDA_DEV);
	return(rc);
} /* end of fdread_change_line() */


uchar	fdread_main_status ()
{
	uchar	rc;
	struct	pio_parms piop;
	struct	timestruc_t  start_time, end_time;
	ulong	elapsed_time;

	FD_TRACE1("in mainsta", FDA_DEV);

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	piop.reg = FDMAIN_STATUS_REG;
	piop.read = TRUE;

        /* loop until the controller is ready for the transfer of */
        /* data or timeout waiting for the controller to be ready */
        curtime(&start_time);
        do {
                /* read main status register */
		fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);
                rc = piop.data;

                /* get elapsed time in loop */
                curtime(&end_time);
                if (start_time.tv_sec == end_time.tv_sec) {
                        elapsed_time = end_time.tv_nsec - start_time.tv_nsec;
                } else
                 {
                        elapsed_time = NS_PER_SEC - start_time.tv_nsec;
                        elapsed_time += (end_time.tv_nsec + (end_time.tv_sec -
				 start_time.tv_sec - 1) * NS_PER_SEC);
                }

                /* check timeout condition */
                if (elapsed_time > FDMAX_CTLR_WAIT) 
                        break;
        } while (rc != FDRFM_MASK);

	FD_BUSIO_DET(piop.bus_val);
	FD_TRACE1("out mainsta", FDA_DEV);
	return(rc);
} /* end of fdread_main_status() */


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
	int	evalue;

	FD_TRACE1("in fdio", bp->b_dev);
	DDHKWD5(HKWD_DD_FDDD, DD_ENTRY_BSTART, 0, bp->b_dev, bp, bp->b_flags,
	    bp->b_blkno, bp->b_bcount);
	drive_number = minor(bp->b_dev) >> 6;
	if (fdadapter->adapter_busy == FALSE) {
		fdadapter->adapter_busy = TRUE;
		fdadapter->active_drive = drive_number;
		fdp = fdadapter->drive_list[drive_number];
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
			FD_TRACE1("call iodone", (dev_t)(bp->b_error));
			FD_TRACE2("bp:", (ulong)bp->b_flags, (caddr_t)bp,
			    (ulong)bp->b_bcount);
			iodone(bp);
			fdunlock_adapter(drive_number);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART, EINVAL,
			    drive_number);
			FD_TRACE1("out fdio", bp->b_dev);
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
			FD_TRACE1("call iodone", (dev_t)(bp->b_error));
			FD_TRACE2("bp:", (ulong)bp->b_flags, (caddr_t)bp,
			    (ulong)bp->b_bcount);
			iodone(bp);
			fdunlock_adapter(drive_number);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART, evalue,
			    drive_number);
			FD_TRACE1("out fdio", bp->b_dev);
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

		fdp->start.cylinder = 
		    bp->b_blkno / (fdp->sectors_per_cylinder);
		temp = bp->b_blkno % (fdp->sectors_per_cylinder);
		fdp->start.head = temp / fdp->sectors_per_track;
		fdp->start.sector = (temp % fdp->sectors_per_track) + 1;
  		bytes_left = (fdp->sectors_per_cylinder - (fdp->start.head *
		    fdp->sectors_per_track) - fdp->start.sector + 1) *
		    fdp->bytes_per_sector;

		/*
		 * Set up the other fields in 'floppy' that are used for
		 * keeping track of reads and writes.
		 */

		fdp->start_block = bp->b_blkno;
		fdp->buf_offset = 0;
		fdp->current.cylinder = fdp->start.cylinder;
		fdp->cylinder_id = fdp->current.cylinder;
		fdp->current.head = fdp->start.head;
		fdp->current.sector = fdp->start.sector;
		if (bytes_left <= fdp->modified_bcount)
			fdp->current.transfer_length = bytes_left;
		else
			fdp->current.transfer_length = fdp->modified_bcount;
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
					FD_TRACE1("call iodone",
					    (dev_t)(bp->b_error));
					FD_TRACE2("bp:", (ulong)bp->b_flags,
					    (caddr_t)bp, (ulong)bp->b_bcount);
					iodone(bp);
					fdunlock_adapter(drive_number);
				}
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
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
				FD_TRACE1("call iodone", (dev_t)(bp->b_error));
				FD_TRACE2("bp:", (ulong)bp->b_flags,
				    (caddr_t)bp, (ulong)bp->b_bcount);
				iodone(bp);
				fdunlock_adapter(drive_number);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
				return;
			}
			fdadapter->error_value = 0;
			fdset_config();
			evalue = fdadapter->error_value;
			if (evalue != 0) {
				fdadapter->reset_needed = TRUE;
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				FD_TRACE1("call iodone", (dev_t)(bp->b_error));
				FD_TRACE2("bp:", (ulong)bp->b_flags,
				    (caddr_t)bp, (ulong)bp->b_bcount);
				iodone(bp);
				fdunlock_adapter(drive_number);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
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
				FD_TRACE1("call iodone", (dev_t)(bp->b_error));
				FD_TRACE2("bp:", (ulong)bp->b_flags,
				    (caddr_t)bp, (ulong)bp->b_bcount);
				iodone(bp);
				fdunlock_adapter(drive_number);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
				return;
			}
			fdadapter->error_value = 0;
			fdset_perp();
			evalue = fdadapter->error_value;
			if (evalue != 0) {
				fdadapter->reset_needed = TRUE;
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				FD_TRACE1("call iodone", (dev_t)(bp->b_error));
				FD_TRACE2("bp:", (ulong)bp->b_flags,
				    (caddr_t)bp, (ulong)bp->b_bcount);
				iodone(bp);
				fdunlock_adapter(drive_number);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
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
				FD_TRACE1("call iodone", (dev_t)(bp->b_error));
				FD_TRACE2("bp:", (ulong)bp->b_flags,
				    (caddr_t)bp, (ulong)bp->b_bcount);
				iodone(bp);
				fdunlock_adapter(drive_number);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
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
				bp->b_flags |= B_ERROR;
				bp->b_error = evalue;
				bp->b_resid = bp->b_bcount;
				fdp->headptr = fdp->headptr->av_forw;
				FD_TRACE1("call iodone", (dev_t)(bp->b_error));
				FD_TRACE2("bp:", (ulong)bp->b_flags,
				    (caddr_t)bp, (ulong)bp->b_bcount);
				iodone(bp);
				fdunlock_adapter(drive_number);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART,
				    evalue, drive_number);
				FD_TRACE1("out fdio", bp->b_dev);
				return;
			}
		}
	} /* adapter_busy */
	FD_TRACE1("out fdio", bp->b_dev);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_BSTART, FDSUCCESS, drive_number);
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

	FD_TRACE1("in load", FDP_DEV);
	drive_number = (uchar)(minor(fdp->device_number) >> 6);
	fdadapter->mottimer.restart = fdp->motor_off_time;
	switch (value) {
	case FDGENERIC:

/*     THIS SECTION IS FOR 4 MEG DEVICE DRIVER IMPLEMENTATION           */
 /* THE FOLLOWING INFORMATION IS SETTING THE VALUES FOR 4 MEG DISKETTE */
		if (fdp->drive_type == D_1354H) { /* default to 3.5, 2.88 M */
			fdp->diskette_type = FDDOUBLE | FD36PRTRCK | FD80CYLS;
			fdp->sector_size = 0x2;
			fdp->bytes_per_sector = 512;
			fdp->sectors_per_track = 36;
			fdp->sectors_per_cylinder = 72;
			fdp->tracks_per_cylinder = 2;
			fdp->cylinders_per_disk = 80;
			fdp->step_size = 1;
			fdp->number_of_blocks = 5760;
			fdp->data_rate = 0x03;
			fdp->head_settle_time = 
			    fdconfparms[drive_number].head_settle;
			fdp->head_load = fdp->head_settle_time;
			fdp->head_unload = 0x1;
			fdp->motor_start = 
			    fdconfparms[drive_number].motor_start;
			fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 +
			    1; /* timer ticks */
			fdp->fill_byte = 0xf6;
			fdp->step_rate_time = 
			    fdconfparms[drive_number].step_rate;
			fdp->step_rate = 0x0a;
			fdp->gap = 0x38;
			fdp->format_gap = 0x53;
			fdp->motor_speed = 300; /* speed in rpms */

			/* 
			 * motor speed (microseconds).
			 */

			fdp->motor_speed_time = 200000;
			fdp->drive_type = D_1354H;
			fdp->data_length = 0xff;
			
		} else

/*     THIS SECTION END FOR 4 MEG DEVICE DRIVER IMPLEMENTATION     */

		if (fdp->drive_type == D_135H) /* default to 3.5", 1.44 M */ {
			fdp->diskette_type = FDDOUBLE | FD18PRTRCK | FD80CYLS;
			fdp->sector_size = 0x2;
			fdp->bytes_per_sector = 512;
			fdp->sectors_per_track = 18;
			fdp->sectors_per_cylinder = 36;
			fdp->tracks_per_cylinder = 2;
			fdp->cylinders_per_disk = 80;
			fdp->step_size = 1;
			fdp->number_of_blocks = 2880;
			fdp->data_rate = 0x0;
			fdp->head_settle_time = 
			    fdconfparms[drive_number].head_settle;
			fdp->head_load = (fdp->head_settle_time + 1) / 2;
			fdp->head_unload = 0; /* obsolete */
			fdp->motor_start = 
			    fdconfparms[drive_number].motor_start;
			fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 +
			    1; /* timer ticks */
			fdp->fill_byte = 0xf6;
			fdp->step_rate_time = 
			    fdconfparms[drive_number].step_rate;
			fdp->step_rate = 16 - fdp->step_rate_time;
			fdp->gap = 0x1b;
			fdp->format_gap = 0x6c;
			fdp->motor_speed = 300; /* speed in rpms */

			/* 
			 * motor speed (microseconds).
			 */

			fdp->motor_speed_time = 200000;
			fdp->drive_type = D_135H;
			fdp->data_length = 0xff;
		} else /* default to 5.25", 1.2 M */        {
			fdp->diskette_type = FDDOUBLE | FD15PRTRCK | FD80CYLS;
			fdp->bytes_per_sector = 512;
			fdp->sector_size = 0x2;
			fdp->sectors_per_track = 15;
			fdp->sectors_per_cylinder = 30;
			fdp->tracks_per_cylinder = 2;
			fdp->cylinders_per_disk = 80;
			fdp->step_size = 1;
			fdp->number_of_blocks = 2400;
			fdp->data_rate = 0x0;
			fdp->head_settle_time =
			    fdconfparms[drive_number].head_settle;
			fdp->head_load = (fdp->head_settle_time + 1) / 2;
			fdp->head_unload = 0; /* obsolete */
			fdp->motor_start =
			    fdconfparms[drive_number].motor_start;
			fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 +
			    1; /* timer ticks */
			fdp->fill_byte = 0xf6;
			fdp->step_rate_time = 
			    fdconfparms[drive_number].step_rate;
			fdp->step_rate = 16 - fdp->step_rate_time;
			fdp->gap = 0x1b;
			fdp->format_gap = 0x54;
			fdp->motor_speed = 360;
			fdp->motor_speed_time = 166667;
			fdp->drive_type = D_96;
			fdp->data_length = 0xff;
		}
		break; /* end of FDGENERIC case */
	case FD2880_3:
		fdp->diskette_type = FDDOUBLE | FD36PRTRCK | FD80CYLS;
		fdp->bytes_per_sector = 512;
		fdp->sector_size = 0x2;
		fdp->sectors_per_track = 36;
		fdp->sectors_per_cylinder = 72;
		fdp->tracks_per_cylinder = 2;
		fdp->cylinders_per_disk = 80;
		fdp->step_size = 1;
		fdp->number_of_blocks = 5760;
		fdp->data_rate = 0x03;
		fdp->head_settle_time = fdconfparms[drive_number].head_settle;
		fdp->head_load = fdp->head_settle_time;
		fdp->head_unload = 0x1;
		fdp->motor_start = fdconfparms[drive_number].motor_start;
		fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 + 1;
		fdp->fill_byte = 0xf6;
		fdp->step_rate_time = fdconfparms[drive_number].step_rate;
		fdp->step_rate = 0x0a;
		fdp->gap = 0x38;
		fdp->format_gap = 0x53;
		fdp->motor_speed = 300;
		fdp->motor_speed_time = 200000;
		fdp->drive_type = D_1354H;
		fdp->data_length = 0xff;
		break; /* end of FD2880_3 case */
	case FD1440_3:
		fdp->diskette_type = FDDOUBLE | FD18PRTRCK | FD80CYLS;
		fdp->bytes_per_sector = 512;
		fdp->sector_size = 0x2;
		fdp->sectors_per_track = 18;
		fdp->sectors_per_cylinder = 36;
		fdp->tracks_per_cylinder = 2;
		fdp->cylinders_per_disk = 80;
		fdp->step_size = 1;
		fdp->number_of_blocks = 2880;
		fdp->data_rate = 0x4;
		fdp->head_settle_time = fdconfparms[drive_number].head_settle;
		fdp->motor_start = fdconfparms[drive_number].motor_start;
		fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 + 1;
		fdp->fill_byte = 0xf6;
		fdp->step_rate_time = fdconfparms[drive_number].step_rate;
		fdp->gap = 0x1b;
		if (fdp->drive_type == D_1354H) {
		    fdp->format_gap = 0x65;
		    fdp->head_unload = 0x1;
		    fdp->step_rate = 0x0d;
		}
		else {
		    fdp->drive_type = D_135H;
		    fdp->format_gap = 0x6c;
		    fdp->head_unload = 0; /* obsolete */
		    fdp->step_rate = 16 - fdp->step_rate_time;
		}
		fdp->head_load = (fdp->head_settle_time + 1) / 2;
		fdp->motor_speed = 300;
		fdp->motor_speed_time = 200000;
		fdp->data_length = 0xff;
		break; /* end of FD1440_3 case */
	case FD720_3:
		fdp->diskette_type = FDDOUBLE | FD9PRTRCK | FD80CYLS;
		fdp->bytes_per_sector = 512;
		fdp->sector_size = 0x2;
		fdp->sectors_per_track = 9;
		fdp->sectors_per_cylinder = 18;
		fdp->tracks_per_cylinder = 2;
		fdp->cylinders_per_disk = 80;
		fdp->step_size = 1;
		fdp->number_of_blocks = 1440;
		fdp->data_rate = 0x6;
		fdp->head_settle_time = fdconfparms[drive_number].head_settle;
		fdp->motor_start = fdconfparms[drive_number].motor_start;
		fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 + 1;
		fdp->fill_byte = 0xf6;
		fdp->step_rate_time = fdconfparms[drive_number].step_rate;
		fdp->gap = 0x2a;
		if (fdp->drive_type == D_1354H) {
		    fdp->head_load = (fdp->head_settle_time + 1) / 4;
		    fdp->head_unload = 0x1;
		    fdp->step_rate = 0x0e;
		}
		else {
		    fdp->drive_type = D_135H;
		    fdp->head_load = (fdp->head_settle_time + 1) / 2;
		    fdp->head_unload = 0; /* obsolete */
		    fdp->step_rate = 16 - fdp->step_rate_time;
		}
		fdp->format_gap = 0x54;
		fdp->motor_speed = 300;
		fdp->motor_speed_time = 200000;
		fdp->data_length = 0xff;
		break; /* end of FD720_3 case */
	case FD1200_5:
		fdp->diskette_type = FDDOUBLE | FD15PRTRCK | FD80CYLS;
		fdp->bytes_per_sector = 512;
		fdp->sector_size = 0x2;
		fdp->sectors_per_track = 15;
		fdp->sectors_per_cylinder = 30;
		fdp->tracks_per_cylinder = 2;
		fdp->cylinders_per_disk = 80;
		fdp->step_size = 1;
		fdp->number_of_blocks = 2400;
		fdp->data_rate = 0x0;
		fdp->head_settle_time = fdconfparms[drive_number].head_settle;
		fdp->head_load = (fdp->head_settle_time + 1) / 2;
		fdp->head_unload = 0; /* obsolete */
		fdp->motor_start = fdconfparms[drive_number].motor_start;
		fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 + 1;
		fdp->fill_byte = 0xf6;
		fdp->step_rate_time = fdconfparms[drive_number].step_rate;
		fdp->step_rate = 16 - fdp->step_rate_time;
		fdp->gap = 0x1b;
		fdp->format_gap = 0x54;
		fdp->motor_speed = 360;
		fdp->motor_speed_time = 166667;
		fdp->drive_type = D_96;
		fdp->data_length = 0xff;
		break; /* end of FD1200_5 case */
	case FD360_5:
		fdp->diskette_type = FDDOUBLE | FD9PRTRCK | FD40CYLS;
		fdp->bytes_per_sector = 512;
		fdp->sector_size = 0x2;
		fdp->sectors_per_track = 9;
		fdp->sectors_per_cylinder = 18;
		fdp->tracks_per_cylinder = 2;
		fdp->cylinders_per_disk = 40;
		fdp->step_size = 2;
		fdp->number_of_blocks = 720;
		fdp->data_rate = 0x1;
		fdp->head_settle_time = fdconfparms[drive_number].head_settle;
		fdp->head_load = (fdp->head_settle_time + 1) / 2;
		fdp->head_unload = 0; /* obsolete */
		fdp->motor_start = fdconfparms[drive_number].motor_start;
		fdp->motor_ticks = (fdp->motor_start * HZ) / 1000 + 1;
		fdp->fill_byte = 0xf6;
		fdp->step_rate_time = fdconfparms[drive_number].step_rate;
		fdp->step_rate = 16 - fdp->step_rate_time;
		fdp->gap = 0x2a;
		fdp->format_gap = 0x50;
		fdp->motor_speed = 360;
		fdp->motor_speed_time = 166667;
		fdp->drive_type = D_96;
		fdp->data_length = 0xff;
		break; /* end of FD360_5 case */
	default:
		fdp->diskette_type = FDUNKNOWN;
		rc = EINVAL;
		break;
	}
	FD_TRACE1("out load", FDP_DEV);
	return(rc);
} /* end of fdload_floppy() */



/*
 * NAME: fdlog_error
 *
 * FUNCTION: Logs a hard diskette error.
 *  This routine logs a diskette error that has occurred on two
 *  consecutive i/o attempts with retries on.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called from the process or the interrupt level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS:
 *  fdp   - a pointer to the floppy structure to use.
 *  value - the error code.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

void fdlog_error (register struct floppy *fdp, uchar value)
{
	uchar	temp;
	int	i;

	FD_TRACE1("in log", FDP_DEV);
	for (i = 0; i < 8; i++) {
		fdadapter->fderrptr->header.resource_name[i] =
			fdp->resource_name[i];
	}
	if (fdp == NULL) {
		switch (value) {

		/*
		 * temporary diskette PIO condition, hardware error
		 */

		case 0x01:
			fdadapter->fderrptr->header.error_id =
			    ERRID_DISKETTE_ERR5;
			break;

		/*
		 * permanent diskette PIO condition, hardware error
		 */

		case 0x02:
			fdadapter->fderrptr->header.error_id =
			    ERRID_DISKETTE_ERR6;
			break;

		/*
		 * unknown error
		 */

		default:
			fdadapter->fderrptr->header.error_id =
			    ERRID_DISKETTE_ERR4;
			break; /* end of default case */
		} /* end of switch on value */

		/*
		 * log the error.
		 */

		errsave((char *)fdadapter->fderrptr, sizeof(struct err_rec0 ));
	} else
	 {
		switch (fdadapter->data_rate) {
		case 0:
			temp = FD500KBPS;
			break;
		case 1:
			temp = FD300KBPS;
			break;
		case 2:
			temp = FD250KBPS;
			break;
		case 3:
			temp = FD1MBPS;
			break;
		case 4:
			temp = FD500KBPS;
			break;
		case 6:
			temp = FD250KBPS;
			break;
		default:
			break;
		} /* end of switch on data_rate */
		fdadapter->fderrptr->data.status1 = temp;
                switch (fdadapter->motor_on) {
                case 0:
                        temp = FDDRIVE0;
                        break;
                case 1:
                        temp = FDDRIVE1;
                        break;
                case FDNO_DRIVE:
                        temp = FDNODRIVE;
                        break;
                default:
                        break;
                } /* end of switch on motor_on */
                fdadapter->fderrptr->data.status1 |= temp;
		if (fdp->retry_flag) 
			temp = FDRETRY;
		else 
			temp = 0;
		if (fdadapter->error_value = ENOTREADY)
			temp |= FDTIMEOUT;
		switch (fdp->drive_type) {
		case D_96:
			temp |= FD5INCHHIGH;
			break;
		case D_135H:
			temp |= FD3INCHHIGH;
			break;
		case D_1354H:
			temp |= FD3INCHHIGH4M;
			break;
		default:
			break;
		} /* end of switch on drive_type */
		fdadapter->fderrptr->data.status2 = temp;
		fdadapter->fderrptr->data.status3 = fdp->diskette_type;
		fdadapter->fderrptr->data.command0 = 
		    fdadapter->command.un1.cmds.command1;
		fdadapter->fderrptr->data.command1 =
		    fdadapter->command.un1.cmds.command2;
		fdadapter->error_value = 0;
		fdadapter->fderrptr->data.mainstat = fdread_main_status();
		if (fdadapter->error_value != 0)
			fdadapter->fderrptr->data.mainstat = 0;
		fdadapter->error_value = 0;
		fdadapter->fderrptr->data.dsktchng = fdread_change_line();
		if (fdadapter->error_value != 0)
			fdadapter->fderrptr->data.dsktchng = 0;
		fdadapter->fderrptr->data.result0 =
		    fdadapter->results.un1.result_array[0];
		fdadapter->fderrptr->data.result1 =
		    fdadapter->results.un1.result_array[1];
		fdadapter->fderrptr->data.result2 =
		    fdadapter->results.un1.result_array[2];
		fdadapter->fderrptr->data.cylinder_num =
		   fdadapter->results.un1.result_array[3];
		fdadapter->fderrptr->data.head_num =
		    fdadapter->results.un1.result_array[4];
		fdadapter->fderrptr->data.sector_num =
 		    fdadapter->results.un1.result_array[5];
		fdadapter->fderrptr->data.bytes_num =
		    fdadapter->results.un1.result_array[6];
		fdadapter->fderrptr->data.head_settle_time =
		    fdp->head_settle_time;
		fdadapter->fderrptr->data.motor_speed = fdp->motor_speed;
		fdadapter->fderrptr->data.Mbytes_read = fdp->rcount_megabytes;
		fdadapter->fderrptr->data.Mbytes_written =
		    fdp->wcount_megabytes;
		fdadapter->fderrptr->data.result3 = 0xff;
		switch (value) {

		/*
		 * diskette overrun condition, hardware error
		 */

		case 0x10:
			fdadapter->fderrptr->header.error_id =
			    ERRID_DISKETTE_ERR1;
			break;

		/*
		 * diskette timeout condition, hardware error
		 */

		case 0xef:
			fdadapter->fderrptr->header.error_id =
			    ERRID_DISKETTE_ERR2;
			break;

		/*
		 * spurious diskette interrupt, unknown error
		 */

		case 0xff:
			fdadapter->fderrptr->header.error_id =
			     ERRID_DISKETTE_ERR4;
			break;

		/* 
		 * Otherwise, probably a media error
		 */

		default:
			fdadapter->fderrptr->header.error_id =
			    ERRID_DISKETTE_ERR3;
			break;
		} /* end of switch on value */

		/*
		 * log the error
		 */

		errsave((char *)fdadapter->fderrptr, 
		    sizeof(struct fd_err_rec ));
	} /* end of fdp != NULL processing */
	FD_TRACE1("out log", FDP_DEV);
} /* end of fdlog_error() */


/*
 * NAME: fdnull
 *
 * FUNCTION: Null function needed by uphysio.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the bottom half of the device driver.  It
 *  can be called from the process or the interrupt level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS:
 *  bp       - a pointer to the buffer header structure.
 *  minparms - the minimum number of buf structs needed to keep the
 *               diskette controller busy.
 *
 * RETURN VALUE DESCRIPTION:
 *  FDSUCCESS - required 0 return value.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

int	fdnull (struct buf *bp, void *minparms)
{
	return(FDSUCCESS);
}



/*
 * NAME: fdsettle_timer_handler
 *
 * FUNCTION: Handles settle timer events.
 *  This routine is called whenever a settle timer pops.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only from the process level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS:
 *  settle_timer - a pointer to the timer structure.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED: curtime
 */

void fdsettle_timer_handler (struct trb *settle_timer)
{
	register struct floppy *fdp;
	uint old_pri;

	fdp = fdadapter->drive_list[fdadapter->active_drive];
	FD_TRACE1("in stimer", FDP_DEV);

	old_pri= disable_lock(fdadapter->int_class, &fdadapter->intr_lock); 

	fdadapter->sub_state = FD_SETTLE_SEEK;
	fdp->cylinder_id = 1;
	fdadapter->error_value = 0;
	fdseek();
	if (fdadapter->error_value) {
		fdadapter->state = FD_NO_STATE;
		fdadapter->sub_state = NULL;
		e_wakeup(&(fdadapter->sleep_anchor));
	}

	unlock_enable(old_pri, &fdadapter->intr_lock);

	FD_TRACE1("out stimer", FDP_DEV);

} /* end of fdsettle_timer_handler() */




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
	int 	rc, old_level,saved_state,saved_sub_state;


	FD_TRACE1("in watch", FDA_DEV);
	fdp = fdadapter->drive_list[fdadapter->active_drive];

	old_level = disable_lock(fdadapter->int_class, &fdadapter->intr_lock);

	saved_state=fdadapter->state;
	saved_sub_state=fdadapter->sub_state;
	fdlog_error(fdp, 0xef);
	fdadapter->reset_needed = TRUE;
	fdadapter->error_value = ENOTREADY;
	fdadapter->state = FD_NO_STATE;
	fdadapter->sub_state = NULL;

	switch (saved_state) {
	case FD_RW:
		FD_TRACE1("RW", FDA_DEV);
		rc = D_UNMAP_SLAVE (fdadapter->dma_id);
		FD_TRACE1("call d_comp", rc);
		fdrw_exit(fdp, ENOTREADY);
		break;

	case FD_SPEED:
		FD_TRACE1("SPEED", FDA_DEV);
		rc = D_UNMAP_SLAVE (fdadapter->dma_id);
		FD_TRACE1("call d_comp", rc);
		break;

	case FD_SETTLE:
		FD_TRACE1("SETTLE", FDA_DEV);
		switch (saved_sub_state) {
		FD_TRACE1("SETREAD1&2", FDA_DEV);
		case FD_SETTLE_READ1:
		case FD_SETTLE_READ2:
			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);
			break;

		case FD_SETTLE_WRITE:
			FD_TRACE1("SETWRITE", FDA_DEV);
			rc = D_UNMAP_SLAVE (fdadapter->dma_id);
			FD_TRACE1("call d_comp", rc);
			break;
		}
		break;

	case FD_FORMAT:
		FD_TRACE1("FORMAT", FDA_DEV);
		rc = D_UNMAP_SLAVE (fdadapter->dma_id);
		FD_TRACE1("call d_comp", rc);
		break;

	case FD_IO_RESET:
		fdio_exit(fdp, ENOTREADY);
		break;

	case FD_TYPE1_WAKEUP:
	case FD_TYPE2_WAKEUP:
	case FD_INITIAL_INTERRUPT:
		FD_TRACE1("OTHERS", FDA_DEV);
		break;
	default:
		ASSERT(fdadapter->state != fdadapter->state);
		break;
	}
	e_wakeup(&(fdadapter->sleep_anchor));

	unlock_enable(old_level, &fdadapter->intr_lock);

	FD_TRACE1("out watch", FDP_DEV);

} /* end of fdwatchdog() */


void fdmotor_timer (struct watchdog *timer)
{

	uint old_pri;
	register struct floppy *fdp;
	uchar drive_number;

	FD_TRACE1("in mtimer", 0xffff);
	drive_number = fdadapter->active_drive;
        fdp = fdadapter->drive_list[drive_number];
	old_pri= disable_lock(fdadapter->int_class, &fdadapter->intr_lock);
	fdenable_controller();
	fdadapter->motor_on = FDNO_DRIVE;
	unlock_enable(old_pri, &fdadapter->intr_lock);
	
	FD_TRACE1("out mtimer", 0xffff);

} /* end of fdmotor_timer() */



int	fdrw_exit (register struct floppy *fdp, int errno_value)
{
	struct buf *bp;
	int     rc;
	
	FD_TRACE1("in rw_exit", FDP_DEV);
	if (fdadapter->d_comp_needed) {
		fdadapter->d_comp_needed = FALSE;
		rc = D_UNMAP_SLAVE (fdadapter->dma_id);
	}
	fdadapter->state = FD_NO_STATE;
	fdadapter->sub_state = NULL;
	bp = fdp->headptr;
	fdp->headptr = fdp->headptr->av_forw;
	bp->b_flags |= B_ERROR;
	bp->b_error = errno_value;
	bp->b_resid = bp->b_bcount - fdp->total_bcount;
	FD_TRACE1("call iodone", (dev_t)(bp->b_error));
	FD_TRACE2("bp:", (ulong)bp->b_flags, (caddr_t)bp, (ulong)bp->b_bcount);
	iodone(bp);
	ASSERT((minor(fdp->device_number) >> 6) == fdadapter->active_drive);
	fdunlock_adapter(fdadapter->active_drive);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC, fdp->device_number);
	FD_TRACE1("out rw_exit", FDP_DEV);
	return(INTR_SUCC);
}


int	fdio_exit (register struct floppy *fdp, int errno_value)
{
	struct buf *bp;
	
	FD_TRACE1("in fd_exit", FDP_DEV);
	fdadapter->state = FD_NO_STATE;
	fdadapter->sub_state = NULL;
	bp = fdp->headptr;
	fdadapter->reset_needed = TRUE;
	bp->b_flags |= B_ERROR;
	bp->b_error = errno_value;
	bp->b_resid = bp->b_bcount;
	fdp->headptr = fdp->headptr->av_forw;
	FD_TRACE1("call iodone", (dev_t)(bp->b_error));
	FD_TRACE2("bp:", (ulong)bp->b_flags, (caddr_t)bp, (ulong)bp->b_bcount);
	iodone(bp);
	fdunlock_adapter(fdadapter->active_drive);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_INTR, INTR_SUCC, fdp->device_number);
	FD_TRACE1("out fd_exit", FDP_DEV);
	return(INTR_SUCC);
}


int	fdcntl_reg_pio (caddr_t piop)
{
	FD_TRACE1("in ctl_pio", FDA_DEV);
	if (((struct pio_parms *)(piop))->read == TRUE) {
		((struct pio_parms *)(piop))->data =
		    FDREAD((ulong)((struct pio_parms *)(piop))->bus_val +
		    ((struct pio_parms *)(piop))->reg);
		FD_TRACE2("R:", ((struct pio_parms *)piop)->data,
		    ((struct pio_parms *)piop)->bus_val +
		    ((struct pio_parms *)piop)->reg, 0);
	} else
	 {
		FD_TRACE2("W:", ((struct pio_parms *)piop)->data,
		    ((struct pio_parms *)piop)->bus_val +
		    ((struct pio_parms *)piop)->reg, 0);
		FDWRITE((ulong)((struct pio_parms *)(piop))->bus_val +
	 	   ((struct pio_parms *)(piop))->reg, 
		   ((struct pio_parms *)(piop))->data);
	}
	FD_TRACE1("out ctl_pio", FDA_DEV);
	return(FDSUCCESS);
}



int	fdcntl_reg_pio_recovery (caddr_t piop, int action)
{
	FD_TRACE1("in ctl_prc", FDA_DEV);
	if (action == PIO_NO_RETRY) {
		fdadapter->reset_needed = TRUE;
		fdlog_error(NULL, 2); /* log permanent PIO error */
	} else
	 {
		fdlog_error(NULL, 1); /* log temporary PIO error */
		if (((struct pio_parms *)(piop))->read == TRUE) {
			FDREAD((ulong)((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg);
			FD_TRACE2("RPR:", ((struct pio_parms *)piop)->data,
			    ((struct pio_parms *)piop)->bus_val +
			    ((struct pio_parms *)piop)->reg, 0);
		} else
		 {
			FD_TRACE2("WPR:", ((struct pio_parms *)piop)->data,
			    ((struct pio_parms *)piop)->bus_val +
			    ((struct pio_parms *)piop)->reg, 0);
			FDWRITE((ulong)((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg,
			    ((struct pio_parms *)(piop))->data);
		}
	}
	FD_TRACE1("out ctl_prc", FDA_DEV);
	return(FDSUCCESS);
}



void	fdset_data_rate ()
{
	register struct	floppy *fdp;
	uchar	drive_number;
	struct	pio_parms piop;

	FD_TRACE1("in d_rate", FDA_DEV);
	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
	fdp = fdadapter->drive_list[drive_number];
	piop.data = fdp->data_rate;
	piop.reg = FDDATA_RATE_REG;
	piop.read = FALSE;
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0)
		fdadapter->data_rate = fdp->data_rate;

	fdadapter->data_rate = fdp->data_rate;
	FD_TRACE1("out d_rate", FDA_DEV);
} /* end of fdset_data_rate() */




void	fdreset ()
{
	FD_TRACE1("in reset", FDA_DEV);
	fdadapter->error_value = 0;
	fdset_data_rate();
	if (fdadapter->error_value) {
		FD_TRACE1("out reset", FDA_DEV);
		return;
	}
	w_start(&(fdadapter->inttimer));
	fdsoft_reset();
	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
	} else {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	FD_TRACE1("out reset", FDA_DEV);
}



void	fdsoft_reset ()
{
	uchar	drive_number, mask;
	struct	pio_parms piop;
#ifdef _POWER
	struct	timestruc_t start_time, end_time;
	ulong	elapsed_time = 0;
#endif
	FD_TRACE1("in s_reset", FDA_DEV);
	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
	if (fdadapter->motor_on == FDNO_DRIVE) {
		mask = FDRESETMASK;
	} else
	 {
		mask = FDDRIVE_0_MASK;
		mask = mask << drive_number;
		mask |= drive_number;
		mask &= 0xfb;
	}
	piop.reg = FDDRIVE_CONTROL_REG;
	piop.read = FALSE;
	piop.data = mask;
	w_start(&(fdadapter->inttimer));

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);

	if (fdadapter->error_value != 0) {
	       w_stop(&(fdadapter->inttimer));
	} else
	 {
#ifdef _POWER
		curtime(&start_time);
		while (elapsed_time < 4000) { /* 4000 nanoseconds for R2 */
			curtime(&end_time);
			if (start_time.tv_sec == end_time.tv_sec) {
				elapsed_time = end_time.tv_nsec - 
				    start_time.tv_nsec;
			} else
		 	 {
				elapsed_time = NS_PER_SEC - start_time.tv_nsec;
				elapsed_time += (end_time.tv_nsec + 
				    (end_time.tv_sec - start_time.tv_sec - 1)
				    * NS_PER_SEC);
			}
		}
#endif
		if (fdadapter->motor_on == FDNO_DRIVE) {
			mask = FDCONTROLMASK | FDNODRIVE_SELECT;
		} else
		 {
			mask = FDDRIVE_0_MASK;
			mask = mask << drive_number;
			mask |= drive_number;
			mask |= FDCONTROLMASK;
		}
		piop.data = mask;

		fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);

		if (fdadapter->error_value != 0) {
			w_stop(&(fdadapter->inttimer));
		} else
		 {
			fdadapter->reset_needed = FALSE;
			fdadapter->reset_performed = TRUE;
			fdadapter->reset_active = TRUE;
			if (fdadapter->motor_on != FDNO_DRIVE)  {
				w_stop(&(fdadapter->mottimer));
				w_start(&(fdadapter->mottimer));
			}
		}
	}
	FD_BUSIO_DET(piop.bus_val);
	FD_TRACE1("out s_reset", FDA_DEV);
}

void fdtop_select_drive ()
{

	uchar   drive_number;
	register struct floppy *fdp;
	uint old_pri;

	FD_TRACE1("in top_sel", FDA_DEV);

	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
	fdp = fdadapter->drive_list[drive_number];
	old_pri= disable_lock(fdadapter->int_class, &fdadapter->intr_lock);
	fdselect_drive ();
	unlock_enable(old_pri, &fdadapter->intr_lock);

	FD_TRACE1("out top_sel", FDA_DEV);

} /* end fdtop_select_drive */


void	fdselect_drive ()
{
	uchar	drive_number, mask;
	struct	pio_parms piop;

	FD_TRACE1("in drv_sel", FDA_DEV);
	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
	mask = FDDRIVE_0_MASK;
	mask = mask << drive_number;
	mask |= drive_number;
	mask |= FDCONTROLMASK;
	piop.reg = FDDRIVE_CONTROL_REG;
	piop.read = FALSE;
	piop.data = mask;
	w_start(&(fdadapter->mottimer));
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->mottimer));
	} else
	 {
		fdadapter->motor_on = drive_number;
	}
	FD_TRACE1("out drv_sel", FDA_DEV);
}



void	fdtop_enable_controller ()
{

	uchar   drive_number;
	register struct floppy *fdp;
	uint old_pri;

	FD_TRACE1("in top_enable ", FDA_DEV);

	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
	fdp = fdadapter->drive_list[drive_number];	
        old_pri= disable_lock(fdadapter->int_class, &fdadapter->intr_lock);
	fdenable_controller ();
	unlock_enable(old_pri, &fdadapter->intr_lock);

	FD_TRACE1("out top_enable", FDA_DEV);

} /* end fdtop_enable_controller */

void	fdenable_controller ()
{
	struct	pio_parms piop;

	FD_TRACE1("in enable", FDA_DEV);
	piop.reg = FDDRIVE_CONTROL_REG;
	piop.read = FALSE;
	piop.data = FDCONTROLMASK | FDNODRIVE_SELECT;
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		w_stop(&(fdadapter->mottimer));
		fdadapter->motor_on = FDNO_DRIVE;
	}
	FD_TRACE1("out enable", FDA_DEV);
}


void	fddisable_controller ()
{
	struct	pio_parms piop;

	FD_TRACE1("in disable", FDA_DEV);
	piop.reg = FDDRIVE_CONTROL_REG;
	piop.read = FALSE;
	piop.data = FDDISABLE2 | FDNODRIVE_SELECT; 
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fdcntl_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		w_stop(&(fdadapter->mottimer));
		fdadapter->motor_on = FDNO_DRIVE;
	}
	FD_TRACE1("out disable", FDA_DEV);
}


#ifdef _POWER
int	fddata_reg_pio (caddr_t piop)
{
	volatile int i, done_flag;
	uchar	drive_number, log_flag;
	volatile uchar value;
	register struct floppy *fdp;
	struct	timestruc_t start_time, end_time;
	ulong	elapsed_time;

	FD_TRACE1("in datapio", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	for (i = 0; i < fdadapter->command.total_bytes; i++) {
		done_flag = FALSE;
		elapsed_time = 0;
		log_flag = 1;
		curtime(&start_time);
		while (done_flag == FALSE) {
			value = FDREAD((ulong)(((struct pio_parms *)
			    (piop))->bus_val) + FDMAIN_STATUS_REG);
			if ((value & FDWRITE_MASK) == FDWRITE_MASK) {
				FD_TRACE2("W:",
				    fdadapter->command.un1.command_array[i],
				    ((struct pio_parms *)(piop))->bus_val +
				    FDDISKETTE_DATA_REG, elapsed_time);
				FDWRITE((ulong)(((struct pio_parms *)
				    (piop))->bus_val) + FDDISKETTE_DATA_REG, 
				    fdadapter->command.un1.command_array[i]);
				done_flag = TRUE;
			} else
			 {
				curtime(&end_time);
				if (start_time.tv_sec == end_time.tv_sec) {
					elapsed_time = end_time.tv_nsec -
		    			    start_time.tv_nsec;
				} else
	 			 {
					elapsed_time = NS_PER_SEC -
					    start_time.tv_nsec;
					elapsed_time += (end_time.tv_nsec +
		    			    (end_time.tv_sec -
					    start_time.tv_sec - 1) *
					    NS_PER_SEC);
				}
				if (log_flag) {
					if (elapsed_time > 50 * NS_PER_uS) {
						log_flag = 0;
						fdadapter->data_reg_excess++;
					}
				}
				if (elapsed_time > FDMAX_LOOP_TIME) {
					value = FDREAD((ulong)
					    (((struct pio_parms *)
					    (piop))->bus_val) +
					    FDMAIN_STATUS_REG);
					if ((value & FDWRITE_MASK) ==
					    FDWRITE_MASK) {
						FD_TRACE2("W:",
						    fdadapter->command.un1.
						    command_array[i],
						    ((struct pio_parms *)
						    (piop))->bus_val +
						    FDDISKETTE_DATA_REG,
						    elapsed_time);
						FDWRITE((ulong)
						    (((struct pio_parms *)
						    (piop))->bus_val) +
						    FDDISKETTE_DATA_REG, 
						    fdadapter->command.un1.
						    command_array[i]);
						done_flag = TRUE;
					} else
			 		 {
						FD_TRACE1("TIMEOUT!!!!",
						    FDP_DEV);
						ASSERT(!(value & 0x40));
						fdadapter->reset_needed = TRUE;
						fdlog_error(fdp, 0xef);
						FD_TRACE1("out datapio",
						    FDA_DEV);
						return(ENOTREADY);
					}
				}
			}
		}
	}
	for (i = 0; i < fdadapter->results.total_bytes; i++) {
		done_flag = FALSE;
		elapsed_time = 0;
		curtime(&start_time);
		while (done_flag == FALSE) {
			value = FDREAD((ulong)(((struct pio_parms *)
			    (piop))->bus_val) + FDMAIN_STATUS_REG);
			if ((value & FDREAD_MASK) == FDREAD_MASK) {
				fdadapter->results.un1.result_array[i] = 
				    FDREAD((ulong)(((struct pio_parms *)
				    (piop))->bus_val) + FDDISKETTE_DATA_REG);
				FD_TRACE2("R:",
				    fdadapter->results.un1.result_array[i],
				    ((struct pio_parms *)(piop))->bus_val +
				    FDDISKETTE_DATA_REG, elapsed_time);
				done_flag = TRUE;
			} else
			 {
				curtime(&end_time);
				if (start_time.tv_sec == end_time.tv_sec) {
					elapsed_time = end_time.tv_nsec -
		    			    start_time.tv_nsec;
				} else
	 			 {
					elapsed_time = NS_PER_SEC -
					    start_time.tv_nsec;
					elapsed_time += (end_time.tv_nsec +
		    			    (end_time.tv_sec -
					    start_time.tv_sec - 1) *
					    NS_PER_SEC);
				}
				if (elapsed_time > FDMAX_LOOP_TIME) {
					value = FDREAD((ulong)
					    (((struct pio_parms *)
					    (piop))->bus_val) +
					    FDMAIN_STATUS_REG);
					if ((value & FDREAD_MASK) ==
					    FDREAD_MASK) {
						fdadapter->results.un1.
						    result_array[i] = 
						    FDREAD((ulong)
						    (((struct pio_parms *)
						    (piop))->bus_val) +
						    FDDISKETTE_DATA_REG);
						FD_TRACE2("R:",
						    fdadapter->results.un1.
						    result_array[i],
						    ((struct pio_parms *)
						    (piop))->bus_val +
						    FDDISKETTE_DATA_REG,
						    elapsed_time);
						done_flag = TRUE;
					} else
			 		 {
						FD_TRACE1("TIMEOUT!!!!",
						    FDP_DEV);
						ASSERT(value & 0x40);
						fdadapter->reset_needed = TRUE;
						fdlog_error(fdp, 0xef);
						FD_TRACE1("out datapio",
						    FDA_DEV);
						return(ENOTREADY);
					}
				}
			}
		}
	}
	FD_TRACE1("out datapio", FDA_DEV);
	return(FDSUCCESS);
} /* end of fddata_reg_pio() */
#endif


int	fddata_reg_pio_recovery (caddr_t piop, int action)
{
	FD_TRACE1("in dpiorec", FDA_DEV);
	fdadapter->reset_needed = TRUE;
	fdlog_error(NULL, 2);
	FD_TRACE1("out dpiorec", FDA_DEV);
	return(EIO);
} /* end of fddata_reg_pio_recovery() */



void    fdsense_interrupt ()
{
	struct  pio_parms piop;

	FD_TRACE1("in sen_int", FDA_DEV);
	fdadapter->command.total_bytes = 1;
	fdadapter->results.total_bytes = 2;
	fdadapter->command.un1.cmds.command1 = FDSENSE_INTERRUPT;

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	FD_TRACE1("out sen_int", FDA_DEV);
} /* end of fdsense_interrupt() */



void	fdsense_drive_status ()
{
	uchar	drive_number;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in sendrst", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->command.total_bytes = 2;
	fdadapter->results.total_bytes = 1;
	fdadapter->command.un1.cmds.command1 = FDSENSE_DRIVE_STATUS;
	fdadapter->command.un1.cmds.command2 = (fdp->current.head << 2) |
	    drive_number;
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	FD_TRACE1("out sendrst", FDA_DEV);
} /* end of fdsense_drive_status() */



void    fdspecify ()
{
	uchar   drive_number;
	register struct floppy *fdp;
	struct  pio_parms piop;

	FD_TRACE1("in specify", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->command.total_bytes = 3;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDSPECIFY;
	fdadapter->command.un1.cmds.command2 = fdp->head_unload |
	    (fdp->step_rate << 4);
	fdadapter->command.un1.cmds.byte2.motor_time = FDDMA |
	    (fdp->head_load << 1);
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	FD_TRACE1("out specify", FDA_DEV);
} /* end of fdspecify() */



void    fdset_perp ()
{
	uchar   drive_number;
	register struct floppy *fdp;
	struct  pio_parms piop;

	FD_TRACE1("in perpend", FDA_DEV);
	/* issues the perpedicular command to setup drive */
	/* perpendicular mode for the drive.             */
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->command.total_bytes = 2;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDPERPENDICULAR;
	fdadapter->command.un1.cmds.command2 = 0x80 |
	    (0x04 << drive_number);
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
					fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	FD_TRACE1("out perpend", FDA_DEV);
} /* end of fdset_perp() */



void    fdset_config ()
{
	uchar	drive_number;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in setcfg", FDA_DEV);
	/* issues the config commands to setup drive */
	/* configuration.                            */
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->command.total_bytes = 4;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDCONFIG;
	fdadapter->command.un1.cmds.command2 = 0x00;
	fdadapter->command.un1.cmds.byte2.config_set = FDCONFIGSET;
	fdadapter->command.un1.cmds.byte3.config_set = 0x00;
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, 
					fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value == 0) {
		if (fdadapter->motor_on != FDNO_DRIVE)  {
			w_stop(&(fdadapter->mottimer));
			w_start(&(fdadapter->mottimer));
		}
	}
	FD_TRACE1("out setcfg", FDA_DEV);
} /* end of fdset_config() */



void	fdseek ()
{
	uchar	drive_number;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in seek", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->command.total_bytes = 3;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDSEEK;
	fdadapter->command.un1.cmds.command2 = drive_number;
	fdadapter->command.un1.cmds.byte2.track = fdp->cylinder_id *
	    fdp->step_size;
	w_start(&(fdadapter->inttimer));
	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
	} else if (fdadapter->motor_on != FDNO_DRIVE)  {
		w_stop(&(fdadapter->mottimer));
		w_start(&(fdadapter->mottimer));
	}
	FD_TRACE1("out seek", FDA_DEV);
} /* end of fdseek() */



void	fdrecalibrate ()
{
	uchar	drive_number;
	struct	pio_parms piop;

	FD_TRACE1("in recal", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdadapter->command.total_bytes = 2;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDRECALIBRATE;
	fdadapter->command.un1.cmds.command2 = drive_number;
	w_start(&(fdadapter->inttimer));

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
		fdadapter->d_comp_needed = TRUE;
	} else if (fdadapter->motor_on != FDNO_DRIVE)  {
		w_stop(&(fdadapter->mottimer));
		w_start(&(fdadapter->mottimer));
	}
	FD_TRACE1("out recal", FDA_DEV);
} /* end of fdrecalibrate() */



void	fdreadid ()
{
	uchar	drive_number;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in readid", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->command.total_bytes = 2;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDREAD_ID | FDMFM;
	fdadapter->command.un1.cmds.command2 = (fdp->head_id << 2) |
	    drive_number;
	w_start(&(fdadapter->inttimer));

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
		fdadapter->d_comp_needed = TRUE;
	} else if (fdadapter->motor_on != FDNO_DRIVE)  {
		w_stop(&(fdadapter->mottimer));
		w_start(&(fdadapter->mottimer));
	}
	FD_TRACE1("out readid", FDA_DEV);
} /* end of fdreadid() */


void	fdrw ()
{
	uchar	drive_number, command;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in r/w", FDA_DEV);
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

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
		fdadapter->d_comp_needed = TRUE;
	} else if (fdadapter->motor_on != FDNO_DRIVE)  {
		w_stop(&(fdadapter->mottimer));
		w_start(&(fdadapter->mottimer));
	}
	FD_TRACE1("out r/w", FDA_DEV);
} /* end of fdrw() */


void	fdrw_ioctl ()
{
	uchar	drive_number, command;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in rwioctl", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->d_comp_needed = FALSE;
	if (fdadapter->sub_state == FD_SETTLE_WRITE)
		command = FDWRITE_DATA;
	else
		command = FDREAD_DATA;
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

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
		fdadapter->d_comp_needed = TRUE;
	} else if (fdadapter->motor_on != FDNO_DRIVE)  {
		w_stop(&(fdadapter->mottimer));
		w_start(&(fdadapter->mottimer));
	}
	FD_TRACE1("out rwioctl", FDA_DEV);
} /* end of fdrw_ioctl() */



void	fdformat_track ()
{
	uchar	drive_number;
	register struct floppy *fdp;
	struct	pio_parms piop;

	FD_TRACE1("in format", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	fdadapter->d_comp_needed = FALSE;
	fdadapter->command.total_bytes = 6;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDFORMAT_TRACK | FDMFM;
	fdadapter->command.un1.cmds.command2 = (fdp->current.head << 2) |
	    drive_number;
	fdadapter->command.un1.cmds.byte2.sector_size = fdp->sector_size;
	fdadapter->command.un1.cmds.byte3.sectors_per_track = 
	    fdp->sectors_per_track;
	fdadapter->command.un1.cmds.byte4.gap_length = fdp->format_gap;
	fdadapter->command.un1.cmds.byte5.data_pattern = fdp->fill_byte;
	w_start(&(fdadapter->inttimer));

	piop.bus_val = FD_BUSIO_ATT(fdadapter->iom, fdadapter->io_address);
	fdadapter->error_value = fddata_reg_pio((caddr_t) &piop);
	FD_BUSIO_DET(piop.bus_val);

	if (fdadapter->error_value != 0) {
		w_stop(&(fdadapter->inttimer));
		fdadapter->d_comp_needed = TRUE;
	} else if (fdadapter->motor_on != FDNO_DRIVE)  {
		w_stop(&(fdadapter->mottimer));
		w_start(&(fdadapter->mottimer));
	}
	FD_TRACE1("out format", FDA_DEV);
} /* end of fdformat_track() */


/* process level only */
int	fdlock_adapter (int drive_number)
{
	int	old_level, rc;
        register struct floppy *fdp;
	
	FD_TRACE1("in lock", FDA_DEV);

	fdp = fdadapter->drive_list[drive_number];

	/* lock the adapter to serialize top half I/O */
	old_level = disable_lock(fdadapter->int_class, &fdadapter->intr_lock);

	/* check if the I/O should be blocked due to PM mode */
	fdcheck_pm_mode (fdp);

	while (fdadapter->adapter_busy == TRUE) {
		rc = e_sleep_thread(&(fdadapter->adapter_sleep_anchor), 
                       &fdadapter->intr_lock, LOCK_HANDLER|INTERRUPTIBLE);
		if ( rc == THREAD_INTERRUPTED) {
			unlock_enable(old_level, &fdadapter->intr_lock);
			FD_TRACE1("out lock", FDA_DEV);
			return(EINTR);
		}
	}
	fdadapter->adapter_busy = TRUE;
	fdadapter->active_drive = drive_number;
	unlock_enable(old_level, &fdadapter->intr_lock);

	FD_TRACE1("out lock", FDA_DEV);
	return(FDSUCCESS);
}

void    fdtop_unlock_adapter (int drive_number)
{
	uint old_pri;
	register struct floppy *fdp;
	
	FD_TRACE1("in  top_unlock", FDA_DEV);

	fdp = fdadapter->drive_list[drive_number];
	old_pri = disable_lock(fdadapter->int_class, &fdadapter->intr_lock); 
	fdunlock_adapter(drive_number);
	unlock_enable(old_pri, &fdadapter->intr_lock);

	FD_TRACE1("out top_unlock", FDA_DEV);
} /* end fdtop_unlock_adapter */

void	fdunlock_adapter (int drive_number)
{
	int	i;
	register struct floppy *fdp;
	
	FD_TRACE1("in unlock", FDA_DEV);
	fdadapter->adapter_busy = FALSE;
	ASSERT(fdadapter->active_drive == drive_number);
	for (i = 0; i < FDMAXDRIVES; i++) {
		if (i != drive_number) {
			fdp = fdadapter->drive_list[i];
			if (fdp != NULL) {
				if (fdp->headptr != NULL) {
					fdio(fdp->headptr);
					FD_TRACE1("out unlock", FDA_DEV);
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
	FD_TRACE1("out unlock", FDA_DEV);
}
		

void	fdtimeout (register struct trb *fdstart_timer_ptr)
{
	uchar	evalue, drive_number;
	register struct buf *bp;
	register struct floppy *fdp;
	int	old_level;

	FD_TRACE1("in timeout", FDA_DEV);

	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);

	fdp = fdadapter->drive_list[drive_number];
	old_level = disable_lock(fdadapter->int_class, &fdadapter->intr_lock);

	fdadapter->state = FD_RW;
	fdadapter->error_value = 0;
	if (fdadapter->reset_needed == TRUE) {
		fdadapter->sub_state = FD_RW_RESET;
		fdreset();

	} else if (fdadapter->sub_state == FD_RW_RECAL1) {
		fdrecalibrate();
	} else
	 {
		fdadapter->sub_state = FD_RW_SEEK1;
		fdseek();
	}
	evalue = fdadapter->error_value;
	if (evalue != 0) {
		fdadapter->reset_needed = TRUE;
		fdp = (register struct floppy * )fdstart_timer_ptr->func_data;
		bp = (register struct buf * )fdp->headptr;
		bp->b_flags |= B_ERROR;
		bp->b_error = evalue;
		bp->b_resid += fdp->modified_bcount - fdp->total_bcount;
		fdp->headptr = fdp->headptr->av_forw;
		FD_TRACE1("call iodone", (dev_t)(bp->b_error));
		FD_TRACE2("bp:", (ulong)bp->b_flags, (caddr_t)bp,
		    (ulong)bp->b_bcount);
		iodone(bp);
		fdunlock_adapter(drive_number);
	}
	unlock_enable(old_level, &fdadapter->intr_lock);
	FD_TRACE1("out timeout", FDA_DEV);
} /* fdtimeout */


/* process level only, used to execute commands that generate interrupts */
int	fdexecute_int_cmd (void (*func)())
{
	int	old_level, rc;
	uchar drive_number;
	register struct floppy *fdp;

	FD_TRACE1("in xintcmd", FDA_DEV);
	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
        fdp = fdadapter->drive_list[drive_number];
	old_level = disable_lock(fdadapter->int_class, &fdadapter->intr_lock);
	fdadapter->error_value = 0;
	(*func)();
	if (fdadapter->error_value == 0)
		e_sleep_thread(&(fdadapter->sleep_anchor), 
			       &fdadapter->intr_lock, LOCK_HANDLER); 
	rc = fdadapter->error_value;
	unlock_enable(old_level, &fdadapter->intr_lock);

	FD_TRACE1("out xintcmd", FDA_DEV);
	return(rc);
} /* fdexecute_int_cmd */


void	fdqueue_check (register struct floppy *fdp, int drive_number)
{
	int 	old_level;

	FD_TRACE1("in qcheck", FDA_DEV);

	old_level = disable_lock(fdadapter->int_class, &fdadapter->intr_lock);
	while (fdp->headptr != NULL) {
		e_sleep_thread(&(fdadapter->adapter_sleep_anchor), 
			       &fdadapter->intr_lock, LOCK_HANDLER);
	}
	unlock_enable(old_level, &fdadapter->intr_lock);

	FD_TRACE1("out qcheck", FDA_DEV);
} /* fdqueue_check */



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



int	fdcfg_data_pio (caddr_t piop)
{
	volatile int i, done_flag;
	uchar	drive_number;
	volatile uchar value;
	register struct floppy *fdp;
	struct	timestruc_t start_time, end_time;
	ulong	elapsed_time;

	FD_TRACE1("in cfgdpio", FDA_DEV);
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	for (i = 0; i < fdadapter->command.total_bytes; i++) {
		done_flag = FALSE;
		elapsed_time = 0;
		curtime(&start_time);
		while (done_flag == FALSE) {
			value = FDREAD((ulong)(((struct pio_parms *)
			    (piop))->bus_val) + FDMAIN_STATUS_REG);
			if ((value & FDWRITE_MASK) == FDWRITE_MASK) {
				FD_TRACE2("W:",
				    fdadapter->command.un1.command_array[i],
				    ((struct pio_parms *)(piop))->bus_val +
				    FDDISKETTE_DATA_REG, elapsed_time);
				FDWRITE((ulong)(((struct pio_parms *)
				    (piop))->bus_val) + FDDISKETTE_DATA_REG, 
				    fdadapter->command.un1.command_array[i]);
				done_flag = TRUE;
			} else
			 {
				curtime(&end_time);
				if (start_time.tv_sec == end_time.tv_sec) {
					elapsed_time = end_time.tv_nsec -
		    			    start_time.tv_nsec;
				} else
	 			 {
					elapsed_time = NS_PER_SEC -
					    start_time.tv_nsec;
					elapsed_time += (end_time.tv_nsec +
		    			    (end_time.tv_sec -
					    start_time.tv_sec - 1) *
					    NS_PER_SEC);
				}
				if (elapsed_time > 2 * NS_PER_SEC) {
					value = FDREAD((ulong)
					    (((struct pio_parms *)
					    (piop))->bus_val) +
					    FDMAIN_STATUS_REG);
					if ((value & FDWRITE_MASK) ==
					    FDWRITE_MASK) {
						FD_TRACE2("W:",
						    fdadapter->command.un1.
						    command_array[i],
						    ((struct pio_parms *)
						    (piop))->bus_val +
						    FDDISKETTE_DATA_REG,
						    elapsed_time);
						FDWRITE((ulong)
						    (((struct pio_parms *)
						    (piop))->bus_val) +
						    FDDISKETTE_DATA_REG, 
						    fdadapter->command.un1.
						    command_array[i]);
						done_flag = TRUE;
					} else
			 		 {
						FD_TRACE1("TIMEOUT!!!!",
						    FDP_DEV);
						FD_TRACE1("out cfgdpio",
						    FDP_DEV);
						fdadapter->reset_needed = TRUE;
						return(ENOTREADY);
					}
				}
			}
		}
	}
	for (i = 0; i < fdadapter->results.total_bytes; i++) {
		done_flag = FALSE;
		elapsed_time = 0;
		curtime(&start_time);
		while (done_flag == FALSE) {
			value = FDREAD((ulong)(((struct pio_parms *)
			    (piop))->bus_val) + FDMAIN_STATUS_REG);
			if ((value & FDREAD_MASK) == FDREAD_MASK) {
				fdadapter->results.un1.result_array[i] = 
				    FDREAD((ulong)(((struct pio_parms *)
				    (piop))->bus_val) + FDDISKETTE_DATA_REG);
				FD_TRACE2("R:",
				    fdadapter->results.un1.result_array[i],
				    ((struct pio_parms *)(piop))->bus_val +
				    FDDISKETTE_DATA_REG, elapsed_time);
				done_flag = TRUE;
			} else
			 {
				curtime(&end_time);
				if (start_time.tv_sec == end_time.tv_sec) {
					elapsed_time = end_time.tv_nsec -
		    			    start_time.tv_nsec;
				} else
	 			 {
					elapsed_time = NS_PER_SEC -
					    start_time.tv_nsec;
					elapsed_time += (end_time.tv_nsec +
		    			    (end_time.tv_sec -
					    start_time.tv_sec - 1) *
					    NS_PER_SEC);
				}
				if (elapsed_time > 2 * NS_PER_SEC) {
					value = FDREAD((ulong)
					    (((struct pio_parms *)
					    (piop))->bus_val) +
					    FDMAIN_STATUS_REG);
					if ((value & FDREAD_MASK) ==
					    FDREAD_MASK) {
						fdadapter->results.un1.
						    result_array[i] = 
						    FDREAD((ulong)
						    (((struct pio_parms *)
						    (piop))->bus_val) +
						    FDDISKETTE_DATA_REG);
						FD_TRACE2("R:",
						    fdadapter->results.un1.
						    result_array[i],
						    ((struct pio_parms *)
						    (piop))->bus_val +
						    FDDISKETTE_DATA_REG,
						    elapsed_time);
						done_flag = TRUE;
					} else
			 		 {
						FD_TRACE1("TIMEOUT!!!!",
						    FDP_DEV);
						FD_TRACE1("out cfgdpio",
						    FDP_DEV);
						fdadapter->reset_needed = TRUE;
						return(ENOTREADY);
					}
				}
			}
		}
	}
	FD_TRACE1("out cfgdpio", FDA_DEV);
	return(FDSUCCESS);
} /* end of fdcfg_data_pio() */




int	fdcfg_data_pio_recovery (caddr_t piop, int action)
{
	FD_TRACE1("in cfgdrec", FDA_DEV);
	fdadapter->reset_needed = TRUE;
	fdlog_error(NULL, 2);
	FD_TRACE1("out cfgdrec", FDA_DEV);
	return(EIO);
} /* end of fdcfg_data_pio_recovery() */





int	fdcfg_cntl_pio (caddr_t piop)
{
	FD_TRACE1("in cfgcpio", FDA_DEV);
	if (((struct pio_parms *)(piop))->read == TRUE) {
		((struct pio_parms *)(piop))->data =
		    FDREAD((ulong)((struct pio_parms *)(piop))->bus_val +
		    ((struct pio_parms *)(piop))->reg);
		FD_TRACE2("R:", ((struct pio_parms *)(piop))->data,
		    ((struct pio_parms *)(piop))->bus_val +
		    ((struct pio_parms *)(piop))->reg, 0);
	} else
	 {
		FD_TRACE2("W:", ((struct pio_parms *)(piop))->data,
		    ((struct pio_parms *)(piop))->bus_val +
		    ((struct pio_parms *)(piop))->reg, 0);
		FDWRITE((ulong)((struct pio_parms *)(piop))->bus_val + ((struct pio_parms *)(piop))->reg, ((struct pio_parms *)(piop))->data);
	}
	FD_TRACE1("out cfgcpio", FDA_DEV);
	return(FDSUCCESS);
}



int	fdcfg_cntl_pio_recovery (caddr_t piop, int action)
{
	FD_TRACE1("in cfgcprc", FDA_DEV);
	if (action == PIO_NO_RETRY) {
		fdadapter->reset_needed = TRUE;
		fdlog_error(NULL, 2); /* log permanent PIO error */
	} else
	 {
		fdlog_error(NULL, 1); /* log temporary PIO error */
		if (((struct pio_parms *)(piop))->read == TRUE) {
			FDREAD((ulong)((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg);
			FD_TRACE2("RPR:", ((struct pio_parms *)(piop))->data,
			    ((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg, 0);
		} else
		 {
			FD_TRACE2("WPR:", ((struct pio_parms *)(piop))->data,
			    ((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg, 0);
			FDWRITE((ulong)((struct pio_parms *)(piop))->bus_val +
			    ((struct pio_parms *)(piop))->reg,
			    ((struct pio_parms *)(piop))->data);
		}
	}
	FD_TRACE1("out cfgcprc", FDA_DEV);
	return(FDSUCCESS);
}



#ifdef DEBUG
int fd_print=0;

void
fd_trace1(char *string, dev_t devno)
{
	int	old_level;

	if (fdadapter->initialized == TRUE)
		old_level = disable_lock(fdadapter->int_class,
				&fdadapter->trace_table->trace_lock);
	if (fd_print)
		printf("%s\t%x\n",string, devno);
	bzero(fdadapter->trace.un.routines.header, 12);
	strncpy(fdadapter->trace.un.routines.header, string, 12);
	fdadapter->trace.un.routines.devno = devno;
	fdmove_trace();
	if (fdadapter->initialized == TRUE)
		unlock_enable(old_level,&fdadapter->trace_table->trace_lock);


}



void
fd_trace2(char *string, ulong data, caddr_t address, ulong time)
{
	int	old_level;

	if (fdadapter->initialized == TRUE)
		old_level = disable_lock(fdadapter->int_class,
				&fdadapter->trace_table->trace_lock);
	if (fd_print)
		printf("%s\t%x\t%x\t%x\n",string, data, address, time);
	bzero(fdadapter->trace.un.routines.header, 4);
	strncpy(fdadapter->trace.un.routines.header, string, 4);
	fdadapter->trace.un.names.data = data;
	fdadapter->trace.un.names.address = address;
	fdadapter->trace.un.names.time = time;
	fdmove_trace();
	if (fdadapter->initialized == TRUE)
		unlock_enable(old_level,&fdadapter->trace_table->trace_lock);

}



void
fdmove_trace()
{
	struct fdtrace_entry	*dest;
	int			n;

	n = fdadapter->trace_table->current;
	dest = &fdadapter->trace_table->trace_buffer[n];
	bcopy(&fdadapter->trace, dest, sizeof(fdadapter->trace));
	if (n < FDTRACE_SIZE - 1) {
		dest++;
		fdadapter->trace_table->current++;
	}
	else {
		dest = fdadapter->trace_table->trace_buffer;
		fdadapter->trace_table->current = 0;
	}
	strcpy(dest, "***FDPOINTER***");
}
#endif /* DEBUG */
