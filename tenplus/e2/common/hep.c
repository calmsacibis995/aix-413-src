static char sccsid[] = "@(#)18	1.8  src/tenplus/e2/common/hep.c, tenplus, tenplus411, GOLD410 11/4/93 10:04:25";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	closehelper, errx, killhelpers, openhelper,
 *		setuphelper, starthelp, stophelp, switchhelper,
 *		Eflushhelper
 *
 * ORIGINS:  9, 10, 27
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File:   hep.c                                                            */
/*                                                                          */
/* Routines to communicate with helpers across pipes                        */
/****************************************************************************/

#include "def.h"
#include "pipe.h"
#include <sys/wait.h>
#include "INeditor_msg.h"

#if !(BLTHELP || PIPEHELP)
/* dummy routines when no helpers are present */

void killhelpers () {}
/*void openhelper () {} */
void Eflushhelper () {}
#else

# ifdef PIPEHELP
#include <signal.h>
#include <errno.h>

#define MAXTRIES 4    /* Maximum number of fork attempts */
# endif

# ifdef BLTHELP
#include <builtin.h>
extern struct hlprtab hlprtab [];

static int builtin;
# endif

struct helper g_hep;    /* current helper   */
static struct helper althep; /* alternate helper */

static char *g_althelper; /* name of alternate helper */

static void setuphelper (char *, char *);
static void starthelp (char *,char *);
static void stophelp (void);
static void switchhelper (void);

/* catalog file descriptors */
#include <nl_types.h>
nl_catd g_helpcatd;
static nl_catd althelpcatd;


/****************************************************************************/
/* closehelper:  tells helper that we are zooming in or out.                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_helper                                         */
/*                                                                          */
/* globals changed:        g_helper                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     If there is no helper now, closehelper returns with no effect.       */
/*     Otherwise it clears the g_helper global, flushes standard output,    */
/*     and invokes stophelp.                                                */
/****************************************************************************/

void closehelper (void)
    {
# ifdef DEBUG
    debug ("closehelper: called w/ g_helper = '%s'", g_helper);
# endif

    if (g_helper == NULL)
	return;

    s_free (g_helper);
    g_helper = NULL;

    (void) fflush (stdout);
    stophelp ();
    }

# ifdef PIPEHELP
/****************************************************************************/
/* errx:  helper error handler                                              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_hep                                            */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Aside from debugging output, errx simply invokes Efatal to issue     */
/*     the "There is an error in the helper program." message.              */
/****************************************************************************/

void errx (void)
    {
#  ifdef DEBUG
    debug ("errx:  g_hep.proc = %d, g_hep.inp = %x, g_hep.outp = %x",
	g_hep.proc, g_hep.inp, g_hep.outp);
    debug ("errx:  althep.proc = %d, althep.inp = %x, althep.outp = %x",
	althep.proc, althep.inp, althep.outp);
#  endif

    Efatal (M_EHGARBAGE, "There is an error in the helper program.");
    }
# endif

/****************************************************************************/
/* killhelpers:  kill all helpers - used by fatal before restart            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     killhelpers simply kills the current helper, switches to the         */
/*     alternate helper, and kills it.                                      */
/****************************************************************************/

void killhelpers (void)
    {
    closehelper (); /* kill the current helper */
    switchhelper ();
    closehelper (); /* kill the alternate helper */
    }

/****************************************************************************/
/* openhelper:  starts a given helper                                       */
/*                                                                          */
/* arguments:              char *helpername - helper to open, or            */
/*                                            NULL to close current helper  */
/*                         char *arg        - argument for Hinit            */
/*                         char *state      - argument for Hrestart, or NUL */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_helper, g_althelper, g_hep, althep,            */
/*                         g_hlprpath, g_killhelper                         */
/*                                                                          */
/* globals changed:        g_helper, g_killhelper                           */
/*                                                                          */
/* synopsis:                                                                */
/*     openhelper opens the specified helper or, if given a null            */
/*     helper name, closes the current helper.  If a helper state           */
/*     object is provided as the third argument, openhelper calls           */
/*     Hrestart rather than Hinit.  If either Hinit or Hrestart             */
/*     fails, set the g_killhelper global true so that the action loop      */
/*     will try to restart the previous helper, if available.               */
/*                                                                          */
/*     If the helper name is the same as the current helper, openhelper     */
/*     calls Hinit or Hrestart with the given argument and then             */
/*     returns.  If it is the same as the current alternate helper,         */
/*     openhelper first switches to that helper and then invokes Hinit      */
/*     or Hrestart as before.                                               */
/*                                                                          */
/*     Otherwise it refers to the g_hlprpath global to see if it can        */
/*     find a helper of the given name.  If it finds it, it invokes         */
/*     starthelp to start it, sets g_helper to the helper name, and         */
/*     invokes Hinit or Hrestart.                                           */
/*                                                                          */
/*     If none of the preceding ways to find the desired helper succeed,    */
/*     simply return.                                                       */
/*                                                                          */
/****************************************************************************/

