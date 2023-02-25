static char sccsid[] = "@(#)87	1.27  src/bos/usr/sbin/netstat/main.c, cmdnet, bos41J, 9513A_all 3/21/95 23:40:52";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: MSGSTR
 *		_afpr
 *		afpr
 *		knownname
 *		kvm_read
 *		main
 *		name2protox
 *		plural
 *		pluraly
 *		symbol_err
 *		usage
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
/* Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <nlist.h>
#include <storclass.h>
#include <stdio.h>
#include <paths.h>

#define nl netstatnl
struct nlist nl[] = {
#define	N_FILE		0
    { "file" },
#define	N_IFNET		1
    { "ifnet" },
#define	N_MBSTAT	2
    { "mbstat" },
#define N_RTREE		3
    { "radix_node_head" },
#define	N_RAWCB		4
    { "rawcb" },
#define N_KMEMNAMES	5
    { "kmemnames" },
#define	N_BUCKETS	6
    { "bucket" },
#define	N_KMEMSTATS	7
    { "kmemstats" },
#define	N_RTSTAT	8
    { "rtstat" },
#define	N_UNIXSW	9
    { "unixsw" },
#define N_V		10
    { "v" },
#define N_SYSTEMCONFIG	11
    { "_system_configuration" },

    /* These variables don't seem to exist anywhere */

#define	N_TPSTAT	12
    { "tp_stat" },
#define N_IDP		13
    { "nspcb"},
#define N_IDPSTAT	14
    { "idpstat"},
#define N_SPPSTAT	15
    { "spp_istat"},
#define N_NSERR		16
    { "ns_errstat"},
#define	ISO_TP		17
    { "tp_isopcb" },
#define N_CLTP		18
    { "cltb"},
#define N_CLTPSTAT	19
    { "cltpstat"},
#define	N_CLNPSTAT	20
    { "clnp_stat"},
#define	N_ESISSTAT	21
    { "esis_stat"},
#define	N_IMP		22
    { "imp_softc" },
#define N_NIMP		23
    { "nimp"},

    /*
     * The symbols up to this point come from /unix.  The symbols
     * after this point come from an unstripped version of netinet.
     * Note that after the nlist call for these symbols, we have to
     * add in the offset of the data for netinet
     */
#define N_NETINETSTART 24

#define	N_ICMPSTAT	24
    { "icmpstat" },
#define	N_IPSTAT	25
    { "ipstat" },
#define	N_TCB		26
    { "tcb" },
#define	N_TCPSTAT	27
    { "tcpstat" },
#define	N_UDB		28
    { "udb" },
#define	N_UDPSTAT	29
    { "udpstat" },
#define N_NDD           30
    { "ndd" },
#define N_IGMPSTAT	31
    { "igmpstat" },
#define N_IPINTRQ	32
    { "ipintrq" },
#define NELEM_NLIST	33
    "",
};

/* internet protocols */
extern	int protopr();
extern	int tcp_stats(), udp_stats(), ip_stats(), icmp_stats();
#ifdef IP_MULTICAST
extern  int igmp_stats();
#endif
/* ns protocols */
extern	int nsprotopr();
extern	int spp_stats(), idp_stats(), nserr_stats();
/* iso protocols */
extern	int iso_protopr();
extern	int tp_stats(), esis_stats(), clnp_stats(), cltp_stats();

#define NULLPROTOX	((struct protox *) 0)
struct protox {
	u_char	pr_index;		/* index into nlist of cb head */
	u_char	pr_sindex;		/* index into nlist of stat block */
	u_char	pr_wanted;		/* 1 if wanted, 0 otherwise */
	int	(*pr_cblocks)();	/* control blocks printing routine */
	int	(*pr_stats)();		/* statistics printing routine */
	char	*pr_name;		/* well-known name */
} protox[] = {
	{ N_TCB,	N_TCPSTAT,	1,	protopr,
	  tcp_stats,	"tcp" },
	{ N_UDB,	N_UDPSTAT,	1,	protopr,
	  udp_stats,	"udp" },
	{ -1,		N_IPSTAT,	1,	0,
	  ip_stats,	"ip" },
	{ -1,		N_ICMPSTAT,	1,	0,
	  icmp_stats,	"icmp" },
#ifdef IP_MULTICAST
	{ -1,		N_IGMPSTAT,	1,	0,
	  igmp_stats,	"igmp" },
#endif
	{ -1,		-1,		0,	0,
	  0,		0 }
};

