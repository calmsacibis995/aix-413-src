static char sccsid[] = "@(#)16  1.85.2.62  src/bos/usr/sbin/install/installp/installp.c, cmdinstl, bos41J, 9523C_all 6/9/95 18:33:12";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: main (installp)
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <locale.h>             /* for call setlocale */
#include <signal.h>             /* for call setlocale */
#include <ulimit.h>             /* for ulimit call */
#include <sys/resource.h>       /* for ulimit call */
#define MAIN_PROGRAM
#include <installp.h>
#include <instl_options.h>
#include <ckp_common_defs.h>
#include <ckprereqmsg.h>
#include <memory.h>
#include <time.h>
#include <inudef.h>

extern char    *sys_errlist[];

extern int      nerr;

int lpp_optind;                 /* index into argv where lpps start         */
boolean do_log = FALSE;         /* To log or not to                         */
int need_to_reexec = FALSE;     /* TRUE if installp is updated  AND if we   */
                                /* need to reexec to process other stuff    */
int updated_installp = FALSE;   /* TRUE if installp is updated              */
int currently_execing = FALSE;  /* TRUE if we HAVE BEEN exec'd by ourselves */
boolean reboot_usr  = FALSE;    /* TRUE if BOOT in a USR .cfginfo file      */
boolean reboot_root = FALSE;    /* TRUE if BOOT in a ROOT .cfginfo file     */
char touch_NIM_netboot_cmd[] = "touch /tmp/mk_netboot";
                                /* cmd to touch file for NIM purposes.     */
/*--------------------------------------------------------------------------*
 * Special globals.
 *--------------------------------------------------------------------------*/

TOC_t      *TocPtr;         /* The pointer to the Media toc               */
Option_t   *SopPtr;         /* the anchor of the Selected Options list    */
Option_t   *SaveSopPtr;     /* anchor to the save sop (due to i ptfs)     */
Option_t   *FailSopPtr;     /* anchor to the pre-installation failure sop */
nl_catd     catd;           /* The file pointer to the message catalog    */

int biron = FALSE;          /* Blast Inst_Roots Or Not variable */

char    CWD [PATH_MAX * 2]; /* dir we were called from */
time_t tp;                  /* for BEGIN time, and progress indicator      */
boolean time_failed=FALSE; 


/*--------------------------------------------------------------------------*
**
** NAME: main (installp)
**
** FUNCTION:
**  Install or update an optional Program Product or installable option.
**
** EXECUTION ENVIRONMENT:
**  Installp can be executed as a stand-alone command, from a SMIT menu, 
**  from instclient, or maintclient. The user must have root authority.
**  The valid media in which installp can work with:
**      single bff image on tape, diskette, or the filesystem
**      (a TOC (table of contents) will be created in memory for these)
**      stacked bff images on tape, diskette, or the filesystem.
**      (a TOC  will be read from the media/or/filesystem and put into memory)
**
** FLAGS :
**
** A             list APAR's included in this update
** a             apply the given lpps
** B             work with only the update images
** b             don't do bosboot
** C             cleanup after a failed install
** c             commit the given lpps
** D             delete the bff when install completed
** d device      device to be used during install
** e logfile     log stdout and stderr
** F             ignore all prereqs and force installation
** f file        use file for a listing of the lpps
** g             include all the prereqs for the listed lpps
** I             use only the install images from list of lpps
** i             output the instructions for given lpps to stdout
** J             outputs messages and progress in SMIT format
** L             invoked from the GUI to list whats on the media
** l             lists all the lpps available on the device
** N             NOSAVE of the replaced files
** O {[r][s][u]} portion of lpp to be installed
** q             quiet restore mode
** P             matchs all previously installed lpps
** p             preview flag
** R             linked to -r
** r             reject the given lpps
** s             list the status of all applied lpps and updates
** T {k}         override the cleanup
** t {directory} alternate save directory path, ... becomes {directory}/...
** u             deinstall the given product options
** v             verify the installation before completion
** V {V0, V1, V2, V3} Verbosity flag -- level of detail displayed -- mainly
**               affects calls to inu_ckreq.  V0 is the default.
** X             expand filesystem as needed
** z blocks      block siZe for restore
**
** RETURNS:
**  Good luck in figuring these out.
**
**--------------------------------------------------------------------------*/


