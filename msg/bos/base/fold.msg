$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)05	1.1  src/msg/bos/base/fold.msg, bosmsg, msg411, GOLD410 2/9/94 08:58:49
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for fold
$
$quote "
$
$ $ Message numbers 197 - 199 reserved for this file.
$
$set MS_FOLD

USAGE  "Usage: fold [-bs] [-w Width] [File...]\n"
ILLNUM "fold: 0653-197 Specify a number for the width.\n"
BADFIL "fold: 0653-198 Cannot open file %s\n"
NOHYPH "fold: 0653-199 Cannot use '-' for standard input.\n"
$ Do not translate fold. This is the command name.
