$ ENGLISH_SOURCE_VERSION= 1.6.2.4
$ @(#)00	1.1  src/msg/bos/base/ncheck.msg, bosmsg, msg411, GOLD410 2/9/94 09:09:08
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
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

$set MS_NCHECK

$ Do not translate superblock

BADFLAG         " bad flag %c\n"

BADFSIZE	"ncheck: 0506-130  Superblock has corrupted filesystem size.\n"

NOTJFS	"ncheck:  0506-131  %s is not recognized as a JFS filesystem.\n"

NOTSUPJFS "ncheck:  0506-132  %s is not a supported JFS filesystem version.\n"

CORRUPTJFS	"ncheck:  0506-133  The %s JFS filesystem superblock is corrupted.\n"

CANTRESB	"ncheck:  0506-134  Cannot read superblock on %s.\n"

CANTOPEN	"ncheck: 0506-135  Cannot find or open %s.\n"

CANTRDIR	"ncheck: 0506-136  Cannot read directory block for inode %d.\n"

OUTCORE		"ncheck: 0506-137  There is not enough memory available.\n"

TOOSPEC		"ncheck: 0506-138  There are too many special files.\n"

SKIPPING	"skipping...\n"

USAGE		"Usage:  ncheck [ [-a] [-i InodeNumbers ...] | [-s] ] [FileSystem]\n"
