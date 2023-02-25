static char sccsid[] = "@(#)63  1.6  src/bos/usr/sbin/install/inulib/inu_fcopy.c, cmdinstl, bos411, 9428A410j 6/17/93 16:14:38";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_fcopy (inulib)
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

#include <stdio.h>

/*
 * NAME: inu_fcopy
 *
 * FUNCTION:
 *	Copy the contents of one file to another.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * ON ENTRY:
 *	fd1	file descriptor of origination file
 *	fd2	file descriptor of destination file
 *
 * RETURNS:
 *	Return value is return value of last putc()
 *	(in case there is an error).
 */
int inu_fcopy(FILE *fd1, FILE *fd2)
{
  int c;
  int rc;

  rc = 0;
  while ( (( c = getc(fd1)) != EOF) && rc != EOF)
    rc = putc(c,fd2);

  if (rc != EOF) rc=0;
  return(rc);
}
