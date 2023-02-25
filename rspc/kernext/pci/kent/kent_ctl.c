static char sccsid[] = "@(#)52	1.5  src/rspc/kernext/pci/kent/kent_ctl.c, pcient, rspc41J, 9519B_all 5/10/95 15:51:42";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: add_status
 *		calculate_flag
 *		calculate_ladrf
 *		clear_stats
 *		del_status
 *		disable_address
 *		enable_address
 *		get_all_stats
 *		get_stats
 *		kent_ctl
 *		mib_addr
 *		mib_get
 *		mib_query
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
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/cdli.h>
#include <sys/dump.h>
#include <sys/mbuf.h>
#include <sys/sleep.h>
/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: kent_ctl()
 *                                                                    
 * FUNCTION: ctl entry point from kernel.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have 
 *	not been disabled.
 *	
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 *		ENETDOWN	- Indicates that the device is permanently down.
 * 		EINVAL		- Indicates that an invalid parameter was 
 * 					specified.
 *		ENOMEM		- Unable to allocate required memory
 *		E2BIG		- Indicates that the structure passed to the
 *					device driver was not large enough to 
 *					hold all the addresses.
 */  

int
kent_ctl( 
	ndd_t *p_ndd,
        int             cmd,
        caddr_t         arg,
        int             len)

{
	kent_acs_t	*p_acs = (kent_acs_t *)p_ndd->ndd_correlator;
	int 		rc = 0;
	int 		iomem;
	int		adapter_flag = FALSE;  	/* This flag is set if the */
						/* adapter needs to be reset */
						/* as a result of the ctl cmd */
	int		ipri;

	KENT_TRACE("IkcB",p_acs,cmd,arg);
	ipri = disable_lock(KENT_OPLEVEL, &p_acs->dev.ctl_lock);

	/* 
	 * if the driver is in a power management mode of suspend/hibernate
	 * then put the caller to sleep until the driver resumes normal operation 
	 * except for open this needs to be done under a lock so that the state
	 * check is valid
	 */
	if (p_acs->dev.state == SUSPEND_STATE)
		e_sleep_thread (&p_acs->dev.pmh_event, 
			&p_acs->dev.ctl_lock, LOCK_HANDLER);

	
	switch (cmd)
	{
	case NDD_GET_STATS:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikc1",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}
		rc = get_stats(p_acs,arg,len);
		break;

	case NDD_GET_ALL_STATS:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikc2",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}
		rc = get_all_stats(p_acs,arg,len);
		break;

	case NDD_CLEAR_STATS:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikc3",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}
		rc = clear_stats(p_acs);
		break;


	case NDD_ENABLE_ADDRESS:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikc4",p_acs, cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}

		rc = enable_address(p_acs, arg, len, &adapter_flag);

		break;

	case NDD_DISABLE_ADDRESS:
		rc = disable_address(p_acs, arg, len, &adapter_flag);
		break;

	
	case NDD_ENABLE_MULTICAST:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikc6",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}
		
		if (p_acs->dev.multi_cnt++ == 0)
		{
			p_acs->ndd.ndd_flags |= NDD_MULTICAST;
			adapter_flag = TRUE;
		}
		break;

	case NDD_DISABLE_MULTICAST:
		if (p_acs->dev.multi_cnt > 0)
		{
			if (--p_acs->dev.multi_cnt == 0)
			{
				p_acs->ndd.ndd_flags &= ~NDD_MULTICAST;
				adapter_flag = TRUE;
			} 
		}
		else
		{
			KENT_ETRACE("Ikc7",p_acs, 0, p_acs->dev.multi_cnt);
			rc = EINVAL;
		}
		break;

	case NDD_PROMISCUOUS_ON:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikc8",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}
		
		if (p_acs->dev.prom_cnt++ == 0)
		{
			p_acs->ndd.ndd_flags |= NDD_PROMISC;
			adapter_flag = TRUE;
		}
		break;

	case NDD_PROMISCUOUS_OFF:
		if (p_acs->dev.prom_cnt > 0)
		{
			if (--p_acs->dev.prom_cnt == 0)
			{
				p_acs->ndd.ndd_flags &= ~NDD_PROMISC;
				adapter_flag = TRUE;
			} 
		}
		else
		{
			KENT_ETRACE("Ikc9",p_acs, 0, p_acs->dev.prom_cnt);
			rc = EINVAL;
		}
		break;

	case NDD_MIB_QUERY:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikca",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}

		rc = mib_query(p_acs, arg,len);
		break;

	case NDD_MIB_GET:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikcb",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}

		rc = mib_get(p_acs, arg,len);
		break;

	case NDD_ADD_STATUS:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikcc",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}

		rc = add_status(p_acs, arg, len);
		break;

	case NDD_DEL_STATUS:
		rc = del_status(p_acs, arg,len);
		break;

	case NDD_MIB_ADDR:
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_ETRACE("Ikcd",p_acs,cmd, p_acs->dev.state);
			rc = ENETDOWN;
			break;
		}

		rc = mib_addr(p_acs, arg,len);
		break;

	case NDD_DUMP_ADDR:
		*(int *)arg = (int)kent_dump;
		break;

	default :
		rc = EOPNOTSUPP;
		break;

	};

	unlock_enable(ipri, &p_acs->dev.ctl_lock);

	if ((adapter_flag) && (p_acs->dev.state != DEAD_STATE))
	{
		ipri = disable_lock(KENT_OPLEVEL,&p_acs->dev.slih_lock);
		disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);
		start_adapter(p_acs);
		unlock_enable(KENT_OPLEVEL, &p_acs->tx.lock);
		unlock_enable(ipri, &p_acs->dev.slih_lock);
	}

	if (p_acs->dev.pio_rc)
	{
		KENT_ETRACE("Ikce",p_acs, cmd, 0);
		ipri = disable_lock(KENT_OPLEVEL,&p_acs->dev.slih_lock);
		bugout(p_acs);
		unlock_enable(ipri, &p_acs->dev.slih_lock);
	}

	KENT_TRACE("IkcE",p_acs,rc,0);
	return(rc);
} /* end kent_ctl() */


