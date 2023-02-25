static char sccsid[] = "@(#)35  1.17.1.17  src/bos/usr/sbin/install/ckprereq/my_lex.c, cmdinstl, bos411, 9428A410j 6/14/94 13:46:28";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            get_token 
 *            initialize_lexical_analyzer 
 *            next_char 
 *            print_output_file
 *            set_failure_code
 *            token_error 
 *            unget_char
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include <check_prereq.h>

/*
 * Static declarations.
 */

static char next_char (boolean * error);

static void token_error (lex_error_type error_type, boolean * error);

static void unget_char (void);

static int     token_index = 0;
static boolean line_starts_with_token;


/*--------------------------------------------------------------------------*
**
** NAME: get_token
**
** FUNCTION:  Retrieves the next token from the current prereq file.
**
**
** RETURNS:   A token value is returned.  In addition, the global
**            parsers.tokens_string_value will point to the string corresponding to
**            the token when the token is A_NAME.  The global
**            parsers.tokens_number_value will contain the int value of the token,
**            when it is A_NUMBER.
**
**
**--------------------------------------------------------------------------*/

token_type get_token (boolean * error)
{
  char       character;
  int        dotted_number_elements;
  token_type token;

  parsers.tokens_number_value = 0;
  memset (parsers.tokens_string_value, '\0', MAX_LEX_STRING_LENGTH);
  token_index = 0;
  line_starts_with_token = FALSE;  /* This is to support a really STUPID
                                      feature. */

  character = next_char (error);
  RETURN_VALUE_ON_ERROR (ILLEGAL_TOKEN);

  while (isspace (character))
   {
     token_index = 0;
     character = next_char (error);
     RETURN_VALUE_ON_ERROR (ILLEGAL_TOKEN);
   }

  if (character == NULL_CHAR)
   {
     token = END_OF_FILE;
     return (token);
   }
  else if (isalpha (character) || (strchr ("-_+", character) != 0 ) )
        {
          token = A_NAME;
          while (             isalpha (character)
                                      ||
                              isdigit (character)
                                      ||
                    (strchr ("-_.+", character) != 0 ) )
           {
             character = next_char (error);
             RETURN_VALUE_ON_ERROR (ILLEGAL_TOKEN);

             if (character == NULL_CHAR)
                break;
           }
          unget_char ();
          parsers.tokens_string_value[token_index] = '\0';
          if (token_index == 1)  /* our string is only one character */
           {
             if (line_starts_with_token) /* STUPID feature! */
              {
                switch (tolower (parsers.tokens_string_value[0]))
                 {
                   case CKPREREQ_ERROR_FLAG :
                   case FIX_ERROR_FLAG :
                   case IFREQ_ERROR_FLAG :
                   case MODIFICATION_ERROR_FLAG :
                   case PREREQ_ERROR_FLAG :
                   case PTF_ERROR_FLAG :
                   case RELEASE_ERROR_FLAG :
                   case SYNTAX_ERROR_FLAG :
                   case VERSION_ERROR_FLAG :
                      if (check_prereq.mark_prereq_file)
                         parsers.error_info[parsers.line_number].start_of_line =
                                                    (parsers.virtual_file + 1);
                      return (get_token (error));  /* Ignore this token. */

                   default  :
                      token_error (ILLEGAL_NAME, error);
                      return (ILLEGAL_TOKEN);
                 } /* end case */
              }
             else
              {
               switch (tolower (parsers.tokens_string_value[0]))
                {
                  case 'f' :  return (FIX);
                  case 'm' :  return (MODIFICATION);
                  case 'o' :  return (OR);
                  case 'p' :  return (PTF);
                  case 'r' :  return (RELEASE);
                  case 'v' :  return (VERSION);
                  default  :
                     token_error (ILLEGAL_NAME, error);
                     return (ILLEGAL_TOKEN);
                } /* end switch */
             } /* end if */
           } /* end if */
          if (token_index >= MAX_LPP_FULLNAME_LEN)
           {
             token_index = MAX_LPP_FULLNAME_LEN - 1;
             token_error (NAME_TOO_LONG, error);
             return (ILLEGAL_TOKEN);
           }
          return (token);
        }

  else if (isdigit (character))
        {
          token = A_NUMBER;
          dotted_number_elements = 0;
          while (isdigit (character) || (character == '.'))
          {
             if (character == '.')
              {
                token = A_DOTTED_NUMBER;
                dotted_number_elements++;
                if (dotted_number_elements > 3)
                {
                   token_error (ILLEGAL_DOTTED_NUMBER, error);
                   return (ILLEGAL_TOKEN);
                }
              }
             character = next_char (error);
             RETURN_VALUE_ON_ERROR (ILLEGAL_TOKEN);

          }
          parsers.dotted_number_count = dotted_number_elements+1;
          if (! (            isspace (character)
                                    ||
                           (character == NULL_CHAR)
                                    ||
                      (strchr ("{})", character) != 0))      )
           {
             token_error (ILLEGAL_NUMBER, error);
             return (ILLEGAL_TOKEN);
           }
          unget_char ();
          parsers.tokens_string_value[token_index] = '\0';
          sscanf (parsers.tokens_string_value, "%d",
                  & parsers.tokens_number_value);
          return (token);
        }
  else if (character == '*')
        {
          while (!isspace (character))
           {
             character = next_char (error);
             RETURN_VALUE_ON_ERROR (ILLEGAL_TOKEN);

           }
           unget_char ();
           parsers.tokens_string_value[token_index] = '\0';
           if (strcmp (parsers.tokens_string_value, "*ifreq") == 0)
            {
              token = IFREQ;
              return (token);
            }
           if (strcmp (parsers.tokens_string_value, "*prereq") == 0)
            {
              token = PREREQ;
              return (token);
            }
           if (strcmp (parsers.tokens_string_value, "*instreq") == 0)
            {
              token = INSTREQ;
              return (token);
            }
           if (strcmp (parsers.tokens_string_value, "*coreq") == 0)
            {
              token = COREQ;
              return (token);
            }
           token_error (REQUISITE_EXPECTED, error);
           return (ILLEGAL_TOKEN);
         }
  else if (character == '=')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = EQUALS;
          return (token);
        }
  else if (character == '>')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = GREATER_THAN;
          return (token);
        }
  else if (character == '<')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = LESS_THAN;
          return (token);
        }
  else if (character == '{')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = OPEN_BRACE;
          return (token);
        }
  else if (character == '}')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = CLOSE_BRACE;
          return (token);
        }
  else if (character == '(')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = OPEN_PAREN;
          return (token);
        }
  else if (character == ')')
        {
          parsers.tokens_string_value[token_index] = '\0';
          token = CLOSE_PAREN;
          return (token);
        }
  parsers.tokens_string_value[token_index] = '\0';
  token_error (UNEXPECTED_CHARACTER, error);
  return (get_token (error));

} /* end get_token */

