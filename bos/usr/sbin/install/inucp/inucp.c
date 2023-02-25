static char sccsid[] = "@(#)10  1.25  src/bos/usr/sbin/install/inucp/inucp.c, cmdinstl, bos411, 9432A411a 8/5/94 16:11:51";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: main (inucp)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_INUCP
#define _H_INUCP

#define MAIN_PROGRAM

#include <locale.h>                  /* for call setlocale */
#include <sys/access.h>
#include <sys/stat.h>
#include <inulib.h>

/*
#include <sys/limits.h>
#include <unistd.h>
#include <inuerr.h>
#include <inudef.h>
#include <inu_toc.h>
#include <commonmsg.h>
#include <instlmsg.h>
*/

#endif /* _H_INUCP */


/*
 * NAME: main (inucp)
 *
 * FUNCTION: Copies the files listed in the given apply list for the
 *           specified lpp option from the given start directory to
 *           / or the alternate directory given.
 *
 * EXECUTION ENVIRONMENT: User command.
 *
 * RETURNS: Error codes as listed in /usr/include/inuerr.h.
 *
 * NOTE: The following files are accessed if they exist :
 *       lpp.acf
 *       <lppname>.al
 *       inst_root/<all files appearing in this directory
 *
 */

/* Syntax :
 *
 * inucp -s <startdirectory>    dir where files to be copied reside
 *      [-e <finaldirectory>]   dir where files to be copied to (/)
 *      <listfile>              full path to the apply list to be used
 *      <lppname>               LPP to be copied (complete)
 *
 */

#define MIN_ARG 1
#define MAX_ARG 2

