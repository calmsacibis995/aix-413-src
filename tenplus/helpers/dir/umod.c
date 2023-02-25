static char sccsid[] = "@(#)40	1.7  src/tenplus/helpers/dir/umod.c, tenplus, tenplus411, GOLD410 11/1/93 17:41:55";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	I_hmod, change_name, fix_ctime, getfield, ls_modify,
 *		modec2i, modify_stat, permissions, strdate, parsedate,
 *		time_change
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL code. -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <database.h> 
/****************************************************************************/
/* File: umod.c - handles modifications to the data being displayed.        */
/****************************************************************************/

#include <sys/mode.h>
#include "index.h"
#include <utime.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>
#include "statmod.h"
#include <libutil.h>
#include <libobj.h>
#include "libhelp.h"
#include "hdir.h"
#include <edexterns.h>
#include <langinfo.h>

#define BUFFER_SIZE     (100 * MB_LEN_MAX)

#define is_now(s)       ((s[0] == 'n' || s[0] == 'N') && \
			 (s[1] == 'o' || s[1] == 'O') && \
			 (s[2] == 'w' || s[2] == 'W'))

#define putfield(f,v)   if (Eputtext( f, 0, v ) == ERROR) \
			  Efatal( IWRITEFIELD, "Cannot write field \"%s\"", f )


extern char *months[12][2];

LOCAL char *get_datefmt();
LOCAL char *strip_date(char *, char *);
LOCAL int   change_name(char *, char *, int);
LOCAL void  fix_ctime(char *);
LOCAL char *getfield(char *);
LOCAL int   ls_modify(ASTRING, int, ASTRING, ASTRING);
LOCAL int   modify_stat(ASTRING, int, ASTRING, ASTRING);
LOCAL int   permissions(char *, char *, char *);
LOCAL int   time_change(char *,char *,char *, int);


/****************************************************************************/
/* I_hmod: called when a field is modified.                                 */
/****************************************************************************/

int I_hmod( register  ASTRING field_name, register int line_number, register ASTRING old_line, register ASTRING new_line )
/* Name of the modified field           */
/* Number of the line that was modified */
/* The previous contents of the field   */
/* The altered contents of the field    */
{
#ifdef DEBUG
    debug( "I_hmod( field_name = \"%s\", line_number = %d, ", field_name,
	line_number );
    debug( "old_line = \"%s\",", old_line );
    debug( "new_line = \"%s\" )", new_line );
#endif

    if (Form_type == STAT_FORM)
	return( modify_stat( field_name, line_number, old_line, new_line ) );

    if (Form_type == LS_FORM)
	return( ls_modify( field_name, line_number, old_line, new_line ) );

    if (seq( field_name, F_NAME ))
	return( change_name( old_line, new_line, line_number ) );

    Efatal( IWRITEFIELD, "Cannot write field \"%s\"", field_name );
  
  exit(2); /* exit in case Efatal falls through (it should never do so) */
  return(0); /* return put here to make lint happy */
}


/****************************************************************************/
/* change_name: change the name of a file, if possible.                     */
/****************************************************************************/

