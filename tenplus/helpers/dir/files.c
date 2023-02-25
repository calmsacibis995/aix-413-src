#if !defined(lint)
static char sccsid[] = "@(#)85	1.7  src/tenplus/helpers/dir/files.c, tenplus, tenplus411, GOLD410 3/23/93 11:55:46";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	I_hins, bad_file_name, can_be_saved, copyfile, cpdir,
 *		get_status, good_name, save_files, undir, uprepare,
 *		urestore, usave, write_status
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
/* File: files.c - manage the saving and deleting of files                  */
/*                                                                          */
/* When a file is saved, the name under which it is saved must be           */
/*   remembered so that when it is restored, it can be found.  There are    */
/*   four routines involved. Usave is called when PICK_COPY, DELETE, or     */
/*   PICK_UP are hit.  They store the status of the save operation and the  */
/*   name of the saved file, if any, by writing into hidden fields in the   */
/*   record in the index file.  This record is then saved on the editor's   */
/*   stack.  When PUT_DOWN, RESTORE, or PUT_COPY is hit, three routines     */
/*   are called in succession.  First I_hbefore is called with PUT_DOWN,    */
/*   RESTORE, or PUT_COPY as an argument.  I/ubefore calls uprepare, which  */
/*   is responsible for resetting the index helper's state.  If the data    */
/*   being restored corresponds to records from the index file (and not     */
/*   characters or text lines) then I_hins will be called with a line       */
/*   number and a count.  The will be remembered for later use.  The        */
/*   editor will restore the lines and then I_hafter will be called with    */
/*   the same key as before.  By this time, the lines have been restored    */
/*   to the screen and the line number and count have been remembered by    */
/*   I_hins.  I_hafter calls urestore while uses the saved information to   */
/*   read the hidden information from the records.  The information is      */
/*   used to selectively restore the files.                                 */
/****************************************************************************/

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/access.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <database.h>
#include <chardefs.h>
#include "libhelp.h"
#include <libobj.h>
#include <libutil.h>
#include <sys/stat.h>
#include "index.h"
#include "statmod.h"
#include <keys.h>

#define BUFSIZE (200 * MB_LEN_MAX)

static char *st_array[] = {     /* Strings to be stored on editor's stack */
    "CANNOT_STAT",
    "PERMANENT_DELETION",
    "TAKEN_FROM_PUTDIR",
    "UNABLE_TO_SAVE",
    "FILE_SAVED",
    NULL
};

/* Possible usave dispositions */

#define CANNOT_STAT         (0)
#define PERMANENT_DELETION  (1)
#define TAKEN_FROM_PUTDIR   (2)
#define UNABLE_TO_SAVE      (3)
#define FILE_SAVED          (4)


static struct {
    int line_number;    /* Line where files are being restored */
    int count;          /* Number of lines being restored      */
} Uins_status = { -1, 0 };

static int uprep_count; /* Remember numberargs value until Uafter time */

LOCAL int     can_be_saved(POINTER * );
LOCAL int     cpdir( char *, char *, int );
LOCAL int     get_status( int );
LOCAL ASTRING good_name(ASTRING);
LOCAL int     save_files( POINTER *, int, int );
LOCAL int     undir(char *);
LOCAL void    write_status(int, char *, int);

/****************************************************************************/
/* I_hins:  called when the editor inserts PICK_COPY'ed or PICK_UP'ed lines */
/* into the file.                                                           */
/****************************************************************************/

void I_hins( ASTRING field_name, int line, int count )
{
#ifdef DEBUG
    debug( "I_hins( fieldname = \"%s\", line = %d, count = %d )", field_name, line, count );

#endif

    Uins_status.line_number = line;
    Uins_status.count = count;
}


/****************************************************************************/
/* bad_file_name: returns TRUE if the argument contains a bad character.    */
/****************************************************************************/

int bad_file_name( char *file_name )
/* File name to be checked */
{

    wchar_t wfile_name[PATH_MAX];
    int indx = 0;

#ifdef MAXDEBUG
    debug( "bad_file_name( %s )", file_name );
#endif

    (void) mbstowcs(wfile_name, file_name, PATH_MAX);
    while (wfile_name[indx] != L'\0') {
      if (wfile_name[indx] == L'/' || !iswprint(wfile_name[indx]))
         return(TRUE);
      else
         indx++;
      } 
return(FALSE);
}


/****************************************************************************/
/* can_be_saved: check to see whether all of the items in the argument list */
/*   can be saved.  If so, returns TRUE, else FALSE.                        */
/****************************************************************************/

