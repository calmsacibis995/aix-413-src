$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)07    1.1 src/msg/bos/base/time.msg, bosmsg, msg411, GOLD410 2/9/94 09:24:59
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIAL
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$set MS_TIME

NOFORK "time: 0551-009 Cannot create another process.\n"
ABTERM "time: 0551-010 The process was stopped abnormally. Try again.\n"

$
$ For the following three values, make them the same length so
$ the output will align properly
$
REAL "Real  "
$ For this and the following two messages, make them the same length so
$ the output will align properly
$ For example, if System translates to 10 letters, make REAL and USER be
$ 10 letters (include blanks at the end.)

USER "User  "
SYS  "System"
USAGE "Usage: time [-p] utility [arguments...]\n"
