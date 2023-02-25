static char sccsid[] = "@(#)70	1.11  src/tenplus/e2/common/text.c, tenplus, tenplus411, GOLD410 3/3/94 19:00:51";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	text, plop, isclosepunct, dowrap 
 *              lrscroll, disp_len, coverup
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
 
#include <langinfo.h>
#include <locale.h>

#include "def.h"
#include "INeditor_msg.h"

void text (void)
    {
    plop();
    }

void plop(void)
    {

    WSTRUCT *wp = g_wp;               /* local copy of g_wp                */
    ATTR    *line;                    /* current line                      */

    int curcol;                       /* original cursor position          */
    int pos;                          /* position in text line             */
    int ndel;                         /* number to delete                  */
    int nins;                         /* number to insert                  */
    int mov_col;                      /* number of columns to shift        */
    int i;                            /* loop counter                      */
    int font = 0;                     /* current font status               */
    int actual_col;                   /* as returned by calc_line_position */

    wchar_t wc;                       /* g_textvalue as a wide char        */
    int ins_width, del_width;

    if (w_line(wp) + w_ftline(wp) >= MAXFLINE)
	{
	Eerror (M_ECANTINSERT,
	    "Do not insert text beyond the field size limit.");
	return;
	}

    line   = (ATTR *) w_text(wp, w_line(wp));
    pos    = calc_line_position2(line, w_col(wp) + w_ftcol(wp), &actual_col);
    curcol = actual_col - w_ftcol(wp);

    /* action depends on the current mode */
    if (g_imodeflag)
        {
     
	del_width = 0;
        /* Insert mode - don't overstrike characters in last column */
	if (coverup(line))
	    {
	    Ebell();
	    return;
	    }
	}
    else
	{

        /* Overwrite mode - determine the number of bytes in the
           current character and then delete it. There is always at least 1 */
	 
	ndel = 1;            
	if (MultibyteCodeSet)
	   {
	   i = pos + 1;
	   while (!firstbyte(line[i]))
               {
               ++i;
	       ++ndel;
               }
	   }

	del_width = char_width(&line[pos]);
	line = (ATTR *) i_delete ((short *)line, pos, ndel);
	}

    /* find out how many bytes need to be inserted and determine the
       font to be used */

    if (MultibyteCodeSet)
	{
	nins = mbtowc(&wc, (char *)g_textvalue, (size_t)MB_CUR_MAX);
	/* insert at least 1 char */
	if (nins < 1) nins = 1;
	ins_width = wc_text_width(wc);
	if (((g_font == W_FONT) && iswalnum(wc)) || (g_font == C_FONT))
	       font = UNDERLINE;
	}
    else
	{
	nins = ins_width = 1;
	if (((g_font == W_FONT) && isalnum(g_textvalue[0])) ||
	     (g_font == C_FONT))
		font = UNDERLINE;
	}

    /* insert the new bytes at the current location */
    line = (ATTR *) i_insert((short *)line, pos, nins);

    if (del_width > ins_width)
	{
	int endpos = obj_count(line) - 1;
	int num = del_width - ins_width;

        line = (ATTR *) i_insert((short *)line, endpos, num);
	i_smear(SPACE|FIRST, num, &line[endpos]);
	}
	
    /* Set the correct font. Only single byte characters are
       modified by G_FONT (graphic font) */

    if ((g_font == G_FONT) && (nins == 1) && (g_textvalue[0] >= '@'))
	{
	line[pos] = control(g_textvalue[0]) | GRAPHIC | FIRST;
	++pos;
	}
    else
	{
	for (i = 0; i < nins; i++)
	    {
	    if (i)
		line[pos] = g_textvalue[i] | font;
	    else
		line[pos] = g_textvalue[i] | font | FIRST;

	    ++pos;
	    }
	}

    /* update the text line in cache */
    wp->w__cache[w_line(wp)] = (POINTER) line;

    /* update the screen and the cursor column */
    sendbox(w_line(wp), w_line(wp), curcol, w_lrcol(wp));
    wp->w__col = calc_column(line, pos) - w_ftcol(wp);

    /* special case, the last column in fields without margins */
    mov_col = 0;
    if (!w_rmar(wp) && (w_col(wp) > w_lrcol(wp)))
	{
	if (nins > 1 && !iswspace(wc) )
	    mov_col = ins_width - 1;
	--wp->w__col;
	}

    /* special case where we've reached the last column */
    /* NAMESIZE is length used by movrt() for RIGHT */
    if ((w_col(wp) + w_ftcol(wp)) >= NAMESIZE)
        --wp->w__col;

    w_setmod (wp, w_line(wp));

    /* wrap only in insert mode or when overwriting past the right margin */
    if (g_imodeflag || (w_rmar(wp) && (w_col(wp) > w_rmar(wp))))
    	dowrap();

    lrscroll();
    wp->w__col -= mov_col;
    }

