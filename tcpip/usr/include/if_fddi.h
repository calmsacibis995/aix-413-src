/* @(#)89	1.3  src/tcpip/usr/include/if_fddi.h, tcpip, tcpip411, GOLD410 9/15/93 13:17:48 */
/* 
 * COMPONENT_NAME: FDDI IF definitions
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/* if_fddi.h -	FDDI definitions - RFC 1103 */

#define	FCF_FDDI		0x50	/* FDDI			*/

#define FDDI_ADDRLEN 	6 
#define FDDI_FRAMECHARS 7     /*  starting delim(1) + FCS(4) + 
				  ED(1) + Frame status(1) */
#define TRCHKFDDI 1

#if 0
#define	RCF_ALL_BROADCAST	0x8000	/* all routes broadcast		*/
#define	RCF_LOCAL_BROADCAST	0x4000	/* single route broadcast	*/
#define	RCF_DIRECTION		0x0080	/* direction			*/
#define	RI_PRESENT		0x80	/* turn on bit 0 of byte 0 src addr */


#define	route_bytes(mac)	(((mac)->mac_rcf >> 8) & 0x1f)
#define	has_route(mac)		((mac)->mac_src[0] & RI_PRESENT)
#define	is_broadcast(mac)	(has_route(mac)		\
				 && ((mac)->mac_rcf & (RCF_LOCAL_BROADCAST \
							| RCF_ALL_BROADCAST)))
#endif

/*
 * FDDI MAC header
 */
struct fddi_mac_hdr {
	struct {
		u_char	reserved[3];	/* Reserved 3 bytes for adapter */
		u_char	Mac_fcf;	/* frame control field		*/
		u_char	Mac_dst[6];	/* destination address		*/
		u_char	Mac_src[6];	/* source address		*/
	} _First;
	struct {
		u_short Mac_rcf;	/* routing control field	*/
		u_short Mac_seg[14];	/* 30 routing segments 		*/
	} _Variable;
};

#if 0
#define	mac_fcf	_First.Mac_fcf
#define	mac_dst	_First.Mac_dst
#define	mac_src	_First.Mac_src
#define mac_rcf	_Variable.Mac_rcf
#define mac_seg	_Variable.Mac_seg
#endif
#define	mac_dst_fddi	_First.Mac_dst
#define	mac_src_fddi	_First.Mac_src

/*
 * mac_to_llc	-	given the ^ to the MAC, get to the LLC
 */
#define mac_to_llc(mac)	\
	(struct ie2_llc_snaphdr *) (((char *) (mac)) + mac_size(mac))

#if 0
#define	mac_size(mac)		\
	(sizeof ((mac)->_First) + (has_route((mac)) ? route_bytes((mac)) : 0))
#endif


#ifndef _802_2_LLC
#define	_802_2_LLC
/*
 * FDDI LLC header
 */
struct ie2_llc_hdr {
	unsigned char	dsap;		/* DSAP				*/
	unsigned char	ssap;		/* SSAP				*/
	unsigned char	ctrl;		/* control field		*/
	unsigned char	prot_id[3];	/* protocol id			*/
	unsigned short	type;		/* type field			*/
};
#endif

#define	DSAP_INET	0xaa		/* SNAP SSAP			*/
#define	SSAP_INET	0xaa		/* SNAP DSAP			*/
#define	SSAP_RESP	0x01		/* SSAP response bit		*/
#define	CTRL_UI		0x03		/* unnumbered info		*/
#define CTRL_XID	0xaf		/* eXchange IDentifier		*/
#define	CTRL_TEST	0xe3		/* test frame			*/

#define	_FDDI_TYPE_IP	0x0800		/* IP protocol */
#define _FDDI_TYPE_ARP	0x0806		/* Addr. resolution protocol */

/* CHANGE THIS TO 2K BEFORE SHIPPING - STEVOBEVODEVO */
#define	_FDDI_MTU	4352

/* defines used by if_fd.c */
#define FDDI_SETADDR	(1<<0)		/* physaddr has been set     */
#define fdadcpy(a,b)          bcopy(a, b, 6)

static  u_char  fd_brdcastaddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/*
 * FDDI Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	fddi_arp {
	struct	arphdr ea_hdr;	/* fixed-size header */
	u_char	arp_sha[6];	/* sender hardware address */
	u_char	arp_spa[4];	/* sender protocol address */
	u_char	arp_tha[6];	/* target hardware address */
	u_char	arp_tpa[4];	/* target protocol address */
};

#define	arp_hrd	ea_hdr.ar_hrd
#define	arp_pro	ea_hdr.ar_pro
#define	arp_hln	ea_hdr.ar_hln
#define	arp_pln	ea_hdr.ar_pln
#define	arp_op	ea_hdr.ar_op

/*
 * FDDI RAW socket addresses
 */
struct sockaddr_fddi {
	ushort			sa_family;	/* address family: AF_802_2 */
	struct fddi_mac_hdr	sa_mac;		/* MAC portion		    */
	struct ie2_llc_hdr	sa_llc;		/* LLC portion		    */
};

#ifdef	_KERNEL
extern u_char fddi_broadcastaddr[6];
struct	arptab *fddi_arptnew();
char *fddi_sprintf();
#endif
