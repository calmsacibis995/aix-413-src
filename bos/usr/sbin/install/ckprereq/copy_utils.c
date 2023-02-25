static char sccsid[] = "@(#)70  1.10.1.21  src/bos/usr/sbin/install/ckprereq/copy_utils.c, cmdinstl, bos411, 9428A410j 4/27/94 18:45:16";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *           dupe_string 
 *           copy_TOC_to_fix_info 
 *           copy_vpd_to_fix_info
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

/*--------------------------------------------------------------------------*
**
** NAME: dupe_string
**
** FUNCTION:  This routine duplicates a string.  Storage is malloced and
**            the copy operation is performed.
**
** RETURNS:   Returns a character pointer to new copy of the string.
**
**
**--------------------------------------------------------------------------*/

char * dupe_string (char * source, boolean * error)
 {
   char * temp;

   if (source == NIL (char))
      return NIL (char);

   temp = strdup (source);
   #pragma covcc !instr
   if (temp == NIL (char))
      * error = TRUE;
   #pragma covcc instr
   return (temp);

 } /* end dupe_string */

/*--------------------------------------------------------------------------*
**
** NAME: copy_TOC_to_fix_info
**
** FUNCTION:  This routine copies relevant information from an Option_t
**            structure to a fix_info_type structure.  Storage for the
**            fix_info_type structure is malloc'ed, initialized.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void copy_TOC_to_fix_info (Option_t         * option,
                           fix_info_type   ** fix,
                           fix_origin_type    origin,
                           boolean          * error)
{

   fix_info_type * new_fix;

   new_fix = create_fix_info_node (error);
   RETURN_ON_ERROR;

   new_fix -> name = dupe_string (option -> name, error);
   RETURN_ON_ERROR;

   new_fix -> product_name = dupe_string (option -> prodname, error);
   RETURN_ON_ERROR;

   new_fix -> cp_flag =
                      convert_content_type_to_cp_flag_type (option -> content);
   if (option -> bff != NIL (BffRef_t) )
      new_fix -> cp_flag |=
                 convert_action_type_to_cp_flag_type (option -> bff -> action);

   if (new_fix -> cp_flag & LPP_MICRO)
   {
      new_fix -> parts_in_fix = LPP_USER;
   }
   else
   {
      new_fix -> parts_in_fix =
                       new_fix -> cp_flag & (LPP_USER | LPP_ROOT | LPP_SHARE);
   }
   new_fix -> op_type = option -> op_type;

   /*
    * Since we're also called to copy sop entries to fix_info structures let's 
    * not give false impressions of having a toc entry for this option.
    */
   if (check_prereq.mode == OP_APPLY && 
       check_prereq.parts_to_operate_on != LPP_ROOT)
      new_fix -> TOC_Ptr = option;
 
   new_fix->supersedes_string = dupe_string (option -> supersedes, error); 
   new_fix -> prereq_file = dupe_string (option -> prereq, error);
   RETURN_ON_ERROR;

   new_fix -> level = option -> level;

   if (check_prereq.keep_description_info)
   {
      new_fix -> description = dupe_string (option -> desc, error);
      RETURN_ON_ERROR;
   }

   if (option -> bff != NIL (BffRef_t) )
   {
      if ((option -> bff -> action == ACT_INSTALLP_UPDT) ||
          (option -> bff -> action == ACT_REQUIRED_UPDT))
      {
          new_fix -> flags |= MANDATORY_UPDATE;
      }
   }

   if (IF_DUPE (option->flag))
      new_fix->flags |= CONFL_TOC_BASE_LEVEL;

   new_fix -> origin = origin;

   if (new_fix -> origin == COMMAND_LINE)
   {
      if (check_prereq.mode != OP_APPLY || IF_SELECTED (option -> flag))
      {
         new_fix -> flags |= EXPL_REQUESTED_PKG;
      }

      switch (check_prereq.mode)
      {
          case OP_APPLY :
            new_fix -> apply_state = TO_BE_EXPLICITLY_APPLIED;
            break;

          case OP_COMMIT :
            new_fix -> commit_state = TO_BE_EXPLICITLY_COMMITTED;
            break;

          case OP_REJECT :
            new_fix -> reject_state = TO_BE_EXPLICITLY_REJECTED;
            break;
       }
   }
   else
   {
      new_fix -> apply_state = IN_TOC;
   }

   * fix = new_fix;

} /* end copy_TOC_to_fix_info */

