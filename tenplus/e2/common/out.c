static char sccsid[] = "@(#)29	1.11  src/tenplus/e2/common/out.c, tenplus, tenplus411, GOLD410 3/4/94 15:07:19";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	Edisplay, Ettyfix, Ettyunfix, fixname, itoattr,
 *              attr_pad, fixtabs, tmclose, tmopen
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/
/* file:  out.c                                                             */
/*                                                                          */
/* output side of terminal handler                                          */
/****************************************************************************/

#include "def.h"
#include "termcap.h"
#include "INeditor_msg.h"

#define DISPSIZE 40   /* number of chars in filename */

static void itoattr (int, int, ATTR *);
static void attr_pad (char *, ATTR *, int);
void fixtabs (void);

/****************************************************************************/
/* Edisplay:  called to fix up the display                                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_dispflag, g_wp                                 */
/*                                                                          */
/* globals changed:        g_dispflag                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     Edisplay is used to update the screen display to be current.         */
/*     If the g_dispflag global is set to DISPINIT, it invokes the          */
/*     Si_redisplay procedure to clear the screen.  In any case, it         */
/*     invokes fixtabs and fixname to fix the tab/margin and filename       */
/*     lines, and sets g_dispflag to DISPNORM to indicate normal display    */
/*     status.   It then invokes Si_display to redisplay the goalscreen     */
/*     and, if not interrupted, updates the cursor position.                */
/*                                                                          */
/* NOTE:  This routine is specific to the TERMCAPS version of the editor.   */
/****************************************************************************/