/*
 * NAME: get_stats
 *
 * FUNCTION: get the statistics for the user
 *		The statistics area is not valid until this command is run.
 *		The driver will only keep up 1 instance of a statistic when
 *		there is duplicate statistics for a single event.  Then as 
 *		a result of this command it will copy the contents of the 
 *		duplicate values to each of the duplicated statistics.
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES:
 *      Check the size of the area, fail if insufficent size
 *
 * RETURNS:
 *      0       - Success
 *      EINVAL  - Insufficient size for the statistics
 */
int
get_stats (
        kent_acs_t      *p_acs,
        ent_ndd_stats_t *stats,
        int             len)
{
        int     rc;
	int 	tmp;
	int 		iomem;

	KENT_TRACE("IgsB",p_acs, stats, len);

	if (len != sizeof(ent_ndd_stats_t))
	{
		KENT_ETRACE("Igs1",p_acs, len, sizeof(ent_ndd_stats_t));
		return(EINVAL);
	}

	/* copies the values the driver has been keeping up */
	bcopy(&NDDSTATS, &stats->ent_ndd_genstats, sizeof(ndd_genstats_t));
	bcopy(&ENTSTATS, &stats->ent_ent_genstats, sizeof(ent_genstats_t));

	stats->ent_ndd_genstats.ndd_elapsed_time = (lbolt-p_acs->dev.stime)/HZ;

	/* begin copying the duplicated statistics. */
	stats->ent_ndd_genstats.ndd_xmitque_cur = p_acs->tx.in_use;
	stats->ent_ent_genstats.hw_txq_len = p_acs->tx.in_use;

	stats->ent_ent_genstats.device_type = ENT_KEN_PCI;

	stats->ent_ent_genstats.dev_elapsed_time = 
		(lbolt-p_acs->dev.stime)/HZ;

	stats->ent_ent_genstats.ndd_flags = p_acs->ndd.ndd_flags;
	
	if (p_acs->dds.use_alt_addr)
	{
		bcopy(p_acs->dds.alt_addr, stats->ent_ent_genstats.ent_nadr, 6);
	}
	else
	{
		bcopy(p_acs->addrs.src_addr, 
			stats->ent_ent_genstats.ent_nadr,6);
	}

	
	iomem = (int) iomem_att(&p_acs->dev.iomap);

	CSR_READ(iomem, 112, &tmp); /* get the missed frame count */
	stats->ent_ent_genstats.no_resources += ADDR_LO(tmp);
	iomem_det((void *)iomem);

	return(0);
}

	
/*
 * NAME: get_all_stats
 *
 * FUNCTION: get all of the statistics for the user
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES:
 *      Check the size of the area, fail if insufficent size
 *
 * RETURNS:
 *      0       - Success
 *      EINVAL  - Insufficient size for the statistics
 */
int
get_all_stats (
        kent_acs_t      *p_acs,
        kent_all_stats_t *stats,
        int             len)
{
        int     rc;

	KENT_TRACE("IgaB",p_acs, stats, len);

	if (len != sizeof(kent_all_stats_t))
	{
		KENT_ETRACE("Iga1",p_acs, len, sizeof(kent_all_stats_t));
		return(EINVAL);
	}

	if (rc = get_stats(p_acs, (ent_ndd_stats_t *)stats, 
		sizeof(ent_ndd_stats_t)))
	{
		KENT_ETRACE("Iga2",p_acs, rc,0);
		return(rc);
	}

	bcopy(&ADAPSTATS, &stats->kent_stats, sizeof(kent_stats_t));
	stats->kent_stats.coll_freq[15] = ENTSTATS.excess_collisions;
	
	KENT_TRACE("IgaE",p_acs,0,0);
	return(0);
}


