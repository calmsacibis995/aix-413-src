$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)10	1.1  src/msg/bos/base/fsck.msg, bosmsg, msg411, GOLD410 2/9/94 08:59:04  
$
$  COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$  FUNCTIONS: fsck messages
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$quote "         define quote character.

$len 
$set MS_FSCK        Define initial set#

$ Message numbers have been assigned a range of 030-040
$ Do not translate fsck, %d, %s, %1$d, %1$s, etc ...  


BADROOT              "There are root file system problems."

CANTFORK             "fsck: 0506-030 Cannot create a process at this time."

CANTSTAT             "fsck: 0506-031 Cannot get information about root %s."
$ Root is Unix technical term.  

CHANGM               "There are changes to the mounted file systems."

CHECKE               "%1$s: %2$s: check=%3$s? (using check=1)"

MNTCTLF              "fsck: 0506-032 The mntctl call failed: %s"
$ Do not translate mntctl.   
$ %s will be replaced by an error message. 

NODEVIG              "%1$s: There is no device for %2$s. It is ignored."

OUTSPAC              "fsck: 0506-033 There is not enough memory available now."

OUTSPAC1             "fsck: 0506-034 There is not enough memory available now."

TERMDONE             " (TERMINATED)"
$ Leave one blank space at beginning of message.   

UNKFILSYS            "fsck: 0506-035 %s is not a known file system."

USAGE                "Usage: fsck [-y|-n|-p] [-f] [-V Vfs] [-d #] [-i #]\n\
\t[-t File] [-o Options] Filesystem ...\n"
$ Do not translate Vfs.
