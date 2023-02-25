static char sccsid[] = "@(#)15  1.9  src/bos/usr/sbin/install/inulib/inu_cp.c, cmdinstl, bos411, 9428A410j 6/17/93 16:14:19";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_cp (inulib)
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
#include <fcntl.h>
#include <inuerr.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef _NO_PROTO
char *acl_get();
int   acl_put();
#else
char *acl_get(char *);
int   acl_put(char *, char *, int);
#endif

/*
 * NAME: inu_cp(source, target)
 *
 * FUNCTION: Copies the contents of the file pointed to by the path "source"
 *           to the file pointed to by the path "target".
 *
 * RETURN VALUE DESCRIPTION:  INUGOOD  -- copied file successfully
 *			      INUBADC2 -- file copy failed
 */

int inu_cp(
char source[],		/* the file to be copied		*/
char target[])		/* the file to be copied to		*/
{
    char *aclp;			/* access control list			*/
    int	 fp1;			/* pointer to the source file		*/
    int  fp2;			/* pointer to the target file		*/
    char buffer[BUFSIZ];	/* buffer used in file copying		*/
    int	 eof = 1;		/* flag	for detecting end of file	*/
    int  rc;			/* return code from save_arc_files	*/
    unsigned nbyte;		/* number of bytes to write to file	*/
    struct stat buf;		/* structure contains status of the file*/

 /* -------------------------------------------------*
  * If the source or target is a directory, quit     *
  * -------------------------------------------------*/

  if (stat(source, &buf) != 0)
      return(INUBADC2);

  if (S_ISDIR(buf.st_mode))
      return(INUBADC2);
  
  if (stat(target, &buf) == 0)   /* target exists */
    {
      if (S_ISDIR(buf.st_mode))  /* but is a directory */
          return(INUBADC2);
    }

/* Begin copy_file	*/

    if((fp1 = open(source,O_RDONLY)) == -1)
	return(INUBADC2);

    inu_rm(target);

    if((fp2 = open(target,O_WRONLY | O_CREAT | O_TRUNC,0766)) == -1)
    {
	close(fp1);
	return(INUBADC2);
    }

    while (eof > 0) {
	eof = read(fp1,buffer,(unsigned)BUFSIZ); /* read one block	     */
	if (eof	< 0)
        {
	    close(fp1);
	    close(fp2);

           /* --------------------------------------------------------
            * Defect 88769
            * Remove the target file if copy failed
            * ---------------------------------------------------------*/
            inu_rm(target);

	    return(INUBADC2);
	}
	if (eof	> 0) {			 /* something was read		     */
	    nbyte = eof;
	    rc = write(fp2,buffer,nbyte);  /* write one block to target      */
	    if (rc != eof)
            {
	        close(fp1);
	        close(fp2);

               /*--------------------------------------------------------
                * Defect 88769
                * Remove the target file if copy failed
                *--------------------------------------------------------*/
                inu_rm(target);

		return(INUBADC2);
	    }
	}
    }
    close(fp1);
    close(fp2);

    /* give the	target file the	same permissions, source file		 */

    rc = stat(source, &buf);
    if (rc == 0) {
	aclp = acl_get(source);
	acl_put(target, aclp, 1);
	rc = chown(target, buf.st_uid, buf.st_gid);
	if (rc == 0)
	    return(INUGOOD);
    }

   /*--------------------------------------------------------
    * Defect 88769
    * Remove the target file if copy failed
    *--------------------------------------------------------*/
    inu_rm(target);

    return(INUBADC2);
}
