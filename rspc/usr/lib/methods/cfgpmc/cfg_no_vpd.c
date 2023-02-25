static char sccsid[] = "@(#)30  1.1  src/rspc/usr/lib/methods/cfgpmc/cfg_no_vpd.c, pwrsysx, rspc411, 9439A411a 9/26/94 04:47:27";
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
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
