$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)66	1.1  src/msg/bos/base/stty-lion.msg, bosmsg, msg411, GOLD410 2/9/94 09:21:28

$
$ COMPONENT_NAME: CMDTTY terminal messuages for tty
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

$
$ messages for command sysline
$

$set MS_STTY_LION

$quote "

UNKNOWN    "%s: %s unknown\n"
TBCUSAGE   "Usage: %s tbc level\n"
GT1USAGE   "Usage: %s goto1 string\n"
GT2USAGE   "Usage: %s goto2 string\n"
SC1USAGE   "Usage: %s screen1 string\n"
SC2USAGE   "Usage: %s screen2 string\n"
INXUSAGE   "Usage: %s in_xpar string\n"
LVXUSAGE   "Usage: %s lv_xpar string\n"
PRXUSAGE   "Usage: %s priority level\n"
