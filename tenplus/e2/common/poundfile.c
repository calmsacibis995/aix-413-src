static char sccsid[] = "@(#)50	1.7  src/tenplus/e2/common/poundfile.c, tenplus, tenplus411, GOLD410 11/4/93 10:18:35";
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, poundfile, debug, dotext, dobox
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
/* file:  poundfile.c                                                       */
/*                                                                          */
/* program to convert an editor delete stack file into a delta file         */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <locale.h>
#include <string.h>

#include "database.h"
#include "libobj.h"
#include "libshort.h"
#include "libutil.h"
#include "poundfile_msg.h"

void poundfile (char *, char *);
void dotext (FILE *, SFILE *, int);
void dobox (FILE *, SFILE *);

/* catalog file descriptor */
nl_catd g_poundcatd;

#ifdef DEBUG
static void debug (char *format1, ...);
#endif

int MultibyteCodeSet;

/****************************************************************************/
/* main:  top level routine                                                 */
/*                                                                          */
/* arguments:              int argc                                         */
/*                         char **argv                                      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_debugfp                                        */
/*                                                                          */
/* globals changed:        g_debugfp                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     The poundfile program expects to be called with the stack file       */
/*     name in argv[1] and the desired text file name in argv[2].           */
/*     If the DEBUG #ifdef is in effect, the g_debugfp channel is           */
/*     opened to write to the file "poundfile.out".  Then main simply       */
/*     calls poundfile with the stackfile and textfile as arguments.        */
/*                                                                          */
/*     This program is invoked as a filter when the user has asked          */
/*     to go to the "pound file" - the file whose name is "#".  It          */
/*     converts one of the editor stacks from its internal form             */
/*     (accessible only through s_read and s_write) to a delta file         */
/*     which the user can conveniently look at.                             */
/****************************************************************************/

main (int argc, char **argv)
    {

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    /* Open poundfile catalog */
    g_poundcatd = catopen(strcat(argv[0],".cat"), NL_CAT_LOCALE);

#ifdef DEBUG
    debug("Poundfile: catalog %s",argv[0]);
#endif

    if (argc != 3)
    {
    char *msg_ptr;

	msg_ptr = catgets (g_poundcatd,MS_POUNDFILE,M_USAGE,"0611-276 Usage: poundfile StackFile TextFile");

	fatal (msg_ptr);
    }

#ifdef DEBUG
    g_debugfp = fopen ("poundfile.out", "w");
#endif

    poundfile (argv [1], argv [2]);
    catclose(g_poundcatd);
    return(0);
    }

/****************************************************************************/
/* poundfile:  converts a stackfile into an ASCII file                      */
/*                                                                          */
/* arguments:              char *stackfile - from which to read stack       */
/*                         char *textfile  - in which to write text stack   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     poundfile opens the given stack file for reading and the text file   */
/*     for writing.  It then loops through the stack file looking for       */
/*     stack frames.  Each stack frame begins with a character defining     */
/*     its type.  Depending on the type of the frame, it either invokes     */
/*     dotext or dobox for each frame.  When finished, it closes the        */
/*     files and returns.                                                   */
/****************************************************************************/
void poundfile (char *stackfile, char *textfile)
    {
    FILE *fp;   /* input file pointer   */
    SFILE *sfp; /* output sfile pointer */
    char type;  /* stack type           */

#ifdef DEBUG
    debug ("poundfile: stackfile = '%s', textfile = '%s'", stackfile, textfile);
#endif
    fp = fopen (stackfile, "r");

    if (fp == NULL)
    {
    char *msg_ptr;

	msg_ptr = catgets(g_poundcatd,MS_POUNDFILE,M_STACK,"0611-277 poundfile: Cannot open stack file %s.\n\
	\tCheck path name and permissions.");
	fatal (msg_ptr, stackfile);
    }
    sfp = sf_open (textfile, "c");

    if (sfp == (SFILE *) ERROR)
    {
    char *msg_ptr;

        msg_ptr = catgets(g_poundcatd,MS_POUNDFILE,M_TEXT,"0611-278 poundfile: Cannot open text file %s.\n\
	\tCheck path name and permissions.");
	fatal (msg_ptr, textfile);
    }
    i_addtype();
    for (;;)
	{
	type = fgetc (fp);
	if (feof (fp))
	    break;

#ifdef DEBUG
	debug ("type = %c", type);
#endif
	switch (type)
	    {
	    case 'L': /* lines from and indexed field */
	    case 'l': /* lines from a non-indexed field */
		dotext (fp, sfp, TRUE);
		break;

	    case 't': /* running text */
		dotext (fp, sfp, FALSE);
		break;

	    case 'b': /* rectangular box of text */
		dobox (fp, sfp);
		break;
	    }
	(void) getl (fp); /* ignore frame pointer */
	}

    (void)fclose (fp);
    (void) sf_close (sfp);
    (void)chmod (textfile, 0444);
    }

