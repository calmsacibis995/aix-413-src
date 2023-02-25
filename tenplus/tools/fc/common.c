#if !defined(lint)
static char sccsid[] = "@(#)66	1.11  src/tenplus/tools/fc/common.c, tenplus, tenplus411, GOLD410 3/3/94 16:12:10";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   compile, corbox, corelate, findboxes,
 *		get_invtext, getattr, getpick, gettab, getzoom,
 *              gprefix , gsuffix, gutboxes, indexmode,
 *		makename, validname, meaningful, write_windows
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *                      Copyrighted as an unpublished work.                   
 *                         INTERACTIVE TEN/PLUS System                        
 *                          TEN/PLUS Forms Definition                         
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, 1988            
 *                             All rights reserved.                           
 *                                                                            
 *   RESTRICTED RIGHTS                                                        
 *   These programs are supplied under a license.  They may be used and/or    
 *   copied only as permitted under such license agreement.  Any copy must    
 *   contain the above notice and this restricted rights notice.  Use,        
 *   copying, and/or disclosure of the programs is strictly prohibited        
 *   unless otherwise provided in the license agreement.                      
 *                                                                            
 *   TEN/PLUS is a registered trademark of INTERACTIVE Systems Corporation    
 *	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
 */
/*****************************************************************************
* File: common.c - functions common to the helper and the forms compiler.    *
*****************************************************************************/


#if !defined(lint)
static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
#endif /* lint */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chardefs.h"
#include "database.h"
#include "libobj.h"
#include "libshort.h"
#include "libutil.h"
#include "window.h"
#include "fc.h"
#include "box.h"

/* maximum height and width for a form */
#define MAXHEIGHT  24
#define MAXWIDTH   80

#define ONEx1      '`'     /* character for one-by-one boxes           */
#define ONExN      '\\'    /* character for one-by-n boxes             */

char *g_filename;          /* Name of the .frm file being compiled      */
char  g_tv[1];             /* fake tv screen array; used by window code */

static char    *data_path;       /* data path for one field of the form  */

static ATTR   **invariant_text;  /* window 0 invariant text */
static BOX     *previous_box;    /* previous box looked at by corrbox used
				   to ensure that ganging meets criteria */

static int      text_count;      /* number of lines in text[]    */
static int      Tab_flag;        /* TRUE if room for tabs on top line */
static int      warn1;
static int      warn2;

static int      corbox(BOX * , POINTER *);
static POINTER *corelate( POINTER *, SFILE *);
static ATTR   **get_invtext(SFILE *, int *);
static void    getattr(POINTER * , BOX *);
static int     getpick(POINTER * , BOX *);
static int     gettab(POINTER * , BOX *);
static int     getzoom( POINTER *, BOX *);
static char    *gprefix (POINTER *);
static int     gutboxes(void);
static char    *gsuffix(void);
static int     indexmode (void);
static char    *makename (char *);
static int     validname (char *);
static int     meaningful( char *);
static int     write_windows( POINTER *, SFILE *);
static int     gettxt( POINTER *, BOX * );
static int     getmsgno( POINTER *, BOX * );

/*****************************************************************************
* compile: convert the .frm file to the .ofm file.                           *
*   Name of the .frm file is in g_filename.                                  *
*   Returns ERROR on failure; otherwise RET_OK.                              *
*****************************************************************************/

int compile(void)
{
    SFILE    *isfp;        /* SFILE pointer for .frm file     */
    SFILE   *osfp;         /* SFILE pointer for .ofm file     */
    POINTER *boxes;        /* Vector of field information     */

    char    *ofname;       /* Name of the .ofm file           */
    int      result;       /* Result of call on write_windows */

#ifdef DEBUG
    debug( "compile: g_filename = \"%s\"", g_filename );
#endif

    /* Open .frm file */
    isfp = sf_open( g_filename, "r" );
    if (isfp == (SFILE *) ERROR) {
	Efatal( "Cannot open .frm file \"%s\" for reading",
               g_filename );
	return( ERROR );
    }

    /* Create the name of the .ofm file */
    ofname = makename( g_filename );
    if (ofname == (char *) ERROR) {
	return( ERROR );
    }

    /* Open .ofm file */
    osfp = sf_open( ofname, "c" );
    if (osfp == (SFILE *) ERROR) {
	(void) sf_close( isfp );
	error( "Cannot create .ofm file \"%s\"", ofname );
	return( ERROR );
    }

    /* Understand the invariant text, make up vector of boxes   */
    boxes = findboxes( isfp );
    if (boxes == (POINTER *) ERROR) {
	(void) sf_close( isfp );
	(void) sf_close (osfp);
	(void) unlink( ofname );
	s_free( ofname );
	return( ERROR );
    }
#ifdef  DEBUG
    debug( "compile: after findboxes: boxes :-");
    s_print( boxes );
#endif

    /* Gut out the boxes to make window 0 invariant text */
    if (gutboxes() == ERROR) {
	(void) sf_close( isfp );
	(void) sf_close (osfp);
	(void) unlink( ofname );
	s_free( ofname );
	return( ERROR );
    }
#ifdef  DEBUG
    debug( "compile: after gutboxes: invariant_text :-");
    s_print( invariant_text );
#endif

    /* correlate boxes with per-field data, modifying ganging
	information in light of indexing                         */

    boxes = corelate( boxes, isfp );
    if (boxes == (POINTER *) ERROR) {
	(void) sf_close( isfp );
	(void) sf_close (osfp);
	(void) unlink( ofname );
	s_free( ofname );
	return( ERROR );
    }

#ifdef DEBUG
    debug( "compile: after corelate: boxes = %#x", boxes );
    s_print( boxes );
#endif

    /* Write out the structure, and free ours for another pass  */
    result = write_windows( boxes, osfp );
    if (result == ERROR)
	(void) unlink( ofname );

    s_free( (char *) boxes );
    s_free( ofname );

    (void) sf_close( isfp );
    (void) sf_close( osfp );

    return( result );
}

