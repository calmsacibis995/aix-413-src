static char     sccsid[] = "@(#)79      1.13.1.12  src/bos/usr/sbin/install/inulib/inu_rcvr_or_rm.c, cmdinstl, bos411, 9428A410j 6/20/94 13:57:35";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: read_stanza_rec, read_options_list, free_optr_mem, 
 *              specified_option, process_update_list, process_archive_list, 
 *              recover_vpd_entry, redo_links, inu_rcvr_or_rm, strip_spaces
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

#include <sys/stat.h>
#include <inulib.h>


#define OPTS_SLOTS      25      /* Num slots allocated in options array */
#define TEMP_UL         "_inu_temp.ul"
#define TEMP_AL         "_inu_temp.al"
#define FOUND           1
#define NOTFOUND        0
#define NO_OPTIONS      0
#define ALL_OPTIONS     1

/* -------------------------------------------------------------------
 *            L O C A  L         P  R  O  T  O  T  Y  P  E  S
 * ------------------------------------------------------------------- */

/**
 **  These prototypes are here in order to avoid needing a ".h" file to
 **  put them into (since this is a common file).
 **/

static int recover_vpd_entry (AFILE_t af);

static int process_update_list (int     caller, 
                                char ** optr, 
                                int     numread);

static int process_archive_list (int     caller, 
                                 char ** optr, 
                                 int     nread);

static int redo_links (char * loc0, 
                       char * loc1, 
                       char * loc2);

static int specified_option (char    ** optr, 
                             AFILE_t    af, 
                             int        numread);

static char ** read_options_list (int * nslots, 
                                  int * nread, 
                                  FILE * fp);

static void free_optr_mem (char ** optr, 
                           int     numslots, 
                           int     numread);

static void save_stanza_rec (AFILE_t   af, 
                             FILE    * fp);

static void read_stanza_rec (AFILE_t   af, 
                             inv_t   * inv);

static int run_ar_cmd (char * cmd, 
                       char * new_archive_file, 
                       char * last_archive, 
                       char * cleanup_cmd);

static void strip_spaces (char s1[]);

static int recover_vpd_entry (AFILE_t);

static int process_update_list (int, char **, int);

/* -----------------------------------------------------------------------
 *                  G L O B A L     variables
 * ----------------------------------------------------------------------- */

int           specified_ops = NO_OPTIONS;     /* options to reject/commit */
extern char * inu_cmdname;         /* Name of command calling these routines */


/* ----------------------------------------------------------------------- *
 *
 * FUNCTION      strip_spaces
 *
 * DESCRIPTION   Removes all occurances of the ' ' character from a string
 *               passed to it.
 *
 * PARAMETERS    s1  -  string to remove spaces from
 *
 * RETURNS       none - void function
 *
 * ----------------------------------------------------------------------- */

static void strip_spaces (char s1[])
{
   int       i, j;              /* indeces into the two strings    */
   char      s2[PATH_MAX * 2];  /* temp string to cp clean s1 into */

   /**
    **  The algorithm is basically to cp every char that's not
    **  a space into a temp string (in order).  Then cp the
    **  temp string back into the passed string.
    **/

   for (i = 0, j = 0;
        s1[i] != '\0';
        i++)
   {
      if (s1[i] != ' ')
         s2[j++] = s1[i];
   } /* end for */

   s2[j] = '\0';

   strcpy (s1, s2);
} /* end strip_spaces */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:     read_stanza_rec
 *
 * DESCRIPTION:  Reads the stanza rec pointed to by af into the inv structure.
 *
 * PARAMETERS:   af  -  an Attribute file ptr to the stanza record
 *               nv -  the inv structure to dump file contents into
 *
 * RETURNS:      none
 *
 * ----------------------------------------------------------------------- */

static void read_stanza_rec (AFILE_t   af, 
                             inv_t   * inv)
{
   char   * pr,          /* private field of inv_t struct  */
          * ft,          /* field of inv_t struct          */
          * format,      /* format field of inv_t struct   */
          * loc0,        /* loc0 field of inv_t struct     */
          * loc1,        /* loc1 field of inv_t struct     */
          * loc2,        /* loc2 field of inv_t struct     */
          * size,        /* size field of inv_t struct     */
          * csum,        /* checksum field of inv_t struct */
          * id,          /* ... and more fields  ...    */
          * reserved,    /* "    "       "         */
          * scratch,     /* "    "       "         */
          * lpp_id;      /* "    "       "         */


   /**************************************************
    **   These fields of the UPDATE_LIST stanza file
    ** may or may not exist.  So, we can't check if
    ** the calls to AFgetatr are successful or not.
    ** In both cases, the return code would be NULL
    **************************************************/

   if (af->AF_catr != (ATTR_t) 0)
   {
      pr = AFgetatr (af->AF_catr, "private");
      ft = AFgetatr (af->AF_catr, "file_type");
      format = AFgetatr (af->AF_catr, "format");
      loc0 = AFgetatr (af->AF_catr, "loc0");
      size = AFgetatr (af->AF_catr, "size");
      csum = AFgetatr (af->AF_catr, "checksum");
      loc1 = AFgetatr (af->AF_catr, "loc1");
      loc2 = AFgetatr (af->AF_catr, "loc2");
      id = AFgetatr (af->AF_catr, "_id");
      reserved = AFgetatr (af->AF_catr, "_reserved");
      scratch = AFgetatr (af->AF_catr, "_scratch");
      lpp_id = AFgetatr (af->AF_catr, "lpp_id");


      /***********************************************
       **  step 2). stuff vpd values into inv_t struct
       ***********************************************/

      strcpy (inv->loc0, loc0);

      inv->loc1 = loc1;
      inv->loc2 = loc2;

      inv->private = (short) atoi (pr);
      inv->file_type = (short) atoi (ft);
      inv->format = (short) atoi (format);
      inv->lpp_id = (short) atoi (lpp_id);

      inv->size = atol (size);
      inv->checksum = atol (csum);
      inv->_reserved = atol (reserved);
      inv->_id = atol (id);
      inv->_scratch = atol (scratch);
   } /* end if */
} /* end read_stanza rec */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:     save_stanza_rec
 *
 * DESCRIPTION:  Writes out the stanza record pointed to by af to the file
 *               specified by fp.
 *
 * PARAMETERS:   af  - Attribute file ptr to the stanza record to * write out
 *               fp  - FILE ptr to file to write record out to
 *
 * RETURNS:      none
 *
 * ----------------------------------------------------------------------- */

