static char sccsid[] = "@(#)09	1.3  src/tcpip/usr/lib/libisode/ts2bsd.c, isodelib7, tcpip411, GOLD410 4/5/93 17:15:17";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: _ts2bsd_stub gen2tp4 gen2tp4X join_tp4_client retry_tp4_socket
 *    sendCmsg start_tp4_server tp42gen tp42genX tp4getCmsg tp4init tp4open 
 *    tp4restore tp4save
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ts2bsd.c
 */

/* ts2bsd.c - TPM: 4.4BSD OSI TP4 interface */
#define STATIC /**/

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ts2bsd.c,v 1.2 93/02/05 17:13:50 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ts2bsd.c,v 1.2 93/02/05 17:13:50 snmp Exp $
 *
 *
 * $Log:	ts2bsd.c,v $
 * Revision 1.2  93/02/05  17:13:50  snmp
 * ANSI - D67743
 * 
 * Revision 7.12  91/03/09  11:58:13  mrose
 * update
 * 
 * Revision 7.11  91/02/22  09:47:17  mrose
 * Interim 6.8
 * 
 * Revision 7.10  91/01/14  13:34:23  mrose
 * loader
 * 
 * Revision 7.9  90/07/09  14:51:13  mrose
 * sync
 * 
 * Revision 7.8  90/03/23  17:31:16  mrose
 * 8
 * 
 * Revision 7.7  90/03/22  08:38:02  mrose
 * touch-up
 * 
 * Revision 7.6  89/12/19  10:18:28  mrose
 * DLOG
 * 
 * Revision 7.5  89/12/18  17:49:48  mrose
 * touch-up
 * 
 * Revision 7.4  89/12/08  16:26:47  mrose
 * keith
 * 
 * Revision 7.1  89/12/01  10:46:26  mrose
 * touch-up
 * 
 * Revision 7.0  89/11/23  22:30:36  mrose
 * Release 6.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include <signal.h>
#include <isode/tpkt.h>

#ifdef	TP4
#include <isode/tp4.h>
#endif

#ifdef	BSD_TP4
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <isode/tailor.h>

/*#define	MAXTP4		8192	/* until we have a dynamic estimate... */
#define	MAXTP4		1024	/* until we have a dynamic estimate... */
#define	TP4SLOP		  12	/* estimate of largest DT PCI */

/*    DATA */

STATIC struct msghdr msgs;
STATIC union osi_control_msg ocm;


extern int  errno;

/* Ancillary routines */
STATIC int sendCmsg(fd, cc, type, data)
int	fd;
int	cc;
int	type;
char   *data;
{
    int	    result;
    register struct msghdr *msg = &msgs;
    register union osi_control_msg *oc = &ocm;

    bzero ((char *) msg, sizeof *msg);
    msg -> msg_control = oc -> ocm_data;

    bzero ((char *) oc, sizeof *oc);
    oc -> ocm_control.ocm_cmhdr.cmsg_level = SOL_TRANSPORT;
    oc -> ocm_control.ocm_cmhdr.cmsg_type = type;
    oc -> ocm_control.ocm_cmhdr.cmsg_len = sizeof oc -> ocm_control.ocm_cmhdr;

    if (cc) {
	bcopy (data, oc -> ocm_control.ocm_cmdata, cc);
	oc -> ocm_control.ocm_cmhdr.cmsg_len += cc;
    }
    msg -> msg_controllen = oc -> ocm_control.ocm_cmhdr.cmsg_len;

    return sendmsg (fd, msg, 0);
}

int tp4getCmsg(fd, cc, type, data)
int	fd;
int	*cc;
int	*type;
char	*data;
{
    int	    result;
    register struct msghdr *msg = &msgs;
    register union osi_control_msg *oc = &ocm;

    bzero ((char *) msg, sizeof *msg);
    msg -> msg_control = oc -> ocm_data;
    msg -> msg_controllen = sizeof oc -> ocm_data;

    bzero ((char *) oc, sizeof *oc);

    result = recvmsg (fd, msg, 0);
    if (result >= 0 && cc && type && data)  {
	register int n =
		(oc -> ocm_control.ocm_cmhdr.cmsg_len
				- sizeof (oc -> ocm_control.ocm_cmhdr));
	if (n > 0) {
		*type = oc-> ocm_control.ocm_cmhdr.cmsg_type;
		if (n > *cc)
			n = *cc;
		bcopy(oc -> ocm_control.ocm_cmdata, data, n);
		*cc = n;
	}
    }
    return (result);
}

/*    UPPER HALF */

STATIC int  TConnect (tb, expedited, data, cc, td)
register struct tsapblk *tb;
char    *data;
int	expedited,
	cc;
struct TSAPdisconnect *td;
{
    int	    len;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;
    struct tp_conn_param tcp;
    register struct tp_conn_param *p = &tcp;

