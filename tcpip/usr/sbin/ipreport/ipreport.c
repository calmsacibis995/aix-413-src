static char sccsid[] = "@(#)37	1.29  src/tcpip/usr/sbin/ipreport/ipreport.c, tcpras, tcpip411, GOLD410 6/15/94 18:54:43";
/*
 * COMPONENT_NAME: TCPIP ipreport.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Modified to support RPC Sat Aug 10 1991 
 */

#include <rpc/rpc.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/x25user.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/devinfo.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>
#include <netinet/if_fddi.h>
#include <if_fcs.h>		/* XXX: not shipped! */
#include <if_op.h>		/* shipped version clashes */
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_var.h>
#include <netinet/tcpip.h>
#include <aixif/net_if.h>
#include "klm_prot.h"
#include "mount.h"
#define NFSSERVER 3
#include "nfs.h"
/*#include <rpcsvc/nfs_prot.h>*/
#include "nlm_prot.h"
#include "rex.h"
#include "rstat.h"
#include "rusers.h"
#include "rwall.h"
#include "sm_inter.h"
#include "spray.h"
#include <rpcsvc/yp_prot.h>
#include <pwd.h>
#include "yppasswd.h"
#include <rpcsvc/ypup_prot.h>
#include "ipr.h"
#include "sniffer.h"


#ifndef LOCAL
#include <nl_types.h>
#include "ipreport_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_IPREPORT,n,s) 
#else
#define MSGSTR(n,s) s
#endif

u_char	packet[MAXPACKET*2];
u_char	ipbuf[MAXPACKET*2];
u_char	tcpbuf[MAXPACKET*2];
int	aix_version=1;		/* default to aix3.2 */
int	ipt_maj_vers=0;
int	ipt_min_vers=0;
int	xflag=0;
int	Xbytes=-1;
int	jflag;
int	Debug;
long thiszone;			/* gmt to local correction */
int	packnum=0;
int	packdisp=0;
int	totaldrops=0;		/* total packets dropped */
int	Sflag;			/* Network General sniffer trace */
int	NGmtype;
int	Verbose;		/* verbosity */
int	Cflag;
int	Nflag;
int	Tflag;
int	trace_header_size;

int s;			/* Socket file descriptor */
struct hostent *hp;	/* Pointer to host info */

char *ie5_sprintf();
char *ether_sprintf();

#define IPT_LEN	8
char version[]	 = "iptrace 2.0";
char fversion[30];
char usage_msg[] = 
"Usage:\tipreport [-CenrsSvx1N] [-c count] [-j pktnum] [-X bytes] tracefile\n\
 -c <count>: display <count> number of packets\n\
 -C: validate checksums\n\
 -e: show ebcdic instead of ascii\n\
 -j <pktnum>: jump to packet number <pktnum>\n\
 -n: number packets\n\
 -N: dont do name resolution\n\
 -r: know about rpc\n\
 -s: start lines with protocol indicator strings\n\
 -S: input file was generated on a sniffer\n\
 -v: verbose\n\
 -x: print packet in hex\n\
 -X <bytes>: limit hex dumps to <bytes>\n\
 -1: compatibility: trace was generated on AIX3.1\n\
";

FILE	 	*fp;
char		*tracefile;

#ifndef LOCAL
#include <locale.h>
#endif

int like_snoop=0, know_rpc=0, packetnums=0;
char *arp_beg="ARP: ";
char *beg_line;
char *eth_beg="ETH: ";
char *ip_beg= "IP:  ";
char *null_beg= "";
char *slp_beg="SLP: ";
char *acl_beg="ACL: ";
char *tcp_beg="TCP: ";
char *tok_beg="TOK: ";
char *udp_beg="UDP: ";
char *x25_beg="X25: ";
char *lok_beg="NLM: ";
char *pm_beg= "PMP: ";
char *icmp_beg= "ICMP: ";
char *fddi_beg="FDDI: ";
char *sol_beg="SOL: ";
char *fcs_beg="FCS: ";
char *ipx_beg="IPX: ";
char *ncp_beg="NCP: ";

int ebc_dick=0;		/* should we print out ascii as ebcdic instead?
				- added for the SNA guys */

extern int IPTreader(), NGreader();
extern int IPTrinit(), NGrinit();

main(argc, argv)
char *argv[];
{
	extern int 		getopt();
	extern int 		optind;
	extern char		*optarg;
	int 			c, cnt = -1;
	struct timeval 		now;
	struct timezone 	tz;
	time_t			time;
	int			(*reader)();
	int			(*rinit)();

#ifndef LOCAL
	setlocale(LC_ALL,"");
	catd = catopen(MF_IPREPORT,NL_CAT_LOCALE);
#endif

	/* default reader is iptrace-format */
	reader = IPTreader;
	rinit = IPTrinit;
	while (( c = getopt(argc, argv, "NCesrn1xX:j:c:dSv")) != EOF) {
		switch(c) {
			case 'e' :
				ebc_dick=1;
				break;
			case 's' :
				like_snoop=1;
				break;
			case 'r' :
				know_rpc=1;
				break;
			case 'n' :
				packetnums=1;
				break;
			case '1':
				aix_version=0;
				break;
			case 'x':
				xflag++;	/* print only hex */
				break;
			case 'X':		/* limit hex dumps */
				Xbytes = atoi(optarg);
				break;
			case 'j':		/* jump-to packet # */
				if ((jflag = atoi(optarg)) < 0)
				    jflag = 0;
				break;
			case 'c':
				if ((cnt = atoi(optarg)) <= 0)
				    cnt = -1;
				break;
			case 'd':
				Debug++;
				break;
			case 'S':
				Sflag++;
				reader = NGreader;
				rinit = NGrinit;
				break;
			case 'v':
				Verbose++;
				break;
			case 'C':
				Cflag++;
				break;
			case 'N':
				Nflag++;
				break;
			default  :
				usage();
				
		}
	}

	if (argv[optind] == NULL)
		usage();

	/* open the tracefile */
	tracefile = argv[optind++];
	if ((fp = ((*tracefile == '-' && strlen (tracefile) == 1) ?
	    stdin : fopen(tracefile, "r"))) == (FILE *)NULL) {
		fprintf(stderr, MSGSTR( OPEN,"ipreport: Can't open %s.\n"),
			tracefile);
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-1);
	}
	if (fp != stdin) fseek(fp, (long)0, SEEK_SET);

	/* initialize reading from tracefile */
	(*rinit)();


	if (gettimeofday(&now, &tz) < 0) {
		perror("ipreport: gettimeofday");
		exit(1);
	}
	thiszone = tz.tz_minuteswest * -60;
	if (localtime((time_t *)&now.tv_sec)->tm_isdst)
		thiszone += 3600;

	for (;;) {
		int cc;

		if ( (cc = (*reader)(packet, &time)) < 0) {
			if (feof(fp)) 
				cleanup(0);
			fprintf(stderr,MSGSTR(CANTREAD, "ipreport: Error reading %s.\n"),
				tracefile);
			if (errno)
			    perror( MSGSTR( ERRORF, "\tError"));
			else
			    fprintf (stderr, "ipreport: reader error [%d]\n",
				    cc);
			cleanup(-1);
		}

		if (!cc)		 /* graceful end by reader */
		    cleanup (0);

		/* jump to packet # jflag */
		if (jflag && (packnum + 1) < jflag) {
		    packnum++;
		    continue;			/* not there yet */
		}

		/* display only cnt # of packets */
		if (cnt >= 0 && --cnt < 0)
		    cleanup(0);			/* stop */

		printit(packet, cc, &time);
	}
	/*NOTREACHED*/
}

