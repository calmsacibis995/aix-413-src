static char sccsid[] = "@(#)99	1.7  src/tcpip/usr/sbin/ipreport/rpc.c, tcpras, tcpip411, GOLD410 3/24/94 15:18:34";
/*
 * COMPONENT_NAME: TCPIP rpc.c
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
 * This file contains the routines to decode and print rpc packets for
 * the ipreport command.
 * 
 * Created Sat Aug 10 1991 
 * 
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/x25user.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/devinfo.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <aixif/net_if.h>
#include "klm_prot.h"
#include "mount.h"
#define NFSSERVER 3
#include "nfs.h"
/*#include <rpcsvc/nfs_prot.h>*/
#include "nlm_prot.h"
#include "rex.h"
#include "rstat.h"
#include "rusers.h"
#include "rwall.h"
#include "sm_inter.h"
#include "spray.h"
#include <rpcsvc/yp_prot.h>
#include <pwd.h>
#include "yppasswd.h"
#include <rpcsvc/ypup_prot.h>
#include "ipr.h"
#include "pacl.h"


/* NLS support */
#ifdef MSG
#include "ipreport_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_IPREPORT,n,s) 
#else
#define MSGSTR(n,s) s
#endif

char *rcp_beg="RPC: ";

#if defined(NLS) || defined(KJI)
#include <locale.h>
#endif

int   c_nfs_dump();
int (*r_get_res_proc())();

/* determines if the passed in packet is a valid rpc packet.*/
isrpc(rm)
struct rpc_msg  *rm;
{
	switch (rm->rm_direction) {
		case CALL :  	/* call must have the right version*/
				if (rm->rm_call.cb_rpcvers != RPC_MSG_VERSION)
					return(0);
				break;
		case REPLY :  	/* reply must be accepted or denied */
				switch (rm->rm_reply.rp_stat) {
					case MSG_ACCEPTED : break;
					case MSG_DENIED   : break;
					default		  : return(0);
				}
				break;
		default	  :	return(0);	/* bad direction, not rpc */
				break;
	}

	return(1);
}


/* print program and procedure */
prt_pp(addr,xid)
u_long addr;
int	xid;
{
	struct cinfo *ci;

	reset_beg_line(rcp_beg);
	printf("%s",beg_line);
	if ( ! (ci=getci(addr,xid)))
	/* we never saw the call for this reply, (probably because it 
	   went by before iptrace was turned on.) */
		printf("No Call on Record in this Trace for this Reply.\n");
	else
		printf("%d(%s) %d(%s)\n", 
			ci->ci_program,
			get_prog_name(ci->ci_program),
			ci->ci_procedure,
			get_proc_name(ci->ci_program,ci->ci_procedure));
}



