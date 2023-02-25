static char sccsid[] = "@(#)31  1.4  src/bos/usr/samples/dlpi/sdlpi.c, sysxpse, bos41J, 9508A 2/13/95 15:44:01";
/*
 * COMPONENT_NAME: SYSXPSE
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *           NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
 *
 * INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 * EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 * OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 * IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 * DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 * DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 * CORRECTION.
 *
 *  * RISC System/6000 is a trademark of International Business Machines
 *    Corporation.
 */

/*
 * This file contains sample source code for sdlpi, a server process that
 * communicates with the STREAMS Data Link Protocol Interface (DLPI) driver.
 * The companion client source code, cdlpi.c, is shipped in the same directory.
 *
 * The usage statement for this server process, sdlpi, is:
 *
 * sdlpi -n Node -s SAP [-b PeerSAP]... [-c] [-f PktFormat] [-m MultiAddr]... [-p Level] [-v]
 * where:
 *	Flags:
 *         b  PeerSAP	Indicates that the peer Service Access Point (SAP)
 *			specified by the PeerSap parameter should be bound to
 *			this stream.  The PeerSap must be specified in
 *			hexadecimal in the following format:  "xx:", "xx:xx",
 *			or "xx:xx:xx:xx:xx", for 802.2, ethernet, or SNAP.
 *			This flag can be specified more than once.  See the
 * 			DL_SUBS_BIND_REQ primitive manpage for more information.
 *
 *         c            Indicates that the global (for all streams) and local
 *			(per stream) statistical counts should be diplayed.
 *			See the DL_GET_STATISTICS_REQ primitive manpage for
 *			more information.
 *
 *         f PktFormat  Indicates that the packet format is to be set to the
 *                      type specified by the PktFormat parameter.  PktFormat
 *			must be one of the following integer values:
 *                             1	NS_PROTO
 *                             2	NS_PROTO_SNAP
 *                             4	NS_INCLUDE_LLC
 *                             8	NS_INCLUDE_MAC
 *			If this parameter is not specified, the default packet
 *			format is NS_PROTO.  See the infoExplorer article,
 *			"DLPI Specific Information" for more information on
 *			packet formats.
 *
 *	   n  Node	Indicates that the network interface specified by the
 *			Node parameter is to be used for communication.
 *			This parameter is absolutely required.
 *			The Node parameter must be specified in the following
 *			format:      /dev/dlpi/tr
 *				     /dev/dlpi/en
 *				     /dev/dlpi/et
 *                                   /dev/dlpi/fddi
 *
 *         m  MultiAddr	Indicates that the multicast address specified by the
 *                      MultiAddr parameter should be enabled.  This flag can
 *			be specified more than once.
 *                      See the DL_ENABMULTI_REQ Primitive manpage in
 *                      infoExplorer for more information on this primitive.
 *
 *         p  Level     Indicates that promiscuous mode should be enabled
 *                      at the level specified by the Level parameter.
 *			Level must be one of the following integer values:
 *                             1	DL_PROMISC_SAP
 *                             2	DL_PROMISC_PHYS
 *                             3	DL_PROMISC_MULTI
 *                      See the DL_PROMISCON_REQ Primitive manpage in
 *                      infoExplorer for more information on this primitive.
 *
 *	   s  SAP     	Indicates the stream is to bound to the Service Access
 *			Point (SAP) specified by the SAP parameter.  This value
 *			must be specified as a hexadecimal number, for example,
 *			0x3.  This parameter is absolutely required.
 *
 *         v		Indicates that the server process is to run in verbose
 *			mode.
 *	  
 * An example of the usage for standard ethernet is:
 *
 *     sdlpi -s 0x660 -n /dev/dlpi/en  -p 2 -f 1 -v
 *
 */