LOCAL int
change_name( char * old_name, char * new_name, int line_number )
{
	int wasntdelta;         /* true if file isn't a delta file */
	int hadafake;           /* true if the file had a fake file */
	ASTRING fakefile;       /* fake name for the old file name */
	ASTRING newfake;        /* fake name for the new file name */
	int keyvalue;           /* what key is being processed     */
	char cmd_buf[2 * PATH_MAX + 100];

#ifdef DEBUG
    debug( "change_name( old_name = \"%s\", new_name = \"%s\", line_number = %d, ",
	old_name, new_name, line_number );
#endif

    if (seq( new_name, old_name )) /* File name unchanged */
	return( FALSE );

    if (*new_name == '\0')   /* If file name has been blanked out */
	if (is_file( old_name )) {
	    Eputcursor( F_NAME, line_number, 1 );
	    Eerror( INOBLANK, "You cannot delete a file by blanking its name out" );
	    return( TRUE );
	}
	else                 /* Blanking out non-filename is okay */
	    return( FALSE );

    keyvalue = Ekeyvalue();
    if (( keyvalue == PUT_DOWN ) || ( keyvalue == PUT_COPY )
	  || ( keyvalue == RESTORE ))
	return( FALSE );        /* These cases will be handled by urestore */

    if (is_file( new_name )) { /* If file already exists */
	Eputcursor( F_NAME, line_number, 1 );
	Eerror( IDUPMOD,
	    "Duplicate file name \"%s\".  Please try again.", new_name );
	return( TRUE );
    }

    if (is_not_file( old_name ))
	return( FALSE );

    if (bad_file_name( new_name )) { /* Bad chars in name */
	Eputcursor( F_NAME, line_number, 1 );
	Eerror( IBADCHAR, "File name \"%s\" contains bad characters", new_name );
	return( TRUE );
    }

    if (strncmp( old_name, "...", 3) == 0 ) {
	Eputcursor( F_NAME, line_number, 1 );
	Eerror( IBADNAME, "You are not allowed to rename file \"%s\"",
	    old_name );
	return( TRUE );
    }

    /* save some info about the file for later */
    wasntdelta = !isdelta(old_name);

    if(wasntdelta) {
    fakefile = (ASTRING) fakename( old_name );
    hadafake = is_file(fakefile);
    }

    /* we need to save the file if it was open */
    if(wasntdelta && hadafake) Esaveascii(old_name);

    /* All tests passed, so rename the file */

    (void) sprintf(cmd_buf, "/usr/bin/mv %s %s", old_name, new_name);
    if (system(cmd_buf) != 0) {
	Eputcursor( F_NAME, line_number, 1 );
	if (is_directory( old_name )) {
	    Eerror( INOMV, "Unable to rename directory \"%s\"", old_name );
	    return( TRUE );
	}
	else {
	    /*
	     * Efatal() call not needed here as 'mv' will come clean
	     * and we donot have to re-read the dir.
	     * But removal of Efatal rquires msg catalog changes ( help )
	     * and hence it it's left out as before. ( Ref def. 97837 )
	     */
	    Efatal( IRENAME, "Unable to rename file \"%s\"", old_name );
	}
    }
    unstat( old_name );

    /* Handle the ... file */

    if (wasntdelta) {

	newfake = (ASTRING) fakename( new_name );

	if (hadafake) {
	    if (link( fakefile, newfake ) != -1) {
		if (unlink( fakefile ) == -1) {
		    (void)unlink( newfake );
		    Eputcursor( F_NAME, line_number, 1 );
		    Efatal( IRENAME, "Cannot rename file \"%s\"", fakefile );
		}
	    }
	    unstat( fakefile );
	    if (Display_state == ALL_FILES) {
		s_free( Esync() );
		get_index_file();
		Ereopen();
	    }
	}
	s_free( newfake );
	s_free( fakefile );
    }

    return( FALSE );
}


/****************************************************************************/
/* fix_ctime: update the F_STATUS_CHANGE_TIME field.                        */
/****************************************************************************/

LOCAL void
fix_ctime( char *file_name )
{
    auto time_t  stime;

#ifdef DEBUG
    debug( "fix_ctime( file_name = \"%s\" )", file_name );
#endif

    unstat( NULL );
    stime = ST_CTIME( file_name );
    if (stime == (time_t) ERROR)
	Efatal( IUPD, "Cannot update field \"%s\"", F_STATUS_CHANGE_TIME );
    putfield( F_STATUS_CHANGE_TIME, strdate( stime ) );
}


/****************************************************************************/
/* getfield: gets the value of a field and checks for ERROR.                */
/****************************************************************************/

