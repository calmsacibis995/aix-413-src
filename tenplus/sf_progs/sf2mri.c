#if !defined(lint)
static char sccsid[] = "@(#)05	1.8  src/tenplus/sf_progs/sf2mri.c, tenplus, tenplus411, GOLD410 7/18/91 13:53:37";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED)  INed Editor
 *
 * FUNCTIONS:	main, sf2mri, add_process_path, cleanup,
 *		print_set_code, message_token, print_object, basename,
 *		fatal, advise, isnumber, print_mri_string
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
 
 /***************************************************************************/
 /* sf2mri.c -- read in a structured file, and create an MRI message */
 /* (.msg format) file out of it. */
 /* Usage:  sf2mri.c [-c] [-o outputfile] [-t datapath_to_translate]... */
 /*                  [-n datapath_to_not_translate]... */
 /*                  [-q datapath_to_query_user_about]... */
 /*                  [-s datapath_to_use_as_set_label]... */
 /*                  [input_structured_file] */

 /* [ The -q options isn't implemented yet.  It may never be. ] */
 /* The idea is that it is called from a shell script that includes */
 /* the right options for translating some particular flavor of */
 /* structured file (e.g. .hmg file, .cmds file, etc) */
 /* Each record in the structured file gets turned into one message */
 /* set in the MRI file, even if the set ends up containing no */
 /* messages */
 /* -c says to compress out null records at the top level of the file */
 /***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <locale.h>


/* Ined Include */
#include <chardefs.h>
#include <database.h>
#include "mri.h"
#include <libobj.h>

 /* We support switching "fonts".  In the structured file, the */
 /* "underscore" font is represented by preceding each character with */
 /* "backspace-underscore",  in the MRI file we put a \_ at the */
 /* beginning of the underscored text, and a \+ at the end. */

#define	FONT_UNDERSCORE	'_'
#define	FONT_NORMAL	'+'

/* copyright info to put into MRI catalog */
char *cright[] = {
  "COMPONENT_NAME: (INED) INed Editor",
  "",
  "ORIGINS:  9, 10, 27",
  "",
  "This module contains IBM CONFIDENTIAL code. -- (IBM",
  "Confidential Restricted when combined with the aggregated",
  "modules for this product) OBJECT CODE ONLY SOURCE MATERIALS",
  "(C) COPYRIGHT International Business Machines Corp. 1985, 1990",
  "All Rights Reserved",
  "",
  "US Government Users Restricted Rights - Use, duplication or",
  "disclosure restricted by GSA ADP Schedule Contract with IBM Corp.",
};


char	*set_path = NULL;	/* Data path whose contents should be */
				/* used as the name of the message set */
				/* in the MRI message catalog */

SFILE	*infp = NULL;
FILE	*outfp = NULL;

 /* We keep track of which objects to print to the MRI message catalog */
 /* by building a structured tree, called paths_to_process, that has */
 /* the same shape as the actual structured tree that we want to print */
 /* out.  We then traverse the two trees in parallel.  In order to */
 /* know how to print the text at each node of interest (either for */
 /* translation or not for translation), we set one of the following */
 /* flags in the object in the paths_to_process tree.  If both bits */
 /* are set, it means that the user should be queried as to whether */
 /* the object should be translated or not. */
 /* (Querying isn't implemented yet) */
#define	XLATE_BIT	0x1
#define NO_XLATE_BIT	0x2
#define	COLLECTION_BIT	0x4	/* Indicates that the node is the */
				/* parent of a collection of text */
				/* nodes that should be handled as a */
				/* unit in the MRI file */
POINTER	*paths_to_process = NULL;

char	*progname;		/* points at argv[0] */
#ifndef	BOOLEAN
#define	BOOLEAN	int
#endif	/* BOOLEAN */

int getopt(int, char **, char *);

/** Local functions **/
static char * basename(char *);


