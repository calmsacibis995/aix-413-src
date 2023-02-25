/* @(#)88	1.9  src/tcpip/usr/include/isode/snmp/objects.h, snmp, tcpip411, GOLD410 10/29/93 09:45:20 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: clnpaddr2oid ipaddr2oid o_clnpaddr o_integer
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
 * FILE: src/tcpip/usr/include/isode/snmp/objects.h
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


#ifndef _OBJECTS_
#define _OBJECTS_

#include <isode/psap.h>

/*  */

typedef struct object_syntax {
    char   *os_name;			/* syntax name */

    IFP	    os_encode;			/* data -> PE */
    IFP	    os_decode;			/* PE -> data */
    IFP	    os_free;			/* free data */

    IFP	    os_parse;			/* str -> data */
    IFP	    os_print;			/* data -> tty */

    char  **os_data1;			/* for moresyntax() in snmpi... */
    int	    os_data2;			/*   .. */
}		object_syntax, *OS;
#define	NULLOS	((OS) 0)

int	readsyntax (), add_syntax ();
OS	text2syn ();

void unchain();
void object_remove();
int delete_object();

/*  */

typedef struct object_type {
    char   *ot_text;			/* OBJECT DESCRIPTOR */
    char   *ot_id;			/* OBJECT IDENTIFIER */
    OID	    ot_name;			/*   .. */
    char   *ot_ltext;			/*   .. */

    OS	    ot_syntax;			/* SYNTAX */

    int	    ot_access;			/* ACCESS */
#define	OT_NONE		0x00
#define	OT_RDONLY	0x01
#define	OT_WRONLY	0x02
#define	OT_RDWRITE	(OT_RDONLY | OT_WRONLY)

    u_long  ot_views;			/* for views */

    int	    ot_status;			/* STATUS */
#define	OT_OBSOLETE	0x00
#define	OT_MANDATORY	0x01
#define	OT_OPTIONAL	0x02
#define	OT_DEPRECATED	0x03

    caddr_t ot_info;			/* object information */
    IFP	    ot_getfnx;			/* get/get-next method */
    IFP	    ot_setfnx;			/* set method */
    IFP	    ot_tsetfnx;			/* table set method */
#define type_SNMP_PDUs_commit	(-1)
#define type_SNMP_PDUs_rollback	(-2)

    caddr_t ot_save;			/* for set method */

    caddr_t ot_smux;			/* for SMUX */

    struct object_type *ot_chain;	/* hash-bucket for text2obj */

    struct object_type *ot_sibling;	/* linked-list for name2obj */
    struct object_type *ot_children;	/*   .. */

    struct object_type *ot_next;	/* linked-list for get-next */
    struct object_type *ot_prev;	/*   .. */
}		object_type, *OT;
#define	NULLOT	((OT) 0)


int	readobjects ();
int	add_objects ();
OT	name2obj (), text2obj ();
OID	text2oid ();
char   *oid2ode_aux ();


typedef struct object_instance {
    OID	    oi_name;			/* instance OID */

    OT	    oi_type;			/* prototype */
}		object_instance, *OI;
#define	NULLOI	((OI) 0)

OI	name2inst (), next2inst (), nextot2inst (), text2inst ();

/* the bare information needed to build up our internal database */
typedef struct import_object_type {
    char    *iot_text;			/* OBJECT DESCRIPTOR */
    char    *iot_id;			/* OBJECT IDENTIFIER */
    int	    iot_syntax;			/* SYNTAX */
    int	    iot_access;			/* ACCESS */
    int	    iot_status;			/* STATUS */
}	import_object_type, *IOT;

int	importobjects ();

/*  */

extern	IFP	o_advise;
extern	IFP	o_adios;


int	o_generic (), o_igeneric (), s_generic ();

int	o_number ();
#define	o_integer(oi,v,number)	o_number ((oi), (v), (integer) (number))

int	o_string ();
int	o_qbstring ();

int	o_specific ();
int	o_ipaddr ();
#ifdef	BSD44
#define	o_clnpaddr(oi,v,value)	o_specific ((oi), (v), (caddr_t) (value))
#endif


int	mediaddr2oid ();
#define	ipaddr2oid(ip,addr) \
	mediaddr2oid ((ip), (u_char*) (addr), sizeof (struct in_addr), 0)
#ifdef	BSD44
#define	clnpaddr2oid(ip,addr) \
	mediaddr2oid ((ip), \
		      (u_char *) (addr) -> isoa_genaddr, \
		      (int) (addr) -> isoa_len, 1)
#endif

OID	oid_extend (), oid_normalize ();

struct type_SMI_ObjectSyntax *alloc_SMI_ObjectSyntax ();

/*  */

extern	int	debug;
extern	char	PY_pepy[];

int cistrncmp();

char   *strdup ();

#endif /* _OBJECTS_ */
