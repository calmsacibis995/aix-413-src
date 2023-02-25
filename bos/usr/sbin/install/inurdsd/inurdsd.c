/* @(#)31       1.8  src/bos/usr/sbin/install/inurdsd/inurdsd.c, cmdinstl, bos411, 9428A410j 3/22/94 09:31:00 */
/*
 * COMPONENT_NAME: (CMDINSTL) Install command support
 *
 * FUNCTIONS: inurdsd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define MAIN_PROGRAM

#include <locale.h>
#include <inulib.h>

/*--------------------------------------------------------------------------
 * NAME: inurdsd
 *
 * FUNCTION: Parse command line, and invokd read of specified data
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Invoked from the command line, assumes that input file is a
 *      diskette device.  Will read the data until the specified number
 *      of characters has been read and direct the data to standard out.
 *
 * INPUT:
 *
 *      -d device       Specifies the input device/file to be read from
 *                      Default is /dev/rfd0
 *      -v num          The volume number of the first volume to ge read.
 *                      Default value is 1
 *      -s datestr      a date string in the format mmddhhMMssyy which
 *                      is the identification of the volume set to be
 *                      processed. Default is 'no id'
 *      -b num          The block number of the first block to be read from
 *                      the input. (block zero is first block) Default is 1
 *      -n num          The number of characters of data to be read.
 *                      Default is MAXINT
 *      -t num          Trace control flag, 0-9, default is 0 ==> no trace
 *
 * RETURNS:  0        - Success
 *           1        - Upon syntax error
 *           2        - Upon read error
 *
 *--------------------------------------------------------------------------*/
main (int     argc, 
      char ** argv, 
      char ** envp)
{
   #define VOLID_SZ 12

   extern int optind;   /* argc index for current command line parm     */
   extern char *optarg; /* pointer to string when option has following  */
   /* value string                                 */

   int  d_opt, v_opt, s_opt, b_opt, n_opt, t_opt;

   char   *devstr;                 /* pointer to input device name         */
   int     volnum;                 /* selected volume number               */
   int     numtord;                /* number of characters to be read      */
   int     blknum;                 /* block number to begin reading        */
   char    volidstr[VOLID_SZ];     /* hold volume ident string             */
   char   *volid;                  /* pointer to volume id                 */
   int     trace;                  /* trace level code                     */
   char    arg;                    /* next command line arg char           */
   boolean syntax_err;             /* indicates a syntax error             */
   int     rc;                     /* return code                          */
   char   *endptr;                 /* pointer to end of arg to strtol      */

   int  dowrite ();                /* default output routine */

   inu_cmdname = "inurdsd";

   (void)setbuf (stdout, NULL);
   (void)setbuf (stderr, NULL);

   (void)setlocale (LC_ALL, "");

   CATOPEN ();

   d_opt = 0;
   v_opt = 0;
   s_opt = 0;
   b_opt = 0;
   n_opt = 0;
   t_opt = 0;

   devstr = "/dev/rfd0";
   volnum = 1;
   volid  = &volidstr[0];
   volidstr[0] = '\0';
   blknum = 1;
   numtord = 9999999;
   trace = 0;

   syntax_err = FALSE;
   rc = 0;

   while ((arg = getopt (argc, argv, "d:v:s:b:n:t:")) != (char) EOF)
   {
      switch (arg)
      {
         case 'd' :
            d_opt++;
            devstr = optarg;
            break;
         case 'v' :
            v_opt++;
            volnum = (int) strtol (optarg, &endptr, 10);
            if (volnum == 0  &&  optarg == endptr)
               syntax_err = TRUE;
            break;
         case 's' :
            s_opt ++;
            volid = optarg;
            if (strlen (volid) != VOLID_SZ)
               syntax_err = TRUE;
            break;
         case 'b' :
            b_opt ++;
            blknum = (int) strtol (optarg, &endptr, 10);
            if (blknum < 0  ||  (blknum == 0  &&  optarg == endptr))
               syntax_err = TRUE;
            break;
         case 'n' :
            numtord = (int) strtol (optarg, &endptr, 10);
            if (numtord <= 0)
               syntax_err = TRUE;
            break;
         case 't' :
            trace = (int) strtol (optarg, &endptr, 10);
            if (trace < 0  ||  trace >9)
               syntax_err = TRUE;
            break;
         default:
            syntax_err = TRUE;

      } /* switch */

      if (trace >= 3)
         inu_msg (SCREEN_LOG, "Argument letter = %c, string value = \"%s\"\n",
                               arg, optarg);

   } /* while */

   if (d_opt > 1 || v_opt > 1 || s_opt > 1 || b_opt > 1 || 
       n_opt > 1 || t_opt > 1)
      syntax_err = TRUE;

   if (syntax_err) 
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, RSD_USAGE_E, RSD_USAGE_D));
      rc = 1;
   } 
   else 
   {
      if (trace > 2)         /* if debug output is requested */
      {
         inu_msg (SCREEN_LOG, "Parameter values -\n");
         inu_msg (SCREEN_LOG, "    input - %s\n", devstr);
         inu_msg (SCREEN_LOG, "    volid - %s\n", volid);
         inu_msg (SCREEN_LOG, "    volnm = %d\n", volnum);
         inu_msg (SCREEN_LOG, "    block = %d\n", blknum);
         inu_msg (SCREEN_LOG, "    size  = %d\n", numtord);
      }

      /* finally do what we came here for */
      if (inurddk (devstr, volnum, volid, blknum, numtord, dowrite, TRUE))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_READ_ERROR_E, 
                                    CMN_READ_ERROR_D), devstr);
         rc = 2;
      }

   } /* else */

   CATCLOSE ();
   exit (rc);

} /* inurdsd */


/* This local routine is used to direct the data read to the standard   */
/* output file.  It returns the number of characters actually written   */
/* or -1 if an error occurs.  The -1 will stop the read routine if it   */
/* is passed back, and will cause that routine to return 1.             */

static int dowrite (char *buff, int buffsz)
{
   return (write (1, buff, buffsz));
}

