static char sccsid[] = "@(#)86  1.80  src/bos/usr/sbin/install/installp/inu_apply_opts.c, cmdinstl, bos41J, 9511A_all 3/9/95 11:39:18";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_3_1_sysck
 *		inu_already_processed
 *		inu_apply_C_updt
 *		inu_apply_opts
 *		inu_check_apply_success
 *		inu_check_instal_update_file
 *		inu_check_sum
 *		inu_chg_vpd_recs_to_applied
 *		inu_ensure_C_available_rec_exists
 *		inu_mk_al_fil
 *		inu_process_copyright
 *		inu_set_default_status
 *		inu_update_copyright_status
 *		inu_set_vpd_status
 *		inu_valid_al
 *		no_cleanup
 *		rm_empty_save_dir_and_symlink
 *		inu_setup_ckeck_sum
 *		inu_cleanup_ckeck_sum
 *		print_copyright_head_tail
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *---------------------------------------------------------------------*/

#include <sys/file.h>
#include <signal.h>
#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>
#include <inu_string.h>

extern int  updated_installp;
extern int  current_off;
extern int  current_vol;
extern boolean  reboot_usr;
extern boolean  reboot_root;

static int  inu_check_instal_update_file (Option_t *, Option_t *, char *);
static void inu_set_default_status       (Option_t *, Option_t *, int);
static int  inu_apply_C_updt             (Option_t *, Option_t *);
static void inu_set_vpd_status           (Option_t *);
static void inu_check_apply_success      (Option_t *);
static void inu_3_1_sysck                (Option_t *);
static int  no_cleanup                   (Option_t *);

static int  inu_chg_vpd_recs_to_applied       (Option_t *);
static int  inu_ensure_C_available_rec_exists (Option_t *);

static void inu_process_copyright       (Option_t *);
static void inu_update_copyright_status (char *, char *, int);
static int  inu_already_processed       (char *, char *, int);
static int  inu_check_sum               (char *);
static void inu_cleanup_check_sum(Option_t *);
static void inu_setup_check_sum();
static void print_copyright_head_tail(char *,int);

static void rm_empty_save_dir_and_symlink (char *savdir);
static void rm_mig_files (Option_t *cur_op, Option_t *next_op);

static void set_next_position(Option_t *, Option_t *);

/** ----------------------------------------------------------------------- *
 **  GLOBALs to this routine (ONLY)
 ** ----------------------------------------------------------------------- */
char inurest_file [PATH_MAX + 1];
static int  keep_symlink = FALSE;     /* keep symlink to empty directory */ 

#define PRINTHEAD 1
#define PRINTTAIL 2


/*--------------------------------------------------------------------------*
**
** NAME: inu_apply_opts ()
**
** FUNCTION: Apply the options on the sop from cur_op to next_op
**
** RETURN VALUE DESCRIPTION:  SUCCESS or FAILURE
**
**--------------------------------------------------------------------------*/

