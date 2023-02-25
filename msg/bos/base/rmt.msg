$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)95	1.1  src/msg/bos/base/rmt.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:01
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_RMT
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate rmt -- this is the command name.
$ Do not translate %d %s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message.
$ The program replaces %d,%1$d,etc with a decimal number.
$ The program replaces %s,%1$s,etc with a string of
$ characters.

EOFM "rmtd: premature EOF\n"

USAGE "rmtd: invalid option %1$c\n"

NALC "rmtd: cannot allocate buffer space\n"
