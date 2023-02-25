static char sccsid[] = "@(#)50  1.7  src/rspc/kernext/pci/ncr810/ncr8xxt.c, pciscsi, rspc41J, 9517A_all 4/25/95 14:57:15";
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: p8xx_adp_str_init
 *		p8xx_build_command
 *		p8xx_close
 *		p8xx_config
 *		p8xx_inquiry
 *		p8xx_ioctl
 *		p8xx_iowait_script_init
 *		p8xx_issue_abort
 *		p8xx_issue_BDR
 *		p8xx_open
 *		p8xx_readblk
 *		p8xx_script_init
 *		p8xx_start_dev
 *		p8xx_start_unit
 *		p8xx_stop_dev
 *		p8xx_test_unit_rdy
 *		p8xx_undo_open
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/************************************************************************/
/*									*/
/* COMPONENT:	PCISCSI							*/
/*									*/
/* NAME:	ncr8xxt.c						*/
/*									*/
/* FUNCTION:	IBM SCSI Adapter Driver Top Half Source File		*/
/*									*/
/*	This adapter driver is the interface between a SCSI device	*/
/*	driver and the actual SCSI adapter.  It executes commands	*/
/*	from multiple drivers which contain generic SCSI device		*/
/*	commands, and manages the execution of those commands.		*/
/*	Several ioctls are defined to provide for system management	*/
/*	and adapter diagnostic functions.				*/
/*									*/
/* NOTES:	Currently, this device driver is not divided into a	*/
/*		pinned and unpinned part.  If this were to be done,	*/
/*		this file (ncr8xxt.c) is the likely unpinned part.	*/
/*		Doing this has the potential of unpinning appx. 10kb	*/
/*		of text that is currently pinned.  			*/
/*									*/
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/seg.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/time.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/scsi.h>
#include <sys/pm.h>
#include "ncr8xx.h"
/* END OF INCLUDED SYSTEM FILES	 */

/*
 * Global pinned device driver static data areas
 */

/* global pointer array for adapter structures */
extern int      cfg_adp_cnt;               /* count of configured adapters */
extern int	adp_cnt;		   /* count of open adapters */
extern struct	p8xx_cdt_table *p8xx_cdtp; /* ptr to driver's global component
					      dump table */

/* some debugging stuff */
#ifdef	DEBUG_8XX
extern	int	p8xx_debug;		/* controls debug msgs */
extern	int	p8xx_dbsv[P8XX_SVMAX];	/* save areas for debug flags */
extern	int	p8xx_dbidx;		/* current save area index */
#endif	/* DEBUG_8XX */

/*
 * Global pageable device driver static data areas
 */

/* global adapter device driver lock word */
lock_t	  p8xx_lock = LOCK_AVAIL;


/*
 * SCRIPTS compiler output file included here
 */

#include "ncr8xxtss.h"

