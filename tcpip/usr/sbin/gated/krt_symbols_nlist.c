static char sccsid[] = "@(#)80	1.1  src/tcpip/usr/sbin/gated/krt_symbols_nlist.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:32";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
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
 *  krt_symbols_nlist.c,v 1.3 1993/04/04 16:28:13 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_NLIST
#define	INCLUDE_IOCTL
#define	INCLUDE_KVM
#define	INCLUDE_NETOPT_POWER
#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#include "krt.h"
#include "krt_var.h"


#ifdef	PROTO_INET
static struct nlist *krt_ipforwarding;
static struct nlist *krt_udpcksum;
#endif	/* PROTO_INET */

#ifndef	vax11c
static struct nlist *krt_version;
#else	/* vax11c */
static struct nlist *krt_multinet_version;
static struct nlist *krt_multinet_product_name;
#endif	/* vax11c */

static struct {
    const char *nl_name;
    struct nlist **nl_ptr;
} nl_names[] = {
#ifdef	KRT_RTREAD_KMEM
    {"_rthost", &krt_rthash[KRT_RTHOST]},
    {"_rtnet", &krt_rthash[KRT_RTNET]},
    {"_rthashsize", &krt_rthashsize},
#endif	/* KRT_RTREAD_KMEM */
#ifdef	KRT_RTREAD_RADIX
    {"_radix_node_head", &krt_radix_head},
#endif	/* KRT_RTREAD_RADIX */
#ifdef	KRT_LLADDR_KMEM
    {"_ifnet", &krt_ifnet},
#endif	/* KRT_LLADDR_KMEM */
#ifdef	PROTO_INET
    {KSYM_IPFORWARDING, &krt_ipforwarding},
    {KSYM_UDPCKSUM, &krt_udpcksum},
#endif	/* PROTO_INET */
#ifndef	vax11c
#ifndef	VRS_SYM
#define	VRS_SYM	"_version"
#endif	/* VRS_SYM */
    {VRS_SYM, &krt_version},
#else	/* vax11c */
    {"_multinet_version", &krt_multinet_version},
    {"_multinet_product_name", &krt_multinet_product_name},
#endif	/* vax11c */
    {NULL, NULL}
};

#define	NL_SIZE	(sizeof (nl_names)/sizeof (nl_names[0]))


int
krt_symbols __PF0(void)
{
    int i;
    static struct nlist nl[NL_SIZE];

    if (!kd) {
	return EBADF;
    }
    
    for (i = NL_SIZE; i--;) {
	register struct nlist *nlp = &nl[i];

	/* Use bcopy to avoid warning about const char * */
	bcopy((caddr_t) &nl_names[i].nl_name, (caddr_t) &nlp->n_name, sizeof(char *));
#ifdef	NLIST_NOUNDER
	if (nlp->n_name) {
	    nlp->n_name++;
	}
#endif	/* NLIST_NOUNDER */
	if (nl_names[i].nl_ptr) {
	    *nl_names[i].nl_ptr = nlp;
	}
    }

#ifdef	_POWER
    knlist(nl, NL_SIZE, sizeof (struct nlist));
#else	/* _POWER */
    if (KVM_NLIST(kd, nl) < 0) {
	trace(TR_ALL, LOG_ERR, "krt_symbols: %s",
	      KVM_GETERR(kd, "kvm_nlist error"));

	return EINVAL;
    }
#endif	/* _POWER */
#ifdef	_AUX_SOURCE
    /*  There is a bug in the Apple A/UX 2.01 nlist function.  It will	*/
    /*  return a value of zero for any symbol that is in the bss region.*/
    /*  It does work correctly for symbols which are in the data region.*/
    /*  This function opens a pipe to the nm(1) command, which works	*/
    /*  correctly, although more slowly, for symbols in the bss region.	*/
    /*  Herb Weiner <herbw@wiskit.rain.com>				*/
    for (i = NL_SIZE; i--;) {
	register struct nlist *nlp = &nl[i];
	
	if (nlp->n_type && !nlp->n_value) {
	    char command [256];
	    char buffer [256];
	    char *bufp;
	    FILE *symbol_pipe;
 
	    sprintf (command, "/bin/nm -d %s | /bin/grep %s",
		     UNIX_NAME,
		     nlp->nl_name);
	    NON_INTR(symbol_pipe, pope (command, "r"));
 
	    fgets (buffer, sizeof (buffer), symbol_pipe);
	    bufp = strchr (buffer, '|');
	    if (bufp) {
		sscanf (bufp, "|%ld|", &nlp->nl_value);
	    }
    
	    pclose (symbol_pipe);
	}
    }
#endif	/* _AUX_SOURCE */

#ifndef vax11c
    if (krt_version->n_value) {
	char *p;

	krt_version_kernel = (char *) task_block_alloc(task_block_pagesize);
	if (KVM_READ(kd,
		     (off_t) krt_version->n_value,
		     krt_version_kernel,
		     task_pagesize - 1) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_symbols: reading kernel version: %s",
		  KVM_GETERR(kd, "kvm_read error"));
	    return EINVAL;
	}
	if (p = (char *) index(krt_version_kernel, '\n')) {
	    *p = (char) 0;
	}
	p = task_mem_strdup(krt_task, krt_version_kernel);
	task_block_free(task_block_pagesize, krt_version_kernel);
	krt_version_kernel = p;
	trace(TR_KRT, 0, NULL);
	trace(TR_KRT, 0, "krt_symbols: %s = %s",
	      krt_version->n_name,
	      krt_version_kernel);
    }
