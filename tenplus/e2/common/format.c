#if !defined(lint)
static char sccsid[] = "@(#)17	1.8  src/tenplus/e2/common/format.c, tenplus, tenplus411, GOLD410 3/23/93 11:50:20";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	dofilter, fill, filter, format
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
/* file:  format.c                                                          */
/*                                                                          */
/* Code for the FORMAT button                                               */
/****************************************************************************/

#include <langinfo.h>
#include <signal.h>
#include <sys/wait.h>
#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"
extern nl_catd g_e2catd;

POINTER *fill (POINTER *, int, int);

/****************************************************************************/
/* dofilter:  executes a specified filter.                                  */
/*                                                                          */
/* arguments:              char *command - to be parsed and executed        */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_intflag, g_keyvalue, g_snode, g_sfp      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     The format of the filter string is "[number[l]] command args".       */
/*     If the string starts with a number it is taken to be the number of   */
/*     paragraphs to pass to the filter.  If the number is provided and is  */
/*     followed immediately by an 'l' character, the number is taken to be  */
/*     a line count rather than a paragraph count.  If no number is given,  */
/*     all lines up to the next blank line are used.                        */
/*                                                                          */
/*     Once the command has been parsed, dofilter writes the specified      */
/*     section of the current file out to a temporary file, sets up another */
/*     temporary file for filter output, calls Emessage to tell the user    */
/*     what is happening, and invokes filter to actually execute the        */
/*     command.                                                             */
/*                                                                          */
/*     If the filter gives an ERROR return, see whether the user stopped    */
/*     the filter by hitting BREAK and, if so, use Eerror to tell him so.   */
/*     Otherwise, if the SHOW_FILTERROR #ifdef is in effect, put up the     */
/*     filter error output in a popbox and wait for the user to type.       */
/*     When SHOW_FILTERROR is not in effect, simply invoke Eerror to tell   */
/*     the user the filter failed without trying to explain why.            */
/*                                                                          */
/*     If the filter returns successfully, see whether the size of the      */
/*     filter output is small enough to fit in the region of the file that  */
/*     was the filter input.  If the output is too large to fit, complain   */
/*     with Eerror.  Otherwise substitute the filter output for the filter  */
/*     input by deleting the old lines and inserting the new ones.          */
/*     When finished, delete the temporary files.                           */
/****************************************************************************/

