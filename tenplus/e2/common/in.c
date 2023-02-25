#if !defined(lint)
static char sccsid[] = "@(#)40	1.11  src/tenplus/e2/common/in.c, tenplus, tenplus411, GOLD410 3/3/94 18:48:54";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   inbox, input_waiting, nextchar,
 *              onechar, quote, timeout, adj_shift
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
/* file:  in.c                                                              */
/*                                                                          */
/* input routines for the TENPLUS editor                                    */
/****************************************************************************/

/* system include files */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <memory.h>

/* application include files */
#include "def.h"
#include "termcap.h"
#include "keynames.h"
#include "INeditor_msg.h"

/* locally used definitions */
#define BOXWIDTH   MAXL
#define FAST_WAIT  5                /* number of seconds to wait for user
				       input 1st time  */

#define SLOW_WAIT  30               /* wait if user has not typed since last
				       timeout */


/* static variables and functions */
static unsigned char *inptr;
static unsigned char inbuf [80 * MB_LEN_MAX];  /* input buffer  */
static int g_alarmflag;                        /* used to time-out reads
						   in nextchar() */
static int timeout (void);
static int g_waitsecs = FAST_WAIT;             /* timeout wait in seconds */

static void adj_shift(ATTR *, ATTR *, int, int *);

/* external definitions */
extern g_St_fd;                   /* fd for non-blocking reads on tty */
extern int errno;                 /* kernel error number */
extern int g_regionflag;          /* true if defining a region */


/* global variables in this source files */
int g_nbytes;                                   /* number of unprocessed;
						   bytes in inbuf[] */

unsigned char tmpbuf [80 * MB_LEN_MAX];         /* temp input buffer */

/****************************************************************************/
/* inbox:  lets user enter text in a box on the tv                          */
/* NOTE:  Box must be only one line high in this version                    */
/*                                                                          */
/* arguments:              int top, bottom, left, right -                   */
/*                                 zero based tv coords of the box          */
/*                         int flag - TRUE if doing an ENTER                */
/*                                                                          */
/* return value:           char * - the user's response (structured)        */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_textvalue g_lastarg                */
/*                                                                          */
/* globals changed:        g_lastarg, g_keyvalue                            */
/*                                                                          */
/* synopsis:                                                                */
/*     If top and bottom are different, call fatal.                         */
/*     Loop on getting an input character from the user. The following      */
/*     keys are processed by inbox. QUOTE, TEXT_KEY, INSERT_MODE,           */
/*     LAST_ARG, BEGIN_LINE, END_LINE, LEFT_ARROW, RIGHT_ARROW, BACKSPACE,  */
/*     DELETE_CHARACTER, EXECUTE, CANCEL, HELP. If in an ENTER box then     */
/*     all other keys fall into the EXECUTE case, otherwise all other keys  */
/*     beep at the user. HELP saves the inbox text so that it can be reused */
/*     when the help message is cancelled.                                  */
/*     QUOTE: Get the next char and convert to control char and display.    */
/*     TEXT_KEY: Add char to buffer, display, scroll if at right edge.      */
/*     INSERT_MODE: Toggle INSERT/OVERWRITE message.                        */
/*     LAST_ARG: Put g_lastarg in buffer and display.                       */
/*     BEGIN_LINE: Put cursor at beginning of buffer. (Not first char)      */
/*     END_LINE: Put cursor one past last char in buffer.                   */
/*     LEFT_ARROW: Move cursor left, scroll at left edge.                   */
/*     RIGHT_ARROW: Move cursor right, scroll at right edge.                */
/*     BACKSPACE: Move cursor left one and fall into DELETE_CHARACTER.      */
/*     DELETE_CHARACTER: Delete char the cursor is one by scrolling text to */
/*                       right of cursor left one position.                 */
/*     EXECUTE: null-terminate buffer and return allocated copy.            */
/*     CANCEL: null-terminate buffer and return allocated copy of it.       */
/*     HELP: null-terminate buffer, save one copy in allocated reuse_buffer */
/*           and return an allocated copy.                                  */
/****************************************************************************/

