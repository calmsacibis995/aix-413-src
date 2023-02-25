/* @(#)70   1.15 src/bos/usr/sbin/install/installp/inu_ls_instr.c, cmdinstl, bos411, 9428A410j 3/6/94 19:31:33 */

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_ls_instr, inu_display_file
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

static boolean inu_display_file (Option_t * toc_op, 
                                 char     * lpp_basename, 
                                 char     * fname);

/* -----------------------------------------------------------------------
 *
 * FUNCTION          inu_display_file
 *
 * DESCRIPTION       Displays (via cat) a file to stdout.
 *
 * PARAMETERS        fname  -  name of file to display part
 *                          -  root or usr (pertinant for dir name)
 *
 * RETURNS           none   - (void function)
 *
 * ----------------------------------------------------------------------- */

static boolean inu_display_file (Option_t * toc_op, 
                                 char     * lpp_basename, 
                                 char     * fname)  /* filename to display */
{
   char            catfile[PATH_MAX * 2];

   sprintf (catfile, "%s/%s", INU_LIBDIR, fname);

   if (access (catfile, R_OK) == 0)
   {
      instl_msg (INFO_MSG, "\n%s", dashed_line);
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_INSTRUCTIONS_I, 
                            INP_INSTRUCTIONS_D), lpp_basename, catfile);
      instl_msg (INFO_MSG, "%s\n", dashed_line);
      inu_cat (catfile, "-", "");
      return (TRUE);
   }
   else
      return (FALSE);

} /* end inu_display_file */

/* -------------------------------------------------------------------------
 *
 * NAME: inu_ls_instr
 *
 * FUNCTION: This function will print out the information required by the -A
 *           option on installp.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS:  SUCCESS   If no errors were found in outputing the information
 *           FAILURE   If any error was found.
 *
 * ------------------------------------------------------------------------- */


int inu_ls_instr (Option_t * op)
{
   boolean    info_printed;
   char       lpp_basename[MAX_LPP_OPTIONNAME_LEN];   /* base name of lpp */

   inu_get_lpplevname_from_Option_t (op, lpp_basename);

   info_printed = inu_display_file (op, lpp_basename, "README");
   info_printed |= inu_display_file (op, lpp_basename, "lpp.README");
   info_printed |= inu_display_file (op, lpp_basename, "lpp.doc");
   info_printed |= inu_display_file (op, lpp_basename, "lpp.instr");
   info_printed |= inu_display_file (op, lpp_basename, "lpp.lps");

   /* Defect 100582 */
   if (info_printed)
      op->bff->flags |= BFF_PASSED;

   return (SUCCESS);

} /* end inu_ls_instr */
