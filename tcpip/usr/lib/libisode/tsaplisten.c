static char sccsid[] = "@(#)15	1.3  src/tcpip/usr/lib/libisode/tsaplisten.c, isodelib7, tcpip411, GOLD410 4/5/93 17:19:17";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: TDoQueues TFreeQueues TNetAcceptAux TNetClose TNetFork TNetListen
 *    TNetListenAux TNetQueue TNetUnique TNetWork TSetQueuesOK add_fd 
 *    bridge_except bridgeaccept1 bridgeaccept2 bridgelisten bridgelisten_aux 
 *    bridgeunique check_events chldser del_fd exception findlblk findlblkbyfd 
 *    freeeblk freelblk neweblk newlblk startlb tcpaccept1 tcpaccept2 tcplisten
 *    tcpunique tp4accept1 tp4accept2 tp4listen tp4unique tsaplose uniqlb 
 *    x25accept1 x25accept2 x25listen x25unique
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/tsaplisten.c
 */

/* tsaplisten.c - "network" listening */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/tsaplisten.c,v 1.2 93/02/05 17:14:03 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/tsaplisten.c,v 1.2 93/02/05 17:14:03 snmp Exp $
 *
 * RFC1085 (LPP) support contributed by the Wollongong Group, Inc.
 *
 *
 * $Log:	tsaplisten.c,v $
 * Revision 1.2  93/02/05  17:14:03  snmp
 * ANSI - D67743
 * 
 * Revision 7.20  91/03/09  11:58:23  mrose
 * update
 * 
 * Revision 7.19  91/02/22  09:47:29  mrose
 * Interim 6.8
 * 
 * Revision 7.18  91/01/24  14:50:52  mrose
 * update
 * 
 * Revision 7.17  91/01/07  12:41:30  mrose
 * update
 * 
 * Revision 7.16  90/12/17  22:18:03  mrose
 * tsel
 * 
 * Revision 7.15  90/10/16  16:24:20  mrose
 * foo
 * 
 * Revision 7.14  90/10/16  11:21:27  mrose
 * iaed
 * 
 * Revision 7.13  90/07/27  08:48:16  mrose
 * update
 * 
 * Revision 7.12  90/07/09  14:51:28  mrose
 * sync
 * 
 * Revision 7.11  90/05/08  08:55:45  mrose
 * touch-up
 * 
 * Revision 7.10  90/03/23  17:31:37  mrose
 * 8
 * 
 * Revision 7.9  90/02/19  13:07:29  mrose
 * update
 * 
 * Revision 7.8  89/12/19  22:02:35  mrose
 * touch-up
 * 
 * Revision 7.7  89/12/19  16:21:35  mrose
 * sync
 * 
 * Revision 7.6  89/12/13  07:05:50  mrose
 * touch-up
 * 
 * Revision 7.5  89/12/11  09:35:37  mrose
 * again
 * 
 * Revision 7.4  89/12/11  09:20:30  mrose
 * update
 * 
 * Revision 7.3  89/12/07  22:15:45  mrose
 * touch-up
 * 
 * Revision 7.2  89/12/07  01:07:43  mrose
 * queued writes
 * 
 * Revision 7.1  89/11/25  16:31:39  mrose
 * sync
 * 
 * Revision 7.0  89/11/23  22:30:46  mrose
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

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <isode/tailor.h>
#include <isode/tpkt.h>
#include <isode/mpkt.h>
#include <sys/ioctl.h>
#ifdef	BSD42
#include <sys/file.h>
#endif
#ifdef	SYS5
#include <fcntl.h>
#endif

#ifdef	LPP
#undef	X25
#undef	BRIDGE_X25
#undef	TP4
#endif

#ifdef	LPP
#include <isode/dgram.h>
#endif
#ifdef	TCP
#include <isode/internet.h>
#endif
#ifdef	X25
#include <isode/x25.h>
#endif
#ifdef	TP4
#include <isode/tp4.h>
#endif


#ifdef	LPP
#undef	MGMT
#endif

/*    DATA */

union naddr {
#ifdef	TCP
    struct sockaddr_in lb_un_isock;
#endif

#if	defined(X25) || defined(BRIDGE_X25)
    struct NSAPaddr	    lb_un_xsock;
#endif

#ifdef	TP4
    struct TSAPaddr	    lb_un_tsock;
#endif

    int	lb_dummy;
};


struct listenblk {
    struct listenblk *lb_forw;	/* doubly-linked list */
    struct listenblk *lb_back;	/*   .. */

    int	    lb_fd;		/* network handle */
#define LISTEN_EXCEPTED	(-2)	/* magic value */

    int	    lb_type;		/* either listener, or exception handler */
#define LB_UNDEF	0
#define LB_LISTEN	1
#define	LB_ACCEPT	2    
#define LB_ACCEPTNOW	3
#define LB_EXCEPTION	4
#ifndef	LPP
#define	LB_QUEUED	5
#endif

    IFP	     lb_magic;

    struct TSAPaddr lb_addr;	/* transport address */

    struct tsapblk *lb_tb;

    union naddr lb_un1;
#define	lb_loc_isock	lb_un1.lb_un_isock
#define	lb_loc_xsock	lb_un1.lb_un_xsock
#define	lb_loc_nsock	lb_un1.lb_un_nsock
#define	lb_loc_tsock	lb_un1.lb_un_tsock
    union naddr lb_un3;
#define	lb_rem_isock	lb_un3.lb_un_isock
#define	lb_rem_xsock	lb_un3.lb_un_xsock
#define	lb_rem_nsock	lb_un3.lb_un_nsock
#define	lb_rem_tsock	lb_un3.lb_un_tsock

    union {
	struct {
	    IFP	    accept1;			/* accept 1 function */
	    IFP	    accept2;			/* accept 2 function */
	} lb_un_accept;
	IFP	    lb_un_except;		/* exception function */
    }	    lb_un2;
#define lb_accept1	lb_un2.lb_un_accept.accept1
#define lb_accept2	lb_un2.lb_un_accept.accept2
#define	lb_except	lb_un2.lb_un_except

    IFP	    lb_close;				/* close function */
};
#define	NULLLBP		((struct listenblk *) 0)

static int  once_only = 0;
static struct listenblk listenque;
static struct listenblk *LHead = &listenque;


struct listenblk  *findlblk (), *newlblk (), *findlblkbyfd ();


/*
 * Event block abstraction. An event block is generated by interfaces
 * that might need to do things occasionally. Currently this is only
 * the bridge but other things should be slot inable.
 * The idea is you but a listenblk of the exception type on a fd.
 * If the fd `goes off' the listenblk is destroyed and an eventblk
 * created which will be called every 60 seconds for blocking listens,
 * and before each listen otherwise.
 */

struct eventblk {
    struct eventblk *ev_forw;		/* doubly-linked list */
    struct eventblk *ev_back;		/*   .. */

    struct TSAPaddr  ev_taddr;		/* associated address */

    IFP              ev_eventfnx;	/* the function to call */
};
#define	NULLEVP	((struct eventblk *) 0)

static struct eventblk eventqueue;
static struct eventblk *EHead = &eventqueue;
static int    ev_onceonly = 0;
static int    ev_count = 0;

struct eventblk *neweblk ();


#define	add_fd(fd) \
{ \
    if ((fd) >= acl_nfds) \
	acl_nfds = (fd) + 1; \
    if (acl_count++ == 0) \
	FD_ZERO (&acl_mask); \
    FD_SET (fd, &acl_mask); \
}
#define	del_fd(fd) \
{ \
    if ((fd) + 1 == acl_nfds) \
	acl_nfds--; \
    if (--acl_count == 0) \
	acl_nfds = 0; \
    FD_CLR (fd, &acl_mask); \
}

static int acl_nfds = 0;
static int acl_count = 0;
static fd_set acl_mask;

static int qw_nfds = 0;
static int qw_count = 0;
static fd_set qw_mask;


