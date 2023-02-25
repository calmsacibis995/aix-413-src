#if !defined(lint)
static char sccsid[] = "@(#)26	1.8  src/tenplus/helpers/editorprf/editorprf.c, tenplus, tenplus411, GOLD410 3/23/93 11:57:17";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	N_uinit, N_ubefore, N_nextitem, N_uafter,
 *		N_zoomin, N_umod, getcolor, N_uins, N_udel
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
 *
 *                      Copyrighted as an unpublished work.                 
 *                         INTERACTIVE TEN/PLUS System                      
 *                        TEN/PLUS Editor Profile Helper                    
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
 
/****************************************************************************/
/* file:  editorprf.c                                                       */
/*                                                                          */
/* Helper for the editor profile file                                       */
/****************************************************************************/

#if !defined(lint)
static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
#endif /* lint */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <chardefs.h>
#include <database.h>
#include <edexterns.h>
#include <keynames.h>
#include "INeditorprf_msg.h"
#include "INeditor_msg.h"
#include <editorprf.h>
#include <libhelp.h>
#include <libobj.h>
#include <keys.h>
#include <libutil.h>


/** Local Functions **/

static int N_nextitem (int , int, char *, int);
static int N_zoomin (void);

/****************************************************************************/
/* N_uinit:  sets up debugging output file                                  */
/****************************************************************************/

int N_uinit (char *filename)
    {
    register char *system_profile; /* name of system profile file */
    static char before [] = { HMOD, NEXT, ZOOM_IN, '\0' };
    static char after [] = { HINS, HDEL, NEXT, PREVIOUS, ZOOM_IN, '\0' };
    SFILE *sfp; /* structured fp for the editorprf */
    int file_size;

#ifdef DEBUG
    if (g_debugfp == NULL)
	{
	g_debugfp = fopen ("editorprf.trc", "a+");

	if (g_debugfp == NULL)
	    Eerror (N_DEBUG, "Cannot open editorprf.trc");
	}
#endif

    Esethook (before, after);

#ifdef DEBUG
    debug ("---> uinit (%s)", filename);
#endif

    sfp = sf_open (filename, "r");
    if (sfp == (SFILE *)ERROR)
	{
	Eerror (N_BADPROF, "Cannot open profile file \"%s\"", filename);
	return (ERROR);
	}
    file_size = obj_count (sf_records (sfp));
    (void) sf_close (sfp);

    if (file_size == 0)
	{
	system_profile = Efixvars (SYS_PROFILE);
	Emessage (N_UINIT, "Copying standard profile \"%s\"...", system_profile);
	s_free (Esync ());
	if (Ecopyprofile (system_profile, filename) == ERROR)
	    {
	    SFILE *sf_p;
	    /* copy was incomplete so make an empty version again */
	    (void)unlink (filename);
	    sf_p = sf_open (filename,"c");
	    if (sf_p != (SFILE *)ERROR)
		{
		(void) sf_close (sf_p);
		/* doesn't do to leave an empty editorprf around */
		Ermfile (filename);
		}
	    }
	s_free (system_profile);
	Ereopen ();
	}
    Edispname(filename);
    return (RET_OK);
    }

/****************************************************************************/
/* N_ubefore: distribute button pushes to the rest of the helper            */
/****************************************************************************/

int N_ubefore (register int key, int flag, char *string, int numvalue)
    {
    /**** this helper only catches next and previous ****/

    if (key == NEXT)
	return (N_nextitem (key, flag, string, numvalue));
    else
	return (N_zoomin ());
    }

/****************************************************************************/
/* N_nextitem:  before handler for the NEXTITEM key                         */
/****************************************************************************/
/*ARGSUSED*/
static int
 N_nextitem (int key, int flag, char *string, int numvalue)
    {
    register char *path;        /* current editor path  */
    register recindex;          /* zero-based index of current record */

#ifdef DEBUG
    debug ("N_nextitem: key = %d, flag = %d, string = '%s', numvalue = %d",
	   key, flag, string, numvalue);
#endif

    path = Egetpath ();

#ifdef DEBUG
    debug( "N_nextitem: path = \"%s\"", path );
#endif

    if (obj_count (path) != 1) /* if not a single digit, let editor do it */
	{
	s_free (path);
	return (0);
	}
    recindex = atoi (path);
    s_free (path);

    switch (flag)
	{
	case NOARGS:

	    if (recindex >= (Enumsiblings () - 1))
		{
		Eerror (N_NEXT, "No more profile items");
		return (1);
		}
	    else
		return (0);

	case NUMBERARG:
	    if ((numvalue >= 0) && (numvalue < Enumsiblings ()))
		return (0); /* let the editor do it */
	    else
		{ /* error - number out of range */
		Eerror(N_ARGNUMBER, "Numeric argument out of range");
		return (1);
		}

	default:  /* error - string argument not allowed */
	    Eerror(N_ARGBAD, "Argument must be a number");
	    return (1);
	}
    }


