static char sccsid[] = "@(#)06	1.2  src/tcpip/usr/sbin/snmpd/token_ring.c, snmp, tcpip411, GOLD410 3/22/94 15:07:44";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/token_ring.c
 */

/* INCLUDES */
#include "snmpd.h"
#include "ddmibs.h"
#include "interfaces.h"

#ifdef DEBUG
void print_token_ring();
extern void print_list();
#endif /* DEBUG */

extern struct interface *ifs;

extern int open_device();       /* Defined in devices.c */
extern int ask_device();
extern int set_device();

extern int tokenringtimeout;    /* Defined in config.c */
extern void update_timers();    /* Defined in timers.c */
extern int SNMP_PDU_variable_position; /* Defined in snmpd.c */

#define MAXUPSTREAM	6
#define MAXFUNCTIONAL	6

#define DOT5IFINDEX 1
#define DOT5COMMANDS 2
#define DOT5RINGSTATUS 3
#define DOT5RINGSTATE 4
#define DOT5RINGOPENSTATUS 5
#define DOT5RINGSPEED 6
#define DOT5UPSTREAM 7
#define DOT5ACTMONPARTICIPATE 8
#define DOT5FUNCTIONAL 9

#define DOT5STATSIFINDEX 10
#define DOT5STATSLINEERRORS 11
#define DOT5STATSBURSTERRORS 12
#define DOT5STATSACERRORS 13
#define DOT5STATSABORTTRANSERRORS 14
#define DOT5STATSINTERNALERRORS 15
#define DOT5STATSLOSTFRAMEERRORS 16
#define DOT5STATSRECEIVECONGESTIONS 17
#define DOT5STATSFRAMECOPIEDERRORS 18
#define DOT5STATSTOKENERRORS 19
#define DOT5STATSSOFTERRORS 20
#define DOT5STATSHARDERRORS 21
#define DOT5STATSSIGNALLOSS 22
#define DOT5STATSTRANSMITBEACONS 23
#define DOT5STATSRECOVERYS 24
#define DOT5STATSLOBEWIRES 25
#define DOT5STATSREMOVES 26
#define DOT5STATSSINGLES 27
#define DOT5STATSFREQERRORS 28

#define DOT5TIMERIFINDEX 29
#define DOT5TIMERRETURNREPEAT 30
#define DOT5TIMERHOLDING 31
#define DOT5TIMERQUEUEPDU 32
#define DOT5TIMERVALIDTRANSMIT 33
#define DOT5TIMERNOTOKEN 34
#define DOT5TIMERACTIVEMON 35
#define DOT5TIMERSTANDBYMON 36
#define DOT5TIMERERRORREPORT 37
#define DOT5TIMERBEACONTRANSMIT 38
#define DOT5TIMERBEACONRECEIVE 39

/* 
 * Conversions is a lookup table index by the above #defines to make easier 
 * set code.  The values in conversions are values that the drivers are 
 * supposed to recognize 
 */
int conversions[] =
{
	0,
	0, /* ifindex */
	TOKEN_COMMANDS,
	TOKEN_RING_STATUS,
	TOKEN_RING_STATE,
	TOKEN_RING_OSTATUS,
	TOKEN_RING_SPEED,
	TOKEN_UPSTREAM,
	TOKEN_FUNCTIONAL,
	0, /* ifindex */
	TOKEN_LINE_ERRS,
	TOKEN_BURST_ERRS,
	TOKEN_AC_ERRS,
	TOKEN_ABORT_ERRS,
	TOKEN_INT_ERRS,
	TOKEN_LOSTFRAMES,
	TOKEN_RX_CONGESTION,
	TOKEN_FRAMECOPIES,
	TOKEN_TOKEN_ERRS,
	TOKEN_SOFT_ERRS,
	TOKEN_HARD_ERRS,
	TOKEN_SIGNAL_LOSS,
	TOKEN_TX_BEACONS,
	TOKEN_RECOVERYS,
	TOKEN_LOBEWIRES,
	TOKEN_REMOVES,
	TOKEN_SINGLES,
	TOKEN_FREQ_ERRS,
	0, /* ifindex */
	TOKEN_RETURN_REPEAT,
	TOKEN_HOLDING,
	TOKEN_QUEUE_PDU,
	TOKEN_VALID_TX,
	TOKEN_NO_TOKEN,
	TOKEN_ACTIVE_MON,
	TOKEN_STANDBY_MON,
	TOKEN_ERR_REPORT,
	TOKEN_BEACON_TX,
	TOKEN_BEACON_RX
};

