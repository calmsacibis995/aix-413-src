$ ENGLISH_SOURCE_VERSION= 1.1.1.1
$ @(#)57	1.1  src/msg/bos/base/strconf.msg, bosmsg, msg411, GOLD410 2/9/94 09:20:24
$
$ COMPONENT_NAME: (CMDPSE) STREAMS commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for strconf.
$ Do not translate strconf.
$ Message numbers have been assigned a range of XXX - YYY
$
$quote "		define quote character.
$set MS_STRCONF		Define initial set#

MSG1	"-t and -m are mutually exclusive"

$ DO NOT TRANSLATE FOLLOWING LINE
MSG2	"stdin"

MSG3	"no modules on stream"

$ DO NOT TRANSLATE FOLLOWING LINES
MSG4	"ioctl I_LOOK"
MSG5	"no\n"
MSG6	"yes\n"

MSG7	"unknown module <%s>"

$ DO NOT TRANSLATE FOLLOWING LINE
MSG8	"ioctl I_FIND"

MSG10	"cannot determine number of modules"
MSG11	"cannot get names of modules"
MSG12	"%s: %d modules in stream, only %d reported"

$ DO NOT TRANSLATE "minor"
USAGE1	"usage: %s [-c <minor>] [-t | -m <module>]\n"
USAGE11	"usage: %s [-t | -m <module>]\n"
