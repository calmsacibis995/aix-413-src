#if !defined(lint)
static char sccsid[] = "@(#)61	1.8  src/tenplus/e2/common/put.c, tenplus, tenplus411, GOLD410 10/27/93 11:13:45";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	boxpick, opendelfile, openput, putback, rstrbox,
 *		rstrlines, rstrtext, savelines, textpick
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
/* File: put.c - routines for PICK/PUT/CLOSE/RESTOR.                        */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

FILE *putfp;     /* PICK/PUT/CLOSE/RESTORE stack       */
FILE *delfp;     /* DELETE/UNDEL/DELCPY/UNDELCPY stack */
char *delname;   /* name of delete file                */
char *putname;   /* name of put  file                  */
char *poundname; /* name of # file                     */

int rstrbox (FILE *, int);
int rstrlines (FILE *, char, int);
int rstrtext (FILE *, int);

/****************************************************************************/
/* boxpick:  picks and optionally closes a rectangular region               */
/*                                                                          */
/* arguments:              int top, bottom, left, right                     */
/*                                  - 0-based field coords of picked box    */
/*                         int flag - TRUE if box should be deleted         */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_argbits, g_keyvalue, g_brkflag,          */
/*                         delfp, putfp                                     */
/*                                                                          */
/* globals changed:        g_brkflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     boxpick refers to the g_argbits entry corresponding to g_keyvalue    */
/*     to determine in which stack to put the picked box.  If the DELSTACK  */
/*     bit is set for this key in g_argbits, the box goes into the delete   */
/*     stack; otherwise into the put stack.  It then writes header data     */
/*     for the box into the stack as the triple {'b', height, width}.       */
/*                                                                          */
/*     To actually pick the box, g_brkflag is set FALSE so that the pick    */
/*     operation is not interrupted.  Then boxpick loops through the lines  */
/*     of the box, writing the picked characters into the stack and padding */
/*     lines with spaces if necessary.  If the input flag is set, the       */
/*     picked characters are also deleted from the window.  Finally the     */
/*     file location of the beginning of this pick frame is written into    */
/*     the stack, and g_brkflag is set back to TRUE.                        */
/*                                                                          */
/*     If the input flag was set so that the box has been deleted, boxpick  */
/*     also scrolls the screen if necessary and invokes sendbox to          */
/*     redisplay the affected region of the screen.                         */
/****************************************************************************/

void boxpick (int top, int bottom, int left1, int right1, int flag)
    {
    int line;       /* zero based field line             */
    int leftpos;    /* left position in text line        */
    int rightpos;   /* right position in text line       */
    int tcount;     /* length of text line               */
    int actual_width; 
    int fill;       /* Number of spaces to fill          */

    long fileloc;   /* file location of frame            */

    FILE *fp;       /* file pointer (putfp or delfp)     */
    ATTR *text1;    /* text from line                    */
    ATTR *leftp, *rightp, *tp;

#ifdef DEBUG
    debug ("boxpick:  t = %d, b = %d, l = %d, r = %d, flag = %c",
	   top, bottom, left1, right1, flag ? 'T' : 'F');
#endif

    if (left1 == right1)
	return;

    fp = (g_argbits [g_keyvalue] & DELSTACK) ? delfp : putfp;
    fileloc = ftell (fp);
    flush_window ();

retry: /* disk full retry point */

    (void) fputc ('b', fp); /* header is 'b', height, width */
    (void) puti (bottom - top + 1, fp);
    (void) puti (right1 - left1, fp);

    /* get each line of text for the height of the box */
    for (line = top; line <= bottom; line++)
    {
	text1     = getli (line);
	leftpos   = calc_line_position3(text1, left1);
	rightpos  = calc_line_position2(text1, right1, &actual_width);
	tcount    = obj_count(text1) - 1;
 
	leftp  = &text1[leftpos];
	rightp = &text1[rightpos];

	if (text1[rightpos] == '\0')
	/* If line is shorter than right1, need to pad with spaces */
	    {
	    int nbytes = rightpos - leftpos + right1 - max(actual_width, left1);

            /* write the number of bytes to the file */
            (void) puti (nbytes, fp);

            /* write text to file */
	    for (tp = leftp; tp < rightp; tp++)
	        (void) puti (*tp, fp);

	    for (fill = max(actual_width, left1); fill < right1; fill++)
	        (void) puti ((SPACE | FIRST), fp);
            }
        else
	    {
            /* write the number of bytes to the file */
            (void) puti (rightpos - leftpos, fp);

            /* write text to file */
	    for (tp = leftp; tp < rightp; tp++)
	        (void) puti (*tp, fp);
            }

        /* if specified delete the text */
	if (flag && leftpos < tcount)
	{
		text1 = (ATTR *) i_delete ((short *)text1, leftpos,
				      min(rightpos-leftpos,tcount-leftpos));
		(void) putli (line, text1);
	}
	s_free ((char *)text1);
    }

    (void) putl (fileloc, fp);

    if ((fflush (fp) == EOF) || (feof (fp) || ferror (fp)))
	{
	if (g_diskfcn != NULL)
	    {
	    (*g_diskfcn)((g_argbits[g_keyvalue] & DELSTACK) ? delname : putname, "boxpick");
	    clearerr (fp);
	    (void) fseek (fp, fileloc, 0); 
	    goto retry;
	    }
	}
    }

