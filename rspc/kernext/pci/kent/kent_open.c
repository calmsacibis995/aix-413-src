static char sccsid[] = "@(#)60	1.13  src/rspc/kernext/pci/kent/kent_open.c, pcient, rspc41J, 9520B_all 5/18/95 12:46:08";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: free_cdt
 *		free_services
 *		init_cdt
 *		init_services
 *		kent_cdt_add
 *		kent_cdt_del
 *		kent_cdt_func
 *		kent_open
 *		reset_adapter
 *		start_adapter
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

#include "kent_proto.h"
#include <sys/mdio.h>
#include <sys/errno.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/sleep.h>

/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;

extern struct cdt 	*p_kent_cdt;
extern int		l_kent_cdt;

/*
 * NAME: kent_open()
 *                                                                    
 * FUNCTION: Open entry point from kernel.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have 
 *	not been disabled.
 *	
 *	This routine synchronizes with the config routine (CFG_TERM in 
 *	particular) through the network services.  This routine will only be
 *	called as a result of an ns_alloc.  The first thing CFG_TERM does is
 * 	call ns_dettach which if called first will prevent any subsequent 
 *	calls to open.  If the open is first the ns_dettach will fail because
 * 	of the reference counts.
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 *		ENOCONNECT	- Indicates that the activation commands failed
 *		EINVAL		- Indicates that an invalid parameter was 
 *					specified
 *		ENETDOWN 	- Indicates that there was a pio failure.
 * 		ENOMEM		- Indicates the driver failed to get resources
 *					for the open.
 */  

int
kent_open( ndd_t *p_ndd)
{
	kent_acs_t	*p_acs = (kent_acs_t *)p_ndd->ndd_correlator;
	int 		rc = 0;
	int 		iomem;
	int		ipri;

	/* 
	 * if the driver is in a power management mode of suspend/hibernate
	 * then put the caller to sleep until the driver resumes normal operation 
	 * the open check for the state does not need to be under a lock as 
	 * this routine cannot be called while the driver is running.
	 */
	if (p_acs->dev.state == SUSPEND_STATE)
		e_sleep(&p_acs->dev.pmh_event, EVENT_SHORT);

	rc = pincode(kent_config);
	if (rc != 0)
	{
		return(ENOMEM);
	}

	KENT_ETRACE("OkoB",p_acs,0,0);

	if (p_acs->dev.state != CLOSED_STATE)
	{
		KENT_ETRACE("Oko1",p_acs->dev.state,0,0);

		/* unpin the driver code */
		unpincode(kent_config);

		return(EINVAL);
	}

	if ((rc = init_services(p_acs)) != 0)
	{
		KENT_ETRACE("Oko2",rc,0,0);
		unpincode(kent_config);
		return(ENOMEM);
	}

	if ((rc = init_tx(p_acs)) != 0)
	{
		KENT_ETRACE("Oko3",rc,0,0);
		free_services(p_acs);
		unpincode(kent_config);
		return(ENOMEM);
	}


	if ((rc = init_rx(p_acs)) != 0)
	{
		KENT_ETRACE("Oko4",rc,0,0);
		free_tx(p_acs);
		free_services(p_acs);
		unpincode(kent_config);
		return(ENOMEM);
	}

	lock_write(&p_acs->dev.dd_clock);
	if (kent_tbl.open_cnt++ == 0)
	{
		if ((rc = init_cdt()) != 0)
		{
			KENT_ETRACE("Oko5",rc,0,0);
			lock_done(&p_acs->dev.dd_clock);
			free_rx(p_acs);
			free_tx(p_acs);
			free_services(p_acs);
			unpincode(kent_config);
			return(ENOMEM);
		}
	}
	lock_done(&p_acs->dev.dd_clock);


	/* 
	 * Need to get the slih lock, start adapter assumes it is held before
	 * the adapter is started
	 */
	ipri = disable_lock(KENT_OPLEVEL, &p_acs->dev.slih_lock);

	rc = start_adapter(p_acs);

	unlock_enable(ipri, &p_acs->dev.slih_lock);
	
	if (rc != 0)
	{
		KENT_ETRACE("Oko6",rc,0,0);
		reset_adapter(p_acs);
		lock_write(&p_acs->dev.dd_clock);
		if (--kent_tbl.open_cnt == 0)
			free_cdt();
		lock_done(&p_acs->dev.dd_clock);
		free_rx(p_acs);
		free_tx(p_acs);
		free_services(p_acs);
		unpincode(kent_config);
		return(ENOCONNECT);
	}

	if (p_acs->dev.pio_rc)
	{	
		KENT_ETRACE("Oko7",rc,0,0);
		reset_adapter(p_acs);
		lock_write(&p_acs->dev.dd_clock);
		if (--kent_tbl.open_cnt == 0)
			free_cdt();
		lock_done(&p_acs->dev.dd_clock);
		free_rx(p_acs);
		free_tx(p_acs);
		free_services(p_acs);
		unpincode(kent_config);
		return(ENETDOWN);
	}

	kent_cdt_add ("kent_acs", (char *) p_acs,  sizeof (kent_acs_t));
	kent_cdt_add ("k_tx_dsc", (char *)p_acs->tx.desc, 
		(p_acs->dds.tx_que_sz * sizeof(kent_desc_t)));
	kent_cdt_add ("k_tx_buf", (char *)p_acs->tx.p_buffer,
		(p_acs->dds.tx_que_sz * KENT_BUFF_SZ));
	kent_cdt_add ("k_rx_dsc", (char *)p_acs->rx.desc, 
		(p_acs->dds.rx_que_sz * sizeof(kent_desc_t)));
	kent_cdt_add ("k_rx_buf", (char *)p_acs->rx.p_buffer,
		(p_acs->dds.rx_que_sz * KENT_BUFF_SZ));

	p_acs->ndd.ndd_flags |= NDD_UP | NDD_RUNNING;
	p_acs->dev.state = OPEN_STATE;

	KENT_ETRACE("OkoE",0,0,0);
	return(0);

} /* end kent_open() */


