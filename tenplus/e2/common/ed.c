#if !defined(lint)
static char sccsid[] = "@(#)51	1.8  src/tenplus/e2/common/ed.c, tenplus, tenplus411, GOLD410 3/24/93 09:25:28";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Eclrhook, Efatal, Ekillhelper, Esethook, action,
 *		backspace, callfunction, clrhooks, enter 
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
/* file:  ed.c                                                              */
/*                                                                          */
/* command dispatcher for the e2 editor                                     */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

#ifdef TIMEPRINT
#include <sys/times.h>
#endif

char *g_restart;

#define SYNCDISK 1000      /* keystroke count for sync'ing disk */

/****************************************************************************/
/* Eclrhook:  clears the control hooks for a set of keys                    */
/*                                                                          */
/* arguments:              char *before - keys to clear Hbefore hooks       */
/*                         char *after  - keys to clear Hafter  hooks       */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_hooks                                          */
/*                                                                          */
/* globals changed:        g_hooks                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     Each character in the arguments to Eclrhook corresponds to a         */
/*     particular key in g_hooks.  For each character in the before         */
/*     string, ~BEFORE is anded in to the hooks for that key.               */
/*     Similarly, ~AFTER is anded into the hooks for each key in the        */
/*     after array.                                                         */
/****************************************************************************/

void Eclrhook (char *before, char *after)
    {

    char *cp; /* string pointer */

#ifdef DEBUG
    debug ("Eclrhook called");
#endif

    for (cp = before; *cp; cp++)
	{
	g_hooks [*cp] &= ~BEFORE;
	}
    for (cp = after; *cp; cp++)
	{
	g_hooks [*cp] &= ~AFTER;
	}
    }

/****************************************************************************/
/* Efatal:  handles fatal errors from the helper                            */
/*                                                                          */
/* arguments:              int helpcode - code for elaboration of message   */
/*                         char *message - fatal error message              */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_syncname, g_debugfp, g_restart,                */
/*                         g_curdir, g_filename                             */
/*                                                                          */
/* globals changed:        g_restart                                        */
/*                                                                          */
/* synopsis:                                                                */
/*    Efatal first ensures the file is open if it has been sync'ed.         */
/*    It then invokes Eerror to put the input message on the screen,        */
/*    with "Helper error:  " prepended.  Finally it closes the current      */
/*    helper and attempts to restart it.  Care is taken to prevent          */
/*    an infinite recursion in case of a restart failure.                   */
/****************************************************************************/

void Efatal (int helpcode, char *message)
    {

#ifdef DEBUG
    debug ("Efatal:  helpcode = %d, message = '%s'", helpcode, message);
#endif

    /* if file has been sync'ed reopen it */
    if (g_syncname)
	Ereopen ();

    /* still in previous Efatal - prevent recursion */
    if (g_restart)
	{
	Eerror (M_EHRESTART, "There was a helper error during restart.","");
	closehelper ();
	return;
	}

    Eerror (helpcode, "Helper error: %s", message);
    logfatal (message, TRUE);

    g_restart = s_string (g_helper);
    closehelper ();

    /* don't restart if in trace mode */
    if (g_debugfp == NULL)
	Emessage (M_ERESTART_TP, "Starting helper program %s again.", g_restart);
    }

/****************************************************************************/
/* Ekillhelper:  routine to return to previous helper environment           */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_killhelper                                     */
/*                                                                          */
/* globals changed:        g_killhelper                                     */
/*                                                                          */
/* synopsis:                                                                */
/*     Ekillhelper simply sets the global variable g_killhelper to TRUE.    */
/*     Its raison d'etre is to provide a user-accessible way to kill the    */
/*     current helper.                                                      */
/****************************************************************************/

void Ekillhelper (void)
    {

#ifdef DEBUG
    debug ("Ekillhelper called");
#endif

    g_killhelper = TRUE;
    }

