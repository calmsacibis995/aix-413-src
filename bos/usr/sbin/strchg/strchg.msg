$ @(#)49	1.1  src/bos/usr/sbin/strchg/strchg.msg, cmdpse, bos411, 9428A410j 5/18/91 20:13:03
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
$ Messages for strchg.
$ Do not translate strchg.
$ Message numbers have been assigned a range of XXX - YYY
$
$quote "		define quote character.
$set MS_STRCHG		Define initial set#

$ do not translate stdin
MSG1	"stdin"

MSG2	"-a and -u are mutually exclusive"

$ do not translate
MSG3	"ioctl I_PUSH"

MSG4	"no modules on stream"

$ do not translate
MSG5	"ioctl I_POP"
MSG6	"ioctl I_LOOK"

MSG7	"no modules on stream"
MSG8	"%s not found; all modules popped"

$ do not translate
MSG9	"ioctl I_POP"

MSG10	"no modules on stream"

$ do not translate
MSG11	"ioctl I_POP"

$ do not translate open
MSG12	"cannot open %s"

MSG13	"out of memory"
MSG14	"out of memory"

$ do not translate
MSG15	"ioctl I_PUSH"

$ note:
$	The %s in all three USAGE messages must line up
$	after translation.
USAGE1	"usage: %s -h <module1>[,<module2>,...]\n"
USAGE2	"       %s -p [-a | -u <module1>]\n"
USAGE3	"       %s -f <file>\n"
