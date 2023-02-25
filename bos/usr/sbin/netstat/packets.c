/* static char sccsid[] = "@(#)72       1.5  src/bos/usr/sbin/netstat/packets.c, cmdnet, bos411, 9428A410j 5/25/94 17:21:21"; */
/*
 * COMPONENT_NAME: (CMDNET) Network commands. 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ndd_var.h>
#include <netns/ns.h>
#include <netns/ns_if.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <stdio.h>
#include <values.h>
#include <sys/ldr.h>    	/* For deferred resolution of NFS lib calls */
#include <nfs/nfs_load.h>
#include <nfs/nfs_fscntl.h>	/* Pulls in rpc/clnt.h and rpc/svc.h */
#include <sys/errno.h>

extern	char *interface;
extern	int unit;

#include <nl_types.h>
#include "netstat_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s) 

#define LONGDASHES  "-------------------------------------------------------------------------------\n"
#define SHORTDASHES "                ---------------------------------------------------------------\n"

#define NAMELENGTH  16

#define NETSTAT_LOAD "/usr/sbin/netstat_load"

typedef struct {
	u_int   buflen;
	caddr_t buf_p;
} io_block_t;

typedef struct dd_stats {
	char    name[NAMELENGTH];
	double  ipkts;
	double  opkts;
	unsigned long idrops;
	unsigned long odrops;
	struct dd_stats  *next;
};

struct  packets {
	unsigned long  ipkts;		/* Received packets */
	unsigned long  opkts;		/* Transmitted packets */
	unsigned long  idrops;		/* Received packets dropped */
	unsigned long  odrops;		/* Transmitted packets dropped */
};

struct packets  dev, dd, ip, tcp, udp, ifn, ifn_all, nfs;

void  devices_and_drivers();
void  demuxer();
void  interfaces();
void  protocols();
void  nfs_rpc();
int  source_name();


/* 
 * pkt_drop_stats()
 * Retrieve the packet counts from the communications system
 * and print them up in a pretty chart.
 */
void
pkt_drop_stats(nl_ip, nl_tcp, nl_udp, ifnetaddr, nl_ndd)
	off_t nl_ip;
	off_t nl_tcp;
	off_t nl_udp;
	off_t ifnetaddr;
	off_t nl_ndd;
{
	bzero(&dev, sizeof(struct packets));
	bzero(&dd, sizeof(struct packets));
	bzero(&ip, sizeof(struct packets));
	bzero(&tcp, sizeof(struct packets));
	bzero(&udp, sizeof(struct packets));
	bzero(&ifn, sizeof(struct packets));
	bzero(&ifn_all, sizeof(struct packets));
	bzero(&nfs, sizeof(struct packets));

	/*
	 * Headers
	 */
	printf("\nSource                         Ipkts                Opkts     Idrops     Odrops\n");
	printf(LONGDASHES);

	/*
	 * Get the device and device driver statistics.
	 */
	devices_and_drivers(nl_ndd);

	/*
	 * Get the demuxer statistics.
	 */
	demuxer(nl_ndd);

	/*
	 * Get the IP/UDP/TCP statistics.
	 */
	protocols(nl_ip, nl_tcp, nl_udp);

	/*
	 * Get the IF layer statistics.
	 */
	interfaces(ifnetaddr);

	/*
	 * Get the NFS/RPC statistics.
	 */
	nfs_rpc();

	/*
	 * Add the final note.
	 */
	printf("(Note:  N/A -> Not Applicable)\n");

	/* Th-th-that's all folks!. */
}


/*
 * interfaces()
 * Get the IF layer statistics from the CDLI-based interfaces.
 */
