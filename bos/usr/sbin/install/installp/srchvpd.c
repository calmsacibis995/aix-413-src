static char sccsid[] = "@(#)14  1.74  src/bos/usr/sbin/install/installp/srchvpd.c, cmdinstl, bos41J, 9513A_all 3/23/95 16:01:15";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_bld_sop_vpd
 *              add_to_sop
 *              check_root_vpd
 *              conv_prod_t_Option_t
 *              get_level
 *              grep_option_vpd
 *              deinstallable
 *              get_next_vpd
 *              is_it_applied_or_committed
 *              inu_update_fixdata
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *--------------------------------------------------------------------------*/

#include <inulib.h>
#include <installp.h>
#include <instl_options.h>

#define INIT_VPD       0    /* indicates first call to get_next_vpd */
#define ROOT_VPD       1    /* processing ROOT VPD */
#define USR_VPD        2    /* processing USR VPD */
#define SHARE_VPD      3    /* processing SHARE VPD */
#define DONE_VPD       4    /* done processing all VPDs */

void        add_to_sop           (Option_t *, Option_t *);
Option_t *  check_root_vpd       (prod_t *, int *);
void        get_level            (Option_t *);
int         grep_option_vpd      (char *, Level_t *, int, Option_t *, int *,
                                  Option_t *);
static int  deinstallable        (Option_t *, prod_t *, Option_t *);
static void get_next_vpd         (int *);
void 	    inu_update_fixdata   (Option_t *,Option_t *);
int 	    inu_update_op_fixdata(Option_t *);

/*--------------------------------------------------------------------------*
** NAME: inu_bld_sop_vpd
**
** FUNCTION: Build a selected option list from what we find in the VPD
**           databases.  This is for doing commits (c_FLAG), apply-root
**           (a_FLAG  &&  ROOT_PART), reject (r_FLAG), deinstall (u_FLAG), 
**           cleanup (C_FLAG) and status (s_FLAG).
**           It determines the part of the vpd to be searched by checking
**           USR_PART, ROOT_PART, and SHARE_PART flags.
**
** RETURNS: SUCCESS - no fatal errors
**          FAILURE - if a fatal error occurs
**
**--------------------------------------------------------------------------*/

int inu_bld_sop_vpd (Option_t * SopPtr, 
                     char     * name, 
                     Level_t  * level, 
                     Option_t * failsop)
{
   char     wildname [MAX_LPP_FULLNAME_LEN + 1];
   char     levname  [MAX_LPP_FULLNAME_LEN];
   char    *ptr;
   boolean  all_flag = FALSE;        /* set if "all" is part of the name */
   boolean  all_keyword = FALSE;     /* set if name is "all" or "ALL" */
   int      any_found = 0; /* set if any match found in any vpd tree searched */
   int      failcode = 0;
   int      vpd_path = INIT_VPD;
   int      len;

  /*-----------------------------------------------------------------------*
   * Search for name in the vpd.  If name doesn't end in ".all" or if name
   * is not "all" or "ALL" then we will search for occurences of name with 
   * ".*" appended (this is wildname).
   *-----------------------------------------------------------------------*/

   if (strcmp (name, "all") == 0  ||  strcmp (name, "ALL") == 0)
   {
      strcpy (wildname, "*");
      all_keyword = TRUE;
      all_flag = TRUE;
   }
   else if (((len = strlen (name)) >= 4)  && 
            ((strcmp (&(name[len - 4]), ".all")) == 0))
   {
      strcpy (wildname, name);
      wildname[len - 3] = '*';
      wildname[len - 2] = '\0';
      all_flag = TRUE;
   }
   else
   {
      strcpy (wildname, name);
      strcat (wildname, ".*");
   }

   /*-----------------------------------------------------------------------*
    * Below is a main while loop that does one tree at a time.  Which one is
    * done first is dependent on the type of operation being performed.
    *
    * APPLY_ROOT:  Loops only once for the ROOT part.
    * REJECT, DEINSTALL or CLEANUP:  Do ROOT first, then USR, and then SHARE.
    * COMMIT or STATUS:  Does the USR first, then ROOT, and then SHARE.
    *
    * At the bottom of the while loop, the vpd_path will be set to the next 
    * vpd_path (via a call to get_next_vpd) until it is set to  DONE_VPD.
    *-----------------------------------------------------------------------*/

   get_next_vpd (&vpd_path);

   while (vpd_path != DONE_VPD)
   {
     /*---------------------------------------------------------------*
      * Get all options that match the search criteria (with wildcard)
      *---------------------------------------------------------------*/
      any_found += grep_option_vpd (wildname, level, vpd_path, SopPtr, 
                                    &failcode, failsop);

     /*---------------------------------------------------------------*
      * If the user didn't specify "all" or "<name>.all" then search 
      * for the name w/o wildcard.
      *---------------------------------------------------------------*/
      if ( ! all_flag)
         any_found += grep_option_vpd (name, level, vpd_path, SopPtr, 
                                       &failcode, failsop);

     /*-------------------------------------------------------------------*
      * Get next state to search on
      *-------------------------------------------------------------------*/
      get_next_vpd (&vpd_path);

   }  /* while (vpd_path != DONE_VPD) */

  /*-----------------------------------------------------------------------*
   * See if any vpd records were found for 'name'
   *-----------------------------------------------------------------------*/

   if ( ! any_found  ||  failcode)
   {
     /*-------------------------------------------------------------------*
      * If the user specified "all" or "<name>.all" (all_flag)
      * then see if "all" was specified, (strcmp)
      * if it wasn't then strip off the ".all" from the name for when
      * we print it out.
      * (if "all" was specified, then leave all_flag set and print out
      *  entirely different messages that doesn't reference the name.)
      *-------------------------------------------------------------------*/
      if (all_flag)
      {
         if (strcmp (name, "all") != 0)
         {
            all_flag = 0;
            if ((ptr = strrchr (name, '.')) != NIL (char))
               *ptr = '\0';
         }
         if (all_keyword == TRUE)
         {
	    if(any_found)
   		return (SUCCESS);
            failcode = STAT_ALL_KW_FAILURE;
	 }
      } 

      if ( ! (C_FLAG || s_FLAG))
      {
         if ( ! failcode)
         {
            if (a_FLAG && ROOT_PART)
               failcode = STAT_NO_USR_MEANS_NO_ROOT;
            else if (r_FLAG)
               failcode = STAT_NUTTIN_TO_REJECT;
            else if (u_FLAG)
               failcode = STAT_NUTTIN_TO_DEINSTL;
            else if (c_FLAG)
               failcode = STAT_NUTTIN_TO_COMMIT;
         }
         inu_add_op_to_fail_sop (failsop, name, level, failcode, NIL(char), "");
      }

   } /* end if */
   
   return (SUCCESS);

} /* inu_bld_sop_vpd */


