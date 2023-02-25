#if !defined(lint)
static char sccsid[] = "@(#)06	1.6  src/tenplus/sf_progs/af.c, tenplus, tenplus411, GOLD410 3/23/93 12:10:07";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	next_stanza, attr_dump, dump_stanza, free_stanza
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: af.c - access routine for a structured file stanza.                */
/****************************************************************************/

/* System Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/* Ined Header Files */
#include <chardefs.h>
#include <database.h>
#include "af.h"


AFILE_t Afp = NULL;     /* Attribute file handle                         */
STANZA  Stanza;         /* Structure version of the stanza just digested */


/****************************************************************************/
/* next_stanza: read and parse the next stanza from the attribute file.     */
/*   The global variable Stanza is updated to reflect the result.           */
/*   ERROR is returned if the file cannot be opened, if bad data is         */
/*   encountered, if malloc fails, or on end of file.  RET_OK is returned,  */
/*   otherwise.                                                             */
/****************************************************************************/

int
next_stanza(void)
{
    auto     ATTR_t   attr;          /* Data from a stanza                 */
    auto     char    *sname;         /* Name field of record stanza        */
    auto     char    *stype;         /* Type field of record stanza        */
    auto     char    *sflags;        /* Flags field of record stanza       */
    auto     char    *scount;        /* Count field of record stanza       */
    auto     char    *svalue;        /* Value field of record stanza       */
    auto     char    *raname;        /* Name of the record array, if any   */
    register char    *cp;            /* Points into the stanza name        */
    register char    *bp;            /* Basename of stanza                 */
    free_stanza();

    attr = AFnxtrec( Afp );

#ifdef MAXDEBUG
    debug( "next_stanza: attr = %#o", attr );
    attr_dump( attr );
#endif

    if (attr == NULL)
	return( ERROR );

    sname  = AFgetatr( attr, "" );
    stype  = AFgetatr( attr, "Type" );
    sflags = AFgetatr( attr, "Flags" );
    scount = AFgetatr( attr, "Count" );
    svalue = AFgetatr( attr, "Value" );
    raname = AFgetatr( attr, "Name" );

    if ((sname  == NULL) || (stype  == NULL) || (sflags == NULL) || 
	(svalue == NULL))
	return( ERROR );

#ifdef DEBUG
    debug( "next_stanza: sname  = \"%s\"", sname );
    debug( "             stype  = \"%s\"", stype );
    debug( "             sflags = \"%s\"", sflags );
    if (scount != NULL)
	debug( "             scount = \"%s\"", scount );
    debug( "             svalue = \"%s\"", svalue );
    if (raname != NULL)
	debug( "             raname = \"%s\"", raname );
#endif

    /* Convert the attributes to the appropriate types */

    Stanza.type = atoi( stype );

    if (Stanza.type == T_CHAR) {
	Stanza.value = malloc( (unsigned) (strlen( svalue ) + 1 ) );
	if (Stanza.value == NULL) {
	    free_stanza();
	    return( ERROR );
	}

	Stanza.count = strlen( svalue );
	(void) strcpy( Stanza.value, svalue );
    }
    else if ((Stanza.type == T_POINTER) && (scount != NULL)) {
	Stanza.count = atoi( scount );
	Stanza.value = NULL;
    }
    else if (Stanza.type == 0)     /* For NULL pointers */
	/* Empty statement */ ;
    else if ((Stanza.type == T_RECORD) && (scount != NULL)) {
	Stanza.count = atoi( scount );
	Stanza.value = NULL;
    }
    else {    /* Bad data in attribute file */
	free_stanza();
	return( ERROR );
    }

    if ((strcmp( sflags, "0" )!=0)) {
        if ((sflags[0] == '0') && (sflags[1] == 'x') && (sflags[3] == '\0'))
            Stanza.flags = sflags[2] - '0';
        else
	{
	    free_stanza();
	    return( ERROR );
	}
    }
    else
	Stanza.flags = 0;

    /* Process the name field of the record array, if any */

    if (raname != NULL) {  /* there was a name field for the record array */
	Stanza.raname = malloc( (unsigned) (strlen( raname ) + 1) );
	if (Stanza.raname == NULL) {
	    free_stanza();
	    return( ERROR );
	}
	(void) strcpy( Stanza.raname, raname );
    }
    else
	Stanza.raname = NULL;

    /* Use the last element of the path name as the name field */

    cp = strrchr( sname, DIR_SEPARATOR );
    if (cp == NULL)
	cp = strrchr (sname, '\\');
    if (cp == NULL)
	cp = sname;
    else
	cp++;

    for (bp = cp; *bp != '\0'; bp++)
	if (*bp > '9' || *bp < '0' && *bp != '*')
	    break;

    if (*bp != '\0') {   /* Got a name */
	Stanza.name = malloc( (unsigned) (strlen( cp ) + 1) );
	if (Stanza.name == NULL) {
	    free_stanza();
	    return( ERROR );
	}
	(void) strcpy( Stanza.name, cp );
    }
    else      /* Got a number */
	Stanza.name = NULL;

#ifdef MAXDEBUG
    dump_stanza();
#endif

    return( RET_OK );
}


/****************************************************************************/
/* attr_dump: debugging print out of an ATTR_t object.                      */
/****************************************************************************/

#ifdef DEBUG

void
attr_dump( ATTR_t attr )
{
    register struct ATTR *ap;

    if (attr == NULL) {
	debug( "NULL stanza passed to attr_dump" );
	return;
    }

    debug( "\"%s\"", attr[0].AT_value );

    for (ap = &attr[1]; ap->AT_name != NULL; ap++)
	debug( "\t%s = \"%s\"", ap->AT_name, ap->AT_value );
}

#endif


/****************************************************************************/
/* dump_stanza: print out a *STANZA for debugging purposes.                 */
/****************************************************************************/

#ifdef DEBUG

void
dump_stanza(void)
{
    debug( "Current Stanza" );
    debug( "\tType  = %d", Stanza.type );
    debug( "\tCount = %d", Stanza.count );
    debug( "\tFlags = %#x", Stanza.flags );
    debug( "\tValue = \"%s\"", Stanza.value );
    if (Stanza.name != NULL)
	debug( "\tName  = \"%s\"", Stanza.name );
    else
	debug( "\tName  = NULL" );
}

#endif


/****************************************************************************/
/* free_stanza: frees the malloced parts of the Stanza struct.              */
/****************************************************************************/

void
free_stanza(void)
{
#ifdef MAXDEBUG
    debug( "free_stanza: Stanza = %#o", Stanza );
    debug( "             Stanza.value = %#o", Stanza.value );
    debug( "             Stanza.name = %#o", Stanza.name );
#endif

    if (Stanza.value != NULL) {
	free( Stanza.value );
	Stanza.value = NULL;
    }

    if (Stanza.name != NULL) {
	free( Stanza.name );
	Stanza.name = NULL;
    }

    if (Stanza.raname != NULL) {
	free( Stanza.raname );
	Stanza.raname = NULL;
    }
}
