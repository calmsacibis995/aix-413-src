static char sccsid[] = "@(#)89  1.36.1.58  src/bos/usr/sbin/install/installp/inu_bld_sop.c, cmdinstl, bos41J, 9510A_all 2/27/95 12:13:26";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_bld_sop, inu_print_pulled_in_msg, 
 *            mv_F_ptfs_from_sop_to_savesop, inu_parse_supersedes, 
 *            inu_ck_tsavdir, 
 *            inu_rm_opt_from_sop, inu_rm_2ndary_dups, inu_rm_dups_from_sop, 
 *            inu_mark_dups_in_toc, inu_check_parts, 
 *            mark_toc_selected_list_as_nil, inu_rm_dup_updates_from_sup, 
 *            set_operation, remove_bos_rte, if_install, 
 *            pull_in_ptfs_from_toc
 *
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <installp.h>
#include <instl_define.h>
#include <instl_options.h>

extern int cmd_line_prod_fail; /* Global variable to indicate failure to
                               install products or updates input from
                               command line */

#ifdef INUDEBUG
extern FILE    *dfp;

#endif  /* INUDEBUG */

extern char    *inu_cmdname;

Option_t *toc_hashlist[TOC_HASH_SIZE];           /* Actual hash table for toc */
int       hashed_the_toc=FALSE;    /* TRUE if toc's been hashed, FALSE if not */
static    char oem_system_name[MAX_LANG_LEN+1];   /* OEM system name */
static    int oem_system = 0;     	   	  /* OEM-proprietary system */

/**
 ** Added static prototypes (these functions are local only to the
 ** routines in this module).
 **/

static int inu_parse_supersedes          (Option_t *);
static int inu_check_parts                (Option_t *);
static void remove_bos_rte                (Option_t *, Option_t *);
static int if_install                     (Option_t *);
static int inu_bld_sop_toc                (TOC_t *, Option_t *, char *, 
                                           Level_t *, Option_t *);

static void add_node_to_toc_hash_table (Option_t *op);
static void hash_the_toc               (Option_t *toc);
       int  get_toc_hash_value         (char *name);

/*--------------------------------------------------------------------------*
**
** NAME: inu_bld_sop ()
**
** FUNCTION:
**  Read the $INUTEMPDIR/user.list and add it to the selected option list.
**
** RETURN VALUE DESCRIPTION:
** SUCCESS - if we were successful enough to continue.
** FAILURE - if we encountered a bad enough error to quit.
**
**
**--------------------------------------------------------------------------*/