/* print out an rpc packet */
rpc_dump(ud, cc,saddr,daddr)
char *ud;		/* user data */
int  cc;		/* number of bytes of user data */
u_long saddr,daddr;	/* src and dest addresses for the packet */
{
	struct rpc_msg rp;
	XDR xdrs;
	int	(*reply_dumpproc)()=NULL;
	char resp[MAXPACKET], buf[MAXPACKET];
	struct cinfo cinfo;
	
	int xid;

	reset_beg_line(rcp_beg);

	xdrmem_create(&xdrs,ud,cc,XDR_DECODE);
        XDR_SETPOS(&xdrs, 0);
	bzero(&rp,sizeof(struct rpc_msg));
	bzero(resp,MAXPACKET);

	printf("%s",beg_line);
	if ((ud==NULL) || (cc==0)) {
		printf("Empty RPC packet\n");
		return;
	}
	sprintf(buf,"XID=%u", xid=((struct rpc_msg *)ud)->rm_xid );

	switch (  ((struct rpc_msg *)ud)->rm_direction) {

		case CALL :  	printf("**CALL**\t%s\n",buf);
				if (! xdr_callmsg(&xdrs, &rp)) {
					printf("%s",beg_line);
					printf ("xdr_callmsg failure\n"); 
					return(1);
				}
				cinfo.ci_reqid 		= xid;
				cinfo.ci_procedure 	= rp.rm_call.cb_proc;
				cinfo.ci_program 	= rp.rm_call.cb_prog;
				cinfo.ci_orighost 	= saddr;
				callsave(&cinfo);
				call_dump(&xdrs,&rp.rm_call);

				/* free up all that malloc'd junk */
				xdrs.x_op=XDR_FREE;
				xdr_callmsg(&xdrs, &rp);
				break;
				
		case REPLY :  	printf("**REPLY**\t%s\n",buf);

				prt_pp(daddr,xid);
				rp.acpted_rply.ar_verf = _null_auth;
				rp.acpted_rply.ar_results.where = 
								(caddr_t) resp;
				rp.acpted_rply.ar_results.proc = 
					r_get_res_proc(XDR_REPLY,xid,daddr);
				reply_dumpproc=
					r_get_res_proc(DUMP_REPLY,xid,daddr);

				if ( ! xdr_replymsg(&xdrs, &rp)) {
					printf("%s",beg_line);
					printf ("xdr_replymsg failure\n"); 
/*					return(1);*/
				}
				reply_dump(&rp.rm_reply);
				if (reply_dumpproc)
					(*reply_dumpproc)(resp);

				/* free up all that malloc'd junk */
				xdrs.x_op=XDR_FREE;
				xdr_replymsg(&xdrs, &rp);
				break;

		default	  :	printf(" NOT VALID RPC PACKET\n");
				hex_dump(ud, cc );
				break;
	}
}


/* dump the rpc reply body */
reply_dump(rb )
struct reply_body *rb;
{
	reset_beg_line(rcp_beg);
	printf("%s",beg_line);
	printf("Reply Stat: ");
	switch (rb->rp_stat) {
		case MSG_ACCEPTED :  	
				printf("MSG_ACCEPTED\n");
				r_accepted_dump(&rb->rp_acpt);
				return(0);
		case MSG_DENIED :  	
				printf("MSG_DENIED\n");
                                r_rejected_dump(&rb->rp_rjct);
				break;
		default	  :	printf("NOT VALID RPC REPLY BODY\n");
				break;
	}
}

r_rejected_dump(rr)
struct rejected_reply *rr;
{
	printf("%s",beg_line);
	printf("Rejected Reply Stat: ");
	switch (rr->rj_stat) {
	case RPC_MISMATCH: 
		printf( " RPC_MISMATCH -- "); 
		printf( " Low 0x%x(%d) ",rr->rj_vers.low,rr->rj_vers.low);
		printf( " High 0x%x(%d) ",rr->rj_vers.high,rr->rj_vers.high);
		break;
	case AUTH_ERROR: 
		printf( " AUTH_ERROR: "); 
		switch (rr->rj_why) {
        	case AUTH_OK: 
			printf(" AUTH_OK ");
			break;
		case AUTH_BADCRED: 
			printf(" (failed at remote end) ");
			printf(" AUTH_BADCRED ");
			break;
		case AUTH_REJECTEDCRED: 
			printf(" (failed at remote end) ");
			printf(" AUTH_REJECTEDCRED ");
			break;
		case AUTH_BADVERF: 
			printf(" (failed at remote end) ");
			printf(" AUTH_BADVERF ");
			break;
		case AUTH_REJECTEDVERF: 
			printf(" (failed at remote end) ");
			printf(" AUTH_REJECTEDVERF ");
			break;
		case AUTH_TOOWEAK: 
			printf(" (failed at remote end) ");
			printf(" AUTH_TOOWEAK ");
			break;
        	case AUTH_FAILED: 
			printf(" (failed locally) ");
			printf(" AUTH_FAILED ");
			break;
        	case AUTH_INVALIDRESP: 
			printf(" (failed locally) ");
			printf(" AUTH_INVALIDRESP ");
			break;
		default:
			printf("Protocol Error- bad rejected reply auth_stat why code");
		}
		break;
	default: 
		printf("Protocol Error- bad rejected reply status code");
	}
	printf("\n");
}


