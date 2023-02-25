static char sccsid[] = "@(#)76  1.29  src/bos/usr/sbin/install/installp/inu_remove_opts.c, cmdinstl, bos412 9/24/94 17:06:49";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: blast_redundant_mig_files 
 *              create_installed_list 
 *              create_migrating_list
 *              inu_remove_one_opt
 *              inu_clean_libdir 
 *              inu_clean_lpp_libdir
 *              inu_remove_opts
 *              process_installed_list
 *              process_namelist
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <installp.h>
#include <instl_options.h>

void blast_redundant_mig_files (char *opname);

static boolean create_installed_list (Option_t * sop, 
                                      Option_t * next_op, 
                                      boolean doing_root);

static boolean create_migrating_list (Option_t * sop, 
                                      Option_t * next_op, 
                                      boolean doing_root);


static boolean process_installed_list (char *old_op_name, 
                                       char *installed_list_file);

static boolean process_namelist (char *option, char *list_file);

static boolean call_mig_rm_install (char * opname, char *installed_list);

static boolean option_is_in_file  (char *opname, char *fnam);


#define   DUMMY_VAL  99     /* any value not returned by inu_level_compare */

/*=================================================================*/
void inu_remove_all_save_files (int tree, Option_t * sop, Option_t * next_op)
{
   int             rc;
   prod_t          prod;
   Option_t       *op;
   Option_t       *op_t;           /* temporary option for getting the savdir */
   char            savdir[PATH_MAX];

   for (op = sop; op != next_op; op = op->next)
   {
      strcpy (prod.lpp_name, op->name);
      prod.state = ST_APPLIED;

      rc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE, &prod);

      while (rc == VPD_OK)
      {
         op_t = conv_prod_t_Option_t (&prod);
         inu_get_savdir (tree, op_t, savdir);
         rm_1_opt (savdir, op_t->name);

         free (op_t);
         vpd_free_vchars (PRODUCT_TABLE, &prod);
         rc = vpdgetnxt (PRODUCT_TABLE, &prod);
      }
   }

} /* inu_remove_all_save_files */


/*=================================================================*/
void inu_remove_save_files (Option_t * sop, 
                            Option_t * next_op)
{
   Option_t       *op;
   char            savdir[PATH_MAX];
   char            lppname[MAX_LPP_NAME];

   for (op = sop;
        op != next_op;
        op = op->next)
   {
      inu_get_savdir (op->vpd_tree, op, savdir);
      strcpy (lppname, op->name);
      rm_1_opt (savdir, lppname);
   } /* for */

} /* inu_remove_save_files */


/*=================================================================*/
boolean inu_remove_one_opt (TOC_t    * toc, 
                            int        tree, 
                            Option_t * sop, 
                            Option_t * next_op)
{
   char            buf[LINE_MAX];
   char            lppname[MAX_LPP_NAME];
   char            option_list[PATH_MAX];
   prod_t          prod;
   lpp_t           lpp;
   int             rc=SUCCESS;
   Option_t        *op;

   if (chdir (INU_LIBDIR) != 0) {
       perror ("chdir");
       instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                                      CMN_CANT_CHDIR_D), 
                         INU_INSTALLP_CMD, INU_LIBDIR);
       return (FAILURE);
   }

   inu_get_prodname (sop, lppname);

   for (op = sop; op != next_op; op = op->next)
      op->Status = STAT_SUCCESS;

   /* build the option file */

   if (inu_mk_opt_fil (sop, next_op, STAT_SUCCESS, NULL) != SUCCESS)
       return (FAILURE);

 
   /* remove the files from the file system */

   strcpy (option_list, INU_TMPDIR);
   strcat (option_list, "/lpp.options");
   sprintf (buf, "/usr/lib/instl/rminstal %s OPTIONLIST %s", lppname, 
            option_list);
   if ((rc = inu_do_exec (buf)) == INUSYSERR)
      inu_quit (INUSYSERR);

   if (rc != 0)
       return (FAILURE);

   inu_remove_all_save_files (tree, sop, next_op);

   for (op = sop; op != next_op; op = op->next)
   {
      strcpy (prod.lpp_name, op->name);
      rc = vpddelall (PRODUCT_TABLE, PROD_LPP_NAME, &prod);
      inu_ck_vpd_sys_rc (rc);
      strcpy (lpp.name, op->name);
      rc = vpddelall (LPP_TABLE, LPP_NAME, &lpp);
      inu_ck_vpd_sys_rc (rc);
   }

   return (SUCCESS);

} /* inu_remove_one_opt */


