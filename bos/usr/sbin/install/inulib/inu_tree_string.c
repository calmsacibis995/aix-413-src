static char sccsid[] = "@(#)09  1.3  src/bos/usr/sbin/install/inulib/inu_tree_string.c, cmdinstl, bos411, 9428A410j 2/11/94 15:35:29";

/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: inu_tree_string
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

#include <inulib.h>
#include <inu_string.h>

/* ------------------------------------------------------------------------
 *
 * NAME: inu_tree_string
 *
 * FUNCTION:
 *     Given a valid vpd_tree character, returns the string string[ROOT2_STR]
 *     if vpd_tree is VPDTREE_ROOT, string[USR2_STR] for VPDTREE_USR, and
 *     string[SHARE2_STR] for VPDTREE_SHARE.  The string that is returned can
 *     be translated. 
 *
 * RETURNS: character string
 * ------------------------------------------------------------------------*/
char * inu_tree_string (char vpd_tree)
{
   switch (vpd_tree)
   {
      case VPDTREE_USR:
         return (inu_unpad (string[USR2_STR]));
         break;
      case VPDTREE_ROOT:
         return (inu_unpad (string[ROOT2_STR]));
         break;
      case VPDTREE_SHARE:
         return (inu_unpad (string[SHARE2_STR]));
         break;
      default:
         return (inu_unpad (string[UNKNOWN_STR]));
         break;
   }
}
