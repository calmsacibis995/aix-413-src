#if !defined(lint)
static char sccsid[] = "@(#)76	1.9  src/tenplus/helpers/dir/uinit.c, tenplus, tenplus411, GOLD410 3/23/93 11:56:58";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	I_hinit, I_hsavestate, I_hrestart, I_hstop, cmpnames,
 *		cmprecords, create_on_tmp, get_directory,
 *		get_index_file, is_reusable, is_visible, on_death,
 *		read_directory, read_profile, update_index_file
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
/* File: uinit.c - create or update the index file.                         */
/****************************************************************************/

#include <limits.h>
#include <database.h>
#include "index.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libobj.h>
#include "libhelp.h"
#include <libprf.h>
#include "hdir.h"
#include <libutil.h>

static POINTER *Hidden_patterns;         /* File not to be displayed      */
static int      Synchronize;


#ifdef DEBUG
#include <signal.h>
LOCAL int      on_death();
#endif

LOCAL int      cmpnames(char *, POINTER *);
LOCAL int      cmprecords(POINTER *, POINTER *);
LOCAL SFILE   *create_on_tmp(void);
LOCAL ASTRING  get_directory(ASTRING);
LOCAL int      is_reusable(void);
LOCAL int      is_visible(char *);
LOCAL POINTER *read_directory(void);
LOCAL void     read_profile(void);
LOCAL void     update_index_file(SFILE *);
LOCAL time_t   g_dirmoddate;

extern int havelsdata;     /* have we gotten ls data for this directory */

static   char     before[] = {       /* Arg to Esethook               */
    U1, U2, U3, U4, U5, LOCAL_MENU, UMOD, ROFIELD, PICK_COPY, PUT_DOWN, PICK_UP,
    PUT_COPY, UINS, ZOOM_IN, ZOOM_OUT, DELETE, RESTORE, 0
};

static   char     after[] = {        /* Arg to Esethook               */
    PUT_DOWN, PUT_COPY, RESTORE, NEXT, PREVIOUS, 0
};


/****************************************************************************/
/* I_hinit: called when a directory is being edited.                        */
/****************************************************************************/

int I_hinit( register ASTRING directory )
/* Full path name of directory   */
{
    static   int      first_time = TRUE; /* TRUE on first I_hinit only    */

#ifdef DEBUG
    if (first_time && (g_debugfp != NULL)) {
	(void)fclose( stderr );   /* Make g_debugfp and stderr go to output file */
	g_debugfp = fopen( "dir.trc", "w" );
	if (g_debugfp == NULL) {
	    g_debugfp = fopen( "/dev/null", "w" );
	    Eerror( INODEBUG, "Unable to write debugging output into .po" );
	}
	else {
	    (void)chmod ( ".po", 0666 );
	     }

	g_setflag( G_COREDUMP );

	debug( ".........::::::Index Helper Start:::::::........." );
	debug( "I_hinit( directory = \"%s\" )", directory );
	(void) signal( SIGHUP, on_death );
	(void) signal( SIGPIPE, on_death );
    }
#endif

    DTinit ();

    if (Pinit( "index" ) == ERROR)
	Efatal( INOPRF,"Unable to find index helper profile file" );

    read_profile();

    Display_state = VISIBLE_FILES;

    Form_type = INDEX_FORM;

    unstat( (char *) NULL );

    if (!seq( Current_directory, directory )) {
	s_free( Current_directory );
	Current_directory = get_directory( directory );
    }
    Writable_directory = is_writable( Current_directory );

#ifdef DEBUG
    debug( "uinit: Current_directory = \"%s\"", Current_directory );
    debug( "uinit: old Index_file_name = \"%s\"", Index_file_name );
#endif

    if (!first_time && chdir( Current_directory ) == ERROR)
	Efatal( INOCHDIRF, "Cannot change to directory \"%s\"",
	Current_directory );

    first_time = FALSE;

    s_free( Index_file_name );
    Index_file_name = pathcat( s_string( Current_directory ),
			       s_string( INDEX_NAME ) );

    /* First check to see if the old .index file can be reused */

    havelsdata = FALSE;
    if (!is_reusable())
	get_index_file();

    if (Eusefile( Index_file_name ) == ERROR)
	Efatal( IOPEN, "Cannot open file \"%s\"", Index_file_name);

    /* Use the correct form */

    if (Writable_directory) {
	if (Euseform( "index" ) == ERROR)
	    Efatal( IFORM, "I_hinit: cannot find index form" );
    }
    else
	if (Euseform( "tindex" ) == ERROR)
	    Efatal( IFORM, "I_hinit: cannot find index form" );

    Esethook( before, after );

    Edispname( Current_directory );

    return( RET_OK );
}