/*--------------------------------------------------------------------------*
**
** NAME: next_char
**
** FUNCTION:  Returns the next character from the current prereq file.
**            Once the end of the prereq file is reached END_OF_FILE is
**            returned and the file pointer remains at the end.  Also,
**            strings that are too long and non-ascii characters are
**            trapped and reported.
**
**
** RETURNS:   The character value or END_OF_FILE of the current chacter.
**            Characters may be put back via unget_char ().
**
**
**--------------------------------------------------------------------------*/

static char next_char (boolean * error)
 {
   int               character;
   boolean           dummy_error = 0;
   error_info_type * previous_end_of_buffer;


   if (* parsers.virtual_file == NULL_CHAR)
    {
      return (NULL_CHAR);
    }

   /* Make sure that our error_info structure is big enough. */

   if (check_prereq.mark_prereq_file &&
       parsers.error_info_buffer_size <= (parsers.line_number + 1) )
    {
      parsers.error_info_buffer_size += INITIAL_NUMBER_OF_LINES_IN_PREREQ_FILE;
      if (parsers.error_info_buffer_size ==
                                        INITIAL_NUMBER_OF_LINES_IN_PREREQ_FILE)
       {
         parsers.error_info = my_malloc ( (parsers.error_info_buffer_size *
                                                    sizeof (error_info_type) ),
                                           error);
         RETURN_ON_ERROR;
       }
      else
       {
         parsers.error_info = my_realloc (parsers.error_info,
                                          (parsers.error_info_buffer_size *
                                                    sizeof (error_info_type) ),
                                          error);
         RETURN_ON_ERROR;
       }
      previous_end_of_buffer = parsers.error_info +
                               parsers.error_info_buffer_size -
                               INITIAL_NUMBER_OF_LINES_IN_PREREQ_FILE;
      memset (previous_end_of_buffer, '\0',
              (INITIAL_NUMBER_OF_LINES_IN_PREREQ_FILE *
                                                  sizeof (error_info_type) ) );
    } /* end if */

   if ( (character = * parsers.virtual_file) == '\n')
    {
       parsers.column = 0;
       parsers.line_number++;
       parsers.virtual_file++;
       if (check_prereq.mark_prereq_file)
          parsers.error_info[parsers.line_number].start_of_line =
                                                          parsers.virtual_file;
       if (token_index < MAX_LEX_STRING_LENGTH)
          parsers.tokens_string_value[token_index] = '\0';

       /* Go ahead and bump up the token_index since unget_char decrements
          it */

       token_index++;
       return (character);
    } /* end if */

   if ( ! isascii (character))
    {
       token_error (NOT_ASCII, & dummy_error);
       character = ' ';             /* Pretend we saw white space. */
    }

   if (parsers.column == 0)
    {
      line_starts_with_token = TRUE;
      if (check_prereq.mark_prereq_file)
         parsers.error_info[parsers.line_number].start_of_line =
                                                          parsers.virtual_file;
    }

    if (token_index < MAX_LEX_STRING_LENGTH)
       parsers.tokens_string_value[token_index++] = character;
    parsers.column++;
    parsers.virtual_file++;
    return (character);
 }  /* end next_char */


