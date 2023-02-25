static char sccsid[] = "@(#)40  1.13.1.8  src/bos/usr/sbin/bootpd/bootpd.c, cmdnet, bos411, 9428A410j 5/27/94 14:44:52";
/*
** COMPONENT_NAME: (CMDNET) bootpd.c
**
** FUNCTIONS:
**
**	main(argc, argv)
**	int chk_access(path, filesize)
**	void dovend_cmu(bp, hp)
**	void dovend_rfc1048(bp, hp, hp1, bootsize)
**	void dump_host(fp, hp)
**	dumptab()
**	char *get_errmsg()
**	char *haddrtoa(haddr, htype)
**	boolean hwlookcmp(host1, host2)
**	void insert_generic(gendata, buff, bytesleft)
**	void insert_ip(tag, iplist, dest, bytesleft)
**	void insert_u_long(value, dest)
**	boolean iplookcmp(host1, host2)
**	void list_ipaddresses(fp, ipptr)
**	static nmatch(ca,cb)
**	reply()
**	void report(priority, fmt, p0, p1, p2, p3, p4)
**	request()
**	sendreply(forward)
**	setarp(ia, ha, len,type)
**	void usage()
**
** ORIGINS: 6,44,27
**
**   (C) COPYRIGHT International Business Machines Corp. 1991
**   All Rights Reserved
**   Licensed Materials - Property of IBM
**   US Government Users Restricted Rights - Use, duplication or
**   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
**
** Permission to use, copy, modify, and distribute this program for any
** purpose and without fee is hereby granted, provided that this copyright,
** permission and disclaimer notice appear on all copies and supporting
** documentation; the name of IBM not be used in advertising or publicity
** pertaining to distribution of the program without specific prior
** permission; and notice be given in supporting documentation that copying
** and distribution is by permission of IBM.  IBM makes no representations
** about the suitability of this software for any purpose.  This program
** is provided "as is" without any express or implied warranties, including,
** without limitation, the implied warranties of merchantability and fitness
** for a particular purpose.
**
** US Government Users Restricted Rights - Use, duplication or
** disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
**
** (C) COPYRIGHT Advanced Graphics Engineering 1990
** All Rights Reserved
*/

/*
 * BOOTP (bootstrap protocol) server daemon.
 *
 * Answers BOOTP request packets from booting client machines.
 * See [SRI-NIC]<RFC>RFC951.TXT for a description of the protocol.
 * See [SRI-NIC]<RFC>RFC1048.TXT for vendor-information extensions.
 * See accompanying man page -- bootpd.8
 *
 *
  Copyright 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon and Stanford
 * University.  Carnegie Mellon makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 *
 * HISTORY
 *
 * 01/22/86     Bill Croft at Stanford University
 *                  Created.
 *
 * 07/30/86     David Kovar at Carnegie Mellon University
 *                  Modified to work at CMU.
 *
 * 07/24/87     Drew D. Perkins at Carnegie Mellon University
 *                  Modified to use syslog instead of Kovar's
 *                  routines.  Add debugging dumps.  Many other fixups.
 *
 * 07/15/88     Walter L. Wimer at Carnegie Mellon University
 *                  Added vendor information to conform to RFC1048.
 *                  Adopted termcap-like file format to support above.
 *                  Added hash table lookup instead of linear search.
 *                  Other cleanups.
 *
 *
 * BUGS
 *
 * Currently mallocs memory in a very haphazard manner.  As such, most of
 * the program ends up core-resident all the time just to follow all the
 * stupid pointers around. . . .
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if_types.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef AIX32
#define _AIXVER3
#define _FIELD_SALEN
#endif

#ifdef AIX31
#define _AIXVER3
#include <netinet/in_netarp.h>
#endif

#ifdef _AIXVER3
#define _FIELD_IFTYPE
#define _USE_NLS
#define _USE_SYSLOG
#define _USE_DEBUG
#endif

#ifdef _INC_SOCKIO_H
#include <sys/sockio.h>
#endif

#ifdef _INC_IFARP_H
#include <net/if_arp.h>
#endif

/*
 *  AIXBUILD defined when building ver 3.2 bootpd in /usr/sbin
 */
#ifdef AIXBUILD
#include <jctype.h>
#endif

#ifdef _USE_SYSLOG
#include <sys/syslog.h>
#else
#define LOG_INFO	6
#define LOG_ERR		3
#define LOG_NOTICE	5
#endif

#ifdef _USE_NLS
#include <locale.h>			/* included for nls support */
#endif

#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

#include "bootp.h"
#include "hash.h"
#include "bootpd.h"


#define HASHTABLESIZE           257     /* Hash table size (prime) */
#define DEFAULT_TIMEOUT          15L    /* Default timeout in minutes */

#define MAX_SECONDARY_DELAY     60     /* maximum number of secondsto delay before responding to next boot request */
/*
 * change definition of SIOCSARP_802_5 if not defined in ioctl.h
 */
#ifndef SIOCSARP_802_5
#define SIOCSARP_802_5 SIOCSARP_TOKEN
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE             "/etc/bootptab"
#endif
#ifndef DUMP_FILE
#define DUMP_FILE               "/etc/bootpd.dump"
#endif

#define satosin(sa)	((struct sockaddr_in *)(sa))


/*
 * Externals, forward declarations, and global variables
 */

extern char Version[];
extern char *sys_errlist[];
extern int  errno, sys_nerr;

void usage();
void insert_u_long();
void dump_host();
void list_ipaddresses();
#ifdef _USE_CMUVEND
void dovend_cmu();
#endif
void dovend_rfc1048();
boolean hwlookcmp();
boolean iplookcmp();
void insert_generic();
void insert_ip();
#ifdef _USE_DEBUG
int dumptab();
#endif
int chk_access();
void report();
char *get_errmsg();


/*
 * IP port numbers for client and server obtained from /etc/services
 */

u_short bootps_port, bootpc_port;


/*
 * Internet socket and interface config structures
 */

struct sockaddr_in sin; 
struct sockaddr_in from;        /* Packet source */
char ifcbuf[2048];      	/* Holds interface configuration */
struct ifconf ifconf;           /* Int. config ioctl block (pnts to ifreq) */
int	netmask;
struct arpreq arpreq;           /* Arp request ioctl block */


/*
 * General
 */
int debug = 0;                      /* Debugging flag */
int s;                              /* Socket file descriptor */
byte buf[1024];                     /* Receive packet buffer */
struct timezone tzp;                /* Time zone offset for clients */
struct timeval tp;                  /* Time (extra baggage) */
long secondswest;                   /* Time zone offset in seconds */

/*
 * Globals below are associated with the bootp database file (bootptab).
 */

char *bootptab = NULL;
#ifdef _USE_DEBUG
char *bootpd_dump = NULL;
#endif



/*
 * Vendor magic cookies for CMU and RFC1048
 */

unsigned char vm_cmu[4]     = VM_CMU;
unsigned char vm_rfc1048[4] = VM_RFC1048;


/*
 * Hardware address lengths (in bytes) based on hardware type code.
 * List in order specified by Assigned Numbers RFC; Array index is
 * hardware type code.  Entries marked as zero are unknown to the author
 * at this time. . . .
 */

unsigned maphaddrlen[MAXHTYPES + 1] = {
     0,     /* Type 0:  Reserved (don't use this)               */
     6,     /* Type 1:  10Mb Ethernet (48 bits)                 */
     1,     /* Type 2:   3Mb Ethernet (8 bits)                  */
     0,     /* Type 3:  Amateur Radio AX.25                     */
     1,     /* Type 4:  Proteon ProNET Token Ring               */
     0,     /* Type 5:  Chaos                                   */
     6,     /* Type 6:  IEEE 802 Networks                       */
     0,     /* Type 7:  ARCNET                                  */
     0,     /* Type 8:  Hyperchannel                            */
     0,     /* Type 9:  Lanstar                                 */
     0,     /* Type 10: Autonet Short Address                   */
     0,     /* Type 11: LocalTalk                               */
     0,     /* Type 12: LocalNet (IBM PCNet or SYSTEK LocalNET  */
     0,     /* Type 13: Ultra Link                              */
     0,     /* Type 14: SMDS                                    */
     0,     /* Type 15: Frame Relay                             */
     0,     /* Type 16: Asynchronous Transmission Mode (ATM)    */
     6,     /* Type 17: IEEE 802.3 Network                      */
     6,     /* Type 18: FDDI                                    */
};


