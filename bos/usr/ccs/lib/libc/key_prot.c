static char sccsid[] = "@(#)90  1.8  src/bos/usr/ccs/lib/libc/key_prot.c, libcrpc, bos411, 9428A410j 10/25/93 20:40:44";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: xdr_cryptkeyarg
 *		xdr_cryptkeyres
 *		xdr_getcredres
 *		xdr_keybuf
 *		xdr_keystatus
 *		xdr_netnamestr
 *		xdr_unixcred
 *		
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)key_prot.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.4 88/02/08 
 */

#include <rpc/rpc.h>
#include <rpc/key_prot.h>


/* 
 * Compiled from key_prot.x using rpcgen.
 * DO NOT EDIT THIS FILE!
 * This is NOT source code!
 */


bool_t
xdr_keystatus(xdrs, objp)
	XDR *xdrs;
	keystatus *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}


#ifndef _KERNEL


bool_t
xdr_keybuf(xdrs, objp)
	XDR *xdrs;
	keybuf objp;
{
	if (!xdr_opaque(xdrs, objp, HEXKEYBYTES)) {
		return (FALSE);
	}
	return (TRUE);
}


#endif


bool_t
xdr_netnamestr(xdrs, objp)
	XDR *xdrs;
	netnamestr *objp;
{
	if (!xdr_string(xdrs, objp, MAXNETNAMELEN)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_cryptkeyarg(xdrs, objp)
	XDR *xdrs;
	cryptkeyarg *objp;
{
	if (!xdr_netnamestr(xdrs, &objp->remotename)) {
		return (FALSE);
	}
	if (!xdr_des_block(xdrs, &objp->deskey)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_cryptkeyres(xdrs, objp)
	XDR *xdrs;
	cryptkeyres *objp;
{
	if (!xdr_keystatus(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case KEY_SUCCESS:
		if (!xdr_des_block(xdrs, &objp->cryptkeyres_u.deskey)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_unixcred(xdrs, objp)
	XDR *xdrs;
	unixcred *objp;
{
	if (!xdr_u_int(xdrs, (uid_t *)&objp->uid)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, (gid_t *)&objp->gid)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->gids.gids_val, (u_int *)&objp->gids.gids_len, MAXGIDS, sizeof(int), xdr_u_int)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_getcredres(xdrs, objp)
	XDR *xdrs;
	getcredres *objp;
{
	if (!xdr_keystatus(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case KEY_SUCCESS:
		if (!xdr_unixcred(xdrs, &objp->getcredres_u.cred)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}

