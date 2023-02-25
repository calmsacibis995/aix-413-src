static char sccsid[] = "@(#)65	1.1  src/bos/usr/samples/fd/fd.c, fdsamp, bos411, 9428A410j 10/12/93 14:35:44";
/*
 *   COMPONENT_NAME: FDSAMP (Diskette Device Driver)
 *
 *   FUNCTIONS: fd_config, fd_open, fd_close, fd_read, fd_write,
 *              fd_ioctl, fdopen_exit
 *
 *   INTERNAL FUNCTIONS: fdcleanup, fdio, fdload_floppy, fdnull
 *
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

/* All globals start with 'fd' */

uchar fdglobal_init = FALSE;	/* allocate and initialize the global
						initialization flag */

/*
 * allocate and initialize the lock word.
 */

lock_t fd_lock = LOCK_AVAIL;

extern	struct adapter_structure *fdadapter;

extern	struct fdconfig_parameters fdconfparms[FDMAXDRIVES];




/*
 * NAME: fd_config
 *
 * FUNCTION: fd_config() initializes the diskette device driver.
 *  It will take data from a structure of type 'fd_config' pointed
 *  to by the uio structure and store it in a local structure
 *  of type 'floppy'.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * INPUTS:
 *  devno - the major and minor device numbers.
 *  cmd   - CFG_INIT if device is being defined.
 *        - CFG_TERM if device is being deleted.
 *  uiop  - pointer to uio structure containing fd_config data.
 *
 * OUTPUTS: none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  uiomove, lockl, unlockl
 *
 * ERROR CODES: The following errno values may be returned:
 *  EBUSY   - drive is not in the closed state.
 *  ENXIO   - invalid minor number.
 *  ENINVAL - invalid cmd argument.
 */

