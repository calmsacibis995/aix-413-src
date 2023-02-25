static char sccsid[] = "@(#)97  1.21.1.20  src/bos/usr/sbin/install/installp/inu_level.c, cmdinstl, bos41B, 412_41B_sync 1/16/95 16:14:01";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_level_ok, inu_level_chk_3_1, inu_report_bad_31_level
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

#include <sys/file.h>
#include <installp.h>
#include <instl_options.h>
#include <inuconvert.h>

#define ROOT_INV_FILE           1       /* constant identifying a root
                                         * filesystem inventory file.  */

static int    inu_check_product        (Option_t *);
static int    inu_check_prod_table   (Option_t *);
static int    inu_get_prod             (Option_t *, prod_t *);


/*--------------------------------------------------------------------------*
**
** NAME: inu_level_ok
**
** FUNCTION: Check current level of LPP.
**
** For all installs, inu_level_ok makes sure the level of the option requested
**   to be installed is > what is currently on the system, unless they specify
**   the -F  flag. If the option isn't installed at all, then that's ok also...
**
** For 3.2 updates inu_level_ok checks for the existance of the update on the
**   system in the applied or committed state and will return FALSE if the
**   update is already installed.
**
** For 3.1 updates, inu_level_ok will do a level check similar to the one
**   done in the 3.1 updatep... Also, it will make sure the current level of
**   the 3.1 option is in the committed state.
**
** RETURN VALUE DESCRIPTION:
**          TRUE:   the lpp-option check out ok
**          FALSE:  the lpp-option failed one of the checks
**
**--------------------------------------------------------------------------*/


