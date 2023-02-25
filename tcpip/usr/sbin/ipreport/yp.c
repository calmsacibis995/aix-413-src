static char sccsid[] = "@(#)03	1.3  src/tcpip/usr/sbin/ipreport/yp.c, tcpras, tcpip411, GOLD410 2/7/92 16:21:42";
/*
 * COMPONENT_NAME: TCPIP yp.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * This is based on the yp xdr code but basically all new.
 */




/*
	yp.c created by 
	Wed Sep 04 11:31:48 CDT 1991 
		starring  the following routines
			prt_datum,
			prt_yp_binding,
			prt_yp_inaddr,
			prt_ypall,
			prt_ypbind_resp,
			prt_ypdomain_wrap_string,
			prt_ypmap_parms,
			prt_ypmap_wrap_string,
			prt_ypowner_wrap_string,
			prt_ypproc_domain_noack_res,
			prt_ypreq_key,
			prt_ypreq_nokey,
			prt_ypreq_xfr,
			prt_ypresp_key_val,
			prt_ypresp_maplist,
			prt_ypresp_master,
			prt_ypresp_order,
			prt_ypresp_val
*/




#define NULL 0
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>



#include "sys/param.h"
#include "sys/pri.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "dirent.h"
#include "sys/mbuf.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/vfs.h"
#include "rpc/types.h"
#include "rpc/xdr.h"
#undef NFSSERVER
#include "nfs.h"
#include "net/spl.h"
#include "netinet/in.h"
#include "ipr.h"
#include <rpcsvc/yp_prot.h>

char *yp_beg="YP:  ";


	
/*
 * PRinT Yellow Pages REQuest with KEY
 * 
 */
prt_ypreq_key(yp)
	struct ypreq_key *yp;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Request with key\n");
	printf("%s",beg_line);
	printf("Domain=%s Map=%s \n",yp->domain,yp->map);
	prt_datum(&yp->keydat);
}


/*
 * dumps a dbm datum data structure.
 */
prt_datum(pdatum)
	datum * pdatum;
{
	printf("%s",beg_line);
	printf("Datum_size=%d\n",pdatum->dsize); 
	hex_dump(pdatum->dptr,pdatum->dsize); 
	printf("\n");
}

/*
 * dumps a ypreq_nokey structure.
 */
prt_ypreq_nokey(ps)
	struct ypreq_nokey *ps;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Request without key\n");
	printf("%s",beg_line);
	printf("Domain=%s Map=%s \n",ps->domain,ps->map);
}

/* Dump Return status values */
prt_yp_stat(stat)
int stat;
{
	printf("%s",beg_line);
	switch (stat) {
	case YP_TRUE    : printf("YP_TRUE: (General purpose success code)\n");
			  break;
	case YP_NOMORE  : printf("YP_NOMORE: (No more entries in map)\n");
			  break;
	case YP_FALSE   : printf("YP_FALSE: (General purpose failure code)\n");
			  break;
	case YP_NOMAP   : printf("YP_NOMAP: (No such map in domain)\n");
			  break;
	case YP_NODOM   : printf("YP_NODOM: (Domain not supported)\n");
			  break;
	case YP_NOKEY   : printf("YP_NOKEY: (No such key in map)\n");
			  break;
	case YP_BADOP   : printf("YP_BADOP: (Invalid operation)\n");
			  break;
	case YP_BADDB   : printf("YP_BADDB: (Server data base is bad)\n");
			  break;
	case YP_YPERR   : printf("YP_YPERR: (NIS server error)\n");
			  break;
	case YP_BADARGS : printf("YP_BADARGS: (Request arguments bad)\n");
			  break;
	case YP_VERS    : printf("YP_VERS: (NIS server version mismatch - server can't supply requested service.)\n");
			  break;
	default         : printf("Bad YP stat\n");
			  break;
	}
}

/*
 * Dumps a ypresp_val structure.
 */

prt_ypresp_val( ps)
	struct ypresp_val *ps;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Response Value\n");
	prt_yp_stat( ps->status);
	prt_datum( &ps->valdat) ;
}

/*
 * Dumps a ypresp_key_val structure.
 */
prt_ypresp_key_val( ps)
	struct ypresp_key_val *ps;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Response Key-Value\n");
	prt_yp_stat( ps->status);
	prt_datum( &ps->valdat);
	prt_datum( &ps->keydat);
}


/*
 * Dumps an in_addr struct.
 * 
 * Note:  There is a data coupling between the "definition" of a struct
 * in_addr implicit in this xdr routine, and the true data definition in
 * <netinet/in.h>.  
 */
prt_yp_inaddr( ps)
	struct in_addr *ps;

{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Internet Addr=%15.15s\n", inet_ntoa(ps->s_addr));
}

/*
 * Dumps a ypbind_binding struct.
 */
prt_yp_binding( ps)
	struct ypbind_binding *ps;

{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	prt_yp_inaddr( &ps->ypbind_binding_addr);
        printf( "port=%d\n",ps->ypbind_binding_port);
}


prt_yp_bind_err_stat(stat)
int stat;
{
	printf("%s",beg_line);

/* Detailed failure reason codes for response field ypbind_error*/

	switch (stat) {

	case YPBIND_ERR_ERR:
		printf("YPBIND_ERR_ERR: Internal error\n");
		break;
	case YPBIND_ERR_NOSERV:
		printf("YPBIND_ERR_NOSERV: No bound server for passed domain\n"
			);
		break;
	case YPBIND_ERR_RESC:
		printf("YPBIND_ERR_RESC: System resource allocation failure\n");
		break;
	default:
		printf("Bogus YP Bind Error Code\n");
	}
}


