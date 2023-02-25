static char sccsid[] = "@(#)62  1.3  src/rspc/kernext/pcmcia/tok/itok_ioctl.c, pcmciatok, rspc41J, 9511A_all 3/9/95 02:37:35";
/*
static char sccsid[] = "@(#)48	1.7  src/rspc/kernext/isa/itok/itok_ioctl.c, isatok, rspc41J 9/8/94 14:23:22";
 */
/*
 *   COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 *   FUNCTIONS: tokioctl(), disable_address(), enable_address(), 
 *		get_addr(), get_mibs(), get_stats(),
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "itok_proto.h"

extern dd_ctrl_t    dd_ctrl;
extern token_ring_all_mib_t itok_isa_mibs_stats;


/*****************************************************************************/
/*
 * NAME:                  tokioctl
 *
 * FUNCTION:              ioctl entry point from kernel
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES: 
 *
 * RETURNS:               0 or errno
 *
 */
/*****************************************************************************/
int tokioctl(ndd_t   *p_ndd,	/* NDD structure for this device.           */
	      int     cmd,	/* ioctl operation desired.                 */
	      caddr_t arg,	/* arg for this cmd (usually a struct ptr). */
	      int     length)	/* length of argument for this command.     */
{
  int            rc;             /* Function return code.                 */
  int            txpri;          /* Old interrupt priority to restore to. */
  register dds_t *p_dds;         /* Device structure.                     */

  /* Get the adapter structure */
  p_dds = (dds_t *)(((ulong) p_ndd) - offsetof(dds_t, ndd));
    
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"IOCb",(ulong)p_dds,(ulong)cmd,(ulong)arg);
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"IOC2",(ulong)p_dds,(ulong)length,
    (ulong)WRK.adap_state);

  rc = 0;
  txpri = disable_lock(PL_IMP, &TX_LOCK);

  /* handle standard ioctl's */
  switch (cmd) {
    case NDD_CLEAR_STATS:		/* Clear all of the statistics */
      WRK.ndd_stime = WRK.dev_stime = lbolt;

      /* Read error log from adapter to clear it. */
      if ((WRK.adap_state == OPEN_STATE) &&(WRK.open_status == OPEN_COMPLETE)) {
	rc = tok_issue_cmd(p_dds, SRB_READ_LOG, TRUE, &WRK.tok_network_errors);
      }
      bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
      bzero(&TOKSTATS, sizeof(TOKSTATS));
      bzero(&DEVSTATS, sizeof(DEVSTATS));
      TOKSTATS.device_type = TOK_IBM_ISA;
      break;

  case NDD_DISABLE_ADDRESS:	/* Disable functional address */
    rc = disable_address(p_dds, arg);
    break;

  case NDD_DUMP_ADDR:		/* Return remote dump routine addr */
    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    *(uint *)arg = tokdump;
    break;

  case NDD_ENABLE_ADDRESS:	/* Enable functional address */
    rc = enable_address(p_dds, arg);
    break;

  case NDD_GET_ALL_STATS:	/* Get all the device statistics */
    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    if (length != sizeof(tr_ibm_isa_all_stats_t)) {
      rc = EINVAL;
      break;
    }

    /* Set general statistics and get statistics from device */
    rc = tok_get_stats(p_dds);

    /* copy statistics to user's buffer */
    bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
    bcopy(&(TOKSTATS), arg + sizeof(ndd_genstats_t),
		sizeof(tok_genstats_t) + sizeof(tr_ibm_isa_stats_t));
    break;

  case NDD_GET_STATS:		/* Get the generic statistics */
				/* that is the tok_ndd_stats  */
    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    if (length != sizeof(tok_ndd_stats_t)) {
      rc = EINVAL;
      break;
    }

    /* set general statistics and get statistics from device */
    rc = tok_get_stats(p_dds);

    /* copy statistics to user's buffer */
    bcopy (&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
    bcopy (&(TOKSTATS), arg + sizeof(ndd_genstats_t), sizeof(tok_genstats_t));
    break;

  case NDD_MIB_ADDR:	/* Get the receive addresses for this device:   */
                        /* physical, broadcast (plus functional, group) */
    rc = get_addr(p_dds, arg, length);
    TRACE_DBG(HKWD_ITOK_ISA_OTHER,"GET@",(ulong)p_dds,(ulong)rc,(ulong)length);
    break;

  case NDD_MIB_GET:		/* Get all the MIB values */
    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    if (length != sizeof(token_ring_all_mib_t)) {
      rc = EINVAL;
      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"GETM",(ulong)p_dds,(ulong)length,(ulong)0);
      break;
    }

    tok_get_mibs(p_dds, arg);

    break;

  case NDD_MIB_QUERY:		/* Query MIB support status */
    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    if (length != sizeof(token_ring_all_mib_t)) {
      rc = EINVAL;
      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"GETQ",(ulong)p_dds,(ulong)length,(ulong)0);
      break;
    }
    /* copy status to user's buffer */
    bcopy (&itok_isa_mibs_stats, arg, sizeof(token_ring_all_mib_t));
    break;

  case NDD_ADD_FILTER: {		/* Add a service access point. */
    ns_8022_t filter;

    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    if (length != sizeof(ns_8022_t)) {
      rc = EINVAL;
      break;
    }

    /* Get the filter information. */
    bcopy(arg, &filter, sizeof(ns_8022_t));

    if (!WRK.sap_allocated[filter.dsap]) {
      /* Issue the add filter/SAP command to the adapter. */
      rc = tok_issue_cmd(p_dds, SRB_OPEN_SAP, TRUE, &filter.dsap);
    }

    /* Increment the reference count. */
    WRK.sap_allocated[filter.dsap]++;

    break;
  }

  case NDD_DEL_FILTER: {	/* Delete a service access point. */
    ns_8022_t filter;

    if (arg == 0) {
      rc = EINVAL;
      break;
    }
    if (length != sizeof(ns_8022_t)) {
      rc = EINVAL;
      break;
    }

    /* Get the filter information. */
    bcopy(arg, &filter, sizeof(ns_8022_t));

    /* Decrement the reference count. */
    if (WRK.sap_allocated[filter.dsap] > 0 ) {
      WRK.sap_allocated[filter.dsap]--;
    }

    if (!WRK.sap_allocated[filter.dsap]) {
    /* Issue the delete filter/SAP command to the adapter. */
      rc = tok_issue_cmd(p_dds, SRB_CLOSE_SAP, TRUE, filter.dsap);
    }

    break;
	
  }
  default:            /* Invalid (unsupported) op */
    rc = EOPNOTSUPP;
  } /* end switch (cmd) */
   
  unlock_enable( txpri, &TX_LOCK );
    
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"IOCe",(ulong)p_dds,(ulong)rc,
    (ulong)NDD.ndd_flags);
  return (rc);
} /* end tokioctl */