/****************************************************************************/
/* opendelfile:  writes and opens up the # file                             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     delfp, delname, poundname                        */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     opendelfile is called from setfile when the filename is "#".         */
/*     It flushes the delete stack, and then runs the poundfile program     */
/*     as a filter to construct the pound file from the delete stack,       */
/*     discarding the filter's standard input and output.  Finally it       */
/*     does a setfile to the actual pound file, whose name was generated    */
/*     by uniqnam in openput.                                               */
/****************************************************************************/

void opendelfile (void)
    {
    char command [100 * MB_LEN_MAX]; /* filter command string */
    char *fcommand;

#ifdef DEBUG
    debug ("opendelfile called");
#endif

    (void) fflush (delfp);
    fcommand = Efixvars ("$SYS/bin/poundfile");
    (void) sprintf (command, "%s %s %s", fcommand, delname, poundname);
    s_free (fcommand);
    filter (command, "/dev/null", "/dev/null");

    if (setfile (poundname, TRUE, TRUE) == ERROR)
	fatal ("cannot open pound file '%s'", poundname);

    Edispname ("#");
    }

/****************************************************************************/
/* openput:  open putfp and delfp files.                                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     putname, delname, poundname, putfp, delfp        */
/*                                                                          */
/* globals changed:        putname, delname, poundname, putfp, delfp        */
/*                                                                          */
/* synopsis:                                                                */
/*     openput invokes uniqnam to get unique file names for the             */
/*     temporary files putname, delname, and poundname.  It also            */
/*     invokes Ermfile on each name so that the editor will remember        */
/*     to delete them during exit processing.  Finally it opens             */
/*     the delfp and putfp file pointers in the correct modes.              */
/*     openput issues a fatal error if the open files on either stack.      */
/****************************************************************************/

void openput (void)
    {
    putname   = s_string (uniqname (1));
    delname   = s_string (uniqname (1));
    poundname = s_string (uniqname (1));

    Ermfile (putname); /* delete files on exit */
    Ermfile (delname);
    Ermfile (poundname);

# ifdef DEBUG
    debug ("openput:  putname = '%s', delname = '%s'", putname, delname);
# endif

    if ((putfp = fopen (putname, "w+")) == NULL)
	{
	(void) printf ("Cannot open put file.  Check permissions on /tmp.\n");
	fatal ("Cannot open put file.  Check permissions on /tmp");
	}

    delfp = ffopen (delname, "w+", "openput");

    s_free (putname);
    }

