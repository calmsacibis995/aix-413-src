static char     sccsid[] = "@(#)60      1.20  src/bos/usr/sbin/install/installp/inuconvert.c, cmdinstl, bos411, 9428A410j 5/25/94 12:57:04";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:  inuconvert
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

/* This function converts an old 3.1 lpp to the new file system.
 *
 * For installs, it sets up the /usr/lpp/<lppname>/inst_root  directory if
 * necessary.
 *
 * For updates, it sets up the /usr/lpp/<lppname>/inst_updt/inst_root
 * directory if necessary. */

/* Syntax:  inuconvert(first, last, expand)
 *
 * Input parameters:
 *
 *      first:   pointer to a structure which specifies the first lpp-option
 *               that must be converted. The caller must put all lpp-option
 *               structures that must be converted in a linked list. The data
 *               type of the lpp-option structure is Option_t, which is
 *               declared in inu_toc.h.
 *
 *      last:    pointer to a structure which specifies the last lpp-option
 *               in the above menntioned linked list that must be converted.
 *
 *      expand:  specifies whether filesystem expansion is permitted.
 *
 * Output parameters:
 *
 *      None.
 *
 * Return values:
 *
 *      Returns FAILURE if something is really hosed and no lpp-options
 *      be converted. Returns SUCCESS if no terminating error was
 *      encountered (some or all of the lpp-options may have failed, so
 *      you must traverse the lpp-option list to determine which ones were
 *      successfully converted).
 *
 * lpp-option structure fields used/modified:
 *
 *      name:  contains the name of the lpp-option (input only).
 *
 *      content: if the lpp-option has a root part that must be installed,
 *               inuconvert will set this field to CONTENT_BOTH. Otherwise,
 *               this field will not be modified (output only).
 *
 *      Status:  inuconvert sets this to STAT_SUCCESS if it successfully
 *               converts the lpp-option. Otherwise, it is set to
 *               STAT_FAILURE_INUCONVERT.
 *
 *
 *      level.ver: contains the version of the lpp-option (input only)
 *
 *      level.rel: contains the release of the lpp-option (input only)
 *
 *      level.mod: contains the modification level of the lpp-option (input
 *      only)
 *
 *      level.fix: contains the fix level of the lpp-option (input only) */

/* include files */

#include <sys/stat.h>
#include <sys/mode.h>
#include <sys/access.h>
#include <installp.h>
#include <inuconvert.h>

/* Constants */

#define MAX_RECORD_SIZE         4096    /* Maximum number of bytes in a
                                         * stanza of an inventory file.
                                         * This is a parameter to AFopen.
                                         * THIS LIMIT SHOULD APPEAR IN THE
                                         * AIX 3.2 PUBLICATIONS. */

#define MAX_NUM_RECORD_FIELDS   30      /* Maximum number of attributes
                                         * per stanza in an inventory file.
                                         * Actual number is 13 (see Install
                                         * PDS). I gave myself some slack.
                                         * This is a parameter to AFopen. */

#define USR_INV_FILE            0       /* constant identifying a  /usr
                                         * filesystem inventory file. */

#define ROOT_INV_FILE           1       /* constant identifying a root
                                         * filesystem inventory file. */

#define IS_A_FILE               0       /* constant for a file */

#define IS_A_DIRECTORY          1       /* constant for a directory */

#define ZERO                    '\0'    /* character with value zero */


/* Errors returned by the moveit function (see below) */

#define MOVEIT_DEST_PATHNAME_FORMAT     -2      /* Destination pathname does
                                                 * not contain a "/". */

#define MOVEIT_CREATE_DIRECTORY         -3      /* Can't create a directory
                                                 * in the destination file's
                                                 * pathname. */

#define MOVEIT_COPY_TO_TEMP             -4      /* Could not copy the source
                                                 * to a temporary file. */

#define MOVEIT_UNLINK_FROM_SOURCE       -5      /* Couldn't unlink from the
                                                 * source file. */

#define MOVEIT_LINK_TEMP_WITH_DEST      -6      /* Couldn't link the
                                                 * destination file with the
                                                 * temporary file. */

#define MOVEIT_UNLINK_TEMP              -7      /* Couldn't unlink the
                                                 * temporary file. */


/* global variables (to the procedures in this file) */

static uchar    expand_ok;                 /* Indicates whether its ok to
                                            * expand filesystems. */
static uchar    update;                    /* Indicates whether this operation
                                            * is an update or an install. */
static char     temp_lpp_option_name[256]; /* Buffer used to parse the lpp.
                                            * name from an lpp-option name.
                                            * ASSUMES maximum lppname of 256
                                            * characters. */
static uchar    usr_inv_file_exists;       /* Flag indicating whether the /usr
                                            * inventory file for a particular
                                            * lpp-option exists. */
static uchar    root_inv_file_exists;      /* Flag indicating whether the root
                                            * inventory file for a particular
                                            * lpp-option exists. */
static char     ver[] = {ZERO,             /* String representation of */
                         ZERO,             /* an lpp-option's version */
                         ZERO};            /* number. */
static char     rel[] = {ZERO,             /* String representation of */
                         ZERO,             /* an lpp-option's release */
                         ZERO};            /* number. */
static char     mod[] = {ZERO,             /* String representation of an */
                         ZERO,             /* lpp-option's modification */
                         ZERO,             /* number. */
                         ZERO,
                         ZERO};
static char     fix[] = {ZERO,             /* String representation of an */
                         ZERO,             /* lpp-option's fix number. */
                         ZERO,
                         ZERO,
                         ZERO};


/* internal function prototypes */

static int conv_inv (int        inv_file_type,
                     char     * lppname,
                     char     * lppopt_name,
                     Option_t * lppopt);

static int conv_stanza (AFILE_t   af,
                        char    * lppname,
                        int       inv_file_type,
                        char    * inv_file);

static int mark_lppopt_broken (Option_t * lppopt);

static int mark_all_lppopt_broken (Option_t * first,
                                   Option_t * last);

static int del_links (char * links,
                      char * inv_file,
                      char * stanza);

static int moveit (char * from,
                   char * to);

static int find_liblpp (char * liblpp,
                        char * lppname);

static int find_conv_script (char * conv_script,
                             char * lppname,
                             char * lppopt_name);

static int has_root_part (Option_t * lppopt);


