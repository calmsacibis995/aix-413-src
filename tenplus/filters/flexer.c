#if !defined(lint)
static char sccsid[] = "@(#)69	1.7  src/tenplus/filters/flexer.c, tenplus, tenplus411, GOLD410 4/13/94 16:54:38";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	lex, proc_dot_cmd, det_cmd_type, proc_non_dot,
 *		proc_word_node, proc_bs_cmd, det_brack_type, is_open_bracket
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
 
/* =======================================================================
 *
 *  flexer.c        -   Routines to lex the input into words and lines
 *                      for format.
 *
 * =======================================================================
 */

#include <stdlib.h>
#include    <string.h>
#include <chardefs.h>
#include    "fdefines.h"
#include    "fexterns.h"
#include <ctype.h>
#include <limits.h>

/* create and initialise structures used internally */

/* cmd_array structure must be compatible with enum dirix decln in fexterns.h */
/* format is:
 *  --  character str describing cmd
 *  --  internal flag corresponding to the dirix stuff in enum dirix .
 *  --
 * do we really have to implement such an extensive structure ????
 *  why don't we use the exclusion method: if we know it do it, else break.
 */

static struct dir cmd_array[] =
{
/** {   ======      =====   },  **/
/** {   string      dirix   },  **/
/** {   ======      =====   },  **/
/*  {   "",         D_NODIR },
    {   " ",        D_NODIR },
 */ /* incorporate the above  checks into the routine */
    {   "1f",       D_1f    },                     /* page 1 has footer too */
    {   "1h",       D_1h    },                     /* page 1 has header too */
    {   "bp",       D_bp    },                     /* break page            */
    {   "br",       D_br    },                     /* break (paragraph)     */
    {   "c",        D_c     },                     /* center                */
    {   "ce",       D_ce    },                     /* center end            */
    {   "cu",       D_cu    },                     /* continuous underline  */
    {   "cui",      D_cui   },                     /* cu, immediate         */
    {   "d",        D_d     },                     /* down 1/2 (ESC 9)      */
    {   "debug",    D_db    },                     /* debug flags           */
    {   "ds",       D_ds    },                     /* double/triple...space */
    {   "ef",       D_ef    },                     /* end footer            */
    {   "eh",       D_eh    },                     /* end header            */
    {   "eo",       D_eo    },                     /* even offset           */
    {   "ep",       D_ep    },                     /* extend page           */
    {   "f",        D_f     },                     /* fill                  */
    {   "fb",       D_fb    },                     /* font: bold            */
    {   "fe",       D_fe    },                     /* footnote end          */
    {   "fi",       D_fi    },                     /* font: italic          */
    {   "fk",       D_fk    },                     /* floating keep         */
    {   "fm",       D_fm    },                     /* footnote marker       */
    {   "fp",       D_fp    },                     /* back to previous font */
    {   "fr",       D_fr    },                     /* font: roman           */
    {   "fs",       D_fs    },                     /* footnote start        */
    {   "ft",       D_ft    },                     /* footer                */
    {   "hd",       D_hd    },                     /* header                */
    {   "hf",       D_hf    },                     /* heading format        */
    {   "hn",       D_hn    },                     /* heading, numbered     */
    {   "hu",       D_hu    },                     /* heading, unnumbered   */
    {   "hy",       D_hy    },                     /* hyphenate             */
    {   "j",        D_j     },                     /* justify               */
    {   "ke",       D_ke    },                     /* keep end              */
    {   "kf",       D_kf    },                     /* floating keep (== fk) */
    {   "ks",       D_ks    },                     /* keep start            */
    {   "lm",       D_lm    },                     /* left margin           */
    {   "ne",       D_ne    },                     /* need (output space)   */
    {   "nf",       D_nf    },                     /* no fill               */
    {   "nh",       D_nh    },                     /* no hyphenate          */
    {   "nj",       D_nj    },                     /* no justify            */
    {   "opt",      D_opt   },                     /* options:  [0] is name */
    {   "p1f",      D_p1f   },                     /* page 1 has footer too */
    {   "p1h",      D_p1h   },                     /* page 1 has header too */
    {   "pb",       D_pb    },                     /* paragraph break       */
    {   "pf",       D_pf    },                     /* page nr format        */
    {   "pl",       D_pl    },                     /* page length           */
    {   "pn",       D_pn    },                     /* page number           */
    {   "po",       D_po    },                     /* page offset           */
    {   "rj",       D_rj    },                     /* right justify         */
    {   "rm",       D_rm    },                     /* right margin          */
    {   "so",       D_so    },                     /* source file           */
    {   "sp",       D_sp    },                     /* space                 */
    {   "ss",       D_ss    },                     /* single space          */
    {   "u",        D_u     },                     /* up 1/2 line (ESC 8)   */
    {   "ue",       D_ue    },                     /* underline end         */
    {   "wu",       D_wu    },                     /* word underline        */
    {   0,          D_NODIR }                      /* end-of-table marker   */
} ;

    /*  Table of open brackets */