r_accepted_dump(ar)
struct accepted_reply *ar;
{
	printf("%s",beg_line);
	printf("Accepted Reply Stat: ");
	switch (ar->ar_stat) {
	case SUCCESS: 
		printf( " SUCCESS "); 
		break;
	case PROG_UNAVAIL: 
		printf( " PROG_UNAVAIL "); 
		break;
	case PROG_MISMATCH: 
		printf( " PROG_MISMATCH "); 
		break;
	case PROC_UNAVAIL: 
		printf( " PROC_UNAVAIL "); 
		break;
	case GARBAGE_ARGS: 
		printf( " GARBAGE_ARGS "); 
		break;
	case SYSTEM_ERR: 
		printf( " SYSTEM_ERR "); 
		break;
	default: 
		printf("Protocol Error- bad accepted reply status code");
	}
	printf("\n");
}


/* dump the rpc call body 
This routine prints out prog vers procnum and the cred struct.
verf is handled by the xdr routines of the client program.
i.e. c_nfs_dump, (Call NFS DUMP) prints out the nfs structures.

struct call_body {
        u_long cb_rpcvers;      /* must be equal to two 
        u_long cb_prog;
        u_long cb_vers;
        u_long cb_proc;
        struct opaque_auth cb_cred;
        struct opaque_auth cb_verf; /* protocol specific - provided by client 
};
*/

call_dump(xdrs,rmc ,cc)
XDR *xdrs;
struct call_body *rmc;
int cc;
{
	char str[100], buf[MAXPACKET], *pcname, *pgname;
	int	(*xdrproc)()=NULL;
	int	(*dumpproc)()=NULL;

	bzero(buf,MAXPACKET);

	xdrproc = get_proc(XDR_CALL, rmc->cb_prog, rmc->cb_proc);
	dumpproc = get_proc(DUMP_CALL, rmc->cb_prog, rmc->cb_proc);


	pgname=get_prog_name(rmc->cb_prog);

	pcname=get_proc_name(rmc->cb_prog,rmc->cb_proc);

	printf("%s",beg_line);
	printf("Program=%d (%s) Version=%d Procedure=%d (%s)\n",
			rmc->cb_prog, pgname, rmc->cb_vers, 
			rmc->cb_proc, pcname);

	c_oa_dump(&rmc->cb_cred);		/* prt creds */

	if (xdrproc) (*xdrproc)(xdrs,buf);	/* decode it */
	if (dumpproc) (*dumpproc)(buf);		/* prt it */

	if (xdrproc) {
		xdrs->x_op = XDR_FREE;
		(*xdrproc)(xdrs,buf);	/* free it */
	}
}

c_oa_dump_1(oa)
struct opaque_auth *oa;
{
	printf("Opaque Authorization Base %x Opaque Authorization Length %d\n",
		oa->oa_base, oa->oa_length);
	if ((oa->oa_length > 0) && (oa->oa_length < 10000))
		hex_dump(oa->oa_base,oa->oa_length);
}

c_oa_dump(oa)
struct opaque_auth *oa;
{
	XDR xdrs;
	static struct authunix_parms  cred;

