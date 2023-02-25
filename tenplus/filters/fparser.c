#if !defined(lint)
static char sccsid[] = "@(#)68	1.5  src/tenplus/filters/fparser.c, tenplus, tenplus411, GOLD410 7/18/91 12:38:51";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	parse_drive, apply, do_default_action, det_und, det_eos_sp
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
 *  fparser.c       -   Routines to parse the input into a tree of text
 *                      for format.
 *
 * =======================================================================
 */

#include "fdefines.h"
#include "fexterns.h"
#include "filters.h"

extern char default_action ; /* j for just; f for fill; c for center (unsure)*/

extern void	exit(int);

/* ====================== parse_drive() ==================================
 *
 * Keep getting lines from the lexer until we have enough to do something
 *  with it.  Then do that something, put it out and free up the tree.
 *
 * In-line directives are currently ignored, for various reasons.
 *
 * The criterion for `have enuf' is if we encounter an end-of-para or a
 *  dotted command. (For now, will probably extend this to deal with out of
 *  space conditions.)
 *
 * It is the `do something with it' part that is the major work of the parser.
 *  Currently, when we have think that we have enuf, we do the following:
 *  --  if we encounter a blank line, end the para (or tree) and send it along
 *      to the apply routine that determines what to do with the tree,
 *      depending on the `flags' or if we've seen a command.  If we haven't
 *      seen a command or the flags are zero, apply() just calls the default
 *      action routine, otherwise, it interprets the flag settings and calls
 *      the appropriate routines to process the routine.
 *  --  if we encounter a dotted command, we do the same thing as above
 *      except that after the tree has been processed, we set some flags,
 *      depending on what the command was, so that the next round of
 *      processing will do the right thing.  Currently the only commands we
 *      process are fill, just (implies fill and no just), and the `no'
 *      variants of these.  Font changes are flagged as ignore;  others are
 *      broken on.
 *
 * Note:  The parser is the only routine that has an overall picture of the
 *  paragraph.  Therefore, any flag modifications that reflect the overall
 *  picture (e.g. the status of the first few underlined words on the next
 *  line depending on the type of underlining of the last few words on the
 *  previous line)  must be handled here.  Also if any modifications in
 *  what commands need to be handled by the program needs to be changed,
 *  this is where they should be made.
 *
 * =======================================================================
 */
/* might want to use the `nrlines' field to control termination of
 * tree processing
 */

