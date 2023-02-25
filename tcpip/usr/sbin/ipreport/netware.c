static char sccsid[] = "@(#)92	1.1  src/tcpip/usr/sbin/ipreport/netware.c, tcpras, tcpip411, GOLD410 12/8/92 10:44:48";
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

#include <stdio.h>

struct IPXAddress {
    char network[4];
    char node[6];
    u_short socket;

#define IPX_SOCKET_NCP	0x451	/* File Service Packet */
#define IPX_SOCKET_SAP	0x452	/* Service Advertising Packet */
#define IPX_SOCKET_RIP	0x453	/* Routing Information Packet */
#define IPX_SOCKET_NB	0x455	/* NetBIOS Packet */
#define IPX_SOCKET_DIAG	0x456	/* Diagnostic Packet */

};

struct ipxHeader {
    u_short checkSum;
    u_short length;
    char transportControl;
    char packetType;

#define IPX_RI      1               /* Routing Information */
#define IPX_ECHO    2               /* Echo Protocol */
#define IPX_ERROR   3               /* Error Protocol */
#define IPX_PE      4               /* Packet Exchange */
#define IPX_SPP     5               /* Sequenced Packet */
#define IPX_NCP     17              /* NetWare Core Protocols Packet */
#define IPX_NB      20              /* NetBIOS Name Packet */

    struct IPXAddress destination;
    struct IPXAddress source;
};

struct ncpRequest {
    u_short	type;

#define NCP_CREATE	0x1111
#define NCP_REQUEST	0x2222
#define NCP_REPLY	0x3333
#define NCP_DESTROY	0x5555
#define NCP_PROCESS	0x9999

    char	sequence;
    char	connectionLow;
    char	task;
    char	connectionHigh;
};

struct ncpReply {
    struct ncpRequest req;
    char	code;
    char	status;
};

extern char *ipx_beg, *ncp_beg, *beg_line;
extern int like_snoop;

static char *
p_ipxttype (type)
char type;
{
    static char s[20];

    sprintf (s, "%d ", type);
    switch (type) {
	case IPX_RI:	return (strcat (s, "(RIP)"));
	case IPX_ECHO:	return (strcat (s, "(ECHO)"));
	case IPX_ERROR: return (strcat (s, "(ERROR)"));
	case IPX_PE:	return (strcat (s, "(IPX)"));	/* netware */
	case IPX_SPP:	return (strcat (s, "(SPX)"));	/* netware */
	case IPX_NCP:	return (strcat (s, "(NCP)"));	/* netware */
	case IPX_NB:	return (strcat (s, "(NetBIOS)"));
    }
    return (s);
}

static char *
p_ipxsocket (socket)
u_short socket;
{
    static char s[20];

    sprintf (s, "%04x ", socket);

    switch (socket) {
	case IPX_SOCKET_NCP: return (strcat (s, "(NCP)"));
	case IPX_SOCKET_SAP: return (strcat (s, "(SAP)"));
	case IPX_SOCKET_RIP: return (strcat (s, "(RIP)"));
	case IPX_SOCKET_NB:  return (strcat (s, "(NetBIOS)"));
	case IPX_SOCKET_DIAG: return (strcat (s, "(DIAG)"));
    }
    return (s);
}

static void
p_line(b1, b2)
char *b1, *b2;
{
    printf ("%s\t%-30.30s%-30.30s\n", beg_line, b1, b2);
}