/*
 * Main hash tables
 */

hash_tbl *hwhashtable;
hash_tbl *iphashtable;
hash_tbl *nmhashtable;




/*
 * Initialization such as command-line processing is done and then the main
 * server loop is started.
 */
extern char **environ;
main(argc, argv)
    int argc;
    char **argv;
{
    struct timeval actualtimeout, *timeout;
    struct bootp *bp = (struct bootp *) buf;
    struct servent *servp;
    char *stmp;
    int n, tolen, fromlen;
    int nfound, readfds;
    int standalone;
#ifdef _USE_MAXDEBUG
    int i;                             /* Used by Alex's print routine */
    struct bootp *b;                   /* Points to the buffer we just got */
#endif
#ifdef _USE_NLS
    (void)setlocale(LC_ALL, "");	/* included for nls support */
#endif
    stmp = NULL;
    standalone = FALSE;
    actualtimeout.tv_usec = 0L;
    actualtimeout.tv_sec  = 60 * DEFAULT_TIMEOUT;
    timeout = &actualtimeout;

    /*
     * Read switches.
     */
    for (argc--, argv++; argc > 0; argc--, argv++) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
                case 't':
                    if (argv[0][2]) {
                        stmp = &(argv[0][2]);
                    } else {
                        argc--;
                        argv++;
                        stmp = argv[0];
                    }
                    if (!stmp || (sscanf(stmp, "%d", &n) != 1) || (n < 0)) {
                        fprintf(stderr,
                                "bootpd: invalid timeout specification\n");
                        break;
                    }
                    actualtimeout.tv_sec = (long) (60 * n);
                    /*
                     * If the actual timeout is zero, pass a NULL pointer
                     * to select so it blocks indefinitely, otherwise,
                     * point to the actual timeout value.
                     */
                    timeout = (n > 0) ? &actualtimeout : NULL;
                    break;
                case 'd':
                    debug++;
                    break;
                case 's':
                    standalone = TRUE;
                    break;
                default:
                    fprintf(stderr,
                            "bootpd: unknown switch: -%c\n",
                            argv[0][1]);
                    usage();
                    break;
            }
        } else {
            if (!bootptab) {
                bootptab = argv[0];
#ifdef _USE_DEBUG
            } else if (!bootpd_dump) {
                bootpd_dump = argv[0];
#endif
            } else {
                fprintf(stderr, "bootpd: unknown argument: %s\n", argv[0]);
                usage();
            }
        }
    }

    /*
     * Set default file names if not specified on command line
     */
    if (!bootptab) {
        bootptab = CONFIG_FILE;
    }
#ifdef _USE_DEBUG
    if (!bootpd_dump) {
        bootpd_dump = DUMP_FILE;
    }
#endif

    /*
     * Get our timezone offset so we can give it to clients if the
     * configuration file doesn't specify one.
     */
    if (gettimeofday(&tp, &tzp) < 0) {
        secondswest = 0L;       /* Assume GMT for lack of anything better */
        report(LOG_ERR, "gettimeofday: %s\n", get_errmsg());
    } else {
        secondswest = 60L * tzp.tz_minuteswest;     /* Convert to seconds */
    }

    /*
     * Allocate hash tables for hardware address, ip address, and hostname
     */
    hwhashtable = hash_Init(HASHTABLESIZE);
    iphashtable = hash_Init(HASHTABLESIZE);
    nmhashtable = hash_Init(HASHTABLESIZE);
    if (!(hwhashtable && iphashtable && nmhashtable)) {
        fprintf(stderr, "Unable to allocate hash tables.\n");
        exit(1);
    }

    if (standalone) {
        /*
         * Go into background and disassociate from controlling terminal.
         */
        if (debug < 3) {
            if (fork())
                exit(0);
            for (n = 0; n < 10; n++)
                (void) close(n);
            (void) open("/", O_RDONLY);
            (void) dup2(0, 1);
            (void) dup2(0, 2);
            n = open("/dev/tty", O_RDWR);
            if (n >= 0) {
                ioctl(n, TIOCNOTTY, (char *) 0);
                (void) close(n);
            }
        }
        /*
         * Nuke any timeout value
         */
        timeout = NULL;
    }


#ifdef _USE_SYSLOG
    /*
     * Initialize logging.
     */
#ifndef LOG_CONS
#define LOG_CONS        0       /* Don't bother if not defined... */
#endif
#ifndef LOG_DAEMON
#define LOG_DAEMON      0
#endif
    openlog("bootpd", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "%s", Version);
#endif

    if (standalone) {
        /*
         * Get us a socket.
         */
        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            report(LOG_ERR, "socket: %s\n", get_network_errmsg());
            exit(1);
        }

        /*
         * Get server's listening port number
         */
        servp = getservbyname("bootps", "udp");
        if (servp) {
            bootps_port = ntohs((u_short) servp->s_port);
        } else {
            report(LOG_ERR,
                   "udp/bootps: unknown service -- assuming port %d\n",
                   IPPORT_BOOTPS);
            bootps_port = (u_short) IPPORT_BOOTPS;
        }

        /*
         * Bind socket to BOOTPS port.
         */
#ifdef _FIELD_SALEN
	sin.sin_len = sizeof(sin);
#endif
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(bootps_port);
        if (bind(s, &sin, sizeof(sin)) < 0) {
	    perror("bind failed:");
            report(LOG_ERR, "bind: %s\n", get_network_errmsg());
            exit(1);
        }
    } else {
        /*
         * Assume socket was passed to us from inetd
         */
        s = 0;
        tolen = sizeof(sin);
        bzero((char *) &sin, tolen);
        if (getsockname(s, &sin, &tolen) < 0) {
            report(LOG_ERR, "getsockname: %s\n", get_network_errmsg());
            exit(1);
        }
        bootps_port = ntohs(sin.sin_port);
    }

    /*
     * Get destination port number so we can reply to client
     */
    servp = getservbyname("bootpc", "udp");
    if (servp) {
        bootpc_port = ntohs(servp->s_port);
    } else {
        report(LOG_ERR,
               "udp/bootpc: unknown service -- assuming port %d\n",
                IPPORT_BOOTPC);
        bootpc_port = (u_short) IPPORT_BOOTPC;
    }


    /*
     * Determine network configuration.
     */
    ifconf.ifc_len = sizeof(ifcbuf);
    ifconf.ifc_buf = ifcbuf;
    if ((ioctl(s, SIOCGIFCONF, (caddr_t) &ifconf) < 0) ||
        (ifconf.ifc_len <= 0)) {
            report(LOG_ERR, "ioctl: %s\n", get_network_errmsg());
            exit(1);
    }

    /*
     * Read the bootptab file once immediately upon startup.
     */
    readtab();

    /*
     * Set up signals to read or dump the table.
     */
    if ((int) signal(SIGHUP, readtab) < 0) {
        report(LOG_ERR, "signal: %s\n", get_errmsg());
        exit(1);
    }
#ifdef _USE_DEBUG
    if ((int) signal(SIGUSR1, dumptab) < 0) {
        report(LOG_ERR, "signal: %s\n", get_errmsg());
        exit(1);
    }
    dumptab();
