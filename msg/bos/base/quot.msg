$ ENGLISH_SOURCE_VERSION= 1.2.1.1
$ @(#)62	1.1  src/msg/bos/base/quot.msg, bosmsg, msg411, GOLD410 2/9/94 09:12:56
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
$quote "
$set MS_QUOT

USAGE	"usage: quot [-cfhn] [filesystem ...]\n"
$ Do not translate "quot" and "cfhn".  "cfhn" is command options
$ Translate "filesystem"
READERR	"4050-400 quot: read error at block %ld\n"
$ Do not translate "quot"
$ "block" is a partition of disk space
BADFSIZE "4050-401 quot: super block has corrupted filesystem size\n"
$ Do not translate "quot"
CANTREAD "4050-402 quot: Cannot read super block on filesystem %s\n"
$ Do not translate "quot"
$ %s will be replaced by the filesystem name
