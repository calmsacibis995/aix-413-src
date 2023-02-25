static char sccsid[] = "@(#)03	1.5  src/tcpip/usr/sbin/snmpd/generic.c, snmp, tcpip411, GOLD410 3/22/94 15:05:38";
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
 * FILE:	src/tcpip/usr/sbin/snmpd/generic.c
 */

/* INCLUDES */
#include <sys/ndd_var.h>
#include "snmpd.h"
#include "ddmibs.h"
#include "interfaces.h"

#ifdef DEBUG 
void print_generic();
void print_list();
#endif /* DEBUG */

extern struct interface *ifs;
extern char *commname;       /* The current community name */
extern int request_id;       /* The current request_id     */
extern int SNMP_PDU_variable_position;  /* The current position in the vars */

extern int ask_device();     /* Defined in devices.c */
extern int set_device();
extern int comp_addr();

extern int ethernettimeout;  /* Defined in config.c */
extern int tokenringtimeout;
extern int fdditimeout;
extern int stimeout;

extern void update_timers(); /* Defined in timers.c */

/* These should probably be arrays of numbers */
static char testcommname[1024] = "";
static int testrequestid = 0;

#define MAXREVWARE	16

#define IFEXTNSIFINDEX 1
#define IFEXTNSCHIPSET 2
#define IFEXTNSREVWARE 3
#define IFEXTNSMULTICASTSTRANSMITTEDOKS 4
#define IFEXTNSBROADCASTSTRANSMITTEDOKS 5
#define IFEXTNSMULTICASTSRECEIVEDOKS 6
#define IFEXTNSBROADCASTSRECEIVEDOKS 7
#define IFEXTNSPROMISCUOUS 8

#define IFEXTNSTESTIFINDEX 9
#define IFEXTNSTESTCOMMUNITY 10
#define IFEXTNSTESTREQUESTID 11
#define IFEXTNSTESTTYPE 12
#define IFEXTNSTESTRESULT 13
#define IFEXTNSTESTCODE 14

#define IFEXTNSRCVADDRIFINDEX 15
#define IFEXTNSRCVADDRESS 16
#define IFEXTNSRCVADDRSTATUS 17

/*  */

