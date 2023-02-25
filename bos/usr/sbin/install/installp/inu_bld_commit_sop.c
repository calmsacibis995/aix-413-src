static char sccsid[] = "@(#)05  1.21  src/bos/usr/sbin/install/installp/inu_bld_commit_sop.c, cmdinstl, bos411, 9428A410j 3/6/94 19:30:33";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_bld_commit_sop
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


/*--------------------------------------------------------------------------*
**
** NAME: inu_bld_commit_sop ()
**
** FUNCTION:Scans the Selected Options List looking for options that have
**          successfully installed so we can build a new sop to do the commits.
**
** NOTE:
**
** RETURNS:  The number of commits that needs to be done.
**
**--------------------------------------------------------------------------*/

int inu_bld_commit_sop 
  (Option_t * sop,        /* Ptr to the 1st option of this operation      */
   Option_t ** next_op,   /* Ptr to the 1st option of the next operation  */
   Option_t * failsop)    /* Ptr to the anchor of the failure sop         */
{
   Option_t  *CopPtr;      /* temp ptr to the Option_t struct type.       */
   Option_t  *op;          /* temp ptr to the Option_t struct type.       */
   Option_t  *rop;         /* temp ptr to the Option_t struct type.       */
   Option_t  *cop;         /* temp ptr to the copied Option_t struct.     */
   Option_t  *savesop;     /* temp ptr to the saved failsop header.       */
   Option_t  *last_option; /* Ptr to the last option in the current bff.  */
   Option_t  *last_insert; /* Ptr to the last option that was inserted.   */
   char      lppname[MAX_LPP_NAME];                 /* top level lpp name */
   int       num_found = 0;  /* count of the number of root applys to do  */
   short     tree;        /* short to tell what parts we're dealing with  */
   int       rc;          /* return code */
   uchar     commit_option;  /* Specifies whether option should be committed */
   uchar     installp_apply_commit; 
            /* is this the commit option of an apply-commit operation. */

   /*-------------------------------------------------------------------*
    * Create the anchor for the COMMIT Selected Option List. The new sop
    * will contain a linked list of options that need to be committed.
    *-------------------------------------------------------------------*/

   CopPtr = create_option ("Committed Selected Option List", 
                           NO,  /* default op_checked field to NO */
                           NO,  /* default quiesce field to NO */
                           CONTENT_UNKNOWN,     /* default content field to
                                                 * unknown */
                           "",  /* default lang field to NIL (char) */
                           NIL (Level_t),       /* default level field to
                                                 * NIL (Level_t) */
                           "",  /* default desc field to NIL (char) */
                           NIL (BffRef_t));     /* default bff field to
                                                 * NIL (BffRef_t) */

   if (CopPtr == NIL (Option_t))
   {
      inu_get_prodname (sop, lppname);
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_COMMIT_ERROR_E, 
                                   INP_COMMIT_ERROR_D), lppname);
      return (SUCCESS);
   }

   /*----------------------------------------------------------------------*
    * Scan through the sop to find the last option of this operation, because
    * we're going to put the commit options in the main sop after the usr and
    * root applys.
    *----------------------------------------------------------------------*/

   for (op = sop;
        op->next != *next_op;
        op = op->next)
   {
     ;
   }

   last_option = op;
   last_insert = CopPtr;

   /*----------------------------------------------------------------------*
    * Scan through the Selected Options List to see if any of the options
    * for this operation needs to be committed.
    *----------------------------------------------------------------------*/

   for (op = sop; op != *next_op; op = op->next)
   {
      /*------------------------------------------------------------------*
       * We don't want to add base levels to the cop because they are
       * already committed (by the apply operation) through auto-commit of
       * base levels.  Note:  inu_bld_commit_sop is only called when 
       * op->operation == OP_APPLYCOMMIT.
       *------------------------------------------------------------------*/

       if (IF_INSTALL (op->op_type))
          continue;

      /*------------------------------------------------------------------*
       * If the apply of the usr and root parts succeed, then add an options
       * structure in the new sop "commit" link list.
       *------------------------------------------------------------------*/

      if (op->Status == STAT_SUCCESS)
      {

         /* The following code was added to fix defect 24146.  It prevents
          * installp from committing a user part if its root part failed.
          *
          * If the user part was successful and the option has a root part */

         commit_option = TRUE;

         if ((op->content == CONTENT_BOTH)  &&  (op->vpd_tree == VPDTREE_USR))
         {
            /* Scan for the corresponding root part and ensure that it was
             * successfully applied. */

            commit_option = FALSE;

            for (rop = op->next; rop != *next_op; rop = rop->next)
            {

               if ((rop->vpd_tree == VPDTREE_ROOT)
                    &&  (rop->Status == STAT_SUCCESS)
                    &&  (strcmp (rop->name, op->name) == 0)
                    &&  (rop->level.ver == op->level.ver)
                    &&  (rop->level.rel == op->level.rel)
                    &&  (rop->level.mod == op->level.mod)
                    &&  (rop->level.fix == op->level.fix)
                    &&  (strcmp (rop->level.ptf, op->level.ptf) == 0))
               {

                  commit_option = TRUE;
                  break;

               }  /* end if corresponding root part was successfully
                   * applied */

            } /* end for scan for corresponding root part */

         } /* if the user part was successfully applied and has a root
            * part */

         /*--------------------------------------------------------------*
          * Make a copy of this option so we have an option entry for
          * the commit operation and add it to the new sop.
          *--------------------------------------------------------------*/

         if (commit_option == TRUE)
         {
            num_found++;
            cop = copy_option (op);     /* copy the apply option struct */
            cop->operation = OP_COMMIT;       /* Mark operation to commit.    */
            last_insert->next = cop;  /* link to the end of last opt  */
            cop->next = NIL (Option_t);       /* link new opt's next to NIL   */
            last_insert = cop;  /* make the new option the last */

         } /* if commit_option */

      } /* the current option was successsfull */

   } /* end main for loop */

   /*----------------------------------------------------------------------*
    * If no options were found that needed to be committed then return 0.
    *----------------------------------------------------------------------*/

   if (num_found == 0)
      return (num_found);


   /*----------------------------------------------------------------------*
    * Call inu_ckreq () which is the library interface to the ckprereq command.
    * The second arg is the ptr to the toc linked list (which in this case
    * we don't want ckprereq to look at so we pass it a null pointer).
    * The third arg is the file ptr to the prereq file (which in this case
    * doesn't exist).
    * The fourth arg is the verbose flag which we don't want set if being
    * called from here.
    *---------------------------------------------------------------------*/

   tree = 0;
   if (ROOT_PART)
      tree = LPP_ROOT;
   if (USR_PART)
      tree |= LPP_USER;
   if (SHARE_PART)
      tree |= LPP_SHARE;

   /*----------------------------------------------------------------------*
    * Set a flag that tells ckprereq whether or not this is the commit 
    * portion of an apply-commit operation for an installp update.
    *---------------------------------------------------------------------*/

   installp_apply_commit = FALSE;
   if (a_FLAG && c_FLAG)
   {
      for (op = CopPtr->next;
           op != NIL (Option_t);
           op = op->next)
      {
         if (op->bff != NIL (BffRef_t))
         {
            if (op->bff->action == ACT_INSTALLP_UPDT)
               installp_apply_commit = TRUE;
         }
      }
   }

   /*----------------------------------------------------------------------*
    * Print a section header for commit part of -ac op.
    *----------------------------------------------------------------------*/
    inu_display_section_hdr (SECT_HDR_PI_PROC, OP_COMMIT);
   
   /*----------------------------------------------------------------------*
    * Clear and save the failsop so that inu_ckreq () doesn't repeat messages 
    * that were printed during requisite checking of the apply portion of this 
    * -ac operation: assumes no pre-commit failures were added to the failsop. 
    * Also, any requisite failures for the commit portion will not be on the 
    * failsop upon return.
    *---------------------------------------------------------------------*/

   savesop = failsop->next;
   failsop->next = NIL (Option_t);
   rc = inu_ckreq (CopPtr, failsop, NIL (TOC_t), FALSE, g_FLAG, 
                                          installp_apply_commit, tree, FALSE);
   failsop->next = savesop;

   if (rc != SUCCESS)
   {
      inu_get_prodname (sop, lppname);
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_COMMIT_ERROR_E, 
                                   INP_COMMIT_ERROR_D), lppname);
      return (SUCCESS);
   }

   /*----------------------------------------------------------------------*
    * If no options were found that needed to be committed then return 0.
    *----------------------------------------------------------------------*/

   if (CopPtr->next == NIL (Option_t))
      return (0);

   /*----------------------------------------------------------------------*
    * Now merge our new sop list (CopPtr) with the main sop list.
    *----------------------------------------------------------------------*/

   last_option->next = CopPtr->next;
   for (op = CopPtr->next;
        op->next != NIL (Option_t);
        op = op->next)
   {
      ;
   }

   op->next = *next_op;
   *next_op = last_option->next;

   return (num_found);

} /* end inu_bld_commit_sop */