#endif

    /*
     * Process incoming requests.
     */
    for (;;) {
        readfds = 1 << s;
        nfound = select(s + 1, &readfds, NULL, NULL, timeout);
        if (nfound < 0) {
            if (errno != EINTR) {
                report(LOG_ERR, "select: %s\n", get_errmsg());
            }
            continue;
        }
        if (!(readfds & (1 << s))) {
            report(LOG_INFO, "exiting after %ld minutes of inactivity\n",
                   actualtimeout.tv_sec / 60);
            exit(0);
        }
        fromlen = sizeof(from);
        n = recvfrom(s, buf, sizeof(buf), 0, &from, &fromlen);
        if (n <= 0) {
            continue;
        }

        if (n < sizeof(struct bootp)) {
                report(LOG_NOTICE, "received short packet\n");
            continue;
        }
/* Start of display Alex's display routine                     */
#ifdef _USE_MAXDEBUG
       b = (struct bootp *)buf;

       report(LOG_INFO,"%d: unsigned char   bp_op;   /* packet opcode type */\n",
                                b->bp_op);
       report(LOG_INFO,"%d: unsigned char   bp_htype;/* hardware addr type */\n",
                                b->bp_htype);
       report(LOG_INFO,"%d: unsigned char   bp_hlen; /* hardware addr length */\n",
                                b->bp_hlen);
       report(LOG_INFO,"%d: unsigned char   bp_hops; /* gateway hops */\n",
                                b->bp_hops);
       report(LOG_INFO,"%d: unsigned long   bp_xid;  /* transaction ID */\n",
                                b->bp_xid);
       report(LOG_INFO,"%d: unsigned short  bp_secs; /* seconds since boot began */\n",
                                b->bp_secs);
       report(LOG_INFO,"%d: unsigned short  bp_unused;\n",
                                b->bp_unused);
       report(LOG_INFO,"%d: in_name bp_ciaddr;      /* client IP address */\n",
                               b->bp_ciaddr);
       report(LOG_INFO,"%d: in_name bp_yiaddr;      /* 'your' IP address */\n",
                               b->bp_yiaddr);
       report(LOG_INFO,"%d: in_name bp_siaddr;      /* server IP address */\n",
                               b->bp_siaddr);
       report(LOG_INFO,"%d: in_name bp_giaddr;      /* gateway IP address */\n",
                               b->bp_giaddr);

       report(LOG_INFO,"unsigned char   bp_chaddr[16];  /* client hardware address */\n");
        for(i = 0; i < 16; i++)
               report(LOG_INFO,"%d.",b->bp_chaddr[i]);
       report(LOG_INFO,"\n");
       report(LOG_INFO,"unsigned char   bp_sname[64];   /* server host name */\n");
        for(i = 0; i < 64; i++)
               report(LOG_INFO,"%d.",b->bp_sname[i]);
       report(LOG_INFO,"\n");
       report(LOG_INFO,"unsigned char   bp_file[128];   /* boot file name */\n");
        for(i = 0; i < 128; i++)
              report(LOG_INFO,"%d.",b->bp_file[i]);
       report(LOG_INFO,"\n");
       report(LOG_INFO,"unsigned char   bp_vend[64];    /* vendor-specific area */\n");
        for(i = 0; i < 64; i++)
               report(LOG_INFO,"%d.",b->bp_vend[i]);
       report(LOG_INFO,"\n");
#endif
        readtab();      /* maybe re-read bootptab */
        switch (bp->bp_op) {
            case BOOTREQUEST:
		if (debug) {
			report(LOG_INFO,"Received boot request. \n");
		}
                request();
		if (debug) {
			report(LOG_INFO,"Finished processing boot request.\n");
		}
                break;

            case BOOTREPLY:
                reply();
                break;
        }
    }
}




/*
 * Print "usage" message and exit
 */

void usage()
{
    fprintf(stderr,
           "usage:  bootpd [-d] [-s] [-t timeout] [configfile [dumpfile]]\n");
    fprintf(stderr, "\t -d\tincrease debug verbosity\n");
    fprintf(stderr, "\t -s\trun standalone (without inetd)\n");
    fprintf(stderr, "\t -t n\tset inetd exit timeout to n minutes\n");
    exit(1);
}



/*
 * Process BOOTREQUEST packet.
 *
 * (Note, this version of the bootpd.c server never forwards
 * the request to another server.  In our environment the
 * stand-alone gateways perform that function.)
 *
 * (Also this version does not interpret the hostname field of
 * the request packet;  it COULD do a name->address lookup and
 * forward the request there.)
 */
request()
{
    register struct bootp *bp = (struct bootp *) buf;
    register struct host *hp;
    register struct host *hp1 = NULL;		/*alt*/
    register int n;
    char path[128];
    struct host dummyhost;
    long bootsize;
    unsigned hlen, hashcode;
    int delay_seconds;
    int dbxbreakpt;

    bp->bp_op = BOOTREPLY;
    if (bp->bp_ciaddr.s_addr == 0) {
        /*
         * client doesnt know his IP address,
         * search by hardware address.
         */
        if (debug) {
            report(LOG_INFO, "request from hardware address %s\n",
			haddrtoa(bp->bp_chaddr, bp->bp_htype));
        }

        dummyhost.htype = bp->bp_htype;
        hlen = haddrlength(bp->bp_htype);
        bcopy(bp->bp_chaddr, dummyhost.haddr, hlen);
        hashcode = hash_HashFunction(bp->bp_chaddr, hlen);
        hp = (struct host *) hash_Lookup(hwhashtable, hashcode, hwlookcmp,
					 &dummyhost);
	if((hp != NULL) && (bp->bp_htype == HTYPE_ETHERNET)){	/*alt*/
		dummyhost.htype = HTYPE_IEEE8023;	/*alt*/
        	hp1 = (struct host *) hash_Lookup(	/*alt*/
		hwhashtable, hashcode, hwlookcmp, &dummyhost);/*alt*/
	}						/*alt*/
		
	if((hp == NULL) && (bp->bp_htype == HTYPE_IEEE802)){	/*8023*/
		bp->bp_htype = HTYPE_IEEE8023;
		dummyhost.htype = bp->bp_htype;			/*8023*/
        	hp = (struct host *) hash_Lookup(hwhashtable, hashcode,/*8023*/
					 hwlookcmp, &dummyhost);/*8023*/
		if (hp != NULL){				/*alt*/
			dummyhost.htype = HTYPE_ETHERNET;	/*alt*/
        		hp1 = (struct host *) hash_Lookup(	/*alt*/
			hwhashtable, hashcode, hwlookcmp, &dummyhost);/*alt*/
		}						/*alt*/
	}							/*8023*/
	if (( hp == NULL) && (bp->bp_htype == HTYPE_ETHERNET )) {
		bp->bp_htype = HTYPE_FDDI;
		dummyhost.htype = bp->bp_htype;
        	hp = (struct host *) hash_Lookup(hwhashtable, hashcode, 
						 hwlookcmp, &dummyhost);
	}
        if (hp == NULL) {
            report(LOG_NOTICE, "hardware address not found: %s\n",
                    haddrtoa(bp->bp_chaddr, bp->bp_htype));
            return;     /* not found */
	}
        (bp->bp_yiaddr).s_addr = hp->iaddr.s_addr;

    } else {

        /*
         * search by IP address.
         */
        if (debug) {
            report(LOG_INFO, "request from IP addr %s\n",
                    inet_ntoa(bp->bp_ciaddr));
        }
        dummyhost.iaddr.s_addr = bp->bp_ciaddr.s_addr;
        hashcode = hash_HashFunction((char *) &(bp->bp_ciaddr.s_addr), 4);
        hp = (struct host *) hash_Lookup(iphashtable, hashcode, iplookcmp,
                                         &dummyhost);
        if (hp == NULL) {
            report(LOG_NOTICE,
                    "IP address not found: %s\n", inet_ntoa(bp->bp_ciaddr));
            return;
        }
    }

    /* This is the partial fix for booting off  */
    /* the 802.3 ethernet.			*/
    if (bp->bp_htype != hp->htype) {
        bp->bp_htype = hp->htype;
    }

#if   defined(FEATURE)
 /*
  * If the host is not a primary boot host, then delay before
  * filling the boot request. The amount of delay time is
  * defined in the bootptab file.
  */
     if ( delay_seconds=chk_secondary(hp)) /* if secondary boot host */
     {
              delay_seconds = ((delay_seconds < MAX_SECONDARY_DELAY) ?
                      delay_seconds : MAX_SECONDARY_DELAY);
	      if ( bp->bp_secs < delay_seconds )
			return;
     }
#endif

    if (debug) {
        report(LOG_INFO, "found %s %s\n", inet_ntoa(hp->iaddr),
                hp->hostname);
    }
    /*
     * If a specific TFTP server address was specified in the bootptab file,
     * fill it in, otherwise zero it.
     */
    (bp->bp_siaddr).s_addr = (hp->flags.bootserver) ?
	hp->bootserver.s_addr : 0L;
    (bp->bp_giaddr).s_addr = (hp->flags.gateway) ?
	hp->gateway->addr[0].s_addr : 0L;
    /*
     * Fill in the client's proper bootfile.
     *
     * If the client specifies an absolute path, try that file with a
     * ".host" suffix and then without.  If the file cannot be found, no
     * reply is made at all.
     *
     * If the client specifies a null or relative file, use the following
     * table to determine the appropriate action:
     *
     *  Homedir      Bootfile    Client's file
     * specified?   specified?   specification   Action
     * -------------------------------------------------------------------
     *      No          No          Null         Send null filename
     *      No          No          Relative     Discard request
     *      No          Yes         Null         Send if absolute else null
     *      No          Yes         Relative     Discard request
     *      Yes         No          Null         Send null filename
     *      Yes         No          Relative     Lookup with ".host"
     *      Yes         Yes         Null         Send home/boot or bootfile
     *      Yes         Yes         Relative     Lookup with ".host"
     *
     */

    /*
     * Who knows. . . . . vestigial code (from Stanford???) which should
     * probably be axed.
     */
    if (strcmp(bp->bp_file, "sunboot14") == 0)
        bp->bp_file[0] = 0;     /* pretend it's null */

    /*
     * This is the code which actually implements the above truth table.
     */
    if (bp->bp_file[0]) {
        /*
         * The client specified a file.
         */
        if (bp->bp_file[0] == '/') {
            strcpy(path, bp->bp_file);          /* Absolute pathname */
        } else {
            if (hp->flags.homedir) {
                strcpy(path, hp->homedir);
                strcat(path, "/");
                strcat(path, bp->bp_file);
            } else {
                report(LOG_NOTICE,
                    "requested file \"%s\" not found: hd unspecified\n",
                    bp->bp_file);
                return;
            }
        }
    } else {
        /*
         * No file specified by the client.
         */
	dbxbreakpt = 0;
        if (hp->flags.bootfile & (*(hp->bootfile) == '/')) {
            strcpy(path, hp->bootfile);
        } else if (hp->flags.homedir && hp->flags.bootfile) {
            strcpy(path, hp->homedir);
            strcat(path, "/");
           strcat(path, hp->bootfile);
        } else {
            bzero(bp->bp_file, sizeof(bp->bp_file));
            goto skip_file;     /* Don't bother trying to access the file */
        }
    }

    /*
     * First try to find the file with a ".host" suffix
     */
    n = strlen(path);
    strcat(path, ".");
    strcat(path, hp->hostname);
    if (chk_access(path, &bootsize) < 0) {
        path[n] = 0;                    /* Try it without the suffix */
        if (chk_access(path, &bootsize) < 0) {
            if (bp->bp_file[0]) {
                /*
                 * Client wanted specific file
                 * and we didn't have it.
                 */
                report(LOG_NOTICE,
                        "requested file not found: \"%s\"\n", path);
                return;
            } else {
                /*
                 * Client didn't ask for a specific file and we couldn't
                 * access the default file, so just zero-out the bootfile
                 * field in the packet and continue processing the reply.
                 */
                bzero(bp->bp_file, sizeof(bp->bp_file));
                goto skip_file;
            }
        }
    }
    strcpy(bp->bp_file, path);
    if (debug)
	report(LOG_INFO, "bootfile = %s\n", bp->bp_file);

skip_file:  ;


    /*
     * Zero the vendor information area, except for the magic cookie.
     */
    bzero((bp->bp_vend) + 4, (sizeof(bp->bp_vend)) - 4);

    if (debug) {
        report(LOG_INFO, "vendor magic field is %d.%d.%d.%d\n",
                (int) ((bp->bp_vend)[0]),
                (int) ((bp->bp_vend)[1]),
                (int) ((bp->bp_vend)[2]),
                (int) ((bp->bp_vend)[3]));
    }

    /*
     * If this host isn't set for automatic vendor info then copy the
     * specific cookie into the bootp packet, thus forcing a certain
     * reply format.
     */
    if (!hp->flags.vm_auto) {
        bcopy(hp->vm_cookie, bp->bp_vend, 4);
    }

    /*
     * Figure out the format for the vendor-specific info.
     */
    if (bcmp(bp->bp_vend, vm_rfc1048, 4)) {
        /* Not an RFC1048 bootp client */
#ifdef _USE_CMUVEND
        dovend_cmu(bp, hp);
#else
        dovend_rfc1048(bp, hp, hp1, bootsize);
#endif
    } else {
        /* RFC1048 conformant bootp client */
        dovend_rfc1048(bp, hp, hp1, bootsize);
    }

    if (hp->flags.dumb_terminal == TRUE) {
        sendreply(0,TRUE);
    }
    else {
        sendreply(0,FALSE);
    }
}