/* isclosepunct: check for closing punction symbols.

   This function assumes that 'x' points to a 'first' byte of
   a character sequence */

int isclosepunct (ATTR *x)
{

    static int kanji = -1;           /* flag indicating code set located */
    char *lp;

    /* if the local code set is not known get it from the locale.
       Set the 'kanji' flag to 1 if that's the current code set. */

    if (kanji == -1)
	{
	kanji = 0;
	lp = nl_langinfo(CODESET);
	if (strcmp(lp, "IBM-932") == 0)
	    kanji = 1;
	}

	switch (de_attr(*x))
	    {
	    case '\'':
	    case '"':
	    case '.':
	    case ',':
	    case ';':
	    case ')':
	    case ']':
	    case '}':
		return (TRUE);

	    /* japanese '.' */
	    case 0xa1:
		if (kanji)
		    return(TRUE);
		else
		    return(FALSE);

	    /* japanese ',' */
	    case 0xa4:
		if (kanji)
		    return(TRUE);
		else
		    return(FALSE);

	    default:
		return (FALSE);
	    }
}

void dowrap(void)
    {
    WSTRUCT *wp = g_wp;                    /* local copy of g_wp           */
    ATTR *cp;                              /* counter                      */
    ATTR *from;                            /* line being wrapped           */
    ATTR *to;                              /* line to wrap to              */
    ATTR *wrap;                            /* what to wrap                 */
    ATTR *fptr;                            /* ptr to leftmost white space  */

    int f_line = w_line(wp);               /* from line number             */
    int t_line;                            /* to line number               */
    int rmarpos;                           /* right margin position        */
    int lmarpos;                           /* left margin position         */
    int fromcol;                           
    int wraplen;                           /* how much to wrap (bytes)     */
    int wrapwidth;                         /* how much to wrap (columns)   */
    int tolen;                             /* wrapline length after wrap   */
    int amt, dist;
    int wrap_flag = g_wmodeflag;           /* CHAR or WORD wrap mode       */
    int curcol = w_col(wp) + w_ftcol(wp);  /* current cursor column        */
    int rtend  = w_ftcol(wp) + w_lrcol(wp);/* right end of screen          */
    int to_width=0;
    int spacelen=0;
    int nbr_fill_blanks; 

    extern int disp_len(ATTR *, int *);

#ifdef DEBUG
debug("entered dowrap: from line = %d, curcol = %d, rtend = %d", 
                            f_line, curcol, rtend);
#endif

    /* can we do any wrapping anyway? */
    if (!w_iswrap(wp))
	return;

    /* determine the number of columns to be wrapped (if any) */
    from      =  (ATTR *) w_text(wp, f_line);
    wrapwidth =  disp_len(from, &wraplen) - w_rmar(wp);
    if (wrapwidth <= 0)
	return;

    /* determine the number of bytes to be wrapped */
    rmarpos =  calc_line_position(from, w_rmar(wp));
    wraplen -= rmarpos;

    /* determine the type of wrapping required */
    if (wrap_flag == CHARWRAP)
	{

        /* character wrapping - determine if punctuation should be considered */
	while (isclosepunct(&from[rmarpos]))
	    if (g_fmtmode == WRAPPUNCT)
		{
                wrapwidth += char_width(&from[rmarpos]);
		--rmarpos;
		++wraplen;
                while (!firstbyte(from[rmarpos]))
                    {
		    --rmarpos;
		    ++wraplen;
                    }
		}
	    else
		{
                wrapwidth -= char_width(&from[rmarpos]);
		++rmarpos;
		--wraplen;
                while (!firstbyte(from[rmarpos]))
                    {
		    ++rmarpos;
		    --wraplen;
                    }
		}
	}
    else
        {

        /* word wrapping - determine where the current word ends */
	fptr = from + rmarpos;
        amt = 0;
	for (;;)
	    {
	    if ((isattrspace(fptr)) || (fptr <= from))
		  break;

            amt += char_width(fptr);
	    backupattr(&fptr);
	    }

	if (isattrspace(fptr))
	    {
	    spacelen  = attrlen(fptr);
	    dist      =  from + rmarpos - fptr;
	    rmarpos   -= dist;
	    wraplen   += dist - spacelen;
	    wrapwidth += amt - char_width(fptr);
	    }
        else
	    {
            /* Full line of characters so don't wrap */
	    /* If at end of line, decrement w_col to keep cursor on last char */
	    if (w_col(wp) > w_lrcol(wp))
		--wp->w__col;
	    return;
	    }
        }

    /* anything to be wrapped? */
    if (wrapwidth <= 0)
	return;

    /* grab what's to be moved, then blank-fill the old line */
    cp      = from + rmarpos;
    wrap    = (ATTR *) s_alloc(wraplen + 1, T_ATTR, NULL);
    fromcol = calc_column(from, rmarpos);
    fromcol += spacelen;
    (void) i_strncpy ((short *)wrap, (short *)cp + spacelen, wraplen);

    /* blank fill the old line. Make sure there is enough space 
       for all the blanks up to the right side of the window. */
    *cp = (ATTR)0;
    nbr_fill_blanks = max(0, w_lrcol(wp) + 1 - attr_text_width(from));
    from = (ATTR *) i_realloc((short *)from, rmarpos + nbr_fill_blanks); 
    i_smear ((short) (SPACE | FIRST), nbr_fill_blanks, 
             (short *)(from + rmarpos));
    (wp->w__cache)[f_line] = from;

    /* where do you go? */
    if (f_line >= w_lrline(wp))
	{
	amt = w_lrline(wp) - (w_lrline (wp)/2) + 1;

	/* special case for two line fields to scroll rather than page */
	if (w_lrline(wp) == 1)
	    amt = 1;

	movdn (amt);
	wp->w__line -= amt;
	}

    /* look at the line to be wrapped onto */
    t_line  = w_line(wp) + 1;
    to      = (ATTR *) w_text (wp, t_line);
    rmarpos = calc_line_position(to, w_rmar(wp));
    lmarpos = calc_line_position(to, w_lmar(wp));

    /* open a new line if there isn't enough room, then 
       wrap text to the left margin of the next line */

    if ((disp_len(to, &amt) + wrapwidth) > w_rmar(wp))
	{
	Einsline ("", w_ftline(wp) + t_line, 1);
	to = (ATTR *) s_alloc (lmarpos + 1, T_ATTR, NULL);
	i_smear ((short)(SPACE | FIRST), lmarpos, (short *)to);
	}
    else
        if (wrap_flag == WORDWRAP)
	    {
	    to = (ATTR *) i_insert((short *)to, (int) lmarpos, 1);
	    to[lmarpos] = (SPACE | FIRST);
	    }

    to = (ATTR *) i_insert((short *)to, (int) lmarpos, wraplen);
    (void) i_strncpy((short *)to+lmarpos, (short *)wrap, wraplen);
    s_free ((char *)wrap);

    /* make the lines the right size, blank-padded. Note that SPACE is a
       single display width so this is OK.  */

    tolen = (obj_count (to) - 1);
    to_width = attr_text_width(to);
    to    = (ATTR *) i_realloc ((short *)to, max(rtend + 1 - to_width,0) + tolen);
    i_smear ((short) (SPACE | FIRST), max(rtend + 1 - to_width, 0), (short *)to + tolen);
    wp->w__cache[t_line] = (POINTER) to;
    w_setmod (wp, t_line);
    sendbox (max(0, w_line(wp)), min(w_line(wp)+1,
				 w_lrline(wp)), 0, w_lrcol(wp));

    /* re-position the cursor if necessary */
    if (curcol >= fromcol)
	{
	++wp->w__line;
	wp->w__col = (curcol - fromcol)  + (w_lmar(wp) - w_ftcol(wp));
	}
    else if (w_lrline(wp) == 0)
	{
	++wp->w__line;
	movup (1);
	}

#ifdef DEBUG
debug("dowrap: curcol  = %d, fromcol = %d, lmar = %d, rmar = %d, w_col = %d",
	curcol, fromcol, w_lmar(wp), w_rmar(wp), wp->w__col);
#endif

    }