/*****************************************************************************
* corbox: correlate a single box with its per-field information.             *
*****************************************************************************/

static int corbox(BOX *box, POINTER *field )
{
    int   bits;         /* INDEXED, HGANGED, SHOWTABS, NAMEFIELD */
    char *prefix;       /* data path                             */
    char *suffix;       /*   "   "                               */

    /* gprefix, indexmode, and suffix must be called in this order,
       since they share datapath.                                   */

    prefix = gprefix( field );
    if (prefix != NULL) {

	/* check for [FIX] from default-maker which wasn't fixed */
	if (*prefix == '[') {
	    error( "Please fix description of field \"%s\"",
				obj_name( box ) );
	    s_free( prefix );
	    return( ERROR );
	}

	bits = indexmode();
	if (bits == ERROR) {
	    s_free( prefix );
	    return( ERROR );
	}

	/* namefield's don't have suffixes  */
	if ((bits & INDEXED) == INDEXED) {
	    suffix = gsuffix();
	    if (suffix == (char *) ERROR) {
		s_free( prefix );
		return( ERROR );
	    }
	}
	else
	    suffix = NULL;

	if (strlen( prefix ) == 0) {
	    s_free( prefix );
	    prefix = NULL;
	}
    }
    else {   /* no prefix, set everything to NULL */
	prefix = NULL;
	suffix = NULL;
	bits = 0;
    }

    /* only gang into and out of indexed fields */
    if ((box->bits & HGANGED) && !(bits & INDEXED)) {
	if (warn1 == 0) {
	    error( "Warning: spatially-ganged non-indexed field \"%s\"",
		   obj_name( box ) );
	    warn1++;
	}

	box->gangp = NULL;
	box->bits &= ~HGANGED;
	if (previous_box && (previous_box->bits & HGANGED))
	    previous_box->gangp = NULL;
    }

    /* some consistency checking - prefix must be the same for ganging  */
    /* refuse to gang some, but not all of a spatial gang               */

    /* if spatially ganged                                              */
    /* and both have prefixes, which don't match OR                     */
    /* one has a prefix and the other doesn't                           */

    /* then, split the ganging at this point. This isn't the most
       conservative strategy, but it does allow for side-by-side,
       independent gangs.                                               */

    if ((box->bits & HGANGED) && 
      previous_box && (previous_box->bits & HGANGED) &&
      !seq (previous_box->prefix, prefix) &&
      box->top == previous_box->top && box->bot == previous_box->bot) {
	if (warn2 == 0) {
	    error( "Spatially ganged fields \"%s\" and \"%s\" with different prefixes",
		obj_name( previous_box ), obj_name( box ) );
	    warn2++;
	}
	previous_box->gangp = NULL;
	previous_box->bits &= ~HGANGED;
    }

    box->prefix = prefix;
    box->suffix = suffix;

    if (box->bits & HGANGED)
	bits |= HGANGED;

    /* preserve the INVISIBLE bit, which is set before this routine is
       called. That, in turn, prevents complaints about a missing pick-name
       for invisible fields.                                            */

    if (box->bits & INVISIBLE)
	box->bits = bits | INVISIBLE;
    else
	box->bits = bits;

    if (getzoom( field, box ) == ERROR)
	return( ERROR );

    if (getpick( field, box ) == ERROR)
	return( ERROR );

    if (gettab( field, box ) == ERROR)   /* puts tabs and margins into box */
	return( ERROR );

    if (getmsgno( field, box ) == ERROR)   
	return( ERROR );

    if (gettxt( field, box ) == ERROR)   
	return( ERROR );


    getattr( field, box );

    if (bits & HGANGED)
	previous_box = box;
    else
	previous_box = NULL;

    return( RET_OK );
}

/*****************************************************************************
* corelate: correlate boxes with per-field data, modifying ganging           *
*   information in light of indexing.                                        *
*****************************************************************************/

