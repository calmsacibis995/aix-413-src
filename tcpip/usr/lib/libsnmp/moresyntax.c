static char sccsid[] = "@(#)57	1.8  src/tcpip/usr/lib/libsnmp/moresyntax.c, snmp, tcpip411, GOLD410 10/29/93 09:58:38";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:  enum_print(), services_print(), privs_print(), moresyntax(),
 *	       time_print()	
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/moresyntax.c
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

#include <stdio.h>
#include <isode/snmp/objects.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

/*    SYNTAX */

static int enum_print(), services_print(), privs_print(), time_print();

static char *ifType[] = {
    "other", "regular1822", "hdh1822", "ddn-x25", "rfc877-x25",
    "ethernet-csmacd", "iso88023-csmacd", "iso88024-tokenBus",
    "iso88025-tokenRing", "iso88026-man", "starLan", "proteon-10Mbit",
    "proteon-80Mbit", "hyperchannel", "fddi", "lapb", "sdlc", "t1-carrier",
     "cept", "basicISDN", "primaryISDN", "propPointToPointSerial",
    "terminalServer-asyncPort", "softwareLoopback", "eon", "ethernet-3Mbit",
    "nsip", "slip", "ultra", "ds3", "sip", "frame-delay"
};

static char *ifStatus[] = {
    "up", "down", "testing"
};

static char *ipForwarding[] = {
    "gateway", "host"
};

static char *ipRouteType[] = {
    "other", "invalid", "direct", "remote"
};

static char *ipRouteProto[] = {
    "other", "local", "netmgmt", "icmp", "egp", "ggp", "hello", "rip", "is-is",
    "es-is", "ciscoIgrp", "bbnSpfIgp", "ospf", "bgp"
};

static char *ipNetToMediaType[] = {
    "other", "invalid", "dynamic", "static"
};

static char *tcpRtoAlgorithm[] = {
    "other", "constant", "rsre", "vanj"
};

static char *tcpConnState[] = {
    "closed", "listen", "synSent", "synReceived", "established", "finWait1",
    "finWait2", "closeWait", "lastAck", "closing", "timewait"
};

static char *egpNeighState[] = {
    "idle", "acquisition", "down", "up", "cease"
};

static char *egpNeighMode[] = {
    "active", "passive", 
};

static char *egpNeighEventTrigger[] = {
    "start", "stop"
};

static char *snmpEnableAuthTraps [] = {
    "enabled", "disabled"
};

static char *validInvalid[] = {
    "valid", "invalid"
};

static char *smuxPstatus[] = {
    "valid", "invalid", "connecting"
};

static struct ivar {
    char   *iv_object;
    IFP	   iv_print;

    char  **iv_values;
    int	    iv_nvalue;
} ivars[] = {
    "ifType", enum_print, ifType,
	sizeof ifType / sizeof ifType[0],
    
    "ifAdminStatus", enum_print, ifStatus,
	sizeof ifStatus / sizeof ifStatus[0],
    
    "ifOperStatus", enum_print, ifStatus,
	sizeof ifStatus / sizeof ifStatus[0],

    "ipForwarding", enum_print, ipForwarding,
	sizeof ipForwarding / sizeof ipForwarding[0],

    "ipRouteType", enum_print, ipRouteType,
	sizeof ipRouteType / sizeof ipRouteType[0],

    "ipRouteProto", enum_print, ipRouteProto,
	sizeof ipRouteProto / sizeof ipRouteProto[0],

    "ipRouteAge", time_print, 0,
	0,

    "ipNetToMediaType", enum_print, ipNetToMediaType,
	sizeof ipNetToMediaType / sizeof ipNetToMediaType[0],

    "tcpRtoAlgorithm", enum_print, tcpRtoAlgorithm,
	sizeof tcpRtoAlgorithm / sizeof tcpRtoAlgorithm[0],

    "tcpConnState", enum_print, tcpConnState,
	sizeof tcpConnState / sizeof tcpConnState[0],

    "egpNeighState", enum_print, egpNeighState,
	sizeof egpNeighState / sizeof egpNeighState[0],

    "egpNeighMode", enum_print, egpNeighMode,
	sizeof egpNeighMode / sizeof egpNeighMode[0],

    "egpNeighEventTrigger", enum_print, egpNeighEventTrigger,
	sizeof egpNeighEventTrigger / sizeof egpNeighEventTrigger[0],

    "snmpEnableAuthTraps", enum_print, snmpEnableAuthTraps,
	sizeof snmpEnableAuthTraps / sizeof snmpEnableAuthTraps[0],

    "sysServices", services_print, 0,
	0,

    "viewAclPrivileges", privs_print, 0,
	0,

    "viewPrimType", enum_print, validInvalid,
	sizeof validInvalid / sizeof validInvalid[0],

    "viewAclType", enum_print, validInvalid,
	sizeof validInvalid / sizeof validInvalid[0],

    "viewTrapType", enum_print, validInvalid,
	sizeof validInvalid / sizeof validInvalid[0],

    "smuxPstatus", enum_print, smuxPstatus,
	sizeof smuxPstatus / sizeof smuxPstatus[0],

    "smuxTstatus", enum_print, validInvalid,
	sizeof validInvalid / sizeof validInvalid[0],

    NULL, 0, 0, 0
};