/****************************************************************************/
/* Esethook:  sets the control hooks for a set of keys                      */
/*                                                                          */
/* arguments:          char *before - set of keys to invoke Hbefore         */
/*                     char *after  - set of keys to invoke Hafter          */
/*                                                                          */
/* return value:       void                                                 */
/*                                                                          */
/* globals referenced: g_hooks                                              */
/*                                                                          */
/* globals changed:    g_hooks                                              */
/*                                                                          */
/* synopsis:                                                                */
/*     Each character in the arguments to Esethook corresponds to a         */
/*     particular key in g_hooks.  For each character in the before         */
/*     string, BEFORE is anded in to the hooks for that key.                */
/*     Similarly, AFTER is anded into the hooks for each key in the         */
/*     after array.                                                         */
/****************************************************************************/

void Esethook (char *before, char *after)

    {
    char *cp; /* string pointer */

#ifdef DEBUG
    debug ("Esethook called");
#endif

    for (cp = before; *cp; cp++)
	{
	g_hooks [*cp] |= BEFORE;
	}
    for (cp = after; *cp; cp++)
	{
	g_hooks [*cp] |= AFTER;
	}
    }

/****************************************************************************/
/* action:  main command dispatch loop                                      */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_dispflag, g_keyvalue, g_sfp, g_popflag,        */
/*                         g_usechar, g_wp,                                 */
/*                         g_debugfp, g_intflag, g_restart,                 */
/*                         g_edfunction, g_killhelper, g_filename           */
/*                                                                          */
/* globals changed:        g_intflag, g_restart, g_killhelper               */
/*                                                                          */
/* synopsis:                                                                */
/*     action is a big FOREVER loop, called directly from main, which       */
/*     waits for the user to type a command and then does whatever is       */
/*     required.  On each pass through the loop, it does the following:     */
/*      .  if doing the last command resulted in setting g_killhelper TRUE  */
/*         (whether by a call to Ekillhelper or some error code), close     */
/*         whatever helper was running and open the previous one for        */
/*         the current file (if any), and set g_killhelper FALSE.           */
/*      .  if doing the last command resulted in setting g_restart non-null */
/*         (i.e. the command resulted in a helper Efatal call), restart     */
/*         the named helper                                                 */
/*      .  checks to see whether any watched files have changed             */
/*      .  checks whether enough keystrokes have accumulated to             */
/*             make it prudent to perform a sync of the disk                */
/*      .  if there is a popbox on the screen, remove it                    */
/*      .  update the screen display if necessary                           */
/*      .  decide if we can use echo-to-break processing                    */
/*         (i.e. if there are just blank spaces after the current cursor    */
/*         position, we can let the operating system do echoing for us)     */
/*      .  read in the next command                                         */
/*      .  set the g_intflag global FALSE                                   */
/*      .  invoke callfunction to do the command                            */
/*     callfunction decides what function to call by using g_keyvalue       */
/*     as an offset into the g_edfunction array, which contains either      */
/*     NULL or a function address for every key.                            */
/****************************************************************************/

