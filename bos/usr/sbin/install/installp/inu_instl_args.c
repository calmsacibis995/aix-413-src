static char sccsid[] = "@(#)96  1.74  src/bos/usr/sbin/install/installp/inu_instl_args.c, cmdinstl, bos41J, 9521B_all 5/26/95 16:01:06";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_change_tdirinfo
 *		inu_check_one
 *		inu_check_tflag
 *		inu_get_root_lpp_desc
 *		inu_get_tdirinfo
 *              inu_grep_for_applied_in_vpd
 *		inu_instl_args
 *		inu_mk_tdirinfo
 *		inu_tdir_guard
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <memory.h>
#include <installp.h>
#include <instl_options.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/utsname.h>

extern int currently_execing;
/*
 * NAME: inu_instl_args (argc, argv)
 *
 * FUNCTION: This function will parse the command line for installp
 *           and fill the "opt" structure.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS: The "opt" structure with the flags used set.
 */

/*
 * This function uses the following error return codes for inuerr.h
 *
 * INUGOOD  normal return, no errors occurred
 * INUINVAL invalid input parameter
 * INUTOOMN too many parameters were found
 * INUEXCLU mutually exclusive parameters used
 */

/* FLAGS :
 *
 * A             list APAR's included in this update
 * a             apply the given lpps
 * B             work with only the update images
 * b             don't do bosboot
 * C             cleanup after a failed install
 * c             commit the given lpps
 * D             delete the bff when install completed
 * d device      device to be used during install
 * e logfile     log stdout and stderr
 * F             ignore all prereqs and force installation
 * f file        use file for a listing of the lpps
 * g             include all the prereqs for the listed lpps
 * I             use only the install images from list of lpps
 * i             output the instructions for given lpps to stdout
 * J             outputs messages and progress in SMIT format
 * k             To provide colon seperated size preview output 
 * L             invoked from the GUI to list whats on the media
 * l             lists all the lpps available on the device
 * N             NOSAVE of the replaced files
 * O {[r][s][u]} portion of lpp to be installed
 * Q             suppress instreq failures
 * q             quiet restore mode
 * P             matchs all previously installed lpps
 * p             preview flag 
 * R             linked to -r
 * r             reject the given lpps
 * s             list the status of all applied lpps and updates
 * T {k}         override the cleanup
 * t {directory} alternate save directory path, ... becomes {directory}/...
 * u             deinstall the given product options
 * v             verify the installation before completion
 * V {V0, V1, V2, V3, V4} Verbosity flag -- level of detail displayed -- mainly 
 *               affects calls to inu_ckreq.  V1 is the default and is not 
 *               specifiable by user (ie. only used internally).
 * X             expand filesystem as needed
 * z blocks      block siZe for restore
 */

/* Define the MACRO's */

#define SEPERATOR   ', '
#define INU_ALL     "all"
#define UPDATEP     "updatep"
#define INSTALLP    "installp"
#define INSTUPDT    "instupdt"
#define INSTLCLIENT "instlclient"

/* Define the valid argument lines */

#define MAX_ARGS        104
#define INSTALLP_ARG    "?AaBbCcDd:e:Ff:gIiJLlNO:kPpQqRrsT:t:UuV:vXxz:"
#define INSTLC_ARG      "?AaBbCcDd:e:Ff:gIiJlNO:kPpQqRrsT:t:UuvXxz:"

#define TDIR_GUARD	"inu_lock_t"	/* alternate save directory lock */

/* functions local to this file */
static int inu_check_tflag (char *dirpath);          /* verify tflag dirpath */
static int inu_mk_tdirinfo ();          /* make id string in malloc'd memory */
static int inu_get_root_lpp_desc (lpp_t *);

/* Set globals */

struct tsavdir_id *inu_get_tdirinfo ();	/* return tsavdir_id */

int userblksize = -1;  /* default the user defined device block siZe to -1 we will figure it out */
static struct tsavdir_id *Tp = &tdirinfo;  /* alternate save directory info */

extern int num_args;     /* number of arguments given on command line */
extern int lpp_optind;   /* NEEDED if installp re-execs itself        */