/*****************************************************************************/
/*
 * NAME:                  disable_address
 *
 * FUNCTION:              disables an alternate (function or group) address
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:                 Will not immediatly set the address on the adapter
 *                        if an open is ongoing, will set a flag and let the
 *                        open thread do it.
 *
 * RETURNS:               0 or error.
 *
 */
/*****************************************************************************/
int disable_address(dds_t   *p_dds,      /* Device structure ptr.        */
                    caddr_t arg)         /* Group or functional address. */
{
  int rc;                                /* Function return code.        */
  int i, j;                              /* Simple loop counters.        */
  int temp_func;                         /* Current functional address.  */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"DISb",(ulong)p_dds,(ulong)*(arg),
    (ulong)0);

  /* Check if there is an argument. */
  if (arg == 0) {
    return(EINVAL);
  }
  rc = 0;
  if (*(uchar *)(arg + 2) & GROUP_ADR_MASK) {	/* group address */
    /* Can only disable the addresses which was enabled */
    if (*(uint *)WRK.open_cfg.grp_addr != *((uint *)(arg + 2))) {
      return(EINVAL);
    }

    /* * set group address to zero */
    *(uint *)WRK.open_cfg.grp_addr = 0;

    /* Not receiving data for a group address anymore.
     * If not receiving data for any functional addresses,
     * then not receiving data for any alternate addresses.  */
    NDD.ndd_flags &= ~TOK_RECEIVE_GROUP;
    if (!(*(uint *)WRK.open_cfg.func_addr)) {
      NDD.ndd_flags &= ~NDD_ALTADDRS;
    }

    /* If the adapter is not open yet, set a flag to have the command
       processed when the adapter is open.  */
    if (WRK.open_status < OPEN_STATUS4) {
      /* Still time to have the ongoing open do the command. */
      WRK.command_to_do |= TOK_GROUP_ADDR_EVENT_BIT;
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"DIS1",(ulong)WRK.open_status,(ulong)0,
			WRK.command_to_do);
      return(0);
    }

    /* Send off the command (change the group address). */
    rc = tok_issue_cmd(p_dds,SRB_SET_GROUP_ADDR, TRUE, WRK.open_cfg.grp_addr);

  } else {	/* functional address */
      temp_func = *(uint *)WRK.open_cfg.func_addr;

      /* can only disable an address which is currently enabled */
      if ((*((uint *)(arg + 2)) & *(uint *)WRK.open_cfg.func_addr) !=
            *((uint *)(arg + 2))) {
        return(EINVAL);
      }
      /* Keep a reference count on each of the bits in the address
         and only turn off the bits whose reference count is 0 */
      for (i = 0, j = 1; i < 31; i++, j *= 2) {
         if (*((uint *)(arg + 2)) & j) {
           WRK.func_addr_ref[i]--;
           if (! WRK.func_addr_ref[i]) {
	     *(uint *)WRK.open_cfg.func_addr &= ~j;
           }
         }
      }
      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DIS2",(ulong)p_dds,
           (ulong)WRK.open_cfg.func_addr,(ulong)0);

      /* Check if it is necessary to issue the set addr command */
      if (*(uint *)WRK.open_cfg.func_addr == temp_func) {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"DIS3",(ulong)WRK.adap_state,
                    (ulong)0,WRK.command_to_do);
        return(0);
      }
	
      /* If not receiving data for any functional addresses
         and not receiving data for a group address
         then not receiving data for any alternate addresses.  */
      if (!(*(uint *)WRK.open_cfg.func_addr)) {
        NDD.ndd_flags &= ~TOK_RECEIVE_FUNC;
        if (!(*(uint *)WRK.open_cfg.grp_addr)) {
          NDD.ndd_flags &= ~NDD_ALTADDRS;
        }
      }
    if (WRK.open_status < OPEN_STATUS5) {
      /* Still time to have the ongoing open do the command. */
        WRK.command_to_do = TOK_FUNC_ADDR_EVENT_BIT;
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"DIS4",(ulong)WRK.open_status,(ulong)0,
        (ulong)WRK.command_to_do);
      return(0);
    }

    rc = tok_issue_cmd(p_dds, SRB_SET_FUNC_ADDR, TRUE, WRK.open_cfg.func_addr);

   }
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"DISe",(ulong)WRK.adap_state,(ulong)0,
    (ulong)0);
  return(rc);
} /* end disable_address */