int	startlb (), uniqlb ();

/*  */

#ifdef	TCP
int	tcplisten (), tcpaccept1 (), tcpaccept2 (), tcpunique ();
#endif
#ifdef	X25
int	x25listen (), x25accept1 (), x25accept2 (), x25unique ();
#endif
#ifdef	BRIDGE_X25
int	bridgelisten (), bridgeaccept1 (), bridgeaccept2 (), bridgeunique ();
int	close_bridge_socket (), bridge_except ();
#endif
#ifdef	TP4
int	tp4listen (), tp4accept1 (), tp4accept2 (), tp4unique ();
#endif

static struct nsapent {
    int	    ns_type;
    int	    ns_stack;

    IFP	    ns_listen;
    IFP	    ns_accept1;
    IFP	    ns_accept2;
    IFP	    ns_unique;
    IFP	    ns_close;
}	nsaps[] = {
#ifdef	TCP
    NA_TCP, TS_TCP,
	tcplisten, tcpaccept1, tcpaccept2, tcpunique, close_tcp_socket,
#endif

#ifdef	X25
    NA_X25, TS_X25,
	x25listen, x25accept1, x25accept2, x25unique, close_x25_socket,
#endif

#ifdef	BRIDGE_X25
    NA_BRG, TS_BRG,
	bridgelisten, bridgeaccept1, bridgeaccept2, bridgeunique, close_bridge_socket,
#endif

#ifdef	TP4
    NA_NSAP, TS_TP4,
    	tp4listen, tp4accept1, tp4accept2, tp4unique, close_tp4_socket,
#endif

    NULL
};


#ifdef	LPP
static int _lpp_fd = NOTOK;

u_short	_lpp_lastport;		/* MOBY HACK */
#endif


#ifdef	BSD42
static int	chldhit;

#ifdef _ANSI_C_SOURCE
static	SFD	chldser (int sig, long code, struct sigcontext *sc);
#else /* !_ANSI_C_SOURCE */
SFD	chldser ();
#endif /* _ANSI_C_SOURCE */

#endif /* BSD42 */

extern int  errno;

/*  */

int	TNetListen (ta, td)
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    return TNetWork (ta, td, startlb, NULLIFP);
}

int	TNetListenAux (ta, magic, td)
register struct TSAPaddr *ta;
IFP	magic;
struct TSAPdisconnect *td;
{
    return TNetWork (ta, td, startlb, magic);
}


int	TNetUnique (ta, td)
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    return TNetWork (ta, td, uniqlb, NULLIFP);
}

/*  */

static int  TNetWork (ta, td, fnx, magic)
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
IFP	fnx;
IFP	magic;
{
    register int    n;
    int     lstn = NOTOK;
    int	    fd;
    register struct NSAPaddr   *na;
    register struct nsapent *ns;
    struct TSAPdisconnect tds;
#ifdef	MGMT
    struct TSAPaddr tas;
    register struct NSAPaddr   *ca;
#endif

    if ((n = ta -> ta_naddr) > NTADDR)
	return tsaplose (td, DR_PARAMETER, NULLCP,
		"illegal number of network addresses");

    if (n == 0) {
	if (fnx == startlb) {
	    for (ns = nsaps; ns -> ns_listen; ns++)
		if (ns -> ns_type == NA_NSAP && (ns -> ns_stack & ts_stacks))
		    break;
	    if (!ns -> ns_listen)
		return tsaplose (td, DR_PARAMETER, NULLCP,
				 "unsupported network address (%d)", NA_NSAP);

	    fd = (*fnx) (ta, NULLNA, ns, magic, td);
#ifdef MGMT
	    if (fd == OK)
		(void) TManGen (STARTLISTEN, NULLBP, ta);
#endif

	     return fd;
	}

	na = ta -> ta_addrs;
	for (ns = nsaps; ns -> ns_listen; ns++)
	    if (ns -> ns_stack & ts_stacks) {
		na -> na_stack = ns -> ns_type;
		na++;
	    }
	if ((n = ta -> ta_naddr = na - ta -> ta_addrs) == 0)
	    return tsaplose (td, DR_PARAMETER, NULLCP,
			     "no transport stacks active!?!");
    }

#ifdef	MGMT
    bzero ((char *) &tas, sizeof tas);
    if (tas.ta_selectlen = ta -> ta_selectlen)
	bcopy (ta -> ta_selector, tas.ta_selector, ta -> ta_selectlen);
    ca = tas.ta_addrs;
#endif

    /*
     * stricter checking in force, the proposed listen address must
     * now be in the communities and not in the tsb communities.
     * This allows us to give a list of addresses and let this code
     * sort out the useful from the useless.
     */
    for (na = ta -> ta_addrs; n-- > 0; na++) {
	int *ip;

	for (ns = nsaps; ns -> ns_listen; ns++)
	    if (ns -> ns_type == na -> na_stack && (ns -> ns_stack & ts_stacks))
		break;

	if (!ns -> ns_listen)	/* not one of our supported stacks */
	    continue;

	for (ip = ts_communities; *ip; ip++)
	    if (na -> na_subnet == *ip)
		break;
	if (!*ip)	/* stack ok, community wrong */
	    continue;

	for (ip = tsb_communities; *ip; ip ++)
	    if (na -> na_subnet == *ip)
		break;
	if (*ip)	/* only reachable via tsbridge - ignore */
	    continue;

	if ((fd = (*fnx) (ta, na, ns, magic, td)) == NOTOK)
	    goto out;

	lstn = OK;
#ifdef	MGMT
	*ca++ = *na;		/* struct copy */
#endif
    }

    if (lstn == OK) {
#ifdef MGMT
	tas.ta_naddr = ca - tas.ta_addrs;
	(void) TManGen (STARTLISTEN, NULLBP, &tas);
#endif

	return OK;
    }

    (void) tsaplose (td, DR_PARAMETER, NULLCP,
		     "no supported network addresses");

out: ;
    (void) TNetClose (ta, &tds);
    return NOTOK;
}

/*  */

static int  startlb (ta, na, ns, magic, td)
register struct TSAPaddr *ta;
register struct NSAPaddr *na;
register struct nsapent *ns;
IFP	magic;
struct TSAPdisconnect *td;
{
    struct TSAPaddr tas;
    register struct listenblk *lb;

    bzero ((char *) &tas, sizeof tas);
    bcopy (ta -> ta_selector, tas.ta_selector,
	   tas.ta_selectlen = ta -> ta_selectlen);
    if (na) {
	tas.ta_naddr = 1;
	tas.ta_addrs[0] = *na;	/* struct copy */
    }

    ta = &tas;

    if (lb = findlblk (ta, LB_LISTEN))
	return tsaplose (td, DR_OPERATION, NULLCP,
		"already listening on %s", taddr2str (ta));

    if ((lb = newlblk (LB_LISTEN, ta)) == NULLLBP)
	return tsaplose (td, DR_CONGEST, NULLCP, NULLCP);
    lb -> lb_accept1 = ns -> ns_accept1;
    lb -> lb_accept2 = ns -> ns_accept2;
    lb -> lb_close = ns -> ns_close;
    lb -> lb_magic = magic;

    if ((lb -> lb_fd = (*ns -> ns_listen) (lb, ta, td)) == NOTOK) {
	freelblk (lb);
	return NOTOK;
    }
    else
	if (lb -> lb_fd == LISTEN_EXCEPTED)
	    lb -> lb_fd = NOTOK;
	else
	    add_fd (lb -> lb_fd);

    return OK;
}

/*  */

static int  uniqlb (ta, na, ns, magic, td)
register struct TSAPaddr *ta;
register struct NSAPaddr *na;
register struct nsapent *ns;
IFP	magic;
struct TSAPdisconnect *td;
{
    int	    fd;
    struct TSAPaddr tas;
    register struct listenblk *lb;

