static char sccsid[] = "@(#)53	1.3  src/rspc/kernext/isa/itok/itok_dump.c, isatok, rspc41J, 9514A_all 4/3/95 13:45:57";
/*
 *   COMPONENT_NAME: ISATOK - 16/4 PowerPC Token-ring driver
 *
 *   FUNCTIONS: tokdump(), tok_dump_read(), tok_dump_wrt(), tok_dump_setup()
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/poll.h>
#include <sys/systemcfg.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_802_5.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "itok_proto.h"

extern dd_ctrl_t        dd_ctrl;


/************************************************************************/
/*                                                                      
 * NAME:                  tokdump 
 *                                                                    
 * FUNCTION:              Dump entry point
 *                                                                  
 * EXECUTION ENVIRONMENT: This routine is called when there is certain to be
 *                        limited functionality available in the system. The
 *                        passed data is already pinned by the caller. There
 *                        are no interrupt or timer kernel services available.
 *                        This routine runs at INTMAX level.
 *                                                                  
 * NOTES:                 The device must previously have been opened.
 *                        The driver ignores extra dumpinit and dumpstart calls.
 *                        The driver checks for the availability of internal
 *                        resources, as these are not guaranteed to be 
 *                        available when the dump routine is called, but the
 *                        system will have tried to quiesce itself before 
 *                        making the first dump call.
 *                        Any lack of resources, or error in attempting to run
 *                        any command, is considered fatal in the context of
 *                        the dump.     
 *                        Normal operation will not continue once this routine
 *                        has been executed.  Once the DUMPWRITE logic has 
 *                        been executed, in fact, it will be impossible to use 
 *                        the driver's normal path successfully.
 *                                                                   
 * RETURNS:               0 or some error.
 *                                                                     
 */
/************************************************************************/
int tokdump(ndd_t   *p_ndd,      /* Network device driver structure. */
	    int	    cmd,         /* Dump command to execute.         */
	    caddr_t arg)         /* User argument.                   */
{
  register dds_t  *p_dds;           /* Device structure ptr.           */
  int              rc;              /* Function return code.           */
  struct dmp_query dump_ptr;        /* Dump info. structure.           */
  int              dump_pri;        /* Old system priority to restore. */
  int              tok_dump_wrt();  /* The new write entry point.      */
    
  /* Setup the device structure. */
  p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));
  TRACE_SYS(HKWD_ITOK_ISA_OTHER,"DUMb",(ulong)p_dds,(ulong)cmd,(ulong)arg);

  rc = 0;
  /* Process dump command */
  dump_pri = i_disable(INTMAX); /* Make sure that our own interrupt handler */
  switch (cmd) {                /* does not run anymore. */
    case DUMPQUERY:
      dump_ptr.min_tsize = CTOK_MIN_PACKET;
      dump_ptr.max_tsize = 4096;
      TRACE_SYS(HKWD_ITOK_ISA_OTHER,"DUMq",(ulong)dump_ptr.min_tsize, 
        (ulong)dump_ptr.max_tsize,(ulong)0);
      bcopy(&dump_ptr,arg,sizeof(struct dmp_query));
      break;
	
    case DUMPSTART:
      TRACE_SYS(HKWD_ITOK_ISA_OTHER,"DUMs",(ulong)0,(ulong)0,(ulong)0);
      if (tok_dump_setup(p_dds)) {
        TRACE_BOTH(HKWD_ITOK_ISA_OTHER,(ulong)"DUM1",(ulong)0,(ulong)0,
          (ulong)0);
        return(ENETDOWN);
      }
      /* The new output function will now be the dump write function. */
      NDD.ndd_output     = tok_dump_wrt;
      WRK.dump_first_wrt = TRUE;
      break;
	
    case DUMPREAD:
	/* Call internal routine to execute the command */
	rc = tok_dump_read(p_dds, arg);
	if (rc == -1)
	    return(ENETDOWN);
	break;
	
    case DUMPEND:
    case DUMPINIT:
    case DUMPTERM:
	break;
	
    default:
      rc = EINVAL;
      break;
    }	/* end switch (cmd) */
    
  TRACE_SYS(HKWD_ITOK_ISA_OTHER, "DUMe",(ulong)0,(ulong)0,(ulong)0);
  i_enable(dump_pri);
  return (rc);
}  /* End of tokdump */


/************************************************************************/
/*
 *  NAME:                  tok_dump_read
 *
 *  FUNCTION:              Process a dump read request
 *
 *  EXECUTION ENVIRONMENT: Called by the dump entry point.
 *
 *  NOTES:
 *
 *  RETURNS:               0 or -1. 
 */
