static char sccsid[] = "@(#)31	1.75  src/bos/usr/sbin/ifconfig/ifconfig.c, cmdnet, bos412, 9446B 11/11/94 20:35:40";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: CMSGSTR
 *		MSGSTR
 *		Perror
 *		SIN
 *		SNS
 *		in_getaddr
 *		in_status
 *		load_if
 *		load_proto
 *		main
 *		notealias
 *		printb
 *		rqtosa
 *		setifaddr
 *		setifbroadaddr
 *		setifdstaddr
 *		setifflags
 *		setifipdst
 *		setifmetric
 *		setifmtu
 *		setifnetmask
 *		setifsubchan
 *		status
 *		unload_if
 *		xns_getaddr
 *		xns_status
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Copyright (c) 1983 Regents of the University of California.
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
 *
 *  (#)ifconfig.c	4.30 (Berkeley) 6/14/90
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>

#define	NSIP
#include <netns/ns.h>
#include <netns/ns_if.h>

#include <sys/protosw.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#ifdef _AIX
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <sys/devinfo.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <aixif/net_if.h>
#include <nlist.h>
#include <nl_types.h>
#include "ifconfig_msg.h"
#include <locale.h>
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd, MS_IFCONFIG, n, s)
#define	CMSGSTR(n,s) NC/**/n, MSGSTR(N/**/n, s)
#endif	_AIX

extern int errno;
struct	ifreq		ifr, ridreq;
struct	ifaliasreq	addreq;
struct	sockaddr_in	netmask;

char	name[30];
int	flags;
int	metric;
int	setaddr;
int	setipdst;
int	doalias;
int	clearaddr;
int	newaddr= 1 ;
int	s;

int	setifflags(), setifaddr(), setifdstaddr(), setifnetmask();
int	setifmetric(), setifbroadaddr(), setifipdst();
int	notealias();
#ifdef _AIX
int	setifmtu(), setifsubchan();
#endif _AIX

#define	NEXTARG		0xffffff

struct	cmd {
	char	*c_name;
	int	c_parameter;		/* NEXTARG means next argv */
	int	(*c_func)();
} cmds[] = {
	{ "up",		IFF_UP,		setifflags } ,
	{ "down",	-IFF_UP,	setifflags },
#ifdef _AIX
	{ "subchan",	NEXTARG,	setifsubchan },
#endif _AIX
#ifdef notdef
	{ "snap",	IFF_SNAP,	setifflags },
	{ "-snap",	-IFF_SNAP,	setifflags },
#endif
	{ "arp",	-IFF_NOARP,	setifflags },
	{ "-arp",	IFF_NOARP,	setifflags },
	{ "debug",	IFF_DEBUG,	setifflags },
	{ "-debug",	-IFF_DEBUG,	setifflags },
	{ "hwloop",	IFF_DO_HW_LOOPBACK,	setifflags },
	{ "-hwloop",	-IFF_DO_HW_LOOPBACK,	setifflags },
	{ "alias",	IFF_UP,		notealias },
	{ "-alias",	-IFF_UP,	notealias },
	{ "delete",	-IFF_UP,	notealias },
#ifdef notdef
#define	EN_SWABIPS	0x1000
	{ "swabips",	EN_SWABIPS,	setifflags },
	{ "-swabips",	-EN_SWABIPS,	setifflags },
#endif
	{ "netmask",	NEXTARG,	setifnetmask },
	{ "metric",	NEXTARG,	setifmetric },
	{ "broadcast",	NEXTARG,	setifbroadaddr },
#ifdef _AIX
	{ "allcast",	IFF_ALLCAST,	setifflags },
	{ "-allcast",	-IFF_ALLCAST,	setifflags },
	{ "mtu",	NEXTARG,	setifmtu },
#endif _AIX
	{ "ipdst",	NEXTARG,	setifipdst },
	{ 0,		0,		setifaddr },
	{ 0,		0,		setifdstaddr },
};

