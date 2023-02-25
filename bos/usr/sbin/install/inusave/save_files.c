static char sccsid[] = "@(#)41  1.8  src/bos/usr/sbin/install/inusave/save_files.c, cmdinstl, bos411, 9428A410j 3/6/94 19:33:27";
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
 * NAME - save_files.c
 *
 * FUNCTION -		Processes entire apply list and saves each filename
 *		contained in it.  It also determines if a given filename is
 *		an archive file or a regular file.
 *		
 * NOTES -		Saves each file specified in the apply list one at
 * 		a time by calling the appropriate routine -- either the
 *		save_regfile routine or the save_arfile routine.
 *
 * RETURNS -	INUGOOD		-- Successful save operation
 *		INUBADSC	-- failed making save directory
 *		INUBADC2	-- copy of file failed in inusave
 *		INUTOOFW	-- too few parameters
 *		INUTOOMN	-- too many parameters
 * 		INUNOAP1	-- apply list not readable
 *		*INUNOWRPR	-- no write permissions
 ------------------------------------------------------------------------ */



/* ---------------------------------------------------------------------
                       I  N  C  L  U  D  E  S 
   --------------------------------------------------------------------- */

#include <stdio.h>
#include <locale.h>
#include <sys/limits.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <inusave.h>


/* GLOBALS */
int reg_ctr=1;  /* regular counter -- "n" part of "update.n"  saved files */
int ar_ctr=1;   /* archive counter -- "n" part of "archive.n" saved files */


/*------------------------------------------------------------------------
        F  U  N  C  T  I  O  N       D  E  F  I  N  I  T  I  O  N  S
  ----------------------------------------------------------------------- */

int save_files
 (
 char *apply_list,     /*  the sorted apply list file in "/tmp"         */
 char *arc_control,    /*  full pathname of the archive control file    */
 char *save_dir,       /*  full path name of save directory             */
 char *tmpdir	       /* temp dir used for sorted apply_list           */
)

{

 FILE *acf_fp;	        /* File pointer to archive control file     */
 FILE *apl_fp;	        /* File pointer to apply list file          */
 FILE *upl_fp;	        /* File pointer to UPDATE_LIST file         */
 FILE *arl_fp;	        /* File pointer to ARCHIVE_LIST file        */
 int  rc=INUGOOD;	/* return code from various function calls  */
 struct stat stat_buf;		/* arc_control file exists and is nonempty    */
 char apply_file [PATH_MAX+1];  /* current file in apply_list being processed */
 char cmd        [PATH_MAX+1]; 	/* Used for system calls   */
 char sorted_acf [PATH_MAX+1];	/* sorted archive control file in /tmp */



 /********************************************************************
  ** shouldn't fail changing to save_dir, even if we're already there
  ********************************************************************/
 
 
 if (chdir (save_dir) == -1)                                   /* then failed */
 {
        /* failed changing to the save dir */
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                          CMN_CANT_CHDIR_D), INU_INUSAVE_CMD, save_dir);
     return (INUBADSC);
 }

 if ((apl_fp = fopen (apply_list, "r")) == NIL (FILE))       /* then failed */
 {
          /* apply list is not readable */
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                CMN_CANT_OPEN_D), INU_INUSAVE_CMD, apply_list); 
     return (INUNOAP1);
 }


 /**
  **  Check if UPDATE_LIST exists.  
  **  If it does exist, 
  **		fseek to the end of it.
  **  If it does not exist, 
  ** 		create it.
  **/

 if (access (UPDATE_LIST, F_OK) != -1)   
 {
     if ((upl_fp = fopen (UPDATE_LIST, "r+")) == NIL (FILE))
     {
         inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                              CMN_CANT_CREATE_D), INU_INUSAVE_CMD, UPDATE_LIST);
         return (INUNOWRPR);
     }
    fseek (upl_fp, 0L, SEEK_END);		/* seek to end of file */
 }

 else if ((upl_fp = fopen (UPDATE_LIST, "w")) == NIL (FILE))     /* create it */
 { 
        /* No write permissions to save dir */
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                          CMN_CANT_CREATE_D), INU_INUSAVE_CMD, UPDATE_LIST);
     return (INUNOWRPR);  
 }


/***********************************************************************
 **  There are two cases here: 
 **	CASE 1:  We need to save only regular (non-archive) files.
 **		 -- We know that this is the case if the archive control
 **		    file either does not exist or is empty.
 **
 **	CASE 2:  We need to save both regular files and archive files
 **		 -- We know that this is the case if the archive control
 **		    file both exists and is nonempty.
 ***********************************************************************/


 /******************************************************************
  **  Initialize reg_ctr to the lowest number possible.  The lowest
  ** number possible is the largest ".n" (from the update.n files
  ** in the save_dir) currently being used plus 1.
  ******************************************************************/

 if (init_ctr (save_dir, "update", &reg_ctr) == -1)   /* then failed */
 {
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D), 
                      INU_INUSAVE_CMD, "scandir");
     return (INUSYSFL);
 }

 rc = stat (arc_control, &stat_buf);                 /* get file info for acf */


/**********
 ** CASE 1
 **********/

 if (rc == -1  ||  stat_buf.st_size == 0)            /* then we have CASE 1 */
 {
     rc = INUGOOD;

     while (rc == INUGOOD  &&  (fscanf (apl_fp, "%s", apply_file)) != EOF)
     {
         strip (apply_file);    /* remove leading period from apply_file name */
         rc = save_regfile (apply_file, save_dir, upl_fp, &reg_ctr);
     }
 }  /* of if */


