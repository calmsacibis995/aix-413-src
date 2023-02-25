#if !defined(lint)
static char sccsid[] = "@(#)84	1.9  src/tenplus/e2/common/disp2.c, tenplus, tenplus411, GOLD410 3/24/93 09:29:55";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   St_init, St_quit, St_goto, St_outchar,
 *              St_getcap, getstring, parse_tc_entry,
 *              St_exitmodes
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
/* File: disp2.c - terminal dependent part of the new display               */
/****************************************************************************/

#include	<langinfo.h>

#include "def.h"
#include <termcap.h>
#include "INeditor_msg.h"

extern int g_St_noutput;     /* rough number of characters output */
extern short g_ospeed;       /* terminal speed, external here */

int gmode = NORMAL;          /* output mode NORMAL, UNDERLINED, STANDOUT,
				            GRAPHIC */

unsigned char *g_intran;     /* the single character table        */
POINTER  *g_multseq;         /* the sequence table                */
ATTR     g_dotchar;          /* character to fill dotted fields   */
char     *g_multcanon;       /* parallel table of canonicals      */

struct modestab modestab [] = {
	{UNDERLINE, NULL, NULL},
	{STANDOUT,  NULL, NULL},
	{GRAPHIC,   NULL, NULL},
}; 
	
/* current cursor; curlin = ERROR => no idea where it is */
static int curlin = ERROR;
static int curcol;

       int  CO;		    /* number of columns                             */
       int  LI;		    /* number of lines                               */
       int  g_highlight;    /* indicates that terminal can use standout mode */

#ifdef LEAK_FINDING
POINTER *termdesc;          /* this needs to be global for leak finding */
#else
static POINTER *termdesc;          /* the terminal description          */
#endif

/* The defined graphics characters are mapped into a zero-based table.  In
St_outchar(), the proper graphics sequence is set by using the de_attr'ized
input character as an offset into this table. */

static char graphtab [] =
    {
    ' ',	/* undefined */
    ' ',	/* G_VLINE   */
    ' ',	/* G_BLOT    */
    ' ',	/* undefined */
    ' ',	/* G_URC     */
    ' ',	/* G_TOR     */
    ' ',	/* G_LLC     */
    ' ',	/* G_LRC     */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* G_HLINE   */
    ' ',	/* G_ITEE    */
    ' ',	/* G_ULC     */
    ' ',	/* G_TEE     */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* G_TOL     */
    ' ',	/* undefined */
    ' ',	/* DOTCHAR   */
    ' ',	/* G_X       */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' ',	/* undefined */
    ' '		/* undefined */
    };


#define hasvalue(entry) (entry != NULL) /* check if a terminfo string is set */

/****************************************************************************/
/* St_init: initialize terminal                                             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     St_init initializes the terminal by clearing the screen and          */
/*     invoking St_goto to go to the home position.                         */
/****************************************************************************/

void St_init (void)
    {

    /* clear the screen                     */
    putp (clear_screen);

    /* go to top of screen                  */
    St_goto (0, 0);
    St_flush ();

    }

/****************************************************************************/
/* St_quit: return the terminal to normal                                   */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     St_quit clears the screen, invokes St_goto to go to the home         */
/*     position and resets the terminal.                                    */
/****************************************************************************/

void St_quit (void)
    {

    St_exitmodes ();

    /* clear the screen */
    putp (clear_screen);

    /* go to top of screen */
    St_goto (0, 0);

    /* reset the terminal */
    resetterm();

    }

/****************************************************************************/
/* St_goto: moves cursor to (line, column) 0 origin...                      */
/*                                                                          */
/* arguments:              int line,column - zero-based tv coords to go to  */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     curlin, curcol, g_St_noutput                     */
/*                                                                          */
/* globals changed:        curlin, curcol, g_St_noutput                     */
/*                                                                          */
/* synopsis:                                                                */
/*     If we know where the cursor is now, compare its position to where we */
/*     want to go to as follows and try to optimize as follows:             */
/*                                                                          */
/*         If we are already in the right spot, simply return.              */
/*                                                                          */
/*         If we are moving left one position                               */
/*             output the backspace character (^h).                         */
/*                                                                          */
/*         If we are moving up one line, output the cursor up sequence.     */
/*                                                                          */
/*         If we are moving to the beginning of the current line,           */
/*             output the carriage-return sequence if available,            */
/*             otherwise output the carriage-return (\r) character.         */
/*                                                                          */
/*         If we are moving to the home position (top left hand corner),    */
/*             output the cursor home sequence.                             */
/*                                                                          */
/*         If we are moving to the beginning of the next line               */
/*             and auto-margin is supported, do nothing                     */
/*             (but remember to update curlin and curcol).                  */
/*                                                                          */
/*         If we are moving to the beginning of the next line               */
/*             output newline character ('\n').                             */
/*                                                                          */
/*     In all other cases, perform the appropriate cursor motion using      */
/*     absolute cursor movement (cursor_address) and update the globals     */
/*     curlin, curcol, and g_St_noutput.                                    */
/****************************************************************************/