int inu_bld_sop
 (TOC_t    * toc,            /* Ptr to the top of the toc options list      */
  Option_t * sop,            /* Ptr to the top of the Selected options list */
  Option_t * savesop,        /* Ptr to the top of the save sop list         */
  Option_t * failsop)        /* Ptr to the top of the save sop list         */
{
   boolean         error = FALSE;
   Level_t         level;       /* Struct to store the levels of user
                                 * requested option. */
   int             rc=0;        /* generic return code. */
   int             irc;         /* return code for 'F' ptfs call to inu_ckreq */
   short           tree;        /* bit or'ed short to pass to inu_ckreq which
                                   trees the user specified (-O urs).   */
   char           *rc_p;        /* generic return code of char pointer */
   FILE           *fp;          /* file pointer to the user_list file */
   int             i;           /* generic counter */
   char            i_level[MAX_LEVEL_LEN];    /* buffer for level from user */
   char            buf[TMP_BUF_SZ];           /* generic buffer */
   char           *p;           /* temporary char pointer...    */
   Option_t       *op;          /* Option pointer                           */
   Option_t       *prev_op;     /* temporary previous Option pointer        */
   Option_t       *tmpsavesop;  /* temporary sop ptr                        */
   Option_t       *save_failsop = NIL (Option_t);
                                /* Another temp ptr                         */
   Option_t       *top;         /* temporary Option pointer                 */
   boolean        all_keyword = FALSE;  /* indicates if "all" was specified */
   int            num_F_ptfs_found=0;      /* number of installp ptfs found */
   stats_table    pip_stats_data; /* pre-installation processing stats data */


   cmd_line_prod_fail = FALSE;

   /* Check to see if this is an OEM-proprietary system */
   if ((fp = fopen ("/usr/lib/objrepos/oem", "r")) != NIL (FILE)) {
       oem_system = 1;
       fgets(&oem_system_name, MAX_LANG_LEN+1, fp);
       strtok((char *) &oem_system_name," \n");
   }

   if (toc != (TOC_t *) NULL)
   {
      for (op = toc->options->next, prev_op = toc->options, rc=SUCCESS;
           op != (Option_t *) NULL && rc==SUCCESS;
           prev_op = op, op = op->next)
      {
         
        /** --------------------------------------------------- *
         **  If the image is an "O" type, like the bos.rte
         **  image, then we want to throw it out of the toc
         **  list so inu_ckreq won't see it.
         ** --------------------------------------------------- */

         if (op->bff->action == ACT_OTHER) 
         {
            prev_op->next = op->next;
            inu_free_op(op);
            op = prev_op;
            continue;
         }


         if (IF_3_2(op->op_type) || IF_3_1(op->op_type))
            rc = inu_parse_supersedes (op);

        /** --------------------------------------------------- *
         **  For 4.1 or later base levels,
         **  parse the data in the supersedes field -- could be
         **  barrier info, compatability info, or a mixture.
         ** --------------------------------------------------- */

         else if (IF_INSTALL(op->op_type))
            rc = inu_parse_post_32_supersedes (op);

      }

      if (rc != SUCCESS)
         return (FAILURE);
   }

   /*----------------------------------------------------------------------*
    * Build user_list file path and open the file...
    *----------------------------------------------------------------------*/

   strcpy (buf, INU_TMPDIR);
   strcat (buf, "/user.list");
   if ((fp = fopen (buf, "r")) == NIL (FILE))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                   CMN_CANT_OPEN_D), INU_INSTALLP_CMD, buf);
      return (FAILURE);
   }

   /*------------------------------------------------------------------*
    * Zero out i_level prior to doing the fscanf.
    *------------------------------------------------------------------*/

   (void) memset (i_level, '\0', sizeof (i_level));

   /*----------------------------------------------------------------------*
    * 1: While there are still lines in the user_list file
    * 2:   Read line from the user list file
    * 3:   Call level_convert () to convert the ascii level strings to a
    *      structure of integers
    * 4:   Call one of the inu_bld_sop_xxx () sub-routines depending on the
    *      the type of operation being performed. The ".toc" routine deals
    *      with media and the ".vpd" deals with the vpd for the option named.
    *----------------------------------------------------------------------*/

   while ((rc_p = fgets (buf, sizeof (buf), fp)) != NIL (char))
   {

      /*------------------------------------------------------------------*
       * search for the end of the name field and null terminate it.
       *------------------------------------------------------------------*/

      for (i = 0; buf[i] != ' '  &&  buf[i] != '\n'  &&  buf[i] != '\t'; i++);
      buf[i] = '\0';
      i++;

      /*------------------------------------------------------------------*
       * search for the next non-whitespace which should be the level field
       * if the level field isn't there set level.ver to -1...
       *------------------------------------------------------------------*/

      while ((buf[i] == ' '  ||  buf[i] == '\t')  &&  buf[i] != '\0')
         i++;
      if (buf[i] == '\0')
      {
         level.ver = -1;
         level.ptf[0] = '\0';
      }
      else
      {
         /*--------------------------------------------------------------*
          * Else, we found another field, find its end and null terminate.
          *--------------------------------------------------------------*/

         p = &buf[i];
         while (buf[i] != ' '  && 
                buf[i] != '\n'  && 
                buf[i] != '\t'  && 
                buf[i] != '\0')
            i++;
         buf[i] = '\0'; /* null terminate the level field */

         /*----------------------------------------------------------*
          * Convert the i_level string into a struct of shorts
          *----------------------------------------------------------*/

         if ((rc = inu_level_convert (p, &level)) != SUCCESS)
         {
            if (toc->hdr_fmt == TOC_FMT_3_1)
               inu_31level_convert (p, &level);
            else
            {
               INU_ERR_COUNT++;
               instl_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_LEVEL_NUM_E, 
                                  CMN_LEVEL_NUM_D), INU_INSTALLP_CMD, i_level);
               continue;
            }
         }
      }

      /*---------------------------------------------------------------------
       * Can't specify a ptf with force apply.  This is only part of the 
       * check to make sure updates are not specified on force apply.  The 
       * other check occurs in inu_bld_sop_toc.
       *--------------------------------------------------------------------*/

      if (a_FLAG  &&  F_FLAG  &&  (level.ptf[0] != '\0'))
      {
         if ((strcmp (level.ptf, "all") != 0) &&
             (strcmp (level.ptf, "ALL") != 0))
         {
            rc = STAT_NO_FORCE_APPLY_PTF;
            inu_add_op_to_fail_sop (failsop, buf, &level, 
                                    STAT_NO_FORCE_APPLY_PTF, 
                                    NIL (char), "");
            continue;
         }
      }

      if (NEEDS_DEVICE)
      {
         if (strcmp (buf, "ALL")==0  ||  (strcmp (buf, "all")==0))
            all_keyword = TRUE;

         if ((rc = inu_bld_sop_toc (toc, sop, buf, &level, failsop)) != SUCCESS)
            return (rc);
      }
      else
      {
         if ((rc = inu_bld_sop_vpd (sop, buf, &level, failsop)) != SUCCESS)
            return (rc);
      }

      /*------------------------------------------------------------------*
       * Zero out i_level prior to doing the fscanf.
       *------------------------------------------------------------------*/

      (void) memset (i_level, '\0', sizeof (i_level));

   }

  /*----------------------------------------------------------------------*
   * After trying to build the Selected Option List, if no options were
   * found to process, then display a message that there's nothing to do.
   *----------------------------------------------------------------------*/

   if (sop->next == NIL (Option_t))
   {
      if (C_FLAG)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NOTHING_TO_CLEAN_ALL_E, 
                                                   INP_NOTHING_TO_CLEAN_ALL_D));
         return (SUCCESS);
      }
      else if (s_FLAG)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_SW_PRODS_IN_VPD_E, 
                                                   INP_NO_SW_PRODS_IN_VPD_D));
         return (FAILURE);
      }
      else if (failsop->next == NIL (Option_t))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PREINST_FAIL_E, 
                                                   INP_PREINST_FAIL_D));
         return (FAILURE);
      }
   }

   if (s_FLAG || i_FLAG || A_FLAG)
   {
      inu_sort_sop (sop, &error);
      if (error)
         return (FAILURE);
   }

   /*----------------------------------------------------------------------*
    * Call inu_ckreq () which is the library interface to the ckprereq command.
    * The third arg is the file ptr to the prereq file (which in this case
    * doesn't exist). The fourth arg is the verbose flag which we don't want
    * set if being called from here.
    * NOTE: Don't call inu_ckreq () if we're doing a cleanup...
    *---------------------------------------------------------------------*/


   if (a_FLAG || c_FLAG || r_FLAG || u_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INUCOMMON, CMN_DONE_I, CMN_DONE_D));

   if (C_FLAG)
   {
      if (sop->next == NIL (Option_t)  &&  failsop->next == NIL (Option_t))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PREINST_FAIL_E, 
                                                   INP_PREINST_FAIL_D));
         return (FAILURE);
      }
      return (SUCCESS);
   }

   tree = 0;
   if (ROOT_PART)
      tree = LPP_ROOT;
   if (USR_PART)
      tree |= LPP_USER;
   if (SHARE_PART)
      tree |= LPP_SHARE;

  /** ---------------------------------------------------------------------- *
   ** Keep going if we have anything on the regular sop OR the failsop.
   ** This allows inu_ckreq to try to resolve problems found on the failsop
   ** (ptf not on media but supersedes is) and to report failures found 
   ** in installp (which it does anyway).
   ** ---------------------------------------------------------------------- */

   if ((a_FLAG || r_FLAG || u_FLAG || c_FLAG)  && 
       (sop->next != NIL (Option_t) || failsop->next != NIL (Option_t)))
   {
      inu_mark_all_as_selected (sop);
      inu_mark_all_as_selected (failsop);

      if (a_FLAG)
      {
         inu_rm_dups_from_sop (sop, SOP_LL, toc, failsop);
         inu_mark_dups_in_toc (toc);

        /** --------------------------------------------------------- *
         **  Move all installp ptfs from the sop to the savesop.
         **  Then pull in all non-installed installp ptfs and required
         **  ptfs from the toc.
         ** --------------------------------------------------------- */

         num_F_ptfs_found = mv_F_ptfs_from_sop_to_savesop (sop, savesop);

         if (USR_PART || SHARE_PART)
         {
           num_F_ptfs_found += pull_in_ptfs_from_toc (toc, savesop, 
                                                      ACT_INSTALLP_UPDT);
           (void) pull_in_ptfs_from_toc (toc, sop, ACT_REQUIRED_UPDT);
         }

         /* ------------------------------------------------------------------ *
         **  If any uninstalled 'F' ptfs were found, call inu_ckreq ()
         **  with savesop -- which contains only 'F' ptfs (installp ptfs).
         **  If the call to inu_ckreq fails or if it comes back with nothing 
         **  on the sop, then we may need to save off the failsop.
         ** ------------------------------------------------------------------ */

         if (num_F_ptfs_found > 0)
         {
            /* Auto-Include is turned on for 'SF' PTFs. */

            if ((irc = inu_ckreq (savesop, failsop, toc, FALSE, TRUE, TRUE, 
                                  tree, FALSE)) == SUCCESS   &&  
                (savesop->next != NIL (Option_t)))
            {
              /** --------------------------------------------------------- *
               **  Since (later on) the sop is processed before the savesop, 
               **  force the sop to contain the installp ptf and its
               **  requisites, and the savesop to contain everything else.
               ** --------------------------------------------------------- */

               tmpsavesop    = sop->next;
               sop->next     = savesop->next;
               savesop->next = tmpsavesop;

              /** --------------------------------------------------------- *
               **  Rip out the stuff on the savesop that inuckreq just 
               **  dragged in to avoid duplicate processing.
               ** --------------------------------------------------------- */

               prune_dups_between_sops (sop, savesop);

               inu_calc_stats_table_values (sop, failsop, savesop, 
                                            &pip_stats_data);
               inu_show_stats_table (sop, failsop, savesop, &pip_stats_data);

               for (top=savesop->next; top != NIL (Option_t); top=top->next)
                  top->Status = STAT_CANCEL;

               return (SUCCESS);
            }

            /*
             * The call to inu_ckreq failed for the SF type ptfs.  Before
             * processing any remaining pkgs with a 2nd call to inu_ckreq
             * we need to save off the failsop so that it doesn't get 
             * reported again in the subsequent call.  After the 2nd 
             * call, we'll re-append the failsop.  Only do all this if 
             * there's something on the sop.  (We don't need to do this
             * for the success case since the failsop will be emtpy on
             * re-exec.) 
             */
            if (sop->next != NIL (Option_t))
            {
               save_failsop  = failsop->next;
               failsop->next = NIL (Option_t);
            }
         }
      } /* a_FLAG */

      if ((sop->next != NIL (Option_t))  ||  (failsop->next != NIL (Option_t)))
      {
         remove_bos_rte (sop, failsop);
         rc = inu_ckreq (sop, failsop, toc, FALSE, g_FLAG, FALSE, tree, FALSE);

        /*
         * We may have saved off entries on the failsop under a 
         * fairly rare situation -- see first call to inu_ckreq above.
         * Let's put them back at the beginning of the failsop.
         */

         if (save_failsop != NIL (Option_t))
         {
            for (op=save_failsop; op->next != NIL (Option_t); op=op->next)
            { ; } 

            /*
             * Prepend the saved failsop to the failsop.
             */
            op->next      = failsop->next;
            failsop->next = save_failsop;
         } 

         if (rc != SUCCESS)
            return (FAILURE);

         if (all_keyword == TRUE)
            cmd_line_prod_fail = FALSE;

         remove_bos_rte (sop, failsop);

         /* If rejecting, fail if a symbolic link to a missing save directory */
         if (r_FLAG  &&  (rc = inu_ck_tsavdir (sop)) != SUCCESS)
            return FAILURE;
      }
     /** --------------------------------------------------------------*
      **  Calculate values for the STATISTICS table then show it.
      ** --------------------------------------------------------------*/

      inu_calc_stats_table_values (sop, failsop, savesop, &pip_stats_data);
      inu_show_stats_table (sop, failsop, savesop, &pip_stats_data);
   }

   /*----------------------------------------------------------------------*
    * After calling inu_ckreq see if there is anything left on the sop
    * to process, if not, then display a message that there's nothing to do.
    *----------------------------------------------------------------------*/

   if (sop->next == NIL (Option_t)  &&  failsop->next == NIL (Option_t))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PREINST_FAIL_E, 
                                                INP_PREINST_FAIL_D));
      return (FAILURE);
   }

   return (SUCCESS);
}

