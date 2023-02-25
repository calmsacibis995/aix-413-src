static char sccsid[] = "@(#)99	1.25  src/tcpip/usr/sbin/snmpd/interfaces.c, snmp, tcpip411, 9436E411a 8/30/94 13:16:42";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_interfaces(), free_interfaces(), disable_interfaces(), 
 *            find_interfaces(), init_interfaces(), adr_compar(),
 *	      reinit_interfaces(), get_interfaces(), find_address(),
 *	      get_addrent(), s_ifAdminStatus(), get_ifDescr(),
 *	      ifa_ifwithnet(), inet_netmatch(), set_interface()
 *            check_interface(), trap_interface()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/interfaces.c
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

#include <isode/snmp/io.h>
#include "snmpd.h"
#include "interfaces.h"
#include "fddi.h"

#include <sys/ioctl.h>
#include <sys/str_tty.h>
#include <sys/comio.h>
#include <string.h>
#ifdef _POWER
#include <fcntl.h>
#include <sys/devinfo.h>
#include <sys/errids.h>
#include <sys/cdli_tokuser.h>
#include <sys/cdli_entuser.h>
#include <sys/cdli_fddiuser.h>
#include <sys/termio.h>
#endif

#ifdef DEBUG
extern void generic_print();
extern void token_ring_print();
extern void ethernet_print();
#endif /* DEBUG */

extern struct interface *get_fddi();
extern SMT_impl *SMT_list;
extern int smtnumber;
extern int macnumber;
extern int portnumber;
extern int pathnumber;

/*  */

#ifdef _POWER

#include <sys/cfgdb.h>		/* cfg db literals */
#include <sys/cfgodm.h>

/* MESSAGES */
#define	DS_MSG	"Product:"
#define	MF_MSG	"Manufacturer:"
#define	PN_MSG	"Part Number:"
#define	FN_MSG	"FRU Number:"

static struct key vpd_data[] = {
    { DS_MSG, "DS", NULL, NULL },	/* Displayable message - ascii */
    { MF_MSG, "MF", NULL, NULL },	/* Manufacturer - ascii */
    { PN_MSG, "PN", NULL, NULL },	/* Part number - ascii */
    { FN_MSG, "FN", NULL, NULL },	/* FRU Number - ascii */
};

#define N_VPDKEYS	(sizeof (vpd_data) / sizeof (struct key))

#endif /* _POWER */

/* FORWARD */
static void check_interface ();

int	ifNumber = 0;

struct interface *ifs = NULL;
struct interface *iftimers = NULL;

static struct address	 *afs = NULL;
struct address *afs_inet = NULL;

int	flush_if_cache = 0;
unsigned long baseifnet = 0;
#ifdef _POWER
unsigned long basendd = 0;
#endif /* _POWER */

/*  */

#define	IFINDEX		0
#define	IFDESCR		1
#define	IFTYPE		2
#define	IFMTU		3
#define	IFSPEED		4
#define	IFPHYSADDRESS	5
#define	IFADMINSTATUS	6
#define	IFOPERSTATUS	7
#ifdef	_POWER
#define	IFLASTCHANGE	8
#endif
#if	defined(_POWER) || defined(BSD44)
#define	IFINOCTETS	9
#endif
#define	IFINUCASTPKTS	10
#if	defined(_POWER) || defined(BSD44)
#define	IFINNUCASTPKTS	11
#define	IFINDISCARDS	12
#endif
#define	IFINERRORS	13
#if	defined(_POWER) || defined(BSD44)
#define	IFINUNKNOWNPROTOS 14
#define	IFOUTOCTETS	15
#endif
#define	IFOUTUCASTPKTS	16
#if	defined(_POWER) || defined(BSD44)
#define	IFOUTNUCASTPKTS	17
#endif
#define	IFOUTDISCARDS	18
#define	IFOUTERRORS	19
#define	IFOUTQLEN	20
#define	IFSPECIFIC	21

extern int ask_device();

static int  
o_interfaces (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifnum,
	    ifvar;
    register struct interface *is;
    register struct ifnet *ifn;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register ndd_t *ndd_entry;
#ifdef	IFLASTCHANGE
    static   int lastq = -1;
    static   integer diff;
#endif

    if (get_interfaces (offset) == NOTOK)
	return generr (offset);

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    ifnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    for (is = ifs; is; is = is -> ifn_next)
		if (is -> ifn_index == ifnum)
		    break;
	    if (is == NULL || !is -> ifn_ready)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		/* find first ready interface */
		for (is = ifs; is; is = is -> ifn_next)
		    if (is -> ifn_ready)
			break;
		if (!is)
		    return NOTOK;
		ifnum = is -> ifn_index;

		if ((new = oid_extend (oid, 1)) == NULLOID)
		    return NOTOK;
		new -> oid_elements[new -> oid_nelem - 1] = ifnum;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	i = ot -> ot_name -> oid_nelem;
		register struct interface *iz;

		if ((ifnum = oid -> oid_elements[i]) == 0) {
		    if ((is = ifs) == NULL)
			return NOTOK;
		    if (is -> ifn_ready)
			goto stuff_ifnum;
		    ifnum = 1;
		}
		for (is = iz = ifs; is; is = is -> ifn_next)
		    if ((iz = is) -> ifn_index == ifnum)
			break;
		for (is = iz -> ifn_next; is; is = is -> ifn_next)
		    if (is -> ifn_ready)
			break;
		if (!is)
		    return NOTOK;
stuff_ifnum: ;
		ifnum = is -> ifn_index;

		oid -> oid_elements[i] = ifnum;
		oid -> oid_nelem = i + 1;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }
    ifn = &is -> ifn_interface -> ac_if;
    ndd_entry = is -> ndd_entry;

    switch (ifvar) {
	case IFINDEX:
	    return o_integer (oi, v, is -> ifn_index);

	case IFDESCR:
	    return o_string (oi, v, is -> ifn_descr, strlen (is -> ifn_descr));

	case IFTYPE:
	    return o_integer (oi, v, is -> ifn_type);

	case IFMTU:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_mtu);
	    return o_integer (oi, v, (int)ndd_entry -> ndd_mtu);

	case IFSPEED:
	    {
		if (is -> ifn_type == IFT_SLIP)
		{
		    int fd;
		    int baudrate = 0;
		    char devname[20];

		    sprintf(devname, "/dev/tty%d",
			    is -> ifn_interface -> ac_if.if_unit);
		    fd = open(devname,O_RDWR|O_NDELAY);

		    if (fd < 0)
		        baudrate = 0;
		    else
		    {
		        struct termios  termio;
					
		        if (ioctl(fd,TIOCGETA,&termio)==-1)
			    baudrate = 0;
		        else
		        {
			    switch (termio.c_cflag&_CBAUD)
			    {
			        case B0 :
			    	    baudrate = 0;
				    break;
			        case B50 :
				    baudrate = 50;
				    break;
			        case B75 :
				    baudrate = 75;
				    break;
			        case B110 :
				    baudrate = 110;
				    break;
			        case B134 :
				    baudrate = 134;
				    break;
			        case B150 :
				    baudrate = 150;
				    break;
			        case B200 :
				    baudrate = 200;
				    break;
			        case B300 :
				    baudrate = 300;
				    break;
			        case B600 :
				    baudrate = 600;
				    break;
			        case B1200 :
				    baudrate = 1200;
				    break;
			        case B1800 :
				    baudrate = 1800;
				    break;
			        case B2400 :
				    baudrate = 2400;
				    break;
			        case B4800 :
				    baudrate = 4800;
				    break;
			        case B9600 :
				    baudrate = 9600;
				    break;
			        case B19200 :
				    baudrate=19200;
				    break;
			        case B38400 :
				    baudrate=38400;
				    break;
			        default :
				    baudrate = 0;
				    break;
			    }
		        }
		        close(fd);
		    }
		    is -> ifn_speed = baudrate;
		}
		
	        return o_integer (oi, v, is -> ifn_speed);
	    }

	case IFPHYSADDRESS:
	    if (is -> ndd_or_not == NDD_DRIVER)
		return o_string(oi,v,is->ndd_physaddr, ndd_entry->ndd_addrlen);
	    return o_string (oi, v,
			         (char *) is -> ifn_interface -> ac_enaddr,
#if	defined(BSD44)
			         ifn -> if_addrlen);
