/* @(#)55       1.1  src/rspc/kernext/pci/kent/kent_dds.h, pcient, rspc41J, 9507B 1/25/95 11:26:22 */
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_KENT_DDS
#define _H_KENT_DDS

#include <sys/err_rec.h>
#include <sys/cdli_entuser.h>

/* -------------------------------------------------------------------- */
/* Define Device Structure                                		*/
/* -------------------------------------------------------------------- */

struct kent_dds
{
	uint 	busid;         	/* the bus id */
	uint 	busintr;	/* the interrupt level */
	uint 	busio;   	/* bus address for IO */
	uint	md_sla;		/* the location of the configuration space */
	uchar	lname[ERR_NAMESIZE];	/* device logical name (i.e. ent0) */
	uchar	alias[ERR_NAMESIZE];	/* alias to the device (i.e. en0) */

	uint	tx_que_sz;	/* the length of the adapter's tx queue */
	uint	rx_que_sz;	/* the length of the adapter's rx queue */
	uchar	use_alt_addr;	/* TRUE => use the following
					 * alt_addr otherwise use the address
					 * from the adapter.
					 */
	uchar	alt_addr[ENT_NADR_LENGTH];
};
typedef struct kent_dds kent_dds_t; 

#endif /* endif ! _H_KENT_DDS */
