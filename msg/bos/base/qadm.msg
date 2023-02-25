$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)52	1.1  src/msg/bos/base/qadm.msg, bosmsg, msg411, GOLD410 2/9/94 09:12:23
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
$set MS_QADM
MSGUSE1	"[-G] | [-XPrinter] | [-DPrinter] | [-KPrinter] | [-UPrinter]"
MSGUSE2 "Configures print queues and queue devices."
$ Do not translate the command word qadm or the flags GXDKU

