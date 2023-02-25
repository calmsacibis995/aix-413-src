static char sccsid[] = "@(#)49	1.3  src/rspc/kernext/pci/kent/kent_close.c, pcient, rspc41J, 9519B_all 5/10/95 15:51:14";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: kent_close
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "kent_proto.h"
#include <sys/errno.h>
#include <sys/lock_alloc.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/sleep.h>
/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: kent_close()
 *                                                                    
 * FUNCTION: Close entry point from kernel.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have 
 *	not been disabled.
 *	
 *	This routine must always succeed.  By the time it is called all users
 *	should have called ns_dealloc, thus no further calls should occur
 *	to any other driver interfaces before this routine completes (although
 *	interrupts are still possible from the adapter).
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 */  

int
kent_close( ndd_t *p_ndd)
{
	kent_acs_t	*p_acs = (kent_acs_t *)p_ndd->ndd_correlator;
	int 		rc = 0;
	int		ipri;
	int		i;

	KENT_ETRACE("CfcB",p_acs,0,0);

	w_stop(&p_acs->tx.wdt);

	ipri = disable_lock(KENT_OPLEVEL, &p_acs->dev.slih_lock);

	/* 
	 * if the driver is in a power management mode of suspend/hibernate
	 * then put the caller to sleep until the driver resumes normal operation 
	 * except for open this needs to be done under a lock so that the state
	 * check is valid
	 */
	if (p_acs->dev.state == SUSPEND_STATE)
		e_sleep_thread (&p_acs->dev.pmh_event, 
			&p_acs->dev.slih_lock, LOCK_HANDLER);

	p_acs->dev.state = CLOSING_STATE;

	unlock_enable(ipri, &p_acs->dev.slih_lock);

	i = 0;
	while (p_acs->tx.in_use > 0)
	{
		ipri = disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);
		tx_complete(p_acs);
		unlock_enable(ipri, &p_acs->tx.lock);

		if (i++ == 10)
			break;

		DELAYMS(10);
	}

	ipri = disable_lock(KENT_OPLEVEL, &p_acs->dev.slih_lock);

	reset_adapter(p_acs);

	unlock_enable(ipri, &p_acs->dev.slih_lock);

	free_rx(p_acs);
	free_tx(p_acs);
	free_services(p_acs);
	bzero(p_acs->addrs.ladrf,KENT_LADRF_LEN);
	
	kent_cdt_del ((char *)p_acs);
	kent_cdt_del ((char *)p_acs->tx.desc);
	kent_cdt_del ((char *)p_acs->tx.p_buffer);
	kent_cdt_del ((char *)p_acs->rx.desc);
	kent_cdt_del ((char *)p_acs->rx.p_buffer);

	lock_write(&p_acs->dev.dd_clock);
	if (--kent_tbl.open_cnt == 0)
	{
		free_cdt();
	}
	lock_done(&p_acs->dev.dd_clock);

	p_acs->dev.state = CLOSED_STATE;
	p_acs->ndd.ndd_flags = NDD_BROADCAST|NDD_SIMPLEX;
	
	KENT_ETRACE("CfcE",0,0,0);

	unpincode(kent_config);

	return(0);
} /* end kent_close() */


