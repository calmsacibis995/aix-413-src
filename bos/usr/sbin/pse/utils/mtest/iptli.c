static char sccsid[] = "@(#)35	1.3  src/bos/usr/sbin/pse/utils/mtest/iptli.c, cmdpse, bos411, 9428A410j 5/22/91 11:41:05";
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

/** Copyright (c) 1990  Mentat Inc.
 ** iptli.c 2.3
 **/


#include <ctype.h>
#include <errno.h>
#ifdef	USE_STDARG
#include <stdarg.h>
#endif
#include <stdio.h>
#include <pse/common.h>
#include "mtest.h"
#include <sys/stropts.h>
#ifdef XTI
#include <xti.h>
#else
#include <tiuser.h>
#endif

#ifndef	AF_INET
#define	AF_INET		2
#endif
#ifndef	TCP_DEV_NAME
#define	TCP_DEV_NAME	"/dev/xti/tcp"
#endif
#ifndef	UDP_DEV_NAME
#define	UDP_DEV_NAME	"/dev/xti/udp"
#endif

#define	UDP_CLIENT_PORT	0xb0b0

extern	boolean	CreateThread(   pfi_t pfi, u32 * thread_id_ptr, char * stptr, u32 stlen, u32 arg   );
staticf	void	do_recv(   int fd, int cnt, int len, char * buf   );
staticf	void	do_send(   int fd, int cnt, int len, char * buf   );
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
staticf	char	* ipa_to_str(   char * buf, ipa_t * ip_addr   );
staticf	boolean	itparse(   char * cmd, int * dir, ipa_t * ip_addr, int * cnt, int * len, int * rep   );
staticf	void	it_send_ack(   int fd   );
staticf	void	it_wait_for_ack(   int fd   );
staticf	void	lt_error(   int fd, char * msg   );
extern	char	* malloc(   int size   );
extern	char	* strchr(   char * str, char ch   );
staticf	boolean	strtohex(   char * to, char * from, int len   );
staticf	void	str_to_ipa(   char * str, ipa_t * ip_addr   );
extern	char	* strtok(   char * str, char * sep   );
staticf	int	tohex(   int ch   );
	void	tcpclient(   char * cmd   );
staticf	void	tcpsrvc(   u32 arg   );
	void	tcpsrvr(   void   );
	void	udpclient(   char * cmd   );
staticf	void	udpsrvc(   u8 * arg   );
	void	udpsrvr(   void   );
#ifdef	USE_STDARG
extern	void	warn(   char * fmt, ...   );
#endif

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

#ifdef	RPROTDIS
#ifndef	RFILL
#define	RFILL	RNORM
#endif
	if ( stream_ioctl(fd, I_SRDOPT, RFILL|RPROTDIS) == -1 ) {
		printf("do_read: couldn't set RPROTDIS read mode, %s\n", errmsg(0));
		t_close(fd);
		free(buf);
		ExitThread(0,1);
	}
#else
	if ( stream_ioctl(fd, I_PUSH, "ntirdwr") == -1 ) {
		printf("do_read: couldn't push ntirdwr, %s\n", errmsg(0));
		t_close(fd);
		free(buf);
		ExitThread(0,1);
	}
#endif
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
#ifndef	RPROTDIS
	if ( stream_ioctl(fd, I_POP, 0) == -1 )
		printf("do_read: I_POP failed, %s\n", errmsg(0));
#endif
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
		tlk = 0;
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

staticf void
str_to_ipa (str, ip_addr)
	char	* str;
	ipa_t	* ip_addr;
{
auto	char	* cp;
	int	i1;
reg	u8	* u = ip_addr->ip_addr;
	
	for ( i1 = 4; i1--; ) {
		*u++ = strtol(str, &cp, 10);
		str = ++cp;
	}
	if ( *cp == ',' ) {
		i1 = strtol(str, nilp(char), 10);
		U16_TO_BE16((u16)i1, ip_addr->ip_port);
	}
	ip_addr->ip_family = AF_INET;
}