static int o_genera (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT     ot = oi -> oi_type;
    register OID    oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    generic_mib_t *ptr;
    generic_mib_t *ptr2;
    register struct interface *is;
    int enter = 0;
    struct fdinfo *fi;
    int timeval = 0;
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
		is -> ndd_or_not == NOT_NDD_DRIVER)
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      /* find first ready interface */
	      for (is = ifs; is; is = is -> ifn_next)
		if ((is -> ifn_ready) &&
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
	      OID     new;
              int     i = ot -> ot_name -> oid_nelem;
	      register struct interface *iz;

	      if ((i >= oid -> oid_nelem) ||
		  (ifnum = oid -> oid_elements[i]) == 0) {
	        if ((is = ifs) == NULL)
	          return NOTOK;
		if ((is -> ifn_ready) &&
		    (is -> ndd_or_not == NDD_DRIVER))
	          goto stuff_ifnum;
		ifnum = 1;
	      }
	      for (is = iz = ifs; is; is = is -> ifn_next)
		if ((iz = is) -> ifn_index == ifnum)
		  break;
	      for (is = iz -> ifn_next; is; is = is -> ifn_next)
	        if ((is -> ifn_ready) &&
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
	(offset == type_SNMP_PDUs_get__next__request) &&
	(is -> last_q - quantum + ifNumber >= 0)) 
    {
      is -> last_q = quantum;
    }
    else if (is -> last_q != quantum) 
    {
      timeval = (is -> ifn_type == IFT_FDDI) ? fdditimeout :
			((is -> ifn_type == IFT_ISO88025) ? tokenringtimeout :
			 (((is -> ifn_type == IFT_ISO88023) ||
			   (is -> ifn_type == IFT_ETHER)) ? ethernettimeout :
			   stimeout));
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

    ptr = &is -> datacache -> ethernet_all.Generic_mib;
    ptr2 = &is -> environ -> ethernet_all.Generic_mib;
    if ((ptr == NULL) || (ptr2 == NULL)) {
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC, GENERIC_1,
	     "%s: device data and/or device info are NULL"), "o_genera");
      return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	/*
	 *  The index variables will be returned if the environment is 
	 *  possible for a get to succeed.  So, if a device has responded to
	 *  queries, but is in limbo and will not respond to gets, the 
	 *  index variable will still be returned when requested.
	 */
	case IFEXTNSIFINDEX:
	  return o_integer(oi, v, is -> ifn_index);
	  break;
	case IFEXTNSCHIPSET:
	  if ((ptr2 -> ifExtnsEntry.chipset[0] == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.chipset[0] == MIB_READ_WRITE)) 
	  {   
	    int status = int_SNMP_error__status_noSuchName;
	    OID new;

            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    
	    if ((new = (OID)calloc(1, sizeof(OID))) == NULL)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
			"%s: Out of Memory"), "o_genera");
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	    }
	    new -> oid_nelem = ptr -> ifExtnsEntry.chipset[0];
	    if ((new -> oid_elements = (uint *)calloc(new -> oid_nelem, 
					sizeof(uint))) == NULL)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
			"%s: Out of Memory"), "o_genera");
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	    }
	    bcopy(&(ptr -> ifExtnsEntry.chipset[1]), new -> oid_elements,
		  new -> oid_nelem * sizeof(uint));

	    status = o_specific(oi, v, (caddr_t *)new);
	    if (new)
		oid_free(new);
	    return status;
          }
	  break;
	case IFEXTNSREVWARE:
	  if ((ptr2 -> ifExtnsEntry.revware[0] == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.revware[0] == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_string(oi, v, ptr -> ifExtnsEntry.revware, MAXREVWARE);
          }
	  break;
	case IFEXTNSMULTICASTSTRANSMITTEDOKS:
	  if ((ptr2 -> ifExtnsEntry.mcast_tx_ok == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.mcast_tx_ok == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsEntry.mcast_tx_ok);
          }
	  break;
	case IFEXTNSBROADCASTSTRANSMITTEDOKS:
	  if ((ptr2 -> ifExtnsEntry.bcast_tx_ok == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.bcast_tx_ok == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsEntry.bcast_tx_ok);
          }
	  break;
	case IFEXTNSMULTICASTSRECEIVEDOKS:
	  if ((ptr2 -> ifExtnsEntry.mcast_rx_ok == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.mcast_rx_ok == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsEntry.mcast_rx_ok);
          }
	  break;
	case IFEXTNSBROADCASTSRECEIVEDOKS:
	  if ((ptr2 -> ifExtnsEntry.bcast_rx_ok == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.bcast_rx_ok == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsEntry.bcast_rx_ok);
          }
	  break;
	case IFEXTNSPROMISCUOUS:
	  if ((ptr2 -> ifExtnsEntry.promiscuous == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsEntry.promiscuous == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsEntry.promiscuous);
          }
	  break;

	/*
	 *  The index variables will be returned if the environment is 
	 *  possible for a get to succeed.  So, if a device has responded to
	 *  queries, but is in limbo and will not respond to gets, the 
	 *  index variable will still be returned when requested.
	 */
	case IFEXTNSTESTIFINDEX:
	  return o_integer(oi, v, is -> ifn_index);
	  break;

	case IFEXTNSTESTCOMMUNITY:
	  if (is -> mib_envt == ENVTOK)
	    return o_string(oi, v, testcommname, strlen(testcommname));
	  break;
	case IFEXTNSTESTREQUESTID:
	  if (is -> mib_envt == ENVTOK)
	    return o_integer(oi, v, testrequestid);
	  break;
	case IFEXTNSTESTTYPE:
	  if ((ptr2 -> ifExtnsTestEntry.type == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsTestEntry.type == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    testrequestid = request_id;
	    strncpy(testcommname, commname, strlen(commname));
	    return o_integer(oi, v, ptr -> ifExtnsTestEntry.type);
          }
	  break;
	case IFEXTNSTESTRESULT:
	  if ((ptr2 -> ifExtnsTestEntry.result == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsTestEntry.result == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsTestEntry.result);
          }
	  break;
	case IFEXTNSTESTCODE:
	  if ((ptr2 -> ifExtnsTestEntry.code == MIB_READ_ONLY) ||
	      (ptr2 -> ifExtnsTestEntry.code == MIB_READ_WRITE)) {
            if ((is -> mib_envt == ENVTOK) && (is -> last_q != quantum)) {
              if (ask_device(is, GET) == NOTOK)
	      {
		if (offset == type_SNMP_PDUs_get__next__request)
		  return NOTOK;
	        return int_SNMP_error__status_noSuchName;
	      }

	      update_timers(is, timeval, 0);
              is -> last_q = quantum;
            }
	    return o_integer(oi, v, ptr -> ifExtnsTestEntry.code);
          }
	  break;

    }

    if (offset == type_SNMP_PDUs_get__next__request)
      return NOTOK;
    return int_SNMP_error__status_noSuchName;
}


