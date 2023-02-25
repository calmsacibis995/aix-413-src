$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)68 1.3 src/msg/bos/base/stty.msg, bosmsg, msg411, GOLD410 4/14/94 09:43:19
$
$ COMPONENT_NAME: CMDTTY terminal control commands
$
$ FUNCTIONS: .msg file for stty
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$

$set MS_STTY

$quote "

USAGE "usage: stty [-ag] [modes]\n"
UNKNOWN "unknown mode: %s\n"
SPLBAUD "input %s / output %s baud; "
ROWS "%d rows; %d columns; "
BAUD "speed %s baud; "
PVDISABLE "_POSIX_VDISABLE not in effect for the device.\n"
