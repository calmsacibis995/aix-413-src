static char sccsid[] = "@(#)02	1.9  src/tenplus/helpers/print/prtty.c, tenplus, tenplus411, GOLD410 11/4/93 10:28:52";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, parse_flags, print, sendit, getterminfo
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
/* File: prtty.c - TENPLUS print helper post-processer for terminals        */
/*                 with printer ports with software controllable            */
/*                 flow control.                                            */
/*                 start sequence: enables main port to printer port flow   */
/*                 stop  sequence: disables main port to printer port flow  */
/*                 The start and stop sequences are retrieved from the      */
/*                 terminfo database.                                       */
/****************************************************************************/

#include    <locale.h>
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <curses.h>
/* the following are low-level curses routines,                     */
/* but they do not have prototypes in the header file               */
extern	int	fixterm(void);
extern	int	putp(char *);
extern	int	resetterm(void);
extern	int	setupterm(char *, int, int *);
extern	char *	tparm(char *, int, ...);
extern	int	tputs(char *, int, int(*)());
#include    <term.h>

#include    <chardefs.h>
#include    <database.h>
#include    <edexterns.h>
#include    <window.h>
#include    <libobj.h>
#include    <libhelp.h>
#include    <libutil.h>
#include    "prtty_msg.h"
#include    "print.h"

#define MAXLINES 60  /* default number of lines per page      */

nl_catd g_catd;
int MultibyteCodeSet;

struct pinfo {
    char *start;    /* start sequence - turn on printer port */
    char *stop;     /* stop sequence - turn off printe port  */
    int lineflag;   /* TRUE if paging requested              */
    int maxlines;   /* number of lines per page              */
    POINTER *files; /* names of the files to print           */
    FILE *infp;     /* input file pointer                    */
    FILE *outfp;    /* output file pointer - the terminal    */
    int linect;     /* current line                          */
};

void parse_flags(int , char *[], struct pinfo *);
void sendit(struct pinfo *);
void getterminfo(struct pinfo *);
void print(struct pinfo *);

/****************************************************************************/
/* main:                                                                    */
/*      environment variable TERM must be set.                              */
/****************************************************************************/

main(int argc, char *argv[])
    {
     struct pinfo prt;
     register struct pinfo *pprt;

     /* set the local environment */
     (void) setlocale(LC_ALL, "");

     if (MB_CUR_MAX == 1)
           MultibyteCodeSet = 0;
     else  MultibyteCodeSet = 1;

     g_catd = catopen(MF_PRTTY, NL_CAT_LOCALE);

     pprt = &prt;

     parse_flags(argc, argv, pprt);

     (void)getterminfo(pprt);

     (void)print(pprt);

     (void)s_free((char *)pprt->files);
     catclose(g_catd);
     return(0); /* return put here to make lint happy */
    }


/****************************************************************************/
/* parse_flags: parse the command line                                      */
/*              -l# is the number of lines to page on                       */
/*              collect filenames in allocated POINTER buffer               */
/****************************************************************************/

void parse_flags(int argc, char *argv[], struct pinfo *pprt)
    {
    register char *argument;
    register char *cp;
    register char *progname;
    char *msg_ptr; /* Message pointer */

    pprt->lineflag = FALSE;
    pprt->maxlines = MAXLINES;
    pprt->files = (POINTER *) s_alloc (0, T_POINTER, "infiles");

    progname = argv[0];

    for (argv++; --argc; argv++)
	{
	argument = *argv;

	if (*argument == '-')
	    {
	    switch (*(++argument))
		{
		case 'l':
		    pprt->lineflag = TRUE;
		    if(*++(argument))
			{
			for (cp = argument; *cp; cp++)
			    if(!isdigit(*cp)) 
				{
				msg_ptr = catgets(g_catd, MS_PRTTY,ARGN, "0611-402 Specify a positive integer after the -l flag.");
				fatal(msg_ptr);
				}
			pprt->maxlines = atoi(argument);
			}
		    break;

		default:
		    msg_ptr = catgets(g_catd,MS_PRTTY,USAGE, "Usage: %s [-l[Number]] [Files]");
		    fatal(msg_ptr, progname);
		}
	    }
	else
	    {
	    pprt->files = s_append(pprt->files, s_string(argument));
	    }

	}

    }


/****************************************************************************/
/* print: open the terminal for writing, if no files open stdin for reading */
/*        else loop through the list of files for reading.                  */
/*        for each file send the start sequence, call print, send the       */
/*        stop sequence                                                     */
/****************************************************************************/

