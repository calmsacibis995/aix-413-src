$ ENGLISH_SOURCE_VERSION= 1.4.1.1
$ @(#)80	1.1  src/msg/bos/base/repquota.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:00
$
$ COMPONENT_NAME: (CMDQUOTA) 
$
$ FUNCTIONS:
$
$ ORIGINS: 18, 27
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ Message to Translators:
$ 	1) Do not translate any occurrences of "repquota"
$ 	2) Do not translate any command options preceded by "-".
$ 	3) Translate any occurrence of "quotas". 
$

$ Messages for the edquota command
$quote "
$set MS_REPQUOTA

NOFSTAB		"4050-500 %s not found in /etc/filesystems\n"
$ Do not translate "/etc/filesystems"
USE1		"repquota [-v] [-g] [-u] -a"
USE2		"repquota [-v] [-g] [-u] filesystem"
$ Translate "filesystem"
NOQUOTA		"4050-501 *** Warning: Quotas are not enabled on this machine\n"
$ Translate "Quotas"
REPORT		"*** Report for %s quotas on %s (%s)\n"
$ Translate "quotas"
LIMITS		"                        Block limits               File limits\n"
$ "block" is a partition of disk space
MSGHD		"User            used    soft    hard  grace    used  soft  hard  grace\n"
$ "grace period" is time/space allotted for error correction
DAYS		"%ddays"