void Edisplay (void)
    {
    WSTRUCT *wp=g_wp;
    ATTR *goalline;
    int count, i, n;

    /* clear screen on full refresh */
    if (g_dispflag == DISPINIT) 
	Si_redisplay ();

#ifdef VBLSCR
    if ((w_ttline (g_wp) + w_line (g_wp)) > (LI - 2)) /* down */
	{
	wp = g_wp;
	g_wp = g_warray;
	i = ((LI - 2) / 3) + 1;
	for (count = i; w_ttline (wp) + w_line (wp) - count > LI - 2;
	     count += i) ;

	n = w_tbline (g_wp) - w_ttline (g_wp);
	g_wp->w__ttline -= count;
	g_wp->w__tbline = w_ttline(g_wp) + n;
	Si_dirty (0, LI - 2);
	for (i = 0; i <= LI-2; i++)
	    {
	    goalline = (ATTR *) g_goalscreen[i];
	    i_smear ((short)(SPACE | FIRST), CO, (short *) goalline);
	    goalline[CO] = '\0';
	    g_goalcrc[i] = g_uniqnum++;
	    }

	sendbox (0, w_lrline (g_wp), 0, w_lrcol (g_wp));
	i = obj_count (g_warray);
	for (g_wp = &g_warray[1]; g_wp < &g_warray[i]; g_wp++)
	    {
	    n = w_tbline(g_wp) - w_ttline(g_wp);
	    g_wp->w__ttline -= count;
	    g_wp->w__tbline = w_ttline(g_wp) + n;
	    sendbox( 0, w_lrline (g_wp), 0, w_lrcol (g_wp));
	    }

	g_wp = wp;

	}
    else if (((w_ttline (g_wp) + w_line (g_wp)) <= 0)) /* up */
	{

	/* If on line 0 and there is no tab rack, don't scroll. */
	if ((w_ttline(g_wp) + w_line(g_wp) != 0) ||
	     getstat (w_flags (&g_warray [0]), SHOWTABS))
	    {
	    wp = g_wp;
	    g_wp = g_warray;
	    i = ((LI - 2) / 3) + 1;
	    n = getstat (w_flags (&g_warray [0]), SHOWTABS) ? 1 : 0;
	    for (count = i; w_ttline (wp) + w_line (wp) + count < n;
		 count += i) ;

	    n = w_tbline(g_wp) - w_ttline(g_wp);
	    g_wp->w__ttline += count;
	    g_wp->w__tbline = w_ttline(g_wp) + n;
	    Si_dirty (0, LI - 2);
	    sendbox (0, w_lrline (g_wp), 0, w_lrcol (g_wp));
	    i = obj_count (g_warray);
	    for (g_wp = &g_warray[1]; g_wp < &g_warray[i]; g_wp++)
		{
		n = w_tbline(g_wp) - w_ttline(g_wp);
		g_wp->w__ttline += count;
		g_wp->w__tbline = w_ttline(g_wp) + n;
		sendbox( 0, w_lrline (g_wp), 0, w_lrcol (g_wp));
		}
	    g_wp = wp;
	    }
	}

    /* Need to scroll form right or left? */
    if ((w_ttcol (g_wp) + w_col (g_wp) ) > (CO - 1)) /* left */
	{
	wp = g_wp;
	g_wp = g_warray;
	i = ((CO - 1) / 3) + 1;
	for (count = i; w_ttcol (wp) + w_col (wp) - count > CO - 1;
	     count += i) ;

	n = w_tbcol(g_wp) - w_ttcol(g_wp);
	g_wp->w__ttcol -= count;
	g_wp->w__tbcol = w_ttcol (g_wp) + n;
	Si_dirty (0, LI - 2);
	for (i = 0; i <= LI-2; i++)
	    {
	    goalline = (ATTR *) g_goalscreen[i];
	    i_smear ((short)(SPACE | FIRST), CO, (short *) goalline);
	    goalline[CO] = '\0';
	    g_goalcrc[i] = g_uniqnum++;
	    }

	sendbox (0, w_lrline (g_wp), 0, w_lrcol(g_wp));
	i = obj_count (g_warray);
	for (g_wp = &g_warray[1]; g_wp < &g_warray[i]; g_wp++)
	    {
	    n = w_tbcol(g_wp) - w_ttcol(g_wp);
	    g_wp->w__ttcol -= count;
	    g_wp->w__tbcol = w_ttcol (g_wp) + n;
	    sendbox (0, w_lrline (g_wp), 0, w_lrcol(g_wp));
	    }

	g_wp = wp;
	}
    else if (w_ttcol (g_wp) + w_col (g_wp) < 0) /* right */
	{
	wp = g_wp;
	g_wp = g_warray;
	i = ((CO - 1) / 3) + 1;
	for (count = i; w_ttcol (wp) + w_col (wp) + count < 0;
	     count += i) ;

	n = w_tbcol(g_wp) - w_ttcol(g_wp);
	g_wp->w__ttcol += count;
	g_wp->w__tbcol = w_ttcol(g_wp) + n;
	Si_dirty (0, LI - 2);
	for (i = 0; i <= LI-2; i++)
	    {
	    goalline = (ATTR *) g_goalscreen[i];
	    i_smear ((short)(SPACE | FIRST), CO, (short *) goalline);
	    goalline[CO] = '\0';
	    g_goalcrc[i] = g_uniqnum++;
	    }

	sendbox (0, w_lrline (g_wp), 0, w_lrcol(g_wp));
	i = obj_count (g_warray);
	for (g_wp = &g_warray[1]; g_wp < &g_warray[i]; g_wp++)
	    {
	    n = w_tbcol(g_wp) - w_ttcol(g_wp);
	    g_wp->w__ttcol += count;
	    g_wp->w__tbcol = w_ttcol(g_wp) + n;
	    sendbox (0, w_lrline (g_wp), 0, w_lrcol(g_wp));
	    }

	g_wp = wp;
	}
#endif

    /* fix file name and tab display if no pop box */
    fixname ();
    if ( !g_popflag)
	fixtabs ();

    /* If display interrupted                   */
    if (Si_display (FALSE) == FALSE)
	{
	g_dispflag = DISPNORM;
	return;
	}

    /* fix cursor position */
    wp = g_wp;
    St_goto (w_ttline (wp) + w_line (wp), w_ttcol (wp) + w_col (wp));
    St_flush ();
    g_dispflag = DISPNORM;
    }

/****************************************************************************/
/* Ettyfix:  restores normal tty state (helper callable)                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_popflag,                                       */
/*                                                                          */
/* globals changed:        g_popflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     Ettyfix restores the terminal to normal (non-editor) state.          */
/*     It flushes the current window and sets the g_popflag global          */
/*     to FALSE and invokes Si_quit.                                        */
/*                                                                          */
/* NOTE:  This routine is specific to the TERMCAPS version of the editor.   */
/****************************************************************************/

void Ettyfix (void)
    {
#ifdef DEBUG
    debug ("Ettyfix called");
#endif

    flush_window ();          /* this is a safety precaution */
    g_popflag = FALSE;
    Si_quit ();
    }

/****************************************************************************/
/* Ettyunfix:  restores editor tty state (helper callable)                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_syncname, g_dispflag                           */
/*                                                                          */
/* globals changed:        g_dispflag                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     Ettyunfix restores the terminal to the state needed in the editor.   */
/*     In the TERMCAPS implementation, all that is necessary is to invoke   */
/*     Si_init and, if the file has not been sync'ed, Refresh.              */
/****************************************************************************/

void Ettyunfix (void)
    {
#ifdef DEBUG
    debug ("Ettyunfix called");
#endif

    Si_init ();
    if ( !g_syncname)     /* file has not been sync'ed */
	Refresh ();
    }

