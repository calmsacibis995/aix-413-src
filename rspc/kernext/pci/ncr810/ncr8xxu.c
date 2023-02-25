static char sccsid[] = "@(#)52  1.3  src/rspc/kernext/pci/ncr810/ncr8xxu.c, pciscsi, rspc41J, 9516A_all 4/14/95 15:16:20";
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: PATCH_ENTRY
 *		p8xx_TIM_build
 *		p8xx_TIM_print
 *		p8xx_alloc_adp
 *		p8xx_del_adp
 *		p8xx_free_adp
 *		p8xx_get_adp
 *		p8xx_get_leftovers
 *		p8xx_hexdump
 *		p8xx_intr_patch
 *		p8xx_negotiate
 *		p8xx_select_period
 *		p8xx_set_adp
 *		p8xx_start_abort_timer
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
/* NAME:	ncr8xxu.c						*/
/*									*/
/* FUNCTION:	IBM SCSI Adapter (53C8XX) Driver Utilities              */
/*									*/
/*	This is a collection of utilities functions used by the NCR	*/
/*	53C8XX SCSI chip adapter driver.               			*/
/*									*/
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/xmem.h>
#include <sys/adspace.h>
#include <sys/dump.h>
#include <sys/intr.h>
#include <sys/watchdog.h>
#include <sys/scsi.h>
#include <sys/syspest.h>
#include <sys/pm.h>
#include "ncr8xx.h"
/* END OF INCLUDED SYSTEM FILES	 */


/* include SCRIPTS header stuff specific to this file */
#include "ncr8xxuss.h"


/*
 * Table of interrupt instructions that we patch.  This table references the
 * ``where used array'' for each interrupt instruction in the SCRIPTS that
 * we need to patch.  It is used by the p8xx_intr_patch() function.
 *
 * Note:	The three interrupts: illegal_target_id, unknown_reselect_id,
 *		and uninitialized_reselect will not be patched because these
 *		cases represent ones in which we do not have a script
 *		associated with a device.  Since we don't recognize the device,
 *		it will not exist in the dev_info hash table that the
 *		top_half_word is used for.  Thus, it is not included because
 *		it is not a valid assignment-patch to the interrupts.
 */

/* This macro makes the table easier to read and reduces the risk of errors */
#define	PATCH_ENTRY(lbl)	lbl, S_COUNT(lbl)

#ifdef P8_TRACE
hash_t	hash_list[ADS_HASH_MASK+2];
#else
hash_t	hash_list[ADS_HASH_MASK+1];
#endif

static struct itbl_s {
	ulong	*used;		/* points to array of offsets where used */
	int	count;		/* nbr of entries in the array */
} itbl[] = {
	PATCH_ENTRY(A_phase_error_Used),
	PATCH_ENTRY(A_io_done_Used),
	PATCH_ENTRY(A_unknown_msg_Used),
	PATCH_ENTRY(A_sync_msg_reject_Used),
	PATCH_ENTRY(A_check_next_io_Used),
	PATCH_ENTRY(A_ext_msg_Used),
	PATCH_ENTRY(A_cmd_select_atn_failed_Used),
	PATCH_ENTRY(A_err_not_ext_msg_Used),
	PATCH_ENTRY(A_neg_select_failed_Used),
	PATCH_ENTRY(A_abort_select_failed_Used),
	PATCH_ENTRY(A_abort_io_complete_Used)
};

/* our debug flag */
#ifdef	DEBUG_8XX
extern	int	p8xx_debug;		/* controls debug msgs */
extern	int	p8xx_dbsv[P8XX_SVMAX];	/* save areas for debug flags */
extern	int	p8xx_dbidx;		/* current save area index */
#endif



