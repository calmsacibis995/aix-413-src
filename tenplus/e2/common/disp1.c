static char sccsid[] = "@(#)29	1.19  src/tenplus/e2/common/disp1.c, tenplus, tenplus411, GOLD410 3/9/94 12:01:53";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:   Si_init, Si_quit, Si_dirty, Si_ovector, Si_osvector,
 *              Si_display, dodots, repaint, scr_seq,
 *		setattr, Si_redisplay,
 *		term_enter, term_exit, winsize
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
/* File: disp1.c - first part of a terminal independent, interruptible      */
/* goal-screen display.                                                     */
/****************************************************************************/

#include "def.h"

#include <sys/ioctl.h>
extern	char *	termdef(int, char);
#include <sys/errno.h>
#include <termio.h>
#include <fcntl.h>

#include "termcap.h"
#include "keynames.h"

       int g_dirty;           /* says there is a dirty region           */
static int topdlin;           /* the current dirty-region               */
static int botdlin;
static int g_opop = 0;        /* last pop state to make popboxes vanish */
static int climit;            /* number of chars needed before a flush
				 OR ERROR */

ATTR    **g_goalscreen;       /* the goal-screen                        */
int      *g_goalcrc;          /* and its per line crcs                  */
ATTR    **g_curscreen;        /* the current screen                     */
int      *g_curcrc;           /* and its per line crcs                  */
int       g_uniqnum;          /* the unique number, used in scrolling   */
int       isafile;            /* determine structure for standard out   */
short     g_ospeed;           /* output speed, from IOCTL               */
int       g_St_noutput;
int       g_St_fd;            /* non-blocking descriptor for
				 interruptability */

static char termbuf [BUFSIZ];   /* buffer for terminal output   */
static struct termio oldmodes;  /* ioctl modes for the standard out */
static struct termio runmodes;
static struct termio oldmodes2;  /* ioctl modes for the standard error */
static struct termio runmodes2;  /* if stderr is redirected to a file. */

/* number of chars / sec at various speeds
   used to make interruptability work smoothly i.e. not to do
   do tons of unwanted flushes at high speed */

static int cps [] =
{0, 5, 8, 11, 13, 15, 20, 30, 60, 120, 180, 240, 500, 1000};

/* symbolic names for parts of struct termio    */
#define INTRCHAR  0
#define QUITCHAR  1
#define ERASECHAR 2
#define KILLCHAR  3
#define EOFCHAR   4
#define EOLCHAR   5

void dummy_enter(void)
{
    enter();
}

/****************************************************************************/
/* Si_init: initializes the display module                                  */
/* Calls St_init to do the terminal-dependent initialization.               */
/* Does the UNIX dependent initialization here.                             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     LI, CO, g_goalscreen, g_goalcrc,                 */
/*                         g_curscreen, g_curcrc, oldmodes, runmodes,       */
/*                         oldmodes2, runmodes2,                            */
/*                         g_ospeed, g_St_noutput, g_St_fd,                 */
/*                         g_dirty, topdlin, botdlin,                       */
/*                                                                          */
/* globals changed:        g_goalscreen,g_goalcrc,                          */
/*                         g_curscreen, g_curcrc, oldmodes, runmodes,       */
/*                         oldmodes2, runmodes2,                            */
/*                         g_ospeed, g_St_noutput, g_St_fd,                 */
/*                         g_dirty, topdlin, botdlin                        */
/* synopsis:                                                                */
/*     Si_init does the terminal-independent part of initializing the       */
/*     screen.                                                              */
/*                                                                          */
/*     If the goalscreen global is already initialized, all that Si_init    */
/*     needs to do is two ioctl calls to reset channel 2 and the g_St_fd    */
/*     channel, and then invoke St_init to do whatever terminal-dependent   */
/*     initialization is necessary.  If the goalscreen global is NULL,      */
/*     it is necessary to set up all the screen-, tty-, and dirt-related    */
/*     globals before proceeding as above.                                  */
/*                                                                          */
/*     When the goalscreen is NULL, Si_init asks the termcaps library       */
/*     how big the screen is, allocates a g_curscreen and goalscreen        */
/*     of the right size, and fills them both with spaces.  The g_curcrc    */
/*     and g_goalcrc arrays are initialized to NULL to indicate those lines */
/*     need not be refreshed yet.                                           */
/*                                                                          */
/*     Si_init initializes the tty globals oldmodes, runmodes, g_ospeed, an */
/*     g_St_fd using ioctl calls to find the current modes, tweak them as   */
/*     required, and set them again.                                        */
/*                                                                          */
/*     Initialization of oldmodes2 and runmodes2 is done if stderr is       */
/*     redirected to a file.                                                */
/*                                                                          */
/*     The variables g_dirty, topdlin and botdlin are                       */
/*     set to indicate that no part of the screen is dirty yet.             */
/****************************************************************************/