char *inbox (int top, int bottom, int left1, int right1, int flag)
{
    ATTR *cp;                  /* buffer pointer */
    ATTR *i, *tcp;
    ATTR buffer [BOXWIDTH];    /* input buffer */
    ATTR *newstr;              /* holds unpackup-ed version of g_lastarg */
    static ATTR *reuse_buffer; /* initial user text for HELP */

    static ATTR aspace[BOXWIDTH];
    static int aspace_initialised = FALSE;

    unsigned char ch;

    int j, width;              /* width of box (in columns)           */
    int actual_width=0;        /* width of text entered so far        */
    int nbytes;                /* number of bytes in the current char */
    int shift = 0;             /* amount we have shifted to right     */
    int widthpos = -1;           /* position of 'width' in text buffer  */

    char text_in_box = FALSE;  /* text in box: catch edits            */
    static char reuse_tib;     /* saved text_in_box for HELP          */

    /* box must be only one line high */
    if (top != bottom)
	fatal ("inbox:  top (%d) not equal to bottom (%d)", top, bottom);
    /* initialise aspace if not already done */
    if (!aspace_initialised) {
       int inum = 0;
       for ( ; inum < BOXWIDTH - 1; inum++)
         aspace[inum] = (ATTR) SPACE | FIRST;
       aspace[BOXWIDTH - 1] = (ATTR)0;
       aspace_initialised = TRUE;
     }

    /* initialise the buffer and determine the box width */
    cp = buffer;
    memset(cp, '\0', sizeof(buffer));
    width = right1-left1+1;

    /* restore the box if returning from HELP */
    if (reuse_buffer)
	{
	i_strcpy ((short *)buffer, (short *)reuse_buffer);
	s_free ((char *)reuse_buffer);
	reuse_buffer = NULL;
	cp           = buffer;
	text_in_box  = reuse_tib;
	widthpos     = calc_line_position(buffer, width);
	Si_ovector (cp, widthpos, top, left1);
	(void) Si_display (FALSE);
	}

    St_goto (top, left1);
    St_flush ();

    for (;;)
	{

	/* get the next input */
	nextin ();
	switch (g_keyvalue)
	    {

	    case QUOTE:

		/* get the character to be quoted */
		nextin ();

		if (g_keyvalue == TEXT_KEY)
		    g_textvalue[0] = control(g_textvalue[0]);
		else
		    {
		    reuse_buffer = (ATTR *) i_string ((short *)buffer);
		    reuse_tib    = text_in_box;
		    return (NULL);
		    }

	    /* text character */
	    case TEXT_KEY:
		text_in_box = TRUE;
		while (!firstbyte (*cp))
		    cp--;

		 /* find out how many bytes we've got */
		 nbytes = mblen((char *) g_textvalue, MB_CUR_MAX);
		 /* copy at least 1 char */
		 if (nbytes < 1) nbytes = 1;

		/* if inserting is there room for the new text? */
		if (g_imodeflag && 
                            (i_strlen((short *)buffer)+nbytes) >= BOXWIDTH)
		    {
		    Ebell ();
		    break;
		    }

		/* only go as far as the end of the buffer */
		if (cp >= &buffer[BOXWIDTH-1])
		    {
		    Ebell ();
		    break;
		    }

		/* move the character to the text buffer */
		for (j = 0; j < nbytes; j++)
		    {
		    if (g_imodeflag)
			{
			for (i = (&buffer[BOXWIDTH]) - 2; i >= cp; i--)
			    *(i+1) = *i;
			}

		/* Put the byte into the buffer. If it is the first byte of
		   the character, set the 'FIRST' flag in the attribute. */

		    if (j)
			*cp++ = g_textvalue[j];
		    else
			*cp++ = g_textvalue[j] | FIRST;
		    }

		/* Determine whether the line has reached the width of the
                   box. If it has start to keep track of how far we've moved */

                adj_shift(buffer, cp, width, &shift);

		break;


	    case INSERT_MODE:

		if(!text_in_box && flag)
		    return (a2string(buffer, i_strlen((short *)buffer)));

		insert(NOARGS);
		fixname();
		break;


	    case LAST_ARG:

		text_in_box = TRUE;
		if (g_lastarg == NULL)
		    g_lastarg = s_alloc (0, T_CHAR, NULL);

		j = i_strlen ((short *)buffer);
		newstr = unpackup(g_lastarg, 0);

		/* append the last argument to the end of the text */
		if (j + i_strlen ((short *)newstr) < BOXWIDTH)
		    {
		    i_strcpy ((short *)(buffer + j), (short *)newstr);
		    s_free((char *)newstr);
		    }
		else
		    {
		    s_free((char *)newstr);
		    Ebell ();
		    break;
		    }

		/* set the pointer to the new end of the buffer */
		cp       = buffer + i_strlen ((short *)buffer);

                shift = 0;
                adj_shift(buffer, cp, width, &shift);
     
		break;


	    case BEGIN_LINE:

		/* if the first key is an BEGIN_LINE key and it is an ENTER
		   box then do the cursor defined region */

		if(!text_in_box && flag)
		    return (a2string(buffer, i_strlen((short *)buffer)));

		cp    = buffer;
		shift = 0;
		break;


	    case END_LINE:

		/* if the first key is an END_LINE key and it is an ENTER
		   box then do the cursor defined region */

		if(!text_in_box && flag)
		    return (a2string(buffer, i_strlen((short *)buffer)));

		/* set pointer to the end of the line and remove
		   trailing spaces */

		cp = buffer + i_strlen ((short *)buffer);
		while (de_attr(*--cp) == SPACE)
		    *cp = (ATTR)0;

		cp++;

                adj_shift(buffer, cp, width, &shift);

                Si_ovector(aspace, 1, top, right1);
		break;


	    case LEFT_ARROW:

		/* if the first key is an ARROW key and it is an ENTER
		   box then do the cursor defined region */

		if(!text_in_box && flag)
		    return (a2string(buffer, i_strlen((short *)buffer)));

		while (!firstbyte (*cp))
		    cp--;

		/* if on first column, ignore */
		if (cp > buffer)
		    {

		    /* Move to the LEFT until we get to the beginning of
		       the previous character. If we are moving back over
		       spaces null terminate the buffer as we go */

		    j = max (0, i_strlen ((short *)buffer) - 1);
		    if (cp == &buffer[j] && *cp == (SPACE | FIRST))
			   *cp = (ATTR)0;

                    backupattr(&cp);
                    adj_shift(buffer, cp, width, &shift);

		    }

		break;


	    case RIGHT_ARROW:

		/* if the first key is an ARROW key and it is an ENTER
		   box then do the cursor defined region */

		if(!text_in_box && flag)
		    return (a2string(buffer, i_strlen((short *)buffer)));

		while (!firstbyte (*cp))
		    cp--;

		/* Move to the RIGHT until we get to the beginning of
		   the next character. Where we move past the end
		   of the current text insert a space instead. */

		   /* don't go beyond the end of the buffer */
		   if (cp == &buffer[BOXWIDTH-1])
		      {
		      Ebell ();
		      break;
		      }
		   else
		      {
			  if (*cp == (ATTR)0)
			      *cp++ = SPACE | FIRST;
                          else skipattr(&cp);

                          adj_shift(buffer, cp, width, &shift);
		      }

                Si_ovector(aspace, 1, top, right1);
		break;


	    case BACKSPACE:

		while (!firstbyte (*cp))
		    cp--;

		/* ignore if on the first column */
		if (cp <= buffer)
		    break;

		/* go back one byte. DELETE CHARACTER case will position
		   us at the beginning of the character if required */

		cp--;

	    /* fall into DELETE-CHARACTER */


	    case DELETE_CHARACTER:

		/* if the first key is a DELCH key and it is an ENTER
		   box then do the deletion to end of line */

		if(!text_in_box && flag)
		    return (a2string(buffer, i_strlen((short *)buffer)));

		while (!firstbyte (*cp))
		    cp--;

                j      = char_width(cp);
		nbytes = attrlen(cp);
		tcp    = cp;
		for (; *tcp && (tcp < &buffer[BOXWIDTH]); tcp++)
		    *tcp = *(tcp + nbytes);

		shift = min (cp - buffer, shift);
		Si_ovector(aspace,j,top, min(right1 - j + 1,
				      left1+attr_text_width(buffer)-shift));

		break;

	    default:

		if (!flag)
		    {
		    Ebell ();
		    break;
		    }


	    case EXECUTE:
	    case CANCEL:

		/* convert ATTR buffer to a char string */
		return (a2string(buffer, i_strlen((short *)buffer)));


	    case HELP:

		/* save state info for return from help */
		reuse_buffer = (ATTR *) i_string ((short *)buffer);
		reuse_tib    = text_in_box;
		return (a2string(buffer, i_strlen((short *)buffer)));

	    }

	/* repaint the line and position the cursor */
        Si_ovector (aspace, width, top, left1);

	widthpos = calc_line_position(buffer + shift, width);

        Si_ovector (buffer+shift, widthpos, top, left1);
           

	(void) Si_display (FALSE);
	St_goto (top, calc_column(buffer + shift, 
                                  (cp-buffer-shift)) + left1);

	St_flush ();
    }
}

