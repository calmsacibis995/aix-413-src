static char sccsid[] = "@(#)45  1.1  src/bos/usr/samples/ndd/entsamp_util.c, entsamp, bos411, 9428A410j 1/14/94 13:40:45";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_pio_retry
 *		entsamp_cdt_func
 *		entsamp_cdt_add
 *		entsamp_cdt_del
 *		entsamp_trace
 *		entsamp_logerr
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
 */

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>

#include "entsamp_mac.h"
#include "entsamp_pio.h"
#include "entsamp.h"
#include "entsamp_errids.h"


extern entsamp_dd_ctl_t	entsamp_dd_ctl;

void entsamp_trace();
void entsamp_logerr();

/*****************************************************************************/
/*
 * NAME:     entsamp_pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an exception.
 *		Retry the pio function and do error logging.
 *
 * EXECUTION ENVIRONMENT: process and interrupt.
 *
 * NOTES:
 *	This routine is called by using macros defined in entsamp_pio.h
 *
 * CALLED FROM:
 *      
 * INPUT:
 *      p_dev_ctl       - device control structure.
 *      excpt_code      - exception code from first pio 
 *      iofunc          - io function to retry
 *      ioaddr          - io address of the exception
 *      ioparam         - parameter to PIO routine
 *      cnt             - for string copies
 *
 * RETURNS:  
 *	0 - retry successfully
 * 	exception code
 */
/*****************************************************************************/
entsamp_pio_retry (
    entsamp_dev_ctl_t	*p_dev_ctl,	/* adapter control structure        */
    int 		excpt_code,	/* exception code from original PIO */
    enum pio_func 	iofunc,		/* io function to retry	            */
    int 		ioaddr,		/* io address of the exception	    */
    long		ioparam,	/* parameter to PIO routine	    */
    int			cnt)		/* for string copies                */

{
    int			retry_count = PIO_RETRY_COUNT;
    int			ipri;
    ndd_statblk_t	stat_blk;   	/* Status block                      */
    
    
    TRACE_SYS(HKWD_ENTSAMP_OTHER, "NioB", (ulong)p_dev_ctl, excpt_code, 
	(ulong)iofunc);
    TRACE_SYS(HKWD_ENTSAMP_OTHER, "NioC", (ulong)ioaddr, ioparam, 0);

    while (TRUE) {
        /* 
         * check if out of retries
         */
        if (retry_count <= 0) {
          /* 
           * Log pio error and send a status block to user
           */
	  entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_PIOFAIL, __LINE__,
		__FILE__, excpt_code, 0, 0); 
          if (p_dev_ctl->device_state == OPENED) { 
		bzero(&stat_blk, sizeof(ndd_statblk_t));
		stat_blk.code = NDD_HARD_FAIL;
		stat_blk.option[0] = NDD_PIO_FAIL;
		(*(NDD.nd_status))(&(NDD), &stat_blk);
          }
          break;
        }

        retry_count--;
	
        /* 
         * retry the pio function, return if successful
         */
        switch (iofunc) {

	  case PUTC:
	    	excpt_code = BUS_PUTCX((char *)ioaddr, (char)ioparam);
	    	break;

	  case PUTS:
	    	excpt_code = BUS_PUTSX((short *)ioaddr, (short)ioparam);
	    	break;

	  case PUTL:
	    	excpt_code = BUS_PUTLX((long *)ioaddr, (long)ioparam);
	    	break;

	  case GETC:
	    	excpt_code = BUS_GETCX((char *)ioaddr, (char *)ioparam);
	    	break;

	  case GETS:
	    	excpt_code = BUS_GETSX((short *)ioaddr, (short *)ioparam);
	    	break;

	  case GETL:
	    	excpt_code = BUS_GETLX((long *)ioaddr, (long *)ioparam);
	    	break;

	  case PUTSTR:
	    	excpt_code = BUS_PUTSTRX((long *)ioaddr, (long *)ioparam, cnt);
	    	break;

	  case GETSTR:
	    	excpt_code = BUS_GETSTRX((long *)ioaddr, (long *)ioparam, cnt);
	    	break;

	  default:
	    	ASSERT(0);
        } 
	
        if (excpt_code == 0) 
          break;
	
    } 
    
    TRACE_SYS(HKWD_ENTSAMP_OTHER, "NioE", (ulong)excpt_code, 0, 0);
    return (excpt_code);
    
} 
/*****************************************************************************/
/*
 * NAME:     entsamp_cdt_func
 *
 * FUNCTION: return the address of the component dump table 
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * INPUT:
 *	none.
 *
 * RETURNS:  
 *	the address of the component dump table
 */