/*--------------------------------------------------------------------------*
**
** NAME: grep_option_vpd
**
** FUNCTION: This function will return all of the options that meet the
**           search criteria.  The search criteria is based on name (which is
**           the LPP from the user list), level (which is the level field from
**           the user list, not required), the part of the tree that is being
**           acted on (root, usr or share) and the operation that is being
**           performed.
**
** RETURNS:  The number options that are put on the selected options list (sop).
**
**--------------------------------------------------------------------------*/

int grep_option_vpd (char     * name, 
                     Level_t  * level, 
                     int        vpd_path, 
                     Option_t * sop, 
                     int      * failcode,
                     Option_t * failsop)
{
   Option_t       *op = NIL (Option_t);
   prod_t          prod;
   int             result;
   int             op_found = 0;

   memset (&prod, NULL, sizeof (prod_t));

  /*-----------------------------------------------------------------------*
   * copy name into the product structure.
   *-----------------------------------------------------------------------*/

   strcpy (prod.lpp_name, name);

  /*-----------------------------------------------------------------------*
   * If we're doing a commit, reject or status, then add the ST_APPLIED
   * to the search criteria for the vpd search.
   *-----------------------------------------------------------------------*/

   if ((c_FLAG  &&  ! (a_FLAG && ROOT_PART))  ||  r_FLAG  ||  s_FLAG)
      prod.state = ST_APPLIED;

  /*-----------------------------------------------------------------------*
   * If a level was given, then add the level to the
   * search criteria for the vpd search.
   *-----------------------------------------------------------------------*/

   if (level->ver != -1)
   {
      prod.ver = level->ver;
      prod.rel = level->rel;
      prod.mod = level->mod;
      prod.fix = level->fix;
      strcpy (prod.ptf, level->ptf);
   }


  /*-----------------------------------------------------------------------*
   * APPLY ROOT
   *-----------------------------------------------------------------------*/

   if (a_FLAG  &&  ROOT_PART)
   {
      if (level->ver == -1)
         result = vpdget (PRODUCT_TABLE, PROD_LPP_NAME, &prod);
      else
         result = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                          PROD_MOD | PROD_FIX | PROD_PTF), &prod);

     /*-------------------------------------------------------------------*
      * If there is a root part to this option AND the usr part is in the
      * APPLIED or COMMITTED state then check the state of the root part.
      *-------------------------------------------------------------------*/

      while (result == VPD_OK)
      {
         if ((prod.cp_flag &LPP_ROOT)  && 
             (prod.state == ST_COMMITTED  ||  prod.state == ST_APPLIED))
         {
            if ((op = check_root_vpd (&prod, failcode)) != NIL (Option_t))
            {
               op->vpd_tree = VPDTREE_ROOT;
               op->content = CONTENT_BOTH;
               if (c_FLAG)
                  op->operation = OP_APPLYCOMMIT;
               else
                  op->operation = OP_APPLY;

               get_level (op);

               if ((B_FLAG  &&  (IF_UPDATE (op->op_type)))  || 
                   (I_FLAG  &&  (IF_INSTALL (op->op_type))))
               {
                  op_found++;
                  add_to_sop (op, sop);
               } /* end if */
            } 
         } 

         vpd_free_vchars (PRODUCT_TABLE, &prod);
         result = vpdgetnxt (PRODUCT_TABLE, &prod);

      } /* end while */

      return (op_found);

   } /* end if (a_FLAG  &&  ROOT_PART) */


  /*-----------------------------------------------------------------------*
   * COMMIT, REJECT, STATUS
   *-----------------------------------------------------------------------*/

   else if (c_FLAG  ||  r_FLAG  ||  s_FLAG)
   {
      if (level->ver == -1)
         result = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE, &prod);
      else
         result = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                          PROD_MOD | PROD_FIX | PROD_PTF |
                                          PROD_STATE), &prod);
      while (result == VPD_OK)
      {
         op = conv_prod_t_Option_t (&prod);

        /*---------------------------------------------------------------*
         * Set some required fields in the option structure...
         *---------------------------------------------------------------*/

         if (c_FLAG)
            op->operation = OP_COMMIT;
         else if (r_FLAG)
            op->operation = OP_REJECT;
         else
            op->operation = OP_STATUS;

         get_level (op);

         switch (vpd_path)
         {
            case ROOT_VPD:
               op->vpd_tree = VPDTREE_ROOT;
               op->content = CONTENT_BOTH;
               break;
            case USR_VPD:
               op->vpd_tree = VPDTREE_USR;
               if (prod.cp_flag &(LPP_ROOT))
                  op->content = CONTENT_BOTH;
               else
                  op->content = CONTENT_USR;
               break;
            case SHARE_VPD:
               op->vpd_tree = VPDTREE_SHARE;
               op->content = CONTENT_SHARE;
               break;
         } /* end if switch */

         if ((B_FLAG  &&  (IF_UPDATE (op->op_type)))  || 
             (I_FLAG  &&  (IF_INSTALL (op->op_type))))
         {
            op_found++;
            add_to_sop (op, sop);
         }

         vpd_free_vchars (PRODUCT_TABLE, &prod);
         result = vpdgetnxt (PRODUCT_TABLE, &prod);

      } /* end while (result == VPD_OK) */

      return (op_found);
   } /* end else if (c_FLAG  ||  r_FLAG  ||  s_FLAG) */


  /*-----------------------------------------------------------------------*
   * CLEANUP
   *-----------------------------------------------------------------------*/

   else if (C_FLAG)
   {
      result = vpdget (PRODUCT_TABLE, PROD_LPP_NAME, &prod);

      while (result == VPD_OK)
      {
        /*---------------------------------------------------------------*
         * Check to see if it's in the APPLYING or COMMITTING states.
         *---------------------------------------------------------------*/

         if (prod.state == ST_COMMITTING  ||  prod.state == ST_APPLYING)
         {
            op = conv_prod_t_Option_t (&prod);

           /*-----------------------------------------------------------*
            * Set some required fields in the option structure...
            *-----------------------------------------------------------*/

            op->Status = STAT_CLEANUP;
            if (prod.state == ST_COMMITTING)
               op->operation = OP_CLEANUP_COMMIT;
            else
               op->operation = OP_CLEANUP_APPLY;

            get_level (op);

            switch (vpd_path)
            {
               case ROOT_VPD:
                  op->vpd_tree = VPDTREE_ROOT;
                  op->content = CONTENT_BOTH;
                  break;
               case USR_VPD:
                  op->vpd_tree = VPDTREE_USR;
                  op->content = CONTENT_USR;
                  break;
               case SHARE_VPD:
                  op->vpd_tree = VPDTREE_SHARE;
                  op->content = CONTENT_SHARE;
                  break;
            } /* switch */

            if ((B_FLAG  &&  (IF_UPDATE (op->op_type)))  || 
                (I_FLAG  &&  (IF_INSTALL (op->op_type))))
            {
               op_found++;
               add_to_sop (op, sop);
            }
         } /* if */
         vpd_free_vchars (PRODUCT_TABLE, &prod);
         result = vpdgetnxt (PRODUCT_TABLE, &prod);

      } /* end while (result == VPD_OK) */

      return (op_found);

   } /* end else if (C_FLAG) */


  /*-----------------------------------------------------------------------*
   * DEINSTALL
   *-----------------------------------------------------------------------*/

   else if (u_FLAG)
   {
     /*-----------------------------------------------
      * We'll be searchin fer base level records only.
      *-----------------------------------------------*/
      prod.ptf[0] = '\0';
      prod.update = INU_FALSE;

      if (level->ver == -1)
         result = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_UPDATE, &prod);
      else
         result = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER |
                          PROD_REL | PROD_MOD | PROD_FIX | PROD_PTF), &prod);


     /** ---------------------------------------------------------------- *
      **  If we were asked to deinstall an v.r.m.f update, goodbye.
      ** ---------------------------------------------------------------- */

      if (result == VPD_OK  &&            /* Query match                  */
          level->ver != -1  &&            /* A level was given on cmdline */
         (prod.cp_flag & LPP_UPDATE))     /* Match is an update           */
      {
         char name_level [MAX_LPP_FULLNAME_LEN+MAX_LEVEL_LEN+1];

         if (name[(strlen(name))-1] == '*')
            name[(strlen(name))-2] = '\0';

         sprintf (name_level, "%s %d.%d.%d.%d", name, level->ver, level->rel, 
                                                      level->mod, level->fix);

         instl_msg (FAIL_MSG, MSGSTR(MS_INSTALLP, INP_NO_DEINST_PTF_E,
                              INP_NO_DEINST_PTF_D), name_level);
         inu_quit(INUSYNTAX);
      }

      while (result == VPD_OK)
      {
         op = conv_prod_t_Option_t (&prod);

        /*---------------------------------------------------------------*
         * Set some required fields in the option structure...
         *---------------------------------------------------------------*/
         op->operation = OP_DEINSTALL;
         (void) get_level (op);

         switch (vpd_path)
         {
            case ROOT_VPD:
               op->vpd_tree = VPDTREE_ROOT;
               op->content = CONTENT_BOTH;
               break;
            case USR_VPD:
               op->vpd_tree = VPDTREE_USR;
               if (prod.cp_flag &(LPP_ROOT))
                  op->content = CONTENT_BOTH;
               else
                  op->content = CONTENT_USR;
               break;
            case SHARE_VPD:
               op->vpd_tree = VPDTREE_SHARE;
               op->content = CONTENT_SHARE;
               break;
         } /* switch */

         if (IF_INSTALL (op->op_type))
         {
            op_found++;
            if ((*failcode = deinstallable (op, &prod, failsop)) == 0)
               (void) add_to_sop (op, sop);
         }

         vpd_free_vchars (PRODUCT_TABLE, &prod);
         result = vpdgetnxt (PRODUCT_TABLE, &prod);

      } /* while (result == VPD_OK) */

      return (op_found);

   } /* end else if (u_FLAG) */

  /*-----------------------------------------------------------------------*
   * ERROR:  We should have never gotten here ! 
   *-----------------------------------------------------------------------*/
   else
      return (FAILURE);

} /* grep_option_vpd */