	printf("%s",beg_line);
	switch (oa->oa_flavor) {
		case AUTH_NULL: 	
			printf("AUTH_NULL "); 
			c_oa_dump_1(oa);
			break;
		case AUTH_UNIX: 	
			/* We are using our xdrs here instead of the same
			   one that rpc_dump was using because the cred 
			   inside the oa was treated as opaque data.
			   We need to de-opaqify it.
			*/
			printf("AUTH_UNIX \n"); 
			xdrmem_create(&xdrs,oa->oa_base,oa->oa_length,
					XDR_DECODE);
			XDR_SETPOS(&xdrs, 0);
			bzero(&cred,sizeof(struct authunix_parms));

			/* I could have had my own version of this routine
			   which would print out the results upon decode,
			   but i wrote cred_dump instead for no particular
			   reason.  Most of the nfs dumps are within a xdr
			   routine.
			*/
			if (!xdr_authunix_parms(&xdrs, &cred)) {
				printf("Bad Authunix Parameters\n");
				return(FALSE);
			}
			cred_dump(&cred);

			/* free up all that malloc'd junk */
			xdrs.x_op=XDR_FREE;
			xdr_authunix_parms(&xdrs, &cred);
			break;

		case AUTH_SHORT: 	
			printf("AUTH_SHORT "); 
			c_oa_dump_1(oa);
			break;

		case AUTH_DES: 		
			printf("AUTH_DES "); 
			c_oa_dump_1(oa);
			break;

		default: 
			printf("Bad Opaque Attribute Flavor\n");
			printf("%s",beg_line);
			c_oa_dump_1(oa);
	}
}


cred_dump(cred)
struct authunix_parms  *cred;
{
	int i;

	printf("%s",beg_line);
	printf("Cred:\n");

	printf("%s",beg_line);
	printf("\tTime=0x%x (",cred->aup_time);
	prt_time(cred->aup_time);
	printf(")\n");

	printf("%s",beg_line);
	printf("\tMachine=%s ", cred->aup_machname);
	printf("Uid=%d Gid=%d Group List Length=%d \n", 	
						cred->aup_uid,
						cred->aup_gid,
						cred->aup_len);

	printf("%s",beg_line);
	printf("\tGroups= ( ");
	for ( i=0 ; i < cred->aup_len; i++) 
		printf("%d ",cred->aup_gids[i]);
	printf(")\n");

	return(cred->aup_machname);

}

/*  copy whatever is left in the xdr buffer, starting w/ a counter. */
xdr_hexcopy(xdrs,results)
XDR *xdrs;
char *results;
{
	int cc;

	cc=xdrs->x_handy;

	*((int *)results) = cc;

	if ((xdrs->x_op == XDR_DECODE) && (cc > 0)) {
			bcopy( xdr_inline(xdrs,0),(results+sizeof(cc)),cc);
	}
	return(TRUE);
}

/* This goes w/ xdr_hexcopy above */
hc_hexdump(results)
char *results;
{
	int cc;

	cc = *((int *)results);
	hex_dump((results+sizeof(cc)),cc);
}
	
xdr_left(xdrs)
XDR *xdrs;
{
	return( xdrs->x_handy );
}

/*  dump whatever is left in the xdr buffer */
xdr_hexdump(xdrs,results)
XDR *xdrs;
char *results;
{
	int cc;

	cc=xdrs->x_handy;

	if ((xdrs->x_op == XDR_DECODE) && (cc > 0)) {
			hex_dump(xdr_inline(xdrs,0),cc);
			bcopy( xdr_inline(xdrs,0),results,cc);
	}
}
		

/* 
   	Returns pointer to xdrresults procedure for the reply packet.   
*/
int (*r_get_res_proc(which, xid, addr))()
int which;
int xid;
u_long addr;
{
	struct cinfo *ci;
	int (*proc)();

	/* If we never saw the call for this reply, (probably because it 
	   went by before iptrace was turned on,) then we have to do
	   a hexdump 
	*/
	if ( ! (ci=getci(addr,xid))) 
		goto beatit;

	/* If we don't have a routine for this prog/proc combo, then
	   we have to do a hexdump also.
	*/
	if (proc = get_proc(which,ci->ci_program,ci->ci_procedure))
		return (proc);

beatit:
	if ((which== XDR_REPLY) || (which== XDR_CALL)) 
		return(xdr_hexcopy);
	else
		return(hc_hexdump);
}
