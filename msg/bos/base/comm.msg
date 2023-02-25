$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)90	1.1  src/msg/bos/base/comm.msg, bosmsg, msg411, GOLD410 2/9/94 08:52:12
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 038 - 044 reserved for this file.
$
$set MS_COMM

CANTOPEN "comm: 0653-038 Cannot find or open %s.\n"
USAGE "Usage: comm [-123] File1 File2\n"
BADFILES "comm: 0653-039 File1 and File2 can not both refer to stdin.\n"