int inu_apply_opts (Option_t * cur_op, Option_t * next_op)
{
   struct stat stat_buf;
   char     status_file [L_tmpnam + 10];   /* path to status file */
   char     location_file [L_tmpnam + 16]; /* path to location file */
   char     lpp_opt_fil [L_tmpnam + 12];   /* path to lpp.options file */
   char     lppname [MAX_LPP_NAME];        /* top level lpp name */
   char     levname [2 * MAX_LPP_NAME];    /* holds lppname, optname &level */
   char     device [PATH_MAX];         /* path to device */
   char     buf [TMP_BUF_SZ];          /* generic character buffer */
   char     temp_string [TMP_BUF_SZ];  /* generic character buffer */
   char     * ptr;                     /* generic character pointer */
   Option_t * op;                      /* generic Option_t pointer */
   Option_t * last_op;                 /* last option of this operation */
   char     status;                    /* status character from status file */
   char     opt_name [MAX_LPP_NAME];   /* option name from status file */
   FILE     * status_fp;               /* file descripter for status file */
   FILE     * fp;                      /* generic file descriptor */
   int      location_fp;               /* file descripter for location file */
   int      any_succeed;               /* flag for if any applys succeeded */
   int      any_cancel;                /* flag for if any applys canceled */
   int      any_failure;               /* flag for if any applys failed */
   int      any_cleanup;               /* flag for if any applys need cleanup */
   int      found;                     /* generic found flag. */
   char     rm_cmd [PATH_MAX];         /* used to remove a junk file in /tmp */
   char     savdir [PATH_MAX + 1];     /* save directory */
   char     tsavdir [PATH_MAX + 1];    /* alternate save directory */
   char     linkdir [PATH_MAX + 1];    /* symlink->linkdir */
   char     *sep;                      /* separate directories */
   int      rc;                        /* return code */

   inu_get_prodname (cur_op, lppname);

   inu_get_lpplevname_from_Option_t (cur_op, levname);

   if (IF_C_UPDT (cur_op->op_type))
   {
      rc = inu_apply_C_updt (cur_op, next_op);
      return rc;
   }

   /*-----------------------------------------------------------------------*
    * cd to the directory where the liblpp.a file resides.
    *-----------------------------------------------------------------------*/

   if ((rc = chdir (INU_LIBDIR)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                           CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, INU_LIBDIR);

      for (op = cur_op; op != next_op; op = op->next)
         op->Status = STAT_CANCEL;

      return (FAILURE);
   }

   /* process the copyrights in the background
    *
    * Note: if we stop doing the fork then we need to revisit
    * inu_process_copyright();
    */
   if(fork()==0)
   {
      /* don't slow down the main process */
      nice((int)1);

      /* free up some memory */
      free_graph ();

      /* do those copyrights! */
      for (op = cur_op; op != next_op; op = op->next)
         inu_process_copyright (op);
      inu_cleanup_check_sum(cur_op);
      exit(0);
   }


   /*-----------------------------------------------------------------------*
    * If a rename script exists, execute it. This is for 3.1 media only.
    * NOTE: Should we not even look for it if this isn't a 3.1 apply?
    *-----------------------------------------------------------------------*/

   if (access ("rename", X_OK) == 0)
      if (inu_do_exec ("./rename 1>/dev/null 2>&1") == INUSYSERR)
         inu_quit (INUSYSERR);

   /*-----------------------------------------------------------------------*
    * If this is an update or a 3.2 install then make an master apply list.
    *-----------------------------------------------------------------------*/

   if (IF_3_1_UPDATE (cur_op->op_type))
   {
      if ((rc = inu_mk_al_fil (cur_op, next_op)) != SUCCESS)
      {
         for (op = cur_op; op != next_op; op = op->next)
            op->Status = STAT_CANCEL;
         return (FAILURE);
      }

      /** ----------------------------------------------------
       **   So, inusave will know what the calling environment
       **   is -- 3.1 or 3.2
       ** ---------------------------------------------------*/

      if (inu_putenv ("INU31OR32=1") != SUCCESS)
         return FAILURE;
   }
   else if (inu_putenv ("INU31OR32=2") != SUCCESS)
       return FAILURE;

   /*-----------------------------------------------------------------------*
    * Set the INUSAVEDIR environment variable.
    * And, if update with -t directory, make the directory and symlink to it.
    *-----------------------------------------------------------------------*/

   if ( ! N_FLAG  &&  
       IF_UPDATE (cur_op->op_type)   && 
       ! IF_C_UPDT (cur_op->op_type))
   {
      int errcnt = 0;

      /* default save directory not present */
      found = FALSE;

      /* save "directory" is not a symbolic link */
      linkdir[0] = '\0';

      /* save dir path */
      inu_get_savdir (cur_op->vpd_tree, cur_op, savdir);

      /* if, for example, an option was updated previously with "-t directory", 
      ** and now another option is being updated, then there could already be
      ** an alternate save directory.  Check if the save directory
      ** is a symbolic link to a directory that isn't accessible, 
      */

      /* anything there? */
      if (lstat (savdir, &stat_buf) == 0)
      {
         /* symbolic link? */
         if (readlink (savdir, linkdir, sizeof (linkdir)) > 0)
         {
            /* symbolic link to unwriteable or no directory */
            if (stat (savdir, &stat_buf) == -1   || 
	        ! (S_ISDIR (stat_buf.st_mode))   || 
	        accessx (linkdir, W_ACC, ACC_SELF) == -1)
            {
                instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                         INP_APPLY_DANGLING_SAVDIR_SYM_LINK_E, 
                         INP_APPLY_DANGLING_SAVDIR_SYM_LINK_D), 
                         linkdir, savdir);
                errcnt++;
            }
         }
         else
            found = TRUE;  /* default save directory exists */
      }

      /* Check if another option at same level has been installed
      ** and if so, don't attempt to remvoe the save directory if
      ** it is already present.  Do the check here because it must
      ** be done before installing for this cur_op. */

      if (t_FLAG  &&  errcnt == 0) 
      {
         /* another option is there */
         if ((found == TRUE  ||  linkdir[0])  &&  
             (inu_different_level (cur_op)) == FALSE)
            keep_symlink = TRUE;
         /* after apply, remove symlink if dir is empty */
         else
            keep_symlink = FALSE;

         /*  Append save directory to alternate starting path */
         if (strlen (savdir) + strlen (t_SAVE_DIR) + 1  >  sizeof (tsavdir))
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PATH_TOO_BIG_E, 
                                                      INP_PATH_TOO_BIG_D));
            errcnt++;
         }
         else
         {
            (void) strcpy (tsavdir, t_SAVE_DIR);
            if (*savdir != '/') /* savedir should always begins with a / */
               (void) strcat (tsavdir, "/");
            (void) strcat (tsavdir, savdir);
         }

         /* if symbolic link exists, warn if different, use exiting symlink */
         if (errcnt == 0)
         {
            if (linkdir[0])
            {
               if (strncmp (linkdir, tsavdir, strlen (tsavdir)))
               {
                  /* unexpected, not always BAD */
                  /* warn, using different alternate */
                  instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                   INP_APPLY_BAD_SAVDIR_SYM_LINK_E, 
                                   INP_APPLY_BAD_SAVDIR_SYM_LINK_D), linkdir);
               }
            }
            else if (found == TRUE)
            {
               /* warn, using existing save directory */
               instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                   INP_APPLY_BAD_SAVDIR_SYM_LINK_E, 
                                   INP_APPLY_BAD_SAVDIR_SYM_LINK_D), savdir);
            }
            else
            {
               /* use symbolic link to alternate path */
               if ((sep = strrchr (savdir, '/')))
                  *sep = '\0';   /* truncate symbolic link name */

               /* Make the directory the symbolic will be in and
               ** make directory, if needed, to the alternate save directory */
               if (inu_mk_dir (savdir) != INUGOOD  || 
                   inu_mk_dir (tsavdir) != INUGOOD)
               {
                  /* Don't remove savdir since it may be there for another
                  ** option */
                  instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                          CMN_NO_MK_DIR_D), inu_cmdname, t_SAVE_DIR);
                  errcnt++;  /* can't create/access directory */
               }

               *sep = '/';   /* restore path to symlink */

               if (errcnt == 0  &&  symlink (tsavdir, savdir) == -1)
               {
                  /* likely savdir permissions or disk space */
                  perror ("symlink");
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                               INP_CANT_SYM_LINK_E, 
                               INP_CANT_SYM_LINK_D), savdir, tsavdir);
                  /* savdir->tsavdir */
                  errcnt++;
               }
            }  /* else */
         }  /* if errcnt == 0 */
      }  /* if tflag  &&  errcnt == 0 */
      if (errcnt)
      {
         for (op = cur_op; op != next_op; op = op->next)
            op->Status = STAT_CANCEL;
         return FAILURE;
      }
   }

   inu_set_savdir (cur_op->vpd_tree, cur_op);

   /*-----------------------------------------------------------------------*
    * Build the path to where the "lpp.options" file is.
    *-----------------------------------------------------------------------*/

   strcpy (lpp_opt_fil, INU_TMPDIR);
   strcat (lpp_opt_fil, "/lpp.options");

   /*-----------------------------------------------------------------------*
    * Build the path to where the device is.
    *-----------------------------------------------------------------------*/

   if (NEEDS_DEVICE  &&  cur_op->vpd_tree != VPDTREE_ROOT)
      strcpy (device, cur_op->bff->path);
   else
      strcpy (device, "NODEVICE");

   /*-----------------------------------------------------------------------*
    * We're ready to call the instal/update script, there's differences
    * between 3.1 and 3.2 for updates and installations...
    *-----------------------------------------------------------------------*
    * If 3.1 update:
    *-----------------------------------------------------------------------*/

   strcpy (buf, INU_LIBDIR);
   if (IF_3_1 (cur_op->op_type)  &&  cur_op->vpd_tree != VPDTREE_ROOT)
   {
      if (IF_3_1_UPDATE (cur_op->op_type))
      {
         strcat (buf, "/update");
         if (inu_check_instal_update_file (cur_op, next_op, buf)) {
	    rm_empty_save_dir_and_symlink (savdir);
            return (FAILURE);
	 }
         ptr = buf + strlen (buf);

         /*--------------------------------------------------------------*
          * build command line for 3.1 update script.
          * buf = $INULIBDIR/update, device = the device, 
          * lpp_opt_fil = the lpp.option file, INU_LIBDIR = lib path
          *--------------------------------------------------------------*/

         sprintf (ptr, " %s %s %s/al", device, lpp_opt_fil, INU_LIBDIR);
      }
      else
      {
         /*---------------------------------------------------------------*
          * Else it must be a 3.1 install:
          *---------------------------------------------------------------*/

         strcat (buf, "/instal");
         if (inu_check_instal_update_file (cur_op, next_op, buf))
            if (cur_op->content == CONTENT_MCODE)
            {
               strcpy (buf, DEF_INSTAL_SCRIPT);
               if (inu_check_instal_update_file (cur_op, next_op, buf))
                  return (FAILURE);
            }
            else
               return (FAILURE);

         ptr = buf + strlen (buf);

         /*-----------------------------------------------------------*
          * build command line for PRE 3.2 instal script.
          * buf = $INULIBDIR/instal, device = the device, 
          * lpp_opt_fil = the lpp.option file
          *-----------------------------------------------------------*/

         sprintf (ptr, " %s %s", device, lpp_opt_fil);
      }
   }
   else
   {
      /*-------------------------------------------------------------------*
       * Else if it's > than 3.1, check to see if there's a user provided
       * instal/update script, if yes then use it, else use default scripts.
       *-------------------------------------------------------------------*/

      if (IF_UPDATE (cur_op->op_type))
      {
         strcat (buf, "/update");
         if (access (buf, R_OK) != 0)
         {
            strcpy (buf, DEF_UPDATE_SCRIPT);
            if (inu_check_instal_update_file (cur_op, next_op, buf))
            {
               if (cur_op->content == CONTENT_MCODE)
               {
                  strcpy (buf, DEF_UPDATE_SCRIPT);
                  if (inu_check_instal_update_file (cur_op, next_op, buf)) 
                  {
	             rm_empty_save_dir_and_symlink (savdir);
                     return (FAILURE);
                  }
               }
               else 
               {
	          rm_empty_save_dir_and_symlink (savdir);
                  return (FAILURE);
               }
            }

            if ( V_FLAG >= V2 )
            {
               ptr = buf + strlen (buf);
               sprintf( ptr, " -V%d ", V_FLAG );
            }
         }

      }
      else
      {
         /*---------------------------------------------------------------*
          * Else it must be a 3.2 installation
          * Build command line for 3.2 instal/update script.
          *---------------------------------------------------------------*/

         strcat (buf, "/instal");
         if (access (buf, R_OK) != 0)
         {
            strcpy (buf, DEF_INSTAL_SCRIPT);
            if (inu_check_instal_update_file (cur_op, next_op, buf)) 
            {
	       rm_empty_save_dir_and_symlink (savdir);
               return (FAILURE);
            }

            if ( V_FLAG >= V2 )
            {
               ptr = buf + strlen (buf);
               sprintf( ptr, " -V%d ", V_FLAG );
            }
         }


      }

      ptr = buf + strlen (buf);

      /*-------------------------------------------------------------------*
       * build command line for 3.2 instal/update script.
       * buf = "./update", device = the device, 
       * lpp_opt_fil = the lpp.option file
       *-------------------------------------------------------------------*/

      sprintf (ptr, " %s %s", device, lpp_opt_fil);

      /*-------------------------------------------------------------------*
       * If we are dealing with a stacked diskette media, create the
       * .location file in INU_TMPDIR for the instal/update script to use
       * to read the bff off of diskette.
       *-------------------------------------------------------------------*/

      strcpy (location_file, INU_TMPDIR);
      strcat (location_file, "/.location");
      if (TocPtr->type == TYPE_FLOP_SKD)
      {
         location_fp = creat (location_file, 0644);
         sprintf (location_file, "%d %d %d", cur_op->bff->vol, cur_op->bff->off,
                  cur_op->bff->size);
         write (location_fp, location_file, strlen (location_file));
         close (location_fp);
      }
      else
         unlink (location_file);

   }  /* end else > 3.1 */

   /*-----------------------------------------------------------------------*
    * Ignore all signals while we update the vpd...
    *-----------------------------------------------------------------------*/

   inu_ignore_all_sigs ();

   /*-----------------------------------------------------------------------*
    * Set correct state for applying in LPP, PROD, and HISTORY databases
    *-----------------------------------------------------------------------*/

   for (op = cur_op; op != next_op; op = op->next)
   {
      inu_vpd_lpp_prod (op, ST_APPLYING);
      inu_vpd_history (op, HIST_PENDING, HIST_APPLY);
      if (IF_INSTALL (cur_op->op_type))
         inu_vpd_set_states (op, SET_STATES_TO_HOLD);
   }


   unlink ("./fixinfo");

   /*-----------------------------------------------------------------------*
    * Set signals for calling the instal scripts...
    *-----------------------------------------------------------------------*/

   inu_init_sigs ();


   /*-----------------------------------------------------------------------*
    * Execute the instal/update script, then mask the return code...
    *-----------------------------------------------------------------------*/

   /* setup next tape restore */
   if (TocPtr->type == TYPE_TAPE_SKD)
   	set_next_position(next_op,op);

 /** ---------------------------------------------------------------- *
  **  Make sure that the INUREST_ATTEMPTED marker (just a file) is
  **  removed before we call the scripts.  We don't need to do this
  **  if we're calling the scripts for the root part only, since no
  **  restore is done. 
  ** ---------------------------------------------------------------- */

   if (USR_PART  ||  SHARE_PART)
   { 
      strcpy (inurest_file, INU_TMPDIR);
      strcat (inurest_file, "/");
      strcat (inurest_file, INUREST_ATTEMPTED);
      (void) unlink (inurest_file);
   }
   /** ---------------------------------------------------------------- *
    **  Make sure that the stat call in the no_cleanup () routine will 
    **  not succeed if we're doing a root part only install.
    ** ---------------------------------------------------------------- */
   else
      inurest_file[0] = '\0';

   if ((rc = inu_do_exec (buf)) == INUSYSERR)
      inu_quit (INUSYSERR);

  /*-----------------------------------------------------------------------
   * We must check to see if there are any <prod_option>.cfginfo files,
   * and, if so, if any of them contain a the keyword "BOOT".  If so, then
   * set either the reboot_usr or reboot_root environment variable to TRUE
   * for processing back in installp.c.  Once we find one we break out of
   * the loop and continue.
   *----------------------------------------------------------------------*/

   if (IF_4_1 (cur_op->op_type)  &&  cur_op->vpd_tree != VPDTREE_SHARE)
   {
      found = FALSE;

      for (op = cur_op; op != next_op && found != TRUE; op = op->next)
      {
          sprintf (buf, "%s.cfginfo", op->name);

          if ((fp = fopen (buf, "r")) != NIL (FILE))
          {
             while (fscanf (fp, "%s\n", temp_string) != EOF)
             {
                 if (strcmp (temp_string, "BOOT") == 0)
                 {
                    if (op->vpd_tree == VPDTREE_ROOT)
                       reboot_root = TRUE;
                    else
                       reboot_usr = TRUE;
                    found = TRUE;
                    break;
                 }
             }
             fclose (fp);
          }
      }
   }

  /*-----------------------------------------------------------------------*
   ** dd which is used as the front end to restbyname in inurest
   ** causes the tape head to be positioned at the begining of the
   ** next image.  The previous strategy was to use the "Y" flag
   ** with restbyname so that the tape record is reversed and current_off
   ** would match the real current_off.
   ** Now with dd, we gotta bump it up.
   ** We bump current_off iff:
   **    1. Device is involved
   **    2. Dealing with usr/share part
   **    3. inurest_file is present -- indicating that inurest
   **       suceeded. If we don't do this, if inurest failed, 
   **       then our current_off would be ahead by 1.
   ** If for any reason dd is taken out as a front end in inurest, 
   ** the following if segment should be removed too.
   **-----------------------------------------------------------------------*/

   if (NEEDS_DEVICE  &&  
       (cur_op->vpd_tree == VPDTREE_USR  ||  cur_op->vpd_tree == VPDTREE_SHARE))
       if (next_op != NIL (Option_t)  &&  next_op->bff != NIL (BffRef_t)   && 
           (stat (inurest_file, &stat_buf) == 0))
          current_off++;


   /*-----------------------------------------------------------------------*
    * Ignore all signals while we update the vpd...
    *-----------------------------------------------------------------------*/

   inu_ignore_all_sigs ();

   /*-----------------------------------------------------------------------*
    * Build the path name for the status file.
    *-----------------------------------------------------------------------*/

   if ( ! IF_3_1 (cur_op->op_type))
   {
      strcpy (status_file, INU_TMPDIR);
      strcat (status_file, "/status");
   }
   else
      strcpy (status_file, "./status");

   /*-----------------------------------------------------------------------*
    * Set the following flags to indicate the different things that took
    * place when processing the status file.
    *-----------------------------------------------------------------------*/

   any_succeed = FALSE;
   any_cancel  = FALSE;
   any_failure = FALSE;
   any_cleanup = FALSE;

   /*-----------------------------------------------------------------------*
    * Check to see if there's a status file, if yes read it and update the
    * status for each option in the Option_t structure.
    * If an option was successful and it's a 3.1, run sysck and lppchk (if -v)
    *-----------------------------------------------------------------------*/

   /* set the default status for the option */
   inu_set_default_status (cur_op, next_op, rc);

   /* We are only going to process the status file in these cases the
    * instal/update failed and the status file exists. The instal/update 
    * of a 3.2 was successful and the status file exists */

   if ((rc != INUGOOD  ||  (rc == INUGOOD  &&  ! IF_3_1 (cur_op->op_type)))
        &&  (status_fp = fopen (status_file, "r")) != NIL (FILE))
   {

      for (op = cur_op; op != next_op; op = op->next)
      {
         found = FALSE;

         while (fgets (buf, TMP_BUF_SZ, status_fp) != NIL (char))
         {
            status = '\0';
            *opt_name = '\0';
            sscanf (buf, "%c %s", &status, opt_name);
            if (strcmp (op->name, opt_name) != 0)
               continue;

            found = TRUE;
            switch (status)
            {
               /*--------------------------------------------------*
                * This option passed the instal/update scripts, 
                * If this is a 3.1 install image, we need to run
                * sysck, if we run it and it fails mark the option as
                * "failed", else if -v (verify) flag was set, run
                * lppchk (), if lppchk fails mark option as "failed", 
                * else mark the option as successful !  !  !  !  ! 
                *--------------------------------------------------*/

               case 'I':
                  /**
                   **  The instlcient cmd or the maintclient cmd
                   **  was updated while running the instlclient cmd.
                   **  For now, we are not gonna do anything special.
                   **  We're just gonna treat it as success.
                   **/
                  op->Status = STAT_SUCCESS;
                  inu_check_apply_success (op);
                  break;

               case 'S':
                  updated_installp = TRUE;

               case 's':
                  op->Status = STAT_SUCCESS;
                  inu_check_apply_success (op);
                  break;

               case 'f':
                  /*----------------------------------------------*
                   * If the -T k flag wasn't set then we want to
                   * try to cleanup, here we'll add the option
                   * name to the lpp.option file to be cleaned up
                   * after we're done processing the status file.
                   *----------------------------------------------*/

                  if (no_cleanup (op))
                  {
                     /* leave it in the ST_APPLYING state */
                     op->Status = STAT_FAILURE;
                     inu_set_vpd_status (op);
                  }
                  else
                     op->Status = STAT_CLEANUP;
                  break;

               case 'i':
                  op->Status = STAT_IFREQ_FAIL;
                  inu_set_vpd_status (op);
                  break;

               case 'c':
               case 'b':
                  op->Status = STAT_CANCEL;
                  inu_set_vpd_status (op);
                  break;

               case 'v':
                  /*----------------------------------------------*
                   * This means there was a sysck error, if the -Tk
                   * flag wasn't set then we want to try to cleanup
                   *----------------------------------------------*
                   * NOTE: Later we may want to add another state that
                   * means BROKEN-BUT-WE-CAN-REJECT so when we get
                   * a sysck error we'll just set the new lpp-option
                   * to the new state and if the user wants to go
                   * back to the previous level then they can reject
                   * the new lpp-option... (currently you can't reject
                   * a lpp-option in the BROKEN state)
                   *----------------------------------------------*/

                  if (no_cleanup (op))
                  {
                     op->Status = STAT_FAILURE;
                     inu_set_vpd_status (op);
                  }
                  else
                     op->Status = STAT_CLEANUP;
                  break;

               default:
                  /*----------------------------------------------*
                   * A valid option was found but the status was
                   * invalid. We'll try to cleanup...
                   *----------------------------------------------*/

                  op->Status = STAT_CLEANUP;

                  /* invalid status character */
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                                     INP_INTRN_INVSTAT_E, INP_INTRN_INVSTAT_D));
                  break;

            }   /* end switch (status) */

            break;    /* found name, so break out of "while" loop */

         }    /* while (fgets (buf, 256, status_fp) != NIL (char)) */

         if ( ! found)
         {
            inu_get_optlevname_from_Option_t (op, levname);

            /* Didn't find option. (Should have !) */

            instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INTRN_OPTNOTFND_E, 
                                         INP_INTRN_OPTNOTFND_D), levname);
            inu_check_apply_success (op);
         }

         rewind (status_fp);
      } /* end for loop on options */

      fclose (status_fp);

   }  /* end if (fopen (status_file, "r") */
   else
   {
      /*-------------------------------------------------------------------*
       * There was no status file, this is ok as long as either
       * everything was successful or everything was cancelled.
       *-------------------------------------------------------------------*/

      if (rc != INUGOOD  &&  rc != INUCANCL)
         /* no status file */
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INTRN_FLNOTFND_E, 
                                                   INP_INTRN_FLNOTFND_D));

      for (op = cur_op; op != next_op; op = op->next)
         inu_check_apply_success (op);
   }

   /*-----------------------------------------------------------------------*
    * Set the signals back to their default settings...
    *-----------------------------------------------------------------------*/

   inu_init_sigs ();

   for (op = cur_op; op != next_op; op = op->next)
   {
      switch (op->Status)
      {
         case STAT_SUCCESS:
            any_succeed = TRUE;
            break;
         case STAT_FAILURE:
            any_failure = TRUE;
            break;
         case STAT_CANCEL:
            any_cancel = TRUE;
            break;
         case STAT_CLEANUP:
            any_cleanup = TRUE;
            break;
         default:
            break;
      }
   }

   if (any_failure  ||  any_cancel  ||  any_cleanup)
      inu_print_fail_stats (cur_op, next_op);

   /*-----------------------------------------------------------------------*
    * If any needs to be cleaned up, call inu_mk_opt_fil () to create the file
    * "lpp.options". Then call inu_cleanup_opts () to cleanup them up.
    *-----------------------------------------------------------------------*/

   if (any_cleanup)
   {
      if ((rc = inu_mk_opt_fil (cur_op, next_op, STAT_CLEANUP, NIL (char)))
          != SUCCESS)
      {
         for (op = cur_op; op != next_op; op = op->next)
         {
            if (op->Status == STAT_CLEANUP)
            {
               op->Status = STAT_FAILURE;
               inu_set_vpd_status (op);
            }
         }
      }
      else
      {
         /*---------------------------------------------------------------*
          * Set the history record to broken prior to calling cleanup...
          *---------------------------------------------------------------*/

         for (op = cur_op; op != next_op; op = op->next)
         {
            if (op->Status == STAT_CLEANUP)
               inu_vpd_history (op, HIST_BROKEN, HIST_APPLY);
         }

         /*---------------------------------------------------------------*
          * Try to cleanup the option that we can, this is a void function
          * because if we fail in inu_cleanup_opts () then we'll just continue.
          *---------------------------------------------------------------*/
         inu_cleanup_opts (cur_op, next_op);
      }
   }

   /*-----------------------------------------------------------------------*
    * If this is a 3.1 package we need to determine if we need to convert
    * this lpp to the 3.2+ format. This is done by testing to see if there
    * is an "/usr/lpp/<lpp-name>/inst_convert" directory. If yes then call
    * inuconvert () to do the conversion.
    *-----------------------------------------------------------------------*/

   if (IF_3_1 (cur_op->op_type)  &&  cur_op->vpd_tree == VPDTREE_USR)
   {
      strcpy (buf, "/usr/lpp/");
      strcat (buf, lppname);
      strcat (buf, "/inst_convert");
      if (stat (buf, &stat_buf) == 0)
      {
         if ((stat_buf.st_mode &S_IFMT) == S_IFDIR)
         {
            /*-----------------------------------------------------------*
             * Find the last option of this operation and call inuconvert.
             *-----------------------------------------------------------*/

            for (op = cur_op; op->next != next_op; op = op->next);

            last_op = op;
         }
         if ((rc = inuconvert (cur_op, last_op, X_FLAG)) != SUCCESS) {
	    rm_empty_save_dir_and_symlink (savdir);
            return (rc);
         }
      }
   }


  /*---------------------------------------------------------------*
   * Remove all the unneeded files lying around in the background
   *---------------------------------------------------------------*/

   if(fork()==0)
   {
       /* don't slow down the main process */
       nice((int)1);
       setpgrp();
       free_graph ();

       rm_empty_save_dir_and_symlink (savdir);

       rm_mig_files (cur_op, next_op);

       rm_biron_files (cur_op, next_op, OP_APPLY);

       exit(0);

   }

   return (rc);

} /* inu_apply_opts */


