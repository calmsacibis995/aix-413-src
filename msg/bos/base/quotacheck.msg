$ ENGLISH_SOURCE_VERSION= 1.3.1.2
$ @(#)64	1.1  src/msg/bos/base/quotacheck.msg, bosmsg, msg411, GOLD410 2/9/94 09:13:02
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
$ 	1) Do not translate any occurrences of "quotacheck"
$ 	2) Do not translate any command options preceeded by "-".
$ 	3) Translate any occurrence of "quotas". 
$
 
$ Messages for the quota command
$quote "
$set MS_QUOTACHECK

NOOPEN		"4050-100 Can't open "
NOTFOUND	"4050-101 %s must be configured in %s\n"
CREATE		"Creating quota file %s\n"
$ Traslate "quota"
USE1		"Usage:\n\t%s\n\t%s\n"
USE2		"quotacheck [-g] [-u] [-v] -a"
USE3		"quotacheck [-g] [-u] [-v] filesystem"
$ Translate "filesystem"
$ The Three lines above will appear together
NOMEM		"4050-102 Out of memory for quota structures\n"
$ Traslate "quota"
PRIV		"4050-007 %s: insufficient privileges\n"
CHK		"*** Checking "
AND		" and "
QUOTAS		" quotas for %s (%s)\n"
$ Translate "quotas"
WARN1		"4050-104 *** Warning: %s\n"
WARN2		"4050-105 Quotas are not enabled on this machine"
$ Translate "Quotas"
FIX		"%-8s fixed:"
INO		"\tinodes %d -> %d"
$ Do not translate "inodes"
BLK		"\tblocks %d -> %d"
$ Do not translate "blocks"
$ "block" is a partition of disk space
NOMEMU		"4050-106 Out of memory for fileusage structures\n"
$ Do not translate "fileusage"
BADFSIZE	"4050-107 super block has corrupted filesystem size\n"
CANTREAD 	"4050-108 Cannot read super block on filesystem %s\n"
QLARGEID	"4050-109 User id %u (%s) is too large, quotas are not maintained\n"

