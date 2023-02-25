#if !defined(lint)
static char sccsid[] = "@(#)72	1.15  src/tenplus/e2/common/pop.c, tenplus, tenplus411, 9436A411a 9/3/94 12:28:46";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Eask, Econfirm, Eerror, Emessage, ask, forcebox,
 *		killbox, sizenewbox, newbox, doformat, waitbox
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* file:  pop.c                                                             */
/*                                                                          */
/* pop up box handling routines                                             */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include <stdarg.h>
#include <time.h>
#include "INeditor_msg.h"

#define BOXTIME 2

static long     g_poptime;       /* time newbox was called          */
static char     g_tmpbuf [2000 * MB_LEN_MAX]; /* sprintf buffer     */

LOCAL void waitbox (void);


static int height;   /* box height (w/ border)   */

extern ATTR g_partchar; /* part-character indicator */

static void drawbox(int , int , int , int ,
             int , int , int , ATTR **);

/****************************************************************************/
/* Eask:  asks question in a popup box and returns answer.                  */
/*                                                                          */
/* arguments:  (variable)  int   msg_no - extended help message number    */
/*                         char *format   - printf-style format for         */
/*                                          question                        */
/*                         char *arg1, *arg2, *arg3, *arg4, *arg5 -         */
/*                                          possible extra args to format   */
/*                                                                          */
/* return value:           char * - structured string with answer to        */
/*                                  question                                */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/* Returns s_alloc'ed char array.  Termininating key in global g_keyvalue   */
/* String is processed through the Efixvars routine.                        */
/* Returns ERROR if user hit the CANCEL key.   Calls the ask procedure      */
/* with enter-key flag set false.                                           */
/****************************************************************************/
char *Eask (int msg_no, char *format1, ...)
    {
	char *arg1, *arg2, *arg3, *arg4, *arg5;

va_list ap;
va_start(ap, format1);
arg1 = va_arg(ap, char *);
arg2 = va_arg(ap, char *);
arg3 = va_arg(ap, char *);
arg4 = va_arg(ap, char *);
arg5 = va_arg(ap, char *);
va_end(ap);

#ifdef DEBUG
    debug ("Eask:  msg_no = %d, message = '%s'", msg_no, g_tmpbuf);
#endif
    return (ask (FALSE, msg_no, format1, arg1, arg2, arg3, arg4, arg5));
    }

/****************************************************************************/
/* Econfirm:  asks a YES/NO question                                        */
/* Returns YES if user touches EXECUTE, no if CANCEL                        */
/*                                                                          */
/* arguments:  (variable)  int   msg_no - extended help message number    */
/*                         char *format   - printf-style format for         */
/*                                          question                        */
/*                         char *arg1, *arg2, *arg3, *arg4, *arg5 -         */
/*                                          possible extra args to format   */
/*                                                                          */
/* return value:           int - YES or NO                                  */
/*                                                                          */
/* globals referenced:     g_keyvalue                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Uses Emessage to display the question, and waits for one character   */
/*     of input.  If user hit HELP, display the help message in a box.      */
/*     Otherwise return YES if user hit EXECUTE, NO if he hit CANCEL.       */
/****************************************************************************/

int Econfirm (int msg_no, char *format2, ...)
    {
	char *arg1, *arg2, *arg3, *arg4, *arg5;

va_list ap;
va_start(ap, format2);
arg1 = va_arg(ap, char *);
arg2 = va_arg(ap, char *);
arg3 = va_arg(ap, char *);
arg4 = va_arg(ap, char *);
arg5 = va_arg(ap, char *);
va_end(ap);

#ifdef DEBUG
    debug ("Econfirm:  msg_no = %d, message = '%s'", msg_no, format2);
#endif

    for (;;)
	{
	Emessage (msg_no, format2, arg1, arg2, arg3, arg4, arg5);
	onechar (TRUE); /* wait for user to type character */

	if (g_keyvalue == HELP)
	    helpbox (msg_no+1, g_help_count);
	else
	    break;
	}
    killbox (FALSE);
    return (g_keyvalue == EXECUTE);
    }

/****************************************************************************/
/* Eerror:  puts message in pop up box, waits for response                  */
/*                                                                          */
/* arguments:  (variable)  int   msg_no - extended help message number    */
/*                         char *format   - printf-style format for message */
/*                         char *arg1, *arg2, *arg3, *arg4, *arg5 -         */
/*                                          possible extra args to format   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Uses Emessage to display the error message and waits for HELP or     */
/*     CANCEL.  If HELP, provide extended help message in a box; otherwise  */
/*     return.                                                              */
/****************************************************************************/

