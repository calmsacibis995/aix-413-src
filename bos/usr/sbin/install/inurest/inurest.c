static char sccsid[] = "@(#)33  1.43.1.21  src/bos/usr/sbin/install/inurest/inurest.c, cmdinstl, bos41J, 9516B_all 4/21/95 09:03:50";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: main (inurest)
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>      /* for call setlocale */
#include <locale.h>     /* for call setlocale */
#include <sys/limits.h>
#include <sys/acl.h>
#include <signal.h>
#include <sys/events.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <inulib.h>
#include <inurest.h>

#include <sys/time.h>

static void go_to_root (void);
static void set_std_in (char * dev);
static void report_progress (int dummy);
static boolean tape_device (char * device);

char *inu_cmdname = "inurest";

/* NAME: main (inurest)
 *
 * FUNCTION:    Restores files named in apply list from media (via command
 *              restore) and archives those files specified by lpp.acf.
 *
 * EXECUTION
 * ENVIRONMENT: User command.
 *
 * RETURNS:     Error codes as listed in /usr/include/inuerr.h.
 *
 * NOTES:       The following files are accessed (if they exist):
 *                 $INULIBDIR/lpp.acf
 *                 /usr/lpp/LPPNAME/lpp.acf
 *                 /usr/sys/inst_updt/lpp_name
 *
 *              Inurest uses restore (1) to restore files from the distribution
 *              media onto the filesystem of the target host.
 *
 * COMMAND LINE
 * ARGUMENTS    argv [0] = program name.
 *              argv [1] = full path name of apply list
 *              argv [2] = lppname
 *              argv [3] = option file
 *
 * ENVIRONMENT
 * VARS USED    INUTEMPDIR   specifies temp dir to use for sorting acf and
 *                           apply list.
 *
 *              INUTREE      one of 'U', 'R' or 'S'
 *
 * RETURNS      INUGOOD      Successful save operation.
 *              INUBADMN     bad parameter passed.
 *              INUTOOFW     too few parameters.
 *              INUTOOMN     too many parameters.
 *              INUNOAP2     Error in proccessing inurest.
 *
 */

#include <sys/time.h>

static void go_to_root (void);
static void set_std_in (char * dev);
static void report_progress (int dummy);
static void catch_child (int dummy);
static int restore_lives=1;

static int files_to_restore;

static int files_restored;
static char command[MAX_COM_LEN];         /* command line buffer */