/****************************************************************************/
/* I_hsavestate: called when the user is leaving the current environment.   */
/*                                                                          */
/* We save information about what the user was doing so that we may put     */
/* everything back the way it was. The things we need to save are:          */
/* The form he was using                                                    */
/* The directory he was looking at                                          */
/* The name of the .index file                                              */
/* If the directory was writeable (and hence the form type)                 */
/* The display state (visable files or all files)                           */
/****************************************************************************/

char *I_hsavestate(void)
{
    POINTER *thelist;   /* the list of things we are using     */
    char tmpbuf[100];   /* a place to put chars until we allocate */

#ifdef DEBUG
    debug( "I_hsavestate()" );
#endif

    /* first we need the tree to hang the info we need in */
    thelist = (POINTER *) s_alloc(NUMTOSAVE,T_POINTER,"save state tree");

    /* first piece of info is the form type */
    (void) sprintf(tmpbuf,"%d",Form_type);
    thelist[0] = s_string( tmpbuf );
    s_newname(thelist[0],"Form_type");

    /* next we need the directory name */
    thelist[1] = s_string( Current_directory );
    s_newname(thelist[1],"Current_directory");

    /* now we need the name of the .index file */
    thelist[2] = s_string( Index_file_name );
    s_newname(thelist[2],"Index_file_name");

    /* next is whether the directory was writeable */
    thelist[3] = s_string( Writable_directory ? "y" : "n" );
    s_newname( thelist[3] , "Writable_Directory" );

    /* next is the display state, visable or all files */
    thelist[4] = s_string( Display_state == VISIBLE_FILES ? "v" : "a" );
    s_newname( thelist[4] , "Display_state" );

    /* next we save whether we have uptodate ls data  */
    thelist[5] = s_string( havelsdata ? "y" : "n" );
    s_newname( thelist[5] , "havelsdata" );


#ifdef DEBUG
    debug( "The save state tree:" );
    s_print( thelist );
#endif

    return( (char *) thelist );
}


/****************************************************************************/
/* I_hrestart: called when the user is reenters an old environment.         */
/*                                                                          */
/* Here we put the things that were saved by I_hsavestate back into the     */
/* variables.                                                               */
/****************************************************************************/

int I_hrestart(char *state)
{
    POINTER *thistree;

#ifdef DEBUG
    debug( "I_hrestart()" );
    debug( "The restart tree:" );
    s_print( state );
#endif

    thistree = (POINTER *) state;

    if (obj_count( thistree ) != NUMTOSAVE)
	Efatal(IBADSTVEC,"bad state vector given to I_hrestart");

    /* first we remember what form we were using */
    Form_type = atoi(thistree[0]);
    if (Form_type != INDEX_FORM && Form_type != LS_FORM &&
       Form_type != STAT_FORM)
	  Efatal(IBADFORMTP,"Bad form type in saved state tree");

    /* now we pick up the current directory */
    s_free( Current_directory );
    Current_directory = s_string( thistree[1] );

    /* now we pick up the index file name */
    s_free( Index_file_name );
    Index_file_name = s_string( thistree[2] );

    /* now we remember if the directory was writable */
    Writable_directory = seq( thistree[3], "y" ) ? TRUE : FALSE;

    /* now we remember if the display state */
    Display_state = seq( thistree[4], "v" ) ? VISIBLE_FILES : ALL_FILES;

    /* now we remember if we had ls data */
    havelsdata = seq( thistree[5], "y" ) ? TRUE : FALSE;

    unstat( (char *) NULL );   /* throw out stat buffer contents */

    if (Pinit( "index" ) == ERROR)   /* grab stuff from the profile file */
	Efatal( INOPRF,"Unable to find index helper profile file" );

    read_profile();            /* reread the profile file */

    if (chdir( Current_directory ) == ERROR)
	Efatal( INOCHDIRF, "Cannot change to directory \"%s\"",
	Current_directory );

    if (Eusefile( Index_file_name ) == ERROR)
	Efatal( IOPEN, "Cannot open file \"%s\"", Index_file_name);

    Edispname( Current_directory );

    Esethook( before, after );

    return( RET_OK );
}


