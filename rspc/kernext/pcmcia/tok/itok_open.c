static char sccsid[] = "@(#)65  1.5  src/rspc/kernext/pcmcia/tok/itok_open.c, pcmciatok, rspc41J, 9516A_all 4/18/95 00:54:16";
/*
static char sccsid[] = "@(#)45	1.5  src/rspc/kernext/isa/itok/itok_open.c, isatok, rspc41J 9/8/94 14:23:41";
 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: tokopen(), tok_ring_startup(), tok_adapter_open(),
 *            tok_finish_open(), tok_setup_timers(), 
 *            tok_remove_timers(), tokopen_cleanup()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "itok_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:                  tokopen
 *
 * FUNCTION:              open entry point.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:                 Only some of the code necessary to complete an
 *                        open is executed here, the rest runs on the
 *                        interrupt thread, i.e. this is an asynchronous
 *                        open.
 *
 * RETURNS:               0 or errcode.
 *
 */
/*****************************************************************************/
int tokopen(ndd_t *p_ndd)            /* Network device driver structure ptr. */
{
  int            i;                  /* Simple loop counter.  */
  int            rc;                 /* Function return code. */
  xmt_elem_t     *cur_elem;          /* Current tx-element.   */
  dds_t 	 *p_dds;             /* Device structure ptr. */

  /* Get the adapter structure. */
  p_dds = (dds_t *) (((ulong) p_ndd) - offsetof(dds_t, ndd));
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"OPNb",(ulong)p_ndd,(ulong)p_dds,(ulong)0);

  /* The driver/adapter must be in a closed state. */
  if (WRK.adap_state != CLOSED_STATE) {
    TRACE_BOTH(HKWD_ITOK_ISA_ERR,"OPN1",(ulong)p_dds,(ulong)EBUSY,
      (ulong)WRK.adap_state);
    return(EBUSY);
  }

#ifdef PCMCIATOK
  if ( p_dds->no_card ) {
    TRACE_BOTH(HKWD_ITOK_ISA_ERR,"OPN+",(ulong)p_dds,(ulong)ENOCONNECT,
      (ulong)WRK.adap_state);
    return(ENOCONNECT);
  }
#endif

  /* Set up state machine and NDD flags */
  NDD.ndd_flags  |= NDD_UP;
  WRK.adap_state  = OPEN_PENDING;
  WRK.open_status = OPEN_STATUS0;   /* Open state-event flag. */

  /* Save the ndd start time for statistics */
  WRK.ndd_stime = lbolt;
  bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));

  /* add our interrupt routine to kernel's interrupt chain */
  if (rc = i_init((struct intr *)(&(IHS)))) {
    TRACE_BOTH(HKWD_ITOK_ISA_ERR,"OPN2",(ulong)p_dds,(ulong)ENOCONNECT,
      (ulong)rc);
    tokopen_cleanup(p_dds);
    return(ENOCONNECT);
  }

  /* Setup the timers. */
  tok_setup_timers(p_dds);

  WRK.srb.busy = FALSE;

  /* Reset the adapter. */
  if (rc = tok_reset_adapter(p_dds)) {
    TRACE_BOTH(HKWD_ITOK_ISA_ERR,"OPN3",(ulong)p_dds,(ulong)ENOCONNECT,
      (ulong)rc);
    tokopen_cleanup(p_dds);

    /* Remove the intr. handler. */
    i_clear((struct intr *)(&(IHS)));

    /* Remove and clear the timers. */
    tok_remove_timers(p_dds);

    return(ENOCONNECT);
  }
#ifdef PM_SUPPORT
  p_dds->pm.activity = PM_ACTIVITY_NOT_SET;