/*--------------------------------------------------------------------------*
**
** NAME: conv_prod_t_Option_t
**
** FUNCTION:  This function creates an Option_t out of a prod_t
**
** RETURNS: The Option_t that is created.
**
**--------------------------------------------------------------------------*/

Option_t * conv_prod_t_Option_t (prod_t * buf)
{
   Option_t       *op;

   op = create_option (buf->lpp_name, 
                       NO,              /* default op_checked to NO */
                       NO,              /* default quiesce to NO */
                       CONTENT_UNKNOWN, /* default content to unknown */
                       "",              /* default lang to NIL (char) */
                       NIL (Level_t),   /* default level to NIL (Level_t) */
                       buf->description, 
                       NIL (BffRef_t)); /* default bff to NIL (BffRef_t) */

   op->prereq = strdup (buf->prereq);
   strcpy (op->prodname, buf->name);
   op->level.ver = buf->ver;
   op->level.rel = buf->rel;
   op->level.mod = buf->mod;
   op->level.fix = buf->fix;
   strcpy (op->level.ptf, buf->ptf);
   op->content = CONTENT_UNKNOWN;
   op->supersedes = strdup (buf->supersedes);

   op->op_type = ((buf->cp_flag &LPP_31_FMT ? OP_TYPE_3_1 : 0)     |
                  (buf->cp_flag &LPP_32_FMT ? OP_TYPE_3_2 : 0)     |
                  (buf->cp_flag &LPP_41_FMT ? OP_TYPE_4_1 : 0)     |
                  (buf->cp_flag &LPP_INSTAL ? OP_TYPE_INSTALL : 0) |
                  (buf->cp_flag &LPP_UPDATE ? OP_TYPE_UPDATE : 0)  |
                  (MIGRATING (buf->cp_flag) ? OP_TYPE_MIGRATING : 0) |
                  (IF_LPP_PKG_PTF_TYPE_C (buf->cp_flag) ? OP_TYPE_C_UPDT: 0) |
                  (IF_LPP_PKG_PTF_TYPE_E (buf->cp_flag) ? OP_TYPE_E_UPDT: 0) |
                  (IF_LPP_PKG_PTF_TYPE_M (buf->cp_flag) ? OP_TYPE_M_UPDT: 0) |
                  (IF_LPP_PKG_PTF_TYPE_ML (buf->cp_flag) ? OP_TYPE_ML_UPDT: 0));

   return (op);

} /* conv_prod_t_Option_t */


