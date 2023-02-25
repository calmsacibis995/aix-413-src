$ ENGLISH_SOURCE_VERSION= 1.10
$ @(#)07	1.1  src/msg/bos/base/nice.msg, bosmsg, msg411, GOLD410 2/9/94 09:09:32
$
$ COMPONENT_NAME: (CMDCNTL) system control commands
$
$ FUNCTIONS:
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
$set MS_NICE

$
$ The following string gives the syntax of the nice command.
$
$ NOTE: The usages of nice command are printed as follows.
$       USAGE1
$       USAGE2
USAGE1 "Usage: nice [-n Increment] Command [Argument...]\n"
USAGE2 "Usage: nice [-Increment] Command [Argument...]\n"
$ Do not translate the command name "nice"
$ Do not translate the flag "-n"

$
$ The following message is for the error message.
$       0509-018
$
$       $ XXXX-XXX                 	message number
$       MSGID "nice: XXXX-XXX ......."  messages
$       $ ......                   	note for translating
$
$ 0509-018
MSGBNUM "nice: 0509-018 Flag must be numeric.\n"
$ Do not translate the command name "nice"