void
interfaces(ifnetaddr)
	off_t ifnetaddr;
{
	struct ifnet ifnet;
	char name[NAMELENGTH];

	if (ifnetaddr == 0) {
		printf(MSGSTR(SYM_NO_DEF, "ifnet: symbol not defined\n"));
		return;
	}
	kvm_read(ifnetaddr, (char *)&ifnetaddr, sizeof ifnetaddr);
	while (ifnetaddr) {
		register char *cp;
		int n, m;
		char *index();

		kvm_read(ifnetaddr, (char *)&ifnet, sizeof ifnet);
		kvm_read((off_t)ifnet.if_name, name, 16);
		name[15] = '\0';
		ifnetaddr = (off_t) ifnet.if_next;
		if (interface != 0 &&
		    (strcmp(name, interface) != 0 || unit != ifnet.if_unit))
			continue;
		cp = index(name, '\0');
		cp += sprintf(cp, "_if%d", ifnet.if_unit);
		cp += 4;
		*cp = '\0';

		ifn.ipkts = ifnet.if_ipackets;
		ifn.opkts = ifnet.if_opackets;
		ifn.idrops = ifnet.if_iqdrops + ifnet.if_ierrors + 
				ifnet.if_noproto;
		ifn.odrops = ifnet.if_snd.ifq_drops + ifnet.if_oerrors +
				ifnet.if_arpdrops;
		printf("%-15.15s %20u %20u %10u %10u\n", name, ifn.ipkts,
			ifn.opkts, ifn.idrops, ifn.odrops);

		/*
		 * Get the subtotals over all interfaces.
		 */
		ifn_all.ipkts += ifn.ipkts;
		ifn_all.opkts += ifn.opkts;
		ifn_all.idrops += ifn.idrops;
		ifn_all.odrops += ifn.odrops;
	}
	printf(SHORTDASHES);
	printf("Net IF Total    %20u %20u %10u %10u\n", ifn_all.ipkts,
			ifn_all.opkts, ifn_all.idrops, ifn_all.odrops);
	printf(LONGDASHES);
}


/*
 * protocols()
 * Get the IP, TCP, and UDP protocol layer statistics.
 */
void
protocols(nl_ip, nl_tcp, nl_udp)
	off_t  nl_ip;
	off_t  nl_tcp;
	off_t  nl_udp;
{
	struct ipstat ips;
	struct tcpstat tcpstat;
	struct udpstat udpstat;
	off_t  off;

	/*
	 * IP counters
	 */
	kvm_read(nl_ip, (caddr_t)&ips, sizeof ips);

	ip.ipkts = ips.ips_total + ips.ipInDiscards;
	ip.opkts = ips.ips_delivered;
	ip.idrops = ips.ips_badsum + ips.ips_tooshort +
			ips.ips_toosmall + ips.ips_badhlen +
			ips.ips_badlen + ips.ipInHdrErrors +
			ips.ipInAddrErrors + ips.ips_noproto +
			ips.ipInDiscards + ips.ips_fragdropped +
			ips.ips_fragtimeout + ips.ips_badoptions +
			ips.ips_badvers;
	ip.odrops = ips.ips_odropped + ips.ips_cantfrag +
			ips.ips_cantforward + ips.ips_noroute;

	printf("%-15.15s %20u %20u %10u %10u\n", "IP", ip.ipkts, ip.opkts,
		ip.idrops, ip.odrops);

	/*
	 * TCP counters
	 */
	kvm_read(nl_tcp, (caddr_t)&tcpstat, sizeof tcpstat);
	tcp.ipkts = tcpstat.tcps_rcvtotal;
	tcp.opkts = tcpstat.tcps_sndtotal - tcpstat.tcps_sndrexmitpack;
	tcp.idrops = tcpstat.tcps_rcvbadsum + tcpstat.tcps_rcvbadoff +
			tcpstat.tcps_rcvshort;
	/* No counters for tcp out drops. */

	printf("%-15.15s %20u %20u %10u %10u\n", "TCP", tcp.ipkts, tcp.opkts,
		tcp.idrops, tcp.odrops);

	/*
	 * UDP counters
	 */
	kvm_read(nl_udp, (caddr_t)&udpstat, sizeof udpstat);
	udp.ipkts = udpstat.udps_ipackets;
	udp.opkts = udpstat.udps_opackets;
	udp.idrops = udpstat.udps_hdrops + udpstat.udps_badsum +
			udpstat.udps_badlen + udpstat.udps_fullsock;
	udp.odrops = udpstat.udps_noport;

	printf("%-15.15s %20u %20u %10u %10u\n", "UDP", udp.ipkts, udp.opkts,
		udp.idrops, udp.odrops);

	printf(SHORTDASHES);
	printf("Protocols Total %20u %20u %10u %10u\n", 
			ip.ipkts + tcp.ipkts + udp.ipkts,
			ip.opkts + tcp.opkts + udp.opkts,
			ip.idrops + tcp.idrops + udp.idrops,
			ip.odrops + tcp.odrops + udp.odrops);
	printf(LONGDASHES);
}


/*
 * devices_and_drivers()
 * Get both the device and device driver statistics from the device drivers.
 */
