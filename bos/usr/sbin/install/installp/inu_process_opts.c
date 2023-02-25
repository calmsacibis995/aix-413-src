static char sccsid[] = "@(#)89  1.45  src/bos/usr/sbin/install/installp/inu_process_opts.c, cmdinstl, bos41J, 9519A_all 5/8/95 08:21:13";

/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: applied_something
 *		did_something
 *		get_chunk_size
 *		get_num_pkgs_to_be_processed
 *		inu_create_anchors
 *		inu_op_string
 *		inu_process_opts
 *		inu_remove_bff
 *		inu_verify_install
 *		report_progress
 *		was_applied
 *		was_something_done
 *		print_oper_msg
 *		does_namelist_exist
 *		is_remove_ok
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <installp.h>
#include <instl_options.h>
#include <inulib.h>
#include <inu_string.h>
#include <time.h>
#include <stdio.h>


/*--------------------------------------------------------------------------*
 *                      Local Function Prototypes
 *--------------------------------------------------------------------------*/

static int  was_applied         (Option_t * op);
static int  was_something_done  (Option_t * op);
static int  applied_something   (Option_t * cur_op, Option_t * next_op);
static int  did_something       (Option_t * cur_op, Option_t * next_op, 
                                 char vpd_tree);
static void inu_verify_install  (Option_t * cur_op);
static void inu_remove_bff      (Option_t * first_op, Option_t * cur_op, 
                                 Option_t * next_op);
static char * inu_op_string     (int);

static int get_num_pkgs_to_be_processed (Option_t * sop);
static int get_chunk_size               (Option_t *firstop, Option_t *next_op);
static void report_progress             (int num_processed, int total);
static void   print_oper_msg (Option_t *prev_op,
                              Option_t *cur_op, 
                              Option_t *next_op);
static int is_remove_ok(Option_t *, Option_t *);
static int does_namelist_exist(char *);
static void consume_child();

/*--------------------------------------------------------------------------*
 *                        Local Variables
 *--------------------------------------------------------------------------*/
static int might_be_migrate;


/*--------------------------------------------------------------------------*
 *                        External  Variables
 *--------------------------------------------------------------------------*/

extern char     *sys_errlist[];
extern int       nerr;
extern int need_to_reexec;     /* TRUE if installp has been updated and we  */
                               /* need to reexec to process other pkgs      */
extern int updated_installp;   /* TRUE if installp has been updated         */

extern TOC_t    *TocPtr;       /* The pointer to the Media toc               */
extern Option_t *SopPtr;       /* the anchor of the Selected OPtions list    */
extern Option_t *SaveSopPtr;   /* anchor to the save sop (due to i ptfs)     */
extern Option_t *FailSopPtr;   /* anchor to the pre-installation failure sop */
extern nl_catd   catd;         /* The file pointer to the message catalog    */
extern int       biron;        /* "Blast Inst Roots Or Not" variable         */

extern time_t    tp;           /* # seconds at program (main) entry */ 
extern boolean   time_failed;


/*--------------------------------------------------------------------------*
**
**  Function:      inu_process_opts
**
**  Description:   Processes the entire Selected Options list, from the
**                 beginning to the end a "chunk" at a time.  The first item
**                 on the sop is SopPtr->next.  The sop is broken up into
**                 "chunks".  The next_op variable points to the first
**                 option of the next sop chunk to process.
**
**  Parameters:    TocPtr - anchor ptr to the toc list
e*                 SopPtr - anchor ptr to the sop
**
**  Returns:       INUGOOD  - if success
**                 non-zero - if failure
**
**--------------------------------------------------------------------------*/