/*
 * NAME: clear_stats
 *
 * FUNCTION: clear the statistics for the user
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES:
 *
 * RETURNS:
 *      0       - Success
 */
int
clear_stats (
        kent_acs_t      *p_acs)
{
        int     rc;
	int 	tmp;
	int 	iomem;

	KENT_TRACE("IcsB",p_acs, 0, 0);

	bzero(&p_acs->ls, sizeof(kent_all_stats_t));
        bzero(&NDDSTATS, sizeof(ndd_genstats_t));
	bzero(&MIBS, sizeof(ethernet_all_mib_t));
	p_acs->dev.stime = lbolt;

	iomem = (int) iomem_att(&p_acs->dev.iomap);

	CSR_READ(iomem, 114, &tmp); /* get the receive collision count */
	ENTSTATS.rx_collisions -= ADDR_LO(tmp);

	CSR_READ(iomem, 112, &tmp); /* get the missed frame count */
	ENTSTATS.no_resources -= ADDR_LO(tmp);

	iomem_det((void *)iomem);

	KENT_TRACE("IcsE",p_acs,0,0);
	return(0);
}

/*
 * NAME: mib_query
 *
 * FUNCTION: fill out the mib structure with the fields the driver supports
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES:
 *      Check the size of the area, fail if insufficent size
 *
 * RETURNS:
 *      0       - Success
 *      EINVAL  - Insufficient size for the statistics
 */
int
mib_query (
        kent_acs_t      *p_acs,
        ethernet_all_mib_t *mib,
        int             len)
{
        int     rc;
	int 	tmp;
	int 		iomem;

	KENT_TRACE("ImqB",p_acs,mib,len);
	if (len < sizeof(ethernet_all_mib_t))
        {
                KENT_ETRACE("Imq1",p_acs,mib,len);
                return(EINVAL);
        }
	bzero(mib, sizeof(ethernet_all_mib_t));

	mib->Generic_mib.ifExtnsEntry.chipset[0] = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsEntry.revware[0] = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsEntry.mcast_tx_ok = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsEntry.bcast_tx_ok = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsEntry.mcast_rx_ok = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsEntry.bcast_rx_ok = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsEntry.promiscuous = MIB_READ_ONLY;
        mib->Generic_mib.ifExtnsTestEntry.community = MIB_NOT_SUPPORTED;
        mib->Generic_mib.ifExtnsTestEntry.request_id = MIB_NOT_SUPPORTED;
	mib->Generic_mib.ifExtnsTestEntry.type = MIB_NOT_SUPPORTED;
        mib->Generic_mib.ifExtnsTestEntry.result = MIB_NOT_SUPPORTED;
        mib->Generic_mib.ifExtnsTestEntry.code = MIB_NOT_SUPPORTED;
        mib->Generic_mib.RcvAddrTable = MIB_READ_ONLY;

	mib->Ethernet_mib.Dot3StatsEntry.align_errs = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.fcs_errs = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.s_coll_frames = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.m_coll_frames = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.sqetest_errs = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.defer_tx = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.late_collisions = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.excess_collisions = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.mac_tx_errs = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.carriers_sense = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3StatsEntry.long_frames = MIB_NOT_SUPPORTED;
	mib->Ethernet_mib.Dot3StatsEntry.mac_rx_errs = MIB_READ_ONLY;
	mib->Ethernet_mib.Dot3CollEntry.freq[0] = MIB_READ_ONLY;

	KENT_TRACE("ImqB",p_acs,mib,len);
        return(0);
}


/*
 * NAME: mib_get
 *
 * FUNCTION: fill out the mib structure with the data
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES:
 *      Check the size of the area, fail if insufficent size
 *
 * RETURNS:
 *      0       - Success
 *      EINVAL  - Insufficient size for the statistics
 */