void Si_init (void)
{
    int i;
    int h, w;

    /* read Terminfo */
    if (g_goalscreen == NULL)
	{

	St_getcap ();
	w = atoi (termdef (fileno (stdin), 'c'));
	h = atoi (termdef (fileno (stdin), 'l'));
	if ((w == 0) || (h == 0))
	    {

	    /* set up screens to correspond to this terminal */
	    h = LI;
	    w = CO;
	    }
	else
	    {

	    /* get screen size into h and w */
	    LI = h;
	    CO = w;
	    }
	
	if (w > 255)
	    w = CO = 255;
	
	/* allocate and set up the 2 screen images          */
	g_goalscreen = (ATTR **) s_alloc (h, T_POINTER, "goalscreen");

	g_goalcrc    = (int *) s_alloc (h * sizeof (int), T_CHAR, "goalcrc");
	g_curscreen  = (ATTR **) s_alloc (h, T_POINTER, "curscreen");

	/* for echo-to-break processing                             */
	g_curcrc     = (int *) s_alloc (h * sizeof (int), T_CHAR, "curcrc");
	for (i = 0; i < h; i++)
	    {
	    g_goalscreen [i] = (ATTR *) s_alloc (w + 2, T_ATTR, NULL);
	    g_curscreen  [i] = (ATTR *) s_alloc (w + 2, T_ATTR, NULL);
	    }

	/* next, set up the UNIX terminal modes */
	setbuf (stdout, termbuf);

	/* set up output speed for libtermcap, default for stderr is tty */
	isafile = FALSE;

	errno = 0;
	ioctl (2, TCGETA, & oldmodes);
	ioctl (2, TCGETA, & runmodes);

	/* If ioctl returns errno equal to ENOTTY then stderr
	   is getting redirected to a file instead of to the screen.
	   If this is the case, we must set up a separte structure
	   for stdout. */
 
	if (errno == ENOTTY)
	    {
	    isafile = TRUE;
	    oldmodes2 = oldmodes;
	    runmodes2 = runmodes;
	    ioctl (1, TCGETA, & oldmodes);
	    ioctl (1, TCGETA, & runmodes);
	    }

	runmodes.c_lflag &= ~ (ECHO | ECHONL | ICANON);
	runmodes.c_iflag &= ~ (INLCR | IGNCR | ICRNL);
        runmodes.c_iflag &= ~ISTRIP;

	/* turn off DOS editing line discipline */
	runmodes.c_line = 0;

	/* does terminal perform XON/XOFF flow control */
	if (xon_xoff)
	    runmodes.c_iflag &= (~IXON);
	else
	    runmodes.c_iflag |= (IXON);

	runmodes.c_cc [ERASECHAR] = 0200;
	runmodes.c_cc [KILLCHAR] = 0200;
	runmodes.c_cc [EOFCHAR] = 1;  /* min # of chars read  */
	runmodes.c_cc [EOLCHAR] = 0;  /* no timeouts          */

	g_St_fd = open ("/dev/tty", O_NDELAY);
	if (g_St_fd == -1)
	     fatal ("Si_init: Cannot open /dev/tty");

	g_ospeed = runmodes.c_cflag & CBAUD;

	/* set up number of bytes required to trigger output flushing
	   set at about 1/2 second's worth */

	climit = cps [g_ospeed] >> 1;

	g_St_noutput = 0;
	g_dirty = FALSE;    /* screen marked all OK so far  */
	topdlin = 0;
	botdlin = 0;

	/* set up command dispatcher kludge */
	g_edfunction [EXECUTE] = (void(*)(int, char *, int))dummy_enter;
	}

    /* If stderr is a file, use runmodes2 structure */
    ioctl (g_St_fd, TCSETAW, & runmodes);
    if (isafile)
       ioctl (2, TCSETAW, & runmodes2);
    else
       ioctl (2, TCSETAW, & runmodes);

    /* throw out the init seq, and clear the screen */
    St_init ();

    for (i = 0; i < LI; i++)
	{
	i_smear ((short)(SPACE | FIRST), CO, (short *)g_goalscreen [i]);
	i_smear ((short)(SPACE | FIRST), CO, (short *)g_curscreen [i]);

	/* set up the current crc so we don't jackpot into
	   refreshing a line of spaces */

	g_curcrc [i] = 0;
	}

}