int inu_process_opts (
    TOC_t    * TocPtr,    /* Ptr to the anchor of the toc linked list    */
    Option_t * SopPtr,    /* Ptr to the top of the Selected Options List */
    Option_t * failsop)   /* Ptr to anchor of the failsop list           */
{
   char  status_path [MAX_LPP_NAME];
   Option_t  * prev_op;     /* Ptr to 1st option of the previous sop chunk */
   Option_t  * cur_op;      /* Ptr to 1st option of the current sop chunk */
   Option_t  * next_op;     /* Ptr to 1st option of the next sop chunk */
   Option_t  * op;          /* Temp Ptr to option of sop ptr  */
   prod_t    prod;
   int       rc = 0;

   int       processed_pkgs=0; /* Number of processed packages              */
   int       total_pkgs=0;     /* Total pkgs to be processed                */
   int       chunk_size=0;     /* size of current sop chunk being processed */
   boolean   unprocessed_pkgs_from_1st_sop = FALSE;    /* done after reexec */
   char * ptr;

   /* set indicator from bos instl (big time saver) */
   might_be_migrate=TRUE;
   if(((ptr=getenv("INUBOSTYPE"))!=NIL(char)) && (atoi(ptr)==1)) {
	might_be_migrate=FALSE;
   } else {
	unlink(STORED_CFGFILES_LIST);
   }

   strcpy (status_path, INU_TMPDIR);
   strcat (status_path, "/status");

   /* initialize string[] array for printing status messages */
   inu_init_strings ();

   /*-----------------------------------------------------------------------*
    *  Determine if the BIRON variable is set or not, and set the global
    *  variable to that effect.
    *-----------------------------------------------------------------------*/

   biron = is_biron_set ();

   total_pkgs = get_num_pkgs_to_be_processed (SopPtr->next);

  /*-----------------------------------------------------------------------*
   * Loop though the entire Selected Options List and perform the operation
   * indicated by the Option_t struct member field cur_op->operation...
   *-----------------------------------------------------------------------*/

   
   for (cur_op = SopPtr->next, prev_op = NIL (Option_t);
        cur_op != NIL (Option_t)  &&  updated_installp == FALSE;
        prev_op = cur_op, cur_op = next_op, processed_pkgs += chunk_size)
   {

     /* reap our dead childern (if any) */
      consume_child();

     /*-------------------------------------------------------------------*
      * Each operation is broken up by when the top level lppname changes
      * or the file tree that we're dealing with changes...
      * inu_set_cur_oper () determines the first option's top level name
      * and then scans through the Selected Options List until one of the
      * above conditions occur.
      * Parameter 2 is passed with an undefined value, upon return of the
      * function it will contain a pointer to the first option of the next
      * operation in the selected option list.
      *-------------------------------------------------------------------*/

      inu_set_cur_oper (cur_op, &next_op);

      chunk_size = get_chunk_size (cur_op, next_op);
 
      if ( ! IF_OTHER_PART_ON_SOP (cur_op->flag))
         report_progress (processed_pkgs, total_pkgs); 

      /* remove that old status file */
      unlink (status_path);

      /*----------------------------------------------------------------------*
       * Conditionally display a message about the operation we are about to do
       *----------------------------------------------------------------------*/
      print_oper_msg (prev_op, cur_op, next_op);


      /*-------------------------------------------------------------------*
       * Call inu_mk_opt_fil (), to build a path for the "lpp.options" file.
       * Then put all of the options in the file based on the Selected
       * Options List.  Initially the Status field of the option structure
       * are set to STAT_SUCCESS so all of the options for this operation
       * will be put into the "lpp.options" file.
       *
       * For an apply operation, we'll check that the prereqs of each 
       * option are already installed (or about to be installed for multiple
       * options from the same bff).  If any prereqs are not satisfied, 
       * the requisite checking routine will move the sop entries outside
       * the list returned by inu_set_cur_oper.  (We only do this for 
       * non-3.1 packages since they call ckprereq from their own install 
       * scripts).
       *-------------------------------------------------------------------*/

      if (a_FLAG  ||  c_FLAG  ||  r_FLAG  ||  u_FLAG)
      {
         if (a_FLAG  &&  ! IF_3_1 (cur_op->op_type))
         {
            inu_pseudo_ckreq (SopPtr, &cur_op, next_op);
            if (cur_op == next_op)
               continue;
         }

         if ((rc = inu_mk_opt_fil (cur_op, next_op, STAT_SUCCESS, NIL (char)))
             != INUGOOD)
            inu_quit (rc);
      }
      else if (C_FLAG)
         if ((rc = inu_mk_opt_fil (cur_op, next_op, STAT_CLEANUP, NIL (char)))
             != INUGOOD)
            inu_quit (rc);



      /*-------------------------------------------------------------------*
       * If our current operation requires control files from liblpp.a that
       * resides on the media then we need to extract them...
       *-------------------------------------------------------------------*/

      /* remove old save files */

      if (cur_op->operation != OP_COMMIT  &&  a_FLAG   && 
          IF_UPDATE (cur_op->op_type))
         inu_remove_save_files (cur_op, next_op);

      /*-------------------------------------------------------------------*
       * If we're dealing with a stacked tape,  position the media to the
       * correct tape mark.
       *-------------------------------------------------------------------*/

      if (cur_op->vpd_tree != VPDTREE_ROOT  &&  NEEDS_DEVICE  && 
          cur_op->operation != OP_COMMIT)
      {
        /*--------------------------------------------------------------*
         * Don't call inu_position_tape for C type ptfs -- Defect 87216
         *--------------------------------------------------------------*/
         if (TocPtr->type == TYPE_TAPE_SKD  &&  ! (IF_C_UPDT (cur_op->op_type)))
            if(inu_position_tape (cur_op) != 0)
	    {
	       /* forget this chunk and try the next */
               for (op = cur_op; op != next_op; op = op->next)
                  op->Status = STAT_CANCEL;

               if (a_FLAG)
                  inu_set_graph_status (cur_op, next_op);

      	       print_oper_msg (prev_op, cur_op, next_op);
               continue;
	    }
		
      }

     /** ---------------------------------------------------------------- *
      **  If we're doing an apply, list instructions, or list APAR info 
      **                             AND
      **  we're dealing with either a USR or a SHARE part OR
      **                     a ROOT part only    
      **                             AND
      **  the option we're dealing with is NOT a 'C' ptf, 
      **  then make sure the libdir is clean, and get the control files.
      ** ---------------------------------------------------------------- */

      if (cur_op->operation != OP_COMMIT  &&  (a_FLAG || i_FLAG || A_FLAG)
                                          &&
          (cur_op->vpd_tree != VPDTREE_ROOT ||
           (cur_op->vpd_tree == VPDTREE_ROOT && ! USR_PART))
                                          &&
          ( ! IF_C_UPDT (cur_op->op_type)))
      {

        /** ---------------------------------------------------------------- *
         **  This state of this set of options is now AVAILABLE.
         ** ---------------------------------------------------------------- */

        /** ---------------------------------------------------- *
         **  We really should NOT be calling inu_clean_libdir 
         **  if we are only doing a -A or a -i.  We do, however, 
         **  want to do everything else inside this big if 
         **  statement.
         ** ---------------------------------------------------- */

         inu_set_vpdtree (cur_op->vpd_tree);

         if (cur_op->vpd_tree != VPDTREE_ROOT)
         {
            if ( ! A_FLAG  &&  ! i_FLAG)
               if (IF_INSTALL (cur_op->op_type))
               {
                  /* ---------------------------------------------------- *
                  * inu_clean_lpp_libdir will be called regardless of what
                  * level is currently applied/committed.  This will remove
                  * option.*.controlfiles.
                  * In 3.2, if a different level was applied/committed,
                  * inu_clean_libdir was called.
                  ** ---------------------------------------------------- */
                  inu_clean_lpp_libdir (cur_op, next_op);
               }
               else
                  inu_clean_libdir (cur_op, next_op);

            inu_set_libdir (cur_op->vpd_tree, cur_op);

            if (IF_3_1_UPDATE (cur_op->op_type))
               /* this is a 3.1 update there are specific files that need to be
                * restored from the liblpp.a servce_num, arpsize size ... */

               rc = inu_get_31cntl_files (cur_op, next_op);
            else  /* this is an install or a 3.2 update */
               rc = inu_get_cntl_files (cur_op, next_op);

            /* cancel the poopies if we couldn't get the control files */
            if (rc != INUGOOD)
            {
               extern int force_abs_seek;
               force_abs_seek=1;
   
               for (op = cur_op; op != next_op; op = op->next)
               op->Status = STAT_CANCEL;

               if (a_FLAG)
                  inu_set_graph_status (cur_op, next_op);

               print_oper_msg (prev_op, cur_op, next_op);
               continue;
            }
         }

        /** ------------------------------------------------------------ *
         **  Call the rminstal script for this chunk of options.  This
         **  script removes the files from the filesystem.  inu_remove_opts
         **  also removes the appropriate VPD records for these options.
         **  In 4.1 we do this after get_cntl_files so that we can run the
         **  pre_rm script if one exists for the option that we are dealing
         **  with.
         ** ------------------------------------------------------------- */

        if (IF_INSTALL (cur_op->op_type)  &&  a_FLAG)
        {
           if (is_remove_ok (cur_op,next_op) == FALSE)
              continue;
        }
     }

      /*-------------------------------------------------------------------*
       * Set vpdremotepath, vpdremote, ODMDIR and INUTREE  environment
       * variables based on the vpdtree that this operation is dealing with.
       *-------------------------------------------------------------------*/

      inu_set_vpdtree (cur_op->vpd_tree);

      /*-------------------------------------------------------------------*
       * Set the inu_libdir global string variable based on the information
       * in the option structure.
       *-------------------------------------------------------------------*/

      inu_set_libdir (cur_op->vpd_tree, cur_op);


      /*------------*
       * STATUS
       *-------------------------------------------------------------------*
       * inu_ls_apars () - Print out a formated version of the fixinfo file
       *-------------------------------------------------------------------*/

      if (A_FLAG)
      {
         inu_update_fixdata(cur_op,next_op);
         rc = inu_ls_apars (cur_op,next_op);
         if (rc != INUGOOD)
            inu_quit (INUACCS);
         else
            continue;
      }


      /*-------------------*
       * PRINT INSTRUCTIONS
       *-------------------------------------------------------------------*
       * inu_ls_instr () - 'cat' out the contents of the lpp.README, lpp.doc
       * and lpp.instr files to LOG.
       *-------------------------------------------------------------------*/

      if (i_FLAG)
      {
         inu_update_fixdata(cur_op,next_op);
         rc = inu_ls_instr (cur_op);
         if (rc != INUGOOD)
            inu_quit (INUACCS);
         else
            continue;
      }


      /*------------*
       * APPLY
       *-------------------------------------------------------------------*
       * Apply the current operation...it could be doing root applys if that
       * was the only part that was requested, OR it could be doing /usr
       * applys, if we're doing /usr applys then the next if statement will
       * see if cooresponding root applys must also be done.
       * When finished, reflect the result of the operation in the 
       * requisite graph.
       *-------------------------------------------------------------------*/

      if (cur_op->operation == OP_APPLY || cur_op->operation == OP_APPLYCOMMIT)
      {
         inu_update_fixdata(cur_op,next_op);
         rc = inu_apply_opts (cur_op, next_op);
         inu_set_graph_status (cur_op, next_op);
         if (rc != INUGOOD)
            continue;
         /*---------------------------------------------------------------*
          * IF -D option was specified, (delete bff after installation)
          * THEN we need to scan down the sop to make sure this bff image
          * isn't needed later on. (This condition could occur due to
          * prereqs forcing only part of an LPP being applied and then
          * another part to be applied later. Rare case yes, but possible)
          *---------------------------------------------------------------*/

         inu_remove_bff (SopPtr->next, cur_op, next_op);

         /*---------------------------------------------------------------*
          * IF the first call to inu_apply_opts () (above) was to do the /usr
          * applys, now we need to see if any root applys need to be done, 
          * so if ROOT_PART is set, inu_bld_root_sop () will add option
          * structs to the end of the /usr apply options in the linked list
          * to perform the root applys. (if the /usr apply failed then it
          * won't be added to the root apply list).
          *---------------------------------------------------------------*/

         if (USR_PART && ROOT_PART)
         {
            /*-----------------------------------------------------------*
             * Build the root sop based on what succeeded in the /usr part.
             * inu_bld_root_sop () returns the number of root applys to do.
             * After building the root sop, check that the requisites of 
             * each root part to be applied are satisfied.  (See notes in 
             * above call to inu_pseudo_ckreq).
             *-----------------------------------------------------------*/

            if ((rc = inu_bld_root_sop (&cur_op, next_op)) > 0)
            {
               if ( ! IF_3_1 (cur_op->op_type)) 
               {
                  inu_pseudo_ckreq (SopPtr, &cur_op, next_op);
                  if (cur_op == next_op)
                     continue;
               }

               /*-------------------------------------------------------*
                * Set the inu_libdir and vpdremote to the root part
                *-------------------------------------------------------*/

               inu_set_libdir (VPDTREE_ROOT, cur_op);
               inu_set_vpdtree (VPDTREE_ROOT);

               /*-------------------------------------------------------*
                * Apply the root parts for the options in this operation.
                * Update the status of the operation in the requisite
                * graph.
                *-------------------------------------------------------*/

               rc = inu_apply_opts (cur_op, next_op);
               inu_set_graph_status (cur_op, next_op); 
               if (rc != INUGOOD)
                  continue;
            }

         }  /* end  if (USR_PART  &&  ROOT_PART) */

         continue;

      } /* end APPLY */


      /*------------*
       * REJECT
       *-------------------------------------------------------------------*
       * reject the selected options
       *-------------------------------------------------------------------*/

      if (r_FLAG)
      {
         if ((rc = inu_reject_opts (cur_op, next_op)) != INUGOOD)
            continue;
      }


      /*------------*
       * DEINSTALL
       *-------------------------------------------------------------------*
       * deinstall the selected options
       *-------------------------------------------------------------------*/

      if (u_FLAG)
      {
         if ((rc = inu_deinstall_opts (cur_op, next_op)) != INUGOOD)
            continue;
      }


      /*------------*
       * COMMIT
       *-------------------------------------------------------------------*
       * commit the selected options
       *-------------------------------------------------------------------*/

      if (cur_op->operation == OP_COMMIT)
      {
         if ((rc = inu_commit_opts (cur_op, next_op)) != INUGOOD)
            continue;
      }

      /*------------*
       * CLEANUP
       *-------------------------------------------------------------------*
       * This call will only take place due to the -C flag given on the
       * command line. If cleanup is needed due to a failed apply, then
       * inu_cleanup_opts () will be called from inu_apply_opts ().
       *-------------------------------------------------------------------*/

      if (C_FLAG)
      {
         /*---------------------------------------------------------------*
          * If this is a cleanup up attempt of a commit, then just try to
          * commit it again...
          *---------------------------------------------------------------*/

         if (cur_op->operation == OP_CLEANUP_COMMIT)
         {
            if ((rc = inu_commit_opts (cur_op, next_op)) != INUGOOD)
               continue;
         }
         else
         {
            if ((rc = inu_cleanup_opts (cur_op, next_op)) != INUGOOD)
               continue;
         }
      } /* end if (C_FLAG) */

   } /* end of for loop that processes each op */

   report_progress (processed_pkgs, total_pkgs);

   /* reap our dead childern (if any) */
   consume_child();

   if (a_FLAG)
   {
      /* Free the requisite graph.  We don't need it anymore and we don't
       * want it taking up valuable resources for the remaining operations. 
       * (The free_graph routine comes from ckprereq.a) */

      free_graph ();
   }

   if (updated_installp == TRUE  &&  cur_op != NIL (Option_t))
   {
      for (op = cur_op; op != NIL (Option_t); op = op->next)
         op->Status = STAT_CANCEL;

      unprocessed_pkgs_from_1st_sop = TRUE;
   }

   /* if we are apply/commit then commit the options that where just applied */

   if (a_FLAG  &&  c_FLAG  &&  applied_something (SopPtr->next, NULL))
   {
      /* build the commit sop */

      cur_op = SopPtr->next;
      next_op = (Option_t *) NULL;
      if (inu_bld_commit_sop (cur_op, &next_op, failsop) > 0)
      {
         /* commit the op's that are returned */

         inu_display_section_hdr (SECT_HDR_INSTLLING, OP_COMMIT);

         total_pkgs = get_num_pkgs_to_be_processed (next_op);

         processed_pkgs=0;

         for (cur_op=next_op; 
              cur_op != NIL (Option_t); 
              cur_op=next_op, processed_pkgs+=chunk_size)
         {

            inu_set_cur_oper (cur_op, &next_op);
            inu_set_vpdtree (cur_op->vpd_tree);
            inu_set_libdir (cur_op->vpd_tree, cur_op);

            chunk_size = get_chunk_size (cur_op, next_op);

            if ( ! IF_OTHER_PART_ON_SOP (cur_op->flag))
               report_progress (processed_pkgs, total_pkgs);

           /*-----------------------------------------------------------------*
            * Conditionally display a message about the operation we are about
            * to do (commit in this case).
            *-----------------------------------------------------------------*/
            print_oper_msg (prev_op, cur_op, next_op);

           /*----------------------------------------------------------------*
            * We have to re-create the lpp.option file for each call to 
            * inu_commit_opts.
            *----------------------------------------------------------------*/
            if ((rc = inu_mk_opt_fil (cur_op, next_op, STAT_SUCCESS, 
                                      NIL (char))) != INUGOOD)
               inu_quit (rc);

            inu_commit_opts (cur_op, next_op);
         } 
         report_progress (processed_pkgs, total_pkgs);
      } 
   } 

   if (A_FLAG || i_FLAG)
      inu_ls_missing_info (SopPtr);

  /** ------------------------------------------------------------ *
   **  Print the post processing header ... 
   ** ------------------------------------------------------------ */

   if (r_FLAG)
      inu_display_section_hdr (SECT_HDR_POST_PROC, OP_REJECT);
   else if (a_FLAG)
      inu_display_section_hdr (SECT_HDR_POST_PROC, OP_APPLY);
   else if (c_FLAG)
      inu_display_section_hdr (SECT_HDR_POST_PROC, OP_COMMIT);
   else if (u_FLAG)
      inu_display_section_hdr (SECT_HDR_POST_PROC, OP_DEINSTALL);


  /** --------------------------------------------------------------------- *
   **  Set the need_to_reexec variable to TRUE if we updated installp and 
   **  need to reexec for unprocessed pkgs that exist either on the same
   **  sop as the installp update or on the saved sop.
   ** --------------------------------------------------------------------- */

   if (updated_installp == TRUE  &&  (unprocessed_pkgs_from_1st_sop == TRUE  || 
                                      SaveSopPtr->next != NIL (Option_t)))
   {
      need_to_reexec = TRUE;
   }

   if (v_FLAG  &&  ! need_to_reexec)
      inu_verify_install (SopPtr->next);

   /*------------------------------------------------------------------------*
    *  If this was an apply operation and potentially a migration operation,
    *  then check to see if there were any configuration files which were
    *  saved and not merged back into the system
    *-----------------------------------------------------------------------*/

   if (a_FLAG && might_be_migrate)
   {
      char sortcmd[128];
      char cfgfile[PATH_MAX+1];
      char part;
      char lastpart = '\0';
      char *cfgsavedir = NULL;
      FILE *cfgfilelist = NULL;

      if ((cfgfilelist = fopen (STORED_CFGFILES_LIST, "r")) != NULL)
      {
           /*-------------------------------------------------------------*
            *  Sort the cfgfiles list by part, then by name.
            *  The cfgfiles list is an unsorted list of filename PART
            *  pairs.  This allows us to recognize each part and spit 
	    *  out an appropriate header for each part.
            *-------------------------------------------------------------*/
	   
	  sprintf (sortcmd,"/usr/bin/sort -b +1 +0 %s -o %s",
			STORED_CFGFILES_LIST,STORED_CFGFILES_LIST);
	  if (system(sortcmd) == 0) {
	      instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_CFGFILES_STORED_I,
	      						INP_CFGFILES_STORED_D));
	      while (fscanf (cfgfilelist, "%s %c\n", cfgfile, &part) == 2) {
                  /*------------------------------------------------------*
                   *  Print out an appropriate header for each part 
                   *  processed.
                   *------------------------------------------------------*/
		  if (part != lastpart) {
		      lastpart = part;

		      switch (part) {
			 case VPDTREE_ROOT:
			    cfgsavedir = ROOT_MIGSAVE;
			    break;
			 case VPDTREE_USR:
			    cfgsavedir = USR_MIGSAVE;
			    break;
			 case VPDTREE_SHARE:
			    cfgsavedir = SHARE_MIGSAVE;
			    break;
		      }
		      instl_msg (SCREEN_LOG, MSGSTR(MS_INSTALLP,INP_CFGLIST_I,
						   INP_CFGLIST_D), cfgsavedir);
		  }
		  instl_msg (SCREEN_LOG, "  %s\n", cfgfile);
	      }
	      instl_msg (SCREEN_LOG, "\n");
	  }
	  unlink (STORED_CFGFILES_LIST);
      }
   }

   /*------------------------------------------------------------------------*
    *  If the user specified the -D flag (remove image after using it) then
    *  we want to make sure that the .toc file in the directory where we used
    *  the images is up-to-date.
    *
    *  We must also verify that the device is a dir (type will be TYPE_DISK).
    *-----------------------------------------------------------------------*/

   if (D_FLAG  &&  TocPtr != NIL (TOC_t)  &&  (TocPtr->type == TYPE_DISK))
   {
      char  cmd [LINE_MAX];  /* used to run the system cmd */

      sprintf (cmd, "/usr/sbin/inutoc %s >/dev/null 2>&1", DEVICE);
      if (inu_do_exec (cmd) == INUSYSERR)
         inu_quit (INUSYSERR);
   }

   return (rc);

} /* end inu_process_opts */