int inu_instl_args (
   int      sargc,          /* number of arguments passed to installp */
   char    *sargv[],        /* argument list passed to installp */
   char    *tmpdir)         /* pointer to the tmpdirectory */
{

   char    *ptr;            /* generic pointer */
   char    *blank;          /* pointer to first blank in a string */
   char     block_size[10]; /* the actual block size given */
   int      exclude;        /* counter for "INUEXCLU" error messages */
   FILE    *fd;             /* file pointer to LPPLIST */
   char    *first;          /* first character in a string */
   int      first_lpp;      /* flag for the first lpp given on command line */
   char     flag[2];        /* command line option flag */
   int      i;              /* string counter */
   int      invalid_arg;    /* counter for "INUINVAL" error messages */
   char     line[MAX_LPP_FULLNAME_LEN + MAX_LEVEL_LEN + 2];
                            /* the line to be added the the LPPLIST */
   int      lpps_given;     /* flag for lpps given on the command line */
   char    *name_level;     /* pointer to the next lpp on the command line */
   char    *next;           /* pointer to next string */
   extern   char *optarg;   /* pointer used by getopt () */
   extern   int  optind;    /* index used by getopt () */
   char    *output;         /* output of strcpy */
   char    *piece;          /* pointer to each piece/part of the -O option */
   int      rc;             /* return code from called functions */
   int      too_many;       /* counter for "INUTOOMN" error messages */
   int      assume_a;       /* true if the a flag was assumed */
   char     val_args[MAX_ARGS];  /* variable string for getopt */
   char     V_ARG[MAX_ARGS];/* buffer to temporarily hold value of V_flag */
   Level_t  level;          /* local var passed to inu_level_convert */
   int      action_flag=0;
   struct   stat statbuf;   /* file stat buffer */



/* verify that the orig_cmd is installp, instlclient, instupdt or updatep
 * other than these, we do not know what to do... */

    if ((ptr = strrchr (sargv[0], '/')) == NIL (char))
        strcpy (INU_COMMAND, sargv[0]);
    else {
        ptr++; /* skip past the "/" */
        strcpy (INU_COMMAND, ptr);
    }

    if (strcmp (INU_COMMAND, INSTALLP) != 0) { /* not installp */
        if (strcmp (INU_COMMAND, INSTLCLIENT) != 0) { /* not instlclient */
            if (strcmp (INU_COMMAND, INSTUPDT) != 0) { /* not instupdt */
                if (strcmp (INU_COMMAND, UPDATEP) != 0) { /* not updatep */
                    instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOTINSTL_E, 
                        INP_NOTINSTL_D), INU_COMMAND);
                    INU_ERR_COUNT++;
                    return (INU_ERR_COUNT);

                } /* end if (updatep) */
            } /* end if (instupdt) */
        } /* end if (instlclient) */
    } /* end if (installp) */


    /* set VAL_ARGS to the argument line we are using */

    /* if instlclient called this function */
    if (strcmp (INU_COMMAND, INSTLCLIENT) == 0)
        strcpy (val_args, INSTLC_ARG);
    else
        strcpy (val_args, INSTALLP_ARG);

    /* initialize variables */
    too_many = 0;        /* count of arguments used more than allowed */
    invalid_arg = 0;     /* count of invalid arguments used */
    exclude = 0;         /* count of mutually exclusive variables used */
    flag[1] = '\0';      /* set the second character of the string to \0 */
    output = NIL (char);  /* output from strcpy of arguments */
    lpps_given = FALSE;  /* flag for lpps on the command line */
    INU_ABORT_SEV = 0;   /* set the abort severity to the lowest possible */
    num_args = sargc;    /* set the number of arguments to argc */
    assume_a = FALSE;    /* we haven't assumed the a flag yet */

    /* create the TMPDIR/user.list file for the user provided list */
    strcpy (LPPLIST, tmpdir);
    strcat (LPPLIST, "/user.list");

    /* Parse the command line and start setting the flags */

    while ((flag[0] = (char) getopt (sargc, sargv, val_args)) != (char) EOF) {
        switch (flag[0]) {

            case 'A':   /* APAR listing */
                (void)inu_check_one (flag, &A_FLAG, too_many);
                break;

            case 'a':   /* Apply */
                (void)inu_check_one (flag, &a_FLAG, too_many);
                break;

            case 'B':   /* Updates only */

                (void) inu_check_one (flag, &B_FLAG, too_many);
                break;

             case 'b':   /* don't do bosboot */
                (void)inu_check_one (flag, &b_FLAG, too_many);
                break;

            case 'C':   /* Cleanup */
                (void)inu_check_one (flag, &C_FLAG, too_many);
                NEEDS_DEVICE = FALSE;
                break;

            case 'c':   /* Commit */
                (void)inu_check_one (flag, &c_FLAG, too_many);
                break;

            case 'D':   /* Remove bff */
                (void)inu_check_one (flag, &D_FLAG, too_many);
                break;

            case 'd':   /* Device */
                rc = inu_check_one (flag, &d_FLAG, too_many);
                if (rc == INUGOOD) { /* then copy the argument */
                    output = strcpy (DEVICE, optarg);
                    if (output == NIL (char)) /* then there was no argument */
                    {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOARG_E, 
                                                           INP_NOARG_D), "d");
                        invalid_arg++;
                    } /* end if (output == NULL) */
                } /* end if (rc == INUGOOD) */
                break;

             case 'e':   /* Logfile */
                rc = inu_check_one (flag, &e_FLAG, too_many);
                if (rc == INUGOOD) { /* then copy the argument */

                    output = strcpy (LOGFILE, optarg);
                    if ((output == NIL (char))  ||  (*LOGFILE == '-')) 
                                     /* then there was no argument */
                    {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOARG_E, 
                                                     INP_NOARG_D), "e");
                        invalid_arg++;
                    } /* end if (output == NULL) */

                } /* end if (rc == INUGOOD) */

                break;

            case 'F':   /* Force Flag */
                (void)inu_check_one (flag, &F_FLAG, too_many);
                break;

            case 'f':   /* Use file */
                if ( ! currently_execing)
                {
                   rc = inu_check_one (flag, &f_FLAG, too_many);

                  /* stat the argument to make sure it's a regular file.
                   * We want to avoid user errors where user uses "-f" 
                   * when he really meant to use "-d" */

                   if (strcmp (optarg, "-") != 0
                                   &&
                       (stat (optarg, &statbuf) != 0 ||
                        ! S_ISREG (statbuf.st_mode)))
                   {
                      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INVAL_E, 
                                           INP_INVAL_D), optarg, "f");
                      invalid_arg++;
                   }
                   else if (rc == INUGOOD)  /* then parse the arguments */
                   {
                      unlink (LPPLIST);
                      output = strcpy (USER_FILE, optarg);
   
                      if (output == NIL (char)) 
                      {
                         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_NOARG_E, INP_NOARG_D), "f");
                         invalid_arg++;
                      }
   
                      if ((rc = inu_get_list (USER_FILE, LPPLIST, 
                                              invalid_arg)) != INUGOOD)
                      {
                         /* Error creating the list of filesets */
                         break;
                      }
   
                      lpps_given = TRUE;
   
                   } /* else if */
                }
                break;

            case 'g':   /* Include */
                (void)inu_check_one (flag, &g_FLAG, too_many);
                break;

            case 'I':   /* Installs only */
                (void)inu_check_one (flag, &I_FLAG, too_many);
                break;

            case 'i':   /* Information Listing */
                (void)inu_check_one (flag, &i_FLAG, too_many);
                break;

            case 'J':   /* SMIT */
                (void)inu_check_one (flag, &J_FLAG, too_many);
                break;

            case 'k':   /* SMIT */
                (void)inu_check_one (flag, &k_FLAG, too_many);
                break;

            case 'L':   /* GUI Listing */
                 (void)inu_check_one (flag, &L_FLAG, too_many);
                 NEEDS_LPPS = FALSE;
                 break;

            case 'l':   /* Listing */
                (void)inu_check_one (flag, &l_FLAG, too_many);
                NEEDS_LPPS = FALSE;
                break;

            case 'N':   /* NO SAVE */
                (void)inu_check_one (flag, &N_FLAG, too_many);
                break;

            case 'O':   /* Optional parts */
                rc = inu_check_one (flag, &O_FLAG, too_many);
                if (rc == INUGOOD) { /* the parse the argument line */
                    first = strcpy (O_ARG, optarg);
                    O_ARG[strlen (O_ARG) +1] = '\0';

                    /* then -O was given w/o arguments */
                    if (O_ARG[0] == '\0') {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOARG_E, 
                                                     INP_NOARG_D), "O");
                        invalid_arg++;
                        break;
                    } /* end if (there are no arguments given) */

                    /* go through the argument list one at a time
                     *  and set each piece as you go */

                    for (i=0; O_ARG[i] != '\0'; i++)
                        switch (O_ARG[i]) {
                            case 'r' :
                                inu_check_one ("O r", &ROOT_PART, too_many);
                                break;
                            case 's' :
                                inu_check_one ("O s", &SHARE_PART, too_many);
                                break;
                            case 'u' :
                                inu_check_one ("O u", &USR_PART, too_many);
                                break;
                            default :
                                flag[0] = O_ARG[i];
                                instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                                         INP_INVAL_E, INP_INVAL_D), flag, "O");
                                invalid_arg++;
                                break;

                        } /* end switch (piece) */

                    if (ROOT_PART  &&  ! SHARE_PART  &&  ! USR_PART){ 
                        if ((inu_putenv ("ROOT_ONLY_OP=y")) != SUCCESS)
                            inu_quit (INUSETVAR);
                    }
                } /* end the parsing of the command line */
                break;

            case 'p':   /* preview */
                (void)inu_check_one (flag, &p_FLAG, too_many);
                break;

            case 'Q':   /* Suppress instreqs */
                /*
                 * No usage checks for Q flag since it's an undocumented
                 * flag.
                 */
                Q_FLAG = TRUE;
                break;

            case 'P':   /* Match */
                (void)inu_check_one (flag, &P_FLAG, too_many);
                LPP_ALL = TRUE;
                break;

            case 'q':   /* Quiet */
                (void)inu_check_one (flag, &q_FLAG, too_many);
                break;

            case 'R':   /* linked to -r */
            case 'r':   /* Reject */
                (void)inu_check_one (flag, &r_FLAG, too_many);
                NEEDS_DEVICE = FALSE;
                break;

            case 's':   /* Status Listing */
                (void)inu_check_one (flag, &s_FLAG, too_many);
                NEEDS_DEVICE = FALSE;
                break;

            case 'T':   /* Trace */
                strcpy (TRACE_ARG, optarg);

                if (TRACE_ARG[0] == 'k') {  /* if the argument was a k */
                    if (NO_CLEANUP) {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ONE_E, 
                                                     INP_ONE_D), "-Tk");
                        too_many++;
                    } /* end if no cleanup already set */

                    NO_CLEANUP = TRUE;
                } /* end if argument was a k */
                else
                {
                   /* the argument was not a 'k' */
                   instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INVAL_E, 
                                        INP_INVAL_D), TRACE_ARG, "T");
                   invalid_arg++;
                }
                break;

            case 't': /* set alternate directory path for saving files */
		if (inu_check_one (flag, &t_FLAG, too_many) != INUGOOD  || 
		    inu_check_tflag (optarg) != INUGOOD) {
                  invalid_arg++;
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_T_I, 
                                                            INP_USAGE_T_D));
		}
                break;

            case 'U':   /* Instupdt UPDATES only */
                if (strcmp (INU_COMMAND, INSTUPDT) == 0) { /* change it to -B */
                    if (B_FLAG)
                    {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ONE_E, 
                                                     INP_ONE_D), "B");
                        too_many++;
                        break;
                    }
                    break;
                } /* end if (instupdt -U) */

                else { /* we have an error */
                    invalid_arg++;
                    break;
                } /* end else there is an error */

            case 'u':      /* Deinstall */
                (void) inu_check_one (flag, &u_FLAG, too_many);
                NEEDS_DEVICE = FALSE;
                break;

            case 'V':   /* Verbosity */
                /*
                 * Need to take care not to call inu_check_one () if the
                 * default value of V_FLAG is currently set since that shows up
                 * as a "TRUE" inside inu_check_one () and causes incorrect
                 * message and processing to result.
                 * (Note: 1. User cannot specify V1.  2. It's still possible to
                 * specify V0 with V2 and not be caught by inu_check_one ().)
                 */
                if (V_FLAG != V1)
                   rc = inu_check_one (flag, (int *) &V_FLAG, too_many);
                else
                   rc = INUGOOD;

                if (rc == INUGOOD)
                { /* parse the argument line */
                   output = strcpy (V_ARG, optarg);
                   if (output == NIL (char)) { /* then there was no argument */
                      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOARG_E, 
                               INP_NOARG_D), "V");
                      invalid_arg++;
                   } else {
                      if (isdigit (*V_ARG)) {
                          V_FLAG = atoi (V_ARG);
                          if (V_FLAG < V0  ||  V_FLAG == V1  ||  V_FLAG > V4) 
                          {
                             instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                                         INP_INVAL_E, INP_INVAL_D), V_ARG, "V");
                             invalid_arg++;
                          }
                      } else {
                          instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INVAL_E,
                                                      INP_INVAL_D), V_ARG, "V");
                          invalid_arg++;
                      }
                   } /* end if (output == NULL) */
                } /* parse the argument line */
                break;

            case 'v':   /* Verify */
                (void)inu_check_one (flag, &v_FLAG, too_many);
                break;

            case 'X':   /* Expand */
                (void)inu_check_one (flag, &X_FLAG, too_many);
                break;

            case 'x':   /* Removed in v3.2 due to destructive nature */
                instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, INP_REM_XFLAG_W, 
                                                   INP_REM_XFLAG_D));
                break;

            case 'z':   /* block siZe */
                rc = inu_check_one (flag, &z_FLAG, too_many);
                if (rc == INUGOOD) { /* then copy the argument */
                    output = strcpy (block_size, optarg);
                    if (output == NIL (char)) /* then there was no argument */
                    {
                        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOARG_E, 
                                                    INP_NOARG_D), "z");
                        invalid_arg++;
                    } /* end if (output == NULL) */
                    else /* set BLOCK_SIZE and userblksize */
                    {
                        BLOCK_SIZE = atoi (block_size);
                        userblksize = BLOCK_SIZE;
                    } /* end else (there was an argument) */
                } /* end if (rc == INUGOOD) */
                break;

            case '?':
                /* output the entire usage message structure */
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_A_AC_I, 
                                                          INP_USAGE_A_AC_D));
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_C_R_U_I, 
                                                          INP_USAGE_C_R_U_D));
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_CL_I, 
                                                          INP_USAGE_CL_D));
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_L_I, 
                                                          INP_USAGE_L_D));
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_S_I, 
                                                          INP_USAGE_S_D));
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_A_I_I, 
                                                          INP_USAGE_A_I_D));

            default:    /* Catch all */
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_LINE_E, 
                                                          INP_LINE_D));
                inu_quit (INUINVAL);

        } /* end switch (flag) */

    } /* end while (getopt) */

   /*------------------------------------------------------- *
    * The function inu_exec_installp is critically dependant
    * upon lpp_optind getting set to the index of the 1st
    * lpp (on the cmd line).
    *------------------------------------------------------- */

    lpp_optind = optind;

    /* If -O not set then set all the parts to true as the default */
    if ( ! O_FLAG)
    {
        USR_PART = TRUE;
        ROOT_PART = TRUE;
        SHARE_PART = TRUE;
    }

    /* Determine if NEEDS_DEVICE should be FALSE based on the given flags */
    if (( ! SHARE_PART  &&  ! USR_PART)  ||  ( ! a_FLAG  &&  c_FLAG))
        NEEDS_DEVICE = FALSE;

    /* Unset default device if NEEDS_DEVICE = FALSE */
    if ( ! NEEDS_DEVICE)
       strcpy (DEVICE, "");
    else if ( ! d_FLAG)
       d_FLAG = TRUE; /* turn on the d_FLAG to use the default device */


  /*---------------------------------------------------------------------- *
   * Up-front validation for Preview (p flag) and Verbosity (V flag).
   *---------------------------------------------------------------------- */

   action_flag = (a_FLAG || c_FLAG) + r_FLAG + u_FLAG;

   if (p_FLAG)
   {
      switch (action_flag)
      {
         case 0:
           instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PREV_AND_MAJOR_FLAG_E, 
                                        INP_PREV_AND_MAJOR_FLAG_D));
           invalid_arg++;
           return (FAILURE);
           break;

        /*---------------------
         * The only valid case.
         *--------------------- */
         case 1:
           break;

         default:
           instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ONE_MAJ_PREV_FLAG_E, 
                                        INP_ONE_MAJ_PREV_FLAG_D));
           invalid_arg++;
           return (FAILURE);
           break;
       }
    }

    /*-----------------------------------------------------------------
     * Error off if a verbosity flag was specified (would have to be 
     * something other than V1) and there was more than one major action
     * flag given (major actions flags are -a, -ac, -c, -r, and -u.
     *-----------------------------------------------------------------*/
    if (V_FLAG != V1  &&  action_flag != 1)
    {
       instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_VERBOSITY_USAGE_E, 
                                                 INP_VERBOSITY_USAGE_D));
       invalid_arg++;
       return (FAILURE);
    }

    /* Parse lppname list if there are options listed at the end of the line */
    if ((fd = fopen (LPPLIST, "a")) == NIL (FILE)) 
    {
        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                     CMN_CANT_OPEN_D), INU_COMMAND, LPPLIST);
        return (FAILURE);
    }

    if (optind != sargc) { /* if there are more arguments then */
       if (NEEDS_LPPS  &&  (f_FLAG || C_FLAG || P_FLAG)) {
          /* then these arguments are an error */

          if (f_FLAG)
             instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ALL_EX_E, 
                                                       INP_ALL_EX_D), 'f');
          if (P_FLAG)
             instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ALL_EX_E, 
                                                       INP_ALL_EX_D), 'P');
     /*   if (C_FLAG)
             instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ALL_EX_E, 
                                                       INP_ALL_EX_D), 'C'); */
             /* we have decided to ignore this and not give an error.
                if it is decided that this should be and error some time
                in the future, all that needs to be done is uncomment the
                above line, remove the "if ( ! C_FLAG)" (but leave the
                guts of the "if ( ! C_FLAG)" around and delete this
                comment altogether. */

          if ( ! C_FLAG)
             exclude++;

       } /* end if (we already have a lpplist) */

       else {  /* the options at the end of the line are valid */
         lpps_given = TRUE;
         first_lpp = TRUE;

         for (;optind != sargc;optind++) {
            /* cycle through the remaining arguments */
            name_level = sargv[optind]; /* get the next argument */

            if (strcmp (name_level, INU_ALL) == 0) {
               LPP_ALL = TRUE;
               first_lpp = FALSE;
               strcpy (line, INU_ALL);
               continue;
            } /* end if (all) */

            if (isdigit (name_level[0])) {
               /* is this a level for the last option */
               if (first_lpp) { /* then we have an error */
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_LEVEL_E, 
                          INP_LEVEL_D), name_level);
                  too_many++;
               } /* end if (first_lpp) */

               else if (inu_level_convert (name_level,&level) == FAILURE)
               {
                  instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_LEVEL_NUM_E, 
                              CMN_LEVEL_NUM_D), INU_INSTALLP_CMD, name_level);
                  invalid_arg++;
               }
               else if (u_FLAG  &&  (strcmp (level.ptf, "") != 0))
               {  /* user has explicitly specified a ptf to be deinstalled */
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_DEINST_PTF_E,
                                         INP_NO_DEINST_PTF_D), name_level);
                  invalid_arg++;
               }
               else
               {  /* the level length is okay */
                  /* The indentation level exceeds the page width. */
                  /* Resetting to the left page margin. */
                  fprintf (fd, "%s %s\n", line, name_level);
                  /* Restoring indentation level to original setting. */
               }  /* end else (do the strcat) */

               *line='\0';
               first_lpp = TRUE;
               continue;
            } /* end if (isdigit == TRUE) */

            if (( ! first_lpp)  &&  (line[0] != '\0'))
               fprintf (fd, "%s\n", line);

            if (strlen (name_level) > MAX_LPP_FULLNAME_LEN) {
               instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INVAL_LPP_E, 
                                            INP_INVAL_LPP_D), name_level);
               invalid_arg++;
            } /* end if (lpp name too big) */

            else { /* this is a valid lppname as far as I know */
               strcpy (line, name_level);
               first_lpp = FALSE;
            } /* end else (valid lppname) */

         }  /* end for (cycle through the remaining arguments) */

         if (line[0] != '\0')
            fprintf (fd, "%s\n", line);

      } /* end else (the options are valid) */
   } /* end if (there are options at the end of the line) */

   /* else validate that we do have a argument list if needed */
   if (NEEDS_LPPS  &&  ! lpps_given) { /* then we still need a list */

      if (P_FLAG || C_FLAG || s_FLAG) { 
         /* put "all" into /tmp/inu_list */
         fprintf (fd, "%s\n", INU_ALL);
         LPP_ALL = TRUE;
      }

      else if (! f_FLAG) {  /* else we have a problem */
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOLIST_E, INP_NOLIST_D));
         invalid_arg++;
      }

   } /* end if (we need a list of lpps) */

   if (lpps_given  &&  (l_FLAG || L_FLAG)) {
      if (l_FLAG)
        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ALL_EX_E, 
                                                  INP_ALL_EX_D), 'l');
      else
        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ALL_EX_E, 
                                                  INP_ALL_EX_D), 'L');
      exclude++;
   } /* end if (we already have a lpplist) */

   fclose (fd);

   /* VALIDATE THE COMBINATIONS */
   exclude = inu_valid_args (&assume_a) + exclude;

   /*---------------------------------------------------------------- *
    * If we're deinstalling or "using force" we only deal with install 
    * images.
    *---------------------------------------------------------------- */
    if (u_FLAG  ||  F_FLAG)
       B_FLAG = FALSE;

    /* Check the error counts and terminate if necessary */
    if (too_many  ||  exclude  ||  invalid_arg)
    {
        /* then some where we encountered an error condition */
        if ( ! J_FLAG )
           (void) correct_usage ();
        INU_ERR_COUNT = too_many + exclude + invalid_arg + INU_ERR_COUNT;
        return (INU_ERR_COUNT);

    }

    if (assume_a)
       instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_A_ASSUMED_W, 
                                                 INP_A_ASSUMED_D));

    /* indicate to open_toc that we don't want to do an chdev */
    if(NEEDS_LPPS == FALSE)
    {
	extern int no_chdev;
	no_chdev=1;
    }
    return (INUGOOD);

}  /* end inu_instl_args */


