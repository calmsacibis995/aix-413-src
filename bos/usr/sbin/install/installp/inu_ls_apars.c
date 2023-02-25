static char sccsid[] = "@(#)69  1.18  src/bos/usr/sbin/install/installp/inu_ls_apars.c, cmdinstl, bos411, 9428A410j 6/16/94 17:34:56";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_ls_apars, inu_ls_missing_info
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <installp.h>
#include <instl_options.h>

/* ------------------------------------------------------------------------ *
 *
 *   FUNCTION NAME     inu_ls_apars
 *
 *   DESCRIPTION       prints out apar info -- simply cats the
 *                     LIBDIR/<optionname>.fixdata file (for usr/share) 
 *		       and cats the
 *                     LIBDIR/inst_root/<optionname>.fixdata file (for root).
 *
 *   PARAMETERS        cur_op   - 1st option to print apar info for
 *                     next_op - 1st option NOt to print apar  info for
 *
 *   RETURNS           always INUGOOD ???
 *
 *   NOTES             This function is called in a for loop that loops
 *                     through the sop (in inu_process_opts () in installp.c).
 *
 * ------------------------------------------------------------------------ */

int inu_ls_apars (Option_t * cur_op, Option_t * next_op)
{

    Option_t *op;

    for (op = cur_op; op != next_op; op = op->next)
	inu_ls_op_apars(op);

    return(SUCCESS);

}

void inu_ls_op_apars (Option_t * op)
{
   char        lpp_basename [PATH_MAX+1]; /* basename of printed lpp*/
   char        fix_file[PATH_MAX + 1];  /* path to fixinfo file   */

   strcpy (fix_file, INU_LIBDIR);
   strcat (fix_file, "/");
   strcat (fix_file, op->name);
   strcat (fix_file, ".fixdata");

   if (access (fix_file, F_OK) == 0)
   {
      op->bff->flags |= BFF_PASSED;
      inu_get_lpplevname_from_Option_t (op, lpp_basename);
      instl_msg (INFO_MSG, "\n%s", dashed_line);
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_APAR_INFO_I, 
                                 INP_APAR_INFO_D), lpp_basename);
      instl_msg (INFO_MSG, "%s\n", dashed_line);

      if (inu_cat (fix_file, "-", "") == FAILURE)
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, CMN_CANT_OPEN_E, 
                              CMN_CANT_OPEN_D), inu_cmdname, fix_file);
      else
         inu_rm (fix_file);
   }

  return;

} /* inu_ls_apars */

/* ------------------------------------------------------------------------ *
 *
 *   FUNCTION NAME     inu_ls_missing_info
 *
 *   DESCRIPTION       prints a sorted list of all the lpp's that did not have
 *                     any info (apar or instructions).  Also prints products
 *                     which could not be found on the installation media.
 *                     Intended to be called after inu_ls_apars or 
 *                     inu_ls_instr.
 *
 *   PARAMETERS        sop - Pointer to list of options to print info about.
 *
 *   RETURNS           Void function
 *
 * ------------------------------------------------------------------------ */

void inu_ls_missing_info (Option_t * sop)
{
   boolean    error = FALSE;
   char       lpp_basename [PATH_MAX+1]; /* basename of printed lpp*/
   boolean    missing_info = FALSE;
   Option_t * op;

   /* Did we have any missing apar or info files? */

   for (op = sop->next; op != NIL (Option_t); op = op->next)
   {
      if ( ! ((op->bff) && (op->bff->flags &BFF_PASSED)) )
         missing_info = TRUE;
   }

   /* Now list all of the things for which we could not find any info. */

   if (missing_info)
   {
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NO_INFO_I, INP_NO_INFO_D));

      /* Make sure that our list is sorted.  If our media is tape or diskette, 
         then our list is not sorted alphabetically, resort it. */

      if (TocPtr->type != TYPE_DISK)
      {
         TocPtr->type = TYPE_DISK;  /* Bold face lie, but what the heck. */

         inu_sort_sop (sop, &error);

         if (error)
            return;
      }

      for (op = sop->next; op != NIL (Option_t); op = op->next)
      {
         if ( ! ((op->bff) && (op->bff->flags &BFF_PASSED)))
         {
            if (op->bff)
                op->bff->flags |= BFF_PASSED;
            inu_get_lpplevname_from_Option_t (op, lpp_basename);
            instl_msg (INFO_MSG, "\t\t%s\n", lpp_basename);
         }
      }
      instl_msg (INFO_MSG, "\n");
   }

   for (op = sop->next; op != NIL (Option_t); op = op->next)
     if (op->bff)
        op->bff->flags &= ~BFF_PASSED;

   return;

} /* inu_ls_missing_info */
