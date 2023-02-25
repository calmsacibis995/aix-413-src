static char sccsid[] = "@(#)63  1.28  src/bos/usr/sbin/install/ckprereq/load_fix_info.c, cmdinstl, bos41J, 9521A_all 5/23/95 16:23:52";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *            load_fix_info_table 
 *            read_product_vpd 
 *            read_current_product_vpd
 *            merge_root_usr_vpd_entries 
 *            merge_sop_and_toc_lists
 *            merge_vpd_and_option_lists
 *
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

/*
 * Includes
 */


#include <check_prereq.h>

static void merge_vpd_and_option_lists (
fix_info_type * vpd_list,
fix_info_type * option_list,
boolean       * error);

static void merge_sop_and_toc_lists (
fix_info_type * sop_list,
fix_info_type * toc_list,
boolean       * error);

static void merge_root_usr_vpd_entries (
fix_info_type * root_list,
fix_info_type * usr_list,
boolean       * error);

static void load_current_product_vpd (
fix_info_type * fix_list,
short           vpd_source,
boolean       * error);

static void read_product_vpd (
prod_t        * product_info,
int             flags,
fix_info_type * fix_header,
boolean       * error);

/*--------------------------------------------------------------------------*
**
** NAME: load_fix_info_table
**
** FUNCTION:  Reads the entire VPD and places all relevant information into
**            a possible_fix list.  This speeds things up by not having to go
**            to ODM and do i/o every time we need something.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void 
load_fix_info_table 
 (
boolean   called_without_sop,  /* Used to ommit costly ops when called by
                                  listing programs and under certain conditions
                                  for non-apply ops from inu_ckreq () */