static void save_stanza_rec (AFILE_t   af, 
                             FILE    * fp)
{
   char     origfile[PATH_MAX + 1]; /* full path name of original saved file */
   char    *update_num;             /* "update.n" string, where n = 1, 2, 3 ... */
   char    *archive_num;            /* "archive.n" string, where
                                     * n = 1, 2, 3 ... */
   char    *arc_name;               /* name of the archive origfile belongs
                                     *  to */
   inv_t    inv;                    /* used for vpd queries &updates */
   char    *option;                 /* name of the specified option */
   char     option2[MAX_LPP_FULLNAME_LEN];

   /* declare space for vpdreslpp_id call          */

   /**
    **  Read the vpd parts of the stanza record pointed to by
    **  the af pointer into the inv structure.
    **/

   read_stanza_rec (af, &inv);


   /**
    **  Write out the initial part of the stanza record
    **/

   strcpy (origfile, af->AF_cbuf);    /* the full path to orig file */

   update_num = AFgetatr (af->AF_catr, "update.n");
   archive_num = AFgetatr (af->AF_catr, "archive.n");


   if (update_num != NIL (char))
   {
      fprintf (fp, "\n%s:", origfile);  /* name of stanza rec */
      fprintf (fp, "\n        update.n = %s", update_num);
   }    /* of if */
   else
   {
      if (archive_num != NIL (char))
      {
         arc_name = AFgetatr (af->AF_catr, "arc_name");
         fprintf (fp, "\n%s:", origfile);             /* name of stanza rec */
         fprintf (fp, "\n        archive.n = %s", archive_num);
         fprintf (fp, "\n        arc_name = %s", arc_name);
      } /* of else */
   }

   /**
    **  If either of the previous conditions passed, 
    **  then the initial part of the stanza record was
    **  successfully written out.
    **
    **  So, now we want to write out the remainder of the
    **  record.
    **/

   if (update_num != NIL (char)  || archive_num != NIL (char))
   {
      /**
       **  Attempt to read the option from the stanza file.
       **  If the option isn't specified in the stanza file
       **  then try to get it from vpdreslpp_id.
       **/

      option = AFgetatr (af->AF_catr, "option");

      if (option == NIL (char))
      {
         if ((vpdreslpp_id (inv.lpp_id, option2)) == VPD_OK)
            fprintf (fp, "\n        option = %s", option2);
      }
      else
         fprintf (fp, "\n        option = %s", option);

      /* 8 spaces  >12345678< needed here */

      fprintf (fp, "\n        %s = %ld", "_id", inv._id);
      fprintf (fp, "\n        %s = %ld", "_reserved", inv._reserved);
      fprintf (fp, "\n        %s = %ld", "_scratch", inv._scratch);
      fprintf (fp, "\n        %s = %d", "lpp_id", inv.lpp_id);
      fprintf (fp, "\n        %s = %d", "private", inv.private);
      fprintf (fp, "\n        %s = %d", "file_type", inv.file_type);
      fprintf (fp, "\n        %s = %d", "format", inv.format);

      /**
       **  inv.loc0  --  is a char [128];
       **  inv.loc1  --  is a char *;
       **  inv.loc2  --  is a char *;
       **/

      if (inv.loc0[0] != '\0')
         fprintf (fp, "\n        %s = %s", "loc0", inv.loc0);

      if (inv.loc1 != NIL (char)  && *(inv.loc1) != '\0')
         fprintf (fp, "\n        %s = %s", "loc1", inv.loc1);

      if (inv.loc2 != NIL (char)  && *(inv.loc2) != '\0')
         fprintf (fp, "\n        %s = %s", "loc2", inv.loc2);

      fprintf (fp, "\n        %s = %ld", "size", inv.size);
      fprintf (fp, "\n        %s = %ld", "checksum", inv.checksum);
      fprintf (fp, "\n");       /* need a newline delimiter between records */
   } /* end if (update_num  ||  archive_num) */
} /* end save_stanza rec */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    read_options_list
 *
 * DESCRIPTION: Reads the lpp.options file into memory (an array).
 *              The array is really an array of ptrs to character strings.
 *
 * PARAMETERS:  nslots  --  number of slots allocated.  Slots are allocated in
 *                          OPTS_SLOTS increments.
 *
 *              nread   --  Actual number of options read in or actual number
 *                          of used slots.
 *              fp      --  file pointer to the lpp.options file.
 *
 * RETURNS:     NULL      --  if malloc, calloc, or realloc fail
 *              array ptr --  if successful.
 *
 * ----------------------------------------------------------------------- */