/****************************************************************************/
/* Si_quit: flush tty buffer, free statics, reset tty                       */
/*                                                                         */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_goalscreen, oldmodes, oldmodes2                */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the goalscreen global is NULL, do nothing.  Otherwise invoke      */
/*     St_quit to handle any terminal-dependent issues, flush standard      */
/*     output, and do an ioctl to restore the old terminal modes.           */
/****************************************************************************/

void Si_quit (void)
{

    /* do any terminal dependent cleanup    */
    if (g_goalscreen)
	{
	St_quit ();
	(void) fflush (stdout);
	ioctl ( 1, TCSETAF, & oldmodes);
	if (isafile)
	   ioctl ( 2, TCSETAF, & oldmodes2);
	else
           ioctl (2, TCSETAF, & oldmodes);
	}
}

/****************************************************************************/
/* Si_dirty: expand dirty region to cover new box                           */
/* clips region to screen size                                              */
/*                                                                          */
/* arguments:              int topl, botl                                   */ 
/*                                 lines of new dirty region to add in      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_dirty, topdlin, botdlin                        */
/*                                                                          */
/* globals changed:        g_dirty, topdlin, botdlin                        */ 
/*                                                                          */
/* synopsis:                                                                */
/*     If the global g_dirty flag is already set, set the topdlin global    */
/*     to the minimum of its current value and the new one.                 */
/*     Set the botdlin global to the maximum of its current value and the   */
/*     new one, thus performing a box union operation                       */
/*                                                                          */
/*     If the global g_dirty flag is not set, set the dirty globals to the  */
/*     corresponding input region and set g_dirty to TRUE.                  */
/****************************************************************************/

Si_dirty (int topl, int botl)
    {

#ifdef  DEBUG
    debug ("Si_dirty (topl=%d, botl=%d)", topl, botl);
#endif

    if (g_dirty)
	{
	if (topdlin > topl)
	    topdlin = topl;
	if (botdlin < botl)
	    botdlin = botl;
	}
    else
	{
	topdlin = topl;
	botdlin = botl;
	g_dirty = TRUE;
	}

#ifdef  DEBUG
    debug ("dirty region now (topdlin=%d, botdlin=%d)", topdlin, botdlin);
#endif
    }

/****************************************************************************/
/* Si_ovector: overwrite char vector at given line and column               */
/*             into the goalscreen                                          */
/*                                                                          */
/* arguments:              ATTR *vector - string to overwrite               */
/*                         int   n      - number of characters in string    */
/*                         int   line,  - line number in g_goalscreen       */
/*                         int   column - column number in g_goalscreen     */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     LI, CO, g_goalscreen, g_goalcrc, g_uniqnum       */
/*                                                                          */
/* globals changed:        g_goalscreen, g_goalcrc, g_uniqnum               */
/*                                                                          */
/* synopsis:                                                                */
/*     Compare the starting line and column to the screen dimensions and    */
/*     issue a fatal error if the vector is off-screen.                     */
/*     Set the g_goalcrc of the line to a new unique number and calculate   */
/*     the line position from the supplied column. Finally write the string */
/*     into the goalscreen at the supplied line and calculated position     */
/****************************************************************************/

