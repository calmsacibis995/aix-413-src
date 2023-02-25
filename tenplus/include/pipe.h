/*NOTUSED*/
/* @(#)93	1.6  src/tenplus/include/pipe.h, tenplus, tenplus411, GOLD410 3/23/93 12:02:17 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
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
 
/*****************************************************************************
* File: pipe.h                                                               *
*****************************************************************************/

#if PIPEHELP || BLTHELP
typedef struct local LOCALSTRUCT;

struct local
    {
    char *(*l__function)(char *, char *, char *, char *, char *, char *, char *); 
    char *l__args;
    char l__rv;
    };

#define l_functions(lp) ((lp)->l__functions)
#define l_args(lp) ((lp)->l__args)
#define l_rv(lp) ((lp)->l__rv)

typedef struct remote REMOTESTRUCT;

struct remote
    {
    char *r__args;
    char r__rv;
    };

#define r_args(lp) ((lp)->r__args)
#define r_rv(lp) ((lp)->r__rv)

/* types for arguments and return values        */

#define A_INT   1
#define A_CHAR  2
#define A_OBJ   3

/* character codes for saying "return the following", and
   "return ERROR"                                               */

#define A_RETURN    0
#define A_ERROR     1

#ifdef PIPEHELP
extern char *doremote (int, ...);
#endif

struct helper
    {
#ifdef PIPEHELP
    int proc;
    FILE *inp; 
		FILE *outp;
#endif
#ifdef BLTHELP
    int builtin;
#endif
    };
#endif /* endif PIPEHELP || BLTHELP */
