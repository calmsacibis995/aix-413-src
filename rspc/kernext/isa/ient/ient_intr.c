static char sccsid[] = "@(#)75  1.9.1.4  src/rspc/kernext/isa/ient/ient_intr.c, isaent, rspc41J, 9517A_all 4/21/95 13:05:56";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: 
 *        ient_intr
 *        ient_rcv
 *        ient_tx_intr
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;

uchar ent_broad_adr[ENT_NADR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


/*****************************************************************************/
/*
 * NAME:     ient_intr
 *
 * FUNCTION: Ethernet driver interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * RETURNS:  
 *    INTR_SUCC - our interrupt
 *    INTR_FAIL - not our interrupt
 */
/*****************************************************************************/

int
ient_intr(struct intr *p_ihs) /* Device control area ptr. */
{
    int                    rc;                 /* Function return code.    */
    volatile intr_stat_reg status_reg;         /* Status reg. byte value.  */
    volatile cmd_reg       command_reg;        /* Command reg. byte value. */
    int                    ipri;               /* Intr. level to restore.*/
    volatile uchar         *ioaddr;            /* IO-space handle.       */
    xmt_elem_t             *tx_elem;
    dds_t                  *p_dds = (dds_t *)p_ihs;  /* Device structure ptr. */
    tx_buf_use_t	   *buf_xmit_next;
    tx_buf_use_t	   *buf_avail;

    rc = INTR_SUCC;
    ipri = disable_lock(PL_IMP, &SLIH_LOCK);

    /* Disable interrupts from the adapter. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);
    *(ioaddr + DDI.io_addr + PAGE0_IMR) = 0;
    eieio();

    /* Read the status register. Snap-shot #1. */
    status_reg.value = *(ioaddr + DDI.io_addr + PAGE0_ISR);
    eieio();

    /* if the driver is in the suspend state the adapter has */
    /* been reset, and we are not expecting any interrupts.  */
    /* It has to be our interrupt, we are not sharing any,   */
    /* so just clear it and return                           */

    if (WRK.adap_state == SUSPEND_STATE)
    {
         /* Acknowledge (clear) the interrupt bits so far. */
         *(ioaddr + DDI.io_addr + PAGE0_ISR) = status_reg.value;
         eieio();
         return(rc);
    }

    TRACE_DBG(HKWD_IEN_ISA_OTHER, "INTb", (ulong)status_reg.value, 0, 0);

    while (status_reg.value)
    {
        /* Acknowledge (clear) the interrupt bits so far. */
        *(ioaddr + DDI.io_addr + PAGE0_ISR) = status_reg.value;

        if (status_reg.value & IEN_ISA_ERROR_MASK)
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "INT3", INTR_SUCC, status_reg.value, 0);
            if (status_reg.stat_bits.cnt_overflow)
            {
                 /* Frame Alignment Errors (Tally 0). */
                 ENTSTATS.align_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR0);
                 /* CRC Errors (Tally 1). */
                 ENTSTATS.fcs_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR1);
                 /* Missed Packet Errors (Tally 2). */
                 ENTSTATS.rx_drop += *(ioaddr + DDI.io_addr + PAGE0_CNTR2);
            }
            ient_logerr(p_dds, ERRID_IEN_ISA_PARITY, __LINE__, __FILE__,
                        status_reg, 0, 0);
        }

        /*
        ** Check for receive interrupts, these come as regular ones or error
        ** ones. Group them in a mask to keep down processing time, in the
        ** case of no receive interrupts.
        */

        if ((status_reg.value & IEN_ISA_RECEIVE_MASK) && 
            (WRK.adap_state == OPEN_STATE))
        {

            /* We have a receive interrupt, either a 'good' or 'bad' one. */

            /* Update the number of receive intr statistics. */
            IEN_ISA_STAT_INCREMENT(NDD.ndd_genstats.ndd_recvintr_lsw,
                                   NDD.ndd_genstats.ndd_recvintr_msw);

            /* Check to see if we had a good receive interrupt. */

            if (status_reg.stat_bits.data_rx)
            {
                /*
                ** Update the alignment and crc error counters, they can be 
                ** different from 0 even in this 'good' case.
                */

                /* Frame Alignment Errors (Tally 0). */
                ENTSTATS.align_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR0);

                /* CRC Errors (Tally 1). */
                ENTSTATS.fcs_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR1);

	        /* Missed Packet Errors (Tally 2). */
        	ENTSTATS.rx_drop += *(ioaddr + DDI.io_addr + PAGE0_CNTR2);

                /* Now process the incoming packets. */
                ient_rcv(p_dds, ioaddr);

            }
            else  /* We have an error case. */
            {
                rcv_stat_reg rcv_stat;  /* Additional adapter receive status. */

                /* Read the receive status register. */
                rcv_stat.value = *(ioaddr + DDI.io_addr + PAGE0_RSR);

                TRACE_BOTH(HKWD_IEN_ISA_ERR,"INT1",(ulong)status_reg.value,
                           (ulong)rcv_stat.value,(ulong)0);

                /*
                ** First update the statistics (CRC, FIFO overrun, FAE,
                ** no buffers.)
                */

                ient_upd_rx_err_stats(p_dds, status_reg, rcv_stat, ioaddr);

                /*
                ** Check for an overflow condition, i.e the ring buffer
                ** is full.
                */

                if (status_reg.stat_bits.rx_overrun)
                {
                    ient_handle_overrun(p_dds, ioaddr);
                }
            } /* End of receive error case. */
        } /* End of receive check. */

        /* Check for transmit interrupts. */
        if (status_reg.value & IEN_ISA_XMIT_MASK)
        {
            /* Update the number of transmission interrupts. */
            IEN_ISA_STAT_INCREMENT(NDD.ndd_genstats.ndd_xmitintr_lsw,
                                   NDD.ndd_genstats.ndd_xmitintr_msw);

            /* Check to see if we had a good transmission. */

            if ((status_reg.stat_bits.data_tx) && (XMIT_BUF_TRANSMITTING))
            {
                ient_tx_intr(p_dds, ioaddr);
            }

            /* Check the transmit error bits. */

            if (status_reg.stat_bits.tx_err)
            {
                volatile tx_stat_reg tx_stat;

                /*
                ** Now read the transmission status register, only do that
                ** if errors encountered.
                */

                tx_stat.value = *(ioaddr + DDI.io_addr + PAGE0_TSR);

                /* Update the none-ndd statistic variables. */
                ient_upd_tx_err_stats(p_dds, tx_stat, ioaddr);

            } /* End of tx-errors encountered. */

        } /* End of transmission check. */

        /* Read the status register */
        status_reg.value = *(ioaddr + DDI.io_addr + PAGE0_ISR);
        eieio();
    };

    /* Re-enable adapter interrupts by resetting the mask. */
    *(ioaddr + DDI.io_addr + PAGE0_IMR) = IEN_ISA_ENB_ALL_INTR;
    eieio();
    iomem_det((void *) ioaddr);

    /* Check if another buffer is loaded and ready to xmit */
    /* Only attempt to transmit if other buffer not currently transmitting */
    if ((XMIT_BUF_READY) && !(XMIT_BUF_TRANSMITTING)) {
        ipri = disable_lock(PL_IMP, &TX_LOCK);
        WHICH_XMIT_BUF_READY(buf_xmit_next);
        ient_xmit(p_dds, buf_xmit_next);
        unlock_enable(ipri, &TX_LOCK);
    }

    /* Check for waiting transmission elements. */
    if ((WRK.nbr_in_wait_q) && (XMIT_BUF_AVAIL))
    {
        ipri = disable_lock(PL_IMP, &TX_LOCK);
        WHICH_XMIT_BUF_AVAIL(buf_avail);
        GET_WAIT_ELEM(buf_avail->elem);
        ient_load_buf(p_dds, buf_avail);

	/* now transmit if no other transmit going on */
        if ((XMIT_BUF_READY) && !(XMIT_BUF_TRANSMITTING)) {
            WHICH_XMIT_BUF_READY(buf_xmit_next);
            ient_xmit(p_dds, buf_xmit_next);
	}
        unlock_enable(ipri, &TX_LOCK);
    }

    unlock_enable(ipri, &SLIH_LOCK);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "INTe", rc, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_rcv
 *
 * FUNCTION: Ethernet driver receive interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *           ient_intr
 *
 * INPUT:
 *           p_dds          - point to the device control area
 *           command_reg    - value from command register
 *
 * RETURNS:  
 *           none
 */