/*--------------------------------------------------------------------------*
**
**  Function:      inu_remove_bff
**
**  Description:   Remove the bff if it ain't needed later
**
**--------------------------------------------------------------------------*/

static void inu_remove_bff (Option_t * first_op, 
                            Option_t * cur_op, 
                            Option_t * next_op)
{
   Option_t       *op;

   /* forget about deleting this puppy? */

   if ( ! D_FLAG  || 
       (cur_op->vpd_tree != VPDTREE_USR  &&  cur_op->vpd_tree != VPDTREE_SHARE))
      return;

   /* did an apply fail? forget it */
   for (op = cur_op; op != next_op; op = op->next)
      if (op->Status != STAT_SUCCESS)
         return;

   /* is the bff needed later? forget it */
   for (; op != NIL (Option_t); op = op->next)
      if (op->vpd_tree == cur_op->vpd_tree  && 
          (op->operation == OP_APPLY  ||  op->operation == OP_APPLYCOMMIT)  && 
          (cur_op->bff == op->bff))
         return;

   /* did install fail before? forget it */
   for (op = first_op; op != cur_op; op = op->next)
      if (op->vpd_tree == cur_op->vpd_tree  && 
          (op->operation == OP_APPLY  ||  op->operation == OP_APPLYCOMMIT)  && 
          cur_op->bff == op->bff  &&  op->Status != STAT_SUCCESS)
         return;

   inu_rm (cur_op->bff->path);

} /* inu_remove_bff */


