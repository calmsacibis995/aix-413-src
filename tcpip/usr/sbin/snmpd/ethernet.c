static char sccsid[] = "@(#)99	1.3  src/tcpip/usr/sbin/snmpd/ethernet.c, snmp, tcpip411, GOLD410 4/4/94 15:51:04";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/ethernet.c
 */

/* INCLUDES */
#include "snmpd.h"
#include "ddmibs.h"
#include "interfaces.h"

#ifdef DEBUG
void print_ethernet();
extern void print_list();
#endif /* DEBUG */

extern struct interface *ifs;

extern int ask_device();                /* Defined in devices.c */

extern int ethernettimeout;              /* Defined in config.c */

extern void update_timers();		 /* Defined in timers.c */

#define MAXHISTNUM	16               /* The largest histogram index */

#define DOT3STATSINDEX 1
#define DOT3STATSALIGNMENTERRORS 2
#define DOT3STATSFCSERRORS 3
#define DOT3STATSSINGLECOLLISIONFRAMES 4
#define DOT3STATSMULTIPLECOLLISIONFRAMES 5
#define DOT3STATSSQETESTERRORS 6
#define DOT3STATSDEFERREDTRANSMISSIONS 7
#define DOT3STATSLATECOLLISIONS 8
#define DOT3STATSEXCESSIVECOLLISIONS 9
#define DOT3STATSINTERNALMACTRANSMITERRORS 10
#define DOT3STATSCARRIERSENSEERRORS 11
#define DOT3STATSFRAMETOOLONGS 12
#define DOT3STATSINTERNALMACRECEIVEERRORS 13

#define DOT3COLLINDEX 14
#define DOT3COLLCOUNT 15
#define DOT3COLLFREQUENCIES 16

/*  */

static int o_ethernet (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    ethernet_mibs_t *ptr;
    ethernet_mibs_t *ptr2;
    register struct interface *is;
    int when = 1;

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
		    (is -> ifn_type != IFT_ETHER && 
		     is -> ifn_type != IFT_ISO88023) ||
		    is -> ndd_or_not == NOT_NDD_DRIVER)
		  return int_SNMP_error__status_noSuchName;
		break;

	    case type_SNMP_PDUs_get__next__request:
                if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	          OID     new;

	          /* find first ready interface */
	          for (is = ifs; is; is = is -> ifn_next)
		    if ((is -> ifn_ready) && ((is -> ifn_type == IFT_ETHER) ||
			(is -> ifn_type == IFT_ISO88023)) &&
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
		  OID     new;
	          register struct interface *iz;

	          if ((i >= oid -> oid_nelem) ||
		      (ifnum = oid -> oid_elements[i]) == 0) {
	            if ((is = ifs) == NULL)
	              return NOTOK;
		    if ((is -> ifn_ready) && ((is -> ifn_type == IFT_ETHER) ||
			(is -> ifn_type == IFT_ISO88023)) &&
			(is -> ndd_or_not == NDD_DRIVER))
	              goto stuff_ifnum;
		    ifnum = 1;
	          }
	          for (is = iz = ifs; is; is = is -> ifn_next)
		    if ((iz = is) -> ifn_index == ifnum)
		      break;
	          for (is = iz -> ifn_next; is; is = is -> ifn_next)
		    if ((is -> ifn_ready) && ((is -> ifn_type == IFT_ETHER) ||
			(is -> ifn_type == IFT_ISO88023)) &&
			(is -> ndd_or_not == NDD_DRIVER))
	              break;
	          if (!is)
		    return NOTOK;
stuff_ifnum: ;
	          ifnum = is -> ifn_index;

	          if ((new = oid_extend(ot -> ot_name, 1)) == NULLOID)
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

    if ((is -> flush_cache == 0) &&
	(offset == type_SNMP_PDUs_get__next__request) &&
	(is -> last_q - quantum + ifNumber >= 0)) {
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

    ptr = &is -> datacache -> ethernet_all.Ethernet_mib;
    ptr2 = &is -> environ -> ethernet_all.Ethernet_mib;
    if ((ptr == NULL) || (ptr2 == NULL)) {
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_ETHERNET, ETHERNET_1,
	     "o_ethernet: device data and/or device info are NULL"));
      return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	/*
	 *  The indexes will only be displayed if the device is ok.
	 *  This prevents indexes for devices that are down.
	 */
	case DOT3STATSINDEX:
	    return o_integer(oi, v, is -> ifn_index);
	    break;
	case DOT3STATSALIGNMENTERRORS:
	  if ((ptr2 -> Dot3StatsEntry.align_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.align_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.align_errs);
          }
	  break;
	case DOT3STATSFCSERRORS:
	  if ((ptr2 -> Dot3StatsEntry.fcs_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.fcs_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.fcs_errs);
          }
	  break;
	case DOT3STATSSINGLECOLLISIONFRAMES:
	  if ((ptr2 -> Dot3StatsEntry.s_coll_frames == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.s_coll_frames == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.s_coll_frames);
          }
	  break;
	case DOT3STATSMULTIPLECOLLISIONFRAMES:
	  if ((ptr2 -> Dot3StatsEntry.m_coll_frames == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.m_coll_frames == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.m_coll_frames);
          }
	  break;
	case DOT3STATSSQETESTERRORS:
	  if ((ptr2 -> Dot3StatsEntry.sqetest_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.sqetest_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.sqetest_errs);
          }
	  break;
	case DOT3STATSDEFERREDTRANSMISSIONS:
	  if ((ptr2 -> Dot3StatsEntry.defer_tx == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.defer_tx == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.defer_tx);
          }
	  break;
	case DOT3STATSLATECOLLISIONS:
	  if ((ptr2 -> Dot3StatsEntry.late_collisions == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.late_collisions == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.late_collisions);
          }
	  break;
	case DOT3STATSEXCESSIVECOLLISIONS:
	  if ((ptr2 -> Dot3StatsEntry.excess_collisions == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.excess_collisions == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.excess_collisions);
          }
	  break;
	case DOT3STATSINTERNALMACTRANSMITERRORS:
	  if ((ptr2 -> Dot3StatsEntry.mac_tx_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.mac_tx_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.mac_tx_errs);
          }
	  break;
	case DOT3STATSCARRIERSENSEERRORS:
	  if ((ptr2 -> Dot3StatsEntry.carriers_sense == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.carriers_sense == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.carriers_sense);
          }
	  break;
	case DOT3STATSFRAMETOOLONGS:
	  if ((ptr2 -> Dot3StatsEntry.long_frames == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.long_frames == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.long_frames);
          }
	  break;
	case DOT3STATSINTERNALMACRECEIVEERRORS:
	  if ((ptr2 -> Dot3StatsEntry.mac_rx_errs == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3StatsEntry.mac_rx_errs == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3StatsEntry.mac_rx_errs);
          }
	  break;

    }

    if (offset == type_SNMP_PDUs_get__next__request)
      return NOTOK;
    return int_SNMP_error__status_noSuchName;
}