cleanup(exitcode)
int exitcode;
{
    printf("\n\n++++++ END OF REPORT %s++++++\n\n", 
	    exitcode ? "(TRUNCATED) " : "");
    printf ("processed %d packets\n", packnum);
    if (packnum != packdisp)
	printf ("displayed %d packets\n", packdisp);
    if (totaldrops && Tflag) 
	printf ("TRACING DROPPED %d PACKET%s THIS TRACE\n", 
		totaldrops, totaldrops > 1 ? "S" : "");
    if (know_rpc) dumpcilist();
    exit (exitcode);
}

IPTrinit ()
{
	char *mins, *majs;

	if (fread(fversion, 1, strlen(version), fp) != strlen(version)) {
		fprintf(stderr,MSGSTR(CANTREAD, "ipreport: Error reading %s.\n"),
			tracefile);
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-2);
	}
	if (strncmp(version, fversion, IPT_LEN)) {
not_trace_file:;
		fprintf(stderr,
			MSGSTR(NOTTRACEF,"ipreport: %s is not a trace file.\n"),
			tracefile);
		exit(-5);
	}
	majs = fversion + IPT_LEN;
	if ((mins = strchr (majs, '.')) == (char *) NULL)
	    goto not_trace_file;
	*mins++ = '\0';
	ipt_maj_vers = atoi (majs);
	ipt_min_vers = atoi (mins);
	printf ("IPTRACE version: %d.%d\n", ipt_maj_vers, ipt_min_vers);

	if (ipt_maj_vers >= 2)
		Tflag = 1;
	else
		Tflag = 0;
}

int
IPTreader(packet, timep) 
u_char	*packet;
time_t	*timep;
{
	int	cc;

	if (fread(&cc, sizeof(cc), 1, fp) != 1)
		return(-1);
	if (fread(timep, sizeof(*timep), 1, fp) != 1)
		return(-2);
	if (fread(packet, sizeof(u_char), cc, fp) != cc)
		return(-3);

	return(cc);
}

int
NGrinit()
{
    char NGident[NG_IDENT_LEN];
    struct f_rec_struct fr;
    struct f_vers_struct fv;

    /* read initial text ident */
    if (fread(NGident, sizeof(NGident), 1, fp) != 1) {
	fprintf(stderr,MSGSTR(CANTREAD, "ipreport: Error reading %s.\n"),
		tracefile);
	perror( MSGSTR( ERRORF, "\tError"));
	exit(-2);
    }
    if (strncmp (NG_IDENT, NGident, strlen(NG_IDENT))) {
	printf ("Unsupported trace format: %12.12s\n", NGident);
	usage();
    }

    /* should have a rec_struct and a vers_struct */
    if (fread(&fr, sizeof(struct f_rec_struct), 1, fp) != 1) {
	fprintf(stderr,MSGSTR(CANTREAD, "ipreport: Error reading %s.\n"),
		tracefile);
	perror( MSGSTR( ERRORF, "\tError"));
	exit(-2);
    }
    SWAP_SHORT(fr.type);
    if (fr.type != REC_VERS) {
	fprintf (stderr, "ipreport: expected REC_VERS (%d), got %d\n",
			REC_VERS, fr.type);
	exit (-3);
    }
    if (fread(&fv, sizeof(struct f_vers_struct), 1, fp) != 1) {
	fprintf(stderr,MSGSTR(CANTREAD, "ipreport: Error reading %s.\n"),
		tracefile);
	perror( MSGSTR( ERRORF, "\tError"));
	exit(-2);
    }

    SWAP_SHORT(fv.maj_vers);
    SWAP_SHORT(fv.min_vers);

    printf ("Sniffer Version: %d.%02d\n", fv.maj_vers, fv.min_vers);
/*     printf ("Record types in this file: %x\n", fv.type); */
    printf ("Network type: %x ", fv.network);
    switch (fv.network) {
	case NG_ETHERNET:
	    printf ("(ethernet)\n"); 
	    NGmtype = ETHERNET_CSMACD;
	    break;
	case NG_TOKEN:
	    printf ("(token_ring)\n"); 
	    NGmtype = ISO88025_TOKRNG;
	    break;
	default:
	    printf ("(unknown)\n"); 
	    NGmtype = -1;
	    break;
    }
/*     printf ("Format version: %x\n", fv.format); */

    if (NGmtype < 0) {
	printf ("Unsupported network type\n");
	exit (-3);
    }
}

int
NGreader(packet, timep)
u_char	*packet;
time_t	*timep;
{
    struct f_rec_struct fr;
    struct f_frame2_struct ff;

again:;
    if (fread(&fr, sizeof(struct f_rec_struct), 1, fp) != 1)
	return(-1);
    SWAP_SHORT(fr.type);
    SWAP_SHORT(fr.length);
    if (fr.type == REC_EOF)
	return (0);			/* end gracefully */

    if (fr.type != REC_FRAME2) {
	char c;
	int i;

	/* eat unknown packet struct */
	for (i = 0; i < fr.length; i++)
	    if (fread(&c, sizeof(char), 1, fp) != 1)
		return(-3);
	goto again;
    }

    if (fread(&ff, sizeof(struct f_frame2_struct), 1, fp) != 1)
	return(-4);
    SWAP_SHORT(ff.time_low);
    SWAP_SHORT(ff.time_mid);
    SWAP_SHORT(ff.time_high);
    SWAP_SHORT(ff.size);
    SWAP_SHORT(ff.true_size);

    if (ff.true_size)
	printf ("\nSniffer truncated packet: was %d, read %d\n",
		ff.true_size, ff.size);
    *timep = (ff.time_mid << 16) + (ff.time_low * .838096);

    if (fread(packet, sizeof(u_char), ff.size, fp) != ff.size)
	return(-5);

    return (ff.size);
}

usage() {
        fprintf(stderr, MSGSTR(USAGE2, usage_msg));
	fprintf(stderr,"\n");
	exit(1);
}

reset_beg_line(str)
char *str;
{
	if (like_snoop)
		beg_line=str;
	else
		beg_line=null_beg;
}
		
setbeg_line(iftype, ifname)
u_char		iftype;
char		*ifname;
{
    if (!like_snoop) 	      
	beg_line=null_beg;
    else
	switch (iftype) {
	    case ETHERNET_CSMACD: beg_line=eth_beg; break;
	    case ISO88025_TOKRNG: beg_line=tok_beg; break;
	    case ISO88023_CSMACD: beg_line=eth_beg; break;
	    case RFC877_X25:					/* aix3.2 */
	    case DD_X25:          beg_line=x25_beg; break;
	    case PROPPTPSERIAL:   beg_line=slp_beg; break;
	    case FDDI:            beg_line=fddi_beg; break;
	    case IFT_FCS:	  beg_line=fcs_beg; break;
	    case 0:
		if (iftype == 0 && aix_version) {  /* XXX: 3.2 if_sl bug */
		    beg_line=slp_beg;
		    break;
		}

		/* fall thru */
	    default:
		if (!strncmp (ifname, "so", 2)) {  /* XXX: gak! */
		    beg_line=sol_beg;
		    break;
		}
		beg_line=null_beg;
		break;
	}
}

