static char sccsid[] = "@(#)85	1.7  src/bos/usr/sbin/crash/socket.c, cmdcrash, bos411, 9428A410j 12/20/91 18:46:49";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prsock,prsock_sb
 *
 * ORIGINS: 27,3
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
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	"crash.h"
#include	"sys/file.h"
#include	"sys/inode.h"
#include	"sys/var.h"
#include	"sys/stat.h"
#include	"sys/socket.h"
#include	"sys/socketvar.h"

prsock(c, ext, all)
register  int	c;
int		ext;
int		all;
{
	struct	file	fbuf;
	struct	file	*filep;
	struct vnode	vn;
	struct inode	inode;
	struct stat	buf;
	struct socket	sock;
	struct socket	*so;
	struct sockbuf	*sb;
	register int	i;
	register int	start, end;

	if (all) {
	    start = 0;
	    end = v.v_file;
	} else {
	    start = c;
	    end = c + 1;
	}

	filep = (struct file *)SYM_VALUE(File);
	for (i = start; i < end; i++) {
		if(readmem(&fbuf, filep + i, sizeof fbuf, CURPROC) != sizeof fbuf) {
			printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_1, "0452-329: Cannot read file table entry %3d.\n") , i);
			return;
		}

		if(fbuf.f_type != DTYPE_SOCKET)
			continue;

		if(readmem(&sock, fbuf.f_data, sizeof(sock), CURPROC) != 
			sizeof (sock)) {
			printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_2, "0452-330: Cannot read socket at address 0x%8x.\n") ,fbuf.f_data);
			return;
		}
		printf("SLOT:%5d  type:0x%04x  opts:0x%04x  linger:0x%04x\n",i,
			sock.so_type, sock.so_options, sock.so_linger);
		printf("     state:0x%04x  pcb:0x%08x  proto:0x%08x\n",
			sock.so_state, sock.so_pcb, sock.so_proto);
		printf("     q0:0x%08x  q0len:%5d q:0x%08x\n", 
			sock.so_q0, sock.so_q0len, sock.so_q);
		printf("     qlen:%5d  qlimit:%5u  head:0x%08x\n",
			sock.so_qlen, sock.so_qlimit, sock.so_head);
		printf("     timeo:%5d  error:%5u  oobmark:%5u  pgid:%5u\n",
			sock.so_timeo, sock.so_error, sock.so_oobmark,
			sock.so_pgid);
		printf("     oobmark:%10d\n", sock.so_oobmark);
		/* print the socket buffers if ext */
		if( ext ) {
			prsock_sb(  catgets(scmc_catd, MS_crsh, SK_SOCKET_MSG_3, "rcv") , &sock.so_rcv );
			prsock_sb(  catgets(scmc_catd, MS_crsh, SK_SOCKET_MSG_4, "snd") , &sock.so_snd );
		}
		printf("\n");
	}
}

/*
 * print a socket buffer
 */
prsock_sb( which, sb )
char	*which;		/* say which one you are */
struct sockbuf *sb;
{
	short	flags;

	printf("     %s: cc:%5u  hiwat:%5u  mbcnt:%5u  mbmax:%5u\n", which,
	    	sb->sb_cc, sb->sb_hiwat, sb->sb_mbcnt, sb->sb_mbmax);
	printf("          lowat:%5u  mb:0x%08x  events:0x%4x\n",
	    	sb->sb_lowat,sb->sb_mb, sb->sb_reqevents);
	printf("          iodone:0x%8x  ioargs:0x%08x ",
		sb->sb_iodone,sb->sb_ioarg);
	flags = sb->sb_flags;
	if( flags & SB_LOCK ) printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_5, " lock") );
	if( flags & SB_WANT ) printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_6, " want") );
	if( flags & SB_WAIT ) printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_7, " wait") );
	if( flags & SB_SEL ) printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_8, " sel") );
	if( flags & SB_KIODONE ) printf( catgets(scmc_catd, MS_crsh, SOCKET_MSG_9, " kiodone") );
	printf("\n");
}
