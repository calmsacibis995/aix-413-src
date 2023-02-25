static char sccsid[] = "@(#)80  1.6.1.22  src/bos/usr/sbin/install/bffcreate/bff.c, cmdinstl, bos41J, 9516A_all 4/18/95 12:50:06";
/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 *   FUNCTIONS: bff
 *              get_service_num
 *              inu_do_file
 *              inu_do_non_stacked_diskette
 *              inu_do_stacked_diskette
 *              inu_do_tape
 *              inu_get_bff_name
 *              space_message
 *              system_message
 *		is_duplicate_bff_filename
 *		init_filetable
 *		AddNameToHashTable 
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */




#include <bffcreate.h>


/* -----------------------------------------------------------------
                        G L O B A L S
   -----------------------------------------------------------------*/

extern      int current_vol;
extern      int current_off;
static      char *bff_dest;
static      char lppname [PATH_MAX];
static      boolean diskette = FALSE;

extern int         q_flag;             /* Boolean  1 ==> q flag was specified   */
extern int         v_flag;             /*  0 ==> v flag was not specified       */
extern int         X_flag;             /*  0 ==> X flag was not specified       */

char        error_log[PATH_MAX]; /* Temporary file to store output        */
char        print_error_log[PATH_MAX];  /* System command to cat output          */



/* -----------------------------------------------------------------------
**
** FUNCTION:             get_service_num
**
** DESCRIPTION:          Gets the "./service_num" file from the 3.1 image, 
**        which should contain the service number.  If it does, the number is
**        returned.  If it does not, 1 is returned and the calling routine
**        will see if it can use that number.  If it cannot, it will continue
**        calling this routine until it gets a service_num it can use.
**
** PARAMETERS:           bff  -- a node in the toc linked list.
**
** RETURNS:              int  -- the service number for the update image
**
** ----------------------------------------------------------------------- */

int get_service_num
 (
     BffRef_t * bff, 
     char     * dest_dir, 
     int      * sn_exists,   /* set here and returned to caller       */
     int        service_num  /* indicates if we've been called before */
)

{
    int rc;
    char cmd [PATH_MAX+1];   /* To issue a system call            */
    FILE *fp;                /* FILE ptr for the service_num file */


  /**
   **   If the service_num parameter is not equal to zero, 
   ** then this routine has already been called before.  This
   ** call is an attempt to come up with a unique bffname.  This
   ** is to not overwrite an existing 3.1 image (that did not have
   ** the "./service_num" file in it.
   **
   **   In this case, we want to increment the service_num and send
   ** it back.  We will keep doing this until we come up with a
   ** unique number.
   **/

  /**
   **  If this is a diskette, give up on trying to come up with some sort of
   **  wonderful name based on the ./service_num file, just seed with some
   **  non-zero value.
   **/

   if (diskette)
   {
      *sn_exists = FALSE;
      return (service_num++);
   }

   if (service_num != 0)    /* will == 0 if this is 1st time we're called */
     return ++service_num;

  /**
   **  Indicate to the calling routine that
   **  the "./service_num" file does NOT exist.
   **/

   *sn_exists = 0;

   /**
    **  Restore the service_num file from the update image via:
    **
    **  q  -  quiet mode (suppress request for media)
    **  p  -  rewind tape to original position -- so as to not invalidate
    **        the current_off variable.
    **  X  -  restore a certain filename
    **  f  -  specify media
    **/

    if (q_flag)
      sprintf (cmd, "/usr/sbin/restore -qxpf%s %s > %s ", bff->path, 
                    "./service_num", error_log);
    else
      sprintf (cmd, "/usr/sbin/restore -xpf%s %s > %s ", bff->path, 
                    "./service_num", error_log);

    rc = system (cmd);
    if (rc != SUCCESS  &&  strcmp (error_log, "/dev/null"))
       system (print_error_log);

    /**
     ** if the system cmd fails, then we know this because no
     ** service_num file will exist.
     **/

    if ((fp = fopen ("service_num", "r")) == NIL (FILE))
    {
        service_num = 1;
    }

    else
    {
        fscanf (fp, "%d", &service_num);
        fclose (fp);

       /**
        **  Indicate to the calling routine that
        **  the "./service_num" file DOES exist.
        **/

        *sn_exists = 1;
    }


    return service_num;
}