LOCAL int
can_be_saved(POINTER * file_list )
/* List of files to be saved */
{
    register int     j;                 /* Counts files in the list   */
    register ASTRING file_name;         /* Name of file being saved   */

#ifdef DEBUG
    debug( "can_be_saved( file_list = %#o )", file_list );
#endif
									
    if (!Pd_exists) {    /* No place to save a file */
	Eerror( INOPD, "There is no place available to save files" );
	return( FALSE );
    }

    for (j = 0; j < obj_count( file_list ); j++) {

	/* Get the file's name */

	file_name = file_list[j];
	if (file_name == (char *) ERROR)
	    Efatal( IFINDNAME, "Cannot find file name" );

	if (is_not_file( file_name ))  /* For non-existent files */
	    continue;

	if (is_regular( file_name )) {

	    /* Don't allow deletion of the index file itself */

	    if ((strcmp( file_name, INDEX_NAME )==0) ) {

		if (Eputcursor( F_NAME, Eline() + j, 0 ) == ERROR)
		    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );
		Eerror( IINDEXSV, "You are not allowed to delete \"%s\"",
		    INDEX_NAME );
		return( FALSE );
	    }

	    if (strncmp( file_name, "...", 3 ) == 0) {

		/* Don't allow deletion of ASCII temp files */

		if (Eputcursor( F_NAME, Eline() + j, 0 ) == ERROR)
		    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );
		Eerror( IASCIISV, "You are not allowed to delete \"%s\"",
		    file_name );
		return( FALSE );
	    }
	}

	else if (is_directory( file_name )) { /* For directories */

	    /* Check for deletion of . and .. */

	    if ((strcmp( file_name, "." )==0)) {
		if (Eputcursor( F_NAME, Eline() + j, 0 ) == ERROR)
		    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );
		Eerror( IDIRSV, "You are not allowed to delete the current directory" );
		return( FALSE );
	    }

	    if ((strcmp( file_name, ".." )==0)) {
		if (Eputcursor( F_NAME, Eline() + j, 0 ) == ERROR)
		    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );
		Eerror( IPARSV, "You are not allowed to delete the parent directory" );
		return( FALSE );
	    }
	}

	else { /* For special files */

	    if (Eputcursor( F_NAME, Eline() + j, 0 ) == ERROR)
		Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );
	    Eerror( ISPECIAL, "Unable to save special file" );
	    return( FALSE );
	}
    }

    return( TRUE );
}


/****************************************************************************/
/* copyfile: makes a copy of the file named by arg1 in a file named arg2.   */
/****************************************************************************/

int copyfile( char *old_file, char *new_file )
{
    register int     c;            /* Holds a character at a time */
    register FILE   *fda;          /* Stream for the old file     */
    register FILE   *fdb;          /* Stream for the new file     */
    auto     int    mode;         /* Of the file being copied    */
    auto     char   *acl;	   /* File access control list    */

#ifdef MAXDEBUG
    debug( "copyfile( old_file = \"%s\", new_file = \"%s\" )", old_file,
	new_file );
#endif

    fda = fopen( old_file, "r" );
    fdb = fopen( new_file, "w" );

    if (fda == NULL || fdb == NULL)
	return( ERROR );

    mode = ST_MODE( old_file );
    acl = (char *)acl_get( old_file );

    for(;;) {
	c = getc( fda );
	if (feof( fda ))
	    break;
	putc( c, fdb );
	if (ferror( fda ) || ferror( fdb ))
	    return( ERROR );
    }

    if (fclose( fda ) == EOF || fclose( fdb ) == EOF)
	return( ERROR );

    /* We set the file modes even though we are about to set the */
    /* access control list, in order to make sure that the SUID bit, */
    /* etc. are copied from the old file to the new file. */
    if (chmod( new_file, mode ) != 0)
	{
	char s [32];

	(void) sprintf (s, "%o", mode);
	Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", s );
	}

    if (acl_put( new_file, acl, TRUE) != 0)
	Eerror ( IBADCHACL, "Unable to change access control list");

    return( 0 );
}


/****************************************************************************/
/* cpdir: use cpio to move a copy a directory hierarchy.                    */
/*   Returns RET_OK if successful, otherwise, ERROR.                        */
/****************************************************************************/