void dofilter (char *command)
    {
    WSTRUCT *wp=g_wp;       /* current window                    */
    ATTR *text1;            /* text of line                      */
    FILE *fp;               /* input file pointer for filter     */
    POINTER *box;           /* error box                         */

    char *string=command;   /* filter string                     */
    char *infile;           /* input file name for filter        */
    char *outfile;          /* output file name for filter       */
    char *etext;            /* text of line (external form)      */
    char *strbuf;
    char *tmp;

    int line;               /* line number being processed       */
    int dlength;            /* length of current field           */
    int flag;               /* TRUE if processing lines          */
    int count;              /* number of lines or parags to use  */
    int numlines;           /* number of lines processed         */
    int numparags;          /* number of paragraphs processed    */
    int retval;             /* filter return value               */

#ifdef DEBUG
    debug ("dofilter:  full string = '%s'", string);
#endif

    flag = FALSE;
    count = 1;
    
    if (isdigit(*string)) /* if number, use it */
	{
	count = atoi (string);
	while (isdigit(*string))
	    skipmb(&string);

	if (*string == 'l')
	    {
	    flag = TRUE;
	    skipmb(&string);
	    }
	if (!ismbspace(string))
	    {
	    Eerror (M_EARGFILTER, "Use a space between the line count and the command name in:\n%s", command);
	    return;
	    }
        skipmbspaces(&string);
	}
#ifdef DEBUG
    debug ("dofilter:  count = %d, flag = %d, string = '%s'",
	   count, flag, string);
#endif

    if (count > MAXFLINE)
	{
	numtoobig ();
	return;
	}

    infile = s_string (uniqname (TRUE));
    outfile = s_string (uniqname (TRUE));

#ifdef DEBUG
    debug ("dofilter:  infile = '%s', outfile = '%s'", infile, outfile);
#endif

    fp = fopen (infile, "w");
    if (fp == NULL)
	{
	Eerror (M_EBADFILTER, "There is a filter error on the command:\n%s", string);
	return;
	}

    flush_window ();
    dlength   = datalength ();
    numlines  = numparags = 0;
    for (line = w_line (wp) + w_ftline (wp); line < dlength; line++)
	{
        static int kanji = -1;           /* flag indicating code set located */
        char *lp;

        /* find out whether we are using japanese code set */
        if (kanji == -1) {
             kanji = 0;
             lp = nl_langinfo(CODESET);
             if (strcmp(lp, "IBM-932") == 0)
                   kanji = 1;
        }

	/* Note:  the line is read in before the count test because
	       paragraphs do not include the terminating blank line */

	text1 = getli (line);
	if ((*text1 == 0) || (de_attr(*text1) == '.') ||
            ((kanji == 1) && (de_attr(*text1) == 0xa1)))
	    numparags++;

	if ((flag ? numlines : numparags) >= count)
	    {
	    s_free ((char *)text1);
	    break;
	    }

	etext = packup(text1);

	(void) fprintf (fp, "%s\n", etext);
	s_free (etext);
	s_free ((char *)text1);
	numlines++;
	}
    if (ferror (fp))
	{
	(void)fclose (fp);
	Eerror (M_EBADFILTER, "There is a filter error on the command:\n%s", string);
	Remove (infile);
	return;
	}
    if (fclose (fp) == ERROR)
	{
	Remove (infile);
	Eerror (M_EBADFILTER, "There is a filter error on the command:\n%s", string);
	return;
	}

    Emessage (M_EDOFILTER, "Running filter:\n%s", command);
    retval = filter (string, infile, outfile);

    fp = fopen (outfile, "r");
    if (fp == NULL)
	{
	Remove (infile);
	Eerror (M_EBADFILTER, "There is a filter error on the command:\n%s", string);
	return;
	}

    if (retval == ERROR) /* filter returned error */
	{
	if (g_intflag)
	    Eerror (M_EDOBREAK, "The filter was stopped by BREAK.","");
	else

	    {
	    tmp = Egetmessage(M_E_FILTERR, "Filter error on command %s:", FALSE);
	    strbuf = s_alloc (obj_count(tmp) + obj_count (command),
			      T_CHAR, NULL);
	    (void) sprintf (strbuf, tmp, string);
	    s_free (tmp);
	    box = (POINTER *) s_alloc (20, T_POINTER, NULL);
	    box [0] = strbuf;
	    box [1] = s_alloc (0, T_CHAR, NULL);

	    for (line = 2; line < 20; line++)
		{
		box [line] = s_getline (fp);
		if (box [line] == NULL)
		    {
		    box = (POINTER *) s_realloc ((char *)box, line);
		    break;
		    }
		}
		{
		newbox (box);
		onechar (FALSE); /* wait for user to type character */
		}

	    if (g_keyvalue == HELP)
		helpbox (H_EBADFILTER, TRUE);

	    killbox (FALSE);
	    s_free ((char *)box);
	    }

	goto done;
	}

    line = w_line (wp) + w_ftline (wp);
    delli (w_line (wp), numlines, TRUE); /* delete and save lines */

    while ((etext = s_getline (fp)) != NULL)
	{
	Einsline ("", line, 1);
	text1 = unpackup (etext, 0);
	putli (line, text1);
	s_free ((char *)text1);
	s_free (etext);
	line++;
	}
done:

    (void) fclose (fp);
    Remove (infile);
    Remove (outfile);
    s_free (infile);
    s_free (outfile);
    }

/****************************************************************************/
/* fill: fills in an array of lines; returns filled array.                  */
/*                                                                          */
/* arguments:              POINTER *linearray - set of lines to be filled   */
/*                         int leftmargin, rightmargin - fill boundaries    */
/*                                                                          */
/* return value:           POINTER * - the same lines after being filled    */
/*                                                                          */
/* globals referenced:     g_wmodeflag  g_fmtmode                           */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Loop through the given lines, re-arranging the words in them to      */
/*     make each line come as close as possible to full line width.         */
/*                                                                          */
/*     The algorithm is to trim spaces off the end of each line and off     */
/*     the beginning of the next, and then concatenate them with one        */
/*     fill character in between.  The fill character is a space unless     */
/*     the last space character of the first line and the first space       */
/*     character of the next line are both underlined, in which case the    */
/*     fill character is also an underlined space.  Having concatenated     */
/*     lines until the line crosses the right margin, break the line at     */
/*     the position of the last space character to the left of the right    */
/*     margin.  Repeat this sequence until all lines have been filled.      */
/****************************************************************************/

