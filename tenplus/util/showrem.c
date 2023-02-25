static char sccsid[] = "@(#)56	1.7  src/tenplus/util/showrem.c, tenplus, tenplus411, GOLD410 11/4/93 12:23:21";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, makename, lock, unlock, 
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
/* file:  showrem.c - showrem program                                       */
/*                                                                          */
/* showrem is used to display reminders.                                    */
/*                                                                          */
/* Usage: showrem [-d]                                                      */
/*                                                                          */
/* The showrem program puts the contents of the $HOME/.reminder file on     */
/* standard output.  If the -d flag is specified, it will remove the file   */
/* before exiting.  A lock file is used to resolve possible contention      */
/* problems.                                                                */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>

#include "showrem_msg.h"
#include "database.h"
#include "util.h"
#include <libutil.h>
#include <libobj.h>

#define NUMTRIES 15            /* number of tries to lock   */
#define SLEEPTIME 2            /* sleep time between tries  */
#define USER_FILE ".reminder"
#define MSGSTR(num,str)         catgets(g_catd,MS_SHOWREM, num, str)
int MultibyteCodeSet;

nl_catd g_catd;


main (int argc, char **argv)
    {
    register FILE *fp;          /* input file pointer        */
    register char *filename;    /* $HOME/.reminder file name */
    register c;                 /* input character           */

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd = catopen(argv[0], NL_CAT_LOCALE);

    if ((argc > 2) || ((argc == 2) && (strcmp (argv [1], "-d"))))
	fatal (MSGSTR(M_USAGE,"Usage: showrem [-d]"), argv[0]);

#ifdef DEBUG
    debug ("showrem:  deleteflag = %s", (argc > 1) ? "TRUE" : "FALSE");
#endif

    filename = makename (USER_FILE);
    (void)lock (filename);
    fp = fopen (filename, "r");

    if (fp == NULL)
	{
	(void)unlock (filename);
	(void)fatal (MSGSTR(M_OPEN,"showrem: 0611-242 Cannot open file %s."), argv[0], filename);
	}
    while ((c = getc (fp)) != EOF)
	(void) putchar (c);

    (void)fclose (fp);

    if (argc > 1)
	(void)unlink (filename);

    (void)unlock (filename);
    (void)free (filename);
    catclose(g_catd);
    exit (0);
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
	fatal (MSGSTR(M_HOME,"showrem: 0611-243 Cannot evaluate the $HOME variable.\n\
	\tSet the HOME shell variable and export it."));

    length = strlen (home) + strlen (filename) + 2;
    fullname = malloc (length);

    if (fullname == NULL)
	(void)fatal (MSGSTR(M_MALLOC,"showrem: 0611-244 Cannot allocate memory in the makename routine."));

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

    (void)fatal (MSGSTR(M_LOCK,"showrem: 0611-245 Cannot open lock file %s."), filename);
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
	(void)fatal (MSGSTR(M_UNLOCK,"showrem: 0611-246 Cannot delete file %s."
	), filename);
    }

