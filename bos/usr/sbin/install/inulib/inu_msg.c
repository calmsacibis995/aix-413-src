static char sccsid[] = "@(#)16  1.14  src/bos/usr/sbin/install/inulib/inu_msg.c, cmdinstl, bos411, 9428A410j 6/16/94 17:33:17";
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inu_msg 
 *            Control where the output is displayed.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdarg.h>
#include <inulib.h> 


int inu_doing_logging=FALSE;

/*--------------------------------------------------------------------------
**
** NAME:      inu_msg
**
** FUNCTION:  Given a output indicator and a list of character strings this
**            function will determine if the message should be output, and
**            if so, where it should go (to the screen, log, both, or 
**            neither).
**
** NOTE 2:    This routine assumes the cmdinstl message catalog is already
**            open and that it'll be closed somewhere else.
**
** RETURNS:   none
**
** ------------------------------------------------------------------------*/

void inu_msg (int output_indicator, char *format, ...)
{
   boolean logging     = FALSE,  /* indicates whether we're logging */ 
           display_msg = FALSE,  /* indicates that we'll display the message */ 
           log_msg     = FALSE;  /* indicates that if we're logging then
                                    we'll log the message */ 
   va_list arg;

   logging = inu_doing_logging;

   va_start (arg, format);

   switch (output_indicator) 
   {
      case PROG_MSG:
      case SCREEN_MSG:
         display_msg = TRUE;
         break;

      case FAIL_MSG:
      case WARN_MSG:
      case INFO_MSG:
      case SUCC_MSG:
      case SCREEN_LOG:
         log_msg     = TRUE;
         display_msg = TRUE;
         break;

      case LOG_MSG:
         log_msg     = TRUE;
         break;

      case DEV_NULL:
         break;

      default:
         /*
          * An illegal (perhaps uninitialized) value was passed as a
          * destination.  Send msg to screen and log (if logging).
          */
         display_msg = TRUE;
         if (logging)
            log_msg = TRUE;
         break;
   }

   if (log_msg)  
      if (logging || ! display_msg)
         vfprintf (stdout, format, arg);

   if (display_msg)
      vfprintf (stderr, format, arg);

   va_end (arg);
}
