static char sccsid[] = "@(#)84  1.7.1.30  src/bos/usr/sbin/install/bffcreate/bffcreate.c, cmdinstl, bos41J, 9518A_all 4/28/95 16:44:13";
/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 *   FUNCTIONS: ensure_dir_exists
 *              main
 *              mark_lpps
 *              mark_toc_as_notfnd
 *              set_tape_blk_size_back
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#define MAIN_PROGRAM

#include <locale.h>
#include <bffcreate.h>

#define BFFCSYNTAX  1

#define IF_4_1_UPDATE(t) (IF_4_1(t) && IF_UPDATE(t))


/* -----------------------------------------------------------------------
**
** Function:      main
**
** Purpose:       This is the main driver for the bffcreate
**                routine.  It basically parses the command line, sets the
**                appropriate flag variables, calls open_toc to create a
**                toc linked list, and marks the toc linked list entries
**                that are to be processed.  After this, the bff routine
**                is called to do the actual processing.  Then a system
**                command is issued to run the inutoc command to create
**                a .toc file in the appropriate directory.
**
** Parameters:    argc --  count of cmd line args
**                argv --  array of cmd line args
**
** Flags:                 
**          -d  <device>             device to copy bffs from
**          -l                       list bffs on device
**          -J                       called from smit 
**          -q                       Quiet mode (don't prompt for device)
**          -t  <target directory>   Target directory to copy bffs into 
**          -v                       Verbose mode (echo bff names when created)
**          -w  <work directory>     Work directory to create temp files
**          -X                       Expand filesystems
**                        
** Returns:       INUGOOD (0)  -- upons success
**                rc           -- from the appropriate function
**                                upon failure
**
** ----------------------------------------------------------------------- */


/** ----------------------------------------------------------------- *
 **                       G L O B A L S
 ** ----------------------------------------------------------------- */

int   userblksize=512;

extern char *optarg;       /* for parsing the command line        */
extern int  optind;        /* for parsing the command line        */

int q_flag=0;              /* Boolean  1 ==> q flag was specified */
int v_flag=0;              /*  0 ==> v flag was not specified     */
int X_flag=0;              /*  0 ==> X flag was not specified     */
int l_flag=0;              /*  0 ==> l flag was not specified     */
int J_flag=0;              /*  0 ==> l flag was not specified     */


void   set_tape_blk_size_back (TOC_t *, char *);
void   resolve_to_full_path (char *, char *, char *);

/** ----------------------------------------------------------------- *
 **                      M A I N      P R O G R A M
 ** ----------------------------------------------------------------- */