/****************************************************************************/
/* putback:  puts data back in file from put or del stack                   */
/* NOTE that PUT can have a numeric argument                                */
/*                                                                          */
/* arguments:              int   argtype  - how called: NOARGS, EMPTYARG... */
/*                         char *strvalue - argument if called as STRINGARG */
/*                         int   numvalue - argument if called as NUMBERARG */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_argbits, delfp, putfp              */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     putback puts data back in the file, either from the delete stack     */
/*     (for undelete or undelete-copy operations) or from the put stack     */
/*     (for put or put-copy operations); and either popping the top         */
/*     element of the stack (for undelete or put operations) or leaving     */
/*     the stack unchanged (for undelete-copy or put-copy operations).      */
/*     It first decides which stack to use and whether it should be popped, */
/*     and, if the input argtype was NUMBERARG, how many times to copy the  */
/*     top stack frame into the file.  If the specified stack is empty,     */
/*     putback simply returns immediately with no error message.            */
/*                                                                          */
/*     For each iteration, it looks at the long integer immediately behind  */
/*     the current file pointer position to get the position of the start   */
/*     of the stack frame, and then looks at that file position for the     */
/*     type of the frame and its data.  Depending on whether the stack      */
/*     frame is lines, box, or text, putback invokes rstrlines, rstrbox, or */
/*     rstrtext to do the actual manipulation of the file for the restore.  */
/*     If the restore procedure fails, it stops iterating immediately, and  */
/*     putback does not pop the stack if it was planning to.                */
/*                                                                          */
/*     If the stack should be popped, it does a seek to the previous frame  */
/*     and leaves the stack file pointer positioned there.  Otherwise it    */
/*     seeks the file back to its original position.                        */
/****************************************************************************/