printit(packet, cc, timep)
u_char  *packet;
int     cc;
time_t  *timep;
{
        u_short                 type;
        u_char                  hlen;
        char                    ifname[IFNAMSIZ];
        u_char                  *origpack,unit;
        struct packet_trace_header      *pth;
        u_char                  xmithlen;

        origpack = packet;

	if (!Sflag) {

        pth = (struct packet_trace_header *)packet;
        type = pth->type;
        hlen = pth->hlen;
        bcopy(pth->ifname, ifname, IFNAMSIZ);
        unit = pth->unit;
	if (ipt_maj_vers >= 2)
		trace_header_size = sizeof(struct packet_trace_header);	
	else
		trace_header_size = sizeof(struct o_packet_trace_header);

	packet += trace_header_size;
        printf("\n");
	if (Tflag && pth->rdrops) {
	    totaldrops += pth->rdrops;
	    printf ("TRACING DROPPED %d PACKET%s after packet %d. TOTAL DROPPED THIS TRACE=%d\n\n", 
		    pth->rdrops, 
		    pth->rdrops > 1 ? "S" : "", 
		    packnum, totaldrops);
	}
	packnum++;	/* total packets processed */
	packdisp++;	/* total displayed */
	if (packetnums) printf("Packet Number %d\n", packnum);

	setbeg_line(pth->iftype, ifname);
	printf("%s",beg_line);

	printf("====( %d bytes %s on interface %s%d )====",
		cc - trace_header_size,
                pth->xmit_pkt ? "transmitted" : "received",
                ifname, unit);
	if (Tflag) {
	    int i = (pth->ts.tv_sec + thiszone) % 86400;

	    (void)printf(" %02d:%02d:%02d.%09d\n",
		    i / 3600, (i % 3600) / 60, 
		    i % 60, pth->ts.tv_nsec);
	}
	else
	    printf ("%s", ctime(timep));

	/* just dump it in hex */
	if (xflag) {
	    if (xflag > 1)	/* include iptrace header */
		hex_dump(origpack, cc);	
	    else		/* strip iptrace header */
		hex_dump(packet, cc - trace_header_size);
	    return;
	}
        xmithlen = media_header_dump(packet, pth->iftype, pth->xmit_pkt, &type, ifname);
        if (type == 0 && !pth->xmit_pkt)
                return;
        if ( (hlen == 0) && (xmithlen != 0) )
                hlen = xmithlen;
        packet += hlen;
        if (!pth->xmit_pkt && !aix_version)
                packet += sizeof(struct ifnet *);
	if (Debug)
	    printf ("type=%d pth->hlen=%d xmithlen=%d, packet - origpack=%d\n",
		    pth-> iftype, pth -> hlen, xmithlen, packet - origpack);

	}
	else {
	    u_short tlow, tmid;

	    packnum++;	/* total packets processed */
	    packdisp++;	/* total displayed */
	    printf ("\n");
	    if (packetnums) printf("Packet Number %d\n", packnum);

	    setbeg_line(NGmtype, "");
	    printf("%s",beg_line);

#if 0	/* not correct.... */
	    tlow = *timep & 0xffff;
	    tmid = *timep >> 16;
	    printf("====( %d bytes Timestamp: %d.%06d )====\n", cc, tmid, tlow);
#else
	    printf("====( %d bytes )====\n", cc);
#endif

	    /* just dump it in hex */
	    if (xflag) {
		hex_dump(origpack, cc);	
		return;
	    }

	    /* fudge it */
	    xmithlen = media_header_dump(packet, NGmtype, 1, &type, "");
	    packet += xmithlen;
	}

#ifndef ETHERTYPE_NS
#define ETHERTYPE_NS	0x0600
#endif
#ifndef ETHERTYPE_NETWARE
#define ETHERTYPE_NETWARE	0x8137
#endif

	if (type>=ETHERTYPE_TRAIL && type<=(ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER))
	{
                printf(" (TRAIL)\n");
		hex_dump(origpack, cc);
	}
	else
        switch (type) {
                case ETHERTYPE_IP  :    printf(" (IP)\n");
                                        dump_ip(packet, 
						cc - (packet - origpack), 0);
                                        break;
		case ETHERTYPE_REVARP:	printf(" (REVARP)\n");
					dump_arp(packet);
					break;
                case ETHERTYPE_ARP :    printf(" (ARP)\n");
                                        dump_arp(packet);
                                        break;
                case ETHERTYPE_PUP :    printf(" (PUP)\n");
					goto hex_it;
		case ETHERTYPE_NS:	printf(" (XNS)\n");
					goto hex_it;
		case ETHERTYPE_NETWARE: printf(" (NETWARE)\n");
					dump_ipx (packet,
						cc - (packet - origpack));
					break;
/* 					goto hex_it; */
		case 65000: /* XXX */   hex_dump(origpack, cc);
                                        break;
                default            :    printf(" (unknown: %x)\n", type);
hex_it:;
					hex_dump(packet, 
						cc - (packet - origpack));
                                        break;
        }

}

struct hostent	*
mygethostbyaddr( addr, size, type )
char	*addr;
int	size, type;
{
	struct hostcache {
		struct hostcache *hc_next;
		char 		*hc_addr;
		struct hostent 	*hc_hostent;
	};
	static struct hostcache *hclist=0;
	struct hostcache *hcp,*newhc;
	struct hostent 	 *hp;

        /* ignore name resolution */
        if (Nflag)
            return ((struct hostent *) NULL);

	/* search cache for addr */
	for (hcp=hclist; hcp; hcp=hcp->hc_next)
		if (bcmp(hcp->hc_addr,addr,size)==0)
			return(hcp->hc_hostent);

	/* if it's not there resolve it */
	hp=gethostbyaddr(addr,size,type);

	/* and add it to the cache */
	newhc= (struct hostcache *) mymalloc(sizeof(struct hostcache));
	newhc->hc_addr = (char *) mymalloc(size);
	if (hp) {
		/* only the name is added to the cache w/ the hostent struct
		   cause that's all the caller wants, but later this 
	           could b changed 2 include the whole struct
	     	*/
		newhc->hc_hostent= 
			(struct hostent *) mymalloc(sizeof(struct hostent));
		bcopy(hp,newhc->hc_hostent,sizeof(struct hostent));
		newhc->hc_hostent->h_name=scopy(hp->h_name);
	}
	else
		newhc->hc_hostent= 0;

	newhc->hc_next=hclist;
	hclist=newhc;
	bcopy(addr,newhc->hc_addr,size);
	
	return(hp);
}

