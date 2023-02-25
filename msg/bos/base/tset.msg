$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)16	1.2  src/msg/bos/base/tset.msg, bosmsg, msg411, GOLD410 4/7/94 12:04:57

$ COMPONENT_NAME: CMDTTY terminal control commands
$
$ FUNCTIONS: .msg file for tset
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ LEVEL 1, 5 Years Bull Confidential Information
$ 

$set MS_TSET

$quote "

USE "usage: tset [-] [-rsIQS] [-eC] [-kC] [-iC] [-m [ident][test speed]:type] [type]\n"
NOTTERM "Not a terminal\n"
BADFLAG "Bad flag -"
FIND "Cannot find termcap\n"
TYPE "Type "
UNKNOWN " unknown\n"
TERMTYPE "Terminal type is "
IS " is "
SETTO " set to "
CANTOPEN "Cannot open "
TOOFEW "Too few args: "
UNEXPECT "Unexpected arg: "
BADEXE "Could not find executable for iprog: "
BADFORK "Error forking process\n"