void Si_ovector (ATTR *vector, int n, int line, int column)
    {
    ATTR *glp;        /* current g_goalscreen line                */
    ATTR *newgoalline;/* new line to replace the g_goalscreen line*/
    ATTR *ocp;        /* index into lp                            */
    ATTR *ovcp;       /* index into vector                        */
    ATTR *nocp;       /* index into newgoalline                   */

    /* following are all expressed in display columns */
    int vwidth;       /* width of vector                                 */
    int vwidth_buf;   /* temporary holder of width of vector             */
    int glwidth;      /* width of complete multiAttrs before column      */
    int right_column; /* column plus vwidth                              */
    int grwidth;      /* width of complete multiAttrs after right_column */
    int gstartcol;    /* width of complete multiAttrs of column 
				after right_column                       */
    int tempcol;      /* temporary index of columns                      */

    /* following are Attr positions on the line */
    int gleftpos;     /* corresponds to glwidth                   */
    int grightpos;    /* corresponds to grwidth                   */
    int gstartpos;    /* corresponds to gstartcol                 */
    int gendpos;      /* last position in glp of Attrs to transfer*/

    int totalattrs;   /* holds total number Attrs required in new line  */

    int  nxfer;
    int  position;
    int glwidth_invalid;

#ifdef  DEBUG
    char *vect_char;
    vect_char = a2string(vector, n);
    debug ("Si_ovector: '%s', len=%d,(line=%d, col=%d), brkflag = %d",
	     vect_char, n, line, column, g_brkflag);
    s_free(vect_char);
#endif

#ifdef CAREFUL
    if (line >= LI || column >= CO || line < 0 || column < 0)
	return;
#endif

    /* get the goal line */
    glp              = (ATTR *) g_goalscreen[line];
    g_goalcrc [line] = ++g_uniqnum;
    Si_dirty (line, line);

    if (MultibyteCodeSet || (__max_disp_width > 1))
        {

        gleftpos = calc_line_position2(glp, column, &glwidth);
        glwidth_invalid = ((glp[gleftpos] == (ATTR)0) &&
                           (glwidth < column));
        ovcp = vector;
        vwidth = 0;
        while ((ovcp < vector+n) && *ovcp)
            {
	    vwidth_buf = vwidth;
            vwidth += char_width(ovcp);
            if (column + vwidth > CO) 
                {  /* too big for the screen */
                vwidth = vwidth_buf;
                break;
                }
            skipattr(&ovcp);
            }
        n = ovcp - vector;

        right_column = column + vwidth;
        grightpos = calc_line_position2(glp, right_column,&grwidth);
 
        gstartpos = grightpos;
        gstartcol = tempcol = grwidth;
        if ((grwidth < right_column) &&
            (glp[grightpos] != (ATTR)0))
	    {
            /* in the middle of a multi-attr? */
            skipattr_idx(glp, &gstartpos);
            gstartcol = tempcol = grwidth + char_width(&glp[grightpos]);
            }

        gendpos = gstartpos + i_strlen((short *)&glp[gstartpos]);
  
        /* note: assumes g_place_filler takes one byte and one display col */
        totalattrs = gleftpos + (column - glwidth) + n +
                     max(0, gstartcol - right_column) + (gendpos - gstartpos);

        newgoalline = (ATTR *)s_alloc(totalattrs + 2,T_ATTR,NULL);
        i_smear((short)(SPACE | FIRST), totalattrs, (short *)newgoalline);

#ifdef  DEBUG
    debug("Si_ovector: Multibyte calculations are:vwidth %d glwidth %d right_column %d grwidth %d ",
         vwidth ,glwidth ,right_column ,grwidth );
    debug("Si_ovector: Multibyte calculations are:gstartcol %d tempcol %d gleftpos %d grightpos %d gstartpos %d gendpos %d totalattrs %d",
         gstartcol ,tempcol ,gleftpos,
         grightpos ,gstartpos ,gendpos ,totalattrs);
#endif

        ovcp = vector;
        ocp = glp;
        nocp = newgoalline;
        
        /* copy Attrs before column */
        while (ocp < glp + gleftpos)
            *nocp++ = *ocp++;

        /* fill leading 1/2 multi Attr with g_place_filler if required */
        for ( tempcol = glwidth; tempcol < column; tempcol++)
            *nocp++ = glwidth_invalid ? SPACE | FIRST : g_place_filler;

        /* copy contents of vector */
        nxfer = n;
        while (nxfer--)
            *nocp++ = *ovcp++;

        /* fill trailing 1/2 multi Attr with g_place_filler if required */
        for (tempcol = right_column; tempcol < gstartcol; tempcol++)
            *nocp++ = g_place_filler;

        /* copy across remaining part of glp if any */
        nxfer = gendpos - gstartpos;
        ocp = &glp[gstartpos];
        while (nxfer--)
            *nocp++ = *ocp++;

        s_free((char *)g_goalscreen[line]);
        g_goalscreen[line] = newgoalline;

        } 
    else 
        {  /* single byte and display width of 1 column path */

        ocp  = &glp[column];
        ovcp = vector;

        nxfer = (n <= CO - column) ? n : CO - column;
        while (nxfer--)
	    *ocp++ = *ovcp++;
        }
    }

/****************************************************************************/
/* Si_osvector:  convert a string to ATTR and display it                    */
/*                                                                          */
/* arguments:              char *str    - to convert and display            */
/*                         int   n      - number of chars in vector         */
/*                         int   line, position -                           */
/*                                        starting position of vector       */
/*                                                                          */
/* return value:         void                                               */
/*                                                                          */
/* globals referenced:   none                                               */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*   Si_osvector converts a string from char * to ATTR * and then feeds     */
/*   the converted string to Si_ovector with the remaining arguments.       */
/****************************************************************************/

void Si_osvector (char *str, int n, int line, int column)

    {
    ATTR *astr;

    /* let Si_ovector worry about width */
    astr = unpackup (str, 0);
    Si_ovector (astr, n, line, column);

    s_free ((char *)astr);
    }

