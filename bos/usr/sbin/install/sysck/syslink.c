static char sccsid[] = "@(#)17  1.8.1.8  src/bos/usr/sbin/install/sysck/syslink.c, cmdinstl, bos411, 9432A411a 8/6/94 17:46:23";
/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: ck_hard_link, ck_soft_link, ck_links, 
 *            ck_symlinks, fix_owngrp, ck_symlink_type, 
 *            ck_link_type
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

#include <sys/mode.h>
#include <sys/fullstat.h>
#include <sys/audit.h>
#include <sys/acl.h>
#include <sys/priv.h>
#include <sys/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include "sysaudit.h"

extern  char    *deleteRootPath(char *);

/*
 * External variables, flags, and function calls
 */

/*
 * NAME:	ck_hard_link
 *
 * FUNCTION:	Test a pair of files for being hard linked.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	If the two files are not hard linked this function will
 *	attempt to hard link them.  If a target exists but is not
 *	a link, the target will be removed.
 *
 * RETURNS:
 *	Zero for success, non-zero otherwise
 */

static int
ck_hard_link (char *a, char *b)
{
	struct	fullstat stat1;
	struct	fullstat stat2;
	int	need_link = 0;
	int	errors = 0;

	/* must be an absolute pathname */

	if (*b != '/') {
                MSG_S(MSG_Absolute_Link,DEF_Absolute_Link,b,0,0,0);
		return -1;
	}
	fullstat (a, FL_STAT|FL_NOFOLLOW, &stat1);

	/*
	 * The destination must not exist, or must be a link to the
	 * source file already.
	 */

	if (fullstat (b, FL_STAT|FL_NOFOLLOW, &stat2)) {
		errors++;

		need_link++;
	} else if (stat1.st_dev != stat2.st_dev ||
			stat1.st_ino != stat2.st_ino) {
		errors++;

			if (xremove (b, &stat2)) {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					a, CantFix, "unlink %s", b, NULL);
				return -1;
			} else {
				mk_vaudit (SYSCK_Check, AUDIT_OK,
					a, Fixed, "unlink %s", b, NULL);
			}
			need_link++;
	}

	/*
	 * If the link does not exist, or the destination file was
	 * removed, the link must be created.
	 */

	if (need_link) {
			if (xlink (a, b)) {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					a, CantFix, "link %s %s", a, b, NULL);
				return -1;
			} else
				mk_vaudit (SYSCK_Check, AUDIT_OK,
					a, Fixed, "link %s %s", a, b, NULL);
	}
	return errors ? ENOTRUST:0;
}

/*
 * NAME:	ck_soft_link
 *
 * FUNCTION:	Test for a soft link between two files.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	If the target does not exist, a symbolic links shall be
 *	created.  If the target does exist, but is not a symbolic
 *	link to the source, the target shall be removed and
 *	recreated as a link to the source.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

static int
ck_soft_link (char *a, char *b, struct  tcbent  *tcbent)
{
	char	link[PATH_MAX];	/* buffer to read symbolic link into */
	int	length;		/* length of symbolic link name */
	int	need_link = 0;	/* a link needs to be created */
	int	errors = 0;	/* count of unrecoverable errors in here */
	struct	fullstat stat;	/* stat buffer for file link testing */

	/* must be an absolute pathname */
	 
	if (*b != '/') {
                MSG_S(MSG_Absolute_Link,DEF_Absolute_Link,b,0,0,0);
		return -1;
	}

	/*
	 * The destination must either not exist, or already be a
	 * symbolic link pointing to the destination file.
	 */

	if (fullstat (b, FL_NOFOLLOW|FL_STAT, &stat)) {
		need_link++;
	} else {

		/*
		 * File and symbolic link both exist.  Read the
		 * contents of the link and verify they are correct.
		 */

		if ((length = readlink (b, link, BUFSIZ)) == -1) {
			errors++;

				if (xremove (b, &stat)) {
					mk_vaudit (SYSCK_Check, AUDIT_FAIL,
						a, NotFixed,
						"unlink %s", b, NULL);
					return -1;
				} else {
					mk_vaudit (SYSCK_Check, AUDIT_OK,
						a, Fixed, "unlink %s", b, NULL);
				}
			need_link++;
		} else {
			if (length > 0)
				link[length] = '\0';
			else
				link[0] = '\0';

                     if (strcmp (deleteRootPath(a), link) != 0) {
				errors++;

					if (xremove (b, &stat)) {
						mk_vaudit (SYSCK_Check,
							AUDIT_FAIL, a, NotFixed,
							"unlink %s", b, NULL);
						return -1;
					} else {
						mk_vaudit (SYSCK_Check,
							AUDIT_OK, a, Fixed,
							"unlink %s", b, NULL);
					}
				need_link++;
			}
		}
	}

	/*
	 * A new symbolic links needs to be created either because
	 * the old one was removed or because there was none to begin
	 * with.
	 */

	if (need_link) {
			if (xsymlink (deleteRootPath(a), b)) {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL, a, CantFix,
					"symlink %s %s", a, b, NULL);
				return -1;
			} else {
				mk_vaudit (SYSCK_Check, AUDIT_OK, a, Fixed,
					"symlink %s %s", a, b, NULL);

			        if (fix_owngrp ( b, tcbent->tcb_owner,
						 tcbent->tcb_group ))
					errors++;
			}
	}
	return errors ? ENOTRUST:0;
}

