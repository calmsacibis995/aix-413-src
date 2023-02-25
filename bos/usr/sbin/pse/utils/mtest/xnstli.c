static char sccsid[] = "@(#)40	1.1  src/bos/usr/sbin/pse/utils/mtest/xnstli.c, cmdpse, bos411, 9428A410j 5/7/91 13:45:29";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1989  Mentat Inc.
 ** xnstli.c 1.13, last change 10/20/90
 **/


#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <pse/common.h>
#include "mtest.h"
#include <sys/stropts.h>
#ifdef XTI
#include <xti.h>
#else
#include <tiuser.h>
#endif

extern	boolean	CreateThread(   pfi_t pfi, u32 * thread_id_ptr, char * stptr, u32 stlen, u32 arg   );
staticf	void	do_send(   int fd, int cnt, int len, char * buf   );
staticf	void	do_recv(   int fd, int cnt, int len, char * buf   );
#ifndef DOS
staticf	void	do_read(   int fd, int cnt, int len, char * buf   );
staticf	void	do_write(   int fd, int cnt, int len, char * buf   );
#endif
#ifndef	EnterCritSec
extern	void	EnterCritSec(   void   );
#endif
extern	void	etime(   int cnt, int len, TIMER * t1, char * msg   );
#ifndef	ExitCritSec
extern	void	ExitCritSec(   void   );
#endif
extern	char	* hextostr(   char * buf, char * cp, int len   );
staticf	void	lt_error(   int fd, char * msg   );
extern	char	* malloc(   int size   );
extern	char	* strchr(   char * str, char ch   );
staticf	boolean	strtohex(   char * to, char * from, int len   );
staticf	boolean	strtoxaddr(   char * to, char * from   );
extern	char	* strtok(   char * str, char * sep   );
staticf	int	tohex(   int ch   );
extern	void	warn(   char * fmt, ...   );
staticf	char	* xaddrtostr(   char * buf, char * xaddr   );
	void	xdgclient(   char * cmd   );
staticf	void	xdgsrvc(   u32 arg   );
	void	xdgsrvr(   void   );
	void	xsclient(   char * cmd   );
staticf	void	xssrvc(   u32 arg   );
	void	xssrvr(   void   );
staticf	boolean	xtparse(   char * cmd, int * dir, char * addr, int * cnt, int * len, int * rep   );
staticf	void	xt_send_ack(   int fd   );
staticf	void	xt_wait_for_ack(   int fd   );

extern	boolean	single_thread;
extern	int	t_errno;

#ifndef DOS
staticf	void
do_read (fd, cnt, len, buf)
	int	fd;
	int	cnt;
	int	len;
	char	* buf;
{
	char	* cp1;
	int	i1;
	int	nrd;

	if ( stream_ioctl(fd, I_PUSH, "ntirdwr") == -1 ) {
		printf("do_read: couldn't push ntirdwr, %s\n", errmsg(0));
		t_close(fd);
		free(buf);
		ExitThread(0,1);
	}
	while ( cnt-- ) {
		i1 = len;
		for ( cp1 = buf; i1; cp1 += nrd, i1 -= nrd) {
			if ( (nrd = stream_read(fd, cp1, i1)) == -1 ) {
				printf("do_read: stream_read failed, %s\n", errmsg(0));
				t_close(fd);
				free(buf);
				ExitThread(0, 1);
			}
		}
	}
	if ( stream_ioctl(fd, I_POP, 0) == -1 )
		printf("do_read: I_POP failed, %s\n", errmsg(0));
}
#endif

staticf	void
do_recv (fd, cnt, len, buf)
	int	fd;
	int	cnt;
	int	len;
	char	* buf;
{
	char	* cp1;
	int	flags;
	int	i1;
	int	nrd;

	flags = 0;	
	while ( cnt-- ) {
		i1 = len;
		for ( cp1 = buf; i1; cp1 += nrd, i1 -= nrd) {
			if ( (nrd = t_rcv(fd, cp1, i1, &flags)) == -1 ) {
				lt_error(fd, "do_recv: t_rcv failed");
				t_close(fd);
				free(buf);
				ExitThread(0, 1);
			}
		}
	}
}