/************************************************************************/
/*									*/
/* NAME:	p8xx_config						*/
/*									*/
/* FUNCTION:	Adapter Driver Configuration Routine			*/
/*									*/
/*	For the INIT option, this routine allocates and initializes	*/
/*	data structures required for processing user requests to the	*/
/*	adapter.  If the TERM option is specified, this routine will	*/
/*	delete a previously defined device and free the structures	*/
/*	associated with it.  The QVPD options is not supported.         */
/*									*/
/*	During device define time, this routine allocates and		*/
/*	initializes data structures for processing of requests from	*/
/*	the device driver heads to the scsi chip device driver.	    	*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure			*/
/*	adap_ddi - adapter dependent information structure		*/
/*	uio	- user i/o area struct					*/
/*	devsw	- kernel entry point struct				*/
/*									*/
/* INPUTS:								*/
/*	devno	- device major/minor number				*/
/*	op	- operation code (INIT, TERM, or QVPD)			*/
/*	uiop	- pointer to uio structure for data for the		*/
/*		  specified operation code				*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	EFAULT	- return from uiomove					*/
/*	EBUSY	- on terminate, means device still opened		*/
/*	ENOMEM	- memory space unavailable for required allocation	*/
/*	EINVAL	- invalid config parameter passed			*/
/*	EIO	- bad operation, or permanent I/O error			*/
/*	ENXIO   - operation not supported               		*/
/*									*/
/************************************************************************/
int
p8xx_config(
	dev_t		devno,
	int		op,
	struct uio	*uiop)
{
    struct devsw p8xx_dsw;
    p8xx_ddi_t	local_ddi;
    int		ret_code = 0, rc;
    extern int	nodev();
    ads_t	*ap;

#ifdef P8_TRACE
    extern hash_t hash_list[ADS_HASH_MASK+2];
    hash_list[ADS_HASH_MASK+1].next = (struct dev_hash *) 0x50384150;
#endif
    P8_SETDBG(P8_CONFIG,P8_STD_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_config routine.\n"));
    P8Tprintf(("p8xxt: --> config: op %x", op));

    rc = lockl(&(p8xx_lock), LOCK_SHORT);	 /* serialize this */
    if (rc == LOCK_SUCC) {
	/* lock succeded */

	ap = p8xx_get_adp(devno);

	switch (op) {

	    case CFG_INIT:			/* initialize an adapter */
		if (ap) {			/* ptr already initialized? */
		    ret_code = EIO;		/* Bad */
		    break;
		}

		P8if(P8_BREAK,(brkpoint()));
		P8if(P8_INFO_2, (p8xx_hexdump("local_ddi", (char *)&local_ddi, 
				 sizeof(local_ddi)))); 

		/*
		 * Allocate memory for our adapter structure
		 */

		ap = p8xx_alloc_adp(devno);
		if (ap == NULL) {
		    ret_code = ENOMEM;
		    break;
		}

	        /*
	         * Move adapter configuration data (from uio space) into local
	         * area (kernel space).
	         */
	        if (uiomove((caddr_t)(&local_ddi), (int)sizeof(p8xx_ddi_t),
	            UIO_WRITE, (struct uio *) uiop)) {
	            /* copy failed */
		    p8xx_free_adp(ap);
	            ret_code = EIO;
		    break;
		}

		/*
		 * Do any data validation here.
		 *
		 * Note: We only check to ensure that the base address is on a
		 * 4 byte boundry.  Most other things may vary.  For example
		 * interrupt level and priority may be different on different
		 * systems (interrupt level is different with plug-in SCSI
		 * adapters).  Additional checks are performed in
		 * p8xx_config_adapter() when it is called below to ensure that
		 * we are addressing a supported chip.
		 */
		if (local_ddi.addi.base_addr & 0x03) { /* PCI config addresses
						     *must* be on word
						     boundaries */
		    
		    /* problem with ddi data */
		    p8xx_free_adp(ap);
		    ret_code = EINVAL;
		    break;
		}

		/*
		 * Set up entry points to the driver for the device switch
		 * table
		 */
                if (cfg_adp_cnt == 0) {
		    p8xx_dsw.d_open     = (int(*)()) p8xx_open;
		    p8xx_dsw.d_close    = (int(*)()) p8xx_close;
		    p8xx_dsw.d_read     = nodev;
		    p8xx_dsw.d_write    = nodev;
		    p8xx_dsw.d_ioctl    = (int(*)()) p8xx_ioctl;
		    p8xx_dsw.d_strategy = (int(*)()) p8xx_strategy;
		    p8xx_dsw.d_ttys     = 0;
		    p8xx_dsw.d_select   = nodev;
		    p8xx_dsw.d_config   = (int(*)()) p8xx_config;
		    p8xx_dsw.d_print    = nodev;
		    p8xx_dsw.d_dump     = (int(*)()) p8xx_dump;
		    p8xx_dsw.d_mpx      = nodev;
		    p8xx_dsw.d_revoke   = nodev;
		    p8xx_dsw.d_dsdptr   = 0;
		    p8xx_dsw.d_selptr   = 0;
		    p8xx_dsw.d_opts     = 0;

		    rc = devswadd(devno, &p8xx_dsw);
		    if (rc != 0) {
		        /* failed to add to dev switch table */
		        p8xx_free_adp(ap);
		        ret_code = EIO;
		        break;
		    }
		}

		/* Copy local ddi to the adapter structure */
		bcopy(&local_ddi, &ap->ddi, sizeof(p8xx_ddi_t));

		/* 
		 * Do any required processing on the configuration data
		 */
                ASSERT((local_ddi.addi.card_scsi_id & 0xF0) == 0);
		ap->ddi.addi.card_scsi_id &= 0x0F;
		ap->max_request = MAXREQUEST;
		ap->bus_bid     = BID_ALTREG(ap->ddi.addi.bus_id, PCI_IOMEM);
		ap->devno       = devno;
		ap->opened      = FALSE;
		ap->req_ack	= NCR8XX_MAX_OFFSET;	/* max chip offset */
		ap->scsi_clk    = local_ddi.scsi_clk;	/* chip's SCLK in ns */
		if (ap->scsi_clk == 0) {		/* none specified? */
		    ap->scsi_clk = NCR8XX_SCSI_CLK;	/* use default */
		    P8Tprintf(("p8xxt: used default scsi_clk %dns",
			ap->scsi_clk));
		} else {
		    P8Tprintf(("p8xxt: used config'd scsi_clk %dns",
			ap->scsi_clk));
		}
		ap->slow_sync   = local_ddi.slow_sync;	/* sync speed flag */

		/*
		 * Call the kernel service to determine if the activity
		 * light is supported.  Record the result so that future
		 * calls to manipulate the light will short-circuit if the
		 * light is not supported.
		 */
		if (write_operator_panel_light(1) != ENODEV) {
		    ap->light_supported = TRUE;
		    (void) write_operator_panel_light(0);
		} else
		    ap->light_supported = FALSE;

		/* Initialize the variable holding the base address for the */
	        /* chip registers. */
    		ap->base_addr0 = ap->ddi.addi.base_addr + P8XX_BASE_OFFSET;

		/* Initialize the io_map structure for iomem_att() */
		ap->iom.key	= IO_MEM_MAP;
		ap->iom.flags	= 0;
		ap->iom.size	= SEGSIZE;
		ap->iom.busaddr	= ap->base_addr0;
		ap->iom.bid	= ap->bus_bid;

		/* Make the adapter ready */
		if ((rc = p8xx_config_adapter(ap)) != 0) {
		    /* unsuccessful adapter config */
                    if (cfg_adp_cnt == 0)
                        (void) devswdel(devno);         /* clean up */
		    p8xx_free_adp(ap);
		    ret_code = rc;
		    break;
		}

		/* Initialize the power management handler 
		 * (alloc_adp set structure to zero, and also already
		 * initialized the private field to point to ap).
		 */
		ap->pmh.activity = PM_NO_ACTIVITY;
		ap->pmh.mode = PM_DEVICE_FULL_ON;
		ap->pmh.device_idle_time = ap->ddi.pm_dev_itime;
		ap->pmh.device_standby_time = ap->ddi.pm_dev_stime;
		ap->pmh.attribute = 0;
		ap->pmh.devno = devno;
		ap->pmh.handler = p8xx_pm_handler;
		ap->pmh.device_logical_name = ap->ddi.addi.resource_name;
                ap->pmh.pm_version = PM_PHASE2_SUPPORT;

                /* Register Power Management handle */
                rc = pm_register_handle((struct pm_handle *)&(ap->pmh),
                                        PM_REGISTER);
		ASSERT(rc == 0);

#ifdef P8_TRACE
                ap->current_trace_line = 1;
                ap->trace_ptr = (struct p8_trace_struct *)
                xmalloc(sizeof(struct p8_trace_struct), 4, pinned_heap);
#endif
                TRACE_1 ("**P8XXSTART*** ", 0)

		ap->str_inited = TRUE;
                cfg_adp_cnt++;
		break;	/* CFG_INIT case */

	    case CFG_TERM:			/* terminate an adapter */
		if (!ap) {
		    /* device already deleted */
		    ret_code = 0;
		    break;
		}

		if (ap->opened) {
		    /* error, someone else has it opened */
		    ret_code = EBUSY;
		    break;
		}

                /* At this point, the adapter will be unconfigured */
                /* so, decrement the count of configured adapters  */
                cfg_adp_cnt--;

		/* Unregister Power Management handle */
                (void) pm_register_handle((struct pm_handle *)&(ap->pmh),
                                   PM_UNREGISTER);

		/* Don't unconfigure with the chip still in idle, since it
		 * is no longer being power-managed by the device driver.
		 * Restore power, but then mask interrupts.
		 */
		if (ap->pmh.mode == PM_DEVICE_IDLE) {
		    p8xx_turn_chip_on(ap, TRUE);
        	    (void) p8xx_write_reg(ap, (uint) SIEN, (uchar) SIEN_SIZE, 0x0000);
        	    (void) p8xx_write_reg(ap, (uint) DIEN, (uchar) DIEN_SIZE, 0x00);
		}

		/* Disable the chip (assume it is powered on) */
		p8xx_write_cfg(ap, PCI_COMMAND, PCI_COMMAND_SIZE,
		        P8XX_COMMAND_DISABLE);

                /* Remove us from the device switch table */
                if (cfg_adp_cnt == 0)
                    (void) devswdel(devno);

		/* Free the adapter structure */
#ifdef P8_TRACE
            (void) xmfree(ap->trace_ptr, pinned_heap);
#endif
		p8xx_free_adp(ap);
		break;

	    case CFG_QVPD:		/* query for adapter VPD */
		ret_code = ENXIO;
		break;

		/* handle invalid config parameter here */
	    default:
		ret_code = EINVAL;

        }

    } else {
	/* lock failed */
	ret_code = EIO;		/* error--kernel service call failed */
	P8_RSTDBG(P8_CONFIG);
	return (ret_code);
    }

    P8printf(P8_EXIT,("Leaving p8xx_config routine, rc=%x.\n", ret_code));
    P8Tprintf(("p8xxt: <-- config: rc %x", ret_code));
    unlockl(&(p8xx_lock));
    P8_RSTDBG(P8_CONFIG);
    return (ret_code);
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_open						*/
/*									*/
/* FUNCTION:	Adapter Driver Open Routine				*/
/*									*/
/*	This routine opens the scsi chip and makes it ready.  It	*/
/*	allocates adapter specific structures and initializes		*/
/*	appropriate fields in them.  The adapter is marked as		*/
/*	opened.								*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*									*/
/* INPUTS:								*/
/*	devno	- device major/minor number				*/
/*	devflag - & with D_KERNEL					*/
/*	chan	- unused						*/
/*	ext	- extended data; this is 0 for normal use, or		*/
/*		  a value of SC_DIAGNOSTIC selects diagnostic mode	*/
/*		  Note: This mode is not supported.             	*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	EIO	- kernel service failed or invalid operation		*/
/*	EPERM	- authority error					*/
/*	ENOMEM	- not enough memory available				*/
/*									*/
/************************************************************************/
int
p8xx_open(
	dev_t	devno,
	ulong	devflag,
	int	chan,
	int	ext)
{
    ads_t	*ap;
    int		rc, ret_code = 0;
    int		i, old_level;
    int		undo_level = 0;
    ulong	*bscript_ptr;

    P8_SETDBG(P8_OPEN,P8_STD_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_open routine.\n"));
    P8Tprintf(("p8xxt: --> open: chan %x", chan));

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_OPEN, ret_code, devno, devflag,
	    chan, ext, 0);

    rc = lockl(&(p8xx_lock), LOCK_SHORT);	 /* serialize this */
    if (rc == LOCK_SUCC) {

	/* no problem with lock word */

	ap = p8xx_get_adp(devno);
	if (!ap || !ap->str_inited) {
	    ret_code = EIO;
	    p8xx_undo_open(ap, undo_level, ret_code, devno);
	    P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
	    P8_RSTDBG(P8_OPEN);
	    return (ret_code);
	}
	if (ext & SC_DIAGNOSTIC) {
	    /* trying to open in DIAG MODE, this is not supported */
	    ret_code = EINVAL;
	    p8xx_undo_open(ap, undo_level, ret_code, devno);
	    P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
	    P8_RSTDBG(P8_OPEN);
	    return (ret_code);
	}
	if ((privcheck(DEV_CONFIG) != 0) && (!(devflag & DKERNEL))) {
	    /* must be normal open */
	    ret_code = EPERM;
	    p8xx_undo_open(ap, undo_level, ret_code, devno);
	    P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
	    P8_RSTDBG(P8_OPEN);
	    return (ret_code);
	}

	if (!ap->opened) {

	    /* not opened yet */
	    ap->errlog_enable = FALSE;

	    if (pincode(p8xx_intr) != 0) {
		/* pin failed */
		ret_code = EIO;
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

	    /*
	     * Open only in normal mode.  Diagnostic mode not supported.
	     */

	    /* Init reset watchdog timer struct */
	    ap->reset_watchdog.dog.next = NULL;
	    ap->reset_watchdog.dog.prev = NULL;
	    ap->reset_watchdog.dog.func = p8xx_watchdog;
	    ap->reset_watchdog.dog.count = 0;
	    ap->reset_watchdog.dog.restart = RESETWAIT;
	    ap->reset_watchdog.ap = ap;
	    ap->reset_watchdog.timer_id = PSC_RESET_TMR;

	    /* Init restart watchdog timer struct */
	    ap->restart_watchdog.dog.next = NULL;
	    ap->restart_watchdog.dog.prev = NULL;
	    ap->restart_watchdog.dog.func = p8xx_watchdog;
	    ap->restart_watchdog.dog.count = 0;
	    ap->restart_watchdog.dog.restart = 0;
	    ap->restart_watchdog.ap = ap;
	    ap->restart_watchdog.timer_id = PSC_RESTART_TMR;

	    /* Initialize the system timers */
	    w_init(&ap->reset_watchdog.dog);
	    w_init(&ap->restart_watchdog.dog);
	    undo_level++;

	    /* Initialize the adapter structure */
	    p8xx_adp_str_init(ap, (struct intr *) &(ap->intr_struct));

	    /* i_init of scsi chip interrupt handler */
	    if (i_init((struct intr *) &(ap->intr_struct))) {
		/* Failed */
		ret_code = EIO;
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: i_init failed rc %x", ret_code));
		P8if(P8_ABORT, (p8xx_hexdump("intr_struct",
		    (char *)&ap->intr_struct, sizeof(ap->intr_struct))));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

	    /* i_init of epow structure */
	    INIT_EPOW(((struct intr *) &(ap->epow_intr_struct)),
		(int(*)())p8xx_epow, ap->bus_bid);
	    if (i_init((struct intr *) &(ap->epow_intr_struct))) {
		/* i_init of epow structure failed */
		ret_code = EIO;
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: i_init for epow failed rc %x",
		    ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

	    /* Reset the chip, make sure it's on */
            old_level = i_disable(ap->ddi.addi.int_prior);
	    switch (ap->pmh.mode) {
	    	case PM_DEVICE_FULL_ON:
	    	case PM_DEVICE_ENABLE:
	    	    p8xx_chip_register_reset(ap);
		    break;
            	case PM_DEVICE_IDLE:
		    p8xx_turn_chip_on(ap, TRUE);
		    break;
		default: /* should not be in open in suspend/hibernate */
		    ret_code = EIO;
	    }
	    i_enable(old_level);

	    if (ret_code) {
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }

	    /*
	     * Create dump table for first adapter (only)
	     */
	    if (!p8xx_cdtp) {
		/* Allocate and set up the component dump table entry */
		p8xx_cdtp = (struct p8xx_cdt_table *) xmalloc((uint)
		    sizeof(struct p8xx_cdt_table), (uint) 2, pinned_heap);
		if (p8xx_cdtp == NULL) {
		    /* error in dump table memory allocation */
		    ret_code = ENOMEM;
		    p8xx_undo_open(ap, undo_level, ret_code, devno);
		    P8Tprintf(("p8xxt: <-- open: xmalloc of ctd failed"));
		    P8_RSTDBG(P8_OPEN);
		    return (ret_code);
		}
		undo_level++;

		/* Initialize the storage for the dump table */
		bzero((char *) p8xx_cdtp, sizeof(struct p8xx_cdt_table));
		if (dmp_add(p8xx_cdt_func) != 0) {
		    ret_code = ENOMEM;
		    p8xx_undo_open(ap, undo_level, ret_code, devno);
		    P8Tprintf(("p8xxt: <-- open: dmp_add failed"));
		    P8_RSTDBG(P8_OPEN);
		    return (ret_code);
		}

	    } else {
		/* set to xmalloc cdt level (p8xx_cdt wasn't NULL)*/
		undo_level++; 
	    }
	    undo_level++; /* set to dmp_add level (either case) */

	    /* 
	     * Allocate storage for the IOWAIT SCRIPT
	     */
	    ap->Scripts.vscript_ptr = (ulong *) xmalloc((uint) PAGESIZE,
		(uint) PGSHIFT, kernel_heap);
	    if (ap->Scripts.vscript_ptr == NULL) {
		/* error in malloc for scripts */
		ret_code = ENOMEM;
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

            if (ltpin(ap->Scripts.vscript_ptr, PAGESIZE)) {
                ret_code = EIO;
                TRACE_1 ("bad iowait ltpin", ret_code)
                return (ret_code);
            }
	    undo_level++;

	    if ((int) (ap->handle = 
		D_MAP_INIT(ap->bus_bid,
		DMA_MASTER, 0, 0)) == DMA_FAIL) {
		/* failed to init the dma channel */
		ret_code = EIO;
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

	    /* 
	     * Set up the area for scripts and copy.  We are setting up the
	     * IOWAIT script here.  This script serves as our router script
	     * and is used by all target devices.
	     */

	    /* First, make certain scripts will fit */
	    assert(sizeof(IOWAIT_SCRIPT) <= PAGESIZE);

	    for (i = 0; i < sizeof(IOWAIT_SCRIPT)/sizeof(ULONG); i++) {
	        ap->Scripts.vscript_ptr[i] = IOWAIT_SCRIPT[i];
	    }
	    P8if(P8_DUMP_SCRIPTS_1,(p8xx_dump_script(ap->Scripts.vscript_ptr,
	        sizeof(IOWAIT_SCRIPT)/sizeof(ULONG))));

	    /* Get bus address of the scripts buffer */
	    if (D_MAP_PAGE(ap->handle, DMA_READ|DMA_BYPASS, 
		ap->Scripts.vscript_ptr,
		&ap->Scripts.bscript_ptr,
		xmem_global)) {

		/* failed to map scripts */
		ret_code = EIO;
		p8xx_undo_open(ap, undo_level, ret_code, devno);
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

	    /* Patch the IOWAIT script */
 	    p8xx_iowait_script_init(ap, 
 		ap->Scripts.vscript_ptr,
 		ap->Scripts.bscript_ptr);
		
            /* malloc the 4K work area for the nexus data */
            ap->IND_TABLE.system_ptr = (ulong *) xmalloc((uint) PAGESIZE, 
					(uint) PGSHIFT, kernel_heap);
            if (ap->IND_TABLE.system_ptr == NULL) {
                /* error in malloc for indirect table */
                ret_code = ENOMEM;
                TRACE_1 ("ind malloc", 0)
                return (ret_code);
            }
	    undo_level++;

            if (ltpin(ap->IND_TABLE.system_ptr, PAGESIZE)) {
                ret_code = EIO;
                TRACE_1 ("bad ind ltpin", ret_code)
                return (ret_code);
            }
	    undo_level++;

	    /* Get bus address of the nexus data */
	    if (D_MAP_PAGE(ap->handle, DMA_READ|DMA_BYPASS, 
		    ap->IND_TABLE.system_ptr,
		    &ap->IND_TABLE.dma_ptr,
		    xmem_global)) {

		/* failed to map nexus page */
		ret_code = EIO;
		TRACE_1("bad map ind", 0)
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

            /* malloc the 4K work area for the local tim entries */
            ap->loc_tim_tbl.system_ptr = (ulong *) xmalloc((uint)
                                     PAGESIZE, (uint) PGSHIFT, kernel_heap);
            if (ap->loc_tim_tbl.system_ptr == NULL) {
                /* error in malloc for loc table */
                ret_code = ENOMEM;
                TRACE_1 ("loc malloc", 0)
                return (ret_code);
            }
	    undo_level++;

            if (ltpin(ap->loc_tim_tbl.system_ptr, PAGESIZE)) {
                ret_code = EIO;
                TRACE_1 ("bad loc ltpin", ret_code)
                return (ret_code);
            }
	    undo_level++;

	    /* Get bus address of the local tim table */
	    if (D_MAP_PAGE(ap->handle, DMA_READ|DMA_BYPASS, 
		    ap->loc_tim_tbl.system_ptr,
		    &ap->loc_tim_tbl.dma_ptr,
		    xmem_global)) {

		/* failed to map local table page */
		ret_code = EIO;
		TRACE_1("bad map loc", 0)
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

            /* malloc the 4K work area for the saved tim entries */
            ap->sav_tim_tbl.system_ptr = (ulong *) xmalloc((uint)
                           PAGESIZE, (uint) PGSHIFT, kernel_heap);
            if (ap->sav_tim_tbl.system_ptr == NULL) {
                /* error in malloc for save tim table */
                ret_code = ENOMEM;
                TRACE_1 ("sav malloc", 0)
                return (ret_code);
            }
	    undo_level++;

            if (ltpin(ap->sav_tim_tbl.system_ptr, PAGESIZE)) {
                ret_code = EIO;
                TRACE_1 ("bad sav ltpin", ret_code)
                return (ret_code);
            }
	    undo_level++;

	    /* Get bus address of the save tim table page */
	    if (D_MAP_PAGE(ap->handle, DMA_READ|DMA_BYPASS, 
		    ap->sav_tim_tbl.system_ptr,
		    &ap->sav_tim_tbl.dma_ptr,
		    xmem_global)) {

		/* failed to map save tim table page */
		D_MAP_CLEAR(ap->handle);
		ret_code = EIO;
		TRACE_1("bad map sav", 0)
		P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
		P8_RSTDBG(P8_OPEN);
		return (ret_code);
	    }
	    undo_level++;

            DIO_INIT(&ap->vlist, 1);
            if (ap->vlist.dvec == NULL) {
		TRACE_1("DIO_INIT vl", 0)
	        return(ENOMEM);
            }
	    undo_level++;

            DIO_INIT(&ap->blist, MAX_BLIST);
            if (ap->blist.dvec == NULL) {
		TRACE_1("DIO_INIT bl", 0)
	        return(ENOMEM);
            }
	    ap->blist_base_vector = (ulong *) ap->blist.dvec;

	    ap->errlog_enable = TRUE;
	    ap->power_state = P8XX_NORMAL_PWR;

	    ap->open_mode = NORMAL_MODE;	/* only mode supported */
	    ap->opened = TRUE;			/* we are now open */
	    adp_cnt++;				/* keep a count */

	}	/* not opened yet */

    } else {

	/* lock failed */
	ret_code = EIO;
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
	P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
	P8_RSTDBG(P8_OPEN);
	return (ret_code);
    }

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);

    unlockl(&(p8xx_lock));
    P8printf(P8_EXIT,("Leaving p8xx_open routine.\n"));
    P8Tprintf(("p8xxt: <-- open: rc %x", ret_code));
    P8_RSTDBG(P8_OPEN);
    return (ret_code);
}



/************************************************************************/
/*									*/
/* NAME:	p8xx_close						*/
/*									*/
/* FUNCTION:	Adapter Driver Close Routine				*/
/*									*/
/*	This routine closes the scsi chip instance and releases any	*/
/*	resources (as well as unpins the code) that were setup at open	*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*									*/
/* INPUTS:								*/
/*	devno	- device major/minor number				*/
/*	chan	- 0; unused						*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	0	- successful						*/
/*	EIO	- kernel service failed or invalid operation		*/
/*									*/
/************************************************************************/

int
p8xx_close(
	dev_t	devno,
	int	chan)
{
    ads_t	*ap;
    int		ret_code = 0, rc;
    int		i, old_level;

    P8printf(P8_ENTRY,("Entering p8xx_close routine.\n"));
    P8Tprintf(("p8xxt: --> close: chan %x", chan));
    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_CLOSE, ret_code, devno);

    rc = lockl(&(p8xx_lock), LOCK_SHORT);	 /* serialize this */
    if (rc != LOCK_SUCC) {
	/* lock kernel call failed */
	ret_code = EIO;
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
	P8Tprintf(("p8xxt: <-- close: rc %x", ret_code));
	return (ret_code);
    }

    ap = p8xx_get_adp(devno);
    if (!ap || !ap->opened) {
	/* adapter never opened */
	ret_code = EIO;
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
	unlockl(&(p8xx_lock));
	P8Tprintf(("p8xxt: <-- close: rc %x", ret_code));
	return (ret_code);
    }

    /* 
     * Loop through the device hash table and stop devices that are still
     * started.
     */
    for (i = 0; i < MAX_DEVICES; i++) {
	if (ap->device_queue_hash[i] != NULL)	/* Device still started? */
	    p8xx_stop_dev(ap, i);	 /* stops and frees resources */
    }

    /* Disable dma and SCSI interrupts from the chip (if it's on) */
    old_level = i_disable(ap->ddi.addi.int_prior);
    if ((ap->pmh.mode == PM_DEVICE_FULL_ON) ||
	(ap->pmh.mode == PM_DEVICE_ENABLE)) {

        (void) p8xx_write_reg(ap, (uint) SIEN, (uchar) SIEN_SIZE, 0x0000);
        (void) p8xx_write_reg(ap, (uint) DIEN, (uchar) DIEN_SIZE, 0x00);
    }
    i_enable(old_level);

    /*
     * Undo the processing done on the open call.
     * The tracehook and freeing of the lock is done within
     * p8xx_undo_open.
     */
    p8xx_undo_open(ap, P8_CLOSE_LEVEL, ret_code, devno);

    P8printf(P8_EXIT,("Leaving p8xx_close routine.\n"));
    P8Tprintf(("p8xxt: <-- close: rc %x", ret_code));
    return (ret_code);

}  /* end p8xx_close */

/**************************************************************************/
/*									  */
/* NAME:  p8xx_undo_open						  */
/*									  */
/* FUNCTION:  Cleans up during failed "open" processing.		  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can only be called by a process.			  */
/*									  */
/* NOTES:  This routine is called to perform processing when a failure	  */
/*	   of the open occurs. This entails freeing storage, unpinning	  */
/*	   the code, etc.						  */
/*									  */
/* DATA STRUCTURES:							  */
/*	adapter_def - scsi chip unique data structure			  */
/*									  */
/* INPUTS:								  */
/*	undo_level  -  value of amount of cleanup required.		  */
/*	ret_code    -  return value used for trace.			  */
/*	devno	    -  device major/minor number.			  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	None								  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*									  */
/**************************************************************************/
void
p8xx_undo_open(
	ads_t   *ap,
	int	undo_level,
	int	ret_code,
	dev_t	devno)
{
    P8printf(P8_ENTRY,("Entering p8xx_undo_open routine.\n"));

    switch  (undo_level) {	/* undo_level controls what resources we
				   must release */
	case P8_CLOSE_LEVEL:    /* free all resources, called by p8xx_close */
            /* Mark as closed */
            ap->opened = FALSE;
            adp_cnt--;
            ASSERT (adp_cnt >= 0);
            if (ap->blist_base_vector != NULL) {
                ap->blist.dvec = (struct d_iovec *) ap->blist_base_vector;
                DIO_FREE(&ap->blist);
            }

	case 20:		/* free memory used in dio structure */
            DIO_FREE(&ap->vlist);

	case 19:		/* deallocate save table mapping resources */
	    D_UNMAP_PAGE(ap->handle, ap->sav_tim_tbl.dma_ptr);

        case 18: 		/* unpin memory taken by save table */
            (void) ltunpin(ap->sav_tim_tbl.system_ptr, PAGESIZE);

	case 17: 		/* free memory taken by save table */
 	    (void) xmfree((void *) ap->sav_tim_tbl.system_ptr, kernel_heap);

	case 16:		/* deallocate loc table mapping resources */
	    D_UNMAP_PAGE(ap->handle, ap->loc_tim_tbl.dma_ptr);

        case 15: 		/* unpin memory taken by loc table */
            (void) ltunpin(ap->loc_tim_tbl.system_ptr, PAGESIZE);

	case 14: 		/* free memory taken by loc table */
	    (void) xmfree((void *) ap->loc_tim_tbl.system_ptr, kernel_heap);

	case 13: 		/* deallocate indirect-tbl mapping resources */
	    D_UNMAP_PAGE(ap->handle, ap->IND_TABLE.dma_ptr);

        case 12: 		/* unpin memory taken by the indirect table */
            (void) ltunpin(ap->IND_TABLE.system_ptr, PAGESIZE);

	case 11: 		/* free memory taken by the indirect table */
	    (void) xmfree((void *) ap->IND_TABLE.system_ptr, kernel_heap);

	case 10: 		/* deallocate iowait script mapping resources */
	    D_UNMAP_PAGE(ap->handle, ap->Scripts.bscript_ptr);

	case 9:			/* deallocate resources obtained by map_init */
	    D_MAP_CLEAR(ap->handle);

        case 8: 		/* unpin memory taken by iowait script */
            (void) ltunpin(ap->Scripts.vscript_ptr, PAGESIZE);

	case 7: 		/* free memory taken by iowait script */
	    (void) xmfree((void *) ap->Scripts.vscript_ptr, kernel_heap);

	case 6:			/* clear the cdt_func if no open adapters */
	    if (adp_cnt == 0)
	        (void) dmp_del(p8xx_cdt_func);

	case 5:			/* free the cdt_table if no open adapters */
	    if (adp_cnt == 0) {
		(void) xmfree((void *) p8xx_cdtp, pinned_heap);
		p8xx_cdtp = NULL;
	    }

	case 4:			/* clear the EPOW structure */
	    i_clear((struct intr *) &(ap->epow_intr_struct));

	case 3:			/* clear out the scsi chip interrupt handler */
	    i_clear((struct intr *) &(ap->intr_struct));

	case 2:			/* clear all system timers */
	    w_clear(&ap->reset_watchdog.dog);
	    w_clear(&ap->restart_watchdog.dog);

	case 1:			/* unpin the device driver */
	    (void) unpincode(p8xx_intr);

	default:
	    break;

    }

    P8printf(P8_EXIT,("Leaving p8xx_undo_open routine.\n"));

    if (undo_level == P8_CLOSE_LEVEL)
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
    else
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);

    unlockl(&(p8xx_lock));
    return;
}



/**************************************************************************/
/*									  */
/* NAME:  p8xx_inquiry							  */
/*									  */
/* FUNCTION:  Issues a SCSI inquiry command to a device.		  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process.			  */
/*									  */
/* NOTES:  This routine is called to issue an inquiry to a device.	  */
/*	   The calling process is responsible for NOT calling this rou-	  */
/*	   tine if the SCIOSTART failed.  Such a failure would indicate	  */
/*	   that another process has this device open and interference	  */
/*	   could cause improper error reporting.			  */
/*									  */
/*									  */
/* DATA STRUCTURES:							  */
/*	adapter_def - scsi chip unique data structure			  */
/*									  */
/* INPUTS:								  */
/*	devno	- passed device major/minor number			  */
/*	arg	- passed argument value					  */
/*	devflag - device flag						  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	0 - for good completion, ERRNO value otherwise.			  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*	EINVAL - Device not opened.					  */
/*	ENOMEM - Could not allocate an scbuf for this command.		  */
/*	ENODEV - Device could not be selected.				  */
/*	ENOCONNECT - No connection (SCSI bus fault)..			  */
/*	ETIMEDOUT - The inquiry command timed out.			  */
/*	EIO    - Error returned from p8xx_strategy.			  */
/*	EIO    - No data returned from inquiry command.			  */
/*	EFAULT - Bad copyin or copyout.					  */
/*									  */
/**************************************************************************/

int
p8xx_inquiry(
	dev_t	devno,
	int	arg,
	ulong	devflag)
{
    int			ret_code = 0,
			dev_index, inquiry_length;
    ads_t		*ap;
    struct sc_buf	*bp;
    struct sc_inquiry	sc_inq;

    P8printf(P8_ENTRY,("Entering p8xx_inquiry routine.\n"));
    P8Tprintf(("p8xxt: --> inquiry: arg %x", arg));

    if ((ap = p8xx_get_adp(devno)) == NULL) {
        P8Tprintf(("p8xxt: <-- inquiry: rc %x", EIO));
	return(EIO);
    }

    /* 
     * Copy in the arg structure. If the buffer resides user space, use
     * copyin, else use bcopy.
     */
    if (!(devflag & DKERNEL)) {
	ret_code = copyin((char *) arg, &sc_inq, sizeof(struct sc_inquiry));
	if (ret_code != 0) {
            P8Tprintf(("p8xxt: <-- inquiry: rc %x", EFAULT));
	    return (EFAULT);
	}
    } else {
	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_inq, sizeof(struct sc_inquiry));
    }

    P8Tprintf(("p8xxt: ID %02x LUN %02x", sc_inq.scsi_id, sc_inq.lun_id));

    /* Obtain device structure from hash on the scsi id and lun id. */
    dev_index = INDEX(sc_inq.scsi_id, sc_inq.lun_id);

    if (ap->device_queue_hash[dev_index] == NULL) {
	/* Device queue structure not allocated */
        P8Tprintf(("p8xxt: <-- inquiry: rc %x", EINVAL));
	return (EINVAL);
    }
    bp = p8xx_build_command();
    if (bp == NULL) {
        P8Tprintf(("p8xxt: <-- inquiry: rc %x", ENOMEM));
	return (ENOMEM);	/* could'nt allocate command */
    }

    bp->scsi_command.scsi_id = sc_inq.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_INQUIRY;
    if (sc_inq.get_extended) {
	bp->scsi_command.scsi_cmd.lun = (sc_inq.lun_id << 5) | 0x01;
	bp->scsi_command.scsi_cmd.scsi_bytes[0] = sc_inq.code_page_num;
    } else {
	bp->scsi_command.scsi_cmd.lun = sc_inq.lun_id << 5;
	bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    }
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;

    /* Always set for the maximum amt of inquiry data */
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 255;
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* 
     * Pass thru the no disconnect flag and the synch/asynch flag for proper 
     * data tranfer to occur.
     */
    bp->scsi_command.flags = (sc_inq.flags & (SC_ASYNC | SC_NODISC));
    P8if(P8_ABORT, {
	if (sc_inq.flags & SC_ASYNC)
	    printf("inquiry forces ASYNC\n");
    });

    bp->bufstruct.b_bcount = 255;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = 15;


    /* 
     * Set resume flag in case caller is retrying this operation.  This assumes
     * the inquiry is only running single-threaded to this device.
     */
    bp->flags = (SC_RESUME | SC_DELAY_CMD);

    /* Call our strategy routine to issue the inquiry command. */
    if (p8xx_strategy(bp)) {
	/* an error was returned */
	(void) xmfree(bp, pinned_heap); /* release buffer */
        P8Tprintf(("p8xxt: <-- inquiry: rc %x", EIO));
	return (EIO);
    }

    iowait((struct buf *) bp);	/* Wait for commmand completion */

    /* 
     * The return value from the operation is examined to determine what
     * type of error occurred.  Since the calling application requires
     * an ERRNO, this value is interpreted here.
     */
    if (bp->bufstruct.b_flags & B_ERROR) {
	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR) {
	    /* if adapter error */
	    switch (bp->general_card_status) {
		case SC_CMD_TIMEOUT:
		    p8xx_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT, 0x37, 0,
			ap, NULL, FALSE);
		    ret_code = ETIMEDOUT;
		    break;
		case SC_NO_DEVICE_RESPONSE:
		    ret_code = ENODEV;
		    break;
		case SC_SCSI_BUS_FAULT:
		    ret_code = ENOCONNECT;
		    break;
		default:
		    ret_code = EIO;
		    break;
	    }
	} else
	    ret_code = EIO;
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) && (bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
	ret_code = EIO;

    /* 
     * Give the caller the lesser of what he asked for, or the actual
     * transfer length
     */
    if (ret_code == 0) {
	inquiry_length = bp->bufstruct.b_bcount - bp->bufstruct.b_resid;
	if (inquiry_length > sc_inq.inquiry_len)
	    inquiry_length = sc_inq.inquiry_len;

	/* 
	 * Copy out the inquiry data. If the buffer resides user space, use
	 * copyin, else use bcopy.
	 */
	if (!(devflag & DKERNEL)) {
	    ret_code = copyout(bp->bufstruct.b_un.b_addr,
			       sc_inq.inquiry_ptr, inquiry_length);
	    if (ret_code != 0)
		ret_code = EFAULT;
	} else {
	    /* buffer is in kernel space */
	    bcopy(bp->bufstruct.b_un.b_addr, sc_inq.inquiry_ptr,
		  inquiry_length);
	}
    }

    (void) xmfree(bp, pinned_heap);	/* release buffer */
    P8printf(P8_EXIT,("Leaving p8xx_inquiry routine. %d\n", ret_code));
    P8Tprintf(("p8xxt: <-- inquiry: rc %x", ret_code));
    return (ret_code);
}

/**************************************************************************/
/*									  */
/* NAME:  p8xx_start_unit						  */
/*									  */
/* FUNCTION:  Issues a SCSI start unit command to a device.		  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process.			  */
/*									  */
/* NOTES:  This routine is called to to issue a start unit command to a	  */
/*	   device.							  */
/*	   The calling process is responsible for NOT calling this rou-	  */
/*	   tine if the SCIOSTART failed.  Such a failure would indicate	  */
/*	   that another process has this device open and interference	  */
/*	   could cause improper error reporting.			  */
/*									  */
/*									  */
/* DATA STRUCTURES:							  */
/*	adapter_def - scsi chip unique data structure			  */
/*									  */
/* INPUTS:								  */
/*	devno	- passed device major/minor number			  */
/*	arg	- passed argument value					  */
/*	devflag - device flag						  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	0 - for good completion, ERRNO value otherwise.			  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*    EINVAL - Device not opened.					  */
/*    ENOMEM - Could not allocate an scbuf for this command.		  */
/*    ENODEV - Device could not be selected.				  */
/*    ETIMEDOUT - The start unit command timed out.			  */
/*    ENOCONNECT - No connection (SCSI bus fault)..			  */
/*    EIO    - Error returned from p8xx_strategy.			   */
/*    EFAULT - Bad copyin or copyout.					  */
/*									  */
/**************************************************************************/
int
p8xx_start_unit(
	dev_t	devno,
	int	arg,
	ulong	devflag)
{
    ads_t		*ap;
    int			ret_code = 0,
			dev_index;
    struct sc_buf	*bp;
    struct sc_startunit	sc_stun;

    P8printf(P8_ENTRY,("Entering p8xx_start_unit.\n"));
    P8Tprintf(("p8xxt: --> start_unit: arg %x", arg));

    if ((ap = p8xx_get_adp(devno)) == NULL) {
	P8Tprintf(("p8xxt: <-- start_unit: rc %x", EIO));
	return(EIO);
    }

    /* 
     * Copy in the arg structure. If the buffer resides user space, use
     * copyin, else use bcopy.
     */
    if (!(devflag & DKERNEL)) {
	/* Buffer is in user space, copy it */
	if (copyin((char *)arg, &sc_stun, sizeof(struct sc_startunit)) != 0) {
	    P8Tprintf(("p8xxt: <-- start_unit: rc %x", EFAULT));
	    return (EFAULT);
	}
    } else {
	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_stun, sizeof(struct sc_startunit));
    }

    /* Obtain device structure from hash on the scsi id and lun id. */
    dev_index = INDEX(sc_stun.scsi_id, sc_stun.lun_id);

    if (ap->device_queue_hash[dev_index] == NULL) {
	/* Device queue structure not allocated */
	P8Tprintf(("p8xxt: <-- start_unit: rc %x", EINVAL));
	return (EINVAL);
    }

    if ((bp = p8xx_build_command()) == NULL) {
	P8Tprintf(("p8xxt: <-- start_unit: rc %x", ENOMEM));
	return (ENOMEM);	/* unable to allocate command */
    }

    bp->scsi_command.scsi_id = sc_stun.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;

    /* 
     * The immediate bit for this command is set depending on the value of
     * the immediate flag.
     */
    bp->scsi_command.scsi_cmd.lun = (sc_stun.lun_id << 5) |
	(sc_stun.immed_flag ? 0x01 : 0);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    /* Set the command start flag */
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = (sc_stun.start_flag ? 0x01 : 0);
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /*
     * Pass thru the no disconnect flag and the synch/asynch flag for proper 
     * data transfer to occur.
     */
    bp->scsi_command.flags = (sc_stun.flags & (SC_ASYNC | SC_NODISC));
    P8if(P8_ABORT, {
	if (sc_stun.flags & SC_ASYNC)
	    printf("start/stop unit forces ASYNC\n");
    });

    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = sc_stun.timeout_value;	/* set timeout value */

    /* 
     * Set resume flag in case caller is retrying this operation.  This assumes
     * the inquiry is only running single-threaded to this device.
     */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (p8xx_strategy(bp)) {
	/* an error was returned */
	(void) xmfree(bp, pinned_heap); /* release buffer */
	P8Tprintf(("p8xxt: <-- start_unit: rc %x", EIO));
	return (EIO);
    }

    iowait((struct buf *) bp);	/* Wait for commmand completion */

    /* 
     * The return value from the operation is examined to determine what
     * type of error occurred.  Since the calling application requires an
     * ERRNO, this value is interpreted here.
     */
    if (bp->bufstruct.b_flags & B_ERROR) {
	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR) {
	    /* if adapter error */
	    switch (bp->general_card_status) {
		case SC_CMD_TIMEOUT:
		    p8xx_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT, 0x3C, 0,
			ap, NULL, FALSE);
		    ret_code = ETIMEDOUT;
		    break;
		case SC_NO_DEVICE_RESPONSE:
		    ret_code = ENODEV;
		    break;
		case SC_SCSI_BUS_FAULT:
		    ret_code = ENOCONNECT;
		    break;
		default:
		    ret_code = EIO;
		    break;
	    }
	} else
	    ret_code = EIO;
    }

    (void) xmfree(bp, pinned_heap);	/* release buffer */
    P8printf(P8_EXIT,("Leaving p8xx_start_unit .\n"));
    P8Tprintf(("p8xxt: <-- start_unit: rc %x", ret_code));
    return (ret_code);
}



/**************************************************************************/
/*									  */
/* NAME:  p8xx_test_unit_rdy						  */
/*									  */
/* FUNCTION:  Issues a SCSI test unit ready command to a device.	  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process.			  */
/*									  */
/* NOTES:  This routine is called to to issue a test unit ready command	  */
/*	   to a device.							  */
/*	   The calling process is responsible for NOT calling this rou-	  */
/*	   tine if the SCIOSTART failed.  Such a failure would indicate	  */
/*	   that another process has this device open and interference	  */
/*	   could cause improper error reporting.			  */
/*									  */
/* DATA STRUCTURES:							  */
/*	adapter_def - scsi chip unique data structure			  */
/*									  */
/* INPUTS:								  */
/*	devno	- passed device major/minor number			  */
/*	arg	- passed argument value					  */
/*	devflag - device flag						  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	0 - for good completion, ERRNO value otherwise.			  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*    EINVAL - Device not opened.					  */
/*    ENOMEM - Could not allocate an scbuf for this command.		  */
/*    ENODEV - Device could not be selected.				  */
/*    ETIMEDOUT - The start unit command timed out.			  */
/*    ENOCONNECT - No connect (SCSI bus fault).				  */
/*    EIO    - Error returned from p8xx_strategy.			   */
/*    EFAULT - Bad copyin or copyout.					  */
/*									  */
/**************************************************************************/
int
p8xx_test_unit_rdy(
	dev_t	devno,
	int	arg,
	ulong	devflag)
{
    ads_t		*ap;
    int			ret_code = 0,
			ret_code2, dev_index;
    struct sc_buf	*bp;
    struct sc_ready	sc_rdy;

    P8printf(P8_ENTRY,("Entering p8xx_test_unit_rdy routine.\n"));
    P8Tprintf(("p8xxt: --> test_unit_rdy: arg %x", arg));

    if ((ap = p8xx_get_adp(devno)) == NULL) {
	P8Tprintf(("p8xxt: <-- test_unit_rdy: rc %x", EINVAL));
	return(EINVAL);
    }

    /* 
     * Copy in the arg structure. If the buffer resides user space, use
     * copyin, else use bcopy.
     */
    if (!(devflag & DKERNEL)) {
	ret_code = copyin((char *) arg, &sc_rdy, sizeof(struct sc_ready));
	if (ret_code != 0) {
	    P8Tprintf(("p8xxt: <-- test_unit_rdy: rc %x", EINVAL));
	    return (EFAULT);
	}
    } else {
	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_rdy, sizeof(struct sc_ready));
    }

    /* 
     * Obtain device structure from hash on the scsi id and lun id.
     */
    dev_index = INDEX(sc_rdy.scsi_id, sc_rdy.lun_id);
    if (ap->device_queue_hash[dev_index] == NULL) {
	/* Device queue structure not allocated */
	P8Tprintf(("p8xxt: <-- test_unit_rdy: rc %x", EINVAL));
	return (EINVAL);
    }

    bp = p8xx_build_command();
    if (bp == NULL) {
	P8Tprintf(("p8xxt: <-- test_unit_rdy: rc %x", ENOMEM));
	return (ENOMEM);	/* could'nt allocate command */
    }

    bp->scsi_command.scsi_id = sc_rdy.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_TEST_UNIT_READY;
    bp->scsi_command.scsi_cmd.lun = (sc_rdy.lun_id << 5);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* 
     * There is no need to set the no disconnect flag for the test unit ready
     * command so just set the synch/asynch flag for the sc_rdy structure.
     */
    bp->scsi_command.flags = sc_rdy.flags & SC_ASYNC;
    P8if(P8_ABORT, {
	if (sc_rdy.flags & SC_ASYNC)
	    printf("test unit ready forces ASYNC\n");
    });
    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;

    /* Initialize default status to zero.		     */
    sc_rdy.status_validity = 0;
    sc_rdy.scsi_status = 0;

    /*
     * Set resume flag in case caller is retrying this operation.  This assumes
     * the inquiry is only running single-threaded to this device.
     */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (p8xx_strategy(bp)) {
	/* an error was returned */
	(void) xmfree(bp, pinned_heap); /* release buffer */
	P8Tprintf(("p8xxt: <-- test_unit_rdy: rc %x", EIO));
	return (EIO);
    }

    iowait((struct buf *) bp);	/* Wait for commmand completion */

    /*
     * The return value from the operation is examined to determine what
     * type of error occurred.  Since the calling application requires an
     * ERRNO, this value is interpreted here.
     */
    if (bp->bufstruct.b_flags & B_ERROR) {
	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR) {
	    /* if adapter error */
	    switch (bp->general_card_status) {
		case SC_CMD_TIMEOUT:
		    p8xx_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
			0x41, 0, ap, NULL, FALSE);
		    ret_code = ETIMEDOUT;
		    break;
		case SC_NO_DEVICE_RESPONSE:
		    ret_code = ENODEV;
		    break;
		case SC_SCSI_BUS_FAULT:
		    ret_code = ENOCONNECT;
		    break;
		default:
		    ret_code = EIO;
		    break;
	    }
	} else if (bp->status_validity & SC_SCSI_ERROR) {
	    /* if a scsi status error */
	    sc_rdy.status_validity = SC_SCSI_ERROR;
	    sc_rdy.scsi_status = bp->scsi_status;
	    ret_code = EIO;
	} else {
	    /* if general error (fall through case) */
	    ret_code = EIO;
	}
    }

    /* 
     * Copy out the device status to the st_ready structure passed in by
     * the calling application.
     */
    if (!(devflag & DKERNEL)) {
	ret_code2 = copyout(&sc_rdy, (char *) arg, sizeof(struct sc_ready));
	if (ret_code2 != 0)
	    ret_code = EFAULT;
    } else {
	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_rdy, sizeof(struct sc_ready));
    }

    (void) xmfree(bp, pinned_heap);	/* release buffer */
    P8printf(P8_EXIT,("Leaving p8xx_test_unit_rdy routine.\n"));
    P8Tprintf(("p8xxt: <-- test_unit_rdy: rc %x", ret_code));
    return (ret_code);
}