#endif /* PM_SUPPORT */

 /* Update the statistics stuff. */
  WRK.dev_stime = lbolt;
  bzero(&TOKSTATS, sizeof(TOKSTATS) );
  bzero(&DEVSTATS, sizeof(DEVSTATS) );
  TOKSTATS.device_type = TOK_IBM_ISA;

  /* Reset the transmit wait list. */
  cur_elem = (xmt_elem_t *)&(WRK.tx_waitq[0]);
  for (i = 0; i < DDI.xmt_que_size; i++, cur_elem++) {
     cur_elem->next        = cur_elem + 1;
     cur_elem->bytes       = 0;
     cur_elem->in_use      = 0;
     cur_elem->mbufp       = 0;
  }
  /* Adjust the last one to point to the beginning. */
  WRK.tx_waitq[DDI.xmt_que_size - 1].next = &WRK.tx_waitq[0];

  /* Setup the free queue. */
  WRK.tx_next_free      = &WRK.tx_waitq[0];
  WRK.tx_last_free      = &WRK.tx_waitq[DDI.xmt_que_size-1];
  WRK.tx_next_srb       = 0;
  WRK.tx_last_srb       = 0;
  WRK.tx_next_arb       = 0;
  WRK.tx_last_arb       = 0;
  WRK.xmits_queued      = 0;
  WRK.srb_waiting       = 0;

  /* Reset the xmit correlator list. */
  for (i = 0; i <= TOK_MAX_CORRELATOR; i++) {
     WRK.xmit_correlators[i] = NULL;
  }

  /* Reset the SAP opened reference counts. */
  for (i = 0; i < 255; i++) {
     WRK.sap_allocated[i] = 0;
  }

  simple_lock(&DD_LOCK);

  /* If this is the first open for this adapter. */
  if ((++dd_ctrl.num_opens) == 1) {

    /* Register for dump. */
    dmp_add((void (*)())tok_cdt_func);
  }

  simple_unlock(&DD_LOCK);

#ifdef PCMCIATOK
	pcmcia_activity_lamp(p_dds, _CSaixTurnOnLED);
#endif

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"OPNe",(ulong)p_dds,(ulong)rc,(ulong)0);
  return(rc);
}


/*****************************************************************************/
/*
 * NAME:                  tok_ring_startup
 *
 * FUNCTION:              Restarts the adapter.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:                 The watchdog structure must be located just before
 *                        the dds pointer in the device driver structure.
 *
 * RETURNS:               Nothing.
 *
 */