/* -----------------------------------------------------------------------
**
** FUNCTION:           space_message
**
** DESCRIPTION:        Issues the appropriate message and returns an rc
**          which indicates whether failure is required or not.  If the
**          X flag was specified from the user, then we need to fail
**          because we've already tried to expand.  Else keep on going.
**
** PARAMETERS:         none
**
** RETURNS:            INUGOOD  -- if successful
**                     INUSYSFL -- if failure
**
** ----------------------------------------------------------------------- */

int space_message  (void)
{

 if (X_flag)
 {
     inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, 
                          CMN_NO_SPACE_D), INU_BFFCREATE_CMD);
     return INUSYSFL;
 }

 inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NOT_ENOUGH_W, CMN_NOT_ENOUGH_D),
                              INU_BFFCREATE_CMD);

 return INUGOOD;
}


/* -----------------------------------------------------------------------
**
** FUNCTION:             system_message
**
** DESCRIPTION:          Issues a system message.
**
** PARAMETERS:           cmdline  -- used in message
**                       lppname  -- used in message
**
** RETURNS:              none
**
** ----------------------------------------------------------------------- */

void system_message (char * cmdline, 
                     char * lppname)
{
   inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, CMN_SYSCOM_D), 
                        inu_cmdname, cmdline);

   inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_FAILED_CREATE_E, 
                                        BFF_FAILED_CREATE_D), lppname);
}


/* -----------------------------------------------------------------------
**
** FUNCTION:     inu_get_bff_name
**
** DESCRIPTION:  Determines what the bff should be named.  There are
**               3 possibilities:
**
**        image type               corresponding name
**        -------------------------------------------------------------------
**        Install image     --    <lppname>.<part>.<v.r.m.f>   
**        3.1 update        --    <lppname>.<v.r.m.f>.<service #>
**        3.2 update        --    <lppname>.<v.r.m.f.ptf>
**        4.1 update        --    <option>.<part>.<v.r.m.f>
**             *4.1 updates will contain one and only one option 
**
**        part = [ "usr" | "share" ]
**
** PARAMETERS:
**        op             --     option ptr in toc linked list
**        destdir        --     dir that bff will
**
** RETURNS:             The newly created bff name.
**
** ----------------------------------------------------------------------- */

/* create path to target bff file */

