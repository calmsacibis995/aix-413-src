static char sccsid[] = "@(#)76	1.1  src/tcpip/usr/sbin/gated/krt_ipmulticast.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:23";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: KRT_MC
 *		__PF1
 *		__PF2
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
 *  krt_ipmulticast.c,v 1.2 1993/04/10 12:25:07 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_IOCTL
#define	INCLUDE_ROUTE
#include "include.h"

#ifdef	IP_MULTICAST

#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"
#include "krt_var.h"

typedef struct _krt_multicast_entry {
    struct _krt_multicast_entry *kme_forw;
    struct _krt_multicast_entry *kme_back;
    flag_t kme_flags;
#define	KMEF_INSTALLED	0x01		/* We installed it in the kernel */
#define	KMEF_EXISTS	0x02		/* Not installed by us */
    flag_t kme_refcount;
    sockaddr_un *kme_group;
    sockaddr_un *kme_router;		/* Router it is installed with */
} krt_multicast_entry;

static krt_multicast_entry krt_multicast_entries = { &krt_multicast_entries, &krt_multicast_entries } ;

#define	KRT_MC(p)	{ for (p = krt_multicast_entries.kme_forw; p != & krt_multicast_entries; p = p->kme_forw)
#define	KRT_MC_END(p)	if (p == &krt_multicast_entries) p = (krt_multicast_entry *) 0; }


static const bits krt_multicast_bits[] = {
    { KMEF_INSTALLED,	"Installed" },
    { KMEF_EXISTS,	"Exists" },
    { 0,		NULL }
} ;


/**/

/* The Deering multicast mods for the kernel require the existence of */
/* a default route (0.0.0.0), a default multicast route (224.0.0.0) or */
/* a specific multicast route to determine the default interface. */
/* This information is ignored since we explicitly set our interface */
/* when sending the packets, but lack of this information would prevent */
/* us from sending to the multicast address.  To work around this we */
/* add our two multicast addresses to the routing table pointing at the */
/* loopback interface.  If these were ever actually used we would get */
/* an error because the loopback interface does not support multicast, */
/* but since we do specify the interface before sending packets this */
/* should *never* happen.  */
	
block_t krt_mc_block = 0;


static krt_multicast_entry *
krt_multicast_create __PF1(group, sockaddr_un *)
{
    krt_multicast_entry *kp;

    if (!krt_mc_block) {
	krt_mc_block = task_block_init(sizeof (*kp));
    }

    kp = (krt_multicast_entry *) task_block_alloc(krt_mc_block);
    insque((struct qelem *) kp,
	   (struct qelem *) krt_multicast_entries.kme_back);

    kp->kme_group = sockdup(group);

    return kp;
}


static void
krt_multicast_request __PF2(type, u_int,
			    kp, krt_multicast_entry *)
{
    krt_rt.rt_dest = kp->kme_group;
    krt_rt.rt_dest_mask = inet_mask_host;
    krt_rt.rt_n_gw = 1;
    krt_rt.rt_state = 0;
    krt_rt.rt_ifap = (if_addr *) 0;
    krt_rt.rt_gwp = gw_timestamp(&krt_gw_list,
				 RTPROTO_KERNEL,
				 krt_task,
				 (as_t) 0,
				 (as_t) 0,
				 RT_T_EXPIRE,
				 (sockaddr_un *) 0,
				 (flag_t) 0);
    
    switch (type) {
    case RTM_ADD:
	krt_rt.rt_router = inet_addr_loopback;
	krt_request(type,
		    &krt_rt,
		    krt_rt.rt_router);

	BIT_SET(kp->kme_flags, KMEF_INSTALLED);
	BIT_RESET(kp->kme_flags, KMEF_EXISTS);
	break;

    case RTM_DELETE:
	krt_rt.rt_router = kp->kme_router ? kp->kme_router : inet_addr_loopback;
	krt_request(type,
		    &krt_rt,
		    krt_rt.rt_router);

	BIT_RESET(kp->kme_flags, KMEF_INSTALLED|KMEF_EXISTS);
	break;

    default:
	assert(FALSE);
    }

    if (kp->kme_router) {
	sockfree(kp->kme_router);
	kp->kme_router = (sockaddr_un *) 0;
    }
}