void parse_drive(void )
{
 struct  text * t ;
 struct  line * l,
	      * prevl = NULL ;
 extern struct line *lex(void);
 extern char *xalloc(int nbytes);
 boolean firsttime = TRUE ;
 boolean seenacmd  = FALSE ;
 static  FLAG_TYPE flags = 0 ; /* command environment; bits indicating action */

    debug0("\nparse_drive(): entered;  getting Text node...") ;
    t = getAnode( text ) ;      /* remember to match up head and tail properly */
				/* don't ever free this; we only allocate one */
start:
    /* t->t_nrlines = 0 ;   */
    /* firsttime = TRUE ;   */

    while ( ( l = lex()) != NULL )  /* don't forget to flush if EOF */
    {
	/* don't forget to test for the empty line as a break */
	if ( firsttime )
	{
	    firsttime = FALSE ;
	    t->t_head = l ;
	}
	else
	{
	    prevl->t_nextline = l ;
	    det_und( l, prevl ) ;  /* det type und for first word on line l */
	    det_eos_sp( l, prevl ) ;    /* det def space if for first word */
	}
	prevl = l ;
	t->t_nrlines ++ ;

	/* what if both || conditions are true; does this have any
	 *  implications on the checks we make below
	 */
	/* Now see if we got enuf; i.e. do we have either of conditions below */
	if ( ( l->t_flags & ( L_EMPTY | L_DOT_CMD )) || ( bigblock == NULL ) )
	{
	    t->t_tail = l;
	    l->t_nextline = NULL ; /* to assure loop termination */
	    /* l->t_next = NULL ; */ /* to assure loop termination */
	    if ( !( l->t_flags & ( L_EMPTY | L_DOT_CMD )) ) /* logic ?? */
	    {   /* then there is no bigblock; i.e. bigblock == NULL */
		/*  and this last line is not empty or a command */
		l->t_flags |= L_NO_KCUF ;
	    }
	    apply( seenacmd, flags, t ) ;   /* apply the actions we have so
					     *  far to the tree and note the
					     *  cmd just seen for the next
					     *  round of processing.
					     */
	    output( t ) ;           /* output the tree */
	    freetree( t ) ;

	    /* ?? Don't try to access the old l if looking for L_NO_KCUF;
	     *  it may have been freed or the flag may have moved up to
	     *  the prev line
	     */
	    if ( bigblock == NULL )
	    {   /* try to get the big block */
		bigblock = xalloc( BIGBLKSIZ ) ; /* xalloc no ret on err */
		if ( t->t_head )
		/* if ( !( l->t_flags & ( L_EMPTY | L_DOT_CMD )) )* danger */
		{
		    /* there was a legitimate line: not cmd or empty */
		    t->t_head->t_flags &= ~L_NO_KCUF ;
		    prevl = t->t_head ;
		    firsttime = FALSE ;
		    goto start ;
		}
	    }

	    /* now set up the flags so that the command just seen will be used
	     *  on the next tree to be gathered.
	     */
	    if ( l->t_flags & L_DOT_CMD ) /* if dotted command... */
	    {
		seenacmd = TRUE ;
		/* first check to see if it is a dotted comment */
		if ( l->t_next->t_flags & W_C_COMMENT )
		    flags |= F_IGNORE ; /* ignore this line */
					/* what effect does this have on
					 *  subsequent environments
					 */
		else    /* not a comment; the w->t_cmdix holds the directive*/
		    switch ( l->t_next->t_cmdix )
		    {
			/* this is the place to add extensions to the stuff
			 *  acted upon by the formatter. e.g. if one wanted
			 *  to proces fonts, one would just add another
			 *  entry into the following switch(){} and
			 *  one in the if-then-else in apply(); one also
			 *  needs to add a routine that will do the
			 *  additional font processing that apply can call
			 */

			case    D_f :           /* fill directive */
			    flags &= ~( F_BREAK | F_IGNORE ) ;
			    flags |= F_FILL ;
			    break ;

			case    D_j :           /* just directive */
			    flags &= ~( F_BREAK | F_IGNORE ) ;
			    flags |= F_JUST ;   /* implies fill only */
			    break ;

			case    D_nf:           /* no-fill directive */
			    flags &= ~( F_IGNORE | F_FILL ) ;
			    flags |= F_BREAK ;
			    break ;

			case    D_nj:           /* no-just directive */
			    flags &= ~( F_IGNORE | F_JUST ) ;
			    flags |= F_BREAK ;
			    break ;

			/* all fonts are flagged as ignore for now */
			case    D_fb:           /* font: bold        */
			case    D_fi:           /* font: italic      */
			case    D_fr:           /* font: roman       */
			case    D_fp:           /* font: previous    */
			    flags &= ~( F_BREAK ) ; /* ??? is this right */
						    /* what about ~(F_FILL|F_JUST)
						    * no this may be temp */
			    flags |= F_IGNORE ;
			    break ;

			/* the remaining directives cause a break */
			default     :
			    flags &= ~( F_IGNORE | F_JUST | F_FILL ) ;
			    flags |= F_BREAK ;

		    } /* end switch(){} */
	    } /* end if-dotted-cmd  */

	    firsttime = TRUE ;
	    goto start ;
	}
    } /* end while */

    /* do we need to worry about having run out of space here ??? */
    if ( firsttime ) /* then head has not been initialized yet */
	return ;    /* null file */
    t->t_tail = prevl ;
    prevl->t_nextline = NULL ; /* to assure loop termination */

    apply( seenacmd, flags, t ) ;   /* apply the actions we have so far */
    output( t ) ;           /* output the tree */
    freetree( t );          /* why bother */
    debug0("\nparse_drive(): exiting");

} /* end main()  */


/* ========================= apply() =====================================
 *
 * Routine to apply routines to the tree according the setting of the flag.
 * =======================================================================
 */
void apply(boolean seenacmd, FLAG_TYPE flags, struct text *tree )
{
    if ( !seenacmd  || !flags )
	do_default_action( tree ) ;
    else
    {
	if ( flags & ( F_BREAK | F_IGNORE ) )
	    ;   /* fall thru */
	else if ( flags & F_JUST )
	{
	    (void) fill( tree ) ;  /* implies fill only */
	    /* just( tree ) ; */
	}
	else if ( flags & F_FILL )  /* has to be this */
	    (void) fill( tree ) ;
	else
	    (void) fprintf( stderr, "\n*** apply(): Internal error; bad flag setting; ignored. ***\n" ) ;
    }
    /* everyone falls thru here */

} /* end apply()  */
/* caller outputs and then frees the tree */


/* ========================= do_default_action() =========================
 *
 *  routine to execute the default action on the tree
 * =======================================================================
 */
void do_default_action( struct text *t )
{
    debug1("\ndo_default_action(): entered: default_action is <%c>", default_action );
    switch ( default_action )
    {
	case 'j' :  (void) fill( t ) ;
		    (void) just( t ) ;         /* I think these only do a line */
		    break ;             /*      at a time */
					/*  ??? so they need to be modified */
	case 'f' :  (void) fill( t ) ;         /* also they shouldn't be the ones */
		    break ;             /*      to deallocate the space */

    /*  case 'c' :  center( t ) ;
		    break ;
     */
	default  :  (void) fprintf(stderr, "\n*** do_default_action(): internal error in switch{%c} ***\n",
			     default_action );
		    exit( EXITBAD );
    }
    debug0("\n do_default_action(): exiting");
} /* end do_default_action( t ) */