static char * inu_get_bff_name (Option_t * op, 
                                char     * destdir)
{
    static char bffname[PATH_MAX];
    int    service_num=0;    /* Must be initialized to 0 for get_service_num */
    int    sn_exists=0;      /* service_num file (for a 3.1 update) exists   */
    int    rc, dupl_no;      /* dupl_no is the return value from
                                is_duplicate_bff_filename */
    struct stat st_buf;      /* for call to stat */
    char   part [20];        /* "usr" or "share" -- indicates part          */

    inu_get_prodname (op, lppname);

    switch (op->content)
    {
         case CONTENT_USR: case CONTENT_BOTH:
              strcpy (part, "usr");
              break;

         case CONTENT_SHARE:
              strcpy (part, "shr");
              break;

         default:
              strcpy (part, "usr");
              break;
    }

    switch (op->bff->action)
    {
         case ACT_INSTALL :

             /**
              **  We know that we're dealing with an install image.
              **/

             sprintf (bffname, "%s/%s.%s.%d.%d.%d.%d", destdir, lppname, 
                                part, op->level.ver, op->level.rel, 
                                op->level.mod, op->level.fix);
             break;

         case ACT_SING_UPDT:
         case ACT_MULT_UPDT:
         case ACT_GOLD_UPDT:
         case ACT_EN_PKG_UPDT:
         case ACT_EN_MEM_UPDT:
         case ACT_INSTALLP_UPDT:
         case ACT_REQUIRED_UPDT:
         case ACT_CUM_UPDT:

             /**
              **  We know that we're dealing with an update image (a ptf).
              **/

             if (IF_3_1_UPDATE (op->op_type))
             {
                /**
                 **   We know we're dealing with a 3.1 update image.  So, 
                 ** if the image contains a "./service_num" file, then
                 ** we can confidently overwrite any existing filename
                 ** that we generate here (if it happens to already exist).
                 **   If the "./service_num" file is not contained in the
                 ** 3.1 update image, then we CANNOT overwrite any existing
                 ** filename.  So, create the name, see if that file already
                 ** exists, ..., loop until we generate a filename that
                 ** does not already exist.
                 **/

                 do
                 {
                     service_num = get_service_num (op->bff, destdir, 
                                                    &sn_exists, service_num);

                     sprintf (bffname, "%s/%s.%d.%d.%d.%d.%d", destdir, 
                            lppname, op->level.ver, op->level.rel, 
                            op->level.mod, op->level.fix, service_num);
                 /**
                  **  while the "./service_num" file does not exist
                  **                     and
                  **  while the bffname we just generated already exists, 
                  **
                  **  generate another bffname.
                  **/
                  rc = stat (bffname, &st_buf);
                 } while ( ! sn_exists  &&  rc == 0);
             }

            /** ------------------------------------------------------------ *
             **  3.2 update (has a ptf)
             ** ------------------------------------------------------------ */

             else if (IF_3_2_UPDATE(op->op_type))
             {
                 sprintf (bffname, "%s/%s.%d.%d.%d.%d.%s", destdir, 
                            lppname, op->level.ver, op->level.rel, 
                            op->level.mod, op->level.fix, op->level.ptf);
             }

            /** ------------------------------------------------------------ *
             **  4.1-or-later update 
             ** ------------------------------------------------------------ */

             else
             {
                 sprintf (bffname, "%s/%s.%s.%d.%d.%d.%d", destdir, 
                         op->name, part, op->level.ver, op->level.rel, 
                         op->level.mod, op->level.fix);
             }

             break;

         default:
             /**
              **  May as well put the level and aprt into the image name, 
              **  if we know what they are (We know that it is NOT an
              **  install or update image).
              **/

             if (part [0] != '\0')
                 sprintf (bffname, "%s/%s.%s.%d.%d.%d.%d", destdir, lppname, 
                                   part, op->level.ver, op->level.rel, 
                                   op->level.mod, op->level.fix);
             else
                 sprintf (bffname, "%s/%s.%d.%d.%d.%d", destdir, lppname, 
                                   op->level.ver, op->level.rel, 
                                   op->level.mod, op->level.fix);
             break;

    } /* of switch */

                /* Check for duplications */
    if ((dupl_no  = is_duplicate_bff_filename (bffname)) != 0) {
                sprintf (bffname, "%s.%d", bffname, dupl_no);
    }

    return (bffname);
}


/* -----------------------------------------------------------------------
**
** FUNCTION:              inu_do_file
**
** DESCRIPTION:           Copies a bff image from one directory to another.
**          The new name under the destination directory is created first.
**
** PARAMETERS:            bff     -- a node in the toc linked list.
**                        destdir -- destination directory of bff
**
** RETURNS:
**                        INUGOOD  --  if success
**                        INUSYSFL --  if system call failed
**
** ----------------------------------------------------------------------- */

/* copy image file */

static int inu_do_file (BffRef_t * bff, 
                        char     * destdir)

{
    int rc;
    char cmdline[PATH_MAX*5];

    bff_dest = inu_get_bff_name (bff->options->next->option, destdir);

    if ((rc = inu_file_eval_size (bff_dest, bff->path, 1)) != SUCCESS) {
        if (rc == INUFS)
           return (rc);
        if ((rc = space_message ()) != INUGOOD)
           return rc;
    }

    sprintf (cmdline, "cp %s %s", bff->path, bff_dest);

    if ((rc = MASK (system (cmdline))) != 0)
    {
        system_message (cmdline, lppname);
        return INUSYSFL;
    }


    if (v_flag)
    {
        printf ("%s\n", bff_dest);
    }
    return INUGOOD;
}