main (int argc, char *argv [])
{
    TOC_t *toc_ptr;        /* pointer to head of toc linked list    */
    struct stat st_buf;    /* for calls to stat                     */
    char device [PATH_MAX+1];  /* device to get bffs from           */
    char destdir [PATH_MAX+1]; /* destination directory of bff      */
    char workdir [PATH_MAX+1]; /* Used to extract lpp_name, and     */
                           /* service.num files into                */
    char cmd [PATH_MAX+1]; /* string to hold a system command in    */
    char cwd [PATH_MAX+1]; /* current working directory             */
    char tmpdir [PATH_MAX+1]; /* temp working dir                   */
    char tmpstr [PATH_MAX+1]; /* temp working string                */
    int flag;              /* current flag being parsed from getopt */
    int rc=INUGOOD;        /* return code from bff function call    */


    inu_cmdname = "bffcreate";             /* for inu_msg call */

   /** ------------------------------------------------------------------- *
    ** set stdout and stderr to no buffering
    ** (this is so command will work correctly in smit)
    ** ------------------------------------------------------------------- */

    setbuf (stdout, NIL (char));
    setbuf (stderr, NIL (char));

    (void) setlocale (LC_ALL, "");

   /*-----------------------------------------------------------------------*
    * Open the message catalog
    *-----------------------------------------------------------------------*/
    CATOPEN ();

    if ((getcwd (cwd, (int) sizeof (cwd) - 1)) == NIL (char))
    {
       perror (inu_cmdname);
       exit (INUSYSFL);
    }

    /* Initialize the hash table FileTable */

    init_filetable ();

   /*------------------------------------------------------- *
    *  Parse the command line
    *------------------------------------------------------- */

    while  ((flag = getopt (argc, argv, "?JlqvXd:t:w:")) != EOF)
    {
        switch (flag)
        {
            case 'd':  /* device */
                strcpy (tmpstr, optarg);
                resolve_to_full_path (optarg, device, cwd);
                break;

            case 'J':      /* called from SMIT */
                J_flag = 1;
                break;

            case 'l':      /* list media contents */
                l_flag = 1;
                break;

            case 't':   /* destination dir */
                strcpy (tmpstr, optarg);
                resolve_to_full_path (tmpstr, destdir, cwd);
                break;

            case 'w':   /* working dir */
                strcpy (tmpstr, optarg);
                resolve_to_full_path (tmpstr, workdir, cwd);
                break;

            case 'q':   /* quiet flag */
                q_flag = 1;
                break;

            case 'v':   /* verbose mode */
                v_flag = 1;
                break;

            case 'X':   /* expand flag */
                if ((putenv ("INUEXPAND=1")) != 0)
                {
                    inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, 
                                         CMN_SET_ENV_VAR_E, CMN_SET_ENV_VAR_D),
                                         inu_cmdname, "INUEXPAND=1");
                    exit (INUSYSFL);
                }
                break;

            case '?':   /* user desires a usage message */
                inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_USAGE_E, BFF_USAGE_D));
                exit (INUSAGE);

            default :   /* usage message */
               inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_USAGE_E, BFF_USAGE_D));
               exit (INUSAGE);

        } /* switch */
    } /* while */


  /** ---------------------------------------------------------------------- *
   **  Ensure that all critical variables have at least the default value.
   ** ---------------------------------------------------------------------- */

   if (device [0] == '\0')
       strcpy (device, DFLT_DEVICE);

   if (destdir [0] == '\0')
       strcpy (destdir, DFLT_DESTDIR);

   if (workdir [0] == '\0')
       strcpy (workdir, DFLT_WORKDIR);


  /** ------------------------------------------------------------- *
   **  A pkg list is required unless the -l flag is given.  
   **  A pkg list and the -l flag are mutually exclusive. 
   ** ------------------------------------------------------------- */

   if (l_flag)
   {
      if (argc > (int) optind)
      {
         inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_NO_LPPS_W_L_FLAG_E, 
                              BFF_NO_LPPS_W_L_FLAG_D));
         inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_USAGE_E, BFF_USAGE_D));
         exit (BFFCSYNTAX);
      }
   }
   else if (argc <= (int) optind)
   {
      inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_NOLPPS_E, BFF_NOLPPS_D));
      inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_USAGE_E, BFF_USAGE_D));
      exit (INUBADLP);
   }

   if ((rc = ensure_dir_exists (workdir)) != INUGOOD)
      exit (rc);

   if ((rc = ensure_dir_exists (destdir)) != INUGOOD)
      exit (rc);

  /** ----------------------------------------------------------- *
   **  Create and cd to a temp working dir so that the call to
   **  open_toc will restore any image files into it (and not
   **  overwrite any that happen to be in the current dir).
   ** ----------------------------------------------------------- */

   sprintf (tmpdir, "%s/XXXXXX", workdir);
   mktemp  (tmpdir);

   if ((rc = ensure_dir_exists (tmpdir)) != INUGOOD)
      exit (rc);

   chdir (tmpdir);


  /** ----------------------------------------------- * 
   **  Read in the .toc file from the device.
   ** ----------------------------------------------- */

   if (open_toc (&toc_ptr, device, q_flag, CALLER_BFFCREATE) != INUGOOD)
   {
      set_tape_blk_size_back (toc_ptr, device);
      exit (INUCRTOC);
   }

  /** -------------------------------------------------------------- *
   **  Set the q_flag to 1 to indicate that the user does not need
   **  to be prompted anymore (the call to open_toc already prompted
   **  the user if it was needed.
   ** -------------------------------------------------------------- */

   q_flag = 1;

  /** -------------------------------------------------------------- *
   ** cd back to working dir and remove the temp working dir.
   ** -------------------------------------------------------------- */

   chdir (workdir);
   inu_rm (tmpdir);

  /** -------------------------------------------------------------- *
   **  Now, mark the toc linked list entries that the user
   **  wants to process as MARKED.  This indicates that a
   **  given entry needs to be processed.
   ** -------------------------------------------------------------- */

   if (l_flag == 1)   
   {
      list_media_contents (toc_ptr);
   }
   else 
   {
      if ((rc=mark_lpps_driver (toc_ptr,argc,argv,device,optind)) != INUGOOD)
      {
         exit (rc);   /* msg already printed */
      }

     /** -------------------------------------------------------------- *
      **  Copy all specified lpps from the media (device) to the
      **  destdir under the name (s) to be created.  Use workdir to
      **  perform any intermediate work in.  This is accomplished
      **  by calling the bff routine.
      ** -------------------------------------------------------------- */

      if ((rc = bff (toc_ptr, destdir, workdir)) == INUGOOD)
      {
         /** -------------------------------------------------------------- *
          **  Since the call to bff was successful, we know the images are 
          **  now in the destdir directory.  Now, we need to create a .toc 
          **  file for them in that dir by calling the inutoc command.
          ** -------------------------------------------------------------- */

          sprintf (cmd, "/usr/sbin/inutoc %s", destdir);

          if ((rc = system (cmd)) != INUGOOD)
          {
              inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, 
                                   CMN_SYSCOM_D), inu_cmdname, cmd);
          }
      }
   }

   set_tape_blk_size_back (toc_ptr, device);

   /* Close the message catalog */
   CATCLOSE ();

   exit (rc);
}