int inu_level_ok (Option_t *op)  /* Option structure for what
                                  * is on the media */
{
   lpp_t     lpp;            /* pointer to lpp info structure */
   int       rc;             /* return code */
   prod_t    prod_usr;       /* product info structure */
   prod_t    prod_root;      /* product info structure */
   uchar     has_root_part;  /* indicates whether lpp-option has a root part  */
   char      lppname[256];   /* buffer for storing basename of an lpp-option */
   char      inv_file[512];         /* buffer for the name of the inventory
                                     * file returned by find_inv_file.     */
   Level_t   syslevel;              /* struct to contain the system level */


   /*-----------------------------------------------------------------------*
    * Read the lpp vpd database based on the lpp name.
    *-----------------------------------------------------------------------*/

   memset (&prod_usr, NULL, sizeof (prod_t));
   memset (&prod_root, NULL, sizeof (prod_t));
   memset (&lpp, NULL, sizeof (lpp_t));

   strcpy (lpp.name, op->name);
   rc = vpdget (LPP_TABLE, LPP_NAME, &lpp);
   inu_ck_vpd_sys_rc (rc);

   vpd_free_vchars (LPP_TABLE, &lpp); /* Don't need these fields. */

   /*-----------------------------------------------------------------------*
    * If an entry isn't found, or one is found but it isn't in a usable state
    * then return TRUE to mean it's OK to install the new one...
    *-----------------------------------------------------------------------*/

   if (rc == VPD_NOMATCH 
             ||  
      (rc == VPD_OK  &&  (lpp.state != ST_APPLIED  &&  lpp.state != ST_COMMITTED)))
   {
      op->level.sys_ver = 0;
      op->level.sys_rel = 0;
      op->level.sys_mod = 0;
      op->level.sys_fix = 0;
      return (TRUE);
   }

   /*-----------------------------------------------------------------------*
    * Set the system's current level. These values are passed to the instal/
    * update scripts.
    *-----------------------------------------------------------------------*/

   op->level.sys_ver = lpp.ver;
   op->level.sys_rel = lpp.rel;
   op->level.sys_mod = lpp.mod;
   op->level.sys_fix = lpp.fix;

   /* Need to determine if this lpp-option has a root part. It has a root
    * part if:
    *
    *     1) Its Option_t structure indicates that it has both a user and
    *        root part.
    *
    *     2) If inuconvert creates a root part for the lpp-option. Inuconvert
    *        will create a root part for an lpp-option if that option has a
    *        root inventory file in /usr/lpp/<lppname>/inst_convert.  The
    *        function "find_inv_file" is defined in inuconvert.c
    */

   convert_vrmf_to_string (op); /* convert an lpp's version, release, 
                                 * modification and fix to strings.
                                 * This function is in inuconvert.c
                                 * and must be called prior to calling
                                 * find_inv_file.
                                 */

   inu_get_prodname (op, lppname);  

   has_root_part = (op->content == CONTENT_BOTH)  || 
                   (find_inv_file (inv_file, lppname, op->name, 
                                   ROOT_INV_FILE) == SUCCESS);

   if ((op->vpd_tree == VPDTREE_USR)  /* Only need to look at usr parts */
                && 
         (has_root_part)                /* that have root parts. */
                && 
       ((IF_INSTALL (op->op_type))  ||  (IF_UPDATE (op->op_type)))
                                        /* Either install or update */
                && 
           ( ! F_FLAG)                    /* Force flag NOT used. */
                && 
          (ROOT_PART))                  /* User requested root part. */
   {
      /* Read the user product database */

      rc = inu_get_prod (op, &prod_usr);
      vpd_free_vchars (PRODUCT_TABLE, &prod_usr);  /* We are not interested
                                                      in these fields. */

      /* If we find a user record read the root record.                   */

      /* NOTE: We are trying to see if installp just needs to sync up a   */
      /* usr part with its root part or vice versa (if one of the parts   */
      /* are missing) without having to reinstall the entire fileset.     */
      /* We don't want to perform this processing if the special case     */
      /* is detected where a 4.1 update is being requested while the      */
      /* 4.1 base level is already installed with the same vrmf OR if a   */
      /* 4.1 update is already installed and the 4.1 base level is        */
      /* being requested.  Subsequent code in this routine will detect    */
      /* This situation and result in "already installed" outcome.        */
 
      if ((rc == VPD_OK) 
                &&
          (IF_3_X (op->op_type) 
           ||
           ((IF_UPDATE (op->op_type) && prod_usr.update == INU_TRUE) ||
            (IF_INSTALL (op->op_type) && prod_usr.update == INU_FALSE))))
      {
         /* if this is an install or an update that has not been superceeded */

         inu_set_vpdtree (VPDTREE_ROOT);        /* set up to read root dbase */
         rc = inu_get_prod (op, &prod_root);    /* read the product database */
         vpd_free_vchars (PRODUCT_TABLE, &prod_root);  /* We are not intersted
                                                        * in these fields. */
         if (rc != VPD_OK)
            prod_root.state = ST_AVAILABLE;

         if (prod_usr.state != prod_root.state)
         {
            /* The user and root parts are not in the same state. */
            /* Determine if we need to reinstall the user part.   */

            if ((prod_usr.state == ST_APPLIED)
                               || 
                (prod_usr.state == ST_COMMITTED))
            {
               /* Don't have to reinstall the user part */

               op->vpd_tree = VPDTREE_ROOT;
               return (TRUE);
            }

            else
            {
               if (IF_UPDATE (op->op_type)
                               && 
                   prod_usr.state == ST_BROKEN
                               && 
                   ( ! c_FLAG  ||  ! g_FLAG  ||  ! N_FLAG))
               {
                  op->Status = STAT_BROKEN;
                  return (FALSE);
               }

               /* the user part needs to be installed */

               inu_set_vpdtree (VPDTREE_USR);   /* reestablish user tree */
               return (TRUE);
            }
         } 
         inu_set_vpdtree (VPDTREE_USR); /* reestablish user tree */
      } 
   } 

   /*-----------------------------------------------------------------------*
    * If the force flag is set, and this is an install, then return TRUE;
    *-----------------------------------------------------------------------*/

   if (F_FLAG  &&  IF_INSTALL (op->op_type))
      return (TRUE);

   /*-----------------------------------------------------------------------*
    * For 3.1 updates:
    *-----------------------------------------------------------------------*/

   if (IF_3_1_UPDATE (op->op_type))
   {
      /*-------------------------------------------------------------------*
       * We need to call inu_level_chk_3_1 to make sure the system level is
       * correct in order to put this update on. (Follows 3.1 rules)
       *-------------------------------------------------------------------*/

      if ((rc = inu_level_chk_3_1 (&(op->level))) != 1)
      {
         /* the level of the update isn't compatible  */

         op->Status = STAT_REQUISITE_FAILURE;
         return (FALSE);
      }

      /*-------------------------------------------------------------------*
       * We must also make sure the lpp-option is in the committed state.
       *-------------------------------------------------------------------*/

      if (lpp.state != ST_COMMITTED)
      {
         op->Status = STAT_BASE_MUST_BE_COMMITTED;
         return (FALSE);
      }
      return (TRUE);
   }
   else
   {

      /*-------------------------------------------------------------------*
       * Check the product table to see if an update or 4.1 base level is 
       * already installed. Do this for 4.1 base levels because updates and 
       * base levels can co-exist with the same vrmf in 4.1.  Therefore, we
       * must check for already installed in the product table.
       *-------------------------------------------------------------------*/

      if (IF_UPDATE (op->op_type) || IF_4_1 (op->op_type))
         return (inu_check_prod_table (op));
   }

   /*-----------------------------------------------------------------------*
    * All that are left are installs that are being re-installed and the -F
    * flag wasn't set.  We need to make sure the lpp-option attempting to be
    * installed is at a level greater then the one currently on the system.
    * This code is only exercised for 3.2 base levels.
    *-----------------------------------------------------------------------*/

   syslevel.ver    =  lpp.ver;
   syslevel.rel    =  lpp.rel;
   syslevel.mod    =  lpp.mod;
   syslevel.fix    =  lpp.fix;
   syslevel.ptf[0] =  '\0';

   if ((inu_level_compare (&(op->level), &syslevel)) <= 0)
   {
      op->Status = STAT_BASE_ALREADY_INSTALLED;
      return (FALSE);
   }

   return (TRUE);  
} 

