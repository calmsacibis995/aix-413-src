static char sccsid[] = "@(#)78  1.12  src/bos/usr/sbin/install/inulib/inurddk.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:33";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: readdsk, ckvol
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

#include <fcntl.h>      /* defines for open             */
#include <inulib.h>

/* Global structure types, used in read and ck vol routines            */


#define BLK_SZ      512
#define BLK_PER_TRK 18
#define TRK_SZ      (BLK_SZ*BLK_PER_TRK)
#define TRK_DISK    160
#define MEDIA_SZ    (TRK_DISK*TRK_SZ)

#define VOLID_SZ    12
#define SD_MAGIC    (0xD7298918)
#define BADDSKT    (-2)

struct vol_head
{
   int             magic;
   char            volid[VOLID_SZ];
   int             volnum;
   char            fill[BLK_SZ - sizeof (int) - sizeof (int) - VOLID_SZ];
};

typedef struct vol_head VOL_REC;

static int ckvol (int       volnum, 
                  char    * volid, 
                  char    * devid, 
                  VOL_REC * buff, 
                  int       prompt);

static int nxtvol (int    vnum, 
                   char * device);

/* NAME: readdsk
 *
 * FUNCTION: Read a block of data from a diskette set
 *
 *           Read data from one or more diskettes in a set, and send the data
 *           to standard output.  The data to be read is controlled by the
 *           parameters to this routine.
 *
 * INPUT :   dev    - Pointer to name of input device to be read. This will be
 *                    assumed to be a diskette device.
 *           vol    - Volume number of the volume set which contains the start
 *                    of the data.  If needed, subsequent volumes will be used
 *                    to get additional data as needed.
 *           volid  - Pointer to volume ident string.  This may be a null
 *                    pointer or a pointer to a null string if the volume id is
 *                    not to be verified (normally used on the first call, to
 *                    get the volid). If this parm is a (non null) pointer to
 *                    an empty string, the actual volume id string will be
 *                    placed in this structure as a returned value.  Thus a
 *                    caller can effectively read the volume id by passing such
 *                    a pointer.
 *           beg    - Begining block number to be read.  Data begining at the
 *                    start of this block will be read and written to standard
 *                    out.
 *           num    - Number of characters of data to be read.
 *           trc    - Trace level control - zero suppresses the output. Trace
 *                    output will go to stderr.
 *           wfunc  - Function to be used for output of data. Any positive or
 *                    zero return code will be ignored,  any negative return
 *                    code will result in termination of this call with a return
 *                    code of 1 to the caller. The function will be passed two
 *                    arguments, a char pointer to a buffer of data and an
 *                    integer count of the valid data in the buffer.
 *           prompt - Should the user be prompted for disks or just return.
 *
 * RETURNS:      0  - no error
 *               1  - unrecoverable read error.
 *
 */

int inurddk (char * dev,       /* device / file specification  */
             int    vol,       /* volume number to begin with  */
             char * volid,     /* volume ident, or null        */
             int    beg,       /* begining block number        */
             int    num,       /* number of characters to read */
             int (* wfunc) (), /* output function to use       */
             int    prompt)    /* prompt user if wrong disk    */