int
mib_get (
        kent_acs_t      *p_acs,
        ethernet_all_mib_t *mib,
        int             len)
{
	int		i,rc;
	int 		iomem;
        ent_ndd_stats_t stats;

	KENT_TRACE("ImgB",p_acs,mib,len);

	if (len < sizeof(ethernet_all_mib_t))
        {
                KENT_ETRACE("Img1",p_acs,mib,len);
                return(EINVAL);
        }

	if (rc = get_stats(p_acs, &stats, sizeof(ent_ndd_stats_t)))
	{
		KENT_ETRACE("Iga2",p_acs, rc,0);
		return(rc);
	}


	bzero(mib, sizeof(ethernet_all_mib_t));
	bcopy(UNKNOWNCHIPSET, mib->Generic_mib.ifExtnsEntry.chipset, 
		CHIPSETLENGTH);
	mib->Generic_mib.ifExtnsEntry.revware[0] = p_acs->dev.chip_rev;
        mib->Generic_mib.ifExtnsEntry.mcast_tx_ok = ENTSTATS.mcast_tx_ok;
        mib->Generic_mib.ifExtnsEntry.bcast_tx_ok = ENTSTATS.bcast_tx_ok;
        mib->Generic_mib.ifExtnsEntry.mcast_rx_ok = ENTSTATS.mcast_rx_ok;
        mib->Generic_mib.ifExtnsEntry.bcast_rx_ok = ENTSTATS.bcast_rx_ok;
	if (p_acs->ndd.ndd_flags & NDD_PROMISC)
        	mib->Generic_mib.ifExtnsEntry.promiscuous = PROMTRUE;
	else
        	mib->Generic_mib.ifExtnsEntry.promiscuous = PROMFALSE;
        mib->Generic_mib.ifExtnsTestEntry.community = 0;
        mib->Generic_mib.ifExtnsTestEntry.request_id = 0;
	mib->Generic_mib.ifExtnsTestEntry.type = 0;
        mib->Generic_mib.ifExtnsTestEntry.result = 0;
        mib->Generic_mib.ifExtnsTestEntry.code = 0;
	/* 
	 * Add 2 to the address count to include broadcast and hardware 
	 * addresses 
	 */
        mib->Generic_mib.RcvAddrTable = p_acs->addrs.addr_cnt + 2;

	mib->Ethernet_mib.Dot3StatsEntry.align_errs = ENTSTATS.align_errs;
	mib->Ethernet_mib.Dot3StatsEntry.fcs_errs = NDDSTATS.ndd_ibadpackets;
	mib->Ethernet_mib.Dot3StatsEntry.s_coll_frames = ENTSTATS.s_coll_frames;
	mib->Ethernet_mib.Dot3StatsEntry.m_coll_frames = ENTSTATS.m_coll_frames;
	mib->Ethernet_mib.Dot3StatsEntry.defer_tx = ENTSTATS.defer_tx;
	mib->Ethernet_mib.Dot3StatsEntry.late_collisions = 
		ENTSTATS.late_collisions;
	mib->Ethernet_mib.Dot3StatsEntry.excess_collisions = 
		ENTSTATS.excess_collisions;
	mib->Ethernet_mib.Dot3StatsEntry.carriers_sense = 
		ENTSTATS.carrier_sense;
	mib->Ethernet_mib.Dot3StatsEntry.long_frames = 0;
	mib->Ethernet_mib.Dot3StatsEntry.mac_rx_errs = ENTSTATS.overrun +
		ENTSTATS.short_frames + stats.ent_ent_genstats.no_resources;
	mib->Ethernet_mib.Dot3StatsEntry.mac_tx_errs = ENTSTATS.underrun + 
		ENTSTATS.tx_timeouts;
	mib->Ethernet_mib.Dot3StatsEntry.sqetest_errs = ENTSTATS.sqetest;
	for (i=0; i<16; i++)
		mib->Ethernet_mib.Dot3CollEntry.freq[i] = 
			ADAPSTATS.coll_freq[i];

	mib->Ethernet_mib.Dot3CollEntry.freq[15] = ENTSTATS.excess_collisions;

	KENT_TRACE("ImqB",p_acs,mib,len);
        return(0);
}

/*
 * NAME: add_status
 *
 * FUNCTION: handles the bad frame status filter
 *	Note: The adapter will always pass up CRC error frames.  The driver
 *	will only pass them on to the user if bad frame status blocks have
 *	been requested.  Thus this command does not need to do anything to 
 *	the adapter itself.
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 */
int
add_status (
	kent_acs_t 	*p_acs,
	ns_com_status_t	*p_status,
	int		len)
{
	KENT_TRACE("IasB",p_acs,p_acs->dev.bf_cnt,0);

	if (len < sizeof(ns_com_status_t))
	{
		KENT_ETRACE("Ias2",p_acs, p_status, len);
		return(EINVAL);
	}
	
	if (p_status->mask & NDD_BAD_PKTS)
	{
		p_acs->dev.bf_cnt++;
		p_acs->ndd.ndd_flags |= ENT_RCV_BAD_FRAME;
	}
	KENT_TRACE("IasE",p_acs,p_acs->dev.bf_cnt,0);
	return(0);
}

/*
 * NAME: del_status
 *
 * FUNCTION: handles the delete of a bad frame status filter
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 *	0 	 - Success
 *	EINVAL	 - The status had not been added
 */
