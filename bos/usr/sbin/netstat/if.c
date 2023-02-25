static char sccsid[] = "@(#)13	1.6.1.8  src/bos/usr/sbin/netstat/if.c, cmdnet, bos41J, 9508A 2/2/95 17:56:23";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: MSGSTR
 *		catchalarm
 *		driverprint
 *		intpr
 *		sidewaysintpr
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Copyright (c) 1983, 1988 Regents of the University of California.
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

#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <net/nh.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netns/ns.h>
#include <netns/ns_if.h>
#include <netiso/iso.h>
#include <netiso/iso_var.h>

#include <stdio.h>
#include <signal.h>

#define	YES	1
#define	NO	0

extern	int tflag;
extern	int dflag;
extern	int nflag;
extern	int Zflag;
extern	char *interface;
extern	int unit;
extern	char *routename(), *netname(), *ns_phost();
extern	int kmem;

#include <nl_types.h>
#include "netstat_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s) 

#ifdef IP_MULTICAST
extern int aflag;
#endif

/*
 * Print a description of the network interfaces.
 */
intpr(interval, ifnetaddr)
	int interval;
	off_t ifnetaddr;
{
	struct ifnet ifnet;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
		struct ns_ifaddr ns;
		struct iso_ifaddr iso;
	} ifaddr;
	off_t ifaddraddr;
#ifdef IP_MULTICAST
	int arpcom_base;