static POINTER *corelate( POINTER *boxes, SFILE *sfp )
{
    POINTER *fields;
    POINTER *field;
    BOX     *box;

    int      i;
    char    *invisible;
    char    *fieldname;

    /* this will suppress multiple messages announcing curious
       horizontal gangs                                        */

    warn1 = warn2 = 0;
    previous_box = NULL;

    /* the order of appearance of the fields and invtext records
       is probably random, so look for it                        */

    fields = (POINTER *) ERROR;

    for (i = 0; i < obj_count (sf_records (sfp)); i++) {

	fields = (POINTER *) sf_read (sfp, i);
	if (fields == (POINTER *) ERROR)
	    break;

	if (fields == NULL)
	    continue;

	if ((strcmp( obj_name( fields ), "fields" ) == 0) && 
	    (obj_count( fields ) != 0))
	    break;

	s_free( (char *) fields );
	fields = (POINTER *) ERROR;
    }

    if (fields == (POINTER *) ERROR) {
	error( "Cannot find per-field data record" );
	s_free( (char *) boxes );
	return( (POINTER *) ERROR );
    }

    for (i = 0; i < obj_count (boxes); i++) {
	box = (BOX *) boxes[i];
	if (i >= obj_count (fields))
	    fieldname = (char *)ERROR;
	else {
	    field = (POINTER *) fields[i];
	    fieldname = s_findnode( field, "name" );
	}
	if (fieldname == (char *) ERROR) {
	    error ( "Field description number %d doesn't have a name",
		   i + 1);
	    s_free( (char *) fields );
	    s_free( (char *) boxes );
	    return( (POINTER *) ERROR );
	}

	/* tripped over leftover [FIX] from default-maker? */
	if (*fieldname == '[') {
	    error( "Please fix the description of field %d", i + 1 );
	    s_free( (char *) fields );
	    s_free( (char *) boxes );
	    return( (POINTER *) ERROR );
	}

	/* if it's an unnamed box, or the name matches properly
	   forge ahead, otherwise it's an error...              */

	if (strlen( obj_name( box ) ) == 0)
	    s_newname( (char *) box, fieldname );

	/* fields that have boxes better not be invisible   */
	if (strcmp( obj_name( box ), fieldname ) == 0) {
	    invisible = s_findnode( field, "invisible" );
	    if (meaningful( invisible )) {
		error( "Field \"%s\" has a box but is also marked invisible",
		       fieldname );
		s_free( (char *) fields );
		s_free( (char *) boxes );
		return( (POINTER *) ERROR );
	    }

	    if (corbox( box, field ) == ERROR) {
		s_free( (char *) fields );
		s_free( (char *) boxes );
		return( (POINTER *) ERROR );
	    }
	}
	else {
	    error( "No description found for field \"%s\"",
		   obj_name( box ) );
	    s_free( (char *) fields );
	    s_free( (char *) boxes );
	    return( (POINTER *) ERROR );
	}
    }

    /* see if there are invisible fields */
    if (obj_count( boxes ) < obj_count( fields )) {
	for (i = 0; i < obj_count( fields ); i++) {
	    invisible = s_findnode((POINTER *) fields[i], "invisible" );

	    if (meaningful( invisible )) {
		fieldname = s_findnode((POINTER *) fields[i], "name" );
		if (fieldname == (char *) ERROR) {
		     error( "Description of invisible field number %d doesn't have a name",
			    i + 1);
		     s_free( (char *) fields );
		     s_free( (char *) boxes );
		     return( (POINTER *) ERROR );
		}
		box = (BOX *) s_alloc( 1, T_BOX, fieldname );
		boxes = s_append( boxes, (char *)box );

		box->bits = INVISIBLE;

		if (corbox( box, (POINTER *) fields[i] ) == ERROR) {
		    s_free ((char *) fields);
		    s_free ((char *) boxes);
		    return ((POINTER *) ERROR);
		}
	    }
	}
    }

    if (obj_count( boxes ) != obj_count( fields )) {
	error( "Number of boxes (%d) doesn't match number of descriptions (%d)",
	       obj_count( boxes ), obj_count( fields ) );
	s_free( (char *) fields );
	s_free( (char *) boxes );
	return( (POINTER *) ERROR );
    }

    s_free( (char *) fields );
    return( boxes );
}

/*****************************************************************************
* findboxes: Takes .frm file SFILE pointer, and produces a vector of boxes.  *
*****************************************************************************/