/****************************************************************************/
/* Si_display: make g_curscreen be like g_goalscreen, interruptably.        */
/* If forceflag is TRUE, pending input will not interrupt the display.      */
/*                                                                          */
/* arguments:              int forceflag - TRUE if display should not be    */
/*                                         interrupted                      */
/*                                                                          */
/* return value:           int - TRUE if display was completed,             */
/*                               FALSE if it was interrupted                */
/*                                                                          */
/* globals referenced:     g_dirty, topdlin, g_curcrc, g_goalcrc,           */
/*                         g_St_noutput, g_popflag                          */
/*                                                                          */
/* globals changed:        g_curcrc, topdlin, g_St_noutput, botdlin,        */
/*                         g_dirty                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     Si_display loops through the dirty lines of the screen from topdlin  */
/*     down to botdlin.  At each line, if the forceflag was not set and     */
/*     input is waiting, return FALSE immediately.  Otherwise fix the line  */
/*     as follows:                                                          */
/*                                                                          */
/*         If the g_goalcrc and g_curcrc values for the line agree, it is   */
/*         fixed.                                                           */
/*         Otherwise invoke repaint to actually copy over the dirty chars.  */
/*                                                                          */
/*     After each dirty line is fixed, topdlin is decremented.  Thus, if    */
/*     the display is interrupted, the next time Si_display is called it    */
/*     will be able to resume exactly where it left off with no wasted      */
/*     effort.                                                              */
/*                                                                          */
/*     If the loop terminates with all lines fixed, mark the entire screen  */
/*     as clean by resetting the g_dirty and botdlin, globals and           */
/*     return TRUE.                                                         */
/****************************************************************************/

int Si_display (int forceflag)
    {
    int line;
    extern int g_popflag;

    if (g_dirty == FALSE)
	return(TRUE);

#ifdef  DEBUG
    debug ("Si_display (%d) called", forceflag);
#endif

    /* loop over all the lines updating them    */
    for (line = topdlin; line <= botdlin; line++)
	{

	/* If we have a text key display it since with normal typing there
	   never is any input_waiting. This saves a non-blocking read. */

	if (!forceflag && g_keyvalue != TEXT_KEY && input_waiting ())
	    {
	    if (! g_opop)
		g_opop = g_popflag;

	    return (FALSE);
	    }

#ifdef  DEBUG
    debug ("line %d g_curcrc = %d, g_goalcrc = %d",
		line, g_curcrc [line], g_goalcrc [line]);

    if (line < botdlin)
	debug ("line %d g_curcrc = %d, g_goalcrc = %d",
		line+1, g_curcrc [line+1], g_goalcrc [line+1]);
#endif

	/* Do quick crc check to see if the goal line is
	   currently displayed in the right place already.
	   This has the same effect on the g_curcrc vector as it has
	   on the screen otherwise we have to repaint the dirty part
	   of the line */

	if (g_curcrc [line] != g_goalcrc [line])
	    repaint (line);

	g_curcrc [line] = g_goalcrc [line];
	topdlin++;
	}

    /* flush any remaining chars now    */
    if (g_St_noutput)
	{
	(void) fflush (stdout);
	g_St_noutput = 0;
	}

    g_dirty = FALSE;
    botdlin = 0;
    g_opop  = g_popflag;

    return (TRUE);
    }

/****************************************************************************/
/* dodots:  fill in the dots of one line of a dotted field.                 */
/*                                                                          */
/* arguments:           int     line     - screen line number to be dotted  */
/*                      WSTRUCT *wp      - current window                   */
/*                                                                          */
/* return value:        none                                                */
/*                                                                          */
/* globals referenced:  g_dotchar, g_goalscreen                             */
/*                                                                          */
/* globals changed:     none                                                */
/*                                                                          */
/* synopsis:                                                                */
/*   dodots substitutes DOTCHARs for trailing spaces on the given line in t */
/*   given window.                                                          */
/****************************************************************************/

void dodots (int line, WSTRUCT *wp)
{

    int  rightloc;             /* location of right in tv line */
    int  leftloc;              /* location of left in tv line  */

    ATTR *scp = NULL;          /* left of modified region */
    ATTR *ecp = NULL;          /* right of modified region */
    ATTR *gline;               /* display contents */

    extern ATTR g_dotchar;     /* character to use for the fill operation */

    gline    = (ATTR *)g_goalscreen[line + w_ttline (wp)];
    leftloc  = calc_line_position(gline, w_ttcol(wp));
    rightloc = calc_line_position(gline, (min(CO-1, w_tbcol(wp))));

    /* dot fill from end of line to first non blank character */
    while (rightloc >= leftloc &&
	  (de_attr(gline[rightloc]) == SPACE ||
	   de_attr(gline[rightloc]) == g_dotchar))
	   {
	   if (ecp == NULL && de_attr(gline[rightloc]) == SPACE)
	       ecp = &gline[rightloc];

	   gline[rightloc--] = (g_dotchar | FIRST);
	   }

    /* clear out old dots */
    while (leftloc <= rightloc)
	{
	if (de_attr(gline[leftloc]) == g_dotchar)
	    {
	    if (scp == NULL)
		scp = &gline[leftloc];

	    gline[leftloc] = (SPACE | FIRST);
	    }
	leftloc++;
	}

    /* mark as dirty so will be properly displayed */
    if (scp || ecp)
	Si_dirty(line+w_ttline(wp), line+w_ttline(wp));
}