/*
 * NAME: init_services()
 *                                                                    
 * FUNCTION: allocates the general system resources needed by the device
 *		driver.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have 
 *	not been disabled.
 *	
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 * 		ENOMEM		- Indicates the driver failed to get resources
 *					for the open.
 */  

int
init_services( kent_acs_t *p_acs)
{
	int iomem;

	KENT_ETRACE("OisB",p_acs,0,0);

	if ((p_acs->dev.dh = D_MAP_INIT(
		BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid), 
		DMA_MASTER, 0, 0)) == (d_handle_t)DMA_FAIL)
	{
		KENT_ETRACE("Ois1",p_acs, (uint)p_acs->dev.dh, 0);
		return (ENOMEM);
	}

	if (D_MAP_PAGE(p_acs->dev.dh,DMA_READ,&p_acs->init,&p_acs->dev.p_d_init,
		xmem_global) != DMA_SUCC)
	{
		KENT_ETRACE("Ois2",p_acs, &p_acs->init,p_acs->dev.p_d_init);
		D_MAP_CLEAR(p_acs->dev.dh);
		return (ENOMEM);
	}

        lock_alloc(&p_acs->dev.slih_lock, LOCK_ALLOC_PIN,
                XKENT_SLIH_LOCK, p_acs->dev.seq_number);
        simple_lock_init(&p_acs->dev.slih_lock);

	lock_alloc(&p_acs->dev.dd_clock, LOCK_ALLOC_PIN,
		XKENT_DD_LOCK, -1);
	lock_init(&p_acs->dev.dd_clock, TRUE);

	p_acs->ihs.next = NULL;
	p_acs->ihs.handler = (int (*)())kent_slih;
	p_acs->ihs.bus_type = BUS_BID;
	p_acs->ihs.flags = INTR_MPSAFE;
	p_acs->ihs.level = p_acs->dds.busintr;
	p_acs->ihs.priority = KENT_OPLEVEL;
	p_acs->ihs.bid = BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid); 
	p_acs->ihs.i_count = 0;
	if (i_init ((struct intr *) (&p_acs->ihs)) != INTR_SUCC)
	{
		KENT_ETRACE("Ois2", p_acs, 0, 0);
		lock_free(&p_acs->dev.slih_lock);
		lock_free(&p_acs->dev.dd_clock);
		D_UNMAP_PAGE(p_acs->dev.dh, &p_acs->dev.p_d_init);
		D_MAP_CLEAR(p_acs->dev.dh);
		return (ENOMEM);
	}

        p_acs->dev.stime = lbolt;
	
	clear_stats(p_acs);

        lock_alloc(&p_acs->dev.ctl_lock, LOCK_ALLOC_PIN,
                XKENT_CTL_LOCK, p_acs->dev.seq_number);
        simple_lock_init(&p_acs->dev.ctl_lock);

	KENT_ETRACE("OisE",p_acs,0,0);
	return (0);
}