/*
 * Name:	p8xx_TIM_build
 *
 * Function:	Builds a scatter/gather list table in the format required
 *		by the NCR 53C8XX SCSI chip for Table Indirect Block Moves
 *		from input buffer address and length parameters.
 *
 *		The format of a table entry is:
 *
 *		     least sig                      most sig
 *		    +---------------------------------------+
 *		0   | cnt1    | cnt2    | cnt3    | xxxx    |
 *		    |---------------------------------------|
 *		4   | addr1   | addr2   | addr3   | addr4   |
 *		    +---------------------------------------+
 *
 *		Notes:	1. The entries in the table are in Little Endian byte
 *			   order.
 * 			2. The addresses are bus addresses.
 *		
 * Returns:	Number of entries in the table or -1 on error.
 */

int	p8xx_TIM_build(
	ads_t		*ap,
        struct cmd_info *com_ptr,
	int		flags,		/* DMA access flags */
	char		*buf,		/* address of the buffer */
	int		bufl,		/* its length */
	struct xmem	*dp,		/* xmem descriptor for buffer */
	tim_t		*tbl,		/* ptr to where to build the table */
	int		tbln)		/* size of table area in # of
					   entries */
{
	int	tu;
	int	rc;
	d_iovec_t	b;

	P8printf(P8_ENTRY,("Entering p8xx_TIM_build routine.\n"));
	P8printf(P8_INFO_2,
	    (" flags=0x%02x, buf=%x, bufl=%d, dp=%x, tbl=%x, tbln=%d\n",
	    flags, buf, bufl, dp, tbl, tbln));
	TRACE_2("TIMbld", (int) buf, (int) bufl)
	TRACE_2("TIMbld", (int) tbl, (int) tbln)

	ap->vlist.used_iovecs = 1;
	ap->vlist.dvec->iov_base = buf;
	ap->vlist.dvec->iov_len = bufl;
	ap->vlist.dvec->xmp = dp;
	ap->blist.total_iovecs = tbln;
	ap->blist.dvec = (struct d_iovec *) ap->blist_base_vector + 
		         com_ptr->resource_index;
	TRACE_1("blist.dvec", (int) ap->blist.dvec)
	rc = D_MAP_LIST(ap->handle, flags, 1, &ap->vlist,
		&ap->blist);
	if ((rc == DMA_DIOFULL) || (rc == DMA_NORES)) {
	    /*
	     * This should never happen since our blist is guaranteed
	     * big enough
	     */
	    TRACE_1("bad maplist", rc)
	    assert(0);
	} else if (rc == DMA_NOACC) {
	    /*
	     * Encountered a page violation
	     */
	    P8printf(P8_EXIT,("Leaving p8xx_TIM_build, rc=%d\n", -1));
	    TRACE_1("bad maplist", rc)
	    return(-1);
	}
	
	/* 
	 * The early versions of the device driver contain unbuilt code
	 * that will break larger than MAX_TIMLEN and off MAX_TIMLEN
	 * boundary entries into nice boundary/MAX_TIMLEN chunks.  The
	 * thought was that if a target decides to disconnect it would
	 * be good to do it on a TIM entry boundary since the SCRIPTS
	 * can handle that kind of disconnect w/o generating an
	 * interrupt back to the device driver.  Phase mis-matches
	 * during a transfer mandate an interrupt, and therefore, we
	 * attempt to avoid them.  The current code remains optimized
  	 * for targets that disconnect after 4K transfers.
	 */

#ifdef P8_TRACE
	/* due to contiguous pages, there are some free iovecs */
	if (ap->blist.used_iovecs != ap->blist.total_iovecs) {
	    TRACE_2("used < tl", ap->blist.used_iovecs,
		    ap->blist.total_iovecs);
	}
#endif
	/*
	 * Our chip expects the TIM entries to be Little Endian
         * Copy the address/length pairs to the data area that will be
	 * read by the chip.
	 */
        com_ptr->tim_tbl_used = ap->blist.used_iovecs;
	for (tu=0; tu < ap->blist.used_iovecs; tu++) {
	    b = &ap->blist.dvec[tu];
	    tbl[tu].tim_len = (ulong)LtoCBO(b->iov_len);
	    tbl[tu].tim_adr = (ulong)LtoCBO(b->iov_base);
	}

	P8if(P8_INFO_3, (p8xx_TIM_print(tbl, tu)));

	P8printf(P8_EXIT,("Leaving p8xx_TIM_build, rc=%d\n", tu));
	TRACE_1("out TIMbld", tu)
	return(tu);			/* we return number of entries
					   we used from the table */
}