dump_ipx(p,len)
char *p;
int len;
{
    struct ipxHeader *ipx = (struct ipxHeader *)p;
    char *sd, *ss;
    char b1[40], b2[40];

    p += sizeof (struct ipxHeader);
    len -= sizeof (struct ipxHeader);

    reset_beg_line(ipx_beg);
    if (!like_snoop) printf("IPX header breakdown:\n");

    sprintf (b1, "Check Sum: 0x%x", ipx -> checkSum);
    sprintf (b2, "Length: %d", ipx -> length);
    p_line (b1, b2);

    sprintf (b1, "Hop Count: %d", ipx -> transportControl);
    sprintf (b2, "Packet Type: %s", p_ipxttype (ipx -> packetType));
    p_line (b1, b2);

    sd = ipx -> destination.network;
    ss = ipx -> source.network;
    sprintf (b1, "Network: %02x %02x %02x %02x", ss[0], ss[1], ss[2], ss[3]);
    sprintf (b2, " --->  %02x %02x %02x %02x", sd[0], sd[1], sd[2], sd[3]);
    p_line (b1, b2);

    sd = ipx -> destination.node;
    ss = ipx -> source.node;
    sprintf (b1, "Node:    %02x:%02x:%02x:%02x:%02x:%02x", 
	    ss[0], ss[1], ss[2], ss[3], ss[4], ss[5]);
    sprintf (b2, " --->  %02x:%02x:%02x:%02x:%02x:%02x", 
	    sd[0], sd[1], sd[2], sd[3], sd[4], sd[5]);
    p_line (b1, b2);

    sprintf (b1, "Socket:  %s", p_ipxsocket (ipx -> destination.socket));
    sprintf (b2, " --->  %s", p_ipxsocket (ipx -> source.socket));
    p_line (b1, b2);

#define ISPORT(p) (ipx -> destination.socket == (p) || ipx -> source.socket == (p))
    if (ISPORT(IPX_SOCKET_NCP)) {
	dump_ncp (p, len);
    }
    else
	hex_dump(p, len);
}

static void
p_ncp (req)
struct ncpRequest *req;
{
    char b1[40], b2[40];

    printf ("%s\tNCP Type: 0x%04x ", beg_line, req -> type);
    switch (req -> type) {
	case NCP_REQUEST:
	    printf ("(REQUEST)"); 
	    break;
	case NCP_REPLY: 
	    printf ("(REPLY)"); 
	    break;
	case NCP_CREATE:
	    printf ("(Create Service Connection)");
	    break;
	case NCP_DESTROY:
	    printf ("(Destroy Service Connection)");
	    break;
	case NCP_PROCESS:
	    printf ("(Request Being Processed)");
	    break;
    }
    printf ("\n");

    sprintf (b1, 
	    req -> type == NCP_PROCESS ? 
		"Sequence Number: (N/A)" : "Sequence Number: %d", 
	    req -> sequence);
    sprintf (b2, "Connection Number: %x", req -> connectionLow);
    p_line (b1, b2);

    printf ((req -> type == NCP_CREATE || req -> type == NCP_DESTROY) ? 
		"%s\tTask Number: (ignored)\n" : "%s\tTask Number: %x\n", 
	    beg_line, req -> task);
}

dump_ncp (p, len)
char *p;
int len;
{
    struct ncpRequest *req = (struct ncpRequest *)p;
    char b1[40], b2[40];

    reset_beg_line(ncp_beg);
    if (!like_snoop) printf("NCP header breakdown:\n");
    switch (req -> type) {
	case NCP_REQUEST:
	    p += sizeof (struct ncpRequest);
	    len -= sizeof (struct ncpRequest);
	    p_ncp (req);
	    printf ("%s\tRequest Type: %d %d\n", beg_line, p[0], p[1]);
	    break;
	case NCP_REPLY:
	{
	    struct ncpReply *rep = (struct ncpReply *)p;

	    p += sizeof (struct ncpReply);
	    len -= sizeof (struct ncpReply);
	    p_ncp (req);
	    sprintf (b1, "Completion Code: %x", rep -> code);
	    sprintf (b2, "Connection Status: %x", rep -> status);
	    p_line (b1, b2);
	    hex_dump(p, len);
	    break;
	}
	case NCP_CREATE:
	    p += sizeof (struct ncpRequest);
	    len -= sizeof (struct ncpRequest);
	    p_ncp (req);
	    hex_dump(p, len);
	    break;
	case NCP_DESTROY:
	    p += sizeof (struct ncpRequest);
	    len -= sizeof (struct ncpRequest);
	    p_ncp (req);
	    hex_dump(p, len);
	    break;
	case NCP_PROCESS:
	{
	    struct ncpReply *rep = (struct ncpReply *)p;

	    p += sizeof (struct ncpReply);
	    len -= sizeof (struct ncpReply);
	    p += sizeof (struct ncpRequest);
	    len -= sizeof (struct ncpRequest);
	    p_ncp (req);
	    sprintf (b1, "Completion Code: (N/A)");
	    sprintf (b2, "Connection Status: (N/A)");
	    p_line (b1, b2);
	    hex_dump(p, len);
	    break;
	}
	default:
	    hex_dump(p, len);
	    break;
    }
}