int	fd_config (dev_t devno, int cmd, register struct uio *uiop)
{
	struct	fd_config configstruct;
	struct	fd_vpd	vpdstruct;
	struct	uio	localuio;
	struct	iovec	*localuio_iovec;
	register struct fd_config *configptr;
	register struct fd_vpd *vpdptr;
	register struct floppy *fdp;
	struct	devsw fddevsw;
	int	drive_number, i, rc = FDSUCCESS, trc;
	caddr_t	iocc_val;
	uchar	temp_val, delete_adapter;

	lockl(&fd_lock, LOCK_SHORT);

	/*
	 * Use minor device number to determine which drive to use.
	 */

	drive_number = minor(devno) >> 6;
	if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
		unlockl(&fd_lock);
		return(ENXIO);
	}

	/*
	 * The drive number is valid.
	 */

	switch (cmd) {
	case CFG_TERM:

		/*
		 * If the adapter structure has never been initialized, then
		 * there is nothing to delete.
		 */

		if (fdglobal_init == FALSE) {
			unlockl(&fd_lock);
			return(ENXIO);
		}

		/*
		 * If the floppy structure has never been allocated for this
		 * drive, then there is nothing to delete.
		 */

		fdp = fdadapter->drive_list[drive_number];
		if (fdp == NULL) {
			unlockl(&fd_lock);
			return(ENXIO);
		}

		/*     
		 * If drive is not closed, return with error.
		 */

		if (fdp->drive_state != FDCLOSED) {
			unlockl(&fd_lock);
			return(EBUSY);
		}

		/*     
		 * Otherwise delete the drive.
		 */

		rc = free((caddr_t)fdp);
		fdadapter->drive_list[drive_number] = NULL;
		delete_adapter = TRUE;
		for (i = 0; i < FDMAXDRIVES; i++) {
			if (fdadapter->drive_list[i] != NULL) {
				delete_adapter = FALSE;
				break;
			}
		}
		if (delete_adapter) {

			/* 
			 * If no other drives are defined
			 * delete from device switch table,
			 * free adapter structure, and clear
			 * global initialization flag.
			 */

			rc = devswdel(devno);
			if (rc != 0) {
				unlockl(&fd_lock);
				return(rc);
			}
			rc = free((caddr_t)fdadapter);
			fdglobal_init = FALSE;
		}
		break; /* end CFG_TERM case */
	case CFG_INIT:

		if (fdglobal_init == FALSE) {

			/*
		 	 * Allocate the adapter structure and initialize
			 * global parameters.
		 	 */

			fdadapter = (struct adapter_structure *)
			    malloc(sizeof(struct adapter_structure ));
			if (fdadapter == NULL) {
				unlockl(&fd_lock);
				return(ENOMEM);
			}
			fdadapter->adapter_busy = FALSE;
			fdadapter->error_value = 0;
			fdadapter->data_reg_excess = 0;
			/*
			 * Initialize various global data structures.
			 */
			fdadapter->sleep_anchor = EVENT_NULL;
			fdadapter->adapter_sleep_anchor = EVENT_NULL;
			for (i = 0; i < FDMAXDRIVES; i++)
				fdadapter->drive_list[i] = NULL;
		}

		/*
	 	 * If the floppy structure has not already been allocated, then
	 	 * allocate it and initialize needed parameters.
	 	 */

		/*
		 * Check to see if the drive is already initialized.  If it 
		 * is, just return with an EBUSY return code.  If not, 
		 * initialize it.
		 */  
		if (fdadapter->drive_list[drive_number] != NULL) {
			unlockl(&fd_lock);
			return(EBUSY);
		} else
		 {
			fdadapter->drive_list[drive_number] = (struct floppy *)
	    	    	    malloc(sizeof(struct floppy ));
			if (fdadapter->drive_list[drive_number] == NULL) {
				if (fdglobal_init == FALSE) {
					rc = free((caddr_t)fdadapter);
				}
				unlockl(&fd_lock);
				return(ENOMEM);
			}
			fdp = fdadapter->drive_list[drive_number];
			fdp->device_number = devno;
			fdp->drive_state = FDCLOSED;
			/* Code was removed dealing with initializing
			 * floppy data structures.
			 */
		}
		if (fdglobal_init == FALSE) {
			fddevsw.d_open = fd_open;
			fddevsw.d_close = fd_close;
			fddevsw.d_read = fd_read;
			fddevsw.d_write = fd_write;
			fddevsw.d_ioctl = fd_ioctl;
			fddevsw.d_strategy = fd_strategy;
			fddevsw.d_ttys = 0;
			fddevsw.d_select = nodev;
			fddevsw.d_config = fd_config;
			fddevsw.d_print = nodev;
			fddevsw.d_dump = nodev;
			fddevsw.d_mpx = nodev;
			fddevsw.d_revoke = nodev;
			fddevsw.d_dsdptr = NULL;
			fddevsw.d_selptr = NULL;
			fddevsw.d_opts = 0;
			rc = devswadd(devno, &fddevsw);
			if (rc != 0) {
				(void)free((caddr_t)fdp);
				(void)free((caddr_t)fdadapter);
				unlockl(&fd_lock);
				return(rc);
			}
		}
		rc = uiomove((caddr_t)(&configstruct), sizeof(configstruct),
		    UIO_WRITE, uiop);
		if (rc != 0) {
			(void)free((caddr_t)fdp);
			if (fdglobal_init == FALSE) {
				(void)devswdel(devno);
				(void)free((caddr_t)fdadapter);
			}
			unlockl(&fd_lock);
			return(rc);
		}
		configptr = &configstruct;

		/* 
		 * copy device information from the
		 * 'fd_config' structure into the
		 * 'floppy' structure.
		 */

		if (fdglobal_init == FALSE) {
			if (configptr->int_class != fdadapter->int_class) {
				rc = free((caddr_t)fdp);
				rc = free((caddr_t)fdadapter);
				rc = devswdel(devno);
				unlockl(&fd_lock);
				return(EINVAL);
			}
		}

		/*
		 * Code was removed dealing with hardware specific
		 * initialization.
		 */

		/*
		 * initialize the adapter hardware if not already done.
		 */
		if (fdglobal_init == FALSE) {
			fdglobal_init = TRUE;
		}
		break; /* end CFG_INIT case */
	default:
		unlockl(&fd_lock);
		return(EINVAL);
		break;
	} /* end switch on cmd */
	unlockl(&fd_lock);
	return(FDSUCCESS);
} /* end fd_config routine */


