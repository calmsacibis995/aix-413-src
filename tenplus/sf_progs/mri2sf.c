#if !defined(lint)
static char sccsid[] = "@(#)14	1.11  src/tenplus/sf_progs/mri2sf.c, tenplus, tenplus411, GOLD410 3/3/94 15:41:31";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED)  INed Editor
 *
 * FUNCTIONS:	main, do_object, getline, peekline, getline2,
 *		getline_unfiltered, getline_raw, get_quoted,
 *		basename, fatal
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
 /***************************************************************************/
 /* mri2sf -- translate an MRI message catalog that was                     */
 /* produced by sf2mri back into a structured file again.                   */
 /* mri2sf takes a single argument, which is the name of the input mri      */
 /* file.                                                                   */
 /***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <locale.h>

#include "chardefs.h"
#include "database.h"
#include "libobj.h"
#include "mri.h"

#ifndef	BOOLEAN
#define	BOOLEAN	int
#endif	/* BOOLEAN */

#define isodigit(c)	(((c) >= '0') && ((c) <= '7'))

FILE	*infp = NULL;
FILE    *hfp = NULL;            /* Message catalog header file */
char	*progname;		/* points at argv[0] */
int	linecount = 0;

POINTER	*do_object(void);
POINTER *get_quoted(void);
char	getline(char **);	/* NOT char * */
char	peekline(void);	/* NOT char * */
static char	*basename(char *);
char	*getline_unfiltered(void);
char	*getline_raw(void);
BOOLEAN	getline2(void);
char *find_define(char *, FILE *);
char *get_number(char *);

int MultibyteCodeSet;

main (int argc, char **argv)
{
  char		*infile = NULL;
  char		*header = NULL;
  POINTER	*record;
  int		recnum = 0;
  SFILE		*outfp;
  char		*line;

  (void) setlocale(LC_ALL, "");

  if (MB_CUR_MAX == 1)
       MultibyteCodeSet = 0;
  else MultibyteCodeSet = 1;
    
#ifdef DEBUG
  g_debugfp = fopen ( "mri2sf.out", "w" );  
  if (g_debugfp == NULL) {
    (void) fprintf (stderr, "Unable to open debugging output file\n" );
    exit ( -1 );
  }
#endif	/* DEBUG */

  progname = argv[0];

  if ((argc > 3) || (argc == 1))
      fatal ("incorrect command line arguments");

  if (argc >= 2)
      infile = argv[1];

  if (argc == 3)
  {
     header = argv[2];
     if ((hfp = fopen (header, "r")) == NULL)
       fatal ("could not open input file \"%s\"", header);
  }
			    
  if ((infp = fopen (infile, "r")) == NULL)
      fatal ("could not open input file \"%s\"", infile);

  if (getline (&line) != NAME_FLAG)
      fatal ("could not find name of structured file in input file");

  if ((outfp = sf_open (basename(line), "c")) == (SFILE *) ERROR)
      fatal ("could not create output structured file \"%s\"", line);

  while (peekline() != '\0') {
    record = do_object();
    if (sf_write (outfp, recnum++, (char *)record) == ERROR)
	fatal ("error writing to structured file");
    s_free ((char *)record);
  }

  (void) sf_close (outfp);

  exit (0);
  return (0);
} /* main() */

 /***************************************************************************/
 /* do_object() -- recursively read in the definition for a tree of */
 /* objects.  Returns a pointer to the top of the tree */
 /***************************************************************************/