void lrscroll(void)
    {
    WSTRUCT *wp = g_wp;          /* local copy of g_wp */

    int colshift = (w_tbcol(wp) - w_ttcol(wp))/3;

    if (colshift < 1)
	colshift = 1;

    while ((w_col(wp) < 0) || (w_col(wp) > w_lrcol(wp))) {
      while (w_col(wp) > w_lrcol(wp))
    	wp->w__col -= movrt(colshift);
    
      while (w_col(wp) < 0)
        wp->w__col += movlf((wp->w__col + w_ftcol(wp) > w_lrcol(wp)) ?
                            colshift : w_ftcol(wp));
    }

#ifdef DEBUG
debug ("lrscroll: colshift = %d, w_line(wp) = %d, w_col(wp) = %d",
	colshift, w_line(wp), w_col(wp));
#endif
    }

int disp_len(ATTR *line, int *wamt)
    {

    int  end, swidth;
    char *string;
  
#ifdef DEBUG
debug ("entered disp_len");
#endif

    /* find the first non white space character */
    string = a2string(line, i_strlen((short *)line));
    for (end = (obj_count(line)-2); end >= 0; backupattr_idx(line, &end))
        {

        /* found it! */
	if (line[end] && (!isattrspace(&line[end])))
	    break;

        /* null terminate as we go */
        string[end] = '\0';
        }

    /* determine the length and display width of the string with
       the white space characters removed */

    *wamt  = strlen(string);
    swidth = text_width(string);
    s_free(string);

#ifdef DEBUG
debug ("disp_len: display width = %d, string length = %d\n", swidth, *wamt);
#endif

    return(swidth);
    }

int coverup (ATTR *line)
    {

    WSTRUCT *wp = g_wp;         /* local copy of g_wp */

    int toright;                /* to right of inserted character */
    int overlap;                /* portion of new character off screen */
    int i;                      /* loop counter */
    int disp_width;             /* display width of g_textvalue */
    int pos;

    disp_width = text_width(g_textvalue);
    toright    = w_lrcol(wp) - w_col(wp) + 1;
    overlap    = disp_width - toright;

    if (disp_width > (w_lrcol(wp) - w_col(wp) + 1))
	return(TRUE);

    if (overlap < 0)
	return(FALSE);

    /* is the character just overwriting blanks? */
    pos = calc_line_position(line, w_col(wp)+w_ftcol(wp));
    for (i = 0; i < (disp_width - overlap); skipattr_idx(&line[pos], &i))
	{
	if (!isattrspace(&line[pos+i]))
	    return(TRUE);
	}

    return(FALSE);
    }