/*
 * This call checks read access to a file.  It returns 0 if the file given
 * by "path" exists and is publically readable.  A value of -1 is returned if
 * access is not permitted or an error occurs.  Successful calls also
 * return the file size in bytes using the long pointer "filesize".
 *
 * The read permission bit for "other" users is checked.  This bit must be
 * set for tftpd(8) to allow clients to read the file.
 */

int chk_access(path, filesize)
char *path;
long *filesize;
{
    struct stat buf;

    if ((stat(path, &buf) == 0) && (buf.st_mode & (S_IREAD >> 6))) {
        *filesize = (long) buf.st_size;
        return 0;
    } else {
        return -1;
    }
}



/*
 * Process BOOTREPLY packet (something is using us as a gateway).
 */

reply()
{
        if (debug) {
            report(LOG_INFO, "processing boot reply\n");
        }
        sendreply(1,FALSE);
}

/*
 *  ASSUMES  global struct ifconf ifconf, global socket s
 *  return number of identical bytes in netnumber of dst and "best" interface
 */
int
getbestif(dst, ifrmax)
	struct in_addr	*dst;
	struct ifreq **ifrmax;
{
	char 		*p, *lastbyte;
	struct ifreq	ifrq, *pifr;
	int		netaddr, dstaddr, nbytes, maxb;

	maxb = 0;
	*ifrmax = ifconf.ifc_req;
	lastbyte = ifconf.ifc_buf + ifconf.ifc_len;
	
#ifdef _FIELD_SALEN
#define max(a,b)	((a) < (b) ? (b) : (a))
#define sasize(p)	max((p).sa_len, sizeof(p))
	for(p = ifconf.ifc_buf;
	    p < lastbyte;
	    p += sizeof(pifr->ifr_name) + sasize(pifr->ifr_addr))
#else
	for(p = ifconf.ifc_buf; p < lastbyte; p += sizeof(*pifr))
#endif
	{
		pifr = (struct ifreq *)p;
		/*
		 *  if not AF_NET, skip it
		 */
		if(pifr->ifr_addr.sa_family != AF_INET)
			continue;
		ifrq = *pifr;
		/*
		 *  make sure interface is up and running
		 */
		if(ioctl(s, SIOCGIFFLAGS, (char *)&ifrq) < 0)
		{
			perror("ioctl(SIOCGIFFLAGS)");
			ifrq.ifr_flags = 0xFFFFFFFF;
		}
		if((ifrq.ifr_flags & IFF_UP) &&
		   (ifrq.ifr_flags & IFF_RUNNING))
		{
			netaddr = satosin(&ifrq.ifr_addr)->sin_addr.s_addr;
			dstaddr = dst->s_addr;

			if(ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifrq) >= 0)
			{
				netmask = satosin(&ifrq.ifr_addr)->sin_addr.s_addr;
				netaddr &= netmask;
				dstaddr &= netmask;
			}
			else
				perror("ioctl(SIOCGIFNETMASK)");
			if((nbytes = nmatch(&dstaddr, &netaddr)) >= maxb)
			{
				maxb = nbytes;
				*ifrmax = pifr;
			}
		}
	}
	return maxb;
}


/*
 * Send a reply packet to the client.  'forward' flag is set if we are
 * not the originator of this reply packet.
 */