#else
#  if     defined(_POWER)
			         is -> ifn_interface -> ac_hwlen);
#  else
			         sizeof is -> ifn_interface -> ac_enaddr);
#  endif /* defined(_POWER) */
#endif /* defined(BSD44) */

	case IFADMINSTATUS:
	    return o_integer (oi, v, is -> ifn_admin);

	case IFOPERSTATUS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_flags & IFF_UP ? OPER_UP
							          : OPER_DOWN);
	    return o_integer (oi, v, OPER_UP);

#ifdef	IFLASTCHANGE
	case IFLASTCHANGE:
#ifdef BSD44
	    if (quantum != lastq) {
		struct timeval now;

		lastq = quantum;

		if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
		{
		    if (gettimeofday (&now, (struct timezone *) 0) == NOTOK)
		    {
			advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_1,
			                                "gettimeofday"));
		        return generr (offset);
		    }
		    diff = (now.tv_sec - ifn -> if_lastchange.tv_sec) * 100
		       + ((now.tv_usec - ifn -> if_lastchange.tv_usec) / 10000);
                }
	        else diff = ndd_entry -> ndd_genstats.ndd_elapsed_time;
	    }
#else
	    /*
	     * aix3.1 had its own way... 
	     * but of course it was never put in the kernel 
	     */
	    diff = ifn -> if_lastchange;	/* always 0 */
#endif
	    return o_number (oi, v, diff);
#endif	/* IFLASTCHANGE */

#ifdef	IFINOCTETS
	case IFINOCTETS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_ibytes);
	    return o_integer(oi, v, ndd_entry->ndd_genstats.ndd_ibytes_lsw);
#endif

	case IFINUCASTPKTS:
#ifndef	IFINNUCASTPKTS
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_ipackets);
	    return o_integer(oi,v, ndd_entry->ndd_genstats.ndd_ipackets_lsw);
#else
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn->if_ipackets - ifn->if_imcasts);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88025:
		    return o_integer(oi, v,
                	ndd_entry -> ndd_genstats.ndd_ipackets_lsw -
			((tok_genstats_t *)(is -> ndd_devspec)) -> mcast_recv -
			((tok_genstats_t *)(is -> ndd_devspec)) -> bcast_recv);

		case IFT_ETHER:
		case IFT_ISO88023:
		    return o_integer(oi, v,
	                ndd_entry -> ndd_genstats.ndd_ipackets_lsw -
			((ent_genstats_t *)(is -> ndd_devspec)) -> mcast_rx_ok -
			((ent_genstats_t *)(is -> ndd_devspec)) -> bcast_rx_ok);

		case IFT_FDDI:
		    return o_integer(oi, v,
			ndd_entry -> ndd_genstats.ndd_ipackets_lsw -
			((fddi_spec_stats_t *)(is->ndd_devspec))->mcast_rx_ok -
			((fddi_spec_stats_t *)(is->ndd_devspec))->bcast_rx_ok);

	 	default:
		    return o_integer(oi,v,
				     ndd_entry->ndd_genstats.ndd_ipackets_lsw);
	    }
#endif

#ifdef	IFINNUCASTPKTS
	case IFINNUCASTPKTS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_imcasts);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88025:
		    return o_integer(oi, v,
			((tok_genstats_t *)(is -> ndd_devspec)) -> mcast_recv +
			((tok_genstats_t *)(is -> ndd_devspec)) -> bcast_recv);

		case IFT_ISO88023:
		case IFT_ETHER:
		    return o_integer(oi, v,
			((ent_genstats_t *)(is -> ndd_devspec)) -> mcast_rx_ok +
			((ent_genstats_t *)(is -> ndd_devspec)) -> bcast_rx_ok);

		case IFT_FDDI:
		    return o_integer(oi, v,
			((fddi_spec_stats_t *)(is->ndd_devspec))->mcast_rx_ok +
			((fddi_spec_stats_t *)(is->ndd_devspec))->bcast_rx_ok);

		default:
		    return o_integer(oi, v, 0);
	    }
#endif

#ifdef	IFINDISCARDS
	case IFINDISCARDS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_iqdrops);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88023:
		case IFT_ETHER:
		    return o_integer (oi, v, 
			ndd_entry -> ndd_genstats.ndd_nobufs + 
			((ent_genstats_t *)(is->ndd_devspec)) -> no_resources +
			((ent_genstats_t *)(is->ndd_devspec)) -> rx_drop);

		case IFT_ISO88025:
	            return o_integer (oi, v, 
			((tok_genstats_t *)(is->ndd_devspec))->rx_congestion);

		case IFT_FDDI:
	            return o_integer (oi, v, 
			ndd_entry -> ndd_genstats.ndd_nobufs + 
			((fddi_spec_stats_t *)(is->ndd_devspec))->pframe_cnt);

		default:
	            return o_integer (oi,v,ndd_entry->ndd_genstats.ndd_nobufs);
		}
#endif

	case IFINERRORS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_ierrors);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88023:
		case IFT_ETHER:
		    return o_integer (oi, v, 
		((ent_genstats_t *)(is -> ndd_devspec)) -> rx_collisions +
		((ent_genstats_t *)(is -> ndd_devspec)) -> fcs_errs +
		((ent_genstats_t *)(is -> ndd_devspec)) -> align_errs +
		((ent_genstats_t *)(is -> ndd_devspec)) -> overrun +
		((ent_genstats_t *)(is -> ndd_devspec)) -> short_frames +
		((ent_genstats_t *)(is -> ndd_devspec)) -> long_frames);

		case IFT_ISO88025:
		case IFT_FDDI:
		default:
	            return o_integer (oi,v,ndd_entry->ndd_genstats.ndd_ierrors);
	    }

#ifdef	IFINUNKNOWNPROTOS
	case IFINUNKNOWNPROTOS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_noproto);
	    return o_integer (oi, v, 0);
#endif

#ifdef	IFOUTOCTETS
	case IFOUTOCTETS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_obytes);
	    return o_integer (oi,v,ndd_entry->ndd_genstats.ndd_obytes_lsw);
#endif

	case IFOUTUCASTPKTS:
#ifndef	IFOUTNUCASTPKTS
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_opackets);
	    return o_integer (oi, v,
			  ndd_entry->ndd_genstats.ndd_ifOutUcastPkts_lsw +
			  ndd_entry->ndd_genstats.ndd_ifOutMcastPkts_lsw +
			  ndd_entry->ndd_genstats.ndd_ifOutBcastPkts_lsw);
#else
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn->if_opackets - ifn->if_omcasts);
	    return o_integer (oi, v,
			  ndd_entry->ndd_genstats.ndd_ifOutUcastPkts_lsw);
#endif

#ifdef	IFOUTNUCASTPKTS
	case IFOUTNUCASTPKTS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_omcasts);
	    return o_integer (oi, v,
			  ndd_entry->ndd_genstats.ndd_ifOutMcastPkts_lsw +
			  ndd_entry->ndd_genstats.ndd_ifOutBcastPkts_lsw);
