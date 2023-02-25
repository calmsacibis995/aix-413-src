/* "@(#)45  1.1  src/rspc/kernext/pci/stok/stok_dds.h, pcitok, rspc41J, 9513A_all 3/21/95 17:27:43" */
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#ifndef _H_STOK_DDS
#define _H_STOK_DDS

#include <sys/err_rec.h>
#include <sys/cdli_tokuser.h>

/* -------------------------------------------------------------------- */
/* Define Device Structure                                		*/
/* -------------------------------------------------------------------- */

struct stok_dds
{
	uint 	busid;         	/* the bus id */
	uint 	busintr;	/* the interrupt level */
	uint 	busio;   	/* bus address for IO */
	uint	md_sla;		/* the location of the configuration space */
	uchar	lname[ERR_NAMESIZE];	/* device logical name (i.e. tok0) */
	uchar	alias[ERR_NAMESIZE];	/* alias to the device (i.e. tr0) */

	uint	xmt_que_sz;	/* length of the software tx queue */
	uint	rx_que_sz;	/* length of the adapter's rx queue */
	uint	pri_xmt_que_sz;	/* length of the software priority tx queue */
   	uchar  	ring_speed;	/* Ring Speed: 0=4Mb,1=16Mb,2=autosence */
				/* if autosence is selected then adapter
				 * will set the ring speed according
				 * to the network ring speed.
				 */

	uchar	use_alt_addr;	/* TRUE => use the following
					 * alt_addr otherwise use the address
					 * from the adapter.
					 */
	uchar  alt_addr[CTOK_NADR_LENGTH];
	uchar  attn_mac;	/* yes, no value for attention frames   */
	uchar  beacon_mac;	/* yes, no value for beacon frames      */
	uchar  priority_tx;	/* yes, no value for priority Tx chnl   */
};
typedef struct stok_dds stok_dds_t; 

#endif /* endif ! _H_STOK_DDS */