/* -----------------------------------------------------------------------
**
** FUNCTION:             inu_do_stacked_diskette
**
** DESCRIPTION:          Gets the image specified in the bff parm from a
**        stacked diskette and copies it to destdir.  This is accomplished
**        by calling the inurdsd (inu read stacked disk) command via a
**        system call.  This routine just writes the image to standard out, 
**        where it is then redirected to destdir.
**
** PARAMETERS:           bff     -- toc linked list node specifying bff info
**                       destdir -- destination of diskette bff
**
** RETURNS:              INUGOOD  -- if success
**                       INUSYSFL -- if failure on system call
**                       FAILURE  -- if failure
**
** ----------------------------------------------------------------------- */

static int inu_do_stacked_diskette (BffRef_t * bff, 
                                    char     * destdir)
{
    int rc;
    char cmdline[PATH_MAX*5];
    Option_t *op;
    static int already_printed=0;


    op = bff->options->next->option;

    /**
     **  Determine what the bff should be called in destdir.
     **/

    bff_dest = inu_get_bff_name (bff->options->next->option, destdir);


   /** ----------------------------------------------------------- * 
    **  If there is no bff size info in the toc, then we need to
    **  give a warning about it and continue.
    ** ----------------------------------------------------------- */

    if (bff->size <= 0)
    {
       if ( ! already_printed)
       {
          inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_NO_SIZE_IN_TOC_I, 
                                               BFF_NO_SIZE_IN_TOC_D)); 
          ++already_printed;
       }
    }

   /** ----------------------------------------------------------- *
    **  Must ensure that there is indeed enough space on the
    **  file system to hold the bff.
    ** ----------------------------------------------------------- */

    else if ((rc = inu_bff_eval_size (destdir, bff->size)) != SUCCESS) {
        if (rc == INUFS)
                return (rc);
        if ((rc = space_message ()) != INUGOOD)
            return rc;
    }

    sprintf (cmdline, "/usr/sbin/inurdsd -d %s -v %d -b %d -n %d > %s ", 
                       op->bff->path, op->bff->vol, op->bff->off, 
                       op->bff->size, bff_dest);

    if ((rc = MASK (system (cmdline))) != 0)
    {
        system_message (cmdline, lppname);
        return INUSYSFL;
    }


    if (v_flag)   /* verbose mode */
    {
        printf ("%s\n", bff_dest);
    }

    return INUGOOD;
}



/* -----------------------------------------------------------------------
**
** FUNCTION:           inu_do_tape
**
** DESCRIPTION:        Pulls a bff from a tape, creates a unique name for
**        it and puts it into the dir specified by destdir.
**
** PARAMETERS:         bff     -  a pointer to a bff node in the toc linked
**                                list.
**                     destdir -  the destination directory that the bff
**                                is supposed to be transfered to.
**
** RETURNS:            SUCCESS - if all went well
**                     FAILURE - if something went wrong
**
** ----------------------------------------------------------------------- */

/* copy tape image */

static int inu_do_tape (BffRef_t * bff, 
                        char     * destdir)
{
    int rc=0;
    char cmdline[PATH_MAX*5];
    static int already_printed=0;

   /**
    **  To expand for tape we must have a size
    **/

    if (bff->size <= 0)
    {
       if ( ! already_printed)
       {
          inu_msg (SCREEN_LOG, MSGSTR (MS_BFF, BFF_NO_SIZE_IN_TOC_I, 
                                               BFF_NO_SIZE_IN_TOC_D));
          ++already_printed;
       }
    }
    else if (inu_bff_eval_size (destdir, bff->size) != SUCCESS)
       if ((rc = space_message ()) != INUGOOD)
          return rc;

    bff_dest = inu_get_bff_name (bff->options->next->option, destdir);

    sprintf (cmdline, 
             "/bin/dd if=%s ibs=62b obs=1240b conv=sync of=%s > %s 2>&1", 
             bff->path, bff_dest, error_log);

    if ((rc = MASK (system (cmdline))) != 0)
    {
        if (strcmp (error_log, "/dev/null"))
           system (print_error_log);

        system_message (cmdline, lppname);
        return (INUSYSFL);
    }

    /**
     **  Because the dd command advances the tape head to the next
     **  position on the tape when it reads the tape images, we
     **  have to increment the current offset.
     **/

    ++current_off;


    if (v_flag)    /* verbose mode */
    {
        printf ("%s\n", bff_dest);
    }

    return rc;
}

