static char sccsid[] = "@(#)52	1.2  src/rspc/kernext/isa/itok/itok_debug.c, isatok, rspc411, 9438B411a 9/21/94 14:41:15";
/*
 * COMPONENT_NAME: ISATOK - IBM 16/4 Token-ring driver.
 *
 * FUNCTIONS: logerr(), tok_cdt_func(), tok_add_cdt(), tok_del_cdt(),
 *            tok_save_trace()
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
 *
 * NAME:                  logerr
 *
 * FUNCTION:              Collect information for making of error log entry.
 *
 * EXECUTION ENVIRONMENT: process and interrupt level
 *
 * NOTES:                 Error log entry made via errsave.
 *
 * RETURNS:               Nothing
 */
/*****************************************************************************/
void logerr( dds_t *p_dds,            /* Device driver structure ptr. */
             ulong  errid)            /* Errid to be logged.          */
{
  tok_error_log_data_t log;        /* Error log information.      */
  register int         i;          /* Simple loop counter.        */
  ulong                aix_errid;  /* Common tokenring error msg. */

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"logB",(ulong)p_dds,(ulong)errid,(ulong)0); 

  /* Initialize the log entry to zero */
  bzero(&log,sizeof(tok_error_log_data_t));

  /* Translate the error id to the common ones (also used by micro-channel).*/
  switch (errid) {
    case TOK_XMIT_TIMEOUT_ERROR:
      aix_errid = ERRID_ITOK_TX_TIMEOUT;
      break;
    case TOK_OPEN_FAILED:
      aix_errid = ERRID_ITOK_ADAP_OPEN;
      break;
    case TOK_BADPARAM:
      aix_errid = ERRID_ITOK_ADAP_OPEN;
      break;
    case TOK_REMOVE_RECEIVED:
      aix_errid = ERRID_ITOK_RMV_ADAP;
      break;
    case TOK_AUTO_REMOVAL:
      aix_errid = ERRID_ITOK_AUTO_RMV;
      break;
    case TOK_LOBE_WIRE_FAULT:
      aix_errid = ERRID_ITOK_WIRE_FAULT;
      break;
    case TOK_SIGNAL_LOSS:
      aix_errid = ERRID_ITOK_WIRE_FAULT;
      break;
    case TOK_TX_BEACON:
      aix_errid = ERRID_ITOK_BEACONING;
      break;
    case TOK_SOFT_ERROR:
      errid = ERRID_ITOK_ESERR;
      break;
    case TOK_HARD_ERROR:
      errid = ERRID_ITOK_PERM_HW;
      break;
  }

  /* Store the error id in the log entry */
  log.errhead.error_id = aix_errid;

  /* Load the device driver name into the log entry */
  strncpy(log.errhead.resource_name, DDI.lname,
    (size_t)sizeof(log.errhead.resource_name));

  /* Start filling in the log with data */
  log.adap_state  = WRK.adap_state;
  log.ring_errors = WRK.tok_network_errors;
  log.tx_cstat    = WRK.tx_stat;
  log.rcv_cstat   = WRK.rcv_stat;
  log.io_addr     = DDI.io_addr;

  /* Load Network address in use value into the table */
  for (i = 0; i < CTOK_NADR_LENGTH; i++) {
     log.tok_addr[i] = WRK.open_cfg.cur_tok_addr[i];
     log.hw_addr[i]  = dd_ctrl.adap_cfg[WRK.minor_no].aip.encoded_hw_addr[i];
  }

  /* Put the Token-Ring Information into Error log structure */
  bcopy(&(p_dds->wrk.tok_network_errors), &(log.ring_errors),
    sizeof(tok_error_log));

  /* log the error here */
  errsave(&log,sizeof(tok_error_log_data_t));

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"logE",(ulong)p_dds,(ulong)errid,
    (ulong)WRK.adap_state); 
   return;
}  /* end logerr */


/*****************************************************************************/
/*
 * NAME:                  tok_cdt_func
 *
 * FUNCTION:              Return the address of the cyclic debug table
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:                 
 *
 * RETURNS:               cyclic debug table address
 *
 */
/*****************************************************************************/
cdt_t *tok_cdt_func(void)
{
  return(&dd_ctrl.cdt);
}


