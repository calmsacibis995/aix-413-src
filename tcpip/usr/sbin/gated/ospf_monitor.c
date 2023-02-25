static char sccsid[] = "@(#)98	1.1  src/tcpip/usr/sbin/gated/ospf_monitor.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:19";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: HLAST
 *		HNEXT
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF6
 *		ciao
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * ospf_monitor.c,v 1.18.2.1 1993/08/27 22:28:27 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_CTYPE
#define	INCLUDE_TIME
#define	INCLUDE_SIGNAL
#include "include.h"
#include "inet.h"
#include "ospf.h"
#include "string.h"

#define HISTORY_SIZE 30			/* Size of history cache */

#define HNEXT(NDX) (((NDX+1) == HISTORY_SIZE) ? 0 : (NDX + 1))
#define HLAST(NDX) (((NDX) == 0) ? HISTORY_SIZE - 1 : (NDX - 1))

int gwsock;				/* socket to receive gw info from */
int gw;					/* gw connection */
u_int16 port;				/* port to listen on */
FILE *localfp;

#undef	ciao
static void
ciao(s)
const char *s;
{
    perror(s);

    exit (1);
}


static SIGTYPE
got_int __PF1(sig, int)
{
    if (gw && (close(gw) < 0))
	printf("Badclose\n");
    /* reset */
    SIGRETURN;
}