staticf int
tohex (chi)
	int	chi;
{
	char	ch = (char)chi;
	
	if ( ch >= '0'  &&  ch <= '9' )
		return ch - '0';
	if ( ch >= 'a'  &&  ch <= 'f' )
		return 10 + ch - 'a';
	if ( ch >= 'A'  &&  ch <= 'F' )
		return 10 + ch - 'A';
	return -1;
}

staticf	char *
ipa_to_str (buf, ip_addr)
	char	* buf;
	ipa_t	* ip_addr;
{
	sprintf(buf, "%d.%d.%d.%d,%d", ip_addr->ip_addr[0], ip_addr->ip_addr[1], ip_addr->ip_addr[2], ip_addr->ip_addr[3]
	, BE16_TO_U16(ip_addr->ip_port));
	return buf;
}

void
udpclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	int	fd;
	int	flags;
	int	i1;
	ipa_t	ip_addr;
	int	len;
	char	opt_buf[256];
	int	rep, rqst;
	struct t_unitdata tud1, tud2;
	struct t_bind tbind;
	ipa_t	our_addr;
	TIMER	t1;

	EnterCritSec();
	/* Critical around itparse until we replace use of strtok */
	if ( !itparse(cmd, &rqst, &ip_addr, &cnt, &len, &rep) ) {
		ExitCritSec();
		free(cmd);
		warn("udpclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	free(cmd);
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		warn("udpclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = t_open(UDP_DEV_NAME, 2, nilp(struct t_info))) == -1) {
		t_error("udpclient: open of udp device failed");
bye1:;
		free(buf);
		return;
	}
	memset((char *)&our_addr, 0, sizeof(our_addr));
	U16_TO_BE16(UDP_CLIENT_PORT, our_addr.ip_port);
	our_addr.ip_family = AF_INET;
	tbind.addr.buf = (char *)&our_addr;
	tbind.addr.len = sizeof(our_addr);
	tbind.addr.maxlen = sizeof(our_addr);
	tbind.qlen = 0;
	if (t_bind(fd, &tbind, &tbind) == -1) {
		lt_error(fd, "udpclient: bind failed");
bye2:;
		t_close(fd);
		goto bye1;
	}
	tud1.addr.buf = (char *)&ip_addr;
	tud1.addr.len = sizeof(ip_addr);
	tud1.opt.len = 0;
	tud1.udata.buf = buf;
	switch ( rqst ) {
	case CLIENT_DISC:
		U16_TO_BE16(UDP_DISCARD_PORT, ip_addr.ip_port);
		ip_addr.ip_family = AF_INET;
		tud1.udata.len = len;
		printf("udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &ip_addr));
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( t_sndudata(fd, &tud1) == -1 ) {
				lt_error(fd, "udpclient: t_sndudata failed");
				goto bye2;
			}
		}
		break;
	case CLIENT_ECHO:
		U16_TO_BE16(UDP_ECHO_PORT, ip_addr.ip_port);
		ip_addr.ip_family = AF_INET;
		tud1.udata.len = len;
		tud2.addr.buf = (char *)&our_addr;
		tud2.addr.maxlen = sizeof(our_addr);
		tud2.opt.maxlen = sizeof(opt_buf);
		tud2.opt.buf = opt_buf;
		tud2.udata.maxlen = len;
		tud2.udata.buf = buf;
		printf("udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &ip_addr));
		i1 = cnt;
		SET_TIME(&t1);
		while ( i1-- ) {
			if ( t_sndudata(fd, &tud1) == -1 ) {
				lt_error(fd, "udp_client: t_sndudata failed");
				goto bye2;
			}
			if ( t_rcvudata(fd, &tud2, &flags) == -1 ) {
				lt_error(fd, "udp_client: t_rcvudata failed");
				goto bye2;
			}
			if ( tud2.udata.len != tud2.udata.maxlen ) {
				printf("udp_client: only got back %d bytes\n", tud2.udata.len);
				goto bye2;
			}
		}
		break;
	case CLIENT_IMSNDU:
		tud1.addr.buf = (char *)&our_addr;
		tud1.addr.len = tbind.addr.len;
		tud1.addr.maxlen = sizeof(our_addr);
		tud1.opt.len = 0;
		tud1.opt.maxlen = sizeof(opt_buf);
		tud1.opt.buf = opt_buf;
		tud1.udata.len = len;
		tud1.udata.maxlen = len;
		printf("udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &our_addr));
		i1 = cnt;
		SET_TIME(&t1);
		while ( i1-- ) {
			if ( t_sndudata(fd, &tud1) == -1 ) {
				lt_error(fd, "udp_client: t_sndudata failed");
				goto bye2;
			}
			if ( t_rcvudata(fd, &tud1, &flags) == -1 ) {
				lt_error(fd, "udp_client: t_rcvudata failed");
				goto bye2;
			}
			if ( tud1.udata.len != tud1.udata.maxlen ) {
				printf("udp_client: only got back %d bytes\n", tud1.udata.len);
				goto bye2;
			}
		}
		break;
	case CLIENT_SNDU:
		U16_TO_BE16(UDPTST_PORT, ip_addr.ip_port);
		ip_addr.ip_family = AF_INET;
		tud1.udata.len = 1;
		/* Send the first datagram to get the server's attention */
		if ( t_sndudata(fd, &tud1) == -1 ) {
			lt_error(fd, "udpclient: t_sndudata failed");
			goto bye2;
		}
		tud1.addr.buf = (char *)&ip_addr;
		tud1.addr.maxlen = sizeof(ip_addr);
		tud1.opt.buf = opt_buf;
		tud1.opt.maxlen = sizeof(opt_buf);
		tud1.udata.buf = buf;
		tud1.udata.maxlen = 1;
		/* Get back the ACK with the new target address */
		if ( (i1 = t_rcvudata(fd, &tud1, &flags)) == -1 ) {
			lt_error(fd, "udpclient: t_rcvudata failed");
			goto bye2;
		}
		printf("udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &ip_addr));
		tud1.addr.buf = (char *)&ip_addr;
		tud1.addr.len = sizeof(ip_addr);
		tud1.opt.len = 0;
		tud1.udata.buf = buf;
		tud1.udata.len = len;
		buf[0] = XDGTST_START;
		if ( cnt < 2 )
			cnt = 2;
		cnt += 5;
		i1 = cnt;
		SET_TIME(&t1);
		while ( i1-- ) {
			if ( i1 == 5 )
				buf[0] = XDGTST_STOP;
			if ( t_sndudata(fd, &tud1) == -1 ) {
				lt_error(fd, "udp_client: t_sndudata failed");
				goto bye2;
			}
		}
		break;
	}
	etime(cnt, len, &t1, buf);
	printf("udpclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
	t_close(fd);
	free(buf);
}

staticf	void
udpsrvc (arg)
	u8	* arg;
{
	char	addr_buf[256];
	char	* buf;
	int	cnt = 0;
	int	fd;
	int	flags;
	ipa_t	* its_addr;
	char	opt_buf[256];
	int	state = XDGTST_IDLE;
	struct t_unitdata tud;
	TIMER	t1;

	fd = *(int *)arg;
	its_addr = (ipa_t *)(arg + sizeof(int));
	if ( (buf = malloc(UNIT_DATA_TEST_MAX_SIZE)) == nilp(char) ) {
		t_close(fd);
		warn("udpsrvc: out of memory\n");
		free((char *)arg);
		ExitThread(0, 1);
	}
#ifdef	VMS
	map_region(fd, buf, UNIT_DATA_TEST_MAX_SIZE);
#endif
	/* Let the client know we are ready */
	tud.addr.buf = (char *)its_addr;
	tud.addr.len = sizeof(ipa_t);
	tud.opt.len = 0;
	tud.udata.buf = buf;
	tud.udata.len = 1;
	if ( t_sndudata(fd, &tud) == -1 ) {
		lt_error(fd, "udpsrvc: t_sndudata failed");
		t_close(fd);
		free((char *)arg);
		ExitThread(0, 1);
	}
	free((char *)arg);
	tud.addr.buf = addr_buf;
	tud.addr.maxlen = sizeof(addr_buf);
	tud.opt.buf = opt_buf;
	tud.opt.maxlen = sizeof(opt_buf);
	tud.udata.buf = buf;
	tud.udata.maxlen = UNIT_DATA_TEST_MAX_SIZE;
	for (;;) {
		if ( t_rcvudata(fd, &tud, &flags) == -1 ) {
			lt_error(fd, "udpsrvc: t_rcvudata failed");
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
			printf("udpsrvc: (%d) bogus data code 0x%x while active\n", fd, buf[0] & 0xFF);
			goto bye;
		case XDGTST_IDLE:
			switch (buf[0]) {
			case XDGTST_START:
				state = XDGTST_START;
				SET_TIME(&t1);
				continue;
			default:
				printf("udpsrvc: (%d) bogus data code 0x%x while idle\n", fd, buf[0] & 0xFF);
				goto bye;
			}
			/*NOTREACHED*/
		}
		printf("udpsrvc(%d): Cnt: %d, len %d, %s\n", fd, cnt, tud.udata.len, buf);
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
udpsrvr () {
	char	* arg_buf;
	char	buf[128];
	char	ch;
	int	err;
	int	fd;
	int	flags;
	int	nfd;
	char	opt_buf[256];
	struct t_bind tbind;
	struct t_unitdata tud;
	ipa_t	our_ip_addr;
	ipa_t	its_ip_addr;

	/* Get the datagram server stream */
	if ( (fd = t_open(UDP_DEV_NAME, 2, nilp(struct t_info))) == -1 ) {
		lt_error(fd, "udpsrvr: t_open of udp device failed");
		return;
	}
	memset(&our_ip_addr, 0, sizeof(our_ip_addr));
	memset(&its_ip_addr, 0, sizeof(its_ip_addr));
	U16_TO_BE16(UDPTST_PORT, our_ip_addr.ip_port);
	our_ip_addr.ip_family = AF_INET;
	tbind.addr.len = sizeof(our_ip_addr);
	tbind.addr.maxlen = sizeof(our_ip_addr);
	tbind.addr.buf = (char *)&our_ip_addr;
	tbind.qlen = 0;
	if ( t_bind(fd, &tbind, &tbind) == -1 ) {
		lt_error(fd, "udpsrvr: t_bind failed");
		t_close(fd);
		return;
	}
	if ( UDPTST_PORT != BE16_TO_U16(our_ip_addr.ip_port) ) {
		warn("udpsrvr: one already running, bye\n");
		ExitThread(0, 1);
	}
	printf("udpsrvr: bound to IP address: %s\n", ipa_to_str(buf, &our_ip_addr));
	for ( ;; ) {
		tud.addr.buf = (char *)&its_ip_addr;
		tud.addr.maxlen = sizeof(its_ip_addr);
		tud.opt.buf = opt_buf;
		tud.opt.maxlen = sizeof(opt_buf);
		tud.udata.buf = &ch;
		tud.udata.maxlen = 1;
		/* Wait for a client to appear */
		if ( (err = t_rcvudata(fd, &tud, &flags)) != 0 ) {
			lt_error(fd, "udpsrvr: t_rcvudata failed");
			break;
		}
		arg_buf = malloc(sizeof(ipa_t) + sizeof(int));
		if ( !arg_buf ) {
			warn("udpsrvr: out of memory\n");
			continue;
		}
		memcpy(arg_buf + sizeof(int), (char *)&its_ip_addr, sizeof(ipa_t));
		if ( single_thread ) {
			*(int *)arg_buf = fd;
			udpsrvc((u8 *)arg_buf);
		} else {
			/* Get a new stream to handle this service */
			if ( (nfd = t_open(UDP_DEV_NAME, 2, nilp(struct t_info))) == -1 ) {
				t_error("udpsrvr: t_open of udp device failed");
				continue;
			}
			tbind.addr.len = 0;
			tbind.addr.maxlen = sizeof(our_ip_addr);
			tbind.addr.buf = (char *)&our_ip_addr;
			tbind.qlen = 0;
			if ( t_bind(nfd, nilp(struct t_bind), &tbind) == -1 ) {
				lt_error(nfd, "udpsrvr: t_bind failed");
				t_close(nfd);
				continue;
			}
			*(int *)arg_buf = nfd;
			printf("udpsrvr: handling connection from %s on fd %d (%s)\n", ipa_to_str(buf, &its_ip_addr), nfd
			, ipa_to_str(&buf[64], &our_ip_addr));
			/* Start a new thread */
			if ( (err = CreateThread((pfi_t)udpsrvc, nilp(u32), nilp(char), (u32)STACK_SIZE, (u32)arg_buf)) != 0 ) {
				t_close(nfd);
				printf("udpsrvr: couldn't create service thread (error code %d)\n", err);
				free(arg_buf);
				continue;
			}
#ifdef	FORKER
			t_close(nfd);
#endif
		}
	}
	t_close(fd);
}

void
tcpclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	int	fd;
	int	flags;
	ipa_t	ip_addr;
	int	len;
	char	opt_buf[128];
	int	rep;
	int	rqst;
	struct t_call	tcall;
	TIMER	t1;

	EnterCritSec();
	/* Critical around itparse until we replace use of strtok */
	if ( !itparse(cmd, &rqst, &ip_addr, &cnt, &len, &rep) ) {
		ExitCritSec();
		free(cmd);
		warn("tcpclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		free(cmd);
		warn("tcpclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = t_open(TCP_DEV_NAME, 2, nilp(struct t_info))) == -1) {
		lt_error(fd, "tcpclient: open of tcp device failed");
		free(cmd);
		free(buf);
		return;
	}
#ifdef	VMS
	map_region(fd, buf, len);
#endif
	if (t_bind(fd, nilp(struct t_bind), nilp(struct t_bind)) == -1) {
		lt_error(fd, "tcpclient: bind failed");
		goto bye2;
	}
	U16_TO_BE16(TCPTST_PORT, ip_addr.ip_port);
	ip_addr.ip_family = AF_INET;
	tcall.addr.buf = (char *)&ip_addr;
	tcall.addr.len = sizeof(ip_addr);
	tcall.addr.maxlen = sizeof(ip_addr);
	tcall.opt.buf = opt_buf;
	tcall.opt.len = 0;
	tcall.opt.maxlen = sizeof(opt_buf);
	tcall.udata.buf = nilp(char);
	tcall.udata.len = 0;
	tcall.udata.maxlen = 0;
	/* Connect to the server. */
	if (t_connect(fd, &tcall, &tcall) == -1) {
		lt_error(fd, "tcpclient: t_connect failed");
		goto bye2;
	}
	printf("tcpclient: connected to ... on fd %d\n", fd);
	while (rep--) {
		/* send the server the command */
		if (t_snd(fd, cmd, strlen(cmd), 0) == -1) {
			lt_error(fd, "tcpclient: couldn't send command");
			goto bye2;
		}
		it_wait_for_ack(fd);
		SET_TIME(&t1);
		switch (rqst) {
		case CLIENT_SEND:
			do_send(fd, cnt, len, buf);
			it_wait_for_ack(fd);
			break;
		case CLIENT_RECV:
			do_recv(fd, cnt, len, buf);
			it_send_ack(fd);
			break;
#ifndef DOS
		case CLIENT_WRITE:
			do_write(fd, cnt, len, buf);
			it_wait_for_ack(fd);
			break;
		case CLIENT_READ:
			do_read(fd, cnt, len, buf);
			it_send_ack(fd);
			break;
#endif
		}
		etime(cnt, len, &t1, buf);
		printf("tcpclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
	}
	/* Perform an orderly release. */
	if (t_sndrel(fd) == -1) {
		lt_error(fd, "tcpclient: t_sndrel failed");
		goto bye2;
	}
	(void)t_rcvrel(fd);
	printf("tcpclient: (%d) disconnected\n", fd);
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
tcpsrvc (arg)
	u32	arg;
{
	char	* buf;
	char	cmd[128];
	int	cnt;
	char	* cp1;
	int	dir;
	int	fd;
	int	flags;
	ipa_t	ip_addr;
	int	len;
	char	msg[128];
	int	rep;
	TIMER	t1;

	fd = (int)arg;
	flags = 0;
	buf = nilp(char);
	printf("tcpsrvc: handling connection from ... on fd %d\n", fd);
	for ( ;; ) {
		/* Read the next command */
		for ( cp1 = cmd; cp1 < A_END(cmd); cp1++ ) {
			if ( t_rcv(fd, cp1, 1, &flags) == -1 ) {
				if ( t_errno == TLOOK  &&  t_look(fd) == T_ORDREL ) {
					/* Perform an orderly release. */
					if (t_rcvrel(fd) == -1 ) {
						lt_error(fd, "tcpsrvc: rcvrel failed");
						printf("tcpsrvc: current state: 0x%x\n", t_getstate(fd));
					} else if ( t_sndrel(fd) == -1 ) {
						lt_error(fd, "tcpsrvc: sndrel failed");
						printf("tcpsrvc: current state: 0x%x\n", t_getstate(fd));
					}
				} else
					lt_error(fd, "tcpsrvc: error receiving command");
				printf("tcpsrvc: (%d) disconnected\n", fd);
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
			printf("tcpsrvc: (%d) bogus command received\n", fd);
			goto bye;
		}
		*++cp1 = '\0';
		EnterCritSec();
		/* Critical around itparse until we replace use of strtok */
		if ( !itparse(cmd, &dir, &ip_addr, &cnt, &len, &rep) ) {
			ExitCritSec();
			warn("tcpsrvc: couldn't parse command '%s'\n");
			goto bye;
		}
		ExitCritSec();
		printf("tcpsrvc: (%d) command: %s", fd, cmd);
		if ( (buf = malloc(len)) == nilp(char) ) {
			t_close(fd);
			warn("tcpsrvc: (%d) out of memory\n", fd);
			ExitThread(0, 1);
		}
#ifdef	VMS
		map_region(fd, buf, len);
#endif
		flags = 0;
		it_send_ack(fd);
		SET_TIME(&t1);
		switch (dir) {
		case SERVER_SEND:
			do_send(fd, cnt, len, buf);
			it_wait_for_ack(fd);
			break;
		case SERVER_RECV:
			do_recv(fd, cnt, len, buf);
			it_send_ack(fd);
			break;
#ifndef DOS
		case SERVER_READ:
			do_read(fd, cnt, len, buf);
			it_send_ack(fd);
			break;
		case SERVER_WRITE:
			do_write(fd, cnt, len, buf);
			it_wait_for_ack(fd);
			break;
#endif
		}
		etime(cnt, len, &t1, buf);
		printf("tcpsrvc(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
#ifdef	VMS
		unmap_region(fd);
#endif
		free(buf);
		buf = nilp(char);
	}
}

void
tcpsrvr () {
	char	buf[128];
	int	err;
	int	fd;
	ipa_t	ip_addr;
	int	nfd;
	struct t_bind tbind;
	struct t_call tcall;
	u32	tid;

	if ( (fd = t_open(TCP_DEV_NAME, 2, nilp(struct t_info))) == -1 ) {
		lt_error(fd, "tcpsrvr: t_open of tcp device failed");
		return;
	}
	memset((char *)&ip_addr, 0, sizeof(ip_addr));
	U16_TO_BE16(TCPTST_PORT, ip_addr.ip_port);
	ip_addr.ip_family = AF_INET;
	tbind.addr.len = sizeof(ip_addr);
	tbind.addr.maxlen = sizeof(ip_addr);
	tbind.addr.buf = (char *)&ip_addr;
	tbind.qlen = 5;
	if ( t_bind(fd, &tbind, &tbind) == -1 ) {
		lt_error(fd, "tcpsrvr: t_bind failed");
		t_close(fd);
		return;
	}
	if ( TCPTST_PORT != BE16_TO_U16(ip_addr.ip_port) ) {
		warn("tcpsrvr: one already running, bye\n");
		ExitThread(0, 1);
	}
	printf("tcpsrvr: bound to IP address: %s\n", ipa_to_str(buf, &ip_addr));
	/* Set the tcall structure for the t_listen(). */
	tcall.addr.buf = (char *)&ip_addr;
	tcall.addr.maxlen = sizeof(ip_addr);
	tcall.opt.buf = (char *)buf;
	tcall.opt.maxlen = sizeof(buf);
	tcall.udata.buf = (char *)buf;
	tcall.udata.maxlen = sizeof(buf);
	while (t_listen(fd, &tcall) != -1) {
		/* Open a new stream to accept this connection. */
		if ((nfd = t_open(TCP_DEV_NAME, 2, nilp(struct t_info))) == -1) {
			lt_error(nfd, "tcpsrvr: t_open failed");
			t_close(fd);
			return;
		}
		if (t_bind(nfd, nilp(struct t_bind), nilp(struct t_bind)) == -1){
			lt_error(nfd, "tcpsrvr: t_bind failed");
bye1:;
			t_close(fd);
			t_close(nfd);
			return;
		}
		/* Accept the connection on the new stream. */
		if (t_accept(fd, nfd, &tcall) == -1) {
			lt_error(fd, "tcpsrvr: t_accept failed");
			goto bye1;
		}
		if ( single_thread )
			tcpsrvc((u32)nfd);
		else if ( (err = CreateThread((pfi_t)tcpsrvc, &tid, nilp(char), (u32)STACK_SIZE, (u32)nfd)) != 0 ) {
			warn("tcpsrvr: couldn't create service thread (error code %d)\n", err);
			t_close(nfd);
		}
#ifdef	FORKER
		else
			t_close(nfd);
#endif
	}
	lt_error(fd, "tcpsrvr: t_listen failed");
}

staticf	boolean
itparse (cmd, dir, ip_addr, cnt, len, rep)
	char	* cmd;
	int	* dir;
	ipa_t	* ip_addr;
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
	if ( strncmp(cp1, "tcpsend", 4) == 0  ||  strncmp(cp1, "stcpsend", 5) == 0 )
		*dir = CLIENT_SEND;
	else if ( strncmp(cp1, "tcprecv", 6) == 0  ||  strncmp(cp1, "stcprecv", 7) == 0 )
		*dir = CLIENT_RECV;
	else if ( strncmp(cp1, "udpsnd", 4) == 0  ||  strncmp(cp1, "sudpsnd", 5) == 0 )
		*dir = CLIENT_SNDU;
	else if ( strncmp(cp1, "tcpread", 6) == 0  ||  strncmp(cp1, "stcpread", 7) == 0 )
		*dir = CLIENT_READ;
	else if ( strncmp(cp1, "tcpwrite", 4) == 0  ||  strncmp(cp1, "stcpwrite", 5) == 0 )
		*dir = CLIENT_WRITE;
	else if ( strncmp(cp1, "udpdis", 4) == 0  ||  strncmp(cp1, "sudpdis", 5) == 0 )
		*dir = CLIENT_DISC;
	else if ( strncmp(cp1, "udpe", 4) == 0  ||  strncmp(cp1, "sudpe", 5) == 0 )
		*dir = CLIENT_ECHO;
	else if ( strncmp(cp1, "udpim", 4) == 0  ||  strncmp(cp1, "sudpim", 5) == 0 )
		*dir = CLIENT_IMSNDU;
	else
		return false;
	if ( !(cp1 = strtok(nilp(char), sep)) )
		return false;
	str_to_ipa(cp1, ip_addr);
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
it_send_ack (fd)
	int	fd;
{
	if ( t_snd(fd, "a", 1, 0) > 0 )
		return;
	lt_error(fd, "it_send_ack: t_snd failed");
	t_close(fd);
	ExitThread(0, 1);
}

staticf	void
it_wait_for_ack (fd)
	int	fd;
{
	char	ch;
	int	flags;

	flags = 0;	
	if ( t_rcv(fd, &ch, 1, &flags) <= 0 ) {
		lt_error(fd, "it_wait_for_ack: t_rcv failed");
		t_close(fd);
		ExitThread(0, 1);
	}
	if ( ch == 'a' )
		return;
	printf("it_wait_for_ack: (%d) bad ACK character 0x%x\n", fd, ch & 0xFF);
	t_close(fd);
	ExitThread(0, 1);
}