static wchar_t open_brack_tab[] =
{
    L'{' ,
    L'[' ,
    L'(' ,
    L'<' ,
    L'\'',
    0
} ;

    /*  Corresponding table of close brackets */
static wchar_t close_brack_tab[] =
{
    L'}' ,
    L']' ,
    L')' ,
    L'>' ,
    L'\'',
    0
} ;

enum dirix det_cmd_type(wchar_t *);
int det_brack_type( wchar_t );
struct word * proc_word_node(void );
struct line * proc_non_dot(void);
struct line * proc_dot_cmd(void);
struct word * proc_bs_cmd(void );
boolean is_open_bracket(wchar_t);

extern FILE *input ;

extern void ungetstr( char *);
char * getline(void);
char * newmbstr(wchar_t *  );
static int wstr_disp_width(wchar_t *);
 /*
  *     Globals:
  */
 wchar_t  *s ,                                    /* pointer to current line */
	  *nextlp ;                          /* pointer to next line, if any */
 wchar_t  *nxtlpext ;       /* extent of next line that contains useful data */
 boolean tildeflag = FALSE ;                /* TRUE if next line has tildes */
 static int len;


/* ========================== lex() ======================================
 *      The lexer reads the input  text intelligently handling the special
 *  cases:
 *  --  process & flag tilde'ed underline stuff correctly
 *  --  place default blank count of 1 for 1st word
 *  --  process and flag & create command nodes & lines properly
 *
 * Basically, the lexer reads just one line of text at a time (with a
 *  one-line lookahead buffer for ~~~) and forms word and line nodes
 *  out of it.  The parser (its caller) is then free to interpret the line.
 *
 * The lexer has a picture of the current line only.  So flags relevant
 *  to the current line are set here.  (Flags relevant to the para are
 *  set in the parser.)
 *
 * =======================================================================
 */

 struct line *
lex(void)
{
 struct line * proc_dot_cmd(void) ,
	     * proc_non_dot(void) ,
	     * templ ;
 wchar_t * temps ;
 size_t    len_s;
 char    * mbs;
 static int dotted_line_processed = FALSE;

    debug0("\n lex(): entered") ;
    if ( ( mbs = getline())  == NULL )
	return( NULL ) ;            /* EOF occurred last time thru */

    debug1("lex: input line = %s\n", mbs);

    if (( *mbs == DOT ) && !dotted_line_processed)
    {
	/* line beginning with dot. Coerce parser to do same 
           things as with empty line. Make sure this line is processed
           next time as an ordinary line. */
        templ = getAnode( line ) ;
	templ->t_flags = L_DOTTED | L_EMPTY;

        /* now init the line structure. */
        templ->t_length = templ->t_wordcount = 0 ;
        templ->t_nextline = NULL;
        templ->t_lastword = templ->t_next = NULL ;
        templ->t_padding = 0 ;

        ungetstr(mbs);
        dotted_line_processed = TRUE;
	return( templ );
    }

    dotted_line_processed = FALSE;

    len_s = strlen(mbs) + 1;
    temps = s = malloc(len_s * sizeof(wchar_t));
    (void) mbstowcs(s, mbs, len_s);
    free(mbs);

/*    if ( *s == LDOT )
	templ = proc_dot_cmd() ;         * process dotted line command */
/*    else  */
	templ = proc_non_dot() ;         /* otherwise process the line as
					  * normal text that may be mixed
					  *  with \ cmds
					  */
    free( temps ) ; /* free the dynamically allocated line */
    debug0("\n lex(): exiting") ;
    return( templ ) ;

}   /* end lex() */


