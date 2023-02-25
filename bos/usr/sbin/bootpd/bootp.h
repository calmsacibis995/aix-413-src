/*
** @(#)15        1.4  src/bos/usr/sbin/bootpd/bootp.h, cmdnet, bos411, 9428A410j 9/10/91 11:35:25
**
** COMPONENT_NAME: (BOSBOOT) bootp.h
**
** FUNCTIONS:
**
** ORIGINS: 6,44
**
** (C) COPYRIGHT International Business Machines Corp. 1991
** All Rights Reserved
** Licensed Materials - Property of IBM
**
** Permission to use, copy, modify, and distribute this program for any
** purpose and without fee is hereby granted, provided that this copyright,
** permission and disclaimer notice appear on all copies and supporting
** documentation; the name of IBM not be used in advertising or publicity
** pertaining to distribution of the program without specific prior
** permission; and notice be given in supporting documentation that copying
** and distribution is by permission of IBM.  IBM makes no representations
** about the suitability of this software for any purpose.  This program
** is provided "as is" without any express or implied warranties, including,
** without limitation, the implied warranties of merchantability and fitness
** for a particular purpose.
**
** US Government Users Restricted Rights - Use, duplication or
** disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
**
** (C) COPYRIGHT Advanced Graphics Engineering 1990
** All Rights Reserved
**
*/
/*
 * Bootstrap Protocol (BOOTP).  RFC951 and RFC1048.
 *
 * $Header: /usr/xtra/xorro/R2/bootp/RCS/bootp.h,v 1.3 90/11/29 13:45:03 jhh Exp $
 *
 *
 * This file specifies the "implementation-independent" BOOTP protocol
 * information which is common to both client and server.
 *
 *
 * Copyright 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon and Stanford
 * University.  Carnegie Mellon makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */


struct bootp {
	unsigned char	bp_op;		/* packet opcode type */
	unsigned char	bp_htype;	/* hardware addr type */
	unsigned char	bp_hlen;	/* hardware addr length */
	unsigned char	bp_hops;	/* gateway hops */
	unsigned long	bp_xid;		/* transaction ID */
	unsigned short	bp_secs;	/* seconds since boot began */
	unsigned short	bp_unused;
	struct in_addr	bp_ciaddr;	/* client IP address */
	struct in_addr	bp_yiaddr;	/* 'your' IP address */
	struct in_addr	bp_siaddr;	/* server IP address */
	struct in_addr	bp_giaddr;	/* gateway IP address */
	unsigned char	bp_chaddr[16];	/* client hardware address */
	unsigned char	bp_sname[64];	/* server host name */
	unsigned char	bp_file[128];	/* boot file name */
	unsigned char	bp_vend[64];	/* vendor-specific area */
};

/*
 * UDP port numbers, server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

#define BOOTREPLY		2
#define BOOTREQUEST		1


/*
 * Vendor magic cookie (v_magic) for CMU
 */
#define VM_CMU		"CMU"

/*
 * Vendor magic cookie (v_magic) for RFC1048
 */
#define VM_RFC1048	{ 99, 130, 83, 99 }



/*
 * RFC1048 tag values used to specify what information is being supplied in
 * the vendor field of the packet.
 */

#define TAG_PAD			((unsigned char)   0)
#define TAG_SUBNET_MASK		((unsigned char)   1)
#define TAG_TIME_OFFSET		((unsigned char)   2)
#define TAG_GATEWAY		((unsigned char)   3)
#define TAG_TIME_SERVER		((unsigned char)   4)
#define TAG_NAME_SERVER		((unsigned char)   5)
#define TAG_DOMAIN_SERVER	((unsigned char)   6)
#define TAG_LOG_SERVER		((unsigned char)   7)
#define TAG_COOKIE_SERVER	((unsigned char)   8)
#define TAG_LPR_SERVER		((unsigned char)   9)
#define TAG_IMPRESS_SERVER	((unsigned char)  10)
#define TAG_RLP_SERVER		((unsigned char)  11)
#define TAG_HOSTNAME		((unsigned char)  12)
#define TAG_BOOTSIZE		((unsigned char)  13)
#define TAG_ALT_IP		((unsigned char) 171)		/*alt*/
#define TAG_ALT_SM		((unsigned char) 172)		/*alt*/
#define TAG_ALT_GW		((unsigned char) 173)		/*alt*/
#define TAG_PRIMARY		((unsigned char) 175)
#define TAG_XDMCP		((unsigned char) 178)
#define TAG_XDMCP_IP		((unsigned char) 179)
#define TAG_END			((unsigned char) 255)



/*
 * "vendor" data permitted for CMU bootp clients.
 */

struct cmu_vend {
	unsigned char	v_magic[4];	/* magic number */
	unsigned long	v_flags;	/* flags/opcodes, etc. */
	struct in_addr 	v_smask;	/* Subnet mask */
	struct in_addr 	v_dgate;	/* Default gateway */
	struct in_addr	v_dns1, v_dns2; /* Domain name servers */
	struct in_addr	v_ins1, v_ins2; /* IEN-116 name servers */
	struct in_addr	v_ts1, v_ts2;	/* Time servers */
	unsigned char	v_unused[25];	/* currently unused */
};


/* v_flags values */
#define VF_SMASK	1	/* Subnet mask field contains valid data */