void	sf2mri(POINTER *, int);
void	print_set_code(POINTER *);
void	print_object(POINTER *, char *, POINTER *);
void	fatal(char *, ...);
void	advise(char *, ...);
void	add_process_path(char *, char);
POINTER	*cleanup(POINTER *, char *);
char	*message_token(void);
BOOLEAN	print_mri_string(char *);
int	isnumber(char *);

int MultibyteCodeSet;

main (int argc, char **argv)
{
  extern char	*optarg;
  extern int	optind;
  char		*outfile = NULL;
  char		*infile = NULL;
  int		c;
  int		i;
  int		compress = FALSE;

  (void) setlocale(LC_ALL, "");

  if (MB_CUR_MAX == 1)
       MultibyteCodeSet = 0;
  else MultibyteCodeSet = 1;
    
#ifdef DEBUG
  g_debugfp = fopen ( "sf2mri.out", "w" ); 
  if (g_debugfp == NULL) {
    (void) fprintf (stderr, "Unable to open debugging output file\n" );
    exit ( -1 );
  }
#endif	/* DEBUG */

  progname = argv[0];

  while ((c = getopt (argc, argv, "co:t:n:q:s:")) != EOF) {
    switch (c) {
    case 'c':
      compress = TRUE;
      break;

    case 'o':
      if (outfile != NULL)
	  fatal ("-o option specified more than once on command line");
      outfile = optarg;
      break;

      /* record a path as one that should be printed out, for */
      /* translation */
    case 't':
      add_process_path (optarg, (char)XLATE_BIT);
      break;

      /* record a path as one that should be printed out, not for */
      /* translation */
    case 'n':
      add_process_path (optarg, (char)NO_XLATE_BIT);
      break;

    case 'q':
      fatal ("the -q option is not supported yet");
      paths_to_process = (POINTER *) s_mknode (paths_to_process,
					       optarg, T_POINTER); 
      obj_setflag (s_findnode (paths_to_process, optarg),
		   (NO_XLATE_BIT | XLATE_BIT));
      break;

    case 's':
      if (set_path != NULL)
	  fatal ("-s option specified more than once on command line");

      /* If the string has a leading /, skip over it */
      if (*optarg == DIR_SEPARATOR)
	  optarg++;
      /* skip over record name, or "*" */
      set_path = strchr (optarg, DIR_SEPARATOR) + 1;
      if (set_path == NULL)
	  /* Weird.  Well, try to use what user gave us. */
	  set_path = optarg;
      break;

    default:
      fatal ("Illegal command line option '-%c' specified", c);
    } /* switch (c) */
  } /* while ((c = getopt (argc, argv, "o:t:n:q:s:")) != EOF) */

  for ( ; optind < argc; optind++) {
    if (infile != NULL)
	fatal ("more than one input file specified on command line");
    infile = argv[optind];
  } /* for ( ; optind < argc; optind++) */

  if (infile == NULL)
      fatal ("no input file name specified");
  else
      /* open input structured file */
      if ((infp = sf_open (infile, "r")) == (SFILE *) ERROR)
	  fatal ("could not open input file \"%s\", or it is not a structured file",
		 infile);

  if (outfile == NULL)
      outfp = stdout;
  else
      if ((outfp = fopen (outfile, "w")) == NULL)
	  fatal ("could not open output file \"%s\"", outfile);

  /* Put name of structured file into MRI catalog */
  (void) fprintf (outfp, "%s%c%s\n", HCOMMENT, NAME_FLAG, basename (infile));

  /* Print out standard header info (SCCS line, copyright) */
  /* We have to do a little work keep sccs from changing these */
  /* keywords when we admin/delta this file. */
  /* This particular line looks at first glance to be all wrong, */
  /* because the W key letter is normally expanded to the same thing */
  /* as ZM<tab>I, which is the first part of this string.  However, */
  /* the version of SCCS that is being used is customized to give the */
  /* W key letter a special meaning, so it is not redundant. */
#define	PC '%'
  (void) fprintf (outfp,
		  "%s%c%cZ%c%cM%c	%cI%c  %cW%c %cG%c %cU%c\n\n",
		  HCOMMENT, COMMENT_FLAG,
		  PC,PC,PC,PC,PC,PC,PC,PC,PC,PC,PC,PC); 

  /* Print out the standard copyright boilerplate */
  for (i = 0; i < (sizeof (cright) / sizeof (cright[0])); i++)
       (void) fprintf (outfp, "%s%c%s\n", HCOMMENT, COMMENT_FLAG, cright[i]);

  /* Tell the MRI software that " is the quote character */
  (void) fprintf (outfp, "$quote \"\n");

  /* process the contents of the structured file */
  sf2mri (paths_to_process, compress);

  (void) sf_close (infp);
  exit (0);
  return(0); /* return put here to make lint happy */
} /* main () */



 /***************************************************************************/
 /* sf2mri() -- actually do the work of translating the contents, once */
 /* the initialization has been done. */
 /* This routine loops through all the records in the structured file. */
 /***************************************************************************/