/*
 * NAME: fd_open
 *
 * FUNCTION: The open entry point.
 *  This routine prepares a diskette device for use.  This includes
 *  making sure the interrupt and dma services are properly set up
 *  and that the proper device characteristics are loaded for the
 *  drive type and diskette type being used.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It 
 *  can be called only by a process and can page fault. 
 *
 * DATA STRUCTURES: 
 *  fdadapter struct - Per adapter information.
 *  floppy    struct - Per device information.
 *  intr      struct - Interrupt handler structure.
 *
 * INPUTS:
 *  devno  - the major & minor device numbers.
 *  devflag - the operation to perform and flags ORed together. 
 *
 * RETURN VALUE DESCRIPTION: FDSUCCESS (0) or error code. 
 *
 * ERROR CODES: The following errno values may be returned: 
 *  ENOTREADY  - no diskette in drive or drive door open.
 *  EBUSY      - drive already opened by another process.
 *  EINVAL     - invalid minor number. (drive or diskette type).
 *  ENXIO      - drive has been deleted.
 *  EIO        - unable to initialize dma or interrupt services.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  i_init, pincode, w_init, talloc, d_init, delay, lockl, unlockl
 *
 * NOTES:
 */

int	fd_open (dev_t devno, ulong devflag)
{
	register struct floppy *fdp;
	int	i, drive_number, rc;

	lockl(&fd_lock, LOCK_SHORT);

	/*
	 * Use minor device number to determine which drive to use.
	 */

	drive_number = minor(devno) >> 6;
	if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
		unlockl(&fd_lock);
		return(EINVAL);
	}

	/*
	 * Set up a pointer to the proper diskette data structure.
	 */

	fdp = fdadapter->drive_list[drive_number];

	/*
	 * See if this drive has been deleted.  If so, return.
	 */

	if (fdp == NULL) {
		unlockl(&fd_lock);
		return(ENXIO);
	}

	/*
	 * If drive is already open or is being opened, set rc to EBUSY
	 * and return.
	 */

	if (fdp->drive_state != FDCLOSED) {
		unlockl(&fd_lock);
		return(EBUSY);
	}
	fdp->drive_state = FDOPENING;

	/* 
	 * If the device driver has not been initialized, then do so.
	 */

	if (fdadapter->initialized == FALSE) {
		rc = pincode(fd_intr);
		if (rc != 0) {
			fdp->drive_state = FDCLOSED;
			unlockl(&fd_lock);
			return(rc);
		}

		/* 
		 * If the adapter structure has not been pinned, then do so.
		 */

		if (fdadapter->pinned == FALSE) {
			rc = pin((caddr_t)fdadapter,
			    sizeof(struct adapter_structure ));
			if (rc != 0) {
				fdp->drive_state = FDCLOSED;
				fdcleanup(drive_number);
				unlockl(&fd_lock);
				return(rc);
			}
			fdadapter->pinned = TRUE;
		}

		/*  
		 * Initialize the timers.
		 */

		/* 5 second timeout condition */
		fdadapter->inttimer.restart = 5;
		fdadapter->inttimer.func = fdwatchdog;
		fdadapter->inttimer.next = NULL;
		fdadapter->inttimer.prev = NULL;
		fdadapter->inttimer.count = 0;
		w_init(&fdadapter->inttimer);
		/* 10 second timeout condition */
		fdp->motor_off_time = 10;
		fdadapter->mottimer.restart = fdp->motor_off_time;
		fdadapter->mottimer.func = fdmotor_timer;
		fdadapter->mottimer.next = NULL;
		fdadapter->mottimer.prev = NULL;
		fdadapter->mottimer.count = 0;
		w_init(&fdadapter->mottimer);
		fdadapter->fdstart_timer = talloc();
		fdadapter->fdstart_timer->flags = 0;
		fdadapter->fdstart_timer->ipri = INTCLASS3;
		fdadapter->fdstart_timer->func = fdtimeout;

		/*
		 * Allocate the error log structure.
		 */

		fdadapter->fderrptr = (struct fd_err_rec *)
		    xmalloc(sizeof(struct fd_err_rec ), 3, pinned_heap);
		if (fdadapter->fderrptr == NULL) {
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			return(EIO);
		}

		/*
		 * Initialize the interrupt handler.
		 */

		fdadapter->fdhandler = (struct intr *)
		    xmalloc(sizeof(struct intr ), 3, pinned_heap);
		if (fdadapter->fdhandler == NULL) {
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			return(ENOMEM);
		}
		fdadapter->fdhandler->handler = fd_intr;
		fdadapter->fdhandler->bus_type = fdadapter->bus_type;
		fdadapter->fdhandler->flags = 0;
		fdadapter->fdhandler->level = fdadapter->bus_int_level;
		fdadapter->fdhandler->priority = INTCLASS3;
		fdadapter->fdhandler->bid = fdadapter->bus_id;
		if (i_init(fdadapter->fdhandler) != INTR_SUCC) {
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			return(EIO);
		}
		fdadapter->int_init = TRUE;

		/*
		 * Initialize the dma services.
		 */

		fdadapter->dma_id = d_init(fdadapter->dma_level, DMA_SLAVE,
		    fdadapter->bus_id);
		if (fdadapter->dma_id == DMA_FAIL) {
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			return(EIO);
		}
		fdadapter->dma_init = TRUE;
		fdadapter->initialized = TRUE;
	} /* end of adapter and structure initializations. */

	/*
	 * Pin the floppy structure for this drive.
	 */

	rc = pin((caddr_t)fdp, sizeof(struct floppy ));
	if (rc != 0) {
		fdcleanup(drive_number);
		fdp->drive_state = FDCLOSED;
		unlockl(&fd_lock);
		return(rc);
	}

	/*
	 * Load the structure 'floppy' with the default values and the minor
	 * device number.
	 */

	fdp->device_number = devno;
	fdadapter->error_value = 0;
	rc = fdload_floppy(minor(devno) & FDTYPEMASK, fdp);
	if (rc != FDSUCCESS) {
		fdcleanup(drive_number);
		fdp->drive_state = FDCLOSED;
		unlockl(&fd_lock);
		return(rc);
	}

	/*
	 * If this is the first open after config, clear the reset and
	 * enable interrupts to catch the reset interrupt before
	 * anything else is done with the hardware.
	 */

	if (fdadapter->first_open == TRUE) {
		fdadapter->error_value = 0;
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS) {
			fdadapter->error_value = rc;
			unlockl(&fd_lock);
			return(fdopen_exit(fdp));
		}
		fdadapter->state = FD_INITIAL_INTERRUPT;
		w_start(&(fdadapter->inttimer));
		fdadapter->error_value = 0;
		rc = fdexecute_int_cmd(fdreset);
		if (rc != FDSUCCESS) {
			fdadapter->error_value = rc;
			unlockl(&fd_lock);
			return(fdopen_exit(fdp));
		}
		fdunlock_adapter(drive_number);
		fdadapter->state = FD_NO_STATE;
		fdadapter->first_open = FALSE;
	}
	fdp->first_move = TRUE;
		
	/*
	 * Call unlockl to release open lock.
	 */

	unlockl(&fd_lock);

	/*
	 * If the DNDELAY flag is set, the open is done.  Set drive state to
	 * FDOPEN and return.
	 */

	if (devflag & DNDELAY) {
		fdp->drive_state = FDOPEN;
		fdadapter->reset_needed = TRUE;
		return(FDSUCCESS);
	}

	/*
	 * Lock the adapter while accessing it
	 */

	fdadapter->error_value = 0;
	rc = fdlock_adapter(drive_number);
	if (rc != FDSUCCESS) {
		fdadapter->error_value = rc;
		return(fdopen_exit(fdp));
	}


	/*
	 * Check for open drive door.  If so, set rc to ENOTREADY, set
	 * state to FDCLOSED,  and return.  If the door is
	 * closed, just continue.
	 */


	/*
	 * Code for doing several error checks was removed from here.
	 */ 

	/*
	 * Check for write protect if opening for write.
	 */

	if ((devflag & DWRITE) || (devflag & DAPPEND)) {
		fdadapter->error_value = 0;
		fdsense_drive_status();
		if (fdadapter->error_value != 0) {
			return(fdopen_exit(fdp));
		}
		if (fdadapter->results.un1.names.byte0.status3 & 0x40) {
			fdadapter->error_value = EWRPROTECT;
			return(fdopen_exit(fdp));
		} /* end of SENSE_DRIVE_STATUS command processing */
	} /* end of write protect check */

	/*
	 * Call the fdtype() routine. The fdtype() routine will 
	 * either succeed and return FDSUCCESS or fail and return an errno.
	 * If it succeeds, it will load the 'floppy' structure with the correct
	 * diskette characteristics.  
	 */


	/*
	 * Unlock the adapter.
	 */

	fdunlock_adapter(drive_number);

	fdp->drive_state = FDOPEN;
	return(FDSUCCESS);
} /* end of fd_open */