/*****************************************************************************/
/*
 * NAME:                  tok_add_cdt
 *
 * FUNCTION:              add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_add_cdt (char *name,  /* label string for area dumped */
                  char *ptr,   /* area to be dumped */
                  int   len)   /* amount of data to be dumped */
{
  int              num_elems;      
  struct cdt_entry temp_entry;

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"ACDb",(ulong)name,(ulong)ptr,(ulong)len);

  strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
  temp_entry.d_len = len;
  temp_entry.d_ptr = ptr;
  temp_entry.d_xmemdp = NULL;

  num_elems = (dd_ctrl.cdt.header._cdt_len - sizeof(dd_ctrl.cdt.header)) /
               sizeof(struct cdt_entry);
  if (num_elems < MAX_CDT_ELEMS) {
    dd_ctrl.cdt.entry[num_elems] = temp_entry;
    dd_ctrl.cdt.header._cdt_len += sizeof(struct cdt_entry);
  }

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"ACDe",(ulong)num_elems,(ulong)0,(ulong)0);
  return;
} /* end tok_add_cdt */


/*****************************************************************************/
/*
 * NAME:                  tok_del_cdt
 *
 * FUNCTION:              delete an entry from the component dump table
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_del_cdt (char *name,  /* label string for area dumped */
                  char *ptr,   /* area to be dumped */
                  int   len)   /* amount of data to be dumped */
{
   int rc, ndx, num_elems;
   struct cdt_entry temp_entry;

   TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DCDb",(ulong)name,(ulong)ptr,(ulong)len); 

   strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
   temp_entry.d_len = len;
   temp_entry.d_ptr = ptr;
   temp_entry.d_xmemdp = NULL;

   TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DCD1",(ulong)name,(ulong)ptr,(ulong)len); 
   num_elems = (dd_ctrl.cdt.header._cdt_len - sizeof(dd_ctrl.cdt.header)) /
                  sizeof(struct cdt_entry);

   TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DCD2",(ulong)name,(ulong)num_elems,(ulong)len); 
   /* find the element in the array (match only the memory pointer) */
   for (ndx = 0; (ndx < num_elems) &&
        (temp_entry.d_ptr != dd_ctrl.cdt.entry[ndx].d_ptr); ndx++)
      ; /* NULL statement */

   TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DCD3",(ulong)name,(ulong)ptr,(ulong)len); 
   /* re-pack the array to remove the element if it is there */
   if (ndx < num_elems) {
      for (ndx++ ; ndx < num_elems; ndx++)
         dd_ctrl.cdt.entry[ndx-1] = dd_ctrl.cdt.entry[ndx];
      bzero (&dd_ctrl.cdt.entry[ndx-1], sizeof(struct cdt_entry));
      dd_ctrl.cdt.header._cdt_len -= sizeof(struct cdt_entry);
      rc = 0;
   }
   else { /* item not in table */
      rc = 1;
   }

   TRACE_DBG(HKWD_ITOK_ISA_OTHER,"DCDe",(ulong)(!rc),(ulong)num_elems,(ulong)0);
   return;
} /* end tok_del_cdt */


/*****************************************************************************/
/*
 * NAME:                  tok_save_trace
 *
 * FUNCTION:              enter trace data in the trace table and call AIX trace
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:                 Do not call directly -- use macros defined in
 *                        tokmisc.h so that tracing can be converted to 
 *                        a zero-run-time-cost option by simply re-defining
 *                        the macros.
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/
void tok_save_trace (ulong hook,          /* Trace hook.         */
                     char  *tag,          /* Description string. */
                     ulong arg1,          /* Argument no. 1.     */
                     ulong arg2,          /* Argument no. 2.     */
                     ulong arg3)          /* Argument no. 3.     */
{
  int i, tracepri;

  tracepri = disable_lock(PL_IMP, &TRACE_LOCK);

  /* Copy the trace point into the internal trace table */
  i = dd_ctrl.tracetable.nexthole;
  dd_ctrl.tracetable.table[i] = *(ulong *)tag;
  dd_ctrl.tracetable.table[i+1] = arg1;
  dd_ctrl.tracetable.table[i+2] = arg2;
  dd_ctrl.tracetable.table[i+3] = arg3;

  if ((i += 4) < TRACE_TABLE_SIZE) {
    dd_ctrl.tracetable.table[i] = TOK_END_TRACE_TABLE;
    dd_ctrl.tracetable.nexthole = i;
  } else {
      dd_ctrl.tracetable.table[0] = TOK_END_TRACE_TABLE;
      dd_ctrl.tracetable.nexthole = 0;
    }

  unlock_enable(tracepri, &TRACE_LOCK);

  /* Call the external trace routine */
  TRCHKGT(hook, *(ulong *)tag, arg1, arg2, arg3, 0);
} /* end tok_save_trace */