/*=================================================================*/
void inu_clean_libdir (Option_t * sop, 
                       Option_t * next_op)
{
   char            libdir[PATH_MAX];
   Option_t      * op;

   inu_set_libdir (sop->vpd_tree, sop);
   strcpy (libdir, "/usr/bin/rm -f ");
   strcat (libdir, INU_LIBDIR);
   strcat (libdir, "/* ");

   /* Could we have a root to dispose of also ? */

   if (sop->vpd_tree == VPDTREE_USR)
   {
      for (op = sop; op != next_op; op = op->next)
      {
         if (IF_3_1 (op->op_type)  ||  op->content == CONTENT_BOTH)
         {
            inu_set_libdir (VPDTREE_ROOT, sop);
            strcat (libdir, INU_LIBDIR);
            strcat (libdir, "/* ");
            inu_set_libdir (sop->vpd_tree, sop);
            break;
         } /* end if */
      } /* end for */
   } /* end if */

   strcat (libdir, "> /dev/null 2>&1");
   if (inu_do_exec (libdir) == INUSYSERR)
      inu_quit (INUSYSERR);

} /* inu_clean_libdir */

/*=================================================================*/
/* clean the LIBDIR for lpps                                       */
void inu_clean_lpp_libdir (Option_t * sop,
                           Option_t * next_op)
{
   char            libdir[PATH_MAX];
   Option_t      * op;

   inu_set_libdir (sop->vpd_tree, sop);

   rm_lpp_files (INU_LIBDIR);

   for (op = sop; op != next_op; op = op->next)
      rm_apply_files (INU_LIBDIR, op);

   /* Could we have a root to dispose of also ? */

   if (sop->vpd_tree == VPDTREE_USR)
   {
      inu_set_libdir (VPDTREE_ROOT, sop);

      rm_lpp_files (INU_LIBDIR);

      for (op = sop; op != next_op; op = op->next)
      {
         if (IF_3_1 (op->op_type)  ||  op->content == CONTENT_BOTH)
            rm_apply_files (INU_LIBDIR, op);
      }
      inu_set_libdir (sop->vpd_tree, sop);
   }

} /* inu_clean_lpp_libdir */


/*=================================================================*/
boolean inu_remove_opts (TOC_t   * toc, 
                         Option_t * sop, 
                         Option_t * next_op)
{
   Option_t      * op;

   /* forget this if it is an update */

   if (IF_UPDATE (sop->op_type))
      return (SUCCESS);

   inu_set_vpdtree (sop->vpd_tree);
   inu_set_libdir (sop->vpd_tree, sop);

   /* create the installed list file */
   if (create_installed_list (sop, next_op, FALSE) == FAILURE)
      return (FAILURE);

   /*--------------------------------------------------------
   *
   *  Ensure that inu_save_config () in rminstal will have
   *  enough space                                         
   *
   *--------------------------------------------------------*/
   if (getspace_migdir (sop, next_op) == FALSE) 
       return (FAILURE);

   if (inu_remove_one_opt (toc, sop->vpd_tree, sop, next_op) != SUCCESS)
      return (FAILURE);

   /* We are finished if we don't have a root part. */

   if (sop->vpd_tree != VPDTREE_USR)
      return (SUCCESS);
   
   /* ---------------------------------------------
   *
   * We need to determine if the current sop chunk
   * has ANY option which has USR &ROOT parts
   *
   * --------------------------------------------- */

   for (op = sop; op != next_op; op = op->next)
        if (op->content == CONTENT_BOTH) 
            break;

   if (op == next_op)      /* they were all USR part only */
      return (SUCCESS);

   /* If we have a root part burn it also. */

   inu_set_vpdtree (VPDTREE_ROOT);
   inu_set_libdir (VPDTREE_ROOT, sop);

   /* time to create the installed list file for the root part */
   if (create_installed_list (sop, next_op, TRUE) == FAILURE)
      return (FAILURE);

   if (getspace_migdir (sop, next_op) == FALSE) 
       return (FAILURE);

   if (inu_remove_one_opt (toc, VPDTREE_ROOT, sop, next_op) != SUCCESS)
      return (FAILURE);

   return (SUCCESS);

} /* inu_remove_opts */