/*--------------------------------------------------------------------------*
**
** Function:    check_root_vpd
**
** Purpose:     Determines whether the corresponding root part of the usr_prod
**              pkg can be installed or not.  It does this by ensuring that 
**              one of the following conditions is true
**              (i)   No corresponding root product vpd record exists for the 
**                    root part of the pkg specified by the usr_prod 
**                    parameter.
**              (ii)  If a corresponding root product vpd record does exist, 
**                    then it must not be in an installed state (i.e.
**                    applied or committed). 
**              If either condition is true, then an Option_t structure
**              is created and initialized for the root part, and returned.
**           
** Parameters:  usr_prod is an entry from the /usr product swvpd database.
**
** RETURNS:     On Success - A ptr to newly malloc'd Option_t structure 
**              On Failure - NULL
**
**--------------------------------------------------------------------------*/

Option_t * check_root_vpd (prod_t * usr_prod, int *failcode)
{
   Option_t      * option;
   prod_t          prod;       /* a local prod struct for querying purposes */
   int             result;
   char            buf[TMP_BUF_SZ];

   memset (&prod, NULL, sizeof (prod_t));
  /** ----------------------------------------------------------------- *
   **  Copy the fields to be searched on from the usr_prod to the local
   **  prod.  Then use the local prod for the vpdget query. 
   ** ----------------------------------------------------------------- */

   strcpy (prod.lpp_name, usr_prod->lpp_name);
   prod.state = usr_prod->state;
   prod.ver   = usr_prod->ver;
   prod.rel   = usr_prod->rel;
   prod.mod   = usr_prod->mod;
   prod.fix   = usr_prod->fix;
   strcpy (prod.ptf, usr_prod->ptf);

   vpdremote ();
   result = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                    PROD_MOD | PROD_FIX | PROD_PTF), &prod);
   vpdlocal ();
   if (result != VPD_NOMATCH  &&  result != VPD_OK)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                               INU_INSTALLP_CMD);
      inu_quit (INUVPDER);
   }

  /** ----------------------------------------------------------------- *
   **  If we found a corresponding root vpd record (based on the usr's
   **  vpd record search criteria)  AND this root record is not already
   **  installed (applied or committed),  
   **  Then create an option for this root part that we can add to the sop. 
   ** ----------------------------------------------------------------- */

   if (result == VPD_OK 
               && 
      prod.state != ST_APPLIED 
               &&  
      prod.state != ST_COMMITTED)
   {
      option = conv_prod_t_Option_t (&prod);
      vpd_free_vchars (PRODUCT_TABLE, &prod);
      return (option);
   }

  /** ----------------------------------------------------------------- *
   **  If we didn't find any root vpd record for this option, then
   **  create an option based on the usr vpd record.  This option will
   **  be added to the sop by the caller of this function.
   ** ----------------------------------------------------------------- */

   else if (result == VPD_NOMATCH)
   {
      option = conv_prod_t_Option_t (usr_prod);
      vpd_free_vchars (PRODUCT_TABLE, &prod);
      return (option);
   }

   else
   {
      /*-------------------------------------------------------------------*
       * If the force flag was specified and this is an install image then
       * if it is in the COMMITTED state then allow the installation.
       *-------------------------------------------------------------------*/

      if ((F_FLAG  &&  (prod.cp_flag & LPP_INSTAL))
                               && 
           (prod.state == ST_COMMITTED))
      {
         option = conv_prod_t_Option_t (&prod);
         vpd_free_vchars (PRODUCT_TABLE, &prod);
         return (option);
      }

      *failcode = STAT_ALREADY_INSTALLED;
   } 

  /** -------------------------------------------------------------------- *
   **  If we get here then the vpdget query was successful.  So, the call
   **  to vpd_free_vchars will succeed.
   ** -------------------------------------------------------------------- */

   vpd_free_vchars (PRODUCT_TABLE, &prod);
   return (NIL (Option_t));

} /* check_root_vpd */