#else	/* vax11c */
    if (krt_multinet_version->n_value && krt_multinet_product_name->n_value) {
	char *p;

	krt_version_kernel = task_block_alloc(task_block_pagesize);
	if (KVM_READ(kd,
		     krt_multinet_product_name->n_value,
		     krt_version_kernel,
		     task_pagesize - 2) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_symbols: reading multinet product name: %s",
		  KVM_GETERR(kd, "kvm_read error"));
	    return EINVAL;
	}
	(void) strcat(krt_version_kernel, " ");
	if (KVM_READ(kd,
		     krt_multinet_version->n_value,
		     krt_version_kernel + strlen(krt_version_kernel),
		     task_pagesize - 1 - strlen(krt_version_kernel)) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_symbols: reading multinet version: %s",
		  KVM_GETERR(kd, "kvm_read error"));
	    return EINVAL;
	}
	p = task_mem_strdup(krt_task, krt_version_kernel);
	task_block_free(task_block_pagesize, krt_version_kernel);
	krt_version_kernel = p;
	trace(TR_KRT, 0, "krt_symbols: %s %s = %s",
	      krt_multinet_product_name->n_name,
	      krt_multinet_version->n_name,
	      krt_version_kernel);
    }
#endif	/* vax11c */

#ifdef	PROTO_INET
    if (krt_ipforwarding->n_value) {
	if (KVM_READ(kd,
		     (off_t) krt_ipforwarding->n_value,
		     (caddr_t) &inet_ipforwarding,
		     sizeof(inet_ipforwarding)) < 0) {
	    trace(TR_ALL, LOG_INFO, "krt_symbols: reading IP forwarding enable flag: %s",
		  KVM_GETERR(kd, "kvm_read error"));
	}
    }
    if (krt_udpcksum->n_value) {
	if (KVM_READ(kd,
		     (off_t) krt_udpcksum->n_value,
		     (caddr_t) &inet_udpcksum,
		     sizeof(inet_udpcksum)) < 0) {
	    trace(TR_ALL, LOG_INFO, "krt_symbols: reading UDP checksum enable flag: %s",
		  KVM_GETERR(kd, "kvm_read error"));
	}
    }
#endif	/* PROTO_INET */

    return 0;
}


#ifdef	SIOCGNETOPT
#ifndef	MAXOPTLEN
#define	MAXOPTLEN	128

struct optreq {
    char name[MAXOPTLEN];
    char data[MAXOPTLEN];
    char getnext;
} ;

#endif	/* MAXOPTLEN */


static struct optreq net_opts[] = {
#define	NOPTS_IPFORWARDING	0
    { "ipforwarding" },
#ifndef _POWER
#define	NOPTS_COMPAT43		1
    { "compat_43" },
#endif /* _POWER */
    { "" }
} ;


int
krt_netopts __PF0(void)
{
    int rc, s;
    struct optreq *op;

    NON_INTR(s, socket(AF_UNIX, SOCK_STREAM, 0));
    if (s < 0) {
	int error = errno;

	trace(TR_ALL, LOG_WARNING, "krt_netopts: socket(AF_UNIX, SOCK_STREAM, 0): %m");

	return error;
    }
    
    for (op = net_opts; *op->name; op++) {
	op->getnext = 0;
	
	if (task_ioctl(s, SIOCGNETOPT, (caddr_t) op, sizeof (*op)) < 0) {
	    trace(TR_ALL, LOG_WARNING, "krt_netopts: ioctl(SIOCGNETOPT, %s): %m",
		  op->name,
		  errno);
	    continue;
	}

	switch (op - net_opts) {
	case NOPTS_IPFORWARDING:
#ifdef	PROTO_INET
	    inet_ipforwarding = atoi(op->data);
#endif	/* PROTO_INET */
	    break;

#ifndef _POWER
	case NOPTS_COMPAT43:
	    if (atoi(op->data)) {
		trace(TR_ALL, 0, "krt_netops: WARNING 4.3 compatibility = %s!",
		      op->data);
	    }
	    break;
#endif /* _POWER */
	    
	default:
	    assert(FALSE);
	}
    }

    NON_INTR(rc, close(s));
    if (rc < 0) {
	trace(TR_ALL, LOG_WARNING, "krt_netopts: close(socket): %m");
    }
    
    return 0;
}
#endif	/* SIOCGNETOPT */
