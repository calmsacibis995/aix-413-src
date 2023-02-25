#if !defined(lint)
static char sccsid[] = "@(#)66	1.5  src/tenplus/filters/fcommand.c, tenplus, tenplus411, GOLD410 7/18/91 12:37:54";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	cmd_line, filename
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
 *  fcommand.c      -   Command line argument processor for format
 *
 * =======================================================================
 */

#include "fdefines.h"
#include "fexterns.h"
#include "filters.h"

FILE    * input ,
	* fopen(const char *filename, const char *mode) ;

int  r_marg = DEF_RMARG ,                             /* right margin value */
     l_marg = DEF_LMARG ;                              /* left margin value */
char default_action = 'f' ;     /* j for just; f for fill; c for center (??)*/

extern int 	atoi(const char *);
extern void	exit(int);
extern void	*malloc(size_t);

/* =========================== cmd_line() ==============================
 *
 *  This routine processes the command line arguments, setting the various
 *      flags and variables.
 *  The possible command line arguments are:
 *  --  -l<number>  set the default left margin to be #
 *  --  -r<number>  set the default right margin to be #
 *  --  -d<char>    set the default action variable where <char> specifies:
 *                      `c' : center    ; as yet unimplemented
 *                      `j' : justify
 *                      `f' : fill
 *  --  string      input filename; else stdin
 *
 * =====================================================================
 */
void cmd_line( int argc, char **argv )          /* that received at main() */
{
    int i ;
    register char *s;
    extern FILE *filename(char *s);
    char *infile = NULL ;

    for( i = 1; i < argc; i++ )
    {
	s = argv[i];
	if (*s == '-')
	    switch ( s[1] )
	    {
		case 'l':
		case 'L':
		    if ( isdigit( *(s + 2) ) )
		    {
			l_marg = atoi( s+2 ) - 1 ;  /* one based: kludge */
			if ( l_marg < MIN_LMARG )
			{
			    (void) fprintf(stderr, "Left margin too low: (%d)\n", l_marg ) ;
			    exit( EXITBAD );
			}
		    }
		    else
		    {
			(void) fprintf(stderr, "Bad left margin parameter: (%s)\n", s+2 ) ;
			exit( EXITBAD );
		    }
		    break;

		case 'r':
		case 'R':
		    if ( isdigit( *(s + 2) ) )
		    {
			r_marg = atoi( s+2 );
			if ( r_marg > MAX_RMARG )
			{
			    (void) fprintf( stderr, "Right margin too high: (%d)\n", r_marg ) ;
			    exit( EXITBAD );
			}
		    }
		    else
		    {
			(void) fprintf( stderr, "Bad right margin parameter: (%s)\n", s+2 ) ;
			exit( EXITBAD );
		    }
		    break;

		case 'd':
		case 'D':
		    /* if ( s[2] != 'j' &&  s[2] != 'c' && s[2] != 'f' )*/
		    if ( isupper( s[2] ) )
			s[2] = tolower( s[2] ) ;
		    if ( s[2] != 'j' && s[2] != 'f' )
		    {
			(void) fprintf( stderr, "Bad default action specifier (%c)\n", s[2] ) ;
			exit( EXITBAD );
		    }
		    default_action = s[ 2 ] ;
		    break;

		default :
		    (void) fprintf( stderr, "Unknown argument (%s); ignored.\n", *s ) ;
	    } /*  switch{} */
	else    /* assume it's a string for a file-name */
	    if ( infile )
		(void) filename (s);
	    else
		infile = s ;

    } /* end `for' (through arguments) */

    if ( l_marg + 1 >= r_marg )
    {
	(void) fprintf( stderr, "Left margin greater-than or equal-to right margin\n" ) ;
	exit( EXITBAD );
    }
    if ( infile )
	input = fopen( infile, "r" );
    else
	input = stdin;

    if (input == NULL)
    {
	(void) fprintf(stderr, "Unable to open input file (%s)\n", infile);
	exit( EXITBAD );
    }

} /* end cmd_line() */


/*========================filename()============================
 *
 *  queues up input file names during argument processing
 *  and unqueues them during file processing.  Remembers
 *  file name if given one, returns FILE pointer of next
 *  if not.
 *
 *==============================================================
 */
FILE *
filename(char *s)
{   struct listnode
    {   struct listnode *nextel;
	char *listval;
    };
    static struct listnode *filelist = NULL, *endlist = NULL;
    FILE *answer;

    answer = NULL;
    if (s)
    {   if (filelist)
	{   endlist->nextel = (struct listnode *)
		malloc (sizeof (struct listnode));
	    endlist->nextel->listval = s;
	    endlist = endlist->nextel;
	    endlist->nextel = NULL;
	}
	else
	{   filelist = endlist = (struct listnode *)
		malloc (sizeof (struct listnode));
	    endlist->nextel = NULL;
	    endlist->listval = s;
	}
    }
    else while (filelist != NULL && answer == NULL)
    {   answer = fopen (filelist->listval, "r");
	if (answer == NULL) (void) fprintf (stderr,
		"Unable to open input file %s\n",filelist->listval);
	filelist = filelist->nextel;
	if (filelist == NULL) endlist = NULL;
    }
    return answer;
}   /* end of  filename() */
