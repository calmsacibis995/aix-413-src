static char sccsid[] = "@(#)88  1.3  src/bos/usr/sbin/install/installp/instl_msg.c, cmdinstl, bos41B, 412_41B_sync 1/14/95 15:21:19";
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: instl_msg 
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
#include <instl_options.h>


/*--------------------------------------------------------------------------
**
** NAME:      instl_msg
**
** FUNCTION:  Given a output indicator and a list of character strings this
**            function will determine if the message should be output (based
**            on the verbosity level (V_FLAG), and if so, where it should go
**            (to the screen, log, or both).
**
** NOTE 1:    stderr - used to output messages to the screen
**            stdout - used to output messages to the log
**
** NOTE 2:    This routine assumes the cmdinstl message catalog is already
**            open and that it'll be closed somewhere else.
**
** RETURNS:   none
**
** ------------------------------------------------------------------------*/

void instl_msg (int output_indicator, char *format, ...)
{
   boolean logging     = FALSE,  /* indicates whether we're logging */
           display_msg = FALSE,  /* indicates that we'll display the message */
           log_msg     = FALSE;  /* indicates that if we're logging then
                                    we'll log the message */
   va_list arg;

   extern int inu_doing_logging;

   logging = inu_doing_logging;

   va_start (arg, format);

   switch (output_indicator) 
   {
      case WARN_MSG:
      case INFO_MSG:
      case SUCC_MSG:
         log_msg = TRUE;
         if (V_FLAG > V0)
            display_msg = TRUE;
         break;

      case SCREEN_MSG:
         display_msg = TRUE;
         break;

      case LOG_MSG:
         log_msg = TRUE;
         break;

      case FAIL_MSG:
      case SCREEN_LOG:
         log_msg = TRUE;
         display_msg = TRUE;
         break;

      case PROG_MSG:
         if (V_FLAG > V0)
            display_msg = TRUE;
         break;

     /*------------------------------------------------------------*
      * For the following set of cases, output the message to the 
      * screen and the log if the verbosity level is greater than
      * or equal to the output_indicator.
      *------------------------------------------------------------*/
      case V0:
      case V1:
      case V2:
      case V3:
      case V4:
         if (output_indicator <= V_FLAG)
         {
            log_msg = TRUE;
            display_msg = TRUE;
         }
         break;

      case DEV_NULL:
         break;

      default:
         fprintf (stderr, "instl_msg:  (Internal error) The first argument \
to instl_msg (%d) was invalid.\n");
         fprintf (stderr, "            The following message was the \
offender.\n");
         log_msg = TRUE;
         display_msg = TRUE;
         break;
   }

   if (log_msg)
      if (logging || ! display_msg) 
      {
         vfprintf (stdout, format, arg);
         va_end (arg);
         va_start (arg, format);
      }

   if (display_msg)
      vfprintf (stderr, format, arg);

   va_end (arg);
}
