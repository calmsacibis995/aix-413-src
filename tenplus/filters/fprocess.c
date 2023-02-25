#if !defined(lint)
static char sccsid[] = "@(#)71	1.5  src/tenplus/filters/fprocess.c, tenplus, tenplus411, GOLD410 7/18/91 12:38:56";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fill, just, printline
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/* =======================================================================
 *
 *  fprocess.c      -   Routines to process the parsed tree before it is
 *                      output, for format.
 *
 * =======================================================================
 */

#include "fdefines.h"
#include "fexterns.h"


#ifdef DEBUG
void printline(struct line * );
#endif

/* ======================== fill() ======================================
 *  The filler is handed a tree of nodes (hopefully with no intervening
 *  commands (or meaningful ones for inline e.g. font changes).
 * Use the right margin for fill calculations, not rmarg-lmarg
 *
 * =======================================================================
 */

 struct text *
fill( struct text *t )
{
 struct line * l, * templ ;
 struct word * prevw ;  /* ??? w not used */

    debug1("\n fill(): entered; l_marg=<%d>", l_marg );
    if (t->t_tail != NULL)
	t->t_tail->t_nextline = NULL ; /* to assure loop termination */

    for ( l = t->t_head ; l ; l = l->t_nextline ) /* thru tree */
    {
	debugline( l ) ;
	if ( l->t_flags & L_NO_KCUF )
	    break ;     /* don't mess w/current line */
	if ( l->t_flags & ( L_DOT_CMD  | L_EMPTY ) ) /* if either flag is up */
	    continue ;
	debug1("\n nextline is %s ", l->t_nextline == NULL ?  "NULL" :
				     l->t_nextline->t_flags & L_EMPTY ?
					"empty" : "not-empty" ) ;
	if (l->t_lastword != NULL)
	    l->t_lastword->t_next = NULL ;   /* to assure loop termination */
	if ( (    l->t_nextline != NULL ) &&            /* first  test */
	     ( !( l->t_nextline->t_flags & L_EMPTY )) &&/* second test */
		  l->t_nextline->t_lastword != NULL )   /* third test  */
	    l->t_nextline->t_lastword->t_next = NULL ;  /* to assure loop termination */

	/* three choices: line is short, long, ok */
	if ( l->t_length + l->t_padding - l->t_next->t_padding  < r_marg )
	/* if ( l->t_length + l->t_padding + FUZZ < r_marg )    */
	{   /* short: needs to be filled, if possible */
	    /* let's see if we can bring the first word from the next line up */

	    debug0("\nline is Short:") ;
	    while ( ( l->t_nextline != NULL ) &&    /* first  test */
		    /* second test */
		    (!( l->t_nextline->t_flags &
			/* ( L_DOT_CMD | L_NO_KCUF | L_EMPTY )) ) &&    */
			( L_DOT_CMD | L_EMPTY )) ) &&
		    ( l->t_nextline->t_next != NULL ) &&
		    (  l->t_nextline->t_next->t_length
		     + l->t_nextline->t_next->t_padding
		     + l->t_padding
		     + l->t_length
		     - l->t_next->t_padding <= r_marg ) /*subtr 1st word pad*/
		  )
	    {
		if ( l->t_lastword->t_cflags  & W_C_COMMENT )
		    break ; /* don't try to put words behind the comment */
		debug1("\n  in while loop; moving word <%s> up ",
			    l->t_nextline->t_next->t_offset ) ;
		/* it is possible to move it up */
		/* so do it and make sure all relevant counts and links are updated */
		/* update current line (expanded) */
		l->t_lastword->t_next    = l->t_nextline->t_next ; /* up */
		l->t_lastword            = l->t_nextline->t_next ; /* update */
		l->t_length             += l->t_nextline->t_next->t_length
					 + l->t_nextline->t_next->t_padding ;
		l->t_wordcount ++ ;

		/* update next line (decreased) */
		l->t_nextline->t_length -= l->t_nextline->t_next->t_length
					 + l->t_nextline->t_next->t_padding ;
		l->t_nextline->t_next    = l->t_nextline->t_next->t_next ; /* done last */
		l->t_nextline->t_wordcount -- ;
		/* now safe to break the final link */
		l->t_lastword->t_next = NULL ;

		/* now see if that was the last word on the line */
		if ( l->t_nextline->t_next == NULL )
		{   /* yes, so free the line node; nothing left in it */
		    debug0(" freeing Nextline");
		    templ = l->t_nextline ;
		    /* but first we have to move the L_NO_KCUF flag, if any
		     *  up to the prev line so that we have at least one
		     *  line in the tree, in this case.
		     */
		    if ( templ->t_flags & L_NO_KCUF ) /* if nextln has it.. */
			l->t_flags |= L_NO_KCUF ;     /* .. move it up ## */
		    l->t_nextline = templ->t_nextline ;
		    free( (char *) templ ) ;
		    t->t_nrlines -- ;
		}
	    }
	    /*  l->t_lastword = NULL ;  */ /* wrapup */ /* ???? */

	}
	else if ( l->t_length + l->t_padding - l->t_next->t_padding > r_marg)
	{   /* long: needs to be broken, if possible */
	    debug0("\nline is Long:") ;
	    /* line needs to be broken at the right spot; bit more difficult */
	    while (l->t_length + l->t_padding - l->t_next->t_padding > r_marg)
	    {
		/* don't try to put words behind a comment or move 1 word down */
		if ( ( l->t_wordcount < 2 ) ||
		     ( l->t_lastword->t_cflags  & W_C_COMMENT ) )
		    break ;
		if ( ( l->t_nextline == NULL            ) ||
		     ( l->t_nextline->t_flags &
			/* ( L_DOT_CMD | L_EMPTY | L_NO_KCUF ) )*/
			( L_DOT_CMD | L_EMPTY ) )
		     /*(!( l->t_nextline->t_flags & ( L_DOT_CMD | L_EMPTY )) )  */
		    /*  ( l->t_nextline->t_flags & L_EMPTY )  */
		   )
		{
		    /* no line below to push onto */
		    /* don't forget to init all fields after alloc */
		    debug0(" no line below; getting a Line node;" ) ;
		    templ = getAnode( line ) ; /* alloc a new line */
		    debug1(" head %c= tail ", (t->t_head == t->t_tail) ? '=' : '!' );
		    /* set the def pad according to nrlines or line below */
		    templ->t_padding  = ( ( t->t_head == t->t_tail )    ||
					  /* ( t->t_nrlines < 3 ) and nextln is empty */
					  ( ( t->t_nrlines < 3 )  && /* and nextln is empty */
					    ( ( !t->t_head->t_nextline ) ||
					      ( t->t_head->t_nextline->t_flags & L_EMPTY )
					    ))
					)
					? l_marg /* lonely line: use default */
					: l->t_padding ; /* from line above */
		    t->t_nrlines++ ;
		    templ->t_flags    = l->t_flags ; /* ??? bad */
		    templ->t_length = templ->t_wordcount /* = templ->t_justablew */ = 0 ;
		    templ->t_nextline = l->t_nextline ; /* ## */
		    templ->t_next = NULL ;
		    /* l->t_nextline = templ ;*/
		    /* t->t_tail         = templ ;*/ /* update tree pointers also */
		    templ->t_nextline = l->t_nextline ; /* generalized */
		    /* tree's tail should still be pointing to the right spot? */
		    /* NO! this is only true if l->t_nextline != NULL */
		    if ( l->t_nextline == NULL ) /* or if (l == t->t_tail) */
			t->t_tail = templ ; /* if the tail pointed to l, update */
		    l->t_nextline = templ ;  /* update line above's pointers */
		    /* since we know that at least the first word on line 'l'
		     *  is going to be moved down (otherwise we wouldn't
		     *  have allocated the new node), update the new line's
		     *  lastword field to point to the lastword in 'l' so that
		     *  the next time this happens, we won't get trash
		     * This precaution is not necessary for the l->t_next
		     */
		    templ->t_lastword = l->t_lastword ;
		    /* templ->t_lastword->t_next gets set to NULL, below in
		     *  l->t_lastword->t_next = l->t_nextline->t_next;
		     *  as templ->t_next was set to NULL, above.
		     */
		}
		/* take off last word */
		/* attach first word on next line at end of last wrd on current line */

		debug1("\n  in while loop; moving last word down<%s>",
			    l->t_lastword->t_offset);
		l->t_lastword->t_next = l->t_nextline->t_next; /* NULL if new */
		l->t_nextline->t_next = l->t_lastword ;
		prevw = l->t_next ; /* first word */
		while ( prevw->t_next != l->t_lastword )
		    prevw = prevw->t_next ; /* skip till we find lastword  */
		l->t_lastword = prevw ;
		l->t_lastword->t_next = NULL ; /* break the link */

		/* update lengths, etc. */
		l->t_length             -= l->t_nextline->t_next->t_length
					   + l->t_nextline->t_next->t_padding;
		l->t_nextline->t_length += l->t_nextline->t_next->t_length
					   + l->t_nextline->t_next->t_padding;
		l->t_wordcount -- ;
		l->t_nextline->t_wordcount ++ ;

	    } /* done */
	} /* else if */
	/* ok: so fall thru  */
	debug0("\n line is OK ");

	/* upd_apbo_flags( l ) ;    */

    } /* for()  thru tree */

    debug0("\n fill(): exiting" );
    return( t ) ;
} /* end fill() */