int
del_status (
	kent_acs_t 	*p_acs,
	ns_com_status_t	*p_status,
	int		len)
{

	KENT_TRACE("IdsB",p_acs,p_acs->dev.bf_cnt,0);

	if (len < sizeof(ns_com_status_t))
	{
		KENT_ETRACE("Ids1",p_acs, p_status, len);
		return(EINVAL);
	}

	if (p_status->mask & NDD_BAD_PKTS)
	{
		if (p_acs->dev.bf_cnt == 0)
 		{
			KENT_ETRACE("Ids2",p_acs,p_acs->dev.bf_cnt,0);
			return(EINVAL);
		}

		p_acs->dev.bf_cnt--;
		if (p_acs->dev.bf_cnt == 0)
		{
			p_acs->ndd.ndd_flags &= ~(ENT_RCV_BAD_FRAME);
		}
	}
	KENT_TRACE("IdsE",p_acs,p_acs->dev.bf_cnt,0);
	return(0);
}

/*
 * NAME: enable_address
 *
 * FUNCTION: handles enabling a group address
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 *	0 	- Success
 *	EINVAL	- an illegal (not a group address) was sent by the user
 *	ENOMEM 	- not enough memory to support additional addresses.
 */
int
enable_address(
	kent_acs_t 	*p_acs,
	kent_addr_t 	addr,
	int		len,
	int 		*adapter_flag)
{
	kent_addr_blk_t	*p_addr_blk;
	int 		i,j;

	KENT_TRACE("IeaB",p_acs, p_acs->addrs.addr_cnt, *(ulong *)&addr[0]);
	KENT_TRACE("IeaC",*(short *)&addr[4], 0,0);

	/* check the address to make sure it is the correct type of address */
	/* and that the length is correct */
	if (len != ENT_NADR_LENGTH)
	{
		KENT_ETRACE("Iea1",p_acs, addr, len);
                return (EINVAL);
        }

	if (!(addr[0] & KENT_MCAST))
        {
                KENT_ETRACE("Iea2",p_acs,
                        *(ulong *)&addr[0], *(ushort *)&addr[4] );
                return(EINVAL);
        }

	/* 
	 * First step is to see if this same address has already been set 
	 * previously.
	 */
	p_addr_blk = p_acs->addrs.addrs;
	for (i=0, j=0; i<p_acs->addrs.addr_cnt; j++)
	{
		if (j == KENT_MAX_ADDR_BLK)
		{
			j = 0;
			p_addr_blk = p_addr_blk->next;
			KENT_ASSERT(p_addr_blk != 0);
		}

		if (p_addr_blk->addrs[j].addr_cnt > 0)
			if (!bcmp(addr, p_addr_blk->addrs[j].addr,
				ENT_NADR_LENGTH))
			{
				/*
				 * The address was already set so, the 
				 * address' reference count only needs to 
				 * be incremented.
				 */
				p_addr_blk->addrs[j].addr_cnt++;
				KENT_TRACE("IeaE",p_acs, p_acs->addrs.addr_cnt, 
					0);
				return(0);
			}
			else
				i++;
			
	}

	/*
	 * Second step, knowing this is a new address, is to see if there 
	 * enough room in the current address blocks to save the address.
	 * To do this the driver will loop through the linked address blocks
	 * (the smallest memory net malloc can give) and see if any of 
	 * the address block have an empty slot.
	 */
	p_addr_blk = p_acs->addrs.addrs;
	while (p_addr_blk)
	{
		/*
		 * if the address block's address count is less than the 
		 * maximum for an address block, then the driver will
		 * search through the slots for a slot with a 0 reference
	 	 * count.  A 0 reference count signifies that the slot is
		 * not being used currently.
		 */
		if (p_addr_blk->blk_cnt < KENT_MAX_ADDR_BLK)
		{
			for (i=0; i< KENT_MAX_ADDR_BLK; i++)
			{
				/* 
				 * When the driver finds an empty slot 
	 			 * it needs to copy in the address, initialize
				 * the reference count, and increment both the
				 * address blocks address count and the 
				 * drivers over all address count.
				 *
				 * After this, calculate_ladrf is called to 	
				 * recalculate the adapter logical address
				 * filter so that packets for this 
				 * address will be received.
				 * The adapter_flag needs to be set so the 
				 * new ladrf will be sent to the adapter.
				 */
			
				if (p_addr_blk->addrs[i].addr_cnt == 0)
				{
					bcopy(addr, 
						p_addr_blk->addrs[i].addr,
						ENT_NADR_LENGTH);
					p_addr_blk->addrs[i].addr_cnt = 1;
					p_addr_blk->blk_cnt++;
					p_acs->addrs.addr_cnt++;
					
					calculate_ladrf(p_acs);
					*adapter_flag = TRUE;
					KENT_TRACE("IeaE",p_acs, 
						p_acs->addrs.addr_cnt,0);
					return(0);
				}
			}
		}
		p_addr_blk = p_addr_blk->next;
	}

	/*
	 * Third step, knowing this is a new address, and that there isn't
	 * any room on the current address blocks, allocate a new address
	 * block, add the address to the block (going through the same
	 * steps described above), and add the block to the front of the
	 * address block list.  This reduces future search time for free blocks
	 * and alleviates the special case for no current address blocks.
	 */
	p_addr_blk = net_malloc( sizeof(kent_addr_blk_t), M_DEVBUF, M_NOWAIT);

	if (p_addr_blk == 0)
	{
		KENT_ETRACE("Iea3",p_acs, p_acs->addrs.addr_cnt, 0);
		return(ENOMEM);
	}
	bzero(p_addr_blk, sizeof(kent_addr_blk_t));
	bcopy(addr, p_addr_blk->addrs[0].addr, ENT_NADR_LENGTH);

	p_addr_blk->addrs[0].addr_cnt = 1;
	p_addr_blk->blk_cnt = 1;
	p_acs->addrs.addr_cnt++;

	p_addr_blk->next = p_acs->addrs.addrs;
	p_addr_blk->prev = 0;
	if (p_addr_blk->next)
		p_addr_blk->next->prev = p_addr_blk;
	p_acs->addrs.addrs = p_addr_blk;

	calculate_ladrf(p_acs);

	p_acs->ndd.ndd_flags |= NDD_ALTADDRS;
	
	*adapter_flag = TRUE;
	KENT_TRACE("IeaE",p_acs, p_acs->addrs.addr_cnt,0);
	return(0);
}