/*
 * NAME: free_services()
 *                                                                    
 * FUNCTION: frees the general system resources used by the device driver.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have 
 *	not been disabled.
 *	
 * NOTES:
 *
 * RETURNS: 
 */  

void
free_services( kent_acs_t *p_acs)
{

	KENT_ETRACE("OfsB",p_acs,0,0);

	lock_free(&p_acs->dev.ctl_lock);
	lock_free(&p_acs->dev.dd_clock);

        /* un register slih  */
        i_clear( (struct intr *)(&p_acs->ihs) );

	lock_free(&p_acs->dev.slih_lock);

	D_UNMAP_PAGE(p_acs->dev.dh, &p_acs->dev.p_d_init);

	D_MAP_CLEAR(p_acs->dev.dh);

	KENT_ETRACE("OfsE",p_acs,0,0);
	return;
}


/*
 * NAME: start_adapter()
 *                                                                    
 * FUNCTION: activates the adapter.  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This routine assumes at least the slih_lock is held when it is called.
 *	
 * NOTES:
 * 	This code should be coded genericly enough to activate the adapter
 *	from anytime between the open call and close call.  It should set up
 * 	the adapter with any addresses / modes set.
 *
 *	It is used to restart the adapter in the case of error or changes to 
 *	the adapter's registers (except the status register)
 *
 * RETURNS: 
 *		0		- successful
 * 		ENOMEM		- Indicates the driver failed to get resources
 *					for the open.
 */  