POINTER *findboxes(SFILE * sfp )
{
    POINTER *boxes;        /* Vector of field data                  */
    BOX     *prevbox;      /* Pointer to box for last field found   */
    BOX     *thisbox;      /* BOX for field being examined */

    ATTR    fieldname[MAXWIDTH]; /* Text within []            */
    ATTR    **text;        /* Content of invtext field in .frm file */
    ATTR    *cp;           /* Points at text[i]            */

    int     i;             /* Indexes into text            */
    int     prevtop;       /* Row of last field found      */
    int     prevbot;       /* Column of last field found   */
    int     j;             /* Points into text[i]          */
    int     k;             /* Indexes across field         */
    int     top, left, bot, right;
    int     onex1;         /* TRUE if the field is ONEx1   */
    int     fieldfound;    /* TRUE if a field is found     */
    int     bad_name;
    char    *buff;         /* holds multibyte form of fieldname */
    char    ch;

#ifdef DEBUG
    debug( "findboxes( sfp = %#x )", sfp );
#endif

    text = get_invtext(sfp, &text_count);
    if (text == (ATTR **) ERROR)
	return( (POINTER *) ERROR);

    boxes = (POINTER *) s_alloc( 0, T_POINTER, "box-vector" );

    prevtop = 0;
    prevbot = 0;
    prevbox = NULL;

    /* for all the lines */
    for (i = 0; i < text_count; i++)
    {
	cp = text[i];
#ifdef	MAXDEBUG
	debug("findboxes: inside outer loop (%d)", i);
#endif

	/* Search across the line for the beginning of all fields */
	for (j = 0; j < MAXWIDTH; j++)
        {
#ifdef	MAXDEBUG
	debug("findboxes: inside inner loop (%d)", j);
#endif
	    fieldname[0] = '\0';
	    fieldfound = TRUE;
	    top = i;
	    left = j;
            /* make sure to ignore subsequent bytes of multibyte chars */
            while (!firstbyte(cp[j]))
                j++;
	    /* Switch on type of fields: multi-column, ONEx1, or ONExN */

	    /*
	     * multi-column:
	     *   bracket followed by alphabetic, or
	     *   bracket followed by space or matching bracket has no
	     *   bracket above it and no bracket above '['
	     */

	    if (de_attr(cp[j]) == '[')
            {   /* fetch the field name */
#ifdef	DEBUG
	        debug("findboxes:\t inner loop - '[' found at posn. %d", j);
#endif
		onex1 = bad_name = FALSE;
		for (k = 0; k < MAXWIDTH; k++)
                {
		    fieldname[k] = cp[j + k + 1];
                    if (!firstbyte(fieldname[k]))
                        continue;
                    ch = de_attr(fieldname[k]);
		    if ((ch == ']') || isspace(ch))
                    {
			fieldname[k] = '\0';
			break;
		    }
		    /* field names with ` in them cause problems */
		    if (ch == ONEx1)
			bad_name = TRUE;
		}

		if (k == MAXWIDTH)
                {
                    while (!firstbyte(fieldname[k-1]))
                        k--;
		    fieldname[k - 1] = '\0';
		    error( "Unreasonable field name \"%s\"", fieldname );
		    s_free( (char *) text );
		    s_free( (char *) boxes );
		    return( (POINTER *) ERROR );
		}

		if (bad_name)
                {
		    error ("Back quotes not allowed in field names (%s)",
						    fieldname);
		    fieldname[0] = '\0';
		    s_free( (char *) text );
		    s_free( (char *) boxes );
		    return( (POINTER *) ERROR );
		}

		/* look across to find its width */
		for (k = j + 1; k < MAXWIDTH; k++)
		    if (firstbyte(cp[k]) && (de_attr(cp[k]) == ']'))
			break;

		if (k == MAXWIDTH)
                {
		    s_free( (char *) text );
		    s_free( (char *) boxes );
		    error( "Field \"%s\" is ill-defined (no ']' was found)",
						fieldname );
		    return( (POINTER *) ERROR );
		}

		right = k;

		/* See if the field is a continuation of the one above */
                ch = de_attr(cp[j+1]);
		if ((i > 0)                        &&
		    (de_attr(text[i-1][j]) == '[') &&
		    (de_attr(text[i-1][k]) == ']') &&
                    !isalnum(ch) && ch != '_' && ch != '-')
		    continue;

		/* now look down to find it's height */
		for (k = i + 1; k < text_count; k++)
		    if ((de_attr(text[k][j]) != '[')         ||
			(!isspace(de_attr(text[k][j+1])) &&
			   (de_attr(text[k][j+1])  != ']'))  ||
			(de_attr(text[k][right]) != ']'))
			break;
		bot = k - 1;
	    }

	    /* start of 1x1 box */
	    else if (de_attr(cp[j]) == ONEx1)
            {
#ifdef	DEBUG
	        debug("findboxes:\t inner loop - 1 x 1 box at posn. %d", j);
#endif
		onex1 = TRUE;
		bot   = i;
		right = j;
	    }
	    /* start of 1xN box (ONExN and prev. line isn't part of 1xN box */
	    else if ((de_attr(cp[j]) == ONExN) &&
		     ((i == 0) || (de_attr(text[i-1][j]) != ONExN)))
            {
#ifdef	DEBUG
	        debug("findboxes:\t inner loop - 1 x N box at posn. %d", j);
#endif
		onex1 = FALSE;

		/* look down to find it's height */
		for (k = i + 1; k < text_count; k++)
		    if (de_attr(text[k][j]) != ONExN)
			break;
		bot = k - 1;
		right = j;
	    }
	    else
		fieldfound = FALSE;

	    if (fieldfound)
            {   /* Allocate a new box */
		buff = packup( fieldname );
		thisbox = (BOX *) s_alloc( 1, T_BOX, buff );
		s_free (buff);

		/* Connect with previous members of a gang, if any */
		if (onex1)  /* 1x1 boxes never gang */
		    prevbox = NULL;
		else
                {   /* gang with previous field? */
		    if ((prevbox != NULL)   &&
			(prevtop == top)    &&
			(prevbot == bot)    &&
			((bot - top) > 0))
		    {
			prevbox->gangp = thisbox;
			prevbox->bits |= HGANGED;
			thisbox->bits |= HGANGED;
		    }

		    /* save top, bot, thisbox for ganging */
		    prevbox = thisbox;
		    prevtop = top;
		    prevbot = bot;
		}

		/* Store the information in the struct */
		thisbox->top   = top;
		thisbox->left  = left;
		thisbox->bot   = bot;
		thisbox->right = right;
		thisbox->gangp = NULL;

		/* field declared to lie in a graphics box if
		   char to the left of the '[' is a vertical bar */

		thisbox->isboxed = (j > 0 && cp[j - 1] == G_VLINE);
		boxes = s_append((POINTER *) boxes, (char *)thisbox );
#ifdef	DEBUG
		debug("findboxes:\t inner loop - new field found: boxes now:-");
		s_print( boxes );
#endif
	    }
	}

    }   /* loop over all lines  */

    invariant_text = text;
#ifdef	DEBUG
    debug("findboxes: invariant_text is :-");
    s_print( invariant_text );
#endif
    return( boxes );
}

/*****************************************************************************
* get_invtext: reads in the invtext field from the .frm file.                *
*   Returns (ATTR **) ERROR on I/O error; else the text is a wchar_t **   *
*   the returned characters are always in internal format!!!!                *
*   Replaces NULL fields with empty strings.                                 *
*   Converts tabs to spaces and graphics characters to internal              *
*   representation.                                                          *
*   Sets the Tab_flag file static if the top line of the text is blank.      *
*****************************************************************************/