/************************************************************************/
tok_dump_read(dds_t   *p_dds,     /* Device structure.           */
              caddr_t arg)	  /* Pointer to read parameters. */
{
  struct dump_read {
    struct mbuf *dump_bufread;
    int         wait_time;
  };	

  struct dump_read    dump_read;	/* mbuf pointer & wait time	*/
  struct timestruc_t  current_time,   /* Keeps the current time       */
                      timeout_time,   /* Time out value               */
                      temp_time;
  struct ip           *p_ip;          /* Pointer to IP header         */
  struct ie2_llc_snaphdr  *p_llc;     /* Pointer to LLC header        */
  struct ie5_mac_hdr  *p_mac;         /* Pointer to MAC header        */
  struct ie5_arp      *p_arp;         /* Pointer to ARP information   */
  ulong		mask;		/* bus_intr_lvl mask		*/
    
  TRACE_DBG(HKWD_ITOK_ISA_OTHER, "DRDb", p_dds, 0, 0);

  bcopy (arg, dump_read, sizeof(dump_read));

  /* Set up time information */
  ms2time(dump_read.wait_time, &temp_time);
  curtime(&current_time);
  ntimeradd(current_time, temp_time, timeout_time);
    
  TRACE_DBG(HKWD_ITOK_ISA_OTHER, "DRDe", 0, 0, 0);
    
  return(0);
} /* End of tok_dump_read() */


/************************************************************************/
/*
 *  NAME:                  tok_dump_wrt
 *
 *  FUNCTION:              Process a dump write request
 *
 *  EXECUTION ENVIRONMENT: Called on the process thread by the dump DD.
 *
 *  NOTES:            
 *
 *  RETURNS:               0 or -1. 
 */
/************************************************************************/
int tok_dump_wrt(ndd_t	*p_ndd,       /* Network device driver structure ptr. */
                 struct mbuf *p_mbuf) /* Data to transmit.                    */
{
  dds_t              *p_dds;           /* Device structure ptr.          */
  int                rc;               /* Function return code.          */
  struct timestruc_t current_time,     /* Keeps the current time         */
  timeout_time,                        /* Time out value                 */
  temp_time;
  uchar      correlator;               /* To keep track of xmissions.    */
  uchar      intr_val;                 /* 'Interrupt' value.             */
  ushort     dhb_offset;               /* Where in shared mem. to write. */
  xmt_elem_t *xlm;                     /* Transmit element.              */
  volatile uchar *memaddr;             /* Shared mem. offset.            */
  volatile uchar *srb_ptr, *aca_area,  /* Shared mem. areas.             */
                 *arb_ptr;
    
  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DWRb",(ulong)0,(ulong)0,(ulong)0);
    
  /* Get adapter structure */
  p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));

  /* If a dump read has not finished processing then we need to  */
  /* finish that processing. */
  if (WRK.dump_read_started) {
    clean_up_read(p_dds);
  }

  /* Map in the some shared memory areas. */
  memaddr = iomem_att(dd_ctrl.mem_map_ptr);
  aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr +
    ACA_AREA_OFFSET;
  arb_ptr = memaddr + DDI.shared_mem_addr + WRK.arb.offset;

  /* Get a device driver transmit element. */
  GET_FREE_ELEM(xlm);

  xlm->mbufp  = p_mbuf;
  xlm->in_use = 1;
  xlm->bytes  = p_mbuf->m_pkthdr.len;

  ADD_2_ARB_Q(xlm)

  /* Send it. */
  /* Map in the SRB shared memory area. */
  srb_ptr = memaddr + BID_ALTREG(DDI.bus_id, ISA_BUSMEM),
		DDI.shared_mem_addr + WRK.srb.offset;

  /* Set up the tx command. */
  *(srb_ptr)                           = SRB_TX_DIR_FRAME;
  *(srb_ptr + 1)                       = 0;
  *(srb_ptr + SRB_RETCODE_OFFSET)      = SRB_INITIATED;
  *(srb_ptr + 3)                       = 0;
  *(srb_ptr + SRB_STATION_ID_OFFSET)   = (uchar)0;
  *(srb_ptr + SRB_STATION_ID_OFFSET+1) = (uchar)0;

  /* Send it. */
  *(aca_area + ISRA_ODD_SET) = 0x28;

  /* Set up timeout information */
  ms2time(1000, &temp_time);
  curtime(&current_time);
  ntimeradd(current_time, temp_time, timeout_time);

  /* Now wait for the ARB command from the adapter. */
  while (!((intr_val = *(aca_area + ISRP_ODD_OFFSET)) & ARB_COMMAND)) {
    /* Check to see if timeout has been reached.  */
    if (ntimercmp(current_time, timeout_time, >)) {

      /* Detach the mory areas. */
      iomem_det(memaddr);

      TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DUMx",(ulong)intr_val,(ulong)0,(ulong)0);
      return (0);
    }
    curtime(&current_time);
  }

  /* Reset the interrupt. */
  *(aca_area + ISRP_ODD_RESET) = ~intr_val;

  /* We should now have the ARB - tx request. */
  correlator = *(arb_ptr + 1) & 0xff;
  dhb_offset = *(arb_ptr + 6) << 8;
  dhb_offset += *(arb_ptr + 7)  & 0xff;
  (*(WRK.tx_func))(p_dds, correlator, dhb_offset);

  /* Detach the aca area. */
  iomem_det(memaddr);

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DWRe",(ulong)0,(ulong)0,(ulong)0);
    
  return(0);
} /* End of tok_dump_wrt() */

