static char sccsid[] = "@(#)38  1.2  src/rspc/kernext/isa/ient/ient_pmgmt.c, isaent, rspc41J, 9517A_all 4/24/95 14:41:25";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS:  ient_pmgmt
 *             ient_hibernate
 *	       ient_enable
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

/*****************************************************************************/
/*
 * NAME:     ient_pmgmt
 *
 * FUNCTION: Ethernet driver power management routine
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
ient_pmgmt(dds_t *p_dds, int ctrl)    
{
	int	ipri,			/* current priority */
		rc=0;

	TRACE_SYS(HKWD_IEN_ISA_OTHER, "PMB", 0, 0, 0);

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
			disable_lock(PL_IMP, &CMD_LOCK);

			if (WRK.adap_state != DEAD_STATE)
                             ient_hibernate(p_dds);

			WRK.adap_state = SUSPEND_STATE;

			unlock_enable(PL_IMP, &CMD_LOCK);
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

			if (WRK.adap_state==OPEN_STATE)
			{
			     rc = ient_init(p_dds);
			}
			/*
			 * change the state of the adapter to reflect anything that
			 * was set before hibernation
			 */

			if (rc==0 && WRK.adap_state==OPEN_STATE)
			{
			     /*
			      * start the adapter, it has been powered off
			      */

			     rc = ient_start(p_dds);
			     if (!rc)
			     {
			          rc = ient_enable(p_dds);
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
			if (rc = pincode(ient_config))
			{
    			     TRACE_SYS(HKWD_IEN_ISA_OTHER, "PMF", (ulong)p_dds, ctrl, rc);
			     return(PM_ERROR);
			}
                        break;
			}
		case PM_PAGE_UNFREEZE_NOTICE:
			{
			unpincode(ient_config);
                        break;
			}
		case PM_RING_RESUME_ENABLE_NOTICE:
			{
                        break;
			}
		default:
    			TRACE_SYS(HKWD_IEN_ISA_OTHER, "PMD", (ulong)p_dds, ctrl, 0);
	}

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "PME", (ulong)p_dds, ctrl, WRK.adap_state);
    return(PM_SUCCESS);

}

/*****************************************************************************/
/*
 * NAME:     ient_hibernate
 *
 * FUNCTION: Ethernet driver power management hibernation routine
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * CALLED FROM:  ient_pmgmt
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
ient_hibernate(dds_t *p_dds)
{
        xmt_elem_t *p_tmp_tx_elm;        /* temporary pointer to tx elements */
	int	   rc=0;

	/* stop any existing watchdog timers */

         w_stop((struct watchdog *)(&(CMDWDT)));
         WRK.cmd_wdt_active  = FALSE;
         w_stop((struct watchdog *)(&(TXWDT)));
         WRK.xmit_wdt_active   = FALSE;

	/* stop the adapter */

        rc=ient_stop(p_dds);

        /* free up all the currently allocated mbufs */

        while (WRK.xmits_queued)
        { 
	     GET_WAIT_ELEM(p_tmp_tx_elm);
	     m_freem(p_tmp_tx_elm->mbufp);
	     p_tmp_tx_elm->flags = 0;
	     p_tmp_tx_elm->in_use = 0;
             ADD_2_FREE_Q(p_tmp_tx_elm);
        }

	/* reinitialize adapter pointers */

        if (rc = ient_setup(p_dds))
        {
            /* undo any resource setup */
            ient_cleanup(p_dds);

            WRK.adap_state = CLOSED_STATE;

            simple_lock(&DD_LOCK);
            ient_del_cdt("DDS", (char *)p_dds, WRK.alloc_size);
            simple_unlock(&DD_LOCK);

            return(rc);
        }

        return(rc);
}
         
/*****************************************************************************/
/*
 * NAME:     ient_enable
 *
 * FUNCTION: Ethernet driver power management enable routine
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * CALLED FROM:  ient_pmgmt
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
ient_enable(dds_t *p_dds)
{
     int 		rc	=0,
			i	=0;
     volatile uchar 	cur_rcr =NULL,
			*ioaddr =NULL,
			cur_val =NULL;
     ient_multi_t       *p_multi= WRK.multi_table;

	/* Why was I trying to use WRK.multi_count?????? */

     if ( WRK.enable_multi || WRK.promiscuous_count || WRK.badframe_count )
          {

          /* Attach to the IO-bus. */
          ioaddr = iomem_att(dd_ctrl.io_map_ptr);

          /* First stop the NIC. */
          ient_stop_NIC(p_dds, ioaddr);

          /* Set page. #2. */
          *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA | 
          IEN_ISA_STOP_NIC | IEN_ISA_PAGE2;
          eieio();

          /* Read the current value of 'RCR' from page 2. */
          cur_rcr = *(ioaddr + DDI.io_addr + PAGE2_RCR);
          eieio();

          /* Switch back to page 0 */
          *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
          IEN_ISA_STOP_NIC;
          eieio();

          /* enable multicasting if it was enabled before */

	  if ( WRK.enable_multi )
          {
               /* Set the 'accept multicast' packets bit. */
               cur_rcr |= IEN_ISA_ACCEPT_MCASTS;
               *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
               eieio();

               /* Switch to page 1. */
               *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
               IEN_ISA_STOP_NIC | IEN_ISA_PAGE1;
               eieio();

               /* Set all the multicast bits. */
               for (i = PAGE1_MAR0; i <= PAGE1_MAR7; i++)
               {
                    *(ioaddr + DDI.io_addr + i) = 0xff;
               }
               eieio();

          }

          /* Switch back to page 0 */
          *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
          IEN_ISA_STOP_NIC;
          eieio();

          /* enable promiscuous mode if it was enabled before */

	  if ( WRK.promiscuous_count )
          {
               /* Set the 'promiscuous mode' bit. */
               cur_rcr |= IEN_ISA_PROMIS_MODE;
               *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
               eieio();
          }

          /* Switch to page 1. */
          *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
          IEN_ISA_STOP_NIC | IEN_ISA_PAGE1;
          eieio();

	  /* if multicasting was not enabled, check to see if 
	     any multicast address were enabled and handle those */

	  if ( WRK.enable_multi )
	  {
               /*
                * First zero out all of the MAR's and then reprogram them
                * with the correct bits for Multicasts that were previously
                * going on.
                */

               for (i = PAGE1_MAR0; i <= PAGE1_MAR7; i++)
               {
                   *(ioaddr + DDI.io_addr + i) = 0x00;
               }
               eieio();

               while (p_multi)
               {
                   cur_val = *(ioaddr + DDI.io_addr + 
			     (PAGE1_MAR0 + p_multi->mar_num));
                   eieio();
                   cur_val |= p_multi->mar_val;

                   *(ioaddr + DDI.io_addr +
                     (PAGE1_MAR0 + p_multi->mar_num)) = cur_val;
                   eieio();

                   p_multi = p_multi->next;
               }

	  } 
	  
	  /* enable the 'bad frame mode' if it was set before */

	  if ( WRK.badframe_count )
	  {
               cur_rcr |= IEN_ISA_RCV_ERRORS;
               *(ioaddr + DDI.io_addr + PAGE0_RCR) = cur_rcr;
               eieio();
	  }

          /* Switch to page 0. */
          *(ioaddr + DDI.io_addr + PAGE0_COMMAND) = IEN_ISA_NO_DMA |
          IEN_ISA_STOP_NIC;
          eieio();

          /* Re-start the NIC. */
          ient_start_NIC(p_dds, ioaddr);

          iomem_det((void *) ioaddr);

          }	/* while any WRK.blah non zero */

     return(rc);
}

