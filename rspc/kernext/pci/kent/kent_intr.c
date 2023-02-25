static char sccsid[] = "@(#)57	1.5  src/rspc/kernext/pci/kent/kent_intr.c, pcient, rspc41J, 9519B_all 5/10/95 15:52:40";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: kent_slih
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
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/dump.h>

/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: kent_slih()
 *                                                                    
 * FUNCTION: interrupt handler for the device driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the interrupt thread.
 *	
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 */  

int
kent_slih( kent_acs_t *p_acs)
{
	int	ipri;
	int	status;
	int	clear;
	int	iomem;
	int	rc = INTR_FAIL;

	KENT_TRACE("SksB",p_acs, 0,0);
	ipri = disable_lock (KENT_OPLEVEL, &(p_acs->dev.slih_lock));

	/*
	 * If the driver is in suspend state the adapter has been reset and
	 * this must not be our interrupt
	 */
	if ((p_acs->dev.state == SUSPEND_STATE) ||
	 	(p_acs->ndd.ndd_output == kent_dump_output))
	{
		unlock_enable(ipri, &(p_acs->dev.slih_lock));
		return(INTR_FAIL);
	}

	iomem = (int) iomem_att(&p_acs->dev.iomap);
	
	CSR_READ(iomem, 0, &status);

	while (status & KENT_CSR0_INTR)
	{
		rc = INTR_SUCC;
		if ((status & (KENT_CSR0_RINT|KENT_CSR0_MISS)))
		{
        		if (++NDDSTATS.ndd_recvintr_lsw == 0)
				NDDSTATS.ndd_recvintr_msw++;

			CSR_WRITE(iomem, 0, 
				(status & (KENT_CSR0_RINT|KENT_CSR0_MISS)));
	
			status = status & ~(KENT_CSR0_RINT|KENT_CSR0_MISS);

			rx_handler(p_acs);
		}
		if ((status & (KENT_CSR0_CERR)))
		{
			ENTSTATS.sqetest++;
		}

		if (!(status & KENT_CSR0_TXON) || !(status & KENT_CSR0_RXON))
		{
			enter_limbo(p_acs);
		}

		clear = (status & KENT_CSR0_MASK) | KENT_CSR0_IENA;
		CSR_WRITE(iomem, 0, clear);

		CSR_READ(iomem, 4, &status);

		if ((status & (KENT_CSR4_MFCO)))
		{
			ENTSTATS.no_resources += 0x10000;
		}

		CSR_WRITE(iomem, 4, (KENT_CSR4_TXSTRTM| KENT_CSR4_APAD_XMT | 
			KENT_CSR4_ASTRP_RCV | KENT_CSR4_JABM | 
			KENT_CSR4_DMAPLUS | status));

		CSR_READ(iomem, 0, &status);
	}

	iomem_det((void *)iomem);
	unlock_enable(ipri, &(p_acs->dev.slih_lock));
	KENT_TRACE("SksE",p_acs,0,0);
	return (rc);
}
