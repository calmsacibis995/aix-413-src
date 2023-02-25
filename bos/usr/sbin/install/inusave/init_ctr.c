static char sccsid[] = "@(#)38  1.5  src/bos/usr/sbin/install/inusave/init_ctr.c, cmdinstl, bos411, 9428A410j 6/18/93 12:30:18";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS:           init_ctr, select, compare
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <inusave.h>

/* -------------------------------------------------------------------------
         L  O  C  A  L           P  R  O  T  O  T  Y  P  E  S
   ------------------------------------------------------------------------- */

static int select (struct dirent * de);
static int compare (struct dirent **d1, struct dirent **d2);

static char * base_name; /* GLOBAL to this file  -- substring used in select
                          function.  Typcial values are "update" or "archive" */

/* -------------------------------------------------------------------------
 *
 * FUNCTION -                   init_ctr
 *
 * DESCRIPTION -                Returns the lowest safe number to be used
 *                      by inusave as a file extension (".n").  Some
 *                      files of the form "update.n" or "archive.n"
 *                      may exist before init_ctr.c is called.  So, in
 *                      order to avoid over-writing a file--such as
 *                      "update.1"--that could already exist, this function
 *                      is called.
 *
 * NOTES   -                    Scans the directory specified by the
 *                      "dirname" parameter for files that contain the
 *                      substring specified by the "substring" parameter.
 *                      Each of these files end in "n", where n is a
 *                      positive integer.
 *
 * RETURNS -                    A positive integer which corresponds to
 *                      one higher than the current highest number being
 *                      used.
 *
  ------------------------------------------------------------------------- */

int init_ctr
(
 char *dirname,     /* directory to search for files that contain substring */
 char *substring,   /* substring that filenames should contain  */
 int  *ctr
)
 /*
   This function determines the lowest safe number to be used by the inusave
   routine to save files as "update.n" or "archive.n".
  */

{
 struct dirent * (*namelist);  /* important field is d_name -- name of file   */
 int highest_num=0;            /* is the highest number currently being used  */
 char *numptr;                 /* points to first char past '.' in "update.n" */
 int rc;                       /* return code from scandir                    */

 base_name = substring;                             /* Used in select function */


 /********************************************************
  **  Scan the directory specified by dirname for all
  ** files meeting the criteria of the select function.
  ** Numerically sort all files by the .n ending, so that
  ** the last file in the sorted list will be the file
  ** that has the highest .n ending.
  ********************************************************/

 rc = scandir (dirname, &namelist, select, compare);

 if (rc == -1)                                           /* then unsuccessful */
 {
        perror (inu_cmdname);
        return -1;                            /* which will always be -1 here */
 }

 numptr = strchr (namelist [rc-1]->d_name, '.') + 1;  /* pt to char after '.' */

 highest_num = atoi (numptr);                 /* convert numptr to an integer */

 *ctr = (highest_num + 1);       /* want to start at the lowest UNUSED number */

 return (INUGOOD);       /* success */

} /* end init_ctr */

/* -----------------------------------------------------------------------
**
** FUNCTION:                    select
**
** DESCRIPTION:
**              This function is used by scandir to select which
**    files in the directory being checked are to be entered
**    in the namelist (saved).
**
** PARAMETERS:  de - is a directory entry structure, which contains
**    info about the directory begin scandir'd
**
** RETURNS:       0  - tells scandir to save this entry
**                1  - tells scandir to discard this entry
**
** ----------------------------------------------------------------------- */

static int select (
            struct dirent * de) /* directory entry -- which points to a file */
{
 if (strstr (de -> d_name, base_name) == 0) /* then d_name contains basename */
    return (0);                       /* zero     ==> save this entry        */
 return (1);                          /* non-zero ==> do NOT save this entry */

} /* end select */


/* -----------------------------------------------------------------------
**
** FUNCTION:            compare
**
** DESCRIPTION:
**              This function is used by scandir to sort the entries
**      in the namelist.
**
** PARAMETERS:  d1  - a directory entry struct compared against d2
**              d2  - a directory entry struct compared against d1
**
** RETURNS:     -1  - if the file specified by d1 has a ".n" that is
**                    less than the file specified by d2.
**
**              +1  - if the converse of the above statement holds
**
** ----------------------------------------------------------------------- */

static int compare (
  struct dirent **d1,    /* a directory entry (file) which is compared to d2 */
  struct dirent **d2)    /* a directory entry (file) which is compared to d1 */

{
 char *p1, *p2; /* Temporary string pointers used to mark the beginning   */
                /* of the numerical ending (digit part) of the filenames. */
 int n1, n2;    /* The converted p1 and p2 ".n" filename endings          */


                /* d_name is the name of the file: "update.n" or "archive.n". */
                /* An example: "update.2" or "archive.4"  */


 p1 = strchr ((*d1)->d_name, '.') + 1;
           /* p1 now points to first char past '.' -- which should be a digit */

 p2 = strchr ((*d2)->d_name, '.') + 1;
           /* p2 now points to first char past '.' -- which should be a digit */

 /* check these somehow ? */

 n1 = atoi (p1);                         /* convert p1 from char * to integer */
 n2 = atoi (p2);                         /* convert p2 from char * to integer */

 return (n1 < n2) ? -1: +1;         /* return '-' if n1 < n2, else return '+' */
}

/* --------------------------------------------------------------------- */
