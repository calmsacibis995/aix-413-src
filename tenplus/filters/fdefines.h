/* @(#)72	1.5  src/tenplus/filters/fdefines.h, tenplus, tenplus411, GOLD410 7/18/91 12:38:00 */

/*
 * COMPONENT_NAME: (INED) INed Editor
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
 *  fdefines.h      -   Definitions and pre-processor information for format
 *
 * =======================================================================
 */

#include <chardefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

extern char *xalloc(int nbytes);

/* random limits */
#define LINESIZE     (81 * MB_LEN_MAX)
#define CMDSIZ       (81 * MB_LEN_MAX)

#define BIGBLKSIZ    2 * BUFSIZ /* the amount of buy-time core to get */


#define DEF_LMARG   0           /* default margins   */
#define DEF_RMARG   65
#define MIN_LMARG   0
#define MAX_RMARG   130

/* Some convenient definitions for upward compatibility and space crunch */
#define FLAGTYPE    char    /* either `char' or `unsigned int' (8/16 bits) */
#define FLAG_TYPE   char    /* either `char' or `unsigned int' (8/16 bits) */
#define boolean     char    /* really need a bit */

/* Some constants */
#define DOT         '.'
#define PERIOD      '.'
#define QUESTION    '?'
#define EXCLAMATION '!'
#define COLON       ':'
#define TILDE       '~'
#define UNDERBAR    '_'
#define TAB         '\t'
#define BACKSPACE   '\b'
#define BACKSLASH   '\\'
#define DOUBLEQUOTE '\"'
#define NEWLINE     '\n'
#define NUL         '\0'

/*  and their wchar_t equivalents */
#define LDOT         L'.'
#define LPERIOD      L'.'
#define LQUESTION    L'?'
#define LEXCLAMATION L'!'
#define LCOLON       L':'
#define LTILDE       L'~'
#define LUNDERBAR    L'_'
#define LTAB         L'\t'
#define LBACKSPACE   L'\b'
#define LBACKSLASH   L'\\'
#define LDOUBLEQUOTE L'\"'
#define LNEWLINE     L'\n'
#define LNUL         L'\0'

/* FLAGS   for the line and word nodes
   For word nodes that are not commands but ordinary text: w->t_flags */

#define W_TILDE     0x01    /* This word used to be tilde'd in the input   */
#define W_CONT      0x02    /* This word is continuosly underlined         */
#define W_UND       0x04    /* This word is word underlined                */
#define W_0_LEN     0x80    /* zero length word */

    /* legal combinations of above flags
     *  1)  (W_TILDE | W_CONT)  This word was CU using tildes.
     *  2)  (W_TILDE | W_UND)   This word was WU using tildes.
     *  3)  (W_CONT)            This word was CU.
     *  4)  (W_UND)             This word was WU.
     *
     * Currently a hyphen does not delimit a word; the are part of the
     *  current word definition:
     *  <word>        ::= <any char that is not white space or NUL>
     *  <white space> ::= <blank>* | <newline>* | <tab>* | <white space>
     *                      but not \<SPACE>
     *
     *  '\' and '-' are currently part of words
     *  The above hyphen combinations are included as a guide to future
     *  upgrades.
     */

/* For word nodes that are commands: w->t_cflags   */
#define W_C_BS      0x01    /* This is a \ cmd                             */
#define W_C_DOT     0x02    /* This is a dotted cmd                        */
#define W_C_COMMENT 0x04    /* This is a \" cmd ( a comment ); who uses it?*/
			    /* This is not in DIREX. Check explicitly in   *
			     * parse.c or PUT IT IN                        *
			     */

/* For line nodes:  l->t_flags  */
#define L_TILDE     0x01    /* This line used to have tildes assoc. w/it   */
#define L_DOT_CMD   0x02    /* This line is a dotted cmd                   */
#define L_BS_CMD    0x04    /* This line has a \ cmd node in it            */
#define L_EMPTY     0x08    /* This line is empty                          */
#define L_NO_KCUF   0x10    /* Don't mess with this line                   */
#define L_DOTTED    0x20    /* Treat this line as though it was empty.
                               Do not output it. All lines starting
                               with dot get this flag the first time
                               they are processed. */


/* flags used by the parser for determine what action to perform on tree */
#define F_BREAK     0x01    /* This directive causes a break               */
#define F_IGNORE    0x02    /* This directive is ignored                   */
#define F_JUST      0x04    /* This directive causes just() to be invoked  */
			    /*  (only in principle. only fill() is invoked */
#define F_FILL      0x08    /* This directive causes fill() to be invoked  */


/* flags used to note the environment of the preceding spaces in lexer.c*/
#define LS_SPACE    0x01    /* In a normal space environment */
#define LS_UBS      0x02    /* In a _<BS><SP> env */

/*  Some macros */
#define getAnode(type)  (struct type *) xalloc( sizeof( struct type ))
#define is_sent_end(c)  ( ( (c) == DOT   )  || ( (c) == EXCLAMATION ) ||  \
			  ( (c) == COLON )  || ( (c) == QUESTION    ) )

/* Debug macros */
#ifdef DEBUG

#define debug(  fmt )                       (void) fprintf( stderr, fmt )
#define debug0( fmt )                       (void) fprintf( stderr, fmt )
#define debug1( fmt, arg1 )                 (void) fprintf( stderr, fmt, arg1 )
#define debug2( fmt, arg1, arg2 )           (void) fprintf( stderr, fmt, arg1, arg2 )
#define debug3( fmt, arg1, arg2, arg3 )     (void) fprintf( stderr, fmt, arg1, arg2, arg3 )
#define debugline( l )                      printline( l )

#else

#define debug(  fmt )                       /* NULL */
#define debug0( fmt )                       /* NULL */
#define debug1( fmt, arg1 )                 /* NULL */
#define debug2( fmt, arg1, arg2 )           /* NULL */
#define debug3( fmt, arg1, arg2, arg3 )     /* NULL */
#define debugline( l )                      /* NULL */

#endif

/* define the return values */
/* output generated */
#define EXITOK    (0)
#define EXITWARN  (1)    /* warning condition (1 - 037) */
#define EXITERROR (040)  /* error occurred (040 - 077) */
#define EXITFATAL (0100) /* fatal error occurred(0100-0177) */

/* specific errors */
#define EXITEXEC  (255)  /* exec failed */
#define EXITBAD   (254)  /* bad parameters, unopenable user file, etc*/
#define EXITIO    (253)  /* io error occurred */
#define EXITFILE  (252)  /* system file missing or in error */
#define EXITSIGNAL (251) /* signal received */
#define EXITSPACE (250)  /* program ran out of memory */
#define EXITOTHER (128)  /* program-specific error conditions (128+)*/

