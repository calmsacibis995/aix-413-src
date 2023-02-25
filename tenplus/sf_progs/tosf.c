static char sccsid[] = "@(#)19	1.7  src/tenplus/sf_progs/tosf.c, tenplus, tenplus411, GOLD410 11/1/93 16:36:22";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, tosf, make_attribute_file, make_object,
 *		fix_record_array, sfname
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: tosf.c - create a structured file from its attribute file          */
/*   representation.                                                        */
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>


/* Ined Includes */
#include <database.h>
#include "af.h"
#include <libobj.h>

#define ATTRIBUTE_FILE "attribute_file"

#ifndef DEBUG
#define LOCAL static
#else
#define LOCAL
#endif


LOCAL int      fix_record_array(SFILE *);
LOCAL char    *make_attribute_file(void);
LOCAL POINTER *make_object( FILE *, char *);
LOCAL void     tosf(char *, FILE *, char *);
LOCAL char    *sfname(char *);
LOCAL char    *find_define(char *, FILE *, char *);
LOCAL char    *get_number(char *);

int MultibyteCodeSet;


/****************************************************************************/
/* main: argument processing and driver function.                           */
/****************************************************************************/


main(int argc, char **argv )
{
    auto char *attribute_file;
    auto char *header_file=NULL;
    FILE *fp=NULL;

    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    

#ifdef DEBUG
    g_debugfp = fopen( "tosf.out", "w" );
    if (g_debugfp == NULL) {
	(void) fprintf( stderr, "Unable to open debugging output file\n" );
	exit( -1 );
    }

#endif

    if (argc > 3) {
	(void) fprintf( stderr, "Usage: tosf [attribute_file][header_file]\n" );
	exit( -1 );
    }

    if (argc == 1) {
	attribute_file = make_attribute_file();
	if (attribute_file == NULL) {
	    (void) fprintf( stderr, "Unable to create disk copy of attribute file\n" );
	    exit( -1 );
	}
    }
    else
    {
	if (argc >= 2)
        {
	   attribute_file = argv[1];
           if (strstr(attribute_file,"_msg.h")!=NULL)
	   {
	       (void) fprintf( stderr, "Usage: tosf [attribute_file][header_file]\n" );
	       exit( -1 );
	   }
        }
	if (argc == 3)
	{
	   header_file = argv[2];
           if (strstr(header_file,"_msg.h")==NULL)
	   {
	       (void) fprintf( stderr, "Usage: tosf [attribute_file][header_file]\n" );
	       exit( -1 );
	   }

	   if ((fp = fopen(header_file,"r"))==NULL)
	   {
	    (void) fprintf( stderr, "tosf: Unable to open header file '%s'",header_file);
	    exit( -1 );
	      
	   }
        }
    }

    tosf( attribute_file,fp,header_file);

    free_stanza();

    if (fp != NULL)
	fclose(fp);

    if (argc == 1)
	(void) unlink( ATTRIBUTE_FILE );

    exit( 0 );
    return(0); /* return put here to make lint happy */
}


/****************************************************************************/
/* tosf: produces a structured file from a previously fromsf'ed attribute   */
/*   file.                                                                  */
/****************************************************************************/

LOCAL void
tosf(char * attribute_file , FILE *fp, char *header_file)
    /* attribute_file:                     Name of the attribute file        */
    /* fp                                  File pointer for header file      */
    /* header_file                         Name of the header file           */

{
    auto     char  *structured_file;    /* Name of the structured file       */
    auto     SFILE *sfp;                /* SFILE pointer for structured file */
    register int    recno;              /* Counts records in structured file */

#ifdef DEBUG
    debug( "tosf( attribute_file = \"%s\" )", attribute_file );
#endif

    /* Obtain name of the structured file */

    structured_file = sfname( attribute_file );
    if (structured_file == NULL) { /* If there is no file name, use newfile */
	auto char cmdbuf[100];

	(void) sprintf( cmdbuf, "/usr/bin/newfile %s", attribute_file );
	if (system( cmdbuf ) != 0)
	    (void) fprintf( stderr, "Unable to convert file \"%s\"\n",
			    attribute_file );
	return;
    }
    else if (structured_file == (char *) ERROR) {
	(void) fprintf( stderr, "Unable to open attribute file \"%s\"\n",
			attribute_file );
	return;
    }

#ifdef DEBUG
    debug( "tosf: structured_file = \"%s\"", structured_file );
#endif

    /* Create structured file */

    sfp = sf_open( structured_file, "c" );
    if (sfp == (SFILE *) ERROR) {
	(void) fprintf( stderr, "Unable to open structured file \"%s\"\n",
		 structured_file );
	return;
    }

    /* Open the file as an attribute file */

    Afp = AFopen( attribute_file, 1024, 100 );
    if (Afp == NULL) {
	(void) fprintf( stderr, "Unable to open attribute file \"%s\"\n",
			attribute_file );
	(void) sf_close( sfp );
	return;
    }

    /* Adjust the record array, if necessary */

    if (!fix_record_array( sfp )) {
	(void) fprintf( stderr,
		 "Unable to read data from attribute file \"%s\"\n",
		 attribute_file );
	AFclose( Afp );
	(void) sf_close( sfp );
	return;
    }

    /* Process each stanza */

    for (recno = 0; recno < obj_count( sf_records( sfp ) ); recno++) {
	POINTER *record;

	record = make_object(fp,header_file);

	if (record == (POINTER *) ERROR) {
	    (void) fprintf( stderr, "Bad data in attribute file \"%s\"\n",
			    attribute_file );
	    break;
	}

#ifdef DEBUG
	debug( "tosf: high-level call to make_object returns %#x", record );
	if (record != NULL)
	    s_print( (char *) record );
#endif

	if ((sf_write( sfp, recno, (char *)record )) == ERROR) {
	    (void) fprintf( stderr,
		     "I/O error writing record to structured file \"%s\"\n",
		     structured_file );
	    s_free((char *) record );
	    break;
	}

	if (record != NULL)
	    s_free((char *) record );
    }

    /* Close files and return */

    if (sf_close( sfp ) == ERROR)
	(void) fprintf( stderr, "Unable to close structured file \"%s\"\n",
		 structured_file );

    AFclose( Afp );
}