int
start_adapter( kent_acs_t *p_acs)
{
	int		i,x,tmp,rc;
	int		status;
	int		iomem;
	struct mdio	md;
	short		md_short;

	KENT_ETRACE("OsaB",p_acs,ENTSTATS.restart_count,p_acs->dev.state);

	if (p_acs->dev.state == DEAD_STATE)
		return(ENETDOWN);

	/* if this is called from any place except open */
	if (p_acs->dev.state != CLOSED_STATE)
		ENTSTATS.restart_count++;

	iomem = (int) iomem_att(&p_acs->dev.iomap);

	if (p_acs->dev.chip_rev < KENT_AMD_P2)
	{
		/* 
		 * initialize the mdio structure used to get to the 
		 * adapter's config registers
		 */
		md.md_data = (char *)&md_short;
		md.md_size = 1;
		md.md_incr = MV_SHORT;
		md.md_sla = p_acs->dds.md_sla;

		/* 
	 	 * turn off dma so that transmit does not have a problem if 
		 * it is going on right now (this is a part of the hardware 
		 * errata for the P1 chip only.  This will break later 
		 * versions.) 
		 */
		md_short = SWAP_SHORT(KENT_CFG_CMD_SERREN | 
			KENT_CFG_CMD_PERREN | KENT_CFG_CMD_IOEN);
		md.md_addr = 0x04;

		if (rc = pci_cfgrw( BID_VAL(IO_PCI,PCI_IOMEM, 
				p_acs->dds.busid), &md,1))
		{
			KENT_ETRACE("Osa1",p_acs, rc, 0);
			kent_logerr(p_acs,ERRID_KENT_ADAP_ERR,__LINE__,__FILE__,
				status, 0,0);
			iomem_det((void *)iomem);
			return (ENETDOWN);
		}

	}

	/* use the stop bit.  The soft reset may cause bad side effects (see 
	 hardware errata) */
	CSR_WRITE(iomem, 0, (KENT_CSR0_IDON | KENT_CSR0_STOP));

	BCR_WRITE(iomem, 2, KENT_BCR2_ASEL);

	CSR_READ(iomem, 112, &tmp); /* get the missed frame count */
	ENTSTATS.no_resources += ADDR_LO(tmp);

	/* call receive handler to prevent us from loosing any receive */
	/*  currently on the queue, the receive descriptors will be reset */
	/*  later in the routine */
	rx_handler(p_acs);

	/* 
	 * call transmit complete to clean up any transmits which are done to
	 * prevent loss of statistics
	 */
	tx_complete(p_acs);

	/* make sure the pci style is set */
	BCR_WRITE(iomem, 20, KENT_BCR20_SWSTYLE);

	/* turn off the following interrupts */
	CSR_WRITE(iomem, 3, (KENT_CSR3_MERRM|KENT_CSR3_TINTM|
			KENT_CSR3_EMBA));
	CSR_WRITE(iomem, 4, (KENT_CSR4_TXSTRTM| KENT_CSR4_APAD_XMT | 
		KENT_CSR4_ASTRP_RCV | KENT_CSR4_JABM | KENT_CSR4_DMAPLUS));

	CSR_READ(iomem, 80, &tmp);
	CSR_WRITE(iomem, 80, tmp | 0xC00);

	/* clear the status register of any old values before re-init starts */
	CSR_READ(iomem, 0, &status);
	CSR_WRITE(iomem, 0, (status & KENT_CSR0_MASK));

	/*
	 * The driver needs to get the log base 2 of the length of the
	 * transmit and receive queues.  To do this the driver will loop
  	 * through dividing the length until it reaches 1 dividing by 
	 * two along the way.  It is guaranteed to be a power of 2.
	 */
	x = p_acs->dds.tx_que_sz/16;
	i = 4;
	while (x != 1)
	{
		x = x/2;
		i++;
	}

	/* 	
	 * add the length of the transmit queue to the mode field (see 
	 * the adapter's spec for the format of the mode field) 
	 */
	p_acs->init.mode = i << KENT_TLEN_MODE;

	if (p_acs->dev.chip_rev < KENT_AMD_P2)
	{
		if (p_acs->init.mode == 0x80000000)
			p_acs->init.mode = 0xC0000000;
	}

	x = p_acs->dds.rx_que_sz/16;
	i = 4;
	while (x != 1)
	{
		x = x/2;
		i++;
	}
	p_acs->init.mode |= i << KENT_RLEN_MODE;

	if (p_acs->dev.chip_rev < KENT_AMD_P2)
	{
		if (p_acs->init.mode & 0x00800000)
			p_acs->init.mode |= 0x00C00000;
	}

	if (p_acs->ndd.ndd_flags & NDD_PROMISC)
		p_acs->init.mode |= KENT_CSR15_PROM;

	p_acs->init.mode = SWAP_LONG(p_acs->init.mode);

	if (p_acs->dds.use_alt_addr)
	{
		bcopy(p_acs->dds.alt_addr, &p_acs->init.padr1, 6);
	}
	else
	{
		bcopy(p_acs->addrs.src_addr, &p_acs->init.padr1, 6);
	}

	if (p_acs->ndd.ndd_flags & NDD_MULTICAST)
	{
		for (i=0; i<KENT_LADRF_LEN; i++)
			p_acs->init.ladrf[i] = 0xff;
			
	}
	else 
	{
		for (i=0; i<KENT_LADRF_LEN; i++)
			p_acs->init.ladrf[i] = p_acs->addrs.ladrf[i];
	}

	p_acs->init.tdra = SWAP_LONG(p_acs->tx.p_d_desc);
	p_acs->init.rdra = SWAP_LONG(p_acs->rx.p_d_desc);

	CSR_WRITE(iomem, 1, (ADDR_LO(p_acs->dev.p_d_init)));
	CSR_WRITE(iomem, 2, (ADDR_HI(p_acs->dev.p_d_init)));
	CSR_WRITE(iomem, 0, (KENT_CSR0_INIT));

	if (p_acs->dev.chip_rev < KENT_AMD_P2)
	{
		/*
	 	 * Now that the reinit is setup, allow the adapter to dma again
		 * Note: This is only needed for the P1 chips, it will break
		 * the P2 chips.
	 	 */

		md_short = SWAP_SHORT(KENT_CFG_CMD_SERREN | 
			KENT_CFG_CMD_PERREN | KENT_CFG_CMD_BMEN | 
			KENT_CFG_CMD_IOEN);
		md.md_addr = 0x04;

		if (rc = pci_cfgrw( BID_VAL(IO_PCI,PCI_IOMEM, 
			p_acs->dds.busid), &md,1))
		{
			KENT_ETRACE("Osa2",p_acs, rc, 0);
			kent_logerr(p_acs,ERRID_KENT_ADAP_ERR,__LINE__,__FILE__,
				status, 0,0);
			iomem_det((void *)iomem);
			return (ENETDOWN);
		}
	}

	CSR_READ(iomem, 0, &status);
	
	/*
	 * The i variable is set up to prevent an infinite loop in the driver
	 * from tying up the machine forever.  The 1 million number is 
	 * completely arbitrary, the driver needs to poll for the completion 
	 * but does not, in the event of no response, want to tie up the whole
  	 * machine.
	 */
	i = 0;
	while (!(status & KENT_CSR0_IDON))
	{
		if (i++ == 10)
		{
			KENT_ETRACE("Osa3",p_acs, status, 0);
			kent_logerr(p_acs,ERRID_KENT_ADAP_ERR,__LINE__,__FILE__,
				status, 0,0);
			iomem_det((void *)iomem);
			return (ENETDOWN);
		}
		DELAYMS(10);
		CSR_READ(iomem,0,&status);
	}

	/*
	 * reset the receive descriptors anew
	 */
	undo_rx(p_acs);

	/*
	 * reset the receive descriptors anew
	 */
	undo_tx(p_acs);

	if (p_acs->dev.chip_rev >= KENT_AMD_P2)
	{
		BCR_READ(iomem, 18, &tmp);
		tmp |= KENT_BCR18_BREADE | KENT_BCR18_BWRITE;
		BCR_WRITE(iomem, 18, tmp);
	}

	CSR_WRITE(iomem, 0, (KENT_CSR0_IENA| KENT_CSR0_STRT));

	CSR_READ(iomem, 0, &status);

	if (!(status & KENT_CSR0_TXON) || !(status & KENT_CSR0_RXON) ||
		!(status & KENT_CSR0_IENA))
	{
		KENT_ETRACE("Osa4",p_acs, status, 0);
		kent_logerr(p_acs,ERRID_KENT_ADAP_ERR,__LINE__, __FILE__,
			status, 0,0);
		iomem_det((void *)iomem);
		return (ENETDOWN);
	}

	iomem_det((void *)iomem);

	KENT_ETRACE("OsaE",p_acs,0,0);
	return (0);
}


