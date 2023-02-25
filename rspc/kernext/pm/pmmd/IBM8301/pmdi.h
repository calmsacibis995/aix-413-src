/* @(#)30	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pmdi.h, pwrsysx, rspc41J, 9515B_all 4/14/95 10:56:00  */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for PM kernel externsion machine dependent code 
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

#ifndef _H_PMDI
#define _H_PMDI

typedef struct	_pm_ODM_basic_data {    
	int     pmc_port1;
	int     pmc_port2;
	int     pmi_req;
	int     isa_bus_id;
	int     intr_level;
	int     intr_priority;
} pm_ODM_basic_data_t;

#endif	/* _H_PMDI */
