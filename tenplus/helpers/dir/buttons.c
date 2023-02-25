#if !defined(lint)
static char sccsid[] = "@(#)86	1.8  src/tenplus/helpers/dir/buttons.c, tenplus, tenplus411, GOLD410 3/11/94 17:15:23";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	I_hafter, I_hbefore, reset_line, set_line,
 *		u1, u2, u3, u4, u5, u9, unext, ustat, uzoomin, uzoomout
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
 
/*****************************************************************************
* File: buttons.c - handles processing of function buttons                   *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <chardefs.h>
#include <database.h>
#include <keys.h>
#include "index.h"
#include <string.h>
#include <libobj.h>
#include "libhelp.h"
#include <libutil.h>
#include <edexterns.h>
#include "INindex_msg.h"

#define INVISIBLE (0100) /* Including window.h causes too many defines */


static ASTRING File_name = NULL;
static ASTRING Field_name = NULL;
int            havelsdata;      /* if 0 we don't have valid ls data */


LOCAL void    set_line(void);
LOCAL void    reset_line(void);
LOCAL int     u1(int, ASTRING, int);
LOCAL int     u2(int, ASTRING, int);
LOCAL int     u3(int, ASTRING, int);
LOCAL int     u4(int, ASTRING, int);
LOCAL int     u5(int, ASTRING, int);
LOCAL int     u9(int, ASTRING, int);
LOCAL void    unext(void);
LOCAL int     ustat(int, ASTRING, int);
LOCAL int     uzoomin(int, ASTRING, int);
LOCAL int     uzoomout(int, ASTRING, int);



/*****************************************************************************
* I_hafter: dispatcher for key catches.                                        *
*****************************************************************************/

void I_hafter( int key )
/* Command code from keynames.h   */
{
#ifdef DEBUG
    debug( "I_hafter( key = %d )", key );
#endif

    switch (key) {

	case RESTORE:
	case PUT_DOWN:
	case PUT_COPY:
	    urestore( key );
	    break;

	case NEXT:
	case PREVIOUS:
	    unext();
	    break;

	default:
	    Efatal( IBADKEY, "Unknown key %d", key );
    }
}


/*****************************************************************************
* I_hbefore: dispatcher for key catches.                                       *
*****************************************************************************/

int I_hbefore( int key, int flag, ASTRING string, int number )
/* Command code from keynames.h   */
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
#ifdef DEBUG
    debug( "I_hbefore( key = %d, flag = %d, string = \"%s\", number = %d )",
	key, flag, (flag != 0) ? string : "", (flag == 2) ? number : 0 );
#endif

    switch ( key ) {    /* keys that understand region args */

	case DELETE:
	case PICK_COPY:
	case PICK_UP:
	    if (flag == BOXREGION)
		return ( 0 );   /* let I_hmod handle it */
	    else {
		Esavefile ();
		return( usave( flag, string, number, key ) );
	    }

	case RESTORE:
	case PUT_COPY:
	case PUT_DOWN:
	    if (flag == BOXREGION)
		return ( 0 );   /* let I_hmod handle it */
	    else
		return( uprepare( flag, string, number, key ) );

	default:        /* complain now if given a line region argument */

	    if (flag == LINEREGION || flag == BOXREGION) {
		Eerror( INOREGION,
		"Command does not take a region argument in this helper" );
		return( 1 );
	    }
	    break;      /* fall through to switch below                 */
    }

    switch ( key ) {    /* non-region keys                              */
	case U1:
	    return( u1( flag, string, number ) );

	case U2:
	    return( u2( flag, string, number) );

	case U3:
	    return( u3( flag, string, number) );

	case U4:
	    return( u4( flag, string, number) );

	case U5:
	    return( u5(flag, string, number) );

	case LOCAL_MENU:
	    return( u9(flag, string, number) );

	case ROFIELD:
	    if (Form_type == INDEX_FORM)
		Eerror( INODIRMOD,
		    "You do not have permission to modify this directory" );
	    else
		Eerror( IROFIELD, "This field may not be modified" );
	    return( 0 );  /* Return value ignored for PSEUDO Keys */

	case ZOOM_IN:
	    return( uzoomin( flag, string, number ) );

	case ZOOM_OUT:
	    return( uzoomout( flag, string, number ) );
    }
  exit(2); /* exit when case falls through */
  return(0); /* return put here to make lint happy */
}


