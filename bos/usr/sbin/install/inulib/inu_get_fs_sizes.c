static char sccsid[] = "@(#)66  1.5  src/bos/usr/sbin/install/inulib/inu_get_fs_sizes.c, cmdinstl, bos411, 9428A410j 6/17/93 16:14:59";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_get_fs_sizes
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>

void inu_get_fs_sizes(char *ptr, size_t total[], size_t max_temp[])
{
	FILE fp;

	/* make size pointer look like a file */
	fp._flag = (_IOREAD | _IONOFD);
	fp._ptr = fp._base = (unsigned char*)ptr;
	fp._cnt = strlen(ptr);
	fp._file = _NFILE;

	inu_fget_fs_sizes(&fp,total,max_temp);
}
