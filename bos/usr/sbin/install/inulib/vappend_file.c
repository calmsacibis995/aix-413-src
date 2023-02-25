static char sccsid[] = "@(#)91  1.2  src/bos/usr/sbin/install/inulib/vappend_file.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:37";
/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: vappend_file  
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>
#include <stdarg.h>

/* ------------------------------------------------------------------------
 *
 * NAME: vappend_file
 *
 * FUNCTION:
 *    Appends info to the end of a given file
 *
 * EXECUTION ENVIRONMENT:
 *     SPOT server
 *
 * NOTES:
 *     Error Condition:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *     parameters:
 *           file     -- filename to append to
 *           format   -- format of info to be written
 *
 *     global:
 *        None
 *
 * RETURNS: (boolean)
 *     TRUE
 *     FALSE
 *
 * OUTPUT:
 *
 * ------------------------------------------------------------------------*/
boolean
vappend_file   (char *file,   
               char *format, 
               ...)   /* variable arguments fo vfprintf */
{
        FILE * fp;
        va_list arg; 

        va_start (arg, format);

        /* open for append */
        if ((fp = fopen (file, "a+")) == (FILE *)0) {
             inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                          CMN_CANT_OPEN_D), inu_cmdname, file);
             return (FALSE);
        }

        vfprintf (fp, format, arg);
        fclose (fp);

        return (TRUE);
}

