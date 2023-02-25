static char sccsid[] = "@(#)96	1.10  src/tcpip/usr/sbin/named/ns_forw.c, tcpnaming, tcpip411, GOLD410 6/3/93 10:19:32";
/* 
 * COMPONENT_NAME: TCPIP ns_forw.c
 * 
 * FUNCTIONS: MSGSTR, ns_forw, nslookup, qcomp, qfindid, qflush, 
 *            qfree, qnew, qremove, retry, retrytime, schedretry, 
 *            unsched 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
static char sccsid[] = "ns_forw.c	4.30 (Berkeley) 6/27/90";
#endif
*/

#include <stdio.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/syslog.h>
#include <arpa/nameser.h>
#include "ns.h"
#include "db.h"

#include <nl_types.h>
extern nl_catd catd;
#include "named_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_NAMED, n, s) 

struct	qinfo *qhead = QINFO_NULL;	/* head of allocated queries */
struct	qinfo *retryqp = QINFO_NULL;	/* list of queries to retry */
struct	fwdinfo *fwdtab;		/* list of forwarding hosts */
struct  tfinfo *tfhead = (struct tfinfo *)0;
					/* list of temporary failures */

int	nsid;				/* next forwarded query id */
extern int forward_only;		/* you are only a slave */
extern int errno;
extern u_short ns_port;

time_t	retrytime();

/*
 * The following changes have been made according to RFC 1123:
 *
 * Temporary failures are cached.  tf_add() adds an address to the list
 * and tf_findaddr() searches the list.  tf_add sets the TTL to the cache 
 * minimum and tf_findaddr removes stale entries before searching.
 * Entries are added when a query to a name server times out beyond the
 * maximum time allowed or beyond the maximum number of retries allowed,
 * provided that the server hasn't responded in any way.  Temporary failures
 * are skipped when a name server is being considered for use.
 */

/*
 * Forward the query to get the answer since its not in the database.
 * Returns FW_OK if a request struct is allocated and the query sent.
 * Returns FW_DUP if this is a duplicate of a pending request. 
 * Returns FW_NOSERVER if there were no addresses for the nameservers.
 * Returns FW_SERVFAIL on malloc error.
 * (no action is taken on errors and qpp is not filled in.)
 */
ns_forw(nsp, msg, msglen, fp, qsp, dfd, qpp)
	struct databuf *nsp[];
	char *msg;
	int msglen;
	struct sockaddr_in *fp;
	struct qstream *qsp;
	int dfd;
	struct qinfo **qpp;
{
	register struct qinfo *qp;
	HEADER *hp;
	u_short id;
	extern char *calloc();

#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"ns_forw()\n");
#endif

	/* Don't forward if we're already working on it. */
	hp = (HEADER *) msg;
	id = hp->id;
	/* Look at them all */
	for (qp = qhead; qp!=QINFO_NULL; qp = qp->q_link) {
		if (qp->q_id == id &&
		    bcmp((char *)&qp->q_from, fp, sizeof(qp->q_from)) == 0 &&
		    ((qp->q_cmsglen == 0 && qp->q_msglen == msglen &&
		     bcmp((char *)qp->q_msg+2, msg+2, msglen-2) == 0) ||
		    (qp->q_cmsglen == msglen &&
		     bcmp((char *)qp->q_cmsg+2, msg+2, msglen-2) == 0))) {
#ifdef DEBUG
			if (debug >= 3)
				fprintf(ddt,"forw: dropped DUP id=%d\n", ntohs(id));
#endif
#ifdef STATS
			stats[S_DUPQUERIES].cnt++;
#endif
			return (FW_DUP);
		}
	}

	qp = qnew();
	if (nslookup(nsp, qp) == 0) {
#ifdef DEBUG
		if (debug >= 2)
			fprintf(ddt,"forw: no nameservers found\n");
#endif
		qfree(qp);
		return (FW_NOSERVER);
	}
	qp->q_stream = qsp;
	qp->q_curaddr = 0;
	qp->q_fwd = fwdtab;
	qp->q_dfd = dfd;
	qp->q_id = id;
	hp->id = qp->q_nsid = htons((u_short)++nsid);
	hp->ancount = 0;
	hp->nscount = 0;
	hp->arcount = 0;
	qp->q_from = *fp;
	if ((qp->q_msg = malloc((unsigned)msglen)) == NULL) {
		syslog(LOG_ERR, "forw: %m");
		qfree(qp);
		return (FW_SERVFAIL);
	}
	bcopy(msg, qp->q_msg, qp->q_msglen = msglen);
	if (!qp->q_fwd) {
		hp->rd = 0;
		qp->q_addr[0].stime = tt;
	}