static char ** read_options_list (int  * nslots, 
                                  int  * nread, 
                                  FILE * fp)
{
   char ** optr;
   char    tmpstr[MAX_LPP_FULLNAME_LEN];
   char    buf[MAX_LPP_FULLNAME_LEN + 50]; /* 50 for poss 3.1 format of:
                                            * OPTION MOD FIX NEWMOD NEWFIX */
   int     numslots = 0, numread = 0;

   /**
    **   Allocate initial memory chunk
    **/

   if ((optr = (char **) calloc (OPTS_SLOTS, sizeof (char *))) == NIL (char *))
      return NIL (char *);

   numslots = OPTS_SLOTS;

   /**
    **  Read entire optionfile into optr array.
    **
    **  3.2 format - has only the OPTION on a line
    **  3.1 format - has OPTION MOD FIX NEWMOD NEWFIX on a line
    **
    **  So, to be able to handle each case, 
    **  Read an entire line,  then parse the line.
    **/


   for (numread = 0;
        fgets (buf, MAX_LPP_FULLNAME_LEN + 50, fp) != NIL (char);
        ++numread)
   {
      /**
       **  Use only 1st string in buf. If 3.2, then there is
       **  only one string on a line.  If 3.1, then there is
       **  5 separate strings per line.
       **/
      sscanf (buf, "%s", tmpstr);

      /**
       **  realloc more memory if necessary
       **/

      if (numread >= numslots)
      {
         numslots += OPTS_SLOTS;

         optr = (char **) realloc (optr, sizeof (char *) * numslots);

         if (optr == NIL (char *))
            return NIL (char *);
      } /* end if */
      /**
       **  Only malloc the exact amount of space needed.
       **/

      optr[numread] = (char *) malloc (strlen (tmpstr) + 1);

      if (optr[numread] == NIL (char))  /* check if malloc failed */
         return NIL (char *);

      strcpy (optr[numread], tmpstr);

   } /* end for */

   *nslots = numslots;
   *nread = numread;

   return optr;

} /* end read_options_list */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    free_optr_mem
 *
 * DESCRIPTION: Frees the memory previously malloc' into an array of ptrs.
 *              Actually frees the memory pointed to by all array, then frees
 *              the array itself.
 *
 * PARAMETERS:  optr     --    ptr to an array of ptrs to free.
 *              numslots -- number of slots (indices) in array actually
 *                          allocated.
 *              numread  -- number of slots that contain ptrs to memory that
 *                          needs to be freed.
 *
 * RETURNS:     none
 *
 * ----------------------------------------------------------------------- */

static void free_optr_mem (char ** optr, 
                           int     numslots, 
                           int     numread)
{
   int  i;

   if (optr == NIL (char *)  ||  numread == 0  ||  numslots == 0)
      return;

   for (i = 0; i < numread; i++)
      free (optr[i]);

   free (optr);

} /* end free_optr_mem */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    specified_option
 *
 * DESCRIPTION: Determines if the option associated with the stanza record af
 *              is in the option array (optr).  A simple yes or no (NOTFOUND)
 *              is returned.
 *
 * PARAMETERS:  optr    --  is an array of option names
 *              af      --  stanza record with associated option to check
 *              numread --  number of options in array
 *
 * RETURNS:     nonzero - if option is a specified option (in optr)
 *              zero - if * option is not a specified option
 *
 * ----------------------------------------------------------------------- */

static int specified_option (char    ** optr, 
                             AFILE_t    af, 
                             int        numread)
{
   int             i;
   char           *option;      /* option of the current record */


   if (specified_ops == ALL_OPTIONS)
      return FOUND;     /* No optionfile specified, so assume all options */

   option = AFgetatr (af->AF_catr, "option");

   /**
    **  If no option is found, then we can assume that
    **  this stanza record is just a "stub" record -- or
    **  that it just has the record name (full path name
    **  of orig file) and the update.n name.  So, we
    **  want to reject/commit this one.  It will never
    **  get rejected/commited if we do not do it here.
    **/

   if (option == NIL (char))
      return FOUND;


   for (i = 0; optr[i]  &&  i < numread; i++)
   {
      if (strcmp (optr[i], option) == 0)
         return FOUND;  /* nonzero */
   } /* end for */

   return NOTFOUND;     /* zero */

} /* end specified_option */

/* ---------------------------------------------------------------------- */

/***************
 ** UPDATE_LIST
 ***************/

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    process_update_list
 *
 * DESCRIPTION: Recovers the files specified in the UPDATE_LIST file that have
                their associated option specified in the file.
 *
 * PARAMETERS:  caller     --  calling routine inurecv or installp
 *              optr       --  array  of specified option names
 *              numread    --  number of options read in
 *
 * RETURNS:     INUBADUL  --  Bad UPDATE_LIST
 *              NUSYSFL   --  System call failed
 *              INUBADC1  --  File copy failed
 *              INUOPEN   --  Cant open file
 *              INUGOOD   --  if successful
 *
 * ----------------------------------------------------------------------- */