/*****************************************************************************/
void tok_ring_startup(struct watchdog *p_wdt)  /* Watchdog structure ptr. */
{
  int rc;                        /* Internal return code.      */
  volatile uchar *ioaddr;        /* IO-space offset.           */
  volatile uchar *memaddr;       /* Shared mem. space offset.  */
  volatile uchar *aca_area;      /* Shared mem. register area. */

  /* Find the 'hard-coded' postion of the dds. */
  dds_t *p_dds = *(dds_t **) ((ulong)p_wdt + sizeof(struct watchdog));

#ifdef PCMCIATOK
  if( p_dds->ddi.io_addr == TOK_PRIMARY_ADDR )
      WRK.minor_no = TOK_PRIMARY_ADAPTER;
  else
      WRK.minor_no = TOK_SECONDARY_ADAPTER;
#endif

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TUPb",(ulong)p_dds,
    (ulong)WRK.open_status,(ulong)0);
  switch (WRK.open_status) {
    case OPEN_STATUS0:
      /* Release the latch that was set in the reset adapter function. */
      ioaddr = iomem_att(dd_ctrl.io_map_ptr);
      *(ioaddr + dd_ctrl.adap_cfg[WRK.minor_no].io_base + RESET_RELEASE_OFFSET)
         = RELEASE_VALUE;
      eieio();
      iomem_det(ioaddr);

      /* Map in the ACA area of the adapter. */
      memaddr = iomem_att(dd_ctrl.mem_map_ptr);
      aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr +
        ACA_AREA_OFFSET;

      /* Configure the shared RAM location via the RRR-register. */
      *(aca_area + RRR_EVEN_OFFSET) = DDI.shared_mem_addr >> 12;
      eieio();

      /* Enable interrupts. */
      *(aca_area + ISRP_EVEN_SET) = SW_INTR_ENABLING;
      eieio();

      /* Unmap the ACA area of the adapter. */
      iomem_det(memaddr);

      WRK.open_status = OPEN_STATUS1;

      /* Make sure that we only reset the adapter and don't continue to */
      /* open the adapter and such IF we are called from the close function.*/
      if (WRK.adap_state != CLOSED_STATE) {
        /* Wait for the adapter to completely reset. */
        /* This is done by just returning from the open call now with NDD_UP, */
        /* and having kicked off a bringup timer. */
        /* Look at the ring startup function for the rest of the open code. */
        /* It takes up to 3 seconds for the reset to finish. */
        INITWDT.count   = 0;
        INITWDT.restart = TOK_RESET_WAIT_TIME;
        w_start(&INITWDT);
      }

      break;
    case OPEN_STATUS1:

      /* Adjust the state variables. */
      WRK.open_status = OPEN_STATUS2;
      WRK.adap_state  = OPEN_PENDING;

      /* Send of the open parameter block. */
      if (rc = tok_adapter_open(p_dds)) {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TUP1",(ulong)EIO,(ulong)rc,(ulong)0);
        tokopen_cleanup(p_dds);
        return;
      }
      break;
    case OPEN_STATUS2: 

      /* Adjust the open state variable. */
      WRK.open_status = OPEN_STATUS3;

    case OPEN_STATUS3:

      /* Adjust the open state variable. */
      WRK.open_status = OPEN_STATUS4;
  
      /* Now check to see if we have any oustanding group/func. address cmds.*/
      /* We could have gotten some via ioctl's called during our open sleep. */
      /* Change the state to reflect that we have successfully opened. */
      if (WRK.command_to_do) {
        if (WRK.command_to_do & TOK_GROUP_ADDR_EVENT_BIT) {
          rc = tok_issue_cmd(p_dds, SRB_SET_GROUP_ADDR, FALSE,
                 WRK.open_cfg.grp_addr);
          break; /* Don't fall-through. */
        }
      } /* Fall-through. */
    case OPEN_STATUS4:

      /* Adjust the open state variable. */
      WRK.open_status = OPEN_STATUS5;

      if (WRK.command_to_do) {
        if (WRK.command_to_do & TOK_FUNC_ADDR_EVENT_BIT) {
          rc = tok_issue_cmd(p_dds, SRB_SET_FUNC_ADDR, FALSE,
                 WRK.open_cfg.func_addr);
          break; /* Don't fall-through. */
        }
        WRK.command_to_do = 0;
      } /* Fall-through. */
    case OPEN_STATUS5: {
      ndd_statblk_t  sb;

      NDD.ndd_flags |= NDD_RUNNING;
      WRK.open_status = OPEN_COMPLETE;
    
      /* Check to see if this is a limbo recovery or just a normal connect. */
      if (NDD.ndd_flags & NDD_LIMBO) {
        NDD.ndd_flags &= ~NDD_LIMBO;
        bzero(&sb, sizeof(sb));
        sb.code = NDD_LIMBO_EXIT;
        NDD.nd_status(&NDD, &sb);
      } else {

         /* Build the 'connected' status block. */
         bzero(&sb, sizeof(sb));
         sb.code = NDD_CONNECTED;
         NDD.nd_status(&NDD, &sb);
       }

      /* Reset the xmit queues. */
     WRK.xmits_queued      = 0;
     WRK.tx_srb_waiting    = 0;
     WRK.tx_arb_waiting    = 0;
     WRK.tx_buf_in_use     = 0;
     WRK.tx_stat           = 0;

    }
  }

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TUPe",(ulong)p_dds,
    (ulong)WRK.open_status,(ulong)0);
}


/*****************************************************************************/
/*
 * NAME:                  tok_adapter_open
 *
 * FUNCTION:              reads the adapter init information and issues
 *                        the open command.
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES:                 note that the SRB address changes after the command
 *                        completion.
 *
 * RETURNS:               0 or errcode
 *
 */