POINTER *do_object(void)
{
  char		*line;
  char		objflags;
  char		objtype;
  char		objname[100];
  char		*objname2;
  		/* Flag on MRI catalog line, */
				/* indicating what kind of thing is on */
				/* this line. */
  POINTER	*object;

  if ((getline (&line)) != TYPE_FLAG)
      fatal ("missing type line");
  objtype = atoi (line);

  if ((getline (&line)) != FLAGS_FLAG)
      fatal ("missing flags line");
  objflags = atoi (line);

  if ((getline (&line)) != PATH_FLAG)
      fatal ("missing datapaty line");

  /* get a copy of the tail end of the data path; that is the object */
  /* name. */

  (void) strcpy (objname, basename (line));

  /* If the object name is "*", it really is null.  Don't make it "", */
  /* make it a NULL pointer. */

  objname2 = (strcmp (objname, "*") == 0) ? NULL : objname;

#ifdef	DEBUG
  debug ("do_object:  objtype = %d, objflags = %d, path = \"%s\"",
	 objtype, objflags, line);
#endif	/* DEBUG */

  switch (objtype) {
    /* NULL objects are represented in the MRI catalog as objects with */
    /* type T_FREED */
  case T_FREED:
    return (NULL);

  case T_CHAR:
    object = get_quoted ();
    break;

  case T_POINTER:
    /* If this object has some quoted text lines associated with it in */
    /* the MRI catalog, then hang them from this object.  Otherwise, */
    /* just create this object, and process all it's children */
    if (peekline() == QUOTE_FLAG)
	object = get_quoted();
    else {
      /* This is a conventional array object; get all its children */
      object = (POINTER *) s_alloc (0, T_POINTER, NULL);
      obj_setflag (object, objflags);
      while (peekline() != ARRAY_END_FLAG)
	  object = s_append (object, (char *)do_object());
      (void) getline (&line);		/* discard the array-end */
					/* indicator line */
    } /* if (peekline() == QUOTE_FLAG) ... else */
    break;

  default:
    fatal ("unknown object type %d", objtype);
  } /* switch (flag) */

  if (objname2 != NULL)
      s_newname ((char *)object, objname2);
  obj_setflag (object, objflags);
  return (object);
} /* do_object() */



 /***************************************************************************/
 /* getline -- get the next input line.  Trim off the prefix, and */
 /* extract the flag character. */
 /* returns the flag character if there is a line to read; otherwise */
 /* returns '\0'.							*/
 /***************************************************************************/

#define	LINEBUFSIZE	(1000)
static char	linebuf[LINEBUFSIZE];
static int	haveline = FALSE;

char getline (char **linep)
{

  if (getline2 ()) {
    /* say that we don't have a line cached up from peeking at it any */
    /* more. */
    /* Say we are using up the line from linebuf */
    haveline = FALSE;

    /*Move the pointer past the prefix and the flag.  */ 
    *linep = linebuf + PREFIX_LEN + 1;

#ifdef	DEBUG
    debug ("get_line getting line \"%s\"", linebuf);
    debug ("get_line:  flag is %c", linebuf[PREFIX_LEN]);
#endif	/* DEBUG */

    /* return the flag character */
    return (linebuf[PREFIX_LEN]);
  } /* if (getline2 ()) */

  return ('\0');
} /* getline() */


char peekline (void)
{
  /* If we have an input line, return the character just after the */
  /* prefix. */
  return (getline2() ? linebuf[PREFIX_LEN] : '\0');
} /* peekline() */

BOOLEAN getline2 (void)
{

  while (getline_raw() != NULL) {
    if (seq (linebuf, "")) {
      /* Say we are using up the line from linebuf */
      haveline = FALSE;
      continue;
    }

    /* If this is a "hidden comment" line */
    if (strncmp (linebuf, HCOMMENT, PREFIX_LEN) == 0) {
      /* look at flag character */
      switch (linebuf[PREFIX_LEN]) {
      case COMMENT_FLAG:
	/* Say we are using up the line from linebuf */
	haveline = FALSE;
	continue;

      default:
	fatal ("unrecognized flag character one input line:\n%s",
	       linebuf); 

      case NAME_FLAG:
      case QUOTE_FLAG:
      case TYPE_FLAG:
      case FLAGS_FLAG:
      case PATH_FLAG:
      case ARRAY_END_FLAG:
	break;
      } /* switch (linebuf[PREFIX_LEN]) */
    } /* if (strncmp (linebuf, HCOMMENT, PREFIX_LEN) == 0) */
    else if (linebuf[0] == MRI_COMMAND_CHAR) {
      /* it's some other MRI command that we don't care about */
      /* (including, possibly, a visible comment). */
      /* Say we are using up the line from linebuf */
      haveline = FALSE;
      continue;
    }
    else if (linebuf[PREFIX_LEN] == QUOTE_FLAG)
	/* it's a line that was translated */
	;
    else
	fatal ("unrecognized line type in input file:\n%s", linebuf);

    return (TRUE);
  } /* while (getline_raw() != NULL) */
  
  return (FALSE);
} /* getline2() */

 /***************************************************************************/
 /* get_quoted -- get a quoted string from the text file, and return */
 /* either a text object or an array object with text objects below */
 /* it. */
 /* Quote marks are discarded. */
 /* Lines may be continued with \ */
 /***************************************************************************/

 /* We support switching "fonts".  In the structured file, the */
 /* "underscore" font is represented by preceding each character with */
 /* "backspace-underscore",  in the MRI file we put a \_ at the */
 /* beginning of the underscored text, and a \+ at the end. */