/*****************************************************************************/

int
ient_rcv(dds_t          *p_dds,    /* Device control area ptr */
         volatile uchar *ioaddr)   /* IO-handle (IO-map.).*/
{
    int             rc;
    int             i;
    int             packet_type;
    uint            buff_addr;
    uint            count, even_count;
    volatile uchar  current;
    rcv_stat_reg    packet_status;
    cmd_reg         cmd;
    volatile rcv_hdr_t       rcv_hdr;
    struct mbuf     *p_mbuf;
    volatile uchar  *memaddr, *ring_start, *cp_char, *dp_char;
    volatile ushort *dp, *cp;
    ndd_t    *p_ndd =  (ndd_t *)&(NDD);
    unsigned char   *net_addr;
    volatile unsigned char   eth_addr[ENT_NADR_LENGTH];

    TRACE_SYS(HKWD_IEN_ISA_RECV, "RCVb", (ulong)p_dds, 0, 0);

    WRK.receiving = TRUE;

    /*
    ** Note (for future maintainers), brief receive logic description:
    ** The boundary register = current = page start register when things first
    ** start out. Then the current register moves along to point to the next
    ** buffer that the adapter can use. The boundary register points to the
    ** first buffer not read yet by the host. Or .. the current is the 'write
    ** pointer' and the boundary the 'read pointer'.
    */

    /* Set Page 1. */
    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                       IEN_ISA_NO_DMA | IEN_ISA_SET_PAGE_1 | IEN_ISA_START_NIC;
    eieio();

    /* Get the next available buffer from the 'current page' register. */

    current = *(ioaddr + DDI.io_addr + PAGE1_CURR);
    eieio();

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA|IEN_ISA_START_NIC;
    eieio();

    /* Map in the receive area part of the shared memory. */
    memaddr = iomem_att(dd_ctrl.mem_map_ptr);

    ring_start = (uchar *) (memaddr + DDI.shared_mem_addr);
    eieio();

    while (WRK.new_packet != current)
    {
        /* Setup the relative buffer address (shorts, not bytes). */
        buff_addr = (WRK.new_packet << 8);

        /* Read the receive status header of the currently oldest packet. */

        cp_char = (volatile uchar *) (ring_start + buff_addr);
        rcv_hdr.rcv_stat.value = *cp_char++;
        rcv_hdr.next_buffer    = *cp_char++;
        rcv_hdr.bytes          = *cp_char++;
        rcv_hdr.bytes          += ((*cp_char) << 8) - 4;
        cp_char++;
        eieio();
        even_count = count = rcv_hdr.bytes;

        /* For netpmon performance tool */
        TRACE_BOTH(HKWD_IEN_ISA_RECV, TRC_RDAT,p_dds->seq_number,
			rcv_hdr.bytes,(ulong) current);

        /* Get the destination addr. */
        for (i = 0, net_addr = (uchar *) cp_char; i < ENT_NADR_LENGTH; i++) {
            eth_addr[i] = *net_addr++;
	    eieio();
	}

        if (even_count & 0x01)     /* Make it an even number of shorts. */
        {
            even_count++;
        }

        /* Get a m-header or buffer. */

        if (even_count <= MHLEN)
        {
            p_mbuf = m_gethdr(M_DONTWAIT, MT_HEADER);
        }
        else
        {
            p_mbuf = m_getclustm(M_DONTWAIT, MT_HEADER, rcv_hdr.bytes);
        }

        if (p_mbuf)
        {
            /* Setup the packet information in the mbuf structure. */
            p_mbuf->m_pkthdr.len = count;
            p_mbuf->m_len        = count;
            p_mbuf->m_flags     |= M_PKTHDR;
            dp = (ushort *)p_mbuf->m_data;
            cp = (ushort *)cp_char;

            /* Note, it's a little bit tricky here, the chip has a problem
            ** distinguishing the difference between the initial ring-buffer
            ** start-condition and the out-of-buffers condition... the National
            ** Semi-conductor folks recommend that you don't use the last 256 
            ** byte buffer.
            */

            if ((even_count + 4 + buff_addr) < DDI.shared_mem_size)
            {
                /*
                ** All the data is sequentially residing in buffers.
                ** Copy the data from shared memory to the mbuf.
                */

		eieio();
                for (i = 0; i < (even_count / 2); i++)
                {
                    *dp++ = *cp++;
                }
            }
            else
            {                        /* We need to do two reads. */
                uint chunk_size;

                /* The data is wrapped in the ring-buffer. */

                chunk_size = DDI.shared_mem_size - buff_addr - 4;

                for (i = 0; i < (chunk_size / 2); i++)
                {
                    *dp++ = *cp++;
                }
                eieio();

                /* Move the cp pointer to the beginning of the ring buffer. */

                cp_char = (volatile uchar *)(ring_start +
                          (IEN_ISA_RCV_START << 8));

                cp = (volatile ushort *) cp_char;


                /* Now copy the rest of the data. */
                for (i = 0; i < ((even_count - chunk_size) / 2); i++)
                {
                    *dp++ = *cp++;
                }
            }

            /* Is this a good packet or one with errors ? */

            if (rcv_hdr.rcv_stat.stat_bits.packet_ok)
            {
                /*
                ** This is a good packet.  Update the receive statistics
                ** counters.
                */

                packet_type = ient_upd_rcv_stats(p_dds, &rcv_hdr, eth_addr);

                if (packet_type & IEN_ISA_BCAST_PACKET)
                {
                    p_mbuf->m_flags |= M_BCAST;
                }

                if (packet_type & IEN_ISA_MCAST_PACKET)
                {
                    p_mbuf->m_flags |= M_MCAST;
                }

                /* For netpmon performance tool */
                TRACE_SYS(HKWD_IEN_ISA_RECV, TRC_RNOT, p_dds->seq_number,
                          (ulong)p_mbuf, count);

                /* Deliver the data (dont free the mbuf, the demuxer owns it. */
                (*(p_ndd->nd_receive))(p_ndd, p_mbuf);

                /* For netpmon performance tool */
                TRACE_SYS(HKWD_IEN_ISA_RECV, "REND", p_dds->seq_number,
                          (ulong) p_mbuf, 0);
            }
            else
            {
                /* This is a bad packet */
                ndd_statblk_t  stat_blk;   /* status block */

		TRACE_BOTH(HKWD_IEN_ISA_RECV, "BADP", (ulong) p_mbuf,
			   (ulong) count, (ulong) 0);

		p_ndd->ndd_genstats.ndd_ibadpackets++;  /* Bump Bad Packets */

                /* Got a bad frame, create a status block. */
                bzero(&stat_blk, sizeof(ndd_statblk_t));
                stat_blk.code = NDD_BAD_PKTS;
                stat_blk.option[0] = (ulong)rcv_hdr.rcv_stat.value; /* TMP */
                stat_blk.option[1] = (ulong)p_mbuf;

                /* Pass the status block to the demuxer. */
                (*(p_ndd->nd_status))(p_ndd, &stat_blk);

                /* we own the mbuf, so we free it */
                m_free(p_mbuf);
            }
        }
        else
        {
            /* System ran out of MBUFs so we have to drop the packet. */

            p_ndd->ndd_genstats.ndd_nobufs++;
            p_ndd->ndd_genstats.ndd_ipackets_drop++;
        }

        /* Advance the boundary register. */
        ient_incr_bnry(p_dds, &rcv_hdr, ioaddr);

    }  /* while more incoming buffers to process. */

    /* Free the receive area. */
    iomem_det((void *) memaddr);

    WRK.receiving = FALSE;

    TRACE_SYS(HKWD_IEN_ISA_RECV, "RCVe",(ulong)WRK.new_packet, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_tx_intr
 *
 * FUNCTION: Ethernet driver transmit interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *           ient_intr
 *
 * INPUT:
 *           p_dds          - point to the device control area
 *           command_reg    - value from command register
 *
 * RETURNS:  
 *    none
 */
/*****************************************************************************/

int
ient_tx_intr(dds_t          *p_dds,     /* Device structure ptr. */
             volatile uchar *ioaddr)    /* IO-handle (IO-map.).  */
{
    int         rc;
    int         ipri;
    ndd_t       *p_ndd = (ndd_t *)&(NDD);
    uchar       status;
    struct mbuf *p_mbuf;
    xmt_elem_t  *tx_elem;
    tx_buf_use_t *buf_tx;
    tx_buf_use_t *buf_avail;
    tx_buf_use_t *buf_xmit_next;

    ipri = disable_lock(PL_IMP, &TX_LOCK);

    TRACE_SYS(HKWD_IEN_ISA_XMIT, "STXb",(ulong)p_dds,0,0);

    /* stop the watchdog timer */
    if (WRK.xmit_wdt_active)
    {
        WRK.xmit_wdt_active = FALSE;
        w_stop(&TXWDT);
    }

    /* Find which buffer was being transmitted - only one at a time */
    if (WRK.buf0.state == TRANSMITTING)
	buf_tx = &(WRK.buf0);
    else if (WRK.buf1.state == TRANSMITTING)
	 buf_tx = &(WRK.buf1);
	 else ASSERT(" got xmit intr with no buf transmitting");

    /* alternate transmit buffers */
    if (buf_tx->offset == BUF0)  {
        WRK.next_buf_xmit = BUF1;  /* xmit BUF1 when READY_TO_XMIT */
    }
    else {
        WRK.next_buf_xmit = BUF0;
    }


    IEN_ISA_STAT_INCREMENT(p_ndd->ndd_genstats.ndd_opackets_lsw,
        p_ndd->ndd_genstats.ndd_opackets_msw);

    if ((ULONG_MAX - buf_tx->elem->bytes) < p_ndd->ndd_genstats.ndd_obytes_lsw)
        p_ndd->ndd_genstats.ndd_obytes_msw++;

    p_ndd->ndd_genstats.ndd_obytes_lsw += buf_tx->elem->bytes;

    if (buf_tx->elem->flags & BDC_BCAST)
    {
        MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok++;
    }

    if (buf_tx->elem->flags & BDC_MCAST)
    {
        MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok++;
    }

    /* Mark transmit element as free */

    m_freem(buf_tx->elem->mbufp);
    buf_tx->elem->flags  = 0;
    buf_tx->elem->in_use = 0;
    ADD_2_FREE_Q(buf_tx->elem);

    /* allow this buffer to be loaded again */
    buf_tx->state = EMPTY; 

    unlock_enable(ipri, &TX_LOCK);

    TRACE_SYS(HKWD_IEN_ISA_XMIT, "STXe",(ulong)WRK.xmits_queued,
        (ulong)0,(ulong)NDD.ndd_genstats.ndd_opackets_lsw);
}

/*****************************************************************************/
/*
 * NAME:                  ient_upd_rx_err_stats
 *
 * FUNCTION:              Update the error statistics counters for rx 
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * RETURNS:  
 */
/*****************************************************************************/

int
ient_upd_rx_err_stats(dds_t *p_dds,              /* Device ctl area.    */
                      intr_stat_reg  status_reg, /* Intr. stat. val.    */
                      rcv_stat_reg   rcv_stat,   /* Rcv. stat. val.     */
                      volatile uchar *ioaddr)    /* IO-handle (IO-map.).*/
{

    /* We MUST read Tally Counters 0..2.in doing so .. update the stats. */
    /* Frame Alignment Errors (Tally 0). */

    ENTSTATS.align_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR0);

    /* CRC Errors (Tally 1). */
    ENTSTATS.fcs_errs += *(ioaddr + DDI.io_addr + PAGE0_CNTR1);

    /* Missed Packet Errors (Tally 2). */
    ENTSTATS.rx_drop += *(ioaddr + DDI.io_addr + PAGE0_CNTR2);

    /* Check for overflow of the FIFO, which is the true overrun situation. */
    if (rcv_stat.stat_bits.fifo_overrun)
    {
        ENTSTATS.overrun++;
    }

    /* Check for deferred reception. */
    if (rcv_stat.stat_bits.deferring)
    {
        ENTSTATS.rx_collisions++;
    }

    /* Check for receive overrun, which is 'ring buffer exhausted'. */
    if (status_reg.stat_bits.rx_overrun)
    {
        ENTSTATS.no_resources++;
    }

    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_upd_rcv_stats
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * INPUT:
 *            p_dds    - point to the device control area
 *
 * RETURNS:   0 for a individual packet
 *            IEN_ISA_BCAST_PACKET bit set for a broadcast packet
 *            IEN_ISA_MCAST_PACKET bit set for a multicast packet
 */
/*****************************************************************************/

int
ient_upd_rcv_stats(dds_t     *p_dds,    /* control area ptr */
                   rcv_hdr_t *rcv_hdr,  /* Receive stat. hdr */
                   unsigned char *eth_addr)
{
    int   packet_type;
    ndd_t *p_ndd = (ndd_t *)&(NDD);

    /* Increment the number of packets received. */
    IEN_ISA_STAT_INCREMENT(p_ndd->ndd_genstats.ndd_ipackets_lsw,
                           p_ndd->ndd_genstats.ndd_ipackets_msw);

    /* Increment the number of bytes received. */

    if ((ULONG_MAX - rcv_hdr->bytes) < p_ndd->ndd_genstats.ndd_ibytes_lsw)
        p_ndd->ndd_genstats.ndd_ibytes_msw++;

    p_ndd->ndd_genstats.ndd_ibytes_lsw += rcv_hdr->bytes;

    /* check if broadcast or multicast */

    if ((*eth_addr) & MULTI_BIT_MASK)
    {
        if (SAME_NADR(eth_addr, ent_broad_adr))
        {
            packet_type = IEN_ISA_BCAST_PACKET;
            MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok++;
        }
        else
        {
            packet_type = IEN_ISA_MCAST_PACKET;
            MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok++;
        }
    }
    else
    {
        packet_type = IEN_ISA_INDIVIDUAL_PACKET;
    }

    return(packet_type);
}


/*****************************************************************************/
/*
 * NAME:                  ient_upd_tx_err_stats
 *
 * FUNCTION:              Update the error statistics counters for tx 
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * RETURNS:  
 */
/*****************************************************************************/

int
ient_upd_tx_err_stats(dds_t  *p_dds,              /* Device ctl area.    */
                      tx_stat_reg    tx_stat,     /* Xmission status.    */
                      volatile uchar *ioaddr)     /* IO-handle (IO-map.).*/
{
    uchar nbr_collisions;

    /* Check for collisions and update counters as need be. */

    nbr_collisions = *(ioaddr + DDI.io_addr + PAGE0_NCR);

    if (nbr_collisions >= 1)
    {
        if (nbr_collisions == 1)
        {
            ENTSTATS.s_coll_frames++;
        }
        else
        {
            ENTSTATS.m_coll_frames++;
        }

        /* Now update the collision distribution array (0 based). */

        if (nbr_collisions <= 16)  /* Just to be save. */
        {
            DEVSTATS.coll_freq[nbr_collisions-1]++;
        }
    }

    if (tx_stat.stat_bits.tx_aborted)           /* Execessive Collisions */
    {
        ENTSTATS.excess_collisions++;
    }

    if (tx_stat.stat_bits.cr_sense_lost)        /* Carrier Sense */
    {
        ENTSTATS.carrier_sense++;
    }

    if (tx_stat.stat_bits.fifo_underrun)        /* DMA Underrun */
    {
        ENTSTATS.underrun++;
    }

    if (tx_stat.stat_bits.cd_heartbeat)         /* Deferred Transmissions */
    {
        ENTSTATS.defer_tx++;
    }

    if (tx_stat.stat_bits.out_window_coll)      /* Late Collisions */
    {
        ENTSTATS.late_collisions++;
    }
    return(0);
}


/*****************************************************************************/
/*
 * NAME:                  ient_incr_bnry
 *
 * FUNCTION:              Increment the cyclic boundary register
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 * The adapter has 16K of memory.  This is split into part transmit
 * buffer and part receive buffer.  The transmit buffer starts at the
 * beginning of the 16K and is comprised of two 2048 byte buffers
 * back to back.  The receive buffers follow, and are comprised of
 * 47 256 byte buffers.  The receive buffers are implemented by the
 * adapter as a circular queue.  The offset of the queue is found by
 * taking the buffer number [16..62] and left shifting it by 8.
 * So, the 47 receive buffers are numbered from 16 to 62.  The
 * boundary register indicates to the adapter where the device
 * driver last read data.  We update that here.  The current register
 * indicates the last buffer the adapter has used to put received
 * data into.  When we get to the end of the receive queue, we reset to
 * the beginning, which is indicated by IEN_ISA_RCV_START
 *
 * RETURNS:
 */
/*****************************************************************************/

void
ient_incr_bnry(dds_t          *p_dds,    /* Device ctl area.     */
               rcv_hdr_t      *rcv_hdr,  /* Receive information. */
               volatile uchar *ioaddr)   /* IO-handle (IO-map.). */
{
    uint new_boundary;



    /*
    ** We need to accomplish two things here, first we need to advance the
    ** boundary register to indicate the last processed buffer, second we need
    ** to update the new packet pointer. to current - 1.
    */

    /* Adjust the cyclic boundary register. */

    new_boundary  = rcv_hdr->next_buffer - 1;
    WRK.new_packet = rcv_hdr->next_buffer;

    if (new_boundary < IEN_ISA_RCV_START)
    {
        new_boundary = 62;            /* Point to the last receive buffer.*/
        WRK.new_packet = IEN_ISA_RCV_START;
    }

    *(ioaddr + DDI.io_addr + PAGE0_BNRY) = new_boundary;
    eieio();

    return;
}


/*****************************************************************************/
/*
 * NAME:                  ient_handle_overrun
 *
 * FUNCTION:              Handle a ring buffer overflow situation
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * RETURNS:  
 *    INTR_SUCC - our interrupt
 *    INTR_FAIL - not our interrupt
 */
/*****************************************************************************/

int
ient_handle_overrun(dds_t          *p_dds,      /* Device ctl area.    */
                    volatile uchar *ioaddr)     /* IO-handle (IO-map.).*/
{
    uchar          resend;  /* Were we doing a xmit, when it happened. */
    volatile uchar isr_reg; /* Interrupt status register content. */

    /*
    ** The procedure for handling overrun situations is very specific, before
    ** anyone modifies the following procedure make sure that the code will
    ** run in a loaded network.
    */

    /* First store the TXP bit from the command register. */

    resend = (*(ioaddr + DDI.io_addr + PAGE0_COMMAND)) & IEN_ISA_START_TX;
    eieio();

    /* Stop the network controller. */
    ient_stop_NIC(p_dds, ioaddr);

    /* Delay at least 1.6 ms. */
    io_delay(1600);

    /* Clear the RBCR registers, one more time. */
    *(ioaddr + DDI.io_addr + PAGE0_RBCR0) = 0;
    *(ioaddr + DDI.io_addr + PAGE0_RBCR1) = 0;
    eieio();

    /* Now branch depending on the resend value. */

    if (resend)
    {
        /* Check to see if the outstanding command completed. */
        isr_reg = (*(ioaddr + DDI.io_addr + PAGE0_ISR)) & IEN_ISA_XMIT_MASK;

        if (isr_reg)
        {
            /* It did, either succesfully or with an error. */
            resend = 0;
        }
    }

    /* Start the NIC, the NIC must be in loopback mode first. */
    *(ioaddr + DDI.io_addr + PAGE0_TCR) = IEN_ISA_INTERNAL_LOOPBACK;
    eieio();

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                                         IEN_ISA_NO_DMA + IEN_ISA_START_NIC;
    eieio();

    /* Now receive the packages from the full ring buffer. */
    ient_rcv(p_dds, ioaddr);

    /* Reset the overflow (OWL) bit in the ISR. */
    *(ioaddr + DDI.io_addr + PAGE0_ISR) = IEN_ISA_ENB_RX_OVR_ERR;
    eieio();

    /* Start up the NIC onto the network. */
    ient_start_NIC(p_dds, ioaddr);

    /* If we need to re-start the last send, do so. */
    if (resend)
    {
        *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = 0x26;
    }
}