    bzero ((char *) &tas, sizeof tas);
    bcopy (ta -> ta_selector, tas.ta_selector,
	   tas.ta_selectlen = ta -> ta_selectlen);
    tas.ta_naddr = 1;
    tas.ta_addrs[0] = *na;	/* struct copy */

    if ((fd = (*ns -> ns_unique) (&tas, td)) == NOTOK)
	return NOTOK;

    bcopy (tas.ta_selector, ta -> ta_selector,
	   ta -> ta_selectlen = tas.ta_selectlen);
    *na = tas.ta_addrs[0];	/* struct copy */
    
    ta = &tas;
    if ((lb = newlblk (LB_LISTEN, ta)) == NULLLBP)
	return tsaplose (td, DR_CONGEST, NULLCP, NULLCP);

    lb -> lb_fd = fd;
    lb -> lb_magic = magic;
    lb -> lb_accept1 = ns -> ns_accept1;
    lb -> lb_accept2 = ns -> ns_accept2;
    lb -> lb_close = ns -> ns_close;

    add_fd (lb -> lb_fd);

    return OK;
}

/*  */

int	TNetAcceptAux (vecp, vec, newfd, ta, nfds, rfds, wfds, efds, secs, td)
int    *vecp;
char  **vec;
int    *newfd;
struct TSAPaddr *ta;
int	nfds;
fd_set *rfds,
       *wfds,
       *efds;
int	secs;
struct TSAPdisconnect *td;
{
    int     accepted,
	    fd,
    	    fd2,
	    n,
    	    xsecs;
    fd_set  ifds,
            ofds,
	    xfds;
    register struct listenblk  *lb, *lb2;
    static int  inited = 0;

    if (!inited) {
#ifdef	BSD42
#ifdef _ANSI_C_SOURCE
	(void) signal (SIGCHLD, (void (*)(int))chldser);
#else /* !_ANSI_C_SOURCE */
	(void) signal (SIGCHLD, chldser);
#endif /* _ANSI_C_SOURCE */
#else
	(void) signal (SIGCLD, SIG_IGN);
#endif
	if (acl_count == 0)
	    FD_ZERO (&acl_mask);
	if (qw_count == 0)
	    FD_ZERO (&qw_mask);

	inited = 1;
    }

    *vecp = 0;
    *vec = NULL;
    if (ta)
	bzero ((char *) ta, sizeof *ta);
    if (newfd)
	*newfd = NOTOK;

    if (acl_count == 0 && ev_count == 0 && qw_count == 0) {
#ifdef 	BSD42
retry: ;

	chldhit = 0;
#endif
	if ((n = xselect (nfds, rfds, wfds, efds, secs)) == NOTOK) {
#ifdef BSD42
	    if (errno == EINTR && chldhit)
		goto retry;
#endif
	    (void) tsaplose (td, DR_CONGEST, "failed", "xselect");
	}

	return n;
    }

    if (acl_nfds >= nfds)
	nfds = acl_nfds + 1;
    ifds = acl_mask;
    if (rfds)
	for (fd = 0; fd < nfds; fd++)
	    if (FD_ISSET (fd, rfds))
		FD_SET (fd, &ifds);

    if (qw_nfds >= nfds)
	nfds = qw_nfds + 1;
    ofds = qw_mask;
    if (wfds)
	for (fd = 0; fd < nfds; fd++)
	    if (FD_ISSET (fd, wfds))
		FD_SET (fd, &ofds);

    FD_ZERO (&xfds);
    if (efds)
	xfds = *efds;	/* struct copy */

    for (;;) {
	xsecs = secs;
	if (ev_count) {
	    if (secs == NOTOK)
		xsecs = 60;	/* infinite timeout, arrange for periodic */
	    else
		check_events (); /* single attempt */
	}

#ifdef 	BSD42
	chldhit = 0;
#endif
	switch (n = xselect (nfds, &ifds, &ofds, efds, xsecs)) {
	    case OK:
	        if (secs == NOTOK && ev_count) { /* just a timeout */
		    check_events ();
		    goto next;
		}
empty: ;
		if (rfds)
		    FD_ZERO (rfds);
		if (wfds)
		    FD_ZERO (wfds);
		if (efds)
		    FD_ZERO (efds);
		return OK;

	    case NOTOK: 
#ifdef BSD42
		if (errno == EINTR && chldhit)
		    goto next;
#endif
		return tsaplose (td, DR_CONGEST, "failed", "xselect");

	    default:
		accepted = 0;
		for (lb = LHead -> lb_forw; lb != LHead; ) {
		    lb = (lb2 = lb) -> lb_forw;
		    if ((fd = lb2 -> lb_fd) == NOTOK)
			continue;
#ifndef	LPP
		    if (lb2 -> lb_type == LB_QUEUED) {
			if (FD_ISSET (fd, &ofds)) {
			    n --;
			    if (TDoQueues (lb2, td) == NOTOK) {
				if (rfds && FD_ISSET (fd, rfds)) {
				    /* on error, force caller to look at it */
				    FD_ZERO (rfds);
				    FD_SET (fd, rfds);
				    if (wfds)
					FD_ZERO (wfds);
				    if (efds)
					FD_ZERO (efds);

				    return 1;
				}

				SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
				      ("unable to propagate failure of queued write"));
				return NOTOK;
			    }

			    FD_CLR (fd, &ofds);
			}

			continue;
		    }
#endif
		    if (FD_ISSET (fd, &ifds)) {
			n --;	/* we are using one up */
			FD_CLR (fd, &ifds);
			switch (lb2 -> lb_type) {
			    case LB_LISTEN:
				errno = 0;
				if ((fd2 = (*lb2 -> lb_accept1) (lb2, td))
				        == NOTOK) {
				    if (errno == EWOULDBLOCK)
					continue;
				    return NOTOK;
				}
				if ((lb2 = findlblkbyfd (fd2)) == NULL)
				    break;
				if (lb2 -> lb_type != LB_ACCEPTNOW)
				    break;
				fd = fd2;
				/* else fall */

			    case LB_ACCEPT:
				{
				    /* take care - lb2 is free'd by accept2 */
				    IFP	    closefnx = lb2 -> lb_close,
					    magicfnx = lb2 -> lb_magic;

				    if (accepted) /* only 1 accept at a time */
					break; /* we'll get it next time */
				    if (ta)
					*ta = lb2 -> lb_addr; /* struct copy */
				    if ((*lb2 -> lb_accept2) (lb2, vecp, vec,
							      td) == NOTOK)
					return NOTOK;
				    if (newfd)
					*newfd = fd;
				    if (magicfnx
					    && (*magicfnx) (vecp, vec, td)
						    == NOTOK) {
					(void) (*closefnx) (fd);
					return NOTOK;
				    }
				    accepted++;
				}
				break;

			    case LB_EXCEPTION:
				if (exception (lb2, td) == NOTOK)
				    return NOTOK;
				break;

			    default:
				return tsaplose (td, DR_UNKNOWN, NULLCP,
						 "invalid block type");
			    }
		    }
		}
		if (!accepted) {
		    for (fd = 0 ; fd < nfds; fd++)
			if (FD_ISSET (fd, &ifds)
			        || FD_ISSET (fd, &ofds)
			        || (efds && FD_ISSET (fd, efds)))
			    break;
		    if (fd >= nfds) {
			if (secs != NOTOK)
			    goto empty;

			goto next;
		    }
		}
		if (rfds)
		    *rfds = ifds;	/* struct copy */
		if (wfds)
		    *wfds = ofds;	/* struct copy */
		return n;
	}
next: ;

	if (acl_nfds >= nfds)
	    nfds = acl_nfds + 1;
	ifds = acl_mask;
	if (rfds)
	    for (fd = 0; fd < nfds; fd++)
		if (FD_ISSET (fd, rfds))
		    FD_SET (fd, &ifds);

