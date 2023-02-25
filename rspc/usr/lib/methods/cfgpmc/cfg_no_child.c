static char sccsid[] = "@(#)29  1.1  src/rspc/usr/lib/methods/cfgpmc/cfg_no_child.c, pwrsysx, rspc411, 9439A411a 9/26/94 04:46:44";
/*
 * COMPONENT_NAME: (PWRSYSX)
 *
 * FUNCTIONS: define_children
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

int
define_children(char *logical_name, int ipl_phase)
{
	/* This device has no child device. */
	return 0;
}