/* ------------------------------------------------------------------------ *
 *
 *  Function:    process_namelist
 *
 *  Purpose:     See if any member of the namelist file is APPLIED/ COMMITTED.
 *               If it is, add it to the lppoption.installed_list.
 *
 *  Parameters:  option -- Name of the lppoption being processed
 *
 *  Returns:     boolean
 *                   -- SUCCESS
 *                   -- FAILURE
 *
 * ------------------------------------------------------------------------ */

static boolean
process_namelist (char *option, 
                  char *installed_list_file)
{
   char        old_op_name [MAX_LPP_FULLNAME_LEN+1], 
               list_file[PATH_MAX]; /* For migration */
   FILE *fp;

   /* form the namelist file path */
   sprintf (list_file, "%s/%s.namelist", INU_LIBDIR, option); 

   /* if it does not exist success */
   if ( ! path_exists (list_file))
      return (SUCCESS);

   if ((fp=fopen (list_file, "r")) == NIL (FILE)) 
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), INU_INSTALLP_CMD, list_file);
      return (FAILURE);
   }

   while (fscanf (fp, "%s%*\n", old_op_name) != EOF)
      if (process_installed_list (old_op_name, installed_list_file) != SUCCESS)
      {
         fclose (fp);
         return (FAILURE);
      }

   fclose (fp);
   return (SUCCESS);

}


/* ------------------------------------------------------------------------ *
 *
 *  Function:    process_installed_list
 *
 *  Purpose:     append old_op_name to option.installed_list
 *               if old_op_name is APPLIED/ COMMITTED.
 *
 *  Parameters:  installed_list_file --  Name of the insalled_list_file
 *                                       to append to.
 *
 *               old_op_name --  Name of the lppoption under which option
 *                               could already be installed.
 *
 *  Returns:     boolean
 *                   -- SUCCESS
 *                   -- FAILURE
 *
 * ------------------------------------------------------------------------ */

static boolean
process_installed_list (char *old_op_name, 
                        char *installed_list_file)
{
  Level_t     *level = NIL (Level_t);

   if ((level = is_it_applied_or_committed (old_op_name)) != NIL (Level_t)) {

       if ( ! vappend_file (installed_list_file, "%s %d.%d.%d.%d\n", 
                                    old_op_name, level->ver, level->rel, 
                                    level->mod, level->fix)) {
            free (level);
            return (FAILURE);
       }

       free (level);
   }

   return (SUCCESS);
}

/* ------------------------------------------------------------------------ *
 *
 *  Function:    create_installed_list
 *
 *  Purpose:     
 *         Creates the installed_list file for each option
 *         if it is being re-installed or exists with another name.
 *         An entry is added to the lppoption.installed_list if it is
 *               -- a Force re-install of same or newer level, or
 *               -- Migration install
 *
 *         create .option.use_invfile to guide the rminstal script in 
 *         the migration path. 
 *
 *  Parameters:  cur_op  -- ptr to current option
 *
 *               next_op -- ptr to next option on the sop 
 *
 *               doing_root -- TRUE for root part of usr &root cur_ops
 *                             FALSE for all others 
 *
 *  Returns:     boolean
 *                   -- SUCCESS
 *                   -- FAILURE
 *
 * ------------------------------------------------------------------------ */

