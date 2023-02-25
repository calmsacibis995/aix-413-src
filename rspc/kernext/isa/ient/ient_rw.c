static char sccsid[] = "@(#)79  1.6.1.3  src/rspc/kernext/isa/ient/ient_rw.c, isaent, rspc41J, 9517A_all 4/21/95 13:06:32";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: 
 *        ient_output
 *        ient_xmit
 *        ient_txq_put
 *        ient_txq_get
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     ient_output
 *
 * FUNCTION: Ethernet driver output routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  
 *           0 - successful
 */
/*****************************************************************************/

int
ient_output(ndd_t          *p_ndd,    /* ndd in the dev_ctl area */
            struct mbuf    *p_mbuf)   /* mbuf (chain) ptr */
{
    dds_t       *p_dds;          /* Device structure ptr.                    */
    int         txpri;           /* Old interrupt level to restore.          */
    xmt_elem_t  *xlm;            /* Where to store info about this xmission. */
    struct mbuf *p_first_mbuf;   /* First part of the packet(s).             */
    struct mbuf *p_next_mbuf;    /* Running mbuf pointer.                    */
    struct mbuf *m;
    tx_buf_use_t *buf_avail;
    tx_buf_use_t *buf_xmit_next;

    TRACE_SYS(HKWD_IEN_ISA_XMIT, "ETXb", (ulong)p_ndd, (ulong)p_mbuf, 0);

    /* Get adapter structure */
    p_dds = (dds_t *) (((ulong) p_ndd) - offsetof(dds_t, ndd));

    /*
    ** Accounting for MIBs - ifHCOutUcastPkts, ifHCOutMulticastPkts
    ** and ifHCOutBroadcastPkts.
    */

    m = p_mbuf;

    while (m)
    {
        if (m->m_flags & (M_BCAST | M_MCAST))
        {
            if (m->m_flags & M_BCAST)
            {
                if (NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw == ULONG_MAX)
                    NDD.ndd_genstats.ndd_ifOutBcastPkts_msw++;
                NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw++;
            }
            else
            {
                if (NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw == ULONG_MAX)
                    NDD.ndd_genstats.ndd_ifOutMcastPkts_msw++;
                NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw++;
            }
        }
        else
        {
            if (NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw == ULONG_MAX)
                NDD.ndd_genstats.ndd_ifOutUcastPkts_msw++;
            NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw++;
        }
        m = m->m_nextpkt;
    }