int inuconvert (Option_t      * first,
                Option_t      * last,
                unsigned char   expand)
{

   int            rc;                  /* Return code */
   uchar          lpp_option_error;    /* Indicates whether an error was
                                        * detected while processing an
                                        * an lpp option. */
   uchar          success;             /* Indicates whether inuconvert
                                        * was successfull. */
   Option_t      *lpp_opt;             /* Pointer to an lpp-option
                                        * structure. */
   Option_t      *op;                  /* Pointer to an lpp-option
                                        * structure. */
   char          *lppopt_name;         /* Points at current lpp-option name. */
   char          *lppname;             /* Points at the current lpp name. */
   int            i;                   /* Loop counter used to process the
                                        * two inventory files in turn. */
   char           old_dir[PATH_MAX + 1]; /* Buffer used to store the
                                          * current directory. */
   char          *conv_script;         /* Pointer to a string containing the
                                        * pathname of the conversion script. */
   char          *sysck;               /* Pointer to a string containing
                                        * the sysck command. */
   char          *inst_root;           /* Pointer to a string containing.
                                        * the pathname of the inst_root
                                        * directory for the current
                                        * lpp-option. */
   char          *liblpp;              /* Pointer to a string containing the
                                        * pathname of liblpp.a for the lpp. */
   char          *ar;                  /* Pointer to a string containing
                                        * the ar command. */
   char           buf1[512];           /* Buffer used to construct commands
                                        * and strings of various sorts. */
   char           buf2[512];           /* Buffer used to construct commands
                                        * and strings of various sorts. */
   char           buf3[512];           /* Buffer used to construct commands
                                        * and strings of various sorts. */
   char           source[256];         /* Source file name. */
   char           destination[256];    /* Destination file name. */
   Option_t      *one_past_last;       /* Pointer to the Option_t structure
                                        * after the one pointed at by last. */
   char          *inv_file;            /* String containing the name of a
                                        * usr inventory file. */
   DIR           *dirp;                /* Pointer to a directory stream. */
   struct dirent *dp;                  /* Pointer to a directory entry. */
   uchar          found;               /* Flag indicating whether a valid
                                         * apply list file was found. */

   /* Initialize some global variables */

   expand_ok = expand;
   success = TRUE;

   /* NOTE: I am relying on installp to properly initialize the following
    *       environment variables:
    *
    *           INUEXPAND              INULIBDIR              INUSAVE
    *
    *       These environment variables are used by the function
    *       inu_file_eval_size to determine whether a filesystem is
    *       expandable. This function is in inulib.
    *
    */

   /* Open the /usr VPD and set up path to root VPD */

   if ((rc = vpdlocalpath (NULL)) != VPD_OK)
   {
      /* Error setting local VPD database directory */

      instl_msg (SCREEN_LOG, MSGSTR(MS_INUCONVERT, ICV_VPD_PATH_E, ICV_VPD_PATH_D),
               inu_cmdname, rc);
      return (FAILURE);
   }

   if ((rc = vpdremotepath (NULL)) != VPD_OK)
   {
      /* error setting path to remote VPD database directory */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   }


   if ((rc = vpdlocal ()) != VPD_OK)
   {
      /* error switching to the local VPD database directory */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   }

   /* Get lppname from the lpp-option name.
    *
    * The lppname is the same for all lpp-options. */

   inu_get_prodname (first, temp_lpp_option_name);
   lppname = temp_lpp_option_name;

   /* For each lpp-option that must be converted */

   one_past_last = last -> next;        /* need this so all elements get
                                         * processed */

   for (lpp_opt = first; lpp_opt != one_past_last; lpp_opt = lpp_opt -> next)

   {
      /* Initialize some variables */

      lpp_option_error = FALSE;
      usr_inv_file_exists = FALSE;
      root_inv_file_exists = FALSE;
      success = TRUE;

      /* update == 1 if this is an update; 0 if its and install */

      update = IF_UPDATE (lpp_opt -> op_type);

      /* convert this lpp-option's version, release, modifacation and fix
       * number's to strings */

      convert_vrmf_to_string (lpp_opt);

      /* Continue with the next lpp-option if the status of this one is
       * not STAT_SUCCESS */

      if ((lpp_opt -> Status) != STAT_SUCCESS)
      {
         continue;
      }

      /* Get the lpp-option name */

      lppopt_name = lpp_opt -> name;

      /* for each inventory file:
       *
       *     /usr inventory file has i = 0
       *     root inventory file has i = 1 */

      for (i = USR_INV_FILE;
           i <= ROOT_INV_FILE;
           i++)
      {
         /* Call conv_inv to install the files in the inventory file.  */

         if ((rc = conv_inv (i, lppname, lppopt_name, lpp_opt)) != SUCCESS)
         {
            /* Terminating error for this lpp-option */

            lpp_option_error = TRUE;
            goto LPP_OPTION_ERROR;
         }

      } /* end for each inventory file */

      /* If a shell conversion script for the /usr filesystem exists in the
       * /usr/lpp/lppname/inst_convert directory, run it (using the "system"
       * function). If the script's return code indicates that an error
       * occured, then a terminating error for this lpp-option has occured
       * ==> continue with the next lpp-option.  */

      conv_script = buf1;

      if ((rc = find_conv_script (conv_script, lppname, lppopt_name)) ==
                                                                       SUCCESS)
      {
         /* The conversion script exists, run it */

         /* Set signals before calling the script */

         inu_init_sigs();

         if ((rc = inu_do_exec (conv_script)) != SUCCESS)
         {
            /* An error occured while running the conversion shell script */

            /* restore signals */

            inu_init_sigs ();

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_CONV_E, ICV_CONV_D),
                     inu_cmdname, conv_script, rc);
            lpp_option_error = TRUE;
            goto LPP_OPTION_ERROR;
         } /* end if */

         /* restore signals */

         inu_init_sigs ();
      } /* end if */


      /* If the /usr inventory file exists run sysck to create any required
       * links/symlinks to the newfiles. sysck will also update the VPD. If
       * sysck returns an error, its a terminating condition for this
       * lpp-option. */

      inv_file = buf1;

      if (find_inv_file (inv_file, lppname, lppopt_name, USR_INV_FILE) ==
                                                                       SUCCESS)
      {
         sysck = buf2;

         /* run sysck */

         strcpy (sysck, "/bin/sysck -O u -i -f ");
         strcat (sysck, inv_file);
         strcat (sysck, " ");
         strcat (sysck, lppopt_name);

         /* Set signals before calling sysck */

         inu_init_sigs ();

         if ((rc = inu_do_exec (sysck)) != SUCCESS)
         {
            /* An error occured while running sysck */

            /* restore signals */

            inu_init_sigs ();
            lpp_option_error = TRUE;
            goto LPP_OPTION_ERROR;
         } /* end if */

         /* restore signals */

         inu_init_sigs ();
      } /* end if */


      /* Common error processing for an lpp-option */