/*--------------------------------------------------------------------------*
**
** Function:     inu_ensure_C_available_rec_exists
**
** Purpose:      Ensures that a product VPD record exists for op.
**
** Parameters:   op  -  the option the VPD record must exist for.
**
** Returns:      SUCCESS -  if record either already exists or one was
**                          successfully created.
**               FAILURE -  if can't ensure that a vpd record exists
**
**--------------------------------------------------------------------------*/

int inu_ensure_C_available_rec_exists (Option_t *op)
{
   prod_t prod;
   int    rc=0;

   memset (&prod, NULL, sizeof (prod_t));
   strcpy (prod.lpp_name, op->name);
   strcpy (prod.ptf, op->level.ptf);
   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;

  /** --------------------------------------------------------------- *
   ** If a VPD record for op doesn't exist, create one.
   ** --------------------------------------------------------------- */

   rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                PROD_MOD | PROD_FIX | PROD_PTF), &prod);

   vpd_free_vchars (PRODUCT_TABLE, &prod);

   inu_ck_vpd_sys_rc (rc);

   if (rc == VPD_NOMATCH)
   {
      strcpy (prod.lpp_name, op->name);
      strcpy (prod.ptf, op->level.ptf);
      prod.ver          = op->level.ver;
      prod.rel          = op->level.rel;
      prod.mod          = op->level.mod;
      prod.fix          = op->level.fix;
      prod.fesn[0]      = '\0';
      prod.comp_id[0]   = '\0';
      prod.name         = op->prodname;
      prod.prereq       = op->prereq;
      prod.cp_flag      = (short) set_cp_flag (op);
      prod.sceded_by[0] = '\0';
      prod.fixinfo      = NullString;
      prod.state        = ST_AVAILABLE;
      prod.supersedes   =  strdup (op->supersedes);
      prod.description  =  strdup (op->desc);
      if (TocPtr != NIL (TOC_t))
          set_media_field (&prod, TocPtr);

     /** -------------------------------------------------------------- *
      ** Set the update field (INU_TRUE if update, INU_FALSE if base).
      ** -------------------------------------------------------------- */

      if (IF_UPDATE(op->op_type))
         prod.update = INU_TRUE;
      else
         prod.update = INU_FALSE;

      rc = vpdadd (PRODUCT_TABLE, &prod);
      inu_ck_vpd_sys_rc (rc);
   }

   /** --------------------------------------------------------------- *
    ** Create a history record for this guy.
    ** --------------------------------------------------------------- */

   inu_vpd_history (op, HIST_PENDING, HIST_APPLY);

   if (rc == VPD_OK)
      rc = SUCCESS;
   else
      rc = FAILURE;

   return rc;

} /* inu_ensure_C_available_rec_exists */