/*
 * sdlpi - dlpi server
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>
#include <sys/dlpistats.h>
#include <signal.h>
#include <sys/cdli.h>

typedef struct address {
	char   address[6];
	int    len;
	struct address  *next;
} addr_t;

addr_t  *mcastlist = NULL;
addr_t  *psaplist = NULL;

char data[4096];     /* data portion of response */
char buf[4096];      /* proto portion of response */
char *Prog;
int Verbose=0;
static int bar = 0;
char src_address[64];
ulong src_addr_sap_len;

void  peerbind();
int   conv();

close_prog()
{
   printf("total number of packets received = %d\n", bar);
   exit(0);
}

main(ac, av)
   int ac;
   char **av;
{
   int fd;
   int pkt_format = NS_PROTO;		/* default */
   struct strioctl strioctl;
   char *p, *node;
   unsigned long sap;
   addr_t *address;
   char buf1[4096];      
   int  peersap=0;
   addr_t  *m, *ps;
   int multicast=0;
   int promiscuous=0, prom_level=0;
   int stats=0;
   int c;
   extern int optind;
   extern char *optarg;

   struct strbuf ctl, dat;
   int flags, nflag=0, sflag=0, fflag=0;

   /* static so init to all zeros */
   static dl_attach_req_t    attach;
   static dl_info_req_t      info;
   static dl_promiscon_req_t promiscon;
   static dl_enabmulti_req_t *emultireq;

   dl_unitdata_req_t *udreq;
   dl_unitdata_ind_t *udind;
   dl_ok_ack_t       *okack;
   dl_info_ack_t     *infoack;

   if (Prog = strrchr(av[0], '/'))
      av[0] = ++Prog;
   else
      Prog = av[0];

   if (ac < 3) usage();

   while ((c = getopt(ac, av, "b:cf:m:n:p:s:v")) != EOF) {
      switch (c) {
	case 'b':
		peersap++;
		address = (addr_t *)malloc(sizeof(addr_t));
		bzero(address, sizeof(addr_t));
       		address->len = conv(optarg, address->address);
		if (!psaplist)
			psaplist = address;
		else {
			for (ps=psaplist; ps->next; ps=ps->next)
				;
			ps->next = address;	
		}
		break;
	case 'c':
		stats++;
		break;
	case 'f':
		fflag++;
                pkt_format = atoi(optarg);
                switch (pkt_format) {
                    case 1:
			pkt_format = NS_PROTO;
			break;
                    case 2:
			pkt_format = NS_PROTO_SNAP;
			break;
                    case 4:
			pkt_format = NS_INCLUDE_LLC;
			break;
                    case 8:
			pkt_format = NS_INCLUDE_MAC;
                        break;
                    default:
                        printf("%s: Invalid packet format specified\n",
                                        optarg);
                        usage();
                }
                break;
	case 'm':
		multicast++;
		address = (addr_t *)malloc(sizeof(addr_t));
		bzero(address, sizeof(addr_t));
       		address->len = conv(optarg, address->address);
		if (!mcastlist)
			mcastlist = address;
		else {
			for (m=mcastlist; m->next; m=m->next)
				;
			m->next = address;	
		}
		break;
	case 'n':
   		node = optarg;
		nflag++;
		break;
	case 'p':
		promiscuous++;
		prom_level = atoi(optarg);
		switch (prom_level) {
		    case 1:
		    case 2:
		    case 3:
			break;
		    default:
			printf("%s: Invalid promiscuous level specified\n",
					optarg);
			usage();
		}
		break;
	case 's':
   		sap = strtoul(optarg, 0, 0);
		sflag++;
		break;
	case 'v':
		Verbose = 1;
		break; 
	default:
		usage();
      }
   }

   if (fflag > 1) {
	printf("%s: You can supply only one packet format flag and parameter\n",
		Prog);
	usage();
   }

   if (!sflag) {
	printf("%s: You must supply a SAP flag and parameter\n", Prog);
	usage();
   }

   if (!nflag) {
	printf("%s: You must supply a node flag and parameter\n", Prog);
	usage();
   }

   signal(SIGINT, close_prog);

   if ((fd = open(node, O_RDWR)) < 0)
      die(node);
   
   /*
    * attach to interface
    */
   
   attach.dl_primitive   = DL_ATTACH_REQ;
   attach.dl_ppa      = 0;

   (void)request(fd, &attach, sizeof attach, buf, sizeof buf);

   okack = (dl_ok_ack_t *)buf;
   if (okack->dl_primitive != DL_OK_ACK)
      errorack(buf, "cannot attach");

   printf("%s: attached\n", Prog);

   /*
    * Set up the packet format registration.  This must be specified after
    * the attach and before the bind.  NS_PROTO is the default packet type,
    * so we do not need to do anything if we are using the default.
    */

   if (pkt_format != NS_PROTO) {
      strioctl.ic_dp = (char *)&pkt_format;
      strioctl.ic_cmd = DL_PKT_FORMAT;
      strioctl.ic_timout = 0;
      strioctl.ic_len = sizeof(long);

      if (ioctl(fd, I_STR, &strioctl) < 0) {
         perror("ioctl I_STR failed");
         exit(1);
      }
   }

   /*
    * bind to sap, unbind, and bind again
    */

   bindsap(fd, sap);
   unbindsap(fd);
   bindsap(fd, sap);


   /*
    * get info
    */
   
   info.dl_primitive   = DL_INFO_REQ;

   (void)request(fd, &info, sizeof info, buf, sizeof buf);

   infoack = (dl_info_ack_t *)buf;
   if (infoack->dl_primitive != DL_INFO_ACK)
      errorack(buf, "cannot get info");

/*
   printf("%s: provider info:\n", Prog);
   printf("  max sdu        = %d\n", infoack->dl_max_sdu);
   printf("  min sdu        = %d\n", infoack->dl_min_sdu);
   printf("  mac type       = %d\n", infoack->dl_mac_type);
   printf("  addr length    = %d\n", infoack->dl_addr_length);
   printf("  current state  = %d\n", infoack->dl_current_state);
   printf("  service mode   = %d\n", infoack->dl_service_mode);
   printf("\n");
*/

   if (promiscuous) {         /* enable promiscuous mode */

      promiscon.dl_primitive   = DL_PROMISCON_REQ;
      switch (prom_level) {
         case 1:
            promiscon.dl_level       = DL_PROMISC_SAP;
            break;
         case 2:
            promiscon.dl_level       = DL_PROMISC_PHYS;
            break;
         case 3:
            promiscon.dl_level       = DL_PROMISC_MULTI;
            break;
      }

      (void)request(fd, &promiscon, sizeof promiscon, buf, sizeof buf);

      okack = (dl_ok_ack_t *)buf;
      if (okack->dl_primitive != DL_OK_ACK)
         errorack(buf, "cannot enable promiscuous mode");
   }

   if (multicast) {                /* enable multicast */

      emultireq = (dl_enabmulti_req_t *) buf1;
      emultireq->dl_primitive   = DL_ENABMULTI_REQ;
      emultireq->dl_addr_length = 6;
      emultireq->dl_addr_offset = sizeof(dl_enabmulti_req_t);
      for (m = mcastlist; m; m=m->next) {
         memcpy(buf1 + emultireq->dl_addr_offset, m->address, 
                emultireq->dl_addr_length);

         (void)request(fd, emultireq, sizeof(*emultireq)+6, buf, sizeof buf);

         okack = (dl_ok_ack_t *)buf;
         if (okack->dl_primitive != DL_OK_ACK)
            errorack(buf, "cannot enable multicast");
      }
   }

   /*
    * peer bind
    */

   if (peersap) 
      peerbind(fd);

   /*
    * unbind peer sap
    */

   if (peersap) {
      dl_subs_unbind_req_t *peer;

      peer = (dl_subs_unbind_req_t *)buf;
      peer->dl_primitive       = DL_SUBS_UNBIND_REQ;
      peer->dl_subs_sap_offset = sizeof(*peer);
      for (ps = psaplist; ps; ps=ps->next) {
          peer->dl_subs_sap_length = ps->len;
          bcopy(ps->address, &peer[1], ps->len);

          (void)request(fd, peer, sizeof(*peer)+ps->len, buf, sizeof buf);

          okack = (dl_ok_ack_t *)buf;
          if (okack->dl_primitive != DL_OK_ACK)
               errorack(buf, "cannot unbind sap");
      }

      printf("unbind sap okay\n");
   }

   /*
    * peer bind
    */

   if (peersap) 
      peerbind(fd);

   /*
    * print out statistics
    */
   if (stats) {
       dl_stat(fd);
   }

   /*			
    * server loop - wait for query, respond with data
    */
   
   ctl.buf = buf;
   ctl.maxlen = sizeof buf;
   dat.buf = data;
   dat.maxlen = sizeof data;
   flags = 0;

   while (getmsg(fd, &ctl, &dat, &flags) != -1) {
      char buf2[1000];
      char dat2[100];

      if (Verbose) dump(0, &ctl, &dat);

      udind = (dl_unitdata_ind_t *)(ctl.buf);
      if (udind->dl_primitive != DL_UNITDATA_IND) {
         warn("bad primitive (%x)", udind->dl_primitive);
         continue;
      }
      
      if (pkt_format != NS_INCLUDE_MAC && pkt_format != NS_INCLUDE_LLC) {
          if (dat.buf[0] != 'q' || dat.buf[1] != 0) {
             /* warn("bad query"); */
             continue;
          }
      }

      query(&dat);

      udreq = (dl_unitdata_req_t *)buf2;
      udreq->dl_primitive = DL_UNITDATA_REQ;
      udreq->dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
      udreq->dl_priority.dl_min = udreq->dl_priority.dl_max = 0;

      if (pkt_format == NS_INCLUDE_MAC) {
	 udreq->dl_dest_addr_length = 0;
         /* place the destination address in the data portion */
         p = dat2;
         bcopy(ctl.buf + udind->dl_src_addr_offset, p, 6);
         p += 6;
         bcopy(src_address, p, src_addr_sap_len);
	 p += src_addr_sap_len;
	 bcopy(dat.buf, p, dat.len);
         ctl.len = sizeof(dl_unitdata_req_t);
	 dat.len += 6 + src_addr_sap_len;
	 dat.buf = dat2;
      }
      else {
      	udreq->dl_dest_addr_length = 6;
	
         memcpy(buf2 + udreq->dl_dest_addr_offset,
         ctl.buf + udind->dl_src_addr_offset, 6);
         ctl.len = sizeof(dl_unitdata_req_t) + 6;
      }

      ctl.buf = buf2;

      if (Verbose) dump("responding with:", 0, &dat);

      if (putmsg(fd, &ctl, &dat, 0))
         die("putmsg");
      ctl.buf = buf;
      ctl.maxlen = sizeof buf;
      flags = 0;
      if (pkt_format == NS_INCLUDE_MAC) {
	 dat.buf = data;
         dat.maxlen = sizeof data;
      }
   }
   die("getmsg");
   
   close(fd);
   exit(0);
}