/*------------------------------------------------------------------------
 * NAME: inu_check_one (cur_flag, cur_option, err_count)
 *
 * FUNCTION: This function will parse the command line for installp
 *           and fill the "opt" structure.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS: The "opt" structure with the flags used set.
 *
 *-----------------------------------------------------------------------*/

int inu_check_one (
char    *cur_flag,      /* the current flag used in the case statement */
int     *cur_option,    /* the "opt" flag being set */
int     err_count)      /* count of errors */

{

    if (*cur_option == TRUE) {
        instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ONE_E, INP_ONE_D), 
                                     cur_flag);
        err_count++;
        return (err_count);
    }

    /* else the flag was not previously used */
    *cur_option = TRUE;
    return (INUGOOD);
} /* end inu_check_one function */


/*------------------------------------------------------------------------
 * The -t flag specifies an alternate save dirpath.
 * When applying updates, the save directory is formed by appending
 * the default save directory to the alternate save dirpath.  
 * Only 1 dirpath is allowed per node.  A guard file is created in
 * the alternate save dirpath.  An identification string is placed
 * into the file.
 *
 * If the identification string is not found in the VPD, it is created
 * and save in the VPD.
 * In the VPD, the id string looks like this:
 *   {status} {use_caution} {allow_path_chg} {time_in_ms} {machine_id} {dirpath}
 * where status is initialized or path has been used, 
 *       use_caution is machine id does not match cpu or OK, 
 *       allow_path_change is dirpath can change or no, an apply uses path, 
 *       time_in_ms is current time in milliseconds when id string created, 
 *       machine_id is the id returned in uname.machine (the CPU planar id), 
 *       dirpath is the alternate save location specified with the -t flag.
 *      
 * The identification string is created once except if the machine id changes
 * and no alternate save location is in use, then the id string is re-generated.
 * If multiple hosts are created from a mksysb tape, then the unique id
 * won't be unique across the network.
 * The best we can do is recreate the id string if the machine part changes
 * and no alternate save directory appears to be in use.
 */