/* ====================== proc_dot_cmd() =================================
 * Process a dotted line command.  Converts a line into a line node
 *  pointing to a word node that contains the string.  The line node is
 *  appropriately flagged. (L_DOT_CMD)
 *
 * Dotted command nodes, being a classification of the command is otherwise
 *  a straight copy of the currennt line.  Thus there is no length restriction
 * =======================================================================
 */

 struct line *
proc_dot_cmd(void)
{
 struct line * l ;                                 /* our working line node */

    debug1("\nproc_dot_cmd(): entered with <%S>; getting Line node", s) ;
    l = getAnode( line ) ;                  /* get a pointer to a line node */
    l->t_next = getAnode( word );/* get a word node; the only one in routine */

    /* indicate a few of the dotted cmd node's properties */
    l->t_flags = L_DOT_CMD ;                   /* flag as a dotted cmd line */
    l->t_next->t_cflags = W_C_DOT ;

    /* line and word is zero padded; init other fields */
    l->t_padding = l->t_next->t_padding = l->t_next->t_flags = 0 ;
    l->t_wordcount = 1 ;
    l->t_nextline = NULL ;
    l->t_next->t_next = l->t_lastword = NULL;
    l->t_next->t_offset = newmbstr( s ) ; /* copy the command verbatim */
    l->t_length = l->t_next->t_length = wcswidth( s, INT_MAX ) ;
    if (l->t_length < 0)
	l->t_length = l->t_next->t_length = 0;

    /* now determine the type of cmd and flag the node appropriately */
    if ( ( *(s + 1) == LDOUBLEQUOTE )  ||      /* is it ." or .\"   ?  */
	 ( ( *(s + 1) == LBACKSLASH   )  && 
           ( *(s + 2) == LDOUBLEQUOTE ) ) )
	l->t_next->t_cflags |= W_C_COMMENT ;
    else
	l->t_next->t_cmdix = det_cmd_type( s ) ;
    /* do we need to determine its equiv. balancing cmd (actually its array
     *      ptr); if so set aside another field
     */

    debug0("\n proc_dot_cmd(): exiting") ;
    return( l ) ;
} /* end proc_dot_cmd()  */

/* ====================== det_cmd_type() =================================
 *
 * Routine to determine the command type.
 *  Looks thru the cmd_array to see if this command exists and returns the
 *  enum associated with it.
 *  The assumption here is that the size of the (legitimate) directive is
 *  no more than CMDSIZ long.  Comments, which can be arbitrarily long, never
 *  get here.
 *
 * =======================================================================
 */
 enum dirix
det_cmd_type( wchar_t *strng )  /* determines the command type */   /* call by value */
{
 int    i ;
 wchar_t   cmd_buf[ CMDSIZ ] ,
	   * cmd_str = cmd_buf ;
 char      mbcmd_buf[ CMDSIZ * sizeof(wchar_t)];


    while ( (*strng != LNUL) && (! iswalpha( *strng )) && (! iswdigit( *strng ) ) )
        strng ++ ;  /* skip over non-alphas like \, <, ., etc. */

    while ( (*strng != LNUL) && ( iswalpha( *strng ) || iswdigit( *strng ) ) )  /* and ignore >,SPACE*/
        *cmd_str++ = ( iswupper( *strng ) )  ? towlower( *strng++ )
                                                : *strng++ ;
    *cmd_str = LNUL ;       /* ending */


    /* now cycle thru the cmd array seeing if this exists and indicate its type */
    cmd_str = cmd_buf ;

    if ( *cmd_str == LNUL )  /* the \<SPACE> case should never get here  */
    {
	debug1("\n det_cmd_type(): exiting with D_NODIR; cmd_buf<%s> ", cmd_buf );
	return( D_NODIR ) ;        /* this is a bad cmd */
    }

    /* otherwise search the array */
    (void) wcstombs(mbcmd_buf, cmd_buf, CMDSIZ * sizeof(wchar_t));
    i = 0;
    while ( cmd_array[ i ].dir_p /* != 0 */ )
	if ( strcmp( mbcmd_buf, cmd_array[ i ].dir_p ) ) /* if not equal */
	    i++ ;
	else    /* we found it */
	{
	    debug1("\n det_cmd_type(): exiting. matched <%s>", cmd_array[i].dir_p );
	    return( cmd_array[ i ].dir_e );     /* return the flag */
	}
    return( D_NODIR ) ;        /* this is a bad cmd */
} /* end  det_cmd_type()  */


