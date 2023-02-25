static char sccsid[] = "@(#)32  1.3  src/bos/usr/samples/dlpi/cdlpi.c, sysxpse, bos411, 9433A411a 8/12/94 13:58:46";
/*
 * COMPONENT_NAME: SYSXPSE
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
 * This file contains sample source code for cdlpi, a client process that
 * communicates with the STREAMS Data Link Protocol Interface (DPLI) driver.
 * The companion server source code, sdlpi.c, is shipped in the same directory.
 *
 * The usage statement for this client process, cdlpi, is:
 *
 * cdlpi -d Address -n Node -s SAP [-c Count] [-f PktFormat]
 *
 * where:
 *	Flags:
 *	   c Count	Specifies the number of times that packets are to
 *			be transmitted to the server process.  This parameter
 *			must be specified as a positive integer greater than
 *			or equal to 1.
 *
 *	   d Address    Specifies the destination address to which the
 * 			packets from the client will be transmitted.  The
 *			destination address must be specified in hexadecimal
 *			in the following format: "xx:xx:xx:xx:xx:xx".  This
 *			parameter is absolutely required.
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
 *	   n Node	Indicates that the network interface specified by the
 *			Node parameter is to be used for communication.
 *			This parameter is absolutely required.
 *			The Node parameter must be specified in the following
 *			format:      /dev/dlpi/tr
 *				     /dev/dlpi/en
 *				     /dev/dlpi/et
 *                                   /dev/dlpi/fddi
 *
 *	   s SAP     	Indicates the stream is to bound to the Service Access
 *			Point (SAP) specified by the SAP parameter.  This value
 *			must be specified as a hexadecimal number, for example,
 *			0x3.  This parameter is absolutely required.
 *
 *
 * An example of the usage for standard ethernet is:
 *
 *     cdlpi -c 5 -d 2:60:8c:2e:aa:3c -f 1 -n /dev/dlpi/en -s 0x660
 *
 */


/*
 * cdlpi - dlpi client
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/poll.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>
#include <sys/dlpistats.h>
#include <netinet/if_ether.h>
#include <sys/cdli.h>

char data[4096];     /* data portion of response */
char buf[4096];      /* proto portion of response */
char *Prog;

char *conv();

