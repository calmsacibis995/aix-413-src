$  @(#)33	1.8.2.2  src/bos/usr/sbin/mount/mount.msg, cmdfs, bos411, 9428A410j 3/18/94 08:58:02
$
$  COMPONENT_NAME: (CMDFS) commands that manipulate files
$
$  FUNCTIONS: message file
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
$
$ mount and umount's messages
$
$quote "         define quote character.

$len 

$set MS_BOTH        Define initial set#

$ Message numbers have been assigned a range of 300-355
$ Do not translate mount, %s, %d, %1$s, %1$d, etc ...

Err_CALLEDWRONG      "0506-300 This program should be called mount or umount, not %s.\n"

Err_FSTABLING        "0506-301 Cannot construct file system data table.\n"

Err_NOMEM            "0506-302 There is not enough memory available now.\n"

Err_STALEFS          "0506-303 The filesystem is stale.  Revision %1$d != revision %2$d.\n"
$ Do not translate stale

Err_MNTCTL           "0506-304 The mntctl call failed."
$ Do not translate mntctl.

Err_FULNAM           "0506-305 Cannot form the full pathname of %s.\n"

Err_FORKFAIL         "0506-306 Cannot create the mount helper process at this time.\n"

Err_AFOPENFAIL       "0506-307 The AFopen call failed.\n"
$ Do not translate AFopen

Err_NOTYPES          "0506-308 There are no stanzas of type %s.\n"

Err_UNKGFS           "0506-309 %1$s has a gfstype %2$d that is not known.\n"

Misc_DATE_FMT        "%sD %sT"

$set MS_MOUNT        Redefine set#

Err_INVALTOPT1       "0506-310 \t%1$s%2$s%3$s%4$s%5$s%6$s%7$s.\n"

Err_INVALTOPT2       "is not valid with the -t flag."
$ Do not add a newline

Err_INHERITMLT       "0506-311 Cannot inherit multiple mounts.\n"

Err_RMTANDRMV        "0506-312 Remote mounts on removeable media are not allowed.\n"

Err_NODEANDALL       "0506-313 The nodename option and the all option should not both be specified.\n"

Err_STUBPERM         "0506-314 Write permission is required to mount over %s.\n"

Err_NOTPRIV          "0506-315 Only the root user or a system group member can perform mount all.\n"
$ Do not translate mount all

Err_OVEROOT          "0506-316 The root filesystem cannot be overmounted.\n"

Err_UNKVFS           "0506-317 %s is not a known vfs type.\n"

Err_UNKVFSOBJ        "0506-318 %1$s is not a known vfs type for object %2$s.\n"

Err_NOHELPER         "0506-319 The mount helper is not defined for %s.\n"

Err_RMVNOTDEV        "0506-320 Only device mounts can be removable.\n"

Err_UNCLEAN          "0506-321 %s is not a clean filesystem.\n"

Err_NOLOG            "0506-322 Cannot determine log device to use for %1$s (%2$s).\n"

Err_STATLOG          "0506-323 Cannot get information about log device %s.\n"

Err_MNTFAIL          "0506-324 Cannot mount %1$s on %2$s: "

Err_PARSEMNT         "0506-325 Cannot parse the mount buffer.\n"

Err_INVALOPT         "0506-326 %s is not a valid flag.\n"

Err_TAXES            "0506-327 Cannot continue inheriting.\n"

Err_UNKVFSTYPE       "0506-328 Cannot find the vfs type.\n"

Err_STRLENFAIL       "0506-329 The strlen call failed.\n"

Err_STRCPYFAIL       "0506-330 The strcpy call failed.\n"

Err_HELPERFAIL       "0506-331 The file system helper failed.\n"

Err_UNKOBJ           "0506-332 Specify the %1$s for stanza %2$s.\n"

Err_UNKOBJLOCAL      "mount object"

Err_UNKOBJREMOT      "remote path"

Err_NOP              "0506-333 %s is already mounted.\n"

Err_UNKFS            "0506-334 %s is not a known file system.\n"

Err_PIPEFAIL         "0506-335 The pipe call failed.\n"
$ Do not translate pipe

Err_RMTBADSTAT       "0506-336 The mount helper returned status %d.\n"

Err_NODFLTVFS        "0506-337 Cannot determine the default vfs type for %s.\n"

Err_INHERITQUAL      "0506-338 Object %s for the inherit mount is not qualified fully.\n"

Err_INHERITSTAT      "0506-339 Cannot get the mount status buffer for the inherit mount.\n"

Err_STATFAIL         "0506-340 Cannot get information about %s: "

Err_MOUNTUSAGE       "Usage: mount [-fipr] [-n Node] [-o Options] [-t Type] [-{v|V} Vfs]\n\
\t[-a | all | [[Node:]Device] [Directory]]\n"

Misc_INHERITMNT      "inherited mount of %1$s on %2$s.\n"

Err_MODES_WARNING    "0506-341 warning: the permissions on %1$s are incompatible\n\
\twith the permissions in the root directory of %2$s.\n"

Err_SBDIRTY          "0506-342 The superblock on %s is dirty.  Run a full fsck to fix.\n"

Err_COMPNOTLOADED    "0506-350 The compression algorithm is either not present or not loaded\n          into the kernel.\n"

$set MS_UMOUNT        Redefine set#

Err_GUIDFAIL         "0506-345 The getuid call failed.\n"

Err_SUIDFAIL         "0506-346 The setuid call failed.\n"

Err_UNOP             "0506-347 Cannot find anything to unmount.\n"

Err_USAGEUMNT        "Usage: umount [-sf] {-a|-n Node|-t Type|all|allr|Device|File|Directory|Filesystem}\n"

Err_UMNTFAIL         "0506-349 Cannot unmount %s: "

Misc_UFORCE          "forced unmount of %s\n"

Misc_REPLAY_LOG      "Replaying log for %s.\n"