/*--------------------------------------------------------------------------*
**
** NAME: inu_bld_sop_toc ()
**
** FUNCTION:Receives the name and level from the user_list of what the
**          user wants to install and searches the toc to try to find it
**          on the media. If it is found then its prereqs are checked, 
**          if the prereqs pass, then add this option to the Sop (Selected
**          Options List).  If a request is not in the TOC, it is put in
**          the failsop.  If the request is an update, inu_ckreq will attempt
**          to resolve the request with a superseding update.  Otherwise, 
**          the failed request is reported by inu_ckreq.
**
** NOTES:   This is done for doing /usr applys (-a), List customer info (-i), 
**          and to list APAR info (-A).
**
**
** RETURNS:  0 - If no errors are found OR
**           non-zero - a fatal error condition occurred.
**           The global variable INU_ERR_COUNT will keep a count of the
**           number of errors encountered.
**
**
**--------------------------------------------------------------------------*/

int inu_bld_sop_toc 
  (TOC_t    * toc,      /* Ptr to the top of the Toc linked list. */
   Option_t * sop,      /* Ptr to the top of the Selected * Option List */
   char     * i_name,   /* The name of the lpp the user * wishes to install */
   Level_t  * level,    /* The level of the lpp the user * wishes to install */
   Option_t * failsop)  /* Ptr to the top of the failure List */
{
   char      *tptr;             /* generic char pointer */
   char      buf[TMP_BUF_SZ];   /* generic char buffer */
   char      sv_name[TMP_BUF_SZ]; /* generic char buffer */
   Option_t  *op;               /* Option located in Option_t struct */
   int       found;             /* Indicates if we found option name in toc. */
   int       xfound;
   int       all_string;        /* Indicates if ".all" was specifed by user. */
   int       rc;                /* Generic return code      */
   char      levname[MAX_LPP_FULLNAME_LEN];
   int       failcode=0;
   int       levelrc=0;
   char      *savesedes;
   char      *sedes;
   int	     i_name_len;

   /*----------------------------------------------------------------------*
    * Save the name specified by the user, before we put regex stuff on it.
    *----------------------------------------------------------------------*/

   strcpy (sv_name, i_name);

   /*----------------------------------------------------------------------*
    * convert ALL or all to match all names
    *----------------------------------------------------------------------*/

   if (strcmp (i_name, "ALL") == 0  ||  (strcmp (i_name, "all") == 0))
   {
      all_string = 1;
      i_name_len=0;
   }
   else
   {
      /*------------------------------------------------------------------*
       * if name ends in .all remove it and replace with a req expression
       *------------------------------------------------------------------*/

      if (((tptr = strstr (i_name, ".all")) != NIL (char))  &&  
          (strlen (tptr) == 4))
      {
         *++tptr = '\0';
         all_string = 1;
      }
      else
      {
         if (((tptr = strstr (i_name, ".ALL")) != NIL (char))  &&  
             (strlen (tptr) == 4))
         {
            *++tptr = '\0';
            all_string = 1;
         }
         else /* Just add the name. */
         {
            all_string = 0;
	    strcat(i_name,".");
         } /* end if */
      } /* end if */

      i_name_len=strlen(i_name);

   } /* end if */

   if (! hashed_the_toc)
   {
      hash_the_toc (toc->options->next);
   }

   /*----------------------------------------------------------------------*
    * search for this option in the toc's linked struct of options.
    *----------------------------------------------------------------------*/

   for (found = 0, xfound = 0; ! found;)
   {
      op = grep_option_toc (toc->options, i_name, i_name_len, level, &found, 
                            sv_name);
      /*------------------------------------------------------------------*
       * if option wasn't found in the toc, then test to see (via found)
       * it at least found the name, if not, then IF the user specified ".all"
       * (via all_string) then give an error stating that no match was found, 
       * ELSE ( ! all_string) call grep_option_toc again but this time pass
       * it just the name (no regex strings) to see if the user specify the
       * full name of the lpp-options, if yes use it , else state that the
       * option wasn't found and break...
       *------------------------------------------------------------------*/

      if (op == NIL (Option_t))
      {
         if ( ! xfound)
         {
            if (all_string)
            {
               if ((inu_add_op_to_fail_sop (failsop, sv_name, level, 
                    STAT_NOTHING_FOUND_ON_MEDIA, NIL (char), "")) != SUCCESS)
               {
                  inu_quit (INUMALLOC);
               }
               break;
            }
            else
            {
               if ((op = grep_option_toc (toc->options, NIL (char), 
                         0, level, &found, sv_name)) == NIL (Option_t))
               {
                  if ( ! found)
                  {
                     if ((inu_add_op_to_fail_sop (failsop, sv_name, level, 
                          STAT_NOT_FOUND_ON_MEDIA, NIL (char), "")) != SUCCESS)
                     {
                        inu_quit (INUMALLOC);
                     }
                  }
                  break;
               }
            } 
         } /* if ( ! found) */
         else
            break;

      } /* end if (op == NIL (Option_t)) */
      else
         found = 0;
      xfound = 1;

#ifdef INUDEBUG
      fprintf (dfp, "--------------------------------------------------\n");
      fprintf (dfp, "inu_bld_sop_toc - after grep_option_toc:\n");
      fprintf (dfp, "    sv_name  = %s\n", sv_name);
      fprintf (dfp, "    i_name   = %s\n", i_name);
      fprintf (dfp, "    op->name = %s\n", op->name);
      fflush (dfp);
#endif  /* INUDEBUG */

      /*------------------------------------------------------------------*
       * if info (-i) or APAR info (-A) then add to list and go on...
       *------------------------------------------------------------------*/

      if (i_FLAG  ||  A_FLAG)
      {
         if ((B_FLAG  &&  IF_UPDATE (op->op_type))  || 
             (I_FLAG  &&  if_install (op)))
         {
            /* does this lpp match the parts that where requested? */

            op->Status = STAT_SUCCESS;
            if (inu_check_parts (op) != SUCCESS)
            {
               if (op->Status != STAT_SUCCESS)
               {
                  inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                                          op->Status, NIL (char), op->desc);
               }
               continue;
            }

            op->operation    = OP_STATUS;
            op->SelectedList = InsertSop_toc (sop, op);
            continue;
         }
      }

      /*------------------------------------------------------------------*
       * At this point all we have to deal with are applys...
       *------------------------------------------------------------------*/

      /*------------------------------------------------------------------*
       * If a level was given AND the force flag was given AND the op found
       * is an update then we need to warn the user that updates cannot be
       * force applied.
       *------------------------------------------------------------------*/
      if (level->ver != -1  &&  F_FLAG  &&  IF_UPDATE (op->op_type))
      {
         inu_add_op_to_fail_sop (failsop, op->name, &(op->level),
                                 STAT_NO_FORCE_APPLY_PTF, NIL (char), op->desc);
         continue;

      }

      if ((B_FLAG  &&  IF_UPDATE  (op->op_type))  || 
          (I_FLAG  &&  if_install (op)))
      {
         /* does this lpp match the parts that where requested? */

         op->Status = STAT_SUCCESS;
         if (inu_check_parts (op) != SUCCESS)
         {
            if (op->Status != STAT_SUCCESS)
            {
               inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                                       op->Status, NIL (char), op->desc);
            }
            continue;
         }


         /*--------------------------------------------------------------*
          * For all installs, inu_level_ok makes sure the level of the
          *   option requested to be installed is greater than what is
          *   currently on the system, unless they specify the -F  flag.
          *   If the option isn't installed at all then that's ok also...
          *
          * For 3.2 updates inu_level_ok checks for the existance of the
          *   update on the system in the applied or committed state and
          *   will return FALSE if the update is already installed.
          *
          * For 3.1 installs, inu_level_ok also makes sure the -acN flags
          *   were also set on the command line.
          *
          * For 3.1 updates, inu_level_ok will do a level check similar to
          *   the one done in the 3.1 updatep...
          *   Also, it will make sure the current level of the 3.1 option
          *   is in the committed state.
          *--------------------------------------------------------------*/

         (void) inu_set_vpdtree (op->vpd_tree);

        /** -------------------------------------------------------------- *
         **  Save off the supersedes info, if any, cuz inu_level_ok may
         **  write the ptf id of the superseding ptf that replaces it into
         **  that field. 
         ** -------------------------------------------------------------- */

         savesedes = NIL (char);

         if (op->supersedes != NIL (char))
            savesedes = strdup (op->supersedes);

         levelrc        = inu_level_ok (op);
         sedes          = op->supersedes; 
         op->supersedes = savesedes;

         if ( ! levelrc) 
         {
            if ((inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                                     op->Status, sedes, op->desc)) != SUCCESS)
            {
               inu_quit (INUMALLOC);
            }

            INU_ERR_COUNT++;
            continue;
         }

         /*---------------------------------------------------------------*
	  * Make sure that we do not try to install an OEM-specific fileset
	  * on a system which is not specific to that OEM, and make sure
	  * that we do not apply a non-OEM fileset update on top of an
	  * OEM-replaced fileset.
          *---------------------------------------------------------------*/

	  if ((oem_system) || ((op->lang[0] == 'o') && 
				(!strncmp(op->lang, "oem_", 4)))) {
            if (inu_oem_chk (op, oem_system_name) != SUCCESS)
            {
               inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                                       op->Status, NIL (char), op->desc);
               continue;
            }
	  }


         /*--------------------------------------------------------------*
          * InsertSop_toc () will make a copy of the option structure
          * and insert the copy into the sop for this option.
          *--------------------------------------------------------------*/

         if ((op->SelectedList = InsertSop_toc (sop, op)) == NIL (Option_t))
            return (FAILURE);