/*--------------------------------------------------------------------------*
**
** NAME: copy_vpd_to_fix_info
**
** FUNCTION:  This routine copies the relevant information from a prod_t
**            structure returned by the swvpd routines over to a
**            fix_info_type structure.  Space is malloced for the new
**            entry and it is inserted in the fix_list.  (Remember that
**            fix_list is the header for the list).
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void copy_vpd_to_fix_info (prod_t        * product_info,
                           fix_info_type * fix_list,
                           short           vpd_source,
                           boolean       * error)
 {
   short           applied_flag;
   short           committed_flag;
   fix_info_type * fix;
   char            fix_name[MAX_LPP_FULLNAME_LEN];
   short           parts_in_fix;
   long            cp_flag;
   char          * supersede_info;

   applied_flag = 0;
   committed_flag = 0;
   parts_in_fix = product_info -> cp_flag & (LPP_USER | LPP_ROOT | LPP_SHARE);
   cp_flag = product_info -> cp_flag;

  /* There is always a header to the fix_list */

   fix = create_fix_info_node (error);
   RETURN_ON_ERROR;

   switch (product_info -> state)
    {
      case ST_AVAILABLE :
         break;

      case ST_BROKEN    :
         /*
          * For deinstall, we want it to look like there are actually parts
          * applied (since deinstall of BROKEN pkgs is a valid operation).
          */
         if (check_prereq.deinstall_submode)
            applied_flag = vpd_source;
         break;

      case ST_APPLIED   :
         applied_flag = vpd_source;   /* Turn this bit on as being applied */
         break;

      case ST_APPLYING  :
         if (!check_prereq.called_from_ls_programs)
            applied_flag = parts_in_fix;  /* Pretend that everything will be ok. */
         fix->flags |= CKP_ST_APPLYING; /* needed only by lppchk */

         /* During a Forced re-apply, it is possible for the ROOT part to be
            committed while re-installing the USER part. */

         if (check_prereq.Force_Install && (vpd_source == LPP_USER) )
            committed_flag = vpd_source;
         break;

      case ST_COMMITTED :
         applied_flag = vpd_source;   /* Turn this bit on as being applied */
         committed_flag = vpd_source;
         break;

      case ST_COMMITTING :
         if (!check_prereq.called_from_ls_programs)
         {
            applied_flag = vpd_source;
            committed_flag = vpd_source;
         }
         fix->flags |= CKP_ST_COMMITTING;  /* needed only by lppchk */
         break;

      default :
         /*
          * Normally, we don't care about pkgs with any other state. 
          * When called by the "ls programs" (particularly lppchk)
          * we want to know about "ING" states.  Set a tag bit and make
          * it look like the pkg is really applied.
          */
         if ((product_info->state == ST_REJECTING ||
              product_info->state == ST_DEINSTALLING) 
                              &&
             (check_prereq.called_from_ls_programs))
         {
            if (product_info->state == ST_REJECTING)
                fix->flags |= CKP_ST_REJECTING;
            else
                fix->flags |= CKP_ST_DEINSTALLING;
         }
         else
            free_fix_info_node (fix);
         return;    /*  This fix is not really there */
    } /* end switch */


   if (product_info -> state == ST_BROKEN)
    {
      /* A fix will be marked as BROKEN according to the following criteria:
           1. It's an update -- a message must be printed out in accordance
              with installp's -acgN rule for recovering broken updates.
           2. It's not an update but we're not doing an APPLY.  This is so that
              we allow a BROKEN base level to be pulled in via auto-include --
              since base levels can be installed on top of BROKEN base levels.
              For COMMIT and REJECT operations, we still want to mark the
              fix_info structure as BROKEN.
           3. We're being called from a listing program lslpp or installp -l. */

      if ((product_info -> cp_flag & LPP_UPDATE)
                          ||
            (check_prereq.mode != OP_APPLY)
                          ||
            (check_prereq.called_from_ls_programs))
       {
         fix -> apply_state = BROKEN;
       }
      else
       {
         fix -> apply_state = AVAILABLE;
       }
    }

   fix -> name = dupe_string (product_info -> lpp_name, error);
   RETURN_ON_ERROR;

   fix -> product_name = dupe_string (product_info -> name, error);
   RETURN_ON_ERROR;

   fix -> level.ver = product_info -> ver;
   fix -> level.rel = product_info -> rel;
   fix -> level.mod = product_info -> mod;
   fix -> level.fix = product_info -> fix;
   strcpy (fix -> level.ptf, product_info -> ptf);
   strcpy (fix -> superseding_ptf, product_info->sceded_by);
   fix->supersedes_string = strdup (product_info->supersedes);
   
   fix -> parts_in_fix = parts_in_fix;
   fix -> cp_flag = cp_flag;

   if (cp_flag & LPP_MICRO)
    {
      #pragma covcc !instr
      if ( ! (cp_flag & LPP_31_FMT) )
       {
        /* "Internal error: unexepected OP_TYPE => LPP_MICRO for %s\n" */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_UNEXP_LPPMICRO_E,
                  CKP_INT_UNEXP_LPPMICRO_D), inu_cmdname,
                  get_fix_name_from_fix_info (fix, fix_name));
         check_prereq.function_return_code = UNEXPECTED_LPP_MICRO;
         * error = TRUE;
         return;
       }
      else
       {
         fix -> parts_in_fix = LPP_USER;  /* Treat as a user part. */
       }
      #pragma covcc instr
    }
   if (product_info -> cp_flag & LPP_INSTAL)
      fix -> op_type |= OP_TYPE_INSTALL;
   if (product_info -> cp_flag & LPP_UPDATE)
      fix -> op_type |= OP_TYPE_UPDATE;
   if (product_info -> cp_flag & LPP_31_FMT)
      fix -> op_type |= OP_TYPE_3_1;
   if (product_info -> cp_flag & LPP_32_FMT)
      fix -> op_type |= OP_TYPE_3_2;
   if (product_info -> cp_flag & LPP_41_FMT)
      fix -> op_type |= OP_TYPE_4_1;

   if (IF_LPP_PKG_PTF_TYPE_C (product_info -> cp_flag))
      fix -> op_type |= OP_TYPE_C_UPDT;
   if (IF_LPP_PKG_PTF_TYPE_E (product_info -> cp_flag))
      fix -> op_type |= OP_TYPE_E_UPDT;
   if (IF_LPP_PKG_PTF_TYPE_M (product_info -> cp_flag))
      fix -> op_type |= OP_TYPE_M_UPDT;
   if (IF_BOSBOOT (product_info->cp_flag))
      fix -> op_type |= OP_TYPE_BOSBOOT;

   fix -> parts_applied = applied_flag;
   fix -> parts_committed = committed_flag;
   fix -> origin = VPD;

   fix -> prereq_file = dupe_string (product_info->prereq, error);
   RETURN_ON_ERROR;

   if (check_prereq.keep_description_info)
   {
     fix->description = dupe_string (product_info->description, error);
     RETURN_ON_ERROR;
   }

   if (check_prereq.keep_apar_info)
   {
      fix->apar_info = dupe_string (product_info->fixinfo, error);
      RETURN_ON_ERROR;
   }

   fix -> next_fix = fix_list -> next_fix;
   fix_list -> next_fix = fix;

 } /* end copy_vpd_to_fix_info */