/*--------------------------------------------------------------------------*
**
** NAME: unget_char
**
** FUNCTION:  This function is called when (one) too many characters have
**            been read from the prereq file via next_char ().  The un-read
**            character will be returned by a subsequent call to
**            next_char ().
**
** RETURNS:   nothing.
**
**
**--------------------------------------------------------------------------*/


static void unget_char (void)
 {
   if (* parsers.virtual_file != NULL_CHAR)
    {
            /* Don't back up over a new line */

      if (* (parsers.virtual_file - 1) != '\n')
       {
         parsers.virtual_file--;   /* back up one character */
         parsers.column--;
       }
      token_index--;
    }
 } /* end unget_char */


/*--------------------------------------------------------------------------*
**
** NAME: token_error
**
** FUNCTION:  This function is called when an error has been detected when
**            tokenizing the prereq_file.  This function reports errors and
**            performs any fix-up action needed to continue processing of
**            the prereq file.
**
** RETURNS:   nothing.
**
**
**--------------------------------------------------------------------------*/


static void token_error (lex_error_type   error_type,
                         boolean        * error)
{
  fix_info_type * ptr;
  char            fix_name[MAX_LPP_FULLNAME_LEN];


  check_prereq.number_of_errors++;

  ptr = parsers.prereq_node;
  get_fix_name_from_fix_info (ptr, fix_name);

  set_failure_code (SYNTAX_ERROR_FLAG, TRUE);

  switch (error_type)
   {
     case ILLEGAL_NAME:

        /*  A lexical error occured for %s
               on line %d, column %d.
               There is an illegal name, PTF ID, or requisite,
               or field name: %s.
               Names must be more than one character in length
               and begin with a letter or '_'.
               Subsequent characters in a name may be a letter, a digit,
               a '_', or a '.'.
               A PTF ID has the same syntax as a name.
               A requisite is one of: *prereq, *coreq, or *ifreq.
               A field name is one of: 'V', 'R', 'M', 'F', 'P', or 'O'.
               Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_ILLNAME_E, 
                 CKP_LEX_ILLNAME_D),
                 inu_cmdname, fix_name, parsers.line_number,
                 parsers.column, parsers.tokens_string_value);
        * error = TRUE;
        check_prereq.function_return_code = LEXICAL_ERROR_RC;
        break;

     case ILLEGAL_NUMBER:

        /*  A lexical error occured for %s
               on line %d, column %d.
               There is an illegal number: %s
               Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_ILLNUM_E, 
                 CKP_LEX_ILLNUM_D),
                 inu_cmdname, fix_name, parsers.line_number,
                 parsers.column, parsers.tokens_string_value);
        * error = TRUE;
        check_prereq.function_return_code = LEXICAL_ERROR_RC;
        break;

     case ILLEGAL_DOTTED_NUMBER:

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_ILLBASE_E, 
                 CKP_LEX_ILLBASE_D),
                 inu_cmdname, fix_name, parsers.line_number,
                 parsers.column, parsers.tokens_string_value);
        * error = TRUE;
        check_prereq.function_return_code = LEXICAL_ERROR_RC;
        break;

     case REQUISITE_EXPECTED:

        /*  A lexical error occured for %s
               on line %d, column %d.
               One of the following was expected instead of %s:
                   *prereq, *coreq, or *ifreq.
               Use local problem reporting procedures. */

           inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_REQEXP_E, 
                    CKP_LEX_REQEXP_D), inu_cmdname, fix_name, 
                    parsers.line_number, parsers.column, 
                    parsers.tokens_string_value);

        * error = TRUE;
        check_prereq.function_return_code = LEXICAL_ERROR_RC;
        break;

     case UNEXPECTED_CHARACTER:

        /*  A lexical error occured for %s
               on line %d, column %d.
               An unexepected character, %c, was found (hexadecimal value: %x).
               The character will be treated as a SPACE.
               Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_UNXCHR_E, 
                 CKP_LEX_UNXCHR_D),
                 inu_cmdname, fix_name, parsers.line_number,
                 parsers.column,
                 * (parsers.virtual_file - 1), * (parsers.virtual_file - 1));

        * error = FALSE;                       /* Not a critical error. */
        check_prereq.function_return_code = LEXICAL_ERROR_RC;
        break;

     case NOT_ASCII:

        /*  A lexical error occured for %s
               on line %d, column %d.
               An illegal character, %c, was found (hexadecimal value: %x).
               All characters must be ASCII.
               The character will be treated as a SPACE.
               Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_ILLCHR_E, 
                 CKP_LEX_ILLCHR_D),
                 inu_cmdname, fix_name, parsers.line_number,
                 parsers.column,
                 * (parsers.virtual_file - 1), * (parsers.virtual_file - 1));

        * error = FALSE;                       /* Not a critical error. */
        check_prereq.function_return_code = LEXICAL_ERROR_RC;
        break;

     case NAME_TOO_LONG:

        /*  A lexical error occured for %s
               on line %d, column %d.
               The name %s is too long.
               Maximum name length is %d.  The name has been truncated.
               Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_LEX_LNGNAME_E, 
                 CKP_LEX_LNGNAME_D),
                 inu_cmdname, fix_name, parsers.line_number,
                 parsers.column,
                 parsers.tokens_string_value, MAX_LPP_FULLNAME_LEN - 1);

        * error = FALSE;                       /* Not a critical error. */
        break;

   } /* end switch */

} /* end token_error */