/*
 * XNS support liberally adapted from
 * code written at the University of Maryland
 * principally by James O'Toole and Chris Torek.
 */

int	in_status(), in_getaddr();
int	xns_status(), xns_getaddr();

/* Known address families */
struct afswtch {
	char *af_name;
	short af_af;
	int (*af_status)();
	int (*af_getaddr)();
	int af_difaddr;
	int af_aifaddr;
	caddr_t af_ridreq;
	caddr_t af_addreq;
} afs[] = {
#define C(x) ((caddr_t) &x)
	{ "inet", AF_INET, in_status, in_getaddr,
	     SIOCDIFADDR, SIOCAIFADDR, C(ridreq), C(addreq) },
	{ "ns", AF_NS, xns_status, xns_getaddr,
	     SIOCDIFADDR, SIOCAIFADDR, C(ridreq), C(addreq) },
	{ 0,	0,	    0,		0 }
};

struct afswtch *afp;	/*the address family being set or asked about*/

main(argc, argv)
	int argc;
	char *argv[];
{
	int af = AF_INET;
	register struct afswtch *rafp;
	int i;

	setlocale(LC_ALL, "");
	catd = catopen(MF_IFCONFIG, NL_CAT_LOCALE);
	if ((argc < 2) || (!strcmp(argv[1],"-?")) || (!strcmp(argv[1],"?"))) {
		fprintf(stderr,
			MSGSTR(N_USAGE,
			"usage: %s interface\n%s%s%s%s%s%s%s%s%s%s"),
			argv[0],
			MSGSTR(N_USAGE2,
"\t[ af ] [ address [ dest_addr ] ] [ up ] [ down ] [ detach ]\n"),
 			MSGSTR(N_USAGE3, "\t[ netmask mask ] [ broadcast broad_addr ]\n"),
			MSGSTR(N_USAGE5, "\t[ mtu n ]\n"),
			MSGSTR(N_USAGEA, "\t[ allcast | -allcast ]\n"),
			MSGSTR(N_USAGEB, "\t[ arp | -arp ]\n"),
			MSGSTR(N_USAGED, "\t[ debug | -debug ]\n"));
		exit(1);
	}
	argc--, argv++;
	strncpy(name, *argv, sizeof(name));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	argc--, argv++;
	if (argc > 0) {
		for (afp = rafp = afs; rafp->af_name; rafp++)
			if (strcmp(rafp->af_name, *argv) == 0) {
				afp = rafp; argc--; argv++;
				break;
			}
		rafp = afp;
		af = ifr.ifr_addr.sa_family = rafp->af_af;
	}
	if (af == AF_ISO) {
		fprintf(stderr, MSGSTR(NOT_SUPPORT,
			"ifconfig: %s: Not Supported.\n"),rafp->af_name);
		exit(1);
	} 

#ifdef	_AIX
	if (argc > 0) 		/* don't worry about proto if query only */
		load_proto();
#endif	_AIX

	s = socket(af, SOCK_DGRAM, 0);
	if (s < 0) {
		perror(MSGSTR(NE_SOCKET, "ifconfig: socket"));
		exit(1);
	}

#ifdef _AIX
        /* If a detach just doit and exit */
        for (i=0; i<argc;) {
                if (strcmp(argv[i],"detach") == 0) {
			if (strcmp(name,"lo0")) {
				if (ioctl(s, SIOCIFDETACH, (caddr_t)&ifr) < 0) {
					perror(MSGSTR(NE_SIOCIFDETACH,
						"ioctl (SIOCIFDETACH)"));
					exit(1);
				} else
					exit(0);
			} else
				exit(0);
                }
		i++;
        }
#endif _AIX

	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
#ifdef _AIX
		/* Couldn't find the interface. Call load_if() to
		 * try to load the extension. If it succeeds try the
		 * SIOCGIFLAGS again. Should work this time...
		 */
		if (load_if(ifr.ifr_name) != 0)
			exit(1);


		if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
			Perror(CMSGSTR(_SIOCGIFFLAGS, "ioctl (SIOCGIFFLAGS)"));
			exit(1);
		}