/******************************************************************************/
/* fixname:  update status line if necessary                                  */
/*                                                                            */
/* arguments:              none                                               */
/*                                                                            */
/* return value:           void                                               */
/*                                                                            */
/* globals referenced:     g_filename, g_spath, g_dispname, g_imodeflag,      */
/*			   g_warray, g_wp, LI, CO                             */
/*                                                                            */
/* globals changed:        none                                               */
/*                                                                            */
/* synopsis:                                                                  */
/*	This routine divides the status line into "fields" which are          */
/*      independently updated. A field is updated (and for the most part its  */
/*      contents are only computed) when the contents of the field has        */
/*	changed.                                                              */
#ifdef VBLSCR
/*      The field sizes are computed based on the width of the screen, and    */
/*      recomputed only if the screen size changes.  The algorithm used       */
/*      attempts to keep the most significant information visible as the      */
/*      screen size gets smaller.                                             */
#endif
/******************************************************************************/

void fixname (void)
{
    static char *dispfname;  /* space to store file/datapath */
    static char *odispfname; /* previous file/datapath */
    static ATTR *blanks;     /* source for blank padding */

    /* strings used by bottom line */
    static ATTR insert1 [10];
    static ATTR sie [10];
    static ATTR insertj [10];
    static ATTR sij [10];
    static ATTR overwrite [10];
    static ATTR soe [10];
    static ATTR overwritej [10];
    static ATTR soj [10];

    static ATTR line [5];
    static ATTR dots [4];
    static ATTR larrow[] = {FIRST | LAR,0};
    static ATTR rarrow[] = {FIRST | RAR,0};
    static ATTR uarrow[] = {FIRST | UAR,0};
    static ATTR darrow[] = {FIRST | DAR,0};
    static ATTR blank[] = {SPACE | FIRST,0};
    static ATTR openparen[] = {FIRST | '(',0};
    static ATTR closeparen[] = {FIRST | ')',0};

    /* information about state of line before update */
    static int wmode;     /* on exit = g_wmodeflag */
    static int imode;     /* on exit = g_imodeflag */
    static int ocurline;  /* previous cursor line number */
    static int onumlines; /* previous number of lines in field */
    static int olarrow, orarrow, ouarrow, odarrow; /* boolean for arrows */

    /* local variables */
    int dot_prefix;        /* TRUE if ... prefix to be used on file name */
    int blank_pad;         /* number of spaces to pad the file name with */
    int numlines, curline;
    int LIminus1 = LI - 1; /* optimization of common expression */
    int cuarrow, cdarrow, clarrow, crarrow; /* current arrow state */

    char *tmp;
    char *cp;
    char *endcp;
    WSTRUCT *wp;

#ifdef VBLSCR
    static int   oldCO;    /* used on initialization and to determine if
			      window width changed */

    static int wfilename;  /* width of file name field */
    static int ins_ovr;    /* TRUE abbreviate insert/overwrite indicator  */
    static int pins_ovr;   /* start column of insert/overwrite indicator */
    static int blineno;    /* TRUE abbreviate line numbers */
    static int pcurline;   /* start of line number */
    static int pnumlines;  /* start of number of lines in file field */
    static int arrows;     /* TRUE no blank spacing arround arrows */
    static int parrows;    /* start of arrow field */

    ATTR aline [8];
    ATTR *ap;              /* attr version of dispfname */
    int width = 0;
    int curend;            /* position available for next field */
    int blen;
    wchar_t wch;
#endif

#ifdef DEBUG
    debug ("enter fixname");
#endif

    /* find out if bottom line should be displayed. g_filename is NULL only
       if we aren't editing a file, which occurs before the editor has
       actually gotten started. In this case, it is safe to just ignore
       the request to update the bottom line. */

    if (g_filename == NULL)
	return;

#ifdef VBLSCR
    if (oldCO != CO || g_dispflag == DISPINIT)
	{
	width = CO;
#endif

    /* get symbol for tokens INSERT, OVERWRITE, Line, and '...' */
    if (dispfname == NULL)
	{

	/* The following code gets the symbol from the message catalog,
	   applies unpackup and uses the resulting ATTR form. */

	if (MultibyteCodeSet)
	    {
	    tmp = Egetmessage (M_EINSERTE, "IN/W_WRAP", FALSE);
	    attr_pad (tmp, insert1, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_ESINSERTE, "IW", FALSE);
	    attr_pad (tmp, sie, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_EINSERTJ, "IN/C_WRAP", FALSE);
	    attr_pad (tmp, insertj, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_ESINSERTJ, "IC", FALSE);
	    attr_pad (tmp, sij, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_EOVERWRITEE, "OV/W_WRAP", FALSE);
	    attr_pad (tmp, overwrite, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_ESOVERWRITEE, "OW", FALSE);
	    attr_pad (tmp, soe, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_EOVERWRITEJ, "OV/C_WRAP", FALSE);
	    attr_pad (tmp, overwritej, 9);
	    s_free (tmp);
	    tmp = Egetmessage (M_ESOVERWRITEJ, "OC", FALSE);
	    attr_pad (tmp, soj, 9);
	    s_free (tmp);
	    }
	else
	    {
	    tmp = Egetmessage (M_EINSERT, "INSERT", FALSE);
	    attr_pad (tmp, insert1, 9);
	    s_free (tmp);

	    tmp = Egetmessage (M_EOVERWRITE, "OVERWRITE", FALSE);
	    attr_pad (tmp, overwrite, 9);
	    s_free (tmp);
	    }

	tmp = Egetmessage (M_ELINE, "Line", FALSE);
	attr_pad (tmp, line, 4);
	s_free (tmp);

	tmp = Egetmessage (M_EDOTS, "...", FALSE);
	attr_pad (tmp, dots, 3);
	s_free (tmp);
    }
#ifdef VBLSCR
	/* minimum screen size rationally supported */
	if (width < 15)
	    width = 15;

	/* set up static variables indicating field width and positions
	   and set width of file name field. There has to be room for at
	   least 1 char. of file name plus 3 dots */

	if (width > 80)
	    wfilename = width - 40;
	else if (width > 59)
	    wfilename = 40;
	else if (width > 23)
	    wfilename = width - 20;
	else if (width > 20)
	    wfilename = 4;
	else
	    wfilename = 0;

	blen = (wfilename > width) ? wfilename : width;
#endif

    /* obtain scratch space, initialize a source of blanks, build
       invariant bottom line text, and force update of bottom line */

#ifdef VBLSCR
    if (dispfname == NULL || wfilename > obj_count (dispfname))
	{
	s_free (dispfname);
	dispfname = s_alloc (PATH_MAX, T_CHAR, NULL);
	s_free (odispfname);
	odispfname = s_alloc (PATH_MAX, T_CHAR, NULL);
	s_free ((char *)blanks);
	blanks = (ATTR *)s_alloc (blen, T_ATTR, NULL);
	}
    else
	blanks = (ATTR *)s_realloc ((char *)blanks, blen);

	odispfname [0] = '\0';
	i_smear ((short)(SPACE | FIRST), blen, (short *)blanks);

	/* blank the bottom line */
	Si_ovector (blanks, width, LIminus1, 0);
	curend = wfilename;

	/* set insert/overwrite flag information */
	ins_ovr = (width < 79);
	if (ins_ovr)
	    {
	    pins_ovr = curend + 1;
	    curend += 3;
	    }
	else
	    {
	    pins_ovr = curend;
	    curend += 10;
	    }

#ifdef DEBUG
    debug ("width = %d position = %d", width, curend);
#endif

	/* put in "Line" token if it fits */
	if (width > 70)
	    {
	    Si_ovector (line, 4, LIminus1, curend);
	    curend += 5;
	    }

	/* put in fields for line numbers */
	if (width > 65)
	    {
	    blineno = FALSE;
	    Si_ovector (openparen, 1, LIminus1, curend + 7);
	    Si_ovector (closeparen, 1, LIminus1, curend + 14);
	    pcurline = curend;
	    pnumlines = pcurline + 8;
	    curend += 15;
	    }
	else
	    {
	    blineno = TRUE;
	    Si_ovector (openparen, 1, LIminus1, curend + 5);
	    Si_ovector (closeparen, 1, LIminus1, curend + 11);
	    pcurline = curend;
	    pnumlines = pcurline + 6;
	    curend += 12;
	    }

	/* put in fields for arrows */
	parrows = curend + ((arrows = (width < 67)) ? 0 : 1);

	/* note new bottom line size */
	oldCO = CO;

	/* force update of these fileds */
	wmode = imode = ocurline = onumlines = ERROR;
	orarrow = olarrow = ouarrow = odarrow = FALSE;
	}
#else
    if (g_dispflag == DISPINIT || dispfname == NULL) {
	if (dispfname == NULL)
	    {
	    dispfname = s_alloc (PATH_MAX, T_CHAR, NULL);
	    odispfname = s_alloc (PATH_MAX, T_CHAR, NULL);
	    }
	odispfname [0] = '\0';
	if (blanks == NULL)
	    {
	    blanks = (ATTR *)s_alloc(DISPSIZE, T_ATTR, NULL);
	    i_smear ((short)(SPACE | FIRST), DISPSIZE, blanks);
	    }
	Si_ovector (line, 4, LIminus1, 51);
	Si_ovector (openparen, 1, LIminus1, 63);
	Si_ovector (closeparen, 1, LIminus1, 70);

	/* force update of these fields */
	wmode = imode = ocurline = onumlines = ERROR;
	}
#endif

    wp = g_wp;
    cp = dispfname;

    /* determine the contents of the file name field */
    if (g_dispname)
	{
	char *dname = g_dispname;
	int len, disp_width = 0;

	/* copy only what will fit of g_dispname */
	while (*dname) 
	    {
	    len = mbtowc(&wch, dname, MB_CUR_MAX);
	    /* copy at least 1 char */
	    if (len < 1) len = 1;
	    disp_width += wc_text_width(wch);
#ifdef VBLSCR
            if (disp_width > wfilename)
#else
            if (disp_width > DISPSIZE)
#endif
		break;
	    while (len--)
	    	*cp++ = *dname++;
	    }
	*cp = '\0';
	}
    else
	{
	char *fname = w_filename (wp) ? w_filename (wp) : g_filename;
	int len = obj_count (fname);

	if (len > obj_count (dispfname))
	    {
	    dispfname = s_realloc (dispfname, len);
	    odispfname = s_realloc (odispfname, len);
	    }

	/* first copy in the file name */
	while (*cp++ = *fname++);
	cp--;

	if (g_spath && *g_spath)  /* add data path information */
	    {
	    char *spath = g_spath;
	    int offset = cp - dispfname;

	    len = offset + 1 + obj_count (spath);
	    if (len > obj_count (dispfname))
		{
		dispfname = s_realloc (dispfname, len);
		odispfname = s_realloc (odispfname, len);
		}

	    cp = dispfname + offset;
	    *cp++ = DIR_SEPARATOR;

	    while (*cp++ = *spath++);
	    cp--;
	    }
	}

    /* file name changed or screen is reinitialized */
#ifdef VBLSCR
    if (wfilename && strcmp (dispfname, odispfname))
	{
	ap = unpackup (dispfname, 0);
	blank_pad = wfilename - attr_text_width (ap);
	dot_prefix = blank_pad < 0;
	(void) strcpy (odispfname, dispfname);
	if (dot_prefix)
	    {
	    char *aptr;
	    int disp_width, n, mlen;

	    Si_ovector (dots, 3, LIminus1, 0);
	    /* NOTE: blank_pad is negative here */

	    /* Ensure ap is truncated at a character boundary */
	    aptr = a2string(ap, i_strlen((short *)ap));
	    for (n = 0, disp_width = 0; disp_width < 3 - blank_pad;)
		{
		mlen = mbtowc(&wch, aptr + n, MB_CUR_MAX);
		/* move by at least 1 char */
		if (mlen < 1) mlen = 1;
		n += mlen;
		disp_width += wc_text_width(wch);
		}
	    free(aptr);

	    /* If end of field not character boundary (n>3), pad with blanks */
	    Si_ovector (ap + n, i_strlen(ap + n), LIminus1, 3);
	    blank_pad += disp_width - 3;
	    }
	else
	    Si_ovector (ap, wfilename, LIminus1, 0);

	if (blank_pad > 0)
	    Si_ovector (blanks, blank_pad, LIminus1, wfilename - blank_pad);
	s_free ((char *)ap);
	}

#else  /* NOT VBLSCR */

    if (strcmp (dispfname, odispfname)) {
	ATTR dispname [DISPSIZE];
	int i;
	int len;
	char *start;
	blank_pad = endcp - cp;
	dot_prefix = blank_pad < 0;

	(void) strcpy (odispfname, dispfname);

	if (blank_pad >= 0) {
	    len = DISPSIZE - blank_pad;
	    start = dispfname;
	} else {
	    len = DISPSIZE - 3;
	    start = dispfname - blank_pad + 3;
	}
        for (i=0; i<len; i++)
	    dispname [i] = start [i];
	start = (char *)dispname; /* future use just as an argument */
	if (dot_prefix) {
	    Si_ovector (dots, 3, LIminus1, 0);
	    /* NOTE: blank_pad is negative here */
	    Si_ovector (start, len, LIminus1, 3);
	} else
	    Si_ovector (start, len, LIminus1, 0);

	if (blank_pad > 0)
	    Si_ovector (blanks, blank_pad, LIminus1, len);
    }
#endif /* VBLSCR */

#ifdef DEBUG
    debug ("imode = %d g_imodeflag = %d", imode, g_imodeflag);
    debug ("insert = %s  overwrite = %s", insert1, overwrite);
#endif

    if (MultibyteCodeSet)
	{
	if (imode != g_imodeflag || wmode != g_wmodeflag)
	    {
	    if (g_wmodeflag == CHARWRAP)
		if (ins_ovr)
		    Si_ovector (g_imodeflag ? sij : soj, 2,
						    LIminus1, pins_ovr);
		else
		    Si_ovector (g_imodeflag ? insertj : overwritej, 9,
						    LIminus1, pins_ovr);
	    else
		if (ins_ovr)
		    Si_ovector (g_imodeflag ? sie : soe, 2,
					      LIminus1, pins_ovr);
		else
		    Si_ovector (g_imodeflag ? insert1 : overwrite, 9,
					      LIminus1, pins_ovr);
	    imode = g_imodeflag;
	    wmode = g_wmodeflag;
	    }
	}
    else
	{
	if (imode != g_imodeflag)
	    {
#ifdef VBLSCR
	    Si_ovector (g_imodeflag ? insert1 : overwrite, ins_ovr ? 1 : 9,
						      LIminus1, pins_ovr);
#else
	    Si_ovector (g_imodeflag ? insert1 : overwrite, 9, LIminus1, 41);
#endif
	     }

	imode = g_imodeflag;
	}

    /* Note: We can't call Enumlines if g_sfp isn't set up.
       This only seems to happen under obscure circumstances when
       saving ASCII files, but it hurts nothing to check... */

    numlines = g_sfp ? Enumlines("") : onumlines;
    curline = w_ftline (wp) + w_line (wp) + 1;

    if (curline != ocurline)
	{
#ifdef VBLSCR
	itoattr (curline, blineno ? 5 : 6, aline);
	Si_ovector (aline, blineno ? 5 : 6, LIminus1, pcurline);
#else
	itoattr (curline, 6, aline);
	Si_ovector (aline, 6, LIminus1, 56);
#endif
	ocurline = curline;
	}

    if (numlines != onumlines)
	{
#ifdef VBLSCR
	itoattr (numlines, blineno ? 5 : 6, aline);
	Si_ovector (aline, blineno ? 5 : 6, LIminus1, pnumlines);
#else
	itoattr (numlines, 6, aline);
	Si_ovector (aline, 6, LIminus1, 64);
#endif
	onumlines = numlines;
	}

    /* put out the arrow indicators */
    cuarrow = (w_ftline (wp) > 0);
    if (cuarrow != ouarrow) {
	if (cuarrow)
#ifdef VBLSCR
	    Si_ovector (uarrow, 1, LIminus1, parrows);
#else
	    Si_ovector (uarrow, 1, LIminus1, 72);
#endif
	else
#ifdef VBLSCR
	    Si_ovector (blank, 1, LIminus1, parrows);
#else
	    Si_ovector (blank, 1, LIminus1, 72);
#endif
	ouarrow = cuarrow;
    }
    
    cdarrow = (w_ftline (wp) + w_lrline (wp) + 1 < numlines);
    if (cdarrow != odarrow) {
	if (cdarrow)
#ifdef VBLSCR
	    Si_ovector (darrow, 1, LIminus1,
	      parrows + 1 + (arrows ? 0 : 1));
#else
	    Si_ovector (darrow, 1, LIminus1, 74);
#endif
	else
#ifdef VBLSCR
	    Si_ovector (blank, 1, LIminus1,
	      parrows + 1 + (arrows ? 0 : 1));
#else
	    Si_ovector (blank, 1, LIminus1, 74);
#endif
	odarrow = cdarrow;
    }

    clarrow = (w_ftcol (wp) > 0);
    if (clarrow != olarrow) {
	if (clarrow)
#ifdef VBLSCR
	    Si_ovector (larrow, 1, LIminus1,
	      parrows + 2 + (arrows ? 0 : 2));
#else
	    Si_ovector (larrow, 1, LIminus1, 76);
#endif
	else
#ifdef VBLSCR
	    Si_ovector (blank, 1, LIminus1,
	      parrows + 2 + (arrows ? 0 : 2));
#else
	    Si_ovector (blank, 1, LIminus1, 76);
#endif
	olarrow = clarrow;
    }

    crarrow = getstat (w_flags (wp), RIGHTARROW);
    if (crarrow != orarrow) {
	if (crarrow)
#ifdef VBLSCR
	    Si_ovector (rarrow, 1, LIminus1,
	      parrows + 3 + (arrows ? 0 : 3));
#else
	    Si_ovector (rarrow, 1, LIminus1, 78);
#endif
	else
#ifdef VBLSCR
	    Si_ovector (blank, 1, LIminus1,
	      parrows + 3 + (arrows ? 0 : 3));
#else
	    Si_ovector (blank, 1, LIminus1, 78);
#endif
	orarrow = crarrow;
    }
#ifdef VBLSCR
    if (CO < 20) { /*for narrow windows we must worry about automatic margins*/
	extern ATTR **g_goalscreen;

	if (auto_right_margin &&
	    !isattrspace(&g_goalscreen [LIminus1][CO-1]) )
	    Si_ovector (blank, 1, LIminus1, CO-1);
    }
#endif  /* VBLSCR */
}