/****************************************************************************/
/* repaint: actually put characters on the screen                           */
/*                                                                          */
/* arguments:              int line - line number of dirty line to repaint  */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_goalscreen, g_curscreen,  CO                   */
/*                                                                          */
/* globals changed:        g_curscreen                                      */
/* synopsis:                                                                */
/*       update the screen with changed characters and set g_curscreen      */
/*       equal to g_goalscreen.                                             */
/*                                                                          */
/****************************************************************************/

void repaint (int line)
{
    ATTR *curline, *goalline;    /* ptrs to the goal & current lines */
    char *curchar, *goalchar;    /* 'char' versions of above */
    int cur_bytes, goal_bytes;   /* num bytes in current char */
    int cur_width, goal_width;   /* width of current char */
    int cur_pos, goal_pos;       /* index into line arrays */
    int cur_col, goal_col;       /* column equivalents of cur_pos, goal_pos */
    wchar_t cur_wch, goal_wch;   /* wide char equivalents of current char */

    int diff = FALSE;              /* TRUE indicates lines are different */
    int count;

#ifdef  DEBUG
    debug ("repaint: line=%d", line);
#endif

    goalline = (ATTR *) g_goalscreen[line];
    curline  = (ATTR *) g_curscreen[line];

    /* Compare the 2 text lines to find out what the differences are.
       Multibyte lines are compared column by column and
       single byte lines a byte at a time. */
    diff = FALSE;
    if (MultibyteCodeSet)
	{
	goalchar = a2string(goalline, i_strlen((short *)goalline));
	curchar = a2string(curline, i_strlen((short *)curline));

	cur_col = goal_col = cur_pos = goal_pos = 0;

	cur_bytes = mbtowc(&cur_wch, curchar, MB_CUR_MAX);
	/* copy at least 1 char */
	if (cur_bytes < 1) cur_bytes = 1;
	cur_width = wc_text_width(cur_wch);

	while (goal_col < CO)
	    {

	    goal_bytes = mbtowc(&goal_wch, &goalchar[goal_pos], MB_CUR_MAX);
	    /* copy at least 1 char */
	    if (goal_bytes < 0) goal_bytes = 1;
	    if (goal_bytes == 0)
	        break;
	    goal_width = wc_text_width(goal_wch);

	    if (cur_col != goal_col || cur_bytes != goal_bytes || 
	            !i_strncmp((short *) &goalline[goal_pos],
	            (short *) &curline[cur_pos], goal_bytes))
	        {
	        diff = TRUE;
	        if (is_graphic(goalline[goal_pos]))
		    attr_set (goalline[goal_pos], GRAPHIC);

	        St_goto(line, goal_col);
	        St_outchar(&goalline[goal_pos], goal_bytes);
	        }

	    goal_pos += goal_bytes;
	    goal_col += goal_width;
	    while (cur_col < goal_col)
		{
		cur_pos += cur_bytes;
		cur_col += cur_width;

		cur_bytes = mbtowc(&cur_wch, &curchar[cur_pos], MB_CUR_MAX);
		/* copy at least 1 char */
		if (cur_bytes < 1) cur_bytes = 1;
		cur_width = wc_text_width(cur_wch);
		}

	    }

	s_free(goalchar);
	s_free(curchar);
	}

    else

	{
	goal_col = 0;
	while (goal_col < CO)
	    {

	    if (goalline[goal_col] != curline[goal_col])
		{
		diff = TRUE;
		if (is_graphic(goalline[goal_col]))
		    attr_set (goalline[goal_col], GRAPHIC);

		St_goto(line, goal_col);
		St_outchar(&goalline[goal_col], 1);
		}

	    goal_col++;
	    }
	}

    /* overwrite the current line with the goal line (if different ) */
    if (diff == TRUE) 
	{
	count = obj_count(goalline);
	if (obj_count(curline) < count)
	    {
	    /*
	    s_free((char *) g_curscreen[line]);
	    g_curscreen[line] = (ATTR *) s_alloc(count, T_ATTR, NULL);
	    */
	    g_curscreen[line] = (ATTR *) i_realloc((short *) curline, count);
	    }
	(void) i_strcpy((short *) g_curscreen[line], (short *) goalline);
	}

    /* flush if required */
    if (g_St_noutput > climit)
	{
	(void) fflush (stdout);
	g_St_noutput = 0;
	}
}