void putback (int argtype, char *strvalue, int numvalue)
    {
    int  i;          /* loop counter                    */
    int  count;      /* number of times to repeat       */
    int  popflag;    /* TRUE if stack should be popped  */

    FILE *fp;        /* file pointer (putfp or delfp)   */
    char type;       /* frame type                      */
    long fileloc;    /* current file location           */
    long frameloc;   /* starting location of frame      */

#ifdef DEBUG
    debug ("putback called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    popflag = (g_keyvalue == PUT_DOWN) || (g_keyvalue == RESTORE);
    fp = (g_argbits [g_keyvalue] & DELSTACK) ? delfp : putfp;

#ifdef DEBUG
    debug ("putback:  fp = %d, putfp = %d, delfp = %d", fp, putfp, delfp);
#endif

    fileloc = ftell (fp);
    if (fileloc == 0L) /* if no frames in the stack */
	return;

    count = (argtype == NUMBERARG) ? numvalue : 1;
    (void) fseek (fp, -(long) sizeof (long), 1);
    frameloc = getl (fp);

    for (i = count; i > 0; i--)
	{
	(void) fseek (fp, frameloc, 0);
	type = fgetc (fp);

#ifdef DEBUG
	debug ("putback:  type = %c", type);
#endif
	switch (type)
	    {
	    case 'L': /* lines from an indexed field */
	    case 'l': /* lines from a non-indexed field */

		if (rstrlines (fp, type, i) != ERROR)
		    continue;
		else
		    break;

	    case 'b': /* rectangular box of text */

		if (rstrbox (fp, i) != ERROR)
		    continue;
		else
		    break;

	    case 't': /* running text */

		if (rstrtext (fp, i) != ERROR)
		    continue;
	    }

            /* switch cases break on errors */
	    popflag = FALSE; 
	    break;
	} /* end for loop */

    if (popflag)
	fileloc = frameloc;

    (void) fseek (fp, fileloc, 0);
    }

/****************************************************************************/
/* rstrbox:  restores box from stack at cursor                              */
/*                                                                          */
/* arguments:              FILE *fp - pointer into put or del stack,        */
/*                                    positioned at start of frame to       */
/*                                    restore                               */
/*                                                                          */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     g_wp, g_brkflag                                  */
/*                                                                          */
/* globals changed:        g_brkflag                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     rstrbox restores the box pointed to by the given file pointer into   */
/*     the file at the current cursor position.  It first reads the height  */
/*     and width of the box out of the file and sets the g_brkflag global   */
/*     to FALSE so that the restore operation is not interrupted.           */
/*                                                                          */
/*     Then, for each line in the box, insert it into the file.  When done, */
/*     set g_brkflag back to TRUE, scroll the screen if necessary, and      */
/*     invoke dirtybox to let the display code know about the change.       */
/****************************************************************************/

int rstrbox (FILE *fp, int loopcount)
    {
    WSTRUCT *wp=g_wp;     /* current window                    */
    ATTR    *text2;       /* text from line                    */

    int wline;            /* 0 based window line               */
    int height;           /* number of lines in box            */
    int width;            /* number of columns in box          */
    int length;           /* max of field length and cursor position */
    int i;                /* looper through box characters     */
    int leftpos;          /* left position in text             */
    int nbytes;           /* number of bytes to be read        */

    height = geti(fp);
    width  = geti(fp);

#ifdef DEBUG
    debug ("rstrbox:  fp = 0%o, height = %d, width = %d, wline = %d",
	   fp, height, width, w_line (wp));
#endif

    length = datalength ();
    length = max (length, w_line (wp) + w_ftline (wp));

    if ((length + (loopcount * height)) >= MAXFLINE)
	{
	Eerror (M_EBOXTOOBIG, "The region is too big to fit here.");
	return (ERROR);
	}

    for (wline = w_line (wp); wline < w_line (wp) + height; wline++)
	{

        /* if line in window, get from cache */
	if (wline <= w_lrline (wp))
	    text2 = (ATTR *) i_string ((short *) w_text (wp, wline));
	else
	    text2 = readline (wline + w_ftline (wp), w_col(wp)+w_ftcol(wp));

        leftpos = calc_line_position3(text2, w_col(wp)+w_ftcol(wp));

	/* read first byte from the stack file.  It tells us how
	   many bytes must be read */

	nbytes = geti(fp);
	text2  = (ATTR *) i_insert ((short *)text2, leftpos, nbytes);

        /* write out the required number of bytes */
	for (i = leftpos; i < leftpos + nbytes; i++)
	    text2[i] = geti(fp);

	(void) putli (wline + w_ftline (wp), text2);
	s_free ((char *)text2);
	}
    return(1);
    }

/****************************************************************************/
/* rstrlines:  restores lines from stack at cursor                          */
/*                                                                          */
/* arguments:              FILE *fp  - pointer into del or put stack        */
/*                         char type - 'L' for complex data; 'l' for text   */
/*                                                                          */
/* return value:           int - RET_OK or ERROR if restore failed          */
/*                                                                          */
/* globals referenced:     g_wp, g_debugfp, g_wnode, g_recdirty, g_record   */
/*                                                                          */
/* globals changed:        g_recdirty, g_record                             */
/*                                                                          */
/* synopsis:                                                                */
/*     rstrlines is used to restore one or more lines from the current      */
/*     stack position into the file at the current cursor position.         */
/*     It expects the stack file pointer to be positioned at the start      */
/*     of a stack frame of the line type.  It reads the number of lines     */
/*     in the stack frame from the file, followed by the pick name.         */
/*                                                                          */
/*     If the input type argument indicates this is complex data, check     */
/*     that the file is indeed indexed on a gang with the same pick name    */
/*     and, if not, invoke Eerror to complain and then return error.        */
/*                                                                          */
/*     Otherwise, insert the right number of lines into the current window, */
/*     and loop through the lines of the stack frame, using s_read to get   */
/*     the line out of the stack.  If the data is complex, set the          */
/*     g_recdirty global true and use fixtree to replace each line of       */
/*     g_record in turn with the object read from the stack.  For simple    */
/*     data, use putli to put the line into the file.  If appropriate,      */
/*     invoke getgngs on each line.  When finished, return RET_OK.          */
/****************************************************************************/

int rstrlines (FILE *fp, char type, int loopcount)
    {
    POINTER *node;
    WSTRUCT *wp=g_wp;     /* current window       */
    ATTR    *unpdata;     /* unpacked data        */
    ATTR    *text3;
    ATTR    *otext;       /* old form of text */
    ATTR    *newtext;     /* new form of text */

    int line;             /* 0 based field line   */
    int wline;            /* 0 based window line  */
    int count;            /* number of lines      */
    int length;           /* max of field length and cursor position */

    char *ptext;          /* packed-up form of text */
    char *data;           /* data for line        */
    char *pickname;       /* pickname from frame  */

    count  = geti(fp);
    length = datalength ();
    length = max (length, w_line (wp) + w_ftline (wp));

    if ((length + (loopcount * count)) >= MAXFLINE)
	{
	Eerror (M_ETOOMANYLINES, "The specified text region is too large for the current field.");
	return (ERROR);
	}

    pickname = s_read (fp);

#ifdef DEBUG
    debug ("rstrlines: fp = 0%o, type = %c, count = %d, pickname = '%s'",
	   fp, type, count, pickname ? pickname : "");
#endif

    /* if the frame contains complex data and the current field is 
       a text field, or an indexed field of another type, complain. */

    if (type == 'L')
	if (( !getstat (w_flags (wp), INDEXED)) ||
	    ( !seq (pickname, w_pickname (wp))))
	    {
	    if (pickname)
		Eerror (M_EPUTLINES, "Do not put %s-type data in this field.",
		    pickname);
	    else
		Eerror (M_EPUTLINES, "Do not put %s-type data in this field.","");

	    s_free (pickname);
	    return (ERROR);
	    }

    s_free (pickname);
    insli (w_line (wp), count);
    wline = w_line (wp);
    line  = w_ftline (wp) + wline;

    while (count--)
	{
	data = s_read (fp);

#ifdef DEBUG
	debug ("  line %d at loc 0%o:\n", line, data);
	if (g_debugfp && data)
	    s_print (data);
#endif
	if (data != NULL)
	    {
	    if (type == 'L')  /* if complex data */
		{

		/* This code is an in-line imitation of part of writeline */
		if ((g_wnode == (char *) ERROR) || (g_wnode == NULL))
		    pfxnode (TRUE);

		node = idxnode (line, TRUE);

		/* because data goes into g_record, DO NOT s_free it! */
		g_record = fixtree (g_record, (char *) node, data);
		g_recdirty = TRUE;
		s_free ((char *) node);

		/* do getgngs while the data is handy */
		if (wline <= w_lrline (wp))
		    getgngs (wline, wline);

		/* If the helper wants Hmod, give it the chance to
		   clear the data for each field in the current gang
		   for which the DOUMOD bit in the form is turned on.
		   Restored data in non-Umod fields or not at this
		   zoom level will be retained even if Hmod rejects
		   the data for some of the fields in the gang.       */

		if (g_helper && g_hooks [UMOD])
		    thrugngs

			if ( !getstat (w_flags (g_wp), DOUMOD))
			    continue;

			text3 = getli (line);

			/* Since we just did an insli, in this context
			   the old value for this field must be blank.  */

			otext = (ATTR *) i_string (NULL);
			ptext = packup (text3);

			if (Hmod (w_name (g_wp), line, "", ptext))
			    {
			    /* restore the null unless Hmod updated it
			       with Eputtext */

			    newtext = getli (line);
			    if (i_seq ((short *)newtext, (short *)text3))
				puttext (line, otext);
			    s_free ((char *)newtext);
			    }
			s_free (ptext);
			s_free ((char *)otext);
			s_free ((char *)text3);

		    endthrugngs

		}
	    else
		{
		unpdata = unpackup (data, 0);
		putli (line, unpdata);
		s_free ((char *)unpdata);
		s_free (data);
		}
	    }

	wline++;
	line++;
	}
    return (RET_OK);
    }

/****************************************************************************/
/* rstrtext:  restores text from stack at cursor                            */
/*                                                                          */
/* arguments:              FILE *fp - pointer into del or put stack         */
/*                                                                          */
/* return value:           int - RET_OK or ERROR if restore failed          */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     rstrtext restores a text region from the given stack into the        */
/*     current cursor position.  If the current field is indexed, it        */
/*     returns ERROR.  It expects the given stack pointer to be             */
/*     positioned at the start of a text region.   It reads the number      */
/*     of lines in the region from the file.                                */
/*                                                                          */
/*     The insertion is accomplished as follows:  the current line is       */
/*     split at the cursor position.  The first line from the region        */
/*     is concatenated onto the part of the line left of the cursor.        */
/*     Intervening lines are simply inserted in the file.  The part         */
/*     of the original current line to the right of the cursor is           */
/*     concatenated onto the last line of the region.  When all lines       */
/*     have been processed, rstrtext returns RET_OK.                        */
/*                                                                          */
/*     Note that because of the way the loop is set up, a text region       */
/*     with only one line in it is simply inserted at the cursor            */
/*     position, and the number of lines in the file does not change.       */
/****************************************************************************/

int rstrtext (FILE *fp, int loopcount)
    {
    WSTRUCT *wp=g_wp;  /* current window        */
    ATTR *text4;       /* text from line        */
    ATTR *toptext;     /* text of top line      */
    ATTR *bottext;     /* text of bottom line   */

    int line;          /* zero based field line */
    int count;         /* number of lines       */
    int pos;           /* position in text      */
    int botline;       /* bottom line (0 based) */
    int length;        /* max of field length and cursor position */

#ifdef DEBUG
    debug ("rstrtext:  fp = 0%o", fp);
#endif

    if (getstat (w_flags (wp), INDEXED))
	{
	Eerror (M_EPUTTEXT, "Do not use the PUT command with a text region in a list field.");
	return (ERROR);
	}

    count   = geti (fp);
    length  = datalength ();
    length  = max (length, w_line (wp) + w_ftline (wp));
    line    = w_line (wp) + w_ftline (wp);
    botline = line + count - 1;

    toptext = (ATTR *) i_string ((short *) w_text (wp, w_line (wp)));
    pos     = calc_line_position(toptext, w_col(wp) + w_ftcol(wp));

    bottext = (ATTR *) i_string ((short *)&toptext[pos]);
    toptext = (ATTR *) i_realloc ((short *)toptext, pos);
    text4   = (ATTR *) s_read (fp);
    toptext = (ATTR *) i_cat ((short *)toptext, (short *)text4);

    (void) putli (line, toptext);
    s_free ((char *)toptext);

    Einsline ("", ++line, count - 1);

    while (line < botline)
	{
	text4 = (ATTR *) s_read (fp);
	(void) putli (line++, text4);
	s_free ((char *)text4);
	}

    text4   = (ATTR *) s_read (fp);
    bottext = (ATTR *) i_cat ((short *)text4, (short *)bottext);
    (void) putli (line, bottext);
    s_free ((char *)bottext);
    return (RET_OK);
    }

/****************************************************************************/
/* savelines:  save lines on put or del stack                               */
/*                                                                          */
/* arguments:              int index - 0 based window index of starting     */
/*                                     line                                 */
/*                         int count - number of lines to save              */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_argbits, g_keyvalue, delfp, putfp,       */
/*                         g_wnode                                          */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     savelines writes a new stack frame into either the delete stack or   */
/*     the put stack, holding the given number of lines from the given      */
/*     file position.  It refers to the g_argbits entry corresponding to    */
/*     g_keyvalue to determine in which stack to put the picked lines.      */
/*     If the DELSTACK bit is set for this key in g_argbits, the lines go   */
/*     into the delete stack; otherwise into the put stack.                 */
/*                                                                          */
/*     If the current field is a simple text field, the stack frame has     */
/*     the following structure:                                             */
/*         'l' - to indicate a line frame of the simple type                */
/*          n  - the number of lines in the frame                           */
/*          <n lines of data>                                               */
/*          floc - the position in the stack file of the start of this      */
/*                 frame                                                    */
/*                                                                          */
/*     If the current field is indexed, the stack frame structure is:       */
/*         'L'  - to indicate a line frame of the complex type              */
/*          n   - the number of lines in the frame                          */
/*          pn  - the pick name for this (possibly ganged) field            */
/*          <n lines of data>                                               */
/*          floc - the position in the stack file of the start of this      */
/*                 frame                                                    */
/*                                                                          */
/*     If the attempt to write into the stack file fails for any reason,    */
/*     savelines issues a fatal error.                                      */
/****************************************************************************/

void savelines (int index1, int count)
    {
    WSTRUCT *wp=g_wp;        /* current window          */
    POINTER *data;           /* data for line           */
    FILE *fp;                /* putfp or delfp          */

    int line;                /* 0 based field line      */
    int savecount;           /* used to save count      */
    char *pickname;          /* pickname for field      */
    long fileloc;            /* file location of frame  */

    fp = (g_argbits [g_keyvalue] & DELSTACK) ? delfp : putfp;

#ifdef DEBUG
    debug ("savelines:  index1 = %d, count = %d, w_name = '%s'",
	   index1, count, w_name (wp));
    debug ("savelines:  fp = %d, putfp = %d, delfp = %d", fp, putfp, delfp);
#endif

    fileloc = ftell (fp);
    putgng (index1, min (index1 + count, w_lrline (wp)));
    pickname = NULL;

    savecount = count;

retry: /* disk full retry point */

    if (getstat (w_flags (wp), INDEXED))
	{
	(void) fputc ('L', fp);
	pickname = makestring (w_pickname (wp)); /* let NULL stay as NULL */
	}
    else
	(void) fputc ('l', fp);

    puti (count, fp);
    if (s_write (pickname, fp) == ERROR)
	fatal ("savelines: s_write failure");
    line = w_line (wp) + w_ftline (wp);

    while (count--)
	{

	/* getdata copied in line */
	if (g_wnode == (char *) ERROR) /* if g_wnode invalid  */
	    pfxnode (FALSE);              /* set up if possible  */

	if (g_wnode == NULL) /* if window has no ulhc in data */
	    data = NULL;
	else

	    /* if at top level of file, do an idxnode (line, YES) 
	       to prevent getting a partial record */

	    data = idxnode (line, g_wnode == (char *) sf_records (g_sfp));

	if (s_write ((char *)data, (FILE *)fp) == ERROR)
	    fatal ("savelines: s_write failure");
	line++;
	}

    s_free (pickname);
    (void) putl (fileloc, fp);

    if ((fflush (fp) == EOF) || (feof (fp) || ferror (fp)))
	{
	if (g_diskfcn != NULL)
	    {
	    (*g_diskfcn)((g_argbits [g_keyvalue] & DELSTACK) ? delname : putname, "savelines");
	    clearerr (fp);
	    (void) fseek (fp, fileloc, 0); 
	    count = savecount;
	    goto retry;
	    }
	}
    }

/****************************************************************************/
/* textpick:  handles PICK and CLOSE operations on text regions             */
/* Assumes that the cursor is at topline, topcol.                           */
/*                                                                          */
/* arguments:              int topline, topcol, botline, botcol             */
/*                                  - start and end of text region in       */
/*                                    0-based field coords                  */
/*                         int flag - whether to delete region              */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_argbits, g_keyvalue                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     textpick writes a new stack frame into either the delete stack or    */
/*     the put stack, holding the given text region.  It refers to the      */
/*     g_argbits entry corresponding to g_keyvalue to determine in which    */
/*     stack to put the picked region.  If the DELSTACK bit is set for      */
/*     this key in g_argbits, the region goes into the delete stack;        */
/*     otherwise into the put stack.  The stack frame written consists      */
/*     of a 't' to indicate a text region, followed by the number of        */
/*     lines in the region, followed by the actual data, followed by        */
/*     the position in the stack file of the start of this frame.           */
/*                                                                          */
/*     The data for the region consists of that portion of the top line     */
/*     to the right of the top column, followed by any intervening lines,   */
/*     followed by that portion of the bottom line to the left of the       */
/*     bottom column.                                                       */
/*                                                                          */
/*     If the input flag is set, the text region is deleted as it is        */
/*     being picked.                                                        */
/****************************************************************************/

void textpick (int topline, int topcol, int botline, int botcol, int flag)
    {
    WSTRUCT *wp=g_wp;   /* current window                         */
    FILE *fp;           /* file pointer (putfp or delfp)          */
    ATTR *text5;        /* text from line                         */
    ATTR *cp;           /* used for s_write calls                 */

    int  line;          /* zero based field line                  */
    int  pos;           /* position in text line                  */
    int  count;         /* delete count                           */
    long fileloc;       /* file location of frame                 */

#ifdef DEBUG
    debug ("textpick:  tl = %d, tc = %d, bl = %d, bc = %d, flag = %d",
	   topline, topcol, botline, botcol, flag);
#endif

    fp      = (g_argbits [g_keyvalue] & DELSTACK) ? delfp : putfp;
    fileloc = ftell (fp);
    flush_window ();

retry: /* disk full retry point */

    (void) fputc ('t', fp); /* header is 't', height */
    (void) puti (botline - topline + 1, fp);

    /* do the first line */
    text5 = (ATTR *) w_text (wp, w_line (wp)); 
    pos   = calc_line_position(text5, w_col(wp) + w_ftcol(wp));

    cp = (ATTR *) i_string ((short *)&text5[pos]);
    if (s_write ((char *)cp, fp) == ERROR)
	fatal ("textpick: s_write failure");
    s_free ((char *)cp);

    /* do the middle lines */
    for (line = topline + 1; line < botline; line++) 
	{
	text5 = getli (line);
	if (s_write ((char *)text5, fp) == ERROR)
	    fatal ("textpick: s_write failure");
	s_free ((char *)text5);
	}


    /* do the last line */
    text5 = getli(botline); 
    pos   = calc_line_position(text5, botcol);
    count = min ((obj_count (text5) - 1), pos);

    /* save end of line for later if deleting */
    if (flag)
	{
	cp = (ATTR *) i_string ((short *)text5);
	if (count > 0)
	    cp = (ATTR *) i_delete ((short *)cp, 0, count);
	}

    text5 = (ATTR *) i_realloc ((short *)text5, count);
    if (s_write ((char *)text5, fp) == ERROR)
	fatal ("textpick3: s_write failure");
    s_free ((char *)text5);
    (void) putl (fileloc, fp);

    if ((fflush (fp) == EOF) || (feof (fp) || ferror (fp)))
	{
	if (g_diskfcn != NULL)
	    {
	    (*g_diskfcn)((g_argbits [g_keyvalue] & DELSTACK) ? delname : putname, "textpick");
	    clearerr (fp);
	    (void) fseek (fp, fileloc, 0);
	    goto retry;
	    }
	}

    if ( !flag)
	return;

    /* delete region if flag is TRUE */
    pos   = calc_line_position(cp, botcol);
    if (pos > (obj_count (cp) - 1)) 
	cp = (ATTR *) i_realloc ((short *)cp, pos);

    text5 = (ATTR *) i_string ((short *) w_text (wp, w_line (wp)));
    pos   = calc_line_position(text5, topcol);
    text5 = (ATTR *) i_realloc ((short *)text5, pos);
    text5 = (ATTR *) i_cat ((short *)text5, (short *)cp);

    (void) putli (topline, text5);
    s_free ((char *)text5);
    Edelline ("", topline + 1, botline - topline);
    }