LOCAL int
cpdir( char *old_directory, char *new_directory, int key )
{
    auto char cmdbuf[BUFSIZE];
    auto int  res;

#ifdef DEBUG
    debug( "cpdir( old_directory = \"%s\", new_directory = \"%s\" )",
	old_directory, new_directory );
#endif

    (void) sprintf( cmdbuf,
	"mkdir %s > /dev/null 2> /dev/null", new_directory );

#ifdef DEBUG
    debug( "cpdir: cmdbuf = \"%s\"", cmdbuf );
#endif

    if (system( cmdbuf ) != 0) {
	Eerror( INOMKDIR, "Unable to create directory \"%s\"", new_directory );
	return( ERROR );
    }

    if (key == PICK_UP || key == DELETE || key == PICK_COPY)
	Emessage(ISAVE, "Saving directory hierarchy \"%s\"...", old_directory );
    else
	Emessage(IRESTOR, "Restoring directory hierarchy \"%s\"...", new_directory );

    if (chdir( old_directory ) == -1) {
	Eerror(INOCHDIR, "Unable to chdir to \"%s\"", old_directory );
	return( ERROR );
    }

    if (key == PICK_UP || key == DELETE || key == PICK_COPY)
	(void) sprintf( cmdbuf,
	    "find . -print 2>/dev/null | cpio -pdl %s >/dev/null 2>/dev/null",
	    new_directory );
    else if (key == PUT_COPY)
	(void) sprintf( cmdbuf,
	   "find . -print 2>/dev/null | cpio -pd %s/%s >/dev/null 2>/dev/null",
	   Current_directory, new_directory );
    else
	(void) sprintf( cmdbuf,
	   "find . -print 2>/dev/null | cpio -pdl %s/%s >/dev/null 2>/dev/null",
	   Current_directory, new_directory );

#ifdef DEBUG
    debug( "cpdir: cmdbuf = \"%s\"", cmdbuf );
#endif

    if ((res = system( cmdbuf )) != 0) {

#ifdef DEBUG
	debug( "cpdir: res = %d", res );
#endif

	if (key == PICK_UP || key == DELETE || key == PICK_COPY)
	    Eerror( IBADCPIO, "Unable to save/restore directory hierarchy \"%s\"",
		    old_directory );
	else
	    Eerror( IBADCPIO, "Unable to save/restore directory hierarchy \"%s\"",
		    new_directory );
	if (chdir( Current_directory ) == -1)
	    Efatal( INOCHDIR, "Cannot change to directory \"%s\"", Current_directory );
	return( ERROR );
    }
    else {
	if (chdir( Current_directory ) == -1)
	    Efatal( INOCHDIR, "Cannot change to directory \"%s\"", Current_directory );
	return( RET_OK );
    }
}


/****************************************************************************/
/* get_status: returns the file dispositon status saved by the editor on    */
/*   stack.                                                                 */
/****************************************************************************/

LOCAL int
get_status( int line_number )
{
    register int     i;          /* Indexes into st_array */
    register ASTRING string;     /* Text read from file   */

#ifdef DEBUG
    debug( "get_status( line_number = %d )", line_number );
#endif

    string = Egettext( F_SSTATUS, line_number );
    if (string == (char *) ERROR)
	Efatal( IREADFIELD, "Cannot read field \"%s\"", F_SSTATUS );

    for (i = 0; st_array[i] != '\0'; i++) {
	if ((strcmp( string, st_array[i] )==0))
	    return( i );
    }

    return( 0 );
}


/****************************************************************************/
/* good_name: the argument file already exists in the directory.  Use user  */
/*   prompts and defaults to create a name which can be used to restore the */
/*   file.                                                                  */
/****************************************************************************/

LOCAL ASTRING
good_name( ASTRING bad_name )
/* Name to be replaced            */
{
    register ASTRING new_name;           /* Substitute name to be returned */
    register int     i;                  /* Used to construct new name     */
    auto     char    buffer[PATH_MAX + 1]; /* Holds new name                 */

#ifdef MAXDEBUG
    debug( "good_name( bad_name = \"%s\" )", bad_name );
#endif

    /* First give the user an opportunity to rename the file */

    new_name = Eask( IDUPNAME,
"File name \"%s\" already exists.\nPress CANCEL to abort file restore,\nor enter a new name:                ",
		     bad_name );

    /* See if user hit CANCEL */

    if (new_name == (char *) ERROR)
	return( (ASTRING) ERROR );

    /* Check for bad characters in the user-supplied name */

    if (bad_file_name( new_name )) {
	Eerror( IBADCHAR, "File name \"%s\" contains bad characters",
	    new_name );
	s_free( new_name );
	new_name = bad_name;
    }

    /* Construct a name, if necessary */

    if (is_not_file( new_name ))
	return( new_name );

    for (i = 0; ; i++) {
	(void) sprintf( buffer, "%d.%s", i, new_name );
	if (is_not_file( buffer ))
	    break;
    }

    return( s_string( buffer ) );
}

