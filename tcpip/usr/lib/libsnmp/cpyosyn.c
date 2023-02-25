static char sccsid[] = "@(#)70	1.6  src/tcpip/usr/lib/libsnmp/cpyosyn.c, snmp, tcpip411, GOLD410 4/5/93 17:28:47";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: cpy_SMI_ObjectSyntax ()
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/cpyosyn.c
 */

#include <stdio.h>
#include <isode/pepsy/SMI-types.h>
#include <isode/snmp/objects.h>

/* EXTERN */
extern struct qbuf *qb_cpy ();

struct type_SMI_ObjectSyntax *
cpy_SMI_ObjectSyntax (i)
struct type_SMI_ObjectSyntax	*i;
{
    struct type_SMI_ObjectSyntax	*o;

    if ((o = alloc_SMI_ObjectSyntax ()) == NULL)
	return NULL;

    o -> offset = i -> offset;
    switch (i -> offset) {
	case type_SMI_ObjectSyntax_number:	/* integer */
	    o -> un.number = i -> un.number;
	    break;

	case type_SMI_ObjectSyntax_string:	/* qbuf * */
	    o -> un.string = qb_cpy (i -> un.string, 0);
	    break;
	case type_SMI_ObjectSyntax_internet:
	    o -> un.internet = qb_cpy (i -> un.internet, 0);
	    break;
	case type_SMI_ObjectSyntax_arbitrary:
	    o -> un.arbitrary = qb_cpy (i -> un.arbitrary, 0);
	    break;

	case type_SMI_ObjectSyntax_object:	/* OID */
	    o -> un.object = oid_cpy (i -> un.object);
	    break;

	case type_SMI_ObjectSyntax_empty:	/* char */
	    o -> un.empty = '\0';
	    break;

	case type_SMI_ObjectSyntax_counter:	/* just an integer... */
	{
	    struct type_SMI_Counter *c;

	    if ((c = (struct type_SMI_Counter *) calloc (1, sizeof *c))
		    == NULL) {
		free_SMI_ObjectSyntax (o);
		return NULL;
	    }
	    c -> parm = i -> un.counter -> parm;
	    o -> un.counter = c;
	    break;
	}
	case type_SMI_ObjectSyntax_gauge:
	{
	    struct type_SMI_Gauge *g;

	    if ((g = (struct type_SMI_Gauge *) calloc (1, sizeof *g))
		    == NULL) {
		free_SMI_ObjectSyntax (o);
		return NULL;
	    }
	    g -> parm = i -> un.gauge -> parm;
	    o -> un.gauge = g;
	    break;
	}
	case type_SMI_ObjectSyntax_ticks:
	{
	    struct type_SMI_TimeTicks *t;

	    if ((t = (struct type_SMI_TimeTicks *) calloc (1, sizeof *t))
		    == NULL) {
		free_SMI_ObjectSyntax (o);
		return NULL;
	    }
	    t -> parm = i -> un.ticks -> parm;
	    o -> un.ticks = t;
	    break;
	}
	default:			/* unknown */
	    free_SMI_ObjectSyntax (o);
	    return NULL;
    }
    return o;
}