/*
 * NAME: disable_address
 *
 * FUNCTION: handles disabling a group address
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 *	0 	- Success
 *	EINVAL	- an illegal (not a group address) was sent by the user
 *	ENOMEM 	- not enough memory to support additional addresses.
 */
int
disable_address(
	kent_acs_t 	*p_acs,
	kent_addr_t 	addr,
	int		len,
	int 		*adapter_flag)
{
	kent_addr_blk_t	*p_addr_blk;
	int 		i,j;

	KENT_TRACE("IdaB",p_acs, p_acs->addrs.addr_cnt, *(ulong *)&addr[0]);

	/* 
	 * validate the address specified for type and length 
	 */
	if (len != ENT_NADR_LENGTH)
	{
		KENT_ETRACE("Ida1",p_acs, addr, len);
                return (EINVAL);
        }

	if (!(addr[0] & KENT_MCAST))
        {
                KENT_ETRACE("Ida2",p_acs,
                        *(ulong *)&addr[0], *(ushort *)&addr[4] );
                return(EINVAL);
        }

	/* 
	 * Need to find the address specified in the list.  If it is not
	 * in the list then an invalid address has been specified.
	 */
	p_addr_blk = p_acs->addrs.addrs;
	for (i=0, j=0; i<p_acs->addrs.addr_cnt; j++)
	{
		if (j == KENT_MAX_ADDR_BLK)
		{
			j = 0;
			p_addr_blk = p_addr_blk->next;
			KENT_ASSERT(p_addr_blk != 0);
		}

		if (p_addr_blk->addrs[j].addr_cnt > 0)
			if (!bcmp(addr, p_addr_blk->addrs[j].addr,
				ENT_NADR_LENGTH))
			{
				/* 
				 * When the driver finds the address, 
				 * decrement the reference count, if the
				 * reference count is still above zero then
				 * nothing else needs to be done.	
				 */
				--p_addr_blk->addrs[j].addr_cnt;
				if (p_addr_blk->addrs[j].addr_cnt)
				{
					KENT_TRACE("IdaE",p_acs,
						p_acs->addrs.addr_cnt,0);
					return(0);
				}

				/*
				 * if the reference count was zero, then two
				 * steps remain.  The driver needs to clean
				 * up the data structures.  Decrementing
				 * the address counts.  If the address blocks
				 * address count is zero, then the address block
				 * needs to be freed (removed from the chain
				 * first).  Then, in either case, the ladrf
				 * needs to be recalculated and the adapter_flag
				 * set so that the adapter uses the new ladrf.
				 */
				 
				p_acs->addrs.addr_cnt--;
				p_addr_blk->blk_cnt--;
				if (p_addr_blk->blk_cnt)
				{
					calculate_ladrf(p_acs);
					*adapter_flag = TRUE;

					KENT_TRACE("IdaE",p_acs,
						p_acs->addrs.addr_cnt,0);
					return(0);
				}

				if (p_addr_blk->prev)
					p_addr_blk->prev->next = 
						p_addr_blk->next;

                                if (p_addr_blk->next != 0)
                                        p_addr_blk->next->prev =
                                        	p_addr_blk->prev;

                                if (p_acs->addrs.addrs == p_addr_blk)
                                {
                                	p_acs->addrs.addrs = p_addr_blk->next;
                                }

                                net_free(p_addr_blk, M_DEVBUF);

				/* 
				 * If this was the last multicast address, 
				 * than clear the ndd flag
				 */
				if (p_acs->addrs.addr_cnt == 0)
					p_acs->ndd.ndd_flags &= ~(NDD_ALTADDRS);

				calculate_ladrf(p_acs);
				*adapter_flag = TRUE;

				KENT_TRACE("IdaE",p_acs,
					p_acs->addrs.addr_cnt, 0);

				return(0);
                       	}
			else
			{
				i++;
			}

	}

	KENT_ETRACE("Ida3",p_acs, p_acs->addrs.addr_cnt,0);
	return(EINVAL);
}

