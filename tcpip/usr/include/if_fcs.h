/* @(#)64	1.1  src/tcpip/usr/include/if_fcs.h, tcpip, tcpip411, GOLD410 11/8/93 12:51:46 */

/*
 * COMPONENT_NAME: TCPIP if_fcs.h
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 */

/* Local if_fcs.h - not shipped */

#ifndef _H_IF_FCS
#define _H_IF_FCS

#ifndef SCBNDD_ADDR_LEN
#define SCBNDD_ADDR_LEN		(12)    /* length of hardware address        */
#endif

struct fcs_mac_hdr {
        uchar   unit_num;       /* ex. port number on multi-port adap.  */
        uchar   entity_num;     /* source and destination entity        */
        ushort  opt_hdr_len;    /* total length of optional hdrs        */
        uchar   hw_addr[SCBNDD_ADDR_LEN];/* hw address (adap. specific) */
};

#ifndef IFT_FCS
#define IFT_FCS			0x1f		/* IP over FCS */
#endif
#ifndef ARPHRD_FCS
#define ARPHRD_FCS		12
#endif
#define FCS_IP_ENTITY_TYPE	0x05
#define FCS_MAC_SIZE		16
#define FCS_LLC_SIZE		8
#define FCS_ARP_PROTO_SIZE	4
#define FCS_ARP_TUPLE_SIZE	12

struct  fcs_arp {
        struct  arphdr fa_hdr;  /* fixed-size header */
};
#define arphrd fa_hdr.ar_hrd
#define arppro fa_hdr.ar_pro
#define arphln fa_hdr.ar_hln
#define arppln fa_hdr.ar_pln
#define arpop  fa_hdr.ar_op

#define FCS_ARP_HDR_SIZE 8

#endif /* _H_IF_FCS */