main (int             argc, 
      char           *argv[])
{
   int       rc;                   /* generic return code */
   int       i;                    /* generic counter */
   int       logfd;                /* for logging */
   boolean   touch_NIM_netboot_flag = FALSE; /* flag to indicate need to 
                                                touch file for NIM. */
   Option_t  *end;
   int       bosboot_failed = 0;   /* delay inu_quit if bosboot failed */
   char      timestamp[INU_TIME_SZ];
   char      buf[TMP_BUF_SZ];      /* generic string */
   char      *ptr;                               
   char      *spot_name = NULL;


   inu_cmdname = "installp";

   if (time (&tp) == -1){
       instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_FUNCCALL_E, 
                            CMN_ERR_FUNCCALL_D), inu_cmdname, "time");
       time_failed = TRUE; 
   }

   TocPtr = NIL (TOC_t);

   /*-----------------------------------------------------------------------*
    * set stdout and stderr to no buffering
    * (this is so command will work correctly in smit)
    *-----------------------------------------------------------------------*/

   setbuf (stdout, NIL (char));
   setbuf (stderr, NIL (char));

   setlocale (LC_ALL, "");

   /*-----------------------------------------------------------------------*
    * Open the message catalog
    *-----------------------------------------------------------------------*/

   CATOPEN ();

   /*-----------------------------------------------------------------------*
    * Call ulimit to set max limit on data for links (defect 80051)
    *-----------------------------------------------------------------------*/

   ulimit (SET_DATALIM, RLIM_INFINITY);


   /*-----------------------------------------------------------------------*
    * Set signals to just remove our files if a signal is trapped...
    *-----------------------------------------------------------------------*/

   inu_set_abort_handler ((void (*) (int)) inu_sig_abort);
   inu_signal(SIGCHLD, SIG_IGN);

   umask ((mode_t) (S_IWGRP | S_IWOTH));    /* set umask to 022     */

   /*-----------------------------------------------------------------------*
    * Make tmp directory via mktemp; mkdir;
    * Set global variable INU_TMPDIR[] to indicate the tmp dir path...
    *-----------------------------------------------------------------------*/

   strcpy (INU_TMPDIR, P_tmpdir);
   strcat (INU_TMPDIR, "inutmpXXXXXX");
   if (mktemp (INU_TMPDIR) == NIL (char))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_TMPDIR_E, 
                                   CMN_NO_MK_TMPDIR_D), inu_cmdname);
      inu_quit (INUOPEN);
   }
   else
   {
      if (mkdir (INU_TMPDIR, (mode_t) 0700) != 0)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_TMPDIR_E, 
                                      CMN_NO_MK_TMPDIR_D), inu_cmdname);
         inu_quit (INUOPEN);
      }
   }

   if (getenv ("INU_EXECING") == NIL (char))
      currently_execing = FALSE;
   else
   {
      currently_execing = TRUE;
      /* wait on our tape to finish ... */
      active_tape_restore();
   }

   /*-----------------------------------------------------------------------*
    * process command line and validate command line;
    *-----------------------------------------------------------------------*/

   if ((rc = inu_instl_args (argc, argv, INU_TMPDIR)) != SUCCESS)
      inu_quit (rc);

   /** -------------------------------------------------------------------- *
    **  has this /usr filesystem been converted into a NIM SPOT?            *
    ** -------------------------------------------------------------------- */

   if ( (spot_name = NIM_usr_spot()) != NULL )
   {
   
      /** ------------------------------------------------------------------- *
       ** Yes, the /usr filesystem is a SPOT.  This means that we need to be  *
       ** careful about install operations because there may be diskless      *
       ** and/or dataless clients which use this SPOT.  If so, their root     *
       ** directories need to be kept in sync with this SPOT.                 *
       ** ------------------------------------------------------------------- */

      /** ------------------------------------------------------------------- *
       ** what kind of install operation was requested?                       *
       ** ------------------------------------------------------------------- */

      if ( (a_FLAG || c_FLAG || u_FLAG || r_FLAG ) && 
                 (getenv ("INUSERVERS") == NIL (char)) )
      {
         /** ---------------------------------------------------------------- *
          ** NOT just listing info and NOT deinstall/reject, so warn user     *
          ** about root syncs & continue                                      *
          ** ---------------------------------------------------------------- */

         instl_msg( SCREEN_LOG, MSGSTR( MS_INSTALLP, 
                                      INP_NIM_USR_SPOT_WARNING,
                                      INP_NIM_USR_SPOT_WARNING_D ), spot_name );

         instl_msg( SCREEN_LOG, "\tnim -Fo check %s\n", spot_name);
      }

   } /* if ( in_NIM_land ) */

  /* -------------------------------------------------------------------
   *  If logging was requested, set the do_log and the inu_doing_logging
   *  variables.
   * ------------------------------------------------------------------- */
   if (e_FLAG) 
   {
      extern inu_doing_logging;
      inu_doing_logging=TRUE;
      do_log = TRUE;
   }

  /* ----------------------------------------------------------------
   *  Logfile should setup if it is not a re-exec of installp
   *  AND logging is requested  
   * -------------------------------------------------------------- */
   if ( ! currently_execing  &&  do_log)
   {
       /* Open the log file */
       if ((logfd = open (LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
       {
           instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                CMN_CANT_OPEN_D), INU_INSTALLP_CMD, LOGFILE);
           inu_quit (INUOPEN);
       }

       /* Move to the end of the file */
       /* if (lseek (logfd, 0L, stderrkkkkk) < 0) {
           instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                CMN_ERR_SYSCALL_D), inu_cmdname, "lseek");
           inu_quit (INUSYSERR);
       } */

       /*  Dup stdout to logfile, i.e. 1>logfile */
       if (dup2 (logfd, STDOUT_FILENO) == -1) 
       { 
          instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                       CMN_ERR_SYSCALL_D), inu_cmdname, "dup");
          inu_quit (INUSYSERR);
       }


       /* Write the time stamp */
       if ( ! time_failed)
          instl_msg (LOG_MSG, "\nBEGIN:%s:%s\n", 
                   ((ptr=ctime (&tp)) == NIL (char)) ? "" : strtok (ptr, "\n"), 
                   (strftime (timestamp, INU_TIME_SZ-1, INU_TIME_FORMAT, 
                   gmtime (&tp)) == 0) ? "" : timestamp); 
       
       /* Write the installp command line */
       write_command_line (argc, argv);

    }

  /**-----------------------------------------------------------------------*
   **  Check to see if the inurid cmd is running.  If it is, we can't
   **  continue cuz it uses the VPD. 
   **-----------------------------------------------------------------------*/

   if (inurid_is_running() && (a_FLAG || c_FLAG || r_FLAG || u_FLAG || C_FLAG))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INURID_RUNNING_E, 
                                                INP_INURID_RUNNING_D));
      inu_quit (INUVPDFL);   /* can't mess w/ VPD cuz inurid is updating it */
   }

   /*-----------------------------------------------------------------------*
    * If invoked from smit, set an environment variable so that inulib
    * routines can use it.
    *-----------------------------------------------------------------------*/

    if (J_FLAG  &&  (getenv ("INU_CALLED_FROM_SMIT") == NULL))
       if ((inu_putenv ("INU_CALLED_FROM_SMIT=1")) != INUGOOD)
          inu_quit (INUSETVAR);

   /* ---------------------------------------------------------------------*
    * We only want to set the installp LOCKS if we are NOT in an exec chain
    * (we've already exec'd ourselves).  We know we're in an exec chain if
    * the env var INU_EXECING has been set (by us).
    *
    * If Cleanup and not execing, skip this so cleanup can occur if there
    * are semaphone locks.
    * ---------------------------------------------------------------------*/

   if ( ! currently_execing  &&  ! C_FLAG)
   {
      /*--------------------------------------------------------------------*
       * Try to create a semaphore lock for each tree the user has requested to
       * perform operations on.
       * The lock id variables need to be globals so inu_quit can use them.
       *--------------------------------------------------------------------*/

      if (USR_PART)
         if ((INU_LOCK_ID_U = inu_testsetlock ("/usr/lpp/inu_LOCK")) == -1) 
         {
             if (errno == EROFS)
                instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_USR_READONLY_E, 
                                                          INP_USR_READONLY_D));
             else
                instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_LOCKPENDING_E, 
                                                          INP_LOCKPENDING_D));
             inu_quit (INUPEND);
         }

      if (ROOT_PART)
         if ((INU_LOCK_ID_M = inu_testsetlock ("/lpp/inu_LOCK")) == -1)
         {
            if (errno == EROFS)
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_USR_READONLY_E, 
                                                         INP_USR_READONLY_D));
            else
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_LOCKPENDING_E, 
                                                         INP_LOCKPENDING_D));
            if (USR_PART)
               inu_unsetlock (INU_LOCK_ID_U);
            inu_quit (INUPEND);
         }

      if (SHARE_PART)
         if ((INU_LOCK_ID_S=inu_testsetlock("/usr/share/lpp/inu_LOCK")) == -1)
         {
            if (errno == EROFS)
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_SHARE_READONLY_E, 
                                                         INP_SHARE_READONLY_D));
            else
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_LOCKPENDING_E, 
                                                         INP_LOCKPENDING_D));

            if (USR_PART)
               inu_unsetlock (INU_LOCK_ID_U);
            if (ROOT_PART)
               inu_unsetlock (INU_LOCK_ID_M);
            inu_quit (INUPEND);
         }
   }

  /** ----------------------------------------------------------------- *
   **  Display the appropriate section header, if we're applying, 
   **  committing or rejecting.
   ** ----------------------------------------------------------------- */

   if (p_FLAG)
   {
      instl_msg (INFO_MSG, "****************************************");
      instl_msg (INFO_MSG, "***************************************\n");

      if      (a_FLAG)
         instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_INST_NOT_I, 
                                                   INP_PREV_INST_NOT_D));
      else if (c_FLAG)
         instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_COMMIT_NOT_I,  
                                                   INP_PREV_COMMIT_NOT_D));
      else if (r_FLAG)
         instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_REJECT_NOT_I, 
                                                   INP_PREV_REJECT_NOT_D));
      else if (u_FLAG)
         instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_DEINST_NOT_I, 
                                                   INP_PREV_DEINST_NOT_D));

      instl_msg (INFO_MSG, "****************************************");
      instl_msg (INFO_MSG, "***************************************\n\n");
   }



   if (a_FLAG || r_FLAG || c_FLAG || u_FLAG || C_FLAG)
   {
      /**
       **  If we're rejecting or deinstalling, then we shouldn't
       **  require any free space in /usr.  We do, however, require
       **  20KB of free space in /tmp -- so the reject or deinstall
       **  script will have some working space.
       **/

      if (r_FLAG || u_FLAG || C_FLAG)
      {
         if ((rc = inu_get_min_space (P_tmpdir, 20)) != SUCCESS)
            inu_quit (rc);
      }

      else   /* we're applying or commiting */
      {
         /*-----------------------------------------------------------*
          * Get the minimum required working space in /tmp and /usr
          * Need at least 100KB in /tmp, and at least 1000KB in /usr
          *-----------------------------------------------------------*/

         if ((rc = inu_get_min_space (P_tmpdir, MIN_SPACE_TMP)) != SUCCESS)
            inu_quit (rc);

         if (USR_PART  ||  SHARE_PART)
         {
            if ((rc = inu_get_min_space ("/usr", MIN_SPACE_USR)) != SUCCESS)
               inu_quit (rc);
         }
      } /* else */

      /*----------------------------------------------------------------------*
       * Validate the VPD for every tree that we're dealing with.
       * If we find any entries in the product and history database in the
       *   applying state and the -C (cleanup) flag was not given, error off.
       *   if -C was given then continue...
       * If we find any entries in the product database in the rejecting, 
       *   committing, or deinstalling state then change the state to broken.
       * If we find any entries in the history file in the PENDING state, 
       *   set those to BROKEN, except for those whose event is set to APPLY, 
       *   in which case, leave alone...
       *---------------------------------------------------------------------*/

      if ((rc = inu_validate_vpd ()) != SUCCESS)
         inu_quit (rc);
   }

   /** --------------------------------------------------------------------- *
    *  Defect 88407 
    *  We wanna do bosboot stuff only for Apply/Commit/Reject/Cleanup 
    *  operations.
    ** --------------------------------------------------------------------- */

   /** --------------------------------------------------------------------- *
    **  We don't wanna do any of the bosboot stuff if we're running 
    **  "installp -Or" from a client (diskless or remote usr).
    ** --------------------------------------------------------------------- */

    /* If don't do bosboot, or preview flag is specified */
    if ( !b_FLAG  && !p_FLAG &&    /* disable flags */   
         (getenv ("INUCLIENTS") == NIL (char)) &&
         (a_FLAG || r_FLAG || u_FLAG || C_FLAG)  && 
         (USR_PART || SHARE_PART))
    {
       /*-------------------------------------------------------------------*
        * If my global bosboot pending bit is set in the database then
        * I have exited an install without completing a bosboot.  I will
        * try to complete the bosboot first.
        *-------------------------------------------------------------------*/

       if (inu_bosboot_ispending ())
       {
           if (rc = inu_bosboot_verify ())
              inu_quit (rc);
           if (rc = inu_bosboot_perform ())
              inu_quit (rc);
           if (rc = inu_bosboot_setpending (FALSE))
              inu_quit (rc);
       }
    }

   /** ----------------------------------------------------------------------*
    ** CWD - is the dir that we were called from.  Save it here, in
    **       case we need it later.  We'll need it if we re-exec our-
    **       selves.
    ** ----------------------------------------------------------------------*/

   if ((getcwd (buf, (int) sizeof (buf))) == NIL (char))
      inu_quit (INUUNKWN);
   else
      strcpy (CWD, buf);

   /*-----------------------------------------------------------------------*
    * At this point, a list of all LPPs to be installed exists in the file
    * "INU_TMPDIR/user.list" if any are to be installed.  DEVICE contains
    * the device where the media can be found.
    *-----------------------------------------------------------------------*/

   if (NEEDS_DEVICE)
   {
      /*-------------------------------------------------------------------*
       * check for existence of device
       *-------------------------------------------------------------------*/
      if ((rc = access (DEVICE, F_OK)) != SUCCESS)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NODEV_E, INP_NODEV_D), 
                                      DEVICE);
         inu_quit (INUACCS);
      }

      /*-------------------------------------------------------------------*
       * If DEVICE isn't a full path then it is a file, so prepend our
       * current directory to it so we can find it when we chdir elsewhere.
       *-------------------------------------------------------------------*/

      if (DEVICE[0] != '/')
      {
         if ((getcwd (buf, (int) sizeof (buf))) == NIL (char))
         {
            inu_quit (INUUNKWN);
         }
         else
         {
            strcat (buf, "/");
            strcat (buf, DEVICE);
            strcpy (DEVICE, buf);
         }
      }

      /*-------------------------------------------------------------------*
       * open_toc () will determine the type of device we're dealing with
       * and call the appropriate sub-function to read the toc off of the
       * media. Types of media:
       * stacked tape:     will contain a toc, 
       * stacked diskette: will have a toc (not designed yet)
       * directory:        will have a .toc file in the directory
       * a bff image:      won't have a toc but a linked toc struct will
       *                   be created for it. This image could be on a
       *                   tape, diskette, or a disk file...
       *
       * cd to the INU_TEPDIR directory...
       *-------------------------------------------------------------------*/

      if ((rc = chdir (INU_TMPDIR)) != SUCCESS)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                              CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, INU_TMPDIR);
         inu_quit (INUCHDIR);
      }

      if ((rc = open_toc (&TocPtr, DEVICE, (int) q_FLAG, CALLER_INSTALLP))
                                                                    != SUCCESS)
         inu_quit (INUNOTIN);

      /* ----------------------------------------------------------
       * 108862 inurest needs to know if it is dealing with
       * a tape device
       *-----------------------------------------------------------*/

      if (TocPtr->type == TYPE_TAPE_SKD  ||  TocPtr->type == TYPE_TAPE_BFF)
      {
         if ((inu_putenv ("INU_DEVICE_IS_TAPE=1")) != SUCCESS)
            inu_quit (INUSETVAR);
      }
      else
         if (TocPtr->type == TYPE_FLOP_BFF)
         {
            if ((inu_putenv ("INU_DEVICE_IS_FLOP_BFF=1")) != SUCCESS)
               inu_quit (INUSETVAR);
	 }

      /*-------------------------------------------------------------------*
       * If the -D flag was given, make sure we are dealing with bffs on
       * the file system...
       *-------------------------------------------------------------------*/

      if (D_FLAG  &&  TocPtr->type != TYPE_DISK)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOT_DISKFILE_E, 
                                                   INP_NOT_DISKFILE_D));
         inu_quit (INUINVAL);
      }


      /*-------------------------------------------------------------------*
       * If we're listing only then call inu_ls_toc () to just display the toc
       * and exit.
       *
       * NOTE: Need to check if the "-O r" flag was set, in which case
       *       we need to read the /usr product vpd to find the ones with
       *       root parts and then read the "inst_root/lpp_name" file to
       *       find the options with root part and display them...?
       -------------------------------------------------------------------*/

      if (l_FLAG || L_FLAG) 
      {
         inu_ls_toc (TocPtr->options);
         inu_quit (INUGOOD);
      }

   }                            /* End if (NEEDS_DEVICE) */
   else
      TocPtr = NIL (TOC_t);

  /*-----------------------------------------------------------------------*
   * NEEDS_LPPS = apply (-a), commit (-c), reject (-r), deinstall (-u), 
   *         apply-root (-a -Or), commit-root (-c -Or), reject-root (-r -Or), 
   *         deinstall-root (-u -Or), list-cust-info (-i), list-apar-info (-A), 
   *         list-status (-s), cleanup (-C)...
   *-----------------------------------------------------------------------*/

   if      (a_FLAG)
   { 
      inu_display_section_hdr (SECT_HDR_PI_PROC, OP_APPLY);
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_VERIF_INST_I, 
                                                INP_VERIF_INST_D)); 
   }
   else if (c_FLAG)
   { 
      inu_display_section_hdr (SECT_HDR_PI_PROC, OP_COMMIT);
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_VERIF_COMMIT_I, 
                                                INP_VERIF_COMMIT_D)); 
   }
   else if (r_FLAG)
   { 
      inu_display_section_hdr (SECT_HDR_PI_PROC, OP_REJECT);
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_VERIF_REJECT_I, 
                                                INP_VERIF_REJECT_D)); 
   }
   else if (u_FLAG)
   { 
      inu_display_section_hdr (SECT_HDR_PI_PROC, OP_DEINSTALL);
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_VERIF_DEINST_I, 
                                                INP_VERIF_DEINST_D)); 
   }

   if (a_FLAG || c_FLAG || r_FLAG || u_FLAG)
   {
      instl_msg (PROG_MSG, MSGSTR (MS_INUCOMMON, CMN_PLEASE_WAIT_I, 
                                                 CMN_PLEASE_WAIT_D));
      instl_msg (PROG_MSG, MSGSTR (MS_INUCOMMON, CMN_VERIF_SELS_I, 
                                                 CMN_VERIF_SELS_D));
   }

   if (NEEDS_LPPS)
   {
      inu_create_anchors ();

      /*-------------------------------------------------------------------*
       * call inu_bld_sop () to process user list,  The parameters are:
       * 1) toc pointer, (NULL if there is no media)
       * 2) a pointer to the Selected Options List that
       *    will be used to do the installs/updates.
       * 3) SaveSopPtr - Options for after a re-exec (if installp updates)
       * 4) FailSopPtr - Options failing pre-installation processing. 
       *
       * inu_bld_sop () will call inu_ckreq () to do up-front ckprereqs on
       * the selected options.
       *
       * If the call to inu_bld_sop does NOT succeed (nonzero return code)
       * then we need to check if the s flag was specified by the user.
       * (if it was we have to return INUGOOD (0) to our calling environment).
       *-------------------------------------------------------------------*/

      if ((rc = inu_bld_sop (TocPtr,SopPtr,SaveSopPtr,FailSopPtr)) != SUCCESS)
      {
         if (s_FLAG)
            inu_quit (INUGOOD);
         else
            inu_quit (rc);
      }

      if (s_FLAG)
      {
         rc = inu_do_status (SopPtr);
         inu_quit (rc);
      }

      /*-------------------------------------------------------------------*
       * inu_set_env_vars () will set all of the needed environment variables
       * needed during the processing of options for installp and its
       * sister commands, (inurest, inusave, inucp...)
       *-------------------------------------------------------------------*/

      if ((rc = inu_set_env_vars ()) != SUCCESS)
         inu_quit (rc);

      if (a_FLAG  &&  (USR_PART  ||  SHARE_PART))
      {
         /*---------------------------------------------------------------*
          * Call eval_size () to make sure we have enough room to do the
          * installations based on the size info in the toc...:
          * (THIS ROUTINE HAS NOT BEEN DESIGNED YET)
          * Size calulations during toc processing:
          * ---------------------------------------
          * if new installation:
          * - add permanent size values to the total-permanent value
          *   (We might want to add a fudge factor in on this)
          * - test/set if temp size values > total-temp values
          * - test/set if page-space size values > total-page-space values
          * - ignore /usr/lpp/SAVESPACE in this case
          *   then expand lv if needed.
          * if re-installation or an update:
          * - ignore permanent size values because it will be assumed that
          *   we are replacing a lpp/update with one about the same size
          *   (We might want to add a fudge factor in on this)
          * - test/set if temp size values > total-temp values
          * - test/set if page-space size values > total-page-space values
          * - if -N was set: ignore SAVESPACE value when adding total-perm
          * - if -N wasn't set: add SAVESPACE value to total-perm value
          *   (if the user is apply and commit then we shouldn't add the
          *    SAVESPACE value but just use the largest value found...)
          *   then expand lv if needed.
          *---------------------------------------------------------------*/

         if ((rc = installp_eval_size (SopPtr, INU_COMMAND)) != SUCCESS)
            inu_quit (rc);
      }

       /*-------------------------------------------------------------------*
        * We are going to do bosboot verification (a preliminary check
        * to see if bosboot will pass) if any of the selected options
        * require a bosboot.  If this passes then I will set the
        * global "bosboot pending" bit in the database to signify we
        * require a bosboot after the images are installed.
        *
        * Also, if processing in NIM env and if bosboot is needed, touch a
        * file for NIM to consider when processing network boot images.
        *-------------------------------------------------------------------*/

       /*-------------------------------------------------------------------*
        *  INUCLIENTS will be set in non-/usr SPOT case only
        *  in which case we don't need to do a bosboot
        *  INUSERVERS will be set for /usr SPOTs
        *-------------------------------------------------------------------*/

        if ( (!p_FLAG)                               /* Ignore for preview */
                &&
             (a_FLAG || r_FLAG || u_FLAG || C_FLAG)  /* Valid bosboot ops? */
                &&
             (USR_PART || SHARE_PART) )              /* Ignore if just doing */
                                                     /*   root parts. */
        {
           /*
            * for disk boot ...
            */
           if (!b_FLAG && (getenv ("INUCLIENTS") == NIL (char))) {
              if ((rc = inu_bosboot_isneeded (SopPtr)) == TRUE) {
                 touch_NIM_netboot_flag = TRUE;   /* saves time below */
                 if ((rc = inu_bosboot_verify ()) != SUCCESS)
                    inu_quit (rc);
                 if ((rc = inu_bosboot_setpending (TRUE)) != SUCCESS)
                    inu_quit (rc);
              }
           }

           /*
            * for network boot ...
            */
           if (((getenv ("INUSERVERS") != NIL (char)) ||
                (getenv ("INUCLIENTS") != NIL (char)))
                         &&
               ((touch_NIM_netboot_flag) ||
                (inu_bosboot_isneeded (SopPtr) == TRUE)))
              inu_do_exec (touch_NIM_netboot_cmd);
        }

     /*-------------------------------------------------------------------*
      * At this point we know what we need to process based on the Selected
      * Options list.  So, since we no longer need the toc list (list of 
      * everything on the media), we'll free it -- if it exists.
      *-------------------------------------------------------------------*/

      if (NEEDS_DEVICE) 
         free_toc_list (TocPtr);

      if (p_FLAG)
      {
         if (a_FLAG)
         {
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_END_I, 
                                                      INP_PREV_END_D));
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_NO_APPLY_I, 
                                                      INP_PREV_NO_APPLY_D));
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
         }
         else if (c_FLAG)
         {
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_END_I, 
                                                      INP_PREV_END_D));
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_NO_COMMIT_I, 
                                                      INP_PREV_NO_COMMIT_D));
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
         }
         else if (r_FLAG)
         {
            instl_msg (INFO_MSG, "\n");
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_END_I, 
                                                      INP_PREV_END_D));
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_NO_REJECT_I, 
                                                      INP_PREV_NO_REJECT_D));
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
         }
         else if (u_FLAG)
         {
            instl_msg (INFO_MSG, "\n");
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_END_I, 
                                                      INP_PREV_END_D));
            instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PREV_NO_DEINST_I, 
                                                      INP_PREV_NO_DEINST_D));
            instl_msg (INFO_MSG, "***************************************");
            instl_msg (INFO_MSG, "***************************************\n");
         }
   
         /* always return good return code for preview */
         inu_quit (INUGOOD);
      }

     /*-------------------------------------------------------------------*
      * inu_process_opts () will loop thru the SopPtr (selected option list)
      * and perform the requested operations, assuming of course there's
      * a sop to process.
      *-------------------------------------------------------------------*/

      if (SopPtr->next != NIL (Option_t))
      {
        /*-------------------------------------------------------------------*
         *  Display the appropriate section header ...
         *-------------------------------------------------------------------*/

         if      (a_FLAG)
            inu_display_section_hdr (SECT_HDR_INSTLLING, OP_APPLY);
         else if (c_FLAG)
            inu_display_section_hdr (SECT_HDR_INSTLLING, OP_COMMIT);
         else if (r_FLAG)
            inu_display_section_hdr (SECT_HDR_INSTLLING, OP_REJECT);
         else if (u_FLAG)
            inu_display_section_hdr (SECT_HDR_INSTLLING, OP_DEINSTALL);

         rc = inu_process_opts (TocPtr, SopPtr, FailSopPtr);
      }

      /*
       * Report "unfound" requests from the command line.
       * (These entries should be on the failsop.)
       */
      if (A_FLAG || i_FLAG)
      {
         inu_ls_missing_info (FailSopPtr);
      }

     /** ------------------------------------------------------------- *
      ** Add the SaveSopPtr (all cancelled) list to the SopPtr list
      ** ------------------------------------------------------------- */

      if (SopPtr->next != NIL (Option_t) && SaveSopPtr->next != NIL (Option_t))
      {
         for (end = SopPtr->next;
              end->next != NIL (Option_t);
              end = end->next)
               ;
         end->next = SaveSopPtr->next;
         SaveSopPtr->next = NIL (Option_t);
      }

      
     /*-------------------------------------------------------------------*
      * if my global bosboot pending bit is set in the database then
      * I need to perform a bosboot at the end of this install.
      *-------------------------------------------------------------------*/
      bosboot_failed = 0;  /* Over cautious */

      if ( !b_FLAG  && !p_FLAG &&     /* No don't do bosboot, No preview */
           (a_FLAG || r_FLAG || u_FLAG || C_FLAG)  && 
           (USR_PART || SHARE_PART))

         if (inu_bosboot_ispending ())
         {
           /* --------------------------------------------------------------  *
            * Defect 138605                                                   *
            * get 7.8 meg free space in /tmp                                  *
            * Not calling bosboot_verify since get_min_space would be quicker *
            * Call bosboot even if inu_get_min_space fails, because we wanna  *
            * print the messages from bosboot, and it might even succeed with *
            * less than 7.8 meg                                               *
            * --------------------------------------------------------------  */
            inu_get_min_space (P_tmpdir, BOSBOOT_SPACE_TMP) ;
 
           /*-----------------------------------------------------------------*
            * Defect -- 88458                                                 *
            * Even if bosboot fails, we still wanna print the summary         *
            *-----------------------------------------------------------------*/
            if ( ! (bosboot_failed = inu_bosboot_perform ()))
                 bosboot_failed = inu_bosboot_setpending (FALSE);
         }

     /*-------------------------------------------------------------------*
      *  If we've done an apply action, we may have applied an installp
      *  update.  If this is the case, then we need to prepare to re-exec
      *  installp.
      *-------------------------------------------------------------------*/

      if (a_FLAG)
      {
        /** --------------------------------------------*
         **  If we do NOT need to re-exec ourselves ...
         ** --------------------------------------------*/

         if ( ! need_to_reexec)
         {
            if (a_FLAG  &&  SopPtr->next != NIL (Option_t))
               inu_display_section_hdr (SECT_HDR_SUMMARIES, OP_APPLY);

           /** ------------------------------------------------------------ *
            **  If we have already exec'ed at least once, then we need to 
            **  read in the intermediate summary log file, and merge it into 
            **  the sop.  The summary is printed based on the sop.
            ** ------------------------------------------------------------ */

            if (currently_execing)
               rc = read_and_merge_sum_log (FailSopPtr, SopPtr, 
                                            INTERMED_SUM_LOG_FILE);
            
            rc |= inu_final_summary_log (FailSopPtr, SopPtr);

            rc |= inu_pif_summary       (FailSopPtr);
            rc |= inu_summary           (SopPtr);

           /*---------------------------------------------------------------
            * Display a reboot warning message if "BOOT" string found in any
            * <option>.cfginfo file processed.
            *---------------------------------------------------------------*/
            if (reboot_root)
               instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_REBOOT_ROOT_I,
                                                           INP_REBOOT_ROOT_D));
            if (reboot_usr)
               instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_REBOOT_USR_I,
                                                           INP_REBOOT_USR_D));
         }
         else
         {
           /** ---------------------------------------------------------*
            **  If we are NOT currently re-execing, but are
            **  about to, then remove the intermediate summary file.
            ** ---------------------------------------------------------*/

            if ( ! currently_execing)
               inu_rm_sum_log (INTERMED_SUM_LOG_FILE);

            (void) inu_inter_summary_log (FailSopPtr, SopPtr);
         }
      }
      else if (c_FLAG  ||  r_FLAG  ||  u_FLAG  ||  C_FLAG)
      {
         if (SopPtr->next != NIL (Option_t))
         {
            if      (c_FLAG)
               inu_display_section_hdr (SECT_HDR_SUMMARIES, OP_COMMIT);
            else if (r_FLAG)
               inu_display_section_hdr (SECT_HDR_SUMMARIES, OP_REJECT);
            else if (u_FLAG)
               inu_display_section_hdr (SECT_HDR_SUMMARIES, OP_DEINSTALL);
         }

         rc  = inu_final_summary_log (FailSopPtr, SopPtr);
         rc |= inu_pif_summary       (FailSopPtr);
         rc |= inu_summary           (SopPtr);
      }


      if (bosboot_failed)
         inu_quit (bosboot_failed);

      /** ---------------------------------------------------------------- *
       ** need_to_reexec is set when installp is updated.
       ** If it's set, we need to re-exec installp for all items that
       ** appear as cancelled in the sop, to finish servicing the user's
       ** request.
       ** ---------------------------------------------------------------- */

      if (need_to_reexec)
      {
         if ((rc = inu_exec_installp (argv)) != INUGOOD)
            instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_EXEC_FAIL_E, 
                                                      INP_EXEC_FAIL_D));
      }

      /* close the message catalog */
      CATCLOSE ();

      /* Return the value returned by inu_process_opts if it is non-zero.
         Otherwise, return the value returned by inu_summary. */

      inu_quit (rc);

   } /* end if (NEEDS_LPPS) */

   /* close the message catalog */
   CATCLOSE ();

   inu_quit (INUGOOD);

 } /* installp */