struct protox nsprotox[] = {
	{ N_IDP,	N_IDPSTAT,	1,	nsprotopr,
	  idp_stats,	"idp" },
	{ N_IDP,	N_SPPSTAT,	1,	nsprotopr,
	  spp_stats,	"spp" },
	{ -1,		N_NSERR,	1,	0,
	  nserr_stats,	"ns_err" },
	{ -1,		-1,		0,	0,
	  0,		0 }
};

struct protox isoprotox[] = {
	{ ISO_TP,	N_TPSTAT,	1,	iso_protopr,
	  tp_stats,	"tp" },
	{ N_CLTP,	N_CLTPSTAT,	1,	iso_protopr,
	  cltp_stats,	"cltp" },
#ifdef notdef
	{ ISO_X25,	N_X25STAT,	1,	x25_protopr,
	  x25_stats,	"x25" },
#endif
	{ -1,		N_CLNPSTAT,	1,	 0,
	  clnp_stats,	"clnp"},
	{ -1,		N_ESISSTAT,	1,	 0,
	  esis_stats,	"esis"},
	{ -1,		-1,		0,	0,
	  0,		0 }
};

struct protox *protoprotox[] = { protox, nsprotox, isoprotox, NULLPROTOX };

struct	pte *Sysmap;

#include <locale.h>
#include <nl_types.h>
#include "netstat_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s) 

char	*system = _PATH_UNIX;
char	*kmemf = _PATH_KMEM;
char	*netinetf = "/etc/drivers/netinet";
long	net_offset;
int	kmem;
int	running = 1;
int	Aflag;
int	aflag;
int	Dflag;
int	iflag;
int	mflag;
int	nflag;
int	pflag;
int	rflag;
int	sflag;
int	tflag;
int	dflag;
int	vflag;
int	Zflag;
int	interval;
char	*interface;
int	unit;
char	*drvname = NULL;

int	af = AF_UNSPEC;

extern	char *malloc();
extern	off_t lseek();