static int process_update_list (int     caller, 
                                char ** optr, 
                                int     numread)
{
   AFILE_t     af;                   /* AF FILE ptr to UPDATE_LIST */
   char        source[PATH_MAX + 1]; /* source file in inu_cp command */
   char       *dest;                 /* destination file in inu_cp command */
   char       *update_file;          /* the saved file name, ie update.1 */
   int         temp_ul_exists = 0;   /* Boolean 1==> exists, 0==> not exist */
   FILE       *tul_fp;               /* Temp Update.List file pointer */
   char       *envar;                /* to get an environment var's value */
   char       *ptr;
   int         rc;

   if ((af = AFopen (UPDATE_LIST, MAX_AFREC_SIZE, MAX_ATTRS)) == (AFILE_t) 0)
   {
      return (INUGOOD);
   }

   /**************************************************************************
    **   Sequentially read all records from the UPDATE_LIST stanza file--using
    ** AF functions which are designed to read attribute files (stanza files).
    ** Each record contains the info necessary to copy the saved file to the
    ** original location, and the info necessary to restore the vpd entry for
    ** the saved file.
    **************************************************************************/

   /* af->AF_cbuf  is name of current record from UPDATE_LIST -- which is */
   /* also original full path name of file that was saved    */

   while ((ptr = AFread (af)) != NULL)   /* sequentially read records from
                                           * UPDATE_LIST til NULL */
   {
      dest = af->AF_cbuf;

      if (dest == NIL (char))
         continue;

      /* get value of "update.n" field--"update.1", for example */

      update_file = AFgetatr (af->AF_catr, "update.n"); /* get update.n val */

      if (specified_option (optr, af, numread) == FOUND)
      {
         if (caller == CALLER_INURECV)
         {
            (void) unlink (af->AF_cbuf);      /* remove in case active */

            getcwd (source, PATH_MAX);
            strcat (source, "/");
            strcat (source, update_file);


            /*
             * Ensure that enough system space exists to allow an inu_cp from
             * source to dest.
             */

            if ((rc = inu_file_eval_size (dest, source, 1)) != INUGOOD)
            {
               if (rc == INUFS)
                  return (rc);
               if (((envar = getenv ("INUEXPAND")) == NIL (char))
                                         || 
                               (envar[0] == '0'))
               {
                  /**
                   ** -X flag was not set, so try to continue execution.
                   **/

                  inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_NOT_ENOUGH_W, 
                                               CMN_NOT_ENOUGH_D), inu_cmdname);
               }
               else  /* not enough space to continue, so die. */
               {
                  inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                                               CMN_NO_SPACE_D), inu_cmdname);
                  return (INUSYSFL);

               } /* end if */

            } /* end if (eval_size) */

            /* Perform the copy */

            if (inu_cp (source, dest) != INUGOOD)       /* then copy failed */
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CPFAIL_E, 
                                    CMN_CPFAIL_D), inu_cmdname, source, dest);
               return (INUBADC1);
            }

            (void) unlink (source);   /* delete "update.n" file in save dir */

            /* RETURN CODE used HERE ??? */

            recover_vpd_entry (af);     /* add vpd info to vpd */
         }
         else  /* Then caller is CALLER_INSTALLP, so we're doing a commit */
         {
            unlink (update_file);
         } /* end if */
      }
      else  /* this record needs to be written out */
      {
         /* Ensure that the TEMP_UL file exists, before saving the stanza rec
          * to it. */

         if ( ! temp_ul_exists)
         {
            if ((tul_fp = fopen (TEMP_UL, "w")) == NIL (FILE))
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                                    CMN_CANT_CREATE_D), inu_cmdname, TEMP_UL);
               return INUOPEN;
            }
            ++temp_ul_exists;
         }

         save_stanza_rec (af, tul_fp);
      } /* end if */
   } /* end while */

   AFclose (af);

   /*
    * Check if TEMP_UL file exists.  If it does, then some options were not
    * recovered/commited. So rename the TEMP_UL to UPDATE_LIST.
    */

   if (temp_ul_exists)
   {
      fclose (tul_fp);
      rename (TEMP_UL, UPDATE_LIST);
   }

   else
      (void) unlink (UPDATE_LIST);

   return (INUGOOD);

} /* end process_update_list */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    run_ar_cmd
 *
 * DESCRIPTION: Runs the command specified by cmd.  If cmd [0] is \0', no
 *              command is executed.
 *              Also runs a cleanup command that removes files after
 *              they have been re-archived (by cmd).
 *
 * PARAMETERS:  cmd               -- ar command to re-archive specified files
 *              new_archive_file  -- intermediate archive used to archive into.
 *              last_archive      -- original archive
 *              cleanup_cmd       -- removes all constituent files after saved.
 *
 * RETURNS:     INUGOOD  - if successful.
 *              INUNORP1 - constituent file was not replaced in archive.
 *              INUSYSFL - system call failed.
 *
 * ----------------------------------------------------------------------- */

static int run_ar_cmd (char * cmd,              /* ar command to re-archive
                                                 * specified files. */
                       char * new_archive_file, /* Intermediate archive used to
                                                 * archive into. */
                       char * last_archive,     /* original archive */
                       char * cleanup_cmd)      /* Removes all constituent
                                                 * files after saved. */
{
   int             rc;
   char           *envar;       /* to get an environment var's value */


   if (*cmd == '\0')
      return (INUGOOD);

   /**
    ** Ensure that enough system space exists to
    ** perform the "ar ro archive c1, c2, ... , cn" command
    **/

   if ((inu_file_eval_size (last_archive, last_archive, 0)) != INUGOOD)
   {
      if (((envar = getenv ("INUEXPAND")) == NIL (char))
                             || 
                      (envar[0] == '0'))
      {
         /**
          **   -X flag was not set, so try to continue execution.
          **  This is because inu_file_eval_size did NOT
          **  try to expand.
          **/

         inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_NOT_ENOUGH_W, 
                                      CMN_NOT_ENOUGH_D), inu_cmdname);
      }
      else  /* not enough space to continue, so die. */
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                                      CMN_NO_SPACE_D), inu_cmdname);
         return (INUSYSFL);
      } /* end if */
   } /* end if */

   rc = inu_do_exec (cmd);   /* cmd = "ar ro last_archive constituent-list" */

   if (rc)       /* then failed */
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ARFAIL_E, CMN_ARFAIL_D), 
                                   inu_cmdname, new_archive_file);
      return (INUNORP1);
   }

   /* Move the copy (the last_archive.new) back to the original (the
    * last_archive). */

   if (rename (new_archive_file, last_archive) != 0)
   {
      inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_RENAME_W, CMN_RENAME_D), 
                                   inu_cmdname, new_archive_file, last_archive);
      return (INUNORP1);
   }

   rc = inu_do_exec (cleanup_cmd);   /* remove files after they're saved */

   if (rc)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D), 
                                   inu_cmdname, cleanup_cmd);
      return (INUNORP1);
   }

   /*
    * Mark cmd as having been ran by emptying out the string.
    */

   *cmd = '\0';

   return (INUGOOD);

} /* end run_ar_cmd */