/* -----------------------------------------------------------------------
**
** FUNCTION:          mark_toc_as_notfnd
**
** DESCRIPTION:
**                    Marks every node in the toc linked list
**                    as UNMARKED -- which indicates that the given node does
**                    not need to be processed.
**
** PARAMETERS:
**                    toc_ptr  -- ptr to head of toc linked list
**
** RETURNS:           none (void function)
**
** ----------------------------------------------------------------------- */

void mark_toc_as_notfnd
 (
    TOC_t * toc_ptr            /* pointer to toc head of toc linked list */
)

{
    BffRef_t *bff;             /* used to traverse bff linked list       */

    for (bff = toc_ptr->content->next;
         bff != NIL (BffRef_t) ;
         bff = bff->next)

         bff->crc = UNMARKED;

}

/* -----------------------------------------------------------------------
**
** FUNCTION:      mark_lpps
**
** DESCRIPTION:   Selectively marks the toc linked list entries
**                as MARKED.  If all is nonzero, then all entries in the toc
**                linked list are marked.  Else, only those names in 
**                argv[optind], argv [optind+1], ... , argv [argc] will be 
**                marked.  This is because they specify which bffs the user 
**                specified on the command line that he wants to process.
**
** PARAMETERS:    toc_ptr  -  anchor to toc linked list (bff linked list)
**                all      -  BOOLEAN 1==> mark all bffs, 0 ==> only some
**                argv     -  same as main's argv (cmd line args)
**                device   -  which device to get bffs from
**
** RETURNS:       INUGOOD   -- if success
**                INUBADLP  -- if a bad lpp was specified
**
** ----------------------------------------------------------------------- */

int mark_lpps

 (
     TOC_t *toc_ptr,    /* toc linked list pointer                     */
     int   all,         /* boolean ==> "all" given from cmdline or not */
     char  *argv [],    /* Actual cmdline arguments                    */
     int   argc,        /* Count of cmdline arguments                  */
     char  *device      /* device containing images                    */
)

