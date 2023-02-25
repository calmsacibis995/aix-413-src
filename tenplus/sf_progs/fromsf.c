#if !defined(lint)
static char sccsid[] = "@(#)09	1.5  src/tenplus/sf_progs/fromsf.c, tenplus, tenplus411, GOLD410 7/18/91 13:52:49";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, fromsf, basename, printobj
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: fromsf.c - create an attribute file representation for a           */
/*   structured file.                                                       */
/****************************************************************************/


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>


/* Ined Includes */
#include <chardefs.h>
#include <database.h>
#include <libobj.h>

int MultibyteCodeSet;

static char *basename(char *);
static void  printobj(char *, POINTER *, POINTER *, FILE *);
static void  fromsf(char *, char *);

/****************************************************************************/
/* main: argument processing and driver function.                           */
/****************************************************************************/

int main(int  argc, char **argv )
{
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;

#ifdef DEBUG

    g_debugfp = fopen( "fromsf.out", "w" );
    if (g_debugfp == NULL) {
	(void) fprintf( stderr, "Unable to open debugging output file\n" );
	exit( -1 );
    }

#endif
    
    if (argc < 2 || argc > 3) {
	(void) fprintf( stderr,
			"Usage: fromsf structured_file [ascii_file]\n" );
	exit( -1 );
    }

    /* If no second argument is given, stdout will be used */

    fromsf( argv[1], (argc == 3) ? argv[2] : NULL );

    exit( 0 );
    return(0); /* return put here to make lint happy */
}


/****************************************************************************/
/* fromsf: produces an attribute file from its structured file argument.    */
/****************************************************************************/

static void
fromsf(char * structured_file, char *ascii_file )
    /* structured_file:              Name of the structured file           */
    /* ascii_file:                   Name of the attribute file            */
{
    auto     SFILE *sfp;          /* SFILE pointer for the structured file */
    auto     FILE  *fp;           /* FILE pointer for the attribute file   */
    register int    i;            /* Counts records in the structured file */

#ifdef DEBUG
    debug( "fromsf( structured_file = \"%s\", ascii_file = \"%s\" )",
	    structured_file, (ascii_file == NULL) ? "NULL" : ascii_file );
#endif

    /* Open structured file  */

    sfp = sf_open( structured_file, "r" );
    if (sfp == (SFILE *) ERROR) {
	(void) fprintf( stderr, "File \"%s\" is not a structured file\n",
		 structured_file );
	return;
    }

#ifdef DEBUG
    debug( "fromsf: SFILE object = %#x", sfp );
    s_print( sfp );
#endif

    /* Check for structured text files.  If there, just use readfile */

/*  if (NOT obj_getflag( sf_records( sfp ), RC_NONTEXT )) {
	auto char cmdbuf[100];

	(void) sf_close( sfp );

	if (ascii_file != NULL)
	    (void) sprintf( cmdbuf, "readfile %s > %s", structured_file,
			    ascii_file );
	else
	    (void) sprintf( cmdbuf, "readfile %s", structured_file );

	if (system( cmdbuf ) != 0)
	    (void) fprintf( stderr, "Unable to convert file \"%s\"\n",
		     structured_file );

	return;
    } */

    /* Backup and create attribute file */

    if (ascii_file != NULL) {
	(void) fbackup( ascii_file );
	fp = fopen( ascii_file, "w" );
	if (fp == NULL) {
	    (void) fprintf( stderr, "Unable to open attribute file \"%s\"\n",
			     ascii_file );
	    (void) sf_close( sfp );
	    return;
	}
    }
    else {
	fp = fdopen( fileno( stdout ), "w" );
	if (fp == NULL) {
	    (void) fprintf( stderr, "Fdopen failure\n" );
	    (void) sf_close( sfp );
	    return;
	}
    }

    /* Convert structured file to attribute file */

    (void) fprintf( fp, "* File name: %s\n\n", basename( structured_file ) );

    /* Insert sccs string */
    /* We have to do a little work keep sccs from changing these keywords
       when we admin/delta this file. */
    {
	char pc = '%';
	(void) fprintf( fp, "* %cW%c - %cE%c\n\n",pc,pc,pc,pc);
    }
    /* Print out default stanza */

    (void) fprintf( fp, "default:\n" );
    (void) fprintf( fp, "\tType = %d\n", T_CHAR );
    (void) fprintf( fp, "\tFlags = 0\n" );
    (void) fprintf( fp, "\tValue = \"\"\n\n" );

    /* Dump the record array information */

    (void) fprintf( fp, "Record_array:\n" );
    if (obj_name( sf_records( sfp ) ) != NULL)
	(void) fprintf( fp, "\tName = \"%s\"\n",
			obj_name( sf_records( sfp ) ) );
    (void) fprintf( fp, "\tType = %d\n", obj_type( sf_records( sfp ) ) );
    if (obj_count( sf_records( sfp ) ) != 0)
	(void) fprintf( fp, "\tCount = %d\n",
			obj_count( sf_records( sfp ) ) );
    if (obj_flag( sf_records( sfp ) ) != '\0')
	(void) fprintf( fp, "\tFlags = %#x\n",
			obj_flag( sf_records( sfp ) ) );
    (void) fprintf( fp, "\n" );

    /* Loop through the records of the file */

    for (i = 0; i < obj_count( sf_records( sfp ) ); i++) {
	auto POINTER *record;       /* The record being converted */
	auto char     recname[100]; /* Name of the record, if any */

	record = (POINTER *) sf_read( sfp, i );
	if (record == (POINTER *) ERROR) {
	    (void) fprintf( stderr, "I/O error reading file \"%s\"\n",
		     structured_file );
	    break;
	}

	if (record == NULL) {
	    (void) fprintf( fp, "/*:\n" );
	    (void) fprintf( fp, "\tType = 0\n" );
	    (void) fprintf( fp, "\n" );
	    continue;
	}

#ifdef DEBUG
	debug( "record[%d]: ", i );
	s_print( record );
#endif

	if (obj_name( record ) != NULL)
	    (void) strcpy( recname, obj_name( record ) );
	else
	    (void) strcpy( recname, "*" );

	printobj( recname, record, record, fp );

	s_free((char *) record );
    }

    /* Close files and return */

    (void) sf_close( sfp );

    if (fclose( fp ) == EOF)
	(void) fprintf( stderr, "Unable to close attribute file \"%s\"\n",
		 ascii_file );

    return;
}


