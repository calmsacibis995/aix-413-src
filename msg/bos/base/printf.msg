$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)41	1.1  src/msg/bos/base/printf.msg, bosmsg, msg411, GOLD410 2/9/94 09:11:38
$
$ COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_PRINTF
$
$ Message numbers 001 - 020 reserved for this file.
$ Do not translate printf.

BADNUM	"printf: 3016-001 Specify a number instead of %s.\n"
NOCOMP	"printf: 3016-002 %s is not completely converted.\n"
BADOCT	"printf: 3016-003 Specify an octal number instead of %s.\n"
SPRTFERR "printf: 3016-004 Cannot form a valid conversion.\n"
LONGLINE "printf: 3016-005 A line cannot be longer than 2048 bytes.\n"
