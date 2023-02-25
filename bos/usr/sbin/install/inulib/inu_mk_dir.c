static char sccsid[] = "@(#)16  1.18  src/bos/usr/sbin/install/inulib/inu_mk_dir.c, cmdinstl, bos411, 9428A410j 3/6/94 19:27:59";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_mk_dir.c (inulib)
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

#include <inulib.h>
#include <sys/access.h>
#include <sys/stat.h>

/*
#include <sys/limits.h>
#include <string.h>
#include <stdio.h>
#include "../messages/instlmsg.h"
#include "../messages/commonmsg.h"
#include "../inudef.h"
#include <inu_toc.h>
#include <inuerr.h>
*/


/*  EXTERNS  */

extern char *inu_cmdname;       /* Name of command calling these routines */


/*
 * NAME: inu_mk_dir.c (directory)
 *
 * FUNCTION: verifies the existance of a directory.  If the directory
 *           does not exist and the parent directory is writable, the
 *           original directory will be created.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS:
 *    INUGOOD if the directory is writable and exists
 *    INUNOWRT if the directory is unwritable
 */

int inu_mk_dir (
char *dir)      /* path to the needed directory */
{
   char   *strip;                 /* pointer to the last "/" in dir */
   char   new_dir[PATH_MAX+1];    /* the striped directory */
   int    rc;                     /* return code from mkdir */
   struct stat st_buf;            /* stat buffer */
   char   tmp_dir[PATH_MAX+1];    /* temp directory used for adding '/' */

    if ((dir[0] == '/')  ||  ((dir[0] == '.')  &&  (dir[1] == '/')))
        strcpy (tmp_dir, dir);
    else
    {
        /* if NOT / or ./  then tack the / to the beginning */

        strcpy (tmp_dir, "/");
	strcat (tmp_dir, dir);

    }

    if (access (tmp_dir, E_ACC) == 0)
    {  /* the directory DOES exist */
       if (stat (tmp_dir, &st_buf) == 0) 
          /* check if dir or file */
          switch (st_buf.st_mode &S_IFMT) 
          {
                case S_IFLNK:                    /* if link */
                case S_IFDIR:                    /* if directory */
                                return (INUGOOD);

                case S_IFBLK:                    /* if block device */
                case S_IFCHR:                    /* if character device */
                case S_IFSOCK:                   /* if socket */
                case S_IFIFO:                    /* if pipe */
                case S_IFREG:                    /* if regular file */
                default:
                                return (FAILURE);

          } /* end switch */
    } /* end if (access) */

    else 
    {  /* the directory DOES NOT exist */

       /* strip directory down one level */
       strip = strrchr (tmp_dir, '/');          /* set strip to the last '/' */
       strip[0] = '\0';                         /* set the last '/' to '\0'  */
       strcpy (new_dir, tmp_dir); /* set new_dir to tmp_dir w/o the last '/' */
       strip[0] = '/';               /* return tmp_dir to its original state */

       /* call inu_mk_dir with the striped down directory */
       if (strcmp (new_dir, "") != 0) 
       {
          if (inu_mk_dir (new_dir) == INUGOOD) 
          {
             /* everything below the directory exists so make this directory */
             rc = mkdir (tmp_dir, 0755);

             if (rc != 0) 
             {
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                                     CMN_NO_MK_DIR_D), inu_cmdname, dir);
                return (INUNOMK);
             } /* end if (mkdir failed) */

             return (INUGOOD);

          } /* end if (everything below exists) */

       } /* end if (strip != tmp_dir) */

       else 
       {
          rc = mkdir (tmp_dir, 0755);

          if (rc != 0) 
          {
             inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                                  CMN_NO_MK_DIR_D), inu_cmdname, dir);
             return (INUNOMK);
          } /* end if (mkdir failed) */

          return (INUGOOD);
       }

    } /* end else (not there) */

} /* end inu_mk_dir () */
