static char sccsid[] = "@(#)58	1.4  src/rspc/kernext/pci/kent/kent_limbo.c, pcient, rspc41J, 9519B_all 5/10/95 15:52:55";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: bugout
 *		enter_limbo
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
#include <sys/mdio.h>
#include <sys/errno.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/dump.h>
/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: enter_limbo()
 *                                                                    
 * FUNCTION: attempts to recover from a transient adapter/network error
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the interrupt and process thread.  It assumes 
 *	interrupts have been disabled.
 *
 *	This routine assumes that the slih lock has already been obtained.
 *	
 * NOTES:
 *
 * RETURNS: 
 */  

void
enter_limbo( kent_acs_t *p_acs)
{
	int rc;

	if (p_acs->dev.state == DEAD_STATE)
		return;
	
	disable_lock(KENT_OPLEVEL, &p_acs->dev.ctl_lock);

	disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);

	w_stop(&p_acs->tx.wdt);

	rc = start_adapter(p_acs);

	unlock_enable(KENT_OPLEVEL, &p_acs->tx.lock);
	unlock_enable(KENT_OPLEVEL, &p_acs->dev.ctl_lock);
	
	if (rc || p_acs->dev.pio_rc)
	{
		bugout(p_acs);
	}
	return;
}

/*
 * NAME: bugout()
 *                                                                    
 * FUNCTION: disables the adapter and driver after a fatal error.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the interrupt and process thread.  It assumes 
 *	interrupts have been disabled.
 *
 *	This routine assumes that the slih lock has already been obtained.
 *	
 * NOTES:
 *
 * RETURNS: 
 */  

void
bugout( kent_acs_t *p_acs)
{
	if (p_acs->dev.state == DEAD_STATE)
		return;

	disable_lock(KENT_OPLEVEL, &p_acs->dev.ctl_lock);

	disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);

	w_stop(&p_acs->tx.wdt);
	
	p_acs->tx.in_use = 0;

	reset_adapter(p_acs);

	kent_logerr(p_acs, ERRID_KENT_DOWN, __LINE__, __FILE__, 0,0,0);

	p_acs->dev.state = DEAD_STATE;

	unlock_enable(KENT_OPLEVEL, &p_acs->tx.lock);
	unlock_enable(KENT_OPLEVEL, &p_acs->dev.ctl_lock);
	
	return;
}