#define	FONT_UNDERSCORE	'_'
#define	FONT_NORMAL	'+'

POINTER *get_quoted (void)
{
  char         inchar;
  char         *outstr;
  char         *instr;
  char         *save_instr;
  static char  outbuf[LINEBUFSIZE*2];
  char         font_state = FONT_NORMAL;
  POINTER      *object;
  int          charlen;
  int          n, val;
  char         mnemonic[20];
  char        *mnem_ptr;
  char        *line_ptr;

  mnem_ptr = mnemonic;
  object = NULL;
  
  if (getline(&instr) != QUOTE_FLAG)
      fatal ("was expecting a quoted string:\n%s", instr);

  save_instr = instr;
  outstr = outbuf;
  while (((inchar = *instr) != '\0') && (inchar != '"'))
      {
      charlen = mblen(instr, MB_CUR_MAX);
      /* copy at least 1 char */
      if (charlen < 1) charlen = 1;

      /* If this is a multi-byte character, then just pass it on ... */
      if (charlen == 1 )
	  instr++;
      else
	 {
         if (font_state == FONT_UNDERSCORE)
             {
             *outstr++ = '_';
             *outstr++ = '\b';
             }
	 while ( charlen-- )
	     *outstr++ = *instr++;
	 continue;
	 }

     if (inchar == '\\')
	 {
	 inchar = *instr++;
	 switch (inchar)
	     {

	     case FONT_NORMAL:
		 switch (font_state)
		     {
		     case FONT_UNDERSCORE:

			 font_state = FONT_NORMAL;
			 continue;

		     case FONT_NORMAL:
		     fatal ("Switched into normal font when already in normal font:\n%s",
save_instr);
		     default:
			 fatal ("Unknown font state '%c'\n", font_state);
		     }

	     case FONT_UNDERSCORE:
		 switch (font_state)
		     {
		     case FONT_NORMAL:
			 font_state = FONT_UNDERSCORE;
			 continue;

		     case FONT_UNDERSCORE:
			  fatal ("Switched into underscore font when alread in underscore font:\n%s", save_instr);

		     default:
			  fatal ("Unknown font state '%c'\n",font_state);
		     }

	     case 'b':
		 *outstr++ = '\b'; continue;

	     case 'f':
		 *outstr++ = '\f'; continue;

	     case 'r':
		 *outstr++ = '\r'; continue;

	     case 't':
		 *outstr++ = '\t'; continue;

	/* When \n is found, append current line to object and start a */
	/* new output line. */

	     case 'n':
		 *outstr = '\0';
		 if (object == NULL)
		     object = (POINTER *) s_alloc (0, T_POINTER, NULL);
		 object = s_append (object, s_string (outbuf));
		 outstr = outbuf;
		 continue;

	/* When line ends with '\' just discard the '\', read in */
	/* another line, and keep going. */

	     case '\0':
		 if ((save_instr = instr = getline_unfiltered()) == NULL)
		     fatal ("unterminated quoted string at end of file");
		 continue;
	     }

      if (isodigit(inchar))
	  {
	  for (n = 0, val = 0; (n < 3) && isodigit (inchar);
	      n++, inchar = *instr++)

	  val = (val * 8) + (inchar - '0');
	  *outstr++ = val;
	  instr--;        /* make instr point at the character just */
			  /* after the last digit again, as it will be */
			  /* stepped when we continue the outer loop. */
	  continue;
	  }

      /* if we haven't handled the character following the */
      /* backslash by this point, just fall through and take it */
      /* for what it is.  Note that this covers the cases of \" */
      /* and \\ */

	 }
	 if ((inchar == '%') && (hfp != NULL))
	 {
	     inchar = *instr;
	     if (inchar != '+')
		continue;

	     instr++;
	     while ((inchar = *instr) != '\\')
		   *mnem_ptr++ = *instr++;

	     *mnem_ptr = '\0';
	     mnem_ptr= find_define(mnemonic,hfp);
	     sprintf(outstr,"%s%s","%+",mnem_ptr);
	     outstr+=(strlen(mnem_ptr)+2);;
	     continue;
	 }

    if (font_state == FONT_UNDERSCORE) {
      *outstr++ = '_';
      *outstr++ = '\b';
    }

    /* copy normal characters */
    *outstr++ = inchar;
  } /* while (((inchar = *instr++) != '\0') && (inchar != '"')) */

  if (font_state != FONT_NORMAL)
      fatal ("Input string ends in non-normal font state:\n%s",
	     save_instr); 

  if (inchar != '"')
      fatal ("unterminated quoted string:\n%s", outbuf);
  *outstr = '\0';

  return ((object == NULL) ? (POINTER *) s_string (outbuf) :
	  s_append (object, s_string (outbuf)));
}

 /***************************************************************************/
 /* Doesn't do any processing on input line.  Returns pointer to */
 /* buffer, or NULL if end of file. */
 /***************************************************************************/