/*
 * NAME:	ck_links
 *
 * FUNCTION:	Test hard links for a TCB entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	Counts the number of links and calls ck_hard_links to
 *	validate each one.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_links (struct tcbent	*tcbent, struct	fullstat *sb)
{
	char	*file = tcbent->tcb_name;
	int	nlinks;
	int	i;
	int	rc;
	int	errors = 0;
	int	severe = 0;

	/*
	 * Count the links in the TCB entry
	 */

	for (nlinks = 0;tcbent->tcb_links[nlinks];nlinks++)
		;

	nlinks++;	/* add one for the object itself */

	/*
	 * Directories may not have links specified.
	 */

	if (S_ISDIR (sb->st_mode)) {
                MSG_S(MSG_Linked_Directory,DEF_Linked_Directory,file,0,0,0);
		return ENOTRUST;
	}

	/*
	 * Each link in the list is now tested for validity.  The
	 * inode data is compared to see if the two filenames refer
	 * to the same filesystem object.
	 */

	for (i = 0;tcbent->tcb_links[i];i++) {
		if (rc = ck_hard_link (file, tcbent->tcb_links[i])) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * There should be one more link than listed in the TCB
	 * entry - that is for the file itself.  Any more or less
	 * gets an error message.
	 */

	if (nlinks != sb->st_nlink) 
		errors++;

	if (severe)
		return -1;

	return errors ? ENOTRUST:0;
}

/*
 * NAME:	ck_symlinks
 *
 * FUNCTION:	Test symbolic links for a TCB entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	Each symbolic link is tested for validity.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_symlinks (struct tcbent *tcbent)
{
        int     i;
        int     rc;
        int     errors = 0;
        int     severe = 0;

        /*
         * Step through the list of symbolic links and test each
         * link for correctness.  Unknown symbolic links are
         * ignored, except in tree-checking mode.  We can't find
         * out how many symbolic links there are otherwise.
         */

        for (i = 0;tcbent->tcb_symlinks[i];i++) {
                if (rc = ck_soft_link (tcbent->tcb_name,
                                       tcbent->tcb_symlinks[i],
                                       tcbent)) {
                        if (rc == -1)
                                severe++;

                        errors++;
                }
        }
        if (severe)
                return -1;

        return errors ? ENOTRUST:0;

}

/*
 * NAME:        fix_owngrp
 *
 * FUNCTION:    Set the owner and group for a symlink to match destination
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 * NOTES:
 *      When a symlink is created the owner and group are not set
 *      to match the destination file.  This function is called to
 *      do this.
 *
 * RETURNS:     Zero for success, non-zero otherwise
 */

static int
fix_owngrp (char    *file,
            uid_t   uid,                 /* User ID of the file's owner */
            gid_t   gid)                 /* Group ID of the file's group */
{

	if (xlchown (file, uid, gid)) {
		mk_audit (SYSCK_Check, AUDIT_FAIL,
			file, CantFix, "owner/group");
		return -1;
	} else
		mk_audit (SYSCK_Check, AUDIT_OK,
			file, Fixed, "owner/group");

	return 0;

}

/*
 * NAME:	ck_symlink_type
 *
 * FUNCTION:	Checks to see if the symlinks exists, and creates it
 *              if it doesn't.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_symlink_type (struct tcbent *tcbent)
{
	int	rc;
	int	severe = 0;

	if (rc = ck_soft_link (tcbent->tcb_target,
			       tcbent->tcb_name,
			       tcbent)) {
		if (rc == -1)
			severe++;
	}

	if (severe)
		return -1;
	else
		return 0;

}

/*
 * NAME:	ck_link_type
 *
 * FUNCTION:	Test hard link and create if it doesn't exist.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_link_type (struct tcbent	*tcbent)
{
	char	*file = tcbent->tcb_name;
	int	rc;
	int	severe = 0;

	/*
	 * The link is now tested for validity.  The inode 
	 * data is compared to see if the two filenames 
	 * refer to the same filesystem object.
	 */

	if (rc = ck_hard_link (tcbent->tcb_target,file)) {
		if (rc == -1)
			severe++;

	}

	if (severe)
		return -1;
	else
		return 0;
}

