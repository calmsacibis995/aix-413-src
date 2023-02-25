static char sccsid[] = "@(#)16	1.10  src/tenplus/sf_progs/readfile.c, tenplus, tenplus411, GOLD410 3/3/94 15:18:44";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, readfile, obj_print, cat, graphfilter, gconvert,
 *		filecheck, basename, dir_name, interrupt, cleanfatal
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* file:  readfile.c: display structured files                              */
/*                                                                          */
/* Prints out contents of structured file                                   */
/*                                                                          */
/* Usage:                                                                   */
/* readfile [-dghstu?] [-_n] [+_n] [file...] [-o outfile] [-m msgcatalog setnumber] [file...] */
/*                                                                          */
/* See manual page for quick description of meanings of options             */
/****************************************************************************/

/* System Includes */
#include <stdio.h>              /* C library io routines                    */
#include <string.h>
#include <ctype.h>              /* 'isalpha()', 'isdigit()', etc            */
#include <errno.h>              /* system call error diagnostics            */
#include <sys/types.h>          /* system type definitions                  */
#include <sys/stat.h>           /* file status macros & 'stat' struct def   */
				/* used in 'filecheck' routine              */
#include <sys/access.h>
#include <signal.h>    
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <limits.h>
#include <chardefs.h>           /* for definition of control ()             */

/*  Ined Header files */
#include <database.h>           /* structured allocation & structured file  */
				/* struct & macro definitions               */
				/* to catch SIGINT                          */
#include <libobj.h>
#include "tree.h"

char sobuf [BUFSIZ];    /* sets you up to buffer stdio by size of disk blck*/

char tempfile[PATH_MAX];     /* name of temp file for -o option              */

#define FIRSTSON  (1)   /* Macros used in the tree diagramming portion  */
#define LASTSON   (2)   /* of the code                                  */
#define ONLYSON   (3)
#define MIDDLESON (4)

int MultibyteCodeSet;

/* Catalog Descriptor file */
nl_catd g_catd;
nl_catd msg_catd;          /* descriptor for optional message catalog    */


extern int errno;          /* for error diagnosis on system calls        */
int dumpfile = FALSE;      /* flags 'dumpfile' as opposed to 'readfile'  */
int graphics = FALSE;      /* flag to convert graphics to printing chars */
int messages = TRUE;       /* flag causing diagnostic msgs to print out  */
			   /* nonexistent files                          */
int headers = FALSE;       /* flag to put a header on each file's output */
int treeprint = FALSE;     /* flag for printout in tree form             */
int startrec = 0;          /* number of record to start reading at       */
int indfac = 5;            /* size of 'tabs' used in pretty-printing     */
FILE *outfp;               /* pointer to output file                     */
int msgcat = FALSE;        /* flag to use message catalog when           */
                           /* displaying text                            */
int msgcatset;		   /* message catalog set number for '-m' option */

void readfile(char *);
void obj_print(int, POINTER *);
void cat (char *);

void graphfilter (char *);
char gconvert (char );
SFILE *filecheck (char *);
void cleanfatal (char *, ...);

/****************************************************************************/
/* main:  top level routine.  Parses args and calls readfile                */
/****************************************************************************/