/*****************************************************************************/
/*
 * NAME:                  enable_address
 *
 * FUNCTION:              enables an alternate (function or group) address
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:                 Will not immediatly set the address on the adapter
 *                        if an open is ongoing, will set a flag and let the
 *                        open thread do it.
 *
 * RETURNS:               0 or error code.
 *
 */
/*****************************************************************************/
int enable_address(dds_t   *p_dds,       /* Device driver structure ptr. */
                   caddr_t arg)          /* Group or functional address. */
{
  int rc;                     /* Function return code.       */
  int i, j;                   /* Simple loop counters.       */
  int  temp_func;             /* Current functional address. */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENAb",(ulong)p_dds,(ulong)*(arg),
    (ulong)0);

  /* Make sure we have an argument. */
  if (arg == 0) {
    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENAe",(ulong)p_dds,(ulong)EINVAL,(ulong)1);
    return(EINVAL);
  }
  rc = 0;
  if (*(uchar *)(arg+2) & GROUP_ADR_MASK) {	/* group address */
    /* Only 1 group address is allowed */
    if (*(uint *)(WRK.open_cfg.grp_addr)) {
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENAe",(ulong)p_dds,(ulong)EINVAL,
        (ulong)2);
      return(EINVAL);
    }

    /* Set new group address */
    *(uint *)WRK.open_cfg.grp_addr = *((uint *)(arg + 2));

    NDD.ndd_flags |= NDD_ALTADDRS+TOK_RECEIVE_GROUP;

    if (WRK.open_status < OPEN_STATUS4) {
      /* Still time to have the ongoing open do the command. */
      WRK.command_to_do = TOK_GROUP_ADDR_EVENT_BIT;
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENA1",(ulong)WRK.open_status,(ulong)0,
        (ulong)WRK.command_to_do);
      return(0);
    }

    /* Send off the command (change the group address). */
    rc = tok_issue_cmd(p_dds,SRB_SET_GROUP_ADDR, TRUE, WRK.open_cfg.grp_addr);

  } else {	/* functional address */
      temp_func = *(uint *)(WRK.open_cfg.func_addr);

      /* Add this address to the functional address mask */
      *(uint *)WRK.open_cfg.func_addr |= *((uint *)(arg + 2));
      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"ENA2",(ulong)p_dds,
         (ulong)WRK.open_cfg.func_addr,(ulong)0);
	
      NDD.ndd_flags |= NDD_ALTADDRS+TOK_RECEIVE_FUNC;

      /* Keep a reference count on each of the bits in the address */
      for (i = 0, j = 1; i < 31; i++, j *= 2) {
         if ( *((uint *) (arg + 2)) & j) {
           WRK.func_addr_ref[i]++;
         }
      }

       /* Check if it is necessary to issue the set addr command */
       if (*(uint *)WRK.open_cfg.func_addr == temp_func) {
         TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENA3",(ulong)WRK.adap_state,
           (ulong)0,(ulong)WRK.command_to_do);
         return(0);
       }

    if (WRK.open_status < OPEN_STATUS5) {
      /* Still time to have the ongoing open do the command. */
      WRK.command_to_do = TOK_FUNC_ADDR_EVENT_BIT;
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENA4",(ulong)WRK.open_status,(ulong)0,
        (ulong)WRK.command_to_do);
      return(0);
    }
	
    rc = tok_issue_cmd(p_dds, SRB_SET_FUNC_ADDR, TRUE, WRK.open_cfg.func_addr);
  }

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"ENAe",(ulong)WRK.adap_state,(ulong)0,
    (ulong)WRK.command_to_do);
  return(rc);
} /* end enable_address */