/*
 * NAME: fd_close
 *
 * FUNCTION: The close entry point.
 *  This routine removes a diskette device from use.  This includes
 *  making sure the interrupt and dma services are properly cleared
 *  and that all outstanding i/o requests have been processed.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *  intr      struct - Interrupt handler structure.
 *
 * INPUTS:
 *  devno - the major & minor device numbers.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: The following errno values may be returned:
 *  EINVAL - invalid minor number. (drive or diskette type).
 *
 * EXTERNAL PROCEDURES CALLED:
 *  e_sleep, d_clear, lockl, unlockl
 */

int	fd_close (dev_t devno)
{
	register struct floppy *fdp;
	int	drive_number, rc;

	lockl(&fd_lock, LOCK_SHORT);
	rc = FDSUCCESS;
	drive_number = minor(devno) >> 6;
	if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
		unlockl(&fd_lock);
		return(ENXIO);
	}
	fdp = fdadapter->drive_list[drive_number];
	fdp->drive_state = FDCLOSING;
	unlockl(&fd_lock);

	/* 
	 * Call fdqueue_check() to check if queue is empty.  If it is,
	 * it will turn off the drive motor and return.  Otherwise, it
	 * will sleep until processing is complete.
	 */

	rc = fdqueue_check(fdp, drive_number);
	if (rc != FDSUCCESS) {
		return(rc);
	}

	/* 
	 * Mark this drive as FDCLOSED and return.
	 */

	fdp->drive_state = FDCLOSED;
	return(FDSUCCESS);
} /* end of fd_close() */