#define MAXADDRESS 20
static int o_rcvtable (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT     ot = oi -> oi_type;
    register OID    oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    generic_mib_t *ptr;
    register struct interface *is;
    struct fdinfo *fi;
    int i;
    ndd_mib_addr_elem_t *address;
    int timeval = 0;

    if ((address = (ndd_mib_addr_elem_t *)calloc (1, 
		    sizeof(ndd_mib_addr_elem_t) + MAXADDRESS)) == NULL) 
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
		"%s: Out of Memory"), "o_rcvtable");
	return int_SNMP_error__status_genErr;
    }
    address -> status = 0;
    address -> addresslen = 1;
    bzero(address -> address, MAXADDRESS);
    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem + 1)
	    {
    	      if (address)
	        free(address);
	      return int_SNMP_error__status_noSuchName;
	    }
	    ifnum = oid -> oid_elements[ot -> ot_name -> oid_nelem];
	    for (is = ifs; is; is = is -> ifn_next)
	      if (is -> ifn_index == ifnum)
		break;
	    for (i = 0; (i < oid -> oid_nelem - ot->ot_name->oid_nelem - 1) &&
			(i < MAXADDRESS); i++)
	      address->address[i]=oid->oid_elements[ot->ot_name->oid_nelem+i+1];
	    address -> addresslen = i;

	    if (is == NULL || !is -> ifn_ready || 
		is -> ndd_or_not == NOT_NDD_DRIVER)
	    {
    	      if (address)
	        free(address);
	      return int_SNMP_error__status_noSuchName;
	    }
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      /* find first ready interface */
	      for (is = ifs; is; is = is -> ifn_next)
		if ((is -> ifn_ready) &&
		    (is -> ndd_or_not == NDD_DRIVER))
		  break;
	      if (!is) 
	      {
    	        if (address)
	          free(address);
		return NOTOK;
	      }
	      ifnum = is -> ifn_index;
	    }
	    else {
              int     i = ot -> ot_name -> oid_nelem;
	      register struct interface *iz;

	      if (i >= oid -> oid_nelem)
		ifnum = 1;
	      else ifnum = oid -> oid_elements[i];

	      for (is = ifs; is; is = is -> ifn_next)
	        if ((is -> ifn_index == ifnum) && 
		    (is -> ifn_ready) &&
		    (is -> ndd_or_not == NDD_DRIVER))
		  break;
	      if (!is)
	      {
    	        if (address)
	          free(address);
		return NOTOK;
	      }
	      for (i=0; (i < oid -> oid_nelem - ot -> ot_name -> oid_nelem - 1)
			&& (i < MAXADDRESS); i++)
	        address->address[i] = 
				 oid->oid_elements[ot->ot_name->oid_nelem+i+1];
	      address -> addresslen = (i == 0 ? 1 : i);
	    }
            break;

	default:
	    return int_SNMP_error__status_genErr;
    }

