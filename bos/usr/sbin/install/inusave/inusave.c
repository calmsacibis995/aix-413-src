static char sccsid[] = "@(#)22	1.31  src/bos/usr/sbin/install/inusave/inusave.c, cmdinstl, bos411, 9428A410j 3/6/94 19:33:19";


/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: 
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
                                                                  

/*------------------------------------------------------------------------
 *
 * NAME   - inusave.c
 *
 * FUNCTION -		Saves files--specified in the apply list--which 
 *		change during install/update into one of three possible
 *		filesystems: root, /usr, and /usr/share.  This file is the
 *		main driver for the process.
 *
 * NOTES  - 		Saves all files listed in the apply list, which 
 *		is specified by the first parameter, to the save directory
 *		specified by the environment variable INUSAVEDIR.  Every 
 *		regular file saved is renamed to "update.n", where n is a
 *		unique number.  Every file that is part of an archive is 
 *		renamed to "archive.n".  
 *			A mapping between the original file names and the 
 *		names under which they are saved is contained in UPDATE_LIST 
 *		for regular files, and in ARCHIVE_LIST for archive files.
 *
 * COMMAND LINE ARGUMENTS -	argv [0] = program name
 *				argv [1] = full path name of apply list
 *				argv [2] = lppname
 *
 * ENVIRONMENT VARS USED  -   
 *                          INUSAVEDIR  - specifies the full path of
 *                                         the directory to save files to
 *
 *                          INUTEMPDIR   - specifies temp dir to use for  
 *                                         sorting acf and apply list.
 *
 *                          INUSAVE      - is '1' if -N flag is set
 *                                         is '0' if -N flag is not set
 *
 * RETURNS -		 (previously existing returns)	
 *		INUGOOD		-- Successful save operation
 *		INUBADSC	-- failed making save directory
 *		INUBADC2	-- copy of file failed in inusave
 *		INUTOOFW	-- too few parameters
 *		INUTOOMN	-- too many parameters
 * 		INUNOAP1	-- apply list not readable
 *
 *	Additional returns: (added this release)
 *      INUSYSFL    -- a system call failed 
 *	INUSRTFL    -- Sort of apply list or of archive control file failed 
 *	INUENVNS    -- Environment var not set 
 *	INUNOWRPR   -- No write permissions to save directory
 *
 ------------------------------------------------------------------------ */



/* ---------------------------------------------------------------------
                       I  N  C  L  U  D  E  S 
   --------------------------------------------------------------------- */

#include <locale.h>
#include <stdio.h>
#include <sys/limits.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <swvpd.h>
#include <string.h>
#include <inuerr.h>
#include <inudef.h>
#include <inu_toc.h>
#include <sys/mode.h>
#include <inusave.h>



	/* Global var externed by eval_size routine, inu_mk_mstrs, ... */
char *inu_cmdname="inusave";
int  inu31or32=1;            /* default to 3.1 calling environment */


main (argc, argv)

int  argc;
char *argv [];