LPP_OPTION_ERROR:
      if (lpp_option_error)
      {
         lpp_opt -> Status = STAT_FAILURE_INUCONVERT;

         /* Mark the LPP broken in the /usr VPD, set the Status field of
          * the lpp-option structure to STAT_FAILURE_INUCONVERT and process
          * the next lpp-option. */

         /* Ignore all signals while updating the VPD */

         inu_ignore_all_sigs ();

         if ((rc = mark_lppopt_broken (lpp_opt)) == FAILURE)
         {
            /* VPD database problem */

            /* restore signals */

            inu_init_sigs ();

            return (FAILURE);
         } /* end if */

         /* restore signals */

         inu_init_sigs ();
      } /* end if */


   } /* end for each lpp-option */


   /* If the file /usr/lpp/lppname/inst_convert/liblpp.a exists */

   liblpp = buf1;

   if ((rc = find_liblpp (liblpp, lppname)) == SUCCESS)

   {
      /* Save the current directory */

      if ((getwd (old_dir)) == NULL)
      {
         /* can't get current directory */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_GETWD_E, ICV_GETWD_D),
                  inu_cmdname);
         success = FALSE;
         goto FAIL;
      } /* end if */

      /*
       *
       * Installs are handled a little differently than updates.
       *
       * For INSTALLS: if the /usr/lpp/lppname/inst_root directory does
       *               not exist, create it.
       *
       * For UPDATES:  if the /usr/lpp/lppname/inst_updt/inst_root directory
       *               does not exit, create it */

      inst_root = buf2;

      strcpy (inst_root, "/usr/lpp/");
      strcat (inst_root, lppname);

      if (update)
         strcat (inst_root, "/inst_updt");

      strcat (inst_root, "/inst_root");

      if ((rc = inu_mk_dir (inst_root)) != SUCCESS)
      {
         /* can't create the inst_root directory  */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_NO_MK_DIR_E, CMN_NO_MK_DIR_D),
                  inu_cmdname, inst_root);
         success = FALSE;
         goto FAIL;
      } /* end if */

      /* Change to the inst_root directory */

      if ((rc = chdir (inst_root)) != SUCCESS)
      {
         /* can't change to the inst_root directory */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_CANT_CHDIR_E, CMN_CANT_CHDIR_D),
                  inu_cmdname, inst_root);
         success = FALSE;
         goto FAIL;
      } /* end if */


      /* Unarchive liblpp.a. If the archive fails due to lack of space in
       * the filesystem and filesystem expansion is allowed, then expand the
       * filesystem by some TBD amount and try again. What are the valid
       * return codes from ar? My documentation doesn't specify them. */

      ar = buf3;

      strcpy (ar, "/usr/ccs/bin/ar x ");
      strcat (ar, liblpp);
      strcat (ar, " >/dev/null 2>/dev/null");

      /* Set signals before calling ar */

      inu_init_sigs ();

      if ((rc = inu_do_exec (ar)) != SUCCESS)
      {
         /* An error occured while running ar.
          *
          * If the filesystem is expandable, expand it by the size of the
          * liblpp.a. */

         /* restore signals */

         inu_init_sigs ();

         if (expand_ok)
         {
            if ((rc = inu_file_eval_size (inst_root, liblpp, TRUE)) != SUCCESS)
            {
               /* can't expand the filesystem */

               if (rc != INUFS)
               {
                  instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_UNARC_E, ICV_UNARC_D),
                           inu_cmdname, liblpp, rc);
               } /* end if */
               success = FALSE;
               rc = chdir (old_dir);
               goto FAIL;

            } /* end if inu_file_eval_size != SUCCESS */


            /* Successfully attempted to expand the file system.  Try
             * running ar again. */

            /* Set signals before calling ar */

            inu_init_sigs ();

            if ((rc = inu_do_exec (ar)) != SUCCESS)
            {
               /* An error occured while running ar */

               /* restore signals */

               inu_init_sigs ();

               instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_UNARC_E, ICV_UNARC_D),
                        inu_cmdname, liblpp, rc);
               success = FALSE;
               rc = chdir (old_dir);
               goto FAIL;
            } /* end if */

         } /* end if expand_ok */
         else /* ar failed and its not ok to expand the filesystem */
         {
            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_UNARC_E, ICV_UNARC_D),
                     inu_cmdname, liblpp, rc);
            success = FALSE;
            rc = chdir (old_dir);
            goto FAIL;
         } /* end if */

      } /* end if inu_do_exec (ar) != SUCCESS */

      /* The ar was successful.
       *
       * Restore signals */

      inu_init_sigs ();

      /* If this is an update operation, we've got some special processing
       * processing to do to help keep things simple for our compiler
       * brethren in Toronto. */

      if (update)
      {
         /* For each lpp-option do the following:
          *
          * Read the directory to find "[<lpp-option>.]al_vv.rr.mmmm.ffff"
          * files. If one is found and it's a valid apply list and we have
          * no more work to do. Otherwise, we have to create one by copying
          * the <lpp-option>.al file to al_vv.rr.mmmm.ffff. THIS IS A
          * KLUDGE, but it should work for the compilers.
          *
          * Note that inu_valid_al (defined in inu_apply_opts.c) determines
          * if a file is a valid apply list file. */

         for (op = first;
              op != one_past_last;
              op = op -> next)
         {
            dirp = opendir (".");

            found = FALSE;
            while ((dp = readdir (dirp)) != (struct dirent *) NULL)
            {
               if (inu_valid_al (op, dp -> d_name))
               {
                  found = TRUE;
                  break;
               } /* end if */
            } /* end while more files in the directory */

            closedir (dirp);

            if (!found)
            {
               /* Must create a valid al (apply list) file.  Copy
                * "<lpp-option>.al" to "al_vv.rr.mmmm.ffff". */

               strcpy (source, op -> name);
               strcat (source, ".al");

               strcpy (destination, op -> name);
               strcat (destination, ".al_");
               strcat (destination, ver);
               strcat (destination, ".");
               strcat (destination, rel);
               strcat (destination, ".");
               strcat (destination, mod);
               strcat (destination, ".");
               strcat (destination, fix);

               if (access (source, E_ACC) == 0)
               {
                  (void) inu_mv (source, destination);
               } /* end if  <lpp-option>.al exists */

            } /* end if valid apply list file not found */

         } /* end for every lpp-option */

      } /* end if update */


      /* Restore the current directory. */

      if ((rc = chdir (old_dir)) != SUCCESS)
      {
         /* can't change to the old_dir directory */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_RETURN_DIR_E, ICV_RETURN_DIR_D),
                  inu_cmdname, old_dir, rc);
         success = FALSE;
         goto FAIL;
      } /* end if */
   } /* end if */
   else /* liblpp.a doesn't exist */
   {
      if (root_inv_file_exists)
      {
         /* A root inventory file exists for one or more lpp-options but we
          * don't have liblpp.a. */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MISS_FILE_E, ICV_MISS_FILE_D),
                  inu_cmdname, lppname, liblpp);
         success = FALSE;
         goto FAIL;
      } /* end if */
   } /* end if */


FAIL:


   if (success)
      return (SUCCESS);
   else
   {
      /* Ignore signals while updating the VPD */

      inu_ignore_all_sigs ();

      rc = mark_all_lppopt_broken (first, one_past_last);

      /* restore signals */

      inu_init_sigs ();

      return (FAILURE);
   } /* end if */


} /* end of inuconvert */


/*****************************************************************************
** Function: conv_inv
**
** Purpose: This function installs the files in the specified inventory file.
*/

