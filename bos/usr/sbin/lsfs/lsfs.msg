$ @(#)47	1.21.1.8  src/bos/usr/sbin/lsfs/lsfs.msg, cmdfs, bos411, 9428A410j 1/6/94 15:59:03
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$ FUNCTIONS: lsfs msg file
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
$
$quote "         define quote character.

$len 
$set MS_LSFS        Define initial set#

$ Do not translate lsfs.
$ Do not translate %s, %c, %d, etc.
$	
$ message range: 0506-900	0506-949 inclusive

BADATTR              "%s: 0506-901 The attribute format is Attribute=Value.\n"

CANCRLV              "%s: 0506-902 Cannot create logical volume.\n"

CANTFINDVG           "%s: cannot find volume group for logical volume %s.\n"

CANTGETLG            "%s: 0506-903 Cannot get volumegroup id from odm.\n"
$ Do not translate volumegroup.  
$ Do not translate odm. 

CANTGETLV            "%s: 0506-904 Cannot get logical volume id from odm.\n"
$ id is identifier.  
$ Do not trnslate odm.  

CANTGLVSIZE          "%s: 0506-906 Cannot get logical volume size.\n"

CANTGVGSIZE          "%s: 0506-907 Cannot get volume group partition size.\n"

CANTREDU             "%s: 0506-908 Cannot reduce size of file system.\n"

DEFAULT              ". The default is %s.\n"

EXISTS               "%s: logical volume %s already being used for filesystem %s.\n"


FILEEXIST            "%s: 0506-909 %s file system already exists.\n"

IM_USAGE             "Usage: imfs [-lx] vgname ...\n"

LS_USAGE             "Usage: lsfs [-q] [-c|-l] {-a | -v Vfs| -u Group | Filesystem ...}\n"

NEEDDEV              "%s: 0506-909 Specify the device.\n"

NEEDMT               "%s: 0506-910 Specify the mount point.\n"

NEEDSIZE             "%s: 0506-911 Specify the size of journalled file systems.\n"
$ Do not translate journalled.  

NEEDVFS              "%s: 0506-912 Vfs type must be specified.\n"

NEEDVG               "%s: 0506-913 Specify one of volumegroup or device.\n"
$ Do not translate volumegroup.  

NEWSIZE              "New File System size is %ld\n"

NO                   "no"

NOLOG                "%s: 0506-914cannot find log for volume group %s.\n"

NOMATCH2             "%s: 0506-915 No record matching %s was found in %s.\n"

NOMEMORY             "%s: 0506-916 There is not enough memory available now.\n"

NOSUCHVFS            "%s: 0506-917 %s is not a known vfs type.\n"

ONEOFVG              "%s: 0506-919 Specify only one of volumegroup or device.\n"

OOPS                 "%s: This program should be called "

OR                   "or "

RM_USAGE             "Usage: rmfs Filesystem\n"

ROORRW               "%s: 0506-920 Specify ro or rw for the -p flag.\n"
$ Do not translate ro, rw. 

SIZEALREADY          "The filesystem size is already %ld.\n"

SIZECH               "Filesystem size changed to %s\n"

SIZEIG               "%s: Warning: device name given, size parameter ignored.\n"

STMNTED              "%s: 0506-921 %s is currently mounted.\n"

UPDATFAIL            "%s: 0506-922 The update of %s failed.\n"

WRONGSTANZA          "%s: Warning: logical volume %s already exists in %s.\n"

YES                  "yes"

YES1                 " yes\n"

$ Do not translate jfs

CANTGETYPE           "%s: 0506-923 Cannot get logical volume type from odm.\n"
JFSTYPE              "%s: 0506-924 %s: logical volume must be type jfs.\n"

INEGNOSENSE          "-i doesn't make sense unless you specify a delta\n"

USEONEONLY  	     "%s: 0506-925 can only use one of b,u,f,k,m for size\n"

INVALSIZE	     "%s: 0506-926 %s: invalid size\n"

EMPTYARG	     "%s: 0506-927 size arg is empty\n"

NOTFS		     "%s: 0506-928 %s: not a filesystem\n"

LV_TOO_BIG           "%s: Warning: logical volume %s larger than filesystem max (%ld).\n"

CH_USAGE             "Usage: chfs [-n Nodename] [-m NewMountpoint] [-u Group] [-A {yes|no}]\n \t[-t {yes|no}] [-p {ro|rw}] [-a Attribute=Value] [-d Attribute]\n \tFilesystem\n"
$ Do not translate ro, rw.  

CR_USAGE             "Usage: crfs -v Vfs {-g Volumegroup | -d Device} -m Mountpoint\n\t[-u Mountgroup] [-A {yes|no}] [-t {yes|no}] [-p {ro|rw}]\n \t[-l Logpartitions] [-n nodename] [-a Attribute=Value]\n"

HEADERINFO           "Name\t\tNodename\tMount Pt\tVFS\tSize\tOptions\tAuto\tAccounting\n"

FRAG_VALUE	"%s:  506-929 Invalid fragment size: -a frag=%s\n"

NBPI_VALUE	"%s:  506-930 Invalid number of bytes per inode: -a nbpi=%s\n"

LVSIZE		 "lv size%s%s%s"

FSSIZE		"fs size%s%s%s"

FGSIZE		"frag size%s%s%s"

NBPISIZE		"nbpi%s%s%s"

COMPALGOR	"compress%s%s"

BADTYPES             "%s: 0506-931 This type of filesystem is not supported by jfs.\n"

MAXSIZE	"Based on the parameters chosen, the new %s JFS file system\nis limited to a maximum size of %u (512 byte blocks)\n\n"

MAXFS_SIZE	"%s:  0506-932  Can not extend a JFS file system with nbpi = %d\nand frag = %d past %u (512 byte blocks).\n"

NOTJFS		"%s:  0506-933  %s is not recognized as a JFS filesystem.\n"

NOTSUPJFS	"%s:  0506-934  %s is not a supported JFS filesystem version.\n"

CORRUPTJFS	"%s:  0506-935  The %s JFS filesystem superblock is corrupted.\n"

CANTRESB	"%s:  0506-936  Cannot read superblock on %s.\n"