/****************************************************************************/
/* I_hstop: called when the helper is about to be shut down.                */
/****************************************************************************/

void I_hstop(void)
{

#ifdef DEBUG
    debug( "I_hstop()" );
#endif

    Pcleanup();
#ifdef DEBUG
    on_death();
#endif
    exit( 0 );
}

/****************************************************************************/
/* cmpnames: used by bsearch to look up a file in a directory list.         */
/****************************************************************************/

LOCAL int
cmpnames( char *a, POINTER *b )
{
    return( strcmp( a, *b ) );
}

/****************************************************************************/
/* cmprecords: used by qsort to compare two records.                        */
/****************************************************************************/

LOCAL int
cmprecords( POINTER *first_record, POINTER *second_record ) 
{
    return( strcmp( *first_record, *second_record ) );
}

/****************************************************************************/
/* create_on_tmp: create a new .index file on /tmp.  Responsible for        */
/*   merging in any comments.  Tries to reuse any previous .index file.     */
/****************************************************************************/

LOCAL SFILE *
create_on_tmp(void)
{
    register SFILE    *sfp;              /* Index file stream                */
    auto     ASTRING   old_index_name;   /* Name of any previous .index file */
    static   POINTER **tmp_list;         /* List of .index files on /tmp     */
    register int       i;                /* Indexes through tmp_list         */
    auto     POINTER  *new_entry;        /* Entry to add to tmp_list         */

#ifdef DEBUG
    debug( "create_on_tmp()" );
#endif

    /* Create a .index on /tmp */

    if (tmp_list == NULL)
	tmp_list = (POINTER **) s_alloc( 0, T_POINTER, "tmp_list" );

    /* Look to see if a tmp file already exists */

#ifdef DEBUG
    debug( "create_on_tmp: searching for entry" );
    debug( "    Current_directory = \"%s\"", Current_directory );
    s_print( tmp_list );
#endif

    for (i = 0; i < obj_count( tmp_list ); i++)
	if (seq( tmp_list[i][0], Current_directory ))
	    break;

    if (i < obj_count( tmp_list )) { /* Directory has an .index on /tmp */
	auto char *file;      /* .index part of /tmp_list[i] */

	file = tmp_list[i][1];
	sfp = sf_open( file, "u" );    /* Reuse the old one */
	if (sfp != (SFILE *) ERROR) {
	    s_free( Index_file_name );
	    Index_file_name = s_string( file );
	    unstat( NULL );

#ifdef DEBUG
	    debug( "create_on_tmp: reusing old entry" );
	    debug( "    Index_file_name = \"%s\"", Index_file_name );
#endif

	    return( sfp );
	}
    }

    /* A new .index must be created on /tmp */

    old_index_name = Index_file_name;

    /* create a name, note that some systems may not like this name */
    Index_file_name = s_string( uniqname( TRUE ) );
    Index_file_name = s_cat( Index_file_name, s_string( INDEX_NAME ));

    /* Now copy any comments from the old .index file */

    copyfile( old_index_name, Index_file_name);

    {           /* make sure that .index file on tmp is writable */
	ushort mode;
	mode = ST_MODE(Index_file_name);
	if (!(mode & S_IWRITE))
	{
	    mode |= S_IWRITE;
	    if (chmod( Index_file_name, (int) mode ) != 0)
	    {
		char s [32];
		(void) sprintf (s, "%o", (int) mode);
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", s );
	    }
	}
    }

    sfp = sf_open( Index_file_name, "u" );
    if (sfp == (SFILE *) ERROR) {

	/* Fix a broken .index file */
	if (g_errno == E_FILE || g_errno == E_ARRAY) {

	    Emessage(IBROKEN, "Attempting to reconstruct a broken .index file..." );

	    ghost( Index_file_name );
	    sfp = sf_open( Index_file_name, "u" );
	}

	/* try again */
	if (sfp == (SFILE *) ERROR)
	    Efatal( IOPEN, "Cannot open file \"%s\" ", Index_file_name);
    }

    /* Put the new .index file into tmp_list */

    new_entry = (POINTER *) s_alloc( 2, T_POINTER, "new_entry" );
    new_entry[0] = s_string( Current_directory );
    new_entry[1] = s_string( Index_file_name );
    tmp_list = (POINTER **) s_append( (POINTER *)tmp_list, (char *)new_entry );

#ifdef DEBUG
    debug( "create_on_tmp: adding new entry" );
    s_print( new_entry );
    s_print( tmp_list );
#endif

    Ermfile( Index_file_name );
    s_free( old_index_name );
    unstat( NULL );

    return( sfp );
}


