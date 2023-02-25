static char sccsid[] = "@(#)11  1.38  src/bos/usr/sbin/install/installp/inu_valid_args.c, cmdinstl, bos411, 9428A410j 6/1/94 12:30:40";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_valid_args.c (installp)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <installp.h>
#include <instl_options.h>

#include <inuerr.h>
#include <ctype.h>
#include <unistd.h>
#include <strings.h>

#include <installpmsg.h>

static void mutex_check (int, int, char *, char *, int *);

/*-----------------------------------------------------------------------
 * Checks the mutual exclusion of the first two arguments.  If both
 * flags are set then an exclusion message will be printed using the
 * third and fourth args and the fifth arg, errors, will be incremented.
 *----------------------------------------------------------------------*/
static void mutex_check (int flag_var1,     /* 1st _FLAG variable */
                         int flag_var2,     /* 2nd _FLAG variable */
                         char * flag_char1, /* 1st actual char flag */
                         char * flag_char2, /* 2nd actual char flag */
                         int  * errors)
{
   if (flag_var1  &&  flag_var2)
   {
      ++*errors;
      if (J_FLAG  &&
          strcmp (flag_char1, "F") == 0  &&  strcmp (flag_char2, "g") == 0)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_EXCLUS_F_G_E,
                    INP_EXCLUS_F_G_D));
      else
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_EXCLUS_E, INP_EXCLUS_D),
                    flag_char1, flag_char2);
   }
}


/*-----------------------------------------------------------------------
 * NAME: inu_valid_args.c
 *
 * FUNCTION: This function will use the external instl_opt structure
 *           to determine if there are any invalid combinations of
 *           options used with the command call.
 *
 * EXECUTION ENVIRONMENT: Called inu_instl_args.c
 *
 * RETURNS: The return code is the number of errors found
 *----------------------------------------------------------------------*/

/*
 * Error Messages:
 *      The error messages used are defined in inu_extern.h
 */

#define INU_ALL "all"

int num_args; /* global for number of arguments entered on cmdline */