#endif

	case IFOUTDISCARDS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_snd.ifq_drops);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88023:
		case IFT_ETHER:
		case IFT_ISO88025:
		case IFT_FDDI:
		default:
		    return o_integer(oi, v, 
				     ndd_entry->ndd_genstats.ndd_xmitque_ovf);
	    }

	case IFOUTERRORS:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_oerrors);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88023:
		case IFT_ETHER:
		    return o_integer(oi, v,
		((ent_genstats_t *)(is -> ndd_devspec)) -> carrier_sense +
		((ent_genstats_t *)(is -> ndd_devspec)) -> underrun +
		((ent_genstats_t *)(is -> ndd_devspec)) -> cts_lost +
		((ent_genstats_t *)(is -> ndd_devspec)) -> excess_collisions +
		((ent_genstats_t *)(is -> ndd_devspec)) -> late_collisions +
		((ent_genstats_t *)(is -> ndd_devspec)) -> tx_timeouts);

		case IFT_ISO88025:
		case IFT_FDDI:
		default: 
		    return o_integer(oi,v,ndd_entry->ndd_genstats.ndd_oerrors);
	    }

	case IFOUTQLEN:
	    if (is -> ifn_offset || is -> ndd_or_not != NDD_DRIVER)
	        return o_integer (oi, v, ifn -> if_snd.ifq_len);

	    switch (is -> ifn_type)
	    {
		case IFT_ISO88023:
		case IFT_ETHER:
		    return o_integer(oi, v,
		((ent_genstats_t *)(is -> ndd_devspec)) -> sw_txq_len +
		((ent_genstats_t *)(is -> ndd_devspec)) -> hw_txq_len); 

		case IFT_ISO88025: 
		    return o_integer(oi, v, 
		((tok_genstats_t *)(is -> ndd_devspec)) -> sw_txq_len + 
		((tok_genstats_t *)(is -> ndd_devspec)) -> hw_txq_len);

		case IFT_FDDI:
		default:
		    return o_integer(oi, v,
				     ndd_entry -> ndd_genstats.ndd_xmitque_max);
	    }

	case IFSPECIFIC:
	    return o_specific (oi, v, (caddr_t) is -> ifn_specific);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*
 * NAME:  s_ifAdminStatus ()
 *
 * FUNCTION: sets the desired state of an interface.  This is only valid for
 *           non-NDD Drivers.
 *
 * RETURNS: int_SNMP_error__status_noError	(success) 
 *	    SNMP_error__status			(failure)
 */
static int  
s_ifAdminStatus (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int			ifnum, admin;
    register OID	oid = oi -> oi_name;
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    register struct interface *is;
    struct ifreq 	ifr;

    /* check that this interface instance exists */
    switch (offset) {
	case type_SNMP_PDUs_set__request:

	    /* only need to query the kernel once */
	    if (get_interfaces (type_SNMP_PDUs_get__request) == NOTOK) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_2,
	                "GENERR: s_ifAdminStatus: get_interfaces failed"));
		return int_SNMP_error__status_genErr;
	    }
	    /* FALL THRU */

	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    ifnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    for (is = ifs; is; is = is -> ifn_next)
		if (is -> ifn_index == ifnum)
		    break;
	    if (is == NULL || !is -> ifn_ready)
		return int_SNMP_error__status_noSuchName;
	    if (!is -> ifn_offset)
		return int_SNMP_error__status_noSuchName;
	    break;

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_3,
	            "s_ifAdminStatus: unexpected operation %d"), offset);
	    return int_SNMP_error__status_genErr;
    }
    if (os == NULLOS) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_4,
                "GENERR: s_ifAdminStatus: no syntax defined for \"%s\""),
		ot -> ot_text);
	return int_SNMP_error__status_genErr;
    }

    memset (&ifr, 0, sizeof (ifr));	/* clear request struct */
    switch (offset) {
	case type_SNMP_PDUs_set__request:
	    /* parse */
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;

	    /* check desired value */
	    admin = *((int *)(ot -> ot_save));
	    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if (admin < ADMIN_MIN || admin > ADMIN_MAX) 
		return int_SNMP_error__status_badValue;

	    /* Check that we can get the current flags */
	    strncpy (ifr.ifr_name, is -> ifn_name, sizeof (is -> ifn_name));
	    if (ioctl(Nd, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_5,
	 "GENERR: s_ifAdminStatus: ioctl (SIOCGIFFLAGS) for \"%s\" failed: %s"),
			ifr.ifr_name, strerror(errno));
		return int_SNMP_error__status_genErr;
	    }
#ifdef DEBUG
	    if (debug > 3) {
		advise (SLOG_DEBUG, "set s_ifAdminStatus on \"%s\" to %d",
			ifr.ifr_name, admin);
	    }
#endif /* DEBUG */
	    break;

	case type_SNMP_PDUs_commit:
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;

	    /* check desired value */
	    admin = *((int *)(ot -> ot_save));
	    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    /* get the current flags */
	    strncpy (ifr.ifr_name, is -> ifn_name, sizeof (is -> ifn_name));
	    if (ioctl(Nd, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_5,
        "GENERR: s_ifAdminStatus: ioctl (SIOCGIFFLAGS) for \"%s\" failed: %s"),
			ifr.ifr_name, strerror(errno));
		return int_SNMP_error__status_genErr;
	    }

	    /* implement action */
	    switch (admin) {
		case ADMIN_UP:				/* UP */
		    ifr.ifr_flags |= IFF_UP;
		    break;
		case ADMIN_DOWN:			/* DOWN */
		    ifr.ifr_flags &= ~IFF_UP;
		    break;
		case ADMIN_TEST:			/* TESTING */
		    /* ACTION not implemented */
		    break;
	    }

	    /* commit new flags */
	    if (ioctl (Nd, SIOCSIFFLAGS, (caddr_t)&ifr) < 0) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_5,
        "GENERR: s_ifAdminStatus: ioctl (SIOCSIFFLAGS) for \"%s\" failed: %s"),
			ifr.ifr_name, strerror(errno));
		return int_SNMP_error__status_genErr;
	    }

	    /* commit new internal value */
	    is -> ifn_admin = admin;
	    is -> ifn_interface -> ac_if.if_flags = ifr.ifr_flags;
	    check_interface (is);		/* link Up/Down trap needed? */
	    break;

	case type_SNMP_PDUs_rollback:
	    /* No work to be done here */
	    break;
    }

    return int_SNMP_error__status_noError;

}

static void
free_interfaces ()
{
    register struct interface *ip, *is;
    int i;
    struct fdinfo *fi;

    for (is = ifs; is; is = ip) {
	ip = is -> ifn_next;
	if (is -> environ)
	  free (is -> environ);
	if (is -> datacache)
	  free (is -> datacache);
	if (is -> datarcvtable)
	  free (is -> datarcvtable);
	if (is -> setrequests)
	  free (is -> setrequests);
	if (is -> setpositions)
	  free (is -> setpositions);
	if (is -> ifn_interface)
	  free(is -> ifn_interface);

	if (is -> ndd_devspec)
	  free(is -> ndd_devspec);
	if (is -> ndd_physaddr)
	  free(is -> ndd_physaddr);
	if (is -> ndd_entry)
	  free(is -> ndd_entry);
	if (is -> ifn_descr)
	    free (is -> ifn_descr);
	if (is -> ifn_specific)
	    oid_free(is -> ifn_specific);
	free ((char *) is);
    }

    for (i = 0; i < smtnumber; i++)
    {
	token_path *ptr = SMT_list[i].token_paths;

	if (SMT_list[i].msgtosend)
	    free_SMT_frame(SMT_list[i].msgtosend, 1);
	if (SMT_list[i].macidnumbers)
	    free(SMT_list[i].macidnumbers);
	if (SMT_list[i].portidnumbers)
	    free(SMT_list[i].portidnumbers);
	if (SMT_list[i].pathidnumbers)
	    free(SMT_list[i].pathidnumbers);

	while (ptr)
	{
	    token_path *ptr2 = ptr;

	    ptr = ptr -> next;
	    free(ptr2);
	}
    }
    free(SMT_list);
    smtnumber = 0;
    macnumber = 0;
    pathnumber = 0;
    portnumber = 0;

    ifs = NULL;
    iftimers = NULL;
}

static void
disable_interfaces ()
{
    advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_6, 
            "interfaces group disabled"));
    free_interfaces ();
}

