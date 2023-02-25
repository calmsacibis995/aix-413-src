static char sccsid[] = "@(#)12  1.25.1.17  src/bos/usr/sbin/install/ckprereq/ckprereq.c, cmdinstl, bos411, 9428A410j 6/9/94 18:10:16";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: main 
 *            read_lpp_names
 *            read_prereq_file
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
** Includes
*/

#define MAIN_PROGRAM
#include <check_prereq.h>
#include <locale.h>      /* For call setlocale */
#include <unistd.h>      /* For getopt */

#include <instl_options.h>

#define PREREQ_FILE_SIZE 1024

extern int optind;
extern char *optarg;
extern int getopt (int, char **, char *);

static boolean read_lpp_names (Option_t  * sop,
                               int         argc,
                               char     ** argv,
                               boolean   * error);

static boolean read_prereq_file (Option_t  * sop,
                                 int         argc,
                                 char     ** argv,
                                 boolean     no_args,
                                 boolean   * error);

/*
 * Needed for ckprereq with no command line arguments.
 */
#define DFLT_PREREQ_FILE   "prereq"

/*--------------------------------------------------------------------------*
** NAME: ckprereq
**
** FUNCTION: ckprereq is the main ckprereq program.
**           there are 4 flags:
**                  -v: verbose
**             -f file: user file instead of prereq
**          -l lppname: get the prereq info out of the vpd
**                      for lppname
**           -O{r|u|s}: Indicates which VPD databases to look at (ROOT USER
**                      SHARE).
**
** RETURN VALUE DESCRIPTION:
**                0 : if all prereqs passed
**   Positive number: the number of conditions that failed
**--------------------------------------------------------------------------*/