#ifdef DEBUG
/****************************************************************************/
/* debug:  version of printf for debugging                                  */
/*                                                                          */
/* arguments:   (variable) char *format, *arg1, *arg2, *arg3, *arg4, *arg5  */
/*                                 - printf-style diagnostic output         */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_debugfp                                        */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     debug uses fprintf to send the given message to the g_debugfp channe */
/*     followed by a newline character.                                     */
/****************************************************************************/

static void debug (char *format1, ...)
    {
  	char *arg1, *arg2, *arg3, *arg4, *arg5;
va_list ap;
va_start(ap, format1);
arg1 = va_arg(ap, char *);
arg2 = va_arg(ap, char *);
arg3 = va_arg(ap, char *);
arg4 = va_arg(ap, char *);
arg5 = va_arg(ap, char *);
va_end(ap);

    if (g_debugfp != NULL)
	{
	(void) fprintf (g_debugfp, format1, arg1, arg2, arg3, arg4, arg5);
	(void) fputc ('\n', g_debugfp);
	(void) fflush (g_debugfp);
	}
    }
#endif

/****************************************************************************/
/* dotext:  handles lines and text frames from stack                        */
/*                                                                          */
/* arguments:              FILE   *fp - input stack file                    */
/*                         SFILE *sfp - output human-readable file          */
/*                         int   flag - TRUE if pickname is in frame        */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     dotext handles stack frames for line and text regions.  It first     */
/*     reads the count of the number of lines in the frame and then,        */
/*     if the input flag is set, reads and ignores the pickname (used       */
/*     only in line regions for indexed files).  It then reads the          */
/*     specified number of lines from the stack file and writes them        */
/*     into the text file.  If a line from the stack file is not of         */
/*     type T_CHAR (which should not happen), it is ignored.                */
/****************************************************************************/

void dotext (FILE *fp, SFILE *sfp, int flag)
    {
    int count;
    int index1;
    char *record;

    count  = geti (fp);
    index1 = obj_count (sf_records (sfp));

#ifdef DEBUG
    debug ("dotext:  count = %d, index = %d", count, index1);
#endif

    if (flag)
	{
	char *cp = s_read (fp);
	if (cp != (char *)ERROR)
	    s_free (cp);
	}

    while (count--)
	{
	record = s_read (fp);
	if (record && record != (char *)ERROR)
	    {
	    if (obj_type (record) == T_CHAR)
		(void) sf_write (sfp, index1++, record);
	    s_free (record);
	    }
	}
    }

/****************************************************************************/
/* dobox:  handles box frames from the stack                                */
/*                                                                          */
/* arguments:              FILE   *fp - input stack file                    */
/*                         SFILE *sfp - output human-readable file          */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     dobox reads a box-region stack frame from the stack file and writes  */
/*     it into the output file.  It first reads the height and width of     */
/*     the box from the stack file, and then reads the specified number of  */
/*     lines from the file and writes them to the output file.              */
/****************************************************************************/

void dobox (FILE *fp, SFILE *sfp)
    {
    int i=0;
    int height;        /* box height                */
    int columns;       /* box columns               */
    int nbytes;        /* bytes per box line        */
    int index2;        /* sfile index               */
    char *record;      /* sfile record              */

    ATTR ch[MB_LEN_MAX];
    ATTR *cp;

    height   = geti(fp);
    columns  = geti(fp);
    index2   = obj_count (sf_records (sfp));

#ifdef DEBUG
    debug ("dobox: height = %d,columns = %d,index = %d",height,columns,index2);
#endif

    cp = (ATTR *) s_alloc ((columns * __max_disp_width) + 1, T_ATTR, "");
    while (height--)
	{
        
        /* get the number of bytes to read */
        nbytes = geti(fp);

        /* retrieve the required number of bytes */
         
        i=0;
	while(nbytes--)
	    cp[i++] = geti(fp);

	cp[i] = 0;
	record = packup(cp);
	(void) sf_write (sfp, index2++, record);
	s_free (record);
	}
    s_free((char *)cp);
    }