/*--------------------------------------------------------------------------*
**
** NAME: set_failure_code
**
** FUNCTION:  This routine is called when it has been determined that no
**            fixes can possibly pass for this requisite. This routine
**            marks the first column of the output file with a code indicating
**            when this occured.
**
**            This feature has very limited value but was a carry over from
**            the days of the dinosaurs.  It is really only used when
**            called from the ckprereq command line AND when we are working on
**            the top-level prereq file.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void set_failure_code (char      failure_code,
                       boolean   major_error)
 {

   if (! check_prereq.mark_prereq_file)
      return;

   if (major_error)
    {
      parsers.error_info[parsers.start_of_current_expression].error_code =
                                                                  failure_code;
      return;
    }

   /* If the code has already been set, then just get out of here. */

   if (parsers.error_info[parsers.start_of_current_expression].error_code ==
                                                                          '\0')
    {
      parsers.error_info[parsers.start_of_current_expression].error_code =
                                                                  failure_code;
      return;
    }

 } /* end set_failure_code */

/*--------------------------------------------------------------------------*
**
** NAME: initialize_lexical_analyzer
**
** FUNCTION:  Initializes the lexical analyzer.
**
** RETURNS:   nothing.
**
**--------------------------------------------------------------------------*/

void initialize_lexical_analyzer (fix_info_type * prereq_node)
 {
    if (check_prereq.mark_prereq_file)
    {
       /*
        * Only initialize the error_info structure when processing 
        * prereq file of ckprereq command line arg ie. once.  we'll 
        * lose it our error info on subsequent calls otherwise.
        */
       parsers.error_info = NIL (error_info_type);
       parsers.error_info_buffer_size = 0;
    }
    parsers.column = 0;
    parsers.line_number = 1;
    parsers.prereq_node = prereq_node;
    parsers.start_of_current_expression = 0;
    parsers.prereq_file = prereq_node -> prereq_file;
    parsers.virtual_file = prereq_node -> prereq_file;

 } /* end initialize_lexical_analyzer */