LOCAL char *
getfield( char *field_name )
{
    auto   ASTRING string;
    static char    buffer[BUFFER_SIZE];

#ifdef DEBUG
    debug( "getfield( field_name = \"%s\" )", field_name );
#endif

    string = Egettext( field_name, 0 );
    if (string == (char *) ERROR)
	Efatal( IREADFIELD, "Cannot read field \"%s\"", field_name );

    if (obj_count( string ) >= BUFFER_SIZE)
	Efatal( ITOOLONG, "Field \"%s\" too long", field_name );

    (void) strcpy( buffer, string );

    s_free( string );

    return( buffer );
}


/****************************************************************************/
/* ls_modify: allows the user to modify selected fields in the ls form.     */
/****************************************************************************/

LOCAL int
ls_modify ( ASTRING field_name, int line_number, ASTRING old_line, ASTRING new_line )
/* Name of the modified field           */
/* Number of the line that was modified */
/* The previous contents of the field   */
/* The altered contents of the field    */
{
    register int      i;   /* Index of modified field              */
    auto     ASTRING  tmp_string;
    auto     char     file_name[PATH_MAX];
    static   char    *table[] = {
	F_NAME,
	F_OWNER,
	F_Y,
	F_G,
	F_O,
	F_LAST_MODIFY_TIME
    };

#ifdef DEBUG
    debug( "ls_modify( field_name = \"%s\", line_number = %d, ", field_name,
	line_number );
    debug( "old_line = \"%s\",", old_line );
    debug( "new_line = \"%s\" )", new_line );
#endif

    /* Initialize common variables */

    if (seq ( field_name, F_NAME ))
	(void) strcpy ( file_name, old_line );
    else {
	tmp_string = Egettext( F_NAME, line_number );
	if (tmp_string == (char *) ERROR)
	    Efatal( IREADFIELD, "Cannot read field \"%s\"", F_NAME );
	(void) strcpy( file_name, tmp_string );
	s_free( tmp_string );
    }

    if (is_not_file( file_name )) {
	Eerror( INOFILE, "There is no file for this directory slot" );
	return( TRUE );
    }

    /* Find the altered field */

    for (i = 0; i < (sizeof( table ) / sizeof( char * )); i++)
	if (seq( field_name, table[i] ))
	    break;
    if (i == (sizeof( table ) / sizeof( char * )))
	Efatal( IMODFIELD, "Cannot modify field \"%s\"", field_name );

    switch (i) {

	case 0: /* F_NAME */

	    return( change_name( old_line, new_line, line_number ) );

	case 1: { /* F_OWNER */
	    auto   struct passwd *apw;
	  

	    apw = getpwnam( new_line );
	    if (apw == NULL) {
		Eerror( IBADCHOWN,
			"Unable to change file ownership to \"%s\"", new_line );
		return( TRUE );
	    }

	    if (chown( file_name, apw->pw_uid, ST_GID( file_name ) ) != 0) {
		Eerror( IBADCHOWN,
			"Unable to change file ownership to \"%s\"", new_line );
		return( TRUE );
	    }
	    break;
	}

	case 2:   /* F_Y */
	case 3:   /* F_G */
	case 4:   /* F_O */

	    if (permissions( new_line, file_name, field_name ) == TRUE) {
		unstat( file_name );
		return( TRUE );
	    }
	    break;

	case 5:   /* F_LAST_MODIFY_TIME */

	    if (time_change( new_line, file_name, field_name, line_number )) {
		unstat( file_name );
		return( TRUE );
	    }
	    break;
    }

    unstat( file_name );

    return( FALSE );
}


/****************************************************************************/
/* modec2i: convert a mode string to the corresponding integer.             */
/*   Returns ERROR on conversion failure.                                   */
/****************************************************************************/

LOCAL int
modec2i( char *mode_string )
{
    register int  mode = 0;
    register char *c;

#ifdef DEBUG
    debug( "modec2i( mode_string = \"%s\" )", mode_string );
#endif

    c = mode_string;

    if (*c == 'r') mode = 4;
    else if (*c != '-') return( ERROR );
    c++;

    if (*c == 'w') mode += 2;
    else if (*c != '-') return( ERROR );
    c++;

    if (*c == 'x') mode += 1;
    else if (*c != '-') return( ERROR );
    c++;

    if (*c != '\0') return( ERROR );

    return( mode );
}


