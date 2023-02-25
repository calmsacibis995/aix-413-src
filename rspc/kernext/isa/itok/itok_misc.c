static char sccsid[] = "@(#)47	1.8  src/rspc/kernext/isa/itok/itok_misc.c, isatok, rspc41J, 9516B_all 4/21/95 09:56:26";
/*
 * COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: tok_q_cmd(), tok_issue_cmd(), tok_ring_recovery(), 
 *            tok_ring_change(), tok_srb_timeout(), tok_cmd_timeout(),
 *            tok_grp_cmd_response(), tok_func_cmd_response(),
 *            tok_open_sap_cmd_response(), tok_close_sap_cmd_response(),
 *            tok_close_cmd_resp(), tok_read_log_cmd_resp()
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
 * NAME:                  tok_q_cmd
 *
 * FUNCTION:              Queues a SRB command.
 *
 * EXECUTION ENVIRONMENT: Process or interrupt.
 *
 * NOTES:                 If called from the process thread, the thread will
 *                        go to sleep waiting on a SRB completion interrupt.
 *                        If called from the interrupt thread a flag is set 
 *                        indicating what srb command must be executed next on
 *                        the next 'srb-free' interrupt. A timeout timer is
 *                        kicked off in both cases.
 *
 * RETURNS:               0, if the command can be executed.
 *                        -1, if (after a wait) a previous cmd is still busy.
 *
 */
/*****************************************************************************/
int tok_q_cmd(dds_t *p_dds,         /* Device structure ptr.        */
              uint  cmd,            /* Command to execute.          */
              uint  process_thread) /* Process or interrupt thread. */
{
  int rc = 0;           /* Function return code. */

  /* Kickoff a timeout timer regardless of running on a interrupt or process */
  /* thread. But only do it if it is not already active. */
  if (!WRK.srb_wdt_active) {
    WRK.srb_wdt_active = TRUE;
    SRBWDT.count   = 0;
    SRBWDT.restart = 1;
    w_start(&SRBWDT);
  }

  /* Check to see if we are running on the process thread. */
  if (process_thread) {
    /* Queue up this command. */
    WRK.srb_waiting++;

    /* Now wait for the SRB release (or timeout timer popping). */
    while (WRK.srb.busy) {
      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"slep",(ulong)p_dds,
        (ulong)WRK.srb_wait_event, (ulong)0);
      e_sleep_thread(&WRK.srb_wait_event, &TX_LOCK, LOCK_HANDLER);
      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"wake",(ulong)p_dds,
        (ulong)WRK.srb_wait_event, (ulong)0);
    }
    /* One less thread sleeping waiting on the srb. */
    WRK.srb_waiting--;

    /* If this was the last thread waiting and the timeout timer is active.*/
    if ((!WRK.srb_waiting) && (WRK.srb_wdt_active)) {
      /* Kill it. */
      w_stop(&SRBWDT);
      WRK.srb_wdt_active = FALSE;
    }
  }  else { /* End of 'it was a process thread. */
       /* Interrupt thread. */
       rc = 0;
       WRK.intr_action = cmd;
     }

  return(rc);
}


/*****************************************************************************/
/*
 * NAME:                  tok_issue_cmd
 *
 * FUNCTION:              Issues a command towards the adapter.
 *                        Only commands that complete with the response
 *                        posted in the SRB are handled here.
 *                        This function can wait for the command to complete
 *                        or simply pass-through.
 *                        In any case the cmd watchdog timer is started.
 *
 * EXECUTION ENVIRONMENT: Process or interrupt.
 *
 * NOTES:                 This function is single-threaded, meaning that
 *                        there should never be more than one command
 *                        outstanding on the adapter at any given time.
 *                        If the function is called from an interrupt
 *                        thread don't wait for the cmd completion.
 *
 * RETURNS:               0, if the command was executed.
 *                        On a process thread :
 *                        -1, if (after a wait) a previous cmd is still busy.
 *                        On an interrupt thread:
 *                        -1, means 'is queued'.
 *
 */