main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	register struct protoent *p;
	register struct protox *tp;	/* for printing cblocks & stats */
	struct protox *name2protox();	/* for -p */
	int ch;


	setlocale(LC_ALL,"");
	catd = catopen(MF_NETSTAT,NL_CAT_LOCALE);
	while ((ch = getopt(argc, argv, "ADI:af:imnp:drstuvZ")) != EOF)
		switch((char)ch) {
		case 'A':
			Aflag++;
			break;
		case 'D':
			Dflag++;
			break;
		case 'I': {
			char *cp;

			iflag++;
			for (cp = interface = optarg; isalpha(*cp); cp++);
			unit = atoi(cp);
			*cp = '\0';
			break;
		}
		case 'a':
			aflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 'f':
			if (strcmp(optarg, "ns") == 0)
				af = AF_NS;
			else if (strcmp(optarg, "inet") == 0)
				af = AF_INET;
			else if (strcmp(optarg, "unix") == 0)
				af = AF_UNIX;
			else if (strcmp(optarg, "iso") == 0) {
				af = AF_ISO; 
				fprintf(stderr,MSGSTR(NOT_SUPPORT,"%s: Not Supported\n"),optarg);
				exit(1);
			}
			else {
				fprintf(stderr,MSGSTR(UNK_ADDR_FAM,"%s: unknown address family\n"), optarg);
				exit(10);
			}
			break;
		case 'i':
			iflag++;
			break;
		case 'm':
			mflag++;
			break;
		case 'n':
			nflag++;
			break;
		case 'p':
			if ((tp = name2protox(optarg)) == NULLPROTOX) {
				fprintf(stderr,MSGSTR(NO_PROTO,"%s: unknown or uninstrumented protocol\n"), optarg);
				exit(10);
			}
			pflag++;
			break;
		case 'r':
			rflag++;
			break;
		case 's':
			sflag++;
			break;
		case 't':
			tflag++;
			break;
		case 'u':
			af = AF_UNIX;
			break;
		case 'v':
			vflag++;
			if ((optind != argc) && (argv[optind] != '-'))
				drvname = argv[optind++];
			break;
		case 'Z':
			Zflag++;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;
	argc -= optind;

	if (argc > 0 && isdigit(argv[0][0])) {
		interval = atoi(argv[0]);
		if (interval <= 0)
			usage();
		argv++, argc--;
		iflag++;
	}
	if (argc > 0) {
		if (argc != 4)
			usage();
		kmemf = *argv++;
		system = *argv++;
		netinetf = *argv++;
		sscanf(&net_offset, *argv++, "%x");
		running = 0;
	}

	kmem = open(kmemf, O_RDWR);
	if (kmem < 0) {
		fprintf(stderr, MSGSTR(CANT_OPEN, "cannot open ")); /*MSG*/
		perror(kmemf);
		exit(1);
	}
	if (!running) {
		int i;

		nlist(system, nl);
		nlist(netinetf, nl + N_NETINETSTART);

		/* netinet vars are offset by net_offset */
		for (i = N_NETINETSTART; i <= NELEM_NLIST; ++i)
			if (nl[i].n_value)
				nl[i].n_value += net_offset;

		/* Convert TOC entries to real addresses */
		for (i = 0; i <= NELEM_NLIST; ++i)
			if (nl[i].n_value && nl[i].n_sclass == C_HIDEXT)
				(void) kvm_read(nl[i].n_value, &nl[i].n_value,
					 sizeof(&nl[i].n_value));
	} else
		knlist(nl, NELEM_NLIST, sizeof (struct nlist));

	if (mflag) {
	    if (nl[N_MBSTAT].n_value) {
		mbpr(nl[N_MBSTAT].n_value, nl[N_BUCKETS].n_value, 
			nl[N_KMEMSTATS].n_value, nl[N_SYSTEMCONFIG].n_value);
		exit(0);
	    } else {
		symbol_err(N_MBSTAT);
		exit (1);
            }
	}
	if (pflag) {
		if (tp->pr_stats) {
			(*tp->pr_stats)(nl[tp->pr_sindex].n_value,
				tp->pr_name);
			exit(0);
		} else {
			printf(MSGSTR(NO_STATS,
				 "%s: no stats routine\n"), tp->pr_name);
			exit(1);
		}
	}
	/*
	 * Keep file descriptors open to avoid overhead
	 * of open/close on each call to get* routines.
	 */
	sethostent(1);
	setnetent(1);
	if (Dflag) {
	    if (!nl[N_NDD].n_value) {
		symbol_err(N_NDD);
		exit(1);
	    }
	    if (!nl[N_IFNET].n_value) {
		symbol_err(N_IFNET);
		exit(1);
	    }
	    pkt_drop_stats(nl[N_IPSTAT].n_value, nl[N_TCPSTAT].n_value,
		nl[N_UDPSTAT].n_value, nl[N_IFNET].n_value, nl[N_NDD].n_value);
	    exit(0);
	}
	if (iflag) {
	    if (nl[N_IFNET].n_value) {
		intpr(interval, nl[N_IFNET].n_value);
		exit(0);
	    } else {
		symbol_err(N_IFNET);
		exit(1);
	    }
	}
	if (vflag) {
	    if (nl[N_IFNET].n_value) {
		drvstats(drvname);
		exit(0);
	    } else {
		symbol_err(N_IFNET);
		exit (1);
	    }
	}
	if (rflag) {
		if (sflag) {
		    if (nl[N_RTSTAT].n_value) {
			rt_stats((off_t)nl[N_RTSTAT].n_value);
		    } else {
			symbol_err(N_RTSTAT);
			exit(1);
		    }
		} else if (nl[N_RTREE].n_value) {
			    routepr((off_t)nl[N_RTREE].n_value);
	        } else {
			fprintf(stderr, MSGSTR(SYM_MISSING,
				"symbol missing: %s\n"), nl[N_RTREE].n_name);
			exit(1);
		}
		exit(0);
	}
    if (af == AF_INET || af == AF_UNSPEC) {
	setprotoent(1);
	setservent(1);
	while (p = getprotoent()) {
		for (tp = protox; tp->pr_name; tp++)
			if (strcmp(tp->pr_name, p->p_name) == 0)
				break;
		if (tp->pr_name == 0 || tp->pr_wanted == 0)
			continue;
		_afpr(tp);
	}
	endprotoent();
	printf("\n");
    }
    if (af == AF_NS || af == AF_UNSPEC)
	afpr(nsprotox);
    if (af == AF_ISO || af == AF_UNSPEC)
	afpr(isoprotox); 
    if ((af == AF_UNIX || af == AF_UNSPEC) && !sflag)
	    unixpr((off_t)nl[N_V].n_value, (off_t)nl[N_FILE].n_value,
		(struct protosw *)nl[N_UNIXSW].n_value);
#ifdef notdef
    if (af == AF_UNSPEC && sflag)
	impstats(nl[N_IMP].n_value, nl[N_NIMP].n_value);
#endif
    exit(0);
}