struct servent *
mygetservbyport (port, proto)
	int port;
	char *proto;
{
	struct portcache {
		struct portcache *pc_next;
		int 		  pc_port;
		char 		 *pc_proto;
		struct servent 	 *pc_servent;
	};
	static struct    portcache 	*pclist=0; 
	struct portcache 		*pcp,     *newpc; 
	struct servent   		*sp; 

	/* search cache for addr */
	for (pcp=pclist; pcp; pcp=pcp->pc_next)
		if ((pcp->pc_port==port) && (strcmp(pcp->pc_proto,proto)==0))
			return(pcp->pc_servent);

	/* if it's not there resolve it */
	sp=getservbyport(port,proto);

	/* and add it to the cache */
	newpc= (struct portcache *) mymalloc(sizeof(struct portcache));
	if (sp) {
		/* only the name is added to the cache w/ the servent struct
		   cause that's all the caller wants, but later this 
	           could b changed 2 include the whole struct
	     	*/
		newpc->pc_servent= 
			(struct servent *) mymalloc(sizeof(struct servent));
		bcopy(sp,newpc->pc_servent,sizeof(struct servent));
		newpc->pc_servent->s_name  = scopy(sp->s_name); 
		newpc->pc_servent->s_proto = scopy(sp->s_proto); 
	}
	else
		newpc->pc_servent= 0;
	newpc->pc_proto=scopy(proto);

	newpc->pc_next=pclist;
	newpc->pc_port=port;
	pclist=newpc;
	
	return(sp);
}


dump_ip(ipp, cc, nodata)
struct	ip		*ipp;
int			cc;
int			nodata;
{
	u_short		osum;
	int		isfrag;
	int		istrunc;
	int		hlen, iplen;
	struct hostent	*hp;
	char		*user_data;
	int		user_len;

	reset_beg_line(ip_beg);

	if (((int)ipp & 0x03) != 0) {	/* alignment problem */
		bcopy(ipp, ipbuf, cc);
		ipp = (struct ip *)ipbuf;
	}

	/* calculate len's, checksum, and fix byte order's like the kernel */
	hlen = ipp->ip_hl << 2;
	osum = ipp->ip_sum;
	ipp->ip_sum = (Cflag && osum) ? in_cksum (ipp, hlen) : 0;
	ipp->ip_id = ntohs(ipp->ip_id);
	ipp->ip_off = ntohs((u_short)ipp->ip_off);
	ipp->ip_len = ntohs((u_short)ipp->ip_len);
	iplen = ipp->ip_len;
	ipp->ip_len -= hlen;		/* XXX! */

	if (!like_snoop) printf("IP header breakdown:\n");
	printf("%s",beg_line);
	printf("\t< SRC = %15.15s >  ", inet_ntoa(ipp->ip_src.s_addr));
	if((hp = mygethostbyaddr( &ipp->ip_src.s_addr, sizeof(ipp->ip_src.s_addr),
	     AF_INET )) != (struct hostent *)0 )
		printf("(%s)", hp->h_name);
	printf("\n");
	printf("%s",beg_line);
	printf("\t< DST = %15.15s >  ", inet_ntoa(ipp->ip_dst.s_addr));
	if((hp = mygethostbyaddr( &ipp->ip_dst.s_addr, sizeof(ipp->ip_dst.s_addr),
	     AF_INET )) != (struct hostent *)0 )
		printf("(%s)", hp->h_name);
	printf("\n");
	printf("%s",beg_line);
	printf("\tip_v=%d, ip_hl=%d, ip_tos=%d",
		ipp->ip_v, hlen, ipp->ip_tos);
	printf(", ip_len=%d, ip_id=%d, ip_off=%d",
		iplen, ipp->ip_id, 
		isfrag=((ipp->ip_off & ~(IP_DF | IP_MF))<<3));
	printf("%s", (ipp->ip_off & IP_DF) ? "DF" : 
		((ipp->ip_off & IP_MF) ? "+" : ""));
	printf("\n");
	printf("%s",beg_line);
	printf("\tip_ttl=%d, ip_sum=%x%s",
		ipp->ip_ttl, osum,
		Cflag ? (ipp->ip_sum ? " (INVALID)" : " (valid)") : "");
	printf(", ip_p = %d ", ipp->ip_p);

	istrunc = (cc < iplen) ? iplen - cc : 0;
	user_len = iplen;
	user_data = (char *) ((char *) ipp + hlen);
	user_len -= hlen;

	switch (ipp->ip_p) {
		case	IPPROTO_IP:
				printf("(dummy IP)\n"); break;
		case	IPPROTO_ICMP:
				printf("(ICMP)"); 
				if (isfrag || istrunc) {
					user_len = 0;	/* we dont care! */
					break;
				}
				dump_icmp((struct icmp *) user_data, user_len);
				user_len = 0;	/* dump_icmp does it all */
				break;
		case	IPPROTO_GGP:
				printf("(GGP)\n"); break;
		case	IPPROTO_TCP:
				printf("(TCP)");
				if (isfrag || istrunc) break;
				dump_tcp((struct tcphdr *) user_data,
					user_len, ipp);
				user_len = 0;
				break;
		case	IPPROTO_EGP:
				printf("(EGP)\n"); break;
		case	IPPROTO_PUP:
				printf("(PUP)\n"); break;
		case	IPPROTO_UDP:
				printf("(UDP)");
				if (isfrag || istrunc) break;
				dump_udp((struct udphdr *) user_data, 
					user_len, ipp);
				user_len = 0;
				break;
		case	IPPROTO_IDP:
				printf("(IDP)\n"); break;
		default	  :	printf("(unknown internet protocol)"); break;
	}
	if (isfrag || istrunc) printf("\n");
	if (istrunc && !nodata)
	    printf("%s\ttruncated-ip, %d bytes missing\n",beg_line, istrunc);

	hex_dump(user_data, user_len - istrunc);
}

dump_tcp(th, user_len, ipp)
struct	tcphdr		*th;
int			user_len;
struct	ip		*ipp;
{
	struct	servent		*sp;
	int			fudge=0;
	u_short			sum=0;
	u_char 			*user_data = (u_char *) (th + 1);
	char *or = "", *ors = " | ";

	user_len -= sizeof(struct tcphdr);
	reset_beg_line(tcp_beg);

	if (((int)th & 0x03) != 0) {	/* alignment problem */
		printf ("COPIED TCP\n");
		bcopy(th, tcpbuf, sizeof(struct tcphdr));
		th = (struct tcphdr *)tcpbuf;
	}

	printf("\n");
	if (!like_snoop) 
		printf("TCP header breakdown:\n");
	printf("%s",beg_line);
	printf("\t<source port=%d", th->th_sport);
	if (sp = mygetservbyport(th->th_sport, "tcp"))
		printf("(%s), ", sp->s_name);
	else
		printf(", ");
	printf("destination port=%d", th->th_dport);
	if (sp = mygetservbyport(th->th_dport, "tcp"))
		printf("(%s) >\n", sp->s_name);
	else
		printf(" >\n");
	printf("%s",beg_line);
	printf("\tth_seq=%x, th_ack=%x", th->th_seq, th->th_ack);
	printf("\n");
	printf("%s",beg_line);
	printf("\tth_off=%d, flags<", th->th_off);
	if (th->th_flags & TH_FIN)  { printf("FIN"); or = ors; }
	if (th->th_flags & TH_SYN)  { printf("%sSYN", or); or = ors; }
	if (th->th_flags & TH_RST)  { printf("%sRST", or); or = ors; }
	if (th->th_flags & TH_PUSH) { printf("%sPUSH", or); or = ors; }
	if (th->th_flags & TH_ACK)  { printf("%sACK", or); or = ors; }
	if (th->th_flags & TH_URG)    printf("%sURG", or);
	printf(">\n");
	printf("%s",beg_line);