/**************************************************************************/
/*									  */
/* NAME:  p8xx_readblk							  */
/*									  */
/* FUNCTION:  Issues a SCSI read command to a device.			  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process.			  */
/*									  */
/* NOTES:  This routine is called to read command to a device.		  */
/*	   The calling process is responsible for NOT calling this rou-	  */
/*	   tine if the SCIOSTART failed.  Such a failure would indicate	  */
/*	   that another process has this device open and interference	  */
/*	   could cause improper error reporting.			  */
/*									  */
/* DATA STRUCTURES:							  */
/*	adapter_def - scsi chip unique data structure			  */
/*									  */
/* INPUTS:								  */
/*	devno	- passed device major/minor number			  */
/*	arg	- passed argument value					  */
/*	devflag - device flag						  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	0 - for good completion, ERRNO value otherwise.			  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*    EINVAL - Device not opened, or transfer size is > 1 page		  */
/*    ENOMEM - Could not allocate an scbuf for this command.		  */
/*    ENODEV - Device could not be selected.				  */
/*    ETIMEDOUT - The inquiry command timed out.			  */
/*    ENOCONNECT - SCSI bus fault.					  */
/*    EIO    - Error returned from p8xx_strategy.			  */
/*    EIO    - No data returned from read command.			  */
/*    EFAULT - Bad copyin or copyout.					  */
/*									  */
/**************************************************************************/
int
p8xx_readblk(
	dev_t	devno,
	int	arg,
	ulong	devflag)
{
    int			ret_code = 0, dev_index;
    ads_t		*ap;
    struct sc_buf	*bp;
    struct sc_readblk	sc_rd;

    P8printf(P8_EXIT,("Entering p8xx_readblk routine.\n"));
    P8Tprintf(("p8xxt: --> readblk: arg %x", arg));

    if ((ap = p8xx_get_adp(devno)) == NULL) {
	P8Tprintf(("p8xxt: --> readblk: rc %x", EINVAL));
	return(EINVAL);
    }


    /*
     * Copy in the arg structure. If the buffer resides user space, use
     * copyin, else use bcopy.
     */
    if (!(devflag & DKERNEL)) {
	ret_code = copyin((char *) arg, &sc_rd,
			  sizeof(struct sc_readblk));
	if (ret_code != 0) {
	    P8Tprintf(("p8xxt: --> readblk: rc %x", EFAULT));
	    return (EFAULT);
	}
    } else {
	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_rd, sizeof(struct sc_readblk));
    }

    /* 
     * Obtain device structure from hash on the scsi id and lun id.
     */
    dev_index = INDEX(sc_rd.scsi_id, sc_rd.lun_id);

    /* 
     * Fail if device queue structure already allocated or the transfer length
     * is too long to be allowed.
     */
    if ((ap->device_queue_hash[dev_index] == NULL) ||
        ((int) sc_rd.blklen > PAGESIZE)) {
	P8Tprintf(("p8xxt: --> readblk: rc %x", EINVAL));
	return (EINVAL);
    }

    if ((bp = p8xx_build_command()) == NULL) {
	P8Tprintf(("p8xxt: --> readblk: rc %x", ENOMEM));
	return (ENOMEM);	/* could'nt allocate command */
    }

    /* Xmalloc a page to be used for the data transfer.	*/
    bp->bufstruct.b_un.b_addr = xmalloc((uint) PAGESIZE, (uint) PGSHIFT,
					pinned_heap);

    if (bp->bufstruct.b_un.b_addr == NULL) {
	/* Unable to get required storage */
	(void) xmfree(bp, pinned_heap); /* release sc_buf */
	P8Tprintf(("p8xxt: --> readblk: rc %x", ENOMEM));
	return (ENOMEM);
    }

    bp->scsi_command.scsi_id = sc_rd.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_READ;

    /* Set up the byte count for this command */
    bp->scsi_command.scsi_cmd.lun = (sc_rd.lun_id << 5) |
	((sc_rd.blkno >> 16) & 0x1f);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = (sc_rd.blkno >> 8) & 0xff;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = sc_rd.blkno & 0xff;
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 1;	/* single block */
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* 
     * Pass thru the no disconnect flag and the synch/asynch flag for proper 
     * data transfer to occur.
     */

    bp->scsi_command.flags = (sc_rd.flags & (SC_ASYNC | SC_NODISC));
    P8if(P8_ABORT, {
	if (sc_rd.flags & SC_ASYNC)
	    printf("readblk forces ASYNC\n");
    });

    bp->bufstruct.b_bcount = (unsigned) sc_rd.blklen;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = sc_rd.timeout_value;

    /* 
     * Set resume flag in case caller is retrying this operation.  This assumes
     * the readblk is only running single-threaded to this device.
     */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the readblk command. */
    if (p8xx_strategy(bp)) {
	/* an error was returned */
	(void) xmfree(bp->bufstruct.b_un.b_addr, pinned_heap);
	(void) xmfree(bp, pinned_heap); /* release sc_buf */
	P8Tprintf(("p8xxt: --> readblk: rc %x", EIO));
	return (EIO);
    }

    iowait((struct buf *) bp);	/* Wait for commmand completion */

    /* 
     * The return value from the operation is examined to determine what
     * type of error occurred.  Since the calling application requires an
     * ERRNO, this value is interpreted here.
     */
    if (bp->bufstruct.b_flags & B_ERROR) {
	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR) {
	    /* if adapter error */
	    switch (bp->general_card_status) {
		case SC_CMD_TIMEOUT:
		    p8xx_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
			0x46, 0, ap, NULL, FALSE);
		    ret_code = ETIMEDOUT;
		    break;
		case SC_NO_DEVICE_RESPONSE:
		    ret_code = ENODEV;
		    break;
		case SC_SCSI_BUS_FAULT:
		    ret_code = ENOCONNECT;
		    break;
		default:
		    ret_code = EIO;
		    break;
	    }
	} else
	    ret_code = EIO;
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) && (bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
	ret_code = EIO;
 
    /* No errors, so give the calling routine the data */
    if (ret_code == 0) {

	/* 
	 * Copy out the readblk data. If the buffer resides user space, use
	 * copyin, else use bcopy.
	 */
	if (!(devflag & DKERNEL)) {
	    ret_code = copyout(bp->bufstruct.b_un.b_addr,
			       sc_rd.data_ptr, sc_rd.blklen);
	    if (ret_code != 0)
		ret_code = EFAULT;
	} else {
	    /* buffer is in kernel space */
	    bcopy(bp->bufstruct.b_un.b_addr, sc_rd.data_ptr,
		  sc_rd.blklen);
	}
    }

    (void) xmfree(bp->bufstruct.b_un.b_addr, pinned_heap);
    (void) xmfree(bp, pinned_heap);	/* release sc_buf */
    P8printf(P8_EXIT,("Leaving p8xx_readblk routine.\n"));
    P8Tprintf(("p8xxt: --> readblk: rc %x", ret_code));
    return (ret_code);
}