#else  _AIX
		Perror(CMSGSTR(_SIOCGIFFLAGS, "ioctl (SIOCGIFFLAGS)"));
		exit(1);
#endif _AIX
	}

	strncpy(ifr.ifr_name, name, sizeof ifr.ifr_name);
	flags = ifr.ifr_flags;
	if (ioctl(s, SIOCGIFMETRIC, (caddr_t)&ifr) < 0)
		perror(MSGSTR(NE_SIOCIFMETRIC, "ioctl (SIOCGIFMETRIC)"));
	else
		metric = ifr.ifr_metric;
	if (argc == 0) {
		status();
		exit(0);
	}

#ifdef _AIX
	/*
	 * Issue the ATTACH ioctl that will cause all protocols to
	 * add the appropriate CDLI filters...
	 */
	if (ioctl(s, SIOCIFATTACH, (caddr_t)&ifr) < 0) {
		perror(MSGSTR(NE_SIOCIFATTACH, "ioctl (SIOCIFATTACH)"));
		exit(1);
	}
#endif /* _AIX */

	while (argc > 0) {
		register struct cmd *p;

		for (p = cmds; p->c_name; p++)
			if (strcmp(*argv, p->c_name) == 0)
				break;
		if (p->c_name == 0 && setaddr)
			p++;	/* got src, do dst */
		if (p->c_func) {
			if (p->c_parameter == NEXTARG) {
				(*p->c_func)(argv[1]);
				argc--, argv++;
			} else
				(*p->c_func)(*argv, p->c_parameter);
		}
		argc--, argv++;
	}
	if (setipdst && af==AF_NS) {
		struct nsip_req rq;
		int size = sizeof(rq);

		rq.rq_ns = addreq.ifra_addr;
		rq.rq_ip = addreq.ifra_dstaddr;

		if (setsockopt(s, 0, SO_NSIP_ROUTE, &rq, size) < 0)
			Perror(CMSGSTR(_ENCAP, "Encapsulation Routing"));
	}
	if (clearaddr) {
		int ret;
		strncpy(rafp->af_ridreq, name, sizeof ifr.ifr_name);
		if ((ret = ioctl(s, rafp->af_difaddr, rafp->af_ridreq)) < 0) {
			if (errno == EADDRNOTAVAIL && (doalias >= 0)) {
				/* means no previous address for interface */
			} else
				Perror(CMSGSTR(_SIOCDIFADDR, "ioctl (SIOCDIFADDR)"));
		}
	}
	if (newaddr) {
		strncpy(rafp->af_addreq, name, sizeof ifr.ifr_name);
		if (ioctl(s, rafp->af_aifaddr, rafp->af_addreq) < 0) 
			Perror(CMSGSTR(_SIOCAIFADDR, "ioctl (SIOCAIFADDR)"));
	}
	exit(0);
}

#define RIDADDR 0
#define ADDR	1
#define MASK	2
#define DSTADDR	3

/*ARGSUSED*/
setifaddr(addr, param)
	char *addr;
	short param;
{
	/*
	 * Delay the ioctl to set the interface addr until flags are all set.
	 * The address interpretation may depend on the flags,
	 * and the flags may change when the address is set.
	 */
	setaddr++;
	if (doalias == 0)
		clearaddr = 1;
	(*afp->af_getaddr)(addr, (doalias >= 0 ? ADDR : RIDADDR));
}

setifnetmask(addr)
	char *addr;
{
	(*afp->af_getaddr)(addr, MASK);
}