static int conv_inv (int        inv_file_type,
                     char     * lppname,
                     char     * lppopt_name,
                     Option_t * lppopt)
{

   char            inv_file[512];       /* Buffer for the pathname of the
                                         * inventory file   */
   AFILE_t         af;                  /* Stanza file descriptor (returned
                                         * from AFopen) */
   int             rc;                  /* return code */


   /* Open the inventory file using AFopen. There are only two possible
    * inventory files /usr:
    *          (/usr/lpp/<lppname>/inst_convert/<lppopt_name>.inv.u)
    * and root:
    *          (/usr/lpp/<lppname>/inst_convert/<lppopt_name>.inv.r). */

   if ((rc = find_inv_file (inv_file, lppname, lppopt_name, inv_file_type)) !=
                                                                       SUCCESS)
   {
      /* inventory file doesn't exist (not an error) */

      return (SUCCESS);
   }

   if ((af = AFopen (inv_file, MAX_RECORD_SIZE, MAX_NUM_RECORD_FIELDS)) ==
                                                                          NULL)
   {
      /* inventory file exists but it can't be opened */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_CANT_OPEN_E, CMN_CANT_OPEN_D),
               inu_cmdname, inv_file);
      return (FAILURE);
   } /* end if */


   /* For each stanza in the inventory file (Each inventory file is read
    * sequentially using AFread and AFgetattr. The stanza name is the name
    * of the newfile) */

   while (AFread (af) != NULL)
   {
      /* Call conv_stanza to install the next newfile in the inventory
       * file. Note that af -> AF_cbuf is the name of the current stanza
       * (the name of the newfile). */

      if ((rc = conv_stanza (af, lppname, inv_file_type, inv_file)) != SUCCESS)
      {
         /* error processing a stanza */
         AFclose (af);
         return (FAILURE);
      } /* end if */
   } /* end while each stanza in the inventory file */

   /* close the inventory file */

   AFclose (af);

   if (inv_file_type == USR_INV_FILE)
   {
      /* The /usr inv file exists and was successfully processed. */
      /* Set usr_inv_file_exists to TRUE so sysck will be run.    */

      usr_inv_file_exists = TRUE;
   }
   else /* we've just successfully processed a root inventory file */
   {
      /* Call has_root_part to update the LPP and PRODUCT databases. */
      /* Update the "content" field of the lpp-option structure to   */
      /* indicate that this lpp has both a usr and root part.        */

      /* ignore signals while updating the VPD */

      inu_ignore_all_sigs ();

      if (has_root_part (lppopt) == SUCCESS)
      {
         /* restore signals */

         inu_init_sigs ();

         root_inv_file_exists = TRUE;
         lppopt -> content = CONTENT_BOTH;
      }
      else /* could not update the LPP and PROCUCT SWVPD databases */
      {
         /* restore signals */
         inu_init_sigs ();

         return (FALSE);
      } /* end if */
   } /* end if */

   return (SUCCESS);

} /* end conv_inv */


/*****************************************************************************
** Function: conv_stanza
**
** Purpose: This funtion installs the the next file in the specified inventory
**          file. The inventory file is read sequentially.
*/