/* -----------------------------------------------------------------------
**
** FUNCTION:              inu_do_non_stacked_diskette
**
**
** DESCRIPTION:          Gets the image specified in the bff parm from a
**       non-stacked diskette and copies it to destdir.  This is accomplished
**       by calling the restore command via a system call to restore the
**       entire image into a temporary working dir -- that this routine
**       creates.  Then we do a find on everything in the current dir
**       and pipe it to backup to create the image in the destdir.  This was
**       done this way because of the case where an image spans more than 1
**       diskette.
**
** PARAMETERS:           bff  --  the toc linked list bff node
**                       destdir -- destination dir (of bff on diskette)
**                       workdir -- working dir to restore into
** RETURNS:
**                       INUGOOD -- upon success
**                       INUCHDIR -- if failed chdir'ing somewhere
**                       INUNOMK  -- if couldn't make a directory
**                       INUSYSFL -- if system call failed
**
** ----------------------------------------------------------------------- */

/* copy diskettes image */

static int inu_do_non_stacked_diskette (BffRef_t * bff, 
                                        char     * destdir, 
                                        char     * workdir)
{
    char tmpdir[PATH_MAX];
    char cmdline[PATH_MAX*5];
    int rc=0;

    /**
     ** create a unique name for the temp dir
     **/

    sprintf (tmpdir, "%s/XXXXXX", workdir);
    mktemp  (tmpdir);


    /**
     **  cd to the workdir
     **/

    if ((rc = chdir (workdir)) != SUCCESS)
    {
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                             CMN_CANT_CHDIR_D), inu_cmdname, workdir);
        return INUCHDIR;
    }

   /**
    **  make the temp dir
    **/

    rc = mkdir   (tmpdir, (mode_t) 0700);

    if (MASK (rc))           /* nonzero rc ==> failure */
    {
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                             CMN_NO_MK_DIR_D), inu_cmdname, tmpdir);
        return INUNOMK;
    }


    if ((rc = chdir (tmpdir)) != SUCCESS)
    {
        inu_msg (SCREEN_LOG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                             CMN_CANT_CHDIR_D), inu_cmdname, tmpdir);
        return INUCHDIR;
    }

    /**
     ** create the bff name
     **/

    bff_dest = inu_get_bff_name (bff->options->next->option, destdir);

    /**
     ** restore the image under the workdir directory -- via restore
     **/

    /* --------------------------------------------------------------
     * Defect 86960
     * Do not redirect stderr to stdout since we wanna see the 
     * prompts, etc.
     * --------------------------------------------------------------*/
    if (q_flag)
        sprintf (cmdline, "/usr/sbin/restore -qf %s > %s ", bff->path, error_log);
    else
        sprintf (cmdline, "/usr/sbin/restore -f %s > %s ", bff->path, error_log);

    if ((rc = MASK (system (cmdline))) != 0)
    {
        if (strcmp (error_log, "/dev/null"))
           system (print_error_log);

        system_message (cmdline, lppname);
        chdir (workdir);
        inu_rm (tmpdir);
        return (INUSYSFL);
    }

    /**
     **  Create the image under the name of bf_dest -- a full path name.
     **/

    sprintf (cmdline, "find . -print | backup -pirf %s > /dev/null", bff_dest);

    if ((rc = MASK (system (cmdline))) != 0)
    {
        chdir (workdir);
        system_message (cmdline, lppname);
        inu_rm (tmpdir);
        return (INUSYSFL);
    }

    /**
     **  If we get here then we successfully created the bff.  So, 
     **  increment the var that keeps track of this.
     **/


    if (v_flag)
    {
        printf ("%s\n", bff_dest);
    }

    chdir (workdir);

     /* remove the stuff that was restored */
    inu_rm (tmpdir);

    return INUGOOD;
}