boolean * error
)
{
   boolean         found;
   char            level_buf [MAX_LEVEL_LEN];
   Option_t      * first_option;
   Option_t      * option;
   fix_info_type * fix;
   fix_info_type * fix2;
   fix_info_type * next_fix;
   fix_info_type * option_list;
   fix_info_type * prev;
   fix_info_type * root_list;
   fix_info_type * sop_list;
   fix_info_type * toc_list;
   fix_info_type * usr_list;
   fix_info_type * vpd_list;

   FILE *	   oem_fp;
   lpp_t	   oem_lpp;


   /* Cheap calls if the path is already set. */

   vpdlocalpath (ROOT_VPD_PATH);
   vpdremotepath (USR_VPD_PATH);

                     /* First read the USR part. */

   vpdremote ();

   usr_list = create_fix_info_node (error);
   RETURN_ON_ERROR;

   load_current_product_vpd (usr_list, LPP_USER, error);
   RETURN_ON_ERROR;

                     /* Next read the ROOT part. */

   vpdlocal ();

   root_list = create_fix_info_node (error);
   RETURN_ON_ERROR;

   load_current_product_vpd (root_list, LPP_ROOT, error);
   RETURN_ON_ERROR;

   /* Now merge the USR and ROOT info.  The root_list will be empty upon
      return.  */

   merge_root_usr_vpd_entries (root_list, usr_list, error);
   RETURN_ON_ERROR;

   if (check_prereq.debug_mode == 2)
    {
      check_prereq.fix_info_anchor = usr_list -> next_fix;
      graph_dump ("usr/root vpd lists");
      check_prereq.fix_info_anchor = NIL (fix_info_type);
    }

   /* Finally, read the SHARE part. */

   vpdremotepath (SHARE_VPD_PATH);
   vpdremote ();

   load_current_product_vpd (usr_list, LPP_SHARE, error);
   RETURN_ON_ERROR;

   vpd_list = usr_list;

   if (check_prereq.debug_mode == 2)
    {
      check_prereq.fix_info_anchor = vpd_list -> next_fix;
      graph_dump ("usr/root/share vpd lists");
      check_prereq.fix_info_anchor = NIL (fix_info_type);
    }

   /* Now get everything we know about the TOC (if we have one) or the SOP
      and merge it together with we we found out from the vpd. */


   /* Now get everything from the SOP and TOC (if we have one) and put them
      on lists.  Then, merge these lists, keeping SOP entries in preference
      to TOC entries. */

   sop_list = create_fix_info_node (error);
   RETURN_ON_ERROR;

   if (check_prereq.SOP != NIL (Option_t))
    {
      for (option = check_prereq.SOP -> next;
           option != NIL (Option_t);
           option = option -> next)
       {
         copy_TOC_to_fix_info (option, & fix, COMMAND_LINE, error);
         RETURN_ON_ERROR;

         fix -> next_fix = sop_list -> next_fix;
         sop_list -> next_fix = fix;
       } /* end for */
    }

   /* Now put everything from the TOC on a fix_info structure list. */

   toc_list = create_fix_info_node (error);
   RETURN_ON_ERROR;

   if (check_prereq.First_Option != NIL (Option_t))
    {
      for (option = check_prereq.First_Option -> next;
           option != NIL (Option_t);
           option = option -> next)
       {
         if (option -> bff -> action != ACT_UNKNOWN)
          {
            copy_TOC_to_fix_info (option, & fix, TOC, error);
            RETURN_ON_ERROR;

            fix -> next_fix = toc_list -> next_fix;
            toc_list -> next_fix = fix;
          }
       } /* end for */
    } /* end if */

   if (check_prereq.debug_mode == 2)
    {
      check_prereq.fix_info_anchor = sop_list -> next_fix;
      graph_dump ("sop list");

      check_prereq.fix_info_anchor = toc_list -> next_fix;
      graph_dump ("toc list");

      check_prereq.fix_info_anchor = NIL (fix_info_type);
    }

   if (! called_without_sop)
   {
      /* The sop_list will be empty on exit. */
      merge_sop_and_toc_lists (sop_list, toc_list, error);
      RETURN_ON_ERROR;
   }

   option_list = toc_list;

   if (check_prereq.debug_mode == 2)
    {
      check_prereq.fix_info_anchor = option_list -> next_fix;
      graph_dump ("combined sop/toc lists");

      check_prereq.fix_info_anchor = NIL (fix_info_type);
    }

   if ((toc_list->next_fix != NIL (fix_info_type)) || ! called_without_sop)
   {
      /* The vpd_list will be empty on exit. */
      merge_vpd_and_option_lists (vpd_list, option_list, error);
      RETURN_ON_ERROR;
   }
   else
      option_list = vpd_list;

   /* Now put everything into the fix_info table. */

   /*  Create a dummy fix node that is the parent of all other fixes.  This
       makes traversing the fixes according to prereq order possible.  In
       addition, it creates a node to hang coreqs to the selected options off
       of as prereqs, but this will be done later.  */

   check_prereq.fix_info_anchor = create_fix_info_node (error);
   RETURN_ON_ERROR;
   check_prereq.fix_info_anchor->flags |= SUCCESSFUL_NODE;
   check_prereq.fix_info_anchor->next_fix = option_list->next_fix;
   free_fix_info_node (option_list);
   check_prereq.fix_info_anchor->prev_fix = NIL (fix_info_type);
   /*
    * NOTE: ~ is included in the name to make the name of this entry always
    * be less than any valid option in the fix_info table (for list traversal
    * purposes on name).
    */
   check_prereq.fix_info_anchor->name = dupe_string ("~FIX_LIST_ANCHOR", error);
   RETURN_ON_ERROR;

   /*
    * A sorted fix_info table is key to implicit supersedes and base level
    * handling.  Do that now.
    */ 
   sort_fix_info_table (error);
   RETURN_ON_ERROR;

   /*
    * Set the apply/commit/reject states of each node we are keeping and 
    * create previous links.
    */
   prev = check_prereq.fix_info_anchor;
   for (fix = check_prereq.fix_info_anchor->next_fix;
        fix != NIL (fix_info_type);
        fix = fix -> next_fix)
   {
      fix->prev_fix = prev;
      prev = fix;
      figure_out_state_of_fix (fix, error);
      RETURN_ON_ERROR;

      /*
       * Here (for want of a better place :( ) we make sure that
       * a fix requiring a root part only apply does not point
       * to a toc entry.  Processing in determine_order_of_fixes()
       * depends upon this.
       */
      if ((fix->TOC_Ptr != NIL (Option_t))              &&
          (check_prereq.parts_to_operate_on & LPP_ROOT) &&
          (fix->parts_in_fix == (LPP_USER | LPP_ROOT))  &&
          (fix->parts_applied == LPP_USER)              &&
          (! check_prereq.Force_Install))
         fix->TOC_Ptr = NIL (Option_t);

   } /* end for */

   /* Check to see if this is an OEM-modified system. */
   oem_fp = fopen ("/usr/lib/objrepos/oem", "r");
 
   /*
    * Make one more pass of the fix_info table to see if 4.1 filesets
    * requested at the command line are already installed via an implicit 
    * superseding fileset.  We only do this for non-root only apply ops.
    * (For root only apply ops, we currently install all superseded root
    * parts that are installed in the usr part. )
    */
   if ((check_prereq.mode == OP_APPLY)                && 
       (check_prereq.parts_to_operate_on != LPP_ROOT) &&
       (! check_prereq.called_from_ls_programs))
   {
      for (fix = check_prereq.fix_info_anchor->next_fix;
           fix != NIL (fix_info_type);
           fix = fix -> next_fix)
      {
         if ((IF_4_1 (fix->op_type))      
                          &&
             /*
              * Make sure we're not doing a force re-install of 4.1 base levels.
              * We don't want them to fail as "already installed."
              */
             ((! IF_INSTALL(fix->op_type)) || 
              (!check_prereq.Force_Install)) 
                          &&
             (fix->origin == COMMAND_LINE))
         {
            /*
             * Sub-loop to see if the requested update has a lower vrmf
             * than one already installed.
             */
            fix2 = fix->next_fix;
            found = FALSE;
            while (fix2 != NIL (fix_info_type))
            {
               if (strcmp (fix2->name, fix->name) != 0)
                  break;

               if ((inu_level_compare (&fix2->level, &fix->level) > 0) &&
                   ((fix2->parts_applied > 0) || (fix2->apply_state == BROKEN)))
               {
                  found = TRUE;
                  break;
               }
               fix2 = fix2->next_fix;

            } /* while */

            if (found && is_compat_levels (fix2->name,
                                           fix2->supersedes_string, 
                                           &fix->level))
            {
               if (fix->flags & EXPL_REQUESTED_PKG)
               {
                  option = move_failed_sop_entry_to_fail_sop (fix, 
                                                  STAT_ALREADY_SUPERSEDED);
                  option->supersedes = strdup (get_level_from_fix_info (
                                                      fix2, level_buf));
               }
               if (IF_INSTALL (fix2->op_type))
                  fix->apply_state = SUPD_BY_NEWER_BASE;
               else
                  fix->apply_state = SUPD_BY_NEWER_UPDT;
               fix->origin = TOC;   
            }     
         } /* end if */

         /*
          * If this is an OEM-specific system, then make sure that
          * we do not install any generic base levels on top of OEM-modified
          * filesets unless the force flag is specified.
          */
         if ((oem_fp != NIL (FILE)) &&
             (IF_INSTALL(fix->op_type)) && 
             (!check_prereq.Force_Install) &&
             (fix->parts_applied > 0))
	 {
	    int rc = 0;

	    /*
	     * Check to see if the fileset is OEM-modified
	     * Unfortunately, this information is kept in the LPP table, so
	     * can't just check the cumulative cp_flag from the fix record.
	     */
   	    memset (&oem_lpp, NULL, sizeof (lpp_t));

            strcpy (oem_lpp.name, fix->name);
	    /* Make sure that we check the right vpd */
            if (fix->parts_in_fix == (LPP_USER | LPP_ROOT)) {
		vpdlocal();
	    } else if (fix->parts_in_fix == LPP_USER) {
   		vpdremotepath (USR_VPD_PATH);
		vpdremote();
	    } else {
   		vpdremotepath (SHARE_VPD_PATH);
		vpdremote();
	    }
   	    rc = vpdget (LPP_TABLE, LPP_NAME, &oem_lpp);
   	    vpd_free_vchars (LPP_TABLE, &oem_lpp);
   	    if ((rc == VPD_SYS) || (rc == VPD_SQLMAX))
	    {
       	        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E,
			CMN_VPDERR_D), INU_INSTALLP_CMD);
		*error = TRUE;
		return;
	    }

	    if (rc != VPD_NOMATCH)
	    {
		if (!(oem_lpp.cp_flag & LPP_OEM_SPECIFIC)) 
		    continue;
	    }

            fix2 = fix->next_fix;
            while (fix2 != NIL (fix_info_type))
            {
               if (strcmp (fix2->name, fix->name) != 0)
                  break;

              /*
               * Eliminate any generic requested base levels which would
               * overwrite an OEM-modified fileset.
               */
               if ((IF_INSTALL(fix2->op_type)) &&
                   (fix2->origin == COMMAND_LINE) &&
		   (strncmp(fix2->TOC_Ptr->lang,"oem_",4)))
               {
                   move_failed_sop_entry_to_fail_sop (fix2, 
						      STAT_OEM_BASELEVEL);
                   fix2->flags |= FAILED_OEM_BASELEVEL;
                   fix2->origin = TOC;   
               }
               fix2 = fix2->next_fix;

            } /* while */
	 }

      } /* end for */
   } /* end if */

   create_hash_table (error);
   RETURN_ON_ERROR;

   load_fix_hash_table (check_prereq.fix_info_anchor, error);
   RETURN_ON_ERROR;

   if (check_prereq.debug_mode)
      graph_dump ("load_fix_info: before processing supersede info.");

   build_expl_supersede_info (error);
   RETURN_ON_ERROR;

   if (check_prereq.called_from_ls_programs)
   {
      /*
       * The following routine creates prereq links from 4.1 updates to 
       * their base levels.  We do this here when load_fix_info is called
       * by the "ls" programs -- lslpp, lppchk, installp -L.  We do not do 
       * it here when called to do installp processing, since we want to call 
       * evaluate_base_levels() first (to process multiple occurrences of 
       * base levels being considered).
       */
      build_base_prereqs (error);
      RETURN_ON_ERROR;
   }

} /* end load_fix_info_table */