#ifdef	DEBUG_8XX
/*
 * Name:	p8xx_TIM_print
 * Function:	Print out the TIM
 * Input:	tim_t *tbl	pointer to TIM to be printed
 *		int tbln	size of the table (used entries)
 * Returns:	nothing.
 */

void	p8xx_TIM_print(
	tim_t	*tbl,
	int	tbln)
{
	int j;
	printf("Printing TIM at 0x%x\n", tbl);
	for (j=0; j < tbln; j++)
	    printf("TIM[%d]: len=0x%08x, addr=0x%08x\n", j,
		CBOtoL(tbl[j].tim_len), CBOtoL(tbl[j].tim_adr));
}
#endif


/*
 * Name:	p8xx_get_adp
 * Function:	locate address of adapter structure from devno
 * Input:	dev_t devno	major/minor number
 * Returns:	pointer to adapter_def struct or NULL if none.
 */


ads_t	*p8xx_get_adp(
	dev_t	devno)
{
	ads_t		*ap;
	hash_t 		*hap;
	int		i;

#ifdef P8_TRACE
     for (i=0;i<ADS_HASH_MASK+1;i++)
     {
	P8if(P8_INFO_3, (printf("%d next %08x, ap %08x, devno %08x\n", i,
		hash_list[i].next, hash_list[i].ap, hash_list[i].devno)));
     }
#endif
	i = devno & ADS_HASH_MASK;
	for (hap = &hash_list[i]; hap != NULL; hap = hap->next) {
	    if (hap->devno == devno)
		break;
	}

	if (hap != NULL) {
	    P8Tprintf(("p8xxu: <-> get_adp: devno %08x", devno));
	    return(hap->ap);
	} else {
	    P8Tprintf(("p8xxu: <-> get_adp: devno %08x ap not found", devno));
	    return(NULL);
	}
}


/*
 * Name:	p8xx_set_adp
 * Function:	Save the address of adapter structure in the devno hash list
 * Input:	dev_t devno	major/minor number
 *		ads_t *ap	pointer to adapter structure
 * Returns:	0 if successfull, -1 if failure
 */

int	
p8xx_set_adp(
	dev_t	devno,
	ads_t	*ap)
{
	hash_t 		*hap, *nhap;
	int		i;

	P8Tprintf(("p8xxu: <-> set_adp: devno %08x ap %08x", devno, ap));
	i = devno & ADS_HASH_MASK;
	hap = (hash_t *) &hash_list[i];
	/* most of the time it will be the initial entry... */
	if (hap->ap == NULL) {
	    hap->ap = ap;
	    hap->devno = devno;

	} else { 
	    /* allocate and init new element */
	    if ((nhap = (hash_t *) xmalloc((uint)sizeof(hash_t), (uint) 2,
	        pinned_heap)) == NULL)
	        return(-1);
	    nhap->next = NULL;
	    nhap->devno = devno;
	    nhap->ap = ap;

	    /* Put new element into hash list */
	    while (hap->next != NULL) {
	        hap = hap->next;
	    }
	    hap->next = nhap;
	}

	P8Tprintf(("p8xxu: <-> set_adp: devno %8.8x nhap %x", devno, nhap));

	return(0);
}


/*
 * Name:	p8xx_del_adp
 * Function:	Removes an adapter structure from the devno hash list
 * Input:	dev_t devno	Major/minor numbers for device
 *		ads_t ap	Pointer to the adapter structure
 * Output:	0 if successful, -1 if adapter structure was not on the
 *		list.
 */

