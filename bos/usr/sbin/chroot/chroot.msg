$ @(#)58	1.6.1.2  src/bos/usr/sbin/chroot/chroot.msg, cmdfiles, bos411, 9428A410j 8/3/93 17:14:35
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $  Message numbers 015 - 022 reserved for this file
$
$set MS_CHROOT
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate chroot -- this is the command name.
$ Do not translate %o %s  %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message.

USAGE "Usage: chroot Directory Command\n"
CANTCHROOT "chroot: 0653-015 Cannot change to the specified new root directory.\n"
$ chroot can change a "command" to a new root but it
$ cannot change a directory to a new root.
NOTFOUND "chroot: 0653-016 Cannot find or run %s.\n"