int main (int argc, char ** argv)
{
   boolean    bad_INUTREE;
   boolean    bad_O_flag;
   boolean  * error;
   boolean    error_value = FALSE;
   boolean    f_flag;       /** Set if -f option used                    **/
   int        count;
   int        flag;
   boolean    l_flag;       /** Set if -l option used                    **/
   int        length;
   int        length_of_prereq;
   int        rc;           /** All Purpose return code                  **/
   Option_t * sop;
   Option_t * failsop;     
   char     * temp_string;
   boolean    Verbose;      /** Verbose flag                             **/


   inu_cmdname = "ckprereq";
   error = & error_value;
   check_prereq.parts_to_operate_on = LPP_USER;

   /*-----------------------------------------------------------------------*
    * set stdout and stderr to no buffering
    * (this is so command will work correctly in smit)
    *-----------------------------------------------------------------------*/

   (void)setbuf (stdout, NIL (char));
   (void)setbuf (stderr, NIL (char));

   setlocale (LC_ALL, "");

   /*-----------------------------------------------------------------------*
    * Open the message catalog
    *-----------------------------------------------------------------------*/

   CATOPEN ();

   /*
    * Set the destinations for printing the various types of messages.  
    * See library routine inu_msg for more concerning 
    * destination of messages.  Essentially, ckp_errs == stderr,
    * ckp_warn == ckp_succ == stdout.
    */

   ckp_errs = SCREEN_LOG;      
   ckp_succ = LOG_MSG;   
   ckp_warn = LOG_MSG; 

   /*
    * set initial values
    */

   f_flag = FALSE;
   l_flag = FALSE;
   Verbose = FALSE;

   if ( (temp_string = getenv ("INUTREE") ) != NIL (char))
    {
      bad_INUTREE = FALSE;
      if (strlen (temp_string) != 1)
       {
         bad_INUTREE = TRUE;
       }
      else
       {
         switch (temp_string[0])
          {
             case VPDTREE_USR :
                check_prereq.parts_to_operate_on = LPP_USER;
                break;

             case VPDTREE_ROOT :
                check_prereq.parts_to_operate_on = LPP_ROOT;
                break;

             case VPDTREE_SHARE :
                check_prereq.parts_to_operate_on = LPP_SHARE;
                break;

             default :
                bad_INUTREE = TRUE;
                break;

          } /* end switch */
       } /* end if */
      if (bad_INUTREE)
       {
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_BAD_INUTREE_E, 
                  CKP_BAD_INUTREE_D), inu_cmdname, VPDTREE_USR, VPDTREE_ROOT, 
                  VPDTREE_SHARE);
         exit (FATAL_ERROR_RC);
       }
    } /* end if */

   /* Create an instance of the Option_t structure to contain prq info. */

   sop = create_option ("Prereq array", FALSE, QUIESCE_NO, ' ',
                                  "NWT", NIL (Level_t), "", NIL (BffRef_t));
   sop -> operation = OP_APPLY;

   /*
    * Loop thru the options passed in and set flags
    *
    *       Default       : read from "prereq".  Results go to "prereq".
    *       -l <lpp_name> : There is no prereq file, just get info on <lpp_name>
    *                       from VPD database.  Results go in "lpp_name.prereq"
    *       -f <path>     : read from "<path>".  Results go to "prereq".
    *       -v            : verbose
    *       -O {r|u|s}    : Sets which VPD databases to look at.  If not
    *                       specified, use value of environment variable
    *                       INUTREE, otherwise, default to USER part.
    */
   count = 0;
   while ( (flag = getopt (argc, argv, "f:l:O:v")) != EOF)
    {
       switch (flag)
        {
           case 'f':     /* -f <path> */

             if (l_flag)
              {
                inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_USAGE_E, 
                         CKP_USAGE_D), inu_cmdname);
                exit (FATAL_ERROR_RC);
              }
             f_flag = TRUE;
             /*
              * NOTE:  This code is a partial implementation of an abandoned
              * requirement to handle multiple prereq files passed to the
              * ckprereq command.  It is being left in for potential future
              * use and because it also works for the case when 1 prereq
              * file is given.
              */
             while (read_prereq_file (sop, argc, argv, FALSE, error) )
              {
                RETURN_VALUE_ON_ERROR (FATAL_ERROR_RC);
              }
              RETURN_VALUE_ON_ERROR (FATAL_ERROR_RC);
             break;

           case 'l':   /*  -l <lpp_name> */

             if (f_flag)
              {
                inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_USAGE_E, 
                         CKP_USAGE_D), inu_cmdname);
                exit (FATAL_ERROR_RC);
              }

             l_flag = TRUE;
             /*
              * See NOTE above for multiple prereq files.  Same applies to 
              * multiple lpp names.
              */
             while (read_lpp_names (sop, argc, argv, error) )
              {
                RETURN_VALUE_ON_ERROR (FATAL_ERROR_RC);
              }
             RETURN_VALUE_ON_ERROR (FATAL_ERROR_RC);

             break;

           case 'v':
             Verbose = TRUE;
             break;

           case 'O' :
             temp_string = optarg;

             bad_O_flag = FALSE;
             if (strlen (temp_string) != 1)
              {
                bad_O_flag = TRUE;
              }
             else
              {
                switch (temp_string[0])
                 {
                   case 'u' :
                     check_prereq.parts_to_operate_on = LPP_USER;
                     break;

                   case 'r' :
                     check_prereq.parts_to_operate_on = LPP_ROOT;
                     break;

                   case 's' :
                     check_prereq.parts_to_operate_on = LPP_SHARE;
                     break;

                   default  :
                     bad_O_flag = TRUE;
                     break;

                 } /* end switch */

                if (bad_O_flag)
                 {
                   inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_BAD_OFLAG_E,
                            CKP_BAD_OFLAG_D), inu_cmdname, 'u', 'r', 's');
                   exit (FATAL_ERROR_RC);
                 } /* end if */
              } /* end if */
             break;

           default:
               inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_USAGE_E, 
                        CKP_USAGE_D), inu_cmdname);
               exit (FATAL_ERROR_RC);
               break;
       } /* end switch (flag) */
       count++;
    } /* end while getopt */

   /* Check for any other parameters, error if there are any. */

   if (optind < argc)
    {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_USAGE_E, CKP_USAGE_D), 
               inu_cmdname);
      exit (FATAL_ERROR_RC);
    }

   if (count == 0)
   {
      f_flag = TRUE;
      read_prereq_file (sop, argc, argv, TRUE, error);
      RETURN_VALUE_ON_ERROR (FATAL_ERROR_RC);
   }
   /*
    * inu_ckreq now expects there to be a failsop.  Let's create a dummy.
    */
   failsop = create_option ( "Dummy Fail Sop",
                             NO,       /* default op_checked field to NO */
                             NO,       /* default quiesce field to NO */
                             CONTENT_UNKNOWN,  /* default content field to
                                                * unknown */
                             NullString,       /* lang field. */
                             NIL (Level_t),    /* default level field to
                                                * NIL (Level_t) */
                             NullString,       /* description. */
                             NIL (BffRef_t));  /* default bff field to
                                                * NIL (BffRef_t) */

   rc = inu_ckreq (sop, failsop, NIL (TOC_t), Verbose, FALSE, FALSE,
                   check_prereq.parts_to_operate_on, TRUE);
   exit (rc);

} /* end main */