int	p8xx_del_adp(
	dev_t	devno,
	ads_t	*ap)
{
	hash_t 		*hap, *ohap;
	int		i;

	i = devno & ADS_HASH_MASK;
	ohap = NULL;
	for (hap = &hash_list[i]; hap->ap != NULL; ohap = hap, hap = hap->next) {
	    if ((hap->devno == devno) || (hap->next == NULL))
		break;
	}
	if (hap->devno == devno) {		/* found it */
	    if (ohap == NULL) {
		if (hap->next == NULL) {        /* first and only */
	            hap->ap = NULL;
		    hap->devno = 0;
		} else {			/* first of several */
						/* copy next element forward */
		    hap->ap = hap->next->ap;
		    hap->devno = hap->next->devno;
		    ohap = hap->next;
		    hap->next = ohap->next;
	            (void) xmfree((void *) ohap, pinned_heap);
		}
	    } else { 				/* not removing first element */
	        ohap->next = hap->next;
	        (void) xmfree((void *) hap, pinned_heap);
	    }
	    P8Tprintf(("p8xxu: del_adp: devno &08x hap %x", devno, hap));
	    return(0);
	} else {			/* not found! */
	    P8Tprintf(("p8xxu: del_adp: devno &08x hap not fnd", devno));
	    return(-1);
	}
}


/*
 * Name:	p8xx_alloc_adp
 * Function:	Allocate & initialize an adapter structure
 * Input:	dev_t devno	device major/minor numbers
 * Output:	pointer to new adapter structure or NULL if malloc fails
 */

ads_t	*
p8xx_alloc_adp(
	dev_t	devno)
{
	ads_t	*ap;

	P8Tprintf(("p8xxu: --> alloc_adp"));

	if (ap = (ads_t *) xmalloc((uint) sizeof(ads_t), 8, pinned_heap)) {

	    bzero((char *)ap, sizeof(ads_t));	/* we want it all zeros */

	    /* Interrrupt structures must point to our adapter structure */
	    ap->intr_struct.ap = ap;
	    ap->epow_intr_struct.ap = ap;
	    ap->pmh.private = (caddr_t) ap;
	    P8Tprintf(("p8xxu: --> xmalloc_adp"));

	    /* Create devno hash entry */
	    if (p8xx_set_adp(devno, ap) != 0) {
		(void) xmfree((void *) ap, pinned_heap);
		ap = NULL;
	    }

	} else {

	    P8Tprintf(("p8xxu: devno %8.8x xmalloc failed", devno));

	}

	P8Tprintf(("p8xxu: <-- alloc_adp"));
	return(ap);		/* return ap (or NULL if failure) */
}


/*
 * Name:	p8xx_free_adp
 * Function:	Free an adapter structure and remove it from the devno
 *		hash list.
 * Input:	ads_t *ap	Pointer to adapter structure to be freed.
 * Output:	none
 */

void	p8xx_free_adp(
	ads_t	*ap)
{
	(void) p8xx_del_adp(ap->devno, ap);
	(void) xmfree((void *) ap, pinned_heap);
	P8Tprintf(("p8xxu: free_adp: freed"));
}


/*
 * Name:	p8xx_select_period
 * Function:	Calculates the proper 53C8xx registers settings for
 *		a given transfer period.
 * Input:	ads_t *ap	Pointer to adapter structure.
 *		int   xferpd	Desired transfer period in 4ns units.
 *		uchar *scntl3	Address of where to store SCNTL3 value.
 *		uchar *sxfer	Address of where to store SXFER value.
 * Returns:	Closest available transfer period (in 4ns units) that is not
 *		greater than the requested value; or zero if the requested
 *		period is outside of our available range (too slow).
 */

