static char sccsid[] = "@(#)34	1.2  src/bos/usr/sbin/pse/utils/mtest/ipsock.c, cmdpse, bos411, 9428A410j 5/22/91 11:40:47";
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
 ** ipsock.c 2.1
 **/


/* Socket style equivalent of the IP TLI test. */

#include <pse/common.h>
#include "mtest.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#define	SO_FIRST_BOGO_PORT	0x2929

typedef struct sockaddr_in	IPA, * IPAP;

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
staticf	char	* ipa_to_str(   char * buf, IPAP ip_addr   );
staticf	boolean	itparse(   char * cmd, int * dir, IPAP ip_addr, int * cnt, int * len, int * rep   );
staticf	void	it_send_ack(   int fd   );
staticf	void	it_wait_for_ack(   int fd   );
staticf	void	lt_error(   char * msg   );
extern	char	* malloc(   int size   );
extern	char	* strchr(   char * str, char ch   );
staticf	boolean	strtohex(   char * to, char * from, int len   );
staticf	void	str_to_ipa(   char * str, IPAP ip_addr   );
extern	char	* strtok(   char * str, char * sep   );
staticf	int	tohex(   int ch_param   );
	int	so_tcpclient(   char * cmd   );
staticf	void	so_tcpsrvc(   u32 arg   );
	int	so_tcpsrvr(   void   );
	int	so_udpclient(   char * cmd   );
staticf	void	so_udpsrvc(   u8 * arg   );
	int	so_udpsrvr(   void   );
extern	void	warn(   char * fmt, ...   );

extern	int	errno;
extern	boolean	single_thread;

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

	while ( cnt-- ) {
		i1 = len;
		for ( cp1 = buf; i1; cp1 += nrd, i1 -= nrd) {
			if ( (nrd = read(fd, cp1, i1)) == -1 ) {
				printf("do_read: read failed, %s\n", errmsg(0));
				close(fd);
				free(buf);
				ExitThread(0, 1);
			}
		}
	}
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
			if ( (nrd = recv(fd, cp1, i1, 0)) <= 0 ) {
				warn("do_recv: recv failed, %s\n", errmsg(0));
				close(fd);
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
			if ( (nsnt = send(fd, cp1, i1, 0)) == -1 ) {
				warn("do_send: send failed, %s\n", errmsg(0));
				close(fd);
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
			if ( (nsnt = write(fd, cp1, i1)) == -1 ) {
				printf("do_write: write failed, %s\n", errmsg(0));
				close(fd);
				free(buf);
				ExitThread(0, 1);
			}
		}
	}
}
#endif