void sf2mri (POINTER *paths_to_procss, int compress)
{
  int		record_number, object_number, nrecords;
  POINTER	*record;	/* Structured record being converted */

  /* Loop through the records in the file */
  nrecords = obj_count (sf_records (infp)); 
  for (record_number = 0;
       record_number < nrecords;
       record_number++) { 

    record = (POINTER *) sf_read (infp, record_number);
    if (record == (POINTER *) ERROR)
	fatal ("error reading record number %d from structured file",
	       record_number);

    /* cheerfully ignore null records */
    if (record == NULL && compress) {
      advise ("ignoring null record number %d", record_number);
      continue;
    }

#ifdef	DEBUG
    debug ("read the following object\n");
    s_print (record);
#endif	/* DEBUG */

    /* tidy up the structured tree */
    record = cleanup (record, "");

    /* Print out the MRI message set code */
    print_set_code (record);

    /* we have to loop through the first-level children of */
    /* paths_to_procss here because these path names could have */
    /* different first components, which would correspond to different */
    /* record names in the structured file. */
    /* In other words, the top node of paths_to_procss corresponds to */
    /* the whole structured file that we are translating; the next */
    /* level down corresponds to the individual records within the */
    /* file. */
    /* Generally, all the paths to process will start with "*", so */
    /* this loop will only be executed once. */
    for (object_number = 0;
	 object_number < obj_count (paths_to_procss);
	 object_number++)
	print_object (record, "", 
                      (POINTER *)paths_to_procss[object_number]);

    s_free ((char *)record);
  } /* for (record_number = 0; ... */

} /* sf2mri() */



 /***************************************************************************/
 /* add_process_path -- add a path to the tree that describes which */
 /* data paths in the structured file to process */
 /***************************************************************************/

void add_process_path(char *string, char flagbits)
{
  int		slen;

  slen = strlen (string);
  /* If the string has a trailing *, then don't mark the leaf node, */
  /* but rather mark the node one above it as being a collection of */
  /* text strings that all belong together */
  if ((slen > 2) && ((strcmp("/*", &(string[slen -2])))==0)) {
    string[slen -2] = '\0';
    flagbits |= COLLECTION_BIT;
  } /* if .. */
  paths_to_process = (POINTER *) s_mknode (paths_to_process, string,
					   T_POINTER);
  obj_setflag (s_findnode (paths_to_process, string), flagbits);

} /* add_process_path() */

 /***************************************************************************/
 /* cleanup -- cleans up a structured tree, by trimming off null objects, */
 /* and condensing arrays of one text line down to just the text line */
 /* alone. */ 
 /* It returns a pointer to the object, which may be different from */
 /* the pointer that was passed in */
 /***************************************************************************/