/*
 * NAME: reset_adapter()
 *                                                                    
 * FUNCTION: resets the adapter.  This function is called when the adapter
 * must be halted and will not be immediately re-started.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	
 * NOTES:
 *
 * RETURNS: 
 */  

void
reset_adapter( kent_acs_t *p_acs)
{
	int		i,x,rc;
	int		status;
	int		iomem;
	struct mdio	md;
	short		md_short;

	KENT_ETRACE("OrsB",p_acs,0,0);

	iomem = (int) iomem_att(&p_acs->dev.iomap);
	
	if (p_acs->dev.chip_rev == KENT_AMD_P1)
	{
		/* 
	 	 * turn off dma so that transmit does not have a problem if 
		 * it is going on right now (this is a part of the hardware 
		 * errata for the P1 chip only.  This will break later 
		 * versions.) 
		 */
		md_short = SWAP_SHORT(KENT_CFG_CMD_SERREN | 
			KENT_CFG_CMD_PERREN | KENT_CFG_CMD_IOEN);
		md.md_addr = 0x04;

		pci_cfgrw( BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid), &md,1);
	}
	

	/* use the stop bit.  The soft reset may cause bad side effects (see 
	 amd errata) */
	CSR_WRITE(iomem, 0, (KENT_CSR0_IDON | KENT_CSR0_STOP));

	iomem_det((void *)iomem);
	KENT_ETRACE("OrsE",p_acs,0,0);
	return;
}

/*
 * NAME: init_cdt
 *                                                                    
 * FUNCTION: initialize component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *	0 	if successful
 *	ENOMEN 	if no memory available
 */  

int
init_cdt ()
{
	KENT_ETRACE("OicB",0,0,0);
	/* 
	 * allocate dump table with initial size
	 */
	p_kent_cdt = (struct cdt *) xmalloc(sizeof(struct cdt_head) + 
		(KENT_CDT_ENTRIES * sizeof(struct cdt_entry)), 
		KENT_WORDSHIFT, pinned_heap);

	if (p_kent_cdt == NULL)
	{
		KENT_ETRACE("Oic1",0,0,0);
		/* fail */
		return (ENOMEM);
	}
	/* 
	 * initialize component dump table 
	 * l_kent_cdt is the total size of the allocated dump table
	 * cdt_len is the current size of the actual entries in the sump table
	 */
	l_kent_cdt = KENT_CDT_ENTRIES;
	p_kent_cdt->cdt_magic = DMP_MAGIC;
	bcopy (KENT_DD_STR, p_kent_cdt->cdt_name, sizeof(KENT_DD_STR));
	p_kent_cdt->cdt_len = sizeof (struct cdt_head);

	/* 
	 * add static data structures: trace table and kent control struct
	 *	also, add the allocated open table at this time.
	 */
	kent_cdt_add ("kent_tbl", (char *) &kent_tbl,  sizeof (kent_tbl));

	/* 
	 * register for dump 
	 */
	dmp_add (((void(*)())kent_cdt_func));

	KENT_ETRACE("OicE",p_kent_cdt,p_kent_cdt->cdt_len,l_kent_cdt);
	/* ok */
	return (0);
}