	/* calulate the checksum like the kernel (see netinet/tcp_input.c) */
	if (Cflag) {
	    register struct tcpiphdr *ti = (struct tcpiphdr *) ipp;
	    int tlen, len;

	    tlen = ((struct ip *)ti)->ip_len;
	    len = sizeof (struct ip) + tlen;

	    ti->ti_next = ti->ti_prev = 0;
	    ti->ti_x1 = 0;
	    ti->ti_len = (u_short)tlen;
	    ti->ti_len = htons(ti->ti_len);
	    sum = in_cksum (ipp, len);
	}
	printf("\tth_win=%d, th_sum=%x%s, th_urp=%x\n", th->th_win,
		th->th_sum, 
		Cflag ? (sum ? " (INVALID)" : " (valid)") : "", th->th_urp);

	if (know_rpc) fudge=4; /* skip weird 4 bytes */

 	if (know_rpc && isrpc(user_data+fudge))
	    rpc_dump(user_data+fudge, user_len-fudge,
		    ipp -> ip_src.s_addr, ipp -> ip_dst.s_addr);
	else
	    hex_dump(user_data, user_len);
}

/* XXX probably should use getservbyname() and cache answers */
#define TFTP_PORT 69            /*XXX*/
#define SUNRPC_PORT 111         /*XXX*/
#define SNMP_PORT 161           /*XXX*/
#define NTP_PORT 123            /*XXX*/
#define SNMPTRAP_PORT 162       /*XXX*/
#define RIP_PORT 520            /*XXX*/

dump_udp(uh, user_len, ipp)
struct	udphdr		*uh;
int 			user_len;
struct	ip		*ipp;
{
	struct	servent		*sp;
	u_char *user_data = (u_char *) (uh + 1);

	user_len -= sizeof(struct udphdr);
	reset_beg_line(udp_beg);

	printf("\n");
	if (!like_snoop) 
		printf("UDP header breakdown:\n");
	printf("%s",beg_line);
	printf("\t<source port=%d", uh->uh_sport);
	if (sp = mygetservbyport(uh->uh_sport, "udp"))
		printf("(%s), ", sp->s_name);
	else
		printf(", ");
	printf("<destination port=%d", uh->uh_dport);
	if (sp = mygetservbyport(uh->uh_dport, "udp"))
		printf("(%s)", sp->s_name);
	printf(" >\n");
	printf("%s",beg_line);
	printf("\t[ udp length = %d | udp checksum = %x ]",
		uh->uh_ulen, uh->uh_sum);
	printf("\n");

#define ISPORT(p) (uh->uh_dport == (p) || uh->uh_sport == (p))
	if (ISPORT(NAMESERVER_PORT))
	    dns_dump (user_data, user_len);
	else if (ISPORT(RIP_PORT))
	    rip_dump (user_data, user_len);
 	else if (know_rpc && isrpc(user_data))
	    rpc_dump(user_data, user_len, 
		    ipp -> ip_src.s_addr, ipp -> ip_dst.s_addr);
	else
	    hex_dump(user_data, user_len);
}

/* ONLY echo/echo_replies have user data broken out.
 * still need to examine other icmp types and do proper
 * breakdown.
 */
