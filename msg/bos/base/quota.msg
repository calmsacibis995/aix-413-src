$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)63	1.1  src/msg/bos/base/quota.msg, bosmsg, msg411, GOLD410 2/9/94 09:13:00
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
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ Message to Translators:
$ 	1) Do not translate any command options preceeded by "-".
$ 	2) Translate any occurrence of "quotas". 
$
 
 
$quote "
$set MS_QUOTA

NOQUOTAONSYS	"4050-200 There are no quotas on this system\n"
$ Translate "quotas"
USE1		"Usage: quota [-guqv]"
$ Do not translate "quota" or "guqv".  The "guqv" are command options.
USE2		"\tquota [-qv] -u username ..."
$ Do not translate "quota", "qv".  "username" is equivalent to "User Name"
USE3		"\tquota [-qv] -g groupname ..."
$ Do not translate "quota", "qv".  "groupname" is equivalent to "Group Name"
$ The three messages above appear together
NOACC		"(no account)"
PERDENU		"4050-201 Quota: %s (uid %d): permission denied\n"
$ Do not translate "quota"
UNKUSR		"4050-202 Quota: %s: unknown user\n"
$ Do not translate "quota"
NOENT		"(no entry)"
PERDENG		"4050-203 Quota: %s (gid %d): permission denied\n"
$ Do not translate "quota".  "gid" is equivalent to "group identification number" 
UNKGRP		"4050-204 Quota: %s: unknown group\n"
$ Do not translate "quota"
FLIMIT		"4050-205 File limit reached on"
$ Maximum available files have been used
FGRACE		"4050-206 In file grace period on"
$ "grace period" is time/space alloted for error correction
FQUOTA		"4050-207 Over file quota on"
$ Translate "quota"
BLIMIT		"4050-208 Block limit reached on"
$ Maximum available blocks have been used
$ "block" is a partition of disk space
BGRACE		"4050-209 In block grace period on"
$ "grace period" is time/space alloted for error correction
$ "block" is a partition of disk space
BQUOTA		"4050-210 Over block quota on"
$ Translate "quota"
$ "block" is a partition of disk space
NONE		"none"
HEAD		"Disk quotas for %s %s (%cid %d): %s\n"
$ Translate "quotas"
MSGFS		"Filesystem"
MSGB		"blocks"
$ "block" is a partition of disk space
MSGQ		"quota"
$ Translate "quota"
MSGL		"limit"
MSGG		"grace"
MSGF		"files"
DAYS		"days"
NOMEM		"4050-212 Quota: out of memory\n"
$ Do not translate "quota"
RDERR		"4050-213 Quota: read error"
$ Do not translate "quota"