yetagain: ;

    if ((is -> datarcvtable != NULL) &&
	(!is -> flush_rcv) &&
	(offset == type_SNMP_PDUs_get__next__request) &&
	(is -> last_q - quantum + ifNumber * *(int *)is -> datarcvtable >= 0)) 
    {
        is -> last_q = quantum;
    }
    else if (is -> last_q != quantum) 
    {
        timeval = (is -> ifn_type == IFT_FDDI) ? fdditimeout :
			((is -> ifn_type == IFT_ISO88025) ? tokenringtimeout :
			 (((is -> ifn_type == IFT_ISO88023) ||
			   (is -> ifn_type == IFT_ETHER)) ? ethernettimeout :
			   stimeout));
    }

    if (is -> mib_envt == ENVTNOTOK) 
    {
      if (ask_device(is, QUERY) == NOTOK)
      {
        if (address)
            free(address);
        if (offset == type_SNMP_PDUs_get__next__request)
	    return NOTOK;
        return int_SNMP_error__status_noSuchName;
      }
    }

    ptr = &is -> environ -> ethernet_all.Generic_mib;
    if (ptr == NULL) 
    {
        advise(SLOG_EXCEPTIONS,  MSGSTR(MS_GENERIC, GENERIC_1,
	       "%s: device data and/or device info are NULL"), "o_rcvtable");
        if (address)
            free(address);
        return int_SNMP_error__status_genErr;
    }

    if ((ptr -> RcvAddrTable != MIB_READ_ONLY) &&
        (ptr -> RcvAddrTable != MIB_READ_WRITE)) 
    {
        if (address)
          free(address);
        if (offset == type_SNMP_PDUs_get__next__request)
             return NOTOK;
        return int_SNMP_error__status_noSuchName;
    }
    
    if ((is -> last_q != quantum) || (is -> datarcvtable == NULL)) 
    {
      if (get_rcvtable(is) == NOTOK) 
      {
        advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC, GENERIC_1,
 	       "%s: device data and/or device info are NULL"), "o_rcvtable");
        if (address)
          free(address);
        if (offset == type_SNMP_PDUs_get__next__request)
             return NOTOK;
        return int_SNMP_error__status_noSuchName;
      }

      update_timers(is, timeval, 1);
    }

    if (offset == type_SNMP_PDUs_get__next__request) {
      int i;
      OID new;
      ndd_mib_addr_t *rcvlist = (ndd_mib_addr_t *)is -> datarcvtable;
      char *q = NULL;
      ndd_mib_addr_elem_t *p = NULL;

      for (i = 0; i < rcvlist -> count; i++)
      {
	q = &is -> datarcvtable[sizeof(u_int) + i * (sizeof(ndd_mib_addr_elem_t)
				+ 4)];
        p = (ndd_mib_addr_elem_t*)q;

	if (comp_addr(p, address) > 0)
	  break;
      }

      if (i == rcvlist -> count) {
	OID     new;
        int     i = ot -> ot_name -> oid_nelem;
	register struct interface *iz;

	if ((ifnum = oid -> oid_elements[i]) == 0) {
	  if ((is = ifs) == NULL) {
            if (address)
              free(address);
	    return NOTOK;
	  }	
	  if (is -> ifn_ready)
	    goto stuff_ifnum2;
	  ifnum = 1;
	}
	for (is = iz = ifs; is; is = is -> ifn_next)
	  if ((iz = is) -> ifn_index == ifnum)
	    break;
	if (!is) {
          if (address)
            free(address);
	  return NOTOK;
	}
	for (is = iz -> ifn_next; is; is = is -> ifn_next)
	  if ((is -> ifn_ready) &&
	      (is -> ndd_or_not == NDD_DRIVER))
	    break;
	if (!is) {
          if (address)
            free(address);
	  return NOTOK;
	}
stuff_ifnum2: ;
	ifnum = is -> ifn_index;
	oid -> oid_elements[i] = ifnum;

	bzero(address -> address, MAXADDRESS);
	address -> addresslen = 1;
	goto yetagain;
      }

      bcopy((char *)(&p -> address), address -> address, p -> addresslen);
      address -> status = p -> status;
      address -> addresslen = p -> addresslen;

      i = p -> addresslen - oid -> oid_nelem + ot -> ot_name -> oid_nelem + 1;
      if ((i > 0) && ((new = oid_extend(oid, i)) == NULLOID)) 
      {
          if (address)
            free(address);
	  return NOTOK;
      }
      if ((i < 1) && ((new = oid_cpy(oid)) == NULLOID))
      {
          if (address)
            free(address);
	  return NOTOK;
      }

      new -> oid_elements[ot -> ot_name -> oid_nelem] = ifnum;
      for (i = 0; i < p -> addresslen; i++)
	new -> oid_elements[ot->ot_name->oid_nelem+i+1] = address->address[i];

      if (v -> name)
        free_SMI_ObjectName (v -> name);
      v -> name = new;
    }
    else {
      int j; 
      char *q = NULL;
      ndd_mib_addr_elem_t *p = NULL;
      ndd_mib_addr_t *rcvlist = (ndd_mib_addr_t *)is -> datarcvtable;

      for (i = 0; i < rcvlist -> count; i++)
      {
	q = &is -> datarcvtable[sizeof(u_int) + i * (sizeof(ndd_mib_addr_elem_t)
				+ 4)];
        p = (ndd_mib_addr_elem_t*)q;

	if ((j=comp_addr(p, address)) >= 0)
	  break;
      }

      if (j != 0)
      {
        if (address)
          free(address);
        return int_SNMP_error__status_noSuchName;
      }

      address -> status = p -> status;
    }

    switch (ifvar) {
	case IFEXTNSRCVADDRIFINDEX:
	      if (address)
		  free(address);
              is -> last_q = quantum;
	      return o_integer(oi, v, is -> ifn_index);
	  break;
	case IFEXTNSRCVADDRESS:
	  {
	      int stat;

              is -> last_q = quantum;
	      stat = o_string(oi, v, address -> address, address -> addresslen);
	      if (address)
		  free(address);
	      return stat;
	  }
	  break;
	case IFEXTNSRCVADDRSTATUS:
	  { 
	      int stat;

              is -> last_q = quantum;
	      stat =  o_integer(oi, v, address -> status);
	      if (address)
		  free(address);
	      return stat;
	  }
	  break;
    }
	
    if (address)
      free(address);
    if (offset == type_SNMP_PDUs_get__next__request)
      return NOTOK;
    return int_SNMP_error__status_noSuchName;
}


