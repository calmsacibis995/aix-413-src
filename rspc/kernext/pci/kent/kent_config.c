static char sccsid[] = "@(#)50	1.7  src/rspc/kernext/pci/kent/kent_config.c, pcient, rspc41J, 9521A_all 5/22/95 18:35:30";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: init_acs
 *		kent_config
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
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/mdio.h>
#include <sys/dump.h>
#include <sys/sleep.h>

/* 
 * Declare the device driver table structue.  This is the major global 
 * structure for the kent device driver.  It contains the internal trace 
 * table, the config lock, and a header to the list of acs structures.  
 */
kent_tbl_t	kent_tbl = 
{
	LOCK_AVAIL,	/* table_lock : locks config */
	0,		/* acs_cnt */
	0,		/* open_cnt */
	0,		/* acs pointer */
	0,		/* trace_lock */
	{
		0,		/* reserved */
		0,		/* reserved */
		0,		/* reserved */
	        0,              /* trace.next */
        	0x4b454e54,     /* "KENT" */
        	0x54524143,     /* "TRAC" */
        	0x4554424c,     /* "ETBL" */
        	0x21212121      /* "!!!!" */
	}
};

/*
 * kent component dump table objects:
 */

struct cdt	*p_kent_cdt = NULL;
int		l_kent_cdt = 0;

/*
 * NAME: kent_config()
 *                                                                    
 * FUNCTION: Handles the CFG_INIT, CFG_TERM, CFG_QVPD sysconfig cmds
 *                                                                    
 * EXECUTION ENVIRONMENT:
 * 	This routine executes on the process thread only.  It assumes that
 * 	interrupts have not been disabled.
 *                                                                   
 * NOTES:
 *	
 *
 * RETURNS: 
 *	CFG_INIT:
 *		0	- Successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		EIO 	- Indicates a permanent I/O error
 *		EBUSY	- Device is already configured
 *		ENOMEM	- Unable to allocate required memory
 *
 *	CFG_TERM:
 *		0	- Successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		EIO 	- Indicates a permanent I/O error
 *		EBUSY	- Device is open configured
 *		ENODEV	- Indicates that the device is not configured
 *
 *	CFG_QVPD:
 *		0	- successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		ENODEV 	- Indicates a permanent I/O error
 *
 */  

int
kent_config(int cmd,		
	    struct uio *p_uio)	
{
	kent_acs_t	*p_acs;
	ndd_config_t 	ndd_cfg;
	int		rc;
	int		i;
	int		tmp;
	int 		iomem;
	kent_acs_t	*tmp_acs;
	char		vpd[100];
	char		*p_vpd;

	/*
	 * Lock to prevent other config calls from running.  This must be a 
	 * lockl because other locks need calls to lock alloc and lock free 
	 * and these calls would need to be locked. 
	 */
	rc = lockl(&kent_tbl.table_lock, LOCK_SHORT);

	if ( rc != LOCK_SUCC )
                return(EINVAL);

	rc = pincode(kent_config);
	if (rc != 0)
	{
	 	unlockl(&kent_tbl.table_lock);
		return(ENOMEM);
	}


	if ( kent_tbl.acs_cnt == 0)
	{
                lock_alloc(&kent_tbl.trace_lock, LOCK_ALLOC_PIN,
                        XKENT_TRACE_LOCK, -1);

                simple_lock_init(&kent_tbl.trace_lock);
	}

	KENT_ETRACE("ckcB", cmd, kent_tbl.acs_cnt, 0);

	if (rc = uiomove ((caddr_t)&ndd_cfg, (int)sizeof(ndd_config_t), 
		UIO_WRITE, p_uio))
	{
		KENT_ETRACE("ckc1", cmd, rc, p_uio); 
		if (kent_tbl.acs_cnt == 0)
		{
			lock_free(&kent_tbl.trace_lock);
		}
	
		unpincode(kent_config);
	 	unlockl(&kent_tbl.table_lock);
		return(ENOMEM);
	}
		