static boolean 
create_installed_list (Option_t * cur_op, 
                       Option_t * next_op, 
                       boolean doing_root)
{

   Option_t       *op;
   char           list_file[PATH_MAX], /* For migration */
                  name_list[PATH_MAX], /* For migration */
                  *migdir_prefix,       
                  migdir [MAX_LPP_NAME+30]; 
   Level_t        *level;             /* for is_it_applied_or_committed */
   int            levcomp=DUMMY_VAL;  /* anything other than -1, 0, or 1 */


    if (doing_root)   /* to distinguish root part op for a U-R sop chunk */
        migdir_prefix="/lpp";
    else
       if ((migdir_prefix=get_migdir_prefix (cur_op)) == NIL (char))
           return (FAILURE);


    for (op = cur_op; op != next_op; op = op->next) {

         /* -------------------------------------------------------- 
         * If we are doing  root parts of USR-ROOT sop chunk, and
         * the option being processed has USR only, then continue 
         * -------------------------------------------------------- */

         if (doing_root  &&  op->content != CONTENT_BOTH)
             continue;

         sprintf (migdir, "%s/%s", migdir_prefix, op->prodname);
         if (doing_root) {            
              /* create /lpp/lppname */
             if (inu_mk_dir (migdir) != INUGOOD)
                 return (FALSE);
         }

         sprintf (list_file, "%s/%s.installed_list", migdir, op->name);

         /* Make sure there's not already a list_file */
         unlink (list_file);

         /* See if option is already applied (-Fa ?) */
         if ((level = is_it_applied_or_committed (op->name)) != NIL (Level_t))
         {
            /* installed_list is not needed if installing older levels */
            if ( (levcomp=inu_level_compare (&op->level, level)) 
                                                         != OLDER_LEVEL ) 
               if ( ! vappend_file (list_file, "%s %d.%d.%d.%d\n", op->name, 
                                    op->level.ver, op->level.rel, 
                                    op->level.mod, op->level.fix)) 
               {
                  free (level);
                  return (FAILURE);
               }
         }

         /*--------------------------------------------------------
         * Process the namelist if
         * 1. The level being installed is newer and option name
         *    is in the namelist? or 
         * 2. Option is not installed at all
         *-------------------------------------------------------- */

         /* form the namelist file path */
         sprintf (name_list, "%s/%s.namelist", INU_LIBDIR, op->name); 

         if ((level == NIL (Level_t))  || ((levcomp == NEWER_LEVEL) && 
                              option_is_in_file (op->name, name_list)) ) {
            if (process_namelist (op->name, list_file) == FAILURE) {
                if (level != NIL (Level_t))
                    free (level);
                return (FAILURE);
            }
         }

         /*--------------------------------------------------------
         * touch the use_invfile file to guide the rminstal script.
         *--------------------------------------------------------*/
         if ( path_exists (list_file)) {  

             /*----------------------------------------------------
             * touch use_invfile file if not 1to1, i.e. no rm_inv present, 
             * and not -Fa. 
             * This causes rminstal.sh to call mig_rm_install ()
             * instead of inu_rm_install ().
             *-----------------------------------------------------*/
             if ((levcomp != SAME_LEVEL)  &&   
                          call_mig_rm_install (op->name, list_file)) {
                 sprintf (list_file, "%s/%s/.%s.use_invfile", migdir_prefix, 
                                                op->prodname, op->name);
                 if (creat (list_file, 644) == -1) {
                     perror ("creat");
                     return (FAILURE);
                 }
             }
         }

          /*--------------------------------------------------------
          * If installing an older level, don't need to save cfgfiles 
          * or rm_inv since we don't allow reverse-migration
          * Blast the inst_root files also if  they exist.  
          * In case of diskless we don't need it since if we move to
          * an older level on the server, the clients should also move
          * to the same level.
          *-------------------------------------------------------- */
       /*
        Not calling this due to -Fa -Or :- (

          if (levcomp == OLDER_LEVEL  ||  levcomp == DUMMY_VAL)
              blast_redundant_mig_files (op->name);
        */

          if (level != NIL (Level_t))
              free (level);

     }  /* of for */

   return (SUCCESS);

}