/*--------------------------------------------------------------------------*
**
**  Function:      was_applied
**
**  Description:   Returns TRUE if the op was applied successfully, FALSE
**                 otherwise.
**
**--------------------------------------------------------------------------*/

static int was_applied (Option_t * op)
{
   if (op->Status == STAT_SUCCESS  && 
       (op->operation == OP_APPLY  ||  op->operation == OP_APPLYCOMMIT))
      return (TRUE);
   else
      return (FALSE);

} /* was_applied */


/*--------------------------------------------------------------------------*
**
**  Function:      was_something_done
**
**  Description:   Returns TRUE if something was done to op
**
**--------------------------------------------------------------------------*/

static int was_something_done (Option_t * op)
{
   if ((op->Status == STAT_SUCCESS  ||  op->Status == STAT_FAILURE)
                                    && 
       (op->operation == OP_REJECT  ||  op->operation == OP_COMMIT  || 
        op->operation == OP_APPLY  ||  op->operation == OP_APPLYCOMMIT  || 
        op->operation == OP_DEINSTALL))
      return (TRUE);
   else
      return (FALSE);

} /* end was_something_done */


/*--------------------------------------------------------------------------*
**
**  Function:      applied_something
**
**  Description:   check to see if we applied something
**
**--------------------------------------------------------------------------*/