/****************************************************************************/
/* modify_stat: allows the user to modify selected fields in the stat form. */
/****************************************************************************/

LOCAL int
modify_stat( ASTRING field_name, int line_number, ASTRING old_line, ASTRING new_line )
/* Name of the modified field           */
/* Number of the line that was modified */
/* The previous contents of the field   */
/* The altered contents of the field    */
{      
    register int             i;                 /* Index of modified field */
    auto     char           *file_name;
    auto     struct passwd  *apw;
    auto     struct group   *agr;
    auto     char            buffer[BUFFER_SIZE]; /* For char * -> int     */
    auto     int             mode;
    auto     int             suid_bit;
    auto     int             sgid_bit;
    auto     int             sticky_bit;
    static   char           *table[] = {
	F_UI,
	F_OWNER,
	F_GI,
	F_GROUP,
	F_Y,
	F_G,
	F_O,
	F_SET_UID,
	F_SET_GID,
	F_STICKY_BIT,
	F_LAST_ACCESS_TIME,
	F_LAST_MODIFY_TIME
    };

#ifdef DEBUG
    debug( "modify_stat( field_name = \"%s\", line_number = %d, ", field_name,
	line_number );
    debug( "old_line = \"%s\",", old_line );
    debug( "new_line = \"%s\" )", new_line );
#endif

    /* Initialize common variables */

    file_name = getfield( F_NAME );
    if (is_not_file( file_name )) {
	Eerror( INOFILE, "There is no file for this directory slot" );
	return( TRUE );
    }

    suid_bit = FALSE;
    sgid_bit = FALSE;
    sticky_bit = FALSE;

    /* Find the altered field */

    for (i = 0; i < (sizeof( table ) / sizeof( char * )); i++)
	if (seq( field_name, table[i] ))
	    break;
    if (i == (sizeof( table ) / sizeof( char * )))
	Efatal( IMODFIELD, "Cannot modify field \"%s\"", field_name );

    switch (i) {

	case  0: { /* F_UI */
	    auto   int            uid;    /* Proposed new user id */
	    extern struct passwd *getpwuid(unsigned long uid);

	    if (sscanf( new_line, "%d", &uid ) != 1) {
		Eerror( IBADUID, "Invalid user id \"%s\".  Must be an integer.",
			new_line);
		return( TRUE );
	    }

	    apw = getpwuid( uid );
	    if (apw ==  NULL) {
		char s [100];

		(void) sprintf (s, "%d", uid);
		Eerror( IBADCHOWN, "Unable to change file ownership to \"%s\"",
			s );
		return( TRUE );
	    }

	    if (geteuid() != ROOT) {

		suid_bit = (ST_MODE( file_name ) & S_ISUID);
		sgid_bit = (ST_MODE( file_name ) & S_ISGID);

		if (suid_bit || sgid_bit)
		    if (!Econfirm( IOWNSET,
"Changing a file's ownership or group will turn off\nthe set-user-ID and set-group-ID bits.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old ownership and group." ))
			return( TRUE );
	    }

	    if (chown( file_name, uid, ST_GID( file_name ) ) == 0)
	    {
		putfield( F_OWNER, apw->pw_name );
	    }
	    else {
		Eerror( IBADCHOWN, "Unable to change file ownership to \"%s\"",
			apw->pw_name );
		return( TRUE );
	    }

	    if (suid_bit)
		putfield( F_SET_UID, "-" );
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;
	}

	case  1: { /* F_OWNER */
	    

	    apw = getpwnam( new_line );
	    if (apw ==  NULL) {
		Eerror( IBADCHOWN, "Unable to change file ownership to \"%s\"",
			new_line );
		return( TRUE );
	    }

	    if (geteuid() != ROOT) {

		suid_bit = (ST_MODE( file_name ) & S_ISUID);
		sgid_bit = (ST_MODE( file_name ) & S_ISGID);

		if (suid_bit || sgid_bit)
		    if (!Econfirm( IOWNSET,
"Changing a file's ownership or group will turn off\nthe set-user-ID and set-group-ID bits.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old ownership and group." ))
			return( TRUE );
	    }

	    if (chown( file_name, apw->pw_uid, ST_GID( file_name ) ) == 0) {
		(void) sprintf( buffer, "%d", apw->pw_uid );
		putfield( F_UI, buffer );
	    }
	    else {
		Eerror( IBADCHOWN, "Unable to change file ownership to \"%s\"",
			new_line );
		return( TRUE );
	    }

	    if (suid_bit)
		putfield( F_SET_UID, "-" );
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;
	}

	case  2: { /* F_GI */
	    auto   int           gid;    /* Proposed new group id */
	    extern struct group *getgrgid (unsigned long gid);

	    if (sscanf( new_line, "%d", &gid ) != 1) {
		Eerror( IBADGID, "Invalid group id \"%s\".  Must be an integer.",
			new_line);
		return( TRUE );
	    }

	    agr = getgrgid( gid );
	    if (agr == NULL) {
		char s [100];

		(void) sprintf (s, "%d", gid);
		Eerror( IBADCHGRP, "Unable to change file group to \"%s\"", s );
		return( TRUE );
	    }

	    if (geteuid() != ROOT) {

		suid_bit = (ST_MODE( file_name ) & S_ISUID);
		sgid_bit = (ST_MODE( file_name ) & S_ISGID);

		if (suid_bit || sgid_bit)
		    if (!Econfirm( IOWNSET,
"Changing a file's ownership or group will turn off\nthe set-user-ID and set-group-ID bits.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old ownership and group." ))
			return( TRUE );
	    }

	    if (chown( file_name, ST_UID( file_name ), gid ) == 0)
	    {
		putfield( F_GROUP, agr->gr_name );
	    }
	    else {
		Eerror( IBADCHGRP, "Unable to change file group to \"%s\"",
			agr->gr_name );
		return( TRUE );
	    }

	    if (suid_bit)
		putfield( F_SET_UID, "-" );
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;
	}

	case  3: { /* F_GROUP */
	   

	    agr = getgrnam( new_line );
	    if (agr ==  NULL) {
		Eerror( IBADCHGRP, "Unable to change file group to \"%s\"",
			new_line );
		return( TRUE );
	    }

	    if (geteuid() != ROOT) {

		suid_bit = (ST_MODE( file_name ) & S_ISUID);
		sgid_bit = (ST_MODE( file_name ) & S_ISGID);

		if (suid_bit || sgid_bit)
		    if (!Econfirm( IOWNSET,
"Changing a file's ownership or group will turn off\nthe set-user-ID and set-group-ID bits.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old ownership and group." ))
			return( TRUE );
	    }

	    if (chown( file_name, ST_UID( file_name ), agr->gr_gid ) == 0) {
		(void) sprintf( buffer, "%d", agr->gr_gid );
		putfield( F_GI, buffer );
	    }
	    else {
		Eerror( IBADCHGRP, "Unable to change file group to \"%s\"",
			new_line );
		return( TRUE );
	    }

	    if (suid_bit)
		putfield( F_SET_UID, "-" );
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;
	}

	case  4: /* F_Y */
	case  5: /* F_G */
	case  6: /* F_O */

	    if (permissions( new_line, file_name, field_name ) == TRUE) {
		fix_ctime( file_name );
		unstat( file_name );
		return( TRUE );
	    }
	    break;

	case  7: /* F_SET_UID */

	    if ((new_line[0] != 's' && new_line[0] != '-') || 
		(new_line[1] != '\0')) {
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
		return( TRUE );
	    }

	    mode = ST_MODE( file_name );
	    if (new_line[0] == 's')
		mode |= S_ISUID;
	    else
		mode &= ~ S_ISUID;

	    if (geteuid() != ROOT) {

		sticky_bit = (ST_MODE( file_name ) & S_ISVTX);

		if (sticky_bit)
		    if (!Econfirm( ISTICKY,
"Changing a file's set_user-ID bit, set_group-ID bit\nor permissions will turn off the sticky bit.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old status." ))
			return( TRUE );
	    }

	    if (geteuid() != ROOT && getegid() != ST_GID( file_name )) {

		sgid_bit = (ST_MODE( file_name ) & S_ISGID);

		if (sgid_bit) {
		    if (!Econfirm( ISGIDOFF,
"Changing the file's permission bits implies\nthat the set-group-ID bit will be turned off.\nTouch EXECUTE to confirm the permission change.\nTouch CANCEL to retain the old permissions." ))
			return( TRUE );
		}
	    }

	    if (chmod( file_name, mode ) != 0) {
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
		return( TRUE );
	    }

	    if (sticky_bit)
		putfield( F_STICKY_BIT, "-" );
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;

	case  8: /* F_SET_GID */

	    if ((new_line[0] != 's' && new_line[0] != '-') ||
		(new_line[1] != '\0')) {
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
		return( TRUE );
	    }

	    mode = ST_MODE( file_name );
	    if (new_line[0] == 's')
		mode |= S_ISGID;
	    else
		mode &= ~ S_ISGID;

	    if (geteuid() != ROOT) {

		sticky_bit = (ST_MODE( file_name ) & S_ISVTX);

		if (sticky_bit)
		    if (!Econfirm( ISTICKY,
"Changing a file's set_user-ID bit, set_group-ID bit\nor permissions will turn off the sticky bit.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old status." ))
			return( TRUE );
	    }

	    if (geteuid() != ROOT && getegid() != ST_GID( file_name )) {

		sgid_bit = (ST_MODE( file_name ) & S_ISGID) | (mode & S_ISGID);

		if (sgid_bit)
		    if (!Econfirm( ISGIDOFF,
"Changing the file's permission bits implies\nthat the set-group-ID bit will be turned off.\nTouch EXECUTE to confirm the permission change.\nTouch CANCEL to retain the old permissions." ))
			return( TRUE );
	    }

	    if (chmod( file_name, mode ) != 0) {
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
		return( TRUE );
	    }
	    if (sticky_bit)
		putfield( F_STICKY_BIT, "-" );
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;

	case  9: /* F_STICKY_BIT */

	    if ((new_line[0] != 't' && new_line[0] != '-') ||
		(new_line[1] != '\0')) {
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
		return( TRUE );
	    }

	    mode = ST_MODE( file_name );
	    if (new_line[0] == 't')
		mode |= S_ISVTX;
	    else
		mode &= ~ S_ISVTX;

	    if (geteuid() != ROOT && getegid() != ST_GID( file_name )) {

		sgid_bit = (ST_MODE( file_name ) & S_ISGID);

		if (sgid_bit) {
		    if (!Econfirm( ISGIDOFF,
"Changing the file's permission bits implies\nthat the set-group-ID bit will be turned off.\nTouch EXECUTE to confirm the permission change.\nTouch CANCEL to retain the old permissions." ))
			return( TRUE );
		}
	    }

	    if (chmod( file_name, mode ) != 0) {
		Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
		return( TRUE );
	    }
	    if (sgid_bit)
		putfield( F_SET_GID, "-" );
	    break;

	case 10: /* F_LAST_ACCESS_TIME */
	case 11: /* F_LAST_MODIFY_TIME */

	    if (time_change( new_line, file_name, field_name, line_number )) {
		unstat( file_name );
		fix_ctime( file_name );
		return( TRUE );
	    }
	    break;
    }

    fix_ctime( file_name );
    return( FALSE );
}