/****************************************************************************/
/* basename: returns a pointer into the argument string indicating the      */
/*   name of the file in the current directory.  That is, it strips off     */
/*   any directory path prefix.                                             */
/****************************************************************************/

static char *
basename(char * file_name )
{
    register char *bp;              /* Points into the argument string */
    bp = strrchr( file_name, DIR_SEPARATOR );

    if (bp == NULL)
	bp = file_name;
    else
	bp += 1;

    return( bp );
}


/****************************************************************************/
/* printobj: recursive object printing function.                            */
/****************************************************************************/

static void
printobj(char * recname, POINTER *record, POINTER *object, FILE *fp )
{
    auto     char *pathname;    /* The stanza name                    */
    register int   i;           /* Counts pointers in a pointer block */

#ifdef DEBUG
    debug(
	"printobj( recname = \"%s\", record = %#x, object = %#x, fp = %#x )",
	recname, record, object, fp );
    if (object != NULL)
	s_print( object );
#endif

    pathname = s_pathname( record, (char *)object );
    if (pathname == (char *) ERROR) {
	(void) fprintf( stderr,
			"Cannot construct stanza entry for object\n" );
	return;
    }
    if ((strcmp(pathname, "/" ))==0)
	pathname[0] = '\0';

#ifdef DEBUG
    debug( "printobj: pathname = \"%s\"", pathname );
#endif

    (void) fprintf( fp, "/%s%s:\n", recname, pathname );

    if (obj_type( object ) != T_CHAR)
	(void) fprintf( fp, "\tType = %d\n", obj_type( object ) );
    if (obj_flag( object ) != 0)
	(void) fprintf( fp, "\tFlags = %#x\n", obj_flag( object ) );

    switch (obj_type( object )) {

	case T_POINTER:

	    (void) fprintf( fp, "\tCount = %d\n", obj_count( object ) );
	    (void) fprintf( fp, "\n" );        /* End the parent stanza */

	    for (i = 0; i < obj_count( object ); i++)
		if (object[i] != NULL)
		    printobj( recname, record, (POINTER *) object[i], fp );
		else {
		    (void) fprintf( fp, "/%s%s/%d:\n", recname, pathname, i );
		    (void) fprintf( fp, "\tType = 0\n" );
		    (void) fprintf( fp, "\n" );
		}
	    break;

	case T_CHAR:

	    if (obj_count( object ) != 0 && object[0] != '\0') {
		register char *cp;

		(void) fputs( "\tValue = \"", fp );

		/*
		 *  Quotation marks must be escaped so that they will
		 *    be treated as data rather than a delimiter by the
		 *    attribute file parsing routines.  Backslashes
		 *    must also be escaped.
		 */

		for (cp = (char *) object; *cp != '\0'; cp++) {

		    if (iscntrl( *cp ))
			(void) fprintf( fp, "\\%3.3o", *cp );
		    else if (*cp == '"')
			(void) fputs( "\\\"", fp );
		    else if (*cp == '\\')
			(void) fputs( "\\\\", fp );
		    else
			(void) fputc( *cp, fp );
		}

		(void) fputs( "\"\n", fp );
	    }
	    (void) fputc( '\n', fp );
	    break;

	default:
	    (void) fprintf( stderr, "Unable to print object value\n" );
	    (void) fputc( '\n', fp );
	    break;
    }

    s_free( pathname );
}