void openhelper (register char *helpername, register char *arg, register char *state)
            /* Hinit argument    */
           /* Hrestart argument */
    {
    register char *filename = (char *)NULL;

# ifdef DEBUG
    debug ("openhelper:  helpername = '%s', arg = '%s'", helpername, arg);
    debug ("openhelper state:");
    if (g_debugfp && state)
	s_print (state);
    debug ("openhelper:  g_helper = '%s', g_althelper = '%s'",
	g_helper, g_althelper);
#  ifdef BLTHELP
    debug ("openhelper:  g_hep.builtin = %d, althep.builtin = %d", g_hep.builtin, althep.builtin);
#  endif
#  ifdef PIPEHELP
    debug ("openhelper:  g_hep.proc = %d, althep.proc = %d", g_hep.proc, althep.proc);
#  endif
# endif


    clrhooks (); /* clear control hooks table */
    (void) fflush (stdout);

    if (seq (helpername, STANDARDFORM))
	helpername = NULL;

    if (seq (helpername, g_helper)) /* if reopening current helper */
	{
# ifdef DEBUG
	debug ("openhelper:  reopening old helper '%s'", helpername);
# endif
	if ((helpername != NULL) && (*helpername != '\0'))
	    setuphelper (arg, state);

	return;
	}

    if (seq (helpername, g_althelper))
	{
# ifdef DEBUG
	debug ("openhelper:  reopening alternate helper '%s'", helpername);
# endif
	switchhelper ();

	if ((helpername != NULL) && (*helpername != '\0'))
	    setuphelper (arg, state);

	return;
	}
    /* if there is a helper, make it the alternate helper */
# if BLTHELP && PIPEHELP
    if (g_hep.proc || g_hep.builtin)
# else
#  ifdef BLTHELP
    if (g_hep.builtin)
#  endif
#  ifdef PIPEHELP
    if (g_hep.proc)
#  endif
# endif
	{
	switchhelper ();    /* switch helper and alternate helper           */
	stophelp ();        /* stop old (alternate) helper                  */
	s_free (g_helper);  /* clear helper name                            */
	g_helper = NULL;
	}

    if ((helpername == NULL) || (*helpername == '\0')) /* if no helper */
	return;

    /***** try to find a "real" helper *****/

# ifdef BLTHELP
    if (builtin = builtin_helper (helpername))
	filename = helpername;
# endif

# ifdef PIPEHELP
    if (filename == (char *)NULL)
	filename = findfile (helpername, ".help", g_hlprpath);
# endif

    if (filename != NULL)   /* if there is a real helper, start it */
	{
# ifdef DEBUG
	debug ("openhelper:  starting real helper '%s'", helpername);
# endif
	starthelp (filename,helpername);
# ifdef BLTHELP
	if ( !builtin) /* free structured findfile allocation */
# endif
	    s_free (filename);
	s_free (g_helper); /* save new helper name */
	g_helper = s_string (helpername);
	clrhooks (); /* clear control hooks table */

	setuphelper (arg, state);
	return;
	}
    else
	{
# ifdef DEBUG
	debug ("openhelper:  cannot find helper '%s'", helpername);
# endif
        return;
	}

    }

/****************************************************************************/
/* setuphelper:  invoke Hinit or Hrestart for a helper                      */
/*                                                                          */
/* arguments:              char *arg        - Hinit argument                */
/*                         char *state      - Hrestart argument             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     setuphelper invokes either Hrestart or Hinit to get a new helper     */
/*     going                                                                */
/****************************************************************************/

