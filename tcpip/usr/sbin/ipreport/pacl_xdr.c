static char sccsid[] = "@(#)19	1.1  src/tcpip/usr/sbin/ipreport/pacl_xdr.c, tcpras, tcpip411, GOLD410 2/7/92 16:27:39";
/* 
 * COMPONENT_NAME: (SYSXNFS) Network File System Kernel
 * 
 * FUNCTIONS: xdr_access_id_type, xdr_access_type,
 *	xdr_access_permissions, xdr_acc_permlist,
 *	xdr_acl_mode_type, xdr_access_identifier,
 *	xdr_acl_entry_xdr, xdr_acl_xdr,
 *	xdr_pcl_privileges, xdr_pcl_privlist,
 *	xdr_pcl_entry_xdr, xdr_pcl_mode_type,
 *	xdr_pcl_xdr, xdr_fh_cookie, xdr_acl_pcl_cookie,
 *	xdr_rep_acl_pcl_cookie, xdr_acl_pcl_req_block,
 *	xdr_acl_pcl_reply, xdr_acl_pcl_inuse
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include "pacl.h"
#include "ipr.h"


bool_t
xdr_access_id_type(xdrs, objp)
XDR *xdrs;
access_id_type *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_access_type(xdrs, objp)
XDR *xdrs;
access_type *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_access_permissions(xdrs, objp)
XDR *xdrs;
access_permissions *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_acc_permlist(xdrs, objp)
XDR *xdrs;
acc_permlist *objp;
{
	if (!xdr_array(xdrs, (char **)&objp->acc_permlist_val, (u_int *)&objp->acc_permlist_len, ~0, sizeof(access_permissions),
	    xdr_access_permissions)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_acl_mode_type(xdrs, objp)
XDR *xdrs;
acl_mode_type *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_access_identifier(xdrs, objp)
XDR *xdrs;
access_identifier *objp;
{
	if (!xdr_access_id_type(xdrs, &objp->id_type)) {
		return (FALSE);
	}
	switch (objp->id_type) {
	case ACCESS_USER_XDR:
		if (!xdr_array(xdrs, (char **)&objp->access_identifier_u.uids.uids_val, (u_int *)&objp->access_identifier_u.uids.uids_len,
		    ~0, sizeof(u_long), xdr_u_long)) {
			return (FALSE);
		}
		break;
	case ACCESS_GROUP_XDR:
		if (!xdr_array(xdrs, (char **)&objp->access_identifier_u.gids.gids_val, (u_int *)&objp->access_identifier_u.gids.gids_len,
		    ~0, sizeof(u_long), xdr_u_long)) {
			return (FALSE);
		}
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_acl_entry_xdr(xdrs, objp)
XDR *xdrs;
acl_entry_xdr *objp;
{
	if (!xdr_access_type(xdrs, &objp->ace_type)) {
		return (FALSE);
	}
	if (!xdr_acc_permlist(xdrs, &objp->ace_access)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->ace_entries.ace_entries_val, (u_int *)&objp->ace_entries.ace_entries_len, ~0,
	    sizeof(access_identifier), xdr_access_identifier)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_acl_xdr(xdrs, objp)
XDR *xdrs;
acl_xdr *objp;
{
	if (!xdr_array(xdrs, (char **)&objp->acl_mode.acl_mode_val, (u_int *)&objp->acl_mode.acl_mode_len, ~0, sizeof(acl_mode_type),
	    xdr_acl_mode_type)) {
		return (FALSE);
	}
	if (!xdr_acc_permlist(xdrs, &objp->u_access)) {
		return (FALSE);
	}
	if (!xdr_acc_permlist(xdrs, &objp->g_access)) {
		return (FALSE);
	}
	if (!xdr_acc_permlist(xdrs, &objp->o_access)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->acl_entry.acl_entry_val, (u_int *)&objp->acl_entry.acl_entry_len, ~0, sizeof(acl_entry_xdr),
	    xdr_acl_entry_xdr)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_pcl_privileges(xdrs, objp)
XDR *xdrs;
pcl_privileges *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_pcl_privlist(xdrs, objp)
XDR *xdrs;
pcl_privlist *objp;
{
	if (!xdr_array(xdrs, (char **)&objp->pcl_privlist_val, (u_int *)&objp->pcl_privlist_len, ~0, sizeof(pcl_privileges),
	    xdr_pcl_privileges)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_pcl_entry_xdr(xdrs, objp)
XDR *xdrs;
pcl_entry_xdr *objp;
{
	if (!xdr_pcl_privlist(xdrs, &objp->pce_access)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->pce_entries.pce_entries_val, (u_int *)&objp->pce_entries.pce_entries_len, ~0,
	    sizeof(access_identifier), xdr_access_identifier)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_pcl_mode_type(xdrs, objp)
XDR *xdrs;
pcl_mode_type *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_pcl_xdr(xdrs, objp)
XDR *xdrs;
pcl_xdr *objp;
{
	if (!xdr_array(xdrs, (char **)&objp->pcl_mode.pcl_mode_val, (u_int *)&objp->pcl_mode.pcl_mode_len, ~0, sizeof(pcl_mode_type),
	    xdr_pcl_mode_type)) {
		return (FALSE);
	}
	if (!xdr_pcl_privlist(xdrs, &objp->pce_access_default)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->pcl_entry.pcl_entry_val, (u_int *)&objp->pcl_entry.pcl_entry_len, ~0, sizeof(pcl_entry_xdr),
	    xdr_pcl_entry_xdr)) {
		return (FALSE);
	}
	return (TRUE);
}



bool_t
prt_fh_cookie(objp)
fh_cookie *objp;
{
	reset_beg_line(acl_beg);
	printf("%s",beg_line);
	printf("FH_Cookie:\n");
	prt_netobj(objp);
	return (TRUE);
}

bool_t
xdr_fh_cookie(xdrs, objp)
XDR *xdrs;
fh_cookie *objp;
{
	if (!xdr_bytes(xdrs, (char **)&objp->fh_cookie_val, (u_int *)&objp->fh_cookie_len, ~0)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
prt_acl_pcl_cookie(objp)
acl_pcl_cookie *objp;
{
	reset_beg_line(acl_beg);
	prt_fh_cookie(&objp->cookie);
	printf("%s",beg_line);
	printf("ID:\n");
	hex_dump(objp->acl_pcl_id.acl_pcl_id_val, 
	    objp->acl_pcl_id.acl_pcl_id_len);
	printf("%s",beg_line);
	printf("size=%u\n",objp->size);

}


bool_t
xdr_acl_pcl_cookie(xdrs, objp)
XDR *xdrs;
acl_pcl_cookie *objp;
{
	if (!xdr_fh_cookie(xdrs, &objp->cookie)) {
		return (FALSE);
	}
	if (!xdr_bytes(xdrs, (char **)&objp->acl_pcl_id.acl_pcl_id_val, (u_int *)&objp->acl_pcl_id.acl_pcl_id_len, ~0)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->size)) {
		return (FALSE);
	}
	return (TRUE);
}

prtaclerr(error)
int error;
{
	reset_beg_line(acl_beg);
	printf("%s",beg_line);
	printf("error=%d\n",error);
}

bool_t
prt_rep_acl_pcl_cookie( objp)
rep_acl_pcl_cookie *objp;
{
	switch (objp->status) {
	case 0:
		prt_acl_pcl_cookie(&objp->rep_acl_pcl_cookie_u.cookie);
		break;
	default:
		prtaclerr(objp->rep_acl_pcl_cookie_u.error);
		break;
	}
	return (TRUE);
}



bool_t
xdr_rep_acl_pcl_cookie(xdrs, objp)
XDR *xdrs;
rep_acl_pcl_cookie *objp;
{
	if (!xdr_int(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case 0:
		if (!xdr_acl_pcl_cookie(xdrs, &objp->rep_acl_pcl_cookie_u.cookie)) {
			return (FALSE);
		}
		break;
	default:
		if (!xdr_int(xdrs, &objp->rep_acl_pcl_cookie_u.error)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}


bool_t
prt_acl_pcl_block(objp)
acl_pcl_block *objp;
{
	prt_acl_pcl_cookie(&objp->cookie);
	reset_beg_line(acl_beg);
	printf("%s",beg_line);
	printf("offset=%d count=%d\n",objp->offset,objp->count);
	printf("%s",beg_line);
	printf("Data:\n");
	hex_dump(objp->acl_pcl_data.acl_pcl_data_val, 
			objp->acl_pcl_data.acl_pcl_data_len);
	printf("%s",beg_line);
	prt_bool(objp->eof);
	printf("\n");
	return (TRUE);
}


bool_t
xdr_acl_pcl_block(xdrs, objp)
XDR *xdrs;
acl_pcl_block *objp;
{
	if (!xdr_acl_pcl_cookie(xdrs, &objp->cookie)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->offset)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->count)) {
		return (FALSE);
	}
	if (!xdr_bytes(xdrs, (char **)&objp->acl_pcl_data.acl_pcl_data_val, (u_int *)&objp->acl_pcl_data.acl_pcl_data_len,
	    ~0)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->eof)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
prt_acl_pcl_req_block(objp)
acl_pcl_req_block *objp;
{
	reset_beg_line(acl_beg);
	prt_acl_pcl_cookie(&objp->cookie);
	printf("%s",beg_line);
	printf("offset=%d count=%d\n",objp->offset,objp->count);
}

bool_t
xdr_acl_pcl_req_block(xdrs, objp)
XDR *xdrs;
acl_pcl_req_block *objp;
{
	if (!xdr_acl_pcl_cookie(xdrs, &objp->cookie)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->offset)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->count)) {
		return (FALSE);
	}
	return (TRUE);
}





bool_t
prt_acl_pcl_reply(objp)
acl_pcl_reply *objp;
{
	switch (objp->status) {
	case 0:
		prt_acl_pcl_block(&objp->acl_pcl_reply_u.block_reply);
		break;
	default:
		prtaclerr(objp->acl_pcl_reply_u.error);
	}
	return (TRUE);
}






bool_t
xdr_acl_pcl_reply(xdrs, objp)
XDR *xdrs;
acl_pcl_reply *objp;
{
	if (!xdr_int(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case 0:
		if (!xdr_acl_pcl_block(xdrs, &objp->acl_pcl_reply_u.block_reply)) {
			return (FALSE);
		}
		break;
	default:
		if (!xdr_int(xdrs, &objp->acl_pcl_reply_u.error)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_acl_pcl_inuse(xdrs, objp)
XDR *xdrs;
acl_pcl_inuse *objp;
{
	if (!xdr_int(xdrs, &objp->error)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->acl_inuse)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->pcl_inuse)) {
		return (FALSE);
	}
	return (TRUE);
}