/*****************************************************************************/
int tok_issue_cmd(dds_t *p_dds,         /* Device structure ptr.        */
                   uint  cmd,           /* Adapter cmd to execute.      */
                   uint  wait,          /* Wait for cmd completion ?    */
                   uchar *arg)          /* Possible cmd parameters.     */
{
  int            i;                     /* Simple loop counter.             */
  int            rc=0;                    /* Function return code.            */
  int            oldpri;                /* Old priority.                    */
  int            *cur_event;            /* What command must complete.      */
  int            cmd_event_bit;         /* For the common waiting place.    */
  volatile uchar *memaddr;              /* Shared mem. offset.              */
  volatile uchar *aca_area, *srb_ptr,   /* Points to shared mem. locations. */
                 *cp, *dp;              /* on the adapter.                  */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CMDb",(ulong)p_dds,(ulong)cmd,
     (ulong)WRK.cmd_event);

  oldpri = i_disable(DDI.intr_priority);
  /* Check that we don't have an outstanding cmd. already. */
  if (WRK.srb.busy) {

    /* We already have some thread using the srb area. */
     rc = tok_q_cmd(p_dds, cmd, wait);
     if (!wait)
       return(0);
  }

  /* Map in the SRB shared memory area. */
  memaddr = iomem_att(dd_ctrl.mem_map_ptr);
  srb_ptr = memaddr + DDI.shared_mem_addr + WRK.srb.offset;

  /* Branch according to what command parameters we need to setup. */
  switch (cmd) {
    case SRB_READ_LOG:
      WRK.srb.busy = TRUE;
      /* Setup the cmd specifics for the timeout function preparation below. */
      cmd_event_bit = WRK.cmd_event = TOK_READ_LOG_EVENT_BIT;
      cur_event     = &WRK.read_log_event;

      /* No command parameters to setup. */

      break;
    case SRB_SET_GROUP_ADDR: {

      WRK.srb.busy = TRUE;
      /* Setup the cmd specifics for the timeout function preparation below. */
      cmd_event_bit = WRK.cmd_event = TOK_GROUP_ADDR_EVENT_BIT;
      cur_event     = &WRK.grp_addr_event;

      /* Setup the command parameters, i.e. the new group addr. */
      /* This is shared memory, so only use 8-bit accesses. */
      for (i = 6; i < 10; i++) /* Loop at the exact place. */
         *(srb_ptr + i)  = (uchar) *arg++;

      break;
    }
    case SRB_SET_FUNC_ADDR: {

      WRK.srb.busy = TRUE;
      /* Setup the cmd specifics for the timeout functioni preparation below. */
      cmd_event_bit = WRK.cmd_event = TOK_FUNC_ADDR_EVENT_BIT;
      cur_event     = &WRK.func_addr_event;

      /* Setup the command parameters, i.e. the new func. addr. */
      /* This is shared memory, so only use 8-bit accesses. */
      for (i = 6; i < 10; i++) /* Loop at the exact place in the cmd-block. */
         *(srb_ptr + i)  =  (uchar) *arg++;

      break;
    }
    case SRB_OPEN:
      /* Setup the cmd specifics for the timeout function. */
      cmd_event_bit = WRK.cmd_event = TOK_OPEN_EVENT_BIT;
      cur_event     = &WRK.open_wait_event;

      /* Setup the command parameters thats stored in the driver work area. */
      /* This is shared memory, so only use 8-bit accesses. */
      for (i = 0,cp = srb_ptr, dp = (uchar *) &WRK.open_cfg;
            i < sizeof(open_param_t); i++)
         *cp++ = *dp++;

      break;
    case SRB_CLOSE:
      /* Setup the cmd specifics for the timeout function. */
      cmd_event_bit = WRK.cmd_event = TOK_CLOSE_EVENT_BIT;
      cur_event     = &WRK.close_event;

      /* No command parameters to setup. */

      break;
    case SRB_OPEN_SAP: {
      uchar sap = *arg;

      /* Setup the cmd specifics for the timeout function. */
      cmd_event_bit = WRK.cmd_event = TOK_SAP_OPEN_EVENT_BIT;
      cur_event     = &WRK.sap_open_event;

      /* Clean out the command block (just in case). */
      for (i = 0; i < SRB_AREA_LENGTH; i++)
         *(srb_ptr + i) = 0;  /* Don't use bzero, use 8bit access. */

      /* Setup the the OPEN SAP command parameters. */
      /* This is shared memory, so only use 8-bit accesses. */
      *(srb_ptr + 16) = sap;
      *(srb_ptr + 17) = TOK_INDIVIDUAL_SAP | TOK_PASS_XID_2_APP;

      break;
    }
    case SRB_CLOSE_SAP: {

      /* Setup the cmd specifics for the timeout function. */
      cmd_event_bit = WRK.cmd_event = TOK_SAP_CLOSE_EVENT_BIT;
      cur_event     = &WRK.sap_close_event;

      /* Setup the the CLOSE SAP command parameters. */
      /* This is shared memory, so only use 8-bit accesses. */
      *(srb_ptr + 4)  = (uchar)((WRK.station_ids[*arg] & 0xff00) >> 8);
      *(srb_ptr + 5)  = (uchar)(WRK.station_ids[*arg] & 0x00ff);

      break;
    }
  }

  /* Setup the command header (using 8-bit accesses). */
  *(srb_ptr) = (uchar)cmd;
  *(srb_ptr + 1)                       = 0;
  *(srb_ptr + SRB_RETCODE_OFFSET)      = SRB_INITIATED;
  *(srb_ptr + 3)                       = 0;
  eieio();  /* Make sure that the powerpc chip does not change the sequence. */

  /* Map in the ACA shared memory area. */
  aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr + 
    ACA_AREA_OFFSET;

  /* Send it. */
  *(aca_area + ISRA_ODD_SET) = 0x28;

  /* Detach the srb area. */
  iomem_det(memaddr); 

  /* Reset the command timeout timer and start it. */
  CMDWDT.count       = 0;
  CMDWDT.restart     = TOK_CMD_TIMEOUT;
  WRK.cmd_wdt_active = TRUE;
  w_start(&(CMDWDT));

  /* Should we wait for the command to complete ? If so go to sleep. */
  if (wait) {
    /* Set the indication that we need a wakeup. */
    WRK.cmd_event |= TOK_WAKEUP_BIT;
    /* Wait for the completion. */
    /* The interrupt thread that get's the command completion interrupt must */
    /* clear the event bit. */
    while (WRK.cmd_event & cmd_event_bit) {
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SLEP",(ulong)p_dds,(ulong)0,(ulong)0);
      e_sleep_thread(cur_event, &TX_LOCK, LOCK_HANDLER);
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"WAKE",(ulong)p_dds,(ulong)0,(ulong)0);
    }
 
    /* Clean up after the timer. */
    if (WRK.cmd_wdt_active) {
      /* It was not the timeout timer that popped, now we don't need it. */
      w_stop(&(CMDWDT));

      /* Free it up. */
      WRK.cmd_wdt_active = FALSE;
    } else {
        /* The timeout timer must have popped, an error occurred. */
        rc = -1;
      }
  } /* if wait. */

  i_enable(oldpri);
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CMDe",(ulong)p_dds,(ulong)cmd,
     (ulong)WRK.adap_state);
  return(rc);
}


