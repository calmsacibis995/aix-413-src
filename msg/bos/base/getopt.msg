$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)21        1.1  src/msg/bos/base/getopt.msg, bosmsg, msg411, GOLD410 2/9/94 08:59:42
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 71
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.1
$
$quote "

$set MS_GETOPT
$ Do not translate getopt, ASCII.

USAGE   "Usage: getopt Flag-string Command-string\n"
NONLSCH "getopt: 0402-110 Only ASCII characters are permitted in option string.\n"
OVERFLOW "getopt: 0402-111 Arguments exceed implementation limit of %d bytes.\n"