	if (qw_nfds >= nfds)
	    nfds = qw_nfds + 1;
	ofds = qw_mask;
	if (wfds)
	    for (fd = 0; fd < nfds; fd++)
		if (FD_ISSET (fd, wfds))
		    FD_SET (fd, &ofds);

	if (efds)
	    *efds = xfds;
    }
}

/*  */

static int exception (lb, td)
struct listenblk *lb;
struct TSAPdisconnect *td;
{
    register struct listenblk *lb2;
    register struct eventblk *eb;

    if ((lb2 = findlblk (&lb -> lb_addr, LB_LISTEN)) != NULLLBP)
	freelblk (lb2);

    if ((eb = neweblk (&lb -> lb_addr)) == NULLEVP)
	return tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
    eb -> ev_eventfnx = lb -> lb_except;

    SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
	  ("exception on %s", taddr2str (&lb -> lb_addr)));

    freelblk (lb);

    return OK;
}


static	check_events ()
{
    register struct eventblk *eb,
			     *ep;
    struct TSAPdisconnect tds;

    for (eb = EHead -> ev_forw; eb != EHead; eb = ep) {
	ep = eb -> ev_forw;
	(void) (*eb -> ev_eventfnx) (eb, &tds);
    }
}

/*  */

int	TNetClose (ta, td)
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    register struct listenblk  *lb,
                               *lp;

    if (ta == NULLTA) {
	if (once_only == 0)
	    return OK;

	for (lb = LHead -> lb_forw; lb != LHead; lb = lp) {
	    lp = lb -> lb_forw;
	    freelblk (lb);
	}
    }
    else {
	if (ta -> ta_naddr > 1) {
	    int	    gotone;
	    register int n = ta -> ta_naddr;
	    register struct NSAPaddr *na = ta -> ta_addrs;
	    struct TSAPaddr tas;

	    tas = *ta;			/* struct copy */
	    tas.ta_naddr = 1;
	    gotone = 0;
	    for (na = ta -> ta_addrs, n = ta -> ta_naddr; n > 0; na++, n--) {
		tas.ta_addrs[0] = *na;	/* struct copy */
		if (lb = findlblk (&tas, LB_LISTEN)) {
		    freelblk (lb);
		    gotone = 1;
		}
	    }
	    if (!gotone)
		return tsaplose (td, DR_PARAMETER, NULLCP,
				 "no such transport addressess");

	    return OK;
	}

	if ((lb = findlblk (ta, LB_LISTEN)) == NULLLBP)
	    return tsaplose (td, DR_PARAMETER, NULLCP,
			     "no such transport addressess");

	freelblk (lb);
    }

    return OK;
}

/*  */

#ifdef	BSD42
#include <sys/wait.h>

/* ARGSUSED */

#ifdef _ANSI_C_SOURCE
static	SFD chldser (int sig, long code, struct sigcontext *sc)
#else /* !_ANSI_C_SOURCE */
static	SFD chldser (sig, code, sc)
int	sig;
long	code;
struct sigcontext *sc;
#endif /* _ANSI_C_SOURCE */
{
    union wait status;

    while (wait3 (&status, WNOHANG, (struct rusage *) NULL) > 0)
	chldhit = 1;

#ifdef SFDINT
    return(0);
#endif
}
#endif

/*  */

#ifdef	LPP
/* ARGSUSED */
#endif

int	TNetFork (vecp, vec, td)
int	vecp;
char  **vec;
struct TSAPdisconnect *td;
{
    int	    pid;
#ifdef	TIOCNOTTY
    int	    sd;
#endif

    switch (pid = fork ()) {
	case OK:
	    break;

	case NOTOK:
	    (void) tsaplose (td, DR_CONGEST, "connection",
			     "unable to fork, so rejecting");
	default:
#ifndef	LPP
	    {
		struct TSAPstart   tss;
		register struct TSAPstart  *ts = &tss;
		register struct tsapblk *tb;

		if (TInit (vecp, vec, ts, td) == NOTOK) {
		    SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
			  ("TNetFork: TInit returns [%s]",
			   TErrString (td -> td_reason)));
		    return pid;
		}

		if ((tb = findtblk (ts -> ts_sd)) == NULL) {
		    SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
			  ("TNetFork: findtblk fails"));
		    return pid;
		}

		(void) (*tb -> tb_closefnx) (tb -> tb_fd);

		tb -> tb_fd = NOTOK;
		freetblk (tb);
	    }
#else
	    if (_lpp_fd != NOTOK) {
		(void) close_tcp_socket (_lpp_fd);
		_lpp_fd = NOTOK;
	    }
#endif

	    return pid;
    }

    (void) TNetClose (NULLTA, td);

#ifdef	SETSID
	(void) setsid ();
#endif
#ifdef	TIOCNOTTY
	if ((sd = open ("/dev/tty", O_RDWR)) != NOTOK) {
	    (void) ioctl (sd, TIOCNOTTY, NULLCP);
	    (void) close (sd);
	}
#else
#ifdef	SYS5
	(void) setpgrp ();
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
#endif
#endif
	isodexport (NULLCP);

#ifdef	BSD42
    (void) signal (SIGCHLD, SIG_DFL);
#else
    (void) signal (SIGCLD, SIG_DFL);
#endif

    return OK;
}

/*    TCP */

#ifdef	TCP
static int  tcplisten (lb, ta, td)
register struct listenblk *lb;
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int	    fd;
    register struct sockaddr_in *isock = &lb -> lb_loc_isock;
    register struct hostent *hp;
    register struct NSAPaddr *na;

    if (ta -> ta_naddr < 1)
	return tsaplose (td, DR_ADDRESS, NULLCP, "TCP address not specified");
    na = ta -> ta_addrs;

    if (na -> na_domain[0]) {
	if ((hp = gethostbystring (na -> na_domain)) == NULL)
	    return tsaplose (td, DR_ADDRESS, NULLCP, "%s: unknown host",
			     na -> na_domain);
    }
    else
	hp = NULL;

    bzero ((char *) isock, sizeof *isock);
    isock -> sin_family = hp ? hp -> h_addrtype : AF_INET;
    if (na -> na_port == 0) {
	register struct servent *sp;

	if ((sp = getservbyname ("tsap", "tcp")) == NULL)
	    sp = getservbyname ("iso-tsap", "tcp");
	isock -> sin_port = sp ? sp -> s_port : htons ((u_short) 102);
    }
    else
	isock -> sin_port = na -> na_port;
    if (hp)
	inaddr_copy (hp, isock);

    switch (na -> na_tset) {
#ifdef	LPP
	case NA_TSET_UDP:
	    if ((fd = start_udp_server (isock, 0, 0, 0)) == NOTOK)
		return tsaplose (td, DR_CONGEST, "failed", "start_udp_server");
	    break;
#endif

	case NA_TSET_TCP:
	default:
	    if ((fd = start_tcp_server (isock, SOMAXCONN, 0, 0)) == NOTOK)
		return tsaplose (td, DR_CONGEST, "failed", "start_tcp_server");
	    break;
    }
    lb -> lb_addr.ta_addrs -> na_tset = na -> na_tset;

    return fd;
}

/*  */

#ifdef	LPP
char   *udpsave ();
#endif


static int  tcpaccept1 (lb, td)
register struct listenblk *lb;
struct TSAPdisconnect *td;
{
    int     fd,
	    tset = lb -> lb_addr.ta_addrs -> na_tset;
    struct listenblk *lb2;
    register struct tsapblk *tb = NULL;
#ifndef	LPP
    register struct tsapkt *t = NULL;
#endif

    if ((lb2 = newlblk (LB_ACCEPT, NULLTA)) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }
    
