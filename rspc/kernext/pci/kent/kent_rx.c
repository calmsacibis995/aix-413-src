static char sccsid[] = "@(#)65	1.6  src/rspc/kernext/pci/kent/kent_rx.c, pcient, rspc41J, 9519B_all 5/10/95 15:53:51";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: free_rx
 *		init_rx
 *		rx_handler
 *		undo_rx
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
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/dump.h>

/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: rx_handler()
 *                                                                    
 * FUNCTION: handles any receive interrupts from the driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine will be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
rx_handler( 	kent_acs_t 	*p_acs)
{
	int 		iomem;
	int		stat1, stat2;
	kent_desc_t	*p_desc;
	ndd_statblk_t   statblk;
	char		*p_buf;
	struct mbuf	*p_frame;
	int		len;

	KENT_RTRACE("RrhB",p_acs,p_acs->rx.nxt_rx,0);

	while (TRUE)
	{
		p_desc = &p_acs->rx.desc[p_acs->rx.nxt_rx];
		p_buf = &p_acs->rx.p_buffer[(p_acs->rx.nxt_rx * KENT_BUFF_SZ)];
		stat1 = SWAP_LONG(p_desc->stat1);
		stat2 = SWAP_LONG(p_desc->stat2);

		if (stat1 & KENT_RX_STAT1_OWN)
			break;

		if (!((stat1 & KENT_RX_STAT1_STP) && 
			(stat1 & KENT_RX_STAT1_ENP)))
		{
			NDDSTATS.ndd_ipackets_drop++;
		}
		else
		if (stat1 & KENT_RX_STAT1_OFLO ||
			stat1 & KENT_RX_STAT1_BUFF)
		{
			if (stat1 & KENT_RX_STAT1_OFLO)
				ENTSTATS.overrun++;

			NDDSTATS.ndd_ierrors++;
		}
		else
		{
			len = KENT_BLEN_MASK & stat2;

			/* 
			 * Remove the CRC value from the packet if the 
			 * adapter didn't strip it.  The adapter only
			 * strips packets that are below the minimum 
			 * length.
			 */
			if (len > ENT_MIN_MTU)
				len -= 4;

			/* 
	 		 * The AMD chip has been noted to pass up 0 length 
			 * packets this is put here to guard against this 
			 * occuring 
			 */
			if (len <= 0)
			{
				ENTSTATS.short_frames++;
				KENT_ERTRACE("Rrh1",ENTSTATS.short_frames,
					p_desc, p_buf);
				ARM_RX_DESC(p_desc->stat1, p_desc->stat2);
				if (++p_acs->rx.nxt_rx == p_acs->dds.rx_que_sz)
					p_acs->rx.nxt_rx = 0;
				continue;
			}
	

			if (len <= MHLEN)
				p_frame = (struct mbuf *)
					m_gethdr(M_DONTWAIT, MT_DATA);
                	else
                        	p_frame = (struct mbuf *)
					m_getclustm(M_DONTWAIT,
						MT_DATA,len);

			if (p_frame == NULL)
			{
				KENT_ERTRACE("Rrh1",p_acs, p_acs->rx.nxt_rx,0);
				NDDSTATS.ndd_ipackets_drop++;
				NDDSTATS.ndd_nobufs++;
				ARM_RX_DESC(p_desc->stat1, p_desc->stat2);
				if (++p_acs->rx.nxt_rx == p_acs->dds.rx_que_sz)
					p_acs->rx.nxt_rx = 0;
				break;
			}

                	p_frame->m_flags |= M_PKTHDR;
			p_frame->m_pkthdr.len = len;
			p_frame->m_len = len;

			bcopy(p_buf, MTOD(p_frame, char *), len);
			
			/* 
			 * Increment the short frames by the runt packet count
			 * (RPC).  This count is the number of runt packets 
			 * since the last good frame was received so needs to 
			 * be kept track of as frames are received.  The 
			 * adapter does not keep an overall count.
			 */
			ENTSTATS.short_frames += 
				(stat2 & KENT_RX_STAT2_RPC) >> 16;
			NDDSTATS.ndd_ierrors += 
				(stat2 & KENT_RX_STAT2_RPC) >> 16;

			ENTSTATS.rx_collisions += 
				(stat2 & KENT_RX_STAT2_RCVCO) >> 24;

			if (stat1 & KENT_RX_STAT1_CRC)
			{
				if (stat1 & KENT_RX_STAT1_FRAM)
					ENTSTATS.align_errs++;

				ENTSTATS.fcs_errs++;

				if (p_acs->ndd.ndd_flags &
					ENT_RCV_BAD_FRAME)
				{
					NDDSTATS.ndd_ibadpackets++;
					statblk.code = NDD_BAD_PKTS;
                        		statblk.option[0] = ENT_RCV_CRC_ERR;
                        		statblk.option[1]=(uint)p_frame;
                        		p_acs->ndd.nd_status(&p_acs->ndd, 
						&statblk);
				}
				else
					NDDSTATS.ndd_ipackets_drop++;

                        	m_freem(p_frame);
			}
			else
			{
				if (++NDDSTATS.ndd_ipackets_lsw == 0)
					NDDSTATS.ndd_ipackets_msw++;

				NDDSTATS.ndd_ibytes_lsw += len;
				if (NDDSTATS.ndd_ibytes_lsw < len)
					NDDSTATS.ndd_ibytes_msw++;

				if (KENT_IS_BCAST(p_buf))
				{
					ENTSTATS.bcast_rx_ok++;
					p_frame->m_flags |= M_BCAST;
				}
				else if (*p_buf & KENT_MCAST)
				{
					ENTSTATS.mcast_rx_ok++;
					p_frame->m_flags |= M_MCAST;
				}
			
				KENT_RTRACE("RNOT",p_acs->dds.md_sla, p_frame,
					p_frame->m_pkthdr.len);
				p_acs->ndd.nd_receive(&p_acs->ndd,p_frame);
				KENT_RTRACE("REND",p_acs->dds.md_sla, p_frame,
					0);
			}

		}

		ARM_RX_DESC(p_desc->stat1, p_desc->stat2);
		if (++p_acs->rx.nxt_rx == p_acs->dds.rx_que_sz)
			p_acs->rx.nxt_rx = 0;
	}
	KENT_RTRACE("RrhE",p_acs, p_acs->rx.nxt_rx, 0);
	return;
} /* end rx_handler() */



