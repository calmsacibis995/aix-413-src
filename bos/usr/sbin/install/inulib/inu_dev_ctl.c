static char sccsid[] = "@(#)60  1.28  src/bos/usr/sbin/install/inulib/inu_dev_ctl.c, cmdinstl, bos412, 9446B 11/14/94 13:49:38";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: get_block_size
 *		get_correct_interface
 *		get_dev_name
 *		is_it_a_tape_device
 *		tape_err
 *		tchg_blksize
 *		tchg_blksize_to_zero
 *		tclose
 *		tcopy_file
 *		tfile_mark
 *		topen
 *		tread_record
 *		trequest_vol
 *		trewind
 *		tseek_file
 *		tseek_back_one
 *		tseek_record
 *		ttape_size
 *		twrite_record
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <inulib.h>
#include <sys/tape.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <fcntl.h>
#include <inu_dev_ctl.h>

#include <inudef.h>
#include <sys/sysmacros.h>

#define TAPE_EXIT  {trewind (tape); tclose (tape); return (-1);}

static struct stop  targ;
static int          blksize = 0;
static int          origblksize = -1;
extern char        *inu_cmdname;      /* routine that called these routines */
extern int          current_vol;
extern int          current_off;
static char        *tbuf = NULL;

static int get_vol_id   (int  tape);

/*--------------------------------------------------------------------------*
**
** NAME: trequest_vol
**
** FUNCTION: Requests the user to insert the proper volume.
**
** RETURNS: The return code from the ioctl call.
**
**--------------------------------------------------------------------------*/

void trequest_vol (int    vol, 
                   char * media)
{
   char            buf[75];

   /*-----------------------------------------------------------------------*
    * make sure we have a real volume number
    *-----------------------------------------------------------------------*/

   if (vol == NO_PROMPT)
      vol = 1;
   fflush (stdin);
   inu_msg (SCREEN_MSG, MSGSTR (MS_INUCOMMON, CMN_INSRT_VOL_I, CMN_INSRT_VOL_D), 
            inu_cmdname, vol, media);
   fscanf (stdin, "%[^\n]", buf);

} /* end trequest_vol */

/*--------------------------------------------------------------------------*
**
** NAME: topen
**
** FUNCTION: Open the tape device.
**
** RETURNS: The return code from the ioctl call.
**
**--------------------------------------------------------------------------*/

int topen (char * media, 
           int    vol, 
           int    mode, 
           int    quiet_flag)
{
   extern int      userblksize;
   int             err=-2;

   do
   {
      /*-------------------------------------------------------------------*
        * The quiet flag only has meaning on the first (NO_PROMPT) access for
        * the toc otherwise, if the new volume is the same as the old volume, 
        * do not prompt the user.
        *-------------------------------------------------------------------*/

      if (err >= 0  &&  current_vol != vol) /* get_vol_id leaves it open */
          tclose (err);

      if ((vol == NO_PROMPT  &&  ! quiet_flag)  || 
          (vol != NO_PROMPT  &&  current_vol != vol)  ||  err == -1)
      {
         (void) trequest_vol (vol, media);
      }

      err = open (media, mode);

   } while (err < 0  ||  (current_vol != vol  &&  vol != NO_PROMPT  && 
           (current_vol = get_vol_id (err)) != vol)) ;

   /*-----------------------------------------------------------------------*
    * If we opened the tape device successfully, then do an ioctl to make
    * sure it's the kind of device we really want, then save the block size
    * of the device into a global variable.
    *-----------------------------------------------------------------------*/

   if (err != -1 && vol == NO_PROMPT)
   {
      struct devinfo  devinfo;

      if (ioctl (err, IOCINFO, &devinfo) == -1  || 
          (devinfo.devtype != DD_TAPE  &&  devinfo.devtype != DD_SCTAPE))
      {
         close (err);
         return (-1);
      }
      
      blksize = devinfo.un.scmt.blksize;
      /*-------------------------------------------------------------------*
       * if the blocksize is not the same as the user specified, 
       * set it to the user specified block size
       *-------------------------------------------------------------------*/

      if (blksize != userblksize && userblksize != -1)
      {
        /*------------------------------------------------------- *
         * if origblksize != -1 then it was already changed once  *
         * This change is made for Feature# 43659                 *
         *------------------------------------------------------- */
         origblksize = (origblksize == -1) ? blksize : origblksize;
         blksize = userblksize;
         close (err);
         tchg_blksize (media, blksize);
         err = open (media, mode);
      } /* end if */


   } /* end if */

   if (vol == NO_PROMPT)
      vol = 1;

   /*-----------------------------------------------------------------------*
   * set the current volume to the one passed in
   *-----------------------------------------------------------------------*/

   current_vol = vol;  /* redundant ? */

   return (err);

} /* end topen */