/*--------------------------------------------------------------------------*
**
** NAME: add_to_sop
**
** FUNCTION: This function adds the option structure to the Selected Options
**           list (sop). If it already exists in the list, then return...
**
** RETURNS: void
**
**--------------------------------------------------------------------------*/

void add_to_sop (Option_t * new_op, 
                 Option_t * sop)
{
   Option_t       *op;
   Option_t       *last_op;

   for (last_op = sop, op = sop->next;
        op != NIL (Option_t);
        op = op->next)
   {
      if ((strcmp (new_op->name, op->name) == 0)  && 
          (new_op->vpd_tree == op->vpd_tree)  && 
          (new_op->operation == op->operation)  && 
          (new_op->level.ver == op->level.ver)  && 
          (new_op->level.rel == op->level.rel)  && 
          (new_op->level.mod == op->level.mod)  && 
          (new_op->level.fix == op->level.fix)  && 
          (strcmp (new_op->level.ptf, op->level.ptf) == 0))
         return;

      last_op = op;
   } /* end for */

   last_op->next = new_op;
   new_op->next = NIL (Option_t);

   return;

} /* add_to_sop */


/*--------------------------------------------------------------------------*
**
** NAME: get_level
**
** FUNCTION: This function reads the lpp database to get the current level
**           of the lpp-option.
**
** RETURNS: void
**
**--------------------------------------------------------------------------*/

void get_level (Option_t * op)
{
   lpp_t           lpp;
   int             rc;

   memset (&lpp, NULL, sizeof (lpp_t));

   if (a_FLAG  &&  ROOT_PART)
      vpdremote ();

   strcpy (lpp.name, op->name);
   rc = vpdget (LPP_TABLE, LPP_NAME, &lpp);
   vpd_free_vchars (LPP_TABLE, &lpp);

   if (a_FLAG  &&  ROOT_PART)
      vpdlocal ();

   if (rc == VPD_OK)
   {
      op->level.sys_ver = lpp.ver;
      op->level.sys_rel = lpp.rel;
      op->level.sys_mod = lpp.mod;
      op->level.sys_fix = lpp.fix;
   } /* end if */
   return;

} /* get_level */


/*--------------------------------------------------------------------------*
** NAME: deinstallable
**
** FUNCTION: Check conditions to verify that the option is deinstallable.
**           Certain options will be deinstallable, but will produce warning
**           messages.  For these options, entries will be added to both
**           the sop and the fail_sop.
**
**           - Partially migrated options are deinstallable, but a warning
**             message should be displayed indicating no unconfiguration will
**             occur.  These options will be added to both the sop and the 
**             failsop (the failsop entry is necessary for the warning message
**             to occur).
**           - 3.1 options and INSTALLED_ON_32 options are only deinstallable.
**             A warning shall be displayed indicating deinstallation is a
**             best attempt.  These options will be added to both the sop and 
**             the failsop (the failsop entry is necessary for the warning mes-
**             sage to occur.
**           - bos.rte is not deinstallable.
**           - Run inu_deinst_chk to see if there's a option.pre_d.  A non-
**             zero return code from inu_deinst_chk indicates that the 
**             the option.pre_d script was found, run, and it returned a 
**             non-zero return code indicating that deinstallation should
**             not proceed.
**
** RETURNS: if the option is deinstallable
**             return 0
**          else
**             return a non-zero pre-installation failure summary (PIF) code
**             to be used for adding the option to the failsop.
**
**--------------------------------------------------------------------------*/

