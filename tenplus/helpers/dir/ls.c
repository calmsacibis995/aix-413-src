#if !defined(lint)
static char sccsid[] = "@(#)96	1.6  src/tenplus/helpers/dir/ls.c, tenplus, tenplus411, GOLD410 3/23/93 11:56:08";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	add_stat_data, add_ls_data, is_upto_date, putfield
 * 
 * ORIGINS:  9, 10, 27
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
/* File: ls.c - deals with the ls form.                                     */
/****************************************************************************/

#include <database.h>
#include "index.h"
#include <limits.h>
#include <libobj.h>
#include "libhelp.h"
#include "hdir.h"

LOCAL int      is_upto_date( time_t , POINTER *);
extern int     havelsdata;      /* used to determine if we have ls data */

/****************************************************************************/
/* add_stat_data: add up-to-date stat information to a record.              */
/****************************************************************************/

POINTER *
add_stat_data( POINTER *record, int long_or_short )
{
    auto     struct stat    statbuf;       /* Returned by stat                */
    auto     ASTRING        file_name;     /* F_NAME field of current record  */
    auto     char           conversion_buffer[100 * MB_LEN_MAX];
    auto     struct passwd *pwbuf;
    auto     struct group  *grpbuf;
    static   char          *type_names[] = {
	"Ordinary",                   /* 00 */
	"Fifo Special",               /* 01 */
	"Character Special",          /* 02 */
	"Undefined",                  /* 03 */
	"Directory",                  /* 04 */
	"Undefined",                  /* 05 */
	"Block Special",              /* 06 */
	"Undefined",                  /* 07 */
	"Ordinary",                   /* 10 */
	"Undefined",                  /* 11 */
	"Undefined",                  /* 12 */
	"Undefined",                  /* 13 */
	"Undefined",                  /* 14 */
	"Undefined",                  /* 15 */
	"Undefined",                  /* 16 */
	"Undefined"                   /* 17 */
    };
    static   char          *modechars[] = {
	"---",     /* 0 */
	"--x",     /* 1 */
	"-w-",     /* 2 */
	"-wx",     /* 3 */
	"r--",     /* 4 */
	"r-x",     /* 5 */
	"rw-",     /* 6 */
	"rwx",     /* 7 */
    };
    static char            *file_types[] = {
	"r",          /* 00 */
	"f",          /* 01 */
	"c",          /* 02 */
	"u",          /* 03 */
	"d",          /* 04 */
	"u",          /* 05 */
	"b",          /* 06 */
	"u",          /* 07 */
	"r",          /* 10 */
	"u",          /* 11 */
	"u",          /* 12 */
	"u",          /* 13 */
	"u",          /* 14 */
	"u",          /* 15 */
	"u",          /* 16 */
	"u"           /* 17 */
    };

    extern   struct group  *getgrgid(unsigned long gid);
    extern   struct passwd *getpwuid(unsigned long uid);

#ifdef DEBUG
    debug( "add_stat_data(): record 0%o flag %d",record,long_or_short );
    s_print(record);
#endif
    /* Get the name of the file to stat */

    file_name = (auto ASTRING)s_findnode( record, F_NAME );
    if ( file_name == (char *) ERROR )
	Efatal( IFINDNAME, "Cannot find file name" );

    /* Put the raw data into the stat buffer */

    if (stat( file_name, &statbuf ) == ERROR)
	return( NULL );

    /* Check if the stat info needs to be updated */

    if (is_upto_date( statbuf.st_ctime, record ))
	return( NULL );

    /* Fill in the data from the stat buffer */

    record = putfield( record, F_T,
		    file_types[((unsigned) (statbuf.st_mode & S_IFMT)) >> 12] );
    record = putfield( record, F_Y, modechars[(statbuf.st_mode & 0700) >> 6] );
    record = putfield( record, F_G, modechars[(statbuf.st_mode & 0070) >> 3] );
    record = putfield( record, F_O, modechars[statbuf.st_mode & 0007] );

    (void) sprintf( conversion_buffer, "%7ld", statbuf.st_size );
    record = putfield( record, F_BYTES, conversion_buffer );

    pwbuf = getpwuid( (int) statbuf.st_uid );
    if (pwbuf == NULL) {
	(void) sprintf( conversion_buffer, "%d", (int) statbuf.st_uid );
	record = putfield( record, F_OWNER, conversion_buffer );
    }
    else
	record = putfield( record, F_OWNER, pwbuf->pw_name );

    record = putfield( record, F_LAST_MODIFY_TIME,
		       strdate( statbuf.st_mtime ) );

#ifdef MAXDEBUG
    debug( "add_stat_data: record = " );
    s_print( record );
#endif

    if (long_or_short == SHORT_FORM)
	return( record );

    (void) sprintf( conversion_buffer, "%d", statbuf.st_nlink );
    record = putfield( record, F_LNS, conversion_buffer );

    grpbuf = getgrgid( (int) statbuf.st_gid );
    if (grpbuf == NULL) {
	(void) sprintf( conversion_buffer, "%d", (int) statbuf.st_gid );
	record = putfield( record, F_GROUP, conversion_buffer );
    }
    else
	record = putfield( record, F_GROUP, grpbuf->gr_name );

    record = putfield( record, F_SET_UID,
		      ((statbuf.st_mode & S_ISUID) ? "s" : "-") );
    record = putfield( record, F_SET_GID,
	     ((statbuf.st_mode & S_ISGID) ? "s" : "-") );
    record = putfield( record, F_STICKY_BIT,
	     ((statbuf.st_mode & S_ISVTX) ? "t" : "-") );
    record = putfield( record, F_LAST_ACCESS_TIME,
		       strdate( statbuf.st_atime ) );
    record = putfield( record, F_STATUS_CHANGE_TIME,
		       strdate( statbuf.st_ctime ) );

    (void) sprintf( conversion_buffer, "%d, %d", major( statbuf.st_dev ),
	minor( statbuf.st_dev ) );
    record = putfield( record, F_DID_DIR, conversion_buffer );

    if ( S_ISCHR(statbuf.st_mode) || S_ISBLK(statbuf.st_mode) ) {
	/* Character special or block special devices only */
	(void) sprintf( conversion_buffer, "%d, %d", major( statbuf.st_rdev ),
	    minor( statbuf.st_rdev ) );
	record = putfield( record, F_DID_SF, conversion_buffer );
    }
    else
	record = putfield( record, F_DID_SF, "N. A." );

    record = putfield( record, F_FILE_TYPE,
	      type_names[(statbuf.st_mode & S_IFMT) >> 12] );

    (void) sprintf( conversion_buffer, "%d", statbuf.st_ino );
    record = putfield( record, F_INODE, conversion_buffer );

    (void) sprintf( conversion_buffer, "%d", statbuf.st_uid );
    record = putfield( record, F_UI, conversion_buffer );

    (void) sprintf( conversion_buffer, "%d", statbuf.st_gid );
    record = putfield( record, F_GI, conversion_buffer );

    (void) sprintf( conversion_buffer, "%ld", ((statbuf.st_size + 511) / 512) );
    record = putfield( record, F_BLOCKS, conversion_buffer );

    (void) sprintf( conversion_buffer, "%s/%s", Current_directory, file_name );
    record = putfield( record, F_FULL_PATH, conversion_buffer );

    return( record );
}