/* ====================== proc_non_dot() =================================
 * routine to process a line that doesn't begin with a dot.  This line of
 *  course may begin with a \ . but then \ commands may occur anywhere
 *  in a non-dotted line. Thus, in encountering a \ (but not a \\) we
 *  call the routine to handle \ cmds.
 * remember to free up the next line if tildeflag is true.
 *
 *
 * =======================================================================
 */

/*
 *  remember the three cases we had to distinguish for proc_word()? make sure
 *  you take care of them here also
 */
 struct line *
proc_non_dot(void)
{
 wchar_t   * str ,
	   * savenextlp ;
 char * mbnextlp;
 size_t len_nextlp;
 int    blanks ;      /* number of leading blanks */
 int    disp_blanks ; /* width of leading blanks */
 struct line * l ;
 struct word * proc_bs_cmd(void) ,
	     * proc_word_node(void) ,
	     * w ;
 boolean firstword = TRUE ;

    debug0("\n proc_non_dot(): entered;  getting Line node...") ;
    l = getAnode( line ) ;
    str = s ;
    /* lookahead for empty line  */
    while (iswspace(*str))  str++ ;

    /* now init the line structure; start linking word and command nodes to it. */
    l->t_length = l->t_wordcount = 0 ;
    l->t_nextline = NULL;
    l->t_lastword = l->t_next = NULL ;
    l->t_flags = l->t_padding = 0 ;

    if ( *str == LNUL )
    {
	/* empty line */
	l->t_flags = L_EMPTY ;
	return( l );
	/* note that empty lines don't have any word nodes; careful when freeing */
    }
    
    /*  so far we have a non-empty line; count blanks and indicate number */
    blanks = 0;
    disp_blanks = 0;
    while ( iswspace(*s) )
    {
	blanks ++ ;
	disp_blanks += ((len = wcwidth(*s)) < 0) ? 1 : len ;
	s ++ ;
    }

    l->t_padding = disp_blanks ;         /* indicate the line indentation */

    /* Now comes the hard part: actually forming the word and cmd nodes.
     *  The difficulty is due to the possibility of tilde's on the next
     *  line
     */
    debug0("\nproc_non_dot(): getting lookahead line...") ;

    /* read the next line, convert it to wide chars and let 
       nextlp point to it.
    */
    mbnextlp = getline();
    len_nextlp = strlen(mbnextlp) + 1;
    /* save nextlp in savenextlp so it can be freed after
       the next line (as pointed to by nextlp) has been processed */
    savenextlp = nextlp = malloc(len_nextlp * sizeof(wchar_t)) ;
    (void) mbstowcs(nextlp, mbnextlp, len_nextlp);
    /* do not free mbnextlp here. After the "next" line has been
       processed, we might have to "unget" mbnextlp, using function ungetstr
     */

     /* if the line that was just read begins with a dot, treat it as
        an empty line. */
     if (*nextlp == LDOT)
           *nextlp = LNUL;

    debug0("\nproc_non_dot(): got it") ;
    tildeflag = FALSE ;
    if ( nextlp == NULL )        /* never mind */
	; /* tildeflag already false */
    else
    {
	/* maybe tildes on line and maybe not */
	while ( iswspace( *nextlp ) )    nextlp ++ ;
	if ( *nextlp == LTILDE )
	{
	    /* we need to make sure that this is not a lone tilde.  If
	     *  it is, we're not interested
	     */
	    nextlp ++ ;
	    while ( iswspace( *nextlp ) ) nextlp ++ ;
	    if  ( *nextlp == LTILDE )
	    {
		tildeflag = TRUE ;
		/* get the len to control access before every ==TILDE check */
		nxtlpext = savenextlp + wcslen( savenextlp ) ;
	    }
	}
	/*  else tildeflag already  FALSE */
    }
    debug1(" ..tildeflag is <%s>", tildeflag ? "TRUE" : "FALSE" ) ;

    /* now we have to figure out how to sync the starting points of
     *  of the two lines to relevant positions in each line
     */
    /* It makes sense to advance the tilde ptr as far as the beginning
     *  of the first word on original line, ignoring leading hanging tildes
     */
    if ( tildeflag )
	nextlp = savenextlp + blanks ; /* advance tildelp so that it is
					*   sync-ed to line above.
					*/
    firstword = TRUE ;

    /* This is the main loop:  word nodes are formed here  */
    while ( *s != LNUL )
    {
	str = s ;
	while ( iswspace( *str )) str ++;    /* move past white space to
					     *  determine the type of
					     *  word we are dealing
					     *  with; i.e. lookahead
					     */
	if ( *str == LNUL )     break ;
	if ( ( *str == LBACKSLASH ) && ( * (str + 1) != LBACKSLASH ) )
	    /* this is a backslash-ed command */
	    w = proc_bs_cmd( );       /* use s and not str  */
	else
	    w = proc_word_node( );    /* use s and not str */
	if ( firstword )
	{
	    l->t_next = l->t_lastword = w ;
	    firstword = FALSE ;
	    if ( !(w->t_flags & W_0_LEN ) )
		w->t_padding = 1 ; /* default blank for 1st word, in case moved up */
	}
	else
	{
	    l->t_lastword->t_next = w ;
	    l->t_lastword = w ;
	}
	/* s, and nextlp get updated by the routines below this one. we'll
	 *  update the counts on the line node at the end.
	 */

	/* now update some of the fields before we process and output */
	/* indicate the # of graphics, non-graphics, fillable spaces, words, etc */

	l->t_length +=  w->t_length + w->t_padding ;
	l->t_wordcount ++ ;

	/* This one causes a dilemma.  This should be done before the parser
	 *  (or the filler) gets at it (in case we have \" <text>) or if we
	 *  have a bad \br.  Normal cases where this approach is
	 *  also desirable  are \j , \nj , and their
	 *  cousins.  Then again, \fB , for example, may be treated as ordinary
	 *  text.  The dilemma is due to possible \cmd word nodes having
	 *  moved down or up. and thus indicating this for the benefit
	 *  of further processors.
	 * Quit for now and flag only before any processing. i.e. here & now.
	 * Note that if anyone needs to look at the L_BS_CMD after any kind
	 *  of processing, then it will have to be updated again in
	 *  upd_apbo_flags() .  But so far the \cmd is relevant only to the
	 *  parser, and he sees the flag before there is a chance for any
	 *  processing to move a \cmd to another line and not update the
	 *  line flag.
	 */
	if ( w->t_flags &  W_C_BS )
	    l->t_flags |= L_BS_CMD ; /* set \ cmd  flags */

    } /* while() */
    if (l->t_lastword != NULL)
	l->t_lastword->t_next = NULL ; /* wrapup; assure loop termination */

    if ( ( tildeflag == FALSE )  && ( savenextlp != NULL ))
	ungetstr( mbnextlp ) ; /* we had gotten a string and didn't use it
				  *  so return it
				  */
    else {
        if ( mbnextlp )  /* tildeflag being true ==> mbnextlp != NULL */
	free( mbnextlp ) ; /* if we used the string, free up the space */
        /* otherwise, mbnextlp was NULL and we didn't even get a line */
        if ( savenextlp )  /* free space for the wchar_t copy as well */
	free( savenextlp ) ;
     }

    debug0("\n proc_non_dot(): exiting") ;
    return( l ) ;

}   /* end routine proc_non_dot() */

