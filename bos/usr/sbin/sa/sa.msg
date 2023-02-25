$ @(#)89    1.7  src/bos/usr/sbin/sa/sa.msg, cmdstat, bos411, 9428A410j 3/11/92 12:45:01
$
$ COMPONENT_NAME: (CMDSTAT) status
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
$
$ messages for sa
$

$quote "
$set MS_SA

LARGEUID     "sa: 0551-092 (WARNING) uid %u (%d) is too large.\n\tIt can NOT be written in file %s\n"

LARGEGAP     "sa: 0551-093 (WARNING) large uid gap while reading file %s, still processing.\n\tCurrent id is %u (%d), Maxid is %u (%d)\n"

MALLOCFAIL   "sa: 0551-094 (FATAL) malloc() failed\n"
REALLOCFAIL  "sa: 0551-095 (FATAL) realloc() failed\n"
EMPTYPASSWD  "sa: 0551-096 (FATAL) passwd file is empty\n"

BADID        "sa: 0551-097 The user id %u (%d) is not recognized. It is ignored.\n"

USOPT        "sa: 0551-098 The -U and -S flags must be separate.\n"
INVOPT       "sa: 0551-099 The %c flag is not valid.\n"
NOFILENM     "sa: 0551-100 There is a missing filename.\n"
CANTSAVE     "sa: 0551-101 Cannot save.\n"
ONEFILE      "sa: 0551-102 Specify only 1 file with the -s flag.\n"
CANTOPEN     "sa: 0551-103 Cannot open %s.\n"

CATEGOR      "Categorizing commands used %d times or fewer as **junk**\n"
$ Do not translate **junk**
BIGUID          "sa: 0551-104 uid %u is too large.\n\tIt can NOT be written in file %s.\n\tYou can use -C instead of -s to write the summary file \n\t in compressed format.Exiting ...\n"
UNEXP		"sa: 0551-105 Unexpected failure on fseek\n"