/*--------------------------------------------------------------------------*
**
** NAME: merge_vpd_and_option_lists
**
** FUNCTION:  Combine the information from the vpd, sop and toc and keep only
**            what is relevant.
**
**            Two lists are passed as arguments: one representing combined 
**            information from the SOP (the list of packages requested by 
**            the user) and TOC (if a toc is present), the other representing 
**            information from the VPD.  Our two lists may contain fix_info 
**            structures representing the same package (eg.  a package on the 
**            installation media may already be installed).  We'll examine the 
**            contents of the vpd list, look up matching structures in the 
**            SOP/TOC list, then throw away one of the two dupes, keeping any
**            pertinent information.  We NEVER want two structures representing
**            the same pkg on our fix_info list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
merge_vpd_and_option_lists 
(
fix_info_type * vpd_list,
fix_info_type * option_list,
boolean       * error
)
{
   boolean           delete_option_fix;
   boolean           delete_vpd_fix;
   char              fix_name[MAX_LPP_FULLNAME_LEN];
   fix_info_type   * next_vpd;
   fix_info_type   * option;
   fix_info_type   * previous_option;
   fix_info_type   * vpd;
   index_list_type * index_node;
   boolean           vpd_fix_found;

   /*
    * Optimize the process by hashing one of the lists.
    */
   create_hash_table (error);
   RETURN_ON_ERROR;
   load_fix_hash_table (option_list, error);
   RETURN_ON_ERROR;

   vpd_fix_found = FALSE;

   /*
    * Iterate on the vpd list, look up matching entries in the SOP/TOC list.
    */
   for (vpd = vpd_list -> next_fix;
        vpd != NIL (fix_info_type);
        vpd = vpd -> next_fix)
    {
      delete_option_fix = FALSE;
      delete_vpd_fix = FALSE;

      /*
       * locate_fix will take advantage of the hashed table.
       */
      option = locate_fix (vpd -> name, & (vpd -> level), USE_FIXID );

      if (option != NIL (fix_info_type))
      {
         vpd_fix_found = TRUE;
         /* 
          * FOUND A MATCH! (An entry in the vpd is on the SOP/TOC list)
          */

         if (check_prereq.mode != OP_APPLY) 
         {
            /************************************************************ 
             * NON-APPLY OPERATION: Use structure from vpd_list.  The info
             * in that structure is sufficient for commit/reject/de-install.
             ************************************************************/
            vpd -> origin = COMMAND_LINE;  /* new origin for this structure. */
            delete_option_fix = TRUE;     

         } else {

         if (option -> origin != COMMAND_LINE)
         {
            /***********************************************************
             * APPLY OPERATION, OPTION WAS NOT REQUESTED ON COMMAND LINE.
             * We will not be re-applying this package (via -g) if:
             *    1. Pkg is BROKEN and we don't have required flags to
             *       allow "over-write" installation.  (-acgN)
             *    2. Some or all parts of the pkg is applied OR the pkg
             *       is superseded by another. 
             *    Therefore we'll keep the structure from the vpd_list.
             ***********************************************************/
            if (vpd -> apply_state == BROKEN)
               if (check_prereq.commit && check_prereq.no_save &&
                                                     check_prereq.Auto_Include)
                  delete_vpd_fix = TRUE;
               else
                  delete_option_fix = TRUE;
            else   
               if ((vpd -> parts_applied == 0) &&           /* not applied. */
                   (vpd -> superseding_ptf[0] == '\0')) /* not superseded.*/
                  delete_vpd_fix = TRUE;                    
               else
                  delete_option_fix = TRUE;

         } else {

            /***********************************************************
             * APPLY OPERATION, OPTION WAS REQUESTED ON COMMAND LINE.
             * Use structure from option_list unless the package has
             * some parts applied and we're NOT doing a force install.
             ***********************************************************/

         delete_vpd_fix = TRUE;
         if (check_prereq.Force_Install)
         {
            /*********************************************************
             * APPLY OPERATION, OPTION WAS REQUESTED ON COMMAND LINE,
             * DOING A FORCE INSTALL (-F flag used). 
             *********************************************************/

            /* 
             * Let's perform some error checking on the vpd structure, based
             * on the fact that we are doing a force install.  We do this
             * now because we will be deleting the vpd structure AND because
             * we start to modify its values below.
             */
            inconsistent_state_error_check (vpd, error);
            RETURN_ON_ERROR;

            /* 
             * If we're doing a force install of root parts, let's get
             * the parts_applied information from the vpd_list structure.
             *
             * To achieve this, we wipe out knowledge of any parts that are 
             * applied which we were requested to re-apply.  The only valid
             * results of this operation is to have nothing still applied,
             * or to have a user part still applied (meaning we were only
             * asked to apply root parts).  All other combinations imply
             * inconsistencies (probably in the database) and will be 
             * flagged with an internal error which causes processing to halt.
             */
            vpd -> parts_applied &= ~ check_prereq.parts_to_operate_on;

            if (vpd -> parts_applied != 0)
            {
               #pragma covcc !instr
               if (vpd -> parts_applied == LPP_USER)
               {
                  /* 
                   * Must be doing a root apply.  Save the modified 
                   * parts_applied info and the real parts_committed info.
                   */ 
                  option -> parts_applied = vpd -> parts_applied;
                  option -> parts_committed = vpd -> parts_committed;
               }
               else
               {
                  /*
                   * Must be an internal error.  Perhaps our database indicates
                   * that a package has a share part and a root part OR
                   * perhaps installp didn't catch a request to install share
                   * parts only on a package which doesn't contain share parts
                   * (often a result of a messed up cp_flag in the database).
                   */
                  inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                           CKP_INT_CODED_MSG_D), inu_cmdname, 
                           INCONSISTNT_PARTS_CODE);
                  inu_msg (ckp_errs, "\t%s\n", 
                           get_fix_name_from_fix_info (vpd, fix_name));

                  check_prereq.function_return_code =
                                                    INTERNAL_ERROR_RC;
                  * error = TRUE;
                  return;
               } /* end if vpd -> parts_applied == LPP_USER */
               #pragma covcc instr
            } /* end if (vpd -> parts_applied != 0) */
         }
         else  
         {
            /*********************************************************
             * APPLY OPERATION, OPTION WAS REQUESTED ON COMMAND LINE,
             * NOT DOING A FORCE INSTALL (-F flag NOT used). 
             *********************************************************/
            if ( IF_4_1 (vpd->op_type)    &&
                 IF_UPDATE (vpd->op_type) && 
                 (vpd->apply_state == BROKEN) ) {
               /*
                * installp passes on requests to install BROKEN 4.1 updates 
                * for inu_ckreq() to consider.  Reflect the BROKEN state in 
                * the option entry which we'll be keeping for subsequent 
                * processing. 
                */
              option->apply_state = BROKEN;
              option->parts_applied |= vpd->parts_applied;
              option->parts_committed |= vpd->parts_committed;

            } else {

            if (vpd -> parts_applied != 0)
            {
               /*
                * This pkg is partially applied.  (It can't be fully applied
                * because installp weeds out things that were requested for
                * re-install when -F is not provided).  As a result, we'll
                * reverse our decision made above and use the structure from
                * the vpd_list instead.  Change some key fields in the vpd
                * structure and perform some consistency checks.
                */
               delete_vpd_fix = FALSE;
               delete_option_fix = TRUE;

               vpd -> origin = COMMAND_LINE;
               vpd -> apply_state = TO_BE_EXPLICITLY_APPLIED;
               if (vpd -> parts_applied == LPP_USER)
               {
                  #pragma covcc !instr
                  if (! ((check_prereq.parts_to_operate_on & LPP_ROOT) || 
                         check_prereq.called_from_command_line))
                  {
                     /* 
                      * We need to apply a ROOT part, but we are not
                      * allowed to do that.  Again, this could be a result
                      * of corrupted vpd (as discussed in the comment above
                      * for the internal error on a force apply).
                      */
                     inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                              CKP_INT_CODED_MSG_E, CKP_INT_CODED_MSG_D), 
                              inu_cmdname, INCONSISTNT_PARTS_CODE);
                     inu_msg (ckp_errs, "\t%s\n", 
                              get_fix_name_from_fix_info (vpd, fix_name));
                     check_prereq.function_return_code = INTERNAL_ERROR_RC;
                     * error = TRUE;
                     return;
                  } /* end if */
                  #pragma covcc instr
               } /* end if */
            } /* end if (vpd->parts_applied ... */
            } /* end if ( IF_4_1 (vpd->op_type... */
         } /* end if (check_prereq.mode == FORCE_INSTALL) */
         } /* end if (option -> origin != COMMAND_LINE) */
         } /* end if (check_prereq.mode != OP_APPLY) */

         if (delete_option_fix)
         {

            vpd->flags = option->flags; /* We may set the flags field
                                           in copy_TOC_to_fix_info. */

            /* 
             * Save some info from the option structure.
             */
            if ((vpd->description[0] == '\0') &&
                (option->description[0] != '\0'))
            {
               vpd->description = dupe_string (option->description, error);
               RETURN_ON_ERROR;
            }

            /*
             * This sys_* stuff is needed back in installp.  Preserve it...
             */
            vpd->level.sys_ver = option->level.sys_ver;
            vpd->level.sys_rel = option->level.sys_rel;
            vpd->level.sys_mod = option->level.sys_mod;
            vpd->level.sys_fix = option->level.sys_fix;

            option->flags |= VISITING_THIS_NODE; /* Tag to delete. */
         }

         if (delete_vpd_fix)
         {
            /* 
             * Save some info from the vpd structure.
             */

            /*
             * Get the superseding_ptf string from the vpd entry 
             * (if there is one).  This tells us if an update is superseded
             * by another that is already installed.
             */
            if (vpd->superseding_ptf[0] != '\0')
            {
               strcpy (option->superseding_ptf, vpd->superseding_ptf);
            }

            if ((option->description[0] == '\0') &&
                (vpd->description[0] != '\0'))
            {
               option->description = dupe_string (vpd->description, error);
               RETURN_ON_ERROR;
            } /* end if */

            /*
             * Perform a quick scan of the warnings list, if non-empty, to
             * see if the vpd structure we're about to delete is on that list.
             * Reset to the option list if so and capture the flags field so
             * that we know about the warning.
             */
            for (index_node = report_anchor.misc_warnings_hdr->next_index_node;
                 index_node != report_anchor.misc_warnings_tail;
                 index_node = index_node->next_index_node)
            {
                if (index_node->fix_ptr == vpd)
                {
                   index_node->fix_ptr = option;
                   option->flags |= vpd->flags;
                   break;
                }
            } /* for */

            vpd->flags |= VISITING_THIS_NODE; /* Tag to delete. */
         } 
      } /* end if (option == NIL */
   } /* end for */

   /* 
    * Internal error check:  If we're doing anything other than an
    * apply operation, at least one entry in the vpd list must have 
    * been a selection that was made on the command line (ie. something 
    * being committed or rejected must be in the vpd or else there is 
    * a problem.)
    */

   #pragma covcc !instr
   if ((! vpd_fix_found) && (check_prereq.mode != OP_APPLY))
   {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
               CKP_INT_CODED_MSG_D), inu_cmdname, COMT_REJ_OPT_MISSING_CODE);
      check_prereq.function_return_code = FATAL_ERROR_RC;
      * error = TRUE;
      return;
   }
   #pragma covcc instr

   /* Hash table not needed. */

   destroy_hash_table ();

   /* 
    * At this point we have tagged the entries that we wish to delete from
    * both the vpd and option lists.  Loop through both lists and perform
    * the deletions and/or copies as appropriate.
    */
   option = option_list -> next_fix;
   previous_option = option_list;
   while (option != NIL (fix_info_type))
   {
       if (option -> flags & VISITING_THIS_NODE)   /* Delete me! */
       {
          previous_option -> next_fix = option -> next_fix;
          free_fix_info_node (option);
       }
       else
       {
          previous_option = option;
       }
       option = previous_option -> next_fix;
   } /* end while */


   /* 
    * Now either copy each vpd entry to the option list or delete it, 
    * depending on the tag. 
    */
   vpd = vpd_list -> next_fix;
   while (vpd != NIL (fix_info_type))
   {
       next_vpd = vpd -> next_fix;
       if (vpd -> flags & VISITING_THIS_NODE)   /* Delete me! */
       {
          free_fix_info_node (vpd);
       }
       else
       {
          vpd -> next_fix = option_list -> next_fix;
          option_list -> next_fix = vpd;
       }
       vpd = next_vpd;
   } /* end while */

   /* 
    * Don't need the vpd_list anymore. 
    */
   free_fix_info_node (vpd_list);
   vpd_list = NIL (fix_info_type);

   return;

} /* end merge_vpd_and_option_lists */