main (int    argc, 
      char * argv[])
{
   FILE            *acf;                         /* lpp.acf file pointer */
   char             acf_path[PATH_MAX + 1];      /* pathname to lpp.acf */
   struct sigaction action;
   FILE            *al;                           /* File pointer to apply
                                                  * list. */
   char             apply_list_path[PATH_MAX + 1];/* pathname to apply list */
   char             byte;
   char             device[PATH_MAX + 1];         /* pathname of device */
   char            *character;                    /* Temp character. */
   char            *expand;                       /* var. for INUEXPAND */
   int              flag;                         /* command line option flag */
   char             lpp_name[MAX_LPP_FULLNAME_LEN]; /* name of top-level lpp */
   char             lpp_path[PATH_MAX + 1];       /* pathname of
                                                   * /usr/lpp/LPPNAME */
   char             member[PATH_MAX + 1];         /* pathname of file to be
                                                   * archived */
   extern char     *optarg;                       /* pointer used by getopt ()*/
   extern int       optind;                       /* index used by getopt () */
   char             quiet[2];                     /* quiet flag for restore
                                                   * command */
   int              rc;                           /* return code from shell */
   FILE            *del_fp;                       /* File pointer to temporary
                                                   * file */
   char             tempfile[PATH_MAX + 1];       /* pathname of temporary
                                                   * file */
   boolean          rewind_tape;
   char             sort_file[PATH_MAX + 1];
   char             sort_file_al[PATH_MAX + 1];
   int              tape_delta;
   boolean          this_is_a_tape_device;
   timer_t          timer_id;
   struct itimerstruc_t timer_value;
   struct itimerstruc_t old_timer_value;
   char             inulibdir[PATH_MAX + 1];
   char            *ptr;
   char             inutmpdir[PATH_MAX + 1];
   char             inurest_stamp [PATH_MAX+1];
   char             cmd [PATH_MAX+1];
   FILE		    *Fd;
#define BUF_SIZE    PATH_MAX*2
   char             buf [BUF_SIZE];
   int              verbose = 0;
   char             tmpcmd[80];
   char             *io;

   /* Begin inurest */

   /*
    * Set stdout and stderr to no buffering (this is so command will work
    * correctly in smit).
    */

   (void) setbuf (stdout, NULL);
   (void) setbuf (stderr, NULL);

   (void) setlocale (LC_ALL, "");

   /*-----------------------------------------------------------------------*
    * Open the message catalog
    *-----------------------------------------------------------------------*/
   CATOPEN ();

   /* set default values */

   strcpy (device, "/dev/rfd0");
   quiet[0] = '\0';

   /* parse command line arguments */

   while ((flag = getopt (argc, argv, "d:qV:")) != EOF)
   {
      switch (flag)
      {
         case 'd':
            strcpy (device, optarg);
            break;
         case 'q':
            strcpy (quiet, "q");
            break;
         case 'V':
            verbose = 1;
            break;
         default:
            inu_msg (SCREEN_MSG, MSGSTR (MS_INUREST, RST_USAGE_E, RST_USAGE_D));
            exit (INUBADMN);
      }
   }

   if (argc - optind < 2)       /* Too few arguments */
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_TOO_FW_E, CMN_TOO_FW_D), 
               INU_INUREST_CMD);
      inu_msg (SCREEN_MSG, MSGSTR (MS_INUREST, RST_USAGE_E, RST_USAGE_D));
      exit (INUTOOFW);
   }
   if (argc - optind > 3)       /* Too many arguments */
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_TOO_MN_E, CMN_TOO_MN_D), 
               INU_INUREST_CMD);
      inu_msg (SCREEN_MSG, MSGSTR (MS_INUREST, RST_USAGE_E, RST_USAGE_D));
      exit (INUTOOMN);
   }

   /* Don't use the -S flag if dealing with non-stack diskettes */
   if (getenv ("INU_DEVICE_IS_FLOP_BFF") != NIL (char))
      strcpy( tmpcmd, "/usr/sbin/restbyname " );
   else
      strcpy( tmpcmd, "/usr/sbin/restbyname -S " );

   if ( verbose )
      strcat ( tmpcmd, " -v" );

   strcpy (apply_list_path, argv[optind++]);
   strcpy (lpp_name, argv[optind++]);

   /* See if the apply list is readable */

   if (access (apply_list_path, R_OK) != 0)
   {
      go_to_root ();
      if (access (apply_list_path, R_OK) != 0)
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INUREST_CMD, apply_list_path);
         exit (INUNOAP2);
      }
   }

   /* path to lpp */

   strcpy (lpp_path, "/usr/lpp/");
   strcat (lpp_path, lpp_name);

   ptr = getenv ("INUTEMPDIR");
   if (ptr == NULL)
      strcpy (inutmpdir, "/tmp");
   else
      strcpy (inutmpdir, ptr);

   ptr = getenv ("INULIBDIR");
   if (ptr == NULL)
      strcpy (inulibdir, lpp_path);
   else
      strcpy (inulibdir, ptr);

   /* sort the and unique the apply list */

   strcpy (sort_file_al, inutmpdir);
   strcat (sort_file_al, "/sorted.al");
   unlink (sort_file_al);
   if (inu_mk_mstr_al (apply_list_path, sort_file_al) == INUGOOD)
      strcpy (apply_list_path, sort_file_al);

   /* go to root */
   go_to_root ();

   /* expand the file systems */

   if ((rc = inurest_eval_size (inutmpdir, inulibdir, apply_list_path)) !=
                                                                       INUGOOD)
   {
      /* the -X flag was not set, so we will attempt to continue after issuing
       * a warning. */

      if (rc == INUFS)
         exit (INUFS);

      if (((expand = getenv ("INUEXPAND")) == NIL (char))  || 
          (expand[0] != '1'))
      {
         /** ----------------------------- *
          **  the -X flag was NOT set
          ** ----------------------------- */

         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                              CMN_NO_SPACE_D), INU_INUREST_CMD);
      }
      else
      {
         /** ----------------------------- *
          **  the -X flag WAS set
          ** ----------------------------- */

         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                              CMN_NO_SPACE_D), INU_INUREST_CMD);
      } /* end if */

      /** ------------------------------------------------------- *
       **  At any rate, we don't have enough space to continue.
       ** ------------------------------------------------------- */

      exit (INUNORP2);
   }

   /* path to the archive control file */

   strcpy (acf_path, inulibdir);
   strcat (acf_path, "/lpp.acf");

   /* path to sorted files */

   strcpy (sort_file, inutmpdir);
   strcat (sort_file, "/sorttemp");

   /* Count the number of files that are to be applied. */

   if ((al = fopen (apply_list_path, "r")) == NIL (FILE))
   {
      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INUREST_CMD, apply_list_path);
      exit (INUNOAP2);
   }

   files_to_restore = 0;
   while ((character = fgets (buf,BUF_SIZE,al)) != NULL)
   {
         files_to_restore++;
   } /* end while */

   fclose (al);

   
  /** ------------------------------------------------------- *
   **  Touch the INUREST_ATTEMPTED file in the INUTEMPDIR
   **  directory that indicates to installp that the restore
   **  of the files was actually attempted.
   ** ------------------------------------------------------- */

   strcpy (inurest_stamp, inutmpdir);
   strcat (inurest_stamp, "/");
   strcat (inurest_stamp, INUREST_ATTEMPTED);

   sprintf (cmd, "/usr/bin/touch %s", inurest_stamp);
   system (cmd);

   /* restore the silly thing */

   set_std_in (device);

    /* restbyname args: -x restore by name.
                        -A restore busy files.
                        -Y reverses tape record after restore.
                        -f device
                        -Z input file list
                        -q rflag ??? */

    /* In order to save I/O time, use dd to buffer for restore.
       Keep the customer informed of what is going on by printing out how
       many files have been restored every 30 seconds.  Sorry for the cryptic
       shell script, but I wanted it to run fast so I chopped down the
       variable names to a minium.

       The meaning of the shell script is:
           Initialize the ksh special variable SECONDS to 0.  This variable is
           automagically maintained by ksh to indicate the elapsed time.
           Set the file count to 0 (c=0).
           Set the time delta to 30 seconds (d=30).
           Set the next time_out to the delta.
           Read from stdin (the output of restbyname).
              Bump up the file count for each line read (c = $c + 1)
              If we have used up our time delta, print a message indicating
              how many files have been restored. */

      /*--------------------------------------------- *
      ** Defect 97000
      ** Use dd as a front end to restbyname
      ** If this is modified, look at inu_apply_opts
      ** may have to be modifed to maintain the true
      ** current_off because it depends on dd to leave
      ** the tape positioned at the begining of the
      ** next bff.
      **
      ** stderr is thrown away to avoid the # of records
      ** in/out messages???
      ** If dd fails for some reason, we would get
      ** restore: 0511-473 A unexpected end of file condition has occurred.
      ** There is no writer on the input pipe.
      **--------------------------------------------- */

      /*-------------------------------------------------- *
      * 108862 use dd front end for tape devices only
      * main () sets INU_DEVICE_IS_TAPE to 1 for both
      * single image tapes, and stack tapes.
      *--------------------------------------------------- */

      if (getenv ("INU_DEVICE_IS_TAPE") == NIL (char))
         sprintf (command, "%s -xYA%sf%s -Z %s", tmpcmd,
                            quiet, device, apply_list_path);
      else
         sprintf (command, 
               "/usr/lib/instl/inu_tape_dd -b 62 -d %s | \
/usr/bin/dd ibs=62b obs=1240b conv=sync 2>/dev/null |\
%s -xA -C 1240 -f - -Z %s", device, tmpcmd, apply_list_path);

   restore_lives=1;
   buf[0] = '\0';

   /* Set up to catch time child termination signal. */
   action.sa_handler = catch_child;
   action.sa_mask.losigs = 0;
   action.sa_mask.hisigs = 0;
   action.sa_flags = 0;
   rc = sigaction (SIGCHLD, &action, (struct sigaction *) 0);

   if( (Fd=popen(command,"r"))==NULL)
   {
      inu_msg (FAIL_MSG, MSGSTR(MS_INUREST, RST_FAILURE_E, RST_FAILURE_D));
      exit(INUNORP2);
   }
   
   /* Set up to catch time ALARM signal. */

   action.sa_handler = report_progress;
   action.sa_mask.losigs = 0;
   action.sa_mask.hisigs = 0;
   action.sa_flags = 0;
   rc = sigaction (SIGALRM, &action, (struct sigaction *) 0);

   /* Set the alarm clock to time out every 30 seconds. */
   timer_value.it_interval.tv_sec = 30;
   timer_value.it_interval.tv_nsec = 0;
   timer_value.it_value.tv_sec = 30;
   timer_value.it_value.tv_nsec = 0;
   timer_id = gettimerid (TIMERID_ALRM, DELIVERY_SIGNALS);
   rc = incinterval (timer_id, &timer_value, &old_timer_value);

   /* Consume the output from the restore command and count the number of
   * files read. */
   files_restored = 0;
   buf[0] = '\0';
   while ( (restore_lives) || (feof(Fd) == 0) )
   {
       if ( (fgets(buf,BUF_SIZE,Fd) != NULL) && (strlen(buf) > 0) )
       {
          files_restored++;
          if ( verbose )
             printf("%s", buf);
       }
   } /* end while */
   fclose(Fd);

   /* Turn off our timer. */
   sigaction (SIGALRM, (struct sigaction *) SIG_DFL, (struct sigaction *) 0);
   sigaction (SIGCHLD, (struct sigaction *) SIG_DFL, (struct sigaction *) 0);

   /* stderr from restbyname reports the total number of files restored, so
      no message needed here. */

   /* if there ain't an archive control file we are finished */

   if (access (acf_path, E_ACC) != 0)
      exit (INUGOOD);

   /* open apply list file */

   if ((al = fopen (apply_list_path, "r")) == NIL (FILE))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INUREST_CMD, apply_list_path);
      exit (INUNOAP2);
   }

   /* open file that will contain the archive members that can be deleted */

   strcpy (tempfile, inutmpdir);
   strcat (tempfile, "/inutemp");
   unlink (tempfile);
   if ((del_fp = fopen (tempfile, "w+")) == NIL (FILE))
   {
      /* open fail; continue the install only some space waste */

      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INUREST_CMD, tempfile);
   }

   /*
    *  sort the archive list file before proccessing it
    *  sort acf_path and put the result in sort_file.
    *  open acf file
    */

   if (inu_mk_mstr_acf (acf_path, sort_file) != INUGOOD)
   {
      if ((acf = fopen (acf_path, "r")) == NIL (FILE))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                              CMN_CANT_OPEN_D), INU_INUREST_CMD, acf_path);
         exit (INUNOAP2);
      }
   }
   else
      if ((acf = fopen (sort_file, "r")) == NIL (FILE))
      {
         if ((acf = fopen (acf_path, "r")) == NIL (FILE))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                 CMN_CANT_OPEN_D), INU_INUREST_CMD, acf_path);
            exit (INUNOAP2);
         }
      }

   /*
    *  For each entry in acf file, if the file name is in the apply list, 
    *  archive it in the specified library.  The del_file contains the members
    *  to be cleaned up.
    */

   if ((rc = inu_archive (acf, al, del_fp)) != 0)
   {
      if (rc == INUFS)
         exit (INUFS);
      else
         exit (INUNORP2);
   }

   /* remove the *.o's that were archived */

   unlink (sort_file);
   if (del_fp != NIL (FILE))
   {
      rewind (del_fp);

      while (fscanf (del_fp, "%s", member) != EOF)
         unlink (member);
      fclose (del_fp);
      unlink (tempfile);
   }

   /* close the message catalog */
   CATCLOSE ();

   exit (INUGOOD);

} /* end main */