static int applied_something (Option_t * cur_op, Option_t * next_op)
{
   Option_t       *op;

   for (op = cur_op; op != next_op; op = op->next)
      if (was_applied (op))
         return (TRUE);

   return (FALSE);
} 


/*--------------------------------------------------------------------------*
**
**  Function:      did_something
**
**  Description:   check to see if we did something
**
**--------------------------------------------------------------------------*/

static int did_something (Option_t * cur_op, Option_t * next_op, char vpd_tree)
{
   Option_t       *op;

   for (op = cur_op; op != next_op; op = op->next)
      if (vpd_tree == op->vpd_tree  &&  was_something_done (op))
         return (TRUE);

   return (FALSE);
} 


/*--------------------------------------------------------------------------*
**
**  Function:      inu_verify_install
**
**  Description:   
**
**--------------------------------------------------------------------------*/

static void inu_verify_install (Option_t * cur_op)
{
   char    buf [LINE_MAX];
   char  * root = NULL, 
         * usr = NULL, 
         * share = NULL;
   char  * called_from_instlclient;


   if (ROOT_PART  &&  did_something (cur_op, NULL, VPDTREE_ROOT))
      root = "r";

   if (USR_PART  &&  did_something (cur_op, NULL, VPDTREE_USR))
      usr = "u";

   if (SHARE_PART  &&  did_something (cur_op, NULL, VPDTREE_SHARE))
      share = "s";

   /** ------------------------------------------------------- *
    **  If we're rejecting only root parts and we were called
    **    from instlclient, 
    **  then don't call lppchk at all, cuz it doesn't buy us
    **    anything.
    ** ------------------------------------------------------- */

   called_from_instlclient = getenv ("INUCLIENTS");

   if (root != NULL  &&  usr == NULL  &&  share == NULL   && 
       (called_from_instlclient != NULL)  &&  (r_FLAG  ||  u_FLAG))
      return;

   if (root != NULL  ||  usr != NULL  ||  share != NULL)
   {
      strcpy (buf, "/usr/bin/lppchk -v -Ours");
      inu_do_exec (buf);
   }
} /* end inu_verify_install */