/* -----------------------------------------------------------------------
 *
 * FUNCTION:     process_archive_list
 *
 * DESCRIPTION:  Analogous to process_update_list in that all files in the
 *               ARCHIVE_LIST file that have their associated option
 *               specified in the option file are either recovered or
 *               removed.  If the caller is inurecv, we assume a reject
 *               action is occurring and therefore the files are recovered
 *               (copied back to their original location).  If the caller is
 *               installp, we assume a commit action is occuring, and the files
 *               are simply removed.
 *
 * PARAMETERS:   caller - calling routine (inurecv or installp)
 *               optr   - ptr to array of specified option names
 *               nread  - number of options contained in optr
 *
 * RETURNS:      INUGOOD  - if successful
 *               INUNORP1 - if constituent file wasn't replaced
 *               INUSYSFL - system call failed
 *
 * ----------------------------------------------------------------------- */

static int process_archive_list (int     caller, 
                                 char ** optr, 
                                 int     nread)
{

   AFILE_t   af;                       /* AF FILE ptr to ARCHIVE_LIST */
   char    * p;                        /* pointer to a temp string */
   char    * origfile;                 /* original file before saved */
   char    * arc_name;                 /* archive name to archive files into */
   char    * archiven;                 /* holds value of archive.n attribute */
   char      cmd[BUFSIZ];              /* string to build commands in */
   char      cleanup_cmd[BUFSIZ];      /* string to build rm command in */
   char      new_archive_file[BUFSIZ]; /* temp archive file */
   char      last_archive[BUFSIZ];     /* name of previous library */
   FILE    * tal_fp;                   /* Temp Archive.List FILE pointer */
   int       temp_al_exists = 0;       /* BOOLEAN - 1 ==> true, 0 ==> false */

   /*****************************************************************
    **  Rename the saved archive file to what it originally was.
    ** Then archive the renamed file to the archive specified by
    ** arc_name.  This is done via using an intermediate temporary
    ** archive file (the ".new" file).
    *****************************************************************/

   if ((af = AFopen (ARCHIVE_LIST, MAX_AFREC_SIZE, MAX_ATTRS)) ==
                                                                (AFILE_t) NULL)
   {
      return INUGOOD;   /* nothing to recover */
   }

   last_archive[0] = '\0';   /* indicate that there is no last archive */
   cmd[0] = '\0';            /* empty out cmd */

   while (AFread (af))  /* until NULL is returned get another stanza record */
   {
      origfile = af->AF_cbuf; /* record name = original full path file */

      if (origfile == NIL (char))
         continue;

      p = getbasename (origfile);       /* assign just the filename to p */
      arc_name = AFgetatr (af->AF_catr, "arc_name");  /* get archive name */

      archiven = AFgetatr (af->AF_catr, "archive.n"); /* value of attribute */

      /**
       **  If the option associated with the current record
       ** is not one that is specified in the options file, 
       ** then saved the record.  This is done by calling
       ** save_stanza_rec.
       **/

      if (specified_option (optr, af, nread) != FOUND)
      {
         /*
          * Run ar command for whatever happens to be in the cmd string.  If
          * the cmd has already been ran, then cmd [0] will == '\0'.  The
          * run_ar_cmd function will detect this, and return INUGOOD.
          */

         if (run_ar_cmd (cmd, new_archive_file, last_archive, 
                         cleanup_cmd) != INUGOOD)
            return INUNORP1;

         if ( ! temp_al_exists)
         {
            ++temp_al_exists;
            if ((tal_fp = fopen (TEMP_AL, "w")) == NIL (FILE))
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                                    CMN_CANT_CREATE_D), inu_cmdname, TEMP_AL);
               return INUOPEN;
            }
         }

         save_stanza_rec (af, tal_fp);
         continue;
      } /* end if specified_option */

      if (caller != CALLER_INURECV)     /* then we're doing a commit */
      {
         if (archiven != NIL (char))
            (void) unlink (archiven);

         continue;
      } /* end if */

      if (strcmp (last_archive, arc_name)  ||  cmd[0] == '\0')
      {
         /* then last_archive != arc_name */

         if (last_archive[0] != '\0')
            if (run_ar_cmd (cmd, new_archive_file, last_archive, 
                            cleanup_cmd) != INUGOOD)
               return INUNORP1;

         strcpy (last_archive, arc_name);
         sprintf (new_archive_file, "%s.new", arc_name);

         if (inu_cp (arc_name, new_archive_file) != INUGOOD)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CPFAIL_E, 
                        CMN_CPFAIL_D), inu_cmdname, arc_name, new_archive_file);
            return (INUNORP1);
         }

         /*************************************************************
          **  Create the commands to archive and clean up files in this
          ** library.  Archive member names will be appended to these
          ** commands.
          **************************************************************/

         sprintf (cmd, "/usr/ccs/bin/ar ro %s", new_archive_file);
         strcpy (cleanup_cmd, "/bin/rm ");
      } /* end if (strcmp) */

      /* Append the next member name to both archive and cleanup commands. */

      strcat (cmd, " ");
      strcat (cmd, p);
      strcat (cleanup_cmd, " ");
      strcat (cleanup_cmd, p);

      if (rename (archiven, p) != 0)    /* rename to proper name */
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSVF_E, 
                                                    CMN_NOSVF_D), archiven);
         return (INUNOSVF);
      }

      /** RETURN CODE HERE ??? **/

      recover_vpd_entry (af);   /* recovers vpd info from the ARCHIVE_FILE
                                 * rec */
   } /* end while */

   /* Finish up last archive, if any */

   if (last_archive[0] != '\0')
   {
      /* then we need to process the last archive */

      if (run_ar_cmd (cmd, new_archive_file, last_archive, cleanup_cmd)
          != INUGOOD)
         return INUNORP1;
   } /* end if */

   AFclose (af);   /* close the attribute file using the AF function */

   if (temp_al_exists)
   {
      fclose (tal_fp);
      rename (TEMP_AL, ARCHIVE_LIST);
   }
   else
      (void) unlink (ARCHIVE_LIST);

   return INUGOOD;

} /* end process_archive_list */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:     recover_vpd_entry
 *
 * DESCRIPTION:  Recovers a vpd entry in the INVENTORY database using the data
 *               specified in the af stanza record.
 *
 * PARAMETERS:   af  -- a stanza record pointer
 *
 * RETURNS:      INUVPDADD  -  Failed adding vpd entry
 *               INUGOOD    -  if successful
 *
 * ----------------------------------------------------------------------- */

