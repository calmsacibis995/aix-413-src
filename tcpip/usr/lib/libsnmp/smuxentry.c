static char sccsid[] = "@(#)65	1.5  src/tcpip/usr/lib/libsnmp/smuxentry.c, snmp, tcpip411, GOLD410 10/29/93 09:58:47";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: setsmuxEntry(), endsmuxEntry(), getsmuxEntry(), 
 *            getsmuxEntrybyname(), getsmuxEntrybyidentity()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/smuxentry.c
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

/* smuxentry.c - smuxEntry routines */

/* LINTLIBRARY */

#include <stdio.h>
#include <string.h>
#include <isode/snmp/smux.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

/*    DATA */

static char *smuxEntries = "snmpd.peers";

static FILE *servf = NULL;
static int  stayopen = 0;

static struct smuxEntry    ses;

/*  */

int	setsmuxEntry (f)
int	f;
{
    if (servf == NULL) {
	if ((servf = fopen (smuxEntries, "r")) == NULL
		&& (servf = fopen ("/etc/snmpd.peers", "r")) == NULL) {
	    fprintf (stderr, MSGSTR(MS_SMUXENTRY, ENTRY_1,
			"WARNING: unable to read %s\n"), smuxEntries);
	    return (NOTOK);
	}
    }
    else
	rewind (servf);
    stayopen |= f;

    return (servf != NULL);
}


int	
endsmuxEntry () 
{
    if (servf && !stayopen) {
	(void) fclose (servf);
	servf = NULL;
    }

    return 1;
}

/*  */

struct smuxEntry  *
getsmuxEntry () 
{
    int	    vecp;
    register int i;
    register struct smuxEntry *se = &ses;
    register char  *cp;
    static char buffer[BUFSIZ + 1];
    static char *vec[NVEC + NSLACK + 1];
    static unsigned int elements[NELEM + 1];

    if (servf == NULL && (servf = fopen (smuxEntries, "r")) == NULL
	    && (servf = fopen ("/etc/snmpd.peers", "r")) == NULL) {
	fprintf (stderr, MSGSTR(MS_SMUXENTRY, ENTRY_1,
		"WARNING: unable to read %s\n"), smuxEntries);
	return NULL;
    }

    bzero ((char *) se, sizeof *se);

    while (fgets (buffer, sizeof buffer, servf) != NULL) {
	if (*buffer == '#')
	    continue;
	if (cp = index (buffer, '\n'))
	    *cp = NULL;
	if ((vecp = str2vec (buffer, vec)) < 3)
	    continue;

	if ((i = str2elem (vec[1], elements)) <= 1)
	    continue;

	se -> se_name = vec[0];
	se -> se_identity.oid_elements = elements;
	se -> se_identity.oid_nelem = i;
	se -> se_password = vec[2];
	se -> se_priority = vecp > 3 ? atoi (vec[3]) : -1;

	return se;
    }

    return NULL;
}

/*  */

struct smuxEntry *
getsmuxEntrybyname (name)
char   *name;
{
    register struct smuxEntry *se;

    (void) setsmuxEntry (0);
    while (se = getsmuxEntry ())
	if (strcmp (name, se -> se_name) == 0)
	    break;
    (void) endsmuxEntry ();

    return se;
}

/*  */

struct smuxEntry *
getsmuxEntrybyidentity (identity)
OID	identity;
{
    register struct smuxEntry *se;

    (void) setsmuxEntry (0);
    while (se = getsmuxEntry ())
	if (oid_cmp (identity, &se -> se_identity) == 0)
	    break;
    (void) endsmuxEntry ();

    return se;
}