/* ------------------------------------------------------------------------ *
**
**  Function:    inu_create_anchors
**
**  Purpose:     Creates the anchors for the three sop linked lists that
**               we could use.
**
**  Parameters:  None
**
**  Returns:     None -- exits if we have a malloc error.
**
** ------------------------------------------------------------------------ */

void inu_create_anchors ()
{
   /*-------------------------------------------------------------------*
    * Create the anchor for the Selected Option List. The "SopPtr" will
    * contain a linked list of options selected by the user to perform the
    * specified operations on...
    *-------------------------------------------------------------------*/

    SopPtr = create_option ("Selected Option List", 
                             NO,       /* default op_checked field to NO */
                             NO,       /* default quiesce field to NO */
                             CONTENT_UNKNOWN,  /* default content field to
                                                * unknown */
                             NullString,       /* lang field. */
                             NIL (Level_t),    /* default level field to
                                                * NIL (Level_t) */
                             NullString,       /* description. */
                             NIL (BffRef_t));  /* default bff field to
                                                * NIL (BffRef_t) */

    SaveSopPtr = create_option ("Save Selected Option List", 
                              NO,       /* default op_checked field to NO */
                              NO,       /* default quiesce field to NO */
                              CONTENT_UNKNOWN,  /* default content field to
                                                 * unknown */
                              NullString,       /* lang field. */
                              NIL (Level_t),    /* default level field to
                                                 * NIL (Level_t) */
                              NullString,       /* description. */
                              NIL (BffRef_t));  /* default bff field to NULL */


    FailSopPtr = create_option ("Failure Selected Option List", 
                              NO,       /* default op_checked field to NO */
                              NO,       /* default quiesce field to NO */
                              CONTENT_UNKNOWN,  /* default content field to
                                                 * unknown */
                              NullString,       /* lang field. */
                              NIL (Level_t),    /* default level field to
                                                 * NIL (Level_t) */
                              NullString,       /* description. */
                              NIL (BffRef_t));  /* default bff field to NULL */

   /** --------------------------------------------------------------------- *
    **  If we failed creating one of the anchors, create_option will give
    **  a malloc error msg.  We then need to gracefully exit. 
    ** --------------------------------------------------------------------- */

    if (SopPtr == NIL (Option_t)  ||  SaveSopPtr == NIL (Option_t)
                                  ||  FailSopPtr == NIL (Option_t))
       inu_quit (INUNOTIN);   /* Failed creating an anchor to a sop list */

} /* inu_create_anchors */


/* ------------------------------------------------------------------------
**
** NAME: inu_op_string
**
** FUNCTION:
**     Given a valid integer operator value (ie. OP_APPLY, OP_COMMIT, etc...)
**     return the cooresponding operation string (ie "APPLYING", or 
**     "COMMITTING", etc...).
**
** RETURNS: character string
** ------------------------------------------------------------------------*/
char * inu_op_string (int operation)
{
   switch (operation)
   {
      case OP_APPLY:
      case OP_APPLYCOMMIT:
         return (inu_unpad (string[APPLYING_STR]));
         break;
      case OP_COMMIT:
         return (inu_unpad (string[COMMITTING_STR]));
         break;
      case OP_REJECT:
         return (inu_unpad (string[REJECTING_STR]));
         break;
      case OP_DEINSTALL:
         return (inu_unpad (string[DEINSTALLING_STR]));
         break;
      default:
         return (inu_unpad (string[UNKNOWN_STR]));
         break;
   }
}