void print(struct pinfo *pprt)
    {
    register int i;
    char	*msg_ptr;

    pprt->outfp = fopen("/dev/tty", "w+");
    if(pprt->outfp == NULL)
	{
	msg_ptr = catgets(g_catd,MS_PRTTY,TERM, "0611-403 Cannot write to the terminal." );
	fatal(msg_ptr);
	}

    (void) fprintf (pprt->outfp, pprt->start); /* send start string */
    (void) fflush (pprt->outfp);

    if(pprt->lineflag)
    pprt->linect = pprt->maxlines;

    if(obj_count(pprt->files) > 0)
       {
       for (i = 0; i < obj_count(pprt->files); i++)
	   {
	   pprt->infp = fopen(pprt->files[i], "r");
	   if(pprt->infp == NULL)
	       {
	       (void) fprintf (pprt->outfp, pprt->stop); /* send stop string */
	       (void) fflush (pprt->outfp);
	       msg_ptr = catgets(g_catd,MS_PRTTY,CNOPEN, "0611-404 Cannot read %s.\n" );
	       (void) fprintf(pprt->outfp, msg_ptr, pprt->files[i]);
	       (void) fflush (pprt->outfp);
	       (void) fprintf (pprt->outfp, pprt->start); /* send start string */
	       (void) fflush (pprt->outfp);
	       continue;
	       }
	   (void)sendit(pprt);
	   (void)fclose(pprt->infp);
	   }
       }
    else
       {
       pprt->infp = stdin;
       (void)sendit(pprt);
       }

    (void) fprintf (pprt->outfp, pprt->stop); /* send stop string */
    (void) fflush (pprt->outfp);
    (void)fclose(pprt->outfp);
    }

/****************************************************************************/
/* sendit: read the input and send it to the terminal                       */
/*         if linecount is enabled ask used for confirmation to             */
/*         send the next page                                               */
/****************************************************************************/

void sendit(struct pinfo *pprt)
    {
    register int c;
    char *msg_ptr;

    while((c = fgetc(pprt->infp)) != EOF)
	{
	 if(pprt->lineflag)
	    {
	    if(pprt->linect == pprt->maxlines)
	       {
	       pprt->linect = 0;
	       (void) fflush (pprt->outfp);
	       (void) fprintf (pprt->outfp, pprt->stop); /* send stop string */
	       (void) fflush (pprt->outfp);

               msg_ptr = catgets(g_catd,MS_PRTTY,TRTN, "Press Enter to continue printing." );
	       (void) fprintf(pprt->outfp, msg_ptr); /* touch return to */
	       (void) fflush (pprt->outfp);
	       while(getchar() != '\n')
		   ;

	       (void) fprintf (pprt->outfp, pprt->start); /* send start string */
	       (void) fflush (pprt->outfp);
	       }

	    if(c == '\n') pprt->linect++;
	    }
	 (void) fputc(c, pprt->outfp);
	}

     (void) fflush (pprt->outfp);
     }


/****************************************************************************/
/* getterminfo: Retrieve terminfo database entries for turning printer on   */
/*              and off.                                                    */
/*                                                                          */
/****************************************************************************/

void getterminfo (struct pinfo *pprt)
    {
    char *termtype;
    int   status;
    char *msg_ptr;

    termtype = getenv ("TERM");

    if (termtype == NULL)
	{
	msg_ptr = catgets(g_catd,MS_PRTTY,TERMNS, "0611-406 The TERM shell variable is not in the environment.\n\tTo activate prtty, set and export the TERM variable." );
	fatal (msg_ptr);
	}

    setupterm(termtype, 1, &status);
    if (status != 1)
	{
	msg_ptr = catgets(g_catd, MS_PRTTY, TUNK,
			"0611-407 Terminal type %s is not recognized." );
	fatal (msg_ptr, termtype);
	}

    /* set the output strings  */
    pprt->start = prtr_on;
    pprt->stop  = prtr_off;

    }


/****************************************************************************/
/* parse_termname:                                                          */
/*                                                                          */
/* arguments:            char *string - the full string (from sf_getnames())*/
/*                       char *termtype - the 'target' we are searching for */
/*                                                                          */
/* return value:         0 - we found the terminal type                     */
/*                       1 - we didn't find it                              */
/*                                                                          */
/* globals referenced:   none                                               */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:  Called from St_getcap.  Passes in a string that contains one  */
/* or more terminal types.  Loops through the string; if it finds an exact  */
/* match, returns 0, otherwise returns 1.                                   */
/****************************************************************************/


int parse_termname (char *string, char *termtype)
{
    char *p1, *p2, savechar;

    p2 = p1 = string;

    while (TRUE)
	{
        /* advance p1 until it points to a white char (or the
           terminating null char) */
        for (; !ismbspace(p1) && *p1 != '\0'; skipmb(&p1)) ;

	savechar = *p1;         /* keep track whether space or NULL */
	*p1 = '\0';             /* makes p2 point to a substring */
	if (strcmp (termtype, p2) == 0)
	    {
	    *p1 = savechar;     /* restore original value of char */
	    return (0);         /* found termtype */
	    }
	if (savechar == '\0')
	    return (1);         /* didn't find termtype in this string*/
	*p1 = savechar;

	/* position at next token within string */
        skipmb(&p1);
        p2 = p1;
	}
}