/****************************************************************************/
/* make_attribute_file: make an actual disk file containing the contents    */
/*   of stdin.  This is done to avoid changing the contents of the          */
/*   attribute file routines.                                               */
/****************************************************************************/

LOCAL char *
make_attribute_file(void)
{
    auto FILE *fp;

#ifdef DEBUG
    debug( "make_attribute_file()" );
#endif

    /* Create the attribute file */

    fp = fopen( ATTRIBUTE_FILE, "w" );
    if (fp == NULL)
	return( NULL );

    while (feof( stdin ) == FALSE) {
	static char buffer[1000];

	(void) gets( buffer );

	if (ferror( stdin ) != 0) {
	    (void) fclose( fp );
	    return( NULL );
	}

	(void) fprintf( fp, "%s\n", buffer );
    }

    if (fclose( fp ) == EOF)
	return( NULL );

    return( ATTRIBUTE_FILE );
}


/****************************************************************************/
/* make_object: make an object based upon the current contents of Stanza    */
/*   and the remainder of the attribute file.  Always leave Stanza          */
/*   containing an unused stanza.  Returns ERROR on bad data.  Returns NULL */
/*   for a NULL object.                                                     */
/****************************************************************************/

LOCAL POINTER *
make_object(FILE * fp,char *header_file)
{
    auto   POINTER *object;        /* The object being constructed */
    char  *new_obj;                /* Value of the new object      */

    if (next_stanza() != RET_OK)
	return( (POINTER *) ERROR );

    if (Stanza.type == '\0')   /* For NULL objects */
	return( NULL );

    if ((Stanza.type != T_CHAR) && (Stanza.type != T_POINTER))
	return( (POINTER *) ERROR );

#ifdef DEBUG
    debug( "make_object: called to process Stanza" );
    dump_stanza();
#endif

    object = (POINTER *) s_alloc( Stanza.count, Stanza.type, Stanza.name );

    if (Stanza.flags != 0)
	obj_setflag( object, Stanza.flags );

#ifdef MAXDEBUG
    debug( "make_object: new object allocated: %#x", object );
    s_print( object );
#endif

    if (Stanza.type == T_CHAR)             /* (3) */
    {
	(void) strcpy( (char *) object, Stanza.value );

	if (fp != NULL)
	{
	    if (*(Stanza.value) == '%')
	    { /* Get the mnemonic */
		new_obj = find_define(Stanza.value,fp,header_file);
#ifdef DEBUG
	debug("tosf: mnemonic %s has value %s",Stanza.value,new_obj);
#endif

		if (strstr(Stanza.value,"%+") != NULL)
		    /* Leave the %+ in the file */
	            (void) strcpy(((char *) object)+2, new_obj );
		else
		    /* Remove the %- from the file */
	            (void) strcpy((char *) object, new_obj );
	    }
	}
    }
    else if (Stanza.type == T_POINTER) {   /* (1) */
	register int      i;
	register int      count;

	/*
	 * Stanza.count changes during the course of reading the
	 * attribute file.  By the following loop should be controlled
	 * by the the Stanza.count value that existed when it first
	 * started executing.  Thus, a stack variable must be allocated
	 * to hold the value.
	 */

	count = Stanza.count;

	for (i = 0; i < count; i++) {
	    auto POINTER *ans;

	    ans = make_object(fp,header_file);

#ifdef MAXDEBUG
	    debug( "make_object: recursive call (position %d) returns %#x",
		   i, ans );
	    if (ans != NULL)
		s_print( ans );
#endif

	    if (ans == (POINTER *) ERROR)
		return( ans );

	    object[i] = (POINTER) ans;
	}
    }
    else              /* Should never get here */
	return( (POINTER *) ERROR );

    return( object );
}