/****************************************************************************/
/* save_files: saves files into the put directory.                          */
/*   The input is a list of files.  Each file corresponds to a record in th */
/*   .index file.  The ultimate disposition of each file is written into tw */
/*   fields in the record, and where appropriate, the corresponding file is */
/*   saved in the put directory.                                            */
/*   If save_files was succesful, 0 is returned. Otherwise, 1 is returned.  */
/****************************************************************************/

LOCAL int
save_files( POINTER *file_list, int key, int line_number )
/* The list of files to be saved            */
/* Either PICK_COPY, DELETE, or PICK_UP     */
/* Of the first line bieng saved            */
{
    register int     j;            /* Indexes through file_list         */
    register ASTRING file_name;
    register int     i;            /* Used for constructing saved names */
    auto     char    new_name[PATH_MAX]; /* Holds constructed name       */
    int result = 0; /* holds return value that will be returned to caller */

#ifdef DEBUG
    debug( "save_files( file_list = %#o, key = %d )", file_list, key );
#endif

    for (j = 0; j < obj_count( file_list ); j++) {

	file_name = file_list[j];

	if (is_not_file( file_name )) {
	    write_status( line_number + j, "", CANNOT_STAT );
	    continue;
	}

	/* Deletions from the put directory are permanent */

	if ((strcmp( Current_directory, Put_directory )== 0)) {
	    if (key == PICK_UP || key == DELETE) {

		write_status( line_number + j, "", PERMANENT_DELETION );


		if (is_directory( file_name )) {
		    if (undir( file_name ) != 0)
                        result = 1;
		    else unstat( file_name );
                }
		else {
		   if (unlink( file_name ) != 0)
                        result = 1;
		   else unstat( file_name );
                }
	    }
	    else {
		auto ASTRING cat_results;

		cat_results = pathcat( s_string( Put_directory ),
				       s_string( file_name ) );
		write_status( line_number + j, cat_results, TAKEN_FROM_PUTDIR );


		s_free( cat_results );
	    }
	    continue;
	}

	/* If the file is ASCII, save the ... file */

	if (! isdelta( file_name )) {
	    ASTRING fakefile;


	    fakefile = (ASTRING) fakename( file_name );
	    if (is_file( fakefile )) {
		Esaveascii( file_name );
		if (unlink( fakefile ) != -1) {
		    s_free( Esync() );
		    get_index_file();
		    Ereopen();
		}
		unstat( file_name );
	    }
	    s_free( fakefile );
	}

	/* For files that must be saved, construct the saved name */

	for (i = 0; ; i++) {
	    (void) sprintf( new_name, "%s/%d.%s", Put_directory, i, file_name );
	    if (is_not_file( new_name ))
		break;
	}

	/* Save the file */

	if (is_directory( file_name )) {
	    auto int res;

	    if ((res = cpdir( file_name, new_name, key )) == ERROR) {

#ifdef DEBUG
		debug( "save_files: res = %d", res );
#endif

		write_status( line_number + j, "", UNABLE_TO_SAVE );

		continue;
	    }
	}
	else {
	    if ((key == PICK_COPY) ||
		(link( file_name, new_name ) == -1))
		if (copyfile( file_name, new_name ) == ERROR) {

		    /* File cannot be backed up.  It will be deleted from
		       screen but not from directory.                       */

		    write_status( line_number + j, "", UNABLE_TO_SAVE );

		    continue;
		}
	}

	write_status( line_number + j, new_name, FILE_SAVED );


	/* If PICK_UP or DELETE was hit, delete the file */

	if (key == PICK_UP || key == DELETE) {
		if (is_directory( file_name )) {
		    if (undir( file_name ) != 0)
                        result = 1;
		    else unstat( file_name );
                }
		else {
		   if (unlink( file_name ) != 0)
                        result = 1;
		   else unstat( file_name );
                }
	}
    }
    return(result);
}


/****************************************************************************/
/* undir: remove a directory and its contents.                              */
/*        Returns 0 when all went well, returns 1 when an error occured.    */
/****************************************************************************/