/*--------------------------------------------------------------------------*
** NAME: read_lpp_names
**
** FUNCTION: Reads a list of lpp_name and level and enters them into the sop.
**
** RETURNS : Boolean value indicating if a prereq file was successfully read.
**
**--------------------------------------------------------------------------*/

static boolean read_lpp_names (Option_t  * sop,
                               int         argc,
                               char     ** argv,
                               boolean   * error)
 {
   Level_t    level;
   char     * lpp_level;
   char     * lpp_name;      /** the name of the LPP being prereqed       **/
   Option_t * option;
   prod_t     ProdVpd;      /* Product DB structure       **/
   char       product_name[MAX_LPP_FULLNAME_LEN];
   int        rc;           /** All Purpose return code                  **/
   char     * supersede_info;
   int        vpd_fields;

   if (optarg != NIL (char) )
    {
      lpp_name = optarg;
      optarg = NIL (char);
    }
   else
    {
      return (FALSE);
    }
   /* Check to see if we have a level. */

   lpp_level = NIL (char);
   if ( (optind < argc) && (argv[optind][0] != '-') )
    {
      lpp_level = argv[optind];
      if (inu_level_convert (lpp_level, & level) == SUCCESS)
       {
         optind++;
         ProdVpd.ver = level.ver;
         ProdVpd.rel = level.rel;
         ProdVpd.mod = level.mod;
         ProdVpd.fix = level.fix;
         strcpy (ProdVpd.ptf, level.ptf);
         vpd_fields = PROD_LPP_NAME | PROD_VER | PROD_REL | PROD_MOD |
                      PROD_FIX | PROD_PTF;
       }
      else
       {
         inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_LEVEL_NUM_E, 
                  CMN_LEVEL_NUM_D), inu_cmdname, lpp_level);
         exit (FATAL_ERROR_RC);
       }
    }

   if (lpp_level == NIL (char) )
    {
      vpd_fields = PROD_LPP_NAME;
    }
   strcpy (ProdVpd.lpp_name, lpp_name);
   switch (check_prereq.parts_to_operate_on)
    {
      case LPP_ROOT :
      case LPP_USER :
         vpdremotepath (USR_VPD_PATH);
         vpdremote ();
         break;

      case LPP_SHARE :
         vpdremotepath (SHARE_VPD_PATH);
         vpdremote ();
         break;

    } /* end switch */

   rc = vpdget (PRODUCT_TABLE, vpd_fields, & ProdVpd);
   switch (rc)
    {

      case VPD_NOMATCH: /** Lpp Not Found **/

        strcpy (product_name, lpp_name);
        if (lpp_level != NIL (char))
         {
           strcat (product_name, " ");
           format_lpp_level (& level,
                             & product_name[strlen (product_name)]);
         }

        /*  Software product %s\n\
            is not in the Software Vital Product Data database.
            Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_NOSWVPD_E, CKP_NOSWVPD_D),
                 inu_cmdname, product_name);
        exit (FATAL_ERROR_RC);
        break;

      case VPD_SYS: /** system error **/

        /* A system error occured while attempting
             to access the Software Vital Product Data database.
             Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                 inu_cmdname);
        exit (FATAL_ERROR_RC);
        break;

      case VPD_BADCNV: /** fatal internal error **/
      case VPD_SQLMAX:

        /* An internal error occured while attemting
              to access the Software Vital Product Data database.
              Use local problem reporting procedures. */

        inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_IVPDERR_E, CMN_IVPDERR_D),
                 inu_cmdname);
        exit (FATAL_ERROR_RC);
        break;

       default:
           break;
    } /** end switch rc **/

   option = create_option ("", FALSE, QUIESCE_NO, ' ', "NWT", NIL (Level_t),
                           "", NIL (BffRef_t));
   strcpy (option -> name, lpp_name);
   RETURN_VALUE_ON_ERROR (FALSE);

   option -> operation = OP_APPLY;
   option -> next = sop -> next;
   sop -> next = option;

   /* Malloc a buffer for the prereq file array and copy it in to it. */

   option -> prereq = dupe_string (ProdVpd.prereq, error);
   RETURN_VALUE_ON_ERROR (FALSE);

   option->level.ver = ProdVpd.ver;
   option->level.rel = ProdVpd.rel;
   option->level.mod = ProdVpd.mod;
   option->level.fix = ProdVpd.fix;
   strcpy (option -> level.ptf, ProdVpd.ptf);
   strcpy (option -> prereq, ProdVpd.prereq);

   return (FALSE);

 } /* read_lpp_names */

