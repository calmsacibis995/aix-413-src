static char sccsid[] = "@(#)54  1.5  src/rspc/kernext/pcmcia/tok/itok_close.c, pcmciatok, rspc41J, 9516A_all 4/18/95 00:54:14";
/*
static char sccsid[] = "@(#)46	1.5  src/rspc/kernext/isa/itok/itok_close.c, isatok, rspc41J 9/8/94 14:23:43";
 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: tokclose()
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
 * NAME:                  tokclose
 *
 * FUNCTION:              close entry point from kernel
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int tokclose(ndd_t *p_ndd)           /* Network device driver structure ptr. */
{
  int        rc;               /* Function return code.                       */
  int        slihpri;          /* Old priority to return to.                  */
  int        sap;              /* SAP to close.                               */
  int        max_wait_cycles;  /* How many times should be wait on tx. tmout. */
  xmt_elem_t *cur_elem;        /* Current tx-element.                         */
  dds_t      *p_dds;           /* Device driver structure.                    */

  rc = 0;

  /* Get the adapter structure */
  p_dds = (dds_t *) (((ulong) p_ndd) - offsetof(dds_t, ndd));

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CLOb",(ulong)p_ndd,(ulong)dd_ctrl.num_opens,
    (ulong)WRK.adap_state); 

#ifdef PCMCIATOK
  if( WRK.adap_state == CLOSED_STATE ) {
      TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CLO1",(ulong)p_ndd,(ulong)dd_ctrl.num_opens,
          (ulong)WRK.adap_state); 
      return( 0 );
  }
#endif

  slihpri = disable_lock(PL_IMP, &SLIH_LOCK);

  /* Do the appropriate shutdown thing, dependent on the current state. */
  switch (WRK.adap_state) {

    case OPEN_STATE:
      /* This would be the normal case. */
      WRK.adap_state = CLOSE_PENDING;
      break;

    case OPEN_PENDING:
      tok_reset_adapter(p_dds);
      WRK.adap_state = CLOSED_STATE;
      break;
  }

  unlock_enable(slihpri, &SLIH_LOCK);

  /* Wait for transmit operations to finish */
  max_wait_cycles = 2 * TOK_XMIT_TIMEOUT;
  while ((WRK.adap_state == CLOSE_PENDING) && WRK.xmits_queued &&
           max_wait_cycles) {
    TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CLOw",(ulong)p_dds,(ulong)WRK.xmits_queued,
              (ulong)0);
    delay(HZ);
    max_wait_cycles--;
  }

  /* Stop the transmit timeout watchdog timer. */
  w_stop(&XMITWDT);

  /* Now free the waiting transmission elements. */
  while (WRK.xmits_queued) {
    GET_ARB_ELEM(cur_elem);

    /* Now free any waiting mbufs. */
    m_freem(cur_elem->mbufp);

    /* Adjust the linked wait list. */
    cur_elem->in_use = 0;
    ADD_2_FREE_Q(cur_elem);  /* Add the element to the free list. */

  }

  /* Close the SAP's, wait for the command to complete. */
  for (sap = 0; sap < 255; sap++) {
     if (WRK.sap_allocated[sap]) {
       rc = tok_issue_cmd(p_dds, SRB_CLOSE_SAP, TRUE, &sap);
       WRK.sap_allocated[sap] = 0;
     }
  }

  /* Check to see if a close operation is necessary. */
  if (WRK.adap_state == CLOSE_PENDING) {
    /* Remove the adapter from the ring. */
    rc |= tok_issue_cmd(p_dds, SRB_CLOSE, TRUE, NULL);
  }

  WRK.adap_state = CLOSED_STATE;
  NDD.ndd_flags &= ~(NDD_UP | NDD_RUNNING | NDD_LIMBO | NDD_ALTADDRS | \
                  TOK_RECEIVE_FUNC | TOK_RECEIVE_GROUP);

  simple_lock(&DD_LOCK);
  /* Last open for any adapter */
  if ((--dd_ctrl.num_opens) == 0) {
    /* Unregister for dump */
    dmp_del(((void (*) ())tok_cdt_func));
  }
  simple_unlock(&DD_LOCK);

  /* Remove the intr. handler. */
  i_clear((struct intr *)(&(IHS)));

  /* Stop/de-allocate the timers. */
  tok_remove_timers(p_dds);

#ifdef PCMCIATOK
  pcmcia_activity_lamp(p_dds, _CSaixTurnOffLED);

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CLOe",(ulong)WRK.adap_state,(ulong)0,(ulong)0);
#else
  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"CLOe",(ulong)0,(ulong)0,(ulong)0);
#endif
  return(0); /* MUST return 0. */
} /* end tokclose */