static int o_ethertable (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    ethernet_mibs_t *ptr;
    ethernet_mibs_t *ptr2;
    int histnum = MAXHISTNUM + 1;
    register struct interface *is;
    int when = 1;

    ifvar = (int) ot -> ot_info;

    switch (offset) {
	    case type_SNMP_PDUs_get__request:
		if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
		    return int_SNMP_error__status_noSuchName;
		ifnum = oid -> oid_elements[oid -> oid_nelem - 2];
		for (is = ifs; is; is = is -> ifn_next)
		    if (is -> ifn_index == ifnum)
		        break;
		if (is == NULL || !is -> ifn_ready || 
		    (is -> ifn_type != IFT_ETHER && 
		     is -> ifn_type != IFT_ISO88023) ||
		    is -> ndd_or_not == NOT_NDD_DRIVER)
		    return int_SNMP_error__status_noSuchName;
		if ((histnum = oid -> oid_elements[oid -> oid_nelem - 1]) > 
		    MAXHISTNUM)
		    return int_SNMP_error__status_noSuchName;
		histnum--;
		break;

	    case type_SNMP_PDUs_get__next__request:
                if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	          OID     new;

	          /* find first ready interface */
	          for (is = ifs; is; is = is -> ifn_next)
		    if ((is -> ifn_ready) && ((is -> ifn_type == IFT_ETHER) ||
			(is -> ifn_type == IFT_ISO88023)) &&
			(is -> ndd_or_not == NDD_DRIVER))
		      break;
	          if (!is)
		    return NOTOK;
	          ifnum = is -> ifn_index;
		  histnum = 0;

	          if ((new = oid_extend (oid, 2)) == NULLOID)
		    return NOTOK;
                  new -> oid_elements[new -> oid_nelem - 2] = ifnum;
                  new -> oid_elements[new -> oid_nelem - 1] = histnum + 1;

	          if (v -> name)
	            free_SMI_ObjectName (v -> name);
	          v -> name = new;
	        }
	        else {
                  int     i = ot -> ot_name -> oid_nelem;
	          register struct interface *iz;
		  OID new;

		  if (i + 2 == oid -> oid_nelem) {
		      ifnum = oid -> oid_elements[i];
		      histnum = oid -> oid_elements[i+1];

		      if (histnum >= MAXHISTNUM) {
			  histnum = 0;
	                  if ((ifnum = oid -> oid_elements[i]) == 0) {
		              if ((is = ifs) == NULL)
	                          return NOTOK;
		              if ((is -> ifn_ready) && 
			          ((is -> ifn_type == IFT_ETHER) ||
			           (is -> ifn_type == IFT_ISO88023)) &&
				  (is -> ndd_or_not == NDD_DRIVER))
	                          goto stuff_ifnum2;
		              ifnum = 1;
                          }
	                  for (is = iz = ifs; is; is = is -> ifn_next)
		              if ((iz = is) -> ifn_index == ifnum)
		                  break;
	                  for (is = iz -> ifn_next; is; is = is -> ifn_next)
		              if ((is -> ifn_ready) && 
				  ((is -> ifn_type == IFT_ETHER) ||
			           (is -> ifn_type == IFT_ISO88023)) &&
				  (is -> ndd_or_not == NDD_DRIVER))
	                          break;
	                  if (!is)
		              return NOTOK;
stuff_ifnum2: ;
	                  ifnum = is -> ifn_index;
		      }
		      else {
	                for (is = iz = ifs; is; is = is -> ifn_next)
		          if ((iz = is) -> ifn_index == ifnum)
		            break;
	                if (!is)
		          return NOTOK;
		      }

		      if (histnum < 0) {
			  histnum = 0;
		      }

	              oid -> oid_elements[i] = ifnum;
	              oid -> oid_elements[i + 1] = histnum + 1;
	              oid -> oid_nelem = i + 2;
	              if ((new = oid_cpy(oid)) == NULLOID)
		          return NOTOK;

	              if (v -> name)
	                  free_SMI_ObjectName (v -> name);
	              v -> name = new;
		  }
		  else {
		      histnum = 0;

	              if ((i >= oid -> oid_nelem) || 
			  ((ifnum = oid -> oid_elements[i]) == 0)) {
		        if ((is = ifs) == NULL)
	                  return NOTOK;
		        if ((is -> ifn_ready) && 
			    ((is -> ifn_type == IFT_ETHER) ||
			     (is -> ifn_type == IFT_ISO88023)) &&
			    (is -> ndd_or_not == NDD_DRIVER))
	                  goto stuff_ifnum3;
		        ifnum = 1;
                      }
	              for (is = iz = ifs; is; is = is -> ifn_next)
		        if ((iz = is) -> ifn_index == ifnum)
		          break;
	              for (is = iz -> ifn_next; is; is = is -> ifn_next)
		        if ((is -> ifn_ready) && 
		            ((is -> ifn_type == IFT_ETHER) ||
			     (is -> ifn_type == IFT_ISO88023)) &&
			    (is -> ndd_or_not == NDD_DRIVER))
	                  break;
	              if (!is)
		        return NOTOK;
stuff_ifnum3: 
	              ifnum = is -> ifn_index;

	              if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		          return NOTOK;
                      new -> oid_elements[new -> oid_nelem - 1] = histnum + 1;
                      new -> oid_elements[new -> oid_nelem - 2] = ifnum;

	              if (v -> name)
	                  free_SMI_ObjectName (v -> name);
	              v -> name = new;
		  }
               }
               break;

        default:
		return int_SNMP_error__status_genErr;
    }

    if ((is -> flush_cache == 0) &&
	(offset == type_SNMP_PDUs_get__next__request) &&
	(is -> last_q - quantum + ifNumber >= 0)) {
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

    ptr = &is -> datacache -> ethernet_all.Ethernet_mib;
    ptr2 = &is -> environ -> ethernet_all.Ethernet_mib;
    if ((ptr == NULL) || (ptr2 == NULL)) {
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_ETHERNET, ETHERNET_1,
	     "o_ethernet: device data and/or device info are NULL"));
      return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	/*
	 *  The indexes will only be displayed if the device is ok.
	 *  This prevents indexes for devices that are down.
	 */
	case DOT3COLLINDEX:
	  if ((ptr2 -> Dot3CollEntry.freq[0] == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3CollEntry.freq[0] == MIB_READ_WRITE))
	      return o_integer(oi, v, is -> ifn_index);
	  break;
	case DOT3COLLCOUNT:
	  if ((ptr2 -> Dot3CollEntry.freq[0] == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3CollEntry.freq[0] == MIB_READ_WRITE))
	      return o_integer(oi, v, histnum + 1);
	  break;
	case DOT3COLLFREQUENCIES:
	  if ((ptr2 -> Dot3CollEntry.freq[0] == MIB_READ_ONLY) ||
	      (ptr2 -> Dot3CollEntry.freq[0] == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, ethernettimeout, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> Dot3CollEntry.freq[histnum]);
          }
	  break;
    }

    if (offset == type_SNMP_PDUs_get__next__request)
      return NOTOK;
    return int_SNMP_error__status_noSuchName;
}


