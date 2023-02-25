$ ENGLISH_SOURCE_VERSION= 1.8.1.1
$ @(#)79        1.1  src/msg/bos/base/renice.msg, bosmsg, msg411, GOLD410 2/9/94 09:13:57
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
$
$ messages for command renice
$

$quote "
$set MS_RENICE

$
$ The following string gives the syntax of the renice command.
$
$ USAGE2: POSIX 1003.2
$
$
USAGE2	"Usage: %s [ -n Increment ] [ -g|-p|-u ] ID ...\n"
$ The %s is replaced by the commane name (renice)
$ Do not translate the flags, "-p", "-g", "-u"

$
$ The following message is for the error message.
$       0509-038, 0509-039, 0509-040, 0509-41, 059-42
$
$       $ XXXX-XXX                        message number
$       MSGID "renice: XXXX-XXX ......."  messages
$       $ ......                          note for translating
$
$ 0509-038
UNK_USER        "%1$s: 0509-038 User %2$s is not known.\n"
$ The first %s is replaced by the commane name (renice).
$ The 2nd %s is replaced by a user name.
$ 0509-039
BADVALUE        "%1$s: 0509-039 The process or process group %2$s is not an integer.\n"
$ The first %s is replaced by the command name (renice).
$ 0509-040
GETPRIO         "%1$s: 0509-040 %2$d: "
$ Do not translate
$ There should not be a trailing newline.
$ 0509-041
SETPRIO         "%1$s: 0509-041 %2$d: "
$ Do not translate
$ There should not be a trailing newline.

$ message with success
NEWPRI          "%1$d: old priority %2$d, new priority %3$d \n"
$ Only the words old priority and new priority can be translated.
$ The first %d is replaced by user ID.
$ The 2nd %d is replaced by old priority.
$ The 3nd %d is replaced by new priority.
MISSING_ID	"%1$s: 0509-042 Specify one or more IDs.\n"
$ The first %s is replaced by the command name (renice).
$ Do not translate "IDs"