POINTER *cleanup (POINTER *object, char *path)
{
  int		i;
  POINTER	*temp;
  char		fullpath[200];
  BOOLEAN	discard_nulls;

  if ((object == NULL) || (obj_type (object) != T_POINTER))
      return (object);

  (void) strcpy (fullpath, path);
  (void) strcat (fullpath, "/");
  if (obj_name (object) != NULL)
      (void) strcat (fullpath, obj_name (object));
  else
      (void) strcat (fullpath, "*");

  /* We only discard null array entries if all the entries in this */
  /* array have non-null names, i.e. are not position-sensitive. */
  discard_nulls = TRUE;
  for (i = 0; i < obj_count (object); i++) {
    /* recurse down the tree */
    if (object[i] != NULL)
	object[i] = (POINTER) cleanup ((POINTER *)object[i], fullpath);

    /* Note that the above call may cause object[i] to become NULL */
    if ((object[i] != NULL) && (obj_name (object[i]) == NULL))
	discard_nulls = FALSE;
  } /* for ... */

  for (i = 0; i < obj_count (object); i++) {
    if (object[i] == NULL) {
      if (discard_nulls) {
	advise ("discarding NULL object (entry number %d in object \"%s\")",
		i, fullpath);
	object = (POINTER *) s_delete ((char *)object, i, 1);
	i--;		/* step backwards so as to examine the new */
			/* object in this position */
      } /* if (discard_nulls ... */
    } /* (object[i] == NULL) */
  } /* for (i = 0; i < obj_count (object); i++) */


  /* If this is an array of one character string, make it just be the */
  /* character string. */
  if ((obj_count (object) == 1) && (obj_type (object[0]) == T_CHAR)
      && (isnumber (obj_name (object[0])))) {
    advise ("trimming one-entry array of text lines down to an");
    advise ("individual text line; path = \"%s\"", fullpath);
    advise ("text = \"%s\"", object[0]);
    /* fix up the name on the object being pulled up one level */
    s_newname (object[0], obj_name(object));
    temp = (POINTER *) object[0];
    s_freenode ((char *)object);
    object = temp;
  } /* if ((obj_count (object) == 1) && ... */
  /* else if this is an empty array, free it and return NULL */
  else if (obj_count (object) == 0) {
    s_free ((char *)object);
    object = NULL;
  } /* if (obj_count (object) == 0) */

  return (object);
} /* cleanup () */


 /***************************************************************************/
 /* print_set_code -- print out the MRI set string (or number) for */
 /* this record. */
 /***************************************************************************/

/* These are used by message_token(), below */
static char	token_c1 = 'A';
static char	token_c2 = 'A';

void print_set_code (POINTER *record)
{
  static int	set_number = 1;
  char		*set_name;

#ifdef	DEBUG
  debug ("print_set_code got the following record:\n");
  if (record == NULL)
      (void) printf ("NULL object\n");
  else
      s_print (record);
#endif	/* DEBUG */

  /* If the user didn't specify the data path of an object to use as */
  /* the set name, or if that field is missing, just use numbers */
  if (set_path == NULL)
      (void) fprintf (outfp, "\n$set %d\n", set_number++);

  else if ((set_name = s_findnode (record, set_path)) == (char *) ERROR) {
    advise ("No field \"%s\" to use as a set name, using a number instead",
	    set_path);
    (void) fprintf (outfp, "\n$set %d\n", set_number++);
  }	    

  else
    (void) fprintf (outfp, "\n$set %s\n", set_name);

} /* print_set_code () */


 /***************************************************************************/
 /* message_token -- generate a value to use as a message token. */
 /* These values have no meaning; they are just generated */
 /* sequentially.  message_token() always returns a pointer to a */
 /* static string that is 3 characters long:  two characters of token, */
 /* and a space. */
 /* This can produce (26 * 2)**2 = 2704 different values */
 /***************************************************************************/