/*  */
init_ethernet ()
{
    register OT	    ot;

    if (ot = text2obj ("dot3StatsIndex")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSINDEX;
    }
    if (ot = text2obj ("dot3StatsAlignmentErrors")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSALIGNMENTERRORS;
    }
    if (ot = text2obj ("dot3StatsFCSErrors")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSFCSERRORS;
    }
    if (ot = text2obj ("dot3StatsSingleCollisionFrames")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSSINGLECOLLISIONFRAMES;
    }
    if (ot = text2obj ("dot3StatsMultipleCollisionFrames")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSMULTIPLECOLLISIONFRAMES;
    }
    if (ot = text2obj ("dot3StatsSQETestErrors")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSSQETESTERRORS;
    }
    if (ot = text2obj ("dot3StatsDeferredTransmissions")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSDEFERREDTRANSMISSIONS;
    }
    if (ot = text2obj ("dot3StatsLateCollisions")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSLATECOLLISIONS;
    }
    if (ot = text2obj ("dot3StatsExcessiveCollisions")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSEXCESSIVECOLLISIONS;
    }
    if (ot = text2obj ("dot3StatsInternalMacTransmitErrors")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSINTERNALMACTRANSMITERRORS;
    }
    if (ot = text2obj ("dot3StatsCarrierSenseErrors")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSCARRIERSENSEERRORS;
    }
    if (ot = text2obj ("dot3StatsFrameTooLongs")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSFRAMETOOLONGS;
    }
    if (ot = text2obj ("dot3StatsInternalMacReceiveErrors")) {
	ot -> ot_getfnx = o_ethernet;
	ot -> ot_info = (caddr_t) DOT3STATSINTERNALMACRECEIVEERRORS;
    }

    if (ot = text2obj ("dot3CollIndex")) {
	ot -> ot_getfnx = o_ethertable;
	ot -> ot_info = (caddr_t) DOT3COLLINDEX;
    }
    if (ot = text2obj ("dot3CollCount")) {
	ot -> ot_getfnx = o_ethertable;
	ot -> ot_info = (caddr_t) DOT3COLLCOUNT;
    }
    if (ot = text2obj ("dot3CollFrequencies")) {
	ot -> ot_getfnx = o_ethertable;
	ot -> ot_info = (caddr_t) DOT3COLLFREQUENCIES;
    }
}