/*****************************************************************************
* reset_line: restores the old line and field.                               *
*****************************************************************************/

LOCAL void
reset_line(void)
{
    auto     POINTER *file_list;
    register int      i;
    auto     int      flags;
    auto     int      use_fname;

#ifdef MAXDEBUG
    debug( "reset_line: File_name = \"%s\", Field_name = \"%s\"",
	File_name, Field_name );
#endif

    file_list = Egetlist( F_NAME, 0, -1 );
    if (file_list == (POINTER *) ERROR)
	Efatal( IREADFIELD, "Cannot read field \"%s\"", F_NAME );

    for (i = 0; i < obj_count( file_list ); i++) {
	if (seq( file_list[i], File_name ))
	    break;
    }

    if (i >= obj_count( file_list ))
	i = 0;

    s_free( File_name );
    File_name = NULL;

    /* If old field is hidden, use F_NAME */

    flags = Egetflags( Field_name );
    if (flags != ERROR)
	use_fname = (flags & INVISIBLE);
    else
	use_fname = TRUE;

    if (use_fname || Eputcursor( Field_name, i, 0) == ERROR)
	if (Eputcursor( F_NAME, i, 0) == ERROR)
	    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );

    s_free((char *) file_list );
    s_free(Field_name );
    Field_name = NULL;
}


/*****************************************************************************
* set_line: remembers the current file name and field name.                  *
*****************************************************************************/

LOCAL void
set_line(void)
{

#ifdef MAXDEBUG
    debug( "set_line: File_name = \"%s\", Field_name = \"%s\"",
	File_name, Field_name );
#endif

    File_name = Egettext( F_NAME, -1 );
    if (File_name == (char *) ERROR)
	Efatal( IREADFIELD, "Cannot read field \"%s\"", F_NAME );

    Field_name = Efieldname();
}


/*****************************************************************************
* u1: Display only the visible files in the directory.                       *
*****************************************************************************/
/*ARGSUSED*/
LOCAL int
u1( int flag, ASTRING string, int number )
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
    char *old_name;     /* Name of the old index file     */

#ifdef DEBUG
    debug( "u1( flag = %d, string = \"%s\", number = %d )",
	flag, (flag != 0) ? string : "", (flag == 2) ? number : 0 );
#endif

    if (Form_type == STAT_FORM) {
	Eerror( INOTAVAIL, "This function is not available from the current display");
	return( 1 );
    }

    if (flag != 0) {
	Eerror( INOARGS, "Command does not take any arguments" );
	return( 1 );
    }

    set_line();

    s_free( Esync() );

    old_name = s_string( Index_file_name );

    Display_state = VISIBLE_FILES;

    get_index_file();

    Ereopen();

    if (!seq( old_name, Index_file_name ))
	Eusefile( Index_file_name );

    reset_line();

    s_free( old_name );

    return( 1 );
}


/*****************************************************************************
* u2: Display all files in the directory.                                    *
*****************************************************************************/
/*ARGSUSED*/
LOCAL int
u2( int flag, ASTRING string, int number)
/* Indicates ENTER possibilities  */
/* User argument string*/
/* Argument converted to a number*/
{
    char *old_name;     /* Name of the old index file     */

#ifdef DEBUG
    debug( "u2()" );
#endif

    if (Form_type == STAT_FORM) {
	Eerror( INOTAVAIL, "This function is not available from the current display");
	return( 1 );
    }

    if (flag != 0) {
	Eerror( INOARGS, "Command does not take any arguments" );
	return( 1 );
    }

    set_line();

    s_free( Esync() );

    old_name = s_string( Index_file_name );

    Display_state = ALL_FILES;

    get_index_file();
    if (Form_type == LS_FORM)
	add_ls_data();

    Ereopen();

    if (!seq( old_name, Index_file_name ))
	Eusefile( Index_file_name );

    reset_line();

    s_free( old_name );

    /* if we u4 then u3 then u2 and then u5 we could be missing ls data  */
    if (Form_type != LS_FORM)
	havelsdata = FALSE;

    return( 1 );
}


/*****************************************************************************
* u3: view directory.                                                        *
*****************************************************************************/
/*ARGSUSED*/
LOCAL int
u3( int flag, ASTRING string, int number)
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
    auto int   line_number;
    auto char *line_string;

