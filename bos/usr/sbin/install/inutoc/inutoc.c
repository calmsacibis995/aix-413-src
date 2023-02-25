static char     sccsid[] = "@(#)14  1.27  src/bos/usr/sbin/install/inutoc/inutoc.c, cmdinstl, bos411, 9428A410j 4/22/94 10:53:59";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: dump_toc
 *              isa_bff
 *              main
 *              process_existing_bffs
 *              remove_toc_entry
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
#include <inutoc.h>

TOC_t * toc_ptr;             /* ptr to toc linked list returned from open_toc */
int     userblksize = 512;
FILE  * fp;                  /* FILE ptr to .toc file          */

Option_t * SopPtr;   /* THIS GUY IS NEVER USED--ONLY HERE TO RESOLVE AN  */
                     /* EXTERN DECLARATION IN inu_sop_func.c (in inulib) */


int main (int    argc, 
          char * argv[])
{

          char   tocdir[PATH_MAX + 1];         /* dir to create .toc file in */
          char   tocfile[PATH_MAX * 2];        /* dir to create .toc file in */
          char   cwd[PATH_MAX + 1];   /* temp string for intermediate work   */
          char * tmpstr;              /* temp string for reading in strings  */
          char   tmpdir[PATH_MAX + 1];       /* to create a temp dir name in */
   struct stat   st_buf;                     /* for call to stat */
          int    rc;

   inu_cmdname = "inutoc";      /* for inu_msg call */

   (void) setlocale (LC_ALL, "");

   CATOPEN ();

   /**
    **  Check command line arguments (none are required)
    **/

   if (argc > 2)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_TOO_MN_E, CMN_TOO_MN_D), 
                                 inu_cmdname);
      inu_msg (INFO_MSG, MSGSTR (MS_INUTOC, INUTOC_USAGE_E, INUTOC_USAGE_D));
      return (INUSAGE);
   }

   /**
    **   Get the current working directory, in case we need it.
    **/

   if ((getcwd (cwd, (int) sizeof (cwd) - 1)) == NIL (char))
   {
      perror (inu_cmdname);
      return (INUSYSFL);
   }

   /**
    **         Set tocdir (the directory to create/update the toc in)
    **  If dir was specified from the command line, 
    **         -- check if it's valid.
    **  If no dir was specified from the command line, 
    **         -- use the default dir -- DFLT_TOC_DIR.
    **/

   if (argc == 2)       /* then a dir was specified by user */
   {
      if (strcmp (argv[1], "-?") == 0)
      {
         inu_msg (INFO_MSG, MSGSTR (MS_INUTOC, INUTOC_USAGE_E, INUTOC_USAGE_D));
         return INUGOOD;
      }

      rc = stat (argv[1], &st_buf);

      if (rc == -1  ||  ! (S_ISDIR (st_buf.st_mode)))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUTOC, INUTOC_BADIR_E, INUTOC_BADIR_D), 
                                    argv[1]);
         return (INUBADIR);
      }

      /**
       **  Assure that tocdir is an absolute path (starts with "/")
       **
       **  Based on the 1st character of the 1st paramter to inutoc, 
       **  there are basically 3 VALID cases:
       **
       **     case 1: argv [1][1] = '/'            as in "inutoc /absolutedir"
       **     case 2: argv [1][1] = '.'            as in "inutoc ./relativedir"
       **     case 3: argv [1][1] = something else as in "inutoc relativedir"
       **
       **     and one detectable INVALID case:
       **
       **     case 4: argv [1][1] = '.'    and
       **             argv [1][2] = '.'            as in "inutoc ../something"
       **/

      switch (argv[1][0])       /* 1st char of 1st paramter */
      {
         case '/':  /* Absolute directory */

            strcpy (tocdir, argv[1]);
            break;


         case '.': /* Relative directory */

            if (argv[1][1] == '/')
            {
               /* call was "inutoc ./dir" */

               tmpstr = &(argv[1][2]);
               sprintf (tocdir, "%s/%s", cwd, tmpstr);
            }
            else
            {
               if (argv[1][1] == '.')
               {
                  /**
                   **  Then call was "inutoc ..something", which is not
                   **  permitted.
                   **/

                  inu_msg (FAIL_MSG, MSGSTR (MS_INUTOC, INUTOC_USAGE_E, 
                                     INUTOC_USAGE_D));
                  return (INUSAGE);
               }
               else /* call was "inutoc ." */
                  strcpy (tocdir, cwd);
            }
            break;


         default: /* Relative directory */

            sprintf (tocdir, "%s/%s", cwd, argv[1]);
            break;
      } /* end switch */

   } /* end if argc == 2 */
   else /* no dir specified from command line */
      strcpy (tocdir, DFLT_TOC_DIR);

   /**
    **  cd to a temporary working directory.  This is so any
    **  restore'ing of files is done in a temp dir and not
    **  in the calling dir.  This way we don't accidently
    **  wipe out any files in the current dir -- like the
    **  lpp_name file.
    **/

   strcpy (tmpdir, "/tmp/XXXXXX");
   mktemp (tmpdir);

   if ((inu_mk_dir (tmpdir)) != INUGOOD)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_TMPDIR_E, 
               CMN_NO_MK_TMPDIR_D), inu_cmdname);
      return INUNOMK;
   }

   if ((chdir (tmpdir)) != SUCCESS)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
               CMN_CANT_CHDIR_D), inu_cmdname, tmpdir);
      return INUCHDIR;
   }

   /**
    **  Attempt to create the .toc file.  This is done here to
    **  handle the case of dealing with a read-only dir. The user
    **  shouldn't have to wait until all other processing is done
    **  to get the "can't create the .toc file" msg.
    **/

   sprintf (tocfile, "%s/%s", tocdir, TOC_FILE);

   if ((fp = fopen (tocfile, "w")) == NULL)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
               CMN_CANT_CREATE_D), inu_cmdname, tocfile);
      exit (INUOPEN);
   } /* end if */

   /**
    **  Read the toc into memory -- give message and exit if fails
    **/

   if (open_toc (&toc_ptr, tocdir, Q_FLAG_ON, CALLER_INUTOC) != INUGOOD)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CREATE_TOC_E, 
                         CMN_CREATE_TOC_D), inu_cmdname, tocdir);
      return (INUCRTOC);
   } /* end if */

   /**
    **  Make sure all bff's in current dir have an entry in the toc
    **/

   process_existing_bffs (tocdir);

   /**
    **  Change back to the original calling directory and
    **/

   if ((chdir (tocdir)) != SUCCESS)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
               CMN_CANT_CHDIR_D), inu_cmdname, tocdir);
      return INUCHDIR;
   }

   /**
    **  remove the temp dir we just created.
    **/

   inu_rm (tmpdir);

   /**
    ** write out updated toc to ".toc" file in current directory
    **/

   dump_toc ();

   CATCLOSE ();

   return INUGOOD;
} /* of main */