#ifdef _POWER

/*
 * This procedure takes the interfaces name, e.g. tr0, and gets the uniquetype
 * name from the PdDv database.  The uniquetype is used to get the entries
 * if_keyword from the PdAt database, e.g. tok.  This is used to get the 
 * device string from CuVPD.
 */
static char *
get_ifDescr (if_name, if_unit, if_type)
char	*if_name;	/* (bare) interface name */
int	if_unit;	/* unit number */
int	if_type;	/* MIB interface type */
{
    struct PdDv	*PdDv;
    struct PdAt	*PdAt;
    struct CuVPD *CuVPD;
    struct listinfo PdDv_info, PdAt_info, CuVPD_info;
    char criteria[128];
    char	buf[1024], *outbuf;
    int		bufl;
	
    sprintf (buf, "%s%d", if_name, if_unit);	/* bare minimum info. */

    /*
     * search PdDv to get the uniquetype key for this
     * ifnet interface.
     */
    sprintf (criteria, "prefix = %s", if_name);
    PdDv = get_PdDv_list (PdDv_CLASS, criteria, &PdDv_info, 1, 1);
    if (PdDv_info.num < 1) {	/* no matches */
	odm_free_list (PdDv, &PdDv_info);
	return strdup (buf);
    }

    /*
     * using uniquetype, search PdAt for an if_keyword so that
     * we can match up with a hardware interface.
     */
    sprintf (criteria, "uniquetype = %s and attribute = if_keyword", 
	    PdDv->uniquetype);
    odm_free_list (PdDv, &PdDv_info);	/* done with PdDv */
    PdAt = get_PdAt_list (PdAt_CLASS, criteria, &PdAt_info, 1, 1);
    if (PdAt_info.num < 1) {	/* no matches */
	odm_free_list (PdAt, &PdAt_info);
	return strdup (buf);
    }

    /*
     * If deflt field == "none", assume no hardware mapping 
     * This is true for lo and sl interfaces.
     */
    if (!strcmp (PdAt->deflt, "none")) {
	odm_free_list (PdAt, &PdAt_info);
	return strdup (buf);
    }

    /*
     * Finally, search CuVPD for an entry 
     * NOTE: assume that if interface name ends in an integer, we must
     *       first insert "s" before the if_unit.  Currently this only
     *	     applies to x25.
     */
    sprintf(criteria, "name = %s%s%d and vpd_type = %d", PdAt->deflt, 
	    isdigit (PdAt->deflt[strlen (PdAt->deflt) - 1]) ? "s" : "",
	    if_unit, HW_VPD);
    odm_free_list (PdAt, &PdAt_info);	/* done with PdAt */
    CuVPD = get_CuVPD_list(CuVPD_CLASS, criteria, &CuVPD_info, 1, 1); 
    if (CuVPD_info.num < 1) {	/* no matches */
	odm_free_list (CuVPD, &CuVPD_info);
	return strdup (buf);
    }

    bufl = strlen (buf);	/* length of minimal info... */

    /* finally got our info.  Now parse it */
    parse_VPD (CuVPD->vpd, buf, vpd_data, N_VPDKEYS);
    odm_free_list (CuVPD, &CuVPD_info);	/* done with CuVPD */

    if ((outbuf = (char *)malloc (strlen (buf) + bufl + 10)) == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),"get_ifDescr");

    /* minimal info, plus full blown ifdescr */
    sprintf (outbuf, "%s%d; %s", if_name, if_unit, buf);
    return outbuf;
}
#endif /* _POWER */