/*--------------------------------------------------------------------------*
 * Change the block size to newsize...
 *--------------------------------------------------------------------------*/
int no_chdev=0;

int    
tchg_blksize (char * media, 
                   int    newsize)
{
   char            cmd[255];
   char            media_name[100];
   int             len;


   if(no_chdev)
	return(0);

   /* if the size is NULL, then set back to original size. */

   if (newsize == 0)
   {
      if (origblksize == -1)
         return;
      else
         newsize = origblksize;
   }

   strcpy (media_name, basename (media));

    /* If this has a .N then remove it. */

   len = strlen (media_name);
   if (media_name[len - 2] == '.')
      media_name[len - 2] = '\0';

   sprintf (cmd, 
     "ODMDIR=/etc/objrepos /usr/sbin/chdev -l %s -a block_size=%d 1>/dev/null", 
            media_name, newsize);

   return (inu_do_exec (cmd));

} /* end tchg_blksize */

/*--------------------------------------------------------------------------*
**
** NAME: tclose
**
** FUNCTION: Closes the tape drive
**
**--------------------------------------------------------------------------*/

void tclose (int tape)
{
   close (tape);
} /* end tclose */

/*--------------------------------------------------------------------------*
**
** NAME: trewind
**
** FUNCTION: Rewind the tape.
**
** RETURNS: The return code from the ioctl call.
**
**--------------------------------------------------------------------------*/

int trewind (int tape)
{
   targ.st_op = STREW;
   targ.st_count = 1;
   return ioctl (tape, STIOCTOP, &targ);

} /* end trewind */

/*--------------------------------------------------------------------------*
**
** NAME: tfile_mark
**
** FUNCTION:
**
** RETURNS: The return code from the ioctl call.
**
**--------------------------------------------------------------------------*/

int tfile_mark (int tape)
{
   targ.st_op = STWEOF;
   targ.st_count = 1;
   return ioctl (tape, STIOCTOP, &targ);

} /* end tfile_mark */

/*--------------------------------------------------------------------------*
**
** NAME: twrite_record
**
** FUNCTION: Write out the a record to the tape
**
** RETURNS: Returns the return code of the system call write.
**
**--------------------------------------------------------------------------*/

int twrite_record (int    tape, 
                   char * record, 
                   int    rec_len)
{
   register int    i;

   if (rec_len < blksize)
   {
      for (i = rec_len; i < blksize; i++)
         record[i] = '\0';
   }

   return write (tape, record, blksize);

} /* end twrite_record */

/*--------------------------------------------------------------------------*
**
** NAME: tread_record
**
** FUNCTION: Read a record for the tape
**
** RETURNS: The return code from the read
**
**--------------------------------------------------------------------------*/

int tread_record (int    tape, 
                  char * record, 
                  int    n_bytes)
{
   return read (tape, record, blksize);

} /* end tread_record */

/*--------------------------------------------------------------------------*
**
** NAME: tcopy_file
**
** FUNCTION: Copy a file to tape.
**
** RETURNS: The return code for the function tfile_mark () ELSE, 
**          -1      if a failure occurred the copy attempt.
**
**--------------------------------------------------------------------------*/

int tcopy_file (char * file_name, 
                int    tape)
{
   int             disk;
   int             byte_count;
   int             wr_stat;

   /*-----------------------------------------------------------------------*
    * Open the disk file
    *-----------------------------------------------------------------------*/

   disk = open (file_name, O_RDONLY);
   if (disk < 0)
      return -1;

   /*-----------------------------------------------------------------------*
    * Malloc the copy buffer
    *-----------------------------------------------------------------------*/

   tbuf = (char *) malloc ((size_t) blksize);
   if (tbuf == NULL)
   {
      close (disk);
      return -1;
   }

   /*-----------------------------------------------------------------------*
    * Loop reading disk file and write it to the tape file
    *-----------------------------------------------------------------------*/

   do
   {
      byte_count = read (disk, tbuf, blksize);
      if ((wr_stat = twrite_record (tape, tbuf, byte_count)) < 0)
      {
         free ((void *) tbuf);
         close (disk);
         return -1;
      }
   } while (byte_count == blksize);

   /*-----------------------------------------------------------------------*
    * Free copy buffer malloc'ed space and close the disk file
    *-----------------------------------------------------------------------*/

   free ((void *) tbuf);
   close (disk);

   return tfile_mark (tape);

} /* end tcopy_file */

