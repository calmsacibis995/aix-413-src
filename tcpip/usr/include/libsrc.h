/* @(#)98	1.7  src/tcpip/usr/include/libsrc.h, tcpip, tcpip411, GOLD410 12/8/92 07:52:10 */
/*
 * libsrc.h -  Header file for TCP/IP SRC/ODM routines.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#define	BLANK		" "		/* pointer to blank string */

/* flat files used for inet services */
#define	INET_SRVFILE	"/etc/services"		
#define	INET_CNFFILE	"/etc/inetd.conf"

/* defines for inetserv object field value verification */
#define	SUNRPC		"sunrpc"
#define	SUNRPC_UDP	"sunrpc_udp"
#define	SUNRPC_TCP	"sunrpc_tcp"
#define	UDP		"udp"
#define	TCP		"tcp"
#define	STREAM		"stream"
#define	DGRAM		"dgram"
#define	TCP_WAIT	"wait"
#define	TCP_NOWAIT	"nowait"
#define	TCP_STAT	"stinet"      

/* defines for SRC packet/status processing */
#define SRCSTATNDX 	6
#define SRC_FD 		13
#define MAXCONN 	40
#define INETEXP	 	"/usr/sbin/inetexp"
#define STINET     	"/usr/sbin/stinet"
#define TELNETD     	"/usr/sbin/telnetd"
#define SRCMSG     	(sizeof(srcpacket))

/* shared memory structures used to store active services infomation */
struct  active_conn {
        int     pid;            /* process id of connection */
        u_long  inetaddr;       /* IP address of connection */
        char    hostname[32];   /* userid for connection */
        };

struct  active_serv {
        char    service[9];
        struct  active_conn connect[MAXCONN];
        };

struct  service_entry {
        struct  active_serv serv[MAXCONN];
        };

struct  telnetd_conn {
        int     pid;            /* process id of connection */
        char    termtype[32];   /* terminal emulition type */
        };

struct  telnetd_entry {
        struct  telnetd_conn conn[MAXCONN];
        };