/*****************************************************************************/
/*
 * NAME:                  tok_ring_recovery
 *
 * FUNCTION:              Restarts the adapter.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:                 Sends a limbo status message to the demuxer.
 *
 * RETURNS:               Nothing.
 *
 */
/*****************************************************************************/
void tok_ring_recovery(dds_t  *p_dds,    /* Device structure ptr.        */
                      ushort status)     /* The ring change information. */
{
  uchar         sap_value;               /* The SAP value to open. */
  ndd_statblk_t sb;                      /* NDD status-block.      */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"XXXb",(ulong)p_dds,(ulong)status,(ulong)0);
  /*  Set the state machine and the NDD flags.  */
  NDD.ndd_flags &= ~NDD_RUNNING;
  NDD.ndd_flags |= NDD_LIMBO;
  WRK.adap_state = OPEN_PENDING;

  /* Build the asynchronous status block */
  bzero (&sb, sizeof(sb));
  sb.code = NDD_LIMBO_ENTER;
  sb.option[0] = NDD_REASON_CODE + status;
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER, "stat",(ulong)p_dds,(ulong)status,(ulong)0);
  NDD.nd_status( &NDD, &sb );

  /* The adapter has closed, reset and reopen it. */
  /* No need to check the completion code here, can't fail. */
  tok_reset_adapter(p_dds);

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"XXXe",(ulong)p_dds,(ulong)status,(ulong)0);

  return;
}


