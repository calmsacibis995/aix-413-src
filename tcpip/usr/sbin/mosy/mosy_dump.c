static char sccsid[] = "@(#)39	1.7  src/tcpip/usr/sbin/mosy/mosy_dump.c, isodelib7, tcpip411, GOLD410 10/29/93 10:04:24";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: lookup_syntax(), get_ttl(), dumpoid(), v_access(),
 *	      v_status(), dump_syntax_table(), object_dump()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/mosy/mosy_dump.c
 */

/*
 * Current restriction on objects.c:  we cannot have 2 mib objects
 * with the same name, even if they are in two different parts of
 * the tree.  This needs to be fixed.
 */

#include <stdio.h>
#include <isode/snmp/objects.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

extern OT get_anchor();		/* from objects.c */
extern char* new_string();	/* from mosy.c 	  */
extern char autogen[];		/* ... */


/*
 * NAME: lookup_syntax
 *
 * FUNCTION: Convert object syntax to mib_desc syntax format.
 *
 * RETURNS: Object syntax in mib_desc syntax format (SUCCESS)
 *	    NULL (FAILURE)
 */
char *
lookup_syntax (syntax) 
char *syntax;
{
    if (lexequ (syntax,"INTEGER") == 0)
	return ("number");
    if (lexequ (syntax, "ObjectID") == 0)
	return ("object");
    if (lexequ (syntax, "Counter") == 0)
	return ("counter");
    if (lexequ (syntax, "Gauge") == 0)
	return ("gauge");
    if (lexequ (syntax, "TimeTicks") == 0)
	return ("ticks");
    if ((lexequ (syntax, "OctetString") == 0) || 
	    (lexequ (syntax, "DisplayString") == 0) ||
	    (lexequ (syntax, "PhysAddress") == 0))
	return ("string");
    if ((lexequ (syntax, "IpAddress") == 0) || 
	    (lexequ (syntax, "NetworkAddress") == 0))
	return ("internet");
    return ("");
}

/*
 * NAME: get_ttl
 *
 * FUNCTION: Get time to live value for an object.
 *
 * RETURNS: time to live value in seconds.
 */
int
get_ttl (id)
char *id;
{

    char *strinst;
    int intid;

    /*
     * Get object instance.
     */
    strinst = (char *) strchr (id, '.');
    strinst++;
    intid = atoi (strinst);

    /* 
     * System group.
     */
    if (strncmp (id, "sys", 3) == 0) {
	/* sysUpTime */
	if ((intid != 3))  
	    return 900;
    }

    /* 
     * Interfaces group.
     */
    if (strncmp (id, "ifEntry", 7) == 0) {
	if ((intid >= 1) && (intid <= 6))
	    return 900;
	if ((intid == 7))
	    return 10;
	if ((intid == 8))
	    return 3;
    }
    if (strncmp (id, "interfaces", 10) == 0) {
	if ((intid == 1))
	    return 900;
    }

    /* 
     * AT group.
     */
    if (strncmp (id, "at", 2) == 0)
	if ((intid >= 1) && (intid <= 3))
	    return 900;

    /* 
     * IP group.
     */
    if (strncmp (id, "ip.", 3) == 0) 
	if (((intid == 1) || (intid == 2)) || (intid == 14))
	    return 900;
    if (strncmp (id, "ipAddr", 6) == 0) {
	if ((intid >= 1) && (intid <= 4))
	    return 900;
    }
    if (strncmp (id, "ipRoute", 7) == 0) {
	if ((intid >= 1) && (intid <= 9))
	    return 10;
    }

    /* tcp group */
    if ((strncmp (id, "tcp.", 4) == 0 )) {
	if (intid == 1)
	    return 900;
	if ((intid >= 2) && (intid <= 12))
	    return 10;
    }
    if ((strncmp (id, "tcpConn", 7) == 0)) {
	if ((intid >= 1) && (intid <= 5))
	    return 10;

    }

    /* 
     * EGP group. 
     */
    if (strncmp (id, "egp.", 4) == 0)
	if ((intid >=1) && (intid <= 4))
	    return 10;
    if (strncmp (id, "egpNeigh", 8) == 0)
	if ((intid == 1) || (intid == 2))
	    return 10;

    /*
     * All others.
     */
    return 1;

}

