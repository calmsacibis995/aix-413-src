static char sccsid[] = "@(#)90	1.9  src/bos/usr/sbin/netstat/unix.c, cmdnet, bos411, 9433B411a 8/17/94 13:51:49";
/*
 * COMPONENT_NAME: (CMDNET) Network commands. 
 *
 * FUNCTIONS: unixpr, unixdomain 
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

/*
 * Display protocol blocks in the unix domain.
 */
#include <sys/param.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/un.h>
#include <sys/unpcb.h>
#define	_KERNEL
#include <sys/file.h>
#include <sys/var.h>

#include <nl_types.h>
#include "netstat_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s) 

/* The folowing were declared in file.h in ACIS 4.3 */
/* By convention, dot h files shouldn't declare data. */
struct	file *fileptr, *fileNFILE;
int	nfile;

int	Aflag;

unixpr(varaddr, fileaddr, unixsw)
	off_t varaddr, fileaddr;
	struct protosw *unixsw;
{
	register struct file *fp;
	struct file *filep;
	struct socket sock, *so = &sock;
	struct var v;

	if (varaddr == 0 || fileaddr == 0) {
		printf(MSGSTR(FLE_NO_NMLST, "v or file not in namelist.\n")); /*MSG*/
		return;
	}
	if (!kvm_read(varaddr, (char *)&v, sizeof (v))) {
		printf(MSGSTR(NFILE_BAD_RD, "v: bad read.\n")); /*MSG*/
		return;
	}
	nfile = v.v_file;
	fileptr = (struct file *)calloc(nfile, sizeof (struct file));
	if (fileptr == (struct file *)0) {
		printf(MSGSTR(OUT_OF_MEM_FTAB, "Out of memory (file table).\n")); /*MSG*/
		return;
	}
	if (!kvm_read(fileaddr, fileptr, nfile * sizeof(struct file))) {
		printf(MSGSTR(FTAB_RD_ERR, "File table read error.\n")); /*MSG*/
		return;
	}
	fileNFILE = fileptr + nfile;
	for (fp = fileptr; fp < fileNFILE; fp++) {
		if (fp->f_type != DTYPE_SOCKET)
			continue;
		if (!kvm_read((off_t)fp->f_data, (char *)so, sizeof (*so)))
			continue;
		/* kludge */
		if (so->so_proto >= unixsw && so->so_proto <= unixsw + 2)
			if (so->so_pcb)
				unixdomainpr(so, fp->f_data);
	}
	free((char *)fileptr);
}

static	char *socktype[] =
    { "#0", "stream", "dgram", "raw", "rdm", "seqpacket" };

unixdomainpr(so, soaddr)
	register struct socket *so;
	caddr_t soaddr;
{
	struct unpcb unpcb, *unp = &unpcb;
	struct mbuf mbuf, *m;
	struct sockaddr_un *sa;
	static int first = 1;

	if (!kvm_read((off_t)so->so_pcb, (char *)unp, sizeof (*unp)))
		return;
	if (unp->unp_addr) {
		m = &mbuf;
		if (!kvm_read((off_t)unp->unp_addr, (char *)m, sizeof (*m)))
			m = (struct mbuf *)0;
		sa = (struct sockaddr_un *)(m->m_dat);
	} else
		m = (struct mbuf *)0;
	if (first) {
		printf(MSGSTR(ACT_UNX_DM_SOCK, "Active UNIX domain sockets\n")); /*MSG*/
		printf(MSGSTR(ADDR_HDING, "SADR/PCB  Type   Recv-Q Send-Q Inode    Conn     Refs     Nextref  Addr\n")); /*MSG*/
		first = 0;
	}
	printf(MSGSTR(ADDR_INFO, "%8x %-6.6s %6d %6d %8x %8x %8x %8x"), /*MSG*/
	    soaddr, socktype[so->so_type], so->so_rcv.sb_cc, so->so_snd.sb_cc,
	    unp->unp_vnode, unp->unp_conn,
	    unp->unp_refs, unp->unp_nextref);
	if (m)
		printf(" %.*s", m->m_len - sizeof(sa->sun_family),
		    sa->sun_path);
	printf (MSGSTR(UPCB_INFO, "\n%8x\n"), so->so_pcb);
}
