$ ENGLISH_SOURCE_VERSION= 1.9
$ @(#)24	1.1  src/msg/bos/base/paste.msg, bosmsg, msg411, GOLD410 2/9/94 09:10:28
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
$quote "
$
$ $ Message numbers 480 - 489 reserved for this file.
$
$set MS_PASTE
$ Do not translate paste

M_NODEL "paste: 0653-480 Specify -d \"\\0\" for null delimiters.\n"
M_USAGE "Usage: paste [-s] [-d List] File...\n"
M_NOPEN "paste: 0653-481 Cannot open file %s.\n"
$ Translator:  %s is a file name.
M_2MANY "paste: 0653-482 Specify fewer than 2000 file names.\n"
M_DIR   "paste: 0653-483 Cannot open file %s, because it is a directory.\n"
