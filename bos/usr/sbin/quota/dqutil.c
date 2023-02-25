static char sccsid[] = "@(#)18	1.4  src/bos/usr/sbin/quota/dqutil.c, cmdquota, bos411, 9428A410j 4/21/93 11:17:02";
/*
 * COMPONENT_NAME: CMDQUOTA
 *
 * FUNCTIONS: 	hasquota, alldigits, oneof
 *
 * ORIGINS: 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980, 1990 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
 *
 */

#include <stdio.h>
#include <jfs/quota.h>
#include "fstab.h"

/*
 *  NAME:  hasquota
 *
 *  FUNCTION:
 * 	Check to see if a particular quota is to be enabled.
 *
 *  RETURN VALUE:
 *
 */
int
hasquota(register struct fstab *fs, int type, char **qfnamep)
{
        register char *opt;
        char          *cp, *index(), *strtok();
	static int    initname = 0;
        static char   *usrname,
                      *grpname;
        static char   buf[BUFSIZ];

        if (!initname) {
        	if ((usrname = (char *)calloc(strlen(qfextension[USRQUOTA]) +
                              strlen(qfname) + 1, sizeof(char))) == NULL) {
                	perror("malloc");
                	exit(1);
        	}
        	if ((grpname = (char *)calloc(strlen(qfextension[GRPQUOTA]) +
                              strlen(qfname) + 1, sizeof(char))) == NULL) {
                	perror("malloc");
                	exit(1);
        	}

                sprintf(usrname, "%s%s", qfextension[USRQUOTA], qfname);
                sprintf(grpname, "%s%s", qfextension[GRPQUOTA], qfname);
                initname = 1;
        }
        strcpy(buf, fs->fs_mntops);
        for (opt = strtok(buf, ","); opt; opt = strtok(NULL, ",")) {
                if (cp = index(opt, '='))
                        *cp++ = '\0';
                if (type == USRQUOTA && strcmp(opt, usrname) == 0)
                        break;
                if (type == GRPQUOTA && strcmp(opt, grpname) == 0)
                        break;
        }
        if (!opt)
                return (0);
        if (cp) {
                *qfnamep = cp;
                return (1);
        }
        (void) sprintf(buf, "%s/%s.%s", fs->fs_file, qfname, qfextension[type]);
        *qfnamep = buf;
        return (1);
}


/*
 *  NAME:  alldigits
 *
 *  FUNCTION:
 * 	Check whether a string is completely composed of digits.
 *
 *  RETURN VALUE:
 *	Returns 0 if not all characters are digits, otherwise returns 1.
 *
 */
int
alldigits(register char *s)
{
	register c;

	c = *s++;
	do {
		if (!isdigit(c))
			return (0);
	} while (c = *s++);
	return (1);
}


/*
 *  NAME: oneof 
 *
 *  FUNCTION:
 *        Check to see if target appears in list of size cnt.
 *
 *  RETURN VALUE:
 * 	  Returns the element number if a match is found, otherwise
 *        returns -1.
 *
 */
int
oneof(register char *target, register char *list[], int cnt)
{
	register int i;

	for (i = 0; i < cnt; i++)
		if (strcmp(target, list[i]) == 0)
			return (i);
	return (-1);
}