/*--------------------------------------------------------------------------*
**
** NAME: merge_sop_and_toc_lists.
**
** FUNCTION:  Combine the two lists.  Entries from the sop list win if there
**            are duplicates.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
merge_sop_and_toc_lists 
 (
fix_info_type * sop_list,
fix_info_type * toc_list,
boolean       * error
)
{
   fix_info_type * previous_toc_fix;
   fix_info_type * sop_fix;
   fix_info_type * toc_fix;

   /* Copy all sop_list entries to the toc_list.  If there is a duplicate entry,
      the sop_list entry wins.  Note that this method will also remove
      duplicate entries in the sop_list, but not from the toc_list.

      installp likes to put duplicate entries on the sop (root and usr). */

   create_hash_table (error);
   RETURN_ON_ERROR;

   load_fix_hash_table (sop_list, error);
   RETURN_ON_ERROR;

   previous_toc_fix = toc_list;
   for (toc_fix = toc_list -> next_fix;
        toc_fix != NIL (fix_info_type);
        toc_fix = toc_fix -> next_fix)
    {
      sop_fix = locate_fix (toc_fix -> name, & (toc_fix -> level), USE_FIXID );
      if (sop_fix != NIL (fix_info_type))
       {
         /* Delete the toc version. */

         previous_toc_fix -> next_fix = toc_fix -> next_fix;
         free_fix_info_node (toc_fix);
         toc_fix = previous_toc_fix;
       }
      else
       {
         previous_toc_fix = toc_fix;
       }
    } /* end for */

   /* Don't need the hash table anymore. */

   destroy_hash_table ();

   /* Now insert the sop fix onto the toc list. */

   previous_toc_fix -> next_fix = sop_list -> next_fix;
   sop_list -> next_fix = NIL (fix_info_type);
   free_fix_info_node (sop_list);
   sop_list = NIL (fix_info_type);

   return;

} /* end merge_sop_and_toc_lists */