main (argc, argv)
int argc;                               /* number of command line arguments */
char *argv[];                           /* pointers to command line arguments */
{
    FILE *acf;                           /* lpp.acf file pointer */
    FILE *firstacf;                      /* lpp.acf file pointer */
    char acf_path[PATH_MAX+1];           /* pathname to lpp.acf */
    FILE *al;                            /* file pointer to apply list */
    char apply_list[PATH_MAX+1];         /* pathname to apply list */
    char cp_path[2*PATH_MAX+1];          /* path to the file being copied */
    int  e_flag;                         /* -e flag counter */
    int  err_cnt;                        /* counter for errors */
    char *expand;                        /* ptr to the value of INUEXPAND */
    char final_dir[PATH_MAX+1];          /* path to root directory for copy */
    char final_path[2*PATH_MAX+1];       /* path to final directory for copy */
    int  first_time;                     /* while loop indicator */
    int  flag;                           /* command line option flag */
    char *inutempdir;                    /* ptr to tmp dir */
    char lpp_name[MAX_LPP_FULLNAME_LEN+1];/* name of top-level lpp */
    char lpp_path[MAX_LPP_FULLNAME_LEN+1];/* pathname of /usr/lpp/LPPNAME */
    char master_al[PATH_MAX+1];          /* file ptr to master apply list */
    char master_acf[PATH_MAX+1];         /* file ptr to the master acf file */
    char path[PATH_MAX+1];               /* path of file in the apply_list */
    extern char *optarg;                 /* pointer used by getopt () */
    extern int optind;                   /* index used by getopt () */
    char *p;                             /* generic char pointer   */
    int  rc;                             /* return code from shell */
    int  s_flag;                         /* -s flag counter */
    struct stat stbuf;                   /* stat buffer */
    char *strip;                         /* pointer to the last "/" in dir */
    char new_dir[PATH_MAX+1];            /* pointer to strip dir */
    int verbose=0;                       /* >0 if verbose option specified */
    int title_printed=0;                 /* >0 if title already printed */

    inu_cmdname = "inucp";

/* Begin inucp */

/*
 *  set stdout and stderr to no buffering
 *  (this is so command will work correctly in smit)
 */

    (void)setbuf (stdout, NULL);
    (void)setbuf (stderr, NULL);

    (void)setlocale (LC_ALL, "");

   /*-----------------------------------------------------------------------*
    * Open the message catalog
    *-----------------------------------------------------------------------*/
   CATOPEN ();

/*
 *  set default values
 */

    strcpy (final_dir, "/");
    s_flag = 0;
    e_flag = 0;
    first_time = TRUE;
    err_cnt = 0;

/*
 *  parse command line arguments
 */

    while ((flag = getopt (argc, argv, "?s:e:V:")) != EOF) {
        switch (flag) {

            case 's':       /* set lpp_path */

                if (access (optarg, R_ACC) != 0) {
                    /* given start_dir is not valid */
                    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                        CMN_CANT_OPEN_D), INU_INUCP_CMD, optarg);
                    exit (INUNOLPP);
                } /* end if (invalid start dir) */

                strncpy (lpp_path, optarg, (MAX_LPP_FULLNAME_LEN));

                if (stat (lpp_path, &stbuf) != -1) {
                    if ( ! (S_ISDIR (stbuf.st_mode))) {
                        /* the startdir given is not a directory */

                        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOT_DIR_E, 
                            CMN_NOT_DIR_D), INU_INUCP_CMD, "-s", lpp_path);
                        exit (INUACCS);
                    } /* end if not dir */
                } /* end if stat passed (cont. even if failed) */

                s_flag++;
                break;

            case 'e':       /* set final_dir */

                if (access (optarg, W_ACC) != 0) {
                    /* given final_dir is not valid */
                    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                        CMN_CANT_OPEN_D), INU_INUCP_CMD, optarg);
                    exit (INUNOLPP);
                } /* end if (invalid start dir) */

                strncpy (final_dir, optarg, (PATH_MAX));

                if (stat (final_dir, &stbuf) != -1) {
                    if (S_ISREG (stbuf.st_mode)) {
                        /* the finaldir given is a file */

                        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOT_DIR_E, 
                            CMN_NOT_DIR_D), INU_INUCP_CMD, "-e", final_dir);
                        exit (INUACCS);
                    } /* end if not dir */
                } /* end if stat passed (cont. even if failed) */

		strcat (final_dir, "/");
                e_flag++;
                break;

            case 'V':
                verbose = 1;
                break;

            case '?':
            default:
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCP, INUCP_USAGE_E, INUCP_USAGE_D));
                exit (INUBADMN);

        } /* end switch (flag) */
    } /* end while (getopt) */

    /* verify that there are enough arguments to continue */

    if ((argc - optind) < MIN_ARG) {    /* Too few arguments */
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCP, INUCP_USAGE_E, INUCP_USAGE_D));
        exit (INUTOOFW);
    }

    if ((argc - optind) > MAX_ARG) {    /* Too many arguments */
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCP, INUCP_USAGE_E, INUCP_USAGE_D));
        exit (INUTOOMN);
    }

    if (s_flag == 0) { /* then we are missing an argument */
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCP, INUCP_USAGE_E, INUCP_USAGE_D));
        exit (INUTOOFW);
    }

    if (s_flag > 1  ||  e_flag > 1) {  /* -s or -e used too many times */
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCP, INUCP_USAGE_E, INUCP_USAGE_D));
        exit (INUTOOMN);
    }

    /* check that the apply_list is valid and then copy it to apply_list */

    if (access (argv[optind], R_ACC) != 0) {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INUCP_CMD, argv[optind]);
        exit (INUNOAP2);
    } /* end if no access (apply_list) */
    strncpy (apply_list, argv[optind++], (PATH_MAX));    /* set apply_list */


    /* We do not currently use this flag, this is for future use */
    strncpy (lpp_name, argv[optind], (MAX_LPP_FULLNAME_LEN));  /* set lpp_name */

/*
 *  build path names for lpp directory, acf file and temporary files
 */

    strcpy (acf_path, lpp_path);
    strcat (acf_path, "/lpp.acf");       /* set acf_path */

