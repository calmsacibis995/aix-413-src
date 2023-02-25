static char sccsid[] = "@(#)49	1.14  src/rspc/kernext/isa/itok/itok_rw.c, isatok, rspc41J, 9518A_all 4/28/95 17:00:38";
/*
** COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
**
** FUNCTIONS: tok_setup_tx(), tok_ssb_response(), tok_tx(), tok_16bit_tx(),
**            tok_our_own(), tok_our_own_16bit(), tok_rcv(), tok_16bit_rcv(),
**            tok_xmit_timeout(), tok_output().
**
** ORIGINS: 27
**
** IBM CONFIDENTIAL -- (IBM Confidential Restricted when
** combined with the aggregated modules for this product)
**                  SOURCE MATERIALS
** (C) COPYRIGHT International Business Machines Corp. 1994
** All Rights Reserved
**
** US Government Users Restricted Rights - Use, duplication or
** disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#include "itok_proto.h"

extern dd_ctrl_t    dd_ctrl;

/*****************************************************************************/
/*
** NAME:                  tok_setup_tx
**
** FUNCTION:              Setup a transmit request in the SRB area.
**
** EXECUTION ENVIRONMENT: process or interrupt level
**
** NOTES:                 Use 8-bit accesses for the shared memory.
**
** CALLED BY:             tokintr           (itok_intr.c)
**                        tok_xmit_timeout
**                        tok_output
**
** RETURNS:               0 or errorcode.
**
*/
/*****************************************************************************/

int
tok_setup_tx(dds_t  *p_dds)              /* Device structure ptr.   */
{
    int           rc;                    /* Function return code. */
    volatile uchar *memaddr;             /* Shared mem. offset.   */
    volatile uchar *aca_area, *srb_ptr;  /* Shared mem. areas.    */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TSSb",(ulong)p_dds,(ulong)WRK.xmits_queued,
          (ulong)WRK.tx_srb_waiting);

    rc = 0;

    /*
    **  Check that the adapter is open.
    */

    if (WRK.adap_state != OPEN_STATE)
    {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TSS1",(ulong) p_dds,
           (ulong) WRK.adap_state, (ulong)ENOTREADY);
        return(ENOTREADY);
    }

    /*
    ** Map in the SRB shared memory area.
    */
    memaddr = iomem_att(dd_ctrl.mem_map_ptr);

    srb_ptr = memaddr + DDI.shared_mem_addr + WRK.srb.offset;

    WRK.srb.busy = TRUE;

    /*
    ** Set up the tx command.
    */

    *(srb_ptr)                           = SRB_TX_DIR_FRAME;
    *(srb_ptr + 1)                       = 0;
    *(srb_ptr + SRB_RETCODE_OFFSET)      = SRB_INITIATED;
    *(srb_ptr + 3)                       = 0;
    *(srb_ptr + SRB_STATION_ID_OFFSET)   = (uchar) 0;
    *(srb_ptr + SRB_STATION_ID_OFFSET+1) = (uchar) 0;

    /*
    **  Make sure that the powerpc chip doesn't change the sequence.
    */
    eieio();

    /*
    ** If the xmit timeout timer is not active, start it.
    */

    if (!WRK.xmit_wdt_active)
    {
        WRK.xmit_wdt_active   = TRUE;
        XMITWDT.count         = 0;
        XMITWDT.restart       = TOK_XMIT_TIMEOUT;

        /*
        ** Save the current number transmitted, so when the timer
        ** expires, we'll just restart it if ndd_opackets_lsw has
        ** changed (indicating a succesfull transmission).
        */

        WRK.xmit_wdt_opackets = NDD.ndd_genstats.ndd_opackets_lsw;

        TRACE_DBG(HKWD_ITOK_ISA_XMIT,"XTOs",(ulong)p_dds,
          (ulong)WRK.xmit_wdt_opackets,(ulong)0);
        w_start(&XMITWDT);
    }

    /*
    ** Map in the ACA shared memory area.
    */

    aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr + 
                                                        ACA_AREA_OFFSET;

    /* Send it. */
    *(aca_area + ISRA_ODD_SET) = 0x28;
    eieio();

    /* Detach the aca area. */
    iomem_det(memaddr);

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TSSe",(ulong)WRK.xmits_queued,
          (ulong)rc, (ulong)WRK.tx_srb_waiting);

    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  tok_ssb_response
**
** FUNCTION:              Handles a ssb adapter response, i.e. a xmit resp.
**
** EXECUTION ENVIRONMENT: interrupt level
**
** NOTES:                 The correlator is not really used here, but gets
**                        important when using more than one tx-buffer.
**                        Don't ever use more than two (2) tx-buffers, the
**                        adapter might not work then (firmware problem).
**
** CALLED BY:             tokintr    (itok_intr.c)
**
** RETURNS:               0 or errorcode.
**
*/
/*****************************************************************************/