staticf	void
do_send (fd, cnt, len, buf)
	int	fd;
	int	cnt;
	int	len;
	char	* buf;
{
	char	* cp1;
	int	i1;
	int	nsnt;

	while ( cnt-- ) {
		i1 = len;
		for ( cp1 = buf; i1; cp1 += nsnt, i1 -= nsnt) {
			if ( (nsnt = t_snd(fd, cp1, i1, 0)) == -1 ) {
				lt_error(fd, "do_send: t_snd failed");
				t_close(fd);
				free(buf);
				ExitThread(0, 1);
			}
		}
	}
}

#ifndef DOS
staticf	void
do_write (fd, cnt, len, buf)
	int	fd;
	int	cnt;
	int	len;
	char	* buf;
{
	char	* cp1;
	int	i1;
	int	nsnt;

	if ( stream_ioctl(fd, I_PUSH, "ntirdwr") == -1 ) {
		printf("do_write: couldn't push ntirdwr, %s\n", errmsg(0));
		t_close(fd);
		free(buf);
		ExitThread(0,1);
	}
	while ( cnt-- ) {
		i1 = len;
		for ( cp1 = buf; i1; cp1 += nsnt, i1 -= nsnt) {
			if ( (nsnt = stream_write(fd, cp1, i1)) == -1 ) {
				printf("do_write: stream_write failed, %s\n", errmsg(0));
				t_close(fd);
				free(buf);
				ExitThread(0, 1);
			}
		}
	}
	if ( stream_ioctl(fd, I_POP, 0) == -1 )
		printf("do_write: I_POP failed, %s\n", errmsg(0));
}
#endif

staticf	void
lt_error (fd, msg)
	char	* msg;
{
	int	tlk;
	
	t_error(msg);
	if ( t_errno != TLOOK )
		return;
	if ( (tlk = t_look(fd)) == -1 )
		printf("t_look failed, t_errno %d, errno %d\n", t_errno, errno);
	else
		printf("TLOOK code: 0x%x\n", tlk);
}

staticf boolean
strtohex (to, from, len)
	char	* to;
	char	* from;
	int	len;
{
	int	i1;
	
	if ( strncmp(from, "0x", 2) == 0 )
		from += 2;
	while ( len-- ) {
		if ( (i1 = tohex((int)*from++)) == -1 )
			return false;
		*to = i1 << 4;
		if ( (i1 = tohex((int)*from++)) == -1 )
			return false;
		*to++ |= i1;
	}
	return true;
}

staticf	boolean
strtoxaddr (addr, cp)
	char	* addr;
	char	* cp;
{
	char	* cp1;

	memset(addr, 0, 12);	
	if ( cp1 = strchr(cp, '/') ) {
		*cp1 = '\0';
		if ( !strtohex(addr, cp, 4) )
			return false;
		cp = ++cp1;
	}
	return strtohex(&addr[4], cp, 6);
}

staticf int
tohex (ch_param)
	int	ch_param;
{
	char	ch = (char)ch_param;
	
	if ( ch >= '0'  &&  ch <= '9' )
		return ch - '0';
	if ( ch >= 'a'  &&  ch <= 'f' )
		return 10 + ch - 'a';
	if ( ch >= 'A'  &&  ch <= 'F' )
		return 10 + ch - 'A';
	return -1;
}

staticf	char *
xaddrtostr (buf, xaddr)
	char	* buf;
	char	* xaddr;
{
	hextostr(buf, xaddr, 4);
	hextostr(&buf[9], &xaddr[4], 6);
	hextostr(&buf[22], &xaddr[10], 2);
	buf[8] = buf[21] = '/';
	return buf;
}