/* -----------------------------------------------------------------------
**
** FUNCTION:              bff
**
** DESCRIPTION:           Does most of the leg work for the bffcreate
**           command.  It determines which type of input device we are
**           dealing with and calls the appropriate routine to get the
**           bff image from the device.  It will also create a unique
**           name for the bff to be called in the destdir directory.
**
** PARAMETERS:            toc     -- anchor to the toc linked list
**                        destdir -- destination directory
**                        workdir -- temporary working directory
**
** RETURNS:               INUGOOD -- if success
**                        rc      -- from a lower level routine if failure
**
** ----------------------------------------------------------------------- */

/* Create the bff(s) in destdir */

int bff (TOC_t * toc, 
         char  * destdir, 
         char  * workdir)

{
    BffRef_t *bff;
    int      rc=0;       /* return code */


    /**
     **   Create a temporary file to store the standard out and standard
     **   error of dd and restore. If either of these commands fail this
     **   file will be sent to standard out.
     **/

    sprintf (error_log, "%s/error.log.XXXXXX", workdir);
    rc = mkstemp (error_log);
    if (rc == -1)
        sprintf (error_log, "/dev/null");
    rc = 0;
    sprintf (print_error_log, "cat %s", error_log);

    /**
     **   Determine what type of image we're processing.  Then process it.
     **/

    switch (toc->type)
    {
        /**
         ** regular floppy image
         **/

        case TYPE_FLOP_BFF:

        /**
         **  Should be only one entry in the toc linked list here.  So
         **  process it.
         **/

            diskette = TRUE;
            if (toc->content->next->crc == MARKED)
                rc = inu_do_non_stacked_diskette (toc->content->next, destdir, 
                                                  workdir);
            break;


        /**
         ** stacked floppy images
         **/

        case TYPE_FLOP_SKD:

            /**
             **  Traverse the toc linked list and process all entries
             **  that have their crc field == to MARKED.
             **/

            diskette = TRUE;
            for (bff =  toc->content->next;
                 bff != NIL (BffRef_t)  &&  rc == INUGOOD;
                 bff =  bff->next)
            {

                if (bff->crc == MARKED)
                    rc = inu_do_stacked_diskette (bff, destdir);
            }

            break;


        /**
         ** stacked tape images
         **/

        case TYPE_TAPE_SKD:

            for (bff  = toc->content->next;
                 bff != NIL (BffRef_t)  &&  rc == INUGOOD;
                 bff  = bff->next)
            {
                /**
                 **  Check if this bff node needs to be processed.
                 **  It's crc field will be == to MARKED if it does.
                 **/

                if (bff->crc == MARKED)
                {
                   /**
                    **  insert volume prompt needed here ???
                    **/

                        if(inu_position_tape (bff->options->next->option) != 0)
				return(1);

                    rc = inu_do_tape  (bff, destdir);
                }
            }
            break;

       /**
        ** regular tape image
        **/

        case TYPE_TAPE_BFF:

            if (toc->content->next->crc == MARKED)
                rc = inu_do_tape (toc->content->next, destdir);
            break;


       /**
        ** otherwise it's a file
        **/

        default:
            for (bff  = toc->content->next;
                 bff != NIL (BffRef_t);
                 bff  = bff->next)
            {
                /**
                 **   Check if this bff node needs to be processed.
                 **  It's crc field will be == to MARKED if it does.
                 **  There should be ONLY one MARKED bff in the
                 **  entire toc linked list (however long the list
                 **  may be).
                 **/

                if (bff->crc == MARKED)
                    rc = inu_do_file (bff, destdir);
            }
            break;

    } /* of switch */

   /**
    **   Remove the temporary file before returning
    **/

    if (strcmp (error_log, "/dev/null"))
        unlink (error_log);

    return rc;
}


