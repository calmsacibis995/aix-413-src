$ ENGLISH_SOURCE_VERSION= 1.8
$	 	"@(#)71	1.1  src/msg/bos/base/mkdir.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:45"
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS: Messages for mkdir
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 355 - 359 reserved for this file.
$
$set MS_MKDIR
$ note: the numbers for the mkdir command
$ note: start at 0653-355
$ Do not translate -%c.  
USAGE    "Usage: mkdir [-p] [-m mode] Directory ...\n"
$
NOMULT	 "mkdir: 0653-355 Cannot have multiple -%c options.\n"
$
NOACCESS "mkdir: 0653-357 Cannot access directory %s.\n"
$
NOMAKE   "mkdir: 0653-358 Cannot create %s.\n"