/*--------------------------------------------------------------------------*
**
** NAME: merge_root_usr_vpd_entries
**
** FUNCTION:  This routine combines fix information retrieved from the ROOT
**            and USR vpds.
**
**            On entry to this routine, we have two lists.  Each list contains
**            state information for each product/fix applied, superseded, or
**            seen (AVAILABLE) by this system.  We want to merge the two lists
**            together so that we have only one list representing the complete
**            state.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
merge_root_usr_vpd_entries 
(
fix_info_type * root_list,
fix_info_type * usr_list,
boolean       * error
)
{
   char            fix_name[MAX_LPP_FULLNAME_LEN];
   fix_info_type * command_line_entry;
   fix_info_type * previous_usr_fix;
   fix_info_type * root_fix;
   fix_info_type * usr_fix;
   fix_info_type * vpd_entry;


   previous_usr_fix = usr_list; /* Seed value in the very weird case that
                                   the root_list is empty. */

   /* To speed things up so that we don't have an n**2 algorythim, use a hash
      table.  Since the root table is much shorter than the usr table, build
      a hash table for the root. */

   create_hash_table (error);
   RETURN_ON_ERROR;

   load_fix_hash_table (root_list, error);
   RETURN_ON_ERROR;

   usr_fix = usr_list -> next_fix;
   previous_usr_fix = usr_list;
   while (usr_fix != NIL (fix_info_type))
    {
      root_fix = locate_fix (usr_fix -> name, & (usr_fix -> level), USE_FIXID );

      if (root_fix != NIL (fix_info_type))
       {
         /* We are going to merge the info from root_fix and usr_fix into
            root_fix, then discard usr_fix. */

        /*--------------------------------------------------------------*
         *   The following "if" statement takes care of defect 98648. 
         *   During the merge of the usr and root part lists the description
         *   was not being stored. As a result, lslpp was not printing
         *   the description for the subsystems which had usr and root
         *   parts.
         *--------------------------------------------------------------*/

           /* Let's make sure we have a description to print in our
               messages. */

         if (root_fix -> description[0] == '\0')
         {
            if (usr_fix -> description[0] != '\0')
            {
                root_fix->description = usr_fix->description;
                usr_fix->description = NIL (char);                          
            } /* end if */
         }

         /*   End of fix for defect 98648 */

         if ((root_fix -> superseding_ptf[0] == '\0')
                               &&
              (usr_fix -> superseding_ptf[0] != '\0'))
          {
            /* Let's not complain about having supersedes info in the usr
               but not the root.  Rather, we should copy the
               superseding_ptf string to the root fix_info node if the usr
               part is actively superseded even if the root part is not
               actively superseded (we're gonna try to sync things up). */

               strcpy (root_fix -> superseding_ptf, usr_fix -> superseding_ptf);
          }
         else
          {
            if ((usr_fix -> superseding_ptf[0] == '\0')
                                     &&
                 (root_fix -> superseding_ptf[0] != '\0'))
             {
                    
               /* We'll warn about having root but NOT usr, if -v flag 
                  specified installp. */
               root_fix->flags |= WARN_NO_USR_SUPS_INFO;
               if ((!check_prereq.called_from_ls_programs) &&
                   (check_prereq.instp_verify))
               {
                  add_entry_to_index_list (report_anchor.misc_warnings_hdr,
                                           root_fix, NIL (Option_t),
                                           &EMPTY_CRITERIA, error);
                  RETURN_ON_ERROR;
               }
              }
            else
              {
                if (! strcmp_non_wsp (root_fix -> superseding_ptf,
                                      usr_fix -> superseding_ptf))
                 {

		  /* Let's not complain about having prereq info in the usr but
                       not the root. */
                      my_free (root_fix -> prereq_file);
                      root_fix -> prereq_file = strdup(usr_fix -> prereq_file);

                 }
              }
           }

          /*
           * Skip all consistency checks for prereq info in usr and root
           * when called to do lslpp or an installp listing.
           */
          if (! check_prereq.called_from_ls_programs)
          {
             if ((root_fix -> prereq_file[0] == '\0')
                                   &&
                  (usr_fix -> prereq_file[0] != '\0'))
             {
                /* Let's not complain about having prereq info in the usr but
                   not the root. */

                my_free (root_fix -> prereq_file);
                root_fix -> prereq_file = usr_fix -> prereq_file;
                usr_fix -> prereq_file = NIL (char);
             }
             else
             {
                if ((usr_fix -> prereq_file[0] == '\0')
                                      &&
                     (root_fix -> prereq_file[0] != '\0'))
                {
                   /* We'll warn about having root but NOT usr if -v flag 
                      specified to installp. */
                  root_fix->flags |= WARN_NO_USR_PRQ_INFO;
                  if ((! check_prereq.called_from_ls_programs) &&
                      (check_prereq.instp_verify))
                  {
                     add_entry_to_index_list (report_anchor.misc_warnings_hdr,
                                              root_fix, NIL (Option_t),
                                              &EMPTY_CRITERIA, error);
                     RETURN_ON_ERROR;
                  }
                }
                else
                {
                   if (! strcmp_non_wsp (root_fix -> prereq_file,
                                         usr_fix -> prereq_file))
                   {
                   /* Error out if they both exist but are different. */
           /*"Error in the Vital Product Data.  The \"usr\" part of a
              product does not have the same requisite file as the \"root\"
              part.\n The product is: %s\n"*/

                      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                               CKP_INCONSISTNT_REQS_E,
                               CKP_INCONSISTNT_REQS_D), inu_cmdname,
                            get_fix_name_from_fix_info (root_fix, fix_name));

                      * error = TRUE;
                      return;
                   }
                }
             }
          }
     
          /*
           * Get the supersedes_string from the usr part if none in the
           * root part.
           */ 
          if (root_fix->supersedes_string[0] == '\0' &&
              usr_fix->supersedes_string[0] != '\0')
          {
             root_fix->supersedes_string = strdup (usr_fix->supersedes_string);
          }

          /*
           * Normally, we want to merge the usr and root parts_applied/commitd
           * information from the root and usr fix into the usr fix.
           * There's a special case to consider.  When the root fix says that
           * it is an available record with usr and root parts while the usr
           * fix says that it is a usr-only fix we want to make the merged
           * records look like a usr-root fix for deinstall and like a
           * usr-only fix for non-deinstall ops (when it will simply be
           * satisfying requisites).
           */
          if (IF_INSTALL (usr_fix->op_type)    &&
              (usr_fix->parts_in_fix == LPP_USER)  &&
              (root_fix->parts_in_fix == (LPP_ROOT | LPP_USER)) &&
              (root_fix->parts_applied == 0))
          {
             if (check_prereq.deinstall_submode)
             {
                /*
                 * Fake the parts applied/committed info for a deinstall.
                 */
                if (usr_fix->parts_applied == LPP_USER)
                   root_fix->parts_applied = (LPP_USER | LPP_ROOT);

                if (usr_fix->parts_committed == LPP_USER)
                   root_fix->parts_committed = (LPP_USER | LPP_ROOT);
             }
             else
             {
                /*
                 * We're keeping the root fix, make it look like the
                 * usr fix.
                 */
                root_fix->parts_applied = usr_fix->parts_applied;
                root_fix->parts_committed = usr_fix->parts_committed;
                root_fix->parts_in_fix = usr_fix->parts_in_fix;
                root_fix->cp_flag = usr_fix->cp_flag;
             }
          }

          else
          {
             root_fix -> parts_applied |= usr_fix -> parts_applied;
             root_fix -> parts_committed |= usr_fix -> parts_committed;
          }

          root_fix->cp_flag |= usr_fix->cp_flag;

          if (usr_fix -> apply_state == BROKEN)
             root_fix -> apply_state = BROKEN;

          previous_usr_fix -> next_fix = usr_fix -> next_fix;
          free_fix_info_node (usr_fix);
        }
       else
        {
          previous_usr_fix = usr_fix;
        }  /* end if */

       usr_fix = previous_usr_fix -> next_fix;

    } /* end while */


   /* Now we have combined all of the root entries with matching usr entries.
      The duplicates were removed off of the usr list and the information
      merged with the root list entries.  Now insert the root_list at the
      head of the usr_list. */

     previous_usr_fix -> next_fix = root_list -> next_fix;
     root_list -> next_fix = NIL (fix_info_type);

     free_fix_info_node (root_list);
     root_list = NIL (fix_info_type);

     destroy_hash_table ();
} /* end merge_root_usr_vpd_entries */