#ifdef _AIX
setifsubchan(addr)
	char *addr;
{
	int	subchan_adr;
     int  indx;

     if ( (addr[1] == 'x') && (addr[0] == '0') ) {  /* is this a hex value  */
          /* validate the string */
          indx = 2;
          if ( addr[indx] == 0){
               fprintf(stderr,
                     MSGSTR(BADSUBCHAN, "bad subchannel address\n"));
               exit(1);
          }
          while (addr[indx] != 0){
               if ((!isxdigit(addr[indx])) || (addr[2] == 0) ) {
                  fprintf(stderr,
                       MSGSTR(BADSUBCHAN, "bad subchannel address\n"));
                  exit(1);
               }
               indx++;
          }
          subchan_adr = strtol(addr,NULL,16);
     } else {
          if ( addr[0] == '0' ) {  /* is this octal */
               indx=0;
               while (addr[indx] != 0 ){
                    if( ( addr[indx] < 0x30 ) || (addr[indx] > 0x37)){
                         fprintf(stderr,
                              MSGSTR(BADSUBCHAN, "bad subchannel address\n"));
                         exit(1);
                    }
                    indx++;
               }
               subchan_adr = strtol(addr,NULL,8);
          } else {
               indx = 0;
               while (addr[indx] != 0){
                    if (!isdigit(addr[indx])) {
                         fprintf(stderr,
                              MSGSTR(BADSUBCHAN, "bad subchannel address\n"));
                         exit(1);
                    }
                    indx++;
               }
               subchan_adr = atoi(addr);
          }
     }


	if (subchan_adr < 0 || subchan_adr >255  || (subchan_adr %1) != 0) {
		fprintf(stderr,
			MSGSTR(BADSUBCHAN, "bad subchannel address\n"));
		exit(1);
	}
	ifr.ifr_flags = subchan_adr; /* use flags as subchannel, type is long */
 	if (ioctl(s, SIOCSIFSUBCHAN, (caddr_t)&ifr) < 0) {
		perror(MSGSTR(NE_SIOCSIFSUBCHAN,"ioctl (set subchannel addr.)"));
		exit(1);
	}	
}
#endif _AIX

setifbroadaddr(addr)
	char *addr;
{
	(*afp->af_getaddr)(addr, DSTADDR);
}

setifipdst(addr)
	char *addr;
{
	in_getaddr(addr, DSTADDR);
	setipdst++;
	clearaddr = 0;
	newaddr = 0;
}
#define rqtosa(x) (&(((struct ifreq *)(afp->x))->ifr_addr))
/*ARGSUSED*/
/* for delete, alias command not yet implemented */
notealias(addr, param)
	char *addr;
{
	if (setaddr && doalias == 0 && param < 0)
		bcopy((caddr_t)rqtosa(af_addreq),
		      (caddr_t)rqtosa(af_ridreq),
		      rqtosa(af_addreq)->sa_len);
	doalias = param;
	if (param < 0) {
		clearaddr = 1;
		newaddr = 0;
	} else
		clearaddr = 0;
}

/*ARGSUSED*/
setifdstaddr(addr, param)
	char *addr;
	int param;
{
	(*afp->af_getaddr)(addr, DSTADDR);
}
setifflags(vname, value)
	char *vname;
	int value;
{
 	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
		Perror(CMSGSTR(_SIOCGIFFLAGS, "ioctl (SIOCGIFFLAGS)"));
 		exit(1);
 	}
	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
 	flags = ifr.ifr_flags;

	if (value < 0) {
		value = -value;
		flags &= ~value;
	} else
		flags |= value;
	ifr.ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0)
		Perror(NC_SIOCSIFFLAGS, vname);
}

setifmetric(val)
	char *val;
{
	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	ifr.ifr_metric = atoi(val);
	if (ioctl(s, SIOCSIFMETRIC, (caddr_t)&ifr) < 0)
		perror(MSGSTR(NE_SIOCSIFMETRIC, "ioctl (set metric)"));
}