/****************************************************************************/
/* get_directory:  used by I_hinit to determine directory from filename     */
/****************************************************************************/

LOCAL ASTRING
get_directory( ASTRING filename )
    /*filename;            either a directory or index file name */
{
	SFILE    *sfp;          /* used in reading name from index file  */
	C_RECORD  record;
	ASTRING   name;

    if (seq( filename + obj_count( filename ) - strlen( INDEX_NAME ),
	     INDEX_NAME )) {
	sfp = sf_open( filename, "r" );
	if (sfp == (SFILE *) ERROR)
	    Efatal( IOPEN, "Cannot open file \"%s\"", filename );

	record = (C_RECORD) sf_read( sfp, 0 );
	if (record == NULL || record == (C_RECORD) ERROR)
	    Efatal( IREAD, "Cannot read file %s", filename );

	name = s_findnode( record, F_PATHNAME );
	if (name == (ASTRING) ERROR)
	    Efatal( IREADFIELD, "Cannot read field \"%s\"", F_PATHNAME );

	name = s_string( name );
	s_free((char *) record );
	if (sf_close( sfp ) == ERROR)
	    Efatal( ICLOSE, "Cannot close file \"%s\"", filename );

	return( name );
    }

    /* If not given an index file as the file name, it is the directory */
    return( s_string( filename ));
}

/****************************************************************************/
/* get_index_file: finds a .index to use.  May have to create one.          */
/****************************************************************************/

void
get_index_file(void)
{
    auto SFILE *sfp;

#ifdef DEBUG
    debug( "get_index_file()" );
#endif

    if (Form_type != INDEX_FORM) {     /* then create on /tmp anyway */
	sfp = create_on_tmp();
	update_index_file( sfp );
	if (sf_close( sfp ) == ERROR)
	    Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name );
	return;
    }

    /* See if the current .index file can be updated */

    sfp = sf_open( Index_file_name, "u" );
    if (sfp == (SFILE *) ERROR) {

	/* Fix a broken .index file */
	if (g_errno == E_FILE || g_errno == E_ARRAY) {

	    Emessage(IBROKEN, "Attempting to reconstruct a broken .index file..." );

	    if (ghost( INDEX_NAME ) == RET_OK)
		sfp = sf_open( INDEX_NAME, "u" );
	}
    }

    /* If this did not work, then a new .index must be created */

    if (sfp == (SFILE *) ERROR)
	sfp = create_on_tmp();

    /* Now update it to reflect the current directory contents */

    update_index_file( sfp );

    if (sf_close( sfp ) == ERROR)
	Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);
}


/****************************************************************************/
/* is_reusable: determines if the .index file can be reused without update  */
/****************************************************************************/

