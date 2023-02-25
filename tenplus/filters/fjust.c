#if !defined(lint)
static char sccsid[] = "@(#)61	1.6  src/tenplus/filters/fjust.c, tenplus, tenplus411, GOLD410 4/13/94 16:54:17";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, text, leadbl, lbreak, getword, putword, putline,
 *		width, wbug
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
 */
 
/* This program does filling or justifying, depending on whether
 * the the last segment of the name by which it is called contains a 'j'.
 *
 * The code is adapted from the ratfor code in Software Tools, by
 * Kernighan and Plauger, Chapter 7.
 *
 * Optional argument -li, where i is an integer, sets the right margin
 * to i.  The default is 65.
 *
 * Restrictions:
 *      (1)  Incorrectly sets the left margin if the first line of
 *           any paragraph is more than twice as long as the
 *           specified right margin.
 *      (2)  Input lines had better not be longer than BUFLEN
 *           characters.
 */

#include <chardefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#define NEWLINE L'\n'
#define NUL     L'\0'
#define TAB     L'\t'
#define BACKSPACE  L'\b'
#define PERIOD  L'.'
#define EXMARK  L'!'
#define QMARK   L'?'
#define HOLD    L'\001'
#define BUFLEN  512
#define RTMARG  65

#ifdef DEBUG
FILE  *bugfile;
#endif


void leadbl (void);
void putline (void);
void lbreak (void);
void putword (void);
int  getword (void);
void text (void);
int width (void);

wchar_t inbuf[BUFLEN], wordbuf[BUFLEN], outbuf[BUFLEN];
int  ix = 0;     /* Index into inbuf */
int  outp = 0;   /* First unused character position in outbuf */
int  outflag = 0;/* 1 means there is an incomplete output line, else 0 */
int  outw = 0;   /* Display width of text currently in outbuf */
int  outwds = 0; /* Number of words in outbuf */
int  indent1;    /* # of spaces to indent on first line of paragraph */
int  indent2 = 0;/* # of spaces to indent on other lines of paragraph */
int  inline = 1; /* Input line number within paragraph */
int  outline = 1;/* Output line number within paragraph */
int  rtmarg;     /* Right margin */
int  dir;        /* Direction to spread blanks while justifying, 0 or 1 */
int  just;       /* Non-zero if just, zero if fill */

int MultibyteCodeSet;

main (int argc, char **argv)
{
	register char  *arg1;
	register char  *q;
	register int  n;

        /* set the local environment */
        (void) setlocale(LC_ALL, "");

        if (MB_CUR_MAX == 1)
             MultibyteCodeSet = 0;
        else MultibyteCodeSet = 1;
    
#ifdef DEBUG
	bugfile = fopen ("de.bug", "w");
#endif

/* find out what name I was called by */
	for (arg1 = q = argv[0]; *q; q++ )
		if (*q == DIR_SEPARATOR) arg1 = q;
	for (q = arg1; *q; q++)
	        if (*q == 'j')
		{       just++;
			break;
		}

	rtmarg = RTMARG;
	if (argc > 1)
	{
	    /* First arg, if any, specifies right margin. */
	    arg1 = argv[1];
	    if (*arg1++ == '-'  &&  *arg1++ == 'l')
	    {
		n = atoi (arg1);
		if (0 < n  &&  n < BUFLEN)  rtmarg = n;
	    }
	}
	while (fgetws (inbuf, BUFLEN, stdin) != NULL)  text ();
	lbreak ();  /* Flush the last line, if any. */
	exit (0);
        return(0);
	/*NOTREACHED*/
}


void text (void)  /* Process one input line. */
{
	leadbl ();
	if (inbuf[0] == NEWLINE)  /* End of paragraph */
	{
	    lbreak ();
	    putline ();
	    inline = outline = 1;
	    indent2 = 0;
	    dir = 0;
	}
	else
	{
	    for (ix = 0;  getword () > 0;  )  putword ();
	    inline++;
	}
}


void leadbl (void)  /* Delete leading blanks and tabs, and determine
	      indentation if necessary. */
{
	register int  i, j;
	int  t;
	wchar_t  ch;

	t = 0;  /* t is number of extra spaces due to tabs. */
	for (i = 0;  ch = inbuf[i], iswspace(ch) && (ch != NEWLINE);  )
	{
	    i++;
	    if (ch == TAB)
		while ((i + t) % 8)  t++;
	}
	/* First two lines of paragraph determine indentation.... */
	if (inline == 1)  indent1 = i + t;
	if (inline == 2)  indent2 = i + t;
	/* Move line to left.... */
	for (j = 0;  inbuf[i] != NUL;  )  inbuf[j++] = inbuf[i++];
	inbuf[j] = NUL;
}