/* -----------------------------------------------------------------------
**
** FUNCTION:            isa_bff
**
** DESCRIPTION:     Determines whether the file specified by the "file"
**        parameter is a bff or not.
**
** PARAMETERS:       file  -  is checked to see if it is a bff (backup
**        format file) or not.
**
** RETURNS:
**              TRUE  - if file IS indeed a bff
**              FALSE - if file is NOT a bff
**
** ----------------------------------------------------------------------- */

int isa_bff (char * file)
{
   int             rc = FALSE;
   struct stat     st_buf;      /* for call to stat     */
   FILE           *bfffp;       /* bff file ptr         */
   char           *rawdata;


   /**
     **  stat file and make sure that 1. it exists, and 2. it is
     **  a regular file (not a directory, named pipe, socket, etc).
     **  All bff's are regular files.
     **/

   rc = stat (file, &st_buf);

   if (rc == -1  ||  ! S_ISREG (st_buf.st_mode))
   {
      rc = FALSE;
   }
   else
   {
      /**
       **  The file at least exists, and is regular.  So see if the
       **  file has the "MAGIC" number in the first few bytes of the
       **  file.  If it does, it's most likely a bff.   If it does
       **  not, then it is not a bff.
       **/

      if ((bfffp = fopen (file, "r")) == NIL (FILE))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                            CMN_CANT_OPEN_D), file);
         return (INUOPEN);
      }

      if ((rawdata = (char *) malloc (NUM_BYTES + 1)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                  CMN_MALLOC_FAILED_D), inu_cmdname);
         exit (INUMALLOC);
      }

      /**
       **  read the first NUM_BYTES of the file -- which will contain
       **  the MAGIC number -- 0x6bea, 0x6cea, or 0x6dea if file is a bff.
       **/

       /*
        * if fread fails, stop now or else malloc'd space
        * may contain misleading information
        */

        if ((rc = fread (rawdata, 1, NUM_BYTES, bfffp)) <= 0)
        {
           rc = FALSE;
           fclose (bfffp);
           free (rawdata);
           return (rc);
        }




      if ((* (rawdata + 3) == 0xea))     /* 1st byte of magic number     */
      {
         switch (* (rawdata + 2))        /* 2nd byte of the MAGIC number */
         {
            case 0x6b:
               rc = TRUE;
               break;
            case 0x6c:
               rc = TRUE;
               break;
            case 0x6d:
               rc = TRUE;
               break;
            default:
               rc = FALSE;
         }
      } /* of if */
      else   /* we know * (rawdata+2) != 0xea */
      {
         rc = FALSE;
      } /* end if */

      fclose (bfffp);
      free (rawdata);

   }    /* of else */

   return rc;

} /* end isa_bff */

