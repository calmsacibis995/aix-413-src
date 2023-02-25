$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)74    1.1 src/msg/bos/base/whoami.msg, bosmsg, msg411, GOLD410 2/9/94 09:28:51
$
$ COMPONENT_NAME: (CMDSTAT) status
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

$quote "         define quote character.

$len
$set MS_WHOAMI        Define initial set#

INTRUDER             "whoami: 0551-300 The user name is not recognized.\n"
$ Do not translate whoami
$ This warning notifies you that a user who is not in the /etc/passwd file
$ has gained access to your system.
