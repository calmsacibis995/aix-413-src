static char sccsid[] = "@(#)42  1.14  src/bos/usr/sbin/install/inusave/save_regfile.c, cmdinstl, bos41J, 9523C_all 6/9/95 18:49:44";

/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS:           save_regfile, write_reg_stanza_rec
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
 * NAME   - save_regfile.c
 *
 * FUNCTION -           Saves ONE regular (non-archive) file as "update.n"
 *              under the save directory specified by save_dir.  The update.n
 *              name, the original full path name of the file, and the
 *              corresponding vpd information is saved in stanza file format
 *              to the file specified by UPDATE_LIST.
 *
 * NOTES  -             It should be emphasized that this routine only
 *              saves ONE regular file and then returns to the caller.
 *              The UPDATE_LIST file is opened and closed by the caller.
 *
 * RETURNS -  INUGOOD           -- Successful save operation
 *            INUBADSC  -- Failed making/changing to save dir
 *            INUBADC2  -- copy of file failed in inusave
 *            *INUSYSFL -- a system call failed
 *
 ------------------------------------------------------------------------ */


/* ---------------------------------------------------------------------
                       I  N  C  L  U  D  E  S
   --------------------------------------------------------------------- */

#include <locale.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <inusave.h>
#include <sys/vnode.h>

 /* Global */

struct stat stat_buf;      /* used for stat calls                      */
extern int inu31or32;      /* indicates 3.1 or 3.2 calling environment */

/* ----------------------------------------------------------------------
     F  U  N  C  T  I  O  N        D  E  F  I  N  I  T  I  O  N  S
   ---------------------------------------------------------------------- */

int save_regfile (char * apply_file, /* file from apply list being processed */
                  char * save_dir,   /* directory files are saved to */
                  FILE * upl_fp,     /* File pointer to UPDATE_LIST file */
                  int  * counter)    /* is the "n" part of the "update.n"
                                      * filename */
{

 char   basename [PATH_MAX+1]; /* the "update.n" name                        */
 char   tmpstr   [PATH_MAX+1]; /* a temp string                              */
 int    rc;                    /* return code from various function calls    */
 int    written=FALSE;         /* to determine if need to inu_cp (for 3.2)   */


 /**
  **  Check if apply_file exists and is a regular file.
  **/

 rc = stat (apply_file, &stat_buf);

 if (rc != 0  ||  ! S_ISREG (stat_buf.st_mode)) /* not exist or not regular */
     return INUGOOD;

 /**
  **  Check if apply_file is a symbolic link.  If it is
  **  return success -- INUGOOD.
  **/

 if ((rc = readlink (apply_file, tmpstr, PATH_MAX+1)) != -1)
     return INUGOOD;

 sprintf (tmpstr, "update.%d", *counter);       /* create saved file basename */
 sprintf (basename, "%s/%s", save_dir, tmpstr); /* full path of saved file */

 rc = inu_cp (apply_file, basename);       /* cp to savedir */
 if (rc != INUGOOD)                        /* inu_cp was NOT successful  */
   {
       /* inu_cp removes the partially copied file on failure */
      inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CPFAIL_E, CMN_CPFAIL_D), 
                       INU_INUSAVE_CMD, apply_file, basename);
      return (INUBADC2);                    /* Failed copying file */
   }

   /* ---------------------------------------------------------------
    * written flag won't be used anymore since we would have already
    * copied the file to the savedir. 
    * ---------------------------------------------------------------*/
 rc = write_reg_stanza_rec (basename, apply_file, upl_fp, counter, &written);

/**
 **  If we did not successfully write out a stanza record to the
 **  update.list file return the error code.
 **/

 if (rc != INUGOOD)
   {
      inu_rm (basename); /* If we could not write the stanza record, remove */
                         /*   the saved (update.n) file from the savedir */
      return rc;         /* saves basename, apply_file name and VPD stuff */
   }

 return (INUGOOD);

} /* end save_regfile */


/* -----------------------------------------------------------------------
**
** FUNCTION:            write_reg_stanza_rec
**
** DESCRIPTION:
**                      Writes out a single stanza file record which contains:
**  the basename ("update.n" saved file), the original name (apply_file), 
**  all the VPD info that corresponds to the original name, and the option
**  name associated with the apply_file.   All of this info is written out
**  in stanza file format to the file UPDATE_LIST in the save directory --
**  which should be the current dir.
**
** PARAMETERS:
**      char *basename    --  the save name of the file ("update.n")
**      char *apply_file  --  full path original name of file
**      FILE *upl_fp      --  FILE pointer to the UPDATE_LIST control file
**      int  counter      --  is the "n" part of the "update.n" filename
**
** RETURNS:
**      INUGOOD  - if succeeds
**      INUVPDFL - if failed getting vpd info
**
** ----------------------------------------------------------------------- */