/*****************************************************************************/
struct cdt *entsamp_cdt_func()
{

   return(&entsamp_dd_ctl.cdt.head);

}

/*****************************************************************************/
/*
 * NAME:     entsamp_cdt_add
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	entsamp_open
 *
 * INPUT:
 *	name		- character string of the name of the data structure
 *	addr		- address of the data structure
 *	len		- length of the data structure
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
entsamp_cdt_add(
  char *name,  	/* label string for area dumped */
  char *addr,   	/* address of the area to be dumped */
  int   len)   	/* amount of data to be dumped */

{
  struct cdt_entry *p_entry;


  TRACE_SYS(HKWD_ENTSAMP_OTHER, "OdaB", (ulong)name, (ulong)addr, len);
   
  p_entry = &entsamp_dd_ctl.cdt.entry[entsamp_dd_ctl.cdt.count];
  strcpy(p_entry->d_name, name);
  p_entry->d_len = len;
  p_entry->d_ptr = addr;
  p_entry->d_xmemdp = NULL;

  entsamp_dd_ctl.cdt.count++;
  entsamp_dd_ctl.cdt.head._cdt_len += sizeof(struct cdt_entry);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "OdaE", 0, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     entsamp_cdt_del
 *
 * FUNCTION: delete an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	entsamp_open
 *	entsamp_close
 *
 * INPUT:
 *	name		- character string of the name of the data structure
 *	addr		- address of the data structure
 *	len		- length of the data structure
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
entsamp_cdt_del(
  char *name,  	/* label string for area dumped */
  char *addr,   	/* address of the area to be dumped */
  int   len)   	/* amount of data to be dumped */

{
  struct cdt_entry *p_entry;
  int i;


   
  TRACE_SYS(HKWD_ENTSAMP_OTHER, "OddB", (ulong)name, (ulong)addr, len);

  /* find the entry in the table (match only the memory pointer) */
  for (p_entry = &entsamp_dd_ctl.cdt.entry[0], i = 0;
        i < entsamp_dd_ctl.cdt.count; p_entry++, i++) {
	if (p_entry->d_ptr == addr) 
		break;
  }

  /* if found the entry, remove the entry by re-arrange the table */
  if (i < entsamp_dd_ctl.cdt.count) {
      	for (; i < entsamp_dd_ctl.cdt.count; p_entry++, i++) {
		strcpy(p_entry->d_name, entsamp_dd_ctl.cdt.entry[i+1].d_name);
		p_entry->d_len = entsamp_dd_ctl.cdt.entry[i+1].d_len;
		p_entry->d_ptr = entsamp_dd_ctl.cdt.entry[i+1].d_ptr;
	}
	entsamp_dd_ctl.cdt.count--;
	entsamp_dd_ctl.cdt.head._cdt_len -= sizeof(struct cdt_entry);

  }
  TRACE_SYS(HKWD_ENTSAMP_OTHER, "OddE", 0, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     entsamp_trace
 *
 * FUNCTION: Put a trace into the internal trace table and the external
 *	     system trace.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *	This routine is only called through macros when DEBUG is defined.
 *	
 * CALLED FROM:
 *	every routine in the driver 
 *
 * INPUT:
 *	hook		- trace hook 
 *	tag		- four letter trace ID
 *	arg1 to arg 3   - arguments to trace
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
entsamp_trace(
   ulong	hook,	/* trace hook */
   char		*tag,	/* trace ID */
   ulong	arg1,	/* 1st argument */
   ulong	arg2,	/* 2nd argument */
   ulong	arg3)	/* 3rd argument */

{	 

	int i;
	int ipri;

	ipri = disable_lock(PL_IMP, &TRACE_LOCK);

	/*
	 * Copy the trace point into the internal trace table
	 */
	i = entsamp_dd_ctl.trace.next_entry;
	entsamp_dd_ctl.trace.table[i] = *(ulong *)tag;
	entsamp_dd_ctl.trace.table[i+1] = arg1;
	entsamp_dd_ctl.trace.table[i+2] = arg2;
	entsamp_dd_ctl.trace.table[i+3] = arg3;
	
	if ((i += 4) < ENTSAMP_TRACE_SIZE) {
		entsamp_dd_ctl.trace.table[i] = ENTSAMP_TRACE_END;
		entsamp_dd_ctl.trace.next_entry = i;
	}
	else {
		entsamp_dd_ctl.trace.table[0] = ENTSAMP_TRACE_END;
		entsamp_dd_ctl.trace.next_entry = 0;
	}
 
	unlock_enable(ipri, &TRACE_LOCK);

	/*
	 * Call the external trace routine
	 */
	TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);
	

}

/*****************************************************************************/
/*
 * NAME:     entsamp_logerr
 *
 * FUNCTION: Collect information for making system error log entry.
 *
 * EXECUTION ENVIRONMENT: process or interrupt 
 *
 * NOTES:
 *	
 * CALLED FROM:
 *	entsamp_setup
 *	entsamp_tx_setup
 *	entsamp_rv_setup
 *	entsamp_tx_free
 *	entsamp_rv_free
 *	entsamp_start
 *	entsamp_intr
 *	entsamp_rv_intr
 *	entsamp_tx_intr
 *	entsamp_tx_timeout
 *	entsamp_ctl_timeout
 *	entsamp_hard_fail
 *
 * INPUT:
 *	p_dev_ctl	- pointer to device control area
 *	errid		- error id for logging 
 *	line		- code line number
 *	fname		- code file name
 *	parm1		- log 4 bytes data 1
 *	parm2		- log 4 bytes data 2
 *	parm3		- log 4 bytes data 3
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
entsamp_logerr(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* pointer to dev_ctl area */
  ulong  errid,              /* Error id for logging */
  int    line,		     /* line number */
  char   *p_fname,	     /* file name */
  ulong  parm1,              /* log 4 bytes data 1 */
  ulong  parm2,              /* log 4 bytes data 2 */
  ulong  parm3)              /* log 4 bytes data 3 */

{
	struct  error_log_def   log;
	uchar   lbuf[64];
	int i;


  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "NlgB", (ulong)p_dev_ctl, (ulong)errid, 
		line);
  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "NlgC", parm1, parm2, parm3);

   	/* Load the error id and device driver name into the log entry */
   	log.errhead.error_id = errid;
        strncpy(log.errhead.resource_name, DDS.lname, ERR_NAMESIZE);

        /* put the line number and filename in the table */
        sprintf(lbuf, "line: %d file: %s", line, p_fname);
        strncpy(log.fname, lbuf, sizeof(log.fname));

   	/* Load POS register value in the table   */
   	for (i = 0; i < NUM_POS_REG; i++)
      		log.pos_reg[i] = WRK.pos_reg[i];

   	/* Load Network address in use value into the table  */
   	for (i = 0; i < ENT_NADR_LENGTH; i++) 
      		log.ent_addr[i] = WRK.net_addr[i];

   	/* Start filling in the table with data  */
   	log.parm1 = parm1;
   	log.parm2 = parm2;
   	log.parm3 = parm3;

   	/* log the error message */
   	errsave(&log, sizeof(struct error_log_def));

  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "NlgE", 0, 0, 0);

}  