static int conv_stanza (AFILE_t   af,
                        char    * lppname,
                        int       inv_file_type,
                        char    * inv_file)
{

   char         *newfile;             /* Stanza name (indicates where the file
                                       * should go in the AIX v3.2 file
                                       * tree). */
   char          dest[PATH_MAX + 1];  /* Destination of "oldfile" (defined
                                       * below). This value is different from
                                       * "newfile" when "oldfile"
                                       * must be moved to the the root
                                       * filesystem. */
   char         *type;                /* The value of the "type" attribute of
                                       * this stanza. */
   char         *oldfile;             /* The value of the "oldpath" attribute
                                       * of this stanza. inuconvert moves the
                                       * file from "oldfile" to "dest". */
   inv_t         inventory_row;       /* Buffer used to read a row from the
                                       * INVENTORY VPD.  */
   int           rc;                  /* Return code. */
   uchar         file_type;           /* Specifies the type of a file:
                                       * IS_A_FILE or IS_A_DIRECTORY are the
                                       * two possible values. */
   struct stat   statbuf;             /* Buffer to hold information returned by
                                       * "stat". */

   /* compute the name of the newfile */

   newfile = af -> AF_cbuf;

   if (*newfile != '/')
   {
      /* the stanza name does not contain an absolute pathname */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_STANZA_PATH_E, ICV_STANZA_PATH_D),
               inu_cmdname, newfile, inv_file);
      return (FAILURE);
   } /* end if */


   /* call AFgetattr to determine whether the stanza is for a FILE or
    * DIRECTORY. */

   if ((type = AFgetatr (af -> AF_catr, "type")) == NULL)
   {
      /* An error occured while reading the type attribute */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_TYPE_E, ICV_TYPE_D), inu_cmdname,
               inv_file, newfile);
      return (FAILURE);
   } /* end if */

   /* Determine whether this is a file or directory */

   file_type = IS_A_FILE;

   if (strcmp (type, "DIRECTORY") == 0)
   {
      file_type = IS_A_DIRECTORY;
   }
   else
   {
      /* If type is not equal to FILE then the inventory file is not
       * properly formatted. */

      if (strcmp (type, "FILE") != 0)
      {
         /* its not a FILE or DIRECTORY */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_STANZA_TYPE_E, ICV_STANZA_TYPE_D),
                  inu_cmdname, type, inv_file, newfile);
         return (FAILURE);
      } /* end if */
   } /* end if */

   /* Call AFgetattr to read the oldfile name. */

   if ((oldfile = AFgetatr (af -> AF_catr, "oldpath")) == NULL)
   {
      /* An error occured while reading the oldpath attribute */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_OLDPATH_E, ICV_OLDPATH_D),
               inu_cmdname, inv_file, newfile);
      return (FAILURE);
   } /* end if */

   if (*oldfile != '/')
   {
      /* the oldpath  does not contain an absolute pathname */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_STANZA_OLDPATH_E,
               ICV_STANZA_OLDPATH_D), inu_cmdname, oldfile, inv_file, newfile);
      return (FAILURE);
   }

   /* Get "oldfile's" inventory VPD data.
    *
    * Note that if the VPD data doesn't exist, I assume * that I deleted it in
    * a previous attempt at * running inuconvert. */

   strcpy (inventory_row.loc0, oldfile);

   if ((rc = vpdget (INVENTORY_TABLE, INV_LOC0, &inventory_row)) != VPD_OK)
   {
      if (rc != VPD_NOMATCH)
      {
         /* Can't get a row from the VPD's INVENTORY database because
          * of some system problem. */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_NOINFO_SWVPD_E,
                  ICV_NOINFO_SWVPD_D), inu_cmdname, oldfile, rc);
         return (FAILURE);
      } /* end if */
   }
   else /* we got the row from the inventory database */
   {
      /* Delete all links and symlinks to oldfile.  This infomation is
       * stored in the file's inventory VPD. */

      if (del_links (inventory_row.loc1, inv_file, newfile) == FAILURE)
      {
         /* couldn't delete the hard links */

         return (FAILURE);
      } /* end if */

      if (del_links (inventory_row.loc2, inv_file, newfile) == FAILURE)
      {
         /* couldn't delete the symbolic links */

         return (FAILURE);
      } /* end if */

      /* Delete oldfile's VPD data. */

      if ((rc = vpddel (INVENTORY_TABLE, INV_LOC0, &inventory_row)) != VPD_OK)
      {
         /* can't delete oldfile's entry from the inventory VPD */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_NODEL_SWVPD_E, ICV_NODEL_SWVPD_D),
                  inu_cmdname, oldfile, rc);
         return (FAILURE);
      } /* end if */

      vpd_free_vchars (INVENTORY_TABLE, &inventory_row);
   } /* end if */

   /* If newfile is part of the /usr filesystem then oldfile must be moved
    * to newfile. If newfile is part of the root filesystem then oldfile
    * must be moved to /usr/lpp/<lppname>/inst_root/newfile for installs.
    * For updates, the oldfile must be moved to
    * /usr/lpp/<lppname>/inst_updt/inst_root/newfile. */

   if (inv_file_type == USR_INV_FILE)
   {
      /* newfile is part of the /usr filesystem */

      strcpy (dest, newfile);
   }
   else
   {
      /* newfile is part of the root filesytem.
       *
       * NOTE: newfile is assumed to contain an absolute pathname (begins
       * with a "/"). */

      strcpy (dest, "/usr/lpp/");
      strcat (dest, lppname);

      if (update)
         strcat (dest, "/inst_updt");

      strcat (dest, "/inst_root");
      strcat (dest, newfile);
   } /* end if */

   /* If oldfile and newfile don't exist, return an error. If the oldfile
    * doesn't exist and * the newfile exists, return success. */

   if ((rc = access (oldfile, E_ACC)) != SUCCESS)
   {
      /* the oldfile doesn't exist  */

      if ((rc = access (dest, E_ACC)) != SUCCESS)
      {
         /* neither the oldfile nor dest exist */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_NOFILE_INVEN_E,
                  ICV_NOFILE_INVEN_D), inu_cmdname, dest, oldfile, inv_file);
         return (FAILURE);
      } /* end if */

      /* The oldfile doesn't exist but the newfile does. */

      return (SUCCESS);
   } /* end if */

   /* The oldfile exists (the dest file may or may not exist). */

   if ((rc = stat (oldfile, &statbuf)) == SUCCESS)
   {
      /* Check that the actual filetype of oldfile agrees with what's
       * specified in the inventory file */

      if ((file_type == IS_A_DIRECTORY) && !S_ISDIR (statbuf.st_mode))
      { /* file types don't agree */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_OLDP_DIR_E, ICV_OLDP_DIR_D),
                  inu_cmdname, newfile, oldfile, inv_file);
         return (FAILURE);
      } /* end if */

      if ((file_type == IS_A_FILE) && !S_ISREG (statbuf.st_mode))
      { /* file types don't agree */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_OLDP_FIL_E, ICV_OLDP_FIL_D),
                  inu_cmdname, newfile, oldfile, inv_file);
         return (FAILURE);
      } /* end if */
   }
   else /* cannot "stat" oldfile */
   {
      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_OLDP_NOINF_E, ICV_OLDP_NOINF_D),
               inu_cmdname, oldfile, inv_file, newfile);
      return (FAILURE);
   } /* end if */

   if (file_type == IS_A_FILE)
   {
      /* Move the oldfile to the destination. */

      if ((rc = moveit (oldfile, dest)) != SUCCESS)
      {
         /* The file can't be moved.
          *
          * If the filesystem is expandable, expand it * by the size of the
          * file. Otherwise this is a * terminating error for this
          * lpp-option. */

         if (expand_ok)
         {
            if ((rc = inu_file_eval_size (dest, oldfile, TRUE)) != SUCCESS)
            {
               /* can't expand the filesystem */

               /* Should I have an error message here? inu_file_eval_size
                * prints out its own. */

               instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MV_NOEXP_E, ICV_MV_NOEXP_D),
                        inu_cmdname, oldfile, dest, rc, inv_file);
               return (FAILURE);
            } /* end if */

            /* Move the oldfile to newfile.  If the file still can't be moved
             * you can't say I didn't try => terminating error for this
             * lpp-option. */

            if ((rc = moveit (oldfile, dest)) != SUCCESS)
            {

               /* The file still can't be moved. */

               instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MV_FAIL_E, ICV_MV_FAIL_D),
                        inu_cmdname, oldfile, dest, rc, inv_file);
               return (FAILURE);
            } /* end if */
         }
         else   /* moveit failed and we are not allowed to even attempt to
                 * expand the filesystem */
         {
            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MV_FAIL_E, ICV_MV_FAIL_D),
                     inu_cmdname, oldfile, dest, rc, inv_file);
            return (FAILURE);
         } /* end if */
      } /* end if first moveit() != SUCCESS */
   }
   else /* oldfile is a directory */
   {
      if (inu_mk_dir (dest) != SUCCESS)
      {
         /* can't create the directory */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MKDIR_E, ICV_MKDIR_D),
                  newfile, inu_cmdname, inv_file);
         return (FAILURE);
      }
      else  /* successfully made the new directory */
      {
         if (rmdir (oldfile) != SUCCESS)
         {
            /* could not remove the old directory */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_RMDIR_E, ICV_RMDIR_D),
                     inu_cmdname, oldfile, inv_file, newfile);
            return (FAILURE);
         } /* end if */
      } /* end if */
   } /* end if */

   return (SUCCESS);

   /* inuconvert will subsequently run syschk (using the /usr inventory
    * file as input, if it exists) to create any new links/symlinks required
    * by this newfile. syschk will also create newfile's VPD data.
    *
    * inuconvert does not create any links/symlinks any newfiles that will
    * ultimately be part of the root filesystem (i.e., files moved to
    * /usr/lpp/lppname/inst_root). It also will not create * any VPD entries
    * for these files. This work must be done during the installation of
    * the root. */

} /* end of conv_stanza */

/*****************************************************************************
** Function: mark_lppopt_broken
**
** Purpose: This function marks the specified lpp-option broken in the VPD's
**          LPP and PRODUCT databases. Note that its ok if the PRODUCT
**          database entries do not exist.
*/