void Eerror (int msg_no, char *format3, ...)
{

	char *arg1, *arg2, *arg3, *arg4, *arg5;

va_list ap;
va_start(ap, format3);
arg1 = va_arg(ap, char *);
arg2 = va_arg(ap, char *);
arg3 = va_arg(ap, char *);
arg4 = va_arg(ap, char *);
arg5 = va_arg(ap, char *);
va_end(ap);

#ifdef DEBUG
    debug ("Eerror:  msg_no = %d, message = '%s'", msg_no, format3);
#endif

    Emessage (msg_no, format3, arg1, arg2, arg3, arg4, arg5);
    Ebell ();
    onechar (FALSE); /* wait for user to type character */

    if (g_keyvalue == HELP)
	helpbox (msg_no+1, g_help_count);

    killbox (FALSE);

    va_end(ap);
    }

/****************************************************************************/
/* Emessage:  displays a message in a pop up box                            */
/*                                                                          */
/* arguments:  (variable)  int   msg_no - extended help message number    */
/*                         char *format   - printf-style format for message */
/*                         char *arg1, *arg2, *arg3, *arg4, *arg5 -         */
/*                                          possible extra args to format   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_ttyflag, g_tmpbuf                              */
/*                                                                          */
/* globals changed:        g_tmpbuf                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     If we are in the middle of trying to restart, put the message on     */
/*     standard error output.  Otherwise display it in a popup box.         */
/****************************************************************************/

void Emessage (int msg_no, char *format4, ...)
    {
    POINTER *list;      /* list to send to newbox           */

    /* help library has fixed format before sending it across pipe */
    char *arg1, *arg2, *arg3, *arg4, *arg5;

va_list ap;
va_start(ap, format4);
arg1 = va_arg(ap, char *);
arg2 = va_arg(ap, char *);
arg3 = va_arg(ap, char *);
arg4 = va_arg(ap, char *);
arg5 = va_arg(ap, char *);
va_end(ap);


    if ((msg_no >= 0) && (msg_no < HELPER_MSG_OFFSET))
        format4 = Egetmessage(msg_no, format4, FALSE);

    (void) sprintf (g_tmpbuf, format4, arg1, arg2, arg3, arg4, arg5);

#ifdef  DEBUG
    debug ("Emessage:  g_tmpbuf = '%s'", g_tmpbuf);
#endif


    if (! g_ttyflag) /* if during restart */
	{
	(void) fprintf (stderr, "%s\n", g_tmpbuf);
	return;
	}

    list = s2pointer (g_tmpbuf);
    newbox (list);
    s_free ((char *) list);
    }

/****************************************************************************/
/* ask:  asks question in a popup box, returns answer.                      */
/*                                                                          */
/* arguments:  (variable)  int   flag     - whether in ENTER box            */
/*                         int   msg_no - extended help message number    */
/*                         char *format   - printf-style format for         */
/*                                          question                        */
/*                         char *arg1, *arg2, *arg3, *arg4, *arg5 -         */
/*                                          possible extra args to format   */
/*                                                                          */
/* return value:           char * - the user-provided answer to the         */
/*                                  question                                */
/*                                                                          */
/* globals referenced:     g_tmpbuf, g_keyvalue                             */
/*                                                                          */
/* globals changed:        g_tmpbuf                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     Returns s_alloc'ed char array.  Termininating key in global          */
/*     g_keyvalue.  Answer string is processed through the Efixvars routine */
/*     Returns ERROR if user hit the CANCEL key.  Uses inbox to gather in   */
/*     the characters of the answer.  Handles HELP key to provide extended  */
/*     help.                                                                */
/****************************************************************************/

