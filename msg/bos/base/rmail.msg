$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)89	1.1  src/msg/bos/base/rmail.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:34
$ 
$ COMPONENT_NAME: CMDMAILX rmail.msg
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
$ messages for rmail
$

$set MS_RMAIL

$quote "


USAGE "Usage: rmail User ...\n\
\tHandles remote mail received through the uucp services.\n"
$ do not translate the command name rmail, or uucp
PCLOSE "Return status from pclose is 0%o.\n"
$ 0%o is replaced by an octal number
$ Do not translate pclose.