static void go_to_root (void)
{
   if (chdir ("/") == 0)
      return;

   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                                CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, "/");
   exit (INUCHDIR);

} /* end go_to_root */

/* setup our standard in */

static void set_std_in (char * dev)
{
   int             fd;
   int             new_fd;
   int             rc;

   if (strcmp (dev, "-") == 0)
      return;

   /* forget it if we can't open up the tty */

   if ((fd = open ("/dev/tty", O_RDONLY)) < 0)
      return;
   rc = close (0);
   new_fd = fcntl (fd, F_DUPFD, 0);
   close (fd);

} /* end set_std_in */

static void report_progress (int dummy)
{
   static boolean first_time = TRUE;
   static int files_restored_prev = 0;

   if (first_time)
    {
      inu_msg (PROG_MSG, MSGSTR (MS_INUREST, RST_INFORM_WAIT_I, 
                                             RST_INFORM_WAIT_D));
      first_time = FALSE;
    }
   if (files_to_restore == 0)
      inu_msg (PROG_MSG, MSGSTR (MS_INUREST, RST_PROGRESS_1_I, 
                         RST_PROGRESS_1_D), files_restored, files_to_restore);
   else if (files_restored > 0 && files_restored != files_restored_prev)
      inu_msg (PROG_MSG, MSGSTR (MS_INUREST, RST_PROGRESS_2_I, 
                         RST_PROGRESS_2_D), files_restored);

   /* keep current count */
   files_restored_prev = files_restored;
   return;
} /* end report_progress */


static void catch_child (int dummy)
{
	int rc;
	restore_lives=0;
	wait (&rc);
	if (rc != 0)
	{
		inu_msg (FAIL_MSG, MSGSTR(MS_INUREST, RST_FAILURE_E, RST_FAILURE_D));
		if (getenv ("INU_DEVICE_IS_TAPE") != NIL (char))
			inu_msg (FAIL_MSG, MSGSTR(MS_INUREST, RST_FAILURE_TAPE_E, RST_FAILURE_TAPE_D));
		inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D),
		INU_INUREST_CMD, command);
		exit (INUNORP2);
	}
	
}

