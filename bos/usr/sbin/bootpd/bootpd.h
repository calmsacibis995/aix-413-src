/* @(#)17	1.9  src/bos/usr/sbin/bootpd/bootpd.h, cmdnet, bos411, 9428A410j 5/19/94 18:08:19  */
/*
** COMPONENT_NAME: (CMDNET) bootpd.h
**
** FUNCTIONS:
**
** ORIGINS: 6,44,27
**
**   (C) COPYRIGHT International Business Machines Corp. 1993
**   All Rights Reserved
**   Licensed Materials - Property of IBM
**   US Government Users Restricted Rights - Use, duplication or
**   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
 * bootpd.h -- common header file for all the modules of the bootpd program.
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


#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#ifndef SIGUSR1
#define SIGUSR1			 30	/* From 4.3 <signal.h> */
#endif

/* Number of htypes defined. This number must be incremented whenever a */
/* new hardware type is defined.					*/
#define MAXHTYPES		 18	 
#define MAXHADDRLEN		  6	/* Max hw address length in bytes */
#define MAXSTRINGLEN		 80	/* Max string length */

/* Hardware types from RFC 1390 - ASSIGNED NUMBERS */
/* If new hardware types are added to this list, MAXHTYPES must be */
/* incremented							   */

        #define HTYPE_ETHERNET            1    
        #define HTYPE_EXP_ETHERNET        2   
        #define HTYPE_AX25                3
        #define HTYPE_PRONET              4
        #define HTYPE_CHAOS               5
        #define HTYPE_IEEE802             6
        #define HTYPE_ARCNET              7
        #define HTYPE_HYPERCHANNEL        8
        #define HTYPE_LANSTAR             9
        #define HTYPE_AUTONET            10
        #define HTYPE_LOCALTALK          11
        #define HTYPE_LOCALNET           12
        #define HTYPE_ULTRA_LINK         13
        #define HTYPE_SMDS               14
        #define HTYPE_FRAME_RELAY        15
        #define HTYPE_ATM                16

/* Additional hardware types used by the bootp daemon (AIX only) */

        #define HTYPE_IEEE8023           17
        #define HTYPE_FDDI               18

/*
 * Return the length in bytes of a hardware address of the given type.
 */
#define haddrlength(type)	(maphaddrlen[(int) (type)])

/*
 * Return pointer to static string which gives full network error message.
 */
#define get_network_errmsg get_errmsg


/*
 * Variables shared among modules.
 */

extern int debug;
extern char *bootptab;
extern unsigned maphaddrlen[MAXHTYPES + 1];

extern hash_tbl *hwhashtable;
extern hash_tbl *iphashtable;
extern hash_tbl *nmhashtable;
extern unsigned char vm_cmu[4];
extern unsigned char vm_rfc1048[4];


/*
 * Functions shared among modules
 */

extern void report();
extern char *get_errmsg();
extern char *haddrtoa();
extern int readtab();



/*
 * Nice typedefs. . .
 */

typedef int boolean;
typedef unsigned char byte;


/*
 * Data structure used to hold an arbitrary-lengthed list of IP addresses.
 */

struct in_addr_list {
    unsigned		addrcount;
    struct in_addr	addr[1];		/* Dynamically extended */
};


/*
 * Data structures used to hold generic binary data.
 */

struct bindata {
    unsigned		length;
    byte		data[1];		/* Dynamically extended */
};


/*
 * Flag structure which indicates which symbols have been defined for a
 * given host.  This information is used to determine which data should or
 * should not be reported in the bootp packet vendor info field.
 */

struct flag {
    unsigned	bootfile	:1,
		bootserver	:1,
		bootsize	:1,
		bootsize_auto	:1,
		cookie_server	:1,
		domain_server	:1,
		gateway		:1,
		generic		:1,
		haddr		:1,
		homedir		:1,
		htype		:1,
		impress_server	:1,
		iaddr		:1,
		log_server	:1,
		lpr_server	:1,
		name_server	:1,
		name_switch	:1,
		rlp_server	:1,
		send_name	:1,
		subnet_mask	:1,
		time_offset	:1,
		timeoff_auto	:1,
		time_server	:1,
		vendor_magic	:1,
		vm_auto		:1,
		dumb_terminal	:1;
};



/*
 * The flags structure contains TRUE flags for all the fields which
 * are considered valid, regardless of whether they were explicitly
 * specified or indirectly inferred from another entry.
 *
 * The gateway and the various server fields all point to a shared list of
 * IP addresses.
 *
 * The hostname, home directory, and bootfile are all shared strings.
 *
 * The generic data field is a shared binary data structure.  It is used to
 * hold future RFC1048 vendor data until bootpd is updated to understand it.
 *
 * The vm_cookie field specifies the four-octet vendor magic cookie to use
 * if it is desired to always send the same response to a given host.
 *
 * Hopefully, the rest is self-explanatory.
 */

struct host {
    struct flag		    flags;		/* ALL valid fields */
    struct in_addr_list	    *cookie_server,
			    *domain_server,
			    *gateway,
			    *impress_server,
			    *log_server,
			    *lpr_server,
			    *name_server,
			    *rlp_server,
			    *time_server;
    char		    *hostname,
			    *homedir,
			    *bootfile;
    struct bindata          *generic;
    byte		    vm_cookie[4],
			    htype,  /* RFC826 says this should be 16-bits but
				       RFC951 only allocates 1 byte. . . */
			    haddr[MAXHADDRLEN];
    long		    time_offset;
    unsigned int	    bootsize;
    struct in_addr	    bootserver,
    			    iaddr,
			    subnet_mask;
    u_long		    delay_limit;
    short		    linkcount; /* since hp in multiple hash tables  */
};				       /* don't want to free more than once */


