$ ENGLISH_SOURCE_VERSION= 1.2.1.1
$ @(#)66        1.1  src/msg/bos/base/ac.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:40
$
$  COMPONENT_NAME: (CMDACCT) Command Accounting
$ 
$  FUNCTIONS: none
$ 
$  ORIGINS: 9, 27
$ 
$  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$  All Rights Reserved
$  Licensed Materials - Property of IBM
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$
$ messages for ac
$ $ Message numbers 100 - 125 reserved for this file.
$set MS_AC
$quote	"
NOFILE	"ac: 0850-100 No %s\n"
BADTTY	"ac: 0850-101 Bad tty name: %s\n"
TOTAL	"\ttotal%9.2f\n"
USAGE	"Usage: ac [ -dp ] [ -w wtmp_file ] [ user ]\n"