void
devices_and_drivers(nl_ndd)
	off_t nl_ndd;
{
	struct ndd  ndd;
	char   ndd_name[NAMELENGTH];
	char   output_name[NAMELENGTH];
	int    length, unit;
        double big_ipkts, big_opkts;
	double total_big_ipkts;
	double total_big_opkts;
	int fd;
	struct sockaddr_ndd sa;
	io_block_t io_block; 		/* Table for socket ioctl */
	struct dd_stats *dd_stats;	/* The new dd stats structure */
	struct dd_stats *dd_stats_h;	/* Points to the head of list */
	struct dd_stats *dd_stats_p;	/* Points to the end of list */

	total_big_ipkts = (double)0;
	total_big_opkts = (double)0;

	/* Get the stats for the devices */
	kvm_read(nl_ndd, (caddr_t)&ndd, sizeof ndd);
	nl_ndd = ndd.ndd_next;

	/*
	 * Walk the ndd structure in kernel memory to determine which
	 * ndd device drivers to open.  Open each device driver, issue a
	 * ndd_ctl() with the NDD_GET_STATS command to get the device stats.
	 * In one pass, get both the device and device driver stats since
	 * an NDD_GET_STATS must be issued to update the genstats structure
	 * for both the devices and the device drivers.  Save the device driver
	 * stats in a linked list so they can be printed out later.
	 */
	dd_stats_h = dd_stats_p = NULL;
	while (nl_ndd) {
		kvm_read(nl_ndd, (caddr_t)&ndd, sizeof ndd);
		kvm_read((off_t)ndd.ndd_name, ndd_name, sizeof(ndd_name));
		nl_ndd = ndd.ndd_next;

		/*
		 * Check the NDD_UP flag before going any further because the
		 * dd could be configured and not open.  If the flag is off,
		 * skip this device and driver.  This could happen if the card
		 * is present but is not being used.
		 */
		if (!(ndd.ndd_flags && NDD_UP))
			continue;

		/* Open a socket to the device specified in ndd_name. */
		if ((fd = socket(AF_NDD, SOCK_DGRAM, 0)) < 0) {
			printf(MSGSTR(NO_OPEN,
			  "Unable to open device %s, errno = %d\n"),
			  ndd_name, errno);
			continue;
		}

		sa.sndd_family = AF_NDD;
		sa.sndd_len = sizeof(struct sockaddr_ndd);
		sa.sndd_filterlen = 0;
		bcopy(ndd_name, sa.sndd_nddname, sizeof(sa.sndd_nddname));
		if (connect(fd, &sa, sizeof(sa))) {
			printf(MSGSTR(NO_DEVCON,
			  "Unable to connect to device %s, errno = %d\n"),
			  ndd_name, errno);
			close(fd);
			continue;
		}

		/*
		 * Set the length to the sizeof the ndd_genstats structure
		 * plus the value of the ndd.ndd_speclen.  The device driver
		 * sets its ndd.speclen to the length of its device specific
		 * statistics structure.  This way netstat can stay generic
		 * and not have to be updated for a new device/device driver.
		 */
		if (io_block.buf_p) {
			free(io_block.buf_p);
		}
		io_block.buflen = sizeof(struct ndd_genstats) +
			ndd.ndd_speclen;
		if ((io_block.buf_p = (caddr_t)malloc(io_block.buflen))
				== NULL) {
			printf(MSGSTR(NO_MEM, "Out of memory\n"));
			exit(1);
		}

		/*
		 * Need to evaluate the return code - may get an error.
		 * The dd can go away - however, the ndd structure would
		 * still be in kernel memory.
		 */
		if (ioctl(fd, NDD_GET_STATS, (caddr_t)&io_block)) {
			printf(MSGSTR(NO_DEVSTATS,
			 "Unable to get statistics on device %s, errno = %d\n"),
			 ndd_name, errno);
			continue;
		}

		/* 
		 * Save off the device driver stats and link them into
		 * the end of the dd_stats linked list.  Don't bother to
		 * free the driver statistics linked list since the
	         * command completes so quickly - freeing would just be a
		 * performance hit.
		 */
		if ((dd_stats = calloc(1, sizeof(struct dd_stats))) == NULL) {
			printf(MSGSTR(NO_MEM, "Out of memory\n"));
			exit(1);
		}

		strcpy(dd_stats->name, ndd_name);

		dd_stats->ipkts = ((double)MAXLONG * 
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ipackets_msw)) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ipackets_lsw) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ibadpackets) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ipackets_drop);

		dd_stats->opkts = ((double)MAXLONG * 
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_opackets_msw)) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_opackets_lsw) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_xmitque_cur) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_oerrors) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_opackets_drop);

		dd_stats->idrops = ((struct ndd_genstats *)io_block.buf_p)
			->ndd_ipackets_drop;

		dd_stats->odrops = ((struct ndd_genstats *)io_block.buf_p)
			->ndd_opackets_drop;

		if (dd_stats_p == NULL) {
			dd_stats_p = dd_stats;
			dd_stats_h = dd_stats;
		}
		else {
			dd_stats_p->next = dd_stats;
			dd_stats_p = dd_stats;
		}

		/* Finished with this device - close it. */
		close(fd);

		/*
		 * Need 64 bit words to store the input and output packet
		 * counts.
		 */
		big_ipkts = ((double)MAXLONG * 
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ipackets_msw)) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ipackets_lsw) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ibadpackets) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ipackets_drop) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_ierrors);

		big_opkts = ((double)MAXLONG * 
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_opackets_msw)) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_opackets_lsw) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_xmitque_cur) +
			(double)(((struct ndd_genstats *)io_block.buf_p)
				->ndd_oerrors);

		/* Create the source name for the table output. */
		if (source_name(ndd_name, output_name, "dev"))
			continue;

		/* Now print out the device statistics. */
		printf("%-15.15s %20.0f %20.0f %10u %10u\n", output_name,
			big_ipkts, big_opkts,
			((struct ndd_genstats *)io_block.buf_p)->ndd_ierrors,
			((struct ndd_genstats *)io_block.buf_p)->ndd_oerrors);

		/* Grand total. */
		total_big_ipkts += big_ipkts;
		total_big_opkts += big_opkts;
		dev.idrops += ((struct ndd_genstats *)io_block.buf_p)
			       ->ndd_ierrors;
		dev.odrops += ((struct ndd_genstats *)io_block.buf_p)
			       ->ndd_oerrors;
	}

	printf(SHORTDASHES);
	printf("Devices Total   %20.0f %20.0f %10u %10u\n", 
			total_big_ipkts, total_big_opkts, dev.idrops,
			dev.odrops);
	printf(LONGDASHES);

	/* Now print out the device driver statistics. */

	dd_stats_p = dd_stats_h;
	total_big_ipkts = (double)0;
	total_big_opkts = (double)0;
	while (dd_stats_p) {
		/* Create the source name for the table output. */
		if (source_name(dd_stats_p->name, output_name, "dd")) {
			dd_stats_p = dd_stats_p->next;
			continue;
		}

		printf("%-15.15s %20.0f %20.0f %10u %10u\n", output_name,
			dd_stats_p->ipkts, dd_stats_p->opkts,
			dd_stats_p->idrops, dd_stats_p->odrops);

		total_big_ipkts += dd_stats_p->ipkts;
		total_big_opkts += dd_stats_p->opkts;
		dd.idrops += dd_stats_p->idrops;
		dd.odrops += dd_stats_p->odrops;

		dd_stats_p = dd_stats_p->next;
	}

	printf(SHORTDASHES);
	printf("Drivers Total   %20.0f %20.0f %10u %10u\n", 
			total_big_ipkts, total_big_opkts, dd.idrops, dd.odrops);
	printf(LONGDASHES);
}