/****************************************************************************/
/* input_waiting: predicate for interruptability                            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - TRUE or FALSE whether input is waiting     */
/*                                                                          */
/* globals referenced:     g_nbytes, g_St_fd, errno, inbuf, inptr           */
/*                                                                          */
/* globals changed:        g_nbytes, inbuf, inptr                           */
/*                                                                          */
/* synopsis:                                                                */
/*     input_waiting returns TRUE if the user has typed some input not      */
/*     yet processed, so that it is appropriate to interrupt the display    */
/*     in progress.  The test is made as follows:                           */
/*                                                                          */
/*         If the g_nbytes global is not zero, then there is input waiting  */
/*                                                                          */
/*         If g_nbytes is zero, see if it is possible to read a             */
/*         bufferfull from the g_St_fd channel.  If so, return TRUE.        */
/*         Otherwise no input is waiting.                                   */
/*                                                                          */
/****************************************************************************/

int input_waiting (void)
{

    int         eno;      /* local copy of errno */

    if (g_nbytes > 0)
	return (TRUE);

    /* do a non-blocking read on /dev/tty */
    do {g_nbytes = read (g_St_fd, inbuf, sizeof (inbuf));}
       while (g_nbytes == -1 && errno == EINTR);

    eno = errno;
    if (g_nbytes == 0)
	return (FALSE);

     /* This really shouldn't happen, but some implementations of the
	i/o subsystem seem to do it to us, so defend against it. */

    if (g_nbytes == -1 && (eno == EWOULDBLOCK || eno == EAGAIN))
       {
       g_nbytes = 0;
       return (FALSE);
       }

    else if (g_nbytes < 0)
	fatal ("input_waiting:  can't handle error return from read");

    inbuf [g_nbytes] = '\0';
    inptr = inbuf;

    return (TRUE);
}

