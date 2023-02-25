$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)43	1.1  src/msg/bos/base/lprm.msg, bosmsg, msg411, GOLD410 2/9/94 09:06:27
$
$ COMPONENT_NAME: (CMDQUE) spooling commands
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_LPRM
MSGUSE1	"[-P Printer] [-] [JobNumber] [UserName ...]"
MSGUSE2 "Removes jobs from the line printer spooling queue."
$ do not translate the command word lprm or the flag -P