/* ------------------------------------------------------------------------
**
** Name:       get_num_pkgs_to_be_processed
**
** Function:   Gets a count of the total # of pkgs to be processed.
**
** Returns:    Total packages to be processed 
**
** ------------------------------------------------------------------------*/

int get_num_pkgs_to_be_processed 
 (Option_t * sop)     /*  First option in sop list */
{
   int total=0;
   Option_t *op;

   switch (sop->operation)
   {
     /** ----------------------------------------------------------------- *
      **  Just a raw count, cuz usr/roots won't appear together here. 
      ** ----------------------------------------------------------------- */
      case OP_APPLY:
      case OP_APPLYCOMMIT:
           for (op=sop; op != NIL (Option_t); op=op->next)
              ++total;
           break;
     /** ------------------------------------------------------------------- *
      **  Count one for each usr/root pair, and 1 for lone usr or lone root.
      ** ------------------------------------------------------------------- */
      case OP_COMMIT:
      case OP_REJECT:
      case OP_DEINSTALL:
           for (op=sop; op != NIL (Option_t); op=op->next)
              if ( ! IF_OTHER_PART_ON_SOP (op->flag))
                 ++total;
           break;
      default:
           total = 0;
           break;
   }
   return total;
}

/* ------------------------------------------------------------------------
**
** Name:       get_chunk_size
**
** Function:   Counts the number of pkgs in the current sop chunk being
**             processed.
**
** Returns:    Chunk size count (1, 2, 3, ..., )
**
** ------------------------------------------------------------------------*/

int get_chunk_size
 (Option_t *firstop,    /* first option in chunk      */
  Option_t *next_op)    /* first option of next chunk */
{
   int chunk_size=0;
   Option_t *op;
   
   for (op=firstop; 
        op != NIL (Option_t)  &&  op != next_op;  
        op=op->next)
   {
     /** ----------------------------------------------------------------- *
      **  Don't count root parts if the usr part is also being processed.
      ** ----------------------------------------------------------------- */

      if ( ! IF_OTHER_PART_ON_SOP (op->flag))
         ++chunk_size;
   }

   return chunk_size;
}

/* ------------------------------------------------------------------------
**
** Name:        report_progress
**
** Function:    Reports  (i)  Number of processed pkgs, and if we can
**                       (ii) Total time 
**
** Returns:     None
**
** ------------------------------------------------------------------------*/

void report_progress 
 (int num_processed,    /* number of pkgs already processed */
  int total)            /* total pkgs to be processed       */
{
   time_t   latestime;  /* contains the # seconds since creation of earth */
   time_t   hrs=0;
   time_t   mins=0;
   time_t   secs=0;
   time_t   diff=0;

  /** ----------------------------------------------------------------------- *
   **  Done nothing or have nothing to do ==> no report 
   ** ----------------------------------------------------------------------- */

   if (num_processed == 0  ||  total == 0)
      return;

  /** ---------------------------------------------------------------------- *
   **  As long as we've had no failures getting the time, get the time. 
   ** ---------------------------------------------------------------------- */

   if (time_failed == FALSE)
   {
      if (time (&latestime) == -1)
      {
          instl_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_ERR_FUNCCALL_E, 
                               CMN_ERR_FUNCCALL_D), inu_cmdname, "time");
          time_failed = TRUE; 
      }
      else
      {
         /** ------------------------------------------------------ *
          **  Get total processing time, in seconds.  Then convert.
          ** ------------------------------------------------------ */

          diff = latestime -  tp;  

          hrs  =  diff / (60*60);
          mins =  (diff - hrs*60*60) / 60;
          secs =  (diff - hrs*60*60 - mins*60) % 60;
      }
   }

  /** ---------------------------------------------------------------------- *
   **  Give "Processed x of y pkgs (total time is xxx)" msg. 
   ** ---------------------------------------------------------------------- */

   if (time_failed == FALSE)
   {
      if (num_processed == total)
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_FINIS_PROC_ALL_PKGS_I, 
                                        INP_FINIS_PROC_ALL_PKGS_D));
      else
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_PROC_XOFY_I, 
                                        INP_PROC_XOFY_D), num_processed, total);

      if (hrs > 0)
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_TT_HMS_I, 
                                        INP_TT_HMS_D), hrs, mins, secs);
      else if (mins > 0)
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_TT_MS_I, INP_TT_MS_D), 
                                        mins, secs);
      else 
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_TT_S_I, INP_TT_S_D), 
                                        secs);

      if (num_processed == total)
      {
         instl_msg (SCREEN_LOG, "\n");
         return;
      }
   }

  /** ---------------------------------------------------------------------- *
   **  Else give "Processed x of y pkgs" msg w/o the total time.
   ** ---------------------------------------------------------------------- */

   else
   {
      if (num_processed == total)
      {
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_FINIS_PROC_ALL_PKGS_I, 
                                               INP_FINIS_PROC_ALL_PKGS_D));
         instl_msg (SCREEN_LOG, "\n");
      }
      else
         instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_PROC_XOFY_I, 
                                  INP_PROC_XOFY_D), num_processed, total);
   }
}

/* ------------------------------------------------------------------------
**
** Name:        just_processed_other_part
**
** Function:    Checks to see if any member of the previous sop chunk was 
**              the complement part of the first member of the current sop 
**              chunk (Note:  ROOT is the complement of USR, and vise versa). 
**
** Returns:     boolean
**
** ------------------------------------------------------------------------*/