    if (gen2tp4X (&tb -> tb_responding, &sock) == NOTOK)
	return tsaplose (td, DR_ADDRESS, NULLCP,
			 "unable to parse remote address");

    len = sizeof *p;
    if (getsockopt (tb -> tb_fd, SOL_TRANSPORT, TPOPT_PARAMS, (char *) p, &len)
	    == NOTOK)
	SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_PARAMS", ("unable to get"));
    else {
	if (p -> p_xpd_service ? !expedited : expedited) {
	    p -> p_xpd_service = expedited ? 1 : 0;
	    if (setsockopt (tb -> tb_fd, SOL_TRANSPORT, TPOPT_PARAMS,
			    (char *) p, sizeof *p) == NOTOK)
		SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_PARAMS",
		      ("unable to set"));
	}

	if (expedited)
	    tb -> tb_flags |= TB_EXPD;
	if (p -> p_tpdusize > 0) {
	    if (p -> p_tpdusize > 10)
		p -> p_tpdusize = 10;
	    tb -> tb_tpduslop = TP4SLOP;
	    tb -> tb_tsdusize = (1 << p -> p_tpdusize) - tb -> tb_tpduslop;
	}
    }

    if (data &&
		sendCmsg(tb->tb_fd, data, cc, TPOPT_CONN_DATA) == NOTOK)
	return tsaplose (td, DR_CONGEST, "TPOPT_CONN_DATA", "unable to send");

/*
   this is a real hack to pass information between TConnect and TRetry:
	if tb_srcref is 0xffff:    this indicates that the connect is still
				   in progress

	if tb_srcref is 0xfffe:	   this indicates the connect is done

	otherwise:		   this indicates a TS error
 */

    if (connect (tb -> tb_fd, (struct sockaddr *) ifaddr, ifaddr -> siso_len)
		== NOTOK) {
	if (errno == EINPROGRESS) {
	    tb -> tb_srcref = 0xffff;
	    return CONNECTING_1;
	}

	if (!(errno & TP_ERROR_MASK))
	    return tsaplose (td, DR_REFUSED, "connection",
			     "unable to establish");

	tb -> tb_srcref = (errno & ~TP_ERROR_MASK) & 0xff;
    }
    else
	tb -> tb_srcref = 0xfffe;

    return DONE;
}

/*  */

STATIC int  TRetry (tb, async, tc, td)
register struct tsapblk *tb;
int	async;
struct TSAPconnect *tc;
struct TSAPdisconnect *td;
{
    int	    len,
	    onoff,
	    flags,
	    reason;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;
    struct tp_conn_param tcp;
    int cmsgtype = 0;
    register struct tp_conn_param *p = &tcp;

    switch (tb -> tb_srcref) {
	case 0xfffe:
	    reason = NOTOK;
	    break;
	
	default:
	    reason = tb -> tb_srcref;

	case 0xffff:
	    if (async)
		switch ((*tb -> tb_retryfnx) (tb, td)) {
	            case NOTOK:
		        goto out;

		    case OK:
			return CONNECTING_1;

		    case DONE:
			break;
		}

	    (void) gen2tp4X (&tb -> tb_responding, &sock);
	    if (connect (tb -> tb_fd, (struct sockaddr *) ifaddr,
			 ifaddr -> siso_len) == NOTOK)
		switch (errno) {
	            case EINPROGRESS:
	                return CONNECTING_1;

		    case EISCONN:
			reason = NOTOK;
			break;

		    default:
			/* for the moment, u.u_error limited to one byte */
			/* this will be fixed real soon now. */
			/* if (!(errno & TP_ERROR_MASK))  */
			if (errno > 0) {
			    (void) tsaplose (td, DR_REFUSED, "connection",
					     "unable to establish");
			    goto out;
			}
			reason = (errno & ~TP_ERROR_MASK) & 0xff;
			break;
		}
	    break;
    }


    if (async)
	(void) ioctl (tb -> tb_fd, FIONBIO, (onoff = 0, (char *) &onoff));

    if (reason == NOTOK) {
	tc -> tc_sd = tb -> tb_fd;
	tc -> tc_tsdusize = tb -> tb_tsdusize = MAXTP4;

	len = sizeof sock;
	if (getsockname (tb -> tb_fd, (struct sockaddr *) ifaddr, &len)
	        != NOTOK) {
	    ifaddr -> siso_len = len;
	    (void) tp42genX (&tb -> tb_responding, &sock);
	}
	else
	    SLOG (tsap_log, LLOG_EXCEPTIONS, "failed", ("getpeername"));
	copyTSAPaddrX (&tb -> tb_responding, &tc -> tc_responding);

	len = sizeof *p;
	if (getsockopt (tb -> tb_fd, SOL_TRANSPORT, TPOPT_PARAMS, (char *) p,
			&len)== NOTOK)
	    SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_PARAMS",
		  ("unable to get"));
	else {
	    if (!p -> p_xpd_service)
		tb -> tb_flags &= ~TB_EXPD;

	    if (p -> p_tpdusize > 0) {
		if (p -> p_tpdusize > 10)
		    p -> p_tpdusize = 10;
		tb -> tb_tpduslop = TP4SLOP;
		tb -> tb_tsdusize = (1 << p -> p_tpdusize) - tb -> tb_tpduslop;
	    }
	}
	tc -> tc_expedited = (tb -> tb_flags & TB_EXPD) ? 1 : 0;
	tc -> tc_cc = sizeof tc -> tc_data;
	if (tp4getCmsg(tb->tb_fd, &cmsgtype, &tc->tc_cc, tc->tc_data) < 0) {
	    tc -> tc_cc = 0;
	} else if (cmsgtype != TPOPT_CFRM_DATA)
	    tc -> tc_cc = 0;