static int recover_vpd_entry (AFILE_t af)  /* pointer to a stanza record
                                            * containing VPD info */
{
   inv_t   inv;       /* used for vpd updates           */
   inv_t   inv2;      /* used for vpd queries           */

   memset (&inv, NULL, sizeof (inv_t));
   memset (&inv2, NULL, sizeof (inv_t));

   /*********************************************************************
    **                         Recover the VPD entry
    **
    ** This is done in 3 steps:
    **         1). Read the saved VPD information from the stanza file
    **             that contains vpd info.  This was accomplished by a
    **             previous call to AFread (af parm).  The following
    **             calls to AFgetatr get the values of individual attri-
    **         butes of the record read from the AFread call.
    **         2). Stuff these values into an inv_t structure.
    **         3). Write the inv_t structure to the VPD using the
    **             vpdadd function call.
    **
    *********************************************************************/

   /*****************
    ** steps 1 and 2
    *****************/

   read_stanza_rec (af, &inv);

   /**
    **  We're searching on loc0 (using the inv2 structure)
    **/

   strcpy (inv2.loc0, inv.loc0);

   /********************************************
    **  step 3).  If the entry does not already
    ** exist, write inv structure to vpd
    ********************************************/

   if ((vpdget (INVENTORY_TABLE, INV_LOC0, &inv2)) != VPD_OK)
   {
      if (inv.loc0[0] != '\0')
      {
         /**
          ** loc0 will never be "" unless there is no vpd info
          ** to add to the vpd.
          **/

         if ((vpdadd (INVENTORY_TABLE, &inv)) != VPD_OK)
         {
            /* failed adding vpd info */

            /*
             * inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ADDVPD_E, 
             *                              CMN_ADDVPD_D), inu_cmdname);
             */
            return (INUVPDADD);
         }
      }
   } /* end if */

   /*********************************************************
    **  If loc0 is not empty -- as in "" --  and if the format
    **  field = INV_FIELD, then we need to redo the HARD and
    **  SOFT links.
    *********************************************************/

   if (inv.loc0[0] != '\0')
      if (inv.format == INV_FILE)
         if ((redo_links (inv.loc0, inv.loc1, inv.loc2)) != INUGOOD)
            return INUSYSFL;            /* system call failed */

   vpd_free_vchars (INVENTORY_TABLE, &inv2);

   return INUGOOD;

} /* end recover_vpd_entry */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    redo_links
 *
 * DESCRIPTION: Ensures that all hard and soft links that should be linked
 *              to the file specified by loc0 do indeed exist.
 *              The parameter loc1 is a comma-separated list of hard links to
 *              loc0. The parameter loc2 is a comma-separated list of soft
 *              links to loc0.
 *
 * PARAMETERS: loc0 -- The file that loc1 and loc2 link to
 *             loc1 -- comma-separated LIST of HARD     links to loc0
 *             loc2 -- comma-separated LIST of SYMBOLIC links to loc0
 *
 * RETURNS:    INUGOOD  --  if successful
 *             INUSYSFL --  system call failed
 *
 * ----------------------------------------------------------------------- */