int	
p8xx_select_period(
	ads_t	*ap,
	int	xferpd,
	uchar	*scntl3,
	uchar	*sxfer)
{
    	int	scfval,
		scfout,
		ccfval,
		xferval,
		xferpdiv,
		result_xferpd;

	 TRACE_2("pd clk", xferpd, ap->scsi_clk);

	/* Set the CCF & SCF value dependant upon the SCSI clock value */
	if (XFER_VAL(xferpd) < XFER_SLOW) {	/* FAST */

	    if (ap->scsi_clk < 20) {		/* GT 50MHz? */
		ccfval = SCNTL3_CCF_DIV3;	/* divide by 3 */
		scfval = SCNTL3_SCF_DIV15;	/* divide by 1.5 */
		scfout = 15;			/* scaled up by 10 */
	    } else {
		ccfval = SCNTL3_CCF_DIV2;	/* divide by 2 */
		scfval = SCNTL3_SCF_DIV1;	/* divide by 1 */
		scfout = 10;			/* scaled up by 10 */
	    }

	} else {				/* SLOW */

	    if (ap->scsi_clk < 20) {		/* clk > 50MHz */
		ccfval = SCNTL3_CCF_DIV3;	/* divide by 3 */
		scfval = SCNTL3_SCF_DIV3;	/* divide by 3 */
		scfout = 30;			/* scaled up by 10 */
	    } else if (ap->scsi_clk < 27) {	/* 50MHz >= clk > 37MHz */
		ccfval = SCNTL3_CCF_DIV2;	/* divide by 2 */
		scfval = SCNTL3_SCF_DIV2;	/* divide by 2 */
		scfout = 20;			/* scaled up by 10 */
	    } else if (ap->scsi_clk < 40) {	/* 36MHz >= clk > 25MHz? */
		ccfval = SCNTL3_CCF_DIV15;	/* divide by 1.5 */
		scfval = SCNTL3_SCF_DIV15;	/* divide by 1.5 */
		scfout = 15;			/* scaled up by 10 */
	    } else {				/* 25MHz >= clk */
		ccfval = SCNTL3_CCF_DIV1;	/* divide by 1 */
		scfval = SCNTL3_SCF_DIV1;	/* divide by 1 */
		scfout = 10;			/* scaled up by 10 */
	    }

	}

	scfout *= ap->scsi_clk;			/* scaled period of SCF
						   divider output */

	TRACE_2("ccf scfv", ccfval, scfval)
	TRACE_1("scfout", scfout)
	
	xferval = XFER_VAL(xferpd) * 10;	/* convert to ns & scale */
	xferpdiv = xferval / scfout;		/* our transfer period
						   divisor */
        if (xferval % scfout)          /* increase if not an exact match */
	    xferpdiv++;

	TRACE_2("xfv xpdv", xferval, xferpdiv)

	if (xferpdiv < SXFER_XFERP_MIN ||
	    ((scfout * xferpdiv) / 10) < XFER_FASTEST) {

	    /* Target wants to run too fast */
	    TRACE_1("tgt too fast", 0)
	    xferpdiv = XFER_FASTEST * 10 / scfout;  /* set to max we support */
	    if ((XFER_FASTEST * 10)% scfout)	    /* be conservative */
		xferpdiv++;

	} else if (xferpdiv > SXFER_XFERP_MAX) {

	    TRACE_1("tgt too slow", 0)
	    return(0);				/* Too slow */

	}

	/* Set output variables */
	*scntl3 = scfval | ccfval;
	*sxfer = ((xferpdiv - SXFER_XFERP_OFS) << SXFER_XFERP_SHIFT);

	/* We return the resulting transfer period in 4ns units */
	result_xferpd = XFER_PRD((scfout * xferpdiv) / 10);

	TRACE_3("slpd", result_xferpd, *scntl3, *sxfer)
	return(result_xferpd);

}

/*
 * Name:	p8xx_start_abort_timer
 * Function:	Starts a watchdog timer to time an abort operation.
 * Input:	dvi_t	*dev_ptr	address of device structure
 *		int	force		If TRUE, forces a restart of the
 *					timer.
 * Returns:	nothing
 */

void	
p8xx_start_abort_timer(
	dvi_t	*dev_ptr,
	int	force)
{
	/*
	 * The timer is started only for aborts that have not
	 * previously been issued.  Otherwise, the current timer is
	 * left running.
	 */
	if ((force || dev_ptr->flags & RETRY_ERROR) && 
	    (!dev_ptr->ap->dump_started)) {
	    W_STOP(&dev_ptr->dev_watchdog.dog);
	    dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
	    dev_ptr->flags &= ~RETRY_ERROR;
	    dev_ptr->retry_count = 1;
	    W_START(&dev_ptr->dev_watchdog.dog);
	} else {
	    dev_ptr->retry_count++;
	}
}