    switch (tset) {
#ifdef	LPP
	case NA_TSET_UDP:
	    if ((fd = join_udp_client (lb -> lb_fd, &lb2 -> lb_rem_isock))
		    == NOTOK)   {
		freelblk (lb2);
		return tsaplose (td, DR_NETWORK, "failed", "join_udp_client");
	    }
	    break;
#endif

	case NA_TSET_TCP:
	default:
	    if ((fd = join_tcp_client (lb -> lb_fd, &lb2 -> lb_rem_isock))
		    == NOTOK) {
		freelblk (lb2);
		return tsaplose (td, DR_NETWORK, "failed", "join_tcp_client");
	    }
	
#ifdef	LPP
	    _lpp_fd = fd;
	    _lpp_lastport = lb -> lb_loc_isock.sin_port;
#endif
#ifdef	EXOS
	    del_fd (lb -> lb_fd);

	    if ((lb -> lb_fd = start_tcp_server (&lb -> lb_loc_isock,
						 SOMAXCONN, 0, 0)) != NOTOK) {
		add_fd (lb -> lb_fd);
	    }
#endif
	    break;
    }

    lb2 -> lb_fd = fd;
    lb2 -> lb_addr = lb -> lb_addr;
    lb2 -> lb_loc_isock = lb -> lb_loc_isock;
    lb2 -> lb_accept2 = lb -> lb_accept2;
    lb2 -> lb_close = lb -> lb_close;
    lb2 -> lb_magic = lb -> lb_magic;

#ifndef	LPP
    if ((tb = newtblk ()) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }

    tb -> tb_fd = fd;
    (void) TTService (tb);

    add_fd (fd);
#else
    lb2 -> lb_type = LB_ACCEPTNOW;
#endif
    lb2 -> lb_tb = tb;

    return fd;

out: ;
#ifndef	LPP
    if (tb) {
	tb -> tb_fd = NOTOK;
	freetblk (tb);
    }
    freetpkt (t);
#endif
    if (lb2)
	freelblk (lb2);

    return NOTOK;
}

/*  */

static int tcpaccept2 (lb, vecp, vec, td)
register struct listenblk *lb;
int    *vecp;
char  **vec;
struct TSAPdisconnect *td;
{
    int     fd;
#ifdef LPP
    int	    tset = lb -> lb_addr.ta_addrs -> na_tset;
#endif
    char    buffer1[BUFSIZ],
            buffer2[BUFSIZ];
#ifdef	SOCKETS
    struct sockaddr_in  in_socket;
    struct sockaddr_in *isock = &in_socket;
    int len;
#endif
#ifndef	LPP
    register struct tsapblk *tb = lb -> lb_tb;
    register struct tsapkt *t = NULL;
#endif

    fd = lb -> lb_fd;
    (void) sprintf (buffer1, "%s+%d",
		    inet_ntoa (lb -> lb_rem_isock.sin_addr),
		    ntohs (lb -> lb_rem_isock.sin_port));

#ifdef	SOCKETS
    len = sizeof *isock;
    if (getsockname (fd, (struct sockaddr *) isock, &len) != NOTOK) {
	(void) sprintf (buffer2, "%s+%d",
			inet_ntoa (isock -> sin_addr),
			ntohs (isock -> sin_port));
    }
    else
#endif
	(void) sprintf (buffer2, "%s", TLocalHostName ());

#ifndef LPP
    if ((t = fd2tpkt (fd, tb -> tb_initfnx,
		      tb -> tb_readfnx)) == NULL || t -> t_errno != OK) {
	(void) tpktlose (tb, td, t ? t -> t_errno : DR_CONGEST, NULLCP,
		NULLCP);
	goto out;
    }

    if (TPDU_CODE (t) != TPDU_CR) {
	(void) tpktlose (tb, td, DR_PROTOCOL, NULLCP,
		"transport protocol mangled: expecting 0x%x, got 0x%x",
		TPDU_CR, TPDU_CODE (t));
	goto out;
    }

    if (lb -> lb_addr.ta_selectlen > 0
	    && (lb -> lb_addr.ta_selectlen != t -> t_calledlen
		    || bcmp (lb -> lb_addr.ta_selector,
			     t -> t_called,
			     lb -> lb_addr.ta_selectlen))) {
	(void) tpktlose (tb, td, DR_SESSION, NULLCP,
			 "not expecting connection for tsap/%s",
			 sel2str (t -> t_called, t -> t_calledlen, 1));
	goto out;
    }

    vec[0] = "tsaplisten";		/* any value will do */

    if ((vec[1] = tcpsave (fd, buffer1, buffer2, td)) == NULL)
	goto out;

    if ((vec[2] = tpkt2str (t)) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, NULLCP);
	goto out;
    }

    vec[*vecp = 3] = NULLCP;

    tb -> tb_fd = NOTOK;
    freetblk (tb);
    freetpkt (t);
    del_fd (lb -> lb_fd);
#else
    vec[0] = "psaplisten";

    switch (tset) {
	case NA_TSET_UDP:
	    if ((vec[1] = udpsave (fd, buffer1, buffer2, td)) == NULL)
		goto out;
	    break;

	case NA_TSET_TCP:
	default:
	    if ((vec[1] = tcpsave (fd, buffer1, buffer2, td)) == NULL)
		goto out;
	    break;
    }

    vec[*vecp = 2] = NULLCP;
#endif

    lb -> lb_fd = NOTOK;
    freelblk (lb);
    return OK;

out: ;
#ifndef	LPP
    if (tb) {
	tb -> tb_fd = NOTOK;
	freetblk (tb);
    }
    freetpkt (t);
#endif
    if (lb)
	freelblk (lb);

    return NOTOK;
}

/*  */

static int  tcpunique (ta, td)
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int	    fd;
    char   *cp;
    struct sockaddr_in in_socket;
    register struct sockaddr_in *isock = &in_socket;
    register struct hostent *hp;
    register struct NSAPaddr *na = ta -> ta_addrs;

    cp = na -> na_domain[0] ? na -> na_domain : TLocalHostName ();
    if ((hp = gethostbystring (cp)) == NULL)
	return tsaplose (td, DR_ADDRESS, NULLCP, "%s: unknown host", cp);

    bzero ((char *) isock, sizeof *isock);
    isock -> sin_family = hp -> h_addrtype;
    inaddr_copy (hp, isock);
    
    switch (na -> na_tset) {
#ifdef	LPP
	case NA_TSET_UDP:
	    if ((fd = start_udp_server (isock, 0, 0, 0)) == NOTOK)
		return tsaplose (td, DR_CONGEST, "failed", "start_udp_server");
	    break;
#endif

	case NA_TSET_TCP:
	default:
	    if ((fd = start_tcp_server (isock, SOMAXCONN, 0, 0)) == NOTOK)
		return tsaplose (td, DR_CONGEST, "failed", "start_tcp_server");
	    break;
    }
    (void) strcpy (na -> na_domain, inet_ntoa (isock -> sin_addr));
    na -> na_port = isock -> sin_port;

    return fd;
}
#endif

/*    X.25 */

#ifdef	X25

/* ARGSUSED */

static int  x25listen (lb, ta, td)
struct listenblk *lb;
struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int     fd;

    if (ta -> ta_naddr < 1)
	return tsaplose (td, DR_ADDRESS, NULLCP, "X.121 DTE not specified");
    
    if ((fd = start_x25_server (ta -> ta_addrs, SOMAXCONN, 0, SO_KEEPALIVE))
	    == NOTOK)
        return tsaplose (td, DR_CONGEST, "failed", "start_x25_server");

    lb -> lb_loc_xsock = *ta -> ta_addrs;

    return fd;
}

/*  */

static int  x25accept1 (lb, td)
register struct listenblk *lb;
struct TSAPdisconnect *td;
{
    int     fd;
    register struct tsapblk *tb;
    struct listenblk *lb2;

    if ((lb2 = newlblk (LB_ACCEPT, NULLTA)) == NULL) {
	tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }

    lb2 -> lb_accept2 = lb -> lb_accept2;
    lb2 -> lb_addr = lb -> lb_addr;
    lb2 -> lb_close = lb -> lb_close;
    lb2 -> lb_loc_xsock = lb -> lb_loc_xsock;
    lb2 -> lb_magic = lb -> lb_magic;