void St_goto (int line, int column)
{

    /* don't make a side trip back to the same place */
    if (line == curlin && column == curcol)
	return;

    /* if we don't know where we are */
    if (curlin == ERROR)
	goto jumpit;

    /* moving cursor left one position */
    if (line == curlin && column == curcol - 1)
	(void) putchar ('\b');

    /* moving up one line */
    else if (hasvalue(cursor_up) && (line == curlin - 1 && column == curcol))
	putp (cursor_up);

    else

    /* beginning of this line */
    if (line == curlin && column == 0)
	if (hasvalue(carriage_return))
	    putp (carriage_return);
	else
	    (void) putchar ('\r');

    /* screen home position - top left hand corner */
    if (hasvalue(cursor_home) && (line == 0) && (column == 0))
	putp (cursor_home);

    /* notice "auto-margin" effect on full lines */

    else if (auto_right_margin &&
	     (line == curlin + 1 && column == 0 && curcol == CO))

	/* do nothing, but remember to set curlin and curcol! */
	    ;

    /* next line */
    else if (line == curlin + 1 && column == 0)
	(void) putchar ('\n');

    else
	{
    jumpit:

	/* set up the appropriate cursor motion sequence */
	putp (tparm(cursor_address, line, column));
	}

    /* update line & column position */
    curlin = line;
    curcol = column;

    /* rough guess, for interruptability    */
    g_St_noutput += 5;

}

/****************************************************************************/
/* St_outchar: output a (possibly graphics) character.                      */
/*                                                                          */
/* arguments:              ATTR *c - pointer to the character to be output  */
/*                         int  nbytes - number of bytes to be output       */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     curcol, g_St_noutput, curlin                     */
/*                                                                          */
/* globals changed:        curcol, g_St_noutput, curlin                     */
/*                                                                          */
/* synopsis:                                                                */
/*     St_outchar considers three classes of chars - underscored, graphics  */
/*     and others.                                                          */
/*                                                                          */
/*     For underscored chars, if first leaves graphics mode if graphic      */
/*     mode is on, then sends the underscore char command, then the         */
/*     char itself, and then the end-underscore command.  If the            */
/*     underscore/end-underscore commands are not available on the current  */
/*     terminal, simply output the char and don't worry about it.           */
/*                                                                          */
/*     For graphics chars (those before 040 in the ascii set), switch       */
/*     on the possible graphics and blot chars and, if none match,          */
/*     pretend the char was a space.  To output the graphics char           */
/*     first issue the enter-graphics command if not already in graphics    */
/*     mode (and remember for later reference the fact that the terminal is */
/*     now in graphics mode), and issue the terminal-dependent graphics     */
/*     char corresponding to the e2 graphics char code.                     */
/*                                                                          */
/*     For other chars, first exit graphics mode if it is turned on,        */
/*     and then output the char.                                            */
/*                                                                          */
/*     Finally, increment curcol and g_St_noutput to show we just issued a  */
/*     character. If the terminal supports auto-margin and we just moved    */
/*     out of the lower right corner, update curlin and curcol to show that */
/*     we have now moved to the upper left.                                 */
/****************************************************************************/