/*
 * Name:	p8xx_intr_patch
 * Description:	Patch an interrupt value into the int instructions in
 *		the specified SCRIPTS.
 * Input:	ulong	*tgt	addr of SCRIPTS to be patched
 *		ulong	*ety	addr of entry to process
 *		int	intr	interrupt value to patch in
 * Returns:	nothing.
 */

void	p8xx_intr_patch(
    ulong	*tgt,
    ulong	*ety,
    int		intr)
{

	int	i, j,
		patched = FALSE;
	ulong	data;

	P8printf(P8_INFO_4, ("intr_patch: tgt=%x, ety=%x, intr=%04x\n",
	    tgt, ety, intr));

	for (j=0; !patched && j < sizeof(itbl)/sizeof(struct itbl_s); j++)
	    for (i=0; i < itbl[j].count; i++)
		if (ety == itbl[j].used) {
		    data = (CBOtoL(tgt[itbl[j].used[i]]) & 0xFFFF0000) | intr;
		    tgt[itbl[j].used[i]] = LtoCBO(data);
		    patched = TRUE;
		}

	ASSERT(patched);

}


/*
 * Name:	p8xx_get_leftovers
 * Description:	Retrieves/calculates left over byte count for the current
 *		block move operation from the NCR chip.
 * Notes:	chip is assumed on.
 * Input:	dvi_t	*dev_ptr	addr of the device info structure
 * Output:	int	*move_kind      value indicating scsi phase
 * Returns:	The leftover byte count or -1 if the chip wasn't last
 *		executing a table indirect addressed block move instruction.
 */