main (int argc, char **argv)
    {
    char *s;
    char *outputfile;      /* name of output file when '-o' exercised    */
    char *msgcatfile;      /* name of message catalog file when '-m' given */
    char backupfile[PATH_MAX]; /* name of backup file if a file is over-     */
			   /* written by '-o'                            */
    int outfile = FALSE;   /* flag for output file designated on cmd line*/
    struct stat buf;       /* file status structure                      */
    int i;                 /* looping variable                           */
    char c;
    int m_argv = -3;       /* index of argv holding -m option            */
    int o_argv = -2;       /* index of argv holding -o option            */
    char *dir_name(char *, char *);
                           /* extracts path prefix from a pathname       */
    char *basename(char *);
                           /* extracts basename from a pathname          */
    char *out;         /*outputfile base name */
    char dirspace[PATH_MAX];    /* allocated space for holding pathnames      */
    char *tmpspace;             /* scratch space                              */
    char *outdir;          /* directory of output file                   */
    void interrupt (void);
                           /* interrupt handler                          */
    int name_max;          /* maximum number bytes per filename allowed
                              on system */
    int multilen, count, totbytes;
    char *cpptr;          /* pointer to file name */
    char *tmpptr;         /* pointer to temporary file name storage area */

#ifdef DEBUG
    g_debugfp = fopen ("trace.out", "w");
    debug ("main:  argc = %d, *argv = %s", argc, *argv);
    debug ("and globals dumpfile = %d, graphics = %d, \
	     messages = %d, headers = %d, treeprint = %d, msgcat = %d",
	     dumpfile, graphics, messages, headers, treeprint, msgcat);
#endif

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
       MultibyteCodeSet = 0;
    else 
       MultibyteCodeSet = 1;
 
    g_catd = catopen(MF_READFILE, NL_CAT_LOCALE);

    setbuf (stdout, sobuf);         /* buffer the output to size BUFSIZ      */

    /* take care of option flags */

    /* for each string of options... */
    while (--argc > 0 && ((c = (*++argv)[0]) == '-' || c == '+'))
	{
	if (msgcat)  /* -m option has been spotted -- must preceed a filename */
                     /*  not another option string                            */
	   fatal (MSGSTR(M_USAGE,"Usage: readfile [-dghstu?] [+n] [-n] [-o Outputfile] [-m MsgCatalog SetNumber] [File...]\n"));
	if (outfile) /* -o option has been spotted -- must preceed a filename */
		     /*  not another option string                            */
	   fatal (MSGSTR(M_USAGE,"Usage: readfile [-dghstu?] [+n] [-n] [-o Outputfile] [-m MsgCatalog SetNumber] [File...]\n"));


	for (s = argv[0] + 1; *s != '\0'; s++) /* for each option in string */
	    {

	    if (c == '+')    /* Starting record number is specified     */
		{
		if (isdigit (*s))
		    {
		    startrec = atoi(s);
		    break;
		    }
		else
		    fatal (MSGSTR(M_USAGE,"Usage: readfile [-dghstu?] [+n] [-n] [-o Outputfile] [-m MsgCatalog SetNumber] [File...]\n"));
		}
	    if (isdigit (*s))   /* size of indent used in display specified   */
		{
		indfac = atoi (s);
		break;          /*  break from 'for' loop, continue 'while'   */
		}
	    else
		{
		switch (*s)
		    {
		    case 'm':
			if (outfile)
			    fatal (MSGSTR(M_FLAGS_CONFLICT, "-m and -o flags conflict.\n"));
			if (argc <= 2)
			    fatal (MSGSTR(M_MINUS_M, "Specify a message catalog file and set number for the -m flag.\n"));
			else
			    msgcat = TRUE; /* -m spotted                  */
			break;
		    case 'o':
			if (msgcat)
			    fatal (MSGSTR(M_FLAGS_CONFLICT, "-m and -o flags conflict.\n"));
			if (argc == 1)
			    fatal (MSGSTR(M_MINUS_O,"0611-452 Specify an output file for the -o flag.\n"));
			else
			    outfile = TRUE; /* -o spotted                */
			break;
		    case 'd':
			dumpfile = TRUE;
			break;
		    case 'h':
			headers = TRUE;
			break;
		    case 'g':
			graphics = TRUE;
			break;
		    case 'u':               /* unbuffered output            */
			setbuf (stdout, NULL);
			break;
		    case 's':               /* suppress error messages      */
			messages = FALSE;
			break;
		    case 't':               /* printout in tree form              */
			treeprint = TRUE;
			break;
		    default:
			(void) printf (MSGSTR(M_USAGE,"Usage: readfile [-dghstu?] [+n] [-n] [-o Outputfile] [-m MsgCatalog SetNumber] [File...]\n"));
			catclose(g_catd);
			exit (1);
		    }  /* end switch */
		}   /* end else   */
	    }   /* end for */
	}  /* end while */

    /* argv[0] now points to first filename on command line                */

    if (msgcat)        /* m_argv is 1 less than subscript of message       */
 	m_argv = -1;   /* file in argv array                               */

    if (outfile)
	o_argv = -1;   /* o_argv is 1 less than subscript of output file in*/
		       /* argv array                                       */
    /* -o and -m are the only options allowed to be mixed in with the      */
    /* filenames on the command line.                                      */
    /* scan command line for -m and -o arguments and mark the arguments    */
    /* which follow as message catalog file and outfile respectively       */

    /* argc is now equal to the number of args not processed above         */
    /* argv [0] is the first arg that's not processed above                */
    for (i=0; i < argc; i++)
	{
	if (argv[i][0] == '-' && argv[i][1] == 'm') /* -m spotted... */
	    {
	    if (strchr(argv[i],'o'))
		fatal (MSGSTR(M_FLAGS_CONFLICT, "-m and -o flags conflict.\n"));

	    if (msgcat)			/* -m previously spotted     */
		fatal (MSGSTR(M_DUP_M, "Use the -m flag only once.\n"));

	    if (! (i < (argc-2)))
		fatal (MSGSTR(M_MINUS_M, "Specify a message catalog file and set number for the -m flag.\n"));
	    else
		{
		msgcat = TRUE;          /* -m spotted                      */
		m_argv = i;             /* index of argv where '-m' appears*/
		}
	    }
	else if (i == m_argv+1)		/* if -m was previous item on cmd line*/
	    msgcatfile = argv[i];
	else if (i == m_argv+2)		/* if msgcatfile was previous item on */
	    {
	    if (!sscanf(argv[i], "%d", &msgcatset))
		fatal (MSGSTR(M_MINUS_M, "Specify a message catalog file and set number for the -m flag.\n"));
	    }
	else if (argv[i][0] == '-' && argv[i][1] == 'o') /* -o spotted... */
	    {
	    if (strchr(argv[i],'m'))
		fatal (MSGSTR(M_FLAGS_CONFLICT, "-m and -o flags conflict.\n"));

	    if (outfile)                /* if -o previously spotted   */
		fatal (MSGSTR(M_DUP_O,"Use the -o flag only once.\n"));

	    if (! (i < (argc-1)))
		fatal (MSGSTR(M_MINUS_O,"0611-452 Specify an output file for the -o flag.\n"));
	    else
		{
		outfile = TRUE;         /* '-o' spotted                    */
		o_argv = i;             /* index of argv where '-o' appears*/
		}   /* end _e_l_s_e */
	    }   /* end _i_f */
	else if (i == o_argv+1)         /* if -o was previous item on cmd line*/
	    outputfile  = argv[i];

	}   /* end _f_o_r  */

    if (msgcat)
	{
#ifdef DEBUG
    debug("msgcatfile '%s' msgcatset %d", msgcatfile, msgcatset);
#endif
	msg_catd = catopen(msgcatfile, NL_CAT_LOCALE);
	}

    if (outfile)
	{
	outdir = dirspace;     /* make outdir point to an allocated space  */
	/* extract outputfile's directory & copy into space pointed to by  */
	/* outdir                                                          */
	outdir = dir_name (outdir, outputfile);

	/* generate a temporary file for output...                         */
	/* put it in same directory that the output file is to go into     */

	/* create backup filename                                          */

	/* if filename exceeds maximum number letters allowed by system, 
           is truncated so ".bak" will fit          */

        out = basename(outputfile);
        name_max = (int)pathconf(outputfile, _PC_NAME_MAX);
        /* set name_max to 10 if pathconf failed */
        if (name_max == -1)  name_max = 10;
        tmpspace = malloc(name_max + 1);

        totbytes = 0; /* variable used for total number of multibyte 
                         characters */ 

        if (MultibyteCodeSet) {
           tmpptr = tmpspace;
           cpptr = out;
           multilen = mblen (cpptr, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (multilen < 1) multilen = 1;
           while ((*cpptr != '\0') &&
                  (totbytes + multilen <= name_max - 4)) {
                 for (count = 0; count < multilen; count++) {
                     *tmpptr++ = *cpptr++;
                     totbytes++;
                 }
                 multilen = mblen (cpptr, MB_CUR_MAX);
		 /* copy at least 1 char */
		 if (multilen < 1) multilen = 1;
            }
            *tmpptr = '\0';
        /* above processing ensures that truncation will not occur in the
           middle of multibyte character */
        } 
        else {
           (void)strncpy(tmpspace, out, name_max - 4);
           tmpspace[name_max - 4] = '\0'; /*null terminate string*/
        }
	if (outdir)
	    (void) sprintf (backupfile, "%s/%s.bak", outdir, tmpspace);
	else
	    (void) sprintf (backupfile, "%s.bak", tmpspace);

	if (outdir)      /* if outfile's directory specified...            */
	    (void) sprintf (tempfile, "%s/rdf%d", outdir, getpid());
	else
	    (void) sprintf (tempfile, "rdf%d", getpid());

	if ((outfp = fopen (tempfile, "w")) == NULL)
	    {
#ifdef DEBUG
	    debug ("Can't open temporary file '%s'", tempfile);
#endif
	    fatal (MSGSTR(M_WRITE,"0611-454 Cannot create the specified output file.\n"));
	    }
        free(tmpspace);
	}
    else
	outfp = stdout;

    (void) signal (SIGINT, (void (*)(int))interrupt);
    (void) signal (SIGTERM, (void (*)(int))interrupt);


    if (argc == 0 || ((argc == 1) && outfile)) /* if no input file specified */
	cat (NULL);      /* cat from standard input */
    else
	for (i=0 ; i < argc ; i++)  /* readfile all files on cmd line except */
				    /* output file                           */
	    {
	    /* if this cmd line arg is '-m' or message catalog file or       */
	    /* catalog set number                                            */
	    if (i == m_argv || i == (m_argv+1) || i == (m_argv+2))
		continue;
	    /* if this cmd line arg is '-o' or output file */
	    else if (i == o_argv || i == (o_argv+1))
		continue;
	    else
		{
		if (headers)
		    {
		    (void) fprintf (outfp, "------------------------------------------\n");
		    (void) fprintf (outfp, "               %s\n", argv[i]);
		    (void) fprintf (outfp, "------------------------------------------\n");
		    }
		readfile (argv[i]);
		}
	    }

    if (outfile)
	{
	/* check whether or not output file already exists      */
	if (stat (outputfile, &buf) == ERROR)
	    {
	    /* if errno is ENOENT, it doesn't exist */
	    if (errno != ENOENT)
	    /* (File exists but something's wrong with it... )  */
		cleanfatal (MSGSTR(M_INVALID,"0611-455 Cannot access the specified output file.\n"), errno, errno);
	    }
	else    /* file exists ... make backup copy             */

	    {
	    if (accessx (outputfile, W_ACC, ACC_SELF) != 0)
		cleanfatal (MSGSTR(M_NO_WRITE,"0611-456 You must have write permission for the specified output file.\n"));

	    /* check whether or not .bak file already exists      */
	    if (stat (backupfile, &buf) == ERROR)
		{
		/* if errno is ENOENT, it doesn't exist */
		if (errno != ENOENT)
		/* (Backup file exists but something's wrong with it... )  */
		    cleanfatal (MSGSTR(M_INVALID,"0611-455 Cannot access the specified output file.\n"), errno, errno);
		}
	    else   /* there is already a .bak file for that file           */
		{
		if (accessx (backupfile, W_ACC, ACC_SELF) != 0)
		    cleanfatal (MSGSTR(M_NO_WRITE_P,"0611-457 You must have write permission for file %s.\n"), backupfile, errno);

		if (unlink (backupfile) == ERROR)
		    cleanfatal (MSGSTR(M_UNLINK,"0611-458 Cannot unlink %s.\n"), backupfile, errno);
		}
	    if (link (outputfile, backupfile) == ERROR)
		cleanfatal (MSGSTR(M_LINK,"0611-459 Cannot link %1$s to %2$s.\n"), outputfile, backupfile);
	    if (unlink (outputfile) == ERROR)
		cleanfatal (MSGSTR(M_UNLINK,"0611-458 Cannot unlink %s.\n"), outputfile);
	    }

	/* Move the contents of the temporary output file to the real one    */
	if (link (tempfile, outputfile) == ERROR)
	    cleanfatal (MSGSTR(M_LINK,"0611-459 Cannot link %1$s to %2$s.\n"), tempfile, outputfile);
	/* Get rid of the temporary file...                                  */
	if (unlink (tempfile) == ERROR)
	    fatal (MSGSTR(M_UNLINK,"0611-458 Cannot unlink %s.\n"), tempfile);
	}
    catclose(g_catd);
    exit (0);
    return(0); /* return put here to make lint happy */
    }

/****************************************************************************/
/* readfile:  prints structured file on standard input.                     */
/****************************************************************************/

void readfile (char *filename)
    /* filename:                 file name to print         */

    {
    register indx;           /* record index               */
    register SFILE *sfp;      /* sfile object for this file */
    register RECORD *records; /* record array pointer       */
    register char *object;    /* record object              */

#ifdef DEBUG
    debug ("readfile called w/ argument 'filename' = '%s'",
	      filename);
    debug ("and externals dumpfile = %d, graphics = %d, \
messages = %d, treeprint = %d, indfac = %d, msgcat = %d",
	       dumpfile, graphics, messages, treeprint, indfac, msgcat);
#endif


    /* if non ascii & not a valid structured file       */
    if ((sfp = filecheck (filename)) == (SFILE *) ERROR)
	return;

    /*  if ascii text                           */
    if (sfp == NULL)
	{
	cat (filename);
	return;
	}

    records = sf_records (sfp);

    if (dumpfile || treeprint)
	(void) fprintf (outfp, MSGSTR(M_NUMRECS,"File %1$s contains %2$d records.\n\n"),
		filename, obj_count (sf_records (sfp)) );

    if (treeprint && obj_getflag (records, RC_NONTEXT))
	{
	for (indx = startrec; indx < obj_count (records); indx++)
	    {
	    object = sf_read (sfp, indx);
	    if (object == (char *) ERROR)
		{
		if (messages)
		     (void) fprintf (stderr, MSGSTR(M_BROKEN,"0611-460 File %s is not functional.\n"), filename);
		(void) sf_close (sfp);
		return;
		}

	    if (msgcat)
		object = catscan((POINTER *) object, msg_catd, msgcatset);

	    tree ((POINTER *) object, 0, indx, ONLYSON);  /* do tree diagram */
	    s_free (object);
	    }
	}
    else if (dumpfile || obj_getflag (records, RC_NONTEXT))
	{
	for (indx = startrec; indx < obj_count (records); indx++)
	    {
	    object = sf_read (sfp, indx);
	    if (object == (char *) ERROR)
		{
		if (messages)
		     (void) fprintf (stderr, MSGSTR(M_BROKEN,"0611-460 File %s is not functional.\n"), filename);
		(void) sf_close (sfp);
		return;
		}
	    (void) fprintf (outfp, MSGSTR(M_REC,"{Record %d} "), indx);

	    if (msgcat)
		object = catscan((POINTER *) object, msg_catd, msgcatset);

	    obj_print(0, (POINTER *)object);
	    (void) fprintf (outfp, "\n\n");
	    s_free (object);
	    }
	}
    else  /* structured text file and !dumpfile              */
	  /* simulate a cat of the file                         */
	{
	for (indx = startrec; indx < obj_count (records); indx++)
	    {
	    object = sf_read (sfp, indx);
	    if (object == (char *) ERROR)
		{
		if (messages)
		     (void) fprintf (stderr, MSGSTR(M_BROKEN,"0611-460 File %s is not functional.\n"),
			      filename);
		(void) sf_close (sfp);
		return;
		}

	    if (msgcat)
		object = catscan((POINTER *) object, msg_catd, msgcatset);

	    if (graphics)              /* convert graphics chars in string to */
		graphfilter (object);  /* printable chars that resemble them */
	    (void) fprintf (outfp, "%s\n", object ? object : "");
	    s_free (object);
	    }
	}
    (void) sf_close (sfp);
    }

/****************************************************************************/
/* obj_print:  recursively print a structured allocation                    */
/*                                                                          */
/* A structured allocation is a tree structure whose nodes can be of one of */
/*     the following two types:                                             */
/*     T_CHAR -- contains header information & a character array.  Always   */
/*         a leaf                                                           */
/*     T_POINTER -- contains header information and an array of pointers    */
/*         which may point to other structured allocations                  */
/*                                                                          */
/*     The source code for the implementation of these data structures is   */
/*         in stree.c in the /lib/obj library.  A detailed description can  */
/*         be found in the Helper Writer's Guide pp 9-1 and in the          */
/*         "Database Library Reference Manual" in the same publication      */
/*                                                                          */
/*                                                                          */
/*     The data structure for a structured file is an array of "records" -- */
/*     pointers to locations in the file where these structured allocations */
/*     are written out.                                                     */
/*                                                                          */
/*     The source code for the implementation of the structured file data   */
/*     structure is in 'sfile.c' in the /lib/obj library and structures and */
/*     macros can be found in <database.h>.  A detailed description can be  */
/*     found in "Database Library Reference Manual".                        */
/****************************************************************************/


void obj_print (int indent, POINTER *object)
    /* indent:      current level of indentation                */
    /* object:      object to print                             */
    {
    register i;               /* index into object data array   */
    register type;            /* object type                    */
    register char *name;      /* object name                    */
    int objcount;             /* object count (# of children)   */
    int flags;                /* object flags                   */

    nputc (SPACE, outfp, indent);  /* indent to current level */

    if (object == NULL)
	{
	(void) fprintf (outfp, MSGSTR(M_NULL_PTR,"NULL pointer\n"));
	return;
	}
    if (object == (POINTER *) ERROR)
	{
	(void) fprintf (outfp, MSGSTR(M_ILL_OBJ,"The object type is not recognized.\n"));
	return;
	}
    type = obj_type (object);
    name = obj_name (object);
    objcount = obj_count (object);
    flags = obj_flag (object);

#ifdef DEBUG
    debug ( "obj_print called w/ arguments indent = %d, \
	     indfac= %d, object w/ name = '%s'",
	     indent, indfac, name);
    debug ("and externals dumpfile=%d, graphics=%d, indfac = %d",
		dumpfile, graphics, indfac);
#endif

    if ((type != T_POINTER) && (type != T_CHAR))
	{
	(void) fprintf (outfp, MSGSTR(M_UNK_NODE,"Type code %1$d is not recognized for this node.\n\
	The count value is %2$d.\n"), type, objcount);
	return;
	}

    if (dumpfile)
	{
	if (type == T_POINTER)
	    (void) fprintf (outfp, MSGSTR(M_PTR_TYPE,"Type = T_POINTER, Name = '%1$s', Size = %2$d, Flags =0%3$o\n"),
		     name ? name : "(null)", objcount, flags);
	else
	    (void) fprintf (outfp, MSGSTR(M_CHAR_TYPE,"Type = T_CHAR, Name = '%1$s', Size = %2$d, Flags = 0%3$o\n"),
		     name ? name : "(null)", objcount, flags);
	}
    else     /* readfile */
	if (type == T_POINTER)
	    (void) fprintf (outfp, MSGSTR(M_PTR_SHORT,"Type = pointer, Name = %s:\n"), name ? name : "(null)");
	else /* type is T_CHAR */
	    (void) fprintf (outfp, MSGSTR(M_CHAR_SHORT,"Type = character, Name = %s:\n"), name ? name : "(null)");


    /* If the node is a pointer array, recurse on each child   */
    /* with the indentation level increased by indent factor   */
    /* If the node has a character array, it is a leaf, so     */
    /* print out the line of text, also indented, & return     */

    indent += indfac;

    if (type == T_POINTER)
	{
	for (i = 0; i < objcount; i++)
	    obj_print (indent, (POINTER *)object [i]);
	}
    else   /* else type is  T_CHAR */
	{
	nputc (SPACE, outfp, indent);   /* indent to current indentation level */

	if (graphics)
	  graphfilter ((char *)object);/* convert graphics chars in string to */
				       /* similar printing chars              */
	(void) fprintf (outfp, "'%s'\n", object);
	}
    }

/****************************************************************************/
/* cat:  print out normal non-delta text file                               */
/****************************************************************************/

void cat (char *filename)
    /* filename:        name of file to cat             */
    {
    register FILE *fp;
    register int c;


#ifdef DEBUG
    debug ("cat called with globals messages = %d, graphics = %d\n",
	    messages, graphics);
    debug ("and argument filename ='%s'\n",
	    filename);
#endif
    if (filename)
	{
	fp = fopen (filename, "r");

	if (fp == NULL && messages)
	    (void) fprintf (stderr, MSGSTR(M_OPEN,"readfile: 0611-461 Cannot open file %s.\n"), filename);
	}
    else
	fp = stdin;


    for (;;)
	{
	c = getc (fp);

	if (feof (fp))
	    {
	    if (filename)
		(void)fclose (fp);
	    break;
	    }

	if (graphics)  /* if graphics chars are to be changed to printing ones*/
	    {
	    c = gconvert ((char)c);
	    }
	putc (c, outfp);

	}
    }


/****************************************************************************/
/* graphfilter: changes every instance of graphics char in string to print- */
/*    able one that resembles the graphics char                             */
/*                                                                          */
/*   By graphics chars, we mean the ones used for making boxes in .frm file */
/****************************************************************************/
void graphfilter (char *string)
    {
#ifdef DEBUG
    debug ("graphfilter called with string = '%s'  string value = %d", string, string);
#endif

    if (string == NULL)
         return;

    while ((*string = gconvert (*string)))
	skipmb(&string);

#ifdef DEBUG
    debug ("graphfilter ended with string = '%s'", string);
#endif

    }

/****************************************************************************/
/* gconvert:  converts its argument to a printing character if it is a      */
/*            grapics char of the types used in forms.  Otherwise it return */
/*            its arg unchanged.  The printing characters used to replace   */
/*            the graphics characters are chosen to resemble those          */
/*            characters.                                                   */
/****************************************************************************/

char gconvert (char c)
    {

#ifdef DEBUG
    debug ("gconvert called with c = %c (%d)", c, c);
#endif



    switch (c)
	{
	case control ('s'):     /* upper left corner            */
	case control ('d'):     /* upper right corner           */
	case control ('f'):     /* lower left corner            */
	case control ('g'):     /* lower right corner           */
	case control ('z'):     /* cross                        */

	    return ('+');

	case control ('q'):     /* horizontal bar               */
	case control ('t'):     /* T joint                      */
	case control ('r'):     /* inverse T                    */

	    return ('-');

	case control ('a'):     /* vertical bar                 */
	case control ('w'):     /* left T                       */
	case control ('e'):     /* right T                      */

	    return ('|');

	default:                /* not a graphics char          */
#ifdef DEBUG
    debug ("gconvert default case taken. c = %c (%d)", c, c);
#endif

	    return (c);

	}
    }


/****************************************************************************/
/* filecheck:   returns an sfp pointer  if <filename> is structured,        */
/*              ((SFILE *) NULL) if <filename> is ascii text,               */
/*              ((SFILE *) ERROR) otherwise (e.g. if it's an object file)   */
/*                                                                          */
/* When <messages> flag is TRUE, diagnostic messages about the file are     */
/* printed                                                                  */
/****************************************************************************/


SFILE *filecheck (char *filename)
    /* filename:                   file to check                */
    {
    register FILE *fp;          /* text file pointer            */
    struct stat buffer;         /* stat call buffer             */
    register int i;             /* looping variable             */
    register wchar_t c;
    SFILE *sfp;                 /* return value                 */



#ifdef DEBUG
    debug ("filecheck called w/ filename = '%s'", filename);
    debug ("and external messages = %d", messages);
#endif

    /* bad permission or nonexistent file ?             */
    if (stat (filename, &buffer) == ERROR)
	{
	if (messages)
	    (void) fprintf (stderr, MSGSTR(M_NO_FILE,"readfile: 0611-462 File %s does not exist or permission is denied.\n"), filename);
        catclose(g_catd);
	exit(errno);
	/*return((SFILE *) ERROR);*/
	}

    /* the macros S_IFMT and S_IFREG are defined in /usr/include/sys/stat.h.
	  The meaning of the values they contain is explained in mknode(2)
	  of the IS/3 man. The st_mode of a stat struct is explained in
	  stat(2) of the manual.                                     */

    if ((buffer.st_mode & S_IFMT) != S_IFREG)
	{
	if (messages)
	    (void) fprintf (stderr, MSGSTR(M_NOT_NORMAL,"readfile: 0611-463 File %s is not a normal file.\n\
	    \tSpecify only normal files (not devices) to readfile.\n"), filename);
	return ((SFILE *) ERROR);
	}

    /*  if delta file */
    if (isdelta (filename))
	{
	sfp = sf_open (filename, "r");

	if (sfp == (SFILE *) ERROR)      /* check for all things that might */
	    {                            /* be broken about the file        */
	    switch (g_errno)
		{
		case E_IO:
		    if (messages)
			{
			(void) fprintf (stderr, "===============================================\n");
			(void) fprintf (stderr, MSGSTR(M_IO_ERROR,"readfile: Internal Error (i/o error in sf_open on file %s).\n"), filename);
			(void) fprintf (stderr, "===============================================\n");
			}
		    return ((SFILE *) ERROR);
		case E_FOPEN:
		    if (messages)
			{
			(void) fprintf (stderr, "===============================================\n");
			(void) fprintf (stderr, MSGSTR(M_EXIST,"readfile: 0611-464 File %s does not exist.\n"), filename);
			(void) fprintf (stderr, "===============================================\n");
			}
		    return((SFILE *) ERROR);

		case E_ARRAY:
		    if (messages)
			{
			(void) fprintf (stderr, "===============================================\n");
			(void) fprintf (stderr, MSGSTR(M_REC_ARRAY,"readfile: 0611-465 A record array in file %s is not functional.\n"), filename);
			(void) fprintf (stderr, "===============================================\n");
			}
		    return((SFILE *) ERROR);

		default:
		    if (messages)
			{
			(void) fprintf (stderr, "===============================================\n");
			(void) fprintf (stderr, MSGSTR(M_OPEN,"readfile: 0611-461 Cannot open file %s.\n"), filename);
			(void) fprintf (stderr, "===============================================\n");
			}
		    return((SFILE *) ERROR);
		}   /* end switch */
	    }   /*  end if        */

	    if ((sf_records(sfp)) == NULL)
		{
		if (messages)
		    {
		    (void) fprintf (stderr, MSGSTR(M_REC_READ,"0611-466 Cannot read records from file %s.\n"), filename);
		    (void) fflush ((FILE *)sobuf);
		    }
		(void) sf_close (sfp);
		return((SFILE *) ERROR);
		}
	else   /* non-busted delta file         */
	    return (sfp);
	}

    else  /* not a delta file */
	{

	fp = fopen (filename, "r");

	if (fp == NULL)
	    {
	    if (messages)
		(void) fprintf (stderr, MSGSTR(M_NO_FILE,"readfile: 0611-462 File %s does not exist or permission is denied.\n"), filename);
	    return ((SFILE *) ERROR);
	    }

	/*  scan first 100 wide chars of file looking for chars unlikely to be in  */
	/*  files you would want to read out, such as object files            */
	for (i = 0; i < 100; i++)
	    {
	    c = getwc (fp);
	    /* if end of file reached */
	    if (feof (fp))
		break;

	    /* make sure current char 'c' is on list of legitimate characters  */
	    /* for a printable file.  If not, return ERROR                     */

	    /* if printing ascii character */
	    if (iswprint (c))
		continue;

	    switch (c)
		{
		/* file formatting characters                               */
		case L'\n':                  /* newline   -- octal 12        */
		case L'\t':                  /* tab       -- octal 11        */
		case L'\f':                  /* form feed -- octal 14        */
		case L'\b':                  /* backspace -- octal 10        */

		/* graphics chars used in forms files                       */
		case control (L's'):         /* upper left corner            */
		case control (L'd'):         /* upper right corner           */
		case control (L'f'):         /* lower left corner            */
		case control (L'g'):         /* lower right corner, also bell*/
		case control (L'z'):         /* cross                        */
		case control (L'q'):         /* horizontal bar               */
		case control (L't'):         /* T joint                      */
		case control (L'r'):         /* inverse T                    */
		case control (L'a'):         /* vertical bar                 */
		case control (L'w'):         /* left T                       */
		case control (L'e'):         /* right T                      */

		    break;

		default:
		    if (messages)
			{
			(void) fprintf (stderr, MSGSTR(M_BAD_CHAR,"readfile: 0611-467 File %1$s contains character 0%2$o which is not valid.\n"),
			    filename, c&0xff);
			(void) fprintf (stderr, MSGSTR(M_NON_PRINTABLE,"readfile: Assuming that the input file is not of a printable type.\n"));
			}
		    (void)fclose (fp);
		    return ((SFILE *) ERROR);
		}   /* end switch           */
	    }   /*  end for                 */

	/* then it's a printable ascii file                                 */
	(void)fclose (fp);
	return ( NULL);
	}   /* end else */
    }   /* end filecheck */


/****************************************************************************/
/* basename:  Inputs pathname to a file or directory & returns the basename */
/****************************************************************************/

char *basename(char *string)
    {
    char *s;                            /* where basename starts in s1       */

    if (s = strrchr (string, DIR_SEPARATOR)) /* find last occurrence of slash         */
	{
	return (++s);              /* basename starts right  after that DIR_SEPARATOR */
	}
    else                           /* else only basename was input          */
	{
	return (string);
	}
    }


/****************************************************************************/
/* dir_name:  extracts parent directory from second arg, which is a pathname */
/*             and copies it into location pointed to by first arg.  Return */
/*             a pointer to that location, or NULL if only a basename is    */
/*             given in the second argument                                 */
/****************************************************************************/

char *dir_name(char *directory, char *string)
    {
    char *s;

    (void) strcpy (directory, string);         /* _d_i_r_e_c_t_o_r_y a copy of _s_t_r_i_n_g     */

    if (s = strrchr (directory, DIR_SEPARATOR))  /* find last occurrence of slash  */
	{
	*s = '\0';                   /* make _d_i_r_e_c_t_o_r_y end before last DIR_SEPARATOR*/
	return (directory);
	}
    else                              /* else return NULL               */
	{
	return (NULL);
	}
    }


/****************************************************************************/
/* interrupt:  when interrupt signals are caught, interrupt removes tmp     */
/*     files before allowing the program to exit                            */
/****************************************************************************/

void interrupt (void)
    {
    (void)unlink (tempfile);
    catclose(g_catd);
    exit (-1);
    }

/****************************************************************************/
/* cleanfatal:  gets rid of temporary file before doing a fatal             */
/****************************************************************************/

void cleanfatal (char *fmt, ...)
    {
    va_list ap;  
    char *a; char *b;
    
    (void)unlink (tempfile);
   
    va_start(ap, fmt);
    a = va_arg(ap, char *); 
    b = va_arg(ap, char *);
    fatal (fmt, a, b);
    va_end(ap);
    }