/* ========================= proc_word_node() ============================
 * routine to process a normal word
 *  be careful to heed flags and update all relevant ptrs.
 *  remember that we are not actually putting blanks or any other
 *  such atomic separator to precede the word; we just put this count into
 *  the padding field and depending on the flags, the outputter will
 *  output the desired number of atomic-separator elements, be they
 *  blanks, _<BS><SPACE> , or TILDE's .
 *
 *  see notes at bottom of prev page.
 * =======================================================================
 */

 struct word *
proc_word_node(void )
{
/* relevant variables
 *  s           => points to the relevant portion of the current line
 *  nextlp      => points to the relevant portion of the next line down;
 *                 only useful if the tildeflag below is true
 *  tildeflag   => true means current line is associated with tilde line below.
 */

 int disp_blanks = 0 ;
 FLAG_TYPE flags = 0 ;
 wchar_t   wordbuf[ BUFSIZ ] ,
	* pwbuf = wordbuf ;
 char	env = 0 ;           /* indicates environment of leading spaces */
 struct word *w ;

    debug1("\n proc_word(): entered with \n<%S>", s) ;
    if ( tildeflag )
	debug1("tildeflag==TRUE; nextlp \n<%S>", nextlp ) ;
    w = getAnode( word ) ;  /* line is not empty; so allocate a word */
    /* need to calculate preceding blanks */

    if ( tildeflag ) /* do tildes */
    {
	/* note that even though tildeflag is up as a warning;  this word may
	 *  not need underlining although tildeptr will need to be updated
	 */
	/* remember if we have preceding hanging underlines; so be aware
	 *  of it here.
	 */

	while ( iswspace(*s) )
	{
	    if ( ( nxtlpext > nextlp ) && ( *nextlp == LTILDE ) )
		flags |= ( W_CONT | W_TILDE ); /* indicate continuous underline*/
	    nextlp ++ ;     /* must be updated either way */
            /* just count up the width of the blanks */
	    disp_blanks += ((len = wcwidth(*s)) < 0) ? 1 : len ;
	    s ++ ;
	}

	/* asking for trouble by tilde-ing already _____ word; so no _<BS><SP> */
	while ( ( (!iswspace( *s )) && (*s != LNUL) ) ||   /* while any kind of text */
		( iswspace( *s )  && ( *(s-1) == LBACKSLASH ) ) ) /* or \<SP> */
		/* this second test should really also modify the `length' of
		 *  word (one less, since \ doesn't count), but it really
		 *  doesn't matter. i.e ff will take care of it properly;
		 *  we just want to collect it.
		 */
	{
            if (*s == LBACKSPACE) {
	        nextlp -- ;         /* update either way */
            } else {
	        nextlp ++ ;         /* update either way */
            }

            *pwbuf++ = *s++ ;   /* copy char */
        }
    }
    else
    {
	/* nextline not tilde'ed; we have three choices as to the type
	 *  of the current word:
	 *  1)  continuous  underline:
	 *      a)  this is the first word in a series: There is no way
	 *              to currently distinguish this from word und
	 *              (Flag it as WU for now. )
	 *      b)  this is not the first:  therefore it has at least one
	 *              _<BS><SP> sequence preceding the real word.
	 *              Flag it as CU
	 *  2)  word underline: Flag it as WU
	 *  3)  normal text ( not just alphabet ! )
	 */

	while ( iswspace(*s) )
	{
	    disp_blanks += ((len = wcwidth(*s)) < 0) ? 1 : len ;
	    s++ ;
	    env |=  LS_SPACE ;
	}
	/* see if case (1a) or (2); i.e.  maybe _<BS><SP> */
	if      ( ( *s         == LUNDERBAR  )  &&
		  ( (*(s + 1)) == LBACKSPACE )  &&
		  (  !iswspace(*(s + 2))     )      /* _<BS><CHARnotSPACE> */
		)    /* Yes; it is atleast  WU */
	    flags |= W_UND ;
	else if ( ( *s         == LUNDERBAR  ) &&
		  ( (*(s + 1)) == LBACKSPACE ) &&
		  ( iswspace(*(s + 2))     )      /* _<BS><SP> */
		)    /* Yes; CU */
	{
	    if ( env == LS_SPACE )
	    {   /* environment has changed; indicate zero len word */
		w->t_next = NULL;
		w->t_offset = NULL ;
		w->t_flags = W_0_LEN ;
		w->t_length = w->t_cflags = 0 ;
		w->t_padding = disp_blanks ;
		return( w ) ;
	    }
	    flags |= W_CONT ;
	    env |=  LS_UBS ;
	    /* we need to skip over these and increment the `blanks'
	     *  count also
	     */
	    while   ( ( *s         == LUNDERBAR  )  &&
		      ( (*(s + 1)) == LBACKSPACE )  &&
		      ( iswspace(*(s + 2))     )
		    ) /* at least one from above test */
	    {
						    /* counts as one `blank' */
		disp_blanks += ((len = wcwidth(*(s + 2))) < 0) ? 1 : len ;
		s += 3 ;    /* incr pointer correctly */
	    }
	}

	if ( ( env == LS_UBS )  && ( iswspace( *s ) || ( *s == LNUL ) ) )
	{   /* environment has changed; indicate zero len word */
	    w->t_next = NULL;
	    w->t_offset = NULL ;
	    w->t_flags = ( W_0_LEN | flags );
	    w->t_length = w->t_cflags = 0 ;
	    w->t_padding = disp_blanks ;
	    return( w ) ;
	}
	/* now go through the text */
	while ( ( (!iswspace( *s )) && (*s != LNUL) ) ||   /* while any kind of text */
		( iswspace( *s )  && ( *(s-1) == LBACKSLASH ) ) ) /* or \<SP> */
		/* this second test should really also modify the `length' of
		 *  word (one less, since \ doesn't count), but it really
		 *  doesn't matter. i.e ff will take care of it properly;
		 *  we just want to collect it.
		 */
	    *pwbuf++ = *s++ ;
	
	if ( iswspace(*s) && (*(s-1) == LBACKSPACE) && (*(s-2) == LUNDERBAR) )
	{   /* then we went two characters too far; current char is SP  */
	    s     -= 2 ;        /* backup s and pwbuf */
	    pwbuf -= 2 ;
	    /* `length' should be okay becuz of BS */
	}
    } /* main else */

    *pwbuf = LNUL ;  

    /* now set up stuff to return */

    w->t_offset  = newmbstr( wordbuf ) ;
    w->t_next    = NULL ;
    w->t_length  = wstr_disp_width(wordbuf);
    w->t_padding = disp_blanks ;
    w->t_flags   = flags ;
    debug0("\n proc_word(): exiting") ;
    return( w ) ;

} /* end routine proc_word_node() */