	tb -> tb_flags |= TB_CONN;
#ifdef  MGMT
	if (tb -> tb_manfnx)
	    (*tb -> tb_manfnx) (OPREQOUT, tb);
#endif
	if (tb -> tb_calling)
	    free ((char *) tb -> tb_calling), tb -> tb_calling = NULL;
	if (tb -> tb_called)
	    free ((char *) tb -> tb_called), tb -> tb_called = NULL;

	return DONE;
    }

    td -> td_reason = reason;
    td -> td_cc = sizeof td -> td_data;
    if (tp4getCmsg(tb->tb_fd, &cmsgtype, &td->td_cc, td->td_data) < 0)
	td -> td_cc = 0;
    else if (cmsgtype != TPOPT_DISC_DATA)
	td -> td_cc = 0;

out: ;
    freetblk (tb);

    return NOTOK;
}

/*  */

STATIC int  TStart (tb, cp, ts, td)
register struct tsapblk *tb;
char   *cp;
struct TSAPstart *ts;
struct TSAPdisconnect *td;
{
    int	    i,
	    len;
    struct tp_conn_param tcp;
    register struct tp_conn_param *p = &tcp;

    len = sizeof *p;
    if (getsockopt (tb -> tb_fd, SOL_TRANSPORT, TPOPT_PARAMS, (char *) p, &len)
	    == NOTOK)
	SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_PARAMS", ("unable to get"));
    else {
	if (p -> p_xpd_service)
	    tb -> tb_flags |= TB_EXPD;
	if (p -> p_tpdusize > 0) {
	    if (p -> p_tpdusize > 10)
		p -> p_tpdusize = 10;
	    tb -> tb_tpduslop = TP4SLOP;
	    tb -> tb_tsdusize = (1 << p -> p_tpdusize) - tb -> tb_tpduslop;
	}
    }

    ts -> ts_sd = tb -> tb_fd;
    copyTSAPaddrX (&tb -> tb_initiating, &ts -> ts_calling);
    copyTSAPaddrX (&tb -> tb_responding, &ts -> ts_called);
    ts -> ts_expedited = (tb -> tb_flags & TB_EXPD) ? 1 : 0;
    ts -> ts_tsdusize = tb -> tb_tsdusize;

    if ((i = strlen (cp)) > 0) {
	if (i > 2 * TS_SIZE)
	    return tsaplose (td, DR_CONNECT, NULLCP,
			     "too much initial user data");

	ts -> ts_cc = implode ((u_char *) ts -> ts_data, cp, i);
    }
    else
	ts -> ts_cc = 0;

    return OK;
}

/*  */

/* ARGSUSED */

STATIC int  TAccept (tb, responding, data, cc, qos, td)
register struct tsapblk *tb;
char   *data;
int	responding,
	cc;
struct QOStype *qos;
struct TSAPdisconnect *td;
{
    int	    len;
    struct tp_conn_param tcp;
    register struct tp_conn_param *p = &tcp;

    len = sizeof *p;
    if (getsockopt (tb -> tb_fd, SOL_TRANSPORT, TPOPT_PARAMS, (char *) p, &len)
	    == NOTOK)
	SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_PARAMS", ("unable to get"));
    else {
	if (!p -> p_xpd_service)
	    tb -> tb_flags &= ~TB_EXPD;
	else
	    if (!(tb -> tb_flags & TB_EXPD)) {
		p -> p_xpd_service = 0;
		if (setsockopt (tb -> tb_fd, SOL_TRANSPORT, TPOPT_PARAMS,
				(char *) p, sizeof *p) == NOTOK)
		    SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_PARAMS",
			  ("unable to set"));
	    }

	if (p -> p_tpdusize > 0) {
	    if (p -> p_tpdusize > 10)
		p -> p_tpdusize = 10;
	    tb -> tb_tpduslop = TP4SLOP;
	    tb -> tb_tsdusize = (1 << p -> p_tpdusize) - tb -> tb_tpduslop;
	}
    }

    if (sendCmsg (tb -> tb_fd, cc, TPOPT_CFRM_DATA, data) == NOTOK)
	return tsaplose (td, DR_CONGEST, "TPOPT_CFRM_DATA", "unable to send");

    tb -> tb_flags |= TB_CONN;