/*
 * NAME: init_rx()
 *                                                                    
 * FUNCTION: initialize the memory and services to setup the receive section
 *		of the driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine cannot be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

int
init_rx( 	kent_acs_t 	*p_acs)
{
	int		l_desc, l_buffer;
	int		i,j;
	int		rc;
	char		*p_buf;

	KENT_RTRACE("RirB",p_acs,0,0);
	

	l_desc = p_acs->dds.rx_que_sz * sizeof(kent_desc_t);
	l_buffer = p_acs->dds.rx_que_sz * KENT_BUFF_SZ;

	/*
	 * The adapter requires that the receive descriptors be in contiguous
	 * memory.   The PCI dma services do not guarantee that mapped
	 * pages are contiguous, so the receive descriptors must fit within
	 * a single page or the driver needs to use rmalloc.
	 */
	p_acs->rx.desc = xmalloc(l_desc, DMA_L2PSIZE, pinned_heap);

	if (p_acs->rx.desc == NULL)
	{
		KENT_ERTRACE("Rir1",p_acs,0,0);
		return (ENOMEM);
	}

	p_acs->rx.p_buffer = xmalloc(l_buffer, DMA_L2PSIZE, pinned_heap);

	if (p_acs->rx.p_buffer == NULL)
	{
		KENT_ERTRACE("Rir2",p_acs,0,0);
		xmfree(p_acs->rx.desc, pinned_heap);
		return (ENOMEM);
	}

	if ((rc = D_MAP_PAGE(p_acs->dev.dh,DMA_READ,p_acs->rx.desc, 
		&p_acs->rx.p_d_desc, xmem_global)) != DMA_SUCC)
        {
                KENT_ERTRACE("Rir3",rc, p_acs->rx.desc, p_acs->rx.p_d_desc);
		xmfree(p_acs->rx.desc,pinned_heap);
		xmfree(p_acs->rx.p_buffer, pinned_heap);
                return (ENOMEM);
        }

	bzero(p_acs->rx.desc, l_desc);
	
	for (i=0, p_buf = p_acs->rx.p_buffer; i<p_acs->dds.rx_que_sz; 
		i++, p_buf += KENT_BUFF_SZ)
	{
		if ((rc = D_MAP_PAGE(p_acs->dev.dh, DMA_READ, p_buf, 
			&p_acs->rx.desc[i].addr, xmem_global)) != DMA_SUCC)
		{
			KENT_ERTRACE("Rir4",rc, p_buf, 
				p_acs->rx.desc[i].addr);

			for (j=0; j<i; j++)
			{
				p_acs->rx.desc[j].addr = 
					SWAP_LONG(p_acs->rx.desc[j].addr);
				D_UNMAP_PAGE(p_acs->dev.dh, 
					(&p_acs->rx.desc[j].addr));
			}

			D_UNMAP_PAGE(p_acs->dev.dh, &p_acs->rx.p_d_desc);
			xmfree(p_acs->rx.desc, pinned_heap);
			xmfree(p_acs->rx.p_buffer, pinned_heap);
			return(ENOMEM);
		}

		p_acs->rx.desc[i].addr = SWAP_LONG(p_acs->rx.desc[i].addr);
		ARM_RX_DESC(p_acs->rx.desc[i].stat1, p_acs->rx.desc[i].stat2);
	}

	p_acs->rx.nxt_rx = 0;
	
	KENT_RTRACE("RirE",p_acs,0,0);
	return(0);
}


/*
 * NAME: free_rx()
 *                                                                    
 * FUNCTION: free up the memory and services used by the receive section
 *		of the driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine cannot be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
free_rx( 	kent_acs_t 	*p_acs)
{
	int i;

	KENT_RTRACE("RfrB",p_acs,0,0);

	D_UNMAP_PAGE(p_acs->dev.dh, &p_acs->rx.p_d_desc);
	
	for (i=0; i<p_acs->dds.rx_que_sz; i++)
	{
		p_acs->rx.desc[i].addr = SWAP_LONG(p_acs->rx.desc[i].addr);
		D_UNMAP_PAGE(p_acs->dev.dh,(&p_acs->rx.desc[i].addr));
	}
	
	xmfree(p_acs->rx.desc,pinned_heap);
	xmfree(p_acs->rx.p_buffer, pinned_heap);

	KENT_RTRACE("RfrE",p_acs,0,0);
	return;
}


/*
 * NAME: undo_rx()
 *                                                                    
 * FUNCTION: clean up the receive section in the event of a limbo condition
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
undo_rx(	kent_acs_t *p_acs)
{
	int i;

	KENT_RTRACE("RurB",p_acs,p_acs->rx.nxt_rx,0);

	for (i=0; i<p_acs->dds.rx_que_sz; i++)
	{
		ARM_RX_DESC(p_acs->rx.desc[i].stat1, p_acs->rx.desc[i].stat2);
	}

	p_acs->rx.nxt_rx = 0;

	KENT_RTRACE("RurE",p_acs,p_acs->rx.nxt_rx,0);
	return;
}