{


  char    apply_list  [PATH_MAX+1];         /* sorted apply list in temp dir */ 
  char    arc_control [PATH_MAX+1]; /* full path to the archive control file */
  char    cmd         [BUFSIZ];  /* command given to shell to invoke inurecv */
  char    tmpstr      [PATH_MAX+1]; /* if used, is full path to the save dir */
  char    tmpstr2     [PATH_MAX+1]; /* used to put an environment variable   */ 
  char    *save_dir;            /* is dir where saved files are located      */
  char    *nflag; 		/* used to determine if -N flag is set       */
  char    *tmpdir;              /* full path to the temp dir                 */
  char    *envar;		/* used to hold results of call to getenv    */
  int     rc;                   /* return code from various function calls   */
  int     rc2;
  int     rc3;
  int     sysrc;                /* return code from system calls             */
  struct  stat stbuf;		/* for call to stat                          */	
  struct  stat stbuf2;

 (void) setlocale (LC_ALL, ""); 

  /*-----------------------------------------------------------------------*
   * Open the message catalog
   *-----------------------------------------------------------------------*/
   CATOPEN ();


/*********************************
 **  do some error checking . . . 
 *********************************/

  if (argc - 1 > SV_NPARAMS)           /* Must have exactly SV_NPARAMS parms  */
  {
	inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_TOO_MN_E, CMN_TOO_MN_D), 
                         INU_INUSAVE_CMD);
	inu_msg (SCREEN_LOG, MSGSTR (MS_INUSAVE, SAV_USAGE_E, SAV_USAGE_D));
  	return (INUTOOMN);
  }

  if (argc - 1 < SV_NPARAMS)            /* Must have exactly SV_NPARAMS parms */
  {
	inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_TOO_FW_E, CMN_TOO_FW_D), 
	                 INU_INUSAVE_CMD);
	inu_msg (SCREEN_LOG, MSGSTR (MS_INUSAVE, SAV_USAGE_E, SAV_USAGE_D));
  	return (INUTOOFW);
  }

  /* if the INUSAVE environment var not set, assume -N flag is NOT set */

  if ((nflag = getenv (INUSAVE)) != NIL (char))      /* then env var is set */
  {
     if ((nflag [0]) == NFLAGSET)     /* then -N flag is set--save nothing */
         return (INUGOOD); 
  }
  else
  {
    /** 
     **  The INUSAVE environment var is not set, so set it and put it
     **  in the environment.  This is for the call to inusave_eval_size.
     **/
     if ((rc = putenv ("INUSAVE=1")) != SUCCESS)
         return (INUUNKWN);
  }


      /* apply list passed from the command line must be a full path filename */

  if (stat (argv[1], &stbuf) == -1)     /* then apply list is NOT readable    */
  {
	inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                   CMN_CANT_OPEN_D), INU_INUSAVE_CMD, argv[1]);
  	return (INUNOAP1);
  }

  else if (S_ISDIR (stbuf.st_mode))
  {
	inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                             CMN_CANT_OPEN_D), INU_INUSAVE_CMD, argv[1]);
  	return (INUNOAP1);
  }


 /********************************************************
  **  Sort the apply list, eliminate duplicate entries
  ** and save the result to the temp dir specified by the
  ** INUTEMPDIR environment variable.  This is the apply
  ** list used by this program from here on out.
  ********************************************************/

  if ((tmpdir = getenv ("INUTEMPDIR")) == NIL (char))
    /* then environment var for temp dir is not set, so . . . */
  	tmpdir = "/tmp";                     /* . . . use default tmp dir */


  strcpy (apply_list, tmpdir);                          /* the directory part */
  strcat (apply_list, "/");                    
  strcat (apply_list, SORTED_AL);                       /* the filename  part */


 /***********************************************
  **  Check if enough space is available to save 
  ** everything specified in the apply list
  ***********************************************/

  if ((rc = inusave_eval_size (argv[1])) != 0)
  {
      if (rc == INUFS)
         return (INUFS);

      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                           CMN_NO_SPACE_D), INU_INUSAVE_CMD);
      return INUSYSFL;
  } 


  if ((save_dir = getenv (INUSAVEDIR)) == NIL (char)) 
             /* then no save dir was specified */
  {
      /**
       ** Create the full dir path to the default save dir:
       ** "/usr/lpp/<lppname>/inst_updt.save"
       **/

      strcpy (tmpstr, DFLT_SAVEDIR);  /* has a trailing "/" */
      strcat (tmpstr, argv [2]);
      strcat (tmpstr, "/");
      strcat (tmpstr, "inst_updt.save");

      rc = stat (tmpstr, &stbuf);

      if (rc == -1)
      {
          /**
	   **  default dir does not exist.  So, create it.
	   **/

          rc = inu_mk_dir (tmpstr);  

          if (rc)           /* nonzero rc ==> failure */
          {
              inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                               CMN_NO_MK_DIR_D), INU_INUSAVE_CMD, tmpstr);
              return (INUBADSC);                       
          }

      }

      else if ( ! (S_ISDIR (stbuf.st_mode)))
      {
          inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                               CMN_CANT_CHDIR_D), INU_INUSAVE_CMD, tmpstr);
          return (INUENVNS);                      /* environment var not set */
      }


     /**
      **  Set and put the environment variable INUSAVEDIR so that the
      **  call to inusave_eval_size will have it.
      **/

      save_dir =  &(tmpstr[0]);
      sprintf (tmpstr2, "INUSAVEDIR=%s", save_dir);

      if ((rc = putenv (tmpstr2)) != SUCCESS)
          return (INUUNKWN);

  } /* of if (save_dir = ...) */


  /*******************************
   **  make the master apply list
   *******************************/

  if ((inu_mk_mstr_al (argv [1], apply_list)) != INUGOOD) /* then sort failed */
  {
	/* Error message will be printed by inu_mk_mstr_al () routine */
  	return (INUSRTFL);                                     /* sort failed */
  }


  /****************************************************
   **  Create the full path to the archive control file
   ** (lpp.acf).  Once we cd to the save_dir, the dir
   ** lpp.acf is in will be lost, unless it's saved.
   ****************************************************/

  
    if ((getcwd (arc_control, (int) sizeof (arc_control))) == NIL (char)) 
    {
        perror (inu_cmdname);
        exit (INUSYSFL);
    }
    else 
        strcat (arc_control, "/lpp.acf"); 	  /* lpp.acf should be in cwd */