    if ((fd = join_x25_client (lb -> lb_fd, &lb2 -> lb_rem_xsock)) == NOTOK) {
	freelblk (lb2);
        return tsaplose (td, DR_NETWORK, "failed", "join_x25_client");
    }

    lb2 -> lb_fd = fd;

    if ((tb = newtblk ()) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
        goto out;
    }

    tb -> tb_fd = fd;
    (void) XTService (tb);

    lb2 -> lb_tb = tb;
    add_fd (fd);

    return fd;

out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    if (lb2)
	freelblk (lb2);

    return NOTOK;
}

/*  */

static int x25accept2 (lb, vecp, vec, td)
register struct listenblk *lb;
int	*vecp;
char	**vec;
struct TSAPdisconnect *td;
{
    register struct tsapblk *tb = lb -> lb_tb;
    register struct tsapkt *t = NULL;
    int	fd;


    if ((t = fd2tpkt (fd = tb -> tb_fd, tb -> tb_initfnx,
		      tb -> tb_readfnx)) == NULL || t -> t_errno != OK) {
        (void) tpktlose (tb, td, t ? t -> t_errno : DR_CONGEST, NULLCP,
                NULLCP);
        goto out;
    }

    if (TPDU_CODE (t) != TPDU_CR) {
        (void) tpktlose (tb, td, DR_PROTOCOL, NULLCP,
                "transport protocol mangled: expecting 0x%x, got 0x%x",
                TPDU_CR, TPDU_CODE (t));
        goto out;
    }

    if (lb -> lb_addr.ta_selectlen > 0
	    && (lb -> lb_addr.ta_selectlen != t -> t_calledlen
		    || bcmp (lb -> lb_addr.ta_selector,
			     t -> t_called,
			     lb -> lb_addr.ta_selectlen))) {
	(void) tpktlose (tb, td, DR_SESSION, NULLCP,
			 "not expecting connection for tsap/%s",
			 sel2str (t -> t_called, t -> t_calledlen, 1));
	goto out;
    }

    vec[0] = "tsaplisten";              /* any value will do */

    if ((vec[1] = x25save (fd, lb -> lb_rem_xsock.na_dte,
			   lb -> lb_rem_xsock.na_dtelen,
			   lb -> lb_loc_xsock.na_dte,
			   lb -> lb_loc_xsock.na_dtelen, td)) == NULL)
        goto out;
    if ((vec[2] = tpkt2str (t)) == NULL) {
        (void) tsaplose (td, DR_CONGEST, NULLCP, NULLCP);
        goto out;
    }

    vec[*vecp = 3] = NULLCP;

    tb -> tb_fd = NOTOK;
    freetblk (tb);
    freetpkt (t);
    lb -> lb_fd = NOTOK;
    del_fd (fd);
    freelblk (lb);

    return fd;

out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    freetpkt (t);
    freelblk (lb);

    return NOTOK;
}

/*  */

static int  x25unique (ta, td)
struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int     fd;
    register struct NSAPaddr *na = ta -> ta_addrs;

    bzero ((char *) na, sizeof *na);
    na -> na_stack = NA_X25;

    if ((fd = start_x25_server (na, SOMAXCONN, 0, SO_KEEPALIVE)) == NOTOK)
        return tsaplose (td, DR_CONGEST, "failed", "start_x25_server");

    return fd;
}
#endif

/*  */

#ifdef BRIDGE_X25

/* ARGSUSED */

static int  bridgelisten (lb, ta, td)
register struct listenblk *lb;
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int	    fd;
    register struct eventblk *eb;

    if ((fd = bridgelisten_aux (lb, ta, td)) != NOTOK)
	return fd;

    if ((eb = neweblk (ta)) == NULLEVP)
	return tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
    eb -> ev_eventfnx = bridge_except;

    return LISTEN_EXCEPTED;
}


static int bridge_except (eb, td)
register struct eventblk *eb;
struct TSAPdisconnect *td;
{
    register struct listenblk *lb;
    register struct TSAPaddr *ta = &eb -> ev_taddr;

    if ((lb = newlblk (LB_LISTEN, ta)) == NULLLBP)
	return tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
    if ((lb -> lb_fd = bridgelisten_aux (lb, ta, td)) == NOTOK) {
	freelblk (lb);
	return OK;
    }
    SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
       ("reconnection on %s", taddr2str (ta)));
    freeeblk (eb);

    lb -> lb_close = close_bridge_socket;
    lb -> lb_accept1 = bridgeaccept1;
    lb -> lb_accept2 = bridgeaccept2;
    add_fd (lb -> lb_fd);

    return OK;
}


static int  bridgelisten_aux (lb, ta, td)
register struct listenblk *lb;
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int     fd;
    register struct listenblk *lb2;

    if (ta -> ta_naddr < 1)
	return tsaplose (td, DR_ADDRESS, NULLCP, "X.121 DTE not specified");

    if ((fd = start_bridge_server (ta -> ta_addrs, SOMAXCONN, 0, SO_KEEPALIVE))
	    == NOTOK)
        return tsaplose (td, DR_CONGEST, "failed", "start_bridge_server");

    lb -> lb_loc_xsock = *ta -> ta_addrs;
    if ((lb2 = newlblk (LB_EXCEPTION, ta)) == NULLLBP) {
	close_bridge_socket (fd);
	return NOTOK;
    }
    lb2 -> lb_fd = get_bridge_assfd (fd);
    lb2 -> lb_except = bridge_except;
    lb2 -> lb_close = close_bridge_socket;

    add_fd (lb2 -> lb_fd);

    return fd;
}

/*  */

static int  bridgeaccept1 (lb, td)
register struct listenblk *lb;
struct TSAPdisconnect *td;
{
    int     fd;
    register struct tsapblk *tb;
    register struct tsapkt *t = NULL;
    struct listenblk *lb2;

    if ((lb2 = newlblk (LB_ACCEPT, NULLTA)) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }
    lb2 -> lb_accept2 = lb -> lb_accept2;
    lb2 -> lb_addr = lb -> lb_addr;
    lb2 -> lb_close = lb -> lb_close;
    lb2 -> lb_loc_xsock = lb -> lb_loc_xsock;
    lb2 -> lb_magic = lb -> lb_magic;

    if ((fd = join_bridge_client (lb -> lb_fd, &lb2 -> lb_rem_xsock)) 
	    == NOTOK) {
	freelblk (lb2);
        return tsaplose (td, DR_NETWORK, "failed", "join_bridge_client");
    }

    lb2 -> lb_fd = fd;

    if ((tb = newtblk ()) == NULL) {
        (void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
        goto out;
    }
    lb2 -> lb_tb = tb;

    tb -> tb_fd = fd;
    (void) BTService (tb);

    add_fd (fd);
    return fd;

out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    if (lb2)
	    freelblk (lb2);
    freetpkt (t);

    return NOTOK;
}

/*  */

static int bridgeaccept2 (lb, vecp, vec, td)
register struct listenblk *lb;
int	*vecp;
char	**vec;
struct TSAPdisconnect *td;
{
    int     fd = lb -> lb_fd;
    register struct tsapblk *tb = lb -> lb_tb;
    register struct tsapkt *t = NULL;


    if ((t = fd2tpkt (tb -> tb_fd, tb -> tb_initfnx, tb -> tb_readfnx)) == NULL
            || t -> t_errno != OK) {
        (void) tpktlose (tb, td, t ? t -> t_errno : DR_CONGEST, NULLCP,
                NULLCP);
        goto out;
    }

    if (TPDU_CODE (t) != TPDU_CR) {
        (void) tpktlose (tb, td, DR_PROTOCOL, NULLCP,
                "transport protocol mangled: expecting 0x%x, got 0x%x",
                TPDU_CR, TPDU_CODE (t));
        goto out;
    }