/*--------------------------------------------------------------------------*
**
** Function:     inu_chg_vpd_recs_to_applied
**
** Purpose:      Change the product VPD record to the APPLIED state, and
**               create a history VPD record (in the ST_COMPLETE state).
**
** Parameters:   op  -  the option to change VPD records for.
**
** Returns:      SUCCESS -  if records were successfully created.
**               FAILURE -  if records were NOT successfully created.
**
**--------------------------------------------------------------------------*/

int inu_chg_vpd_recs_to_applied (Option_t *op)
{
   prod_t prod;
   int    rc = INUGOOD;

   memset (&prod, NULL, sizeof (prod_t));
   strcpy (prod.lpp_name, op->name);
   strcpy (prod.ptf, op->level.ptf);
   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;

  /** --------------------------------------------------------------- *
   ** if op's VPD record exists, change it from AVAILABLE to APPLIED
   ** --------------------------------------------------------------- */

   rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                PROD_MOD | PROD_FIX | PROD_PTF), &prod);

   inu_ck_vpd_sys_rc (rc);

   if (rc == VPD_OK)
   {
      prod.state = ST_APPLIED;
      rc = vpdchgget (PRODUCT_TABLE, &prod);
      inu_ck_vpd_sys_rc (rc);
      vpd_free_vchars (PRODUCT_TABLE, &prod);

     /** ------------------------------------------------------------------- *
      **  Change the history record for this option to the HIST_APPLY state.
      ** ------------------------------------------------------------------- */

      inu_vpd_history (op, HIST_COMPLETE, HIST_APPLY);

      rc = SUCCESS;
   }

   else
      rc = FAILURE;

   return rc;

} /* inu_chg_vpd_recs_to_applied */