/*--------------------------------------------------------------------------*
**
** NAME: load_current_product_vpd
**
** FUNCTION:  For the given vpd_source, reads the entire VPD and places all
**            relevant information into a possible_fix list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
load_current_product_vpd 
 (
fix_info_type * fix_list,
short           vpd_source,
boolean       * error
)
{
   prod_t  product_info;


   product_info.state = ST_AVAILABLE;
   read_current_product_vpd (& product_info, PROD_STATE, fix_list, vpd_source,
                             error);
   RETURN_ON_ERROR;

   product_info.state = ST_BROKEN;
   read_current_product_vpd (& product_info, PROD_STATE, fix_list, vpd_source,
                             error);
   RETURN_ON_ERROR;

   product_info.state = ST_APPLIED;
   read_current_product_vpd (& product_info, PROD_STATE, fix_list, vpd_source,
                             error);
   RETURN_ON_ERROR;

   product_info.state = ST_COMMITTED;
   read_current_product_vpd (& product_info, PROD_STATE, fix_list, vpd_source,
                             error);
   RETURN_ON_ERROR;

   if (check_prereq.called_from_command_line ||
       check_prereq.called_from_ls_programs)
    {
      product_info.state = ST_APPLYING;
      read_current_product_vpd (& product_info, PROD_STATE, fix_list,
                                vpd_source, error);
      RETURN_ON_ERROR;

      product_info.state = ST_COMMITTING;
      read_current_product_vpd (& product_info, PROD_STATE, fix_list,
                                vpd_source, error);
      RETURN_ON_ERROR;

      if (check_prereq.called_from_ls_programs)
      {
         product_info.state = ST_REJECTING;
         read_current_product_vpd (& product_info, PROD_STATE, fix_list,
                                   vpd_source, error);
         RETURN_ON_ERROR;

         product_info.state = ST_DEINSTALLING;
         read_current_product_vpd (& product_info, PROD_STATE, fix_list,
                                   vpd_source, error);
         RETURN_ON_ERROR;
      } 

   } /* end if */

} /* end load_current_product_vpd */