/*------------------------------------------------------------------------
 * NAME: inu_check_tflag (dirpath)
 *
 * FUNCTION: verify that the specified directory is accessible
 *	     and save it away.
 *
 * EXECUTION ENVIRONMENT: Part of user command.  Since a directory path
 *			  can be part of a remote filesystem, this
 *			  process may wait for a network timeout
 *			  if a remote system fails to respond.
 *
 * RETURNS: INUGOOD on success, else FAILURE
 *
 *-----------------------------------------------------------------------*/
static int
inu_check_tflag (char *dirpath)
{
	int	    rc = FAILURE;	/* until proven good */
	struct stat statbuf;            /* for information about a file */
	char        sep[] = "/";	/* directory separator */
	char	    cwd[PATH_MAX + 1];  /* current directory path */
	char	    linkdir[PATH_MAX + 1];  /* where symbolic link points */

	/* Error conditions are detected after the switch statement.
	   if path is empty, do nothing and error out below.
	   if path begins with a /, then copy path.
	   if dirpath is a relative path (no leading /), 
	     get the current directory and append dirpath to it. */
	switch (*dirpath) {
	  case '\0':    /* fail, don't assume current directory */
	    break;
	  case '/':     /* full path specified */
	    if (strlen (dirpath) + 1 < sizeof (t_SAVE_DIR))
	      strcpy (t_SAVE_DIR, dirpath);
	    else
	      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
			  	      INP_PATH_TOO_BIG_E, INP_PATH_TOO_BIG_D));
	    break;
	  default:      /* Append relative path to current directory. */
	    if (getcwd (t_SAVE_DIR, sizeof (t_SAVE_DIR)) == NULL) {
		perror ("getcwd");
		t_SAVE_DIR[0] = '\0'; /* better safe */
	    } else {
		if (t_SAVE_DIR[strlen (t_SAVE_DIR)-1] == '/')
		  sep[0] = '\0';	/* even in /, don't append another / */
		if ((strlen (t_SAVE_DIR) + strlen (sep) +
		    strlen (dirpath) + 1) > sizeof (t_SAVE_DIR)) {
		  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PATH_TOO_BIG_E, 
					  INP_PATH_TOO_BIG_D));
		} else {
		  strcat (t_SAVE_DIR, sep);
		  strcat (t_SAVE_DIR, dirpath); /* append to cwd */
		}
	    }
	    break;
	}

	/* Path must be a directory and writeable.  For root the directory
	   is always writeable locally, when permissions allow on a remote
           filesystem, and never on a read-only filesystem.  Also, paths
	   beneath a read-only directory could be mounted writeable.

	   If the path is a symbolic link, the directory must already exist.
	   If inu_mk_dir can't create the path, it gives an error msg.

           If a path is specified, 
	     if the path does not exist make the directory and
	        double check that it is there (although inu_mk_dir did already).
	       if it's a directory, 
		 if it's writeable
		   get real path (cd to directory and call getcwd)     */
	if (t_SAVE_DIR[0] == '/') {
	  cwd[0] = '\0';
	  if (readlink (t_SAVE_DIR, linkdir, sizeof (linkdir)) > 0)
	    (void) inu_mk_dir (linkdir);
	    
	  if (inu_mk_dir (t_SAVE_DIR) == INUGOOD) { 	/* make dir */
	    if (stat (t_SAVE_DIR, &statbuf) == -1) {
	      perror ("stat");
	      instl_msg (FAIL_MSG, "\t%s\n", t_SAVE_DIR);
	    } else if ( ! (S_ISDIR (statbuf.st_mode))) { /* is a directory */
	      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
				   CMN_NO_MK_DIR_D), inu_cmdname, t_SAVE_DIR);
	    } else if (accessx (t_SAVE_DIR, W_ACC, ACC_SELF) == -1) {
	      /* accessx isn't detecting read-only remote filesystems
		 when file permissions have write access ! */
	      perror ("accessx");	/* warn if unwritable */
	      instl_msg (FAIL_MSG, "\t%s\n", t_SAVE_DIR);
            } else if (getcwd (cwd, sizeof (cwd)) == NULL) { /* save pwd */
	      perror ("getcwd");
	      instl_msg (FAIL_MSG, "\t%s\n", cwd);
	    } else if (chdir (t_SAVE_DIR) == -1) { /* change to dirpath */
	      perror ("chdir");
              instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                               CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, t_SAVE_DIR);
            /* now, getcwd will resolve any symbolic links to real path */
	    } else if (getcwd (t_SAVE_DIR, sizeof (t_SAVE_DIR)) == NULL) {
	      perror ("getcwd");	/* need this even after chdir */
	      instl_msg (FAIL_MSG, "\t%s\n", t_SAVE_DIR);
            } else if ( ! strcmp (t_SAVE_DIR, "/")) { /* avoid symlinks to self */
	      /* silently turn off t_FLAG if alternate path is / */
	      t_FLAG = FALSE;	/* alternate path is default path */
	      t_SAVE_DIR[0] = '\0';	/* unset, just to be sure */
              rc = INUGOOD;		/* OK */
	    } else {
	      Tp->tsavdir = t_SAVE_DIR;	/* initialize */
	      if (inu_mk_tdirinfo () == INUGOOD) {	/* allow 1 tsavdir */
                /* guard creates guard file, and chown it */
	        if (inu_tdir_guard (TRUE) == FAILURE) {
                  instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_TFLAG_LOCK_E, 
				          INP_TFLAG_LOCK_D), 
			  t_SAVE_DIR, t_SAVE_DIR, TDIR_GUARD);
	        } else
	          rc = INUGOOD;			/* OK */
              }
	    }
	  }

	  /* change back to original directory */
	  if (cwd[0]  &&  chdir (cwd) == -1) {
	    rc = FAILURE;
	    perror ("chdir");
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
			  	 CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, cwd);
	  }
        }

	if (rc != INUGOOD)
	  t_SAVE_DIR[0] = '\0';

	return rc;
} /* end of  inu_check_tflag function */