char *ask (int flag, int msg_no, ...)
    {
    POINTER *message; /* message as a pointer array       */

    int numspaces;    /* number of trailing spaces        */
    char *answer;     /* text that the user has typed in  */
    char *cp;         /* used to count trailing spaces    */
    char *retval;     /* Saved so a s_free can be done    */

    /* help library has fixed format before sending it across pipe */
    char *arg1, *arg2, *arg3, *arg4, *arg5, *format5;

va_list ap;
va_start(ap, msg_no);
format5 = va_arg(ap, char *);
arg1 = va_arg(ap, char *);
arg2 = va_arg(ap, char *);
arg3 = va_arg(ap, char *);
arg4 = va_arg(ap, char *);
arg5 = va_arg(ap, char *);
va_end(ap);

    if ((msg_no >= 0) && (msg_no < HELPER_MSG_OFFSET))
        format5 = Egetmessage(msg_no,format5,FALSE);

    (void) sprintf (g_tmpbuf, format5, arg1, arg2, arg3, arg4, arg5);

#ifdef DEBUG
    debug ("ask:  flag = %d, msg_no = %d, message = '%s'",
	   flag, msg_no, g_tmpbuf);
#endif


    for (;;)
	{
	message = s2pointer (g_tmpbuf);
	    numspaces = newbox (message);

	s_free ((char *) message);

	/* The user is given (numspaces - 1) chars to enter text */
	if (numspaces <= 1) /* if no room for answer */
	    {
	    onechar (TRUE); /* get next char typed */

	    if (g_keyvalue == HELP)
		{
		helpbox (msg_no+1, g_help_count);
		continue;
		}
	    killbox (FALSE);

	    if (g_keyvalue == CANCEL)
		return ((char *) ERROR);

	    return (s_alloc (0, T_CHAR, NULL));
	    }

	    answer = inbox (g_popbottom - 1, g_popbottom - 1,
			 g_popright - numspaces + 1, g_popright - 1, flag);

/* inbox will return NULL if a quote error occurs  */
/* reseting g_tmpbuf will cause inbox to be called */
/* the next time through the loop                  */

	if (answer == NULL)
	    {
	    char *tmp;
	    tmp = s_string (g_tmpbuf);

	    Eerror (M_EQUOTE, "Specify an alphabetic character with the QUOTE command.");
	    (void) strcpy (g_tmpbuf, tmp);
	    continue;
	    }

	if (g_keyvalue == CANCEL)
	    {
	    s_free (answer);
	    killbox (FALSE);
	    return ((char *) ERROR);
	    }

	if (g_keyvalue == HELP)
	    {
	    helpbox (msg_no+1, g_help_count);
	    s_free (answer);
	    }
	else
	    break;
	}

    killbox (FALSE);
    retval = Efixvars (answer);
    s_free (answer);

    return (retval);
    }

/****************************************************************************/
/* forcebox: make display redisplay the dirty-box                           */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     g_poptop, g_popbottom, g_goalcrc, g_uniqnum      */
/*                                                                          */
/* globals changed:        g_goalcrc, g_uniqnum                             */
/*                                                                          */
/* synopsis:                                                                */
/*     each element of g_goalcrc from g_poptop to g_popbottom is assigned   */
/*     the next g_uniqnum value.  The uniqnum global is incremented at each */
/*     step.                                                                */
/****************************************************************************/

void forcebox (void)
    {
    int i;

    extern int *g_goalcrc;
    extern int g_uniqnum;

    for (i = g_poptop; i <= g_popbottom; i++)
	g_goalcrc [i] = ++g_uniqnum;
    }

/****************************************************************************/
/* killbox: remove the popup box                                            */
/* Waits BOXTIME seconds if waitflag is TRUE                                */
/*                                                                          */
/* arguments:              int waitflag - whether to wait before killing    */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_popflag, g_goalscreen, g_poptop, g_popleft     */
/*                         height, width                                    */
/*                                                                          */
/* globals changed:        g_popflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     If the g_popflag global is not set, no effect.  Otherwise, if the    */
/*     waitflag is given, invoke waitbox.  Finally, set g_popflag false     */
/*     and repaint the part of the screen under the box, either via a call  */
/*     to Edisplay, or by calling sendbox for each affected window.         */
/****************************************************************************/

void killbox (int waitflag)
    {
    int i;
    int top, bottom, left1, right1;
    int j, line;

    ATTR spacep[2];
    WSTRUCT *Owp;
    WSTRUCT *wp;

#ifdef DEBUG
    debug ("killbox:  waitflag = %s, g_popflag = %d",
	   waitflag ? "TRUE" : "FALSE", g_popflag);
#endif

    if (g_popflag == 0)
	return;

    if (waitflag)
	waitbox ();

    Owp = g_wp;
    g_popflag--;

    /* restore old text to tv array */
    height = obj_count (g_warray);
    for (i = 0; i < height; i++)
	{
	wp = g_wp = g_warray+i;
	top = g_poptop - w_ttline (wp);
	bottom = g_popbottom - w_ttline (wp);

	left1 = g_popleft - w_ttcol (wp);
	right1 = g_popright - w_ttcol (wp);

	if (height > 1)
	    {
	    if (top < 0)
		top = 0;
	    if (bottom > w_lrline (wp))
		bottom = w_lrline (wp);
	    if (left1 < 0)
		left1 = 0;
	    if (right1 > w_lrcol (wp))
		right1 = w_lrcol (wp);
	    }
	if (w_cache (wp))
	    {
	    for (line = top; line <= bottom; line++)
		sendbox (line, line, 0, w_lrcol(wp));
	    }
	else
	    {
            /* it is assumed that SPACE takes 1 byte to store and
               takes one column on the screen */
	    spacep[0] = SPACE | FIRST;
	    spacep[1] = '\0';
	    for (line=top+w_ttline (wp); line<= bottom+w_ttline (wp); line++)
		for (j = left1+w_ttcol (wp); j <= right1+w_ttcol (wp); j++)
		    Si_ovector (spacep, 1, line, j);
	    }
	}

    g_wp = Owp;

    }