/****************************************************************************/
/* scr_seq: seq analog that ignores DELIMIT attribute and                   */
/* understands graphic bit.                                                 */
/****************************************************************************/

int scr_seq (ATTR *s1, ATTR *s2)
{

    ATTR c;

    if ((s1 == NULL) || (*s1 == (ATTR) 0))
	return ((s2 == NULL) || (*s2 == (ATTR) 0));

    if ((s2 == NULL) || (*s2 == (ATTR) 0))
	return (FALSE);

    while (*s1 && *s2)
	{
	c = *s1++;

	if (is_graphic (c))
	    attr_set (c, GRAPHIC);

	if (c != undelimit(*s2))
	    return (FALSE);
	s2++;
	}

    return (! (*s1 || *s2));   /* YES if both got to null together */
}

/****************************************************************************/
/* setattr: set attribute of of line after mode changing char is sent       */
/* converts the attribute of all characters in line from start to end, or   */
/* up to a DELIMIT attr to the attribute of newc.                           */
/****************************************************************************/

int setattr (ATTR *line, int start, int end, ATTR newc)

{
    ATTR attr = get_attr (newc);
    int i;

    /* clears attr of start column, will be reset by calling function. */
    attr_unset (line[start], DELIMIT);

    for (i = start; i <= end; i++)
	{
	if (is_attrset (line [i], DELIMIT))
	    break;
	line [i] = de_attr (line [i]);
	attr_set (line [i], attr);
	}

    if (i > end) /* end of line is a delimiter */
	i = end;

    return (i);
}

/****************************************************************************/
/* Si_redisplay: force total repaint                                        */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     LI, CO, g_curscreen, g_curcrc                    */
/*                                                                          */
/* globals changed:        g_curscreen, g_curcrc                            */
/*                                                                          */
/* synopsis:                                                                */
/*     Si_redisplay forces a complete erase and display sequence.           */
/*     It first invokes St_init to reset the terminal, then marks           */
/*     the entire screen as dirty, and finally smears spaces all            */
/*     over the screen and sets all of the g_curcrc array to NULLs.         */
/****************************************************************************/

void Si_redisplay (void)
    {
    ATTR *curline;
    int i;
    int li = LI;

#ifdef  DEBUG
    debug ("Si_redisplay called\n");
#endif

    /* clear screen and set up terminal */
    St_init ();

    Si_dirty (0, li-1);
    for (i = 0; i < li; i++)
	{
	curline = (ATTR *) g_curscreen [i];
	i_smear ((short)(SPACE | FIRST), CO, (short *) curline);
	curline[CO] = '\0';
	g_curcrc [i] = 0;
	}
    }

/****************************************************************************/
/* term_enter: alternate form of initializing the display module            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           VOID                                             */
/*                                                                          */
/* globals referenced:                                                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     For those cases where one wishes to perform an operation similar     */
/*     to Si_init, but without clobbering g_goalscreen.  The routine makes  */
/*     the ioctl calls, calls St_init to clear the screen and home the      */
/*     cursor, and blanks out g_curscreen.                                  */
/****************************************************************************/

void term_enter (void)
    {
    ATTR *curline;
    int i;

    /* If stderr is a file use runmodes2 structure. */
    ioctl (g_St_fd, TCSETAW, & runmodes);
    if (isafile) 
        {
        ioctl (2, TCSETAW, & runmodes2);
        }
    else {
        ioctl (2, TCSETAW, & runmodes);
        }

    /* throw out the init seq, and clear the screen */
    St_init ();

    for (i = 0; i < LI; i++)
	{
	curline = (ATTR *) g_curscreen [i];
	i_smear ((short)(SPACE | FIRST), CO, (short *) curline);
	curline[CO] = '\0';

	/* force the crc to be different than g_goalscreen so the */
	/* refresh can happen */

	g_curcrc [i] = 0;
	}

    Si_dirty (0, LI-1);
    g_popflag = FALSE;
    }

/****************************************************************************/
/* term_exit: leave the terminal in its original, pre-editor state          */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           VOID                                             */
/*                                                                          */
/* globals referenced:                                                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     For those cases where one wishes to perform an operation similar     */
/*     to Si_quit, but without doing a screen flush.  The routine calls     */
/*     St_quit, fflush, and then a series of ioctl calls.                   */
/****************************************************************************/