POINTER *fill (POINTER *linearray, int leftmargin, int rightmargin)
    {
    int  i;               /* array index                    */
    int  len;             /* length of current line         */
    int  disp_len;        /* display width of current line  */
    int  rightpos;        /* position of right margin       */
    int  leftpos;         /* position of left margin        */
    char *linechar;       /* 'line' with attributes removed */

    ATTR *cp;             /* line character pointer         */
    ATTR *line;           /* current line                   */
    ATTR *nextline;       /* next line                      */
    ATTR fillchar;        /* space filling char             */

#ifdef CAREFUL

    (void) s_typecheck (linearray, "fill", T_POINTER);

    if (leftmargin < 0)
	fatal ("fill:  leftmargin (%d) less than one", leftmargin);

    if (rightmargin < 0)
	fatal ("fill:  rightmargin (%d) less than one", rightmargin);

    if (rightmargin <= leftmargin)
	fatal ("fill:  rightmargin (%d) <= leftmargin (%d)",
	       rightmargin, leftmargin);
#endif

#ifdef DEBUG
    debug ("fill:  linearray = 0%o, leftmargin = %d, rightmargin = %d",
	     linearray, leftmargin, rightmargin);
#endif

    for (i = 0; i < obj_count (linearray); i++)
	{
	line = (ATTR *) linearray [i];
	for (;;)
	    {
	    if (i > 0) /* if not first line, fix indentation */
		{
		while (isattrspace (line))
		    line = (ATTR *) i_delete ((short *)line, 0, attrlen(line));

		line = (ATTR *) i_insert ((short *)line, 0, leftmargin);
		i_smear ((short)(SPACE | FIRST), leftmargin, (short *)line);
		}

            linechar = a2string(line, i_strlen((short *)line));
	    disp_len = text_width(linechar);
            len      = obj_count(line) - 1;
            rightpos = calc_line_position(line, rightmargin);
            leftpos  = calc_line_position(line, leftmargin);
            s_free(linechar);

	    if ((disp_len > rightmargin) || (i == obj_count (linearray) - 1))
		{
		linearray [i] = (char *) line;
		break;
		}

            /* get next line and remove leading spaces */
	    nextline = (ATTR *) linearray [i + 1];
	    while (isattrspace (nextline)) 
		nextline = (ATTR *) i_delete ((short *)nextline, 0, attrlen(nextline));

	    if (g_wmodeflag == WORDWRAP)
		{

		/* see if last period in line and first space
		   on next line are both underlined */

		fillchar = (ATTR) SPACE | FIRST;
		for (cp = &line [len-1]; cp > line; backupattr(&cp))
		    if (isattrspace (cp))
			{
			fillchar = *cp;
			break;
			}

		for (cp = nextline; *cp; skipattr(&cp))
		    if (isattrspace (cp))
			{
			if (isattrspace(cp))
			    fillchar = *cp;

			break;
			}

		line = (ATTR *) i_insert ((short *)line, len, 1);
		line [len] = fillchar;

		/* add extra space if necessary */
		if (line[len-1] == '.')
		    {
		    line = (ATTR *) i_insert ((short *)line, len, 1);
		    line [len] = fillchar;
		    }
		}

	    line = (ATTR *) i_cat ((short *)line, (short *) nextline);
	    linearray = (POINTER *) s_delete ((char *)linearray, i + 1, 1);
	    }

	/* At this point the line is at least to the right margin. */
	/* If it is exactly the right length, continue.            */

	if (disp_len <= (rightmargin + 1))
	    continue;

	if (g_wmodeflag == CHARWRAP)
	    {
	    cp = &line[rightpos];
	    while (!firstbyte(*cp) || isclosepunct(cp))
		if (g_fmtmode == WRAPPUNCT)
		    --cp;
		else
		    ++cp;
	    }
	else
	    {

	    /* Find split point by looping backwards throught the characters  
	       in the line until the the space character closest to, but not 
	       beyond, the right margin is found. */

	    for (cp = &line[rightpos+1]; cp>&line[leftpos]; backupattr(&cp))
		if (isattrspace (cp))
		    break;

	    /* if we're not on a space character, no split point was found    */
	    if (!isattrspace (cp)) /* if we're at the beginning of the line */
		for (; *cp; skipattr(&cp))    /* split after word */
		    if (isattrspace (cp))
			break;

	    if (*cp == (ATTR) 0) /* if only word on line, don't break */
		continue;
	    }

	/* insert new line and copy rest of line to new line */
	linearray = (POINTER *) s_insert ((char *)linearray, i + 1, 1);
	linearray [i + 1] = (char *) i_string ((short *)cp);
	linearray [i] = (char *) i_realloc ((short *)line, cp - line);
	}

    return (linearray);
    }

