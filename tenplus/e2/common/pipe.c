#if !defined(lint)
static char sccsid[] = "@(#)62	1.7  src/tenplus/e2/common/pipe.c, tenplus, tenplus411, GOLD410 3/23/93 11:52:57";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	dolocal, doremote, dosetup, eatarg, eatint, putanswer,
 *		rstsigs, savesigs
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
/* File: pipe.c - deals with calling functions across a Unix pipe.          */
/****************************************************************************/

#ifdef PIPEHELP
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "pipe.h"
#include "pipe_inc.h"
#include <libobj.h>




static FILE *ifp, *ofp;     /* pipe FILE objects                        */
static void (*intsig)(int);     /* place to save state of SIGINT            */
static char *argptr;        /* pointer into the current arg list        */
extern LOCALSTRUCT g_locals [];   /* definitions of local functions     */
extern REMOTESTRUCT g_remotes []; /* definitions of remote functions    */
extern int g_nlocals;       /* number of local functions - 1            */
extern struct helper g_hep;

#define LOCAL

LOCAL int dolocal (void);
LOCAL char *eatarg (void);
LOCAL int eatint (void);
LOCAL void putanswer (char *, char );
LOCAL void rstsigs (void);
LOCAL void savesigs (void);
void errx(void);
/****************************************************************************/
/* dolocal: execute a function here for our partner.                        */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - RET_OK or ERROR on communication failure   */
/*                                                                          */
/* globals referenced:     ifp, g_nlocals, g_locals, g_hep                  */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     dolocal reads a 1 byte code from the input pipe specifying           */
/*     the index of the function in g_locals to invoke.  If it is           */
/*     not a valid code, return ERROR.  Otherwise look up the               */
/*     form of the function's arguments in g_locals and use eatint          */
/*     and eatarg to read the argument values off the pipe.                 */
/*                                                                          */
/*     Now that we have all necessary information on this side of           */
/*     the pipe, invoke the function.  If it did not "suicide" by           */
/*     clearing g_hep.procs, write its return value on the pipe.            */
/*     In either case, return RET_OK to the caller in this process.         */
/****************************************************************************/

LOCAL int dolocal (void)
    {
    register LOCALSTRUCT *lp;
    register char *rv;
    char code;
    register POINTER *freelist;
    register char *args;
    char *a [7];

    savesigs ();
    code = fgetc (ifp);
    rstsigs ();

    debug ("dolocal (%d):", ((int) code) & 0xff);

    /* a little checking, anyway                                */

    if (code < 2 || code > g_nlocals || feof (ifp))
	{
	char abuf [50];
	debug ("helper check: code %d", code);
	if ((int)code != -1)
	    debug ("'%s'",
		fgetline (ifp, abuf, sizeof (abuf) - 1) == EOF ? "EOF" : abuf);
	errx ();
	return (ERROR);
	}
    lp = &g_locals [code];
    args = l_args (lp);
    freelist = (POINTER *) s_alloc (0, T_POINTER, NULL);

    /* set up the shared argument pointer to aim at a   */
    argptr = (char *) a;

    /* loop setting up the arguments                    */

    while (args && *args)
	{
	if (*args++ != A_INT)
	    {
	    rv = eatarg ();

	    if (rv == (char *) ERROR)
		{
		errx ();
		s_free ((char *) freelist);
		return (ERROR);
		}
	    freelist = s_append (freelist, rv);
	    }
	else
	    if (eatint () == ERROR)
		{
		s_free ((char *) freelist);
		return (ERROR);
		}
	}

    /* now that the args are in place in a, call the function   */

    rv = (char *)
	    (*(lp->l__function)) (a[0], a[1], a[2], a[3], a[4], a[5], a[6]);

    /* and return what it returned      */
    /* except for suicide functions     */

    if (g_hep.proc)
	putanswer (rv, l_rv (lp));

    /* throw away the argument list     */
    s_free ((char *) freelist);
    return (RET_OK);
    }