/*-------------------------------------------------------------------------
 * NAME: inu_tdir_guard ()
 *
 * FUNCTION: if no guard file, create it.  If a guard file, check allowed id.
 *
 * GOAL:  Create a guard file atomicly so that 1 node has control of a
 *        directory path.
 * OBSTACLE:  In AIX, NFS filesystem can be mounted, open and written, without
 *           the NFS portmap or rpc.lockd process available to support locking.
 *           Also, the current implementation of AFS lacks support for locking.
 * SOLUTION: None.  A small window exists where 2 nodes could appear to have
 *           control of the guard file. The code in this function appends the
 *           unique idstring to the guard file.  AFS and NFS do local caching.
 *           Getting the end of file offset and then writing won't be atomic
 *           over NFS. 
 *             A "correct" solution is to lock the file but that requires that
 *           the NFS lock daemon is running.  Attempting to lock without NFS
 *           running, causes the requesting process to hang in the kernel, which
 *           is  unacceptable. Also, AFS lacks a locking mechanism currently.
 *
 * NOTE: The guard file can be remote or local.  Instead of locking, the guard
 *      file is opened so that all writes are appended to the file.  Then the
 *      file is closed to flush the write to the remote system.  Then the id
 *      string is compared to the first line of the guard file.  This procedure
 *      still allows time for 2 nodes to believe they control the guard file on
 *      an NFS or AFS filesystem.
 *         To aid in identifying the node controlling the guard file, 
 *      the nodename is placed in the second line of the file.
 *
 *      Another approach, not used here, would be something like the following:
 *      Precede the idstring with an @ symbol.  If any file in the directory
 *      begins with an @ and doesn't match @idstring, disallow access.
 *
 * EXECUTION ENVIRONMENT: Part of user command. 
 *
 * RETURNS: INUGOOD on success, else FAILURE
 *
 *---------------------------------------------------------------------------*/