    if (WRK.adap_state != OPEN_STATE)
    {
        if (WRK.adap_state == DEAD_STATE)
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "ETX1", ENETDOWN, WRK.adap_state, 0);
            return(ENETDOWN);
        }
        else
        {
            TRACE_SYS(HKWD_IEN_ISA_ERR, "ETX2", ENETUNREACH, WRK.adap_state, 0);
            return(ENETUNREACH);
        }
    }

    txpri = disable_lock(PL_IMP, &TX_LOCK);

    /*
     * if the driver is in a power management mode of suspend/hibernate
     * then put the caller to sleep until the driver resumes normal operation
     * except for open this needs to be done under a lock so that the state
     * check is valid
     */
     if (WRK.adap_state == SUSPEND_STATE)
          e_sleep_thread (&WRK.pm_event,&TX_LOCK, LOCK_HANDLER);

    p_first_mbuf = p_mbuf;

    /*
    ** This 'while loop' loops for every packet in the chain.
    ** (Not for every mbuf).
    */

    while (p_mbuf)
    {
        /* Check to see if the wait queue (staging area) is full. */


        if (WAITQ_FULL)
        {
            /*
            ** It was full, if we did not have room for any of the packets
            ** in the chain return an error code. If we had room for
            ** at least one packet then return a good completion code.
            */

            if (p_mbuf == p_first_mbuf)
            {
                /* No room for any of them. */

                while (p_mbuf)
                {
                    /*
                    ** For each packet in the chain, increment an error
                    ** counter.
                    */
                    NDD.ndd_genstats.ndd_xmitque_ovf++;
                    p_mbuf = p_mbuf->m_nextpkt;
                }

                unlock_enable(txpri, &TX_LOCK);
                return(EAGAIN);

            }
            else
            {
                /* Since some of the mbufs fit on the wait queue, the
                ** device driver has to free those mbufs that didn't fit
                ** on the transmit queue.  No error is returned.
                */

                while (p_mbuf)
                {
                    /* Count the ones that we are dropping. */
                    NDD.ndd_genstats.ndd_xmitque_ovf++;
                    NDD.ndd_genstats.ndd_opackets_drop++;
                    p_next_mbuf = p_mbuf->m_nextpkt;

                    TRACE_DBG(HKWD_IEN_ISA_OTHER, "Mfre", (ulong)p_dds,
                              (ulong)p_mbuf,
                              (ulong)NDD.ndd_genstats.ndd_opackets_drop);

                    m_freem(p_mbuf);
                    p_mbuf = p_next_mbuf;
                }
            }
        }
        else
        {
            /* there is a free transmit element available */

            /* Get a device driver transmit element. */
            GET_FREE_ELEM(xlm);
            xlm->mbufp       = p_mbuf;
            xlm->in_use      = 1;
            xlm->bytes       = p_mbuf->m_pkthdr.len;

            TRACE_DBG(HKWD_IEN_ISA_OTHER, "WAIT", (ulong)WRK.xmits_queued,
                      (ulong)0, (ulong)p_mbuf->m_pkthdr.len);


	    /** If one of the transmit buffers is ** available, we can load it.  
	     ** We will load either the current  element, or get the first one 
	     ** waiting off the wait list if there is a wait list 
	     **/
            if ( XMIT_BUF_AVAIL)
            {
                WHICH_XMIT_BUF_AVAIL(buf_avail);
		if (WRK.nbr_in_wait_q == 0)  {
			buf_avail->elem = xlm;
		}
		else {
			GET_WAIT_ELEM(buf_avail->elem);
                	ADD_2_WAIT_Q(xlm);

		}
                ient_load_buf(p_dds, buf_avail);
            }
            else
            {
                ADD_2_WAIT_Q(xlm);

                TRACE_BOTH(HKWD_IEN_ISA_OTHER, "FASq", (ulong)WRK.xmits_queued,
                           (ulong)0, 0);
            }

            /* Check if a buffer is loaded and ready to xmit */
	    /* Only attempt to transmit if other buffer not currently transmitting */
       	    if ((XMIT_BUF_READY) && !(XMIT_BUF_TRANSMITTING)) {
           	WHICH_XMIT_BUF_READY(buf_xmit_next);
            	ient_xmit(p_dds, buf_xmit_next);
            }


            /* setup for the next packet in the chain (if any more). */

            p_next_mbuf       = p_mbuf->m_nextpkt;
            p_mbuf->m_nextpkt = NULL;

            if (WRK.xmits_queued > NDD.ndd_genstats.ndd_xmitque_max)
            {
                NDD.ndd_genstats.ndd_xmitque_max = WRK.xmits_queued;
            }

            TRACE_SYS(HKWD_IEN_ISA_XMIT, "WQUE", (ulong)p_dds->seq_number,
                      (ulong)p_mbuf, (ulong)p_mbuf->m_pkthdr.len);

            p_mbuf = p_next_mbuf;
        }
    }

    unlock_enable(txpri, &TX_LOCK);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_load_buf
 *
 * FUNCTION: Loads packet into the shared Mem buffer
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * NOTES:
 *
 * RETURNS:  
 *           0 - OK
 */
/*****************************************************************************/

