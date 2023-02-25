static char sccsid[] = "@(#)50	1.2  src/bos/usr/sbin/slibclean/slibclean.c, cmdcntl, bos411, 9428A410j 4/25/91 17:15:11";
/*
 * COMPONENT_NAME: (CMDCNTL) slibclean
 *
 * FUNCTIONS: command to remove library routines that are not in use
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/ldr.h>

extern int errno;
int
main(argc,argv)
int	argc;
char	**argv;
{
	unload(L_PURGE);
	if (errno)
		perror(argv[0]);
}