#ifdef  MGMT
    if (tb -> tb_manfnx)
	(*tb -> tb_manfnx) (OPREQIN, tb);
#endif

    return OK;
}

/*  */

/* life would be nice if we didn't have to worry about the maximum number of
   bytes that can be written in a single syscall() */

#ifndef	MSG_MAXIOVLEN
#define	MSG_MAXIOVLEN	NTPUV
#endif


STATIC int  TWrite (tb, uv, expedited, td)
register struct tsapblk *tb;
register struct udvec *uv;
int	expedited;
struct TSAPdisconnect *td;
{
    register int cc;
    int	    flags,
	    j,
	    len;
#ifdef	MGMT
    int	    dlen;
#endif
    register char *bp,
		  *ep;
    register struct qbuf *qb;
    register struct msghdr *msg = &msgs;
    struct iovec iovs[MSG_MAXIOVLEN];
    register struct iovec *vv,
			  *wv;
    SFP	    pstat;

    bzero ((char *) msg, sizeof *msg);

    flags = expedited ? MSG_OOB : 0;
    
#ifdef	MGMT
    dlen = 0;
#endif

    if (!expedited && (tb -> tb_flags & TB_QWRITES)) {
	int	onoff,
		nc;
	struct udvec *xv;

	cc = 0;
	for (xv = uv; xv -> uv_base; xv++)
	    cc += xv -> uv_len;
#ifdef	MGMT
	dlen = cc;
#endif

	if ((qb = (struct qbuf *) malloc (sizeof *qb + (unsigned) cc))
		== NULL) {
	    (void) tsaplose (td, DR_CONGEST, NULLCP,
			     "unable to malloc %d octets for pseudo-writev, failing...",
			     cc);
	    freetblk (tb);

	    return NOTOK;
	}
	qb -> qb_forw = qb -> qb_back = qb;
	qb -> qb_data = qb -> qb_base, qb -> qb_len = cc;

	bp = qb -> qb_data;
	for (xv = uv; xv -> uv_base; xv++) {
	    bcopy (xv -> uv_base, bp, xv -> uv_len);
	    bp += xv -> uv_len;
	}

	if (tb -> tb_qwrites.qb_forw != &tb -> tb_qwrites) {
	    nc = 0;
	    goto insert;
	}

	vv = iovs;
	vv -> iov_base = qb -> qb_data, vv -> iov_len = qb -> qb_len;
	vv++;

	msg -> msg_iov = iovs;
	msg -> msg_iovlen = vv - iovs;

	pstat = signal (SIGPIPE, SIG_IGN);
	(void) ioctl (tb -> tb_fd, FIONBIO, (onoff = 1, (char *) &onoff));

	nc = sendmsg (tb -> tb_fd, msg, MSG_EOR);

	(void) ioctl (tb -> tb_fd, FIONBIO, (onoff = 0, (char *) &onoff));
	(void) signal (SIGPIPE, pstat);

	if (nc != cc) {
	    if (nc == NOTOK) {
		if (errno != EWOULDBLOCK) {
		    (void) tsaplose (td, DR_CONGEST, "failed", "sendmsg");
		    goto losing;
		}

		nc = 0;
	    }
	    if ((*tb -> tb_queuePfnx) (tb, 1, td) == NOTOK)
		goto losing;

	    qb -> qb_data += nc, qb -> qb_len -= nc;
insert: ;
	    insque (qb, tb -> tb_qwrites.qb_back);
	    DLOG (tsap_log, LLOG_TRACE,
		  ("queueing blocked write of %d of %d octets", nc, cc));
	}
	else
	    free ((char *) qb);
	goto done;

losing: ;
	free ((char *) qb);
	freetblk (tb);

	return NOTOK;
    }

    pstat = signal (SIGPIPE, SIG_IGN);

    ep = (bp = uv -> uv_base) + (cc = uv -> uv_len);
    while (uv -> uv_base) {
	wv = (vv = iovs) + MSG_MAXIOVLEN;
	for (len = tb -> tb_tsdusize; len > 0 && vv < wv; len -= j) {
	    j = min (cc, len);
#ifdef	MGMT
	    dlen += j;
#endif
	    vv -> iov_base = bp, vv -> iov_len = j, vv++;
	    bp += j, cc -= j;

	    if (bp >= ep) {
		if ((bp = (++uv) -> uv_base) == NULL)
		    break;
		ep = bp + (cc = uv -> uv_len);
	    }
	}

	if (expedited || uv -> uv_base == NULL)
	    flags |= MSG_EOR;

	msg -> msg_iov = iovs;
	msg -> msg_iovlen = vv - iovs;

	if (sendmsg (tb -> tb_fd, msg, flags) == NOTOK) {
	    (void) tsaplose (td, DR_CONGEST, "failed", "sendmsg");
	    freetblk (tb);

	    (void) signal (SIGPIPE, pstat);
	    return NOTOK;
	}
    }