/****************************************************************************/
/* N_uafter:  handles after hook on NEXT, PREVIOUS and ZOOMIN               */
/****************************************************************************/
/*ARGSUSED*/ 
void N_uafter (int key)
    {
    register char *form;        /* name of zoom form    */
    register char *path;        /* current editor path  */

    path = Egetpath ();

    if (*path == '\0')          /* at the top level of the file */
	{
	s_free (path);
	return;
	}
    s_free (path);

    form = Egettext ("FORM", 0);

    if (form == (char *) ERROR)
	return;

    if (Euseform (form) == ERROR)
	Eerror (N_FORM, "Cannot find %s form", form);

    s_free (form);
    }

/****************************************************************************/
/* N_zoomin:  handles the before control hook for ZOOM-IN                   */
/****************************************************************************/

static int N_zoomin (void)
    {
    register char *form;        /* name of current form */
    register line;              /* zero-based current line number */

#ifdef DEBUG
    debug ("zoomin called");
#endif

    form = Eformname ();

    if (!seq (form, "editorprf"))
	{
	s_free (form);
	return (0);
	}
    s_free (form);

    line = Eline ();

    if (line  >= Enumlines (""))
	{
	Eerror (N_ZOOMIN, "Cannot ZOOM-IN here");
	return (1);
	}
    return (0);
    }

/****************************************************************************/
/* N_umod: handles the Hmod call                                            */
/****************************************************************************/

#define COLORMASK 7
#define INTENSE 8
#define NUMCOLORS 8

/* the color defines must correspond to the index of entries in the color
   list */
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define WHITE 7

#define GROUP1 1
#define GROUP2 2
#define GROUP3 4

/* field type (color or intense) */
#define COLOR_FIELD 0
#define INTENSE_FIELD 1

/* color list */
    static char **colorlist = NULL;