char *message_token(void)
{
  static char	token_str[PREFIX_LEN + 1];

  token_str[0] = token_c1;
  token_str[1] = token_c2;
  token_str[2] = SPACE;
  token_str[3] = '\0';

#ifdef LOTSA_MESSAGES
  if (++token_c2 == 'Z' + 1)
      token_c2 = 'a';
  if (token_c2 == 'z' + 1) {
    token_c2 = 'A';
    if (++token_c1 == 'Z' + 1)
	token_c1 = 'a';
    if (token_c1 == 'z' + 1)
	fatal ("too many messages");
  } /* if (token_c2 == 'z') */
#else  
  if (++token_c2 == 'Z' + 1) {
    token_c2 = 'A';
    if (++token_c1 == 'Z' + 1)
	fatal ("too many messages");
  } /* if (token_c2 == 'z') */
#endif
  
  return (token_str);
} /* message_token() */




 /***************************************************************************/
 /* print_object -- print out the structured tree rooted at object, */
 /* based on the the paths_to_process specification tree */
 /* This implements a dual, parallel, recursive algorith, by searching */
 /* down both the object tree and the paths_to_process tree */
 /* simultaneously. */
 /* NULL object pointers are treated as objects of type T_FREED */
 /***************************************************************************/

void print_object (POINTER *object, char *path_to_here, 
                   POINTER *paths_to_procss)
{
  char		current_path[200];
  int		action, i, j;
  char		objtype, objflags;
  BOOLEAN	found_underscore;

#ifdef	DEBUG
  debug ("print_object:  path_to_here = %s\n", path_to_here);
  debug ("print_object got the following object:\n");
  if (object == NULL)
      (void) printf ("NULL object\n");
  else
      s_print (object);
  debug ("print_object got the following paths_to_procss:\n");
  if (paths_to_procss == NULL)
      (void) printf ("NULL object\n");
  else
      s_print (paths_to_procss);
#endif	/* DEBUG */


  if (paths_to_procss == NULL) {
#ifdef DEBUG
    debug ("print_object returning");
#endif	/* DEBUG */
    return;
  }

  /* if the paths_to_procss name at this level is "*" (indicating */
  /* that any name is acceptable), or the paths_to_procss name */
  /* matches the object name, then process this object */
  if ((strcmp(obj_name (paths_to_procss), "*"))==0 ||
      ((object != NULL) && (obj_name (object) != NULL) &&
       (strcmp(obj_name (paths_to_procss), obj_name (object)))==0)) {

    /* print out a line telling the object type */
    objtype = (object == NULL) ? T_FREED : obj_type (object);
    (void) fprintf (outfp, "%s%c%d\n", HCOMMENT, TYPE_FLAG, objtype);

    /* print out the object flags */
    objflags = (object == NULL) ? 0 : obj_flag (object);
    (void) fprintf (outfp, "%s%c%d\n", HCOMMENT, FLAGS_FLAG, objflags);

    /* build up the full pathname of this object, given path_to_here */
    /* and name of paths_to_procss object.  Note that this means */
    /* that elements of arrays get the name "*", not numbers */
    (void) strcpy (current_path, path_to_here);
    (void) strcat (current_path, "/");
    (void) strcat (current_path, obj_name (paths_to_procss));

    /* print out the object's path */
    (void) fprintf (outfp, "%s%c%s\n", HCOMMENT, PATH_FLAG, current_path);

    /* action tells us whether or not it should be translated to */
    /* non-english language */
    action = obj_flag (paths_to_procss);
    found_underscore = FALSE;	/* say we haven't encountered an */
				/* underscore in the text to translate */
    if (objtype == T_CHAR) {
      /* See if this node in the paths_to_procss tree is marked as */
      /* (1) requiring translation, or (2) not requiring translation, */
      /* or (3) requiring an answer from the user before being */
      /* translated. */

      switch (action & (XLATE_BIT | NO_XLATE_BIT)) {
      case XLATE_BIT:
	(void) fprintf (outfp, "%s%c", message_token(), QUOTE_FLAG);
	found_underscore = print_mri_string ((char *)object);
	(void) fprintf (outfp, "%c\n", QUOTE_FLAG);
	break;

      case NO_XLATE_BIT:
	(void) fprintf (outfp, "%s%c", HCOMMENT, QUOTE_FLAG);
	found_underscore = print_mri_string ((char *)object);
	(void) fprintf (outfp, "%c\n", QUOTE_FLAG);
	break;

      case XLATE_BIT | NO_XLATE_BIT:
	fatal ("user querying not implemented yet");
      } /* switch (action) */
    } /* if (objtype == T_CHAR) */

    if (objtype == T_POINTER) {
      /* If this node is marked in the paths_to_procss tree as a text */
      /* "collection", then print out all it's children text nodes as */
      /* a single quoted string */
      /* However, if there is no text hanging from this object, then */
      /* don't print any for it. */
      if ((action & COLLECTION_BIT) && (obj_count (object) > 0)) {
	switch (action & (XLATE_BIT | NO_XLATE_BIT)) {
	case XLATE_BIT:
	  (void) fprintf (outfp, "%s%c", message_token(), QUOTE_FLAG);
	  break;

	case NO_XLATE_BIT:
	  (void) fprintf (outfp, "%s%c", HCOMMENT, QUOTE_FLAG);
	  break;

	case XLATE_BIT | NO_XLATE_BIT:
	  fatal ("user querying not implemented yet");
	} /* switch ... */

	for (i = 0; i < obj_count (object); i++) {
	  if (object[i] != NULL)
	    found_underscore |= print_mri_string (object[i]);
	  /* print either a line-continuation \n\ or a string-ending " */
	  if (i < obj_count (object) - 1) {
	    (void) fprintf (outfp, "\\n");
	    /* only break into separate lines those messages that will */
	    /* be translated (so that the non-translated ones go */
	    /* through as single-line comments) */
	    if (action & XLATE_BIT)
		(void) fprintf (outfp, "\\\n");
	  }
	  else
	      (void) fprintf (outfp, "%c\n", QUOTE_FLAG);
	} /* for ... */
      } /* if ((action & COLLECTION_BIT) && (obj_count (object) > 0)) */
      else {
	/* This is a regular array object, just process its children */

	/* Print all the appropriate children of the object.  Do it in */
	/* the order specified by paths_to_procss, !in the order */
	/* found in the structured file. */
	/* This has the appearance of an O(N**2) algorithm, but in fact */
	/* at any level of the tree, at least one of the object counts */
	/* will be quite small, so it is OK */
	/* Loop through the paths to be printed */
	for (i = 0; i < obj_count (paths_to_procss); i++) {
	  /* Loop through the objects available */
	  for (j = 0; j < obj_count (object); j++)
	      print_object ((POINTER *)object[j], current_path, 
                            (POINTER *)paths_to_procss[i]);
	} /* for (i ... */

	/* print out end-of-array indicator */
	(void) fprintf (outfp, "%s%c\n", HCOMMENT, ARRAY_END_FLAG);
      }	/* if ((action ... else */
    } /* if (objtype == T_POINTER) */

    /* If we found some underscored text, include a comment to the */
    /* translator explaining how to handle it. */
    if (found_underscore) {
      (void) fprintf (outfp, 
"%c%c TRANSLATOR:  in this message, please include the sequences \\%c\n",
	       MRI_COMMAND_CHAR, MRI_COMMENT_CHAR, FONT_UNDERSCORE);
      (void) fprintf (outfp, "%c%c and \\%c exactly as shown.\n", 
	       MRI_COMMAND_CHAR, MRI_COMMENT_CHAR, FONT_NORMAL);
    } /* if (found_underscore) */
  } /* if ((strcmp ... */

#ifdef DEBUG
  debug ("print_object returning");
#endif	/* DEBUG */
}

 /***************************************************************************/
 /* basename: returns a pointer into the argument string indicating the     */
 /*   name of the file in the current directory.  That is, it strips off    */
 /*   any directory path prefix.                                            */
 /***************************************************************************/

