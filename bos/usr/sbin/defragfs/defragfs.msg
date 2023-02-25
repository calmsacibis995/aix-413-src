
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


$set MS_DEFRAGFS        Redefine set#

$ $ Message numbers have been assigned a range of 400-449
$ Do not translate defragfs , %s, %d, %1$s, %1$d, etc ... 


USAGE    "Usage : defragfs [-q | -r] {device | mount-path} \n"
NOTJFS   "defragfs: 0506-400 %s is not recognized as a JFS file system. \n"
VERSION  "defragfs: 0506-401 %s JFS version number not supported. \n"
BITMAP   "defragfs: 0506-402 error reading bit map %d \n"
MALLOC   "defragfs: 0506-403 malloc failed \n"
IOERR    "defragfs: 0506-404 i/o read error \n"
ALLOCBIT "defragfs: 0506-405 bit allocator internal error \n"
SUPBLK   "defragfs: 0506-406 i/o error reading superblock \n"
MAPINC   "defragfs: 0506-407 bad bit map size: size =%d, mapsize = %d\n"
OPENFAIL "defragfs: 0506-408 failure in opening device %s \n" 
READONLY "defragfs: 0506-409 %s must be mounted read-write \n" 
GETVFS   "defragfs: 0506-410 getvfsnumber internal error \n" 
MNTERR   "defragfs: 0506-411 mount of %s failed  \n" 
UMNTERR  "defragfs: 0506-412 device %s got unmounted \n" 
STANZA   "defragfs: 0506-413 no stanza for %s in /etc/filesystems"
BEFORE   "statistics before running defragfs:\n"
AFTER    "statistics after running defragfs:\n"
OTHER    "other statistics:\n"
NFMOVED  "number of fragments moved %d\n"
NBMOVED  "number of logical blocks moved %d\n"
NALLOC   "number of allocation attempts %d\n"
NEXACT   "number of exact matches %d\n"
NSHORT   "number of free spaces shorter than a block %d\n"
NSFRAG   "number of free fragments in short free spaces %d\n"
NFREE    "number of free fragments %d\n"
NINUSE   "number of allocated fragments %d\n"