/*--------------------------------------------------------------------------*
**
** Function:     inu_apply_C_updt
**
** Purpose:      Applies the 'Cume' options from [and including] cur_op to
**               [but not including] next_op.
**
** Parameters:   cur_op - ptr to 1st option of cur_op chunk to process
**               next_op - ptr to 1st option of NEXT cur_op chunk to process
**
** Returns:      SUCCESS - when everything succeeds
**               FAILURE - when 1 or more options failed to apply
**
**--------------------------------------------------------------------------*/

int inu_apply_C_updt (Option_t * cur_op, Option_t *next_op)
{
   Option_t *op;                     /* tmp traversal ptr */
   int       rc = 0;                 /* tmp return code      */
   int       final_rc = SUCCESS;     /* function return code */
   boolean   any_succeed = FALSE;    /* for printing success status */
   boolean   any_failure = FALSE;    /* for printing failure status */
   char      levname [MAX_LPP_NAME + MAX_LEVEL_LEN];

   inu_set_default_status (cur_op, next_op, INUCANCL);

   for (op = cur_op;  op != next_op;  op = op->next)
   {
     /** -------------------------------------------------------------------- *
      ** It's an error for the prereq info for this option to be missing, 
      ** since it's a 'Cume' fix.  And 'Cume' fixes are nothing more than
      ** prereq houses.
      ** -------------------------------------------------------------------- */

      if (op->prereq == NIL (char))
      {
         inu_get_optlevname_from_Option_t (op, levname);
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_EMPTY_PREREQ_FILE_E, 
                                      CMN_EMPTY_PREREQ_FILE_D), 
                                      inu_cmdname, levname);
         return FAILURE;
      }

     /** ------------------------------------------------------------------- *
      **  Make sure there's an AVAILABLE record for this guy.
      ** ------------------------------------------------------------------- */

      inu_ensure_C_available_rec_exists (op);

      if (inu_chg_vpd_recs_to_applied (op) == INUGOOD)
      {
         inu_add_supersedes (op);
         op->Status = STAT_SUCCESS;
         any_succeed = TRUE;
      }
      else
      {
         op->Status = STAT_FAILURE;
         final_rc = FAILURE;
         any_failure = TRUE;
      }

   } /* for */

   if (any_failure)
      inu_print_fail_stats (cur_op, next_op);

   return final_rc;

} /* inu_apply_C_updt */


