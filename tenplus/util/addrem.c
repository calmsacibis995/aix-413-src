static char sccsid[] = "@(#)51	1.7  src/tenplus/util/addrem.c, tenplus, tenplus411, GOLD410 11/4/93 12:23:19";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, makename, lock, unlock
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
/* file:  addrem.c - addrem program                                         */
/*                                                                          */
/* addrem is used to append text to the $HOME/.reminder file                */
/*                                                                          */
/* Usage: addrem reminder                                                   */
/*                                                                          */
/* The addrem program appends the reminder message to the $HOME/.reminder   */
/* file.  A lock file is used to resolve possible contention problems.      */
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>

#include "addrem_msg.h"
#include "database.h"
#include "util.h"
#include <libutil.h>
#include <libobj.h>

#define NUMTRIES 15            /* number of tries to lock   */
#define SLEEPTIME 2            /* sleep time between tries  */
#define USER_FILE ".reminder"
#define MSGSTR(num,str)         catgets(g_catd,MS_ADDREM, num, str)

int MultibyteCodeSet;
nl_catd g_catd;

main (int argc, char **argv)
    {
    register FILE *fp;          /* output file pointer       */
    register char *filename;    /* $HOME/.reminder file name */

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd= catopen(MF_ADDREM, NL_CAT_LOCALE);

#ifdef DEBUG
    debug("addrem: Message catalog %s",argv[0]);
#endif

    if (argc < 2)
	(void)fatal (MSGSTR(M_USAGE,"Usage: addrem String"), argv[0]);

#ifdef DEBUG
    debug ("addrem:  string = '%s'", argv[1]);
#endif

    filename = makename (USER_FILE);
    (void)lock (filename);
    fp = fopen (filename, "a");

    if (fp == NULL)
	{
	(void)unlock (filename);
	(void)fatal (MSGSTR(M_OPEN,"%1$s: 0611-227 Cannot find or open file %2$s."), argv[0], filename);
	}
    (void) fprintf (fp, "%s\n", argv[1]);
    (void)fclose (fp);
    (void)unlock (filename);
    (void)free (filename);
    catclose(g_catd);
    (void)exit (0);
    return(0);
    /*NOTREACHED*/
    }

/****************************************************************************/
/* makename:  returns file name for reminder file or lock file              */
/* Returns an allocated character pointer that should be free'd             */
/****************************************************************************/

char *makename (char *filename)
    /* filename:   file name in the $HOME directory */
    {
    register char *home;        /* value of $HOME   */
    register length;            /* full name length */
    register char *fullname;    /* full file name   */

    home = getenv ("HOME");

    if (home == NULL)
	(void)fatal (MSGSTR(M_HOME,"0611-228 Cannot evaluate the $HOME variable.\n\
	Set the shell variable HOME and export it."));

    length = strlen (home) + strlen (filename) + 2;
    fullname = malloc (length);

    if (fullname == NULL)
	(void)fatal (MSGSTR(M_MALLOC,"0611-229 Cannot allocate memory in the makename routine."));

    (void) sprintf (fullname, "%s/%s", home, filename);

#ifdef DEBUG
    debug ("makename:  fullname = '%s'", fullname);
#endif

    return (fullname);
    }

/****************************************************************************/
/* lock:  tries to lock the $HOME/.remlock file                             */
/****************************************************************************/

void lock (char *filename)
    /* filename:                lock file name  */
    {
    register i;              /* loop counter    */

#ifdef DEBUG
    debug ("lock called");
#endif

    for (i = 0; i < NUMTRIES; i++)
	if (Llock (filename) != ERROR)
	    return;
	else
	    (void) sleep (SLEEPTIME);

    (void)fatal (MSGSTR(M_LOCK,"0611-230 Cannot open lock file %s."), filename);
    }

/****************************************************************************/
/* unlock:  tries to unlock the $HOME/.remlock file                         */
/****************************************************************************/

void unlock (char *filename)
    {
#ifdef DEBUG
    debug ("unlock called");
#endif

    if (Lunlock (filename) == ERROR)
	(void)fatal (MSGSTR(M_UNLOCK,"0611-231 Cannot delete file %s."), filename);
    }