static ATTR **get_invtext(SFILE *sfp, int *text_count1)
{
    ATTR   **text;       /* text to be returned in internal format */
    ATTR   *cp;          /* unpacked version of each line i.e. text[i] */
    int    i;            /* Counts lines of invariant text  */

    /*
     * Get the invariant text array
     *   the order of appearance of the fields and invtext records
     *   is probably random, so look for it
     */

#ifdef DEBUG
    debug( "get_invtext: entry " );
#endif

    *text_count1 = 0;
    text = (ATTR **) ERROR;

    for (i = 0; i < obj_count( sf_records( sfp ) ); i++) {

	text = (ATTR **) sf_read( sfp, i );
	if (text == NULL)
	    continue;

	if (strcmp( obj_name( text ), "invtext" ) == 0)
	    break;

	s_free( (char *) text );
	text = (ATTR **) ERROR;
    }

    if (text == (ATTR **) ERROR) {
	error( "No invariant text was found" );
	return( (ATTR **) ERROR );
    }

    /* 1 line forms will look all wrong... */
    if (obj_type( text ) == T_CHAR)
	text = (ATTR **) s_pointer( (char *) text );

    /* get the untabbed version */
    for (i = 0; i < obj_count( text ); i++)
    {
	(*text_count1)++;

	if (text[i] == NULL)
	    text[i] = (ATTR *) s_alloc( 0, T_ATTR, NULL );

	cp = unpackup( (char *) text[i], MAXWIDTH );
	if ((obj_count (cp) - 1) > MAXWIDTH)
        {
	    error( "Line %d of the invariant text is too wide", i );
	    s_free( (char *) text );
	    return( (ATTR **) ERROR );
	}
	s_free((char *) text[i] );
	text[i] = cp;
    }

    /* if the first line of invariant text for window 0 is all
       whitespace, allow display of tab/margin line            */
    Tab_flag = SHOWTABS;
    for (cp = text[0]; *cp != '\0' && isspace(de_attr(*cp)); cp++)
        /* null statement */ ;
    if (*cp != '\0')
	    Tab_flag = 0;

    return( text );
}

/*****************************************************************************
* getattr: set up the NAMEFIELD  and DOTTED bits in w_flags.                 *
*****************************************************************************/

static void getattr(POINTER *field, BOX *box )
{
    char *namefield;
    char *dotted;
    char *doumod;
    char *userro;
    char *crscroll;
    char *changehelper;
    char *displayonly;
    char *catentry;

    namefield = s_findnode( field, "namefield" );
    if (meaningful( namefield ))
	box->bits |= NAMEFIELD;

    dotted = s_findnode( field, "dotted" );
    if (meaningful( dotted ))
	box->bits |= DOTTED;

    doumod = s_findnode( field, "doumod" );
    if (meaningful( doumod ) )
	box->bits |= DOUMOD;

    userro = s_findnode( field, "userro" );
    if (meaningful( userro ))
	box->bits |= USERRO;

    crscroll = s_findnode( field, "crscroll" );
    if (meaningful( crscroll ))
	box->bits |= CRSCROLL;

    changehelper = s_findnode( field, "changehelper" );
    if (meaningful( changehelper ))
	box->bits |= CHANGEHELPER;


}

/*****************************************************************************
* getpick: set up the pick-name in the box struct, or set it to NULL         *
*****************************************************************************/

static int getpick(POINTER *field, BOX *box )
{
    char *pick;

    pick = s_findnode( field, "pick-name" );

    /* use his favorite pick-name, or copy the zoom form name, or complain for
       indexed, visible fields                                          */

    if (meaningful( pick ))
	box->pickname = s_string( pick );
    else if (box->zoom != NULL)
	box->pickname = s_string( box->zoom );
    else {
	if (box->bits & INDEXED) {

	    /* allow null pick-name for invisible fields */
	    if (! (box->bits & INVISIBLE)) {
		error( "Field \"%s\" needs to have a pick-name specified",
		       obj_name( box ) );
		return( ERROR );
	    }
	}
	box->pickname = NULL;
    }
    if (previous_box && (box->bits & HGANGED) &&
      (previous_box->bits & HGANGED) &&
      !seq (box->pickname, previous_box->pickname) &&
      seq (box->prefix, previous_box->prefix) &&
      box->top != previous_box->top &&
      box->bot != previous_box->bot)
	error ( "Ganged fields \"%s\" and \"%s\" don't have the same pick name",
	       obj_name ( previous_box ), obj_name ( box ) );
    return( RET_OK );
}

/*****************************************************************************
* gettab: return and check tab line, break out left and right margin         *
* positions if specified. The margins and tab line are directly installed    *
* into the box struct, to save some multiple return code.                    *
*****************************************************************************/

static int gettab(POINTER *field, BOX *box )
{
    int   i;
    ATTR *cp;
    char *pkcp;
    int   tabflag;
    int   nblnks;

#ifdef DEBUG
    debug( "gettab: entry " );
#endif

    /* set up default return */
    box->lmarg = box->rmarg = -1;
    box->tablin = NULL;
    tabflag = FALSE;    /* says at least 1 tab seen */

    pkcp = s_findnode( field, "tabs" );
    if (pkcp == (char *) ERROR) {   /* no tabs is an acceptable idea */
	box->lmarg = box->rmarg = 0;
	return( RET_OK );
    }

    /* get allocated, MAXWIDTH wide, untabbed version */
    cp = unpackup( pkcp, MAXWIDTH );
    for (i = 0; i < obj_count (cp) - 1; i++) {
	switch ((char) de_attr(cp[i])) {

	    case 'l':
		if (box->lmarg != -1)
		    error( "Warning: field \"%s\" has multiple left margins",
			obj_name( box ) );
		else
		    box->lmarg = i;
		cp[i] = SPACE;
		break;

	    case 'r':
		if (box->rmarg != -1)
		    error( "Warning: field \"%s\" has multiple right margins",
		       obj_name( box ) );
		box->rmarg = i;
		cp[i] = SPACE;
		break;

	    case 't':
		tabflag = TRUE;
		break;

	    case SPACE:
		break;

	    default:
		error( "Bad character '%c' in tab line for field \"%s\"",
		       cp[i], obj_name( box ) );
		s_free((char *) cp );
		return( ERROR );
	}
    }

    /* some additional consistency checking.
	(1) no margins in 1 line fields
	(2) no left margin without a right margin
	(3) no right margin in last column              */

    if (box->top == box->bot) {
	if (box->lmarg != -1 || box->rmarg != -1) {
	    error( "Margins are not allowed in one line field \"%s\"",
		   obj_name( box ) );
	    s_free((char *) cp );
	    return( ERROR );
	}
    }

    if (box->lmarg != -1 && box->rmarg == -1) {
	error( "A left margin is not allowed without a right margin in field \"%s\"",
	       obj_name( box ) );
	s_free((char *) cp );
	return( ERROR );
    }

    if ( ( (box->bits & HGANGED) || (box->bits & INDEXED) ) &&
	 ( (box->lmarg != -1) || (box->rmarg != -1) ) )
	{
	error ( "Margins not permitted in list fields");
	return (ERROR);
	}

    if (tabflag)
	{
	/* calculate number of leading blanks before packup compresses them */
	for (nblnks = 0; nblnks < (obj_count (cp) - 1) && cp[nblnks] == SPACE; nblnks++);
	nblnks = nblnks / 8;
	pkcp = packup(cp);

	/* if more than eight blanks must undo packup's work */
	if (nblnks)
	    {

	    /* put blanks back in string that packup converted to tabs */
	    pkcp = s_insert (pkcp, 0, nblnks * 7);
	    smear (SPACE, nblnks * 8, pkcp);
	    }
	box->tablin = pkcp;
	s_free ((char *)cp);
	}
    else
	{
	box->tablin = NULL;
	s_free((char *)cp );
	}

    return( RET_OK );
}

