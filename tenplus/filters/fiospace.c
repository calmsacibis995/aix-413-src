#if !defined(lint)
static char sccsid[] = "@(#)67	1.5  src/tenplus/filters/fiospace.c, tenplus, tenplus411, GOLD410 7/18/91 12:38:27";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	getline, ungetstr, getnode, newstr, xalloc, xrealloc,
 *		freetree
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
 *  fiospace.c     -   Module to handle file input and space allocation.
 *                     for format
 *
 * =======================================================================
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <chardefs.h>
#include "fdefines.h"
#include "fexterns.h"
#include "filters.h"

 static char * peekline = NULL ; /* for the next line lookahead buffer */

/* ========================== getline() ==================================
 *
 *  This routine reads a line in and converts tabs to appropriate number of
 *      spaces and then returns the line in a dynamically allocated string.
 *      The caller had better release the space eventually.
 *  Caveat:
 *      Line may not be longer than currently available dynamic space.
 * =======================================================================
 */

 char *
getline(void)
{
 char * xrealloc(char * , int),
      * xalloc(int nbytes) ;
 extern FILE *filename(char *s);
 register char * l1, * u1, * savel1 ;    /* lower and upper limits for str */
 char * temps ;
 int c ;
 int i = 0 ;    /* to note how many chars we've gathered */

    debug0("\ngetline(): entered") ;
    if ( peekline /* != NULL */ )   /* then return the line previously read */
    {
	/* debug0("  peekline == NonNULL;") ;   */
	l1 = peekline ;
	peekline = NULL ;
	debug1(" peekline==NonNULL;  return line <%s>", l1 ) ;
	return( l1 ) ;
    }
    debug0("  peekline == NULL") ;
    /* check to see if eof occured last time around */
    if (input == NULL) return NULL;
    if ( ( c = getc( input ) )  != EOF )
	(void) ungetc( c, input ) ;    /* no */
    else
	return NULL ;           /* yes, return */

    /* get an initial chunk of space */
    savel1 = l1 = xalloc( LINESIZE ) ;
    debug1( "got initial chunk @%o\n ", savel1);
    u1 = l1 + LINESIZE - 1 ;        /* note upper limit; assure \0 space */

    while ( ( c = getc( input ) )  != NEWLINE )
    {
	debug1("%c ", c);
	if (c == EOF)
	{   (void)fclose (input);
	    if ((input = filename (0)) == NULL) break;
	    else continue;
	}
	/* now check to see if we have enuf space */
	if ( ( l1 + 8 ) > u1 )  /* if the lower limit == upper limit, get more */
				/* also see if expansion of tabs is possible */
				/* a little wasteful over-estimation */
	{
	    debug0( "trying for more");
	    if ( ( temps  = xrealloc( savel1, i + LINESIZE ) ) == NULL ) /* expand string */
	    {
		(void) fprintf(stderr,"\n*** line too long; breaking it ***\n") ;
		(void) ungetc( c, input ) ;
		*l1 = NUL ;
		return( savel1 ) ;
	    }
	    debug1( "got more @%o\n ", temps);
	    /* otherwise we got a new string;  note the new limits */
	    l1 = temps + ( l1 - savel1 ); /* make l1 point to same relative spot */
					  /* bracketing to prevent overflow */
	    u1 -= (int)savel1 ;  /* get number of bytes in previous string */
	    u1 += (int)temps + LINESIZE ;    /* get new upper limit */
	    savel1 = temps ;    /* and we're back in business; continue */
	}
	/* copy over expanding tabs to newlines till we get \n (remove it) */
	if ( c == TAB )
	{   /* expand tabs */
	    i ++ ;          /* at least one TAB */
	    *l1++ = SPACE ;
	    while ( i % 8 )
	    {
		i ++ ;
		*l1++ = SPACE ;
	    }
	}
	else
	{
	    i++ ;           /* count chars so we can expand tabs properly */
	    *l1++ = c ;
	}
    }
    i ++ ;
    *l1 = NUL ;     /* add terminating nul */
    debug2("\ngot a NEWLINE; shrinking to %d bytes from %d bytes\n",
	 i, u1-savel1 );
    if ( ( temps = xrealloc( savel1, i ) ) != NULL ) /* try to shrink it */
	return( temps ) ;   /* shrink succesful */
    return( savel1 ) ;      /* no space to shrink */
} /* end getline() */


/* ========================== ungetstr() ==================================
 *
 * take the str pointed to by `s' and "put it back."  Actually just make
 *  peekline point to the string so that getline can use this line the next
 *  time around.
 *
 * =======================================================================
 */

void ungetstr ( char *s )
{
    /* debug1("\n ungetstr(): ungetting  <%s>", s ) ;   */
    peekline = s ;

} /* end ungetstr */


