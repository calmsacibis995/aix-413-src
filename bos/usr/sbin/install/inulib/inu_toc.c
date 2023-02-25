static char     sccsid[] = "@(#)76      1.74  src/bos/usr/sbin/install/inulib/inu_toc.c, cmdinstl, bos412, 9446C 11/14/94 14:26:26";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: get_opt_info
 *              inu_dump_lvl_msg
 *              inu_set_prod_name
 *              inu_toc_level_convert
 *              load_bff_toc
 *              load_disk_toc
 *              load_stack_disk_toc
 *              load_tape_toc
 *              open_toc
 *              read_opt_info
 *              set_action
 *              set_content
 *              set_hdr_fmt
 *              set_op_type
 *              set_vpdtree
 *              stacked_disk_dowrite
 *              toc_size
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <values.h>
#include <inu_dev_ctl.h>

/*-------------------------------------------------------------------------*
 * Global variables
 *-------------------------------------------------------------------------*/

int             current_vol;
int             current_off;
extern char    *inu_cmdname;    /* routine that called these routines           */
int             toc_caller;

/*-------------------------------------------------------------------------*
 * A MACRO for if we want to error return when dealing with a tape device.
 *-------------------------------------------------------------------------*/

#define TAPE_RETURN  {trewind (tape); tclose (tape); return (FAILURE);}

/*-------------------------------------------------------------------------*
 * A MACRO for if we want to continue on with the possibility of dealing
 * with a tape with bff.
 *-------------------------------------------------------------------------*/

#define TAPE_RETURN_BFF {trewind (tape); tclose (tape); return (TAPE_BFF);}

/*-------------------------------------------------------------------------*
 * A MACRO for if we want to error return when dealing with stacked diskettes.
 *-------------------------------------------------------------------------*/

#define FLOP_RETURN     {fclose (media_ptr); unlink (".toc"); return (FAILURE);}

#define BADDSKT (-2)

static int set_action (int  * action, 
                       char * code);

static int inu_set_prod_name (Option_t *, 
                              char *);

static int inu_toc_level_convert (char     * level, 
                                  Option_t * op, 
                                  int        bff_fmt, 
                                  char     * i_action);

static int verify_level_syntax (char *op_name, int bff_fmt, Level_t *level,
                                int action);


/*--------------------------------------------------------------------------*
**
** FUNCTION:    open_toc
**
** DESCRIPTION: Determines the type of input device and calls the appropriate
**              TOC creation routine.
**
** PARAMETERS:  toc     - Pointer to the top of the toc link list.
**              media   - Name of the input device.
**
** RETURNS:     Pointer to a table of contents or NIL if malformed file.
**
**--------------------------------------------------------------------------*/

int open_toc (TOC_t ** toc,     /* Pointer to the top of the toc link list */
              char   * media,   /* Name of the input device */
              int      q_flag,  /* whether the -q flag was specified */
              int      caller)  /* calling routine */
{
   struct stat     mstat;
   BffRef_t       *bff_head;   /* ptr to the anchor of the bff link list */
   Option_t       *op_head;    /* ptr to the anchor of the opt link list */
   char           *cmd;        /* Command to generate a .toc for a directory */
   int             tape;       /* return from the topen on the tape device */
   int             rc;
  
   /*----------------------------------------------------------------*
    * Added for feature 43659                                        *
    *----------------------------------------------------------------*/
    FILE *fp;
    boolean bff=FALSE;
    int real_block_size;    /* blocksize at which tape is actually written */
    extern int userblksize;

   toc_caller = caller;

   /*-----------------------------------------------------------------------*
    * Set the default value of current vol to -1.
    *-----------------------------------------------------------------------*/

   current_vol = -1;

   /*-----------------------------------------------------------------------*
    * Do a stat on the media to make sure it is a valid media.
    *-----------------------------------------------------------------------*/

   if ((rc = stat (media, &mstat)) < 0)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, CMN_BAD_DEV_D), 
                                   inu_cmdname, media);
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Create the first bff ref structure to be pointed to from the Toc struct
    *-----------------------------------------------------------------------*/

   if ((bff_head = create_bffref (0, 0, 0, 0, 0, "", 0, ACT_UNKNOWN)) ==
                                                                NIL (BffRef_t))
      return (FAILURE);

   /*-----------------------------------------------------------------------*
    * Create the first option structure to be pointed to from the Toc struct
    *-----------------------------------------------------------------------*/

   op_head = create_option ("Selected Option List", 
                            NO,               /* default op_checked field to
                                               * NO */
                            NO,               /* default quiesce field to NO */
                            CONTENT_UNKNOWN,  /* default content field
                                               * to unknown */
                            "",               /* default lang field to
                                               * NIL (char) */
                            NIL (Level_t),    /* default level field to
                                               * NIL (Level_t) */
                            NullString,       /* default desc field to
                                               * NIL (char) */
                            NIL (BffRef_t));  /* default bff field to
                                               * NIL (BffRef_t) */
    if (op_head == NIL (Option_t))
      return (FAILURE);

   /*-----------------------------------------------------------------------*
    * Create the Toc anchor for the options and bff links lists.
    *-----------------------------------------------------------------------*/

   if ((*toc = create_toc (bff_head, op_head)) == NIL (TOC_t))
      return (FAILURE);

   /*
    * This prevents possible infinite recursion.  The inutoc command
    * calls this routine.  Below a sytem call is issued to the inutoc
    * command, if the media passed is a directory.
    */

   if (caller == CALLER_INUTOC)
      return SUCCESS;

   /*-----------------------------------------------------------------------*
    * Determine the type of device we're dealing with.
    *-----------------------------------------------------------------------*/

   switch (mstat.st_mode &S_IFMT)
   {
      case S_IFDIR:
         /*---------------------------------------------------------------*
          * The input media is a directory name.  We assume that the
          * directory contains a file called '.toc'.
          *---------------------------------------------------------------*/

         cmd = (char *) malloc (strlen (media) + 40);
         if (cmd == NIL (char))
          {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                 CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
          }
         strcpy (cmd, media);
         if (media[strlen (media) - 1] == '/')
            strcat (cmd, ".toc");
         else
            strcat (cmd, "/.toc");
         if (stat (cmd, &mstat) != 0)
         {
            strcpy (cmd, "/usr/sbin/inutoc ");
            strcat (cmd, media);
            strcat (cmd, ">/dev/null 2>&1");
            rc = inu_do_exec (cmd);
            if (rc != 0)
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CREATE_TOC_E, 
                                    CMN_CREATE_TOC_D), inu_cmdname, media);
               free (cmd);
               return (FAILURE);
            }
         }
         free (cmd);
         rc = load_disk_toc (*toc, media);

         break;

      case S_IFBLK:
      case S_IFCHR:

         /*---------------------------------------------------------------*
          * The input media is a character device (probably a tape drive).
          * The tape can contain either a single backup-format-file or a
          * TOC and multiple backup format files.
          *---------------------------------------------------------------*/

         current_vol=0;   /* initialize it */
         /*---------------------------------------------*
          * Added for feature 43659                     *
          *---------------------------------------------*/
          if (is_it_a_tape_device (media))
          {
            /*----------------------------------------------------------*
             * Feature 43659                                            *
             * Tape devices have to be opened with no-rewind-on-close,  *
             * and  no-retension-on-open, i.e. with 0.1 (0.5 --Low      *
             * density, 0.1 High density).The following code appends .1 *
             * to /dev/rmt? (On AIX) if it is not already specified.    *
             * Actually it is device independent and figures out the    *
             * relevant interface.                                      *
             *----------------------------------------------------------*/
             get_correct_interface (&media, &rc);
             if (rc == TRUE)
             {
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                     CMN_BAD_DEV_D), inu_cmdname, media);
                return (FAILURE);
             }

             if ((rc = stat (media, &mstat)) < 0)
             {
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                     CMN_BAD_DEV_D), inu_cmdname, media);
                return (FAILURE);
             }

             if ((tape = topen (media, NO_PROMPT, O_RDONLY, q_flag)) == -1)
             {
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                     CMN_BAD_DEV_D), inu_cmdname, media);
                return (FAILURE);
             }

             if ((rc = load_tape_toc (*toc, media, tape)) == TAPE_BFF)
             {
   		int starting_tape_blksize;
   		int real_tape_blksize;
    		char binval [62*512];



                inu_msg (PROG_MSG, MSGSTR (MS_INUCOMMON, 
                                     CMN_DEV_INFORM_WAIT_I, 
                                     CMN_DEV_INFORM_WAIT_D), inu_cmdname);

                /*--------------------------------------------------*
                 * Added for feature 43659                          *
                 * Now we have 2 possibilities:                     *
                 *   1. Tape contains a single bff                  *
                 *   2. Tape is empty                               *
                 * -------------------------------------------------*/

                /* tape file is closed at this time */

                /* -------------------------------------------------*
                 * Case2:                                           *
                 *    Need to get the real block size that the tape *
                 *    is written at and set the devices' block size *
                 *    to that value.                                *
                 * -------------------------------------------------*/


                /* not calling topen because it changes block size */
                if ((tape = open (media, O_RDONLY)) == -1)
                {
                   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                        CMN_BAD_DEV_D), inu_cmdname, media);
                   return (FAILURE);
                }

                if (trewind (tape) != SUCCESS)
                {
                   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                        CMN_BAD_DEV_D), inu_cmdname, media);
                   return (FAILURE);
                }

                /* set block size to zero */
                if (tchg_blksize_to_zero (tape,0,&starting_tape_blksize) != SUCCESS)
                {
                   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                        CMN_BAD_DEV_D), inu_cmdname, media);
                   return (FAILURE);
                }

   		rc=read(tape,binval,sizeof(binval));
		
		/* try 512 for 1/4" tape */
		if(rc<0)
                {
                   if (trewind (tape) != SUCCESS)
                   {
                   	inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
					CMN_BAD_DEV_D), inu_cmdname, media);
                   	return (FAILURE);
                   }
                   if (tchg_blksize_to_zero (tape,512,0) != SUCCESS)
                   {
                   	inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
					CMN_BAD_DEV_D), inu_cmdname, media);
                   	return (FAILURE);
                   }
   		   rc=read(tape,binval,512);
                }

		if(rc<=0)
		{
                   tclose (tape);
                   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
				CMN_BAD_DEV_E, CMN_BAD_DEV_D), 
				inu_cmdname, media);
                   return (FAILURE);
		}
		if(rc != 512 &&  rc != 1024)
		   rc=0;
		else
   		   real_tape_blksize=rc;

                if ((*(binval+3) == 0xea)) /* 1st byte of magic # */
                {
                   switch (*(binval+2)) /* 2nd byte of magic # */
                      {
                         case 0x6b: bff=TRUE;
                                    break;
                         case 0x6c: bff=TRUE;
                                    break;
                         case 0x6d: bff=TRUE;
                                    break;
                         default: bff=FALSE;
                      }
                 }

                 if (bff)
                 {
                      trewind (tape);       /* it is a bff */
                      tclose (tape);
   		      if(starting_tape_blksize != real_tape_blksize)
				tchg_blksize (media, real_tape_blksize);
                      rc = load_bff_toc (*toc, media, S_IFCHR, TRUE, 
                                         CALLER_OPEN_TOC);
                      (*toc)->type = TYPE_TAPE_BFF;
                 }
                 else
                 {
                      trewind (tape);      /* it is empty */
                      tclose (tape);
                      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
                                           CMN_BAD_DEV_E, CMN_BAD_DEV_D), 
                                           inu_cmdname, media);
                      rc = -1;
                      /* printf ("%s is neither a stack tape, nor
                         contains valid backup format file\n", media); */
                 }


             } /* if ((rc=load_tape_toc (*toc,media,tape)) == TAPE_BFF) */

          } /* if (is_it_a_tape_device (media)) */

          else
          {
            /**
             **  We could be dealing with a FLOPPY.
             **/
             if ((rc = load_stack_disk_toc (*toc, media)) < 0)
             {
                if (rc == BADDSKT)
                {
                   rc = FAILURE;  /* Do not change the return code */
                   break;
                }
                rc = load_bff_toc (*toc, media, S_IFCHR, TRUE, CALLER_OPEN_TOC);
                (*toc)->type = TYPE_FLOP_BFF;
             }
          }
 
          break;

      case S_IFREG:
         /*---------------------------------------------------------------*
          * The input media is a regular file.  We assume that this regular
          * file is a single backup-format file.
          *---------------------------------------------------------------*/

         rc = load_bff_toc (*toc, media, S_IFREG, TRUE, CALLER_OPEN_TOC);
         break;

      default:
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                      CMN_BAD_DEV_D), inu_cmdname, media);
         break;

   } /* end switch (mstat.st_mode &S_IFMT) */

  /** ------------------------------------------------------------------ *
   **  If we were called by installp, then verify the toc.  We don't 
   **  wanna error off for missing size file, ... if we're called by
   **  one of the low-level utility cmds -- such as bffcreate or inutoc. 
   **  We shouldn't set policy in low level cmds ! 
   ** ------------------------------------------------------------------ */

   if (rc == INUGOOD  &&  caller == CALLER_INSTALLP)
      if ((rc = verify_toc_semantics (*toc)) != INUGOOD)
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                              CMN_BAD_TOC_D), inu_cmdname);
   return (rc);

} /* end open_toc */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     load_tape_toc
**
** DESCRIPTION:  Reads the file after the second file mark on a tape media, 
**               and interprets this file as a table of contents.
**
** PARAMETERS:   media   - Name of the input device.
**
** RETURNS:      Pointer to a table of contents or NIL if malformed file.
**
**--------------------------------------------------------------------------*/