	switch(cmd)
	{
		case CFG_INIT:
		{
			/* 
			 * Check to see if this number has already been 
			 * allocated.  This is probably unnecessary 
			 * as the config method should also be checking.
			 */
			p_acs = kent_tbl.p_acs;
			while (p_acs != 0)
			{
				if (p_acs->dev.seq_number == 
					ndd_cfg.seq_number)
				{
					KENT_ETRACE("cki1",ndd_cfg.seq_number, 
						p_acs, 0);
			 		rc = EBUSY;
					break;
				}
				p_acs = p_acs->next_acs;
			}	

			if (rc != 0)
				break;

			/* 
	 		 * get ACS memory from the pinned heap, page aligned
	 		 */
			p_acs = xmalloc(sizeof(kent_acs_t), KENT_LONGSHIFT, 
					pinned_heap );	
	
			if ( p_acs == NULL )
			{
				KENT_ETRACE("cki2",0,0,0);
				rc = ENOMEM;
				break;
			}
	
			bzero(p_acs, sizeof(kent_acs_t) );

			if ((rc = copyin(ndd_cfg.dds, &p_acs->dds, 
				sizeof(kent_dds_t))) != 0)
			{
				KENT_ETRACE("cki3",rc,0,0);
				xmfree(p_acs, pinned_heap);
				rc = EINVAL;
				break;
			}

		 	KENT_ETRACE("cdds","busi","d   ",p_acs->dds.busid);
		 	KENT_ETRACE("cdds","busi","ntr ",p_acs->dds.busintr);
		 	KENT_ETRACE("cdds","busi","o   ",p_acs->dds.busio);
		 	KENT_ETRACE("cdds","md_s","la  ",p_acs->dds.md_sla);
		 	KENT_ETRACE("cdds","lnam","e   ",
				*(ulong *)&p_acs->dds.lname[0]);
		 	KENT_ETRACE("cdds","alia","s   ",
				*(ulong *)&p_acs->dds.alias[0]);
		 	KENT_ETRACE("cdds","tx_q","ue_s",p_acs->dds.tx_que_sz);
		 	KENT_ETRACE("cdds","rx_q","ue_s",p_acs->dds.rx_que_sz);
		 	KENT_ETRACE("cdds","use_","alt_",
				p_acs->dds.use_alt_addr);
		 	KENT_ETRACE("cdds","alt_a","ddr ",
				*(ulong *)&p_acs->dds.alt_addr[0]);
		 	KENT_ETRACE("cdds", *(ulong *)&p_acs->dds.alt_addr[3],
				0,0);
			/*
			 * Initialize all of the one time values of the acs.
			 */
			init_acs(p_acs);

			p_acs->dev.seq_number = ndd_cfg.seq_number;

			/* initialize the one time configurations on the adapter*/

			if (rc = init_adapter(p_acs))
			{
				KENT_ETRACE("cki4",rc,0,0);
				xmfree(p_acs, pinned_heap);
				rc = EIO;
				break;
			}
		

			/* 
			 * With the driver configured, need to register 
			 * power management services 
			 */

			if (pm_register_handle(&p_acs->pmh, PM_REGISTER) == 
				PM_ERROR)
			{
				KENT_ETRACE("cki5",rc,0,0);
				xmfree(p_acs, pinned_heap);
				rc = EIO;
				break;
			}
						
			if ((rc = ns_attach(&p_acs->ndd)) != 0)
			{
				KENT_ETRACE("cki6",rc,0,0);
				pm_register_handle(&p_acs->pmh, PM_UNREGISTER);
				xmfree(p_acs, pinned_heap);
				rc = EIO;
				break;
			}

			p_acs->next_acs = 0;
			if (kent_tbl.p_acs == 0)
			{
				kent_tbl.p_acs = p_acs;
			}
			else
			{
				tmp_acs = kent_tbl.p_acs;
				while (tmp_acs->next_acs != 0)
					tmp_acs = tmp_acs->next_acs;
				tmp_acs->next_acs = p_acs;
			}
			kent_tbl.acs_cnt++;

		} /* end CFG_INIT */	
		break;

		case CFG_QVPD:
		{

			/* 
			 * find the acs structure for the adapter.	
			 */
			tmp_acs = kent_tbl.p_acs;
			while (tmp_acs != 0)
			{
				if (tmp_acs->dev.seq_number == 
					ndd_cfg.seq_number)
				{
					p_acs = tmp_acs;
					break;	
				}
				tmp_acs = tmp_acs->next_acs;
			}
			if (tmp_acs == 0)
			{
				KENT_ETRACE("ckq1",ndd_cfg.seq_number,0,0);
				rc = ENODEV;
				break;
			}
			
			/* 
			 * PCI adapters do not inheriently have VPD, so the 
			 * driver will fake it with the information desired
			 * by our users.
			 */
			p_vpd = vpd;
			strncpy(p_vpd,"*NA",3);
			p_vpd += 3;
			*p_vpd = (char)0x05;
			p_vpd += 1;
			bcopy(p_acs->addrs.src_addr,p_vpd,ENT_NADR_LENGTH);
			p_vpd += ENT_NADR_LENGTH;
			strncpy(p_vpd,"*DS",3);
			p_vpd += 3;
			*p_vpd = (char)0x12;
			p_vpd += 1;

			/*
			 * Note: the vpd needs to be 0 terminated.  The strcpy
			 *  will put a zero at the end of the string, but need 
			 *  to include it in the length (p_vpd += 35).
			 */
			strcpy(p_vpd,"PCI Ethernet Adapter (22100020)");
			p_vpd += 35;

			if (rc = copyout(vpd, ndd_cfg.p_vpd, (int)(p_vpd-vpd)))
			{
				KENT_ETRACE("ckq2", cmd, rc, p_uio); 
				rc = EINVAL;	
				break;
			}

		} /* end CFG_QVPD */
		break;

		case CFG_TERM:
		{
			/*
			 * get the acs structure for the adapter.
			 */
			tmp_acs = kent_tbl.p_acs;
			while (tmp_acs != 0)
			{
				if (tmp_acs->dev.seq_number == 
					ndd_cfg.seq_number)
				{
					p_acs = tmp_acs;
					break;	
				}
				tmp_acs=tmp_acs->next_acs;
			}
			if (tmp_acs == 0)
			{
				KENT_ETRACE("ckt1",ndd_cfg.seq_number,0,0);
				rc = ENODEV;
				break;
			}
			
			/*
			 * attempt to detach.  This needs to done first to 
			 * check for any possible opens active on the adapter
			 * and to prevent any future ones if one is not active
			 * currently.
			 */
			if ((rc = ns_detach (&p_acs->ndd)) != 0)
			{
				KENT_ETRACE("ckt2",rc,0,0);
				rc = EBUSY;
				break;
			}

			pm_register_handle(&p_acs->pmh, PM_UNREGISTER);

			/* remove the acs address from the acs chain */
			if (kent_tbl.p_acs == p_acs)
			{
				kent_tbl.p_acs = kent_tbl.p_acs->next_acs;
			}
			else
			{
				tmp_acs = kent_tbl.p_acs;
				while (tmp_acs->next_acs != p_acs)
					tmp_acs = tmp_acs->next_acs;
				tmp_acs->next_acs = 
					tmp_acs->next_acs->next_acs;
			}

			xmfree( p_acs, pinned_heap );

			kent_tbl.acs_cnt--;
			break;
		}  /* end case CFG_TERM */
		break;

		default:
			rc = EINVAL;

	} /* end switch (cmd) */

