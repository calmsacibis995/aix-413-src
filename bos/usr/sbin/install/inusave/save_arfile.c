static char sccsid[] = "@(#)40  1.19  src/bos/usr/sbin/install/inusave/save_arfile.c, cmdinstl, bos41J, 9523C_all 6/9/95 18:49:29";

/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS:           save_arfile, write_ar_stanza_rec
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

#include <locale.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <inusave.h>


/*------------------------------------------------------------------------
 *
 * NAME - save_arfile.c
 *
 * FUNCTION -           Saves ONE archive file to the save directory
 *              under the name "archive.n".  Also adds an entry in the
 *              ARCHIVE_LIST file, which is basically a stanza file
 *              entry.  The name of each record is the name of the
 *              original file.  The attributes of each record consist
 *              of the corresponding VPD information.
 *
 * NOTES  -             It should be stressed that this routine saves
 *              only ONE archive file.  The "archive.list" file  is
 *              opened and closed by the caller.
 *
 * RETURNS -    INUGOOD         -- Successful save operation
 *              INUBADSC        -- failed making save directory
 *              INUBADC2        -- copy of file failed in inusave
 *              INUTOOFW        -- too few parameters
 *              INUTOOMN        -- too many parameters
 *              INUNOQAP1       -- apply list not readable
 *              INUSYSCL        -- system call failed
 ------------------------------------------------------------------------ */


extern int inu31or32;      /* indicates 3.1 or 3.2 calling environment */

/* ----------------------------------------------------------------------
         F  U  N  C  T  I  O  N        D  E  F  I  N  I  T  I  O  N  S
   ---------------------------------------------------------------------- */

int save_arfile
 (
  char *const_name,            /* constituent file name (.o file)             */
  char *arc_name,              /* name of  archive in acf                     */
  char *apply_file,                /* used for VPD query                          */
  char *save_dir,              /* full path name of save directory            */
  FILE *arl_fp,                /* File pointer to ARCHIVE_LIST file           */
  int  *counter                /* is the "n" part of the "archive.n" filename */
)

{

  char cmd        [PATH_MAX+1]; /* command given to shell for archive         */
  char *member;                 /* member name extracted from const_name      */
  char basename   [PATH_MAX+1]; /* saved file's name in save_dir--"archive.n" */
  int rc;                       /* return code from various functions         */
  int written=FALSE;     /* boolean - TRUE ==> archive.list entry was written */



 /*********************************************************
  ** unarchive member from arc_name by using a system call
  *********************************************************/

  member = get_member (const_name);          /* everything right of last "/" */


 /******************************************************************
  **  Construct the archive command to extract member from arc_name
  ******************************************************************/

  sprintf (cmd, "/usr/ccs/bin/ar x %s %s 2> /dev/null", arc_name, member);

  rc = system (cmd);                         /* extract member from arc_name  */

  if (MASK (rc))                             /* then system call failed . . . */
  {

     /*
     **  If the name of the save_dir contains the substring ".save" -- as
     **  in "/usr/lpp/<lppname>/inst_<ptf>.save" -- then we are saving
     **  files for an update.  Because updates can add archive member files
     **  to existing libraries, we can't give an error just because the
     **  member does not already exist.  Just return INUGOOD in this case.
     */
      if (strstr (save_dir, ".save"))
        {
          if ((access (member, F_OK)) == 0) /* Partially unarchived */
             inu_rm (member);  /* it is a failure */
          else
             return (INUGOOD);
        }

      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D), 
                       INU_INUSAVE_CMD, cmd);
      return (INUSYSFL);
  }


/*************************************************************
 **  If the member is accessable, then save the corresponding
 ** VPD information out to the ARCHIVE_LIST stanza file.
 *************************************************************/


  if ((access (member, F_OK)) == 0)        /* if member exists, then save it */
  {
      sprintf (basename, "archive.%d", *counter);     /* construct saved name */
      rc = inu_mv (member, basename);         /* move member to basename */
      if (rc != INUGOOD)    /* mv was NOT successful */
         {
           inu_rm (member);   /* could have invoked inu_rm once */
                              /* this approach does not consider how the */   
                              /* file is removed in inu_rm */ 
           inu_rm (basename);
           inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_BADMV_E, CMN_BADMV_D), 
                            INU_INUSAVE_CMD, member, basename);
           return (INUBADC2);        /* the copy (mv in this case) failed */
         }

      /* written flag is not going to be used */
      rc = write_ar_stanza_rec (basename, const_name, arc_name, arl_fp, 
                                apply_file, counter, &written);
     /**
      **  If we did not successfully write out a stanza record to the
      **  archive.list file return the error code.
      **/

      if (rc != INUGOOD)
        {
          inu_rm (member);    /* remove both the hummers */
          inu_rm (basename);
          return rc;
        }

  } /* of if (access) */

  return INUGOOD;
}


