static char sccsid[] = "@(#)64	1.3  src/bos/usr/sbin/acct/lib/devtolin.c, cmdacct, bos411, 9428A410j 5/23/94 17:57:31";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: devtolin
 *
 * ORIGINS: 3, 9, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	convert device to linename (as in /dev/linename)
 *	return ptr to LSZ-byte string, "?" if not found
 *	device must be character device
 *	maintains small list in tlist for speed
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <dirent.h>

#define TSIZE1	300	/* # distinct names, for speed only */
static	tsize1;
static struct tlist {
	char	tname[LSZ + 4];	/* linename */
	dev_t	tdev;		/* device */
} tl[TSIZE1];
static char *devdir[] = { "/dev", "/dev/pts"};
static struct dirent *d;

dev_t	lintodev();

char *
devtolin(device)
dev_t device;
{
	register struct tlist *tp;
	register i;
	register char buf[LSZ + 4];
	DIR *fdev;

	for (tp = tl; tp < &tl[tsize1]; tp++)
		if (device == tp->tdev)
			return(tp->tname);

	for (i = 0; i < 2; i++) {
		if ((fdev = opendir(devdir[i])) == NULL)
			return("?");
		while ( (d = readdir(fdev)) != NULL) {
			buf[0] = '\0';
			if (i > 0)
				strcpy(buf, "pts/");
			strcat(buf, d->d_name);
			if (lintodev(buf) == device) {
				if (tsize1 < TSIZE1) {
					tp->tdev = device;
					CPYN(tp->tname, buf);
					tsize1++;
				}
				closedir(fdev);
				return(buf);
			}
		}
		closedir(fdev);
	}
	return("?");
}