int N_umod (char *fieldname, int line, char *newtext)
    {
    int color;
    register int field;
    char *oldcolor;
    register int i;
    int group;
    int field_type;
    int accept;
    char intensity;
    char tmpcolor[2];

    static struct 
	{
	char *numeric_name; /* name of numeric field */
	char *color_name; /* name of color field to process */
	char *intense_name; /* name of intense field */
	int default_color;
	char *color; /* character string representation of color code */
	int group; /* mask indicating groups where inequality is required */
	}
    field_info [] =
	{
	    {NNORMAL, "n", "ni", WHITE, NULL, GROUP1 | GROUP3},
	    {NBACKGROUND, "b", "bi", BLACK, NULL, GROUP1},
	    {NUNDERLINE, "u", "ui", BLUE, NULL, GROUP2 | GROUP3},
	    {NUNDERBKG, "ub", "ubi", WHITE, NULL, GROUP2},
	};

#define NUM_ENTRIES (sizeof(field_info)/sizeof(*field_info))

    /* all fields should be one line only. Guarantee this. */
    if (line)
	{
	Eerror (LINE1, "Please modify only the first line of these fields");
	return (FALSE);
	}

    /* determine which field we are in */
    for (field=0; field<NUM_ENTRIES; field++)
	if (seq (fieldname, field_info [field].color_name))
	    {
	    field_type = COLOR_FIELD;
	    break;
	    }
	else if (seq (fieldname, field_info [field].intense_name))
	    {
	    field_type = INTENSE_FIELD;
	    break;
	    }

    /* field not found in list ==> bogus form */
    if (field == NUM_ENTRIES)
	{
	Eerror (FIELDNAME, "Illegal field name '%s'", fieldname);
	return (TRUE);
	}

    /* get numeric value for field */
    if ((oldcolor = Egettext (field_info [field].numeric_name, 0)) ==
      (char *)ERROR)
	{
	Eerror (COLORFRM, "color form missing field '%s'",
	  field_info [field].numeric_name);
	return (FALSE);
	}

    /* create data for numeric field if necessary */
    if (obj_count (oldcolor) < 1)
	oldcolor = (char *)s_realloc (oldcolor, 1);

    oldcolor [0] &= ~ALPHAMASK; /* strip bits which make it alpha */

    /* process the field to determine the new color */
    if (field_type == COLOR_FIELD)
	{
	color = getcolor (newtext);
	if (color == ERROR) /* bad color specified */
	    {
	    s_free (oldcolor);
	    return (TRUE);
	    }
	if (color == NUMCOLORS) /* use default color */
	    color = field_info [field].default_color;

	/* replace old color by new value */
	oldcolor [0] = (oldcolor [0] & ~COLORMASK) | color;
	}
    else /* field_type == INTENSE_FIELD */
	oldcolor [0] = (oldcolor [0] & COLORMASK) |
	  ((newtext [0] && !ismbspace(&newtext [0])) ? INTENSE : 0);

    /* set up color codes for checking for sufficient distinctness */
    field_info[field].color = oldcolor;
    for (i=0; i<NUM_ENTRIES; i++)
	if (i != field)
	    {
	    field_info [i].color = Egettext (field_info [i].numeric_name, 0);
	    field_info [i].color [0] &= ~ALPHAMASK;
	    }

    /* verify selected color set will yield distinguishable text */
    accept = TRUE;
    group = field_info [field].group;
    for (i=0; i<NUM_ENTRIES; i++)
	if (i != field && (field_info[i].group & group) &&
	  field_info [i].color[0] == oldcolor[0])
	    {
	    /* distionction problem found, ask user to accept or reject */
	    accept = Econfirm (BADCOLORS, "Warning: selected color scheme will not sufficiently distinguish\nbetween foreground, background, and underlined text.\nTouch EXECUTE to accept color scheme.\nTouch CANCEL to restore default color scheme.");
	    break;
	    }

    /* check to do requested change or restore defaults */
    if (accept)
	{
	/* do requested modification */
	oldcolor [0] |= ALPHAMASK;
	Eputtext (field_info [field].numeric_name, 0, oldcolor);
	}
    else
	{
	/* restore defaults */
	tmpcolor[1] = '\0';
	for (i=0; i<NUM_ENTRIES; i++)
	    {
	    Edelline (field_info[i].color_name, 0, -1);
	    Eputtext (field_info[i].color_name, 0,
		      colorlist [field_info[i].default_color]);
	    Edelline (field_info[i].numeric_name, 0, -1);
	    intensity = field_info[i].color[0] & INTENSE;
	    tmpcolor[0] = field_info[i].default_color | intensity | ALPHAMASK;
	    Eputtext (field_info[i].numeric_name, 0, tmpcolor);
	    }
	}

    /* free allocations made */
    for (i=0; i<NUM_ENTRIES; i++)
	    s_free (field_info [i].color);

    /* return - keep changes made, do not restore old value */
    return (FALSE);
    }

int getcolor (char *name)
    {
    register int i;

    if (colorlist == NULL)
	{
	colorlist = s2pointer(Egetmessage (COLORLIST, "black\nblue\ngreen\n\
cyan\nred\nmagenta\nbrown\nwhite", FALSE));
	if (colorlist == (POINTER) ERROR ||
	  obj_type (colorlist) != T_POINTER ||
	  obj_count (colorlist) != NUMCOLORS)
	    {
	    if (colorlist != (char **)ERROR)
		s_free ((char *)colorlist);
	    colorlist = NULL;
	    Eerror (BADCOLORLIST,
	      "Bad color list in editorprf help message file");
	    return (ERROR);
	    }
	}

    for (i=0; i<NUMCOLORS; i++)
	if (seq (name, colorlist [i]))
	    break;
    if (i == NUMCOLORS && name [0]) /* not found and not empty */
	{
	Eerror (UNK_COLOR, "Unknown color selected");
	return (ERROR);
	}
    return (i);
    }

#define COLOR_FORM "pcolor"

void N_uins (char *fieldname, int line, int count)
{
    int numlines;
    char *formname = Eformname ();
    int color_form;

    /* only worry about insert line for the color form */
    color_form = seq (formname, COLOR_FORM);
    s_free (formname);

    if (!color_form)
	return;

    if ((numlines = Enumlines (fieldname)) > 1)
	{
	Eerror (NOINSERT, "These fields are limited to one line each");
	Edelline (fieldname, line, count);
	}
}

void N_udel (char *fieldname, int line)
{
    char *formname;
    int color_form;

    if (line)
	return;

    /* only worry about delete line for the color form */
    formname = Eformname ();
    color_form = seq (formname, COLOR_FORM);
    s_free (formname);

    if (!color_form)
	return;

    N_umod (fieldname, line, "");
}