void lbreak (void)  /* Force the current line to end. */
{
	if (outflag)  putline ();
}


int getword (void)  /* Copy a word from inbuf to wordbuf, updating ix. */
{
	register int  j;

	while (iswspace(inbuf[ix]) && (inbuf[ix] != NEWLINE))  ix++;
	j = 0;
	while ((inbuf[ix] != NUL)  &&  !iswspace(inbuf[ix]))
			wordbuf[j++] = inbuf[ix++];
	wordbuf[j] = NUL;
	return (j);
}


void putword (void)  /* Put a word from wordbuf to output buffer. */
{
	register int  i;
	int  w, len, ind;
	register int  ii, jj;
	int  nextra, ne, nb, nholes;

	w = width ();
	ind = outline == 1 ? indent1 : indent2;
	len = rtmarg - ind;
	if (outflag  &&  outw + w > len)
	{
	  if (just)
	  {
	    nextra = len - outw + 1;
	    if (outbuf[outp-2] == HOLD)
	    {
		outbuf[outp-2] = L' ';
		nextra++;   outp--;   outw--;
	    }
	    if (nextra > 0  &&  outwds > 1)
	    {
		dir = !dir;
		ne = nextra;
		nholes = outwds - 1;
		ii = outp - 2;
		jj = ii + ne;
		while (ii < jj)
		{
		    outbuf[jj] = outbuf[ii];
		    if (iswspace(outbuf[ii]))
		    {
			nb = dir  ?  (ne - 1) / nholes + 1  :  ne / nholes;
			ne -= nb;
			nholes--;
			for (;  nb > 0;  nb--)  outbuf[--jj] = L' ';
		    }
		    ii--;  jj--;
		}
		outp += nextra - 1;
	    }
	  }
	  lbreak ();
	}
	/* Copy wordbuf to outbuf....  */
	for (i = 0;  wordbuf[i] != NUL;  )
	    outbuf[outp++] = wordbuf[i++];
	/*  After some chars. such as a period, insert another char.
	 *  as a place-holder.  It will later be changed to a blank.
	 *  Thus a period  will be printed with an extra space after
	 *  it.
	 */
	if (outbuf[outp-1] == PERIOD  ||
	    outbuf[outp-1] == EXMARK  ||  outbuf[outp-1] == QMARK)
	    outbuf[outp++] = HOLD;
	outbuf[outp++] = L' ';
	outflag = 1;
	outw += w + 1;
	outwds++;
}


void putline (void)  /* Output a line with indentation. */
{
	register int  i, ind;
	register wchar_t  *p;

	outbuf[outp++] = NEWLINE;
	outbuf[outp] = NUL;
	/* If outline is 2, and the first input line is very long,
	 * indent2 will be 0, which is incorrect.
	 */
	ind = outline == 1 ? indent1 : indent2;
	for (i = 1;  i <= ind;  i++)  (void) putc (SPACE, stdout);
	/* Change place-holders to blanks.... */
	for (p = outbuf;  *p;  p++)
	    if (*p == HOLD)  *p = L' ';
	(void) fputws (outbuf, stdout);
	outline++;
	outp = outw = outwds = outflag = 0;
}


int width (void)  /* Computes the width of the word in wordbuf, which differs
	   * from the number of characters.
	   */
{
	register int  i, n;
        int width_last_char = 0;

	n = 0;
	for (i = 0;  wordbuf[i] != NUL;  i++)
	{
	    if (wordbuf[i] == BACKSPACE)  n -= width_last_char;
	    else if (wordbuf[i] != NEWLINE)  n +=
                   ((width_last_char = wcwidth( wordbuf[i])) < 0 ? 1 : width_last_char);
	}
	/* Periods, etc., will be printed with an extra space
	 * following....
	 */
	if (wordbuf[i-1] == PERIOD  ||
	    wordbuf[i-1] == EXMARK  ||  wordbuf[i-1] == QMARK)  n++;
	return (n);
}


#ifdef DEBUG
wbug (a, b, c, d, e, f, g)
	{(void) fprintf (bugfile, a, b, c, d, e, f, g);}
#endif