static int redo_links (char * loc0,  /* The file that loc1 and loc2 link to */
                       char * loc1,  /* comma-separated LIST of HARD links to
                                      * loc0 */
                       char * loc2)  /* comma-separated LIST of SYMBOLIC links
                                      * to loc0 */
{
   char           *lnfile;             /* the file linked to loc0 */
   char           *marker;             /* marks current char within loc1 or
                                        * loc2 */
   char            cmd[PATH_MAX + 1];  /* command string for ln -sf command */
   int             rc;                 /* return code */
   int             eos = 0;            /* end of string marker */
   char            buf[PATH_MAX + 1];  /* buffer used for readlink call              */
   int             bufsize = PATH_MAX; /* size of buf passed to readlink call        */
   int             link_exists = 0;    /* BOOLEAN - indicates if link exists
                                        * or not  */

   /***********************
    **  Process Hard links
    ***********************/

   lnfile = marker = loc1;

   while ((marker != NIL (char))  &&  (*marker != '\0'))
   {
      for (;
           * marker != '\0'  &&  *marker != ',';
           ++marker)
      {
         ;
      } /* end for */

      if (*marker == '\0')      /* then we're at the end of the string */
         eos = 1;
      else
         *marker = '\0';        /* found a comma, so process file name before
                                 * it */

      /**
       ** Make sure that no spaces are in lnfile
       **/

      strip_spaces (lnfile);

      if (access (lnfile, F_OK) != -1)  /* then lnfile exists */
         (void) unlink (lnfile);        /* Remove any existing hard link   */

      (void) link (loc0, lnfile);       /* Create the HARD link     */

      if (eos == 0)     /* then we're not at the end of the string */
         lnfile = ++marker;
   } /* end while */

   /**********************************
    **  Process Soft (symbolic) links
    **********************************/

   eos = 0;     /* Indicate that we're not at the end of the string */
   lnfile = marker = loc2;

   while ((marker != NIL (char))  &&  (*marker != '\0'))
   {
      for (;
           * marker != '\0'  &&  *marker != ',';
           ++marker)
      {
         ;
      } /* end for */

      if (*marker == '\0')      /* then we're at the end of the string */
         eos = 1;
      else
         *marker = '\0';
      /**
       ** Make sure that no spaces are in lnfile
       **/

      strip_spaces (lnfile);

      if ((rc = readlink (lnfile, buf, bufsize)) != -1)
      {
         /* then lnfile to loc0 link exists */

         buf[rc] = '\0';        /* buf returned is NOT null terminated */

         if (strcmp (loc0, buf) != 0)   /* check if links to correct file */
            (void) unlink (lnfile);     /* Remove any incorrect link      */
         else
            link_exists = 1;
      }

      if ( ! link_exists) /* link from lnfile to loc0 does NOT already exist */
      {
         sprintf (cmd, "/bin/ln -sf %s %s > /dev/null 2>&1", loc0, lnfile);

         rc = inu_do_exec (cmd);

         if (rc)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, 
                                 CMN_SYSCOM_D), inu_cmdname, cmd);
            return INUSYSFL;
         }
      } /* of if ( ! link_exists) */

      link_exists = 0;

      if (eos == 0)     /* then we're not at the end of the string */
         lnfile = ++marker;
   } /* end while */

   return INUGOOD;

} /* end redo_links */

/* ----------------------------------------------------------------------- *
 *
 * FUNCTION:    inu_rcvr_or_rm
 *
 * DESCRIPTION: Common routine called by inurecv, and installp.
 *
 *              Basically does either a reject (recover all saved files)
 *              or a commit (delete all saved files) on those files in
 *              the save dir that have their associated option specified
 *              in the optionfile paramter (lpp.options).
 *
 * PARAMETERS:  optionfile  --  file containing options to recover/commit
 *              caller      --  calling routine (inurecv or installp)
 *
 * RETURNS:     INUGOOD  -- if successful
 *              INUNOSAV -- no save dir specified
 *              INUOPEN  -- can't open a file
 *              INUSYSFL -- system call failed rc from process_update_file
 *                          or process_archive_list
 *
 * ----------------------------------------------------------------------- */