    if (lb -> lb_addr.ta_selectlen > 0
	    && (lb -> lb_addr.ta_selectlen != t -> t_calledlen
		    || bcmp (lb -> lb_addr.ta_selector,
			     t -> t_called,
			     lb -> lb_addr.ta_selectlen))) {
	(void) tpktlose (tb, td, DR_SESSION, NULLCP,
			 "not expecting connection for tsap/%s",
			 sel2str (t -> t_called, t -> t_calledlen, 1));
	goto out;
    }

    vec[0] = "tsaplisten";              /* any value will do */

    if ((vec[1] = bridgesave (tb -> tb_fd, lb -> lb_rem_xsock.na_dte,
			      lb -> lb_rem_xsock.na_dtelen,
			      lb -> lb_loc_xsock.na_dte,
			      lb -> lb_loc_xsock.na_dtelen, td)) == NULL)
        goto out;
    if ((vec[2] = tpkt2str (t)) == NULL) {
        (void) tsaplose (td, DR_CONGEST, NULLCP, NULLCP);
        goto out;
    }

    vec[*vecp = 3] = NULLCP;

    tb -> tb_fd = NOTOK;
    freetblk (tb);
    freetpkt (t);
    del_fd (fd);
    lb -> lb_fd = NOTOK;
    freelblk (lb);

    return fd;

out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    freetpkt (t);
    freelblk (lb);

    return NOTOK;
}

/*  */

/* ARGSUSED */

static int  bridgeunique (ta, td)
struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    return tsaplose (td, DR_ADDRESS, NULLCP,
	    "unique listens not supported at the X.25 bridge");
}
#endif

/*    TP4 */

#ifdef	TP4

/*    TP4 from 4.4BSD */

#ifdef	BSD_TP4
/* ARGSUSED */

static int  tp4listen (lb, ta, td)
struct listenblk *lb;
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    return start_tp4_server (ta, SOMAXCONN, SO_KEEPALIVE, 0, td);
}

/*  */

static int  tp4accept1 (lb, td)
register struct listenblk *lb;
struct TSAPdisconnect *td;
{
    int	    fd;
    register struct tsapblk *tb;
    struct listenblk *lb2;

    if ((lb2 = newlblk (LB_ACCEPTNOW, NULLTA)) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }

    lb2 -> lb_accept2 = lb -> lb_accept2;
    lb2 -> lb_addr = lb -> lb_addr;
    lb2 -> lb_close = lb -> lb_close;
    lb2 -> lb_loc_tsock = lb -> lb_loc_tsock;

    if ((fd = join_tp4_client (lb -> lb_fd, &lb2 -> lb_rem_tsock, td))
	    == NOTOK) {
	freelblk (lb2);
	return NOTOK;
    }

    lb2 -> lb_fd = fd;

    if ((tb = newtblk ()) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }

    tb -> tb_fd = fd;
    (void) tp4init (tb);

    lb2 -> lb_tb = tb;

    copyTSAPaddrY (&tb -> tb_responding, &lb2 -> lb_rem_tsock);

    return fd;

out:	;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    if (lb2)
	freelblk (lb2);
    return NOTOK;
}

/*  */

static int  tp4accept2 (lb, vecp, vec, td)
register struct listenblk *lb;
int	*vecp;
char	**vec;
struct TSAPdisconnect *td;
{
    int	    cc,
	    cmsgtype,
	    fd = lb -> lb_fd,
	    len;
    char    udata[TS_SIZE];
    register struct tsapblk *tb = lb -> lb_tb;
    union sockaddr_osi	sock;
    struct sockaddr_iso	*ifaddr = &sock.osi_sockaddr;
    static char buffer[BUFSIZ];

    len = sizeof sock;
    if (getsockname (fd, (struct sockaddr *) ifaddr, &len) != NOTOK) {
	ifaddr -> siso_len = len;
	(void) tp42genX (&tb -> tb_initiating, &sock);
    }
    else
	SLOG (tsap_log, LLOG_EXCEPTIONS, "failed",
	      ("getsockname on incoming connection"));

    cc = sizeof udata;
    if (tp4getCmsg (fd, &cc, &cmsgtype, udata) == NOTOK) {
	(void) tsaplose (td, DR_CONGEST, "TPOPT_CONN_DATA", "unable to get");
	goto out;
    }
    if (cmsgtype != TPOPT_CONN_DATA)
	cc = 0;

    vec[0] = "tsaplisten";		/* any value will do */
	
    if ((vec[1] = tp4save (tb -> tb_fd, td)) == NULL)
	goto out;
    len = 0;
    if (cc > 0)
	len += explode (buffer + len, (u_char *) udata, cc);
    buffer[len] = NULL;
    vec[2] = buffer;

    vec[*vecp = 3] = NULLCP;

    tb -> tb_fd = NOTOK;
    freetblk (tb);
    lb -> lb_fd = NOTOK;
    freelblk (lb);
    
    return OK;
	
out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    freelblk (lb);

    return NOTOK;
}

/*  */

/* ARGSUSED */

static int  tp4unique (ta, td)
struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    int	    fd;
    register struct NSAPaddr *na = ta -> ta_addrs;

    bzero ((char *) na, sizeof *na);
    na -> na_stack = NA_NSAP;

    return start_tp4_server (na, SOMAXCONN, SO_KEEPALIVE, 0, td);
}
#endif

/*    TP4 from SunLink OSI */

#ifdef	SUN_TP4
/* ARGSUSED */

static int  tp4listen (lb, ta, td)
struct listenblk *lb;
register struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    return start_tp4_server (ta, SOMAXCONN, 0, 0, td);
}

/*  */

static int  tp4accept1 (lb, td)
register struct listenblk *lb;
struct TSAPdisconnect *td;
{
    int	    fd;
    register struct tsapblk *tb;
    struct listenblk *lb2;

    if ((lb2 = newlblk (LB_ACCEPT, NULLTA)) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }

    lb2 -> lb_loc_isock = lb -> lb_loc_isock;
    lb2 -> lb_addr = lb -> lb_addr;
    lb2 -> lb_accept2 = lb -> lb_accept2;
    lb2 -> lb_close = lb -> lb_close;
    lb2 -> lb_magic = lb -> lb_magic;

    if ((fd = join_tp4_client (lb -> lb_fd, &lb -> lb_rem_tsock, td)) 
	    == NOTOK) {
	freelblk (lb2);
	return NOTOK;
    }

    lb2 -> lb_fd = fd;

    if ((tb = newtblk ()) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
	goto out;
    }
    tb -> tb_fd = fd;
    (void) tp4init (tb);

    add_fd (fd);
    lb2 -> lb_tb = tb;

    return fd;

out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    if (lb2)
	freelblk (lb2);

    return NOTOK;
}

/*  */

static int tp4accept2 (lb, vecp, vec, td)
register struct listenblk *lb;
int	*vecp;
char	**vec;
struct TSAPdisconnect *td;
{
    int	    cc,
	    fd = lb -> lb_fd,
    	    header_len,
	    len;
    char    data[TC_SIZE];
    register struct tsapblk *tb = lb -> lb_tb;
    register struct tp4pkt *tp = NULL;
    static char buffer[BUFSIZ];

    if ((tp = newtp4pkt ((TP_EVENT) 0)) == NULL) {
	(void) tsaplose (td, DR_CONGEST, NULLCP, NULLCP);
	goto out;
    }

    header_len = sizeof (TP_MSG_CONNECT);
    if ((cc = recvfrom (fd, data, sizeof data, 0, (struct sockaddr *) tp,
			&header_len)) == NOTOK) {
	(void) tpktlose (tb, td, DR_CONGEST, "failed", "recvfrom");
	goto out;
    }

    if (tp -> tp4_event != TP_CONNECT_IND) {
	(void) tpktlose (tb, td, DR_REMOTE, NULLCP,
			 "transport protocol mangled: expecting 0x%x got 0x%x",
			 TP_CONNECT_IND, tp -> tp4_event);
	goto out;
    }

