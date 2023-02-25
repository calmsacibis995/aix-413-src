$ @(#)20	1.3.1.2  src/bos/usr/sbin/quota/edquota.msg, cmdquota, bos411, 9428A410j 9/29/93 13:45:07
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
$ 	1) Do not translate any occurrences of "edquota"
$ 	2) Do not translate any command options preceded by "-".
$
 
$quote "
$set MS_EDQUOTA

PERDEN		"4050-300 edquota: permission denied\n"
USE1		"Usage: edquota [-u] [-p username] username ...\n"
$ "username" is equivalent to "User Name"
USE2		"\tedquota -g [-p groupname] groupname ...\n"
$ "groupname" is equivalent to "Group Name"
USE3		"\tedquota [-u] -t\n\tedquota -g -t\n"
NOUSER		"4050-301 %s: no such user\n"
NOGRP		"4050-302 %s: no such group\n"
UNKTYPE		"4050-303 %d: unknown quota type\n"
$ Translate "quota"
NOMEM		"4050-304 edquota: out of memory\n"
WARN		"4050-305 Warning: %s\n"
NOQUOTAS	"4050-105 Quotas are not enabled on this machine"
$ Translate "Quotas"
CREATEQ		"Creating quota file %s\n"
$ Translate "quota"
RDERR		"4050-307 edquota: read error in "
EDQ		"edquota: "
PROC		"4050-308 You have too many processes\n"
FORK		"fork"
$ Do not translate "fork".  "fork" is the creation of a process
QUOTAS		"Quotas for %s %s:\n"
QLIM1		 "%s: %s %d, limits (soft = %d, hard = %d)\n"
BUSE		"blocks in use:"
$ "block" is a partition of disk space
QLIM2		"%s %d, limits (soft = %d, hard = %d)\n"
IUSE		"\tinodes in use:"
$ Do not translate "inodes".  "inode" is equivalent to a file
TMP		"4050-309 Can't read temporary file again\n"
FORM1		"4050-310 %s: bad format\n"
FORM2		"4050-311 %s: %s: bad format\n"
BLKUSE		" blocks in use: %d, limits (soft = %d, hard = %d)"
$ the first character of BLKUSE must be a blank
$ "block" is a partition of disk space
INOUSE		"\tinodes in use: %d, limits (soft = %d, hard = %d)"
$ Do not translate "inodes".  "inode" is equivalent to a file
ALLOC		"4050-312 %s: cannot change current allocation\n"
TIME		"Time units may be: days, hours, minutes, or seconds\n"
GRACE		"Grace period before enforcing soft limits for %ss:\n"
$ "grace period" is time/space alloted for error correction
BGRACE		"%s: block grace period: %s, "
$ "grace period" is time/space alloted for error correction
$ "block" is a partition of disk space
FGRACE		"file grace period: %s\n"
$ "grace period" is time/space alloted for error correction
BFGRACE		" block grace period: %d %s file grace period: %d %s"
$ the first character of BFGRACE must be a blank
$ "grace period" is time/space alloted for error correction
$ "block" is a partition of disk space
DAYS		"days"
HOURS		"hours"
MINUTES		"minutes"
SECONDS		"seconds"
SEC		"second"
MINS		"minute"
HR		"hour"
DAY		"day"
BADUNIT		"4050-313 %s: bad units, specify %s\n"
$ "units" refer to "time units"
DHMS		"days, hours, minutes, or seconds"
