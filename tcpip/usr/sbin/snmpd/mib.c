static char sccsid[] = "@(#)04	1.9  src/tcpip/usr/sbin/snmpd/mib.c, snmp, tcpip411, GOLD410 1/21/94 17:27:28";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: init_mib(), fin_mib(), getkmem(), showmib(), newset(),
 *            find_set(), ifvar(), free_set()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/mib.c
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */


#include "snmpd.h"
#if	defined (BSD42) || defined (_POWER)
#include <sys/file.h>
#endif
#ifdef	SYS5
#include <fcntl.h>
#endif

#if	defined(AIX) && !defined(L_SET) 	/* aix on RT */
#  define L_SET 0
#endif

/*    DATA */

static	int	kd;


struct nlist nl[] = {
#if	defined(_POWER) || defined(ps2)
#define NELEM_NLIST 16
    { "arptabp" },
    { "arptabsize" },
    { "icmpstat" },
    { "ifnet" },
    { "ipstat" },
    { "lbolt" },
    { "lbolt" },
    { "lbolt" },
    { "tcb" },
    { "tcpstat" },
    { "udb" },
    { "udpstat" },
    { "rtstat" },
#ifdef ps2
    { "ipforwarding" },		/* not readable in aix3.1 */
#else
	/* NOTE: dummy for ipforwarding, _POWER.  Is really boottime. */
    { "lbolt" },		/* what ever happened to boottime? */
#endif	/* ps2 */
#ifdef	BSD44
    { "radix_node_head" },
#endif
    { "ndd" },
#else
    { "_arptab" },
    { "_arptab_size" },
    { "_icmpstat" },
    { "_ifnet" },
    { "_ipstat" },
    { "_rthashsize" },
    { "_rthost" },
    { "_rtnet" },
    { "_tcb" },
    { "_tcpstat" },
    { "_udb" },
    { "_udpstat" },
    { "_rtstat" },
    { "_ipforwarding" },
#ifdef	BSD44
    { "_radix_node_head" },
    { "_iso_systype" },
    { "_clnp_stat" },
    { "_esis_stat" },
#endif
#endif /* _POWER */
    NULL
};

struct timeval my_boottime;
OID    nullSpecific = NULLOID;

/*  */

/*
 * 	NAME:   init_mib ()
 *
 *	PURPOSE:
 *		nlist the kernel, and check that every symbol requested
 *		has a value.
 */
init_mib ()
{
    register struct nlist *nz;

#ifdef _POWER
    errno = 0;

    if (knlist (nl, NELEM_NLIST, sizeof (struct nlist)) == NOTOK && errno)
#else
    if (nlist (UNIX, nl) == NOTOK)
#endif
	adios (MSGSTR(MS_MIB, MIB_7, "%s, cannot nlist"), UNIX);

    for (nz = nl; nz -> n_name; nz++)
	if (nz -> n_value == 0)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_1,
		    "\"%s\" not in %s (warning)"), nz -> n_name, UNIX);

    if ((kd = open (KMEM, O_RDONLY)) == NOTOK)
	adios (MSGSTR(MS_MIB, MIB_8, "%s, cannot read"), KMEM);

    if ((nullSpecific = text2oid ("0.0")) == NULLOID)
	adios (MSGSTR(MS_MIB, MIB_9, "text2oid(\"0.0\") failed"));
}

/*
 *	NAME:   fin_mib () 
 *
 *	PURPOSE:
 *		check that every variable configured to respond
 *		with the generic get function has a preconfigured value.
 */
fin_mib () 
{
    register OT	    ot;

    for (ot = text2obj ("ccitt"); ot; ot = ot -> ot_next)
	if (ot -> ot_status == OT_MANDATORY
	        && ot -> ot_getfnx == o_generic
	        && ot -> ot_info == NULL)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_2,
		    "variable \"%s.0\" has no value (warning)"), ot -> ot_text);

    if (gettimeofday (&my_boottime, (struct timezone *) 0) == NOTOK) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_3, "gettimeofday"));
	bzero ((char *) &my_boottime, sizeof my_boottime);
    } 
}

#ifdef DEBUG
/* 
 *	NAME:   showmib () 
 *
 *	PURPOSE:
 *		function which will show what variables this agent is 
 *		configured to respond to.
 */
showmib () 
{
    register OT	    ot;

    for (ot = text2obj ("ccitt"); ot; ot = ot -> ot_next)
	if (ot -> ot_status == OT_MANDATORY
	        && ot -> ot_getfnx != NULLIFP)
	    /* NOT IN CATALOG, BECAUSE OF DEBUG */
	    advise (SLOG_DEBUG,
		    "registered variable: \"%s.0\"", ot -> ot_text);
}
#endif /* DEBUG */

int	
getkmem (n, buffer, cc)
struct nlist *n;
caddr_t	buffer;
int	cc;
{
    if (n -> n_value == 0) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_1,
		"\"%s\" not in %s (warning)"), n -> n_name, UNIX);
	return NOTOK;
    }
    if (lseek (kd, (long) n -> n_value, L_SET) == NOTOK) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_4,
		"lseek of 0x%x for \"%s\" in kmem failed"),
		(long) n -> n_value, n -> n_name);
	return NOTOK;
    }
    if (read (kd, buffer, cc) != cc) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_5,
		"read of \"%s\" from kmem failed"),
		n -> n_name);
	return NOTOK;
    }

    return OK;
}

/*
 *	TABLE SET functions
 */
struct settab *
new_set (head, ifvar)
struct settab *head;
int ifvar;
{
    struct settab *st;

    if ((st = (struct settab *) calloc (1, sizeof *st)) == NULL) {
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "settab");
    }
    st -> st_ifvar = ifvar;
    insque (st, head -> st_back);
    return st;
}

struct settab *
find_set (head, ifvar)
struct settab *head;
int ifvar;
{
    struct settab *st;

    for (st = head -> st_forw; st != head; st = st -> st_forw) 
	if (st -> st_ifvar == ifvar)
	    return st;
    return ((struct settab *) NULL);
}

int
parse_set (ost, head, ifvar, os, v)
struct settab	**ost,
		*head;
int ifvar;
OS os;
struct type_SNMP_VarBind *v;
{
    struct settab *st;

    /* locate an existing stab entry, or create a new one */
    if ((*ost = st = find_set (head, ifvar)) != (struct settab *) NULL) {
	if (st -> st_value) {
	    (*os -> os_free) (st -> st_value);
	    st -> st_value = NULL;
	}
    }
    else if ((*ost = st = new_set (head, ifvar)) == (struct settab *) NULL) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_MIB, MIB_6,
		"GENERR: parse_set: new_set failed"));
	return int_SNMP_error__status_genErr;
    }

    /* parse */
    if ((*os -> os_decode) (&st -> st_value, v -> value) == NOTOK)
	return int_SNMP_error__status_badValue;
    return int_SNMP_error__status_noError;
}

void
free_set (head, ifvar, os)
struct settab *head;
int ifvar;
OS os;
{
    struct settab *st;

    if ((st = find_set (head, ifvar)) == NULL)
	return;
    if (st -> st_value) 
	(*os -> os_free) (st -> st_value);
    remque (st);
    free (st);
}
