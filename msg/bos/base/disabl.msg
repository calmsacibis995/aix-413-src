$ ENGLISH_SOURCE_VERSION= 1.5.1.1
$ @(#)57	1.1  src/msg/bos/base/disabl.msg, bosmsg, msg411, GOLD410 2/9/94 08:56:20
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_DISABL
MSGUSE1	"[-c] [-rReason] PrinterName ..."
MSGUSE2 "Disables printers."
$ do not translate "disable" or the flags -cr