/****************************************************************************/
/* doremote: ask partner to execute function for us.                        */
/*                                                                          */
/* arguments:              int  code   - specifying which function to call  */
/*                         va_dcl      - varargs declaration                */
/*                                                                          */
/* return value:           char * - return value of the function,           */
/*                                  or ERROR                                */
/*                                                                          */
/* globals referenced:     g_hep, g_remotes, ofp, ifp                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     doremote executes the function specified by the given code           */
/*     by sending it over the pipe, with its arguments taken from           */
/*     the given target.  It looks up the argument specification            */
/*     in the g_remotes table, and then writes the function and             */
/*     its arguments on the output pipe (using s_write).                    */
/*                                                                          */
/*     It then listens to the input pipe to get the result of the           */
/*     remote call.  If the code that comes back is neither A_RETURN        */
/*     nor A_ERROR, it is a function code that the other side of            */
/*     the pipe wants executed on this side in order to determine           */
/*     the return value for the function we asked it to execute.            */
/*     Thus, doremote invokes dolocal for each function code that           */
/*     appears on the pipe.  When the code indicates the remote             */
/*     function is ready to return an answer, doremote uses s_read          */
/*     to read the answer and returns it to its own caller.                 */
/****************************************************************************/

char *doremote (int code, ...)
    {
    register char *rv;
    char *junk;
    register char *args;
    register REMOTESTRUCT *rp;
    register long *lp;
    register char *object;
    register POINTER *objptr;
    register va_list vargptr; /* pointer into current argument list */

    if (g_hep.proc == 0)
	return (0);

    rp = &g_remotes [code];
    args = r_args (rp);
    va_start (vargptr, code);

    debug ("doremote (%d):", ((int) code) & 0xff);

    savesigs ();
    (void) fputc (code, ofp);
    rstsigs ();

    while (args && *args)
	{
	switch (*args++)
	    {
	case A_INT:
	    /* put int out as a structured long */
	    lp = (long *) s_alloc (1, T_RECORD, NULL);
	    *lp = (long) (va_arg (vargptr, int));

# ifdef PIPETRACE
    debug ("\trmtarg %ld", *lp);
# endif

	    savesigs ();
	    if (s_write ((char *) lp, ofp) == ERROR)
		errx ();
	    rstsigs ();
	    s_free ((char *) lp);

	    break;

	case A_CHAR:
	    /* put character string out as structured character array */
	    object = s_string (va_arg (vargptr, POINTER));

# ifdef PIPETRACE
    debug ("\trmtarg '%s'", object);
# endif

	    savesigs ();
	    if (s_write (object, ofp) == ERROR)
		errx ();
	    rstsigs ();
	    s_free (object);

	    break;

	case A_OBJ:
	    objptr = va_arg (vargptr, POINTER *);
	    savesigs ();
	    if (s_write ((char *)objptr, (FILE *)ofp) == ERROR)
		errx ();
	    rstsigs ();

	    break;
	    }
	}

    va_end (vargptr);

    /* as soon as the last argument is written, the other side should
       swing into action                                                */
    savesigs ();
    (void) fflush (ofp);
    rstsigs ();

    /* now loop executing functions forever                             */

    while (1)
	{
	savesigs ();
	code = fgetc (ifp);
	rstsigs ();

	if (code == A_RETURN || code == A_ERROR)
	    break;
	(void) ungetc (code, ifp);

	/* if the local function was a suicide attempt          */
	if ((dolocal () == ERROR) || (g_hep.proc == 0))
	    return (0);
	}

    if (code == A_ERROR)
	return ((char *) ERROR);

    /* return what the remote function returned */

    savesigs ();
    rv = s_read (ifp);
    rstsigs ();

    if (r_rv (rp) == A_INT)
	{
	junk = (char *) *((long *) rv);
	s_free (rv);
# ifdef PIPETRACE
	debug ("remote returns %d", junk);
# endif
	return ((char *) junk);
	}
    else
	{
# ifdef PIPETRACE
	if (rv == NULL)
	    debug ("remote returns NULL");
	else
	    debug ("remote returns '%s'",
		   obj_type (rv) == T_CHAR ? rv : "POINTER");
# endif
	return (rv);
	}
    }

/****************************************************************************/
/* dosetup: sets up the stdio objects for pipe protocol                     */
/*                                                                          */
/* arguments:              FILE *to, from - pipe file descriptors           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     ifp, ofp                                         */
/*                                                                          */
/* globals changed:        ifp, ofp                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     dosetup sets the characteristics of ifp and ofp so that they         */
/*     will behave properly in the pipe protocol.                           */
/****************************************************************************/

void dosetup (FILE *to, FILE *from)
    {
    ifp = from;
    ofp = to;
    }

/****************************************************************************/
/* eatarg: get an object from pipe into argument vector                     */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL char * - the argument eaten, or ERROR      */
/*                                                                          */
/* globals referenced:     ifp, argptr                                      */
/*                                                                          */
/* globals changed:        argptr                                           */
/*                                                                          */
/* synopsis:                                                                */
/*     eatarg uses s_read to read the next object from the input pipe.      */
/*     If it sees an error, it returns error.  Otherwise it makes the       */
/*     global argptr point to the object read off the pipe, increments      */
/*     argptr by the size of a pointer, and returns the object.             */
/****************************************************************************/