/*
 * Dumps a ypbind_resp structure.
 */
prt_ypbind_resp( ps)
	struct ypbind_resp *ps;

{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Bind Response:\n");
	printf("%s",beg_line);
	switch ( ps->ypbind_status ){
		case YPBIND_SUCC_VAL:
			printf(" YPBIND_SUCC_VAL\n");
			prt_yp_binding(ps->ypbind_respbody);
			break;
		case YPBIND_FAIL_VAL:
			printf(" YPBIND_FAIL_VAL\n"); 
			prt_yp_bind_err_stat(ps->ypbind_respbody);
			break;
		default:
			printf("Protocol Error- Bad YP Bind Status\n");
	}
}
/*
 * Dumps a ypmap_parms structure.
 */
prt_ypmap_parms( ps)
	struct ypmap_parms *ps;

{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Domain=%s Map=%s Ordernum=%d Owner=%s\n",
			ps->domain,
			ps->map,
			ps->ordernum,
			ps->owner);
}
 /* 
 * FUNCTIONS: prt_ypall, xdr_ypbind_setdom, xdr_ypmaplist, 
 *	      prt_ypmaplist_wrap_string, 
 *            prt_yppushresp_xfr, xdr_ypreq_xfr, xdr_ypresp_maplist, 
 *            prt_ypresp_master, xdr_ypresp_order 
 */



/*
 * Dumps a ypresp_master structure.
 */
prt_ypresp_master(ps)
	struct ypresp_master *ps;
{
	reset_beg_line(yp_beg);

	prt_yp_stat(ps->status);
	printf("%s",beg_line);
	printf("Master=%s\n",ps->master );
}

/*
 * Dumps a ypresp_order structure.
 */
prt_ypresp_order(ps)
	struct ypresp_order *ps;
{
	reset_beg_line(yp_beg);

	prt_yp_stat(ps->status);
	printf("%s",beg_line);
	printf("Ordernum=%d\n",ps->ordernum );
}

/*
 * Dumps a ypmaplist.
 */
prt_ypmaplist(lst)
	struct ypmaplist **lst;
{
	struct ypmaplist *p;
	reset_beg_line(yp_beg);

	for (p= *lst; p; p=p->ypml_next) {
		printf("%s",beg_line);
		printf("Map: %s",p->ypml_name);
	}
}


/*
 * Dumps a ypresp_maplist.
 */
prt_ypresp_maplist(ps)
	struct ypresp_maplist *ps;

{
	reset_beg_line(yp_beg);

	prt_yp_stat(ps->status);
	printf("%s",beg_line);
	prt_ypmaplist(&ps->list );
}

/*
 * Dumps a yppushresp_xfr structure.
 */
prt_yppushresp_xfr(ps)
	struct yppushresp_xfr *ps;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Push Response XFR: transid=%d \n",ps->transid);
	prt_yp_stat(ps->status);
}


/*
 * Dumps a ypreq_xfr structure.
 */
prt_ypreq_xfr(ps)
	struct ypreq_xfr *ps;
{
	reset_beg_line(yp_beg);

	prt_ypmap_parms(&ps->map_parms);
	printf("%s",beg_line);
	printf("Transid=%d proto=%d port=%d\n",ps->transid, 
						ps->proto, ps->port);
}


#ifdef ypall_compile
/*
 * Dumps a stream of struct ypresp_key_val's.  This is used
 * only by the client side of the batch enumerate operation.
 */
prt_ypall(callback)
	struct ypall_callback *callback;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	bool more;
	struct ypresp_key_val kv;
	bool s;
	char keybuf[YPMAXRECORD];
	char valbuf[YPMAXRECORD];

	if (xdrs->x_op == XDR_ENCODE)
		return(FALSE);

	if (xdrs->x_op == XDR_FREE)
		return(TRUE);

	kv.keydat.dptr = keybuf;
	kv.valdat.dptr = valbuf;
	kv.keydat.dsize = YPMAXRECORD;
	kv.valdat.dsize = YPMAXRECORD;
	
	for (;;) {
		if (! xdr_bool(xdrs, &more) )
			return (FALSE);
			
		if (! more)
			return (TRUE);

		s = xdr_ypresp_key_val(xdrs, &kv);
		
		if (s) {
			s = (*callback->foreach)(kv.status, kv.keydat.dptr,
			    kv.keydat.dsize, kv.valdat.dptr, kv.valdat.dsize,
			    callback->data);
			
			if (s)
				return (TRUE);
		} else {
			return (FALSE);
		}
	}
}
#endif


/*
 * Dumps a ypbind_setdom structure.
 */
prt_ypbind_setdom(ps)
	struct ypbind_setdom *ps;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("SetDomain=%s\n",ps->ypsetdom_domain);
	
	prt_yp_binding(ps->ypsetdom_binding);

	printf("%s",beg_line);
	printf("SetDom Version=%d\n",ps->ypsetdom_vers);
}

prt_ypdomain_wrap_string(str)
char *str;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("Domain=%s\n",str);
}

prt_ypproc_domain_noack_res(d)
int d;
{
	reset_beg_line(yp_beg);

	printf("%s",beg_line);
	printf("No Ack Result=%d\n",d);
}