/*--------------------------------------------------------------------------*
**
** NAME: inu_mk_al_fil ()
**
** FUNCTION: This function is only for 3.1 updates, It is here for backwards
**   compatibility. It searches the INU_LIBDIR (directory where the liblpp.a
**   file resides), and finds the apply lists for this update. The  naming
**   convention is "<lpp-option>.al_vv.rr.mmmm.ffff". If a file of this type
**   is found, then the level is compared to the system level. (3.1 update
**   rules are used). If the level checks out ok then the contents of the
**   file is merged into a file called "<lpp-option>.al" and also into a file
**   called "al". The contents of these files are then sorted and uniq'ed.
**
**
** NOTES:  cur_op is the first Option_t for this operation, next_op is the 
**   first Option_t for the next operation.
**
** RETURN VALUE DESCRIPTION:
**
**--------------------------------------------------------------------------*/

int inu_mk_al_fil (Option_t * cur_op, Option_t * next_op)
{
   char            cmd [256];   /* generic character buffer */
   FILE          * al;          /* cumulative apply list for this LPP       */
   FILE          * al_opt;      /* cumulative apply list for an option      */
   char            al_opt_path [MAX_LPP_NAME];   /* pathname of al_opt      */
   FILE          * al_temp;     /* apply list for a single update level     */
   int             any_found;   /* flag: TRUE if any apply lists are applied */
   struct dirent * dp;          /* pointer to a directory entry             */
   DIR           * dirp;        /* directory containing the apply lists     */
   Option_t      * op;          

   /*----------------------------------------------------------------------*
    * Concatenate all apply lists with a level > the current one.
    *----------------------------------------------------------------------*/

   if ((al = fopen ("al", "w")) == NIL (FILE))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                               CMN_CANT_CREATE_D), INU_INSTALLP_CMD, "al");
      return (INUOPEN);
   }

   /*----------------------------------------------------------------------*
    * merge apply lists for this option into "al" files
    *----------------------------------------------------------------------*/

   any_found = FALSE;
   for (op = cur_op; op != next_op; op = op->next)
   {
      /*------------------------------------------------------------------*
       * This is a 3.1 update so we must merge "possibly multiple"
       * apply lists into one called <lpp-option>.al>.
       *------------------------------------------------------------------*/

      /*------------------------------------------------------------------*
       * Open file "<option>.al" to merge all al files for this option.
       *------------------------------------------------------------------*/

      strcpy (al_opt_path, op->name);
      strcat (al_opt_path, ".al");
      if ((al_opt = fopen (al_opt_path, "w")) == NIL (FILE))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                          CMN_CANT_CREATE_D), INU_INSTALLP_CMD, al_opt_path);
         return (INUOPEN);
      }

      /*------------------------------------------------------------------*
       * Read the directory to find "<option>.al_vv.rr.mmmm.ffff"
       * files, if one is found and it's a valid apply list, then
       * copy it to the "<option>.al" file and the "al" file...
       *------------------------------------------------------------------*/

      dirp = opendir (".");
      while ((dp = readdir (dirp)) != NIL (struct dirent))
      {
         if (inu_valid_al (op, dp->d_name))
         {
            al_temp = fopen (dp->d_name, "r");
            inu_fcopy (al_temp, al);
            rewind (al_temp);
            inu_fcopy (al_temp, al_opt);
            fclose (al_temp);
            any_found = TRUE;
         }
      }
      closedir (dirp);

      /*------------------------------------------------------------------*
       * Close cumm option apply list, sort &remove redundant files
       *------------------------------------------------------------------*/

      fclose (al_opt);
      strcpy (cmd, "/bin/sort -u -o al.temp ");
      strcat (cmd, al_opt_path);
      if (inu_do_exec (cmd) == INUSYSERR)
          inu_quit (INUSYSERR);
      inu_mv ("al.temp", al_opt_path);

   }    /* end of for loop on the options for this bff */

   fclose (al);

   /*---------------------------------------------------------------------*
    * If any "<option>.al" apply lists were found (any_found), then
    * do a sort and uniq on the master apply list file (the "al" file).
    *---------------------------------------------------------------------*/

   if (any_found)
   {
      vpdterm ();
      if (inu_do_exec ("/bin/sort -u -o al_temp al") == INUSYSERR)
         inu_quit (INUSYSERR);
      inu_mv ("al_temp", "al");
   }

   return (SUCCESS);

} /* inu_mk_al_fil */


/*--------------------------------------------------------------------------*
**
** NAME: inu_valid_al
**
** FUNCTION: determines if a file is an apply list for an update level that
**       must be applied to the currently installed LPP to bring it up to
**       the new update level.
**
** NOTES: The apply lists are of the form "al_V.R.M.F" or
**    "option_name.al_V.R.M.F" where V, R, M and F are the version, 
**    release, modification and fix of the apply list.
**
** RETURN VALUE DESCRIPTION: 0 -- the apply list is not valid for this option
**               1 -- the apply list is valid for this option
**
**--------------------------------------------------------------------------*/

int inu_valid_al (Option_t * op,     /* Ptr to Option_t struct for this
                                        lpp-option */
                  char * file_name)  /* File name of apply list
                                        candidate. */
{
   int        length;      /* length of the option name  */
   Level_t    level;       /* level of apply list */
   int        fil_ndx;     /* index into file_name beginning at level */

   fil_ndx = 0;

   /*----------------------------------------------------------------------*
    * In the form of "al_V.R.M.F"
    *----------------------------------------------------------------------*/

   if (strncmp (file_name, "al_", (size_t) 3) == 0)
      fil_ndx = 3;
   else
   {
      /*------------------------------------------------------------------*
       * In the form of "<lpp-option>.al_V.R.M.F"
       *------------------------------------------------------------------*/

      length = strlen (op->name);
      if (strncmp (file_name, op->name, length) == 0  && 
          strncmp (&file_name[length], ".al_", 4) == 0)

         /*--------------------------------------------------------------*
          * option names matched
          *--------------------------------------------------------------*/

         fil_ndx = length + 4;
   }
   if (fil_ndx > 0)
   {
      /*------------------------------------------------------------------*
       * Convert the "V.R.M.F" into the level structure...
       *------------------------------------------------------------------*/

      inu_31level_convert (&file_name[fil_ndx], &level);
      level.sys_ver = op->level.sys_ver;
      level.sys_rel = op->level.sys_rel;
      level.sys_mod = op->level.sys_mod;
      level.sys_fix = op->level.sys_fix;

      /*------------------------------------------------------------------*
       * compare to current level of installed option.
       *------------------------------------------------------------------*/

      if (inu_level_chk_3_1 (&level) == 1)
         return (1);
   }
   return (0);

} /* inu_valid_al */


/*--------------------------------------------------------------------------*
**
** NAME:  inu_3_1_sysck
**
** FUNCTION:  process 3_1 sysck and lppchk
**
**--------------------------------------------------------------------------*/

static void inu_3_1_sysck (Option_t *op)
{
   char            levname [MAX_LPP_NAME + MAX_LEVEL_LEN];

   if (inu_run_sysck (op->name) != SUCCESS)
   {
      op->Status = STAT_FAILURE;
      inu_get_optlevname_from_Option_t (op, levname);
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_SYSCK_ERROR_E, 
                               INP_SYSCK_ERROR_D), levname);
      return;
   }

   op->Status = STAT_SUCCESS;

} /* inu_3_1_sysck */


/*--------------------------------------------------------------------------*
**
** NAME:  inu_check_apply_success
**
** FUNCTION:  check 3.1 with sysck and set status in vpd database
**
**--------------------------------------------------------------------------*/