/***********************************************

WSTR_DISP_WIDTH

Returns the display width of wstr, taking backspaces into 
account. 

************************************************/

static int wstr_disp_width(wchar_t *ws)
{
  int idx = wcslen(ws);
  int total_width = 0;
  /* number of characters that has been wiped out by backspaces
     at a certain point in ws. */
  int chars_backspaced = 0; 

  while (--idx >= 0) {
     if(ws[idx] == L'\b') 
         chars_backspaced++;
     else {
         idx -= chars_backspaced;
         chars_backspaced = 0;
         if (idx >= 0)
	      total_width += ((len = wcwidth(ws[idx])) < 0) ? 1 : len ;
     }
  } 
  return total_width;
}


/* ========================= proc_bs_cmd() ===============================
 * routine to process a \ cmd node
 *  be careful to heed flags and update all relevant ptrs.
 * note that \\ and \<SPACE> don't constitute \ cmd beginnings and are
 *  handled elsewhere.
 *
 * =======================================================================
 */

 struct word *
proc_bs_cmd(void )
{
 FLAG_TYPE flags = 0;
 struct word  * w ;
 int    disp_blanks = 0 , /* width of blanks preceding word */
	brack_num;
 wchar_t   cmdbuf[ BUFSIZ ] ,
           * cmdp = cmdbuf ;
 
    debug1("\n proc_bs_cmd(): entered with <%S>; getting Word node...",s ) ;
    if ( tildeflag )
	debug1("\n proc_bs_cmd(): tildeflag==TRUE; nextline <%S>", nextlp ) ;
    w = getAnode( word ) ;
    flags |= W_C_BS ;

    /* this section has to be smart about grouping the commands properly;
     *  e.g. brackets, \", etc
     */
    while ( iswspace( *s ) )
    { 
	disp_blanks += ((len = wcwidth(*s)) < 0) ? 1 : len ;
	s++ ;
	if ( tildeflag )    nextlp ++ ;
	/* asking for trouble by underlining \ cmds */
    }

    *cmdp++ = *s++ ;    /* next char is \ so copy it */
    if ( tildeflag )    nextlp ++ ;

    if ( *s == LDOUBLEQUOTE )
    {   /* this is easy; just a comment; treat it as a composite word */
	while (*s != LNUL)    /* copy whole line */
	{
	    *cmdp++ = *s++ ;
	    /* tildeflag inconsequential here */
	}
	*cmdp = LNUL ;
	flags |= W_C_COMMENT ;
    }
    else /* not a comment */
    {
	if ( is_open_bracket( *s ) )
	{
	    brack_num = det_brack_type( *s ) ;  /* obtain parallel index */
	    /* copy everything upto matching close bracket of the given type */
	    /* we better have one !!! */
	    *cmdp++ = *s ;  /* want initial open-bracket */
	    if ( tildeflag )    nextlp ++ ;
	    do
	    {
		*cmdp++ = *++s ;  /* at least a close-bracket or a NUL */
		if ( tildeflag )    nextlp ++ ;
	    } while ( ( *s != close_brack_tab[ brack_num ] ) && 
                      (*s != LNUL) ) ;
	    /* we've got what we wanted ?? */
	    if ( *s == LNUL )
		;
	    else
		s ++ ;
	}
	else    /* assume text follows \ s just copy till no more; right ??  */
	{
	    while ( ( ! iswspace( *s ) ) && (*s != LNUL) )
	    {
		*cmdp++ = *s++ ;
		if ( tildeflag )    nextlp ++ ;
	    }
	}
	*cmdp = LNUL ;
	w->t_cmdix = det_cmd_type( cmdbuf ) ; /* hopefully this is smart enuf
					       * to skip over irrelevant stuff
					       */
    } /* else (not a comment)  */

    /* set up fields before returning */
    w->t_padding = disp_blanks ;
    w->t_cflags  = flags ;       /* command flags since command node */
    w->t_offset  = newmbstr( cmdbuf ) ;
    w->t_length  = wcswidth(cmdbuf, INT_MAX);
    if (w->t_length < 0)
	w->t_length = 0;
    /* default the others */
    w->t_next    = NULL ;
    w->t_flags   = 0 ;

    debug1("\n proc_bs_cmd(): exiting with <%S>", s) ;
    return( w ) ;

}   /* end  proc_bs_cmd( )  */