static int mark_lppopt_broken (Option_t * lppopt)
{
   lpp_t           lpp_row;     /* Buffer used to read a row from the LPP
                                   VPD. */
   prod_t          prod_row;    /* Buffer used to read a row from the PRODUCT
                                 * VPD  */
   int             rc;          /* Return code */
   int             i;           /* Loop counter  */
   uchar           lpp_entry;   /* Flag indicating whether there is a LPP
                                 * entry for the lpp-option in the SWVPD. */
   uchar           prod_entry;  /* Flag indicating whether there is a product
                                 * entry for the lpp-option in the SWVPD. */

   /* Update both the /usr and root LPP databases.
    *
    * Update /usr when i=1;
    *
    * update root when i=2. */

   lpp_entry = TRUE;
   prod_entry = TRUE;

   for (i = 1; i <= 2; i++)
   {
      strcpy (lpp_row.name, lppopt -> name);
      if ((rc = vpdget (LPP_TABLE, LPP_NAME, &lpp_row)) != VPD_OK)
      {
         if ((rc != VPD_NOMATCH) || (i == 1))
         {
            /* can't get a row from the VPD's LPP database */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MRKBRK_READ_E,
                     ICV_MRKBRK_READ_D), inu_cmdname, lppopt -> name, rc);
            return (FAILURE);
         }
         else
         {
            lpp_entry = FALSE;
         } /* end if */
      } /* end if */

      if (lpp_entry)
      {
         lpp_row.state = ST_BROKEN;

         if ((rc = vpdchgget (LPP_TABLE, &lpp_row)) != VPD_OK)
         {
            /* can't update a row in the VPD's LPP database */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MRKBRK_UPDT_E,
                     ICV_MRKBRK_UPDT_D), inu_cmdname, lppopt -> name, rc);
            return (FAILURE);
         } /* end if */

         vpd_free_vchars (LPP_TABLE, &lpp_row);
      } /* end if */

      /* now update the PRODUCT database */

      strcpy (prod_row.lpp_name, lppopt -> name);
      prod_row.ver = lppopt -> level.ver;
      prod_row.rel = lppopt -> level.rel;
      prod_row.mod = lppopt -> level.mod;
      prod_row.fix = lppopt -> level.fix;
      strcpy (prod_row.ptf, lppopt -> level.ptf);

      if ((rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                        PROD_MOD | PROD_FIX | PROD_PTF),
                        &prod_row)) != VPD_OK)
      {
         if ((rc != VPD_NOMATCH) || (i == 1))
         {
            /* can't get a row from the VPD's PRODUCT database */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MRKBRK_READ_E,
                     ICV_MRKBRK_READ_D), inu_cmdname, lppopt -> name, rc);
            return (FAILURE);
         }
         else
         {
            prod_entry = FALSE;
         } /* end if */
      } /* end if */

      if (prod_entry)
      {
         prod_row.state = ST_BROKEN;

         if ((rc = vpdchgget (PRODUCT_TABLE, &prod_row)) != VPD_OK)
         {
            /* can't update a row in the VPD's PRODUCT database */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_MRKBRK_UPDT_E,
                     ICV_MRKBRK_UPDT_D), inu_cmdname, lppopt -> name, rc);
            return (FAILURE);
         } /* end if */

         vpd_free_vchars (PRODUCT_TABLE, &prod_row);
      } /* end if */

      /* If we've just updated the /usr database switch to the root
       * database. * Otherwise, switch back to the /usr database. */

      if (i == 1)
      { /* just updated the /usr database */

         if ((rc = vpdremote ()) != VPD_OK)
         {
            /* can't switch to the root VPD database */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                     inu_cmdname);
            return (FAILURE);
         } /* end if */
      }
      else /* switch back to the /usr database */
      {
         if ((rc = vpdlocal ()) != VPD_OK)
         {
            /* can't switch to the usr VPD database */

            instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                     inu_cmdname);
            return (FAILURE);
         } /* end if */
      } /* end if */
   } /* end for ... */

   return (SUCCESS);

} /* end mark_lppopt_broken */


/*****************************************************************************
** Function: mark_all_lppopt_broken
**
** Purpose: This function marks all the specified lpp-options broken in the
**          VPD's LPP database.
*/

static int mark_all_lppopt_broken (Option_t * first,
                                   Option_t * last)
{

   Option_t       *lpp_opt;     /* Pointer used to traverse the list of
                                 * lpp-options. */

   /* For each lpp-option (note that the option pointed at by last is not
    * processed) */

   for (lpp_opt = first;
        lpp_opt != last;
        lpp_opt = lpp_opt -> next)
   {
      /* Continue with the next lpp-option if the status of this one is
       * not STAT_SUCCESS */

      if ((lpp_opt -> Status) != STAT_SUCCESS)
      {
         continue;
      } /* end if */

      /* Mark it broken */

      if (mark_lppopt_broken (lpp_opt) == FAILURE)
      {
         /* Can't update VPD database */

         return (FAILURE);
      } /* end if */
   } /* end for all lpp-options */

   return (SUCCESS);

} /* end mark_all_lppopt_broken */


/*****************************************************************************
** Function: del_links
**
** Purpose: Delete all links in a comma separated list of link names.
**
*/

static int del_links (char * links,
                      char * inv_file,
                      char * stanza)
{

   char           *cur_link;    /* Pointer to the link to be deleted. */
   char           *next_link;   /* Pointer to the next link to be deleted. */
   int             rc;          /* return code */


   /* Might get a storage protection error here. I'm modifying a string
    * returned by the SWVPD. */

   cur_link = links;

   /* check for a pointer to a null string */

   if ((*cur_link) == ZERO)
      return (SUCCESS);

   while (cur_link != NULL)
   {
      /* Find the next comma in the list. */

      if ((next_link = strchr (cur_link, ',')) != NULL)
      {
         /* replace it with ZERO and increment next_link */

         *next_link = ZERO;
         next_link++;
      }

      /* remove the link */

      if (rc = unlink (cur_link))
      {
         /* could not remove a link */

         instl_msg (SCREEN_LOG, MSGSTR( MS_INUCONVERT, ICV_RMV_LINK_E, ICV_RMV_LINK_D),
                  inu_cmdname, cur_link, rc, inv_file, stanza);
         return (FAILURE);
      } /* end if */

      /* prepare to process the next link */

      cur_link = next_link;
   } /* end if */

   return (SUCCESS);

} /* end del_links */


/*****************************************************************************
** Function: moveit
**
** Purpose: Move a file from source to destination
**
*/