/************************************
 **  Check if save directory exists.
 ** If it DOESN'T, then create it.
 ************************************/


  if (access (save_dir, F_OK) == -1)     /* then save_dir does NOT exist ...  */
  {
      rc = inu_mk_dir (save_dir);  /* ... so, create it.  */

      if (rc)        /* then inu_mk_dir failed because nonzero rc ==> failure */
      {
          inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                               CMN_NO_MK_DIR_D), INU_INUSAVE_CMD, save_dir);
          return (INUBADSC);                       
      }
  } 


 /*************************************************************************
  ** Set the VPD path to allow access to the appropriate object repository
  *************************************************************************/

  if ((envar = getenv (ODMDIR)) == NIL (char))
  {
      envar = DFLT_VPD_OR;              /* default object repository         */
      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_ODMDIR_W, CMN_ODMDIR_D), 
                       INU_INUSAVE_CMD);
  }

  vpdremotepath (envar);                /* set path to ODMDIR (env var) */
  vpdremote ();                           /* change path to ODMDIR        */



/*****************************************
 **  Determine if we're called from a 3.1
 ** or a 3.2 environment.  The default is
 ** 3.1 (inu31or32 will be set to 1), so
 ** only set it to 2 if the env var is 2.
 *****************************************/

 if ((envar = getenv ("INU31OR32")) != NIL (char)  && 
     envar [0]  ==  '2' 
   )
    inu31or32 = 2;

/********************************************************************
 ** save ALL files specified in the apply list--regular files as well
 ** as archive files.
 ********************************************************************/

  rc = save_files (apply_list, arc_control, save_dir, tmpdir);


 /**
  **  If -- for some reason -- the update.list file or the archive.list
  **  files are empty, then remove the one (s) that are empty.
  **/

  rc2 = stat (UPDATE_LIST,  &stbuf);
  rc3 = stat (ARCHIVE_LIST, &stbuf2); 

  if (rc2 == 0  &&  stbuf.st_size == 0)
     (void) unlink (UPDATE_LIST);   /* cuz it is empty */

  if (rc3 == 0  &&  stbuf2.st_size == 0)    
     (void) unlink (ARCHIVE_LIST);  /* cuz it is empty */
        

  if (rc == INUGOOD)                        /* then everything was saved okay */
      return (INUGOOD);

  /* close the message catalog */
  CATCLOSE ();

  return (rc);      /* which is some failed condition from call to save_files */
}  