void action (void)
    {
    int             key;             /* local copy of g_keyvalue */
    static   int    keycount = 0;    /* used for sync'ing disk   */
    WSTRUCT         *wp;             /* ptr to current window    */
    int             currline;        /* used for Hmod control    */
    int oline;                       /* temporary variable       */

#ifdef TIMEPRINT
    int real, user, sys;             /* used for timing test     */
    struct tms before, after;        /* for sys/user time        */
    long oldtime=0, newtime;         /* for real time            */
#endif

    /* previous cursor line, for Hmod test  */
    currline = ERROR;

    for (;;)
	{
	if (g_killhelper)
	    {

	    /* pop zoom stack if pushed on same file */
	    if (checkpop ())
		popstate (TRUE);
	    else

		/* no helper to pop- just kill this one   */
		closehelper ();

	    g_killhelper = FALSE;
	    }

	/* restart the helper that called Efatal  */
	if (g_restart)
	    {

	    /* don't restart in trace mode       */
	    if (g_debugfp == NULL)
		openhelper (g_restart,
			seq (g_restart, DIRHELPER) ? g_curdir : g_filename,
			NULL);

	    s_free (g_restart);
	    g_restart = NULL;
	    }

	/* sync disk every SYNCDISK keystrokes */
	if ((++keycount > SYNCDISK) && (g_keyvalue != TEXT_KEY))
	    {

#ifdef DEBUG
	    debug ("--- sync'ing disk - keycount = %d", keycount);
#endif

	    keycount = 0;
	    flush_window ();
	    flushrecord (FALSE);
	    (void) fflush (sf_fp (g_sfp));
	    sync ();
	    watchfiles ();
	    }

	/* remove pop up box if displayed */
	if (g_popflag)
	    killbox (TRUE);

	/* If we have a text key display it since with normal typing there
	   never is any input_waiting. This saves a non-blocking read. */

	if (g_keyvalue == TEXT_KEY || !input_waiting ())
	    Edisplay ();

#ifdef TIMEPRINT
	if (g_debugfp && oldtime)
	    {
	    newtime = times (&after);

	    real = newtime - oldtime;
	    user = after.tms_utime - before.tms_utime;
	    sys = after.tms_stime - before.tms_stime;

	    debug ("  real = %d.%2.2d, user = %d.%2.2d, sys = %d.%2.2d\n",
		   real / 60, (real % 60) * 100 / 60,
		   user / 60, (user % 60) * 100 / 60,
		   sys / 60, (sys % 60) * 100 / 60);
	    }
#endif

	/* read in a command */
	nextin();
	key = g_keyvalue;

	if (g_debugfp)
	    {
	    debug ("action:  key = %d", key);
	    if (key == TEXT_KEY)
   	         debug ("  textvalue = %s", g_textvalue);

# ifdef TIMEPRINT
	    oldtime = times (&before);
# endif
	    }

	g_intflag = FALSE;
	callfunction (g_edfunction [key], NOARGS, "", 0);

	/* Check to see if Hmod needs to be called (via call to putgng) */
	if (getstat (w_flags (g_wp), DOUMOD))
	    {
	    wp = g_wp;
	    if (currline != (wp->w__line) + (wp->w__ftline))
		{
		oline = currline - wp->w__ftline;
		currline = (wp->w__line) + (wp->w__ftline);
		if (oline >= 0 && oline <= (wp->w__lrline))
		    putgng (0, wp->w__lrline);
		}
	    }
	}
    }

/****************************************************************************/
/* backspace:  handles the BACKSPACE command                                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_imodeflag                                */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If on the left edge of current window, do nothing.                   */
/*     Otherwise back up the cursor one character to the left.              */
/*     If in insert mode, delete the character we just backed over.         */
/*     If in overwrite mode, replace the character we just backed           */
/*     over with a space and remove any extra unwanted bytes.               */
/****************************************************************************/

void backspace (void)
{
    WSTRUCT *wp=g_wp;      /* current window                               */
    ATTR    *wline;        /* current text line                            */ 

    int nbytes;            /* no of bytes in the character backspaced over */
    int pos;               /* position in text of current character        */
    int width;             /* display width of backed over character       */
    int length;            /* length of text line                          */

    /* if cursor on left edge of field, do nothing */
    if (w_col(wp) == 0)
	return;

    /* get the text line and determine the position of the current character */
    wline = (ATTR *) w_text (wp, w_line(wp));
    pos   = calc_line_position(wline, (w_col(wp) + w_ftcol(wp)));
    if (pos == 0)
        return;

    /* Go back a byte. We're now positioned at the end of the
       character to be deleted */

    --pos;

    /* Count up how many bytes to remove. There is always at least 1 */
    nbytes = 1;
    while (!firstbyte(wline[pos]))
        {
        pos--;
        nbytes++;
        }  

    /* move the screen column back to the previous character */
    width = char_width(wline+pos);
    wp->w__col -= width;

    /* If insert mode delete the character */
    if (g_imodeflag)
	{
	delsp (w_line(wp), pos, nbytes);
	return;
	}

    /* If overwrite mode replace the character with a single space and
       remove any extra unwanted bytes */

    wline[pos] = SPACE | FIRST;
    if (nbytes > 1)
	{
	int tmp;

	length = obj_count(wline) - 1;
	tmp = min(nbytes-1,length-pos+1);
	/* Remove tmp bytes from wline */
	(void) i_strcpy ((short *)wline+pos+1,
			 (short *)wline+pos+1+tmp);
	i_smear ((short)(SPACE | FIRST), tmp,
					 (short *)(wline+length-tmp));
	*(wline+length) = (ATTR) '\0';
	}

    /* update the screen and the text line in cache */
    wp->w__cache[w_line(wp)] = (char *) wline;
    sendbox (w_line(wp), w_line(wp), w_col(wp),w_lrcol(wp));
    w_setmod (wp, w_line(wp));
}