/****************************************************************************
* getmsgno: gets the message number an converts it to a real integer and    *
*           sets the CATENTRY flag.                                         * 
*****************************************************************************/

static int getmsgno( POINTER *field, BOX *box )
{
char *msgtxt;

	msgtxt = s_findnode(field, "cat-msgno");

	if (msgtxt != (char *)ERROR)
	{
	   if (meaningful (msgtxt))
           {
	      box->bits |= CATENTRY;
	      box->msgno = atoi(msgtxt);
	      return(RET_OK);
           }
	   return(ERROR);
	}   
	return(RET_OK);
}

/****************************************************************************
* gettxt: gets the default text from the form or uses a standard message.   *
*****************************************************************************/
static int gettxt( POINTER *field, BOX *box )
{
char *deftxt;


	deftxt = s_findnode(field,"cat-deftxt");
   
	if (deftxt != (char *)ERROR)
	{
	    if (meaningful (deftxt))
	    {
	        box->deftxt = s_string(deftxt);
	        return(RET_OK);
	    }
	    else
	        box->deftxt = s_string("fc: No default text from tosf");
	        if (meaningful (box->deftxt))
	            return(RET_OK);

            return(ERROR);
        }
	return(RET_OK);

}
/*****************************************************************************
* getzoom: set up the zoom form name in the box struct, or set it to NULL.   *
*****************************************************************************/

static int getzoom( POINTER *field, BOX *box )
{
    char *zoom;
    char *filename;

    zoom = s_findnode( field, "no-zoom" );
    if (meaningful( zoom )) {
	box->bits |= NOZOOM;
	zoom = s_findnode( field, "zoom-form" );
	if ( meaningful ( zoom ) )
	    error ( "Field \"%s\"\nhas zoom-form specified,\nand it has the no-zoom bit set" ,
		    obj_name ( box) );
	return( RET_OK );
    }

    /* now, either use his name, or let the editor guess */
    zoom = s_findnode( field, "zoom-form" );
    if (! meaningful( zoom )) {

	/* this had to be watered down to a warning so that the sindex form
	   would continue to compile...                                 */

	filename = strrchr( g_filename, DIR_SEPARATOR );
	if (filename == NULL)
	    filename = g_filename;
	else
	    filename += 1;

	if ((box->bits & INDEXED) && (! seq( filename, "sindex.frm" ))) {
	    error( "Field \"%s\"\nshould have a zoom-form specified,\nor it should have the no-zoom bit set",
		   obj_name( box ) );
	    return( ERROR );
	}
	box->zoom = NULL;
    }
    else
	box->zoom = s_string( zoom );

    return( RET_OK );
}

/*****************************************************************************
* gprefix: extract the prefix portion of the data path, and                  *
* return an salloced copy of it.                                             *
*****************************************************************************/

static char *gprefix (POINTER *field)
{
    register char    *cp;
    register char    *cp2;
    char    c;
    int	    chlen;
    char    path[PATH_MAX];

    cp = data_path = s_findnode( field, "data-path" );
    if (cp == (char *) ERROR)
	return(NULL);

    /* is the string all whitespace? */
    for (cp2 = cp; (*cp2 != '\0') && isspace(*cp2); ) {
        chlen = mblen(cp2, MB_CUR_MAX);
	/* copy at least 1 char */
	if (chlen < 1) chlen = 1;
        cp2 += chlen;
    }
    if (*cp2 == '\0')
        return( NULL );

    /* look forward for recognized metacharacters, '*', '[' */
    cp2 = path;
    while (*cp != '\0')
    {   /* metacharacters legal only after '/' or at start */
	if (*cp == '/' || cp == data_path)
	{
	    if (*cp == '/')
		c = *(cp + 1);
	    else
		c = *cp;

	    if (c == '*' || c == '[')
	    {
		if (*cp == '/')
		    cp++;   /* leave pointer aimed at metacharacter */
		break;
	    }
	}
        chlen = mblen(cp, MB_CUR_MAX);
	/* copy at least 1 char */
	if (chlen < 1) chlen = 1;
        while (chlen > 0) {
            *cp2++ = *cp++;
	    chlen--;
	}
    }

    *cp2 = '\0';
    data_path = cp;

    return(s_string(path));
}