/****************************************************************************/
/* sizenewbox: determine size of box required to display the given list.    */
/*                                                                          */
/* arguments:              count - number lines in box                      */
/*                         maxwidth - maximum width of box                  */
/*                         pop_tvline, pop_tvcol - x and y cursor positions */
/*                         int *left, *right, *top, *bottom - pointers to   */
/*                           locations to store box corner coordinates      */
/*                                                                          */
/* return value:           returns whether the box is offset left/right     */
/*                         from the cursor                                  */
/*                                                                          */
/* globals referenced:     g_wp, LI, CO                                     */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     determine where to place to box on the screen to keep it next to     */
/*     the cursor.                                                          */
/*                                                                          */
/*         The placement algorithm is:                                      */
/*                                                                          */
/*         Try to put the box below and to the right of the cursor.         */
/*                                                                          */
/*         If it is too tall to fit below the cursor, put the bottom of     */
/*         the box on the bottom of the screen, and let the top be wherever */
/*         it needs to be from there.                                       */
/*                                                                          */
/*         If it is too wide to fit to the right of the cursor, put the     */
/*         right side of the box on the right side of the screen, and the   */
/*         left side wherever it needs to be.  If the box collided with the */
/*         bottom of the screen, allow an extra column width for the box to */
/*         try to keep the cursor outside the box to the left.              */
/*                                                                          */
/*     Note that if the cursor is sufficiently close to the lower right     */
/*     corner, it will be buried under the box.                             */
/****************************************************************************/

sizenewbox (int count,int maxwidth,int pop_tvline,int pop_tvcol,
		      int *left2, int *right2,
		      int *top, int *bottom)
    {
    int offset;
    int height1, width1;
    int lines, cols;

    lines = (w_tbline(g_warray) ? min ( LI, w_tbline(g_warray) + 1 ) : LI );
    cols = (w_tbcol(g_warray) ? min ( CO, w_tbcol(g_warray) + 1 ) : CO );

    height1 = min (lines - 3, count) + 2;  /* get box height and width */
    width1 = min (cols - 2, maxwidth) + 2;

    offset = 0; /* this is zero if box fits above or below cursor, else 1 */

#ifdef DEBUG
    debug ("sizenewbox: count = %d, maxwidth = %d, height = %d, width = %d",
	count, maxwidth, height1, width1);
    debug ("        tvline = %d, tvcol = %d", pop_tvline, pop_tvcol);
#endif

    /* fix top and bottom of box */
    if (height1 < ((lines - 2) - pop_tvline)) /* if box will fit below cursor */
	{
	*top = pop_tvline + 1;
	*bottom = *top + height1 - 1;
	}
    else /* if box too tall put at bottom of screen */
	{
	*bottom = lines - 2;
	*top = *bottom - height1 + 1;
	offset = 1; /* have box be one column off the cursor */
	}

    /* fix left and right of box */
    if (width1 < cols - pop_tvcol - offset) /* if box fits to right of cursor */
	{
	*left2 = pop_tvcol + offset;
	*right2 = *left2 + width1 - 1;
	}
    else /* otherwise put right edge on right edge of tv screen */
	{
	*right2 = cols - 1;
	*left2 = *right2 - width1 + 1;
	}
#ifdef DEBUG
    debug ("sizenewbox:  top = %d, bot = %d, left = %d, right = %d",
	*top, *bottom, *left2, *right2);
#endif
	return (offset);
	}