/* -----------------------------------------------------------------------
**
** FUNCTION:          is_duplicate_bff_filename
**
** DESCRIPTION:
**      This function is added to check if there are more than one bff with
**      same file name on the input device/directory, if there are more than 1
**      bffs with same file name then the first one gets copied as it is and
**      the subsequent ones are appended with ".dupl_no", where
**      dupl_no is number of times same file name appears on the input device
**      , for example if /dev/rmt contains following bffs:
**      bosext2.usr.3.2.0.0  bosext2.usr.3.2.0.0  bosext2.usr.3.2.0.0
**      then your output directory say, /usr/sys/inst.images will contain
**      bosext2.usr.3.2.0.0  bosext2.usr.3.2.0.0.1
**      bosext2.usr.3.2.0.0.2
**
** PARAMETERS: filename - bff file name that is generated from inu_get_bff_na
me
**
** RETURNS:    dupl_no - number of duplicate file names.
**
** ----------------------------------------------------------------------- */

struct FuncName {
        char *name;     /* name of the bff file */
        int  dup;       /* no. of times I have seen it */
	struct FuncName *chain;
};

 /* defination for type def */

typedef struct FuncName *Name;
typedef struct FuncName FuncNameNode;

#define HASHTABLESIZE 1021

/* hash table length is a prime number so that we get better range of
 * hash values and less hash collisions (two or more different bff file
 * names getting same hash value).
 */

static Name FileTable[HASHTABLESIZE];

/*
 * Given a file name, hash the name, and see of the name has already 
 * been defined in the hash table, if it alreadt exists, then just 
 * increment the count "dup". Otherwise, add the name into the hash table, 
 * and set the count to zero.
 *
 * Initially all the elements of FileTable will be set to NULL.
 *
 */


int is_duplicate_bff_filename (filename)
char *filename;
{
    register unsigned h = 0;    /* h is the hash number of the file
                                   name-uniquely generated */
    register char *p;           /* running pointer for hastable array-
				   stop at one element and update the 
				   count - points to structure*/
    register Name next, 
		  previous ; 
    int 	  length;            /*length of the file name */


    /* Convert the file name into a hash number */
    for (p = filename; *p != '\0'; p++) {
        h = (h << 1) ^ (*p);
    }

    /* We want to make sure that the number generated above fits into
       the hash table, therefore we do hash_number modulo 
       HASHSIZE-1 as fo llows
    */
    h %= (HASHTABLESIZE-1);

    next = FileTable[h];
    if (next == 0) {

        /* This means, the filename doesn't exist in our hash tables, 
	   so add it
        */

        AddNameToHashTable (&FileTable[h], filename);
        return FileTable[h]->dup;
    } else {

	/* Check to see if it exists in the links... */

    	while (next != NULL) {
        	if (strcmp (filename, next->name) == 0) {
			next->dup += 1;
			return next->dup;
		} else {
			previous = next;
                	next = next->chain;
		}
        }

	/* we will come here if the name is a unique name, but its hash 
	   value matches the hash value of the names. So we add this name 
	   into the hash table.
	*/

	AddNameToHashTable (&(previous->chain), filename);

	return previous->chain->dup;
    }

}

/* -----------------------------------------------------------------------
**
** FUNCTION:    AddNameToHashTable
**
** DESCRIPTION: Adds name to the hash table and sets duplicate count to zero.
**
** PARAMETERS:  p - address of the previous node.
**              filename - bff file name.
**
** DESCRIPTION: Sets the duplicate count = 0 and returns to calling function.
**
** ----------------------------------------------------------------------- */


AddNameToHashTable (p, filename)
Name	*p;
char	*filename;
{
	int length;

        *p = (Name) malloc (sizeof (FuncNameNode));

        length = strlen (filename);

        /* Get the space to hold filename */
        (*p)->name = (char *) malloc (length + 1);

        /* Copy the filename into our hash table */
        strncpy ((*p)->name, filename, length);

	/* Add null to end of string in filename);*/
	 (*p)->name[length] = '\0';

        (*p)->dup = 0;
	 (*p)->chain = NULL;
	return ;
}

/* -----------------------------------------------------------------------
**
** FUNCTION:    init_filetable
**
** DESCRIPTION: Initialize the hash table.
**
** PARAMETERS:  none.
**
** RETURNS:     none.
**
** ----------------------------------------------------------------------- */ 

init_filetable ()
{
        /* Initialize the hash table, this is called from main () in
           bffcreate.c
        */
        bzero ((char *) &FileTable[0], sizeof (FileTable));
}