static void inu_check_apply_success (Option_t *op)
{
   if (op->Status == STAT_SUCCESS)
      if (IF_3_1 (op->op_type))
         inu_3_1_sysck (op);
   inu_set_vpd_status (op);

} /* inu_check_apply_success */


/*--------------------------------------------------------------------------*
**
** NAME:  inu_set_vpd_status
**
** FUNCTION:  set the status of the option in the vpd database
**
**--------------------------------------------------------------------------*/

static void inu_set_vpd_status (Option_t *op)
{
   switch (op->Status)
   {
      case STAT_SUCCESS:
         if (IF_INSTALL (op->op_type))
            inu_vpd_lpp_prod (op, ST_COMMITTED);
         else
            inu_vpd_lpp_prod (op, ST_APPLIED);
         inu_vpd_history (op, HIST_COMPLETE, HIST_APPLY);
         break;

      case STAT_CANCEL:
      case STAT_IFREQ_FAIL:
         inu_vpd_lpp_prod (op, ST_AVAILABLE);
         if (IF_INSTALL (op->op_type))
            inu_vpd_set_states (op, SET_HOLD_STATES_TO_PREVIOUS);
         break;

      case STAT_CLEANUP:
         break;

      default:
         inu_vpd_lpp_prod (op, ST_BROKEN);
         inu_vpd_history (op, HIST_BROKEN, HIST_APPLY);
         if (IF_INSTALL (op->op_type))
            inu_vpd_set_states (op, SET_HOLD_STATES_TO_BROKEN);

         break;
   }
} /* inu_set_vpd_status */


/*--------------------------------------------------------------------------*
**
** NAME:  inu_set_default_status
**
** FUNCTION:  sets the default Status for the option according to system
**            return code value 
**
**--------------------------------------------------------------------------*/

static void inu_set_default_status (Option_t * cur_op, 
                                    Option_t * next_op, 
                                    int        status)
{
   Option_t       *op;
   int             new_status;

   switch (status)
   {
      case INUCANCL:
         new_status = STAT_CANCEL;
         break;
      case INUGOOD:
         new_status = STAT_SUCCESS;
         break;
      default:
         if (no_cleanup (cur_op))
            new_status = STAT_FAILURE;
         else
            new_status = STAT_CLEANUP;
         break;
   }
   for (op = cur_op; op != next_op; op = op->next)
      op->Status = new_status;

} /* inu_set_default_status */


/*--------------------------------------------------------------------------*
**
** NAME:  inu_check_instal_update_file
**
** FUNCTION:  check for instal script and print error if not found
**
**--------------------------------------------------------------------------*/

static int inu_check_instal_update_file (Option_t * cur_op, 
                                         Option_t * next_op, 
                                         char     * buf)
{
   Option_t       *op;

   if (access (buf, R_OK) != 0)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                               CMN_CANT_OPEN_D), INU_INSTALLP_CMD, buf);
      for (op = cur_op; op != next_op; op = op->next)
         op->Status = STAT_CANCEL;
      return (FAILURE);
   }
   return (SUCCESS);

} /* inu_check_instal_update_file */


/*--------------------------------------------------------------------------*
**
** NAME:  no_cleanup
**
** FUNCTION:  check to see if no cleanup is needed
**
**--------------------------------------------------------------------------*/

static int no_cleanup (Option_t *op)
{
   struct stat stbuf;

   if (NO_CLEANUP)
      return (TRUE);
   if (IF_INSTALL (op->op_type))
      return (FALSE);

  /** --------------------------------------------------- *
   **  If the -acN flags were given and we DID call the
   **  inurest command, then we're automatically broken.
   **  So, don't even attempt to clean up.
   ** --------------------------------------------------- */
   if (N_FLAG  &&  a_FLAG  &&  c_FLAG  &&  (stat (inurest_file, &stbuf)==0))
      return (TRUE);
   return (FALSE);

} /* no_cleanup */


/*--------------------------------------------------------------------------*
**
** NAME:     inu_process_copyright
**
** FUNCTION: Prints the copyright file in the current directory only if
**           1.)  It exists
**           2.)  Another one just like it has not already been printed for
**                the current product (lpp) during the current installation
**                session.
**           
**           If the copyright file is unique it is saved (cat'd to) a product's
**           master copyright file. 
**
** DETAIL:   Uniqueness of a copyright is determined by obtaining the 
**           check sum of the copyright file.  The check sum is stored in 
**           the variable ck_sum.  The copyright file being processed is 
**           stored in the variable copyright.  Unique copyrights for a 
**           given product (lpp) must be displayed during the installation
**           process.  A temporary file, cr_print_status, will be used only
**           during the install session to determine whether a copyright
**           is unique and should be displayed.  For each product option
**           all unique copyrights must be saved.  Unique copyrights for a
**           given product will be saved in a master copyright file, 
**           copyright.master, which is located in 
**          
**           /usr/lpp/<prod_name>/copyright.master for USR and ROOT
**           /usr/share/lpp/<prod_name>/copyright.master for SHARE
**
**           The file cr_save_status is maintained in the same directory 
**           as the master copyright file and is used to determine unique-
**           ness of the copyrights saved for that product.
**
** RETURNS:  None
**
**--------------------------------------------------------------------------*/

void inu_process_copyright (Option_t *op)
{
   int ck_sum;
   char copyright       [3 * MAX_LPP_FULLNAME_LEN];
   char cr_save_status  [3 * MAX_LPP_FULLNAME_LEN];
   char cr_master       [3 * MAX_LPP_FULLNAME_LEN];

   if (IF_3_X (op->op_type))
      sprintf (copyright, "copyright");
   else
      sprintf (copyright, "%s.copyright", op->name);

   /* if there's no copyright to process then return */
   if (access (copyright, R_OK) != 0)
      return;

   /*----------------------------------------------------------------------
    * create the paths to the copyright save status file, print status file, 
    * and master save file
    *---------------------------------------------------------------------*/

   switch (op->vpd_tree)
   {
      case VPDTREE_SHARE:
         sprintf (cr_save_status, "/usr/share/lpp/%s/.cr.status", 
                  op->prodname);
         sprintf (cr_master, "/usr/share/lpp/%s/copyright.master", 
                  op->prodname);
         break;
      default:
         sprintf (cr_save_status, "/usr/lpp/%s/.cr.status", op->prodname);
         sprintf (cr_master, "/usr/lpp/%s/copyright.master", op->prodname);
         break;
   }

   ck_sum = inu_check_sum (copyright);

   /* print this copyright if it ain't ever been printed before */

   if( ! inu_already_processed (cr_save_status, op->prodname, ck_sum))
   {
      static int did_one=0;
	
      print_copyright_head_tail(op->prodname,PRINTHEAD);

      if(did_one)
         fprintf(stderr, "\n");
      did_one=1;

      inu_cat (copyright, "&", "");
      inu_cat (copyright, cr_master, "a+");
      vappend_file (cr_master, "\n");
      inu_update_copyright_status (cr_save_status, op->prodname, ck_sum);
   }

   unlink (copyright);

}  /* inu_process_copyright */


/*--------------------------------------------------------------------------*
**
** NAME:     inu_already_processed
**
** FUNCTION: Checks to see if the current copyright has already been processed
**           by looking in status_file for a record matching name with a 
**           check sum of chk_sum on the same line.
**
** RETURNS:  TRUE if match found, FALSE otherwise.
**
**--------------------------------------------------------------------------*/