int 
inu_tdir_guard (int verbose)
{
        int             fd = -1;        /* file descriptor for lock file */
        int             n, readsize;    /* bytes read or written */
        int             rc = INUGOOD;   /* have the lock? */
        char            *buf;           /* malloc'd memory */
        char            *bufidstr;      /* get unique id into malloc'd buffer */
        struct utsname  u;              /* machine info */
        struct stat     stbuf;
	char	        cwd[PATH_MAX + 1];  /* current directory path */
	     
        /* check/create the guard file for the alternate directory */

	if (Tp->tsavdir == NULL  ||  Tp->tsavdir[0] == NUL)
	  return FAILURE;

	if (getcwd (cwd, sizeof (cwd)) == NULL)
	  perror ("getcwd");
        else if (uname (&u) == -1) { /* to form id string for guard file */
          perror ("uname");
	} else if (inu_mk_tdirinfo () == INUGOOD) {/* retrieve identification */
	  /* generate id string expected in the guard file */
          n = strlen (Tp->time) + strlen (Tp->machine_id) +
	      _SYS_NMLN + strlen (" \n\n") + 1;	  /* nodename + whitespace */
          if ((bufidstr = malloc (n)) == NULL) {
            perror ("malloc");
	  } else if (chdir (Tp->tsavdir) == -1) {
	    if (verbose == TRUE) {
	      perror ("chdir");
	      instl_msg (FAIL_MSG, "\t%s\n", Tp->tsavdir);
	    }
	  /* later, chdir to cwd before returning from function ! */
	  } else 
	    rc = INUGOOD;			/* passed errors ok */
	}
	if (rc == FAILURE)			/* detected error above */
	  return rc;

	rc = FAILURE;				/* until set good again */

        sprintf (bufidstr, "%s %s\n%s\n", Tp->time, Tp->machine_id, u.nodename);

	/* If no guard file, create it and write id string.
           Don't lock with inu_testsetlock because AFS lacks locking and for NFS
           portmap and rpc.lockd msut be running else locking deep sleeps ! */
        if ((fd = open (TDIR_GUARD, O_RDWR | O_CREAT | O_SYNC | O_APPEND, 
                       0644)) == -1) {
          perror ("open");		       /* caller displays path */
        } else if (stat (TDIR_GUARD, &stbuf) == -1) {
          perror ("stat");
	  (void) close (fd);
	  fd = -1;
        } else if (stbuf.st_size == 0) {        /* write the guard file */
          if (write (fd, bufidstr, strlen (bufidstr)) == -1)
            perror ("write");
          else if (close (fd) == -1)
            perror ("close");
          else if ((fd = open (TDIR_GUARD, O_RDONLY /* | O_NDELAY */, 0)) == -1)
            perror ("open");
          else if (stat (TDIR_GUARD, &stbuf) == -1) {
            perror ("stat");
	    (void) close (fd);
	    fd = -1;
	  }
        }

        /* read the contents of the guard file; see who controls this path */
        if (fd != -1) {
          /* read in enough to verify time, machine id, and nodename */
          readsize = (stbuf.st_size < n) ? stbuf.st_size : n;

          if ((buf = malloc (readsize+1)) == NULL) {
            perror ("malloc");
          } else if ((n = read (fd, buf, readsize)) == -1) {
            perror ("read");
          } else {
	    buf[n] = '\0';	/* null terminate */
	    if ( ! strcmp (buf, bufidstr)) {
	      if (chown (TDIR_GUARD, 0, 0) == -1) {
		if (verbose == TRUE)
		  perror ("chown");
	      } else
	        rc = INUGOOD;	/* caller should do a chown on file */
	    } else if (verbose == TRUE)
	      instl_msg (FAIL_MSG, "%s/%s:\n%s\n",Tp->tsavdir,TDIR_GUARD,buf);
	  }
	  (void) close (fd);
        }

	if (bufidstr)
	  free (bufidstr);
	if (buf)
          free (buf);

	/* chdir to cwd before  returning from function */
	if (cwd[0]  &&  chdir (cwd) == -1) {
	  perror ("chdir");
	  instl_msg (FAIL_MSG, "\t%s\n", cwd);
	  rc = FAILURE;
	}

	return rc;	/* on failure, caller displays message */
} /* end of inu_tdir_guard function */


