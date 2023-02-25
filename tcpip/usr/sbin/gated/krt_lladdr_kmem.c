static char sccsid[] = "@(#)77	1.1  src/tcpip/usr/sbin/gated/krt_lladdr_kmem.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:25";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  krt_lladdr_kmem.c,v 1.1 1993/03/22 02:39:33 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

#define	INCLUDE_KVM
#define	INCLUDE_NLIST
#define	INCLUDE_CTYPE
#define	INCLUDE_ETHER
#include "include.h"
#include "krt.h"
#include "krt_var.h"


struct nlist *krt_ifnet;

/*
 *	Default krt_lladdr, poke around in the kernel
 */

#if	defined(AIX) && !defined(_POWER)
#define	ac_enaddr	ac_lanaddr
#endif	

/*
 * Obtain physical address of the interface by peeking in the kernel.
 * This *hack* assumes that the interface uses an arpcom structure where
 * the physical address resides immediately after the ifnet structure.
 */
sockaddr_un *
krt_lladdr __PF1(ifr, struct ifreq *)
{
    struct ifnet *ifp, *ifnet;
    int unit = 0;
    size_t namelen;

    if (!kd) {
	return (sockaddr_un *) 0;
    }
    
    /* Get length of name and fetch unit */
    {
	register char *sp = ifr->ifr_name;
	char *lp = ifr->ifr_name + IFNAMSIZ;

	while (isalpha(*sp)) {
	    sp++;
	}

	namelen = sp - ifr->ifr_name;

	do {
	    unit = (unit * 10) + (*sp - '0');
	} while (*++sp && sp < lp) ;
    }

    if (KVM_READ(kd,
		 krt_ifnet->n_value,
		 (caddr_t) &ifnet,
		 sizeof (ifnet)) < 0) {
	trace(TR_ALL, LOG_ERR, "krt_lladdr: reading ifnet for %.*s: %s",
	      IFNAMSIZ, ifr->ifr_name,
	      KVM_GETERR(kd, "kvm_read error"));
	return (sockaddr_un *) 0;
    }

    for (ifp = ifnet; ifp; ifp = ifp->if_next) {
	struct arpcom arpcom;
	char name[IFNAMSIZ];

	/* Read ifnet */
	if (KVM_READ(kd,
		     (off_t) ifp,
		     (caddr_t) &arpcom,
		     sizeof (arpcom)) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_lladdr: reading arpcom for %.*s: %s",
		  IFNAMSIZ, ifr->ifr_name,
		  KVM_GETERR(kd, "kvm_read error"));
	    break;
	}
	ifp = &arpcom.ac_if;

	/* And interface name */
	if (KVM_READ(kd,
		     (off_t) ifp->if_name,
		     (caddr_t) name,
		     sizeof name) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_lladdr: reading interface name for %.*s: %s",
		  IFNAMSIZ, ifr->ifr_name,
		  KVM_GETERR(kd, "kvm_read error"));
	    break;
	}
	ifp->if_name = name;	

	if (BIT_TEST(ifp->if_flags, IFF_BROADCAST)
#ifdef	IFT_OTHER
	    && krt_type_to_ll(ifp->if_type) == LL_8022
#endif	/* IFT_OTHER */
	    ) {
	    /* Assume broadcast nets have 802.2 addresses */

	    if (unit == ifp->if_unit
		&& !strncmp(ifp->if_name, ifr->ifr_name, namelen)) {
		/* This is the one we want */

		return sockbuild_ll(LL_8022,
				    (byte *) arpcom.ac_enaddr,
#ifdef	KRT_RT_SOCK
				    (size_t) ifp->if_addrlen
#else	/* KRT_RT_SOCK */
#ifndef	ETHER_ADDRLEN
#define	ETHER_ADDRLEN	6
#endif	/* ETHER_ADDRLEN */
				    ETHER_ADDRLEN
#endif	/* KRT_RT_SOCK */
				    );
	    }
	}
    }

    return (sockaddr_un *) 0;
}