/* print routine wrapper for ns & iso */
afpr(px)
	struct protox *px;
{
	struct protox *tp;

	for (tp = px; tp->pr_name; tp++)
		_afpr(tp, sflag);
}

/* generic af print routine */
_afpr(tp)
	struct protox *tp;
{
	if (sflag) {
		if (tp->pr_stats)
		    if (nl[tp->pr_sindex].n_value) {
			(*tp->pr_stats)(nl[tp->pr_sindex].n_value,
				tp->pr_name);
		    } 
	} else {
	    if ((int)tp->pr_index >= 0 && tp->pr_index < NELEM_NLIST)
		if (nl[tp->pr_index].n_value) {
		    if (tp->pr_cblocks)
			(*tp->pr_cblocks)(nl[tp->pr_index].n_value,
				tp->pr_name);
		} 
	}
}

kvm_read(p, d, size)
	off_t 	p;
	char	*d;
	int	size;
{
	return read_memory(kmem, running, d, p, size);
}

char *
plural(n)
	int n;
{
	static char *s;

	if (!s)
		s = MSGSTR(PLURAL, "s");
	return (n != 1 ? s : "");
}

#ifdef IP_MULTICAST
char *
pluraly(n)
	int n;
{
	static char *s;

	if (!s)
		s = MSGSTR(PLURAL, "s");
	return (n != 1 ? "ies" : "y");
}
#endif

symbol_err(ind)
    int ind;
{
    if (ind >= 0 || ind < NELEM_NLIST)
	fprintf(stderr, MSGSTR(SYM_NOT_FOUND, "can't find symbol: %s\n"), 
			nl[ind].n_name);
}

/*
/*
 * Find the protox for the given "well-known" name.
 */
struct protox *
knownname(name)
	char *name;
{
	struct protox **tpp, *tp;

	for (tpp = protoprotox; *tpp; tpp++)
	    for (tp = *tpp; tp->pr_name; tp++)
		if (strcmp(tp->pr_name, name) == 0)
			return(tp);
	return(NULLPROTOX);
}

/*
 * Find the protox corresponding to name.
 */
struct protox *
name2protox(name)
	char *name;
{
	struct protox *tp;
	char **alias;			/* alias from p->aliases */
	struct protoent *p;

	/*
	 * Try to find the name in the list of "well-known" names. If that
	 * fails, check if name is an alias for an Internet protocol.
	 */
	if (tp = knownname(name))
		return(tp);

	setprotoent(1);			/* make protocol lookup cheaper */
	while (p = getprotoent()) {
		/* assert: name not same as p->name */
		for (alias = p->p_aliases; *alias; alias++)
			if (strcmp(name, *alias) == 0) {
				endprotoent();
				return(knownname(p->p_name));
			}
	}
	endprotoent();
	return(NULLPROTOX);
}

usage()
{
	fputs(MSGSTR(USAGE,
	"usage: netstat [-Aan] [-f address_family] [system]\n"), stderr);
	fputs(MSGSTR(USAGE1,
	"               [-D]\n"), stderr);
	fputs(MSGSTR(USAGE2,
	"               [-imnrsv] [-f address_family] [-p proto] [system]\n"),
								stderr);
	fputs(MSGSTR(USAGE3,
	"               [-n] [-I interface] [interval] [system]\n"), stderr);
	exit(1);
}