/*
 * NAME: fd_read
 *
 * FUNCTION: The raw i/o read entry point.
 *  This routine handles raw i/o read requests.  The uphysio() kernel
 *  service handles all of the processing necessary to transform the
 *  raw i/o request to a block i/o request including calling the
 *  strategy routine.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  floppy struct - Per device information.
 *
 * INPUTS:
 *  devno - the major & minor device numbers.
 *  uiop  - a pointer to a uio structure specifying caller's data.
 *
 * RETURN VALUE DESCRIPTION: returns value returned by uphysio().
 *
 * ERROR CODES: The error codes are all generated by uphysio().
 *
 * EXTERNAL PROCEDURES CALLED: uphysio
 */

int	fd_read (dev_t  devno, register struct uio *uiop)
{
	extern	int	fdnull();
	int	rc;

	rc = uphysio(uiop, B_READ, 2, devno, fd_strategy, fdnull, NULL);
	return(rc);
}



/*
 * NAME:fd_write
 *
 * FUNCTION: The raw i/o write entry point.
 *  This routine handles raw i/o write requests.  The uphysio() kernel
 *  service handles all of the processing necessary to transform the
 *  raw i/o request to a block i/o request including calling the
 *  strategy routine.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  floppy struct - Per device information.
 *
 * INPUTS:
 *   devno - the major & minor device numbers.
 *   uiop  - a pointer to a uio structure specifying caller's data.
 *
 * RETURN VALUE DESCRIPTION: returns value returned by uphysio().
 *
 * ERROR CODES: The error codes are all generated by uphysio().
 *
 * EXTERNAL PROCEDURES CALLED: uphysio
 */

int	fd_write (dev_t  devno, register struct uio *uiop)
{
	extern	int	fdnull();
	int	rc;

	rc = uphysio(uiop, B_WRITE, 2, devno, fd_strategy, fdnull, NULL);
	return(rc);
}




