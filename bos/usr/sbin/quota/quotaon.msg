$ @(#)31	1.3.1.1  src/bos/usr/sbin/quota/quotaon.msg, cmdquota, bos411, 9428A410j 4/21/93 11:20:04
$
$ COMPONENT_NAME: CMDQUOTA
$
$ FUNCTIONS: 
$
$ ORIGINS: 18, 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ Message to Translators:
$ 	1) Do not translate any occurrences of "quotaon", "quotaoff". 
$ 	2) Do not translate any command options preceeded by "-".
$ 	3) Translate any occurrence of "quotas". 
$

$ Messages for the quotaon command
$quote "
$set MS_QUOTAON

NAME		"4050-000 Name must be quotaon or quotaoff\n"
NOFSTAB		"4050-001 %s: not configured in /etc/filesystems\n"
$ Do not translate "/etc/filesystems". 
USE1		"Usage:\n\t%s [-g] [-u] [-v] -a\n"
USE2		"\t%s [-g] [-u] [-v] filesystem\n"
$ Translate "filesystem". 
QTOFF		"%s: quotas turned off\n"
$ Translate "quotas". 
QON		"quotaon: using %s on %s\n"
QTON		"%s: %s quotas turned on\n"
$ Translate "quotas". 
NOMNT		"4050-002 %s: not mounted\n"
FSRDONLY	"4050-003 %s: mounted read-only\n"
NOMEM 		"4050-004 Out of memory\n"
STALEFS 	"4050-005 Filesystem error: revision %d != %d\n"
$ Translate "Filesystem". 