static int deinstallable (Option_t * op, 
                          prod_t * prod,
                          Option_t * failsop)
{
   int    failcode = 0;

   if (MIGRATING (prod->cp_flag))
      inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                              STAT_WARN_DEINST_MIG, NIL(char), "");

   else if (INSTALLED_ON_32 (prod->cp_flag))
      inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                              STAT_WARN_DEINST_3_2, NIL(char), "");

   else if (IF_3_1 (op->op_type))
      inu_add_op_to_fail_sop (failsop, op->name, &(op->level), 
                              STAT_WARN_DEINST_3_1, NIL(char), "");

   else if (strcmp (op->name, "bos.rte") == 0)
      failcode = STAT_NO_DEINST_BOS;

   else if (ROOT_PART && USR_PART && op->vpd_tree == VPDTREE_ROOT)
      failcode = inu_deinst_chk (op->prodname, op->name, op->vpd_tree, FALSE);

   else
      failcode = inu_deinst_chk (op->prodname, op->name, op->vpd_tree, TRUE);

   return (failcode);

} /* deinstallable */


/*--------------------------------------------------------------------------*
**
** NAME: is_it_applied_or_committed 
**
** FUNCTION: Query the product database to see  if the option
**           is APPLIED/COMMITTED.  Use for base levels only.
**
** ARGUMENT: option -- option name
**
** RETURNS: (Level_t) 
**          Set to a level structure on success
**          NIL (Level_t) on failure
**
**          On vpd errors, malloc errors calls inu_quit ()
**
** NOTE: Calling functions will have to free the malloced
**       Level_t struct.
**--------------------------------------------------------------------------*/

Level_t *
is_it_applied_or_committed (char *option)
{
   lpp_t           lpp;       /* a local lpp struct for querying purposes */
   int             result;
   Level_t         *level = NIL (Level_t);
   
   init_vchars (LPP_TABLE, &lpp);

   /*-----------------------------------------------------------------------*
   * copy name into the product structure.
   *-----------------------------------------------------------------------*/

   strcpy (lpp.name, option);

   result = vpdget (LPP_TABLE, LPP_NAME, &lpp);

   if (result != VPD_NOMATCH  &&  result != VPD_OK) {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                               INU_INSTALLP_CMD);
      inu_quit (INUVPDER);
   }

   if (result == VPD_OK 
               &&  
       (lpp.state == ST_APPLIED  ||  lpp.state == ST_COMMITTED))
   {

      if ((level = (Level_t *) malloc (sizeof (Level_t))) == NIL (Level_t)){
           instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                    CMN_MALLOC_FAILED_D), inu_cmdname);
           inu_quit (INUMALLOC);
      }

      level->ver = lpp.ver;
      level->rel = lpp.rel;
      level->mod = lpp.mod;
      level->fix = lpp.fix;
      level->ptf[0] = '\0';
   }

   if (result == VPD_OK)    
      vpd_free_vchars (LPP_TABLE, &lpp);

   return (level);

} /* is_it_applied_or_committed */


/*--------------------------------------------------------------------------*
** NAME:     get_next_vpd
**
** FUNCTION: Determine next vpd path for processing based on current value
**           of vpd_path.  Then set vpd path variables so that proper vpd
**           is searched.
**
** RETURNS:  None.
**
**--------------------------------------------------------------------------*/
static void 
get_next_vpd (int * vpd_path)
{
   if (a_FLAG && ROOT_PART)
   {
      if (*vpd_path == INIT_VPD)
         *vpd_path = ROOT_VPD;
      else
         *vpd_path = DONE_VPD;
   }

   else if (r_FLAG || u_FLAG || C_FLAG)
   {
      switch (*vpd_path)
      {
         case INIT_VPD:
            if (ROOT_PART)
               *vpd_path = ROOT_VPD;
            else if (USR_PART)
               *vpd_path = USR_VPD;
            else if (SHARE_PART)
               *vpd_path = SHARE_VPD;
            else
               *vpd_path = DONE_VPD;
            break;

         case ROOT_VPD:
            if (USR_PART)
               *vpd_path = USR_VPD;
            else if (SHARE_PART)
               *vpd_path = SHARE_VPD;
            else
               *vpd_path = DONE_VPD;
            break;

         case USR_VPD:
            if (SHARE_PART)
               *vpd_path = SHARE_VPD;
            else
               *vpd_path = DONE_VPD;
            break;

         default:
            *vpd_path = DONE_VPD;
            break;

      } /* switch */
   }

   else if (c_FLAG  ||  s_FLAG)
   {
      switch (*vpd_path)
      {
         case INIT_VPD:
            if (USR_PART)
               *vpd_path = USR_VPD;
            else if (ROOT_PART)
               *vpd_path = ROOT_VPD;
            else if (SHARE_PART)
               *vpd_path = SHARE_VPD;
            else
               *vpd_path = DONE_VPD;
            break;

         case USR_VPD:
            if (ROOT_PART)
               *vpd_path = ROOT_VPD;
            else if (SHARE_PART)
               *vpd_path = SHARE_VPD;
            else
               *vpd_path = DONE_VPD;
            break;

         case ROOT_VPD:
            if (SHARE_PART)
               *vpd_path = SHARE_VPD;
            else
               *vpd_path = DONE_VPD;
            break;
         default:
            *vpd_path = DONE_VPD;
            break;

      } /* switch */
   }

   else
      *vpd_path = DONE_VPD;

  /*--------------------------------------------------------------------
   * Set vpd path based on *vpd_path established above
   *--------------------------------------------------------------------*/

   if (*vpd_path == USR_VPD)
      vpdlocal ();
   else if (*vpd_path == ROOT_VPD)
   {
      vpdremotepath (VPD_ROOT_PATH);
      if (a_FLAG && ROOT_PART)
         vpdlocal ();
      else
         vpdremote ();
   }
   else if (*vpd_path == SHARE_VPD)
   {
      vpdremotepath (VPD_SHARE_PATH);
      vpdremote ();
   }

} /* get_next_vpd */