int inu_already_processed (char * status_file, 
                           char * name, 
                           int    chk_sum)
{
   char   processed_name [MAX_LPP_FULLNAME_LEN];
   int    processed_chk_sum;
   FILE * status_fp;

   if ((status_fp = fopen (status_file, "r")) != NIL (FILE))
   {
      while (fscanf (status_fp, "%s %d", processed_name, &processed_chk_sum) 
             != EOF)
         if (chk_sum == processed_chk_sum
		&& strcmp(name,processed_name) == 0)
         {
            fclose (status_fp);
            return TRUE;
         }

      fclose (status_fp);
   }
   return FALSE;

}  /* inu_already_processed */


/*--------------------------------------------------------------------------*
**
** NAME: inu_update_copyright_status
**
** FUNCTION: Appends name and check sum information to the copyright
**           status_file.
**
** Returns:       None.
**
**--------------------------------------------------------------------------*/

void inu_update_copyright_status (char * status_file, 
                                  char * name, 
                                  int    chk_sum)
{
   FILE * status_fp;
   if ((status_fp = fopen (status_file, "a+")) != NIL (FILE))
   {
      fprintf (status_fp, "%s %d\n", name, chk_sum);
      fclose (status_fp);
   }
} /* inu_update_copyright_status */

/*---------------------------------------------------------------------------
 * NAME: rm_empty_save_dir_and_symlink
 *                                                                    
 * FUNCTION: if save directory is a symbolic link, try to remove
 *           target directory.  If directory was empty, it gets removed
 *           and then the symbolic link is removed.
 *                                                                    
 * EXECUTION ENVIRONMENT: User process.  savdir is set ONLY if t_FLAG is true.
 *                                                                   
 * RETURNS: NONE
 *------------------------------------------------------------------------- */  

static void rm_empty_save_dir_and_symlink (char *savdir)
{
   struct stat   stbuf;
   char          linkdir[PATH_MAX+1];    /* symbolic link target */ 
  
   /* When keep_symlink is true don't remove the symlink and an empty 
    * directory because another option was probably updated and it's
    * temporary storage is probably unmounted at the moment */

   if (t_FLAG  &&  keep_symlink == FALSE)
   { 
      /* applied with alternate save dir */
      if (readlink (savdir, linkdir, sizeof (linkdir)) > 0)
         /* a symbolic link */
         if (stat (savdir, &stbuf) != -1)
         {
            /* TBD: Ideally when a save dir is under an alternate save
             * location, then empty directories above the savdir, up to
             * what would have been the default root, would also be removed */

             if (rmdir (linkdir) != -1)
                /* remove empty save directory */
                (void) unlink (savdir);
          }
          else
          {
             /* else symbolic link exists but directory does not. */
             (void) unlink (savdir);   /* remove symbolic link */
          }

   } /* if (t_FLAG  &&  keep_symlink == FALSE) */

} /* rm_empty_save_dir_and_symlink */


/*--------------------------------------------------------------------------*
**
**  Function   rm_mig_files
**  
**  Purpose    Removes migration-specific files.  Right now this only 
**             includes <option>.namelist.
**
**  Returns    None
**
**--------------------------------------------------------------------------*/

void rm_mig_files (Option_t *cur_op, Option_t *next_op)
{
   Option_t *op;
   char     blast_file [PATH_MAX+1];

 /** ----------------------------------------------------- *
  **  Process this sop chunk ...
  ** ----------------------------------------------------- */

   for (op=cur_op;
        op != next_op;
        op=op->next)
   {
      if (IF_INSTALL (op->op_type))
      {
         switch (op->vpd_tree)
         {
            case VPDTREE_USR:
                 sprintf (blast_file, "/usr/lpp/%s/%s.namelist", 
                               op->prodname, op->name);
                 break;
            case VPDTREE_ROOT:
                 sprintf (blast_file, "/usr/lpp/%s/inst_root/%s.namelist", 
                               op->prodname, op->name);
                 break;
            case VPDTREE_SHARE:
                 sprintf (blast_file, "/usr/share/lpp/%s/%s.namelist", 
                               op->prodname, op->name);
                 break;
         }

        /** ----------------------------------------------------- *
         **  Don't care if the file doesn't exist.  Quicker to
         **  just try to blast it.
         ** ----------------------------------------------------- */

         (void) unlink (blast_file);
      }
   }
}

static void set_next_position(Option_t *next_op, Option_t *op)
{
	static char pos_1[128];
	static char pos_2[128];

	sprintf (pos_1, "INU_CURRENT_TAPE_POSITION=%d", current_off);
	inu_putenv (pos_1);

	/* if this is the last option or it's an installp update don't
	 * do any positioning ...
	 */
	if (next_op == NIL (Option_t) || next_op->bff->vol != current_vol
		|| op->bff->action == ACT_INSTALLP_UPDT)
	{
		inu_putenv("INU_NEXT_TAPE_POSITION=0");
		inu_putenv ("INU_REWIND_TAPE=0");
		inu_putenv("INU_POSITION_TAPE=0");
	}
	else
	{
		inu_putenv("INU_POSITION_TAPE=1");
		if(current_off >= next_op->bff->off)
			inu_putenv("INU_REWIND_TAPE=1");
		else
			inu_putenv("INU_REWIND_TAPE=0");
		sprintf(pos_2, "INU_NEXT_TAPE_POSITION=%d", next_op->bff->off);
		inu_putenv(pos_2);
	}
}


static FILE * copy_fd=(FILE *)NULL;
static void inu_cleanup_check_sum(Option_t *op)
{
	print_copyright_head_tail(op->prodname,PRINTTAIL);
	if(copy_fd!=(FILE *)NULL)
		fclose(copy_fd);
	copy_fd=(FILE *)NULL;
}

static void inu_setup_check_sum()
{
        char copyname[PATH_MAX];

	/* remove that file just incase */
	sprintf(copyname,"%s/work.copyright",INU_TMPDIR);
	unlink(copyname);

	/* get the sum's of all the copyrights in the dir */
	inu_do_exec("sum *.copyright copyright > $INUTEMPDIR/work.copyright 2>/dev/null");

	/* open our file discriptor with all the sum's in it */
	copy_fd=fopen(copyname,"r");
}

/*--------------------------------------------------------------------------*
**
** NAME:     inu_check_sum
**
** FUNCTION: Returns the checksum of the file passed in as arg1.
**
** Returns:  The checksum.
**
**--------------------------------------------------------------------------*/

static int inu_check_sum (char *filename)
{
   int     i;
   int	   sum_val;
   char	   show[PATH_MAX];

   /* are we setup for getting check sums? */
   if(copy_fd== (FILE *)NULL)
   {
        inu_setup_check_sum();
        if(copy_fd== (FILE *)NULL)
	     return(0);
   }

   /* reset the cksum file to search for a single cksum */
   fseek(copy_fd,0,0);

   while(fscanf(copy_fd,"%d %d %s",&sum_val,&i,show)==3)
   {
	if(strcmp(filename,show)==0)
	     return(sum_val);
   }

   return(0);

}  /* inu_check_sum */

static void print_copyright_head_tail(char *name,int heads_or_tails)
{
	static int printed_head=0;

	if(heads_or_tails == PRINTHEAD)
	{
		if(printed_head==1)
			return;
		instl_msg (SCREEN_MSG, MSGSTR (MS_INSTALLP, INP_BEGIN_COPYRIGHT_I, 
			INP_BEGIN_COPYRIGHT_D), name);
		printed_head=1;
		return;
	}
	if(heads_or_tails == PRINTTAIL)
	{
		if(printed_head==0)
			return;
		instl_msg (SCREEN_MSG, MSGSTR (MS_INSTALLP, INP_END_COPYRIGHT_I,
			INP_END_COPYRIGHT_D), name);
		printed_head=0;
		return;
	}
}