/*
 * demuxer()
 * Get the demuxer statistics.
 */
void
demuxer(nl_ndd)
	off_t nl_ndd;
{
	struct ndd  ndd;
	char   ndd_name[NAMELENGTH];
	char   output_name[NAMELENGTH];
	int    length, unit;
	struct ns_dmx_ctl dmx_stats;
	ulong  idrops, total_idrops = 0;
        double big_ipkts, ipkts, total_ipkts = 0;

	kvm_read(nl_ndd, (caddr_t)&ndd, sizeof ndd);
	nl_ndd = ndd.ndd_next;

	/* Loop through each device */
	while (nl_ndd) {
		kvm_read(nl_ndd, (caddr_t)&ndd, sizeof ndd);
		kvm_read((off_t)ndd.ndd_name, ndd_name, sizeof(ndd_name));
		kvm_read((off_t)ndd.ndd_nsdemux, (caddr_t)&dmx_stats,
			sizeof(dmx_stats));
		nl_ndd = ndd.ndd_next;

		/* Create the source name for the table output. */
		if (source_name(ndd_name, output_name, "dmx"))
			continue;

		/* Make 64 bit numbers from 32 bit numbers. */
		big_ipkts = (double)MAXLONG * 
			(double)ndd.ndd_genstats.ndd_ipackets_msw;
		big_ipkts += (double)ndd.ndd_genstats.ndd_ipackets_lsw;

		idrops = dmx_stats.nd_dmxstats.nd_nofilter +
			    dmx_stats.nd_dmxstats.nd_nobufs;
		ipkts = big_ipkts - (double)idrops;

		/* The demuxer has no outgoing packets --> N/A */
		printf("%-15.15s %20.0f                  N/A %10u        N/A\n",
			output_name, ipkts, idrops);

		/* Add to the demuxer grand total */
		total_ipkts += ipkts;
		total_idrops += idrops;
	}

	printf(SHORTDASHES);
	printf("Demuxer Total   %20.0f                  N/A %10u        N/A\n", 
			total_ipkts, total_idrops);
	printf(LONGDASHES);
}