char *getline_unfiltered(void)
{
  char	*result;

  result = getline_raw();
  /* say we are using up the line from linebuf */
  haveline = FALSE;
  return (result);
}

char *getline_raw(void)
{
  if ( !haveline) {
    if (fgets (linebuf, LINEBUFSIZE, infp) != linebuf)
	return (NULL);

    linecount++;

    /* trim off trailing \n */
    linebuf[strlen (linebuf) -1] = '\0';
    haveline = TRUE;
  }
  return (linebuf);
}


 /***************************************************************************/
 /* basename: returns a pointer into the argument string indicating the     */
 /*   name of the file in the current directory.  That is, it strips off    */
 /*   any directory path prefix.                                            */
 /***************************************************************************/

static char *
basename(char *file_name)
{
    char *bp;              /* Points into the argument string */

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

  (void) fprintf (stderr, "%s:  fatal error at line %d:  ", progname, linecount);
  va_start(ap, str);
  arg1 = va_arg(ap, char *);
  arg2 = va_arg(ap, char *);
  (void) fprintf (stderr, str, arg1, arg2);
  va_end(ap);

 (void) fprintf (stderr, "\n");
 exit (1);
}

/*************************************************************************
* find_define: Searches for the mnemonic in the header file,  finds the  *
*              #define for the mnemonic and puts it in the file          *
*************************************************************************/
char *find_define(char *ptr, FILE *fp)
{
static char string[81];
char *srch_ptr;
char *mnem = NULL;

#ifdef DEBUG
	debug("mri2sf: Searching for '%s'",ptr);
#endif


    while ((srch_ptr =fgets(string,80,fp)) != NULL)
    {
      if ((srch_ptr=strstr(srch_ptr,ptr)) != NULL)
      {
        mnem = get_number(string);
	break;
      }
    }
    if (mnem == NULL)
    {
	(void) fprintf( stderr, "mri2sf: Error in Message catalog header\n" );
	exit( -1 );
    }

#ifdef DEBUG
	debug("mri2sf: mnemonic found in the header file '%s'",mnem);
#endif

    return(mnem);
}

/**************************************************************************
* get_number:searches along the string for any digit.  The digits found   *
*            are what we are looking for.  Otherwise the message catalog  *
*            file has a problem.                                          *
**************************************************************************/
char *get_number(char *srch_ptr)
{
char *mnem_ptr;


#ifdef DEBUG
	debug("mri2sf: mnemonic found in this line '%s'",srch_ptr);
#endif

   while (!isspace(*srch_ptr))  /* Move to the #define */  
	srch_ptr++;

   while (isspace(*srch_ptr))
	srch_ptr++;

   while (!isspace(*srch_ptr)) /* Move to the start of the mnemonic */
	srch_ptr++;
     
   while (!isdigit(*srch_ptr) && srch_ptr !=NULL) 
	srch_ptr++;

   if (srch_ptr == NULL)
   {
        (void) fprintf( stderr, "mri2sf: Error in Message catalog header\n" );
        exit( -1 );
   }

   mnem_ptr = srch_ptr;

   while (isdigit(*srch_ptr))
      srch_ptr++;

   *srch_ptr = '\0';

   return(mnem_ptr);
}
