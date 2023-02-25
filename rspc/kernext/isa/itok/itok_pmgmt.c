static char sccsid[] = "@(#)77  1.1  src/rspc/kernext/isa/itok/itok_pmgmt.c, isatok, rspc41J, 9516B_all 4/21/95 13:02:52";
/*
 * COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: tok_pmgmt tok_hibernate tok_enable
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "itok_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     tok_pmgmt
 *
 * FUNCTION: Tokenring driver power management routine
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *
 * CALLED FROM:  pm_core routine
 *
 * INPUT:
 *            private - pointer to the device control area
 *            ctrl - specifies the mode or a notice, a mode is a requested
 *                   state to change to and a notice is either for pinning
 *                   code or ring resume.  
 *
 * RETURNS:  
 *            0 - if successful
 *
 *            EBUSY - if busy
 */
/*****************************************************************************/

int
tok_pmgmt(dds_t *p_dds, int ctrl)    
{
	int	ipri,			/* current priority */
		rc=0;

	TRACE_SYS(HKWD_ITOK_ISA_OTHER,"pmgmtb",(ulong)p_dds,(ulong)ctrl,0);

        switch(ctrl)
        {
                case PM_DEVICE_SUSPEND:
                case PM_DEVICE_HIBERNATION:
                        {
                        PMGMT.activity = -1;
                        PMGMT.mode = ctrl;
                        WRK.pm_state = WRK.adap_state;

                        /*
                        * if the driver is closed or closing then nothing needs to be
                        * done to put the adapter in hibernate/suspend state.  Just
                        * remember the state to prevent future calls.  The closing state
                        * will reset the driver in the future, and as the driver does
                        * not know which resources have been freed it is best for it
                        * to do nothing.
                        */
                        if (WRK.adap_state == CLOSED_STATE ||
                            WRK.adap_state == CLOSE_PENDING)
                        {
                             WRK.adap_state = SUSPEND_STATE;
                             break;
			}
                        /* disable interrupts and grab locks */

                        ipri=disable_lock(PL_IMP, &SLIH_LOCK);
                        disable_lock(PL_IMP, &TX_LOCK);

                        if (WRK.adap_state != DEAD_STATE)
                             tok_hibernate(p_dds);

                        WRK.adap_state = SUSPEND_STATE;

                        unlock_enable(PL_IMP, &TX_LOCK);
                        unlock_enable(ipri, &SLIH_LOCK);
                        break;
                        }
                case PM_DEVICE_FULL_ON:
                case PM_DEVICE_ENABLE:
                        {
                        PMGMT.mode=ctrl;

                        /*
                         * if the driver is not in suspend state, then nothing other
                         * than the mode needs to be changed
                         */
                        if (WRK.adap_state != SUSPEND_STATE)
                             break;

                        WRK.adap_state = WRK.pm_state;
                        PMGMT.activity = 1;

			/* make sure the adapter hasn't been removed from the system,
			   and that all of the device specific config is done */

                        if (WRK.adap_state==OPEN_STATE)
                        {
                             rc = tok_cfg_dds(p_dds);
                        }
                        /*
                         * change the state of the adapter to reflect anything that
                         * was set before hibernation
                         */

                        if ( (!rc) && WRK.adap_state==OPEN_STATE)
                        {
                             /*
                              * start the adapter, it has been powered off
                              */

                             rc = tok_reset_adapter(p_dds);
                             if (!rc)
                             {
                                  rc = tok_enable(p_dds);
                             }
                        }

                        if (rc && WRK.adap_state != CLOSED_STATE)
                        {
                                WRK.adap_state=DEAD_STATE;
                        }

                        /*
                         * be sure to wake up all users waiting for the device
                         * to resume
                         */
                        e_wakeup(&WRK.pm_event);

                        break;
                        }
                case PM_DEVICE_DPMS_SUSPEND:
                case PM_DEVICE_DPMS_STANDBY:
                        {
                        PMGMT.mode=ctrl;
                        break;
                        }
                case PM_PAGE_FREEZE_NOTICE:
                        { 
                        if (rc = pincode(tokconfig))
                        {
			     TRACE_SYS(HKWD_ITOK_ISA_OTHER,"pmgmtf",(ulong)p_dds,(ulong)ctrl,rc);
                             return(PM_ERROR);
                        }
                        break;
                        }
                case PM_PAGE_UNFREEZE_NOTICE:
                        {
                        unpincode(tokconfig);
                        break;
                        }
                case PM_RING_RESUME_ENABLE_NOTICE:
                        {
                        break;
                        }
                default:
			TRACE_SYS(HKWD_ITOK_ISA_OTHER,"pmgmtd",(ulong)p_dds,(ulong)ctrl,0);
        }

	TRACE_SYS(HKWD_ITOK_ISA_OTHER,"pmgmte",(ulong)p_dds,(ulong)ctrl,WRK.adap_state);
	return(PM_SUCCESS);
}