#ifdef INUDEBUG
         fprintf (dfp, "inu_bld_sop_toc - after InsertSop_toc, op->name=%s\n", 
                  op->name);
         fflush (dfp);
#endif  /* INUDEBUG */

         if ((rc = set_operation (op->SelectedList)) != SUCCESS)
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_INTERNAL_E, 
                                         CMN_INTERNAL_D), INU_INSTALLP_CMD);
            return (FAILURE);
         }

      }

   }    /* End of for loop on finding options in the toc option list... */

   return (SUCCESS);

} /* end inu_bld_sop_toc */

/*--------------------------------------------------------------------------*
**
** NAME: set_operation
**
** FUNCTION: Based on the installp parameters, set the operation field in
**           the Option_t structure.  This is structure is part of the sop.
**
** RETURNS:  SUCCESS - correct parm found and op->operation was set.
**           FAILURE - need operation wasn't found, programming error
**
**--------------------------------------------------------------------------*/

int set_operation (Option_t * op)
{
   if (a_FLAG)
      op->operation = OP_APPLY;
   else if (c_FLAG)
      op->operation = OP_COMMIT;
   else if (r_FLAG)
      op->operation = OP_REJECT;
   else if (u_FLAG)
      op->operation = OP_DEINSTALL;
   else if (s_FLAG)
      op->operation = OP_STATUS;
   else if (i_FLAG)
      op->operation = OP_INFO;
   else
      return (FAILURE);
   return (SUCCESS);

} /* end set_operation */