int
ient_load_buf(dds_t      *p_dds,       /* Device structure ptr. */
	      tx_buf_use_t *buf)   
{
    ndd_t           *p_ndd = &(NDD);
    int             rc;
    int             i;
    uint            len;
    char            residue;
    struct mbuf     *m;                /* Temp. mbuf pointer. */
    volatile ushort *dp, *cp;
    volatile uchar  *memaddr;
    volatile uchar  *dp_char, *cp_char;

    TRACE_BOTH(HKWD_IEN_ISA_OTHER, "XMTl", (ulong)buf->offset, 
		buf->elem->bytes, 0);

    buf->state = LOADING;

    /* alternate load buffers */
    if (buf->offset == BUF0)  {
	WRK.next_buf_load = BUF1;  /* Load when EMPTY */
    }
    else {
	WRK.next_buf_load = BUF0;
    }
    

    /* Call ndd_trace if it is enabled */
    if (p_ndd->ndd_trace)
    {
        (*(p_ndd->ndd_trace))(p_ndd,buf->elem->mbufp,buf->elem->mbufp->m_data,
            p_ndd->ndd_trace_arg);
    }

    /* Pad short packet with garbage */

    if (buf->elem->bytes < ENT_MIN_MTU)
        buf->elem->bytes = ENT_MIN_MTU;

    if (buf->elem->bytes > ENT_MAX_MTU) {
	assert("buf->elem->bytes > ENT_MAX_MTU");
    }

    /* Check for broadcasts and multicasts. */

    if (buf->elem->mbufp->m_flags & M_BCAST)
        buf->elem->flags |= BDC_BCAST;

    if (buf->elem->mbufp->m_flags & M_MCAST)
        buf->elem->flags |= BDC_MCAST;

    /* Map in the tx buffer */

    memaddr = iomem_att(dd_ctrl.mem_map_ptr);

    /* Copy the mbuf message out to the shared ram. */

    dp = ( ushort *) (memaddr + DDI.shared_mem_addr + buf->offset);

    m = buf->elem->mbufp;

    /* check the len of the packet */

    residue = FALSE;

    while (m)
    {
        len = m->m_len;

        /* This 'if' will never be true on the first m-buffer. */

        if (residue)   /* Check for an odd-byte boundary. */
        {
            *dp_char = *(m->m_data);
            m->m_data++;
            dp++;
            len--;
            residue = FALSE;
        }

        for (i = 0, cp = (ushort *)m->m_data; i < (len/2);i++)
        {
            *dp++ = *cp++;
        }

        if ((len & 1) == 1)
        {
            /* Now we ended on a odd-byte boundary in the m-chain.*/
            dp_char  = (uchar *) dp; 
            cp_char  = (uchar *) cp;
            *dp_char =  *cp_char;
            residue  = TRUE;
            dp_char++;
        }

        m = m->m_next; /* Get the next buffer of data in the chain. */
    }

    iomem_det((void *) memaddr);
    buf->state = READY_TO_XMIT;
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_xmit
 *
 * FUNCTION: Notifies adapter to transmit buffer which is already loaded
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * NOTES:
 *
 * RETURNS:  
 *           0 - OK
 */
/*****************************************************************************/


ient_xmit(dds_t      *p_dds,       /* Device structure ptr. */
          tx_buf_use_t  *buf)	   /* which xmit buffer to use */

{
    volatile uchar  *ioaddr;

    TRACE_BOTH(HKWD_IEN_ISA_OTHER, "XMTt", (ulong)buf->offset, 
		buf->elem->bytes, 0);

    buf->state = TRANSMITTING;

    /* Start the xmit timeout timer  */

    WRK.xmit_wdt_active = TRUE;
    TXWDT.count         = 0;
    TXWDT.restart       = IENT_XMIT_TIMEOUT;


    TRACE_DBG(HKWD_IEN_ISA_XMIT, "XTOs", (ulong)p_dds,
             (ulong)0, 0);

    w_start(&TXWDT);

    /* Map in the IO-space. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* set the starting address for the transfer */
    *(ioaddr + DDI.io_addr + PAGE0_TPSR) = (buf->offset >> 8);

    /* Set the byte count. */
    *(ioaddr + DDI.io_addr + PAGE0_TBCR0) = buf->elem->bytes & 0xff;
    *(ioaddr + DDI.io_addr + PAGE0_TBCR1) = buf->elem->bytes >> 8;

    /* Initiate the transmission. */
    eieio();

    *(ioaddr + DDI.io_addr + PAGE0_COMMAND) =
                    IEN_ISA_NO_DMA | IEN_ISA_START_TX | IEN_ISA_START_NIC;

    /* For netpmon performance tool */
    TRACE_SYS(HKWD_IEN_ISA_XMIT, TRC_WEND, WRK.nbr_in_wait_q,
              (int)buf->elem->mbufp, (ulong)buf->elem->bytes);

    iomem_det((void *) ioaddr);

    TRACE_BOTH(HKWD_IEN_ISA_OTHER, "XMTe",(ulong)buf->elem->bytes, 
			(ulong)buf->offset, 0);
    return(0);
}