/*
 * NAME: fd_ioctl
 *
 * FUNCTION: The ioctl entry point.
 *  This routine handles most all of the i/o besides data reads and
 *  writes and provides the diagnostics interface to the diskette
 *  drives.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *  devinfo   struct - A standard system structure used with the
 *                     IOCINFO ioctl.
 *  fdinfo    struct - Used to pass information for the
 *                     FDIOCGINFO and the FDIOCSINFO ioctls.
 *  fd_status struct - Used to pass the status of the drive and
 *                     the device driver with the FDIOCSTATUS
 *                     ioctl.
 *  fdparms   struct - Used to pass information for the
 *                     FDIOCGETPARMS and FDIOCSETPARMS ioctls.
 *
 * INPUTS:
 *  devno - a pointer to a buffer header structure.
 *  op    - the ioctl operation to perform.
 *  arg   - a parameter used to pass data to and from user space.
 *          The actual use is dependent on the operation.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: The following errno values may be returned:
 *  EINVAL - either the 'op' parameter, the 'arg' parameter, or
 *           data pointed to by the 'arg' parameter is invalid.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  lockl, unlockl, pin, unpin,copyin, copyout, xmalloc, xmfree,
 *  xmemat, xmemdt, xmemin, malloc, free
 */

int	fd_ioctl (dev_t devno, int op, register long arg, ulong devflag)
{
	register struct floppy *fdp;
	struct devinfo devinfo;
	struct fdinfo fdinfo;
	struct fd_status fdstatus;
	struct fdparms fdparms;
	int	drive_number, rc = FDSUCCESS, local_error_value;

	lockl(&fd_lock, LOCK_SHORT);
	drive_number = minor(devno) >> 6;
	fdp = fdadapter->drive_list[drive_number];

	switch (op) {

	/* 
	 * The following ioctl operation is defined for every device
	 * that uses the ioctl interface.
	 *
	 *
	 *  IOCINFO - returns some information about the diskette.
	 *      This is a standard ioctl option that can be issued
	 *      to find out information about any device that uses
	 *      ioctls.  A pointer to a structure of type devinfo
	 *      should be passed in the 'arg' parameter.  The
	 *      information about the diskette will be loaded
	 *      into the devinfo structure.
	 */

	case IOCINFO:
		devinfo.devtype = DD_DISK;
		devinfo.flags = DF_RAND;
		devinfo.un.dk.bytpsec = (short)(fdp->bytes_per_sector);
		devinfo.un.dk.secptrk = (short)(fdp->sectors_per_track);
		devinfo.un.dk.trkpcyl = (short)(fdp->tracks_per_cylinder);
		devinfo.un.dk.numblks = (long)(fdp->number_of_blocks);

		/* 
		 * Call copyout to copy the devinfo structure
		 * to the user.
		 */

		rc = copyout((char *)(&devinfo), (char *)(arg),
		    sizeof(struct devinfo ));
		if (rc != 0) {
			unlockl(&fd_lock);
			return(rc);
		}
		break; /* end of IOCINFO */
		/* Code was removed dealing with other ioctl's.  The ioctl's
		 * are generally modeled after the IOCINFO ioctl.
		 */
	default:
		rc = EINVAL;
		break; /* end of default case */
	} /* end of switch on op */
	unlockl(&fd_lock);
	return(rc);
} /* end of fd_ioctl() */


/*
 * The following are the top half internal device driver routines.
 * All routines assume that the adapter lock has been secured by the 
 * calling routine.
 */


	/* Various utility functions have been removed. */

int	fdopen_exit (struct floppy *fdp)
{
	int	local_error_value, rc;
	uchar	drive_number;

	drive_number = minor(fdp->device_number) >> 6;
	local_error_value = fdadapter->error_value;
	fddisable_controller();
	rc = unpin((caddr_t)fdp, sizeof(struct floppy));
	fdcleanup(drive_number);
	fdp->drive_state = FDCLOSED;
	fdunlock_adapter(drive_number);
	return(local_error_value);
}
	/* Various utility routines have been removed. */