/* ------------------------------------------------------------------------ *
 *  Function:    blast_redundant_mig_files 
 *
 *  Purpose:     Remove unecessary files 
 *
 *  Parameters:  opname -- Product option
 *
 *  Global:      INULIBDIR 
 *
 *  Returns:     (void) 
 *               
 *
 * ------------------------------------------------------------------------ */
void
blast_redundant_mig_files (char *opname)
{
   char list_file [PATH_MAX];
   sprintf (list_file, "%s/%s.rm_inv", INU_LIBDIR, opname);
   if (path_exists (list_file))
       unlink  (list_file);

   sprintf (list_file, "%s/inst_root/%s.rm_inv", INU_LIBDIR, opname);
   if (path_exists (list_file))
       unlink (list_file);

   sprintf (list_file, "%s/%s.cfgfiles", INU_LIBDIR, opname);
   if (path_exists (list_file))
       unlink (list_file);

   sprintf (list_file, "%s/inst_root/%s.cfgfiles", INU_LIBDIR, opname);
   if (path_exists (list_file))
       unlink (list_file);

   return;
} 



/* ------------------------------------------------------------------------ *
 *
 *  Function:    call_mig_rm_install 
 *
 *  Purpose:     To figure out if migration path should be taken
 *
 *  Parameters:  opname -- Product option
 *               installed_list  -- file name
 *
 *  Returns:     (boolean)    TRUE
 *                             FALSE
 *
 *
 * ------------------------------------------------------------------------ */
static boolean
call_mig_rm_install (char * opname, char *installed_list)
{
   char file_name [PATH_MAX],
        junk [MAX_LPP_FULLNAME_LEN+1]; 
   FILE *fp;
   int num_entries_in_installed_list=0;

   sprintf (file_name, "%s/%s.rm_inv", INU_LIBDIR, opname);

   /* rm_inv exists exist success */
   if ( path_exists (file_name))
        return (TRUE);

   if ((fp=fopen (installed_list, "r")) == NIL (FILE))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
                           CMN_CANT_OPEN_D), INU_INSTALLP_CMD, installed_list);
      return (FALSE);
   }

   while (fscanf (fp, "%s %*s\n", junk) != EOF) {
         num_entries_in_installed_list++;
	 /* No many-to-1 implicit migrations allowed */
	 /* Also no rm_inv file required if migrating from bos.obj */
         if ((num_entries_in_installed_list > 1 ) ||
             (strcmp(junk, "bos.obj") == 0)) {
            fclose(fp);
            return (TRUE);
         }
   }

   fclose(fp);
   return (FALSE);

}


/* ------------------------------------------------------------------------ *
 *
 *  Function:    option_is_in_file 
 *
 *  Purpose:     To figure out if option exists in file 
 *
 *  Parameters:  opname -- Product option
 *               fnam -- file name
 *
 *  Returns:     (boolean)    TRUE
 *                             FALSE
 *
 *
 * ------------------------------------------------------------------------ */
static boolean
option_is_in_file  (char *opname, char *fnam)
{
   char op [MAX_LPP_FULLNAME_LEN+1]; 
   FILE *fp;

   /* rm_inv exists exist success */
   if ( ! path_exists (fnam))
        return (FALSE);

   if ((fp=fopen (fnam, "r")) == NIL (FILE))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
                           CMN_CANT_OPEN_D), INU_INSTALLP_CMD, fnam);
      return (FALSE);
   }

   while (fscanf (fp, "%s%*\n", op) != EOF) {
         if (strcmp (op,opname) == 0) {
            fclose(fp);
            return (TRUE);
         }
   }

   fclose(fp);
   return (TRUE);

}