void term_exit (void)
    {

    /* do any terminal dependent cleanup    */
    St_quit ();
    (void) fflush (stdout);
    g_popflag = FALSE;
    ioctl ( 1, TCSETAF, & oldmodes);
    if (isafile)
        {
        ioctl (2, TCSETAF, & oldmodes2);
        }
    else {
        ioctl (2, TCSETAF, & oldmodes);
        }
    }

/****************************************************************************/
/* winsize: determine size of terminal window, redisplay if changed.	    */
/*                                            				    */
/* arguments:  none                           				    */
/*                                            				    */
/* return value:  TRUE if redisplayed, FALSE if not			    */
/*                                            				    */
/* globals referenced:  CO, LI                				    */
/*                                            				    */
/* globals changed:  CO, LI                				    */
/*                                            				    */
/* synopsis:                                  				    */
/*     Calls termdef() to learn number of columns and rows in screen.  If   */
/*     termdef() returns 0 for either, uses old CO and LI values.  If new   */
/*     values are different from old, frees up memory associated with       */
/*     screen structures, and re-allocates it, and re-displays screen.      */
/*     Assumes that St_getcap() has already been called.                    */
/****************************************************************************/

int winsize(void)
{

  int  w, h, i;
  int  line, column;
  char olddir [PATH_MAX];	/* copy of g_curdir  */

  char *oldform;
  char *newfile;
  char *fieldname;

  /* Ask the kernel what it thinks the screen size is */
  w = atoi (termdef (fileno (stdin), 'c'));
  h = atoi (termdef (fileno (stdin), 'l'));

  /* if the results of termdef() are either untrustworthy, or the same
     as what we already think the screen size is, then don't do
     anything more. */

  if (((w <= 0) || (h <= 0)) ||
      ((w == CO) && (h == LI)))
      return (FALSE);

  /* Free up the old screen structures */
  if (g_goalscreen != NULL)
    {

    /* free up old line buffers */
    s_free ((char *)g_goalscreen);
    s_free ((char *)g_goalcrc);
    s_free ((char *)g_curscreen);
    s_free ((char *)g_curcrc);
    }

  LI = h;
  CO = w;

  if (w > 255)
    w = CO = 255;
	
  /* allocate and set up the 2 screen images */
  g_goalscreen = (ATTR **) s_alloc (h, T_POINTER, "goalscreen");
  g_goalcrc    = (int *)   s_alloc (h * sizeof (int), T_CHAR, "goalcrc");
  g_curscreen  = (ATTR **) s_alloc (h, T_POINTER, "curscreen");

  /* for echo-to-break processing */
  g_curcrc     = (int *)   s_alloc (h * sizeof (int), T_CHAR, "curcrc");
  for (i = 0; i < h; i++)
    {
    g_goalscreen [i] = (ATTR *) s_alloc (w + 2, T_ATTR, NULL);
    g_curscreen  [i] = (ATTR *) s_alloc (w + 2, T_ATTR, NULL);
    }

#ifdef	DEBUG
  debug ("winsize:  w_filename (g_wp) is \"%s\"", w_filename (g_wp));
#endif

  /* write the window contents out to the disk files */
  flush_window();

  /* save cursor position and current window name */
  line = Eline ();
  column = Ecolumn ();
  fieldname = Efieldname ();

  /* All the cached forms have the old size, so throw them away and
     force re-reads. */

  if (g_formcache != NULL)
    {
    s_free ((char *)g_formcache);
    g_formcache = NULL;
    }

  oldform = g_formname;

  /* say that the current form is called "*wrongsize*", so that this
     condition can be recognized elsewhere in the code */

  g_formname = s_string (WRONGSIZEFORM);

#ifdef	DEBUG
    debug ("winsize:  w_filename (g_wp) is \"%s\"", w_filename (g_wp));
#endif

    /* If the current INed window is associated with a file, then
       start a new window on that file, otherwise use the filename
       from the invariant text window ( g_warray[0] ) .... */

    if ( w_filename (g_wp) )
       newfile = w_filename (g_wp);
    else
       newfile = w_filename (g_warray);

    (void) strcpy (olddir, g_curdir); /* save current directory */
    if (setfile ( newfile, TRUE, TRUE) == ERROR)
	fatal ("winsize: setfile failure on \"%s\"", newfile);
    if (! seq (olddir, g_curdir))
        {
        (void) strcpy (g_curdir, olddir);

        if (chdir (olddir) == ERROR) /* change to the old directory */
            fatal ("winsize:  cannot change to directory '%s'", olddir);
        }

  s_free (oldform);

  /* restore cursor position */
  Eputcursor ( fieldname, line, column);

  /* force repaint of entire screen */
  g_dispflag = DISPINIT;
  Edisplay();
  return (TRUE);
}