/*--------------------------------------------------------------------------*
**
**  Function        inu_parse_supersedes
**
**  Purpose         Converts the supersedes field of the option passed
**                  in from "lpp ptf lpp ptf lpp ptf" format to "ptf ptf ptf"
**                  format.  And stores the "ptf ptf ptf" back into the super-
**                  sedes field.
**
**                  3 error checks:
**                     1. op must supersede pkgs with the same option name
**                     2. Ptf id of seding fix must be <= 9 chars in length
**                     3. Cannot supersede self
**
**  Returns         SUCCESS   -   if conversion was successful
**                  FAILURE   -   if conversion was un-successful
**
**--------------------------------------------------------------------------*/

static int inu_parse_supersedes (Option_t * op)
{
   FILE            fp;
   int             len;
   char            sceded[100];
   char            opt_name[200];

   if (op->supersedes == NIL(char) || *op->supersedes == '\0')
      return (SUCCESS);

   if (IF_INSTALL (op->op_type)  ||  IF_3_1 (op->op_type))
   {
      free (op->supersedes);
      op->supersedes = NULL;
      return (SUCCESS);
   }

   /* make supersedes pointer look like a file */

   fp._flag = (_IOREAD | _IONOFD);
   fp._ptr = fp._base = (unsigned char *) op->supersedes;
   fp._cnt = strlen (op->supersedes);
   fp._file = _NFILE;

   *sceded = '\0';
   *opt_name = '\0';

   if (fscanf (&fp, "%199s %99s", opt_name, sceded) == EOF)
      return (SUCCESS);
   *op->supersedes = '\0';
   do
   {
      if (strcmp (opt_name, op->name) != 0)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INVAL_SCED_OPT_E, 
                              INP_INVAL_SCED_OPT_D), opt_name, op->name);
         free (op->supersedes);
         op->supersedes = NULL;
         return (FAILURE);

      }
      if (strlen (sceded) > sizeof (op->level.ptf) - 1)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INVAL_SCED_SCED_E, 
                  INP_INVAL_SCED_SCED_D), sceded, sizeof (op->level.ptf) - 1, 
                  sizeof (op->level.ptf) - 1);
     
         free (op->supersedes);
         op->supersedes = NULL;
         return (FAILURE);
      }

      strcat (sceded, " ");
      strcat (op->supersedes, sceded);
      *sceded = '\0';
      *opt_name = '\0';

   } while (fscanf (&fp, "%199s %99s", opt_name, sceded) != EOF);

   /* adjust the supersedes pointer */

   len = strlen (op->supersedes);
   if (len == 0)
   {
      free (op->supersedes);
      op->supersedes = NULL;
      return (SUCCESS);
   }
   op->supersedes[len - 1] = '\0';
   op->supersedes = realloc (op->supersedes, len);

   /**
    **  if this option erroneously supersedes itself, 
    **  then print an error message and return non-zero
    **  --  which indicates failure.
    **/

   if (strstr (op->supersedes, op->level.ptf))
   {
      char            levname[PATH_MAX * 2];

      inu_get_optlevname_from_Option_t (op, levname);
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_PTF_CANT_SEDE_SELF_E, 
               INP_PTF_CANT_SEDE_SELF_D), levname);
      return (FAILURE);
   }

   return (SUCCESS);
} 