/****************************************************************************/
/* fix_record_array: correct the record array based on data from the        */
/*   attribute file.  Return FALSE on any error, else TRUE.                 */
/*   Makes sure the next stanza has been read in.                           */
/****************************************************************************/

LOCAL int
fix_record_array(SFILE * sfp )
    /* sfp:             SFILE pointer for structured file */
{
    /* Obtain the record array information */

    if (next_stanza() != RET_OK)
	return( FALSE );

#ifdef DEBUG
    debug( "fix_record_array" );
    dump_stanza();
#endif

    if (Stanza.raname != NULL)
	s_newname((char *) sf_records( sfp ), Stanza.raname );

    if (Stanza.flags != 0)
	obj_setflag( sf_records( sfp ), Stanza.flags );

    if (Stanza.count != 0)
	sfp->sf__records = (RECORD *) s_realloc( (char *) sf_records( sfp ),
						 Stanza.count );

#ifdef DEBUG
    debug( "fix_record_array: record array = %#x", sf_records( sfp ) );
    s_print( (char *) sf_records( sfp ) );
#endif

    return( TRUE );
}


/****************************************************************************/
/* sfname: read the name of the structured file from the first line of      */
/*   the attribute file.  Return NULL if it is not there.  Return ERROR     */
/*   if attribute file could not be opened.                                 */
/****************************************************************************/

LOCAL char *
sfname(char * file_name )
    /* file_name:                        Name of the attribute file      */
{
    auto   FILE *fp;                  /* FILE pointer for attribute file */
    static char  structured_file[80]; /* Name of the attribute file      */
    auto   int   anscount;            /* Result of scanf call            */

#ifdef MAXDEBUG
    debug( "sfname( file_name = \"%s\" )", file_name );
#endif

    /* Use stdio to read the first line of the file */

    fp = fopen( file_name, "r" );
    if (fp == NULL)
	return( (char *) ERROR );

    anscount = fscanf( fp, "* File name: %s", structured_file );

    (void) fclose( fp );

    if (anscount == 1)
	return( structured_file );
    else
	return( NULL );
}

/*************************************************************************
* find_define: Positions the file pointer to the MS_FORMS set then       *
*              searches for the mnemonic in the header file,  finds the  *
*              #define for the mnemonic and puts it in the file          *
*************************************************************************/
LOCAL char *find_define(char *ptr, FILE *fp,char *header_file)
{
static char string[81];
char *srch_ptr;
char *mnem = NULL;

#ifdef DEBUG
	debug("tosf: Searching for '%s'",ptr);
#endif

    if (fp != NULL)
        fclose (fp);

    if (header_file != NULL)
        fp = fopen(header_file, "r");

    while ((srch_ptr =fgets(string,80,fp)) != NULL)
    {
        if ((srch_ptr=strstr(string,"MS_FORMS")) != NULL)
        {
            break;
        }
    }

    if (srch_ptr == NULL)
    {
	(void) fprintf( stderr, "tosf: Requires MS_FORMS definition\n" );
	exit( -1 );
    }

    while ((srch_ptr =fgets(string,80,fp)) != NULL)
    {
	   if ((srch_ptr=strstr(srch_ptr,ptr+2)) != NULL)
           {
               mnem = get_number(string);
               break;
           }
    }


#ifdef DEBUG
	debug("tosf: mnemonic found in the header file '%s'",mnem);
#endif

    return(mnem);

}
/**************************************************************************
* get_number:searches along the string for any digit.  The digits found   *
*            are what we are looking for.  Otherwise the message catalog  *
*            file has a problem.                                          *
**************************************************************************/
LOCAL char *get_number(char *srch_ptr)
{
char *mnem_ptr;


#ifdef DEBUG
	debug("tosf: mnemonic found in this line '%s'",srch_ptr);
#endif

   while (!isspace(*srch_ptr)) /* Move to the #define */
	srch_ptr++;

   while (isspace(*srch_ptr))
	srch_ptr++;

   while (!isspace(*srch_ptr)) /* Move to the start of the menmonic */
	srch_ptr++;
     
   while (!isdigit(*srch_ptr) && srch_ptr !=NULL) 
	srch_ptr++;

   if (srch_ptr == NULL)
   {
        (void) fprintf( stderr, "tosf: Error in Message catalog header\n" );
        exit( -1 );
   }

   mnem_ptr = srch_ptr;

   while (isdigit(*srch_ptr))
      srch_ptr++;

   *srch_ptr = '\0';

   return(mnem_ptr);
}