/*
 * source_name()
 * Generically create a source name for printing.
 */
int
source_name(orig_name, new_name, keyword)
char  *orig_name;
char  *new_name;
char  *keyword;		/* "dev", "dd", "dmx" */
{
	char  tmp_name[NAMELENGTH];
	int  unit, length;
	/* 
	 * Determine whether the unit number for this device name is one
	 * or two digits in length.
	 */
	if ((orig_name[strlen(orig_name) - 2] >= '0') && 
	    (orig_name[strlen(orig_name) - 2] <= '9')) {
		unit = atoi(&orig_name[strlen(orig_name) - 2]);
		length = strlen(orig_name) - 2;
	}
	else  {
		unit = atoi(&orig_name[strlen(orig_name) - 1]);
		length = strlen(orig_name) - 1;
	}

	/* Sanity check */
	if (unit < 0 || unit > 99)
		return (1);

	tmp_name[0] = '\0';
	strncat(tmp_name, orig_name, length);
	sprintf(new_name, "%s_%s%i", tmp_name, keyword, unit);
	return (0);
}


/*
 * nfs_rpc()
 * Get the NFS and RPC statistics.  NFS may not be loaded, so
 * implementation must be via deferred resolution.
 * Compile the code with DEBUG turned on to follow the deferred
 * resolution.
 */