/************************************************************************/
/*
 *  NAME:                  tok_dump_setup
 *
 *  FUNCTION:              Sync's ongoing and waiting transmissions.
 *
 *  EXECUTION ENVIRONMENT: Called by the dump entry point.
 *
 *  NOTES:
 *
 *  RETURNS:               0 or -1.
 */
/************************************************************************/
int tok_dump_setup(dds_t *p_dds)           /* Device structure ptr. */
{
  int                i;            /* Simple loop counter.        */
  int                rc;           /* Function return code.       */
  int                index;        /* Current xmit-elem index.    */
  xmt_elem_t         *cur_elem;    /* Current xmit-element.       */
  volatile uchar     *memaddr;     /* Shared mem. offset.         */
  volatile uchar     *aca_area;    /* Shared mem. registers.      */

  /* First make sure that we don't get any more interrupts. */
  /* i.e. disable the interrupts from the adapter. */
  /* Map in the ACA shared memory area. */
  memaddr = iomem_att(dd_ctrl.mem_map_ptr);
  aca_area = memaddr + dd_ctrl.adap_cfg[WRK.minor_no].bios_addr +
    ACA_AREA_OFFSET;
  
  /* Disable adapter software interrupts. */
  *(aca_area + ISRP_EVEN_RESET) = 0x40;

  /* Stop any active timers. */
  if (WRK.xmit_wdt_active) {
    WRK.xmit_wdt_active = 0;
    w_stop(&XMITWDT);
  }
  if (WRK.init_wdt_active) {
    WRK.init_wdt_active = 0;
    w_stop(&INITWDT);
  }
  if (WRK.cmd_wdt_active) {
    WRK.cmd_wdt_active = 0;
    w_stop(&CMDWDT);
  }

 /* Reset the transmit wait list, freeing any mbufs. */
  cur_elem = (xmt_elem_t *)&(WRK.tx_waitq[0]);
  for (i = 0; i < DDI.xmt_que_size; i++, cur_elem++) {
     if (cur_elem->in_use) {
       /* This data will just be flushed. */
       m_freem(cur_elem->mbufp);
     }
     /* Reset all information. */
     cur_elem->bytes  = 0;
     cur_elem->in_use = 0;
     cur_elem->mbufp  = 0;
  }

  /* Reset the rest of the xmit-list variables. */
  WRK.xmits_queued      = 0;
  WRK.tx_buf_in_use     = 0;
  WRK.tx_stat           = 0;
  WRK.tx_next_free = &WRK.tx_waitq[0];
  WRK.tx_last_free = &WRK.tx_waitq[DDI.xmt_que_size-1];

  /* Finally sync up with the adapter. */
  *(aca_area + ISRA_ODD_SET) = 0x0f; /* Free the command areas. */

  /* Detach the aca area. */
  iomem_det(memaddr);

  return(0);
} /* End of tok_dump_setup() */

/* Convert a micro-second to a rtcl_t time value.  */
#define	MSEC_PER_SEC	(NS_PER_SEC / NS_PER_MSEC)
ms2time(int                msec,      /* Timeout value (input).         */
        struct timestruc_t *tp)       /* Converted time value (output). */
{
  tp->tv_sec = msec / MSEC_PER_SEC;
  tp->tv_nsec = (msec % MSEC_PER_SEC) * NS_PER_MSEC;
}

/************************************************************************/
/*
 *  NAME: clean_up_read
 *
 *  FUNCTION:
 *
 *  This routine reads in all frames currently on the receive chain and
 *  drops them.  It then updates the receive counters and issues a
 *  RECV_VALID to the adapter to notify that we have processed the
 *  receive interrupt.
 *
 *  EXECUTION ENVIRONMENT:
 *  This routine is called from the tok_dump_wrt entry point to clean
 *  up any outstanding read interrupts before processing the write
 *  request.
 */
/************************************************************************/
int clean_up_read(dds_t *p_dds)     /* Device structure ptr. */
{
  int rc;

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EANb",(ulong)0,(ulong)0,(ulong)0);
  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"EANb",(ulong)0,(ulong)0,(ulong)0);
  return(rc);
} /* End of clean_up_read() */
