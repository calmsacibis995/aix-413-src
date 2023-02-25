$ ENGLISH_SOURCE_VERSION= 1.10
$ @(#)93	1.1  src/msg/bos/base/backup.msg, bosmsg, msg411, GOLD410 2/9/94 08:46:55
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_BACKUP

EXECF "backup: 0511-470 Cannot run %s"
$ %s is the name of a program.
$ This message is passed to the perror() function which appends a 
$ colon (:) followed by an error message and a newline.

OPTBAD "backup: 0511-471 Cannot use the %1$c flag with the %2$c flag.\n"
$ %1$c is a flag.
$ %2$c is a flag.

USAGE1 "Usage for backup by name: \n\
    backup  -i[oqv] [-b Number] [-f Device] [-l Number] [-p[e RegExp]]\n"

USAGE2 "Usage for backup by inode: \n\
    backup [-Level] [-b Number] [-f Device] [-u] [-c] [-w|-W] [Filesystem]\n"