static void setuphelper (char *arg, char *state)
                      /* argument for Hinit    */
                   /* argument for Hrestart */
    {
    if (state)
	{
	if (Hrestart (state) == ERROR)
	    {
	    g_killhelper = TRUE;
	    Eerror (M_EURESTART, "Cannot start helper program %s again.", g_helper);
	    }
	}
    else
	{
	if  (Hinit (arg) == ERROR)
	    g_killhelper = TRUE;
	}
    }

/****************************************************************************/
/* starthelp:   starts a new helper                                         */
/*                                                                          */
/* arguments:              char *helper - name of helper to start           */
/*                                                                          */
/* return value:           static void                                      */
/*                                                                          */
/* globals referenced:     g_hep                                            */
/*                                                                          */
/* globals changed:        g_hep                                            */
/*                                                                          */
/* synopsis:                                                                */
/*     starthelp opens the input and output pipes for the new helper        */
/*     and then forks to create the new helper.  It tries several times     */
/*     to do the fork, if necessary.  It invokes signal to make the         */
/*     helper ignore most signals, including BREAK.  If any part of         */
/*     this process fails, it issues a fatal error.                         */
/****************************************************************************/

static void starthelp (register char *helper,char *helpername)
    {
    char cat_name[20];

# ifdef PIPEHELP
    int p1 [2];
    int p2 [2];
    int i;
    int count;
# endif

#ifdef DEBUG
    debug ("starthelp:  starting helper '%s'", helper);
#endif

    sprintf(cat_name,"%s%s%s","IN",helpername,".cat");

#ifdef DEBUG
    debug ("starthelp:  Opening helper '%s'", cat_name);
#endif

    g_helpcatd = catopen (cat_name, NL_CAT_LOCALE); 


# ifdef BLTHELP

    if (builtin)
	{
	g_hep.builtin = builtin;
	return;
	}
# endif

# ifdef PIPEHELP
    /*$$$$ this should be a more elaborate test*/

    if (fexists (helper))
	{
	if (pipe (p1) == ERROR || pipe (p2) == ERROR)
	    fatal ("spawn: out of file descriptors");

	for (count = 0; count < MAXTRIES; count++)
	    {
	    g_hep.proc = fork ();
	    if (g_hep.proc != -1)
		break;
	    (void) sleep (2);
	    }

	if (count == MAXTRIES)
	    fatal ("spawn: unable to fork helper");

	if (g_hep.proc)
	    {    /* the parent process*/
	    g_hep.inp = fdopen (p1 [0], "r");
	    (void) close (p1 [1]);
	    g_hep.outp = fdopen (p2 [1], "w");
	    (void) close (p2 [0]);
	    }
	else
	    {   /* the child process*/
	    (void) close (0);
	    (void) dup (p2 [0]);

	    (void) close (1);
	    (void) dup (p1 [1]);

	    (void) close (2);
	    (void) open ("/dev/null", O_WRONLY);

	    /* arrange to handle each signal in the default manner, */
	    /* except those that are being ignored, which continue to */
	    /* be ignored. */
	    for (i = 1; i < NSIG; i++)
		if (signal (i, SIG_IGN) != SIG_IGN)
		    (void) signal (i, SIG_DFL);

	    /* Helpers should normally ignore BREAKs */

	    (void) signal (SIGINT, SIG_IGN);

	    /* Make the helper ignore QUIT signals as well */

	    (void) signal (SIGQUIT , SIG_IGN);


	    for (i = 3; i < _NFILE; i++)
		(void) close (i);

	    execl (helper, helper, (char *)NULL);
	    }
	}
    else
	g_hep.proc = ERROR;

    if (g_hep.proc == ERROR)
	fatal ("Cannot start helper '%s'", helper);

    /* make the pipe code set up stdio files    */
    dosetup (g_hep.outp, g_hep.inp);
# endif
    }

