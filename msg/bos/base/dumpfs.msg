$ ENGLISH_SOURCE_VERSION= 1.9.2.4
$ @(#)78	1.1  src/msg/bos/base/dumpfs.msg, bosmsg, msg411, GOLD410 2/9/94 08:57:25
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "         define quote character.

$len
$set Define        Define initial set#


$set MS_DUMPFS        Redefine set#

$ $ Message numbers have been assigned a range of 010-026 
$ Do not translate dumpfs, %s, %d, %1$s, %1$d, etc ... 

USAGE		"Usage: dumpfs FileSystem ...\n"

NOTJFS	"dumpfs:  0506-010  %s is not recognized as a JFS filesystem.\n"

NOTSUPJFS	"dumpfs:  506-011  %s is not a supported JFS filesystem version.\n"

CORRUPTJFS	"dumpfs:  506-012  The %s JFS filesystem superblock is corrupted.\n"

CANTRESB	"dumpfs:  506-013 Cannot read superblock on %s.\n"

CANTMAP		"dumpfs: 0506-014 Cannot map logical to physical block.\n"

CANTLSEEK	"dumpfs: 0506-015 Cannot lseek.\n"
$ Do not translate lseek. 

CANTGI		"dumpfs: 0506-016 Cannot get inode for inode map.\n"
$ Do not translate inode, map. 

CANTGB		"dumpfs: 0506-017 Cannot get inode for disk block map\n"


MAGICN		"magic\t\t\t0x%x%x%x%x\tcpu type\t\t0x%x\n"

FILSYSTY	"file system type\t%d\t\tfile system version\t%d\n"

FILSYSSI	"block size\t\t%d\t\tallocation group size\t%d (frags)\n"),

FILSYSNA	"file system name\t%.*s\t\tvolume name\t\t%.*s\n"

LOGDEVC		"log device\t\t0x%-13.x log serial number\t0x%x\n"

FILSYSSTA	"file system state\t%d\t\tread only\t\t%d\n"

LASTUP		"last change\t\t%s"

DINODEMAP	"\n\
Inode Map:\n"

DISKBLKMAP	"\n\
Disk Block Map:\n"

PAGCNT		"PAGE %d: size=%d, freecnt=%d, agsiz=%d, agcnt=%d\n"

AGSIZ	"totalags=%d, lastalloc=%d, clsize=%d, clmask=0x%x, version=%d\n"

AGCNT		"ags:"
$ Do not translate.

TREE		"tree:"

WPMAP		"maps:"

WMAP		"wmap:"

PMAP		"pmap:"

FILSYSSZ	"file system size\t%-15.d fragment size\t\t%d\n"

IAGSIZE 	"inodes/allocation grp\t%d\t\tcompress\t\t%d\n"