static char *
basename(char *file_name)
{
    register char *bp;              /* Points into the argument string */

    bp = strrchr (file_name, DIR_SEPARATOR);

    if (bp == NULL)
	bp = file_name;
    else
	bp += 1;

    return ( bp );
}


 /***************************************************************************/
 /* fatal -- report a fatal error */
 /***************************************************************************/

void fatal (char *str, ...)
{
  va_list ap;
  char *arg1; char *arg2;

  (void) fprintf (stderr, "%s:  fatal error:  ", progname);

  va_start(ap, str);
  arg1 = va_arg(ap, char *);
  arg2 = va_arg(ap, char *);
  (void) fprintf (stderr, str, arg1, arg2);
  va_end(ap);
  
  (void) fprintf (stderr, "\n");
  exit (1);
}

 /***************************************************************************/
 /* advise -- report an advisory message */
 /***************************************************************************/

void advise (char *str, ...)
{
  char *arg1; char *arg2;
  va_list ap;

  (void) fprintf (stderr, "%s (advisory):  ", progname);
 
  va_start(ap, str);
  arg1 = va_arg(ap, char *); 
  arg2 = va_arg(ap, char *);
  (void) fprintf (stderr, str, arg1, arg2);
  va_end(ap); 

  (void) fprintf (stderr, "\n");
}     

 /***************************************************************************/
 /* isnumber -- returns TRUE if string consists of only the digits 0 - 9 */
 /***************************************************************************/