/****************************************************************************/
/* nextchar:  get the next character from the terminal                      */
/*                                                                          */
/* arguments:                      chr,  the next character in string       */
/*                                       format                             */
/*                                                                          */
/* return value:                   num, the number of bytes in the returned */
/*                                      character                           */
/*                                                                          */
/* globals referenced:             g_nbytes, inptr, inbuf                   */
/*                                                                          */
/* globals changed:                g_nbytes, inptr, inbuf                   */
/*                                                                          */
/* synopsis:                                                                */
/*     See if any unread input is waiting in the global inbuf cache.        */
/*     If so, pull off a character and return.  Otherwise do a read to get  */
/*     the next buffer full, pull off the next character and return.        */
/****************************************************************************/

int nextchar (unsigned char *chr)
    {

    int          eno;       /* local copy of errno */
    int n, ntmp;            /* number of bytes read into tmpbuf */
    int num;                /* number of bytes in the returned char */

    unsigned char *cp;
    unsigned char *pin;     /* scratch pointer into inbuf */
    unsigned char *ptmp;    /* scratch pointer into tmpbuf */

    /* If there are bytes in the buffer unprocessed get the next
       character from it. We can assume that 'inptr' always points
       the first byte of a multibyte sequence */

    if (g_nbytes)
	{
	num = mblen( (char *) inptr, MB_CUR_MAX);
	if (num == -1)
	    return(num);

	cp = chr;
	for (n=0; n<num; n++)
	    {
	    *cp++ = *inptr++;
	    g_nbytes--;
	    }

#ifdef DEBUG
	debug ("nextchar() returns %s %d from cache", chr, num);
#endif

	*cp = '\0';
	return (num);
	}

    /* start of read loop */
    do
	{
	g_alarmflag = FALSE;
	if ((!g_popflag) && (!g_regionflag))
	    {
	    (void) signal (SIGALRM, (void (*)(int))timeout);
	    alarm (g_waitsecs);
	    }

	inptr = inbuf;
	g_nbytes = read (0, tmpbuf, sizeof (tmpbuf));
	eno = errno;

	pin = inbuf;
	ptmp = tmpbuf;
	for (ntmp = g_nbytes; ntmp > 0 ; ntmp--)
	    if (*ptmp)
		*pin++ = *ptmp++;
	    else                /* change input NUL to ^[^[0 */
		{
		*pin++ = (unsigned char) '\033';
		*pin++ = (unsigned char) '\033';
		*pin++ = (unsigned char) '0';
		ptmp++;
		g_nbytes+=2;
		}

	if (g_alarmflag)
	    {
	    g_nbytes = 0;
	    g_waitsecs = SLOW_WAIT; /* slow down future timeouts */
	    watchfiles ();
	    g_alarmflag = TRUE; /* force continuation of the loop */
	    }
	else
	    {
	    g_waitsecs = FAST_WAIT; /* didn't time out, so use fast waits */
	    alarm (0); /* disable alarm */
	    }


	} /* end of do/while loop */
    while (g_alarmflag || (g_nbytes == -1 && eno == EINTR));

    if ((g_nbytes == 0) ||
	 (g_nbytes == -1 && (eno == EWOULDBLOCK || eno == EAGAIN)))
	{
	    cp = chr;
	    g_nbytes = 0;
	    *cp = '\0';
	    return(0);
	}

#ifdef DEBUG
    debug ("nextchar() read %d bytes", g_nbytes);
#endif

    inbuf [g_nbytes] = '\0';

    /* Return a complete character to the calling function.
       We can assume that 'inptr' always points the first byte
       of the character */

    num = mblen( (char *) inptr, MB_CUR_MAX);
    if (num == -1)
	return(num);

    cp = chr;
    for (n=0; n<num; n++)
	{
	*cp++ = *inptr++;
	g_nbytes--;
	}

    *cp = '\0';
    return (num);

}