/*
 * NAME: dumpoid
 *
 * FUNCTION: Convert objects from inputfile in mib.defs format to
 *           mib_desc format.  The mib_desc format looks like the 
 * 	     following:
 *		    obj.text obj.name  	      syntax ttl
 *		    sysDescr 1.3.6.1.2.1.1.1. string 900
 */
int
dumpoid (xfp) 
FILE *xfp;
{

    register OT ot;
    char *syntax;
    int  ttl;

    fprintf (xfp, MSGSTR(MS_INFO, INFO_37, "# MIB variable descriptions\n"));

    /*
     * Convert objects to mib_desc file format.
     */
    for (ot = get_anchor(); ot; ot = ot -> ot_next) {
	char otname[80];

	/* 
	 * Skip aggregates.
	 */
	if (ot -> ot_syntax == NULL) 
	    continue;
	syntax = lookup_syntax (ot -> ot_syntax -> os_name);
	ttl = get_ttl (ot -> ot_id);
	sprintf (otname, "%s.", sprintoid (ot -> ot_name));
	fprintf (xfp, "%-23s %-23s %-15s %d\n", 
		ot -> ot_text, otname, syntax, ttl);
    }
    return OK;
}

/***** support for importobjects() c-code creation *****/

/* be nice.  Turn access and syntax into #defines that humans can read */
static char *
v_access (access)
int	access;
{
    if (!access)
	return ("OT_NONE");
    if ((access & OT_RDWRITE) == OT_RDWRITE)
	return ("OT_RDWRITE");
    if (access & OT_WRONLY)
	return ("OT_WRONLY");
    return ("OT_RDONLY");
}

static char *
v_status (access)
int	access;
{
    if (!access)
	return ("OT_NONE");	/* should be OBSOLETE... but doesn't apply.. */
    if ((access & OT_DEPRECATED) == OT_DEPRECATED)
	return ("OT_DEPRECATED");
    if (access & OT_OPTIONAL)
	return ("OT_OPTIONAL");
    return ("OT_MANDATORY");
}

extern object_syntax syntaxes[];		/* from syntax.c */
extern OS synlast;		 		/* ... */

static void
dump_syntax_table (cfp)
FILE *cfp;
{
    register OS	    os;
    int	i = 0;

    fprintf (cfp, MSGSTR(MS_INFO,INFO_38,"/* a mapping to syntax name */\n\n"));
    fprintf (cfp, "static char *syntax[] = {\n");
    fprintf (cfp, "#define type_SYNTAX_NULLOS\t%d\n\tNULLCP,\n", i++);
    for (os = syntaxes; os < synlast; os++, i++)
	fprintf (cfp, "#define type_SYNTAX_%s\t%d\n\t\"%s\",\n",
		os -> os_name, i, os -> os_name);
    fprintf (cfp, "};\n\n");
}


/*
 *	Generates static structures (to avoid name conflicts) plus
 *	an init stub (init_objects).
 */
int
object_dump (cfp)
FILE *cfp;
{
    register OT ot;

    fprintf (cfp, MSGSTR(MS_INFO, INFO_39,
	    "/* automatically generated by %s, do not edit! */\n"), 
	    autogen);
    fprintf (cfp, "\n#ifndef PEPYPATH\n#include <isode/snmp/objects.h>\n#else\n#include \"objects.h\"\n#endif\n\n");
    fprintf (cfp, "#ifndef NULL\n#define NULL 0\n#endif\n\n");

    /* Dump the syntax table. */ 
    dump_syntax_table (cfp);

    /* minimal object table */
    fprintf (cfp, "static struct import_object_type table_MIB_iobjects[] = {\n");
    for (ot = get_anchor(); ot; ot = ot -> ot_next) 
	fprintf (cfp, "\t{ \"%s\", \"%s\",\n\t\ttype_SYNTAX_%s, %s, %s},\n", 
		ot -> ot_text, ot -> ot_id, 
		ot -> ot_syntax == NULLOS ? "NULLOS" : 
			ot -> ot_syntax -> os_name,
		v_access (ot -> ot_access), v_status (ot -> ot_status));
    fprintf (cfp, "\t{ NULL, NULL,\n\t\ttype_SYNTAX_NULLOS, 0, 0},\n};\n\n");

    /* init_objects subroutine */
    fprintf (cfp, "int\ninit_objects()\n{\n");
    fprintf (cfp, "\treturn (importobjects (table_MIB_iobjects, syntax));\n}\n\n");
    return OK;
}