dump_icmp(icmp, user_len)
struct	icmp		*icmp;
int			user_len;
{
	char *user_data;
	int hlen = user_len;	/* for now... */
	int cc;

	cc =
	    sizeof (icmp->icmp_type) +
	    sizeof (icmp->icmp_code) +
	    sizeof (icmp->icmp_cksum) +
	    sizeof (icmp->icmp_hun);

	reset_beg_line(icmp_beg);

	printf("\n");
	if (!like_snoop) 
		printf("ICMP header breakdown:\n");

	printf("%s",beg_line);
	printf("\ticmp_type=%d ", icmp->icmp_type);
	switch (icmp->icmp_type) {
	    case ICMP_ECHOREPLY:
	    case ICMP_ECHO:
		printf("%s", icmp->icmp_type == ICMP_ECHO ? 
			"(ECHO_REQUEST)" : "(ECHO_REPLY)");
		printf("  icmp_id=%d  icmp_seq=%d",
			icmp->icmp_id, icmp->icmp_seq);

		/* only part of the header used for echo's */
#if 0
		/* not worth it to print the data! */
		hlen = 
		    sizeof (icmp->icmp_type) +
		    sizeof (icmp->icmp_code) +
		    sizeof (icmp->icmp_cksum) +
		    sizeof (icmp->icmp_hun);
#endif

		/* todo... breakout timing info that ping inserts */
		break;
	    case ICMP_UNREACH:
		printf("(DEST UNREACH)\n");
		printf("%s",beg_line);
		printf("\ticmp_code=%d ", icmp->icmp_code);
		switch (icmp->icmp_code) {
		    case ICMP_UNREACH_NET:
			printf("(NET %s unreachable)",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			break;
		    case ICMP_UNREACH_HOST:
			printf("(HOST %s unreachable)",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			break;
		    case ICMP_UNREACH_PROTOCOL:
			printf("(%s: PROTOCOL %d unreachable)",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr),
				icmp->icmp_ip.ip_p);
			break;
		    case ICMP_UNREACH_PORT:
		    {
			register struct ip *oip;
			register struct udphdr *ouh;
			register int ohlen;
			char buf[80];
			struct	servent	*sp;

			oip = &icmp->icmp_ip;
			ohlen = oip->ip_hl * 4;
			ouh = (struct udphdr *)(((u_char *)oip) + ohlen);
			ntohs(ouh->uh_dport);
			switch (oip->ip_p) {
			    case IPPROTO_TCP:
#if 0
				/* not worth it to lookup the port */
				if (sp = mygetservbyport(ouh->uh_dport, "tcp"))
				    sprintf (buf, "%d (%s)", ouh->uh_dport,
				    sp -> s_name);
				else
#endif
				    sprintf (buf, "%d", ouh->uh_dport);

				printf("(%s: TCP PORT %s unreachable, src=%d)",
				    inet_ntoa(oip->ip_dst.s_addr),buf,
				    ouh->uh_sport);
				    break;
			    case IPPROTO_UDP:
#if 0
				/* not worth it to lookup the port */
				if (sp = mygetservbyport(ouh->uh_dport, "tcp"))
				    sprintf (buf, "%d (%s)", ouh->uh_dport,
				    sp -> s_name);
				else
#endif
				    sprintf (buf, "%d", ouh->uh_dport);

				printf("(%s: UDP PORT %s unreachable, src=%d)",
				    inet_ntoa(oip->ip_dst.s_addr),buf,
				    ouh->uh_sport);
				    break;
			    default:
				printf("(%s: PROTOCOL %d PORT %d unreachable)",
				    inet_ntoa(oip->ip_dst.s_addr),
				    oip->ip_p, ouh->uh_dport);
				break;
			}
			break;
		    }
		    case ICMP_UNREACH_NEEDFRAG:
			printf("(%s unreachable: need to frag)",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			break;
		    case ICMP_UNREACH_SRCFAIL:
			printf("(%s unreachable: source route failed)",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			break;
		}
		if (Verbose) {
		    if (like_snoop)
			printf("\n%s\tReferenced IP header:\n", beg_line);
		    else
			printf("\nReferenced ");
		    dump_ip (&icmp->icmp_ip, user_len - cc, 1);
		}
		break;
	    case ICMP_SOURCEQUENCH:
		printf("(SOURCEQUENCH)");
		break;
	    case ICMP_REDIRECT:
/* 		printf("(REDIRECT)  icmp_code=%d",icmp->icmp_code); */
		printf("(REDIRECT)\n");
		printf("%s",beg_line);
		printf("\ticmp_code=%d ", icmp->icmp_code);
		switch (icmp->icmp_code) {
		    case ICMP_REDIRECT_NET:
			printf("(redirect %s ",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			printf("to NET %s)",
				inet_ntoa(icmp->icmp_gwaddr.s_addr));
			break;
		    case ICMP_REDIRECT_HOST:
			printf("(redirect %s ",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			printf("to HOST %s)",
				inet_ntoa(icmp->icmp_gwaddr.s_addr));
			break;
		    case ICMP_REDIRECT_TOSNET:
			printf("(redirect-TOS %s ",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			printf("to NET %s)",
				inet_ntoa(icmp->icmp_gwaddr.s_addr));
			break;
		    case ICMP_REDIRECT_TOSHOST:
			printf("(redirect-TOS %s ",
				inet_ntoa(icmp->icmp_ip.ip_dst.s_addr));
			printf("to HOST %s)",
				inet_ntoa(icmp->icmp_gwaddr.s_addr));
			break;
		}
		if (Verbose) {
		    if (like_snoop)
			printf("\n%s\tReferenced IP header:\n", beg_line);
		    else
			printf("\nReferenced ");
		    dump_ip (&icmp->icmp_ip, user_len - cc, 1);
		}
		break;
	    case ICMP_TIMXCEED:
		printf("(TIME_EXCEEDED)  icmp_code=%d",icmp->icmp_code);
		switch (icmp->icmp_code) {
		    case ICMP_TIMXCEED_INTRANS:
			printf("(IN_TRANSIT)");
			break;
		    case ICMP_TIMXCEED_REASS:
			printf("(IN_REASS)");
			break;
		}
		if (like_snoop)
		    printf("\n%s\tReferenced IP header:\n", beg_line);
		else
		    printf("\nReferenced ");
		dump_ip (&icmp->icmp_ip, user_len - cc, 1);
		break;
	    case ICMP_PARAMPROB:
		printf("(PARAMPROB)");
		break;
	    case ICMP_TSTAMP:
		printf("(TSTAMP)");
		break;
	    case ICMP_TSTAMPREPLY:
		printf("(TSTAMP_REPLY)");
		break;
	    case ICMP_IREQ:
		printf("(IREQ)");
		break;
	    case ICMP_IREQREPLY:
		printf("(IREQ_REPLY)");
		break;
	    case ICMP_MASKREQ:
		printf("(MASKREQ)");
		break;
	    case ICMP_MASKREPLY:
		printf("(MASK_REPLY)");
		break;
	    default	  :	printf("(unknown ICMP packet)"); break;
	}
	printf("\n");

	user_data = (char *) ((char *) icmp + hlen);
	user_len -= hlen;

	hex_dump(user_data, user_len);
}


dump_arp(ar)
struct arphdr			*ar;
{

	reset_beg_line(arp_beg);
	printf("%s",beg_line);
	printf("hardware address format = %x", ar->ar_hrd);
	switch (ar->ar_hrd) {
		case ARPHRD_ETHER:
			printf(" (ethernet)\n");
			break;
		case ARPHRD_802_3:
			printf(" (802.3, 802.5)\n");
			break;
/*		case ARPHRD_FCS:
 *			printf(" (fcs)\n");
 *			break;
 */
		default:
			printf("\n");
	}
	
	printf("%s",beg_line);
	printf("protocol address format = %x", ar->ar_pro);
	switch (ar->ar_pro) {
		case ETHERTYPE_IP:
			printf(" (IP)\n");
			break;
		case ETHERTYPE_TRAIL:
			printf(" (IP trailers)\n");
			break;
		default:
			printf("\n");
	}
	printf("%s",beg_line);
	printf("address lengths ; hardware = %x, protocol = %x\n", 
		ar->ar_hln, ar->ar_pln);
	printf("%s",beg_line);
	printf("arp operation = %x", ar->ar_op);
	switch (ar->ar_op) {
		case ARPOP_REQUEST:
			printf(" (request)\n");
			break;
		case ARPOP_REPLY:
			printf(" (reply)\n");
			break;
		default:
			printf("\n");
			break;
	}
	switch (ar->ar_hrd) {
		case ARPHRD_ETHER:
			dump_ether_arp(ar);
			break;
		case ARPHRD_802_3:
			dump_802_arp(ar);
			break;
/*		case ARPHRD_FCS:
 *			dump_fcs_arp(ar);
 *			break;
 */
		
		default:
			hex_dump((char *)(ar) + sizeof(struct arphdr),
				(ar->ar_hln + ar->ar_pln)*2);
			break;
	}
	
}

dump_fcs_arp(fa)
struct	fcs_arp	*fa;
{
	struct hostent	*hp;
	int 		i;
	char		*ha;
	char		*arpsha = (char *)(fa) + FCS_ARP_HDR_SIZE;
	char		*arpspa = (char *)(arpsha) + fa->arphln;
	char		*arptha = (char *)(arpspa) + FCS_ARP_PROTO_SIZE;
	char		*arptpa = (char *)(arptha) + fa->arphln;

	printf("%s",beg_line);
	printf("source addresses:\n");
	ha = (arpsha);
	for (i=(fa->arphln); i>0; i-=FCS_ARP_TUPLE_SIZE) {
		printf("%s\t\t",beg_line);
		fcs_addr_print(ha);
		ha += FCS_ARP_TUPLE_SIZE;
	}
	printf("%s",beg_line);
	printf("\t\t protocol [%s]",
		inet_ntoa(*(struct in_addr *)arpspa));
	if((hp = mygethostbyaddr( arpspa, FCS_ARP_PROTO_SIZE,
	     AF_INET )) != (struct hostent *)0 )
		printf(" (%s)", hp->h_name);
	printf("\n");

	printf("%s",beg_line);
	printf("target addresses:\n");
	ha = (arptha);
	for (i=(fa->arphln); i>0; i-=FCS_ARP_TUPLE_SIZE) {
		printf("%s\t\t",beg_line);
		fcs_addr_print(ha);
		ha += FCS_ARP_TUPLE_SIZE;
	}
	printf("%s",beg_line);
	printf("\t\t protocol [%s]",
		inet_ntoa(*(struct in_addr *)arptpa));
	if((hp = mygethostbyaddr( arptpa, FCS_ARP_PROTO_SIZE,
	     AF_INET )) != (struct hostent *)0 )
		printf(" (%s)", hp->h_name);
	printf("\n");
}

dump_ether_arp(ea)
struct	ether_arp	*ea;
{
	struct hostent	*hp;

	printf("%s",beg_line);
	printf("source addresses: hw [%s]\n",
		ether_sprintf(ea->arp_sha));
	printf("%s",beg_line);
	printf("\t\t protocol [%s]",
		inet_ntoa(*(struct in_addr *)ea->arp_spa));
	if((hp = mygethostbyaddr( ea->arp_spa, sizeof(ea->arp_spa),
	     AF_INET )) != (struct hostent *)0 )
		printf(" (%s)", hp->h_name);
	printf("\n");

	printf("%s",beg_line);
	printf("target addresses: hw [%s]\n",
		ether_sprintf(ea->arp_tha));
	printf("%s",beg_line);
	printf("\t\t protocol [%s]",
		inet_ntoa(*(struct in_addr *)ea->arp_tpa));
	if((hp = mygethostbyaddr( ea->arp_tpa, sizeof(ea->arp_tpa),
	     AF_INET )) != (struct hostent *)0 )
		printf(" (%s)", hp->h_name);
	printf("\n");
}

dump_802_arp(ta)
struct	ie5_arp	*ta;
{
	struct hostent	*hp;

	printf("%s",beg_line);
	printf("source addresses: hw [%s]\n",
		ie5_sprintf(ta->arp_sha));
	printf("%s",beg_line);
	printf("\t\t protocol [%s]",
		inet_ntoa(*(struct in_addr *)ta->arp_spa));
	if((hp = mygethostbyaddr( ta->arp_spa, sizeof(ta->arp_spa),
	     AF_INET )) != (struct hostent *)0 )
		printf(" (%s)", hp->h_name);
	printf("\n");

	printf("%s",beg_line);
	printf("target addresses: hw [%s]\n",
		ie5_sprintf(ta->arp_tha));
	printf("%s",beg_line);
	printf("\t\t protocol [%s]",
		inet_ntoa(*(struct in_addr *)ta->arp_tpa));
	if((hp = mygethostbyaddr( ta->arp_tpa, sizeof(ta->arp_tpa),
	     AF_INET )) != (struct hostent *)0 )
		printf(" (%s)", hp->h_name);
	printf("\n");
}

media_header_dump(hp, iftype, xmit_flag, type, ifname)
char		*hp;
u_char		iftype;
u_char		xmit_flag;
u_short		*type;
char		*ifname;
{
	struct ether_header		*eh;
	struct fddi_mac_hdr		*macf;
	struct fcs_mac_hdr		*macfcs;
	struct ie5_mac_hdr		*mac;
	struct ie3_mac_hdr		*mac3;
	struct ie2_llc_snaphdr		*llc;
	struct op_hdr			*op;
	u_char				xmit_hlen;

	xmit_hlen = 0;
	switch (iftype) {
	    case ETHERNET_CSMACD:
		eh = (struct ether_header *)hp;
		if (like_snoop) 
			printf("%s   ",beg_line);
		else
			printf("ETHERNET packet : ");
		printf("[ %s -> ", 
			ether_sprintf(eh->ether_shost)); 
		printf("%s ]  type %x ",
			ether_sprintf(eh->ether_dhost), 
			eh->ether_type); 
		xmit_hlen = sizeof(struct ether_header);
		*type = eh->ether_type;
		break;

	    case ISO88025_TOKRNG:
		reset_beg_line(tok_beg);
		printf("%s",beg_line);
		printf("802.5 packet\n");
		mac = (struct ie5_mac_hdr *)hp;
		if (ipt_maj_vers >= 2) 
			llc = (struct ie2_llc_snaphdr *)
				((caddr_t)mac + mac_size(mac));
		else {
			if (xmit_flag)
				llc = snd_mac_to_llc(mac);
			else
				llc = rcv_mac_to_llc(mac);
		}
		mac_dmp(mac);
		/* we handle sniffer data now, be careful... */
		if (llc -> dsap == 0xaa) {
		    llc_dmp(llc);
		    *type = llc->type;
		    xmit_hlen = mac_size(mac) + sizeof(*llc);
		}
		else {
		    /* assume no llc */
		    *type = 65000;			/* XXX */
		    xmit_hlen = mac_size(mac);
		}
		break;

	    case ISO88023_CSMACD:
		printf("%s",beg_line);
		printf("802.3 packet\n");
		mac3 = (struct ie3_mac_hdr *)hp;
		llc =  (struct ie2_llc_snaphdr *)((mac3) + 1);
		ie3_mac_dmp(mac3);
		llc_dmp(llc);
		*type = llc->type;
		xmit_hlen = sizeof(*mac3) + sizeof(*llc);
		break;

	    case RFC877_X25:		/* aix3.2 */
	    case DD_X25:
		printf("%s",beg_line);
		printf("X.25 packet  ");
		xmit_hlen = offsetof(struct x25_buffer, body.user_data[0]);
		*type = ETHERTYPE_IP;
		break;

	    case FDDI:
		printf("%s",beg_line);
		printf("FDDI packet  ");
		macf = (struct fddi_mac_hdr *)hp;
		llc = (struct ie2_llc_snaphdr *)
			((caddr_t)macf + mac_size_f(macf));
		fddi_mac_dmp(macf);
		llc_dmp(llc);
		*type = llc->type;
		xmit_hlen = mac_size(macf) + sizeof(*llc);
		break;

	    case IFT_FCS:
		printf("%s",beg_line);
		printf("FCS packet  ");
		macfcs = (struct fcs_mac_hdr *)hp;
		llc = ((char *)macfcs) + FCS_MAC_SIZE + macfcs->opt_hdr_len;
		if (macfcs->entity_num == FCS_IP_ENTITY_TYPE) {
			fcs_mac_dmp(macfcs);
			llc_dmp(llc);
			*type = llc->type;
			xmit_hlen = (FCS_MAC_SIZE + FCS_LLC_SIZE + 
				     (macfcs->opt_hdr_len));
		}
		else {
			printf("Unknown Entity number [%x]\n",macfcs->entity_num);
			hex_dump(macfcs, FCS_MAC_SIZE + FCS_LLC_SIZE + 
				macfcs->opt_hdr_len);
		}
		break;

	    case PROPPTPSERIAL:		/* 3.1 */
	    case 0:	 		/* XXX: 3.2 if_sl bug */
		if (iftype || aix_version) {
		    printf("%s",beg_line);
		    printf("SLIP packet  ");
		    xmit_hlen = 0;
		    *type = ETHERTYPE_IP;
		    break;
		}
		/* FALL-THRU */

	    case OTHER_TYPE:
	    default:

		printf("%s",beg_line);

		/* special cases with non-unique iftype's */
		if (!strncmp (ifname, "so", 2)) {
		    printf("SOL packet  ");
		    if (xmit_flag || ipt_min_vers) {
			op = (struct op_hdr *)hp;
			printf ("proc_id=%ld\n", op -> proc_id);
			llc = (struct ie2_llc_snaphdr *)&op -> snap;
			llc_dmp(llc);
			*type = llc->type;
			xmit_hlen = sizeof(*op);
		    }
		    else 	/* XXX: LLC HEADER MISSING ON RECV!!!! */
			*type = ETHERTYPE_IP;
		    break;
		}

		/* default case */
		printf("OTHER packet  ");
		xmit_hlen = 0;
		*type = ETHERTYPE_IP;
		break;
	}

	return(xmit_hlen);
}

mac_dmp(mac)
struct ie5_mac_hdr *mac; {
        register i;
        register nseg;

	printf("%s",beg_line);
	if (!like_snoop) printf("\n");
	printf("802.5 MAC header:\n");
	printf("%s",beg_line);
	printf("access control field = %x, frame control field = %x\n", 
		mac->mac_acf, mac->mac_fcf);
	printf("%s",beg_line);
#ifndef mac_src_802_5
#define mac_src_802_5 mac_src
#define mac_dst_802_5 mac_dst
#endif
        printf("[ src = %s, ", ie5_sprintf(mac->mac_src_802_5));
        printf("dst = %s]\n", ie5_sprintf(mac->mac_dst_802_5));

        if (has_route(mac)) {
                nseg  = route_bytes(mac) - sizeof (mac->mac_rcf);
                nseg /= sizeof (mac->mac_seg[0]);

		printf("%s",beg_line);
                printf("routing control field = %04x,  %d routing segments \n", 
			mac->mac_rcf, nseg);

		/* its possible to get here, but no routing segments */
		if (nseg) {
		    printf("%s",beg_line);
		    printf("routing segments [ ");
		    for (i = 0; i < nseg; ++i)
			    printf("%x ", mac->mac_seg[i]);

		    printf(" ]\n");
		}
        }
}

fddi_mac_dmp(mac)
struct fddi_mac_hdr *mac; 
{
        register i;
        register nseg;

	printf("%s",beg_line);
	if (!like_snoop) printf("\n");
	printf("FDDI MAC header:\n");
	printf("%s",beg_line);
	printf("frame control field = %x\n", 
		mac->mac_fcf);
	printf("%s",beg_line);
        printf("[ src = %s, ", ie5_sprintf(mac->mac_src_f));
        printf("dst = %s]\n", ie5_sprintf(mac->mac_dst_f));

        if (has_route(mac)) {
                nseg  = route_bytes(mac) - sizeof (mac->mac_rcf);
                nseg /= sizeof (mac->mac_seg[0]);

		printf("%s",beg_line);
                printf("routing control field = %04x,  %d routing segments \n", 
			mac->mac_rcf, nseg);

		printf("%s",beg_line);
                printf("routing segments [ ");
                for (i = 0; i < nseg; ++i)
                        printf("%x ", mac->mac_seg[i]);

                printf(" ]\n");
        }
}

fcs_mac_dmp(mac)
struct fcs_mac_hdr *mac; 
{
#define addr mac->hw_addr

        register i;
        register nseg;

	printf("%s",beg_line);
	if (!like_snoop) printf("\n");
	printf("FCS MAC header:\n");
	printf("%s",beg_line);
	printf("Nport number = %x%x%x,", addr[1], addr[2], addr[3]);
	printf(" IPA String = %x%x%x%x%x%x%x%x\n", addr[4], addr[5], addr[6],
			addr[7], addr[8], addr[9], addr[10], addr[11]);
        printf("Unit number = %x,", mac->unit_num);
        printf(" Entity number = %x\n", mac->entity_num);
	printf("%s",beg_line);
        printf("Total length of optional hdrs = [%x]\n", mac->opt_hdr_len);

	if (mac->opt_hdr_len) {
		printf("%s",beg_line);
		printf("Optional hdrs:\n");
		hex_dump(((char *)mac) + FCS_MAC_SIZE + FCS_LLC_SIZE, 
			 mac->opt_hdr_len);
		printf("%s\n",beg_line);
	}

}

llc_dmp(llc)
struct ie2_llc_snaphdr *llc; {

	printf("%s",beg_line);
	printf("802.2 LLC header:\n");
	printf("%s",beg_line);
        printf("dsap %x, ssap %x, ctrl %x, proto %x:%x:%x, type %x"
                , llc->dsap
                , llc->ssap
                , llc->ctrl
                , llc->prot_id[0]
                , llc->prot_id[1]
                , llc->prot_id[2]
                , llc->type);
}

/*
 * Convert Token ring address to printable (loggable) representation.
 */
char *
ie5_sprintf(ap)
        register u_char *ap;
{
        register i;
        static char tokenbuf[18];
        register char *cp = tokenbuf;
        static char digits[] = "0123456789abcdef";

        for (i = 0; i < 6; i++) {
                *cp++ = digits[*ap >> 4];
                *cp++ = digits[*ap++ & 0xf];
                *cp++ = ':';
        }
        *--cp = 0;
        return (tokenbuf);
}

hex_dump(bp, cc)
u_char *bp;
int  cc;
{
        int     i;
	int	end;
	int	skipping = 0, skipped = 0;
	u_char	*bbp;

	end = Xbytes >= 0 ? Xbytes : cc;
        for ( i = 0, bbp = bp; i < end ; bp += 16, i += 16) {

		/* skip blocks of identical bits */
		if (i && (cc - i) > 16) {	/* guarantee last line */
		    if (!bcmp (bbp, bp, 16)) {
			skipping++;
			skipped++;
		    }
		    else
			skipping = 0;
		    bbp += 16;
		    if (skipping)
			continue;
		}
		printf("%s",beg_line);
		if (skipped) {
		    printf ("********\n%s", beg_line);
		    skipped = 0;
		}
                printf("%8.8x    ", i);
                dump_line(bp, MIN(16, cc - i));
        }
}

hex_dump_line(bp, byte_count)
u_char          *bp;
int             byte_count;
{
        int     j;

        for ( j = 0 ; j < 16 ; j++) {
                if (j >= byte_count)
                        break;
                if (j % 4 == 0)
                        printf(" ");
                printf("%2.2x", bp[j]);
        }
        while ( j < 16 ) {
                if (j % 4 == 0)
                        printf(" ");
                printf("  ");
		j++;
        }
}

dump_line(bp, byte_count)
u_char          *bp;
int             byte_count;
{
        int     j;

	hex_dump_line(bp, byte_count);

        printf("     |");
        for ( j = 0 ; j < 16 ; j++) {
                if (j >= byte_count)
                        break;
		if (!ebc_dick)
			printf("%c", isprint(bp[j]) ? bp[j] : '.');
		else {
			char asc[]=" ";
			char ebc[]=" ";
			
			*ebc=bp[j];
			if (0==NLxin(asc,ebc,1)) 
				printf(".");
			else
				printf("%c", isprint(*asc) ? *asc : '.');
		}
        }
        while ( j++ < 16 )
                printf(" ");
        printf("|\n");
}

char *
ether_sprintf(cp)
	u_char *cp;
{
	static	char	ether_buf[20];

	sprintf(ether_buf, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
		cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
	return(ether_buf);
}

fcs_addr_print(cp)
	u_char *cp;
{
	int i;

	printf("Nport[%x%x%x]", cp[1], cp[2], cp[3]);
	printf(" IPA String[%x%x%x%x%x%x%x%x%x]\n", cp[4],cp[5],cp[6],cp[7],
			   cp[8],cp[9],cp[10],cp[11], cp[12]);
}

#ifndef mac_len
#undef mac_src
#undef mac_dst
#endif

ie3_mac_dmp(mac)
struct ie3_mac_hdr *mac; {

	printf("%s",beg_line);
	if (!like_snoop) printf("\n");
	printf("802.3 MAC header:");
        printf("[ src = %s, ", ie5_sprintf(mac->ie3_mac_src));
        printf("dst = %s]\n", ie5_sprintf(mac->ie3_mac_dst));
	if (!like_snoop) printf("\n");
	printf("%s",beg_line);
        printf("frame length = %d\n", mac->ie3_mac_len);
}