    (void) tp42genX (&tb -> tb_responding, &tp -> tp4_called);
    (void) tp42genX (&tb -> tb_initiating, &tp -> tp4_calling);

    vec[0] = "tsaplisten";		/* any value will do */
	
    if ((vec[1] = tp4save (tb -> tb_fd, td)) == NULL)
	goto out;
    len = explode (buffer, (u_char *) tp, sizeof (TP_MSG_CONNECT));
    if (cc > 0)
	len += explode (buffer + len, (u_char *) data, cc);
    buffer[len] = NULL;
    vec[2] = buffer;

    vec[*vecp = 3] = NULLCP;

    tb -> tb_fd = NOTOK;
    freetblk (tb);
    freetp4pkt (tp);
    del_fd (lb -> lb_fd);
    lb -> lb_fd = NOTOK;
    freelblk (lb);

    return OK;
	
out: ;
    if (tb) {
        tb -> tb_fd = NOTOK;
        freetblk (tb);
    }
    if (tp)
	freetp4pkt (tp);
    freelblk (lb);
    return NOTOK;
}

/*  */

/* ARGSUSED */

static int  tp4unique (ta, td)
struct TSAPaddr *ta;
struct TSAPdisconnect *td;
{
    return tsaplose (td, DR_ADDRESS, NULLCP,
	    "unique listens not yet supported with SunLink OSI");
}
#endif

#endif

/*    INTERNAL */

static struct listenblk  *newlblk (type, ta)
int	type;
struct TSAPaddr *ta;
{
    register struct listenblk  *lb;

    lb = (struct listenblk *) calloc (1, sizeof *lb);
    if (lb == NULLLBP)
	return lb;

    lb -> lb_fd = NOTOK;
    if (ta)
	lb -> lb_addr = *ta;	/* struct copy */
    lb -> lb_type = type;

    if (once_only == 0) {
	LHead -> lb_forw = LHead -> lb_back = LHead;
	once_only++;
    }

    insque (lb, LHead -> lb_back);

    return lb;
}


static	freelblk (lb)
register struct listenblk *lb;
{
    if (lb == NULLLBP)
	return;

#ifdef	MGMT
    if (lb -> lb_type == LB_LISTEN)
	(void) TManGen (ENDLISTEN, NULLBP, &lb -> lb_addr);
#endif

#ifndef	LPP
    if (lb -> lb_type != LB_QUEUED)
#endif
	if (lb -> lb_fd != NOTOK) {
	    del_fd (lb -> lb_fd);
	    (void) (*lb -> lb_close) (lb -> lb_fd);
	}

    remque (lb);

    free ((char *) lb);
}

/*  */

static struct listenblk  *findlblk (ta, type)
register struct TSAPaddr *ta;
int	type;
{
    register struct listenblk  *lb;

    if (once_only == 0)
	return NULLLBP;

    for (lb = LHead -> lb_forw; lb != LHead; lb = lb -> lb_forw)
	if (lb -> lb_type == type
	        && bcmp ((char *) &lb -> lb_addr, (char *) ta, sizeof *ta) ==0)
	    return lb;

    return NULLLBP;
}

static struct listenblk  *findlblkbyfd (fd)
int	fd;
{
    register struct listenblk  *lb;

    if (once_only == 0)
	return NULLLBP;

    for (lb = LHead -> lb_forw; lb != LHead; lb = lb -> lb_forw)
#ifndef	LPP
	if (lb -> lb_type != LB_QUEUED)
#endif
	    if (lb -> lb_fd == fd)
		return lb;

    return NULLLBP;
}

/*  */

static struct eventblk  *neweblk (ta)
struct TSAPaddr *ta;
{
    register struct eventblk  *eb;

    eb = (struct eventblk *) calloc (1, sizeof *eb);
    if (eb == NULLEVP)
	return eb;

    eb -> ev_taddr = *ta;	/* struct copy */

    if (ev_onceonly == 0) {
	EHead -> ev_forw = EHead -> ev_back = EHead;
	ev_onceonly++;
    }

    insque (eb, EHead -> ev_back);

    ev_count++;

    return eb;
}


#ifdef	BRIDGE_X25
static	freeeblk (eb)
register struct eventblk *eb;
{
    if (eb == NULLEVP)
	return;

    remque (eb);

    free ((char *) eb);

    ev_count--;
}
#endif

/*  */

#ifdef	LPP
#ifdef	lint
/* VARARGS */

int	tsaplose (td, reason, what, fmt)
struct TSAPdisconnect *td;
int	reason;
char   *what,
       *fmt;
{
    return tsaplose (td, reason, what, fmt);
}
#endif
#endif

/*    queued writes */

#ifndef	LPP
static int  TNetQueue (tb, insert, td)
register struct tsapblk *tb;
int	insert;
struct TSAPdisconnect *td;
{
    register struct listenblk *lb;

    if (once_only == 0) {
	LHead -> lb_forw = LHead -> lb_back = LHead;
	once_only++;
    }

    for (lb = LHead -> lb_forw; lb != LHead; lb = lb -> lb_forw)
	if (lb -> lb_type == LB_QUEUED && lb -> lb_tb == tb)
	    break;

    if (!insert) {
	if (lb != LHead)
	    TFreeQueues (lb);

	return OK;
    }
    if (lb != LHead)		/* should "never happen" */
	return OK;

    if ((lb = newlblk (LB_QUEUED, NULLTA)) == NULLLBP) {
	SLOG (tsap_log, LLOG_EXCEPTIONS, NULLCP,
	      ("unable to allocate listenblk for queued writes"));
	return tsaplose (td, DR_CONGEST, NULLCP, "out of memory");
    }
    lb -> lb_fd = (lb -> lb_tb = tb) -> tb_fd;

    if (lb -> lb_fd >= qw_nfds)
	qw_nfds = lb -> lb_fd + 1;
    if (qw_count++ == 0)
	FD_ZERO (&qw_mask);
    FD_SET (lb -> lb_fd, &qw_mask);

    return OK;
}

/*  */

static int  TDoQueues (lb, td)
register struct listenblk *lb;
struct TSAPdisconnect *td;
{
    register struct tsapblk *tb = lb -> lb_tb;

    switch ((*tb -> tb_drainPfnx) (tb, td)) {
	case NOTOK:
	    return NOTOK;

	case DONE:
	default:
	    TFreeQueues (lb);
	    /* and fall... */

	case OK:
	    return OK;
    }
}

/*  */

static int  TFreeQueues (lb)
register struct listenblk *lb;
{
    if (lb -> lb_fd + 1 == qw_nfds)
	qw_nfds--;
    if (--qw_count == 0)
	qw_nfds = 0;
    FD_CLR (lb -> lb_fd, &qw_mask);
    lb -> lb_fd = NOTOK;

    freelblk (lb);

    return(0);
}

/*  */

int	TSetQueuesOK (sd, onoff, td)
int	sd;
int	onoff;
struct TSAPdisconnect *td;
{
    int	    result;
    SBV	    smask;
    register struct tsapblk *tb;

    missingP (td);

    smask = sigioblock ();

    tsapPsig (tb, sd);

    result = OK;
    if (onoff) {
	if (tb -> tb_drainPfnx == NULLIFP)
	    result = tsaplose (td, DR_OPERATION, NULLCP,
			       "queued writes not supported by TS-stack");
	else {
	    tb -> tb_flags |= TB_QWRITES;
	    tb -> tb_queuePfnx = TNetQueue;
	}
    }
    else
	if (tb -> tb_qwrites.qb_forw != &tb -> tb_qwrites)
	    result = tsaplose (td, DR_WAITING, NULLCP,
			       "queued writes still waiting to drain");
	else {
	    tb -> tb_flags &= ~TB_QWRITES;
	    tb -> tb_queuePfnx = NULLIFP;
	}

    (void) sigiomask (smask);

    return result;
}
#endif