static void
krt_multicast_free __PF1(kp, krt_multicast_entry *)
{
    if (BIT_TEST(kp->kme_flags, KMEF_INSTALLED)) {
	krt_multicast_request(RTM_DELETE, kp);
    }
		
    remque((struct qelem *) kp);

    sockfree(kp->kme_group);
    task_block_free(krt_mc_block, (void_t) kp);
}


void
krt_multicast_add __PF1(group, sockaddr_un *)
{
    krt_multicast_entry *kp;

    KRT_MC(kp) {
	if (sockaddrcmp(group, kp->kme_group)) {
	    break;
	}
    } KRT_MC_END(kp);

    if (kp) {
	/* Exists */

	if (BIT_TEST(kp->kme_flags, KMEF_EXISTS)) {
	    /* Already exists in kernel */

	    /* Delete and reinstall the way we want it */
	    krt_multicast_request(RTM_DELETE, kp);
	    krt_multicast_request(RTM_ADD, kp);
	}
    } else {
	/* Allocate a new one */
	kp = krt_multicast_create(group);

	/* Install it in kernel */
	krt_multicast_request(RTM_ADD, kp);
    }

    kp->kme_refcount++;
}


void
krt_multicast_delete __PF1(group, sockaddr_un *)
{
    krt_multicast_entry *kp;

    KRT_MC(kp) {
	if (sockaddrcmp(group, kp->kme_group)) {
	    /* Found it */

	    if (!--kp->kme_refcount) {
		/* Last reference, remove it */

		krt_multicast_free(kp);
	    }
	    break;
	}
    } KRT_MC_END(kp);
}


int
krt_multicast_install __PF2(group, sockaddr_un *,
			    router, sockaddr_un *)
{
    krt_multicast_entry *kp;

    KRT_MC(kp) {
	if (sockaddrcmp(group, kp->kme_group)) {
	    if (BIT_TEST(kp->kme_flags, KMEF_EXISTS)) {
		/* Delete duplicate entries */

		return TRUE;
	    }

	    kp->kme_router = sockdup(router);
	    if (kp->kme_refcount) {
		/* One of ours, fix the router */

		krt_multicast_request(RTM_DELETE, kp);
		krt_multicast_request(RTM_ADD, kp);
	    }

	    return FALSE;
	}
    } KRT_MC_END(kp);

    kp = krt_multicast_create(group);

    kp->kme_router = sockdup(router);
    BIT_SET(kp->kme_flags, KMEF_EXISTS);

    return FALSE;
}


#ifdef	RTM_CHANGE
void
krt_multicast_change __PF2(type, int,
			   rtp, rt_parms *)
{
    krt_multicast_entry *kp;
    
    KRT_MC(kp) {
	if (sockaddrcmp(rtp->rtp_dest, kp->kme_group)) {
	    break;
	}
    } KRT_MC_END(kp);

    switch (type) {
    case RTM_DELETE:
    case RTM_OLDDEL:
	if (kp->kme_refcount) {
	    /* We are using this, force it to be reinstalled */

	    krt_multicast_request(RTM_ADD, kp);
	} else {
	    /* We are not using it, release it */

	    krt_multicast_free(kp);
	}
	break;

    case RTM_ADD:
    case RTM_OLDADD:
	krt_multicast_install(rtp->rtp_dest, rtp->rtp_router);
	break;

    case RTM_CHANGE:
	if (kp) {
	    if (kp->kme_refcount > 1) {
		/* We are using this, don't let it change */
		
		krt_multicast_request(RTM_DELETE, kp);
		krt_multicast_request(RTM_ADD, kp);
	    }
	} else {
	    /* We don't yet know about it */

	    krt_multicast_install(rtp->rtp_dest, rtp->rtp_router);
	}
	break;

    default:
	/* Ignore */
	break;
    }
}
#endif	/* RTM_CHANGE */


void
krt_multicast_dump __PF2(tp, task *,
			 fd, FILE *)
{
    if (krt_multicast_entries.kme_forw != &krt_multicast_entries) {
	krt_multicast_entry *kp;
	
	(void) fprintf(fd, "\tIP Multicast default interfaces:\n");

	KRT_MC(kp) {
	    (void) fprintf(fd, "\t\t%-15A\trefcount %d  flags <%s>\n",
			   kp->kme_group,
			   kp->kme_refcount,
			   trace_bits(krt_multicast_bits, kp->kme_flags));
	} KRT_MC_END(kp) ;

	(void) fprintf(fd, "\n");
    }
}

#endif	/* IP_MULTICAST */