static int moveit (char * from,
                   char * to)
{
   int    rc;                       /* return code  */
   char  *last_slash;               /* pointer to the last "/" in a character
                                     * string */
   char   temp_file[PATH_MAX + 1];  /* buffer to hold name of temporary file */

   if ((last_slash = strrchr (to, '/')) == NULL)
   {
      /* no "/" in the destination pathname */

      return (MOVEIT_DEST_PATHNAME_FORMAT);
   }


   if (last_slash != to)
   {
      /* There is more than one slash in the destination pathname. Call
       * inu_mk_dir to ensure that all directories in the destination path
       * exist prior to moving the file */

      *last_slash = ZERO;

      if (inu_mk_dir (to) != SUCCESS)
      {
         /* can't create a directory in the destination file's path */

         *last_slash = '/';

         return (MOVEIT_CREATE_DIRECTORY);
      } /* end if */

      *last_slash = '/';
   } /* end if */


   /* First copy the destination file to a temporary file before moving it
    * to the destination.
    *
    * Motivation: The files "from" and "to" might be linked. If they
    *             are linked, removing the "from" file also removes the "to"
    *             file.
    *
    *             For example, the directory /usr/lpp/msg might be
    *             linked to /usr/lib/nls/msg. The files
    *             /usr/lpp/msg/En_US/foo and /usr/lib/nls/msg/En_US/foo
    *             are the same and unlinking the former causes the the latter
    *             to disappear also. Not cool!
    */

   strcpy (temp_file, to);
   strcat (temp_file, ".INUCONVERT");

   if ((rc = inu_cp (from, temp_file)) != INUGOOD)
   {
      /* could not copy the source to the temp file  */

      unlink (temp_file);
      return (MOVEIT_COPY_TO_TEMP);
   }
   else
   {
      /* copy succeeded, unlink the source file */

      if (unlink (from) != SUCCESS)
      {
         /* couldn't unlink the source file */

         unlink (temp_file);
         return (MOVEIT_UNLINK_FROM_SOURCE);
      } /* end if */

      unlink (to);      /* in case it already exists */

      if (link (temp_file, to) != SUCCESS)
      {
         /* Couldn't link the destination file to the temporary file. */

         unlink (temp_file);
         return (MOVEIT_LINK_TEMP_WITH_DEST);
      } /* end if */

      if (unlink (temp_file))
      {
         /* couldn't unlink the temporary file */

         return (MOVEIT_UNLINK_TEMP);
      } /* end if */

      return (SUCCESS);

   } /* end if */

} /* end of moveit */



/*****************************************************************************
** Function: convert_vrmf_to_string
**
** Purpose: Convert the specified lpp-option's version, release, modfication
**          and fix numbers to strings.
**
*/

void convert_vrmf_to_string (Option_t * lppopt)
{
   short           version;         /* local copy of the version number */
   short           release;         /* local copy of the release number */
   short           modification;    /* local copy of the modification number */
   short           fix_local;       /* local copy of the fix number */
   short           i;               /* loop counter */
   char          * digit = "0123456789";  /* integer to character
                                           * conversion array */

   version = lppopt -> level.ver;
   for (i = 1; i >= 0; i--)
   {
      *(ver + i) = digit[version % 10];
      version /= 10;
   } /* end for */

   release = lppopt -> level.rel;
   for (i = 1; i >= 0; i--)
   {
      *(rel + i) = digit[release % 10];
      release /= 10;
   } /* end for */

   modification = lppopt -> level.mod;
   for (i = 3; i >= 0; i--)
   {
      *(mod + i) = digit[modification % 10];
      modification /= 10;
   } /* end for */

   fix_local = lppopt -> level.fix;
   for (i = 3; i >= 0; i--)
   {
      *(fix + i) = digit[fix_local % 10];
      fix_local /= 10;
   } /* end for */

} /* end convert_vrmf_to_string */



/*****************************************************************************
** Function: find_liblpp
**
** Purpose: Determine if a liblpp.a file exists. It can have any of the
**          following names:
**
**
**            /usr/lpp/<lppname>/inst_convert/<lppname>.vv.rr.lliblpp.a
**
**            /usr/lpp/<lppname>/inst_convert/<lppname>.vv.lliblpp.a
**
**            /usr/lpp/<lppname>/inst_convert/<lppname>.lliblpp.a
**
**            /usr/lpp/<lppname>/inst_convert/liblpp.a
**
**          where vv=version and rr=release. These files are seached for in the
**          order listed above. If one exists, this function returns it along
**          with a successful return code. Otherwise, the return code is set to
**          FAILURE.
*/

static int find_liblpp (char * liblpp,
                        char * lppname)
{

   int             i;   /* loop counter */


   for (i = 0; i <= 5; i++)
   {
      strcpy (liblpp, "/usr/lpp/");
      strcat (liblpp, lppname);
      strcat (liblpp, "/inst_convert/");

      switch (i)
      {
         case 0:
            strcat (liblpp, lppname);
            strcat (liblpp, ".");
            strcat (liblpp, ver);
            strcat (liblpp, ".");
            strcat (liblpp, rel);
            strcat (liblpp, ".");
            strcat (liblpp, mod);
            strcat (liblpp, ".");
            strcat (liblpp, fix);
            strcat (liblpp, ".");
            break;

         case 1:
            strcat (liblpp, lppname);
            strcat (liblpp, ".");
            strcat (liblpp, ver);
            strcat (liblpp, ".");
            strcat (liblpp, rel);
            strcat (liblpp, ".");
            strcat (liblpp, mod);
            strcat (liblpp, ".");
            break;

         case 2:
            strcat (liblpp, lppname);
            strcat (liblpp, ".");
            strcat (liblpp, ver);
            strcat (liblpp, ".");
            strcat (liblpp, rel);
            strcat (liblpp, ".");
            break;

         case 3:
            strcat (liblpp, lppname);
            strcat (liblpp, ".");
            strcat (liblpp, ver);
            strcat (liblpp, ".");
            break;

         case 4:
            strcat (liblpp, lppname);
            strcat (liblpp, ".");
            break;

         default:
            break;

      } /* end switch */

      strcat (liblpp, "liblpp.a");

      if (access (liblpp, E_ACC) == SUCCESS)
         return (SUCCESS);

   } /* end for */

   return (FAILURE);

} /* end find_liblpp */


/*****************************************************************************
** Function: find_inv_file
**
** Purpose: Determine if an inventory file exists. It can have any of the
**          following names:
**
**
**      /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.rr.mmmm.ffff.inv.{u|r}
**
**      /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.rr.mmmm.inv.{u|r}
**
**      /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.rr.inv.{u|r}
**
**      /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.inv.{u|r}
**
**      /usr/lpp/<lppname>/inst_convert/<lppopt_name>.inv.{u|r}
**
**
**      where vv=version, rr=release, mmmm=modification and ffff=fix.
**      find_inv_file seaches for these files in the order listed above.
**      If one exists, this function returns it along with a successful return
**      code. Otherwise, the return code is set to FAILURE.
*/

int find_inv_file (char * inv_file,
                   char * lppname,
                   char * lppopt_name,
                   int    inv_file_type)
{
   int             i;   /* loop counter */

   for (i = 0; i <= 4; i++)
   {
      strcpy (inv_file, "/usr/lpp/");
      strcat (inv_file, lppname);
      strcat (inv_file, "/inst_convert/");
      strcat (inv_file, lppopt_name);
      strcat (inv_file, ".");

      switch (i)
      {
         case 0:
            strcat (inv_file, ver);
            strcat (inv_file, ".");
            strcat (inv_file, rel);
            strcat (inv_file, ".");
            strcat (inv_file, mod);
            strcat (inv_file, ".");
            strcat (inv_file, fix);
            strcat (inv_file, ".");
            break;

         case 1:
            strcat (inv_file, ver);
            strcat (inv_file, ".");
            strcat (inv_file, rel);
            strcat (inv_file, ".");
            strcat (inv_file, mod);
            strcat (inv_file, ".");
            break;

         case 2:
            strcat (inv_file, ver);
            strcat (inv_file, ".");
            strcat (inv_file, rel);
            strcat (inv_file, ".");
            break;

         case 3:
            strcat (inv_file, ver);
            strcat (inv_file, ".");
            break;

         default:
            break;

      } /* end switch */

      if (inv_file_type == USR_INV_FILE)
      {
         strcat (inv_file, "inv.u");
      }
      else
      {
         strcat (inv_file, "inv.r");
      } /* end if */

      if (access (inv_file, E_ACC) == SUCCESS)
         return (SUCCESS);

   } /* end for */

   return (FAILURE);

} /* end find_inv_file */