#ifndef IP_MULTICAST
#define	IFFBITS \
"\020\1UP\2BROADCAST\3DEBUG\4LOOPBACK\5POINTOPOINT\6NOTRAILERS\7RUNNING\10NOARP\
\11PROMISC\12ALLMULTI\13OACTIVE\14SIMPLEX\20SNAP\21HWLOOP\22ALLCAST\23BRIDGE"
#else
#define	IFFBITS \
"\020\1UP\2BROADCAST\3DEBUG\4LOOPBACK\5POINTOPOINT\6NOTRAILERS\7RUNNING\10NOARP\
\11PROMISC\12ALLMULTI\13OACTIVE\14SIMPLEX\20SNAP\21HWLOOP\22ALLCAST\23BRIDGE\24MULTICAST"
#endif IP_MULTICAST
/*
 * Print the status of the interface.  If an address family was
 * specified, show it and it only; otherwise, show them all.
 */
status()
{
	register struct afswtch *p = afp;
	short af = ifr.ifr_addr.sa_family;

	printf("%s: ", name);
	printb(MSGSTR(N_FLAGS, "flags"), flags, IFFBITS);
	if (metric)
		printf(MSGSTR(N_METRIC," metric %d"), metric);
	putchar('\n');
	if ((p = afp) != NULL) {
		(*p->af_status)(1);
	} else for (p = afs; p->af_name; p++) {
		ifr.ifr_addr.sa_family = p->af_af;
		(*p->af_status)(0);
	}
}