/*
 * NAME: calculate_ladrf
 *
 * FUNCTION: runs a crc on the group addresses set and get the resulting 64 bit
 * 	flag for the adapter
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 */
void
calculate_ladrf(
	kent_acs_t 	*p_acs)
{
	int i,j;
	uchar flagno, flag;
	kent_addr_blk_t *p_addr_blk;

	KENT_TRACE("IclB",p_acs, p_acs->addrs.addr_cnt,0);

	/* clear the ladrf */
	for (i=0; i<KENT_LADRF_LEN; i++)
		p_acs->addrs.ladrf[i] = 0x00;

	/* calculate the bit flag for every address set */
	p_addr_blk = p_acs->addrs.addrs;
	for (i=0,j=0; i<p_acs->addrs.addr_cnt; j++)
	{
		/* 
		 * If we are at the end of the address block, move on to the 
		 * next one.
		 */
		if (j == KENT_MAX_ADDR_BLK)
		{
			j = 0;
			p_addr_blk = p_addr_blk->next;
			KENT_ASSERT(p_addr_blk != 0);
		}
	
		/*
		 * If the address' reference count is greater than zero, than
		 * the address is set, and the bit flag needs to be 
		 * calculated from it.
		 */
		if (p_addr_blk->addrs[j].addr_cnt > 0)
		{
			i++;
			calculate_flag(p_acs, p_addr_blk->addrs[j].addr, 
				&flagno, &flag);
			p_acs->addrs.ladrf[flagno] |= flag;
		}

	}
	KENT_TRACE("IclE",p_acs, *(ulong *)&p_acs->addrs.ladrf[0], 
		*(ulong *)&p_acs->addrs.ladrf[4]);
	return;
}

/*
 * Do not change the poly variable (or its initialized value).  It is needed 
 * to calculate the crc value of the addresses to set up the ladrf.
 */
static char poly[32] = {1,1,1,0,1,1,0,1,1,0,1,1,1,0,0,0,
 1,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,};
/*****************************************************************************/
/*
 * NAME:     calculate_flag
 *
 * FUNCTION: Hashes the multicast address.
 *		NOTE: be very careful changing this routine.  
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    In order for this adapter to receive packets that have
 *           destination addresses other than the adapters physical node
 *           address, it is a requirement to maintain a list of these
 *           destination addresses.  The adapter uses a 64 bit flag to 
 *	     determine which multicast packets to receive.  The adapter
 *	     takes the crc value of the multicast address and then the
 *	     6 high order bits from the CRC value.  The 6 high order bits
 *	     indicate a bit in the 64 bit flag, if this bit is set the adapter
 *	     passes the packet on.
 *
 * RETURNS:  
 *
 */
/*****************************************************************************/

void
calculate_flag(
	kent_acs_t *p_acs, 
	caddr_t addr, 
	uchar *flagno, 
	uchar *flag)
{
   	long crc[33];
	int i, j, j1, j2;
	char currentbyte, bit;
	int hashcode;

    	KENT_TRACE("IcfB", (ulong)addr, (ulong)&addr[3], 0);

        /* clear crc */
        for (j = 0; j < 32; j++)
         	crc[j] = 1;

	/* 
	 * for each byte of the address
	 */
         for (j = 0; j < 6; j++)
         {
               	currentbyte = addr[j];
		/* 
		 * for each bit in the byte.
		 */
               	for (j1 = 0; j1 < 8; j1++)
               	{
                   	bit = ((currentbyte>>j1) & 1);

			/* shift the current crc bits */
                   	for(j2 = 32; j2 > 0; j2--)
                   		crc[j2] = crc[j2-1];

                   	crc[0] = 0;

                   	if (bit ^ crc[32])
                   		for(j2 = 0; j2 < 32; j2++)
                   			crc[j2] ^=poly[j2];
               	}
         }
 
         hashcode = 0;
         for (j = 0; j < 6; j++)
         	hashcode = (hashcode << 1) + crc[j];

	/*
	 * The ladrf is made up of the 6 high order bits in the crc.  From
	 * this the driver calculates the byte of the ladrf to change (flagno) 
	 * and the bit in the byte (flag).
	 */
    	*flagno = hashcode >> 3;
    	*flag = 1 << (hashcode & 0x07);
    
    	KENT_TRACE("IclE", *flagno, *flag, 0);
    	return;
}