    (void) signal (SIGPIPE, pstat);

done: ;
#ifdef  MGMT
    if (tb -> tb_manfnx)
	(*tb -> tb_manfnx) (USERDT, tb, dlen);
#endif

    return OK;
}

/*  */

STATIC int  TDrain (tb, td)
register struct tsapblk *tb;
struct TSAPdisconnect *td;
{
    int	    nc,
	    onoff,
	    result;
    register struct qbuf *qb;
    register struct msghdr *msg = &msgs;
    struct iovec vvs;
    register struct iovec *vv = &vvs;
    SFP	    pstat;
    SBV	    smask;

    bzero ((char *) msg, sizeof *msg);
    msg -> msg_iov = vv, msg -> msg_iovlen = 1;

    pstat = signal (SIGPIPE, SIG_IGN);
    smask = sigioblock ();

    (void) ioctl (tb -> tb_fd, FIONBIO, (onoff = 1, (char *) &onoff));

    while ((qb = tb -> tb_qwrites.qb_forw) != &tb -> tb_qwrites) {
	vv -> iov_base = qb -> qb_data, vv -> iov_len = qb -> qb_len;

	if (nc = sendmsg (tb -> tb_fd, msg, MSG_EOR) != qb -> qb_len) {
	    if (nc == NOTOK) {
		if (errno != EWOULDBLOCK) {
		    result = tsaplose (td, DR_NETWORK, "failed",
				      "write to network");
		    goto out;
		}

		nc = 0;
	    }

	    qb -> qb_data += nc, qb -> qb_len -= nc;
	    DLOG (tsap_log, LLOG_TRACE,
		  ("wrote %d of %d octets from blocked write", nc,
		   qb -> qb_len));

	    result = OK;
	    goto out;
	}

	DLOG (tsap_log, LLOG_TRACE,
	      ("finished blocked write of %d octets", qb -> qb_len));
	remque (qb);
	free ((char *) qb);
    }
    result = DONE;

out: ;
    (void) ioctl (tb -> tb_fd, FIONBIO, (onoff = 0, (char *) &onoff));

    (void) sigiomask (smask);
    (void) signal (SIGPIPE, pstat);

    return result;
}

/*  */

/* ARGSUSED */