/*****************************************************************************/
/*
 * NAME:                  tok_ring_change
 *
 * FUNCTION:              Logs the tokenring network ring change and
 *                        issues a read/clear log to the adapter if
 *                        necessary.
 *                        If the adapter is shutdown a close command
 *                        will be issued.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:                 If a serious error is reported the adapter will
 *                        be issued a close command and the following
 *                        command completion interrupt will re-open the
 *                        adapter.
 *
 * RETURNS:               0,
 *                        error if a close command was issued and failed.
 *
 */
/*****************************************************************************/
int tok_ring_change(dds_t  *p_dds,     /* Device structure ptr.        */
                    ushort status)     /* The ring change information. */
{

  int         i;                     /* Simple loop counter.                 */
  int         rc = 0;                /* Return code.                         */
  int         limbo_code;            /* Error code converted to standard.    */
  char        fatal_error;           /* Should a close command be issued.    */
  char        clear_log;             /* Should a read log command be issued. */
  ushort      cur;                   /* Running bit in the status word.      */
  uchar       cur_bit;               /* Current status bit, more than one    */
                                     /* status bit can be set.               */
  static char *status_msgs[] = {
   "","","","","",
   "Ring recovery, adapter transmitting/receiving claim tokens",
   "Only station on the ring",
   "Error counter overflow",
   "A remove MAC frame has been received",
   "",
   "Adapter hardware error on beacon auto-removal",
   "An open or short circuit has been detected in the lobe data path",
   "The adapter is transmitting beacon frames",
   "This adapter has transmitted a soft error report MAC frame",
   "Beacon frames are being transmitted or received",
   "Absence of any received signal detected"}; /* Possible status changes.   */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TRCb",(ulong)p_dds,(ulong)status,(ulong)0);
  fatal_error = clear_log = 0;

  /* Loop for each status bit. */
  for (cur = status,i = 0;cur && (!fatal_error);cur >>= 1, i++) {
     cur_bit = cur & 0x01;
     if ((cur_bit) && (status_msgs[i] != NULL)) {
       /* Log this one. */
       logerr(p_dds, TOK_RING_CHANGE + i);

       /* Also update the MIBS stuff. */
       switch (i) {
         case 5:
           WRK.network_status =  TR_MIB_RINGRECOVERY;
           limbo_code = TOK_RECOVERY_THRESH;
           break;
         case 6:
           WRK.network_status =  TR_MIB_SINGLESTATION;
           limbo_code = TOK_ADAP_OPEN;
           break;
         case 8:
           WRK.network_status =  TR_MIB_REMOVE_RX;
           limbo_code = TOK_RMV_ADAP;
           break;
         case 10:
           WRK.network_status =  TR_MIB_AUTOREMOVAL_ERR;
           limbo_code = TOK_RMV_ADAP;
           break;
         case 11:
           WRK.network_status =  TR_MIB_LOBEWIREFAULT;
           limbo_code = TOK_WIRE_FAULT;
           break;
         case 12:
           WRK.network_status =  TR_MIB_TX_BEACON;
           limbo_code = TOK_BEACONING;
           break;
         case 13:
           WRK.network_status =  TR_MIB_SOFT_ERR;
           limbo_code = TOK_ADAP_OPEN;
           break;
         case 14:
           WRK.network_status =  TR_MIB_HARD_ERROR;
           limbo_code = TOK_PERM_HW_ERR;
           break;
         case 15:
           WRK.network_status =  TR_MIB_SIGNAL_LOSS;
           limbo_code = TOK_WIRE_FAULT;
           break;
       }
     }
     /* Check too see if we need to restart the adapter. */
     if ((cur_bit) && ((i == 8) || (i == 10) || (i == 11))) {
       /* We need to issue a close command. */
       fatal_error = 1;
     }

     /* Check to see if the error counters must be reset. */
     if ((cur_bit) && (i == 7)) {
       clear_log = 1;
     }
  }

  /* Check if we have to read (reset the error counters). */
  if (clear_log) {
    rc = tok_issue_cmd(p_dds, SRB_READ_LOG, FALSE, NULL);
  }

  if (fatal_error) {
    /* Initiate the ring recovery procedure, i.e. restart the adapter. */
    tok_ring_recovery(p_dds, limbo_code);
  }
 TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"TRCe",(ulong)rc,(ulong)fatal_error,
   (ulong)clear_log);
 return(rc);
}