static int s_genera (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    int                 ifnum;
    int                 ifvar;
    register OID        oid = oi -> oi_name;
    register OT         ot = oi -> oi_type;
    register OS         os = ot -> ot_syntax;
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
#ifdef _POWER
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
#endif /* _POWER */
	    break;

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC, GENERIC_1,
		    "s_genera: unexpected operation %d"), offset);
	    return int_SNMP_error__status_genErr;
    }

    if (is -> mib_envt == ENVTNOTOK) 
        if (ask_device(is, QUERY) == NOTOK)
	    return int_SNMP_error__status_noSuchName;

    switch (offset)
    {
	case type_SNMP_PDUs_set__request:
	    {
		ndd_mib_t mibtoadd;
		char *ndd_tempbuf;
		char *ndd_posbuf;
		int k, m;

		switch (ifvar)
		{
		    case IFEXTNSTESTTYPE:
			{
			    int k;
    			    generic_mib_t *ptr2;

			    ptr2 = &is -> environ -> ethernet_all.Generic_mib;
			    if (ptr2 -> ifExtnsTestEntry.type != 
				MIB_READ_WRITE)
			    {
				if (is -> setrequests)
				{
				    free(is -> setrequests);
				    free(is -> setpositions);
				    is -> setsize = 0;
				}
				return int_SNMP_error__status_noSuchName;
			    }

			    k = -1;
			    if (oid_cmp(v -> value -> un.object, 
				       text2oid("testFullDuplexLoopBack") == 0))
				k = GEN_MIB_TESTFULLDUPLEXLOOPBACK;

			    if (k != GEN_MIB_TESTFULLDUPLEXLOOPBACK)
			    {
				if (is -> setrequests)
				{
				    free(is -> setrequests);
				    free(is -> setpositions);
				    is -> setsize = 0;
				}
				return int_SNMP_error__status_badValue;
			    }
			    mibtoadd.mib = GENERIC_TYPE;
			    mibtoadd.status = 0;
			    mibtoadd.mib_len = sizeof(long);
			    bcopy(&k,(char *)(mibtoadd.mib_value),sizeof(long));
			}
			break;

		    case IFEXTNSPROMISCUOUS:
			{
			    int k = v -> value -> un.number;
			    int results;
			    long promparam;
			    int countproms = 0;

			    if ((k != PROMTRUE) || (k != PROMFALSE))
			    {
				if (is -> setrequests)
				{
				    free(is -> setrequests);
				    free(is -> setpositions);
				    is -> setsize = 0;
				}
				return int_SNMP_error__status_badValue;
			    }

			    if (k == PROMTRUE)
				promparam = NDD_PROMISCUOUS_ON;
			    else promparam = NDD_PROMISCUOUS_OFF;

			    /* 
			     *  This is the promiscuous setting ioctl.
			     *  It is in the set portion of the set code,
			     *  as opposed to the commit portion of the set
			     *  code, because snmpd does not know if the 
			     *  device driver supports promiscuous mode or
			     *  not.  So, the design is that prom will be
			     *  set on or off here, and rolled back if required
			     *  in the rollback stage.  Nothing will be done
			     *  in the commit stage. 
			     * 
			     *  Also, the device drivers currently do not 
			     *  allow the snmp set protocol to be used with
			     *  this variable.  The belief is that this is an
			     *  important variable to set.  So, snmpd will
			     *  use the normal ioctl method.  The belief is
			     *  also that snmpd should be the super super user
			     *  and have the ability to say on and off.  So,
			     *  to turn prom off, snmpd will need to loop until
			     *  the prom mode goes off.
			     */
			    k = 1;
			    while ((is -> fi_fd != NOTOK) && k)
			    {
				if (open_device(is) == NOTOK)
				    goto correctproms;

			    	results = ioctl(is -> fi_fd, promparam, 0);
				close_device(is);

			        if (results == NOTOK)
			        {
				    if (errno == EOPNOTSUPP)
				      return int_SNMP_error__status_noSuchName;

				    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC,
				       GENERIC_3,
				       "s_genera: Promiscuous ioctl failed"));

correctproms: ;			    /*
				     * We have failed somewhere.  Try to 
				     * correct the device drivers ref count 
				     */
			            if (promparam == NDD_PROMISCUOUS_OFF)
				        promparam = NDD_PROMISCUOUS_ON;
				    else promparam = NDD_PROMISCUOUS_OFF;
				    if (open_device(is) == NOTOK)
				        return int_SNMP_error__status_genErr;
				    for (k = 0; k < countproms; k++)
				    {
			    	        results=ioctl(is->fi_fd, promparam, 0);
					if (results == NOTOK)
					    break;
				    }
				    close_device(is);

				    return int_SNMP_error__status_genErr;
			        }
				countproms++;

				if (ask_device(is, GET) == NOTOK)
				{
				    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC,
				       GENERIC_6,
				       "s_genera: Promiscuous get failed"));
				    goto correctproms;	
				}
	if (is->datacache->ethernet_all.Generic_mib.ifExtnsEntry.promiscuous
				    == v -> value -> un.number)
				    k = 0;
			    }
			    return int_SNMP_error__status_noError;
			}

		    default:
			if (is -> setrequests)
			{
			    free(is -> setrequests);
			    free(is -> setpositions);
			    is -> setsize = 0;
			}
			return int_SNMP_error__status_noSuchName;
		}

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
				"%s: out of memory"), "s_genera");
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
				"%s: out of memory"), "s_genera");
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
	    if (ifvar != IFEXTNSPROMISCUOUS)
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
	    if (ifvar == IFEXTNSPROMISCUOUS)
	    {
		int k = v -> value -> un.number;
		int results;
		long promparam;

		if (k == PROMTRUE)
		    promparam = NDD_PROMISCUOUS_OFF;
		else promparam = NDD_PROMISCUOUS_ON;

                if (open_device(is) != NOTOK)
		{

		    if (is -> fi_fd != NOTOK)
			results = ioctl(is -> fi_fd, promparam, 0);

		    if (results == NOTOK)
		        advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC, GENERIC_4,
			   "s_genera: Could not rollback promiscuous set"));
		    close_device(is);
		}
	    }
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