int load_tape_toc (TOC_t * toc,    /* Pointer to the top of the toc link list */
                   char  * media,  /* Name of the input device */
                   int     tape)   /* the return from the topen on the tape
                                    * device */
{
   int            n;               /* return value from scanf routines */
   FILE          *media_ptr;       /* The FILE ptr to the open of the tape
                                      device */
   BffRef_t       bff;             /* A temporary bff structure for reading in
                                    * bff */
   char           description[BUFSIZ];
   BffRef_t      *nu_bff;          /* The bff ptr returned from the
                                    * create_bffref */
   Option_t       op;              /* A temp opt struct for reading in
                                    * options */
   Option_t      *nu_op;           /* The opt ptr returned from the
                                      create_option */
   OptionRef_t   *op_ref_head;     /* The ptr to the anchor of the bff link
                                    * list */
   OptionRef_t   *nu_opref;        /* The ptr to the OptionRef_t structure. */
   int            rc;              /* generic return code integer */
   int            first_time;      /* generic first time switch for a do
                                    * loop */
   char          *p;               /* generic char pointer */
   char          *q;               /* generic char pointer */
   char           buf[TMP_BUF_SZ]; /* temp buffer for receiving items
                                    * from the toc */
   char           action[TMP_BUF_SZ]; /* temp char to receive action char from
                                       * toc */
   char           location[10];    /* temp string to receive loc string
                                    * from toc   */
   char           level[100];      /* temp string to receive loc string from
                                      toc */
   char           prodname[MAX_LPP_FULLNAME_LEN];  /* Product name for an
                                                    * lpp. */
   char           productname[MAX_LPP_FULLNAME_LEN]; /* new prodname field */
   char 	  tape_bff[62*512];
   int  	  starting_tape_blksize=0;
   int	    	  real_tape_blksize=0;
   extern 	  int no_chdev;


   /*-----------------------------------------------------------------------*
    * Position to the correct tape mark to get to the toc file on the tape.
    * If positioning fails, the tape might contain bff.
    *-----------------------------------------------------------------------*/


   if ((rc = tseek_file (tape, TAPE_TOC_POSITION - 1, TSEEK_ABS)) < 0)
   {
      TAPE_RETURN_BFF;
   }
   else /* type is a stacked tape */
   {
      toc->type = TYPE_TAPE_SKD;
   } /* end if */

   /*-----------------------------------------------------------------------*
    * Open the tape for reading, and read the toc header information.
    *-----------------------------------------------------------------------*/


   tchg_blksize_to_zero(tape,0,&starting_tape_blksize);
   media_ptr = fdopen (tape, "r");
   setbuf(media_ptr,tape_bff);

   real_tape_blksize=read(tape,tape_bff,sizeof(tape_bff));
   tseek_back_one(tape);

   /* 1/4" tape don't work the same as 8mm or 4mm ... */
   if(real_tape_blksize < 0)
      real_tape_blksize=512;

   /* nope nothing to do */
   if(real_tape_blksize == 0)
      TAPE_RETURN_BFF;

   if(real_tape_blksize != 512 &&  real_tape_blksize != 1024)
	real_tape_blksize=0;

   tchg_blksize_to_zero(tape,real_tape_blksize,0);

   media_ptr->_bufendp=tape_bff+(sizeof(tape_bff));

   n = fscanf (media_ptr, TAPE_HEADER_FMT, 
               &(toc->vol),               /* Volume       */
               &(toc->dt),                /* Date of creation */
               buf);                        /* Header format    */

   /*-----------------------------------------------------------------------*
    * If we didn't get correct number of items back from the fscanf, then
    * set return code to continue with the possibility of bff because this
    * must not be a real tape toc file.
    *-----------------------------------------------------------------------*/

   if (n != ITEMS_IN_TAPE_HDR)
   {
      TAPE_RETURN_BFF;
   }

   /*-----------------------------------------------------------------------*
    * Set the current volume to what the toc says.
    *-----------------------------------------------------------------------*/

   current_vol = toc->vol;

   /*-----------------------------------------------------------------------*
    * Set the header format to the integer value in the toc structure.
    *-----------------------------------------------------------------------*/

   if ((rc = set_hdr_fmt (&(toc->hdr_fmt), buf)) != SUCCESS)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      TAPE_RETURN;
   }

   /*-----------------------------------------------------------------------*
    * Read the bff header line from the toc file.
    *-----------------------------------------------------------------------*/

   if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      TAPE_RETURN;
   }

   /*-----------------------------------------------------------------------*
    * Read the bff line in the toc, and then read each option for the bff, 
    * do this until we get to the end of the toc file.
    *-----------------------------------------------------------------------*/

   first_time = 1;
   do
   {
      /*-------------------------------------------------------------------*
       * Scan the input buffer to see if it's the following format.
       * it could be 3.1 or 3.2+ because both may not have the size field.
       * The following are valid formats:
       *                    1:1:10000 3 R S    (valid for 3.2+)
       *                    1:1 3 R S          (valid for 3.1 and 3.2+)
       *
       * Search for the end of the volume field, null terminate it, convert
       * it to an integer and put it into the bff structure.
       *-------------------------------------------------------------------*/

      if ((p = strchr (buf, ':')) == ((char *) 0))
      {
         if (first_time)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            TAPE_RETURN;
         }
         else
            break;
      }
      first_time = 0;
      *p = '\0';
      bff.vol = atoi (buf);
      p++;

      /*-------------------------------------------------------------------*
       * search for the end of the offset field.
       *-------------------------------------------------------------------*/

      for (q = p;
           isdigit (*q);
           q++)
      {
         ;
      }

      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         TAPE_RETURN;
      }

      if (*q != ':')
      {
         bff.size = 0;
         *q = '\0';
         bff.off = atoi (p);
         q++;
      }
      else
      {
         /*---------------------------------------------------------------*
          * Else, null terminate the offset field, convert it to an
          * integer and store if in the bff structure.
          *---------------------------------------------------------------*/

         *q = '\0';
         bff.off = atoi (p);
         p = ++q;

         /*---------------------------------------------------------------*
          * If the next char isn't a numeric then there is no size field.
          *---------------------------------------------------------------*/

         if ( ! isdigit (*p))
            bff.size = 0;
         else
         {
            while (isdigit (*q))
               q++;
            *q = '\0';
            bff.size = atoi (p);
            q++;
         } /* end if */
      } /* end if */

      /*-------------------------------------------------------------------*
       * Skip whitespaces...
       *-------------------------------------------------------------------*/

      for (;
           *q == ' '  ||  *q == '\t';
           q++)
      {
        ;
      } /* end for */

      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         TAPE_RETURN;
      }

      /*-------------------------------------------------------------------*
       * put the the format into the bff structure.
       *-------------------------------------------------------------------*/

      bff.fmt = *q;
      q++;

      /*-------------------------------------------------------------------*
       * Skip whitespaces...
       *-------------------------------------------------------------------*/

      for (;
           *q == ' '  ||  *q == '\t';
           q++)
      {
         ;
      } /* end for */

      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         TAPE_RETURN;
      }

      /*-------------------------------------------------------------------*
       * put the platform into the bff structure.
       *-------------------------------------------------------------------*/

      bff.platform = *q;
      q++;

      /*-------------------------------------------------------------------*
       * Skip whitespaces...
       *-------------------------------------------------------------------*/

      for (;
           *q == ' '  ||  *q == '\t';
           q++)
      {
         ;
      } /* end for */

      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         TAPE_RETURN;
      }

      /*-------------------------------------------------------------------*
       * put the the action field into the bff structure.
       *-------------------------------------------------------------------*/

      sscanf (q, "%s", action);
      q += strlen (action);

     /*-------------------------------------------------------------------*
      * Skip whitespaces...
      *-------------------------------------------------------------------*/

      for (  ;
           *q != '\0'   &&  (*q == ' '  ||  *q == '\t');
           q++)
         ;

      /*-------------------------------------------------------------------*
       * Set the bff action constant based on code in TOC
       *-------------------------------------------------------------------*/

      if (set_action (&(bff.action), action) != SUCCESS)
      {
         TAPE_RETURN;
      }

      productname[0] = '\0';

      if (bff.fmt == FMT_4_1)
      {
         sscanf (q, " %[^ {]", productname);

        /*-----------------------------------------------------------------*
         * No product name in the lpp_name file of a 4.1 image is an error.
         *-----------------------------------------------------------------*/
         if (*q == '\0'  ||  *q == '{')
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MISS_PROD_NAME_E, 
                                         CMN_MISS_PROD_NAME_D), inu_cmdname);
            TAPE_RETURN;
         }
      }

      /*-------------------------------------------------------------------*
       * Create and add the BffRef struct to the toc linked list.
       *-------------------------------------------------------------------*/

      if ((nu_bff = create_bffref (bff.vol, 
                                   bff.off, 
                                   bff.size, 
                                   bff.fmt, 
                                   bff.platform, 
                                   media, 
                                   bff.crc, 
                                   bff.action)) == NIL (BffRef_t))
      {
         TAPE_RETURN;
      }
      if (toc_caller == CALLER_INUTOC)
      {
         nu_bff->action_string = malloc (strlen (action) + 1);
         if (nu_bff->action_string == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                         CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
         }
         strcpy (nu_bff->action_string, action);
      }
      (void) insert_bffref (toc->content, nu_bff);

      if ((op_ref_head = create_optionref (NIL (Option_t))) ==
                                                             NIL (OptionRef_t))
      {
         TAPE_RETURN;
      }
      else
      {
         nu_bff->options = op_ref_head;
      }

      /*-------------------------------------------------------------------*
       * Read the first option line from the toc file.
       *-------------------------------------------------------------------*/

      if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         TAPE_RETURN;
      }

      /*-------------------------------------------------------------------*
       * Read the options which makes up this backup-format file.
       *-------------------------------------------------------------------*/

      inu_set_prod_name (NIL (Option_t), prodname);
      do
      {
         memset (&op, '\0', sizeof (Option_t));

         if (bff.fmt < FMT_3_2)
         {
            n = sscanf (buf, _3_1_TAPE_ENTRY_FMT, 
                        op.name,        /* LPP.OPTION   */
                        level,          /* Level        */
                        &(op.quiesce),  /* Quiesce      */
                        &(op.content),  /* Content      */
                        op.lang,        /* Language     */
                        description);   /* description  */

            if (n < ITEMS_IN_3_1_TAPE_ENTRY)
               break;
         }
         else
         {
            /*-----------------------------------------------------------*
             * ELSE, This is a 3.2 format or greater.
             * In 3.2 the option line is the same no matter what type of
             * media being dealt with. In 3.1 the the location field did
             * not exist for tape and preload media, but did for diskette.
             *-----------------------------------------------------------*/

            n = sscanf (buf, SINGLE_BFF_ENTRY_FMT, 
                        op.name,        /* LPP.OPTION   */
                        level,          /* Level        */
                        location,       /* Location (ignored for now) */
                        &(op.quiesce),  /* Quiesce      */
                        &(op.content),  /* Content      */
                        op.lang,        /* Language     */
                        description);   /* Description  */

            if (n < ITEMS_IN_SINGLE_BFF_ENTRY)
               break;
         }

         /*---------------------------------------------------------------*
          * Convert the level ascii string into the Level_t struct format.
          *---------------------------------------------------------------*/

         if ((rc = inu_toc_level_convert(level,&op,bff.fmt,action)) != SUCCESS)

         {
            TAPE_RETURN;
         }

         /*---------------------------------------------------------------*
          * Create new option
          *---------------------------------------------------------------*/

         if ((nu_op = create_option (op.name, 
                                     NO, 
                                     op.quiesce, 
                                     op.content, 
                                     op.lang, 
                                     &op.level, 
                                     description, 
                                     nu_bff)) == NIL (Option_t))
         {
            TAPE_RETURN;
         } /* end if */

         /*---------------------------------------------------------------*
          * Insert the new option into the linked list
          *---------------------------------------------------------------*/

         (void) insert_option (toc->options, nu_op);

         /*---------------------------------------------------------------*
          * Set the content field. (Convert the 3.1 content values to 3.2)
          *---------------------------------------------------------------*/

         if ((rc = set_content (nu_op)) != SUCCESS)
         {
            TAPE_RETURN;
         }

         /*---------------------------------------------------------------*
          * Set the option member vpd_tree based on the content field
          *---------------------------------------------------------------*/

         (void) set_vpdtree (nu_op);

         /*---------------------------------------------------------------*
          * Set the option member op_type based on the whether this is
          * an update or an installation and whether is for 3.1 or not.
          *---------------------------------------------------------------*/

         (void) set_op_type (nu_op);

        /*------------------------------------------------------------------*
         * If no value for the product name field exists, then one wasn't
         * required (cuz we're not dealing with a 4.1 image).  So, just use
         * "up to the 1st dot" method for the product name.
         * Else  a product name was given,  so use it.
         *------------------------------------------------------------------*/

         if (productname [0]  == '\0')
         {
            if (inu_set_prod_name (nu_op, prodname) != SUCCESS)
            {
               TAPE_RETURN;   /* multi-statement macro w/o squiggly's */
            }
         }
         else if (inu_set_prod_name (nu_op, productname) != SUCCESS)
         {
            TAPE_RETURN;
         }

         /*---------------------------------------------------------------*
          * Create a new option reference list for this bff
          *---------------------------------------------------------------*/

         if ((nu_opref = create_optionref (nu_op)) == NIL (OptionRef_t))
         {
            TAPE_RETURN;
         }

         /*---------------------------------------------------------------*
          * Insert option reference into linked list for this bff
          *---------------------------------------------------------------*/

         (void) insert_optionref (op_ref_head, nu_opref);

         /*---------------------------------------------------------------*
          * Read the next line from the toc file, it could be the beginning
          * of option info (prereq, size, supersede) or it could be another
          * lpp-option line, or it could be the end of the option list.
          *---------------------------------------------------------------*/

         if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            TAPE_RETURN;
         }

         /*---------------------------------------------------------------*
          * Test to see if it's prereq, size, and/or supersede info for
          * this option: The beginning of this info is indicated by the
          * "[\n", the end of this information is indicated by a "]\n".
          * NOTE: The brace must be in column one.
          *---------------------------------------------------------------*/

         if (buf[0] == '['  &&  buf[1] == '\n')
         {
            if ((rc = get_opt_info (nu_op, media_ptr)) != SUCCESS)
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                            CMN_BAD_TOC_D), inu_cmdname);
               TAPE_RETURN;
            }
            /*-----------------------------------------------------------*
             * get_opt_info () just read the ']' (ending delimiter for the
             * option info) , now read the next line from the toc file
             *-----------------------------------------------------------*/

            if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                            CMN_BAD_TOC_D), inu_cmdname);
               TAPE_RETURN;
            }

         }

         /*---------------------------------------------------------------*
          * If we find a right curly brace, then we're at the end of
          * the list of options for this bff, so break out of this loop.
          *---------------------------------------------------------------*/

      } while ( ! (buf[0] == '}'  &&  buf[1] == '\n'));

      /*----------------------------------------------------------------------*
       * Read the next bff header line from the toc file.
       * if we get a NIL (char) then we're done with this toc file.
       *----------------------------------------------------------------------*/

   } while ((fgets (buf, TMP_BUF_SZ, media_ptr)) != NIL (char));

   /*-----------------------------------------------------------------------*
    * skip to the next tape mark if we aren't already there.
    *-----------------------------------------------------------------------*/

   if (feof (media_ptr) == 0)
      (void) tseek_file (tape, 1, TSEEK_REL);

   current_off = TAPE_TOC_POSITION + 1;
   tclose (tape);

   if(starting_tape_blksize != real_tape_blksize && no_chdev==0)
   {
  	rc=fork();
	if (rc<0)
	{
		tchg_blksize (media, real_tape_blksize);
		tape=open(media,O_RDONLY);
   		tseek_file (tape, TAPE_TOC_POSITION, TSEEK_ABS);
		close(tape);
	}
	else if (rc==0)
	{
		lock_tape_restore();
		tchg_blksize (media, real_tape_blksize);
		tape=open(media,O_RDONLY);
   		tseek_file (tape, TAPE_TOC_POSITION, TSEEK_ABS);
		exit(0);
	}

   }

   return (SUCCESS);

} /* end load_tape_toc */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     stacked_disk_dowrite
**
** DESCRIPTION:  Writes the given data to the .toc file for the stacked
**               diskette being read.
**
** PARAMETERS:   buff   - pointer to the data buffer
**               buffsz - number of bytes of valid data in data buffer
**
** RETURNS:      none.
**
**--------------------------------------------------------------------------*/

