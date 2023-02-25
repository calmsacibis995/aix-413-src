/* @(#)15	1.16  src/tcpip/usr/sbin/snmpd/snmpd.h, snmp, tcpip411, 9434A411a 8/18/94 14:32:36 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: generr
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/snmpd.h
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* system */
/*  we need to know the byte order (BYTE_ORDER) of the machine for 
    various tcp/ip files.  This define is located in different files
    across the different OS's,  and is usually included automatically
    by the tcp/ip files.  For portability, we will not explicitly
    include the machine file unless we have to.   (RLF) */

#ifndef _SNMPD_H_
#define _SNMPD_H_

#if	defined(BSD44) && !defined(_POWER)
#include <machine/machparam.h>
#endif

#include <stdio.h>
#include <errno.h>

/* XXX:  If _BSD is defined, the ps2 will use an
   incompatible version of "struct nlist"!!! */

#if	defined(ps2)
#undef	_BSD
#endif
#include <nlist.h>
#if	defined(ps2)
#define	_BSD
#endif
#include <sys/time.h>

/* isode library */
#include <isode/snmp/objects.h>	/* includes psap.h, manifest.h, and general.h */

/* local encode/decode */
#include <isode/pepsy/SMI-types.h>
#include <isode/pepsy/SNMP-types.h>

#include <isode/snmp/logging.h>	/* local logging routines */
#include "snmpd_msg.h"

extern errno;

#define SNMPDCONF	"/etc/snmpd.conf"

/* straight from mib-2 */
struct snmpstat {
    int	s_inpkts;
    int	s_outpkts;
    int	s_badversions;
    int	s_badcommunitynames;
    int	s_badcommunityuses;
    int	s_asnparseerrs;
    int	s_totalreqvars;
    int	s_totalsetvars;

    /* input error codes */
    int	s_intoobigs;
    int	s_innosuchnames;
    int	s_inbadvalues;
    int s_inreadonlys;
    int	s_ingenerrs;

    /* input packet types */
    int	s_ingetrequests;
    int	s_ingetnexts;
    int	s_insetrequests;
    int	s_ingetresponses;
    int	s_intraps;

    /* output error codes */
    int	s_outtoobigs;
    int	s_outnosuchnames;
    int	s_outbadvalues;
    int	s_outgenerrs;

    /* output packet types */
    int	s_outgetrequests;
    int	s_outgetnexts;
    int	s_outsetrequests;
    int	s_outgetresponses;
    int	s_outtraps;

    int	s_enableauthentraps;
#define	TRAPS_ENABLED	1			/* snmpEnableAuthenTraps */
#define	TRAPS_DISABLED	2			/*   .. */
};

/* for readconfig() */

#define	PASS1	0
#define	PASS2	1

/* default select timeout (in seconds), for checking interfaces */
#define D_ICHECKTIME	60
#define MINTIMEOUT	30

/* minimum packet size. 255 octets for displayString, plus msg body. */
#define MINPACKET	300

/* default smux and other timeouts */
#define D_SMUXTIME	15
#define D_DEVTIME	30

/* ask_query variables */
#define QUERY		 1
#define GET		 2

/* MIB section */

/* number of entries in the various tables, for cache control */
#define IFENTRIES		22
#define IPROUTEENTRIES		11
#define IPNETTOMEDIAENTRIES	4
#define TCPCONNENTRIES		5
#define UDPENTRIES		2

/* maximum length allowed for a displayString */
#define MAXDISPLAYLEN          256

/*  */

#if	defined(AIX) || defined(_AIX)
#define UNIX	"/unix"
#else
#define UNIX	"/vmunix"
#endif
#define KMEM	"/dev/kmem"

#define	generr(offset)	((offset) == type_SNMP_PDUs_get__next__request \
				    ? NOTOK : int_SNMP_error__status_genErr)

extern struct nlist nl[];
#define	N_ARPTAB	0		/* Now arptabp */
#define	N_ARPTAB_SIZE	1
#define	N_ICMPSTAT	2
#define	N_IFNET		3
#define	N_IPSTAT	4
#define	N_RTHASHSIZE	5		/* Not used */
#define	N_RTHOST	6		/* Not used */
#define	N_RTNET		7		/* Not used */
#define	N_TCB		8
#define	N_TCPSTAT	9
#define	N_UDB		10
#define	N_UDPSTAT	11
#define	N_RTSTAT	12
#define	N_IPFORWARDING	13
#ifdef	BSD44
#define	N_RADIX_NODE_HEAD 14
#define	N_ISO_SYSTYPE	15
#define	N_CLNP_STAT	16
#define	N_ESIS_STAT	17
#endif
#define N_NDD		15

/* MESSAGES */
#define NA_MSG	"not available!"

#ifdef _POWER

struct key {
    char *mesg;
    char *keyword;  /* keyword we are looking for in VPD */
    char *value;    /* its final value */
    int (*func) ();
};
#endif

/*
 *	set table.  Intended for sets on table entries, ala
 *	atEntry, ipNetToMediaEntry, ipRouteEntry.
 */
struct settab {
    struct settab	*st_forw;	/* doubly linked list */
    struct settab	*st_back;	/*         ...        */
    caddr_t		st_value;	/* saved value */
    int			st_ifvar;	/* variable */
};
#define ST_NOSET	0			/* nothing happening yet */
#define ST_INSET	0x01			/* set in-progress */
#define ST_ISSET	0x02			/* set already performed */
#define ST_DELETE	0x04			/* delete some old entry  */
#define ST_ADDSET	0x08			/* add/set an entry */
#define ST_NEWSET	0x10			/* table-add  */
#define ST_COMPLETE	0x20			/* table-add entry complete */
#define ST_EXISTS	0x40			/* table-entry exists */

struct settab *find_set (), *new_set ();
void free_set ();

/*  */

extern	int	quantum;
extern  int	Nd;
extern  struct timeval my_boottime;
extern	OID    nullSpecific;

extern	void	adios (), advise ();

#endif /* _SNMPD_H_ */
