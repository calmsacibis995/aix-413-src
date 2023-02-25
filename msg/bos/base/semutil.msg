$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)19	1.1  src/msg/bos/base/semutil.msg, bosmsg, msg411, GOLD410 2/9/94 09:16:40
$ 
$ COMPONENT_NAME: CMDSEND semutil.msg
$ 
$ FUNCTIONS: 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for semutil
$

$set MS_SEMUTIL

$quote "

ARG             "argument count error\n"
MEM             "memory allocation error\n"
LOCK            "Lockfile path: %s\n"
KEY             "key = 0x%lx\n"
SEM             "semaphore removed\n"
UNKNOWN         "unknown command code %s\n"
$   New message
$   Do not translate the words, semutil, value, and remove.
SM_USAGE	"usage: semutil {value | remove} [<queue_directory_path>]\n"
