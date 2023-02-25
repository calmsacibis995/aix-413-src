static char sccsid[] = "@(#)20	1.22  src/bos/usr/sbin/install/inurecv/inurecv.c, cmdinstl, bos411, 9428A410j 3/6/94 19:33:15";
/* COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: main (inurecv)
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

/* -------------------------------------------------------------------------
 * NAME: inurecv
 *
 * FUNCTION: Recover all regular and archived files that were saved by
 *           (a) previous execution (s) of inusave.
 *
 * EXECUTION ENVIRONMENT: User command.
 *
 * USAGE SYNTAX:
 *   inurecv  <lppname>
 *      where:
 *              <lppname> is the name of the LPP in question.
 *
 * ENVIRONMENT VARS USED -
 *        INUSAVE - is '0' if -N flag is NOT set
 *        is '1' if -N flag IS set
 *
 *        INUSAVEDIR - contains a full path of
 *        the save directory.
 *
 *        ODMDIR  - contains the ODM object repos-
 *                  itory to use in the call to
 *                  vpdremotepath.
 *
 * RETURNS:
 *      INUGOOD         files were recovered
 *      INUTOOMN        too many parameters
 *      INUTOOFW        too few parameters
 *      INUBADC1        copy of file failed
 *      INUNOSAV        no save directory (/usr/lpp/LPPNAME/inst_updt.save)
 *      INUNOSVF        no save file as specified in *.list file
 *      INUNORP1        error replacing constituent file in archive
 *
 * Additional returns:
 *      INUSYSFL        a system call failed
 *      INUBADUL    	UPDATE_LIST is bad 
 *      INUBADARL    	ARCHIVE_LIST is bad
 *      INUVPDADD   	failed recovering vpd entry
 *
 * ------------------------------------------------------------------------ */

#define MAIN_PROGRAM

#include <stdio.h>
#include <locale.h>           /* for call setlocale */
#include <string.h>
#include <inulib.h>


            /* Global var externed by inu_msg, eval_size routines */
char *inu_cmdname="inurecv";

#define MIN_RCV_NPARAMS     1
#define MAX_RCV_NPARAMS     2

static void process_config_list (void);


/* -----------------------------------------------------------------------
		M  A  I  N        P  R  O  G  R  A  M
   ----------------------------------------------------------------------- */

main (argc, argv)

int     argc;
char    *argv[];

{

int   rc;
char  *nflag;  	                /* used to see if -N flag was set    */
char  *setenv;	                /* used to get value of env vars     */
char  apply_file  [PATH_MAX+1];	/* current apply file                */
char  optionfile [PATH_MAX+1];	/* full path to the lpp.options file */
FILE  *tmp_fp;	                /* FILE ptr to a temp file           */


  (void) setlocale (LC_ALL, "");

  /*-----------------------------------------------------------------------*
   * Open the message catalog
   *-----------------------------------------------------------------------*/
  CATOPEN ();


  if (argc - 1 < MIN_RCV_NPARAMS)       /* MIN_RCV_NPARAMS should be 1  */
  {
      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_TOO_FW_E, CMN_TOO_FW_D), 
                       INU_INURECV_CMD);
      inu_msg (SCREEN_LOG, MSGSTR (MS_INURECV, RCV_USAGE_E, RCV_USAGE_D));
      exit (INUTOOFW);                /* too few parameters */
  }

  if (argc - 1 > MAX_RCV_NPARAMS)       /* MAX_RCV_NPARAMS should be 2 */
  {
      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_TOO_MN_E, CMN_TOO_MN_D), 
                       INU_INURECV_CMD);
      inu_msg (SCREEN_LOG, MSGSTR (MS_INURECV, RCV_USAGE_E, RCV_USAGE_D)); 
      exit (INUTOOMN);
  }

  if (argc - 1 == 2)
      strcpy (optionfile, argv[2]);

  else
      optionfile[0] = '\0';   /* by default, indicates ALL options specified */


/************************************
 **  Check if -N flag has been set.
 ** If the INUSAVE environment var
 ** isn't set, assume -N is NOT set.
 ************************************/

  if ((nflag = getenv (INUSAVE)) != NIL (char))    /* get environment var   */
      if (nflag [0] == NFLAGSET)               /* then -N flag has been set */
        exit (INUGOOD);                            /* so, recover nothing   */


 /**************************************************************************
  **  Set the VPD path to allow access to the appropriate object repository
  **************************************************************************/

  if ((setenv = getenv (ODMDIR)) == NIL (char))
  {
      setenv = DFLT_VPD_OR;                  /* the default object repository */
      inu_msg (LOG_MSG, MSGSTR (MS_INUCOMMON, CMN_ODMDIR_W, CMN_ODMDIR_D), 
                            inu_cmdname);
  }

  vpdremotepath (setenv);                    /* set path to ODMDIR (env var) */
  vpdremote ();                              /* change path to ODMDIR        */



  /************************************************************************
   **  Process the three files UPDATE_LIST, ARCHIVE_LIST, and config.list, 
   ** if they exist.  In the very least, UPDATE_LIST should exist.  The
   ** two files UPDATE_LIST and ARCHIVE_LIST are processed by the 
   ** inu_rcvr_or_rm routine.
   ************************************************************************/


  process_config_list  ();       /* recovers files listed in config.list  */

  rc = inu_rcvr_or_rm (optionfile, CALLER_INURECV, argv [1]);  /* lpp name */

  /* close the message catalog */
  CATCLOSE ();

  return rc; 
} /* of main */


/* ---------------------------------------------------------------------- */

/******************
 * config.list    
 ******************/

/* ----------------------------------------------------------------------- 
**
** FUNCTION:  		process_config_list
**
** DESCRIPTION:		Basically copies files from the source to the dest-
**    ination as specified in the "config.list" file, if it
**    exists.  THIS ROUTINE IS KEPT IN TACT ALMOST EXACTLY 
**    AS THE 3.1 VERSION.  IT IS HERE FOR BACKWARD COMPATIBILITY
**    ONLY.
**
** PARAMETERS:         none
**
** RETURNS:            none
**
** ----------------------------------------------------------------------- */

void process_config_list ()
{

  char  line  [BUFSIZ];        /* file info buffer             */
  FILE  *fd;                   /* FILE pointer to config.list  */
  char  source[BUFSIZ];
  char  destination[BUFSIZ];


  if ((fd = fopen ("config.list", "r")) != NIL (FILE))
  {
      while (fgets (line, BUFSIZ, fd) != NIL (char))   /* until EOF, get line */
      {
          if (sscanf (line, "%s %s", &source, &destination) != 2)
          {
              inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_BAD_FL_FORMAT_E, 
                             CMN_BAD_FL_FORMAT_D), inu_cmdname, "config.list");
              exit (INUBADC1);   	              /* wrong num of tokens */
          }

         (void) inu_cp (source, destination);
      } /* of while */

      fclose (fd);
  } /* of if */

}  /* of process_config_list */
/* ---------------------------------------------------------------------- */