static int      fd;     /* file to write toc data into */

static int stacked_disk_dowrite (char * buff,   /* pointer to data buffer to be
                                                 * written */
                                 int    buffsz) /* number of bytes to write */
{
   int   pos;    /* position of first null character in input block */

   for (pos = 0;
        pos < buffsz  &&  buff[pos] != '\0';
        pos++);

   if (pos == buffsz)
      return (write (fd, buff, buffsz));
   else
   {
      write (fd, buff, pos + 1);
      return (-1);
   }
}

/*--------------------------------------------------------------------------*
**
** FUNCTION:    load_stack_disk_toc
**
** DESCRIPTION: Reads the blocks staring at the second block on a disk media, 
**              and interprets these blocks as a table of contents.
**
** PARAMETERS:  media   - Name of the input device.
**
** RETURNS:     Pointer to a table of contents or NIL if malformed file.
**
**--------------------------------------------------------------------------*/

int load_stack_disk_toc (TOC_t * toc,   /* Pointer to the top of the toc link
                                         * list */
                         char  * media) /* Name of the input device */
{
   char            description[BUFSIZ];
   int             n;               /* return value from scanf routines */
   FILE           *media_ptr;       /* The FILE ptr to the open of the toc
                                     * file */
   BffRef_t        bff;             /* a temporary bff structure for reading in
                                     * bff */
   BffRef_t       *nu_bff;          /* the bff ptr returned from the
                                     * create_bffref */
   Option_t        op;              /* a temp opt struct for reading in
                                     * options */
   Option_t       *nu_op;           /* the opt ptr returned from the
                                     * create_option */
   OptionRef_t    *op_ref_head;     /* The ptr to the anchor of the bff link
                                     * list */
   OptionRef_t    *nu_opref;        /* The ptr to the OptionRef_t structure. */
   int             rc;              /* generic return code integer */
   int             first_time;      /* generic first time switch for a do
                                     * loop */
   char           *p;               /* generic char pointer */
   char           *q;               /* generic char pointer */
   char            buf[TMP_BUF_SZ]; /* temp buffer for receiving items
                                     * from the toc */
   char            action[TMP_BUF_SZ]; /* temp char to receive action char from
                                        * toc */
   char            location[10];    /* temp string to receive loc string
                                     * from toc   */
   char            level[100];      /* temp string to receive loc string from
                                     * toc */
   char            prodname[MAX_LPP_FULLNAME_LEN];  /* product name to which
                                                     * lpp belongs */
   char            productname[MAX_LPP_FULLNAME_LEN]; /* new prodname field */
   boolean         prompt;


   /*-----------------------------------------------------------------------*
    * Read the first block of the diskette toc into a file.
    *-----------------------------------------------------------------------*/

   if ((fd = creat (".toc", 644)) == -1)
   {
      return (FAILURE);
   }

   if (getenv ("INU_CALLED_FROM_SMIT") != NULL)
      prompt = TRUE;
   else
      prompt = FALSE;

   rc = inurddk (media, 1, NULL, 1, 512, stacked_disk_dowrite, prompt);
   if (rc < 0)
      return (rc);

   /*-----------------------------------------------------------------------*
    * Open the toc for reading, and read the toc header information.
    *-----------------------------------------------------------------------*/

   media_ptr = fopen (".toc", "r");
   n = fscanf (media_ptr, TAPE_HEADER_FMT, 
               &(toc->vol),   /* Volume           */
               &(toc->dt),    /* Date of creation */
               buf);            /* Header format    */

   /*-----------------------------------------------------------------------*
    * If we didn't get correct number of items back from the fscanf, then
    * set the return code to continue trying a diskette bff image since this
    * must not be a real diskette toc file.
    *-----------------------------------------------------------------------*/

   if (n != ITEMS_IN_TAPE_HDR)
   {
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Read the rest of the toc file from the diskette.
    *-----------------------------------------------------------------------*/

   inurddk (media, 1, NULL, 2, MAXINT, stacked_disk_dowrite, TRUE);
   close (fd);

   /*-----------------------------------------------------------------------*
    * Set the toc type to indicate a stacked diskette media.
    *-----------------------------------------------------------------------*/

   toc->type = TYPE_FLOP_SKD;

   /*-----------------------------------------------------------------------*
    * Set the current volume to what the toc says.
    *-----------------------------------------------------------------------*/

   current_vol = toc->vol;

   /*-----------------------------------------------------------------------*
    * Set the header format to the integer value in the toc structure.
    *-----------------------------------------------------------------------*/

   if ((rc = set_hdr_fmt (&(toc->hdr_fmt), buf)) != SUCCESS)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      FLOP_RETURN;
   }

   /*-----------------------------------------------------------------------*
    * Read the bff header line from the toc file.
    *-----------------------------------------------------------------------*/

   if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      FLOP_RETURN;
   }

   /*-----------------------------------------------------------------------*
    * Read the bff line in the toc, and then read each option for the bff, 
    * do this until we get to the end of the toc file.
    *-----------------------------------------------------------------------*/

   first_time = 1;
   do
   {
      /*-------------------------------------------------------------------*
       * Scan the input buffer to see if it's the following format.
       * it could be 3.1 or 3.2+ because both may not have the size field.
       * The following are valid formats:
       *                    1:1:10000 3 R S    (valid for 3.2+)
       *                    1:1 3 R S          (valid for 3.1 and 3.2+)
       *
       * Search for the end of the volume field, null terminate it, convert
       * it to an integer and put it into the bff structure.
       *-------------------------------------------------------------------*/

      if ((p = strchr (buf, ':')) == ((char *) 0))
      {
         if (first_time)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            FLOP_RETURN;
         }
         else
            break;
      }
      first_time = 0;
      *p = '\0';
      bff.vol = atoi (buf);
      p++;

      /*-------------------------------------------------------------------*
       * search for the end of the offset field.
       *-------------------------------------------------------------------*/

      for (q = p; isdigit (*q); q++);
      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         FLOP_RETURN;
      }
      if (*q != ':')
      {
         bff.size = 0;
         *q = '\0';
         bff.off = atoi (p);
         q++;
      }
      else
      {
         /*---------------------------------------------------------------*
          * Else, null terminate the offset field, convert it to an
          * integer and store if in the bff structure.
          *---------------------------------------------------------------*/

         *q = '\0';
         bff.off = atoi (p);
         p = ++q;

         /*---------------------------------------------------------------*
          * If the next char isn't a numeric then there is no size field.
          *---------------------------------------------------------------*/

         if ( ! isdigit (*p))
            bff.size = 0;
         else
         {
            while (isdigit (*q))
               q++;
            *q = '\0';
            bff.size = atoi (p);
            q++;
         }
      } /* end if */

      /*-------------------------------------------------------------------*
       * Skip whitespaces...
       *-------------------------------------------------------------------*/

      for (; *q == ' '  ||  *q == '\t'; q++);
      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         FLOP_RETURN;
      }

      /*-------------------------------------------------------------------*
       * put the the format into the bff structure.
       *-------------------------------------------------------------------*/

      bff.fmt = *q;
      q++;


      /*-------------------------------------------------------------------*
       * Skip whitespaces...
       *-------------------------------------------------------------------*/

      for (;
           *q == ' '  ||  *q == '\t';
           q++)
      {
        ;
      } /* end for */

      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         FLOP_RETURN;
      }

      /*-------------------------------------------------------------------*
       * put the the format into the bff structure.
       *-------------------------------------------------------------------*/

      bff.platform = *q;
      q++;

      /*-------------------------------------------------------------------*
       * Skip whitespaces...
       *-------------------------------------------------------------------*/

      for (;
           *q == ' '  ||  *q == '\t';
           q++)
      {
        ;
      } /* end for */

      if (*q == '\0')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         FLOP_RETURN;
      }
      /*-------------------------------------------------------------------*
       * put the action field into the bff structure.
       *-------------------------------------------------------------------*/

      sscanf (q, "%s", action);
      q += strlen (action);

      /*-------------------------------------------------------------------*
       * Set the bff action constant based on code in TOC
       *-------------------------------------------------------------------*/

      if (set_action (&(bff.action), action) != SUCCESS)
      {
         FLOP_RETURN;
      }

      productname[0] = '\0';

      if (bff.fmt == FMT_4_1)
      {
         sscanf (q, " %[^ {]", productname);

        /*-----------------------------------------------------------------*
         * No product name in the lpp_name file of a 4.1 image is an error.
         *-----------------------------------------------------------------*/
         if (*q == '\0'  ||  *q == '{')
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MISS_PROD_NAME_E, 
                                         CMN_MISS_PROD_NAME_D), inu_cmdname);
            FLOP_RETURN;
         }
      }


      /*-------------------------------------------------------------------*
       * Create and add the BffRef struct to the toc linked list.
       *-------------------------------------------------------------------*/

      if ((nu_bff = create_bffref (bff.vol, 
                                   bff.off, 
                                   bff.size, 
                                   bff.fmt, 
                                   bff.platform, 
                                   media, 
                                   bff.crc, 
                                   bff.action)) == NIL (BffRef_t))
      {
         FLOP_RETURN;
      }
      if (toc_caller == CALLER_INUTOC)
      {
         nu_bff->action_string = malloc (strlen (action) + 1);
         if (nu_bff->action_string == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                     CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
         }
         strcpy (nu_bff->action_string, action);
      }

      (void) insert_bffref (toc->content, nu_bff);

      if ((op_ref_head = create_optionref (NIL (Option_t))) ==
                                                             NIL (OptionRef_t))
      {
         FLOP_RETURN;
      }
      else
         nu_bff->options = op_ref_head;

      /*-------------------------------------------------------------------*
       * Read the first option line from the toc file.
       *-------------------------------------------------------------------*/

      if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         FLOP_RETURN;
      }

      /*-------------------------------------------------------------------*
       * Read the options which makes up this backup-format file.
       *-------------------------------------------------------------------*/

      inu_set_prod_name (NIL (Option_t), prodname);
      do
      {
         memset (&op, '\0', sizeof (Option_t));

         if (toc->hdr_fmt == TOC_FMT_3_1)
         {
            n = sscanf (buf, _3_1_TAPE_ENTRY_FMT, 
                        op.name,        /* LPP.OPTION   */
                        level,          /* Level        */
                        &(op.quiesce),  /* Quiesce      */
                        &(op.content),  /* Content      */
                        op.lang,        /* Language     */
                        description);   /* description  */

            if (n < ITEMS_IN_3_1_TAPE_ENTRY)
               break;
         }
         else
         {
            /*-----------------------------------------------------------*
             * ELSE, This is a 3.2 format or greater.
             * In 3.2 the option line is the same no matter what type of
             * media being dealt with. In 3.1 the the location field did
             * not exist for tape and preload media, but did for diskette.
             *-----------------------------------------------------------*/

            n = sscanf (buf, SINGLE_BFF_ENTRY_FMT, 
                        op.name,        /* LPP.OPTION   */
                        level,  /* Level        */
                        location,       /* Location (ignored for now) */
                        &(op.quiesce),  /* Quiesce      */
                        &(op.content),  /* Content      */
                        op.lang,        /* Language     */
                        description);   /* Description  */

            if (n < ITEMS_IN_SINGLE_BFF_ENTRY)
               break;
         } /* end if */

         /*---------------------------------------------------------------*
          * Convert the level ascii string into the Level_t struct format.
          *---------------------------------------------------------------*/

         if ((rc = inu_toc_level_convert(level,&op,bff.fmt,action)) != SUCCESS)

         {
            FLOP_RETURN;
         } /* end if */

         /*---------------------------------------------------------------*
          * Create new option
          *---------------------------------------------------------------*/

         if ((nu_op = create_option (op.name, 
                                     NO, 
                                     op.quiesce, 
                                     op.content, 
                                     op.lang, 
                                     &op.level, 
                                     description, 
                                     nu_bff)) == NIL (Option_t))
         {
            FLOP_RETURN;
         }

         /*---------------------------------------------------------------*
          * Insert the new option into the linked list
          *---------------------------------------------------------------*/

         (void) insert_option (toc->options, nu_op);

         /*---------------------------------------------------------------*
          * Set the content field. (Convert the 3.1 content values to 3.2)
          *---------------------------------------------------------------*/

         if ((rc = set_content (nu_op)) != SUCCESS)
         {
            FLOP_RETURN;
         }

         /*---------------------------------------------------------------*
          * Set the option member vpd_tree based on the content field
          *---------------------------------------------------------------*/

         (void) set_vpdtree (nu_op);

         /*---------------------------------------------------------------*
          * Set the option member op_type based on the whether this is
          * an update or an installation and whether is for 3.1 or not.
          *---------------------------------------------------------------*/

         (void) set_op_type (nu_op);

        /*------------------------------------------------------------------*
         * If no value for the product name field exists, then one wasn't
         * required (cuz we're not dealing with a 4.1 image).  So, just use
         * "up to the 1st dot" method for the product name.
         * Else  a product name was given,  so use it.
         *------------------------------------------------------------------*/

         if (productname [0]  == '\0')
         {
            if (inu_set_prod_name (nu_op, prodname) != SUCCESS)
            {
               FLOP_RETURN;
            }
         }
         else if (inu_set_prod_name (nu_op, productname) != SUCCESS)
         {
            FLOP_RETURN;
         }

         /*---------------------------------------------------------------*
          * Create a new option reference list for this bff
          *---------------------------------------------------------------*/

         if ((nu_opref = create_optionref (nu_op)) == NIL (OptionRef_t))
         {
            FLOP_RETURN;
         }

         /*---------------------------------------------------------------*
          * Insert option reference into linked list for this bff
          *---------------------------------------------------------------*/

         (void) insert_optionref (op_ref_head, nu_opref);

         /*---------------------------------------------------------------*
          * Read the next line from the toc file, it could be the beginning
          * of option info (prereq, size, supersede) or it could be another
          * lpp-option line, or it could be the end of the option list "}".
          *---------------------------------------------------------------*/

         if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            FLOP_RETURN;
         }

         /*---------------------------------------------------------------*
          * Test to see if it's prereq, size, and/or supersede info for
          * this option: The beginning of this info is indicated by the
          * "[\n", the end of this information is indicated by a "]\n".
          * NOTE: The brace must be in column one.
          *---------------------------------------------------------------*/

         if (buf[0] == '['  &&  buf[1] == '\n')
         {
            if ((rc = get_opt_info (nu_op, media_ptr)) != SUCCESS)
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                            CMN_BAD_TOC_D), inu_cmdname);
               FLOP_RETURN;
            }
            /*-----------------------------------------------------------*
             * get_opt_info () just read the ']' (ending delimiter for the
             * option info) , now read the next line from the toc file
             *-----------------------------------------------------------*/

            if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                            CMN_BAD_TOC_D), inu_cmdname);
               FLOP_RETURN;
            }

         }

         /*---------------------------------------------------------------*
          * If we find a right curly brace, then we're at the end of
          * the list of options for this bff, so break out of this loop.
          *---------------------------------------------------------------*/

      } while ( ! (buf[0] == '}'  &&  buf[1] == '\n'));

      /*----------------------------------------------------------------------*
       * Read the next bff header line from the toc file.
       * if we get a NIL (char) then we're done with this toc file.
       *----------------------------------------------------------------------*/

   } while ((fgets (buf, TMP_BUF_SZ, media_ptr)) != NIL (char));

   /*-----------------------------------------------------------------------*
    * skip to the next tape mark if we aren't already there.
    *-----------------------------------------------------------------------*/

   fclose (media_ptr);
   unlink (".toc");

   return (SUCCESS);

} /* end load_stack_disk_toc */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    load_bff_toc
**
** DESCRIPTION: Restores 'lpp_name' from a bff and constructs a single LPP TOC
**              from the contents of this file.
**
** PARAMETERS:  media   - Name of the input device.
**
** NOTE:        This routine can be called from open_toc () when it is trying to
**              build the initial linked toc list. And it can be called from
**              the command inutoc when it is trying to add a new bff into the
**              linked toc list.
**
** RETURNS:     SUCCESS     - Funcion completed successfully.
**              FAILURE     - Funcion failed, but the toc linked list hadn't
**                            been modified yet.
**              TOC_FAILURE - Funcion failed, AND the toc linked list had been
**                            modified.  This is important when called from the
**                            command inutoc because it needs to cleanup the
**                            link list from the failed operation.  If this is
**                            called from open_toc () the cleanup isn't
**                            necessary because the calling routine terminates
**                            if an error occurs.
**
**--------------------------------------------------------------------------*/