/****************************************************************************/
/* newbox: put up pop up box with given lines in it.                        */
/*                                                                          */
/* arguments:              POINTER *list - lines to display in box          */
/*                                                                          */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     g_wp, g_poptop, g_popbottom, g_popright,         */
/*                         g_popleft, g_goalscreen                          */
/*                                                                          */
/* globals changed:        g_popflag, g_poptime                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If there is already a box on the screen, invoke waitbox to make sure */
/*     user has time to read it.  (In this version, we need to make         */
/*     an explicit call to killbox(YES) to remove the previous box.)  Then  */
/*     set the g_poptime global so we know when we started putting up this  */
/*     box.  Measure the box and figure out where to put it on the screen.  */
/*                                                                          */
/*     Next, make a copy of the screen contents under the box position and  */
/*     write the box into the screen array, including a border of graphics  */
/*     characters and an arrow pointing toward the cursor (if not hidden).  */
/*     Actually put up the box via a call to Edisplay or Si_display.        */
/*                                                                          */
/*     Set the g_popflag global TRUE                                        */
/*     so the rest of the world knows we have a box on screen.              */
/****************************************************************************/

int newbox (POINTER *list)
    {
    WSTRUCT *wp = g_wp;
    ATTR *tvpos;           /* position in tv array     */
    ATTR **newlist;
    ATTR *line;
    ATTR *acp;

    int linesize;          /* size of unpacked line    */
    int i;                 /* loop counter             */
    int offset;
    int count, maxwidth;
    int blanks, len;
    int pop_tvline, pop_tvcol;

    /* variables indicating the amount the dirty region needs to be
       expanded to the left or right due to overwriting part of a multibyte
       character by the border of the box. */

#ifdef DEBUG
    debug ("newbox:  g_popflag = %d", g_popflag);
#endif

#ifdef CAREFUL
    (void) s_typecheck (list, "newbox", T_POINTER);
#endif

    killbox (TRUE);  /* get rid of previous box  */
    (void) time (&g_poptime);
    newlist = doformat ((ATTR **)list, CO - 2);

    maxwidth = 0;
    count = obj_count (newlist);
    for (i = 0; i < count; i++)
	{
	len = attr_text_width (newlist[i]); /* last value of len used later */
	if (maxwidth < len)
	    maxwidth = len;
	}
    line = newlist[count - 1];
    blanks = maxwidth - len; /* number of blanks added by formating */
    for (acp = line + len - 1; acp >= line && isattrspace(acp); 
         backupattr(&acp))
	blanks++;

    /***** figure out where box should go on screen *****/
    pop_tvline = w_ttline (wp) + w_line (wp); /* calculate cursor tv coords */
    pop_tvcol = w_ttcol (wp) + w_col (wp);

    /* figure out dimensions of box: */
    offset = sizenewbox (count, maxwidth, pop_tvline, pop_tvcol,
      &g_popleft, &g_popright, &g_poptop, &g_popbottom);

    /* draw the box. The insides of the new box are filled from newlist */
    drawbox(g_poptop, g_popbottom, g_popleft, g_popright, 
            pop_tvline, pop_tvcol, offset, newlist);

    Si_dirty (g_poptop, g_popbottom);

    s_free ((char *)newlist);

    /* force display to believe the lines are dirty     */
    forcebox ();
    g_popflag++;
    Edisplay ();                 /* have display put up the box */
    return (blanks);
    }

/****************************************************************************/
/* doformat: format lines in list into new structured allocation            */
/*    length is maximum width in columns of new lines.                      */
/****************************************************************************/
ATTR **doformat (ATTR **list, int length)
{
    int i, count, len, newlines = 0;
    ATTR **newlist;
    ATTR *line;

    /* guarantee that we can do something, even if its not quite right,
       in very small windows */
    if (length < 3)
       length = 3;

    count = obj_count (list);
    newlist = (ATTR **)s_alloc (count, T_POINTER, NULL);
    for (i = 0; i < count; i++)
	{
	newlist [i+newlines] = (ATTR *)unpackup ((char *)list [i], 0);
	len = attr_text_width(newlist[i+newlines]);
	while (len > length)
	    {
	    int newlen, newpos, newpos_buf, newlen_buf;

	    line = newlist[i+newlines];
	    newlines++;
	    newlist = (ATTR **)s_realloc((char *)newlist, count+newlines);

	    newpos = calc_line_position2(line, length, &newlen); 
	    backupattr_idx(line, &newpos); 
            newlen -= char_width(line + newpos);
	    newpos_buf = newpos;
            newlen_buf = newlen;
	    while (backupattr_idx(line, &newpos), 
                   newlen -= char_width(line + newpos),
                   !isattrspace(&line[newpos]) )
		if (newlen < length / 3)
		    {
		    newpos = newpos_buf;
                    newlen = newlen_buf;
		    break;
		    }
            newlen += char_width(line + newpos);
            skipattr_idx(line, &newpos);

	    len -= newlen;
	    newlist[i+newlines] = (ATTR *) i_string ((short *)line+newpos);
	    newlist[i+newlines-1] = (ATTR *) i_realloc ((short *)newlist[i+newlines-1],newpos);
	    }
	}
    return (newlist);
}