LOCAL int 
undir( char *directory )
{
    auto char cmdbuf[100 * MB_LEN_MAX];

#ifdef DEBUG
    debug( "undir( directory = \"%s\" )", directory );
#endif

    (void) sprintf( cmdbuf, "/bin/rm -r %s < /dev/null > /dev/null 2> /dev/null",
	directory );

    if (system( cmdbuf ) != 0) {
	Eerror( IRMDIR, "Unable to remove directory \"%s\"", directory );
        return(1);
    }

    return(0);
}


/****************************************************************************/
/* uprepare: sets up global variables for subsequent urestore call.         */
/****************************************************************************/

int uprepare( int flag, ASTRING string, int number, int key )
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
/* Either PUT_DOWN or PUT_COPY    */
{
#ifdef DEBUG
    debug( "uprepare( flag = %d, string = \"%s\", number = %ld, key = %d )",
	flag, (flag != 0) ? string : "", (flag == 2) ? number : 0l, key );
#endif

    if (Form_type == STAT_FORM) {
	Eerror( INORESTORE, "Lines may not be restored while viewing file statistics" );
	return( TRUE );
    }

    Uins_status.line_number = -1;
    Uins_status.count = 0;
    uprep_count = flag == NUMBERARG ? number : 1;

    return( FALSE );
}


/****************************************************************************/
/* urestore: called when PUT_DOWN, RESTORE, or PUT_COPY are hit.            */
/****************************************************************************/

void urestore( int key )
/* Either PUT_DOWN, RESTORE or PUT_COPY */
{
    register int     j;             /* Counts the records being restored    */
    auto     int     status;        /* Disposition status at file save time */
    auto     int     screen_line;   /* Line number being restored           */
    auto     int     num_lines;     /* How many lines being restored        */
    register ASTRING file_name;     /* File being restored                  */
    register ASTRING saved_name;    /* Name under which file was saved      */
    register ASTRING new_name;      /* Replacement name                     */

#ifdef DEBUG
    debug( "urestore( key = %d )", key );
#endif

    /* Check if the restore is for other than files */

    if (Uins_status.line_number == -1 || Uins_status.count == 0)
	return;

    /* Loop through the restored files */

    screen_line = Uins_status.line_number;
    num_lines = Uins_status.count * uprep_count;

    for (j = num_lines; j > 0; j--, screen_line++) {

	status = get_status( screen_line );

	switch (status) {

	    case FILE_SAVED:
	    case TAKEN_FROM_PUTDIR:

		file_name = Egettext( F_NAME, screen_line );
		if (file_name == (char *) ERROR)
		    Efatal( IREADFIELD, "Cannot read field \"%s\"", F_NAME);

		if (is_file( file_name )) {  /* Name collision */
		    new_name = good_name( file_name );
		    if (new_name == (ASTRING) ERROR) {
			if (Edelline( F_NAME, screen_line, 1 ) == ERROR)
			    Efatal( IDEL, "Cannot delete line from field \"%s\"", F_NAME);
			Eerror( IABORT, "File restoration aborted.\nFile can be found in your putdir." );
			screen_line -= 1;
			s_free( file_name );
			break;
		    }
		    else
			if (Eputtext( F_NAME, screen_line, new_name ) ==
			    ERROR)
			    Efatal( IWRITEFIELD, "Cannot write field \"%s\"", F_NAME);

		    s_free( file_name );
		    file_name = new_name;
		}

		saved_name = Egettext( F_SNAME, screen_line );
		if (saved_name == (char *) ERROR)
		    Efatal( IREADFIELD, "Cannot read field \"%s\"", F_SNAME);

		if (is_directory( saved_name )) {
		    if (cpdir( saved_name, file_name, key ) == ERROR) {
			if (Edelline( F_NAME, screen_line, 1 ) == ERROR)
			    Efatal( IDEL, "Cannot delete line from field \"%s\"", F_NAME);
			screen_line -= 1;
			s_free( file_name );
			s_free( saved_name );
			break;
		    }


		}
		else {
		    if (ST_NLINK( saved_name ) == 1) {
			if (link( saved_name, file_name ) == -1) {
			    if (copyfile( saved_name, file_name ) == ERROR) {
				if (Edelline( F_NAME, screen_line, 1 ) == ERROR)
				    Efatal( IDEL, "Cannot delete line from field \"%s\"", F_NAME);
				screen_line -= 1;
				Eerror( IBADCOPY, "Unable to restore file \"%s\"",
				    file_name );
			    }


			}


		    }
		    else if (copyfile( saved_name, file_name ) == ERROR) {
			if (Edelline( F_NAME, screen_line, 1 ) == ERROR)
			    Efatal( IDEL, "Cannot delete line from field \"%s\"", F_NAME);
			screen_line -= 1;
			Eerror( IBADCOPY, "Unable to restore file \"%s\"", file_name );
		    }

		    unstat( file_name );
		}

		/* Don't unlink until the last pass in a NUMBERARGS PUT_DOWN */

		if ((key == PUT_DOWN || key == RESTORE) &&
		    (status == FILE_SAVED) && (j <= Uins_status.count)) {
		    if (is_directory( saved_name ))
			undir( saved_name );
		    else
			(void)unlink( saved_name );
		    unstat( saved_name );
		}

		s_free( file_name );
		s_free( saved_name );
		break;

	    case PERMANENT_DELETION:
		file_name = Egettext( F_NAME, screen_line );
		if (file_name == (char *) ERROR)
		    Efatal( IREADFIELD, "Cannot read field \"%s\"", F_NAME);
		Eerror( INOFRESTOR, "File \"%s\" was permanently deleted from\nyour put directory and cannot be restored", file_name);
		if (Edelline( F_NAME, screen_line, 1 ) == ERROR)
		    Efatal( IDEL, "Cannot delete line from field \"%s\"", F_NAME);
		screen_line -= 1;
		s_free( file_name );
		break;

	    case UNABLE_TO_SAVE:
	    case CANNOT_STAT:
		break;

	    default:
		Efatal( IBADSTAT, "Unknown status %d", status);
	}
    }
}