/*****************************************************************************/
/*
 * NAME:     tok_hibernate
 *
 * FUNCTION: Tokenring driver power management hibernation routine
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * CALLED FROM:  tok_pmgmt
 *
 * INPUT:
 *         pointer to the device driver structure 
 *
 * RETURNS: 
 *            0 - if successful
 *
 */
/*****************************************************************************/

int 
tok_hibernate(dds_t *p_dds)
{
	int	   rc=0;
	xmt_elem_t *cur_elem;	/* Current tx-element. */

        /* stop any existing watchdog timers */

	w_stop((struct watchdog *)(&(INITWDT)));
	WRK.init_wdt_active  = FALSE;
	w_stop((struct watchdog *)(&(CMDWDT)));
	WRK.cmd_wdt_active  = FALSE;
	w_stop((struct watchdog *)(&(XMITWDT)));
	WRK.xmit_wdt_active   = FALSE;
	w_stop((struct watchdog *)(&(SRBWDT)));
	WRK.srb_wdt_active   = FALSE;

	/* give 1 sec for transmit operations to finish */

	if ( WRK.xmits_queued )
	{
		delay(HZ);
	}

	/* Remove the adapter from the ring. */

	rc = tok_issue_cmd(p_dds, SRB_CLOSE, TRUE, NULL);

        /* stop the adapter */

	tok_reset_adapter(p_dds);

	/* Now free the waiting transmission elements. */

	while (WRK.xmits_queued)
	{
		GET_ARB_ELEM(cur_elem);

		/* Now free any waiting mbufs. */
		m_freem(cur_elem->mbufp);

		/* Adjust the linked wait list. */
		cur_elem->in_use = 0;

		/* Add the element to the free list. */
		ADD_2_FREE_Q(cur_elem);
	}

	/* Reset the xmit queues. */

	WRK.xmits_queued      = 0;
	WRK.tx_srb_waiting    = 0;
	WRK.tx_arb_waiting    = 0;
	WRK.tx_buf_in_use     = 0;
	WRK.tx_stat           = 0;

	/* clear all ndd_genstats and device stats */

	bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
	bzero(&TOKSTATS, sizeof(TOKSTATS) );
	bzero(&DEVSTATS, sizeof(DEVSTATS) );


	return(0);
}
         
/*****************************************************************************/
/*
 * NAME:     tok_enable
 *
 * FUNCTION: Tokenring driver power management enable routine
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * CALLED FROM:  tok_pmgmt
 *
 * INPUT:
 *         pointer to the device driver structure
 *
 * RETURNS:
 *            0 - if successful
 *
 */
/*****************************************************************************/

int
tok_enable(dds_t *p_dds)
{
	int 		rc	=0,
			i	=0;
	xmt_elem_t	*cur_elem;	/* Current tx-element. */

	/* Save the ndd start time for statistics */
	WRK.ndd_stime = lbolt;

	/* status request block is not busy currently */
	WRK.srb.busy = FALSE;

	/* Reset the transmit wait list. */
	cur_elem = (xmt_elem_t *)&(WRK.tx_waitq[0]);

	for (i = 0; i < DDI.xmt_que_size; i++, cur_elem++)
	{
		cur_elem->next   = cur_elem + 1;
		cur_elem->bytes  = 0;
		cur_elem->in_use = 0;
		cur_elem->mbufp  = 0;
	}

	/* Adjust the last one to point to the beginning. */
	WRK.tx_waitq[DDI.xmt_que_size - 1].next = &WRK.tx_waitq[0];

	/* Setup the free queue. */
	WRK.tx_next_free = &WRK.tx_waitq[0];
	WRK.tx_last_free = &WRK.tx_waitq[DDI.xmt_que_size-1];
	WRK.tx_next_arb  = 0;
	WRK.tx_last_arb  = 0;
	WRK.srb_waiting  = 0;

	/* Reset the xmit correlator list. */
	for (i = 0; i <= TOK_MAX_CORRELATOR; i++)
	{
		WRK.xmit_correlators[i] = NULL;
	}

	/* open any SAP's that were open before */

	for (i = 0; i < 255; i++)
	{
		if (WRK.sap_allocated[i])
		{
		    tok_issue_cmd(p_dds, SRB_OPEN_SAP, FALSE, &WRK.sap_dsap[i]);
		}
	}

     return(rc);
}

