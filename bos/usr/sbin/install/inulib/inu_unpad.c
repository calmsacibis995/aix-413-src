static char sccsid[] = "@(#)10  1.4  src/bos/usr/sbin/install/inulib/inu_unpad.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:31";

/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: inu_unpad
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
#include <string.h>

/* ------------------------------------------------------------------------
 *
 * NAME: inu_unpad
 *
 * FUNCTION:
 *     Given a string, it returns a string without the blanks.  If there
 *     is a malloc error for some reason, return the original in_string.
 *
 * RETURNS: character string
 * ------------------------------------------------------------------------*/
char * inu_unpad (char * in_string)
{
   char  *out_string;

   if ((out_string = (char *) malloc (strlen (in_string) + 1)) != NIL (char))
   {
      strcpy (out_string, in_string);
      (void) strtok (out_string, " ");
   }
   else
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                                 CMN_MALLOC_FAILED_D));

      /* in the event of a malloc error, return NULL */
      out_string = NULL;
   }

   return (out_string);
}