#define MINRCVTABLEVALUE	1
#define MAXRCVTABLEVALUE	4
#define NONVOLATILE		4
#define DELETE_ADDRESS		2
#define DEFAULTADDRESSSIZE	6

static int s_rcvtable (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT     ot = oi -> oi_type;
    register OID    oid = oi -> oi_name;
    int ifvar;
    int ifnum;
    generic_mib_t *ptr;
    register struct interface *is;
    struct fdinfo *fi;
    int i;
    ndd_mib_addr_elem_t *address;
    int number = v -> value -> un.number;

    if ((address = (ndd_mib_addr_elem_t *)calloc (1, 
		    sizeof(ndd_mib_addr_elem_t) + MAXADDRESS)) == NULL) 
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
	       "%s: Out of Memory"), "s_rcvtable");
	return int_SNMP_error__status_genErr;
    }
    address -> status = 0;
    address -> addresslen = 0;
    bzero(address -> address, MAXADDRESS);
    ifvar = (int) ot -> ot_info;
    switch (offset) 
    {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem + 1)
	    {
    	      if (address)
	        free(address);
	      return int_SNMP_error__status_noSuchName;
	    }
	    ifnum = oid -> oid_elements[ot -> ot_name -> oid_nelem];
	    for (is = ifs; is; is = is -> ifn_next)
	      if (is -> ifn_index == ifnum)
		break;
	    for (i = 0; (i < oid -> oid_nelem - ot -> ot_name -> oid_nelem) &&
			(i < MAXADDRESS); i++)
	      address->address[i]=oid->oid_elements[ot->ot_name->oid_nelem+i+1];
	    address -> addresslen = i;

	    if (is == NULL || !is -> ifn_ready || 
		is -> ndd_or_not == NOT_NDD_DRIVER)
	    {
    	      if (address)
	        free(address);
	      return int_SNMP_error__status_noSuchName;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    if (is -> mib_envt == ENVTNOTOK) 
        if (ask_device(is, QUERY) == NOTOK)
	    return int_SNMP_error__status_noSuchName;

    switch (offset)
    {
	case type_SNMP_PDUs_set__request:
	    {
		int m = v -> value -> un.number;

		if ((m < MINRCVTABLEVALUE) || (m > MAXRCVTABLEVALUE) ||
		    m == NONVOLATILE)
		{
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC, GENERIC_5,
			   "%s: bad value"), "s_rcvtable");
		    return int_SNMP_error__status_badValue;
		}

		if (address -> addresslen != DEFAULTADDRESSSIZE)
		    return int_SNMP_error__status_badValue;
	    }
	    break;

	case type_SNMP_PDUs_commit:
	    {
		int k, m = v -> value -> un.number;
		struct nddctl io_block;
		int countadds = 0;
      		ndd_mib_addr_t *rcvlist;
		char *q = NULL;
		ndd_mib_addr_elem_t *p = NULL;
		int results = 0;

		io_block.nddctl_buflen = DEFAULTADDRESSSIZE;
		bcopy(address->address, io_block.nddctl_buf, 
		      DEFAULTADDRESSSIZE);

		if (m == DELETE_ADDRESS)
		{
		    /* 
		     *  This is the address setting ioctl.  This only
		     *  sets multicast temporary address or deletes them.
		     * 
		     *  The device drivers currently do not 
		     *  allow the snmp set protocol to be used with
		     *  this variable.  The belief is that this is an
		     *  important variable to set.  So, snmpd will
		     *  use the normal ioctl method.  The belief is
		     *  also that snmpd should be the super super user
		     *  and have the ability to say on and off.  So,
		     *  to turn the address off, snmpd will need to loop until
		     *  the address goes off.
		     */
		    k = 1;
		    while ((is -> fi_fd != NOTOK) && k)
		    {
		        if (open_device(is) == NOTOK)
			    return int_SNMP_error__status_genErr;

		    	results = ioctl(is -> fi_fd, NDD_DISABLE_ADDRESS,
					&io_block);

			close_device(is);

		        if (results == NOTOK)
		        {
			    if (errno == EOPNOTSUPP)
			      return int_SNMP_error__status_noSuchName;

			    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC,
			       GENERIC_7,
			       "s_rcvtable: deleting an address failed"));

correctaddress: ;	    /*
			     * We have failed somewhere.  Try to 
			     * correct the device drivers ref count 
			     */
		            if (open_device(is) == NOTOK)
			        return int_SNMP_error__status_genErr;
			    for (k = 0; k < countadds; k++)
			    {
		    	        results=ioctl(is->fi_fd, NDD_ENABLE_ADDRESS, 
					      &io_block);
				if (results == NOTOK)
				    break;
			    }

			    close_device(is);
			    return int_SNMP_error__status_genErr;
		        }
			countadds++;

			if (get_rcvtable(is) == NOTOK)
			{
			    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC,
			       GENERIC_8,
		   "s_rcvtable: retrieval of the receive table failed"));
			    goto correctaddress;	
			}

			rcvlist = (ndd_mib_addr_t *)is -> datarcvtable;
			for (i = 0; i < rcvlist -> count; i++)
			{
    q=&is-> datarcvtable[sizeof(u_int) + i * (sizeof(ndd_mib_addr_elem_t) + 4)];
			    p = (ndd_mib_addr_elem_t*)q;

			    if (comp_addr(p, address) == 0)
			    	break;
		        }
			if (i == rcvlist -> count)
			    k = 0;
		    }
		    return int_SNMP_error__status_noError;
		}

                if (open_device(is) == NOTOK)
		    return int_SNMP_error__status_genErr;

		if (ioctl(is->fi_fd, NDD_ENABLE_ADDRESS, &io_block) == NOTOK)
		{
		    close_device(is);

		    if (errno == EOPNOTSUPP)
		      return int_SNMP_error__status_noSuchName;

		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERIC,
		       GENERIC_9,
		       "s_rcvtable: adding an address failed"));
		}
		close_device(is);
	    }
	    break;
    }

    return int_SNMP_error__status_noError;
}


