$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)72	1.1  src/msg/bos/base/mkfifo.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:48
$
$ COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_MKFIFO

NOFILE	"mkfifo: must specify file\n"
EMODE	"mkfifo: invalid mode\n"
USAGE	"Usage: mkfifo [-m mode] file ...\n"