/* ========================= det_brack_type() ============================
 * routine to determine the closing bracket type of a given char
 *
 * =======================================================================
 */
 int
det_brack_type( wchar_t c )
{
 int    i = 0 ;

    debug1("\n det_brack_type(): entered with <%C>", c) ;
    while ( c != open_brack_tab[ i ] ) i++ ;
    /* what about bounds checking; no need since we already know we have a
     *  bracket; so proper termination is guaranteed.
     */
    debug2("\n det_brack_type(): exiting: i <%d> matching bracket <%c>",
	     i, close_brack_tab[ i ] ) ;
    return( i );
}   /* end det_brack_type( )    */


 boolean
is_open_bracket(wchar_t c )
{
 int    i = 0 ;

    while ( open_brack_tab[ i ]   /* != zero */ )
    {
	if ( c == open_brack_tab[ i ] )
	    return( TRUE ) ;
	i++ ;
    }
    return( FALSE ) ;
} /* end is_open_bracket()  */



/* ===========================  newmbstr  ================================
 * Takes a wchar_t string and converts it into a multibyte char string
 * which is stored on the heap. Returns a pointer to the multibyte char
 * string.
 *
 * Input:     wstr:  the wchar_t string to be converted. 
 *
 * Returns:   pointer to multi byte string.
 *
 * =======================================================================
 */

char *newmbstr(wchar_t *wstr)
{
  size_t len_wstr = wcslen(wstr) * MB_CUR_MAX + 1;
  char *str = malloc(len_wstr);

  (void) wcstombs(str, wstr, len_wstr);
  return(str);
}
  
 
