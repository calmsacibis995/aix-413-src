$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)31        1.1  src/msg/bos/base/sleep.msg, bosmsg, msg411, GOLD410 2/9/94 09:17:53
$
$ COMPONENT_NAME: (CMDCNTL) system control commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 18
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ @OSF_COPYRIGHT@
$
$ RCSfile: sleep.msg,v Revision: 1.4 (OSF) Date: 90/10/07 17:03:17 
$
$quote "

$set MS_SLEEP
$ Do not translate sleep

USG "Usage: sleep Seconds\n"
$
BADCHAR "sleep: 0509-020 Specify time as a positive integer.\n"