/****************************************************************************/
/* usave: called on PICK_UP, DELETE or PICK_COPY.                           */
/*   Names are created under which files can be saved in the put directory  */
/*   without conflicting with other files already there.  The new name will */
/*   look like x/d.y where x is the name of the put directory, d is a small */
/*   integer, and y is the name of the file.  The file is then linked or    */
/*   copied to the newly named file in the put directory.  Write_status is  */
/*   called to save information in the record on the editor's stack.        */
/****************************************************************************/

int usave( int flag, ASTRING string, int number, int key )
    /*flag        Indicates ENTER possibilities  */
    /*string      User argument string           */
    /*number      Argument converted to a number */
    /*key         PICK_COPY, DELETE or PICK_UP   */
{
    register POINTER *file_list;     /* The files being saved         */
    register int      line_number;   /* Of the first line to be saved */

#ifdef DEBUG
    debug( "usave( flag = %d, string = \"%s\", number = %d, key = %d )",
	flag, (flag != 0) ? string : "", (flag == 2) ? number : 0, key );
#endif
									
    if (Form_type == STAT_FORM && (key == PICK_UP || key == DELETE)) {
	Eerror( INOREMOVE, "Lines may not be removed while viewing file statistics" );
	return( TRUE );
    }

    if (Form_type == STAT_FORM)
	return( FALSE );

    line_number = Eline();

    switch ( flag ) {

	case NOARGS:
	    number = 1;
	    break;

	case EMPTYARG:
	    Eerror( INOJOIN, "JOIN is not available when editing directories" );
	    return( TRUE );

	case NUMBERARG:
	    break;

	case LINEREGION:
	    {
	    int t, l, b, r;     /* top, left, bottom, and right of region  */
	    Lgetregion (string, &t, &l, &b, &r);
	    line_number = t;
	    number = b - t + 1;
	    break;
	    }

	default:
	    Efatal( IBADFLAG, "Unexpected flag %d", flag );
    }

    file_list = Egetlist( F_NAME, line_number, number );
    if (file_list == (POINTER *) ERROR)
	Efatal( IREADFIELD, "Cannot read field \"%s\"", F_NAME);

    if (can_be_saved( file_list )) {
        int result = save_files( file_list, key, line_number );
	s_free( (char *) file_list );
	return( result );
    }

    s_free( (char *) file_list );
    return( 1 );
}


/****************************************************************************/
/* write_status: constructs the string to be saved by the editor on usaves. */
/****************************************************************************/

LOCAL void
write_status( int line_number, char *saved_name, int status )
{
#ifdef MAXDEBUG
    debug( "write_status( line_number = %d, saved_name = \"%s\", status = %d )",
	line_number, saved_name, status );
#endif

    if (Eputtext( F_SSTATUS, line_number, st_array[status] ) == -1)
	Efatal( IWRITEFIELD, "Cannot write field \"%s\"", F_SSTATUS);

    if (Eputtext( F_SNAME, line_number, saved_name ) == -1)
	Efatal( IWRITEFIELD, "Cannot write field \"%s\"", F_SNAME);
}