/*--------------------------------------------------------------------------*
**
** NAME: inu_update_fixdata
**
** FUNCTION: This function reads the fix data from the install medium
** 	     and adds a fix record to the fix database 
**	     if it does not already exist.
**
** RETURNS: nothing
**
**--------------------------------------------------------------------------*/


void inu_update_fixdata (Option_t * cur_op, Option_t * next_op)
{

    int lock;			/* odm lock number */
    int rc;			/* return code from inu_update_op_fixdata */

    Option_t * op;		/* Option pointer */

   /*-----------------------------------------------------------------------*
    * fork another process to do update in the background.
    * The parent just returns and continues.
    *-----------------------------------------------------------------------*/

    if ((fork()) != 0)
	return;

	
    /*-----------------------------------------------------------------------*
     * All of this is done as the child process 				    * 
     *-----------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*
      create another process group so that installp's signals don't get
      caught by this process.  This process should always complete 
      *-----------------------------------------------------------------------*/

    setpgid(0,0);
    nice((int)1);
    /* vpdterm(); */

    /* lock the odm */
    if ((lock = odm_lock("/usr/lib/objrepos/fix_lock",0)) == -1)
	exit(-1);


    for (op = cur_op; op != next_op; op = op->next)
	if ((rc=inu_update_op_fixdata(op)) == VPD_SYS)
	{
	    odm_unlock(lock);
	    /* vpdterm(); */
	    exit(-1);
	}
    
    odm_unlock(lock);
    /* vpdterm(); */
    exit(0);

}