/****************************************************************************/
/* callfunction:  calls a remote function                                   */
/*                                                                          */
/* arguments:              int (*function)() - ptr to function to call      */
/*                         int   type        - how called:  NOARGS or...    */
/*                         char *strvalue    - string argument if           */
/*                                                 type is STRINGARG        */
/*                         int   numvalue    - numeric argument if          */
/*                                                 type is NUMBERARG        */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_readonly, g_hooks, g_restart       */
/*                                                                          */
/* globals changed:        g_keyvalue                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     callfunction essentially just calls the given function               */
/*     with the given arguments.  However, it must also worry               */
/*     about the possibility of attempting to modify a read-only            */
/*     file or read-only form field, and must invoke Hbefore                */
/*     and/or Hafter if the function has before or after hooks              */
/*     on it.                                                               */
/*                                                                          */
/*     If there is a read-only problem, callfunction either calls           */
/*     Eerror or, if hooks are set on the ROFILE or ROFIELD pseudo-         */
/*     keys, calls Hbefore instead.   In any case, it does NOT call         */
/*     the function if the MODIFIER bit of the key is set and the           */
/*     file or field is read-only.                                          */
/*                                                                          */
/*     If there is a before hook on the key, callfunction invokes           */
/*     Hbefore.  If it succeeds, and if g_restart is not set, it            */
/*     invokes the function.  Finally, it invokes Hafter if there is        */
/*     an after hook on the key.                                            */
/*                                                                          */
/*     callfunction stashes a copy of g_keyvalue prior to calling           */
/*     Hbefore in case Hbefore changes it.  This ensures that you           */
/*     still see the hooks for the key you came in with at the time         */
/*     you are deciding whether to call Hafter or not.                      */
/****************************************************************************/
void callfunction (void (*function)(int, char *, int),
			      int type, char *strvalue, int numvalue)
    {
    int key;                  /* local copy of g_keyvalue */

    key = g_keyvalue;

    /* if function is modifier, make sure file and field are not readonly */
    if (g_argbits [key] & MODIFIER)
	{
	if (g_readonly)
	    {
	    if (g_hooks [ROFILE])
		Hbefore (ROFILE, STRINGARG, g_filename, 0);
	    else
		{
		if (g_readonly == DIR_NOREAD)
		    Eerror (M_EDIRREADONLY,
		   "You must have directory write permission to edit the specified file.","");
		else
		    Eerror (M_EREADONLY, "Cannot modify this file.  Check permissions.","");
		}

	    return;
	    }

	if (getstat (w_flags (g_wp), USERRO))
	    {
	    if (g_hooks [ROFIELD])
		Hbefore (ROFIELD, STRINGARG, w_name (g_wp), 0);
	    else
		Eerror (M_EROFIELD, "Cannot modify this field.","");

	    return;
	    }
	}

    if (g_hooks [key] & BEFORE)
	if ((Hbefore (key, type, strvalue, numvalue)) || (g_restart))
	    return;

    /* In case the helper manages to side-effect g_keyvalue.
       Calling Eerror in Ubefore when dealing with the DELSTACK
       is the only extant example, but there are probably others.       */

    g_keyvalue = key;

    if (function == NULL)
	Eerror (M_ENOFUNCTION, "The editor does not recognize the function key or keys used.","");
    else
	(*function) (type, strvalue, numvalue);

    if (g_hooks [key] & AFTER)
	Hafter (key);
    }