{
    BffRef_t *bff;             /* used to traverse bff linked list       */
    int      i;                /* dummy counter                          */
    int      found=0;          /* whether bff found in toc linked list or not */
    char     lppname [PATH_MAX*2]; /* Holds the lpp name being compared */
    int      nmarked=0;        /* number of images marked to process    */

    char     cl_prodname[MAX_LPP_FULLNAME_LEN];   /* prodname from cmdline */
    Level_t  cl_level;                            /* level from cmdline    */


   /** --------------------------------------------------------------- *
    **  If all = TRUE, then argc = optind + 1.  So, the outer for loop
    **  should only execute once.
    ** --------------------------------------------------------------- */

    for (i=optind;  i < argc;  i++)
    {
       found = 0;

       strcpy (cl_prodname, argv[i]);

      /** --------------------------------------------------------------- *
       **  If there's another argument, and it begins with a digit, then
       **  we'll treat it as a level (vv.rr.mmmm.ffff.[ppppppp]).
       ** --------------------------------------------------------------- */

       if (i<argc  &&  isdigit (argv[i+1][0]))
       {
          if (inu_level_convert (argv[i+1], &cl_level) != SUCCESS)
          {
             inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_INVALID_LEVEL_E, 
                                  BFF_INVALID_LEVEL_D), argv[i], argv[i+1]);
             inu_msg (SCREEN_LOG, "\n");
             inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_USAGE_E, BFF_USAGE_D));
             return INUBADLP;
          }
          ++i;
       }
       else
          cl_level.ver = -1;   /* indicates no level specified */

      /** --------------------------------------------------------------- *
       **  Search for the pkg in the toc linked list (unless all == TRUE, 
       **  then mark all entries) and mark it as MARKED.
       ** --------------------------------------------------------------- */

       for (bff=toc_ptr->content->next;  bff != NIL (BffRef_t);  bff=bff->next)
       {
	   if (bff->options->next->option == NULL)
             {
                bff->crc = UNMARKED;
                continue;
             }

           if (all == TRUE)
           {
               bff->crc = MARKED;
               continue;
           }
          
           inu_get_prodname (bff->options->next->option, lppname);

          /** ------------------------------------------------------------ *
           **  If we have a match on the product name ...
           ** ------------------------------------------------------------ */

           if (strcmp (cl_prodname, lppname) == 0 || 
		(IF_4_1_UPDATE(bff->options->next->option->op_type) &&
		 strcmp(bff->options->next->option->name,cl_prodname)==0))
           {
              /** ---------------------------------------------------------- *
               **  If no level was given on the cmdline, we have a match ...
               ** ---------------------------------------------------------- */

               if (cl_level.ver == -1)  
               {
                  bff->crc = MARKED;
                  found = TRUE;
                  ++nmarked;
               }

              /** ------------------------------------------------------- *
               **  else we need to make sure the level matches.
               ** ------------------------------------------------------- */

               else if (inu_level_compare (&cl_level, 
                             &(bff->options->next->option->level)) == 0)
               {
                  bff->crc = MARKED;
                  found = TRUE;
                  ++nmarked;
               }
           }
       }  /* for */

      /** --------------------------------------------------------------- *
       **  If the specified lpp was not found, issue a message and return 
       **  failure.
       ** --------------------------------------------------------------- */

       if (found == 0  &&  all == FALSE)
       {
          if (cl_level.ver == -1)
          {
             inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_PKG_NOT_FOUND_E, 
                                  BFF_PKG_NOT_FOUND_D), argv[i], device);
          }
          else
          {
             inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_PKG_W_LEVEL_NOT_FOUND_E, 
                                  BFF_PKG_W_LEVEL_NOT_FOUND_D), 
                                  argv[i-1], argv [i], device);
          }

          return INUBADLP;
       }
    } /* for */

    return INUGOOD;
}


/* -----------------------------------------------------------------------
**
** Function:          ensure_dir_exists
**
** Purpose:           Simply ensures that the directory specified by
**                    dir really does exist.  If it doesn't exist, it gets
**                    created.  If it does, it has to be a directory, or 
**                    an error is returned.
**
** Parameters:        dir   -- the directory to ensure the existence of
**
** Returns:           INUGOOD  -- if success
**                    INUBADIR -- if dir exists, but is really a file
**                    rc       -- from inu_mk_dir if it fails
**
** ----------------------------------------------------------------------- */

int ensure_dir_exists
 (
     char * dir  /* FULL path of a dir to ensure the existence of */
)

{
    struct stat st_buf;      /* for stat call */
    int rc=0;


    if ((stat (dir, &st_buf) == 0))
    {
        /**
         ** if dir is a file and not a dir, return error
         **/

        if ( ! (S_ISDIR (st_buf.st_mode)))
        {
           /**
            **  dir is NOT a dir, but is a file.
            **/

            inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                                 CMN_CANT_CHDIR_D), inu_cmdname, dir);
            return INUBADIR;
        }
        return INUGOOD;
    }

    /**
     **  If we get here, then dir does NOT exist.  So, make it.
     **/

    if ((rc = inu_mk_dir (dir)) != INUGOOD)
    {
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                             CMN_NO_MK_DIR_D), inu_cmdname, dir);
        return rc;
    }

    return rc;
}


/* -----------------------------------------------------------------------
**
**  Function     set_tape_blk_size_back
**
**  Purpose      Sets the tape block size back to what it was before we
**               were invoked.
**
**  Returns      None
**
**  Notes        None
**
** ----------------------------------------------------------------------- */

void set_tape_blk_size_back (TOC_t *toc_ptr, char *device)
{

  /**
   **  If the device that we're dealing with is a tape device, 
   **  then we need to set the block size of the tape device back
   **  to what it originally was.
   **/

   if (toc_ptr->type == TYPE_TAPE_SKD  ||  toc_ptr->type == TYPE_TAPE_BFF)
       if (device != NIL (char)  &&  *device != '\0')
           tchg_blksize (device, 0);
}