/*--------------------------------------------------------------------------*
**
** NAME: tseek_file
**
** FUNCTION: Do a seek on the tape
**
** RETURNS: The return code from the ioctl call.
**
**--------------------------------------------------------------------------*/

int tseek_file (int tape, 
                int nfiles, 
                int seek_code)
{
   int             err;

   if (seek_code == TSEEK_ABS)
      err = trewind (tape);

   targ.st_op = STFSF;
   targ.st_count = nfiles;

   return ioctl (tape, STIOCTOP, &targ);

} /* end tseek_file */

int tseek_back_one (int tape)
{

   targ.st_op = STRSF;
   targ.st_count = 1;
   ioctl (tape, STIOCTOP, &targ);
   targ.st_op = STFSF;
   targ.st_count = 1;
   ioctl (tape, STIOCTOP, &targ);

}

/*--------------------------------------------------------------------------*
**
** NAME: tseek_record
**
** FUNCTION: Do a seek on the tape
**
** RETURNS: The return code from the ioctl call.
**
**--------------------------------------------------------------------------*/

int tseek_record (int tape, 
                  int nrecs, 
                  int seek_code)
{
   int             err;

   if (seek_code == TSEEK_ABS)
      err = trewind (tape);

   targ.st_op = STFSR;
   targ.st_count = nrecs;

   return ioctl (tape, STIOCTOP, &targ);

} /* end tseek_record */

/*--------------------------------------------------------------------------*
**
** NAME: ttape_size
**
** FUNCTION: Return the tape size.
**
**--------------------------------------------------------------------------*/

int ttape_size (int tape)
{
   return 4096;

} /* end ttape_size */

/*--------------------------------------------------------------------------*
**
** NAME: tape_err
**
** FUNCTION: Print the errno for tape I/O error.
**
**--------------------------------------------------------------------------*/

int tape_err (int err)
{
   printf ("TAPE I/O Error: %d, errno = %d\n", err, errno);
   exit (-1);

} /* end tape_err */

/*--------------------------------------------------------------------------*
**
** FUNCTION          tchg_blksize_to_zero 
**
** DESCRIPTION       sets tape block size to newsize even if newsize is zero 
**
** NOTES             tchg_blksize can not set blksize to 0, hence this
**                   function
**                   This change is made for Feature# 43659                 *
**              
** PARAMETERS        media -- is the path for the tape device
**                   newsize -- is 0 (will work with others too) 
**         
** RETURNS           whatever chdev returns 
**                   
**--------------------------------------------------------------------------*/

boolean tchg_blksize_to_zero (int media,         /* tape device */
			      int newsize,
                              int *oldsize)  
{
        struct devinfo  devinfo;
        struct stchgp targ={0,0};

	/* do we want to save the original size? */
	if(oldsize!=(int *)0)
	{
        	ioctl (media, IOCINFO, &devinfo);
        	*oldsize = devinfo.un.scmt.blksize;
      		/* remember the old tape block size */
      		origblksize = (origblksize == -1) ? devinfo.un.scmt.blksize : origblksize;
	}
	else if(origblksize==newsize)
	{
		/* the block size in the vpd is already set to the correct value
		 * so don't muck with it any more
		 */
		origblksize= -1;
	}
	targ.st_blksize=newsize;
        ioctl(media, STIOCHGP, &targ);
}