/*--------------------------------------------------------------------------*
**
** Function name inu_parse_post_32_supersedes
**
** Purpose       Validates the contents of the supersedes section (obtained 
**               from the lpp_name file or the toc) for post 3.2 pkgs (4.1 
**               and later).  Error checking is really all this routine does.
**                 
** Assumptions   Format coming in is (1 to n lines worth):
**               "<option name> v.r.m.f"
** 
**               Format going out is the same:
**               "<option name> v.r.m.f"
**
**               Error checks:
**                 General checks:
**                  1. Each line must follow "name level" format
**                  2. All level's must be of vv.rr.mmmm.ffff form
**                  3. No ptf permitted in level 
**
**                 Barrier entries       (name == op's name ==> barrier file)
**                  1. Specified level must not be greater than pkg level 
**
**                 Compatability entries (name != op's name ==> compat entry)
**                     (name will usually not match op's name, not enforced)
**                  1. Currently, no special checks.
**
**  Returns        SUCCESS   -   if supersedes info is legal
**                 FAILURE   -   if supersedes info is not legal
**
**--------------------------------------------------------------------------*/

static int inu_parse_post_32_supersedes 
  (Option_t *op)   /* option ptr containing supersedes info to validate */
{
   char    name      [MAX_LPP_FULLNAME_LEN+1];         /* from barrier info  */
   char    level_str [MAX_LEVEL_LEN+1];                /* from barrier info  */
   char    levname   [MAX_LPP_FULLNAME_LEN+MAX_LEVEL_LEN+1]; /* name + level */
   int     n;                                       /*  for parsing       */
   Level_t Level; /* used to convert level string to check legality of level */ 
   FILE    fp;                                /* For reading supersedes info */

   #define BARRIER_ENTRY(n1,n2) strcmp(n1,n2) ? FALSE: TRUE
   #define COMPAT_ENTRY(n1,n2)  strcmp(n1,n2) ? TRUE : FALSE

  /** -------------------------------------------------------------------- *
   **  Empty supersedes ==> nothing to do
   ** -------------------------------------------------------------------- */

   if (op->supersedes == NIL(char) || *op->supersedes == '\0')
      return (SUCCESS);

  /** ----------------------------------------------------------------------- *
   **  Make the supersedes string look like a file ptr, so we can fscanf it.
   ** ----------------------------------------------------------------------- */

   fp._flag = (_IOREAD | _IONOFD);
   fp._ptr  = fp._base = (unsigned char *) op->supersedes;
   fp._cnt  = strlen (op->supersedes);
   fp._file = _NFILE;

  /** -------------------------------------------------------------------- *
   **  If nothing to do, return SUCCESS.
   ** -------------------------------------------------------------------- */

   if ((n=fscanf(&fp, "%s %s", name, level_str)) == EOF)
      return (SUCCESS);

  /** -------------------------------------------------------------------- *
   **  Read and validate until either an error occurs or no more to read.
   ** -------------------------------------------------------------------- */

   do
   {
     /** ----------------------------------------------------------------- *
      **  Make sure we have exactly 2 entries per line.
      ** ----------------------------------------------------------------- */

      if (n != 2)
      {
         inu_get_optlevname_from_Option_t (op, levname);

         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_SEDES_PARSING_E,
                                      INP_SEDES_PARSING_D), levname);
         return (FAILURE);
      }

     /** ----------------------------------------------------------------- *
      **  Make sure a legal level was specified... 
      **   -Version illegally contains a non-digit  
      **   -No ptfs permitted in the level
      ** ----------------------------------------------------------------- */

      if ((inu_level_convert(level_str,&Level) != SUCCESS) || 
             Level.ver == -1  ||  Level.ptf[0] != '\0')
      {
         inu_get_optlevname_from_Option_t (op, levname);
         instl_msg (FAIL_MSG, MSGSTR(MS_INSTALLP, INP_BAD_LEVEL_IN_SEDES_E, 
                               INP_BAD_LEVEL_IN_SEDES_D), level_str, levname);
         return (FAILURE);
      }

     /** ----------------------------------------------------------------- *
      **  If a higher level was specified in a barrier entry, error off. 
      ** ----------------------------------------------------------------- */

      if (BARRIER_ENTRY(op->name, name) &&  
          inu_level_compare(&(Level),&(op->level)) > 0)
      {
         inu_get_optlevname_from_Option_t (op, levname);
         instl_msg (FAIL_MSG, MSGSTR(MS_INSTALLP, INP_BAD_LEVEL_IN_SEDES_E, 
                              INP_BAD_LEVEL_IN_SEDES_D), level_str, levname);

         instl_msg (FAIL_MSG, MSGSTR(MS_INSTALLP, INP_LEVEL_TOO_HIGH_E, 
                                    INP_LEVEL_TOO_HIGH_D));
         return (FAILURE);
      }
   
   } while (fscanf (&fp, "%s %s", name, level_str) != EOF);

   return (SUCCESS);
}