/*****************************************************************************/
/*
 * NAME:                  get_addr
 *
 * FUNCTION:              returns the receive addresses for this device
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:               0 or errorcode.
 */
/*****************************************************************************/
int get_addr(dds_t   *p_dds,               /* Device structure ptr. */
             caddr_t arg,                  /* Address argument.     */
             int     length)               /* Length in bytes.      */
{
  int                 i;                                /* Simple loop counter*/
  int                 rc;                               /* Return code.       */
  int                 elem_len;                         /* Address length.    */
  int                 count;                            /* How many addresses.*/
  ndd_mib_addr_t      *p_table = (ndd_mib_addr_t *)arg; /* Must be this type. */
  ndd_mib_addr_elem_t *p_elem;                          /* Current address.   */
  /* The two possible broadcast addresses. */
  uchar broad_adr1[CTOK_NADR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uchar broad_adr2[CTOK_NADR_LENGTH] = {0xC0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

  if (arg == 0) {
    return(EINVAL);
  }
  if (length < sizeof(ndd_mib_addr_t)) {
    return(EINVAL);
  }

  rc = count = 0;
  elem_len = 6 + CTOK_NADR_LENGTH;
  length -= sizeof(u_int);   /* room for count field */
  arg += sizeof(u_int);

  /* copy the specific network address in use first */
  if (length >= elem_len) {
    p_elem             = (ndd_mib_addr_elem_t *)arg;
    p_elem->status     = NDD_MIB_VOLATILE;
    p_elem->addresslen = CTOK_NADR_LENGTH;
    for (i = 0; i < CTOK_NADR_LENGTH; i++)
       p_elem->address[i] = WRK.open_cfg.cur_tok_addr[i];
    length -= elem_len;
    arg    += elem_len;
    count++;
  } else {
      rc = E2BIG;
    }

  /* copy the first broadcast address */
  if (length >= elem_len) {
    p_elem             = (ndd_mib_addr_elem_t *)arg;
    p_elem->status     = NDD_MIB_NONVOLATILE;
    p_elem->addresslen = CTOK_NADR_LENGTH;
    for (i = 0; i < CTOK_NADR_LENGTH; i++)
       p_elem->address[i] = broad_adr1[i];
    length -= elem_len;
    arg    += elem_len;
    count++;
  } else {
      rc = E2BIG;
    }

  /* copy the second broadcast address */
  if (length >= elem_len) {
    p_elem             = (ndd_mib_addr_elem_t *)arg;
    p_elem->status     = NDD_MIB_NONVOLATILE;
    p_elem->addresslen = CTOK_NADR_LENGTH;
    for (i = 0; i < CTOK_NADR_LENGTH; i++)
       p_elem->address[i] = broad_adr2[i];
    length -= elem_len;
    arg    += elem_len;
    count++;
  } else {
      rc = E2BIG;
    }

  if ( NDD.ndd_flags & TOK_RECEIVE_FUNC) {
    /* copy the functional address */
    if (length >= elem_len) {
      p_elem             = (ndd_mib_addr_elem_t *)arg;
      p_elem->status     = NDD_MIB_VOLATILE;
      p_elem->addresslen = CTOK_NADR_LENGTH;
      *(char *)(p_elem->address)     = 0xC0;
      *(char *)(p_elem->address + 1) = 0x00;
      *(ulong *)((char *)(p_elem->address + 2)) = 
                  *((ulong *)(WRK.open_cfg.func_addr));
      length -= elem_len;
      arg += elem_len;
      count++;
    } else {
	rc = E2BIG;
      }
  }

  if (NDD.ndd_flags & TOK_RECEIVE_GROUP) {
    /* copy the group address */
    if (length >= elem_len) {
      p_elem             = (ndd_mib_addr_elem_t *)arg;
      p_elem->status     = NDD_MIB_VOLATILE;
      p_elem->addresslen = CTOK_NADR_LENGTH;
      *(char *)(p_elem->address)     = 0xC0;
      *(char *)(p_elem->address + 1) = 0x00;
      *(ulong *)((char *)(p_elem->address + 2)) = 
      *((ulong *)(WRK.open_cfg.grp_addr));
      length -= elem_len;
      arg += elem_len;
      count++;
    } else {
        rc = E2BIG;
      }
  }
		
  /* put the final count into the buffer */
  p_table->count = count;
  return(rc);
} /* end get_addr */


/*****************************************************************************/
/*
 * NAME:                  tok_get_mibs
 *
 * FUNCTION:              Gathers all the mib variables and puts them in 
 *                        the MIB struct
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_get_mibs(dds_t	               *p_dds,    /* Device structure ptr. */
                  token_ring_all_mib_t *arg)      /* User MIBS struct ptr. */
{
  int i;                 /* Simple loop counter. */
  int rc;                /* Temp. return code. */
  int ring_state;        /* Network ring state.  */

  /* Read error log from adapter to get statistics */
  rc = tok_get_stats(p_dds);

  bzero(arg, sizeof(token_ring_all_mib_t));

  bcopy(TR_MIB_IBM16, arg->Generic_mib.ifExtnsEntry.chipset,CHIPSETLENGTH);
  arg->Generic_mib.ifExtnsEntry.mcast_tx_ok = TOKSTATS.mcast_xmit;
  arg->Generic_mib.ifExtnsEntry.bcast_tx_ok = TOKSTATS.bcast_xmit;
  arg->Generic_mib.ifExtnsEntry.mcast_rx_ok = TOKSTATS.mcast_recv;
  arg->Generic_mib.ifExtnsEntry.bcast_rx_ok = TOKSTATS.bcast_recv;
  arg->Generic_mib.ifExtnsEntry.promiscuous = PROMFALSE;

  arg->Generic_mib.RcvAddrTable = 3;
  if ( NDD.ndd_flags & TOK_RECEIVE_FUNC) {
    arg->Generic_mib.RcvAddrTable++;
  }
  if (NDD.ndd_flags & TOK_RECEIVE_GROUP) {
    arg->Generic_mib.RcvAddrTable++;
  }

  arg->Token_ring_mib.Dot5Entry.ring_status  = WRK.network_status;
  
  /* Translate the device driver ring state to the MIBS world. */
  switch (WRK.adap_state) {
    case OPEN_STATE:
      ring_state = TR_MIB_OPENED;
      break;
    case OPEN_PENDING:
      ring_state = TR_MIB_OPENING;
      break;
    case CLOSED_STATE:
      ring_state = TR_MIB_CLOSED;
      break;
    case CLOSE_PENDING:
      ring_state = TR_MIB_CLOSING;
      break;
  }
  arg->Token_ring_mib.Dot5Entry.ring_state   = ring_state;
  arg->Token_ring_mib.Dot5Entry.ring_ostatus = WRK.last_open;

  /* What ring-speed are we running at. */
  if (DDI.ring_speed) {
    arg->Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_SIXTEENMEGABIT;
  } else {
      arg->Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_FOURMEGABIT;
    }

  /* Get the upstream neighbor's address (NAUN) */
  if (tok_get_ring_info(p_dds)) {
    bzero(arg->Token_ring_mib.Dot5Entry.upstream, CTOK_NADR_LENGTH);
  } else {
    for (i = 0; i < CTOK_NADR_LENGTH; i++)
       arg->Token_ring_mib.Dot5Entry.upstream[i] =
         WRK.ring_info.upstream_node_addr[i];
    }

  arg->Token_ring_mib.Dot5Entry.participate = TR_MIB_TRUE;

  arg->Token_ring_mib.Dot5Entry.functional[0] = 0xC0;
  arg->Token_ring_mib.Dot5Entry.functional[1] = 0x00;
  arg->Token_ring_mib.Dot5Entry.functional[2] = WRK.open_cfg.func_addr[0];
  arg->Token_ring_mib.Dot5Entry.functional[3] = WRK.open_cfg.func_addr[1];
  arg->Token_ring_mib.Dot5Entry.functional[4] = WRK.open_cfg.func_addr[2];
  arg->Token_ring_mib.Dot5Entry.functional[5] = WRK.open_cfg.func_addr[3];

  arg->Token_ring_mib.Dot5StatsEntry.line_errs     = TOKSTATS.line_errs;
  arg->Token_ring_mib.Dot5StatsEntry.burst_errs    = TOKSTATS.burst_errs;
  arg->Token_ring_mib.Dot5StatsEntry.ac_errs       = TOKSTATS.ac_errs;
  arg->Token_ring_mib.Dot5StatsEntry.abort_errs    = TOKSTATS.abort_errs;
  arg->Token_ring_mib.Dot5StatsEntry.int_errs      = TOKSTATS.int_errs;
  arg->Token_ring_mib.Dot5StatsEntry.lostframes    = TOKSTATS.lostframes;
  arg->Token_ring_mib.Dot5StatsEntry.rx_congestion = TOKSTATS.rx_congestion;
  arg->Token_ring_mib.Dot5StatsEntry.framecopies   = TOKSTATS.framecopies;
  arg->Token_ring_mib.Dot5StatsEntry.token_errs    = TOKSTATS.token_errs;
  arg->Token_ring_mib.Dot5StatsEntry.soft_errs     = TOKSTATS.soft_errs;
  arg->Token_ring_mib.Dot5StatsEntry.hard_errs     = TOKSTATS.hard_errs;
  arg->Token_ring_mib.Dot5StatsEntry.signal_loss   = TOKSTATS.signal_loss;
  arg->Token_ring_mib.Dot5StatsEntry.tx_beacons    = TOKSTATS.tx_beacons;
  arg->Token_ring_mib.Dot5StatsEntry.recoverys     = TOKSTATS.recoverys;
  arg->Token_ring_mib.Dot5StatsEntry.lobewires     = TOKSTATS.lobewires;
  arg->Token_ring_mib.Dot5StatsEntry.removes       = TOKSTATS.removes;
  arg->Token_ring_mib.Dot5StatsEntry.singles       = TOKSTATS.singles;
}

/*****************************************************************************/
/*
 * NAME:                  tok_get_ring_info
 *
 * FUNCTION:              reads the adapter memory to get ring information
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:               0 - adapter is up and running OK
 *	                  ENETUNREACH - adapter is in any other state
 */
/*****************************************************************************/
int tok_get_ring_info(dds_t *p_dds)   /* Device structure ptr. */
{
  int             i;                  /* Simple loop counter.   */
  uchar           *dp;                /* Ring info destination. */
  volatile uchar  *memaddr;           /* Shared mem. offset.    */
  volatile uchar  *param_area_ptr;    /* Shared mem. ptr.       */

  if (WRK.adap_state != OPEN_STATE) {
    return(ENETUNREACH);
  }

  /* Map in the param shared memory area. */
  memaddr = iomem_att(dd_ctrl.mem_map_ptr);
  param_area_ptr = memaddr + DDI.shared_mem_addr + WRK.adap_param_offset;

  /* Byte copy the data, remember that some devices are 8 bit wide only. */
  for (dp = (uchar *)&WRK.ring_info, i = 0;i < sizeof(tok_ring_info_t);i++)
     *dp++ = *param_area_ptr++;

  iomem_det(memaddr);

  return(0);

} /* end get_ring_info */

/*****************************************************************************/
/*
 * NAME:                  tok_get_stats
 *
 * FUNCTION:              reads the error log and adapter memory to 
 *                        get statistics
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:               nothing
 */
/*****************************************************************************/
int tok_get_stats(dds_t *p_dds)     /* Device structure ptr. */
{
  int i;
  int rc;

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"GETb",(ulong)p_dds,(ulong)WRK.adap_state,
    (ulong)0);

  rc = 0;
  NDD.ndd_genstats.ndd_elapsed_time = NDD_ELAPSED_TIME(WRK.ndd_stime);
  TOKSTATS.dev_elapsed_time         = NDD_ELAPSED_TIME(WRK.dev_stime);
  TOKSTATS.ndd_flags                = NDD.ndd_flags;
  TOKSTATS.sw_txq_len               = WRK.xmits_queued;
  TOKSTATS.hw_txq_len               = 1;
  NDD.ndd_genstats.ndd_xmitque_cur  = TOKSTATS.sw_txq_len + TOKSTATS.hw_txq_len;

  /* Return the hw. address. */
  for (i = 0; i < CTOK_NADR_LENGTH; i++) {
     TOKSTATS.tok_nadr[i] = p_dds->vpd.vpd[i];
  }

  if (WRK.adap_state != OPEN_STATE) {
    return(ENETUNREACH);
  }

  /* Read the current log from the adapter. */
  rc = tok_issue_cmd (p_dds, SRB_READ_LOG, TRUE, NULL);

  TOKSTATS.line_errs           += WRK.tok_network_errors.line_errors;
  TOKSTATS.burst_errs          += WRK.tok_network_errors.burst_errors;
  TOKSTATS.abort_errs          += WRK.tok_network_errors.abort_delimiters;
  TOKSTATS.int_errs            += WRK.tok_network_errors.internal_errors;
  TOKSTATS.lostframes          += WRK.tok_network_errors.lost_frames;
  TOKSTATS.rx_congestion     += WRK.tok_network_errors.receive_congestion_count;
  NDD.ndd_genstats.ndd_ierrors
        += WRK.tok_network_errors.receive_congestion_count;
  TOKSTATS.framecopies         += WRK.tok_network_errors.frame_copied_errors;
  TOKSTATS.token_errs          += WRK.tok_network_errors.token_errors;
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"GETe",(ulong)p_dds,(ulong)0,(ulong)0);
  return(rc);
} /* end get_stats */