	schedretry(qp, retrytime(qp));
#ifdef DEBUG
	if (debug)
		fprintf(ddt,
		   "forw: forw -> %s %d (%d) nsid=%d id=%d %dms retry %d sec\n",
			inet_ntoa(Q_NEXTADDR(qp,0)->sin_addr),
			ds, ntohs(Q_NEXTADDR(qp,0)->sin_port),
			ntohs(qp->q_nsid), ntohs(qp->q_id),
			qp->q_addr[0].nsdata->d_nstime,
			qp->q_time - tt.tv_sec);
	if ( debug >= 10)
		fp_query(msg, ddt);
#endif
	if (sendto(ds, msg, msglen, 0, (struct sockaddr *)Q_NEXTADDR(qp,0),
		   sizeof(struct sockaddr_in)) < 0){
#ifdef DEBUG
		if (debug >= 5)
			fprintf(ddt,"error returning msg errno=%d\n",errno);
#endif
	}
#ifdef STATS
	stats[S_OUTPKTS].cnt++;
#endif
	if (qpp)
		*qpp = qp;
	hp->rd = 1;
	return (0);
}

/*
 * Lookup the address for each nameserver in `nsp' and add it to
 * the list saved in the qinfo structure.
 */
nslookup(nsp, qp)
	struct databuf *nsp[];
	register struct qinfo *qp;
{
	register struct namebuf *np;
	register struct databuf *dp, *nsdp;
	register struct qserv *qs;
	register int n, i;
	struct hashbuf *tmphtp;
	char *dname, *fname;
	int oldn, naddr, class, found_arr;
	time_t curtime;
	int qcomp();

#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"nslookup(nsp=x%x,qp=x%x)\n",nsp,qp);
#endif

	naddr = n = qp->q_naddr;
	curtime = (u_long) tt.tv_sec;
	while ((nsdp = *nsp++) != NULL) {
		class = nsdp->d_class;
		dname = nsdp->d_data;
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"nslookup: NS %s c%d t%d (x%x)\n",
				dname, class, nsdp->d_type, nsdp->d_flags);
#endif
		/* skip negative entries */
		if (nsdp->d_flags & (DB_NAMEERROR | DB_DATAERROR))
			goto skipserver;

		/* don't put in people we have tried */
		for (i = 0; i < qp->q_nusedns; i++)
			if (qp->q_usedns[i] == nsdp) {
#ifdef DEBUG
				if (debug >= 2)
fprintf(ddt, "skipping used NS w/name %s\n", nsdp->d_data);
#endif /* DEBUG */
				goto skipserver;
			}

		tmphtp = ((nsdp->d_flags & DB_F_HINT) ? fcachetab : hashtab);
try_again:
		np = nlookup(dname, &tmphtp, &fname, 1);
		if (np == NULL || fname != dname) {
#ifdef DEBUG
			if (debug >= 3)
			    fprintf(ddt,"%s: not found %s %x\n",dname,fname,np);
#endif
			continue;
		}
		found_arr = 0;
		oldn = n;
		/* look for name server addresses */
		for (dp = np->n_data; dp != NULL; dp = dp->d_next) {
			if (dp->d_type != T_A || dp->d_class != class ||
			    dp->d_flags & (DB_NAMEERROR | DB_DATAERROR))
				continue;
			/*
			 * Don't use records that may become invalid to
			 * reference later when we do the rtt computation.
			 * Never delete our safety-belt information!
			 */
			if ((dp->d_zone == 0) &&
			    (dp->d_ttl < (curtime+900)) &&
			    !(dp->d_flags & DB_F_HINT) )
		        {
#ifdef DEBUG
				if (debug >= 3)
					fprintf(ddt,"nslookup: stale entry '%s'\n",
					    np->n_dname);
#endif
				/* Cache invalidate the NS RR's */
				if (dp->d_ttl < curtime)
					delete_all(np, class, T_A);
				/*
				 * Check for valid A records in the hints,
				 * since the stale cache entry may be hiding
				 * a valid hint.
				 */
				if (tmphtp != fcachetab) {
#ifdef DEBUG
					if (debug >= 3)
						fprintf(ddt,
						    "nslookup: trying hints\n");
#endif
					tmphtp = fcachetab;
					goto try_again;
				}
				n = oldn;
				break;
			}

			found_arr++;
			/* don't put in duplicates */
			qs = qp->q_addr;
			for (i = 0; i < n; i++, qs++)
				if (bcmp((char *)&qs->ns_addr.sin_addr,
				    dp->d_data, sizeof(struct in_addr)) == 0)
					goto skipaddr;
			/* Don't use a temporary failure */
#ifdef DEBUG
			if(debug >= 5)
				fprintf(ddt,"nslookup: adding NS %s\n",
				    inet_ntoa(*(struct in_addr *)dp->d_data));
#endif	
			if (tf_findaddr(*(struct in_addr *)dp->d_data))
				goto skipaddr;
			qs->ns_addr.sin_family = AF_INET;
			qs->ns_addr.sin_port = ns_port;
			qs->ns_addr.sin_addr = 
				    *(struct in_addr *)dp->d_data;
			qs->ns = nsdp;
			qs->nsdata = dp;
			qp->q_addr[n].nretry = 0;
			qs->gotresp = 0;
			n++;
			if (n >= NSMAX)
				goto out;
	skipaddr:	;
		}
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"nslookup: %d ns addrs\n", n);
#endif
		if (found_arr == 0 && qp->q_system == 0)
			(void) sysquery(dname, class, T_A);