LOCAL int
is_reusable(void)
{
#ifdef DEBUG
    debug( "is_reusable()" );
#endif

    /* If the index file does not exist or is nor readable,
       then it cannot be reused                             */

#ifdef DEBUG
    debug( "is_reusable: Index_file_name = \"%s\"", Index_file_name );
    debug( "    is_readable( Index_file_name ) = %s",
	   is_readable( Index_file_name ) ? "TRUE" : "FALSE");
#endif

    if (!is_readable( Index_file_name ))
	return( FALSE );

    /* If the user requests synchronization,
       then you had better do it.             */

#ifdef DEBUG
    debug( "    Synchronize = %s", Synchronize ? "TRUE" : "FALSE");
#endif

    if (Synchronize)
    {
	register C_RECORD  record;    /* A specific index file record */
	char *modp;
	time_t filedate;
	register SFILE    *sfp;       /* Index file stream */

	sfp = sf_open( Index_file_name, "u" );
	if (sfp == (SFILE *) ERROR) return( FALSE );

	record = (C_RECORD) sf_read( sfp, 0 );
	if (record == NULL || record == (C_RECORD) ERROR)
	{
	    if (sf_close( sfp ) == ERROR)
		Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);
	    return( FALSE );
	}

	modp = s_findnode( record, F_MODDATE );
	if (modp == (char *) ERROR) {
	    s_free((char *) record );
	    if (sf_close( sfp ) == ERROR)
		Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);
	    return( FALSE );
	}

	filedate = atol( modp );

	if (!_file_ok( Current_directory ))
	    Efatal( ISTATDIR, "Cannot stat directory \"%s\"",Current_directory);

	if (filedate != ST_MTIME( Current_directory )) {
	    if (sf_close( sfp ) == ERROR)
		Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);
	    s_free((char *) record );
	    return ( FALSE );
	}

	s_free((char *) record );

	if (sf_close( sfp ) == ERROR)
	    Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);
    }

    /* The index file must always be writable so that PICK-UP works */

#ifdef DEBUG
    debug( "    is_writable( Index_file_name ) = %s",
	   is_writable( Index_file_name ) ? "TRUE" : "FALSE");
#endif

    return( is_writable( Index_file_name ) );
}


/****************************************************************************/
/* is_visible: TRUE if the file should normally appear in the index.        */
/****************************************************************************/

LOCAL int
is_visible( char *file_name )
	     /*file_name;      File being checked              */
{
    register int   i;             /* Indexes through hidden patterns */

#ifdef DEBUG
    debug( "is_visible( file_name = \"%s\" )", file_name );
#endif

    for (i = 0; i < obj_count( Hidden_patterns ); i++)
	if (gmatch( file_name, Hidden_patterns[i] ))
	    return( FALSE );

    return( TRUE );
}


/****************************************************************************/
/* on_death: called when this helper dies.                                  */
/****************************************************************************/

#ifdef DEBUG
LOCAL int
on_death( int signo )
{
    debug( "on_death( signo = %d )", signo );

    s_free( Current_directory );
    s_free( Index_file_name );
    s_free( Put_directory );
    s_free( Hidden_patterns );

    (void)fclose( g_debugfp );

}
#endif


/****************************************************************************/
/* read_directory: returns an allocated array of the names of the files in  */
/*   the current directory.                                                 */
/****************************************************************************/

#define	MAX_FILES	(1000)		/* max number of files per dir */

LOCAL POINTER *
read_directory(void)
{
    POINTER	dir_list[MAX_FILES];	/* One record per file            */
    POINTER	*dir_listp;		/* allocated copy 		  */
    register ASTRING	file_name;	/* File name to enter in index    */
    register FILE	*stream;	/* I/O stream for directory file  */
    auto struct dirent	*file_entry;	/* Holds directory entry for file */
    register int	file_count;	/* Counts the files in dir_list   */
    auto struct stat	stat_buf;
    DIR *dirp;
									
    /* Open directory */
    stream = fopen(".", "r");
    if (stream == NULL)
	Efatal (IOPENDIR, "cannot open directory \".\"");

    if (fstat( fileno( stream ), &stat_buf ) != 0)
	Efatal( ISTATDIR, "Cannot stat directory \"%s\"", ".");

    /* save mod date for update_index_file */
    g_dirmoddate = stat_buf.st_mtime;

    dirp = opendir(".");
    if (dirp == NULL)
	Efatal( IOPENDIR, "Cannot open directory \"%s\"", "." );

    /* Loop, reading directory entries */
    for (file_count = 0 ;
	 ((file_entry = readdir(dirp)) != NULL) &&
	 (file_count < MAX_FILES) ; ) {
#ifdef DEBUG
      debug("read_directory: filename = %s, namlen = %d", file_name,
	    file_entry->d_namlen);
#endif

      if (Display_state == ALL_FILES || is_visible(file_entry->d_name)) {
	file_name = s_alloc( file_entry->d_namlen + 1, T_CHAR, NULL );
	(void) strncpy( (char *)file_name, file_entry->d_name, (size_t)file_entry->d_namlen );

#ifdef DEBUG
	debug("read_directory:  File added to list, Display_state is %o, All_FILES = %o",
	      Display_state, ALL_FILES); 
#endif

	dir_list[file_count++] = file_name;
      }
    }

    dir_listp = (POINTER *) s_alloc( file_count, T_POINTER, NULL );
    while (file_count-- > 0)
	dir_listp[file_count] = dir_list[file_count];

    qsort( (char *) dir_listp, (unsigned) obj_count( dir_listp ),
	sizeof( char * ), cmprecords ); 

    closedir (dirp);
    if (fclose( stream ) == EOF)
	Efatal( ICLOSEDIR, "Cannot close directory \"%s\"", ".");
#ifdef DEBUG
    debug("read_directory:  goodbye");
#endif
    return( dir_listp );
}


