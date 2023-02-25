static char sccsid[] = "@(#)85	1.7  src/bos/usr/sbin/netstat/host.c, cmdnet, bos411, 9428A410j 4/26/91 18:10:16";
/*
 * COMPONENT_NAME: (CMDNET) Network commands 
 *
 * FUNCTIONS: host_pr 
 *
 * ORIGINS: 26 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef	HAVE_IMP
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netinet/in.h>
#include <netimp/if_imp.h>
#include <netimp/if_imphost.h>

extern 	int nflag;
extern	char *inetname();

#include <nl_types.h>
#include "netstat_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s) 

/*
 * Print the host tables associated with the ARPANET IMP.
 * Symbolic addresses are shown unless the nflag is given.
 */
hostpr(impsoftcaddr, nimpaddr)
	off_t impsoftcaddr, nimpaddr;
{
	struct mbuf *hosts, mb;
	struct imp_softc imp_softc;
	register struct mbuf *m;
	register struct hmbuf *mh;
	register struct host *hp;
	char flagbuf[10], *flags;
	int i, nimp;

	if (impsoftcaddr == 0) {
		printf(MSGSTR(SYM_NO_NMLST, "imp_softc: symbol not in namelist\n"));
		return;
	}
	if (nimpaddr == 0) {
		printf(MSGSTR(NIMP_NO_SYM, "nimp: symbol not in namelist\n"));
		return;
	}
	kvm_read(nimpaddr, (char *)&nimp, sizeof (nimp));
	for (i = 0; i < nimp; i++) {
	    kvm_read(impsoftcaddr, (char *)&imp_softc, sizeof (imp_softc));
	    impsoftcaddr += sizeof (imp_softc);
	    m = imp_softc.imp_hosts;
	    printf(MSGSTR(IMP_HSTTAB, "IMP%d Host Table\n"), i);
	    printf(MSGSTR(HEADING, "Flags Host   Imp      Qcnt Q Address RFNM Timer\n")); /*MSG*/
	    while (m) {
		kvm_read((off_t)m, (char *)&mb, sizeof (mb));
		m = &mb;
		mh = (struct hmbuf *)(m->m_dat);
		if (mh->hm_count == 0) {
			m = m->m_next;
			continue;
		}
		for (hp = mh->hm_hosts; hp < mh->hm_hosts + HPMBUF; hp++) {
			if ((hp->h_flags&HF_INUSE) == 0 && hp->h_timer == 0)
				continue;
			flags = flagbuf;
			*flags++ = hp->h_flags&HF_INUSE ? 'A' : 'F';
			if (hp->h_flags&HF_DEAD)
				*flags++ = 'D';
			if (hp->h_flags&HF_UNREACH)
				*flags++ = 'U';
			*flags = '\0';
			printf("%-5.5s %-6d %-8d %-4d %-9x %-4d %d\n",
				flagbuf,
				hp->h_host,
				ntohs(hp->h_imp),
				hp->h_qcnt,
				hp->h_q,
				hp->h_rfnm,
				hp->h_timer);
		}
		m = m->m_next;
	    }
	}
}

impstats(impsoftcaddr, nimpaddr)
	off_t impsoftcaddr, nimpaddr;
{
	struct imp_softc imp_softc;
	int i, nimp;
	extern char *plural();

	if (impsoftcaddr == 0 || nimpaddr == 0)
		return;
	kvm_read(nimpaddr, (char *)&nimp, sizeof (nimp));
	for (i = 0; i < nimp; i++) {
		kvm_read(impsoftcaddr, (char *)&imp_softc, sizeof (imp_softc));
		impsoftcaddr == sizeof(imp_softc);
		printf(MSGSTR(IMP_STATS, "imp%d statistics:\n"), i);
#define ndx(f) ((f) == 1 ? 0 : 1)
#define	p(f,i,m) printf(MSGSTR(i+ndx(imp_softc.f), m), imp_softc.f, plural(imp_softc.f))
		p(imp_if.if_ipackets, IMP_INMSG, "\t%u input message%s\n");
		p(imp_if.if_opackets, IMP_OUTMSG, "\t%u output message%s\n");
		printf(MSGSTR(IMP_BLOCKED, "\t%u times output blocked at least %d sec.\n"),
		    imp_softc.imp_block, IMP_OTIMER);
		p(imp_incomplete, IMP_INCOM, "\t%u \"incomplete\" message%s\n");
		p(imp_lostrfnm, IMP_LOST_RFNM, "\t%u lost RFNM message%s\n");
		p(imp_badrfnm, IMP_LATE_RFNM, "\t%u late/bogus RFNM/incomplete message%s\n");
		p(imp_garbage, IMP_UNKMSG, "\t%u unknown message type%s\n");
	}
}
#else
hostpr() { }
#endif