int inu_update_op_fixdata (op)
    Option_t	*op;		/* pointer to option */
{

    FILE *file_ptr;		/* stream ptr to fixdata file */
    int stanza_length;		/* keeps track of length of fixdata stanza */
    char *stanza;		/* ptr to fixdata stanza */
    char terminating_char;	/* used by get_ascii_string */
    char *class_name;		/* odm class name */
    char *descrip_name;		/* name of odm descriptor */
    char savename[64];		/* save space */
    char *descrip_value;	/* value of odm descriptor */
    int skip_spaces;		/* arg to get_value.. */
    int descriptor_index;	/* index into vpd table for descriptor */
    char *new_entry;		/* ptr to new entry in odm data base */
    char *descriptor_offset;	/* offset into vpd table for descriptor */
    CLASS_SYMBOL Class;
    int rc;			/* generic return code */
    char **vchar_location;      /* ptr to vchar location in the structure */
    fix_t buf;			/* fix db entry */
    int addentry;		/* flag indicating whether to add the entry */
    char fix_file[PATH_MAX + 1];/* path to fixinfo file   */

    /*-----------------------------------------------------------------------*
     * Create the name of the fixdata file 				    *
     *-----------------------------------------------------------------------*/

    strcpy (fix_file, INU_LIBDIR);
    strcat (fix_file, "/");
    strcat (fix_file, op->name);
    strcat (fix_file, ".fixdata");

    /*-----------------------------------------------------------------------*
     * Open the fix database file for reading,
     *-----------------------------------------------------------------------*/

    if ((file_ptr = fopen (fix_file, "r")) == NIL (FILE))
	return(VPD_SYS);

    /* init the vpd table */

    vpdlocalpath(VPD_LOCAL_PATH);
    vpdlocal();

    if ((rc = vpdinit(FIX_TABLE)) != VPD_OK)
	return(VPD_SYS);

    /* mount the fix class */

    if ((Class = odm_mount_class ("fix")) == NULL ||  
	Class == (CLASS_SYMBOL) -1 )

	return(VPD_SYS);

    /* for every apar in the fix db file */

    while ((stanza_length = get_ascii_phrase(file_ptr,STANZA,&stanza)) > 0)
    {

	skip_spaces = FALSE;

        class_name = (char *) get_value_from_string(stanza,":\n",skip_spaces,
					   &terminating_char);

	if (strcmp(class_name,"fix") != 0)
	    continue;
	
        new_entry = (char *) malloc(Class->structsize);

        if (new_entry == (char *) NULL)
	    return(VPD_SYS);

        bzero(new_entry,Class->structsize);


	/*------------------------------------------------------------*/
	/* Since the variable length char (vchars) are stored in a    */
	/* separate buffer rather than in the structure itself, we will */
	/* go through the class info and malloc space for the vchars */
	/*------------------------------------------------------------*/
	
	for (descriptor_index = 0; descriptor_index < Class->nelem;
	     descriptor_index++)
	{
	    if  ((Class->elem)[descriptor_index].type == ODM_VCHAR)
	    {
		vchar_location =
		    (char **)
			(new_entry + (Class->elem)[descriptor_index].offset);
		
		*vchar_location = (char *) NULL;
	    } 

        } 

	addentry=FALSE;
	skip_spaces=TRUE;
        while (TRUE)
        {


	    /*-------------------------------------------*/
	    /* Find the name of the next descriptor      */
	    /*-------------------------------------------*/
	    
	    descrip_name =
		(char *) get_value_from_string((char *) NULL,"=\n",
				      skip_spaces,
				      &terminating_char);

	    if (descrip_name == (char *) NULL ||
		(*descrip_name == '\0' && terminating_char == '\0'))

		/*---------------------------*/
		/* Found the end-of-stanza.  */
		/*---------------------------*/
		
		break;

	    if (terminating_char != '=')
	    {
		addentry=FALSE;
		break;
	    }
	    
	    rc = (int) strcpy(&savename[0],descrip_name);

	    /*-------------------------------------------*/
	    /* Determine the offset for this descriptor. */
	    /*-------------------------------------------*/

	    for (descriptor_index = 0; descriptor_index < Class->nelem;
		 descriptor_index++)

		if (strcmp((Class->elem)[descriptor_index].elemname,
			   descrip_name) == 0)
		    break;

	    
	    if (descriptor_index >= Class->nelem)
	    {
               addentry = FALSE;
               break;
	    }

	    /*-----------------------------------*/
	    /* Get the value for this descriptor */
	    /*-----------------------------------*/
	    
	    descrip_value = (char *) get_value_from_string((char *) NULL,"\n",
                                     skip_spaces, &terminating_char);
	    
	    if (descrip_value == (char *) NULL)
		break;
	    
	    if (strcmp(&savename[0],"name")==0)
	    {
		strncpy((char *) buf.name,descrip_value,sizeof(buf.name));
		
		if ((rc=vpdget(FIX_TABLE,FIX_NAME,&buf))== VPD_NOMATCH)
		    addentry=TRUE;
		else
		{
		    /* free vchar memory allocated to get fix table entry */
		    vpd_free_vchars (FIX_TABLE, &buf);
		    
		    /* go on to next iteration */
		    break;
		}
	    }
	    
	    /*-------------------------------------------------------*/
	    /* We now have the descriptor name and its value.  Store */
	    /* it in the structure.                                  */
	    /*-------------------------------------------------------*/


	    descriptor_offset = new_entry +
		Class->elem[descriptor_index].offset;
	    
	    
	    switch ((Class->elem)[descriptor_index].type)
	    {
	    case ODM_CHAR:
		*descriptor_offset = '\0';
		strncat(descriptor_offset,descrip_value,
			(Class->elem)[descriptor_index].size - 1);
		break;
	    case ODM_VCHAR:
		/*------------------------------------------------*/
		/* Since the vchars are not put directly into the */
		/* structure, we need to save the string in the   */
		/* buffer pointed to by descriptor_offset.        */
		/*------------------------------------------------*/
		
		vchar_location =  (char **) descriptor_offset;
		
		if (*vchar_location != NULL)
		{
		    /*-----------------------------------------------*/
		    /* When *vchar_location != NULL occurs, this     */
		    /* means that the user has two lines in the      */
		    /* stanza for the same value.  Free the previous */
		    /* buffer and malloc a new buffer so we only     */
		    /* keep the last value.                          */
		    /*-----------------------------------------------*/

		    free(*vchar_location);
		    *vchar_location = NULL;
		} /* endif */
		
		*vchar_location =
		    (char *) malloc(strlen(descrip_value) + 1);
		
		if (*vchar_location == NULL)
		{
		    addentry=FALSE;
		    break;
		} 
		
		strcpy(*vchar_location,descrip_value);
		
		break;
	    default:
		/* go to next stanza */
		break;
	    }
	    
	    
	} /* endwhile */
	
	
	if (addentry == TRUE)
	    if((rc = vpdadd(FIX_TABLE,new_entry)) != SUCCESS)
		return(VPD_SYS);
	
	free(new_entry);

	/*------------------------------------------------------------*/
	/* Since the variable length char (vchars) are stored in a    */
	/* separate buffer rather than in the structure itself,       */
	/* free the space used for the vchars 			  */
	/*------------------------------------------------------------*/
	
	for (descriptor_index = 0; descriptor_index < Class->nelem;
	     descriptor_index++)
	{
	    
	    if  ((Class->elem)[descriptor_index].type == ODM_VCHAR)
	    {
		vchar_location = (char **)
		    (new_entry + (Class->elem)[descriptor_index].offset);
		
		if (*vchar_location != NULL)
		{
		    free(*vchar_location);
		    *vchar_location = NULL;
		}
		
	    } /* endif */
	} 
	
    } /* endwhile */
    fclose(file_ptr);
    
    return(SUCCESS);
    
}