/*  */

static int o_tokenring (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    int when;
    token_ring_mib_t *ptr;
    token_ring_mib_t *ptr2;
    register struct interface *is;

    ifvar = (int) ot -> ot_info;

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
	      return int_SNMP_error__status_noSuchName;
	    ifnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    for (is = ifs; is; is = is -> ifn_next)
	      if (is -> ifn_index == ifnum)
		break;
	    if (is == NULL || !is -> ifn_ready || 
	        is -> ifn_type != IFT_ISO88025 || 
		is -> ndd_or_not == NOT_NDD_DRIVER)
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      /* find first ready interface */
	      for (is = ifs; is; is = is -> ifn_next)
		if ((is -> ifn_ready) && (is -> ifn_type == IFT_ISO88025) &&
		    (is -> ndd_or_not == NDD_DRIVER))
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
              int     i = ot -> ot_name -> oid_nelem;
	      register struct interface *iz;
	      OID new;

	      if ((i >= oid -> oid_nelem) ||
		  (ifnum = oid -> oid_elements[i]) == 0) {
	        if ((is = ifs) == NULL)
	          return NOTOK;
		if ((is -> ifn_ready) && (is -> ifn_type == IFT_ISO88025) &&
		    (is -> ndd_or_not == NDD_DRIVER))
	          goto stuff_ifnum;
		ifnum = 1;
	      }
	      for (is = iz = ifs; is; is = is -> ifn_next)
		if ((iz = is) -> ifn_index == ifnum)
		  break;
	      for (is = iz -> ifn_next; is; is = is -> ifn_next)
		if ((is -> ifn_ready) && (is -> ifn_type == IFT_ISO88025) &&
		    (is -> ndd_or_not == NDD_DRIVER))
	          break;
	      if (!is)
		return NOTOK;
stuff_ifnum: ;
	      ifnum = is -> ifn_index;

	      if ((new = oid_extend (ot -> ot_name, 1)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 1] = ifnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
            break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    if (!is -> flush_cache &&
	(is -> last_q - quantum + ifNumber >= 0) &&
	(offset == type_SNMP_PDUs_get__next__request)) {
      is -> last_q = quantum;
    }

    if (is -> mib_envt == ENVTNOTOK) 
    {
      if (ask_device(is, QUERY) == NOTOK) 
      {
	if (offset == type_SNMP_PDUs_get__next__request)
	    return NOTOK;
        return int_SNMP_error__status_noSuchName;
      }
    }

    ptr = &is -> datacache -> token_ring_all.Token_ring_mib;
    ptr2 = &is -> environ -> token_ring_all.Token_ring_mib;
    if ((ptr == NULL) || (ptr2 == NULL)) {
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_TOKENRING, TOKENRING_1,
	     "%s: device data and/or device info are NULL"), "o_tokenring");
      return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	/*
	 * The index values will only be displayed if the envt is ok.  This
	 * is so that there won't be if values for down devices 
	 */
	case DOT5IFINDEX:
	  return o_integer(oi, v, is -> ifn_index);
	  break;
	case DOT5COMMANDS:
	  if ((ptr2 -> Dot5Entry.commands == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.commands == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5Entry.commands);
          }
	  break;
	case DOT5RINGSTATUS:
	  if ((ptr2 -> Dot5Entry.ring_status == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.ring_status == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5Entry.ring_status);
          }
	  break;
	case DOT5RINGSTATE:
	  if ((ptr2 -> Dot5Entry.ring_state == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.ring_state == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5Entry.ring_state);
          }
	  break;
	case DOT5RINGOPENSTATUS:
	  if ((ptr2 -> Dot5Entry.ring_ostatus == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.ring_ostatus == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5Entry.ring_ostatus);
          }
	  break;
	case DOT5RINGSPEED:
	  if ((ptr2 -> Dot5Entry.ring_speed == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.ring_speed == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5Entry.ring_speed);
          }
	  break;
	case DOT5UPSTREAM:
	  if ((ptr2 -> Dot5Entry.upstream[0] == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.upstream[0] == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    if (strcmp(ptr -> Dot5Entry.upstream, "") == 0)
	    {
	      if (offset == type_SNMP_PDUs_get__next__request)
	        return NOTOK;
              return int_SNMP_error__status_noSuchName;
	    }
	    return o_string(oi, v, ptr -> Dot5Entry.upstream, MAXUPSTREAM);
	  }
	  break;
	case DOT5ACTMONPARTICIPATE:
	  if ((ptr2 -> Dot5Entry.participate == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.participate == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5Entry.participate);
          }
	  break;
	case DOT5FUNCTIONAL:
	  if ((ptr2 -> Dot5Entry.functional[0] == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5Entry.functional[0] == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_string(oi, v, ptr -> Dot5Entry.functional, MAXFUNCTIONAL);
          }
	  break;

	/*
	 * The index values will only be displayed if the envt is ok.  This
	 * is so that there won't be if values for down devices 
	 */
	case DOT5STATSIFINDEX:
	  return o_integer(oi, v, is -> ifn_index);
	  break;
	case DOT5STATSLINEERRORS:
	  if ((ptr2 -> Dot5StatsEntry.line_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.line_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.line_errs);
          }
	  break;
	case DOT5STATSBURSTERRORS:
	  if ((ptr2 -> Dot5StatsEntry.burst_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.burst_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.burst_errs);
          }
	  break;
	case DOT5STATSACERRORS:
	  if ((ptr2 -> Dot5StatsEntry.ac_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.ac_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.ac_errs);
          }
	  break;
	case DOT5STATSABORTTRANSERRORS:
	  if ((ptr2 -> Dot5StatsEntry.abort_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.abort_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.abort_errs);
          }
	  break;
	case DOT5STATSINTERNALERRORS:
	  if ((ptr2 -> Dot5StatsEntry.int_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.int_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.int_errs);
          }
	  break;
	case DOT5STATSLOSTFRAMEERRORS:
	  if ((ptr2 -> Dot5StatsEntry.lostframes == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.lostframes == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.lostframes);
          }
	  break;
	case DOT5STATSRECEIVECONGESTIONS:
	  if ((ptr2 -> Dot5StatsEntry.rx_congestion == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.rx_congestion == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.rx_congestion);
          }
	  break;
	case DOT5STATSFRAMECOPIEDERRORS:
	  if ((ptr2 -> Dot5StatsEntry.framecopies == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.framecopies == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.framecopies);
          }
	  break;
	case DOT5STATSTOKENERRORS:
	  if ((ptr2 -> Dot5StatsEntry.token_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.token_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.token_errs);
          }
	  break;
	case DOT5STATSSOFTERRORS:
	  if ((ptr2 -> Dot5StatsEntry.soft_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.soft_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.soft_errs);
          }
	  break;
	case DOT5STATSHARDERRORS:
	  if ((ptr2 -> Dot5StatsEntry.hard_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.hard_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.hard_errs);
          }
	  break;
	case DOT5STATSSIGNALLOSS:
	  if ((ptr2 -> Dot5StatsEntry.signal_loss == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.signal_loss == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.signal_loss);
          }
	  break;
	case DOT5STATSTRANSMITBEACONS:
	  if ((ptr2 -> Dot5StatsEntry.tx_beacons == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.tx_beacons == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.tx_beacons);
          }
	  break;
	case DOT5STATSRECOVERYS:
	  if ((ptr2 -> Dot5StatsEntry.recoverys == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.recoverys == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.recoverys);
          }
	  break;
	case DOT5STATSLOBEWIRES:
	  if ((ptr2 -> Dot5StatsEntry.lobewires == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.lobewires == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.lobewires);
          }
	  break;
	case DOT5STATSREMOVES:
	  if ((ptr2 -> Dot5StatsEntry.removes == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.removes == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.removes);
          }
	  break;
	case DOT5STATSSINGLES:
	  if ((ptr2 -> Dot5StatsEntry.singles == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.singles == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.singles);
          }
	  break;
	case DOT5STATSFREQERRORS:
	  if ((ptr2 -> Dot5StatsEntry.freq_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5StatsEntry.freq_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5StatsEntry.freq_errs);
          }
	  break;

	/*
	 * The index values will only be displayed if the envt is ok.  This
	 * is so that there won't be if values for down devices 
	 */
	case DOT5TIMERIFINDEX:
	  return o_integer(oi, v, is -> ifn_index);
	  break;
	case DOT5TIMERRETURNREPEAT:
	  if ((ptr2 -> Dot5TimerEntry.return_repeat == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.return_repeat == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.return_repeat);
          }
	  break;
	case DOT5TIMERHOLDING:
	  if ((ptr2 -> Dot5TimerEntry.holding == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.holding == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.holding);
          }
	  break;
	case DOT5TIMERQUEUEPDU:
	  if ((ptr2 -> Dot5TimerEntry.queue_pdu == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.queue_pdu == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.queue_pdu);
          }
	  break;
	case DOT5TIMERVALIDTRANSMIT:
	  if ((ptr2 -> Dot5TimerEntry.valid_tx == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.valid_tx == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.valid_tx);
          }
	  break;
	case DOT5TIMERNOTOKEN:
	  if ((ptr2 -> Dot5TimerEntry.no_token == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.no_token == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.no_token);
          }
	  break;
	case DOT5TIMERACTIVEMON:
	  if ((ptr2 -> Dot5TimerEntry.active_mon == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.active_mon == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.active_mon);
          }
	  break;
	case DOT5TIMERSTANDBYMON:
	  if ((ptr2 -> Dot5TimerEntry.standby_mon == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.standby_mon == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.standby_mon);
          }
	  break;
	case DOT5TIMERERRORREPORT:
	  if ((ptr2 -> Dot5TimerEntry.err_report == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.err_report == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.err_report);
          }
	  break;
	case DOT5TIMERBEACONTRANSMIT:
	  if ((ptr2 -> Dot5TimerEntry.beacon_tx == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.beacon_tx == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.beacon_tx);
          }
	  break;
	case DOT5TIMERBEACONRECEIVE:
	  if ((ptr2 -> Dot5TimerEntry.beacon_rx == MIB_READ_ONLY) ||
	      (ptr2 -> Dot5TimerEntry.beacon_rx == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
	          return NOTOK;
		return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, tokenringtimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot5TimerEntry.beacon_rx);
          }
	  break;
    }
    if (offset == type_SNMP_PDUs_get__next__request)
      return NOTOK;
    return int_SNMP_error__status_noSuchName;
}


static int s_tokenring (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    int when;
    token_ring_mib_t *ptr;
    token_ring_mib_t *ptr2;
    register struct interface *is;

    ifvar = (int) ot -> ot_info;
    /* check that this interface instance exists */
    switch (offset) {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    ifnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    for (is = ifs; is; is = is -> ifn_next)
		if (is -> ifn_index == ifnum)
		    break;
	    if (is == NULL)
		return int_SNMP_error__status_noSuchName;
	    if (!is -> ifn_ready)
	    {
		if (is -> setrequests)
		{
		    free(is -> setrequests);
		    free(is -> setpositions);
		    is -> setsize = 0;
		}
		return int_SNMP_error__status_noSuchName;
	    }
            if (is -> ndd_or_not != NDD_DRIVER)
	    {
		if (is -> setrequests)
		{
		    free(is -> setrequests);
		    free(is -> setpositions);
		    is -> setsize = 0;
		}
		return int_SNMP_error__status_noSuchName;
	    }
	    break;

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_TOKENRING, TOKENRING_2,
		    "s_tokenring: unexpected operation %d"), offset);
	    return int_SNMP_error__status_genErr;
    }

    if (is -> mib_envt == ENVTNOTOK) 
      if (ask_device(is, QUERY) == NOTOK) 
        return int_SNMP_error__status_genErr;

    switch (offset)
    {
	case type_SNMP_PDUs_set__request:
	    {
		ndd_mib_t mibtoadd;
		char *ndd_tempbuf;
		char *ndd_posbuf;
		int k, m;
		int number = v -> value -> un.number;

		switch (ifvar)
		{
		    case DOT5ACTMONPARTICIPATE:
			if ((number < 0) || (number > 1))
			{
			    if (is -> setrequests)
			    {
				free(is -> setrequests);
				free(is -> setpositions);
				is -> setsize = 0;
			    }
			    return int_SNMP_error__status_badValue;
			}

		    case DOT5COMMANDS:
		    case DOT5RINGSPEED:
			if ((number < 1) || (number > 4))
			{
			    if (is -> setrequests)
			    {
				free(is -> setrequests);
				free(is -> setpositions);
				is -> setsize = 0;
			    }
			    return int_SNMP_error__status_badValue;
			}

		    case DOT5TIMERRETURNREPEAT:
		    case DOT5TIMERHOLDING:
		    case DOT5TIMERQUEUEPDU:
		    case DOT5TIMERVALIDTRANSMIT:
		    case DOT5TIMERNOTOKEN:
		    case DOT5TIMERACTIVEMON:
		    case DOT5TIMERSTANDBYMON:
		    case DOT5TIMERERRORREPORT:
		    case DOT5TIMERBEACONTRANSMIT:
		    case DOT5TIMERBEACONRECEIVE:
			{
			    mibtoadd.mib = conversions[ifvar];
			    mibtoadd.status = 0;
			    mibtoadd.mib_len = sizeof(long);
			    bcopy(&k,(char *)(mibtoadd.mib_value),sizeof(long));
			}
			break;

		    case DOT5FUNCTIONAL:
			{
			    char *s;

			    mibtoadd.mib = TOKEN_FUNCTIONAL;
			    mibtoadd.status = 0;
			    mibtoadd.mib_len = MAXFUNCTIONAL;
			    if ((s = qb2str(v -> value -> un.string)) == NULLCP)
			    {
				if (is -> setrequests)
				{
				    free(is -> setrequests);
				    free(is -> setpositions);
				    is -> setsize = 0;
				}
				return int_SNMP_error__status_genErr;
			    }
			    bcopy(s,(char *)(mibtoadd.mib_value),MAXFUNCTIONAL);
			    free(s);
			}
			break;

		    default:
			if (is -> setrequests)
			{
			    free(is -> setrequests);
			    free(is -> setpositions);
			    is -> setsize = 0;
			}
			return int_SNMP_error__status_noSuchName;
		}

		/*
		 *  The mib set code works like this:
		 *  The is -> setsize value is used to keep track of the 
		 *  current set packet size.   m is set to the number of 
		 *  requests in the packet already.
		 *  From this information, k, the size of the new packet,
		 *  can be calculated.  K is used to allocate a new temp
		 *  packet, that has both the old packet and the new added
		 *  part in it.  m is used to allocate a new position holder
		 *  array.  This array is used to tell the user which PDU 
		 *  entry was bad.  After both have been allocated and filled,
		 *  the existing old ones are freed, and the variables set to
		 *  point to the new ones.
		 */
		if (is -> setsize == 0)
		{
		    is -> setsize = sizeof(u_int);
		    m = 0;
		}
		else m = ((ndd_mib_set_t *)(is->setrequests)) -> count;

		k = is -> setsize + sizeof(mib_t) + sizeof(u_int) +
		    sizeof(u_short) + mibtoadd.mib_len;
		if ((ndd_tempbuf = (char *)calloc(1, k)) == NULL)
		{
		    if (is -> setrequests)
		    {
			free(is -> setrequests);
			free(is -> setpositions);
			is -> setsize = 0;
		    }
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
			   "%s: out of memory"), "s_tokenring");
		    return int_SNMP_error__status_genErr;
		}

		if ((ndd_posbuf = (char *)calloc(1, m + 1)) == NULL)
		{
		    if (is -> setrequests)
		    {
			free(is -> setrequests);
			free(is -> setpositions);
			is -> setsize = 0;
		    }
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,	
			   "%s: out of memory"), "s_tokenring");
		    return int_SNMP_error__status_genErr;
		}

		if (is -> setrequests)
		    bcopy(is -> setrequests, ndd_tempbuf, is -> setsize);
		bcopy((char *)&mibtoadd, &ndd_tempbuf[is -> setsize],
		      k - is -> setsize);

		if (is -> setpositions)
		    bcopy(is -> setpositions, ndd_posbuf, m);
		ndd_posbuf[m] = SNMP_PDU_variable_position;
                if (is -> setrequests)
		    free(is -> setrequests);
		is -> setrequests = ndd_tempbuf;
		is -> setsize = k;

		if (is -> setpositions)
		    free(is -> setpositions);
		is -> setpositions = ndd_posbuf;
		((ndd_mib_set_t *)(is -> setrequests)) -> count = m + 1;
	    }
	    break;

	case type_SNMP_PDUs_commit:
	    {
		if (is -> setrequests)
		{
		    int k;

		    k = set_device(is);
		    free (is -> setrequests);
		    is -> setsize = 0;
		    return(k);
		}
	    }
	    break;

	case type_SNMP_PDUs_rollback:
	    if (is -> setrequests)
	    {
		free(is -> setrequests);
		free(is -> setpositions);
		is -> setsize = 0;
	    }
	    break;
    }

    return int_SNMP_error__status_noError;
}