/*--------------------------------------------------------------------------*
** NAME: read_prereq_file
**
** FUNCTION: Reads a list of prereq files and enters them into the sop.
**
** RETURNS : Boolean value indicating if a prereq file was successfully read.
**
**--------------------------------------------------------------------------*/

static boolean read_prereq_file (Option_t  * sop,
                                 int         argc,
                                 char     ** argv,
                                 boolean     no_args,
                                 boolean   * error)
 {

  int        buf_sz;       /* Size of buffer that hold the prereq file array */
  int        c;
  char     * file_name;
  int        i;
  Option_t * option;
  FILE     * prereq_file;

  if (no_args)
   {
     file_name = DFLT_PREREQ_FILE;
   }
  else
   {
     if ( optarg != NIL (char) )
      {
        file_name = optarg;
        optarg = NIL (char);
      }
     else
      {
        if ( (optind < argc) && (argv[optind][0] != '-') )
         {
           file_name = argv[optind];
           optind++;
         }
        else
         {
           return (FALSE);
         }
      }
   }

  check_prereq.output_file_name = file_name; /* used to write return codes to */

  if ( (prereq_file = fopen (file_name, "r")) == NIL (FILE))
  {
     /* Cannot access or open file %s.
          Check path name and permissions. */

     inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, CMN_CANT_OPEN_D),
              inu_cmdname, file_name);
     * error = TRUE;
     return (FALSE);
   }

   option = create_option ("", FALSE, QUIESCE_NO, ' ',
                                "NWT", NIL (Level_t), "",
                                NIL (BffRef_t));
   strcpy (option -> name, file_name);
   option -> operation = OP_APPLY;
   option -> next = sop -> next;
   sop -> next = option;

   buf_sz = PREREQ_FILE_SIZE;
   option -> prereq = my_malloc (buf_sz, error);
   RETURN_VALUE_ON_ERROR (FATAL_ERROR_RC);

  /*
   * Read in the entire prereq file into the malloc'ed buffer
   * If we run out of memory realloc some more memory...
   */

   i = 0;
   do
    {
      c = getc (prereq_file);
      option -> prereq[i++] = c;
      if (i >= buf_sz)
       {
         buf_sz += PREREQ_FILE_SIZE;
         option -> prereq = my_realloc (option -> prereq, buf_sz, error);
         RETURN_VALUE_ON_ERROR (FALSE);
       }
   }  while (c != EOF);
   option -> prereq[--i] = '\0';

   fclose (prereq_file);

   return (FALSE);
 } /* end read_prereq_files */