/*****************************************************************************/
int tok_adapter_open (dds_t *p_dds) /* Pointer to dds structure          */
{
  int            rc;                            /* Function return code.    */
  int            ring_speed;                    /* Network speed (1 = 16M)  */
  ushort         init_status;                   /* Adapter response.        */
  volatile uchar *memaddr;                      /* Shared mem. space offset.*/
  volatile uchar *aca_area, *srb_ptr, *cp, *dp; /* SHared mem. areas.       */

  /* Map in the ACA shared memory area. */
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TAOb",(ulong)p_dds,(ulong)0,(ulong)0);
  memaddr = iomem_att(dd_ctrl.mem_map_ptr);
  aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr + 
    ACA_AREA_OFFSET;

  /* Setup the new SRB address. */
  WRK.srb.offset = (((*(aca_area + WRBR_EVEN_OFFSET))) & 0xff) << 8;
  WRK.srb.offset += (((*(aca_area + WRBR_ODD_OFFSET))) & 0xff);

  /* Map in the SRB shared memory area. */
  srb_ptr = memaddr + DDI.shared_mem_addr + WRK.srb.offset; 

  /* Read the ring-speed. */
#ifdef PCMCIATOK
  ring_speed = DDI.ring_speed;
#else
  ring_speed = *(srb_ptr + INIT_STATUS_OFFSET) & 0xff;
  ring_speed &= 0x01;   /* Just get the ring_speed bit. */
#endif
  /* Compare the actual ring speed to the user configured one. */
  if (ring_speed != DDI.ring_speed) {
    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TAO1",(ulong)p_dds,(ulong)ring_speed,
      (ulong)DDI.ring_speed);
    /* Unmap the srb area. */
    iomem_det(memaddr);
    return(EINVAL);
  }

  /* Reflect the ring speed in the ndd flags. */
  if (ring_speed) {
    NDD.ndd_flags |= TOK_RING_SPEED_16;
  } else {
      NDD.ndd_flags |= TOK_RING_SPEED_4;
    }
   
  /* Read the initialization status. */
  init_status = (*(srb_ptr + BRINGUP_CODE_OFFSET + 1) & 0xff) + ((*(srb_ptr +
    BRINGUP_CODE_OFFSET) & 0xff) << 8);
  if (init_status) {
    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TAO2",(ulong)p_dds,(ulong)init_status,
      (ulong)WRK.srb.offset);
    /* Unmap the srb area. */
    iomem_det(memaddr);
    return(EIO);
  }

  /* Read and save the 'adapter address' offset. */
  WRK.adap_addr_offset = (*(srb_ptr + ADAPTER_ADDR_OFFSET + 1) & 0xff) +
     ((*(srb_ptr + ADAPTER_ADDR_OFFSET) & 0xff) << 8);

  /* Read and save the 'adapter parameters' offset. */
  WRK.adap_param_offset = (*(srb_ptr + ADAPTER_PARAM_OFFSET + 1) & 0xff) +
     ((*(srb_ptr + ADAPTER_PARAM_OFFSET) & 0xff) << 8);

  /* Unmap the SRB shared memory area. */
  iomem_det(memaddr);

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"TAOa",(ulong)WRK.srb.offset,
    (ulong)WRK.adap_addr_offset,(ulong)WRK.adap_param_offset);

  /* Send off the open command. */
  rc = tok_issue_cmd(p_dds, SRB_OPEN, FALSE, NULL);
  
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TAOe",(ulong)p_dds,(ulong)rc,(ulong)
    WRK.adap_state);
  return(rc);
}


/*****************************************************************************/
/*
 * NAME:                  tok_finish_open    
 *
 * FUNCTION:              Handles the open command response.
 *
 * EXECUTION ENVIRONMENT: interrupt level
 *
 * NOTES:                 There is one spin loop here, I don't think the
 *                        driver will ever get stuck here (even in error
 *                        cases), but ....
 *
 * RETURNS:               0 or errcode
 *
 */