/****************************************************************************/
/* filter:  executes a specified filter command                             */
/* Returns ERROR on error or non-zero return status from child process.     */
/* NULL infile or outfile prevents the redirection                          */
/*                                                                          */
/* arguments:              char *command - filter command to execute        */
/*                         char *infile  - file containing input            */
/*                         char *outfile - file in which to put output      */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     filter opens channels to the given infile and outfile and executes   */
/*     the filter command in a sub-process.  Error signals from the process */
/*     are trapped so that they do not crash the editor itself.  The        */
/*     filter procedure returns RET_OK or ERROR depending on whether the    */
/*     sub-process returned zero or nonzero status.                         */
/****************************************************************************/

int filter (char *command, char *infile, char *outfile)
    {
    int i;                   /* for disabling signals    */
    int pid;                 /* process id               */
    int status;              /* return status            */
    extern void quit (void); /* SIGQUIT catcher          */

#ifdef DEBUG
    debug ("filter:  command = '%s', infile = '%s', outfile = '%s'",
	   command, infile, outfile);
#endif

    pid = fork ();

    /* the parent process */
    if (pid)
	(void) signal (SIGQUIT, SIG_IGN);

    else  /* the child process*/
	{

	/* for each signal type, reset the handling to the default, */
	/* UNLESS it is set to be ignored, in which case we leave it */
	/* being ignored. */

	for (i = 1; i < NSIG; i++)
	    if (signal (i, SIG_IGN) != SIG_IGN)
		(void) signal (i, SIG_DFL);

	if (infile)
	    {
	   (void)close (0);

	    if (open (infile, 0) != 0)
	    {
		catclose(g_e2catd);
		exit (1);
	    }
	    }
	if (outfile)
	    {
	    (void)close (1);

	    if (creat (outfile, (mode_t)0666) != 1)
	    {
		catclose(g_e2catd);
		exit (1);
            }
	    (void)close (2);
	    dup (1);
	    }
	execl ("/bin/sh", "sh", "-c", command, (char *)NULL);
	catclose(g_e2catd);
	exit (1);
	}

    while (wait (&status) != pid)
	;

    /* restore quit signal to its original value                */
    /* this keeps subshell quits from hurting the editor        */

    (void) signal (SIGQUIT, (void(*)(int))quit);

#ifdef DEBUG
    debug ("filter:  status = 0%o", status);
#endif

    return (status == 0 ? RET_OK : ERROR);
    }

/****************************************************************************/
/* format:  format current paragraph to current margins.                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current field is indexed, invoke Eerror to complain.          */
/*     Otherwise, find the first line of the paragraph to format,           */
/*     either by simply using the current cursor line or, if the            */
/*     FORMAT_PARAGRAPH #ifdef is in effect, by looping backwards           */
/*     through the lines of the paragraph containing the cursor to          */
/*     the first line.                                                      */
/*                                                                          */
/*     Once the first line is found, pick up and delete the remaining       */
/*     lines of the paragraph, invoke the fill procedure to do the          */
/*     formatting, and put down the filled version of the paragraph.        */
/*     If the window has margins, use them.  Otherwise format the           */
/*     paragraph across the entire window width.                            */
/****************************************************************************/

void format (int type, char *strvalue, int numvalue)
    {
    int firstline;           /* first line of paragraph (zero based) */
    int i;                   /* loop index                           */

    WSTRUCT *wp=g_wp;        /* current window                       */
    ATTR    *text2;          /* text of line                         */
    POINTER *linearray;      /* array of lines in paragraph          */

#ifdef DEBUG
    debug ("format:  current line = %d", w_line (wp) + w_ftline (wp));
#endif

    if (getstat (w_flags (wp), INDEXED))
	{
	Eerror (M_EIDXFORMAT, "Cannot format lines in the field where the cursor is located.","");
	return;
	}

    flush_window ();

    /* find first line of paragraph */
    firstline = w_line (wp) + w_ftline (wp);
    linearray = (POINTER *) s_alloc (0, T_POINTER, "line array");

    for (i = firstline; ; i++)
	{
	text2 = getli (i);
	if (*text2 == 0) /* if nothing on line, all done */
            {
            s_free ((char *)text2);
	    break;
            }

	linearray = s_append (linearray, (char *)text2);
	}

    if (firstline <= w_ftline (wp)) /* if paragraph off top of window, scroll */
	{
	(void) movup (w_ftline (wp) - firstline);
	wp->w__line = 0;
	}

    delli (firstline - w_ftline (wp), obj_count (linearray), TRUE);
    if (w_ismar (wp))
	linearray = fill (linearray, (int)w_lmar (wp), (int)w_rmar (wp));
    else
	linearray = fill (linearray, (int)w_lmar (wp), w_lrcol (wp));

    Einsline ("", firstline, obj_count (linearray));
    for (i = 0; i < obj_count (linearray); i++)
	putli (firstline + i, (ATTR *)linearray [i]);

    s_free ((char *) linearray);
    }