/*
 *  make the master apply list.
 */


    if ((inutempdir = getenv ("INUTEMPDIR")) == NIL (char))
            strcpy (master_al, "/tmp");
    else
        strcpy (master_al, inutempdir);

    strcat (master_al, "/sorted_al_file");

    rc = inu_mk_mstr_al (apply_list, master_al); /* sort/uniq the apply_list */

    if (rc != INUGOOD)
        inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSORT_E, CMN_NOSORT_D), 
                                   INU_INUCP_CMD, apply_list);

/*
 *  open apply list file for in_al
 */

    if ((al = fopen (master_al, "r")) == NULL) {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                   CMN_CANT_OPEN_D), INU_INUCP_CMD, master_al);
        unlink (master_al);
        fclose (al);
        exit (INUNOAP2);
    }

/*
 *  Verify that there is enough space
 */

    rc = inucp_eval_size (lpp_path, final_dir, master_al);

    if (rc == INUFS) 
    {
            unlink (master_al);
            fclose (al);
            exit (INUFS);
    }

    if (rc != INUGOOD)  /* then there was an error getting space */
    {
        if (((expand = getenv ("INUEXPAND")) == NIL (char))  || 
            (expand[0] != '1'))
        {
           /** ----------------------------- *
            **  the -X flag was NOT set 
            ** ----------------------------- */ 

            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                               CMN_NO_SPACE_D), INU_INUCP_CMD);
        }

        else 
        { 
           /** ------------------------------------------------------ *
            **  the -X flag WAS set 
            **  >>>>>>>>> NOTE:  <<<<<<<<<<< 
            **  For now, we'll just give the same msg whether the -X 
            **  was given or not.  LATER we may want to give a msg 
            **  which explicitly advises the user to specify the -X 
            **  flag to installp (msg we give does not).
            ** ------------------------------------------------------ */ 

            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                               CMN_NO_SPACE_D), INU_INUCP_CMD);
        } 

        unlink (master_al);
        fclose (al);
        exit (INUNOSPC);


    } /* end if (inucp_eval_size fails) */