/* -----------------------------------------------------------------------
**
**  Function    mark_lpps_driver
**
**  Purpose     Marks each toc entry that the user has selected
**
**  Returns     INUGOOD - if all successful
**             INUBADLP - if bad lpp name given
**
**  Notes       None
**
** ----------------------------------------------------------------------- */

int mark_lpps_driver 
 (TOC_t *toc_ptr,    /* ptr to toc linked list   */
  int argc,          /* number of cmdline args   */
  char *argv[],      /* actual cmdline args      */
  char *device,      /* device containing images */
  int cur_index)     /* current index into argv  */
{
   int rc=INUGOOD;
   int all_keyword=FALSE;

  /** ------------------------------------------------------- *
   **  Make sure the all keyword wasn't used illegally. 
   ** ------------------------------------------------------- */

   if (check_for_all_keyword (&all_keyword, argv, argc, cur_index) != INUGOOD)
      return INUBADLP;

  /** ------------------------------------------------------- *
   **  UNMARK all entries in the toc linked list ...
   ** ------------------------------------------------------- */

   mark_toc_as_notfnd (toc_ptr);

  /** --------------------------------------------------------------- *
   **  Mark all toc entries that the user specified from the cmdline
   ** --------------------------------------------------------------- */

   if ((rc = (mark_lpps (toc_ptr, all_keyword, argv, argc, device))) != INUGOOD)
      set_tape_blk_size_back (toc_ptr, device);

   return rc;
}

/* -----------------------------------------------------------------------
**
**  Function    list_media_contents
**
**  Purpose     Prints a report of media contents based on product name and
**              level.  The installp -l listing is based on option name and
**              level.
**
**  Returns     None
**
**  Notes       None
**
** ----------------------------------------------------------------------- */

void list_media_contents 
 (TOC_t * toc)    /* ptr to Toc linked list head */
{
  BffRef_t   *bff;                    /* Traversal ptr                   */
  char        level [MAX_LEVEL_LEN];  /* level string                    */ 
  int         num_products=0;         /* # products to print             */
  int         product_num=0;          /* current product bing processing */
  prod_lev  **sort_array,             /* contains entry for each image   */
             *node;                   /* current product being processed */
  int         i;

 /** ------------------------------------------------------------------ *
  **  Count the number of bffs on the media (product pkgs).
  ** ------------------------------------------------------------------ */

  for (bff= toc->content->next;
       bff != NIL (BffRef_t);
       bff = bff->next)
  {
     num_products ++;
  }

 /** ------------------------------------------------------------------ *
  **  Copy the prodnames/levels into an array that we can sort. 
  ** ------------------------------------------------------------------ */

  sort_array = (prod_lev **) malloc (num_products * (sizeof (prod_lev *)));

  product_num = 0;
  for (bff= toc->content->next;
       bff != NIL (BffRef_t);
       bff = bff->next)
  {
     if ((node = (prod_lev *) malloc (sizeof (prod_lev))) == NULL)
     {
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                             CMN_MALLOC_FAILED_D));
        exit (INUMALLOC);
     }
     if(IF_4_1_UPDATE(bff->options->next->option->op_type))
     	strcpy (node->prodname,    bff->options->next->option->name);
     else
        strcpy (node->prodname,    bff->options->next->option->prodname);

     node->level.ver       =   bff->options->next->option->level.ver;
     node->level.rel       =   bff->options->next->option->level.rel;
     node->level.mod       =   bff->options->next->option->level.mod;
     node->level.fix       =   bff->options->next->option->level.fix;
     strcpy (node->level.ptf,   bff->options->next->option->level.ptf);

     node->content         =   bff->options->next->option->content;

     sort_array [product_num++] = node;
  }

 /** ------------------------------------------------------------------ *
  **  Sort the list and print report header
  ** ------------------------------------------------------------------ */

  sort_prod_list (sort_array, num_products);

  if (J_flag)
     fprintf (stdout, "#");

  inu_msg (LOG_MSG, MSGSTR (MS_BFF, BFF_PKG_LIST_HDR_I, BFF_PKG_LIST_HDR_D));

  if (J_flag)
     fprintf (stdout, "#");

  fprintf (stdout, "----------------------------------------");
  fprintf (stdout, "--------------------------------------\n");


 /** ------------------------------------------------------------------ *
  **  Print the actual report of the form "prodname level content".
  ** ------------------------------------------------------------------ */

  for (i=0;
       i < num_products;
       i++)
  {
     if (sort_array[i]->level.ptf[0] != '\0')
        sprintf (level, "%d.%d.%d.%d.%s",         sort_array[i]->level.ver, 
                        sort_array[i]->level.rel, sort_array[i]->level.mod, 
                        sort_array[i]->level.fix, sort_array[i]->level.ptf);
     else
        sprintf (level, "%d.%d.%d.%d",           sort_array[i]->level.ver, 
                        sort_array[i]->level.rel, sort_array[i]->level.mod, 
                        sort_array[i]->level.fix);

     fprintf (stdout, "   %-30s %-25s ", sort_array[i]->prodname, level);
     

     switch (sort_array[i]->content)
     {
        case CONTENT_USR:
             inu_msg (LOG_MSG, MSGSTR (MS_STRINGS, USR2_STR, USR2_STR_D));
             break;
        case CONTENT_BOTH:
             inu_msg (LOG_MSG, MSGSTR (MS_STRINGS, BOTH_STR, BOTH_STR_D));
             break;
        case CONTENT_SHARE:
             inu_msg (LOG_MSG, MSGSTR (MS_STRINGS, SHARE2_STR, 
                                                      SHARE2_STR_D));
             break;
     }

     fprintf (stdout, "\n");
  }
}



