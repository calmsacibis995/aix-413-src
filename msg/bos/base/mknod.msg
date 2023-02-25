$ ENGLISH_SOURCE_VERSION= 1.6.2.3
$ @(#)78        1.1  src/msg/bos/base/mknod.msg, bosmsg, msg411, GOLD410 2/9/94 09:08:07
$
$ COMPONENT_NAME: (CMDFILES) commands that deal with the file system
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
$ $ Message numbers 360 - 366 reserved for this file.
$
$set MS_MKNOD
$ note: the numbers for the mknod command

USRUSAGE	"Usage: mknod Name p\n"
$ This Usage message is displayed to users who are not in the system group.
$ Those users may only use  mknod  to create FIFO pipes.
SYSUSAGE	"Usage: mknod Name {b|c} Major Minor\n\
\tmknod Name p\n"
$ This Usage message is displayed to users who belong to the system group.
