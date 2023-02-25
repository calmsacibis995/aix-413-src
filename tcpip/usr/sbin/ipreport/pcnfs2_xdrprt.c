static char sccsid[] = "@(#)90	1.2  src/tcpip/usr/sbin/ipreport/pcnfs2_xdrprt.c, tcpras, tcpip411, GOLD410 6/10/93 14:19:47";
/*
 * COMPONENT_NAME: TCPIP pcnfs2_xdrprt.c
 *
 * ORIGINS: 24 27
 *
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
 * This file contains the XDR and PRT routines to decode and print PCnfs
 * rpc packets for the ipreport command FOR VERSION 2 of PCNFS.
 * 
 * It was mostly stolen and modified from
 * the latest PD sun pcnfsd_xdr.c code for the latest pcnfs.
 * 
 * Created on Wed Jul 29 15:19:34 CDT 1992
 */


#include <uinfo.h>
#include <sys/types.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <pwd.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <netdb.h>
#include "ipr.h"
#include "pcnfsd.h"

int             buggit=0;
static	char * pc_beg="PC:  ";

/*
 * *************** RPC parameters ******************** 
		see ipr.h
*/

/* RE_SID: @(%)/tmp_mnt/vol/dosnfs/shades_SCCS/unix/pcnfsd/v2/src/SCCS/s.pcnfsd_xdr.c 1.1 91/09/03 12:47:06 SMI */

/*
 * *************** XDR procedures ***************** 
 */