/*****************************************************************************/
int tok_finish_open(dds_t   *p_dds)  /* Device structure ptr. */
{
  int            rc;            /* Function return code.    */
  volatile uchar *memaddr;      /* Shared mem. space offset.*/
  volatile uchar *srb_ptr;      /* Shared mem. area.        */

  /* Make the open thread proceed now (is waiting on this interrupt). */
  if (WRK.cmd_event & TOK_OPEN_EVENT_BIT) {
    /* If the command complete request bit is set, then wakeup the waiting */
    /* process thread. */
    if (WRK.cmd_event & TOK_WAKEUP_BIT) {
      WRK.cmd_event = 0;
      /* Wakeup the waiting thread, which will also cancel the timeout. */
      e_wakeup(&WRK.open_wait_event);
    } else {
        WRK.cmd_event = 0;
        /* Cancel the timeout timer. */
        TOK_STOP_CMD_TIMER;
      }
  }

  /* Map in the SRB shared memory area. */
  memaddr = iomem_att(dd_ctrl.mem_map_ptr);
  srb_ptr = memaddr + DDI.shared_mem_addr + WRK.srb.offset;

  /* Make sure that the command in progress is an open. */
  if (*srb_ptr != SRB_OPEN) {
    /* Unmap the SRB shared memory area. */
    iomem_det(memaddr);
    return(-1);
  }
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TFOb",(ulong)p_dds,(ulong)0,(ulong)0);

  /* Wait for the command to finish. */
  do {
 } while (*(srb_ptr + 2) == 0xfe);

  if ((rc = *(srb_ptr + 2)) == 0x0) {

    /* Successful adapter open. */
    /* Get the SRB, SSB, ARB and ASB offsets. */
    WRK.asb.offset = ((*(srb_ptr + ASB_ADDRESS_OFFSET) & 0xff) << 8) +
              (uchar)(*(srb_ptr + ASB_ADDRESS_OFFSET + 1));
    WRK.arb.offset = ((*(srb_ptr + ARB_ADDRESS_OFFSET) & 0xff) << 8) +
              (uchar)(*(srb_ptr + ARB_ADDRESS_OFFSET + 1));
    WRK.ssb.offset = ((*(srb_ptr + SSB_ADDRESS_OFFSET) & 0xff) << 8) +
              (uchar)(*(srb_ptr + SSB_ADDRESS_OFFSET + 1));
    /* Note that this is the new srb location after the first temp. one. */
    /* Calculate the srb offset last, otherwise you won't get the other ones.*/
    WRK.srb.offset = ((*(srb_ptr + SRB_ADDRESS_OFFSET) & 0xff) << 8) +
              (uchar)(*(srb_ptr + SRB_ADDRESS_OFFSET + 1));

    /* Unmap the SRB shared memory area. */
    iomem_det(memaddr);
    /* Indicate that we are not waiting on an open event anymore. */
    WRK.cmd_event    = 0;
    WRK.open_retries = 0;

    /* The adapter is now open. */
    WRK.adap_state     = OPEN_STATE;
    WRK.network_status = TR_MIB_NOPROBLEM;           /* MIBS stuff. */
    WRK.last_open      = TR_MIB_LASTOPEN;            /* MIBS stuff. */

    /* Now continue the bringup .. allocate the SAP and so on. */
    tok_ring_startup(&INITWDT);

  } else {
      /* Unmap the SRB shared memory area. */
      iomem_det(memaddr);
      /* Open failed. */
      WRK.adap_state = DEAD_STATE;
      WRK.open_retries++;
      logerr(p_dds, TOK_OPEN_FAILED + rc);

      /* Update the MIBS 'ring open status' variable. */
      switch (rc) {
        case 5:
          WRK.last_open = TR_MIB_BADPARAM;
          break;
        case 7:
          WRK.last_open = TR_MIB_LOBEFAILED;
          break;
      }

      /* Reset the open state, for the state-event machine. */
      WRK.network_status = TR_MIB_NO_STATUS;

      /* Now check that the open call did not fail with a beaconing and 
         we have not already retried 3 times. */
      if (((rc == 5) && (WRK.open_retries < 3)) || (rc != 5)) {
        /* No need to check the completion code here, can't fail. */
        tok_reset_adapter(p_dds);
      }
  }

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TFOe",(ulong)rc,(ulong)WRK.adap_state,
    (ulong)0);
  return(rc);
}