#endif
	struct sockaddr *sa;
	char name[16];

	if (ifnetaddr == 0) {
		printf(MSGSTR(SYM_NO_DEF, "ifnet: symbol not defined\n"));
		return;
	}
	if (Zflag) {
		if_zero_stats(ifnetaddr);
		return;
	}	
	if (interval) {
		sidewaysintpr((unsigned)interval, ifnetaddr);
		return;
	}
	kvm_read(ifnetaddr, (char *)&ifnetaddr, sizeof ifnetaddr);
	printf(MSGSTR(HEADING1, "Name  Mtu   Network     Address            Ipkts Ierrs    Opkts Oerrs")); /*MSG*/
	printf(MSGSTR(HEADING2, "  Coll")); /*MSG*/
	if (tflag)
		printf(MSGSTR(HEADING3, "  Time"));
	if (dflag)
		printf(MSGSTR(HEADING3a, "  Drop"));
	putchar('\n');
	ifaddraddr = 0;
	while (ifnetaddr || ifaddraddr) {
		struct sockaddr_in *sin;
		register char *cp;
		int n, m;
		char *index();
		struct in_addr inet_makeaddr();
		if (ifaddraddr == 0) {
#ifdef IP_MULTICAST
		        arpcom_base = ifnetaddr;
#endif
			kvm_read(ifnetaddr, (char *)&ifnet, sizeof ifnet);
			kvm_read((off_t)ifnet.if_name, name, 16);
			name[15] = '\0';
			ifnetaddr = (off_t) ifnet.if_next;
			if (interface != 0 &&
			    (strcmp(name, interface) != 0 || unit != ifnet.if_unit))
				continue;
			(void) sprintf(name, "%s%d", name, ifnet.if_unit);
			cp = index (name, '\0');
			if ((ifnet.if_flags&IFF_UP) == 0)
				*cp++ = '*';
			*cp = '\0';
			ifaddraddr = (off_t)ifnet.if_addrlist;
		}
		printf("%-5.5s %-5d ", name, ifnet.if_mtu);

		if (ifaddraddr == 0) {
			printf(MSGSTR(NONE11, "none       "));
			printf(MSGSTR(NONE15, "none           "));
		} else {
			kvm_read(ifaddraddr, (char *)&ifaddr, sizeof ifaddr);
#define CP(x) ((char *)(x))
			cp = (CP(ifaddr.ifa.ifa_addr) - CP(ifaddraddr)) +
				CP(&ifaddr); sa = (struct sockaddr *)cp;
			switch (sa->sa_family) {
			case AF_UNSPEC:
				printf(MSGSTR(NONE11, "none       "));
				printf(MSGSTR(NONE15, "none           "));
				break;
			case AF_INET:
				sin = (struct sockaddr_in *)sa;
#ifdef notdef
				/* can't use inet_makeaddr because kernel
				 * keeps nets unshifted.
				 */
				in = inet_makeaddr(ifaddr.in.ia_subnet,
					INADDR_ANY);
				printf("%-11.11s ", netname(in));
#else
				printf("%-11.11s ",
					netname(htonl(ifaddr.in.ia_subnet),
						0L));
#endif
				printf("%-15.15s ", routename(sin->sin_addr));
				break;
			case AF_NS:
				{
				struct sockaddr_ns *sns =
					(struct sockaddr_ns *)sa;
				u_long net;
				char netnum[8];
				char *ns_phost();

				*(union ns_net *) &net = sns->sns_addr.x_net;
		sprintf(netnum, "%lxH", ntohl(net));
				upHex(netnum);
				printf("ns:%-8s ", netnum);
				printf("%-15s ", ns_phost(sns));
				}
				break;
			case AF_LINK:
				{
				struct sockaddr_dl *sdl =
					(struct sockaddr_dl *)sa;
				    cp = (char *)LLADDR(sdl);
				    n = sdl->sdl_alen;
				}
				m = printf(MSGSTR(LINK, "<Link>"));
				goto hexprint;
			default:
				m = printf("(%d)", sa->sa_family);
				for (cp = sa->sa_len + (char *)sa;
					--cp > sa->sa_data && (*cp == 0);) {}
				n = cp - sa->sa_data + 1;
				cp = sa->sa_data;
			hexprint:
				while (--n >= 0)
					m += printf("%x%c", *cp++ & 0xff,
						    n > 0 ? '.' : ' ');
				m = 28 - m;
				while (m-- > 0)
					putchar(' ');
				break;
			}
			ifaddraddr = (off_t)ifaddr.ifa.ifa_next;
		}
		printf("%8d %5d %8d %5d %5d",
		    ifnet.if_ipackets, ifnet.if_ierrors,
		    ifnet.if_opackets, ifnet.if_oerrors,
		    ifnet.if_collisions);
		if (tflag)
			printf(" %5d", ifnet.if_timer);
		if (dflag)
			printf(" %5d", ifnet.if_snd.ifq_drops);
		putchar('\n');
#ifdef IP_MULTICAST
		if (aflag && ifaddraddr == 0) {
		  /*
		   * print any internet multicast addresses
		   */
		  off_t multiaddr;
		  struct in_multi inm;
		  int my_nflag = nflag;

		  nflag = 1; /* avoid waiting for name lookup */
		  multiaddr = (off_t)ifnet.if_multiaddrs;
		  while (multiaddr != 0) {
		    kvm_read(multiaddr, (char *)&inm, sizeof inm);
		    multiaddr = inm.inm_next;
		    printf("%23s %-19.19s\n", "",
			   routename(inm.inm_addr));
		  }
		  nflag = my_nflag;
		  /*
		   * print link-level addresses
		   * (Is there a better way to determine
		   *  the type of network??)
		   */
		  if (strncmp(name, "en", 2) == 0 ||    /* Ethernet   */
		      strncmp(name, "et", 2) == 0 ||    /* ISO 8802.3 */
		      strncmp(name, "tr", 2) == 0 ||    /* ISO 8802.5 */
		      strncmp(name, "fi", 2) == 0 ) {   /* FDDI       */
		    off_t multiaddr;
		    struct arpcom ac;
		    struct driver_multi enm;
		    char *driverprint();
		    
		    kvm_read(arpcom_base, (char *)&ac, sizeof(ac));
		    if(ac.ac_multiaddrs) {
		    /*  printf("%23s %s\n", "",
			     driverprint(ac.ac_hwaddr)); */
		      multiaddr = (off_t)ac.ac_multiaddrs;
		      while (multiaddr != 0) {
			kvm_read(multiaddr, (char *)&enm, sizeof enm);
			multiaddr = (off_t)enm.enm_next;
			printf("%23s %s", "",
			       driverprint(enm.enm_addrlo));
			if (bcmp(enm.enm_addrlo, enm.enm_addrhi, 6))
			  printf(" to %s",
				 driverprint(enm.enm_addrhi));
			printf("\n");
		      }
		    }
		  }
		}
#endif
	}
}
#ifdef IP_MULTICAST
/*
 * Return a printable string representation of a 48 bit MAC address.
 */
char *driverprint(enaddr)
	char enaddr[6];
{
	static char string[18];
	unsigned char *en = (unsigned char *)enaddr;

	sprintf(string, "%02x:%02x:%02x:%02x:%02x:%02x",
		en[0], en[1], en[2], en[3], en[4], en[5] );
	string[17] = '\0';
	return(string);
}
#endif

#define	MAXIF	50
struct	iftot {
	char	ift_name[16];		/* interface name */
	int	ift_ip;			/* input packets */
	int	ift_ie;			/* input errors */
	int	ift_op;			/* output packets */
	int	ift_oe;			/* output errors */
	int	ift_co;			/* collisions */
	int	ift_dr;			/* drops */
} iftot[MAXIF];

u_char	signalled;			/* set if alarm goes off "early" */

/*
 * Print a running summary of interface statistics.
 * Repeat display every interval seconds, showing statistics
 * collected over that interval.  Assumes that interval is non-zero.
 * First line printed at top of screen is always cumulative.
 */
