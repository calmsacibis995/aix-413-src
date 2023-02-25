static char     sccsid[] = "@(#)60      1.7.1.19  src/bos/usr/sbin/install/installp/inu_commit_opts.c, cmdinstl, bos411, 9428A410j 6/6/94 15:50:23";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_commit_opts
 *              nothing_still_installed
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *---------------------------------------------------------------------*/

#include <sys/file.h>
#include <signal.h>
#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>


/*--------------------------------------------------------------------------*
**
** NAME:     inu_commit_opts ()
**
** FUNCTION: commits the options on sop from cur_op to next_op
**
** RETURN VALUE DESCRIPTION: always returns INUGOOD
**
**--------------------------------------------------------------------------*/

int inu_commit_opts (Option_t * cur_op, Option_t * next_op)
{
   char         lppname [MAX_LPP_NAME];      /* top level lpp name */
   char         lpp_opt_fil [L_tmpnam + 12]; /* path to lpp.options file */
   char         linkdir [PATH_MAX + 1];      /* link to directory path */
   char		savdir [PATH_MAX + 1];       /* save directory */
   char         buf [TMP_BUF_SZ];            /* generic character buffer */
   struct stat  stbuf;                       /* stat file information */
   Option_t    *op;                          /* generic Option_t pointer */

   (void) inu_get_prodname (cur_op, lppname);

   /*-----------------------------------------------------------------------*
    * Build the path to where the "lpp.options" file is.
    *-----------------------------------------------------------------------*/

   strcpy (lpp_opt_fil, INU_TMPDIR);
   strcat (lpp_opt_fil, "/lpp.options");

   /*-----------------------------------------------------------------------*
    * Ignore all signals while we update the vpd...
    *-----------------------------------------------------------------------*/

   inu_ignore_all_sigs ();

   /*-----------------------------------------------------------------------*
    * Set correct state for committing in PROD, and HISTORY databases
    *-----------------------------------------------------------------------*/

   for (op = cur_op; op != next_op; op = op->next)
   {
      (void) inu_vpd_lpp_prod (op, ST_COMMITTING);
      (void) inu_vpd_history (op, HIST_PENDING, HIST_COMMIT);
   }

   /*-----------------------------------------------------------------------*
    * Process the savedir for each option
    *-----------------------------------------------------------------------*/

   for (op = cur_op; op != next_op; op = op->next)
   {
      (void) inu_set_savdir (op->vpd_tree, op);

      /*----------------------------------------------------------------------*
       * If committing an update, warn if save files aren't removed because the
       * savdir is a "dangling" symbolic link (target directory not accessible).
       * This code expects the symbolic link to the alternate directory
       * to include the savdir path, as in: /savdir->/overhere/savdir
       *---------------------------------------------------------------------*/

      if (IF_UPDATE (op->op_type)  &&  ! IF_C_UPDT (op->op_type))
      {
         (void) inu_get_savdir (op->vpd_tree, op, savdir);

         if (strlen (savdir)  &&  stat (savdir, &stbuf) == -1)
         {
            /* directory missing */
            if (lstat (savdir, &stbuf) == 0)
            {
               /* a dangling symbolic link? */
               if (readlink (savdir, linkdir, sizeof (linkdir)) > 0)
               {
                  /* target of symbolic link is not accessible
                   * warn that saved files are not being removed */
                  instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                       INP_COMMIT_DANGLING_SAVDIR_SYM_LINK_E, 
                                       INP_COMMIT_DANGLING_SAVDIR_SYM_LINK_D), 
                                       linkdir, savdir);
                  /* unlink the symbolic link, rmdir () only for directories */
                  if (unlink (savdir) == -1)
                     perror ("unlink");  
               }
               /* else remove code will later delete symlink and directory */
            }
         }

         /*----------------------------------------------------------------*
          * Call inu_rcvr_or_rm () to remove the files from savdir
          *----------------------------------------------------------------*/
         else if (inu_rcvr_or_rm (lpp_opt_fil, CALLER_INSTALLP, lppname)
                                                                     != INUGOOD)
         {
                instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, INP_PART_COMMIT_W, 
                                     INP_PART_COMMIT_D), op->name, savdir);
         }
      }

      /*----------------------------------------------------------------*
       * If this is a 3_X option then the savedir will be the same for 
       * each of the other options on the sop chunk.
       *----------------------------------------------------------------*/
      if (IF_3_X (op->op_type))
         break;

   } /* for */

   /*-----------------------------------------------------------------------*
    * Set correct state for committed in LPP, PROD, and HISTORY databases
    *-----------------------------------------------------------------------*/

   for (op = cur_op; op != next_op; op = op->next)
   {
      op->Status = STAT_SUCCESS;
      (void) inu_vpd_lpp_prod (op, ST_COMMITTED);
      (void) inu_vpd_history (op, HIST_COMPLETE, HIST_COMMIT);
      if (IF_INSTALL (op->op_type))
         (void) inu_vpd_set_states (op, DELETE_HOLD_STATES);
   }

   /*-----------------------------------------------------------------------*
    * Set the signals back to their default settings...
    *-----------------------------------------------------------------------*/

   inu_init_sigs ();

   /* Ideally when a save dir is under an alternate save location, 
    * then empty directories above the savdir, up to what would
    * have been the default root, would also be removed */

   rm_biron_files (cur_op, next_op, OP_COMMIT);
   return  (INUGOOD);
} 


/* ------------------------------------------------------------------------ *
**
**  Function:  nothing_still_applied_for_this_opt
**
**  Purpose:   To determine if any of op's sister options (option belonging
**             to same lpp) are in the state specified by the state parameter
**             (ST_APPLIED or ST_COMMITTED).  
**
**  Returns:   TRUE -  if NONE of lpp's options which op belongs to are in the
**                     state specified by the state parameter.
**   
**             FALSE - if one or more of lpp's options are in the 
**                     specified state. 
**                     
** -------------------------------------------------------------------------- */

int nothing_still_installed (Option_t * op, int state)
{
   int     rc;                  
   prod_t  prod;
   char    lppname [PATH_MAX];

  /*---------------------------------------------------------------------- *
   * If we're dealing with bos.rte, then it's okay to blast the lpp.cleanup
   * and the lpp.reject scripts, cuz bos.rte only has one option.
   *---------------------------------------------------------------------- */

   if (strcmp (op->name, "bos.rte") == 0)
      return (TRUE);

   memset (&prod, NULL, sizeof (prod_t));

  /*---------------------------------------------------------- *
   * Initialize the prod_t structure with the query info. 
   *---------------------------------------------------------- */

   (void) inu_get_prodname (op, lppname);
   strcat (lppname, ".*");
   strcpy (prod.lpp_name, lppname);

   prod.state = state;

   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;
   strcpy (prod.ptf, op->level.ptf);

  /*---------------------------------------------------------- *
   * Initialize the prod_t structure with the query info. 
   *---------------------------------------------------------- */

   rc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_STATE | PROD_VER |
                PROD_REL | PROD_MOD | PROD_FIX | PROD_PTF, &prod); 

   inu_ck_vpd_sys_rc (rc);

  /*------------------------------------------------------------------- *
   * At least one other option for this product IS in the applied state 
   *------------------------------------------------------------------- */

   if (rc == VPD_OK)
   {
      vpd_free_vchars (PRODUCT_TABLE, &prod);
      return (FALSE);
   }

   return (TRUE);   /* NONE of appropriate opts have an APPLIED usr part */
}