/*
 * NAME: free_cdt
 *                                                                    
 * FUNCTION: remove the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *
 * RETURNS: none
 */  

void
free_cdt ()
{
	KENT_ETRACE("OfcB",0,0,0);
	/* Delete global structures */
	kent_cdt_del ((char *) &kent_tbl);

	dmp_del (((void(*)())kent_cdt_func));
	xmfree(p_kent_cdt, pinned_heap);
	p_kent_cdt = NULL;
	l_kent_cdt = 0;
	
	KENT_ETRACE("OfcE",0,0,0);
	return;
}

/*
 * NAME: kent_cdt_add
 *                                                                    
 * FUNCTION: add an entry to the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: none
 */  

void
kent_cdt_add (
	char	*p_name,
	char	*ptr,
	int	len)
{
	int		rc;
	int		num_entries;
	int		new_size;
	struct cdt	*p_old;
	struct cdt	*p_new;

	 
	KENT_ETRACE("OcaB",NUM_ENTRIES(p_kent_cdt),0,0);
	/* get number of entries */
	num_entries = NUM_ENTRIES (p_kent_cdt);

	if (num_entries == l_kent_cdt)
	{
		/* 
		 * enlarge the entries by KENT_CDT_ENTRIES
		 */
		new_size = sizeof (struct cdt_head) + 
			((l_kent_cdt + KENT_CDT_ENTRIES) * 
			sizeof (struct cdt_entry));

		/* grow dump table */
		p_new = (struct cdt *) xmalloc (new_size, KENT_WORDSHIFT, 
						pinned_heap);

		if (p_new == NULL)
		{
			/* 
			 * can't expand dump table drop entry
			 */
			KENT_ETRACE("cca1",0,0,0);
			return;
		}
		/* 
		 * initialize new dump table with old values
		 * (cdt_len is copied as well)
		 */
		bcopy (p_kent_cdt, p_new, p_kent_cdt->cdt_len);

		/* 
		 * swap curent (old) dump table with new 
		 * 	Notice p_kent_cdt will always have a valid 
 		 *	pointer to avoid errors if the dump function
		 *	were called during the swap
		 */
		p_old = p_kent_cdt;
		p_kent_cdt = p_new;
		l_kent_cdt += KENT_CDT_ENTRIES;
		xmfree (p_old, pinned_heap);

	}
	/*
	 * Add entry 
	 */
	bcopy (p_name, p_kent_cdt->cdt_entry[num_entries].d_name, 
			sizeof (((struct cdt_entry *) 0)->d_name));
	p_kent_cdt->cdt_entry[num_entries].d_len = len;
	p_kent_cdt->cdt_entry[num_entries].d_ptr = ptr;
	p_kent_cdt->cdt_entry[num_entries].d_segval = 0;
	p_kent_cdt->cdt_len += sizeof (struct cdt_entry);


	KENT_ETRACE("OcaE",NUM_ENTRIES(p_kent_cdt),0,0);
	/* ok */
	return ;
}
/*
 * NAME: kent_cdt_del
 *                                                                    
 * FUNCTION: deletes an entry to the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: none
 */
void
kent_cdt_del (
	char	*ptr)
{
	int	i;
	int	rc;
	int	num_entries;


	KENT_ETRACE("OcdB",NUM_ENTRIES(p_kent_cdt),0,0);
	/*
	 * find entry that matches this ptr 
	 */
	num_entries = NUM_ENTRIES(p_kent_cdt);
	for (i = 0; i < num_entries; i++)
	{
		if (ptr == p_kent_cdt->cdt_entry[i].d_ptr)
		{
			/* found it */
			break;
		}
	}
	if (i < num_entries)
	{
		/* 
		 * re-pack entries 
		 */
		for ( ; i < num_entries; i++)
		{
			p_kent_cdt->cdt_entry[i] = p_kent_cdt->cdt_entry[i+1];
		}
		p_kent_cdt->cdt_len -= sizeof (struct cdt_entry);
	}


	KENT_ETRACE("OcdE",NUM_ENTRIES(p_kent_cdt),0,0);
	/* ok */
	return ;
}

/*
 * NAME: kent_cdt_func
 *
 * FUNCTION: component dump function called at dump time
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *
 * RETURNS: ptr to the current component dump table
 */

struct cdt *
kent_cdt_func(
        int     pass )
{
        return (p_kent_cdt);
}