void
xdgclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	int	fd;
	int	flags;
	int	i1;
	char	opt_buf[256];
	int	len;
	int	rep, rqst;
	struct t_unitdata tud;
	TIMER	t1;
	char	xns_addr[12];

	EnterCritSec();
	/* Critical around xtparse until we replace use of strtok */
	if ( !xtparse(cmd, &rqst, xns_addr, &cnt, &len, &rep) ) {
		ExitCritSec();
		free(cmd);
		warn("xdgclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	free(cmd);
	xns_addr[11] = XDGTST_SOCKET;
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		warn("xdgclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = t_open("/dev/xdg", 2, (struct t_info *)0)) == -1) {
		t_error("xdgclient: open of /dev/xdg failed");
bye1:;
		free(buf);
		return;
	}
	if (t_bind(fd, (struct t_bind *)0, (struct t_bind *)0) == -1) {
		lt_error(fd, "xdgclient: bind failed");
bye2:;
		t_close(fd);
		goto bye1;
	}
	tud.addr.buf = xns_addr;
	tud.addr.len = sizeof(xns_addr);
	tud.opt.buf = nilp(char);
	tud.opt.len = 0;
	tud.udata.buf = buf;
	tud.udata.len = 1;
	/* Send the first datagram to get the server's attention */
	if ( t_sndudata(fd, &tud) == -1 ) {
		lt_error(fd, "xdgclient: t_sndudata failed");
		goto bye2;
	}
	tud.addr.buf = buf;
	tud.addr.maxlen = len;
	tud.opt.buf = opt_buf;
	tud.opt.maxlen = sizeof(opt_buf);
	tud.udata.buf = xns_addr;
	tud.udata.maxlen = sizeof(xns_addr);
	/* Get back the ACK with the new target address */
	if ( (i1 = t_rcvudata(fd, &tud, &flags)) == -1 ) {
		lt_error(fd, "xdgclient: t_rcvudata failed");
		goto bye2;
	}
	printf("xdgclient: (%d) sending to '%s'\n", fd, xaddrtostr(buf, xns_addr));
	tud.addr.buf = xns_addr;
	tud.addr.len = sizeof(xns_addr);
	tud.opt.buf = nilp(char);
	tud.opt.len = 0;
	tud.udata.buf = buf;
	tud.udata.len = len;
	buf[0] = XDGTST_START;
	if ( cnt < 2 )
		cnt = 2;
	cnt += 5;
	i1 = cnt;
	SET_TIME(&t1);
	while ( i1-- ) {
		if ( i1 == 5 )
			buf[0] = XDGTST_STOP;
		if ( t_sndudata(fd, &tud) == -1 ) {
			lt_error(fd, "xdg_client: t_sndudata failed");
			goto bye2;
		}
	}
	etime(cnt, len, &t1, buf);
	printf("xdgclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
	t_close(fd);
	free(buf);
}

staticf	void
xdgsrvc (arg)
	u32	arg;
{
	char	addr_buf[256];
	char	* buf;
	int	cnt = 0;
	int	fd;
	int	flags;
	char	opt_buf[256];
	int	state = XDGTST_IDLE;
	struct t_unitdata tud;
	TIMER	t1;

	fd = (int)arg;
	if ( (buf = malloc(UNIT_DATA_TEST_MAX_SIZE)) == nilp(char) ) {
		t_close(fd);
		warn("xdgsrvc: out of memory\n");
		ExitThread(0, 1);
	}
#ifdef	VMS
	map_region(fd, buf, UNIT_DATA_TEST_MAX_SIZE);
#endif
	tud.addr.buf = addr_buf;
	tud.addr.maxlen = sizeof(addr_buf);
	tud.opt.buf = opt_buf;
	tud.opt.maxlen = sizeof(opt_buf);
	tud.udata.buf = buf;
	tud.udata.maxlen = UNIT_DATA_TEST_MAX_SIZE;
	for (;;) {
		if ( t_rcvudata(fd, &tud, &flags) == -1 ) {
			lt_error(fd, "xdgsrvc: t_rcvudata failed\n");
			break;
		}
		cnt++;
		/* Do a quick check for the most common state */
		if ( state == XDGTST_START  &&  buf[0] == XDGTST_START )
			continue;
		/* Some kind of transition is happening... */
		switch (state) {
		case XDGTST_START:
			if ( buf[0] == XDGTST_STOP ) {
				etime(cnt, tud.udata.len, &t1, buf);
				break;
			}
			printf("xdgsrvc: (%d) bogus data code 0x%x while active\n", fd, buf[0] & 0xFF);
			goto bye;
		case XDGTST_IDLE:
			switch (buf[0]) {
			case XDGTST_START:
				state = XDGTST_START;
				SET_TIME(&t1);
				continue;
			default:
				printf("xdgsrvc: (%d) bogus data code 0x%x while idle\n", fd, buf[0] & 0xFF);
				goto bye;
			}
			/*NOTREACHED*/
		}
		printf("xdgsrvc(%d): Cnt: %d, len %d, %s\n", fd, cnt, tud.udata.len, buf);
		break;
	}
bye:;
#ifdef	VMS
	unmap_region(fd);
#endif
	t_close(fd);
	free(buf);
}

void
xdgsrvr () {
	char	buf[128];
	char	ch;
	int	err;
	int	fd;
	int	flags;
	int	nfd;
	char	opt_buf[256];
	struct t_bind tbind;
	struct t_unitdata tud;
	char	our_xns_addr[12];
	char	its_xns_addr[12];

	/* Get the datagram server stream */
	if ( (fd = t_open("/dev/xdg", 2, (struct t_info *)0)) == -1 ) {
		lt_error(fd, "xdgsrvr: t_open of /dev/xdg failed");
		return;
	}
	our_xns_addr[10] = 0;
	our_xns_addr[11] = XDGTST_SOCKET;
	tbind.addr.len = sizeof(our_xns_addr);
	tbind.addr.maxlen = sizeof(our_xns_addr);
	tbind.addr.buf = our_xns_addr;
	tbind.qlen = 0;
	if ( t_bind(fd, &tbind, &tbind) == -1 ) {
		lt_error(fd, "xdgsrvr: t_bind failed");
		t_close(fd);
		return;
	}
	if ( our_xns_addr[10] != 0  ||  our_xns_addr[11] != XDGTST_SOCKET ) {
		warn("xdgsrvr: one already running, bye\n");
		ExitThread(0, 1);
	}
	printf("xdgsrvr: bound to XNS address: %s\n", xaddrtostr(buf, our_xns_addr));
	for ( ;; ) {
		tud.addr.buf = its_xns_addr;
		tud.addr.maxlen = sizeof(its_xns_addr);
		tud.opt.buf = opt_buf;
		tud.opt.maxlen = sizeof(opt_buf);
		tud.udata.buf = &ch;
		tud.udata.maxlen = 1;
		/* Wait for a client to appear */
		if ( (err = t_rcvudata(fd, &tud, &flags)) != 0 ) {
			lt_error(fd, "xdgsrvr: t_rcvudata failed");
			break;
		}
		if ( single_thread )
			nfd = fd;
		else {
			/* Get a new socket to handle this service */
			if ( (nfd = t_open("/dev/xdg", 2, (struct t_info *)0)) == -1 ) {
				t_error("xdgsrvr: t_open of /dev/xdg failed");
				continue;
			}
			tbind.addr.len = sizeof(our_xns_addr);
			tbind.addr.maxlen = sizeof(our_xns_addr);
			tbind.addr.buf = our_xns_addr;
			tbind.qlen = 0;
			if ( t_bind(nfd, (struct t_bind *)0, &tbind) == -1 ) {
				lt_error(nfd, "xdgsrvr: t_bind failed");
				t_close(nfd);
				continue;
			}
			printf("xdgsrvr: handling connection from %s on fd %d (%s)\n", xaddrtostr(buf, its_xns_addr), nfd
			, xaddrtostr(&buf[64], our_xns_addr));
			/* Start a new thread */
			if ( (err = CreateThread((pfi_t)xdgsrvc, nilp(u32), nilp(char), (u32)STACK_SIZE, (u32)nfd)) != 0 ) {
				t_close(nfd);
				printf("xdgsrvr: couldn't create service thread (error code %d)\n", err);
				continue;
			}
		}
		/* Now tell the client who to talk to */
		tud.addr.buf = its_xns_addr;
		tud.addr.len = sizeof(its_xns_addr);
		tud.opt.len = 0;
		tud.udata.buf = our_xns_addr;	/* The new address */
		tud.udata.len = sizeof(our_xns_addr);
		if ( t_sndudata(fd, &tud) == -1 ) {
			lt_error(fd, "xdgsrvr: t_sndudata failed");
			if ( !single_thread )
				t_close(nfd);	/* This should get the service thread's attention */
		}
#ifdef	FORKER
		else if ( !single_thread )
			t_close(nfd);
#endif
		if ( single_thread )
			xdgsrvc((u32)fd);
	}
	t_close(fd);
}

void
xsclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	int	fd;
	int	flags;
	int	len;
	int	rep;
	int	rqst;
	struct t_call	tcall;
	TIMER	t1;
	char	xns_addr[12];

	EnterCritSec();
	/* Critical around xtparse until we replace use of strtok */
	if ( !xtparse(cmd, &rqst, xns_addr, &cnt, &len, &rep) ) {
		ExitCritSec();
		free(cmd);
		warn("xsclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		free(cmd);
		warn("xsclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = t_open("/dev/xs", 2, (struct t_info *)0)) == -1) {
		lt_error(fd, "xsclient: open of /dev/xs failed");
		free(cmd);
		free(buf);
		return;
	}
#ifdef	VMS
	map_region(fd, buf, len);
#endif
	if (t_bind(fd, (struct t_bind *)0, (struct t_bind *)0) == -1) {
		lt_error(fd, "xsclient: bind failed");
		goto bye2;
	}
	xns_addr[11] = XSTST_SOCKET;
	tcall.addr.buf = xns_addr;
	tcall.addr.len = sizeof(xns_addr);
	tcall.addr.maxlen = sizeof(xns_addr);
	tcall.opt.buf = (char *)0;
	tcall.opt.len = 0;
	tcall.opt.maxlen = 0;
	tcall.udata.buf = (char *)0;
	tcall.udata.len = 0;
	tcall.udata.maxlen = 0;
	/* Connect to the server. */
	if (t_connect(fd, &tcall, &tcall) == -1) {
		lt_error(fd, "xsclient: t_connect failed");
		goto bye2;
	}
	printf("xsclient: connected to ... on fd %d\n", fd);
	while (rep--) {
		/* send the server the command */
		if (t_snd(fd, cmd, strlen(cmd), 0) == -1) {
			lt_error(fd, "xsclient: couldn't send command");
			goto bye2;
		}
		xt_wait_for_ack(fd);
		SET_TIME(&t1);
		switch (rqst) {
		case CLIENT_SEND:
			do_send(fd, cnt, len, buf);
			xt_wait_for_ack(fd);
			break;
		case CLIENT_RECV:
			do_recv(fd, cnt, len, buf);
			xt_send_ack(fd);
			break;
#ifndef DOS
		case CLIENT_WRITE:
			do_write(fd, cnt, len, buf);
			xt_wait_for_ack(fd);
			break;
		case CLIENT_READ:
			do_read(fd, cnt, len, buf);
			xt_send_ack(fd);
			break;
#endif
		}
		etime(cnt, len, &t1, buf);
		printf("xsclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
	}
	/* Perform an orderly release. */
	if (t_sndrel(fd) == -1) {
		lt_error(fd, "xsclient: t_sndrel failed");
		goto bye2;
	}
	(void)t_rcvrel(fd);
	printf("xsclient: (%d) disconnected\n", fd);
bye2:;
#ifdef	VMS
	unmap_region(fd);
#endif
bye1:;
	free(cmd);
	free(buf);
	/* Close the transport stream. */
	t_close(fd);
}

staticf	void
xssrvc (arg)
	u32	arg;
{
	char	* buf;
	char	cmd[128];
	int	cnt;
	char	* cp1;
	int	dir;
	int	fd;
	int	flags;
	int	len;
	char	msg[128];
	int	rep;
	TIMER	t1;

	fd = (int)arg;
	flags = 0;
	buf = nilp(char);
	printf("xssrvc: handling connection from ... on fd %d\n", fd);
	for ( ;; ) {
		/* Read the next command */
		for ( cp1 = cmd; cp1 < A_END(cmd); cp1++ ) {
			if ( t_rcv(fd, cp1, 1, &flags) == -1 ) {
				if ( t_errno == TLOOK  &&  t_look(fd) == T_ORDREL ) {
					/* Perform an orderly release. */
					if (t_rcvrel(fd) == -1 ) {
						lt_error(fd, "xssrvc: rcvrel failed");
						printf("xssrvc: current state: 0x%x\n", t_getstate(fd));
					} else if ( t_sndrel(fd) == -1 ) {
						lt_error(fd, "xssrvc: sndrel failed");
						printf("xssrvc: current state: 0x%x\n", t_getstate(fd));
					}
				} else
					lt_error(fd, "xssrvc: error receiving command");
				printf("xssrvc: (%d) disconnected\n", fd);
bye:;
				/* Close the transport stream. */
				t_close(fd);
				if ( buf ) {
#ifdef	VMS
					unmap_region(fd);
#endif
					free(buf);
				}
				return;
			}
			if ( *cp1 == '\n' )
				break;
		}
		if ( *cp1 != '\n' ) {
			printf("xssrvc: (%d) bogus command received\n", fd);
			goto bye;
		}
		*++cp1 = '\0';
		EnterCritSec();
		/* Critical around xtparse until we replace use of strtok */
		if ( !xtparse(cmd, &dir, msg, &cnt, &len, &rep) ) {
			ExitCritSec();
			warn("xssrvc: couldn't parse command '%s'\n");
			goto bye;
		}
		ExitCritSec();
		printf("xssrvc: (%d) command: %s", fd, cmd);
		if ( (buf = malloc(len)) == nilp(char) ) {
			t_close(fd);
			warn("xssrvc: (%d) out of memory\n", fd);
			ExitThread(0, 1);
		}
#ifdef	VMS
		map_region(fd, buf, len);
#endif
		flags = 0;
		xt_send_ack(fd);
		SET_TIME(&t1);
		switch (dir) {
		case SERVER_SEND:
			do_send(fd, cnt, len, buf);
			xt_wait_for_ack(fd);
			break;
		case SERVER_RECV:
			do_recv(fd, cnt, len, buf);
			xt_send_ack(fd);
			break;
#ifndef DOS
		case SERVER_READ:
			do_read(fd, cnt, len, buf);
			xt_send_ack(fd);
			break;
		case SERVER_WRITE:
			do_write(fd, cnt, len, buf);
			xt_wait_for_ack(fd);
			break;
#endif
		}
		etime(cnt, len, &t1, buf);
		printf("xssrvc(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
#ifdef	VMS
		unmap_region(fd);
#endif
		free(buf);
		buf = nilp(char);
	}
}

void
xssrvr () {
	char	buf[128];
	int	err;
	int	fd;
	int	nfd;
	struct t_bind tbind;
	struct t_call tcall;
	u32	tid;
	char	xns_addr[12];

	if ( (fd = t_open("/dev/xs", 2, (struct t_info *)0)) == -1 ) {
		lt_error(fd, "t_open of /dev/xs failed");
		return;
	}
	xns_addr[10] = 0;
	xns_addr[11] = XSTST_SOCKET;
	tbind.addr.len = sizeof(xns_addr);
	tbind.addr.maxlen = sizeof(xns_addr);
	tbind.addr.buf = xns_addr;
	tbind.qlen = 5;
	if ( t_bind(fd, &tbind, &tbind) == -1 ) {
		lt_error(fd, "xssrvr: t_bind failed");
		t_close(fd);
		return;
	}
	if ( xns_addr[10] != 0  ||  xns_addr[11] != XSTST_SOCKET ) {
		warn("xssrvr: one already running, bye\n");
		ExitThread(0, 1);
	}
	printf("xssrvr: bound to XNS address: %s\n", xaddrtostr(buf, xns_addr));
	/* Set the tcall structure for the t_listen(). */
	tcall.addr.buf = xns_addr;
	tcall.addr.maxlen = sizeof(xns_addr);
	tcall.opt.buf = (char *)buf;
	tcall.opt.maxlen = sizeof(buf);
	tcall.udata.buf = (char *)buf;
	tcall.udata.maxlen = sizeof(buf);
	while (t_listen(fd, &tcall) != -1) {
		/* Open a new stream to accept this connection. */
		if ((nfd = t_open("/dev/xs", 2, (struct t_info *)0)) == -1) {
			lt_error(nfd, "xssrvr: t_open failed");
			t_close(fd);
			return;
		}
		if (t_bind(nfd, (struct t_bind *)0, (struct t_bind *)0) == -1) {
			lt_error(nfd, "xssrvr: t_bind failed");
bye1:;
			t_close(fd);
			t_close(nfd);
			return;
		}
		/* Accept the connection on the new stream. */
		if (t_accept(fd, nfd, &tcall) == -1) {
			lt_error(fd, "xssrvr: t_accept failed");
			goto bye1;
		}
		if ( single_thread )
			xssrvc((u32)nfd);
		else if ( (err = CreateThread((pfi_t)xssrvc, &tid, nilp(char), (u32)STACK_SIZE, (u32)nfd)) != 0 ) {
			warn("xssrvr: couldn't create service thread (error code %d)\n", err);
			t_close(nfd);
		}
#ifdef	FORKER
		else
			t_close(nfd);
#endif
	}
	lt_error(fd, "xssrvr: t_listen failed");
}

staticf	boolean
xtparse (cmd, dir, addr, cnt, len, rep)
	char	* cmd;
	int	* dir;
	char	* addr;
	int	* cnt;
	int	* len;
	int	* rep;
{
	char	buf[128];
	char	* cp1;
	char	* sep = " \t\n";
	
	strcpy(buf, cmd);
	if ( !(cp1 = strtok(buf, sep)) )
		return false;
	if ( strncmp(cp1, "send", 4) == 0 )
		*dir = CLIENT_SEND;
	else if ( strncmp(cp1, "recv", 4) == 0 )
		*dir = CLIENT_RECV;
	else if ( strncmp(cp1, "sndu", 4) == 0 )
		*dir = CLIENT_SNDU;
	else if ( strncmp(cp1, "read", 4) == 0 )
		*dir = CLIENT_READ;
	else if ( strncmp(cp1, "write", 5) == 0 )
		*dir = CLIENT_WRITE;
	else
		return false;
	if ( !(cp1 = strtok(nilp(char), sep))  ||  !strtoxaddr(addr, cp1) )
		return false;
	if ( !(cp1 = strtok(nilp(char), sep))  ||  (*cnt = atoi(cp1)) <= 0 )
		return false;
	if ( !(cp1 = strtok(nilp(char), sep))  ||  (*len = atoi(cp1)) <= 0 )
		return false;
	if ( !(cp1 = strtok(nilp(char), sep)) ) {
		*rep = 1;
		return true;
	}
	if ( (*rep = atoi(cp1)) <= 0 )
		return false;
	return true;
}

staticf	void
xt_send_ack (fd)
	int	fd;
{
	if ( t_snd(fd, "a", 1, 0) > 0 )
		return;
	lt_error(fd, "xt_send_ack: t_snd failed");
	t_close(fd);
	ExitThread(0, 1);
}

staticf	void
xt_wait_for_ack (fd)
	int	fd;
{
	char	ch;
	int	flags;

	flags = 0;	
	if ( t_rcv(fd, &ch, 1, &flags) <= 0 ) {
		lt_error(fd, "xt_wait_for_ack: t_rcv failed");
		t_close(fd);
		ExitThread(0, 1);
	}
	if ( ch == 'a' )
		return;
	printf("xt_wait_for_ack: (%d) bad ACK character 0x%x\n", fd, ch & 0xFF);
	t_close(fd);
	ExitThread(0, 1);
}