#ifdef DEBUG
void print_ethernet(vals)
ethernet_mibs_t *vals;
{

advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.align_errs = %ld", vals -> Dot3StatsEntry.align_errs);	/* dot3StatsAlignmentErrors */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.fcs_errs = %ld", vals -> Dot3StatsEntry.fcs_errs);	/* dot3StatsFCSErrors */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.s_coll_frames = %ld", vals -> Dot3StatsEntry.s_coll_frames);	/* dot3StatsSingleCollisionFrames */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.m_coll_frames = %ld", vals -> Dot3StatsEntry.m_coll_frames);	/* dot3StatsMultipleCollisionFrames */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.sqetest_errs = %ld", vals -> Dot3StatsEntry.sqetest_errs);	/* dot3StatsSQETestErrors */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.defer_tx = %ld", vals -> Dot3StatsEntry.defer_tx);	/* dot3StatsDeferredTransmissions */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.late_collisions = %ld", vals -> Dot3StatsEntry.late_collisions);	/* dot3StatsLateCollisions */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.excess_collisions = %ld", vals -> Dot3StatsEntry.excess_collisions);/* dot3StatsExcessiveCollisions */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.mac_tx_errs = %ld", vals -> Dot3StatsEntry.mac_tx_errs);	/* dot3StatsInternalMacTransmitErrors */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.carriers_sense = %ld", vals -> Dot3StatsEntry.carriers_sense);	/* dot3StatsCarrierSenseErrors */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.long_frames = %ld", vals -> Dot3StatsEntry.long_frames);	/* dot3StatsFrameTooLongs */
advise(SLOG_EXCEPTIONS, "Dot3StatsEntry.mac_rx_errs = %ld", vals -> Dot3StatsEntry.mac_rx_errs);	/* dot3StatsInternalMacReceiveErrors */


advise(SLOG_EXCEPTIONS, "Dot3CollEntry.count[0] = %ld", vals -> Dot3CollEntry.count[0]);	/* dot3CollCount */
advise(SLOG_EXCEPTIONS, "Dot3CollEntry.freq[0] = %ld",  vals -> Dot3CollEntry.freq[0]);	/* dot3CollFrequencies */

}
#endif /* DEBUG */