int
tok_ssb_response(dds_t *p_dds)           /* Device structure ptr. */
{
    int            rc;                   /* Function return code.            */
    xmt_elem_t     *tx_elem;             /* Actual tx-element in the wait-q. */
    uchar          correlator;           /* Transmission handle.             */
    uchar          retcode;              /* Adapter xmit return code.        */
    uchar          tx_error;             /* Additional tx error code.        */
    volatile uchar *memaddr;             /* Shared mem. offset.              */
    volatile uchar *ssb_ptr;             /* Shared mem. area.                */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"SSBb",(ulong)p_dds,
          (ulong)WRK.tx_srb_waiting,(ulong)WRK.xmits_queued);

    rc = 0;

    /* Map in the SSB area. */
    memaddr = iomem_att(dd_ctrl.mem_map_ptr);

    ssb_ptr = memaddr + DDI.shared_mem_addr + WRK.ssb.offset;

    /*
    ** Get the tx correlator.
    */

    correlator = (uchar)(*(ssb_ptr + 1));
    retcode    = (uchar)(*(ssb_ptr + 2));

    if (retcode == 0x22)
    {
        tx_error = (uchar)(*(ssb_ptr + 6));
    }

    iomem_det(memaddr);

    /* Find the transmission element information. */
    tx_elem = (xmt_elem_t *) WRK.xmit_correlators[correlator];

    /* Update the transmission statistics. */
    if (retcode)
    {
        /* We had a transmission error. */

        NDD.ndd_genstats.ndd_oerrors++;

        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SSB1",(ulong)p_dds,
           (ulong)retcode,(ulong)tx_error);

        /* Log an error to the error log. */
    }
    else
    {
        /* We had a good transmission. */
        if (ULONG_MAX == NDD.ndd_genstats.ndd_opackets_lsw)
            NDD.ndd_genstats.ndd_opackets_msw++;

        NDD.ndd_genstats.ndd_opackets_lsw++;

        if ((ULONG_MAX - tx_elem->bytes) < NDD.ndd_genstats.ndd_obytes_lsw)
            NDD.ndd_genstats.ndd_obytes_msw++;

        NDD.ndd_genstats.ndd_obytes_lsw += tx_elem->bytes;

        TRACE_SYS(HKWD_ITOK_ISA_XMIT,"WEND",(ulong)p_dds->seq_number,
          (ulong)tx_elem->mbufp,(ulong)tx_elem->bytes);

        if (tx_elem->mbufp->m_flags & M_MCAST)
        {
            TOKSTATS.mcast_xmit++;
        }

        if (tx_elem->mbufp->m_flags & M_BCAST)
        {
            TOKSTATS.bcast_xmit++;
        }

    }

    /* Now we are done with the mbuf, free it. */
    m_freem(tx_elem->mbufp);

    /* Adjust the linked wait list. */
    tx_elem->in_use = 0;

    ADD_2_FREE_Q(tx_elem);  /* Add the element to the free list. */

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"SSBe",(ulong)correlator,
          (ulong)WRK.xmits_queued,(ulong)retcode);

    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  tok_tx
**
** FUNCTION:              8 bit transmit rountine.
**
** EXECUTION ENVIRONMENT: interrupt level
**
** NOTES:                 All shared memory accesses must be 8 bit wide.
**
** CALLED BY:             tokintr  (itok_intr.c)  via WRK.tx_func
**
** RETURNS:               0
**
*/
/*****************************************************************************/

