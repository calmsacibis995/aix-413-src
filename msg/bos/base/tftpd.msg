$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)04	1.1  src/msg/bos/base/tftpd.msg, bosmsg, msg411, GOLD410 2/9/94 09:24:49
$ 
$ COMPONENT_NAME: TCPIP tftpd.msg
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
$ messages for tftpd 
$ The range of message numbers assigned to this file are 601-614
$
$len 
$quote "         define quote character.
$set MS_TFTPD        Define initial set#
$ Do not translate tftpd, %s, %d, %c, %1$s, %2$s etc ...

BAD_BUS              "tftpd: 0827-601 Cannot open the bus. The return code is %d.\n"
$ Do not translate bus.
UNDEFERR             "tftpd: 0827-602 The error code is not defined."
BADFILE              "tftpd: 0827-603 Cannot find the specified file."
ILLACESS             "tftpd: 0827-604 There is a disk access violation."
DISKFULL             "tftpd: 0827-605 The disk is full or the allocation is exceeded."
ILLOPE               "tftpd: 0827-606 A tftp operation is not valid."
$ Do not translate TFTP.
BADID                "tftpd: 0827-607 A transfer identifier is not known."
FILEXIST             "tftpd: 0827-608 The specified file already exists."
BADUSER              "tftpd: 0827-609 The user does not exist."
MAXTIME              "tftpd: 0827-610 timer: the specified timeout period is reached.\n"
SENDERR1             "tftpd: 0827-611 Cannot write. The error is %m.\n"
RECERR1              "tftpd: 0827-612 Cannot read. The error is %m.\n"
SENDERR2             "tftpd: 0827-613 Cannot write. The error is %m.\n"
RECERR2              "tftpd: 0827-614 Cannot read. The error is  %m.\n"
LOGSND               "Sent %1$s to %2$s.\n"
LOGRCV               "Received %1$s from %2$s\n"
SETDEBUG             "setsockopt (SO_DEBUG): %m"
$ Do Not Translate 