/*--------------------------------------------------------------------------*
**
** NAME: read_product_vpd
**
** FUNCTION: Reads the product vpd databases for all entries that match
**           the criterial specifed in product_info and flags.  The entries
**           are added into the fix list pointed to by the fix_header.
**
** RETURNS:  This is a void function.
**
**--------------------------------------------------------------------------*/

static void 
read_product_vpd 
 (
prod_t        * product_info,
int             flags,
fix_info_type * fix_header,
boolean       * error
)
{
   /* Cheap calls if the path is already set. */

   vpdlocalpath (ROOT_VPD_PATH);
   vpdremotepath (USR_VPD_PATH);

   vpdremote ();
   read_current_product_vpd (product_info, flags, fix_header, (short) LPP_USER,
                             error);
   RETURN_ON_ERROR;

   vpdlocal ();
   read_current_product_vpd (product_info, flags, fix_header, (short) LPP_ROOT,
                             error);
   RETURN_ON_ERROR;

   vpdremotepath (SHARE_VPD_PATH);
   vpdremote ();
   read_current_product_vpd (product_info, flags, fix_header, (short) LPP_SHARE,
                             error);
   RETURN_ON_ERROR;

} /* end read_product_vpd */

/*--------------------------------------------------------------------------*
**
** NAME: read_current_product_vpd
**
** FUNCTION: Reads the current product vpd database for all entries that match
**           the LPP name.  The entries are added into the possible_fix list
**           pointed to by the fix_header.
**
** RETURNS:  This is a void function.
**
**--------------------------------------------------------------------------*/

