static char sccsid[] = "@(#)20  1.10  src/bos/usr/sbin/install/inulib/inu_rm.c, cmdinstl, bos411, 9428A410j 10/27/93 13:27:22";
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS: inu_rm
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

/*
#include <stdio.h>
#include "../inudef.h"
#include <inu_toc.h>
*/

#include <inulib.h>

int inu_rm(char *filename)
{
	char com[PATH_MAX+12];
	int rc;

	sprintf(com, "/bin/rm -rf %s", filename);
	rc = inu_do_exec (com);
	return(rc);
}