in_status(force)
	int force;
{
	struct sockaddr_in *sin ;
	char *inet_ntoa();

	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	if (ioctl(s, SIOCGIFADDR, (caddr_t)&ifr) < 0) {
		if (errno == EADDRNOTAVAIL || errno == EAFNOSUPPORT) {
			if (!force)
				return;
			bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
		} else
			perror(MSGSTR(NE_SIOCGIFADDR, "ioctl (SIOCGIFADDR)"));
	}
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	printf(MSGSTR(N_INET, "\tinet %s "), inet_ntoa(sin->sin_addr));
	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	if (ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			perror(MSGSTR(NE_SIOCGIFNETMASK,"ioctl (SIOCGIFNETMASK)"));
		bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
	} else
		netmask.sin_addr =
		    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	if (flags & IFF_POINTOPOINT) {
		if (ioctl(s, SIOCGIFDSTADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			else
			    perror(MSGSTR(NE_SIOCGIFDSTADDR,
						"ioctl (SIOCGIFDSTADDR)"));
		}
		strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
		sin = (struct sockaddr_in *)&ifr.ifr_dstaddr;
		printf("--> %s ", inet_ntoa(sin->sin_addr));
	}
	printf(MSGSTR(N_NETMASK, "netmask 0x%x "),ntohl(netmask.sin_addr.s_addr));
	if (flags & IFF_BROADCAST) {
		if (ioctl(s, SIOCGIFBRDADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			else
			    perror(MSGSTR(NE_SIOCGIFADDR, "ioctl (SIOCGIFADDR)"));
		}
		strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		if (sin->sin_addr.s_addr != 0)
			printf(MSGSTR(N_BROADCAST, "broadcast %s"),
						inet_ntoa(sin->sin_addr));
	}
	putchar('\n');
}

xns_status(force)
	int force;
{
	struct sockaddr_ns *sns;

	close(s);
	s = socket(AF_NS, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno == EPROTONOSUPPORT)
			return;
		perror(MSGSTR(NE_SOCKET, "ifconfig: socket"));
		exit(1);
	}
	if (ioctl(s, SIOCGIFADDR, (caddr_t)&ifr) < 0) {
		if (errno == EADDRNOTAVAIL || errno == EPFNOSUPPORT) {
			if (!force)
				return;
			bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
		} else
			perror(MSGSTR(NE_SIOCGIFADDR, "ioctl (SIOCGIFADDR)"));
	}
	strncpy(ifr.ifr_name, name, sizeof ifr.ifr_name);
	sns = (struct sockaddr_ns *)&ifr.ifr_addr;
	printf(MSGSTR(N_NS, "\tns %s "), ns_ntoa(sns->sns_addr));
	if (flags & IFF_POINTOPOINT) { /* by W. Nesheim@Cornell */
		if (ioctl(s, SIOCGIFDSTADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    bzero((char *)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			else
			    perror(MSGSTR(NE_SIOCGIFDSTADDR,
						"ioctl (SIOCGIFDSTADDR)"));
		}
		strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
		sns = (struct sockaddr_ns *)&ifr.ifr_dstaddr;
		printf("--> %s ", ns_ntoa(sns->sns_addr));
	}
	putchar('\n');
}

Perror(code, cmd)
	int code;
	char *cmd;
{
	extern int errno;

	if (code == -1) fprintf(stderr, MSGSTR(code, ""));
	fprintf(stderr, "ifconfig: ");
	switch (errno) {

	case ENXIO:
		fprintf(stderr, MSGSTR(N_NOSUCH, "%s: no such interface\n"),
			cmd);
		break;

	case EPERM:
		fprintf(stderr, MSGSTR(N_NOPERM, "%s: permission denied\n"),
			cmd);
		break;

	default:
		perror(cmd);
	}
	exit(1);
}

struct	in_addr inet_makeaddr();

#define SIN(x) ((struct sockaddr_in *) &(x))
struct sockaddr_in *sintab[] = {
SIN(ridreq.ifr_addr), SIN(addreq.ifra_addr),
SIN(addreq.ifra_mask), SIN(addreq.ifra_broadaddr)};

in_getaddr(s, which)
	char *s;
{
	register struct sockaddr_in *sin = sintab[which];
	struct hostent *hp;
	struct netent *np;
	int val;

	sin->sin_len = sizeof(*sin);
	if (which != MASK)
		sin->sin_family = AF_INET;

	/* see if it's a name or an address */
	if (isinet_addr(s))
		sin->sin_addr.s_addr = inet_addr(s);
	else if (hp = gethostbyname(s))
		bcopy(hp->h_addr, (char *)&sin->sin_addr, hp->h_length);
	else if (np = getnetbyname(s))
		sin->sin_addr = inet_makeaddr(np->n_net, INADDR_ANY);
	else {
		fprintf(stderr, MSGSTR(NE_BADVAL, "%s: bad value\n"), s);
		exit(1);
	}
}

/*
 * Print a value a la the %b format of the kernel's printf
 */
printb(s, v, bits)
	char *s;
	register char *bits;
	register unsigned int v;
{
	register int i, any = 0;
	register char c;

	if (bits && *bits == 8)
		printf("%s=%o", s, v);
	else
		printf("%s=%x", s, v);
	bits++;
	if (bits) {
		putchar('<');
		while (i = *bits++) {
			if (v & (1 << (i-1))) {
				if (any)
					putchar(',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					putchar(c);
			} else
				for (; *bits > 32; bits++)
					;
		}
		putchar('>');
	}
}

#define SNS(x) ((struct sockaddr_ns *) &(x))
struct sockaddr_ns *snstab[] = {
SNS(ridreq.ifr_addr), SNS(addreq.ifra_addr),
SNS(addreq.ifra_mask), SNS(addreq.ifra_broadaddr)};

xns_getaddr(addr, which)
char *addr;
{
	struct sockaddr_ns *sns = snstab[which];
	struct ns_addr ns_addr();

	sns->sns_family = AF_NS;
	sns->sns_len = sizeof(*sns);
	sns->sns_addr = ns_addr(addr);
	if (which == MASK)
		printf(MSGSTR(NE_ATTEMPT, "Attempt to set XNS netmask will be ineffectual\n"));
}

#ifdef _AIX
setifmtu(vname)
	char *vname;
{
	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	ifr.ifr_mtu = atol(vname);
	if (ioctl(s, SIOCSIFMTU, (caddr_t)&ifr) < 0)
		perror(MSGSTR(NE_SIOCSIFMTU,"ioctl (SIOCSIFMTU)"));
}

#define  NETDIR  "/usr/lib/drivers/"
load_proto() 
{
	char path[128];
	struct cfg_load load;
	struct cfg_kmod kmod;

	bzero(&load, sizeof(load));
	bzero(&kmod, sizeof(kmod));
	load.path = path;
	sprintf(path, "%s%s%s", NETDIR, "net", afp->af_name);
		
	/* if the protocol code hasn't been loaded yet,
	 * then load it up
	 */
	if (sysconfig(SYS_QUERYLOAD, &load, sizeof(load)) == CONF_FAIL) {
		fprintf(stderr, MSGSTR(NE_KEXT,
			"ifconfig: error querying kernel extension "));
		perror(load.path);
		exit(1);
	}
	if (load.kmid == 0) {
		if (sysconfig(SYS_KLOAD, &load, sizeof(load)) == CONF_FAIL) {
			fprintf(stderr, MSGSTR(NE_ERR_LOAD,
				"ifconfig: error loading "));
			perror(load.path);
			exit(1);
		}
		kmod.cmd = CFG_INIT;
		kmod.kmid = load.kmid; /* move module id over */
		kmod.mdiptr = (caddr_t)&kmod.kmid;
		kmod.mdilen = sizeof(kmod.kmid);
		
		/* call sysconfig() to config protocol */
		if (sysconfig(SYS_CFGKMOD, &kmod, sizeof(kmod)) == CONF_FAIL) {
			fprintf(stderr, MSGSTR(NE_CFGKMOD,
				"sysconfig(SYS_CFGKMOD) failed! \n"));
			perror(load.path);
			exit(1);
		}
		(void) srcaddinet(); /* inform SRC inet is up */
	}
}

load_if(if_name)
	char *if_name;
{
	struct	device_req	device;
	char			dev_name[16];
	char			*cp;
	char 			path[128];
	struct cfg_load 	load;
	struct cfg_kmod 	kmod;

	/* we never try to load lo0 */
	if (strcmp(if_name, "lo0") == 0)
		return(0);

	bzero(&load, sizeof(load));
	bzero(&kmod, sizeof(kmod));
	bzero(&device, sizeof(struct device_req));
	strcpy(device.dev_name, if_name); 

	/* 
	 * Set up the load path of the IF kernel extension. Special
	 * case the non-conformers.
	 */
	load.path = path;
	strcpy(dev_name, if_name); 
        cp = dev_name;
        while(*cp < '0' || *cp > '9') cp++;
	*cp = '\0';
	if (!strcmp(dev_name, "et"))
		sprintf(path, "%sif_en", NETDIR);
	else if (!strcmp(dev_name, "so"))
		sprintf(path, "%sif_op", NETDIR);
	else if (!strcmp(dev_name, "fi"))
		sprintf(path, "%sif_fd", NETDIR);
	else 
		sprintf(path, "%sif_%s", NETDIR, dev_name);

	/* The network interface extension may already be loaded for 
	 * another unit of the interface type. Need to check before
	 * loading or we will get multiple copies
	 */
	if (sysconfig(SYS_QUERYLOAD, &load, sizeof(load)) == CONF_FAIL) {
		fprintf(stderr, MSGSTR(NE_KEXT,
				"ifconfig: error querying kernel extension "));
		perror(load.path);
		return(-1);
	}

	if (load.kmid == 0) {
		if (sysconfig(SYS_KLOAD, &load, sizeof(load)) == CONF_FAIL) {
			fprintf(stderr, MSGSTR(NE_KLOAD,
						"ifconfig: error loading "));
			perror(load.path);
			return(-1);
		}
	}
	kmod.kmid = load.kmid; 
	kmod.cmd = CFG_INIT;
	/* device struct is passed to the IF via uio feature of sysconfig */
	kmod.mdiptr = (caddr_t)&device;
	kmod.mdilen = sizeof(device);
	
	/* call sysconfig() to config the driver */
	if (sysconfig(SYS_CFGKMOD, &kmod, sizeof(kmod)) == CONF_FAIL) {
		fprintf(stderr, MSGSTR(NE_EENTRY,
				"ifconfig: error calling entry point for "));
		perror(load.path);
		return(-1);
	}
	(void) srcaddinet(); /* inform SRC inet and interface is up */
	return(0);
}
#endif _AIX