/*--------------------------------------------------------------------------*
**
** NAME: inu_level_chk_3_1  NOTE: This function is for 3.1 updates ONLY ! 
**
** FUNCTION: compares a struct of 4 integers containing the version, release, 
**       modification and fix of an update to be applied to the same fields
**       that currently reside on the system
**
** RETURN VALUE DESCRIPTION: -1 - Level is less than or equal to current level.
**               0 - equal to current level.
**               1 - Level is greater than current level and the
**               version and release are equal.
**               2 - version or release is greater than current
**               version or release.
**
**--------------------------------------------------------------------------*/

int inu_level_chk_3_1 (Level_t * level) /* Ptr to struct with the level to
                                         * compare */
{
   /* Begin inu_level_chk_3_1 */

   if (level->ver < level->sys_ver)
      return (-1);      /* version less than current version */
   if (level->ver > level->sys_ver)
      return (2);       /* version greater than current */

   if (level->rel < level->sys_rel)
      return (-1);      /* release less than current release */
   if (level->rel > level->sys_rel)
      return (2);       /* release greater than current */

   if (level->mod < level->sys_mod)
      return (-1);      /* mod. less than current mod. */
   if (level->mod > level->sys_mod)
      return (1);       /* mod. greater than current */

   if (level->fix < level->sys_fix)
      return (-1);      /* fix less than current */
   if (level->fix == level->sys_fix)
      return (0);       /* fix equal to current */
   return (1);  /* fix greater than current */
} 

/* --------------------------------------------------------------------
 *
 * Function: Op_is_broken
 *
 * Purpose:  Determines if the option passed in is in the BROKEN state.
 *           Does this by querying the product table of the SWVPD.
 *
 * Returns:  TRUE  -- if op is BROKEN
 *           FALSE -- if op is NOT BROKEN
 *
 * -------------------------------------------------------------------- */

int Op_is_broken (Option_t * op)
{
   prod_t          prod;
   int             rc;

   memset (&prod, NULL, sizeof (prod_t));
   strcpy (prod.lpp_name, op->name);
   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;
   prod.state = ST_BROKEN;
   strcpy (prod.ptf, op->level.ptf);

   rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_STATE | PROD_VER | 
                                PROD_REL | PROD_MOD | PROD_FIX | PROD_PTF), 
                                &prod);
   vpd_free_vchars (PRODUCT_TABLE, &prod);
   if (rc == VPD_OK)
      return TRUE;
   return FALSE;
}

/* -------------------------------------------------------------------- * *
 *
 * Function:       inu_check_product
 *
 * Purpose:        Perform "BROKENs" check:
 *                  1. If this option is BROKEN,
 *                      make sure the -acgN flags were given.
 *                  2. If other pkgs with the same option name are BROKEN
 *                      disallow (inu_ckreq will pull it back in if it's
 *                      a requisite of a BROKEN).
 *
 * Returns:        TRUE  -  okay to install op
 *                 FALSE -  NOT okay to install op
 *
 * -------------------------------------------------------------------- */