/*
 *  Run the copy.....
 */

    /* loop through the apply list */
    while (fscanf (al, "%s", path) != EOF)
    { 
       /* first we must strip off the ./ from path */
       if ((p = strchr (path, '/')) != NIL (char)) { 
          if ((path[0] == '.')  ||  (path[0] == '/'))
             p++;
          else 
          {
             if (first_time) 
             {
                inu_msg (WARN_MSG, MSGSTR (MS_INUCP, INUCP_WRG_FMT_W, 
                                                     INUCP_WRG_FMT_D));
                first_time = FALSE;
             } /* end (only print message once) */
             p = path;
          } 
       } /* end if (there is no /) */
       else 
       {
          if (first_time) 
          {
             inu_msg (WARN_MSG, MSGSTR (MS_INUCP, INUCP_WRG_FMT_W, 
                                                  INUCP_WRG_FMT_D));
             first_time = FALSE;
          } /* end (only print message once) */
          p = path;
       } 

       /* add final_dir to front of path */
       strcpy (final_path, final_dir);
       strcat (final_path, p);

       /* add the lpp_path to front of path for copy */
       strcpy (cp_path, lpp_path);
       strcat (cp_path, "/");
       strcat (cp_path, p);

       /* strip directory down one level */
       strip = strrchr (final_path, '/');    /* set strip to the last '/' */
       strip[0] = '\0';        /* set the last '/' to '\0'  */
       strcpy (new_dir, final_path); /* set new to dir w/o the last '/' */
       strip[0] = '/';     /* return dir to its original state */

       if (strcmp (new_dir, "") != 0) 
       {
          /* Verify access to write in the directory */
          rc = inu_mk_dir (new_dir);
          if (rc != INUGOOD) 
          {
             /* then something went wrong when making the dir. */
             inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                CMN_CANT_OPEN_D), INU_INUCP_CMD, final_path);
             unlink (master_al);
             fclose (al);
             exit (INUNODIR);
          } /* end if (inu_mk_dir failed) */

       } /* end if (new_dir not the base dir) */

       if (stat (cp_path, &stbuf) != -1) 
       {
          if (S_ISDIR (stbuf.st_mode)) 
          {
             if (stat (final_path, &stbuf) != -1) 
             {
                if (S_ISDIR (stbuf.st_mode))
                   continue;
                else 
                {
                   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                                  CMN_NO_MK_DIR_D), INU_INUCP_CMD, final_path);
                   unlink (master_al);
                   fclose (al);
                   exit (INUNOMK);
                }
             }
             else 
             {
                if ( verbose )
                {
                   if ( ! title_printed )
                   {
                      printf("\n+-----------------------------------------------------------------------------+\n");
                      inu_msg (INFO_MSG, MSGSTR (MS_INUCP, 
                            INUCP_INST_ROOT_I, INUCP_INST_ROOT_D), lpp_path );
                      printf("+-----------------------------------------------------------------------------+\n");

                      title_printed = 1;
                   }
                   printf("%s\n", final_path);
                }

                rc = mkdir (final_path, 0755);
                if (rc != 0) 
                {
                   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                                  CMN_NO_MK_DIR_D), INU_INUCP_CMD, final_path);
                   unlink (master_al);
                   fclose (al);
                   exit (INUNOMK);
                } /* end if (mkdir failed) */
                continue;
             }
          } /* end if (cp_path is a directory) */

          else 
          { /* it is not a directory */

             /* display copy info if verbose specified */
             if ( verbose )
             {
                if ( ! title_printed )
                {
                   printf("\n+-----------------------------------------------------------------------------+\n");
                   inu_msg (INFO_MSG, MSGSTR (MS_INUCP, 
                            INUCP_INST_ROOT_I, INUCP_INST_ROOT_D), lpp_path );
                   printf("+-----------------------------------------------------------------------------+\n");

                   title_printed = 1;
                }
                printf("%s\n", final_path);
             }

             /* copy file to the final destination */
             rc = inu_cp (cp_path, final_path);
             if (rc != INUGOOD) 
             {
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CPFAIL_E, 
                             CMN_CPFAIL_D), INU_INUCP_CMD, cp_path, final_path);
                unlink (master_al);
                fclose (al);
                exit (INUBADC1);
             } /* end if (rc) */
          } /* end else (it is not a directory) */

       } /* end if (stat cp_path) */

       else 
       {
          inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                     CMN_CANT_OPEN_D), INU_INUCP_CMD, cp_path);
          err_cnt++;
          continue;
       }

    } /* end while (read) */

    if ( verbose )
       printf("\n");

/*
 *  open acf file for next section of code.
 */


    if ((firstacf = fopen (acf_path, "r")) != NULL) 
    {/* acf file does exist */

       if (inutempdir == NIL (char))
          strcpy (master_acf, "/tmp");
       else
          strcpy (master_acf, inutempdir);

       strcat (master_acf, "sorted_acf_file");

       rc = inu_mk_mstr_acf (acf_path, master_acf);

       if (rc != INUGOOD)
          inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSORT_E, 
                                     CMN_NOSORT_D), INU_INUCP_CMD, acf_path);

        acf = fopen (master_acf, "r");

        if (chdir (final_dir) != 0)
	{
 	   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                              CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, final_dir);
           unlink (master_al);
           unlink (master_acf);
           fclose (al);
           fclose (firstacf);
           fclose (acf);
           exit (INUBADAR);
	}

        rc = inu_archive (acf, al, NIL (FILE));

        if (rc != INUGOOD) 
        {
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ARFAIL_E, 
                                        CMN_ARFAIL_D), INU_INUCP_CMD);
            unlink (master_al);
            unlink (master_acf);
            fclose (al);
            fclose (firstacf);
            fclose (acf);
	    if (rc == INUFS)
		exit (INUFS);
	    else
            	exit (INUBADAR);
        } /* end if (rc) */

        unlink (master_acf);
        fclose (firstacf);
        fclose (acf);

    } /* end if (acf file exists) */

    unlink (master_al);
    fclose (al);

    /* close the message catalog */
    CATCLOSE ();

    if (err_cnt > 0) 
        exit (INUBADC1);
    else  
        return (INUGOOD); 

} /* end of program */