sendreply(forward, rtinfo)
    int forward;
    int rtinfo;
{
        register struct bootp *bp = (struct bootp *) buf;
        struct in_addr dst, client_s_addr;
        struct sockaddr_in to;
	struct ifreq	*ifrbest;
	int	ret;
	int	samesubnet = 1;
        struct  arpreq *retarp, *setarp();

        retarp = (struct arpreq *) NULL;
        to = sin;

        to.sin_port = htons(bootpc_port);
        /*
         * If the client IP address is specified, use that
         * else if gateway IP address is specified, use that
         * else make a temporary arp cache entry for the client's NEW
         * IP/hardware address and use that.
         */
        if (bp->bp_ciaddr.s_addr) {
                dst = bp->bp_ciaddr;
        } else if ((bp->bp_giaddr.s_addr) && (forward == 0)) {
                dst = bp->bp_giaddr;
		to.sin_port = htons(bootps_port);
        } else {
                dst = bp->bp_yiaddr;
                retarp = setarp(&dst, bp->bp_chaddr, bp->bp_hlen,bp->bp_htype,rtinfo);
        }

        if (forward == 0)
	{
                /*
                 * If we are originating this reply, we
                 * need to find our own interface address to
                 * put in the bp_siaddr field of the reply.
                 * If this server is multi-homed, pick the
                 * 'best' interface (the one on the same net
                 * as the client).
                 */
                int matchbytes;

		matchbytes = getbestif(&dst, &ifrbest);
		
		if (bp->bp_siaddr.s_addr == 0)
			bp->bp_siaddr = satosin(&ifrbest->ifr_addr)->sin_addr;
                if (bp->bp_giaddr.s_addr == 0) {
                        if (matchbytes == 0)
			{
				report(LOG_ERR, "No route to %s\n",
				       inet_ntoa(dst));
				return;
			}
			bp->bp_giaddr = satosin(&ifrbest->ifr_addr)->sin_addr;
                }

		client_s_addr = bp->bp_ciaddr.s_addr ? bp->bp_ciaddr : bp->bp_yiaddr;
		/* gw specified but actually client and server are in 
                   same subnet
                 */
		if( bp->bp_giaddr.s_addr &&
			( ( client_s_addr.s_addr & netmask)
			== (bp->bp_siaddr.s_addr & netmask) ) )
		{
			/* client and server in same subnet */
			samesubnet = 1;
		}
		else
		{
			/* client and server are NOT in same subnet */
			samesubnet = 0;
		}
		if ( !(bp->bp_ciaddr.s_addr)
				&& bp->bp_giaddr.s_addr
				&& (samesubnet == 1))
		{
			to.sin_port = htons(bootpc_port);
			dst = bp->bp_yiaddr;
			bp->bp_giaddr = bp->bp_siaddr;
                        retarp = setarp(&dst, bp->bp_chaddr, bp->bp_hlen,bp->bp_htype, rtinfo);
		}


        }
	
	to.sin_addr = dst;

	if (debug) {
        	report(LOG_INFO, "The following addresses are included in the bootp reply\n");
        	report(LOG_INFO, "Client IP address (bp->bp_ciaddr) = %s\n",
              		inet_ntoa(bp->bp_ciaddr.s_addr ? bp->bp_ciaddr : bp->bp_yiaddr));
        	report(LOG_INFO, "Server IP address (bp->bp_siaddr) = %s\n",
              		inet_ntoa(bp->bp_siaddr));
        	if (bp->bp_giaddr.s_addr)
                	report(LOG_INFO, 
				"Gateway IP address (bp->bp_giaddr) = %s\n", 
				inet_ntoa(bp->bp_giaddr));
	}
	if (bp->bp_htype == HTYPE_IEEE8023)  /*8023*/
		bp->bp_htype = HTYPE_IEEE802;  /*8023*/
	else if (bp->bp_htype == HTYPE_FDDI) /*FDDI*/
		bp->bp_htype = HTYPE_ETHERNET; /*FDDI*/
        if ((ret = sendto(s, bp, sizeof(struct bootp), 0, &to, sizeof(to)))
	    < 0) {
		report(LOG_ERR, "sendto: %s\n", get_network_errmsg());
	}

        /*
         * fix for older Xstations on token ring... if rtinfo is not set,
         * sleep a bit (to allow the sendto to get to the wire) and
         * delete the arp table entry.
         */
        if ((rtinfo == FALSE) && ( retarp != (struct arpreq *) NULL )) {
            sleep (1);
            delarp (retarp);
        }
}


#define BRADS
/*
 * Return the number of leading bytes matching in the
 * internet addresses supplied.
 */
static nmatch(ca, cb)
    register char *ca, *cb;
{
    register int n, m;

#ifdef BRADS 
/*
 * Return number of bits that match.
 */

    for (m = n = 0 ; n < 4 ; n++) {
        if (*ca++ != *cb++) {
	    ca--;cb--;
	    n = ~(*ca ^ *cb); /* first 0 bit is first different bit */
	    m *=8;
	    if(n&0x80){ m++;
	        if(n&0x40){ m++;
	            if(n&0x20){ m++;
	                if(n&0x10){ m++;
	                    if(n&0x08){ m++;
	                        if(n&0x04){ m++;
	                            if(n&0x02){ m++;
	                                if(n&0x01) m++;}}}}}}}
            return(m);
	}
        m++;
    }
    return(8*m);
#else
    register n,m;

    for (m = n = 0 ; n < 4 ; n++) {
        if (*ca++ != *cb++)
            return(m);
        m++;
    }
    return(m);
#endif
}



/*
 * Setup the arp cache so that IP address 'ia' will be temporarily
 * bound to hardware address 'ha' of length 'len'.
 * HACK HACK HACK HACK
 */
struct arpreq *
setarp(ia, ha, len,type, rtinfo)
        struct in_addr *ia;
        byte *ha;
        int len;
	char type;
        int rtinfo;
{

        static struct arpreq arpreq;
        struct arpreq *retarp;
        struct sockaddr_in *si;
        int s;                         /* Used to hold socket               */

        retarp = (struct arpreq *) NULL;
        bzero((caddr_t)&arpreq, sizeof(arpreq));

        /* Start of MOTT hack to make this work on PS/2                     */
        si = (struct sockaddr_in *)&arpreq.arp_pa;
        si->sin_family = AF_INET;
	si->sin_addr = *ia;

	/* If old style boot is requested, then create a permanent arp entry */
	if (rtinfo)
        	arpreq.arp_flags = ATF_INUSE | ATF_COM | ATF_PERM;
	else
        	arpreq.arp_flags = ATF_INUSE | ATF_COM;
		
	arpreq.arp_ha.sa_family = AF_UNSPEC;
#ifdef _FIELD_SALEN
	si->sin_len = 8; 		/* char + char + short + long */
	arpreq.arp_ha.sa_len = 2 + len; /* char + char + hwaddrlen */
#endif
        bcopy(ha, arpreq.arp_ha.sa_data, len);

        s = socket(AF_INET,SOCK_DGRAM,0);   /* Get a socket for ioctl       */
        if (s < 0)                     /* Did we get the socket we need?    */
          report(LOG_ERR,"No socket in setarp(%i)",s);
        else
	{
#ifdef _FIELD_IFTYPE
		arpreq.at_length = (u_short) len;
		switch (type) {
		case HTYPE_ETHERNET: arpreq.ifType = IFT_ETHER;
			             report(LOG_INFO,"Creating 10Mb Ethernet arp table entry\n");
				     break;
		case HTYPE_IEEE802:  arpreq.ifType = IFT_ISO88025;

			             report(LOG_INFO,"Creating Token Ring arp table entry\n");
				/*
				 * The following two lines of code enable
				 * bootpd to respond over token ring
				 * bridges.  The problem with using bootpd
				 * over bridges is that the initial request
				 * can get to us (via an all-routes-broadcast),
				 * but by the time the packet makes it thru
				 * the kernel up to bootpd, all the token-ring
				 * hardware-level routing information has been
				 * stripped off.  When we insert the arp
				 * table entry for this host, by NOT providing
				 * token-ring routing information, we are
				 * in effect limiting the bootpd reply to
				 * the LOCAL-RING only. 
				 * Now, the "fix" for this problem:
				 * when we insert the arp table entry,
				 * we provide a routing-control-field
				 * which is set to all-routes-broadcast.
				 * This will ensure that the bootpd reply
				 * will get back to the client, regardless
				 * of local-ring or over bridges.  We also
				 * turn off the permanent flag so that this
				 * entry will time-out, forcing an arp
				 * request at a later point in time which
				 * will fill in the correct routing 
				 * information for this client.
				 */
				 /* Fix for older Xstations that can not
				  * handle arp requests and all-rings
			   	  * broadcast replies. (IBM Eproms <= 1.4)
                                  */
				     if (!rtinfo) {
                                         report(LOG_INFO, "Broadcast reply to all rings.\n");
					 /* set all-rings broadcast */
                                         arpreq.arp_rcf = 0x8270; 

					/* Return the arp entry structure so
					 * that the arp entry can be deleted.
					 * This arp entry should be deleted
					 * so that we do not continue to
					 * use all-rings broadcast mode to
					 * route packets to the client after
					 * the client has booted.
					 */
                                         retarp = &arpreq;
                                     }
				     break;
		case HTYPE_FDDI    : arpreq.ifType = IFT_FDDI;
			             report(LOG_INFO,"Creating FDDI arp table entry\n");
				     /* fddi is a ring like tr and may need to boot
                                        over bridges and stuff someday, so let's go
                                        ahead and put the broadcast bit in day one */
				     /* We will leave the if condition in just in case
				        some fddi client somewhere can't handle arps.
			                I don't expect anyone will ever use the dt: tag
                                        for fddi clients, but they can if they need to. */
                                     if (!rtinfo) {
                                       report(LOG_INFO, "Broadcast reply to all rings.\n");
                                       /* set all-rings broadcast */
                                       arpreq.arp_rcf = 0x8270;
                                       /* return the arp entry structure address */
                                       retarp = &arpreq;
                                     }
				     break;
		default		   : arpreq.ifType = IFT_ISO88023; /*8023*/
			             report(LOG_INFO,"Creating IEEE802.3 Ethernet arp table entry\n");
		}
#endif
		if (ioctl(s, SIOCSARP,(caddr_t)&arpreq) < 0)
		{
			report(LOG_ERR, "ioctl(SIOCSARP): %s\n",
			       get_network_errmsg());
		}
		else if (debug) {
			report(LOG_INFO, "ioctl(SIOCSARP): Arp entry created successfully.\n");
		}
		close(s);  
	}
	return(retarp);
}