/****************************************************************************/
/* read_profile: construct directory for saving PICK_UP'ed & PICK_COPY'ed   */
/*   files.  Determines whether the directory should be synchronized.       */
/*   Determines which files should be hidden.  Sets Pd_exists,              */
/*   Put_directory, Synchronize, and Hidden_patterns globals.               */
/****************************************************************************/

LOCAL void
read_profile(void)
{
    ASTRING         pdnametmp;

#ifdef DEBUG
    debug( "read_profile()" );
#endif

    /* Determine whether the index file should be synchronized */

    Synchronize =  Pcheckbox( "0/Synchronize" );

#ifdef DEBUG
    debug( "read_profile: Synchronize = %s", Synchronize ? "TRUE" : "FALSE" );
#endif

    /* Determine where the Put Directory is */

    if (Put_directory != NULL)
	s_free( Put_directory );

    pdnametmp = Pgetsingle( "0/Put_directory" );

    if (pdnametmp != (ASTRING) ERROR) {

	Put_directory = (ASTRING) Efixvars( pdnametmp );

#ifdef DEBUG
	debug( "read_profile: Put_directory = \"%s\"", Put_directory );
#endif

	/* If the directory doesn't exist, try to create it */

	if (is_not_file( Put_directory )) {
	    char cmd[PATH_MAX + 7];   /* Command for creating put directory */
	    (void) sprintf( cmd, "mkdir %s", Put_directory );
	    system( cmd );
	    unstat( Put_directory );
	}
	Pd_exists = is_writable( Put_directory );
	s_free( pdnametmp );
    }
    else {
	Put_directory = NULL;
	Pd_exists = FALSE;

#ifdef DEBUG
	debug( "read_profile: Put_directory = NULL" );
#endif

    }

    /* Now find the hidden patterns */

    if (Hidden_patterns != NULL)
	s_free((char *) Hidden_patterns );

    Hidden_patterns = Pgetmulti( "0/Hidden_patterns" );

    if (Hidden_patterns == ((POINTER *) ERROR))
	Hidden_patterns = (POINTER *) s_alloc( 0, T_POINTER, NULL );

#ifdef DEBUG
    debug( "read_profile: Hidden_patterns are:" );
    s_print( Hidden_patterns );
#endif
}


/****************************************************************************/
/* update_index_file: make the file given as argument correspond as         */
/*   closely as possible both to the directory and the old .index file.     */
/*   Touch the file only if it really needs to be changed.                  */
/****************************************************************************/

