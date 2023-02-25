static char sccsid[] = "@(#)43	1.2  src/rspc/usr/lib/methods/cfgpm8301/cfg_no_vpd.c, pwrsysx, rspc41J, 9515B_all 4/14/95 10:54:35";
/*
 * COMPONENT_NAME: (PWRSYSX)
 *
 * FUNCTIONS: query_vpd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cfgodm.h>

int query_vpd(struct CuDv *cudv, mid_t kmid, dev_t devno, char *vpd)
{
	/* This device has no VPD. */
	*vpd = '\0';
	return 0;
}