void
nfs_rpc()
{
	int  rc_load, rc_ioctl;
	t_rsstat	rsstat;		/* RPC server stats */
	t_rcstat	rcstat;		/* RPC client stats */
	t_svstat	svstat;		/* NFS server stats */
	t_clstat	clstat;		/* NFS client stats */
	t_nfs_stats_pkts nfs_pkts;	/* NFS+RPC outgoing counters */

	/* Deferred resolution */
	int  (*entry_pt)();
#ifdef DEBUG
	int  lb_rc;
	char *mybuf[1000];
	struct ld_info  *info;
#endif

	/*
	 * The NFS/RPC library routines will not be unloaded when
	 * netstat terminates.  When netstat is restarted, the
	 * NFS/RPC library routines will be loaded into the same
	 * shared library space, so no space will be wasted.
	 *
	 * If the NFS/RPC library is not on the system, just return.
	 * No NFS/RPC stats will appear in the table.
	 */
	if ((entry_pt = load(NETSTAT_LOAD, 1, "")) == NULL) {
#ifdef DEBUG
		printf("nfs_rpc(): load() failed. errno=%d\n", errno);
		mybuf[0] = "/tmp/netstat";
		mybuf[1] = "/usr/sbin/netstat_load";
		loadquery(L_GETMESSAGES,&mybuf[1],sizeof(mybuf));
		execvp("/etc/execerror", mybuf);
#endif
		return;
	}

	if (loadbind (0, entry_pt, nfs_rpc) == -1)
		return;

#ifdef DEBUG
	/* Look at the list of loaded functions */
	lb_rc = loadquery(L_GETINFO, mybuf, sizeof(mybuf));
	if (lb_rc) {
        	perror("loadquery getinfo");
        	return;
   	}
   	printf("\nModule Data from file:\n");
   	info = (void *)mybuf;
   	while(1) {
      		char * m;
      		m = info->ldinfo_filename;
      		/* offset to member name (if any) */
      		m = m + strlen(m) + 1; /*member name*/
      		printf(" filetextorg = %X", info->ldinfo_textorg);
      		printf(" dataorg = %X", info->ldinfo_dataorg);
      		printf(" module name = %s %s", info->ldinfo_filename, m);
      		printf("\n");
      		if (info->ldinfo_next == 0){
            		printf("\n");
            		break;
      		}
      		info = (void *)((char *)info+info->ldinfo_next);
   	}
#endif

	/*
	 * Determine which NFS/RPC kernel extensions are currently loaded on
	 * the system.  If load_nfs() returns 0, then nothing is loaded
	 * and there are no packet stats to report.  Just return.
	 */
	if ((rc_load = load_nfs(NFS_KLOAD_STAT,0)) == 0)
		return;

	/*
	 * Rebind load_nfs() in order that the reference to nfs_cntl() 
	 * is resolved at runtime.  Rebind load_nfs() with the return
	 * code, rc, from the previous call.  If the rebind fails, we cannot
	 * retrieve the NFS/RPC statistics, so just return.
	 */
	if (load_nfs(rc_load,0) == 0)
		return;

	/*
	 * Determine what is loaded and report the stats accordingly.
	 * It is possible that not all NFS and RPC kernel extensions will
	 * be loaded.  If the NFS server kernel extension is loaded, then
	 * the NFS client and RPC server and client are also loaded.  The 
	 * value returned by load_nfs() is all inclusive -- all kernel
	 * extensions defined in sys/nfs.h to have values less than or
	 * equal to the returned value are loaded.
	 */

	if (rc_load >= NFS_KLOAD_KRPC) {
		/*
		 * NFS_CNTL_GET_CLKRPC_STAT returns the statistics for the
		 * RPC client.
		 */
		rc_ioctl = nfs_cntl(NFS_CNTL_GET_CLKRPC_STAT, &rcstat,
			  NFS_CNTL_CLKRPC_SIZE);

		if (rc_ioctl == 0) {
			nfs.idrops += rcstat.rcbadcalls;
                        printf("NFS/RPC Client  %20u                  N/A %10u        N/A\n", 
				rcstat.rccalls, rcstat.rcbadcalls);
		}

		/*
		 * NFS_CNTL_GET_SVKRPC_STAT returns the statistics for the
		 * RPC server.
		 */
		rc_ioctl = nfs_cntl(NFS_CNTL_GET_SVKRPC_STAT, &rsstat,
			  NFS_CNTL_SVKRPC_SIZE);

		if (rc_ioctl == 0) {
			nfs.idrops += rsstat.rsbadcalls;
			printf("NFS/RPC Server  %20u                  N/A %10u        N/A\n", 
				rsstat.rscalls, rsstat.rsbadcalls);
		}
	}

	if (rc_load >= NFS_KLOAD_CLNT) {
		/*
		 * NFS_CNTL_GET_SERV_STAT returns the statistics for the
		 * NFS server.
		 */
		rc_ioctl = nfs_cntl(NFS_CNTL_GET_CLNT_STAT, &clstat,
			  NFS_CNTL_CLNT_SIZE);

		if (rc_ioctl == 0) {
			nfs.idrops += clstat.nbadcalls;
			printf("NFS Client      %20u                  N/A %10u        N/A\n", 
				clstat.ncalls, clstat.nbadcalls);
		}
	}

	if (rc_load >= NFS_KLOAD_SERV) {
		/*
		 * NFS_CNTL_GET_SERV_STAT returns the statistics for the
		 * NFS server.
		 */
		rc_ioctl = nfs_cntl(NFS_CNTL_GET_SERV_STAT, &svstat,
			  NFS_CNTL_SERV_SIZE);

		if (rc_ioctl == 0) {
			nfs.idrops += svstat.nbadcalls;
			printf("NFS Server      %20u                  N/A %10u        N/A\n", 
				svstat.ncalls, svstat.nbadcalls);
		}
	}

	/*
	 * Get the number of output packets and output packets dropped 
	 * for all of NFS and RPC.  These statistics are not available
	 * individually for NFS and RPC.  Only global counts summing
	 * NFS and RPC output statistics are available.
	 */
	rc_ioctl = nfs_cntl(NFS_CNTL_GET_PKTS_STAT, &nfs_pkts,
			    NFS_CNTL_PKTS_SIZE);

	if (rc_ioctl == 0) {
		nfs.opkts = nfs_pkts.nfs_good_sends;
		nfs.odrops = nfs_pkts.nfs_send_tries - nfs_pkts.nfs_good_sends;
	}

	/*
	 * Grand total.  Summing Ipkts does not make sense, so N/A.
	 */
	printf(SHORTDASHES);
	printf("NFS/RPC Total                    N/A %20u %10u %10u\n",
		nfs.opkts, nfs.idrops, nfs.odrops);
	printf(LONGDASHES);
}
