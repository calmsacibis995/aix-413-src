/* @(#)80        1.1  src/rspc/kernext/isa/ide/ataidecfg.h, isaide, rspc41J, 9515A_all 4/11/95 12:02:18 */
/*
 *   COMPONENT_NAME: ISAIDE
 *
 *   FUNCTIONS: none
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

#ifndef _H_ATAIDECFG
#define _H_ATAIDECFG

/*
 * ddi structure for the IDE Adapter
 */

typedef	struct ide_adap_ddi {
	struct adap_ddi	adap;
	int	pm_dev_itime;	/* PM Device Idle time */
	int	pm_dev_stime;	/* PM Device Standby time */
} ide_adap_ddi_t;


#endif	/* _H_ATAIDECFG */