/*****************************************************************************/
/*
 * NAME:                  tok_srb_timeout
 *
 * FUNCTION:              Handle the timeout of a command waiting to
 *                        get access to the srb area. 
 *
 * EXECUTION ENVIRONMENT: System thread.
 *
 * NOTES:                 For the function to have access to the dds, the
 *                        watchdog structure has to be located right before
 *                        the dds.
 *
 * RETURNS:               nothing.
 *
 */
/*****************************************************************************/
void tok_srb_timeout(struct watchdog *p_wdt)  /* Timer structure ptr. */
{
  int rc;

  /* Find the 'hard-coded' postion of the dds. */
  dds_t *p_dds = *(dds_t **) ((ulong)p_wdt + sizeof(struct watchdog));

  /* A command waiting to get to the srb area timed out. */

  /* Check if is was an interrupt thread. */
  if (WRK.intr_action) {
    uint  cmd;                        /* Command to execute. */
    /* Try it again */
    cmd             = WRK.intr_action;
    WRK.intr_action = 0;
    rc = tok_issue_cmd(p_dds, cmd, FALSE, NULL);
  }

  /* If it's a process thread : it's sleeping, so wake it up. */
  if (WRK.srb_waiting) {

    /* If there was more then one, kick off the timer again. */
    if (WRK.srb_waiting > 1) {
      WRK.srb_wdt_active = TRUE;
      SRBWDT.count   = 0;
      SRBWDT.restart = 1;
      w_start(&SRBWDT);
    }
    e_wakeup(&WRK.srb_wait_event);
  }

  return;
}


/*****************************************************************************/
/*
 * NAME:                  tok_cmd_timeout
 *
 * FUNCTION:              Complete processing of a timeout of an
 *                        adapter command.
 *
 * EXECUTION ENVIRONMENT: System thread.
 *
 * NOTES:                 For the function to have access to the dds, the
 *                        watchdog structure has to be located right before
 *                        the dds.
 *
 * RETURNS:               nothing.
 *
 */
/*****************************************************************************/
void tok_cmd_timeout(struct watchdog *p_wdt)  /* Timer structure ptr. */
{
  /* Find the 'hard-coded' postion of the dds. */
  dds_t *p_dds = *(dds_t **) ((ulong)p_wdt + sizeof(struct watchdog));
  int   rc;                         /* Return code.                 */
  int   event;                      /* What cmd/event timed out ?   */
  int   wakeup;                     /* Is there a process sleeping. */
  int   slihpri;                    /* Save priority.               */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"OUTb",(ulong)0,(ulong)0,(ulong)0);

  /* Interrupt protect here. */
  slihpri = disable_lock(PL_IMP, &SLIH_LOCK);

  event = WRK.cmd_event & (~TOK_WAKEUP_BIT); /* Mask off the cmd-complete bit.*/
  wakeup = WRK.cmd_event & TOK_WAKEUP_BIT;
  WRK.cmd_event = 0;
  /* Wake up the cmd-thread that got stuck. */
  switch (event) {
    case TOK_READ_LOG_EVENT_BIT:
      /* The read-log failed, now wakeup the read-log thread. */
      if (wakeup) {
        e_wakeup(&WRK.read_log_event);
      }
      break;
    case TOK_OPEN_EVENT_BIT:
      /* The open failed, now wakeup the open thread. */
      if (wakeup) {
        e_wakeup(&WRK.open_wait_event);
      }
      break;
    case TOK_CLOSE_EVENT_BIT:
      /* The close failed, now wakeup the close thread. */
      if (wakeup) {
        e_wakeup(&WRK.close_event);
      } else {
          /* Retry the reset of adapter. */
          rc = tok_issue_cmd(p_dds, SRB_CLOSE, FALSE, NULL);
        }
      break;
    case TOK_SAP_OPEN_EVENT_BIT:
      /* The sap open failed, now wakeup the sap open thread. */
      if (wakeup) {
        e_wakeup(&WRK.sap_open_event);
      }
      break;
    case TOK_SAP_CLOSE_EVENT_BIT:
      /* The sap close failed, now wakeup the sap close thread. */
      if (wakeup) {
        e_wakeup(&WRK.sap_close_event);
      }
      break;
    case TOK_GROUP_ADDR_EVENT_BIT:
      /* The set-group-addr failed, now wakeup the set-group-addr thread. */
      if (wakeup) {
        e_wakeup(&WRK.grp_addr_event);
      }
      break;
    case TOK_FUNC_ADDR_EVENT_BIT:
      /* The set-func-addr failed, now wakeup the set-func-addr thread. */
      if (wakeup) {
        e_wakeup(&WRK.func_addr_event);
      }
      break;
  }

  /* Show that the command timer is available again. */
  WRK.cmd_wdt_active = FALSE;

  unlock_enable(slihpri, &SLIH_LOCK);

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"OUTe",(ulong)event,(ulong)0,(ulong)0);
}