sidewaysintpr(interval, off)
	unsigned interval;
	off_t off;
{
	struct ifnet ifnet;
	off_t firstifnet;
	register struct iftot *ip, *total;
	register int line;
	struct iftot *lastif, *sum, *interesting;
	int oldmask;
	int catchalarm();

	kvm_read(off, (char *)&firstifnet, sizeof (off_t));
	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	for (off = firstifnet, ip = iftot; off;) {
		char *cp;

		kvm_read(off, (char *)&ifnet, sizeof ifnet);
		ip->ift_name[0] = '(';
		kvm_read((off_t)ifnet.if_name, ip->ift_name + 1, 15);
		if (interface && strcmp(ip->ift_name + 1, interface) == 0 &&
		    unit == ifnet.if_unit)
			interesting = ip;
		ip->ift_name[15] = '\0';
		cp = index(ip->ift_name, '\0');
		sprintf(cp, "%d)", ifnet.if_unit);
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
		off = (off_t) ifnet.if_next;
	}
	lastif = ip;

	(void)signal(SIGALRM, catchalarm);
	signalled = NO;
	(void)alarm(interval);
banner:
	printf(MSGSTR(BANNER1, "    input    %-6.6s    output      "), interesting->ift_name);
	if (lastif - iftot > 0) {
		if (dflag)
			printf("      ");
		printf(MSGSTR(BANNER2, "     input   (Total)    output"));
	}
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
		ip->ift_dr = 0;
	}
	putchar('\n');
        printf(MSGSTR(VALUES1, " packets  errs  packets  errs colls "));
	if (dflag)
		printf(MSGSTR(DROPS, "drops "));
	if (lastif - iftot > 0)
                printf(MSGSTR(VALUES1, " packets  errs  packets  errs colls "));
	if (dflag)
		printf(MSGSTR(DROPS, "drops "));
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	sum->ift_dr = 0;
	for (off = firstifnet, ip = iftot; off && ip < lastif; ip++) {
		kvm_read(off, (char *)&ifnet, sizeof ifnet);
		if (ip == interesting) {
			printf("%8d %5d %8d %5d %5d",
				ifnet.if_ipackets - ip->ift_ip,
				ifnet.if_ierrors - ip->ift_ie,
				ifnet.if_opackets - ip->ift_op,
				ifnet.if_oerrors - ip->ift_oe,
				ifnet.if_collisions - ip->ift_co);
			if (dflag)
				printf(" %5d",
				    ifnet.if_snd.ifq_drops - ip->ift_dr);
		}
		ip->ift_ip = ifnet.if_ipackets;
		ip->ift_ie = ifnet.if_ierrors;
		ip->ift_op = ifnet.if_opackets;
		ip->ift_oe = ifnet.if_oerrors;
		ip->ift_co = ifnet.if_collisions;
		ip->ift_dr = ifnet.if_snd.ifq_drops;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
		sum->ift_dr += ip->ift_dr;
		off = (off_t) ifnet.if_next;
	}
	if (lastif - iftot > 0) {
		printf(" %8d %5d %8d %5d %5d",
			sum->ift_ip - total->ift_ip,
			sum->ift_ie - total->ift_ie,
			sum->ift_op - total->ift_op,
			sum->ift_oe - total->ift_oe,
			sum->ift_co - total->ift_co);
		if (dflag)
			printf(" %5d", sum->ift_dr - total->ift_dr);
	}
	*total = *sum;
	putchar('\n');
	fflush(stdout);
	line++;
	oldmask = sigblock(sigmask(SIGALRM));
	if (! signalled) {
		sigpause(0);
	}
	sigsetmask(oldmask);
	signalled = NO;
	(void)alarm(interval);
	if (line == 21)
		goto banner;
	goto loop;
	/*NOTREACHED*/
}

/*
 * Called if an interval expires before sidewaysintpr has completed a loop.
 * Sets a flag to not wait for the alarm.
 */
catchalarm()
{
	signalled = YES;
}

if_zero_stats(ifnetaddr)
caddr_t ifnetaddr;
{
	struct	ifnet	ifnet;

#define	zeroit(field) {	\
	(void) lseek(kmem, ifnetaddr+offsetof(struct ifnet, field), 0); \
	ifnet.field = 0; \
	(void) write(kmem, &ifnet.field, sizeof(ifnet.field)); \
}

	kvm_read(ifnetaddr, (char *)&ifnetaddr, sizeof ifnetaddr);
	while (ifnetaddr) {
		kvm_read(ifnetaddr, (char *)&ifnet, sizeof ifnet);
		zeroit(if_ipackets);
		zeroit(if_ierrors);
		zeroit(if_opackets);
		zeroit(if_oerrors);
		zeroit(if_collisions);
		zeroit(if_ibytes);
		zeroit(if_obytes);
		zeroit(if_imcasts);
		zeroit(if_omcasts);
		zeroit(if_iqdrops);
		zeroit(if_noproto);
		zeroit(if_snd.ifq_drops);
		ifnetaddr = (off_t) ifnet.if_next;
	}
}