int load_bff_toc (TOC_t * toc,     /* Pointer to the top of the toc link list */
                  char  * media,   /* file name of media */
                  int     class,   /* S_IFREG-regular file, or S_IFCHR-char
                                    * device */
                  int     q_flag,  /* whether the -q flag was specified */
                  int     caller)  /* Who called me, open_toc () function or
                                    * inutoc command */
{
   char            description[BUFSIZ];
   int             n;               /* Return value from scanf routines */
   FILE           *media_ptr;       /* The FILE ptr to the open of the tape
                                     * device */
   BffRef_t        bff;             /* A temporary bff structure for reading in
                                     * bff */
   BffRef_t       *nu_bff;          /* The bff ptr returned from the
                                     * create_bffref  */
   Option_t        op;              /* A temp opt struct for reading in
                                     * options */
   Option_t       *nu_op;           /* The opt ptr returned from the
                                     * create_option */
   OptionRef_t    *op_ref_head;     /* The ptr to the anchor of the bff link
                                     * list */
   int             rc;              /* generic return code integer */
   char            buf[TMP_BUF_SZ]; /* temp buffer for receiving items
                                     * from the toc */
   char            action[TMP_BUF_SZ]; /* temp char to receive action char from
                                        * toc */
   char            location[10];    /* temp string to receive loc string
                                     * from toc   */
   char            level[100];      /* temp string to receive loc string from
                                     * toc */
   int             new_q_flag;      /* The flag indicating to inu_restore
                                     * whether to prompt or not, based on the
                                     * value of q_flag and whether a tape
                                     * device is used. */
   char            prodname[MAX_LPP_FULLNAME_LEN]; /* product name to which
                                                    * lpp belongs */
   char            productname[MAX_LPP_FULLNAME_LEN]; /* new prodname field */
   char            *q;  /* temp pointer */

   /*-----------------------------------------------------------------------*
    * If this is a character device, do not prompt again ..........
    *-----------------------------------------------------------------------*/


   if (class == S_IFCHR)
      new_q_flag = TRUE;
   else
      new_q_flag = q_flag;

   if ((rc = inu_restore (media, new_q_flag, TYPE_DISK, NULL, 
                          "./lpp_name")) != SUCCESS)
   {
      if (caller != CALLER_INUTOC)
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                      CMN_BAD_DEV_D), inu_cmdname, media);
      return (FAILURE);
   }

   if ((media_ptr = fopen ("./lpp_name", "r")) == NIL (FILE))
   {
      if (caller != CALLER_INUTOC)
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                      CMN_BAD_DEV_D), inu_cmdname, media);
      return (FAILURE);
   }

   toc->vol = 0;      /* Volume      */
   toc->dt = 0;       /* Date of creation    */
   toc->hdr_fmt = TOC_FMT_NONE;       /* Header format   */
   toc->type = TYPE_DISK;

   /*-----------------------------------------------------------------------*
    * Read the Bff header line from the lpp_name file.
    *-----------------------------------------------------------------------*/

   bff.vol         =   0;
   bff.off         =   0;
   productname [0] = '\0';

   n = fscanf (media_ptr, "%c %c %s %[^ {]", 
               &(bff.fmt),          /* Format                       */
               &(bff.platform),     /* Platform                     */
               action,              /* Action                       */
               &(productname[0]));  /* product name required in 4.1 */

  /*-----------------------------------------------------------------*
   * No product name in the lpp_name file of a 4.1 image is an error.
   *-----------------------------------------------------------------*/
   if (bff.fmt == FMT_4_1    &&    n < ITEMS_IN_SINGLE_BFF + 1)
   {
      if (caller != CALLER_INUTOC)
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MISS_PROD_NAME_E, 
                                      CMN_MISS_PROD_NAME_D), inu_cmdname);
      return (FAILURE);
   }

   else if (bff.fmt < FMT_4_1)
   {
      if (n < ITEMS_IN_SINGLE_BFF)
      {
         if (caller != CALLER_INUTOC)
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                         CMN_BAD_DEV_D), inu_cmdname, media);
         return (FAILURE);
      }
   }



   /*-----------------------------------------------------------------------*
    * Set the bff action constant based on code in TOC
    *-----------------------------------------------------------------------*/

   if (set_action (&(bff.action), action) != SUCCESS)
      return (FAILURE);

   /*-----------------------------------------------------------------------*
    * Create and add the BffRef struct to the toc linked list.
    *-----------------------------------------------------------------------*/

   if ((nu_bff = create_bffref (bff.vol, 
                                bff.off, 
                                bff.size, 
                                (int) bff.fmt, 
                                bff.platform, 
                                media, 
                                bff.crc, 
                                bff.action)) == NIL (BffRef_t))
   {
      return (FAILURE);
   }

   if (toc_caller == CALLER_INUTOC)
    {
      nu_bff->action_string = malloc (strlen (action) + 1);
      if (nu_bff->action_string == NIL (char))
       {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
       }
      strcpy (nu_bff->action_string, action);
    }

   (void) insert_bffref (toc->content, nu_bff);

   /*-----------------------------------------------------------------------*
    * Create a option reference for this bff to point to the next option
    *-----------------------------------------------------------------------*/

   if ((op_ref_head = create_optionref (NIL (Option_t))) == NIL (OptionRef_t))
      return (TOC_FAILURE);
   nu_bff->options = op_ref_head;

   /*-----------------------------------------------------------------------*
    * skip past the left curly brace at the end of the bff header line.
    *-----------------------------------------------------------------------*/

   if ((n = fscanf (media_ptr, "%[ \t{\n]", buf)) < 1)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, CMN_BAD_DEV_D), 
                                   inu_cmdname, media);
      return (TOC_FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Read the first option line from the toc file.
    *-----------------------------------------------------------------------*/

   if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, CMN_BAD_DEV_D), 
                                   inu_cmdname, media);
      return (TOC_FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Read the options which comprise this backup-format file.
    *-----------------------------------------------------------------------*/

   inu_set_prod_name (NIL (Option_t), prodname);
   do
   {
      memset (&op, '\0', sizeof (Option_t));

      n = sscanf (buf, SINGLE_BFF_ENTRY_FMT, 
                  op.name,         /* LPP.OPTION   */
                  level,           /* Level        */
                  location,        /* Location (ignored for now) */
                  &(op.quiesce),   /* Quiesce      */
                  &(op.content),   /* Content      */
                  op.lang,         /* Language     */
                  description);    /* Description  */

      if (n < ITEMS_IN_SINGLE_BFF_ENTRY)
         break;

      /*-------------------------------------------------------------------*
       * if there is a comment in the description, remove it
       *-------------------------------------------------------------------*/

      for (n = 0;
           (description[n] != '#'  &&  description[n] != '\0');
           n++)
      {
        ;
      } /* end for */

      if (description[n] == '#')
         description[n] = '\0';

      for ( q=description; *q == ' ' || *q == '\t' ; q++);
      if( *q == '\0' )
          strcpy(description,"Description is not available");

      /*-------------------------------------------------------------------*
       * Convert the level ascii string into the Level_t struct format.
       *-------------------------------------------------------------------*/

      if ((rc = inu_toc_level_convert(level,&op,bff.fmt,action)) != SUCCESS)
         return (TOC_FAILURE);

      /*-------------------------------------------------------------------*
       * Create new option and add to option link list
       *-------------------------------------------------------------------*/

      if ((nu_op = create_option (op.name, 
                                  NO, 
                                  op.quiesce, 
                                  op.content, 
                                  op.lang, 
                                  &op.level, 
                                  description, 
                                  nu_bff)) == NIL (Option_t))
         return (TOC_FAILURE);
      (void) insert_option (toc->options, nu_op);

      /*-------------------------------------------------------------------*
       * Set the content field. (Convert the 3.1 content values to 3.2)
       *-------------------------------------------------------------------*/

      if ((rc = set_content (nu_op)) != SUCCESS)
         return (TOC_FAILURE);

      /*-------------------------------------------------------------------*
       * Set the option member vpd_tree based on the content field
       *-------------------------------------------------------------------*/

      (void) set_vpdtree (nu_op);

      /*-------------------------------------------------------------------*
       * Set the option member op_type based on the whether this is
       * an update or an installation and whether is for 3.1 or not.
       *-------------------------------------------------------------------*/

      (void) set_op_type (nu_op);

     /*-------------------------------------------------------------------*
      * If no value for the product name field exists, then one wasn't
      * required (cuz we're not dealing with a 4.1 image).  So, just use
      * the "up to the 1st dot" method for the product name.
      * Else  a product name was given,  so use it.
      *-------------------------------------------------------------------*/

      if (productname [0]  == '\0')
      {
         if (inu_set_prod_name (nu_op, prodname) != SUCCESS)
            return (TOC_FAILURE);
      }
      else if (inu_set_prod_name (nu_op, productname) != SUCCESS)
      {
         return (TOC_FAILURE);
      }

      /*-------------------------------------------------------------------*
       * Add new option to option reference list for this bff
       *-------------------------------------------------------------------*/

      (void) insert_optionref (op_ref_head, create_optionref (nu_op));

      /*-------------------------------------------------------------------*
       * Read the next line from the toc file, it could be the beginning
       * of option info (prereq, size, supersede) or it could be another
       * lpp-option line, or it could be the end of the option list "}".
       *-------------------------------------------------------------------*/

      if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                      CMN_BAD_DEV_D), inu_cmdname, media);
         return (TOC_FAILURE);
      }

      /*-------------------------------------------------------------------*
       * Test to see if it's prereq, size, and/or supersede info for
       * this option: The beginning of this info is indicated by the
       * "[\n", the end of this information is indicated by a "]\n".
       * NOTE: The brace must be in column one.
       *-------------------------------------------------------------------*/

      if (buf[0] == '['  &&  buf[1] == '\n')
      {
         if ((rc = get_opt_info (nu_op, media_ptr)) != SUCCESS)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                         CMN_BAD_DEV_D), inu_cmdname, media);
            return (TOC_FAILURE);
         }

         /*---------------------------------------------------------------*
          * get_opt_info () just read the ']' (ending delimiter for the
          * option info) , now read the next line from the toc file.
          *---------------------------------------------------------------*/

         if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
                                         CMN_BAD_DEV_D), inu_cmdname, media);
            return (TOC_FAILURE);
         }
      }

      /*-------------------------------------------------------------------*
       * If we find a right curly brace, then we're at the end of
       * the list of options for this bff, so break out of this loop.
       *-------------------------------------------------------------------*/

   } while ( ! (buf[0] == '}'  &&  buf[1] == '\n'));

   fclose (media_ptr);
   unlink ("./lpp_name");

   return (SUCCESS);

} /* end load_bff_toc */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     load_disk_toc
**
** DESCRIPTION:  Reads a .toc file from the path specified in 'media'.
**
** PARAMETERS:   media   - Name of the input device.
**
** RETURNS:      Pointer to a table of contents or nil if malformed .toc file.
**
**--------------------------------------------------------------------------*/