/*--------------------------------------------------------------------------*
**
** FUNCTION          get_block_size 
**
** DESCRIPTION       gets the real block size that the tape is written at 
**
** NOTES             This idea is similar to what they do in some of the 
**                   cmdarch commands 
**                   This change is made for Feature# 43659                 *
**              
** PARAMETERS        tape -- is the file descriptor for the tape device
**         
** RETURNS           The actual block size that was read
**
**--------------------------------------------------------------------------*/
int get_block_size (int tape)
{
  extern int  userblksize;
  int bsize;
  const int bigblock=32768; /* this is just a large number  32K */
  char * tbuf;  

  if ((tbuf  = (char *) malloc (bigblock * sizeof (char))) == NULL)
     return (0);

  if ((bsize = read (tape, tbuf, (size_t) bigblock)) == -1)
     bsize = userblksize;
  trewind (tape);
  free (tbuf);
  return (bsize);
}

/*--------------------------------------------------------------------------*
**
** FUNCTION          is_it_a_tape_device
**
** DESCRIPTION       Determines if the character device is a tape device 
**
** NOTES             Just does an ioctl 
**                   This change is made for Feature# 43659                 *
**              
** PARAMETERS        path -- is the path for the tape device
**         
** RETURNS           TRUE  if it is a tape device 
**                   FALSE if not
**
**--------------------------------------------------------------------------*/

boolean is_it_a_tape_device (char * path)
{
  int fd;
  struct devinfo tape_devinfo;

  if ((fd=open (path, O_RDONLY)) == -1) 
    return (FALSE);

  if (ioctl (fd, IOCINFO, &tape_devinfo) == -1  || 
      (tape_devinfo.devtype != DD_TAPE  &&  tape_devinfo.devtype != DD_SCTAPE)) 
  {
     close (fd);
     return (FALSE);
  }
  close (fd);
  return (TRUE);
}


/*--------------------------------------------------------------------------*
**
** NAME              get_correct_interface
**
** DESCRIPTION       If user specifes rmt0, it gets rmt0.1, i.e.
**                   it returns the correct device interface for installing
**
** NOTES             It is device independent, it gets correct interface 
**                   even if the tape device is not names rmt*
**                   This change is made for Feature# 43659                 *
**              
** PARAMETERS        tape -- Path to the tape device 
**                   error -- contains the error code 
**         
** RETURNS           void function, error contains error code 
**                   
**--------------------------------------------------------------------------*/

void 
get_correct_interface (char ** tape,  /* path to the tape device */
                       int * error)  /* error code */
{
   int orig_minor=0,          /* what user had specified  */
       orig_major=0,          /* major number             */ 
       length;                /* to hold strlen value     */

   char  tape_dev[MAXNAMLEN]="",  /* rmt0.1, rmt1.1, etc.  */
         dev_path[PATH_MAX+1]="", /* /dev/rmt0             */
         *tpath;                  /* /dev                  */

   struct stat tape_stat;      /* to do a stat          */

   DIR *dfp;                   /* directory pointer */
   struct dirent *dp;          /* directory entry pointer */

   int new_minor=0;

   *error = FALSE;     
 
   if (stat (*tape, &tape_stat) == -1)    /* do a stat */
   {
      *error = TRUE;
      return;
   }

   /* minor is a macro in sysmacros.h */
   if ((new_minor=minor (tape_stat.st_rdev) % 8)  == 1 || new_minor == 5)   /* already 0.1, 1.1, etc. */
   {
      *error=FALSE;
      return;
   }

   if ((tpath = (char *) malloc (PATH_MAX * sizeof (char))) == NULL)
   {
      /*  in other places in inulib they are using malloc's error msg., so
      printf ("ERROR: malloc () failed \n");
      */
      *error = TRUE;
      return ;
   }
     
   orig_minor = minor (tape_stat.st_rdev); /* what the user had specified */
   orig_major = major (tape_stat.st_rdev); /* what the user had specified */
 
   get_dev_name (*tape, dev_path, tape_dev); /* rmt from /dev/rmt */

   if (dev_path[0] == '\0')               /* relative path */
      strcpy (dev_path , ".");

   if ((dfp=opendir (dev_path)) == (DIR *)0)
   {
      *error=TRUE;
      return;
   }

   /*---------------------------------------------------------*
   * open the directory file, and read all the entries       *
   * it kind of does an ls /dev/rmt0*                        *
   * and for each match, it checks to see if the minor number*
   * is 1                                                    *
   *                                                         *
   *---------------------------------------------------------*/
   length = strlen (tape_dev);  /* do it outside the loop */

   for (dp= readdir (dfp); dp != (struct dirent *)0; dp= readdir (dfp))
   {
      if (strncmp (tape_dev, dp->d_name, length) == 0)
      {
         sprintf (tpath, "%s/%s", dev_path, dp->d_name);
         if (stat (tpath, &tape_stat) == -1)
         {
            closedir (dfp);
            *error=TRUE;
            return;
         }

         if (major (tape_stat.st_rdev) != orig_major) 
            continue; /* different device -- highly unlikely case */
 
         if ((new_minor = minor (tape_stat.st_rdev) % 8) == 1 || new_minor == 5) 
         {
            /* rmt1.1 will wrongly pass for rmt10.1, so the following 
               check */
            if (((minor (tape_stat.st_rdev) - orig_minor) > 7)  ||   
                ((minor (tape_stat.st_rdev) - orig_minor) < -7)) 
               continue;
 
            *tape = tpath; /* gotcha */
 
            break;  /* get outta the loop now */
         }
      }
   } /* of while */

   closedir (dfp);
   *error=FALSE;
   return;
}

