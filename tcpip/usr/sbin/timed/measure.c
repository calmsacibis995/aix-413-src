static char sccsid[] = "@(#)54	1.5.1.2  src/tcpip/usr/sbin/timed/measure.c, tcptimer, tcpip411, GOLD410 6/15/94 13:57:51";
/* 
 * COMPONENT_NAME: TCPIP measure.c
 * 
 * FUNCTIONS: in_cksum, measure 
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
 * Copyright (c) 1983 Regents of the University of California.
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
static char sccsid[] = "measure.c	2.6 (Berkeley) 6/18/88";
#endif  not lint */

#include <nl_types.h>
#include "timed_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s)  catgets(catd, MS_TIMED, n, s)

#include "globals.h"
#include <protocols/timed.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define BIASP	 	43199999
#define BIASN		-43200000
#define MODULO	 	86400000
#define PROCESSING_TIME	5 	/* ms. to reduce error in measurement */

#define PACKET_IN	1024

extern int id;
int measure_delta;
extern int sock_raw;
static n_short seqno = 0;

/*
 * Measures the differences between machines' clocks using
 * ICMP timestamp messages.
 */

measure(wait, addr)
struct timeval *wait;
struct sockaddr_in *addr;
{
	int length;
	int status;
	int msgcount, trials;
	int cc, count;
	fd_set ready;
	long sendtime, recvtime, histime;
	long min1, min2, diff;
	register long delta1, delta2;
	struct timeval tv1, tout;
	u_char packet[PACKET_IN], opacket[64];
	register struct icmp *icp = (struct icmp *) packet;
	register struct icmp *oicp = (struct icmp *) opacket;
	struct ip *ip = (struct ip *) packet;

	min1 = min2 = 0x7fffffff;
	status = HOSTDOWN;
	measure_delta = HOSTDOWN;

/* empties the icmp input queue */
	FD_ZERO(&ready);
empty:
	tout.tv_sec = tout.tv_usec = 0;
	FD_SET(sock_raw, &ready);
	/* select only on the file descriptors that we are interested in
	 * since selecting on FD_SETSIZE causes performance problems
	 */
	if (select(sock+1, &ready, (fd_set *)0, (fd_set *)0, &tout)) {
		length = sizeof(struct sockaddr_in);
		cc = recvfrom(sock_raw, (char *)packet, PACKET_IN, 0, 
		    (struct sockaddr_in *)NULL, &length);
		if (cc < 0)
			return(-1);
		goto empty;
	}

	/*
	 * To measure the difference, select MSGS messages whose round-trip
	 * time is smaller than RANGE if ckrange is 1, otherwise simply
	 * select MSGS messages regardless of round-trip transmission time.
	 * Choose the smallest transmission time in each of the two directions.
	 * Use these two latter quantities to compute the delta between
	 * the two clocks.
	 */

	length = sizeof(struct sockaddr_in);
	oicp->icmp_type = ICMP_TSTAMP;
	oicp->icmp_code = 0;
	oicp->icmp_cksum = 0;
	oicp->icmp_id = id;
	oicp->icmp_rtime = 0;
	oicp->icmp_ttime = 0;
	FD_ZERO(&ready);
	msgcount = 0;
	for (trials = 0; msgcount < MSGS && trials < TRIALS; ++trials) {
		oicp->icmp_seq = ++seqno;
		oicp->icmp_cksum = 0;

		tout.tv_sec = wait->tv_sec;
		tout.tv_usec = wait->tv_usec;

    		(void)gettimeofday (&tv1, (struct timezone *)0);
		sendtime = oicp->icmp_otime = (tv1.tv_sec % (24*60*60)) * 1000 
							+ tv1.tv_usec / 1000;
		oicp->icmp_cksum = in_cksum((u_short *)oicp, sizeof(*oicp));
	
		count = sendto(sock_raw, (char *)opacket, sizeof(*oicp), 0, 
				addr, sizeof(struct sockaddr_in));
		if (count < 0) {
			syslog(LOG_ERR, MSGSTR(SENDTO2,"sendto: %m"));
			return;
		}
		for (;;) {
			FD_SET(sock_raw, &ready);
			/* select only on the file descriptors that we
			 * are interested in since selecting on FD_SETSIZE
			 * causes performance problems
			 */
			if ((count = select(sock_raw+1, &ready, (fd_set *)0,
			    (fd_set *)0, &tout)) <= 0)
				break;
			cc = recvfrom(sock_raw, (char *)packet, PACKET_IN, 0, 
			    (struct sockaddr_in *)NULL, &length);
			(void)gettimeofday(&tv1, (struct timezone *)0);
			if (cc < 0)
				return(-1);
			icp = (struct icmp *)(packet + (ip->ip_hl << 2));
			if((icp->icmp_type == ICMP_TSTAMPREPLY) &&
			    icp->icmp_id == id && icp->icmp_seq == seqno)
				break;
		}
		if (count <= 0)
			continue;		/* resend */
		recvtime = (tv1.tv_sec % (24*60*60)) * 1000 +
		    tv1.tv_usec / 1000;
		diff = recvtime - sendtime;
		/*
		 * diff can be less than 0 aroud midnight
		 */
		if (diff < 0)
			continue;
		msgcount++;
		histime = ntohl((u_long)icp->icmp_rtime);
		/*
		 * a hosts using a time format different from 
		 * ms. since midnight UT (as per RFC792) should
		 * set the high order bit of the 32-bit time
		 * value it transmits.
		 */
		if ((histime & 0x80000000) != 0) {
			status = NONSTDTIME;
			break;
		}
		status = MMGOOD;
		delta1 = histime - sendtime;
		/*
		 * Handles wrap-around to avoid that around 
		 * midnight small time differences appear 
		 * enormous. However, the two machine's clocks
		 * must be within 12 hours from each other.
		 */
		if (delta1 < BIASN)
			delta1 += MODULO;
		else if (delta1 > BIASP)
			delta1 -= MODULO;
		delta2 = recvtime - histime;
		if (delta2 < BIASN)
			delta2 += MODULO;
		else if (delta2 > BIASP)
			delta2 -= MODULO;
		if (delta1 < min1)  
			min1 = delta1;
		if (delta2 < min2)
			min2 = delta2;
		if (diff < RANGE) {
			min1 = delta1;
			min2 = delta2;
			break;
		}
	}

	/*
	 * If no answer is received for TRIALS consecutive times, 
	 * the machine is assumed to be down
	 */
	 if (status == MMGOOD) {
		measure_delta = (min1 - min2)/2 + PROCESSING_TIME;
	}
	return(status);
}
in_cksum(addr, len)
u_short *addr;
int len;
{
	register int nleft = len;
	register u_short *w = addr, tmp;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		tmp = *(u_char *)w;
		sum += (tmp << 8);
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}