/*****************************************************************************/
/*
 * NAME:                  tok_setup_timers    
 *
 * FUNCTION:              Allocate the watchdog timers.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_setup_timers(dds_t   *p_dds)  /* Device structure ptr. */
{
  /* Setup the init timer structure */
  while(w_init((struct watchdog *)(&(INITWDT)))); 
  WRK.init_wdt_active = FALSE;
  p_dds->init_wdt_dds = p_dds;
  INITWDT.func        = tok_ring_startup;

  /* Setup the cmd timer structure */
  while(w_init((struct watchdog *)(&(CMDWDT))));
  WRK.cmd_wdt_active  = FALSE;
  p_dds->cmd_wdt_dds  = p_dds;
  CMDWDT.func         = tok_cmd_timeout;

  /* Setup the transmit timeout timer structures */
  while(w_init((struct watchdog *)(&(XMITWDT)))); 
  WRK.xmit_wdt_active   = FALSE;
  p_dds->xmit_wdt_dds   = p_dds;
  XMITWDT.func          = tok_xmit_timeout;

  /* Setup the srb timeout timer structures */
  while(w_init((struct watchdog *)(&(SRBWDT)))); 
  WRK.srb_wdt_active    = FALSE;
  p_dds->srb_wdt_dds    = p_dds;
  SRBWDT.func           = tok_srb_timeout;
  return;
}


/*****************************************************************************/
/*
 * NAME:                  tok_remove_timers    
 *
 * FUNCTION:              De-allocate the watchdog timers.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_remove_timers(dds_t   *p_dds)  /* Device structure ptr. */
{
  /* Reset the init timer structure */
  w_stop((struct watchdog *)(&(INITWDT)));
  WRK.init_wdt_active  = FALSE;
  while (w_clear(&INITWDT));

  /* Reset the cmd timer structure */
  w_stop((struct watchdog *)(&(CMDWDT)));
  WRK.cmd_wdt_active  = FALSE;
  while (w_clear(&CMDWDT));

  /* Reset the transmit timeout timer structures */
  w_stop((struct watchdog *)(&(XMITWDT)));
  WRK.xmit_wdt_active   = FALSE;
  while (w_clear(&XMITWDT));

  /* Reset the srb timeout timer structures */
  w_stop((struct watchdog *)(&(SRBWDT)));
  WRK.srb_wdt_active   = FALSE;
  while (w_clear(&SRBWDT));
}


/*****************************************************************************/
/*
 * NAME:                  tokopen_cleanup    
 *
 * FUNCTION:              Cleans up after an open or open attempt.
 *
 * EXECUTION ENVIRONMENT: process or interrupt level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tokopen_cleanup(dds_t *p_dds)  /* Device structure ptr. */
{
  int rc;

 TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"OPCb",(ulong)WRK.open_status,(ulong)
   WRK.adap_state,(ulong)0);

  /* Cleanup accordingly to how far we got during the open. */
  switch(WRK.open_status) {
    case OPEN_COMPLETE:

      /* Fall through here. */
    case OPEN_STATUS4:

      /* Fall through here. */
    case OPEN_STATUS3:
      rc = tok_issue_cmd(p_dds, SRB_CLOSE, FALSE, NULL);

      /* Fall through here. */
    case OPEN_STATUS2:

      /* Fall through here. */
    case OPEN_STATUS1:

      /* Fall through here. */
    case OPEN_STATUS0:

      WRK.adap_state = CLOSED_STATE;
      NDD.ndd_flags &= ~(NDD_UP | NDD_RUNNING | NDD_LIMBO | NDD_ALTADDRS | \
                  TOK_RECEIVE_FUNC | TOK_RECEIVE_GROUP);
    }

 TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"OPCe",(ulong)WRK.adap_state,(ulong)0,(ulong)0);

  return;
}