/************************************************************************/
/*									*/
/* NAME:  p8xx_adp_str_init						*/
/*									*/
/* FUNCTION:  Initializes adapter stucture variables.			*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called by a process.			*/
/*									*/
/* NOTES:  This routine is called to initialize the adapter structure	*/
/*	   variables, arrays, etc.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t	*ap	address of adapter structure			*/
/*	struct intr *ip	address of adapter's interrupt structure	*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None								*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*									*/
/************************************************************************/
void
p8xx_adp_str_init(
	ads_t		*ap,
	struct intr	*ip)
{

    int	    i;

    P8printf(P8_ENTRY,("Entering p8xx_adp_str_init routine \n"));
    P8Tprintf(("p8xxt: --> adp_str_init: ip %x", ip));

    /* init the adapter interrupt handler struct */
    ip->next     = (struct intr *) NULL;
    ip->handler  = p8xx_intr;
    ip->bus_type = BUS_BID;		/* get bus type from the BID */
    ip->level    = ap->ddi.addi.int_lvl;
    ip->flags	 = ap->ddi.intr_flags;
    ip->priority = ap->ddi.addi.int_prior;
    ip->bid      = ap->bus_bid;
    P8printf(P8_INFO,("intr level=0x%02x priority=0x%02x bid=0x%08x\n",
	ip->level, ip->priority, ip->bid));
    P8Tprintf(("p8xxt: intr level=%02x priority=%02x bid=%08x",
	ip->level, ip->priority, ip->bid));