/*****************************************************************************
* gsuffix: return an s_string of what's left of g_datapath. Won't get        *
* here unless we have an indexed field to begin with.                        *
*****************************************************************************/

static char *gsuffix(void)
{
    if (*data_path != '\0')
        return(s_string( data_path ));
    else
	return(NULL);
}

/*****************************************************************************
* gutboxes: remove boxes from invariant text.                                *
*   Uses the invariant text field as it is read from the file.               *
*   Replaces characters that delimit or name fields with blanks.             *
*   Pads the results so that it is full width and heighth for the screen.    *
*   Sets the invariant_text global variable to the result.                   *
*****************************************************************************/

static int gutboxes(void)
{
    register int      i;        /* Counts lines in the invariant text       */
    register int      j;        /* counts chars or attrs in invariant text  */
    register ATTR    *cp;       /* Points into a line of the invariant text */

    if (text_count >= MAXHEIGHT) {
#if VBLSCR
	error( "Warning: The form is longer than screen" );
#else
	error( "The form is longer than screen" );
	return (ERROR);
#endif
    }

    /* Loop over each line of invariant_text */
    for (i = 0; i < text_count; i++) {

	/* Replace field delimiters and names by blanks */
	for (cp = invariant_text[i]; *cp != '\0'; )
        {
            /* skip subsequent bytes of multibyte chars */
            if (!firstbyte(*cp))
            {
                cp++;
                continue;
            }

	    switch (de_attr(*cp))
            {
		case ONEx1:
		case ONExN:
		    *cp++ = SPACE | FIRST;
		    break;

		case '[':
		    *cp++ = SPACE | FIRST;
                    while (de_attr(*cp) != ']')
                        *cp++ = SPACE | FIRST;
                    /* replace the terminating bracket */
		    *cp++ = SPACE | FIRST;
		    break;

		default:
		    cp++;
		    break;
	    }
	}
    }

    /* Pad out the invariant text to the screen size */
    invariant_text = (ATTR **) s_realloc( (char *)invariant_text,
			  (unsigned)MAXHEIGHT );

    for (i = text_count; i < MAXHEIGHT; i++)
    {
	invariant_text[i] = (ATTR *) s_alloc(MAXWIDTH+1, T_ATTR, NULL);
	for (j = 0; j < MAXWIDTH; invariant_text[i][j++] = SPACE | FIRST);
        invariant_text[i][MAXWIDTH] = (ATTR) '\0';
    }
    return (RET_OK);
}

/*****************************************************************************
* indexmode: determine indexing mode. g_datapath is set to point at          *
* the metacharacter, or NULL if there isn't any.                             *
*****************************************************************************/

static int indexmode (void)
{
    char    c;

    c = *data_path++;
    switch (c)
    {
	case '*':
	    break;

	case '\0':
	    return( 0 );

	case '[':
	    error( "Please fix the indicated data paths" );
	    return( ERROR );

	default:
	    error( "Unexpected character '%c' found in data-path", c );
	    return( ERROR );
    }

    c = *data_path;

    if (c == '/')
	data_path++;
    else if (c != '\0')
    {
	error( "Unexpected character '%c' found in data-path", c );
	return( ERROR );
    }

    return( INDEXED );
}

/*****************************************************************************
* makename: takes in "baz/foo.frm" and produces "baz/foo.ofm" as an          *
*   salloc. The original is left alone.                                      *
*****************************************************************************/

static char *makename (char *frmname)
{
    char *cp;
    char *ext;         /* the file extension + the '.' */
    int index1;

#ifdef DEBUG
    debug( "makename( frmname = \"%s\" )", frmname );
#endif

    /* check if valid form file */
    if (validname (frmname) == ERROR)
	{
	Eerror ( "Invalid form file name %s", frmname);
	return ((char *) ERROR);
	}

    /* construct new name */
    cp = s_string (frmname);
    ext = strrchr (cp, '.');
    if (ext == NULL)
	index1 = strlen (cp);
    else
	{
	index1 = ext - cp;
	cp = s_delete (cp, index1, strlen (ext));
	}

    cp = s_insert (cp, index1, 4);
    (void) strcat (cp, ".ofm");
    return (cp);
}

/*****************************************************************************
* validname: checks if the given formname and file is a valid form file.     *
*****************************************************************************/

static int validname (char *frmname)
{
    char *ext;                  /* file extension or suffix */
    int   ret;                  /* return code */

    ext = strrchr (frmname, '.');               /* get file extension */
    if (ext != NULL)
	ext++;

    /* check extension and set return value */
    if ((ext != NULL) && (seq (ext, "frm")))
	ret = RET_OK;
    else
	ret = ERROR;

    return (ret);
}

/****************************************************************************
* meaningful:  true if the object exists & has more than just white-space.  *
*****************************************************************************/

static int meaningful( char *sptr )
{
    int i;     /* loop counter              */
    int len;   /* length of object          */

    if (sptr == (char *) ERROR || sptr == NULL)
	return( FALSE );

    len = obj_count( sptr );
    for (i = 0; i < len; i++) {
	if (*sptr != SPACE && *sptr != '\t')
	    return( TRUE );
	sptr++;
    }

    return( FALSE );
}
/*****************************************************************************
* write_windows: produces and writes out window structures.                  *
*   Returns ERROR on I/O failure; otherwise RET_OK.                          *
*****************************************************************************/

