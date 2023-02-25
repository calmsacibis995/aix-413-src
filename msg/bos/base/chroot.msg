$ ENGLISH_SOURCE_VERSION= 1.6.1.2
$ @(#)41	1.1  src/msg/bos/base/chroot.msg, bosmsg, msg411, GOLD410 2/9/94 08:49:20
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