/* ======================== just() ======================================
 *  The justifier is handed a tree of already filled line nodes ad justifies
 *      the lines in alternating directions
 *
 * Use the right margin for just calculations, not rmarg-lmarg
 *
 *  Scan every l->t_length access and width to check for repl
 * =======================================================================
 */

 struct text *
just( struct text *t )
{
 struct line * l ;
 short blankstofill = 0 ;
 short perword, leftover ;
 static boolean forward = TRUE ;    /* dir'n to pad; reset after every line */
 struct word * w ;
 int i ;

    debug0("\n just(): entered" );
    if (t->t_tail != NULL)
	t->t_tail->t_nextline = NULL ; /* to assure loop termination */

    /* last line in a para never gets justified */
    /* also the last line in the para is usually L_EMPTY */

    if ( (t->t_head == t->t_tail) ||
	 ( t->t_head->t_nextline->t_flags & L_EMPTY) )
    {
	debug0("\njust(): return from SPECIAL case ");
	return( t ) ;
    }

    for ( l = t->t_head ;
	  !( ( l->t_nextline == NULL ) || (l->t_nextline->t_flags & L_EMPTY));
	  l = l->t_nextline )
    {
	debugline( l ) ;
	if ( l->t_flags & L_NO_KCUF )
	    break ;     /* last line */
	if ( ( l->t_flags & L_DOT_CMD  ) || ( l->t_wordcount < 2 ) )
	    continue ;      /* strange cases */
	if (l->t_lastword != NULL)
	    l->t_lastword->t_next = NULL ; /* to guarantee loop termination */
	blankstofill  = ( r_marg - l->t_length ) - l->t_padding ;
	blankstofill += l->t_next->t_padding ;  /* first word's padding is in
						 * in `length' calculation;
						 * was subtracted; add in.
						 */
	debug1("\n\tblankstofill=%d ", blankstofill);
	if ( blankstofill > 0 )
	{
	    perword  = blankstofill / ( l->t_wordcount - 1 ) ;
	    leftover = blankstofill % ( l->t_wordcount - 1 ) ;
	    debug2(" perword=%d; leftover=%d\n", perword, leftover);

	    /* first fill the perword stuff; easy */
	    if ( perword )      /* first word not justified */
		for ( w = l->t_next->t_next ; w != NULL ; w = w->t_next )
		{
		    debug1("\n\t\tin perword for{}; pre-pad=%d;", w->t_padding );
		    w->t_padding += perword ;
		    debug1(" post-pad=%d ", w->t_padding );
		}
	    /* now do the leftover stuff */
	    debug1("\n\t forward is %s", forward ? "TRUE" : "FALSE" );
	    if ( leftover )
		if ( forward )
		{   /* distrib leftovers in forward direction */
		    debug0("\n\tin forward");
		    forward = FALSE ;
		    w = l->t_next->t_next ; /* not first word */
		    for ( i= 0 ; i < leftover ; i++ )
		    {
			w->t_padding ++ ;
			debug2("\n\t\tin leftover for{}; word is <%s> pad++ =%d ",
				w->t_offset,  w->t_padding );
			w = w->t_next ;
		    }
		}
		else
		{   /* distrib leftovers backwards */
		    debug0("\n\tin backward");
		    forward = TRUE ;
		    w = l->t_next ;         /* at least one; not first word */
		    for ( i= l->t_wordcount ; i > leftover ; i-- )
			w = w->t_next ; /*  skip to the right starting node */
		    debug1("\n\tfound word to start at <%s>", w->t_offset );
		    /* now start adding */
		    for (;  w != NULL ; w = w->t_next )
		    {
			w->t_padding ++ ;
			debug1("\n\t\t in leftover for{}; pad++ =%d ", w->t_padding );
		    }
		}

	} /* end if ()  */
    } /* end for () thru tree */

    debug0("\n just(): exiting" );
    return( t ) ;
} /* end just() */


#ifdef DEBUG
void printline(struct line * l )
{
 struct word * w ;

    if ( l == NULL )
    {
	(void) fprintf(stderr, "\nprintline: line is empty; returning");
	return ;
    }
    (void) fprintf(stderr, "\n in printline; doing line @ %o\n\tflags = %o;  padding = %d;  length = %d;  wordcount = %d;" /* justablew = %d"*/,
	    l, l->t_flags, l->t_padding, l->t_length, l->t_wordcount /*,  l->t_justablew */ );
    for ( w = l->t_next ; w ; w = w->t_next )
    {
	(void) fprintf(stderr, "\n\t\t word is <%s>; padding %d; length %d; flags %o; cflags %o",
		w->t_offset, w->t_padding, w->t_length, w->t_flags, w->t_cflags) ;
    }
}
#endif