    P8if(P8_INFO_2, (p8xx_hexdump("struct intr", (char *) ip, 
			sizeof(struct intr))));

    /* initialize other basic global variables to the adp_str */
    ap->iowait_inited      = FALSE;
    ap->dump_started       = 0;

    /* Initialize the various pointers in the structure */
    ap->DEVICE_ACTIVE_head  = NULL;
    ap->DEVICE_ACTIVE_tail  = NULL;
    ap->DEVICE_WAITING_head = NULL;
    ap->DEVICE_WAITING_tail = NULL;
    ap->REQUEST_WFR_head = NULL;
    ap->REQUEST_WFR_tail = NULL;
    ap->blocked_bp = NULL;
    ap->ABORT_BDR_head      = NULL;
    ap->ABORT_BDR_tail      = NULL;

    for (i = 0; i < NUM_TAG; i++)
    {
        ap->command[i].tag = (unsigned char)i;
        ap->command[i].in_use = FALSE;
    }

    for (i = 0; i < TAG_TABLE_SIZE; i++)
        ap->TAG_alloc[i] = TAG_UNUSED;

    ap->IND_TABLE.system_ptr = NULL;
    ap->IND_TABLE.dma_ptr = NULL;
    ap->loc_tim_tbl.system_ptr = NULL;
    ap->loc_tim_tbl.dma_ptr = NULL;
    ap->sav_tim_tbl.system_ptr = NULL;
    ap->sav_tim_tbl.dma_ptr = NULL;

    for (i = 0; i < MAX_DEVICES; i++)
	ap->device_queue_hash[i] = NULL;

    for (i = 0; i < MAX_SIDS; i++)
    {
        ap->sid_info[i].bscript_ptr = NULL;
    }

    for (i = 0; i < IOVEC_TABLE_SIZE; i++)
        ap->iovec_alloc[i] = IOVEC_UNUSED;

    P8printf(P8_EXIT,("Leaving p8xx_adp_str_init routine\n"));
    P8Tprintf(("p8xxt: <-- adp_str_init"));
    return;
}



/**************************************************************************/
/*									  */
/* NAME:  p8xx_ioctl							  */
/*									  */
/* FUNCTION:  Scsi Chip Device Driver Ioctl Routine			  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can only be called by a process.			  */
/*									  */
/* NOTES:  This routine will accept commands to perform specific          */
/*	   functions.  Supported commands are:				  */
/*									  */
/*	   IOCINFO	- Returns information about the adapter.	  */
/*	   SCIOSTART	- Open a SCSI ID/LUN				  */
/*	   SCIOSTOP	- Close a SCSI ID/LUN.				  */
/*	   SCIOINQU	- Issues a SCSI Inquiry command to a device.	  */
/*	   SCIOSTUNIT	- Issues a SCSI start unit command to a device.	  */
/*	   SCIOTUR	- Issues a SCSI test unit ready command to device.*/
/*	   SCIOREAD	- Issues a SCSI 6-byte read command to device.	  */
/*	   SCIOHALT	- Issues a abort to SCSI device.		  */
/*	   SCIORESET	- Issues a Bus Device Reset to a SCSI device.	  */
/*	   SCIODIAG	- No operation.  Returns ENXIO.			  */
/*	   SCIOTRAM	- No operation.	 Returns ENXIO.			  */
/*	   SCIODNLD	- No operation.	 Returns ENXIO.			  */
/*	   SCIOSTARTTGT - No operation.	 Returns ENXIO.			  */
/*	   SCIOSTOPTGT	- No operation.	 Returns ENXIO.			  */
/*	   SCIOEVENT	- No operation.	 Returns ENXIO.			  */
/*	   SCIOGTHW	- No support for gathered writes.  Returns EINVAL.*/
/*									  */
/* DATA STRUCTURES:							  */
/*									  */
/* INPUTS:								  */
/*    devno  - major/minor number					  */
/*    cmd    - code used to determine which operation to perform.	  */
/*    arg    - address of a structure which contains values used in the	  */
/*	       'arg' operation.						  */
/*    devflag - & with D_KERNEL						  */
/*    chan   - not used (for multiplexed devices).			  */
/*    ext    - not used.						  */
/*									  */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*									  */
/* ERROR DESCRIPTION:							  */
/*      EIO - lock failed or adapter not initialized                      */
/*      EFAULT - failed copyout                                           */
/*      ENXIO - called no-op ioctl command                                */
/*      EINVAL - SCIOGTHW or unknown ioctl command                        */
/*									  */
/**************************************************************************/
int
p8xx_ioctl(
	dev_t	devno,		/* major and minor device numbers */
	int	cmd,		/* operation to perform */
	int	arg,		/* pointer to the user structure */
	ulong	devflag,	/* not used */
	int	chan,		/* not used */
	int	ext)		/* not used */

{
    ads_t		*ap;
    int			rc, ret_code = 0;
    struct devinfo	scinfo;

    P8_SETDBG(P8_IOCTL,P8_STD_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_ioctl routine.\n"));
    P8printf(P8_INFO_2,("devno=%x,cmd=%x,arg=%x,devflag=%x,chan=%x,ext=%x\n",
	     devno, cmd, arg, devflag, chan, ext));
    P8Tprintf(("p8xxt: --> ioctl: cmd %x arg %x",cmd,arg));


    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_IOCTL, ret_code, devno,
	    cmd, devflag, chan, ext);

    /* lock the global lock to serialize with open/close/config */
    rc = lockl(&(p8xx_lock), LOCK_SHORT);	 /* serialize this */
    if (rc != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
	P8Tprintf(("p8xxt: <-- ioctl: rc %x", EIO));
	P8_RSTDBG(P8_IOCTL);
	return (EIO);	/* error--kernel service call failed */
    }

    if ((ap = p8xx_get_adp(devno)) == NULL || !ap->str_inited || !ap->opened) {

	/* scsi chip has not been inited, defined, or opened */
	unlockl(&(p8xx_lock));
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
	P8Tprintf(("p8xxt: <-- ioctl: rc %x devno %x", EIO, devno));
	P8_RSTDBG(P8_IOCTL);
	return (EIO);
    }

    switch (cmd) {		/* cmd switch */

	case IOCINFO:		/* get device information */
	    scinfo.devtype = DD_BUS;
	    scinfo.flags = 0;
	    scinfo.devsubtype = DS_SCSI;
	    scinfo.un.scsi.card_scsi_id = (char) ap->ddi.addi.card_scsi_id;
	    scinfo.un.scsi.max_transfer = ap->max_request;
	    if (!(devflag & DKERNEL)) {  	/* for a user process */
		rc = copyout(&scinfo, (char *) arg, sizeof(struct devinfo));
		if (rc != 0)
		    ret_code = EFAULT;
	    } else {				/* for a kernel process */
		bcopy(&scinfo, (char *) arg, sizeof(struct devinfo));
	    }
	    break;

	case SCIOSTART:		/* start a device */
	    ret_code = p8xx_start_dev(ap, INDEX(arg >> 8, arg));
	    break;

	case SCIOSTOP:		/* stop a device */
	    ret_code = p8xx_stop_dev(ap, INDEX(arg >> 8, arg));
	    break;

	case SCIOHALT:		/* issue a SCSI abort cmd */
	    ret_code = p8xx_issue_abort(ap, INDEX(arg >> 8, arg));
	    break;

	case SCIORESET:		/* issue a SCSI abort cmd */
	    ret_code = p8xx_issue_BDR(ap, INDEX(arg >> 8, arg));
	    break;

	case SCIOINQU:		/* issue a SCSI device inquiry cmd */
	    ret_code = p8xx_inquiry(devno, arg, devflag);
	    break;

	case SCIOSTUNIT:	/* issue a SCSI device start unit */
	    ret_code = p8xx_start_unit(devno, arg, devflag);
	    break;

	case SCIOTUR:		/* issue  SCSI device test unit ready */
	    ret_code = p8xx_test_unit_rdy(devno, arg, devflag);
	    break;

	case SCIOREAD:		/* issue a SCSI read cmd (6-byte) */
	    ret_code = p8xx_readblk(devno, arg, devflag);
	    break;

	case SCIODIAG:		/* run adapter diagnostics command */
	case SCIOTRAM:		/* no-op, no chip ram to test */
	case SCIODNLD:		/* no-op, no microcode to download */
	case SCIOSTARTTGT:	/* no-op, target mode not supported */
	case SCIOSTOPTGT:	/* no-op, target mode not supported */
	case SCIOEVENT:		/* no-op, async event notification not
				   supported */
	    ret_code = ENXIO;
	    break;

	default:		/* catch unknown ioctls and SCIOGTHW here */
	    ret_code = EINVAL;
	    break;

    }	/* cmd switch */

    unlockl(&(p8xx_lock));
    P8printf(P8_EXIT,("Leaving p8xx_ioctl routine.\n"));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
    P8Tprintf(("p8xxt: <-- ioctl: rc %x devno %x", ret_code, devno));
    P8_RSTDBG(P8_IOCTL);
    return (ret_code);
}