/* itoattr: convert number to attr form blank padding to indicated length */

static void itoattr (int num, int len, ATTR *ptr)
{
    int i;

    /* It is assumed here that the number of digits written in the
       do loop is less or equal to the number of spaces written in
       the for loop.
    */
    for (i=0; i<len; i++)
	*ptr++ = SPACE | FIRST;
    *ptr-- = '\0';
    do {
	*ptr-- = ('0' + num % 10) | FIRST;
	num /= 10;
    } while (num);
}

/* attr_pad: convert indicated string to attr */
/* pad to indicated display width */
static void attr_pad (char *s, ATTR *a, int n)
{
    int i, j, width, len;
    short flag;
    wchar_t wch;

    /* copy the string */
    for (i = 0; i < n && *s; i += width)
	{
	len = mbtowc(&wch, s, MB_CUR_MAX);
	/* copy at least 1 char */
	if (len < 1) len = 1;
	width = wc_text_width(wch);
	flag = FIRST;
	for (j = len; j > 0; j--)
	    {
	    *a++ = *s++ | flag;
	    flag = 0;
	    }
	}

    /* If last character exceeded the width */
    if (i > n)
	{
	a -= len;
	i -= width;
	}

    /* blank pad it */
    if (i < n)
	for (;i<n;i++)
	    *a++ = SPACE | FIRST;
    *a = '\0';
}