STATIC int  TRead (tb, tx, td, async, oob)
register struct tsapblk *tb;
register struct TSAPdata *tx;
struct TSAPdisconnect *td;
int	async,
	oob;
{
    int	    cc;
    register struct qbuf *qb;
    register struct msghdr *msg = &msgs;
    register union osi_control_msg *oc = &ocm;
    struct iovec iovs[1];
    static struct qbuf *spare_qb = 0;

    bzero ((char *) tx, sizeof *tx);
    tx -> tx_qbuf.qb_forw = tx -> tx_qbuf.qb_back = &tx -> tx_qbuf;

    for (;;) {
	qb = NULL;
	if (spare_qb) {
		if (spare_qb -> qb_len >= tb -> tb_tsdusize)
			qb = spare_qb;
		else
			free ((char *)spare_qb);
		spare_qb = NULL;
	}
	if (qb == NULL && (qb = (struct qbuf *) malloc ((unsigned) (sizeof *qb
					  	     + tb -> tb_tsdusize)))
	        == NULL) {
	    (void) tsaplose (td, DR_CONGEST, NULLCP, NULLCP);
	    break;
	}
	qb -> qb_data = qb -> qb_base;

	bzero ((char *) msg, sizeof *msg);
	msg -> msg_iov = iovs;
	msg -> msg_iovlen = 1;
	msg -> msg_control = oc -> ocm_data;
	msg -> msg_controllen = sizeof oc -> ocm_data;

	iovs[0].iov_base = qb -> qb_data;
	iovs[0].iov_len = tb -> tb_tsdusize;

	bzero ((char *) oc, sizeof *oc);

	if ((cc = recvmsg(tb -> tb_fd, msg, 0)) == NOTOK) {
	/*if ((cc = recvmsg(tb -> tb_fd, msg, oob ? MSG_OOB : 0)) == NOTOK) */
	    if (!(errno & TP_ERROR_MASK)) {
		(void) tsaplose (td, DR_CONGEST, "failed", "recvfrom");
		break;
	    }

	    if ((td -> td_reason = errno & ~TP_ERROR_MASK) != DR_NORMAL)
		SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
		      ("TP error %d", td -> td_reason));
	    td -> td_cc = 0;
	    break;
	}

	if (msg -> msg_controllen) {
	    if (msg -> msg_controllen < sizeof oc -> ocm_control.ocm_cmhdr) {
		(void) tsaplose (td, DR_CONGEST, NULLCP,
		    "truncated control message, got %d expecting at least %d",
				 msg -> msg_controllen,
				 sizeof oc -> ocm_control.ocm_cmhdr);
		break;
	    }

	    if (oc -> ocm_control.ocm_cmhdr.cmsg_level != SOL_TRANSPORT) {
		(void) tsaplose (td, DR_CONGEST, NULLCP,
				 "unexpected message (level 0x%x, type 0x%x)",
				 oc -> ocm_control.ocm_cmhdr.cmsg_level,
				 oc -> ocm_control.ocm_cmhdr.cmsg_type);
		break;
	    }
	    if (oc -> ocm_control.ocm_cmhdr.cmsg_type == TPOPT_DISC_DATA) {
		td -> td_reason = DR_NORMAL;
		if ((td -> td_cc = oc -> ocm_control.ocm_cmhdr.cmsg_len
				    - sizeof oc -> ocm_control.ocm_cmhdr) > 0)
		    bcopy (oc -> ocm_control.ocm_cmdata, td -> td_data, cc);
		break;
	    }
	}

	if (msg -> msg_flags & MSG_OOB) {
	    if (cc > 0) {
		insque (qb, tx -> tx_qbuf.qb_back);
		tx -> tx_cc = (qb -> qb_len = cc);
	    }
	    else
		free ((char *) qb);
	    tx -> tx_expedited = 1;

	    return OK;
	}

	tb -> tb_len += (qb -> qb_len = cc);
	if (cc > 0) {
	    register struct qbuf *qb2 = tb -> tb_qbuf.qb_back;

	    if (qb2 != &tb->tb_qbuf && qb2->qb_len + cc <= tb->tb_tsdusize) {
		bcopy(qb -> qb_data, qb2 -> qb_len + qb2 -> qb_data, cc);
		qb2 -> qb_len += cc;
		(spare_qb = qb) -> qb_len = tb -> tb_tsdusize;
	    } else
		insque (qb, qb2);
	} else
	    free ((char *) qb);

#ifdef	MGMT
	if (tb -> tb_manfnx)
	    (*tb -> tb_manfnx) (USERDR, tb, tb -> tb_len);
#endif
	if (!(msg -> msg_flags & MSG_EOR)) {
	    if (async) {
		register struct qbuf *qb2 = tb -> tb_qbuf.qb_back;
		if (qb2 != &tb->tb_qbuf && qb2->qb_len <= tb->tb_tsdusize)
			printf("short return %d\n", qb2->qb_len);
		return DONE;
	    }
	    continue;
	}
	tx -> tx_expedited = 0;
	if (tb -> tb_qbuf.qb_forw != &tb -> tb_qbuf) {
	    tx -> tx_qbuf = tb -> tb_qbuf;	/* struct copy */
	    tx -> tx_qbuf.qb_forw -> qb_back =
		tx -> tx_qbuf.qb_back -> qb_forw = &tx -> tx_qbuf;
	    tx -> tx_cc = tb -> tb_len;
	    tb -> tb_qbuf.qb_forw =
		tb -> tb_qbuf.qb_back = &tb -> tb_qbuf;
	    tb -> tb_len = 0;
	}

	{
	    register struct qbuf *qb2 = tb -> tb_qbuf.qb_back;
	    if (qb2 != &tb->tb_qbuf && qb2->qb_len <= tb->tb_tsdusize)
		    printf("short return %d\n", qb2->qb_len);
	}
	return OK;
    }
    if (qb)
	free ((char *) qb);

    freetblk (tb);

    return NOTOK;
}

/*  */

STATIC int  TDisconnect (tb, data, cc, td)
register struct tsapblk *tb;
char   *data;
int	cc;
struct TSAPdisconnect *td;
{
    int	    result;

    if (sendCmsg(tb -> tb_fd, cc, TPOPT_DISC_DATA, data) == NOTOK)
	result = tsaplose (td, DR_CONGEST, "TPOPT_DISC_DATA",
			   "unable to send");
    else
	result = OK;

    freetblk (tb);

    return result;
}
/*  */

/* ARGSUSED */

STATIC int  TLose (tb, reason, td)
register struct tsapblk *tb;
int	reason;
struct TSAPdisconnect *td;
{
    register struct msghdr *msg = &msgs;
    register union osi_control_msg *oc = &ocm;

    SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP, ("TPM error %d", reason));

    bzero ((char *) msg, sizeof *msg);
    msg -> msg_control = oc -> ocm_data;
    msg -> msg_controllen = sizeof oc -> ocm_data;

    bzero ((char *) oc, sizeof *oc);
    oc -> ocm_control.ocm_cmhdr.cmsg_level = SOL_TRANSPORT;
    oc -> ocm_control.ocm_cmhdr.cmsg_type = TPOPT_DISC_DATA;
    oc -> ocm_control.ocm_cmhdr.cmsg_len = sizeof oc -> ocm_control.ocm_cmhdr;

    if (sendmsg (tb -> tb_fd, msg, 0) == NOTOK)
	SLOG (tsap_log, LLOG_EXCEPTIONS, "TPOPT_DISC_DATA",
	      ("unable to send"));
}