/*
 * NAME: mib_addr
 *
 * FUNCTION: returns the address for the adapter
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	Returns the cards addresses in source, broadcast, group order.
 *	NOTE: the addresses are put into the ndd_addr_elem_t structure which 
 * 		is put into the area provided in a compact manner (you cannot
 *		index the elements as if they were an array (there isn't the
 *		normal 'c' spacers to fill out the structure)).
 *
 * RETURNS: 
 *	0 	 - Success
 *	E2BIG	 - If the buffer given was not large enough to hold all of the 
 * 			addresses, as many as could fit are copied in.
 */

int
mib_addr(
	kent_acs_t *p_acs,
	ndd_mib_addr_t *p_mib_addr,
	int	len)
{
	ndd_mib_addr_elem_t 	*p_mib_addr_elem;
	kent_addr_blk_t		*p_addr_blk;
	int 			i,j;

	KENT_TRACE("ImaB",p_acs, p_acs->addrs.addr_cnt, p_mib_addr);

	/* 
	 * Check the user structure for minimal valid length 
	 * Note this minimal length is not long enough to return an address in.
	 */
	if (len < sizeof(ndd_mib_addr_t))
	{
		KENT_ETRACE("Ima1",p_acs,len, 0);
		return(EINVAL);
	}

	/* 
	 * initialize the address count of addresses returned.
	 * len will be decremented to remove the size of the count from 
	 * the user's buffer so that len can be used accurately later to 
	 * see if there is room for another address.
	 */
	p_mib_addr->count = 0;
	p_mib_addr_elem = &p_mib_addr->mib_addr[0];
	len -= 4;

	/*
	 * If there is room in the block of memory add the broad cast address 
	 * Note KENT_MIB_ADDR_ELEM_LEN is used instead of sizeof the structure
 	 * because the structure definition does not allow enough space for
	 * the address to fit.
	 */
	if ((p_mib_addr->count + 1) * KENT_MIB_ADDR_ELEM_LEN <= len)
	{
		p_mib_addr->count++;
		p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
		p_mib_addr_elem->addresslen = ENT_NADR_LENGTH;
		for(i=0; i<ENT_NADR_LENGTH; i++)
			p_mib_addr_elem->address[i] = 0xff;
		p_mib_addr_elem = (ndd_mib_addr_elem_t *)
		&p_mib_addr_elem->address[ENT_NADR_LENGTH];
	}
	else 
	{
		KENT_ETRACE("Ima2",p_acs,p_mib_addr->count, 0);
		return (E2BIG);
	}

	/* 
	 * If there is room add the source address for this adapter 
	 */
	if ((p_mib_addr->count + 1) * KENT_MIB_ADDR_ELEM_LEN <= len)
	{
		p_mib_addr->count++;
		p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
		p_mib_addr_elem->addresslen = ENT_NADR_LENGTH;
		bcopy(p_acs->addrs.src_addr, p_mib_addr_elem->address, 
			ENT_NADR_LENGTH);
		p_mib_addr_elem = (ndd_mib_addr_elem_t *)
			&p_mib_addr_elem->address[ENT_NADR_LENGTH];
	}
	else 
	{
		KENT_ETRACE("Ima3",p_acs,p_mib_addr->count, 0);
		return (E2BIG);
	}

	/*
	 * Add the multicast addresses to the buffer, check each one for room
	 */
	p_addr_blk = p_acs->addrs.addrs;

	for (i=0, j=0; i<p_acs->addrs.addr_cnt; j++)
	{
		if (j == KENT_MAX_ADDR_BLK)
		{
			p_addr_blk = p_addr_blk->next;
			j=0;
			KENT_ASSERT(p_addr_blk != 0);
		}

		if (p_addr_blk->addrs[j].addr_cnt > 0)
		{
			i++;
			if ((p_mib_addr->count + 1) * KENT_MIB_ADDR_ELEM_LEN
				<= len)
			{
				p_mib_addr->count++;
				p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
				p_mib_addr_elem->addresslen = ENT_NADR_LENGTH;
				bcopy(p_addr_blk->addrs[j].addr, 
					p_mib_addr_elem->address, 
					ENT_NADR_LENGTH);
				p_mib_addr_elem = (ndd_mib_addr_elem_t *)
					&p_mib_addr_elem->address[ENT_NADR_LENGTH];
			}
			else 
			{
				KENT_ETRACE("Ima5",p_acs,p_mib_addr->count, 0);
				return (E2BIG);
			}
		}
	}

	KENT_TRACE("ImaE",p_acs, p_mib_addr->count,len);
	return (0);
}
	