/* Delete the arp table entry that was created in setarp.    */
/* This is only used with the token ring and fddi arp table entries.  */
delarp (arpreq)
struct arpreq *arpreq;
{
        int s;                     /* Used to hold socket               */

        s = socket(AF_INET,SOCK_DGRAM,0);   /* Get a socket for ioctl   */
        if (s < 0) {               /* Did we get the socket we need?    */
            report(LOG_ERR,"No socket in delarp(%i)",s);
        }
        else {
            if (ioctl(s, SIOCDARP, (caddr_t)arpreq) < 0) {
                report(LOG_ERR, "ioctl(SIOCDARP): %s\n", 
			get_network_errmsg());
            }
	    else if (debug) {
		report(LOG_INFO, "ioctl(SIOCDARP): Arp entry deleted successfully.\n");
	    }
            close(s);
        }
}

#ifdef _USE_DEBUG

/*
 * Dump the internal memory database to bootpd_dump.
 */

dumptab()
{
    register int n;
    register struct host *hp;
    register FILE *fp;
    long t;

    /*
     * Open bootpd.dump file.
     */
    if ((fp = fopen(DUMP_FILE, "w")) == NULL) {
        report(LOG_ERR, "error opening dumpfile - \"%s\": %s\n", bootpd_dump,
                        get_errmsg());
        exit(1);
    }

    t = time(NULL);
    fprintf(fp, "\n# %s\n", Version);
    fprintf(fp, "# %s: dump of bootp server database.\n", bootpd_dump);
    fprintf(fp, "#\n# Dump taken %s", ctime(&t));
    fprintf(fp, "#\n#\n# Legend:\n");
    fprintf(fp, "#\thd -- home directory\n");
    fprintf(fp, "#\tbf -- bootfile\n");
    fprintf(fp, "#\tbs -- bootfile size in 512-octet blocks\n");
    fprintf(fp, "#\tcs -- cookie servers\n");
    fprintf(fp, "#\tds -- domain name servers\n");
    fprintf(fp, "#\tgw -- gateways\n");
    fprintf(fp, "#\tha -- hardware address\n");
    fprintf(fp, "#\thd -- home directory for bootfiles\n");
    fprintf(fp, "#\tht -- hardware type\n");
    fprintf(fp, "#\tim -- impress servers\n");
    fprintf(fp, "#\tip -- host IP address\n");
    fprintf(fp, "#\tlg -- log servers\n");
    fprintf(fp, "#\tlp -- LPR servers\n");
    fprintf(fp, "#\tns -- IEN-116 name servers\n");
    fprintf(fp, "#\trl -- resource location protocol servers\n");
    fprintf(fp, "#\tsa -- server IP address in bootp reply packet\n");
    fprintf(fp, "#\tsm -- subnet mask\n");
    fprintf(fp, "#\tto -- time offset (seconds)\n");
    fprintf(fp, "#\tts -- time servers\n\n\n");

    n = 0;
    for (hp = (struct host *) hash_FirstEntry(nmhashtable); hp != NULL;
         hp = (struct host *) hash_NextEntry(nmhashtable)) {
            dump_host(fp, hp);
            fprintf(fp, "\n");
            n++;
    }
    fclose(fp);

    if (debug) {
    	report(LOG_INFO, "dumped %d entries to \"%s\".\n", n, bootpd_dump);
    }
}



/*
 * Dump all the available information on the host pointed to by "hp".
 * The output is sent to the file pointed to by "fp".
 */

void dump_host(fp, hp)
FILE *fp;
struct host *hp;
{
    register int i;
    register byte *dataptr;

    if (hp) {
        if (hp->hostname) {
           fprintf(fp, "%s:", hp->hostname);
        }
        if (hp->flags.iaddr) {
            fprintf(fp, "ip=%s:", inet_ntoa(hp->iaddr));
        }
        if (hp->flags.htype) {
            fprintf(fp, "ht=%u:", (unsigned) hp->htype);
            if (hp->flags.haddr) {
                fprintf(fp, "ha=%s:", haddrtoa(hp->haddr, hp->htype));
            }
        }
        if (hp->flags.subnet_mask) {
            fprintf(fp, "sm=%s:", inet_ntoa(hp->subnet_mask));
        }
        if (hp->flags.cookie_server) {
            fprintf(fp, "cs=");
            list_ipaddresses(fp, hp->cookie_server);
            fprintf(fp, ":");
        }
        if (hp->flags.domain_server) {
            fprintf(fp, "ds=");
            list_ipaddresses(fp, hp->domain_server);
            fprintf(fp, ":");
        }
        if (hp->flags.gateway) {
            fprintf(fp, "gw=");
            list_ipaddresses(fp, hp->gateway);
            fprintf(fp, ":");
        }
        if (hp->flags.impress_server) {
            fprintf(fp, "im=");
            list_ipaddresses(fp, hp->impress_server);
            fprintf(fp, ":");
        }
        if (hp->flags.log_server) {
            fprintf(fp, "lg=");
            list_ipaddresses(fp, hp->log_server);
            fprintf(fp, ":");
        }
        if (hp->flags.lpr_server) {
            fprintf(fp, "lp=");
            list_ipaddresses(fp, hp->lpr_server);
            fprintf(fp, ":");
        }
        if (hp->flags.name_server) {
            fprintf(fp, "ns=");
            list_ipaddresses(fp, hp->name_server);
            fprintf(fp, ":");
        }
        if (hp->flags.rlp_server) {
            fprintf(fp, "rl=");
            list_ipaddresses(fp, hp->rlp_server);
            fprintf(fp, ":");
        }
	if (hp->flags.bootserver) {
	    fprintf(fp, "sa=%s:", inet_ntoa(hp->bootserver));
	}
        if (hp->flags.time_server) {
            fprintf(fp, "ts=");
            list_ipaddresses(fp, hp->time_server);
            fprintf(fp, ":");
        }
        if (hp->flags.time_offset) {
            if (hp->flags.timeoff_auto) {
                fprintf(fp, "to=auto:");
            } else {
                fprintf(fp, "to=%ld:", hp->time_offset);
            }
        }
        if (hp->flags.homedir) {
            fprintf(fp, "hd=%s:", hp->homedir);
        }
        if (hp->flags.bootfile) {
            fprintf(fp, "bf=%s:", hp->bootfile);
        }
        if (hp->flags.bootsize) {
            fprintf(fp, "bs=");
            if (hp->flags.bootsize_auto) {
                fprintf(fp, "auto:");
            } else {
                fprintf(fp, "%d:", hp->bootsize);
            }
        }
        if (hp->flags.name_switch && hp->flags.send_name) {
            fprintf(fp, "hn:");
        }
        if (hp->flags.vendor_magic) {
            fprintf(fp, "vm=");
            if (hp->flags.vm_auto) {
                fprintf(fp, "auto:");
            } else if (!bcmp(hp->vm_cookie, vm_cmu, 4)) {
                fprintf(fp, "cmu:");
            } else if (!bcmp(hp->vm_cookie, vm_rfc1048, 4)) {
                fprintf(fp, "rfc1048");
            } else {
                fprintf(fp, "%d.%d.%d.%d:",
                            (int) ((hp->vm_cookie)[0]),
                            (int) ((hp->vm_cookie)[1]),
                            (int) ((hp->vm_cookie)[2]),
                            (int) ((hp->vm_cookie)[3]));
            }
        }
        if (hp->flags.generic) {
            fprintf(fp, "generic=");
            dataptr = hp->generic->data;
            for (i = hp->generic->length; i > 0; i--) {
                fprintf(fp, "%02X", (int) *dataptr++);
            }
            fprintf(fp, ":");
        }
    }
}