	KENT_ETRACE("ckcE", cmd, kent_tbl.acs_cnt, rc);

	if (kent_tbl.acs_cnt == 0)
	{
		lock_free(&kent_tbl.trace_lock);
	}

	unpincode(kent_config);
	unlockl(&kent_tbl.table_lock);

	return(rc);

} /* end kent_config() */


/*
 * NAME: init_acs
 *                                                                    
 * FUNCTION: Initialize the ACS structure.  Services are allocated in 
 * 		init_services.  This routine is intended to just initialize 
 *		the acs the first time.  init_services will also initialize 
 *		those variables which need to be reset on close and such.
 *		init_tx, init_rx will take care of the receive and transmit
 *		parts of the acs themselves (they are only called in 
 *		kent_open).
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *		
 */  
void
init_acs(kent_acs_t *p_acs)
{
	KENT_TRACE("ciaB",p_acs, 0,0);

	p_acs->ndd.ndd_correlator = (char *)p_acs;
	p_acs->ndd.ndd_name = p_acs->dds.lname;
	p_acs->ndd.ndd_alias = p_acs->dds.alias;
	p_acs->ndd.ndd_flags = NDD_BROADCAST | NDD_SIMPLEX;
#ifdef FDDI_DEBUG
	p_acs->ndd.ndd_flags |= NDD_DEBUG;
#endif
	p_acs->ndd.ndd_open = kent_open;
	p_acs->ndd.ndd_close = kent_close;
	p_acs->ndd.ndd_output = kent_output;
	p_acs->ndd.ndd_ctl = kent_ctl;
	p_acs->ndd.ndd_mtu = ENT_MAX_MTU;
	p_acs->ndd.ndd_mintu = ENT_MIN_MTU;
	p_acs->ndd.ndd_type = NDD_ISO88023;
	p_acs->ndd.ndd_addrlen = ENT_NADR_LENGTH;
	p_acs->ndd.ndd_hdrlen = KENT_HDR_LEN;
	if (p_acs->dds.use_alt_addr)
		p_acs->ndd.ndd_physaddr = p_acs->dds.alt_addr;
	else
		p_acs->ndd.ndd_physaddr = p_acs->addrs.src_addr;
	p_acs->ndd.ndd_specstats = (caddr_t) &p_acs->ls.ent_gen_stats;
	p_acs->ndd.ndd_speclen = sizeof (p_acs->ls.ent_gen_stats);
	
	p_acs->dev.pio_rc = FALSE;

	p_acs->dev.iomap.key = IO_MEM_MAP;
	p_acs->dev.iomap.bid = BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid); 
	p_acs->dev.iomap.busaddr = p_acs->dds.busio;
	p_acs->dev.iomap.size = KENT_IO_SIZE; 
	p_acs->dev.iomap.flags = 0;

	/* 
	 * initialize the power management structure.
	 */
	p_acs->pmh.activity = 0;
	p_acs->pmh.mode = PM_DEVICE_FULL_ON;
	p_acs->pmh.device_idle_time = 0;
	p_acs->pmh.device_standby_time = 0;
	p_acs->pmh.idle_counter = 0;
	p_acs->pmh.handler = (int(*)(unsigned char*,int))kent_pmh;
	p_acs->pmh.private = (caddr_t) p_acs;
	p_acs->pmh.devno = 0;
	p_acs->pmh.attribute = 0;
	p_acs->pmh.next1 = 0;
	p_acs->pmh.next2 = 0;
	p_acs->pmh.device_idle_time1 = 0;
	p_acs->pmh.device_idle_time2 = 0;
	p_acs->pmh.device_logical_name = p_acs->dds.lname;
	p_acs->pmh.reserve[0] = '\0';
	p_acs->pmh.reserve[1] = '\0';
	p_acs->pmh.pm_version = 0x0100; 	/* Hardcoded as defined by */
						/*  Power Management */
	p_acs->pmh.extension = 0;
	
	p_acs->dev.pmh_event = EVENT_NULL;

	p_acs->dev.state = CLOSED_STATE;

	KENT_TRACE("ciaE",p_acs, 0,0);
	return;
}