/****************************************************************************/
/* stophelp:  stops the current helper                                      */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           static void                                      */
/*                                                                          */
/* globals referenced:     g_hep                                            */
/*                                                                          */
/* globals changed:        g_hep                                            */
/*                                                                          */
/* synopsis:                                                                */
/*     stophelp sends a kill signal to the procedure of the g_hep global,   */
/*     and waits for it to return.  When it does, it nulls out the          */
/*     procedure subfield of g_hep and closes its input and output channels */
/****************************************************************************/

static void stophelp (void)
    {
# ifdef PIPEHELP
    int status; /* return status of child */
    register int res;
    extern   int errno;

#  ifdef DEBUG
    debug ("stophelp called w/ g_hep.proc = %d", g_hep.proc);
#  endif
# endif

    /* Close the current helper catalog */
    catclose(g_helpcatd);

# ifdef BLTHELP
#  ifdef DEBUG
    debug ("stophelp called w/ g_hep.builtin = %d", g_hep.builtin);
#  endif
    if (g_hep.builtin)
	{
	g_hep.builtin = 0;
	return;
	}
# endif

# ifdef PIPEHELP
    if (g_hep.proc == 0)
	return;

    /* closing the pipes should cause a sigpipe to be delivered to
       the helper, and the kill below will generally be unnecessary */
    (void) fclose (g_hep.inp);
    (void) fclose (g_hep.outp);

    (void) kill (g_hep.proc, 1);

    do
	{
	errno = 0;
	res = wait (&status);
	}
    while (res != g_hep.proc && (res != ERROR || errno != ECHILD));

    g_hep.proc = 0;
# endif
    }

/****************************************************************************/
/* switchhelper:  toggles g_hep and althep                                  */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           static void                                      */
/*                                                                          */
/* globals referenced:     g_hep, althep, g_helper, g_althelper             */
/*                                                                          */
/* globals changed:        g_hep, althep, g_helper, g_althelper             */
/*                                                                          */
/* synopsis:                                                                */
/*     switchhelper simply transposes the values of g_helper with           */
/*     g_althelper, and g_hep with althep.  After doing so, if hep.proc     */
/*     is not null (i.e. if there really was an alternate helper to         */
/*     switch to), switchhelper invokes dosetup to set up the               */
/*     new helper's input and output pipes.                                 */
/****************************************************************************/

static void switchhelper (void)
    {
    register char *name;
    struct helper tmphep;
    nl_catd tmpcat;


# ifdef DEBUG
#  ifdef BLTHELP
    debug ("switchhelper:  g_hep.builtin = %d", g_hep.builtin);
    debug ("switchhelper:  althep.builtin = %d", althep.builtin);
#  endif
#  ifdef PIPEHELP
    debug ("switchhelper:  g_hep.proc = %d, g_hep.inp = %x, g_hep.outp = %x",
	g_hep.proc, g_hep.inp, g_hep.outp);
    debug ("switchhelper:  althep.proc = %d, althep.inp = %x, althep.outp = %x",
	althep.proc, althep.inp, althep.outp);
#  endif
# endif

    tmphep = g_hep;
    g_hep = althep;
    althep = tmphep;

    /* Swap the message catalogs */
    tmpcat = g_helpcatd;
    g_helpcatd = althelpcatd;
    althelpcatd = tmpcat;

    name = g_helper;
    g_helper = g_althelper;
    g_althelper = name;

# ifdef PIPEHELP
    /* Make the pipe code set up stdio files.
       This must be conditional on the file pointers being non-null
       because this routine is called when there may NOT be an alternate
       helper.                                                              */

    if (g_hep.proc)
	dosetup (g_hep.outp, g_hep.inp);
# endif
    }

/****************************************************************************/
/* Eflushhelper:  terminate g_althelper if it has the given name            */
/*                                                                          */
/* arguments:              char *name - name of helper to flush             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_althelper                                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given name is the name of the alternate helper, switch to     */
/*     that helper, invoke closehelper to kill it gracefully, and then      */
/*     switch back to the original helper.  If the given name does not      */
/*     correspond to g_althelper, do nothing.                               */
/****************************************************************************/

void Eflushhelper (char *name)
    {
    if (seq (name, g_althelper))
	{
	switchhelper ();
	closehelper (); /* kill the alternate helper */
	switchhelper ();
	}
    }
#endif /* endif for whether editor has helpers */
