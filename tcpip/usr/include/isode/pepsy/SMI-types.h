/* @(#)33	1.3  src/tcpip/usr/include/isode/pepsy/SMI-types.h, isodelib7, tcpip411, GOLD410 4/5/93 13:21:51 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: free_SMI_ApplicationSyntax free_SMI_Counter free_SMI_Gauge 
 *    free_SMI_ObjectSyntax free_SMI_SimpleSyntax free_SMI_TimeTicks
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/SMI-types.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* automatically generated by pepsy 6.0 #104 (vikings.austin.ibm.com), do not edit! */

#ifndef	_module_SMI_defined_
#define	_module_SMI_defined_

#ifndef	PEPSY_VERSION
#define	PEPSY_VERSION		2
#endif

#include <isode/psap.h>
#include <isode/pepsy.h>
#include <isode/pepsy/UNIV-types.h>

#include <isode/pepsy/SMI_defs.h>

#define	type_SMI_ObjectName	OIDentifier
#define	free_SMI_ObjectName	oid_free

#define	type_SMI_NetworkAddress	type_SMI_IpAddress
#define	free_SMI_NetworkAddress	free_SMI_IpAddress

#define	type_SMI_IpAddress	qbuf
#define	free_SMI_IpAddress	qb_free

#define	type_SMI_Opaque	qbuf
#define	free_SMI_Opaque	qb_free

#define	type_SMI_DisplayString	qbuf
#define	free_SMI_DisplayString	qb_free

struct type_SMI_ObjectSyntax {
    int         offset;
#define	type_SMI_ObjectSyntax_number	1
#define	type_SMI_ObjectSyntax_string	2
#define	type_SMI_ObjectSyntax_object	3
#define	type_SMI_ObjectSyntax_empty	4
#define	type_SMI_ObjectSyntax_internet	5
#define	type_SMI_ObjectSyntax_counter	6
#define	type_SMI_ObjectSyntax_gauge	7
#define	type_SMI_ObjectSyntax_ticks	8
#define	type_SMI_ObjectSyntax_arbitrary	9

    union {
        integer     number;

        struct qbuf *string;

        OID     object;

        char    empty;

        struct type_SMI_IpAddress *internet;

        struct type_SMI_Counter *counter;

        struct type_SMI_Gauge *gauge;

        struct type_SMI_TimeTicks *ticks;

        struct type_SMI_Opaque *arbitrary;
    }       un;
};
#define	free_SMI_ObjectSyntax(parm)\
	(void) fre_obj((char *) parm, _ZSMI_mod.md_dtab[_ZObjectSyntaxSMI], &_ZSMI_mod, 1)

struct type_SMI_SimpleSyntax {
    int         offset;
#define	type_SMI_SimpleSyntax_number	1
#define	type_SMI_SimpleSyntax_string	2
#define	type_SMI_SimpleSyntax_object	3
#define	type_SMI_SimpleSyntax_empty	4

    union {
        integer     number;

        struct qbuf *string;

        OID     object;

        char    empty;
    }       un;
};
#define	free_SMI_SimpleSyntax(parm)\
	(void) fre_obj((char *) parm, _ZSMI_mod.md_dtab[_ZSimpleSyntaxSMI], &_ZSMI_mod, 1)

struct type_SMI_ApplicationSyntax {
    int         offset;
#define	type_SMI_ApplicationSyntax_internet	1
#define	type_SMI_ApplicationSyntax_counter	2
#define	type_SMI_ApplicationSyntax_gauge	3
#define	type_SMI_ApplicationSyntax_ticks	4
#define	type_SMI_ApplicationSyntax_arbitrary	5

    union {
        struct type_SMI_IpAddress *internet;

        struct type_SMI_Counter *counter;

        struct type_SMI_Gauge *gauge;

        struct type_SMI_TimeTicks *ticks;

        struct type_SMI_Opaque *arbitrary;
    }       un;
};
#define	free_SMI_ApplicationSyntax(parm)\
	(void) fre_obj((char *) parm, _ZSMI_mod.md_dtab[_ZApplicationSyntaxSMI], &_ZSMI_mod, 1)

struct type_SMI_Counter {
    integer     parm;
};
#define	free_SMI_Counter(parm)\
	(void) fre_obj((char *) parm, _ZSMI_mod.md_dtab[_ZCounterSMI], &_ZSMI_mod, 1)

struct type_SMI_Gauge {
    integer     parm;
};
#define	free_SMI_Gauge(parm)\
	(void) fre_obj((char *) parm, _ZSMI_mod.md_dtab[_ZGaugeSMI], &_ZSMI_mod, 1)

struct type_SMI_TimeTicks {
    integer     parm;
};
#define	free_SMI_TimeTicks(parm)\
	(void) fre_obj((char *) parm, _ZSMI_mod.md_dtab[_ZTimeTicksSMI], &_ZSMI_mod, 1)
#endif