void St_outchar (ATTR *c, int nbytes)
{

    char *cp;                     /* 'c' with attributes removed       */
    char *cpp;                    /* initial value of 'cp' to be freed */

    int  c_attr = get_attr (*c);  /* the attributes of the first byte  */
    int i, width;

    char graph_ch = '\0';
    ATTR bit;

    /* remove the attributes from the character */
    cpp = cp = a2string(c, nbytes);

    /* is it a graphic character? */
    if (is_graphic (*c))
	{
	/* If *cp not in graphtab, then graph_ch is set to a SPACE */
	graph_ch = graphtab[*cp];
	if (graph_ch == ' ')
	    attr_unset(c_attr, GRAPHIC);
        else
	    attr_set (c_attr, GRAPHIC);
	}
    
    /* if equal, do nothing to modes */
    if ((c_attr & ~(FIRST)) != gmode)
	{

	/* if exit_attribute_mode available, then exit all video
	   modes to exit any video mode */

	if ((gmode != NORMAL) && hasvalue(exit_attribute_mode))
	    {
	    putp (exit_attribute_mode);
	    putp (font_0);
	    gmode = NORMAL;
	    }

	/* loop over modes and enter/exit the proper modes */
	for (i = 0; i < NUM_ATTRS; i++)
	    {

	    /* determine bit position */
	    bit = modestab[i].bitnum;
	    if (is_attrset(c_attr, bit) != is_attrset(gmode, bit))
		if (is_attrset(c_attr, bit))
		    {
		    putp (modestab[i].getin);
		    if (modestab[i].getout != NULL)
			attr_set (gmode, bit);
		    }
		else
		    {
		    putp (modestab[i].getout);
		    attr_unset (gmode, bit);
		    }
	    }
	}

    /*  put out graphics character, or space */
    if (is_attrset(c_attr, GRAPHIC) || graph_ch == ' ')
	{
	putchar(graph_ch);
	}
    else
	{

	/* normal character */
	if (MultibyteCodeSet)
	    {

	    /* put out a multibyte character */
            while(nbytes--)
	        (void) putchar(*cp++);
	    }
	else
	    {

	    /* put out a single byte character */
	    (void) putchar (*cp);
	    }
	}

    /* increment column position by the display width */
    s_free(cpp);
    width        = char_width(c);
    curcol       += width;
    g_St_noutput += width;

    /* gone passed right hand margin? */
    if (auto_right_margin && curcol >= CO)
	{
	curlin++;
	curcol = 0;
	}

}

/****************************************************************************/
/* St_getcap: get needed capabilities from terms.bin                        */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_sysdir, g_intran, g_multseq,                   */
/*                         g_multcanon, termdesc                            */
/*                                                                          */
/* globals changed:        g_intran, g_multseq, g_multcanon, termdesc       */
/*                                                                          */
/* synopsis:                                                                */
/*     St_getcap initializes all termcaps variables needed for talking to   */
/*     the terminal.  If for any reason it is unable to get what it needs,  */
/*     there is no way to continue, and the editor stops with exit (-1).    */
/*                                                                          */
/*     It reads the structured terminal description file, looking in        */
/*     g_sysdir/terms.bin, looking for the record corresponding             */
/*     to the value of the TERM environment variable.  TERM can either      */
/*     be set to the record number to read, or (more typically) its name.   */
/*                                                                          */
/*     Once it has found the correct record, it reads all the terminal-     */
/*     dependent output string sequences (e.g. how to set the cursor        */
/*     location), the terminal flags (e.g. whether the terminal supports    */
/*     automatic margin-wrapping), and the numbers (how many lines and      */
/*     columns are on the screen).  Finally, it also reads the input        */
/*     sequence tables that map keyboard sequences into editor commands.    */
/*                                                                          */
/****************************************************************************/