/* -----------------------------------------------------------------------
**
** FUNCTION:  process_existing_bffs
**
** DESCRIPTION:         Ensures that every bff in the tocdir directory has
**    a valid entry in the toc linked list.  Basically, this is done by
**    brute force.  Every bff in the tocdir is always added to the toc
**    linked list.  This is to ensure that the real contents of the bff
**    are always reflected by the toc linked list.
**
** PARAMETERS:      tocdir - spcecifies the directory to
**
** RETURNS:         none (void function)
**
** ----------------------------------------------------------------------- */

void process_existing_bffs (char * tocdir)
{

   DIR            *dirp;        /* pointer to the directory to create/update
                                 * toc in   */
   struct dirent  *dp;  /* dir ptr containing info returned from readdir call */
   int             rc;
   BffRef_t       *prev,        /* ptr to previous node in toc linked list            */
                  *bff; /* current bff ptr in toc linked list                 */
   char            filepath[PATH_MAX + 1];      /* full path to the bff file             */
   char           *tocfile;     /* bff filename from the toc linked list */
   char            tmpdir[PATH_MAX + 1];        /* temp working directory                */



   /**
    **  Open the directory for reading.
    **/

   if ((dirp = opendir (tocdir)) == NIL (DIR))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                         CMN_CANT_OPEN_D), inu_cmdname, tocdir);
      exit (INUOPEN);
   }


   /**
     **         Read each filename in the tocdir directory.
     **  If a given file is a bff file, then it should
     **  have a corresponding entry in the toc linked list.
     **/

   while ((dp = readdir (dirp)) != NIL (struct dirent))
   {
      if (strcmp (dp->d_name, ".") == 0  ||  strcmp (dp->d_name, "..") == 0)
         continue;

      /**
       **   Check if dp->d_name is a bff image. If it is, add an
       **  entry in the toc linked list for it.
       **/

      strcpy (filepath, tocdir);
      strcat (filepath, "/");
      strcat (filepath, dp->d_name);

      if (isa_bff (filepath) != TRUE)
         continue;


      /**
       **  Add the bff to the toc linked list
       **/

      rc = load_bff_toc (toc_ptr, filepath, S_IFREG, 1, CALLER_INUTOC);

      switch (rc)
      {
         case INUGOOD:  /* IS a bff, and inserted correctly   */
            break;

         case FAILURE:  /* Is NOT a bff, so just continue  */
            break;

         case TOC_FAILURE:      /* IS a bff, but inserted incorrectly */
            /* So, remove the incorrect entry     */

            for (bff = toc_ptr->content->next, prev = toc_ptr->content;
                 bff != NIL (BffRef_t);
                 bff = bff->next)
            {
               tocfile = getbasename (bff->path);

               /**
                **  Check if we have the correct bff
                ** node in the linked list.  If we do, 
                ** remove it and exit the loop.
                **/

               if (strcmp (tocfile, dp->d_name) == 0)
               {
                  remove_toc_entry (prev, bff);
                  break;        /* exit the for loop */
               }
               else
                  prev = prev->next;
            }
            break;

         default:
            break;
      } /* end switch */
   } /* end while */

   closedir (dirp);

} /* end process_existing_bffs */

