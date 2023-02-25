$ @(#)20        1.10  src/bos/usr/sbin/killall/killall.msg, cmdcntl, bos411, 9428A410j 4/25/91 17:15:26
$
$ COMPONENT_NAME: (CMDCNTL) system control commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
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
$quote "
$set MS_KILLALL
$ Do not translate killall

USAGE "Usage: killall [-] [-SignalName|-SignalNumber]\n"
NOPRTBL "0509-016 Cannot find the process table.\n"
NOANC "0509-017 Cannot find an ancestor process; will retry one time.\n"
$ The killall command will try one more time. This message will come
$ out again if it fails again, but there will be no more retries.