skipserver:	;
	}
out:
#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"nslookup: %d ns addrs total\n", n);
#endif
	qp->q_naddr = n;
	if (n > 1)
		qsort((char *)qp->q_addr, n, sizeof(struct qserv), qcomp);
	return (n - naddr);
}

qcomp(qs1, qs2)
	struct qserv *qs1, *qs2;
{

	return (qs1->nsdata->d_nstime - qs2->nsdata->d_nstime);
}

/*
 * Arrange that forwarded query (qp) is retried after t seconds.
 */
schedretry(qp, t)
	struct qinfo *qp;
	time_t t;
{
	register struct qinfo *qp1, *qp2;

#ifdef DEBUG
	if (debug > 3) {
		fprintf(ddt,"schedretry(%#x, %dsec)\n", qp, t);
		if (qp->q_time)
		   fprintf(ddt,"WARNING: schedretry(%x,%d) q_time already %d\n", qp->q_time);
	}
#endif
	t += (u_long) tt.tv_sec;
	qp->q_time = t;

	if ((qp1 = retryqp) == NULL) {
		retryqp = qp;
		qp->q_next = NULL;
		return;
	}
	if (t < qp1->q_time) {
		qp->q_next = qp1;
		retryqp = qp;
		return;
	}
	while ((qp2 = qp1->q_next) != NULL && qp2->q_time < t)
		qp1 = qp2;
	qp1->q_next = qp;
	qp->q_next = qp2;
}

/*
 * Unsched is called to remove a forwarded query entry.
 */
unsched(qp)
	struct qinfo *qp;
{
	register struct qinfo *np;

#ifdef DEBUG
	if (debug > 3) {
		fprintf(ddt,"unsched(%#x, %d )\n", qp, ntohs(qp->q_id));
	}
#endif
	if( retryqp == qp )  {
		retryqp = qp->q_next;
	} else {
		for( np=retryqp; np->q_next != QINFO_NULL; np = np->q_next ) {
			if( np->q_next != qp)
				continue;
			np->q_next = qp->q_next;	/* dequeue */
			break;
		}
	}
	qp->q_next = QINFO_NULL;		/* sanity check */
	qp->q_time = 0;
}

/*
 * Retry is called to retransmit query 'qp'.
 */