LOCAL char *eatarg (void)
    {
    register char *object;

    savesigs ();
    object = s_read (ifp);
    rstsigs ();

    if (object == (char *) ERROR)
	return ((char *) ERROR);

    *((POINTER *)argptr) = object;

# ifdef PIPETRACE
    if (object == NULL)
	debug ("\targ NULL");
    else
	debug ("\targ '%s'", obj_type (object) == T_CHAR ? object : "POINTER");
# endif

    argptr += sizeof (POINTER);
    return (object);
    }

/****************************************************************************/
/* eatint: get an integer (actually, T_RECORD) from pipe                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL int (RET_OK or ERROR)                      */
/*                                                                          */
/* globals referenced:     ifp, argptr                                      */
/*                                                                          */
/* globals changed:        argptr                                           */
/*                                                                          */
/* synopsis:                                                                */
/*     eatint eats an integer from the pipe and puts it into the argument   */
/*     vector.  The actual form of the integer on the pipe is a structured  */
/*     T_RECORD object, whose value is put into the global argument vector. */
/*     The argument vector pointer argptr is incremented, and the T_RECORD  */
/*     object is freed.                                                     */
/****************************************************************************/

LOCAL int eatint (void)
    {
    register long *object;

    savesigs ();
    object = (long *) s_read (ifp);
    rstsigs ();

    if (object == (long *) ERROR)
        {
#ifdef DEBUG
	debug ("eatint:  s_read returned ERROR, g_errno = %d\n", g_errno);
#endif
	errx();
	return (ERROR);
	}

    *((int *) argptr) = *object;

# ifdef PIPETRACE
    debug ("\tintarg %d", *((int *) argptr));
# endif

    argptr += sizeof (int);
    s_free ((char *) object);
    return (RET_OK);
    }

/****************************************************************************/
/* putanswer: put a local return value onto pipe for partner                */
/*                                                                          */
/* arguments:              char *value - the return value to put on pipe    */
/*                         char  type  - code specifying type of answer     */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     ofp                                              */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given value is ERROR or the given type is NULL, write         */
/*     an A_ERROR code onto the output pipe.  Otherwise write an            */
/*     A_RETURN code on the pipe and prepare to send the value.             */
/*     If the value is an integer, convert it into a T_RECORD object.       */
/*     Use s_write to write the value.                                      */
/****************************************************************************/

LOCAL void putanswer (char *value, char type)
    {
    register long *lp;

    /* functions that return "nothing" secretly return ERROR  */

    savesigs ();

    if (value == (char *) ERROR || type == '\0')
	{

# ifdef PIPETRACE
	debug ("\treturns ERROR");
# endif

	(void) fputc (A_ERROR, ofp);
	}
    else
	{
	(void) fputc (A_RETURN, ofp);
	if (type == A_INT)
	    {
	    lp = (long *) s_alloc (1, T_RECORD, NULL);
	    *lp = (long) value;

# ifdef PIPETRACE
	    debug ("\treturns %ld", *lp);
# endif

	    value = (char *) lp;
	    }

# ifdef PIPETRACE
	else
	    {
	    if (value == NULL)
		debug ("\treturns NULL");
	    else
		debug ("\treturns '%s'",
		       obj_type (value) == T_CHAR ? value : "POINTER");
	    }
# endif

	if (s_write (value, ofp) == ERROR)
	    errx ();
	s_free (value);
	}
    (void) fflush (ofp);
    rstsigs ();
    }

/****************************************************************************/
/* rstsigs: restore state of signals after pipe dialogue                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     intsig                                           */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     The SIGINT signal is restored to whatever the intsig global holds.   */
/****************************************************************************/

LOCAL void rstsigs (void)
    {
    (void) signal (SIGINT, intsig);
    }


/****************************************************************************/
/* savesigs: save partner process signal-state                              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           LOCAL void                                       */
/*                                                                          */
/* globals referenced:     intsig                                           */
/*                                                                          */
/* globals changed:        intsig                                           */
/*                                                                          */
/* synopsis:                                                                */
/*     The intsig global is set to whatever currently catches the           */
/*     SIGINT signal, and the SIGINT signal is set to be ignored.           */
/****************************************************************************/

LOCAL void savesigs (void)
    {
      /* cast the return value from signal into a "pointer to routine */
      /* that returns void" */
    intsig =  signal (SIGINT, SIG_IGN);
    }
#endif