/*--------------------------------------------------------------------------*
**
** NAME              get_dev_name
**
** DESCRIPTION       If user specifes /dev/rmt0.5, it gets rmt0
**
** NOTES             calls basename 
**                   This change is made for Feature# 43659                 *
**              
** PARAMETERS        path is an input (/dev/rmt0.5) 
**                   devpath is an output (/dev)
**                   devname is an output (rmt0)
**         
** RETURNS           void procedure 
**                   
**--------------------------------------------------------------------------*/
void 
get_dev_name (char *path,       /* input, device path */
                   char *devpath,    /* output, device directory */
                   char *devname)    /* output, device name      */
{
   char * temp_cptr,   /* teporary char ptr */
        * temp_cptr2;
 
   temp_cptr = getbasename (path);
   strncpy (devname, temp_cptr, strcspn (temp_cptr, ".")); /* device dir */
   if (path != temp_cptr)  /* not a relative path */
     strncpy (devpath, path, (strlen (path) - strlen (temp_cptr) - 1)); 
}


/*--------------------------------------------------------------------------*
**
** FUNCTION:     get_vol_id
**
** DESCRIPTION:  Reads the file after the second file mark on a tape media, 
**               and interprets this file as a table of contents.
**
** PARAMETERS:   media   - Name of the input device.
**
** RETURNS:      Pointer to a table of contents or NIL if malformed file.
**
**--------------------------------------------------------------------------*/

int 
get_vol_id (int  tape) /* the return from the get_vol_id on the tape device */
{
   int      n, rc;            /* return value */
   FILE     *media_ptr;       /* FILE ptr to the open of the tape device */
   char     buf [TMP_BUF_SZ]; /* temp char to receive action char from toc */
   TOC_t toc;                 /* Pointer to the top of the toc link list */

   /*-----------------------------------------------------------------------*
    * Position to the correct tape mark to get to the toc file on the tape.
    * If positioning fails, the tape might contain bff.
    *-----------------------------------------------------------------------*/

   (void) trewind (tape);

   if ((rc = tseek_file (tape, TAPE_TOC_POSITION - 1, TSEEK_ABS)) < 0)
   {
      TAPE_EXIT;
   }

   /*-----------------------------------------------------------------------*
    * Open the tape for reading, and read the toc header information.
    *-----------------------------------------------------------------------*/

   media_ptr = fdopen (tape, "r");
   n = fscanf (media_ptr, TAPE_HEADER_FMT, 
               &(toc.vol),               /* Volume       */
               &(toc.dt),                /* Date of creation */
               buf);                     /* Header format    */

   /*-----------------------------------------------------------------------*
   * If we didn't get correct number of items back from the fscanf, then
    * set return code to continue with the possibility of bff because this
    * must not be a real tape toc file.
    *-----------------------------------------------------------------------*/

   if (n != ITEMS_IN_TAPE_HDR)
   {
      TAPE_EXIT;
   }

   /*-----------------------------------------------------------------------*
   * Set the current volume to what the toc says.
   *-----------------------------------------------------------------------*/
   current_vol = toc.vol;

   if (feof (media_ptr) == 0)   /* have not reached end yet */
      (void) tseek_file (tape, 1, TSEEK_REL);  /* skip to end of this file */

   current_off = TAPE_TOC_POSITION + 1;

   return (toc.vol);

} /* end get_vol_id */