bool_t
xdr_ident(xdrs, objp)
	XDR *xdrs;
	ident *objp;
{
	if (!xdr_string(xdrs, objp, IDENTLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_message(xdrs, objp)
	XDR *xdrs;
	message *objp;
{
	if (!xdr_string(xdrs, objp, MESSAGELEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_password(xdrs, objp)
	XDR *xdrs;
	password *objp;
{
	if (!xdr_string(xdrs, objp, PASSWORDLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_client(xdrs, objp)
	XDR *xdrs;
	client *objp;
{
	if (!xdr_string(xdrs, objp, CLIENTLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_printername(xdrs, objp)
	XDR *xdrs;
	printername *objp;
{
	if (!xdr_string(xdrs, objp, PRINTERNAMELEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_username(xdrs, objp)
	XDR *xdrs;
	username *objp;
{
	if (!xdr_string(xdrs, objp, USERNAMELEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_comment(xdrs, objp)
	XDR *xdrs;
	comment *objp;
{
	if (!xdr_string(xdrs, objp, COMMENTLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_spoolname(xdrs, objp)
	XDR *xdrs;
	spoolname *objp;
{
	if (!xdr_string(xdrs, objp, SPOOLNAMELEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_printjobid(xdrs, objp)
	XDR *xdrs;
	printjobid *objp;
{
	if (!xdr_string(xdrs, objp, PRINTJOBIDLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_homedir(xdrs, objp)
	XDR *xdrs;
	homedir *objp;
{
	if (!xdr_string(xdrs, objp, OPTIONSLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_options(xdrs, objp)
	XDR *xdrs;
	options *objp;
{
	if (!xdr_string(xdrs, objp, OPTIONSLEN)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_arstat(xdrs, objp)
	XDR *xdrs;
	arstat *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_alrstat(xdrs, objp)
	XDR *xdrs;
	alrstat *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pirstat(xdrs, objp)
	XDR *xdrs;
	pirstat *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pcrstat(xdrs, objp)
	XDR *xdrs;
	pcrstat *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_psrstat(xdrs, objp)
	XDR *xdrs;
	psrstat *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_mapreq(xdrs, objp)
	XDR *xdrs;
	mapreq *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_maprstat(xdrs, objp)
	XDR *xdrs;
	maprstat *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}


bool_t
xdr_v2_info_args(xdrs, objp)
	XDR *xdrs;
	v2_info_args *objp;
{
	if (!xdr_comment(xdrs, &objp->vers)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_info_results(xdrs, objp)
	XDR *xdrs;
	v2_info_results *objp;
{
	if (!xdr_comment(xdrs, &objp->vers)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->facilities.facilities_val, (u_int *)&objp->facilities.facilities_len, FACILITIESMAX, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_init_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_init_args *objp;
{
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_init_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_init_results *objp;
{
	if (!xdr_pirstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_spoolname(xdrs, &objp->dir)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_start_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_start_args *objp;
{
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_spoolname(xdrs, &objp->file)) {
		return (FALSE);
	}
	if (!xdr_options(xdrs, &objp->opts)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->copies)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_start_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_start_results *objp;
{
	if (!xdr_psrstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_printjobid(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pr_list(xdrs, objp)
	XDR *xdrs;
	pr_list *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct pr_list_item), xdr_pr_list_item)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pr_list_item(xdrs, objp)
	XDR *xdrs;
	pr_list_item *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_printername(xdrs, &objp->device)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->remhost)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_pr_list(xdrs, &objp->pr_next)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_list_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_list_results *objp;
{
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_pr_list(xdrs, &objp->printers)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_queue_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_queue_args *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->just_mine)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pr_queue(xdrs, objp)
	XDR *xdrs;
	pr_queue *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct pr_queue_item), xdr_pr_queue_item)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pr_queue_item(xdrs, objp)
	XDR *xdrs;
	pr_queue_item *objp;
{
	if (!xdr_int(xdrs, &objp->position)) {
		return (FALSE);
	}
	if (!xdr_printjobid(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->size)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->status)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_spoolname(xdrs, &objp->file)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_pr_queue(xdrs, &objp->pr_next)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_queue_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_queue_results *objp;
{
	if (!xdr_pirstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->just_yours)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->qlen)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->qshown)) {
		return (FALSE);
	}
	if (!xdr_pr_queue(xdrs, &objp->jobs)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_cancel_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_cancel_args *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_printjobid(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_cancel_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_cancel_results *objp;
{
	if (!xdr_pcrstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_status_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_status_args *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_status_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_status_results *objp;
{
	if (!xdr_pirstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->avail)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->printing)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->qlen)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->needs_operator)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->status)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_admin_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_admin_args *objp;
{
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_admin_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_admin_results *objp;
{
	if (!xdr_pirstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_requeue_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_requeue_args *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_printjobid(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->qpos)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_requeue_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_requeue_results *objp;
{
	if (!xdr_pcrstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_hold_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_hold_args *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_printjobid(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_hold_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_hold_results *objp;
{
	if (!xdr_pcrstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_release_args(xdrs, objp)
	XDR *xdrs;
	v2_pr_release_args *objp;
{
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_printjobid(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_pr_release_results(xdrs, objp)
	XDR *xdrs;
	v2_pr_release_results *objp;
{
	if (!xdr_pcrstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_mapreq_arg(xdrs, objp)
	XDR *xdrs;
	mapreq_arg *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct mapreq_arg_item), xdr_mapreq_arg_item)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_mapreq_arg_item(xdrs, objp)
	XDR *xdrs;
	mapreq_arg_item *objp;
{
	if (!xdr_mapreq(xdrs, &objp->req)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->name)) {
		return (FALSE);
	}
	if (!xdr_mapreq_arg(xdrs, &objp->mapreq_next)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_mapreq_res(xdrs, objp)
	XDR *xdrs;
	mapreq_res *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct mapreq_res_item), xdr_mapreq_res_item)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_mapreq_res_item(xdrs, objp)
	XDR *xdrs;
	mapreq_res_item *objp;
{
	if (!xdr_mapreq(xdrs, &objp->req)) {
		return (FALSE);
	}
	if (!xdr_maprstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->name)) {
		return (FALSE);
	}
	if (!xdr_mapreq_res(xdrs, &objp->mapreq_next)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_mapid_args(xdrs, objp)
	XDR *xdrs;
	v2_mapid_args *objp;
{
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_mapreq_arg(xdrs, &objp->req_list)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_mapid_results(xdrs, objp)
	XDR *xdrs;
	v2_mapid_results *objp;
{
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	if (!xdr_mapreq_res(xdrs, &objp->res_list)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_auth_args(xdrs, objp)
	XDR *xdrs;
	v2_auth_args *objp;
{
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_ident(xdrs, &objp->id)) {
		return (FALSE);
	}
	if (!xdr_password(xdrs, &objp->pw)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_auth_results(xdrs, objp)
	XDR *xdrs;
	v2_auth_results *objp;
{
	if (!xdr_arstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->uid)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->gid)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->gids.gids_val, (u_int *)&objp->gids.gids_len, EXTRAGIDLEN, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	if (!xdr_homedir(xdrs, &objp->home)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->def_umask)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_alert_args(xdrs, objp)
	XDR *xdrs;
	v2_alert_args *objp;
{
	if (!xdr_client(xdrs, &objp->system)) {
		return (FALSE);
	}
	if (!xdr_printername(xdrs, &objp->pn)) {
		return (FALSE);
	}
	if (!xdr_username(xdrs, &objp->user)) {
		return (FALSE);
	}
	if (!xdr_message(xdrs, &objp->msg)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_v2_alert_results(xdrs, objp)
	XDR *xdrs;
	v2_alert_results *objp;
{
	if (!xdr_alrstat(xdrs, &objp->stat)) {
		return (FALSE);
	}
	if (!xdr_comment(xdrs, &objp->cm)) {
		return (FALSE);
	}
	return (TRUE);
}
#if RPC_SVC
 void msg_out(msg) char *msg; {_msgout(msg);}
#endif
#if RPC_HDR
 extern void msg_out();
#endif

/*
 * *************** PRT procedures ***************** 
 */

bool_t
prt_v2_info_args(objp)
	v2_info_args *objp;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	printf("Version=%s\n",objp->vers);
	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

bool_t
prt_v2_info_results(objp)
	v2_info_results *objp;
{
	int i,*p;
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	printf("Version=%s\n",objp->vers);
	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	printf("%s",beg_line);
	i=objp->facilities.facilities_len;
	printf("%d Facilities:\n",i);
	printf("%s",beg_line);
	for (p=objp->facilities.facilities_val; i>0; i--)
		printf("%d ",*p++);
	printf("\n");

	return (TRUE);
}



bool_t
prt_v2_auth_args(objp)
	v2_auth_args *objp;
{
        char            username[32];


	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Client=%s\n",objp->system);

	printf("%s",beg_line);
        scramble(objp->id, username);
	printf("(Scrambled)   Ident=%s Passwd=%s\n",objp->id,objp->pw);
	printf("%s",beg_line);
	printf("(Unscrambled) Ident=%s \n", username );

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}


bool_t
prt_v2_auth_results(objp)
	v2_auth_results *objp;
{
	int i,*p;

	reset_beg_line(pc_beg);

	prt_ar_stat(objp->stat);

	printf("%s",beg_line);
	printf("Uid=%u Gid=%u\n",objp->uid,objp->gid);

	printf("%s",beg_line);
	i=objp->gids.gids_len;
	printf("%d Groups in Grouplist:\t",i);
	for (p=objp->gids.gids_val; i>0; i--)
		printf("%d ",*p++);
	printf("\n");

	printf("%s",beg_line);
	printf("HomeDirectory=%s:\n",objp->home);

	printf("%s",beg_line);
	printf("Umask=%o \n",objp->def_umask);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_list_results(objp)
	v2_pr_list_results *objp;
{
	struct pr_list_item *p;

	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	for (p=objp->printers; p; p=p->pr_next) {
		printf("%s",beg_line);
		printf("PrinterName=%s Device=%s RemHost=%s Comment=%s\n",
			p->pn,p->device,p->remhost,p->cm);
	}
		
	return (TRUE);
}

bool_t
prt_v2_pr_init_args(objp)
	v2_pr_init_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("System=%s\n",objp->system);

	printf("%s",beg_line);
	printf("PrinterName=%s\n",objp->pn);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_init_results(objp)
	v2_pr_init_results *objp;
{
	reset_beg_line(pc_beg);

	prt_pir_stat(objp->stat);

	printf("%s",beg_line);
	printf("Spool=%s\n",objp->dir);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_queue_args(objp)
	v2_pr_queue_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("PrinterName=%s\tClient=%s\tUser=%s\n",
			objp->pn, objp->system,objp->user);

	printf("%s",beg_line);
	printf("JustMine: %s\n", (objp->just_mine?"TRUE":"FALSE") );

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

bool_t
prt_v2_pr_queue_results(objp)
	v2_pr_queue_results *objp;
{
	pr_queue p;

	reset_beg_line(pc_beg);

	prt_pir_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	printf("%s",beg_line);
	printf("JustYours: %s\n", (objp->just_yours?"TRUE":"FALSE") );

	printf("%s",beg_line);
	printf("QueueLength=%d  QueueShown=%d\n",
			objp->qlen,objp->qshown);

	for (p=objp->jobs; p; p=p->pr_next) {
		printf("%s",beg_line);
		printf("Position=%d PrintJobID=%s Size=%s Status=%s\n",
			p->position, p->id, p->size, p->status);
		printf("%s",beg_line);
		printf(" Client=%s User=%s File=%s Comment=%s\n",
			p->system, p->user, p->file, p->cm);
	}

	return (TRUE);
}

bool_t
prt_v2_pr_cancel_args(objp)
	v2_pr_cancel_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);

	printf("PrinterName=%s Client=%s\n", objp->pn,objp->system);

	printf("%s",beg_line);
	printf("User=%s PrintJobID=%s\n", objp->user,objp->id);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

prt_pcr_stat(pcres)
int pcres;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	switch (pcres) {
	case PC_RES_OK:              printf("PC_RES_OK              "); break; 
	case PC_RES_NO_SUCH_PRINTER: printf("PC_RES_NO_SUCH_PRINTER "); break; 
	case PC_RES_NO_SUCH_JOB:     printf("PC_RES_NO_SUCH_JOB     "); break; 
	case PC_RES_NOT_OWNER:       printf("PC_RES_NOT_OWNER       "); break; 
	case PC_RES_FAIL:            printf("PC_RES_FAIL            "); break; 
	default:	             printf("Bad PC RES! ");            break; 
	}
	printf(" (%d)\n",pcres);
}

bool_t
prt_v2_pr_cancel_results(objp)
	v2_pr_cancel_results *objp;
{
	reset_beg_line(pc_beg);

	prt_pcr_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

prt_v2_pr_start_args(objp)
	v2_pr_start_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Client=%s Printername=%s User=%s \n",
				objp->system, objp->pn, objp->user);

	printf("%s",beg_line);
	printf("Spool=%s Options=%s Copies=%d\n",
			objp->file,objp->opts,objp->copies);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}


prt_psr_stat(psres)
int psres;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	switch (psres) {
		case PS_RES_OK:      printf("PS_RES_OK      "); break; 
		case PS_RES_ALREADY: printf("PS_RES_ALREADY "); break; 
		case PS_RES_NULL:    printf("PS_RES_NULL    "); break; 
		case PS_RES_NO_FILE: printf("PS_RES_NO_FILE "); break; 
		case PS_RES_FAIL:    printf("PS_RES_FAIL    "); break; 
		default:	     printf("Bad PS RES! ");    break; 
	}
	printf(" (%d)\n",psres);
}

bool_t
prt_v2_pr_start_results(objp)
	v2_pr_start_results *objp;
{
	reset_beg_line(pc_beg);

	prt_psr_stat(objp->stat);

	printf("%s",beg_line);
	printf("PrintJobID=%s \n",objp->id);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_status_args(objp)
	v2_pr_status_args *objp;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);
	printf("PrinterName=%s Comment=%s\n",objp->pn,objp->cm);
	return (TRUE);
}

bool_t
prt_v2_pr_status_results(objp)
	v2_pr_status_results *objp;
{
	reset_beg_line(pc_beg);

	prt_pir_stat(objp->stat);

	printf("%s",beg_line);
	printf("Available=%s Printing=%s QueueLength=%d NeedOperator=%s\n",
		objp->avail?"TRUE":"FALSE",
		objp->printing?"TRUE":"FALSE",
		objp->qlen,
		objp->needs_operator?"TRUE":"FALSE");

	printf("%s",beg_line);
	printf("Status=%s Comment=%s\n",objp->status,objp->cm);
	return (TRUE);
}

bool_t
prt_v2_pr_admin_args(objp)
	v2_pr_admin_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Client=%s User=%s\n",objp->system,objp->user);

	printf("%s",beg_line);
	printf("PrinterName=%s Comment=%s\n",objp->pn,objp->cm);
	return (TRUE);
}

bool_t
prt_v2_pr_admin_results(objp)
	v2_pr_admin_results *objp;
{
	reset_beg_line(pc_beg);

	prt_pir_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

prt_v2_pr_requeue_args(objp)
	v2_pr_requeue_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("PrinterName=%s Client=%s User=%s\n",
			objp->pn, objp->system,objp->user);

	printf("%s",beg_line);
	printf("PrintJobID=%s QueuePosition=%d \n",objp->id,objp->qpos);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

bool_t
prt_v2_pr_requeue_results(objp)
	v2_pr_requeue_results *objp;
{
	prt_pcr_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);
	return (TRUE);
}

bool_t
prt_v2_pr_hold_args(objp)
	v2_pr_hold_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("PrinterName=%s Client=%s User=%s\n",
			objp->pn, objp->system,objp->user);

	printf("%s",beg_line);
	printf("PrintJobID=%s \n",objp->id);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_hold_results(objp)
	v2_pr_hold_results *objp;
{
	reset_beg_line(pc_beg);

	prt_pcr_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_release_args(objp)
	v2_pr_release_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("PrinterName=%s Client=%s User=%s\n",
			objp->pn, objp->system,objp->user);

	printf("%s",beg_line);
	printf("PrintJobID=%s \n",objp->id);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

bool_t
prt_v2_pr_release_results(objp)
	v2_pr_release_results *objp;
{
	reset_beg_line(pc_beg);

	prt_pcr_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}

prt_maprstat(int stat)
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);

	switch (stat) {
		case MAP_RES_OK:      printf("MAP_RES_OK      "); break; 
		case MAP_RES_UNKNOWN: printf("MAP_RES_UNKNOWN "); break; 
		case MAP_RES_DENIED:  printf("MAP_RES_DENIED  "); break; 
		default:              printf("Bad_MAPRES!     "); break; 
	}
	printf(" (%d)\n",stat);
}

prt_mapreq(int req)
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	switch (req) {
		case     MAP_REQ_UID:   printf("MAP_REQ_UID   "); break; 
		case     MAP_REQ_GID:   printf("MAP_REQ_GID   "); break; 
		case     MAP_REQ_UNAME: printf("MAP_REQ_UNAME "); break; 
		case     MAP_REQ_GNAME: printf("MAP_REQ_GNAME "); break; 
		default:                printf("Bad_MAPREQ!   "); break; 
	}
	printf(" (%d)\n",req);
}


bool_t
prt_v2_mapid_args(objp)
	v2_mapid_args *objp;
{
	mapreq_arg p;
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	for (p=objp->req_list; p; p=p->mapreq_next) {
		prt_mapreq(p->req);
		printf("%s",beg_line);
		printf("Id=%d Name=%s\n",
			p->id,p->name);
	}

	return (TRUE);
}

bool_t
prt_v2_mapid_results(objp)
	v2_mapid_results *objp;
{
	mapreq_res p;
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	for (p=objp->res_list; p; p=p->mapreq_next) {
		prt_mapreq(p->req);
		prt_maprstat(p->stat);
		printf("%s",beg_line);
		printf("Id=%d Name=%s\n",
			p->id,p->name);
	}
	return (TRUE);
}

prt_v2_alert_args(objp)
	v2_alert_args *objp;
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	printf("Client=%s PrinterName=%s User=%s\n",
			objp->system,objp->pn, objp->user);

	printf("Message=%s \n", objp->msg);
	
	return (TRUE);
}


prt_alr_stat(int stat)
{
	reset_beg_line(pc_beg);

	printf("%s",beg_line);
	switch (stat) {
		case  ALERT_RES_OK:   	printf("ALERT_RES_OK   "); break; 
		case  ALERT_RES_FAIL:   printf("ALERT_RES_FAIL "); break; 
		default:                printf("BadALERT STAT! "); break; 
	}
	printf(" (%d)\n",stat);
}


prt_v2_alert_results(objp)
	v2_alert_results *objp;
{
	reset_beg_line(pc_beg);

	prt_alr_stat(objp->stat);

	printf("%s",beg_line);
	printf("Comment=%s\n",objp->cm);

	return (TRUE);
}