#ifdef DEBUG
    debug( "u3()" );
#endif

    if (flag != 0) {
	Eerror( INOARGS, "Command does not take any arguments" );
	return( 1 );
    }

    if (Form_type == INDEX_FORM) {
	Eerror( IALREADY, "You are already looking at that display" );
	return( 1 );
    }

    if (Form_type == LS_FORM) {
	line_string = Egetpath();
	set_line();
	s_free( Esync() );
	Form_type = INDEX_FORM;
	s_free( Index_file_name );
	Index_file_name = pathcat( s_string( Current_directory ),
				   s_string( INDEX_NAME ) );
	get_index_file();
	Ereopen();
	Eusefile( Index_file_name );
	Esetpath( line_string, Writable_directory ? "index" : "tindex" );
	Edispname( Current_directory );
	reset_line();
    }
    else {      /* STAT_FORM */
	char *temp0;

	line_string = Egetpath();
	temp0 = strrchr( line_string, DIR_SEPARATOR );
	if (temp0 == (char *) ERROR)
	    Efatal( IBADPATH, "Egetpath returned an incorrect path" );
	line_number = atoi( temp0 );
	s_free( line_string );
	s_free( Esync() );
	Form_type = INDEX_FORM;
	s_free( Index_file_name );
	Index_file_name = pathcat( s_string( Current_directory ),
				   s_string( INDEX_NAME ) );
	get_index_file();
	Ereopen();
	Eusefile( Index_file_name );
	Esetpath( "", Writable_directory ? "index" : "tindex" );
	Edispname( Current_directory );
	if (Eputcursor( F_NAME, line_number, 0 ) == ERROR)
	    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME );
    }

    Form_type = INDEX_FORM;

    return( 1 );
}


/*****************************************************************************
* u4: view ls -l information for the directory.                              *
*****************************************************************************/
/*ARGSUSED*/
LOCAL int
u4( int flag, ASTRING string, int number)
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
#ifdef DEBUG
    debug( "u4()" );
#endif

    if (Form_type == LS_FORM) {
	Eerror( IALREADY,
		"You are already looking at that display" );
	return( 1 );
    }

    if (flag != 0) {
	Eerror( INOARGS, "Command does not take any arguments" );
	return( 1 );
    }

    if (Form_type == INDEX_FORM) {
	set_line();
	s_free( Esync() );
	Form_type = LS_FORM;
	get_index_file();
	add_ls_data();
	Ereopen();
	Eusefile( Index_file_name );
	Edispname( Current_directory );
	Esetpath( "", Writable_directory ? "ls" : "tls" );
	reset_line();
    }
    else { /* STAT FORM */
	auto ASTRING field_name;
	auto int     line_number;
	auto ASTRING line_string;
	auto int     flags;
	auto int     use_fname;
	auto char    *temp0;

	line_string = Egetpath();
	temp0 = strrchr( line_string, DIR_SEPARATOR );
	if (temp0 == (char *) ERROR)
	    Efatal( IBADPATH, "Egetpath returned an incorrect path" );
	line_number = atoi( temp0 );
	s_free( line_string );

	field_name = Efieldname();

	if (havelsdata == FALSE)     /* we got here via u5 key */
	{
	    s_free( Esync() );
	    add_ls_data();
	    Ereopen();
	}

	Esetpath( "", Writable_directory ? "ls" : "tls" );
	Edispname( Current_directory );

	/* If field on ls form is hidden, use F_NAME */

	flags = Egetflags( field_name );
	if (flags != ERROR) {
	    use_fname = (flags & INVISIBLE);
	}
	else
	    use_fname = TRUE;

	if (use_fname || Eputcursor( field_name, line_number, 0) == 
	    ERROR)
	    if (Eputcursor( F_NAME, line_number, 0 ) == ERROR)
		Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"", F_NAME);

	s_free( field_name );
    }

    Form_type = LS_FORM;

    return( 1 );
}


/*****************************************************************************
* u5: view stat information for the file.                                    *
*****************************************************************************/
/*ARGSUSED*/
LOCAL int
u5(int flag, ASTRING string, int number)
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
#ifdef DEBUG
    debug( "u5()" );