static int write_windows( POINTER *boxes, SFILE *sfp )
{
    WSTRUCT *windows;   /* The WSTRUCT object to be written out */
    WSTRUCT *wpt;       /* Points at individual window structs */
    WSTRUCT *foreman;   /* First field in gang    */

    BOX     *bp;        /* Points at corresponding box struct  */
    POINTER *packed_invtxt;

    char    *pkbuf;     /* packed buffer for invariant text  window zero */
    char    *bigbuf;    /* packed buffer with room for trailing spaces   */

    int     i;         /* Indexes through the window array    */
    int     ntabs;     /* number of leading tabs                        */
    int     nblnks;    /* number of trailing blanks                     */
    int     j;         /* sub index                                     */

#ifdef DEBUG
    debug( "write_windows( boxes = %#x, sfp = %#x )", boxes, sfp );
    s_print( boxes );
#endif

    windows = (WSTRUCT *) s_alloc( obj_count( boxes ) + 1, T_WSTRUCT, NULL );

    /* First, make up window 0 */
    wpt = windows;
    wpt->w__flags = Tab_flag;
    wpt->w__name = NULL;            /* helper name NULL for window 0    */

    /* loop over all lines converting from internal wide character */
    /* form to external form with leading tabs expanded to spaces  */

    packed_invtxt = (char **) invariant_text;
    for (i = 0; i < obj_count(invariant_text); i++)
	{

	/* find out how many trailing blanks before packup operation */
	for (nblnks = 0; nblnks < MAXWIDTH; nblnks++)
	    {
	    if (de_attr(invariant_text[i][MAXWIDTH-nblnks-1]) != SPACE) break;
	    }

	pkbuf = packup( invariant_text[i] );
	free((char *) invariant_text [i]);
	for (ntabs = 0; ntabs < obj_count (pkbuf) && (*(pkbuf + ntabs) == '\t');
	     ntabs++);

	/* expand leading tabs to spaces */
	pkbuf = s_insert (pkbuf, 0, ntabs * 7);
	smear (SPACE, ntabs * 8, pkbuf);

	/* allocate buffer to hold all of line including trailing blanks */
	bigbuf = s_alloc (obj_count (pkbuf) + nblnks, T_CHAR, NULL);
	for (j=0; j < obj_count (pkbuf); j++)
	    {
	    bigbuf[j] = pkbuf [j];
	    }

	s_free (pkbuf);
	while (nblnks--) bigbuf[j++]=SPACE;
	packed_invtxt[i] = bigbuf;
	}

    wpt->w__cache  = packed_invtxt;  /* set up invariant text            */
    wpt->w__ftline = 0;             /* phoney screen binding I guess    */
    wpt->w__ftcol  = 0;
    wpt->w__lrline = MAXHEIGHT - 1; /* width and height are full screen */
    wpt->w__lrcol  = MAXWIDTH - 1;
    wpt->w__ttline = 0;
    wpt->w__ttcol  = 0;
    wpt->w__tbline = MAXHEIGHT - 1;
    wpt->w__tbcol  = MAXWIDTH - 1;
    wpt->w__tvloc  = g_tv;
    wpt->w__rmar   = 0;
    wpt->w__lmar   = 0;

    /* now loop through the rest of the windows, touching their insides */
    for (i = 1; i < obj_count( windows ); i++) {
	bp = (BOX *) boxes[i - 1];     /* boxes counts from zero  */
	wpt = &windows[i];             /* windows counts from one */

	wpt->w__flags   = bp->bits;
	wpt->w__ttline  = bp->top;
	wpt->w__ttcol   = bp->left;
	wpt->w__tbline  = bp->bot;
	wpt->w__tbcol   = bp->right;
	wpt->w__tvloc   = g_tv + bp->top * MAXWIDTH + bp->left;
	wpt->w__lrline  = bp->bot - bp->top;
	wpt->w__lrcol   = bp->right - bp->left;
	wpt->w__name    = s_string( obj_name( bp ) );
	wpt->w__ftline  = 0;
	wpt->w__ftcol   = 0;
	wpt->w__line    = 0;
	wpt->w__col     = 0;
	wpt->w__pfx     = bp->prefix ? s_string( bp->prefix ) : NULL;
	wpt->w__sfx     = bp->suffix ? s_string( bp->suffix ) : NULL;
	wpt->w__rmar    = bp->rmarg >= 0 ? (char) bp->rmarg : 0;
	wpt->w__lmar    = bp->lmarg >= 0 ? (char) bp->lmarg : 0;
	wpt->w__tabs    = bp->tablin ? s_string( bp->tablin ) : NULL;
	wpt->w__nxtgang = bp->gangp ? &windows[i + 1] : NULL;
	wpt->w__zoom    = bp->zoom ? s_string( bp->zoom ) : NULL;
	wpt->w__pickname = bp->pickname ? s_string( bp->pickname ) : NULL;
	wpt->w__msgno    = bp->msgno;
	wpt->w__deftxt   = bp->deftxt ? s_string(bp->deftxt):NULL;
    }

    /*
     * Fix up the horizontal ganging, which needs to be a circular
     *   singly-linked list, with the leftmost field (ONLY) having the
     *   HGANGED bit on. We find all "foremen", that is, leftmost windows
     *   in gangs, and give them (ONLY) the HGANGED bit.
     */

    for (i = 1; i < obj_count( windows ); i++) {
	wpt = &windows[i];
	if (wpt->w__flags & HGANGED) {

	    foreman = wpt;
	    for (j = i + 1; j < obj_count( windows ); j++) {
		wpt = &windows[j];
		wpt->w__flags &= ~HGANGED;     /* Turn off in other members   */
		if (wpt->w__nxtgang == NULL) { /* End of gang points to start */
		    wpt->w__nxtgang = foreman;
		    i = j;
		    break;
		}
	    }
	}
    }

    if (sf_write( sfp, 0, (char *) windows ) == ERROR) {
	error( "I/O error writing .ofm file" );
	s_free( (char *) windows );
	return( ERROR );
    }

    s_free( (char *) windows );
    return( RET_OK );
}