/****************************************************************************/
/* fixtabs:  fixes tab display and margin/wrap-mode settings in terminal    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_dispflag                                 */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     fixtabs updates the display of tab stops and margins on the top line */
/*     of the screen.  It only does the update if the g_dispflag global is  */
/*     set to DISPINIT, or the tab stops or margins have changed since the  */
/*     last fixtabs, or the editor has moved into a new window, or the      */
/*     field coordinates of the upper left corner of the window have        */
/*     changed.                                                             */
/*                                                                          */
/*     fixtabs works by erasing the first line of the screen and then writ- */
/*     ing the margins and tabs into it.                                    */
/*                                                                          */
/****************************************************************************/

void fixtabs (void)
    {
    WSTRUCT *wp;          /* current window               */

    int lmargin;          /* left margin displayed?       */
    int rmargin;          /* right margin displayed?      */
    int nodisplay;        /* tabs or margins displayed?   */
    int tablen;           /* length of displayed tab rack */

    static WSTRUCT *owp;  /* old window                   */
    static ocrc;          /* old crc for g_goalscreen[0]  */
    static oftcol;        /* old ftcol                    */
    static olmar;         /* old left margin              */
    static ormar;         /* old right margin             */
    static onodisplay;    /* old nodisplay                */
#ifdef VBLSCR
    static oftline;      /* old ftline                   */
    static ottcol;       /* old ttcol                    */
    static ottline;      /* old ttline                   */
    int startcol, n;     /* screen start col. & # cols. in tab rack */
#endif
    ATTR *goalline;      /* same as g_goalscreen[0] */

#ifdef DEBUG
    debug ("fixtabs called");
#endif

    if (( !getstat (w_flags (&g_warray [0]), SHOWTABS)))
	{
	/* mark that we have been here so that when we come back to
	 * a window that does showtabs they will display
	 */
	owp = g_wp;
	return;
	}

    wp = g_wp;

    /* determine how much of the tab rack should be displayed, if any */
    if (w_tabs (wp))
	tablen = min(obj_count(w_tabs (wp))-w_ftcol (wp), w_lrcol (wp)+1);
    else
	tablen = 0;

    /* determine if left margin should be displayed */
    lmargin = ((w_lmar (wp) || w_rmar (wp)) &&
	       (w_lmar (wp) >= w_ftcol (wp)) &&
	       (w_lmar (wp) <= (w_lrcol (wp) + w_ftcol (wp))));

    /* determine if right margin should be displayed */
    rmargin = ((w_rmar (wp)) &&
	       (w_rmar (wp) >= w_ftcol (wp)) &&
	       (w_rmar (wp) <= (w_lrcol (wp) + w_ftcol (wp))));

    /* determine if any tabs or margins should be displayed */
    nodisplay = (tablen <= 0) && !lmargin && !rmargin;

    if (g_dispflag != DISPINIT)
	{
	/* has the tab rack changed? */

	if ((wp == owp) &&             /* same window as before         */
	    (g_goalcrc[0] == ocrc) &&  /* tabs haven't changed          */
	    (w_lmar (wp) == olmar) &&  /* margins haven't changed       */
	    (w_rmar (wp) == ormar) &&
#ifdef VBLSCR
	    (w_ttcol(wp) == ottcol) && /* form hasn't shifted on screen */
	    (w_ttline(wp) == ottline) && /* form hasn't shifted on screen */
	    (oftline == w_ftline (wp)) &&   /* and field hasn't shifted     */
#endif
	    (w_ftcol (wp) == oftcol))   /* and screen hasn't shifted     */
	    return;

	/* if no displayed tabs or margins, this time and last time   */
	if (nodisplay && onodisplay && g_goalcrc[0] == ocrc)
	    return;
	}

#ifdef DEBUG
    debug ("fixtabs:  redisplaying tabs");
#endif

    /* blank out tabline, regardless of eventual display        */
    goalline = (ATTR *) g_goalscreen[0];
    i_smear((short)(SPACE | FIRST), CO, (short *) goalline);
    goalline[CO] = '\0';
    g_goalcrc[0] = ++g_uniqnum;
    Si_dirty(0,0);

    /* display tab line */
    if (tablen > 0)
#ifdef VBLSCR
	{
	startcol = w_ttcol(wp) < 0 ? 0 : w_ttcol(wp);
	n = min(tablen, w_lrcol(wp) + 1);
	n = min(n, ((w_tbcol(wp) >= 0) ? w_tbcol(wp) + 1 : 0));
	if (n + startcol > CO)
	    n = CO - startcol;
	Si_osvector (w_tabs(wp) + w_ftcol(wp) +
	    (w_ttcol (wp) > 0 ? 0 : -w_ttcol (wp)), n, 0, startcol);
	}

    n = w_ttcol (wp) + w_lmar (wp) - w_ftcol (wp);
    if (lmargin && n >= 0 && n < CO)
	Si_osvector ("l", 1, 0, w_ttcol (wp) + w_lmar (wp) - w_ftcol (wp));

    n = w_ttcol (wp) + w_rmar (wp) - w_ftcol (wp);
    if (rmargin && n >= 0 && n  < CO)
	Si_osvector ("r", 1, 0, w_ttcol (wp) + w_rmar (wp) - w_ftcol (wp));
#else
	Si_osvector (w_tabs (wp) + w_ftcol (wp), min (tablen, w_lrcol (wp) + 1),
	    0, w_ttcol (wp));

    if (lmargin)
	Si_osvector ("l", 1, 0, w_ttcol (wp) + w_lmar (wp) - w_ftcol (wp));

    if (rmargin)
	Si_osvector ("r", 1, 0, w_ttcol (wp) + w_rmar (wp) - w_ftcol (wp));
#endif

    owp        = wp;
    ocrc       = g_goalcrc[0];
    olmar      = w_lmar (wp);
    ormar      = w_rmar (wp);
    oftcol     = w_ftcol (wp);
    onodisplay = nodisplay;
    }

/****************************************************************************/
/* tmclose:  setting up terminal for exit                                   */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_ttyflag                                        */
/*                                                                          */
/* globals changed:        g_ttyflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     tmclose closes down the terminal in preparation for leaving the      */
/*     editor.  If the g_ttyflag global is not set to TRUE, it does nothing */
/*     Otherwise it sets g_ttyflag to FALSE and invokes Si_quit.            */
/*                                                                          */
/* NOTE:  This routine is specific to the TERMCAPS version of the editor.   */
/****************************************************************************/

void tmclose (void)
    {

#ifdef DEBUG
    debug ("tmclose called");
#endif

    if ( !g_ttyflag)
	return;

    g_ttyflag = FALSE;
    Si_quit ();
    }

/****************************************************************************/
/* tmopen:  initializes the terminal                                        */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_ttyflag                                        */
/*                                                                          */
/* globals changed:        g_ttyflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     tmopen initializes the terminal for use by the editor.  It           */
/*     invokes Si_init and sets the g_ttyflag global to TRUE.               */
/*                                                                          */
/* NOTE:  This routine is specific to the TERMCAPS version of the editor.   */
/****************************************************************************/

void tmopen (void)
    {
    Si_init ();
    g_ttyflag = TRUE;
    }