int
tok_tx(dds_t  *p_dds,                    /* Device structure ptr. */
       uchar  correlator,                /* Current xmit handle.  */
       ushort offset,                    /* Tx-buffer offset.     */
       volatile uchar *memaddr)          /* Shared mem. offset.   */
{
    int            i;                    /* Simple loop counter.        */
    int            rc;                   /* Function return code.       */
    struct mbuf    *m;                   /* Temp. mbuf pointer.         */
    xmt_elem_t     *cur_elem;            /* Info on current packet.     */
    volatile uchar *tx_target_addr, *dp, /* Shared mem. areas.          */
                   *cp, *asb_ptr;

    TRACE_DBG(HKWD_ITOK_ISA_XMIT,"TXTb",(ulong)correlator,(ulong)offset,
          (ulong)WRK.tx_last_free);

    rc = 0;

    /* Map in the tx buffer */
    tx_target_addr = memaddr + DDI.shared_mem_addr + offset;

    GET_ARB_ELEM(cur_elem);

    /* Save the xmit element with the correlator as key. */
    WRK.xmit_correlators[correlator] = cur_elem;

    /* Copy the mbuf message out to the shared ram. */
    dp = ( uchar *) tx_target_addr;

    m = cur_elem->mbufp;

    while (m)
    {
        for (i = 0,cp = mtod(m, caddr_t);i < m->m_len;i++)
            *dp++ = (uchar) *cp++;
        m = m->m_next;
    }

    /* Map in the ASB area. */
    asb_ptr = memaddr + DDI.shared_mem_addr + WRK.asb.offset;


    *(asb_ptr)       = SRB_TX_DIR_FRAME;    /* Setup the ASB response block. */
    *(asb_ptr + 1)   = (uchar) correlator;
    *(asb_ptr + 2)   = 0;
    *(asb_ptr + 4)   = (uchar) 0;
    *(asb_ptr + 5)   = (uchar) 0;
    *(asb_ptr + 6)   = (uchar) ((cur_elem->bytes >> 8) & 0xff);
    *(asb_ptr + 7)   = (uchar) (cur_elem->bytes & 0xff);

    /*
    ** Make sure that the powerpc chip doesn't change the sequence.
    */
    eieio();

    TRACE_DBG(HKWD_ITOK_ISA_XMIT,"TXTe",(ulong)rc, (ulong)correlator,
          (ulong)cur_elem->bytes);

    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  tok_16bit_tx
**
** FUNCTION:              16 bit transmit routine
**
** EXECUTION ENVIRONMENT: interrupt level
**
** NOTES:                 Most shared memory accesses should be 16 bit wide,
**                        don't try 32 bit ... won't work.
**
** CALLED BY:             tokintr    (itok_intr.c)  via WRK.tx_func
**
** RETURNS:               0
**
*/
/*****************************************************************************/

int
tok_16bit_tx(dds_t          *p_dds,      /* Device structure ptr. */
             uchar          correlator,  /* Current xmit handle.  */
             ushort         offset,      /* Tx-buffer offset.     */
             volatile uchar *memaddr)    /* Shared mem. offset.   */
{
    int             i;                   /* Simple loop counter.             */
    int             rc;                  /* Function return code.            */
    uint            len;                 /* Length of current packet segment.*/
    char            residue;             /* Is packet length an odd number?  */
    struct mbuf     *m;                  /* Temp. mbuf pointer.              */
    xmt_elem_t      *cur_elem;           /* Packet information.              */
    volatile ushort *dp, *cp,            /* Shared mem areas (16 bit).       */
                    *tx_target_addr;
    volatile uchar  *dp_char, *cp_char,  /* Shared mem areas (8 bit).        */
                    *asb_ptr;

    TRACE_DBG(HKWD_ITOK_ISA_XMIT,"TXTb",(ulong)correlator,(ulong)offset,
          (ulong)WRK.tx_last_free);

    rc = 0;

    /* Map in the tx buffer */
    tx_target_addr = (ushort *)(memaddr + DDI.shared_mem_addr + offset);

    GET_ARB_ELEM(cur_elem);

    /* Save the xmit element with the correlator as key. */
    WRK.xmit_correlators[correlator] = cur_elem;

    /* Copy the mbuf message out to the shared ram. */
    dp = (ushort *) tx_target_addr;

    m = cur_elem->mbufp;
    residue = FALSE;

    while (m)
    {
        len = m->m_len;

	if (len == 0) {
        	m = m->m_next;    /* Get the next buffer of data in the chain. */
		continue;
	}

        /* This if will never be true on the first m-buffer. */
        if (residue)       /* Check for an odd-byte boundary. */
        {
            *dp_char = *(m->m_data);
            m->m_data++;

            /*
            ** Adapter has to be on a 16-bit boundary.
            */

            dp++;
            len--;
            residue = FALSE;
        }

        for (i = 0, cp = (ushort *)m->m_data; i < (len/2); i++)
            *dp++ = *cp++;

        if ((len & 1) == 1)
        {
            dp_char  = (uchar *) dp; 
            cp_char = (uchar *) cp;
            *dp_char =  *cp_char;
            dp_char++;
            residue  = TRUE;

            /*
            ** Now we ended on a odd-byte boundary in the m-chain.
            */
        }
        m = m->m_next;        /* Get the next buffer of data in the chain. */
    }

    /* Map in the ASB area. */
    asb_ptr = memaddr + DDI.shared_mem_addr + WRK.asb.offset;

    /* Setup the ASB response block. */
    *(asb_ptr)       = SRB_TX_DIR_FRAME;
    *(asb_ptr + 1)   = (uchar) correlator;
    *(asb_ptr + 2)   = 0;
    *(asb_ptr + 4)   = (uchar) 0;
    *(asb_ptr + 5)   = (uchar) 0;
    *(asb_ptr + 6)   = (uchar) ((cur_elem->bytes >> 8) & 0xff);
    *(asb_ptr + 7)   = (uchar) (cur_elem->bytes & 0xff);

    /*
    ** Make sure that the powerpc chip doesn't change the sequence.
    */
    eieio();

    TRACE_DBG(HKWD_ITOK_ISA_XMIT,"TXTe",(ulong)rc,(ulong)correlator,
          (ulong)cur_elem->bytes);

    return(rc);
}


/*****************************************************************************/
/*
**  NAME:                  tok_our_own                     
**
**  FUNCTION:              Check for our own tokenring address. 
**
**  EXECUTION ENVIRONMENT: Interrupt level
**
**  NOTES:                 8 bit version.
** 
**  CALLED BY:             tok_rcv
**
**  RETURNS:               TRUE if the address in question is our own.
*/
/*****************************************************************************/

int
tok_our_own(dds_t          *p_dds,      /* Device structure ptr.         */
            volatile uchar *src_addr)   /* Points to the tokenring addr. */
{
    int   i;                            /* Simple loop counter.  */
    int   rc;                           /* Function return code. */
    uchar *our_addr;                    /* Temp. ptr.            */

    rc = TRUE;

    our_addr = dd_ctrl.adap_cfg[WRK.minor_no].aip.encoded_hw_addr;

    /*
    ** Wipe out a possible route bit in the first character.
    */

    if (*our_addr == (*src_addr & 0x7F))
    {
        *our_addr++;
        *src_addr++;

        for (i = 0; (i < (CTOK_NADR_LENGTH - 1)) && rc; i++)
        {
            if (*our_addr++ != *src_addr++)
                rc = FALSE;
        }
    }
    else
    {
        rc = FALSE;
    }

    return(rc);
}


/*****************************************************************************/
/*                                                        
**  NAME:                  tok_our_own_16bit
**                                                          
**  FUNCTION:              Check for our own tokenring address.
**                                                             
**  EXECUTION ENVIRONMENT: interrupt level
**
**  NOTES:                 16 bit version.
**
**  CALLED BY:             tok_16bit_rcv
**
**  RETURNS:               TRUE if the address in question is our own. 
*/
/*****************************************************************************/

int
tok_our_own_16bit(dds_t           *p_dds,      /* Device structure.      */
                  volatile ushort *src_addr)   /* Points to the address. */
{
    int    i;                                  /* Simple loop counter.   */
    int    rc;                                 /* Function return code.  */
    ushort *our_addr;                          /* Temp ptr.              */

    rc = TRUE;

    our_addr = (ushort *) dd_ctrl.adap_cfg[WRK.minor_no].aip.encoded_hw_addr;

    /* Wipe out a possible route bit in the first character. */

    if (*our_addr == (*src_addr & 0x7FFF))
    {
        *our_addr++;
        *src_addr++;

        for (i = 0; (i < 2) && rc; i++)
        {
            if (*our_addr++ != *src_addr++)
                rc = FALSE;
        }
    }
    else
        rc = FALSE;

    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  tok_rcv
**
** FUNCTION:              8 bit data receive routine
**
** EXECUTION ENVIRONMENT: interrupt level
**
** NOTES:                 The incoming data packet can be spread out over
**                        several shared mem. buffers.
**
** CALLED BY:             tokintr   (itok_intr.c)  via WRK.rx_func
**
** RETURNS:               0
**
*/
/*****************************************************************************/

int
tok_rcv(dds_t          *p_dds,              /* Device structure ptr.         */
        volatile uchar *arb_area,           /* Attached shared mem. area.    */
        volatile uchar *memaddr)            /* Attached shared mem. area.    */
{
    int     i;                              /* Simple loop counter.          */
    int     rc;                             /* Function return code.         */
    uchar   station_id[2];                  /* Our adapter station id.       */
    ushort  first,                          /* First part of the packet.     */
    offset,                                 /* Running offset.               */
    frame_length,                           /* Entire packet length.         */
    buffer_length,                          /* Current buffer length.        */
    next_buffer_offset;                     /* Possible next part of packet. */
    struct mbuf    *mbufp;                  /* Where to put the data.        */
    uchar          *dp;                     /* Running mbufp data pointer.   */
    volatile uchar *old_cp;                 /* Shared mem areas.             */
    volatile uchar *cp, *base, *asb_ptr;    /* Shared mem. areas.            */

    TRACE_DBG(HKWD_ITOK_ISA_RECV,"RCVb",(ulong)p_dds,(ulong)arb_area,(ulong)0);

    rc = 0;

    /*
    ** Get the offset to the first receive buffer (one packet can span
    ** more than one receive buffer.
    */

    first = offset = (((*(arb_area + RCV_BUFFER_OFFSET)) & 0xff) << 8) +
                       (*(arb_area + RCV_BUFFER_OFFSET + 1) & 0xff);

    station_id[0] = (*(arb_area + RCV_STATION_OFFSET)) & 0xff;
    station_id[1] =  (*(arb_area + RCV_STATION_OFFSET + 1)) & 0xff;

    /* Get the length of the entire packet. */
    frame_length = (((*(arb_area + RCV_FRAME_LEN_OFFSET)) & 0xff) << 8) +
                     (*(arb_area + RCV_FRAME_LEN_OFFSET + 1) & 0xff);

    /* Check for a valid packet size. */
    if ((frame_length >= TOK_MIN_DATA) || (frame_length <= WRK.max_frame_len))
    {
        /*
        ** Packet appears to be good.
        */

        /* Netpmon trace entry point */
        TRACE_SYS(HKWD_ITOK_ISA_RECV,"RDAT",(ulong)p_dds->seq_number,
		  (ulong)frame_length,(ulong)0);

	/* Map in the (first) receive buffer. */
	base = memaddr + DDI.shared_mem_addr;

	cp = base + offset;

	/*
	** Check that we are not receiving one of our own broadcast packets.
	*/

	if (!tok_our_own(p_dds, (cp + RCV_BUFF_HDR_LENGTH + 8)))
	{
	    /*
	    **  Get a mbuf for the received packet.
	    */

	    if ((frame_length) <= MHLEN)
	    {
		mbufp = m_gethdr(M_DONTWAIT, MT_HEADER);
	    }
	    else
	    {
		if (mbufp = m_getclustm(M_DONTWAIT,MT_DATA, frame_length))
	        {
		    mbufp->m_flags |= M_PKTHDR;
		}
	    }

	    if (mbufp != NULL)
	    {
		mbufp->m_len   = frame_length;
		dp = (uchar *) mbufp->m_data;

		/*
		** Get the frame from one or more receive buffers.
		*/
		offset = RCV_BUFF_HDR_LENGTH;

		buffer_length = (((*(cp + BUFFER_LEN_OFFSET)) & 0xff) << 8) +
		                 ((*(cp + BUFFER_LEN_OFFSET + 1)) & 0xff);
		next_buffer_offset = NEXT_BUFFER_OFFSET;

		/*
		** Now get the stuff that's the purpose with
		** all this , i.e. LLC-header and DATA.
		*/

		do {
		    old_cp = cp;
		    cp += (ushort) offset;

		    for (i = 0; i < buffer_length; i++)
			*dp++ = *cp++;

		    offset = (((*(old_cp + next_buffer_offset)) & 0xff) << 8)
			      + ((*(old_cp + next_buffer_offset + 1)) & 0xff);

		    if (offset > 0)
                    {
			cp = base + offset;

			buffer_length = (((*(cp + BUFFER_LEN_OFFSET-2)) &
					  0xff) << 8)
			                  + ((*(cp + BUFFER_LEN_OFFSET-1)) &
				          0xff);

			offset = RCV_BUFF_HDR_LENGTH - 2;
			next_buffer_offset = 0;
		    }
		} while (offset > 0);

		TRACE_SYS(HKWD_ITOK_ISA_RECV,"RNOT",(ulong)p_dds->seq_number,
			  (ulong)mbufp,(ulong)frame_length);

		/* Update the receive statistics. */
		if (ULONG_MAX == NDD.ndd_genstats.ndd_ipackets_lsw)
		{
		    NDD.ndd_genstats.ndd_ipackets_msw++;
		}

		NDD.ndd_genstats.ndd_ipackets_lsw++;

		if ((ULONG_MAX - mbufp->m_len) <
		    NDD.ndd_genstats.ndd_ibytes_lsw)
		{
		    NDD.ndd_genstats.ndd_ibytes_msw++;
		}
		NDD.ndd_genstats.ndd_ibytes_lsw += mbufp->m_len;

		/*
		** Check for broadcasts and group addresses.
		*/

		if (*(uchar *)((mbufp->m_data)+2) & GROUP_ADR_MASK)
		{
		    if (*((int *)((mbufp->m_data)+4)) == -1)
		    {
			mbufp->m_flags |= M_BCAST;
			TOKSTATS.bcast_recv++;
		    }
		    else
		    {
			mbufp->m_flags |= M_MCAST;
			TOKSTATS.mcast_recv++;
		    }
		}

		/* Deliver the data to the user. */
		mbufp->m_pkthdr.len = mbufp->m_len;
		NDD.nd_receive(&NDD, mbufp);

		TRACE_SYS(HKWD_ITOK_ISA_RECV,"REND",(ulong)mbufp,(ulong)0,
			  (ulong)0);

	    }
	    else
	    {
		/* No mbufs currently available. */
		NDD.ndd_genstats.ndd_nobufs++;
	    }
	} /* Not our own broadcast message. */
    }
    else  /* Packet didn't meet our min or max criteria. */
    {
        TRACE_DBG(HKWD_ITOK_ISA_RECV,"RCV1",(ulong) frame_length,(ulong) 0,
          (ulong) 0);
        NDD.ndd_genstats.ndd_ibadpackets++;
    }

    /* Map in the ASB area. */
    asb_ptr = memaddr + DDI.shared_mem_addr + WRK.asb.offset;

    /* Setup the ASB response block. */
    *(asb_ptr )      = ARB_RECEIVED_DATA;
    *(asb_ptr + 2)   = 0;
    *(asb_ptr + 4)   = station_id[0];
    *(asb_ptr + 5)   = station_id[1];
    *(asb_ptr + 6)   = (uchar) ((first >> 8) & 0xff);
    *(asb_ptr + 7)   = (uchar) (first & 0xff);

    /* Make sure that the powerpc chip doesn't change the sequence. */
    eieio();

    TRACE_DBG(HKWD_ITOK_ISA_RECV,"RCVe",(ulong)rc,(ulong)frame_length,
	      (ulong)0);
    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  tok_16bit_rcv
**
** FUNCTION:              16 bit data receive routine.
**
** EXECUTION ENVIRONMENT: interrupt level
**
** NOTES:                 The incoming data packet can be spread out over
**                        several shared mem. buffers.
**
** CALLED BY:             tokintr   (itok_intr.c)  via WRK.rx_func
**
** RETURNS:               0
**
*/
/*****************************************************************************/

int
tok_16bit_rcv(dds_t           *p_dds,    /* Device structure ptr.            */
              volatile ushort *arb_area, /* Attached shared mem. area.       */
              volatile uchar  *memaddr)  /* Attached shared mem. area.       */
{
    int             i;                   /* Simple loop counter.             */
    int             rc;                  /* Function return code.            */
    ushort          station_id,          /* Our adapter station id.          */
                    first,               /* First part of the packet.        */
                    offset,              /* Running offset.                  */
                    frame_length,        /* Length of the entire packet.     */
                    even_frame_length,   /* Length rounded to nearest even #.*/
                    buffer_length,       /* Packet part length.              */
                    next_buffer_offset,  /* Address of the next packet buffer.*/
                    *dp;                 /* Data destination pointer.        */
    struct mbuf     *mbufp;              /* Where to put the data.           */
    volatile ushort *cp, *base,          /* Shared mem areas (16 bit).       */
                    *arb_16bit_area;
    volatile ushort *old_cp;             /* Shared mem areas (16 bit).       */
    volatile uchar  *asb_ptr;            /* Shared mem area.                 */

    TRACE_DBG(HKWD_ITOK_ISA_RECV,"RCVb",(ulong)p_dds,(ulong)arb_area,(ulong)0);

    rc = 0;

    arb_16bit_area = (ushort *) arb_area;

    /*
    ** Get the offset to the first receive buffer (one packet can span
    ** more than one receive buffer.
    */

    first = offset = (ushort)(*(arb_16bit_area + 3));
    station_id = (ushort)(*(arb_16bit_area + 2));

    /* Get the length of the entire packet. */
    even_frame_length = frame_length = (ushort)(*(arb_16bit_area + 5));

    /* Check for a valid packet size. */

    if ((frame_length >= TOK_MIN_DATA) || (frame_length <= WRK.max_frame_len))
    {
	/* Packet appears to be good */
	/* Netpmon trace entry point */
	TRACE_SYS(HKWD_ITOK_ISA_RECV,"RDAT",(ulong)p_dds->seq_number,
		  (ulong)frame_length,(ulong)0);

	/* Pad to an even number of bytes. */
	if (even_frame_length & 0x01)
	    even_frame_length++;

	/* Map in the (first) receive buffer. */
	base = (ushort *)(memaddr + DDI.shared_mem_addr);
	cp = base + (offset/2);

        /*
        ** Check that we are not receiving one of our own broadcast packets.
        */
    
        if (!tok_our_own_16bit(p_dds, (cp + 8)))
        {
            /*  get a mbuf for the received packet */
            if ((even_frame_length) <= MHLEN)
            {
                mbufp = m_gethdr(M_DONTWAIT, MT_HEADER);
            }
            else
            {
                if (mbufp = m_getclustm(M_DONTWAIT, MT_DATA, even_frame_length))
                {
                    mbufp->m_flags |= M_PKTHDR;
                }
            }
    
            if (mbufp != NULL)
            {
                mbufp->m_len = frame_length;
                dp = (ushort *) mbufp->m_data;
    
                /* Get the frame from one or more receive buffers. */
                offset = 4;
                buffer_length = *(cp + 3);
                next_buffer_offset = 1;
    
                /*
                ** Now get the stuff that's the purpose with
                ** all this , i.e. LLC-header and DATA.
                */
    
                do {
                    /*
                    ** Make sure that the buffer length is an even number of
                    **  bytes.
                    */
    
                    if (buffer_length & 0x01)
                        buffer_length++;
    
                    old_cp = cp;
                    cp += (ushort) offset;
    
                    for (i = 0; i < (buffer_length / 2); i++)
                    {
                        *dp++ = *cp++;
                    }
    
                    offset = *(old_cp + next_buffer_offset);
    
                    if (offset > 0)
                    {
                        cp = base + (offset/2);
                        buffer_length = *(cp + 2);
                        offset = 3;
                        next_buffer_offset = 0;
                    }
                } while (offset > 0);
    
                TRACE_SYS(HKWD_ITOK_ISA_RECV,"RNOT",(ulong)p_dds->seq_number,
                  (ulong)mbufp,(ulong)frame_length);
    
                /* Update the receive statistics. */
                if (ULONG_MAX == NDD.ndd_genstats.ndd_ipackets_lsw)
                {
                    NDD.ndd_genstats.ndd_ipackets_msw++;
                }
    
                NDD.ndd_genstats.ndd_ipackets_lsw++;
    
                if ((ULONG_MAX - mbufp->m_len) < NDD.ndd_genstats.ndd_ibytes_lsw)
                {
                    NDD.ndd_genstats.ndd_ibytes_msw++;
                }
                NDD.ndd_genstats.ndd_ibytes_lsw += mbufp->m_len;
    
                /* Check for broadcasts and group addresses. */
                if (*(uchar *)((mbufp->m_data)+2) & GROUP_ADR_MASK)
                {
                    if (*((int *)((mbufp->m_data)+4)) == -1)
                    {
                        mbufp->m_flags |= M_BCAST;
                        TOKSTATS.bcast_recv++;
                    }
                    else
                    {
                        mbufp->m_flags |= M_MCAST;
                        TOKSTATS.mcast_recv++;
                    }
                }
    
                /* Send the data up to the user. */
                mbufp->m_pkthdr.len = mbufp->m_len;
                NDD.nd_receive(&NDD, mbufp);
    
                TRACE_SYS(HKWD_ITOK_ISA_RECV,"REND", (ulong)mbufp, (ulong)0,
                         (ulong)0);
            }
            else
            {
                /* No mbufs currently available. */
                NDD.ndd_genstats.ndd_nobufs++;
            }
    
        } /* Not our own broadcast message. */
    }
    else /* Packet didn't meet Min or Max criteria. */
    {
        TRACE_BOTH(HKWD_ITOK_ISA_RECV,"RCV1",(ulong) frame_length, (ulong)0,
                   (ulong)0);
        NDD.ndd_genstats.ndd_ibadpackets++;
    }

    /* Map in the ASB area. */
    asb_ptr = (uchar *)(memaddr + DDI.shared_mem_addr + WRK.asb.offset);

    /* Setup the ASB response block. */
    *(asb_ptr )       = ARB_RECEIVED_DATA;
    *(asb_ptr + 2)   = 0;
    *(asb_ptr + 4)   = (uchar) (station_id >> 8) & 0xff;
    *(asb_ptr + 5)   = (uchar) (station_id & 0xff);
    *(asb_ptr + 6)   = (uchar) (first >> 8) & 0xff;
    *(asb_ptr + 7)   = (uchar) (first & 0xff);

    /* Make sure that the powerpc chip doesn't change the sequence. */
    eieio();

    TRACE_DBG(HKWD_ITOK_ISA_RECV,"RCVe",(ulong)rc,(ulong)frame_length,(ulong)0);
    return(rc);
}


/*****************************************************************************/
/*
** NAME:                  tok_xmit_timeout
**
** FUNCTION:              Handles a transmission that did not complete.
** 
** EXECUTION ENVIRONMENT: interrupt level
**
** NOTES:
**
** RETURNS:               nothing
*/
/*****************************************************************************/

void
tok_xmit_timeout(struct watchdog *p_wdt)          /* Watchdog structure ptr. */
{
    dds_t      *p_dds = *(dds_t **) ((ulong)p_wdt + sizeof(struct watchdog));
    xmt_elem_t *cur_elem;
    int        txpri;

    TRACE_BOTH(HKWD_ITOK_ISA_XMIT,"xtoB",(ulong)p_dds,(ulong)WRK.xmits_queued,
           (ulong)0);

    txpri = disable_lock(PL_IMP, &TX_LOCK);

    /*
    ** If the wait transmit queue is empty, nothing to do.
    */

    if (!WRK.xmits_queued)
    {
        WRK.xmit_wdt_active = FALSE;
        unlock_enable(txpri, &TX_LOCK);
        return;
    }

    /*
    ** The transmit wait queue is not empty, if any packets have been
    ** transmitted, just restart the timer and check again later.
    */

    if (WRK.xmit_wdt_opackets != NDD.ndd_genstats.ndd_opackets_lsw)
    {
        TRACE_DBG(HKWD_ITOK_ISA_XMIT, "XTOs",(ulong)p_dds,
          (ulong)WRK.xmit_wdt_opackets,
          (ulong)NDD.ndd_genstats.ndd_opackets_lsw);

        WRK.xmit_wdt_active   = TRUE;              /* Just making sure. */
        XMITWDT.count         = 0;
        XMITWDT.restart       = TOK_XMIT_TIMEOUT;
        WRK.xmit_wdt_opackets = NDD.ndd_genstats.ndd_opackets_lsw;
        w_start(&XMITWDT);

        unlock_enable(txpri, &TX_LOCK);
        return;
    }

    /*
    ** The transmission failed, log an error, and cleanup the element.
    */

    WRK.xmit_wdt_active = FALSE;
    TOKSTATS.tx_timeouts++;

    GET_ARB_ELEM(cur_elem);

    m_freem(cur_elem->mbufp);

    ADD_2_FREE_Q(cur_elem);

    logerr(p_dds, TOK_XMIT_TIMEOUT_ERROR);

    /*
    ** Check for waiting transmission elements.
    */

    if (WRK.tx_srb_waiting)
    {
        WRK.tx_srb_waiting--;

        /*
        ** Setup the SRB command and fire it off (1.st intr.).
        */
        tok_setup_tx(p_dds);
    }

    unlock_enable(txpri, &TX_LOCK);
    TRACE_BOTH(HKWD_ITOK_ISA_XMIT,"xtoE",(ulong)p_dds,(ulong)0,(ulong)0);

    return;
}


/*****************************************************************************/
/*
** NAME:                  tok_output
**
** FUNCTION:              write entry point
**
** EXECUTION ENVIORNMENT: process or interrupt level
**
** RETURNS:               0 if successful
**                        errno value on failure
*/
/*****************************************************************************/

int
tok_output (ndd_t       *p_ndd,    /* Network device driver structure ptr. */
            struct mbuf *p_mbuf)   /* Data to transmit.                    */
{
    dds_t       *p_dds;            /* Device structure ptr.                */
    int         txpri;             /* Old interrupt level to restore.      */
    xmt_elem_t  *xlm;              /* Where to store info about this xmit. */
    struct mbuf *p_first_mbuf,     /* First part of the packet(s).         */
                *p_next_mbuf, *m;  /* Running mbuf pointer.                */

    /*
    ** Get adapter structure
    */

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

    if ((WRK.adap_state != OPEN_STATE)  || (WRK.open_status != OPEN_COMPLETE))
    {
        if (WRK.adap_state == DEAD_STATE)
        {
            return(ENETDOWN);
        }
        else
        {
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
        /*
        ** Check to see if the wait queue (staging area) is full.
        ** It was full, if we did not have room for any of the packets
        ** in the chain return an error code. If we had room for
        ** at least one packet then return a good completion code.
        */

        if (WAITQ_FULL)
        {
            if (p_mbuf == p_first_mbuf)
            {
                /* No room for any of them. */
                while (p_mbuf)
                {
                    /*
                    ** For each packet in the chain, increment an error counter.
                    */
                    NDD.ndd_genstats.ndd_xmitque_ovf++;
                    p_mbuf = p_mbuf->m_nextpkt;
                }
                unlock_enable(txpri, &TX_LOCK);
                return(EAGAIN);
            }
            else
            {
                /*
                ** Since some of the mbufs fit on the wait queue, the
                ** device driver has to free those mbufs that didn't fit
                ** on the transmit queue.  No error is returned.
                */

                while (p_mbuf)
                {
                    /* Count the ones that we are dropping. */
                    NDD.ndd_genstats.ndd_xmitque_ovf++;
                    NDD.ndd_genstats.ndd_opackets_drop++;
                    p_next_mbuf = p_mbuf->m_nextpkt;

                    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"Mfre",(ulong)p_dds,
                  (ulong)p_mbuf,
                  (ulong)NDD.ndd_genstats.ndd_opackets_drop);

                    m_freem(p_mbuf);
                    p_mbuf = p_next_mbuf;
                }
            }
        }
        else
        {
            /* We have a transmit element. */
            if (p_mbuf->m_pkthdr.len <= WRK.max_frame_len )
            {
                /* Get a device driver transmit element. */
                GET_FREE_ELEM(xlm);
                xlm->mbufp       = p_mbuf;
                xlm->in_use      = 1;
                xlm->bytes       = p_mbuf->m_pkthdr.len;

                TRACE_DBG(HKWD_ITOK_ISA_OTHER,"WAIT",(ulong)WRK.xmits_queued,
                    (ulong)WRK.tx_buf_in_use,(ulong)p_mbuf->m_pkthdr.len);

                /*
                ** If there are transmit buffers available and no requests
                ** waiting to get one, then process the xmit request.
                */

                if (!WRK.srb.busy)
                {
                    /*
                    ** Setup the SRB command and fire it off (generates first
                    ** intr.).
                    */
                    tok_setup_tx(p_dds);

                }
                else
                {
                    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"FASq",(ulong)WRK.xmits_queued,
                        (ulong)WRK.tx_buf_in_use,(ulong)0);
                    WRK.tx_srb_waiting++;
                }

                /* Queue up the element. */
                ADD_2_ARB_Q(xlm);

            }
            else
            {
                /* The packet violated the max MTU size. */
                TRACE_DBG(HKWD_ITOK_ISA_OTHER,"Mbig",(ulong)p_dds,(ulong)p_mbuf,
                    (ulong)p_mbuf->m_pkthdr.len);
                NDD.ndd_genstats.ndd_opackets_drop++;
                m_freem(p_mbuf);
            }

            /*
            ** setup for the next packet in the chain (if any more).
            */

            p_next_mbuf       = p_mbuf->m_nextpkt;
            p_mbuf->m_nextpkt = NULL;

            if (WRK.xmits_queued > NDD.ndd_genstats.ndd_xmitque_max)
            {
                NDD.ndd_genstats.ndd_xmitque_max = WRK.xmits_queued;
            }

            TRACE_SYS(HKWD_ITOK_ISA_XMIT,"WQUE",(ulong)p_dds->seq_number,
                (ulong)p_mbuf,(ulong)p_mbuf->m_pkthdr.len);
            p_mbuf = p_next_mbuf;
        }
    }

    unlock_enable(txpri, &TX_LOCK);
    return(0);
}