retry(qp)
	register struct qinfo *qp;
{
	register int n;
	register HEADER *hp;

#ifdef DEBUG
	if (debug > 3)
		fprintf(ddt,"retry(x%x) id=%d\n", qp, ntohs(qp->q_id));
#endif
	if((HEADER *)qp->q_msg == NULL) {		/*** XXX ***/
		qremove(qp);
		return;
	}						/*** XXX ***/

	/* try next address */
	n = qp->q_curaddr;
	if (qp->q_fwd) {
		qp->q_fwd = qp->q_fwd->next;
		if (qp->q_fwd)
			goto found;
		/* out of forwarders, try direct queries */
	} else
		++qp->q_addr[n].nretry;
	if (!forward_only) {
		do {
			if (++n >= qp->q_naddr)
				n = 0;
			if (qp->q_addr[n].nretry < MAXRETRY)
				goto found;
			/*
			 * If this server hasn't responded at all, then add
			 * it to the list of temporary failures.
			 */
			if (!qp->q_addr[n].gotresp)
				(void) tf_add(qp->q_addr[n].ns_addr.sin_addr);

		} while (n != qp->q_curaddr);
	}
	/*
	 * Give up. Can't reach destination.
	 */
	hp = (HEADER *)(qp->q_cmsg ? qp->q_cmsg : qp->q_msg);
	if (qp->q_system == PRIMING_CACHE) {
		/* Can't give up priming */
		unsched(qp);
		schedretry(qp, (time_t)60*60);	/* 1 hour */
		hp->rcode = NOERROR;	/* Lets be safe, reset the query */
		hp->qr = hp->aa = 0;
		qp->q_fwd = fwdtab;
		for (n = 0; n < qp->q_naddr; n++)
			qp->q_addr[n].nretry = 0;
		return;
	}
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"give up\n");
#endif
	n = ((HEADER *)qp->q_cmsg ? qp->q_cmsglen : qp->q_msglen);
	hp->id = qp->q_id;
	hp->qr = 1;
	hp->ra = 1;
	hp->rd = 1;
	hp->rcode = SERVFAIL;
#ifdef DEBUG
	if (debug >= 10)
		fp_query(qp->q_msg, ddt);
#endif
	if (send_msg((char *)hp, n, qp)) {
#ifdef DEBUG
		if (debug)
			fprintf(ddt,"gave up retry(x%x) nsid=%d id=%d\n",
				qp, ntohs(qp->q_nsid), ntohs(qp->q_id));
#endif
	}
	qremove(qp);
	return;

found:
	if (qp->q_fwd == 0 && qp->q_addr[n].nretry == 0)
		qp->q_addr[n].stime = tt;
	qp->q_curaddr = n;
	hp = (HEADER *)qp->q_msg;
	hp->rd = (qp->q_fwd ? 1 : 0);
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"%s(addr=%d n=%d) -> %s %d (%d) nsid=%d id=%d %dms\n",
			(qp->q_fwd ? "reforw" : "resend"),
			n, qp->q_addr[n].nretry,
			inet_ntoa(Q_NEXTADDR(qp,n)->sin_addr),
			ds, ntohs(Q_NEXTADDR(qp,n)->sin_port),
			ntohs(qp->q_nsid), ntohs(qp->q_id),
			qp->q_addr[n].nsdata->d_nstime);
	if ( debug >= 10)
		fp_query(qp->q_msg, ddt);
#endif
	/* NOSTRICT */
	if (sendto(ds, qp->q_msg, qp->q_msglen, 0,
	    (struct sockaddr *)Q_NEXTADDR(qp,n),
	    sizeof(struct sockaddr_in)) < 0){
#ifdef DEBUG
		if (debug > 3)
			fprintf(ddt,"error resending msg errno=%d\n",errno);
#endif
	}
	hp->rd = 1;	/* leave set to 1 for dup detection */
#ifdef STATS
	stats[S_OUTPKTS].cnt++;
#endif
	unsched(qp);
	schedretry(qp, qp->q_fwd ? (2*RETRYBASE) : retrytime(qp));
}

/*
 * Compute retry time for the next server for a query.
 * Use a minimum time of RETRYBASE (4 sec.) or twice the estimated
 * service time; * back off exponentially on retries, but place a 45-sec.
 * ceiling on retry times for now.  (This is because we don't hold a reference
 * on servers or their addresses, and we have to finish before they time out.)
 */
time_t
retrytime(qp)
register struct qinfo *qp;
{
	time_t t;
	struct qserv *ns = &qp->q_addr[qp->q_curaddr];

#ifdef DEBUG
	if (debug > 3)
		fprintf(ddt,"retrytime: nstime %dms.\n",
		    ns->nsdata->d_nstime / 1000);
#endif
	t = (time_t) MAX(RETRYBASE, 2 * ns->nsdata->d_nstime / 1000);
	t <<= ns->nretry;
	t = MIN(t, 45);			/* max. retry timeout for now */
#ifdef notdef
	if (qp->q_system)
		return ((2 * t) + 5);	/* system queries can wait. */
#endif
	return (t);
}

qflush()
{
	while (qhead)
		qremove(qhead);
	qhead = QINFO_NULL;
}

