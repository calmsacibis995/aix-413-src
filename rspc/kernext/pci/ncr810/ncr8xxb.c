static char sccsid[] = "@(#)38  1.10  src/rspc/kernext/pci/ncr810/ncr8xxb.c, pciscsi, rspc41J, 9520A_all 5/16/95 12:14:56";
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: 
 * 		p8xx_abdr
 *		p8xx_abdr_sel_fail
 *		p8xx_abort_current_command
 *		p8xx_alloc_iovec
 *		p8xx_alloc_resources
 *		p8xx_alloc_tag
 *		p8xx_bdr_done
 *		p8xx_calc_resid
 *		p8xx_cdt_func
 *		p8xx_check_wfr_queue
 *		p8xx_chip_register_init
 *		p8xx_chip_register_reset
 *		p8xx_cleanup_reset
 *		p8xx_command_reset_scsi_bus
 *		p8xx_config_adapter
 *		p8xx_deq_abort_bdr
 *		p8xx_deq_active
 *		p8xx_deq_wait
 *		p8xx_deq_wfr
 *		p8xx_do_patch
 *		p8xx_dump
 *		p8xx_dump_dev
 *		p8xx_dumpend
 *		p8xx_dump_intr
 *		p8xx_dump_script
 *		p8xx_dumpstrt
 *		p8xx_dumpwrt
 *		p8xx_empty_cmd_save
 *		p8xx_enq_abort_bdr
 *		p8xx_enq_active
 *		p8xx_enq_wait
 *		p8xx_enq_wfr
 *		p8xx_epow
 *		p8xx_fail_cmd
 *		p8xx_freeze_qs
 *		p8xx_free_iovec
 *		p8xx_free_resources
 *		p8xx_handle_extended_messages
 *		p8xx_ignore_residue
 *		p8xx_intr
 *		p8xx_iodone
 *		p8xx_issue_abort_bdr
 *		p8xx_issue_abort_script
 *		p8xx_issue_bdr_script
 *		p8xx_issue_cmd
 *		p8xx_logerr
 *		p8xx_message_out
 *		p8xx_mode_patch
 *		p8xx_patch_tag_changes
 *		p8xx_phase_mismatch
 *		p8xx_pm_handler
 *		p8xx_poll_for_bus_reset
 *		p8xx_prepare
 *		p8xx_prep_main_script
 *		p8xx_process_bp_save
 *		p8xx_process_q_clr
 *		p8xx_q_full
 *		p8xx_read_cfg
 *		p8xx_read_reg
 *		p8xx_recover_byte
 *		p8xx_reset_iowait_jump
 *		p8xx_restore_iowait_jump
 *		p8xx_rpt_status
 *		p8xx_save_residue
 *		p8xx_scsi_interrupt
 *		p8xx_scsi_reset_received
 *		p8xx_sdtr_answer
 *		p8xx_sdtr_neg_done
 *		p8xx_sdtr_reject
 *		p8xx_sel_glitch
 *		p8xx_set_disconnect
 *		p8xx_sip
 *		p8xx_sir_abort_error
 *		p8xx_start
 *		p8xx_start_chip
 *		p8xx_strategy
 *		p8xx_turn_chip_on
 *		p8xx_trace_1
 *		p8xx_trace_2
 *		p8xx_trace_3
 *		p8xx_unfreeze_qs
 *		p8xx_update_dataptrs
 *		p8xx_verify_tag
 *		p8xx_watchdog
 *		p8xx_wdtr_answer
 *		p8xx_wdtr_reject
 *		p8xx_wide_patch
 *		p8xx_write_cfg
 *		p8xx_write_reg
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
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
/* NAME:	ncr8xxb.c						*/
/*									*/
/* FUNCTION:	IBM SCSI Adapter Driver Bottom Half Source File		*/
/*									*/
/*	This adapter driver is the interface between a SCSI device	*/
/*	driver and the actual SCSI adapter.  It executes commands	*/
/*	from multiple drivers which contain generic SCSI device		*/
/*	commands, and manages the execution of those commands.		*/
/*	Several ioctls are defined to provide for system management.	*/
/*									*/
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
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
#include <sys/syspest.h>
#include <sys/vmuser.h>
#include <sys/mdio.h>
#include <sys/pm.h>
#include "ncr8xx.h"
/* END OF INCLUDED SYSTEM FILES	 */

/************************************************************************/
/* Global pinned device driver static data areas			*/
/************************************************************************/

extern hash_t	hash_list[ADS_HASH_MASK+1];

/* global pointer table for adapter structures */
int     cfg_adp_cnt = 0;                /* count of configured adapters */
int	adp_cnt = 0;			/* count of open adapters */
struct p8xx_cdt_table *p8xx_cdtp = NULL; /* ptr to driver's global component
					   dump table */
#ifdef	DEBUG_8XX
int	p8xx_debug = 0x40000;    	/* controls debug msgs */
int	p8xx_dbsv[P8XX_SVMAX];		/* save areas for debug flags */
int	p8xx_dbidx = 0;			/* current save area index */
#endif

/*
 * SCRIPTS compiler output included here
 */

#include "ncr8xxbss.h"


/************************************************************************/
/*									*/
/* NAME:  p8xx_chip_register_init					*/
/*									*/
/* FUNCTION:  Initializes adapter chip registers.			*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*									*/
/* NOTES:								*/
/*	This routine is called to initialize the adapter chip registers	*/
/*	before command processing is to begin.				*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of our adapter structure		*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None								*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*	None								*/
/************************************************************************/
void
p8xx_chip_register_init(
	ads_t	*ap)
{

    P8printf(P8_ENTRY,("Entering p8xx_chip_register_init\n"));
    P8printf(P8_DEVICE_STATUS_1,("ENTRY "));
    P8if(P8_DEVICE_STATUS_1,(p8xx_rpt_status(ap, "Before register init",0)));
    TRACE_1("in chip init", 0)

    p8xx_write_reg(ap, (uint)SCNTL0, (char)SCNTL0_SIZE, (uchar)SCNTL0_INIT);
    p8xx_write_reg(ap, (uint)SCNTL3, (char)SCNTL3_SIZE, (char)SCNTL3_INIT_FAST);
    p8xx_write_reg(ap, (uint)SIEN0,  (char)SIEN0_SIZE,  (uchar)SIEN0_MASK);
    p8xx_write_reg(ap, (uint)SIEN1,  (char)SIEN1_SIZE,  (uchar)SIEN1_MASK);

    /* Enable response to reselection and OR in chip id */
    p8xx_write_reg(ap, (uint)SCID, (char)SCID_SIZE, SCID_INIT | 
	ap->ddi.addi.card_scsi_id);

    /* Set the SCSI id that the chip responds to when being reselected */
    p8xx_write_reg(ap, (uint)RESPID, (char)RESPID_SIZE,
	(uchar) (0x01 << ap->ddi.addi.card_scsi_id));
    p8xx_write_reg(ap, (uint)SXFER,  (char)SXFER_SIZE,  (uchar)SXFER_INIT);
    p8xx_write_reg(ap, (uint)DIEN,   (char)DIEN_SIZE,   (uchar)DIEN_MASK);
    p8xx_write_reg(ap, (uint)DCNTL,  (char)DCNTL_SIZE,  (uchar)DCNTL_INIT);
    p8xx_write_reg(ap, (uint)CTEST4, (char)CTEST4_SIZE, (uchar)CTEST4_INIT);
    p8xx_write_reg(ap, (uint)STIME0, (char)STIME0_SIZE, (uchar)STIME0_INIT);

    if (ap->ddi.chip_type == IS_A_53C825) {
	if (!((p8xx_read_reg(ap, GPREG, GPREG_SIZE) & GPREG_SE_DRIVERS))) {
	    /* gpio3 is zero, meaning this is an NCR8251D */
            p8xx_write_reg(ap, (uint)STEST2, (char)STEST2_SIZE, 
		(uchar)STEST2_INIT);
	}

        p8xx_write_reg(ap, (uint)DMODE,  (char)DMODE_SIZE,  
		DMODE_INIT | DMODE_ER | DMODE_BOF);
    } else {
        p8xx_write_reg(ap, (uint)DMODE, (char)DMODE_SIZE, DMODE_INIT);
    }

    p8xx_write_reg(ap, (uint)STEST3, (char)STEST3_SIZE, (uchar)STEST3_INIT);


    
    /*
     * First, its a configuration option.
     */
    if (ap->slow_sync
#ifdef SLOW_IF_2_CABLES
	/*
	 * Plus (if desired), we use the two low order bits of the
	 * GPREG register to indicate which cables are currently
	 * plugged in.  Bit 0 is for external and bit 1 is for
	 * internal.  If both cables are in use we make an assumption
	 * (perhaps unwaranted) that the cable length is sufficient to
	 * result in unreliable operation at the FAST (10mb/sec)
	 * synchronous speed, and so, set our maximum speed to
	 * 5mb/sec.  The bit will be 0 when the corresponding cable is
	 * plugged in and terminated.
	 */
	|| (((p8xx_read_reg(ap, GPREG, GPREG_SIZE) & GPREG_BOTH_CABLES) == 0)
	     && (ap->ddi.chip_type == IS_A_53C810))

#endif
    ) {
	P8printf(P8_SYNC, ("Setting 5mb/sec max SCSI bus xfer speed\n"));
	ap->xfer_pd = NCR8XX_SLOW_PERIOD; /* 200ns xfer period (in 4ns units) */
    } else {
	P8printf(P8_SYNC, ("Setting 10mb/sec max SCSI bus xfer speed\n"));
	ap->xfer_pd = NCR8XX_FAST_PERIOD; /* 100ns xfer period (in 4ns units) */
    }

    P8printf(P8_DEVICE_STATUS_2,("EXIT "));
    P8if(P8_DEVICE_STATUS_2,(p8xx_rpt_status(ap, "After register init",0)));
    P8printf(P8_EXIT,("Leaving p8xx_chip_register_init\n"));
    TRACE_1("out chip init", 0)
}



/************************************************************************/
/*									*/
/* NAME:  p8xx_chip_register_reset					*/
/*									*/
/* FUNCTION:  Resets SCSI chip registers.				*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called with interrupts disabled.	*/
/*	It is assumed the chip is powered on.			        */
/*									*/
/* NOTES:  This routine is called to reset the adapter chip registers 	*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of our adapter structure		*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None								*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*									*/
/************************************************************************/
void
p8xx_chip_register_reset(ads_t	*ap)
{

    P8printf(P8_ENTRY,("Entering p8xx_chip_register_reset routine.\n"));
    TRACE_1("in chip_reset", 0)

    /* Disable dma and SCSI interrupts from the chip */
    (void) p8xx_write_reg(ap, SIEN, SIEN_SIZE, 0x0000);
    (void) p8xx_write_reg(ap, DIEN, DIEN_SIZE, 0x00);

    /* Assert a chip reset by writing to the ISTAT register */
    (void) p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, ISTAT_RST);
    io_delay(1);   /* delay 1 usec */
    (void) p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, 0x00);

    /* Call routine to re-initialize all chip registers  */
    p8xx_chip_register_init(ap);

    P8printf(P8_EXIT,("Leaving p8xx_chip_register_reset routine.\n"));
    TRACE_1("out chip_reset", 0)
}


/************************************************************************/
/*									*/
/* NAME:  p8xx_start_chip						*/
/*									*/
/* FUNCTION:  Starts the SCSI chip.					*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called with interrupts disabled.	*/
/*      The chip is assumed to be powered on.                           */
/*									*/
/* NOTES:	This routine is called to start the SCSI chip.  It does	*/
/*		this by storing the bus address of the scripts to be	*/
/*		executed by the chip into its DSP register.		*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*	ads_t	*ap		addr of adapter structure.		*/
/*	dvi_t	*dev_ptr	addr of devinfo structure.		*/
/*	uint	script_bus_addr	This is the bus address of the SCRIPTS	*/
/*				to be processed by the chip.		*/
/*	int	entry_offset	This is the offset within the SCRIPTS	*/
/*				that the chip should begin execution.	*/
/*	uint	tim_bus_addr	This is the bus address of the Table	*/
/*				Indirect structure that contains what	*/
/*				is effectively a scatter/gather list	*/
/*				for the data transfer. If zero, the DSA	*/
/*				register is not changed.		*/
/*	int	tim_no		Number of TIM entries			*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None								*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*									*/
/************************************************************************/
void
p8xx_start_chip(
	ads_t	*ap,
	struct cmd_info	*com_ptr,
	uint	script_bus_addr,
	int	entry_offset,
	uint	tim_bus_addr,
	uint	tim_no)
{
	dvi_t	*dev_ptr;

	TRACE_2("in schp", (int) script_bus_addr, (int) entry_offset)
	P8printf(P8_ENTRY,("Entering p8xx_start_chip routine.\n"));
	P8printf(P8_INFO_2,
	    (" script_bus_addr=0x%x, entry_offset=0x%x, ep=0x%x (bus),\n",
	    script_bus_addr, entry_offset, script_bus_addr+entry_offset));
	P8printf(P8_INFO_2,(" tim_bus_addr=0x%x\n",
	    tim_bus_addr));

	P8debug(ap->chip_strt_cnt++);

	if (com_ptr) {

            dev_ptr = com_ptr->device;

	    /*
	     * If a TIM was specified, set its bus addr and count to our work
	     * area and the bus addr to the chip's DSA register.  Note that
	     * the tim_cnt is picked up via the SCRIPTS.
	     */

	    if (tim_bus_addr) {
		uchar tag_index = com_ptr->tag;
		saved_t *move_info_ptr;

		P8debug(ap->chip_strt_wd_cnt++);
		move_info_ptr = (saved_t *) (
                ((ulong) ap->IND_TABLE.system_ptr + (16 * tag_index)));
                move_info_ptr->tim_bus_addr = LtoCBO(tim_bus_addr);
                move_info_ptr->tim_cnt = LtoCBO(tim_no);
		move_info_ptr->tim_restore = 0;
		move_info_ptr->tables_differ = 0;
    	        TRACE_2("strt chp", (int) com_ptr->tag, (int) 
		        move_info_ptr->tim_bus_addr)
    	        TRACE_3("schp", (int) com_ptr->tag, (int) move_info_ptr->tim_cnt,
                    (int) move_info_ptr)
	    }

	    /*
	     * If we don't have another device waiting to start we set
	     * the check_next_io interrupt to a nop; thus causing the
	     * scripts to go straight to waiting for reselection.  This
	     * avoids the immediate interrupt following the sending of
	     * the command (which we essentially ignore).
	     * Note that if another request subsequently comes in we will
	     * interrupt the chip out of the wait via the SIGP flag by
	     * calling SUSPEND_CHIP() in p8xx_start().
	     */
	    if (ap->DEVICE_WAITING_head) {
		INT_ON(dev_ptr->sid_ptr->vscript_ptr, A_check_next_io_Used);
		P8debug(ap->wait_req_cnt++);
	    } else {
		INT_OFF(dev_ptr->sid_ptr->vscript_ptr, A_check_next_io_Used);
		P8debug(ap->no_int_cnt++);
	    }
	}


#ifdef CACHE_PARANOIA
	/*
	 * Cache coherency paranoia strikes: Ensure chip sees our latest
	 * updates to the SCRIPTS
	 */
	TRACE_1("sync cache", 0);
	SYNC();
	if (dev_ptr)
	    vm_cflush(dev_ptr->worka_v, PAGESIZE);
#endif
	    
	/* Now we start the device at the requested SCRIPTS command */
	p8xx_write_reg(ap, (uint)DSP, DSP_SIZE, script_bus_addr + entry_offset);

	P8if(P8_DEVICE_STATUS_2,
	    (p8xx_rpt_status(ap, "At exit from start_chip",0)));
	P8printf(P8_EXIT,("Leaving p8xx_start_chip routine.\n"));
	TRACE_1("out schip", tim_no);

}

/************************************************************************/
/*									*/
/* NAME:	p8xx_config_adapter					*/
/*									*/
/* FUNCTION:	Adapter Configuration Routine				*/
/*									*/
/*	This internal routine performs actions required to make		*/
/*	the driver ready for the first open to the adapter.		*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine must be called from the process level only.	*/
/*									*/
/* DATA STRUCTURES:							*/
/*	Global_adapter_ddi						*/
/*									*/
/* INPUTS:								*/
/*	ads_t	ap	address of the adapter structure 		*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	return code = 0 for successful return				*/
/*		    = EIO for unsuccessful operation			*/
/*									*/
/*									*/
/************************************************************************/
int
p8xx_config_adapter(
	ads_t	*ap)
{
    uint	id, i;

    P8printf(P8_ENTRY,("Entering p8xx_config_adapter routine.\n"));

    /*
     * Don't try WDTR negotiation if the chip is narrow
     */
    if (ap->ddi.chip_type == IS_A_53C810) {
        for (i=0;i<MAX_SIDS; i++) 
	    ap->device_state[i].flags |= AVOID_WDTR;
    }

    /*
     * Do the PCI configuration
     */

    /*
     * Disable all i/o and memory space accesses
     */
    if (p8xx_write_cfg(ap, PCI_COMMAND, 
			   PCI_COMMAND_SIZE, P8XX_COMMAND_DISABLE)) {
	P8printf(P8_EXIT,(
	    "leaving p8xx_config_adapter: Unable to enable adapter\n"));
	return(EIO);
    }
    /*
     * Write the base i/o space address
     */
    if (p8xx_write_cfg(ap, PCI_BASEADDR0, PCI_BASEADDR0_SIZE,
	ap->ddi.addi.base_addr)) {
	P8printf(P8_EXIT,(
	    "leaving p8xx_config_adapter: Unable to set base addr\n"));
	return(EIO);
    }
    /*
     * Now enable the i/o space address
     */
    if (p8xx_write_cfg(ap, PCI_COMMAND, PCI_COMMAND_SIZE, P8XX_COMMAND_INIT)) {
	P8printf(P8_EXIT,(
	    "leaving p8xx_config_adapter: Unable to enable adapter\n"));
	return(EIO);
    }
    if (p8xx_write_cfg(ap, PCI_LATTMR, PCI_LATTMR_SIZE,
	P8XX_LAT_TMR)) {
	P8printf(P8_EXIT,(
	    "leaving p8xx_config_adapter: Unable to set latency timer\n"));
	return(EIO);
    }

    /*
     * Clear any pending interrupts that may have been left over from boot.
     */
    p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, 0x00);
    (void) p8xx_read_reg(ap, DSTAT, DSTAT_SIZE);
    (void) p8xx_read_reg(ap, SIST, SIST_SIZE);

    P8printf(P8_EXIT,("Leaving p8xx_config_adapter: Okay\n"));
    return (0);

}  /* end p8xx_config_adapter */



/************************************************************************/
/*									*/
/* NAME:	p8xx_logerr						*/
/*									*/
/* FUNCTION:	Adapter Driver Error Logging Routine			*/
/*									*/
/*	This routine provides a common point through which all driver	*/
/*	error logging passes.						*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This is an internal routine which can be called from any	*/
/*	other driver routine.  However, read_regs should be false if    */
/*      interrupts are not disabled and the chip is not known to be     */
/*      powered on.							*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*	error_log_def - adapter driver error logging information	*/
/*			structure					*/
/*									*/
/* INPUTS:								*/
/*	errid	- the error log unique id for this entry		*/
/*	add_hw_status - additional hardware status for this entry	*/
/*	errnum	- a unique value which identifies which error is	*/
/*		  causing this call to log the error			*/
/*	data1	- additional, error dependent data			*/
/*	ads_t *ap	address of adapter structure			*/
/*	cmd_t *com_ptr	address of command info structure		*/
/*	uchar read_regs	flag to control reading of registers		*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	none								*/
/*									*/
/************************************************************************/
void
p8xx_logerr(
	int	errid,
	int	add_hw_status,
	int	errnum,
	int	data1,
	ads_t	*ap,
	cmd_t	*com_ptr,
	uchar	read_regs)
{
    struct error_log_def log;
    caddr_t bus_addr;
    int	    cmd_length, cmd_filler;
    ulong tmp_word = 0;

    P8printf(P8_ENTRY,("Entering p8xx_logerr routine.\n"));
    TRACE_1("logerr", errnum)

    if ((ap->errlog_enable) && (!ap->dump_started)) {
	
	/* set up info and log */
	bzero(&log, sizeof(struct error_log_def));
	log.errhead.error_id = (uint) errid;
	bcopy(&ap->ddi.addi.resource_name[0], &log.errhead.
	      resource_name[0], 8);
	log.data.diag_validity = 0;
	log.data.diag_stat = 0;
	if (add_hw_status != 0) {
	    log.data.ahs_validity = 0x01;
	    log.data.ahs = (uchar) add_hw_status;
	}
	log.data.un.card2.errnum = (uint) errnum;

	/* See if additional data is needed due to error id */
	if (((uint) errid == (uint) ERRID_SCSI_ERR7) ||
	    ((uint) errid == (uint) ERRID_SCSI_ERR8))
	    log.data.sys_rc = (uint) data1;

	/* Indicate which register caused this error logging to be called */
	if ((((uint) errid == (uint) ERRID_SCSI_ERR1) ||
	     ((uint) errid == (uint) ERRID_SCSI_ERR2)) &&
	    (data1 != 0))
	    log.data.un.card2.reg_validity = data1;

	/* See if additional data is needed due to add_hw_status value */
	if ((uint) add_hw_status & DMA_ERROR)
	    log.data.sys_rc = (uint) data1;

	if (read_regs) {
	    
	    /* On an interrupt level, read these regs */
	    log.data.un.card2.istat_val  = p8xx_read_reg(ap,ISTAT,ISTAT_SIZE);
	    log.data.un.card2.sstat0_val = p8xx_read_reg(ap,SSTAT0,SSTAT0_SIZE);
	    log.data.un.card2.sstat1_val = p8xx_read_reg(ap,SSTAT1,SSTAT1_SIZE);
	    log.data.un.card2.sstat2_val = p8xx_read_reg(ap,SSTAT2,SSTAT2_SIZE);
	    log.data.un.card2.sdid_val   = p8xx_read_reg(ap,SDID,SDID_SIZE);
	    log.data.un.card2.scid_val   = p8xx_read_reg(ap,SCID,SCID_SIZE);
	    log.data.un.card2.ctest3_val = p8xx_read_reg(ap,CTEST3,CTEST3_SIZE);

	    /*
	     * The following lines are #ifdef'd out since a direct read of
	     * these registers will clear pending interrupts.  To log the
	     * contents of these 3 registers, we would have to pass their
	     * values in.  This is left as a possible future enhancement.
	     */
#if 0
	    log.data.un.card2.dstat_val = p8xx_read_reg(ap,DSTAT,DSTAT_SIZE);
	    log.data.un.card2.sist0_val = p8xx_read_reg(ap,SIST0,SIST0_SIZE);
	    log.data.un.card2.sist1_val = p8xx_read_reg(ap,SIST1,SIST1_SIZE);
#endif
	    log.data.un.card2.ssid_val   = p8xx_read_reg(ap,SSID,SSID_SIZE);
	    log.data.un.card2.sxfer_val  = p8xx_read_reg(ap,SXFER,SXFER_SIZE);
	    log.data.un.card2.scntl2_val = p8xx_read_reg(ap,SCNTL2,SCNTL2_SIZE);
	    log.data.un.card2.scntl3_val = p8xx_read_reg(ap,SCNTL3,SCNTL3_SIZE);
	    log.data.un.card2.gpreg_val  = p8xx_read_reg(ap,GPREG,GPREG_SIZE);
	    log.data.un.card2.scntl1_val = p8xx_read_reg(ap,SCNTL1,SCNTL1_SIZE);
	    log.data.un.card2.sfbr_val  = p8xx_read_reg(ap,SFBR,SFBR_SIZE);
	    log.data.un.card2.sbcl_val  = p8xx_read_reg(ap,SBCL,SBCL_SIZE);
	    log.data.un.card2.scratcha_val  =
				p8xx_read_reg(ap,SCRATCHA, SCRATCHA_SIZE);
	    log.data.un.card2.scratchb_val  =
				p8xx_read_reg(ap,SCRATCHB, SCRATCHB_SIZE);
	    log.data.un.card2.dsa_val  = p8xx_read_reg(ap, DSA, DSA_SIZE);

	    log.data.un.card2.dbc_val  = 
				p8xx_read_reg(ap, DBC_DCMD, DBC_DCMD_SIZE);
	    log.data.un.card2.dsp_val  = p8xx_read_reg(ap, DSP, DSP_SIZE);
	    log.data.un.card2.dsps_val = p8xx_read_reg(ap, DSPS, DSPS_SIZE);

	    if (com_ptr != NULL) {
		
                if (com_ptr->bp != NULL) {
                    cmd_length = (int) com_ptr->bp->scsi_command.scsi_length;
                    log.data.un.card2.scsi_cmd[0] = com_ptr->bp->
                                scsi_command.scsi_cmd.scsi_op_code;
                    log.data.un.card2.scsi_cmd[1] = com_ptr->bp->
                                scsi_command.scsi_cmd.lun;
                    for (cmd_filler = 2; cmd_filler < cmd_length; cmd_filler++)
                        log.data.un.card2.scsi_cmd[cmd_filler] =
                                com_ptr->bp->scsi_command.scsi_cmd.scsi_bytes
                                [cmd_filler - 2];
                }
                if (com_ptr->device != NULL) {
                    log.data.un.card2.target_id = com_ptr->device->scsi_id;
                    log.data.un.card2.queue_state =
                                       com_ptr->device->queue_state;
                    log.data.un.card2.cmd_activity_state =
                                       com_ptr->device->cmd_activity_state;
                    log.data.un.card2.resource_state = com_ptr->resource_state;
		}
	    }

	    /* read the PCI configuration registers */
   	    cmd_length = p8xx_read_cfg(ap, PCI_SIGNATURE_OFFS, 
				       PCI_SIGNATURE_SIZE, &tmp_word);
	    /* store device id in sense data */
	    log.data.un.card2.pos0_val = (uchar) (tmp_word >> 24);
	    log.data.un.card2.pos1_val = (uchar) (tmp_word >> 16);

   	    (void) p8xx_read_cfg(ap, PCI_STATUS, PCI_STATUS_SIZE, &tmp_word);
	    /* store high-order byte in pos2_val field of sense data */
	    log.data.un.card2.pos2_val = (uchar) (tmp_word >> 8);
	    log.data.un.card2.pos3_val = (uchar) tmp_word;

   	    (void) p8xx_read_cfg(ap, PCI_REV_ID, PCI_REV_ID_SIZE, &tmp_word);
	    log.data.un.card2.pos4_val = (uchar) tmp_word;
	}

	/* Log the error here */
	errsave(&log, sizeof(struct error_log_def));

	/* check for NCR825 TERM PWR failure, make additional entry if needed */
	if (read_regs && (ap->ddi.chip_type == IS_A_53C825) &&
            (!(log.data.un.card2.gpreg_val & GPREG_TP_OKAY))) {
	    log.data.ahs = TERM_PWR_FAIL;
	    log.data.un.card2.errnum = 5;
	    log.errhead.error_id = ERRID_SCSI_ERR2;
	    errsave(&log, sizeof(struct error_log_def));
	}
	    
    }
    P8printf(P8_EXIT,("Leaving p8xx_logerr routine.\n"));

}  /* end p8xx_logerr */



/*
 * NAME:	p8xx_read_cfg
 *
 * FUNCTION:	Access the PCI configuration registers
 *		This routine reads from a selected PCI configuration
 *		register and returns the data.  Note that only word
 *		aligned access is permitted (seems to be a PCI config
 *		space restriction).
 *
 * EXECUTION ENVIRONMENT:
 *		This is an internal routine which can be called at
 *		any point to read a single word from the PCI 
 *		configuration registers.
 * INPUT: 	ads_t *ap	address of the adapter struct
 *		uint offset	offset into PCI config space
 *		uint length	and the length of the register to read
 *		uint *data	where the read data goes
 * RETURNS:	0 if successfully read; or
 *		1 if unable to read.
 */

int	
p8xx_read_cfg(
	ads_t	*ap,		/* ptr to adapter structure */
	uint	offset,		/* offset into PCI config area */
	uint	length,		/* length to read */
	uint	*data)		/* where to put the data read */
{
	int	rc = 0;
	struct mdio	md;

    P8printf(P8_READ_REG,("Entering p8xx_read_cfg routine, REG=0x%04X, ",
	offset));

    /*
     * Use the machdd, it should handle any serialization issues
     */
    P8printf(P8_READ_REG,("slot=0x%02x\n", ap->ddi.addi.slot));
    md.md_addr = offset;
    md.md_data = (char *)data;
    md.md_size = length;
    md.md_incr = MV_BYTE;
    md.md_sla  = ap->ddi.addi.slot;


    rc = pci_cfgrw(ap->bus_bid, &md, FALSE);
    P8printf(P8_READ_REG,("offset=0x%08d len=0x%d, data= 0x%08x\n",
		          offset, length, (*data)));
    if (rc == 0)
	switch (length) {
	    case 1:
		*data = *data >> 24;
		break;
	    case 2:
		*data = *data >> 16;
		*data = CBOtoS(*data);
		break;
	    case 4:
		*data = CBOtoL(*data);
		break;
	    default:
                ASSERT(0);		/* only 1, 2, or 4 byte access */
	}

    P8printf(P8_READ_REG,("Leaving p8xx_read_cfg routine, RC=%d, DATA=0x%08x\n",
	rc, *data));
    return(rc);

}



/*
 * NAME:	p8xx_write_cfg
 *
 * FUNCTION:	Write the PCI configuration registers
 *		This routine writes to a selected PCI configuration
 *		register.  Note that only word 	aligned access 
 *		is permitted (seems to be a PCI config space restriction).
 *
 * EXECUTION ENVIRONMENT:
 *		This is an internal routine which can be called at
 *		any point to write a single word to the PCI 
 *		configuration registers.
 * INPUT: 	ads_t *ap	address of the adapter struct
 *		uint offset	offset into PCI config space
 *		uint length	and the length of the register to read
 *		uint data	data to be written
 * RETURNS:	0 if successfully written; or
 *		1 if it failed.
 */

int	
p8xx_write_cfg(
	ads_t	*ap,		/* ptr to adapter structure */
	uint	offset,		/* offset into PCI config area */
	uint	length,		/* length to write */
	uint	data)		/* and the data */
{
	int	rc = 0;
	struct mdio	md;
	uint		dbuf;

    P8printf(P8_WRITE_REG,("Entering p8xx_write_cfg routine, REG=0x%X 0x%x, ",
	offset, data));

    dbuf = LtoCBO(data);		/* Swap the bytes.  Note that for
					   lengths of 1 and 2 this DTRT by
					   placing the byte(s) to be written
					   at the lowest (first) address */

    /*
     * Use the machdd
     */
    switch (length) {
	case 1:
	    md.md_incr = MV_BYTE;
	    break;
	case 2:
	    md.md_incr = MV_SHORT;
	    break;
	case 4:
	    md.md_incr = MV_WORD;
	    break;
	default:
            ASSERT(0);		/* only 1, 2, or 4 byte access */
    }

    md.md_addr = offset;
    md.md_data = (char *)&dbuf;
    md.md_size = 1;
    md.md_sla  = ap->ddi.addi.slot;

    rc = pci_cfgrw(ap->bus_bid, &md, TRUE);  /* machdd will handle any
					      * serialization issues.
					      */

    return(rc);

}

/************************************************************************/
/*									*/
/* NAME:	p8xx_read_reg						*/
/*									*/
/* FUNCTION:	Access the specified register.				*/
/*									*/
/*	This routine reads from a selected adapter			*/
/*	register and returns the data, performing			*/
/*	appropriate error detection and recovery.			*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This is an internal routine which can be called at any		*/
/*	point to read a single register; 8,24 or 32 bit			*/
/*      The chip is assumed to be powered on.				*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*									*/
/* INPUTS:								*/
/*	ads_t	*ap	address of the adapter structure		*/
/*	uint	offset	offset of the register to be read.  Note that	*/
/*			we are using the ``Mem I/O'' offsets here (even */
/*			though our registers are accessed in ``Config	*/
/*			Mem I/O'' space).				*/
/*	char	reg_size length of register.				*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	return code = 32-bits of data					*/
/*									*/
/* NOTES:								*/
/*	This routine will swap bytes from Little Endian (the bus order) */
/*	to Big Endian.							*/
/*									*/
/************************************************************************/
int
p8xx_read_reg(
	ads_t	*ap,		/* ptr to adapter structure */
	uint	offset,
	char	reg_size)
{
	int	old_level,
		data;
	volatile char *io;

    P8printf(P8_READ_REG,("Entering p8xx_read_reg routine, REG=0x%X, ",
	offset));

    P8debug(ap->pio_r_cnt++);
    ASSERT(offset < 256);  /* stay in our address range */

    /*
     * Disable our interupts while reading registers to prevent our
     * interrupts from killing the read
     */
    old_level = i_disable(ap->ddi.addi.int_prior);

    P8printf(P8_READ_REG,("base_addr=%x\n", ap->base_addr0));
    P8if(P8_READ_REG,(p8xx_hexdump("iom", (char *) &ap->iom, sizeof(ap->iom))));
    io = (volatile char *)iomem_att(&ap->iom);

    /*
     * Read register size number of bytes and convert byte order from
     * cannonical bus to system if necessary (more than 1 byte).
     * Note that the 3 byte case has a problem in that it actually reads
     * 4 bytes and simply masks off the extra byte.  It is conceivable
     * that this could cause problems in some cases; though for the
     * only case where we currently read a 3-byte register (the DBC
     * register), it does work okay.
     */
    switch (reg_size) {
	case 1:
	    data = *((volatile char *)(io + offset));
	    break;
	case 2:
	    data = *((volatile short *)(io + offset));
	    data = CBOtoS(data);
	    break;
	case 3:
	    data = *((volatile int *)(io + offset));
	    data = CBOtoL(data) & 0x00ffffff;
	    break;
	case 4:
	default:
	    data = *((volatile int *)(io + offset));
	    data = CBOtoL(data);
	    break;
    }

    iomem_det((void *)io);
    i_enable(old_level);

    P8printf(P8_READ_REG,("Leaving p8xx_read_reg routine, DATA=0x%x\n", data));
    return (data);

}  /* end p8xx_read_reg */



/************************************************************************/
/*									*/
/* NAME:	p8xx_write_reg						*/
/*									*/
/* FUNCTION:	Store passed data in specified register.		*/
/*									*/
/*	This routine loads a selected adapter register with the		*/
/*	passed data value, performing appropriate error detection	*/
/*	and recovery.							*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This is an internal routine which can be called at any		*/
/*	point to load a single register.				*/
/*      The chip is assumed to be powered on.				*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*									*/
/* INPUTS:								*/
/*	ads_t	*ap	address of the adapter structure		*/
/*	uint	offset	offset of the register to write.  Note that	*/
/*			we are using the ``Mem I/O'' offsets here (even */
/*			though our registers are accessed in ``Config	*/
/*			Mem I/O'' space).				*/
/*	char	reg_size length of register.				*/
/*	int	data	value to be written				*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	0	Always							*/
/*									*/
/* NOTES:								*/
/*	This routine will swap bytes from Big Endian (the system order) */
/*	to Little Endian.						*/
/*									*/
/************************************************************************/
int
p8xx_write_reg(
	ads_t	*ap,		/* ptr to adapter structure */
	uint	offset,
	char	reg_size,
	int	data)
{
    int		old_level;
    volatile char *io;

    P8printf(P8_WRITE_REG,("Entering p8xx_write_reg routine, REG=0x%X 0x%x, ",
	offset, data));

    P8debug(ap->pio_w_cnt++);
    ASSERT(offset < 256);  /* stay in our address range */

    /*
     * Disable our interupts while writing registers to prevent our
     * interrupts from killing the write
     */
    old_level = i_disable(ap->ddi.addi.int_prior);

    P8printf(P8_WRITE_REG,("base_addr=%x\n", ap->base_addr0));
    P8if(P8_READ_REG,(p8xx_hexdump("iom", (char *) &ap->iom, sizeof(ap->iom))));
    io = (volatile char *)iomem_att(&ap->iom);

    /*
     * Write register size number of bytes, converting the byte order from
     * system to canonical bus if necessary (more than 1 byte).
     * Note that the 3 byte case has a problem in that it actually writes
     * 4 bytes (high-order byte is set to zero).  It is likely that this is
     * a bad thing to do!  Note that we do not currently write *any* three
     * byte registers.
     */
    switch (reg_size) {
	case 1:
	    *((volatile char *)(io + offset)) = (char)data;
	    break;
	case 2:
	    *((volatile short *)(io + offset)) = StoCBO((ushort)data);
	    break;
	case 3:
	    data &= 0x00ffffff;
	case 4:
	default:
	    *((volatile int *)(io + offset)) = LtoCBO((ulong)data);
	    break;
    }

    iomem_det((void *) io);
    i_enable(old_level);

    return(0);

}  /* end p8xx_write_reg */



/************************************************************************/
/*									*/
/* NAME:	p8xx_strategy						*/
/*									*/
/* FUNCTION: Scsi Chip Device Driver Strategy Routine			*/
/*									*/
/*	This routine will accept commands from the device heads		*/
/*	and queue them up to the appropriate device structure for	*/
/*	execution.  The command, in the form of scbufs, contains	*/
/*	a bufstruct at the beginning of the structure and pertinent	*/
/*	data appended after cannot exceed the maximum transfer size	*/
/*	allowed.  Note that the av_forw field of the bufstruct MUST	*/
/*	be NULL as chained commands are not supported.			*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called by a process or interrupt	*/
/*	handler.  It can page fault only if called under a process	*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*	scbuf - input command structure.				*/
/*									*/
/* INPUTS:								*/
/*	bp - command buffer pointer					*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	0 - For good completion						*/
/*	ERRNO - value otherwise						*/
/*									*/
/* ERROR DESCRIPTION							*/
/*	EIO - Adapter not defined or initialized			*/
/*	    - Adapter not opened					*/
/*	    - Device not started					*/
/*	    - Device is being stopped					*/
/*	ENXIO - Device queue is currently awaiting a resume cmd		*/
/*									*/
/************************************************************************/
int
p8xx_strategy(
	struct sc_buf	*bp)
{
    int		ret_code;
    dev_t	devno;
    int		dev_index;
    int		new_pri;
    dvi_t	*dev_ptr;
    ads_t	*ap;
#ifdef DEBUG_8XX
    unsigned	ai,af,bi,bf;
#endif

    P8_SETDBG(P8_STRATEGY,P8_STD_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_strategy routine \n"));
    P8printf(P8_INFO_2,("bp=0x%08x, b_addr=0x%08x, b_bcount=%d\n",
	bp, bp->bufstruct.b_un.b_addr, bp->bufstruct.b_bcount));

    ret_code = 0;
    devno = bp->bufstruct.b_dev;
    ap = p8xx_get_adp(devno);
    assert(ap != NULL);

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_STRATEGY, ret_code, devno, bp,
	    bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
	    bp->bufstruct.b_bcount);

    P8debug(ap->strategy_cnt++);
    P8if(P8_PRTCNT, {
	if (ap->strategy_cnt > 1 && (ap->strategy_cnt-1)%10000 == 0) {
	    printf("---\n");
	    printf("Strategy calls:              %u\n", ap->strategy_cnt-1);
	    printf("Intr taken/processed/ph_mis: %u/%u/%u\n",
		ap->intr_cnt,ap->intr_proc_cnt,ap->phase_mismatch_cnt);
	    printf("SIGP - set/taken:            %u/%u\n",
		ap->sigp_set_cnt,ap->sigp_intr_cnt);
	    printf("PIOs - input/output:   %u/%u/\n",
		ap->pio_r_cnt,ap->pio_w_cnt);
	    printf("Chip starts/w-intr/w-out:    %u/%u/%u\n",
		ap->chip_strt_cnt, ap->wait_req_cnt, ap->no_int_cnt);

	    ai = (ap->pio_r_cnt+ap->pio_w_cnt)*10 / ap->intr_proc_cnt;
	    af = ai % 10;
	    ai = ai / 10;
	    bi = (ap->pio_r_cnt+ap->pio_w_cnt)*10 / ap->strategy_cnt;
	    bf = bi % 10;
	    bi = bi / 10;
	    printf("PIOs per intr/call:          %d.%d/%d.%d\n",
		ai,af, bi, bf);
	}
    });

    if ((ap->str_inited) && (ap->opened)) {

	/*
	 * Passed the init and the open test.  Get index into device table
	 * for this device.
	 */
	dev_index = INDEX(bp->scsi_command.scsi_id,
	    (bp->scsi_command.scsi_cmd.lun) >> 5);

	/* Disable interrupt to keep this single threaded. */
	new_pri = i_disable(ap->ddi.addi.int_prior);

	dev_ptr = ap->device_queue_hash[dev_index];
        TRACE_2 ("in strat", (int) bp, (int) dev_ptr)

	/* Miscellaneous validation of request */
        /* a check that if we are stopping, we will only accept commands */
        /* to get through a contingent allegiance condition */
	if ((dev_ptr != NULL) &&
	    (bp->bufstruct.b_bcount <= ap->max_request)
            && ((dev_ptr->queue_state & STOPPING_MASK) != STOPPING)
	    && (bp->resvd1 == 0x0)) {

	    /* Passed check of device structure info */

            TRACE_1 ("qstate", dev_ptr->queue_state)
            /* ACTIVE and WAIT_FLUSH case */
            if (dev_ptr->queue_state & ACTIVE_or_WAIT_FLUSH)
            {
                /* if we get here, device is not halted anymore */
                /* set normal state */
                bp->bufstruct.av_forw = NULL;   /* only accept one cmd */
                bp->bufstruct.b_work = 0;
                /* Initialize the following flag to the "no */
                /* error" state.                            */
                bp->bufstruct.b_error = 0;
                bp->bufstruct.b_flags &= ~B_ERROR;
                bp->bufstruct.b_resid = 0;
                bp->status_validity = 0;
                bp->general_card_status = 0;
                bp->scsi_status = 0;
                bp->resvd7 = FALSE;         /* not expedited */

                /* Queue this command to the device's pending      */
                /* queue for processing by p8xx_start.             */
                p8xx_start(bp, dev_ptr);
	    }
            /* case HALTED: */
            else if (dev_ptr->queue_state & HALTED)
                /* The HALTED flag indicates all the queue was    */
                /* flushed when we informed the head of a prior   */
                /* error.  Do not continue the processing if the  */
                /* caller has not set the SC_RESUME flag.         */
            {
                if (bp->flags & SC_RESUME)
                {
                    /* initialize bp */
                    bp->bufstruct.av_forw = NULL;   /* only accept one cmd */
                    bp->bufstruct.b_work = 0;
                    /* Initialize the following flag to the "no */
                    /* error" state.                            */
                    bp->bufstruct.b_error = 0;
                    bp->bufstruct.b_flags &= ~B_ERROR;
                    bp->bufstruct.b_resid = 0;
                    bp->status_validity = 0;
                    bp->general_card_status = 0;
                    bp->scsi_status = 0;
                    bp->resvd7 = FALSE;             /* not expedited */
                    dev_ptr->queue_state = ACTIVE;
                    p8xx_start(bp, dev_ptr);
	        } else {

		    /* Check queue_state and RESUME flag failed */
		    bp->bufstruct.b_error = ENXIO;
		    bp->bufstruct.b_flags |= B_ERROR;
		    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		    bp->status_validity = 0;
	    	    TRACE_1("iodone bp", (int) bp)
		    iodone((struct buf *) bp);
		    ret_code = ENXIO;
	        }
	    } else if (dev_ptr->queue_state & HALTED_CAC) {
            /* The HALTED_CAC condition indicates the driver is in
             * the initial state of processing a contingent allegiance
             * condition for a cmd. queueing target.  The queues for
             * this target lun are frozen pending action by the head
             * device driver.  First, we require a SC_RESUME.  With the
             * SC_RESUME, we will either expedite an untagged SCSI cmd
             * to the target, or process a SC_Q_CLR.
             */
                if (bp->flags & SC_RESUME) {
                    if (bp->flags & SC_Q_CLR)
                        p8xx_process_q_clr(dev_ptr, bp);
                    else {
                        /* initialize the bp */
                        bp->bufstruct.av_forw = NULL;   /* accept one cmd */
                        bp->bufstruct.b_work = 0;
                        bp->bufstruct.b_error = 0;      /* no errors */
                        bp->bufstruct.b_flags &= ~B_ERROR;
                        bp->bufstruct.b_resid = 0;
                        bp->status_validity = 0;
                        bp->general_card_status = 0;
                        bp->scsi_status = 0;

                        /* mark the bp as being expedited */
                        bp->resvd7 = TRUE;

                        /* in this state, can't send a tagged cmd */
                        if (bp->q_tag_msg != SC_NO_Q) {
                            bp->bufstruct.b_error = ENXIO;
                            bp->bufstruct.b_flags |= B_ERROR;
                            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                            iodone((struct buf *) bp);
	    	            TRACE_1("iodone bp", (int) bp)
                            ret_code = ENXIO;
                        } else {
                            /* change state to wait_info by leaving the */
                            /* value of the STOPPING bit alone, turning */
                            /* all others off except the WAIT_INFO bit. */
                            if (dev_ptr->queue_state & STOPPING)
                                dev_ptr->queue_state = WAIT_INFO | STOPPING;
                            else
                                dev_ptr->queue_state = WAIT_INFO;

                            p8xx_start(bp, dev_ptr);
                        }
                    }
                } /* end if SC_RESUME while in HALTED_CAC */
                else { /* check of queue_state &  SC_RESUME failed */
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    	    TRACE_1("iodone bp", (int) bp)
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
                }

            } else if (dev_ptr->queue_state & WAIT_INFO) {

            /* WAIT_INFO case:
             * The driver is waiting for a command to process the queue
             * during error recovery of a CAC:  either Q_CLR to clear all
             * the remaining commands on the queue, or Q_RESUME, to restart
             * the queue, keeping all commands active.  A command may be
             * present in the sc_buf in the Q_RESUME case.
             */
                if (bp->flags & SC_Q_CLR)
                    /* the queues must be cleared and the bp iodoned */
                    p8xx_process_q_clr(dev_ptr, bp);

                else if (bp->flags & SC_Q_RESUME) {
                    /* the queues have to be resumed */

                    /* adjust the queue state, the CAC processing is finished */

                    if (dev_ptr->queue_state & STOPPING)
                        dev_ptr->queue_state = ACTIVE | STOPPING;
                    else
                        dev_ptr->queue_state = ACTIVE;

                    /* check for a command associated with the sc_buf */
                    if (bp->scsi_command.scsi_length != 0) {
                        /* initialize bp */
                        bp->bufstruct.av_forw = NULL;   /* only accept 1 cmd */
                        bp->bufstruct.b_work = 0;
                        /* Initialize the following flag to the "no */
                        /* error" state.                            */
                        bp->bufstruct.b_error = 0;
                        bp->bufstruct.b_flags &= ~B_ERROR;
                        bp->bufstruct.b_resid = 0;
                        bp->status_validity = 0;
                        bp->general_card_status = 0;
                        bp->scsi_status = 0;
                        bp->resvd7 = FALSE;         /* not expedited */
                        (void) p8xx_unfreeze_qs(dev_ptr, bp, FALSE);
                    } else {
                        (void) p8xx_unfreeze_qs(dev_ptr, NULL, TRUE);
                        bp->bufstruct.b_error = 0;      /* no errors */
                        bp->bufstruct.b_flags &= ~B_ERROR;
	    	        TRACE_1("iodone bp", (int) bp)
                        iodone((struct buf *) bp);
                    }
                } else {
                    /* the bp->flags value is inconsistent w/ queue_state */
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    	    TRACE_1("iodone bp", (int) bp)
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
                }
            } else {  /* we should never get here, queue state is bad */
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    bp->status_validity = 0;
	    	    TRACE_1("iodone bp", (int) bp)
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
                    TRACE_1("default", 0)
            }
	} else {

	    /* Device structure validation failed */
	    bp->bufstruct.b_error = EIO;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->status_validity = SC_ADAPTER_ERROR;
	    bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	    TRACE_1("iodone bp invld", (int) bp)
	    iodone((struct buf *) bp);
	    ret_code = EIO;
	}

        TRACE_1 ("out strat", ret_code)
	i_enable(new_pri);

    } else {

	/* SCSI chip has not been initialized or opened */
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	bp->status_validity = SC_ADAPTER_ERROR;
	bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	iodone((struct buf *) bp);
	ret_code = EIO;
    }

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, devno);
    P8printf(P8_EXIT,("Leaving p8xx_strategy routine \n"));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, EIO, devno);
    P8_RSTDBG(P8_STRATEGY);
    return (ret_code);
}

/************************************************************************
 *								
 * NAME:	p8xx_wide_patch
 *						
 * FUNCTION:    Enables/disables special SCRIPTS instructions necessary to
 *		execute wide scsi
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.
 *						
 * DATA STRUCTURES:			
 *	dev_info structure - target lun unique data structure
 *					
 * INPUTS:			
 *      dev_ptr	   - pointer to target structure
 *	new_script - flag used to avoid extra patches, true if the script
 *		     being patched was just xmalloc'ed and initialized
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_wide_patch(
	dvi_t  *dev_ptr,
	uchar  new_script)
{
    sid_t *sid_ptr = dev_ptr->sid_ptr;
    ulong bscript_ptr;
#ifdef P8_TRACE
    ads_t *ap = dev_ptr->ap;
#endif
    ulong   *target_script; 

    TRACE_1("widepatch", (int) sid_ptr->vscript_ptr)
    target_script =  sid_ptr->vscript_ptr;
    bscript_ptr = sid_ptr->bscript_ptr;

    if (sid_ptr->io_table.scntl3 & SCNTL3_EWS) {
	/* patch the script for to handle odd-aligned wide transfers */

	/* special calls to output data and save_pointers: */
    	target_script[S_INDEX(Ent_send_data) + 1] = 
				LtoCBO(Ent_output_wide_data + bscript_ptr);
    	target_script[S_INDEX(Ent_save_ptrs_patch) + 1] = 
				LtoCBO(Ent_save_pointers_wide + bscript_ptr);
    	target_script[S_INDEX(Ent_save_ptrs_patch1) + 1] = 
				LtoCBO(Ent_save_pointers_wide + bscript_ptr);

	/* all receive instructions will be chained moves */
	target_script[S_INDEX(Ent_in_not_zero)] = P8XX_CHAINED_RECEIVE;

	/* check SCRATCHA2 at command completion */
	target_script[S_INDEX(Ent_cmd_complete_patch)] = 
					P8XX_MOVE_SCRATCHA2_AND_09;

	/* need to shadow SCNTL2 when leaving data input and output routines */
	target_script[S_INDEX(Ent_scntl2_patch_out)] = 
					P8XX_JUMP_WHEN_NOT_DATA_OUT;
	target_script[S_INDEX(Ent_scntl2_patch_in)] = 
					P8XX_JUMP_WHEN_NOT_DATA_IN;

    } else if (!new_script) {
	/*
	 * the special script code is not needed (transfers are 8 bits, so
	 * WSS and WSR are not relavent:executed.  Since scripts by
	 * default do not execute the special instructions, we don't need
	 * to patch them back to the default state if this script was
	 * just initialized.
	 */ 

	/* normal calls to output data and save_pointers: */
    	target_script[S_INDEX(Ent_send_data) + 1] = 
				LtoCBO(Ent_output_data + bscript_ptr);
    	target_script[S_INDEX(Ent_save_ptrs_patch) + 1] = 
				LtoCBO(Ent_save_pointers + bscript_ptr);
    	target_script[S_INDEX(Ent_save_ptrs_patch1) + 1] = 
				LtoCBO(Ent_save_pointers + bscript_ptr);

	/* all receive instructions will be block moves */
	target_script[S_INDEX(Ent_in_not_zero)] = P8XX_BLOCK_RECEIVE;
	
	/* iodone interrupt instead of checking SCNTL2 at command completion */
	/* 2nd word (A_io_done) should already be patched */
	target_script[S_INDEX(Ent_cmd_complete_patch)] = 
					LtoCBO(P8XX_INTR_PATCH);

	/* no need to save SCNTL2, just return */
	target_script[S_INDEX(Ent_scntl2_patch_out)] = P8XX_RETURN_NOT_DATA_OUT;
	target_script[S_INDEX(Ent_scntl2_patch_in)] = P8XX_RETURN_NOT_DATA_IN;
    }
}

/*
 * Name:	p8xx_do_patch
 * Function:	Patch scripts to control SCSI mode (async/sync + speed)
 * Input:	dvi_t *dev_ptr		device info struct ptr
 *		uchar sxfer		SXFER register value
 *		uchar scntl3		SCNTL3 register value
 *
 *		ulong sxfer_move	SCRIPTS move inst to set SXFER
 *		ulong scntl3_move	SCRIPTS move inst to set SCNTL3
 *
 * Returns:	nothing
 */

void
p8xx_do_patch(
	dvi_t	*dev_ptr,
	uchar	sxfer,
	uchar	scntl3)
{
	int	i;
	ulong	sxfer_move,
		scntl3_move;
        sid_t   *sid_ptr = dev_ptr->sid_ptr;
	uint	*target_script = (uint *)dev_ptr->sid_ptr->vscript_ptr;
#ifdef P8_TRACE
	ads_t	*ap = dev_ptr->ap;
#endif
  
	TRACE_2("dopatch", (int) scntl3, (int) sxfer)
	P8printf(P8_SYNC,
	    ("p8xx_do_patch: ID %d/%d - scntl3 %02x, sxfer %02x\n",
	    dev_ptr->scsi_id, dev_ptr->lun_id,
	    scntl3, sxfer));

	sid_ptr->io_table.sxfer  = sxfer;
	sid_ptr->io_table.scntl3 = scntl3;
	sxfer_move  = (ulong) (SXFER_MOVE_MASK  | (sxfer << 8));
	scntl3_move = (ulong) (SCNTL3_MOVE_MASK | (scntl3 << 8));

	for (i=0; i<S_COUNT(R_sxfer_patch_Used); i++)
	    target_script[R_sxfer_patch_Used[i]] = LtoCBO(sxfer_move);
	for (i=0; i<S_COUNT(R_scntl3_patch_Used); i++)
	    target_script[R_scntl3_patch_Used[i]] = LtoCBO(scntl3_move);
}


/*
 * Name:	p8xx_mode_patch
 * Function:	Patch scripts for default ASYNC/SYNC modes
 * Input:	ads_t *ap		Adapter struct ptr
 * 		int   mode		Default mode to patch.
 * 		dvi_t  *dev_ptr		device info struct
 * Returns:	nothing.
 */

void
p8xx_mode_patch(
	ads_t	*ap,
	int	mode,
	dvi_t	*dev_ptr)
{
	uchar	best_pd,
		scntl3,
		sxfer;

        sid_t   *sid_ptr = dev_ptr->sid_ptr;

	switch (mode) {

	    case SYNC_MODE:
		if ((best_pd = p8xx_select_period(ap, ap->xfer_pd,
		    &scntl3, &sxfer))) {
		    ASSERT(best_pd == ap->xfer_pd);
		    sxfer |= ap->req_ack;	/* include req/ack offset */
		    scntl3 |= (sid_ptr->io_table.scntl3 & SCNTL3_EWS);
		    TRACE_1("sync mode", best_pd * 4)
		    break;

		} else {

		    ASSERT(best_pd);
		    TRACE_1("bad sync mode", 0)

		    /* Fall through to async mode */
		}
	    case ASYNC_MODE:
		sxfer  = 0;
		scntl3 = SCNTL3_INIT_SLOW | 
			(sid_ptr->io_table.scntl3 & SCNTL3_EWS);
		TRACE_1("async mode", 0)
		break;

	}

	p8xx_do_patch(dev_ptr, sxfer, scntl3);
}



/************************************************************************/
/*									*/
/* NAME:	p8xx_start						*/
/*									*/
/* FUNCTION: Scsi Chip Device Driver Start Command Routine		*/
/*									*/
/*	This routine is called from the strategy routine when a command */
/*	becomes available for a device that does not currently have a	*/
/*	command in the process of executing.  Resources are allocated,	*/
/*	QUEUE, and an attempt is made to suspend the chip to get this	*/
/*	command running.						*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called by a process or interrupt	*/
/*	handler.  It can page fault only if called under a process	*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - scsi chip unique data structure			*/
/*									*/
/* INPUTS:								*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	NONE								*/
/*									*/
/* ERROR DESCRIPTION							*/
/*	NONE								*/
/*									*/
/************************************************************************/
void
p8xx_start(
          struct sc_buf *bp,
	  dvi_t	*dev_ptr)
{
    ads_t	*ap;
    int		tag, ret_code;
    sid_t	*sid_ptr;
    uint	entry_point;
    uchar	chk_disconnect;

    P8_SETDBG(P8_START,P8_STD_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_start routine.\n"));

    ap = dev_ptr->ap;
    sid_ptr = dev_ptr->sid_ptr;
    TRACE_1 ("in strt ", (int) dev_ptr)

    /* if NOT (epow/suspend/hibernate or
     *    restart_in_progress and bp->flags says to delay the cmd or
     *    flushing or starving or blocked and not expedited cmd)
     * then process the bp
     * else temporarily put it aside
     */
    if ((ap->power_state == P8XX_NORMAL_PWR) && 
        (!(sid_ptr->restart_in_prog && (bp->flags & SC_DELAY_CMD))) && 
        ((!(dev_ptr->flags & FLUSH_or_STARVE_or_BLOCKED)) || bp->resvd7)) {
        /* Attempt to allocate the resources that are  */
        /* needed. i.e. tag and iovecs                */
        tag = p8xx_alloc_resources(bp, dev_ptr);
        TRACE_3("strt", (int) tag, (int) bp, (int) bp->bufstruct.b_un.b_addr)
        TRACE_3("strt", (int) bp->bufstruct.b_blkno, 
                        (int) bp->bufstruct.b_bcount, 
			(int) bp->scsi_command.scsi_cmd.scsi_op_code)

        if (tag <= PSC_ALLOC_FAILED)
        {  /* Resources cannot be allocated */
           p8xx_enq_wfr(bp, ap);
           dev_ptr->flags |= BLOCKED;
           return;
        }

        /* resources were allocated okay */

	if (ap->DEVICE_ACTIVE_head == NULL) {

	    /* Write system trace to indicate start of command to adapter */
	    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, ap->devno,
		bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
		bp->bufstruct.b_bcount);

	    /* Make sure the chip is on. If it's off, only Idle is 
	     * possible, since we won't issue commands thru p8xx_start
	     * when in Suspend or Hibernate.
             */
	    if (ap->pmh.mode == PM_DEVICE_IDLE) 
		p8xx_turn_chip_on(ap, TRUE);

	    /*
	     * Prepare the script
	     */
	    chk_disconnect = bp->scsi_command.flags & SC_NODISC;
	    P8if(P8_ABORT, {
		    if (bp->scsi_command.flags & SC_NODISC)
			printf("start forces NODISC\n");
	    });

	    p8xx_set_disconnect(dev_ptr, chk_disconnect);

	    entry_point = p8xx_negotiate(&ap->command[tag], bp);
            p8xx_patch_tag_changes(&ap->command[tag], bp->q_tag_msg);

	    p8xx_prep_main_script(ap->Scripts.vscript_ptr,
		    dev_ptr->sid_ptr->vscript_ptr, &ap->command[tag],
		    dev_ptr->sid_ptr->bscript_ptr);

	    (void)p8xx_prepare(&ap->command[tag]);	/* build TIM */

	    /*
	     * Start the chip. The call to p8xx_negotiate above gave us the 
             * proper offset based upon the need for synchronous negotiation.
	     */
	    p8xx_start_chip(ap, &ap->command[tag], 
                     dev_ptr->sid_ptr->bscript_ptr, entry_point,
		     ap->command[tag].tim_bus_addr,
		     ap->command[tag].tim_tbl_used);

	    /* Start the watchdog timer for this command */
            if (bp->timeout_value == 0)
                dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
            else
                dev_ptr->dev_watchdog.dog.restart = (ulong)bp->timeout_value+1;
            W_START(&(dev_ptr->dev_watchdog.dog));

	    /* Update the power mgmt. activity field */
            ap->pmh.activity = PM_ACTIVITY_OCCURRED;

	    /* Add to the DEVICE ACTIVE QUEUE */
	    p8xx_enq_active(dev_ptr, &ap->command[tag]);
            TRACE_1("started cmd", (int) &ap->command[tag])

	    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, ap->devno);

	} else if (ap->DEVICE_WAITING_head == NULL) {

	    /* Add to the waiting queue and get attention of chip */
	    p8xx_enq_wait(&ap->command[tag]);
	    SUSPEND_CHIP(ap);

	} else {

	    /* Another device already waiting so simply add to queue */
	    p8xx_enq_wait(&ap->command[tag]);

	}

    } else {
	/* Don't want to try to allocate resources */
        /* Add to the BP SAVE QUEUE */
        TRACE_1 ("not trying", (int) dev_ptr)
#ifdef P8_TRACE
        if (sid_ptr->restart_in_prog && (bp->flags & SC_DELAY_CMD))
            TRACE_1("restarting", (int) sid_ptr)
        if ((dev_ptr->flags & FLUSH_or_STARVE_or_BLOCKED) && !bp->resvd7)
            TRACE_2("flags", dev_ptr->flags, bp->resvd7)
	if (ap->power_state != P8XX_NORMAL_PWR)
            TRACE_1("power", (int) ap->power_state)
#endif
	if (bp->resvd7) {
	    /* expedited bps are placed at the head of the bp_save queue */
	    if (dev_ptr->bp_save_head == NULL)
	        dev_ptr->bp_save_tail = bp;
	    else
		bp->bufstruct.av_forw = (struct buf *) dev_ptr->bp_save_head;

	    dev_ptr->bp_save_head = bp;
	    
	} else {
	    /* other bps are placed at the tail of the bp_save queue */
            if (dev_ptr->bp_save_tail == NULL)
                dev_ptr->bp_save_head = bp;
            else
                dev_ptr->bp_save_tail->bufstruct.av_forw = (struct buf *) bp;

            dev_ptr->bp_save_tail = bp;
	}
        TRACE_1 ("bp_save", (int) dev_ptr->bp_save_head)

    }

    P8if(P8_DEVICE_STATUS_2,(p8xx_rpt_status(ap, "At exit from start",0)));

    P8printf(P8_EXIT,("Leaving p8xx_start routine.\n"));
    P8_RSTDBG(P8_START);
    TRACE_1 ("out strt", (int) dev_ptr)
    return;
}


/**************************************************************************/
/*									  */
/* NAME:	p8xx_prepare						  */
/*									  */
/* FUNCTION:	This routine does preparation for a data transfer	  */
/*		operation for the active request.  Its primary		  */
/*		functions are to Build the Table Indirect Move list for   */
/*		the SCRIPT to use.     					  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process.			  */
/*									  */
/* DATA STRUCTURES:							  */
/*    cmd_info structure - command information structure.		  */
/*									  */
/* INPUTS:								  */
/*    cmd_info structure - pointer to command information structure	  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	Returns the number of TIM entries				  */
/*									  */
/**************************************************************************/
int	
p8xx_prepare(
	struct cmd_info	*com_ptr)
{
    struct sc_buf	*bp;
    ads_t  *ap;
    int			tim_no = 0;

    P8printf(P8_ENTRY,("Entering p8xx_prepare routine.\n"));

    com_ptr->tim_tbl_used = 0;

    /* Obtain pointer to the command to process. */
    bp = com_ptr->bp;
    ap = com_ptr->device->ap;
    TRACE_2("prepare", (int) bp, (int) com_ptr)

    /* p8xx_prepare sets the nexus only if tim_bus_addr is non-null */
    com_ptr->tim_bus_addr = (uint) (ap->loc_tim_tbl.dma_ptr +
				2 * com_ptr->resource_index);

    if (bp->bufstruct.b_bcount > 0) {

	if (bp->bp == NULL) {		/* NOT spanned command */

            /* tim_t holds 2 ulongs */
	    tim_no = BUILD_TIM(ap, com_ptr, (&(bp->bufstruct)), ((tim_t *) \
		(ap->loc_tim_tbl.system_ptr + 2 * com_ptr->resource_index)), \
		           com_ptr->TCE_count);

	} else {			/* spanned command */

	    struct buf	*next_bp = bp->bp;
	    tim_t	*cur_ptr;
	    int		cur_no   = com_ptr->TCE_count;
            int		tmp_no;

	    cur_ptr = (tim_t *) (ap->loc_tim_tbl.system_ptr +
                           2 * com_ptr->resource_index);
	    TRACE_1("spanned bp", (int) bp)
	    while (next_bp && cur_no >= 0) {
		if ((tmp_no = BUILD_TIM(ap, com_ptr, next_bp, cur_ptr, cur_no)) 
			< 0) {
		    tim_no = tmp_no;
		    break;
		}
		cur_no  -= tmp_no;
		tim_no  += tmp_no;
		cur_ptr += tmp_no;
		next_bp =  next_bp->av_forw;

	    }
	}

	if (tim_no < 0) {

	    /* Table build failed, most likely is a xmem problem */
	    P8printf(P8_EXIT,("Leaving p8xx_prepare routine.\n"));
	    TRACE_1("fail prepare", (int) tim_no)
	    return(-1);

	}

	com_ptr->tim_tbl_used = tim_no;

	/*
	 * Save a copy of the TIM.  This will be used to hold the "saved"
	 * version of the data pointer information.
	 */
	if (tim_no > 0) {
	    bcopy(ap->loc_tim_tbl.system_ptr + 2 * com_ptr->resource_index, 
		  ap->sav_tim_tbl.system_ptr + 2 * com_ptr->resource_index,
		  tim_no * sizeof(tim_t));
	}

    } else {
	com_ptr->tim_tbl_used = 0;
    }

    P8printf(P8_EXIT,("Leaving p8xx_prepare routine.\n"));
    TRACE_1("out prepare", (int) com_ptr->tim_tbl_used)

    return(com_ptr->tim_tbl_used);
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_iodone						*/
/*									*/
/* FUNCTION:	Adapter Driver Iodone Routine				*/
/*									*/
/*	This routine handles completion of commands initiated through	*/
/*	the p8xx_ioctl routine.						*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine runs on the IODONE interrupt level, so it can	*/
/*	be interrupted by the interrupt handler.			*/
/*									*/
/* DATA STRUCTURES:							*/
/*	sc_buf	- input/output request struct used between the adapter	*/
/*		  driver and the calling SCSI device driver		*/
/*									*/
/* INPUTS:								*/
/*	bp	- pointer to the passed sc_buf				*/
/*									*/
/* RETURN VALUE DESCRIPTION:  none					*/
/*									*/
/* ERROR DESCRIPTION:  The following errno values may be returned:	*/
/*	none								*/
/*									*/
/************************************************************************/
void
p8xx_iodone(
	struct sc_buf	*bp)
{
	P8printf(P8_ENTRY,("Entering p8xx_iodone routine.\n"));
	e_wakeup(&bp->bufstruct.b_event);
	P8printf(P8_EXIT,("Leaving p8xx_iodone routine.\n"));
}
/**************************************************************************/
/*                                                                        */
/* NAME:        p8xx_alloc_resources                                      */
/*                                                                        */
/* FUNCTION:    Calls the appropriate routines to allocate resources.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts must be disabled.                                      */
/*                                                                        */
/* NOTES:  This routine is called to the determine what resources are re- */
/*         quired for allocation to take place.  Upon completion,         */
/*         d_master will have been called, and the DMA address is saved   */
/*         along with other needed information in the device structure.   */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    sc_buf structure - pointer to sc_buf received from caller.          */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the sc_buf, or NULL, if it could not         */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    tag             - Allocation of resources successful.               */
/*    PSC_NO_TAG_FREE - A command tag could not be allocated.             */
/*    PSC_NO_TCE_FREE - Allocation unsuccessful due to a lack of TCEs.    */
/*    PSC_NO_STA_FREE - A small transfer error was unavailable            */
/*                                                                        */
/**************************************************************************/
int
p8xx_alloc_resources(
    		    struct sc_buf *bp,
                    struct dev_info * dev_ptr)
{
    ads_t   *ap;
    int tag;
    struct cmd_info *com_ptr;


    ap = dev_ptr->ap;
    TRACE_1 ("in allc ", (int) dev_ptr)
    /* So, try to allocate a cmd_info structure, i.e. a TAG */
    tag = p8xx_alloc_tag (bp, ap);
    if (tag == PSC_NO_TAG_FREE)
    {
       TRACE_1 ("out allc", PSC_NO_TAG_FREE)
       return(PSC_NO_TAG_FREE);
    }

    /* Miscellaneous initialization in the cmd_info structure */
    com_ptr = &ap->command[tag];
    com_ptr->bp = bp;
    com_ptr->preempt_counter = PSC_MAX_PREEMPTS;
    com_ptr->active_fwd = NULL;
    com_ptr->active_bkwd = NULL;
    com_ptr->device = dev_ptr;
    com_ptr->next_dev_cmd = NULL;
    com_ptr->flags = 0;

    if (bp->bufstruct.b_bcount > 0) {
        /* Must be a normal data transfer if execution */
        /* reached here.                               */
        if (p8xx_alloc_iovec(ap, com_ptr) == PSC_NO_TCE_FREE)
        {   /* Not enough TCE's available */
            TRACE_1 ("out allc", PSC_NO_TCE_FREE)
            p8xx_FREE_TAG(com_ptr);
            return (PSC_NO_TCE_FREE);
        }
    } else {
	/*
	 * (bp->bufstruct.b_bcount == 0)
         * there is not data to transfer, thus no
         * need to allocate resources.
	 */
        com_ptr->resource_state = NO_RESOURCES_USED;
    }

    TRACE_1 ("out allc", PSC_NO_ERR)
    return (tag);
}  /* p8xx_alloc_resources */

/************************************************************************
 *								
 * NAME:	p8xx_alloc_iovec
 *						
 * FUNCTION:    Calculates and reserves the required number of iovecs that
 *		will be needed for any call to D_MAP_LIST.  Handles blocking
 *		if a prior transfer could not reserve enough iovecs.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.
 *						
 * DATA STRUCTURES:			
 *	iovec_alloc - array of bits representing reserved iovecs	
 *					
 * INPUTS:			
 *      ap	   - pointer to per adapter data structure
 *	com_ptr    - pointer to command information structure
 *				
 * RETURN VALUE DESCRIPTION:
 *    index - Able to allocate iovec(s), returns bit index.
 *    PSC_NO_TCE_FREE - Unable to allocate iovec(s).
 *                  
 * ERROR DESCRIPTION:
 *
 **************************************************************************/
int
p8xx_alloc_iovec(
              ads_t * ap,
              struct cmd_info * com_ptr)
{
    struct sc_buf *bp;
    int     cur_bit, cur_word;
    int     index, index1;
    int     keep_looking = TRUE;
    int     mask, free_blk_len_mask;
    int     tmp_table;
    int     baddr, num_iovec, num_large_iovec, i, j;


    /* Compute the number of iovecs required using the  */
    /* buffer address and byte count from the scsi    */
    /* command.                                       */
    bp = com_ptr->bp;
    baddr = (int) bp->bufstruct.b_un.b_addr;
    num_iovec = (((baddr & (PAGESIZE - 1)) + bp->bufstruct.b_bcount - 1) /
               PAGESIZE) + 1;

    if (num_iovec == 1) {
        /* An iovec is allocated from the small iovec area */
	if ( (index = p8xx_getfree (ap->iovec_alloc[0])) < PSC_WORDSIZE) {
          /* a free iovec was found - allocate it */
            com_ptr->resource_index = index;
            com_ptr->TCE_count = 1;

            ALLOC(ap->iovec_alloc[0], index)
            com_ptr->resource_state = SMALL_TCE_RESOURCES_USED;
            TRACE_3 ("sTCE", com_ptr->resource_index, com_ptr->TCE_count, 
			index)
            return (index);      /* iovec allocated */
	}
    }

    /* The iovecs are allocated from the large iovec area */
    /* Note that this allows spill-over into the large area when the small */
    /* area becomes filled. */

    if ((ap->blocked_bp != NULL) && (ap->blocked_bp != bp))
    {
   	/* a previous request from the large iovec area */
	/* remains unsatisfied, so we can't process    */
	/* this request.  Mark the work field and return */
	bp->bufstruct.b_work = LARGE_TCE_RESOURCES_USED;
	TRACE_1("blocked by", (int) ap->blocked_bp)
	return (PSC_NO_TCE_FREE);
    }

    num_large_iovec = num_iovec;

    TRACE_2 ("in TCELG", (int) num_large_iovec, bp->bufstruct.b_bcount)

    /* initialize the loop variables - start at first free iovec (index) */
    cur_word = 1;
    index = p8xx_getfree (ap->iovec_alloc[cur_word]) + PSC_WORDSIZE;

    mask = 0xFFFFFFFF >> (index-PSC_WORDSIZE+1);
    free_blk_len_mask = (~(ap->iovec_alloc[cur_word])) & mask;
    /* free_blk_len_mask has a one at the bit after the end of the free */
    /* block that starts at index. */
    TRACE_2("inx fmsk", index, free_blk_len_mask)

    /* Scan the alloc table for a free block of resources */
    /* index is the first free slot of the block */
    /* index1 is one past the last free slot of the block */
    while (keep_looking) 
    {
        /* free_blk_len_mask - the first n '0's will remain '0'             */
        /*            the rest of word will be inverted after the first '1' */
        /* ex - free_blk_len_mask (0000 1111 0000) == 0000 0000 1111        */
        /* free_blk_len_mask is used to find the length of the free block   */
	cur_bit = p8xx_getfree (free_blk_len_mask);
	index1 = cur_word * PSC_WORDSIZE + cur_bit;

        /* make sure it didn't overflow */
        if (index1 > NUM_IOVEC) {
            index1 = NUM_IOVEC;
            keep_looking = FALSE;
        }

	if ( index1 - index >= num_large_iovec)
	{
	    /* The iovecs were found and a pointer to   */
	    /* first is stored in the dev_info struct. */
	    com_ptr->resource_index = index;
	    com_ptr->TCE_count = num_large_iovec;

	    /* allocate it and return where it starts.  Outside loop */
	    /* is for number of words, inner loop is for bits in the word */
	    index1 = index % PSC_WORDSIZE;
	    for ( i=index / PSC_WORDSIZE; i <= cur_word; i++ )
	    {
	        for ( j=index1; j<PSC_WORDSIZE && num_large_iovec != 0; 
		    j++, num_large_iovec--)
	           ALLOC(ap->iovec_alloc[i],j)
	        index1 = 0;	/* reset index1 after first longword filled */
	    }
	    com_ptr->resource_state = LARGE_TCE_RESOURCES_USED;
	    TRACE_2 ("out TCE ", com_ptr->resource_index, com_ptr->TCE_count)
	    return (index);
       	}
       	else if (free_blk_len_mask == 0)
       	{
	    /* we haven't reached the end of an unallocated block,   */
	    /* but so far the block is too small.  Advance to the    */
	    /* next word in the table to see if there enough         */
	    /* contiguous resources.                                 */
	    cur_word++;
            free_blk_len_mask = ~ap->iovec_alloc [cur_word];
	}
	else 
	{
	    /* the current block is not big enough */
	    /* see if there is another             */

            if (cur_bit >= PSC_WORDSIZE) {
                /* advance to the next longword */
                cur_word++;
                tmp_table = ap->iovec_alloc [cur_word];
	    }
	    else
   	        /* there may still be some unallocated resources in the */
	      	/* current longword.  Mask out up to cur_bit. */
	        tmp_table = ap->iovec_alloc [cur_word] & 
	 		(0xFFFFFFFF >> cur_bit);

	    cur_bit = p8xx_getfree(tmp_table);
	    mask = 0xFFFFFFFF >> (cur_bit+1);
            free_blk_len_mask = (~tmp_table) & mask;
	    index = cur_word * PSC_WORDSIZE + cur_bit;
	}
    } /* while */

    /* large allocation has failed */
    ap->blocked_bp = bp;
    bp->bufstruct.b_work = LARGE_TCE_RESOURCES_USED;

    TRACE_1 ("NO TCEs!", (int) com_ptr)
    return (PSC_NO_TCE_FREE);     /* Not enough TCEs available */
}


/**************************************************************************/
/*                                                                        */
/* NAME:        p8xx_free_resources                                       */
/*                                                                        */
/* FUNCTION:    Calls the appropriate routines to free all resources.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts must be disabled.                                      */
/*                                                                        */
/* NOTES:  This routine is called to the determine what resources are to  */
/*         be freed.  Pointers and save locations are initialized         */
/*         to the unused state.                                           */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    command structure - pointer to command information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    None                                                                */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p8xx_free_resources(struct cmd_info * com_ptr)
{
    ads_t *ap = com_ptr->device->ap;

    TRACE_1 ("in free ", (int) com_ptr)

    switch (com_ptr->resource_state)
    {
        case SMALL_TCE_RESOURCES_USED:
        case LARGE_TCE_RESOURCES_USED:
            p8xx_free_iovec(ap, com_ptr);
            break;
        case NO_RESOURCES_USED:
        default:
            /* No action is required */
            break;
    }

    com_ptr->resource_state = 0;
    p8xx_FREE_TAG(com_ptr);
    TRACE_1 ("out free", 0)
}  /* p8xx_free_resources */

/**************************************************************************/
/*                                                                        */
/* NAME:  p8xx_free_iovec                                                 */
/*                                                                        */
/* FUNCTION:  Free iovecs after command completion.                       */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts must be disabled.                                      */
/*                                                                        */
/* NOTES:  This routine is called when freeing resources for a command.   */
/*         The reserved iovecs will be freed to the proper table.         */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    ap_str structure - adapter information structure.                   */
/*                                                                        */
/* INPUTS:                                                                */
/*    com_ptr - pointer to command structure                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p8xx_free_iovec(
             ads_t *ap,
             struct cmd_info * com_ptr)
{
    int     word, bit;

    /* assumes that free_iovec is not called if no iovecs were used */

    /* let the system know that the mapping is no longer required */
    ap->blist.used_iovecs = com_ptr->tim_tbl_used;
 
    ap->blist.dvec = (struct d_iovec *) ap->blist_base_vector +
                         com_ptr->resource_index;
    TRACE_1("blist.dvec", (int) ap->blist.dvec)

    D_UNMAP_LIST(ap->handle, &ap->blist);

    if (com_ptr->resource_state == SMALL_TCE_RESOURCES_USED) {
	/*
         * The iovec was allocated from the small area.
         * Return this iovec to the small iovec area.    
	 */
        FREE(ap->iovec_alloc[com_ptr->resource_index/PSC_WORDSIZE],
	     com_ptr->resource_index % PSC_WORDSIZE)
    } else {
	/*
         * The iovecs are allocated from the large iovec area
         * Return the iovecs to the large iovec area.
	 */
	TRACE_2("index cnt", com_ptr->resource_index, com_ptr->TCE_count)
	word = com_ptr->resource_index / PSC_WORDSIZE; 
	bit = com_ptr->resource_index % PSC_WORDSIZE; 
	while (com_ptr->TCE_count > 0) {
            for ( ; bit < PSC_WORDSIZE && com_ptr->TCE_count > 0;
                          bit++, com_ptr->TCE_count--)
		FREE(ap->iovec_alloc[word], bit) 
	    /* advance to the next long word in the alloc table */
	    bit = 0; 
	    word++;
        }
    }
    /* Clear the iovec mgmt information in the cmd_info struct. */
    com_ptr->resource_index = 0;
    com_ptr->tim_tbl_used = 0;
    com_ptr->TCE_count = 0;
    return;
}
/**************************************************************************/
/*                                                                        */
/* NAME:  p8xx_alloc_tag                                                  */
/*                                                                        */
/* FUNCTION:  Allocates a command tag					  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* NOTES:  This routine allocates a command tag. 	                  */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_info structure - pointer to command information structure       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    Value >= 0  - the allocated TAG.                                    */
/*    PSC_NO_TAG_FREE - Unable to allocate TAG.                           */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p8xx_alloc_tag(
   		struct sc_buf * bp,
		ads_t *ap)
{
  int i, index, tag;

  tag = PSC_NO_TAG_FREE;

  /* Check if it is a command with command queuing or not */
  if ( bp->q_tag_msg != SC_NO_Q ) {
     /* Command queuing, so allocate from pool */
      for (i = START_Q_TAGS; i < TAG_TABLE_SIZE; i++) {
	  index = p8xx_getfree (ap->TAG_alloc[i]);
     	  if (index < PSC_WORDSIZE)
     	  {  /* a free tag was found - so allocate it */
		ALLOC(ap->TAG_alloc[i], index)
        	tag = i * PSC_WORDSIZE + index;
        	TRACE_1 ("Tag Q!", (int) tag)
		ap->command[tag].in_use = TRUE;
	        break;
     	  }
      }
  } else {
     /* No command queuing so allocate the dedicated command struct */
     index = INDEX(bp->scsi_command.scsi_id,
                       (bp->scsi_command.scsi_cmd.lun) >> 5);
     if (!IN_USE(ap->TAG_alloc[index / PSC_WORDSIZE], 
		index % PSC_WORDSIZE)) 
     {
	ALLOC(ap->TAG_alloc[index / PSC_WORDSIZE], index % PSC_WORDSIZE)
	tag = index;
	ap->command[tag].in_use = TRUE;
        TRACE_1 ("Tag NQ!", (int) tag)
     }
  }
#ifdef P8_TRACE
  if (tag == PSC_NO_TAG_FREE)
	TRACE_1("No Tag free", (int) bp->q_tag_msg)
#endif

  return (tag);
}


/**************************************************************************/
/*									  */
/* NAME: p8xx_enq_active, p8xx_enq_wait, p8xx_enq_abort_bdr		  */
/*									  */
/* FUNCTION:								  */
/*	Utility routines to handle queuing of device structures to each	  */
/*	of the queues in use.						  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*									  */
/* DATA STRUCTURES:							  */
/*    adapter_def structure - pointer to per adapter information	  */
/*									  */
/* INPUTS:								  */
/*    dev_info structure - pointer to device information structure	  */
/*    cmd_info structure - pointer to command information structure	  */
/*									  */
/* RETURN VALUE DESCRIPTION:  none					  */
/*									  */
/* ERROR DESCRIPTION:  The following errno values may be returned:	  */
/*	none								  */
/*									  */
/**************************************************************************/

void
p8xx_enq_active(
               struct dev_info * dev_ptr,
               struct cmd_info * com_ptr)
{
    ads_t       *ap;

    ap = dev_ptr->ap;
    TRACE_3 ("enqA", (int) com_ptr->tag, (int) com_ptr, 
                     (int) dev_ptr->active_head)
    P8printf(P8_ENTRY,("p8xx_enq_active dev_ptr 0x%x\n", dev_ptr));

    if (dev_ptr->active_head == NULL)  /* only ENQ for first command */
    {
        if (ap->DEVICE_ACTIVE_head == NULL)
        {
            dev_ptr->DEVICE_ACTIVE_fwd = NULL;
            dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
            ap->DEVICE_ACTIVE_head = dev_ptr;
            ap->DEVICE_ACTIVE_tail = dev_ptr;
	    LIGHT_ON(ap);
        }
        else 
        {
            dev_ptr->DEVICE_ACTIVE_bkwd = ap->DEVICE_ACTIVE_tail;
            dev_ptr->DEVICE_ACTIVE_fwd = NULL;
            ap->DEVICE_ACTIVE_tail->DEVICE_ACTIVE_fwd = dev_ptr;
            ap->DEVICE_ACTIVE_tail = dev_ptr;
	}

        com_ptr->active_fwd = NULL;
        com_ptr->active_bkwd = NULL;
        dev_ptr->active_head = com_ptr;
        dev_ptr->active_tail = com_ptr;
    }
    else
    {
        com_ptr->active_bkwd = dev_ptr->active_tail;
        com_ptr->active_fwd = NULL;
        dev_ptr->active_tail->active_fwd = com_ptr;
        dev_ptr->active_tail = com_ptr;
    }
    TRACE_2 ("out enqA", (int) dev_ptr, (int)dev_ptr->active_head)
}

/************************************************************************/
void
p8xx_enq_wait(
             struct cmd_info * com_ptr)
{
    struct dev_info * dev_ptr;
    ads_t       *ap;

    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;
    P8printf(P8_ENTRY,("p8xx_enq_wait dev_ptr 0x%x\n", dev_ptr));

    TRACE_2 ("in enqW", (int) com_ptr, (int) com_ptr->device)
    if (ap->DEVICE_WAITING_head == NULL) {
         ap->DEVICE_WAITING_head = com_ptr;
    } else {
        ap->DEVICE_WAITING_tail->active_fwd = com_ptr;
    }
    com_ptr->active_fwd = NULL;
    ap->DEVICE_WAITING_tail = com_ptr;

    /* enqueue on chain of waiting commands attached to a particular dev_ptr */
    if (dev_ptr->wait_head == NULL) {
	dev_ptr->wait_head = com_ptr;
    } else {
	dev_ptr->wait_tail->next_dev_cmd = com_ptr; 
    }
    com_ptr->next_dev_cmd = NULL;
    dev_ptr->wait_tail = com_ptr;

    TRACE_2 ("out enqW", (int) dev_ptr->wait_head, (int) dev_ptr->wait_tail)
    TRACE_2 ("out enqW", (int) com_ptr, (int) com_ptr->device)
}

/************************************************************************/
void
p8xx_enq_wfr(
	    struct sc_buf * bp,
            ads_t       *ap)
{

    P8printf(P8_ENTRY,("p8xx_enq_wfr bp 0x%x\n", bp));

    TRACE_1 ("in enqWFR ", (int) bp)
    if (ap->REQUEST_WFR_head == NULL) {
         ap->REQUEST_WFR_head = bp;
         ap->REQUEST_WFR_tail = bp;
    } else {
         ap->REQUEST_WFR_tail->bufstruct.av_forw = (struct buf *)bp;
         ap->REQUEST_WFR_tail = bp;
    }
    TRACE_1 ("out eqWR", 0)
}

/************************************************************************/
/*
 * Name:        p8xx_enq_abort_bdr
 * Function:    Place dev_ptr on the adapter's Bus Device Reset queue.
 * Input:       dvi_t *dev_ptr  dev_ptr to enqueue.
 * Returns:     nothing.
 */
void
p8xx_enq_abort_bdr(
                  struct dev_info * dev_ptr)
{
    ads_t       *ap;

    ap = dev_ptr->ap;
    P8printf(P8_ENTRY|P8_ABORT,("p8xx_enq_abort_bdr: ID 0x%02x LUN 0x%02x\n",
        dev_ptr->scsi_id, dev_ptr->lun_id));

    TRACE_1 ("in enqB ", (int) dev_ptr)
    dev_ptr->queue_state &= STOPPING;
    dev_ptr->queue_state |= WAIT_FLUSH;
    if (ap->ABORT_BDR_head == NULL) {
        dev_ptr->ABORT_BDR_fwd = NULL;
        dev_ptr->ABORT_BDR_bkwd = NULL;
        ap->ABORT_BDR_head = dev_ptr;
        ap->ABORT_BDR_tail = dev_ptr;
    } else {
        dev_ptr->ABORT_BDR_bkwd = ap->ABORT_BDR_tail;
        dev_ptr->ABORT_BDR_fwd = NULL;
        ap->ABORT_BDR_tail->ABORT_BDR_fwd = dev_ptr;
        ap->ABORT_BDR_tail = dev_ptr;
    }
    TRACE_1 ("out enqB", (int) dev_ptr)
}

/**************************************************************************/
/*                                                                        */
/* NAME: p8xx_deq_active, p8xx_deq_wait, p8xx_deq_wfr, p8xx_deq_abort_bdr */
/*                                                                        */
/* FUNCTION:                                                              */
/*      Utility routines to handle queuing.                               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* ERROR DESCRIPTION:  The following errno values may be returned:        */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
void
p8xx_deq_active(
               struct dev_info * dev_ptr,
               struct cmd_info * com_ptr)
{
    ads_t       *ap;

    P8printf(P8_ENTRY,("p8xx_deq_active dev_ptr 0x%x\n", dev_ptr));

    ap = dev_ptr->ap;

    TRACE_2 ("in deqA ", (int) dev_ptr, (int) com_ptr)
    TRACE_1 ("act head", (int) dev_ptr->active_head)

    if (dev_ptr->active_head == dev_ptr->active_tail) /* only one */
    {
        dev_ptr->active_head = NULL;
        dev_ptr->active_tail = NULL;
    }
    else if (dev_ptr->active_head == com_ptr)  /* first one */
    {
        dev_ptr->active_head = com_ptr->active_fwd;
	com_ptr->active_fwd->active_bkwd = com_ptr->active_bkwd;
    }
    else if (dev_ptr->active_tail == com_ptr) /* last one */
    {
	dev_ptr->active_tail = com_ptr->active_bkwd;
	com_ptr->active_bkwd->active_fwd = com_ptr->active_fwd;
    }
    else        /* in the middle */
    {
 	com_ptr->active_bkwd->active_fwd = com_ptr->active_fwd;
	com_ptr->active_fwd->active_bkwd = com_ptr->active_bkwd;
    }
    com_ptr->active_fwd = NULL;
    com_ptr->active_bkwd = NULL;

    /* remove dev_ptr if this was the last command */
    TRACE_1 ("act head", (int) dev_ptr->active_head)
    if (dev_ptr->active_head == NULL)
    {
        if (ap->DEVICE_ACTIVE_head == ap->DEVICE_ACTIVE_tail)
        {
            ap->DEVICE_ACTIVE_head = NULL;
            ap->DEVICE_ACTIVE_tail = NULL;
	    LIGHT_OFF(ap);
        }
        else if (ap->DEVICE_ACTIVE_head == dev_ptr)      /* first one */
        {
            ap->DEVICE_ACTIVE_head = dev_ptr->DEVICE_ACTIVE_fwd;
            dev_ptr->DEVICE_ACTIVE_fwd->DEVICE_ACTIVE_bkwd =
                                                dev_ptr->DEVICE_ACTIVE_bkwd;
        }
        else if (ap->DEVICE_ACTIVE_tail == dev_ptr)  /* last one */
        {
            ap->DEVICE_ACTIVE_tail = dev_ptr->DEVICE_ACTIVE_bkwd;
            dev_ptr->DEVICE_ACTIVE_bkwd->DEVICE_ACTIVE_fwd =
                                             dev_ptr->DEVICE_ACTIVE_fwd;
        }
        else        /* in the middle */
        {
            dev_ptr->DEVICE_ACTIVE_bkwd->DEVICE_ACTIVE_fwd =
                                             dev_ptr->DEVICE_ACTIVE_fwd;
            dev_ptr->DEVICE_ACTIVE_fwd->DEVICE_ACTIVE_bkwd =
                                            dev_ptr->DEVICE_ACTIVE_bkwd;
        }
        dev_ptr->DEVICE_ACTIVE_fwd = NULL;
        dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
        dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;
    }
    TRACE_1 ("out deqA", (int) dev_ptr)
}

/************************************************************************/
void
p8xx_deq_wait(
             struct cmd_info * com_ptr)
{
    struct dev_info * dev_ptr;
    struct cmd_info * cur_com_ptr, * prev_com_ptr;
    ads_t       *ap;

    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;
    P8printf(P8_ENTRY,("p8xx_deq_wait dev_ptr 0x%x\n", dev_ptr));

    TRACE_2 ("in deqW ", (int) com_ptr, (int) ap->DEVICE_WAITING_head)

    TRACE_2 ("dev wait", (int) dev_ptr->wait_head, 
		      (int) dev_ptr->wait_tail)

    /* dequeue cmd_info from wait queue anchored in dev_info structure */
    if (dev_ptr->wait_head == dev_ptr->wait_tail)
    {
	dev_ptr->wait_head = NULL;
	dev_ptr->wait_tail = NULL;
    }
    else
    {
	dev_ptr->wait_head = com_ptr->next_dev_cmd;
    }
    com_ptr->next_dev_cmd = NULL;

    /* dequeue command from global queue */
    /* only element in the list */
    if (ap->DEVICE_WAITING_head == ap->DEVICE_WAITING_tail)
    {
        ap->DEVICE_WAITING_head = NULL;
        ap->DEVICE_WAITING_tail = NULL;
    }
    /* first element in the list */
    else if (com_ptr == ap->DEVICE_WAITING_head)
    {
        ap->DEVICE_WAITING_head = com_ptr->active_fwd;
    }
    /* not the first element in the list */
    else
    {
        cur_com_ptr = ap->DEVICE_WAITING_head;
        while (cur_com_ptr != com_ptr)
        {
            prev_com_ptr = cur_com_ptr;
            cur_com_ptr = cur_com_ptr->active_fwd;
        }
        /* the element has been found - remove it from chain */
        prev_com_ptr->active_fwd = com_ptr->active_fwd;

        /* if this is the last element in the wait q, reset  */
        /* the tail pointer */
	if (com_ptr == ap->DEVICE_WAITING_tail)
	    ap->DEVICE_WAITING_tail = prev_com_ptr;
    }
    com_ptr->active_fwd = NULL;
    TRACE_2 ("out devw", (int) dev_ptr->wait_head, 
		      (int) dev_ptr->wait_tail)
    TRACE_2 ("out deqW", (int) com_ptr, (int) ap->DEVICE_WAITING_head)
}

/************************************************************************/
void
p8xx_deq_wfr(
	    struct sc_buf * bp,
	    struct sc_buf * prev_bp,
            ads_t       *ap)
{
    TRACE_2 ("deqWFR ", (int) bp, (int) prev_bp)

    if (prev_bp == NULL)   /* bp is first in list */
    {
         ap->REQUEST_WFR_head =
		(struct sc_buf *) bp->bufstruct.av_forw;
    }
    else 		  /* in middle or end of list */
    {
        prev_bp->bufstruct.av_forw = bp->bufstruct.av_forw;
    }

    if (ap->REQUEST_WFR_tail == bp)   /* bp is at end of list */
            ap->REQUEST_WFR_tail = prev_bp;
	    
    bp->bufstruct.av_forw = NULL;

    if (ap->blocked_bp == bp)
	ap->blocked_bp = NULL;
}

/************************************************************************/
/*
 * Name:	p8xx_deq_abort_bdr
 * Function:	Removes a dev_ptr from the adapter's abort Bus Device Reset
 *		queue.
 * Input:	dvi_t *dev_ptr	dev_ptr to be removed.
 * Returns:	nothing.
 */
void
p8xx_deq_abort_bdr(
                  struct dev_info * dev_ptr)
{
    ads_t       *ap;

    ap = dev_ptr->ap;
    P8printf(P8_ENTRY,("p8xx_deq_abort_bdr dev_ptr 0x%x\n", dev_ptr));

    TRACE_1 ("in deqB ", (int) dev_ptr)
    if (ap->ABORT_BDR_head == ap->ABORT_BDR_tail) {
        ap->ABORT_BDR_head = NULL;
        ap->ABORT_BDR_tail = NULL;
    } else if (ap->ABORT_BDR_head == dev_ptr) {
        /* first one */
        ap->ABORT_BDR_head = dev_ptr->ABORT_BDR_fwd;
        dev_ptr->ABORT_BDR_fwd->ABORT_BDR_bkwd = dev_ptr->ABORT_BDR_bkwd;
    } else if (ap->ABORT_BDR_tail == dev_ptr) {
	/* last one */
        ap->ABORT_BDR_tail = dev_ptr->ABORT_BDR_bkwd;
        dev_ptr->ABORT_BDR_bkwd->ABORT_BDR_fwd = dev_ptr->ABORT_BDR_fwd;
    } else {
        /* in the middle */
        dev_ptr->ABORT_BDR_bkwd->ABORT_BDR_fwd = dev_ptr->ABORT_BDR_fwd;
        dev_ptr->ABORT_BDR_fwd->ABORT_BDR_bkwd = dev_ptr->ABORT_BDR_bkwd;
    }
    dev_ptr->ABORT_BDR_fwd = NULL;
    dev_ptr->ABORT_BDR_bkwd = NULL;
    TRACE_1 ("out deqB", (int) dev_ptr)
}

/**************************************************************************/
/*									  */
/* NAME: p8xx_dump							  */
/*									  */
/* FUNCTION: Determine what type of dump operation is being sought	  */
/*									  */
/*	Allocate necessary segment registers and parse type of		  */
/*	dump operation.	 Call the specified routine.			  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*									  */
/*	This routine must be called in the interrupt environment.  It	  */
/*	can not page fault and is pinned.				  */
/*									  */
/* (NOTES:) This routine handles the following operations :		  */
/*	DUMPINIT   - initializes bus attached disk as dump device	  */
/*	DUMPSTART  - prepares device for dump				  */
/*	DUMPQUERY  - returns the maximum and minimum number of bytes that */
/*		     can be transferred in a single DUMPWRITE command	  */
/*	DUMPWRITE  - performs write to disk				  */
/*	DUMPEND	   - cleans up device on end of dump			  */
/*	DUMPTERM   - terminates the bus attached disk as dump device	  */
/*									  */
/* (DATA STRUCTURES:) uio	      - structure containing information  */
/*					about the data to transfer	  */
/*		      adapter_def    - adapter info structure		  */
/*		      dev_info	     - device info structure		  */
/*		      sc_buf	     - i/o request struct between driver  */
/*				       and device			  */
/*		      dmp_query	      - queried transfer information is	  */
/*					returned			  */
/*									  */
/* INPUTS:								  */
/*	devno	- device major/minor number				  */
/*	uiop	- pointer to uio structure for data for the		  */
/*		  specified operation code				  */
/*	cmd	- type of dump operation being requested		  */
/*	arg	- ptr to dump query struct or sc_buf, or id/lun encoding  */
/*	chan	- unused						  */
/*	ext	- indicates normal or power mgmt dump			  */
/*									  */
/* INTERNAL PROCEDURES CALLED:						  */
/*	p8xx_dumpstrt							  */
/*	p8xx_dumpwrt							  */
/*	p8xx_dumpend							  */
/*									  */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned */
/*	and the caller is left responsible to recover from the error.	  */
/*	For a dump_write, the sc_buf also contains error-related status.  */
/*									  */
/* RETURNS:								  */
/*	0 for success, errno otherwise					  */
/*									  */
/**************************************************************************/
int
p8xx_dump(
	dev_t		devno,
	struct uio	*uiop,
	int		cmd,
	int		arg,
	int		chan,
	int		ext)
{
    struct sc_buf *scbuf_ptr;
    struct dmp_query *dmp_qry_ptr;
    ads_t	*ap;
    dvi_t	*dev_ptr;
    int		dev_index;
    int		ret_code = PSC_NO_ERR;
    int		old_level;

    P8printf(P8_ENTRY,("Entering p8xx_dump\n"));

    ap = p8xx_get_adp(devno);
    assert(ap != NULL);
    TRACE_1 ("in p8xx_dump", devno);

    /* check to make sure adapter is inited and opened */
    if ((!ap->str_inited) || (!ap->opened))
	return (ENXIO);
    switch (cmd) {

	case DUMPINIT:
            TRACE_2 ("DUMPINIT", (int) &arg, ext)
	    break;

	case DUMPQUERY:
	    dmp_qry_ptr = (struct dmp_query *) arg;
	    dmp_qry_ptr->min_tsize = 512;
	    dmp_qry_ptr->max_tsize = ap->max_request;
            TRACE_2 ("DUMPQUERY", (int) &arg, ext)
	    break;

	case DUMPSTART:
	    old_level = i_disable(INTMAX);
	    ret_code = p8xx_dumpstrt(ap, arg, ext);
	    i_enable(old_level);
	    break;

	case DUMPWRITE:
	    old_level = i_disable(INTMAX);
	    ret_code = p8xx_dumpwrt(ap, (struct sc_buf *) arg, ext);
	    i_enable(old_level);
	    break;

	case DUMPEND:
            TRACE_2 ("DUMPEND", (int) &arg, ext)
            P8if(0x00020000,(brkpoint()));

	    if (!ap->dump_started)
		ret_code = EINVAL;
	    else {
		ap->dump_started = FALSE;
	 	if (ext == PM_DUMP) 
		    p8xx_dumpend(ap);
            }
	    break;

	case DUMPTERM:
            TRACE_1 ("DUMPTERM", (int) &arg)
	    break;

	default:
	    ret_code = EINVAL;
    }

    P8printf(P8_EXIT,("Leaving p8xx_dump\n"));
    TRACE_1 ("out p8xx_dump", ret_code)
    return (ret_code);

}  /* end p8xx_dump */


/************************************************************************/
/*									*/
/* NAME: p8xx_dumpstrt							*/
/*									*/
/* FUNCTION: Notify device of pending dump				*/
/*									*/
/*	Prepare device for dump, fail existing active commands on all   */
/*	devices, including the dump device.  Make sure a tag and other  */
/*	resources will be available for the subsequent dump commands.   */
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*									*/
/*	This routine must be called in the interrupt environment.  It	*/
/*	can not page fault and is pinned				*/
/*									*/
/* (DATA STRUCTURES:)							*/
/*		      adapter_def    - adapter info structure		*/
/*		      sc_buf	- controller info structure		*/
/*		      dev_info	  - device info structure		*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	address of the adapter structure		*/
/*	arg		argument passed via devdump, encoding the       */
/*			     target scsi device's id and lun		*/
/*	ext		parameter passed via devdump                    */
/*									*/
/* CALLED BY:								*/
/*	p8xx_dump							*/
/*									*/
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is	*/
/*	returned and the caller is left responsible to recover from the	*/
/*	error.  Note the goal on exiting this routine is to have the    */
/*	needed resources available for the coming dump, and no active   */
/*	cmds.  Errors that do not prevent these goals are ignored.      */
/*									*/
/* RETURNS:								*/
/*	EIO	  -  Severe adapter state error or chip hung	        */
/*	EINVAL	  -  Invalid arg on a PM_DUMP call			*/ 
/*									*/
/************************************************************************/
int
p8xx_dumpstrt(ads_t	*ap,
	      int	arg,
	      int	ext)
{
    dvi_t	*dev_ptr;
    cmd_t	*com_ptr;
    int		ret_code = PSC_NO_ERR;
    int 	dev_index;

    P8printf(P8_ENTRY,("Entering p8xx_dumpstrt\n"));

    TRACE_2 ("DUMPSTART", (int) &arg, ext)
    ap->dump_started |= P8XX_DMP_STARTING;

    if (ext == PM_DUMP) {
	if (!(ap->dump_started & P8XX_PM_DMP)) {
	    ap->dump_started |= P8XX_PM_DMP;
            /*
             * Store dump_dev_ptr, make sure cmd resources
	     * will be available for the dump
             */
	    dev_index = INDEX(arg >> 8, arg);
            if ((dev_ptr = ap->device_queue_hash[dev_index]) != NULL) {
	        ap->dump_dev_ptr = dev_ptr;
	        p8xx_empty_cmd_save(ap->dump_dev_ptr);

                p8xx_turn_chip_on(ap, FALSE);
	    } else {
		ret_code = EINVAL;
	    }
	}
	ASSERT(ap->DEVICE_ACTIVE_head == NULL);

    } else if (!(ap->dump_started & P8XX_SYS_DMP)) {
	/* normal system dump */
	if ((ap->pmh.mode != PM_DEVICE_FULL_ON) &&
	    (ap->pmh.mode != PM_DEVICE_ENABLE))
	    /* need to turn chip on ? */
	    p8xx_turn_chip_on(ap, TRUE);

        /*
         * Clear all abort_bdr queues. Then enter abort commands for all 
         * active device queues.
         */
        ap->ABORT_BDR_head = NULL;
        ap->ABORT_BDR_tail = NULL;

	/*
	 * Clear the waiting and wfr queues.  Then empty the cmd
	 * save queue and free any resources that may be allocated.
	 */
        while ((com_ptr = ap->DEVICE_WAITING_head) != NULL) {
            p8xx_freeze_qs(com_ptr->device);
	    p8xx_empty_cmd_save(com_ptr->device);
        }

        /*
         * Queue an abort command for each device on the active queue
         */
        dev_ptr = ap->DEVICE_ACTIVE_head;
        while (dev_ptr != NULL) {
            /* Put an abort command on the ABORT_BDR queue for this device */
            dev_ptr->flags |= SCSI_ABORT;
            p8xx_enq_abort_bdr(dev_ptr);
            dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
        }

  	SUSPEND_CHIP(ap);
	while ((ap->DEVICE_ACTIVE_head != NULL) && 
	       (ret_code != PSC_DUMP_HANG)) {
	    ret_code = p8xx_dump_intr(ap->DEVICE_ACTIVE_head, TRUE);
	}

	/* If we managed to clean the active queue without the chip getting
	 * hung, any errors that occurred are incidental to our goal of
	 * stopping all i/o, and can be ignored.
         */
	if ((ret_code == PSC_DUMP_HANG) || (ap->DEVICE_ACTIVE_head != NULL))
	    ret_code = EIO;
	else
	    ret_code = PSC_NO_ERR;
	ap->dump_started |= P8XX_SYS_DMP;
    }

    P8printf(P8_EXIT,("Leaving p8xx_dumpstrt\n"));
    TRACE_1 ("out dumpstrt", ret_code)
    return (ret_code);

}  /* end p8xx_dumpstrt */

/**************************************************************************/
/*									  */
/* NAME: p8xx_dump_intr							  */
/*									  */
/* FUNCTION: Handle polling for interrupts for dump routines		  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*									  */
/*	This routine may be called in the interrupt or process envt. 	  */
/*	It can not page fault and is pinned.				  */
/*									  */
/* (DATA STRUCTURES:)							  */
/*		      adapter_def    - adapter info structure		  */
/*		      sc_buf	- controller info structure		  */
/*		      dev_info	  - device info structure		  */
/*									  */
/* INPUTS:								  */
/*    dev_info structure - pointer to device information structure,	  */
/*		assumed to be at head of active queue			  */
/*    aborting_cmds - flag indicating whether we are called due to a      */
/*	        DUMPSTRT (and are aborting cmds in preparation for the    */
/*		dump), or due to a DUMPWRITE.				  */
/*									  */
/* CALLED BY:								  */
/*	p8xx_dumpwrt, p8xx_dumpstrt					  */
/*									  */
/* RETURNS:								  */
/*    0 - success, errno otherwise					  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*	Besides the return code, sc_bufs will be marked when an error     */
/*    occurs.  No entries will be made in the system error log.		  */
/*									  */
/**************************************************************************/
int
p8xx_dump_intr(
	dvi_t	*first_dev_ptr,
	int	aborting_cmds)
{
    ads_t	*ap;
    int		dev_index, ret_code,
		loop, i, base_interrupt;
    int		rc, save_isr, save_dsr, save_stat,
		dma_stat, save_interrupt;
    int		start_new_job;			/* 1=start new job up */
    int		multiple_intr,
		time_out_value, restart_point,
		chip_started_flag;
    int		tag;
    cmd_t	*com_ptr;
    struct sc_buf *bp, *rc_bp;
    dvi_t	*dev_ptr;
    saved_t     *move_info_ptr;

    P8printf(P8_ENTRY,("Entering p8xx_dump_intr\n"));

    ap = first_dev_ptr->ap;
    TRACE_1 ("in dumpintr", (int) first_dev_ptr)

    com_ptr = NULL;
    i = 0;
    rc_bp = first_dev_ptr->active_head->bp;

    /* note timeout_value of 0 not supported in the dump context */
    if ((rc_bp) && (rc_bp->timeout_value != 0)) 
        time_out_value = rc_bp->timeout_value;
    else
	time_out_value = LONGWAIT;

    /* loop until the device is removed from the active queue or there
     * is a time-out.
     */
    while ((ap->DEVICE_ACTIVE_head == first_dev_ptr) && 
	   (++i < (time_out_value * 1000))) {
        /* Delay for 1000th of a second */
        io_delay(1000);
        start_new_job = FALSE;

        save_isr = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);

        if (!(save_isr & ISTAT_INTR)) {

            /* If not our interrupt go around again */
            continue;

        } else {

            tag = p8xx_read_reg(ap, SCRATCHA3, SCRATCHA3_SIZE);
            TRACE_1 ("tag is ", tag)

            if (save_isr & ISTAT_SIP) {
		start_new_job = p8xx_sip(ap, save_isr, tag);
	
            }  else if (save_isr & ISTAT_DIP) {
	        /* DMA interrupt */
                save_dsr = p8xx_read_reg(ap, DSTAT, DSTAT_SIZE);
                com_ptr = &ap->command[tag];
                TRACE_1 ("dump dip", save_dsr)

                if (save_dsr & SIR) {

                    /* SCRIPTS interrupt */
                    save_interrupt = p8xx_read_reg(ap, DSPS, DSPS_SIZE);

                    if (com_ptr->in_use == TRUE) {
                        dev_ptr = com_ptr->device;
                        bp = com_ptr->bp;
                        TRACE_3("dint", (int) tag, (int) save_interrupt, 
					(int) bp)
                        TRACE_3("dint", (int) tag, (int) com_ptr, (int) dev_ptr)
                    }     
                    else if ((save_interrupt != A_suspended) &&
                          (save_interrupt != A_unknown_reselect_id) &&
                          (save_interrupt != A_uninitialized_reselect)) {
                          /* tag is not valid or needed for these */
                          /* interrupts that come from the iowait */
                          /* part of the script. */
                        TRACE_2 ("bad tag", (int) tag, (int) save_interrupt)
                        p8xx_command_reset_scsi_bus(ap, 85);
			continue;
                    }

                    switch (save_interrupt) {

                    case A_suspended:
                        TRACE_1 ("suspended", 0)
                        start_new_job = TRUE;
                        break;

                    case A_check_next_io:
                    	TRACE_2 ("chk nxtd", (int) dev_ptr, (int) bp)
                        /*
                         * The target has disconnected from the SCSI bus.
                         */
                        INT_OFF(dev_ptr->sid_ptr->vscript_ptr, 
				A_check_next_io_Used);
                        start_new_job = TRUE;
                        break;

	            case A_io_done_wsr:
		        TRACE_1("dmp io_done wsr", 0)
		        rc = p8xx_recover_byte(ap, com_ptr, bp);
		        if (rc == 0) {
			    /* adjust the nexus, since recover byte modifies */
			    /* only the current pointers */
		            move_info_ptr = (saved_t *) (
                            ((ulong) ap->IND_TABLE.system_ptr + (16 * tag)));

		            move_info_ptr->tim_cnt = 
				LtoCBO(p8xx_read_reg(ap, SCRATCHB,
						 SCRATCHB_SIZE));
		        } else if (rc == PSC_COPY_ERROR) {
			    /* got an error trying to move the byte to memory */
                            if (!(bp->bufstruct.b_flags & B_ERROR)) {
                                /* mark the bp if no previous error */
                                bp->status_validity = 0;
                                bp->bufstruct.b_error = EFAULT;
                                bp->bufstruct.b_flags |= B_ERROR;
                                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                            }
			    /* set up for either an abort or a call to 
			     * fail_cmd, if an abort/bdr isn't already pending
			     */
			    if (!((dev_ptr->flags & SCSI_ABORT) ||
			          (dev_ptr->flags & SCSI_BDR))) {
                            	dev_ptr->flags |= SCSI_ABORT;
 		            	p8xx_enq_abort_bdr(dev_ptr);
			    }
		        }
		        /*
		         * the io_done wsr (io_done while a byte from a wide	
		         * scsi receive has not been transferred across the pci
		         * bus) processing now falls into the normal io_done
		         * processing..
		         */

		    case A_io_done_wss:
		        TRACE_1("dmp io_done wss", 0)
		    case A_io_done:
                        TRACE_2 ("duiodone", (int) dev_ptr, (int) bp)

                        P8printf(P8_INFO_1,("p8xx_dump: Current I/O done\n"));
                        /* Adjust residual */
                        move_info_ptr = (saved_t *) (((ulong) 
				ap->IND_TABLE.system_ptr + (16 * tag)));

                        TRACE_2("scr A, B",
                           p8xx_read_reg(ap, SCRATCHA, SCRATCHA_SIZE),
                           p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE))
                        TRACE_2("dsa dmod",
                           p8xx_read_reg(ap, DSA, DSA_SIZE),
                           p8xx_read_reg(ap, DMODE, DMODE_SIZE))

                        /* adjust the residual from the saved pointers */
                        if (move_info_ptr->tim_cnt == 0) {
                            bp->bufstruct.b_resid = 0;
                        } else {
                            bp->bufstruct.b_resid = p8xx_calc_resid(ap,
                                    LtoCBO(move_info_ptr->tim_cnt), com_ptr);
                        }

		        /* account for any byte on the chip but not sent */
		        if (save_interrupt == A_io_done_wss)
			    bp->bufstruct.b_resid++;
			
                        TRACE_2("nex resd", (int) move_info_ptr, 
				(int) bp->bufstruct.b_resid)

                        save_stat = GET_STATUS(dev_ptr->sid_ptr->vscript_ptr);
                        TRACE_1("status is", (int) save_stat)
                        P8printf(P8_INFO_1, 
				("STATUS BUFFER = 0x%x\n", save_stat));

                        start_new_job = TRUE;
			dev_ptr->flags |= RETRY_ERROR;
                        if (save_stat == SC_GOOD_STATUS) {
                            /* scsi status is ok */
                            TRACE_1 ("du_iodgds", (int) dev_ptr)
                            /* set scsi status in sc_buf */
			    if (!(bp->bufstruct.b_flags & B_ERROR)) {
                                bp->bufstruct.b_error = 0;
                                bp->status_validity = 0;
                                bp->scsi_status = SC_GOOD_STATUS;
			    }

                            p8xx_deq_active(dev_ptr, com_ptr);
                            /* free resources */
                            p8xx_free_resources(com_ptr);

                        } else {   /* status is not okay */

                            TRACE_1 ("diodchk", (int) dev_ptr);
                            /* set scsi status */
                            bp->scsi_status = save_stat;
                            bp->status_validity = SC_SCSI_ERROR;
                            bp->bufstruct.b_error = EIO;
                            bp->bufstruct.b_flags |= B_ERROR;
                            p8xx_deq_active(dev_ptr, com_ptr);
                            p8xx_free_resources(com_ptr);
			    /* 
			     * if other cmds remain active, we are
			     * going to issue an abort, so do not
			     * worry about resolving a possible CAC.
			     * Also, do not worry about renegotiating
			     * on a check condition in the dump context.
			     */
                        }  /* end else bad status */

                        /* if this was the last command, no */
                        /* longer need to send an abort. */
                        if ((dev_ptr->active_head == NULL) &&
                            ((ap->ABORT_BDR_head == dev_ptr) ||
                            (dev_ptr->ABORT_BDR_bkwd != NULL))) {
                            p8xx_deq_abort_bdr(dev_ptr);
                            dev_ptr->flags &= ~SCSI_ABORT;
                            dev_ptr->flags &= ~SCSI_BDR;
                            dev_ptr->queue_state = ACTIVE;
                        }
                        break;

	            case A_save_ptrs_wsr:
	            case A_save_ptrs_wss:
		        p8xx_save_residue(ap, save_interrupt, com_ptr);
		        break;
			
		    case A_wdtr_neg_done:
		        p8xx_wdtr_answer(dev_ptr->sid_ptr->vscript_ptr, com_ptr);
		        break;

		    case A_sdtr_neg_done:
			p8xx_sdtr_neg_done(ap, dev_ptr, com_ptr);
		        break;

		    case A_wdtr_msg_reject:
			p8xx_wdtr_reject(ap, dev_ptr, com_ptr);
		        break;

	            case A_wdtr_msg_ignored:
		        /* the target read in our WDTR, but did not go to 
		         * MSG-IN.  Treat this device as narrow/async. 
			 */
		        TRACE_1("wdtr ignore", (int) dev_ptr->sid_ptr)
		        dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR;
	                dev_ptr->sid_ptr->io_table.scntl3  &= ~SCNTL3_EWS;

	            case A_sdtr_msg_ignored:
		        /* the target read in our SDTR, but did not go to
		         * MSG-IN.  Treat this device as async. 
			 */
		        TRACE_1("sdtr ignore", (int) dev_ptr->sid_ptr)

		    case A_sdtr_msg_reject:
		    case A_sync_msg_reject:
			p8xx_sdtr_reject(ap, dev_ptr, com_ptr);
		        break;

		    case A_ignore_residue:
		        p8xx_ignore_residue(com_ptr);
		        break;

		    case A_ext_msg:
		    case A_unknown_ext_msg:
		    case A_modify_data_ptr:
		    case A_target_sdtr_sent:
		    case A_target_wdtr_sent:
		        p8xx_handle_extended_messages(com_ptr, save_interrupt);
		        break;

                    case A_abort_io_complete:
                        TRACE_1 ("dabrt cmp", (int) dev_ptr);
                        dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;

                        /* flush the aborted LUN */
                        p8xx_fail_cmd(dev_ptr);
                        start_new_job = TRUE;
			break;

                    case A_bdr_io_complete:
			p8xx_bdr_done(ap, dev_ptr);
			start_new_job = TRUE;
			break;

#ifdef P8DEBUG_SCRIPT
		    case A_chkpoint:
		    
                        dma_stat = p8xx_read_reg(ap, DSP, DSP_SIZE);
		        TRACE_3("chkp", (int) dma_stat, 
			   p8xx_read_reg(ap, SCRATCHA, SCRATCHA_SIZE),
			   p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE))
		        TRACE_2("dsa dmode", 
			   p8xx_read_reg(ap, DSA, DSA_SIZE),
			   p8xx_read_reg(ap, DMODE, DMODE_SIZE))
                        P8if(0x00020000,(brkpoint()));
		        p8xx_write_reg(ap, DSP, DSP_SIZE, dma_stat);
		        break;
#endif
		    case A_neg_select_failed:
		    case A_cmd_select_atn_failed:

		  	if (aborting_cmds) {
			    /* there should be an abort queued for this
			     * lun anyway.  Just leave on the active
			     * queue and process the reselect.  The cmd
			     * will get removed from the active queue
			     * after the abort.
			     */
			    INT_ON(dev_ptr->sid_ptr->vscript_ptr, 
					A_check_next_io_Used);
			    p8xx_start_chip(ap, NULL, 
				ap->Scripts.bscript_ptr, Ent_iowait_entry_point,
				0, 0);
			} else {
			    /* Nothing should be reselecting us at this
			     * point.  Leave the cmd on the active queue,
			     * and reset the bus to force the bad target
			     * to get off.
			     */
			    p8xx_command_reset_scsi_bus(ap, 18);
			}
			break;
			
                    case A_abort_select_failed:
		    case A_bdr_select_failed:
		        p8xx_abdr_sel_fail(ap, dev_ptr, com_ptr, save_interrupt);
                        break;

		    case A_phase_error:
		    case A_err_not_ext_msg:
		    case A_unknown_msg:
			p8xx_sir_abort_error(ap, dev_ptr, com_ptr, 
					save_interrupt, save_isr);
			break;

		    case A_unknown_reselect_id:
			TRACE_1("du unk rsl", 0)
		    case A_uninitialized_reselect:
			TRACE_1("d unint rsl", 0)
			p8xx_command_reset_scsi_bus(ap, 18);
			break;

                    default:
                        TRACE_1 ("d_default2", save_interrupt)
                        start_new_job = TRUE;
                        break;
                    }   /* End switch */
                } /* End of if SIR */
                
                else if (save_dsr & DABRT) {
                    /* under the current design, this should not occur */
                    P8printf(P8_INFO_1,
                        ("p8xx_dump_intr: Abort caused interrupt\n"));

                    /* clear the abort from istat */
                    (void) p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, 0x00);
                    start_new_job = TRUE;

                } else if ((save_dsr & IID) || (save_dsr & BF) || 
			   (save_dsr & HPE)) {
		    /* should not occur, bus reset if it did */
		    TRACE_1("iid/bf/hpe", save_dsr)
		    p8xx_command_reset_scsi_bus(ap,0x88);

		} else {  /* WTD, SSI  - none should occur */
		    TRACE_1("wtd/ssi", save_dsr)
		    start_new_job = TRUE;
	        }	

            }       /* End DMA interrupt */

            if (start_new_job) {
		chip_started_flag = FALSE;
    		if ((dev_ptr = ap->ABORT_BDR_head) != NULL) {
		    p8xx_abdr(ap, dev_ptr);
		    chip_started_flag = TRUE;
		}

                if ((chip_started_flag == FALSE) &&
                    (ap->DEVICE_ACTIVE_head != NULL)) {
                    dev_ptr = ap->DEVICE_ACTIVE_head;
                    while (dev_ptr != NULL) {
			p8xx_restore_iowait_jump( (uint *) 
			    ap->Scripts.vscript_ptr, dev_ptr, 
			    dev_ptr->sid_ptr->bscript_ptr);
                        dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
                    }
                    p8xx_start_chip(ap, com_ptr, ap->Scripts.bscript_ptr,
                        Ent_iowait_entry_point, 0, 0);
                }
            }
	} /* interrupt */
    }   /* while */

    /*
     * If the command did not complete within the time specified in
     * the command, then attempt to reset the scsi bus.
     */
    if (i >= (time_out_value * 1000)) {
        ret_code = ETIMEDOUT;

        if ((rc_bp != NULL) && (!(rc_bp->bufstruct.b_flags & B_ERROR))) {
            rc_bp->status_validity = SC_ADAPTER_ERROR;
            rc_bp->general_card_status = SC_CMD_TIMEOUT;
            rc_bp->scsi_status = SC_GOOD_STATUS;
            rc_bp->bufstruct.b_flags |= B_ERROR;
            rc_bp->bufstruct.b_resid = rc_bp->bufstruct.b_bcount;
            rc_bp->bufstruct.b_error = EIO;
        }

        p8xx_command_reset_scsi_bus(ap,12);  /* reset SCSI bus */

	p8xx_poll_for_bus_reset(ap);

        if (ap->reset_pending) {
	    /* reset did not occur, chip is hung. 
	     * We make another attempt to free the chip, but return an
	     * error regardless of success.  Since from dump_start a
	     * time-out followed by a bus reset is not considered an
	     * error, adjust the ret_code from ETIMEDOUT to an internal
	     * indicator if we were called from dump_start.
	     */
	
            p8xx_chip_register_reset(ap);
            p8xx_command_reset_scsi_bus(ap,11);

	    p8xx_poll_for_bus_reset(ap);

	    if (aborting_cmds)
	        ret_code = PSC_DUMP_HANG;
	}
    } else {
	/* we didn't time-out. set ret_code based on rc_bp, or to zero */
        if (rc_bp)
	    ret_code = (int) rc_bp->bufstruct.b_error;
	else
	    ret_code = PSC_NO_ERR;
    }
 
    /* if ret_code is not zero, we have a fatal error and expect */
    /* not to be called again to process any more commands */
    P8printf(P8_EXIT,("Leaving p8xx_dumpintr\n"));
    TRACE_1 ("out dintr", ret_code)
    return (ret_code);

}  /* end p8xx_dump_intr */

/*************************************************************************/
/*									 */
/* NAME: p8xx_dumpwrt							 */
/*									 */
/* FUNCTION: Write to the dump device.					 */
/*									 */
/*      Executes the DUMPWRITE command, sending the command in the       */
/*	sc_buf to the target device.	 			         */
/*	Actual division of labor is that dumpwrt validates the state and */
/*	parameters, then calls p8xx_dump_dev to allocate resources and   */
/*	start the chip.  dumpwrt then calls p8xx_dump_intr to learn the  */
/*	success or failure of the command.			         */
/*									 */
/* EXECUTION ENVIRONMENT:						 */
/*									 */
/*	This routine can  be called in the interrupt environment.  It	 */
/*	can not page fault and is pinned.				 */
/*									 */
/* (DATA STRUCTURES:)							 */
/*		      adapter_def    - adapter info structure		 */
/*		      sc_buf	- controller info structure		 */
/*		      dev_info	  - device info structure		 */
/*									 */
/* INPUTS:								 */
/*	ads_t *ap	address of the adapter structure		 */
/*	bp		argument passed via devdump, pointing to the     */
/*			     sc_buf to be written              	         */
/*	ext		parameter passed via devdump                     */
/*									 */
/* CALLED BY:								 */
/*	p8xx_dump							 */
/*									 */
/* INTERNAL PROCEDURES CALLED:						 */
/*	p8xx_dump_dev	  p8xx_dump_intr				 */
/*									 */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is return- */
/*	ed and the caller is left responsible to recover from the error. */
/*									 */
/* RETURNS:								 */
/*	0    	  -  success            				 */
/*	EINVAL	  -  dump not started      				 */
/*	ENXIO	  -  bad sc_buf, dump device not opened			 */
/*	errno from p8xx_dump_intr                       		 */
/*									 */
/*************************************************************************/
int
p8xx_dumpwrt(
	ads_t		*ap,
	struct sc_buf	*bp,
	int		ext)
{
    dvi_t	*dev_ptr;
    int		dev_index,
    		ret_code = PSC_NO_ERR;

    P8printf(P8_ENTRY,("Entering p8xx_dumpwrt\n"));

    TRACE_2 ("DUMPWRITE", (int) bp, ext)
    /* determine the dev_ptr */
    dev_index = INDEX(bp->scsi_command.scsi_id,
	(bp->scsi_command.scsi_cmd.lun) >> 5);
    dev_ptr = ap->device_queue_hash[dev_index];

    if (dev_ptr == NULL)
	return (ENXIO);

    /* Reject request if dump is not started */
    if (((ext == PM_DUMP) && (!ap->dump_started)) ||
        ((ext != PM_DUMP) && !(ap->dump_started & P8XX_SYS_DMP)))
 	ret_code = EINVAL;

    if (ret_code == PSC_NO_ERR) {
	/* send a command to the target */
        ret_code = p8xx_dump_dev(ap, bp, dev_ptr); 

        if (ret_code == PSC_NO_ERR)
	    /* poll for interrupts until the command completes */
	    ret_code = p8xx_dump_intr(dev_ptr, FALSE);
    }

    P8printf(P8_EXIT,("Leaving p8xx_dumpwrt\n"));
    TRACE_1 ("out dumpwrt", ret_code)
    return(ret_code);

}  /* end p8xx_dumpwrt */

/************************************************************************/
/*									*/
/* NAME: p8xx_dump_dev							*/
/*									*/
/* FUNCTION: Send a SCSI commmand to the dump device		        */
/*									*/
/*	This routine controls the starting of the SCSI command.  The    */
/*	required resources are obtained, any needed script patching or  */
/*	set-up is done, and the SCRIPT is started at an appropriate     */
/*	point.      							*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*									*/
/*	This routine can be called in the interrupt environment.  It	*/
/*	can not page fault and is pinned.				*/
/*									*/
/* (DATA STRUCTURES:)							*/
/*		      adapter_def    - adapter info structure		*/
/*		      sc_buf	- controller info structure		*/
/*		      dev_info	  - device info structure		*/
/*									*/
/* INPUTS:								*/
/*									*/
/*	ads_t *ap	address of adapter structure			*/
/*	sc_buf *bp	address of sc_buf containing scsi cmd to issue  */
/*	dvi_t *dev_ptr	address of device information structure		*/
/*									*/
/* CALLED BY:								*/
/*	p8xx_dumpwrt							*/
/*									*/
/* INTERNAL PROCEDURES CALLED:						*/
/*									*/
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is	*/
/*	returned and the caller is left responsible to recover from the	*/
/*	error.								*/
/*									*/
/* RETURNS:								*/
/*	0	Success, request enqueued.				*/
/*	ENOMEM	Resource allocation failed				*/
/*	EIO	Adapter queue state indicates an error			*/
/*									*/
/************************************************************************/
int
p8xx_dump_dev(
	ads_t	*ap,
	struct sc_buf *bp,
	dvi_t	*dev_ptr)
{
    uchar chk_disconnect;
    int tag, ret_code;
    uint entry_point;
    cmd_t *com_ptr;

    TRACE_2 ("dump_dev", (int) bp, (int) dev_ptr)
    P8printf(P8_ENTRY,("Entering p8xx_dump_dev routine.\n"));

    bp->bufstruct.av_forw = NULL;   /* only accept one cmd */
    bp->bufstruct.b_work = 0;
    /* Initialize the following flag to the "no error" state. */
    bp->bufstruct.b_error = 0;
    bp->bufstruct.b_flags &= ~B_ERROR;
    bp->bufstruct.b_resid = 0;
    bp->status_validity = 0;
    bp->general_card_status = 0;
    bp->scsi_status = 0;
    bp->resvd7 = FALSE;         /* not expedited (shouldn't matter) */

    if ((tag = p8xx_alloc_resources(bp, dev_ptr)) <= PSC_ALLOC_FAILED)
        return (ENOMEM);

    com_ptr = &ap->command[tag];
    if (ap->DEVICE_ACTIVE_head == NULL) {
        /*
	 * Prepare the script, then start the chip
	 */
	chk_disconnect = bp->scsi_command.flags & SC_NODISC;
	P8if(P8_ABORT, { if (bp->scsi_command.flags & SC_NODISC)
			printf("start forces NODISC\n");
	});

	p8xx_set_disconnect(dev_ptr, chk_disconnect);

	entry_point = p8xx_negotiate(com_ptr, bp);
        p8xx_patch_tag_changes(com_ptr, bp->q_tag_msg);

	p8xx_prep_main_script(ap->Scripts.vscript_ptr,
		    dev_ptr->sid_ptr->vscript_ptr, com_ptr,
		    dev_ptr->sid_ptr->bscript_ptr);

	(void) p8xx_prepare(com_ptr);	/* build TIM */

	/*
	 * Start the chip. The call to p8xx_negotiate above gave us the 
         * proper offset based upon the need for synchronous negotiation.
	 */
	p8xx_start_chip(ap, com_ptr, dev_ptr->sid_ptr->bscript_ptr, 
		entry_point, com_ptr->tim_bus_addr, com_ptr->tim_tbl_used);

	/* Add to the DEVICE ACTIVE QUEUE */
	p8xx_enq_active(dev_ptr, com_ptr);
        TRACE_1("started cmd", (int) com_ptr)
        ret_code = PSC_NO_ERR;

    } else {
        TRACE_1 ("dmpdev error", (int) ap->DEVICE_ACTIVE_head)
        /* A severe error has occurred.  The dump cannot proceed */
	ret_code = EIO;
    }

    P8printf(P8_EXIT,("Leaving p8xx_dump_dev routine.\n"));
    TRACE_1 ("out dump_dev", ret_code);
    return (ret_code);
}

/************************************************************************
 *								
 * NAME:	p8xx_dumpend
 *						
 * FUNCTION:   This function is called by p8xx_dump to process the DUMPEND
 *	call at the end of a power management dump.  The routine puts various
 *	data structures and the script into a known state.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, in the dump context.
 *						
 * DATA STRUCTURES:			
 *	dev_info structure - target lun that received the dump
 *	sid_info structure - target id that received the dump
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_dumpend(ads_t *ap)
{
    int i;
    dvi_t *dev_ptr;
    sid_t *sid_ptr;

    TRACE_1("dumpend", (int) ap->dump_dev_ptr)

    /* all adapter-based queues should be clear */
    ap->DEVICE_ACTIVE_head = ap->DEVICE_ACTIVE_tail = NULL;
    ap->DEVICE_WAITING_head = ap->DEVICE_WAITING_tail = NULL;
    ap->ABORT_BDR_head = ap->ABORT_BDR_tail = NULL;

    dev_ptr = ap->dump_dev_ptr;
    ASSERT(dev_ptr);

    /* the device active, wait and abort queues should be clear */
    dev_ptr->ABORT_BDR_fwd = dev_ptr->ABORT_BDR_bkwd = NULL;
    dev_ptr->active_head = dev_ptr->active_tail = NULL;
    dev_ptr->wait_head = dev_ptr->wait_tail = NULL;

    dev_ptr->flags = RETRY_ERROR;
    dev_ptr->queue_state        = ACTIVE;
    dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;

    /* Ensure full patching of script for next command to this id */
    sid_ptr = dev_ptr->sid_ptr;
    sid_ptr->disconnect_flag = FALSE;
    sid_ptr->restart_in_prog = FALSE;
    sid_ptr->tag_flag = ap->ddi.addi.card_scsi_id << 3; /* init. to a */
                                                 /* value never used. */
    sid_ptr->tag_msg_flag = 0xFF;
    sid_ptr->lun_flag = 0xFF;
    sid_ptr->bdring_lun = NULL;

    p8xx_reset_iowait_jump(&ap->Scripts, dev_ptr, FALSE);

    ap->dump_dev_ptr = NULL;
}




/************************************************************************/
/*									*/
/* NAME:	p8xx_cdt_func						*/
/*									*/
/* FUNCTION:	Adapter Driver Component Dump Table Routine		*/
/*									*/
/*	This routine builds the driver dump table during a system dump. */
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine runs on the interrupt level, so it cannot malloc	*/
/*	or free memory.							*/
/*									*/
/* DATA STRUCTURES:							*/
/*	cdt	- the structure used in the master dump table to keep	*/
/*		  track of component dump entries.			*/
/*									*/
/* INPUTS:								*/
/*	int arg		1 dump dd is starting to get dump table		*/
/*			  entries.					*/
/*			2 dump dd is finished getting the dump table	*/
/*			  entries.					*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	Return code is a pointer to the struct cdt to be dumped for	*/
/*	this component.							*/
/*									*/
/* ERROR DESCRIPTION:  The following errno values may be returned:	*/
/*	none								*/
/*									*/
/************************************************************************/
struct cdt *
p8xx_cdt_func(
	int	arg)
{
    int	i, ent = 0;
    hash_t *hap;

    P8printf(P8_ENTRY,("Entering p8xx_cdt_func routine.\n"));

    if (arg == 1) {
	/* Only build the dump table on the initial dump call */

	/* init the table */
	if (p8xx_cdtp == NULL) {
	    p8xx_cdtp = (struct p8xx_cdt_table *) xmalloc(
		(uint) sizeof(struct p8xx_cdt_table), (uint) 2, pinned_heap);
	}
	bzero((char *) p8xx_cdtp, sizeof(struct p8xx_cdt_table));

	/* Init the head struct */
	p8xx_cdtp->p8xx_cdt_head._cdt_magic = DMP_MAGIC;
	strcpy(p8xx_cdtp->p8xx_cdt_head._cdt_name, "ncr8xx");
	/* _cdt_len is filled in below */

	/* Now begin filling in elements */
        for (i=0; i < (ADS_HASH_MASK+1); i++) {
 	    hap = (hash_t *) &hash_list[i];
	    while (hap->ap != NULL) {
		p8xx_cdtp->p8xx_entry[ent].d_segval = 0;
		strcpy(p8xx_cdtp->p8xx_entry[ent].d_name, "adp_str");
		p8xx_cdtp->p8xx_entry[ent].d_ptr = (char *) hap->ap;
		p8xx_cdtp->p8xx_entry[ent].d_len = (sizeof(ads_t));
		ent++;
	        hap = hap->next;
		if (hap == NULL)
		    break;
	    }
	}

	/* Fill in the actual table size */
	p8xx_cdtp->p8xx_cdt_head._cdt_len = sizeof(struct cdt_head) +
	    (sizeof(struct cdt_entry) * ent);
    }
    return ((struct cdt *) p8xx_cdtp);

}  /* end p8xx_cdt_func */

/************************************************************************
 *								
 * NAME:	p8xx_empty_cmd_save
 *						
 * FUNCTION:   This function is called to clear a cmd save queue by freeing
 *	the resources associated with the commands, and moving the sc_bufs
 *	back to the bp_save queue.  The WFR queue is assumed to not contain
 * 	any sc_bufs for the target scsi id/lun.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  
 *						
 * DATA STRUCTURES:			
 *	dev_info structure - anchors the particular cmd_save and bp_save queue
 *					
 * INPUTS:			
 *      dev_ptr	   - pointer to target structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void 
p8xx_empty_cmd_save(dvi_t *dev_ptr) 
{

    cmd_t * com_ptr;
    struct sc_buf * bp;

    /* process each element in the cmd save queue */
    while ((com_ptr = dev_ptr->cmd_save_head) != NULL) {
        p8xx_free_resources(com_ptr);
        p8xx_DEQ_CMD_SAVE(dev_ptr)

        /* move the sc_buf to the bp_save queue */
	bp = com_ptr->bp;
	if (bp != NULL) {
	    bp->bufstruct.av_forw = (struct buf *) dev_ptr->bp_save_head;
	    dev_ptr->bp_save_head = bp;
	    if (dev_ptr->bp_save_tail == NULL)
		dev_ptr->bp_save_tail = bp;
	}
    }
    dev_ptr->cmd_save_tail = NULL;
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_fail_cmd						*/
/*									*/
/* FUNCTION:	Fail Active and Pending Commands Routine		*/
/*									*/
/*	This internal routine is called by the interrupt handler	*/
/*	to clear out pending and active commands for a particular	*/
/*	device that has experienced some sort of failure that is	*/
/*	not recoverable by the adapter driver.				*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can only be called on priority levels greater	*/
/*	than, or equal to that of the interrupt handler.		*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*	sc_buf	- input/output request struct used between the adapter	*/
/*		  driver and the calling SCSI device driver		*/
/*									*/
/* INPUTS:								*/
/*	dev_info structure - pointer to device information structure	*/
/*									*/
/* RETURN VALUE DESCRIPTION:  The following are the return values:	*/
/*	none								*/
/*									*/
/************************************************************************/
void
p8xx_fail_cmd(
	dvi_t	*dev_ptr)
{
    ads_t	*ap;
    struct sc_buf *bp, *prev_bp;
    int     old_pri;
    int     dev_index;
    struct dev_info * cur_dev_ptr;
    struct cmd_info * com_ptr;

    P8printf(P8_ENTRY,("Entering p8xx_fail_cmd routine.\n"));
    ap = dev_ptr->ap;

    /* 
     * This assumes that for the failed command(s), the following are set
     * previously:  sc_buf status, sc_buf resid value, and sc_buf b_error
     * field.
     */
    old_pri = i_disable(ap->ddi.addi.int_prior);
    TRACE_1 ("in failC", (int) dev_ptr)

    /* return all commands on the active queue */
    while (dev_ptr->active_head != NULL)
    {
        bp = dev_ptr->active_head->bp;
        com_ptr = dev_ptr->active_head;

        /* check if this buf has been marked with an error */
        if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR)))
        { /* set b_flags B_ERROR flag */
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = ENXIO;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            bp->status_validity = 0;
        }

        p8xx_free_resources(com_ptr);

        if ((bp != NULL) && (!ap->dump_started)) {
	    TRACE_1("iodone bp", (int) bp)
            iodone((struct buf *) bp);
	}

        p8xx_deq_active(dev_ptr, com_ptr);
    }   /* while */

    /* return all commands on the wait queue for this device */
    while (dev_ptr->wait_head != NULL)
    {
        bp = dev_ptr->wait_head->bp;
        com_ptr = dev_ptr->wait_head;

        /* Set b_flags B_ERROR flag if not already set.      */
        /* (on certain error flows, we might mark a bp,      */
        /* put it on the wait_q, and initiate an action      */
        /* that eventually results in fail_cmd being called) */
        if (!(bp->bufstruct.b_flags & B_ERROR)) {
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = ENXIO;
            bp->status_validity = 0;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
        }

        p8xx_free_resources(com_ptr);
        p8xx_deq_wait (com_ptr);
        if (!ap->dump_started) { 
	    TRACE_1("iodone bp", (int) bp)
	    iodone((struct buf *) bp);
	}
    }   /* while */

    if (!ap->dump_started) {
        /* in the dump context, leave the other pending/frozen queues
	 * as they are, to allow for the possibility that the cmds
	 * are intentionally frozen.  But in the normal context,
         * flush these queues too.
	 */

        /* return all commands on the cmd_save queue for this device */
        while (dev_ptr->cmd_save_head != NULL) {
	
            bp = dev_ptr->cmd_save_head->bp;
            com_ptr = dev_ptr->cmd_save_head;
            /* set b_flags B_ERROR flag */
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = ENXIO;
            bp->status_validity = 0;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            p8xx_free_resources(com_ptr);
            p8xx_DEQ_CMD_SAVE(dev_ptr)
	    TRACE_1("iodone bp", (int) bp)
            iodone((struct buf *) bp);
	}
        dev_ptr->cmd_save_tail = NULL;

        /* return any sc_buf on the wait for resources queue for this device */
        if (dev_ptr->flags & BLOCKED) {
            /* there is a bp on the queue for this device, need to find it */
            prev_bp = NULL;
            bp = ap->REQUEST_WFR_head;
            while (bp != NULL) {
                dev_index = INDEX(bp->scsi_command.scsi_id,
                          (bp->scsi_command.scsi_cmd.lun) >> 5);
                cur_dev_ptr = ap->device_queue_hash[dev_index];
                if (cur_dev_ptr == dev_ptr) {
                    /* this request belongs to the device */

                    /* set b_flags B_ERROR flag */
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_error = ENXIO;
                    bp->status_validity = 0;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    p8xx_deq_wfr (bp, prev_bp, ap);
		    TRACE_1("iodone bp", (int) bp)
                    iodone((struct buf *) bp);
                    break;  /* only one command on WFR queue; we found */
                            /* it so break out the the while loop      */

                } else {
                    /* this request doesn't belong to the device - skip it */
                    prev_bp = bp;
                    bp = (struct sc_buf *) bp->bufstruct.av_forw;
                }
            }   /* while */
        }

        /* return all commands on the bp_save queue for this device */
        while (dev_ptr->bp_save_head != NULL) {
            TRACE_1 ("bp_save", (int) dev_ptr->bp_save_head)
            bp = dev_ptr->bp_save_head;
            /* set b_flags B_ERROR flag */
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = ENXIO;
            bp->status_validity = 0;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            p8xx_DEQ_BP_SAVE(dev_ptr)
	    TRACE_1("iodone bp", (int) bp)
            iodone((struct buf *) bp);
        }
        dev_ptr->bp_save_tail = NULL;
    } /* end cleanup of pending queues in normal context */

    /* turn off BLOCKED, STARVE, etc. */
    dev_ptr->flags = RETRY_ERROR;
    dev_ptr->dev_watchdog.save_time = 0;

    /* change queue state to HALTED */
    if (dev_ptr->queue_state & STOPPING) {
	if (!ap->dump_started)
            e_wakeup(&dev_ptr->stop_event);
        dev_ptr->queue_state = HALTED | STOPPING;

    } else {
        dev_ptr->queue_state = HALTED;
    }
    TRACE_1 ("out fail", (int) dev_ptr)
    i_enable(old_pri);

    P8printf(P8_EXIT,("Leaving p8xx_fail_cmd routine.\n"));

}  /* end p8xx_fail_cmd */

/************************************************************************/
/*									*/
/* NAME:	p8xx_intr						*/
/*									*/
/* FUNCTION:	Adapter Driver Interrupt Handler			*/
/*									*/
/*	This routine processes adapter interrupt conditions.		*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine runs on the interrupt level, therefore, it must	*/
/*	only perform operations on pinned data.				*/
/*									*/
/* NOTES:  This routine handles interrupts which are caused by scsi	*/
/*     scripts processing.  This entails processing the interrupts	*/
/*     handling error cleanup and logging, and issuing outstanding	*/
/*     commands for other devices					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	adapter_def - adapter unique data structure (one per adapter)	*/
/*	sc_buf	- input/output request struct used between the adapter	*/
/*		  driver and the calling SCSI device driver		*/
/*	dev_info - driver structure which holds information related to	*/
/*		   a particular device and hence a command		*/
/*	intr	- kernel interrupt information structure		*/
/*									*/
/* INPUTS:								*/
/*	handler - pointer to the intrpt handler structure. This		*/
/*		is assumed to be the first thing in the adapter struct	*/
/*		since this is how we obtain addressability to the	*/
/*		adapter structure.					*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	INTR_FAIL    interrupt was not processed			*/
/*	INTR_SUCC    interrupt was processed				*/
/*									*/
/* ERROR DESCRIPTION:  The following errno values may be returned:	*/
/*	none								*/
/*									*/
/************************************************************************/
int
p8xx_intr(
	struct pintr	*itsp)
{
    struct sc_buf *bp, *tbp;
    saved_t 	*move_info_ptr;
    dvi_t	*dev_ptr = NULL;
    dvi_t	*tmp_dev_ptr;
    ads_t	*ap;
    struct cmd_info *com_ptr;
    int     tag;

    int		save_isr, save_dsr;	/* save status registers */
    int		save_interrupt, base_interrupt;
    int		command_issued_flag = 0,
		check_for_disconnect;
    int		issue_abrt_bdr;		/* are we doing abort/bdr */
    int		restart_point;
    int		dev_index;
    int		start_new_job = FALSE;
    int		save_sist;
    uint	*target_script;		/* used to patch script */
    uchar	save_stat;
    int		rc;

    P8_SETDBG(P8_INTR,P8_STD_DBMK);
    P8printf(P8_ENTRY,("Entering p8xx_intr routine\n"));

    ap = itsp->ap;		/* get adapter structure ptr */

    /* don't check for shared interrupt if we are powered off */
    if ((ap->pmh.mode != PM_DEVICE_FULL_ON) &&
        (ap->pmh.mode != PM_DEVICE_ENABLE)) {
	TRACE_1("idle outintr", INTR_FAIL)
	return(INTR_FAIL);		/* Interrupt not processed */
    }
	
    /*
     * Read the ISTAT register so we'll know which interrupt got us
     * here (and if its our interrupt, PCI interrupts can be shared).
     */
    save_isr = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
    TRACE_1("in intr", (int) save_isr)
    P8printf(P8_INFO_2,("ISTAT is 0x%02X\n", save_isr));

    P8debug(ap->intr_cnt++);

    /*
     * If neither SIP nor DIP are set, this is not our interrupt
     */
    if (!(save_isr & ISTAT_INTR)) {
	TRACE_1("not ours, out intr", 0)
	return(INTR_FAIL);		/* Interrupt not processed */
    }
    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, ap->devno);

    ap->pmh.activity = PM_ACTIVITY_OCCURRED;

    P8debug(ap->intr_proc_cnt++);

    /* Check to see if a command is active */
    if (ap->DEVICE_ACTIVE_head != NULL) {

        tag = p8xx_read_reg(ap, SCRATCHA3, SCRATCHA3_SIZE);
        TRACE_1 ("tag is ", tag)
 
	/*
	 * DMA Interrupt w/o SCSI Interrupt?
	 */

	if ((save_isr & ISTAT_DIP) && (!(save_isr & ISTAT_SIP))) {

	    /* Verify the interrupt and check for other conditions */
	    save_dsr = p8xx_read_reg(ap, DSTAT, DSTAT_SIZE);
	    TRACE_1("save_dsr", (int) save_dsr)
	    P8printf(P8_INFO_2,("DSTAT is 0x%x\n", save_dsr));
	    P8printf(P8_INFO_3,("Interrupt "));
	    P8if(P8_INFO_3,(p8xx_rpt_status(ap, "At DMA interrupt",1)));

	    /*
	     * Planned SCRIPT Interrupt
	     */
	    if (save_dsr & SIR) {

                /* DSPS contains the value which identifies */
                /* the specific planned SCRIPTS interrupt.  */
                save_interrupt = p8xx_read_reg(ap, DSPS, DSPS_SIZE);

		P8printf(P8_INFO_2,("PLANNED INTR is 0x%x\n", save_interrupt));

                if (ap->command[tag].in_use == TRUE) {
                    com_ptr = &ap->command[tag];
		    dev_ptr = com_ptr->device;
                    bp = com_ptr->bp;
                    TRACE_3("dint", (int) tag, (int) save_interrupt, (int) bp)
                    TRACE_3("dint", (int) tag, (int) com_ptr, (int) dev_ptr)
                }      
                else if ((save_interrupt != A_suspended) &&
                          (save_interrupt != A_unknown_reselect_id) &&
                          (save_interrupt != A_uninitialized_reselect)) {
                          /* tag is not valid or needed for these */
                          /* interrupts that come from the iowait */
                          /* part of the script. */
                    TRACE_2 ("bad tag", (int) tag, (int) save_interrupt)
		    p8xx_logerr(ERRID_SCSI_ERR10, ADAPTER_INT_ERROR,
			    0x55, 0, ap, NULL, TRUE);
                    p8xx_command_reset_scsi_bus(ap, 85);
                    TRACE_1 ("out intr", 0)
                    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, ap->devno);
                    return (INTR_SUCC);
                }

		/*
		 * Switch on type of interrupt
		 */
		switch (save_interrupt) {	 /* completed command */

#ifdef P8DEBUG_SCRIPT
		case A_chkpoint:
                    save_interrupt = p8xx_read_reg(ap, DSP, DSP_SIZE);
		    TRACE_3("chkp", (int) save_interrupt, 
			   p8xx_read_reg(ap, SCRATCHA, SCRATCHA_SIZE),
			   p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE))
		    TRACE_2("dsa dmode", 
			   p8xx_read_reg(ap, DSA, DSA_SIZE),
			   p8xx_read_reg(ap, DMODE, DMODE_SIZE))
                    P8if(0x00020000,(brkpoint()));
		    p8xx_write_reg(ap, DSP, DSP_SIZE, save_interrupt);
		    break;
#endif
			
		case A_suspended:
                    TRACE_1 ("suspended", 0)

		    /*
		     * Chip was suspended out of the wait reselect inst
		     */

		    start_new_job = TRUE;
		    P8debug(ap->sigp_intr_cnt++);
		    break;

		case A_check_next_io:
                    TRACE_2 ("chk nxtd", (int) dev_ptr, (int) bp)

		    /*
		     * The target has disconnected from the SCSI bus.
		     * If there was an implied save pointers, it has
	             * already been handled by the script.  Prepare
		     * to start another command.
		     */

		    INT_OFF(dev_ptr->sid_ptr->vscript_ptr, A_check_next_io_Used);
		    start_new_job = TRUE;
		    break;

	        case A_io_done_wsr:
		    TRACE_1("io_done wsr", 0)
		    rc = p8xx_recover_byte(ap, com_ptr, bp);
		    if (rc == 0) {
			/* adjust the nexus, since recover byte modifies */
			/* only the current pointers */
		        move_info_ptr = (saved_t *) (
                        ((ulong) ap->IND_TABLE.system_ptr + (16 * tag)));

		        move_info_ptr->tim_cnt = LtoCBO(p8xx_read_reg(ap, SCRATCHB,
						 SCRATCHB_SIZE));
		    } else if (rc == PSC_COPY_ERROR) {
			/* got an error trying to move the byte to memory */
                        if (!(bp->bufstruct.b_flags & B_ERROR)) {
                            /* mark the bp if no previous error */
                            bp->status_validity = 0;
                            bp->bufstruct.b_error = EFAULT;
                            bp->bufstruct.b_flags |= B_ERROR;
                            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                        }
			/* if an abort/bdr isn't already pending,
			 * set up for either an abort or a call to fail_cmd
			 */
			if (!((dev_ptr->flags & SCSI_ABORT) ||
			      (dev_ptr->flags & SCSI_BDR))) {
                            dev_ptr->flags |= SCSI_ABORT;
 		            p8xx_enq_abort_bdr(dev_ptr);
			}
		    }
		    /*
		     * the io_done wsr (io_done while a byte from a wide	
		     * scsi receive has not been transferred across the pci
		     * bus) processing now falls into the normal io_done
		     * processing..
		     */

		case A_io_done_wss:
		    TRACE_1("io_done wss", 0)
		case A_io_done:
                    TRACE_2 ("A_iodone", (int) dev_ptr, (int) bp)
		    P8printf(P8_INFO_1,("p8xx_intr: Current I/O done\n"));

		    W_STOP(&dev_ptr->dev_watchdog.dog);

		    /* Adjust residual */
		    move_info_ptr = (saved_t *) (
                    ((ulong) ap->IND_TABLE.system_ptr + (16 * tag)));

		    TRACE_2("scr A, B", 
			   p8xx_read_reg(ap, SCRATCHA, SCRATCHA_SIZE),
			   p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE))
		    TRACE_2("dsa dmod", 
			   p8xx_read_reg(ap, DSA, DSA_SIZE),
			   p8xx_read_reg(ap, DMODE, DMODE_SIZE))

		    /* adjust the residual from the saved pointers */
		    if (move_info_ptr->tim_cnt == 0) {
			bp->bufstruct.b_resid = 0;
		    } else {
			bp->bufstruct.b_resid = p8xx_calc_resid(ap,
				LtoCBO(move_info_ptr->tim_cnt), com_ptr);
		    }

		    /* account for any byte on the chip but not sent */
		    if (save_interrupt == A_io_done_wss)
			bp->bufstruct.b_resid++;
			
                    TRACE_2("nex resd", (int) move_info_ptr, 
				(int) bp->bufstruct.b_resid)
                    P8if(0x00020000,(brkpoint()));

                    if ((ap->ABORT_BDR_head == dev_ptr) ||
                        (dev_ptr->ABORT_BDR_bkwd != NULL))
                        /* there is an abort or bdr to queued */
                        issue_abrt_bdr = TRUE;
                    else
                        /* there isn't an abort or bdr queued */
                        issue_abrt_bdr = FALSE;

		    if (issue_abrt_bdr) {
		        TRACE_1 ("A_iodBDR", (int) dev_ptr)
			start_new_job = p8xx_issue_abort_bdr(dev_ptr,
                                       bp, TRUE, save_isr & ISTAT_CONNECTED);
			break;
		    } 

		    /*
		     * Else issue a command and complete previous command
		     * get status byte because issue will destroy it
		     */
		    save_stat = GET_STATUS(dev_ptr->sid_ptr->vscript_ptr);
			P8printf(P8_INFO_2,("STATUS BUFFER = 0x%x\n",
			    save_stat));
                    TRACE_1("status is", (int) save_stat)

		    /* special trace hook to note that command is done */
		    DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0, ap->devno, bp);

		    if (save_stat == SC_GOOD_STATUS) {
		        /* SCSI status is okay */

		        command_issued_flag = p8xx_issue_cmd(ap);

			/* if the first command finished, */
			if (dev_ptr->active_head == com_ptr) {
			    /* no need for starvation */
			    if (dev_ptr->flags & STARVATION) {
				dev_ptr->flags &= ~STARVATION;
				/* release any held cmds */
				(void) p8xx_unfreeze_qs(dev_ptr, NULL, FALSE);
                                if (!command_issued_flag)
                                    command_issued_flag = p8xx_issue_cmd(ap);

			    }

			    dev_ptr->dev_watchdog.save_time = 0;
			    /* start the timer for any next 
			     * cmd (issue_cmd above will not 
			     * have started it since com_ptr 
			     * was still queued when issue_cmd 
			     * was called). 
			     */
			    if (com_ptr->active_fwd != NULL) {
			        tbp = com_ptr->active_fwd->bp;
	    			if (tbp->timeout_value == 0)
                		    dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
	   			else
                		    dev_ptr->dev_watchdog.dog.restart =
					  (ulong) tbp->timeout_value + 1;
            			w_start(&dev_ptr->dev_watchdog.dog);
			    }

			} else { /* other than first command */
			    dev_ptr->active_head->preempt_counter--;
			    if (dev_ptr->active_head->preempt_counter == 0) {
			        dev_ptr->flags |= STARVATION;
				p8xx_freeze_qs(dev_ptr);
				/* move pending cmds and bps */
			    }
            		    w_start(&dev_ptr->dev_watchdog.dog);
			}

			p8xx_deq_active(dev_ptr, com_ptr);

			/* bp should only be null for abort or
			 * bdr ioctls, in which case we should
			 * not get this interrupt.  Also, the
			 * original code included a check
			 * for B_ERROR being previously set.
			 * It appears if this was previously
			 * set, we want to issue an abort or
			 * bdr (in which case we would have
			 * done so above and not gotten here),
			 * or else have aborted the chip and
			 * reset the scsi bus, clearing any
			 * SIR interrupts first.
			 */

                        TRACE_1 ("A_iodgds", (int) dev_ptr)
                        /* set scsi status in sc_buf */
                        bp->bufstruct.b_error = 0;
                        bp->status_validity = 0;
                        bp->scsi_status = SC_GOOD_STATUS;
                        dev_ptr->flags |= RETRY_ERROR;
                        /* free resources */
                        p8xx_free_resources(com_ptr);
			TRACE_1("iodone bp", (int) bp)
                        iodone((struct buf *) bp);
                        if ((dev_ptr->queue_state & STOPPING) &&
                            (dev_ptr->active_head == NULL)) {
                            e_wakeup(&dev_ptr->stop_event);
                        }

		    } else { 
			/* Status is NOT okay */

                        TRACE_1 ("bad status", save_stat)
                        dev_ptr->flags |= RETRY_ERROR;

                        if ((dev_ptr->active_head != dev_ptr->active_tail) 
			    && ((save_stat == SC_CHECK_CONDITION)
                                  || (save_stat == SC_COMMAND_TERMINATED))) {
                            /* handle starvation and preempt */

                            /* if the first command finished */
                            if (dev_ptr->active_head == com_ptr) {
                                /* no need for starvation */
                                if (dev_ptr->flags & STARVATION) {
                                    dev_ptr->flags &= ~STARVATION;

                                    /* release any held cmds */
                                    (void) p8xx_unfreeze_qs(dev_ptr, NULL, FALSE);
                                }

                                dev_ptr->dev_watchdog.save_time = 0;
                                /* start the timer for any next command */
                                if (com_ptr->active_fwd != NULL) {
                                    tbp = com_ptr->active_fwd->bp;
                                    if (tbp->timeout_value == 0)
                                        dev_ptr->dev_watchdog.dog.restart 
					    = (ulong) 0;
                                    else
                                        dev_ptr->dev_watchdog.dog.restart 
					    = (ulong) tbp->timeout_value + 1;
                                    w_start(&dev_ptr->dev_watchdog.dog);
                                }
                            } else { /* other than first command */
                                dev_ptr->active_head->preempt_counter--;
                                if (dev_ptr->active_head->preempt_counter == 0) {
                                    dev_ptr->flags |= STARVATION;

                                    /* move pending cmds and bps */
                                    p8xx_freeze_qs(dev_ptr);
                                }
                                w_start(&dev_ptr->dev_watchdog.dog);
                            }

                            p8xx_deq_active(dev_ptr, com_ptr);

                            /* set bp */
                            bp->adap_q_status = SC_DID_NOT_CLEAR_Q;
                            bp->scsi_status = save_stat;
                            bp->status_validity = SC_SCSI_ERROR;
                            bp->bufstruct.b_flags |= B_ERROR;
                            bp->bufstruct.b_error = EIO;

                            /* set queue state */
                            if (dev_ptr->queue_state & STOPPING)
                                dev_ptr->queue_state = STOPPING_or_HALTED_CAC;
                            else
                                dev_ptr->queue_state = HALTED_CAC;

			    /* renegotiate after a check condition */
			    if (save_stat == SC_CHECK_CONDITION)
				dev_ptr->sid_ptr->negotiate_flag = TRUE;

                            if (!(dev_ptr->flags & STARVATION)) {
                                /* freeze queues if not already
                                 * frozen due to starvation.
                                 */

                                p8xx_freeze_qs(dev_ptr);
                            }

                            command_issued_flag = p8xx_issue_cmd(ap);

                            /* free resources and return the bp */
                            p8xx_free_resources(com_ptr);
			    TRACE_1("iodone bp", (int) bp)
                            /* we don't check stopping, we will resolve CAC 1st. */
                            iodone((struct buf *) bp);

                        } /* end if CAC */

                        else if (save_stat == SC_QUEUE_FULL) {
                            TRACE_1 ("iodon qfull", (int) com_ptr)
                            /* deq_active and enq_wait, trying to delay */
                            p8xx_deq_active(dev_ptr, com_ptr);
                            p8xx_q_full(dev_ptr, com_ptr);

                            command_issued_flag = p8xx_issue_cmd(ap);

                        } else {
                            /* not a CAC.  If queueing the
                             * target has probably flushed its
                             * queue, but we will issue an abort
                             * to be sure.  Otherwise we just
                             * call fail cmd to clean up.
                             */
                            TRACE_2 ("chk noq", save_stat, (int) dev_ptr)

                            /* set scsi stat in sc_buf */
                            bp->scsi_status = save_stat;
                            bp->status_validity = SC_SCSI_ERROR;
                            bp->bufstruct.b_flags |= B_ERROR;
                            bp->bufstruct.b_error = EIO;

			    /* if a check condition, need to renegotiate */
			    if (save_stat == SC_CHECK_CONDITION)
				dev_ptr->sid_ptr->negotiate_flag = TRUE;

                            p8xx_enq_abort_bdr(dev_ptr);
                            dev_ptr->flags |= SCSI_ABORT;
                            /* issue_abort_bdr returns whether
                             * we should start a new job.  It
                             * will determine if we actually
                             * need to send an abort, or
                             * whether a call to fail_cmd is sufficient.
                             */
                            command_issued_flag = !p8xx_issue_abort_bdr(dev_ptr,
                                       bp, FALSE, save_isr & ISTAT_CONNECTED);

                            /* if no abort or bdr was issued
                             * see if we can issue a regular command.
                             */
                            if (!command_issued_flag)
                                command_issued_flag = p8xx_issue_cmd(ap);

                        } /* end else not a CAC */
                    }   /* end else bad status */

			
                    if (ap->REQUEST_WFR_head != NULL)
                        p8xx_check_wfr_queue(command_issued_flag, ap);
                    else if (!command_issued_flag && (ap->DEVICE_ACTIVE_head 
								!= NULL))
                         p8xx_start_chip(ap, NULL, ap->Scripts.bscript_ptr,
                                Ent_iowait_entry_point, 0, 0);

                    break;

	        case A_save_ptrs_wsr:
	        case A_save_ptrs_wss:
		    p8xx_save_residue(ap, save_interrupt, com_ptr);
		    break;
			
		case A_wdtr_neg_done:
		    /* wdtr complete and successful */
		    p8xx_wdtr_answer(dev_ptr->sid_ptr->vscript_ptr, com_ptr);
		    break;

		case A_sdtr_neg_done:
		    p8xx_sdtr_neg_done(ap, dev_ptr, com_ptr);
		    break;

		case A_wdtr_msg_reject:
		    p8xx_wdtr_reject(ap, dev_ptr, com_ptr);
		    break;

	        case A_wdtr_msg_ignored:
		    /* the target read in our WDTR, but did not go to */
		    /* MSG-IN.  Treat this device as narrow/async.    */ 
		    TRACE_1("wdtr ignore", (int) dev_ptr->sid_ptr)
		    dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR;
	            dev_ptr->sid_ptr->io_table.scntl3  &= ~SCNTL3_EWS;

	        case A_sdtr_msg_ignored:
		    /* the target read in our SDTR, but did not go to */
		    /* MSG-IN.  Treat this device as async.           */
		    TRACE_1("sdtr ignore", (int) dev_ptr->sid_ptr)

		case A_sdtr_msg_reject:
		case A_sync_msg_reject:
		    p8xx_sdtr_reject(ap, dev_ptr, com_ptr);
		    break;

		case A_ignore_residue:
		    p8xx_ignore_residue(com_ptr);
		    break;

		case A_ext_msg:
		case A_unknown_ext_msg:
		case A_modify_data_ptr:
		case A_target_sdtr_sent:
		case A_target_wdtr_sent:
		    p8xx_handle_extended_messages(com_ptr, save_interrupt);
		    break;

		case A_abort_io_complete:
                    TRACE_1 ("ABORT done", (int) dev_ptr)
		    P8printf(P8_INFO_1,("p8xx_intr: ABORT_IO_COMPLETE \n"));

                    W_STOP(&dev_ptr->dev_watchdog.dog);
                    dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;
                    /* flush the aborted LUN */
                    p8xx_fail_cmd(dev_ptr);

                    if (dev_ptr->ioctl_wakeup == TRUE) {
                        dev_ptr->ioctl_errno = PSC_NO_ERR;
                        dev_ptr->ioctl_wakeup = FALSE;
                        e_wakeup(&dev_ptr->ioctl_event);
                    }
                    start_new_job = TRUE;
                    break;

		case A_bdr_io_complete:
		    p8xx_bdr_done(ap, dev_ptr);
                    start_new_job = TRUE;
                    break;

		case A_neg_select_failed:
		case A_cmd_select_atn_failed:
		    TRACE_2("Cneg bad", (int) tag, (int) dev_ptr)

		    /* 
		     * The chip was reselected before it could win the
		     * bus and select the target.  Return the command to
		     * the wait queue, and resume the chip at the
		     * IOWAIT script.
		     */

		    P8printf(P8_INFO_1,("p8xx_intr: select_atn_failed \n"));
		    W_STOP(&dev_ptr->dev_watchdog.dog);
                    p8xx_deq_active(dev_ptr, com_ptr);

                    /* place this dev on front of WAIT Q */
                    if (ap->DEVICE_WAITING_head == NULL) {
                        ap->DEVICE_WAITING_head = com_ptr;
                        ap->DEVICE_WAITING_tail = com_ptr;
                        com_ptr->active_fwd = NULL;
                        com_ptr->next_dev_cmd = NULL;
                        dev_ptr->wait_tail = com_ptr;
                    } else {      /* list not empty */
                        com_ptr->next_dev_cmd = dev_ptr->wait_head;
                        com_ptr->active_fwd = ap->DEVICE_WAITING_head;
                        ap->DEVICE_WAITING_head = com_ptr;
                        if (dev_ptr->wait_tail == NULL)
                            dev_ptr->wait_tail = com_ptr;
                    }
                    dev_ptr->wait_head = com_ptr;

		    /*
		     * If no commands are outstanding, the SCSI bus
		     * is hung. Attempt a SCSI bus reset to clear up
		     * this condition.
		     */
		    if (ap->DEVICE_ACTIVE_head == NULL) {
			p8xx_command_reset_scsi_bus(ap,1);
		    } else {
                        /* restart command timer */
                        if (dev_ptr->active_head != NULL)
                            w_start(&dev_ptr-> dev_watchdog.dog);

			/* Restart IOWAIT routine */
			p8xx_start_chip(ap, com_ptr, ap->Scripts.bscript_ptr,
			    Ent_iowait_entry_point, 0, 0);
		    }
		    break;

		case A_abort_select_failed:
		case A_bdr_select_failed:
		    p8xx_abdr_sel_fail(ap, dev_ptr, com_ptr, save_interrupt);
		    break;

		/* 
		 * For any phase or unknown message error do an abort
		 */
		case A_phase_error:
		case A_err_not_ext_msg:
		case A_unknown_msg:
		    p8xx_sir_abort_error(ap, dev_ptr, com_ptr,
					 save_interrupt, save_isr);
		    break;

		case A_unknown_reselect_id:
		    /*
		     * A device reselected us, but we can't figure out
		     * who it is, including the possibility that the id
		     * is the same as our own.
		     */
		    P8printf(P8_INFO_1,("p8xx_intr: unknwn_reselect.\n"));
                    TRACE_1 ("unkn resl id", 0)

		    P8if(P8_ABORT, {
			p8xx_rpt_status(ap, "Unknown Reselect ID", 2);
		    });
		    p8xx_logerr(ERRID_SCSI_ERR10, UNKNOWN_RESELECT_ID,
			0x67, 0, ap, NULL, TRUE);
		    p8xx_command_reset_scsi_bus(ap,4);
		    break;

		case A_uninitialized_reselect:
		    /* 
		     * An unconfigured device is trying to select us
		     * (spurious interrupt) or a device which was
		     * aborted or BDR'ed is trying to reselect us and
		     * is to trying finish off a cmd.
		     */
		    P8printf(P8_INFO_1,("p8xx_intr: uninit_reselect.\n"));
                    TRACE_1 ("bad resl", 0)
		    P8if(P8_ABORT, {
			p8xx_rpt_status(ap, "Uninitialized Reselect", 2);
		    });

		    p8xx_logerr(ERRID_SCSI_ERR10, UNINITIALIZED_RESELECT,
			0x69, 0, ap, NULL, TRUE);
		    p8xx_command_reset_scsi_bus(ap,5);
		    break;

		default:
		    /* 
		     * Unknown interrupt flag. This should not happen.
		     * If it does, we have a fatal error which we must
		     * back out of.
		     */
		    P8printf(P8_INFO_1,("p8xx_intr: SCRIPT err dflt \n"));
                    TRACE_1 ("default ", (int) dev_ptr)

		    p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
			0x6E, 0, ap, NULL, TRUE);
		    start_new_job = TRUE;
		    break;

		}	/* end script interrupt (SIR) switch */

	    } else if (save_dsr & DABRT) {

		/*
		 * ABORT CAUSED THIS INTERRUPT
		 */
		P8printf(P8_INFO_1,("p8xx_intr: Abort caused int\n"));

		/* Interrupt cleared up above */
                TRACE_1 ("abrt int", 0)
                /* With the use of SIGP, the interrupt 
                 * handler no longer expects to see an 
                 * ABRT interrupt.  Just assume the   
                 * chip has been stopped gracefully... 
		 */
		p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
		    0x73, 0, ap, NULL, TRUE);
                dev_ptr = NULL;
                start_new_job = TRUE;

	    } else if (save_dsr & IID) {

		/*
		 * ILLEGAL INSTRUCTION DETECTED
		 */
                TRACE_1("iid", p8xx_read_reg(ap, DSP, DSP_SIZE))
		P8printf(P8_INFO_1|P8_ABORT,
		    ("p8xx_intr: ILLEGAL Inst int, DSP %08x\n",
		    p8xx_read_reg(ap, DSP, DSP_SIZE)));

		P8if(P8_ABORT, {
		    static int ill_cnt = 0;
		    p8xx_rpt_status(ap, "At Illegal inst",2);
                    if (p8xx_verify_tag(ap, com_ptr)) 
			p8xx_dump_script(com_ptr->device->sid_ptr->vscript_ptr,
			    PAGESIZE/sizeof(ULONG));
		    printf("Illegal Instruction Count = %d\n", ++ill_cnt);
		});
		P8if(P8_ASSERT,{ASSERT(0)});
		dev_ptr = NULL;
		p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
		    0x78, 0, ap, NULL, TRUE);
		/* This is a should not occur, but if it somehow has, be sure
	         * the bus is cleared.
		 */
		p8xx_command_reset_scsi_bus(ap,0x78);

	    } else if ((save_dsr & BF) || (save_dsr & HPE)) {

		/* 
		 * HOST BUS FAULT or HOST PARITY ERROR
		 */

		P8if(P8_ABORT, {
		    p8xx_rpt_status(ap, "Bus Fault",1);
		});
                TRACE_1 ("host bus", (int) save_dsr)

		P8if(P8_ASSERT,{ASSERT(0)});
		if (save_dsr & BF)
		    p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
		        0x7D, 0, ap, NULL, TRUE);
		else
		    p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
		        0x7E, 0, ap, NULL, TRUE);

                P8if(0x00020000,(brkpoint()));
		p8xx_cleanup_reset(ap, HBUS_ERR);

	    } else if ((save_dsr & WTD) ||
		       (save_dsr & SSI)) {

		/*
		 * WATCH DOG TIMER INTERRUPT
		 * SINGLE STEP INTERRUPT (none should occur)
		 */
		if (save_dsr & WTD) {
		    P8printf(P8_INFO_1,("p8xx_intr: wtd\n"));
                    TRACE_1 ("watch tm", 0)
		    p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR, 0x7F, 0,
			ap, NULL, TRUE);
		} else {
		    P8printf(P8_INFO_1,("p8xx_intr: ss int\n"));
                    TRACE_1 ("sstep int", 0)
		    p8xx_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR, 0x80, 0,
			ap, NULL, TRUE);
		}

		start_new_job = TRUE;
	    }

	} else {
            /* else (save_isr & ISTAT_SIP) 
	     *
	     * A SCSI Interrupt -- Handles case of both DIP and SIP as well
	     */
	    start_new_job = p8xx_sip(ap, save_isr, tag);

	}   /* end else if end of SCSI interrupt (ISTAT_SIP) */

	if (start_new_job) {
            command_issued_flag = p8xx_issue_cmd(ap);
            if (ap->REQUEST_WFR_head != NULL)
                p8xx_check_wfr_queue(command_issued_flag, ap);
            else if (!command_issued_flag && (ap->DEVICE_ACTIVE_head != NULL))
                p8xx_start_chip(ap, NULL, ap->Scripts.bscript_ptr,
                                Ent_iowait_entry_point, 0, 0);
	}

    } else {		/* an interrupt was received with no command active */

	/*
	 * If an EPOW has occurred, a reset may have been sent by the
	 * EPOW handler.  If there are commands this will be handled by the
	 * SCSI reset cleanup.	If not, it will hit this code and have the
	 * watchdog reset.
	 */
        TRACE_1 ("no cmd", (int) save_isr)

	(void) p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, 0x00);
	if (save_isr & ISTAT_DIP) {
	    /* Clear pending DMA interrupt */
	    (void) p8xx_read_reg(ap, DSTAT, DSTAT_SIZE);
	}
	if (save_isr & ISTAT_SIP) {
	    save_sist = p8xx_read_reg(ap, SIST, SIST_SIZE);
            TRACE_1("sip", (int) save_sist)
	    if (save_sist & SCSI_RST) {
		W_STOP(&ap->reset_watchdog.dog);
		/* clean active queue */
		p8xx_logerr(ERRID_SCSI_ERR10, SCSI_BUS_RESET,
		    0xB1, 0, ap, NULL, TRUE);
		p8xx_scsi_reset_received(ap);
	    }
	}
    }

    P8printf(P8_EXIT,("Leaving p8xx_intr\n"));

    TRACE_1 ("out intr", 0);
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, ap->devno);
    P8_RSTDBG(P8_INTR);
    return (INTR_SUCC);

}  /* end of p8xx_intr	    */

/************************************************************************
 *								
 * NAME:	p8xx_sip
 *						
 * FUNCTION:   This function is called to process all SCSI interrupts.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *	save_isr   - holds contents that were read from ISTAT register
 *	tag	   - index identifying likely cmd associated with the interrupt
 *				
 * RETURN VALUE DESCRIPTION:
 *	start_new_job - boolean value indicating whether the caller can
 *	   	start the chip on new work (start_new_job == TRUE), or
 *		whether the chip is now busy (start_new_job == FALSE)
 *						
 * ERROR DESCRIPTION			
 *      errors will generally result in system error log entries
 *							
 ************************************************************************/
int
p8xx_sip(ads_t *ap, int save_isr, int tag)
{
    int base_interrupt, save_interrupt;
    int restart_point;
    int check_for_disconnect;
    int start_new_job = FALSE;
    dvi_t *dev_ptr;
    cmd_t *com_ptr;

    /* Get our SCSI interrupt registers */
    base_interrupt = p8xx_read_reg(ap, SIST, SIST_SIZE);

    /* Mask out the non-fatal interrupts */
    save_interrupt = base_interrupt & SIEN_MASK;

    dev_ptr = NULL;
    check_for_disconnect = TRUE;
    com_ptr = &ap->command[tag];
    TRACE_3("sist", (int) tag, (int) com_ptr, base_interrupt)

scsi_int:
    switch (save_interrupt) {

        case PHASE_MIS:		/* Phase mis-match */
	    P8printf(P8_INFO_1,("p8xx_intr: PHASE_MISMATCH (0x%04x)\n",
		    save_interrupt));
	    P8debug(ap->phase_mismatch_cnt++);

            if (p8xx_verify_tag(ap, com_ptr)) {
                dev_ptr = com_ptr->device;
                TRACE_1 ("phase ms", (int) dev_ptr)

		(void) p8xx_phase_mismatch(dev_ptr->sid_ptr->vscript_ptr,
			com_ptr, &restart_point);

		p8xx_start_chip(ap, com_ptr, dev_ptr->sid_ptr->bscript_ptr,
			restart_point, 0, 0);

	    } else {
		P8if(P8_ABORT, {
		    static int	bad_phase;
		    printf("*** [%d] Phase mismatch w/invalid DSP: %x ***\n",
			    ++bad_phase, p8xx_read_reg(ap, DSP, DSP_SIZE));
		});

                TRACE_1 ("phase ms", 0)
		p8xx_logerr(ERRID_SCSI_ERR10, PHASE_ERROR, 0x82, 0,
			ap, NULL, TRUE);
		p8xx_cleanup_reset(ap, PHASE_ERR);
	    }
	    break;

	case SCSI_SEL_TIMEOUT:
	    P8printf(P8_INFO_1,("p8xx_intr: SELECTION TIMEOUT (0x%04x)\n",
		    save_interrupt));

            if (p8xx_verify_tag(ap, com_ptr)) {

                dev_ptr = com_ptr->device;
                TRACE_1 ("sel time out", (int) dev_ptr)

		if (check_for_disconnect)
		    /*
		     * Issue an abort if there is more than 1 active cmd
		     */
		    start_new_job = p8xx_scsi_interrupt(com_ptr, dev_ptr,
                            save_interrupt, (dev_ptr->active_head != 
			    dev_ptr->active_tail) | DISC_PENDING);
		else
		    start_new_job = p8xx_scsi_interrupt(com_ptr, dev_ptr,
			    save_interrupt, 
			    (dev_ptr->active_head != dev_ptr->active_tail));

	    } else {
		/* clean active queue */
                TRACE_1 ("sel time out", 0)
		p8xx_logerr(ERRID_SCSI_ERR10, UNEXPECTED_SELECT_TIMEOUT,
			0x87, 0, ap, NULL, TRUE);
		p8xx_cleanup_reset(ap, DEVICE_ERR);
	    }
	    break;

	case SCSI_GROSS_ERR:
	    P8printf(P8_INFO_1,("p8xx_intr: SCSI GROSS ERROR (0x%04x)\n",
		    save_interrupt));
	    TRACE_1 ("gross error", 0)

	case SCSI_PARITY_ERR:
	    P8printf(P8_INFO_1,("p8xx_intr: SCSI PARITY ERROR (0x%04x)\n",
		    save_interrupt));
            TRACE_1 ("par err ", 0)

            if (p8xx_verify_tag(ap, com_ptr)) {
                dev_ptr = com_ptr->device;
                TRACE_1 ("par sg err ", (int) dev_ptr)

		start_new_job = p8xx_scsi_interrupt(com_ptr, dev_ptr,
			    save_interrupt, TRUE);
	    } else {

                TRACE_1 ("no com", 0)
		if (save_interrupt == SCSI_GROSS_ERR)
		    p8xx_logerr(ERRID_SCSI_ERR10, GROSS_ERROR,
			    0x8C, 0, ap, NULL, TRUE);
		else /* parity error */
		    p8xx_logerr(ERRID_SCSI_ERR10, BAD_PARITY_DETECTED,
			    0x8C, 0, ap, NULL, TRUE);

		/* clean active queue */
		p8xx_cleanup_reset(ap, SBUS_ERR);
	    }
	    break;

	case SCSI_UNEXP_DISC:
	    P8printf(P8_INFO_1,("p8xx_intr: UNEXP DISC (0x%04x)\n",
		    save_interrupt));

            if ((base_interrupt & SCSI_RESELECTED) &&
                (save_isr       & ISTAT_CONNECTED)) {
		p8xx_sel_glitch(ap, com_ptr, save_isr);
                start_new_job = FALSE;
	    } /* end if chip glitch */

            else if (p8xx_verify_tag(ap, com_ptr)) {
                dev_ptr = com_ptr->device;
                TRACE_1 ("uex dis ", (int) dev_ptr)

		P8if(P8_ABORT,(brkpoint()));
		/* Send an abort to make sure the target will not attempt
		 * to continue with the command.
		 */
		start_new_job = p8xx_scsi_interrupt(com_ptr, dev_ptr, 
			save_interrupt, TRUE);
	    } else {
                TRACE_1 ("uex dis ", 0)
		/* clean active queue */
		p8xx_logerr(ERRID_SCSI_ERR10, UNEXPECTED_BUS_FREE,
			0x8C, 0, ap, NULL, TRUE);
		p8xx_cleanup_reset(ap, DISC_ERR);
	    }
	    break;

	case SCSI_RST:
	    P8printf(P8_INFO_1,("p8xx_intr: SCSI RESET (0x%04x)\n",
		    save_interrupt));
            TRACE_1 ("RESET!!!", 0)

	    if (!ap->dump_started)
		W_STOP(&ap->reset_watchdog.dog);

	    /* clean active queue */
	    p8xx_logerr(ERRID_SCSI_ERR10, SCSI_BUS_RESET,
		    0x91, 0, ap, NULL, TRUE);
	    p8xx_scsi_reset_received(ap);
	    start_new_job = TRUE;
	    break;

	default:
	    P8printf(P8_INFO_1,("p8xx_intr: [default] (0x%04x)\n",
		    save_interrupt));
            TRACE_1 ("scsi dft", save_interrupt)

	    /* 
	     * If here, we probably have 2 interrupt bits set.
	     * Other case would be if at least 1 of the masked
	     * interrupt bits is set.   We process the most
	     * severe interrupt and continue processing.
	     */

	    if (save_interrupt & SCSI_RST) {
	        /* on reset throw away the others and execute */
	        save_interrupt = SCSI_RST;
		goto scsi_int;
	    }

	    /* chip glitch work-around */
            if ((save_interrupt & SCSI_UNEXP_DISC) &&
                (base_interrupt & SCSI_RESELECTED) &&
                (save_isr       & ISTAT_CONNECTED)) {

                TRACE_1("dft glitch", save_interrupt)

		/* not a glitch, since a 2nd fatal scsi interrupt
		 * was presented.  Process the unexpected disconnect.
		 */
		base_interrupt &= ~SCSI_RESELECTED;
                save_interrupt = SCSI_UNEXP_DISC;
                goto scsi_int;
            }

            if (save_interrupt & SCSI_UNEXP_DISC) {
                check_for_disconnect = FALSE;
	        save_interrupt = (save_interrupt & ~SCSI_UNEXP_DISC);
	        goto scsi_int;
	    }
	    if ((save_interrupt & SCSI_PARITY_ERR)
	        || (save_interrupt & SCSI_GROSS_ERR)) {   

	        /* 
		 * Insure that a SCSI_PARITY_ERR or SCSI_GROSS_ERR
		 * gets processed as SCSI_PARITY_ERR case.
		 */
		save_interrupt = SCSI_PARITY_ERR;
		goto scsi_int;
	    }
	    if (save_interrupt & PHASE_MIS) {
	        save_interrupt = PHASE_MIS;
	        goto scsi_int;
	    }
	    if (save_interrupt & SCSI_SEL_TIMEOUT) {
	        save_interrupt = SCSI_SEL_TIMEOUT;
	        goto scsi_int;
	    }

	    P8if(P8_INFO_2,(p8xx_rpt_status(ap,
	        "Unrecognized interrupt",1)));
	    ASSERT(save_interrupt != 0);

	    /* We should never get here.   */
	    p8xx_logerr(ERRID_SCSI_ERR2, UNKNOWN_ADAPTER_ERROR,
		    0xA0, 0, ap, NULL, TRUE);
	    start_new_job = TRUE;
	    break;

	}	    /* end of SCSI interrupt switch */

    return(start_new_job);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_check_wfr_queue                                    */
/*                                                                      */
/* FUNCTION:    Examine the waiting for resources queue                 */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled. Chip is assumed on.              */
/*                                                                      */
/* NOTES:                                                               */
/*  this is called to see if a cmd which is waiting for resources       */
/*  may be satisfied                                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      command_issued_flag - FALSE means no commands issued yet        */
/*      ap - associated adapter_def structure                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p8xx_check_wfr_queue(uchar command_issued_flag, ads_t *ap)
{
    struct sc_buf *bp, *prev_bp, *current_bp;
    struct sc_buf *failed_bps_head, *failed_bps_tail;
    struct dev_info *dev_ptr;
    int tag;

    TRACE_1 ("in chkwfrq", command_issued_flag)

    prev_bp = NULL;
    failed_bps_head = NULL;
    bp = ap->REQUEST_WFR_head;
    /* loop while Q not empty */
    while (bp != NULL)
    {
	dev_ptr = ap->device_queue_hash[
        INDEX(bp->scsi_command.scsi_id, (bp->scsi_command.scsi_cmd.lun) >> 5)];
 
	if (((bp == ap->blocked_bp) || 
	     (bp->bufstruct.b_work != LARGE_TCE_RESOURCES_USED))
	     || (ap->blocked_bp == NULL))
	/* if either the 1st bp needing large resources, or a bp not */
	/* needing large resources, or the 1st bp needing large resources */
	/* was already satisfied, then try to acquire resources    */
	{
	    tag = p8xx_alloc_resources(bp, dev_ptr);
            if (tag > PSC_ALLOC_FAILED) 
            {   /* allocation successful */

		/* save bp and advance in the list, before dequeueing */
	 	current_bp = bp;
	    	bp = (struct sc_buf *) bp->bufstruct.av_forw;
	
                p8xx_deq_wfr(current_bp, prev_bp, ap);
	        dev_ptr->flags &= ~BLOCKED;

		/* place the allocated cmd on the wait queue */
                p8xx_enq_wait(&ap->command[tag]);

                if (command_issued_flag == FALSE)
                    command_issued_flag = p8xx_issue_cmd(ap);

		/* process the bp_save queue */
		if ((current_bp = p8xx_process_bp_save(dev_ptr)) != NULL)
		{
		    /* one of the sc_bufs on the bp_save queue failed */
		    /* allocation.  We will append it to the wfr_q    */
		    /* after we complete the current walk thru the q. */
		    /* Here we build a list, using failed_bps_head    */
		    /* and failed_bps_tail. */
		    if (failed_bps_head == NULL)
		    {
			failed_bps_tail = current_bp;
		    }
		    current_bp->bufstruct.av_forw = (struct buf *) 
					failed_bps_head;
		    failed_bps_head = current_bp;
		}

		/* bp has already been adjusted, we don't adjust prev_bp */
		/* since we dequeued an element.  Continue at top of loop */
		continue;
            }
	}

	/* we either didn't try, or failed, allocation.  Move to the next */
	/* element in the wfr_q. */
	prev_bp = bp;
	bp = (struct sc_buf *) bp->bufstruct.av_forw;	
    }   /* end while */

    /* if any allocation attempts failed after processing bp_save queues, */
    /* append the newly blocked bps onto the wfr_q.   */
    if (failed_bps_head != NULL)
    {
	TRACE_1 ("append wfr", (int) failed_bps_head)
	if (ap->REQUEST_WFR_head == NULL) {
	    ap->REQUEST_WFR_head = failed_bps_head;
	} else {
	    ap->REQUEST_WFR_tail->bufstruct.av_forw = 
				(struct buf *) failed_bps_head;
	}
	ap->REQUEST_WFR_tail = failed_bps_tail;
    }

    /* if the chip has not been previously restarted, and we could */
    /* expect a legal reselection, restart the iowait script. */
    if ((command_issued_flag == FALSE) &&
        (ap->DEVICE_ACTIVE_head != NULL)) {
        p8xx_start_chip(ap, NULL, ap->Scripts.bscript_ptr,
                            Ent_iowait_entry_point, 0, 0);
    }
    TRACE_1 ("out chkwfrq", 0)
}  /* end of p8xx_check_wfr_queue */

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_unfreeze_qs                                        */
/*                                                                      */
/* FUNCTION:    Resumes processing of commands held on cmd_save or      */
/*      bp_save queue, sending a command to the chip if appropriate.    */
/*                                                                      */
/*      This function resumes the queues, handling the bp input		*/
/*      parameter if it is not null by issuing it or placing it in the  */
/*      correct queue.  						*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.  Chip may be on or off.                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure with frozen queues        */
/*      bp - pointer to any sc_buf with a command to issue after the    */
/*	     queues are unfrozen.					*/
/*      check_chip - flag indicating whether it is necessary to try to  */
/*	     start the chip on an unfrozen command or bp.		*/
/*      eaddr - effective address for pio, or NULL                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      indication whether chip should be checked again                 */
/*                                                                      */
/************************************************************************/
int
p8xx_unfreeze_qs(struct dev_info *dev_ptr, struct sc_buf *bp, 
		 uchar check_chip)
{
    struct cmd_info *com_ptr;
    struct sc_buf *failed_bp;
    int ret_code = TRUE;

    ads_t *ap = dev_ptr->ap;

    TRACE_2("unfreez", (int) dev_ptr, (int) bp)
    /* if cmd_save is not null, and
     *    power is normal, and
     *    either we aren't delaying after a restart or the first bp on
     *   	the cmd save queue doesn't need delaying (we're assuming
     *  	no other ones need delaying either), and
     *    the cmd is expedited if we are processing a CAC, and
     *    we aren't starving the device
     * then move the cmds to the waiting queue
     */
    if ((dev_ptr->cmd_save_head != NULL) &&

	 ((ap->power_state == P8XX_NORMAL_PWR) &&
         (!(dev_ptr->sid_ptr->restart_in_prog && 
	      (dev_ptr->cmd_save_head->bp->flags & SC_DELAY_CMD))) && 
	 (bp->resvd7 || (!(dev_ptr->queue_state & WAIT_INFO_or_HALTED_CAC))) && 
         (!(dev_ptr->flags & STARVATION)))) {

	/* place elements on the command save queue back on the wait queue */

	/* Update the wait list pointers */
	TRACE_2 ("cmd save", (int) dev_ptr->cmd_save_head, 
			     (int) dev_ptr->cmd_save_tail)
	com_ptr = dev_ptr->cmd_save_head;
	while (com_ptr != NULL) {
	    com_ptr->active_fwd = com_ptr->next_dev_cmd;
            com_ptr = com_ptr->next_dev_cmd;
	}

	/* mark the front of the sublist */
  	if (ap->DEVICE_WAITING_tail == NULL)
	    ap->DEVICE_WAITING_head = dev_ptr->cmd_save_head;
	else
	    ap->DEVICE_WAITING_tail->active_fwd = dev_ptr->cmd_save_head;

	/* mark the end of the list */
	ap->DEVICE_WAITING_tail = dev_ptr->cmd_save_tail;

	/* adjust the anchor pointers in the dev info structure */
	dev_ptr->wait_head = dev_ptr->cmd_save_head;
	dev_ptr->wait_tail = dev_ptr->cmd_save_tail;
	dev_ptr->cmd_save_head = NULL;
	dev_ptr->cmd_save_tail = NULL;
    }

    if (dev_ptr->bp_save_head != NULL) {
	/* process bp save will move the bps to the wait q if appropriate */
	failed_bp = p8xx_process_bp_save(dev_ptr); 
  	if (failed_bp != NULL)
	    p8xx_enq_wfr(failed_bp, ap);
    }
        
    /* Finally, handle any command passed in */
    if (bp != NULL) {
	/* before we call start, need to make sure that this command can't 
	 * be started on the chip ahead of other commands for the target.  
	 * This can happen if the active queue is empty. 
	 */

	/* should we append to bp_save ? */
	if ((dev_ptr->bp_save_head != NULL) || (dev_ptr->flags & BLOCKED)) {
	    dev_ptr->bp_save_tail->bufstruct.av_forw = (struct buf *) bp;
	    dev_ptr->bp_save_tail = bp;
	}
	else if (ap->DEVICE_ACTIVE_head != NULL) {
	    /* We know the chip is powered on.  Also, we can safely call
	     * p8xx_start, there is no chance of dispatching bp in front of
	     * another cmd for the same target.  p8xx_start will try to allocate
	     * resources for bp and append to the wait queue.  p8xx_start won't 
	     * signal the chip unless the cmd structure for bp is the first
	     * element on the wait queue.  So we follow the call to p8xx_start 
             * with a check to detect if a SUSPEND chip call is still needed.
	     */
	    p8xx_start(bp, dev_ptr);

	    /* If there is a waiting cmd, and it is not bp, then the chip
	     * needs to be signalled that there is new work to process.
	     */
	    if ((ap->DEVICE_WAITING_head != NULL) && 
		(ap->DEVICE_WAITING_head->bp != bp)) {
	        TRACE_1 ("sigp unfrz", (int) dev_ptr->wait_head)
		SUSPEND_CHIP(ap);
	    }

	} else { /* ACTIVE queue is NULL */  

	    /* Chip might be powered off, since there are no active cmds.
	     * Turn the chip on if necessary.  Then determine whether 
	     * p8xx_start should be called directly or not.
	     */ 
	    if (ap->pmh.mode == PM_DEVICE_IDLE)
		p8xx_turn_chip_on(ap, TRUE);

	    if (ap->DEVICE_WAITING_head != NULL) {
	        /* We'll start the chip on a command received prior to the
	         * one referenced by bp.  This is mandatory if dev_ptr->
	         * wait_head is not null (we would send cmds out of order
		 * to the same target).  But even if none of the waiting cmds
		 * are for dev_ptr, it is simpler to issue one of them first,
		 * since then we don't need to worry about the results of
		 * the call to p8xx_start.
	         * Since the active queue is empty, it is safe to call 
		 * p8xx_issue command. 
		 */

	        /* update the power mgmt. activity field */
                ap->pmh.activity = PM_ACTIVITY_OCCURRED;

	        (void) p8xx_issue_cmd(ap);
	        p8xx_start(bp, dev_ptr);
	    } else {
		/* okay to call p8xx_start, which if it can allocate resources,
		 * will start the chip directly on bp's cmd.  If allocation
		 * fails, we know the wait queue is empty and don't need to
		 * worry about starting the chip on any other pending work.
		 */ 
		p8xx_start(bp, dev_ptr);
	    }
	} /* end else active queue is null */
    } /* end if bp is not null */

    else if (check_chip) {
	/* routine was called from the top-half with a null bp.  We need to
	 * see whether we should try to issue a command directly or signal
	 * the chip that there is work.
	 */
	if (ap->DEVICE_WAITING_head != NULL) {
	    ret_code = FALSE;
	    if (ap->DEVICE_ACTIVE_head == NULL) {
	        /* update the power mgmt. activity field */
                ap->pmh.activity = PM_ACTIVITY_OCCURRED;

	        if (ap->pmh.mode == PM_DEVICE_IDLE)
		    p8xx_turn_chip_on(ap, TRUE);

	        (void) p8xx_issue_cmd(ap);
	    } else
		SUSPEND_CHIP(ap);
	}
    }
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_process_q_clr                                      */
/*                                                                      */
/* FUNCTION:    Handles the SC_Q_CLR flag by sending an abort.          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled. Chip may be on or off.                     */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure with contingent alleg.    */
/*      bp - pointer to sc_buf to iodone after the queues are flushed.  */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p8xx_process_q_clr(struct dev_info *dev_ptr, struct sc_buf *bp)
{
    ads_t *ap = dev_ptr->ap;

    TRACE_2 ("q_clear", (int) dev_ptr, (int) bp)
    dev_ptr->flags |= SCSI_ABORT;
    p8xx_enq_abort_bdr(dev_ptr);

    if (ap->DEVICE_ACTIVE_head == NULL) {
	ap->pmh.activity = PM_ACTIVITY_OCCURRED;
	/* Handle the power mgt mode:  turn chip on then
	 * issue the next cmd if idle, normal processing
	 * for full_on/enable.  Note there is a small 
	 * window in which we could be processing a CAC
	 * and go to suspend/hibernate, which is handled
	 * in the default clause (the bp is not started,
	 * it will get appended to bp_save).  This won't
	 * occur with correct pm operation.
	 */
	switch(ap->pmh.mode) {
	    case PM_DEVICE_IDLE:
		p8xx_turn_chip_on(ap, TRUE);
	    case PM_DEVICE_FULL_ON:
	    case PM_DEVICE_ENABLE:
                (void) p8xx_issue_cmd(ap);
		break;

	    default:
		break;
	}
    } else if (ap->DEVICE_WAITING_head == NULL)
	SUSPEND_CHIP(ap);

    /* put the bp on the bp_save queue, it will be iodoned at */
    /* the completion of the abort. */
    bp->bufstruct.av_forw = NULL;
    if (dev_ptr->bp_save_head == NULL)
        dev_ptr->bp_save_head = bp;
    else
	dev_ptr->bp_save_tail->bufstruct.av_forw = (struct buf *) bp;
    dev_ptr->bp_save_tail = bp;
    TRACE_1("out clr", (int) dev_ptr)
}

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_q_full                                             */
/*                                                                      */
/* FUNCTION:    Handles the SC_QUEUE_FULL SCSI status.           r      */
/*                                                                      */
/*      This function places the failed command back on the wait queue, */
/*      and restarts the command timer.                                 */
/* 									*/
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure associated w/ the cmd.    */
/*      com_ptr - pointer to the command which received the status      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p8xx_q_full(struct dev_info *dev_ptr, struct cmd_info *com_ptr)
{
    struct cmd_info *trail_ptr, *work_ptr;
    struct sc_buf * bp;
    ads_t *ap = dev_ptr->ap;

   /* this function handles a queue full status by placing the unsuccessful
    * command back on to the wait queue.  The command is placed in front of
    * any other commands on the wait queue for the same target lun.  The 
    * hope is that during the time we spend processing the iodone interrupt
    * where we discovered the queue full status, the target has had an
    * opportunity to arbitrate for the bus to begin a reselection.  Additional
    * processing that could be done here if this simple approach does not work
    * well during testing would be to move all commands for the target to the
    * end of the wait_q, or perhaps better (but more complicated), remove them
    * from the wait_q and delay resubmitting them.
    */
    TRACE_2 ("q_full", (int) dev_ptr, (int) com_ptr)
    if (dev_ptr->wait_head == NULL) {
	/* place at end of the wait queue */
     	p8xx_enq_wait(com_ptr);
    }
    else { /* there is already a command for this target on the wait q */
	trail_ptr = NULL;
 	work_ptr = ap->DEVICE_WAITING_head;
	while(work_ptr != dev_ptr->wait_head) {
	    trail_ptr = work_ptr;
	    work_ptr = work_ptr->active_fwd;
	}

    	if (trail_ptr == NULL)   /* command is first in list */
            ap->DEVICE_WAITING_head = com_ptr;
    	else 		  /* in middle or end of list */
            trail_ptr->active_fwd = com_ptr;

	dev_ptr->wait_head = com_ptr;
	com_ptr->next_dev_cmd = work_ptr;
	com_ptr->active_fwd = work_ptr;
    }

    /* now restart the timer for the active commands */
    if (dev_ptr->active_head != NULL) {
	bp = dev_ptr->active_head->bp;
	if (bp->timeout_value == 0)
            dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
	else
            dev_ptr->dev_watchdog.dog.restart = (ulong) bp->timeout_value + 1;
        W_START(&dev_ptr->dev_watchdog.dog);
    }
}

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_freeze_qs                                          */
/*                                                                      */
/* FUNCTION:    Moves commands to the cmd_save and bp_save queues.      */
/*                                                                      */
/*      This function processes the pending queues after an error       */
/*      occurs.                                                         */
/* 									*/
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure associated w/ the cmd.    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p8xx_freeze_qs(struct dev_info *dev_ptr)
{
    struct cmd_info *trail_ptr, *work_ptr, *target_ptr;
    struct sc_buf *prev_bp, *bp;
    struct dev_info *cur_dev_ptr;
    ads_t *ap = dev_ptr->ap;
    int dev_index;

    /* WAIT QUEUE */
    TRACE_1 ("freezeq", (int) dev_ptr)
    if (dev_ptr->wait_head != NULL) {
        /* move items from the wait_q to the cmd_save q */

        trail_ptr = NULL;
        work_ptr = ap->DEVICE_WAITING_head;
        target_ptr = dev_ptr->wait_head;

        /* loop until last element is found */
        while (target_ptr != NULL) {

    	    while(work_ptr != target_ptr) {
	        trail_ptr = work_ptr;
	        work_ptr = work_ptr->active_fwd;
	    }

	    /* work_ptr now equals the target pointer, which is at the head
	     * of a sublist to be extracted.  Now locate the end of the sublist,
	     * ie. the next element not for the same device (which could be 
	     * null)
	     */
            while ((work_ptr->next_dev_cmd != NULL) &&
	        (work_ptr->active_fwd == work_ptr->next_dev_cmd)) {
	        work_ptr = work_ptr->active_fwd;
	    }

            /* at this point trail_ptr is before the first element, and work_ptr
	     * is at the last command to be extracted in the current sublist 
	     */

            TRACE_2 ("sublist", (int) trail_ptr, (int) work_ptr)
	    /* identify the next target (might be null) */
	    target_ptr = work_ptr->next_dev_cmd;

	    if (target_ptr != NULL) {  
	        /* we are going to stay in the loop, mark the front of the */
	        /* intermediate sublist */

	        /* was the first element at the head of the list ? */
	        if (trail_ptr == NULL)
	            ap->DEVICE_WAITING_head = work_ptr->active_fwd;
	        else
	            trail_ptr->active_fwd = work_ptr->active_fwd;
	    }
        } /* end while target_ptr != NULL */

        TRACE_2 ("sublist", (int) trail_ptr, (int) work_ptr)

        /* on exit of the loop, trail_ptr preceeds the last sublist extracted */
        if (ap->DEVICE_WAITING_tail == dev_ptr->wait_tail)
	    ap->DEVICE_WAITING_tail = trail_ptr;

        /* mark the front of the final sublist */
        if (trail_ptr == NULL)
            ap->DEVICE_WAITING_head = work_ptr->active_fwd;
        else
            trail_ptr->active_fwd = work_ptr->active_fwd;

        /* extracted chain is already linked, just adjust the head pointers */
        /* note that active_fwd for the extracted list is not meaningful    */
        dev_ptr->cmd_save_head = dev_ptr->wait_head; 
        dev_ptr->cmd_save_tail = dev_ptr->wait_tail; 
        dev_ptr->wait_head = NULL;
        dev_ptr->wait_tail = NULL;
    }

    TRACE_2("ap wait", (int) ap->DEVICE_WAITING_head,
                       (int) ap->DEVICE_WAITING_tail)
    TRACE_2("cmd sv", (int) dev_ptr->cmd_save_head, 
		      (int) dev_ptr->cmd_save_tail)

    /* WFR QUEUE */
    if (dev_ptr->flags & BLOCKED) {
	/* find the blocked bp and place it at the front of the bp_save queue */
        prev_bp = NULL;
        bp = ap->REQUEST_WFR_head;
        while (bp != NULL) {
            dev_index = INDEX(bp->scsi_command.scsi_id,
                          (bp->scsi_command.scsi_cmd.lun) >> 5);

            cur_dev_ptr = ap->device_queue_hash[dev_index];
            if (cur_dev_ptr == dev_ptr) {   
		/* this request belongs to the device */
                p8xx_deq_wfr (bp, prev_bp, ap);
                break;  /* only one command on WFR queue; we found */
                        /* it so break out the the while loop      */
            } else {   
		/* this request doesn't belong to the device - skip it */
                prev_bp = bp;
                bp = (struct sc_buf *) bp->bufstruct.av_forw;
            }
        }  /* end while */

	/* insert the found bp */
	bp->bufstruct.av_forw = (struct buf *) dev_ptr->bp_save_head;
        dev_ptr->bp_save_head = bp;
	if (dev_ptr->bp_save_tail == NULL)
	    dev_ptr->bp_save_tail = bp;
	dev_ptr->flags &= ~BLOCKED;
    }

}
/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_process_bp_save                                    */
/*                                                                      */
/* FUNCTION:    Moves commands to the cmd_save and bp_save queues.      */
/*                                                                      */
/*      This function sees if an element on the bp_save queue has       */
/*      become unblocked, and attempts allocation if so.  If resource   */
/*      allocation fails, a pointer to the failing bp is returned (no   */
/*      additional elements on the bp_save q are processed).            */
/* 									*/
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure associated w/ the save_q  */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      sc_buf pointer or NULL.  If not NULL, this points to a sc_buf   */
/*      for which resource allocation failed.                           */
/*                                                                      */
/************************************************************************/
struct sc_buf *
p8xx_process_bp_save(struct dev_info *dev_ptr)
{
    ads_t *ap;
    struct sc_buf *ret_bp, *bp;
    sid_t *sid_ptr;
    int tag;
	
    ret_bp = NULL;

    ap = dev_ptr->ap;
    TRACE_1 ("proc bpsave", (int) dev_ptr)
    /* If normal power conditions are not present, we don't need to be 
     * processing this.  
     */
    if (ap->power_state == P8XX_NORMAL_PWR) {
	sid_ptr = dev_ptr->sid_ptr;
        while ((bp = dev_ptr->bp_save_head) != NULL) {
            TRACE_1 ("bpsave", (int) dev_ptr->bp_save_head)
	    /* if we don't need to delay due to a restart AND
	     *   the bp is expedited if we're blocking, starving, or flushing
	     *   the queue, AND
	     *   the bp is expedited if we are processing a CAC
	     */
	    if ( (!(sid_ptr->restart_in_prog && 
		   (bp->flags & SC_DELAY_CMD)))       &&
	         ((!(dev_ptr->flags & FLUSH_or_STARVE_or_BLOCKED)) || 
		    bp->resvd7)        && 
		 ((!(dev_ptr->queue_state & WAIT_INFO_or_HALTED_CAC)) || 
		    bp->resvd7))
	    {
		p8xx_DEQ_BP_SAVE(dev_ptr)
		bp->bufstruct.av_forw = NULL;
                if (dev_ptr->bp_save_head == NULL)
                    dev_ptr->bp_save_tail = NULL;
		TRACE_1("deq", (int) dev_ptr)
        	tag = p8xx_alloc_resources(bp, dev_ptr);
		if (tag > PSC_ALLOC_FAILED)
        	{  /* Resources were allocated */
		    dev_ptr->flags &= ~BLOCKED;
                    p8xx_enq_wait(&ap->command[tag]);
		} else {
		    /* resource allocation failed, so marked the */
		    /* dev_ptr as blocked, return the bp so that */
		    /* the caller can place it on the wfr queue, */
		    /* and stop processing the bp_save queue.    */
		    dev_ptr->flags |= BLOCKED;
		    ret_bp = bp;	
		    break;
		}
	    } else 
		/* can't process queue, so exit loop */
		break;
	} /* end while */
    }  /* end if */
    
    return (ret_bp);
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_issue_cmd						*/
/*									*/
/* FUNCTION:	issues a waiting command, or ABORT/BDR command		*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	Called by the interrupt handler.  Chip must be on.		*/
/*									*/
/* DATA STRUCTURES:							*/
/*									*/
/* INPUTS:								*/
/*	ap	pointer to the adapter structure			*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	int return value						*/
/*	TRUE - command issued all is well				*/
/*	FALSE - command not issued all is not well			*/
/*									*/
/* ERROR DESCRIPTION:  The following errno values may be returned:	*/
/*									*/
/************************************************************************/

int
p8xx_issue_cmd(
	ads_t	*ap)
{
    dvi_t	*dev_ptr;
    struct sc_buf *bp;
    struct cmd_info *com_ptr;
    uint	*target_script;	/* used in patching SCRIPT */
    uint 	entry_point; 
    uchar	chk_disconnect;

    P8printf(P8_ENTRY,("Entering p8xx_issue_cmd routine\n"));
    TRACE_1 ("in issue", 0)

    /* 
     * If abort/bdr queue is not empty we deal with it first
     */
    if (((dev_ptr = ap->ABORT_BDR_head) != NULL) && 
	       (ap->power_state != EPOW_PENDING)) {
	p8xx_abdr(ap, dev_ptr);
	return(TRUE);
    }

    /* Is there something on the waiting queue */
    com_ptr = ap->DEVICE_WAITING_head;

    if ((com_ptr != NULL) && (ap->power_state != EPOW_PENDING)) {
        bp = com_ptr->bp;
        dev_ptr = com_ptr->device;

	/* Trace hook to indicate new command for adapter */
	DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, ap->devno,
	    bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
	    bp->bufstruct.b_bcount);

	/*
	 * Prepare script
	 */
	chk_disconnect = bp->scsi_command.flags & SC_NODISC;
	P8if(P8_ABORT, {
	    if (bp->scsi_command.flags & SC_NODISC)
		printf("issue_cmd forces NODISC\n");
	});

	p8xx_set_disconnect(dev_ptr, chk_disconnect);
	entry_point = p8xx_negotiate(com_ptr, bp);
        p8xx_patch_tag_changes(com_ptr, bp->q_tag_msg);

	p8xx_prep_main_script(ap->Scripts.vscript_ptr,
	    dev_ptr->sid_ptr->vscript_ptr, com_ptr,
	    dev_ptr->sid_ptr->bscript_ptr);

	(void)p8xx_prepare(com_ptr);	/* build TIM */

	/* Move this request from the waiting to the active queue */
	p8xx_deq_wait(com_ptr);

	/*
	 * Start the chip. p8xx_negotiate gives us the proper offset
	 * based upon the need for synchronous negotiation.
	 */
	p8xx_start_chip(ap, com_ptr, dev_ptr->sid_ptr->bscript_ptr, entry_point,
		     com_ptr->tim_bus_addr, com_ptr->tim_tbl_used);

        TRACE_1 ("issue cd", (int) dev_ptr)
        /* if this will be the only active command for the lun, start the */
        /* command timer. */
        if (dev_ptr->active_head == NULL) {
            if (bp->timeout_value == 0)
                dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
            else
                dev_ptr->dev_watchdog.dog.restart =
                                (ulong) bp->timeout_value + 1;
            W_START(&dev_ptr->dev_watchdog.dog);
        }
        p8xx_enq_active(dev_ptr, com_ptr);

	P8printf(P8_EXIT,("Leaving p8xx_issue_cmd with TRUE\n"));
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, ap->devno);
        TRACE_1 ("out issu", (int) dev_ptr)
	return (TRUE);	/* command issued */

    }	/* if */

    /*
     * No command to issue.  Return FALSE.
     */
     
    P8printf(P8_EXIT,("Leaving p8xx_issue_cmd with FALSE\n"));
    return (FALSE);	/* command not issued */

}  /* p8xx_issue_cmd */


/************************************************************************
 *								
 * NAME:	p8xx_abdr
 *						
 * FUNCTION:    This function is called when an abort or bdr is going
 *	to be issued to the chip because of an element on the AbortBdr queue.
 *      If a BDR is queued for the same device, it receives priority.		
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * DATA STRUCTURES:			
 *	cmd_info structure - a cmd structure is used for the abort/bdr
 *					
 * INPUTS:			
 *      ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target scsi id/lun structure (should be on 
 *		     Abort/Bdr queue)
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_abdr(ads_t *ap, dvi_t *dev_ptr)
{
    cmd_t *com_ptr;

    com_ptr = &ap->command[ap->ddi.addi.card_scsi_id << 3];

    /* Note the assumption that an item on the abort/bdr queue has no
     * associated command structure
     */
    com_ptr->bp = NULL;
    com_ptr->resource_state = NO_RESOURCES_USED;
    com_ptr->in_use = TRUE;

    if (dev_ptr->flags & SCSI_BDR) {

        /* 
	 * Its a BDR.  This dev_ptr could be on the queue for an abort
	 * to a lun.  Access the dev_ptr associated with the bdr.
	 */
        dev_ptr = dev_ptr->sid_ptr->bdring_lun;
        com_ptr->device = dev_ptr;

        dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;

	p8xx_issue_bdr_script(dev_ptr, com_ptr, FALSE);

	P8printf(P8_ABORT,("%s at %s:%d for ID %d\n",
	    "BDR set",__FILE__,__LINE__,dev_ptr->scsi_id));

    } else /* an abort */ {

	dev_ptr->cmd_activity_state = ABORT_IN_PROGRESS;

        com_ptr->device = dev_ptr;

	p8xx_issue_abort_script(com_ptr, FALSE);

	P8printf(P8_ABORT,("%s at %s:%d for ID %d\n",
	        "ABORT set",__FILE__,__LINE__,dev_ptr->scsi_id));
    }

    p8xx_deq_abort_bdr(dev_ptr);
    p8xx_enq_active(dev_ptr, com_ptr);
    p8xx_start_abort_timer(dev_ptr, FALSE);

    TRACE_1 ("out abdr", (int) dev_ptr)
    P8printf(P8_EXIT,("p8xx_abdr: leaving TRUE\n"));

}

/************************************************************************/
/*									*/
/* NAME:	p8xx_cleanup_reset					*/
/*									*/
/* FUNCTION:	Cleans up the active queues due to a register error or	*/
/*		a dma error.  Chip is assumed on.			*/
/*									*/
/* NOTES:								*/
/*  This is called when an error occurrs that requires a bus reset to	*/
/*  be issued.	All commands will be cleaned up upon completion of the	*/
/*  reset by p8xx_scsi_reset_received.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*									*/
/* INPUTS:								*/
/*	ap	pointer to the adapter structure			*/
/*	int	err_type						*/
/*		1=dma 2=register 3=phase 4=device			*/
/*		5=disc 6=host 7=host bus 8=scsi bus			*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	none								*/
/*									*/
/* ERROR DESCRIPTION:  The following errno values may be returned:	*/
/*	none								*/
/*									*/
/************************************************************************/
void
p8xx_cleanup_reset(
	ads_t	*ap,
	int	err_type)
{
    dvi_t *dev_ptr;	/* device structure */
    struct cmd_info *com_ptr;
    struct sc_buf *bp;

    TRACE_1 ("in clres", err_type)
    P8printf(P8_ENTRY|P8_ABORT,
	("Cleaning up pending commands for SCSI bus reset, error=0x%x\n",
	err_type));

    /* Loop while devices are on Active queue */
    for (dev_ptr = ap->DEVICE_ACTIVE_head;
	 dev_ptr != NULL;
	 dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd) {

	/* 
	 * This error may have been caused by a timeout or epow or a glitch
	 * on the reset line
	 */
        com_ptr = dev_ptr->active_head;
        while (com_ptr != NULL) {
            bp = com_ptr->bp;

            /* if active cmd status not already set */
            if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR))) {

		/* Set SCSI status in sc_buf to say adapter error */
		bp->status_validity = SC_ADAPTER_ERROR;
		switch (err_type) {
		    case PHASE_ERR:
		    case DISC_ERR:
		    case SBUS_ERR:
			bp->general_card_status = SC_SCSI_BUS_FAULT;
			break;
		    case DEVICE_ERR:
			bp->general_card_status = SC_NO_DEVICE_RESPONSE;
			break;
		    case REG_ERR:
		    case DMA_ERR:
		    case HBUS_ERR:
		    default:
			bp->general_card_status = SC_HOST_IO_BUS_ERR;
			break;
		}
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
	    }
            com_ptr = com_ptr->active_fwd;
        } /* end while */


	/* If this device is on the abort/BDR queue, remove it */
	if ((ap->ABORT_BDR_head == dev_ptr) ||
	    (dev_ptr->ABORT_BDR_bkwd != NULL))
	    p8xx_deq_abort_bdr(dev_ptr);

	if ((dev_ptr->ioctl_wakeup == TRUE) && (!ap->dump_started)) {
	    dev_ptr->ioctl_errno = EIO;
	    dev_ptr->ioctl_wakeup = FALSE;
	    e_wakeup(&dev_ptr->ioctl_event);
	}
    } /* end for loop walking the active queue */

    if ((err_type == REG_ERR) || (err_type == HBUS_ERR)) {
	p8xx_chip_register_reset(ap);
	p8xx_command_reset_scsi_bus(ap,100+err_type);
    } else {
	p8xx_command_reset_scsi_bus(ap,100+err_type);
    }

    P8printf(P8_EXIT,("leaving p8xx_cleanup_reset\n"));
    TRACE_1 ("out clre", err_type)
}  /* end p8xx_cleanup_reset */

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_scsi_reset_received                                */
/*                                                                      */
/* FUNCTION:    Cleans up the active queues after a scsi bus reset has  */
/*              occurred.                                               */
/*                                                                      */
/* EXECUTION ENVIRONMENT:						*/
/*  Called from interrupt handler.  Chip is assumed on.			*/
/*                                                                      */
/* NOTES:                                                               */
/*  After a scsi bus reset occurs, all active commands are cleaned up.  */
/*  In addition, all resources are freed and all queues are searched to */
/*  ensure command restart occurs without error.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ads_t - pointer to adapter def structure                        */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p8xx_scsi_reset_received(ads_t *ap)
{
    dvi_t *dev_ptr;   /* device structure */
    dvi_t *tmp_dev_ptr;   /* device structure used for bdrs */
    cmd_t *com_ptr;   /* command structure */
    struct sc_buf *bp, *tmp_bp;
    int     i, dev_index;
    uchar not_dump_context;

    P8printf(P8_ENTRY|P8_ABORT,
	("Cleaning up pending commands following a SCSI bus reset\n"));
    TRACE_1 ("in reset rcv", 0)

    ap->reset_pending = FALSE;
    not_dump_context = !ap->dump_started;

    /* turn off activity light here, since the code that follows doesn't make
     * the usual call to deq_active where the light management would normally
     * be done.
     */
    if (ap->DEVICE_ACTIVE_head)
        LIGHT_OFF(ap);

    /* loop while devices are on Active queue */
    for (dev_ptr = ap->DEVICE_ACTIVE_head;
         dev_ptr != NULL;
         dev_ptr = ap->DEVICE_ACTIVE_head) {
        /*
         * This error may have been caused by a timeout or EPOW or a
         * glitch on the reset line
         */
	if (not_dump_context)
            W_STOP(&dev_ptr->dev_watchdog.dog);

        dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;

	/* process all active commands for the device */
        while ((com_ptr = dev_ptr->active_head) != NULL) {
            /* free resources for this command sta's, tce's */
            p8xx_free_resources(com_ptr);
            if ((bp = com_ptr->bp) != NULL) {

                /* if active cmd status not set */
                if (!(bp->bufstruct.b_flags & B_ERROR)) {
                    bp->status_validity = SC_ADAPTER_ERROR;
                    bp->general_card_status = SC_SCSI_BUS_RESET;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_error = EIO;
                }
		if (not_dump_context) {
		    TRACE_1("iodone bp", (int) bp)
                    iodone((struct buf *) bp);
	 	}
            }

            dev_ptr->active_head = com_ptr->active_fwd;
	    com_ptr->active_fwd = NULL;
	    com_ptr->active_bkwd = NULL;
        } /* end loop processing each command for the device */
	dev_ptr->active_tail = NULL;

        if ((ap->ABORT_BDR_head == dev_ptr) ||
            (dev_ptr->ABORT_BDR_bkwd != NULL))
            p8xx_deq_abort_bdr(dev_ptr);

        if ((dev_ptr->ioctl_wakeup == TRUE) && not_dump_context) {
            if (dev_ptr->flags & CAUSED_TIMEOUT)
                dev_ptr->ioctl_errno = ETIMEDOUT;
            else
                dev_ptr->ioctl_errno = EIO;

            dev_ptr->ioctl_wakeup = FALSE;
            e_wakeup(&dev_ptr->ioctl_event);
        }

        if (dev_ptr->flags & SCSI_BDR) {
	    /* make sure all luns are marked, as the bdr won't be issued */
            dev_index = dev_ptr->scsi_id << 3;
	    for (i=0; i < MAX_LUNS; i++) {
                if ((tmp_dev_ptr = ap->device_queue_hash[dev_index + i])
                        != NULL) {
		    /* luns with active commands will be handled by the */
		    /* outmost while loop */
                    if (tmp_dev_ptr->active_head == NULL) {
		        p8xx_fail_cmd(tmp_dev_ptr);
		    } else  /* mark flags so we don't hit this code again */
		        tmp_dev_ptr->flags &= ~SCSI_BDR;	
                }
            } /* end for loop checking each lun */
	} else 
            p8xx_fail_cmd(dev_ptr);  /* return pending cmds */ 

        ap->DEVICE_ACTIVE_head = dev_ptr->DEVICE_ACTIVE_fwd;
        dev_ptr->DEVICE_ACTIVE_fwd = NULL;
        dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
    }   /* end of for loop */

    ap->DEVICE_ACTIVE_tail = NULL;

    /* Cleanup the abort/bdr queue */
    while ((dev_ptr = ap->ABORT_BDR_head) != NULL) {
	p8xx_deq_abort_bdr(dev_ptr);
        if ((dev_ptr->ioctl_wakeup == TRUE) && not_dump_context) {
            if (dev_ptr->flags & CAUSED_TIMEOUT) 
                dev_ptr->ioctl_errno = ETIMEDOUT;
            else
                dev_ptr->ioctl_errno = EIO;

            dev_ptr->ioctl_wakeup = FALSE;
            e_wakeup(&dev_ptr->ioctl_event);
        }

	if (dev_ptr->flags & SCSI_BDR) {
	    /*
	     * no luns had active commands, if they did the flag value 
	     * would have been reset above.  Reset the flag and queue state 
	     */
            dev_index = dev_ptr->scsi_id << 3;
	    for (i=0; i < MAX_LUNS; i++) {
                if ((tmp_dev_ptr = ap->device_queue_hash[dev_index + i])
                        != NULL) {
 		    p8xx_fail_cmd(tmp_dev_ptr);
		}
	    } /* end for loop checking each lun */
	} else /* either an abort, or a bdr for an id which had active cmds */
	    p8xx_fail_cmd(dev_ptr);
    } /* end loop processing the abdr queue */

/* Finding a command or bp at this point means there were no
 * active commands (we would have called fail_cmd and returned them).
 * So this code needs to find any pending commands and bps, free any associated
 * resources, and park them on the correct save queue.
 */
    com_ptr = ap->DEVICE_WAITING_head;
    while (com_ptr != NULL) {
	if (com_ptr->bp->flags & SC_DELAY_CMD) {

	    dev_ptr = com_ptr->device;
	    /* find the next command before extracting. Upon exit of */
	    /* the loop we point to the end of a sublist about to be */
	    /* moved to the command save queue */
	    while ((com_ptr->active_fwd != NULL) &&
		   (dev_ptr == com_ptr->active_fwd->device)) {
		com_ptr = com_ptr->active_fwd;
	    }
	    com_ptr = com_ptr->active_fwd;
	    p8xx_freeze_qs(dev_ptr);
	} else
	    com_ptr = com_ptr->active_fwd;
    }

    bp = ap->REQUEST_WFR_head;
    while (bp != NULL) {
	if (bp->flags & SC_DELAY_CMD) {
	    tmp_bp = bp;
	    bp = (struct sc_buf *) bp->bufstruct.av_forw;
	    p8xx_freeze_qs(ap->device_queue_hash[INDEX(tmp_bp->
	      scsi_command.scsi_id, (tmp_bp->scsi_command.scsi_cmd.lun) >> 5)]);
	} else
	    bp = (struct sc_buf *) bp->bufstruct.av_forw;
    }

    /*
     * Clear the DMA and SCSI FIFO's on the chip
     */
    p8xx_write_reg(ap, CTEST3, CTEST3_SIZE, CTEST3_FLF);
    p8xx_write_reg(ap, STEST3, STEST3_SIZE, STEST3_CSF | STEST3_INIT);

    for (i = 0; i < MAX_SIDS; i++) {
        /* For all active target ids, note that a hard reset has occured */
        if (ap->sid_info[i].vscript_ptr != NULL) {
            ap->sid_info[i].negotiate_flag = TRUE;
            ap->sid_info[i].restart_in_prog = TRUE;
        }
	/* Any remembered negotiation results are also invalid */
        ap->device_state[i].flags &= ~NEGOT_DONE;
    }

    if (not_dump_context) {
        ap->restart_watchdog.dog.restart = ap->ddi.addi.cmd_delay + 1;
        W_START(&(ap->restart_watchdog.dog));
    }

    TRACE_1 ("out resetrcv", 0)
    P8printf(P8_EXIT,("leaving p8xx_scsi_reset_received\n"));
}  /* end p8xx_scsi_reset_received */

/**************************************************************************/
/*									  */
/* NAME:  p8xx_epow							  */
/*									  */
/* FUNCTION:  Scsi chip device driver suspend routine.			  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*	This routine can be called by a process. Chip not assumed on. 	  */
/*									  */
/* NOTES:  This routine is called when an EPOW (Early Power Off Warning)  */
/*	   has been detected.						  */
/*									  */
/* DATA STRUCTURES:							  */
/*	adapter_def - adapter unique data structure (one per adapter)	  */
/*	intr	-  kernel interrupt handler information structure	  */
/*									  */
/* INPUTS:								  */
/*	itsp - pointer to the intrpt handler structure			  */
/*									  */
/* RETURN VALUE DESCRIPTION:						  */
/*	INTR_SUCC - interrupt was not processed				  */
/*	INTR_SUCC - interrupt was processed				  */
/*									  */
/* ERROR DESCRIPTION:							  */
/*									  */
/**************************************************************************/
int
p8xx_epow(
	struct pintr	*itsp)
{
    ads_t	*ap;
    dvi_t	*dev_ptr;
    int		old_pri1, old_pri2, i;
    struct sc_buf * ret_bp;

    P8printf(P8_ENTRY,("Entering p8xx_epow routine.\n"));

    ap = itsp->ap;		/* ptr to associated adapt struct */
    TRACE_1("epow int", 0)

    if ((itsp->its.flags & EPOW_SUSPEND) || ((itsp->its.flags & EPOW_BATTERY) &&
	 (!(ap->ddi.addi.battery_backed)))) {

	P8printf(P8_INFO_1,("p8xx_epow: suspend\n"));
	ap->power_state |= EPOW_PENDING;
	/* reset the bus if chip is on */
	if ((ap->pmh.mode != PM_DEVICE_IDLE) &&
	    (!(ap->power_state & P8XX_PM_SYS_PWR)))
	    p8xx_command_reset_scsi_bus(ap,9);

    } else {

	/* 
	 * Restart again after an EPOW has occurred
	 */

	if (itsp->its.flags & EPOW_RESUME) {

	    /************************************************************/
	    /* Why do we disable interrupts when reseting the epow	*/
	    /* state flags but not when setting them???		RWW	*/
	    /************************************************************/

	    /* 
	     * Handle a resume command
	     */
	    P8printf(P8_INFO_1,("p8xx_epow: resume\n"));

	    /* Disable to int level during this */
	    old_pri1 = i_disable(ap->ddi.addi.int_prior);

	    /* Disable to close window around the test */
	    old_pri2 = i_disable(INTEPOW);

	    if (((!(itsp->its.flags & EPOW_BATTERY)) &&
		(!(itsp->its.flags & EPOW_SUSPEND))) &&
		(ap->power_state & EPOW_PENDING)) {
		ap->power_state &= ~EPOW_PENDING; /* reset epow state */
                i_enable(old_pri2);     /* return to lower priority */
                for (i = 0; i < MAX_DEVICES; i++) {
                    if (((dev_ptr = ap->device_queue_hash[i]) != NULL) &&
                        (dev_ptr->bp_save_head != NULL)) {
                        if ((ret_bp = p8xx_process_bp_save(dev_ptr)) != NULL)
                            p8xx_enq_wfr(ret_bp, ap);
                    }
                }
		if ((ap->DEVICE_ACTIVE_head == NULL) && 
	            (ap->DEVICE_WAITING_head != NULL)) {
		    /* The above call to process_bp_save might have
		     * moved a command to the wait queue.  If there is
		     * nothing on the active queue, it's okay to start
		     * the chip by calling issue_cmd.  First check for
		     * the remote possibility that the chip is off.
		     */
		    /* set power mgmt. activity field */
		    ap->pmh.activity = PM_ACTIVITY_OCCURRED;
		    if (ap->pmh.mode == PM_DEVICE_IDLE)
			p8xx_turn_chip_on(ap, TRUE);

		    (void) p8xx_issue_cmd(ap);
		}
	    } else {
		/*
		 * Either a SUSPEND has re-occurred, or this adap was not put
		 * in epow pending state. for these cases--leave adapter as is
		 */
	        i_enable(old_pri2);	/* return to lower priority */
	    }
	    i_enable(old_pri1); /* re-enable */
	}
    }

    P8printf(P8_EXIT,("leaving epow: intr_succeeded\n"));
    return (INTR_SUCC);
}  /* end p8xx_epow */

/************************************************************************/
/*									*/
/* NAME:  p8xx_pm_handler					        */
/*									*/
/* FUNCTION:  Routine to handle requests from power mgmt. core		*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*									*/
/* DATA STRUCTURES: 							*/
/*	the pm handle structure in the adapter_def structure is altered */
/*									*/
/* INPUTS: 								*/
/*	private - pointer to adapter structure				*/
/*      requested_mode - device pm state requested by the pm core       */
/*									*/
/* RETURN VALUE DESCRIPTION:     					*/
/*	PM_SUCCESS - transition made					*/
/*	PM_ERROR - transition not made					*/
/*									*/
/************************************************************************/
int 
p8xx_pm_handler(caddr_t private, int requested_mode)
{
    ads_t  *ap = (ads_t *) private;
    dvi_t  *dev_ptr;
    int ret_code = PM_SUCCESS;
    int old_pri, i, dev_index;
    uchar check_chip, cmd_to_service;
    struct sc_buf *bp;

    if (ap == NULL)
        return (PM_ERROR);

    TRACE_2("pmh rq/c", requested_mode, ap->pmh.mode)  
    switch (requested_mode) {
        case PM_DEVICE_FULL_ON:
	    if (ap->pmh.mode == PM_DEVICE_ENABLE) 
		/* transition to the requested state */
		ap->pmh.mode = requested_mode;
	    else
	        ret_code = PM_ERROR; /* bad state transition */
	    break;

	case PM_DEVICE_ENABLE:
	    if (pincode(p8xx_pm_handler) == 0) {
    	  	old_pri = i_disable(ap->ddi.addi.int_prior);
		switch(ap->pmh.mode) {
		   case PM_DEVICE_FULL_ON:
		        ap->pmh.mode = requested_mode;
		   	break;

		   case PM_DEVICE_SUSPEND:
		   case PM_DEVICE_HIBERNATION:
		    	ap->power_state &= ~P8XX_PM_SYS_PWR;
			TRACE_1("power state", ap->power_state)
			p8xx_turn_chip_on(ap, TRUE);
			
			check_chip = TRUE;
			for (i=0; i<MAX_DEVICES;i++) {
			    if (((dev_ptr = ap->device_queue_hash[i]) != NULL) 
				  && (dev_ptr->bp_save_head != NULL)) {
				check_chip = p8xx_unfreeze_qs(dev_ptr, NULL, 
							check_chip);
			    }
			}
			break;

		    case PM_DEVICE_IDLE:
			p8xx_turn_chip_on(ap, TRUE);
			break;

		    default:
			ret_code = PM_ERROR;
			break;
		} /* end of inner switch */
		i_enable(old_pri);
		unpincode(p8xx_pm_handler);

	    } else
		ret_code = PM_ERROR;
	    break;

	case PM_DEVICE_IDLE:
	    if (pincode(p8xx_pm_handler) == 0) {
    	  	old_pri = i_disable(ap->ddi.addi.int_prior);
	        switch(ap->pmh.mode) {

		    case PM_DEVICE_ENABLE:
			/* Check if conditions permit going to device_idle */
			if ((ap->DEVICE_ACTIVE_head != NULL) ||
			    (ap->ABORT_BDR_head != NULL) ||
			    (ap->reset_pending)) {
			    ret_code = PM_ERROR;
			} else {
			    /* We can go to device idle, and try to turn off 
			     * the chip. First mask all interrupts from the 
			     * chip, then call planar control.
			     */
    			    (void) p8xx_write_reg(ap, SIEN, SIEN_SIZE, 0x0000);
        		    (void) p8xx_write_reg(ap, DIEN, DIEN_SIZE, 0x00);
			    pm_planar_control(ap->devno, (PMDEV_PCI0000 | 
				BID_NUM(ap->bus_bid) << 8 | ap->ddi.addi.slot),
                                PM_PLANAR_OFF);
			    ap->pmh.mode = requested_mode;
			}
			break;

		    case PM_DEVICE_SUSPEND:
		    case PM_DEVICE_HIBERNATION:
			ap->power_state &= ~P8XX_PM_SYS_PWR;

                        /* 
                         * Must allow any frozen commands a chance 
                         * to execute before going to idle.  Go to enable
			 * if one of these cmds can be started now.
			 * First determine if there are any held commands.
                         */
			cmd_to_service = FALSE;
			for (i=0;i<MAX_DEVICES;i++) {
			    if (((dev_ptr = ap->device_queue_hash[i]) != NULL) 
			      && ((dev_ptr->bp_save_head != NULL) ||
				  (dev_ptr->cmd_save_head != NULL)))  {
			        cmd_to_service = TRUE;
				break;
			    }
			}

			if  (!cmd_to_service) {

			    ap->pmh.mode = requested_mode;
			    TRACE_1("no cmds to serve", 0)

			} else {
			    /* cmds are available, turn chip on (changing
			     * state to enable as well), then call unfreeze.
		             */	
			    p8xx_turn_chip_on(ap, TRUE);

		            check_chip = TRUE;
			    for (i=INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);
				 i<MAX_DEVICES;i++) {
			        if (((dev_ptr = ap->device_queue_hash[i]) 
					!= NULL) 
				   && ((dev_ptr->bp_save_head != NULL) || 
				       (dev_ptr->cmd_save_head != NULL)))  {
				    check_chip = p8xx_unfreeze_qs(dev_ptr, 
							NULL, check_chip);
				}
			    }
			}
			break;

		    default:
			ret_code = PM_ERROR;
			break;
		} /* end of inner switch */
		
		i_enable(old_pri);
		unpincode(p8xx_pm_handler);
	    } else 
		ret_code = PM_ERROR;
	    break;

	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
    	    old_pri = i_disable(ap->ddi.addi.int_prior);
	    switch (ap->pmh.mode) {

		case PM_DEVICE_ENABLE:
		    if ((ap->DEVICE_ACTIVE_head != NULL) ||
			(ap->ABORT_BDR_head != NULL) ||
			(ap->reset_pending)) {

			ret_code = PM_ERROR;
			break;
		    }
		    /* if we are going to make the transition, the logic
		     * is almost the same as for the idle case.  Fall into
		     * that logic.
		     */

		case PM_DEVICE_IDLE:
		    
		    /* assuming can't be in idle/have no active cmds and 
		     * have anything on the wait queue
		     */
		    ASSERT(ap->DEVICE_WAITING_head == NULL);

		    /* process the waiting-for-resources queue */
		    while ((bp = ap->REQUEST_WFR_head) != NULL) {
			TRACE_1("processing wfr", (int) bp)
            		dev_index = INDEX(bp->scsi_command.scsi_id,
                          (bp->scsi_command.scsi_cmd.lun) >> 5);
            		dev_ptr = ap->device_queue_hash[dev_index];
		
			/* remove from wfr queue */
                	p8xx_deq_wfr (bp, NULL, ap);

			/* put at head of bp save queue */
        		if (dev_ptr->bp_save_tail == NULL)
            		    dev_ptr->bp_save_head = bp;
        		else
            		    dev_ptr->bp_save_tail->bufstruct.av_forw = 
				(struct buf *) bp;
        		dev_ptr->bp_save_tail = bp;
		    }

		    if (ap->pmh.mode == PM_DEVICE_ENABLE) {
			/* block interrupts from the chip, to handle
			 * the possibility that the chip won't really
			 * be powered off.
			 */
    			(void) p8xx_write_reg(ap, SIEN, SIEN_SIZE, 0x0000);
        		(void) p8xx_write_reg(ap, DIEN, DIEN_SIZE, 0x00);
			 
		        if (requested_mode == PM_DEVICE_SUSPEND) { 
			    /* for suspend, call planar_control since
			     * chip isn't already off. (for hibernate, pm
			     * core will handle it)
			     */
			    TRACE_1("planar off", 0)
			    pm_planar_control(ap->devno, (PMDEV_PCI0000 | 
				BID_NUM(ap->bus_bid) << 8 | ap->ddi.addi.slot),
                                PM_PLANAR_OFF);
			}
		    }

		    ap->pmh.mode = requested_mode;
		    ap->power_state |= P8XX_PM_SYS_PWR;
		    break;

	        default:
		    ret_code = PM_ERROR;
		    break;
	    } /* end of inner switch */
	    i_enable(old_pri);
	    break;

        case PM_PAGE_FREEZE_NOTICE:
            /* Pin the bottom half of the driver */
            if (pincode(p8xx_intr))
                ret_code = PM_ERROR;
            break;

        case PM_PAGE_UNFREEZE_NOTICE:

            /* Unpin the bottom half of the driver */
            (void) unpincode(p8xx_intr);
 	    break;

	default:
	    ret_code = PM_ERROR;
	    break;
    }
    TRACE_2("pmh c/rc", ap->pmh.mode, ret_code)  
    return (ret_code);
}

/************************************************************************
 *								
 * NAME:	p8xx_turn_chip_on
 *						
 * FUNCTION:   This function is called to manage the resumption of
 *	power to the chip.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine can be called from the process environment.
 *						
 * DATA STRUCTURES:			
 *	dev_info structure - target lun unique data structure
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *	change_pmh_mode - flag used to indicate whether to change the
 *			pmh mode to the Enable state
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void 
p8xx_turn_chip_on(ads_t *ap, int change_pmh_mode) {
    int i;

    TRACE_1("turn chip on", change_pmh_mode)
    if (change_pmh_mode)
        ap->pmh.mode = PM_DEVICE_ENABLE;

    (void) pm_planar_control(ap->devno, (PMDEV_PCI0000 | 
             BID_NUM(ap->bus_bid) << 8 | ap->ddi.addi.slot), PM_PLANAR_ON);
    (void) p8xx_config_adapter(ap);

    /* call chip_register_reset to ensure all registers are at
     * their power-on values, and then to set the appropriate
     * registers to non-default values.
     */
    p8xx_chip_register_reset(ap);

    /* Since the chip was powered off, it's likely target devices were
     * also powered off.  Set the flag to force negotiation with them
     * on the next command, to be sure.
     */
    for (i = 0; i < MAX_SIDS; i++) {
        /* For all active target ids, note negotiation is needed */
        if (ap->sid_info[i].vscript_ptr != NULL) {
            ap->sid_info[i].negotiate_flag = TRUE;
        }
	/* Any remembered negotiation results are also considered invalid */
        ap->device_state[i].flags &= ~NEGOT_DONE;
    }

}

/************************************************************************/
/*									*/
/* NAME:  p8xx_watchdog						        */
/*									*/
/* FUNCTION:  Command timer routine.					*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.  Chip not assumed on.	*/
/*									*/
/* NOTES:								*/
/*	This routine is called when a command has timed out.  A BDR	*/
/*	is issued in an attempt to cleanup the device.  This routine	*/
/*	executes to reset the bus during normal command executtion.	*/
/*									*/
/* DATA STRUCTURES: 							*/
/*	w - gives you the address of the dev_info structure		*/
/*									*/
/* INPUTS: 								*/
/*	ap - pointer to adapter structure				*/
/*	w - pointer to watch dog timer structure which timed out	*/
/*									*/
/* RETURN VALUE DESCRIPTION: None					*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*									*/
/************************************************************************/
void
p8xx_watchdog(
	struct watchdog *w)
{
    ads_t	*ap;
    dvi_t	*dev_ptr, *tmp_dev_ptr;
    cmd_t	*com_ptr;
    struct sc_buf *bp;
    struct timer *tdog;
    int		i, old_pri;
    uchar	check_chip;
    int		dev_index, lun;
    int     base_time;    /* save the largest timeout value on the */
                          /* active list; used to detect real t_o  */



    tdog = (struct timer *) w;
    ap = tdog->ap;

    P8printf(P8_ENTRY|P8_ABORT,
	("Entering p8xx_watchdog, timer_id = %x\n",
	tdog->timer_id));

    old_pri = i_disable(ap->ddi.addi.int_prior);
    TRACE_1 ("in watch", tdog->timer_id)

    P8if(P8_DEVICE_STATUS,(p8xx_rpt_status(ap, "At watchdog",1)));

    if (tdog->timer_id == PSC_RESET_TMR) {

	W_STOP(&ap->reset_watchdog.dog);

	/*
	 * If we failed in causing the SCSI bus to reset.  This
	 * guarantees that the chip is hung, so reset the chip.
	 */

	/* update the power mgmt. activity field */
 	ap->pmh.activity = PM_ACTIVITY_OCCURRED;

	/* assume the chip is powered on, since we won't power it off if a scsi
	 * bus reset is pending.
	 */
	p8xx_chip_register_reset(ap);
	p8xx_command_reset_scsi_bus(ap,11);

    } else if (tdog->timer_id == PSC_RESTART_TMR) {

        TRACE_1 ("restart timer", 0);
	W_STOP(&ap->restart_watchdog.dog);
	if (ap->power_state != P8XX_NORMAL_PWR) {
	    /* An EPOW is pending, or a suspend/hibernate.
             * Restart the timer 
	     */
	    ap->restart_watchdog.dog.restart = ap->ddi.addi.cmd_delay + 1;
	    W_START(&ap->restart_watchdog.dog);
	} else {
	    /* Loop through hash table restarting each device */
	    check_chip = TRUE;
	    for (i = 0; i < MAX_SIDS; i++) {
		if ((ap->sid_info[i].vscript_ptr != NULL) && 
		    (ap->sid_info[i].restart_in_prog)) {

		    ap->sid_info[i].restart_in_prog = FALSE;
                    dev_index = ap->sid_info[i].scsi_id << 3;
	            TRACE_1("dev index", (int) dev_index)
                    /* for each lun, unfreeze its queues */
                    for (lun = 0; lun < MAX_LUNS; lun++) {
                        if ((dev_ptr = ap->device_queue_hash[
                                dev_index + lun]) != NULL) {
			    TRACE_2("devptr", (int) dev_ptr, (int) check_chip)
                            check_chip = p8xx_unfreeze_qs(dev_ptr,
                                        NULL, check_chip);
                        }
                    } /* end lun for loop */
		}
	    } /* end id for loop */
	} /* end else not EPOW */

    } else { /* PSC_COMMAND_TMR */

	/* If here a possibly fatal command timeout occurred */
	
	/* The chip is assumed to be powered on, since the cmd timer 
	 * wouldn't have been running unless there was an active
	 * command (and the chip must be on to have an active cmd.
	 */

	dev_ptr = (dvi_t *) ((uint) (tdog));

        TRACE_2("cmd tmr", (int) dev_ptr, (int) dev_ptr->cmd_activity_state)
	P8printf(P8_INFO_2,("ID %02x, Cmd_act_st = %d\n",
	    dev_ptr->scsi_id, dev_ptr->cmd_activity_state));

	W_STOP(&dev_ptr->dev_watchdog.dog);
	dev_ptr->flags |= CAUSED_TIMEOUT;

        /* is an abort currently running, or is a bdr pending or
         * currently running?  (the pending bdr case is included in
         * case we wrote sigp to issue a bdr and the chip is hung.)
         */
        if ((dev_ptr->cmd_activity_state == ABORT_IN_PROGRESS) ||
            (dev_ptr->flags & SCSI_BDR)) {
	    P8printf(P8_INFO_1,
		("p8xx_watchdog: ABORT/BDR in PROGRESS\n"));

            if (((bp = dev_ptr->active_head->bp) != NULL) &&
                (!(bp->bufstruct.b_flags & B_ERROR)))
            {
                bp->status_validity = SC_ADAPTER_ERROR;
                bp->general_card_status = SC_CMD_TIMEOUT;
                bp->scsi_status = SC_GOOD_STATUS;
                bp->bufstruct.b_flags |= B_ERROR;
                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                bp->bufstruct.b_error = EIO;
            }
	    /* update the power mgmt. activity field */
 	    ap->pmh.activity = PM_ACTIVITY_OCCURRED;
	    p8xx_command_reset_scsi_bus(ap,12);	 /* reset SCSI bus */

	} else {     /* Regular command */

	    P8printf(P8_INFO_1,
		("p8xx_watchdog: regular cmd\n"));
            TRACE_1 ("regular cmd", (int) dev_ptr)

            if (((bp = dev_ptr->active_head->bp) == NULL) ||
                (dev_ptr->cmd_activity_state == NOTHING_IN_PROGRESS)) {
                /* this should not occur, a null bp indicates an abort */
                /* or bdr.  Log the error and exit. */
                p8xx_logerr(ERRID_SCSI_ERR6, UNKNOWN_ADAPTER_ERROR, 0xB7,
                   0, ap, dev_ptr->active_head, TRUE);
                TRACE_1 ("out watch", -1)
                i_enable(old_pri);
                return;
            }

            /* is there more than one active command (are we queuing?) */
            if (dev_ptr->active_head != dev_ptr->active_tail) {
		/*
                 * determine if this is a valid timeout.     
                 * walk the list of commands on this device's
                 * active queue to ensure that no cmd on the list
                 * has a timeout value greater than that at the
                 * head of the list. If a greater value exists,
                 * an erroneous timeout may have occurred as a
                 * result of the device reordering commands.
		 */
                base_time = bp->timeout_value;
                com_ptr = dev_ptr->active_head->active_fwd;
                while(com_ptr != NULL) {
                    if (com_ptr->bp != NULL) {
                        if (com_ptr->bp->timeout_value > base_time )
                            base_time = com_ptr->bp->timeout_value;
                        else if (com_ptr->bp->timeout_value == 0) {
                            base_time = 0;
                            break;
                        }
                    }
                    com_ptr = com_ptr->active_fwd;
                } /* end while */

		/*
                 * if a command on the active list has a timeout
                 * value greater than that of the head command,
                 * then restart the wdog with the difference of
                 * of the two timeout values; this was not a true
                 * timeout.   save_time is used so the code does
                 * restart the timer for the same command more than
                 * once.  Without using save_time, the timer would
                 * again expire and we would find the same command
                 * with a timeout value greater than the head and
                 * perpetually restart the timer.
                 * Restart the timer with a zero value if a cmd on 
                 * the active list has a timeout value of zero. 
		 */
                if (((base_time > bp->timeout_value) && (base_time >
                         dev_ptr->dev_watchdog.save_time)) ||
                     (base_time == 0)) {
                    if (base_time)
                        dev_ptr->dev_watchdog.dog.restart =
                            base_time - bp->timeout_value + 1;
                    else
                        dev_ptr->dev_watchdog.dog.restart = 0;
                    dev_ptr->dev_watchdog.save_time = base_time;
                    dev_ptr->flags &= ~CAUSED_TIMEOUT;
                    w_start(&dev_ptr->dev_watchdog.dog);
                    TRACE_1("false to", dev_ptr->dev_watchdog.dog.restart)
                    i_enable(old_pri);
                    return;
                }
                else {  /* a real timeout has been detected */
                    /* reset save_time */
                    dev_ptr->dev_watchdog.save_time = 0;
                }
            } /* end if command queuing device */

            /* at this point we know we that a valid bp has actually
             * timed-out.  We mark the bp, and then set up for a bdr
	     * If we were doing wide negotiation for this command,
	     * assume the negotiation sequence led to the timeout, and
	     * handle the bp specially.
	     */

	    if (dev_ptr->ioctl_wakeup) {
		dev_ptr->ioctl_errno = ETIMEDOUT;
		dev_ptr->ioctl_wakeup = FALSE;
		e_wakeup(&dev_ptr->ioctl_event);
	    }

	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->scsi_status = SC_GOOD_STATUS;

	    if (dev_ptr->active_head->flags & NEGOT_CMD) {
	        dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR; 
		/* turn off the bit in scntl3, it is possibly on */
	        dev_ptr->sid_ptr->io_table.scntl3  &= ~SCNTL3_EWS;
		dev_ptr->sid_ptr->negotiate_flag = TRUE;
                bp->bufstruct.b_error = ENXIO;
	        bp->general_card_status = SC_SCSI_BUS_FAULT;
                bp->status_validity = 0;
	    } else {
                bp->bufstruct.b_error = EIO;
                bp->status_validity = SC_ADAPTER_ERROR;
	        bp->general_card_status = SC_CMD_TIMEOUT;
	    }

            /* if a bdr is not already queued, set up for one and signal */
            /* the chip */
            if (!(dev_ptr->flags & SCSI_BDR)) {

                dev_ptr->flags &= ~RETRY_ERROR;
                dev_ptr->retry_count = 1;
                if (!(dev_ptr->flags & SCSI_ABORT))
                    p8xx_enq_abort_bdr(dev_ptr);

                /* mark all LUNs for the scsi id */
                dev_index = dev_ptr->scsi_id << 3;
                for (i = 0; i< MAX_LUNS; i++) {
                    if ((tmp_dev_ptr = ap->device_queue_hash[dev_index + i])
                        != NULL) {
                        tmp_dev_ptr->flags |= SCSI_BDR;
                        tmp_dev_ptr->queue_state &= STOPPING;
                        tmp_dev_ptr->queue_state |= WAIT_FLUSH;
                    }
                }

                dev_ptr->sid_ptr->bdring_lun = dev_ptr;

                /* signal the chip that there is new work */
	        SUSPEND_CHIP(ap);
		/*
                 * we start timing the command here, even though
                 * at this point we are only trying to stop the chip
                 * The timer guards against the chip being hung, at
                 * the risk of leading to a bus reset if a device
                 * at this point is on the bus for 5 seconds and the 
                 * SIGP therefore has no effect.
		 */
                dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
                W_START(&dev_ptr->dev_watchdog.dog);

            } /* if no BDR already queued */
        } /* else regular command */
    }   /* else command timer */
    TRACE_1 ("outwatch", 0)
    i_enable(old_pri);

    P8if(P8_DEVICE_STATUS,
	(p8xx_rpt_status(ap, "Exit from watchdog",1)));
    P8printf(P8_EXIT,("Leaving p8xx_watchdog\n"));

}  /* p8xx_watchdog */

/************************************************************************/
/*									*/
/* NAME:  p8xx_command_reset_scsi_bus					*/
/*									*/
/* FUNCTION:  Resets the SCSI bus lines.				*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process. Chip assumed on.	*/
/*									*/
/* NOTES:  This routine will toggle the SCSI bus reset line for 30	*/
/*	microseconds to assert a reset on the SCSI bus. This routines	*/
/*	executes to reset the bus during normal command executtion.	*/
/*									*/
/* DATA STRUCTURES: None						*/
/*									*/
/* INPUTS:								*/
/*	ads_t *ap	Pointer to our adapter structure		*/
/*	int reason	Used to help with debugging SCSI bus reset	*/
/*			errors.						*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	PSC_NO_ERR - for good completion.				*/
/*	ERRNO	   - value otherwise					*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*									*/
/************************************************************************/
void
p8xx_command_reset_scsi_bus(
	ads_t	*ap,
	int	reason)
{
    uchar reset_chip;
    int	    rc, i;
    int	    old_pri;
    static int	count = 0;

    P8printf(P8_ENTRY|P8_ABORT,("Resetting the SCSI bus, reason %d, count %d\n",
	reason, ++count));
    P8if(P8_ASSERT, {assert(0);});

    old_pri = i_disable(ap->ddi.addi.int_prior);
    TRACE_1 ("in busrs", reason)

    p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, ISTAT_ABRT);	/* assert SIOP abort */
    reset_chip = TRUE;

    /* Check to see when abort has occurred */
    for (i = 0; i < 30; i++) {
	io_delay(1);

	/* Deassert abort cmd */
	rc = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
	if (!(rc & ISTAT_DIP)) {
	    if (rc & ISTAT_SIP) {
		(void) p8xx_read_reg(ap, SIST0, SIST0_SIZE);
		io_delay(1);
		(void) p8xx_read_reg(ap, SIST1, SIST1_SIZE);
	    }
	    continue;
	}

	/* Deassert abort cmd */
	p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, 0x00);

	/* Read the SCSI and DMA status registers to clear them */
	rc = p8xx_read_reg(ap, DSTAT, DSTAT_SIZE);

	if (!(rc & DABRT)) {
	    p8xx_write_reg(ap, ISTAT, ISTAT_SIZE, ISTAT_ABRT);
	    continue;
	}
	reset_chip = FALSE;
	break;
    }

    /* 
     * Handle fall through case if we failed in causing a chip abort, the
     * chip is hung. reset the chip and DMA before doing a scsi bus reset.
     */
    if (reset_chip)
	p8xx_chip_register_reset(ap);

    /* 
     * Write SCNTL1 to assert SCSI reset
     */
    (void) p8xx_write_reg(ap, SCNTL1, SCNTL1_SIZE, SCNTL1_RST);
    ap->reset_pending = TRUE;
    io_delay(30);

    /* Write SCNTL1 to deassert SCSI reset */
    (void) p8xx_write_reg(ap, SCNTL1, SCNTL1_SIZE, SCNTL1_INIT);

    /* start the reset bus watchdog timer */
    if (!ap->dump_started)
	W_START(&ap->reset_watchdog.dog);

    i_enable(old_pri);
    P8printf(P8_EXIT,("LEAVING p8xx_command_reset_scsi_bus\n"));

}  /* p8xx_command_reset_scsi_bus */


/************************************************************************
 *								
 * NAME:	p8xx_poll_for_bus_reset
 *						
 * FUNCTION:   This function is called to poll the chip to see if a scsi
 *	reset has occurred, or until a timeout occurs.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts completely disabled.  The chip 
 *	is assumed to be on.
 *						
 * NOTES:  If the reset occurs, p8xx_scsi_reset_received is called to
 *	clean adapter queues and otherwise process the reset.
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *	It is left to the caller to determine if the reset occurred or
 *	a time-out occurred (the ap->reset_pending flag is cleared if
 *	the reset occurred).
 ************************************************************************/
void
p8xx_poll_for_bus_reset(ads_t *ap)
{
    int i, save_isr, save_sist;

    i = 0;
    /* wait up to RESETWAIT seconds for the bus reset to complete */
    while (++i < (RESETWAIT * 1000)) {
        /* Delay for 1000th of a second */
        io_delay(1000);

        save_isr = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
        if (!(save_isr & ISTAT_INTR)) {

            /* If not our interrupt go around again */
	    continue;
	}

        if (save_isr & ISTAT_DIP) {
            /* Clear pending DMA interrupt */
            (void) p8xx_read_reg(ap, DSTAT, DSTAT_SIZE);
        }

        if (save_isr & ISTAT_SIP) {
	    /* Clear pending SCSI interrupt */
            save_sist = p8xx_read_reg(ap, SIST, SIST_SIZE);
            TRACE_1("sip", (int) save_sist)
            if (save_sist & SCSI_RST) {
		/* the SCSI interrupt was for a bus reset */
                p8xx_scsi_reset_received(ap);
	        break;
	    }
        }

    } /* end loop waiting for bus reset */
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p8xx_verify_tag                                                 */
/*                                                                        */
/* FUNCTION:  Finds the dev_info whose script addr is in the DSP          */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      called by the interrupt handler, chip assumed on              	  */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:     	                                                          */
/*	ads_t * - pointer to the adapter information structure		  */
/*      com_ptr - pointer to the command structure associated w/ the tag  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      TRUE    tag is valid                                              */
/*      FALSE   tag appears invalid given dsp or i/o errors               */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int 
p8xx_verify_tag(ads_t *ap, struct cmd_info * com_ptr)
{
    int     dsp;

    /* theory is get script dma addr from DSP       */
    /* and then compare it with the information     */
    /* accessible via the candidate command struct. */

    dsp = p8xx_read_reg(ap, DSP, DSP_SIZE);
    TRACE_1 ("fnd_info", dsp)

    /* if dsp is within the main script associated with the candidate
     * command structure, assume we have the correct tag if the tag 
     * is also marked in use.
     */
    if ((com_ptr->device != NULL) && 
	(dsp >= (com_ptr->device->sid_ptr->bscript_ptr + 
		Ent_scripts_entry_point)) && 
	(dsp <= (com_ptr->device->sid_ptr->bscript_ptr + 
		Ent_cmd_msg_in_buf))) {
	if (com_ptr->in_use)
            return (TRUE);
    }
    TRACE_3("vtag", (int) com_ptr, (int) dsp, 0)
    return (FALSE);
}  /* end of p8xx_verify_tag */

/************************************************************************/
/*									*/
/* NAME:  p8xx_issue_abort_bdr						*/
/*									*/
/* FUNCTION:  Handles the issueing of bdr/aborts for interrupts		*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	may be called by interrupt or process.  chip assumed on.	*/
/*									*/
/* DATA STRUCTURES: None						*/
/*									*/
/* INPUTS:								*/
/*	dvi_t, struct sc_buf,             			  	*/
/*	int iodone_flag	                    				*/
/*	uchar connected - flag indicating BSY on SCSI bus		*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	none								*/
/*									*/
/* ERROR DESCRIPTION:							*/
/*									*/
/************************************************************************/
int
p8xx_issue_abort_bdr(
                    struct dev_info * dev_ptr,
                    struct sc_buf *bp,
                    uint iodone_flag, uchar connected)
{
    struct cmd_info * com_ptr;
    ads_t *ap;

    ap = dev_ptr->ap;
    TRACE_1 ("in issAB", (int) dev_ptr)

    /* set the status if not previously marked */
    if (!(bp->bufstruct.b_flags & B_ERROR))
    {
	/* if called after an iodone, need to get the status */
	if (iodone_flag)
        {
            bp->scsi_status = GET_STATUS(dev_ptr->sid_ptr->vscript_ptr);
            if (bp->scsi_status != SC_GOOD_STATUS)
            {
                bp->status_validity = SC_SCSI_ERROR;
                bp->bufstruct.b_error = EIO;
                bp->bufstruct.b_flags |= B_ERROR;
		if (bp->scsi_status == SC_CHECK_CONDITION)
		    dev_ptr->sid_ptr->negotiate_flag = TRUE;   
            }
            else
            {
                bp->bufstruct.b_error = 0;
                bp->scsi_status = SC_GOOD_STATUS;
                bp->status_validity = 0;
            }
        }
        else  /* not iodone flag */
        {
            bp->status_validity = 0;
            bp->scsi_status = 0;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            bp->bufstruct.b_error = ENXIO;
            bp->bufstruct.b_flags |= B_ERROR;
        }
    }

    if (dev_ptr->flags & SCSI_BDR)
    {
	/* access the dev_ptr the bdr is anchored upon, to ensure the 
	 * correct ioctl sleeper (if any) will be identified 
	 */
	if (dev_ptr != dev_ptr->sid_ptr->bdring_lun)
	{
	    dev_ptr = dev_ptr->sid_ptr->bdring_lun;
	}

        com_ptr = &ap->command[ap->ddi.addi.card_scsi_id << 3];
        com_ptr->bp = NULL;
        com_ptr->device = dev_ptr;
	com_ptr->resource_state = NO_RESOURCES_USED;
	com_ptr->in_use = TRUE;
        p8xx_enq_active(dev_ptr, com_ptr);

	p8xx_issue_bdr_script(dev_ptr, com_ptr, connected);
 
        dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;
        dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
        dev_ptr->flags &= ~RETRY_ERROR;
        dev_ptr->retry_count = 1;

        W_START(&dev_ptr->dev_watchdog.dog);
        p8xx_deq_abort_bdr(dev_ptr);
        TRACE_1 ("outissB", 0)
        return (FALSE);
    }
    else        /* abort was requested */
    {
        /* remove device from ABRT/BDR queue */
        p8xx_deq_abort_bdr(dev_ptr);
	
	/* if there are still active commands (assuming this one is
	 * still on the queue), then issue the abort.
         */
        if (dev_ptr->active_head != dev_ptr->active_tail)
	{
	    dev_ptr->cmd_activity_state = ABORT_IN_PROGRESS;
            com_ptr = &ap->command[ap->ddi.addi.card_scsi_id << 3];
            com_ptr->bp = NULL;
            com_ptr->device = dev_ptr;
	    com_ptr->resource_state = NO_RESOURCES_USED;
	    com_ptr->in_use = TRUE;
            p8xx_issue_abort_script(com_ptr, connected);

            dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
            dev_ptr->flags &= ~RETRY_ERROR;
            dev_ptr->retry_count = 1;

            W_START(&dev_ptr->dev_watchdog.dog);
            TRACE_1("out issabrt", 0)
	    return (FALSE);
	}
	else /* no other active commands */
	{
            p8xx_fail_cmd(dev_ptr);

            /* if IOCTL wakeup flag is TRUE then call wakeup */
            /* and set IOCTL status to EIO                   */
            if (dev_ptr->ioctl_wakeup == TRUE)
            {
                dev_ptr->ioctl_errno = EIO;
                dev_ptr->ioctl_wakeup = FALSE;
                e_wakeup(&dev_ptr->ioctl_event);
            }
	} /* end else no other active commands */
    }   /* end else  abort */
    TRACE_1 ("outissAB", 4)
    return (TRUE);
}  /* p8xx_issue_abort_bdr */

/**************************************************************************/
/*                                                                        */
/* NAME:  p8xx_scsi_interrupt                                             */
/*                                                                        */
/* FUNCTION:  Handles the scsi interrupts (besides RESET) when a command  */
/*      structure has been identified.                                    */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Called by the interrupt handler, chip assumed on.                 */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      struct cmd_info:  command associated with the error               */
/*      struct dev_info:  target id/lun associated with the error         */
/*      save_interrupt:   encodes which error occurred                    */
/*      issue_abrt_bdr:   bit 1: is an unexpected disconnect int. pending */
/*                               (only used for Selection Time-outs)      */
/*                        bit 0: should an abort be issued                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      flag indicating whether a new job should be started on the chip   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p8xx_scsi_interrupt(
                      struct cmd_info * com_ptr,
                      struct dev_info * dev_ptr,
                      int save_interrupt,
                      int issue_abrt_bdr)
{
    struct sc_buf *bp;
    ads_t       *ap;
    struct dev_info *tmp_dev_ptr;
    uchar   log_err, gen_err;
    int     i, istat_val, sist_val;
    uint    dev_index;

    ap = dev_ptr->ap;
    TRACE_2 ("in sint ", (int) dev_ptr, (int) com_ptr)
 
    P8printf(P8_ENTRY,("Entering p8xx_scsi_interrupt\n"));
    P8printf(P8_INFO_2,
        (" dev_ptr=0x%x, save_interrupt=0x%04x, issue_abrt_bdr=%s\n",
        dev_ptr, save_interrupt, issue_abrt_bdr ? "TRUE":"FALSE"));

    bp = com_ptr->bp;
    TRACE_1("bp is ", (int) bp)

    if (!ap->dump_started)
        W_STOP(&dev_ptr->dev_watchdog.dog);

    switch (save_interrupt)
    {
    case SCSI_UNEXP_DISC:
        log_err = UNEXPECTED_BUS_FREE;
        gen_err = SC_SCSI_BUS_FAULT;
        break;
    case SCSI_GROSS_ERR:
        log_err = GROSS_ERROR;
        gen_err = SC_SCSI_BUS_FAULT;
        break;
    case SCSI_PARITY_ERR:
        log_err = BAD_PARITY_DETECTED;
        gen_err = SC_SCSI_BUS_FAULT;
        break;
    case SCSI_SEL_TIMEOUT:
        dev_ptr->flags |= SELECT_TIMEOUT;
        log_err = UNEXPECTED_SELECT_TIMEOUT;
        gen_err = SC_NO_DEVICE_RESPONSE;
        break;
    }
    /* for selection timeouts, the chip was designed to */
    /* return an unexpected disconnect after a selection */
    /* timeout.  So in the cases of a selection timeout */
    /* where the ISTAT shows another SIP interrupt wait- */
    /* ing, it can be assumed that this is the discon-  */
    /* nect that is expected.  An attempt is made to    */
    /* clear it here via a read loop of the istat for up */
    /* to 10 usecs.  */
    if ((save_interrupt == SCSI_SEL_TIMEOUT) && 
	(issue_abrt_bdr & DISC_PENDING))
    {
        /* turn off bit indicating unexpected disconnect pending */
        issue_abrt_bdr &= ~DISC_PENDING;

        /* loop looking to clear the unexpected disconnect int.  */
        for (i = 0; i < 10; i++)
        {
            io_delay(1);
            istat_val = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
            if (istat_val & ISTAT_SIP)
            {
                sist_val = p8xx_read_reg(ap, SIST, SIST_SIZE);
                if (sist_val & SCSI_UNEXP_DISC)
                        break;
            }
        }  /* end for loop */
    }
    /* for unexpected disconnects, it is possible that a */
    /* selection timeout interrupt is stacked due to the */
    /* design of the chip.  Here, the interrupt is      */
    /* cleared and the error returned is set to repre-  */
    /* set a selection timeout.                         */
    if (save_interrupt == SCSI_UNEXP_DISC) {

        istat_val = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
        if (istat_val & ISTAT_SIP) {
            sist_val = p8xx_read_reg(ap, SIST, SIST_SIZE);
            if (sist_val & SCSI_SEL_TIMEOUT) {
                save_interrupt = SCSI_SEL_TIMEOUT;
                dev_ptr->flags |= SELECT_TIMEOUT;
                log_err = UNEXPECTED_SELECT_TIMEOUT;
                gen_err = SC_NO_DEVICE_RESPONSE;
		/* Always send an abort if we get a selection */
		/* time-out and are queuing to the target */
		if (dev_ptr->active_head == dev_ptr->active_tail)
                    issue_abrt_bdr = FALSE;
		else
		    issue_abrt_bdr = TRUE;
            }
        }

	if (save_interrupt != SCSI_SEL_TIMEOUT) {
	    /* an actual unexpected disconnect may leave have left data in the 
	     * fifos, clear the fifos out.
	     */
	    p8xx_write_reg(ap, CTEST3, CTEST3_SIZE, CTEST3_FLF);
	    p8xx_write_reg(ap, STEST3, STEST3_SIZE, STEST3_CSF | STEST3_INIT);

	    /* unexpected disconnects that occur during commands that had
	     * negotiation are treated specially.  We do the processing
	     * here, and mark the bp so it won't be logged.
	     */
	    if ((com_ptr->flags & NEGOT_CMD) &&
		    (save_interrupt == SCSI_UNEXP_DISC)) {
	        dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR; 
		/* turn off the bit in scntl3, it is possibly on */
	        dev_ptr->sid_ptr->io_table.scntl3  &= ~SCNTL3_EWS;
		dev_ptr->sid_ptr->negotiate_flag = TRUE;
        	if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR))) {
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_error = ENXIO;
                    bp->status_validity = 0;
	            bp->general_card_status = SC_SCSI_BUS_FAULT;
		}
	    }
	}
    }

    /* if we were requested to issue an abort (via the issue_abrt_bdr flag) */
    /* issue it if an abort or bdr is not already in progress */
    if (((dev_ptr->cmd_activity_state != ABORT_IN_PROGRESS) &&
         (dev_ptr->cmd_activity_state != BDR_IN_PROGRESS)) && (issue_abrt_bdr))
    {
        if ((bp != NULL) &&
            (!(bp->bufstruct.b_flags & B_ERROR)))
        {
            /* set scsi stat in sc_buf and bytes   */
            /* based on the error that got us here */
            bp->status_validity = SC_ADAPTER_ERROR;
            bp->general_card_status = gen_err;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = EIO;
            if (save_interrupt != SCSI_SEL_TIMEOUT)
                p8xx_logerr(ERRID_SCSI_ERR10, log_err,
                           0xBE, 0, ap, com_ptr, TRUE);
        }

        /* start the abort SCRIPT */
	if ((save_interrupt != SCSI_SEL_TIMEOUT) &&
	    (save_interrupt != SCSI_UNEXP_DISC)) {
	    /* read ISTAT to see if we are connected to the scsi bus */
   	    istat_val = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
            p8xx_issue_abort_script(com_ptr, istat_val & ISTAT_CONNECTED);
	} else
            p8xx_issue_abort_script(com_ptr, FALSE);

        P8printf(P8_ABORT,("%s at %s:%d for ID %d\n",
            "ABORT set",__FILE__,__LINE__,dev_ptr->scsi_id));
        dev_ptr->cmd_activity_state = ABORT_IN_PROGRESS;
        dev_ptr->flags |= SCSI_ABORT;
        p8xx_start_abort_timer(dev_ptr, TRUE);
        return (FALSE);
    }
    else
    {
        /* if we issued a bdr due to a command timeout and got */
        /* a scsi interrupt, or if the interrupt we got would */
	/* normally cause us to issue an abort, and we were already */
        /* trying to issue an abort, then reset the scsi bus. */
        TRACE_1("not abort", 0)
        if ((dev_ptr->flags & CAUSED_TIMEOUT) ||
	   ((dev_ptr->cmd_activity_state == ABORT_IN_PROGRESS) 
	     && issue_abrt_bdr)) {
            p8xx_command_reset_scsi_bus(ap, 7);       /* reset scsi bus */
            return (FALSE);
        }

	/* if we issued a bdr ioctl and any of the luns have active */
	/* commands besides the bdr ioctl, reset the bus and exit.  */
        /* Otherwise, mark the other luns to indicate a bdr is no   */
	/* longer pending, and set their queue state and flags.     */
	if (dev_ptr->cmd_activity_state == BDR_IN_PROGRESS) {
	    dev_index = dev_ptr->scsi_id << 3;
            for (i=0; i < MAX_LUNS; i++) {
                if ((tmp_dev_ptr = ap->device_queue_hash[dev_index + i]) 
							!= NULL) {
		    if (((tmp_dev_ptr != dev_ptr) &&
			 (tmp_dev_ptr->active_head != NULL))  ||
			((tmp_dev_ptr == dev_ptr) && (dev_ptr->active_head !=
			 dev_ptr->active_tail))) {
                        p8xx_command_reset_scsi_bus(ap, 8);
                        return (FALSE);
		    } else if (tmp_dev_ptr != dev_ptr) {
		        /* fail cmd handles dev_ptr below, after additional */
			/* processing */
			p8xx_fail_cmd(tmp_dev_ptr);
		    }
		}
	    } /* end for loop */
        }

        if (bp != NULL) {
            if (!(bp->bufstruct.b_flags & B_ERROR)) {
                /* set scsi stat in sc_buf and bytes   */
                /* based on the error that got us here, handling */
		/* unexpected disconnect during wide negotiation */
		/* specially. */

                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                bp->bufstruct.b_flags |= B_ERROR;

		if ((com_ptr->flags & NEGOT_CMD) &&
		    (save_interrupt == SCSI_UNEXP_DISC)) {
		    dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR; 
		    /* turn off the bit in scntl3, it is possibly on */
	            dev_ptr->sid_ptr->io_table.scntl3  &= ~SCNTL3_EWS;
		    dev_ptr->sid_ptr->negotiate_flag = TRUE;
                    bp->bufstruct.b_error = ENXIO;
                    bp->status_validity = 0;
	            bp->general_card_status = SC_SCSI_BUS_FAULT;
		} else {
                    bp->bufstruct.b_error = EIO;
                    bp->status_validity = SC_ADAPTER_ERROR;
                    bp->general_card_status = gen_err;
		}

                /* For the case of a scsi selction failure, or
                 * an unexpected disconnect (bus free) occuring
                 * ing during negotiation, no log entry should be
	         * made. An error, however, should be returned to
		 * the calling process.
		 */
                if ((save_interrupt != SCSI_SEL_TIMEOUT) &&
                    ((save_interrupt != SCSI_UNEXP_DISC) ||
		       (!(com_ptr->flags & NEGOT_CMD)))  &&
                    ((save_interrupt != SCSI_UNEXP_DISC) ||
                       (dev_ptr->cmd_activity_state !=
                        NEGOTIATE_IN_PROGRESS)))
                {
                    p8xx_logerr(ERRID_SCSI_ERR10, log_err,
                               0xC3, 0, ap, com_ptr, TRUE);
                }
            }
            /* status already set and error logged */
            p8xx_free_resources(com_ptr);
	    if (!ap->dump_started) {
	        TRACE_1("iodone bp", (int) bp)
                iodone((struct buf *) bp);
	    }
        }
        else if (save_interrupt != SCSI_SEL_TIMEOUT) {
	     /* bp should be null only for abort/bdr ioctls */
            p8xx_logerr(ERRID_SCSI_ERR10, log_err, 0xC8, 0, ap, 
		com_ptr, TRUE);
        }

        dev_ptr->flags = RETRY_ERROR;
        if ((ap->ABORT_BDR_head == dev_ptr) ||
            (dev_ptr->ABORT_BDR_bkwd != NULL))
            /* remove any abort or bdr to queued */
            p8xx_deq_abort_bdr(dev_ptr);

        /* remove device from active queue */
        p8xx_deq_active(dev_ptr, com_ptr);
        p8xx_fail_cmd(dev_ptr);

        if ((dev_ptr->ioctl_wakeup == TRUE) && (!ap->dump_started)) {
            if (save_interrupt != SCSI_SEL_TIMEOUT)
                dev_ptr->ioctl_errno = PSC_NO_ERR;
            else
                dev_ptr->ioctl_errno = EIO;
            dev_ptr->ioctl_wakeup = FALSE;
            e_wakeup(&dev_ptr->ioctl_event);
        }

    }
    TRACE_1 ("out sint", 0)
    return (TRUE);
}

/************************************************************************
 *								
 * NAME:	p8xx_sel_glitch
 *						
 * FUNCTION:    Handles problem NCR chip has when target devices or a noisy
 *		bus cause a glitch on the Select line.  Determine the 
 *		appropriate point to restart the microcode.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is on.
 *						
 * DATA STRUCTURES:			
 *	None
 *					
 * INPUTS:			
 *      ap	   - pointer to adapter information structure
 *      com_ptr	   - pointer to command information structure
 *	save_isr   - holds the results of a prior pio to ISTAT
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_sel_glitch(ads_t *ap, cmd_t *com_ptr, int save_isr)
{
    int dsp_val;
    dvi_t *dev_ptr = com_ptr->device;

    dsp_val = p8xx_read_reg(ap, DSP, DSP_SIZE);
    TRACE_1("chip glitch", dsp_val)
    P8if(P8_UNEXP_BF,(brkpoint()));

    if (((dsp_val >= ap->Scripts.bscript_ptr) &&
         (dsp_val <= ap->Scripts.bscript_ptr + Ent_lun_msg_buf)) || 
	 (dev_ptr == NULL)) {
        /* Restart chip at the "wait reselect" instruction 
         * if dsp is in the iowait script or null dev_ptr 
	 */
	TRACE_1("iowait", (int) dsp_val)
        p8xx_start_chip(ap, NULL, ap->Scripts.bscript_ptr,
                            Ent_iowait_entry_point, 0, 0);
    } else { 
	/*
         * need to finish the last command, unless
         * a dma interrupt (probably a SIR) is
         * waiting
         */
        if (!(save_isr & ISTAT_DIP)) {
            uchar opcode;
            uchar * sptr;
            int restart_point;
            uint offset = dsp_val & 0x00000fff;

            /* If we are processing a selection attempt that lost
	     * out to the reselection, just continue.  This is indicated 
	     * if we are executing a select instruction, or if we are
	     * in the process of recovering from losing arbitration.
	     *
	     * If we are processing a Wait Disconnect instruction, we
	     * know the disconnect has completed, so we can just continue.
	     *
	     * If we are in the portion of the abort/bdr script that waits
	     * for the target to go bus free, we know this has occurred and
	     * can just restart the script to interrupt the abort/bdr completed.
	     *
	     * Otherwise we are somewhere in the main script, still doing the
	     * various processing that might occur after a legal disconnect.
	     * We cannot reliably know where to restart the SCRIPT since
	     * DSP is not reliable, so we have to give up on the command
	     * that just finished.  The script is restarted to process the
	     * reselection, the abort will go out after the reselection is
	     * serviced.
             */

            /*
             * Determine the opcode associated with the current
             * dsp value.  Remember that the script is byte-
             * swapped, so the opcode is 3 bytes from the
             * start of the instruction.
             */
            sptr = (uchar *) dev_ptr->sid_ptr->vscript_ptr;
            opcode = sptr[offset + 3];
	    TRACE_2("finish", (int) opcode, (int) dsp_val)

	    /* Now decide which case we have, and where to restart */
            if (opcode == 0x41) /* select instruction */
		restart_point = offset; 
            else if ((offset >= Ent_failed_selection_hdlr) &&
	        (offset <= Ent_end_failed_sel_hdlr)) 
		restart_point = Ent_failed_selection_hdlr;
	    else if ((offset >= Ent_failed_negotiation_selection_hdlr) &&
		     (offset <= Ent_end_failed_neg_sel_hdlr))
		restart_point = Ent_failed_negotiation_selection_hdlr;
	    else if ((offset >= Ent_failed_abort_bdr_selection_hdlr) &&
		     (offset <= Ent_end_failed_abdr_sel_hdlr))
		restart_point = Ent_failed_abort_bdr_selection_hdlr;
            else if (opcode == 0x48) /* wait disconnect instruction */
		restart_point = offset + 8;
	    else if ((offset >= Ent_wait_for_bus_free) &&
		     (offset <= Ent_end_wait_for_bus_free))
		restart_point = Ent_end_wait_for_bus_free - 8;
	    else {
		/* we are still executing instructions related to the
		 * command that disconnected prior to the glitched
		 * reselection.  Since we cannot rely on DSP to give
		 * an accurate indication where to restart the chip,
		 * queue up an abort (the bp will be marked after the
	         * abort), and process the reselection.
		 */

                dev_ptr->flags |= SCSI_ABORT;
                p8xx_enq_abort_bdr(dev_ptr);
		restart_point = -1;
	    }

 	    if (restart_point != -1)
                p8xx_start_chip(ap, NULL, dev_ptr->sid_ptr->bscript_ptr,
                            restart_point, 0, 0);
	    else
                p8xx_start_chip(ap, NULL, ap->Scripts.bscript_ptr,
                            Ent_iowait_entry_point, 0, 0);

        } else {
            /* dip pending too */
	    TRACE_1("dip pending", (int) dsp_val);
        }
    } /* end else not on wait reselect */
}

/************************************************************************/
/*                                                                      */
/* NAME:        p8xx_patch_tag_changes                                  */
/*                                                                      */
/* FUNCTION:    Patches Scripts to reflect changes to the tag 		*/
/*     	This function is called when it is determined that the tag has  */
/*	changed since the script was last patched.  This may also imply */
/* 	that the lun is different than what is patched.                 */
/*	Four items are handled:						*/
/*	1. the lun is patched into identify messages if it has changed  */
/*	2. if the lun changed, patch the jump to the reconnect point    */
/*	3. the identify messages are patched to handle a q_tag message	*/
/*	   and/or a new tag.  Note the whole message is patched, we 	*/
/*	   could get smarter and decide whether we just need to 	*/
/*	   change the tag itself.					*/
/*	4. the tag is patched into register move instructions		*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      com_ptr - pointer to the command information structure for      */
/*              the particular script that needs to be altered.         */
/*      q_tag_msg - the queue tag message from the sc_buf               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p8xx_patch_tag_changes(struct cmd_info * com_ptr, uchar q_tag_msg)
{
    ulong  *script_ptr;
    ulong   id_bit_mask;
    uchar lun, sdtr_patch;
    int     i, offset_to_jump;
    struct dev_info * dev_ptr;
    ads_t *ap;
    
    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;
    script_ptr = (ulong *) dev_ptr->sid_ptr->vscript_ptr;
    TRACE_3 ("ptag", (int) com_ptr->tag, (int) script_ptr, (int) dev_ptr)

    /* record the new tag being patched */
    dev_ptr->sid_ptr->tag_flag = com_ptr->tag;
    dev_ptr->sid_ptr->tag_msg_flag = q_tag_msg;

    ASSERT(q_tag_msg == SC_NO_Q || q_tag_msg == SC_SIMPLE_Q || 
	   q_tag_msg == SC_HEAD_OF_Q || q_tag_msg == SC_ORDERED_Q);

    TRACE_2 ("new lun", dev_ptr->lun_id, (int) script_ptr);

    /* record the new lun being patched */
    dev_ptr->sid_ptr->lun_flag = dev_ptr->lun_id;
    lun = com_ptr->bp->scsi_command.scsi_cmd.lun >> 5;

    /* patch the lun in identify messages */
    id_bit_mask = (lun << 24);
    script_ptr[(Ent_identify_msg_buf / 4)] &= 0xF0000000;
    script_ptr[(Ent_neg_msg_buf / 4)] &= 0xF0000000;
    script_ptr[(Ent_identify_msg_buf / 4)] |= id_bit_mask;
    script_ptr[(Ent_neg_msg_buf / 4)] |= id_bit_mask;

    /* 
     * If any negotiation is on-going, determine the kind
     */
    if (com_ptr->flags & NEGOT_CMD) {
        if (dev_ptr->cmd_activity_state == SDTR_IN_PROGRESS)
	    sdtr_patch = TRUE;
        else
	    sdtr_patch = FALSE;
        TRACE_1("sdtrpatch", (int) sdtr_patch)
    }

    /* now complete the patching of the buffers used in MSG-OUT */
    if (q_tag_msg != SC_NO_Q) {
 	id_bit_mask = 0;
        id_bit_mask |= (((ulong) (q_tag_msg + 0x1F) << 16) |
		       ((ulong) (com_ptr->tag) << 8)); 
        script_ptr[(Ent_identify_msg_buf / 4)] |= id_bit_mask;
	/* patch number of bytes to move in message out - word already */
	/* reversed. */
        script_ptr[(Ent_message_loop / 4)] = 0x0300000E;	

	/* do the patches needed for negotiation */
	if (com_ptr->flags & NEGOT_CMD) {
	    id_bit_mask |= 0x00000001;
            script_ptr[(Ent_neg_msg_buf / 4)] |= id_bit_mask;
	    if (sdtr_patch) {
                script_ptr[S_INDEX(Ent_neg_msg_buf)+1] = (SCSI_SDTR_MSG << 8) | 
		    (((ulong) ap->xfer_pd) << 8) | (ap->req_ack);
                script_ptr[S_INDEX(Ent_start_negotiation_msg_out_loop)] =  
			0x0800000E;
	    } else {
                script_ptr[S_INDEX(Ent_neg_msg_buf)+1] = (SCSI_WDTR_MSG << 8); 
                script_ptr[S_INDEX(Ent_start_negotiation_msg_out_loop)] =  
			0x0700000E;
	    }
	}

    } else /* no q tag */ {
	/*
	 * The identify_msg_buf was already cleared out above, and the identify
	 * byte has been patched.  Now patch the move instruction.
	 */ 
        script_ptr[S_INDEX(Ent_message_loop)] = 0x0100000E;

        /* Do the patches needed for the negotiation message.  */
	if (com_ptr->flags & NEGOT_CMD) {
	    if (sdtr_patch) {
                script_ptr[S_INDEX(Ent_neg_msg_buf)] |= (SCSI_SDTR_MSG >> 8); 
                script_ptr[S_INDEX(Ent_neg_msg_buf)+1] = 
		    (((ulong) ap->xfer_pd) << 24) | (((ulong) ap->req_ack) << 16);
                script_ptr[S_INDEX(Ent_start_negotiation_msg_out_loop)] =  
			0x0600000E;
                TRACE_1("neg buf", script_ptr[S_INDEX(Ent_neg_msg_buf)])
	    } else {
                script_ptr[S_INDEX(Ent_neg_msg_buf)] |= (SCSI_WDTR_MSG >> 8); 
                script_ptr[S_INDEX(Ent_neg_msg_buf)+1] = (SCSI_WDTR_MSG << 24); 
                script_ptr[S_INDEX(Ent_start_negotiation_msg_out_loop)] =  
			0x0500000E;
                TRACE_1("neg buf", script_ptr[S_INDEX(Ent_neg_msg_buf)])
	    }
	}
    } /* end not q-tag */

    /* Finally, patch the tag into the register move instructions */
    for (i = 0; i < S_COUNT(R_tag_patch_Used); i++) {
        script_ptr[R_tag_patch_Used[i]] &= 0xff00ffff;
        script_ptr[R_tag_patch_Used[i]] |= (((ulong) com_ptr->tag) << 16);
    }
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_prep_main_script					*/
/*									*/
/* FUNCTION:	Adapter Script Command Loading Routine			*/
/*	This will patch in the SCSI command bytes, found by looking	*/
/*	at the sc_buf to be executed.  Then, it will patch in the	*/
/*	number of bytes to be transfered (if any).  In addition, it	*/
/*	will patch the jump within the IO_WAIT script to jump to	*/
/*	this particular script when the target reselects the chip.	*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	dev_info structure						*/
/*	sc_buf structure						*/
/*									*/
/* INPUTS:								*/
/*	*iowait_vir_addr - the virtual address pointing to the		*/
/*		IO_WAIT script that all the scripts will be dependent	*/
/*		on as a router.						*/
/*	*script_vir_addr - the virtual address of the script just	*/
/*		created and copied into memory.	 The script that needs	*/
/*		to be loaded with the actual SCSI command to be sent to */
/*		the device.  This is the pointer to the TOP		*/
/*		of the script, not to the "main" portion of the script. */
/*	*cmd_ptr - pointer to the cmd_info structure which holds	*/
/*		the sc_buf which contains the SCSI command given to us	*/
/*		by the driver head.					*/
/*	script_bus_addr - bus address of the script to be patched       */
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/*									*/
/************************************************************************/
void
p8xx_prep_main_script(
	ulong	*iowait_vir_addr,
	ulong	*script_vir_addr,
	struct cmd_info	*com_ptr,
	uint	script_bus_addr)
{

    ulong	*target_script, *io_wait_ptr;
    char	*script_byte_ptr;
    uchar	scsi_command, lun;
    ulong	cmd_byte_length, word_buffer;
    int		i, offset_to_jump;
    struct sc_buf *bp;
#ifdef P8_TRACE
    ads_t       *ap = com_ptr->device->ap;
#endif

    TRACE_2("prepmain", (int) iowait_vir_addr, (int) script_vir_addr)
    P8printf(P8_ENTRY,("Entering p8xx_prep_main_script\n"));
    P8printf(P8_INFO_2,(" iowait_vir_addr=0x%x, script_vir_addr=0x%x\n",
	iowait_vir_addr, script_vir_addr));

    /*
     * Entering p8xx_prep_main_script
     */
    script_byte_ptr = (char *) script_vir_addr;
    target_script = (ulong *) script_vir_addr;
    io_wait_ptr = (ulong *) iowait_vir_addr;
    bp = com_ptr->bp;

    /*
     * Load the byte length for the SCSI cmds.
     */
    cmd_byte_length = (ulong) bp->scsi_command.scsi_length;
    word_buffer = CBOtoL(target_script[R_cmd_bytes_out_count_Used[0]]);
    word_buffer = ((word_buffer & 0xFF000000) | (cmd_byte_length));
    target_script[R_cmd_bytes_out_count_Used[0]] = LtoCBO(word_buffer);

    /*
     * Load the SCSI command into the command buffer
     */
    scsi_command = bp->scsi_command.scsi_cmd.scsi_op_code;
    script_byte_ptr[Ent_cmd_buf] = scsi_command;
    script_byte_ptr[Ent_cmd_buf + 1] = bp->scsi_command.scsi_cmd.lun;
    for (i = 2; i < cmd_byte_length; i++)
	script_byte_ptr[Ent_cmd_buf + i] =
	    bp->scsi_command.scsi_cmd.scsi_bytes[i - 2];

    P8printf(P8_INFO_2,
	("SCSI COMMAND ^^^^^^^^^^^^^^^^^^^^^^^^^^^ SCSI COMMAND\n"));
    P8printf(P8_INFO_2,("op_code is       0x%x  for ID  0x%x\n",
	bp->scsi_command.scsi_cmd.scsi_op_code,
	bp->scsi_command.scsi_id));
    P8printf(P8_INFO_2,
	("SCSI COMMAND ^^^^^^^^^^^^^^^^^^^^^^^^^^^ SCSI COMMAND\n"));


    /*
     * Almost every define and address label in the SCRIPTS is in terms
     * of word offsets except those used in JUMP commands.  These are in
     * terms of bytes.	All offsets are relative to the beginning address
     * of the scripts, where the IO_WAIT script also resides.
     */
    lun = bp->scsi_command.scsi_cmd.lun >> 5;
    switch (bp->scsi_command.scsi_id) {
	case 0:
	    offset_to_jump = E_scsi_0_lun_Used[lun];
	    break;
	case 1:
	    offset_to_jump = E_scsi_1_lun_Used[lun];
	    break;
	case 2:
	    offset_to_jump = E_scsi_2_lun_Used[lun];
	    break;
	case 3:
	    offset_to_jump = E_scsi_3_lun_Used[lun];
	    break;
	case 4:
	    offset_to_jump = E_scsi_4_lun_Used[lun];
	    break;
	case 5:
	    offset_to_jump = E_scsi_5_lun_Used[lun];
	    break;
	case 6:
	    offset_to_jump = E_scsi_6_lun_Used[lun];
	    break;
	case 7:
	    offset_to_jump = E_scsi_7_lun_Used[lun];
	    break;
	case 8:
	    offset_to_jump = E_scsi_8_lun_Used[lun];
	    break;
	case 9:
	    offset_to_jump = E_scsi_9_lun_Used[lun];
	    break;
	case 10:
	    offset_to_jump = E_scsi_A_lun_Used[lun];
	    break;
	case 11:
	    offset_to_jump = E_scsi_B_lun_Used[lun];
	    break;
	case 12:
	    offset_to_jump = E_scsi_C_lun_Used[lun];
	    break;
	case 13:
	    offset_to_jump = E_scsi_D_lun_Used[lun];
	    break;
	case 14:
	    offset_to_jump = E_scsi_E_lun_Used[lun];
	    break;
	case 15:
	    offset_to_jump = E_scsi_F_lun_Used[lun];
	    break;
	default:
	    break;
    }

    io_wait_ptr[offset_to_jump] =
	LtoCBO((uint)script_bus_addr + Ent_script_reconnect_point);

    /* 
     * Init byte count variables and mark prep_main_script complete.
     */
    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
    com_ptr->flags |= PREP_MAIN_COMPLETE;

    P8if(P8_DUMP_SCRIPTS_2,(p8xx_dump_script(script_vir_addr,
	PAGESIZE)));
    P8printf(P8_EXIT,("Leaving p8xx_prep_main_script\n"));

}


/************************************************************************/
/*									*/
/* NAME:	p8xx_set_disconnect					*/
/*									*/
/* FUNCTION:	Adapter Script Initialization Routine			*/
/*	This function goes into the main script and patches the		*/
/*	identify message to have the disconnect bit either off or on	*/
/*	depending on the flags value from the command of SC_NODISC.	*/
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
/*	dev_ptr - pointer to the device information structure for	*/
/*		the particular script that needs to be altered.		*/
/*									*/
/*	chk_disconnect - flag used to determine if the device wants no	*/
/*		disconnect to take place.				*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/*									*/
/************************************************************************/
void
p8xx_set_disconnect(
	dvi_t	*dev_ptr,
	uchar	chk_disconnect)
{
    ulong  *vscript_ptr;
    ulong   id_bit_mask;

    vscript_ptr =  dev_ptr->sid_ptr->vscript_ptr;

    /* store an indicator of the patched value: */
    /*     0x80 means no disconnect privilege, */
    /*     0x00 means disconnect privilege      */
    dev_ptr->sid_ptr->disconnect_flag = chk_disconnect;

    if (chk_disconnect & SC_NODISC)
        id_bit_mask = 0x80000000;
    else
        id_bit_mask = 0xC0000000;

    id_bit_mask |= (((ulong) dev_ptr->lun_id) << 24);
    vscript_ptr[S_INDEX(Ent_identify_msg_buf)]   = id_bit_mask;
    vscript_ptr[S_INDEX(Ent_neg_msg_buf)] = id_bit_mask;

    return;
}


/************************************************************************/
/*									*/
/* NAME:	p8xx_wdtr_answer					*/
/*									*/
/* FUNCTION:	Process a target's response to a WDTR message    	*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	Called with interrupts disabled, chip is assumed on.		*/
/*									*/
/* DATA STRUCTURES:							*/
/*	None.								*/
/*									*/
/* INPUTS:								*/
/*	*script_vir_addr - the virtual address of the script that needs */
/*		to be altered.						*/
/*	*com_ptr - pointer to the command information structure		*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None                                                            */
/*									*/
/************************************************************************/
void
p8xx_wdtr_answer(
	uint	*script_vir_addr,
	cmd_t	*com_ptr)
{
    dvi_t       *dev_ptr;
    ads_t	*ap; 
    uchar	xfer_width;

    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;

    TRACE_1("wdtr done", (int) dev_ptr)
    /* Patch the script so it will continue with sdtr negotiation: 
     * Adjust the neg_msg move so it doesn't include i-t-l-q nexus
     * information.  Then set the following instruction to jump to the sdtr
     * portion of the negotiation script.
     */
    script_vir_addr[S_INDEX(Ent_neg_msg_buf)] = SCSI_SDTR_MSG | 
			    (ulong) ap->xfer_pd;
    script_vir_addr[S_INDEX(Ent_neg_msg_buf)+1] = ((ulong) ap->req_ack) << 24;
    script_vir_addr[S_INDEX(Ent_start_negotiation_msg_out_loop)] = 
			LtoCBO(P8XX_MOVE5_PATCH);
    script_vir_addr[S_INDEX(Ent_negotiation_phase_patch)] = 
			LtoCBO(P8XX_JUMP_PATCH);

    /* Get transfer width */
    xfer_width = (uchar) (script_vir_addr[S_INDEX(Ent_extended_msg_buf)] >> 8);
    TRACE_2("wdtr", (int) dev_ptr, (int) xfer_width)

    if (xfer_width == 0x01) {
	/* 16 bit transfers */
	dev_ptr->sid_ptr->io_table.scntl3 |= SCNTL3_EWS;
    }
    else if (xfer_width == 0x00) {
	/* 8 bit transfers */
	dev_ptr->sid_ptr->io_table.scntl3 &= ~SCNTL3_EWS;
    }
    else {
	/* xfer width indicates 32 bit transfers or a reserved value.  The
	 * device has violated scsi protocol.  Chip should be set for 8 bit 
	 * data transfers.  We should also avoid sending future WDTRs to this
	 * device.  The chip will reject this WDTR message and move to SDTR
  	 * negotiation.
 	 */
	dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR; 
	dev_ptr->sid_ptr->io_table.scntl3 &= ~SCNTL3_EWS;
        script_vir_addr[S_INDEX(Ent_neg_msg_buf)] = 0x07000000 | 
		((ulong) SCSI_SDTR_MSG >> 8); 
        script_vir_addr[S_INDEX(Ent_neg_msg_buf)+1] = 
	        (((ulong) ap->xfer_pd) << 24) | (((ulong) ap->req_ack) << 16);
        script_vir_addr[S_INDEX(Ent_start_negotiation_msg_out_loop)] = 
			LtoCBO(P8XX_MOVE6_PATCH);
    }	

    /* activate or deactivate special microcode only needed for wide scsi */
    p8xx_wide_patch(dev_ptr, FALSE);

    RESTART_CHIP(dev_ptr, com_ptr);
}

/************************************************************************
 *								
 * NAME:	p8xx_wdtr_reject
 *						
 * FUNCTION:   This function is called to process a planned Script 
 *	interrupt that indicates the target rejected our WDTR with a
 *	Message Reject message.  The script is patched for narrow
 *	transfers and preparations for the upcoming SDTR message are
 *	made.  Then the chip is restarted.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * DATA STRUCTURES:			
 *	target script (dev_ptr->sid_ptr->vscript_ptr) is patched
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target structure
 *      com_ptr	   - pointer to command structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_wdtr_reject(ads_t *ap,
	dvi_t *dev_ptr,
	cmd_t *com_ptr)
{
    uint *target_script;   /* used to patch script */

    TRACE_1("wdtr reject", (int) dev_ptr->sid_ptr)

    /*
     * Target rejected the WDTR (legally).  Continue w/
     * the synchronous data negotiation sequence.
     */
    dev_ptr->sid_ptr->io_table.scntl3 &= ~SCNTL3_EWS;

    /* patch out special microcode only needed for wide scsi */
    p8xx_wide_patch(dev_ptr, FALSE);

    target_script = (uint *) dev_ptr->sid_ptr->vscript_ptr;
    target_script[S_INDEX(Ent_neg_msg_buf)] = SCSI_SDTR_MSG |
			    (ulong) ap->xfer_pd;
    target_script[S_INDEX(Ent_neg_msg_buf)+1] = ((ulong) ap->req_ack) << 24;
    target_script[S_INDEX(Ent_start_negotiation_msg_out_loop)] =
			LtoCBO(P8XX_MOVE5_PATCH);
    target_script[S_INDEX(Ent_negotiation_phase_patch)] = 
			LtoCBO(P8XX_JUMP_PATCH);
    RESTART_CHIP(dev_ptr, com_ptr);
}

/************************************************************************
 *								
 * NAME:	p8xx_sdtr_reject
 *						
 * FUNCTION:   This function is called to process a planned Script 
 *	interrupt that indicates the target rejected our SDTR with a
 *	Message Reject message.  The script is patched for asynchronous
 *	transfers and other preparations for the upcoming command are
 *	made.  Then the chip is restarted at the next instruction.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * DATA STRUCTURES:			
 *	target script (dev_ptr->sid_ptr->vscript_ptr) is patched
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target structure
 *      com_ptr	   - pointer to command structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_sdtr_reject(ads_t *ap,
		dvi_t *dev_ptr,
		cmd_t *com_ptr)
{
    TRACE_1("sdtr reject", (int) dev_ptr->sid_ptr)

    /*
     * We tried to negotiate for synch mode, but failed.
     * ASYNC mode patched into the script.
     */
    P8printf(P8_SYNC, ("3 resetting negotiate_flag for ID/LUN %d/%d\n",
			dev_ptr->scsi_id,dev_ptr->lun_id));
    dev_ptr->sid_ptr->negotiate_flag = FALSE;
    dev_ptr->sid_ptr->async_device = TRUE;
    p8xx_prep_main_script(ap->Scripts.vscript_ptr,
		dev_ptr->sid_ptr->vscript_ptr,
		com_ptr, dev_ptr->sid_ptr->bscript_ptr);
    dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;

    P8printf(P8_INFO_1,("p8xx_intr: NEG_FAILED \n"));

    /* Patch the SCRIPT for Async.  */
    p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);

    /* and restart script */
    RESTART_CHIP(dev_ptr, com_ptr);

}

/************************************************************************
 *								
 * NAME:	p8xx_sdtr_neg_done
 *						
 * FUNCTION:   This function is called to process a planned Script 
 *	interrupt that indicates that the target sent its extended message
 *	in reply to our SDTR (synchronous data transfer) message.  The
 *	script is prepared for the Command and Data phases, and restarted
 *	according to the contents of the target's reply message.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * NOTES: p8xx_sdtr_answer is called to actually interpret the reply message
 *
 * DATA STRUCTURES:			
 *	ap->Scripts.vscript_ptr (pointer to iowait script)
 *	dev_ptr->sid_ptr->vscript_ptr (pointer to target script)
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target structure
 *      com_ptr	   - pointer to command structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_sdtr_neg_done(ads_t *ap,
		dvi_t *dev_ptr,
		cmd_t *com_ptr)
{

    TRACE_1 ("sdtr done", (int) dev_ptr)

    /* sync complete and successful */
    P8printf(P8_SYNC, ("2 resetting negotiate_flag for ID %d\n",
			 dev_ptr->scsi_id));
    dev_ptr->sid_ptr->negotiate_flag = FALSE;
    dev_ptr->sid_ptr->async_device = FALSE;

    P8printf(P8_INFO_1,("p8xx_intr: SYNC_NEG_DONE \n"));

    /* supports synch negotiation */
    if (p8xx_sdtr_answer((uint *)dev_ptr->sid_ptr->vscript_ptr, dev_ptr) 
		== PSC_NO_ERR) {

	/*
	 * Setup script to issue the command already
	 * tagged to this device queue
	 */
	p8xx_prep_main_script(ap->Scripts.vscript_ptr,
	            dev_ptr->sid_ptr->vscript_ptr,
	            com_ptr, dev_ptr->sid_ptr->bscript_ptr);
	dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	RESTART_CHIP(dev_ptr, com_ptr);

    } else {			/* verify_neg_answer rc */
	/*
	 * Is non-zero, we must issue a msg reject.
	 * First, prep the script.
	 */
	p8xx_prep_main_script(ap->Scripts.vscript_ptr,
	        dev_ptr->sid_ptr->vscript_ptr,
	        com_ptr, dev_ptr->sid_ptr->bscript_ptr);
	dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	p8xx_start_chip(ap, com_ptr, dev_ptr->sid_ptr->bscript_ptr,
			    Ent_reject_target_sync, 0, 0);
    }
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_sdtr_answer					*/
/*									*/
/* FUNCTION:	Process a target's response to our SDTR message		*/
/*	 If negotiation has ended normally (either the target	        */
/*	has requested asynchronous transfers, or else the target has	*/
/*	requested a transfer period we can support), this function	*/
/*	returns zero.  A non-zero return from this function indicates	*/
/*	that the caller must reject the target's SDTR message, and that */
/*	the script has been patched for asynchronous transfers.		*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine is called with interrupts disabled.		*/
/*									*/
/* DATA STRUCTURES:							*/
/*	None.								*/
/*									*/
/* INPUTS:								*/
/*	*script_vir_addr - the virtual address of the script that needs */
/*		to be altered.					     	*/
/*	*dev_ptr - pointer to the target device information structure	*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	Non-zero means start script at message reject                   */
/*									*/
/************************************************************************/
int
p8xx_sdtr_answer(
	uint	*script_vir_addr,
	dvi_t	*dev_ptr)
{
    ads_t	*ap = dev_ptr->ap;
    uchar	xfer_pd, best_pd,
		req_ack;
    int		rc = PSC_NO_ERR;
    uchar	scntl3_val, sxfer_val;

    /* 
     * Entering p8xx_sdtr_answer
     */

    /* Get transfer period and REQ/ACK offset that the device wants */
    xfer_pd = (uchar) (script_vir_addr[S_INDEX(Ent_extended_msg_buf)] >> 8);
    req_ack = (uchar) (script_vir_addr[S_INDEX(Ent_extended_msg_buf)]);

    TRACE_2 ("v negot ", (int) xfer_pd, (int) req_ack)

    if ((xfer_pd != ap->xfer_pd) || (req_ack != ap->req_ack)) {

	if (req_ack == 0) {

	    /* 
	     * If req_ack is zero, we must treat this like an
	     * asynchronous device.
	     */
	    dev_ptr->sid_ptr->async_device = TRUE;

	    /* 
	     * Patch the SCRIPT for async transfers.
	     */
	    P8printf(P8_SYNC, 
                ("sdtr_answer: ID %d set to async (reqack 0)\n", 
		dev_ptr->scsi_id));
	    p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);

	} else {


	    if ((best_pd = p8xx_select_period(ap, xfer_pd,
		&scntl3_val, &sxfer_val))) {

		/* add in the req_ack offset and transfer width */
	        scntl3_val |= dev_ptr->sid_ptr->io_table.scntl3 & SCNTL3_EWS;
		sxfer_val |= req_ack;		/* include req/ack */

		P8printf(P8_SYNC,
		    ("verify_neg_answer: ID %d/%d - tgt_pd %dns, our_pd %dns\n",
		    dev_ptr->scsi_id, dev_ptr->lun_id,
		    xfer_pd * 4, best_pd * 4));

		p8xx_do_patch(dev_ptr, sxfer_val, scntl3_val);

	    } else {

		/*
		 * Target's requested sync transfer period is slower than we
		 * can set the chip.  Patch the script for async and inform
 		 * the caller.  The caller must then make sure the target
		 * finds out that the negotiation has ended without a sync
		 * agreement.  This can be done either by initiating
		 * another exchange of SDTR messages, this time using a zero
		 * req/ack offset to force the exchange to be async, or by
		 * issuing a Message Reject to the just-received SDTR response.
		 */

		dev_ptr->sid_ptr->async_device = TRUE;

		/* Patch the SCRIPTS for async transfers. */
		P8printf(P8_SYNC,
		    ("verify_neg_answer: ID %d/%d set to async (chip can't run slow enough [%dns])\n",
		    dev_ptr->scsi_id, dev_ptr->lun_id, xfer_pd * 4));
		p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);

		rc = (int) PSC_FAILED;
	    }
	}

    } else {			/* target accepts our default values */

	/* Patch in values in SCRIPTS for our default sync mode */
	P8printf(P8_SYNC,
	    ("verify_neg_answer: ID %d/%d accepts our default period [%dns]\n",
	    dev_ptr->scsi_id, dev_ptr->lun_id, xfer_pd * 4));
	p8xx_mode_patch(ap, SYNC_MODE, dev_ptr);

    }

    /* Leaving p8xx_sdtr_answer */

    return (rc);

}


/************************************************************************/
/*									*/
/* NAME:	p8xx_reset_iowait_jump					*/
/*									*/
/* FUNCTION:	Adapter Script Jump Reset Routine			*/
/*	This function does a patch of the jump instruction within the	*/
/*	IO_WAIT script.	 It resets the jump back to the original default*/
/*	jump that existed before it was initialized to run a script.	*/
/*	This function should be called before issuing an abort or bdr	*/
/*	Script.	 This function acts as a safety net against an illegal  */
/*	or spurious interrupt by a target device that is supposed to be */
/*	inactive.							*/
/*									*/
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
/*	*iowait_p - pointer to scripts structure which contains both	*/
/*		the virtual and bus addresses of the io_wait scripts.	*/
/*		that all the scripts will be dependenton as a router.	*/
/*	*dev_ptr  - pointer to the device information structure		*/
/*	all_luns  - flag indicating whether jumps for each lun should   */
/*		    be reset						*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/*									*/
/************************************************************************/
void
p8xx_reset_iowait_jump(
	scripts_t	*iowait_p,
	dvi_t		*dev_ptr,
	uchar		all_luns)
{
    int	    scsi_id, offset_to_jump, orig_byte_jump_offset;
    int     i;
    ulong  *io_wait_ptr;
    uint    io_wait_bus;
    uchar   lun;
#ifdef P8_TRACE
    ads_t  *ap = dev_ptr->ap;
#endif

    TRACE_1("clear jmp", (int) dev_ptr)
    P8printf(P8_ENTRY,
	("Entering reset_iowait_jump, iowait_p=0x%x, dev_ptr=0x%08x\n",
	iowait_p, dev_ptr));

    io_wait_ptr = iowait_p->vscript_ptr;
    io_wait_bus = iowait_p->bscript_ptr;

    /* Are we reseting the jump for all luns? */
    /* If so, we want the jump address for lun 0 */
    /* to use as our base */
    if (all_luns)
        lun = 0x00;
    else
        lun = dev_ptr->lun_id;

    scsi_id = dev_ptr->scsi_id;

    TRACE_2 ("id lun", (int) scsi_id, (int) lun)
    switch (scsi_id) {
	case 0:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[0] - 1;
	    offset_to_jump = E_scsi_0_lun_Used[lun];
	    break;
	case 1:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[1] - 1;
	    offset_to_jump = E_scsi_1_lun_Used[lun];
	    break;
	case 2:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[2] - 1;
	    offset_to_jump = E_scsi_2_lun_Used[lun];
	    break;
	case 3:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[3] - 1;
	    offset_to_jump = E_scsi_3_lun_Used[lun];
	    break;
	case 4:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[4] - 1;
	    offset_to_jump = E_scsi_4_lun_Used[lun];
	    break;
	case 5:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[5] - 1;
	    offset_to_jump = E_scsi_5_lun_Used[lun];
	    break;
	case 6:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[6] - 1;
	    offset_to_jump = E_scsi_6_lun_Used[lun];
	    break;
	case 7:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[7] - 1;
	    offset_to_jump = E_scsi_7_lun_Used[lun];
	    break;
	case 8:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[8] - 1;
	    offset_to_jump = E_scsi_8_lun_Used[lun];
	    break;
	case 9:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[9] - 1;
	    offset_to_jump = E_scsi_9_lun_Used[lun];
	    break;
	case 10:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[10] - 1;
	    offset_to_jump = E_scsi_A_lun_Used[lun];
	    break;
	case 11:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[11] - 1;
	    offset_to_jump = E_scsi_B_lun_Used[lun];
	    break;
	case 12:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[12] - 1;
	    offset_to_jump = E_scsi_C_lun_Used[lun];
	    break;
	case 13:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[13] - 1;
	    offset_to_jump = E_scsi_D_lun_Used[lun];
	    break;
	case 14:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[14] - 1;
	    offset_to_jump = E_scsi_E_lun_Used[lun];
	    break;
	case 15:
	    orig_byte_jump_offset = A_uninitialized_reselect_Used[15] - 1;
	    offset_to_jump = E_scsi_F_lun_Used[lun];
	    break;
	default:
	    break;

    }

    if (!all_luns)
        io_wait_ptr[offset_to_jump] =
            LtoCBO((orig_byte_jump_offset * 4) + (uint) io_wait_bus);
    else
    {
        /* Starting with lun 0, patch each jump address (every 2nd word) */
        for (i = 0; i < MAX_LUNS; i++)
        {
            io_wait_ptr[offset_to_jump + (2 * i)] =
            LtoCBO((orig_byte_jump_offset * 4) +
                                (uint) io_wait_bus);
        }
    }

    P8printf(P8_EXIT,("Leaving p8xx_reset_iowait_jump, offset_to_jump=0x%x\n",
	offset_to_jump));
    return;
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_restore_iowait_jump				*/
/*									*/
/* FUNCTION:	Adapter Script Command Loading Routine			*/
/*	This will patch the iowait script with the address required	*/
/*	for the particular device.  This routine is called after a	*/
/*	selection failure when trying to issue an abort or bdr to a	*/
/*	device.								*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	dev_info structure						*/
/*	sc_buf structure						*/
/*									*/
/* INPUTS:								*/
/*	*iowait_vir_addr - the virtual address pointing to the		*/
/*		IO_WAIT script that all the scripts will be dependent	*/
/*		on as a router.						*/
/*	script_bus_addr - the dma address of the script just		*/
/*		created and copied into memory.				*/
/*	*dev_ptr - pointer to the dev_info structure which holds	*/
/*		the sc_buf which contains the SCSI command given to us	*/
/*		by the driver head.					*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/*									*/
/************************************************************************/
void
p8xx_restore_iowait_jump(
	uint	*iowait_vir_addr,
	dvi_t	*dev_ptr,
	uint	script_bus_addr)
{
    ulong  *io_wait_ptr;
    int	    offset_to_jump;
    uchar   lun;
#ifdef P8_TRACE
    ads_t  *ap = dev_ptr->ap;
#endif

    TRACE_1("rest jmp", (int) dev_ptr)
    io_wait_ptr = (ulong *) iowait_vir_addr;

    lun = dev_ptr->lun_id;

    /*
     * Almost every define and address label in the SCRIPTS is in terms
     * of word offsets except those used in JUMP commands.  These are in
     * terms of bytes.	All offsets are relative to the beginning address
     * of the scripts, where the IO_WAIT script also resides.
     */

    switch (dev_ptr->scsi_id) {
	case 0:
	    offset_to_jump = E_scsi_0_lun_Used[lun];
	    break;
	case 1:
	    offset_to_jump = E_scsi_1_lun_Used[lun];
	    break;
	case 2:
	    offset_to_jump = E_scsi_2_lun_Used[lun];
	    break;
	case 3:
	    offset_to_jump = E_scsi_3_lun_Used[lun];
	    break;
	case 4:
	    offset_to_jump = E_scsi_4_lun_Used[lun];
	    break;
	case 5:
	    offset_to_jump = E_scsi_5_lun_Used[lun];
	    break;
	case 6:
	    offset_to_jump = E_scsi_6_lun_Used[lun];
	    break;
	case 7:
	    offset_to_jump = E_scsi_7_lun_Used[lun];
	    break;
	case 8:
	    offset_to_jump = E_scsi_8_lun_Used[lun];
	    break;
	case 9:
	    offset_to_jump = E_scsi_9_lun_Used[lun];
	    break;
	case 10:
	    offset_to_jump = E_scsi_A_lun_Used[lun];
	    break;
	case 11:
	    offset_to_jump = E_scsi_B_lun_Used[lun];
	    break;
	case 12:
	    offset_to_jump = E_scsi_C_lun_Used[lun];
	    break;
	case 13:
	    offset_to_jump = E_scsi_D_lun_Used[lun];
	    break;
	case 14:
	    offset_to_jump = E_scsi_E_lun_Used[lun];
	    break;
	case 15:
	    offset_to_jump = E_scsi_F_lun_Used[lun];
	    break;
	default:
	    break;
    }

    io_wait_ptr[offset_to_jump] = 
	LtoCBO((uint)script_bus_addr + Ent_script_reconnect_point);
    TRACE_2("off jmp", offset_to_jump, (uint)script_bus_addr + 
				       Ent_script_reconnect_point)
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_phase_mismatch					*/
/*									*/
/* FUNCTION:	Handles phase mismatch interrupts from the chip		*/
/*	This routine is called when the chip signals us that the SCSI   */
/*	phase asserted by the target does not match the instruction, 	*/
/*	which will typically be a block move instruction.  The       	*/
/*	resultant processing depends on the phase we expected to see.	*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine is called by the interrupt handler, or during a    */
/*      dump while interrupts are at INTMAX.  Chip is assumed on.       */
/*									*/
/* DATA STRUCTURES:							*/
/*	None.								*/
/*									*/
/* INPUTS:								*/
/*	*script_vir_addr - the virtual address of the executing script	*/
/*	*com_ptr - pointer to the cmd_info structure for the executing  */
/*		command.                                              	*/
/*	*restart_point - output variable to hold offset to restart chip */
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	PSC_NO_ERR for success.						*/
/*	PSC_FAILED if we weren't executing a Table Indirect Addressed	*/
/*		   block move (on the chip).				*/
/*									*/
/************************************************************************/
int
p8xx_phase_mismatch(
	ulong	*script_vir_addr,
	struct cmd_info	*com_ptr,
	int     *restart_point)
{
    dvi_t       *dev_ptr;
    ads_t	*ap;
    int		left_over_bytes;
    int		xfer_kind;
    int		ret_code = PSC_NO_ERR;

    P8printf(P8_ENTRY,("Entering p8xx_phase_mismatch\n"));
    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;
    TRACE_1("phase mis", (int) script_vir_addr)

    /*
     * There may be some bytes left in the chip.  Determine the kind of
     * transfer that was interrupted, and the number of bytes that were
     * not transferred.
     */
    if ((left_over_bytes = p8xx_get_leftovers(dev_ptr, &xfer_kind)) == -1) {
	TRACE_1("leftovers bad", -1)
	*restart_point = Ent_cleanup_phase;
	ret_code = PSC_FAILED;
    }
    else if ((xfer_kind == DCMD_DATA_OUT) || (xfer_kind == DCMD_DATA_IN)) {

        if (com_ptr->tim_tbl_used == 0) {
	    TRACE_1("no data", 0)
	    *restart_point = Ent_save_scntl2;
	    ret_code = PSC_FAILED;
        } else {

	    p8xx_update_dataptrs(ap, com_ptr, left_over_bytes);
	    *restart_point = Ent_save_scntl2;
	}
    }
    else if (xfer_kind == DCMD_CMD) {
        /*
         * We require the target to completely move in the command.
	 * Restart the script so that we will take a phase_error
	 * interrupt.  The response will then be to issue an abort
	 * message, or to reset the scsi bus.  Log the phase_error
	 * here. Note that a second error log entry may
	 * be generated after the phase_error interrupt, but that
	 * this entry will have the sense data to indicate where  
	 * the actual phase error occurred. 
	 */
	/* A possible code enhancement would be to call the phase
	 * error code from here, rather than generating the SCRIPTS
	 * interrupt.
	 */
        p8xx_logerr(ERRID_SCSI_ERR10, PHASE_ERROR, 0xCD, 0, ap, com_ptr, TRUE);
	*restart_point = Ent_phase_error_entry;
    }
    else if (xfer_kind == DCMD_MSG_OUT) {

	*restart_point = p8xx_message_out(ap, com_ptr, left_over_bytes);
    }
    TRACE_1("restrt", *restart_point)

    /* clear DMA and SCSI FIFOs */
    p8xx_write_reg(ap, CTEST3, CTEST3_SIZE, CTEST3_FLF);
    p8xx_write_reg(ap, STEST3, STEST3_SIZE, STEST3_CSF | STEST3_INIT);

    P8printf(P8_EXIT,("Leaving p8xx_phase_mismatch\n"));
    return (ret_code);
}

/************************************************************************
 *								
 * NAME:	p8xx_update_dataptrs
 *						
 * FUNCTION:    Modifies the local pointers after a phase mismatch
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is assumed on.
 *						
 * DATA STRUCTURES:			
 *	saved_t - the information regarding the pointer status is
 *		  adjusted since the current and saved pointers will
 *		  differ
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter information structure
 *      com_ptr	   - pointer to command information structure
 *	left_over_bytes - number of bytes on chip
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_update_dataptrs(ads_t *ap, cmd_t *com_ptr, int left_over_bytes) 
{
    tim_t  *tim_ptr;
    int	   xfd_bytes;
    int    tim_ent;
    int	   tim_adr, tim_len;
    uint   dsa_val;
    uchar tag_index = com_ptr->tag;
    saved_t *move_info_ptr;
    int tmp_tim_restore;
    int tim_ent_at_last_save;

    dsa_val = p8xx_read_reg(ap, DSA, DSA_SIZE);

    /* Determine which TIM entry we were on */
    tim_ent = (dsa_val - com_ptr->tim_bus_addr) / sizeof(tim_t);
    TRACE_2("total addr", com_ptr->tim_tbl_used, (int) com_ptr->tim_bus_addr)
    TRACE_2("dsa, ent", (int) dsa_val, (int) tim_ent)
    if (tim_ent < 0 || tim_ent >= com_ptr->tim_tbl_used) {
	/* tim_ent MUST be valid */
	ASSERT(tim_ent >= 0 && tim_ent <= com_ptr->tim_tbl_used); 
	tim_ent = 0;			/* make it valid */
    }

    tim_ptr = (tim_t *) ( ((uint) ap->loc_tim_tbl.system_ptr) +
	    ((com_ptr->resource_index + tim_ent) * sizeof(tim_t)));
    xfd_bytes = CBOtoL(tim_ptr->tim_len) - left_over_bytes;
    /* 
     * xfd_bytes now contains the transferred count for the CURRENT TIM 
     * entry. 
     */
    TRACE_2("timptr", (int) tim_ptr, (int) xfd_bytes)
    TRACE_2("before", (int) tim_ptr->tim_adr, (int) tim_ptr->tim_len)

    /*
     * Update the address field of the TIM entry to reflect the bytes
     * xferred so far.
     */
    tim_adr = CBOtoL(tim_ptr->tim_adr);
    tim_adr += xfd_bytes;
    tim_ptr->tim_adr = LtoCBO(tim_adr);

    /*
     * Update the length field of the TIM entry also
     */
    tim_len = CBOtoL(tim_ptr->tim_len);
    tim_len -= xfd_bytes;
    tim_ptr->tim_len = LtoCBO(tim_len); 

    TRACE_2("after", (int) tim_ptr->tim_adr, (int) tim_ptr->tim_len)

    /*
     * Update the dptr_info to indicate that the loc and sav tables
     * differ, and put the number of tim entries that need to be
     * copied into tim_restore.
     */

    move_info_ptr = (saved_t *) (
                ((ulong) ap->IND_TABLE.system_ptr + (16 * tag_index)));

    move_info_ptr->tables_differ = LtoCBO((int) TRUE);

    /* this pio could be eliminated if the SCRIPT looked at the nexus in
     * its save pointers processing
     */
    p8xx_write_reg(ap, SCRATCHA0, SCRATCHA0_SIZE, 0x01);

    /* 
     * number of iovecs needing to be copied is
     * the current entry (tim_ent) minus
     *    the entry as of the last save pointers plus
     *	      one (for the current entry)
     */
    tim_ent_at_last_save = LtoCBO(move_info_ptr->tim_cnt) - com_ptr->tim_tbl_used;
    tmp_tim_restore = tim_ent - tim_ent_at_last_save + 1;

    move_info_ptr->tim_restore = LtoCBO(tmp_tim_restore);
    TRACE_2("move info", (int) move_info_ptr->tim_bus_addr,
	                 (int) move_info_ptr->tim_cnt)
    TRACE_2("move info", (int) move_info_ptr->tables_differ,
	                 (int) move_info_ptr->tim_restore)
}

/************************************************************************
 *								
 * NAME:	p8xx_message_out
 *						
 * FUNCTION:    Handles a phase mismatch during the message out phase
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is assumed on.
 *						
 * DATA STRUCTURES:			
 *	None
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter information structure
 *      com_ptr	   - pointer to command information structure
 *	left_over_bytes - number of bytes on chip
 *				
 * RETURN VALUE DESCRIPTION:
 *	Offset at which to restart the script
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
int
p8xx_message_out(ads_t *ap, cmd_t *com_ptr, int left_over_bytes) 
{
    uint dsp_val;
    int   restart_point;
    uchar fall_to_default = FALSE;
    dvi_t *dev_ptr = com_ptr->device;
    uchar sstat1_val = p8xx_read_reg(ap, SSTAT1, SSTAT1_SIZE);
    uchar sbdl_val = p8xx_read_reg(ap, SBDL, SBDL_SIZE);

    dsp_val = p8xx_read_reg(ap, DSP, DSP_SIZE);
    /* back dsp up to identify which MSG_OUT_MOVE is being executed */
    dsp_val -= 8;
    TRACE_1("executing", (int) dsp_val)

    switch (dsp_val & 0x00000FFF) {
	/* allow the target the opportunity to reject our extended
	 * message.  However, if the target is not going to message
	 * in, send an abort.  If the target is going to message-in,
	 * the message should be latched on the data lines.  Make sure
	 * it is a message reject.  If it is, start the SCRIPT so that
	 * it reads in the message reject, then continues with any
	 * remaining message out, or allows the target to choose the
	 * next state.
	 */
	case Ent_renegotiate_wdtr_loop:
	case Ent_renegotiate_sdtr_loop:
	case Ent_start_negotiation_msg_out_loop:
	    TRACE_2("phse dat", (int) sstat1_val, (int) sbdl_val)
	    if (((sstat1_val & 0xF8) == SSTAT1_MSG_IN) || (sbdl_val == 0x07)) {
		switch (dsp_val & 0x00000FFF) {
		    case Ent_renegotiate_sdtr_loop:
		        restart_point = Ent_start_sdtr_msg_in_phase;
		        break;

		    case Ent_renegotiate_wdtr_loop:
			/* determine if the wdtr message or the sdtr
			 * message has been rejected. 
			 */
			if (left_over_bytes < SCSI_SDTR_MSG_LEN)
			    restart_point = Ent_start_sdtr_msg_in_phase;
			else      /* the wdtr was not completed */
			    restart_point = Ent_start_wdtr_msg_in_phase;
			break;

		    case Ent_start_negotiation_msg_out_loop:
		    {
		        uchar *sptr;  
		        uchar cur_byte;  
			int opcode_offset;

		        sptr = (uchar *) dev_ptr->sid_ptr->vscript_ptr;
			TRACE_1("byte 0", (int) sptr[Ent_neg_msg_buf]);
			if (sptr[Ent_neg_msg_buf] == 0x07) {
			    if (left_over_bytes == SCSI_SDTR_MSG_LEN) {
				/* target rejected our wide response,
				 * treat as narrow.  Don't negotiate
				 * for wide with this target again.
				 */
			        restart_point = Ent_start_wdtr_msg_in_phase;
		        	dev_ptr->sid_ptr->io_table.flags |= AVOID_WDTR;
	        		dev_ptr->sid_ptr->io_table.scntl3  
								&= ~SCNTL3_EWS;
			    } else  /* rejected our SDTR */
				restart_point = Ent_start_sdtr_msg_in_phase;

			} else if (sptr[Ent_neg_msg_buf] == 0x01) {
			    restart_point = Ent_start_sdtr_msg_in_phase;

			} else /* assume identify message*/ {
			    sptr = sptr + Ent_neg_msg_buf + 1;
			    cur_byte = *sptr;
			    if (cur_byte == 0x01)
				opcode_offset = 2;
			    else
				opcode_offset = 4;

			    sptr += opcode_offset;
			    TRACE_2 ("byte1 op", (int) cur_byte, (int) (*sptr))

			    if (*sptr == SCSI_WDTR_OPCODE) {
				if (left_over_bytes < SCSI_WDTR_MSG_LEN) 
				    restart_point = Ent_start_wdtr_msg_in_phase;
				else
				    fall_to_default = TRUE;

			    } else { /* SDTR_OPCODE */
				if (left_over_bytes < SCSI_SDTR_MSG_LEN) 
				    restart_point = Ent_start_sdtr_msg_in_phase;
				else
				    fall_to_default = TRUE;
			    }
			} /* end tests of first byte of neg_msg_buf */
			break;
		   } /* end start_negotiation block */	

		   default:
			fall_to_default = TRUE;

		} /* end inner switch */

		if (!fall_to_default) /* break unless we decided to */
		    break;            /* abort the target */
	    }
	    /* else fall into the default clause and treat as a
	     * phase error, since the target is presenting something
	     * other than a Message-Reject while we are in MSG_OUT and
	     * have ATN raised.
	     */
	
	/* the target should not be permitted to leave message out
	 * for the other cases (when we are sending an identify msg
	 * followed by a queue tag message, when we are sending an
	 * identify message followed by an abort message, etc.
	 */
	default:
	/* Restart the script so that we will take a phase_error
	 * interrupt.  The response will then be to issue an abort
	 * message, or to reset the scsi bus.
	 */
	/* A possible code enhancement would be to call the phase
 	 * error code from here, rather than generating the SCRIPTS
 	 * interrupt.
 	 */
	restart_point = Ent_phase_error_entry;
        /* Log the error.  Note that a second error log entry may
	 * be generated after the phase_error interrupt, but that
	 * this entry will have the sense data to indicate where  
	 * the actual phase error occurred. 
	 */
        p8xx_logerr(ERRID_SCSI_ERR10, PHASE_ERROR, 0xD2, 0, ap, com_ptr, TRUE);
	break;
    }

    return(restart_point);
}

/************************************************************************
 *								
 * NAME:	p8xx_recover_byte
 *						
 * FUNCTION:    Moves a bytes left on the chip into system memory.
 *	When wide scsi is being used, situations arise where the target
 *	has transferred a byte across the scsi bus to the chip, but the
 *	chip has not transferred the byte across the PCI bus.  This function
 *	determines the correct system address given the bus address, and
 *	copies this residue byte.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is assumed on.
 *						
 * DATA STRUCTURES:			
 *	struct sc_buf - pointer to scsi command and buf struct(s)
 *					
 * INPUTS:			
 *      ap	   - pointer to adapter information structure
 *      com_ptr	   - pointer to command structure
 *      bp	   - pointer to sc_buf structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	Returns an error indication:
 *						
 * ERROR DESCRIPTION			
 *	PSC_OVERFLOW - the residue byte is not wanted
 *      PSC_COPY_ERROR - xmemout failed
 *	zero otherwise
 *							
 ************************************************************************/
uint
p8xx_recover_byte(ads_t *ap, cmd_t *com_ptr, struct sc_buf *bp)
{
    /* 
     * Move a byte from SCRATCHA1 (shadow of SWIDE) to system memory
     * Adjust current data pointers
     */

    uchar last_byte;
    int i, ret_code;
    uchar *target_addr;
    tim_t *tim_ptr;
    int total_tims = com_ptr->tim_tbl_used;
    uint tot_spanned_bytes, bytes_moved = 0;
    uint tmp_word;
    uint remaining_tims;

    last_byte = p8xx_read_reg(ap, SCRATCHA1, SCRATCHA1_SIZE);
    TRACE_1("recover", last_byte)

    remaining_tims = p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE) & 0x0000FFFF; 
    if (remaining_tims == 0) {
	TRACE_1("overflow", 0);
	/* 
	 * the swide byte is probably wide residue or overflow.  We have
	 * already moved the correct number of bytes to system memory,
	 * the extra byte is just ignored.  Optionally we could log an
	 * error here.
         */
        return(PSC_OVERFLOW);
    }
	
    /* determine number of bytes moved to system memory so far */
    bytes_moved = bp->bufstruct.b_bcount - p8xx_calc_resid(ap,
	             remaining_tims, com_ptr);
    TRACE_1("bytes moved ", (int) bytes_moved)

    /* determine the target address */
    if (bp->bp == NULL)
        target_addr = (uchar *) bp->bufstruct.b_un.b_addr + bytes_moved;

    else if (bytes_moved == 0) { /* shouldn't occur */
	bp = (struct sc_buf *) bp->bp;
	target_addr = (uchar *) bp->bufstruct.b_un.b_addr;

    } else { /* spanned command, find the bufstruct associated  */
             /* with the current data transfer                  */
	struct buf *tmp_bp = bp->bp;	/* the first bp used for data xfer */

        /* find the bp associated with the last byte */
        /* transferred to system memory              */
        tot_spanned_bytes = tmp_bp->b_bcount;
        while (tot_spanned_bytes < bytes_moved) {
            tmp_bp = tmp_bp->av_forw;
            tot_spanned_bytes += tmp_bp->b_bcount;
        }

	if (tot_spanned_bytes == bytes_moved) {
            /* the last byte transferred to system memory was the  */
            /* last byte for the bp.  Put the residual byte at the */
            /* address specified by the next bp.                   */
	    bp = (struct sc_buf *) tmp_bp->av_forw;
	    TRACE_1("moved whole bp", (int) bp);
	    target_addr = (uchar *) bp->bufstruct.b_un.b_addr;
	} else {
            /* the last byte transferred to system memory was in the */
            /* middle of bp's address range.  Put the residual byte  */
            /* within this bp at the next specified address          */
            tot_spanned_bytes = tot_spanned_bytes - tmp_bp->b_bcount;
	    TRACE_1("moved partial bp", tot_spanned_bytes);
	    bp = (struct sc_buf *) tmp_bp;
	    target_addr = (uchar *) bp->bufstruct.b_un.b_addr +
		bytes_moved - tot_spanned_bytes;
	}
    }
    TRACE_2("bp addr", (int) bp, (int) target_addr)

    if (bp->bufstruct.b_xmemd.aspace_id == XMEM_GLOBAL) {       
	/* The data is in kernel space, assignment is sufficient */
        *(target_addr) = last_byte;
    } else {
        /* use xmemout to copy the data out to user space */
        ret_code = xmemout(&last_byte, target_addr, 1,
                           &bp->bufstruct.b_xmemd);

	if (ret_code != XMEM_SUCC) {
	    TRACE_1("bad xmemout", ret_code)
            p8xx_logerr(ERRID_SCSI_ERR6, XMEM_COPY_ERROR, 0x50,
                        0, ap, com_ptr, FALSE);
	    return(PSC_COPY_ERROR);
	}
    }

    /*
     * Finally, increment the current data pointers.  Access the
     * current tim entry, determine if we just moved the last byte of that
     * entry, and adjust the current data pointers accordingly.
     */
    tim_ptr = (tim_t *) ((uint) ap->loc_tim_tbl.system_ptr +
         sizeof(tim_t) * 
	    (com_ptr->resource_index + total_tims - remaining_tims));

    if (CBOtoL(tim_ptr->tim_len) == 1) {
	/* need to point to next TIM entry */
	p8xx_write_reg(ap, SCRATCHB, SCRATCHB_SIZE, remaining_tims - 1);
	p8xx_write_reg(ap, DSA, DSA_SIZE, 
		       p8xx_read_reg(ap, DSA, DSA_SIZE) + 8);
    } else {
	tmp_word = CBOtoL(tim_ptr->tim_len);
	tim_ptr->tim_len = LtoCBO(tmp_word-1);
	tmp_word = CBOtoL(tim_ptr->tim_adr);
	tim_ptr->tim_adr = LtoCBO(tmp_word+1);
    }

    return(0);
}

/************************************************************************
 *								
 * NAME:	p8xx_calc_resid
 *						
 * FUNCTION:    Determines the number of bytes not transferred
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.
 *						
 * DATA STRUCTURES:			
 *	loc_tim_tbl - current data pointers set of tim vectors
 *					
 * INPUTS:			
 *      ap	   - pointer to adapter information structure
 *	tim_cnt	   - number of unprocessed tim vectors
 *      com_ptr	   - pointer to command structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	number of bytes not transferred
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
uint
p8xx_calc_resid(ads_t *ap, ulong tim_cnt, cmd_t *com_ptr)
{
    saved_t *move_info_ptr;
    /*
     * Determine the total number of untransferred bytes
     */

    uint resid = 0;
    int i;
    ulong total_tims = com_ptr->tim_tbl_used;
    tim_t *tim_ptr;

    TRACE_2("index cnt", com_ptr->resource_index, tim_cnt)
    TRACE_1("factor", (int) (com_ptr->resource_index + total_tims - tim_cnt))
    tim_ptr = (tim_t *) ((uint) ap->loc_tim_tbl.system_ptr + 
	 com_ptr->resource_index * sizeof(tim_t));
    TRACE_2("calc rsd", (int) total_tims, (int) tim_ptr)

    for (i = total_tims - tim_cnt; i < total_tims; i++) {
	resid += CBOtoL(tim_ptr[i].tim_len);
	TRACE_1("new resid", (int) resid)
    }
	
    return (resid);
}


/************************************************************************
 *								
 * NAME:	p8xx_save_residue
 *						
 * FUNCTION:    Execute a save_pointers when there is a residue byte
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is on.
 *						
 * DATA STRUCTURES:			
 *	blist.dvec - original iovecs from D_MAP_LIST
 *	saved_t	   - data pointer information
 *					
 * INPUTS:			
 *      ap	   - pointer to adapter information structure
 *	direction  - flag indicating transfer direction
 *      com_ptr	   - pointer to command structure
 *				
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_save_residue(ads_t *ap, int direction, cmd_t *com_ptr) 
{
    saved_t *move_info_ptr;
    d_iovec_t dvec;
    tim_t *tim_ptr;
    ulong tmp_word, remaining_tims;
    uint ret_code;

    /* The block move of the Save Pointers message cleared WSR and WSS in
     * SCNTL2.  But we need to clear our shadow register.
     */
    p8xx_write_reg(ap, SCRATCHA2, SCRATCHA2_SIZE, 0x00);
    
    /* Do the direction-dependent processing */
    if (direction == A_save_ptrs_wsr) {
        TRACE_1("sav ptr wsr", (int) com_ptr)
        /*
         * Call recover_byte to move the byte to system memory
         * and adjust DSA and SCRATCHB as needed.  Issue an abort
	 * and return on error.
         */
	ret_code = p8xx_recover_byte(ap, com_ptr, com_ptr->bp);
	if (ret_code == PSC_COPY_ERROR) {
	    /* we failed in moving the byte to system memory,
	     * abort the target to end the command
	     */
	    p8xx_abort_current_command(ap, com_ptr, TRUE); 
	    return;
	}
	/* get the current scratchb_val, for use in setting tim_restore below */
	remaining_tims = p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE);

    } else {
        TRACE_1("sav ptr wss", (int) com_ptr)
        /*
         * Adjust the current data pointers to reflect that the byte
	 * currently in SODL has not been transferred.
	 */
	remaining_tims = p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE);
        dvec = (struct d_iovec *) ap->blist_base_vector +
                         com_ptr->resource_index;
	dvec = dvec + (com_ptr->tim_tbl_used - remaining_tims);

        tim_ptr = (tim_t *) ap->loc_tim_tbl.system_ptr + 
	                     com_ptr->resource_index;
	tim_ptr = tim_ptr + (com_ptr->tim_tbl_used - remaining_tims);

	TRACE_2("cur vecs", (int) dvec, (int) tim_ptr)
	/*
	 * If we are past the last vector for the command, or if adding
	 * one to the current vector would exceed the original length,
	 * back up to the previous vector so set it to move 1 byte.
         * Otherwise back up the data pointers in the current vector.
	 */
	if ((remaining_tims == 0) || 
	    (dvec->iov_len == CBOtoL(tim_ptr->tim_len))) {
	    /* update the previous tim */
	    tim_ptr--;
	    dvec--;
	    tim_ptr->tim_len = LtoCBO(1);
	    tim_ptr->tim_adr = LtoCBO((uint) dvec->iov_base + 
					     dvec->iov_len - 1);
	    remaining_tims++;
	    /* update SCRATCHB and DSA */
	    p8xx_write_reg(ap, SCRATCHB, SCRATCHB_SIZE, remaining_tims);
	    tmp_word = p8xx_read_reg(ap, DSA, DSA_SIZE);
	    tmp_word -= 8;
	    p8xx_write_reg(ap, DSA, DSA_SIZE, tmp_word);
 
	} else {
	    tmp_word = CBOtoL(tim_ptr->tim_len);
	    tim_ptr->tim_len = LtoCBO(tmp_word+1);
	    tmp_word = CBOtoL(tim_ptr->tim_adr);
	    tim_ptr->tim_adr = LtoCBO(tmp_word-1);
	}
    }

    /* Indicate that the current and saved tables now differ */
    move_info_ptr = (saved_t *) (
            ((ulong) ap->IND_TABLE.system_ptr + (16 * com_ptr->tag)));
    move_info_ptr->tables_differ = LtoCBO((int) TRUE);
    p8xx_write_reg(ap, SCRATCHA0, SCRATCHA0_SIZE, 0x01);
    move_info_ptr->tim_restore = LtoCBO(remaining_tims);

    /* Finally, have the SCRIPT continue the Save Pointers processing */
    p8xx_start_chip(ap, com_ptr, 
	    com_ptr->device->sid_ptr->bscript_ptr,
	    Ent_save_pointers, 0, 0);
	
}

/************************************************************************
 *								
 * NAME:	p8xx_ignore_residue
 *						
 * FUNCTION:    Handles the ignore wide residue message
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is on.
 *						
 * DATA STRUCTURES:			
 *	data pointer structures (loc_tim_tbl, saved_t structure)
 *					
 * INPUTS:			
 *      com_ptr	   - pointer to command structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_ignore_residue(cmd_t *com_ptr)
{
    /*
     * First validate the 2nd byte, it is only valid if we are doing
     * wide transfers, and in that case only if it is 0x01h.  If it is
     * not valid, restart the chip at a point where it will respond
     * with a message reject.
     * If the byte is valid, determine if there is a residue byte currently
     * on the chip.  If there is, just clear the appropriate flag, no
     * adjustments to the data pointers are needed.  If there is not, the
     * data pointers need to be adjusted, since we are discarding a byte
     * we have already moved to system memory. Determine the last byte received
     * during the DATA-IN phase, and its associated TIM entry.  Adjust the 
     * TIM entry, and adjust DSA and SCRATCHB if they don't reflect the correct 
     * TIM entry.  Finally, if any updates were made to the local TIM table,
     * set the appropriate dptr info to indicate the tables differ and which
     * TIM entry differs.
     * Note we are assuming the IWR message immediately follows the DATA_IN
     * phase, as required by the SCSI spec.
     */

    ads_t       *ap;
    dvi_t	*dev_ptr;
    uint        *script_vir_addr;
    uint        *script_bus_addr;
    uint        *target_script;
    uchar	ignore_field;

    P8printf(P8_ENTRY,("Entering p8xx_ignore_residue\n"));
    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;
    script_vir_addr = (uint *) dev_ptr->sid_ptr->vscript_ptr;
    script_bus_addr = (uint *) dev_ptr->sid_ptr->bscript_ptr;

    target_script = script_vir_addr;
    ignore_field = target_script[S_INDEX(Ent_cmd_msg_in_buf)] >> 24;
    TRACE_2("ignore r", (int) com_ptr, (int) ignore_field)

    if ((!(dev_ptr->sid_ptr->io_table.scntl3 & SCNTL3_EWS)) ||
	  (ignore_field != 0x01)) {
        /* the IWR message is inappropriate, reject it and continue */
        p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
             Ent_reject_target_sync, 0, 0);
	TRACE_1("rejecting IWR", (int) dev_ptr->sid_ptr->io_table.scntl3)

    } else {
	/* See if there is a byte in SWIDE by reading our shadow register.
	 * If there is, clear our shadow register, and we are done. Else,
	 * adjust the data pointers backwards 1 byte. 
	 */
        uchar wsr_shadow = p8xx_read_reg(ap, SCRATCHA2, SCRATCHA2_SIZE);	

	if (wsr_shadow & SCNTL2_WSR) {
	    TRACE_1("discarding", (int) wsr_shadow)
	    p8xx_write_reg(ap, SCRATCHA2, SCRATCHA2_SIZE, wsr_shadow ^ 0x01);

        } else {
	    /* determine last byte transferred */
            uint scratchb_val = p8xx_read_reg(ap, SCRATCHB, SCRATCHB_SIZE);
            uint dsa_val = p8xx_read_reg(ap, DSA, DSA_SIZE);
	    tim_t *loc_tim, *sav_tim;
            uchar previous_tim, updates_needed;
            uint tmp_word;
	    uchar tag_index = com_ptr->tag;
            saved_t *move_info_ptr;

            loc_tim = (tim_t *) 
		((dsa_val & 0x00000FFF) + (uint) ap->loc_tim_tbl.system_ptr);
            sav_tim = (tim_t *) 
		((dsa_val & 0x00000FFF) + (uint) ap->sav_tim_tbl.system_ptr);

	    TRACE_2("tims", (int) sav_tim, (int) loc_tim)

	    /* 
	     * previous_tim indicates if the byte to be discarded is the
	     * last byte of the previous tim.  We are assuming that the
	     * tim_len fields in the saved and current tables differ if
	     * we are looking at the current tim, since a phase mismatch
	     * would have occurred prior to our receipt of the IWR message.
	     */
		
            if (scratchb_val == 0) {
	        previous_tim = TRUE;
	    } else if (loc_tim->tim_len == sav_tim->tim_len) {
	        previous_tim = TRUE;
            } else
	        previous_tim = FALSE;

	    if (previous_tim) {
	        sav_tim = (tim_t *) ((uint) sav_tim - 8);
	        loc_tim = (tim_t *) ((uint) loc_tim - 8);
	        TRACE_2("prev tim", (int) sav_tim, (int) loc_tim)

		/* adjust scratchb and dsa */
		scratchb_val += 1;
		p8xx_write_reg(ap, SCRATCHB, SCRATCHB_SIZE, scratchb_val);
		p8xx_write_reg(ap, DSA, DSA_SIZE, dsa_val - 8);

		/* Since DSA and SCRATCHB indicated the TIM had been
		 * processed, the tim entry should indicate all bytes 
		 * were moved.  Note that the update in response to the
		 * IWR message will still be made below.
		 */
		tmp_word = CBOtoL(loc_tim->tim_adr) + CBOtoL(loc_tim->tim_len);
		loc_tim->tim_adr = LtoCBO(tmp_word);
		loc_tim->tim_len = 0;
		TRACE_2("adjust", (int) dsa_val, (int) scratchb_val)
	    }

	    /* we now adjust the data pointers */
	    tmp_word = CBOtoL(loc_tim->tim_len);
	    loc_tim->tim_len = LtoCBO(tmp_word + 1);
	    tmp_word = CBOtoL(loc_tim->tim_adr);
	    loc_tim->tim_adr = LtoCBO(tmp_word - 1);
	    TRACE_2("new vals", loc_tim->tim_len, loc_tim->tim_adr)

	    /* set tables differ and tim_restore */
            move_info_ptr = (saved_t *) (
                    ((ulong) ap->IND_TABLE.system_ptr + (16 * tag_index)));
            move_info_ptr->tables_differ = LtoCBO((int) TRUE);
            move_info_ptr->tim_restore = LtoCBO(scratchb_val);
	}

	/* Finally, restart the chip to continue on */
	p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
			    Ent_msg_done, 0, 0);
    }
    return;
}

/************************************************************************/
/*									*/
/* NAME:	p8xx_handle_extended_messages				*/
/*									*/
/* FUNCTION:	Adpater Script to Read and Process Extended Messages	*/
/*	This function does is called for two cases:			*/
/*	First, an extended message was received before a command was	*/
/*	sent, after a command was sent, or after data was received.	*/
/*	We must read in the variable length extended message.  Thus	*/
/*	we set up the byte count of how many message bytes be read	*/
/*	in and complete the reading in of the messages.			*/
/*	Second, if the extended message is a command to modify the	*/
/*	data pointer, then we must modify the dma address that the	*/
/*	script will dma to when the script continues.			*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	Called with interrupts disabled.  Chip is assumed on.   	*/
/*									*/
/* DATA STRUCTURES:							*/
/*	None.								*/
/*									*/
/* INPUTS:								*/
/*	*com_ptr - pointer to the cmd_info structure which holds	*/
/*		the sc_buf which contains the SCSI command given to us	*/
/*		by the driver head.					*/
/*	interrupt_flag - Interrupt that resulted in this call by the	*/
/*		interrupt handler.					*/
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/************************************************************************/
void
p8xx_handle_extended_messages(
        struct cmd_info *com_ptr,
	int	interrupt_flag)
{
    dvi_t	*dev_ptr;
    ads_t	*ap;
    uint	*script_vir_addr;
    uint	*script_bus_addr;
    uint	*target_script;
    uchar	ext_msg_opcode;
    uchar	ec_flag;
    ulong	ext_msg_length;

    P8printf(P8_ENTRY,("Entering p8xx_handle_extended_messages\n"));

    dev_ptr = com_ptr->device;
    ap = dev_ptr->ap;
    TRACE_1("ext msg intrpt", 0)
    script_vir_addr = (uint *) dev_ptr->sid_ptr->vscript_ptr;
    script_bus_addr = (uint *) dev_ptr->sid_ptr->bscript_ptr;

    target_script = script_vir_addr;

    /* 
     * Get extended message values, ext_msg_length is the xfer period &
     * ext_msg_opcode is req/ack offset
     */
    ext_msg_length = (target_script[S_INDEX(Ent_extended_msg_buf)] >> 24);
    ext_msg_opcode = (target_script[S_INDEX(Ent_extended_msg_buf)] >> 16);

    /*
     * Patch the remaining number of the message bytes to read
     */
    target_script[R_ext_msg_size_Used[0]] &= 0x000000FF;
    if (ext_msg_length != 0)
	target_script[R_ext_msg_size_Used[0]] |= LtoCBO(ext_msg_length - 1);

    switch(interrupt_flag) {
    case A_ext_msg:
	/* Patch the script to interrupt after the remaining message bytes
	 * are read.  If the message is the SCSI-1 Extended Identify 
	 * message, we just read the message and continue (rather than
	 * interrupting).
	 */
	if (ext_msg_opcode == 0x02) {
	    target_script[S_INDEX(Ent_ext_msg_patch)] =
		LtoCBO(P8XX_NOP_PATCH);
	} else {
	    target_script[S_INDEX(Ent_ext_msg_patch)] =
		LtoCBO(P8XX_INTR_PATCH);
	    switch(ext_msg_opcode) {
		case 0x00:	/* Modify Data Pointers */
		    target_script[S_INDEX(Ent_ext_msg_patch) + 1] =
			LtoCBO(A_modify_data_ptr);
		    break; 
		case 0x01:	/* Synchronous Negotiation */
		    target_script[S_INDEX(Ent_ext_msg_patch) + 1] =
			LtoCBO(A_target_sdtr_sent);
		    break; 
		case 0x03:	/* Wide Negotiation */
		    target_script[S_INDEX(Ent_ext_msg_patch) + 1] =
			LtoCBO(A_target_wdtr_sent);
		    break; 
		default:
		    target_script[S_INDEX(Ent_ext_msg_patch) + 1] =
			LtoCBO(A_unknown_ext_msg);
		    break; 
	    }
	}
	p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
	    Ent_complete_ext_msg, 0, 0);
	break;

    case A_modify_data_ptr:
	/* 
	 * Go to script that sends msg reject and then
	 * continues on
	 */
	p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
	    Ent_reject_target_sync, 0, 0);
	break;

    case A_target_wdtr_sent: {
	uchar xfer_width = ext_msg_length;
	TRACE_1("wdtr sent", (int) xfer_width)
	/*	
	 * wide negotiation negates any previous synchronous data xfer 
 	 * agreement.  Although we will send a SDTR after our WDTR response
	 * patch the script for async as a precaution
	 */
        p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);
        com_ptr->flags |= NEGOT_CMD;

	/* send a narrow reply if the target is requesting narrow */
	/* or if we can only do narrow */
	if ((ap->ddi.chip_type == IS_A_53C810) || (xfer_width == 0x00)) {
            target_script[S_INDEX(Ent_wdtr_msg_out_buf)] = 
			SCSI_WDTR_MSG & 0xFFFFFF00;
            dev_ptr->sid_ptr->io_table.scntl3 &= ~SCNTL3_EWS;
	} else  { /* patch to run at 16 */
            target_script[S_INDEX(Ent_wdtr_msg_out_buf)] = 
			SCSI_WDTR_MSG;
            dev_ptr->sid_ptr->io_table.scntl3 |= SCNTL3_EWS;
	}

        /* prepare for the upcoming SDTR message exchange */
        target_script[S_INDEX(Ent_negotiation_phase_patch)] =
            LtoCBO(P8XX_JUMP_PATCH);
        target_script[S_INDEX(Ent_wdtr_msg_out_buf)+1] = SCSI_SDTR_MSG;
        target_script[S_INDEX(Ent_wdtr_msg_out_buf)+1] |= (ulong) ap->xfer_pd;
        target_script[S_INDEX(Ent_sdtr_msg_out_buf)] = 
						    (NCR8XX_DEF_OFFSET << 24);

	if (ap->ddi.chip_type != IS_A_53C810)
	    /* activate or deactivate special microcode for wide scsi */
	    p8xx_wide_patch(dev_ptr, FALSE);

	/* start chip at renegotiate_wdtr */
	p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
		    Ent_renegotiate_wdtr, 0, 0);
	}
	break;

    case A_target_sdtr_sent:
	TRACE_2("sdtr sent", (int) ext_msg_opcode, (int) ext_msg_length)
        if ((com_ptr->bp->scsi_command.flags & SC_ASYNC) &&
	    (ap->ddi.chip_type == IS_A_53C810)) {
            /*
             * The target is trying to do sync negotiation but we do
             * not want to do sync neg.  Thus we will send a reject
             * msg out and force it into ASYNC mode
             */

            dev_ptr->sid_ptr->async_device = TRUE;

            /* Patch the SCRIPT for async transfers. */
            p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);

            /*
             * Go to script that sends msg reject
             * for sync and then continues on
             */
            p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
                Ent_reject_target_sync, 0, 0);

	} else {

	    /* 
	     * Being here tells us that we have previously patched
	     * this interrupt in order to move in the target's
	     * sync request.  Now, we must examine the request to
	     * determine if it is higher, lower, or equal to what
	     * we wish to be the sync transfer rate. In this case
	     * the ext_msg_length is really the xfer period and the
	     * ext_msg_opcode is really the rec/ack offset
	     */

	    uchar   xfer_pd = ext_msg_length,
		    req_ack = ext_msg_opcode,
		    best_pd, scntl3_val, sxfer_val;

	    /*
	     * If the REQ/ACK offset is zero, or the requested
	     * transfer period is slower than we can set the chip,
	     * we will run async.
	     */
	    if ((req_ack == 0) ||
	        (best_pd = p8xx_select_period(ap, xfer_pd, &scntl3_val,
	    	    &sxfer_val)) == 0) {

	        /*
	         * We're going to run asynchronous
	         */
	     
	        /* Patch the SCRIPTS to run asynchronously. */
	        p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);

	        dev_ptr->sid_ptr->async_device = TRUE;

	        /* send a SDTR msg indicating async */
	    	/* Patch in the negotiation message */
		target_script[S_INDEX(Ent_sdtr_msg_out_buf)] =
		        (SCSI_SDTR_MSG | (ulong) xfer_pd);
		target_script[S_INDEX(Ent_sdtr_msg_out_buf)+1] =
		        ((ulong) 0x00 << 24);

		p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
			    Ent_renegotiate_sdtr, 0, 0);

	    } else {		/* we're going synchronous */
	        dev_ptr->sid_ptr->async_device = FALSE;
		
		/* include any previously established WDTR result */
                scntl3_val |= (dev_ptr->sid_ptr->io_table.scntl3 & SCNTL3_EWS);

	        /* We first ensure that the target's offset is okay */
	        if (req_ack > NCR8XX_MAX_OFFSET) {
		    target_script[S_INDEX(Ent_sdtr_msg_out_buf)+1]
		        = ((ulong) NCR8XX_MAX_OFFSET << 24);
		    sxfer_val |= NCR8XX_MAX_OFFSET;
	        } else {
		    target_script[S_INDEX(Ent_sdtr_msg_out_buf)+1]
		        = ((ulong) req_ack << 24);
		    sxfer_val |= req_ack;
	        }

	        /*
	         * Note that best_pd contains the closest match for the
	         * target's requested transfer period that we can do.
	         * If these values do not match, we have three possibilities:
	         *
	         *	xfer_pd < best_pd: Target wants to go faster than we
	         *		can. We will re-negotiate with our values.
	         *
	         *	xfer_pd > best_pd: Target wants to run at a rate
	         *		we cannot send at.  We will send at the next
	   	 *		slower rate, the chip can still receive at
		 *		the target's rate.
		 *
		 *	xfer_pd == best_pd: We have an exact match
		 *
		 * In all 3 cases, we can echo best_pd back to the target.
	         */

		target_script[S_INDEX(Ent_sdtr_msg_out_buf)] =
		        (SCSI_SDTR_MSG | (ulong) best_pd);

	        p8xx_do_patch(dev_ptr, sxfer_val, scntl3_val);

	        /* echo the SDTR msg using default values */
		p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
		        Ent_renegotiate_sdtr, 0, 0);

	    }
	}
	break;

    case A_unknown_ext_msg:
    default:
	/*
	 * Note: the unknown_msg_hdlr entry point in the SCRIPTS
	 * simply does an immediate interrupt of unknown_msg.
	 */
	TRACE_1("bad ext msg", interrupt_flag)
	p8xx_start_chip(ap, com_ptr, (uint) script_bus_addr,
	    Ent_unknown_msg_hdlr, 0, 0);
	break;
    } /* end of switch on kind of interrupt */
		    
}


/************************************************************************
 *								
 * NAME:	p8xx_abort_current_command
 *						
 * FUNCTION:    Issues an abort in response to the action of a target we
 *		are already on the active_q for, using the same cmd_info
 *		structure.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled.  Chip is on.
 *						
 * DATA STRUCTURES:			
 *	dev_info structure - target lun unique data structure
 *	struct sc_buf      - structure relating to the scsi cmd from the head dd
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter information structure
 *      com_ptr	   - pointer to cmd_info structure
 *	new_script - flag used to indicate if we are currently on the scsi bus
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_abort_current_command(ads_t *ap, cmd_t *com_ptr, uchar connected)
{
    dvi_t *dev_ptr = com_ptr->device;
    struct sc_buf *bp = com_ptr->bp;

    if (!(dev_ptr->flags & SCSI_BDR)) {
        dev_ptr->cmd_activity_state = ABORT_IN_PROGRESS;

        /* note we are still on the active q */
	/* Issue an abort SCRIPT call */
	p8xx_issue_abort_script(com_ptr,  connected);
	P8printf(P8_ABORT, ("%s at %s:%d for ID %d, intr 0x%02x\n",
		"ABORT set",__FILE__,__LINE__, dev_ptr->scsi_id, connected));

        dev_ptr->flags |= SCSI_ABORT;

	/* Mark the bp */
        if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR))) {
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->status_validity = SC_ADAPTER_ERROR;
	    bp->general_card_status = SC_SCSI_BUS_FAULT;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_error = EIO;
	}

        /* if an abort was already queued */
        /* then dequeue it. */
        if ((ap->ABORT_BDR_head == dev_ptr) || 
	    (dev_ptr->ABORT_BDR_bkwd != NULL))
            p8xx_deq_abort_bdr(dev_ptr);

    } else { /* a BDR has been requested */
        if (dev_ptr != dev_ptr->sid_ptr->bdring_lun) {
            dev_ptr = dev_ptr->sid_ptr->bdring_lun;
            com_ptr = &ap->command[ap->ddi.addi.card_scsi_id << 3];
            com_ptr->bp = NULL;
            com_ptr->device = dev_ptr;
            p8xx_enq_active(dev_ptr, com_ptr);
        }

	p8xx_issue_bdr_script(dev_ptr, com_ptr, connected);

        dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;

        p8xx_deq_abort_bdr(dev_ptr);
    }

    p8xx_start_abort_timer(dev_ptr, TRUE);

    /* Note that any ioctl sleeper will be wakened at */
    /* the completion of the abort or bdr.  */
}

/************************************************************************
 *								
 * NAME:	p8xx_sir_abort_error
 *						
 * FUNCTION:   This function is called when a SIR (planned Script 
 *	interrupt) indicates that an abort message should be issued as part
 *	of the error recovery procedure.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled or in the dump
 *      context.  The chip is assumed to be on.
 *						
 * DATA STRUCTURES:			
 *	dev_info structure - target lun unique data structure
 *					
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target structure
 *      com_ptr	   - pointer to command structure
 *	save_interrupt - identifies the SIR
 *	save_isr   - holds the results of a prior pio read to ISTAT
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_sir_abort_error(ads_t *ap,
	dvi_t *dev_ptr, cmd_t *com_ptr, int save_interrupt,
	int save_isr)
{
    struct sc_buf *bp = com_ptr->bp;

#ifdef P8_TRACE
    if (save_interrupt == A_phase_error)
        TRACE_1 ("phase error", (int) dev_ptr)
    else if (save_interrupt == A_unknown_msg)
        TRACE_1 ("unk msg", (int) dev_ptr)
    else
        TRACE_1 ("not ext msg", (int) dev_ptr)
#endif

    P8printf(P8_INFO_1|P8_ABORT, ("p8xx_intr: %s @ %x, SBCL=%02x\n",
	       save_interrupt == A_phase_error   ? "phase_error" :
	       save_interrupt == A_err_not_ext_msg ? "err_not_ext_msg" :
	       "unknown_msg", p8xx_read_reg(ap,DSP,DSP_SIZE),
	       p8xx_read_reg(ap,SBCL,SBCL_SIZE)));

    P8if(P8_ABORT, 
        {p8xx_rpt_status(ap,
	    save_interrupt == A_phase_error   ? "phase_error" :
	    save_interrupt == A_err_not_ext_msg ? "err_not_ext_msg" :
	    "unknown_msg", 2);
        P8if(P8_ASSERT,{assert(0)});
    });

    if (!ap->dump_started) {
	W_STOP(&dev_ptr->dev_watchdog.dog);

	if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR))) {

	    /* There's no previous error so log this one. */
	    switch (save_interrupt) { /* for command logging */

		case A_phase_error:
		    p8xx_logerr(ERRID_SCSI_ERR10, PHASE_ERROR,
				0x5A, 0, ap, com_ptr, TRUE);
		    break;

		case A_err_not_ext_msg:
		    p8xx_logerr(ERRID_SCSI_ERR10, ERROR_NOT_EXTENT,
				0x5A, 0, ap, com_ptr, TRUE);
		    break;

		case A_unknown_msg:
#ifdef P8_TRACE
		{
		    uint *target_script;
                    target_script = (uint *) dev_ptr->sid_ptr->vscript_ptr;
                    TRACE_1 ("unk msg2", (target_script
                                      [Ent_cmd_msg_in_buf / 4] >> 24))
		}
#endif

		default:
		    p8xx_logerr(ERRID_SCSI_ERR10, UNKNOWN_MESSAGE,
				0x5A, 0, ap, com_ptr, TRUE);
		    break;
	    }
	}
    } /* end of normal-mode only processing */

    if ((dev_ptr->cmd_activity_state == ABORT_IN_PROGRESS) ||
	(dev_ptr->cmd_activity_state == BDR_IN_PROGRESS)) {

	/*
	 * Remove device from active queue if there is a 
	 * command tagged to this 
	 */
	if (bp != NULL) {
	    if (!(bp->bufstruct.b_flags & B_ERROR)) {
		/* no previous error */
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->status_validity = SC_ADAPTER_ERROR;
		bp->general_card_status = SC_SCSI_BUS_FAULT;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
	    }
	}

	P8printf(P8_ABORT, ("%s at %s:%d for ID %d, intr 0x%02x\n",
	   "RESET set",__FILE__,__LINE__, dev_ptr->scsi_id, save_interrupt));

	p8xx_command_reset_scsi_bus(ap,3);

    } else {

        /* at this point in the processing, we got an error for a command 
	 * other than a bdr or abort, and logged it if needed.  Now
         * we issue an abort, unless a BDR is already pending, in which 
	 * case we issue the BDR.
	 */
	p8xx_abort_current_command(ap, com_ptr, save_isr & ISTAT_CONNECTED); 
    }
}

/************************************************************************
 *								
 * NAME:	p8xx_abdr_sel_fail
 *						
 * FUNCTION:   This function is called to process a planned Script 
 *	interrupt that indicates that the chip was unable to arbitrate
 *	or lost arbitration when executing a Select instruction for an
 *	abort or bdr command.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target structure
 *      com_ptr	   - pointer to command structure
 *	save_interrupt - flag used to distinguish between abort or bdr
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_abdr_sel_fail(ads_t *ap, 
	dvi_t *dev_ptr, cmd_t *com_ptr, int save_interrupt)
{
    int i, dev_index;
    uchar no_cmds_active;
    dvi_t *tmp_dev_ptr;

    P8printf(P8_INFO_1,("p8xx_intr: ABRTBDR_SEL_FAILED \n"));
    TRACE_1 ("ABneg bad", (int) dev_ptr)

    /* deq_active the unstarted Abort/BDR */
    p8xx_deq_active(dev_ptr, com_ptr);

    /*
     * if there are outstanding cmds for lun (aborts) or id (bdrs), 
     * we need to restore the iowait jump
     */
    if (save_interrupt == A_abort_select_failed) {
        dev_ptr->cmd_activity_state &= ~ABORT_IN_PROGRESS;
        if (dev_ptr->active_head != NULL) {
	    p8xx_restore_iowait_jump( (uint *) ap->Scripts.vscript_ptr, 
		    dev_ptr, dev_ptr->sid_ptr->bscript_ptr);
	}

    } else {  /* a BDR */ 

        dev_ptr->cmd_activity_state &= ~BDR_IN_PROGRESS;

        dev_index = dev_ptr->scsi_id << 3;
        for (i = 0; i< MAX_LUNS; i++) {
            if (((tmp_dev_ptr = ap->device_queue_hash[dev_index + i]) != NULL) 
			&& (tmp_dev_ptr->active_head != NULL)) {
                p8xx_restore_iowait_jump((uint *) ap->Scripts.vscript_ptr, 
		    tmp_dev_ptr, tmp_dev_ptr->sid_ptr->bscript_ptr);
            }
        } /* end of for loop checking each lun */
    }

    /* Put device on front of abort queue   */
    if (ap->ABORT_BDR_head == NULL) {
        dev_ptr->ABORT_BDR_fwd = NULL;
	dev_ptr->ABORT_BDR_bkwd = NULL;
	ap->ABORT_BDR_head = dev_ptr;
	ap->ABORT_BDR_tail = dev_ptr;
    } else {
	/* List not empty */
	ap->ABORT_BDR_head->ABORT_BDR_bkwd = dev_ptr;
	dev_ptr->ABORT_BDR_bkwd = NULL;
	dev_ptr->ABORT_BDR_fwd = ap->ABORT_BDR_head;
	ap->ABORT_BDR_head = dev_ptr;
    }

    if (ap->DEVICE_ACTIVE_head == NULL)
	no_cmds_active = TRUE;
    else
	no_cmds_active = FALSE;

    /* 
     * If there is a bp associated with the abort/bdr,
     * place it back on the active queue, to be handled
     * when the abort/bdr (or bus reset) is resolved.
     */

    if (com_ptr->bp)
        p8xx_enq_active(dev_ptr, com_ptr);

    /*
     * If no commands are outstanding, the SCSI bus is hung. Attempt a 
     * scsi bus reset to clear up this condition.
     */
    if (no_cmds_active ||
	(dev_ptr->retry_count >= PSC_RETRY_COUNT)) {
	p8xx_command_reset_scsi_bus(ap,2);
    } else {
	/* Restart IOWAIT routine */
	INT_ON(dev_ptr->sid_ptr->vscript_ptr, A_check_next_io_Used);
	p8xx_start_chip(ap, NULL, ap->Scripts.bscript_ptr,
			    Ent_iowait_entry_point, 0, 0);
    }
}

/************************************************************************
 *								
 * NAME:	p8xx_bdr_done
 *						
 * FUNCTION:   This function is called to process a planned Script 
 *	interrupt that indicates a bdr completed successfully.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled, or in the dump
 *	context.  The chip is assumed to be on.
 *						
 * INPUTS:			
 *	ap	   - pointer to adapter_def structure
 *      dev_ptr	   - pointer to target structure
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p8xx_bdr_done(ads_t *ap,
	dvi_t *dev_ptr)
{
    int i, dev_index;
    dvi_t *tmp_dev_ptr;
    uchar abrt_was_pending;

    TRACE_1 ("BDR done", (int) dev_ptr)
    P8printf(P8_INFO_1,("p8xx_intr: BDR_IO_COMPLETE \n"));
    if (!ap->dump_started)
        W_STOP(&dev_ptr->dev_watchdog.dog);

    dev_ptr->cmd_activity_state = NOTHING_IN_PROGRESS;

    /* reflect the hard reset for the id */
    dev_ptr->sid_ptr->negotiate_flag = TRUE;
    dev_ptr->sid_ptr->restart_in_prog = TRUE;

    /* return the commands for all LUNs */
    dev_index = dev_ptr->scsi_id << 3;
    for (i = 0; i< MAX_LUNS; i++) {
        if ((tmp_dev_ptr = ap->device_queue_hash[dev_index + i]) != NULL) {

	    tmp_dev_ptr->flags &= ~SCSI_BDR_or_ABORT;
            p8xx_fail_cmd(tmp_dev_ptr);

            /* finally, handle any queued aborts, and wakeup any
             * ioctl sleepers as needed.  
             */
            if ((ap->ABORT_BDR_head == tmp_dev_ptr) || 
		(tmp_dev_ptr->ABORT_BDR_bkwd != NULL)) {
                p8xx_deq_abort_bdr(tmp_dev_ptr);
                abrt_was_pending = TRUE;
            } else
                abrt_was_pending = FALSE;

            if ((tmp_dev_ptr->ioctl_wakeup == TRUE) && (!ap->dump_started)) {
                if ((tmp_dev_ptr == dev_ptr) && (abrt_was_pending == FALSE))
                    dev_ptr->ioctl_errno = PSC_NO_ERR;
                else
                    tmp_dev_ptr->ioctl_errno = EIO;

                tmp_dev_ptr->ioctl_wakeup = FALSE;
                e_wakeup(&tmp_dev_ptr->ioctl_event);
            }
        } /* end of check for null dev_info structure */
    } /* end of loop checking all luns */

    dev_ptr->sid_ptr->bdring_lun = NULL;

    if (!ap->dump_started) {
        ap->restart_watchdog.dog.restart = ap->ddi.addi.cmd_delay + 1;
        W_START(&(ap->restart_watchdog.dog));
    }

}

/************************************************************************/
/*									*/
/* NAME:	p8xx_issue_abort_script					*/
/*									*/
/* FUNCTION:								*/
/*	This function will patch in the ABORT message into the message	*/
/*	buffer and patch all interrupts within the abort_bdr script	*/
/*	to reflect that this was an abort script executing if it ever	*/
/*	hits an interrupt because of an error.				*/
/*									*/
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
/*	com_ptr - pointer to the command structure.			*/
/*	connected - indicates whether to try to select the device or	*/
/*		whether we are already connected to it and should just	*/
/*		try to raise ATN and send the abort message.  We start	*/
/*		the script at a different point depending on the value. */
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/************************************************************************/
void
p8xx_issue_abort_script(
	cmd_t	*com_ptr,
	uchar	connected)
{
    ads_t	*ap;
    dvi_t	*dev_ptr = com_ptr->device;
    sid_t       *sid_ptr;
    ulong	*target_script,
		*target_script_b;
    ulong	id_bit_mask, abort_entry_point;
    int		rc;

    P8printf(P8_ENTRY|P8_ABORT, ("ABORT issued for ID %d LUN %d\n",
	dev_ptr->scsi_id, dev_ptr->lun_id));

    ap = dev_ptr->ap;
    TRACE_2 ("iss abrt", (int) com_ptr, (int) connected)

    sid_ptr = dev_ptr->sid_ptr;
    sid_ptr->tag_flag = ap->ddi.addi.card_scsi_id << 3;
    sid_ptr->lun_flag = 0xFF;

    target_script   = (ulong *) sid_ptr->vscript_ptr;
    target_script_b = (ulong *) sid_ptr->bscript_ptr;

    /* update wait script so that the LUN cannot reselect us */
    p8xx_reset_iowait_jump(&ap->Scripts, dev_ptr, FALSE);

    if (!connected) {
	id_bit_mask = 0x80000000;
	id_bit_mask |= (0x00060000 | (dev_ptr->lun_id << 24));
	abort_entry_point = Ent_abort_sequence;
	p8xx_intr_patch(target_script, A_abort_select_failed_Used,
	    A_abort_select_failed);
    } else {
	id_bit_mask = 0x06000000;
	abort_entry_point = Ent_abdr2_sequence;
    }

    target_script[S_INDEX(Ent_abort_bdr_msg_out_buf)] = id_bit_mask;
    p8xx_intr_patch(target_script, A_abort_io_complete_Used,
	A_abort_io_complete);

    P8if(P8_DEVICE_STATUS_2,
	(p8xx_rpt_status(ap, "Exit from issue_abort_script",0)));

    /* Finally, patch the tag into the register move instructions */
    for (rc = 0; rc < S_COUNT(R_abdr_tag_patch_Used); rc++)
    {
        target_script[R_abdr_tag_patch_Used[rc]] &= LtoCBO(0xffff00ff);
        target_script[R_abdr_tag_patch_Used[rc]] |=
                        LtoCBO(((ulong) com_ptr->tag) << 8);
    }

    TRACE_1 ("iss abrt", (int) LtoCBO(target_script_b +
                                (abort_entry_point / 4)));

    /* Start the chip going */
    p8xx_start_chip(ap, NULL, (uint) target_script_b, abort_entry_point, 0, 0);

    P8printf(P8_EXIT,
	("Leaving p8xx_issue_abort_script, abort_entry_point=0x%x\n",
	abort_entry_point));
}



/************************************************************************/
/*									*/
/* NAME:	p8xx_issue_bdr_script					*/
/*									*/
/* FUNCTION:								*/
/*	This function will patch in the BDR message into the message	*/
/*	buffer and patch all interrupts within the abort_bdr script	*/
/*	to reflect that this was a bdr script executing if it ever	*/
/*	hits an interrupt because of an error.				*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine can be called by a process.			*/
/*	It can page fault only if called under a process		*/
/*	and the stack is not pinned.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*	sid_info - information on the state of the target id		*/
/*									*/
/* INPUTS:								*/
/*	*dev_ptr - address of the device info structure			*/
/*	*com_ptr - address of the command info structure		*/
/*	connected - indicates whether to try to select the device or	*/
/*		whether we are already connected to it and should just	*/
/*		try to raise ATN and send the abort message.  We start	*/
/*		the script at a different point depending on the value. */
/*									*/
/* RETURN VALUE DESCRIPTION:						*/
/*	None.								*/
/************************************************************************/
void
p8xx_issue_bdr_script(
	dvi_t	*dev_ptr,
	cmd_t   *com_ptr,
	int	connected)
{
    ads_t	*ap;
    ulong	*target_script;
    int		i;

    P8printf(P8_ENTRY|P8_ABORT, ("BDR issued for ID %d LUN %d\n",
	dev_ptr->scsi_id, dev_ptr->lun_id));

    ap = dev_ptr->ap;
    target_script = (ulong *) dev_ptr->sid_ptr->vscript_ptr;

    TRACE_1("issue bdr script", (int) connected)
    TRACE_2("id lun", (int) dev_ptr->scsi_id, (int) dev_ptr->lun_id)
    /* update wait script so that none of the LUNs can reselect us */
    p8xx_reset_iowait_jump(&ap->Scripts, dev_ptr, TRUE);

    /* force patching related to the LUN on the next command after */
    /* the BDR. */
    dev_ptr->sid_ptr->tag_flag = ap->ddi.addi.card_scsi_id << 3;
    dev_ptr->sid_ptr->lun_flag = 0xFF;

    target_script[S_INDEX(Ent_abort_bdr_msg_out_buf)] = 0x0C000000;

    /* 
     * Patch in a BDR interrupt
     */
    p8xx_intr_patch(target_script, A_abort_io_complete_Used,
	A_bdr_io_complete);
    p8xx_intr_patch(target_script, A_abort_select_failed_Used,
	A_bdr_select_failed);

    /* Finally, patch the tag into the register move instructions */
    for (i = 0; i < S_COUNT(R_abdr_tag_patch_Used); i++)
    {
        target_script[R_abdr_tag_patch_Used[i]] &= LtoCBO(0xffff00ff);
        target_script[R_abdr_tag_patch_Used[i]] |=
                        LtoCBO(((ulong) com_ptr->tag) << 8);
    }

    /* Start the chip */
    if (!connected)
        p8xx_start_chip(ap, NULL, dev_ptr->sid_ptr->bscript_ptr,
	    Ent_bdr_sequence, 0, 0);
    else
        p8xx_start_chip(ap, NULL, dev_ptr->sid_ptr->bscript_ptr,
  	    Ent_abdr2_sequence, 0, 0);

}


#ifdef	DEBUG_8XX
/*
 * NAME:	p8xx_dump_script
 *
 * DESCRIPTION:	Print a scripts buffer in hex
 *
 */

void	p8xx_dump_script(
	ulong	*script_ptr,
	int	size)
{
	int	i, d, f;

	printf("SCRIPTS dump from 0x%08x [virtual]", script_ptr);
	d = (ulong)script_ptr & 0xfff;
	f = (d) ? '/' : '-';
	for (i=0; i < size; i++, d+=sizeof(ulong)) {
	    if (!(i%8))
		/* Start a new line */
		printf("\n%03x%c%03x ",i*4,f,d);
	    printf("%08x ", (i<size-24) ? CBOtoL(script_ptr[i]) : script_ptr[i]);
	}
	printf("\n");

}
#endif

#ifdef	DEBUG_8XX
/*
 * NAME:	p8xx_rpt_status
 *
 * Function:	Prints device information/status if in debug mode
 * Input:	ads_t *ap	addr of adapter structure.
 *		char *msg	prefix string for message hdr 
 *		int flag	set to 1 to force printing of registers
 *				even if chip is currently connected to the
 *				SCSI bus (doing this may interfere with
 *				the chip's operation so be careful).
 *				If set to a 2, we will also read and
 *				print the DSTAT, SIST0 & SIST1 registers
 *				which clears interrupts!!
 * Returns:	nothing.
 */

void	p8xx_rpt_status(
	ads_t	*ap,
	char	*msg,
	int	flag)
{
	int	istat_save, i;
	struct reg_ent {
	    char	*fmt;
	    int		reg;
	    int		size;
	    int		flag;
	};
	static struct reg_ent	reg_tbl[] = {
	    "ISTAT  = 0x%02x\t\t",	ISTAT,	ISTAT_SIZE,	0,
	    "SBCL   = 0x%02x\t\t",	SBCL,	SBCL_SIZE,	1,
	    "SOCL   = 0x%02x\n",	SOCL,	SOCL_SIZE,	1,

	    /* Reading these 3 registers clears pending interrupts! */
	    "DSTAT  = 0x%02x\t\t",	DSTAT,	DSTAT_SIZE,	2,
	    "SIST0  = 0x%02x\t\t",	SIST0,	SIST0_SIZE,	2,
	    "SIST1  = 0x%02x\n",	SIST1,	SIST1_SIZE,	2,

	    "SSTAT0 = 0x%02x\t\t",	SSTAT0,	SSTAT0_SIZE,	1,
	    "SSTAT1 = 0x%02x\t\t",	SSTAT1,	SSTAT1_SIZE,	1,
	    "SSTAT2 = 0x%02x\n",	SSTAT2,	SSTAT2_SIZE,	1,
	    "SCNTL0 = 0x%02x\t\t",	SCNTL0,	SCNTL0_SIZE,	1,
	    "SCNTL1 = 0x%02x\t\t",	SCNTL1,	SCNTL1_SIZE,	1,
	    "SCNTL2 = 0x%02x\n",	SCNTL2,	SCNTL2_SIZE,	1,
	    "SCNTL3 = 0x%02x\t\t",	SCNTL3,	SCNTL3_SIZE,	1,
	    "DCNTL  = 0x%02x\t\t",	DCNTL,	DCNTL_SIZE,	1,
	    "DIEN   = 0x%02x\n",	DIEN,	DIEN_SIZE,	1,
	    "SCID   = 0x%02x\t\t",	SCID,	SCID_SIZE,	1,
	    "SDID   = 0x%02x\t\t",	SDID,	SDID_SIZE,	1,
	    "SXFER  = 0x%02x\n",	SXFER,	SXFER_SIZE,	1,
	    "SIEN0  = 0x%02x\t\t",	SIEN0,	SIEN0_SIZE,	1,
	    "SIEN1  = 0x%02x\t\t",	SIEN1,	SIEN1_SIZE,	1,
	    "DMODE  = 0x%02x\n",	DMODE,	DMODE_SIZE,	1,
	    "SFBR   = 0x%02x\t\t",	SFBR,	SFBR_SIZE,	1,
	    "DCMD   = 0x%02x\t\t",	DCMD,	DCMD_SIZE,	1,
	    "STIME0 = 0x%02x\n",	STIME0, STIME0_SIZE,	1,
	    "CTEST0 = 0x%02x\t\t",	CTEST0, CTEST0_SIZE,	1,
	    "CTEST1 = 0x%02x\t\t",	CTEST1, CTEST1_SIZE,	1,
	    "CTEST2 = 0x%02x\n",	CTEST2, CTEST2_SIZE,	1,
	    "CTEST3 = 0x%02x\t\t",	CTEST3, CTEST3_SIZE,	1,
	    "CTEST4 = 0x%02x\t\t",	CTEST4, CTEST4_SIZE,	1,
	    "CTEST5 = 0x%02x\n",	CTEST5, CTEST5_SIZE,	1,
	    "CTEST6 = 0x%02x\t\t",	CTEST6, CTEST6_SIZE,	1,
	    "GPCNTL = 0x%02x\t\t",	GPCNTL, GPCNTL_SIZE,	1,
	    "GPREG  = 0x%02x\n",	GPREG,  GPREG_SIZE,	1,
	    "MACNTL = 0x%02x\t\t",	MACNTL, MACNTL_SIZE,	1,
	    "SSID   = 0x%02x\t\t",	SSID,   SSID_SIZE,	1,
	    "ADDER  = 0x%02x\n",	ADDER,  ADDER_SIZE,	1,
	    "SLPAR  = 0x%02x\t\t",	SLPAR,  SLPAR_SIZE,	1,
	    "STIME0 = 0x%02x\t\t",	STIME0, STIME0_SIZE,	1,
	    "STIME1 = 0x%02x\t\t",	STIME1, STIME1_SIZE,	1,
	    "RESPID = 0x%02x\n",	RESPID, RESPID_SIZE,	1,
	    "STEST0 = 0x%02x\t\t",	STEST0, STEST0_SIZE,	1,
	    "STEST1 = 0x%02x\t\t",	STEST1, STEST1_SIZE,	1,
	    "STEST2 = 0x%02x\t\t",	STEST2, STEST2_SIZE,	1,
	    "STEST3 = 0x%02x\n",	STEST3, STEST3_SIZE,	1,
	    "SSIDL  = 0x%02x\t\t",	SSIDL,  SSIDL_SIZE,	1,
	    "SODL   = 0x%02x\t\t",	SODL,   SODL_SIZE,	1,
	    "SBDL   = 0x%02x\n",	SBDL,   SBDL_SIZE,	1,
	    "DSA    = 0x%08x\t",	DSA,	DSA_SIZE,	1,
	    "DBC    = 0x%08x\t",	DBC,	DBC_SIZE,	1,
	    "TEMP   = 0x%08x\n",	TEMP,	TEMP_SIZE,	1,
	    "DNAD   = 0x%08x\t",	DNAD,	DNAD_SIZE,	1,
	    "DSP    = 0x%08x\t",	DSP,	DSP_SIZE,	1,
	    "DSPS   = 0x%08x\n",	DSPS,	DSPS_SIZE,	1,
	    "SCRA   = 0x%08x\t",	SCRATCHA,SCRATCHA_SIZE,	1,
	    "SCRB   = 0x%08x\t",	SCRATCHB,SCRATCHB_SIZE,	1,
	    "ADDER  = 0x%08x\n",	ADDER,	ADDER_SIZE,	1,
	};

	istat_save = p8xx_read_reg(ap, ISTAT, ISTAT_SIZE);
	printf("%s: Device status: (Chip%s connected to SCSI bus)\n",msg,
	    (istat_save & ISTAT_CONNECTED) ? "" : " NOT");
	if (!(istat_save & ISTAT_CONNECTED) && flag == 0)
	    flag = 1;
	for (i=0; i < sizeof(reg_tbl)/sizeof(struct reg_ent); i++)
	    if (flag >= reg_tbl[i].flag) {
		io_delay(5);
		printf(reg_tbl[i].fmt,
		    p8xx_read_reg(ap, reg_tbl[i].reg, reg_tbl[i].size));
	    }

	/*
	 * If flag is 2 or greater, dump regs in hex as well
	 */
	if (flag >= 2) {
	    printf("Hex dump of registers:\n");
	    for (i=0; i <= SCRATCHB; i+=4) {
		printf("%08x ", p8xx_read_reg(ap, i, 4));
		if ((i+4) % 16 == 0)
		    printf(" %02x/%02x%s\n", i-12,i-12+P8XX_BASE_OFFSET);
	    }
	}
}
#endif

#ifdef P8_TRACE
void
p8xx_trace_1(ads_t *ap, char *string, int data)
{
    int     i;

/*
    return;
*/
    if (ap->current_trace_line >= (P8XX_TRACE_SIZE - 0x10))
        ap->current_trace_line = 1;
    for (i = 0; i < 13; i++)
        ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.one_val.header1[i] = *(string + i);
    for (i = strlen(string); i < 12; i++)
    {
        ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.one_val.header1[i] = '\0';
    }
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.one_val.data = data;
    ap->current_trace_line++;

    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[0] = '1';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[1] = '1';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[2] = '1';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[3] = '1';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[4] = 'P';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[5] = 'O';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[6] = 'I';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[7] = 'N';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[8] = 'T';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[9] = 'E';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[10] = 'R';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[11] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[12] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[13] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[14] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[15] = '*';
}
#endif

#ifdef P8_TRACE
void
p8xx_trace_2(ads_t *ap, char *string, int val1, int val2)
{
    int     i;

/*
    return;
*/
    if (ap->current_trace_line >= (P8XX_TRACE_SIZE - 0x10))
        ap->current_trace_line = 1;
    for (i = 0; i < 9; i++)
        ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.two_vals.header2[i] = *(string + i);
    for (i = strlen(string); i < 8; i++)
    {
        ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.two_vals.header2[i] = '\0';
    }
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.two_vals.data1 = val1;
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.two_vals.data2 = val2;
    ap->current_trace_line++;
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[0] = '2';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[1] = '2';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[2] = '2';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[3] = '2';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[4] = 'P';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[5] = 'O';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[6] = 'I';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[7] = 'N';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[8] = 'T';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[9] = 'E';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[10] = 'R';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[11] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[12] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[13] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[14] = '*';
    ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.header[15] = '*';
}
#endif

#ifdef P8_TRACE
void
p8xx_trace_3(ads_t *ap, char *string, int data1, int data2, int data3)
{
    int     i;

    if (ap->current_trace_line > (P8XX_TRACE_SIZE - 0x10))
    {
        ap->current_trace_line = 1;
    }
    for (i = 0; i < 5; i++)
        ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.three_vals.header3[i] = *(string + i);
/*
    for (i = strlen(string); i < 4; i++)
    {
        ap->trace_ptr->trace_buffer
         [ap->current_trace_line].un.three_vals.header3[i] = '\0';
    }
*/
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.three_vals.val1 = data1;
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.three_vals.val2 = data2;
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.three_vals.val3 = data3;
    ap->current_trace_line++;
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[0] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[1] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[2] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[3] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[4] = 'P';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[5] = 'O';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[6] = 'I';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[7] = 'N';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[8] = 'T';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[9] = 'E';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[10] = 'R';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[11] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[12] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[13] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[14] = '*';
    ap->trace_ptr->trace_buffer
        [ap->current_trace_line].un.header[15] = '*';
}
#endif