staticf	void
lt_error (msg)
	char	* msg;
{
	printf("%s, %s\n", msg, errmsg(0));
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
	IPAP	ip_addr;
{
auto	char	* cp;
	int	i1;
reg	u8	* u = (u8 *)&ip_addr->sin_addr.s_addr;
	
	memset((char *)ip_addr, 0, sizeof(IPA));
	cp = str;
	for ( i1 = 4; *cp  &&  i1--; ) {
		*u++ = strtol(cp, &cp, 10);
		if (*cp)
			cp++;
	}
	if ( *cp == ',' ) {
		i1 = strtol(cp+1, nilp(char), 10);
		U16_TO_BE16((u16)i1, &ip_addr->sin_port);
	}
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
ipa_to_str (buf, ip_addr)
	char	* buf;
	IPAP	ip_addr;
{
reg	u8	* addr = (u8 *)&ip_addr->sin_addr.s_addr;
reg	u8	* port = (u8 *)&ip_addr->sin_port;

	sprintf(buf, "%d.%d.%d.%d,%d", addr[0], addr[1], addr[2], addr[3], BE16_TO_U16(port));
	return buf;
}

int
so_udpclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	int	fd;
	int	flags;
	int	i1, i2;
	IPA	ip_addr;
	IPA	its_addr;
	int	len;
	int	rep, rqst;
	TIMER	t1;

	memset(&ip_addr, 0, sizeof(ip_addr));
	memset(&its_addr, 0, sizeof(ip_addr));
	EnterCritSec();
	/* Critical around itparse until we replace use of strtok */
	if ( !itparse(cmd, &rqst, &ip_addr, &cnt, &len, &rep) ) {
		ExitCritSec();
		free(cmd);
		warn("so_udpclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	free(cmd);
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		warn("so_udpclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		lt_error("so_udpclient: open of datagram socket failed");
bye1:;
		free(buf);
		ExitThread(0, 1);
	}
	switch ( rqst ) {
	case CLIENT_DISC:
		U16_TO_BE16(UDP_DISCARD_PORT, &ip_addr.sin_port);
		ip_addr.sin_family = AF_INET;
		printf("so_udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &ip_addr));
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( sendto(fd, buf, len, 0, &ip_addr, sizeof(ip_addr)) == -1 ) {
				lt_error("so_udpclient: sendto failed");
bye2:;
				close(fd);
				goto bye1;
			}
		}
		break;
	case CLIENT_ECHO:
		U16_TO_BE16(UDP_ECHO_PORT, &ip_addr.sin_port);
		ip_addr.sin_family = AF_INET;
		printf("so_udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &ip_addr));
		i1 = cnt;
		SET_TIME(&t1);
		while ( i1-- ) {
			if ( sendto(fd, buf, len, 0, &ip_addr, sizeof(ip_addr)) == -1 ) {
				lt_error("so_udpclient: sendto failed");
				goto bye2;
			}
			i2 = sizeof(its_addr);
			if ( recvfrom(fd, buf, len, 0, &its_addr, &i2) == -1 ) {
				lt_error("so_udpclient: recvfrom failed");
				goto bye2;
			}
		}
		break;
	case CLIENT_IMSNDU:
		for ( i1 = SO_FIRST_BOGO_PORT;; i1++) {
			U16_TO_BE16(i1, (u8 *)&its_addr.sin_port);
			its_addr.sin_family = AF_INET;
			if ( bind(fd, &its_addr, sizeof(its_addr)) == 0 )
				break;
			if ( errno == EADDRINUSE )
				continue;
			lt_error("so_udpclient: bind failed");
			goto bye2;
		}
		printf("so_udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &its_addr));
		i1 = cnt;
		SET_TIME(&t1);
		while ( i1-- ) {
			if ( sendto(fd, buf, len, 0, &its_addr, sizeof(its_addr)) == -1 ) {
				lt_error("so_udpclient: sendto failed");
				goto bye2;
			}
			i2 = sizeof(its_addr);
			if ( recvfrom(fd, buf, len, 0, &its_addr, &i2) == -1 ) {
				lt_error("so_udpclient: recvfrom failed");
				goto bye2;
			}
		}
		break;
	case CLIENT_SNDU:
		U16_TO_BE16(UDPTST_PORT, (u8 *)&ip_addr.sin_port);
		ip_addr.sin_family = AF_INET;
		/* Send the first datagram to get the server's attention */
		if ( sendto(fd, buf, 1, 0, &ip_addr, sizeof(ip_addr)) == -1 ) {
			lt_error("so_udpclient: sendto failed");
			goto bye2;
		}
		/* Get back the ACK with the new target address */
		i2 = sizeof(ip_addr);
		if ( (i1 = recvfrom(fd, buf, 1, 0, &ip_addr, &i2)) == -1 ) {
			lt_error("so_udpclient: recvfrom failed");
			goto bye2;
		}
		printf("so_udpclient: (%d) sending to '%s'\n", fd, ipa_to_str(buf, &ip_addr));
		buf[0] = XDGTST_START;
		if ( cnt < 2 )
			cnt = 2;
		cnt += 5;
		i1 = cnt;
		SET_TIME(&t1);
		while ( i1-- ) {
			if ( i1 == 5 )
				buf[0] = XDGTST_STOP;
			if ( sendto(fd, buf, len, 0, &ip_addr, sizeof(ip_addr)) == -1 ) {
				lt_error("so_udpclient: sendto failed");
				goto bye2;
			}
		}
		break;
	}
	etime(cnt, len, &t1, buf);
	printf("so_udpclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
	close(fd);
	free(buf);
	ExitThread(0, 1);
}

staticf	void
so_udpsrvc (arg)
	u8	* arg;
{
	char	* buf;
	int	cnt = 0;
	int	fd;
	IPAP	its_addr;
	int	its_addr_len;
	int	i1;
	int	len;
	int	state = XDGTST_IDLE;
	TIMER	t1;

	fd = *(int *)arg;
	its_addr = (IPAP)(arg + sizeof(int));
	if ( (buf = malloc(UNIT_DATA_TEST_MAX_SIZE)) == nilp(char) ) {
		close(fd);
		warn("so_udpsrvc: out of memory\n");
		free((char *)arg);
		ExitThread(0, 1);
	}
	/* Let the client know we are ready */
	if ( sendto(fd, buf, 1, 0, its_addr, sizeof(IPA)) == -1 ) {
		lt_error("so_udpsrvc: sendto failed");
		close(fd);
		free((char *)arg);
		ExitThread(0, 1);
	}
	free((char *)arg);
	for (;;) {
		its_addr_len = sizeof(IPA);
		if ( (len = recvfrom(fd, buf, UNIT_DATA_TEST_MAX_SIZE, 0, its_addr, &its_addr_len)) == -1 ) {
			lt_error("so_udpsrvc: recvfrom failed");
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
				etime(cnt, len, &t1, buf);
				break;
			}
			printf("so_udpsrvc: (%d) bogus data code 0x%x while active\n", fd, buf[0] & 0xFF);
			goto bye;
		case XDGTST_IDLE:
			switch (buf[0]) {
			case XDGTST_START:
				state = XDGTST_START;
				SET_TIME(&t1);
				continue;
			default:
				printf("so_udpsrvc: (%d) bogus data code 0x%x while idle\n", fd, buf[0] & 0xFF);
				goto bye;
			}
			/*NOTREACHED*/
		}
		printf("so_udpsrvc(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
		break;
	}
bye:;
	close(fd);
	free(buf);
}

int
so_udpsrvr () {
	char	* arg_buf;
	char	buf[128];
	char	ch;
	int	err;
	int	fd;
	int	flags;
	int	i1;
	int	nfd;
	IPA	our_ip_addr;
	IPA	its_ip_addr;

	/* Get the datagram server stream */
	if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
		lt_error("so_udpsrvr: couldn't get datagram socket");
		ExitThread(0, 1);
	}
	memset(&our_ip_addr, 0, sizeof(our_ip_addr));
	memset(&its_ip_addr, 0, sizeof(its_ip_addr));
	U16_TO_BE16(UDPTST_PORT, (u8 *)&our_ip_addr.sin_port);
	our_ip_addr.sin_family = AF_INET;
	if ( bind(fd, &our_ip_addr, sizeof(our_ip_addr)) == -1 ) {
		lt_error("so_udpsrvr: bind failed");
		close(fd);
		ExitThread(0, 1);
	}
	printf("so_udpsrvr: bound to IP address: %s\n", ipa_to_str(buf, &our_ip_addr));
	for ( ;; ) {
		/* Wait for a client to appear */
		i1 = sizeof(its_ip_addr);
		if ( (err = recvfrom(fd, &ch, 1, 0, &its_ip_addr, &i1)) == -1 ) {
			lt_error("so_udpsrvr: recvfrom failed");
			break;
		}
		arg_buf = malloc(sizeof(IPA) + sizeof(int));
		if ( !arg_buf ) {
			warn("so_udpsrvr: out of memory\n");
			continue;
		}
		memcpy(arg_buf + sizeof(int), (char *)&its_ip_addr, sizeof(IPA));
		if ( single_thread ) {
			*(int *)arg_buf = fd;
			so_udpsrvc((u8 *)arg_buf);
		} else {
			/* Get a new socket to handle this service */
			if ( (nfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
				lt_error("so_udpsrvr: couldn't get service socket");
				continue;
			}
			*(int *)arg_buf = nfd;
			printf("so_udpsrvr: handling connection from %s on fd %d\n", ipa_to_str(buf, &its_ip_addr), nfd);
			/* Start a new thread */
			if ( (err = CreateThread((pfi_t)so_udpsrvc, nilp(u32), nilp(char), (u32)STACK_SIZE, (u32)arg_buf)) != 0 ) {
				close(nfd);
				printf("so_udpsrvr: couldn't create service thread (error code %d)\n", err);
				free(arg_buf);
				continue;
			}
		}
#ifdef	FORKER
			close(nfd);
#endif
	}
	close(fd);
	return 0;
}

int
so_tcpclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	int	fd;
	IPA	ip_addr;
	int	len;
	int	rep;
	int	rqst;
	TIMER	t1;

	memset(&ip_addr, 0, sizeof(ip_addr));
	EnterCritSec();
	/* Critical around itparse until we replace use of strtok */
	if ( !itparse(cmd, &rqst, &ip_addr, &cnt, &len, &rep) ) {
		ExitCritSec();
		free(cmd);
		warn("so_tcpclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		free(cmd);
		warn("so_tcpclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		lt_error("so_tcpclient: couldn't get stream socket");
		free(cmd);
		free(buf);
		ExitThread(0, 1);
	}
	U16_TO_BE16(TCPTST_PORT, (u8 *)&ip_addr.sin_port);
	ip_addr.sin_family = AF_INET;
	/* Connect to the server. */
	if ( connect(fd, &ip_addr, sizeof(ip_addr)) == -1 ) {
		lt_error("so_tcpclient: connect failed");
		goto bye2;
	}
	printf("so_tcpclient: connected to ... on fd %d\n", fd);
	while (rep--) {
		/* send the server the command */
		if (send(fd, cmd, strlen(cmd), 0) == -1) {
			lt_error("so_tcpclient: couldn't send command");
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
		printf("so_tcpclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
	}
	printf("so_tcpclient: (%d) disconnected\n", fd);
bye2:;
bye1:;
	free(cmd);
	free(buf);
	/* Close the transport stream. */
	close(fd);
	return 0;
}

staticf	void
so_tcpsrvc (arg)
	u32	arg;
{
	char	* buf;
	char	cmd[128];
	int	cnt;
	char	* cp1;
	int	dir;
	int	fd;
	IPA	ip_addr;
	int	len;
	char	msg[128];
	int	rep;
	TIMER	t1;

	fd = (int)arg;
	buf = nilp(char);
	printf("so_tcpsrvc: handling connection from ... on fd %d\n", fd);
	for ( ;; ) {
		/* Read the next command */
		for ( cp1 = cmd; cp1 < A_END(cmd); cp1++ ) {
			switch ( recv(fd, cp1, 1, 0) ) {
			case 0:
				printf("so_tcpsrvc: (%d) disconnected\n", fd);
				goto bye;
			case -1:
				lt_error("so_tcpsrvc: recv failed reading next command");
bye:;
				if ( buf )
					free(buf);
				close(fd);
				ExitThread(0, 1);
			default:
				break;
			}
			if ( *cp1 == '\n' )
				break;
		}
		if ( *cp1 != '\n' ) {
			printf("so_tcpsrvc: (%d) bogus command received\n", fd);
			goto bye;
		}
		*++cp1 = '\0';
		EnterCritSec();
		/* Critical around itparse until we replace use of strtok */
		if ( !itparse(cmd, &dir, &ip_addr, &cnt, &len, &rep) ) {
			ExitCritSec();
			warn("so_tcpsrvc: couldn't parse command '%s'\n", cmd);
			goto bye;
		}
		ExitCritSec();
		printf("so_tcpsrvc: (%d) command: %s", fd, cmd);
		if ( (buf = malloc(len)) == nilp(char) ) {
			close(fd);
			warn("so_tcpsrvc: (%d) out of memory\n", fd);
			ExitThread(0, 1);
		}
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
		printf("so_tcpsrvc(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
		free(buf);
		buf = nilp(char);
	}
}

int
so_tcpsrvr () {
	char	buf[128];
	int	err;
	int	fd;
	IPA	ip_addr;
	int	ip_addr_len;
	int	nfd;
	u32	tid;

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		lt_error("so_tcpsrvr: couldn't open stream socket");
		ExitThread(0, 1);
	}
	memset(&ip_addr, 0, sizeof(ip_addr));
	ip_addr.sin_family = AF_INET;
	U16_TO_BE16(TCPTST_PORT, (u8 *)&ip_addr.sin_port);
	if ( bind(fd, &ip_addr, sizeof(ip_addr)) == -1 ) {
		lt_error("so_tcpsrvr: bind failed");
		close(fd);
		ExitThread(0, 1);
	}
	printf("so_tcpsrvr: bound to IP address: %s\n", ipa_to_str(buf, &ip_addr));
	if ( listen(fd, 5) == -1 ) {
		lt_error("so_tcpsrvr: listen failed");
		close(fd);
		ExitThread(0, 1);
	}
	for (;;) {
		/* Accept the connection on the new socket. */
		ip_addr_len = sizeof(IPA);
		if ( (nfd = accept(fd, &ip_addr, &ip_addr_len)) == -1) {
			lt_error("so_tcpsrvr: accept failed");
			close(fd);
			close(nfd);
			ExitThread(0, 1);
		}
		if ( single_thread )
			so_tcpsrvc((u32)nfd);
		else if ( (err = CreateThread((pfi_t)so_tcpsrvc, &tid, nilp(char), (u32)STACK_SIZE, (u32)nfd)) != 0 ) {
			warn("so_tcpsrvr: couldn't create service thread (error code %d)\n", err);
			close(nfd);
		}
#ifdef	FORKER
		else
			close(nfd);
#endif
	}
}

staticf	boolean
itparse (cmd, dir, ip_addr, cnt, len, rep)
	char	* cmd;
	int	* dir;
	IPAP	ip_addr;
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
	if ( send(fd, "a", 1, 0) > 0 )
		return;
	lt_error("it_send_ack: send failed");
	close(fd);
	ExitThread(0, 1);
}

staticf	void
it_wait_for_ack (fd)
	int	fd;
{
	char	ch;

	if ( recv(fd, &ch, 1, 0) <= 0 ) {
		lt_error("it_wait_for_ack: recv failed");
		close(fd);
		ExitThread(0, 1);
	}
	if ( ch == 'a' )
		return;
	printf("it_wait_for_ack: (%d) bad ACK character 0x%x\n", fd, ch & 0xFF);
	close(fd);
	ExitThread(0, 1);
}