/****************************************************************************/
/* add_ls_data: adds stat information to a closed .index file.              */
/****************************************************************************/

void
add_ls_data(void)
{
    auto     SFILE         *sfp;           /* For the index file              */
    register int            i;             /* Counts records in theindex file */
    auto     POINTER       *record;        /* The data for one file           */
    auto     POINTER       *result;        /* Of call on add_stat_data        */

#ifdef DEBUG
    debug( "add_ls_data()" );
#endif

    sfp = (auto SFILE *)sf_open( Index_file_name, "u" );
    if ( sfp == (SFILE *) ERROR )
	Efatal( IOPEN, "Cannot open file \"%s\"", Index_file_name );

    Emessage( ISTATS, "Obtaining statistics for each file..." );

	record = (POINTER *) sf_read( sfp, 1 );
	if (record == (POINTER *) ERROR)
	    Efatal( IREADFILE, "Cannot read file \"%s\"", Index_file_name );

    for (i = 0; i < obj_count( record ); i++) {

	if (record[i] != NULL) {

	    /* Ignore lines with no file name. */

	    if ( s_findnode( (POINTER *)record[i], F_NAME ) == (char *) ERROR )
		continue;

	    result = add_stat_data( (POINTER *)record[i], SHORT_FORM );
	    if (result != NULL) record[i] = (POINTER) result;
	}
    }

    if (sf_write((SFILE *) sfp, 1, (char *)record ) == ERROR)
	Efatal( IWRITEFILE, "Cannot write file \"%s\"", Index_file_name );

    s_free((char *) record );

    if (sf_close( sfp ) == ERROR)
	Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);

    havelsdata = TRUE;     /* we can zoomout from the stat form */
}


/****************************************************************************/
/* is_upto_date: TRUE if the data in the record is upto data with the stat  */
/*   time.                                                                  */
/****************************************************************************/

LOCAL int
is_upto_date( time_t last_stat_time, POINTER *record )
{
    auto struct tm *atm;
    auto char      *time_field;      /* F_STATUS_CHANGE_TIME field */
    auto time_t     time_from_field;

#ifdef MAXDEBUG
    debug( "is_upto_date()" );
#endif

    time_field = (auto char *)s_findnode( record, F_STATUS_CHANGE_TIME );
    if (time_field == (char *) ERROR)
	return( FALSE );

    atm = parsedate( time_field );
    if (atm == (struct tm *) ERROR)
	return( FALSE );

    time_from_field = mktime( atm );

    return( last_stat_time <= time_from_field );
}


/****************************************************************************/
/* putfield: add or replace a field in a record.                            */
/****************************************************************************/

POINTER *
putfield( POINTER *record, char *field_name, char *value )
{
    auto ASTRING field;
    auto POINTER field_pointer;

#ifdef MAXDEBUG
    debug( "put_field( record = %#X, field_name = \"%s\", value = \"%s\" )",
	   record, field_name, value );
#endif

    field = (auto ASTRING)s_string( value );
    s_newname( field, field_name );
    field_pointer = (auto POINTER)s_findnode( record, field_name );
    if (field_pointer != (char *) ERROR) {
	(void) s_fixtree( record, field_pointer, field, TRUE );
	s_free( field_pointer );
    }
    else
	record = (POINTER *) s_append( record, field );

    return( record );
}