/*--------------------------------------------------------------------------*
**
**  FUNCTION NAME  inu_check_parts
**
**  DESCRIPTION    verifies that the option specified by the
**                 parameter op belongs to a valid part, ie usr, root or
**                 share.  The only valid exeption is mcode.
**
**  PARAMATERS
**
**  RETURNS        SUCCESS  -  if the part is valid
**                 FAILURE  -  if the part is NOT valid
**
**--------------------------------------------------------------------------*/

static int inu_check_parts (Option_t *op)
{
   /*------------------------------------------------------------------*
    * if this is a USR and ROOT (BOTH) bff and the user didn't ask for
    * both parts ("-O ru"), then don't install the USR part either.
    *------------------------------------------------------------------*/

   if (op->content == CONTENT_BOTH)
   {
      if ( ! USR_PART  &&  ! ROOT_PART)
         return (FAILURE);

      if (USR_PART  &&  ! ROOT_PART)
      {
         op->Status = STAT_MUST_APPLY_ROOT_TOO;
         return (FAILURE);
      }
      return (SUCCESS);
   }

   /*------------------------------------------------------------------*
    * if this is a USR bff and the user didn't ask for it, continue
    *------------------------------------------------------------------*/

   if (op->content == CONTENT_USR)
   {
      if ( ! USR_PART)
         return (FAILURE);
      else
         return (SUCCESS);
   } 

   /*------------------------------------------------------------------*
    * if this is a SHARE bff and the user didn't ask for it, continue
    *------------------------------------------------------------------*/

   if (op->content == CONTENT_SHARE)
   {
      if ( ! SHARE_PART)
         return (FAILURE);
      else
         return (SUCCESS);
   }

   if (op->content == CONTENT_MCODE)
      return SUCCESS;

  /*------------------------------------------------------------------*
   * Don't know what part this could be
   *------------------------------------------------------------------*/

   op->Status = STAT_NUTTIN_TO_APPLY;

   return (FAILURE);

} 


/* ------------------------------------------------------------------*
**
**  Function        remove_bos_rte
**
**  Purpose         Removes bos.rte from the sop in either of 2 cases:
**                    1. Performing root part operations
**                    2. apply operation on bos.rte.
**
**  Parameters      sop - selected options list 
**
** ------------------------------------------------------------------*/