errorack(p, msg)
   dl_error_ack_t *p;
   char *msg;
{
   extern int errno;

   if (p->dl_errno != DL_SYSERR)
      fprintf(stderr, "%s: dl_errno %d\n", msg, p->dl_errno);
   else {
      errno = p->dl_unix_errno;
      perror(msg);
   }
   exit(1);
}

dump(msg, ctl, dat)
   char *msg;
   struct strbuf *ctl;
   struct strbuf *dat;
{
   if (msg) { printf("%s\n", msg); }
   if (ctl) { printf("ctlbuf:\n"); hexdump(ctl->buf, ctl->len); }
   if (dat) { printf("datbuf:\n"); hexdump(dat->buf, dat->len); }
}

hexdump(p, len)
   char *p;
   int len;
{
   int count = 0;

   while (len-- > 0) {
      if (count++ % 16 == 0 && count != 1)
         printf("\n");
      printf("%02x ", *p++);
   }
   printf("\n");
}

/*
 * request - make a DLPI request
 *
 *   returns length of user data
 *   stores proto data in res, user data in data[]
 */

int
request(fd, req, req_len, res, res_len)
   int fd;
   caddr_t req;
   int req_len;
   caddr_t res;
   int res_len;
{
   struct strbuf ctl, dat;
   int flags = 0;

   ctl.buf = req;
   ctl.len = req_len;

   if (putmsg(fd, &ctl, 0, 0))
      die("putmsg");

   ctl.buf = res;
   ctl.maxlen = res_len;
   dat.buf = data;
   dat.maxlen = sizeof data;
   dat.len = 0;

   if (getmsg(fd, &ctl, &dat, &flags) != 0) 
      die("getmsg");
   
   return dat.len;
}