/*    LOWER HALF */

/* ARGSUSED */

int	tp4open (tb, local_ta, local_na, remote_ta, remote_na, td, async)
register struct tsapblk *tb;
struct TSAPaddr *local_ta,
		*remote_ta;
struct NSAPaddr *local_na,
		*remote_na;
struct TSAPdisconnect *td;
int	async;
{
    int	    fd,
	    onoff;
    struct TSAPaddr tzs;
    register struct TSAPaddr *tz = &tzs;
    register struct NSAPaddr *nz = tz -> ta_addrs;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;

    bzero ((char *) tz, sizeof *tz);
    if (local_ta)
	*tz = *local_ta;	/* struct copy */
    if (local_na) {
	*nz = *local_na;	/* struct copy */
	tz -> ta_naddr = 1;
    }

    (void) gen2tp4 (tz, &sock);

    if ((fd = socket (AF_ISO, SOCK_SEQPACKET, 0)) == NOTOK)
	return tsaplose (td, DR_CONGEST, "socket", "unable to start");

    if (ifaddr->siso_nlen || ifaddr->siso_tlen)
       if (bind (fd, (struct sockaddr *) ifaddr, ifaddr -> siso_len) == NOTOK) {
	(void) tsaplose (td, DR_ADDRESS, "socket", "unable to bind");
	(void) close (fd);
	return NOTOK;
    }

    tb -> tb_fd = fd;
    (void) tp4init (tb);

    if (async)
	(void) ioctl (fd, FIONBIO, (onoff = 1, (char *) &onoff));

    return (async ? OK : DONE);
}

/*  */

/* ARGSUSED */

STATIC int  retry_tp4_socket (tb, td)
register struct tsapblk *tb;
struct TSAPdisconnect *td;
{
    fd_set  mask;

    FD_ZERO (&mask);
    FD_SET (tb -> tb_fd, &mask);
    if (xselect (tb -> tb_fd + 1, NULLFD, &mask, NULLFD, 0) < 1)
	return OK;

    return DONE;
}

/*  */

/* ARGSUSED */

char   *tp4save (fd, td)
int fd;
struct TSAPdisconnect *td;
{
    static char buffer[BUFSIZ];

    (void) sprintf (buffer, "%c%d", NT_BSD, fd);
    return buffer;
}

/*  */

int	tp4restore (tb, buffer, td)
register struct tsapblk *tb;
char   *buffer;
struct TSAPdisconnect *td;
{
    int	    fd, len, ucdlen;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;

    if (sscanf (buffer, "%d", &fd) != 1 || fd < 0)
	return tsaplose (td, DR_PARAMETER, NULLCP,
			"bad initialization vector \"%s\"", buffer);

    tb -> tb_fd = fd;
    (void) tp4init (tb);
    len = sizeof sock;
    if (getsockname (fd, (struct sockaddr *) ifaddr, &len) == NOTOK) {
	(void) tsaplose (td, DR_CONGEST, "listen",
			 "getsockname failed for tp4restore");
	(void) close (fd);
	return NOTOK;
    }
    ifaddr -> siso_len = len;
    (void) tp42genX (&tb -> tb_responding, &sock);
    len = sizeof sock;
    if (getpeername (fd, (struct sockaddr *) ifaddr, &len) == NOTOK) {
	(void) tsaplose (td, DR_CONGEST, "listen",
			 "getpeername failed for tp4restore");
	(void) close (fd);
	return NOTOK;
    }
    ifaddr -> siso_len = len;
    (void) tp42genX (&tb -> tb_initiating, &sock);
    return OK;
}

/*  */

int	tp4init (tb)
register struct tsapblk *tb;
{
    tb -> tb_connPfnx = TConnect;
    tb -> tb_retryPfnx = TRetry;

    tb -> tb_startPfnx = TStart;
    tb -> tb_acceptPfnx = TAccept;

    tb -> tb_writePfnx = TWrite;
    tb -> tb_readPfnx = TRead;
    tb -> tb_discPfnx = TDisconnect;
    tb -> tb_losePfnx = TLose;

    tb -> tb_drainPfnx = TDrain;

#ifdef  MGMT
    tb -> tb_manfnx = TManGen;
#endif

    tb -> tb_flags |= TB_TP4;

    tb -> tb_tsdusize = MAXTP4 - (tb -> tb_tpduslop = 0);

    tb -> tb_retryfnx = retry_tp4_socket;

    tb -> tb_closefnx = close_tp4_socket;
    tb -> tb_selectfnx = select_tp4_socket;
}

