$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)75   1.1 src/msg/bos/base/ntsc.msg, bosmsg, msg411, 9435B411a 8/30/94 12:45:49
$
$ COMPONENT_NAME: (PCIVCA) PCI Video Capture Adapter Device Driver 
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$set MS_NTSC

USAGE "Usage: ntsc [-v] [on|off]\n"
NTSCON "ntsc on succeeded.\n"
NTSCOFF "ntsc off succeeded.\n"
CANTON "ntsc on failed.\n"
CANTOFF "ntsc off failed.\n"
CANTOPEN "Can not switch to ntsc on while overlay is active.\n"
CANTIOCTL "Already set to ntsc on. Use ntsc off first.\n"                  