/*  */
init_tokenring ()
{
    register OT	    ot;

    if (ot = text2obj ("dot5IfIndex")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5IFINDEX;
    }
    if (ot = text2obj ("dot5Commands")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5COMMANDS;
    }
    if (ot = text2obj ("dot5RingStatus")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5RINGSTATUS;
    }
    if (ot = text2obj ("dot5RingState")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5RINGSTATE;
    }
    if (ot = text2obj ("dot5RingOpenStatus")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5RINGOPENSTATUS;
    }
    if (ot = text2obj ("dot5RingSpeed")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5RINGSPEED;
    }
    if (ot = text2obj ("dot5UpStream")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5UPSTREAM;
    }
    if (ot = text2obj ("dot5ActMonParticipate")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5ACTMONPARTICIPATE;
    }
    if (ot = text2obj ("dot5Functional")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5FUNCTIONAL;
    }

    if (ot = text2obj ("dot5StatsIfIndex")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSIFINDEX;
    }
    if (ot = text2obj ("dot5StatsLineErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSLINEERRORS;
    }
    if (ot = text2obj ("dot5StatsBurstErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSBURSTERRORS;
    }
    if (ot = text2obj ("dot5StatsACErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSACERRORS;
    }
    if (ot = text2obj ("dot5StatsAbortTransErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSABORTTRANSERRORS;
    }
    if (ot = text2obj ("dot5StatsInternalErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSINTERNALERRORS;
    }
    if (ot = text2obj ("dot5StatsLostFrameErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSLOSTFRAMEERRORS;
    }
    if (ot = text2obj ("dot5StatsReceiveCongestions")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSRECEIVECONGESTIONS;
    }
    if (ot = text2obj ("dot5StatsFrameCopiedErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSFRAMECOPIEDERRORS;
    }
    if (ot = text2obj ("dot5StatsTokenErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSTOKENERRORS;
    }
    if (ot = text2obj ("dot5StatsSoftErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSSOFTERRORS;
    }
    if (ot = text2obj ("dot5StatsHardErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSHARDERRORS;
    }
    if (ot = text2obj ("dot5StatsSignalLoss")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSSIGNALLOSS;
    }
    if (ot = text2obj ("dot5StatsTransmitBeacons")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSTRANSMITBEACONS;
    }
    if (ot = text2obj ("dot5StatsRecoverys")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSRECOVERYS;
    }
    if (ot = text2obj ("dot5StatsLobeWires")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSLOBEWIRES;
    }
    if (ot = text2obj ("dot5StatsRemoves")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSREMOVES;
    }
    if (ot = text2obj ("dot5StatsSingles")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSSINGLES;
    }
    if (ot = text2obj ("dot5StatsFreqErrors")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5STATSFREQERRORS;
    }

    if (ot = text2obj ("dot5TimerIfIndex")) {
	ot -> ot_getfnx = o_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERIFINDEX;
    }
    if (ot = text2obj ("dot5TimerReturnRepeat")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERRETURNREPEAT;
    }
    if (ot = text2obj ("dot5TimerHolding")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERHOLDING;
    }
    if (ot = text2obj ("dot5TimerQueuePDU")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERQUEUEPDU;
    }
    if (ot = text2obj ("dot5TimerValidTransmit")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERVALIDTRANSMIT;
    }
    if (ot = text2obj ("dot5TimerNoToken")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERNOTOKEN;
    }
    if (ot = text2obj ("dot5TimerActiveMon")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERACTIVEMON;
    }
    if (ot = text2obj ("dot5TimerStandbyMon")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERSTANDBYMON;
    }
    if (ot = text2obj ("dot5TimerErrorReport")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERERRORREPORT;
    }
    if (ot = text2obj ("dot5TimerBeaconTransmit")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERBEACONTRANSMIT;
    }
    if (ot = text2obj ("dot5TimerBeaconReceive")) {
	ot -> ot_getfnx = o_tokenring;
	ot -> ot_setfnx = s_tokenring;
        ot -> ot_info = (caddr_t) DOT5TIMERBEACONRECEIVE;
    }
}

#ifdef DEBUG
void print_token_ring(vals)
token_ring_mib_t *vals;
{

	advise(SLOG_EXCEPTIONS, "Dot5Entry.commands = %d", vals -> Dot5Entry.commands);	/* dot5Commands */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.ring_status = %ld", vals -> Dot5Entry.ring_status);	/* dot5RingStatus */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.ring_state = %d", vals -> Dot5Entry.ring_state);	/* dot5RingState */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.ring_ostatus = %d", vals -> Dot5Entry.ring_ostatus);	/* dot5RingOpenStatus */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.ring_speed = %d", vals -> Dot5Entry.ring_speed);	/* dot5RingSpeed */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.unpstream = %s", vals -> Dot5Entry.upstream);	/* dot5UpStream */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.participate = %d", vals -> Dot5Entry.participate);	/* dot5ActMonParticipate */
	advise(SLOG_EXCEPTIONS, "Dot5Entry.functional = %s", vals -> Dot5Entry.functional);	/* dot5Functional */

	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.line_errs = %ld", vals -> Dot5StatsEntry.line_errs);	/* dot5StatsLineErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.burst_errs = %ld", vals -> Dot5StatsEntry.burst_errs);	/* dot5StatsBurstErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.ac_errs = %ld", vals -> Dot5StatsEntry.ac_errs);		/* dot5StatsACErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.abort_errs = %ld", vals -> Dot5StatsEntry.abort_errs);	/* dot5StatsAbortTransErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.int_errs = %ld", vals -> Dot5StatsEntry.int_errs);	/* dot5StatsInternalErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.lostframes = %ld", vals -> Dot5StatsEntry.lostframes);	/* dot5StatsLostFrameErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.congestion = %ld", vals -> Dot5StatsEntry.rx_congestion);	/* dot5StatsReceiveCongestions */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.framecopies = %ld", vals -> Dot5StatsEntry.framecopies);	/* dot5StatsFrameCopiedErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.token_errs = %ld", vals -> Dot5StatsEntry.token_errs);	/* dot5StatsTokenErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.soft_errs = %ld", vals -> Dot5StatsEntry.soft_errs);	/* dot5StatsSoftErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.hard_errs = %ld", vals -> Dot5StatsEntry.hard_errs);	/* dot5StatsHardErrors */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.signal_loss = %ld", vals -> Dot5StatsEntry.signal_loss);	/* dot5StatsSignalLoss */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.tx_beacons = %ld", vals -> Dot5StatsEntry.tx_beacons);	/* dot5StatsTransmitBeacons */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.recoverys = %ld", vals -> Dot5StatsEntry.recoverys);	/* dot5StatsRecoverys */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.lobewires = %ld", vals -> Dot5StatsEntry.lobewires);	/* dot5StatsLobeWires */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.removes = %ld", vals -> Dot5StatsEntry.removes);		/* dot5StatsRemoves */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.singles = %ld", vals -> Dot5StatsEntry.singles);		/* dot5StatsSingles */
	advise(SLOG_EXCEPTIONS, "Dot5StatsEntry.freq_errs = %ld", vals -> Dot5StatsEntry.freq_errs);	/* dot5StatsFreqErrors */

	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.return_repeat = %ld", vals -> Dot5TimerEntry.return_repeat);	/* dot5TimerReturnRepeat */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.holding = %ld", vals -> Dot5TimerEntry.holding);		/* dot5TimerHolding */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.queue_pdu = %ld", vals -> Dot5TimerEntry.queue_pdu);	/* dot5TimerQueuePDU */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.valid_tx = %ld", vals -> Dot5TimerEntry.valid_tx);	/* dot5TimerValidTransmit */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.no_token = %ld", vals -> Dot5TimerEntry.no_token);	/* dot5TimerNoToken */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.active_mon = %ld", vals -> Dot5TimerEntry.active_mon);	/* dot5TimerActiveMon */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.standby_mon = %ld", vals -> Dot5TimerEntry.standby_mon);	/* dot5TimerStandbyMon */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.err_report = %ld", vals -> Dot5TimerEntry.err_report);	/* dot5TimerErrorReport */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.beacon_tx = %ld", vals -> Dot5TimerEntry.beacon_tx);	/* dot5TimerBeaconTransmit */
	advise(SLOG_EXCEPTIONS, "Dot5TimerEntry.beacon_rx = %ld", vals -> Dot5TimerEntry.beacon_rx);	/* dot5TimerBeaconReceive */
}
#endif /* DEBUG */