int write_reg_stanza_rec (
        char * basename,    /* the save name of the file ("update.n")       */
        char * apply_file,  /* full path original name of file              */
        FILE * upl_fp,      /* FILE pointer to the UPDATE_LIST control file */
        int  * counter,     /* is the "n" part of the "update.n" filename   */
        int  * written)     /* boolean -- TRUE ==> stanza rec written out   */
{

 inv_t inv;                               /* for VPD queries           */
 int   rc = 0;                              /* return code               */
 char  option [MAX_LPP_FULLNAME_LEN];     /* option name of apply_file */

 inv.loc1 = "";   /* Make these guys point to somewhere */
 inv.loc2 = "";


/***********************************************************
 ** if this inventory entry exists in the VPD, then save it
 ***********************************************************/

 strcpy (inv.loc0, apply_file); /* search for apply_file in VPD */

 if ((vpdget (INVENTORY_TABLE, INV_LOC0, &inv)) == VPD_OK)
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
           fprintf (upl_fp, "\n%s:", apply_file);
           fprintf (upl_fp, "\n        update.n = update.%d",    *counter);
           fprintf (upl_fp, "\n");    /* newline delimiter between records */

           (* counter)++;             /* increment counter for next file */
           return (INUGOOD);
        } /* end if */

       /**
	** If we can't find a lpp_id that matches, just act like it 
        ** doesn't exist at all.
        **/
        
        return (INUGOOD);

    } /* end if */

   /*********************************************************************
    ** Each stanza record consists of 3 things:
    ** (rec name)   1.) full path original name of saved file (apply_file)
    ** (attribute)  2.) name of saved file (update.n)
    ** (attributes) 3.) all corresponding vpd info (specified in inv
    **                  structure)
    *********************************************************************/

    fprintf (upl_fp, "\n%s:", apply_file);/* apply_file = name of stanza rec */

          /* 8 spaces  >12345678< needed here */

    fprintf (upl_fp, "\n        update.n = update.%d",    *counter);

   /**
    **  Do not save the option if called in a 3.1 environment -- cuz
    **  we do not offer the ability to later reject on a per-option
    **  basis.
    **/

    if (inu31or32 != 1)
        fprintf (upl_fp, "\n        option = %s",                 option);

    fprintf (upl_fp, "\n        %s = %ld",   "_id",       inv._id);
    fprintf (upl_fp, "\n        %s = %ld",   "_reserved", inv._reserved);
    fprintf (upl_fp, "\n        %s = %ld",   "_scratch",  inv._scratch);
    fprintf (upl_fp, "\n        %s = %d",    "lpp_id",    inv.lpp_id);
    fprintf (upl_fp, "\n        %s = %d",    "private",   inv.private);
    fprintf (upl_fp, "\n        %s = %d",    "file_type", inv.file_type);
    fprintf (upl_fp, "\n        %s = %d",    "format",    inv.format);

    if ((inv.loc0) [0] != '\0')
        fprintf (upl_fp, "\n        %s = %s",     "loc0", inv.loc0);
    if (inv.loc1 != NIL (char)  &&  *(inv.loc1) != '\0')
        fprintf (upl_fp, "\n        %s = \"%s\"", "loc1", inv.loc1);
    if (inv.loc2 != NIL (char)  &&  *(inv.loc2) != '\0')
        fprintf (upl_fp, "\n        %s = \"%s\"", "loc2", inv.loc2);

    fprintf (upl_fp, "\n        %s = %ld",   "size",      inv.size);
    fprintf (upl_fp, "\n        %s = %ld",   "checksum",  inv.checksum);
    fprintf (upl_fp, "\n");   /* need a newline delimiter between records */

    /********************
     ** delete VPD entry
     ********************/

    if (vpddel (INVENTORY_TABLE, INV_LOC0, &inv) != VPD_OK)
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUSAVE, SAV_DELVPD_W, SAV_DELVPD_D)); /* warning */

    (* counter)++;                    /* increment counter for next file */
    * written = TRUE;

    vpd_free_vchars (INVENTORY_TABLE, &inv);
 }
 else if (inu31or32 == 1)
 {
    /**
     **  For backwards compatibility with 3.1, we need to save a stanza record
     **  in the update.list file so that inurecv can later recover
     **  this file.
     **/

     fprintf (upl_fp, "\n%s:", apply_file);
     fprintf (upl_fp, "\n        update.n = update.%d",    *counter);
     fprintf (upl_fp, "\n");    /* newline delimiter between records */

     ++*counter;                    /* increment counter for next file */

     return INUGOOD;
 }
 else if (stat_buf.st_nlink > 1)
    return INUGOOD;    /* with no message printed */
 else
 {
    /**
     **  If we get here, then the apply list file (apply_file) was
     ** a regular file (not a dir, not a link) that just did not have
     ** a vpd entry.
     **
     **  Since we can't distinguish between an apply only file and
     ** a file that has a missing vpd entry in the inventory table, 
     ** we'll continue for now.
     **/

   /*
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUSAVE, SAV_VPDNF_E, SAV_VPDNF_D), 
                          apply_file);
    */
 }
 return (INUGOOD);

} /* end write_stanza_rec */