qremove(qp)
register struct qinfo *qp;
{
#ifdef DEBUG
	if(debug > 3)
		fprintf(ddt,"qremove(x%x)\n", qp);
#endif
	unsched(qp);			/* get off queue first */
	qfree(qp);
}

struct qinfo *
qfindid(id)
register u_short id;
{
	register struct qinfo *qp;

#ifdef DEBUG
	if(debug > 3)
		fprintf(ddt,"qfindid(%d)\n", ntohs(id));
#endif
	for (qp = qhead; qp!=QINFO_NULL; qp = qp->q_link) {
		if (qp->q_nsid == id)
			return(qp);
	}
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"qp not found\n");
#endif
	return(NULL);
}

struct qinfo *
qnew()
{
	register struct qinfo *qp;

	if ((qp = (struct qinfo *)calloc(1, sizeof(struct qinfo))) == NULL) {
#ifdef DEBUG
		if (debug >= 5)
			fprintf(ddt,"qnew: calloc error\n");
#endif
		syslog(LOG_ERR, "forw: %m");
		exit(12);
	}
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"qnew(x%x)\n", qp);
#endif
	qp->q_link = qhead;
	qhead = qp;
	return( qp );
}

qfree(qp)
struct qinfo *qp;
{
	register struct qinfo *np;

#ifdef DEBUG
	if(debug > 3)
		fprintf(ddt,"qfree( x%x )\n", qp);
	if(debug && qp->q_next)
		fprintf(ddt,"WARNING:  qfree of linked ptr x%x\n", qp);
#endif
	if (qp->q_msg)
	 	free(qp->q_msg);
 	if (qp->q_cmsg)
 		free(qp->q_cmsg);
	if( qhead == qp )  {
		qhead = qp->q_link;
	} else {
		for( np=qhead; np->q_link != QINFO_NULL; np = np->q_link )  {
			if( np->q_link != qp )  continue;
			np->q_link = qp->q_link;	/* dequeue */
			break;
		}
	}
	(void)free((char *)qp);
}

/*
 * Add an Internet address to the list of temporary failures.
 * Set the time to live to min_cache_ttl.
 * Silently ignore memory allocation failures.
 */
tf_add(addr)
struct in_addr addr;
{
	register struct tfinfo *tf;
	extern struct timeval   tt;
	extern int min_cache_ttl;

#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"give up on %s\n", inet_ntoa(addr));
#endif

	if (tf_findaddr(addr))
		return;

	tf = (struct tfinfo *)calloc(1, sizeof(struct tfinfo));
	if (tf == NULL) {
#ifdef DEBUG
		if(debug > 3)
			fprintf(ddt,"tf_add: calloc error\n");
#endif

		syslog(LOG_ERR, MSGSTR(FMEMERR, "forw: %m")); /*MSG*/
		return;
	}

	tf->tf_next = tfhead;
	tf->tf_sin_addr = addr;
	tf->tf_ttl = tt.tv_sec + min_cache_ttl;
	tfhead = tf;

	return;
}

tf_findaddr(addr)
struct in_addr addr;
{
	register struct tfinfo *tf;
	register struct tfinfo *nxt_tf;

	/* Remove stale entries */
	while ((tfhead != (struct tfinfo *)0) && 
	       (tfhead->tf_ttl <= tt.tv_sec)) {
		nxt_tf = tfhead->tf_next;
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"tf_findaddr: remove stale head entry %s\n", 
			inet_ntoa(tfhead->tf_sin_addr));
#endif
		free(tfhead);
		tfhead = nxt_tf;
	}
	for (tf = tfhead; tf != (struct tfinfo *)0; tf = tf->tf_next) {
		if (tf->tf_next != (struct tfinfo *)0) {
			nxt_tf = tf->tf_next;
			if (nxt_tf->tf_ttl <= tt.tv_sec) {
				tf->tf_next = nxt_tf->tf_next;
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"tf_findaddr: remove stale entry %s\n", 
			inet_ntoa(nxt_tf->tf_sin_addr));
#endif
				free(nxt_tf);
			}
		}
	}

	for (tf = tfhead; tf != (struct tfinfo *)0; tf = tf->tf_next) {
		if (bcmp(&tf->tf_sin_addr, &addr, sizeof(struct in_addr)) 
			== 0) {
#ifdef DEBUG
			if(debug >= 5)
				fprintf(ddt,
				    "tf_findaddr: found match %s ttl %d\n",
				    inet_ntoa(addr), tf->tf_ttl - tt.tv_sec);
#endif	
			return(1);
		}
	}
	return(0);
}