/****************************************************************************/
/* clrhooks:  clears the g_hooks table                                      */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_hooks                                          */
/*                                                                          */
/* globals changed:        g_hooks                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     clrhooks simply sets all NUMKEYS elements of the g_hooks array       */
/*     to NULL.                                                             */
/****************************************************************************/

void clrhooks (void)
    {
    int i;

    for (i = 0; i < NUMKEYS; i++)
	g_hooks [i] = '\0';
    }

/****************************************************************************/
/* enter:  handler for the ENTER and LASTARG keys                           */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_usechar, g_lastarg,                */
/*                         g_edfunction, g_wp, g_argbits                    */
/*                                                                          */
/* globals changed:        g_usechar, g_lastarg                             */
/*                                                                          */
/* synopsis:                                                                */
/*     enter handles the ENTER and LASTARG commands.  It puts up            */
/*     the popbox with ENTER in it and, if given LASTARG, the               */
/*     current value of g_lastarg.  It then waits for the user              */
/*     to respond, and parses the response.                                 */
/*                                                                          */
/*     If the user hit CANCEL, enter returns immediately.  Otherwise        */
/*     it updates the value of g_lastarg and finds the function for         */
/*     the key the user terminated the popbox with, and examines the        */
/*     string of characters that came out of the ENTER box (either          */
/*     via g_lastarg or by being typed, or some combination.)  If it        */
/*     can parse the string into a number, it does so.                      */
/*                                                                          */
/*     Now, with the key function and arguments in hand, enter decides      */
/*     what to do.  Possibilities are:                                      */
/*         .   move the cursor and define a region                          */
/*             (e.g. ENTER DOWNARROW)                                       */
/*         .   invoke callfunction with EMPTYARG                            */
/*             (e.g. ENTER USE)                                             */
/*         .   invoke callfunction with STRINGARG                           */
/*             (e.g. ENTER foo USE)                                         */
/*         .   invoke callfunction with NUMBERARG                           */
/*             (e.g. ENTER 5 +PAGE)                                         */
/*         .   invoke Eerror for bad syntax                                 */
/*             (e.g. ENTER foo +PAGE)                                       */
/*     Decisions about what argument combinations constitute bad syntax     */
/*     are made by reference to the g_argbits array element for g_keyvalue. */
/****************************************************************************/