boolean just_processed_other_part (Option_t *start_prev_chunk,
                                   Option_t *end_prev_chunk,
                                   Option_t *cur_op)
{
   Option_t *op;

   if (start_prev_chunk != NIL (Option_t))
      for (op = start_prev_chunk; op != end_prev_chunk; op = op->next)
         if (op != NIL (Option_t))
            if ((strcmp (op->name, cur_op->name) == 0)  
                                  &&
                (inu_level_compare (&(op->level), &(cur_op->level)) == 0) 
                                  &&
                 op->content == cur_op->content
                                  &&
                 op->operation == cur_op->operation)
               return (TRUE);

   return (FALSE);
}


/* ------------------------------------------------------------------------
**
** Name:        print_oper_msg
**
** Function:    Prints a message about the operation we are about to 
**              perform on the list of software from cur_op to next_op.
**              Only prints sop entries in the current sop chunk that
**              are unique, ie. that are not members of the previous
**              sop chunk.  (ie. prevents printing messages for usr and
**              root parts.)
**
** Returns:     None.
**
** ------------------------------------------------------------------------*/

void print_oper_msg (Option_t *prev_chunk_start, 
                     Option_t *cur_op, 
                     Option_t *next_op)
{
   Option_t *op;
   boolean   printed_header;
   boolean   check_prev_sop_chunk;
   char      levname [MAX_LPP_NAME + MAX_LEVEL_LEN]; /* lppname, opname & lev */

   if (A_FLAG || i_FLAG)
      return;

  /** ---------------------------------------------------------------------- *
   **  Scan the current sop chunk.  Stop when we detect that any member of
   **  the current sop chunk is a member of the previous sop chunk.  Set a
   **  flag accordingly.
   ** ---------------------------------------------------------------------- */
   check_prev_sop_chunk = FALSE;
   for (op = cur_op; op != next_op; op = op->next)
   {
      if (just_processed_other_part (prev_chunk_start, cur_op, op))
      {
         check_prev_sop_chunk = TRUE;
         break;
      }
   }
           
  /** ---------------------------------------------------------------------- *
   **  Scan the current sop chunk.  Print each sop chunk member which does
   **  not appear on the previous sop chunk.
   ** ---------------------------------------------------------------------- */
   printed_header = FALSE;
   for (op = cur_op; op != next_op; op = op->next)
   {
      if ( (!check_prev_sop_chunk) ||
           (!just_processed_other_part (prev_chunk_start, cur_op, op)))
      { 
         if (!printed_header)
         {
            switch (cur_op->operation)
            {
               case OP_CLEANUP_COMMIT:
                  instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, 
                                                 INP_CLEANUP_COMMIT_I,
                                                 INP_CLEANUP_COMMIT_D));
                  break;
               case OP_CLEANUP_APPLY:
                  instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_CLEANING_I,
                                                              INP_CLEANING_D));
                  break;
               case OP_APPLY:
               case OP_APPLYCOMMIT:
                  if(cur_op->Status == STAT_CANCEL)
                  {
                        instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, 
                                               INP_PERFORMING_OPER_I,
                                               INP_PERFORMING_OPER_D),
                                   inu_unpad (string[CANCELLED_STR]));
                        break;
                  }
               default:
                  instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, 
                                                 INP_PERFORMING_OPER_I,
                                                 INP_PERFORMING_OPER_D),
                                         inu_op_string (cur_op->operation));
                  break;
            }
            printed_header = TRUE; 
         }

         inu_get_optlevname_from_Option_t (op, levname);
         instl_msg (SCREEN_LOG, "\t%s\n", levname);
      }
   }
   if (printed_header)
      instl_msg (SCREEN_LOG, "\n");
}

/* ------------------------------------------------------------------------
**
** Name:        is_remove_ok
**
** Function:    determins if we really need to call inu_remove_opts & does it
**
** Returns:     TRUE/FALSE 
**
** ------------------------------------------------------------------------*/

static int is_remove_ok(Option_t *cur_op, Option_t *next_op)
{
   Option_t * op;

   /* indicator from bos instl  */
   if (might_be_migrate == FALSE)
      return (TRUE);

   /* Do we have to run inu_remove_opts? */
   for (op = cur_op; op != next_op; op = op->next)
   {
      /* is there another version applied? or FILESET.namelist */
      if (op->level.sys_ver || does_namelist_exist (op->name))
      {
         /* darn we have todo the remove, coffee break */
         if (inu_remove_opts (TocPtr, cur_op, next_op) == FAILURE)
         {
            for (op = cur_op; op != next_op; op = op->next)
               op->Status = STAT_CANCEL;
            inu_set_graph_status (cur_op, next_op);
            return (FALSE);
         }
         return (TRUE);
      }
   }
   /* nope didn't have to call inu_remove_opts */
   return (TRUE);
}

/* ------------------------------------------------------------------------
**
** Name:        does_namelist_exist
**
** Function:    determins if a namelist file exists for migration.
**
** Returns:     TRUE/FALSE 
**
** ------------------------------------------------------------------------*/
static int does_namelist_exist(char *lpp_name)
{
   char path[PATH_MAX];

   sprintf (path,"%s/%s.namelist",INU_LIBDIR, lpp_name);
   if (access (path,R_OK) == 0)
      return (TRUE);

   return (FALSE);
}

/* ------------------------------------------------------------------------
**
** Name:        consume_child
**
** Function:    Reaps dead child processes 
**
** ------------------------------------------------------------------------*/
static void consume_child()
{
   int stat;
   struct rusage rusage;

   pid_t my_pid;

   do
   {
      my_pid = wait3 (&stat, WNOHANG, &rusage);
   } while (my_pid > 0);

   return;
}