/*
 * bind to sap
 */
bindsap(fd, sap)
   int fd;
   unsigned long  sap;
{
   dl_bind_ack_t        *bindack;
   static dl_bind_req_t  bind;
   char  *p;
   int    i;

   bind.dl_primitive      = DL_BIND_REQ;
   bind.dl_sap            = sap;
   bind.dl_max_conind     = 0;
   bind.dl_service_mode   = DL_CLDLS;
   bind.dl_conn_mgmt      = 0;
   bind.dl_xidtest_flg    = 0;

   (void)request(fd, &bind, sizeof bind, buf, sizeof buf);

   bindack = (dl_bind_ack_t *)buf;
   if (bindack->dl_primitive != DL_BIND_ACK)
      errorack(buf, "cannot bind");

   bcopy(buf + bindack->dl_addr_offset, src_address, bindack->dl_addr_length);
   src_addr_sap_len = bindack->dl_addr_length;

   printf("%s: bound to:\n", Prog);
   printf("  sap       = %x\n", bindack->dl_sap);
   printf("  address   = ");
   p = buf + bindack->dl_addr_offset;
   for (i = bindack->dl_addr_length; i--;)
      printf("%02x:", *p++);
   printf("\n");
}

/*
 * unbind
 */
unbindsap (fd)
   int fd;
{
   static dl_unbind_req_t   unbind;
   dl_ok_ack_t             *okack;

   unbind.dl_primitive     = DL_UNBIND_REQ;

   (void)request(fd, &unbind, sizeof unbind, buf, sizeof buf);

   okack = (dl_ok_ack_t *)buf;
   if (okack->dl_primitive != DL_OK_ACK)
      errorack(buf, "cannot unbind");
}