int load_disk_toc (TOC_t * toc,   /* Pointer to the top of the toc link list */
                   char  * media) /* file name of media */
{
   char            description[BUFSIZ];
   int             n;           /* return value from scanf routines */
   FILE           *media_ptr;         /* The FILE ptr to the open of the tape
                                       * device */
   char            toc_file[PATH_MAX];
   int             sort_vol;
   BffRef_t        bff;              /* a temporary bff structure for reading
                                      * in bff */
   char            bff_path[PATH_MAX];
   BffRef_t       *nu_bff;           /* the bff ptr returned from the
                                      * create_bffref */
   Option_t        op;               /* a temp opt struct for reading in
                                      * options */
   Option_t       *nu_op;            /* the opt ptr returned from the
                                      * create_option */
   OptionRef_t    *op_ref_head;      /* The ptr to the anchor of the bff link
                                      * list */
   OptionRef_t    *nu_opref;         /* The ptr generic OptionRef_t structure */
   int             rc;               /* generic return code integer */
   char            buf[TMP_BUF_SZ];  /* temp buffer for receiving items
                                      * from the toc */
   char            action[TMP_BUF_SZ]; /* temp char to receive action char from
                                        * toc */
   char            location[10];     /* temp string to receive loc string
                                      * from toc   */
   char            hdr_fmt[10];      /* temp string to receive hdr_fmt str from
                                      * toc */
   char            level[100];       /* temp string to receive loc string from
                                      * toc */
   char            prodname[MAX_LPP_FULLNAME_LEN];  /* product name to which
                                                     * lpp belongs */
   char            productname[MAX_LPP_FULLNAME_LEN]; /* new prodname field */


   bff.vol = 0;  /* Not used for this media. */
   bff.off = 0;  /* Not used for this media. */
   bff.size = 0; /* Not used for this media. */
   bff.crc = 0;  /* Not used for this media. */
   bff.options = NIL (OptionRef_t);
   bff.next = NIL (BffRef_t);

   /*-----------------------------------------------------------------------*
    * Set up the path to the .toc file in the requested directory.
    *-----------------------------------------------------------------------*/

   strncpy (toc_file, media, PATH_MAX);
   if (media[strlen (media) - 1] == '/')
      strncat (toc_file, ".toc", PATH_MAX - strlen (toc_file));
   else
      strncat (toc_file, "/.toc", PATH_MAX - strlen (toc_file));

   /*-----------------------------------------------------------------------*
    * Set toc type to hard disk.
    *-----------------------------------------------------------------------*/

   toc->type = TYPE_DISK;

   /*-----------------------------------------------------------------------*
    * Open the .toc file for reading, 
    *-----------------------------------------------------------------------*/

   if ((media_ptr = fopen (toc_file, "r")) == NIL (FILE))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, CMN_BAD_DEV_D), 
                                   inu_cmdname, media);
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Read the toc header information.
    * The 3.1 preload format is different then the 3.1 tape toc header and
    * any 3.2 toc format. (The 3.1 tape toc header format is the same as
    * the 3.2 toc format.)
    *-----------------------------------------------------------------------*/

   if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Check the first field, if it is a "0" then this is a 3.2 toc file, 
    * if it is a field that is > 1 character in length then it must be a 3.1
    * preload toc file. Based on this, use the correct scanf format string.
    *-----------------------------------------------------------------------*/

   if (buf[0] == '0'  &&  (buf[1] == '\t'  ||  buf[1] == ' '))
   {
      n = sscanf (buf, TAPE_HEADER_FMT, 
                  &(toc->vol),        /* Volume           */
                  &(toc->dt), /* Date of creation */
                  hdr_fmt);     /* Header format    */

      if (n != ITEMS_IN_TAPE_HDR)
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         return (FAILURE);
      }
   }
   else
   {
      /*-------------------------------------------------------------------*
       * ELSE, This is a 3.1 preload toc.
       *-------------------------------------------------------------------*/

      n = sscanf (buf, PRELOAD_HEADER_FMT, 
                  &(toc->dt), /* Date of creation */
                  hdr_fmt);     /* Header format    */

      if (n != ITEMS_IN_PRELOAD_HDR  ||  hdr_fmt[0] != '1')
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         return (FAILURE);
      }
   } /* end if */

   /*-----------------------------------------------------------------------*
    * Set the header format to the integer value in the toc structure.
    *-----------------------------------------------------------------------*/

   if ((rc = set_hdr_fmt (&(toc->hdr_fmt), hdr_fmt)) != SUCCESS)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    * Read the first bff header line from the toc file.
    *-----------------------------------------------------------------------*/

   if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);
      return (FAILURE);
   }

   /*-----------------------------------------------------------------------*
    *  Read each of the bffs in succession
    *-----------------------------------------------------------------------*/

   sort_vol = 0;
   do
   {
      char            bffname[PATH_MAX + 1];

      sort_vol++;

      /*-------------------------------------------------------------------*
       * Bff Header for preload disk
       *-------------------------------------------------------------------*/


      productname[0] = '\0';
      n = sscanf (buf,  "%s %c %c %s %[^ {]", 
                  bffname,              /* path to bff file    */
                  &(bff.fmt),           /* Format of bff spec  */
                  &(bff.platform),      /* Platform            */
                  action,               /* Action              */
                  &(productname[0]));   /* product name        */

      sprintf (bff_path, "%s/%s", media, bffname);

      if (bff.fmt != FMT_4_1)
      {
        productname[0] = '\0';  /* Ignore any product_name, it shouldn't be */
                                /* there, but don't get bent out of shape   */
                                /* if it is. */
      }
      else
      {
        /*-----------------------------------------------------------------*
         * No product name in the lpp_name file of a 4.1 image is an error.
         *-----------------------------------------------------------------*/
         if (n < ITEMS_IN_PRELOAD_BFF + 1)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MISS_PROD_NAME_E, 
                                         CMN_MISS_PROD_NAME_D), inu_cmdname);
            return (FAILURE);
         }
      }

      if (n < ITEMS_IN_PRELOAD_BFF)
         break;

      /*-------------------------------------------------------------------*
       * Set the bff action constant based on code in TOC
       *-------------------------------------------------------------------*/

      if (set_action (&(bff.action), action) != SUCCESS)
         return (FAILURE);

      /*-------------------------------------------------------------------*
       * Create and add the BffRef struct to the toc linked list.
       *-------------------------------------------------------------------*/

      if ((nu_bff = create_bffref (sort_vol, 
                                   0,   /* set offset to 0 */
                                   bff.size, 
                                   (int) bff.fmt, 
                                   bff.platform, 
                                   bff_path, 
                                   bff.crc, 
                                   bff.action)) == NIL (BffRef_t))
      {
         return (FAILURE);
      }

      if (toc_caller == CALLER_INUTOC)
       {
         nu_bff->action_string = malloc (strlen (action) + 1);
         if (nu_bff->action_string == NIL (char))
          {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                 CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
          }
         strcpy (nu_bff->action_string, action);
       }

      (void) insert_bffref (toc->content, nu_bff);

      /*-------------------------------------------------------------------*
       * Create a option reference for this bff to point to the next option
       *-------------------------------------------------------------------*/

      if ((op_ref_head = create_optionref (NIL (Option_t))) == NIL (OptionRef_t))
         return (FAILURE);
      nu_bff->options = op_ref_head;

      /*-------------------------------------------------------------------*
       * Read the first option line from the toc file.
       *-------------------------------------------------------------------*/

      if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                      CMN_BAD_TOC_D), inu_cmdname);
         return (FAILURE);
      }

      /*-------------------------------------------------------------------*
       * Process the options which makes up this backup-format file.
       *-------------------------------------------------------------------*/

      inu_set_prod_name (NIL (Option_t), prodname);
      do
      {
         memset (&op, '\0', sizeof (Option_t));

         /*---------------------------------------------------------------*
          * If this is a 3.1 preload toc file then use the scanf format
          * string that doesn't have the location field in it...
          *---------------------------------------------------------------*/

         if (toc->hdr_fmt == TOC_FMT_3_1)
         {
            n = fscanf (media_ptr, PRELOAD_ENTRY_FMT, 
                        op.name,        /* LPP.OPTION       */
                        level,  /* Level            */
                        &(op.quiesce),  /* Quiesce          */
                        &(op.content),  /* Content          */
                        op.lang,        /* Language         */
                        description);   /* Description      */

            if (n < ITEMS_IN_PRELOAD_ENTRY)
               break;
         }
         else
         {
            /*-----------------------------------------------------------*
             * ELSE, This is a 3.2 format or greater.
             * In 3.2 the option line is the same no matter what type of
             * media being dealt with. In 3.1 the the location field did
             * not exist for tape and preload media, but did for diskette.
             *-----------------------------------------------------------*/

            n = sscanf (buf, SINGLE_BFF_ENTRY_FMT, 
                        op.name,        /* LPP.OPTION   */
                        level,  /* Level        */
                        location,       /* Location (ignored for now) */
                        &(op.quiesce),  /* Quiesce      */
                        &(op.content),  /* Content      */
                        op.lang,        /* Language     */
                        description);   /* Description  */

            if (n < ITEMS_IN_SINGLE_BFF_ENTRY)
               break;
         } /* end if */

         /*---------------------------------------------------------------*
          * Convert the level ascii string into the Level_t struct format.
          *---------------------------------------------------------------*/

         if ((rc = inu_toc_level_convert(level,&op,bff.fmt,action)) != SUCCESS)
            return (FAILURE);

         /*---------------------------------------------------------------*
          * Create new option and add to option link list
          *---------------------------------------------------------------*/

         if ((nu_op = create_option (op.name, 
                                     FALSE, 
                                     op.quiesce, 
                                     op.content, 
                                     op.lang, 
                                     &op.level, 
                                     description, 
                                     nu_bff)) == NIL (Option_t))
            return (FAILURE);
         (void) insert_option (toc->options, nu_op);

         /*---------------------------------------------------------------*
          * Set the content field. (Convert the 3.1 content values to 3.2)
          *---------------------------------------------------------------*/

         if ((rc = set_content (nu_op)) != SUCCESS)
            return (FAILURE);

         /*---------------------------------------------------------------*
          * Set the option member vpd_tree based on the content field
          *---------------------------------------------------------------*/

         (void) set_vpdtree (nu_op);

         /*---------------------------------------------------------------*
          * Set the option member op_type based on the whether this is
          * an update or an installation and whether is for 3.1 or not.
          *---------------------------------------------------------------*/

         (void) set_op_type (nu_op);

        /*-----------------------------------------------------------------*
         * If no value for the product name field exists, then one wasn't
         * required (cuz we're not dealing with a 4.1 image).  So, just use
         * the "up to the 1st dot" method for the product name.
         * Else  a product name was given,  so use it.
         *-----------------------------------------------------------------*/

         if (productname [0]  == '\0')
         {
            if (inu_set_prod_name (nu_op, prodname) != SUCCESS)
               return (FAILURE);
         }
         else if (inu_set_prod_name (nu_op, productname) != SUCCESS)
         {
            return (FAILURE);
         }

         /*---------------------------------------------------------------*
          * Create a new option reference list for this bff
          *---------------------------------------------------------------*/

         if ((nu_opref = create_optionref (nu_op)) == NIL (OptionRef_t))
            return (FAILURE);

         /*---------------------------------------------------------------*
          * Insert option reference into linked list for this bff
          *---------------------------------------------------------------*/

         (void) insert_optionref (op_ref_head, nu_opref);

         /*---------------------------------------------------------------*
          * Read the next line from the toc file, it could be the beginning
          * of option info (prereq, size, supersede) or it could be another
          * lpp-option line, or it could be the end of the option list "}".
          *---------------------------------------------------------------*/

         if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            return (FAILURE);
         }

         /*---------------------------------------------------------------*
          * Test to see if it's prereq, size, and/or supersede info for
          * this option: The beginning of this info is indicated by the
          * "[\n", the end of this information is indicated by a "]\n".
          * NOTE: The brace must be in column one.
          *---------------------------------------------------------------*/

         if (buf[0] == '['  &&  buf[1] == '\n')
         {
            if ((rc = get_opt_info (nu_op, media_ptr)) != SUCCESS)
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                           CMN_BAD_TOC_D), inu_cmdname);
               return (FAILURE);
            }

            /*-----------------------------------------------------------*
             * get_opt_info () just read the ']' (ending delimiter for the
             * option info) , now read the next line from the toc file.
             *-----------------------------------------------------------*/

            if ((fgets (buf, TMP_BUF_SZ, media_ptr)) == NIL (char))
            {
               inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                            CMN_BAD_TOC_D), inu_cmdname);
               return (FAILURE);
            }
         }

         /*-------------------------------------------------------------------*
          * If we find a right curly brace, then we're at the end of
          * the list of options for this bff, so break out of this loop.
          *-------------------------------------------------------------------*/

      } while ( ! (buf[0] == '}'  &&  buf[1] == '\n'));

      /*----------------------------------------------------------------------*
       * Read the next bff header line from the toc file.
       * if we get a NIL (char) then we're done with this toc file.
       *----------------------------------------------------------------------*/

   } while ((fgets (buf, TMP_BUF_SZ, media_ptr)) != NIL (char));

   fclose (media_ptr);

   return (SUCCESS);

} /* end load_disk_toc */

