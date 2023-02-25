static char sccsid[] = "@(#)50	1.7  src/rspc/kernext/isa/itok/itok_intr.c, isatok, rspc41J, 9523A_all 6/5/95 17:45:58";
/*
** COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
**
** FUNCTIONS: tokintr()
**           
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

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
** NAME:                  tokintr
**
** FUNCTION:              Second level interrupt handler (slih)
**
** EXECUTION ENVIRONMENT: Interrupt level
**
** NOTES:                 Uses byte accesses, when accessing shared memory,
**                        since some adapters are 8 bit, no performance gain
**                        in using shorts for the 16 bit adapters.
**
** RETURNS:               nothing
**
*/
/*****************************************************************************/

int
tokintr(struct intr *ihsptr)
{
    int            rc;                       /* Internal return code.       */
    int            slihpri;                  /* Old priority level.         */
    dds_t          *p_dds = (dds_t *)ihsptr; /* Device structure.           */
    uchar          intr_val;                 /* What type of interrupt.     */
    uchar          correlator;               /* To keep track of xmissions. */
    volatile uchar *memaddr;                 /* Shared mem. offset.         */
    volatile uchar *aca_area, *asb_ptr,      /* Shared mem. areas.          */
                   *srb_ptr,  *ssb_ptr,
                   *arb_ptr;

    rc = 0;

    slihpri = disable_lock(PL_IMP, &SLIH_LOCK);

    /* Attach to the adapter control area shared memory. */

    memaddr = iomem_att(dd_ctrl.mem_map_ptr);

    aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr +
                                          ACA_AREA_OFFSET;

    /* if the driver is in the suspend state the adapter has */
    /* been reset, and we are not expecting any interrupts.  */
    /* It has to be our interrupt, we are not sharing any,   */
    /* so just clear it and return                           */

    if (WRK.adap_state == SUSPEND_STATE)
    {
         /* Reset the interrupt. */
         *(aca_area + ISRP_ODD_RESET) = ~intr_val;
         eieio();
	 /* detach from the adapter control area shared memory. */
    	 iomem_det(memaddr);
         unlock_enable(slihpri, &SLIH_LOCK);
         return(INTR_SUCC);
    }

    /* Disallow interrupts. */
    *(aca_area + ISRP_EVEN_RESET) = 0x40;

    /* Read the intr. value. */
    intr_val = *(aca_area + ISRP_ODD_OFFSET);

    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TINb",(ulong)intr_val,
	      (ulong)WRK.adap_state,(ulong)WRK.tx_last_free);

    /* Reset the interrupt. */
    *(aca_area + ISRP_ODD_RESET) = ~intr_val;

    /* Check for a spurious intr. */
    if (!intr_val)
        goto reenable_intr;   /* Must re-enable the interrupt controller. */

    /* Check if we are waiting on that special open done interrupt. */

    if (WRK.adap_state == OPEN_PENDING)
    {
                                     /* This is the special open-interrupt. */
        rc = tok_finish_open(p_dds);
        if (rc)
	{
            TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TIN1",(ulong)intr_val,
		      (ulong)WRK.adap_state,(ulong)rc);
        }
    }
    else                             /* Normal processing interrupt. */
    {

#ifdef ASB_STUFF
        if (ASB_COMMAND)
        {
            uchar asb_retcode;

            /* We have an adapter status block request. */
            asb_ptr = memaddr + DDI.shared_mem_addr + WRK.asb.offset;

            if ((asb_retcode = *(asb_ptr + 2)) != ASB_AVAILABLE)
            {
                /*
		** Not much we can do here, if serious errors exists, they
                ** will be dealt with in other places, just leave this hook.
                */
            }
	}
#endif

	/* Check for a system request block response. */
	if (SRB_RESPONSE)
        {
	    uint   new_cmd;      /* Some srb-command waiting.          */
	    uchar  srb_retcode;  /* Command response from the adapter. */
	    ushort srb_opcode;   /* Command opcode.                    */

	    WRK.srb.busy = FALSE;

	    /* Map in the SRB area. */
	    srb_ptr = memaddr + DDI.shared_mem_addr + WRK.srb.offset;

	    /* A SRB request completed. */
	    srb_opcode  = (*(srb_ptr)) & 0xff;
	    srb_retcode = *(srb_ptr + SRB_RETCODE_OFFSET);

	    switch (srb_opcode)
            {
	      case SRB_TX_UI_FRAME:
	      case SRB_TX_DIR_FRAME:

	        /*
		** The adapter has just responded with the interrupt
		** from the tx-setup function, step 1 in sending a packet.
		** The command should now be in progress.
		** Now wait on the ARB interrupt (yes,more than one
		** intr/xmit !).
		*/

		if ((srb_retcode != SRB_SUCCESS) && (srb_retcode !=
						     SRB_IN_PROGRESS))
		{
		    /* Update error statistics. */
		    NDD.ndd_genstats.ndd_oerrors++;
		}
		break;

	      case SRB_OPEN_SAP:
		/* Handle the response to the 'open SAP' request. */
		tok_open_sap_cmd_resp(p_dds, srb_ptr);
		break;

	      case SRB_CLOSE_SAP:
		/* Handle the response to the 'close SAP' request. */
		tok_close_sap_cmd_resp(p_dds, srb_ptr);
		break;

	      case SRB_CLOSE:
		/* Handle the response to the 'close' request. */
		tok_close_cmd_resp(p_dds);
                break;

	      case SRB_SET_GROUP_ADDR:
                /*
		** Handle the response to the 'set group address'
		** request.
		*/
                rc = tok_grp_cmd_response(p_dds);
		if (rc)
                {
		    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TIN2",(ulong)intr_val,
			      (ulong)WRK.adap_state,(ulong)rc);
		}
		break;

	      case SRB_SET_FUNC_ADDR:
		/*
		** Handle the response to the 'set funtional address'
		** request.
		*/
		rc = tok_func_cmd_response(p_dds);
		if (rc)
                {
		    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TIN3",(ulong)intr_val,
			      (ulong)WRK.adap_state,(ulong)rc);
		}
		break;

	      case SRB_READ_LOG : 
	        /*
		** Handle the response to the 'read log' request.
		*/
		tok_read_log_cmd_resp(p_dds, srb_ptr);
		break;
	    }

	    /* Now see if there's outstanding srb-commands. */
	    if (WRK.intr_action)
            {
		TRACE_DBG(HKWD_ITOK_ISA_OTHER,"ACTb",
			  (ulong)WRK.intr_action,
			  (ulong)WRK.srb_waiting,(ulong)WRK.xmits_queued);
		/*
		** Kill the srb timeout timer, if no other commands
		** waiting.
		*/
		if ((!WRK.srb_waiting) && (WRK.srb_wdt_active))
	        {
		    /* Kill it. */
		    w_stop(&SRBWDT);
		    WRK.srb_wdt_active = FALSE;
		}

		new_cmd         = WRK.intr_action;
		WRK.intr_action = 0;

		rc = tok_issue_cmd(p_dds, new_cmd, FALSE, NULL);

	    }
	    else
	    {
		/* Check for sleeping srb process thread's. */
		if (WRK.srb_waiting)
		{
		    e_wakeup(&WRK.srb_wait_event);
		}
		else
		{
		    if (WRK.tx_srb_waiting)
		    {
			WRK.tx_srb_waiting--;
			/*
			** Setup the SRB command and fire it off
			** (1.st intr.).
			*/
			tok_setup_tx(p_dds);
		    }
		}
	    }
	}

	if (SSB_RESPONSE)
	{
	    /* The transmission is now complete. */
	    /* Stop the xmit timeout timer. */
	    w_stop(&XMITWDT);
            
	    /* Update the number of transmit interrupts. */
	    if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX)
	    {
		NDD.ndd_genstats.ndd_xmitintr_msw++;
	    }
	    NDD.ndd_genstats.ndd_xmitintr_lsw++;

	    /* Handle the system status block. */
	    tok_ssb_response(p_dds);
	    *(aca_area + ISRA_ODD_SET) = 0x01;
	}

	if (ARB_COMMAND)
	{
	    ushort arb_opcode;

	    /*
	    ** The interrupt was an adapter request block,
	    ** i.e. unsolicited msg.
	    */

	    arb_ptr = memaddr + DDI.shared_mem_addr + WRK.arb.offset;
	    arb_opcode = (*(arb_ptr)) & 0xff;

	    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TIN2",(ulong)arb_opcode,
		      (ulong)0,(ulong)0);

	    if (arb_opcode == ARB_RECEIVED_DATA)
	    {
		rc = (*(WRK.rx_func))(p_dds, arb_ptr, memaddr);
		if (rc)
		{
		    /* Update the number of receive errors. */
		    NDD.ndd_genstats.ndd_ierrors++;
		    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TIN4",(ulong)intr_val,
			      (ulong)NDD.ndd_genstats.ndd_ierrors,
			      (ulong)rc);
		}

		/* Update the number of rcv interrupts. */
		if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX)
	        {
		    NDD.ndd_genstats.ndd_recvintr_msw++;
		}
		NDD.ndd_genstats.ndd_recvintr_lsw++;

		/* Also update the MIBS statistics. */
		WRK.network_status = TR_MIB_NOPROBLEM;

		/* Acknowledge the packet. */
		*(aca_area + ISRA_ODD_SET) = 0x12;

	    }
	    else
	    {
		if (arb_opcode == ARB_TRANSMIT_DATA)
		{
		    ushort dhb_offset;
		    
		    correlator = *(arb_ptr + 1) & 0xff;
		    dhb_offset = (*(arb_ptr + 6) << 8) + (*(arb_ptr + 7)
							  & 0xff);

		    (*(WRK.tx_func))(p_dds, correlator,
				     dhb_offset, memaddr);

		    /* Now the packet should be sent. */
		    *(aca_area + ISRA_ODD_SET) = 0x12;

		    /*
		    ** We did not ask for the asb free interrupt,
		    ** so clear it.
		    */
		} 
		else
	        {
		    if (arb_opcode == ARB_RING_CHANGE)
			tok_ring_change(p_dds, ((*(arb_ptr + 6)) << 8) +
					(*(arb_ptr + 7)));
		    *(aca_area + ISRA_ODD_SET) = 0x02;
		}
	    }
	} /* End of if ARB command. */

    } /* End of not an open interrupt. */

reenable_intr:

    /* Re-enable the interrupt controller. */
    {
	int converted_intr;                   /* HW. intr. level.  */
	volatile uchar *ioaddr;               /* IO-space handle.  */

	converted_intr = dd_ctrl.adap_cfg[WRK.minor_no].intr_priority;

	/*
	** Remember on a PC, if intr level is 9 it's really 2
	** (cascaded intr).
	*/
            
	if (converted_intr == 9)
	    converted_intr = 2;
            
	ioaddr = iomem_att(dd_ctrl.io_map_ptr);
	*(ioaddr + 0x02f0 + converted_intr) = 0x01;
	iomem_det(ioaddr);
    }

    /* Reallow adapter interrupts. */
    *(aca_area + ISRP_EVEN_SET) = 0x40;
    iomem_det(memaddr);

    unlock_enable(slihpri, &SLIH_LOCK);
    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TINe",(ulong)rc,(ulong)0,(ulong)0);

    return(INTR_SUCC);
}