int	
p8xx_get_leftovers(
    dvi_t	*dev_ptr,
    int		*move_kind)
{
    ads_t	*ap;
    int		dma_fifo_bytes, dbc_bytes, left_over_bytes;
    int		xfer_kind;
    uchar	dcmd;
    uint	dsa_val;
    uchar	sstat0_sav, sstat1_sav, sstat2_sav;

    ap = dev_ptr->ap;
    TRACE_1("get_leftovers", (int) ap)

    /* Get residual byte count & command code from current block move */
    dbc_bytes = p8xx_read_reg(ap, DBC_DCMD, DBC_DCMD_SIZE);
    dcmd = dbc_bytes >> 24;	/* isolate just command */
    dbc_bytes &= 0x00ffffff;	/* and byte count */

    /* Calculate bytes left in the DMA FIFO */
    dma_fifo_bytes = (((p8xx_read_reg(ap, (uint) DFIFO,
			(char) DFIFO_SIZE) & 0x7F) -
			(dbc_bytes & 0x7F)) & 0x7F);
    left_over_bytes = dma_fifo_bytes + dbc_bytes;

    /*
     * Ensure that the NCR chip was executing a block move instruction
     */

    if ((dcmd & DCMD_TYPE_MASK) == DCMD_BLK_MOVE) 
	/* 
	 * identify the kind of move 
	 */
	xfer_kind = (int) dcmd & DCMD_SCSI_MASK;
    else  {
        /* just set it to a value indicating no byte recovery is needed */
	left_over_bytes = 0;
	xfer_kind = (int) DCMD_RSVD_IN;
	TRACE_1("not block", (int) dcmd)
    }

    switch (xfer_kind) {
        case DCMD_CMD:
	/*
	 * The target should not leave the Command phase prematurely.
	 * The number of bytes not transferred is not important, as an
	 * error has occurred and the command bytes will eventually be
	 * resent from the beginning.
         */
	    break;

	/*
	 * May need to correct our residual count due to bytes that are still
	 * in the chip.
	 */
	case DCMD_DATA_OUT:
	case DCMD_MSG_OUT:
	    /*
	     * Account for bytes possibly left in the SODL & SODR registers.
	     */
	    sstat0_sav = p8xx_read_reg(ap, SSTAT0, SSTAT0_SIZE);
	    if (sstat0_sav & SSTAT0_OLF)		/* SODL */
		left_over_bytes++;
	    if ((sstat0_sav & SSTAT0_ORF)		/* SODR */
		&& !(dev_ptr->sid_ptr->async_device))
		left_over_bytes++;

	    /* 820/825s have two byte SODL & SODR registers */
	    switch (ap->ddi.chip_type) {
	    case IS_A_53C820:
	    case IS_A_53C825:
		sstat2_sav = p8xx_read_reg(ap, SSTAT2, SSTAT2_SIZE);
		if (sstat2_sav & SSTAT2_OLF)		/* SODL */
		    left_over_bytes++;
		if ((sstat2_sav & SSTAT2_ORF)		/* SODR */
		    && !(dev_ptr->sid_ptr->async_device))
		    left_over_bytes++;
		break;
	    default:			/* default is 810 */
		break;
	    }	

	    TRACE_2("dbc dfifo", dbc_bytes, dma_fifo_bytes)
	    break;

        case DCMD_DATA_IN:     /* receive data command */
	
	    /*
	     * If synchronous, add any bytes in the SCSI FIFO, as 
	     * indicated by bits 7-4 of SSTAT1.  If asynchronous,
	     * add any bytes in SIDL.  SWIDE should be included too,
	     * although the assumption is that it should always be
 	     * empty given our script.
	     */
	    if (!dev_ptr->sid_ptr->async_device) 
		left_over_bytes += 
		        (p8xx_read_reg(ap, SSTAT1, SSTAT1_SIZE) >> 4);
	    else {
		sstat0_sav = p8xx_read_reg(ap, SSTAT0, SSTAT0_SIZE);
		if (sstat0_sav & SSTAT0_ILF)
		    left_over_bytes++;

                /* 820/825s have a two byte SIDL register */
                switch (ap->ddi.chip_type) {
                case IS_A_53C820:
                case IS_A_53C825:
                    sstat2_sav = p8xx_read_reg(ap, SSTAT2, SSTAT2_SIZE);
                    if (sstat2_sav & SSTAT2_ILF)
                        left_over_bytes++;
                    break;
                default:                    /* default is 810 */
                    break;
                }
	    }

	    /* check for a byte in SWIDE, as indicated by SCNTL2 */
	    if (p8xx_read_reg(ap, SCNTL2, SCNTL2_SIZE) & SCNTL2_WSR) {
		left_over_bytes++;
	    }
	    break;

	default:
	    left_over_bytes = -1;
	    TRACE_1("unknown move", xfer_kind)
    }

    TRACE_2("getleft", xfer_kind, left_over_bytes)
    *move_kind = xfer_kind;
    return(left_over_bytes);
}


/*
 * Name:	p8xx_negotiate
 * Description:	This function is used to determine if a synchronous
 *		negotiation is required and if so, will perform the
 *		necessary setup for it.
 * Input:	dvi_t	*dev_ptr	device structure pointer.
 *		struct sc_buf *bp	buf struct pointer.
 * Note:	The common way to call this function is as an argument to the
 *		p8xx_start_chip function (to provide the scripts entry offset
 *		value).
 * Returns:	Scripts entry offset at which to start the chip.
 */

int	
p8xx_negotiate(
	cmd_t	*com_ptr,
	struct sc_buf *bp)
{
	ads_t	*ap;
	dvi_t	*dev_ptr = com_ptr->device;
	int	script_ofs;

	ap = dev_ptr->ap;
	TRACE_2("negotiate", (int) dev_ptr->scsi_id,  (int)dev_ptr->lun_id)
	P8printf(P8_INFO_1,("entering p8xx_negotiate, ID %x,%x.\n",
	    dev_ptr->scsi_id, dev_ptr->lun_id));