static int
find_interfaces ()
{
    int				i;
    struct ifnet 	       *ifnet;
    register struct interface  *is,
			      **ifp,
			      **ift;
    struct nlist 		nzs;
    register struct nlist *nz = &nzs;
    char 		       *cp;
    ndd_t		       *nddlist;
    int	    			if_unit;
    char			devname[20];

    ifp = &ifs;
    ift = &iftimers;
    i = 0;

    /* ndd list base */
    if (getkmem (nl + N_NDD, (caddr_t) &nddlist, sizeof nddlist) == NOTOK) {
	disable_interfaces ();
	return NOTOK;
    }
    basendd = (unsigned long)nddlist;

    for (; nddlist; i++) {
	ndd_t *nddtemp;
	caddr_t ndd_phys;
	caddr_t ndd_devspec;

	if ((is = (struct interface *) calloc (1, sizeof *is)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                 "find_interfaces");

	if ((nddtemp = (ndd_t *) calloc (1, sizeof *nddtemp)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                 "find_interfaces");

	is -> ndd_offset = (unsigned long) nddlist;
	is -> ndd_entry = nddtemp;

	nz -> n_name = "ndd_t", nz -> n_value = is -> ndd_offset;
	if (getkmem (nz, (caddr_t) nddtemp, sizeof *nddtemp) == NOTOK) {
	    free(nddtemp);
	    free(is);
	    disable_interfaces ();
	    return NOTOK;
	}

	nz -> n_name = "ndd_name",
	nz -> n_value = (unsigned long) nddtemp -> ndd_name;
	if (getkmem (nz, (caddr_t) is -> ndd_name, sizeof is -> ndd_name - 1)
		 == NOTOK) 
	{
	    free(nddtemp);
	    free(is);
	    disable_interfaces ();
	    return NOTOK;
	}

	nz -> n_name = "ndd_alias";
	nz -> n_value = (unsigned long) nddtemp -> ndd_alias;
	if (nz -> n_value)
	{
	    if (getkmem (nz, (caddr_t) is -> ndd_alias, sizeof is->ndd_alias-1)
		 == NOTOK) 
	    {
		free(nddtemp);
		free(is);
		disable_interfaces ();
		return NOTOK;
	    }
	}
	else is -> ndd_alias[0] = 0;

	nddlist = nddtemp -> ndd_next;

	if (nddtemp -> ndd_type != IFT_ETHER &&
	    nddtemp -> ndd_type != IFT_ISO88023 &&
	    nddtemp -> ndd_type != IFT_ISO88025 &&
	    nddtemp -> ndd_type != IFT_FDDI)
	{
	    free(nddtemp);
	    free(is);
	    i--;
	    continue;
	}

	if ((ndd_devspec = (caddr_t) calloc (1, nddtemp->ndd_speclen)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                 "find_interfaces");
	is -> ndd_devspec = ndd_devspec;

	nz -> n_name = "ndd_specstats",
	nz -> n_value = (unsigned long) nddtemp -> ndd_specstats;
	if (getkmem (nz, (caddr_t) is -> ndd_devspec, nddtemp -> ndd_speclen)
		 == NOTOK) 
	{
	    free(ndd_devspec);
	    free(nddtemp);
	    free(is);
	    disable_interfaces ();
	    return NOTOK;
	}

	if ((ndd_phys = (caddr_t) calloc (1, nddtemp -> ndd_addrlen)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                 "find_interfaces");
	is -> ndd_physaddr = ndd_phys;

	nz -> n_name = "ndd_physaddr",
	nz -> n_value = (unsigned long) nddtemp -> ndd_physaddr;
	if (getkmem (nz, (caddr_t) is -> ndd_physaddr, nddtemp -> ndd_addrlen)
		 == NOTOK) 
	{
	    free(ndd_phys);
	    free(nddtemp);
	    free(ndd_devspec);
	    free(is);
	    disable_interfaces ();
	    return NOTOK;
	}

	is -> ifn_index = i + 1;
	is -> ifn_indexmask = 1 << i;
	is -> ndd_or_not = NDD_DRIVER;
	is -> ifn_type = nddtemp -> ndd_type;
	is -> ifn_flags = 0;
	is -> ifn_speed = 0;
	is -> last_q = quantum - 2;
	is -> mib_envt = ENVTNOTOK;
	is -> flush_cache = 1;
	is -> flush_rcv = 1;
	is -> timeout = 0;
	is -> datarcvtable = NULL;
	is -> setrequests = NULL;
	is -> setpositions = NULL;
	is -> setsize = 0;
	is -> time_next = NULL;
	is -> fi_fd = -1;
	is -> ifn_offset = 0;
	is -> ifn_next = NULL;

	switch (is -> ifn_type) {
	    case IFT_ETHER:
	    case IFT_ISO88023: 
	        is -> ifn_speed = 10000000;
		if ((is -> environ =
		       (combo_mibs_t *)calloc(1, sizeof(combo_mibs_t)))
		    == NULL)
	          adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                       "find_interfaces");
		if ((is -> datacache =
		       (combo_mibs_t *)calloc(1, sizeof(combo_mibs_t)))
		    == NULL)
	          adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                       "find_interfaces");

                is -> ifn_specific = text2oid("dot3");

		break;
	    case IFT_FDDI:
	        is -> ifn_speed = 100000000; 
		if ((is -> environ =
		       (combo_mibs_t *)calloc(1, sizeof(combo_mibs_t)))
		    == NULL)
	          adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                       "find_interfaces");
		if ((is -> datacache =
		       (combo_mibs_t *)calloc(1, sizeof(combo_mibs_t)))
		    == NULL)
	          adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                       "find_interfaces");

                is -> ifn_specific = text2oid("fddi");

		break;
	    case IFT_ISO88025:
		if ((is -> environ = 
		       (combo_mibs_t *)calloc(1, sizeof(combo_mibs_t)))
		   == NULL)
	          adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                       "find_interfaces");
		if ((is -> datacache = 
		       (combo_mibs_t *)calloc(1, sizeof(combo_mibs_t)))
		   == NULL)
	          adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                       "find_interfaces");

                is -> ifn_specific = text2oid("dot5");
		break;

	    default:
                is -> ifn_specific = oid_cpy(nullSpecific);
	        break;
	}

	if (ask_device(is, QUERY) == OK) 
	{
	    if (is -> ifn_type == IFT_ISO88025)
	    {
		if (ask_device(is, GET) == OK)
		  is -> ifn_speed = is -> datacache ->
			     token_ring_all.Token_ring_mib.Dot5Entry.ring_speed;
		else is -> ifn_speed = TR_MIB_UNKNOWN;
		
		switch (is -> ifn_speed)
		{
		    case TR_MIB_ONEMEGABIT:
			is -> ifn_speed = 1000000;
			break;

		    case TR_MIB_FOURMEGABIT:
			is -> ifn_speed = 4000000;
			break;

		    case TR_MIB_SIXTEENMEGABIT:
			is -> ifn_speed = 16000000;
			break;

		    case TR_MIB_UNKNOWN:
		    default:
			is -> ifn_speed = 0;
		}
             }
	}

        if (is -> ifn_type == IFT_FDDI)
	{
	      struct interface *ptr;

	      ptr = get_fddi(is);
	      if (ptr)
		  *ift = ptr, ift = &ptr -> time_next;
	}

	is -> ndd_name[sizeof is -> ndd_name - 1] = NULL;
	if ((cp = strpbrk(is -> ndd_name, "0123456789")) == NULL)
	    if_unit = 0; /* Unknown number */
	else 
	{
	    if_unit = atoi(cp);
	    *cp = NULL;
	}
    	is -> ifn_descr = get_ifDescr (is -> ndd_name, if_unit, 
		is -> ifn_type);
	(void) sprintf (is -> ndd_name + strlen (is -> ndd_name), "%d", 
			if_unit);

	is -> ifn_admin = OPER_UP;
	is -> ifn_ready = 1;

	if (debug)
	    advise (SLOG_EXCEPTIONS,
		    "add interface %d: %s 0x%x with mask 0x%x alias %s",
		    is -> ifn_index, is -> ndd_name, is -> ifn_offset,
		    is -> ifn_indexmask, is -> ndd_alias);

	*ifp = is, ifp = &is -> ifn_next;
	*ift = is, ift = &is -> time_next;
    }

    /* ifnet base */
    if (getkmem (nl + N_IFNET, (caddr_t) &ifnet, sizeof ifnet) == NOTOK) {
	disable_interfaces ();
	return NOTOK;
    }
    baseifnet = (unsigned long)ifnet;

    while (ifnet) 
    {
	register struct ifnet *ifn;
	struct arpcom *ifnphy;
	struct interface *is2;

	if ((is = (struct interface *) calloc (1, sizeof *is)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                 "find_interfaces");

	if ((ifnphy = (struct arpcom *) calloc (1, sizeof *ifnphy)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                 "find_interfaces");

        is -> ifn_interface = ifnphy;
	ifn = &is -> ifn_interface -> ac_if;

	is -> ifn_offset = (unsigned long) ifnet;

	nz -> n_name = "struct ifnet", nz -> n_value = is -> ifn_offset;
	if (getkmem (nz, (caddr_t) ifn, sizeof *is -> ifn_interface) == NOTOK) {
	    free(ifnphy);
	    free(is);
	    disable_interfaces ();
	    return NOTOK;
	}
	ifnet = ifn -> if_next;

	nz -> n_name = "if_name",
	nz -> n_value = (unsigned long) ifn -> if_name;
	if (getkmem (nz, (caddr_t) is -> ifn_name, sizeof is -> ifn_name - 1)
		 == NOTOK) 
	{
	    free(ifnphy);
	    free(is);
	    disable_interfaces ();
	    return NOTOK;
	}

        /* 
	 * Currently, it is believed that the ndd_alias and the ifname
	 *       will be the same.
 	 */
        is2 = ifs; 
	sprintf(devname, "%s%d", is -> ifn_name, ifn -> if_unit);
	while (is2)
	{
	    if ((strcmp(is2 -> ndd_name, devname) == 0) || 
	    	(strcmp(is2 -> ndd_alias, devname) == 0))
	    {
		is2 -> ifn_offset = is -> ifn_offset;
		is2 -> ifn_interface = ifnphy;
		strcpy(is2 -> ifn_name, is -> ifn_name);
		free(is);
		break;
	    }
	    is2 = is2 -> ifn_next;
	}

        if (!is2)
	{
	    is -> ndd_or_not = NOT_NDD_DRIVER;
	    is -> ndd_entry = NULL;
	    is -> mib_envt = ENVTNOTOK;
	    is -> flush_cache = 1;
	    is -> flush_rcv = 1;
	    is -> timeout = 0;
	    is -> ifn_type = ifn -> if_type;
	    is -> datarcvtable = NULL;
	    is -> setrequests = NULL;
	    is -> setpositions = NULL;
	    is -> setsize = 0;
            is -> environ = NULL;
	    is -> datacache = NULL;
	    is -> ifn_specific = oid_cpy(nullSpecific);
	    is -> ifn_index = i + 1;
	    is -> ifn_indexmask = 1 << i;
	    i++;
	    is -> last_q = quantum - 2;
	    is -> ifn_next = NULL;
	    *ifp = is, ifp = &is -> ifn_next;
	}
	else is = is2;

	ifn = &is -> ifn_interface -> ac_if;
	is -> ifn_flags = ifn -> if_flags;

	if (is -> ndd_or_not != NDD_DRIVER)
	    is -> ifn_speed = ifn -> if_baudrate;

#ifdef _POWER
    	is -> ifn_descr = get_ifDescr (is -> ifn_name, ifn -> if_unit, 
		ifn -> if_type);
#endif
	(void) sprintf (is -> ifn_name + strlen (is -> ifn_name), "%d",
			ifn -> if_unit);

	/* 
	 * assume a well-behaved BSD44 interface driver has if_addrlen
	 * set to the proper value, ie, 0 if no physAddress.
	 */

#if	!defined(BSD44)
	if (is -> ifn_type <= IFT_OTHER || is -> ifn_type == IFT_LOOP){
	    bzero ((char *) is -> ifn_interface.ac_enaddr,
		   sizeof is -> ifn_interface.ac_enaddr);
#  if     defined(_POWER)
	    is -> ifn_interface.ac_hwlen = 6;	/* ala bsd4.3 arpcom */
#  endif	/* defined(_POWER) */
	}
#endif	/* !defined(BSD44) */

	is -> ifn_admin = OPER_UP;

	is -> ifn_ready = 1;
	if (debug)
	    advise (SLOG_DEBUG, MSGSTR(MS_INTER, INTER_8,
		    "add interface %d: %s 0x%x with mask 0x%x"),
		    is -> ifn_index, is -> ifn_name, is -> ifn_offset,
		    is -> ifn_indexmask);
    }
    ifNumber = i;

    return OK;
}

init_interfaces () 
{
    register OT	    ot;

    find_interfaces ();

    if (ot = text2obj ("ifNumber")) 
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &ifNumber;

    (void) get_interfaces (type_SNMP_PDUs_get__request);

    if (ot = text2obj ("ifIndex"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINDEX;
    if (ot = text2obj ("ifDescr"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFDESCR;
    if (ot = text2obj ("ifType"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFTYPE;
    if (ot = text2obj ("ifMtu"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFMTU;
    if (ot = text2obj ("ifSpeed"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFSPEED;
    if (ot = text2obj ("ifPhysAddress"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFPHYSADDRESS;
    if (ot = text2obj ("ifAdminStatus"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_setfnx = s_ifAdminStatus;		/* setable! */
	ot -> ot_info = (caddr_t) IFADMINSTATUS;
    if (ot = text2obj ("ifOperStatus"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOPERSTATUS;
#ifdef	IFLASTCHANGE
    if (ot = text2obj ("ifLastChange"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFLASTCHANGE;
#endif
#ifdef	IFINOCTETS
    if (ot = text2obj ("ifInOctets"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINOCTETS;
#endif
    if (ot = text2obj ("ifInUcastPkts"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINUCASTPKTS;
#ifdef	IFINNUCASTPKTS
    if (ot = text2obj ("ifInNUcastPkts"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINNUCASTPKTS;
#endif
#ifdef	IFINDISCARDS
    if (ot = text2obj ("ifInDiscards"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINDISCARDS;
#endif
    if (ot = text2obj ("ifInErrors"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINERRORS;
#ifdef	IFINUNKNOWNPROTOS
    if (ot = text2obj ("ifInUnknownProtos"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFINUNKNOWNPROTOS;
#endif
#ifdef	IFOUTOCTETS
    if (ot = text2obj ("ifOutOctets"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOUTOCTETS;
#endif
    if (ot = text2obj ("ifOutUcastPkts"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOUTUCASTPKTS;
#ifdef	IFOUTNUCASTPKTS
    if (ot = text2obj ("ifOutNUcastPkts"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOUTNUCASTPKTS;
#endif
    if (ot = text2obj ("ifOutDiscards"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOUTDISCARDS;
    if (ot = text2obj ("ifOutErrors"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOUTERRORS;
    if (ot = text2obj ("ifOutQLen"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFOUTQLEN;
    if (ot = text2obj ("ifSpecific"))
	ot -> ot_getfnx = o_interfaces,
	ot -> ot_info = (caddr_t) IFSPECIFIC;
}

/*  */

static int  
adr_compar (a, b)
register struct address **a,
			**b;
{
    int    i;

    if ((i = (*a) -> adr_address.sa.sa_family
	 	- (*b) -> adr_address.sa.sa_family))
	return (i > 0 ? 1 : -1);

    return elem_cmp ((*a) -> adr_instance, (*a) -> adr_insize,
		     (*b) -> adr_instance, (*b) -> adr_insize);
}

#ifdef DYNAMIC_INTERFACES
static int Reinit = OK;

static int
reinit_interfaces ()
{
    int		rc;

    Reinit = NOTOK;			/* lock out following code */
    free_interfaces ();
    if ((rc = find_interfaces ()) == OK) 
	rc = get_interfaces (type_SNMP_PDUs_get__request);
    Reinit = OK;

    /* 
     * Send a cold start trap to notify managers of our interfaces changes.
     */
    do_trap (int_SNMP_generic__trap_coldStart, 0,
	    (struct type_SNMP_VarBindList *) 0);

    /* 
     * our view vs a manager's view of the interface table has changed...
     * don't give them the requested information (it may be incorrect).
     * But be nice, send back a genErr.
     */
    return int_SNMP_error__status_genErr;
}
#endif /* DYNAMIC_INTERFACES */

int
get_interfaces (offset)
int	offset;
{
    int	    adrNumber = 0;
    register struct interface  *is;
    register struct address    *as,
			       *ap,
			      **base,
			      **afe,
			      **afp;
    static   int first_time = 1;
    static   int lastq = -1;
    static   int tic = 0;
#ifdef DYNAMIC_INTERFACES
    struct	ifnet	*ifnetp;
    ndd_t	*nddp;
#endif /* DYNAMIC_INTERFACES */

    /* multi-request packet? */
    if (quantum == lastq)
	return OK;

    /*	walk thru interfaces?   Cache up to ifEntry.22 * ifNumber calls */
    if (!flush_if_cache
	    && offset == type_SNMP_PDUs_get__next__request
	    && quantum == lastq + 1
	    && tic < (IFENTRIES * ifNumber)) {		/* XXX: caching! */
	lastq = quantum;
	tic ++;
	return OK;
    }
    lastq = quantum, flush_if_cache = 0, tic = 0;

    for (as = afs; as; as = ap) {
	ap = as -> adr_next;

	free ((char *) as);
    }
    afs = afs_inet = NULL;

    afp = &afs;

#ifdef DYNAMIC_INTERFACES
    /* 
     * check if the base ifnet pointer has changed.  Should never happen.
     */
    if (Reinit == OK) {
	if (getkmem (nl + N_IFNET, (caddr_t) &ifnetp, sizeof ifnetp) == NOTOK) {
	    disable_interfaces ();
	    return NOTOK;
	}
	if ((unsigned long)ifnetp != baseifnet) {
	    advise (SLOG_NOTICE, MSGSTR(MS_INTER, INTER_9,
	            "base ifnet pointer changed. re-initializing interfaces"));
	    return reinit_interfaces ();
	}

	if (getkmem (nl + N_NDD, (caddr_t) &nddp, sizeof nddp) == NOTOK) {
	    disable_interfaces ();
	    return NOTOK;
	}
	if ((unsigned long)nddp != basendd) {
	    advise (SLOG_NOTICE, MSGSTR(MS_INTER, INTER_18,
		    "base ndd pointer changed. re-initializing interfaces"));
	    return reinit_interfaces ();
	}
    }
#endif /* DYNAMIC_INTERFACES */

    /* 
     *	loop thru each interfaces that we already know about.
     */
    for (is = ifs; is; is = is -> ifn_next) {
	struct arpcom ifns;
	register struct ifnet *ifn = &ifns.ac_if;
	ndd_t		ndd_space;
	register ndd_t *ndd_entry = &ndd_space;
#if	defined(BSD43) || defined(RT)
	struct ifaddr ifaddr;
	register struct ifaddr *ifa;
#ifdef	BSD44
	union sockaddr_un ifsocka,
			  ifsockb;
#endif
	union sockaddr_un ifsockc;
	register union sockaddr_un *ia,
				   *ib;
	register union sockaddr_un *ic = &ifsockc;
#endif
#ifndef	BSD44
	struct ifreq ifreq;
#endif
	struct nlist nzs;
	register struct nlist *nz = &nzs;

	if (is -> ndd_or_not == NDD_DRIVER)
	{
	    caddr_t ndd_phys;
	    caddr_t ndd_devspec;

	    nz -> n_name = "ndd_t", nz -> n_value = is -> ndd_offset;
	    if (getkmem (nz, (caddr_t) ndd_entry, sizeof *ndd_entry) == NOTOK)
	        return NOTOK;

	    if (Reinit == OK) {
		/*
	 	 * if an extra pointer is hanging at the end of the ifnet chain,
		 * or our next pointers don't match, then we must re-initialize
		 */
		if (is -> ndd_entry -> ndd_next != ndd_entry -> ndd_next)
		{
		    advise (SLOG_NOTICE, MSGSTR(MS_INTER, INTER_10,
		     "interface pointers changed. re-initializing interfaces"));
		    return reinit_interfaces ();
		}
	    }
	    /* Update ndd entry with new one */
	    *(is -> ndd_entry) = ndd_space;

	    if ((ndd_devspec = (caddr_t)calloc(1,is->ndd_entry->ndd_speclen)) 
		 == NULL)
	        adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                     "get_interfaces");
	    if (is -> ndd_devspec)
		free (is -> ndd_devspec);
	    is -> ndd_devspec = ndd_devspec;

	    nz -> n_name = "ndd_specstats",
	    nz -> n_value = (unsigned long) is -> ndd_entry -> ndd_specstats;
	    if (getkmem(nz,(caddr_t)is->ndd_devspec,is->ndd_entry->ndd_speclen)
		 == NOTOK) 
	    {
		disable_interfaces ();
		return NOTOK;
	    }

	    if ((ndd_phys = (caddr_t) calloc (1, is->ndd_entry->ndd_addrlen)) 
		== NULL)
	        adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	                     "get_interfaces");
	    if (is -> ndd_physaddr)
		free (is -> ndd_physaddr);
	    is -> ndd_physaddr = ndd_phys;

	    nz -> n_name = "ndd_physaddr",
	    nz -> n_value = (unsigned long) is -> ndd_entry -> ndd_physaddr;
	    if (getkmem(nz,(caddr_t)is->ndd_physaddr,is->ndd_entry->ndd_addrlen)
		 == NOTOK) 
	    {
		disable_interfaces ();
		return NOTOK;
	    }
	}

	if (is -> ifn_interface != NULL)
	{
	    nz -> n_name = "struct ifnet", nz -> n_value = is -> ifn_offset;
	    if (getkmem (nz, (caddr_t) ifn, sizeof ifns) == NOTOK)
	        return NOTOK;

#ifdef DYNAMIC_INTERFACES
	    if (Reinit == OK) 
	    {
	        /*
	         * if an extra pointer is hanging at the end of the ifnet chain,
	         * or our next pointers don't match, then we must re-initialize
	         */
	        if (is -> ifn_interface -> ac_if.if_next != ifn -> if_next)
	        {
		    advise (SLOG_NOTICE, MSGSTR(MS_INTER, INTER_10,
		     "interface pointers changed. re-initializing interfaces"));
		    return reinit_interfaces ();
	        }
	    }
#endif /* DYNAMIC_INTERFACES */

#ifndef	BSD44
	    ia = (union sockaddr_un *) &ifaddr.ifa_addr,
	    ib = (union sockaddr_un *) &ifaddr.ifa_broadaddr;
#else
	    ia = &ifsocka, ib = &ifsockb;
#endif	/* BSD44 */

	    for (ifa = ifn -> if_addrlist; ifa; ifa = ifaddr.ifa_next) 
	    {
	        nz -> n_name = "struct ifaddr",
	        nz -> n_value = (unsigned long) ifa;
	        if (getkmem (nz, (caddr_t) &ifaddr, sizeof ifaddr) == NOTOK)
		    continue;
#ifndef	BSD44
	        if (ia -> sa.sa_family == AF_UNSPEC)
	  	    continue;

	        if (Nd != NOTOK) {
		    (void) strcpy (ifreq.ifr_name, is -> ifn_name);
		    if (ioctl (Nd, SIOCGIFNETMASK, (char *) &ifreq) == NOTOK) 
		    {
		        if (debug)
			    advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_11,
			            "SIOCGIFNETMASK on %s"), is -> ifn_name);
		        bzero ((char *) ic, sizeof *ic);
		    }
		    ic -> sa = ifreq.ifr_addr;	/* struct copy */
	        }
	        else bzero ((char *) ic, sizeof *ic);

#else	/* else BSD44 */
	        nz -> n_name = "union sockaddr_un",
	        nz -> n_value = (unsigned long) ifaddr.ifa_addr;

	        /* XXX: ifaddr struct without an ifa_addr associated ?? */
	        if (nz -> n_value == 0)
	 	    continue;

	        if (getkmem (nz, (caddr_t) ia, sizeof *ia) == NOTOK)
		    continue;

	        if (ia -> sa.sa_family == AF_UNSPEC)
		    continue;

	        if (ia -> sa.sa_family == AF_INET) 
		{
		    nz -> n_value = (unsigned long) ifaddr.ifa_broadaddr;
		    if (getkmem (nz, (caddr_t) ib, sizeof *ib) == NOTOK)
		        continue;
	        }

	        nz -> n_value = (unsigned long) ifaddr.ifa_netmask;
	        /* XXX: ifaddr struct without an ifa_addr associated ?? */
	        if (nz -> n_value == 0)
	 	    continue;
	        if (getkmem (nz, (caddr_t) ic, sizeof *ic) == NOTOK)
		    continue;
#endif	/* BSD44 */
	        if (ia -> sa.sa_family == AF_INET
		        && ic -> un_in.sin_addr.s_addr == 0) 
		{
		    if (IN_CLASSA (ia -> un_in.sin_addr.s_addr))
		        ic -> un_in.sin_addr.s_addr = IN_CLASSA_NET;
		    else
		        if (IN_CLASSB (ia -> un_in.sin_addr.s_addr))
			    ic -> un_in.sin_addr.s_addr = IN_CLASSB_NET;
		        else ic -> un_in.sin_addr.s_addr = IN_CLASSC_NET;
	        }

	        if (as = find_address (ia))
		    as -> adr_indexmask |= is -> ifn_indexmask;
	        else {
		    if ((as=(struct address *) calloc (1, sizeof *as)) == NULL)
		        adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
		                     "get_interfaces");
		    *afp = as, afp = &as -> adr_next, adrNumber++;

		    as -> adr_address = *ia;		/* struct copy */
		    if (ia -> sa.sa_family == AF_INET)
		        as -> adr_broadaddr = *ib;	/* struct copy */
		    as -> adr_netmask = *ic;		/*   .. */

		    as -> adr_indexmask = is -> ifn_indexmask;

		    switch (ia -> sa.sa_family) {
		        case AF_INET:
			    as -> adr_insize =
			        ipaddr2oid (as -> adr_instance,
					    &ia -> un_in.sin_addr);
			    if (afs_inet == NULL)  /* needed for find_address */
			        afs_inet = as;
			    break;

		        default:
			    bzero ((char *) as -> adr_instance,
			           sizeof as -> adr_instance);
			    as -> adr_insize = 0;
			    break;
		    }
	        }
	    }

	    *(is -> ifn_interface) = ifns;	/* struct copy */

	    /* 
	     * assume a well-behaved BSD44 interface driver has if_addrlen
	     * set to the proper value, ie, 0 if no physAddress.
	     */

#if	!defined(BSD44)
	    if (is -> ifn_type <= IFT_OTHER || is -> ifn_type == IFT_LOOP)
	    {
	        bzero ((char *) is -> ifn_interface.ac_enaddr,
		       sizeof is -> ifn_interface.ac_enaddr);
#  if     defined(_POWER)
	        is -> ifn_interface.ac_hwlen = 6;	/* ala bsd4.3 arpcom */
#  endif	/* defined(_POWER) */
	    }
#endif	/* !defined(BSD44) */
	}

	check_interface (is);		/* check if we went up/down */
    }

    /* count the number of interfaces */
    ifNumber = 0;
    for (is = ifs; is; is = is -> ifn_next) {
	    ifNumber += (is -> ifn_ready = 1);
    }

    if (debug && first_time) {
	first_time = 0;

	for (as = afs; as; as = as -> adr_next) {
	    OIDentifier	oids;

	    oids.oid_elements = as -> adr_instance;
	    oids.oid_nelem = as -> adr_insize;
	    advise (SLOG_DEBUG, MSGSTR(MS_INTER, INTER_12,
		    "add address: %d/%s with mask 0x%x"),
		    as -> adr_address.sa.sa_family, sprintoid (&oids),
		    as -> adr_indexmask);
	}
    }

    if (adrNumber <= 1)
	return OK;

    if ((base = (struct address **)
		    malloc ((unsigned) (adrNumber * sizeof *base))) == NULL)
	adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"),
	             "get_interfaces");

    afe = base;
    for (as = afs; as; as = as -> adr_next)
	*afe++ = as;

#ifdef _ANSI_C_SOURCE
    qsort ((char *) base, adrNumber, sizeof *base, 
	   (int(*)(const void *, const void *))adr_compar);
#else /* !_ANSI_C_SOURCE */
    qsort ((char *) base, adrNumber, sizeof *base, adr_compar);
#endif /* !_ANSI_C_SOURCE */

    afp = base;
    as = afs = *afp++;
    afs_inet = NULL;
    while (afp < afe) {
	switch (as -> adr_address.sa.sa_family) {
	    case AF_INET:
	        if (afs_inet == NULL)
		    afs_inet = as;
	        break;
	}

	as -> adr_next = *afp;
	as = *afp++;
    }
    switch (as -> adr_address.sa.sa_family) {
        case AF_INET:
	    if (afs_inet == NULL)
		afs_inet = as;
	    break;

    }
    as -> adr_next = NULL;

    free ((char *) base);

    return OK;
}

/*
 * send off an interface link Up/Down trap.
 * included is the interface index (ifIndex).
 */
static void
trap_interface (is, trapType)
struct interface  *is;
int trapType;
{
    struct type_SNMP_VarBindList *bind;
    struct type_SNMP_VarBind	*v;
    OI oi;
    char buf[20];

    sprintf (buf, "ifIndex.%d", is -> ifn_index);
    if ((oi = text2inst (buf)) == NULLOI) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_13, 
                "trap_interface: lost OI for %s"), buf);
	return;
    }

    /* build up our varbind */
    if ((bind = (struct type_SNMP_VarBindList *)
	    calloc (1, sizeof *bind)) == NULL) {
no_mem:;
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
	        "%s: Out of Memory"), "trap_interface");
	goto out;
    }

    if ((v = (struct type_SNMP_VarBind *) calloc (1, sizeof *v)) == NULL) 
	goto no_mem;
    bind -> VarBind = v;

    if ((v -> name = oid_cpy (oi -> oi_name)) == NULLOID) 
	goto no_mem;

    if (o_integer (oi, v, is -> ifn_index) != int_SNMP_error__status_noError) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_14,
                "trap_interface: o_integer failed"));
	goto out;
    }

    do_trap (trapType, 0, bind);

out:;
    if (bind)
	free_SNMP_VarBindList (bind);
}

static void
check_interface (is)
struct interface  *is;
{
    register struct ifnet *ifn = &is -> ifn_interface -> ac_if;

    if (is -> ifn_flags == ifn -> if_flags)	/* no change in status */
	return;

    if (!is -> ifn_offset)
	return;

    /* did we go up or down? */
    if ((is -> ifn_flags & IFF_UP) && !(ifn -> if_flags & IFF_UP))
	trap_interface (is, int_SNMP_generic__trap_linkDown);
    else if (!(is -> ifn_flags & IFF_UP) && (ifn -> if_flags & IFF_UP))
	trap_interface (is, int_SNMP_generic__trap_linkUp);

    is -> ifn_flags = ifn -> if_flags;		/* update status */
}

/*  */

struct address *
find_address (addr)
register union sockaddr_un *addr;
{
    register struct address *as;
    register struct in_addr *in;

    switch (addr -> sa.sa_family) {
	case AF_INET:
	    in = &addr -> un_in.sin_addr;
	    for (as = afs_inet; as; as = as -> adr_next)
		if (as -> adr_address.sa.sa_family != AF_INET)
		    break;
		else
		    if (bcmp ((char *) in,
			      (char *) &as -> adr_address.un_in.sin_addr,
			      sizeof *in) == 0)
			return as;
	    break;

	default:
	    break;
    }

    return NULL;
}

/*  */

struct address *
get_addrent (ip, len, head, isnext)
register unsigned int *ip;
int	len;
struct address *head;
int	isnext;
{
    int	    family = 0;
    register struct address *as;

    if (head)
	family = head -> adr_address.sa.sa_family;
    for (as = head; as; as = as -> adr_next)
	if (as -> adr_address.sa.sa_family != family)
	    break;
	else
	    switch (elem_cmp (as -> adr_instance, as -> adr_insize, ip, len)) {
		case 0:
		    if (!isnext)
			return as;
		    if ((as = as -> adr_next) == NULL
		            || as -> adr_address.sa.sa_family != family) {
			flush_if_cache = 1;
			return NULL;
		    }
		    /* else fall... */

		case 1:
		    return (isnext ? as : NULL);
	    }

    flush_if_cache = 1;

    return NULL;
}

/*
 * NAME: ifa_ifwithnet ()
 *
 * NOTE: idea lifted from kernel code, if.c
 * Purpose:	find an interface that this address can go out over.
 *		Used to avoid ENETUNREACH error on sets.  Maps
 *		arptable entries to a specific interface.
 */
struct address *
ifa_ifwithnet (addr)
struct in_addr addr;
{
    register struct address *as;

    for (as = afs_inet; as; as = as -> adr_next) {
	if (as -> adr_address.sa.sa_family != AF_INET)
	    continue;
	if (inet_netmatch (as -> adr_address.un_in.sin_addr, addr))
	    return as;
    }
    return ((struct address *)0);
}

/*
 * NAME: inet_netmatch ()
 *
 * NOTE: lifted from kernel code, in.c  Changed to use inet_netof, 
 * instead of in_netof.
 */
int
inet_netmatch (addr1, addr2)
	struct in_addr addr1, addr2;
{
	return (inet_netof(addr1) == inet_netof(addr2));
}

#ifndef	_POWER
/*
 * NAME: set_interface ()
 *
 * PURPOSE: add user config data to the interfaces table.
 */
set_interface (name, ava)
char   *name,
       *ava;
{
    int	    i;
    u_long  l;
    register char   *cp;
    register struct interface *is;

    for (is = ifs; is; is = is -> ifn_next)
	if (strcmp (is -> ifn_name, name) == 0)
	    break;
    if (!is) {
        advise (SLOG_DEBUG, MSGSTR(MS_INTER, INTER_15,
                "no such interface as \"%s\""), name);
	return;
    }

    if ((cp = index (ava, '=')) == NULL)
	return;
    *cp++ = NULL;

    if (lexequ (ava, "ifType") == 0) {
	if (sscanf (cp, "%d", &i) != 1 || i < TYPE_MIN || i > TYPE_MAX) {
malformed: ;
            advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_16,
                        "malformed attribute \"%s = %s\""), ava, cp);
	    return;
	}

	switch (is -> ifn_type = i) {
	    case IFT_ETHER:
	    case IFT_ISO88023:
	        is -> ifn_speed = 10000000;
		break;

	    default:
	        break;
	}
	return;
    }

    if (lexequ (ava, "ifSpeed") == 0) {
	if (sscanf (cp, "%u", &l) != 1)
	    goto malformed;

	is -> ifn_speed = l;
	return;
    }
    
    if (lexequ (ava, "ifAdminStatus") == 0) {
	if (sscanf (cp, "%d", &i) != 1 || i < ADMIN_MIN || i > ADMIN_MAX)
	    goto malformed;

	is -> ifn_admin = i;
	return;
    }

    advise (SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_17,
                "unknown attribute \"%s = %s\""), ava, cp);
}
#endif	/* ! _POWER */

