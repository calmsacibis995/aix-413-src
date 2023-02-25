/* @(#)00	1.10  src/tcpip/usr/sbin/snmpd/interfaces.h, snmp, tcpip411, GOLD410 3/22/94 15:06:49 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/interfaces.h
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#ifndef _INTERFACES_H_
#define _INTERFACES_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include "snmpd.h"
#include "ddmibs.h"

#ifdef BSD44
#include <net/if_types.h>
#include <netiso/iso.h>		/* to get struct sockaddr_iso */
#endif

#ifdef _POWER
#  include <netinet/in_netarp.h>	/* to get struct arpcom */
#else	/* !_POWER */
#  if	defined(ps2) || defined (_AIX370)
#      include <sys/if_ieee802.h>	/* to get struct arpcom */
#      define ac_enaddr ac_lanaddr
#      define at_enaddr at_lanaddr
#  else	/* !ps2 */
#      include <netinet/if_ether.h>	/* to get struct arpcom */
#  endif
#endif

#if	defined(_POWER) || (defined(RT) && defined(AIX))
#define DYNAMIC_INTERFACES
#endif

/*  */

extern	int	ifNumber;

#if	defined(_POWER) && !defined(BSD44)
/* re-map BSD44 ifnet struct names into AIX3.1 names */
#define if_type		ifType
#define	if_ibytes	ifInOctets
#define if_imcasts	ifInNUcastPkts
#define if_iqdrops	ifInDiscards
#define if_noproto	ifInUnknownProto
#define if_obytes	ifOutOctets
#define	if_omcasts	ifOutNUcastPkts
#define if_lastchange	ifLastChange
#endif

struct interface {
    int	    ifn_index;			/* 1..ifNumber */
    int	    ifn_indexmask;		/* 1 << (index - 1) */

    int	    ifn_ready;			/* has an address associated with it */

    int ndd_or_not;
#define NOT_NDD_DRIVER	0
#define NDD_DRIVER	1

    ndd_t   *ndd_entry;			/* The ndd entry */
    caddr_t ndd_physaddr;
    caddr_t ndd_devspec;
    unsigned long ndd_offset;		/* where in kmem */

    struct arpcom *ifn_interface;	/* ifnet+physaddr */
    unsigned long ifn_offset;		/* where in kmem */

    char    ifn_name[IFNAMSIZ];		/* e.g., "en0" */
    char    ndd_name[IFNAMSIZ];		/* e.g., "en0" */
    char    ndd_alias[IFNAMSIZ];	/* e.g., "ent0" */
#ifdef _POWER
    char    *ifn_descr;			/* full description.  _POWER only */
#endif
    int     ifn_flags;

    int	    ifn_type;			/* ifType */

#ifndef BSD44				/* already in if_types.h */
#define IFT_OTHER       0x1             /* none of the following */
#define IFT_1822        0x2             /* old-style arpanet imp */
#define IFT_HDH1822     0x3             /* HDH arpanet imp */
#define IFT_X25DDN      0x4             /* x25 to imp */
#define IFT_X25         0x5             /* PDN X25 interface */
#define IFT_ETHER       0x6             /* Ethernet I or II */
#define IFT_ISO88023    0x7             /* CMSA CD */
#define IFT_ISO88024    0x8             /* Token Bus */
#define IFT_ISO88025    0x9             /* Token Ring */
#define IFT_ISO88026    0xa             /* MAN */
#define IFT_STARLAN     0xb
#define IFT_P10         0xc             /* Proteon 10MBit ring */
#define IFT_P80         0xd             /* Proteon 10MBit ring */
#define IFT_HY          0xe             /* Hyperchannel */
#define IFT_FDDI        0xf
#define IFT_LAPB        0x10
#define IFT_SDLC        0x11
#define IFT_T1          0x12
#define IFT_CEPT        0x13
#define IFT_ISDNBASIC   0x14
#define IFT_ISDNPRIMARY 0x15
#define IFT_PTPSERIAL   0x16
#define IFT_LOOP        0x18            /* loopback */
#define IFT_EON         0x19            /* ISO over IP */
#define IFT_XETHER      0x1a            /* obsolete 3MB experimental ethernet */
#define IFT_NSIP        0x1b            /* XNS over IP */
#define IFT_SLIP        0x1c            /* IP over generic TTY */
#endif /* BSD44 */

#define	TYPE_MIN	1
#define	TYPE_MAX	IFT_SLIP	/* 28 */

    u_long  ifn_speed;			/* ifSpeed */
    int	    ifn_admin;			/* ifAdminStatus */
#define	ADMIN_UP	1		/* ifAdminStatus */
#define	ADMIN_DOWN	2
#define	ADMIN_TEST	3

#define	ADMIN_MIN	1
#define	ADMIN_MAX	3

#define	OPER_UP		1		/* ifOperStatus */
#define	OPER_DOWN	2

    OID ifn_specific;                   /* ifSpecific */

    int flush_cache;            /* Used to determine to get cache      */
    int flush_rcv;              /* Used to determine to rcv cache      */
    unsigned int timeout;       /* Holds time remaining until timeout  */
    int last_q; 		/* Used to determine if values should  */
				/* be requested from the device driver */
    struct interface *time_next; /* Used to point at next timer to pop */

    int mib_envt;
#define ENVTNOTOK	0
#define ENVTOK		1
    combo_mibs_t *environ;
    combo_mibs_t *datacache;

    char *datarcvtable;
    char *setrequests;
    char *setpositions;
    int setsize;

    int fi_fd;                  /* File descriptor */
    struct interface *ifn_next;
};

extern struct interface *ifs;
extern struct interface *iftimers;

int	set_interface (), sort_interface ();

/*  */

union sockaddr_un {             /* 'cause sizeof (struct sockaddr_iso) == 32 */
    struct sockaddr         sa;

    struct sockaddr_in      un_in;
#ifdef  BSD44
    struct sockaddr_iso     un_iso;
#endif
};


struct address {
#ifdef _POWER 
#define      ADR_SIZE        MAX_HWADDR       /* defined in in_netarp.h */
#else  /* !_POWER */
#define ADR_SIZE     (20 + 1 + 1)             /* defined in interfaces.h */
#endif /* POWER */

    unsigned int    adr_instance[ADR_SIZE];
    int	    adr_insize;

    union sockaddr_un adr_address;	/* address */
    union sockaddr_un adr_broadaddr;	/*   broadcast, only if AF_INET */
    union sockaddr_un adr_netmask;	/*   network mask */

    int	    adr_indexmask;		/* mask of interfaces with address */

    struct address *adr_next;
};

extern struct address *afs_inet;

struct address *find_address (), *get_addrent ();
struct address *ifa_ifwithnet ();

#endif /* _INTERFACES_H_ */
