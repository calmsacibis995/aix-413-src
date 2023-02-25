$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)56	1.1  src/msg/bos/base/lsvfs.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:00  
$
$  COMPONENT_NAME: (CMDFS) commands that manipulate files
$
$  FUNCTIONS: message file
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_LSVFS        Define initial set#

$ $ The messages have been assigned a range of 360-375
$ Do not translate lsvfs.
$ Do not translate %s, %d, %c, etc.

OOPS                 "%s: 0506-360 This program should be called "

OR                   "or "

DEFAULT              ". The default is %s.\n"

STANZAEXISTS         "%s: 0506-361 Vfs %s already exists.\n"

BADNAME              "%s: 0506-362 The name field in the vfs line is not valid.\n"

BADNUMB              "%s: 0506-363 The number field in the vfs line is not valid.\n"

BADMOUNT             "%s: 0506-364 The mount field in the vfs line is not valid.\n"

BADFILSYS            "%s: 0506-365 The file system field in the vfs line is not valid.\n"

CANTOPEN             "%s: 0506-366 Cannot find or open %s: "
$ This message should not have a new line.   

MEMERR               "%s: 0506-367 There is not enough memory available now.\n"

NOMATCH              "%1$s: 0506-368 No record matching %2$s was found in %3$s.\n"

LS_USAGE             "Usage: lsvfs [ -a | Vfs Name ]\n"

CR_USAGE             "Usage: crvfs Vfs Entry.\n"

CH_USAGE             "Usage: chvfs Vfs Entry.\n"

RM_USAGE             "Usage: rmvfs Vfs Name.\n"

BADVFS               "%s: 0506-369 The %s vfs entry is not valid.\n"

UPDATFAIL	     "%s: update of %s failed\n"