int inu_valid_args (assume_a)
int *assume_a;
{

   int apply_only = FALSE;       /* for apply only situation */
   int apply_commit = FALSE;     /* for apply and commit situation */
   int commit_only = FALSE;      /* for commit only situation */
   int reject_only = FALSE;      /* for reject only situation */
   int deinstall_only = FALSE;   /* for deinstall only situation */
   int install_only = FALSE;     /* for install images only */
   int update_only = FALSE;      /* for update images only */
   int actions = 0;              /* count of major action requested */
   int errors = 0;               /* count of errors found */

   int ROOT_PART_ONLY = (ROOT_PART  &&  ! SHARE_PART  &&  ! USR_PART);

   /* set *_only flags */
   if (a_FLAG && c_FLAG)
      apply_commit = TRUE;
   else if (a_FLAG)
      apply_only = TRUE;
   else if (c_FLAG)
      commit_only = TRUE;

   if (r_FLAG)  /* then reject_only */
      reject_only = TRUE;

   if (u_FLAG)  /* then deinstall_only */
      deinstall_only = TRUE;

   if (I_FLAG  &&  ! B_FLAG)
      install_only=TRUE;

   if ( ! I_FLAG  &&  B_FLAG)
      update_only=TRUE;

   /* test the the flags are set correctly for old command calls */
   /* instupdt -I implies -IacN and -U implies -aB */
   if (strcmp (INU_COMMAND, "instupdt") == 0) { /* then check -I */

      if (install_only  &&  ! a_FLAG)
      {
         instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, INP_INUPDT_MSG_W, 
                  INP_INUPDT_MSG_D), "I", "aI");
         a_FLAG = TRUE;
         apply_only = TRUE;
      }

      if (update_only  &&  ! c_FLAG)
      {
         instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, INP_INUPDT_MSG_W, 
                              INP_INUPDT_MSG_D), "U", "aB");
         a_FLAG = TRUE;
         apply_only = TRUE;
      }

   } /* end if (instupdt) */

   /* updatep implies -B (do the updates only) */
   if (strcmp (INU_COMMAND, "updatep") == 0) /* then set the -B flag */
      I_FLAG = FALSE;

   /* There should be only one of the following major function specified */
   /* installp -a or -ac or -c or -r or -R or -A or -i or -C or -s or -l */
   /* or -L                                                              */
   /* Increment the actions counter for each flag that is set to TRUE.   */
   actions = (a_FLAG || c_FLAG) + r_FLAG + u_FLAG + A_FLAG + i_FLAG +
             C_FLAG + s_FLAG + l_FLAG + L_FLAG;

   if (actions > 1) { /* then there is an error */
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_ACTION_E, INP_ACTION_D));
      errors++;
   } 

   if (actions == 0  &&  num_args > 1) { /* then default to a_FLAG */
      a_FLAG = TRUE;
      apply_only = TRUE;
      *assume_a = TRUE;
   }

   /* verify the mutual exclusion of the following flag combinations */
   mutex_check (F_FLAG, commit_only, "F", "c", &errors);
   mutex_check (F_FLAG, g_FLAG, "F", "g", &errors);
   mutex_check (F_FLAG, r_FLAG, "F", "r", &errors);
   mutex_check (F_FLAG, B_FLAG, "F", "B", &errors);
   mutex_check (F_FLAG, u_FLAG, "F", "u", &errors);
   mutex_check (B_FLAG, u_FLAG, "B", "u", &errors);
   mutex_check (B_FLAG, I_FLAG, "B", "I", &errors);

   /* If neither B_FLAG nor I_FLAG set, set both as default.
   ** This is counter-intuitive.  In the future I would like to
   ** create global variables for UPDATE_ONLY, INSTALL_ONLY, 
   ** and INSTALL_AND_UPDATE.  This will avoid the confusion of
   ** having to set two mutually exclusive flags, I_FLAG and B_FLAG, 
   ** below.
   */
   if ( ! B_FLAG  &&  ! I_FLAG)
   {
      B_FLAG = TRUE;
      I_FLAG = TRUE;
   }

   /* installp (-r or -u) and "all" */
   if ((r_FLAG  ||  u_FLAG)  &&  LPP_ALL  && 
       ( ! P_FLAG  ||  ! C_FLAG  ||  ! s_FLAG))
   {
      errors++;
      if (r_FLAG)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_EXCL_ALL_E, 
                                                   INP_EXCL_ALL_D), "r");
      else
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_EXCL_ALL_E, 
                                                   INP_EXCL_ALL_D), "u");
   }

   /* It was decided that the -N -t is an error because the user
      may really want to save replaced files but be confused about -N */
   mutex_check (N_FLAG, t_FLAG, "N", "t", &errors);

   /* installp -Ors is invalid */
   mutex_check (ROOT_PART && SHARE_PART, ! USR_PART, "Or", "Os", &errors);

   /* installp -Or can not have -d or -D */
   mutex_check (ROOT_PART_ONLY, d_FLAG, "Or", "d", &errors);
   mutex_check (ROOT_PART_ONLY, D_FLAG, "Or", "D", &errors);

   /* installp -Or and {-A or -I or -l or -L} */
   mutex_check (ROOT_PART_ONLY, A_FLAG, "A", "Or", &errors);
   mutex_check (ROOT_PART_ONLY, i_FLAG, "i", "Or", &errors);
   mutex_check (ROOT_PART_ONLY, l_FLAG, "l", "Or", &errors);
   mutex_check (ROOT_PART_ONLY, L_FLAG, "L", "Or", &errors);

   /* Force reapply of the USR part without the ROOT part not allowed */
   if (a_FLAG  &&  F_FLAG  &&  USR_PART  &&  ! ROOT_PART)
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_FORCE_APPLY_USR_I, 
                                                INP_FORCE_APPLY_USR_D));
   }

   /* installp -C and -Tk */
   mutex_check (C_FLAG, NO_CLEANUP, "C", "Tk", &errors);

   /* installp -q w/o -d */
   if (q_FLAG  &&  ! d_FLAG)
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_MUST_HAVE_E, 
                                   INP_MUST_HAVE_D), "q", "d device");
   }

   /* installp -z w/o -d */
   if (z_FLAG  &&  ! d_FLAG)
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_MUST_HAVE_E, 
                                   INP_MUST_HAVE_D), "z", "d device");
   }

   /* installp -N w/o -ac */
   if (N_FLAG  &&  ! apply_commit)
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_AN_INVAL_E, 
                                                INP_AN_INVAL_D));
   }

   /* installp -d and -C or -s or -r or -R or -u or (-c w/o -a) */
   if (d_FLAG  && 
       (C_FLAG || s_FLAG || reject_only || deinstall_only || commit_only))
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                INP_BAD_USE_D), "d");
   }

   /* installp -D  &&  
    *          (-c or -r or -R or -u or -C or -i or -l or -L or -s or -A) */
   if (D_FLAG  && 
       (commit_only || reject_only || deinstall_only || C_FLAG || i_FLAG || 
        l_FLAG || L_FLAG || s_FLAG || A_FLAG))
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                INP_BAD_USE_D), "D");
   }

   /* installp (-F or -g or -Tk) and (-C or -i or -l or -L or -s or -A) */
   if ((F_FLAG || g_FLAG || NO_CLEANUP)  && 
       (C_FLAG || i_FLAG || l_FLAG || L_FLAG || s_FLAG || A_FLAG))
   {
      errors++;
      if (F_FLAG)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                   INP_BAD_USE_D), "F");
      if (g_FLAG)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                   INP_BAD_USE_D), "g");
      if (NO_CLEANUP  &&  ! C_FLAG)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                   INP_BAD_USE_D), "Tk");
   }

   /* installp (-P or -q) and (-c w/o -a) or -r or -R or -u or -s or -C */
   if ((P_FLAG || q_FLAG)  && 
       (commit_only || reject_only || deinstall_only || s_FLAG || C_FLAG))
   {
      errors++;
      if (P_FLAG)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                   INP_BAD_USE_D), "P");
      if (q_FLAG)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                   INP_BAD_USE_D), "q");
   }

   /* installp -v and (-i or -l or -L or -s or -A) */
   if (v_FLAG  && 
       (i_FLAG || l_FLAG || L_FLAG || s_FLAG || A_FLAG))
   {
      errors++;
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_USE_E, 
                                                INP_BAD_USE_D), "v");
   }

   if ( ! a_FLAG  &&  t_FLAG) 
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_MUST_HAVE_E, 
                                                INP_MUST_HAVE_D), "t", "a");
      errors++;
   }

   return (errors);

} /* inu_valid_args */