/*
 * NAME: init_adapter
 *                                                                    
 * FUNCTION: initializes the adapter.  Performs those actions on the adapter 
 * 	that only need to be done once, such as configuration of the adapter's
 * 	configuration registers and reading its APROM.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *		
 */  
int
init_adapter(kent_acs_t *p_acs)
{
	struct mdio	md;
	int		md_data;
	short		md_short;
	int		tmp;
	int 		iomem;
	int 		rc;

	KENT_TRACE("cidB",p_acs, 0,0);

	/* 
	 * initialize the adapter's configuration space.  Put
	 * the bus io addr down to the adapter for access to 
	 * its registers.
	 */
	md.md_data = (char *)&md_data;
	md.md_size = 1;
	md.md_incr = MV_WORD; /* 4 byte increments */
	md.md_sla = p_acs->dds.md_sla;

	md_data = SWAP_LONG(p_acs->dds.busio);
	md.md_addr = 0x10;

	rc = pci_cfgrw(BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid), &md, 1);

	if (rc)
	{
		KENT_ETRACE("cid1",p_acs,rc, p_acs->dds.md_sla);
		return(EINVAL);
	}

	/*
	 * Set up the adapter's command field.  This needs to
	 * be done in order to access anything beyond the 
	 * configuration space on the adapter.
	 */
	md_short = SWAP_SHORT(KENT_CFG_CMD_SERREN|KENT_CFG_CMD_PERREN|
		KENT_CFG_CMD_BMEN| KENT_CFG_CMD_IOEN);
	md.md_data = (char *)&md_short;
	md.md_incr = MV_SHORT;
	md.md_addr = 0x04;

	rc = pci_cfgrw( BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid), &md, 1);

	if (rc)
	{
		KENT_ETRACE("cid2",p_acs,rc,md_short);
		return(EINVAL);
	}

			
	md.md_data = &p_acs->dev.chip_rev;
	md.md_incr = MV_BYTE;
	md.md_addr = 0x08;

	rc = pci_cfgrw( BID_VAL(IO_PCI,PCI_IOMEM, p_acs->dds.busid), &md, 0);

	if (rc)
	{
		KENT_ETRACE("cid3",p_acs,rc,md_short);
		return(EINVAL);
	}

	iomem = (int)iomem_att(&p_acs->dev.iomap);

	/* Need to set the DWIO mode early to be able to */
	/* do anything correctly later.  Best way to do  */
	/* this is to do a 4byte write to offset 0x10,   */
	/* but we do not want to erase anything by mistake */
	/* so will read from the location before writing */

	PIO_GETLRX(iomem + KENT_IO_RDP, &tmp);
	PIO_PUTLRX(iomem + KENT_IO_RDP, tmp);

	/* Need to get the adapter's burned in source address.  */
	PIO_GETLRX(iomem + KENT_IO_APROM1, &tmp);
	*(int *)&p_acs->addrs.src_addr[0] = SWAP_LONG(tmp);
	PIO_GETLRX(iomem + KENT_IO_APROM2, &tmp);
	*(short *)&p_acs->addrs.src_addr[4] = (short)(ADDR_HI(SWAP_LONG(tmp)));

	BCR_READ(iomem, 19, &tmp);

	iomem_det((void *)iomem);

	if ((tmp & KENT_BCR19_EEDET) && (!(tmp & KENT_BCR19_PVALID)))
	{
		KENT_ETRACE("cid4",rc,0,0);
		return(EIO);
	}

	KENT_TRACE("cidE",p_acs, 0,0);
	return(0);
}
