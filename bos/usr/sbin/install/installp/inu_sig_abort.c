static char sccsid[] = "@(#)08  1.7  src/bos/usr/sbin/install/installp/inu_sig_abort.c, cmdinstl, bos411, 9428A410j 4/20/94 15:47:36";
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS: inu_sig_abort
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <installp.h>
#include <inuerr.h>
#include <stdlib.h>

/*
 * NAME: inu_sig_abort
 *
 * FUNCTION: cleans up from user interrupt.
 *
 * NOTES: Global variable "inu_abort_sev" indicates what level of cleanup
 *	  must be performed
 *
 * RETURN VALUE DESCRIPTION: NONE
 */
void inu_sig_abort(
int rc)		/* return code to shell */
{
/* Begin inu_sig_abort */

	rc = INUABORT;
/*
	if (INU_SIG_ABORT == 2)
		if(inu_cleanup_opts() != INUGOOD)
			rc = INUUNREC;
*/

	inu_quit(rc);

} /* End inu_sig_abort */
