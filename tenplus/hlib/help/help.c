#if !defined(lint)
static char sccsid[] = "@(#)75	1.7  src/tenplus/hlib/help/help.c, tenplus, tenplus411, GOLD410 3/23/93 12:01:18";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	errx, main, Eask, Econfirm, Eerror, Efatal, Emessage,
 *		makelist, fatal
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
 

/* File: help.c                                                             */
/*                                                                          */
/* helper interface routines (helper side)                                  */
/*                                                                          */
/*   NOTES:                                                                 */
/*                                                                          */
/*   All helper callable editor functions have corresponding routines in    */
/*   the editor that start with "E", such as "Eerror".  Since the current   */
/*   pipe protocol (funp) does not allow POINTER arrays or routines that    */
/*   return either a string or ERROR (an int), both the editor and helper   */
/*   have interface special interface routines.  The editor interface       */
/*   routines start with "I", and the helper routines start with "E".  This */
/*   is done so that when the helpers are built into the editor (without    */
/*   their "E" routines) they can use the editor "E" routines directly,     */
/*   instead of having to go through the pipe protocol conversions.         */
/*                                                                          */
/*   The ask, getprefix, getsuffix and gettext functions all return either  */
/*   string or ERROR.  The editor has both "E" and "I" routines for all of  */
/*   them.  The "E" routine matches the protocol and the "I" routine in the */
/*   editor converts the ERROR integer into a special error string.  On the */
/*   helper side, there is also a special "E" routine which converts the    */
/*   error string back to ERROR.                                            */
/*                                                                          */
/*   Similarly, the getlist routine returns a pointer array (or ERROR).     */
/*   The Egetlist routine in the editor does that, and there are Igetlist   */
/*   in the editor and Egetlist is the helper to get it through the pipe.   */
/*                                                                          */
/*   The putlist and menu routines pass POINTER arrays to the editor.  The  */
/*   Eputlist and Emenu routines in the editor takes POINTER arrays, while  */
/*   the Igetlist and Imenu routines take string versions of the POINTER    */
/*   arrays which are produced by the Eputlist and Emenu routines in the    */
/*   helper.                                                                */
/*                                                                          */
/*   The error, fbhelp, fatal and message routines have helper interface    */
/*   routines for converting their printf type arguments into a single      */
/*   string which is required by the pipe protocol.                         */
/****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <limits.h>
#include <stdlib.h>
#include <locale.h>

#include <database.h>
#include <libhelp.h>
#include <edexterns.h>
#include <libshort.h>
#include <libobj.h>

static char g_buffer [2000 * MB_LEN_MAX];

int MultibyteCodeSet;


/****************************************************************************/
/* errx:  pipe protocol error handler, helper side                          */
/*                                                                          */
/*     When the helper gets an illegal code across the pipe or when         */
/*     a read on the pipe returns EOF, call Hstop to do anything            */
/*     that might be needed and exit cleanly.  This is how helpers          */
/*     die when the editor exits.                                           */
/****************************************************************************/

void errx (void)
    {
    Hstop ();
    }

/****************************************************************************/
/* main:                                                                    */
/****************************************************************************/

/* this isn't needed on the helper side, but is included for 
   compatibility with the editor.  Doremote tests hep.proc to
   decide if the helper has been stopped, because the editor
   wants it to return 0 in that case.                           */

struct
    {
    int proc;
    FILE *inp, *outp;
    }
    g_hep = {1, stdin, stdout};

void main ()

    {

    (void) setlocale(LC_ALL, "");
    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    /* trap the SIGPIPE and SIGHUP signals to Hstop so the helper can
       perform any desired analysis before being killed */

       (void) signal (SIGPIPE, (void(*)(int))Hstop);
       (void) signal (SIGHUP,  (void(*)(int))Hstop);

    /* set g_debugfp to NULL here so that the default setting to stderr
       in the object library is overridden.  The helper may, of course,
       open it to some real debug output file in its Hinit. */

    g_debugfp = NULL;

    /* opening the debug file here, as opposed to in any specific */
    /* helper program's Hinit routine, allows debugging of the initial */
    /* call to Hinit.  It also causes ALL the helpers to produce */
    /* debugging output. */
#ifdef	DEBUG_ALL_HELPERS
    g_debugfp = fopen( "helpers.trc", "a" );
    debug ("helper main routine");
#endif

    /* set the full-disk function to point to the Helpers version */
    g_diskfcn = (int(*)(char *, char *))Ediskfull;

    i_addtype ();  /* add T_ATTR type */

    dosetup (stdout, stdin);

    while (1)
	dolocal ();
    }

/****************************************************************************/
/* Eask:  interface to the Iask routine                                     */
/****************************************************************************/

char *Eask (int msg_no, char *format, ...)
     /* editor help information code */
     /* printf type format string    */
    /*  char *arg1;      extra arguments for sprintf  */
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    format = Egetmessage(msg_no, format, FALSE);

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    (void) sprintf (g_buffer, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);


    return (Iask (msg_no, g_buffer));
    }

/****************************************************************************/
/* Econfirm:  interface to the Iconfirm routine                             */
/****************************************************************************/

int Econfirm (int msg_no, char *format, ...)
    /*int   msg_no;  editor help information code */
    /*char *format;    printf type format string    */
    /*char *arg1;      extra arguments for sprintf  */
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    format = Egetmessage(msg_no, format, FALSE);

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    (void) sprintf (g_buffer, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);


    return (Iconfirm (msg_no, g_buffer));
    }

/****************************************************************************/
/* Eerror:  interface to the Ierror routine                                 */
/****************************************************************************/

void Eerror (int msg_no, char *format, ...)
    /*char *arg1;      extra arguments for sprintf  */

    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    format = Egetmessage(msg_no, format, FALSE);

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    (void) sprintf (g_buffer, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);


    Ierror (msg_no, g_buffer);
    }

/****************************************************************************/
/* Efatal:  interface to the Ierror and Ekill routines                      */
/****************************************************************************/

void Efatal (int msg_no, char *format, ...)
/* editor help information code */
/* printf type format string    */
/* extra arguments for sprintf  */
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    format = Egetmessage (msg_no, format, FALSE);

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    (void) sprintf (g_buffer, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);


    Ifatal (msg_no, g_buffer);
    }

/****************************************************************************/
/* Emessage:  interface to the Imessage routine                             */
/****************************************************************************/

void Emessage (int msg_no, char *format, ...)
    /* editor help information code */
    /* printf type format string   */
    /* extra arguments for sprintf */

    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    format = Egetmessage (msg_no, format, FALSE);

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    (void) sprintf (g_buffer, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);

    Imessage (msg_no, g_buffer);
    }

/****************************************************************************/
/* makelist:  converts array of strings to a POINTER array.                 */
/* The input array should end in a NULL pointer                             */
/* Makes copies of all the strings                                          */
/****************************************************************************/

POINTER *makelist (char **array)

    {
    register i;
    register POINTER *list;

#ifdef DEBUG
    debug ("makelist:  array = 0%o", array);
#endif

    for (i = 0; array [i]; i++) /* get length of list */
	;

    list = (POINTER *) s_alloc (i, T_POINTER, NULL);

    for (i = 0; array [i]; i++) /* get length of list */
	list [i] = s_string (array [i]);

    return (list);
    }

/****************************************************************************/
/* fatal: helper library version of fatal                                   */
/****************************************************************************/

void fatal (char *format, ...)
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    Efatal (0, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);
    }