LOCAL void
update_index_file(SFILE *sfp)
	     /*sfp;        Index file descriptor             */
{
    register POINTER  *drl;       /* Directory record list             */
    register int       i;         /* Indexes through the index file    */
    register int       j;         /* Indexes through the index file    */
    register char     *pointer;   /* File's location in directory list */
    register C_RECORD  record;    /* A specific index file record      */
    register char     *file_name; /* File name in .index record        */
    register int       place;     /* Index into drl being deleted      */
    POINTER *borg;                /* holds newly made record 0         */
    char datebuf[30];             /* directory mod date gets expanded  */
    char *alodate;                /* allocated version of mod date     */
    char *adirname;               /* allocated copy of directory name */
    int  rec_dirty;               /* TRUE if sf_write is necessary     */

#ifdef DEBUG
    debug( "update_index_file( sfp = %#x )", sfp );
#endif

    drl = read_directory();

    if (obj_count( sf_records( sfp )) == 0) { /* we are using a new file */
	sf_insert( sfp, 0, 2 );     /* put in two null records */
	rec_dirty = TRUE;
	record = (C_RECORD) s_alloc( 0, T_POINTER, NULL );
    }
    else {  /* we are using an old file */

	rec_dirty = FALSE;

	record = (C_RECORD) sf_read( sfp, 1 );

	if (record == NULL || record == (C_RECORD) ERROR)
	    Efatal( IREAD, "Cannot read file %s", Index_file_name );

	for ( i = 0; i < obj_count(record); i++ ) {

	    /* Delete records corresponding to deleted files */


	    file_name = s_findnode( (POINTER *)record[i], F_NAME );

	    /* Ignore lines that have no file name. This allows */
	    /* blank lines and lines that are headings or       */
	    /* comments to be put in the .index file.           */

	    if (file_name == ((char *) ERROR))
		continue;

	    /* Look for the file in the directory list */

	    pointer = bsearch( file_name, (char *) drl, obj_count( drl ),
		sizeof( char * ), cmpnames );
	    if (pointer == 0) {       /* File has been deleted    */
		s_free(record[i]);
		record = (C_RECORD) s_delete( (char *)record, i, 1 );
		i -= 1;
		rec_dirty = TRUE;
		continue;
	    }

	    place = ((POINTER *) pointer) - drl;
	    s_free( drl[place] );
	    drl = (POINTER *) s_delete( (char *)drl, place, 1 );

	}
    }

    /* Re-allocate the record array to avoid storage fragmentation */

    if (obj_count( drl ) > 0) { /* directory contains new files */

	j = obj_count( record  );

	if ((record = (POINTER *) s_insert( (char *)record, j, obj_count( drl ) )) == 
	   (POINTER *) ERROR)
	    Efatal( IINSREC, "Cannot insert record %d in SFILE object", j);

	rec_dirty = TRUE;

	/* Add the remaining files in drl to the index file. */

	for (i = 0; i < obj_count( drl ); i++, j++) {
	    POINTER *zorg;
	    s_newname( drl[i], F_NAME );
	    zorg = (POINTER *) s_alloc(1,T_POINTER,NULL);
	    zorg[0] = drl[i];
	    record[j] = (POINTER) zorg;
	}
    }

    if (rec_dirty)
	if (sf_write( sfp, 1, (char *)record ) == ERROR)
	    Efatal( IWRITE, "Cannot write file \"%s\"", Index_file_name);

    s_free( (char *) record );

    /* See whether we need to update record 0.       */
    /* Note that the directory name must be correct, */
    /* so we need only worry about the mod date      */

    (void) sprintf(datebuf,"%ld",g_dirmoddate);  /* set by read_directory */

    rec_dirty = FALSE;

    record = (C_RECORD) sf_read( sfp, 0 );
    if (record == NULL || record == (C_RECORD) ERROR)
	rec_dirty = TRUE;
    else
	rec_dirty = (seq (record[0], datebuf)) ? FALSE : TRUE;

    if (rec_dirty || (Display_state == ALL_FILES)) {

	/* If viewing all files, force the date to one guaranteed in the
	   past so that re-editing the file will force resynchronization
	   thus getting rid of the "hidden" files. */
	alodate = s_string( (Display_state == ALL_FILES) ? "0" : datebuf );
	s_newname( alodate, F_MODDATE );
	borg = (POINTER *) s_alloc( 2, T_POINTER, NULL );
	borg[0] = alodate;
	adirname = s_string( Current_directory );
	s_newname( adirname, F_PATHNAME );
	borg[1] = adirname;

	if (sf_write( sfp, 0, (char *)borg ) == ERROR)
	    Efatal( IWRITE, "Cannot write file \"%s\"", Index_file_name);

	s_free( (char *) borg);
    }

    s_freenode( (char *) drl );
    s_free( (char *) record);
}