/*
 * peer bind
 */
void
peerbind(fd)
   int fd;
{
   dl_subs_bind_req_t *peer;
   addr_t             *ps;
   dl_ok_ack_t        *okack;
   dl_error_ack_t     *err_ack;

   peer = (dl_subs_bind_req_t *)buf;
   peer->dl_primitive       = DL_SUBS_BIND_REQ;
   peer->dl_subs_bind_class = DL_PEER_BIND;
   peer->dl_subs_sap_offset = sizeof(*peer);

   for (ps = psaplist; ps; ps=ps->next) {
       peer->dl_subs_sap_length = ps->len;
       bcopy(ps->address, &peer[1], ps->len);

       (void)request(fd, peer, sizeof(*peer)+ps->len, buf, sizeof buf);

       err_ack = (dl_error_ack_t *)buf;
       if (err_ack->dl_primitive != DL_SUBS_BIND_ACK) {
          errno = err_ack->dl_unix_errno;
          die("cannot peer bind, dlerror %d", err_ack->dl_errno);
       }
   }
   printf("peer bind okay\n");
}

int
dl_stat(fd)
        int fd;
{
        dl_get_statistics_req_t stat;
        dl_get_statistics_ack_t *statack;
        struct statistics *local;
        struct statistics *global;
	dl_ok_ack_t       *okack;

        stat.dl_primitive = DL_GET_STATISTICS_REQ;

        (void)request(fd, &stat, sizeof stat, buf, sizeof buf);

	statack = (dl_get_statistics_ack_t *)buf;
	if (statack->dl_primitive != DL_GET_STATISTICS_ACK)
		errorack(buf, "cannot get statistics");
        if (!statack->dl_stat_length ||
                statack->dl_stat_length != 2*sizeof(struct statistics)) {
                printf("No statistics available\n");
                return;
        }

        local = (struct statistics *)(buf + statack->dl_stat_offset);
        global = (struct statistics *)(buf + statack->dl_stat_offset +
                        sizeof(struct statistics));

        printf("\n%s: provider local (Per Stream) statistics:\n", Prog);
        printf("Number of received packets      = %d\n", local->rx_pkts);
        printf("Number of transmitted packets   = %d\n", local->tx_pkts);
        printf("Number of received bytes        = %d\n", local->rx_bytes);
        printf("Number of transmitted bytes     = %d\n", local->tx_bytes);
        printf("Number of incoming pkts discard = %d\n", local->rx_discards);
        printf("Number of outgoing pkts discard = %d\n", local->tx_discards);
        printf("Number of times no buffers      = %d\n", local->no_bufs);
        printf("Number of successful binds      = %d\n", local->binds);
        printf("Number of unknown message types = %d\n", local->unknown_msgs);
        printf("Status of phys level promisc    = %d\n", local->promisc_phys);
        printf("Status of sap level promisc     = %d\n", local->promisc_sap);
        printf("Status of multi level promisc   = %d\n", local->promisc_multi);
        printf("Number of enab_multi addresses  = %d\n",local->multicast_addrs);

        printf("\n%s: provider global (All Streams) statistics:\n", Prog);
        printf("Number of received packets      = %d\n", global->rx_pkts);
        printf("Number of transmitted packets   = %d\n", global->tx_pkts);
        printf("Number of received bytes        = %d\n", global->rx_bytes);
        printf("Number of transmitted bytes     = %d\n", global->tx_bytes);
        printf("Number of incoming pkts discard = %d\n", global->rx_discards);
        printf("Number of outgoing pkts discard = %d\n", global->tx_discards);
        printf("Number of times no buffers      = %d\n", global->no_bufs);
        printf("Number of successful binds      = %d\n", global->binds);
        printf("Number of unknown message types = %d\n", global->unknown_msgs);
        printf("Status of phys level promisc    = %d\n", global->promisc_phys);
        printf("Status of sap level promisc     = %d\n", global->promisc_sap);
        printf("Status of multi level promisc   = %d\n", global->promisc_multi);
        printf("Number of enab_multi addresses  = %d\n", global->multicast_addrs
);

}


/*
 * query - queries how many times we've done this
 */

query(dat)
   struct strbuf *dat;
{

   (void)sprintf(data, "%05d", ++bar);

   dat->buf = data;
   dat->len = strlen(data)+1;
   dat->maxlen = sizeof data;
}

warn(s, a, b, c)
   char *s;
   int a, b, c;
{
   extern int errno;
   char buf[200];
   sprintf(buf, s, a, b, c);
   if (errno)
      perror(buf);
   else
      fprintf(stderr, "%s\n", buf);
}

die(s, a, b, c)
   char *s;
   int a, b, c;
{
   warn(s, a, b, c);
   exit(1);
}

int
conv(cp, buf)
   char *cp;
   char *buf;
{
   char *p;
   char *addr;

   addr = buf;
   for (p = strtok(cp, ":"); p; p = strtok(0, ":")) {
      *addr++ = strtoul(p, 0, 16);
   }
   return ((int)addr - (int)buf);
}

usage()
{
   fprintf(stderr, "usage: %s -n Node -s SAP [-b PeerSAP]... [-c] [-f PktFormat] [-m MultiAddr]... [-p Level] [-v]\n", Prog);
   exit(1);
}
