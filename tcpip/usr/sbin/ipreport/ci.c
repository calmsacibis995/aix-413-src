static char sccsid[] = "@(#)86	1.5  src/tcpip/usr/sbin/ipreport/ci.c, tcpras, tcpip411, GOLD410 10/8/93 10:25:23";
/*
 * COMPONENT_NAME: TCPIP ci.c
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
 * This file contains routines to handle the call info list. We keep
 * track of call data so that when the reply comes we can know what it
 * was a reply to, and therefore how to decode it. 
 * 
 * Created Sat Aug 10 1991
 * This is all new code.
 * 
 */



#include <rpc/rpc.h>
#include <sys/time.h>
#include <arpa/inet.h>
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
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
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
#include <sys/types.h>
#include "ipr.h"

/* FUTURE BUG FIX 1:
    NLM_PROGs, and maybe others, never reply to any rpc call.
    Therefore we don't really need to save nlmprog things. We could
    change this code to ignore those guys (thereby breaking modularity
    since it doesn't even know what programs it's saving now,) or have
    the caller not call callsave in those cases.

    If we're running out of malloc space ipreporting a large nfslock
    iptrace this mite b the soln.


   FUTURE BUG FIX 2:
    nfsread may eventually need to have passed otw arg info included
    in the cinfo structure to be smarter about reporting read or
    readdir transactions that go over maxpacketsize

   FUTURE BUG FIX 3:
    The dumpcilist routine could easily print out prog and proc names.
*/

/* make a malloc'd copy of the cinfo struct */
struct cinfo *cinfocopy(cinfo)
struct cinfo *cinfo;
{
	struct cinfo *ci;
	
	/* copy the struct */
	ci=mymalloc(sizeof (struct cinfo));
	bcopy(cinfo,ci,(sizeof (struct cinfo)));
	return(ci);
}


struct cinfo *headcinfo=NULL;	/* cinfo list */

/*        The reply to an rpc call has no indication as to what
    program or procedure the reply is for. Therefore if we want to
    print out the data, other than in a hex dump format, that is in
    that reply, we need to keep a record of which call it is an answer
    to. There will be cases where we have a reply which we never saw
    the call for. We have no choice but to do a hex dump in those
    cases. This routine keeps a record of all the calls.... A call is
    uniquely determined by a reqid and originating hostname.
*/
callsave(cinfo)
struct cinfo *cinfo;
{
	struct cinfo *ci;
/*
	The cinfo struct we are passed will be overwritten later.
	We need to make a malloc'd copy.
*/
	ci= cinfocopy(cinfo);
	addtocilist(ci);
}

/* find a cinfo in the global list, called by reply_dump */
struct cinfo *getci(machine,reqid)
u_long machine;
int  reqid;
{
	struct cinfo *ci;

	for (ci=headcinfo; ci; ci=ci->ci_next)
		if ((ci->ci_reqid==reqid) && (ci->ci_orighost==machine)) 
			return (ci);
	return(0);
}


addtocilist(thing)
struct cinfo *thing;
{
	/* Stick it at the beginning of the list. 
	   This could be made faster if need be.
	   If it is hashed or sorted, then getci will have to change too.
	*/

	thing->ci_next=headcinfo;
	headcinfo=thing;
}

/* this routine could easily print out prog and proc names */
dumpcilist()
{
	struct cinfo *ci;

	printf("Summary of RPC CALL packets\n");

	for (ci=headcinfo; ci; ci=ci->ci_next)
	{
		printf("reqid=%d prog=%d proc=%d caller=%s\n",
			ci->ci_reqid,
			ci->ci_program,
			ci->ci_procedure,
			inet_ntoa(ci->ci_orighost));
	}
}
