#if !defined(lint)
static char sccsid[] = "@(#)70	1.5  src/tenplus/filters/foutput.c, tenplus, tenplus411, GOLD410 7/18/91 12:38:46";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	output, upd_apbo_flags
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
 *  foutput.c       -   Routines to output the tree for format.
 *
 * =======================================================================
 */

#include    <stdio.h>
#include <chardefs.h>
#include    "fdefines.h"
#include    "fexterns.h"
#include    "filters.h"


/* ======================== output() =====================================
 *  The outputter is handed a tree of nodes and
 *  outputs it being careful to restore previously tilde'ed lines back
 *  again.  Note that the lines have already been formatted by the parser
 *  calling just/fill
 *  Notes on underlining:
 *  -- remember that we just fkag a word as being tilde'ed. we don't
 *      actually put the _<BS><SP> in the word.  Similarly, for padding
 *      purposes in CU words.
 *
 *  Don't free up space here
 * =======================================================================
 */
    /* make sure the outputter knows that W_TILDE means put out
     *  preceding tildes and ( W_TILDE | W_UND ) means put out preceding blanks
     */

void output( struct text *t )
{
 char   * str,
	nextline[ BUFSIZ ],
	* nextlp = nextline,
	fillchar ;
 struct word * w ;
 struct line * l ;
 int    i ;
 boolean tildeflag = FALSE,
	firstword = TRUE ;
 int first_time;

    /* Zip thru word nodes outputting them.  Special cases occur for :
     *  --  first word.
     *  --  tilde'ed words ( CU and WU )
     * Don't forget to output terminating \n on the current line.
     * If tildeflag is up add the terminating NUL and output next line and \n
     */

    debug0("\noutput(): entered");
    l = t->t_head ;
    first_time = TRUE;
    /*  at least one line */
    do /* while head != tail */
    {
        if (!first_time) 
	       l = l->t_nextline ;
        first_time = FALSE;
        
        if ( ( l == NULL ) || ( l->t_flags & L_NO_KCUF ) )
	    return ;
#ifdef DEBUG
    /* initialize the line to all nulls for debugging/printing */
    for ( i = 0 ; i < BUFSIZ ; i++ )
	nextline[ i ] = NUL ;
    i = 0 ; /* re-init in case it is used below */
#endif

	(void) upd_apbo_flags( l ) ;   /* update the flags before outputting */

	nextlp = nextline ;
	tildeflag = FALSE ;
	debug1("\n\tin do{}; doing line @ <%o>",l );

	/* dotted line ? then do not output */
	if ( l->t_flags & L_DOTTED )
	{
	    debug0("  line is dotted") ;
	    continue ;              /* the do{} while() */
	}

	/* empty line ? then just output \n */
	if ( l->t_flags & L_EMPTY )
	{
	    debug0("  line is empty") ;
	    (void) putchar( NEWLINE ) ;
	    continue ;              /* the do{} while() */
	}

	firstword = ( l->t_next->t_flags & W_0_LEN ) ? FALSE : TRUE ;
	tildeflag = ( l->t_flags & L_TILDE ) ; /* see if line is tilde'ed */
	debug1("  tildeflag <%s> ", tildeflag ? "TRUE" : "FALSE" ) ;
	/* output the left margin that was in the input */
	debug0("output l margin<") ;
	for ( i = l->t_padding ; i > 0 ; i-- )
	{
	    (void) putchar( SPACE ) ;
	    if ( tildeflag ) *nextlp++ = SPACE ;  /* also put blanks in nextln */
	}
	debug0(">") ;

	/* first word in each line is special   */
	w = (struct word*)l ; /* so that the loop is generalized */
	/*
	 * the following if clause was added as an ad hoc fix to prevent
	 *      blowing up when l->t_lastword is null.  It might just hide
	 *      a more deeply rooted problem, or it might have been the
	 *      real bug.
	 */
	if (l != NULL && l->t_lastword != NULL)
	    l->t_lastword->t_next = NULL ; /* to assure loop termination */

	while ( ( w = w->t_next ) != NULL ) /* process the line */
	{
	    /* Here, to `pad' means to precede the word w/approp # of atomic
	     *  separators.
	     * cases:
	     *  --  word not _ or ~  : Just use blanks to pad here
	     *  --  word is ~ and WU : use blanks to pad this and nextline
	     *                          and ~ to underline word on next line
	     *  --  word is ~ and CU : use blanks to pad this line; use ~ to
	     *                         pad and underline word on nextline
	     *  --  word is not ~ and WU : use blanks to pad here
	     *  --  word is not ~ and CU : use _<BS><SP> to pad here
	     *  if tildeflag is up and no explicit char is specified above
	     *      for next line, then use SPACE.
	     */

	    /* first output the padding */
	    debug0("\n\t\tword pad <") ;
	    if ( ! firstword )  /* first word has no padding */
	    {
		/* set up the fillchar for the padding on the next line */
		/* To be used only if tildeflag is set */

		/* if word is tilde CU then nextline gets ~ else space */
		if (tildeflag && ( w->t_flags == ( W_TILDE | W_CONT ) ) )
		     fillchar = TILDE ;
		else fillchar = SPACE ;
		/* padding needs to be done carefully on next line; use
		 *  tildeflag to determine padding on next line
		 */
		for ( i = w->t_padding ; i > 0 ; i-- )
		{
		    if ( w->t_flags & W_TILDE ) /* if word wuz tilde'd; CU or WU */
			;   /* fall thru below */
		    else if ( w->t_flags & W_CONT ) /* if word is normal CU */
		    {
			(void) putchar( UNDERBAR ) ;
			(void) putchar( BACKSPACE ) ;
			/* fall thru below */
		    }
		    else /* word is normal text or WU; just put out blanks */
			;   /* fall thru below */

		    /* everyone falls thru to here */
		    (void) putchar( SPACE ) ;
		    if ( tildeflag ) *nextlp++ = fillchar ;
		}
	    }   /* padding complete */

	    debug0(">;\tword <") ;
	    /* now output the word itself */
	    firstword = FALSE ; /* past the first word */
	    /* tilde WU and CU both get tilde's for fillchar of next line */
	    fillchar = ( w->t_flags & W_TILDE ) ? TILDE
						: SPACE ;
	    for ( str = w->t_offset ; str && *str != NUL ; str++ )
	    {
		(void) putchar( *str ) ;  /* ok */
		if ( tildeflag )
			*nextlp++ = fillchar ;
	    }
	    debug0(">") ;
	    if ( tildeflag )
		debug1("\n\ttildeflag==TRUE; nextline so far\n<%s>",nextline);
	} /* end while thru line */

	(void) putchar( NEWLINE ) ;
	if ( tildeflag )
	{
	    *nextlp = NUL ;
	    (void) printf( "%s\n", nextline ) ;
	}
    } while ( l != t->t_tail ) ; /* end do{} */
    debug0("\noutput(): exiting");

} /* end output( )  */


/* ======================= upd_apbo_flags() ===============================
 * Update After Process and Before Output Flags:
 *
 * now we need to go thru the line and set the line flags that need to
 *  be updated after any kind of processing and before output
 *
 * =======================================================================
 */

void upd_apbo_flags( struct line *l )     /* call by ref */
{
 struct word * w ;

    debug0("\n upd_apbo_flags(): entered;  ");
    for ( w = l->t_next ; w != NULL ; w = w->t_next )   /* update */
    {
	/* This is done after any kind of processing and before output.
	 *  It is sufficient to do this after the fill phase.
	 */
	if ( w->t_flags &  W_TILDE )
	    l->t_flags |= L_TILDE ; /* set ~  flag; i.e. just indicate existence */

    } /* for() thru update */
    debug0("  exiting");

} /* end upd_apbo_flags( ) */