/* -----------------------------------------------------------------------
**
** FUNCTION:            write_ar_stanza_rec
**
** DESCRIPTION:
**              Writes out the basename ("archive.n" name), the original
**    file name (apply_file), all the VPD info that corresponds to the
**    original name, and which option the file is associated with.  All
**    of this information is written out in stanza file format to the
**    file ARCHIVE_LIST.
**
** PARAMETERS:
**              char *basename   --  the save name of the file ("archive.n")
**              char *const_name --  the constituent file name (the .o file)
**              char *arc_name   --  the archive name          (the lib file)
**              FILE *arl_fp     --  FILE pointer to the "archive.list"
**                                   control file
**              char *apply_file --  current filename from apply list being
**                                   processed
**              int  counter     --  is the "n" part of the "archive.n"
**                                   filename
**
** RETURNS:     INUGOOD      --   if successful
**              INUVPDFL     --   failed getting vpd info
**
** ----------------------------------------------------------------------- */

int write_ar_stanza_rec
 (
 char *basename,    /* the save name of the file ("archive.n"             */
 char *const_name,  /* the constituent file name (the .o file)            */
 char *arc_name,    /* the archive name          (the lib file)           */
 FILE *arl_fp,      /* FILE pointer to the "archive.list" control file    */
 char *apply_file,  /* current filename from apply list being processed   */
 int  *counter,     /* is the "n" part of the "archive.n" filename        */
 int  *written /* boolean-TRUE ==> archive.list entry successfully written */
)


{

 inv_t inv;         /* for VPD queries */

 char  option [MAX_LPP_FULLNAME_LEN];   /* option name of apply_file    */


/***********************************************************
 ** if this inventory entry exists in the VPD, then save it
 ***********************************************************/

 strcpy (inv.loc0, arc_name);


 if ((vpdget (INVENTORY_TABLE, INV_LOC0, &inv)) == VPD_OK)       /* not found */
 {
     if ((vpdreslpp_id (inv.lpp_id, option)) != VPD_OK)
     {
       /**
        ** If we were called in a 3.1 environment, then we cannot fail
        ** because of the apply_file not having an entry in the swvpd.  So, 
        ** we want to save a "stub" entry in the update.list -- which
        ** contains the bare minimum info to allow inurecv to recover the
        ** apply_file.
        **/

        if (inu31or32 == 1)
        {
            fprintf (arl_fp, "\n%s:", apply_file);
            fprintf (arl_fp, "\n        archive.n = archive.%d", *counter);
            fprintf (arl_fp, "\n        arc_name = %s", arc_name);
            fprintf (arl_fp, "\n");    /* newline delimiter between records */

            ++*counter;

            return INUGOOD;
        }

       /**
	** If we can't find a lpp_id that matches, just act like it 
        ** doesn't exist at all.
        **/
        
        return INUGOOD;
     }


     fprintf (arl_fp, "\n%s:", apply_file);     /* write out the record name */

           /* 8 spaces  >12345678< needed here */
     fprintf (arl_fp, "\n        archive.n = archive.%d", *counter);
     fprintf (arl_fp, "\n        arc_name = %s", arc_name);

    /**
     **  Do not save the option if called in a 3.1 environment -- cuz
     **  we do not offer the ability to later reject on a per-option
     **  basis.
     **/

     if (inu31or32 != 1)
         fprintf (arl_fp, "\n        option = %s",                 option);

     fprintf (arl_fp, "\n        %s = %ld", "_id",       inv._id);
     fprintf (arl_fp, "\n        %s = %ld", "_reserved", inv._reserved);
     fprintf (arl_fp, "\n        %s = %ld", "_scratch",  inv._scratch);
     fprintf (arl_fp, "\n        %s = %d",  "lpp_id",    inv.lpp_id);
     fprintf (arl_fp, "\n        %s = %d",  "private",   inv.private);

     if ((inv.loc0) [0] != '\0')
         fprintf (arl_fp, "\n        %s = %s",  "loc0",  inv.loc0);
     if (inv.loc1 != NIL (char)  &&  *(inv.loc1) != '\0')
         fprintf (arl_fp, "\n        %s = \"%s\"",  "loc1",  inv.loc1);
     if (inv.loc2 != NIL (char)  &&  *(inv.loc2) != '\0')
         fprintf (arl_fp, "\n        %s = \"%s\"",  "loc2",  inv.loc2);

     fprintf (arl_fp, "\n");     /* need a newline delimiter between records */


     /***********************
     ** Delete the VPD entry
     ************************/

/*
     if (vpddel (INVENTORY_TABLE, INV_LOC0, &inv) != VPD_OK)
         inu_msg (SCREEN_LOG, MSGSTR (MS_INUSAVE, SAV_DELVPD_W, SAV_DELVPD_D));
*/
     ++*counter;
     *written = TRUE;

     vpd_free_vchars (INVENTORY_TABLE, &inv);
 }

 else if (inu31or32 == 1)
 {
    /**
     **  If we were called in a 3.1 environment, then we save a stanza
     **  record in the archive.list file -- regardless of whether the
     **  apply_file has an entry in the swvpd or not.
     **/

     fprintf (arl_fp, "\n%s:", apply_file);
     fprintf (arl_fp, "\n        archive.n = archive.%d", *counter);
     fprintf (arl_fp, "\n        arc_name = %s", arc_name);
     fprintf (arl_fp, "\n");    /* newline delimiter between records */

     ++*counter;
 }

 return INUGOOD;

} /* end write_stanza_rec */
