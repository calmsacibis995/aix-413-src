$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)74	1.1  src/msg/bos/base/syncd.msg, bosmsg, msg411, GOLD410 2/9/94 09:22:12  
$
$ COMPONENT_NAME: (CMDFS) file system commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_SYNCD        Define initial set#
$ $ The messages have been assigned a range of 380-384.

$ Do not translate syncd.

USG                  "Usage: syncd time\n"

BADCHAR              "syncd: 0506-380 The parameter must be a number.\n"