/***********
 ** CASE 2
 ***********/

 else           /* we have CASE 2:    so, save archive AND non-archive files  */
 {
   /************************************************************************
    ** sort the archive control file by the SECOND field (the archive name)
    ************************************************************************/

     strcpy (sorted_acf, tmpdir);    /* temp dir part of (full path) acf name */
     strcat (sorted_acf, "/");
     strcat (sorted_acf, SORTED_ACF);

     rc = inu_mk_mstr_acf (arc_control, sorted_acf);             /* sort acf */

     if (MASK (rc))                      /* then the sort failed in some way */
     {
         return (INUSYSFL);     /* return INUSYSFL or rc from sort ??? */
     }

     acf_fp = fopen (sorted_acf, "r");   /* Just created this, so should work */


    /**
     **  Check if ARCHIVE_LIST exists.  
     **  If it does exist, 
     **		fseek to the end of it.
     **  If it does not exist, 
     ** 		create it.
     **/

    if (access (ARCHIVE_LIST, F_OK) != -1)   
    {
        if ((arl_fp = fopen (ARCHIVE_LIST, "r+")) == NIL (FILE))
        {
            inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                             CMN_CANT_CREATE_D), INU_INUSAVE_CMD, ARCHIVE_LIST);
            return (INUNOWRPR);
        }
        fseek (arl_fp, 0L, SEEK_END);
    }

    else if ((arl_fp = fopen (ARCHIVE_LIST, "w")) == NIL (FILE)) /* create it */
    { 
        /* No write permissions to save dir */
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                             CMN_CANT_CREATE_D), INU_INUSAVE_CMD, ARCHIVE_LIST);
        return (INUNOWRPR);  
    }

        rc = save_both_types (save_dir, arl_fp, upl_fp, acf_fp, apl_fp);

    fclose (acf_fp);
    fclose (arl_fp);
 } /* of else */

 fclose (apl_fp);
 fclose (upl_fp);
 
 return rc;
} /* of save_files */


/* -----------------------------------------------------------------------
**
** FUNCTION: 		save_both_types 
**
** DESCRIPTION:		Saves both regular files and archive files as opposed   
**				to just regular files.
**
** PARAMETERS:
**		char save_dir [PATH_MAX+1] --  full pathname of the save directory 
**		FILE *arl_fp  --   File pointer to "Archive.List" file
**		FILE *upl_fp  --   File pointer to "Update.List" file    
**		FILE *acf_fp  --   File pointer to archive control file 
**		FILE *apl_fp  --   File pointer to apply list file
**
** RETURNS:
**        INUSYSFL - system call failed  (save unsuccessful)
**        rc 	   - from save_regfile and save_arfile (save successful)
**
** ----------------------------------------------------------------------- */
int save_both_types 
 (
  char save_dir [PATH_MAX+1],  /* full pathname of the save directory         */
  FILE *arl_fp,                /* File pointer to "Archive.List" file         */
  FILE *upl_fp,                /* File pointer to "Update.List" file          */
  FILE *acf_fp,                /* File pointer to archive control file        */
  FILE *apl_fp                 /* File pointer to apply list file             */
)

{

  char apply_file [PATH_MAX+1]; /* current file in apply_list being processed */
  char arc_name   [PATH_MAX+1]; /* archive file name     (.a file)            */
  char const_name [PATH_MAX+1]; /* constituent file name (.o file)            */
  int rc=INUGOOD;               /* return code from various functions         */
  int match=0;       	   /* 1 if apply_file matches const_name, 0 otherwise */


 /******************************************************************
  **  Initialize ar_ctr to the lowest number possible.  The lowest
  ** number possible is the largest ".n" (from the archive.n files
  ** in the save_dir) currently being used PLUS 1.
  ******************************************************************/


 if (init_ctr (save_dir, "archive", &ar_ctr) == -1)   /* then failed */
 {
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D), 
                      INU_INUSAVE_CMD, "scandir");
     return (INUSYSFL);
 }


 /************************************************************************
  **  The outer while loop processes the apply list--one file at a time
  ***********************************************************************/

  while (rc == INUGOOD  &&  (fscanf (apl_fp, "%s", apply_file)) != EOF)
  {            

    /*************************************************************************
    **  The inner while loop searches through the entire archive control file 
    ** for the apply file obtained from the outer loop.  The inner while loop 
    ** continues processing until either a match (between const_name and
    ** apply_file) has been found or until all of the acf has been read.
    *************************************************************************/ 

       while ( ! match  &&  
              (fscanf (acf_fp, "%s %s", const_name, arc_name)) != EOF)
       {
           strip (const_name);       /* remove leading period from const_name */
           strip (apply_file);       /* remove leading period from apply_file */

           if (strcmp (const_name, apply_file) == 0)  /* apply_file is in acf */
           {
               match = 1;                /* indicates a match has been found */
               strip (arc_name);
               rc = save_arfile (const_name, arc_name, apply_file, 
                                 save_dir, arl_fp, &ar_ctr); 
           }
       } /* of while ( ! match) */

       if ( ! match)               /* then apply_file must be a REGULAR file */
           rc = save_regfile (apply_file, save_dir, upl_fp, &reg_ctr);
       else
           match = 0;                  /* reset match for next pass thru loop */

       fseek (acf_fp, 0l, SEEK_SET) ;
        /* better to seek than to open and close file each pass thru the loop */
  } /* of while (rc == INUGOOD) */

  return rc;
} /* of save_both_types */