main(ac, av)
   int ac;
   char **av;
{
   int fd, c, i, j, num_pack = 0;
   int flags, icnt, count=1, total = 0;
   struct strbuf ctl, dat;
   long  saplen;
   char  src_address[64];
   ulong src_addr_sap_len;
   int sflag=0, nflag=0, dflag=0, fflag=0;
   int pkt_format = NS_PROTO;		/* default */
   struct strioctl strioctl;

   /* static so init to all zeros */

   static dl_attach_req_t attach;
   static dl_bind_req_t bind;
   static dl_info_req_t info;
   static dl_unitdata_req_t *udreq;
   static dl_unitdata_ind_t *udind;

   dl_ok_ack_t *okack;
   dl_bind_ack_t *bindack;
   dl_info_ack_t *infoack; 

   char *p;

   char *node, *address;
   unsigned long sap;

   if (Prog = strrchr(av[0], '/'))
      av[0] = ++Prog;
   else
      Prog  = av[0];

   while ((c = getopt(ac, av, "c:d:f:n:s:")) != EOF) {
      switch (c) {
	case 'c':
   		count = atoi(optarg);
		if (count < 1) {
                        printf("%s: Invalid count specified\n", optarg);
                        usage();
		}
		break;
	case 'd':
		dflag++;
   		address = conv(optarg);
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
	case 'n':
   		node = optarg;
		nflag++;
		break;
	case 's':
   		sap = strtoul(optarg, 0, 0);
		sflag++;
		break;
	default:
		usage();
      }
   }

   if (!dflag) {
	printf("%s: You must supply a destination flag and parameter\n",
		Prog);
	usage();
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
 
   icnt = count;

   if ((fd = open(node, O_RDWR)) < 0)
      die(node);
   
   /*
    * attach to interface
    */
   
   attach.dl_primitive = DL_ATTACH_REQ;
   attach.dl_ppa       = 0;

   (void)request(fd, &attach, sizeof attach, buf, sizeof buf, "attach");

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
    * bind to sap
    */

   bind.dl_primitive    = DL_BIND_REQ;
   bind.dl_sap          = sap;
   bind.dl_max_conind   = 0;
   bind.dl_service_mode = DL_CLDLS;
   bind.dl_conn_mgmt    = 0;
   bind.dl_xidtest_flg  = 0;

   (void)request(fd, &bind, sizeof bind, buf, sizeof buf, "bind");

   bindack = (dl_bind_ack_t *)buf;
   if (bindack->dl_primitive != DL_BIND_ACK)
      errorack(buf, "cannot bind");

   /* 
    * Save the source address in the event the packet format has been set to
    * NS_INCLUDE_MAC.  We will generically determine the address length below
    * using the sap length returned in the dl_info_ack_t.
    */

   src_addr_sap_len = bindack->dl_addr_length;
   bcopy(buf + bindack->dl_addr_offset, src_address, bindack->dl_addr_length);

   printf("%s: bound to:\n", Prog);
   printf("  sap       = %x\n", bindack->dl_sap);
   printf("  address   = ");
   p = buf + bindack->dl_addr_offset;
   for (i = bindack->dl_addr_length; i--;)
      printf("%02x:", *p++);
   printf("\n");

   /*
    * get info
    */
   
   info.dl_primitive   = DL_INFO_REQ;

   (void)request(fd, &info, sizeof info, buf, sizeof buf, "info");

   infoack = (dl_info_ack_t *)buf;
   if (infoack->dl_primitive != DL_INFO_ACK)
      errorack(buf, "cannot get info");

   /*
    * Save the sap length from the DL_INFO_ACK so that we can
    * generically determine the source address length.  This is
    * used if a packet format other than NS_PROTO is specified.
    */
 
   saplen = infoack->dl_sap_length;

/*
   printf("%s: provider info:\n", Prog);
   printf("  max sdu       = %d\n", infoack->dl_max_sdu);
   printf("  min sdu       = %d\n", infoack->dl_min_sdu);
   printf("  mac type      = %d\n", infoack->dl_mac_type);
   printf("  addr length   = %d\n", infoack->dl_addr_length);
   printf("  current state = %d\n", infoack->dl_current_state);
   printf("  service mode  = %d\n", infoack->dl_service_mode);
   printf("\n");
*/

   /*
    * client request
    */

   while (count--) {
      struct strbuf ctl, dat;
      struct pollfd fds;
      int nfds;

      udreq = (dl_unitdata_req_t *)buf;
      udreq->dl_primitive = DL_UNITDATA_REQ;
      udreq->dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
      udreq->dl_priority.dl_min = udreq->dl_priority.dl_max = 0;

      /* 
       * Build the header as specified by the packet format.
       * NOTE THAT THESE EXAMPLES ARE FOR STANDARD ETHERNET!
       */

      switch (pkt_format) {
	case NS_PROTO:
      		udreq->dl_dest_addr_length = 6;
      		memcpy(buf + udreq->dl_dest_addr_offset, address,
               		udreq->dl_dest_addr_length);

      		/*
       		 * print out the header
       		 */

      		p=buf+udreq->dl_dest_addr_offset;
      		for (i=udreq->dl_dest_addr_length; i--;)
         		printf("%02x:", *p++);
      		printf("\n");

      		ctl.len = sizeof(dl_unitdata_req_t)+6;
      		data[0] = 'q';
      		data[1] = 0;
		break;

	case NS_PROTO_SNAP:
         	die("NS_PROTO_SNAP is not applicable to standard ethernet");

	case NS_INCLUDE_LLC:
         	die("NS_INCLUDE_LLC is not applicable to standard ethernet");

	case NS_INCLUDE_MAC:
      		/* 
       		 * Build the MAC header for standard ethernet.  You need to
		 * know what interface type you are using so that you can
		 * build the proper MAC header.
       		 */

      		udreq->dl_dest_addr_length = 0;
      		/* place the destination address in the data portion */
      		p = data;
      		bcopy(address, p, 6);
      		/* place the source address and sap in the data portion */
      		p += 6;
      		bcopy(src_address, p, src_addr_sap_len);	
		/* copy in the data */
		p += src_addr_sap_len;
      		memset(p, 'q', 1);	
		
      		/*
       		 * print out the MAC header
       		 */

      		p=data;
      		for (i=6+src_addr_sap_len; i--;)
         		printf("%02x:", *p++);
      		printf("\n");

      		ctl.len = sizeof(dl_unitdata_req_t);
		break;
      }

      ctl.buf = buf;
      dat.buf = data;
      dat.len = 100;
      dat.maxlen = sizeof data;

      if (putmsg(fd, &ctl, &dat, 0))
         die("putmsg");

      ctl.maxlen = sizeof buf;
      ctl.buf = buf;
      dat.maxlen = sizeof data;
      dat.buf = data;

      fds.fd = fd;
      fds.events = POLLIN | POLLPRI;
      if ((nfds = poll(&fds, 1, 5000)) < 0)   /* 5 seconds */
         die("poll");
      
      if (nfds) {
         static dl_unitdata_ind_t *udi;
         static dl_uderror_ind_t *err;
         extern int errno;

         if (fds.revents & (POLLERR | POLLHUP)) {
            printf("poll revents = %x\n", fds.revents);
            break;
         }

         if (getmsg(fd, &ctl, &dat, &flags) != 0)
            die("getmsg");

         udi = (dl_unitdata_ind_t *)ctl.buf;
         if (udi->dl_primitive != DL_UNITDATA_IND) {
            err = (dl_uderror_ind_t *)ctl.buf;
            printf("primitive = %d\n", err->dl_primitive);
            printf("addr len  = %d\n", err->dl_dest_addr_length);
            printf("addr off  = %d\n", err->dl_dest_addr_offset);
            printf("unixerrno = %d\n", err->dl_unix_errno);
            printf("errno     = %d\n", err->dl_errno);
            errno = err->dl_unix_errno;
            die("dlpi error");
         }

         if (dat.len)
            num_pack++;
            /* printf("unitdata_ind: data = <%s>\n", dat.buf); */
      } else
        printf("cdlpi: no response (dropped %d) in loop <%d>\n",
                       ++total, icnt-count);
   }

   printf("total number of packets sent  = %d\n", icnt);
   printf("total number of acks received = %d\n", num_pack);
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

/*
 * request - make a DLPI request
 *
 *   returns length of user data
 *   stores proto data in res, user data in data[]
 */

int
request(fd, req, req_len, res, res_len, foo)
   int fd;
   caddr_t req;
   int req_len;
   caddr_t res;
   int res_len;
   char *    foo;
{
   struct strbuf ctl, dat;
   int flags = 0;

   ctl.buf = req;
   ctl.len = req_len;
   dat.len = 0;
   dat.buf = data;

   if (putmsg(fd, &ctl, &dat, 0))
      die("putmsg - %s", foo);

   ctl.buf    = res;
   ctl.maxlen = res_len;
   dat.len    = 0;
   dat.buf    = data;
   dat.maxlen = sizeof data;

   if (getmsg(fd, &ctl, &dat, &flags) != 0)
      die("getmsg - %s", foo);
   
   return dat.len;
}

char *
conv(cp)
   char *cp;
{
   static char buf[100];
   char *addr;
   char *p;

   addr = buf;
   for (p = strtok(cp, ":"); p; p = strtok(0, ":"))
      *addr++ = strtoul(p, 0, 16);
   return buf;
}

die(s, a, b, c)
   char *s;
{
   char buf[256];
   sprintf(buf, s, a, b, c);
   perror(buf);
   exit(0);
}

usage()
{
   fprintf(stderr, "usage: %s -d Destination -n Node -s SAP [-c Count] [-f PktFormat]\n", Prog);
   exit(1);
}

