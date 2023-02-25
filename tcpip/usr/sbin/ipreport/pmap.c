static char sccsid[] = "@(#)94	1.1  src/tcpip/usr/sbin/ipreport/pmap.c, tcpras, tcpip411, GOLD410 10/11/91 13:33:50";
/*
 * COMPONENT_NAME: TCPIP pmap.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 24 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 *
 * Implements the program,version to port number mapping for rpc.
 *
 *
 */

#include <rpc/rpc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/pmap_prot.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "ipr.h"

/*
 * Stuff for the rmtcall service
 */
#define ARGSIZE 9000

typedef struct encap_parms {
	u_long arglen;
	char *args;
};

bool_t
xdr_encap_parms(xdrs, epp)
	XDR *xdrs;
	struct encap_parms *epp;
{

	return (xdr_bytes(xdrs, &(epp->args), &(epp->arglen), ARGSIZE));
}

bool_t
prt_encap_parms(epp)
	struct encap_parms *epp;
{
	reset_beg_line(pm_beg);
	printf("%s",beg_line);

	printf("Parms:\n");

	return (hex_dump(epp->args, epp->arglen));
}

typedef struct rmtcallargs {
	u_long	rmt_prog;
	u_long	rmt_vers;
	u_long	rmt_port;
	u_long	rmt_proc;
	struct encap_parms rmt_args;
};

bool_t
xdr_rmtcall_args(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcallargs *cap;
{

	/* does not get a port number */
	if (xdr_u_long(xdrs, &(cap->rmt_prog)) &&
	    xdr_u_long(xdrs, &(cap->rmt_vers)) &&
	    xdr_u_long(xdrs, &(cap->rmt_proc))) {
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	}
	return (FALSE);
}

bool_t
prt_rmtcall_args(cap)
	register struct rmtcallargs *cap;
{
	reset_beg_line(pm_beg);
	printf("%s",beg_line);

	printf("Prog=%d Vers=%d Proc=%d\n", cap->rmt_prog, 
					    cap->rmt_vers,
					    cap->rmt_proc);

	prt_encap_parms(&cap->rmt_args);
	return (TRUE);
}

bool_t
xdr_rmtcall_result(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcallargs *cap;
{
	if (xdr_u_long(xdrs, &(cap->rmt_port)))
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	return (FALSE);
}

bool_t
prt_rmtcall_result(cap)
	register struct rmtcallargs *cap;
{
	reset_beg_line(pm_beg);
	printf("%s",beg_line);

	printf("Port=%d\n",cap->rmt_port);
	prt_encap_parms(&(cap->rmt_args));
	return (TRUE);
}


bool_t
prt_long(lng)
	int *lng;
{
	reset_beg_line(pm_beg);
	printf("%s",beg_line);

	printf("Returning %d\n",*lng);
	return (TRUE);
}

bool_t
prt_pmap(regs)
	struct pmap *regs;
{

	reset_beg_line(pm_beg);
	printf("%s",beg_line);

	printf("Prog=%d Vers=%d Prot=%d Port=%d\n",
			regs->pm_prog, regs->pm_vers,
			regs->pm_prot, regs->pm_port);
	return (TRUE);
}

bool_t
xdr_pmap(xdrs, regs)
	XDR *xdrs;
	struct pmap *regs;
{

	if (xdr_u_long(xdrs, &regs->pm_prog) && 
		xdr_u_long(xdrs, &regs->pm_vers) && 
		xdr_u_long(xdrs, &regs->pm_prot))
		return (xdr_u_long(xdrs, &regs->pm_port));
	return (FALSE);
}


/* 
 * What is going on with linked lists? (!)
 * First recall the link list declaration from pmap_prot.h:
 *
 * struct pmaplist {
 *	struct pmap pml_map;
 *	struct pmaplist *pml_map;
 * };
 *
 * Compare that declaration with a corresponding xdr declaration that 
 * is (a) pointer-less, and (b) recursive:
 *
 * typedef union switch (bool_t) {
 * 
 *	case TRUE: struct {
 *		struct pmap;
 * 		pmaplist_t foo;
 *	};
 *
 *	case FALSE: struct {};
 * } pmaplist_t;
 *
 * Notice that the xdr declaration has no nxt pointer while
 * the C declaration has no bool_t variable.  The bool_t can be
 * interpreted as ``more data follows me''; if FALSE then nothing
 * follows this bool_t; if TRUE then the bool_t is followed by
 * an actual struct pmap, and then (recursively) by the 
 * xdr union, pamplist_t.  
 *
 * This could be implemented via the xdr_union primitive, though this
 * would cause a one recursive call per element in the list.  Rather than do
 * that we can ``unwind'' the recursion
 * into a while loop and do the union arms in-place.
 *
 * The head of the list is what the C programmer wishes to past around
 * the net, yet is the data that the pointer points to which is interesting;
 * this sounds like a job for xdr_reference!
 */
bool_t
xdr_pmaplist(xdrs, rp)
	register XDR *xdrs;
	register struct pmaplist **rp;
{
	/*
	 * more_elements is pre-computed in case the direction is
	 * XDR_ENCODE or XDR_FREE.  more_elements is overwritten by
	 * xdr_bool when the direction is XDR_DECODE.
	 */
	bool_t more_elements;
	register int freeing = (xdrs->x_op == XDR_FREE);
	register struct pmaplist **next;

	while (TRUE) {
		more_elements = (bool_t)(*rp != NULL);
		if (! xdr_bool(xdrs, &more_elements))
			return (FALSE);
		if (! more_elements)
			return (TRUE);  /* we are done */
		/*
		 * the unfortunate side effect of non-recursion is that in
		 * the case of freeing we must remember the next object
		 * before we free the current object ...
		 */
		if (freeing)
			next = &((*rp)->pml_next); 
		if (! xdr_reference(xdrs, (caddr_t *)rp,
		    (u_int)sizeof(struct pmaplist), xdr_pmap))
/*			return (FALSE);*/
			return (TRUE); 	/* may be a continuation packet prob */
		rp = (freeing) ? next : &((*rp)->pml_next);
	}
}

prt_pmaplist(rp)
	struct pmaplist *rp;
{
	reset_beg_line(pm_beg);
	printf("%s",beg_line);
	printf("PMAP List is:\n");

	for (;rp;rp=rp->pml_next)
		prt_pmap(rp->pml_map);
}