#endif

    if (Form_type == STAT_FORM) {
	Eerror( IALREADY,
		"You are already looking at that display" );
	return( 1 );
    }

    ustat(NOARGS,"",0);

    return( 1 );
}


/*****************************************************************************
* u9: put up a menu of possible actions.                                     *
*****************************************************************************/

LOCAL int
u9(int flag, ASTRING string, int number)
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
  
    int line=0;
    int menu_item;
    int menu_no;
    char * msg_ptr;

    typedef int (*pfi)(int, ASTRING , int);

    static  pfi funlst1[] = {  /* INDEX_FORM */
	u1, u2, u4, u5
    };
    static  pfi funlst2[] = {  /* STAT FORM */
	u3, u4
    };
    static  pfi funlst3[] = {  /* LS_FORM */
	u1, u2,  u3, u5
    };


#ifdef DEBUG
    debug( "u9()" );
#endif


    if (Form_type == STAT_FORM) {
	menu_no = MN_IU9MENU2;
	msg_ptr = "\t       File Manager\n\
\n\
Select a menu option by moving the cursor to\n\
an item and presssing EXECUTE, or press CANCEL\n\
to remove the menu or HELP to display\n\
help information.\n\n\
@(3)              Return to normal directory display\n\
@(4) or ZOOM-OUT  Show Details About Files#0";
	menu_item = 0;
    }

    else if (Form_type == LS_FORM) {
	menu_no = MN_IU9MENU3;
	msg_ptr = "\t       File Manager\n\
\n\
Select a menu option by moving the cursor to\n\
an item and presssing EXECUTE, or press CANCEL\n\
to remove the menu or HELP to display\n\
help information.\n\n\
@(1)              Display Visible Files\n\
@(2)              Display All Files\n\
@(3)              Return to Normal Directory Display\n\
@(5) or ZOOM-IN   Show More Details About This File#0";
	menu_item = 2;
    }

    else {
	menu_no = MN_IU9MENU1;
	msg_ptr = "\t       File Manager\n\
\n\
Select a menu option by moving the cursor to\n\
an item and presssing EXECUTE, or press CANCEL\n\
to remove the menu or HELP to display\n\
help information.\n\n\
@(1)              Display Visible Files\n\
@(2)              Display All Files\n\
@(4)              Show Details About Files\n\
@(5)              Show More Details About This File#0";
	menu_item = 0;
    }

    line = Emenu(menu_no, msg_ptr, menu_item, NULL);

    if (line != ERROR)
    {
	if ( Form_type == STAT_FORM)
        (*(funlst2[line]))( 0, "", 0 );
    else
	if ( Form_type == LS_FORM)
	    (*(funlst3[line]))( 0, "", 0 );
	else
	    (*(funlst1[line]))( 0, "", 0 );

    }
    return( 1 );
}

/*****************************************************************************
* unext: next or previous file when viewing stat info.                       *
*****************************************************************************/

LOCAL void
unext(void)
{
    auto SFILE   *sfp;           /* For the index file              */
    auto int      i;             /* Counts records in theindex file */
    auto POINTER *record;        /* The data for one file           */
    auto POINTER *result;        /* Of call on add_stat_data        */
    auto ASTRING  path;          /* Should be the record number     */
    auto char    *temp0;

#ifdef DEBUG
    debug( "unext()" );
#endif

    if (Form_type != STAT_FORM)
	return;

    path = Egetpath();
    temp0 = strrchr( path, DIR_SEPARATOR );
    if (temp0 == (char *) ERROR)
	Efatal( IBADPATH, "Egetpath returned an incorrect path" );
    temp0++;
    i = atoi( temp0 );
    s_free( path );

    s_free( Esync() );

    sfp = sf_open( Index_file_name, "u" );
    if (sfp == (SFILE *) ERROR)
	Efatal( IOPEN, "Cannot open file \"%s\"", Index_file_name );

    record = (POINTER *) sf_read( sfp, 1 );
    if (record == (POINTER *) ERROR)
	Efatal( IREADFILE, "Cannot read file \"%s\"", Index_file_name );

    if (i < obj_count( record )) {

	if (record[i] != NULL) {
	    result = add_stat_data((POINTER *)record[i], LONG_FORM );
	    if (result != (POINTER *) NULL)
		record[i] = (POINTER) result;
	}
    }
    if (sf_write( sfp, 1, (char *)record ) == ERROR)
	Efatal( IWRITEFILE, "Cannot write file \"%s\"");

    s_free((char *) record );

    if (sf_close( sfp ) == ERROR)
	Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);

    Ereopen();
}