/*  */
init_generic ()
{
    register OT	    ot;

    if (ot = text2obj ("ifExtnsIfIndex")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSIFINDEX;
    }
    if (ot = text2obj ("ifExtnsChipSet")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSCHIPSET;
    }
    if (ot = text2obj ("ifExtnsRevWare")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSREVWARE;
    }
    if (ot = text2obj ("ifExtnsMulticastsTransmittedOks")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSMULTICASTSTRANSMITTEDOKS;
    }
    if (ot = text2obj ("ifExtnsBroadcastsTransmittedOks")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSBROADCASTSTRANSMITTEDOKS;
    }
    if (ot = text2obj ("ifExtnsMulticastsReceivedOks")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSMULTICASTSRECEIVEDOKS;
    }
    if (ot = text2obj ("ifExtnsBroadcastsReceivedOks")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSBROADCASTSRECEIVEDOKS;
    }
    if (ot = text2obj ("ifExtnsPromiscuous")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_setfnx = s_genera;
      ot -> ot_info = (caddr_t) IFEXTNSPROMISCUOUS;
    }

    if (ot = text2obj ("ifExtnsTestIfIndex")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSTESTIFINDEX;
    }
    if (ot = text2obj ("ifExtnsTestCommunity")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSTESTCOMMUNITY;
    }
    if (ot = text2obj ("ifExtnsTestRequestId")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSTESTREQUESTID;
    }
    if (ot = text2obj ("ifExtnsTestType")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_setfnx = s_genera;
      ot -> ot_info = (caddr_t) IFEXTNSTESTTYPE;
    }
    if (ot = text2obj ("ifExtnsTestResult")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSTESTRESULT;
    }
    if (ot = text2obj ("ifExtnsTestCode")) {
      ot -> ot_getfnx = o_genera;
      ot -> ot_info = (caddr_t) IFEXTNSTESTCODE;
    }

    if (ot = text2obj ("ifExtnsRcvAddrIfIndex")) {
      ot -> ot_getfnx = o_rcvtable;
      ot -> ot_info = (caddr_t) IFEXTNSRCVADDRIFINDEX;
    }
    if (ot = text2obj ("ifExtnsRcvAddress")) {
      ot -> ot_getfnx = o_rcvtable;
      ot -> ot_info = (caddr_t) IFEXTNSRCVADDRESS;
    }
    if (ot = text2obj ("ifExtnsRcvAddrStatus")) {
      ot -> ot_getfnx = o_rcvtable;
      ot -> ot_setfnx = s_rcvtable;
      ot -> ot_info = (caddr_t) IFEXTNSRCVADDRSTATUS;
    }
}