/*--------------------------------------------------------------------------*
**
** NAME: print_output_file
**
** FUNCTION:  Prints the prereq file with error codes.  Only called by
**            ckprereq.c, the command line interface.
**
** RETURNS:   nothing.
**
**--------------------------------------------------------------------------*/

void print_output_file (void)
 {
   int        line_number;
   int        max_lines_parsed;
   char     * virtual_file;


   check_prereq.output_file = fopen (check_prereq.output_file_name, "w");
   if (check_prereq.output_file == NIL (FILE))
    {
      /* If we can not open the file for output, we must pretend nothing
         is wrong.  This situation will occur if we are installing a root
         part and we do not have write permission to the SPOT (usr part)
         because it is on a server. */

      return;

      /* Cannot access or open file %s.
           Check path name and permissions. */
      /*
       inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                CMN_CANT_OPEN_D), inu_cmdname, PrqFile);
       exit (FATAL_ERROR_RC); */
    }

   /* Now we have to overwrite the original file with itself, but adding
      an error code in column 1 as appropriate.  The error codes are stored
      in an array indexed by line number.  The error codes were established
      during parsing.  */

   line_number = 1;
   max_lines_parsed = parsers.line_number;
   virtual_file = parsers.error_info[line_number].start_of_line;

   if ( (parsers.error_info_buffer_size >= line_number)
                            &&
        (parsers.error_info[line_number].error_code != '\0') )
    {
      putc (parsers.error_info[line_number].error_code, 
            check_prereq.output_file);
      putc (' ', check_prereq.output_file);
    }

   while ( * virtual_file != '\0')
    {
      putc (* virtual_file, check_prereq.output_file);
      if (* virtual_file == '\n')
       {
         line_number++;
         if (line_number <= max_lines_parsed)
          {
            virtual_file = parsers.error_info[line_number].start_of_line;

            if ( (parsers.error_info_buffer_size >= line_number)
                                      &&
                   (parsers.error_info[line_number].error_code != '\0') )
             {
               putc (parsers.error_info[line_number].error_code,
                     check_prereq.output_file);
               putc (' ', check_prereq.output_file);
             }
          }
         else
          {
            virtual_file++;
          }
        }
       else
        {
         virtual_file++;
        }
    }  /* end while */

   fclose (check_prereq.output_file);

 } /* end print_output_file */