/* ========================== det_und() ==================================
 *  This routine determines the type of underlining the first word on line
 *  `l' requires, depending on the type of underlining of the word next to
 *  it, and of the last two words (if any) on the previous line.
 *
 * =======================================================================
 */

/* (unsure) ??? use t_wordcount field to determine the number of words in the line */

void det_und(struct line * l, struct line *prevl ) /* call by ref, since pointers */
{
 register struct word * w = NULL ;

    /* we know that l isn't the first line */
    if ( /* first check if the current line should be considered */
	 (!( l->t_flags & L_EMPTY ))                            &&
	 (   l->t_next )                                        &&
	 (   l->t_next->t_flags & ( W_CONT | W_UND ) )          &&
	 /* ok; the current line wants consideration; check previous line */
	 (!( prevl->t_flags & L_EMPTY ))                        &&
	 (   prevl->t_lastword )                                &&
	 (   prevl->t_lastword->t_flags & ( W_CONT | W_UND ) )
       )
    {   /* then only are we ready to consider CUing the first word on line l
	 *  we know that there is at least one word on each line and that
	 *  they have some kind of underlining.  Note that the first word in
	 *  l, if underlined, has been marked W_UND, since it shouldn't have
	 *  had any leading _<BS><SP>
	 */

	/* l->t_next is CU iff:
	 *  --  last two words on the prev line are underlined and the space
	 *      between them is also underlined, or
	 *  --  if the first two words on the current line are underlined and
	 *      the space between them is also underlined
	 */
	if ( ( ( l->t_wordcount > 1 )       &&
	       ( l->t_next->t_next->t_flags & W_CONT )
	     ) ||
	     ( ( prevl->t_wordcount > 1 )   &&
	       ( prevl->t_lastword->t_flags & W_CONT )
	     )
	   )
	{   /* then mark the first word CU */
	    l->t_next->t_flags &= ~W_UND ;   /* first erase WU */
	    l->t_next->t_flags |= W_CONT ;  /* then mark it CU */
	}

	/* l->t_next is CU iff: ( special case )
	 *  --  the last word in the previously line is underlined and the
	 *      second-to-last word has no kind of underlining, and the first
	 *      word on the current line is underlined and the second word
	 *      on the same line has no underlining of any kind
	 */
	else if ( ( ( l->t_wordcount > 1 ) &&   /* necessary to have one ?? */
		    /* if next word has either kind of underlining, quit */
		    (!( l->t_next->t_next->t_flags & ( W_UND | W_CONT )) )
		  ) ||
		  ( l->t_wordcount == 1 ) /* one word on line and UND */
		)
	{   /* then check the second-to-last word on the previous line to
	     *  make sure that it too has no underlining of any kind; if so
	     *  then CU the first word
	     */
	    if ( prevl->t_wordcount == 1 )  /* one word on line and UND */
	    {   /* then mark the first word CU */
		l->t_next->t_flags &= ~W_UND ;   /* first erase WU */
		l->t_next->t_flags |= W_CONT ;  /* then mark it CU */
	    }
	    else
	    {
		for ( w = prevl->t_next ; w->t_next != prevl->t_lastword ; w = w->t_next )
		    ; /* skip across prev line and find the second to last word */

		/* if it has any kind of underlining, quit */
		if ( !( w->t_flags & ( W_UND | W_CONT ) ) )
		{   /* then mark the first word CU */
		    l->t_next->t_flags &= ~W_UND ;   /* first erase WU */
		    l->t_next->t_flags |= W_CONT ;  /* then mark it CU */
		}
	    }
	} /* special case else if () */
    } /* end of main if () */

} /* end routine det_und() */


/* ========================== det_eos_sp() ===============================
 * Determine End-Of-Sentence Spacing
 * This routine determines the default underlining for the first word on the
 *  current line depending on how the last word on the previous line is
 *  punctuated.
 * Note that the first word comes in with a default padding of one already
 *  set up.
 * All this routine is interested in doing is seeing if this default needs to
 *  be two if the last word on the prev line ends with a . or : or ? or !
 *
 * =======================================================================
 */

void det_eos_sp(struct line * l, struct line *prevl )     /* call by ref, since pointers */
{
 register char * s ;

    /* we know a prevl exists */
    if ( ( prevl->t_lastword           == NULL )        ||
	 ( prevl->t_lastword->t_offset == NULL )        ||
	 ( prevl->t_flags & ( L_EMPTY | L_DOT_CMD ))    ||
	 ( l == NULL )                                  ||
	 ( l->t_flags & ( L_EMPTY | L_DOT_CMD ))        ||
	 ( l->t_next->t_flags & W_0_LEN )
       )
	return ;    /* safety check */
    s = prevl->t_lastword->t_offset ;  /* get at the word */
    while ( *s )
	s++ ;                          /* skip to end of string */
    if ( is_sent_end( *( s - 1 ) ) )   /* assume no trailing blanks in word */
    {
	l->t_next->t_padding ++ ;   /* = 2 */
	l->t_length ++ ;            /* line's length has also been increased */
    }
} /* end det_eos_sp()  */
