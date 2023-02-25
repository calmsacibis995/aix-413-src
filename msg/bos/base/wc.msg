$ ENGLISH_SOURCE_VERSION= 1.12
$ @(#)69	1.1  src/msg/bos/base/wc.msg, bosmsg, msg411, GOLD410 2/9/94 09:28:38
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
$ $ Message numbers 755 - 764 reserved for this file.
$
$set MS_WC

USAGE "Usage: wc [-c | -m][-lw] [File ...]\n\
       wc -k [-c [-lw]] [File ...]\n"
$ The above is the usage statement.
CANTOPEN "wc: 0653-755 Cannot open %s.\n"
$ %s is the file name.
TOTAL "total\n"
$ When the user uses the wc command on more than one file at a time, the
$ word "total" appears to the right of the total characters, words, and lines
$ of all files specified.