/**************************************************************************/
/*									  */
/* NAME:	p8xx_build_command					  */
/*									  */
/* FUNCTION:	Builds an internal command for ioctl routines.		  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process.			  */
/*									  */
/* NOTES:  This routine is called to initialize fields within the sc_buf  */
/*	   structure that is allocated via this routine.  This routine    */
/*	   is called by ioctl routines that issue a command via the	  */
/*	   p8xx_strategy routine.					  */
/*									  */
/* DATA STRUCTURES:							  */
/*	sc_buf	- input/output request struct used between the adapter	  */
/*		  driver and the calling SCSI device driver		  */
/*									  */
/* INPUTS:								  */
/*	none								  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	returns a pointer to the sc_buf, or NULL, if it could not	  */
/*	be allocated.							  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*      NULL - was not able to allocate memory for bp                     */
/*									  */
/**************************************************************************/
struct sc_buf *
p8xx_build_command()
{
    struct sc_buf *bp;

    P8printf(P8_ENTRY,("Entering p8xx_build_command routine.\n"));
    P8Tprintf(("p8xxt: --> build_command"));

    /* Allocate a sc_buf area for this command	*/
    bp = (struct sc_buf *) xmalloc((uint) PAGESIZE, (uint) PGSHIFT,
	pinned_heap);
    if (bp == NULL) {
	/* xmalloc failed--return NULL pointer */
	P8Tprintf(("p8xxt: <-- build_command: returns %x", NULL));
	return (NULL);
    }

    /* 
     * Clear the sc_buf structure to insure all fields are initialized to zero.
     */
    bzero(bp, sizeof(struct sc_buf));

    /* Initialize other fields of the sc_buf.	*/
    bp->bufstruct.b_iodone = (void (*) ()) p8xx_iodone;
    bp->bufstruct.b_event = EVENT_NULL;
    bp->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
    bp->bufstruct.b_un.b_addr = (char *) bp + sizeof(struct sc_buf);

    /* Additional sc_buf initialization */
    bp->timeout_value = LONGWAIT;	/* set default timeout value */

    P8printf(P8_EXIT,("Leaving p8xx_build_command routine.\n"));
    P8Tprintf(("p8xxt: <-- build_command: returns %x", bp));
    return (bp);
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_script_init					*/
/*									*/
/* FUNCTION:	Adapter Script Initialization Routine			*/
/*	This function prepares an instance of the main script for use	*/
/*	with a target scsi device.  Jumps are initialized, buffers are  */
/*	patched with the correct values, etc.                           */
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	None.								*/
/*									*/
/* INPUTS:								*/
/*	*dev_ptr - address of target devices dev_info structure   	*/
/*	*iowait_vir_addr - the virtual address pointing to the		*/
/*		IO_WAIT script that all the scripts will be dependent	*/
/*		on as a router.						*/
/*	*script_vir_addr - the virtual address of the script just	*/
/*		created and copied into memory.	 The script that needs	*/
/*		to be initialized.					*/
/*	dev_info_hash - encoded scsi id/scsi lun value                	*/
/*	iowait_bus_addr - the bus address of the IO_WAIT script.	*/
/*	script_bus_addr - the bus address of the target script.		*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/*									*/
/************************************************************************/
void
p8xx_script_init(
	ads_t   *ap,
	dvi_t	*dev_ptr,
	ulong	*script_vir_addr,
	int	dev_info_hash,
	uint	iowait_bus_addr,
	uint	script_bus_addr)
{
    ulong	id_bit_mask;
    ulong	*target_script;
    ulong	word_buffer;
    uint	jump_address;
    int		i, scsi_id;

    P8printf(P8_ENTRY,("Entering p8xx_script_init routine\n"));
    P8Tprintf(("p8xxt: --> script_init: [virt] device %08x", script_vir_addr));
    P8Tprintf(("p8xxt:                  [bus]  iowait %08x device %08x",
	iowait_bus_addr, script_bus_addr));

    target_script = (ulong *) script_vir_addr;

    /*
     * We will patch all the absolute jump values with the virtual address
     * where we want the jump to go to.  We take the relative jump value
     * (in bytes) sitting at the jump address and add it to the base, virtual
     * address and write that back into the jump address location.  We must
     * be sure to convert between system and canonical bus byte order before
     * and after the addition of the base address.
     */

    for (i = 0; i < PATCHES; i++) {
	jump_address = CBOtoL(PSC_SCRIPT[LABELPATCHES[i]]);
	jump_address += script_bus_addr;
	target_script[LABELPATCHES[i]] = LtoCBO(jump_address);
    }

    /*
     * Patch references from the target's SCRIPTS back to the iowait
     * SCRIPT
     */
    for (i=0; i < S_COUNT(E_wait_reselect_Used); i++) {
	target_script[E_wait_reselect_Used[i]] =
	    LtoCBO(iowait_bus_addr + Ent_iowait_entry_point);
    }

    jump_address = script_bus_addr + (A_restore_patch_Used [0]*4);
    target_script[E_restore_patch_addr_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_tbl_addr1_ref_Used [0]*4);
    target_script[A_dptr_tbl_addr1_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_tbl_addr2_ref_Used [0]*4);
    target_script[A_dptr_tbl_addr2_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_cnt_addr1_ref_Used [0]*4);
    target_script[A_dptr_cnt_addr1_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_cnt_addr2_ref_Used [0]*4);
    target_script[A_dptr_cnt_addr2_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_cnt_addr3_ref_Used [0]*4);
    target_script[A_dptr_cnt_addr3_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_cnt_addr4_ref_Used [0]*4);
    target_script[A_dptr_cnt_addr4_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_cnt_addr5_ref_Used [0]*4);
    target_script[A_dptr_cnt_addr5_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_cur_sav_tbl_addr1_ref_Used [0]*4);
    target_script[A_cur_sav_tbl_addr1_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_cur_sav_tbl_addr2_ref_Used [0]*4);
    target_script[A_cur_sav_tbl_addr2_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_cur_loc_tbl_addr1_ref_Used [0]*4);
    target_script[A_cur_loc_tbl_addr1_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_cur_loc_tbl_addr2_ref_Used [0]*4);
    target_script[A_cur_loc_tbl_addr2_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_diff_addr1_ref_Used [0]*4);
    target_script[A_dptr_diff_addr1_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_diff_addr2_ref_Used [0]*4);
    target_script[A_dptr_diff_addr2_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_diff_addr3_ref_Used [0]*4);
    target_script[A_dptr_diff_addr3_Used[0]] = LtoCBO(jump_address);

    jump_address = script_bus_addr + (A_dptr_restore_addr_ref_Used [0]*4);
    target_script[A_dptr_restore_addr_Used[0]] = LtoCBO(jump_address);

    /*
     * BEGIN ID PATCHES
     */

    /*
     * We need to set the SCSI ID bits used in all the SELECT with ATN
     * commands to start the SCSI protocol with a target device.
     */
    scsi_id = SID(dev_info_hash);
    for (i=0; i < S_COUNT(R_target_id_Used); i++) {
	word_buffer = CBOtoL(target_script[R_target_id_Used[i]]);
	word_buffer = ((word_buffer & 0xFF00FFFF) | (scsi_id << 16));
	target_script[R_target_id_Used[i]] = LtoCBO(word_buffer);
    }

    /* patch the address of the table-indirect table (IND_TABLE) into */
    /* the appropriate script microcode instructions. */
    word_buffer = CBOtoL(target_script[A_NEXUS_data_base_adr0_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) |
                (((ulong) ap->IND_TABLE.dma_ptr & 0x000000FF) << 8));
    target_script[A_NEXUS_data_base_adr0_Used[0]] = LtoCBO(word_buffer);

    word_buffer = CBOtoL(target_script[A_NEXUS_data_base_adr1_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) |
                ((ulong) ap->IND_TABLE.dma_ptr & 0x0000FF00));
    target_script[A_NEXUS_data_base_adr1_Used[0]] = LtoCBO(word_buffer);

    word_buffer = CBOtoL(target_script[A_NEXUS_data_base_adr2_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) |
                (((ulong) ap->IND_TABLE.dma_ptr & 0x00FF0000) >> 8));
    target_script[A_NEXUS_data_base_adr2_Used[0]] = LtoCBO(word_buffer);

    word_buffer = CBOtoL(target_script[A_NEXUS_data_base_adr3_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) |
                (((ulong) ap->IND_TABLE.dma_ptr & 0xFF000000) >> 16));
    target_script[A_NEXUS_data_base_adr3_Used[0]] = LtoCBO(word_buffer);

    /* patch the address of the save tim table into */
    /* the appropriate script microcode instructions. */
    for (i=0; i < 2; i++) {
        word_buffer = CBOtoL(target_script[A_save_tbl_base_adr0_Used[i]]);
        word_buffer = ((word_buffer & 0xFFFF00FF) |
                (((ulong) ap->sav_tim_tbl.dma_ptr & 0x000000FF) << 8));
        target_script[A_save_tbl_base_adr0_Used[i]] = LtoCBO(word_buffer);

        word_buffer = CBOtoL(target_script[A_save_tbl_base_adr1_Used[i]]);
        word_buffer = ((word_buffer & 0xFFFF00FF) |
                ((ulong) ap->sav_tim_tbl.dma_ptr & 0x0000FF00));
        target_script[A_save_tbl_base_adr1_Used[i]] = LtoCBO(word_buffer);

        word_buffer = CBOtoL(target_script[A_save_tbl_base_adr2_Used[i]]);
        word_buffer = ((word_buffer & 0xFFFF00FF) |
                (((ulong) ap->sav_tim_tbl.dma_ptr & 0x00FF0000) >> 8));
        target_script[A_save_tbl_base_adr2_Used[i]] = LtoCBO(word_buffer);

        word_buffer = CBOtoL(target_script[A_save_tbl_base_adr3_Used[i]]);
        word_buffer = ((word_buffer & 0xFFFF00FF) |
                (((ulong) ap->sav_tim_tbl.dma_ptr & 0xFF000000) >> 16));
        target_script[A_save_tbl_base_adr3_Used[i]] = LtoCBO(word_buffer);
    }

    /*
     * point to the location in memory where our identify_msg_buf resides
     */
    target_script[E_identify_msg_addr_Used[0]] =
	LtoCBO(script_bus_addr + Ent_identify_msg_buf);

    /*
     * initialize the identify_msg_buffer to the lun id of the device
     * this script is associated with.	Bit 7 is set to show that it is	
     * an identify message.  Bit 6 is set to show we allow disconnections.
     * The lun pattern of (0-8) is held in Bits 2-0.
     */

    id_bit_mask = 0xC0000000;		/* allows the target to disconnect */

    P8Tprintf(("p8xxt: id_bit_mask %8.8X ap %x", id_bit_mask, ap));
    id_bit_mask |= ((ulong) LUN(dev_info_hash) << 24);
    target_script[S_INDEX(Ent_identify_msg_buf)] = id_bit_mask;
    target_script[S_INDEX(Ent_identify_msg_buf) + 1] = 0x00000000;


    /*
     * BEGIN BUFFER PATCHES
     */

    /* point to the location in memory where our cmd_msg_in_buf resides. */
    for (i = 0; i < sizeof(E_cmd_msg_in_addr_Used)/sizeof(ULONG); i++) {
	target_script[E_cmd_msg_in_addr_Used[i]] =
	    LtoCBO(script_bus_addr + Ent_cmd_msg_in_buf);
    }

    /* Clear out the message buffers used in synchronous negotiation. */
    target_script[S_INDEX(Ent_cmd_msg_in_buf)]   = 0x00000000;
    target_script[S_INDEX(Ent_cmd_msg_in_buf)+1] = 0x00000000;

    for (i = 0; i < sizeof(E_status_addr_Used)/sizeof(ULONG); i++) {
	target_script[E_status_addr_Used[i]] =
	    LtoCBO(script_bus_addr + Ent_status_buf);
    }

    /* Set the SCSI reject message id into the reject message buffer */
    for (i = 0; i < sizeof(E_reject_msg_addr_Used)/sizeof(ULONG); i++) {
        target_script[E_reject_msg_addr_Used[i]] =
	    LtoCBO(script_bus_addr + Ent_reject_msg_buf);
    }
    target_script[S_INDEX(Ent_reject_msg_buf)]   = 0x07000000;
    target_script[S_INDEX(Ent_reject_msg_buf)+1] = 0x00000000;

    /* patch address of q-tag buffer */
    target_script[E_tag_msg_addr_Used[0]] =
        LtoCBO(script_bus_addr + Ent_tag_msg_buf);
    target_script[S_INDEX(Ent_tag_msg_buf)] = 0x00000000;
    target_script[S_INDEX(Ent_tag_msg_buf) + 1] = 0x00000000;

    /* point to the location in memory where our negotiation out_buf reside. */
    target_script[E_neg_msg_addr_Used[0]] =
	LtoCBO(script_bus_addr + Ent_neg_msg_buf);

    target_script[E_wdtr_msg_out_addr_Used[0]] =
	LtoCBO(script_bus_addr + Ent_wdtr_msg_out_buf);

    target_script[E_sdtr_msg_out_addr_Used[0]] =
	LtoCBO(script_bus_addr + Ent_sdtr_msg_out_buf);

    /* 
     * Patch the messages needed to do negotiation.
     */

    id_bit_mask = 0xC0000000;
    id_bit_mask |= ((ulong) LUN(dev_info_hash) << 24);
    target_script[S_INDEX(Ent_neg_msg_buf)] = 
			id_bit_mask | (SCSI_WDTR_MSG >> 8);
    target_script[S_INDEX(Ent_neg_msg_buf) + 1] = 0x01000000; 

    /* patch the buffers used for target-initiated negotiation */
    target_script[S_INDEX(Ent_wdtr_msg_out_buf)] = SCSI_WDTR_MSG;
    target_script[S_INDEX(Ent_wdtr_msg_out_buf)+1] = SCSI_SDTR_MSG;
    target_script[S_INDEX(Ent_wdtr_msg_out_buf)+1] |= (ulong) ap->xfer_pd;
    target_script[S_INDEX(Ent_sdtr_msg_out_buf)] = (NCR8XX_DEF_OFFSET << 24);

    /* point to the location in memory where our cmd_buf resides. */
    target_script[E_cmd_addr_Used[0]] =
	LtoCBO(script_bus_addr + Ent_cmd_buf);

    /* Clear out the buffers to be used to hold the SCSI commands */
    target_script[S_INDEX(Ent_cmd_buf)]   = 0x00000000;
    target_script[S_INDEX(Ent_cmd_buf)+1] = 0x00000000;
    target_script[S_INDEX(Ent_cmd_buf)+2] = 0x00000000;
    target_script[S_INDEX(Ent_cmd_buf)+3] = 0x00000000;

    /* Clear out the status buffers */
    target_script[S_INDEX(Ent_status_buf)]   = 0x00000000;
    target_script[S_INDEX(Ent_status_buf)+1] = 0x00000000;

    /* point to the location in memory where our extended_msg_buf resides. */
    for (i = 0; i < sizeof(E_extended_msg_addr_Used)/
		    sizeof(E_extended_msg_addr_Used[0]); i++)
	target_script[E_extended_msg_addr_Used[i]] =
	    LtoCBO(script_bus_addr + Ent_extended_msg_buf);

    /* Clear out the extended msg buffers */
    target_script[S_INDEX(Ent_extended_msg_buf)]   = 0x00000000;
    target_script[S_INDEX(Ent_extended_msg_buf)+1] = 0x00000000;

    /* point to the location in memory where our abort_msg_out_buf resides */
    for (i = 0; i < S_COUNT(E_abort_bdr_msg_out_addr_Used); i++)
	target_script[E_abort_bdr_msg_out_addr_Used[i]] =
	    LtoCBO(script_bus_addr + Ent_abort_bdr_msg_out_buf);

    /* Clear out the abort_msg_out msg buffers and set the abort message  */
    target_script[S_INDEX(Ent_abort_bdr_msg_out_buf)]   = 0x06000000;
    target_script[S_INDEX(Ent_abort_bdr_msg_out_buf)+1] = 0x00000000;

    /* point to location in memory where our abort_bdr_msg_in_buf resides. */
    for (i = 0; i < S_COUNT(E_abort_bdr_msg_in_addr_Used); i++)
	target_script[E_abort_bdr_msg_in_addr_Used[i]] =
	    LtoCBO(script_bus_addr + Ent_abort_bdr_msg_in_buf);

    /* Clear out the abort_msg_in msg buffers */
    target_script[S_INDEX(Ent_abort_bdr_msg_in_buf)]   = 0x00000000;
    target_script[S_INDEX(Ent_abort_bdr_msg_in_buf)+1] = 0x00000000;

    if (dev_ptr != NULL) {
	/* 
	 * Setup the DSA and SCRATCHB register addresses in the scripts.
	 * Note that these are I/O space (bus) addresses as seen by the chip
	 * and hence the msb *must* not be set.
	 */
	for (i=0; i < S_COUNT(E_DSA_Addr_Used); i++)
	    target_script[E_DSA_Addr_Used[i]] =
		LtoCBO(ap->base_addr0 + DSA);

	for (i=0; i < S_COUNT(E_SCRATCHA0_addr_Used); i++)
	    target_script[E_SCRATCHA0_addr_Used[i]] =
		LtoCBO(ap->base_addr0 + SCRATCHA0);

	for (i=0; i < S_COUNT(E_SCRATCHB_addr_Used); i++)
	    target_script[E_SCRATCHB_addr_Used[i]] =
		LtoCBO(ap->base_addr0 + SCRATCHB);

    }

    P8if(P8_DUMP_SCRIPTS_1, {
	if (ap) {
	    p8xx_dump_script(script_vir_addr, sizeof(PSC_SCRIPT)/sizeof(ULONG));
	}
    });
    P8printf(P8_EXIT,("Leaving p8xx_script_init.\n"));
    P8Tprintf(("p8xxt: <-- script_init ap %x", ap));
    return;
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_iowait_script_init					*/
/*									*/
/* FUNCTION:	Adapter Script Initialization Routine			*/
/*	This function patches the iowait script.  The targets of jumps  */
/*	within the script are set.  Similarly, the destination address  */
/*	for the identify message following reselection is patched.      */
/*	The adapter's scsi id is patched so that the script will not    */
/*      accept reselection from that id.				*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	None.								*/
/*									*/
/* INPUTS:								*/
/*	*iowait_vir_addr - the virtual address pointing to the		*/
/*		IO_WAIT script that all the scripts will be dependent	*/
/*		on as a router.						*/
/*	iowait_bus_addr - the bus address of the IO_WAIT script.	*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/*									*/
/************************************************************************/
void
p8xx_iowait_script_init(
	ads_t   *ap,
	ulong	*iowait_vir_addr,
	uint	iowait_bus_addr)
{
    ulong	id_bit_mask;
    ulong	word_buffer;
    uint	jump_address;
    int		i, scsi_id;

    P8printf(P8_ENTRY,("Entering p8xx_iowait_script_init routine\n"));
    P8Tprintf(("p8xxt: --> iowait_init: [virt] %08x [bus] %08x",
	iowait_vir_addr, iowait_bus_addr));

    /*
     * We will patch all the absolute jump values with the virtual address
     * where we want the jump to go to.  We take the relative jump value
     * (in bytes) sitting at the jump address and add it to the base, virtual
     * address and write that back into the jump address location.  We must
     * be sure to convert between system and canonical bus byte order before
     * and after the addition of the base address.
     */

    /* Patch the jump values for the IOWAIT script */
    for (i = 0; i < IOWAIT_PATCHES; i++) {
	jump_address = CBOtoL(IOWAIT_SCRIPT[IOWAIT_LABELPATCHES[i]]);
	jump_address += iowait_bus_addr;
	iowait_vir_addr[IOWAIT_LABELPATCHES[i]] = LtoCBO(jump_address);
    }

    /*
     * Patch the interrupt for the case where we have been reselected by
     * a target but we don't have a valid, set jump point.  We only do this
     * once, at the first open of the adapter.
     */

    if (ap && !ap->iowait_inited) {

        /*
         * Using the initiator's SCSI ID, index into the jump table at the
         * scsi_id_patch entry and alter (patch) it into an invalid SCSI ID
         * (a 0xFF).  That way, if a target ever selects us with this id,
         * we will fall into the unknown_reselect_id interrupt at the bottom
         * of the table.
         */
        i = S_INDEX(Ent_scsi_id_patch) + ap->ddi.addi.card_scsi_id * 2;
        iowait_vir_addr[i] = LtoCBO(0x800C00FF);

        for (i = 0; i < MAX_LUNS; i++) {
            iowait_vir_addr[E_scsi_0_lun_Used[i]] =
    	        LtoCBO(((A_uninitialized_reselect_Used[0] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_1_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[1] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_2_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[2] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_3_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[3] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_4_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[4] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_5_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[5] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_6_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[6] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_7_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[7] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_8_lun_Used[i]] =
    	        LtoCBO(((A_uninitialized_reselect_Used[8] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_9_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[9] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_A_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[10] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_B_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[11] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_C_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[12] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_D_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[13] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_E_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[14] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
            iowait_vir_addr[E_scsi_F_lun_Used[i]] =
	        LtoCBO(((A_uninitialized_reselect_Used[15] - 1) * 
		sizeof(ULONG) + iowait_bus_addr));
        }

        for (i = 0; i < MAX_SIDS; i++) {
            iowait_vir_addr[E_lun_msg_addr_Used[i]] =
	        LtoCBO(iowait_bus_addr + Ent_lun_msg_buf);
	}

        iowait_vir_addr[S_INDEX(Ent_lun_msg_buf)]   = 0x00000000;
        iowait_vir_addr[S_INDEX(Ent_lun_msg_buf)+1] = 0x00000000;

        ap->iowait_inited = TRUE; /* only do this once - ever */
    } 

    P8if(P8_DUMP_SCRIPTS_1, {
	if (ap) {
	    p8xx_dump_script(iowait_vir_addr, sizeof(IOWAIT_SCRIPT)/sizeof(ULONG));
	}
    });
    P8printf(P8_EXIT,("Leaving p8xx_iowait_script_init.\n"));
    P8Tprintf(("p8xxt: <-- iowait_script_init ap %x", ap));
}
/************************************************************************/
/*									*/
/* NAME:	p8xx_start_dev						*/
/*									*/
/* FUNCTION:  Allocates resources and starts a device			*/
/*									*/
/*	This routine initializes the device queue to prepare for	*/
/*	command processing.						*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called by a process.			*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of adapter structure			*/
/*	int dev_index	index to device information			*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	0 - for good completion, ERRNO value otherwise.			*/
/*									*/
/* ERROR DESCRIPTION							*/
/*	EINVAL - device not opened					*/
/*	ENOMEM - unable to xmalloc memory				*/
/*									*/
/************************************************************************/
int
p8xx_start_dev(
	ads_t	*ap,
	int	dev_index)
{

    int			i, rc = 0;
    struct dev_info	*dev_ptr = NULL;
    sid_t		*sid_ptr;
    ulong		*wptr;

    P8_SETDBG(P8_START_DEV,P8_FLW_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_start_dev routine, dev_index=%x\n",
	dev_index));
    P8Tprintf(("p8xxt: --> start_dev: dev_index %x SCSI id %x ap %x",
	dev_index, SID(dev_index), ap));

    if (ap->device_queue_hash[dev_index] != NULL) {
	/* Device queue structure already allocated */
	rc = EINVAL;
	goto exit_on_error;
    }

    if (ap->ddi.addi.card_scsi_id == SID(dev_index)) {
	/* Device SCSI ID matches adapter SCSI ID.  */
	rc = EINVAL;
	goto exit_on_error;
    }

    /* device queue structure doesn't exist yet */
    ap->device_queue_hash[dev_index] =
	xmalloc((uint) sizeof(dvi_t), 4, pinned_heap);
    if (ap->device_queue_hash[dev_index] == NULL) {
	/* malloc failed */
	rc = ENOMEM;
	goto exit_on_error;
    }
    dev_ptr = ap->device_queue_hash[dev_index];
    bzero(dev_ptr, (int) sizeof(dvi_t));
    dev_ptr->ap = ap;		/* it points back to our adapter struct */
    dev_ptr->dev_watchdog.ap = ap; /* so does the watchdog timer struct */

    /* 
     * Malloc 4K area for this device's TIM & SCRIPTS
     */

    sid_ptr = &ap->sid_info[SID(dev_index)];
    if (sid_ptr->vscript_ptr == NULL) {
        /* no script allocated for this target id yet,
	 * need to allocate and map a new page
	 */
        sid_ptr->vscript_ptr = 
	    (ulong *) xmalloc((uint) PAGESIZE, (uint) PGSHIFT, pinned_heap);

    	if (sid_ptr->vscript_ptr == NULL) {
	    rc = ENOMEM;
	    goto exit_on_error;
    	}

	/* map the page and copy in the script */
    	if (D_MAP_PAGE(ap->handle, DMA_READ|DMA_BYPASS, sid_ptr->vscript_ptr, 
		&sid_ptr->bscript_ptr, xmem_global)) {
	    rc = ENOMEM;
	    goto exit_on_error;
    	}
    
	/* Copy template scripts to the scripts area */
	/* First, make certain scripts will fit */
	assert(sizeof(PSC_SCRIPT) <= PAGESIZE);

	for (i = 0; i < sizeof(PSC_SCRIPT)/sizeof(ULONG); i++) {
	    sid_ptr->vscript_ptr[i] = PSC_SCRIPT[i];
	}
	P8if(P8_DUMP_SCRIPTS_1,(p8xx_dump_script(sid_ptr->vscript_ptr,
	    sizeof(PSC_SCRIPT)/sizeof(ULONG))));

	/* initialize the sid_info structure */
        sid_ptr->scsi_id = SID(dev_index);
        sid_ptr->async_device = FALSE;
        sid_ptr->restart_in_prog = FALSE;
        sid_ptr->disconnect_flag = FALSE;
        sid_ptr->tag_flag = ap->ddi.addi.card_scsi_id << 3; /* init. to a */
                                                  /* value never used. */
        sid_ptr->tag_msg_flag = 0xFF;
        sid_ptr->lun_flag = 0xFF;
        sid_ptr->bdring_lun = NULL;

        /* Call the setup routine for this script entry */
        p8xx_script_init(ap, dev_ptr,
		     sid_ptr->vscript_ptr,
		     dev_index,
		     ap->Scripts.bscript_ptr,
		     sid_ptr->bscript_ptr);

	/*
	 * Set negotiate flag and the io_table.  Patch the script 
	 * if negotiation is not needed. Note the NEGOT_DONE flag is not
	 * maintained within the sid_info.io_table.flags variable, so we
	 * make sure it is not set to avoid confusion.
         */
	sid_ptr->io_table.flags = ap->device_state[sid_ptr->scsi_id].flags &
		~NEGOT_DONE;
        if (!(ap->device_state[sid_ptr->scsi_id].flags & NEGOT_DONE)) {
    	    P8printf(P8_SYNC,("0 setting negotiate_flag for ID %d\n",
	        sid_ptr->scsi_id));
	    sid_ptr->negotiate_flag = TRUE;
        } else {
	    /*
	     * Re-set our previously negotiated values
	     */
	    P8printf(P8_SYNC,("0 restoring sync xfer values for ID %d\n",
	        sid_ptr->scsi_id));
	    sid_ptr->negotiate_flag  = FALSE;
	    sid_ptr->io_table.sxfer  = ap->device_state[sid_ptr->scsi_id].sxfer;
	    sid_ptr->io_table.scntl3 = ap->device_state[sid_ptr->scsi_id].scntl3;
            dev_ptr->sid_ptr = sid_ptr;
	    p8xx_do_patch(dev_ptr, sid_ptr->io_table.sxfer,
	        sid_ptr->io_table.scntl3); 

	    p8xx_wide_patch(dev_ptr, TRUE);
        }
    }

    dev_ptr->sid_ptr = sid_ptr;
 	    
    /* Initialize device queue flags. */
    dev_ptr->DEVICE_ACTIVE_fwd	 = NULL;
    dev_ptr->DEVICE_ACTIVE_bkwd	 = NULL;
    dev_ptr->DEVICE_WAITING_fwd	 = NULL;
    dev_ptr->ABORT_BDR_fwd	 = NULL;
    dev_ptr->ABORT_BDR_bkwd	 = NULL;
    dev_ptr->active_head = NULL;
    dev_ptr->active_tail = NULL;
    dev_ptr->wait_head = NULL;
    dev_ptr->wait_tail = NULL;
    dev_ptr->cmd_save_head = NULL;
    dev_ptr->cmd_save_tail = NULL;
    dev_ptr->bp_save_head = NULL;
    dev_ptr->bp_save_tail = NULL;

    dev_ptr->scsi_id		 = SID(dev_index);
    dev_ptr->lun_id		 = LUN(dev_index);

    dev_ptr->ioctl_wakeup	 = FALSE;
    dev_ptr->ioctl_event	 = EVENT_NULL;
    dev_ptr->stop_event		 = EVENT_NULL;
    dev_ptr->flags		 = RETRY_ERROR;

    /* init watchdog timer struct */
    dev_ptr->dev_watchdog.dog.next	 = NULL;
    dev_ptr->dev_watchdog.dog.prev	 = NULL;
    dev_ptr->dev_watchdog.dog.func	 = p8xx_watchdog;
    dev_ptr->dev_watchdog.dog.count	 = 0;
    dev_ptr->dev_watchdog.dog.restart	 = 0;
    dev_ptr->dev_watchdog.timer_id	 = PSC_COMMAND_TMR;
    w_init(&(dev_ptr->dev_watchdog.dog));

    /* init command state flags */
    dev_ptr->queue_state	= ACTIVE;
    dev_ptr->opened		= TRUE;
    dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;

exit_on_error:
    if (rc != 0) {		/* an error encountered? */
	if (dev_ptr != NULL) {
	    ap->device_queue_hash[dev_index] = NULL;
	    (void) xmfree(dev_ptr, pinned_heap);
	}
    } else {
	P8Tprintf(("p8xxt: device %x started (initialized)",
	    dev_ptr->scsi_id));
    }

    P8printf(P8_EXIT,("Leaving p8xx_start_dev routine, rc=%d\n",rc));
    P8Tprintf(("p8xxt: <-- start_dev: rc %x", rc));
    P8_RSTDBG(P8_START_DEV);
    return (rc);
}

/************************************************************************/
/*									*/
/* NAME:  p8xx_stop_dev							*/
/*									*/
/* FUNCTION:  Stops a device and deallocates resources.			*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*									*/
/* NOTES:  This routine is called to stop a device.  Any further	*/
/*	command processing for the device will be halted from this	*/
/*	point on.							*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of adapter structure			*/
/*	int dev_index	index to device information			*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	0 - for good completion, ERRNO value otherwise.			*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*    EINVAL - device not opened.					*/
/*									*/
/************************************************************************/
int
p8xx_stop_dev(
	ads_t	*ap,
	int	dev_index)
{
    int		i, id_hash, old_level, rc = 0;
    uchar 	last_lun;
    dvi_t	*dev_ptr;

    P8_SETDBG(P8_START_DEV,P8_FLW_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_stop_dev routine, dev_index=%x\n",
	dev_index));
    P8Tprintf(("p8xxt: --> stop_dev: dev_index %x ap %x", dev_index, ap));

    if (ap->device_queue_hash[dev_index] == NULL) {
	/* Device queue structure not allocated */
	rc = EINVAL;
	goto exit_on_error;
    }

    /* 
     * Obtain device structure pointer and disable interrupts for processing.
     */
    dev_ptr = ap->device_queue_hash[dev_index];
    old_level = i_disable(ap->ddi.addi.int_prior);
    dev_ptr->queue_state |= STOPPING;

    /* Search all the queues of this device to determine if there are */
    /* any commands outstanding.  If so, wait for them to complete.   */
    while ((dev_ptr->active_head != NULL) || (dev_ptr->wait_head != NULL)
        || (dev_ptr->flags & BLOCKED) || (dev_ptr->cmd_save_head != NULL) ||
           (dev_ptr->bp_save_head != NULL)) {
	e_sleep(&dev_ptr->stop_event, EVENT_SHORT);
    }

    i_enable(old_level);	/* let interrupts in */

    /* Free the device's script entry for other   */
    /* use, if all other LUNs are closed.         */
    dev_ptr->opened = FALSE;
    last_lun = TRUE;

    id_hash = (dev_ptr->scsi_id & 0x0F) << 3;
    i = 0;
    while ((i < MAX_LUNS) && (last_lun == TRUE)) {
        if ((ap->device_queue_hash[id_hash | i] != NULL) &&
            (ap->device_queue_hash[id_hash | i]->opened))
            last_lun = FALSE;
        i++;
    }

    if (last_lun) {
        sid_t *scsi_ptr;

        scsi_ptr = dev_ptr->sid_ptr;
        /* 
         * Free the device's SCRIPTS workarea
         */
        if (scsi_ptr->vscript_ptr != NULL) {
	    (void) xmfree(scsi_ptr->vscript_ptr, pinned_heap);
	    scsi_ptr->vscript_ptr = NULL;
	    scsi_ptr->bscript_ptr = 0;
        }

        /*
         * Remember if we have done sync negotiation with this device
         */
        if (scsi_ptr->negotiate_flag) {
	    ap->device_state[scsi_ptr->scsi_id].flags &= ~NEGOT_DONE;
        } else {
	    ap->device_state[scsi_ptr->scsi_id].sxfer  = scsi_ptr->io_table.sxfer;
	    ap->device_state[scsi_ptr->scsi_id].scntl3 = scsi_ptr->io_table.scntl3;
	    ap->device_state[scsi_ptr->scsi_id].flags  = scsi_ptr->io_table.flags;
	    ap->device_state[scsi_ptr->scsi_id].flags |=  NEGOT_DONE;
        }
    }


    /* Free the device's watchdog timer entry.	 */
    w_clear(&(dev_ptr->dev_watchdog.dog));

    /* Free the device information table and clear the hash table entry. */
    ap->device_queue_hash[dev_index] = NULL;
    (void) xmfree(dev_ptr, pinned_heap);

exit_on_error:
    P8printf(P8_EXIT,("Leaving p8xx_stop_dev routine, rc=%d.\n", rc));
    P8Tprintf(("p8xxt: <-- stop_dev: rc %x", rc));
    P8_RSTDBG(P8_START_DEV);
    return (rc);
}

/************************************************************************/
/*									*/
/* NAME:    p8xx_issue_abort						*/
/*									*/
/* FUNCTION:  Issues a SCSI abort to a device.				*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*									*/
/* NOTES:  This routine causes an abort command to be issued to a SCSI	*/
/*	device.  Note that this action will also halt the device	*/
/*	queue.  The calling process is responsible for NOT calling this	*/
/*	routine if the SCIOSTART failed.  Such a failure would indicate	*/
/*	that another process has this device open and interference	*/
/*	could cause improper error reporting.				*/
/*									*/
/*									*/
/* DATA STRUCTURES:							*/
/*    scsi chip structure - scsi chip information.			*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of adapter structure			*/
/*	int dev_index	index to device information			*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*    0 - for good completion, ERRNO value otherwise			*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*    EINVAL	- device not opened.					*/
/*    ETIMEDOUT - Command timed out.					*/
/*    ENOCONNECT- SCSI bus fault.					*/
/*    EIO	- Adapter error                       			*/
/*									*/
/************************************************************************/
int
p8xx_issue_abort(
	ads_t	*ap,
	int	dev_index)
{
    dvi_t		*dev_ptr;
    int			old_pri1;
    int			ret_code = 0;

    P8printf(P8_ENTRY,("Entering p8xx_issue_abort routine\n"));
    P8Tprintf(("p8xxt: --> issue_abort: dev_index %x ap %x",
	dev_index, ap));

    old_pri1 = i_disable(ap->ddi.addi.int_prior);

    if ((dev_ptr = ap->device_queue_hash[dev_index]) == NULL) {
	i_enable(old_pri1);			/* re-enable */
	P8Tprintf(("p8xxt: <-- issue_abort: rc %x", EINVAL));
	return (EINVAL);
    }

    TRACE_1("abort ioctl", (int) dev_ptr)

    P8printf(P8_ABORT,("Issue abort for ID 0x%02x LUN 0x%02x\n",
	dev_ptr->scsi_id, dev_ptr->lun_id));

    if (dev_ptr->queue_state & STOPPING_or_HALTED_or_FLUSHING)
    {
        i_enable(old_pri1);     /* re-enable */
        return (EIO);
    }

    /* reject for EPOW or Power Mgmt Suspend/Hibernate */
    if (ap->power_state != P8XX_NORMAL_PWR) {
        i_enable(old_pri1);     /* re-enable */
        return (EIO);
    }
	
    /* Handle if device is already being BDRed or Aborted. */
    if (dev_ptr->flags & SCSI_BDR_or_ABORT)
    {
        i_enable(old_pri1);
        return (EIO);
    }

    /* Handle if there are active commands for the lun */
    if (dev_ptr->active_head != NULL)
    {
        dev_ptr->flags |= SCSI_ABORT;
        p8xx_enq_abort_bdr(dev_ptr);

        /* stop chip if it is waiting for reselection */
	SUSPEND_CHIP(ap);

        dev_ptr->ioctl_wakeup = TRUE;
        while (dev_ptr->ioctl_wakeup)
            e_sleep(&dev_ptr->ioctl_event, EVENT_SHORT);
        ret_code = dev_ptr->ioctl_errno;
    }
    else /* just return all the pended commands */
    {
        p8xx_fail_cmd(dev_ptr);
    }

    P8printf(P8_EXIT,("Leaving p8xx_issue_abort routine. rc=0x%x\n", ret_code));
    i_enable(old_pri1); /* re-enable */
    return (ret_code);  /* no error */
}  /* p8xx_issue_abort  */

/************************************************************************/
/*									*/
/* NAME:    p8xx_issue_BDR						*/
/*									*/
/* FUNCTION:  Issues a Bus Device Reset to a SCSI device.		*/
/*									*/
/*	This performs actions necessary to send a SCSI bus		*/
/*	device reset message to the scsi controller.			*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*									*/
/* NOTES:								*/
/*	This routine causes a reset command to be issued to a SCSI	*/
/*	device.  Note that this action will also halt the device queue.	*/
/*									*/
/*	The calling process is responsible for NOT calling this routine	*/
/*	if the SCIOSTART failed.  Such a failure would indicate that	*/
/*	another process has this device open and interference could	*/
/*	cause improper error reporting.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*    adapter_def - adapter unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of adapter structure			*/
/*	int dev_index	index to device information			*/
/*								 	*/
/* RETURN VALUE DESCRIPTION:						*/
/*    0 - for good completion, ERRNO value otherwise			*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*    EINVAL	- device not opened.					*/
/*    ETIMEDOUT - Command timed out.					*/
/*    ENOCONNECT- No connection.					*/
/*    EIO	- Adapter error                       			*/
/*									*/
/************************************************************************/
int
p8xx_issue_BDR(
	ads_t	*ap,
	int	dev_index)
{
    dvi_t	*dev_ptr, *tmp_dev_ptr;
    cmd_t	*com_ptr;
    int		old_pri1;
    int		i, ret_code;
    uchar	istat_val;
    caddr_t	bus_addr;
    int		rc;


    P8printf(P8_ENTRY,("Entering p8xx_issue_BDR routine.\n"));
    P8Tprintf(("p8xxt: --> issue_BDR: dev_index %x ap %x", dev_index, ap));

    /* 
     * Obtain device structure pointer and disable interrupts for processing.
     */

    old_pri1 = i_disable(ap->ddi.addi.int_prior);

    if ((dev_ptr = ap->device_queue_hash[dev_index]) == NULL) {
	i_enable(old_pri1);	/* re-enable */
	P8Tprintf(("p8xxt: <-- issue_BDR: rc %x", EINVAL));
	return (EINVAL);
    }

    TRACE_1("BDR ioctl  ", (int) dev_ptr)

    P8printf(P8_ABORT,("Issue BDR for ID 0x%02x LUN 0x%02x\n",
	dev_ptr->scsi_id, dev_ptr->lun_id));

    if (dev_ptr->queue_state & STOPPING_or_HALTED_or_FLUSHING) {
        i_enable(old_pri1);     /* re-enable */
        return (EIO);
    }

    /* reject for EPOW or Power Mgmt Suspend/Hibernate */
    if (ap->power_state != P8XX_NORMAL_PWR) {
        i_enable(old_pri1);     /* re-enable */
        return (EIO);
    }

    /* Handle if device is already being BDRed. */
    if (dev_ptr->flags & SCSI_BDR) {
        i_enable(old_pri1);
        return (EIO);
    }

    /* We are going to issue the BDR.  Mark all the luns of the target */
    /* SCSI id, indicating that a BDR is going to be issued.        */
    dev_index &= 0xF8;
    for (i = 0; i< MAX_LUNS; i++)
    {
        if ((tmp_dev_ptr = ap->device_queue_hash[dev_index + i]) != NULL)
        {
            tmp_dev_ptr->flags |= SCSI_BDR;
            tmp_dev_ptr->queue_state &= STOPPING;
            tmp_dev_ptr->queue_state |= WAIT_FLUSH;
        }
    }

    /* Indicate the BDR is based on this dev_ptr */
    dev_ptr->sid_ptr->bdring_lun = dev_ptr;

    /* Make sure the chip is powered on */
    if (ap->pmh.mode == PM_DEVICE_IDLE) 
        p8xx_turn_chip_on(ap, TRUE);

    /* issue the command if the chip is idle */
    if (ap->DEVICE_ACTIVE_head == NULL)
    {
        /* start timer */
        dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
        W_START(&dev_ptr->dev_watchdog.dog);

        dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;
        dev_ptr->flags &= ~RETRY_ERROR;
        dev_ptr->retry_count = 1;

	/* update the power mgmt. activity field */
	ap->pmh.activity = PM_ACTIVITY_OCCURRED;

        com_ptr = &ap->command[ap->ddi.addi.card_scsi_id << 3];

        com_ptr->bp = NULL;
        com_ptr->device = dev_ptr;
        com_ptr->resource_state = NO_RESOURCES_USED;
        com_ptr->in_use = TRUE;

        p8xx_enq_active(dev_ptr, com_ptr);

	/* Issue the cmd */
	p8xx_issue_bdr_script(dev_ptr, com_ptr, FALSE);

    } else {
	p8xx_enq_abort_bdr(dev_ptr);

        TRACE_1 ("sigp bdr", (int) dev_ptr)
        /* stop chip if it is waiting for reselection */
	SUSPEND_CHIP(ap);

    }

    dev_ptr->ioctl_wakeup = TRUE;
    while (dev_ptr->ioctl_wakeup)
	e_sleep(&dev_ptr->ioctl_event, EVENT_SHORT);

    ret_code = dev_ptr->ioctl_errno;

    i_enable(old_pri1); /* re-enable */

    P8printf(P8_EXIT,("Leaving p8xx_issue_BDR routine. rc=0x%x\n", ret_code));
    P8Tprintf(("p8xxt: <-- issue_BDR: rc %x", ret_code));
    return (ret_code);

}  /* p8xx_issue_BDR */
