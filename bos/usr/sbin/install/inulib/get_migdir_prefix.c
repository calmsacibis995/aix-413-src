static char sccsid[] = "@(#)38  1.1  src/bos/usr/sbin/install/inulib/get_migdir_prefix.c, cmdinstl, bos411, 9428A410j 1/20/94 10:24:07";

/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: get_migdir_prefix
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <installp.h>

/* ------------------------------------------------------------------------ *
 *
 *  Function:    get_migdir_prefix
 *
 *  Purpose:     Return the correct libdir based on vpd_tree
 *
 *  Parameters:  op
 *
 *  Global:      None
 *
 *  Returns:     Pointer to libdir on success
 *               Pointer to NIL(char) on Failure.
 *
 * ------------------------------------------------------------------------ */
char *
get_migdir_prefix (Option_t *op)
{
   char *prefix=NIL(char);

   switch (op->vpd_tree)
       {
         case VPDTREE_USR:
                            prefix="/usr/lpp";
                               break;
         case VPDTREE_ROOT:
                           prefix="/lpp";
                                break;
         case VPDTREE_SHARE:
                            prefix="/usr/share/lpp";
                                 break;
         default:
                            break;
       }

   return (prefix);

}