/*-----------------------------------------------------------------------
 *                                                                     
 * NAME:  inu_mk_tdirinfo                                              
 *                                                                     
 * FUNCTION:                                                           
 *   This function creates the unique id info for the t_Flag           
 *   If id is not in the VPD, the id is created and saved in the VPD.  
 *                                                                     
 * NOTES:                                                              
 *   The id is in the description field of the lpp record named        
 *   "__SWVPD_CTL__" in the ROOT vpd.                                  
 *   If the description field is empty, the id is created.  The        
 *   string values, separated by a blank, from tdirinfo are copied     
 *   to a line buffer and that line is written to the VPD.             
 *                                                                     
 *   INPUT: none                                                        
 *                                                                     
 *   OUTPUT: error messages, id string saved in VPD, tdirinfo changed  
 *                                                                     
 * RETURN VALUE DESCRIPTION:                                           
 *   INUGOOD on sccess, FAILURE on error                               
 *                                                                     
 * EXTERNAL PROCEDURES CALLED:                                         
 *      gettimer, uname                                                
 *                                                                     
 *---------------------------------------------------------------------*/
static int
inu_mk_tdirinfo ()
{
    int			    rc = FAILURE;
    int			    ok_to_change = FALSE;   /* change save dirpath */
    char		    *none = "*";            /* no name */
    char		    tbuf[2*sizeof (long)+1]; /* time value */
    struct timestruc_t      cur_time;   	    /* current time */
    struct utsname          uname_buf;  	    /* system info */
    lpp_t                   lpp_rec;                /* VPD record for lpp */
    struct tsavdir_id       tdirinfo_tmp;

    tdirinfo_tmp = *Tp;	        		    /* save before get */

    if (uname (&uname_buf) == -1)
      perror ("uname");
    else {
      (void) inu_get_tdirinfo ();
      /* if the planar board is changed the machine value changes.
	 If mksysb tape is used to install many machines, they could have
	 the same id string.  Try to change the id string. */
      if (Tp->machine_id) {
	if (strcmp (Tp->machine_id, uname_buf.machine)  || 
	    (tdirinfo_tmp.time  &&  strcmp (Tp->time, tdirinfo_tmp.time)))
	  Tp->use_caution = T_USE_CAUTION;   /* machine id  or time mismatch */

	/* restrict -t to 1 alternate save location in use */
	if (Tp->tsavdir == NULL  ||  Tp->tsavdir[0] == NUL)
	  ok_to_change = TRUE;         /* when no path is already set */
	else if (tdirinfo_tmp.tsavdir  &&  tdirinfo_tmp.tsavdir[0]  && 
	         strcmp (Tp->tsavdir, tdirinfo_tmp.tsavdir)) {
          if (Tp->allow_path_change == T_NOCHANGE  || 
              inu_grep_for_applied_in_vpd () == INUGOOD) {
	    /* Deny request to use a different alternate save dirpath.
	       There are files saved in current alternate save dirpath by an
	       option that is applied but not yet committed or rejected.
               To use new dirpath, commit or reject options using tsavdir. */
            instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ONE_TDIR_E, 
                                 INP_ONE_TDIR_D), 
		                 Tp->tsavdir, 	  /* use this existing path */
                                 tdirinfo_tmp.tsavdir);  /* new path denied */
	    return rc;		              /* failure */
	  } else {
	    ok_to_change = TRUE;
	    if (inu_tdir_guard (FALSE) == INUGOOD) {    /* control old path? */
	      char *old_guard_file;

	      /* try to give up the old path */
              if (old_guard_file =
                  malloc (strlen (Tp->tsavdir) + strlen (TDIR_GUARD) + 2)) {
		sprintf (old_guard_file, "%s/%s", Tp->tsavdir, TDIR_GUARD);
                (void) unlink (old_guard_file);   /* try to give up old path */
		free (old_guard_file);
	      }
	    }
	  }
	}
      }

      /* set tdirinfo if not set or machine id or time has changed
	 and it's OK to change the save path.  If a save path is in use, 
	 then the machine id must remain unchanged.  It's difficult to detect
         2 hosts, created from the same mksysb image, with the same tsavedir. */
      if (Tp->machine_id == NULL  ||  ok_to_change == TRUE)
        if (gettimer (TIMEOFDAY, &cur_time) == -1)
          perror ("gettimer");
        else {
	  Tp->use_caution = T_OK;
          (void) sprintf (tbuf, "%8.8X", 
                          cur_time.tv_sec * 1000 + cur_time.tv_nsec / 1000000);
	  Tp->time = tbuf;
	  /* should always have a machine id, but just in case there's none */
	  Tp->machine_id = (uname_buf.machine[0] ? uname_buf.machine : none);
	  Tp->tsavdir = tdirinfo_tmp.tsavdir;
	  rc = inu_change_tdirinfo (); 			/* updates the VPD */
	  bzero (Tp, sizeof (*Tp));			/* force re-read */
	  if (rc == INUGOOD)
	    rc = (inu_get_tdirinfo () ? INUGOOD : FAILURE); /* reset Tp */
        }
    }

    if (Tp->machine_id)
      rc = INUGOOD;
    return (rc);
} /* inu_mk_tdirinfo */


/*---------------------------------------------------------------------------
 * NAME: inu_change_tdirinfo
 *
 * FUNCTION:  Update the VPD from the tflag info.
 *
 * RETURNS:   INUGOOD on success
 *            FAILURE on error; caller should zero *Tp
 *-------------------------------------------------------------------------*/