/****************************************************************************/
/* permissions: handle modifications to the F_Y, F_G, and F_O fields.       */
/****************************************************************************/

LOCAL int
permissions( char *new_line, char *file_name, char *field_name )
{
    auto     int             mode;
    auto     ushort          mask;
    auto     ushort          shift;
    auto     int             sgid_bit;
    auto     int             sticky_bit;

#ifdef DEBUG
    debug(
    "permissions( new_line = \"%s\", file_name = \"%s\", field_name = \"%s\" )",
    new_line, file_name, field_name );
#endif

    if ((seq( file_name, ".") || seq( file_name, INDEX_NAME )) &&
	 seq( field_name, F_Y )) {
	Eerror( INOCHANGE,
		"You may not alter the owner's permission bits on this file" );
	return( TRUE );
    }

    if (seq( field_name, F_Y )) {
	mask = 0700;
	shift = 6;
    }
    else if (seq( field_name, F_G )) {
	mask = 0070;
	shift = 3;
    }
    else {
	mask = 0007;
	shift = 0;
    }

    mode = modec2i( new_line );
    if (mode == ERROR) {
	Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
	return( TRUE );
    }

    mode <<= shift;
    mode |= (ST_MODE( file_name ) & (~ mask));

    sgid_bit = FALSE;
    sticky_bit = FALSE;

    if (geteuid() != ROOT) {

	 sticky_bit = (ST_MODE( file_name ) & S_ISVTX);

	 if (sticky_bit)
	     if (!Econfirm( ISTICKY,
"Changing a file's set_user-ID bit, set_group-ID bit\nor permissions will turn off the sticky bit.\nTouch EXECUTE to confirm the change.\nTouch CANCEL to retain the old status." ))
		 return( TRUE );
    }

    if (geteuid() != ROOT && getegid() != ST_GID( file_name )) {

	sgid_bit = (ST_MODE( file_name ) & S_ISGID);

	if (sgid_bit)
	    if (!Econfirm( ISGIDOFF,
"Changing the file's permission bits implies\nthat the set-group-ID bit will be turned off.\nTouch EXECUTE to confirm the permission change.\nTouch CANCEL to retain the old permissions." ))
		return( TRUE );
     }

    if (chmod( file_name, mode ) != 0) {
	Eerror( IBADCHMOD, "Unable to change modes to \"%s\"", new_line );
	return( TRUE );
    }

    if (sticky_bit)
	putfield( F_STICKY_BIT, "-" );
    if (sgid_bit)
	putfield( F_SET_GID, "-" );

    return( FALSE );
}