/* -----------------------------------------------------------------------
**
**  Function    check_for_all_keyword
**
**  Purpose     Checks if the "all" keyword was given on cmdline, and if
**              it was, whether it was legal or not.
**
**  Returns     INUGOOD - if no illegal use or not given at all
**              nonzero - if illegal use of all flag
**
**  Notes       None
**
** ----------------------------------------------------------------------- */

int check_for_all_keyword 
  (int  * all_keyword,         /* We set this and return answer to caller */ 
   char * argv[],           /* Actual cmdline arguments                */
   int    argc,             /* Count of cmdline arguments              */
   int    cur_index)        /* Current index we're processing          */
{
   int rc=INUGOOD;
   int num_in_lpp_list=0;
   int i=0;

  /** ---------------------------------------------------------- *
   **  See if the "all" keyword was given on the cmdline.
   ** ---------------------------------------------------------- */

   for (i=cur_index; i <= argc-1; ++i)
      if (strcmp (argv[i], "all") == 0 
                    || 
          strcmp (argv[i], "ALL") == 0)
      {
         *all_keyword = TRUE;
         break;
      }
   
  /** ---------------------------------------------------------- *
   **  If all was given with an lpp list, issue an error msg.  
   ** ---------------------------------------------------------- */

   if ((argc - cur_index) > 1  &&  *all_keyword == TRUE)
   {
      inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_NO_ALL_W_PKG_LIST_E, 
                                           BFF_NO_ALL_W_PKG_LIST_D));
      rc = BFFCSYNTAX;
   }
   return rc;
}

/* -----------------------------------------------------------------------
**
**  Function    resolve_to_full_path
**
**  Purpose     Resolves the input path to a full path name and sends it
**              back as outpath.
**             
**  Returns     None
**
** ----------------------------------------------------------------------- */

void   resolve_to_full_path 
 (char *inpath,         /*  Path name to resolve                   */
  char *outpath,        /*  Resolved path name                     */
  char *cwd)            /*  Full path to current working directory */
{

  /** --------------------------------------------------------------- *
   **  If we already have a full path, return it.
   ** --------------------------------------------------------------- */

   if (inpath[0] == '/')
      strcpy (outpath, inpath);

  /** --------------------------------------------------------------- *
   **  If '.' was given, just return cwd. 
   ** --------------------------------------------------------------- */

   else if (inpath[0] == '.'  &&  inpath[1] == '\0')
      strcpy (outpath, cwd);

  /** --------------------------------------------------------------- *
   **  Else if we have a relative path beginning with "./", 
   **  axe the "./", and prepend the rest of the given path with cwd.
   ** --------------------------------------------------------------- */

   else if (inpath[0] == '.'  &&  inpath[1] == '/')
      sprintf (outpath, "%s/%s", cwd, &(inpath[2]));

  /** --------------------------------------------------------------- *
   **  Else we must have a "../" relative path, 
   **  so, prepend the resto of the given path with cwd.
   ** --------------------------------------------------------------- */

   else 
      sprintf (outpath, "%s/%s", cwd, inpath);
}