/****************************************************************************/
/* waitbox:  makes sure last box was up for BOXTIME seconds                 */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     g_poptime                                        */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the g_poptime global is zero, do nothing.  Otherwise see how      */
/*     long ago g_poptime was.  If it was less than BOXTIME seconds ago,    */
/*     go to sleep for the remaining time.                                  */
/****************************************************************************/

LOCAL void waitbox (void)
    {
    int timediff;          /* time last box was up     */
    long curtime;

    if (g_poptime == 0L)
	return;

    (void) time (&curtime); /* see if box has been up long enough */
    timediff = curtime - g_poptime;

#ifdef DEBUG
    debug ("waitbox:  curtime = %ld, g_poptime = %ld, timediff = %d",
	   curtime, g_poptime, timediff);
#endif

    if (timediff < BOXTIME)
	(void) sleep (((unsigned) (BOXTIME - timediff)));
    }


/***********************************************************

DRAWBOX

Draws a box on the screen by drawing in g_goalscreen. The box
within the outline is filled with spaces. If appropriate, an
arrow is painted in the outline of the box to indicate where the
cursor is.

arguments: topline: line coordinate (on screen) of top of box
           bottomline: line coordinate (on screen) of bottom of box
           leftcol:    column coordinate (on screen) of left side of box
           rightcol:   column coordinate (on screen) of right side of box
              
           cursline:   line coordinate (on screen) of cursor
           curscol:    column coordinate (on screen) of cursor

           offset:     determines whether the box is offset left/right
                       from the cursor. This is the offset returned by
                       function sizenewbox.

           textarray:  array of structured ATTR text lines that have
                       to go in the box.

return value: none

globals referenced: g_goalscreen

globals changed:    g_goalscreen

NOTE: It is assumed that the characters used to draw the box
      (ie. SPACE, G_ULC, G_LLC, G_HLINE, G_VLINE, G_URC, G_LRC,
      DAR, LAR, RAR and UAR) are single ATTR characters and that
      they are one column wide.

***********************************************************/

static void drawbox(int topline, int bottomline, int leftcol, int rightcol,
             int cursline, int curscol, int offset,
             ATTR **textarray)
{
  int boxline, textline;
  ATTR begin_char, end_char;
  ATTR *leftp;

#ifdef DEBUG
    debug ("drawbox: topline = %d, bottomline = %d, leftcol = %d",
                     topline, bottomline, leftcol);
    debug ("         rightcol = %d, cursline = %d, curscol = %d",
                     rightcol, cursline, curscol);
    debug ("         offset = %d", offset); 
#endif

  /* draw top line of the box */
  drawline((ATTR)(G_ULC | FIRST), (ATTR)(G_URC | FIRST), (ATTR)(G_HLINE | FIRST),
           topline, leftcol, rightcol, &leftp, NULL, g_goalscreen);

  /* if the cursor sits on the top line of the box, put an
     up arrow (UAR) on the top line, at the column where the 
     cursor is located.
  */
  if ((topline == cursline + 1) || (cursline == topline)) 
      /* curscol + offset - leftcol is the number of columns that
         the cursor is to the right of the left side of the box.
         Because it is assumed that the characters that make up the
         box are all single ATTR, single width, this equals the 
         number of bytes from the left side of the box (pointed at
         by leftp).
      */

      if (topline == cursline +1)
          *(leftp + curscol - leftcol) = UAR;
      else
          if (curscol == leftcol - 1 )
              *(leftp + curscol - leftcol + 1) = LAR;

  /* fill in the box between the top and bottom line */
  for (boxline = topline + 1, textline = 0; 
       boxline < bottomline;
       boxline++, textline++) {
      end_char = G_VLINE | FIRST;
      begin_char = G_VLINE | FIRST;
      if (boxline == cursline) {
           /* cursor sits on the current box line. See if it sits
              on the left or the right side of the box. If so,
              use a left arrow (LAR) or right arrow (RAR) instead
              of a vertical line (G_VLINE)
           */
           if (leftcol == curscol + 1)
                begin_char = LAR | FIRST;
           else {
                if (rightcol == curscol - 1)
                   end_char = RAR | FIRST;
           }
      }
      drawline(begin_char, end_char, (ATTR)(SPACE | FIRST), boxline,
               leftcol, rightcol, &leftp, textarray[textline],
               g_goalscreen);
  }


  /* draw bottom line of the box */
  drawline((ATTR)(G_LLC | FIRST), (ATTR)(G_LRC | FIRST), (ATTR)(G_HLINE | FIRST),
           bottomline, leftcol, rightcol, &leftp, NULL,
           g_goalscreen);

  /* if the cursor sits on the bottom line of the box, put a
     down arrow (DAR) on the bottom line, at the column where the 
     cursor is located.
  */
  if (bottomline == cursline - 1) 
      /* curscol + offset - leftcol is the number of columns that
         the cursor is to the right of the left side of the box.
         Because it is assumed that the characters that make up the
         box are all single ATTR, single width, this equals the 
         number of bytes from the left side of the box (poined at
         by leftp).
      */
      *(leftp + curscol + offset - leftcol) = DAR;

}