/* Determines the number of blocks required to create the media resident form
 * of the internal TOC_t representation. */

int toc_size (TOC_t * toc)
{
   int             n_bytes;

   BffRef_t       *b;
   Option_t       *o;

   n_bytes = 2 + 1 + 2 + 1 + 1 + 1 + 12 + 1;

   for (b = toc->content->next; b != NIL (BffRef_t); b = b->next)
      n_bytes += 14;

   for (o = toc->options->next;
        o != NIL (Option_t);
        o = o->next)
   {
      n_bytes += strlen (o->name);
      n_bytes += strlen (o->lang);
      n_bytes += sizeof (o->level);
      n_bytes += 9;                  /* White space and fixed length fields */
   }

   if ((n_bytes % LRECL) != 0)
      n_bytes += LRECL;
   n_bytes /= LRECL;

   return n_bytes;      /* n_bytes is now in blocks !    */

} /* end toc_size */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     set_hdr_fmt
**
** DESCRIPTION:  Sets the header format field (hdr_fmt) to the integer value
**               for the option structure.
**
** PARAMETERS:   hdr_fmt - pointer to integer to contain hdr_fmt code.
**               code    - character code to translate.
**
**--------------------------------------------------------------------------*/

int set_hdr_fmt (int  * hdr_fmt, /* Ptr to the int in the toc structure
                                  * member hdr_fmt */
                 char * string)  /* Ptr to the char string received from the
                                 * toc file */
{
   /*-----------------------------------------------------------------------*
    * Set the (integer) hdr_fmt field of the toc ptr to the value that
    * indicates which toc format we're dealing with (3.1, 3.2, or 4.1).
    * If the hdr value isn't a recognized value, return FAILURE.
    *-----------------------------------------------------------------------*/

   switch (string[0])
   {
      case '1':  *hdr_fmt = TOC_FMT_3_1;       /* Release 3.1 format */
                 break;

      case '2':  *hdr_fmt = TOC_FMT_3_2;       /* Release 3.2 format */
                 break;

      case '3':  *hdr_fmt = TOC_FMT_3_2; /* Temporary, waiting on build tools */
                 break;

      default:   return (FAILURE);

   } /* switch */

   return (SUCCESS);

} /* end set_hdr_fmt */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     set_action
**
** DESCRIPTION:  Sets action based on character code on media.
**
** PARAMETERS:   action  - pointer to integer to contain action code.
**               code    - character code to translate.
**
**--------------------------------------------------------------------------*/

static int set_action (int  * action,  /* Ptr to the int in the bff structure
                                        * member action */
                       char * code)    /* Ptr to the character receive from
                                        * the toc file */
{
   if (strlen (code) > 2)
    {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D),
                                   inu_cmdname);
      return (FAILURE);
    }

   switch (code[0])
   {
      case 'I':
      case 'i':
         *action = ACT_INSTALL;
         if (code[1] != '\0')
            * action = ACT_UNKNOWN;
         break;
      case 'G':
      case 'g':
         *action = ACT_GOLD_UPDT;
         if (code[1] != '\0')
            * action = ACT_UNKNOWN;
         break;
      case 'M':
      case 'm':
         switch (code[1])
         {
            case '\0':
               * action = ACT_MULT_UPDT;
               break;
            case 'C':
               *action = ACT_CUM_UPDT;
               break;
            case 'E':
               * action = ACT_EN_PKG_UPDT;
               break;
            case 'L':
               * action = ACT_MAINT_LEV_UPDT;
               break;
            default:
               * action = ACT_UNKNOWN;
         } /* end switch */
         break;
      case 'O':
      case 'o':
         *action = ACT_OTHER; 
         break;
      case 'S':
      case 's':
         switch (code[1])
          {
            case '\0':
               *action = ACT_SING_UPDT;
               break;
/*          case 'e':
               *action = ACT_EN_MEM_UPDT;
               break;                           */
            case 'F':
               *action = ACT_INSTALLP_UPDT;
               break;
            case 'R':
               *action = ACT_REQUIRED_UPDT;
               break;
            default:
               *action = ACT_UNKNOWN;
           }
          break;

       default:
         *action = ACT_UNKNOWN;
   }
   return (SUCCESS);

} /* end set_action */


/*--------------------------------------------------------------------------*
**
** FUNCTION:     set_content
**
** DESCRIPTION:  Sets the option structure member content to what has been
**               defined to 3.2
**
** PARAMETERS:   op  - pointer to the option structure.
**
**--------------------------------------------------------------------------*/

int set_content (Option_t * op)      /* Ptr to the option structure. */
{
   char    lppname[PATH_MAX * 2];   /* also for the warning message printed
                                     * out */

   switch (op->content)
   {
         /**
          **  All three of these guys imply we're dealing with 3.1
          **/

      case CONTENT_MRI:
      case CONTENT_PUBS:
      case CONTENT_OBJECT:
         /**
          **  Check for an inconsistency between the format field
          **  (indicates level -- 3.1 or 3.2 -- of installp) and the
          **  content field ('O', 'P', and 'S'  ==> 3.1
          **                  everything else   ==> 3.2).
          **
          **  If an inconsistency exists, fix it by asuming 'U' for the
          **  content field and give a warning message to the effect of
          **  "inconsistency, but continuing anyway."
          **/

         if (op->bff->fmt == FMT_3_2)
         {
            inu_get_lpplevname_from_Option_t (op, lppname);
            inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_BD_CNTFLD_W, 
                                         CMN_BD_CNTFLD_D), inu_cmdname, 
                                         op->content, lppname, CONTENT_USR);
         }
         op->content = CONTENT_USR;
         break;
      case CONTENT_BOTH:
      case CONTENT_MCODE:
      case CONTENT_SHARE:
      case CONTENT_USR:
         break;
      default:
         if (op->bff->fmt == FMT_3_2)
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                         CMN_BAD_TOC_D), inu_cmdname);
            return (FAILURE);
         }
         inu_get_lpplevname_from_Option_t (op, lppname);
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BD_CNTFLD_W, 
                                      CMN_BD_CNTFLD_D), inu_cmdname, 
                                      op->content, lppname, CONTENT_USR);
         op->content = CONTENT_USR;
   }

   return (SUCCESS);

} /* end set_content */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     set_vpdtree
**
** DESCRIPTION:  Sets the option structure member vpdtree based on the content
**               field.
**
** PARAMETERS:   op  - pointer to the option structure.
**
**--------------------------------------------------------------------------*/