/****************************************************************************/
/* strdate: convert a time_t (as returned by stat) to a character string    */
/*   suitable for printing.                                                 */
/****************************************************************************/

char * strdate( time_t longtime )
{
    char              tmbuf[BUFFER_SIZE], final_fmt[BUFFER_SIZE];
    char              final_date[BUFFER_SIZE];
    static char       date_buffer[BUFFER_SIZE];

#ifdef MAXDEBUG
    debug( "strdate( longtime = %ld )", longtime );
#endif

    /* Convert date and time to string */
    setlocale(LC_ALL, "");
    strcpy((char *)final_fmt, (char *)get_datefmt());
    strftime(final_date, BUFFER_SIZE, final_fmt, localtime(&longtime));
    (void) strcpy(date_buffer,final_date); 
    return( date_buffer );
}


char *get_datefmt()
{
    char *no_a;
    char *no_z;
    char *init_fmt;
    char *fmt_var;
    char fmt[100];

    init_fmt = nl_langinfo(D_T_FMT);

    fmt_var = "%Z";
    strcpy(fmt,strip_date(fmt_var, init_fmt)); 
    fmt_var = "%a";
    strcpy(fmt,strip_date(fmt_var, fmt));

    return((char *)fmt);
}

char *strip_date(char *fmt_rmv, char *tmp_date)
{
    int firstlength, totfirst, totlast, endlength, datelngth;
    char *ptrlocn, *last_buf;
    static char first_buf[100];

    firstlength = 0;
    totfirst = 0;
    totlast = 0;
    endlength = 0;
    memset(first_buf, '\0', 100);

    if ((ptrlocn = strstr(tmp_date, fmt_rmv)) != '\0')
       {
       endlength = strlen(ptrlocn); 
       firstlength = strlen(tmp_date);
       totlast = endlength - 2;
       totfirst = firstlength - endlength;
       strncpy(first_buf, tmp_date, totfirst); 
       last_buf = ptrlocn+2;
       if (isspace((int)last_buf[0]))
          strncat(first_buf, last_buf+1, totlast); 
       else
          strncat(first_buf, last_buf, totlast); 
       }
    else
       {
       strcpy(first_buf,tmp_date);
       }
    return(first_buf);
}

