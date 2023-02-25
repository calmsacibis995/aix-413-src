$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)10	1.1  src/msg/bos/base/no.msg, bosmsg, msg411, GOLD410 2/9/94 09:09:41
$ @(#) 10  1.1  src/msg/bos/base/no.msg, bosmsg, msg411, GOLD410 2/9/94 09:09:41 
$
$ Messages for no.c 
$
$ COMPONENT_NAME:  CMDNET   Network Configuration  
$
$ ORIGINS: 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ Message numbers have been assigned a range of 054-063.
$
$ len 
$quote "         define quote character.
$set MS_NO        Define initial set#

BADSOCK "0821-055 no: Cannot create a socket. The error number is %d.\n"
$ Do not translate socket.
DEFFAIL "0821-056 no: The ioctl SIOCGNETOPT system call failed.\n"
$ Do not translate ioctl, SIOCDNETOPT.
GETFAIL "0821-057 no: The ioctl SIOCGNETOPT system call failed.\n"
$ Do not translate ioctl, SIOCGNETOPT.
NOTROOT "0821-058 no: Only the root user can set network options.\n"
SETFAIL "0821-059 no: The ioctl SIOCSNETOPT system call failed.\n"
$ Do not translate ioctl, SIOCSNETOPT.
USAGE1 "\nusage:%1$s -o option[%2$cnewvalue] [ -o ... ]\n"
USAGE2 "\t%s -d option\n"
USAGE3 "\t%s -a\n"