static const char *
lntoa __PF1(addr, u_long)
{
    static int i = 0;
    static char bufs[8][20];
    unsigned char *p = (unsigned char *) &addr;

    i = (i + 1) % (sizeof bufs / sizeof bufs[0]);
    sprintf(bufs[i], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return bufs[i];
}


static void
print_params __PF2(fp, FILE *,
		   buf, char *)
{
    int i;

    for (i = 0; i < 70; i++) {
	if (buf[i] == '\0')
	    fprintf(fp, " ");
	else if ((buf[i] == '\n') || (buf[i] == '\r'))
	    break;
	else
	    fprintf(fp, "%c", buf[i]);
    }
    fprintf(fp, "\n");
    fflush(fp);
}


static void
remote_help __PF0(void)
{
    printf("Remote-commands:\n");
    printf("   a <area id> <type> <ls id> <adv rtr>: show link state advertisement\n");
    printf("   c: show cumulative log\n");
    printf("   e: show cumulative errors\n");
    printf("   l: <retrans> dump lsdb (except for ASEs)\n");
    printf("   A: <retrans> dump ASEs\n");
    printf("   o: print ospf routing table\n");
    printf("   I: show interfaces\n");
    printf("   N <r>: show neighbors - if r is set will print retrans lst\n");
}


static void
help __PF0(void)
{
    printf("Local commands:\n");
    printf("   ?: help \n");
    printf("   ?R: remote command information \n");
    printf("   d: show configured destinations\n");
    printf("   h: show history\n");
    printf("   x: exit\n");
    printf("   @ <remote command>: use last destination \n");
    printf("   @<dest index> <remote command>: use configured destination \n");
    printf("   F <filename>: write monitor information to filename\n");
    printf("   S: write monitor information to stdout (defalut)\n");
}


struct DEST {
    u_int32 dest;
    char *name;
    byte *auth;
} *dtab[100];

int destcnt;


/*
 * Syntax:
 *    <gw ip addr> <gw name> [<password>]
 *
 */
static void
monconf __PF1(fn, char *)
{
    int line = 0;
    FILE *fp, *fopen();
    char buf[LINE_MAX];
    static const char *tokens = " \t\n";

    if (!(fp = fopen(fn, "r"))) {
	fprintf(stderr, "monconf: Can't open monitor conf file %s\n", fn);
	exit(1);
    }

    while (fgets(buf, sizeof(buf), fp)) {
	char *cp = buf;
	char *name = (char *) 0;
	byte *auth = (byte *) 0;
	struct in_addr addr;
	
	line++;
	
	switch (*cp) {
	case '\0':
	case '\n':
	case '#':
	    /* Blank line or a comment */
	    continue;
	}

	if (!(cp = strtok(cp, tokens))) {
	    /* Blank line */
	    continue;
	}

	if (!inet_aton(cp, &addr)) {
	    fprintf(stderr, "invalid address (%s) on line %d\n",
		    cp,
		    line);
	}
	
	name = strtok(NULL, tokens);
	if (!name) {
	    fprintf(stderr, "router name missing on line %d\n",
		    line);
	}

	/* Authentication field is optional */
	auth = (byte *) strtok(NULL, tokens);

	dtab[destcnt] = (struct DEST *) ((void_t) calloc(1,
							 (size_t) (sizeof (struct DEST) +
								   strlen(name) +
								   (auth ? strlen((char *) auth) : 0) + 2)));
	if (!dtab[destcnt]) {
	    fprintf(stderr, "out of memory reading %s at line %d\n",
		    fn,
		    line);
	    exit (1);
	}

	dtab[destcnt]->dest = addr.s_addr;
	if (name) {
	    dtab[destcnt]->name = (char *) (dtab[destcnt] + 1);
	    (void) strcpy(dtab[destcnt]->name, name);
	    if (auth) {
		dtab[destcnt]->auth = (byte *) (dtab[destcnt]->name + strlen(dtab[destcnt]->name) + 1);
		(void) strcpy((char *)dtab[destcnt]->auth, (char *) auth);
	    }
	}

	destcnt++;
    }
    
    fclose(fp);
}


static void
print_local __PF3(fp, FILE *,
		  to, u_int32,
		  name, char *)
{
#ifdef FD_ZERO
    fd_set ready;
#else	/* FD_ZERO */
    int ready;
#endif	/* FD_ZERO */
    struct timeval wait;
    struct sockaddr_in from;
    char buf[BUFSIZ];
    int i;

#ifndef FD_ZERO
#define FD_ZERO(SET) (*SET) = 0
#define FD_SET(FD,SET)	((*SET) |= (1 << (FD)))
#define FD_ISSET(FD,SET) ((*SET) & (1 << (FD)))
#endif
    FD_ZERO(&ready);
    FD_SET(gwsock, &ready);
    wait.tv_sec = 20;
    wait.tv_usec = 0;
    if (select(gwsock + 1, &ready, 0, 0, &wait) < 0) {
	perror("select");
	return;
    }
    if (FD_ISSET(gwsock, &ready)) {
	i = sizeof (from);
	if ((gw = accept(gwsock, (struct sockaddr *) &from, &i)) < 0) {
	    perror("accept");
	    return;
	}
	fprintf(fp, "\n          Source <<%-16s",
		lntoa(from.sin_addr.s_addr));

	if (name)
	    fprintf(fp, " %s", name);

	fprintf(fp, ">>\n");

	while ((i = read(gw, buf, sizeof buf - 1)) > 0) {
	    buf[i] = (char) 0;
	    fputs(buf, fp);
	}
	if (i < 0)
	    perror("read");
	if (close(gw) < 0)
	    printf("Badclose\n");
	fprintf(fp, "\n");
    } else
	printf("Connection timeout: try again...\n");
}


static void
txmon __PF6(fd, int,
	    req, u_int8,
	    buf, char *,
	    to, u_int32,
	    name, char *,
	    auth, byte *)
{
    u_int32 p[MAXPARAMS];
    size_t len = OSPF_HDR_SIZE + MON_REQ_SIZE;
    byte packet[OSPF_HDR_SIZE + MON_REQ_SIZE];
    struct OSPF_HDR *o_hdr = (struct OSPF_HDR *) ((void_t) packet);
    struct MON_HDR *m = &(o_hdr->un.mon);
    struct sockaddr_in dst;
    int dstlen = sizeof(dst);
    int local_wait = FALSE, i;
    int ret;
    char 	ip_addr_st[20], ip_mask_st[20], area_id_st[20];
    struct in_addr addr;

    /* sendit */
    bzero((caddr_t) p, (MAXPARAMS * 4));
    printf("   remote-command <%c ", req);

    switch(req) {
    case 'a':
 	/* 
 	 * show advertisement 
 	 */
 	sscanf(buf, "%*s %*s %s %d %s %s",
	       area_id_st,
	       &p[1],
	       ip_addr_st,
	       ip_mask_st);

	if (!inet_aton(area_id_st, &addr)) {
	    fprintf(stderr, "Invalid Area ID: %s\n",
		    area_id_st);
	}
	p[0] = addr.s_addr;
	if (!inet_aton(ip_addr_st, &addr)) {
	    fprintf(stderr, "Invalid LinkState-ID: %s\n",
		    ip_addr_st);
	}
 	p[2] = addr.s_addr;
	if (!inet_aton(ip_mask_st, &addr)) {
	    fprintf(stderr, "Invalid Advertising Router: %s\n",
		    ip_mask_st);
	}
 	p[3] = addr.s_addr;
 	printf("%s %d %s %s ",
	       lntoa(p[0]),
	       p[1],
	       lntoa(p[2]),
	       lntoa(p[3]));
 	local_wait = TRUE;
 	break;
 	
    case 'c':
    case 'e':
    case 'l':
    case 'A':
    case 'o':
    case 'N':
    case 'I':
	local_wait = TRUE;
	/* Fall through */

    default:
 	ret = sscanf(buf, "%*s %*s %ld %ld %ld %ld %ld %ld %ld",
		     &p[0],
		     &p[1],
		     &p[2],
		     &p[3],
		     &p[4],
		     &p[5],
		     &p[6]);
 	for (i = 0; i < ret; i++) {
	    if (p[i])
		printf("%d ", p[i]);
  	}
    }
    printf("> sent to %s\n", lntoa(to));

    m->type = MREQUEST;
    m->req = req;
    for (i = 0; i < MAXPARAMS; i++) {
	m->p[i] = htonl(p[i]);
    }
    if (local_wait) {
	m->port = port;
	m->local = 1;
    } else {
	m->port = 0;
	m->local = 0;
    }
    o_hdr->version = OSPF_VERSION;
    o_hdr->type = O_MON;
    o_hdr->length = htons(len);
    o_hdr->rtr_id = 0;
    o_hdr->area_id = 0;
    o_hdr->checksum = 0;
    o_hdr->Auth[0] = o_hdr->Auth[1] = 0;

    o_hdr->AuType = htons(auth ? OSPF_AUTH_SIMPLE : OSPF_AUTH_NONE);
    o_hdr->checksum = inet_cksum((caddr_t) packet, len);

    if (auth) {
	size_t al = strlen((char *) auth);

	if (al > OSPF_AUTH_SIZE) {
	    al = OSPF_AUTH_SIZE;
	}
	
	bcopy((caddr_t) auth, (caddr_t) o_hdr->Auth, al);
    }

    dst.sin_family = AF_INET;
    dst.sin_port = 0;
    dst.sin_addr.s_addr = (u_int32) to;
    if ((sendto(fd, (char *) packet, len, 0,
		(struct sockaddr *) & dst, dstlen)) < 0) {
	perror("txpkt sendto");
	close(fd);
	exit(1);
    }
    if (local_wait)
	print_local(localfp, to, name);
}


int
main __PF2(argc, int,
	   argv, char **)
{
    int sock, ndx, n;
    int bufndx = 0, curndx = 0;
    int length, cc, val;
    char buf[50][200];
    char dest_st[30], req_st[5], ndx_st[10], filename[30];
    char *name = 0, *lastname = 0;
    byte *auth = 0, *lastauth = 0;
    u_int32 p[MAXPARAMS], dest, lastdest = 0;
    struct sockaddr_in saddr;
    FILE *hp = (FILE *) NULL;
    struct in_addr addr;

    /* Init file descriptor */
    localfp = stdout;
    
    signal(SIGINT, got_int);
    /* create a raw Multicast socket */
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_OSPF)) < 0)
	ciao("> socket(AF_INET, SOCK_RAW) fails");

    /* create a stream socket for receiving info */
    if ((gwsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	ciao("> socket(AF_INET, SOCK_STREAM) fails");
    }
    cc = sizeof(val);
#ifdef SO_RCVBUF
    if (getsockopt(gwsock, SOL_SOCKET, SO_RCVBUF,
		   (char *) &val, &cc)) {
	perror("getsockopt");
	exit(1);
    }
#endif
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = 0;
    if (bind(gwsock, (struct sockaddr *) &saddr, sizeof(saddr))) {
	perror("bind: gw sock");
	exit(1);
    }
    length = sizeof(saddr);
    if (getsockname(gwsock, (struct sockaddr *) &saddr, &length)) {
	perror("getsockname failed");
	exit(1);
    }
    port = saddr.sin_port;
    listen(gwsock, 5);

    if (argc > 1)
	monconf(argv[1]);

    printf("[ %d ] dest command params > ", curndx);
    while (1) {
	bufndx = HNEXT(bufndx);
	curndx++;
	bzero(buf[bufndx], 200);
	if (!gets(buf[bufndx]))
	    exit(1);

	if (!strlen(buf[bufndx])) {	/* blank line */
	    ndx = curndx - 1;
	    goto history;
	}
	sscanf(buf[bufndx], "%s %s",
	       dest_st,
	       req_st);

	if (buf[bufndx][0] == '!') {
	    sscanf(&buf[bufndx][1], "%s", ndx_st);
	    if (buf[bufndx][0] == '!')
		ndx = curndx - 1;
	    else {
		ndx = atoi(ndx_st);
		if ((ndx > (curndx - 1)) ||
		    (ndx < (curndx + 1 - HISTORY_SIZE))) {
		    printf("Bad history index\n");
		    goto again;
		}
	    }
	history:
	    ndx = (ndx % HISTORY_SIZE);
	    sscanf(buf[ndx], "%s %s",
		   dest_st,
		   req_st);
	    bcopy(buf[ndx], buf[bufndx], 200);
	}
	if (isalpha(dest_st[0]) || ispunct(dest_st[0])) {
	    switch (dest_st[0]) {
	    case 'd':
		if (destcnt) {
		    for (ndx = 0; ndx < destcnt; ndx++) {
			printf("%d: %-15s %s\n",
			       ndx + 1,
			       lntoa(dtab[ndx]->dest),
			       dtab[ndx]->name);
		    }
		} else
		    printf("No destinations configured\n");
		break;

	    case 'h':
	    {
		int i;
		int start, end;

		/* wrap around? */
		if (curndx > (HISTORY_SIZE - 1)) {
		    start = curndx - HISTORY_SIZE;
		    end = curndx;
		} else {
		    start = 1;
		    end = curndx;
		}
		for (i = start;
		     i < end;
		     i++) {
		    printf("%d > ", i);
		    print_params(stdout,
				 buf[i % HISTORY_SIZE]);
		}
	    }
		break;

	    case '?':
		if (dest_st[1] == 'r' || dest_st[1] == 'R') {
		    remote_help();
		    break;
		}
		help();
		break;

	    case 'x':
		if (localfp != stdout) {
		    fclose(localfp);
		    if (hp)
			fclose(hp);
		}
		exit(1);

	    case '@':
		if (dest_st[1] == 0) {
		    if (lastdest) {
			name = lastname;
			dest = lastdest;
			auth = lastauth;
			goto lastd;
		    } else {
			printf("No last dest\n");
			goto again;
		    }
		} else if (isdigit(dest_st[1])) {
		    if (!destcnt) {
			printf("Destination table not configured\n");
			goto again;
		    }
		    ndx = atoi(&dest_st[1]);
		    if (ndx > 0 && ndx <= destcnt) {
			dest = dtab[ndx-1]->dest;
			name = dtab[ndx-1]->name;
			auth = dtab[ndx-1]->auth;
			lastdest = dest;
			lastname = name;
			lastauth = auth;
			goto lastd;
		    } else {
			printf("Invalid index %c\n", dest_st[1]);
			goto again;
		    }
		} else {
		    printf("Invalid index %c\n", dest_st[1]);
		    goto again;
		}

	    case 'F':
		n = sscanf(buf[bufndx], "%*s %s", filename);
		if (n < 0) {
		    printf("Filename not given\n");
		} else if ((localfp = fopen(filename, "w")) == (FILE *) NULL) {
		    fprintf(stderr, "Can't open %s\n",
			    filename);
		    localfp = stdout;
		}
		break;
	    case 'H':
		n = sscanf(buf[bufndx], "%*s %s", filename);
		if (n < 0) {
		    printf("Turning off history log\n");
		    if (hp) {
			fclose(hp);
			hp = (FILE *) NULL;
			}
		} else if ((hp = fopen(filename, "w")) == (FILE *) NULL) {
		    fprintf(stderr, "Can't open %s\n",
			    filename);
		    hp = (FILE *) NULL;;
		} else
		    printf("Turning on history log\n");
		break;

	    case 'S':
		if (localfp != stdout) {
		    fclose(localfp);
		    localfp = stdout;
		}
		break;

	    case 'q':
		goto exit;
	    }	
	    goto again;
	} else {
	    if (dest_st[0] == '\n' || !strlen(dest_st)) {
		bufndx = HLAST(bufndx);
		curndx--;
		goto again;
	    }
	    if (!inet_aton(dest_st, &addr)) {
		printf("   dest: (%s): unknown host \n", dest_st);
		goto again;
	    }
	    dest = addr.s_addr;
	    lastdest = dest;
	    lastname = name;
	    lastauth = auth;
	}
      lastd:
	if (!strlen(req_st)) {
	    printf("   Illegal request: %s\n\n", req_st);
	    help();
	    goto again;
	}
	if (hp)
	    print_params(hp, buf[bufndx]);
	txmon(sock, req_st[0], buf[bufndx], dest, name, auth);
      again:
	bzero((caddr_t) p, (MAXPARAMS * 4));
	dest = 0;
	name = (char *) NULL;
	bzero(dest_st, 30);
	bzero(req_st, 5);
	printf("[ %d ] dest command params > ", curndx);
    }

 exit:
    return 0;
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * ------------------------------------------------------------------------
 * 
 *                 U   U M   M DDDD     OOOOO SSSSS PPPPP FFFFF
 *                 U   U MM MM D   D    O   O S     P   P F
 *                 U   U M M M D   D    O   O  SSS  PPPPP FFFF
 *                 U   U M M M D   D    O   O     S P     F
 *                  UUU  M M M DDDD     OOOOO SSSSS P     F
 * 
 *     		          Copyright 1989, 1990, 1991
 *     	       The University of Maryland, College Park, Maryland.
 * 
 * 			    All Rights Reserved
 * 
 *      The University of Maryland College Park ("UMCP") is the owner of all
 *      right, title and interest in and to UMD OSPF (the "Software").
 *      Permission to use, copy and modify the Software and its documentation
 *      solely for non-commercial purposes is granted subject to the following
 *      terms and conditions:
 * 
 *      1. This copyright notice and these terms shall appear in all copies
 * 	 of the Software and its supporting documentation.
 * 
 *      2. The Software shall not be distributed, sold or used in any way in
 * 	 a commercial product, without UMCP's prior written consent.
 * 
 *      3. The origin of this software may not be misrepresented, either by
 *         explicit claim or by omission.
 * 
 *      4. Modified or altered versions must be plainly marked as such, and
 * 	 must not be misrepresented as being the original software.
 * 
 *      5. The Software is provided "AS IS". User acknowledges that the
 *         Software has been developed for research purposes only. User
 * 	 agrees that use of the Software is at user's own risk. UMCP
 * 	 disclaims all warrenties, express and implied, including but
 * 	 not limited to, the implied warranties of merchantability, and
 * 	 fitness for a particular purpose.
 * 
 *     Royalty-free licenses to redistribute UMD OSPF are available from
 *     The University Of Maryland, College Park.
 *       For details contact:
 * 	        Office of Technology Liaison
 * 		4312 Knox Road
 * 		University Of Maryland
 * 		College Park, Maryland 20742
 * 		     (301) 405-4209
 * 		FAX: (301) 314-9871
 * 
 *     This software was written by Rob Coltun
 *      rcoltun@ni.umd.edu
 */