static void remove_bos_rte (Option_t * sop, Option_t *failsop)
{
   Option_t       *op, *p_op;

   if (sop->next == NIL (Option_t))
      return;

  /** ------------------------------------------------------------------ *
   **  Remove any illegal "bos.rte" options that may exist
   ** ------------------------------------------------------------------ */

   for (op=sop->next, p_op=sop;
        op != NIL (Option_t);
        p_op=op, op=op->next)
   {
      if (op->vpd_tree == VPDTREE_ROOT 
                        &&  
              IF_INSTALL (op->op_type) 
                        && 
          strcmp (op->name, "bos.rte") == 0)
      {
         p_op->next = op->next;
         inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                                 STAT_NUTTIN_TO_APPLY, NIL(char), op->desc);
      }

      else if (op->vpd_tree == VPDTREE_USR 
                       && 
               op->bff->action == ACT_OTHER
                       && 
          strcmp (op->name, "bos.rte") == 0)
      {
         p_op->next = op->next;
      }
   }
}

/* returns true if op is an install or gold update */

static int if_install (Option_t * op)
{
   /* install image? */

   if (IF_INSTALL (op->op_type))
      return (TRUE);

   /* Do we have a bff associated, Can't be golden update with out a bff */

   if (op->bff == NIL (BffRef_t))
      return (FALSE);

   /* If action is golden update this can be considered an install */

   return (IF_GOLD_UPDATE (op->bff->action));

} /* end if_install */

inu_ck_tsavdir (Option_t * sop)  /* Ptr to top of the Selected options list */
{
    int             rc = SUCCESS;       /* found alternate save directories */
    int		    errcount = 0;	/* count the errors */
    int		    errsave;		/* save errno */
    int		    len;
    struct stat     stbuf;
    char            savdir[PATH_MAX+1];
    char            linkdir[PATH_MAX+1]; /* symbolic link points here */
    char            tsavdir[PATH_MAX+1]; /* alternate save directory */
    char            optlevname[PATH_MAX];

    /* if rejecting an update and if there is a symbolic link to the
       save directory, then the save directory must exist to check
       if there were files replaced during the apply. */
    for (sop = sop->next; sop != NIL (Option_t); sop = sop->next) {
      if (IF_UPDATE (sop->op_type)  &&   ( ! (IF_C_UPDT (sop->op_type)))) {
        (void) inu_get_savdir (sop->vpd_tree, sop, savdir);
        if (stat (savdir, &stbuf) == -1) {  /* following symlink fails */
	  errsave = errno;		  /* then read symlink */
          if (readlink (savdir, linkdir, sizeof (linkdir)-1) > 0) {
	    if (errcount++ == 0) {	/* display message once */

	      /* get alternate path by stripping savdir from linkdir */
	      if ((len = strlen (linkdir) - strlen (savdir)) > 0  && 
		  strcmp (linkdir+len, savdir) == 0) {
	        strncpy (tsavdir, linkdir, len);
		tsavdir[len] = '\0';
	      } else 			      /* or symlink is strange */
                strcpy (tsavdir, linkdir);      /* unknown alternate save dir */

	      instl_msg (FAIL_MSG, "%s:\n", linkdir);
	      errno = errsave;
	      perror ("stat");		      /* display stat error */

	      /* unresolved symbolic link to nowhere */
              instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, 
                      INP_REJECT_DANGLING_SAVDIR_SYM_LINK_E, 
                      INP_REJECT_DANGLING_SAVDIR_SYM_LINK_D), 
                      savdir, linkdir, tsavdir);
	    }

	    /* display all products with an unresolved sym link */
	    (void) inu_get_optlevname_from_Option_t (sop, optlevname);
	    instl_msg (FAIL_MSG, "\t  %s\n", optlevname);
	    rc = FAILURE;
          }
	}
      }
    }

    return rc;
} /* end inu_ck_tsavdir function */


/* -------------------------------------------------------------------- *
**
**  Function   get_toc_hash_value
**
**  Purpose    Calcualate the hash index value.
**             Algorithm is the traditional add up all the ascii values 
**             of each character of the option name and mod it with the
**             hash table size.
**
**  Returns    hash index value, a number between 0 - (TOC_HASH_SIZE -1)
**
** -------------------------------------------------------------------- */

int get_toc_hash_value (char *name)
{
   int  len, i;
   long tot=0;

   len = strlen (name);

   for (i=0; i<len; i++)
      tot += (int) (name[i]);
      
   return (tot % TOC_HASH_SIZE);
}


/* -------------------------------------------------------------------- *
**
**  Function   add_node_to_toc_hash_table
**
**  Purpose    Adds the hash node (really an Option_t structure) to the
**             toc hash table.
**
**  Returns    None
**
** -------------------------------------------------------------------- */

static void add_node_to_toc_hash_table (Option_t *op)
{
   int hv;               /* Hash index value */
   Option_t *hash_op;    /* traversal ptr    */

   hv = get_toc_hash_value (op->name);

   op->hash_next = NIL(Option_t);

  /** ------------------------------------------------------------- *
   **  If this hash entry is empty, insert this node, and return.
   ** ------------------------------------------------------------- */

   if (toc_hashlist[hv] == NIL(Option_t))
   {
      toc_hashlist[hv] = op;
      return;
   }

  /** ----------------------------------------------------- *
   **  Insert the node at the end of this bucket
   ** ----------------------------------------------------- */

   for (hash_op = toc_hashlist[hv]; 
        hash_op->hash_next != NIL(Option_t);
        hash_op = hash_op->hash_next)
      ;

   hash_op->hash_next = op;
}

/* -------------------------------------------------------------------- *
**
**  Function    hash_the_toc
**
**  Purpose     Abstractualizes the toc into a hash table.
**
**  Returns     None
**
** -------------------------------------------------------------------- */

static void hash_the_toc 
  (Option_t *toc)  /* Ptr to the 1st option in the toc list */
{
   Option_t *op;

   for (op=toc; op != NIL(Option_t); op=op->next)
      add_node_to_toc_hash_table (op);

   hashed_the_toc = TRUE;
}

