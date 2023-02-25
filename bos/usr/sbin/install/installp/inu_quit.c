static char sccsid[] = "@(#)00  1.21  src/bos/usr/sbin/install/installp/inu_quit.c, cmdinstl, bos411, 9428A410j 6/8/94 16:08:23";

/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_quit (installp)
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
#include <installp.h>
#include <instl_options.h>
#include <inu_dev_ctl.h>
#include <sys/types.h>
#include <time.h>

extern int cmd_line_prod_fail;  /* Global variable to indicate failure to
				   install products or updates input from
				   the command line */
/*
 * NAME: inu_quit
 *
 * FUNCTION:
 *	Exit with exit code after doing some housework.
 *	Remove all files that have been created to facilitate
 *	the install.  Close the VPD.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS: None.
 */


void inu_quit (
int	code)				/* exit code			*/
{
   time_t tp;   /* for END time */
   char   timestamp[INU_TIME_SZ], 
          *ptr;
   extern int inu_doing_logging;


	if ( ! NO_CLEANUP) {
		 (void) chdir ("/");
		 (void) inu_rm (INU_TMPDIR);
		/**
		 (void) rmdir (INU_TMPDIR);
		 **/
	}

   /*-----------------------------------------------------------------------*
	* Unlock the flock for each tree that we were working with.
    *-----------------------------------------------------------------------*/
	if (USR_PART)
		 (void) inu_unsetlock (INU_LOCK_ID_U);
	
	if (ROOT_PART)
		 (void) inu_unsetlock (INU_LOCK_ID_M);

	if (SHARE_PART)
		 (void) inu_unsetlock (INU_LOCK_ID_S);

   /*-----------------------------------------------------------------------*
	* close the vpd.
    *-----------------------------------------------------------------------*/
	 (void) vpdterm ();


   /**
    **   If the device that we're dealing with is a tape device, 
    **  then we need to set the block size of the tape device back
    **  to what it originally was.
    **/

    if (TocPtr->type == TYPE_TAPE_SKD  ||  TocPtr->type == TYPE_TAPE_BFF)
    {
       lock_tape_restore();
       if (DEVICE [0] != '\0') 
            tchg_blksize (DEVICE, NULL); 
       delete_tape_lock();
    }

    if (inu_doing_logging == TRUE) { /* if logging is on */
        if (time (&tp) == -1)
            instl_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_FUNCCALL_E, 
                                 CMN_ERR_FUNCCALL_D), inu_cmdname, "time");
        else
            instl_msg (LOG_MSG, "\nEND:%s:%s\n", 
                     ((ptr=ctime(&tp)) == NIL(char)) ? "" : strtok (ptr, "\n"),
                     (strftime (timestamp, INU_TIME_SZ-1, INU_TIME_FORMAT, 
                     gmtime (&tp)) == 0) ? "" : timestamp);
     }



  exit (code);
}