void enter (void)

    {
    WSTRUCT *wp;               /* current window                   */
    char *strvalue;            /* string value                     */
    char *cp;                  /* used to scan retval              */

    int bits;                  /* arg bits for command             */
    int numvalue;              /* numeric value                    */
    int oldline;               /* 0 based field line               */
    int oldcol;                /* 0 based field column             */
    int pos;                   /* position of current char in text */
    int right1;                 /* position of window right edge    */

    ATTR *wline;               /* current text line                */

    void (*function) (int, char *, int);   /* dispatch function    */

    /* reuse the key? */
    if (g_keyvalue == LAST_ARG)
	g_usechar = TRUE;

    /* watch for CANCEL */
    strvalue = ask (TRUE, M_EENTER, "ENTER:                             ");
    if (strvalue == (char *) ERROR)
	return;

    bits = g_argbits [g_keyvalue];
    if (*strvalue)
	{

	/* remember argument for RESTORE button in window */
	s_free (g_lastarg);
	g_lastarg = s_string (strvalue);
	}

    if (g_keyvalue == EXECUTE || g_keyvalue == ENTER)
	{
	s_free (strvalue);
	return;
	}

    function = (void (*)(int, char *, int))g_edfunction [g_keyvalue];

    /* check for empty arg */
    if (*strvalue == '\0')
	{

	/* check for cursor defined region */
	if (bits & CURSORDEF)
	    {
	    wp = g_wp;
	    oldline = w_line(wp) + w_ftline(wp);
	    oldcol  = w_col(wp) + w_ftcol(wp);

	    switch (g_keyvalue)
		{
		case HOME:
		    wp->w__line = 0;
		    wp->w__col = 0;
		    break;

		case UP_ARROW:
		    if (--(wp->w__line) < 0)
			wp->w__line = w_lrline(wp);

		    break;

		case DOWN_ARROW:
		    if (++(wp->w__line) > w_lrline(wp))
			wp->w__line = 0;

		    break;

		case LEFT_ARROW:

		    /* get the text line and calculate the line position */
		    wline = (ATTR *) w_text (wp, w_line(wp));
		    pos   = calc_line_position(wline, w_col(wp));

		    if (--pos < 0)
			pos = calc_line_position(wline, w_lrcol(wp));
                    else 
		        while (!firstbyte(wline[pos]))
			    {
			    if (--pos < 0)
				{
			        pos = calc_line_position(wline, w_lrcol(wp));
				break;
				}
                            }

	            wp->w__col = calc_column(wline, pos);
	            break;


		case RIGHT_ARROW:

		    /* get the current line text and calculate
		       the line position */

		    wline = (ATTR *) w_text(wp, wp->w__line);
		    pos   = calc_line_position(wline, w_col(wp));
		    right1 = calc_line_position(wline, w_lrcol(wp));

		    /* go forward a byte, checking the right hand edge */
		    if (++pos > right1)
		        pos = 0;

		    /* If necessary go forward until we find the beginning of
		       the next character */

		    while (!firstbyte(wline[pos]))
		        {
		        ++pos;
		        if (pos > right1)
			    {
			    pos = 0;
			    }
		        }

		    /* calculate the new screen column */
		    wp->w__col = calc_column(wline,pos);
		    break;

		case RETURN:
		    newline ();
		    Edisplay ();

		    break;

		case TAB:
		case MINUS_TAB:
		    if (!(wp->w__tabs))
			break;

		default:
		    callfunction (function, NOARGS, "", 0);
		    Edisplay ();
		}

	    markregion (BOXREGION, oldline, oldcol,
				   (w_line(wp) + w_ftline(wp)),
				   (w_col(wp) + w_ftcol(wp)));
	    }
	else

	    /* does command takes empty arg? */
	    if (bits & EMPTYARG)
		callfunction (function, EMPTYARG, "", 0);
	    else
		Eerror (M_EARGEMPTY,
			"The specified command does not accept an empty parameter string.","");

	s_free (strvalue);
	return;
	}

    /* argument entered - is it numeric? */
    for (cp = strvalue + (*strvalue == '-'); *cp; cp++)
	if (!isdigit(*cp))
	    break;

    /* non-numeric character was found */
    if (*cp)
	{

	/* command takes string arg */
	if (bits & STRINGARG)
	    callfunction (function, STRINGARG, strvalue, 0);
	else
	    Eerror (M_EARGSTRING, "The specified command does not accept a string parameter.","");

	s_free (strvalue);
	return;
	}

    /* we have a numeric argument */
    if (bits & NUMBERARG && *strvalue != '-')
	{
	numvalue = atoi (strvalue);
	callfunction (function, NUMBERARG, strvalue, numvalue);
	}
    else
	if (bits & STRINGARG)
	    callfunction (function, STRINGARG, strvalue, 0);

	else
	    if (*strvalue == '-')
		Eerror (M_ENEGNUMBER,
			    "Do not specify a negative numeric parameter.","");
	    else
		Eerror (M_EARGNUMBER,
			  "The specified command does not accept a numeric parameter.","");

    s_free (strvalue);

    }