void St_getcap (void)
    {
    SFILE *sfp;

    char *message, *value, *termtype, *bin_termtype, *readfrom;
    char termfile [PATH_MAX];
    extern char *g_sysdir;

    int i, status;
    POINTER *termnames;

    readfrom = getenv ("TDESC");
    if ((readfrom == NULL) || (*readfrom == '\0'))
	{
	(void) sprintf (termfile, "%s/terms.bin", g_sysdir);
	readfrom = termfile;
	}

    sfp = sf_open (readfrom, "r");
    if (sfp == (SFILE *)ERROR)
	{

	message = Egetmessage(M_E_BAD_TERMSFILE,"0611-098 The terminal definition file %s\n\
	is damaged.  Install INed again or use local problem reporting procedures.\n",FALSE);

	/* The message in the catalog includes a trailing \n. */
	(void) fprintf (stderr, message, readfrom);
	s_free (message);
	fatal ("getcap: cannot open %s", readfrom);
	}

    termtype = bin_termtype = getenv ("TERM");
    if (termtype == NULL)
	{

	message = Egetmessage (M_E_TERM_NOT_SET,"0611-102 The TERM environment variable, which names your terminal\n\
	type, is not set.  Set it and export it.\n",FALSE);

	(void) fprintf (stderr, message);
	s_free (message);
	exit (-1);
	}

    /* find the entry for the given terminal name in the terms.bin file */
    termnames = sf_getnames (sfp);
    for (i = 0; i < obj_count (termnames); i++)
	if (parse_tc_entry (termnames[i], bin_termtype))
	    {
	    termdesc = (POINTER *) sf_read (sfp, i);
	    goto found;
	    }

    /* use DEFAULT terminal if given terminal name not found in terms.bin */
    /* ensure termtype is not already DEFAULT */
    if (strcmp(bin_termtype,"DEFAULT") != 0)
        { /* attempt to use DEFAULT terminal from terms.bin */
	bin_termtype = "DEFAULT";
	for (i = 0; i < obj_count(termnames); i++)
	    if (parse_tc_entry(termnames[i], bin_termtype))
		{
		termdesc = (POINTER *) sf_read (sfp, i);
		goto found;
		}
	}

found:
    s_free ((char *)termnames);
    (void) sf_close (sfp);

    if (termdesc == NULL || termdesc == (POINTER *) ERROR)
	{
	message = Egetmessage (M_E_BAD_TERMSFILE,"0611-098 The terminal definition file %s\n\
	is damaged.  Install INed again or use local problem reporting procedures.\n",FALSE);

	/* The message in the catalog includes a trailing \n. */
	(void) fprintf (stderr, message, readfrom);
	s_free (message);
	fatal ("Error reading terminal description record");
	}

    if (obj_type (termdesc) != T_POINTER)
	{
	message = Egetmessage (M_E_BAD_TERMSFILE,"0611-098 The terminal definition file %s\n\
	is damaged.  Install INed again or use local problem reporting procedures.\n",FALSE);

	/* The message in the catalog includes a trailing \n. */
	(void) fprintf (stderr, message, readfrom);
	s_free (message);
	fatal ("Read non-POINTER terminal description record");
	}

    message = Egetmessage (M_E_USING_TTY,"Using the %s terminal description.\n",FALSE);

    /* The message in the catalog includes a trailing \n. */
    (void) fprintf (stderr, message, bin_termtype);
    s_free (message);

    setupterm(termtype, 1, &status); 
    if (status != 1)
	{
	message = Egetmessage (M_E_TUNK, "Terminal type %s is not recognized.\n",FALSE);

	/* The message in the catalog includes a trailing \n. */
	(void) fprintf (stderr, message, termtype);
	s_free (message);
	fatal ("Unknown terminal type");
	}

    /* check whether minimum required screen handling characteristics
    are available */
    if (!(hasvalue(clear_screen) && hasvalue(cursor_address)))
	{
	message = Egetmessage (M_E_INSUFF_TERMINFO, "%s: Insufficient terminal characteristics.\n", FALSE);

	/* The message in the catalog includes a trailing \n. */
	(void) fprintf (stderr, message, termtype);
	s_free (message);
	fatal ("Insufficient terminal characteristics");
	}

    if (hasvalue(font_0) && hasvalue(font_1) && hasvalue(box_chars_1))
    {

	modestab[2].getin = font_1;	/* enter graphics mode */
	modestab[2].getout = font_0;	/* exit graphics mode  */

	/* box character graphics set */
	graphtab[de_attr(G_ULC)]   = box_chars_1[0];
	graphtab[de_attr(G_HLINE)] = box_chars_1[1];
	graphtab[de_attr(G_URC)]   = box_chars_1[2];
	graphtab[de_attr(G_VLINE)] = box_chars_1[3];
	graphtab[de_attr(G_LRC)]   = box_chars_1[4];
	graphtab[de_attr(G_LLC)]   = box_chars_1[5];
	graphtab[de_attr(G_TEE)]   = box_chars_1[6];
	graphtab[de_attr(G_TOR)]   = box_chars_1[7];
	graphtab[de_attr(G_ITEE)]  = box_chars_1[8];
	graphtab[de_attr(G_TOL)]   = box_chars_1[9];
	graphtab[de_attr(G_X)]     = box_chars_1[10];

    }
    else
    {

	modestab[2].getin =
	modestab[2].getout = NULL;

	graphtab[de_attr(G_ULC)]   =
	graphtab[de_attr(G_URC)]   =
	graphtab[de_attr(G_LRC)]   =
	graphtab[de_attr(G_LLC)]   =
	graphtab[de_attr(G_TEE)]   =
	graphtab[de_attr(G_TOR)]   =
	graphtab[de_attr(G_ITEE)]  =
	graphtab[de_attr(G_TOL)]   =
	graphtab[de_attr(G_X)]     = '+';
	graphtab[de_attr(G_HLINE)] = '-';
	graphtab[de_attr(G_VLINE)] = '|';

    }

    graphtab[de_attr(G_BLOT)] = '@';

    modestab[0].getin = enter_underline_mode;
    modestab[0].getout = exit_underline_mode;
    modestab[1].getin = enter_standout_mode;
    modestab[1].getout = exit_standout_mode;

    g_highlight = hasvalue(enter_standout_mode);
    g_dotchar = (ATTR) SPACE;
    graphtab[de_attr(DOTCHAR)] = g_dotchar;

    CO = columns;		/* number of columns */
    LI = lines;            	/* number of lines   */

    /* set up the input translation */
    g_intran    = (unsigned char *) getstring ("g_intran");
    g_multseq   = (POINTER *) getstring ("g_multseq");
    g_multcanon = getstring ("g_multcanon");

    if (g_intran == NULL || g_multseq == NULL || g_multcanon == NULL)
	{

	message = Egetmessage (M_E_BAD_TERMSFILE,"0611-098 The terminal definition file %s\n\
	is damaged.  Install INed again or use local problem reporting procedures.\n",FALSE);

	/* The message in the catalog includes a trailing \n. */
	(void) fprintf (stderr, message, readfrom);
	s_free (message);
	fatal ("failure reading input translation");
	}

}