/*  */

/* ARGSUSED */

int	start_tp4_server (local_ta, backlog, opt1, opt2, td)
struct TSAPaddr *local_ta;
int	backlog,
	opt1,
	opt2;
struct TSAPdisconnect *td;
{
    int	    sd,
	    onoff;
    register struct NSAPaddr *na = local_ta -> ta_addrs;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;

    (void) gen2tp4 (local_ta, &sock);

    if ((sd = socket (AF_ISO, SOCK_SEQPACKET, 0)) == NOTOK)
	return tsaplose (td, DR_CONGEST, "socket", "unable to start");

    if (bind (sd, (struct sockaddr *) ifaddr, ifaddr -> siso_len) == NOTOK) {
	(void) tsaplose (td, DR_ADDRESS, "socket", "unable to bind");
	(void) close (sd);
	return NOTOK;
    }

    if (na -> na_addrlen == 0) {	/* unique listen */
	int	len;

	len = sizeof sock;
	if (getsockname (sd, (struct sockaddr *) ifaddr, &len) == NOTOK) {
	    (void) tsaplose (td, DR_CONGEST, "listen",
			     "getsockname failed for unique");
	    (void) close (sd);
	    return NOTOK;
	}
	ifaddr -> siso_len = len;

	(void) tp42gen (local_ta, &sock);
    }

    if (opt1)
	(void) setsockopt (sd, SOL_SOCKET, opt1, NULLCP, 0);
    if (opt2)
	(void) setsockopt (sd, SOL_SOCKET, opt2, NULLCP, 0);
    
    onoff = 1;

    (void) listen (sd, backlog);

    return sd;
}

/*  */

int	join_tp4_client (fd, remote_ta, td)
int	fd;
struct TSAPaddr *remote_ta;
struct TSAPdisconnect *td;
{
    int	    len,
	    sd;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;

    len = sizeof sock;
    if ((sd = accept (fd, (struct sockaddr *) ifaddr, &len)) == NOTOK)
	return tsaplose (td, DR_NETWORK, "socket", "unable to accept");
    ifaddr -> siso_len = len;
	
    (void) tp42gen (remote_ta, &sock);
    
    return sd;
}

/*  */

STATIC	int  gen2tp4 (generic, specific)
struct TSAPaddr *generic;
union sockaddr_osi *specific;
{
    register char *cp;
    struct sockaddr_iso	*ifaddr = &specific -> osi_sockaddr;

    bzero ((char *) specific, sizeof *specific);

    cp = ifaddr -> siso_data;

    if (generic -> ta_naddr > 0) {
	register struct NSAPaddr *na = generic -> ta_addrs;

	if (ifaddr -> siso_nlen = na -> na_addrlen) {
	    bcopy (na -> na_address, cp, na -> na_addrlen);
	    cp += na -> na_addrlen;
 	}
    }

    if (ifaddr -> siso_tlen = generic -> ta_selectlen) {
	bcopy (generic -> ta_selector, cp, generic -> ta_selectlen);
	cp += generic -> ta_selectlen;
    }

    if ((ifaddr -> siso_len = cp - (char *) ifaddr) < sizeof *ifaddr)
	ifaddr -> siso_len = sizeof *ifaddr;
    ifaddr -> siso_family = AF_ISO;

    return OK;
}



STATIC	int  gen2tp4X (generic, specific)
struct tsapADDR *generic;
union sockaddr_osi *specific;
{
    struct TSAPaddr tas;

    copyTSAPaddrX (generic, &tas);
    return gen2tp4 (&tas, specific);
}

/*  */

int	tp42gen (generic, specific)
struct TSAPaddr *generic;
union sockaddr_osi *specific;
{
    register char *cp;
    register struct NSAPaddr *na = generic -> ta_addrs;
    struct sockaddr_iso	*ifaddr = &specific -> osi_sockaddr;

    bzero ((char *) generic, sizeof *generic);

    cp = ifaddr -> siso_data;
    if (na -> na_addrlen = ifaddr -> siso_nlen) {
	na -> na_stack = NA_NSAP;
	na -> na_community = ts_comm_nsap_default;
	bcopy (cp, na -> na_address, na -> na_addrlen);
	cp += na -> na_addrlen;

	generic -> ta_naddr++;
    }

    if (generic -> ta_selectlen = ifaddr -> siso_tlen)
	bcopy (cp, generic -> ta_selector, generic -> ta_selectlen);

    return OK;
}


int	tp42genX (generic, specific)
struct tsapADDR *generic;
union sockaddr_osi *specific;
{
    int	    result;
    struct TSAPaddr tas;

    if ((result = tp42gen (&tas, specific)) == OK)
	copyTSAPaddrY (&tas, generic);

    return result;
}
#else
int	_ts2bsd_stub () { return(0); };
#endif
