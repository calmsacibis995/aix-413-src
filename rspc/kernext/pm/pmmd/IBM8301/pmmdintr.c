static char sccsid[] = "@(#)33	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pmmdintr.c, pwrsysx, rspc41J, 9515B_all 4/14/95 10:56:06";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management Kernel Extension Code
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/pm.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/intr.h>

#include <pmmi.h>
#include "pm_sig750.h"


/*---------------------------------*/
/*	external functions 	   */
/*---------------------------------*/

/*---------------------------------*/
/*	external vars   	   */
/*---------------------------------*/
extern pm_md_data_t	pm_md_data;

/*---------------------------------*/
/*	global var declaration     */
/*---------------------------------*/

/*---------------------------------*/
/*	proto type declaration     */
/*---------------------------------*/
int  pm_intr(struct intr *p_intr);


/*
 * NAME:  pm_intr
 *
 * FUNCTION:  PMI interrupt handler
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the interrupt environment only.
 *      It can not page fault.
 *
 * NOTES:
 *	PMI (IRQ13) is shared with IDE.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      INTR_SUCC: Interrupt was from Signetics
 *      INTR_FAIL: Not our interrupt
 */
int
pm_intr(struct intr *p_intr)
{
	char	pmi_req;

	/*
	 *  Check if PMI from Signetics (bit 2 of port 0x838)
	 */
	pmi_req = (char) read_port(pm_md_data.pmdi.pmi_req);
	if (!(pmi_req & PMI_REQ)) {
		return INTR_FAIL;
	}
#ifdef PM_DEBUG
	printf("[button press interrupt]\n");
#endif /* PM_DEBUG */

	/*
	 *  Stop the failsafe timer (clear GATE_POWER_FAILSAFE)
	 */
	write_ctl_bit(GATE_PWR_FAILSAFE_BYTE, GATE_PWR_FAILSAFE_BIT, 0);

	/*
	 *  Reload the failsafe timer (set CLR_FAILSAFE)
	 */
	write_ctl_bit(CLR_FAILSAFE_BYTE, CLR_FAILSAFE_BIT, 1);

	/*
	 *  Call pmmi_event_receive_entry() to indicate button press
	 */
	if (pm_md_data.callback.pmmi_event_receive_entry != NULL) {
		(*(pm_md_data.callback.pmmi_event_receive_entry))
		    (PM_EVENT_POWER_SWITCH_OFF);
	}

	/*
	 *  Acknowledge interrupt to Signetics (set INTERRUPT_ACKNOWLEDGE)
	 */
	write_ctl_bit(INTERRUPT_ACK_BYTE, INTERRUPT_ACK_BIT, 1);

	/*
	 *  Clear PMI request (bit 2 of port 0x838)
	 */
	write_port(pm_md_data.pmdi.pmi_req, (pmi_req & ~PMI_REQ));
	return INTR_SUCC;
}