/*****************************************************************************/
/*
 * NAME:                  tok_grp_cmd_response
 *
 * FUNCTION:              Handles the set-group-addr. command response.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:
 *
 * RETURNS:               0
 *
 */
/*****************************************************************************/
int tok_grp_cmd_response(dds_t *p_dds)     /* Device structure ptr. */
{
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"GRPb",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);

  /* Make sure it was the right event. */
  if (WRK.cmd_event & TOK_GROUP_ADDR_EVENT_BIT) {
    if (WRK.cmd_event & TOK_WAKEUP_BIT) {
      /* There is a process waiting on this interrupt. */
      WRK.cmd_event = 0;
      e_wakeup(&WRK.grp_addr_event);
    } else {
        WRK.cmd_event = 0;
        /* Just cancel the timeout timer, no processes waiting. */
        TOK_STOP_CMD_TIMER;

        /* Check to see if we are in the middle of an adapter bringup. */
        if (WRK.open_status == OPEN_STATUS4) {
          /* Continue the adapter bringup. */
          tok_ring_startup(&INITWDT);
        }
      }
  }

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"GRPe",(ulong)0,(ulong)0,(ulong)0);

  return(0);
}


/*****************************************************************************/
/*
 * NAME:                  tok_func_cmd_response
 *
 * FUNCTION:              Handles the set-functional-addr command response.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:
 *
 * RETURNS:               0
 *
 */
/*****************************************************************************/
int tok_func_cmd_response(dds_t *p_dds)  /* Device structure ptr. */
{
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"FUNb",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);

  /* Make sure it was the right event. */
  if (WRK.cmd_event & TOK_FUNC_ADDR_EVENT_BIT) {
    if (WRK.cmd_event & TOK_WAKEUP_BIT) {
      /* There is a process waiting on this interrupt. */
      WRK.cmd_event = 0;
      e_wakeup(&WRK.func_addr_event);
    } else {
        WRK.cmd_event = 0;
        /* Just cancel the timeout timer, no processes waiting. */
        TOK_STOP_CMD_TIMER;

        /* Check to see if we are in the middle of an adapter bringup. */
        if (WRK.open_status == OPEN_STATUS5) {
          /* Continue the adapter bringup. */
          tok_ring_startup(&INITWDT);
        }
      }
  }
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"FUNe",(ulong)0,(ulong)0,(ulong)0);
  return(0);
}


/*****************************************************************************/
/*
 * NAME:                  tok_open_sap_cmd_resp
 *
 * FUNCTION:              Handles the set-functional-addr command response.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_open_sap_cmd_resp(dds_t          *p_dds,   /* Device structure ptr.  */
                           volatile uchar *srb_ptr) /* Sys request block ptr. */
{
  ushort sap_value;

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SAPb",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);
  /* Save off our station id. */
  sap_value = (ushort) *(srb_ptr + 16); /* Get the sap. */
  /* Get the station id for that sap. */
  WRK.station_ids[sap_value] = ((*(srb_ptr + 4) & 0xff) << 8) +
   (uchar)(*(srb_ptr + 5) & 0x00ff);

  /* Make sure it was the right event. */
  if (WRK.cmd_event & TOK_SAP_OPEN_EVENT_BIT) {
    if (WRK.cmd_event & TOK_WAKEUP_BIT) {
      /* There is a process waiting on this interrupt. */
      WRK.cmd_event = 0;
      e_wakeup(&WRK.sap_open_event);
    } else {
        WRK.cmd_event = 0;
        /* Just cancel the timeout timer, no processes waiting. */
        TOK_STOP_CMD_TIMER;

        /* Check to see if we are in the middle of an adapter bringup. */
        if (WRK.open_status == OPEN_STATUS3) {
          /* Continue the adapter bringup. */
          tok_ring_startup(&INITWDT);
        }
      }
  }
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"SAPe",(ulong)WRK.station_ids[sap_value],
    (ulong)sap_value, (ulong)0);
  return;
}