void set_vpdtree (Option_t *op)      /* Ptr to the option structure. */
{
   switch (op->content)
   {
         case CONTENT_SHARE:
         op->vpd_tree = VPDTREE_SHARE;
         break;
      case CONTENT_BOTH:
      case CONTENT_USR:
      default:
         op->vpd_tree = VPDTREE_USR;
         break;
   }
} /* end set_vpdtree */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     set_op_type
**
** DESCRIPTION:  Set the op_type field based on whether this option is an
**               update or an installation and whether or not it is 3.1 media
**               or not.
**
** PARAMETERS:   op  - pointer to the option structure.
**
**--------------------------------------------------------------------------*/

void set_op_type (Option_t * op)      /* Ptr to the option structure. */
{
   op->op_type = 0;

   switch (op->bff->fmt)
   {
      case FMT_3_1:
      case FMT_3_1_1:
                     op->op_type = OP_TYPE_3_1;
                     break;
      case FMT_3_2:  op->op_type = OP_TYPE_3_2;
                     break;
      case FMT_4_1:  op->op_type = OP_TYPE_4_1;
                     break;
      default:
                     break;
   }

   if (IF_ACT_UPDATE (op->bff->action))
   {
      op->op_type |= OP_TYPE_UPDATE;

      switch (op->bff->action)
      {
         case (ACT_CUM_UPDT):
               op->op_type |= OP_TYPE_C_UPDT;
               break;

         case (ACT_EN_PKG_UPDT):
               op->op_type |= OP_TYPE_E_UPDT;
               break;

         case (ACT_MULT_UPDT):
               op->op_type |= OP_TYPE_M_UPDT;
               break;

         default:
               break;
      }
   }

   if (op->bff->action == ACT_INSTALL)
      op->op_type |= OP_TYPE_INSTALL;

   /*
    * AIX 3.2.4 Overloads the "quiesce" field with the values    
    * ('B' Quiesce and perform bosboot; 'b' No quiesce but perform bosboot).
    * I perform the mapping between the character to a bit representation
    * here and in inu_toc_vpd.c set_cp_flag ().
    */
   if (QUIESCE_TO_BOSBOOT (op->quiesce))
      op->op_type |= OP_TYPE_BOSBOOT;

} /* end set_op_type */

/*--------------------------------------------------------------------------*
**
** FUNCTION:     get_opt_info
**
** DESCRIPTION:  This routine is called if it was detected that there is
**               prereq, size, and/or supersede info in the toc file for this
**               option.  The next line to read in the toc file should be the
**               prereq info or a line with a percent sign (in the case that
**               there aren't any prereq info).
**
** PARAMETERS:   toc - pointer to the anchor of the toc link list.
**
**--------------------------------------------------------------------------*/

int get_opt_info (Option_t * op,        /* Ptr to Option that has option info
                                         * in the toc for */
                  FILE     * media_ptr) /* FILE ptr to the toc file */
{
   int  bytes_needed;
   int  rc;
   char levname [MAX_LPP_FULLNAME_LEN+MAX_LEVEL_LEN+1];


   /*-----------------------------------------------------------------------*
    * Read the prereq info from the toc file to the option structure
    *-----------------------------------------------------------------------*/

   if ((rc = read_opt_info (&op->prereq, media_ptr)) == FAILURE)
      return (FAILURE);

   if (rc == R_BRACE_FOUND)
      return (SUCCESS);

   /*-----------------------------------------------------------------------*
    * Read the size info from the toc file to the option structure
    *-----------------------------------------------------------------------*/

   if ((rc = read_opt_info (&op->size, media_ptr)) == FAILURE)
      return (FAILURE);

   if (rc == R_BRACE_FOUND)
      return (SUCCESS);

   /*-----------------------------------------------------------------------*
    * Read the supersede info from the toc file to the option structure
    *-----------------------------------------------------------------------*/

   if ((rc = read_opt_info (&op->supersedes, media_ptr)) == FAILURE)
      return (FAILURE);

  /** ------------------------------------------------------------------- *
   **  Make sure 4.1 updates do NOT contain supersedes info.
   ** ------------------------------------------------------------------- */

   if (IF_4_1(op->op_type)     &&
       IF_UPDATE(op->op_type)  &&
       *op->supersedes != '\0')
   {
      inu_get_optlevname_from_Option_t (op, levname);

      inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_41_UPDT_HAS_SEDES_E,
                                 INP_41_UPDT_HAS_SEDES_D), levname );

      inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_SEDES_IN_41_UPDT_E,
                                 INP_NO_SEDES_IN_41_UPDT_D));
      return (FAILURE);
   }


   if (rc == PERCENT_FOUND)
   {  
     /**--------------------------------------------------------------------*
      **  Read the netls info from the toc file into the option structure.
      **--------------------------------------------------------------------*/

      if ((rc=read_netls_info(op, media_ptr)) != R_BRACE_FOUND
                             &&            rc != PERCENT_FOUND)
         return (FAILURE);
   }

  /** ---------------------------------------------------------------------*
   **  For the instfix and inutoc commands only, read the apar info 
   **  from the toc file to the option structure
   ** ---------------------------------------------------------------------*/

   if (rc != R_BRACE_FOUND)
 
      if (toc_caller == CALLER_INSTFIX || toc_caller == CALLER_INUTOC)

         if ((rc = read_opt_info (&op->fixdata, media_ptr)) == FAILURE)
            return (FAILURE);


  /*-----------------------------------------------------------------------*
   * If we're not at a ']', we need to skip down to the next ']' or '}'.
   * This allows flexibility to add to the toc format between the last
   * defined '%'-delimited section and the ']'.
   *-----------------------------------------------------------------------*/

   if (rc != R_BRACE_FOUND)
   {
      rc = skip_to_brace(media_ptr);

      if (rc != R_BRACE_FOUND)
         return (FAILURE);
   }

   return (SUCCESS);
} 

/*--------------------------------------------------------------------------*
**
** FUNCTION:    read_opt_info
**
** DESCRIPTION: Receives the address to a pointer to a buffer in the option
**              structure, this function mallocs memory for the pointer and
**              reads the information from the toc file and puts it into the
**              malloc'ed buffer.  It knows when to stop reading by detecting
**              the '%' delimiter or the ending ']' delimiter.
**
**              The info being read is prereq, size, and/or supersede info for
**              this option.
**                Example:
**                        [\n
**                        <prereq info>
**                        %\n
**                        <size info>
**                        %\n
**                        <supersede info>
**                        ]\n
**
**              NOTE: The brace and percent sign must be in column one.
**
** RETURNS:     FAILURE       - means a fatal error occurred like the malloc
**                              failed.
**              PERCENT_FOUND - means the '%' delimiter was detected (end of
**                              this part).
**              R_BRACE_FOUND - means the ']' delimiter was detected (end of
**                              info).
**
** PARAMETERS:  mbuf      - the address to a pointer to buffer in the option
**                          structure.
**              media_ptr - FILE pointer to the toc file.
**
**--------------------------------------------------------------------------*/

int read_opt_info (char ** mbuf,       /* Ptr to Option that has option info in
                                        * the toc for */
                   FILE  * media_ptr)  /* FILE ptr to the toc file */
{
          char            buf[TMP_BUF_SZ];
          int             first_time;
          int             i;
   static char          * scratch_buffer;
   static int             scratch_buffer_size = 0;

   if (scratch_buffer_size == 0)
    {
      scratch_buffer = malloc (MEM_BUF_SZ);
      if (scratch_buffer == NIL (char))
       {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
       }
    }

   scratch_buffer[0] = NUL;

   /*-----------------------------------------------------------------------*
    * Read in the next field into the malloc'ed buffer
    * If we run out of memory realloc some more memory...
    *-----------------------------------------------------------------------*/

   while (fgets (buf, TMP_BUF_SZ, media_ptr) != NIL (char))
   {
      if (  (strlen (buf) == 2)        /* Have we reached the end? */
                      && 
           (buf[0]=='%' || buf[0]==']'))

      {
         * mbuf = strdup (scratch_buffer);
         if (* mbuf == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                         CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
         }

         if (buf[0] == '%')
            return (PERCENT_FOUND);
         else
            return (R_BRACE_FOUND);
      }

      /*-------------------------------------------------------------------*
       * If we are out of memory realloc some more.
       *-------------------------------------------------------------------*/

      if ((strlen (buf) + strlen (scratch_buffer)) >= scratch_buffer_size)
      {
         scratch_buffer_size += MEM_BUF_SZ;
         scratch_buffer = realloc (scratch_buffer, scratch_buffer_size);
         if (scratch_buffer == NIL (char))
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                         CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
         }
      }
      strcat (scratch_buffer, buf);
   } 

   /*-----------------------------------------------------------------------*
    * IF we got here then there is a format error in the toc file.
    *-----------------------------------------------------------------------*/

   return (FAILURE);

} 


/*--------------------------------------------------------------------------*
**
** FUNCTION      read_netls_info 
**
** DESCRIPTION   Reads any netls info into the sop member. 
**
** Returns       R_BRACE_FOUND - upon success, and ']' found after netls info
**               PERCENT_FOUND - upon success,
**                    Either no netls info or successfully parsed it
**               FAILURE       - if any parsing problem is encountered
** 
** PARAMETERS    op - the sop member to read netls info into
**               fp - toc or lpp_name file
**
**--------------------------------------------------------------------------*/

int read_netls_info (Option_t *op, FILE *fp)
{
   char    buf[TMP_BUF_SZ];      /* to parse netls file      */
   char    levname [PATH_MAX];   /* totally for msg purposes */

  /** ----------------------------------------------------------------- *
   **  Parse the 1st line of the Netls file (the vendor id).
   ** ----------------------------------------------------------------- */

   if ((fgets (buf, TMP_BUF_SZ, fp)) != NIL (char))
   {
     /** ------------------------------------------ *
      **  If we found a ']', there's no NetLs info 
      ** ------------------------------------------ */

      if (buf[0] == ']')
         return (R_BRACE_FOUND);
      if (buf[0] == '%')
         return (PERCENT_FOUND);

      if ((op->netls= (Netls_t *)malloc (sizeof (Netls_t) + 1)) == NIL (Netls_t)
                                 || 
        (op->netls->vendor_id= (char *)malloc (strlen (buf) + 1)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
      }
     /** ----------------------------------- *
      **  Eat the traling newline ... 
      ** ----------------------------------- */

      sscanf (buf, "%s", op->netls->vendor_id);
   }
   else
      return (FAILURE);

  /** ----------------------------------------------------------------- *
   **  Parse the 2nd line of the Netls file (the product id).
   ** ----------------------------------------------------------------- */

   if ((fgets (buf, TMP_BUF_SZ, fp)) != NIL (char))
   {
      if (buf[0] == ']')
      {
         inu_get_optlevname_from_Option_t (op, levname);
         inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_MISS_NETLS_PID_E, 
                                      INP_MISS_NETLS_PID_D), levname);
         return (FAILURE);
      }

      if ((op->netls->prod_id = (char *) malloc (strlen (buf) + 1)) 
                                                                 == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
      }
     /** ----------------------------------- *
      **  Eat the traling newline ... 
      ** ----------------------------------- */

      sscanf (buf, "%s", op->netls->prod_id);
   }
   else
      return (FAILURE);

  /** ----------------------------------------------------------------- *
   **  Parse the 3rd line of the Netls file (the product version).
   ** ----------------------------------------------------------------- */

   if ((fgets (buf, TMP_BUF_SZ, fp)) != NIL (char))
   {
      if (buf[0] == ']')
      {
         inu_get_optlevname_from_Option_t (op, levname);
         inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_MISS_NETLS_PVER_E, 
                                      INP_MISS_NETLS_PVER_D), levname);
         return (FAILURE);
      }

      if ((op->netls->prod_ver = (char *) malloc (strlen (buf) + 1)) == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
      }
     /** ----------------------------------- *
      **  Eat the traling newline ... 
      ** ----------------------------------- */

      sscanf (buf, "%s", op->netls->prod_ver);
   }
   else
      return (FAILURE);

  /** ----------------------------------------------------------------- *
   **  Should find a ']' or a '%' here ...  Error if we don't. 
   ** ----------------------------------------------------------------- */

   if ((fgets (buf, TMP_BUF_SZ, fp)) != NIL (char))
   {
      if (buf[0] == ']')
         return (R_BRACE_FOUND);
      else if (buf[0] == '%')
         return (PERCENT_FOUND);
   }

   return (FAILURE);
}

/*--------------------------------------------------------------------------*
**
** FUNCTION      inu_dump_lvl_msg
**
** DESCRIPTION   Prints out an error message about invalid level.
**
** NOTES         We assume that if we're called, op's level is incorrect.
**               And this implies that some error message concerning the
**               incorrect level needs to be given here.
**
** PARAMETERS
**
**--------------------------------------------------------------------------*/