int isnumber (char *str)
{
  while (*str) {
    /* note that isdigit() should only be called with values that have */
    /* already passed the isascii() test */
    if ((! isascii(*str)) || (! isdigit (*str++)))
	return (FALSE);
  }
  return (TRUE);
} /* isnumber() */



 /***************************************************************************/
 /* print_mri_string -- translates a string into a form appropriate for */
 /* putting in an MRI catalog, and prints it to the outfp */
 /* If any underscored characters are encountered, print_mri_string */
 /* returns TRUE, otherwise FALSE. */
 /***************************************************************************/

BOOLEAN print_mri_string(char *str)
{
  char		font_state = FONT_NORMAL;
  BOOLEAN	did_underscore = FALSE;

  for (; *str != '\0'; str++) {
    /* See if this character is underscored, i.e. preceded by _\b */
    if ((*str == '_') && (str[1] == '\b')) {
      str +=2;			/* step over _\b */
      if (font_state != FONT_UNDERSCORE) {
	(void) putc ('\\', outfp);			/* go into underscore */
	(void) putc (FONT_UNDERSCORE, outfp);		/* font in MRI file. */
	font_state = FONT_UNDERSCORE;
	did_underscore = TRUE;
      }
      if (*str == '\0')		/* check for end of string */
	  break;
    }
    /* it's a normal character, not underscored */
    else if (font_state != FONT_NORMAL) {
      (void) putc ('\\', outfp);			/* go out of underscore */
      (void) putc (FONT_NORMAL, outfp);		/* font in MRI file. */
      font_state = FONT_NORMAL;
    }

    switch (*str) {
	  case '\b':
	    (void) putc ('\\', outfp); (void) putc ('b', outfp); continue;
	  case '\f':
	    (void) putc ('\\', outfp); (void) putc ('f', outfp); continue;
	  case '\n':
	    (void) putc ('\\', outfp); (void) putc ('n', outfp); continue;
	  case '\r':
	    (void) putc ('\\', outfp); (void) putc ('r', outfp); continue;
	  case '\t':
	    (void) putc ('\\', outfp); (void) putc ('t', outfp); continue;
	  case '\\':
	    (void) putc ('\\', outfp); (void) putc ('\\', outfp); continue;
	  case '"':
	    (void) putc ('\\', outfp); (void) putc ('"', outfp); continue;
    } /* switch (*str) */

    if (!(isprint (*str))) {
      (void) fprintf (outfp, "\\%3.3o", *str);
      continue;
    } /* if (! (isprint (*str)) */

  (void) putc (*str, outfp);
  } /* for (; *str != '\0'; str++) */

  if (font_state != FONT_NORMAL) {
    (void) putc ('\\', outfp);			/* go out of underscore */
    (void) putc (FONT_NORMAL, outfp);		/* font in MRI file. */
    font_state = FONT_NORMAL;
  }

  return (did_underscore);
}