	if (!(dev_ptr->sid_ptr->negotiate_flag)) {

	    /*
	     * Negotiation not required, SCRIPT already patched.
	     */
	    dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	    script_ofs = Ent_scripts_entry_point;

	} else if ((bp->scsi_command.flags & SC_ASYNC) &&
		   (ap->ddi.chip_type == IS_A_53C810)) {
	    /*
	     * SC_ASYNC requires us to avoid SDTR negotiation.  It is
	     * currently not supported for the NCR825, since its use
	     * as coded in the config methods would cause us to configure
	     * some narrow/sync devices as narrow/async devices.
             * If SC_ASYNC is supported, we patch the SCRIPT for 
	     * async transfers.  The transfer width is narrow.
	     */
	    TRACE_1("SC_ASYNC", (int) bp)
	    P8printf(P8_SYNC, ("Forced ASYNC\n"));
	    p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);
	    dev_ptr->sid_ptr->async_device = TRUE;
	    dev_ptr->sid_ptr->negotiate_flag = FALSE;
	    dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	    script_ofs = Ent_scripts_entry_point;

	} else {

	    uint    *target_script = dev_ptr->sid_ptr->vscript_ptr;

	    /*
	     * Negotiation is required for this target.  Negotiation
	     * is done in ASYNC.
	     */
	    TRACE_1("must negotiate",(int) dev_ptr->sid_ptr);
	    P8printf(P8_SYNC, ("Must negotiate\n"));
	    p8xx_mode_patch(ap, ASYNC_MODE, dev_ptr);
	    com_ptr->flags |= NEGOT_CMD;

	    if (dev_ptr->sid_ptr->io_table.flags & AVOID_WDTR) {
		TRACE_1("sdtr", (int) dev_ptr->sid_ptr->io_table.flags)
	        dev_ptr->cmd_activity_state = SDTR_IN_PROGRESS;
		/* patch jump for sync negotiation */
		target_script[S_INDEX(Ent_negotiation_phase_patch)] = 
		    		LtoCBO(P8XX_JUMP_PATCH); 
	    }
	    else {
	        dev_ptr->cmd_activity_state = NEGOTIATE_IN_PROGRESS;
		/* patch jump to nop, do wide negotiation first */
		target_script[S_INDEX(Ent_negotiation_phase_patch)] = 
		    		LtoCBO(P8XX_NOP_PATCH); 
	    }
	    /* we start the script at the same place for either wdtr or sdtr */
	    script_ofs = Ent_begin_negotiation;
	}

	P8printf(P8_INFO_1,("leaving p8xx_negotiate, script_ofs=%x\n",
	    script_ofs));
	TRACE_1("out negot", (int) script_ofs)
	return(script_ofs);
}


#ifdef	DEBUG_8XX
/*
 * Name:	p8xx_hexdump
 * Description:	Prints a range of storage out in hex
 * Input:	char	*hdr	String to print as part of header.
 *		char	*data	Data to be hex printed.
 *		int	size	Its size.
 * Note:	Compiled in when debugging only.
 * Returns:	nothing
 */

void	
p8xx_hexdump(
	char	*hdr,
	char	*data,
	int	size)
{

/* Macro to print ascii column */
#define	PASCII(d,i,j)						\
	{							\
	    printf("  *");					\
	    for (;j < i; j++)					\
		printf("%c",(d[j] >= ' ' && d[j] < 0x7f ) ?	\
		    d[j] : '.');				\
	    printf("*\n");					\
	}

	int	i, j, k;

	printf("Hexdump: %s (%x bytes @ %x)\n", hdr,size,data);

	/* Print the lines */
	for (i=0, j=0; i < size; i++) {
	    if ((k = (i%16)) == 0) {	/* 16 chars/line */
		/* Begin a new line */
		if (i != 0)
		    PASCII(data,i,j);	/* finish last line */
		printf("%04x  ",i);
	    }
	    if (i && k && (i % 4) == 0)
		printf(" ");		/* extra space between words */
	    printf("%02x", data[i]);
	}

	/* Deal with a short last line */
	if (k != 0) {	
	    /* Position to where the ASCII is to go */
	    for (k++; k < 16; k++) {
		if ((k%4) == 0)
		    printf("   ");
		else
		    printf("  ");
	    }
	    PASCII(data,i,j);
	}

}
#endif	/* DEBUG_8XX */