void read_current_product_vpd 
 (
prod_t        * product_info,
int             flags,
fix_info_type * fix_header,
short           vpd_source,
boolean       * error
)
{
   int status;

   status = vpdget (PRODUCT_TABLE, flags, product_info);
   while (status != VPD_NOMATCH)
    {
      switch (status)
       {
         case VPD_OK :
            if ( ! ((check_prereq.ignore_AVAILABLE_entries)
                                       &&
                     (product_info -> state == ST_AVAILABLE)
                                       &&
                     (product_info -> sceded_by[0] == '\0')))
             {
               copy_vpd_to_fix_info (product_info, fix_header, vpd_source,
                                     error);
               RETURN_ON_ERROR;
             }

            vpd_free_vchars (PRODUCT_TABLE, product_info);
            break;

         case VPD_NOMATCH :
            break;

         #pragma covcc !instr
         case VPD_SYS :

            /* A system error occured while attempting
                 to access the Software Vital Product Data database.
                 Use local problem reporting procedures. */

            inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                     CMN_VPDERR_D), inu_cmdname);
            check_prereq.function_return_code = VPD_SYS_ERROR_RC;
            * error = TRUE;
            return;
         #pragma covcc instr

         #pragma covcc !instr
         case VPD_SQLMAX :

            /* An internal error occured while attemting
                  to access the Software Vital Product Data database.
                  Use local problem reporting procedures. */

            inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_IVPDERR_E, 
                     CMN_IVPDERR_D), inu_cmdname);
            check_prereq.function_return_code = VPD_INTERNAL_ERROR_RC;
            * error = TRUE;
            return;
         #pragma covcc instr

       } /* end switch */

      status = vpdgetnxt (PRODUCT_TABLE, product_info);
    } /* end while */

   return;

} /* end read_current_product_vpd */
