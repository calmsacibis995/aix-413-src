$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)26	1.1  src/msg/bos/base/pathchk.msg, bosmsg, msg411, GOLD410 2/9/94 09:10:35
$
$ COMPONENT_NAME: (CMDPOSIX)
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1990
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ messages for command head
$
$quote "         define quote character.
$len
$set MS_PATHCHK        Define initial set#
USAGE		"Usage: pathchk [-p] filename(s)\n"
POSIX_PATH	"pathchk: Pathname %s exceeds _POSIX_PATH_MAX %d\n"
POSIX_NAME	"pathchk: Name %s exceeds _POSIX_NAME_MAX %d\n"
CHAR_SET	"is not in the portable filename character set for %s\n"
PATHN_MAX	"pathchk: Pathname %s exceeds PATH_MAX %d\n"
NAME_MAX	"pathchk: Name %s exceeds NAME_MAX %d\n"
$ Do not translate pathchk.