{

   VOL_REC         blk1;        /* hold vol ident block */

   int             d_pos;               /* current pos in diskette */
   int             t_read;              /* total data read so far  */
   int             filed;               /* file descriptor for input */
   int             rc;                  /* general return code  */
   int             rd_sz;               /* size to be read */
   int             rd_act;              /* actual data amount read */
   int             volnumber;           /* active volume number */
   char            volident[VOLID_SZ];  /* active volume ident  */
   char            track_data[TRK_SZ];  /* data buffer */

   filed = ckvol (vol, volid, dev, &blk1, prompt);   /* get initial volume */
   if (filed < 0)
      return (filed);

   volnumber = blk1.volnum;     /* save vol num and id  */
   memcpy (volident, blk1.volid, VOLID_SZ);
   if (volid != (char *) NULL  &&         /* if pointer to null string */
       (*volid == '\0'))                /* then return value    */
   {
      memcpy (volid, blk1.volid, VOLID_SZ);
   } /* end if */

   d_pos = beg * BLK_SZ;                /* compute start pos    */
   rc = lseek (filed, d_pos, SEEK_SET); /* position file        */

   t_read = 0;                          /* init total read      */

   while (t_read < num)
   {
      rd_sz = TRK_SZ - (d_pos % TRK_SZ);    /* compute data left on trk */
      if (rd_sz > (num - t_read))
      {                         /* if that is past end  */
         rd_sz = num - t_read;  /* then just read to end */
      } /* end if */
      rd_act = read (filed, track_data, rd_sz);    /* read data */
      if (rd_act < 0)
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_READ_ERROR_E, 
                                      CMN_READ_ERROR_D), inu_cmdname, dev);
         return (INUSYSERR);    /* abandon execution    */
      }
      else
      {
         d_pos += rd_act;       /* step disk position   */
         t_read += rd_act;      /* step total read      */

         if (wfunc (track_data, rd_act) < 0)    /* output data, ck return */
         {
            close (filed);
            return (1);                         /* output pgm requested stop */
         } /* end if */

         /* write (stdout, track_data, rd_act) ; *//* send to std out      */

         if (d_pos >= MEDIA_SZ)
         {
            close (filed);
            volnumber += 1;                       /* get next volume      */
            filed = ckvol (volnumber, volident, dev, &blk1, prompt);
            if (filed < 0)
               return (filed);

            d_pos = BLK_SZ;                       /* position to blk 1    */
            rc = lseek (filed, d_pos, SEEK_SET);
         } /* end if */
      } /* end if */
   } /* end while */
   close (filed);
   return (0);
} /* end inurddk */


static int ckvol (int       volnum, 
                  char    * volid, 
                  char    * devid, 
                  VOL_REC * buff, 
                  int       prompt)
{
   int             errflg;      /* boolean, any error   */
   int             fd;          /* file descriptor      */
   int             rc;          /* read data size       */

   errflg = TRUE;       /* init to enter loop   */
   while (errflg)       /* loop til no error    */
   {
      errflg = FALSE;   /* init for this iter.  */

      fd = open (devid, O_RDONLY);      /* attempt open         */
      if (fd < 0)
      {
         errflg = TRUE;
         return (FAILURE);
      }
      else
      {
         rc = read (fd, buff, 512);     /* read first block     */
         if (rc <= 0)
         {
            errflg = TRUE;
            close (fd);
            return (FAILURE);
         }
         else
            if (buff->magic != SD_MAGIC)
            {
               /* magic number is wrong */

               close (fd) ;
               return (FAILURE) ;
            }
            else
               if ((volid != (char *) NULL)  &&   /* if no id, or match ok */
                   (*volid != '\0')  && 
                   (strncmp (buff->volid, volid, VOLID_SZ) != 0))
               {
                  errflg = TRUE;            /* ident specified not found    */
                  if ( ! prompt)
                  {
                     close (fd);
                     inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_INV_DSKT_E, 
                                          CMN_INV_DSKT_D), inu_cmdname, volnum);
                     return (BADDSKT) ;
                  }
               }
               else
                  if (buff->volnum != volnum)
                  {
                     errflg = TRUE;     /* correct set but wrong volume */
                     if ( ! prompt)
                     {
                        close (fd);
                        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
                                             CMN_INV_DSKT_E, CMN_INV_DSKT_D), 
                                             inu_cmdname, volnum);
                        return (BADDSKT) ;
                     }
                  } /* end if */
      } /* end if */

      if (errflg == TRUE)       /* if any error         */
      {
         close (fd);    /* close the file       */
         nxtvol (volnum, devid);        /* prompt for volume    */
      } /* endif */
   } /* end while */

   return (fd); /* return file descr    */
} /* end ckvol  */

static int nxtvol (int    vnum, 
                   char * device)
{
   inu_msg (SCREEN_MSG, MSGSTR (MS_INUCOMMON, CMN_INSRT_VOL_I, CMN_INSRT_VOL_D),
                                inu_cmdname, vnum, device);
   getchar ();

} /* end nxtvol */