int inu_rcvr_or_rm (char * optionfile, 
                    int    caller, 
                    char * lppname)
{
   char           *envar;       /* hold an environment variable */
   char            save_dir[PATH_MAX + 1]; /* full path to the save dir */
   char            tmpstr[PATH_MAX + 1];   /* full path to default save dir */
   int             rc;
   int             rc2, numslots = 0,      /* number of slots in optr array */
                   numread = 0;            /* number of optr slots really
                                            * used */
   char          **optr;                   /* pointer to a calloc'd array of
                                            * memory used to hold option
                                            * names. */
   struct stat     st_buf;                 /* buffer for stat call */
   struct stat     st_buf2;                /* buffer for stat call */
   FILE           *fp;                     /* FILE ptr for optionsfile */
   char            cwd[PATH_MAX + 1];      /* current working dir string */

   /**
    **  Get the current dir, so we can chdir back to it.
    **/

   if ((getcwd (cwd, PATH_MAX)) == NIL (char))
      return INUSYSFL;

   /**
    **  Ensure that optionfile -- if it was specified -- is a valid file.
    **/

   if (optionfile[0] == '\0')
   {
      specified_ops = ALL_OPTIONS;
   }
   else
   {
      /* NO optionfile specified, so assume all options were specified */

      if (stat (optionfile, &st_buf) == -1)  /* optionfile is NOT readable */
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                              CMN_CANT_OPEN_D), inu_cmdname, optionfile);
         return (INUOPEN);
      }
      else
      {
         if (S_ISDIR (st_buf.st_mode))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                 CMN_CANT_OPEN_D), inu_cmdname, optionfile);
            return (INUOPEN);
         }
      } /* end if */
   } /* end if */


   /***************************************
    **  Check if the save directory exists.
    ***************************************/

   if ((envar = getenv (INUSAVEDIR)) == NIL (char))  /* no save dir specified */
   {
      /**
       ** Create the full dir path to the default save dir:
       ** "/usr/lpp/<lppname>/inst_updt.save"
       **/

      strcpy (tmpstr, DFLT_SAVEDIR);    /* "/usr/lpp/" */
      strcat (tmpstr, lppname);
      strcat (tmpstr, "/");
      strcat (tmpstr, "inst_updt.save");
      strcpy (save_dir, tmpstr);
   }
   else
   {
      strcpy (save_dir, envar); /* for call to inu_rm_save_dir */
   }

   if (chdir (save_dir) != 0)   /* then nothing to recover */
      return (INUGOOD);         /* no save_dir is not an error */

   /*
    * If caller is not inurecv and a valid optionfile was specified, then
    * read the optionfile -- into optr, an array of option names.
    */

   if ( ! (caller == CALLER_INURECV  &&  optionfile[0] == '\0'))
   {
      if ((fp = fopen (optionfile, "r")) == NIL (FILE))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                              CMN_CANT_OPEN_D), inu_cmdname, optionfile);
         return INUOPEN;
      }

      optr = read_options_list (&numslots, &numread, fp);

      if (optr == NIL (char *))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return INUSYSFL;
      }
   } /* end if */

   /* Recovers or removes files listed in UPDATE_LIST. */

   if ((rc = process_update_list (caller, optr, numread)) != INUGOOD)
      return rc;

   /* Recovers or removes files listed in ARCHIVE_LIST. */

   if ((rc = process_archive_list (caller, optr, numread)) != INUGOOD)
      return rc;


   if (caller != CALLER_INURECV)
      free_optr_mem (optr, numslots, numread);

   /*
    * If both UPDATE_LIST and ARCHIVE_LIST do not exist, or are empty, call
    * inu_rm_sav_dir.
    */

   rc = stat (UPDATE_LIST, &st_buf);
   rc2 = stat (ARCHIVE_LIST, &st_buf2);

    /*  chdir back to the dir we were called from. */

   if (chdir (cwd) != 0)
   {
      perror (inu_cmdname);
      return INUCHDIR;
   }

   if ((rc == -1  &&  rc2 == -1)  || 
       (rc == -1  &&  st_buf2.st_size == 0)  || 
       (rc2 == -1  &&  st_buf.st_size == 0)  || 
       (st_buf.st_size == 0  &&  st_buf2.st_size == 0))

      inu_rm_save_dir (save_dir);

   return INUGOOD;

} /* end inu_rcvr_or_remove */

/* ----------------------------------------------------------------------
 *
 *  FUNCTION     rm_1_opt
 *
 *  DESCRIPTION  This is ANOTHER hook into the routines in this file.  It is
 *               almost the same as the inu_rcvr_or_rm function.  The  main
 *               differences are that only one option will be specified, and it
 *               will be commited every time.  Also the one option is passed as
 *               a parameter, rather than in a file.   The save directory is
 *               also passed as a parm instead of through an environment
 *               variable.
 *
 *  PARAMETERS   svdir  -- the directory to remove the files that are
 *                         associated with the option parameter from.
 *
 *               option -- the option to be commited.  All files associated
 *                         with this option (in the save dir) will be removed.
 *
 *  RETURNS      None, this function is most likely called from installp
 *               while it is cleaning up.  So, no return code is used.
 *
 * ---------------------------------------------------------------------- */

void rm_1_opt (char * svdir, 
               char * option)
{
   char          **optr;        /* pointer to a calloc'd array of memory used
                                 * to hold option names.   */
   int             rc, rc2;
   struct stat     st_buf;
   struct stat     st_buf2;
   char            cwd[PATH_MAX + 1];  /* current working dir string */

   /**
    **  Get the current dir, so we can chdir back to it.
    **/

   if ((getcwd (cwd, PATH_MAX)) == NIL (char))
      return;

   /**
    **  Do some error checking . . .
    **/

   if (chdir (svdir) != 0)      /* need to chdir back to prev location ??? */
      return;

   if (strlen (option) == 0)
      return;

   if ((optr = (char **) calloc (1, sizeof (char *))) == NIL (char *))
      return;

   /**
    **  Get space for the options array -- even though it will only
    **  have one option in it -- and put the option name in the array.
    **/

   optr[0] = (char *) malloc (strlen (option) + 1);

   if (optr[0] == NIL (char))   /* check if malloc failed */
      return;

   strcpy (optr[0], option);


   /* Recovers or removes files listed in UPDATE_LIST. */

   if ((process_update_list (CALLER_INSTALLP, optr, 1)) != INUGOOD)
      return;

   /* Recovers or removes files listed in ARCHIVE_LIST. */

   if ((process_archive_list (CALLER_INSTALLP, optr, 1)) != INUGOOD)
      return;

   /* Free the allocated memory. */

   free (optr[0]);   /* free the memory occupied by the option name */
   free (optr);      /* free the memory occupied by the ptr address */

   /* If both UPDATE_LIST and ARCHIVE_LIST do not exist, or are empty, call
    * inu_rm_sav_dir */

   rc = stat (UPDATE_LIST, &st_buf);
   rc2 = stat (ARCHIVE_LIST, &st_buf2);

   /* chdir back to the dir we were called from. */

   if (chdir (cwd) != 0)
      return;

   if ((rc == -1  &&  rc2 == -1)  || 
       (rc == -1  &&  st_buf2.st_size == 0)  || 
       (rc2 == -1  &&  st_buf.st_size == 0)  || 
       (st_buf.st_size == 0  &&  st_buf2.st_size == 0))

      inu_rm_save_dir (svdir);

} /* end rm_1_opt */
