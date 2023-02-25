$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)29	1.1  src/msg/bos/base/skulker.msg, bosmsg, msg411, GOLD410 2/9/94 09:17:39
$
$ COMPONENT_NAME: (CMDCNTL) system control commands
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
$quote "
$set MS_SKULKER


1 "\n%1$s started at %2$s on %3$s\n"
2 "\n%1$s finished at %2$s on %3$s\n"
$ NOTE
$ NOTE: The %1$s is the name of the program (e.g. skulker)
$ NOTE: The %2$s is a time (the output of the 'date' command)
$ NOTE: The %3$s is a machine name (the output of the 'uname' command)