/*  */

static int  
enum_print (x, os)
integer *x;
OS	os;
{
    int	    i = *x;

    if (i <= 0 || i > os -> os_data2)
	printf ("unknown(%d)", i);
    else
	printf ("%s(%d)", os -> os_data1[i - 1], i);
    return(0);
}

/* ARGSUSED */

static int  
services_print (x, os)
integer *x;
OS	os;
{
    printf ("%s", sprintb (*x,
			   "\020\01physical\02datalink/subnetwork\03internet\04transport\05session\06presentation\07application"));
    return(0);
}

static int  
privs_print (x, os)
integer *x;
OS	os;
{
    printf ("%s", sprintb ((int) *x,
			   "\020\01get\02get-next\03get-response\04set\05trap"));
    return(0);
}


static int  
time_print (x, os)
integer *x;
OS	os;
{
    int	    d,
	    h,
	    m,
	    s,
    	    ds;

    ds = *x;
    m = ds / 60, s = ds % 60;
    h = m / 60, m = m % 60;
    d = h / 24, h = h % 24;

    if (d > 0)
	printf ("%d days, ", d);
    if (d > 0 || h > 0)
	printf ("%d hours, ", h);
    if (d > 0 || h > 0 || m > 0)
	printf ("%d minutes, ", m);
    printf ("%d seconds", s);
    printf ("(total %d seconds)", ds);

    return OK;
}


moresyntax () 
{
    register struct ivar *iv;
    register OT	   ot;
    register OS	   os;

    for (iv = ivars; iv -> iv_object; iv++)
	if (ot = text2obj (iv -> iv_object)) {
	    if ((os = ot -> ot_syntax) == NULL) {
		/* USED TO BE ADVISE() */
		/*
		fprintf (stderr, MSGSTR(MS_MORES, MORES_1,
			"moresyntax: no syntax defined for object \"%s\"\n"),
			iv -> iv_object);
		*/
		continue;
	    }

	    (void) add_syntax (iv -> iv_object, 
				os -> os_encode, os -> os_decode, 
				os -> os_free, os -> os_parse,
			       iv -> iv_print);
	    if ((os = text2syn (iv -> iv_object)) == NULL) {
		/* USED TO BE ADIOS() */
		fprintf (stderr, MSGSTR(MS_MORES, MORES_2,
			"moresyntax: lost syntax for object \"%s\"\n"),
			iv -> iv_object);
		exit (1);
	    }
	    ot -> ot_syntax = os;
	    os -> os_data1 = iv -> iv_values;
	    os -> os_data2 = iv -> iv_nvalue;
	}

/* We don't care anymore.  If they don't have the required entries
 * in mib.defs, then they don't get the extra print features.  That's all.
 */
#if 0
	else
	    /* USED TO BE ADVISE() */
	    fprintf (stderr, MSGSTR(MS_MORES, MORES_3,
		"moresyntax: no \"%s\" object\n", iv -> iv_object);
#endif
}