/****************************************************************************/
/* onechar:  wait for user to type EXECUTE, HELP or CANCEL                  */
/*                                                                          */
/* arguments:              int flag - TRUE if EXECUTE is OK                 */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue                                       */
/*                                                                          */
/* globals changed:        g_keyvalue                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     Call nextin to get a character.  If it is REFRESH, Refresh the       */
/*     screen and then proceed as if the user hit CANCEL.  If it is EXECUTE */
/*     and the flag argument is set, or if it is HELP or CANCEL, return.    */
/*     In the case of any other key, beep and wait for a better character.  */
/****************************************************************************/

void onechar (int flag)            /* TRUE if EXECUTE is OK */
    {

    for (;;)
	{
	if (Si_display (FALSE))
	    St_flush ();

	/* get a character */
	nextin ();

	switch (g_keyvalue)
	    {
	    case REFRESH:
		Refresh ();
		g_keyvalue = CANCEL;

	    case HELP:
	    case CANCEL:
		return;

	    case EXECUTE:
		if (flag)
		    {
		    return;
		    }

	    default:
		Ebell ();
	    }
	}
    }

/****************************************************************************/
/* quote:  handler for the QUOTE command                                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           VOID                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_textvalue                          */
/*                                                                          */
/* globals changed:        g_textvalue                                      */
/*                                                                          */
/* synopsis:                                                                */
/*                                                                          */
/*     Invoke nextin to get a character.  If it is a TEXT, convert          */
/*     g_textvalue from a normal character to the corresponding             */
/*     control character.  Otherwise invoke Eerror to complain about        */
/*     attempt to quote a non-text.                                         */
/****************************************************************************/

void quote (void)
    {

    /* get a character */
    nextin ();

    if (g_keyvalue == TEXT_KEY)
	{
	g_textvalue[0] = control(g_textvalue[0]);
	text ();
	}
    else
	Eerror (M_EQUOTE, "Specify an alphabetic character with the QUOTE command.");
    }

/****************************************************************************/
/* timeout:  routine to catch alarms for time-out reads                     */
/****************************************************************************/

static int timeout (void)
    {
    g_alarmflag = TRUE;
    }

/******************************************************************

ADJ_SHIFT

buffer + shift points at the left most character in the current
enter box that is not invariant text (ie that has been entered
by the user). 

cp points at the character that is currently
under the character. This character could be the 0 ATTR if the
cursor is at the end of the entered text.

width is the width in columns of the enter box.

This function adjusts shift so that the character pointed at by
cp is always in the box. If cp points at the null ATTR, 1 
columni is assumed (long enough for the cursor itself). 

*****************************************************************/

static void adj_shift(ATTR *buffer, ATTR *cp, int width, 
                              int *shift)
{
  if (cp - buffer < *shift)
       /* cursor is to the left of the visible text, so scroll the 
          visible text to the right */
       *shift = cp - buffer; 
  else {
       /* number of columns to be reserved for character under the
          cursor  */
       int reserved_columns = (*cp == (ATTR)0 ?
                               1 :
                               char_width(cp));
     
       /* number of columns the visible text has to be scrolled
          to the left */
       int nbr_columns_to_shift = 
              max(0, calc_column(buffer + *shift, cp - buffer - *shift) + 
                     reserved_columns - width);

       while (nbr_columns_to_shift > 0) {
           nbr_columns_to_shift -= char_width(buffer + *shift);
           skipattr_idx(buffer, shift);
       }
   } 
}