static int inu_check_product (Option_t * op)
{
   prod_t          prod;
   int             rc, this_op_is_broken = 0;

  /** ----------------------------------------------------------------- *
   **  Don't check for BROKENS if 4.1, cuz sceded_by field isn't used.
   ** ----------------------------------------------------------------- */

   if ( !(IF_3_2(op->op_type) || IF_3_1(op->op_type)) )
      return (TRUE);

   memset (&prod, NULL, sizeof (prod_t));

   /** --------------------------------------------------------------- *
    **  See if anything is in the BROKEN state of the product to which
    **  op belongs to.  If not, then everything's okay.  We'll search
    **  by name, state AND an empty sceded_by field, since its ok for
    **  updates that are BROKEN to be superseded.
    ** --------------------------------------------------------------- */

   strcpy (prod.lpp_name, op->name);
   prod.state = ST_BROKEN;
   prod.sceded_by[0] = '\0';
   rc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE | PROD_SCEDED_BY, 
                &prod);

   if (rc == VPD_NOMATCH)
      return (TRUE);

   vpd_free_vchars (PRODUCT_TABLE, &prod);

   /** --------------------------------------------------------------- *
    **  Somethin's BROKEN.  Make sure the required flags are given
    **  (-acgN).
    ** --------------------------------------------------------------- */

   this_op_is_broken = Op_is_broken (op);

   if ( ! c_FLAG  ||  ! N_FLAG  ||  ! g_FLAG)
   {
      if (this_op_is_broken)
         op->Status = STAT_BROKEN;
      else
         op->Status = STAT_OTHER_BROKENS;
      return (FALSE);
   } 

   if (this_op_is_broken)
      return (TRUE);
   else
   {
      op->Status = STAT_OTHER_BROKENS;
      return (FALSE);
   }
} 

/* -------------------------------------------------------------------- *
 *
 * Function:  inu_check_prod_table
 *
 * Purpose:   Ensures that the update specified by the op parameter
 *            can be legally applied.  Checks for:
 *              1. If the update is already installed, return FALSE
 *              2. If the update is a 3.2 and is superseded, return FALSE
 *              3. If above pass, call a routine to check for BROKENs
 *
 * Returns:   TRUE - if the update can be legally applied
 *           FALSE - if the update CANNOT be legally applied
 *
 * -------------------------------------------------------------------- */

static int inu_check_prod_table (Option_t * op)
{
   prod_t          prod;
   int             rc;
   char            broken_op[200];

   memset (&prod, NULL, sizeof (prod_t));
   strcpy (prod.lpp_name, op->name);
   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;
   strcpy (prod.ptf, op->level.ptf);

  /*
   * For 3.2 pkgs,
   * We'll ignore the fixid on the vpd search, since it is quite possible for
   * a ptf to supersede another with a different fix id.  This makes a basic
   * assumption that ptf ids are unique for a given vrm.
   */

   if (IF_3_2(op->op_type))
      rc = vpdget (PRODUCT_TABLE,
                   (PROD_LPP_NAME | PROD_VER | PROD_REL | PROD_MOD |
                    PROD_PTF), &prod);
   else
      rc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME |
                   PROD_VER | PROD_REL | PROD_MOD | PROD_FIX), &prod);


   inu_ck_vpd_sys_rc (rc);

   if (rc == VPD_OK)
      vpd_free_vchars (PRODUCT_TABLE, &prod);  /* Don't need this data. */

   if (rc == VPD_NOMATCH)
      return (inu_check_product (op));

   if (rc == VPD_OK)
   {
      switch (prod.state)
      {
         case ST_AVAILABLE:
            if (*prod.sceded_by == '\0')
               return (inu_check_product (op));
            op->Status = STAT_ALREADY_SUPERSEDED;
            op->supersedes = strdup (prod.sceded_by); 
            return (FALSE);

         case ST_APPLIED:
         case ST_COMMITTED:
            op->Status = STAT_ALREADY_INSTALLED;
            return (FALSE);

         case ST_BROKEN:
         default:
            return (inu_check_product (op));
      } 
   } 

  return (FALSE); /* An error occurred.  Should report it ! */
} 


/* --------------------------------------------------------------------
 *
 * Function:  inu_get_prod
 *
 * Purpose:   Returns the product record of the specified lpp option.
 *
 * Returns:   The product record of the specified lpp option.
 *
 * -------------------------------------------------------------------- */

static int inu_get_prod (Option_t * op, 
                         prod_t   * prod)
{
   int             rc;

   strcpy (prod->lpp_name, op->name);
   prod->ver = op->level.ver;
   prod->rel = op->level.rel;
   prod->mod = op->level.mod;
   prod->fix = op->level.fix;
   strcpy (prod->ptf, op->level.ptf);

   rc = vpdget (PRODUCT_TABLE, 
                (PROD_LPP_NAME | PROD_VER | PROD_REL | PROD_MOD | PROD_FIX |
                 PROD_PTF), prod);

   inu_ck_vpd_sys_rc (rc);  /* check for terminating errors */

   return (rc);

} /* end inu_get_prod */