#ifdef DEBUG 
void print_generic(vals)
generic_mib_t *vals;
{
        int i;

        for (i = 0; i < 16; i++)
            advise(SLOG_EXCEPTIONS,"ifExtnsEntry.chipset[%d] = %ld", i, vals -> ifExtnsEntry.chipset[i]);	/* ifExtnsChipSet */
        advise(SLOG_EXCEPTIONS,"ifExtnsEntry.revware = %s", vals -> ifExtnsEntry.revware);		/* ifExtnsRevWare */
	advise(SLOG_EXCEPTIONS,"ifExtnsEntry.mcast_tx_ok = %ld", vals -> ifExtnsEntry.mcast_tx_ok);	/* ifExtnsMulticastsTransmittededOks */
	advise(SLOG_EXCEPTIONS,"ifExtnsEntry.bcast_tx_ok = %ld", vals -> ifExtnsEntry.bcast_tx_ok);	/* ifExtnsBroadcastsTransmittedOks */
	advise(SLOG_EXCEPTIONS,"ifExtnsEntry.mcast_rx_ok = %ld", vals -> ifExtnsEntry.mcast_rx_ok);	/* ifExtnsMulticastsReceivedOks */
	advise(SLOG_EXCEPTIONS,"ifExtnsEntry.bcast_rx_ok = %ld", vals -> ifExtnsEntry.bcast_rx_ok);	/* ifExtnsBroadcastsReceivedOks */
	advise(SLOG_EXCEPTIONS,"ifExtnsEntry.promiscuous = %ld", vals -> ifExtnsEntry.promiscuous);	/* ifExtnsPromiscuous */


        advise(SLOG_EXCEPTIONS,"ifExtnsTestEntry.community = %s", vals -> ifExtnsTestEntry.community);		/* ifExtnsTestCommunity - snmpd only */
        advise(SLOG_EXCEPTIONS,"ifExtnsTestEntry.request_id = %ld", vals -> ifExtnsTestEntry.request_id);	/* ifExtnsTestRequestId - snmpd only */
        advise(SLOG_EXCEPTIONS,"ifExtnsTestEntry.type = %ld", vals -> ifExtnsTestEntry.type);		/* ifExtnsTestType */
        advise(SLOG_EXCEPTIONS,"ifExtnsTestEntry.result = %d", vals -> ifExtnsTestEntry.result);		/* ifExtnsTestResult */
	advise(SLOG_EXCEPTIONS,"ifExtnsTestEntry.code = %ld", vals -> ifExtnsTestEntry.code);			/* ifExtnsTestCode */

        advise(SLOG_EXCEPTIONS,"RcvAddrTable = %d", vals -> RcvAddrTable); /* RcvAddrTable */

}

void print_list() 
{
  struct interface *is;

  is = iftimers;
  advise(SLOG_EXCEPTIONS, "Timer list:");
  while (is) {
    advise(SLOG_EXCEPTIONS, "TIMER: %d", is -> ifn_index); 
    advise(SLOG_EXCEPTIONS, "   timeout = %d", is -> timeout);
    advise(SLOG_EXCEPTIONS, "   flush_cache = %d", is -> flush_cache);
    advise(SLOG_EXCEPTIONS, "   flush_rcv   = %d", is -> flush_rcv);
    is = is -> time_next;
  }
}
#endif /* DEBUG */