/*
 * Dump an entire struct in_addr_list of IP addresses to the indicated file.
 *
 * The addresses are printed in standard ASCII "dot" notation and separated
 * from one another by a single space.  A single leading space is also
 * printed before the first adddress.
 *
 * Null lists produce no output (and no error).
 */

void list_ipaddresses(fp, ipptr)
    FILE *fp;
    struct in_addr_list *ipptr;
{
    register unsigned count;
    register struct in_addr *addrptr;

    if (ipptr) {
        count = ipptr->addrcount;
        addrptr = ipptr->addr;
        if (count-- > 0) {
            fprintf(fp, "%s", inet_ntoa(*addrptr++));
            while (count-- > 0) {
                fprintf(fp, " %s", inet_ntoa(*addrptr++));
            }
        }
    }
}
#endif          /* DEBUG */



#ifdef _USE_CMUVEND

/*
 * Insert the CMU "vendor" data for the host pointed to by "hp" into the
 * bootp packet pointed to by "bp".
 */

void dovend_cmu(bp, hp)
    register struct bootp *bp;
    register struct host *hp;
{
    struct cmu_vend *vendp;
    register struct in_addr_list *taddr;

    /* Fill in vendor information. Subnet mask, default gateway,
        domain name server, ien name server, time server */
    vendp = (struct cmu_vend *) bp->bp_vend;
    if (hp->flags.subnet_mask) {
        (vendp->v_smask).s_addr = hp->subnet_mask.s_addr;
        (vendp->v_flags) |= VF_SMASK;
        if (hp->flags.gateway) {
            (vendp->v_dgate).s_addr = hp->gateway->addr->s_addr;
        }
    }
    if (hp->flags.domain_server) {
        taddr = hp->domain_server;
        if (taddr->addrcount > 0) {
            (vendp->v_dns1).s_addr = (taddr->addr)[0].s_addr;
            if (taddr->addrcount > 1) {
                (vendp->v_dns2).s_addr = (taddr->addr)[1].s_addr;
            }
        }
    }
    if (hp->flags.name_server) {
        taddr = hp->name_server;
        if (taddr->addrcount > 0) {
            (vendp->v_ins1).s_addr = (taddr->addr)[0].s_addr;
            if (taddr->addrcount > 1) {
                (vendp->v_ins2).s_addr = (taddr->addr)[1].s_addr;
            }
        }
    }
    if (hp->flags.time_server) {
        taddr = hp->time_server;
        if (taddr->addrcount > 0) {
            (vendp->v_ts1).s_addr = (taddr->addr)[0].s_addr;
            if (taddr->addrcount > 1) {
                (vendp->v_ts2).s_addr = (taddr->addr)[1].s_addr;
            }
        }
    }
    strcpy(vendp->v_magic, vm_cmu);

    if (debug) {
        report(LOG_INFO, "sending CMU-style reply\n");
    }
}

#endif /* _USE_CMUVEND */



/*
 * Insert the RFC1048 vendor data for the host pointed to by "hp" into the
 * bootp packet pointed by "bp".
 */

void dovend_rfc1048(bp, hp, hp1, bootsize)
    register struct bootp *bp;
    register struct host *hp;
    register struct host *hp1;			/*alt*/
    long bootsize;
{
    int bytesleft, len;
    byte *vp;
    char *tmpstr;

    vp = bp->bp_vend;
    bytesleft = sizeof(bp->bp_vend);    /* Initial vendor area size */
    bcopy(vm_rfc1048, vp, 4);           /* Copy in the magic cookie */
    vp += 4;
    bytesleft -= 4;

    if (hp->flags.time_offset) {
        *vp++ = TAG_TIME_OFFSET;                        /* -1 byte  */
        *vp++ = 4;                                      /* -1 byte  */
        if (hp->flags.timeoff_auto) {
            insert_u_long(htonl(secondswest), &vp);
        } else {
            insert_u_long(htonl(hp->time_offset), &vp);         /* -4 bytes */
        }
        bytesleft -= 6;
    }
    if (hp->flags.subnet_mask) {
        *vp++ = TAG_SUBNET_MASK;                        /* -1 byte  */
        *vp++ = 4;                                      /* -1 byte  */
        insert_u_long(hp->subnet_mask.s_addr, &vp);     /* -4 bytes */
        bytesleft -= 6;                                 /* Fix real count */
        if (hp->flags.gateway) {
            insert_ip(TAG_GATEWAY, hp->gateway, &vp, &bytesleft);
        }
    }

    if (hp->flags.bootsize) {
        bootsize = (hp->flags.bootsize_auto) ?
                   (bootsize / 512 + 1) : (hp->bootsize);   /* Round up */
        *vp++ = TAG_BOOTSIZE;
        *vp++ = 2;
        *vp++ = (byte) ((bootsize >> 8) & 0xFF);
        *vp++ = (byte) (bootsize & 0xFF);
        bytesleft -= 4;         /* Tag, length, and 16 bit blocksize */
    }

    if (hp->flags.domain_server) {
        insert_ip(TAG_DOMAIN_SERVER, hp->domain_server, &vp, &bytesleft);
    }
    if (hp->flags.name_server) {
        insert_ip(TAG_NAME_SERVER, hp->name_server, &vp, &bytesleft);
    }
    if (hp->flags.rlp_server) {
        insert_ip(TAG_RLP_SERVER, hp->rlp_server, &vp, &bytesleft);
    }
    if (hp->flags.time_server) {
        insert_ip(TAG_TIME_SERVER, hp->time_server, &vp, &bytesleft);
    }
    if (hp->flags.log_server) {
        insert_ip(TAG_LOG_SERVER, hp->log_server, &vp, &bytesleft);
    }
    if (hp->flags.lpr_server) {
        insert_ip(TAG_LPR_SERVER, hp->lpr_server, &vp, &bytesleft);
    }
    if (hp->flags.cookie_server) {
        insert_ip(TAG_COOKIE_SERVER, hp->cookie_server, &vp, &bytesleft);
    }

    if (hp->flags.name_switch && hp->flags.send_name) {
        /*
         * Check for room for hostname.  Add 2 to account for
         * TAG_HOSTNAME and length.
         */
        len = strlen(hp->hostname);
        if ((len + 2) > bytesleft) {
            /*
             * Not enough room for full (domain-qualified) hostname, try
             * stripping it down to just the first field (host).
             */
            tmpstr = hp->hostname;
            len = 0;
            while (*tmpstr && (*tmpstr != '.')) {
                tmpstr++;
                len++;
            }
        }
        if ((len + 2) <= bytesleft) {
            *vp++ = TAG_HOSTNAME;
            *vp++ = (byte) (len & 0xFF);
            bcopy(hp->hostname, vp, len);
            vp += len;
        }
    }
    if (hp->flags.generic) {
        insert_generic(hp->generic, &vp, &bytesleft);
    }

    if (hp1 != NULL){						/*alt*/
	*vp++ = TAG_ALT_IP;					/*alt*/
	*vp++ = 4;						/*alt*/
	insert_u_long (hp1->iaddr.s_addr, &vp);			/*alt*/
	bytesleft -= 6;						/*alt*/
	if (hp1->flags.subnet_mask){				/*alt*/
		*vp++ = TAG_ALT_SM;				/*alt*/
		*vp++ = 4;					/*alt*/
		insert_u_long (hp1->subnet_mask.s_addr, &vp);	/*alt*/
		bytesleft -= 6;					/*alt*/
		if (hp1->flags.gateway){			/*alt*/
			insert_ip(TAG_ALT_GW, hp1->gateway, &vp, &bytesleft);
		}						/*alt*/
 	}							/*alt*/
    }								/*alt*/


    if (bytesleft >= 1) {
        *vp = TAG_END;
    }
    if (debug > 2){
     int i;							/*alt*/
        report(LOG_INFO,"RFC1048 vendor data ( bp_vend[64] ) \n");
        for(i = 0; i < 64; i++)					/*alt*/
               fprintf(stderr,"%d.",bp->bp_vend[i]);		/*alt*/
    }								/*alt*/

    if (debug) {
        report(LOG_INFO, "sending RFC1048-style reply\n");
    }
}