/*****************************************************************************/
/*
 * NAME:                  tok_close_sap_cmd_resp
 *
 * FUNCTION:              Handles the set-functional-addr command response.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_close_sap_cmd_resp(dds_t *p_dds)    /* Device structure ptr. */
{
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CSPb",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);
  /* Make sure it was the right event. */
  if (WRK.cmd_event & TOK_SAP_CLOSE_EVENT_BIT) {
    if (WRK.cmd_event & TOK_WAKEUP_BIT) {
      /* There is a process waiting on this interrupt. */
      WRK.cmd_event = 0;
      e_wakeup(&WRK.sap_close_event);
    } else {
        WRK.cmd_event = 0;
        /* Just cancel the timeout timer, no processes waiting. */
        TOK_STOP_CMD_TIMER;
      }
  }
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CSPe",(ulong)0,(ulong)0,(ulong)0);
  return;
}


/*****************************************************************************/
/*
 * NAME:                  tok_close_cmd_resp
 *
 * FUNCTION:              Handles the close command response.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:                 Normally this function executes as part of a
 *                        shutdown sequence, but can also be part of
 *                        fatal error handeling.
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_close_cmd_resp(dds_t *p_dds)    /* Device structure ptr. */
{
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CRPb",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);
  /* Wakeup the close thread that got us here. */
  if (WRK.adap_state != DEAD_STATE) {
    if (WRK.cmd_event & TOK_CLOSE_EVENT_BIT) {
      if (WRK.cmd_event & TOK_WAKEUP_BIT) {
        /* There is a process waiting on this interrupt. */
        WRK.cmd_event = 0;
        e_wakeup(&WRK.close_event);
      } else {
          WRK.cmd_event = 0;
          /* Just cancel the timeout timer, no processes waiting. */
          TOK_STOP_CMD_TIMER;
        }
    }
  } else {
      /* The adapter has encountered a fatal error. */
      WRK.adap_state = OPEN_PENDING;

      /* Re-open the adapter and don't wait for the command response. */
      tok_adapter_open(p_dds, FALSE);
    }
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CRPe",(ulong)0,(ulong)0,(ulong)0);
  return;
}


/*****************************************************************************/
/*
 * NAME:                  tok_read_log_cmd_resp
 *
 * FUNCTION:              Handles the read-log command response.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread.
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_read_log_cmd_resp(dds_t          *p_dds,    /* Device structure ptr. */
                           volatile uchar *srb_ptr)  /* Shared mem. ptr.      */
{
  int   i;                   /* Simple loop counter.                     */
  uchar *log_ptr;            /* Storage area for the network log errors. */
  uchar *err_log;            /* Ptr. to the adapter network log errors.  */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"LOGb",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);
  /* Point to the log in shared memory. */
  err_log = srb_ptr + TOK_SRB_ERR_LOG_OFFSET;

  /* Point to the drivers storage for the log and copy the log to the driver. */
  log_ptr = (uchar *)&WRK.tok_network_errors;
  for (i = 0; i < sizeof(tok_error_log); i++)
     *(log_ptr++) = *(err_log++);

  if (WRK.cmd_event & TOK_WAKEUP_BIT) {
    /* Wakeup the thread that issued the command. */
    WRK.cmd_event = 0;
    e_wakeup(&WRK.read_log_event);
  } else {
      WRK.cmd_event = 0;
      /* Just cancel the timeout timer, no processes waiting. */
      TOK_STOP_CMD_TIMER;
    }

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"LOGe",(ulong)WRK.cmd_event,(ulong)0,(ulong)0);
  return;
}