/* -----------------------------------------------------------------------
**
** FUNCTION:       dump_toc
**
** DESCRIPTION:    This is the function that actually creates the
**        .toc file in the directory specified by tocdir --
**        which should be the current dir.
**
** PARAMETERS:    none
**
** RETURNS:       none (void function)
**
** ----------------------------------------------------------------------- */

void dump_toc (void)

/**
 **  writes entire toc linked list out to a file called ".toc"
 **/
{
   BffRef_t       *bff; /* ptr to current bff node in list being processed  */
   char           *timestamp;   /* for time stamp creation     */
   time_t          ts;  /* for time stamp creation        */
   struct tm      *tp;  /* for time stamp creation        */
   OptionRef_t    *optr;        /* to traverse option linked list within bff
                                 * list */
   int             nrecs_written = 0;   /* number of bff nodes written to .toc
                                         * file */
   int             rc = INUGOOD;
   char           *bffname;     /* name of the bff (no full path in front of
                                 * it) */

   /**
    ** Get current date and time and
    ** put it into the form mmddhhmmssyy and
    ** call it timestamp.
    **/

   if ((ts = time (NULL)) == -1)
      inu_msg (WARN_MSG, MSGSTR (MS_INUTOC, INUTOC_SYSTIME_W, 
                                            INUTOC_SYSTIME_D));
   tp = gmtime (&ts);
   timestamp = ctime (&ts);

   /**
    **  In order to avoid the orbit/sccs problem -- the following
    **  format string gets incorrectly expanded by Orbit/sccs --
    **  the following macro is used.
    **/

#define INU_TIME_FORMAT "%m\
%d\
%H\
%M\
%S\
%y"

    strftime (timestamp, 35, INU_TIME_FORMAT, tp);

    /**
     **  line #1: the toc header--format is "Volume Time-stamp Header-format"
     **/

     fprintf (fp, "0 %s 2\n", timestamp);


    /**
     **  Write out toc info for each node in the bff linked list
     **/

    /* the bff for loop */

    for (bff = toc_ptr->content->next;
         bff != NIL (BffRef_t)  &&  rc == INUGOOD;
         bff = bff->next)
    {
       /**
        ** line #2
        **/

       /**
        **  open_toc puts the full path name of the bff in the bff->path
        **  variable -- for example "/usr/sys/inst.images/bosnet.bff".
        **  We only want to put the actual file name in the toc -- for
        **  example "bosnet.bff".  Hence the call to getbasename ().
        **/

       bffname = getbasename (bff->path);
      
       /* -------------------------------------------------------------
        * Print the product name only for 4.1 bffs -- Defect 86167
        * ------------------------------------------------------------ */
       fprintf (fp, "%s %c %c %s %s {\n", bffname, bff->fmt, bff->platform, 
                     bff->action_string, (bff->fmt == FMT_4_1) ? 
                     bff->options->next->option->prodname : "");

       if (bff->action == ACT_UNKNOWN)
        {
          inu_msg (WARN_MSG, MSGSTR (MS_INUTOC, INUTOC_UNKNOWN_TYPE_W, 
                   INUTOC_UNKNOWN_TYPE_D), inu_cmdname, bffname, 
                   bff->action_string);
        }
       /**
        ** loop thru all the options for this lpp (bff)
        **/

       /* the option for loop */

       for (optr = bff->options->next;  /* 1st one is a head (anchor) ptr */
            (optr != NIL (OptionRef_t))  &&  rc == INUGOOD;
            optr = optr->next)
       {

          /**
           ** line #3 -- the "Option" line
           **/

          if (optr->option != NIL (Option_t))
          {
             fprintf (fp, "%s ", optr->option->name);
             fprintf (fp, "%02d.", optr->option->level.ver);
             fprintf (fp, "%02d.", optr->option->level.rel);
             fprintf (fp, "%04d.", optr->option->level.mod);
             fprintf (fp, "%04d", optr->option->level.fix);

             if (optr->option->level.ptf != NIL (char))
                if (* (optr->option->level.ptf) != '\0')
                   fprintf (fp, ".%s ", optr->option->level.ptf);
                else
                   fprintf (fp, " ");

             fprintf (fp, "1 %c ", optr->option->quiesce);
             fprintf (fp, "%c ", optr->option->content);
             fprintf (fp, "%s ", optr->option->lang);
             fprintf (fp, "%s\n", optr->option->desc);

            /**
             ** line #'s 4 - 8
             **/

             if ((optr->option->prereq)[0] != '\n')
                fprintf (fp, "[\n");
             else
                fprintf (fp, "[");
             fprintf (fp, "%s", optr->option->prereq);

             /**
              ** line #'s 9 - ?
              **/

             fprintf (fp, "%%\n");
             fprintf (fp, "%s", optr->option->size);

             /**
              ** line #'s ? - ?
              **/

             fprintf (fp, "%%\n");
             fprintf (fp, "%s", optr->option->supersedes);

            /** ------------------------------------------------- *
             **  Write the NetLs info ...
             ** ------------------------------------------------- */

             fprintf (fp, "%%\n");

             if (optr->option->netls != NIL (Netls_t)            &&  
                 optr->option->netls->vendor_id != NIL (char)    && 
                 optr->option->netls->prod_id   != NIL (char)    && 
                 optr->option->netls->prod_ver  != NIL (char))
             {
                fprintf (fp, "%s\n", optr->option->netls->vendor_id);
                fprintf (fp, "%s\n", optr->option->netls->prod_id);
                fprintf (fp, "%s\n", optr->option->netls->prod_ver);
             }

            /** ------------------------------------------------- *
             **  Write the fixdata info ...
             ** ------------------------------------------------- */

             fprintf (fp, "%%\n");
             fprintf (fp, "%s", optr->option->fixdata);


	     /* write the closing brace */

             fprintf (fp, "]\n");
             ++nrecs_written;

             /**
              **  Flush the buffer and write everything out to
              **  disk.  If the filesystem is full, then this
              **  is where we will detect it.
              **/

             rc = fflush (fp);

          } /* end if optr->option != NIL */
       } /* end option for */

       fprintf (fp, "}\n");

    } /* end for bff (lpp)  */

   if (rc == INUGOOD)
      rc = fclose (fp);

   if (rc != 0)
   {
      /**
       **  Then we had a problem writing the .toc file out.
       **  So, remove any half-existing ".toc" file and exit.
       **/

      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
               CMN_CANT_CREATE_D), inu_cmdname, ".toc");

      if (access (".toc", F_OK) == 0)
         (void) unlink (TOC_FILE);

      exit (INUNOWRT);
   }

   /**
    **  If no complete toc entries were written out
    **  remove the .toc file, cuz it only contains
    **  a partial toc entry.
    **/

   if (nrecs_written == 0)
      unlink (TOC_FILE);        /* only the header part is there--so remove
                                 * file */
} /* end dump_toc */

/* -----------------------------------------------------------------------
**
** FUNCTION:        remove_toc_entry
**
** DESCRIPTION:     Simply removes a node from a linked list.  To be
**        exact, this function removes the node specified by
**        the parameter bff.
**
**        CAUTION:  The space occupied by bff (the node that gets
**        removed) is NOT reclaimed by a call to free.
**
** PARAMETERS:    prev - a pointer to the node before the node to remove.
**                bff  - a pointer to the node to delete from the list
**
** RETURNS:        None (void function)
**
** ----------------------------------------------------------------------- */


void remove_toc_entry (BffRef_t * prev, 
                       BffRef_t * bff)
{

   /**
     **  We have 4 possible cases to cover here:
     ** -----------------------------------------
     **
     **  H   = head ptr to the linked list
     **  X   = some bff node in the linked list
     **  bff = node to remove from linked list
     **
     **  case 1:  H --> bff --> NULL
     **  case 2:  H --> bff --> X --> X --> NULL
     **  case 3:  H --> X --> bff --> X --> NULL
     **  case 4:  H --> X --> X --> bff --> NULL
     **/

   prev->next = bff->next;

} /* end remove_toc_entry */