/****************************************************************************/
/* parsedate: parses a date field of the format which is printed by strdate */
/*    This procedure is used when the user modifies the last access or last */
/*    modification time of a file.  The user must enter the modified time   */
/*    in the same format as printed by strdate.                             */
/****************************************************************************/

struct tm *parsedate( char *date_buffer )
{
    char mon[20 * MB_LEN_MAX];
    char              final_date[BUFFER_SIZE];
    char              final_fmt[BUFFER_SIZE];
    int  month, day, year, hour, minute, second;
    static struct tm tm;

#ifdef MAXDEBUG
    debug( "parsedate( date_buffer = \"%s\" )", date_buffer );
#endif
    
    /* Convert date and time to string */
    setlocale(LC_ALL, "");
    strcpy((char *)final_fmt, (char *)get_datefmt());
    if (strptime(date_buffer, final_fmt, &tm) == NULL)
        return((struct tm *)ERROR);
    else
        return( &tm );
}


/****************************************************************************/
/* time_change: handle modifications to the F_LAST_ACCESS_TIME and          */
/*   F_LAST_MODIFY_TIME fields.                                             */
/****************************************************************************/

LOCAL int
time_change( char *new_line, char *file_name, char *field_name, int line_number )
{
    auto struct tm      *atm;
    auto time_t          time_value;
    auto time_t          new_time;
    auto struct  {
	time_t actime;
	time_t modtime;
    } utimbuf;

    auto int             is_access_time;
    auto time_t         *anewfield;
    auto time_t         *aotherfield;

#ifdef DEBUG
    debug( "time_change( new_line = \"%s\", file_name = \"%s\", field_name = \"%s\", line_number = %d )",
	   new_line, file_name, field_name, line_number );
#endif

    is_access_time = (seq( field_name, F_LAST_ACCESS_TIME ));
    if (is_access_time) {
	anewfield = &utimbuf.actime;
	aotherfield = &utimbuf.modtime;
    }
    else {
	anewfield = &utimbuf.modtime;
	aotherfield = &utimbuf.actime;
    }

    if (is_now( new_line ))
	new_time = time ((long *) 0);
    else {
	atm = parsedate( new_line );
	if (atm == (struct tm *) ERROR) {
	    Eerror( IBADTIME, "Invalid time specification \"%s\"", new_line );
	    return( TRUE );
	}
	new_time = mktime( atm );
    }

    time_value = ((is_access_time) ? ST_ATIME( file_name ) :
				     ST_MTIME( file_name ));
    if (time_value == (time_t) ERROR)
	Efatal( ISTAT, "Cannot stat file \"%s\"", file_name );

    *anewfield = new_time;
    *aotherfield = time_value;

    if (utime( file_name, (struct utimbuf *)&utimbuf ) != 0) {
	Eerror( IUTIME, "File %s: cannot reset time to \"%s\"",
		(is_access_time) ? "access" : "modify", new_line );
	return( TRUE );
    }

    if (Eputtext( field_name, line_number, strdate( new_time )) == ERROR)
	Efatal( IWRITEFIELD, "Cannot write field \"%s\"", field_name );

    return( FALSE );
}
