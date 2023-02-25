static char sccsid[] = "@(#)88	1.13  src/bos/usr/sbin/install/installp/inu_do_status.c, cmdinstl, bos411, 9428A410j 5/17/94 17:05:46";

/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inu_do_status
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ----------------------------------------------------------------------- 
**
** FUNCTION:            inu_do_status
**
** DESCRIPTION:         Loops through the selected options list -- which 
**           at this point should only contain options in the APPLIED 
**           state -- and prints status info concerning each option.  The
**           status info consists of name, part, level, and state.
**
** PARAMETERS:          SopPtr   --   Ptr to the selected options list.
**
** RETURNS:
**
** ----------------------------------------------------------------------- */


/* #include <stdio.h>
   #include <installp.h>
   #include <instl_define.h>
   #include <instl_options.h>
   #include <inu_string.h>
*/

#include <installp.h>
#include <instl_define.h>
#include <instl_options.h>
#include <inu_string.h>
#include <ckp_common_defs.h>

int inu_do_status (Option_t * SopPtr)
{

   Option_t *op;
   char     *part;
   char     level [MAX_LEVEL_LEN];

   if (SopPtr->next == NIL (Option_t))
      return INUGOOD;

  /*---------------------------------------------------------------
   * Get the translated messages and put them in the strings array
   *--------------------------------------------------------------*/

   inu_init_strings ();

  /*---------------------------------------------------------------
   * Display the header lines -- which could be different depending
   * on whether we were called by SMIT or not.
   *--------------------------------------------------------------*/

   if (J_FLAG)
   {
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_SMIT_STATUS_HEAD_I, 
                                                INP_SMIT_STATUS_HEAD_D), "# ");
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_STATUS_HEAD_LINE_I, 
                                                INP_STATUS_HEAD_LINE_D), "# ");
   }
   else
   {
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_STATUS_I, 
                                                INP_STATUS_D), " ");
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_STATUS_LINE_I, 
                                                INP_STATUS_LINE_D), " ");
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_STATUS_HEAD_I, 
                                                INP_STATUS_HEAD_D), " ");
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_STATUS_HEAD_LINE_I, 
                                                INP_STATUS_HEAD_LINE_D), " ");
   }


   for (op = SopPtr->next; op != NULL; op = op->next)
   {
      switch (op->vpd_tree)
      {
         case VPDTREE_ROOT  :  part = inu_unpad (string [ROOT_STR]);  break;
         case VPDTREE_USR   :  part = inu_unpad (string [USR_STR]);   break;
         case VPDTREE_SHARE :  part = inu_unpad (string [SHARE_STR]); break;
      }

      if (op->level.ptf [0] == '\0')
         sprintf (level, "%d.%d.%d.%d", op->level.ver, op->level.rel, 
                  op->level.mod, op->level.fix);
      else
         sprintf (level, "%d.%d.%d.%d.%s", op->level.ver, op->level.rel, 
                  op->level.mod, op->level.fix, op->level.ptf);

      if (J_FLAG)
         printf ("  %-35s %-25s\n", op->name, level);

      else if (strlen (op->name) > 35)
      {
         printf (" %s\n", op->name);
         printf ("                                     %-7s %-23s %-7s\n", 
                 part, level, inu_unpad (string [APPLIED_STR]));
      }
      else
         printf (" %-35s %-7s %-23s %-7s\n", op->name, part, level,
                 inu_unpad (string [APPLIED_STR]));
   }

   return INUGOOD;
}