/*****************************************************************************
**
** Function: find_conv_script
**
** Purpose: Determine if a conversion script exists. It can have any of the
**          following names:
**
**
**        /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.rr.mmmm.ffff.instal
**
**        /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.rr.mmmm.instal
**
**        /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.rr.instal
**
**        /usr/lpp/<lppname>/inst_convert/<lppopt_name>.vv.instal
**
**        /usr/lpp/<lppname>/inst_convert/<lppopt_name>.instal
**
**
**      where vv=version, rr=release, mmmm=modification and ffff=fix.
**      find_inv_file seaches for these files in the order listed above.
**      If one exists, this function returns it along with a successful return
**      code. Otherwise, the return code is set to FAILURE.
*/

static int find_conv_script (char * conv_script,
                             char * lppname,
                             char * lppopt_name)
{
   int             i;   /* loop counter */


   for (i = 0; i <= 4; i++)
   {
      strcpy (conv_script, "/usr/lpp/");
      strcat (conv_script, lppname);
      strcat (conv_script, "/inst_convert/");
      strcat (conv_script, lppopt_name);
      strcat (conv_script, ".");

      switch (i)
      {
         case 0:
            strcat (conv_script, ver);
            strcat (conv_script, ".");
            strcat (conv_script, rel);
            strcat (conv_script, ".");
            strcat (conv_script, mod);
            strcat (conv_script, ".");
            strcat (conv_script, fix);
            strcat (conv_script, ".");
            break;

         case 1:
            strcat (conv_script, ver);
            strcat (conv_script, ".");
            strcat (conv_script, rel);
            strcat (conv_script, ".");
            strcat (conv_script, mod);
            strcat (conv_script, ".");
            break;

         case 2:
            strcat (conv_script, ver);
            strcat (conv_script, ".");
            strcat (conv_script, rel);
            strcat (conv_script, ".");
            break;

         case 3:
            strcat (conv_script, ver);
            strcat (conv_script, ".");
            break;

         default:
            break;

      } /* end switch */

      strcat (conv_script, "instal");

      if (access (conv_script, E_ACC) == SUCCESS)
         return (SUCCESS);

   } /* end for */

   return (FAILURE);

} /* end find_conv_script */


/*****************************************************************************
**
** Function: has_root_part
**
** Purpose: This function updates specified lpp-option's /usr and root
**          LPP and PRODUCT Software Vital Product Databases to indicate
**          that the lpp-option has a root part.
*/

static int has_root_part (Option_t * lppopt)
{
   lpp_t           lpp_row;   /* buffer used to read a row from the LPP VPD */
   prod_t          prod_row;  /* buffer used to read a row from the PRODUCT
                               * VPD  */
   int             rc;        /* return code */


   /* Update the /usr LPP database */

   strcpy (lpp_row.name, lppopt -> name);

   if ((rc = vpdget (LPP_TABLE, LPP_NAME, &lpp_row)) != VPD_OK)
   {
      /* can't get a row from the VPD's LPP database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   lpp_row.cp_flag |= LPP_ROOT;

   if ((rc = vpdchgget (LPP_TABLE, &lpp_row)) != VPD_OK)
   {
      /* can't update a row in the VPD's LPP database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   /* Have just successfully updated the /usr LPP database. Now update the
    * root LPP database */

   if ((rc = vpdremote ()) != VPD_OK)
   {
      /* can't switch to the root VPD database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   /* delete any existing root LPP entry */

   rc = vpddel (LPP_TABLE, LPP_NAME, &lpp_row);

   /* add a new entry for the root LPP database */

   lpp_row.state = ST_AVAILABLE;

   if ((rc = vpdadd (LPP_TABLE, &lpp_row)) != VPD_OK)
   {
      /* can't add a row to the VPD's root LPP database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   vpd_free_vchars (LPP_TABLE, &lpp_row);

   /* Now switch back to the /usr database             */

   if ((rc = vpdlocal ()) != VPD_OK)
   {
      /* can't switch to the usr VPD database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   /* Update the /usr PRODUCT database */

   strcpy (prod_row.lpp_name, lppopt -> name);
   prod_row.ver = lppopt -> level.ver;
   prod_row.rel = lppopt -> level.rel;
   prod_row.mod = lppopt -> level.mod;
   prod_row.fix = lppopt -> level.fix;
   strcpy (prod_row.ptf, lppopt -> level.ptf);

   if ((rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                     PROD_MOD | PROD_FIX | PROD_PTF),
                     &prod_row)) != VPD_OK)
   {
      /* can't get a row from the VPD's PRODUCT database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   prod_row.cp_flag |= LPP_ROOT;

   if ((rc = vpdchgget (PRODUCT_TABLE, &prod_row)) != VPD_OK)
   {
      /* can't update a row in the VPD's PRODUCT database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   /* Have just successfully updated the /usr PRODUCT database. * Now update
    * the root PRODUCT database */

   if ((rc = vpdremote ()) != VPD_OK)
   {
      /* can't switch to the root VPD database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   } /* end if */

   /* delete any existing entry from the root PRODUCT database */

   rc = vpddel (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                PROD_MOD | PROD_FIX | PROD_PTF),
                                &prod_row);

   /* add a new entry for the root PRODUCT database */

   prod_row.state = ST_AVAILABLE;

  /** ---------------------------------------------------------------------- *
   ** Set the update field to INU_TRUE, if an update, INU_FALSE otherwise.
   ** ---------------------------------------------------------------------- */

   if (IF_UPDATE(lppopt->op_type))
      prod_row.update = INU_TRUE;
   else
      prod_row.update = INU_FALSE;

   if ((rc = vpdadd (PRODUCT_TABLE, &prod_row)) != VPD_OK)
   {
      /* can't add a row to the VPD's root PRODUCT database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), inu_cmdname);
      return (FAILURE);
   }

   vpd_free_vchars (PRODUCT_TABLE, &prod_row);

   /* Now switch back to the /usr database             */

   if ((rc = vpdlocal ()) != VPD_OK)
   {
      /* can't switch to the usr VPD database */

      instl_msg (SCREEN_LOG, MSGSTR( MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D) , inu_cmdname);
      return (FAILURE);
   } /* end if */

   return (SUCCESS);

} /* end of has_root_part */