/* ========================== getnode() ==================================
 *
 * This routine tries to allocate a node of size `size'.
 *
 * =======================================================================
 */
/*
 char *
getnode (int size )
{
 char * xalloc(int nbytes) ;

    return( xalloc( size ) ) ;

} * end getnode */


/* ========================== newstr() ===================================
 *
 * Routine allocates space for the string and copies it over.
 *
 * =======================================================================
 */
char *
newstr(char * str )
{
 char * xalloc(int nbytes) ;

    return(strcpy( xalloc( strlen( str ) + 1 ), str ) ) ;

} /* end newstr() */


/* ========================== xalloc() ==================================
 *
 * Routine to allocate nbytes (from calloc()).  If calloc() is out of
 *  space, it sees if there is a big-block that can be freed.  If there is
 *  no big-block, it dies.
 *
 * =======================================================================
 */

char *
xalloc(int nbytes )
{
 void *calloc(size_t, size_t);
 register char *n;

    if ( ( n = (char *)calloc( 1, (unsigned) nbytes )) == NULL)
    {
	/* no more space; see if there is a big-block available to free */
	if ( bigblock )
	{
	    free( bigblock ) ;
	    bigblock = NULL ;               /* don't have it anymore */
	    return( xalloc( nbytes ) ) ;    /* and try again */
	}
	else
	{
	    (void) fprintf(stderr, "\n*** xalloc(): no space; can't allocate %d bytes ***\n",
			nbytes ) ;
	    exit( EXITSPACE ) ;
	}
    }
    return n ;
} /* end xalloc() */


/* ========================== xrealloc() ================================
 *
 * Routine to re-allocate nbytes (from realloc()).  If realloc() is out of
 *  space, it sees if there is a big-block that can be freed.  If there is
 *  no big-block, it dies.
 *
 * =======================================================================
 */

 char *
xrealloc(char * ptr, int nbytes )
{
 register char *n;

    if ( ( n = (char *) realloc((void *) ptr, (unsigned) nbytes ) ) == NULL )
    /* expand string */
    {
	/* no more space; see if there is a big-block available to free */
	if ( bigblock )
	{
	    free( bigblock ) ;
	    bigblock = NULL ;                   /* don't have it anymore */
	    return( xrealloc( ptr, nbytes ) ) ; /* and try again */
	}
	else
	{
	    (void) fprintf(stderr, "\n*** xrealloc(): no space; can't reallocate %d bytes ***\n",
			nbytes ) ;
	    exit( EXITSPACE ) ;
	}
    }
    return n ;
} /* end xrealloc() */

/* ========================== freetree() ================================
 *
 * Routine to free the dynamically allocated space used by the tree.
 *  do what?
 *  well, for each word in the line: free the dynamic string space, then
 *       free that node taking care not to lose the other nodes in the string
 *  then free the line
 *  then when all the lines are freed, DON'T free the tree's root node
 *
 * If a line has the L_NO_KCUF flag on it; don't free it (should be last line)
 *  If there is such a line, make the head and tail point to it and update
 *  the nrlines field to 1.
 *
 * =======================================================================
 */

void freetree( struct text *t )
{
 struct line * l, * l2 ;
 struct word * w, * w2 ;

    debug0("\n freetree(): entered ") ;
    l = t->t_head ;
    while ( l )
    {
	if ( l->t_flags & L_NO_KCUF )   /* this line is the last line */
	{   /* hope the parser removes this flag and tries to get bigblock */
	    t->t_head = t->t_tail = l ;
	    t->t_nrlines = 1 ;
	    return ;
	}
	debug0("\n ...freeing line @: ") ;
	debug1("\n\t <%o>", l ) ;
	l2 = l->t_nextline ;
	if ( l->t_flags & L_EMPTY )
	{
	    free((char *) l ) ;        /* line is empty; free it */
	    debug0(" line is empty\n ") ;
	}
	else
	{
	    debug0("\n\t\t freeing offset @\t freeing word @") ;
	    w = l->t_next ;
	    while( w )
	    {
		w2 = w->t_next ;
		debug1("\n\t\t <%s>", w->t_offset ) ;
		if ( w->t_offset )          /* free the string */
		    free( w->t_offset ) ;
		debug1("\t\t\t <%o>", w ) ;
		free( (char *) w ) ;        /* free the word */
		w = w2 ;
	    }
	    free( (char *) l ) ;            /* free the line */
	}
	l = l2 ;
    } /* while */
    /* don't free the tree top */
    t->t_head = t->t_tail = NULL ;
    t->t_nrlines = 0 ;

    debug0("\n freetree(): exiting") ;

} /* end freetree() */