/*
 * Compare function to determine whether two hardware addresses are
 * equivalent.  Returns TRUE if "host1" and "host2" are equivalent, FALSE
 * otherwise.
 *
 * This function is used when retrieving elements from the hardware address
 * hash table.
 */

boolean hwlookcmp(host1, host2)
    struct host *host1, *host2;
{
    if (host1->htype != host2->htype) {
        return FALSE;
    }
    if (bcmp(host1->haddr, host2->haddr, haddrlength(host1->htype))) {
        return FALSE;
    }
    return TRUE;
}




/*
 * Compare function for doing IP address hash table lookup.
 */

boolean iplookcmp(host1, host2)
    struct host *host1, *host2;
{
    return (host1->iaddr.s_addr == host2->iaddr.s_addr);
}



/*
 * Insert a tag value, a length value, and a list of IP addresses into the
 * memory buffer indirectly pointed to by "dest", without going past "stop".
 * "ipst_prt" is a pointer to an ipaddr_t (i.e. a structure containing a
 * pointer to a linked list of IP addresses and a linkcount specifying how
 * many hosts are using this list).
 *
 * This is used to fill the vendor-specific area of a bootp packet in
 * conformance to RFC1048.
 */

void insert_ip(tag, iplist, dest, bytesleft)
    byte tag;
    struct in_addr_list *iplist;
    byte **dest;
    int *bytesleft;
{
    register struct in_addr *addrptr;
    register unsigned addrcount;
    byte *d;

    if (iplist && (*bytesleft >= 6)) {
        d = *dest;                              /* Save pointer for later */
        **dest = tag;
        (*dest) += 2;
        (*bytesleft) -= 2;                  /* Account for tag and length */
        addrptr = iplist->addr;
        addrcount = iplist->addrcount;
        while ((*bytesleft >= 4) && (addrcount > 0)) {
            insert_u_long(addrptr->s_addr, dest);
            addrptr++;
            addrcount--;
            (*bytesleft) -= 4;                  /* Four bytes per address */
        }
        d[1] = (byte) ((*dest - d - 2) & 0xFF);
    }
}



/*
 * Insert generic data into a bootp packet.  The data is assumed to already
 * be in RFC1048 format.  It is inserted using a first-fit algorithm which
 * attempts to insert as many tags as possible.  Tags and data which are
 * too large to fit are skipped; any remaining tags are tried until they
 * have all been exhausted.
 */

void insert_generic(gendata, buff, bytesleft)
    struct bindata *gendata;
    byte **buff;
    int *bytesleft;
{
    byte *srcptr;
    register int length, numbytes;

    if (gendata) {
        srcptr = gendata->data;
        length = gendata->length;
        while ((length > 0) && (*bytesleft > 0)) {
            switch (*srcptr) {
                case TAG_END:
                    length = 0;         /* Force an exit on next iteration */
                    break;
                case TAG_PAD:
                    *(*buff)++ = *srcptr++;
                    (*bytesleft)--;
                    length--;
                    break;
                default:
                    numbytes = srcptr[1] + 2;
                    if (*bytesleft >= numbytes) {
                        bcopy(srcptr, *buff, numbytes);
                        (*buff) += numbytes;
                        (*bytesleft) -= numbytes;
                    }
                    srcptr += numbytes;
                    length -= numbytes;
                    break;
            }
        }
    }
}




/*
 * Convert a hardware address to an ASCII string.
 */

char *haddrtoa(haddr, htype)
    register byte *haddr;
    byte htype;
{
    static char haddrbuf[2 * MAXHADDRLEN + 1];
    register char *bufptr;
    register unsigned count;

    bufptr = haddrbuf;
    for (count = haddrlength(htype); count > 0; count--) {
        sprintf(bufptr, "%02X", (unsigned) (*haddr++ & 0xFF));
        bufptr += 2;
    }
    return (haddrbuf);
}



/*
 * Insert the unsigned long "value" into memory starting at the byte
 * pointed to by the byte pointer (*dest).  (*dest) is updated to
 * point to the next available byte.
 *
 * Since it is desirable to internally store network addresses in network
 * byte order (in struct in_addr's), this routine expects longs to be
 * passed in network byte order.
 *
 * However, due to the nature of the main algorithm, the long must be in
 * host byte order, thus necessitating the use of ntohl() first.
 */

void insert_u_long(value, dest)
    unsigned long value;
    byte **dest;
{
    register byte *temp;
    register int n;

    value = ntohl(value);       /* Must use host byte order here */
    temp = (*dest += 4);
    for (n = 4; n > 0; n--) {
        *--temp = (byte) (value & 0xFF);
        value >>= 8;
    }
    /* Final result is network byte order */
}



/*
 * Return pointer to static string which gives full filesystem error message.
 */

char *get_errmsg()
{
    static char errmsg[80];

    if (errno < sys_nerr) {
        return sys_errlist[errno];
    } else {
        sprintf(errmsg, "Error %d", errno);
        return errmsg;
    }
}



/*
 * This routine reports errors and such via stderr and syslog() if
 * appopriate.  It just helps avoid a lot of "#ifdef SYSLOG" constructs
 * from being scattered throughout the code.
 *
 * The syntax is identical to syslog(3), but %m is not considered special
 * for output to stderr (i.e. you'll see "%m" in the output. . .).  Also,
 * control strings should normally end with \n since newlines aren't
 * automatically generated for stderr output (whereas syslog strips out all
 * newlines and adds its own at the end).
 */

/*VARARGS2*/
void report(priority, fmt, p0, p1, p2, p3, p4)
    int priority;
    char *fmt;
{
    /*
     * Print the message
     */
    switch (priority) {
    case LOG_ERR    : if (debug) {
                          fprintf(stderr, "BOOTPD: ");
                          fprintf(stderr, fmt, p0, p1, p2, p3, p4);
                      }
                      break;
    case LOG_NOTICE : if (debug > 1) {
                          fprintf(stderr, "BOOTPD: ");
                          fprintf(stderr, fmt, p0, p1, p2, p3, p4);
                      }
                      break;
    case LOG_INFO   : if (debug > 2) {
                          fprintf(stderr, "BOOTPD: ");
                          fprintf(stderr, fmt, p0, p1, p2, p3, p4);
                      }
                      break;
    }

#ifdef _USE_SYSLOG
    syslog(priority, fmt, p0, p1, p2, p3, p4);
#endif
}
#if    defined(FEATURE)
 /*
  * Check if the current host is a secondary boot host
  * for the terminal that is requesting the boot.
  * The boot host is a secondary boot host only
  * if the 'T175' tag value is set to non-zero value,
  * otherwise the boot host is considered as a primary boot host.
  * If the 'T175' tag is not there at all,
  * the host behaves like a primary boot host.
  * The function returns total number of seconds that
  * the file server should wait before it responds to next boot request.
  */
 int chk_secondary(hp)
     struct host *hp;
 {
     struct bindata *gendata;
     register int length, numbytes;
     byte *srcptr;


     if (!( gendata = hp->generic) )
        return(0);

     srcptr = gendata->data;
     length = gendata->length;
     while (length > 0)
     {
             switch (*srcptr)
             {
                 case TAG_END:
                       return(0);

                 case TAG_PRIMARY:
                       srcptr += 2;
                       /* this arithmetic is necessary since a decimal
			  number is read in as an hex number */
                       return((*srcptr/16)*10 + (*srcptr%16));
                 case TAG_PAD:
                      srcptr++;
                      length--;
                      break;

                 default:
                    numbytes = srcptr[1] + 2;
                    srcptr += numbytes;
                    length -= numbytes;
                    break;
             } /* the end of switch */
    } /* the end of while */
    return(0);
}
#endif   /* FEATURE */
