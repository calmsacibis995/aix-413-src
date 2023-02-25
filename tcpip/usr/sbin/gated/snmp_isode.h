/* @(#)25	1.1  src/tcpip/usr/sbin/gated/snmp_isode.h, tcprouting, tcpip411, GOLD410 12/6/93 17:27:36 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: INT_OID
 *		PROTOTYPE
 *		STR_OID
 *		defined
 *		oid2ipaddr
 *		ot2object
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
 * snmp_isode.h,v 1.10.2.1 1993/08/27 22:28:44 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#if	defined(PROTO_ISODE_SNMP)

extern int doing_snmp;
extern flag_t snmp_trace_flags;
extern pref_t snmp_preference;
extern u_short snmp_port;
extern int snmp_quantum;
extern int snmp_debug;
#ifdef	INCLUDE_ISODE_SNMP
extern OID snmp_nullSpecific;
#endif	/* INCLUDE_ISODE_SNMP */

#define	SMUX_PORT	199

#define	SMUX_TIMER_STARTUP	0
#define	SMUX_TIMER_MAX		1

#define	oid2ipaddr(ip,addr) \
	oid2mediaddr ((ip), (byte*) (addr), sizeof (struct in_addr), 0)

#ifdef	INCLUDE_ISODE_SNMP
struct object_table {
    const char *ot_object;
    _PROTOTYPE(ot_getfunc,
	       int,
	       (OI,
		struct type_SNMP_VarBind *,
		int));
    _PROTOTYPE(ot_setfunc,
	       int,
	       (OI,
		struct type_SNMP_VarBind *,
		int));
    const int	ot_info;
    OT	ot_type;
};

struct snmp_tree {
    struct snmp_tree *r_forw;
    struct snmp_tree *r_back;
    const char *r_text;			/* Name of this tree */
    OID r_name;				/* OID of the tree */
    const int r_mode;			/* Mode (readWrite, readOnly) */
    struct object_table *const r_table;	/* Table of objects */
    flag_t r_flags;			/* state */
#define	SMUX_TREE_REGISTER	0x01	/* Tree has been registered */
#define	SMUX_TREE_ACTION	0x02	/* Action pending */
#define	SMUX_TREE_OBJECTS	0x04	/* Objects have been converted */
};

#endif	/* INCLUDE_ISODE_SNMP */

#define	STR_OID(ip, addr, len) { register byte *str_oid_cp = (byte *) addr; \
				    register int str_oid_i = len; \
				    while (str_oid_i--) *ip++ = *str_oid_cp++ & 0xff; }

#define	INT_OID(ip, int) *ip++ = int

#define	ot2object(ot)	((struct object_table *) ((void_t) ot->ot_info))
    
PROTOTYPE(snmp_restart,
	  extern void,
	  (task *));
PROTOTYPE(snmp_init,
	  extern void,
	  (void));
PROTOTYPE(snmp_var_init,
	  extern void,
	  (void));
PROTOTYPE(snmp_last_free,
	  extern void,
	  (unsigned int **));
PROTOTYPE(snmp_last_match,
	  extern int,
	  (unsigned int **,
	   unsigned int *,
	   size_t,
	   int));
PROTOTYPE(oid2mediaddr,
	  extern int,
	  (unsigned int *,
	   byte *,
	   int,
	   int));

#ifdef	INCLUDE_ISODE_SNMP
PROTOTYPE(snmp_trap,
	  extern void,
	  (const char *,
	   OID,
	   int,
	   int,
	   struct type_SNMP_VarBindList *));
PROTOTYPE(snmp_tree_register,
	  extern void,
	  (struct snmp_tree *));
PROTOTYPE(snmp_tree_unregister,
	  extern void,
	  (struct snmp_tree *));

PROTOTYPE(snmp_varbinds_build,
	  extern int,
	  (int,
	   struct type_SNMP_VarBindList *,
	   struct type_SNMP_VarBind *,
	   int *,
	   struct snmp_tree *,
	   _PROTOTYPE(build,
		      int,
		      (OI,
		       struct type_SNMP_VarBind *,
		       int,
		       void_t)),
	   void_t));
PROTOTYPE(snmp_varbinds_free,
	  extern void,
	  (struct type_SNMP_VarBindList *));
#endif	/* INCLUDE_ISODE_SNMP */

#endif		/* defined(PROTO_ISODE_SNMP) */


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
 */