/***********************************************************

DRAWLINE

Draws a horizontal line on the screen by drawing in scr_array.  

arguments: begin_char: first character of the line
           last_char:  last character of the line
           middle_char: the characters used between the first and
                        the last character. If text is not NULL,
                        this char is used to pad out the text if the
                        text does not fill the whole box. If text is
                        NULL, all characters between the begin_char
                        and the last_char are middle_chars.
           line:        line coordinate (on screen) of line
           startcol:    column coordinate (on screen) of beginning of
                        line.
           endcol:      column coordinate (on screen) of end of line
           text:        text that has to go into the line. 
                        Text is NULL if no text is to
                        be included in the line.
           scr_array:   two dimensional ATTR array in which the line is
                        to be written.

return value: *leftp: pointer into scr_array to the ATTR that is
                      the left most ATTR of the box.

NOTE: it is assumed that begin_char, middle_char and last_char are
      all single ATTR characters with display width 1.

***********************************************************/

void drawline(ATTR begin_char, ATTR last_char, ATTR middle_char,
                int line, int startcol, int endcol, ATTR **leftp,
                ATTR *text1, ATTR **scr_array)
{
  /* width in columns of the line to be drawn */
  int linewidth = endcol - startcol + 1;

#ifdef DEBUG
    debug ("drawline: line = %d, startcol = %d, endcol = %d",
                      line, startcol, endcol);
#endif

  if (__max_disp_width == 1 && !MultibyteCodeSet) {
      /* all characters have display width 1 and take up one ATTR */
      scr_array[line][startcol] = begin_char;
      scr_array[line][endcol] = last_char;
      *leftp = &scr_array[line][startcol];
      if (text1 == NULL)
          i_smear(middle_char, linewidth - 2, (short *)*leftp + 1);
      else {
          int textlen = i_strlen((short *)text1);
      
          (void) i_strncpy((short *)*leftp + 1, (short *)text1,
                           min(linewidth - 2, textlen));
          if (textlen < linewidth - 2)
              i_smear((short) middle_char, linewidth - 2 - textlen,
                      (short *)*leftp + textlen + 1); 
      }
  }
  else {
      /* width of the scr_array line up to the character 
         that will be overwritten by the begin_char */
      int begin_width;

      /* index in the scr_array line of character that 
         will be overwritten by begin_char */
      int begin_victim_idx = 
              calc_line_position2(scr_array[line], startcol,
                                  &begin_width);

      /* width of the scr_array line from the (beginning of) character
         overwritten by the begin_char up to the character 
         that will be overwritten by the last_char */
      int last_width;

      /* index in the scr_array line of character that 
         will be overwritten by last_char */
      int last_victim_idx = begin_victim_idx +
              calc_line_position2(&scr_array[line][begin_victim_idx],
                         endcol - begin_width, &last_width);

      /* width of all the characters that will be overwritten by
         the line to be drawn. (this width is equal or larger
         than the width of the line itself, depending on the 
         width of the characters that are overwritten by the
         begin_char and last_char. */
      int victim_width = last_width + 
            char_width(&scr_array[line][last_victim_idx]);

      /* number of ATTRs taken up by all characters that will be
         overwritten by the line to be drawn */
      int nbr_victim_attrs = last_victim_idx - begin_victim_idx +
             attrlen(&scr_array[line][last_victim_idx]);

      /* number of display columns that are taken by the part of 
         the begin_victim_idx character that is to the left of startcol. */
      int front_begin_victim = startcol - begin_width;

      /* pointer to rightmost character of box in scr_array */
      ATTR *rightp;

      /* pointer to leftmost position in box (pointing into
         scr_array) from which middle characters must be 
         written. */
      ATTR *fillp;

      /* if text is not NULL, width_linetext is the width of that
         part of text that will go into the line. */
      int width_linetext;

      /* if text is not NULL, endtextidx is the index of the first
         ATTR after the last ATTR in text that will go in the line */
      int endtextidx;

      /* if text is not NULL, nbr_newattrs is the number of ATTRs 
         taken up by characters forming the line */
      int nbr_newattrs;

#ifdef DEBUG
      debug ("drawline before adjust: victim_width = %d, " 
             "linewidth = %d, front_begin_victim = %d, "
             "nbr_victim_attrs = %d", 
             victim_width, linewidth,
             front_begin_victim, nbr_victim_attrs);
      debug ("            begin_victim_idx = %d, last_victim_idx = %d, "
             "last_width = %d, begin_width = %d",
             begin_victim_idx, last_victim_idx,
             last_width, begin_width);
#endif

      /* adjust the space in scr_array taken by the characters 
         that will be overwritten by the line, to accomodate the 
         characters that make up the line.
      */
 
      if (text1 == NULL) {
          /* because all characters that will be overwritten by the line
             will be replaced by characters that are single-width,
             single-ATTR, make sure that there are space_width ATTRs
             available for the line from begin_victim_idx onwards. */
          if (victim_width > nbr_victim_attrs)
              scr_array[line] = (ATTR *)i_insert((short *)scr_array[line],
                     begin_victim_idx, victim_width - nbr_victim_attrs); 
          else if (victim_width < nbr_victim_attrs)
                   scr_array[line] = (ATTR *)i_delete((short *)scr_array[line],
                         begin_victim_idx, nbr_victim_attrs - victim_width); 
      }
      else {
          endtextidx = calc_line_position2(text1, linewidth - 2,
                                           &width_linetext);
          nbr_newattrs = endtextidx + (victim_width - width_linetext);

          if (nbr_newattrs > nbr_victim_attrs)
              scr_array[line] = (ATTR *)i_insert((short *)scr_array[line],
                     begin_victim_idx, nbr_newattrs - nbr_victim_attrs); 
          else if (nbr_newattrs < nbr_victim_attrs)
                   scr_array[line] = (ATTR *)i_delete((short *)scr_array[line],
                         begin_victim_idx, nbr_victim_attrs - nbr_newattrs); 
      }
 
      /* overwrite the part of the begin_victim_idx character that
         is to the left of the startcol with spaces */
      i_smear((short)(SPACE | FIRST), front_begin_victim, 
              (short *)&scr_array[line][begin_victim_idx]);

      /* find the address of the ATTR where begin_char is to go */
      *leftp = &scr_array[line][begin_victim_idx +
                                   front_begin_victim];

      if (text1 == NULL) {
          rightp = *leftp + linewidth - 1;
          fillp = *leftp + 1;
      }
      else {
          (void) i_strncpy((short *) *leftp + 1, (short *)text1, endtextidx);
          fillp = *leftp + endtextidx + 1;
          rightp = fillp + (linewidth - 2) - width_linetext;
      }

      /* draw the line from *leftp */
      **leftp = begin_char;
      *rightp = last_char;
      i_smear(middle_char, rightp - fillp, (short *)fillp);
       
      /* overwrite the part of the last_victim_idx character
         that comes after the last_char with spaces. */
      i_smear((short)(SPACE | FIRST), 
           victim_width - linewidth - front_begin_victim,
           (short *)rightp + 1);

#ifdef DEBUG
      debug ("drawline ends: rightp - fillp = %d, victim_width = %d, " 
             "linewidth = %d, front_begin_victim = %d, "
             "width_line_text = %d, nbr_victim_attrs = %d,\n     ", 
             "begin_victim_idx = %d, endtextidx = %d, last_victim_idx = %d, "
             "last_width = %d, begin_width = %d",
             rightp - fillp, victim_width, linewidth,
             front_begin_victim, width_linetext, nbr_victim_attrs,
             begin_victim_idx, endtextidx, last_victim_idx,
             last_width, begin_width);
#endif

  }
}