int
inu_change_tdirinfo ()
{
  int    rc = FAILURE;
  int    size;
  char   *s;         /* string pointer */
  char   space[]=" ";
  lpp_t  lpp_rec;    /* VPD record for lpp */

  if ( ! Tp->status)
    Tp->status = T_SINIT;                 /* initialize */
  if ( ! Tp->use_caution)
    Tp->use_caution = T_OK;               /* initialize */

  /* never allow the path to change, this will be relaxed 
     in the future when there will be a check for UPDATES
     in the APPLIED state */
  if ( ! Tp->allow_path_change)
    Tp->allow_path_change = T_ALLOW_CHG;  /* or T_NOCHANGE */

  /* concatenate status values, time, machine id and dirpath */
  size = sizeof (Tp->status)+1 + sizeof (Tp->use_caution)+1 +
	 sizeof (Tp->allow_path_change)+1;
  size += (Tp->time       ? strlen (Tp->time)+1       : 0);
  size += (Tp->machine_id ? strlen (Tp->machine_id)+1 : 0);
  size += (Tp->tsavdir    ? strlen (Tp->tsavdir)      : 0);
  size++;			/* for terminating NUL */
  if ((s = malloc (size)) == NULL)
    perror ("malloc");
  else {
    (void) sprintf (s, "%c %c %c ", 
		   Tp->status, Tp->use_caution, Tp->allow_path_change);
    if (Tp->time) {
      strcat (s, Tp->time);
      strcat (s, space);
      if (Tp->machine_id) {
        strcat (s, Tp->machine_id);
	strcat (s, space);
        if (Tp->tsavdir)
	  strcat (s, Tp->tsavdir);
      }
    }

    /* get lpp record, change description, update VPD */
    bzero (&lpp_rec, sizeof (lpp_rec));
    strcpy (lpp_rec.name, "__SWVPD_CTL__");
    if (inu_get_root_lpp_desc (&lpp_rec) != VPD_OK) {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                   inu_cmdname);
      free (s);
    } else {
      if (lpp_rec.description)
	free (lpp_rec.description);
      lpp_rec.description = s;
      if (vpdremotepath (VPD_ROOT_PATH) != VPD_OK  ||  vpdremote () != VPD_OK  
          ||  vpdchgget (LPP_TABLE, &lpp_rec) != VPD_OK)
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                                    CMN_VPDERR_D), inu_cmdname);
      else
	rc = INUGOOD;
      vpd_free_vchars (LPP_TABLE, &lpp_rec);     /* also free's s */
    }
  }
  return rc;
} /* end of inu_change_tdirinfo function */


/*----------------------------------------------------------------------------
 * NAME: inu_get_tdirinfo
 *
 * FUNCTION:  If tflag info is empty, get it from the VPD record.
 *
 * RETURNS:   Pointer to &tdirinfo.
 *            Data is valid if (tdirinfo.machine_id != NULL) 
 *---------------------------------------------------------------------------*/
struct tsavdir_id *
inu_get_tdirinfo ()
{
  char   *s;         /* string pointer, never free'd */
  lpp_t  lpp_rec;    /* VPD record for lpp */

  /* tdirinfo is in description in "__SWVPD_CTL__" record in lpp ROOT VPD */
  if ( ! (Tp->machine_id)) {
    bzero (&lpp_rec, sizeof (lpp_rec));
    strcpy (lpp_rec.name, "__SWVPD_CTL__");
    if (inu_get_root_lpp_desc (&lpp_rec) != VPD_OK)
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                   inu_cmdname);
    else {
      if (lpp_rec.description  &&  lpp_rec.description[0]  && 
          (s = strdup (lpp_rec.description))  && 
	  (*s == T_SINIT  ||  *s == T_SUSED)) {
        Tp->status = *s++;
        if (*s++ == ' '  &&  (*s == T_USE_CAUTION  ||  *s == T_OK)) {
	  Tp->use_caution = *s++;
	  if (*s++ == ' '  &&  (*s == T_NOCHANGE  ||  *s == T_ALLOW_CHG)) {
	    Tp->allow_path_change = *s++;
	    if (*s++ == ' '  &&  *s) {
	      Tp->time = s++;
	      for (; *s != ' '  &&  *s; s++);
	      if (*s == ' ') {
	        *s++ = '\0';
	        if (*s) {	/* must have machine id to be valid */
	          Tp->machine_id = s++;
	          for (; *s != ' '  &&  *s; s++);
	          if (*s == ' ') {
		    *s++ = '\0';
		    if (*s)
		      Tp->tsavdir = s;
		  }
	        } 
	      }
            }
          }
        }
      }
      vpd_free_vchars (LPP_TABLE, &lpp_rec);
    }
  }
  if ( ! Tp->machine_id) {      /* need status fields, time and machine_id */
    bzero (Tp, sizeof (*Tp));	/* wasn't valid */
  }
  return Tp;
} /* inu_get_tdirinfo */


/*---------------------------------------------------------------------------
 * NAME: inu_get_root_lpp_desc
 *
 * FUNCTION:  If the lpp description is empty, attempt to get the description
 *            from the vpd record.
 *
 * RETURNS:   VPD_OK on success, description may be NULL or an empty string, ""
 *            VPD_NOMATCH if no match in vpd
 *            VPD_SYS if a system error occurred or strdup failed
 *-------------------------------------------------------------------------*/
static int
inu_get_root_lpp_desc (lpp_t *lpp)
{
   int       rc = VPD_SYS;
   char      *s;

   if ((lpp->description == NULL  ||  *(lpp->description) == NUL)  && 
       (rc = vpdremotepath (VPD_ROOT_PATH)) == VPD_OK  && 
       (rc = vpdremote ()) == VPD_OK) {
     if ((s = strdup (lpp->name)) == NULL)
       perror ("strdup");
     else {
       (void) bzero (lpp, sizeof (*lpp));
       (void) strcpy (lpp->name, s);
       free (s);
       rc = vpdget (LPP_TABLE, LPP_NAME, lpp);
     }
   }
   if (lpp->description)
     rc = VPD_OK;
   return rc;
} /* end inu_get_root_lpp_desc function */


/*--------------------------------------------------------------------------
 * NAME: inu_grep_for_applied_in_vpd
 *
 * FUNCTION: search for an UPDATE in the applied state.
 *
 * RETURNS:  INUGOOD if an update is in the applied state
 *           FAILURE if no update is in the applied state
 *
 *-------------------------------------------------------------------------*/
int inu_grep_for_applied_in_vpd ()
{
   int      rc = INUGOOD;		/* assume something in applied state */
   prod_t   prod;

   bzero (&prod, sizeof (prod));
   strcpy (prod.lpp_name, "*");		/* search all for */
   prod.state = ST_APPLIED;		/* anything in the applied state */
   /* prod.cp_flag can be LPP_UPDATE, an update, or ..., for a product */
   
   /* on a natch more exact is to check if (prod.cp_flag &LPP_UPDATE) */
   if (vpdremotepath (VPD_ROOT_PATH) == VPD_OK  &&  vpdremote () == VPD_OK  && 
       vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE, &prod) == VPD_NOMATCH)
     if (vpdremotepath (VPD_SHARE_PATH) == VPD_OK  &&  
         vpdremote () == VPD_OK  && 
         vpdget (PRODUCT_TABLE,PROD_LPP_NAME | PROD_STATE,&prod) == VPD_NOMATCH)
       if (vpdremotepath (VPD_USR_PATH) == VPD_OK  &&  
           vpdremote () == VPD_OK  && 
           vpdget (PRODUCT_TABLE,PROD_LPP_NAME|PROD_STATE,&prod) == VPD_NOMATCH)
	 rc = FAILURE;			/* nothing in the applied state */

   if (rc == INUGOOD)
     vpd_free_vchars (PRODUCT_TABLE, &prod);
   return (rc);

} /* end inu_grep_for_applied_in_vpd function */