void inu_dump_lvl_msg (Option_t * op, 
                       char     * level)
{
   char            lpplev[PATH_MAX * 2];   /* lppname and level        */

   inu_get_lppname (op->name, lpplev);
   strcat (lpplev, " ");
   strcat (lpplev, level);

   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_LEVEL_NUM_E, CMN_LEVEL_NUM_D),
                                inu_cmdname, lpplev);

   if (toc_caller != CALLER_INUTOC)
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D), 
                                   inu_cmdname);

} /* end inu_dump_lvl_msg */

/*--------------------------------------------------------------------------*
**
** FUNCTION             inu_toc_level_convert
**
** DESCRIPTION
**
** PARAMETERS
**
**--------------------------------------------------------------------------*/

static int inu_toc_level_convert (char     * level, 
                                  Option_t * op, 
                                  int        bff_fmt, 
                                  char     * i_action)
{
   int             rc;
   int             action;

  /** -------------------------------------------------- *
   **  If we're dealing w/ a 3.1 image ...
   ** -------------------------------------------------- */

   if (bff_fmt == FMT_3_1  || bff_fmt == FMT_3_1_1)
   {
      inu_31level_convert (level, &(op->level));
      return (SUCCESS);
   }

   if ((rc = inu_level_convert (level, &(op->level))) == SUCCESS)
   {
      if (set_action (&action, i_action) != SUCCESS)
         return (FAILURE);

      if (verify_level_syntax(op->name,bff_fmt,&(op->level),action) != SUCCESS)
         return (FAILURE);

      return (SUCCESS);
   }

   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, CMN_BAD_TOC_D),
                                inu_cmdname);

   inu_dump_lvl_msg (op, level);

   return (FAILURE);
} 

/*--------------------------------------------------------------------------*
**
** FUNCTION     inu_set_prod_name
**
** DESCRIPTION
**
** PARAMETERS
**
**--------------------------------------------------------------------------*/

static int inu_set_prod_name (Option_t * op, 
                              char     * prodname)
{
   char  lppname [MAX_LPP_FULLNAME_LEN];
   char  *pn;
   char  *op_n;
   int   i;
   int   rc=SUCCESS;

   if (op == NIL (Option_t))
   {
      *prodname = '\0';
      return (SUCCESS);
   }

  /* ------------------------------------------------------------- *
   *  Determine if we're dealing with a 3.1, 3.2 or a 4.1 image.
   * ------------------------------------------------------------- */

   switch (op->op_type &OP_TYPE_VERSION_MASK)   /* mask with (4 | 8 | 16) */
   {
      case OP_TYPE_3_1:
           if (*prodname == '\0')
           {
              inu_get_lppname (op->name, prodname);
           }
           strcpy (op->prodname, prodname);
           rc = SUCCESS;
           break;


      case OP_TYPE_3_2:
           if (*prodname == '\0')
           {
              inu_get_lppname (op->name, prodname);
              rc = SUCCESS;
           }
           else
           {
              inu_get_lppname (op->name, lppname);

              if (strcmp (lppname, prodname) != 0)
              {
                 inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                              CMN_BAD_TOC_D), inu_cmdname);
                 rc = FAILURE;
              }
           }
           strcpy (op->prodname, prodname);
           break;


      case OP_TYPE_4_1:
           if (prodname[0] == '\0')
           {
              inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                           CMN_BAD_TOC_D), inu_cmdname);
              return FAILURE;
           }

          /* ---------------------------------------------------------------- *
           *  prodname must match the 1st strlen (prodname) chars of op->name
           * ---------------------------------------------------------------- */

          for (pn = prodname, op_n = op->name, i = 0;
              (*pn != '\0'  &&  *op_n != '\0')  &&  (*pn == *op_n);
               pn++, op_n++, i++)
               ;

           if (i != strlen (prodname))
           {
              inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                           CMN_BAD_TOC_D), inu_cmdname);
              inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_INVALID_OPT_NAME_E,
                                           CMN_INVALID_OPT_NAME_D), inu_cmdname,
                                           op->name, prodname);
              rc = FAILURE;
           }
           strcpy (op->prodname, prodname);
           break;

      default:
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_TOC_E, 
                                        CMN_BAD_TOC_D), inu_cmdname);
           return FAILURE;

   } /* switch */

   return rc;

} /* end inu_set_prod_name */

/* ----------------------------------------------------------------------*
**
**  Function  -  missing_size_info
**  
**  Purpose   -  Checks the options in the sop chunk being processed
**               and ensures that a size file exists.
**
**  Returns   -    TRUE - 1 or more size files are missing
**                FALSE - no size files are missing 
**
** ----------------------------------------------------------------------*/

int missing_size_file (Option_t *op, Option_t *next_op, char *o1)
{
   Option_t *top;

   for (top=op;  
        top != NIL (Option_t)  &&  top != next_op;
        top=top->next) 
     /** ---------------------------------------------------- *
      **  If the size file is empty and we're not dealing w/
      **  an MC, ME, or MM update, then error.
      ** ---------------------------------------------------- */

      if ((top->size == NIL (char)  ||  top->size[0] == '\0')
                            && 
          ! (IF_C_UPDT (top->op_type))  &&  
          ! (IF_E_UPDT (top->op_type))  && 
          ! (IF_M_UPDT (top->op_type)))
      {
         inu_get_optlevname_from_Option_t (top, o1);
         return (TRUE);
      }
 
   return (FALSE);
}

/* ----------------------------------------------------------------------*
**
**  Function  -   inu_mixed_levels
**  
**  Purpose   -   Ensures that the sop package being processed does not
**                contain options at different levels. 
**
**  Returns   -   TRUE - if 1 or more options have different levels
**               FALSE - if no options have different levels
**
** ----------------------------------------------------------------------*/

int inu_mixed_levels (BffRef_t *bff, Option_t *next_op, char *o1, char *o2)
{
  Option_t *op,   /* current option traversed ptr */
           *lop;  /* last option traversed ptr    */

  /** ----------------------------------------------------------- *
   **  If we have at least 2 options within this bff, then we
   **  need to ensure that they are at the same level. 
   ** ----------------------------------------------------------- */
 
   if (bff->options->next->option->next != NIL (Option_t))
   {
      for (lop = bff->options->next->option, op = lop->next; 
           op != next_op; 
           lop = op, op = op->next)
      {
        /** ---------------------------------------------------- *
         **  If the levels are different, then return TRUE.
         ** ---------------------------------------------------- */

         if (inu_level_compare (&lop->level, &op->level) != 0)   
         {
            inu_get_optlevname_from_Option_t (lop, o1);
            inu_get_optlevname_from_Option_t (op, o2);
            return (TRUE);
         }
      }
   }

  return (FALSE);
}

/* ----------------------------------------------------------------------*
** 
**  Function    -   verify_toc_semantics
**
**  Purpose     -   Performs some semantics checking on the toc.  
**                  Currently this includes checks for:
**                   1. Mixed levels within a package
**                   2. Existence of a size file for 4.1 or greater images
**
**  Returns     -   INUGOOD  - if all conditoins test successful
**                  FAILURE  - if one or more conditions fail
**
** ----------------------------------------------------------------------*/

int verify_toc_semantics (TOC_t *tp)      /*  toc ptr */
{
   Option_t *op,           /* traversal ptr */
            *next_op;      /* ptr to 1st option of next bff chunk */
   BffRef_t *bff;
   Level_t  l1, l2;
   char     o1[MAX_LPP_NAME+35], o2[MAX_LPP_NAME+35];

  /** ----------------------------------------------------- *
   **  Loop thru the bff list and do our checks ...
   ** ----------------------------------------------------- */

   for (bff  = tp->content->next;  
        bff != NIL (BffRef_t);  
        bff  = bff->next)
   { 
     /** -------------------------------------------------------------- *
      **  Set next_op to point to the 1st op of the next bff chunk ... 
      ** -------------------------------------------------------------- */

      if (bff->next == NIL (BffRef_t))
          next_op = NIL (Option_t);
      else
          next_op = bff->next->options->next->option;

     /** ----------------------------------------------------------- *
      **  Make sure we have no mixed levels here ... 
      ** ----------------------------------------------------------- */

   /*--------- Commented out as per defect 159905 -----------------
    * if (inu_mixed_levels (bff, next_op, o1, o2))
    * {
    *    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MIXT_LEVELS_E, 
    *                                 CMN_MIXT_LEVELS_D), inu_cmdname, o1, o2);
    *    return (FAILURE);
    * }
    */

     /** ----------------------------------------------------------- *
      **  Make sure a size file exists for 4.1 images ...
      ** ----------------------------------------------------------- */

      if (IF_4_1 (bff->options->next->option->op_type) 
                         &&  
          missing_size_file (bff->options->next->option, next_op, o1))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MSG_SIZE_FILE_E, 
                                      CMN_MSG_SIZE_FILE_D), inu_cmdname, o1);
         return (FAILURE);
      }
   }
   return (INUGOOD);
}


/*--------------------------------------------------------------------------*
**
** FUNCTION:    skip_to_brace
**
** DESCRIPTION: Consumes the TOC down to the next "]\n" or "}\n"
**
** RETURNS:     FAILURE       - means the toc entry did not have an
**                              ending brace (format error)
**
**              R_BRACE_FOUND - means the ']' delimiter was detected
**                              (end of info).
**
** PARAMETERS:
**              media_ptr - FILE pointer to the toc file.
**
**
**--------------------------------------------------------------------------*/

static int skip_to_brace(FILE  * media_ptr)  /* FILE ptr to the toc file */
{
   char            buf[TMP_BUF_SZ];

   while (fgets (buf, TMP_BUF_SZ, media_ptr) != NIL (char) )
   {
      if (strlen (buf) == 2)        /* Have we reached the end? */
      {
         if (buf[0] == ']')
            return (R_BRACE_FOUND);
         if (buf[0] == '}')
            return (FAILURE);
      }
   }
   return (FAILURE);
}



/* --------------------------------------------------------------- *
**
**  Function     verify_level_synatax
**
**  Purpose      Verifies the syntactical correctness of a level.
**
**  Returns      SUCCESS - if level is legal
**               FAILURE - if level is not legal
**
** --------------------------------------------------------------- */

static int verify_level_syntax
  (char *op_name,        /* Option name                     */
   int bff_fmt,          /* format of the bff               */
   Level_t *level,       /* Level to verify                 */
   int action)           /* Type of pkg (install or update) */
{

 /** --------------------------------------------------------------- *
  **   Verify level of a base level
  ** --------------------------------------------------------------- */

  if (action == ACT_INSTALL)
  {
    /** --------------------------------------------------------------- *
     **  No base level should have anything in the ptf field
     ** --------------------------------------------------------------- */

     if (level->ptf[0] != '\0')
     {
        inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_FIX_ID_E,
                                   INP_NO_FIX_ID_D), op_name,
                 level->ver, level->rel, level->mod, level->fix, level->ptf);
        return (FAILURE);
     }

    /** --------------------------------------------------------------- *
     **  As of defect 165817 it is no longer true that a 4.1 base level 
     **  must have fix=0, in v.r.m.f
     ** ---------------------------------------------------------------
     *
     * if (bff_fmt == FMT_4_1  &&  level->fix != 0)
     * {
     *    inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_FIX_IN_41_BASE_E,
     *                               INP_NO_FIX_IN_41_BASE_D), op_name,
     *             level->ver, level->rel, level->mod, level->fix, level->ptf);
     *
     *    inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_FIX_MUST_BE_0_E,
     *                               INP_FIX_MUST_BE_0_D) );
     *    return (FAILURE);
     * }
     */
  }

 /** --------------------------------------------------------------- *
  **  Verify level of an update
  ** --------------------------------------------------------------- */

  else if (IF_ACT_UPDATE(action))
  {
    /** --------------------------------------------------------------- *
     **  A 4.1 update can't have anything in the ptf field
     ** --------------------------------------------------------------- */

     if (bff_fmt == FMT_4_1  &&  level->ptf[0] != '\0')
     {
        inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_41_UPDT_HAS_FIX_ID_E,
                                   INP_41_UPDT_HAS_FIX_ID_D), op_name,
                 level->ver, level->rel, level->mod, level->fix, level->ptf);

        inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_FIX_ID_IN_41_UPDT_E,
                                   INP_NO_FIX_ID_IN_41_UPDT_D));
        return (FAILURE);
     }

    /** --------------------------------------------------------------- *
     **  A 4.1 update can't have mod=0 AND fix=0
     ** --------------------------------------------------------------- */

     if (bff_fmt == FMT_4_1  &&  level->mod==0  && level->fix ==0 )
     {
        inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_41_UPDT_HAS_ZERO_MF_E,
                                   INP_41_UPDT_HAS_ZERO_MF_D), op_name,
                 level->ver, level->rel, level->mod, level->fix, level->ptf);

        inu_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_NO_ZERO_MOD_AND_FIX_E,
                                   INP_NO_ZERO_MOD_AND_FIX_D));
        return (FAILURE);
     }
  }

  return (SUCCESS);
}