/*****************************************************************************
* ustat: view stat information for the file.                                 *
*****************************************************************************/
/*ARGSUSED*/
LOCAL int
ustat( int flag, ASTRING string, int number)
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
    auto char     tmpbuf[10];    /* int -> Char * conversion        */
    auto SFILE   *sfp;           /* For the index file              */
    auto int      i;             /* Current record in index file    */
    auto POINTER *record;        /* The data for one file           */
    auto POINTER *result;        /* Of call on add_stat_data        */
    auto ASTRING  field_name;
    auto int      flags;
    auto int      use_fullpath;
    auto ASTRING  old_name;

#ifdef DEBUG
    debug( "ustat()" );
#endif

    if (flag != 0) {
	Eerror( INOARGS, "Command does not take any arguments" );
	return( 1 );
    }

    i = Eline();

    field_name = Efieldname();

    s_free( Esync() );

    old_name = s_string( Index_file_name );

    if (Form_type == INDEX_FORM) {
	Form_type = STAT_FORM;
	get_index_file();
	}

    sfp = sf_open( Index_file_name, "u" );
    if (sfp == (SFILE *) ERROR)
	Efatal( IOPEN, "Cannot open file \"%s\"", Index_file_name );

    record = (POINTER *) sf_read( sfp, 1 );
    if (record == (POINTER *) ERROR)
	Efatal( IREADFILE, "Cannot read file \"%s\"", Index_file_name );

    if (i < obj_count( record )) {

	if (record[i] != NULL) {
	    result = add_stat_data( (POINTER *)record[i], LONG_FORM );
	    if (result != (POINTER *) NULL)
		record[i] = (POINTER) result;
	}
    }

    if (sf_write( sfp, 1, (char *)record ) == ERROR)
	Efatal( IWRITEFILE, "Cannot write file \"%s\"", Index_file_name );

    s_free((char *) record );

    if (sf_close( sfp ) == ERROR)
	Efatal( ICLOSE, "Cannot close file \"%s\"", Index_file_name);

    Ereopen();

    if (!seq( old_name, Index_file_name ))
	{
	Eusefile( Index_file_name );
	Edispname( Current_directory );
	}

    s_free( old_name );

    Form_type = STAT_FORM;

    (void) sprintf( tmpbuf, "1/%d", i );
    Esetpath( tmpbuf, Writable_directory ? "stat" : "tstat" );

    /* If field on stat form is hidden, use F_FULL_PATH */

    flags = Egetflags( field_name );

    if (flags != ERROR)
	use_fullpath = (flags & INVISIBLE);
    else
	use_fullpath = TRUE;

    if (use_fullpath || Eputcursor( field_name, 0, 0) == ERROR)
	if (Eputcursor( F_FULL_PATH, 0, 0 ) == ERROR)
	    Efatal( IPUTCURS, "Cannot put cursor in field \"%s\"" );

    s_free( field_name );

    return( 1 );
}


/*****************************************************************************
* uzoomin: handles zooming from the ls to the stat form.                     *
*****************************************************************************/

LOCAL int
uzoomin( int flag, ASTRING string, int number )
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
#ifdef DEBUG
    debug( "uzoomin( flag = %d, string = \"%s\", number = %d )",
	flag, (flag != 0) ? string : "", (flag == 2) ? number : 0 );
#endif

    if (Form_type == LS_FORM)
	return( ustat( flag, string, number ) );
    else
	return( 0 );
}


/*****************************************************************************
* uzoomout: handles zooming out from the stat to the ls form.                *
*****************************************************************************/

LOCAL int
uzoomout( int flag, ASTRING string, int number )
/* Indicates ENTER possibilities  */
/* User argument string           */
/* Argument converted to a number */
{
#ifdef DEBUG
    debug( "uzoomout( flag = %d, string = \"%s\", number = %d )",
	flag, (flag != 0) ? string : "", (flag == 2) ? number : 0 );
#endif

    if (Form_type == STAT_FORM)
    {

	return( u4( flag, string, number) );
    }
    else return( 0 );
}