/****************************************************************************/
/* getstring: get string from terminal description                          */
/*                                                                          */
/* arguments:              char *name - the name to look for                */
/*                                                                          */
/* return value:           char *   - the value read from the terminal      */
/*                                    description, or NULL.                 */
/*                                                                          */
/* globals referenced:     termdesc                                         */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Invoke s_findnode to look for the node with the given name in the    */
/*     termdesc record.  If found, return its value; otherwise return NULL. */
/****************************************************************************/

char *getstring (char *name)
    {

    char *rv;
    rv = s_findnode (termdesc, name);

    /* Note: *rv may reasonably be NULL in g_intran and friends */
    if (rv == (char *) ERROR)
	return (NULL);

    return (rv);
    }

/****************************************************************************/
/* parse_tc_entry: parse def.trm entry for terminal name                    */
/*                                                                          */
/* arguments:   termname - list of names of terminal description            */
/*              termtype - name of terminal description to be found         */
/*                                                                          */
/* return value: TRUE if a name was found to match termtype, otherwise      */
/*               FALSE                                                      */
/*                                                                          */
/* globals referenced: none                                                 */
/*                                                                          */
/* globals changed: none                                                    */
/*                                                                          */
/* synopsis:                                                                */
/*   Check termtype against each terminal in termname for a match.  The     */
/*   names in termtype are delineated by one or more blank characters.      */
/*                                                                          */
/****************************************************************************/

int parse_tc_entry (char *termname, char *termtype)

{
    char *first;    /* points to first character in a terminal name */
    char *last;     /* points to first character following terminal name */
    char  c;        /* used to save contents of last  */

    /* return TRUE if termname matches */
    if (strcmp (termname, termtype) == 0)   
	return (TRUE);

    /* Set last to point to first non-space character. 'isspace()' or
       'iswspace()' is not required here as the strings are not user
       defined */

    last = termname;
    while (*last == SPACE)
       last++;

    /* check each name */
    while (*last)
    {
	first = last;
	while (*last && *last != SPACE)
            last++;

	c = *last;
	*last = '\0';
        if (strcmp (first, termtype) == 0)
	{
   	    *last = c;
	    return (TRUE);
	}
	*last = c;
	while (*last == SPACE)
	    last++;
    }
    return (FALSE);
}

/****************************************************************************/
/* St_exitmodes: takes the terminal out of any modes currently active.      */
/*                                                                          */
/* arguments: none                                                          */
/*                                                                          */
/* return value: none                                                       */
/*                                                                          */
/* globals referenced: gmode                                                */
/*                                                                          */
/* globals changed: gmode                                                   */
/*                                                                          */
/* synopsis:                                                                */
/*   Use Terminfo exit_attribute_mode and font_0 (exit graphics mode) to    */
/*   exit any display modes if possible, otherwise individually exit any    */
/*   modes that are in effect. Then set the gmode flag to NORMAL.           */
/****************************************************************************/

void St_exitmodes (void)
{
    ATTR bit;
    int i;

    if (hasvalue(exit_attribute_mode))
    {
	putp (exit_attribute_mode);
	putp (font_0);
    }
    else
	/* loop over modes and exit the proper modes */
	for (i = 0; i < NUM_ATTRS; i++)
	    {
	    bit = modestab[i].bitnum;   /* determine bit position */
	    if (is_attrset(gmode, bit))
		{
		putp (modestab[i].getout);
		attr_unset (gmode, bit);
		}
	    }
    gmode = NORMAL;
}
